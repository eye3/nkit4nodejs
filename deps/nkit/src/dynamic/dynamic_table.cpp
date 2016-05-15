/*
   Copyright 2010-2014 Boris T. Darchiev (boris.darchiev@gmail.com)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "nkit/dynamic.h"

#include <cstring>
#include <algorithm>

namespace nkit
{
  namespace detail
  {
    //--------------------------------------------------------------------------
    KeyItem make_key_item(const Data data, const uint64_t type)
    {
      KeyItem key;
      switch (type)
      {
      case INTEGER:
        key.i64_ = data.i64_;
        break;
      case BOOL:
      case DATE_TIME:
      case UNSIGNED_INTEGER:
        key.ui64_ = data.ui64_;
        break;
      case STRING:
        key.shared_string_ = data.shared_string_;
        break;
      case FLOAT:
        key.f_ = data.f_;
        break;
      default:
        key.i64_ = 0;
        nkit::abort_with_core("Impossible case");
        break;
      }
      return key;
    }

    KeyItem make_key_item(const Dynamic & v)
    {
      return make_key_item(v.data_, v.type_);
    }

    //--------------------------------------------------------------------------
    struct DynamicTypeTableItem
    {
      std::string pattern_;
      uint64_t type_;
    };

    //--------------------------------------------------------------------------
    static const DynamicTypeTableItem types_map_[] =
    {
      { std::string("STRING")             , STRING },
      { std::string("UNSIGNED_INTEGER")   , UNSIGNED_INTEGER },
      { std::string("INTEGER")            , INTEGER },
      { std::string("FLOAT")              , FLOAT },
      { std::string("BOOL")               , BOOL },
      { std::string("DATE_TIME")          , DATE_TIME },
      { std::string("")                   , DYNAMIC_TYPES_COUNT }
    };

    //--------------------------------------------------------------------------
    const std::string & dynamic_type_to_string(const uint64_t type)
    {
      size_t i = 0;
      while (!types_map_[i].pattern_.empty())
      {
        if (types_map_[i].type_ == type)
         return types_map_[i].pattern_;
        ++i;
      }
      return S_EMPTY_;
    }

    //--------------------------------------------------------------------------
    uint64_t string_to_dynamic_type(const std::string & str)
    {
      size_t i = 0;
      while (!types_map_[i].pattern_.empty())
      {
        if (NKIT_STRCASECMP(types_map_[i].pattern_.c_str(), str.c_str()) == 0)
          return types_map_[i].type_;
        ++i;
      }
      return DYNAMIC_TYPES_COUNT;
    }

    struct DynamicTypeAffinityItem
    {
      int64_t from_;
      int64_t to_;
    };

    static const DynamicTypeAffinityItem types_affinity_[] =
    {
      { STRING            , STRING },
      { UNSIGNED_INTEGER  , UNSIGNED_INTEGER },
      { DATE_TIME         , UNSIGNED_INTEGER },
      { BOOL              , UNSIGNED_INTEGER },
      { INTEGER           , INTEGER },
      { FLOAT             , FLOAT }
    };

    //--------------------------------------------------------------------------
    const size_t types_affinity_size_ =
        sizeof(types_affinity_)/sizeof(types_affinity_[0]);

    //--------------------------------------------------------------------------
    int64_t get_dynamic_type_affinity(const int64_t type)
    {
      for (size_t i=0; i < types_affinity_size_; ++i)
      {
        if (types_affinity_[i].from_ == type)
         return types_affinity_[i].to_;
      }
      return DYNAMIC_TYPES_COUNT;
    }

    //--------------------------------------------------------------------------
    bool parse_table_def_item(const std::string & query,
        std::string * name, uint64_t * vtype)
    {
      std::string stype;
      simple_split(query, ":", name, &stype);
      if (stype.empty())
        stype = "STRING";
      *vtype = string_to_dynamic_type(stype);
      return *vtype != DYNAMIC_TYPES_COUNT;
    }

    //--------------------------------------------------------------------------
    const AggregatorFunction aggregator_functions_[] =
    {
      { "COUNT" , AT_COUNT },
      { "MIN"   , AT_MIN   },
      { "MAX"   , AT_MAX   },
      { "SUM"   , AT_SUM   }
    };

    //--------------------------------------------------------------------------
    const size_t aggregator_functions_size_ =
        sizeof(aggregator_functions_)/sizeof(aggregator_functions_[0]);

    //--------------------------------------------------------------------------
    AggregatorType StringToAggregatorType(const std::string & aggr_name)
    {
      for (size_t i=0; i < aggregator_functions_size_; ++i)
      {
        if (NKIT_STRCASECMP(aggregator_functions_[i].name_, aggr_name.c_str())
            == 0)
          return aggregator_functions_[i].type_;
      }

      return AT_MAX_VALUE;
    }

    //--------------------------------------------------------------------------
    GroupIndex::Ptr GroupIndex::Create(const SharedTable * shared_table,
        const std::string & index_def, const std::string & aggr,
        std::string * error)
    {
      if (index_def.empty())
      {
        *error = "Index definitions could not be empty";
        return Ptr();
      }

      if (aggr.empty())
      {
        *error = "Aggregator definitions could not be empty";
        return Ptr();
      }

      StringVector grouped_table_def;
      SizeVector column_nums;
      StringVector mask;
      StringVector column_names;
      simple_split(index_def, ",", &column_names);
      StringVector::const_iterator _column_name = column_names.begin(),
          column_name_end = column_names.end();
      for (; _column_name != column_name_end; ++_column_name)
      {
        std::string column_name(*_column_name);
        bool minus = false;
        if (column_name[0] == '-')
        {
          column_name.erase(0, 1);
          minus = true;
        }
        size_t col_num = shared_table->column_number(column_name);
        if (col_num == Dynamic::npos)
        {
          *error = "Could not find column '" + column_name + "'";
          return Ptr();
        }

        int64_t type = shared_table->get_column(col_num).type_;
        int64_t affinity_type = get_dynamic_type_affinity(type);
        if (affinity_type == DYNAMIC_TYPES_COUNT)
        {
          *error = "Unknown affinity for type '" +
              dynamic_type_to_string(affinity_type) + "'";
          return Ptr();
        }

        mask.push_back(string_cast(minus ? -affinity_type : affinity_type));
        grouped_table_def.push_back(column_name + ":" +
            dynamic_type_to_string(type));
        column_nums.push_back(col_num);
      }

      AggregatorVector aggregators;
      StringVector aggr_defs;
      simple_split(aggr, ",", &aggr_defs);
      StringVector::const_iterator aggr_def = aggr_defs.begin(),
          aggr_def_end = aggr_defs.end();
      for (; aggr_def != aggr_def_end; ++aggr_def)
      {
        std::string aggr_func, column_name_, column_name, dummi;
        simple_split(*aggr_def, "(", &aggr_func, &column_name_);
        simple_split(column_name_, ")", &column_name, &dummi);
        AggregatorType aggr_type = StringToAggregatorType(aggr_func);
        size_t col_num(0);
        uint64_t col_type(DYNAMIC_TYPES_COUNT);
        if (aggr_type == AT_MAX_VALUE)
        {
          *error = "Unknown aggregator function '" + aggr_func + "'";
          return Ptr();
        }
        else if (aggr_type != AT_COUNT && !column_name.empty())
        {
          col_num = shared_table->column_number(column_name);
          if (col_num == Dynamic::npos)
          {
            *error = "Could not find column '" + column_name + "'";
            return Ptr();
          }
          col_type = shared_table->get_column(col_num).type_;
          if (col_type == STRING)
          {
            *error = "Column '" + column_name +
                "' could not be used with SUM/MIN/MAX aggregators:"
                " its type is STRING";
            return Ptr();
          }
        }
        else if (aggr_type == AT_COUNT)
        {
          column_name = "COUNT";
          col_type = UNSIGNED_INTEGER;
        }
        else
        {
          *error = "Aggregator function '" + aggr_func +
              "' must be provided with column name";
          return Ptr();
        }

        grouped_table_def.push_back(column_name + ":" +
            dynamic_type_to_string(col_type));
        aggregators.push_back(Aggregator::Create(col_num, col_type, aggr_type));
      }

      IndexCompare comp;
      if (!GetComparator(mask, &comp, error))
      {
        return Ptr();
      }

      Dynamic grouped_table = Dynamic::Table(grouped_table_def, error);
      if (!grouped_table.IsTable())
        return Ptr();

      return Ptr(new GroupIndex(grouped_table, column_nums, aggregators, comp));
    }

    //--------------------------------------------------------------------------
    GroupIndex::GroupIndex(Dynamic grouped_table,
        const SizeVector & index_column_nums,
        const AggregatorVector & aggregators,
        const IndexCompare & cmp)
      : index_column_nums_(index_column_nums)
      , index_columns_count_(index_column_nums_.size())
      , aggregators_(aggregators)
      , aggr_columns_count_(aggregators_.size_)
      , grouped_table_(grouped_table)
      , current_row_(index_columns_count_ + aggr_columns_count_)
      , index_map_(cmp)
      , index_key_(0)
    {
    }

    //--------------------------------------------------------------------------
    GroupIndex::~GroupIndex()
    {
    }

    //--------------------------------------------------------------------------
    bool GroupIndex::UpdateIndex(const Data * row)
    {
      IndexMap::iterator key = index_map_.find(index_key_);
      if (key != index_map_.end())
      {
        Bucket & bucket = key->second;
        for (size_t i = 0; i < aggregators_.size_; ++i)
          aggregators_.vfunc_[i]->Update(&(bucket[i]), row);
        return false;
      }
      else
      {
        Bucket bucket(aggregators_);
        for (size_t i = 0; i < aggregators_.size_; ++i)
          aggregators_.vfunc_[i]->Update(&(bucket[i]), row);
        index_map_[index_key_] = bucket;
        return true;
      }
    }

    //--------------------------------------------------------------------------
    bool GroupIndex::UpdateIndex(const DataVector & data,
        const DynamicTypeVector & types)
    {
      index_key_.size_ = 0;

      SizeVector::const_iterator col = index_column_nums_.begin(),
            last = index_column_nums_.end();
      for (; col != last; ++col)
      {
        Data d = data[*col];
        const uint64_t arg_type = types[*col];
        index_key_.key_[index_key_.size_] = make_key_item(d, arg_type);
        ++index_key_.size_;
      }

      if (UpdateIndex(data.data()))
      {
        IndexMap::const_iterator it = index_map_.find(index_key_);
        MakeRow(it);
        grouped_table_.data_.shared_table_->AppendRowUnsafe(current_row_);
      }

      return true;
    }

    bool GroupIndex::UpdateIndex(const SharedTable & src_tbl, const Data * row)
    {
      index_key_.size_ = 0;

      SizeVector::const_iterator col = index_column_nums_.begin(),
            last = index_column_nums_.end();
      for (; col != last; ++col)
      {
        const Data data = row[*col];
        const uint64_t type = src_tbl.get_column(*col).type_;

        index_key_.key_[index_key_.size_] = make_key_item(data, type);
        ++index_key_.size_;
      }

      return UpdateIndex(row);
    }

    //--------------------------------------------------------------------------
    Dynamic GroupIndex::Build(const SharedTable & src_tbl)
    {
      size_t rows = src_tbl.height();
      if (rows == 0)
        return grouped_table_;

      // fill index
      for (size_t i = 0; i < rows; ++i)
      {
        const Data * row = src_tbl.storage_->get(i);
        UpdateIndex(src_tbl, row);
      }

      // make table from index
      SharedTable * dst_tbl = grouped_table_.data_.shared_table_;
      IndexMap::const_iterator it = index_map_.begin(), end = index_map_.end();
      for (; it != end; ++it)
      {
        MakeRow(it);
        dst_tbl->AppendRowUnsafe(current_row_);
      }

      return grouped_table_;
    }

    Dynamic GroupIndex::Build()
    {
      // update table from index
      SharedTable * dst_tbl = grouped_table_.data_.shared_table_;
      IndexMap::const_iterator it = index_map_.begin(), end = index_map_.end();
      for (size_t r = 0; it != end; ++it, ++r)
      {
        MakeRow(it);
        dst_tbl->SetRowUnsafe(current_row_, r);
      }

      return grouped_table_;
    }

    void GroupIndex::MakeRow(IndexMap::const_iterator & it)
    {
      for (size_t i = 0; i < index_columns_count_; ++i)
        current_row_[i].i64_ = it->first.key_[i].i64_;

      for (size_t i = 0; i < aggr_columns_count_; ++i)
        current_row_[index_columns_count_ + i].i64_ = it->second[i].i64_;
    }

    //--------------------------------------------------------------------------
    SharedTable * SharedTable::Create(const std::string & table_def,
        std::string * error)
    {
      StringVector vdef;
      simple_split(table_def, ",", &vdef);
      return Create(vdef, error);
    }

    //--------------------------------------------------------------------------
    SharedTable * SharedTable::Create(const StringVector & table_def,
        std::string * error)
    {
      if (table_def.empty())
      {
        if (error)
          *error = "Table definition could not be empty";
        return NULL;
      }

      Columns columns;
      StringVector::const_iterator col = table_def.begin(),
          end = table_def.end();
      for (; col != end; ++col)
      {
        std::string name;
        uint64_t type;
        if (!parse_table_def_item(*col, &name, &type))
        {
          if (error)
            *error = "Wrong table definition: '" + *col + "'";
          return NULL;
        }
        columns.push_back(Column(name, type));
      }

      return new SharedTable(columns);
    }

    //--------------------------------------------------------------------------
    SharedTable::SharedTable(const Columns & columns)
      : RefCounted()
      , rows_(0)
      , columns_(columns)
      , storage_(new StorageImpl(0))
      , table_index_set_()
    {
      storage_->set_grow_factor(columns_.size());
    }

    //--------------------------------------------------------------------------
    SharedTable::~SharedTable()
    {
      Clear();
      delete storage_;
      storage_ = NULL;
    }

    //--------------------------------------------------------------------------
    void SharedTable::Clear()
    {
      size_t const col_size = columns_.size();
      for (size_t row_num = 0; row_num != rows_; ++row_num)
      {
        Data * cache = storage_->get(row_num);
        for (size_t col_num = 0; col_num < col_size; ++col_num)
        {
          Data d = cache[col_num];
          const uint64_t type = columns_[col_num].type_;
          if (is_ref_counted(type))
            Operation<OP_DEC_REF_DATA>::farray[type](d);
        } // for
      }

      UnSubscribeAll();
      columns_.clear();
      storage_->clear();
      rows_ = 0;
      // XXX free storage memory here?
    }

    //--------------------------------------------------------------------------
    StringVector SharedTable::column_names() const
    {
      StringVector ret;
      Columns::const_iterator it = columns_.begin(), last = columns_.end();
      for ( ;it != last; ++it)
        ret.push_back(it->name_);
      return ret;
    }

    //--------------------------------------------------------------------------
    StringVector SharedTable::column_types() const
    {
      StringVector ret;
      Columns::const_iterator it = columns_.begin(), last = columns_.end();
      for ( ;it != last; ++it)
        ret.push_back(dynamic_type_to_string(it->type_));
      return ret;
    }

    //--------------------------------------------------------------------------
    void SharedTable::SetColumnName(size_t pos, const std::string & name)
    {
      if (pos >= width())
        return;

      if (column_number(name) != Dynamic::npos) // this name should not exists
        columns_[pos].name_ = name;
    }

    //--------------------------------------------------------------------------
    size_t SharedTable::column_number(const std::string & column_name) const
    {
      Columns::const_iterator it = columns_.begin(), last = columns_.end();
      for (size_t pos = 0 ;it != last; ++it, ++pos)
      {
        if (column_name == it->name_)
            return pos;
      }

      return Dynamic::npos;
    }

    //--------------------------------------------------------------------------
    SharedTable::SharedTable(const SharedTable & from)
      : rows_(from.rows_)
      , columns_(from.columns_)
      , storage_(from.storage_->clone())
    {
    }

    //--------------------------------------------------------------------------
    SharedTable * SharedTable::Clone(bool full) const
    {
      SharedTable * result = new SharedTable(*this);

      size_t col_count = columns_.size();
      for (size_t r = 0; r < rows_; ++r)
      {
        Data * row = result->storage_->get(r);
        for (size_t c = 0; c < col_count; ++c)
        {
          uint64_t type = columns_[c].type_;
          if (is_ref_counted(type))
          {
            if (full)
            {
              Dynamic tmp(Operation<OP_CLONE>::farray[type](row[c]));
              Operation<OP_INC_REF_DATA>::farray[type](tmp.data_);
              row[c].i64_ = tmp.data_.i64_;
            }
            else
            {
              Operation<OP_INC_REF_DATA>::farray[type](row[c]);
            }
          }
        }
      }

      return result;
    }

    //--------------------------------------------------------------------------
    Dynamic SharedTable::GetCellValue(const size_t row_num,
      const size_t col_num) const
    {
      if (row_num >= height())
        return D_NONE;

      if (col_num >= width())
        return D_NONE;

      const Data data = *(storage_->get(row_num) + col_num);
      const uint64_t type = columns_[col_num].type_;
      Dynamic ret;
      ret.FromData(type, data);
      return Dynamic(ret);
    }

    //--------------------------------------------------------------------------
    bool SharedTable::AppendRow(const DynamicVector & vargs)
    {
      if (!Validate(vargs))
        return false;

      Data * cache = storage_->extend();

      size_t const col_size = columns_.size(), args_size = vargs.size();
      for (size_t col_num = 0; col_num < col_size; ++col_num)
      {
        const uint64_t col_type = columns_[col_num].type_;
        if (col_num < args_size)
        {
          Data d = vargs[col_num].data_;
          const uint64_t arg_type = vargs[col_num].type_;
          if (col_type != arg_type)
          {
            storage_->remove(height());
            return false;
          }
          if (is_ref_counted(arg_type))
            Operation<OP_INC_REF_DATA>::farray[arg_type](d);
          cache[col_num] = d;
        }
        else
        {
          cache[col_num] = GetDefault(col_type);
        }
      }

      TableIndexSet::const_iterator index = table_index_set_.begin(),
        index_last = table_index_set_.end();
      for (; index != index_last; ++index)
        (*index)->NotifyRowInsert(cache, rows_, false);
      ++rows_;

      return true;
    }

    //--------------------------------------------------------------------------
    bool SharedTable::DeleteRow(const size_t row_num)
    {
      if (row_num >= height())
        return false;

      Data * cache = storage_->get(row_num);
      TableIndexSet::const_iterator index = table_index_set_.begin(),
            index_last = table_index_set_.end();
      for (; index != index_last; ++index)
        (*index)->NotifyRowDelete(cache, row_num, true);

      const size_t col_size = columns_.size();
      for (size_t col_num = 0; col_num < col_size; ++col_num)
      {
        Data d = cache[col_num];
        uint64_t const type = columns_[col_num].type_;

        if (is_ref_counted(type))
          Operation<OP_DEC_REF_DATA>::farray[type](d);
      }

      storage_->remove(row_num);
      --rows_;

      return true;
    }

    //--------------------------------------------------------------------------
    bool SharedTable::DeleteRow(const std::set<size_t> & NKIT_UNUSED(row_set))
    {
      // TODO Impl this
      assert(false);
      return false;
    }

    //--------------------------------------------------------------------------
    bool SharedTable::SetRow(
      const size_t row_num, const DynamicVector & vargs)
    {
      if (row_num >= height() || !Validate(vargs))
        return false;

      Data * cache = storage_->get(row_num);

      // remove old key in all indexes
      TableIndexSet::const_iterator index = table_index_set_.begin(),
        index_last = table_index_set_.end();
      for (; index != index_last; ++index)
        (*index)->NotifyRowDelete(cache, row_num, false);

      size_t const col_size = columns_.size(), args_size = vargs.size();
      for (size_t col_num = 0; col_num < col_size; ++col_num)
      {
        uint64_t col_type = columns_[col_num].type_;

        if (col_num < args_size)
        {
          uint64_t arg_type = vargs[col_num].type_;
          if (unlikely(col_type != arg_type))
            return false;

          // remove old value
          Data d = cache[col_num];
          if (is_ref_counted(col_type))
            Operation<OP_DEC_REF_DATA>::farray[col_type](d);

          // add new value
          d = vargs[col_num].data_;
          if (is_ref_counted(arg_type))
              Operation<OP_INC_REF_DATA>::farray[arg_type](d);
          cache[col_num] = d;
        }
        else
        {
          cache[col_num] = GetDefault(col_type);
        }
      }

      // insert new key to all indexes
      index = table_index_set_.begin(), index_last = table_index_set_.end();
      for (; index != index_last; ++index)
        (*index)->NotifyRowInsert(cache, row_num, false);

      return true;
    }

    //--------------------------------------------------------------------------
    bool SharedTable::InsertRow(const size_t row_num,
        const DynamicVector & vargs)
    {
      if (row_num >= height() || !Validate(vargs))
        return false;

      const size_t col_size = columns_.size(), args_size = vargs.size();
      // TODO replace  from new/delete to stack allocation
      //Data * pre_cache = new Data[col_size];
      Data * pre_cache = (Data *)alloca(col_size*sizeof(Data));
      for (size_t col_num = 0; col_num < col_size; ++col_num)
      {
        if (col_num < args_size)
        {
          const uint64_t type = vargs[col_num].type_;
          Data d = vargs[col_num].data_;
          if (is_ref_counted(type))
              Operation<OP_INC_REF_DATA>::farray[type](d);
          pre_cache[col_num] = d;
        }
        else
        {
          pre_cache[col_num] = GetDefault(columns_[col_num].type_);
        }
      }

      storage_->insert(row_num, pre_cache);

      TableIndexSet::const_iterator index = table_index_set_.begin(),
        index_last = table_index_set_.end();
      for (; index != index_last; ++index)
        (*index)->NotifyRowInsert(pre_cache, row_num, true);
      ++rows_;
      //delete [] pre_cache;
      return true;
    }

    //--------------------------------------------------------------------------
    bool SharedTable::SetCellValue(const size_t row_num, const size_t col_num,
        const Dynamic & v)
    {
      size_t const col_size = columns_.size();
      if (row_num >= height() && col_num >= col_size)
        return false;

      const int64_t type = columns_[col_num].type_;
      if (type != v.type_)
        return false;

      Data * cache = storage_->get(row_num);

      TableIndexSet::const_iterator index = table_index_set_.begin(),
        index_last = table_index_set_.end();
      for (; index != index_last; ++index)
      {
        if ((*index)->IsIndexedColumn(col_num))
          (*index)->NotifyRowDelete(cache, row_num, false);
      }

      Data d = cache[col_num];

      if (is_ref_counted(type))
        Operation<OP_DEC_REF_DATA>::farray[type](d);
      d = v.data_;
      if (is_ref_counted(v.type_))
        Operation<OP_INC_REF_DATA>::farray[type](d);
      cache[col_num] = d;

      index = table_index_set_.begin(), index_last = table_index_set_.end();
      for (; index != index_last; ++index)
      {
        if ((*index)->IsIndexedColumn(col_num))
          (*index)->NotifyRowInsert(cache, row_num, false);
      }

      return true;
    }

    //--------------------------------------------------------------------------
    void SharedTable::Subscribe(TableIndex::Ptr table_index)
    {
      assert(table_index);
      table_index_set_.insert(table_index);
      table_index->NotifyReferToTable(true);
    }

    //--------------------------------------------------------------------------
    void SharedTable::UnSubscribe(TableIndex::Ptr table_index)
    {
      assert(table_index);
      TableIndexSet::iterator pos = table_index_set_.find(table_index);
      if (pos != table_index_set_.end())
      {
        (*pos)->NotifyReferToTable(false);
        table_index_set_.erase(pos);
      }
    }

    //--------------------------------------------------------------------------
    void SharedTable::UnSubscribeAll()
    {
      TableIndexSet::iterator index = table_index_set_.begin(),
        last = table_index_set_.end();
      for(; index != last; ++index)
        (*index)->NotifyReferToTable(false);

      table_index_set_.clear();
    }

    //--------------------------------------------------------------------------
    bool SharedTable::Validate(const DynamicVector & vargs) const
    {
      const size_t col_size = columns_.size();
      const size_t args_size = vargs.size();

      if (!args_size)
        return false;

      const size_t size = std::min(col_size, args_size);
      for (size_t col = 0; col < size; ++col)
      {
        if (columns_[col].type_ != vargs[col].type_)
          return false;
      }

      return true;
    }

    bool SharedTable::Validate(const DynamicVector & vargs, DataVector * data,
        DynamicTypeVector * types) const
    {
      const size_t col_size = columns_.size();
      const size_t args_size = vargs.size();

      if (!args_size)
        return false;

      const size_t size = std::min(col_size, args_size);
      for (size_t col = 0; col < size; ++col)
      {
        if (columns_[col].type_ != vargs[col].type_)
          return false;

        (*data)[col] = vargs[col].data_;
        (*types)[col] = vargs[col].type_;
      }

      return true;
    }

    //--------------------------------------------------------------------------
    Data SharedTable::GetDefault(const uint64_t type) const
    {
      Dynamic d = Dynamic::GetDefault(type);
      if (is_ref_counted(type))
        Operation<OP_INC_REF_DATA>::farray[type](d.data_);
      return d.data_;
    }

    //--------------------------------------------------------------------------
    void SharedTable::AppendRowUnsafe(const DataVector & vargs)
    {
      Data * cache = storage_->extend();

      size_t const col_size = columns_.size(), args_size = vargs.size();
      for (size_t col_num = 0; col_num < col_size; ++col_num)
      {
        const uint64_t type = columns_[col_num].type_;
        if (col_num < args_size)
        {
          Data d = vargs[col_num];
          if (is_ref_counted(type))
            Operation<OP_INC_REF_DATA>::farray[type](d);
          cache[col_num] = d;
        }
        else
        {
          cache[col_num] = GetDefault(type);
        }
      }
      ++rows_;
    }

    //--------------------------------------------------------------------------
    void SharedTable::SetRowUnsafe(const DataVector & vargs, size_t r)
    {
      Data * cache = storage_->get(r);

      size_t const col_size = columns_.size();
      for (size_t col_num = 0; col_num < col_size; ++col_num)
        cache[col_num] = vargs[col_num];
    }
  } // namespace detail

  //----------------------------------------------------------------------------
  TableIndex::Ptr TableIndex::Create(
    detail::SharedTable * shared_table,
    const std::string & index_definition,
    std::string * error)
  {
    if (index_definition.empty())
    {
      *error = "Index definition could not be empty";
      return Ptr();
    }

    StringVector mask;
    SizeVector col_set;

    StringVector column_names;
    simple_split(index_definition, ",", &column_names);
    StringVector::const_iterator _column_name = column_names.begin(),
        column_name_end = column_names.end();
    for (; _column_name != column_name_end; ++_column_name)
    {
      std::string column_name(*_column_name);
      bool minus = false;
      if (column_name[0] == '-')
      {
        column_name.erase(0, 1);
        minus = true;
      }
      size_t col_num = shared_table->column_number(column_name);
      if (col_num == Dynamic::npos)
      {
        *error = "Could not find column with name '" + column_name + "'";
        return Ptr();
      }

      int64_t type = shared_table->get_column(col_num).type_;
      mask.push_back(string_cast(minus ? -type : type));
      col_set.push_back(col_num);
    }

    detail::IndexCompare comp;
    if (!GetComparator(mask, &comp, error))
    {
      return Ptr();
    }

    return Ptr(new TableIndex(shared_table, col_set, comp));
  }

  //----------------------------------------------------------------------------
  TableIndex::TableIndex(detail::SharedTable * shared_table,
      const SizeVector & column_nums, const detail::IndexCompare & cmp)
    : shared_table_(shared_table)
    , column_nums_(column_nums)
    , index_map_(cmp)
    , refer_to_table_(false)
  {
    BuildIndex();
  }

  //----------------------------------------------------------------------------
  TableIndex::~TableIndex()
  {
  }

  //----------------------------------------------------------------------------
  void TableIndex::MakeKey(detail::IndexKey & index_key, detail::Data * data)
  {
    detail::KeyItem key;
    SizeVector::const_iterator col = column_nums_.begin(),
          last = column_nums_.end();
    for (;col != last; ++col)
    {
      key = make_key_item(data[*col], shared_table_->get_column(*col).type_);
      index_key.key_[index_key.size_] = key;
      ++index_key.size_;
    }
  }

  //----------------------------------------------------------------------------
  bool TableIndex::IndexKeyFrom(detail::IndexKey & index_key,
      const DynamicVector & vargs)
  {
    if (vargs.size() < column_nums_.size()
        || vargs.size() > detail::IndexKey::MAX_SIZE)
      return false;

    DynamicVector::const_iterator it = vargs.begin(), last = vargs.end();
    for (; it != last; ++it)
    {
      // FIXME FIXME conversion!
      index_key.key_[index_key.size_] = make_key_item(it->data_, it->type_);
      ++index_key.size_;
    }

    return true;
  }

  //----------------------------------------------------------------------------
  void TableIndex::NotifyRowDelete(detail::Data * data, const size_t row_num,
      const bool incremental)
  {
    detail::IndexKey index_key(0);
    MakeKey(index_key, data);

    IndexMap::iterator row_pos = index_map_.find(index_key);
    assert(row_pos != index_map_.end());

    SizeVector::iterator item =
        std::find(row_pos->second.begin(), row_pos->second.end(), row_num);
    assert(item != row_pos->second.end());

    row_pos->second.erase(item);
    if (row_pos->second.size() == 0)
      index_map_.erase(row_pos);

    if (incremental)
    {
      IndexMap::iterator index = index_map_.begin(),
        last = index_map_.end();
      for (; index != last; ++index)
      {
        SizeVector::iterator
          reference = std::lower_bound(
            index->second.begin(), index->second.end(), row_num),
          end = index->second.end();
        for (; reference != end; ++reference)
          --(*reference);
      } // for
    } // if
  }

  //----------------------------------------------------------------------------
  void TableIndex::NotifyRowInsert(
    detail::Data * data, const size_t row_num, const bool incremental)
  {
    detail::IndexKey index_key(0);
    MakeKey(index_key, data);

    SizeVector & row = index_map_[index_key];
    if (incremental)
    {
      IndexMap::iterator index = index_map_.begin(),
        last = index_map_.end();
      for (; index != last; ++index)
      {
        SizeVector::iterator reference = std::lower_bound(
            index->second.begin(), index->second.end(), row_num), end =
            index->second.end();
        for (; reference != end; ++reference)
          ++(*reference);
      } // for
    }

    row.push_back(row_num);
    if (incremental)
      std::stable_sort(row.begin(), row.end());
  }

  //----------------------------------------------------------------------------
  void TableIndex::NotifyReferToTable(bool is)
  {
    refer_to_table_ = is;
  }

  //----------------------------------------------------------------------------
  bool TableIndex::IsIndexedColumn(const size_t col_num) const
  {
    return std::find(column_nums_.begin(), column_nums_.end(), col_num)
      != column_nums_.end();
  }

  //----------------------------------------------------------------------------
  void TableIndex::BuildIndex()
  {
    detail::IndexKey index_key(0);
    size_t const rows = shared_table_->height();
    for (size_t row = 0; row < rows; ++row)
    {
      MakeKey(index_key, shared_table_->storage_->get(row));
      index_map_[index_key].push_back(row);
      index_key.size_ = 0;
    }
  }

  //----------------------------------------------------------------------------
  Dynamic TableIndex::ConstIterator::operator[] (const size_t col_num)
  {
    if (table_index_ == NULL || row_it_ == row_it_end_)
      return D_NONE;
    return table_index_->shared_table_->GetCellValue(*row_it_, col_num);
  }

  //----------------------------------------------------------------------------
  TableIndex::ConstIterator TableIndex::GetLower(const DynamicVector & than)
  {
    if (refer_to_table_)
    {
      detail::IndexKey index_key(0);

      if (!IndexKeyFrom(index_key, than))
        return end();

      IndexMap::const_iterator pos = index_map_.lower_bound(index_key),
          end = index_map_.end();
      if (pos != end)
        return ConstIterator(pos, end, this);
    }
    return end();
  }

  //----------------------------------------------------------------------------
  TableIndex::ConstIterator TableIndex::GetLower(const Dynamic & a1,
      const Dynamic & a2)
  {
    DynamicVector vargs(2);
    vargs[0] = a1;
    vargs[1] = a2;
    return GetLower(vargs);
  }

  //----------------------------------------------------------------------------
  TableIndex::ConstIterator TableIndex::GetGrater(const DynamicVector & than)
  {
    if (refer_to_table_)
    {
      detail::IndexKey index_key(0);

      if (!IndexKeyFrom(index_key, than))
        return end();

      IndexMap::const_iterator pos = index_map_.upper_bound(index_key),
          end = index_map_.end();
      if (pos != end)
        return ConstIterator(pos, end, this);
    }
    return end();
  }

  //----------------------------------------------------------------------------
  TableIndex::ConstIterator TableIndex::GetLowerOrEqual(
      const DynamicVector & than)
  {
    ConstIterator pos(GetEqual(than));
    if (pos != end())
      return pos;
    return GetLower(than);
  }

  //----------------------------------------------------------------------------
  TableIndex::ConstIterator TableIndex::GetGraterOrEqual(
      const DynamicVector & than)
  {
    ConstIterator pos(GetEqual(than));
    if (pos != end())
      return pos;
    return GetGrater(than);
  }

  //----------------------------------------------------------------------------
  TableIndex::ConstIterator TableIndex::GetEqual(const DynamicVector & with)
  {
    if (refer_to_table_)
    {
      detail::IndexKey index_key(0);
      if (!IndexKeyFrom(index_key, with))
        return end();

      IndexMap::const_iterator pos = index_map_.find(index_key),
          end = index_map_.end();
      if (pos != end)
        return ConstIterator(pos, end, this);
    }
    return end();
  }

  //----------------------------------------------------------------------------
  TableIndex::ConstIterator TableIndex::GetEqual(const Dynamic & a1,
      const Dynamic & a2)
  {
    DynamicVector vargs(2);
    vargs[0] = a1;
    vargs[1] = a2;
    return GetEqual(vargs);
  }

  //----------------------------------------------------------------------------
  TableIndex::ConstIterator TableIndex::GetEqual(const Dynamic & a1,
      const Dynamic & a2, const Dynamic & a3)
  {
    DynamicVector vargs(3);
    vargs[0] = a1;
    vargs[1] = a2;
    vargs[2] = a3;
    return GetEqual(vargs);
  }
} // namespace nkit

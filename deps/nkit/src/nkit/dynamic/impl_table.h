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

#include <nkit/dynamic/table_storage_impl.h>

#include <nkit/types.h>

#include "nkit/dynamic.h"

#if defined(NKIT_WINNT)
#  pragma warning(push)
/*
C4351 ref : http://msdn.microsoft.com/en-us/library/1ywe7hcy%28v=vs.110%29.aspx
  If the array's element type does not have a constructor, the elements of the array will be
  initialized with the corresponding zero representation for that type.
  C4351 means that you should inspect your code. If you want the compiler's previous behavior.
*/
#  pragma warning(disable : 4351)
#endif // NKIT_WINNT

namespace nkit
{
  namespace detail
  {
    //--------------------------------------------------------------------------
    enum AggregatorType
    {
      // NOTE : Do not change the order
      AT_COUNT = 0,
      AT_SUM,
      AT_MIN,
      AT_MAX,
      AT_MAX_VALUE
    }; // enum AggregatorType

    //--------------------------------------------------------------------------
    struct AggregatorFunction
    {
      const char * name_;
      AggregatorType type_;
    };

    AggregatorType StringToAggregatorType(const std::string & aggr_name);

    //--------------------------------------------------------------------------
    class Aggregator
    {
      typedef void (Aggregator::*Method)(Data *, const Data *);

    public:
      typedef detail::ref_count_ptr<Aggregator> Ptr;

      static Ptr Create(const size_t column_num, const uint64_t type,
          const AggregatorType aggregator_type)
      {
        return Ptr(new Aggregator(column_num, type, aggregator_type));
      }

      Aggregator()
        : column_num_(0)
        , type_(DYNAMIC_TYPES_COUNT)
        //, aggregator_type_(AT_MAX_VALUE)
        , method_(NULL)
      { }

      Aggregator(const size_t column_num, const uint64_t type,
          const AggregatorType aggregator_type)
        : column_num_(column_num)
        , type_(type)
        //, aggregator_type_(aggregator_type)
        , method_()
      {
        switch (aggregator_type)
        {
        case AT_SUM:
          method_ = &Aggregator::Sum;
          default_data_ = detail::GetDefaultData(type_);
          break;
        case AT_COUNT:
          method_ = &Aggregator::Count;
          default_data_ = detail::GetDefaultData(type_);
          break;
        case AT_MIN:
          method_ = &Aggregator::Min;
          default_data_ = detail::GetMaxData(type_);
          break;
        case AT_MAX:
          method_ = &Aggregator::Max;
          default_data_ = detail::GetMinData(type_);
          break;
        default :
          abort_with_core(__PRETTY_FUNCTION__);
          break;
        } // switch
      }

      Data default_data() const
      {
        return default_data_;
      }

      uint64_t GetType() const
      {
        return type_;
      }

      void Update(Data * out, const Data * row)
      {
        (this->*method_)(out, row);
      }

    private:
      void Sum(Data * out, const Data * row)
      {
        Operation<OP_ADD>::farray[type_][type_](*out, row[column_num_]);
      }

      void Min(Data * out, const Data * row)
      {
        *out = Operation<OP_MIN>::farray[type_][type_](*out, row[column_num_]);
      }

      void Max(Data * out, const Data * row)
      {
        *out = Operation<OP_MAX>::farray[type_][type_](*out, row[column_num_]);
      }

      void Count(Data * out, const Data * NKIT_UNUSED(row))
      {
        ++out->ui64_;
      }

    private:
      size_t column_num_;
      uint64_t type_;
      //AggregatorType aggregator_type_;
      Method method_;
      Data default_data_;
    }; // class Aggregator

    typedef std::vector<Aggregator::Ptr> AggregatorVector;

    //--------------------------------------------------------------------------
    class GroupIndex
    {
      friend class Impl<detail::TABLE>;
      friend class nkit::GroupedTableBuilder;
      //------------------------------------------------------------------------
      struct Aggregators
      {
        explicit Aggregators(const AggregatorVector & aggrs) :
            vfunc_(aggrs), size_(aggrs.size()) { }
        AggregatorVector vfunc_;
        const size_t size_;
      };

      //------------------------------------------------------------------------
      class Bucket
      {
      public:
        Bucket() {}

        Bucket(const Bucket & from) : results_(from.results_) {}

        Bucket & operator = (const Bucket & from)
        {
          results_ = from.results_;
          return *this;
        }

        Bucket(const Aggregators & aggregators)
          : results_(aggregators.size_)
        {
          size_t size = aggregators.size_;
          for (size_t i = 0; i < size; ++i)
            results_[i] = aggregators.vfunc_[i]->default_data();
        }

        Data & operator[](size_t i)
        {
          return results_[i];
        }

        const Data & operator[](size_t i) const
        {
          return results_[i];
        }

      private:
        DataVector results_;
      };

      //------------------------------------------------------------------------
      typedef std::map<IndexKey, Bucket, IndexCompare> IndexMap;

    public:
      typedef detail::ref_count_ptr<GroupIndex> Ptr;

    public:
      ~GroupIndex();

    private :
      static Ptr Create(const SharedTable * shared_table,
        const std::string & index_def, const std::string & aggr,
        std::string * error);

      bool UpdateIndex(const DataVector & data,
          const DynamicTypeVector & types);
      Dynamic Build(const SharedTable & source_table);
      Dynamic Build();

      GroupIndex(Dynamic grouped_table,
          const SizeVector & index_column_nums,
          const AggregatorVector & aggregators,
          const IndexCompare & cmp);

      bool UpdateIndex(const Data * row);
      bool UpdateIndex(const SharedTable & source_table, const Data * row);
      void MakeRow(IndexMap::const_iterator & it);

    private :
      const SizeVector index_column_nums_;
      size_t index_columns_count_;
      Aggregators aggregators_;
      size_t aggr_columns_count_;
      Dynamic grouped_table_;
      DataVector current_row_;
      IndexMap index_map_;
      detail::IndexKey index_key_;
    }; // class GroupIndex

    //--------------------------------------------------------------------------
    class SharedTable : public RefCounted
    {
      friend class nkit::TableIndex;
      friend class GroupIndex;

      struct Column
      {
        Column()
          : type_(detail::DYNAMIC_TYPES_COUNT)
        {}

        Column(const std::string & name, const uint64_t type)
          : type_(type), name_(name)
        {}

        int64_t type_;
        std::string name_;
      }; // struct Column

      // types
      typedef std::vector<Column> Columns;
      typedef std::set<TableIndex::Ptr> TableIndexSet;

    public:
      static SharedTable * Create(const std::string & table_def,
          std::string * error);
      static SharedTable * Create(const StringVector & table_def,
          std::string * error);

      static SharedTable * Get(Data & data)
      {
        return data.shared_table_;
      }

      static const SharedTable * Get(const Data & data)
      {
        return data.shared_table_;
      }

      SharedTable * Clone(bool full) const;
      ~SharedTable();
      void Clear();

      // properties
      StringVector column_names() const;
      StringVector column_types() const;
      void SetColumnName(size_t pos, const std::string & name);
      size_t column_number(const std::string & column_name) const;

      bool empty() const { return rows_ == 0; }
      size_t height() const { return rows_; }
      size_t width() const { return columns_.size(); }

      // Table management
      Dynamic GetCellValue(const size_t row_num,
        const size_t col_num) const;
      bool AppendRow(const DynamicVector & args);
      bool SetCellValue(const size_t row_num,
        const size_t col_num, const Dynamic & v);
      bool DeleteRow(const size_t row_num);
      bool DeleteRow(const std::set<size_t> & row_set);
      bool SetRow(const size_t row_num, const DynamicVector & vect);
      bool InsertRow(const size_t row_num, const DynamicVector & vect);

      // Notification management interface
      void Subscribe(TableIndex::Ptr table_index);
      void UnSubscribe(TableIndex::Ptr table_index);
      void UnSubscribeAll();

      TableIndexSet GetIndexSet() const { return table_index_set_; }
      void SetIndexSet(const TableIndexSet & table_index_set)
      {
        table_index_set_ = table_index_set;
      }

      bool Validate(const DynamicVector & vargs) const;

      bool Validate(const DynamicVector & vargs, DataVector * data,
          DynamicTypeVector * types) const;

    private:
      SharedTable(const SharedTable & );
      SharedTable & operator = (const SharedTable & );

      SharedTable(const Columns & columns);

      const Column & get_column(const size_t col_num) const
      {
        return columns_[col_num];
      }

      Data GetDefault(const uint64_t type) const;

      void AppendRowUnsafe(const DataVector & vargs);
      void SetRowUnsafe(const DataVector & vargs, size_t r);

      size_t rows_;
      Columns columns_;
      StorageImpl * storage_;
      TableIndexSet table_index_set_;
    };

    //--------------------------------------------------------------------------
    template <>
    class Impl<detail::TABLE> : public ImplDefault
    {
    public:
      static bool Create(Dynamic & v, const std::string & table_def,
          std::string * error)
      {
        SharedTable * table = SharedTable::Create(table_def, error);
        if (!table)
        {
          v.Reset();
          return false;
        }

        Reset(&v, table);
        return true;
      }

      static bool Create(Dynamic & v, const StringVector & table_def,
          std::string * error)
      {
        SharedTable * table = SharedTable::Create(table_def, error);
        if (!table)
        {
          v.Reset();
          return false;
        }

        Reset(&v, table);
        return true;
      }

      static void OP_CLEAR(Dynamic & v)
      {
        GetSharedPtr(v.data_)->Clear();
      }

      static void OP_DEC_REF_DYNAMIC(Dynamic & v)
      {
        detail::SharedTable * shared = GetSharedPtr(v.data_);
        if (shared->DecRef() == 0)
        {
          delete shared;
          v.Reset();
        }
      }

      static void OP_INC_REF_DATA(Data & data)
      {
        GetSharedPtr(data)->IncRef();
      }

      static void OP_DEC_REF_DATA(Data & data)
      {
        detail::SharedTable * shared = GetSharedPtr(data);
        if (shared->DecRef() == 0)
        {
          delete shared;
          data.i64_ = 0;
        }
      }

      static Dynamic OP_CLONE(const Data & v)
      {
        const SharedTable * table = GetSharedPtr(v);
        Dynamic result;
        Reset(&result, table->Clone(true));
        return result;
      }

      static bool OP_GET_BOOL(const Data & v)
      {
        return !OP_IS_EMPTY(v);
      }

      static bool OP_IS_EMPTY(const Data & v)
      {
        return GetSharedPtr(v)->empty();
      }

      static size_t OP_GET_SIZE(const Data & v)
      {
        return GetSharedPtr(v)->height();
      }

      template <typename T>
      static bool OP_EQ(const Dynamic & lv, const Dynamic & rv)
      {
        if (!rv.IsTable())
          return false;

        const SharedTable * ltbl = GetSharedPtr(lv.data_);
        const SharedTable * rtbl = GetSharedPtr(rv.data_);

        size_t h_size = ltbl->height(),
            w_size = ltbl->width();

        if (h_size != rtbl->height())
          return false;

        if (w_size != rtbl->width())
          return false;

        for (size_t row = 0; row < h_size; ++row)
        {
          for (size_t col = 0; col < w_size; ++col)
          {
            if (ltbl->GetCellValue(row, col) != rtbl->GetCellValue(row, col))
                return false;
          }
        }

        return true;
      }

      static StringVector GetColumnNames(const Dynamic & v)
      {
        return GetSharedPtr(v.data_)->column_names();
      }

      static StringVector GetColumnTypes(const Dynamic & v)
      {
        return GetSharedPtr(v.data_)->column_types();
      }

      static size_t GetColumnNumber(const Dynamic & v,
        const std::string & column_name)
      {
        return GetSharedPtr(v.data_)->column_number(column_name);
      }

      static void SetColumnName(Dynamic & v,
          size_t pos, const std::string & name)
      {
        return GetSharedPtr(v.data_)->SetColumnName(pos, name);
      }

      static size_t GetWidth(const Dynamic & v)
      {
        return GetSharedPtr(v.data_)->width();
      }

      static size_t GetHeight(const Dynamic & v)
      {
        return GetSharedPtr(v.data_)->height();
      }

      static Dynamic GetCellValue(const Dynamic & table,
          const size_t row_num, const size_t col_num)
      {
        return GetSharedPtr(table.data_)->GetCellValue(row_num, col_num);
      }

      static Dynamic::TableIterator begin_t(const Dynamic & table)
      {
        return Dynamic::TableIterator(GetSharedPtr(table.data_), 0);
      }

      static Dynamic::TableIterator end_t(const Dynamic & table)
      {
        return Dynamic::TableIterator(GetSharedPtr(table.data_),
            GetSharedPtr(table.data_)->height());
      }

      static bool AppendRow(Dynamic & table, const Dynamic & v1)
      {
        std::vector<Dynamic> args(1);
        args[0] = v1;
        return GetSharedPtr(table.data_)->AppendRow(args);
      }

      static bool AppendRow(Dynamic & table,
          const Dynamic & v1, const Dynamic & v2)
      {
        std::vector<Dynamic> args(2);
        args[0] = v1;
        args[1] = v2;
        return GetSharedPtr(table.data_)->AppendRow(args);
      }

      static bool AppendRow(Dynamic & table,
          const Dynamic & v1, const Dynamic & v2, const Dynamic & v3)
      {
        std::vector<Dynamic> args(3);
        args[0] = v1;
        args[1] = v2;
        args[2] = v3;
        return GetSharedPtr(table.data_)->AppendRow(args);
      }

      static bool AppendRow(Dynamic & table,
          const Dynamic & v1, const Dynamic & v2, const Dynamic & v3,
          const Dynamic & v4)
      {
        std::vector<Dynamic> args(4);
        args[0] = v1;
        args[1] = v2;
        args[2] = v3;
        args[3] = v4;
        return GetSharedPtr(table.data_)->AppendRow(args);
      }

      static bool AppendRow(Dynamic & table,
          const Dynamic & v1, const Dynamic & v2, const Dynamic & v3,
          const Dynamic & v4, const Dynamic & v5)
      {
        std::vector<Dynamic> args(5);
        args[0] = v1;
        args[1] = v2;
        args[2] = v3;
        args[3] = v4;
        args[4] = v5;
        return GetSharedPtr(table.data_)->AppendRow(args);
      }

      static bool AppendRow(Dynamic & table,
          const Dynamic & v1, const Dynamic & v2, const Dynamic & v3,
          const Dynamic & v4, const Dynamic & v5, const Dynamic & v6)
      {
        std::vector<Dynamic> args(6);
        args[0] = v1;
        args[1] = v2;
        args[2] = v3;
        args[3] = v4;
        args[4] = v5;
        args[5] = v6;
        return GetSharedPtr(table.data_)->AppendRow(args);
      }

     static bool AppendRow(Dynamic & table,
         const Dynamic & v1, const Dynamic & v2, const Dynamic & v3,
         const Dynamic & v4, const Dynamic & v5, const Dynamic & v6,
         const Dynamic & v7)
     {
       std::vector<Dynamic> args(7);
       args[0] = v1;
       args[1] = v2;
       args[2] = v3;
       args[3] = v4;
       args[4] = v5;
       args[5] = v6;
       args[6] = v7;
       return GetSharedPtr(table.data_)->AppendRow(args);
     }

      static bool AppendRow(Dynamic & table, const DynamicVector & args)
      {
        return GetSharedPtr(table.data_)->AppendRow(args);
      }

      static bool DeleteRow(Dynamic & table, const size_t row_num)
      {
        return GetSharedPtr(table.data_)->DeleteRow(row_num);
      }

      static bool DeleteRow(Dynamic & table, const std::set<size_t> & row_set)
      {
        return GetSharedPtr(table.data_)->DeleteRow(row_set);
      }

      static bool SetRow(Dynamic & table,
        const size_t row_num, const DynamicVector & vargs)
      {
        return GetSharedPtr(table.data_)->SetRow(row_num, vargs);
      }

      static bool InsertRow(Dynamic & table,
        const size_t row_num, const DynamicVector & vargs)
      {
        return GetSharedPtr(table.data_)->InsertRow(row_num, vargs);
      }

      static bool SetCellValue(Dynamic & table, const size_t row_num,
        const size_t col_num, const Dynamic & v)
      {
        return GetSharedPtr(table.data_)->SetCellValue(row_num, col_num, v);
      }

      static TableIndex::Ptr CreateIndex(Dynamic & table,
        const std::string & index_definition, std::string * error)
      {
        TableIndex::Ptr table_index =
          TableIndex::Create(GetSharedPtr(table.data_), index_definition,
              error);
        if (table_index)
            GetSharedPtr(table.data_)->Subscribe(table_index);
        return table_index;
      }

      static void DeleteIndex(Dynamic & table, TableIndex::Ptr index)
      {
        if (index)
          GetSharedPtr(table.data_)->UnSubscribe(index);
      }

      static void DeleteAllIndices(Dynamic & table)
      {
        GetSharedPtr(table.data_)->UnSubscribeAll();
      }

      static Dynamic Group(const Data & table,
        const std::string & definitions, const std::string & expr,
        std::string * error)
      {
        GroupIndex::Ptr index =
          GroupIndex::Create(GetSharedPtr(table), definitions, expr, error);
        if (!index)
          return D_NONE;

        return index->Build(*table.shared_table_);
      }

    private:
      static SharedTable * GetSharedPtr(Data & data)
      {
        return SharedTable::Get(data);
      }

      static const SharedTable * GetSharedPtr(const Data & data)
      {
        return SharedTable::Get(data);
      }

      static void Reset(Dynamic * v, detail::SharedTable * const tbl)
      {
        v->type_ = TABLE;
        v->data_.shared_table_ = tbl;
      }
    };

    //--------------------------------------------------------------------------
    class DynamicTableBuilder
    {
    public:
      DynamicTableBuilder(const std::string & def)
        : table_(Dynamic::Table(def, NULL))
        , width_(table_.width())
      {
        new_row_.reserve(width_);
      }

      DynamicTableBuilder(const StringVector & def)
        : table_(Dynamic::Table(def, NULL))
        , width_(table_.width())
      {
        new_row_.reserve(width_);
      }

      DynamicTableBuilder & operator << (const Dynamic & v)
      {
        new_row_.push_back(v);
        CheckAppend();
        return *this;
      }

      DynamicTableBuilder & operator << (const std::string & v)
      {
        new_row_.push_back(Dynamic(v));
        CheckAppend();
        return *this;
      }

      DynamicTableBuilder & operator << (const int64_t v)
      {
        new_row_.push_back(Dynamic(v));
        CheckAppend();
        return *this;
      }

      DynamicTableBuilder & operator << (const uint64_t v)
      {
        new_row_.push_back(Dynamic(v));
        CheckAppend();
        return *this;
      }

      template <typename T>
      DynamicTableBuilder & operator << (const T v)
      {
        new_row_.push_back(Dynamic(v));
        CheckAppend();
        return *this;
      }

      Dynamic table() const { return table_; }

    private:
      void CheckAppend()
      {
        if(new_row_.size() == width_)
        {
          table_.AppendRow(new_row_);
          new_row_.clear();
        }
      }

    private:
      Dynamic table_;
      size_t width_;
      DynamicVector new_row_;
    };
  } // namespace detail
} // namespace nkit

#define DTBL(def, x) (( nkit::detail::DynamicTableBuilder(def) << x ).table())

#if defined(NKIT_WINNT)
#  pragma warning(pop)
#endif // NKIT_WINNT



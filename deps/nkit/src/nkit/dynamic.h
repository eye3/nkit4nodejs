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

#ifndef __NKIT__DYNAMIC__H__
#define __NKIT__DYNAMIC__H__

#include <nkit/types.h>
#include <nkit/tools.h>
#include <nkit/constants.h>
#include <nkit/dynamic/table_limits.h>
#include <nkit/detail/ref_count_ptr.h>

#include <cassert>
#include <cstring>
#include <limits>

#if defined(max)
#  undef max
#endif

#if defined(min)
#  undef min
#endif

namespace nkit
{
  class Dynamic;
  typedef std::vector<Dynamic> DynamicVector;
  typedef std::map<std::string, Dynamic> StringDynamicMap;
  typedef std::map<Dynamic, Dynamic> DynamicMap;
  class GroupedTableBuilder;

  namespace detail
  {
    enum DynamicType
    {
      /***********************************************************
       * Copy-by-value types
       **********************************************************/
      UNDEF = 0,
      INTEGER,          // 1
      UNSIGNED_INTEGER, // 2
      FLOAT,            // 3
      BOOL,             // 4
      DATE_TIME,        // 5

      NONE,             // 6 !!! IT MUST BE LAST COPY-BY-VALUE TYPE !!!

      /***********************************************************
       * Reference-counted types
       **********************************************************/
      STRING,           // 7
      LIST,             // 8
      DICT,             // 9
      MONGODB_OID,      // 10
      TABLE,            // 11

      /***********************************************************
       * !!! End marker. IT MUST BE LAST IN THIS ENUM !!!
       **********************************************************/
      DYNAMIC_TYPES_COUNT
    };

    typedef std::vector<uint64_t> DynamicTypeVector;

    const std::string & dynamic_type_to_string(const uint64_t type);
    uint64_t string_to_dynamic_type(const std::string & str);

    inline bool is_ref_counted(const uint64_t type)
    {
      return type > NONE;
    }

    template <uint64_t>
    class Impl;
    class ImplDefault;
    class Aggregator;
    class GroupIndex;
    class SharedString;
    class SharedVector;
    class SharedMap;
    class SharedTable;

    NKIT_STATIC_ASSERT(sizeof(int64_t) == sizeof(uint64_t), PlatformError);
    NKIT_STATIC_ASSERT(sizeof(int64_t) >= sizeof(double), PlatformError);
    NKIT_STATIC_ASSERT(sizeof(int64_t) >= sizeof(SharedString*), PlatformError);
    NKIT_STATIC_ASSERT(sizeof(int64_t) >= sizeof(SharedVector*), PlatformError);
    NKIT_STATIC_ASSERT(sizeof(int64_t) >= sizeof(SharedMap*), PlatformError);
    NKIT_STATIC_ASSERT(sizeof(int64_t) >= sizeof(SharedTable*), PlatformError);

    union Data
    {
      int64_t  i64_;
      uint64_t ui64_;
      double  f_;
      SharedString * shared_string_;
      SharedVector * shared_vector_;
      SharedMap * shared_map_;
      SharedTable * shared_table_;
    };

    Data GetDefaultData(uint64_t type);
    Data GetMaxData(uint64_t type);
    Data GetMinData(uint64_t type);

    typedef std::vector<Data> DataVector;

    //--------------------------------------------------------------------------
    union KeyItem
    {
      int64_t i64_;
      uint64_t ui64_;
      double  f_;
      SharedString * shared_string_;
    };

    KeyItem make_key_item(const Data data, const uint64_t type);
    KeyItem make_key_item(const Dynamic & );

    //--------------------------------------------------------------------------
    struct IndexKey
    {
      static const uint16_t MAX_SIZE = NKIT_TABLE_INDEX_PART_SIZE * 21;
      IndexKey() : size_(0) { }
      explicit IndexKey(size_t size) : size_(size) { }
      IndexKey(const IndexKey & from)
        : size_(from.size_)
      {
        std::memcpy(key_, from.key_, sizeof(from.key_));
      }

      IndexKey & operator =(const IndexKey & from)
      {
        std::memcpy(key_, from.key_, sizeof(from.key_));
        size_ = from.size_;
        return *this;
      }

      KeyItem key_[MAX_SIZE];
      uint64_t size_;
    };

    //--------------------------------------------------------------------------
    typedef int64_t (*KeyCompare)(const KeyItem *, const KeyItem *);

    //--------------------------------------------------------------------------
    class IndexCompare
    {
      static const uint16_t MAX_SIZE =
          (IndexKey::MAX_SIZE / NKIT_TABLE_INDEX_PART_SIZE) + 1;

    public:
      IndexCompare()
      {
        std::memset(fcomp_, 0, sizeof(fcomp_));
      }

      bool AppendPartCompare(KeyCompare fcomp)
      {
        size_t i = 0;
        while(i < MAX_SIZE)
        {
          if (!fcomp_[i])
            break;
          i++;
        }

        if (i == MAX_SIZE)
          return false;
        fcomp_[i] = fcomp;
        return true;
      }

      bool operator ()(const IndexKey & k1, const IndexKey & k2) const
      {
        const KeyItem * pk1 = k1.key_;
        const KeyItem * pk2 = k2.key_;

        int64_t result = 0;

        for (size_t i = 0; i < MAX_SIZE && fcomp_[i]; ++i)
        {
          result = fcomp_[i](pk1, pk2);
          if ( result != 0)
            break;

          pk1 += NKIT_TABLE_INDEX_PART_SIZE;
          pk2 += NKIT_TABLE_INDEX_PART_SIZE;
        }

        return result < 0;
      }

    private:
      KeyCompare fcomp_[MAX_SIZE];
    };

    bool GetComparator(const StringVector & mask, IndexCompare * result,
        std::string * error);

  } // namespace detail

  //----------------------------------------------------------------------------
  class TableIndex
  {
  public:
    typedef detail::ref_count_ptr<TableIndex> Ptr;

    typedef std::map<detail::IndexKey, SizeVector, detail::IndexCompare>
      IndexMap;

    class ConstIterator
    {
    public:
      ConstIterator()
        : table_index_(NULL)
      {
        index_it_ = index_end_;
        row_it_ = row_it_end_;
      }

      ConstIterator(TableIndex::IndexMap::const_iterator index_it,
          TableIndex::IndexMap::const_iterator index_end,
          TableIndex * table_index)
        : index_it_(index_it)
        , index_end_(index_end)
        , table_index_(table_index)
      {
        if (table_index_ != NULL && index_it_ != index_end_)
        {
          row_it_ = index_it_->second.begin();
          row_it_end_ = index_it_->second.end();
        }
        else
        {
          row_it_ = row_it_end_;
          table_index_ = NULL; // 'end' marker
        }
      }

      ConstIterator & operator++ ()
      {
        if (row_it_ != row_it_end_)
          ++row_it_;

        if (row_it_ == row_it_end_)
        {
          if (index_it_ != index_end_)
          {
            ++index_it_;
            if (index_it_ != index_end_)
            {
              row_it_ = index_it_->second.begin();
              row_it_end_ = index_it_->second.end();
            }
            else
              table_index_ = NULL;
          }
          else
            table_index_ = NULL;
        }

        return *this;
      }

      ConstIterator operator++ (int)
      {
        ConstIterator temp = *this;
        ++(*this);
        return temp;
      }

      Dynamic operator[] (const size_t col_num);

      friend bool operator == (const ConstIterator & rhs,
          const ConstIterator & lhs);
      friend bool operator != (const ConstIterator & rhs,
          const ConstIterator & lhs);

    private:
      TableIndex::IndexMap::const_iterator index_it_;
      TableIndex::IndexMap::const_iterator index_end_;
      SizeVector::const_iterator row_it_;
      SizeVector::const_iterator row_it_end_;
      TableIndex * table_index_;
    }; // class ConstIterator

    static Ptr Create(detail::SharedTable * shared_table,
      const std::string & index_definition, std::string * error);

    // life time management
    TableIndex(detail::SharedTable * shared_table, const SizeVector & col_set,
        const detail::IndexCompare & cmp);
    ~TableIndex();

    // operations
    ConstIterator begin()
    {
      return ConstIterator(index_map_.begin(), index_map_.end(), this);
    }

    ConstIterator end()
    {
      return ConstIterator(index_map_.end(), index_map_.end(), this);
    }

    ConstIterator GetLower(const DynamicVector & than);
    ConstIterator GetLower(const Dynamic & a1, const Dynamic & a2);
    ConstIterator GetGrater(const DynamicVector & than);
    ConstIterator GetLowerOrEqual(const DynamicVector & than);
    ConstIterator GetGraterOrEqual(const DynamicVector & than);
    ConstIterator GetEqual(const DynamicVector & with);
    ConstIterator GetEqual(const Dynamic & a1, const Dynamic & a2);
    ConstIterator GetEqual(const Dynamic & a1, const Dynamic & a2,
        const Dynamic & a3);

    size_t size() const { return index_map_.size(); }

  private:
    // methods
    void BuildIndex();
    void MakeKey(detail::IndexKey & index_key, detail::Data * data);
    bool IndexKeyFrom(detail::IndexKey & index_key,
        const DynamicVector & vargs);

  public:
    // notification/communication interface with SharedTable
    void NotifyRowDelete(detail::Data * data, const size_t row_num,
        bool incremental);
    void NotifyRowInsert(detail::Data * data, const size_t row_num,
        bool incremental);
    void NotifyReferToTable(bool is);

    bool IsIndexedColumn(const size_t col_num) const;

  private:
    detail::SharedTable * shared_table_;
    SizeVector column_nums_;
    IndexMap index_map_;
    bool refer_to_table_; // TODO: may be remove this member ?
  }; // class TableIndex

  //----------------------------------------------------------------------------
  class Dynamic
  {
  //----------------------------------------------------------------------------
  private: // types & friends
    friend detail::KeyItem detail::make_key_item(const Dynamic & );
    template <uint64_t> friend class detail::Impl;
    friend class detail::ImplDefault;
    friend class detail::SharedString;
    friend class detail::SharedVector;
    friend class detail::SharedMap;
    friend class detail::SharedTable;
    friend class detail::Aggregator;
    friend class detail::GroupIndex;
    friend class TableIndex;
    friend class GroupedTableBuilder;

    //--------------------------------------------------------------------------
    struct NkitInitializer
    {
      NkitInitializer();
    };

    static NkitInitializer nkit_initializer_;

  //----------------------------------------------------------------------------
  public: // constants
    static const size_t npos = size_t(-1);

  //----------------------------------------------------------------------------
  public: // types
    struct MongodbOIDType { MongodbOIDType() {} };
    static MongodbOIDType MONGODB_OID_MARKER;
    struct NoneType { NoneType() {} };
    static NoneType NONE_MARKER;
    typedef DynamicVector::const_iterator ListConstIterator;
    typedef StringDynamicMap::const_iterator DictConstIterator;

  public: // methods
    //--------------------------------------------------------------------------
    // constructors

    // default is undefined type
    Dynamic();

    // signed integer
    explicit Dynamic(const int8_t v);
    explicit Dynamic(const int16_t v);
    explicit Dynamic(const int32_t v);
    explicit Dynamic(const int64_t v);
    explicit Dynamic(const uint8_t v);
    explicit Dynamic(const uint16_t v);
    explicit Dynamic(const uint32_t v);
    explicit Dynamic(const uint64_t v);

    static Dynamic UInt64(const uint64_t v);
    static Dynamic UInt64(const Dynamic & v);

    // BOOL
    explicit Dynamic(const bool v);

    // DOUBLE
    explicit Dynamic(const double v);

    // NONE
    explicit Dynamic(const NoneType &);
    static Dynamic None();

    // STRING
    explicit Dynamic(const std::string & v);
    explicit Dynamic(const char * v);
    Dynamic(const char * v, const size_t length);

    // DATE_TIME
    Dynamic(const uint64_t year, const uint64_t month, const uint64_t day,
        const uint64_t hh, const uint64_t mm, const uint64_t ss,
        const uint64_t microsec = 0);
    Dynamic(const std::string & year, const std::string & month,
        const std::string & day,const std::string & hh,
        const std::string & mm, const std::string & ss,
        const std::string & microsec = S_ZERO_);
    static Dynamic DateTimeLocal();
    static Dynamic DateTimeGmt();
    static Dynamic DateTimeFromTimestamp(const time_t timestamp);
    static Dynamic DateTimeFromTm(const struct tm & _tm);
    static Dynamic DateTimeFromISO8601(const std::string & date_time,
          std::string * const error);
    static Dynamic DateTimeFromDefault(const std::string & str,
        std::string * const error);
    static Dynamic DateTimeFromString(const std::string & str,
        const char * format);

    // create empty DICT
    static Dynamic Dict();

    // create DICT from map
    template <typename T>
    static Dynamic Dict(const std::map<std::string, T> & map)
    {
      Dynamic result(Dynamic::Dict());
      typename std::map<std::string, T>::const_iterator it = map.begin(),
          end = map.end();
      for (; it != end; ++it)
        result[it->first] = Dynamic(it->second);
      return result;
    }

    template <typename K, typename V>
    static Dynamic Dict(const typename std::map<K, V> & map)
    {
      Dynamic result(Dynamic::Dict());
      typename std::map<K, V>::const_iterator it = map.begin(),
          end = map.end();
      for (; it != end; ++it)
      {
        result[string_cast(it->first)] = Dynamic(it->second);
      }
      return result;
    }

    // create empty LIST
    static Dynamic List();

    // create list from a sequence
    template <typename T>
    static Dynamic List(const T & c)
    {
      Dynamic result(Dynamic::List());
      typename T::const_iterator str = c.begin(), end = c.end();
      for (; str != end; ++str)
        result.PushBack(Dynamic(*str));
      return result;
    }

    // Mongodb OID
    static Dynamic MongodbOID();
    static Dynamic MongodbOID(const std::string & oid);

    // copy
    Dynamic(const Dynamic & v);

    // assign
    Dynamic & operator =(const Dynamic & sample);

    Dynamic Clone() const;
    void Clear();

    int64_t type() const { return type_; }
    bool IsSameType(const Dynamic & v) const { return type_ == v.type_; }
    bool IsUndef() const { return type_ == detail::UNDEF; }
    bool IsNone() const {return type_ == detail::NONE;}
    bool IsBool() const { return type_ == detail::BOOL; }
    bool IsSignedInteger() const { return type_ == detail::INTEGER; }
    bool IsUnsignedInteger() const { return type_ == detail::UNSIGNED_INTEGER; }
    bool IsInteger() const { return IsSignedInteger() || IsUnsignedInteger(); }
    bool IsFloat() const { return type_ == detail::FLOAT; }
    bool IsNumber() const { return IsInteger() || IsFloat(); }
    bool IsString() const { return type_ == detail::STRING; }
    bool IsDateTime() const { return type_ == detail::DATE_TIME; }
    bool IsList() const { return type_ == detail::LIST; }
    bool IsDict() const { return type_ == detail::DICT; }
    bool IsTable() const { return type_ == detail::TABLE; }
    bool IsMongodbOID() const { return type_ == detail::MONGODB_OID; }

    void Swap(Dynamic & with)
    {
      uint64_t tmp;

      tmp = type_;
      type_ = with.type_;
      with.type_ = tmp;

      tmp = data_.ui64_;
      data_.ui64_ = with.data_.ui64_;
      with.data_.ui64_ = tmp;
    }

    static Dynamic GetDefault(int64_t type);

    ~Dynamic();

    // save methods
    template <typename T>
    inline bool SaveTo(std::set<T> * const out) const;

    template <typename T>
    inline bool SaveTo(std::vector<T> * const out) const;

    template <typename T>
    inline bool SaveTo(std::list<T> * const out) const;

    template <typename T>
    inline bool SaveTo(std::map<std::string, T> * const out) const;

    void SaveTo(std::string * const out) const
    {
      *out = IsString() ? GetConstString() : GetString();
    }

    void SaveTo(int8_t * const out) const
    {
      *out = static_cast<int8_t>(GetSignedInteger());
    }

    void SaveTo(int16_t * const out) const
    {
      *out = static_cast<int16_t>(GetSignedInteger());
    }

    void SaveTo(int32_t * const out) const
    {
      *out = static_cast<int32_t>(GetSignedInteger());
    }

    void SaveTo(int64_t * const out) const
    {
      *out = GetSignedInteger();
    }

    void SaveTo(uint8_t * const out) const
    {
      *out = static_cast<uint8_t>(GetUnsignedInteger());
    }

    void SaveTo(uint16_t * const out) const
    {
      *out = static_cast<uint16_t>(GetUnsignedInteger());
    }

    void SaveTo(uint32_t * const out) const
    {
      *out = static_cast<uint32_t>(GetUnsignedInteger());
    }

    void SaveTo(uint64_t * const out) const
    {
      *out = GetUnsignedInteger();
    }

    // in-place converters
    Dynamic & ConvertToDateTime();
    Dynamic & ConvertToUInt64();
    Dynamic & ConvertToMongodbOID();

    // getters
    int64_t  GetSignedInteger() const;
    uint64_t GetUnsignedInteger() const;
    double  GetFloat() const;
    std::string GetString(const char * format = "") const;
    const std::string & GetConstString() const;
    bool GetBoolean() const;

    // cast operators
    operator std::string() const { return GetString(); }
    operator const std::string & () const { return GetConstString(); }
    operator int64_t() const { return GetSignedInteger(); }
    operator uint64_t() const { return GetUnsignedInteger(); }
    operator double() const { return GetFloat(); }
    operator bool() const;

    // compare operators
    bool operator < (const Dynamic & v) const;
    bool operator == (const Dynamic & v) const;
    bool operator <= (const Dynamic & v) const;
    bool operator > (const Dynamic & v) const;
    bool operator >= (const Dynamic & v) const;
    bool operator != (const Dynamic & v) const;

    // arithmetic operators
    Dynamic   operator +(const Dynamic & v) const;
    Dynamic   operator -(const Dynamic & v) const;
    Dynamic   operator /(const Dynamic & v) const;
    Dynamic   operator *(const Dynamic & v) const;

    Dynamic & operator +=(const Dynamic & v);
    Dynamic & operator -=(const Dynamic & v);
    Dynamic & operator *=(const Dynamic & v);
    Dynamic & operator /=(const Dynamic & v);

    // STRING specific
    bool StartsWith(const std::string & text) const;
    bool EndsWith(const std::string & text) const;
    bool Replace(const std::string & what, const std::string & with);

    // DATE_TIME specific
    uint32_t year() const;
    uint32_t month() const;
    uint32_t day() const;
    uint32_t hours() const;
    uint32_t minutes() const;
    uint32_t seconds() const;
    uint32_t microseconds() const;
    int64_t timestamp() const;
    bool leap() const;
    void AddDays(const int32_t dd);
    void AddHours(const int32_t hh);
    void AddMinutes(const int32_t mm);
    void AddSeconds(const int32_t ss);
    bool SetHour(const uint32_t hh);
    bool SetMinute(const uint32_t mm);
    bool SetSecond(const uint32_t ss);

    // LIST specific
    const Dynamic & operator[](const size_t pos) const;
    Dynamic & operator[](const size_t pos);
    const Dynamic & GetByIndex(const size_t pos) const;
    Dynamic & GetByIndex(const size_t pos);
    bool GetByIndex(const size_t pos, const Dynamic ** out) const;
    bool GetByIndex(const size_t pos, Dynamic ** out);
    size_t IndexOf(const Dynamic & value) const;
    size_t IIndexOf(const std::string & value) const;
    void PushBack(const Dynamic & item);
    void PopBack();
    void PushFront(const Dynamic & item);
    void PopFront();
    void Erase(const size_t & pos);
    void Erase(const size_t & from, const size_t & to);
    void Erase(const SizeSet & pos_set);
    const Dynamic & front() const;
    Dynamic & front();
    const Dynamic & back() const;
    Dynamic & back();
    Dynamic & Extend(const Dynamic & v);

    template <typename T>
    void Join(const std::string & delimiter, const std::string & prefix,
        const std::string & postfix, T & formatter,
        std::string * const out) const;

    void Join(const std::string & delimiter, const std::string & prefix,
        const std::string & postfix, const std::string & format,
        std::string * const out) const;

    void Join(const std::string & delimiter, const std::string & prefix,
        const std::string & postfix, const char * format,
        std::string * const out) const;

    void Join(const std::string & delimiter, const std::string & prefix,
        const std::string & postfix, std::string * const out) const;

    ListConstIterator begin_l() const;
    ListConstIterator end_l() const;

    // DICT specific
    Dynamic & operator[](const char * const key);
    Dynamic & operator[](const std::string & key);
    const Dynamic & operator[](const char * const key) const;
    const Dynamic & operator[](const std::string & key) const;
    Dynamic & Get(const char * const key);
    Dynamic & Get(const std::string & key);
    const Dynamic & Get(const char * const key) const;
    const Dynamic & Get(const std::string & key) const;
    bool Get(const char * key, Dynamic ** const value);
    bool Get(const std::string & key, Dynamic ** const value);
    bool Get(const char * key, const Dynamic ** const value) const;
    bool Get(const std::string & key, const Dynamic ** const value) const;
    void Erase(const char * const key);
    void Erase(const std::string & key);
    Dynamic & Update(const char * key, const Dynamic & value);
    Dynamic & Update(const std::string & key, const Dynamic & value);
    Dynamic & Update(const Dynamic & hash);
    void GetKeys(StringSet * const keys) const;

    DictConstIterator begin_d() const; // TODO: rename this methods & iterator
    DictConstIterator end_d() const;
    DictConstIterator FindByKey(const std::string & s) const;

    // TABLE specific
    class TableIterator
    {
    private:
      friend bool operator == (const TableIterator & rhs,
          const TableIterator & lhs);
      friend bool operator != (const TableIterator & rhs,
          const TableIterator & lhs);

    public:
      explicit TableIterator(const detail::SharedTable * shared_table)
        : shared_table_(shared_table)
        , row_it_(0)
      {}

      TableIterator(const detail::SharedTable * shared_table, size_t row_it)
        : shared_table_(shared_table)
        , row_it_(row_it)
      {}

      Dynamic operator[](const size_t col_num) const;

      TableIterator & operator++ ()
      {
        assert(shared_table_ != NULL);
        ++row_it_;
        return *this;
      }

      TableIterator operator++ (int)
      {
        assert(shared_table_ != NULL);
        TableIterator temp = *this;
        ++(*this);
        return temp;
      }

    private :
      const detail::SharedTable * shared_table_;
      size_t row_it_;
    };

    TableIterator begin_t() const;
    TableIterator end_t() const;

    static Dynamic Table(const std::string & table_def, std::string * error);
    static Dynamic Table(const StringVector & table_def, std::string * error);

    StringVector GetColumnNames() const;
    StringVector GetColumnTypes() const;
    size_t GetColumnNumber(const std::string & column_name) const;
    void SetColumnName(size_t pos, const std::string & name);

    // Columns count
    size_t width() const;

    // Rows count
    size_t height() const; // 'size_t size()' equivalent

    Dynamic GetCellValue(const size_t row_num, const size_t col_num) const;
    void JoinColumnCells(const size_t col, const std::string & delimiter,
        std::string * out) const;
    void SaveColumn(const size_t col, Dynamic * list) const;

    // Set methods. They update all existing indices
    bool AppendRow(const Dynamic & v);
    bool AppendRow(const Dynamic & v1, const Dynamic & v2);
    bool AppendRow(const Dynamic & v1, const Dynamic & v2, const Dynamic & v3);
    bool AppendRow(const Dynamic & v1, const Dynamic & v2, const Dynamic & v3,
        const Dynamic & v4);
    bool AppendRow(const Dynamic & v1, const Dynamic & v2, const Dynamic & v3,
        const Dynamic & v4, const Dynamic & v5);
    bool AppendRow(const Dynamic & v1, const Dynamic & v2, const Dynamic & v3,
        const Dynamic & v4, const Dynamic & v5, const Dynamic & v6);
    bool AppendRow(const Dynamic & v1, const Dynamic & v2, const Dynamic & v3,
        const Dynamic & v4, const Dynamic & v5, const Dynamic & v6,
        const Dynamic & v7);
    bool AppendRow(const DynamicVector & args);
    bool SetRow(const size_t row_num, const DynamicVector & vect);
    bool InsertRow(const size_t row_num, const DynamicVector & vect);
    bool DeleteRow(const size_t row_num);
    bool DeleteRow(const std::set<size_t> & row_set);
    bool SetCellValue(const size_t row_num, const size_t col_num, const Dynamic & v);

    /* 'index_definition' parameter is a string - comma delimited names of
     * columns, optionally with '-' prefix (that means reverse ordering).
     * E.g. - "name, -salary"
     * */
    TableIndex::Ptr CreateIndex(
      const std::string & index_definition, std::string * error);
    void DeleteIndex(TableIndex::Ptr index);
    void DeleteAllIndices();

    /*
     * 'aggr' parameter is a string - comma delimited aggregator functions
     * i.e. "SUM(column_name), MAX(column_name), MIN(column_name), COUNT"
     * */
    Dynamic Group(const std::string & index_def, const std::string & aggr,
      std::string * error);

    static detail::ref_count_ptr<GroupedTableBuilder> CreateGroupedTableBuilder(
        const std::string & table_def,
        const std::string & index_def,
        const std::string & aggr,
        std::string * error);

    // TODO: UnorderedGroup - Collapse
    // TODO: PartialIndex with predicate

    // STRING, LIST, DICT and TABLE specific
    size_t size() const;
    bool empty() const;

  private: // methods
    void FromData(uint64_t type, const detail::Data & data);

    void Reset()
    {
      type_ = detail::UNDEF;
      data_.ui64_ = 0;
    }

    void Reset(const Dynamic & v)
    {
      type_ = v.type_;
      data_.i64_ = v.data_.i64_;
    }

  private: // members
    int64_t type_;
    detail::Data data_;
  }; // class Dynamic

  //----------------------------------------------------------------------------
  inline void swap(Dynamic & one, Dynamic & another)
  {
    one.Swap(another);
  }

  //----------------------------------------------------------------------------
  inline void join(const Dynamic & container, const std::string & delimiter,
      const std::string & prefix, const std::string & postfix,
      std::string * const out)
  {
    container.Join(delimiter, prefix, postfix, out);
  }

  extern Dynamic D_NONE;

} // namespace nkit

#include "nkit/dynamic/impl.h"

namespace nkit
{
  /*----------------------------------------------------------------------------
   * Joins items of LIST, applying formatter to each item before join.
   * For example of formatter see <class CharEraser> and its usage in
   * test_dynamic.cpp: DynamicListJoinFormatter test case.
   * -------------------------------------------------------------------------*/
  template <typename T>
  void Dynamic::Join(const std::string & delimiter, const std::string & prefix,
      const std::string & postfix, T & formatter,
      std::string * const out) const
  {
    if (IsList())
      detail::Impl<detail::LIST>::Join<T>(*this, delimiter, prefix, postfix,
          formatter, out);
  }

  //----------------------------------------------------------------------------
  class CharEraser
  {
    typedef std::set<char> DelChars;

  public:
    CharEraser(const std::string & del_chars)
    {
      del_chars_.insert(del_chars.begin(), del_chars.end());
      not_found_ = del_chars_.end();
    }

    const std::string & operator ()(const Dynamic & d)
    {
      if (!d.IsString())
        return result_;

      const std::string & src = d.GetConstString();
      result_.reserve(src.size());
      result_.clear();
      std::string::const_iterator it = src.begin(), end = src.end();
      for (; it != end; ++it)
      {
        char ch(*it);
        if (del_chars_.find(ch) == not_found_)
          result_ += ch;
      }
      return result_;
    }

  private:
    DelChars del_chars_;
    DelChars::const_iterator not_found_;
    std::string result_;
  };

  //----------------------------------------------------------------------------
  class GroupedTableBuilder
  {
  public:
    typedef detail::ref_count_ptr<GroupedTableBuilder> Ptr;
    static Ptr Create(const std::string & table_def,
            const std::string & index_def,
            const std::string & aggr,
            std::string * error)
    {
      Dynamic validator_table = Dynamic::Table(table_def, error);
      if (!validator_table.IsTable())
        return Ptr();

      detail::GroupIndex::Ptr index = detail::GroupIndex::Create(
          validator_table.data_.shared_table_, index_def, aggr, error);
      return Ptr(new GroupedTableBuilder(validator_table, index));
    }

    bool InsertRow(const DynamicVector & vargs)
    {
      if (!validator_table_.Validate(vargs, &current_data_, &current_types_))
        return false;
      return index_->UpdateIndex(current_data_, current_types_);
    }

    Dynamic GetResult() const
    {
      return index_->Build();
    }

  private:
    GroupedTableBuilder(const Dynamic & validator_table,
        detail::GroupIndex::Ptr index)
      : validator_table_holder_(validator_table)
      , validator_table_(*validator_table_holder_.data_.shared_table_)
      , index_(index)
      , current_data_(validator_table.width())
      , current_types_(validator_table.width())
    {}

  private:
    Dynamic validator_table_holder_;
    const detail::SharedTable & validator_table_;
    detail::GroupIndex::Ptr index_;
    detail::DataVector current_data_;
    detail::DynamicTypeVector current_types_;
  };

  //----------------------------------------------------------------------------
  // Stream operators
  std::ostream & operator << (std::ostream & os, const Dynamic & v);
  std::ostream & operator << (std::ostream & os, const DynamicVector & v);

  // Stream manipulators
  std::ostream & json(std::ostream & ios);
  std::ostream & json_hr(std::ostream & ios);
  std::ostream & json_hr_table(std::ostream & ios);

  //----------------------------------------------------------------------------
  inline std::string operator + (const char * const str, const Dynamic & v)
  {
    return std::string(str + v.GetString());
  }

  inline std::string operator + (const Dynamic & v, const char * const str)
  {
    return std::string(v.GetString() + str);
  }

  inline bool operator == (const nkit::Dynamic::TableIterator & lhs,
      const nkit::Dynamic::TableIterator & rhs)
  {
    return rhs.row_it_ == lhs.row_it_;
  }

  inline bool operator != (const nkit::Dynamic::TableIterator & lhs,
      const nkit::Dynamic::TableIterator & rhs)
  {
    return rhs.row_it_ != lhs.row_it_;
  }

  inline bool operator == (const TableIndex::ConstIterator & lhs,
      const TableIndex::ConstIterator & rhs)
  {
    return (rhs.table_index_ == NULL && lhs.table_index_ == NULL)
        || (rhs.index_it_ == lhs.index_it_ && rhs.row_it_ == lhs.row_it_);
  }

  inline bool operator != (const TableIndex::ConstIterator & lhs,
      const TableIndex::ConstIterator & rhs)
  {
    return !operator == (lhs, rhs);
  }

  template <typename T>
  inline bool Dynamic::SaveTo(std::set<T> * const out) const
  {
    if (!IsList())
      return false;
    out->clear();
    DLIST_FOREACH(it, *this)
    {
      T tmp;
      it->SaveTo(&tmp);
      out->insert(tmp);
    }
    return true;
  }

  template <typename T>
  inline bool Dynamic::SaveTo(std::vector<T> * const out) const
  {
    if (!IsList())
      return false;
    out->clear();
    DLIST_FOREACH(it, *this)
    {
      T tmp;
      it->SaveTo(&tmp);
      out->push_back(tmp);
    }
    return true;
  }

  template <typename T>
  inline bool Dynamic::SaveTo(std::list<T> * const out) const
  {
    if (!IsList())
      return false;
    out->clear();
    DLIST_FOREACH(it, *this)
    {
      T tmp;
      it->SaveTo(&tmp);
      out->push_back(tmp);
    }
    return true;
  }

  template <typename T>
  inline bool Dynamic::SaveTo(std::map<std::string, T> * const out) const
  {
    if (!IsDict())
      return false;
    out->clear();
    DDICT_FOREACH(it, *this)
    {
      T tmp;
      SaveTo(&tmp);
      (*out)[it->first] = tmp;
    }
    return true;
  }

} // namespace nkit

namespace std
{
  template <>
  inline void swap<nkit::Dynamic>(nkit::Dynamic & one, nkit::Dynamic & another)
  {
    one.Swap(another);
  }
}

#endif  // __NKIT__DYNAMIC__H__

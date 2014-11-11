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

#include <algorithm>
#include <iostream>
#include <iomanip>

#include "nkit/dynamic.h"
#include "nkit/logger.h"
#include "nkit/tools.h"
#include "nkit/transcode.h"

namespace nkit
{
  Dynamic::MongodbOIDType Dynamic::MONGODB_OID_MARKER;
  Dynamic::NoneType Dynamic::NONE_MARKER;

  Dynamic D_NONE(Dynamic::NONE_MARKER);

  Dynamic::NkitInitializer Dynamic::nkit_initializer_;

  namespace detail
  {
    const DynamicVector ConstVectorAdapter::empty_list_;
    DynamicVector VectorAdapter::empty_list_;

    const StringDynamicMap ConstMapAdapter::empty_map_;
    StringDynamicMap MapAdapter::empty_map_;

    Data GetDefaultData(uint64_t type)
    {
      return Operation<OP_GET_DEFAULT_DATA>::farray[type]();
    }

    Data GetMaxData(uint64_t type)
    {
      return Operation<OP_GET_MAX_DATA>::farray[type]();
    }

    Data GetMinData(uint64_t type)
    {
      return Operation<OP_GET_MIN_DATA>::farray[type]();
    }

    //--------------------------------------------------------------------------
    template<uint64_t opcode, uint64_t current_type>
    struct unary_op_filler_t
    {
      static void run()
      {
        Operation<opcode>::farray[current_type] =
            Operation<opcode>::template Run<current_type>;
        unary_op_filler_t<opcode, current_type-1>::run();
      }
    };

    template<uint64_t opcode>
    struct unary_op_filler_t<opcode, 0>
    {
      static void run()
      {
        Operation<opcode>::farray[0] = Operation<opcode>::template Run<0>;
      }
    };

    template <uint64_t opcode>
    struct unary_op_filler
    {
      static void run()
      {
        unary_op_filler_t<opcode, detail::DYNAMIC_TYPES_COUNT-1>::run();
      }
    };

    template<uint64_t opcode, uint64_t ltype, uint64_t rtype>
    struct binary_op_filler_lr
    {
      static void run()
      {
        Operation<opcode>::farray[ltype][rtype] =
            Operation<opcode>::template Run<ltype, rtype>;
        binary_op_filler_lr<opcode, ltype, rtype-1>::run();
      }
    };

    template<uint64_t opcode, uint64_t ltype>
    struct binary_op_filler_lr<opcode, ltype, 0>
    {
      static void run()
      {
        Operation<opcode>::farray[ltype][0] =
            Operation<opcode>::template Run<ltype, 0>;
      }
    };

    template<uint64_t opcode, uint64_t ltype>
    struct binary_op_filler_l
    {
      static void run()
      {
        binary_op_filler_lr<opcode, ltype, detail::DYNAMIC_TYPES_COUNT-1>::run();
        binary_op_filler_l<opcode, ltype-1>::run();
      }
    };

    template<uint64_t opcode>
    struct binary_op_filler_l<opcode, 0>
    {
      static void run()
      {
        binary_op_filler_lr<opcode, 0, detail::DYNAMIC_TYPES_COUNT-1>::run();
      }
    };

    template<uint64_t opcode>
    struct binary_op_filler
    {
      static void run()
      {
        binary_op_filler_l<opcode, detail::DYNAMIC_TYPES_COUNT-1>::run();
      }
    };

    typedef void (*OperationArrayFiller)(void);

    template <uint64_t opcode>
    struct fillers_filler
    {
      static void fill(OperationArrayFiller fillers[OPERATIONS_COUNT])
      {
        fillers[opcode] = unary_op_filler<opcode>::run;
        fillers_filler<opcode-1>::fill(fillers);
      }
    };

    template <uint64_t opcode>
    struct binary_fillers_filler
    {
      static void fill(OperationArrayFiller fillers[OPERATIONS_COUNT])
      {
        fillers[opcode] = binary_op_filler<opcode>::run;
        binary_fillers_filler<opcode-1>::fill(fillers);
      }
    };

    template <>
    struct binary_fillers_filler<0>
    {
      static void fill(OperationArrayFiller fillers[OPERATIONS_COUNT])
      {
        fillers[0] = binary_op_filler<0>::run;
      }
    };

    template <>
    struct fillers_filler<BINARY_UNARY_DELIMITER>
    {
      static void fill(OperationArrayFiller fillers[OPERATIONS_COUNT])
      {
        binary_fillers_filler<BINARY_UNARY_DELIMITER-1>::fill(fillers);
      }
    };

    void InitComparators();
    void FillOperations()
    {
      OperationArrayFiller fillers[OPERATIONS_COUNT];
      fillers_filler<OPERATIONS_COUNT-1>::fill(fillers);
      for (uint64_t i = 0; i < OPERATIONS_COUNT; ++i)
        if (i != BINARY_UNARY_DELIMITER)
          fillers[i](); // invokes filler for concrete operation buffer
      InitComparators();
    }

    //--------------------------------------------------------------------------

  } // namespace detail
} // namespace nkit

//------------------------------------------------------------------------------
namespace nkit
{
  Dynamic::NkitInitializer::NkitInitializer()
  {
    detail::FillOperations();
    NKIT_FORCE_USED(timezone_offset());
    Transcoder::Build();
  }

  // copy constructor
  Dynamic::Dynamic(const Dynamic & from)
  {
    if (from.type_ >= detail::DYNAMIC_TYPES_COUNT)
    {
      NKIT_LOG_ERROR(__PRETTY_FUNCTION__ <<
          "(): wrong type id: " << from.type_);
      Reset();
    }
    else
    {
      Reset(from);
      if (detail::is_ref_counted(type_))
        detail::Operation<detail::OP_INC_REF_DATA>::farray[type_](data_);
    }
  }

  // assignment
  Dynamic& Dynamic::operator =(const Dynamic & from)
  {
    if (this != &from)
    {
      if (detail::is_ref_counted(type_))
      {
        if (from.type_ >= detail::DYNAMIC_TYPES_COUNT)
        {
          NKIT_LOG_ERROR(__PRETTY_FUNCTION__ <<
              "(): wrong type id: " << from.type_);
          detail::Operation<detail::OP_DEC_REF_DYNAMIC>::farray[type_](*this);
          Reset();
        }
        else
        {
          Dynamic saved_copy(from);

          detail::Operation<detail::OP_DEC_REF_DYNAMIC>::farray[type_](*this);

          Reset(from);

          if (detail::is_ref_counted(type_))
            detail::Operation<detail::OP_INC_REF_DATA>::farray[type_](data_);
        }
      }
      else
      {
        if (!IsNone())
          Reset(from);
        if (detail::is_ref_counted(type_))
          detail::Operation<detail::OP_INC_REF_DATA>::farray[type_](data_);
      }
    }

    return *this;
  }

  void Dynamic::FromData(uint64_t type, const detail::Data & data)
  {
    type_ = type;
    data_ = data;
    if (detail::is_ref_counted(type_))
        detail::Operation<detail::OP_INC_REF_DATA>::farray[type_](data_);
  }

  Dynamic Dynamic::Clone() const
  {
    return detail::Operation<detail::OP_CLONE>::farray[type_](data_);
  }

  Dynamic Dynamic::GetDefault(int64_t type)
  {
    switch (type)
    {
    case detail::INTEGER:
      return Dynamic(int64_t(0));
    case detail::FLOAT:
      return Dynamic(0.0);
    case detail::BOOL:
      return Dynamic(false);
    case detail::DATE_TIME:
      return DateTimeLocal();
    case detail::NONE:
      return D_NONE;
    case detail::STRING:
      return Dynamic("");
    case detail::MONGODB_OID:
      return MongodbOID();
    case detail::LIST:
      return List();
    case detail::DICT:
      return Dict();
    case detail::TABLE:
      return Table("unkown", NULL);
    default:
      return Dynamic();
    }
  }

  Dynamic::~Dynamic()
  {
    if (detail::is_ref_counted(type_))
      detail::Operation<detail::OP_DEC_REF_DYNAMIC>::farray[type_](*this);
  }

  Dynamic::Dynamic()
    : type_(detail::UNDEF)
  {
    detail::Impl<detail::UNDEF>::Create(*this);
  }

  Dynamic::Dynamic(const bool v)
    : type_(detail::BOOL)
  {
    detail::Impl<detail::BOOL>::Create(*this, v);
  }

  Dynamic::Dynamic(const int8_t v)
    : type_(detail::INTEGER)
  {
    detail::Impl<detail::INTEGER>::Create(*this, v);
  }

  Dynamic::Dynamic(const int16_t v)
    : type_(detail::INTEGER)
  {
    detail::Impl<detail::INTEGER>::Create(*this, v);
  }

  Dynamic::Dynamic(const int32_t v)
    : type_(detail::INTEGER)
  {
    detail::Impl<detail::INTEGER>::Create(*this, v);
  }

  Dynamic::Dynamic(const int64_t v)
    : type_(detail::INTEGER)
  {
    detail::Impl<detail::INTEGER>::Create(*this, v);
  }

  Dynamic::Dynamic(const uint8_t v)
    : type_(detail::INTEGER)
  {
    detail::Impl<detail::INTEGER>::Create(*this, v);
  }

  Dynamic::Dynamic(const uint16_t v)
    : type_(detail::INTEGER)
  {
    detail::Impl<detail::INTEGER>::Create(*this, v);
  }

  Dynamic::Dynamic(const uint32_t v)
    : type_(detail::INTEGER)
  {
    detail::Impl<detail::INTEGER>::Create(*this, v);
  }

  Dynamic::Dynamic(const uint64_t v)
    : type_(detail::INTEGER)
  {
    detail::Impl<detail::INTEGER>::Create(*this, v);
  }

  Dynamic Dynamic::UInt64(const uint64_t v)
  {
    Dynamic result;
    detail::Impl<detail::UNSIGNED_INTEGER>::Create(result, v);
    return result;
  }

  Dynamic Dynamic::UInt64(const Dynamic & v)
  {
    return Dynamic::UInt64(v.GetUnsignedInteger());
  }

  Dynamic::Dynamic(const double v)
    : type_(detail::FLOAT)
  {
    detail::Impl<detail::FLOAT>::Create(*this, v);
  }

  Dynamic::Dynamic(const std::string & v)
    : type_(detail::STRING)
  {
    detail::Impl<detail::STRING>::Create(*this, v);
  }

  Dynamic::Dynamic(const char * v)
    : type_(detail::STRING)
  {
    detail::Impl<detail::STRING>::Create(*this, v);
  }

  Dynamic::Dynamic(const char * v, const size_t length)
    : type_(detail::STRING)
  {
    detail::Impl<detail::STRING>::Create(*this, v, length);
  }

  Dynamic::Dynamic(const NoneType &)
    : type_(detail::NONE)
  {
    detail::Impl<detail::NONE>::Create(*this);
  }

  Dynamic Dynamic::None()
  {
    Dynamic result;
    detail::Impl<detail::NONE>::Create(result);
    return result;
  }

  Dynamic::Dynamic(const uint64_t year, const uint64_t mon,
      const uint64_t dd, const uint64_t hh,
      const uint64_t mm, const uint64_t ss, const uint64_t microsec)
    : type_(detail::DATE_TIME)
  {
    detail::Impl<detail::DATE_TIME>::Create(*this,
      year, mon, dd, hh, mm, ss, microsec);
  }

  Dynamic::Dynamic(const std::string & year, const std::string & month,
      const std::string & day, const std::string & hh, const std::string & mm,
      const std::string & ss, const std::string & microsec)
    : type_(detail::DATE_TIME)
  {
    detail::Impl<detail::DATE_TIME>::Create(*this, ::atoi(year.c_str()),
        ::atoi(month.c_str()), ::atoi(day.c_str()), ::atoi(hh.c_str()),
        ::atoi(mm.c_str()), ::atoi(ss.c_str()),
        ::atoi(microsec.c_str()));
  }

  Dynamic Dynamic::DateTimeLocal()
  {
    Dynamic result;
    detail::Impl<detail::DATE_TIME>::CreateLocal(result);
    return result;
  }

  Dynamic Dynamic::DateTimeGmt()
  {
    Dynamic result;
    detail::Impl<detail::DATE_TIME>::CreateGmt(result);
    return result;
  }

  Dynamic Dynamic::DateTimeFromTimestamp(const time_t timestamp)
  {
    Dynamic result;
    detail::Impl<detail::DATE_TIME>::Create(result, timestamp);
    return result;
  }

  Dynamic Dynamic::DateTimeFromTm(const struct tm & _tm)
  {
    uint32_t year = static_cast<uint32_t>(_tm.tm_year + 1900);
    uint32_t month = static_cast<uint32_t>(_tm.tm_mon + 1);
    uint32_t day = static_cast<uint32_t>(_tm.tm_mday);
    uint32_t hour = static_cast<uint32_t>(_tm.tm_hour);
    uint32_t min = static_cast<uint32_t>(_tm.tm_min);
    uint32_t sec = static_cast<uint32_t>(_tm.tm_sec);
    return Dynamic(year, month, day, hour, min, sec);
  }

  bool make_date_time_from_string(const std::string & str,
      const std::string & example, const char * const format,
      Dynamic * out, std::string * const error)
  {
    static const std::string WRONG_VALUES("Wrong date-time value: ");

    if (str.size() != example.size())
    {
      *error = WRONG_VALUES + str;
      return false;
    }

    uint32_t year, mon, dd, hh, mm, ss;
    if (NKIT_SSCANF(str.c_str(), format, &year, &mon, &dd, &hh, &mm, &ss) != 6)
    {
      *error = WRONG_VALUES + str;
      return false;
    }

    *out = Dynamic(year, mon, dd, hh, mm, ss, 0);
    if (!*out)
    {
      *error = WRONG_VALUES + str;
      return false;
    }
    return true;
  }

  Dynamic Dynamic::DateTimeFromDefault(const std::string & str,
      std::string * const error)
  {
    static const char * const FORMAT = "%4u-%02u-%02u %02u:%02u:%02u";
    static const std::string EXAMPLE("1998-07-17 14:08:55");
    Dynamic result;
    make_date_time_from_string(str, EXAMPLE, FORMAT, &result, error);
    return result;
  }

  Dynamic Dynamic::DateTimeFromISO8601(const std::string & str,
      std::string * const error)
  {
    static const char * const FORMAT1 = "%4u-%02u-%02uT%02u:%02u:%02u";
    static const std::string EXAMPLE1("1998-07-17T14:08:55");

    static const char * const FORMAT2 = "%4u%02u%02uT%02u%02u%02u";
    static const std::string EXAMPLE2("19980717T140855");

    Dynamic result;
    if (str[4] == '-')
      make_date_time_from_string(str, EXAMPLE1, FORMAT1, &result, error);
    else
      make_date_time_from_string(str, EXAMPLE2, FORMAT2, &result, error);
    return result;
  }

  Dynamic Dynamic::DateTimeFromString(const std::string & str,
      const char * format)
  {
#ifdef NKIT_WINNT
    struct tm _tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
    struct tm _tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif
    if (NKIT_STRPTIME(str.c_str(), format, &_tm) == NULL)
      return Dynamic();
    return DateTimeFromTm(_tm);
  }

  Dynamic Dynamic::List()
  {
    Dynamic result;
    detail::Impl<detail::LIST>::Create(result);
    return result;
  }

  // empty hash
  Dynamic Dynamic::Dict()
  {
    Dynamic result;
    detail::Impl<detail::DICT>::Create(result);
    return result;
  }

  Dynamic Dynamic::MongodbOID()
  {
    Dynamic result;
    detail::Impl<detail::MONGODB_OID>::Create(result);
    return result;
  }

  Dynamic Dynamic::MongodbOID(const std::string & oid)
  {
    Dynamic result;
    detail::Impl<detail::MONGODB_OID>::Create(result, oid);
    return result;
  }

  int64_t Dynamic::GetSignedInteger() const
  {
    return detail::Operation<detail::OP_GET_INT>::farray[type_](data_);
  }

  uint64_t Dynamic::GetUnsignedInteger() const
  {
    return detail::Operation<detail::OP_GET_UINT>::farray[type_](data_);
  }

  double Dynamic::GetFloat() const
  {
    return detail::Operation<detail::OP_GET_FLOAT>::farray[type_](data_);
  }

  std::string Dynamic::GetString(const char * format) const
  {
    return detail::Operation<detail::OP_GET_STRING>::farray[type_](
        data_, format);
  }

  const std::string & Dynamic::GetConstString() const
  {
    return detail::Operation<detail::OP_GET_CONST_STRING>::farray[type_](data_);
  }

  bool Dynamic::GetBoolean() const
  {
    return GetSignedInteger() != 0;
  }

  Dynamic & Dynamic::ConvertToDateTime()
  {
    if (IsInteger() || IsString())
      *this = Dynamic::DateTimeFromTimestamp(GetUnsignedInteger());
    else if (!IsDateTime())
      *this = Dynamic::DateTimeFromTimestamp(0);
    return *this;
  }

  Dynamic & Dynamic::ConvertToUInt64()
  {
    if (!IsUnsignedInteger())
      *this = Dynamic::UInt64(GetUnsignedInteger());
    return *this;
  }

  Dynamic & Dynamic::ConvertToMongodbOID()
  {
    if (IsInteger())
    {
      std::string tmp(GetString());
      size_t sz(tmp.size());
      if (sz < 24)
        tmp.append(24 - sz, 'f');
      *this = Dynamic::MongodbOID(tmp);
    }
    else
      *this = Dynamic::MongodbOID(std::string(24, 'f'));

    return *this;
  }

  Dynamic::operator bool() const
  {
    return detail::Operation<detail::OP_GET_BOOL>::farray[type_](data_);
  }

  bool Dynamic::operator <(const Dynamic & v) const
  {
    return detail::Operation<detail::OP_LT>::farray[type_][v.type_](*this, v);
  }

  bool Dynamic::operator ==(const Dynamic & v) const
  {
    return detail::Operation<detail::OP_EQ>::farray[type_][v.type_](*this, v);
  }

  bool Dynamic::operator !=(const Dynamic & v) const
  {
    return !operator ==(v);
  }

  bool Dynamic::operator <=(const Dynamic & v) const
  {
    return operator <(v) || operator ==(v);
  }

  bool Dynamic::operator >(const Dynamic & v) const
  {
    return !operator <=(v);
  }

  bool Dynamic::operator >=(const Dynamic & v) const
  {
    return !operator <(v);
  }

  Dynamic Dynamic::operator +(const Dynamic & v) const
  {
    Dynamic result(detail::is_ref_counted(type_) ? Clone() : *this);
    detail::Operation<detail::OP_ADD>::farray[type_][v.type_](
        result.data_, v.data_);
    return result;
  }

  Dynamic Dynamic::operator -(const Dynamic & v) const
  {
    Dynamic result(detail::is_ref_counted(type_) ? Clone() : *this);
    detail::Operation<detail::OP_SUB>::farray[type_][v.type_](
        result.data_, v.data_);
    return result;
  }

  Dynamic Dynamic::operator *(const Dynamic & v) const
  {
    Dynamic result(detail::is_ref_counted(type_) ? Clone() : *this);
    if (unlikely(!detail::Operation<detail::OP_MUL>::farray[type_][v.type_](
        result.data_, v.data_)))
      return Dynamic();
    return result;
  }

  Dynamic Dynamic::operator /(const Dynamic & v) const
  {
    Dynamic result(detail::is_ref_counted(type_) ? Clone() : *this);
    if (unlikely(!detail::Operation<detail::OP_DIV>::farray[type_][v.type_](
        result.data_, v.data_)))
      return Dynamic();
    return result;
  }

  Dynamic& Dynamic::operator +=(const Dynamic & v)
  {
    detail::Operation<detail::OP_ADD>::farray[type_][v.type_](
        data_, v.data_);
    return *this;
  }

  Dynamic& Dynamic::operator -=(const Dynamic & v)
  {
    detail::Operation<detail::OP_SUB>::farray[type_][v.type_](
        data_, v.data_);
    return *this;
  }

  Dynamic& Dynamic::operator *=(const Dynamic & v)
  {
    if (unlikely(!detail::Operation<detail::OP_MUL>::farray[type_][v.type_](
        data_, v.data_)))
      *this = Dynamic();
    return *this;
  }

  Dynamic& Dynamic::operator /=(const Dynamic & v)
  {
    if (unlikely(!detail::Operation<detail::OP_DIV>::farray[type_][v.type_](
        data_, v.data_)))
      *this = Dynamic();
    return *this;
  }

  void Dynamic::Clear()
  {
    detail::Operation<detail::OP_CLEAR>::farray[type_](*this);
  }

  size_t Dynamic::IndexOf(const Dynamic & v) const
  {
    if (IsList())
      return detail::Impl<detail::LIST>::GetIndexOf(*this, v);
    return npos;
  }

  size_t Dynamic::IIndexOf(const std::string & str) const
  {
    if (IsList())
      return detail::Impl<detail::LIST>::FindStringPosI(*this, str);
    return npos;
  }

  const Dynamic & Dynamic::operator[](const size_t pos) const
  {
    return GetByIndex(pos);
  }

  Dynamic & Dynamic::operator[](const size_t pos)
  {
    return GetByIndex(pos);
  }

  const Dynamic & Dynamic::GetByIndex(const size_t pos) const
  {
    if (IsList())
      return detail::Impl<detail::LIST>::GetItem(*this, pos);
    else
      return D_NONE;
  }

  Dynamic & Dynamic::GetByIndex(const size_t pos)
  {
    if (IsList())
      return detail::Impl<detail::LIST>::GetItem(*this, pos);
    else
      return D_NONE;
  }

  bool Dynamic::GetByIndex(const size_t pos, const Dynamic ** out) const
  {
    if (IsList())
      return detail::Impl<detail::LIST>::GetItem(*this, pos, out);
    else
      return false;
  }

  bool Dynamic::GetByIndex(const size_t pos, Dynamic ** out)
  {
    if (IsList())
      return detail::Impl<detail::LIST>::GetItem(*this, pos, out);
    else
      return false;
  }

  Dynamic::ListConstIterator Dynamic::begin_l() const
  {
    if (IsList())
      return Dynamic::ListConstIterator(detail::Impl<detail::LIST>::Begin(*this));
    else
      return Dynamic::ListConstIterator();
  }

  Dynamic::ListConstIterator Dynamic::end_l() const
  {
    if (IsList())
      return Dynamic::ListConstIterator(detail::Impl<detail::LIST>::End(*this));
    else
      return Dynamic::ListConstIterator();
  }

  void Dynamic::PushBack(const Dynamic & item)
  {
    if (IsList())
      detail::Impl<detail::LIST>::PushBack(*this, item);
  }

  void Dynamic::PushFront(const Dynamic & item)
  {
    if (IsList())
      detail::Impl<detail::LIST>::PushFront(*this, item);
  }

  void Dynamic::PopFront()
  {
    if (IsList())
      detail::Impl<detail::LIST>::PopFront(*this);
  }

  void Dynamic::Erase(const SizeSet & pos_set)
  {
    if (IsList())
      detail::Impl<detail::LIST>::Erase(*this, pos_set);
  }

  void Dynamic::Erase(const size_t & pos)
  {
    if (IsList())
      detail::Impl<detail::LIST>::Erase(*this, pos);
  }

  void Dynamic::Erase(const size_t & from, const size_t & to)
  {
    if (IsList())
      detail::Impl<detail::LIST>::Erase(*this, from, to);
  }

  void Dynamic::PopBack()
  {
    if (IsList())
      detail::Impl<detail::LIST>::PopBack(*this);
  }

  const Dynamic & Dynamic::front() const
  {
    if (IsList())
      return detail::Impl<detail::LIST>::GetFront(*this);
    return D_NONE;
  }

  Dynamic & Dynamic::front()
  {
    if (IsList())
      return detail::Impl<detail::LIST>::GetFront(*this);
    return D_NONE;
  }

  const Dynamic & Dynamic::back() const
  {
    if (IsList())
      return detail::Impl<detail::LIST>::GetBack(*this);
    return D_NONE;
  }

  Dynamic & Dynamic::back()
  {
    if (IsList())
      return detail::Impl<detail::LIST>::GetBack(*this);
    return D_NONE;
  }

  Dynamic & Dynamic::Extend(const Dynamic & rv)
  {
    if (IsList() && this != &rv)
    {
      if (rv.IsList())
        detail::Impl<detail::LIST>::Extend(data_, rv.data_);
      else
        PushBack(rv);
    }
    return *this;
  }

  void Dynamic::Join(const std::string & delimiter, const std::string & prefix,
      const std::string & postfix, const std::string & format,
      std::string * out) const
  {
    if (IsList())
      detail::Impl<detail::LIST>::Join(*this,
          delimiter, prefix, postfix, format.c_str(), out);
  }

  void Dynamic::Join(const std::string & delimiter, const std::string & prefix,
      const std::string & postfix, const char * format,
      std::string * out) const
  {
    if (IsList())
      detail::Impl<detail::LIST>::Join(*this,
          delimiter, prefix, postfix, format, out);
  }

  void Dynamic::Join(const std::string & delimiter, const std::string & prefix,
      const std::string & postfix, std::string * out) const
  {
    if (IsList())
      detail::Impl<detail::LIST>::Join(*this, delimiter, prefix, postfix, out);
  }

  Dynamic & Dynamic::Update(const Dynamic & rv)
  {
    if (IsDict() && this != &rv)
      detail::Impl<detail::DICT>::Update(*this, rv);
    return *this;
  }

  void Dynamic::GetKeys(StringSet * const keys) const
  {
    if (IsDict())
      detail::Impl<detail::DICT>::GetKeys(*this, keys);
  }

  void Dynamic::Erase(const char * const key)
  {
    Erase(std::string(key));
  }

  void Dynamic::Erase(const std::string & key)
  {
    if (IsDict())
      detail::Impl<detail::DICT>::DeleteByKey(*this, key);
  }

  Dynamic & Dynamic::Update(const char * key, const Dynamic & value)
  {
    return Update(std::string(key), value);
  }

  Dynamic & Dynamic::Update(const std::string & key, const Dynamic & value)
  {
    if (IsDict())
      return detail::Impl<detail::DICT>::Set(*this, key, value);
    return D_NONE;
  }

  Dynamic & Dynamic::operator[](const std::string & key)
  {
    return Get(key);
  }

  const Dynamic & Dynamic::operator[](const std::string & key) const
  {
    return Get(key);
  }

  Dynamic & Dynamic::operator[](const char * const key)
  {
    return Get(std::string(key));
  }

  const Dynamic & Dynamic::operator[](const char * const key) const
  {
    return Get(std::string(key));
  }

  Dynamic & Dynamic::Get(const char * key)
  {
    if (IsDict())
      return detail::Impl<detail::DICT>::Get(*this, key);
    return D_NONE;
  }

  Dynamic & Dynamic::Get(const std::string & key)
  {
    if (IsDict())
      return detail::Impl<detail::DICT>::Get(*this, key);
    return D_NONE;
  }

  const Dynamic & Dynamic::Get(const std::string & key) const
  {
    if (IsDict())
      return detail::Impl<detail::DICT>::Get(*this, key);
    return D_NONE;
  }

  const Dynamic & Dynamic::Get(const char * const key) const
  {
    if (IsDict())
      return detail::Impl<detail::DICT>::Get(*this, key);
    return D_NONE;
  }

  bool Dynamic::Get(const char * const key, Dynamic ** const value)
  {
    return Get(std::string(key), value);
  }

  bool Dynamic::Get(const std::string & key, Dynamic ** const value)
  {
    if (IsDict())
      return detail::Impl<detail::DICT>::Get(*this, key, value);
    else
      return false;
  }

  bool Dynamic::Get(const char * key, const Dynamic ** const value) const
  {
    return Get(std::string(key), value);
  }

  bool Dynamic::Get(const std::string & key, const Dynamic ** const value) const
  {
    if (IsDict())
      return detail::Impl<detail::DICT>::Get(*this, key, value);
    else
      return false;
  }
  Dynamic::DictConstIterator Dynamic::begin_d() const
  {
    if (IsDict())
      return detail::Impl<detail::DICT>::Begin(*this);
    else
      return Dynamic::DictConstIterator();
  }

  Dynamic::DictConstIterator Dynamic::end_d() const
  {
    if (IsDict())
      return detail::Impl<detail::DICT>::End(*this);
    else
      return Dynamic::DictConstIterator();
  }

  Dynamic::DictConstIterator Dynamic::FindByKey(const std::string & key) const
  {
    if (IsDict())
      return detail::Impl<detail::DICT>::Find(*this, key);
    else
      return Dynamic::DictConstIterator();
  }
/*
  bool Dynamic::SaveTo(StringMap * const out) const
  {
    if (!IsDict())
      return false;
    out->clear();
    DDICT_FOREACH(it, *this)
      (*out)[it->first] = it->second.GetString();
    return true;
  }

  bool Dynamic::SaveTo(StringSet * const out) const
  {
    if (!IsList())
      return false;
    out->clear();
    DLIST_FOREACH(it, *this)
      out->insert(it->GetString());
    return true;
  }

  bool Dynamic::SaveTo(StringVector * const out) const
  {
    if (!IsList())
      return false;
    out->clear();
    DLIST_FOREACH(it, *this)
      out->push_back(it->GetString());
    return true;
  }

  bool Dynamic::SaveTo(IntVector * const out) const
  {
    if (!IsList())
      return false;
    out->clear();
    DLIST_FOREACH(it, *this)
      out->push_back(it->GetSignedInteger());
    return true;
  }

  bool Dynamic::SaveTo(UintVector * const out) const
  {
    if (!IsList())
      return false;
    out->clear();
    DLIST_FOREACH(it, *this)
      out->push_back(it->GetUnsignedInteger());
    return true;
  }

  bool Dynamic::SaveTo(Uint16Set * const out) const
  {
    if (!IsList())
      return false;
    out->clear();
    DLIST_FOREACH(it, *this)
      out->insert((uint16_t)it->GetUnsignedInteger());
    return true;
  }

  bool Dynamic::SaveTo(Int16Set * const out) const
  {
    if (!IsList())
      return false;
    out->clear();
    DLIST_FOREACH(it, *this)
      out->insert((int16_t)it->GetSignedInteger());
    return true;
  }
*/
  bool Dynamic::empty() const
  {
    return detail::Operation<detail::OP_IS_EMPTY>::farray[type_](data_);
  }

  size_t Dynamic::size() const
  {
    return detail::Operation<detail::OP_GET_SIZE>::farray[type_](data_);
  }

  bool Dynamic::StartsWith(const std::string & text) const
  {
    if (!IsString())
      return false;
    return detail::Impl<detail::STRING>::StartsWith(*this, text);
  }

  bool Dynamic::EndsWith(const std::string & text) const
  {
    if (!IsString())
      return false;
    return detail::Impl<detail::STRING>::EndsWith(*this, text);
  }

  bool Dynamic::Replace(const std::string & what, const std::string & with)
  {
    if (!IsString())
      return false;
    return detail::Impl<detail::STRING>::Replace(*this, what, with);
  }

  uint32_t Dynamic::year() const
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::year(data_);
    else
      return 0;
  }

  uint32_t Dynamic::month() const
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::month(data_);
    else
      return 0;
  }

  uint32_t Dynamic::day() const
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::day(data_);
    else
      return 0;
  }

  uint32_t Dynamic::hours() const
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::hours(data_);
    else
      return 0;
  }

  uint32_t Dynamic::minutes() const
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::minutes(data_);
    else
      return 0;
  }

  uint32_t Dynamic::seconds() const
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::seconds(data_);
    else
      return 0;
  }

  uint32_t Dynamic::microseconds() const
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::microseconds(data_);
    else
      return 0;
  }

  int64_t Dynamic::timestamp() const
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::GetTimestump(data_);
    return 0;
  }

  bool Dynamic::leap() const
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::IsLeap(data_);
    return false;
  }

  /*
  Dynamic Dynamic::date() const
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::GetDate(data_);
    return Dynamic();
  }

  Dynamic Dynamic::time() const
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::GetTime(data_);
    return Dynamic();
  }
  */

  void Dynamic::AddDays(const int32_t days_count)
  {
    if (IsDateTime())
      detail::Impl<detail::DATE_TIME>::AddDays(data_, days_count);
  }

  void Dynamic::AddHours(const int32_t hh)
  {
    if (IsDateTime())
      detail::Impl<detail::DATE_TIME>::AddSeconds(data_, hh*3600);
  }

  void Dynamic::AddMinutes(const int32_t mm)
  {
    if (IsDateTime())
      detail::Impl<detail::DATE_TIME>::AddSeconds(data_, mm*60);
  }

  void Dynamic::AddSeconds(const int32_t ss)
  {
    if (IsDateTime())
      detail::Impl<detail::DATE_TIME>::AddSeconds(data_, ss);
  }

  bool Dynamic::SetHour(const uint32_t h)
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::SetHour(data_, h);
    else
      return false;
  }

  bool Dynamic::SetMinute(const uint32_t m)
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::SetMinute(data_, m);
    else
      return false;
  }

  bool Dynamic::SetSecond(const uint32_t s)
  {
    if (IsDateTime())
      return detail::Impl<detail::DATE_TIME>::SetSecond(data_, s);
    else
      return false;
  }

  Dynamic Dynamic::Table(const std::string & table_def, std::string * error)
  {
    Dynamic result;
    detail::Impl<detail::TABLE>::Create(result, table_def, error);
    return result;
  }

  Dynamic Dynamic::Table(const StringVector & table_def, std::string * error)
  {
    Dynamic result;
    detail::Impl<detail::TABLE>::Create(result, table_def, error);
    return result;
  }

  size_t Dynamic::GetColumnNumber(const std::string & column_name) const
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::GetColumnNumber(*this, column_name);
    return Dynamic::npos;
  }

  void Dynamic::SetColumnName(size_t pos, const std::string & name)
  {
    if (IsTable())
      detail::Impl<detail::TABLE>::SetColumnName(*this, pos, name);
  }

  StringVector Dynamic::GetColumnNames() const
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::GetColumnNames(*this);
    return StringVector();
  }

  StringVector Dynamic::GetColumnTypes() const
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::GetColumnTypes(*this);
    return StringVector();
  }

  size_t Dynamic::width() const
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::GetWidth(*this);
    return 0;
  }

  size_t Dynamic::height() const
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::GetHeight(*this);
    return 0;
  }

  Dynamic Dynamic::GetCellValue(const size_t row_num,
      const size_t col_num) const
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::GetCellValue(*this, row_num, col_num);
    return D_NONE;
  }

  void Dynamic::JoinColumnCells(const size_t col, const std::string & delimiter,
      std::string * out) const
  {
    if (!IsTable())
      return;

    out->clear();
    if (empty())
      return;

    TableIterator it = begin_t(), end = end_t();
    out->assign(it[col].GetString());
    for (++it; it != end; ++it)
    {
      out->append(delimiter);
      out->append(it[col].GetString());
    }
  }

  void Dynamic::SaveColumn(const size_t col, Dynamic * list) const
  {
    if (!IsTable() || width() <= col)
      return;

    *list = Dynamic::List();
    if (empty())
      return;

    TableIterator it = begin_t(), end = end_t();
    for (; it != end; ++it)
      list->PushBack(it[col]);
  }

  Dynamic::TableIterator Dynamic::begin_t() const
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::begin_t(*this);
    return TableIterator(NULL);
  }

  Dynamic::TableIterator Dynamic::end_t() const
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::end_t(*this);
    return TableIterator(NULL);
  }

  bool Dynamic::AppendRow(const DynamicVector & args)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::AppendRow(*this, args);
    return false;
  }

  bool Dynamic::AppendRow(const Dynamic & v)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::AppendRow(*this, v);
    return false;
  }

  bool Dynamic::AppendRow(const Dynamic & v1, const Dynamic & v2)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::AppendRow(*this, v1, v2);
    return false;
  }

  bool Dynamic::AppendRow(const Dynamic & v1, const Dynamic & v2,
      const Dynamic & v3)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::AppendRow(*this, v1, v2, v3);
    return false;
  }

  bool Dynamic::AppendRow(const Dynamic & v1, const Dynamic & v2,
      const Dynamic & v3, const Dynamic & v4)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::AppendRow(*this, v1, v2, v3, v4);
    return false;
  }

  bool Dynamic::AppendRow(const Dynamic & v1, const Dynamic & v2,
      const Dynamic & v3, const Dynamic & v4, const Dynamic & v5)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::AppendRow(*this, v1, v2, v3, v4, v5);
    return false;
  }

  bool Dynamic::AppendRow(const Dynamic & v1, const Dynamic & v2,
      const Dynamic & v3, const Dynamic & v4, const Dynamic & v5,
      const Dynamic & v6)
  {
     if (IsTable())
      return detail::Impl<detail::TABLE>::AppendRow(
          *this, v1, v2, v3, v4, v5, v6);
    return false;
  }

  bool Dynamic::AppendRow(const Dynamic & v1, const Dynamic & v2,
      const Dynamic & v3, const Dynamic & v4, const Dynamic & v5,
      const Dynamic & v6, const Dynamic & v7)
  {
     if (IsTable())
      return detail::Impl<detail::TABLE>::AppendRow(
          *this, v1, v2, v3, v4, v5, v6, v7);
    return false;
  }

  bool Dynamic::SetRow(const size_t row_num, const DynamicVector & vargs)
  {
     if (IsTable())
       return detail::Impl<detail::TABLE>::SetRow(*this, row_num, vargs);
     return false;
  }

  bool Dynamic::InsertRow(const size_t row_num, const DynamicVector & vargs)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::InsertRow(*this, row_num, vargs);
    return false;
  }

  bool Dynamic::DeleteRow(const size_t row_num)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::DeleteRow(*this, row_num);
    return false;
  }

  bool Dynamic::DeleteRow(const std::set<size_t> & row_set)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::DeleteRow(*this, row_set);
    return false;
  }

  bool Dynamic::SetCellValue(const size_t row_num,
      const size_t col_num, const Dynamic & v)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::SetCellValue(
          *this, row_num, col_num, v);
    return false;
  }

  // index life time management
  TableIndex::Ptr Dynamic::CreateIndex(
      const std::string & index_definition, std::string * error)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::CreateIndex(
          *this, index_definition, error);
    return TableIndex::Ptr();
  }

  void Dynamic::DeleteIndex(TableIndex::Ptr index)
  {
    if (IsTable())
      detail::Impl<detail::TABLE>::DeleteIndex(*this, index);
  }

  void Dynamic::DeleteAllIndices()
  {
    if (IsTable())
      detail::Impl<detail::TABLE>::DeleteAllIndices(*this);
  }

  Dynamic Dynamic::Group(const std::string & columns, const std::string & aggr,
    std::string * error)
  {
    if (IsTable())
      return detail::Impl<detail::TABLE>::Group(data_, columns, aggr, error);
    return D_NONE;
  }

  GroupedTableBuilder::Ptr Dynamic::CreateGroupedTableBuilder(
          const std::string & table_def,
          const std::string & index_def,
          const std::string & aggr,
          std::string * error)
  {
    return GroupedTableBuilder::Create(table_def, index_def, aggr, error);
  }

  Dynamic Dynamic::TableIterator::operator[](const size_t col_num) const
  {
    assert(shared_table_ != NULL);
    return shared_table_->GetCellValue(row_it_, col_num);
  }
}  // namespace nkit

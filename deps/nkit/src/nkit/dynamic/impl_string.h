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

#include <cstdlib>

namespace nkit
{
  namespace detail
  {
    //--------------------------------------------------------------------------
    class SharedString : public Shared<std::string>
    {
    public:
      SharedString(const std::string & s) : Shared<std::string>(s) {}
      SharedString(const char * s, const size_t len)
        : Shared<std::string>(s, len) {}
      SharedString(size_t n, char c) : Shared<std::string>(n, c) {}

      static const SharedString * Get(const Data & data)
      {
        return data.shared_string_;
      }

      static SharedString * Get(Data & data)
      {
        return data.shared_string_;
      }
    };

    //--------------------------------------------------------------------------
    template <typename T1, typename T2>
    struct Lt
    {
      static bool OP(const Dynamic & v, const Dynamic & rv);
    };

    template <>
    struct Lt<Impl<STRING>, Impl<STRING> >
    {
      inline static bool OP(const Dynamic & v, const Dynamic & rv);
    };

    template <>
    struct Lt<Impl<STRING>, Impl<MONGODB_OID> >
    {
      inline static bool OP(const Dynamic & v, const Dynamic & rv);
    };

    template <>
    struct Lt<Impl<MONGODB_OID>, Impl<STRING> >
    {
      inline static bool OP(const Dynamic & v, const Dynamic & rv);
    };

    template <>
    struct Lt<Impl<MONGODB_OID>, Impl<MONGODB_OID> >
    {
      inline static bool OP(const Dynamic & v, const Dynamic & rv);
    };

    //--------------------------------------------------------------------------
    template <typename T1, typename T2>
    struct Eq
    {
      static bool OP(const Dynamic & v, const Dynamic & rv);
    };

    template <>
    struct Eq<Impl<STRING>, Impl<STRING> >
    {
      inline static bool OP(const Dynamic & v, const Dynamic & rv);
    };

    template <>
    struct Eq<Impl<STRING>, Impl<MONGODB_OID> >
    {
      inline static bool OP(const Dynamic & v, const Dynamic & rv);
    };

    template <>
    struct Eq<Impl<MONGODB_OID>, Impl<STRING> >
    {
      inline static bool OP(const Dynamic & v, const Dynamic & rv);
    };

    template <>
    struct Eq<Impl<MONGODB_OID>, Impl<MONGODB_OID> >
    {
      inline static bool OP(const Dynamic & v, const Dynamic & rv);
    };

    //--------------------------------------------------------------------------
    template <>
    class Impl<STRING> : public ImplDefault
    {
    public:
      static void Create(Dynamic & v, const std::string & s)
      {
        Reset(&v, new detail::SharedString(s));
      }

      static void Create(Dynamic & v, const char * s)
      {
        Reset(&v, new detail::SharedString(s));
      }

      static void Create(Dynamic & v, const char * s, const size_t length)
      {
        Reset(&v, new detail::SharedString(s, length));
      }

      static void OP_CLEAR(Dynamic & v)
      {
        GetSharedPtr(v.data_)->GetRef().clear();
      }

      static Dynamic OP_CLONE(const Data & v)
      {
        return Dynamic(GetString(v));
      }

      static void OP_DEC_REF_DYNAMIC(Dynamic & v)
      {
        detail::SharedString * shared = GetSharedPtr(v.data_);
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
        detail::SharedString * shared = GetSharedPtr(data);
        if (shared->DecRef() == 0)
        {
          delete shared;
          data.i64_ = 0;
        }
      }

      static int64_t OP_GET_INT(const Data & v)
      {
        return NKIT_STRTOLL(OP_GET_CONST_STRING(v).c_str(), NULL, 10);
      }

      static uint64_t OP_GET_UINT(const Data & v)
      {
        return NKIT_STRTOULL(OP_GET_CONST_STRING(v).c_str(), NULL, 10);
      }

      static bool OP_GET_BOOL(const Data & v)
      {
        return !OP_IS_EMPTY(v);
      }

      static bool OP_IS_EMPTY(const Data & v)
      {
        return OP_GET_CONST_STRING(v).empty();
      }

      static size_t OP_GET_SIZE(const Data & v)
      {
        return OP_GET_CONST_STRING(v).size();
      }

      static double GET_DOUBLE(const Data & v)
      {
        return strtod(OP_GET_CONST_STRING(v).c_str(), NULL);
      }

      static std::string OP_GET_STRING(const Data & v,
        const char * NKIT_UNUSED(format))
      {
        return GetString(v);
      }

      static std::string OP_GET_STRING(const Dynamic & v,
        const char * NKIT_UNUSED(format))
      {
        return GetString(v.data_);
      }

      static const std::string & OP_GET_CONST_STRING(const Data & v)
      {
        return GetString(v);
      }

      static const std::string & OP_GET_CONST_STRING(const Dynamic & v)
      {
        return GetString(v.data_);
      }

      static bool StartsWith(const Dynamic & v,
          const std::string & with)
      {
        const std::string & what = GetString(v.data_);
        return starts_with(what, with);
      }

      static bool EndsWith(const Dynamic & v,
          const std::string & with)
      {
        const std::string & what = GetString(v.data_);
        return ends_with(what, with);
      }

      static bool Replace(Dynamic & v,
          const std::string & from, const std::string & to)
      {
        std::string & value = GetString(v.data_);
        if (value.size() < from.size())
          return false;
        size_t start_pos = 0;
        while((start_pos = value.find(from, start_pos)) != std::string::npos)
        {
          value.replace(start_pos, from.length(), to);
          start_pos += to.length();
        }
        return false;
      }

      template<typename T>
      static bool OP_LT(const Dynamic & v, const Dynamic & rv)
      {
        return Lt<Impl<STRING>, T>::OP(v, rv);
      }

      template<typename T>
      static bool OP_EQ(const Dynamic & v, const Dynamic & rv)
      {
        return Eq<Impl<STRING>, T>::OP(v, rv);
      }

      template<typename T>
      static void OP_ADD(Data & v, const Data & rv)
      {
        GetString(v).append(T::OP_GET_STRING(rv, ""));
      }

      template<typename T>
      static void OP_SUB(Data & v, const Data & rv)
      {
        uint64_t count = T::OP_GET_UINT(rv);
        size_t truncate_by = (size_t) count;
        std::string & old = GetString(v);
        size_t old_size(old.size());
        size_t new_size =
            (old_size <= truncate_by) ? 0 : old_size - truncate_by;

        old.resize(new_size);
      }

      template<typename T>
      static bool OP_MUL(Data & v, const Data & rv)
      {
        int32_t repeat_count = (uint32_t) T::OP_GET_UINT(rv);
        std::string & old = GetString(v);
        std::string new_string;
        new_string.reserve(old.size() * repeat_count);
        for (int32_t i = 0; i < repeat_count; ++i)
          new_string.append(old);

        old.swap(new_string);
        return true;
      }

      template<typename T>
      static bool OP_DIV(Data & v, const Data & rv)
      {
        uint64_t count = T::OP_GET_UINT(rv);
        size_t divide_by = (size_t) count;
        if (divide_by == 0)
          return false;
        std::string & old = GetString(v);
        size_t old_size(old.size());
        size_t new_size = (old_size - (old_size % (size_t) divide_by))
            / (size_t) divide_by;
        old.resize(new_size);
        return true;
      }

    private:
      static const std::string & GetString(const Data & v)
      {
        return GetSharedPtr(v)->GetRef();
      }

      static std::string & GetString(Data & v)
      {
        return GetSharedPtr(v)->GetRef();
      }

      static const detail::SharedString * GetSharedPtr(const Data & data)
      {
        return detail::SharedString::Get(data);
      }

      static detail::SharedString * GetSharedPtr(Data & data)
      {
        return detail::SharedString::Get(data);
      }

      static void Reset(Dynamic * v, detail::SharedString * const str)
      {
        v->type_ = STRING;
        v->data_.shared_string_ = str;
      }
    };

    template <typename T1, typename T2>
    bool Lt<T1, T2>::OP(const Dynamic & v, const Dynamic & rv)
    {
      return T1::OP_GET_CONST_STRING(v) < T2::OP_GET_STRING(rv, "");
    }

    inline bool Lt<Impl<STRING>, Impl<STRING> >::OP(const Dynamic & v,
        const Dynamic & rv)
    {
      return Impl<STRING>::OP_GET_CONST_STRING(v) <
          Impl<STRING>::OP_GET_CONST_STRING(rv);
    }

    template <typename T1, typename T2>
    bool Eq<T1, T2>::OP(const Dynamic & v, const Dynamic & rv)
    {
      return T1::OP_GET_CONST_STRING(v) == T2::OP_GET_STRING(rv, "");
    }

    inline bool Eq<Impl<STRING>, Impl<STRING> >::OP(const Dynamic & v,
        const Dynamic & rv)
    {
      return Impl<STRING>::OP_GET_CONST_STRING(v) ==
          Impl<STRING>::OP_GET_CONST_STRING(rv);
    }

  } // namespace detail
} // namespace nkit

#include "nkit/dynamic/impl_mongodb_oid.h"

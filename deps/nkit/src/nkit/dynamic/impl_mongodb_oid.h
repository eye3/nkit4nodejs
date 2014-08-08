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

namespace nkit
{
  namespace detail
  {
    template <>
    class Impl<detail::MONGODB_OID> : public ImplDefault
    {
    public:
      static void Create(Dynamic & v, const std::string & s)
      {
        if ((s.size() != 24) || !is_hex_lower(s))
          v.Reset();
        else
          Reset(&v, new detail::SharedString(s));
        /* TODO: consider to use isxdigit function
         *
        const size_t size = s.size();
        if (size != 24)
        {
          for (size_t i = 0; i < size; ++i)
          {
              if (!isxdigit(static_cast<int>(s[i])))
              {
                v.Reset();
                return;
              }
          }
        }
        Reset(&v, new detail::SharedString(s));
        */
      }

      static void Create(Dynamic & v)
      {
        Reset(&v, new detail::SharedString(24, 'f'));
      }

      static void OP_CLEAR(Dynamic & NKIT_UNUSED(v))
      {}

      static Dynamic OP_CLONE(const Data & v)
      {
        return Dynamic::MongodbOID(GetString(v));
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

      static bool OP_GET_BOOL(const Data & v)
      {
        return !OP_IS_EMPTY(v);
      }

      static bool OP_IS_EMPTY(const Data & v)
      {
        return GetString(v).empty();
      }

      static size_t OP_GET_SIZE(const Data & v)
      {
        return GetString(v).size();
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

      template<typename T>
      static bool OP_LT(const Dynamic & v, const Dynamic & rv)
      {
        return Lt<Impl<MONGODB_OID>, T>::OP(v, rv);
      }

      template<typename T>
      static bool OP_EQ(const Dynamic & v, const Dynamic & rv)
      {
        return Eq<Impl<MONGODB_OID>, T>::OP(v, rv);
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
        v->type_ = MONGODB_OID;
        v->data_.shared_string_ = str;
      }
    };

    //--------------------------------------------------------------------------
    inline bool Lt<Impl<MONGODB_OID>, Impl<STRING> >::OP(const Dynamic & v,
        const Dynamic & rv)
    {
      return Impl<MONGODB_OID>::OP_GET_CONST_STRING(v) <
          Impl<STRING>::OP_GET_CONST_STRING(rv);
    }

    inline bool Lt<Impl<STRING>, Impl<MONGODB_OID> >::OP(const Dynamic & v,
        const Dynamic & rv)
    {
      return Impl<STRING>::OP_GET_CONST_STRING(v) <
          Impl<MONGODB_OID>::OP_GET_CONST_STRING(rv);
    }

    inline bool Lt<Impl<MONGODB_OID>, Impl<MONGODB_OID> >::OP(
        const Dynamic & v, const Dynamic & rv)
    {
      return Impl<MONGODB_OID>::OP_GET_CONST_STRING(v) <
          Impl<MONGODB_OID>::OP_GET_CONST_STRING(rv);
    }

    //--------------------------------------------------------------------------
    inline bool Eq<Impl<MONGODB_OID>, Impl<STRING> >::OP(const Dynamic & v,
        const Dynamic & rv)
    {
      return Impl<MONGODB_OID>::OP_GET_CONST_STRING(v) ==
          Impl<STRING>::OP_GET_CONST_STRING(rv);
    }

    inline bool Eq<Impl<STRING>, Impl<MONGODB_OID> >::OP(const Dynamic & v,
        const Dynamic & rv)
    {
      return Impl<STRING>::OP_GET_CONST_STRING(v) ==
          Impl<MONGODB_OID>::OP_GET_CONST_STRING(rv);
    }

    inline bool Eq<Impl<MONGODB_OID>, Impl<MONGODB_OID> >::OP(
        const Dynamic & v, const Dynamic & rv)
    {
      return Impl<MONGODB_OID>::OP_GET_CONST_STRING(v) ==
          Impl<MONGODB_OID>::OP_GET_CONST_STRING(rv);
    }

  } // namespace detail
} // namespace nkit

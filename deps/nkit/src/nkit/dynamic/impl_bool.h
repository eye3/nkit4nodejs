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

#include <cstdio>
#include <cstdarg>

namespace nkit
{
  namespace detail
  {
    template <>
    class Impl<detail::BOOL> : public ImplDefault
    {
    public:
      static void Create(Dynamic & v, const bool b)
      {
        Reset(&v, b);
      }

      static void OP_CLEAR(Dynamic & v) { v.Reset(); }

      static Dynamic OP_CLONE(const Data & v)
      {
        Dynamic result;
        Reset(&result, v.i64_ != 0);
        return result;
      }

      static void OP_DEC_REF_DYNAMIC(Dynamic & ) {}
      static void OP_INC_REF_DATA(Data & ) {}
      static void OP_DEC_REF_DATA(Data & ) {}

      static int64_t OP_GET_INT(const Data & v)
      {
        return v.i64_;
      }

      static uint64_t OP_GET_UINT(const Data & v)
      {
        return v.ui64_;
      }

      static bool OP_GET_BOOL(const Data & v)
      {
        return v.ui64_ != 0;
      }

      static bool OP_IS_EMPTY(const Data & NKIT_UNUSED(v))
      {
        return false;
      }

      static double GET_DOUBLE(const Data & v)
      {
        return static_cast<double>(v.i64_);
      }

      static const std::string & OP_GET_CONST_STRING(const Data & v)
      {
        return v.ui64_ == 0 ? S_FALSE_ : S_TRUE_;
      }

      static std::string OP_GET_STRING(const Data & v,
          const char * NKIT_UNUSED(format))
      {
        return string_cast(v.i64_);
      }

      static std::string OP_GET_STRING(const Dynamic & v,
          const char * format)
      {
        return OP_GET_STRING(v.data_, format);
      }

      template<typename T>
      static bool OP_LT(const Dynamic & v, const Dynamic & rv)
      {
        return v.data_.i64_ < T::OP_GET_INT(rv.data_);
      }

      template<typename T>
      static bool OP_EQ(const Dynamic & v, const Dynamic & rv)
      {
        return v.data_.i64_ == T::OP_GET_INT(rv.data_);
      }

      template<typename T>
      static const Data & OP_MIN(const Data & v, const Data & rv)
      {
        return (v.ui64_ < T::OP_GET_UINT(rv)) ? v : rv;
      }

      template<typename T>
      static const Data & OP_MAX(const Data & v, const Data & rv)
      {
        return (v.ui64_ >= T::OP_GET_UINT(rv)) ? v : rv;
      }

      static Data OP_GET_DEFAULT_DATA()
      {
        return OP_GET_MIN_DATA();
      }

      static Data OP_GET_MAX_DATA()
      {
        Data result;
        result.ui64_ = 1;
        return result;
      }

      static Data OP_GET_MIN_DATA()
      {
        Data result;
        result.ui64_ = 0;
        return result;
      }

    private:
      static void Reset(Dynamic * v, const bool b)
      {
        v->type_ = BOOL;
        v->data_.i64_ = (b ? 1 : 0);
      }
    };
  } // namespace detail
} // namespace nkit

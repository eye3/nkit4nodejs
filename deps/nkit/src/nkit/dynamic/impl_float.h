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
    class Impl<detail::FLOAT> : public ImplDefault
    {
    public:
      static void Create(Dynamic & v, const double f)
      {
        Reset(&v, f);
      }

      static void OP_CLEAR(Dynamic & v) { v.Reset(); }

      static Dynamic OP_CLONE(const Data & v)
      {
        Dynamic result;
        Reset(&result, v.f_);
        return result;
      }

      static void OP_DEC_REF_DYNAMIC(Dynamic & ) {}
      static void OP_INC_REF_DATA(Data & ) {}
      static void OP_DEC_REF_DATA(Data & ) {}

      static int64_t OP_GET_INT(const Data & v)
      {
        return static_cast<int64_t>(v.f_);
      }

      static uint64_t OP_GET_UINT(const Data & v)
      {
        return static_cast<uint64_t>(v.f_);
      }

      static bool OP_GET_BOOL(const Data & v)
      {
        return v.f_ != 0.0;
      }

      static bool OP_IS_EMPTY(const Data & NKIT_UNUSED(v))
      {
        return false;
      }

      static double GET_DOUBLE(const Data & v)
      {
        return v.f_;
      }

      static std::string OP_GET_STRING(const Data & v, const char * format)
      {
        char buf[128 + 1];
        if (*format != '\0')
        {
          NKIT_SNPRINTF(buf, 128, format, v.f_);
        }
        else
        {
          NKIT_SNPRINTF(buf, 128, "%f", v.f_);
        }
        return buf;
      }

      static std::string OP_GET_STRING(const Dynamic & v,
          const char * format)
      {
        return OP_GET_STRING(v.data_, format);
      }

      template<typename T>
      static bool OP_LT(const Dynamic & v, const Dynamic & rv)
      {
        return v.data_.f_ < T::GET_DOUBLE(rv.data_);
      }

      template<typename T>
      static bool OP_EQ(const Dynamic & v, const Dynamic & rv)
      {
        return v.data_.f_ == T::GET_DOUBLE(rv.data_);
      }

      template<typename T>
      static void OP_ADD(Data & v, const Data & rv)
      {
        v.f_ += T::GET_DOUBLE(rv);
      }

      template<typename T>
      static void OP_SUB(Data & v, const Data & rv)
      {
        v.f_ -= T::GET_DOUBLE(rv);
      }

      template<typename T>
      static bool OP_MUL(Data & v, const Data & rv)
      {
        v.f_ *= T::GET_DOUBLE(rv);
        return true;
      }

      template<typename T>
      static bool OP_DIV(Data & v, const Data & rv)
      {
        double d = T::GET_DOUBLE(rv);
        if (d == 0.0)
          return false;
        v.f_ /= d;
        return true;
      }

      template<typename T>
      static const Data & OP_MIN(const Data & v, const Data & rv)
      {
        if (v.f_ < T::GET_DOUBLE(rv))
          return v;
        return rv;
      }

      template<typename T>
      static const Data & OP_MAX(const Data & v, const Data & rv)
      {
        if (v.f_ > T::GET_DOUBLE(rv))
          return v;
        return rv;
      }

      static Data OP_GET_DEFAULT_DATA()
      {
        Data result;
        result.f_ = 0.0;
        return result;
      }

      static Data OP_GET_MAX_DATA()
      {
        Data result;
        result.f_ = static_cast<double>(MAX_INT64_VALUE);
        return result;
      }

      static Data OP_GET_MIN_DATA()
      {
        Data result;
        result.f_ = static_cast<double>(MIN_INT64_VALUE);
        return result;
      }

    private:
      static void Reset(Dynamic * v, const double d)
      {
        v->type_ = FLOAT;
        v->data_.f_ = d;
      }
    };
  } // namespace detail
} // namespace nkit

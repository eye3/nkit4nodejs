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
#if defined(NKIT_USE_CUSTOM_UINT64_CAST_TO_DOUBLE)

    static inline double cast_to_double( uint64_t value )
    {
        return static_cast<double>( int64_t(value/2) ) * 2.0 +
            int64_t(value & 1);
    }

#else

    static inline double cast_to_double( uint64_t value )
    {
        return static_cast<double>( value );
    }

#endif // if !defined(NKIT_USE_CUSTOM_UINT64_CAST_TO_DOUBLE)

    template <typename T>
    struct IntOperations
    {
      static bool lt(const int64_t lv, const Data & rv)
      {
        return lv < T::OP_GET_INT(rv);
      }

      static bool lt(const uint64_t lv, const Data & rv)
      {
        return lv < T::OP_GET_UINT(rv);
      }

      static bool eq(const int64_t lv, const Data & rv)
      {
        return lv == T::OP_GET_INT(rv);
      }

      static bool eq(const uint64_t lv, const Data & rv)
      {
        return lv == T::OP_GET_UINT(rv);
      }

      static bool gte(const int64_t lv, const Data & rv)
      {
        return !lt(lv, rv);
      }

      static bool gte(const uint64_t lv, const Data & rv)
      {
        return !lt(lv, rv);
      }

      static void add(int64_t & lv, const Data & rv)
      {
        lv += T::OP_GET_INT(rv);
      }

      static void add(uint64_t & lv, const Data & rv)
      {
        lv += T::OP_GET_UINT(rv);
      }

      static void sub(int64_t & lv, const Data & rv)
      {
        lv -= T::OP_GET_INT(rv);
      }

      static void sub(uint64_t & lv, const Data & rv)
      {
        lv -= T::OP_GET_UINT(rv);
      }

      static bool mul(int64_t & lv, const Data & rv)
      {
        lv *= T::OP_GET_INT(rv);
        return true;
      }

      static bool mul(uint64_t & lv, const Data & rv)
      {
        lv *= T::OP_GET_UINT(rv);
        return true;
      }

      static bool div(int64_t & lv, const Data & rv)
      {
        int64_t si = T::OP_GET_INT(rv);
        if (si == 0)
          return false;
        lv /= si;
        return true;
      }

      static bool div(uint64_t & lv, const Data & rv)
      {
        uint64_t ui = T::OP_GET_UINT(rv);
        if (ui == 0)
          return false;
        lv /= ui;
        return true;
      }
    };

    template <>
    struct IntOperations<Impl<detail::INTEGER> >
    {
      static bool lt(const int64_t lv, const Data & rv)
      {
        return lv < rv.i64_;
      }

      static bool lt(const uint64_t lv, const Data & rv)
      {
        return (rv.i64_ >= 0) && (lv < uint64_t(rv.i64_));
      }

      static bool eq(const int64_t lv, const Data & rv)
      {
        return lv == rv.i64_;
      }

      static bool eq(const uint64_t lv, const Data & rv)
      {
        return (rv.i64_ >= 0) && (lv == uint64_t(rv.i64_));
      }

      static bool gte(const int64_t lv, const Data & rv)
      {
        return !lt(lv, rv);
      }

      static bool gte(const uint64_t lv, const Data & rv)
      {
        return !lt(lv, rv);
      }

      static void add(int64_t & lv, const Data & rv)
      {
        lv += rv.i64_;
      }

      static void add(uint64_t & lv, const Data & rv)
      {
        if (rv.i64_ < 0)
          lv -= uint64_t( - rv.i64_);
        else
          lv += uint64_t(rv.i64_);
      }

      static void sub(int64_t & lv, const Data & rv)
      {
        lv -= rv.i64_;
      }

      static void sub(uint64_t & lv, const Data & rv)
      {
        if (rv.i64_ < 0)
          lv += uint64_t( - rv.i64_);
        else
          lv -= uint64_t(rv.i64_);
      }

      static bool mul(int64_t & lv, const Data & rv)
      {
        lv *= rv.i64_;
        return true;
      }

      static bool mul(uint64_t & lv, const Data & rv)
      {
        int64_t si = rv.i64_;
        if (unlikely(si < 0))
          return false;
        lv *= rv.ui64_;
        return true;
      }

      static bool div(int64_t & lv, const Data & rv)
      {
        int64_t si = rv.i64_;
        if (unlikely(si == 0))
          return false;
        lv /= si;
        return true;
      }

      static bool div(uint64_t & lv, const Data & rv)
      {
        int64_t si = rv.i64_;
        if (unlikely(si <= 0))
          return false;
        lv /= rv.ui64_;
        return true;
      }
    };

    template <>
    struct IntOperations<Impl<detail::UNSIGNED_INTEGER> >
    {
      static bool lt(const int64_t lv, const Data & rv)
      {
        return (lv < 0) || (uint64_t(lv) < rv.ui64_);
      }

      static bool lt(const uint64_t lv, const Data & rv)
      {
        return lv < rv.ui64_;
      }

      static bool eq(const int64_t lv, const Data & rv)
      {
        return (lv >= 0) && (uint64_t(lv) == rv.ui64_);
      }

      static bool eq(const uint64_t lv, const Data & rv)
      {
        return lv == rv.ui64_;
      }

      static bool gte(const int64_t lv, const Data & rv)
      {
        return !lt(lv, rv);
      }

      static bool gte(const uint64_t lv, const Data & rv)
      {
        return !lt(lv, rv);
      }

      static void add(int64_t & lv, const Data & rv)
      {
        if (lv < 0)
        {
          uint64_t ulv = uint64_t( - lv);
          if (ulv > rv.ui64_)
            lv = - int64_t(ulv - rv.ui64_);
          else
            lv = int64_t(rv.ui64_ - ulv);
        }
        else
          lv = int64_t(uint64_t(lv) + rv.ui64_);
      }

      static void add(uint64_t & lv, const Data & rv)
      {
        lv += rv.ui64_;
      }

      static void sub(int64_t & lv, const Data & rv)
      {
        if (lv < 0)
          lv = - int64_t(uint64_t( - lv) + rv.ui64_);
        else
          lv = int64_t(uint64_t(lv) - rv.ui64_);
      }

      static void sub(uint64_t & lv, const Data & rv)
      {
        lv -= rv.ui64_;
      }

      static bool mul(int64_t & lv, const Data & rv)
      {
        lv *= rv.ui64_;
        return true;
      }

      static bool mul(uint64_t & lv, const Data & rv)
      {
        lv *= rv.ui64_;
        return true;
      }

      static bool div(int64_t & lv, const Data & rv)
      {
        uint64_t ui = rv.ui64_;
        if (unlikely(ui == 0))
          return false;

        if (lv < 0)
        {
          lv = -lv;
          lv /= ui;
          lv = -lv;
        }
        else
          lv /= ui;

        return true;
      }

      static bool div(uint64_t & lv, const Data & rv)
      {
        uint64_t ui = rv.i64_;
        if (unlikely(ui == 0))
          return false;
        lv /= ui;
        return true;
      }
    };

    //--------------------------------------------------------------------------
    template <>
    class Impl<detail::INTEGER> : public ImplDefault
    {
    public:
      static void Create(Dynamic & v, const int8_t i)
      {
        Reset(&v, static_cast<const int64_t>(i));
      }

      static void Create(Dynamic & v, const int16_t i)
      {
        Reset(&v, static_cast<const int64_t>(i));
      }

      static void Create(Dynamic & v, const int32_t i)
      {
        Reset(&v, static_cast<const int64_t>(i));
      }

      static void Create(Dynamic & v, const int64_t i)
      {
        Reset(&v, static_cast<const int64_t>(i));
      }

      static void Create(Dynamic & v, const uint8_t i)
      {
        Reset(&v, static_cast<const uint64_t>(i));
      }

      static void Create(Dynamic & v, const uint16_t i)
      {
        Reset(&v, static_cast<const uint64_t>(i));
      }

      static void Create(Dynamic & v, const uint32_t i)
      {
        Reset(&v, static_cast<const uint64_t>(i));
      }

      static void Create(Dynamic & v, const uint64_t i)
      {
        Reset(&v, static_cast<const uint64_t>(i));
      }

      static void OP_CLEAR(Dynamic & v) { v.Reset(); }

      static Dynamic OP_CLONE(const Data & v)
      {
        Dynamic result;
        Reset(&result, v.i64_);
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
        return v.i64_ != 0;
      }

      static bool OP_IS_EMPTY(const Data & NKIT_UNUSED(v))
      {
        return false;
      }

      static double GET_DOUBLE(const Data & v)
      {
        return static_cast<double>(v.i64_);
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
        return IntOperations<T>::lt(v.data_.i64_, rv.data_);
      }

      template<typename T>
      static bool OP_EQ(const Dynamic & v, const Dynamic & rv)
      {
        return IntOperations<T>::eq(v.data_.i64_, rv.data_);
      }

      template<typename T>
      static void OP_ADD(Data & v, const Data & rv)
      {
        IntOperations<T>::add(v.i64_, rv);
      }

      template<typename T>
      static void OP_SUB(Data & v, const Data & rv)
      {
        IntOperations<T>::sub(v.i64_, rv);
      }

      template<typename T>
      static bool OP_MUL(Data & v, const Data & rv)
      {
        return IntOperations<T>::mul(v.i64_, rv);
      }

      template<typename T>
      static bool OP_DIV(Data & v, const Data & rv)
      {
        return IntOperations<T>::div(v.i64_, rv);
      }

      template<typename T>
      static const Data & OP_MIN(const Data & v, const Data & rv)
      {
        return IntOperations<T>::lt(v.i64_, rv) ? v : rv;
      }

      template<typename T>
      static const Data & OP_MAX(const Data & v, const Data & rv)
      {
        return IntOperations<T>::gte(v.i64_, rv) ? v : rv;
      }

      static Data OP_GET_DEFAULT_DATA()
      {
        Data result;
        result.i64_ = 0;
        return result;
      }

      static Data OP_GET_MAX_DATA()
      {
        Data result;
        result.i64_ = MAX_INT64_VALUE;
        return result;
      }

      static Data OP_GET_MIN_DATA()
      {
        Data result;
        result.i64_ = MIN_INT64_VALUE;
        return result;
      }

    private:
      static void Reset(Dynamic * v, const int64_t i)
      {
        v->type_ = INTEGER;
        v->data_.i64_ = i;
      }

      static void Reset(Dynamic * v, const uint64_t ui)
      {
        v->type_ = INTEGER;
        v->data_.ui64_ = ui;
      }
    };

    //--------------------------------------------------------------------------
    template <>
    class Impl<UNSIGNED_INTEGER> : public ImplDefault
    {
    public:
      static void Create(Dynamic & v, const uint8_t i)
      {
        Reset(&v, static_cast<const uint64_t>(i));
      }

      static void Create(Dynamic & v, const uint16_t i)
      {
        Reset(&v, static_cast<const uint64_t>(i));
      }

      static void Create(Dynamic & v, const uint32_t i)
      {
        Reset(&v, static_cast<const uint64_t>(i));
      }

      static void Create(Dynamic & v, const uint64_t i)
      {
        Reset(&v, static_cast<const uint64_t>(i));
      }

      static void OP_CLEAR(Dynamic & v) { v.Reset(); }

      static Dynamic OP_CLONE(const Data & v)
      {
        Dynamic result;
        Reset(&result, v.ui64_);
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
        return cast_to_double(v.ui64_);
      }

      static std::string OP_GET_STRING(const Data & v,
          const char * NKIT_UNUSED(format))
      {
        return string_cast(v.ui64_);
      }

      static std::string OP_GET_STRING(const Dynamic & v,
          const char * format)
      {
        return OP_GET_STRING(v.data_, format);
      }

      template<typename T>
      static bool OP_LT(const Dynamic & v, const Dynamic & rv)
      {
        return IntOperations<T>::lt(v.data_.ui64_, rv.data_);
      }

      template<typename T>
      static bool OP_EQ(const Dynamic & v, const Dynamic & rv)
      {
        return IntOperations<T>::eq(v.data_.ui64_, rv.data_);
      }

      template<typename T>
      static void OP_ADD(Data & v, const Data & rv)
      {
        IntOperations<T>::add(v.ui64_, rv);
      }

      template<typename T>
      static void OP_SUB(Data & v, const Data & rv)
      {
        IntOperations<T>::sub(v.ui64_, rv);
      }

      template<typename T>
      static bool OP_MUL(Data & v, const Data & rv)
      {
        return IntOperations<T>::mul(v.ui64_, rv);
      }

      template<typename T>
      static bool OP_DIV(Data & v, const Data & rv)
      {
        return IntOperations<T>::div(v.ui64_, rv);
      }

      template<typename T>
      static const Data & OP_MIN(const Data & v, const Data & rv)
      {
        return IntOperations<T>::lt(v.ui64_, rv) ? v : rv;
      }

      template<typename T>
      static const Data & OP_MAX(const Data & v, const Data & rv)
      {
        return IntOperations<T>::gte(v.ui64_, rv) ? v : rv;
      }

      static Data OP_GET_DEFAULT_DATA()
      {
        return OP_GET_MIN_DATA();
      }

      static Data OP_GET_MAX_DATA()
      {
        Data result;
        result.ui64_ = MAX_UINT64_VALUE;
        return result;
      }

      static Data OP_GET_MIN_DATA()
      {
        Data result;
        result.ui64_ = 0;
        return result;
      }

    private:
      static void Reset(Dynamic * v, const uint64_t ui)
      {
        v->type_ = UNSIGNED_INTEGER;
        v->data_.ui64_ = ui;
      }
    };
  } // namespace detail
} // namespace nkit

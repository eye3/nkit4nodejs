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

#ifndef __NKIT__DYNAMIC__IMPL__H__
#define __NKIT__DYNAMIC__IMPL__H__

namespace nkit
{
  namespace detail
  {
    enum OpCode
    {
      /***********************************************************
       * Binary opcodes
       **********************************************************/
      OP_EQ = 0,
      OP_LT,
      OP_ADD,
      OP_SUB,
      OP_MUL,
      OP_DIV,
      OP_MIN,
      OP_MAX,

      /***********************************************************
       * !!! Binary opcodes goes before this marker !!!
       **********************************************************/
      BINARY_UNARY_DELIMITER,

      /***********************************************************
       * Unary opcodes
       **********************************************************/
      OP_GET_INT,
      OP_GET_UINT,
      OP_GET_BOOL,
      OP_GET_STRING,
      OP_GET_CONST_STRING,
      OP_GET_FLOAT,
      OP_DEC_REF_DYNAMIC,
      OP_INC_REF_DATA,
      OP_DEC_REF_DATA,
      OP_IS_EMPTY,
      OP_GET_SIZE,
      OP_CLEAR,
      OP_CLONE,
      OP_GET_DEFAULT_DATA,
      OP_GET_MAX_DATA,
      OP_GET_MIN_DATA,

      /***********************************************************
       * !!! End marker. IT MUST BE LAST IN THIS ENUM !!!
       **********************************************************/
      OPERATIONS_COUNT
    };

    //--------------------------------------------------------------------------
    template <uint64_t opcode, typename Runner>
    struct UnaryOperation
    {
      static Runner farray[detail::DYNAMIC_TYPES_COUNT];
    };

    template <uint64_t opcode, typename Runner>
    Runner UnaryOperation<opcode, Runner>::farray[detail::DYNAMIC_TYPES_COUNT];

    template <uint64_t opcode, typename Runner>
    struct BinaryOperation
    {
      static Runner farray[detail::DYNAMIC_TYPES_COUNT]
                           [detail::DYNAMIC_TYPES_COUNT];
    };

    template <uint64_t opcode, typename Runner>
      Runner BinaryOperation<opcode, Runner>::farray
        [detail::DYNAMIC_TYPES_COUNT][detail::DYNAMIC_TYPES_COUNT];

    template <uint64_t opcode>
    struct Operation;

    /***********************************************************
     * Binary operations
     **********************************************************/
    template <>
    struct Operation<OP_EQ> :
      BinaryOperation<OP_EQ, bool (*)(const Dynamic & lv, const Dynamic & rv)>
    {
      template <uint64_t ltype, uint64_t rtype>
      static bool Run(const Dynamic & lv, const Dynamic & rv)
      {
        return Impl<ltype>::template OP_EQ<Impl<rtype> >(lv, rv);
      }
    };

    template <>
    struct Operation<OP_LT> :
      BinaryOperation<OP_LT, bool (*)(const Dynamic & lv, const Dynamic & rv)>
    {
      template <uint64_t ltype, uint64_t rtype>
      static bool Run(const Dynamic & lv, const Dynamic & rv)
      {
        return Impl<ltype>::template OP_LT<Impl<rtype> >(lv, rv);
      }
    };

    template <>
    struct Operation<OP_ADD> :
      BinaryOperation<OP_ADD, void (*)(Data &, const Data &)>
    {
      template <uint64_t ltype, uint64_t rtype>
      static void Run(Data & lv, const Data & rv)
      {
        Impl<ltype>::template OP_ADD<Impl<rtype> >(lv, rv);
      }
    };

    template <>
    struct Operation<OP_SUB> :
      BinaryOperation<OP_SUB, void (*)(Data & lv, const Data & rv)>
    {
      template <uint64_t ltype, uint64_t rtype>
      static void Run(Data & lv, const Data & rv)
      {
        Impl<ltype>::template OP_SUB<Impl<rtype> >(lv, rv);
      }
    };

    template <>
    struct Operation<OP_MUL> :
      BinaryOperation<OP_MUL, bool (*)(Data & lv, const Data & rv)>
    {
      template <uint64_t ltype, uint64_t rtype>
      static bool Run(Data & lv, const Data & rv)
      {
        return Impl<ltype>::template OP_MUL<Impl<rtype> >(lv, rv);
      }
    };

    template <>
    struct Operation<OP_DIV> :
      BinaryOperation<OP_DIV, bool (*)(Data & lv, const Data & rv)>
    {
      template <uint64_t ltype, uint64_t rtype>
      static bool Run(Data & lv, const Data & rv)
      {
        return Impl<ltype>::template OP_DIV<Impl<rtype> >(lv, rv);
      }
    };

    template <>
    struct Operation<OP_MIN> :
      BinaryOperation<OP_MIN, const Data & (*)(const Data & lv,
          const Data & rv)>
    {
      template <uint64_t ltype, uint64_t rtype>
      static const Data & Run(const Data & lv, const Data & rv)
      {
        return Impl<ltype>::template OP_MIN<Impl<rtype> >(lv, rv);
      }
    };

    template <>
    struct Operation<OP_MAX> :
      BinaryOperation<OP_MAX, const Data & (*)(const Data & lv,
          const Data & rv)>
    {
      template <uint64_t ltype, uint64_t rtype>
      static const Data & Run(const Data & lv, const Data & rv)
      {
        return Impl<ltype>::template OP_MAX<Impl<rtype> >(lv, rv);
      }
    };

    /***********************************************************
     * Unary operations
     **********************************************************/
    template <>
    struct Operation<OP_GET_INT> :
      UnaryOperation<OP_GET_INT, int64_t (*)(const Data &)>
    {
      template <uint64_t TypeCode>
      static int64_t Run(const Data & v)
      {
        return Impl<TypeCode>::OP_GET_INT(v);
      }
    };

    template <>
    struct Operation<OP_GET_UINT> :
      UnaryOperation<OP_GET_UINT, uint64_t (*)(const Data & )>
    {
      template <uint64_t TypeCode>
      static uint64_t Run(const Data & v)
      {
        return Impl<TypeCode>::OP_GET_UINT(v);
      }
    };

    template <>
    struct Operation<OP_GET_BOOL> :
      UnaryOperation<OP_GET_BOOL, bool (*)(const Data & )>
    {
      template <uint64_t TypeCode>
      static bool Run(const Data & v)
      {
        return Impl<TypeCode>::OP_GET_BOOL(v);
      }
    };

    template <>
    struct Operation<OP_IS_EMPTY> :
      UnaryOperation<OP_IS_EMPTY, bool (*)(const Data & )>
    {
      template <uint64_t TypeCode>
      static bool Run(const Data & v)
      {
        return Impl<TypeCode>::OP_IS_EMPTY(v);
      }
    };

    template <>
    struct Operation<OP_GET_SIZE> :
      UnaryOperation<OP_GET_SIZE, size_t (*)(const Data & )>
    {
      template <uint64_t TypeCode>
      static size_t Run(const Data & v)
      {
        return Impl<TypeCode>::OP_GET_SIZE(v);
      }
    };

    template <>
    struct Operation<OP_GET_FLOAT> :
      UnaryOperation<OP_GET_FLOAT, double (*)(const Data &)>
    {
      template <uint64_t TypeCode>
      static double Run(const Data & v)
      {
        return Impl<TypeCode>::GET_DOUBLE(v);
      }
    };

    template <>
    struct Operation<OP_GET_STRING> :
      UnaryOperation<OP_GET_STRING,
        std::string (*)(const Data &, const char*)>
    {
      template <uint64_t TypeCode>
      static std::string Run(const Data & v, const char * format)
      {
        return Impl<TypeCode>::OP_GET_STRING(v, format);
      }
    };

    template <>
    struct Operation<OP_GET_CONST_STRING> :
      UnaryOperation<OP_GET_CONST_STRING,
        const std::string & (*)(const Data &)>
    {
      template <uint64_t TypeCode>
      static const std::string & Run(const Data & v)
      {
        return Impl<TypeCode>::OP_GET_CONST_STRING(v);
      }
    };

    template <>
    struct Operation<OP_DEC_REF_DYNAMIC> :
      UnaryOperation<OP_DEC_REF_DYNAMIC, void (*)(Dynamic &)>
    {
      template <uint64_t TypeCode>
      static void Run(Dynamic & to)
      {
        Impl<TypeCode>::OP_DEC_REF_DYNAMIC(to);
      }
    };

    template <>
    struct Operation<OP_INC_REF_DATA> :
      UnaryOperation<OP_INC_REF_DATA, void (*)(Data &)>
    {
      template <uint64_t TypeCode>
      static void Run(Data & to)
      {
        Impl<TypeCode>::OP_INC_REF_DATA(to);
      }
    };

    template <>
    struct Operation<OP_DEC_REF_DATA> :
      UnaryOperation<OP_DEC_REF_DATA, void (*)(Data &)>
    {
      template <uint64_t TypeCode>
      static void Run(Data & to)
      {
        Impl<TypeCode>::OP_DEC_REF_DATA(to);
      }
    };

    template <>
    struct Operation<OP_CLEAR> :
      UnaryOperation<OP_CLEAR, void (*)(Dynamic &)>
    {
      template <uint64_t TypeCode>
      static void Run(Dynamic & v)
      {
        return Impl<TypeCode>::OP_CLEAR(v);
      }
    };

    template <>
    struct Operation<OP_CLONE> :
      UnaryOperation<OP_CLONE, Dynamic (*)(const Data &)>
    {
      template <uint64_t TypeCode>
      static Dynamic Run(const Data & v)
      {
        return Impl<TypeCode>::OP_CLONE(v);
      }
    };

    template <>
    struct Operation<OP_GET_DEFAULT_DATA> :
      UnaryOperation<OP_GET_DEFAULT_DATA, Data (*)()>
    {
      template <uint64_t TypeCode>
      static Data Run()
      {
        return Impl<TypeCode>::OP_GET_DEFAULT_DATA();
      }
    };

    template <>
    struct Operation<OP_GET_MAX_DATA> :
      UnaryOperation<OP_GET_MAX_DATA, Data (*)()>
    {
      template <uint64_t TypeCode>
      static Data Run()
      {
        return Impl<TypeCode>::OP_GET_MAX_DATA();
      }
    };

    template <>
    struct Operation<OP_GET_MIN_DATA> :
      UnaryOperation<OP_GET_MIN_DATA, Data (*)()>
    {
      template <uint64_t TypeCode>
      static Data Run()
      {
        return Impl<TypeCode>::OP_GET_MIN_DATA();
      }
    };
    /***********************************************************
     * END operations
     **********************************************************/

    /***********************************************************
     * Type implementations
     **********************************************************/
    class ImplDefault
    {
    public:
      static int64_t OP_GET_INT(const Data & NKIT_UNUSED(v))
      {
        return 0;
      }

      static uint64_t OP_GET_UINT(const Data & NKIT_UNUSED(v))
      {
        return 0;
      }

      static bool OP_GET_BOOL(const Data & NKIT_UNUSED(v))
      {
        return false;
      }

      static double GET_DOUBLE(const Data & NKIT_UNUSED(v))
      {
        return 0.0;
      }

      static const std::string & OP_GET_CONST_STRING(
          const Data & NKIT_UNUSED(v))
      {
        return S_EMPTY_;
      }

      static const std::string & OP_GET_CONST_STRING(const Dynamic & v)
      {
        return OP_GET_CONST_STRING(v.data_);
      }

      static std::string OP_GET_STRING(const Data & v,
        const char * NKIT_UNUSED(format))
      {
        return OP_GET_CONST_STRING(v);
      }

      static std::string OP_GET_STRING(const Dynamic & v,
        const char * format)
      {
        return OP_GET_STRING(v.data_, format);
      }

      static size_t OP_GET_SIZE(const Data & NKIT_UNUSED(v))
      {
        return 1;
      }

      static bool OP_IS_EMPTY(const Data & NKIT_UNUSED(v))
      {
        return true;
      }

      template<typename T>
      static bool OP_LT(const Dynamic & v, const Dynamic & rv)
      {
        return v.type_ < rv.type_;
      }

      template<typename T>
      static bool OP_EQ(const Dynamic & v, const Dynamic & rv)
      {
        return v.type_ == rv.type_;
      }

      template<typename T>
      static void OP_ADD(Data & NKIT_UNUSED(v), const Data & NKIT_UNUSED(rv))
      {
      }

      template<typename T>
      static void OP_SUB(Data & NKIT_UNUSED(v), const Data & NKIT_UNUSED(rv))
      {
      }

      template<typename T>
      static bool OP_MUL(Data & NKIT_UNUSED(v), const Data & NKIT_UNUSED(rv))
      {
        return true;
      }

      template<typename T>
      static bool OP_DIV(Data & NKIT_UNUSED(v), const Data & NKIT_UNUSED(rv))
      {
        return true;
      }

      template<typename T>
      static const Data & OP_MIN(const Data & v, const Data & NKIT_UNUSED(rv))
      {
        return v;
      }

      template<typename T>
      static const Data & OP_MAX(const Data & v, const Data & NKIT_UNUSED(rv))
      {
        return v;
      }

      static Data OP_GET_DEFAULT_DATA()
      {
        abort_with_core("Not implemented"); // we should not reach this place
        return OP_GET_MIN_DATA();
      }

      static Data OP_GET_MAX_DATA()
      {
        abort_with_core("Not implemented"); // we should not reach this place
        Data result;
        result.i64_ = 0;
        return result;
      }

      static Data OP_GET_MIN_DATA()
      {
        abort_with_core("Not implemented"); // we should not reach this place
        Data result;
        result.i64_ = 0;
        return result;
      }
    };

    //--------------------------------------------------------------------------
    template <>
    class Impl<detail::UNDEF> : public ImplDefault
    {
    public:
      static void Create(Dynamic & v)
      {
        v.Reset();
      }

      static Dynamic OP_CLONE(const Data & NKIT_UNUSED(v))
      {
        return Dynamic();
      }

      static size_t OP_GET_SIZE(const Data & NKIT_UNUSED(v))
      {
        return 0;
      }

      static void OP_CLEAR(Dynamic & NKIT_UNUSED(to)) {}
      static void OP_DEC_REF_DYNAMIC(Dynamic & ) {}
      static void OP_INC_REF_DATA(Data & ) {}
      static void OP_DEC_REF_DATA(Data & ) {}
    };

    //--------------------------------------------------------------------------
    template <>
    class Impl<detail::NONE> : public ImplDefault
    {
    public:
      static void Create(Dynamic & v)
      {
        Reset(&v);
      }

      static size_t OP_GET_SIZE(const Data & NKIT_UNUSED(v))
      {
        return 0;
      }

      static Dynamic OP_CLONE(const Data & NKIT_UNUSED(v))
      {
        return D_NONE;
      }

      static void OP_CLEAR(Dynamic & v) { v.Reset(); }
      static void OP_DEC_REF_DYNAMIC(Dynamic & ) {}
      static void OP_INC_REF_DATA(Data & ) {}
      static void OP_DEC_REF_DATA(Data & ) {}

    private:
      static void Reset(Dynamic * v)
      {
        v->type_ = NONE;
        v->data_.i64_ = 0;
      }
    };

    //--------------------------------------------------------------------------
    class RefCounted
    {
    public:
      RefCounted() : refcount_(1) {}
      ~RefCounted() { assert(refcount_ == 0); } // non-virtual dtor
      size_t IncRef() { assert(refcount_ > 0); ++refcount_; return refcount_; }
      size_t DecRef() { assert(refcount_ > 0); --refcount_; return refcount_; }
      size_t ref_count() const { return refcount_; }
    private:
      size_t refcount_;
    };

    //--------------------------------------------------------------------------
    template<typename T>
    class Shared : public RefCounted
    {
    public:
      Shared() {}
      Shared(const T & v) : value_(v) {}
      template<typename P1>
      Shared(P1 p1) : value_(p1) {}
      template<typename P1, typename P2>
      Shared(P1 p1, P2 p2) : value_(p1, p2) {}
      ~Shared() {} // non-virtual dtor
      const T * GetPtr() const { return &value_; }
      T * GetPtr() { return &value_; }
      const T & GetRef() const { return value_; }
      T & GetRef() { return value_; }
      void Set(T * new_value) { value_ = *new_value; }
      void Set(const T & new_value) { value_ = new_value; }

    private:
      Shared(const Shared&);
      Shared & operator =(const Shared&);

    private:
      T value_;
    };
  } // namespace detail
} // namespace nkit

#include <nkit/dynamic/impl_bool.h>
#include <nkit/dynamic/impl_integer.h>
#include <nkit/dynamic/impl_float.h>
#include <nkit/dynamic/impl_date_time.h>
#include <nkit/dynamic/impl_string.h>
#include <nkit/dynamic/impl_list.h>
#include <nkit/dynamic/impl_dict.h>
#include <nkit/dynamic/impl_table.h>

#endif // __NKIT__DYNAMIC__IMPL__H__

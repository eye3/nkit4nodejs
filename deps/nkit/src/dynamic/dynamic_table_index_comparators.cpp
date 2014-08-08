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

#ifndef __NKIT__TABLE__INDEX__COMPARE__H__
#define __NKIT__TABLE__INDEX__COMPARE__H__

#include "nkit/dynamic.h"
#include "nkit/logger_brief.h"

namespace nkit
{
  namespace detail
  {
    template<int T>
    struct KeyItemComparator;

    template<>
    struct KeyItemComparator<INTEGER>
    {
      static inline int64_t Compare(const KeyItem & a1, const KeyItem & a2)
      {
        return (a1.i64_ < a2.i64_) ? -1 : (a1.i64_ > a2.i64_ ? 1 : 0);
      }
    };

    template<>
    struct KeyItemComparator<-INTEGER>
    {
      static inline int64_t Compare(const KeyItem & a1, const KeyItem & a2)
      {
        return (a1.i64_ > a2.i64_) ? -1 : (a1.i64_ < a2.i64_ ? 1 : 0);
      }
    };

    template<>
    struct KeyItemComparator<UNSIGNED_INTEGER>
    {
      static inline int64_t Compare(const KeyItem & a1, const KeyItem & a2)
      {
        return (a1.ui64_ < a2.ui64_) ? -1 : (a1.ui64_ > a2.ui64_ ? 1 : 0);
      }
    };

    template<>
    struct KeyItemComparator<-UNSIGNED_INTEGER>
    {
      static inline int64_t Compare(const KeyItem & a1, const KeyItem & a2)
      {
        return (a1.ui64_ > a2.ui64_) ? -1 : (a1.ui64_ < a2.ui64_ ? 1 : 0);
      }
    };

    template<>
    struct KeyItemComparator<DATE_TIME>
    {
      static inline int64_t Compare(const KeyItem & a1, const KeyItem & a2)
      {
        return (a1.ui64_ < a2.ui64_) ? -1 : (a1.ui64_ > a2.ui64_ ? 1 : 0);
      }
    };

    template<>
    struct KeyItemComparator<-DATE_TIME>
    {
      static inline int64_t Compare(const KeyItem & a1, const KeyItem & a2)
      {
        return (a1.ui64_ > a2.ui64_) ? -1 : (a1.ui64_ < a2.ui64_ ? 1 : 0);
      }
    };

    template<>
    struct KeyItemComparator<FLOAT>
    {
      static inline int64_t Compare(const KeyItem & a1, const KeyItem & a2)
      {
        return (a1.f_ < a2.f_) ? -1 : (a1.f_ > a2.f_ ? 1 : 0);
      }
    };

    template<>
    struct KeyItemComparator<-FLOAT>
    {
      static inline int64_t Compare(const KeyItem & a1, const KeyItem & a2)
      {
        return (a1.f_ > a2.f_) ? -1 : (a1.f_ < a2.f_ ? 1 : 0);
      }
    };

    template<>
    struct KeyItemComparator<STRING>
    {
      static inline int64_t Compare(const KeyItem & a1, const KeyItem & a2)
      {
        return a1.shared_string_->GetRef().compare(
            a2.shared_string_->GetRef());
      }
    };

    template<>
    struct KeyItemComparator<-STRING>
    {
      static inline int64_t Compare(const KeyItem & a1, const KeyItem & a2)
      {
        return - a1.shared_string_->GetRef().compare(
            a2.shared_string_->GetRef());
      }
    };

    //--------------------------------------------------------------------------
    template <int T1>
    struct Comparator1
    {
      static int64_t Compare(const KeyItem * a1, const KeyItem * a2)
      {
        return KeyItemComparator<T1>::Compare(a1[0], a2[0]);
      }
    };

    template <int T1, int T2>
    struct Comparator2
    {
      inline static int64_t Compare(const KeyItem * a1, const KeyItem * a2)
      {
        int64_t result = KeyItemComparator<T1>::Compare(a1[0], a2[0]);
        result || (result = KeyItemComparator<T2>::Compare(a1[1], a2[1]));
        return result;
      }
    };

    template <int T1, int T2, int T3>
    struct Comparator3
    {
      inline static int64_t Compare(const KeyItem * a1, const KeyItem * a2)
      {
        int64_t result = KeyItemComparator<T1>::Compare(a1[0], a2[0]);
        result || (result = KeyItemComparator<T2>::Compare(a1[1], a2[1]));
        result || (result = KeyItemComparator<T3>::Compare(a1[2], a2[2]));
        return result;
      }
    };

    template <int T1, int T2, int T3, int T4>
    struct Comparator4
    {
      inline static int64_t Compare(const KeyItem * a1, const KeyItem * a2)
      {
        int64_t result = KeyItemComparator<T1>::Compare(a1[0], a2[0]);
        result || (result = KeyItemComparator<T2>::Compare(a1[1], a2[1]));
        result || (result = KeyItemComparator<T3>::Compare(a1[2], a2[2]));
        result || (result = KeyItemComparator<T4>::Compare(a1[3], a2[3]));
        return result;
      }
    };

    //--------------------------------------------------------------------------
    typedef std::map<std::string, KeyCompare > COMPARATORS_MAP;

    COMPARATORS_MAP & GetComparatorsMap()
    {
      static COMPARATORS_MAP cmp_map;
      return cmp_map;
    }

    //--------------------------------------------------------------------------
    bool GetComparator(const StringVector & mask, IndexCompare * result,
        std::string * error)
    {
      size_t full_parts_count = mask.size() / NKIT_TABLE_INDEX_PART_SIZE;
      size_t last_part_size = mask.size() % NKIT_TABLE_INDEX_PART_SIZE;
      COMPARATORS_MAP & map = GetComparatorsMap();
      std::string sub_mask;
      size_t i = 0;
      for (; i < full_parts_count; ++i)
      {
        sub_mask.clear();
        for (size_t p = 0; p < NKIT_TABLE_INDEX_PART_SIZE; ++p)
          sub_mask += mask[i*NKIT_TABLE_INDEX_PART_SIZE + p];
        COMPARATORS_MAP::const_iterator it = map.find(sub_mask);
        if (it == map.end() || !result->AppendPartCompare(it->second))
        {
          *error = "Comparator does not defined for mask '" + sub_mask + "'";
          return false;
        }
      }

      if (last_part_size)
      {
        sub_mask.clear();
        for (size_t p = 0; p < last_part_size; ++p)
          sub_mask += mask[i*NKIT_TABLE_INDEX_PART_SIZE + p];
        COMPARATORS_MAP::const_iterator it = map.find(sub_mask);
        if (it == map.end() || !result->AppendPartCompare(it->second))
        {
          *error = "Comparator does not defined for mask '" + sub_mask + "'";
          return false;
        }
      }

      return true;
    }

    //--------------------------------------------------------------------------
    template <int v>
    struct IndexedDynamicType
    {
      static const uint64_t type = STRING;
      typedef IndexedDynamicType<FLOAT> next;
    };

    template <>
    struct IndexedDynamicType<FLOAT>
    {
      static const uint64_t type = FLOAT;
      typedef IndexedDynamicType<UNSIGNED_INTEGER> next;
    };

    template <>
    struct IndexedDynamicType<UNSIGNED_INTEGER>
    {
      static const uint64_t type = UNSIGNED_INTEGER;
      typedef IndexedDynamicType<INTEGER> next;
    };

    template <>
    struct IndexedDynamicType<INTEGER>
    {
      static const uint64_t type = INTEGER;
      typedef IndexedDynamicType<UNDEF> next;
    };

    template <>
    struct IndexedDynamicType<UNDEF>
    {
      static const uint64_t type = UNDEF;
    };

    //--------------------------------------------------------------------------
    const std::string & v2s(int v)
    {
      static const std::string TYPE_NUMS[DYNAMIC_TYPES_COUNT] =
        {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"};

      return TYPE_NUMS[v];
    }

    //--------------------------------------------------------------------------
    struct EmptyCompareFunctionSetter
    {
      static void set(COMPARATORS_MAP &) {}
      static void _set2(COMPARATORS_MAP & ) {}
      static void _set3(COMPARATORS_MAP & ) {}
      static void _set4(COMPARATORS_MAP & ) {}
      static void _set23(COMPARATORS_MAP & ) {}
      static void _set24(COMPARATORS_MAP & ) {}
      static void _set34(COMPARATORS_MAP & ) {}
    };

    void set_map(COMPARATORS_MAP & map, const std::string & key, KeyCompare f)
    {
      COMPARATORS_MAP::const_iterator it = map.find(key);
      if (it != map.end())
        abort_with_core("!!! " + key + " !!!");

      map[key] = f;
    }

    //--------------------------------------------------------------------------
    template <int v>
    struct CompareFunctionSetter1
    {
      static const int v_next = IndexedDynamicType<v>::next::type;

      static void set(COMPARATORS_MAP & map)
      {
        set_map(map, v2s(v), Comparator1<v>::Compare);
        set_map(map, "-" + v2s(v), Comparator1<-v>::Compare);
        CompareFunctionSetter1<v_next>::set(map);
      }
    };

    template <>
    struct CompareFunctionSetter1<INTEGER>
    {
      static void set(COMPARATORS_MAP & map)
      {
        set_map(map, "1", Comparator1<INTEGER>::Compare);
        set_map(map, "-1", Comparator1<-INTEGER>::Compare);
      }
    };

//------------------------------------------------------------------------------
#if NKIT_TABLE_INDEX_PART_SIZE >= 2
    template <int v1, int v2>
    struct CompareFunctionSetter2
    {
      static const int v1_next = IndexedDynamicType<v1>::next::type;
      static const int v2_next = IndexedDynamicType<v2>::next::type;

      static void set(COMPARATORS_MAP & map)
      {
        _set(map);
        CompareFunctionSetter2<v1_next, v2>::set(map);
        CompareFunctionSetter2<v1, v2_next>::_set2(map);
      }

      static void _set2(COMPARATORS_MAP & map)
      {
        _set(map);
        CompareFunctionSetter2<v1, v2_next>::_set2(map);
      }

      static void _set(COMPARATORS_MAP & map)
      {
        set_map(map, v2s(v1) + v2s(v2), Comparator2<v1, v2>::Compare);
        set_map(map, v2s(v1) + "-" + v2s(v2), Comparator2<v1, -v2>::Compare);
        set_map(map, "-" + v2s(v1) + v2s(v2), Comparator2<-v1, v2>::Compare);
        set_map(map, "-" + v2s(v1) + "-" + v2s(v2),
            Comparator2<-v1, -v2>::Compare);

      }
    };

    template<int v2>
    struct CompareFunctionSetter2<UNDEF, v2> :
      public EmptyCompareFunctionSetter {};

    template <int v1>
    struct CompareFunctionSetter2<v1, UNDEF> :
      public EmptyCompareFunctionSetter {};

    template <>
    struct CompareFunctionSetter2<INTEGER, INTEGER> :
      public EmptyCompareFunctionSetter
    {
      static void _set2(COMPARATORS_MAP & map)
      {
        set_map(map, "11", Comparator2<1, 1>::Compare);
        set_map(map, "-11", Comparator2<-1, 1>::Compare);
        set_map(map, "1-1", Comparator2<1, -1>::Compare);
        set_map(map, "-1-1", Comparator2<-1, -1>::Compare);
      }
    };

#endif // NKIT_TABLE_INDEX_PART_SIZE >= 2

//------------------------------------------------------------------------------
#if NKIT_TABLE_INDEX_PART_SIZE >= 3
    template <int v1, int v2, int v3>
    struct CompareFunctionSetter3
    {
      static const int v1_next = IndexedDynamicType<v1>::next::type;
      static const int v2_next = IndexedDynamicType<v2>::next::type;
      static const int v3_next = IndexedDynamicType<v3>::next::type;

      static void set(COMPARATORS_MAP & map)
      {
        _set(map);
        CompareFunctionSetter3<v1_next, v2, v3>::set(map);
        CompareFunctionSetter3<v1, v2_next, v3>::_set2(map);
        CompareFunctionSetter3<v1, v2, v3_next>::_set3(map);
      }

      static void _set2(COMPARATORS_MAP & map)
      {
        _set(map);
        CompareFunctionSetter3<v1, v2_next, v3>::_set2(map);
        CompareFunctionSetter3<v1, v2, v3_next>::_set3(map);
      }

      static void _set3(COMPARATORS_MAP & map)
      {
        _set(map);
        CompareFunctionSetter3<v1, v2, v3_next>::_set3(map);
      }

      static void _set(COMPARATORS_MAP & map)
      {
        set_map(map,
            v2s(v1) +
            v2s(v2) +
            v2s(v3),
            Comparator3<v1, v2, v3>::Compare);

        set_map(map,
            v2s(v1) +
            "-" + v2s(v2) +
            v2s(v3),
            Comparator3<v1, -v2, v3>::Compare);

        set_map(map,
            "-" + v2s(v1) +
            v2s(v2) +
            v2s(v3),
            Comparator3<-v1, v2, v3>::Compare);

        set_map(map,
            "-" + v2s(v1) +
            "-" + v2s(v2) +
            v2s(v3),
            Comparator3<-v1, -v2, v3>::Compare);

        set_map(map,
            v2s(v1) +
            v2s(v2) +
            "-" + v2s(v3),
            Comparator3<v1, v2, -v3>::Compare);

        set_map(map,
            v2s(v1) +
            "-" + v2s(v2) +
            "-" + v2s(v3),
            Comparator3<v1, -v2, -v3>::Compare);

        set_map(map,
            "-" + v2s(v1) +
            v2s(v2) +
            "-" + v2s(v3),
            Comparator3<-v1, v2, -v3>::Compare);

        set_map(map,
            "-" + v2s(v1) +
            "-" + v2s(v2) +
            "-" + v2s(v3),
            Comparator3<-v1, -v2, -v3>::Compare);
      }
    };

    template <>
    struct CompareFunctionSetter3<INTEGER, INTEGER, INTEGER>:
      public EmptyCompareFunctionSetter
    {
      static void _set3(COMPARATORS_MAP & map)
      {
        set_map(map, "111", Comparator3<1, 1, 1>::Compare);
        set_map(map, "1-11", Comparator3<1, -1, 1>::Compare);
        set_map(map, "-111", Comparator3<-1, 1, 1>::Compare);
        set_map(map, "-1-11", Comparator3<-1, -1, 1>::Compare);
        set_map(map, "11-1", Comparator3<1, 1, -1>::Compare);
        set_map(map, "1-1-1", Comparator3<1, -1, -1>::Compare);
        set_map(map, "-11-1", Comparator3<-1, 1, -1>::Compare);
        set_map(map, "-1-1-1", Comparator3<-1, -1, -1>::Compare);
      }
    };

    template <int v2, int v3>
    struct CompareFunctionSetter3<UNDEF, v2, v3> :
      public EmptyCompareFunctionSetter {};

    template <int v1, int v3>
    struct CompareFunctionSetter3<v1, UNDEF, v3> :
      public EmptyCompareFunctionSetter {};

    template <int v3>
    struct CompareFunctionSetter3<UNDEF, UNDEF, v3> :
      public EmptyCompareFunctionSetter {};

    template <int v1, int v2>
    struct CompareFunctionSetter3<v1, v2, UNDEF> :
      public EmptyCompareFunctionSetter {};

    template <int v2>
    struct CompareFunctionSetter3<UNDEF, v2, UNDEF> :
      public EmptyCompareFunctionSetter {};

    template <int v1>
    struct CompareFunctionSetter3<v1, UNDEF, UNDEF> :
      public EmptyCompareFunctionSetter {};

    template <>
    struct CompareFunctionSetter3<UNDEF, UNDEF, UNDEF> :
      public EmptyCompareFunctionSetter {};

#endif // NKIT_TABLE_INDEX_PART_SIZE >= 3

//------------------------------------------------------------------------------
#if NKIT_TABLE_INDEX_PART_SIZE >= 4
    template <int v1, int v2, int v3, int v4>
    struct CompareFunctionSetter4
    {
      static const int v1_next = IndexedDynamicType<v1>::next::type;
      static const int v2_next = IndexedDynamicType<v2>::next::type;
      static const int v3_next = IndexedDynamicType<v3>::next::type;
      static const int v4_next = IndexedDynamicType<v4>::next::type;

      static void set(COMPARATORS_MAP & map)
      {
        _set(map);
        CompareFunctionSetter4<v1_next, v2, v3, v4>::set(map);
        CompareFunctionSetter4<v1, v2_next, v3, v4>::_set2(map);
        CompareFunctionSetter4<v1, v2, v3_next, v4>::_set3(map);
        CompareFunctionSetter4<v1, v2, v3, v4_next>::_set4(map);
      }

      static void _set2(COMPARATORS_MAP & map)
      {
        _set(map);
        CompareFunctionSetter4<v1, v2_next, v3, v4>::_set2(map);
        CompareFunctionSetter4<v1, v2, v3_next, v4>::_set3(map);
        CompareFunctionSetter4<v1, v2, v3, v4_next>::_set4(map);
      }

      static void _set3(COMPARATORS_MAP & map)
      {
        _set(map);
        CompareFunctionSetter4<v1, v2, v3_next, v4>::_set3(map);
        CompareFunctionSetter4<v1, v2, v3, v4_next>::_set4(map);
      }

      static void _set4(COMPARATORS_MAP & map)
      {
        _set(map);
        CompareFunctionSetter4<v1, v2, v3, v4_next>::_set4(map);
      }

      static void _set(COMPARATORS_MAP & map)
      {
        set_map(map, v2s(v1) +
            v2s(v2) +
            v2s(v3) +
            v2s(v4),
            Comparator4<v1, v2, v3, v4>::Compare);

        set_map(map, v2s(v1) +
            "-" + v2s(v2) +
            v2s(v3) +
            v2s(v4),
            Comparator4<v1, -v2, v3, v4>::Compare);

        set_map(map, "-" + v2s(v1) +
            v2s(v2) +
            v2s(v3) +
            v2s(v4),
            Comparator4<-v1, v2, v3, v4>::Compare);

        set_map(map, "-" + v2s(v1) +
            "-" + v2s(v2) +
            v2s(v3) +
            v2s(v4),
            Comparator4<-v1, -v2, v3, v4>::Compare);

        set_map(map, v2s(v1) +
            v2s(v2) +
            "-" + v2s(v3) +
            v2s(v4),
            Comparator4<v1, v2, -v3, v4>::Compare);

        set_map(map, v2s(v1) +
            "-" + v2s(v2) +
            "-" + v2s(v3) +
            v2s(v4),
            Comparator4<v1, -v2, -v3, v4>::Compare);

        set_map(map, "-" + v2s(v1) +
            v2s(v2) +
            "-" + v2s(v3) +
            v2s(v4),
            Comparator4<-v1, v2, -v3, v4>::Compare);

        set_map(map, "-" + v2s(v1) +
            "-" + v2s(v2) +
            "-" + v2s(v3) +
            v2s(v4),
            Comparator4<-v1, -v2, -v3, v4>::Compare);

        set_map(map, v2s(v1) +
            v2s(v2) +
            v2s(v3) +
            "-" + v2s(v4),
            Comparator4<v1, v2, v3, -v4>::Compare);

        set_map(map, v2s(v1) +
            "-" + v2s(v2) +
            v2s(v3) +
            "-" + v2s(v4),
            Comparator4<v1, -v2, v3, -v4>::Compare);

        set_map(map, "-" + v2s(v1) +
            v2s(v2) +
            v2s(v3) +
            "-" + v2s(v4),
            Comparator4<-v1, v2, v3, -v4>::Compare);

        set_map(map, "-" + v2s(v1) +
            "-" + v2s(v2) +
            v2s(v3) +
            "-" + v2s(v4),
            Comparator4<-v1, -v2, v3, -v4>::Compare);

        set_map(map, v2s(v1) +
            v2s(v2) +
            "-" + v2s(v3) +
            "-" + v2s(v4),
            Comparator4<v1, v2, -v3, -v4>::Compare);

        set_map(map, v2s(v1) +
            "-" + v2s(v2) +
            "-" + v2s(v3) +
            "-" + v2s(v4),
            Comparator4<v1, -v2, -v3, -v4>::Compare);

        set_map(map, "-" + v2s(v1) +
            v2s(v2) +
            "-" + v2s(v3) +
            "-" + v2s(v4),
            Comparator4<-v1, v2, -v3, -v4>::Compare);

        set_map(map, "-" + v2s(v1) +
            "-" + v2s(v2) +
            "-" + v2s(v3) +
            "-" + v2s(v4),
            Comparator4<-v1, -v2, -v3, -v4>::Compare);
      }
    };

    template <>
    struct CompareFunctionSetter4<INTEGER, INTEGER, INTEGER, INTEGER>:
      public EmptyCompareFunctionSetter
    {
      static void _set4(COMPARATORS_MAP & map)
      {
        set_map(map, "1111", Comparator4<1, 1, 1, 1>::Compare);
        set_map(map, "1-111", Comparator4<1, -1, 1, 1>::Compare);
        set_map(map, "-1111", Comparator4<-1, 1, 1, 1>::Compare);
        set_map(map, "-1-111", Comparator4<-1, -1, 1, 1>::Compare);
        set_map(map, "11-11", Comparator4<1, 1, -1, 1>::Compare);
        set_map(map, "1-1-11", Comparator4<1, -1, -1, 1>::Compare);
        set_map(map, "-11-11", Comparator4<-1, 1, -1, 1>::Compare);
        set_map(map, "-1-1-11", Comparator4<-1, -1, -1, 1>::Compare);

        set_map(map, "111-1", Comparator4<1, 1, 1, -1>::Compare);
        set_map(map, "1-11-1", Comparator4<1, -1, 1, -1>::Compare);
        set_map(map, "-111-1", Comparator4<-1, 1, 1, -1>::Compare);
        set_map(map, "-1-11-1", Comparator4<-1, -1, 1, -1>::Compare);
        set_map(map, "11-1-1", Comparator4<1, 1, -1, -1>::Compare);
        set_map(map, "1-1-1-1", Comparator4<1, -1, -1, -1>::Compare);
        set_map(map, "-11-1-1", Comparator4<-1, 1, -1, -1>::Compare);
        set_map(map, "-1-1-1-1", Comparator4<-1, -1, -1, -1>::Compare);
      }
    };

    template <int v2, int v3, int v4>
    struct CompareFunctionSetter4<UNDEF, v2, v3, v4> :
      public EmptyCompareFunctionSetter {};

    template <int v1, int v3, int v4>
    struct CompareFunctionSetter4<v1, UNDEF, v3, v4> :
      public EmptyCompareFunctionSetter {};

    template <int v3, int v4>
    struct CompareFunctionSetter4<UNDEF, UNDEF, v3, v4> :
      public EmptyCompareFunctionSetter {};

    template <int v1, int v2, int v4>
    struct CompareFunctionSetter4<v1, v2, UNDEF, v4> :
      public EmptyCompareFunctionSetter {};

    template <int v2, int v4>
    struct CompareFunctionSetter4<UNDEF, v2, UNDEF, v4> :
      public EmptyCompareFunctionSetter {};

    template <int v1, int v4>
    struct CompareFunctionSetter4<v1, UNDEF, UNDEF, v4> :
      public EmptyCompareFunctionSetter {};

    template <int v4>
    struct CompareFunctionSetter4<UNDEF, UNDEF, UNDEF, v4> :
      public EmptyCompareFunctionSetter {};

    template <int v1, int v2, int v3>
    struct CompareFunctionSetter4<v1, v2, v3, UNDEF> :
      public EmptyCompareFunctionSetter {};

    template <int v2, int v3>
    struct CompareFunctionSetter4<UNDEF, v2, v3, UNDEF> :
      public EmptyCompareFunctionSetter {};

    template <int v1, int v3>
    struct CompareFunctionSetter4<v1, UNDEF, v3, UNDEF> :
      public EmptyCompareFunctionSetter {};

    template <int v3>
    struct CompareFunctionSetter4<UNDEF, UNDEF, v3, UNDEF> :
      public EmptyCompareFunctionSetter {};

    template <int v1, int v2>
    struct CompareFunctionSetter4<v1, v2, UNDEF, UNDEF> :
      public EmptyCompareFunctionSetter {};

    template <int v2>
    struct CompareFunctionSetter4<UNDEF, v2, UNDEF, UNDEF> :
      public EmptyCompareFunctionSetter {};

    template <int v1>
    struct CompareFunctionSetter4<v1, UNDEF, UNDEF, UNDEF> :
      public EmptyCompareFunctionSetter {};

    template <>
    struct CompareFunctionSetter4<UNDEF, UNDEF, UNDEF, UNDEF> :
      public EmptyCompareFunctionSetter {};
#endif

    //--------------------------------------------------------------------------
    void InitComparators()
    {
      COMPARATORS_MAP & map = GetComparatorsMap();

      CompareFunctionSetter1<
        IndexedDynamicType<DYNAMIC_TYPES_COUNT>::type>::set(map);

#if NKIT_TABLE_INDEX_PART_SIZE >= 2
      CompareFunctionSetter2<
        IndexedDynamicType<DYNAMIC_TYPES_COUNT>::type,
        IndexedDynamicType<DYNAMIC_TYPES_COUNT>::type>::set(map);
#endif

#if NKIT_TABLE_INDEX_PART_SIZE >= 3
      CompareFunctionSetter3<
        IndexedDynamicType<DYNAMIC_TYPES_COUNT>::type,
        IndexedDynamicType<DYNAMIC_TYPES_COUNT>::type,
        IndexedDynamicType<DYNAMIC_TYPES_COUNT>::type>::set(map);
#endif

#if NKIT_TABLE_INDEX_PART_SIZE >= 4
      CompareFunctionSetter4<
        IndexedDynamicType<DYNAMIC_TYPES_COUNT>::type,
        IndexedDynamicType<DYNAMIC_TYPES_COUNT>::type,
        IndexedDynamicType<DYNAMIC_TYPES_COUNT>::type,
        IndexedDynamicType<DYNAMIC_TYPES_COUNT>::type>::set(map);
#endif
    }

  } // namespace detail
} // namespace nkit

#endif // __NKIT__TABLE__INDEX__COMPARE__H__

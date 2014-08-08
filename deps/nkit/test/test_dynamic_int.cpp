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

#include <iostream>
#include <algorithm>
#include <string>
#include <limits>

#include "nkit/detail/config.h"
#include "nkit/test.h"
#include "nkit/dynamic.h"
#include "nkit/dynamic_getter.h"
#include "nkit/version.h"

#define TABLE_GROW_SIZE 10

namespace nkit_test
{
  using namespace nkit;

  uint64_t UI64_MAX = std::numeric_limits<uint64_t>::max();
  uint64_t UI64_MAX_MINUS_1 = UI64_MAX - 1;
  int64_t I64_MAX = std::numeric_limits<int64_t>::max();
  int64_t I64_MAX_MINUS_1 = I64_MAX - 1;
  int64_t I64_MIN = std::numeric_limits<int64_t>::min();
  int64_t I64_MIN_PLUS_1 = I64_MIN + 1;
  int64_t I64_MIN_PLUS_2 = I64_MIN + 2;

  NKIT_TEST_CASE(DynamicIntMul)
  {
    const Dynamic SIX(6);
    const Dynamic N_SIX(-6);
    const Dynamic N_THREE(-3);

    //--------------------------------------------------------------------------
    // int64 & int64
    // 2 * 3 = 6
    // 2 * -3 = -6
    // -2 * 3 = -6
    // -2 * -3 = 6

    // 2 * 3 = 6
    Dynamic v1(3);
    Dynamic v2(2);
    NKIT_TEST_ASSERT(SIX == (v1 * v2));
    v2 *= v1;
    NKIT_TEST_ASSERT(SIX == v2);

    // 2 * -3 = -6
    v1 = Dynamic(-3);
    v2 = Dynamic(2);
    v2 *= v1;
    NKIT_TEST_ASSERT(N_SIX == v2);

    // -2 * 3 = -6
    v1 = Dynamic(3);
    v2 = Dynamic(-2);
    v2 *= v1;
    NKIT_TEST_ASSERT(N_SIX == v2);

    // -2 * -3 = 6
    v1 = Dynamic(-3);
    v2 = Dynamic(-2);
    v2 *= v1;
    NKIT_TEST_ASSERT(SIX == v2);

    //--------------------------------------------------------------------------
    // int64 & uint64
    // 2 * 3 = 6
    // -2 * 3 = -6

    // 2 * 3 = 6
    v1 = Dynamic::UInt64(3);
    v2 = Dynamic(2);
    v2 *= v1;
    NKIT_TEST_ASSERT(SIX == v2);

    // -2 * 3 = -6
    v1 = Dynamic::UInt64(3);
    v2 = Dynamic(-2);
    v2 *= v1;
    NKIT_TEST_ASSERT(N_SIX == v2);

    //--------------------------------------------------------------------------
    // uint64 & int64
    // 2 * 3 = 6
    // 2 * -3 -> error

    // 2 * 3 = 6
    v1 = Dynamic(3);
    v2 = Dynamic::UInt64(2);
    v2 *= v1;
    NKIT_TEST_ASSERT(SIX == v2);

    // 2 * -3 -> error
    v1 = Dynamic(-3);
    v2 = Dynamic::UInt64(2);
    v2 *= v1;
    NKIT_TEST_ASSERT(N_THREE == v1);
    NKIT_TEST_ASSERT(v2.IsUndef());

    //--------------------------------------------------------------------------
    // uint64 & uint64
    // 2 * 3 = 6

    // 2 * 3 = 6
    v1 = Dynamic::UInt64(3);
    v2 = Dynamic::UInt64(2);
    v2 *= v1;
    NKIT_TEST_ASSERT(SIX == v2);
  }

  NKIT_TEST_CASE(DynamicIntDiv)
  {
    const Dynamic THREE(3);
    const Dynamic N_THREE(-3);

    //--------------------------------------------------------------------------
    // int64 & int64
    // 6 / 2 = 3
    // -6 / 2 = -3
    // 6 / -2 = -3
    // -6 / -2 = 3

    // 6 / 2 = 3
    Dynamic v1(2);
    Dynamic v2(6);
    NKIT_TEST_ASSERT(THREE == (v2 / v1));
    v2 /= v1;
    NKIT_TEST_ASSERT(THREE == v2);

    // -6 / 2 = -3
    v1 = Dynamic(2);
    v2 = Dynamic(-6);
    v2 /= v1;
    NKIT_TEST_ASSERT(N_THREE == v2);

    // 6 / -2 = -3
    v1 = Dynamic(-2);
    v2 = Dynamic(6);
    v2 /= v1;
    NKIT_TEST_ASSERT(N_THREE == v2);

    // -6 / -2 = 3
    v1 = Dynamic(-2);
    v2 = Dynamic(-6);
    v2 /= v1;
    NKIT_TEST_ASSERT(THREE == v2);


    //--------------------------------------------------------------------------
    // int64 & uint64
    // 6 / 2 = 3
    // -6 / 2 = -3

    // 6 / 2 = 3
    v1 = Dynamic::UInt64(2);
    v2 = Dynamic(6);
    v2 /= v1;
    NKIT_TEST_ASSERT(THREE == v2);

    // -6 / 2 = -3
    v1 = Dynamic::UInt64(2);
    v2 = Dynamic(-6);
    v2 /= v1;
    NKIT_TEST_ASSERT(N_THREE == v2);

    //--------------------------------------------------------------------------
    // uint64 & int64
    // 6 / 2 = 3
    // 6 / -3 -> error

    // 6 / 2 = 3
    v1 = Dynamic(2);
    v2 = Dynamic::UInt64(6);
    v2 /= v1;
    NKIT_TEST_ASSERT(THREE == v2);

    // 6 / -3 -> error
    v1 = Dynamic(-3);
    v2 = Dynamic::UInt64(6);
    v2 /= v1;
    NKIT_TEST_ASSERT(N_THREE == v1);
    NKIT_TEST_ASSERT(v2.IsUndef());

    //--------------------------------------------------------------------------
    // uint64 & uint64
    // 6 / 2 = 3

    // 6 / 2 = 3
    v1 = Dynamic::UInt64(2);
    v2 = Dynamic::UInt64(6);
    v2 /= v1;
    NKIT_TEST_ASSERT(THREE == v2);
  }

  NKIT_TEST_CASE(DynamicIntSub)
  {
    const Dynamic ZERO(0);
    const Dynamic ONE(1);
    const Dynamic N_ONE(-1);
    const Dynamic TWO(2);
    const Dynamic N_TWO(-2);
    const Dynamic THREE(3);

    //--------------------------------------------------------------------------
    // int64 & int64
    // 1 - 1 = 0
    // -1 - 1 = -2
    // 2 - 1 = 1
    // 1 - 2 = -1
    // 1 - (-2) = 3
    // 2 - (-1) = 3

    Dynamic v1(1);
    Dynamic v2(1);
    NKIT_TEST_ASSERT(ZERO == (v1 - v2));
    v2 -= v1;
    NKIT_TEST_ASSERT(ZERO == v2);

    v1 = Dynamic(1);
    v2 = Dynamic(-1);
    v2 -= v1;
    NKIT_TEST_ASSERT(N_TWO == v2);

    v1 = Dynamic(1);
    v2 = Dynamic(2);
    v2 -= v1;
    NKIT_TEST_ASSERT(ONE == v2);

    v1 = Dynamic(2);
    v2 = Dynamic(1);
    v2 -= v1;
    NKIT_TEST_ASSERT(N_ONE == v2);

    v1 = Dynamic(-2);
    v2 = Dynamic(1);
    v2 -= v1;
    NKIT_TEST_ASSERT(THREE == v2);

    v1 = Dynamic(-1);
    v2 = Dynamic(2);
    v2 -= v1;
    NKIT_TEST_ASSERT(THREE == v2);

    v1 = Dynamic(I64_MIN_PLUS_1);
    v2 = Dynamic(0);
    v2 -= v1;
    NKIT_TEST_ASSERT(Dynamic(I64_MAX) == v2);

    v1 = Dynamic(I64_MAX);
    v2 = Dynamic(0);
    v2 -= v1;
    NKIT_TEST_ASSERT(Dynamic(I64_MIN_PLUS_1) == v2);

    //--------------------------------------------------------------------------
    // int64 & uint64
    // 1 - 1 = 0
    // 1 - 2 = -1
    // 2 - 1 = 1
    // -1 - 1 = -2
    // INT64_MAX - UINT64_MAX = INT64_MIN

    // 1 - 1 = 0
    v1 = Dynamic::UInt64(1);
    v2 = Dynamic(1);
    v2 -= v1;
    NKIT_TEST_ASSERT(ZERO == v2);

    // 1 - 2 = -1
    v1 = Dynamic::UInt64(2);
    v2 = Dynamic(1);
    v2 -= v1;
    NKIT_TEST_ASSERT(N_ONE == v2);

    // 2 - 1 = 1
    v1 = Dynamic::UInt64(1);
    v2 = Dynamic(2);
    v2 -= v1;
    NKIT_TEST_ASSERT(ONE == v2);

    // -1 - 1 = -2
    v1 = Dynamic::UInt64(1);
    v2 = Dynamic(-1);
    v2 -= v1;
    NKIT_TEST_ASSERT(N_TWO == v2);

    // I64_MAX - UI64_MAX = I64_MIN
    v1 = Dynamic::UInt64(UI64_MAX);
    v2 = Dynamic(I64_MAX);
    v2 -= v1;
    NKIT_TEST_ASSERT(Dynamic(I64_MIN) == v2);

    //--------------------------------------------------------------------------
    // uint64 & int64
    // 1 - 1 = 0
    // 1 - (-1) = 2
    // I64_MAX - I64_MIN = UI64_MAX
    // UI64_MAX - I64_MAX - 1 = I64_MAX

    // 1 - 1 = 0
    v1 = Dynamic(1);
    v2 = Dynamic::UInt64(1);
    v2 -= v1;
    NKIT_TEST_ASSERT(ZERO == v2);
    NKIT_TEST_ASSERT(Dynamic::UInt64(0) == v2);

    // 1 - (-1) = 2
    v1 = Dynamic(-1);
    v2 = Dynamic::UInt64(1);
    v2 -= v1;
    NKIT_TEST_ASSERT(TWO == v2);
    NKIT_TEST_ASSERT(Dynamic::UInt64(2) == v2);

    // I64_MAX - I64_MIN = UI64_MAX
    v1 = Dynamic(I64_MIN);
    v2 = Dynamic::UInt64(I64_MAX);
    v2 -= v1;
    NKIT_TEST_ASSERT(Dynamic::UInt64(UI64_MAX) == v2);

    // UI64_MAX - I64_MAX - 1 = I64_MAX
    v1 = Dynamic(I64_MAX);
    v2 = Dynamic::UInt64(UI64_MAX);
    v2 -= v1;
    v2 -= ONE;
    NKIT_TEST_ASSERT(Dynamic(I64_MAX) == v2);
    NKIT_TEST_ASSERT(Dynamic::UInt64(I64_MAX) == v2);

    //--------------------------------------------------------------------------
    // uint64 & uint64
    // 1 - 1 = 0
    // 2 - 1 = 1
    // UI64_MAX_MINUS_1 - I64_MAX = I64_MAX

    // 1 - 1 = 0
    v1 = Dynamic::UInt64(1);
    v2 = Dynamic::UInt64(1);
    v2 -= v1;
    NKIT_TEST_ASSERT(ZERO == v2);

    // 2 - 1 = 1
    v1 = Dynamic::UInt64(1);
    v2 = Dynamic::UInt64(2);
    v2 -= v1;
    NKIT_TEST_ASSERT(ONE == v2);

    // UI64_MAX_MINUS_1 - I64_MAX = I64_MAX
    v1 = Dynamic::UInt64(I64_MAX);
    v2 = Dynamic::UInt64(UI64_MAX_MINUS_1);
    v2 -= v1;
    NKIT_TEST_ASSERT(Dynamic::UInt64(I64_MAX) == v2);
  }

  NKIT_TEST_CASE(DynamicIntAdd)
  {
    const Dynamic ZERO(0);
    const Dynamic ONE(1);
    const Dynamic N_ONE(-1);
    const Dynamic TWO(2);

    //--------------------------------------------------------------------------
    // int64 & int64
    // 1 + 1 = 2
    // -1 + 1 = 0
    // -2 + 1 = -1
    // -1 + 2 = 1
    // 1 + (-2) = -1
    // 2 + (-1) = 1
    Dynamic v1(1);
    Dynamic v2(1);
    NKIT_TEST_ASSERT(TWO == (v1 + v2));
    v2 += v1;
    NKIT_TEST_ASSERT(TWO == v2);

    v1 = Dynamic(1);
    v2 = Dynamic(-1);
    v2 += v1;
    NKIT_TEST_ASSERT(ZERO == v2);

    v1 = Dynamic(1);
    v2 = Dynamic(-2);
    v2 += v1;
    NKIT_TEST_ASSERT(N_ONE == v2);

    v1 = Dynamic(2);
    v2 = Dynamic(-1);
    v2 += v1;
    NKIT_TEST_ASSERT(ONE == v2);

    v1 = Dynamic(-2);
    v2 = Dynamic(1);
    v2 += v1;
    NKIT_TEST_ASSERT(N_ONE == v2);

    v1 = Dynamic(-1);
    v2 = Dynamic(2);
    v2 += v1;
    NKIT_TEST_ASSERT(ONE == v2);

    v1 = Dynamic(I64_MAX);
    v2 = Dynamic(I64_MIN_PLUS_2);
    v2 += v1;
    NKIT_TEST_ASSERT(ONE == v2);

    //--------------------------------------------------------------------------
    // int64 & uint64
    // 1 + 1 = 2
    // -1 + 1 = 0
    // -1 + 2 = 1
    // -2 + 1 = -1
    // I64_MIN + I64_MAX = -1
    // I64_MAX + I64_MIN_PLUS_2 = 1
    // INT64_MIN + UI64_MAX = I64_MAX

    // 1 + 1 = 2
    v1 = Dynamic::UInt64(1);
    v2 = Dynamic(1);
    v2 += v1;
    NKIT_TEST_ASSERT(TWO == v2);

    // -1 + 1 = 0
    v1 = Dynamic::UInt64(1);
    v2 = Dynamic(-1);
    v2 += v1;
    NKIT_TEST_ASSERT(ZERO == v2);

    // -1 + 2 = 1
    v1 = Dynamic::UInt64(2);
    v2 = Dynamic(-1);
    v2 += v1;
    NKIT_TEST_ASSERT(ONE == v2);

    // -2 + 1 = -1
    v1 = Dynamic::UInt64(1);
    v2 = Dynamic(-2);
    v2 += v1;
    NKIT_TEST_ASSERT(N_ONE == v2);

    // I64_MIN + I64_MAX = -1
    v1 = Dynamic::UInt64(I64_MAX);
    v2 = Dynamic(I64_MIN);
    v2 += v1;
    NKIT_TEST_ASSERT(N_ONE == v2);

    // I64_MAX + I64_MIN_PLUS_2 = 1
    v1 = Dynamic::UInt64(I64_MAX);
    v2 = Dynamic(I64_MIN_PLUS_2);
    v2 += v1;
    NKIT_TEST_ASSERT(ONE == v2);

    // I64_MIN + UI64_MAX = I64_MAX
    v1 = Dynamic::UInt64(UI64_MAX);
    v2 = Dynamic(I64_MIN);
    v2 += v1;
    NKIT_TEST_ASSERT(Dynamic(I64_MAX) == v2);

    //--------------------------------------------------------------------------
    // uint64 & int64
    // 1 + 1 = 2
    // 1 + (-1) = 0
    // 2 + (-1) = 1
    // I64_MAX + I64_MIN_PLUS_2 = 1
    // UI64_MAX + I64_MIN = I64_MAX

    // 1 + 1 = 2
    v1 = Dynamic(1);
    v2 = Dynamic::UInt64(1);
    v2 += v1;
    NKIT_TEST_ASSERT(TWO == v2);

    // 1 + (-1) = 0
    v1 = Dynamic(-1);
    v2 = Dynamic::UInt64(1);
    v2 += v1;
    NKIT_TEST_ASSERT(ZERO == v2);

    // 2 + (-1) = 1
    v1 = Dynamic(-1);
    v2 = Dynamic::UInt64(2);
    v2 += v1;
    NKIT_TEST_ASSERT(ONE == v2);

    // I64_MAX + I64_MIN_PLUS_2 = 1
    v1 = Dynamic(I64_MIN_PLUS_2);
    v2 = Dynamic::UInt64(I64_MAX);
    v2 += v1;
    NKIT_TEST_ASSERT(ONE == v2);

    // UI64_MAX + I64_MIN = I64_MAX
    v1 = Dynamic(I64_MIN);
    v2 = Dynamic::UInt64(UI64_MAX);
    v2 += v1;
    NKIT_TEST_ASSERT(Dynamic(I64_MAX) == v2);

    //--------------------------------------------------------------------------
    // uint64 & uint64
    // 1 + 1 = 2
    // I64_MAX + I64_MAX = UI64_MAX_MINUS_1

    // 1 + 1 = 2
    v1 = Dynamic::UInt64(1);
    v2 = Dynamic::UInt64(1);
    NKIT_TEST_ASSERT(TWO == (v1 + v2));
    v2 += v1;
    NKIT_TEST_ASSERT(TWO == v2);

    // I64_MAX + I64_MAX = UI64_MAX_MINUS_1
    v1 = Dynamic::UInt64(I64_MAX);
    v2 = Dynamic::UInt64(I64_MAX);
    NKIT_TEST_ASSERT(Dynamic::UInt64(UI64_MAX_MINUS_1) == (v1 + v2));
    v2 += v1;
    NKIT_TEST_ASSERT(Dynamic::UInt64(UI64_MAX_MINUS_1) == v2);
  }

  NKIT_TEST_CASE(DynamicIntEq)
  {
    NKIT_TEST_ASSERT(Dynamic(1) == Dynamic(1));
    NKIT_TEST_ASSERT(Dynamic::UInt64(1) == Dynamic(1));
    NKIT_TEST_ASSERT(Dynamic(1) == Dynamic::UInt64(1));
    NKIT_TEST_ASSERT(Dynamic::UInt64(1) == Dynamic::UInt64(1));
  }

  NKIT_TEST_CASE(DynamicIntLt)
  {
    Dynamic one(1);
    Dynamic d_ui64_max_minus_1 = Dynamic::UInt64(UI64_MAX_MINUS_1);
    Dynamic d_ui64_max = d_ui64_max_minus_1 + one;
    Dynamic d_ui64_min = d_ui64_max + one;

    Dynamic d_i64_max_minus_1(I64_MAX_MINUS_1);
    Dynamic d_i64_max = d_i64_max_minus_1 + one;
    Dynamic d_i64_min = d_i64_max + one;

    NKIT_TEST_ASSERT(d_ui64_max_minus_1 < d_ui64_max);
    NKIT_TEST_ASSERT(d_i64_max_minus_1 < d_ui64_max_minus_1);
    NKIT_TEST_ASSERT(d_ui64_max_minus_1 < d_ui64_max);
    NKIT_TEST_ASSERT(d_ui64_max_minus_1 > d_ui64_min);
    NKIT_TEST_ASSERT(d_i64_max_minus_1 < d_i64_max);
    NKIT_TEST_ASSERT(d_i64_max_minus_1 > d_i64_min);
    NKIT_TEST_ASSERT(d_ui64_min > d_i64_min);
  }
} // namespace nkit_test

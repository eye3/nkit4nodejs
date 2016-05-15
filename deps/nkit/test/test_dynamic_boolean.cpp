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

#include <nkit/dynamic_json.h>
#include <nkit/logger_brief.h>
#include <nkit/test.h>

namespace nkit_test
{
  using namespace nkit;

  NKIT_TEST_CASE(DynamicBoolInDict)
  {
    Dynamic d = DDICT("true" << true << "false" << false);
    NKIT_TEST_ASSERT(d["true"]);
    NKIT_TEST_ASSERT(!d["false"]);

    std::string json = DynamicToJson(d);
    CINFO(json);
    std::string error;
    d = DynamicFromJson(json, &error);
    NKIT_TEST_ASSERT(d["true"]);
    NKIT_TEST_ASSERT(!d["false"]);
  }

  NKIT_TEST_CASE(DynamicBool)
  {
    std::string error;
    Dynamic d;
    NKIT_TEST_ASSERT(!d);

    Dynamic none = Dynamic(Dynamic::NONE_MARKER);
    NKIT_TEST_ASSERT(!none);
    none = Dynamic("qwe");
    NKIT_TEST_ASSERT(!none);

    d = Dynamic("qwe");
    NKIT_TEST_ASSERT(d);
    d = Dynamic("");
    NKIT_TEST_ASSERT(!d);

    d = Dynamic::MongodbOID();
    NKIT_TEST_ASSERT(d);
    d = Dynamic::MongodbOID("123456789012345678901234");
    NKIT_TEST_ASSERT(d);

    d = Dynamic(true);
    NKIT_TEST_ASSERT(d);
    d = Dynamic(false);
    NKIT_TEST_ASSERT(!d);

    d = Dynamic(2.3);
    NKIT_TEST_ASSERT(d);
    d = Dynamic(0.0);
    NKIT_TEST_ASSERT(!d);

    d = Dynamic(uint64_t(1));
    NKIT_TEST_ASSERT(d);
    d = Dynamic(uint64_t(0));
    NKIT_TEST_ASSERT(!d);

    d = Dynamic(uint32_t(1));
    NKIT_TEST_ASSERT(d);
    d = Dynamic(uint32_t(0));
    NKIT_TEST_ASSERT(!d);

    d = Dynamic(uint16_t(1));
    NKIT_TEST_ASSERT(d);
    d = Dynamic(uint16_t(0));
    NKIT_TEST_ASSERT(!d);

    d = Dynamic(uint8_t(1));
    NKIT_TEST_ASSERT(d);
    d = Dynamic(uint8_t(0));
    NKIT_TEST_ASSERT(!d);

    d = Dynamic(int64_t(-1));
    NKIT_TEST_ASSERT(d);
    d = Dynamic(int64_t(0));
    NKIT_TEST_ASSERT(!d);

    d = Dynamic(int32_t(-1));
    NKIT_TEST_ASSERT(d);
    d = Dynamic(int32_t(0));
    NKIT_TEST_ASSERT(!d);

    d = Dynamic(int16_t(-1));
    NKIT_TEST_ASSERT(d);
    d = Dynamic(int16_t(0));
    NKIT_TEST_ASSERT(!d);

    d = Dynamic(int8_t(-1));
    NKIT_TEST_ASSERT(d);
    d = Dynamic(int8_t(0));
    NKIT_TEST_ASSERT(!d);

    d = Dynamic::DateTimeFromDefault("2012-07-08 00:12:33", &error);
    NKIT_TEST_ASSERT(d);
    d = Dynamic::DateTimeFromDefault("0000-00-00 00:00:00", &error);
    NKIT_TEST_ASSERT(!d);

    d = DDICT("key" << "value");
    NKIT_TEST_ASSERT(d);
    d.Clear();
    NKIT_TEST_ASSERT(d.IsDict());
    NKIT_TEST_ASSERT(!d);

    d = DLIST("item1" << "item2");
    NKIT_TEST_ASSERT(d);
    d.Clear();
    NKIT_TEST_ASSERT(d.IsList());
    NKIT_TEST_ASSERT(!d);
  }

} // namespace nkit_test

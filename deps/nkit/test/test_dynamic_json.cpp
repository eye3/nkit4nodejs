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

  NKIT_TEST_CASE(DynamicJsonVector)
  {
    DynamicVector dv;
    dv.push_back(DDICT("1" << 1 << "2" << 2));
    dv.push_back(DDICT("2" << 2 << "3" << 3));

    std::string json;
    DynamicToJson(dv, &json);

    std::string error;
    Dynamic l = DynamicFromJson(json, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(l, error);
    NKIT_TEST_ASSERT(l.IsList() && l.size() == 2);

    size_t idx = 0;
    NKIT_TEST_ASSERT(l[idx] == dv[idx]);
    idx++;
    NKIT_TEST_ASSERT(l[idx] == dv[idx]);
  }

  NKIT_TEST_CASE(DynamicJsonFile)
  {
    DynamicVector dv;
    dv.push_back(DDICT("1" << 1 << "2" << 2));
    dv.push_back(DDICT("2" << 2 << "3" << 3));

    std::string json, file_path, error;
    file_path = "./DynamicJsonFile.tmp";
    NKIT_TEST_ASSERT_WITH_TEXT(DynamicToJsonFile(dv, file_path, &error), error);

    Dynamic l = DynamicFromJsonFile(file_path, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(l, error);
    NKIT_TEST_ASSERT(l.IsList() && l.size() == 2);

    size_t idx = 0;
    NKIT_TEST_ASSERT(l[idx] == dv[idx]);
    idx++;
    NKIT_TEST_ASSERT(l[idx] == dv[idx]);

    std::remove(file_path.c_str());
  }

  NKIT_TEST_CASE(DynamicJsonBigInts)
  {
    uint64_t ui64_max_minus_1 = std::numeric_limits<uint64_t>::max() - 1;

    uint64_t ui64_max = std::numeric_limits<uint64_t>::max();

    uint64_t i64_max_plus_1 =
          static_cast<uint64_t>(std::numeric_limits<int64_t>::max())
        + static_cast<uint64_t>(1);

    int64_t i64_max_minus_1 =
          static_cast<int64_t>(std::numeric_limits<int64_t>::max())
        - static_cast<int64_t>(1);

    int64_t i64_max = static_cast<int64_t>(std::numeric_limits<int64_t>::max());

    int64_t i64_min_plus_1 =
          static_cast<int64_t>(std::numeric_limits<int64_t>::min())
        + static_cast<int64_t>(1);

    int64_t i64_min =
          static_cast<int64_t>(std::numeric_limits<int64_t>::min());

    int64_t small_i64 = 123132;
    double small_double = 123132.5675;

    std::string json = "{\"begin\": \"begin\""
        ",\"ui64_max_minus_1\":" + string_cast(ui64_max_minus_1) +
        ",\"ui64_max\":" + string_cast(ui64_max) +
        ",\"i64_max_plus_1\":" + string_cast(i64_max_plus_1) +
        ",\"i64_max_minus_1\":" + string_cast(i64_max_minus_1) +
        ",\"i64_max\":" + string_cast(i64_max) +
        ",\"i64_min_plus_1\":" + string_cast(i64_min_plus_1) +
        ",\"i64_min\":" + string_cast(i64_min) +
        ",\"small_i64\":" + string_cast(small_i64) +
        ",\"small_double\":" + string_cast(small_double, 4) +
        ",\"big_double\":" + "18446744073709651620" +
        ",\"end\": \"end\"}";
    std::string error;
    Dynamic v(DynamicFromJson(json, &error));
    NKIT_TEST_ASSERT_WITH_TEXT(error.empty(), error);

    NKIT_TEST_ASSERT(v["ui64_max_minus_1"].GetUnsignedInteger() ==
        ui64_max_minus_1);
    NKIT_TEST_ASSERT(v["ui64_max"].GetUnsignedInteger() ==
        ui64_max);
    NKIT_TEST_ASSERT(v["i64_max_plus_1"].GetUnsignedInteger() ==
        i64_max_plus_1);
    NKIT_TEST_ASSERT(v["i64_max_minus_1"].GetSignedInteger() ==
        i64_max_minus_1);
    NKIT_TEST_ASSERT(v["i64_max"].GetSignedInteger() ==
        i64_max);
    NKIT_TEST_ASSERT(v["i64_min_plus_1"].GetSignedInteger() ==
        i64_min_plus_1);
    NKIT_TEST_ASSERT(v["i64_min"].GetSignedInteger() ==
        i64_min);
    NKIT_TEST_ASSERT(v["i64_min"].GetSignedInteger() ==
        i64_min);
    NKIT_TEST_ASSERT(v["small_i64"].GetSignedInteger() ==
        small_i64);
    NKIT_TEST_ASSERT(v["small_double"].GetFloat() ==
        small_double);
    NKIT_TEST_ASSERT(v["big_double"].IsFloat());

    std::string out;
    DynamicToJson(v, &out);

    NKIT_TEST_ASSERT(DynamicFromJson(out, &error) == v);
  }

  bool compare_by_pdate(const Dynamic & match1, const Dynamic & match2)
  {
    NKIT_TEST_ASSERT(match1.IsDict());
    NKIT_TEST_ASSERT(match2.IsDict());

    const Dynamic & data1 = match1["data"];
    const Dynamic & data2 = match2["data"];
    NKIT_TEST_ASSERT(data1.IsDict());
    NKIT_TEST_ASSERT(data2.IsDict());

    const Dynamic & date1 = data1["publish_date"];
    const Dynamic & date2 = data2["publish_date"];

    return date1 > date2;
  }

  NKIT_TEST_CASE(DynamicJsonWithTable)
  {
    Dynamic table = DTBL("first:STRING, second:INTEGER",
           "1" << 1
        << "2" << 2
        << "3" << 3);

    CINFO(json_hr << table);
    CINFO(json_hr_table << table);
    CINFO(json << table);

    Dynamic etalon = DLIST(
           DDICT("first" << "1" << "second" << 1)
        << DDICT("first" << "2" << "second" << 2)
        << DDICT("first" << "3" << "second" << 3)
      );

    std::string json = DynamicToJson(table);
    std::string error;
    Dynamic list = DynamicFromJson(json, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(error.empty(), error);
    NKIT_TEST_ASSERT(list == etalon);
  }

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(DynamicJsonOStream)
  {
    std::string str_config("string value \\f \\n \\t \\b \\r \\/ "
        "\\\\ \\/ / \\"),
        str_check, str_default;
    uint64_t ui64_config(33);
    bool b_config(true);

    Dynamic dict = DDICT(
          "str" << str_config
      << "int" << ui64_config
      << "bool" << b_config
      << "list" << DLIST("1" << 2 << 3.0 << "four" << b_config)
      << "dict" << DDICT(
           "str" << str_config
        << "int" << ui64_config
        << "bool" << b_config
        << "xx" << "string"
        << "list1" << DLIST("1")
        << "list" << DLIST("1" << 2 << 3.0 << "four" << b_config)
        << "dict" << DDICT(
               "str" << str_config
            << "int" << ui64_config
            << "bool" << b_config
            << "xx" << "string"
            << "list1" << DLIST("1")
            << "list" << DLIST("1" << 2)
          )
        )
    );

    std::string error;
    std::stringstream ss;

    ss << dict;
    Dynamic res = DynamicFromJson(ss.str(), &error);
    NKIT_TEST_ASSERT_WITH_TEXT(res == dict, error);
  }

  NKIT_TEST_CASE(DynamicJsonWrongCases)
  {
    std::string json = "1,B,1,4,8F,61,84C,140,44,84DC,F0,C8,5440,F0,"
        "258,7139,8F,61,8E91,8F,61,3178,F0,190,319F,DC,A4,D7C0,276,"
        "1AE,22B2,8F,61,69BF";
    std::string error;
    Dynamic d = DynamicFromJson(json, &error);
    NKIT_TEST_ASSERT(d.IsUndef());

    error.clear();
    d = DynamicFromJson("123", &error);
    NKIT_TEST_ASSERT(d.IsUndef());
  }
} // namespace nkit_test

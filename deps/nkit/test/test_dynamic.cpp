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

#include "nkit/test.h"
#include "nkit/dynamic_json.h"
#include "nkit/detail/config.h"
#include "nkit/dynamic_getter.h"
#include "nkit/logger_brief.h"
#include "nkit/version.h"

#include <iostream>
#include <algorithm>
#include <string>

#define TABLE_GROW_SIZE 10

namespace nkit_test
{
  using namespace nkit;

  template <typename T>
  bool test_dynamic_to_type(T value)
  {
    T etalon(value), v;
    Dynamic d(etalon);
    v << d;
    return etalon == v;
  }

  NKIT_TEST_CASE(DynamicToType)
  {
    NKIT_TEST_ASSERT(test_dynamic_to_type(uint8_t(4)));
    NKIT_TEST_ASSERT(test_dynamic_to_type(int8_t(4)));
    NKIT_TEST_ASSERT(test_dynamic_to_type(int8_t(-4)));

    NKIT_TEST_ASSERT(test_dynamic_to_type(uint16_t(4)));
    NKIT_TEST_ASSERT(test_dynamic_to_type(int16_t(4)));
    NKIT_TEST_ASSERT(test_dynamic_to_type(int16_t(-4)));

    NKIT_TEST_ASSERT(test_dynamic_to_type(uint32_t(4)));
    NKIT_TEST_ASSERT(test_dynamic_to_type(int32_t(4)));
    NKIT_TEST_ASSERT(test_dynamic_to_type(int32_t(-4)));

    NKIT_TEST_ASSERT(test_dynamic_to_type(uint64_t(4)));
    NKIT_TEST_ASSERT(test_dynamic_to_type(int64_t(4)));
    NKIT_TEST_ASSERT(test_dynamic_to_type(int64_t(-4)));

    NKIT_TEST_ASSERT(test_dynamic_to_type(std::string("4")));

    Dynamic dict_etalon = DDICT(1 << 11 <<
        2 << 22 <<
        3 << 33);

    std::map<uint16_t, uint16_t> map;
    map << dict_etalon;
    Dynamic dict = Dynamic::Dict(map);
    NKIT_TEST_ASSERT(dict == dict_etalon);
  }

  NKIT_TEST_CASE(DynamicListConstructor)
  {
    StringSet combination;
    Dynamic::List(combination);

    StringVector vect;
    Dynamic::List(vect);
  }

  NKIT_TEST_CASE(Dynamic_GetByIndexOperator)
  {
    const Dynamic list(DLIST(1 << 2));
    CINFO(list[1]);

    Dynamic dict(DDICT("1"<< 1 << "2" << 2));
    CINFO(dict["1"]);
  }

  NKIT_TEST_CASE(Dynamic_IntStrCmp)
  {
    Dynamic str1("1");
    Dynamic str2("2");
    Dynamic i1(1);
    Dynamic i2(2);
    NKIT_TEST_ASSERT(str1 < str2 && str2 > str1);
    NKIT_TEST_ASSERT(str1 < i2 && i2 > str1);
    NKIT_TEST_ASSERT(str2 > i1 && i1 < str2);

    Dynamic ui1 = Dynamic::UInt64(1);
    Dynamic ui2 = Dynamic::UInt64(2);
    NKIT_TEST_ASSERT(str1 < ui2 && ui2 > str1);
    NKIT_TEST_ASSERT(str2 > ui1 && ui1 < str2);

    Dynamic mongo_oid1("123456789012345678901233");
    Dynamic mongo_oid2("123456789012345678901235");
    NKIT_TEST_ASSERT(mongo_oid1 < mongo_oid2 && mongo_oid2 > mongo_oid1);
  }

  NKIT_TEST_CASE(Dynamic_IntStrEq)
  {
    Dynamic str1("1");
    Dynamic i1(1);
    NKIT_TEST_ASSERT(str1 == i1 && i1 == str1);

    Dynamic mongo_oid1("123456789012345678901234");
    Dynamic mongo_oid2("123456789012345678901234");
    NKIT_TEST_ASSERT(mongo_oid1 == mongo_oid2);
  }

  NKIT_TEST_CASE(DynamicGetter)
  {
    std::string str_config("string value"), str_check, str_default;
    uint64_t ui64_config(33), ui64_check, ui64_default(0);
    bool b_config(true), b_check, b_default(false);

    Dynamic config = DDICT(
           "str" << str_config <<
           "int" << ui64_config <<
           "bool" << b_config <<
           "hash" << DDICT(
               "str" << str_config <<
               "int" << ui64_config <<
               "bool" << b_config
               )
            );

    DynamicGetter loader(config, "DynamicGetter #1");
    DynamicGetter sub_loader;
    loader
      .Get(".str", &str_check, str_default)
      .Get(".int", &ui64_check, ui64_default)
      .Get(".bool", &b_check, b_default)
      .Get(".hash", &sub_loader)
    ;
    NKIT_TEST_ASSERT_WITH_TEXT(loader.ok(), loader.error());
    NKIT_TEST_ASSERT_WITH_TEXT(str_check == str_config, loader.error());
    NKIT_TEST_ASSERT_WITH_TEXT(ui64_check == ui64_config, loader.error());
    NKIT_TEST_ASSERT_WITH_TEXT(b_check == b_config, loader.error());

    str_check = str_default;
    ui64_check = ui64_default;
    b_check = b_default;
    sub_loader
      .Get(".str", &str_check, str_default)
      .Get(".int", &ui64_check, ui64_default)
      .Get(".bool", &b_check, b_default)
    ;
    NKIT_TEST_ASSERT_WITH_TEXT(sub_loader.ok(), sub_loader.error());
    NKIT_TEST_ASSERT_WITH_TEXT(str_check == str_config, sub_loader.error());
    NKIT_TEST_ASSERT_WITH_TEXT(ui64_check == ui64_config, sub_loader.error());
    NKIT_TEST_ASSERT_WITH_TEXT(b_check == b_config, sub_loader.error());

    // explicit error
    loader = DynamicGetter(config, "DynamicGetter #2");
    loader.Get(".str1", &str_check);
    NKIT_TEST_ASSERT(!loader.ok());
    std::cout << loader.error() << std::endl;

    loader = DynamicGetter(config, "DynamicGetter #3");
    loader.Get(".hash", &sub_loader);
    sub_loader.Get(".str1", &str_check);
    NKIT_TEST_ASSERT(!sub_loader.ok());
    std::cout << sub_loader.error() << std::endl;
  }

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(DynamicForeachError)
  {
    Dynamic undef;
    DLIST_FOREACH(item, undef)
    {
      CINFO(*item);
      NKIT_TEST_ASSERT(false);
    }

    DDICT_FOREACH(pair, undef)
    {
      NKIT_TEST_ASSERT(false);
    }

    Dynamic none = D_NONE;
    DLIST_FOREACH(item, none)
    {
      CINFO(*item);
      NKIT_TEST_ASSERT(false);
    }

    DDICT_FOREACH(pair, none)
    {
      NKIT_TEST_ASSERT(false);
    }

    Dynamic str("qwe");
    DLIST_FOREACH(item, str)
    {
      CINFO(*item);
      NKIT_TEST_ASSERT(false);
    }
    DDICT_FOREACH(pair, str)
    {
      NKIT_TEST_ASSERT(false);
    }

    Dynamic i64(int64_t(1));
    DLIST_FOREACH(item, i64)
    {
      CINFO(*item);
      NKIT_TEST_ASSERT(false);
    }

    DDICT_FOREACH(pair, i64)
    {
      NKIT_TEST_ASSERT(false);
    }

    Dynamic ui64(uint64_t(1));
    DLIST_FOREACH(item, ui64)
    {
      CINFO(*item);
      NKIT_TEST_ASSERT(false);
    }

    DDICT_FOREACH(pair, ui64)
    {
      NKIT_TEST_ASSERT(false);
    }

    Dynamic b(true);
    DLIST_FOREACH(item, b)
    {
      CINFO(*item);
      NKIT_TEST_ASSERT(false);
    }

    DDICT_FOREACH(pair, b)
    {
      NKIT_TEST_ASSERT(false);
    }
  }

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(DynamicPath)
  {
    std::string name;
    size_t index_var(0);
    DynamicPath dp;

    dp = DynamicPath(".[]", &name);
    NKIT_TEST_ASSERT_WITH_TEXT(!dp.ok(), dp.Print());

    dp = DynamicPath(".[", &name);
    NKIT_TEST_ASSERT_WITH_TEXT(!dp.ok(), dp.Print());

    dp = DynamicPath(".]", &name);
    NKIT_TEST_ASSERT_WITH_TEXT(!dp.ok(), dp.Print());

    dp = DynamicPath("[10].name", &name);
    NKIT_TEST_ASSERT_WITH_TEXT(!dp.ok(), dp.Print());

    dp = DynamicPath(".[10].[30].name", &name);
    NKIT_TEST_ASSERT_WITH_TEXT(!dp.ok(), dp.Print());

    dp = DynamicPath(".[10]name", &name);
    NKIT_TEST_ASSERT_WITH_TEXT(!dp.ok(), dp.Print());

    dp = DynamicPath(".", &name);
    NKIT_TEST_ASSERT_WITH_TEXT(dp.ok(), dp.Print());

    dp = DynamicPath(".%", &name);
    NKIT_TEST_ASSERT_WITH_TEXT(dp.ok(), dp.error());
    name = "name1";
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".&" + name), dp.Print());
    name = "name2";
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".&" + name), dp.Print());

    dp = DynamicPath(".%[%]", &name, &index_var);
    NKIT_TEST_ASSERT_WITH_TEXT(dp.ok(), dp.error());
    name = "name1";
    index_var = 0;
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".&" + name + "[&0]"), dp.Print());
    name = "name2";
    index_var = 4;
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".&" + name + "[&4]"), dp.Print());

    dp = DynamicPath(".name[%]", &index_var);
    NKIT_TEST_ASSERT_WITH_TEXT(dp.ok(), dp.error());
    name = "name1";
    index_var = 0;
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".name[&0]"), dp.Print());
    name = "name2";
    index_var = 4;
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".name[&4]"), dp.Print());

    dp = DynamicPath(".aa[%][30].%", &index_var, &name);
    NKIT_TEST_ASSERT_WITH_TEXT(dp.ok(), dp.error());
    name = "name1";
    index_var = 0;
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".aa[&0][30].&"+name), dp.Print());
    index_var = 4;
    name = "name2";
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".aa[&4][30].&"+name), dp.Print());

    dp = DynamicPath(".aa[%].%[30]", &index_var, &name);
    NKIT_TEST_ASSERT_WITH_TEXT(dp.ok(), dp.error());
    name = "name1";
    index_var = 0;
    NKIT_TEST_ASSERT_WITH_TEXT(
        dp.Print() == (".aa[&0].&"+name + "[30]"), dp.Print());
    index_var = 4;
    name = "name2";
    NKIT_TEST_ASSERT_WITH_TEXT(
        dp.Print() == (".aa[&4].&"+name + "[30]"), dp.Print());

    dp = DynamicPath() / &index_var / &name;
    name = "name1";
    index_var = 0;
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".[&0].&"+name), dp.Print());
    index_var = 4;
    name = "name2";
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".[&4].&"+name), dp.Print());

    dp = DynamicPath() / "bison" / &index_var / &name;
    name = "name1";
    index_var = 0;
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".bison[&0].&"+name), dp.Print());
    index_var = 4;
    name = "name2";
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".bison[&4].&"+name), dp.Print());

    dp = DynamicPath() / "bison" / 10 / &name;
    name = "name1";
    index_var = 0;
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".bison[10].&"+name), dp.Print());
    index_var = 4;
    name = "name2";
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".bison[10].&"+name), dp.Print());

    dp = DynamicPath() / "bison" / &index_var / 30 / &name;
    name = "name1";
    index_var = 0;
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".bison[&0][30].&"+name), dp.Print());
    index_var = 4;
    name = "name2";
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".bison[&4][30].&"+name), dp.Print());

    dp = DynamicPath() / "bison" / 30 / &index_var / &name;
    name = "name1";
    index_var = 0;
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".bison[30][&0].&"+name), dp.Print());
    index_var = 4;
    name = "name2";
    NKIT_TEST_ASSERT_WITH_TEXT(dp.Print() == (".bison[30][&4].&"+name), dp.Print());
  }

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(DynamicStartsWith)
  {
    std::string etalon("qwe");
    Dynamic vstr = Dynamic(etalon + "asd");
    NKIT_TEST_ASSERT(vstr.StartsWith(etalon));
    vstr = Dynamic("asd" + etalon);
    NKIT_TEST_ASSERT(!vstr.StartsWith(etalon));
    Dynamic tmp = Dynamic(true);
    NKIT_TEST_ASSERT(!tmp.StartsWith(etalon));
    tmp = Dynamic(uint64_t(1));
    NKIT_TEST_ASSERT(!tmp.StartsWith(etalon));

    std::string str = etalon + "----------";
    NKIT_TEST_ASSERT(starts_with(str, etalon.c_str()));
  }

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(DynamicEndsWith)
  {
    std::string etalon("qwe");
    Dynamic vstr = Dynamic("asd" + etalon);
    NKIT_TEST_ASSERT(vstr.EndsWith(etalon));
    vstr = Dynamic(etalon + "asd");
    NKIT_TEST_ASSERT(!vstr.EndsWith(etalon));
    Dynamic tmp = Dynamic(true);
    NKIT_TEST_ASSERT(!tmp.EndsWith(etalon));
    tmp = Dynamic(uint64_t(1));
    NKIT_TEST_ASSERT(!tmp.EndsWith(etalon));

    std::string str = "----------" + etalon;
    NKIT_TEST_ASSERT(ends_with(str, etalon.c_str()));
  }

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(DynamicReplace)
  {
    std::string from("ab"), to("abcd");
    Dynamic str1 = Dynamic(from + "---" + from + "+++" + from + "!");
    Dynamic str2 = Dynamic(to + "---" + to + "+++" + to + "!");
    str1.Replace(from, to);
    NKIT_TEST_ASSERT(str1 == str2);

    from = "abcd";
    to = "ab";
    str1 = Dynamic(from + "---" + from + "+++" + from + "!");
    str2 = Dynamic(to + "---" + to + "+++" + to + "!");
    str1.Replace(from, to);
    NKIT_TEST_ASSERT(str1 == str2);
  }
  //----------------------------------------------------------------------------
  void JustForCopy(Dynamic v)
  {
    std::cout << "Dynamic copy " << v.GetString() << '\n';
  }

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(DynamicAssignOperations)
  {
    Dynamic v_str("qweqeqe");
    Dynamic v_copy(uint16_t(32));
    v_copy = v_str;
    v_str = Dynamic();
    JustForCopy(v_copy);
    NKIT_TEST_ASSERT(v_copy.IsString());
    v_copy = Dynamic();

    Dynamic v_list(Dynamic::List());
    v_list.PushBack(Dynamic(uint64_t(1)));
    v_copy = Dynamic(uint16_t(32));
    v_copy = v_list;
    v_list = Dynamic();
    JustForCopy(v_copy);
    NKIT_TEST_ASSERT(v_copy.IsList());
    v_copy = Dynamic();

    Dynamic v_hash(Dynamic::Dict());
    v_hash["1"] = (Dynamic(uint64_t(1)));
    v_copy = Dynamic(uint16_t(32));
    v_copy = v_hash;
    v_hash = Dynamic();
    JustForCopy(v_copy);
    NKIT_TEST_ASSERT(v_copy.IsDict());
    v_copy = Dynamic();
  }

  NKIT_TEST_CASE(DynamicString)
  {
    std::string str1("str1");
    const char * str2 = "str2";
    const char * str3 = "str3";
    size_t len3 = strlen(str3);
    Dynamic v_str1(str1);
    NKIT_TEST_ASSERT(v_str1.GetConstString() == str1);

    Dynamic v_str2(str2);
    NKIT_TEST_ASSERT(v_str2.GetConstString() == std::string(str2));

    Dynamic v_str3(str3, len3);
    NKIT_TEST_ASSERT(v_str3.GetConstString() == std::string(str3));

    NKIT_TEST_ASSERT((v_str1 + v_str2).GetConstString() == (str1+str2));

    v_str1 += v_str2;
    NKIT_TEST_ASSERT(v_str1.GetConstString() == (str1+str2));
    NKIT_TEST_ASSERT(v_str1.StartsWith(str1));
    NKIT_TEST_ASSERT(v_str1.StartsWith(str1+str2));
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

  NKIT_TEST_CASE(DynamicEmpty)
  {
    std::string error;
    Dynamic d;
    NKIT_TEST_ASSERT(d.empty());

    Dynamic none = Dynamic(Dynamic::NONE_MARKER);
    NKIT_TEST_ASSERT(none.empty());
    none = Dynamic("qwe");
    NKIT_TEST_ASSERT(none.empty());

    d = Dynamic("qwe");
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic("");
    NKIT_TEST_ASSERT(d.empty());

    d = Dynamic::MongodbOID();
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic::MongodbOID("123456789012345678901234");
    NKIT_TEST_ASSERT(!d.empty());

    d = Dynamic(true);
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic(false);
    NKIT_TEST_ASSERT(!d.empty());

    d = Dynamic(2.3);
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic(0.0);
    NKIT_TEST_ASSERT(!d.empty());

    d = Dynamic(uint64_t(1));
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic(uint64_t(0));
    NKIT_TEST_ASSERT(!d.empty());

    d = Dynamic(uint32_t(1));
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic(uint32_t(0));
    NKIT_TEST_ASSERT(!d.empty());

    d = Dynamic(uint16_t(1));
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic(uint16_t(0));
    NKIT_TEST_ASSERT(!d.empty());

    d = Dynamic(uint8_t(1));
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic(uint8_t(0));
    NKIT_TEST_ASSERT(!d.empty());

    d = Dynamic(int64_t(-1));
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic(int64_t(0));
    NKIT_TEST_ASSERT(!d.empty());

    d = Dynamic(int32_t(-1));
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic(int32_t(0));
    NKIT_TEST_ASSERT(!d.empty());

    d = Dynamic(int16_t(-1));
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic(int16_t(0));
    NKIT_TEST_ASSERT(!d.empty());

    d = Dynamic(int8_t(-1));
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic(int8_t(0));
    NKIT_TEST_ASSERT(!d.empty());

    d = Dynamic::DateTimeFromDefault("2012-07-08 00:12:33", &error);
    NKIT_TEST_ASSERT(!d.empty());
    d = Dynamic::DateTimeFromDefault("0000-00-00 00:00:00", &error);
    NKIT_TEST_ASSERT(d.empty());

    d = DDICT("key" << "value");
    NKIT_TEST_ASSERT(!d.empty());
    d.Clear();
    NKIT_TEST_ASSERT(d.IsDict());
    NKIT_TEST_ASSERT(d.empty());

    d = DLIST("item1" << "item2");
    NKIT_TEST_ASSERT(!d.empty());
    d.Clear();
    NKIT_TEST_ASSERT(d.IsList());
    NKIT_TEST_ASSERT(d.empty());
  }

  NKIT_TEST_CASE(DynamicSize)
  {
    std::string error;
    std::string sample("qwe");

    Dynamic d;
    NKIT_TEST_ASSERT(d.size() == 0);

    Dynamic none = Dynamic(Dynamic::NONE_MARKER);
    NKIT_TEST_ASSERT(none.size() == 0);
    none = Dynamic(sample);
    NKIT_TEST_ASSERT(none.size() == 0);

    d = Dynamic(sample);
    NKIT_TEST_ASSERT(d.size() == sample.size());
    d = Dynamic("");
    NKIT_TEST_ASSERT(d.size() == 0);

    d = Dynamic::MongodbOID();
    NKIT_TEST_ASSERT(d.size() == 24);
    d = Dynamic::MongodbOID("123456789012345678901234");
    NKIT_TEST_ASSERT(d.size() == 24);

    d = Dynamic(true);
    NKIT_TEST_ASSERT(d.size() == 1);
    d = Dynamic(false);
    NKIT_TEST_ASSERT(d.size() == 1);

    d = Dynamic(2.3);
    NKIT_TEST_ASSERT(d.size() == 1);
    d = Dynamic(0.0);
    NKIT_TEST_ASSERT(d.size() == 1);

    d = Dynamic(uint64_t(1));
    NKIT_TEST_ASSERT(d.size() == 1);
    d = Dynamic(uint64_t(0));
    NKIT_TEST_ASSERT(d.size() == 1);

    d = Dynamic(uint32_t(1));
    NKIT_TEST_ASSERT(d.size() == 1);
    d = Dynamic(uint32_t(0));
    NKIT_TEST_ASSERT(d.size() == 1);

    d = Dynamic(uint16_t(1));
    NKIT_TEST_ASSERT(d.size() == 1);
    d = Dynamic(uint16_t(0));
    NKIT_TEST_ASSERT(d.size() == 1);

    d = Dynamic(uint8_t(1));
    NKIT_TEST_ASSERT(d.size() == 1);
    d = Dynamic(uint8_t(0));
    NKIT_TEST_ASSERT(d.size() == 1);

    d = Dynamic(int64_t(-1));
    NKIT_TEST_ASSERT(d.size() == 1);
    d = Dynamic(int64_t(0));
    NKIT_TEST_ASSERT(d.size() == 1);

    d = Dynamic(int32_t(-1));
    NKIT_TEST_ASSERT(d.size() == 1);
    d = Dynamic(int32_t(0));
    NKIT_TEST_ASSERT(d.size() == 1);

    d = Dynamic(int16_t(-1));
    NKIT_TEST_ASSERT(d.size() == 1);
    d = Dynamic(int16_t(0));
    NKIT_TEST_ASSERT(d.size() == 1);

    d = Dynamic(int8_t(-1));
    NKIT_TEST_ASSERT(d.size() == 1);
    d = Dynamic(int8_t(0));
    NKIT_TEST_ASSERT(d.size() == 1);

    d = Dynamic::DateTimeFromDefault("2012-07-08 00:12:33", &error);
    NKIT_TEST_ASSERT(d.size() == 1);
    d = Dynamic::DateTimeFromDefault("0000-00-00 00:00:00", &error);
    NKIT_TEST_ASSERT(d.size() == 0);

    d = DDICT("key1" << "value1" << "key2" << "value2");
    NKIT_TEST_ASSERT(d.size() == 2);
    d.Clear();
    NKIT_TEST_ASSERT(d.IsDict());
    NKIT_TEST_ASSERT(d.size() == 0);

    d = DLIST("item1" << "item2");
    NKIT_TEST_ASSERT(d.size() == 2);
    d.Clear();
    NKIT_TEST_ASSERT(d.IsList());
    NKIT_TEST_ASSERT(d.size() == 0);
  }

  NKIT_TEST_CASE(DynamicListPushPopFrontBack)
  {
    Dynamic item1("item1");
    Dynamic item2("item2");
    Dynamic item3("item3");
    Dynamic list(DLIST(item1 << item2));
    NKIT_TEST_ASSERT(list.size() == 2);
    NKIT_TEST_ASSERT(list.front() == item1);
    NKIT_TEST_ASSERT(list.back() == item2);

    list.PushBack(item3);
    NKIT_TEST_ASSERT(list.size() == 3);
    NKIT_TEST_ASSERT(list.front() == item1);
    NKIT_TEST_ASSERT(list.back() == item3);

    list.PopBack();
    NKIT_TEST_ASSERT(list.size() == 2);
    NKIT_TEST_ASSERT(list.front() == item1);
    NKIT_TEST_ASSERT(list.back() == item2);

    list.PushFront(item3);
    NKIT_TEST_ASSERT(list.size() == 3);
    NKIT_TEST_ASSERT(list.front() == item3);
    NKIT_TEST_ASSERT(list.back() == item2);

    list.PopFront();
    NKIT_TEST_ASSERT(list.size() == 2);
    NKIT_TEST_ASSERT(list.front() == item1);
    NKIT_TEST_ASSERT(list.back() == item2);
  }

  NKIT_TEST_CASE(DynamicListExtend)
  {
    Dynamic item1("item1");
    Dynamic item2("item2");
    Dynamic item3("item3");
    Dynamic list1(DLIST(item1 << item2));
    Dynamic list2(DLIST(item2 << item3));
    Dynamic list_check1(DLIST(item1 << item2 << item2 << item3));
    Dynamic list_check2(DLIST(item1 << item2
        << item2 << item3
        << item2 << item3));
    Dynamic list_check3(DLIST(item1 << item2
        << item2 << item3
        << item2 << item3
        << item2 << item3));

    list1 += list2;
    NKIT_TEST_ASSERT(list1 == list_check1);

    list1 += list2;
    NKIT_TEST_ASSERT(list1 == list_check2);

    list1.Extend(list2);
    NKIT_TEST_ASSERT(list1 == list_check3);
  }

  NKIT_TEST_CASE(DynamicListJoin)
  {
    Dynamic item1("item1");
    Dynamic item2("item2");
    Dynamic item3("item3");
    Dynamic list1(DLIST(item1 << item2 << item3));
    std::string delimiter("'"), prefix("prefix"), postfix("postfix"), out,
        out_check(prefix + item1.GetString() + postfix
            + delimiter + prefix + item2.GetString() + postfix
            + delimiter + prefix + item3.GetString() + postfix
            );
    list1.Join("'", prefix, postfix, &out);
    NKIT_TEST_ASSERT(out == out_check);

    item1 = Dynamic::DateTimeLocal();
    item2 = Dynamic::DateTimeGmt();
    list1 = DLIST(item1 << item2);
    out_check = prefix + item1.GetString(DATE_TIME_DEFAULT_FORMAT_) + postfix
        + delimiter
        + prefix + item2.GetString(DATE_TIME_DEFAULT_FORMAT_) + postfix;
    out.clear();
    list1.Join("'", prefix, postfix, DATE_TIME_DEFAULT_FORMAT_, &out);
    NKIT_TEST_ASSERT(out == out_check);
  }

  NKIT_TEST_CASE(DynamicListJoinFormatter)
  {
    Dynamic item1("it$e,m.1");
    Dynamic item2("i$te,m.2");
    Dynamic item3("$ite,m.3");
    Dynamic list1(DLIST(item1 << item2 << item3));
    std::string delimiter("'"), prefix("prefix"), postfix("postfix"), out,
            etalog(prefix + "item1" + postfix
                + delimiter + prefix + "item2" + postfix
                + delimiter + prefix + "item3" + postfix
                );
    CharEraser char_eraser("$,.");
    list1.Join("'", prefix, postfix, char_eraser, &out);
    NKIT_TEST_ASSERT(out == etalog);
  }

  NKIT_TEST_CASE(DynamicListIndex)
  {
    Dynamic item1("Item 1");
    Dynamic item2("Item 2");
    Dynamic item3("элеМенТ 3");
    std::string iitem3(item3.GetString());
    std::transform(iitem3.begin(), iitem3.end(), iitem3.begin(), ::tolower);
    Dynamic item4(true);
    size_t i1(0), i2(1), i3(2);
    Dynamic list1(DLIST(item1 << item2 << item3));

    NKIT_TEST_ASSERT(list1.IndexOf(item1) == 0);
    NKIT_TEST_ASSERT(list1.IndexOf(item2) == 1);
    NKIT_TEST_ASSERT(list1.IndexOf(item3) == 2);

    NKIT_TEST_ASSERT(list1.GetByIndex(0) == item1);
    NKIT_TEST_ASSERT(list1.GetByIndex(1) == item2);
    NKIT_TEST_ASSERT(list1.GetByIndex(2) == item3);

    NKIT_TEST_ASSERT(list1[i1] == item1);
    NKIT_TEST_ASSERT(list1[i2] == item2);
    NKIT_TEST_ASSERT(list1[i3] == item3);

    list1[i1] = item4;
    NKIT_TEST_ASSERT(list1[i1] == item4);
    NKIT_TEST_ASSERT(list1[i2] == item2);
    NKIT_TEST_ASSERT(list1[i3] == item3);

    list1.GetByIndex(i1) = item1;
    NKIT_TEST_ASSERT(list1[i1] == item1);
    NKIT_TEST_ASSERT(list1[i2] == item2);
    NKIT_TEST_ASSERT(list1[i3] == item3);

    list1.GetByIndex(i1).Swap(item4);
    NKIT_TEST_ASSERT(item4.IsString());
    NKIT_TEST_ASSERT(list1.GetByIndex(i1).IsBool());
    list1.GetByIndex(i1).Swap(item4);
    NKIT_TEST_ASSERT(list1.GetByIndex(i1).IsString());
    NKIT_TEST_ASSERT(list1[i1] == item1);
    NKIT_TEST_ASSERT(list1[i2] == item2);
    NKIT_TEST_ASSERT(list1[i3] == item3);

  NKIT_TEST_ASSERT(list1.IIndexOf(item3.GetString()) == i3);
    NKIT_TEST_ASSERT(list1.IIndexOf(iitem3) == i3);
    NKIT_TEST_ASSERT(list1.IIndexOf("qwe") == Dynamic::npos);
  }

  NKIT_TEST_CASE(DynamicDict)
  {
    const char * ch1 = "k1";
    const char * ch2 = "k2";
    const char * ch3 = "k3";
    std::string k1(ch1), k2(ch2), k3(ch3);
    Dynamic v1("item1");
    Dynamic v2(true);
    Dynamic v3(1);
    Dynamic hash1 = DDICT(k1 << v1
        << k2 << v2
        << k3 << v3);

    NKIT_TEST_ASSERT(hash1[ch1] == hash1[k1]);
    NKIT_TEST_ASSERT(v1 == hash1[k1]);
    NKIT_TEST_ASSERT(hash1[ch2] == hash1[k2]);
    NKIT_TEST_ASSERT(v2 == hash1[k2]);
    NKIT_TEST_ASSERT(hash1[ch3] == hash1[k3]);
    NKIT_TEST_ASSERT(v3 == hash1[k3]);

    NKIT_TEST_ASSERT(hash1.Get(ch1) == hash1.Get(k1));
    NKIT_TEST_ASSERT(v1 == hash1.Get(k1));
    NKIT_TEST_ASSERT(hash1.Get(ch2) == hash1.Get(k2));
    NKIT_TEST_ASSERT(v2 == hash1.Get(k2));
    NKIT_TEST_ASSERT(hash1.Get(ch3) == hash1.Get(k3));
    NKIT_TEST_ASSERT(v3 == hash1.Get(k3));

    Dynamic * test1, * test2;
    NKIT_TEST_ASSERT(hash1.Get(ch1, &test1));
    NKIT_TEST_ASSERT(hash1.Get(k1, &test2));
    NKIT_TEST_ASSERT(*test1 == *test2);
    NKIT_TEST_ASSERT(*test1 == v1);

    NKIT_TEST_ASSERT(hash1.Get(ch2, &test1));
    NKIT_TEST_ASSERT(hash1.Get(k2, &test2));
    NKIT_TEST_ASSERT(*test1 == *test2);
    NKIT_TEST_ASSERT(*test1 == v2);

    NKIT_TEST_ASSERT(hash1.Get(ch3, &test1));
    NKIT_TEST_ASSERT(hash1.Get(k3, &test2));
    NKIT_TEST_ASSERT(*test1 == *test2);
    NKIT_TEST_ASSERT(*test1 == v3);

    hash1.Erase(ch1);
    NKIT_TEST_ASSERT(!hash1.Get(ch1, &test1));
    hash1.Update(ch1, v1);
    NKIT_TEST_ASSERT(hash1[ch1] == v1);

    hash1.Erase(k1);
    NKIT_TEST_ASSERT(!hash1.Get(k1, &test1));
    hash1.Update(k1, v1);
    NKIT_TEST_ASSERT(hash1[k1] == v1);

    hash1.Update(k1, v2);
    NKIT_TEST_ASSERT(hash1[ch1] == v2);

    hash1.Update(ch1, v3);
    NKIT_TEST_ASSERT(hash1[k1] == v3);

    hash1.Update(ch1, v1);

    // skin-deep Update
    const char * _ch1 = "_k1";
    const char * _ch2 = "_k2";
    const char * _ch3 = "_k3";
    std::string _k1(_ch1), _k2(_ch2), _k3(_ch3);
    Dynamic _v1("_item1");
    Dynamic _v2(false);
    Dynamic _v3(-1);
    Dynamic hash2 = DDICT(_k1 << _v1
        << _k2 << _v2
        << _k3 << _v3);
    hash2.Update(hash1);
    NKIT_TEST_ASSERT(hash2[k1] == v1);
    NKIT_TEST_ASSERT(hash2[k2] == v2);
    NKIT_TEST_ASSERT(hash2[k3] == v3);
    NKIT_TEST_ASSERT(hash2[_k1] == _v1);
    NKIT_TEST_ASSERT(hash2[_k2] == _v2);
    NKIT_TEST_ASSERT(hash2[_k3] == _v3);

    // GetKeys
    StringSet keys;
    hash2.GetKeys(&keys);
    std::string joined_keys, check_keys(_k1 + _k2 + _k3 + k1 + k2 + k3);
    nkit::join(keys, "", "", "", &joined_keys);
    NKIT_TEST_ASSERT(joined_keys == check_keys);

    // deep Update
    hash1 = DDICT(
           _k1 << DDICT(_k1 << 11
                     << _k2 << DDICT(_k1 << 33)
                     << _k3 << DLIST(_k1 << 31))
        << _k2 << DDICT(_k1 << 21)
        << _k3 << DDICT(_k1 << 31)
        );

    hash2 = DDICT(
           _k1 << DDICT(_k1 << 111
                     << _k2 << DDICT(_k1 << 333))
        << _k2 << DLIST(_k1 << 21)
        << _k3 << true
        );

    hash1.Update(hash2);
    NKIT_TEST_ASSERT(hash1[_k1][_k1] == hash2[_k1][_k1]);
    NKIT_TEST_ASSERT(hash1[_k1][_k2][_k1] == hash2[_k1][_k2][_k1]);
    NKIT_TEST_ASSERT(hash1[_k1] != hash2[_k1]);
    NKIT_TEST_ASSERT(hash1[_k2] == hash2[_k2]);
    NKIT_TEST_ASSERT(hash1[_k3] == hash2[_k3]);
  }

  NKIT_TEST_CASE(DynamicForEach)
  {
    Dynamic item1("Item 1");
    Dynamic item2("Item 2");
    Dynamic item3("элеМенТ 3");

    std::cout << "----------------------------------" << std::endl;
    Dynamic list1(DLIST(item1 << item2 << item3));
    DLIST_FOREACH(item, list1)
    {
      CINFO(*item);
    }
    std::cout << "----------------------------------" << std::endl;

    const Dynamic list2(list1);
    DLIST_FOREACH(item, list2)
    {
      CINFO(*item);
      //item.Clear();
    }
    std::cout << "----------------------------------" << std::endl;

    const char * ch1 = "k1";
    const char * ch2 = "k2";
    const char * ch3 = "k3";
    std::string k1(ch1), k2(ch2), k3(ch3);
    Dynamic v1("item1");
    Dynamic v2(true);
    Dynamic v3(1);

    Dynamic hash1 = DDICT(k1 << v1
        << k2 << v2
        << k3 << v3);
    DDICT_FOREACH(item, hash1)
    {
      std::cout << item->first << std::endl;
      CINFO(item->second);
    }
    std::cout << "----------------------------------" << std::endl;

    const Dynamic hash2(hash1);
    DDICT_FOREACH(item, hash2)
    {
      std::cout << item->first << std::endl;
      CINFO(item->second);
      //item.second.Clear();
    }
    std::cout << "----------------------------------" << std::endl;

    Dynamic check_list3 = Dynamic::List();
    Dynamic list3 = DLIST(hash1 << hash2);
    DLIST_FOREACH(item, list3)
    {
      Dynamic hash = Dynamic::Dict();
      DDICT_FOREACH(sub_item, *item)
      {
        hash[sub_item->first]= sub_item->second;
      }
      check_list3.PushBack(hash);
    }
    NKIT_TEST_ASSERT(check_list3 == list3);

    Dynamic list4 = DLIST(1 << 2);
    NKIT_TEST_ASSERT(list4.size() == 2);
    DLIST_FOREACH_MUTABLE(item, list4)
    {
      *item += Dynamic(1);
    }
    NKIT_TEST_ASSERT(list4.size() == 2);
    size_t index(0);
    NKIT_TEST_ASSERT(list4[index] != Dynamic(1));
    NKIT_TEST_ASSERT(list4[index] == Dynamic(2));
    ++index;
    NKIT_TEST_ASSERT(list4[index] != Dynamic(2));
    NKIT_TEST_ASSERT(list4[index] == Dynamic(3));
  }

  NKIT_TEST_CASE(DynamicListErase)
  {
    Dynamic list = DLIST(0 << 1 << 2 << 3 << 4);
    SizeSet pos_set;
    pos_set.insert(1);
    pos_set.insert(3);
    list.Erase(pos_set);
    size_t index(0);
    NKIT_TEST_ASSERT(list.size() == 3);
    NKIT_TEST_ASSERT(list[index++] == Dynamic(0));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(2));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(4));

    list = DLIST(0 << 1 << 2 << 3 << 4);
    list.Erase(1, 10);
    NKIT_TEST_ASSERT(list.size() == 1);
    index = 0;
    NKIT_TEST_ASSERT(list[index++] == Dynamic(0));

    list = DLIST(0 << 1 << 2 << 3 << 4);
    list.Erase(1, 5);
    NKIT_TEST_ASSERT(list.size() == 1);
    index = 0;
    NKIT_TEST_ASSERT(list[index++] == Dynamic(0));

    list = DLIST(0 << 1 << 2 << 3 << 4);
    list.Erase(1, 3);
    NKIT_TEST_ASSERT(list.size() == 3);
    index = 0;
    NKIT_TEST_ASSERT(list[index++] == Dynamic(0));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(3));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(4));

    list = DLIST(0 << 1 << 2 << 3 << 4);
    list.Erase(4, 6);
    NKIT_TEST_ASSERT(list.size() == 4);
    index = 0;
    NKIT_TEST_ASSERT(list[index++] == Dynamic(0));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(1));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(2));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(3));

    list = DLIST(0 << 1 << 2 << 3 << 4);
    list.Erase(5, 6);
    NKIT_TEST_ASSERT(list.size() == 5);
    index = 0;
    NKIT_TEST_ASSERT(list[index++] == Dynamic(0));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(1));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(2));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(3));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(4));

    list = DLIST(0 << 1 << 2 << 3 << 4);
    list.Erase(50, 53);
    NKIT_TEST_ASSERT(list.size() == 5);
    index = 0;
    NKIT_TEST_ASSERT(list[index++] == Dynamic(0));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(1));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(2));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(3));
    NKIT_TEST_ASSERT(list[index++] == Dynamic(4));
  }
}

int main(int NKIT_UNUSED(argc), char ** NKIT_UNUSED(argv))
{
#ifdef _WIN32
#ifdef _DEBUG
  int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag(tmpFlag);
#endif
#endif

  return nkit::test::run_all_tests();
}

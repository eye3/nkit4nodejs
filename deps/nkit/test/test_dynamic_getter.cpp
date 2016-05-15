/*
   Copyright 2010-2015 Boris T. Darchiev (boris.darchiev@gmail.com)

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

  // Format:
  // NKIT_OPTION(type, prop_name, option_name)
  // NKIT_OPTION_DEFAULT(type, prop_name, option_name, default_value)

  #define NKIT_OPTIONS_CLASS_NAME Address
  NKIT_OPTIONS_BEGIN
    NKIT_OPTION(std::string,                street,   "street")
    NKIT_OPTION(uint64_t,                   no,       "no")
    NKIT_OPTION(uint64_t,                   corpus,   "corpus")
    NKIT_OPTION(uint64_t,                   flat_no,  "flat_no")
  NKIT_OPTIONS_END
  #undef NKIT_OPTIONS_CLASS_NAME

  #define NKIT_OPTIONS_CLASS_NAME Person1
  NKIT_OPTIONS_BEGIN
    NKIT_OPTION(bool,                       male,     "male")
    NKIT_OPTION(std::string,                name,     "name")
    NKIT_OPTION(std::vector<Address::Ptr>,  address,  "address")
    NKIT_OPTION_DEFAULT(uint16_t,           age,      "age",    17)
  NKIT_OPTIONS_END
  #undef NKIT_OPTIONS_CLASS_NAME

  #define NKIT_OPTIONS_CLASS_NAME Person2
  NKIT_OPTIONS_BEGIN
    NKIT_OPTION(bool,                       male,     "male")
    NKIT_OPTION(std::string,                name,     "name")
    NKIT_OPTION(std::list<Address::Ptr>,    address,  "address")
    NKIT_OPTION_DEFAULT(uint16_t,           age,      "age",    18)
  NKIT_OPTIONS_END
  #undef NKIT_OPTIONS_CLASS_NAME

  typedef std::map<std::string, Address::Ptr> AddressMap;

  //--------------------------------------------------------------------------
  NKIT_TEST_CASE(OptionsPathWithDot)
  {
    Dynamic etalon = DDICT(
      "address_map" << DDICT(
            "asia.0" << DDICT(
              "street" << "Lenina"
           << "no" << 93
           << "corpus" << 4
           << "flat_no" << 162
           )
         << "ipe.1" << DDICT(
              "street" << "Kirova"
           << "no" << 3
           << "corpus" << 44
           << "flat_no" << 58
           )
       )
    );

    AddressMap address_map;
    DynamicGetter getter(etalon);
    getter.delimiter('/');
    getter.Get("/address_map", &address_map);
    NKIT_TEST_ASSERT_WITH_TEXT(getter.ok(), getter.error());
    NKIT_TEST_EQ(address_map["asia.0"]->street(), "Lenina");
    NKIT_TEST_EQ(address_map["asia.0"]->no(), 93);
    NKIT_TEST_EQ(address_map["asia.0"]->corpus(), 4);
    NKIT_TEST_EQ(address_map["asia.0"]->flat_no(), 162);

    NKIT_TEST_EQ(address_map["ipe.1"]->street(), "Kirova");
    NKIT_TEST_EQ(address_map["ipe.1"]->no(), 3);
    NKIT_TEST_EQ(address_map["ipe.1"]->corpus(), 44);
    NKIT_TEST_EQ(address_map["ipe.1"]->flat_no(), 58);
  }

  NKIT_TEST_CASE(Options)
  {
    Dynamic etalon = DDICT(
           "name" << "Boris"
        << "male" << true
        << "age" << 45
        << "address" << DLIST(
             DDICT(
                "street" << "Lenina"
             << "no" << 93
             << "corpus" << 4
             << "flat_no" << 162
             )
           )
        );

    DynamicGetter getter(etalon);
    Person1::Ptr p1 = Person1::Create(getter);
    if (!p1)
    {
      CERR(getter.error());
      return;
    }

    Dynamic result = p1->SaveToDynamic();
    NKIT_TEST_EQ(etalon, result);

    p1 = Person1::Create();
    p1->name("Olya");
    p1->male(false);
    Address::Ptr a = Address::Create();
    a->street("Kirova");
    std::vector<Address::Ptr> vv;
    vv.push_back(a);
    p1->address(vv);
    result = p1->SaveToDynamic();
    NKIT_TEST_EQ(result["age"], Dynamic(17));
    NKIT_TEST_EQ(result["male"], Dynamic(false));
    NKIT_TEST_EQ(result["name"], Dynamic("Olya"));
    NKIT_TEST_ASSERT(result["address"].IsList());
    NKIT_TEST_EQ(result["address"].size(), 1);
    NKIT_TEST_EQ(result["address"][size_t(0)]["street"], Dynamic("Kirova"));

    Person2::Ptr p2 = Person2::Create();
    p2->male(true);
    p2->name("Victor");
    std::list<Address::Ptr> ll;
    a = Address::Create();
    a->street("Mira");
    ll.push_back(a);
    p2->address(ll);
    result = p2->SaveToDynamic();
    NKIT_TEST_EQ(result["age"], Dynamic(18));
    NKIT_TEST_EQ(result["male"], Dynamic(true));
    NKIT_TEST_EQ(result["name"], Dynamic("Victor"));
    NKIT_TEST_ASSERT(result["address"].IsList());
    NKIT_TEST_EQ(result["address"].size(), 1);
    NKIT_TEST_EQ(result["address"][size_t(0)]["street"], Dynamic("Mira"));
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

}  // namespace nkit_test

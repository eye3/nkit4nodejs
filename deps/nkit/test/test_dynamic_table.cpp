/*
   Copyright 2014 Vasiliy Soshnikov (dedok.mad@gmail.com)

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
#include "nkit/logger_brief.h"
#include "nkit/version.h"

#define TABLE_GROW_SIZE 10

namespace nkit_test
{
  using namespace nkit;

  void CreateCases()
  {
    std::string error;

    { // StringVector ctor
      StringVector column_names;
      column_names.push_back("name:STRING");
      column_names.push_back("name1:INTEGER");
      column_names.push_back("name2:BOOL");
      column_names.push_back("name3:INTEGER");
      column_names.push_back("name4:FLOAT");

      for (size_t i = 0; i < TABLE_GROW_SIZE; ++i)
      {
        std::string const v = "name" + string_cast(i) + "STRING";
        column_names.push_back(v);
      }
      Dynamic table = Dynamic::Table(column_names, &error);
      NKIT_TEST_ASSERT_WITH_TEXT(table.width() == column_names.size(), error);
    }

    { // query syntax ctor
      std::string query(
          "name:STRING,name1:INTEGER,name2:BOOL,name3:INTEGER,name4:FLOAT");
      Dynamic table = Dynamic::Table(query, &error);
      NKIT_TEST_ASSERT(table.width() == 5);
    }
  }

  #define TABLE Dynamic::Table("name:STRING,name1:INTEGER,name2:BOOL,name3:INTEGER,name4:FLOAT", &error)

  #define COMMON_TABLE_INIT_(t) \
  do { \
    t = TABLE;\
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A3"), Dynamic(3), Dynamic(false), Dynamic(1), Dynamic(1.0))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A1"), Dynamic(1), Dynamic(false))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A2"), Dynamic(2))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A"), Dynamic(1), Dynamic(false), Dynamic(1), Dynamic(99.0))); \
  } while(0)

  #define COMMON_TABLE_INIT_2(t) \
  do { \
    t = TABLE;\
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A3"), Dynamic(10), Dynamic(false), Dynamic(1), Dynamic(100.0))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A1"), Dynamic(1), Dynamic(false))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A2"), Dynamic(50))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A"), Dynamic(20), Dynamic(false), Dynamic(1), Dynamic(200.0))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A"), Dynamic(20), Dynamic(false), Dynamic(1), Dynamic(200.0))); \
  } while(0)

  #define _COMMON_TABLE_INIT_(t) \
  do { \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A3"), Dynamic(3), Dynamic(false), Dynamic(1), Dynamic(1.0))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A1"), Dynamic(1), Dynamic(false))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A2"), Dynamic(2))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A"), Dynamic(1), Dynamic(false), Dynamic(1), Dynamic(99.0))); \
  } while(0)

  #define _COMMON_TABLE_INIT_2(t) \
  do { \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A3"), Dynamic(10), Dynamic(false), Dynamic(1), Dynamic(100.0))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A1"), Dynamic(1), Dynamic(false))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A2"), Dynamic(50))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A"), Dynamic(20), Dynamic(false), Dynamic(1), Dynamic(200.0))); \
    NKIT_TEST_ASSERT(t.AppendRow(Dynamic("A"), Dynamic(20), Dynamic(false), Dynamic(1), Dynamic(200.0))); \
  } while(0)

  #define COMMON_TABLE_INIT(t) \
  do { \
    COMMON_TABLE_INIT_(t); \
    NKIT_TEST_ASSERT(t == e); \
  } while(0)

  #define _COMMON_TABLE_INIT(t) \
  do { \
    _COMMON_TABLE_INIT_(t); \
    NKIT_TEST_ASSERT(t == e); \
  } while(0)

  static Dynamic e;

  void EnvInit()
  {
    std::string error;
    COMMON_TABLE_INIT(e);
  }

  void AppendCases()
  {
    std::string error;
    Dynamic table = TABLE;
    nkit::TableIndex::Ptr index;
    nkit::TableIndex::ConstIterator it;

    COMMON_TABLE_INIT(table);

    // Fail cases tests
    for (uint64_t ops = 0; ops < TABLE_GROW_SIZE; ++ops) // wrong type
       NKIT_TEST_ASSERT(table.AppendRow(Dynamic(ops), Dynamic(3), Dynamic(false), Dynamic(1)) ==
          false);

    NKIT_TEST_ASSERT(table == e);

    NKIT_TEST_ASSERT(!table.CreateIndex("", &error));
    NKIT_TEST_ASSERT(!table.CreateIndex("name,name1,name10", &error));

    index = table.CreateIndex("name,name1,name3", &error);
    NKIT_TEST_ASSERT(index);
    NKIT_TEST_ASSERT(index->GetEqual(Dynamic(3), Dynamic(1)) == index->end());
    NKIT_TEST_ASSERT(index->GetEqual(Dynamic("A3"), Dynamic(3)) == index->end());

    // Good cases test
    it = index->GetEqual(Dynamic("A3"), Dynamic(3), Dynamic(1));
    NKIT_TEST_ASSERT(it[4] == Dynamic(1.0));

    for (size_t ops = 0; ops < TABLE_GROW_SIZE; ++ops)
      NKIT_TEST_ASSERT(table.AppendRow(Dynamic("A3"), Dynamic(3),
          Dynamic(false), Dynamic(1), Dynamic(double(2.0 + ops))));

    it = index->GetEqual(Dynamic("A3"), Dynamic(3), Dynamic(1));
    NKIT_TEST_ASSERT(it != index->end());
    for (; it != index->end(); ++it)
    {
  #if defined(MY_TRACE_)
        print(it[4]);
  #endif // MY_TRACE_
    }
  }

  void InsertCases()
  {
    std::string error;
    Dynamic table = TABLE;
    nkit::TableIndex::Ptr index;
    nkit::TableIndex::ConstIterator it;

    COMMON_TABLE_INIT(table);

    // Fail cases tests
    for (size_t ops = 0; ops < TABLE_GROW_SIZE; ++ops) // wrong type
    {
      DynamicVector vargs;
      vargs.push_back(Dynamic(1));
      vargs.push_back(Dynamic(3));
      vargs.push_back(Dynamic(false));
      vargs.push_back(Dynamic(1));
      vargs.push_back(Dynamic("name"));
      NKIT_TEST_ASSERT(table.InsertRow(0, vargs) == false);
    }
    NKIT_TEST_ASSERT(table == e);

    index = table.CreateIndex("name,name1,name3", &error);
    NKIT_TEST_ASSERT(index);
    NKIT_TEST_ASSERT(index->GetEqual(Dynamic(3), Dynamic(1)) == index->end());
    NKIT_TEST_ASSERT(index->GetEqual(Dynamic("A3"), Dynamic(3)) == index->end());

    // Good cases tests
    for (size_t ops = 0; ops < TABLE_GROW_SIZE; ++ops)
    {
      DynamicVector vargs;
      vargs.push_back(Dynamic("A3"));
      vargs.push_back(Dynamic(3));
      vargs.push_back(Dynamic(false));
      vargs.push_back(Dynamic(1));
      vargs.push_back(Dynamic(double(2.0 + ops)));
      NKIT_TEST_ASSERT(table.InsertRow(0, vargs));
    }
    it = index->GetEqual(Dynamic("A3"), Dynamic(3), Dynamic(1));
    NKIT_TEST_ASSERT(it != index->end());
    for (; it != index->end(); ++it)
    {
  #if defined(MY_TRACE_)
        print(it[4]);
  #endif // MY_TRACE_
    }

    for (size_t ops = 0; ops < TABLE_GROW_SIZE; ++ops)
    {
      DynamicVector vargs;
      vargs.push_back(Dynamic("A10"));
      vargs.push_back(Dynamic(3));
      vargs.push_back(Dynamic(false));
      vargs.push_back(Dynamic(1));
      vargs.push_back(Dynamic(double(1.0 + ops)));
      NKIT_TEST_ASSERT(table.InsertRow(table.height() / 2, vargs));
    }

    it = index->GetEqual(Dynamic("A10"), Dynamic(3), Dynamic(1));
    NKIT_TEST_ASSERT(it != index->end());
    for (; it != index->end(); ++it)
    {
  #if defined(MY_TRACE_)
        print(it[4]);
  #endif // MY_TRACE_
    }

    it = index->GetEqual(Dynamic("A"), Dynamic(1), Dynamic(1));
    NKIT_TEST_ASSERT(it != index->end());
    NKIT_TEST_ASSERT(it[4] == Dynamic(99.0));

    it = index->GetEqual(Dynamic("A2"), Dynamic(2), Dynamic::GetDefault(detail::INTEGER));
    NKIT_TEST_ASSERT(it != index->end());

    it = index->GetEqual(Dynamic("A1"), Dynamic(1), Dynamic::GetDefault(detail::INTEGER));
    NKIT_TEST_ASSERT(it != index->end());
  }

  void DeleteCases()
  {
    std::string error;
    Dynamic table = TABLE;
    nkit::TableIndex::Ptr index;
    nkit::TableIndex::ConstIterator it;

    COMMON_TABLE_INIT(table);

    index = table.CreateIndex("name,name1,name3", &error);

    // Fail cases tests
    NKIT_TEST_ASSERT(index);
    NKIT_TEST_ASSERT(index->GetEqual(Dynamic(3), Dynamic(1)) == index->end());
    NKIT_TEST_ASSERT(index->GetEqual(Dynamic("A3"), Dynamic(3)) == index->end());

    // Good cases tests
    for (size_t ops = 0; ops < TABLE_GROW_SIZE; ++ops)
    {
      DynamicVector vargs;
      vargs.push_back(Dynamic("A3"));
      vargs.push_back(Dynamic(3));
      vargs.push_back(Dynamic(false));
      vargs.push_back(Dynamic(1));
      vargs.push_back(Dynamic(double(2.0 + ops)));
      NKIT_TEST_ASSERT(table.InsertRow(0, vargs));
    }

    for (size_t ops = 0; ops < TABLE_GROW_SIZE; ++ops)
      NKIT_TEST_ASSERT(table.DeleteRow(0));

    it = index->GetEqual(Dynamic("A3"), Dynamic(3), Dynamic(1));
    NKIT_TEST_ASSERT(it != index->end());
    for (; it != index->end(); ++it)
    {
  #if defined(MY_TRACE_)
        print(it[4]);
  #endif // MY_TRACE_
    }

    NKIT_TEST_ASSERT(table == e);
  }

  void SetCases()
  {
    std::string error;
    Dynamic table = TABLE;
    nkit::TableIndex::Ptr index;
    nkit::TableIndex::ConstIterator it;

    DynamicVector vargs;
    vargs.push_back(Dynamic("NEW"));
    vargs.push_back(Dynamic(0));
    vargs.push_back(Dynamic(true));
    vargs.push_back(Dynamic(0));
    vargs.push_back(Dynamic(0.0));

    COMMON_TABLE_INIT(table);

    index = table.CreateIndex("name,name1", &error);
    NKIT_TEST_ASSERT_WITH_TEXT(index, error);

    NKIT_TEST_ASSERT(table.SetRow(table.height() / 2, vargs));
    DynamicVector::iterator vit = vargs.begin();
    for (size_t col = 0; vit != vargs.end(); ++vit, ++col)
    {
  #if defined(MY_TRACE_)
    print(*vit, "", false);
    std::cout << " <> ";
    print(table.GetCellValue(table.height() / 2, col));
  #endif // MY_TRACE_
      NKIT_TEST_ASSERT(*vit == table.GetCellValue(table.height() / 2, col));
    } // for
    NKIT_TEST_ASSERT(index->GetEqual(Dynamic("NEW"),Dynamic(0)) != index->end());
  }
/*
#define MY_TRACE_

  void PrintTable(const Dynamic & table)
  {
  #if !defined(MY_TRACE_)
    (void)table;
  #else
    size_t shift_size = 0;
    StringVector cols = table.GetColumnNames();
    StringVector::const_iterator beg = cols.begin();

    for (; beg != cols.end(); ++beg)
    {
      if (beg->size() > shift_size)
        shift_size = beg->size();
    }

    std::string shift;
    beg = cols.begin();
    for (; beg != cols.end(); ++beg)
    {
      std::cout << shift << " | " << *beg;
      shift = std::string(shift_size, ' ');
    }
    std::cout << std::endl;

    std::string shift_;
    const size_t h_size = table.height(), w_size = table.width();
    for (size_t row = 0; row < h_size; ++row)
    {
      for (size_t col = 0; col < w_size; ++col)
      {
        //if (col > 0)
        //  shift_ = std::string(shift_size, ' '); //shift_ = shift.size();
        print(std::cout, table.GetCellValue(row, col), shift_ + " | ", false);
      }
      std::cout << std::endl;
    }
  #endif // MY_TRACE_
  }
*/
  void GroupCases()
  {
    std::string error;
    Dynamic table = TABLE;

    _COMMON_TABLE_INIT(table);
    _COMMON_TABLE_INIT_(table);
    _COMMON_TABLE_INIT_2(table);
    /*
       Error cases
    */
    {
      std::string error;
      Dynamic group_table(table.Group("",
          "COUNT, SUM(name4)", &error));
      NKIT_TEST_ASSERT(!error.empty());
    }
    // --------------------
    {
      std::string error;
      Dynamic group_table(table.Group("name,name1",
          "", &error));
      NKIT_TEST_ASSERT(!error.empty());
    }
    // --------------------
    {
      std::string error;
      Dynamic group_table(table.Group("name,name1,__NOT_EXISTS__",
          "COUNT, SUM(name4)", &error));
      NKIT_TEST_ASSERT(!error.empty());
    }
    // --------------------
    {
      std::string error;
      Dynamic group_table(table.Group("name,name1,name3",
          "COUNT, SUM(name4),__WRONG_AGGREGATOR__(name3)", &error));
      NKIT_TEST_ASSERT(!error.empty());
    }
    // --------------------
    {
      std::string error;
      Dynamic group_table(table.Group("name,name1,name3",
          "COUNT, MAX(name)", &error)); // string -> MAX
      NKIT_TEST_ASSERT(!error.empty());
    }
    /*
       Good cases
     */
    {
      std::string error;
      //CINFO(table);
      Dynamic group_table(table.Group("name3",
        "COUNT, SUM(name4)", &error));
      NKIT_TEST_ASSERT_WITH_TEXT(error.empty(), error);
      //CINFO(group_table);
    }
    {
      std::string error;
      Dynamic group_table(table.Group("name,name3",
        "COUNT, SUM(name4),MAX(name4)", &error));
      NKIT_TEST_ASSERT_WITH_TEXT(error.empty(), error);
      //CINFO(group_table);
    }
    {
      CINFO(table);
      std::string error;
      Dynamic group_table(table.Group("name,name1,name3",
        "  COUNT, SUM(name4),MIN(name1), MAX(name4)   ", &error));
      NKIT_TEST_ASSERT_WITH_TEXT(error.empty(), error);
      //CINFO(group_table);

      // | STRING     | INTEGER     | INTEGER     | INTEGER     | FLOAT     | INTEGER     | FLOAT
      Dynamic et(Dynamic::Table("name:STRING,name1:INTEGER,name2:INTEGER,COUNT:INTEGER,SUM:FLOAT,MIN:INTEGER,MAX:FLOAT", &error));
      // | A3         | 3           | 1           | 2           | 2.000000  | 0           | 1.000000
      NKIT_TEST_ASSERT(et.AppendRow(Dynamic("A3"), Dynamic(3), Dynamic(1), Dynamic(2), Dynamic(2.0), Dynamic(0), Dynamic(1.0)));
      // | A1         | 1           | 0           | 3           | 0.000000  | 0           | 0.000000
      NKIT_TEST_ASSERT(et.AppendRow(Dynamic("A1"), Dynamic(1), Dynamic(0), Dynamic(3), Dynamic(0.0), Dynamic(0), Dynamic(0.0)));
      // | A2         | 2           | 0           | 2           | 0.000000  | 0           | 0.000000
      NKIT_TEST_ASSERT(et.AppendRow(Dynamic("A2"), Dynamic(2), Dynamic(0), Dynamic(2), Dynamic(0.0), Dynamic(0), Dynamic(0.0)));
      // | A | 1 | 1 | 2 | 198.000000 | 0 | 99.000000
      NKIT_TEST_ASSERT(et.AppendRow(Dynamic("A"), Dynamic(1), Dynamic(1), Dynamic(2), Dynamic(198.0), Dynamic(0), Dynamic(99.0)));
      // | A3 | 10 | 1 | 1 | 100.000000 | 0 | 100.000000
      NKIT_TEST_ASSERT(et.AppendRow(Dynamic("A3"), Dynamic(10), Dynamic(1), Dynamic(1), Dynamic(100.0), Dynamic(0), Dynamic(100.0)));
      // | A2 | 50 | 0 | 1 | 0.000000 | 0 | 0.000000
      NKIT_TEST_ASSERT(et.AppendRow(Dynamic("A2"), Dynamic(50), Dynamic(0), Dynamic(1), Dynamic(0.0), Dynamic(0), Dynamic(0.0)));
      // | A | 20 | 1 | 2 | 400.000000 | 0 | 200.000000
      NKIT_TEST_ASSERT(et.AppendRow(Dynamic("A"), Dynamic(20), Dynamic(1), Dynamic(2), Dynamic(400.0), Dynamic(0), Dynamic(200.0)));

      CINFO(group_table);
      CINFO(et);

      NKIT_TEST_ASSERT(group_table == et);
    }
  }

  void TableIteratorCases()
  {
    std::string error;
    Dynamic table = TABLE;
    COMMON_TABLE_INIT(table);
    Dynamic::TableIterator it = table.begin_t();
    Dynamic::TableIterator end = table.end_t();

    for (size_t row = 0; it != end; ++it, ++row)
    {
      for (size_t col = 0; col < table.width(); ++col)
        NKIT_TEST_ASSERT(it[col] == table.GetCellValue(row, col));
    }

    it = table.begin_t();
    for (size_t row = 0; it != end; it++, ++row)
    {
      for (size_t col = 0; col < table.width(); ++col)
        NKIT_TEST_ASSERT(it[col] == table.GetCellValue(row, col));
    }
  }

  NKIT_TEST_CASE(DynamicTable)
  {
    Dynamic etalon_name1("Son");
    Dynamic etalon_age1(38);
    Dynamic etalon_name2("Father");
    Dynamic etalon_age2(56);
    Dynamic tbl = DTBL("name :STRING, age: INTEGER",
        etalon_name1 << etalon_age1 <<
        etalon_name2 << etalon_age2);
    NKIT_TEST_ASSERT(tbl.GetCellValue(0, 0) == etalon_name1);
    NKIT_TEST_ASSERT(tbl.GetCellValue(0, 1) == etalon_age1);
    NKIT_TEST_ASSERT(tbl.GetCellValue(1, 0) == etalon_name2);
    NKIT_TEST_ASSERT(tbl.GetCellValue(1, 1) == etalon_age2);

    Dynamic::TableIterator it = tbl.begin_t(), end = tbl.end_t();
    NKIT_TEST_ASSERT(it != end);
    NKIT_TEST_ASSERT(it[0] == etalon_name1);
    NKIT_TEST_ASSERT(it[1] == etalon_age1);
    NKIT_TEST_ASSERT(++it != end);
    NKIT_TEST_ASSERT(it[0] == etalon_name2);
    NKIT_TEST_ASSERT(it[1] == etalon_age2);
    NKIT_TEST_ASSERT(++it == end);

    Dynamic clone = tbl.Clone();
    NKIT_TEST_ASSERT(clone == tbl);
  }

  NKIT_TEST_CASE(DynamicTableCreateCases)
  {
    EnvInit();
    CreateCases();
  }

  NKIT_TEST_CASE(DynamicTableAppendCases)
  {
    EnvInit();
    AppendCases();
  }

  NKIT_TEST_CASE(DynamicTableInsertCases)
  {
    EnvInit();
    InsertCases();
  }

  NKIT_TEST_CASE(DynamicTableDeleteCases)
  {
    EnvInit();
    DeleteCases();
  }

  NKIT_TEST_CASE(DynamicTableSetCases)
  {
    EnvInit();
    SetCases();
  }

  _NKIT_TEST_CASE(DynamicTableGroupCases)
  {
    EnvInit();
    GroupCases();
  }

  NKIT_TEST_CASE(DynamicTableIterator)
  {
    EnvInit();
    TableIteratorCases();
  }

} // namespace nkit_test

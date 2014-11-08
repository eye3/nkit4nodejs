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

#include <stdio.h>
//#include <valgrind/callgrind.h>

#include <algorithm>
#include <iomanip>

#include <sqlite3.h>
#include <csv_parser.h>

#include <nkit/test.h>
#include "nkit/dynamic_json.h"
#include <nkit/logger_brief.h>
#include <nkit/detail/ref_count_ptr.h>

using namespace nkit;

namespace nkit
{
  namespace detail
  {
    std::string __PROGRAMM_DIR;
  }
}

const std::string & PROGRAMM_DIR_ = nkit::detail::__PROGRAMM_DIR;

#if defined(NKIT_POSIX_PLATFORM)
char path_delimiter_ = '/';
#elif defined(NKIT_WINNT_PLATFORM)
char path_delimiter_ = '\\';
#else
#  error "Unknown platform"
#endif

//------------------------------------------------------------------------------
bool run_write_statement(sqlite3 * db, const std::string & st,
    std::string * error = NULL)
{
  char * _error = NULL;
  int rc = sqlite3_exec(db, st.c_str(), NULL, NULL, &_error);
  if (rc != SQLITE_OK || _error != NULL)
  {
    if (_error != NULL)
    {
      if (error)
        *error = _error;
      sqlite3_free(_error);
    }
    else
      *error = "ERROR while executing '" + st + ";";
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
class SqliteTransaction;
class SqliteTable;

class SqliteMemoryDb
{
  friend class SqliteTransaction;
  friend class SqliteTable;

public:
  typedef detail::ref_count_ptr<SqliteMemoryDb> Ptr;
  static Ptr Create(std::string * error)
  {
    sqlite3 * db;
    int rc = sqlite3_open(":memory:", &db);
    if (rc)
      return Ptr();

    if (!run_write_statement(db, "PRAGMA synchronous = OFF", error))
      return Ptr();
    if (!run_write_statement(db, "PRAGMA journal_mode = MEMORY", error))
      return Ptr();

    return Ptr(new SqliteMemoryDb(db));
  }

  ~SqliteMemoryDb()
  {
    sqlite3_close(db_);
    db_ = NULL;
  }

  detail::ref_count_ptr<SqliteTransaction> CreateTransaction(std::string * error);
  detail::ref_count_ptr<SqliteTable> CreateTable(const std::string & name,
      const std::string & columns_def, std::string * error);
  detail::ref_count_ptr<SqliteTable> CreateTable(const std::string & name,
      const Dynamic & src, std::string * error);

private:
  SqliteMemoryDb(sqlite3 * db)
    : db_(db)
  {}

private:
  sqlite3 * db_;
};

//------------------------------------------------------------------------------
struct TypeMapping
{
  std::string sqlite_column_type_;
  std::string dynamic_type_;
};

static const TypeMapping sqlite3_type_to_dynamic_type[] = {
    {"text", "STRING"}
  , {"primary", "UNSIGNED_INTEGER"}
  , {"unsigned", "UNSIGNED_INTEGER"}
  , {"int", "INTEGER"}
  , {"real", "FLOAT"}
  , {"double", "FLOAT"}
  , {"float", "FLOAT"}
  , {"boolean", "BOOL"}
  , {"datetime", "DATE_TIME"}
  , {"", ""}
};

static const TypeMapping dynamic_type_to_sqlite3_type[] = {
    {"TEXT", "STRING"}
  , {"UNSIGNED", "UNSIGNED_INTEGER"}
  , {"INT", "INTEGER"}
  , {"DOUBLE", "FLOAT"}
  , {"BOOLEAN", "BOOL"}
  , {"DATETIME", "DATE_TIME"}
  , {"", ""}
};

const std::string & get_sqlite3_type_by_dynamic_type(const std::string & type)
{
  size_t i = 0;
  while (!dynamic_type_to_sqlite3_type[i].dynamic_type_.empty())
  {
    if (dynamic_type_to_sqlite3_type[i].dynamic_type_ == type)
      return dynamic_type_to_sqlite3_type[i].sqlite_column_type_;
    i++;
  }

  return dynamic_type_to_sqlite3_type[0].sqlite_column_type_;
}

class SqliteTable
{
  friend class SqliteMemoryDb;

public:
  typedef detail::ref_count_ptr<SqliteTable> Ptr;

public:
  ~SqliteTable() {}

  Dynamic Group(const std::string & aggr_def,
      const std::string & _group_def, std::string * error)
  {
    data_of_current_select_ = Dynamic();
    data_types_of_current_select_.clear();

    std::string group_def;
    std::string order_def;
    if (_group_def.find('-') != _group_def.npos)
    {
      StringVector group_def_list, order_def_list;
      simple_split(_group_def, ",", &group_def_list);
      size_t size = group_def_list.size();
      for (size_t i=0; i<size; ++i)
      {
        if (group_def_list[i][0] == '-')
        {
          group_def_list[i].erase(0, 1);
          order_def_list.push_back(group_def_list[i] + " DESC");
        }
        else
          order_def_list.push_back(group_def_list[i] + " ASC");
      }
      join(group_def_list, ", ", "", "", &group_def);
      join(order_def_list, ", ", "", "", &order_def);
    }
    else
      group_def = _group_def;

    char *_error;
    std::string sql("SELECT " + group_def + ", " + aggr_def + " FROM " + name_ +
        " GROUP BY " + group_def);
    if (!order_def.empty())
      sql += " ORDER BY " + order_def;
    //CINFO(sql);
    int rc = sqlite3_exec(db_->db_, sql.c_str(), callback,
        this, &_error);
    if( rc != SQLITE_OK )
    {
      if (error)
        *error = _error;
      sqlite3_free(_error);
      return Dynamic();
    }

    return data_of_current_select_;
  }

  operator Dynamic()
  {
    data_of_current_select_ = Dynamic();
    data_types_of_current_select_.clear();

    char *_error;
    std::string select_from("SELECT * FROM ");
    int rc = sqlite3_exec(db_->db_, (select_from + name_).c_str(), callback,
        this, &_error);
    if( rc != SQLITE_OK )
    {
      sqlite3_free(_error);
      return Dynamic();
    }

    return data_of_current_select_;
  }

  bool InsertRow(const StringVector & values, std::string * error)
  {
    if (insert_row_values_.capacity() < insert_row_values_.size())
      insert_row_values_.reserve(insert_row_values_.size());
    insert_row_values_.clear();
    join(values, ", ", "\"", "\"", &insert_row_values_);
    const std::string stmt = insert_row_begin_ + insert_row_values_ + ");";
    //CINFO(stmt);
    return run_write_statement(db_->db_, stmt.c_str(), error);
  }

  bool CreateIndex(const std::string & name,
      const std::string & index_columns, std::string * error)
  {
    if (name.empty())
    {
      if (error)
        *error = "Empty index name";
      return false;
    }

    if (index_columns.empty())
    {
      if (error)
        *error = "Empty columns definition for index '" + name + "'";
      return false;
    }

    StringVector columns;
    nkit::simple_split(index_columns, ",", &columns);

    std::string sql;
    join(columns, ", ", "", "", &sql);
    std::string stmt = "CREATE INDEX " + name + " ON " + name_ +
        " (" + sql + ");";
    return run_write_statement(db_->db_, stmt.c_str(), error);
  }

private:
  static Ptr Create(SqliteMemoryDb * db,
      const std::string & name, const Dynamic & src, std::string * error);

  static Ptr Create(SqliteMemoryDb * db,
      const std::string & name, const std::string & columns_def,
      std::string * error)
  {
    if (name.empty())
    {
      if (error)
        *error = "Empty table name";
      return Ptr();
    }

    if (columns_def.empty())
    {
      if (error)
        *error = "Empty columns definition";
      return Ptr();
    }

    StringVector _columns;
    StringVector column_names, column_types, column_defs;
    nkit::simple_split(columns_def, ",", &_columns);
    StringVector::const_iterator it = _columns.begin(),
        end = _columns.end();
    for (; it != end; ++it)
    {
      std::string column_name, column_type;
      nkit::simple_split(*it, ":", &column_name, &column_type);
      if (column_type.empty())
        column_type = "TEXT";
      column_names.push_back(column_name);
      column_types.push_back(column_type);
    }

    std::string sql_defs;
    join_pairs(column_names, column_types, " ", ", ", "", "", &sql_defs);
    if (!run_write_statement(db->db_, "CREATE TABLE IF NOT EXISTS " + name +
        " (" + sql_defs + ");", error))
      return Ptr();
    return Ptr(new SqliteTable(db, name, column_names, column_types));
  }

  SqliteTable(SqliteMemoryDb * db, const std::string & name,
      const StringVector & column_names,
      const StringVector & column_types)
    : db_(db)
    , name_(name)
    , column_names_(column_names)
    , column_types_(column_types)
  {
    std::string column_names_sql;
    join(column_names_, ", ", "", "", &column_names_sql);
    insert_row_begin_ = "INSERT INTO " + name_ + "(" + column_names_sql +
        ") VALUES (";
  }

  std::string GetColumnTypeByName(const std::string & name,
      uint64_t * dynamic_type) const
  {
    static std::string COUNT_("COUNT(*)");
    static std::string MAX_("MAX(");
    static std::string MIN_("MIN(");
    static std::string SUM_("SUM(");

    for (size_t i=0; i < column_names_.size(); ++i)
    {
      if (stristr(name, COUNT_) == 0)
      {
        *dynamic_type = detail::UNSIGNED_INTEGER;
        return detail::dynamic_type_to_string(*dynamic_type);
      }

      std::string column_name;
      size_t begin = stristr(name, MAX_);
      if (begin != std::string::npos)
      {
        begin += MAX_.size();
        size_t end = name.find(')', begin);
        column_name = name.substr(begin, end - begin);
      }

      if (column_name.empty())
      {
        size_t begin = stristr(name, MIN_);
        if (begin != std::string::npos)
        {
          begin += MIN_.size();
          size_t end = name.find(')', begin);
          column_name = name.substr(begin, end - begin);
        }
      }

      if (column_name.empty())
      {
        size_t begin = stristr(name, SUM_);
        if (begin != std::string::npos)
        {
          begin += SUM_.size();
          size_t end = name.find(')', begin);
          column_name = name.substr(begin, end - begin);
        }
      }

      if (column_name.empty())
        column_name = name;

      if (column_names_[i] == column_name)
      {
        const std::string & sqlite_type = column_types_[i];
        size_t tmi = 0;
        while (!sqlite3_type_to_dynamic_type[tmi].sqlite_column_type_.empty())
        {
          if (stristr(sqlite_type,
                sqlite3_type_to_dynamic_type[tmi].sqlite_column_type_) !=
              std::string::npos)
          {
            *dynamic_type = detail::string_to_dynamic_type(
                sqlite3_type_to_dynamic_type[tmi].dynamic_type_);
            return sqlite3_type_to_dynamic_type[tmi].dynamic_type_;
          }
          ++tmi;
        }
      }
    }

    *dynamic_type = detail::STRING;
    return detail::dynamic_type_to_string(*dynamic_type);
  }

  static int callback(void * _self, int argc, char **argv, char **column_names)
  {
    std::string error;
    SqliteTable * self = static_cast<SqliteTable *>(_self);
    if (unlikely(self->data_of_current_select_.IsUndef()))
    {
      StringVector table_def_list;
      detail::DynamicTypeVector data_types_of_current_select;
      for (int i = 0; i < argc; ++i)
      {
        std::string name = column_names[i];
        uint64_t dynamic_type;
        std::string type = self->GetColumnTypeByName(name, &dynamic_type);
        data_types_of_current_select.push_back(dynamic_type);
        table_def_list.push_back(name + ":" + type);
      }
      std::string table_def;
      join(table_def_list, ",", "", "", &table_def);
      //std::cout << table_def << '\n';
      self->data_of_current_select_ = Dynamic::Table(table_def, &error);
      if (self->data_of_current_select_.IsUndef())
        return 1;
      self->data_types_of_current_select_ = data_types_of_current_select;
    }

    DynamicVector row;
    for(int i=0; i<argc; i++)
    {
      Dynamic cell;
      const char * value = argv[i];
      if (likely(value))
      {
        switch (self->data_types_of_current_select_[i])
        {
        case detail::INTEGER:
          cell = Dynamic(static_cast<int64_t>(NKIT_STRTOLL(value, NULL, 10)));
          break;
        case detail::UNSIGNED_INTEGER:
          cell = Dynamic::UInt64(
              static_cast<uint64_t>(NKIT_STRTOULL(value, NULL, 10)));
          break;
        case detail::FLOAT:
          cell = Dynamic(std::strtod(value, NULL));
          break;
        case detail::BOOL:
          cell = Dynamic(NKIT_STRTOLL(value, NULL, 10) != 0);
          break;
        case detail::DATE_TIME:
          cell = Dynamic::DateTimeFromTimestamp(NKIT_STRTOULL(value, NULL, 10));
          break;
        default:
          cell = Dynamic(value);
          break;
        }
      }
      row.push_back(cell);
    }

    return self->data_of_current_select_.AppendRow(row) ? 0 : 1;
  }

private:
  SqliteMemoryDb * db_;
  std::string name_;
  StringVector column_names_;
  StringVector column_types_;
  std::string insert_row_begin_;
  std::string insert_row_values_;
  Dynamic data_of_current_select_;
  detail::DynamicTypeVector data_types_of_current_select_;
};

//------------------------------------------------------------------------------
class SqliteTransaction
{
  friend class SqliteMemoryDb;

public:
  typedef detail::ref_count_ptr<SqliteTransaction> Ptr;

public:
  ~SqliteTransaction()
  {
    Commit();
  }

  bool Commit(std::string * error = NULL)
  {
    if (commited_)
      return true;
    commited_ = true;
    return run_write_statement(db_->db_, "END TRANSACTION", error);
  }

private:
  static Ptr Create(SqliteMemoryDb * db, std::string * error)
  {
    if (!run_write_statement(db->db_, "BEGIN TRANSACTION", error))
      return Ptr();
    return Ptr(new SqliteTransaction(db));
  }

  SqliteTransaction(SqliteMemoryDb * db)
    : db_(db)
    , commited_(false)
  {}

private:
  SqliteMemoryDb * db_;
  bool commited_;
};

//------------------------------------------------------------------------------
detail::ref_count_ptr<SqliteTransaction> SqliteMemoryDb::CreateTransaction(
    std::string * error)
{
  return SqliteTransaction::Create(this, error);
}

detail::ref_count_ptr<SqliteTable> SqliteMemoryDb::CreateTable(
    const std::string & name, const std::string & columns_def,
    std::string * error)
{
  return SqliteTable::Create(this, name, columns_def, error);
}

detail::ref_count_ptr<SqliteTable> SqliteMemoryDb::CreateTable(
    const std::string & name, const Dynamic & src, std::string * error)
{
  return SqliteTable::Create(this, name, src, error);
}

detail::ref_count_ptr<SqliteTable> SqliteTable::Create(SqliteMemoryDb * db,
    const std::string & name, const Dynamic & src, std::string * error)
{
  StringVector column_names = src.GetColumnNames();
  StringVector column_types = src.GetColumnTypes();
  StringVector::iterator it = column_types.begin(), end = column_types.end();
  for (; it != end; ++it)
    it->assign(get_sqlite3_type_by_dynamic_type(*it));

  column_names.insert(column_names.begin(), "k");
  column_types.insert(column_types.begin(), "INTEGER PRIMARY KEY");
  std::string sql_defs;
  join_pairs(column_names, column_types, " ", ", ", "", "", &sql_defs);

  if (!run_write_statement(db->db_, "CREATE TABLE IF NOT EXISTS " + name +
      " (" + sql_defs + ");", error))
    return Ptr();

  Ptr dst(new SqliteTable(db, name, column_names, column_types));

  SqliteTransaction::Ptr tr = db->CreateTransaction(error);
  if (!tr)
    return Ptr();

  size_t width = src.width();
  size_t k(0);
  Dynamic::TableIterator row = src.begin_t(), tend = src.end_t();
  for (; row != tend; ++row)
  {
    StringVector srow;
    srow.push_back(string_cast(k++));

    for (size_t c = 0; c < width; ++c)
      srow.push_back(row[c].GetString());

    if (!dst->InsertRow(srow, error))
      return Ptr();
  }

  if (!tr->Commit(error))
    return Ptr();

  return dst;
}

//------------------------------------------------------------------------------
const size_t ITER_SIZE = 2000000;
const size_t FACTOR = ITER_SIZE / 40;
static int rnumbers[ITER_SIZE];

//------------------------------------------------------------------------------
void Init()
{
#ifdef NKIT_WINNT
  srand ((unsigned int)time(NULL));
#else
  srand (time(NULL));
#endif
  for (size_t i = 0; i < ITER_SIZE; i++)
    rnumbers[i] = rand() % FACTOR + 1;
}

//------------------------------------------------------------------------------
#define METRIC(expr_, duration) \
do \
{\
  TimeMeter tm; \
  tm.Start(); \
  { expr_; } \
  tm.Stop(); \
  *duration = tm.GetTotal(); \
} while(0)

//--------------------------------------------------------------------------
void GetMinusPlusPermutations(size_t count, StringList * minus_plus_permutatins)
{
  minus_plus_permutatins->push_back(std::string(count, ' '));
  for (size_t i = 1; i < count; ++i)
  {
    std::string signes(i, '-');
    signes.append(count - i, ' ');
    std::sort(signes.begin(), signes.end());
    do
    {
      minus_plus_permutatins->push_back(signes);

    } while (std::next_permutation(signes.begin(), signes.end()));
  }
  minus_plus_permutatins->push_back(std::string(count, '-'));
}

void GetIndexCombinations(const StringVector & fields,
    StringVector * combinations)
{
  uint64_t count = 0;
  UniqueCombinationGenerator<std::string> ucgen(fields);
  while (true)
  {
    StringSet combination;
    if (!ucgen.GetNext(&combination))
      break;

    Dynamic tmp = Dynamic::List(combination);
    StringVector permutation;
    tmp.SaveTo(&permutation);
    std::sort(permutation.begin(), permutation.end());
    do
    {
      StringList minus_plus_permutatins;
      GetMinusPlusPermutations(combination.size(), &minus_plus_permutatins);
      StringList::const_iterator it = minus_plus_permutatins.begin(), end =
          minus_plus_permutatins.end();
      for (; it != end; ++it)
      {
        count++;
        std::string joined;
        join_pairs(*it, permutation, "", ",", "", "", &joined);
        combinations->push_back(joined);
      }

    } while (std::next_permutation(permutation.begin(), permutation.end()));
  }
}
/*
void PrepareData(std::vector<StringVector> * rows)
{
  std::string error;
  std::string filename = PROGRAMM_DIR_ + "../../data/CrimeStatebyState.csv";
  //+ "../../data/CrimeStatebyStateSmall.csv";
  std::ifstream file(filename.c_str());
  NKIT_TEST_ASSERT(file.good());
  size_t c = 1;
  CSVIterator _row(file);
  CSVIterator end;
  if (_row != end)
    ++_row;

  for (; _row != end; ++_row)
  {
    StringVector & row = *_row;
    row.insert(row.begin(), string_cast(c++));
    std::string s = row[4] + "-12-31 00:00:00";
    Dynamic dt = Dynamic::DateTimeFromDefault(s, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(dt, error);

    row.push_back(string_cast(dt.timestamp()));
    rows->push_back(row);
  }

  std::random_shuffle(rows->begin(), rows->end());
}
*/

const size_t REPEAT_COUNT = 10000;
const size_t ARRAY_SIZE = 5;
const size_t TOTAL = REPEAT_COUNT * ARRAY_SIZE;

template <typename T>
void populate(const T src[ARRAY_SIZE], DynamicVector * vv)
{
  for (size_t i1 = 0; i1 < ARRAY_SIZE; ++i1)
  {
    for (size_t i2 = 0; i2 < REPEAT_COUNT; ++i2)
      vv->push_back(Dynamic(src[i1]));
  }
}

void populate(const uint64_t src[ARRAY_SIZE], DynamicVector * vv)
{
  for (size_t i1 = 0; i1 < ARRAY_SIZE; ++i1)
  {
    for (size_t i2 = 0; i2 < REPEAT_COUNT; ++i2)
      vv->push_back(Dynamic::UInt64(src[i1]));
  }
}

void shift(size_t offset, DynamicVector * vv)
{
  assert(offset < vv->size());
  DynamicVector tmp(vv->size(), Dynamic());
  std::copy(vv->begin() + (vv->size() - offset), vv->end(),
      tmp.begin());
  std::copy(vv->begin(), vv->begin() + (vv->size() - offset),
      tmp.begin() + offset);
  *vv = tmp;
}

Dynamic PrepareData()
{
  std::string error;

  const char * browser[ARRAY_SIZE] = {
      "chrome",
      "opera",
      "firefox",
      "safari",
      "ie"};
  DynamicVector browsers;
  populate(browser, &browsers);
  size_t offset = 0;
  shift(++offset, &browsers);

  const char * os[ARRAY_SIZE] = {
      "Windows",
      "MacOS",
      "Ubuntu",
      "Debian",
      "iOS"};
  DynamicVector oses;
  populate(os, &oses);
  shift(++offset, &oses);

  const int64_t year[ARRAY_SIZE] = {
      2014,
      2013,
      2012,
      2011,
      2010};
  DynamicVector years;
  populate(year, &years);
  shift(++offset, &years);

  const double rate[ARRAY_SIZE] = {
      0.1,
      0.2,
      0.3,
      0.4,
      0.5};
  DynamicVector rates;
  populate(rate, &rates);
  shift(++offset, &rates);

  const int64_t likes[ARRAY_SIZE] = {
      MAX_INT64_VALUE - 1,
      MAX_INT64_VALUE - 2,
      MAX_INT64_VALUE - 3,
      MAX_INT64_VALUE - 4,
      MAX_INT64_VALUE - 5};
  DynamicVector likess;
  populate(likes, &likess);
  shift(++offset, &likess);

  const bool flag[ARRAY_SIZE] = {
      true,
      false,
      true,
      false,
      true};
  DynamicVector flags;
  populate(flag, &flags);
  shift(++offset, &flags);

  std::vector<DynamicVector> rows;
  for (size_t i = 0; i < TOTAL; ++i)
  {
    DynamicVector row;
    row.push_back(browsers[i]);
    row.push_back(oses[i]);
    row.push_back(years[i]);
    row.push_back(rates[i]);
    row.push_back(likess[i]);
    row.push_back(flags[i]);
    rows.push_back(row);
  }

  std::random_shuffle(rows.begin(), rows.end());

  Dynamic table = Dynamic::Table("browser:STRING, os:STRING, year:INTEGER,"
      "rate:FLOAT, likes:INTEGER, flag:BOOL", &error);
  for (size_t i = 0; i < TOTAL; ++i)
    table.AppendRow(rows[i]);

  return table;
}

void TestGroup(Dynamic dynamic_table, SqliteTable::Ptr sqlite_table,
    const std::string & index_def)
{
  CINFO(index_def);

  TimeMeter tm;
  std::string error;
  std::string aggr_def("COUNT, MIN(flag), MAX(likes), SUM(year)");

  //-----------------------------------------------
  // grouping dynamic table by builder
  GroupedTableBuilder::Ptr builder =
      Dynamic::CreateGroupedTableBuilder(
          "browser:STRING, os:STRING, year:INTEGER,"
            "rate:FLOAT, likes:INTEGER, flag:BOOL",
          index_def, aggr_def, &error);
  NKIT_TEST_ASSERT_WITH_TEXT(builder, error);

  size_t count = dynamic_table.height();
  size_t width = dynamic_table.width();
  DynamicVector row(width);
  tm.Start();
  for (size_t i = 0; i < count; ++i)
  {
    for (size_t c = 0; c < width; ++c)
      row[c] = dynamic_table.GetCellValue(i, c);
    NKIT_TEST_ASSERT(builder->InsertRow(row));
  }
  Dynamic grouped_by_builder = builder->GetResult();
  tm.Stop();
  NKIT_TEST_ASSERT(grouped_by_builder.IsTable());
  double grouped_by_builder_time = tm.GetTotal();
  tm.Clear();

  //-----------------------------------------------
  // grouping dynamic table by method Group()
  tm.Start();

  //CALLGRIND_START_INSTRUMENTATION;
  Dynamic grouped_table = dynamic_table.Group(index_def, aggr_def, &error);
  //CALLGRIND_STOP_INSTRUMENTATION;

  tm.Stop();
  NKIT_TEST_ASSERT_WITH_TEXT(grouped_table.IsTable(), error);
  double dynamic_group_time = tm.GetTotal();
  tm.Clear();

  if (grouped_by_builder != grouped_table)
  {
    CINFO("-----------------------------------------------------");
    CINFO(grouped_table);
    CINFO("-----------------------------------------------------");
    CINFO(grouped_by_builder);
    NKIT_TEST_ASSERT(false);
  }

  //-----------------------------------------------
  // grouping sqlite3 table
  tm.Start();
  Dynamic sqlite_grouped_table = sqlite_table->Group(
      "COUNT(*), MIN(flag), MAX(likes), SUM(year)", index_def, &error);
  tm.Stop();
  NKIT_TEST_ASSERT_WITH_TEXT(sqlite_grouped_table.IsTable(), error);
  if (sqlite_grouped_table != grouped_table)
  {
    CINFO("-----------------------------------------------------");
    CINFO(grouped_table);
    CINFO("-----------------------------------------------------");
    CINFO(sqlite_grouped_table);
    NKIT_TEST_ASSERT(false);
  }
  double sqlite3_group_time = tm.GetTotal();
  tm.Clear();
  if (dynamic_table.height() > grouped_table.height())
  {
    std::cout << "builder/method: " <<
        grouped_by_builder_time / dynamic_group_time << '\n';
    std::cout << "Sqlite/Dynamic: " <<
        sqlite3_group_time / dynamic_group_time << '\n';
    std::cout << "Factor:       " << std::setprecision(2) <<
        dynamic_table.height() / grouped_table.height() << '\n';
    //NKIT_TEST_ASSERT(sqlite3_group_time / dynamic_group_time > 1.0);
  }
}

_NKIT_TEST_CASE(TestGroupBuilder)
{
  std::string error;
  Dynamic dynamic_table = PrepareData();

  std::string index_def("browser,os");
  std::string aggr_def("COUNT, MIN(flag), MAX(likes), SUM(year)");

  GroupedTableBuilder::Ptr builder =
      Dynamic::CreateGroupedTableBuilder(
          "browser:STRING, os:STRING, year:INTEGER,"
            "rate:FLOAT, likes:INTEGER, flag:BOOL",
          index_def, aggr_def, &error);

  size_t count = dynamic_table.height();
  size_t width = dynamic_table.width();
  DynamicVector row(width);
  for (size_t i = 0; i < count; ++i)
  {
    for (size_t c = 0; c < width; ++c)
      row[c] = dynamic_table.GetCellValue(i, c);
    NKIT_TEST_ASSERT(builder->InsertRow(row));
  }
  Dynamic grouped_by_builder = builder->GetResult();

  Dynamic grouped_by_method = dynamic_table.Group(index_def, aggr_def, &error);

  NKIT_TEST_ASSERT_WITH_TEXT(grouped_by_method.IsTable(), error);
  NKIT_TEST_ASSERT(grouped_by_builder = grouped_by_method);
}

_NKIT_TEST_CASE(TestCompare)
{
  std::string error;
  detail::IndexKey k1, k2;
  //0, 1, 1960, 1960,
  //0, 1, 2005, 2005,
  //more
  k1.key_[0].i64_ = 0;
  k1.key_[1].i64_ = 1;
  k1.key_[2].ui64_ = 1960;
  k1.key_[3].i64_ = 1960;
  k1.size_ = 4;
  k2.key_[0].i64_ = 0;
  k2.key_[1].i64_ = 1;
  k2.key_[2].ui64_ = 2005;
  k2.key_[3].i64_ = 2005;
  k2.size_ = 4;

  StringVector mask;
  mask.push_back("1");
  mask.push_back("1");
  mask.push_back("-2");
  mask.push_back("1");

  nkit::detail::IndexCompare compare;
  NKIT_TEST_ASSERT_WITH_TEXT(
      nkit::detail::GetComparator(mask, &compare, &error), error);

  compare(k1, k2);
  return;

}

/*
_NKIT_TEST_CASE(TestIndex)
{
  std::string error;
  typedef std::map<detail::IndexKey, SizeVector, detail::IndexCompare const>
    IndexMap;
  StringVector mask;
  mask.push_back("1");
  mask.push_back("1");
  mask.push_back("-2");
  mask.push_back("1");

  nkit::detail::IndexCompare compare;
  NKIT_TEST_ASSERT_WITH_TEXT(
      nkit::detail::GetComparator(mask, &compare, &error),
      error);

  IndexMap index_map(compare);

  Dynamic dt1960(1960,1,1,0,0,0);
  Dynamic dt1962(1962,1,1,0,0,0);
  Dynamic dt2005(2005,1,1,0,0,0);
  Dynamic dt1971(1971,1,1,0,0,0);

  dt1960 = Dynamic::UInt64(1960);
  dt1962 = Dynamic::UInt64(1962);
  dt2005 = Dynamic::UInt64(2005);
  dt1971 = Dynamic::UInt64(1);

  Dynamic t = DTBL("state:INTEGER,"
      "type:INTEGER,"
      "dt:UNSIGNED_INTEGER,"
      "year:INTEGER,"
      "year2:INTEGER",
      0 << 1 << dt1960 << 1960 << 1960 <<
      0 << 2 << dt2005 << 2005 << 2005 <<
      0 << 1 << dt2005 << 2005 << 2005 <<
      0 << 1 << dt2005 << 1 << 1 <<
      0 << 1 << dt1962 << 1962 << 1962);

  CINFO(t);
  Dynamic::TableIterator tbl_it = t.begin_t(), tbl_end = t.end_t();
  for (; tbl_it != tbl_end; ++tbl_it)
  {
    detail::IndexKey key(4);
    Dynamic tmp(tbl_it[0]);
    CINFO(tmp);
    key.key_[0] = KeyFromData(tmp.data_, tmp.type_);

    tmp = tbl_it[1];
    CINFO(tmp);
    key.key_[1] = KeyFromData(tmp.data_, tmp.type_);

    tmp = tbl_it[2];
    CINFO(tmp);
    key.key_[2] = KeyFromData(tmp.data_, tmp.type_);

    tmp = tbl_it[3];
    CINFO(tmp);
    key.key_[3] = KeyFromData(tmp.data_, tmp.type_);

    index_map[key] = SizeVector();
    CINFO("---");
  }

  CINFO(index_map.size());
  IndexMap::const_iterator it = index_map.begin(), end = index_map.end();
  for (; it != end; ++it)
  {
    std::cout <<
        it->first.key_[0].i64_ << ", " <<
        it->first.key_[1].i64_ << ", " <<
        it->first.key_[2].ui64_ << ", " <<
        it->first.key_[3].i64_ << ", " <<
        '\n';
  }
}
*/

_NKIT_TEST_CASE(TestEmptyTableGroup)
{
  std::string error;

  Dynamic table = Dynamic::Table("c1:STRING, c2:INTEGER, c3:DATE_TIME", &error);
  Dynamic grouped_table = table.Group("c1", "COUNT, SUM(c2)", &error);
  NKIT_TEST_ASSERT_WITH_TEXT(grouped_table.IsTable(), error);
  NKIT_TEST_ASSERT(grouped_table.empty());
}

NKIT_TEST_CASE(TestGroup)
{
  std::string error;

  Dynamic dynamic_table = PrepareData();

  //-----------------------------------------------
  // getting sqlite3 table from dynamic table
  SqliteMemoryDb::Ptr db = SqliteMemoryDb::Create(&error);
  NKIT_TEST_ASSERT_WITH_TEXT(db, error);
  SqliteTable::Ptr sqlite_table = db->CreateTable("t", dynamic_table, &error);
  NKIT_TEST_ASSERT_WITH_TEXT(sqlite_table, error);

  //TestGroup(dynamic_table, sqlite_table, "browser, -year");
  //return;

  StringVector fields;
  //fields.push_back("browser");
  fields.push_back("year");
  fields.push_back("os");
  fields.push_back("likes");
  fields.push_back("flag");
  fields.push_back("rate");

  StringVector index_combinations;
  GetIndexCombinations(fields, &index_combinations);

  size_t total = index_combinations.size();
  StringVector::const_iterator it = index_combinations.begin(), comb_end =
      index_combinations.end();
  for (size_t i=0; it != comb_end; ++it, ++i)
  {
    TestGroup(dynamic_table, sqlite_table, *it);
    std::cout << i << "/" << total << '\n';
  }
}

_NKIT_TEST_CASE(TestSelectFromSqlite3)
{
  Init();
  std::string error;

  CINFO("Creating sqlite3 table");
  TimeMeter tm;
  SqliteMemoryDb::Ptr db = SqliteMemoryDb::Create(&error);
  NKIT_TEST_ASSERT_WITH_TEXT(db, error);

  SqliteTable::Ptr sqlite_table = db->CreateTable("t", "k:INTEGER PRIMARY KEY,"
      "name1:DATETIME, name2:TEXT, name3:INTEGER, name4:TEXT,"
      " name5:INTEGER", &error);
  NKIT_TEST_ASSERT_WITH_TEXT(sqlite_table, error);

  SqliteTransaction::Ptr tr = db->CreateTransaction(&error);
  NKIT_TEST_ASSERT_WITH_TEXT(tr, error);

  StringVector row;
  row.reserve(6);
  for (size_t it_factor = 0; it_factor < ITER_SIZE; ++it_factor)
  {
    const size_t uniq = rnumbers[it_factor];
    row.clear();
    row.push_back(string_cast(it_factor));
    row.push_back(string_cast(1386000000 + uniq));
    row.push_back("name_" + string_cast(uniq));
    row.push_back(string_cast(uniq + FACTOR));
    row.push_back(std::string("name_" + string_cast(uniq + FACTOR)));
    row.push_back(string_cast(uniq + FACTOR + 1));
    NKIT_TEST_ASSERT_WITH_TEXT(sqlite_table->InsertRow(row, &error), error);
  }

  NKIT_TEST_ASSERT_WITH_TEXT(tr->Commit(&error), error);

  tm.Start();
  Dynamic table = *sqlite_table;
  tm.Stop();
  double sqlite3_select_time = tm.GetTotal();
  tm.Clear();
  std::cout << "sqlite3_select_time: " << sqlite3_select_time << '\n';

  tm.Start();
  table = table.Clone();
  size_t col_count = table.width();
  size_t tmp_count = 0;
  Dynamic::TableIterator it = table.begin_t(), end = table.end_t();
  for (; it != end; ++it)
  {
    for (size_t c = 0; c < col_count; ++c)
    {
      Dynamic tmp = it[c];
      tmp_count += tmp.size();
    }
  }
  tm.Stop();
  double dynamic_select_time = tm.GetTotal();
  tm.Clear();
  std::cout << "dynamic_select_time: " << dynamic_select_time << '\n';
  CINFO(tmp_count);

  tm.Start();
  Dynamic grouped_table = table.Group("name1,name2",
      "COUNT,MIN(name3),MAX(name5),SUM(name5)", &error);
  tm.Stop();
  NKIT_TEST_ASSERT_WITH_TEXT(grouped_table.IsTable(), error);
  double dynamic_group_time = tm.GetTotal();
  tm.Clear();
  std::cout << "dynamic_group_time: " << dynamic_group_time << '\n';
  //CINFO(table);
  //CINFO(grouped_table);

  tm.Start();
  Dynamic sqlite_grouped_table = sqlite_table->Group(
      "COUNT(*), MIN(name3), MAX(name5), SUM(name5)", "name1, name2", &error);
  tm.Stop();
  NKIT_TEST_ASSERT_WITH_TEXT(sqlite_grouped_table.IsTable(), error);
  //CINFO(sqlite_grouped_table);
  NKIT_TEST_ASSERT(sqlite_grouped_table == grouped_table);
  double sqlite3_group_time = tm.GetTotal();
  tm.Clear();
  std::cout << "sqlite3_group_time: " << sqlite3_group_time << '\n';

  std::cout << "Dynamic group(3/4) is faster then "
      "sqlite3 group(4) by " << sqlite3_group_time / dynamic_group_time
      << " times" << '\n';
}

//------------------------------------------------------------------------------
_NKIT_TEST_CASE(PerfCreateIndex)
{
  Init();
  std::string error;
  TimeMeter tm;

  // Creating dynamic table
  std::string table_def(
      "name1:INTEGER,name2:STRING,name3:INTEGER,name4:STRING,name5:DATE_TIME");
  Dynamic table = Dynamic::Table(table_def, &error);

  tm.Start();
  for (size_t it_factor = 0; it_factor < ITER_SIZE; ++it_factor)
  {
    const uint64_t uniq = rnumbers[it_factor];
    NKIT_TEST_ASSERT(
        table.AppendRow(Dynamic(uniq),
            Dynamic(std::string("name_" + string_cast(uniq))),
            Dynamic(uniq + FACTOR),
            Dynamic(std::string("name_" + string_cast(uniq + FACTOR))),
            Dynamic::DateTimeFromTimestamp(1386000000 + uniq + FACTOR + 1)));
  }
  tm.Stop();
  double dynamic_create_table_time = tm.GetTotal();
  tm.Clear();

  // Creating sqlite3 table
  SqliteMemoryDb::Ptr db = SqliteMemoryDb::Create(&error);
  NKIT_TEST_ASSERT_WITH_TEXT(db, error);

  SqliteTable::Ptr sqlite_table = db->CreateTable("t", "k:INTEGER PRIMARY KEY,"
      "name1:INTEGER, name2:TEXT, name3:INTEGER, name4:TEXT,"
      "name5:DATETIME", &error);
  NKIT_TEST_ASSERT_WITH_TEXT(sqlite_table, error);

  SqliteTransaction::Ptr tr = db->CreateTransaction(&error);
  NKIT_TEST_ASSERT_WITH_TEXT(tr, error);

  tm.Start();
  StringVector row;
  row.reserve(6);
  for (size_t it_factor = 0; it_factor < ITER_SIZE; ++it_factor)
  {
    const size_t uniq = rnumbers[it_factor];
    row.clear();
    row.push_back(string_cast(it_factor));
    row.push_back(string_cast(uniq));
    row.push_back("name_" + string_cast(uniq));
    row.push_back(string_cast(uniq + FACTOR));
    row.push_back(std::string("name_" + string_cast(uniq + FACTOR)));
    row.push_back(string_cast(uniq + FACTOR + 1));
    NKIT_TEST_ASSERT_WITH_TEXT(sqlite_table->InsertRow(row, &error), error);
  }

  NKIT_TEST_ASSERT_WITH_TEXT(tr->Commit(&error), error);
  tm.Stop();
  double sqlite3_create_table_time = tm.GetTotal();
  tm.Clear();

  NKIT_TEST_ASSERT(sqlite3_create_table_time / dynamic_create_table_time > 1.0);

  std::cout << "Dynamic create table if faster then sqlite3 create table by: "
      << sqlite3_create_table_time / dynamic_create_table_time << '\n';

  nkit::TableIndex::Ptr index;

  double dynamic_create_index_time;
  METRIC(index = table.CreateIndex("name1", &error),
      &dynamic_create_index_time);
  NKIT_TEST_ASSERT_WITH_TEXT(index, error);

  double sqlite3_create_index_time;
  METRIC(
      NKIT_TEST_ASSERT_WITH_TEXT( sqlite_table->CreateIndex( "idx1", "name5", &error), error),
      &sqlite3_create_index_time);

  NKIT_TEST_ASSERT(sqlite3_create_index_time / dynamic_create_index_time > 1.0);

  std::cout << "Dynamic create index(1) if faster then "
      "sqlite3 create index(1) by: "
      << sqlite3_create_index_time / dynamic_create_index_time << '\n';

  METRIC(index = table.CreateIndex("name1,name2", &error),
      &dynamic_create_index_time);
  NKIT_TEST_ASSERT_WITH_TEXT(index, error);

  METRIC(
      NKIT_TEST_ASSERT_WITH_TEXT( sqlite_table->CreateIndex( "idx2", "name1, name2", &error), error),
      &sqlite3_create_index_time);

  NKIT_TEST_ASSERT(sqlite3_create_index_time / dynamic_create_index_time > 1.0);

  std::cout << "Dynamic create index(2) if faster then "
      "sqlite3 create index(2) by: "
      << sqlite3_create_index_time / dynamic_create_index_time << '\n';

  METRIC(index = table.CreateIndex("name1,name2,name3", &error),
      &dynamic_create_index_time);
  NKIT_TEST_ASSERT_WITH_TEXT(index, error);

  METRIC(
      NKIT_TEST_ASSERT_WITH_TEXT( sqlite_table->CreateIndex( "idx3", "name1, name2, name3", &error), error),
      &sqlite3_create_index_time);

  NKIT_TEST_ASSERT(sqlite3_create_index_time / dynamic_create_index_time > 1.0);

  std::cout << "Dynamic create index(3) if faster then "
      "sqlite3 create index(3) by: "
      << sqlite3_create_index_time / dynamic_create_index_time << '\n';
}

int main(int argc, char ** argv)
{
  if (argc >= 1)
  {
    detail::__PROGRAMM_DIR = argv[0];
    size_t pos = detail::__PROGRAMM_DIR.rfind(path_delimiter_);
    if (pos != detail::__PROGRAMM_DIR.npos)
      detail::__PROGRAMM_DIR.erase(pos + 1);
  }

  return nkit::test::run_all_tests();
}

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

#include <stack>
#include <cstdlib>
#include <errno.h>

#include <yajl/yajl_parse.h>

#include "nkit/dynamic_json.h"
#include "nkit/constants.h"
#include "nkit/tools.h"

namespace nkit
{
  DynamicToJsonOptions DEFAULT_DYNAMIC_TO_JSON_OPTIONS_;
  namespace detail
  {
    int json::xalloc = ::std::ios_base::xalloc();
  } // namespace detail

  const std::string INT_64_MAX =
      string_cast(std::numeric_limits<int64_t>::max());
  const std::string INT_64_MIN =
      string_cast(std::numeric_limits<int64_t>::min()).substr(1);
  const size_t INT_64_MAX_LEN = INT_64_MAX.size();
  const size_t INT_64_MIN_LEN = INT_64_MIN.size();
  const std::string UINT64_T_MAX =
      string_cast(std::numeric_limits<uint64_t>::max());
  const size_t UINT64_T_MAX_LEN = UINT64_T_MAX.size();

  class DynamicConstructor
  {
    enum ContainerType
    {
      CT_UNDEFINED = 0,
      CT_MAP,
      CT_ARRAY
    };

    static const size_t NUMERIC_BUFFER_SIZE = 1024;

  public:
    DynamicConstructor(Dynamic * root)
      //: root_(root)
      : current_container_(root)
      , current_value_(NULL)
      , container_type_(CT_UNDEFINED)
    {}

    static int on_null(void * ctx)
    {
      DynamicConstructor * self = (DynamicConstructor *)ctx;
      return self->OnNull();
    }

    static int on_boolean(void * ctx, int v)
    {
      DynamicConstructor * self = (DynamicConstructor *)ctx;
      return self->OnBoolean(v);
    }

    static int on_integer(void * ctx, long long v)
    {
      DynamicConstructor * self = (DynamicConstructor *)ctx;
      return self->OnInteger(v);
    }

    static int on_double(void * ctx, double v)
    {
      DynamicConstructor * self = (DynamicConstructor *)ctx;
      return self->OnDouble(v);
    }

    /** A callback which passes the string representation of the number
     *  back to the client.  Will be used for all numbers when present */
    static int on_number(void * ctx, const char * str, size_t len)
    {
      DynamicConstructor * self = (DynamicConstructor *)ctx;
      return self->OnNumber(str, len);
    }

    /** strings are returned as pointers into the JSON text when,
     * possible, as a result, they are _not_ null padded */
    static int on_string(void * ctx, const unsigned char * str, size_t len)
    {
      DynamicConstructor * self = (DynamicConstructor *)ctx;
      return self->OnString((const char *)str, len);
    }

    static int on_start_map(void * ctx)
    {
      DynamicConstructor * self = (DynamicConstructor *)ctx;
      return self->OnStartMap();
    }

    static int on_map_key(void * ctx, const unsigned char * str, size_t len)
    {
      DynamicConstructor * self = (DynamicConstructor *)ctx;
      return self->OnMapKey((const char *)str, len);
    }

    static int on_end_map(void * ctx)
    {
      DynamicConstructor * self = (DynamicConstructor *)ctx;
      return self->OnEndMap();
    }

    static int on_start_array(void * ctx)
    {
      DynamicConstructor * self = (DynamicConstructor *)ctx;
      return self->OnStartArray();
    }

    static int on_end_array(void * ctx)
    {
      DynamicConstructor * self = (DynamicConstructor *)ctx;
      return self->OnEndArray();
    }

  private:
    bool OnNull()
    {
      if (container_type_ == CT_MAP)
        *current_value_ = Dynamic();
      else if (container_type_ == CT_ARRAY)
        current_container_->PushBack(Dynamic());
      else
        return false;
      return true;
    }

    bool OnBoolean(int v)
    {
      if (container_type_ == CT_MAP)
        *current_value_ = Dynamic(v != 0);
      else if (container_type_ == CT_ARRAY)
        current_container_->PushBack(Dynamic(v != 0));
      else
        return false;
      return true;
    }

    bool OnInteger(int64_t v)
    {
      if (container_type_ == CT_MAP)
        *current_value_ = Dynamic(v);
      else if (container_type_ == CT_ARRAY)
        current_container_->PushBack(Dynamic(v));
      else
        return false;
      return true;
    }

    bool OnDouble(double v)
    {
      if (container_type_ == CT_MAP)
        *current_value_ = Dynamic(v);
      else if (container_type_ == CT_ARRAY)
        current_container_->PushBack(Dynamic(v));
      else
        return false;
      return true;
    }

    Dynamic ParseNumber(const char * str, size_t size)
    {
      assert(size < NUMERIC_BUFFER_SIZE);
      memcpy((void*)numeric_buffer_, (void*)str, size);
      numeric_buffer_[size] = 0;
      size_t digits_count = size;
      const char * digits = numeric_buffer_;
      const char * INT_64_BOUNDERY = INT_64_MAX.c_str();
      size_t INT_64_BOUNDERY_LEN = INT_64_MAX_LEN;
      if (str[0] == '-')
      {
        INT_64_BOUNDERY_LEN = INT_64_MIN_LEN;
        INT_64_BOUNDERY = INT_64_MIN.c_str();
        --digits_count;
        ++digits;
      }

      bool has_dot = strchr(numeric_buffer_, '.') != NULL;

      if (has_dot || digits_count > UINT64_T_MAX_LEN)
      {
        return Dynamic(strtod(numeric_buffer_, NULL));
      }
      else if ((digits_count < INT_64_BOUNDERY_LEN)
        || (digits_count == INT_64_BOUNDERY_LEN
            && strcmp(digits, INT_64_BOUNDERY) <= 0))
      {
        int64_t tmp = NKIT_STRTOLL(numeric_buffer_, NULL, 10);
        return Dynamic(tmp);
      }
      else
      {
        char * endptr = NULL;
        uint64_t ui64 = NKIT_STRTOULL(numeric_buffer_, &endptr, 10);
        if (unlikely((errno == ERANGE) || *endptr != '\0'))
          return Dynamic(strtod(numeric_buffer_, NULL));
        else
          return Dynamic(ui64);
      }
    }

    bool OnNumber(const char * str, size_t len)
    {
      if (container_type_ == CT_MAP)
        *current_value_ = ParseNumber(str, len);
      else if (container_type_ == CT_ARRAY)
        current_container_->PushBack(ParseNumber(str, len));
      else
        return false;
      return true;
    }

    bool OnString(const char * str, size_t len)
    {
      if (container_type_ == CT_MAP)
        *current_value_ = Dynamic(str, len);
      else if (container_type_ == CT_ARRAY)
        current_container_->PushBack(Dynamic(str, len));
      else
        return false;
      return true;
    }

    bool OnStartMap()
    {
      Dynamic new_map = Dynamic::Dict();
      if (container_type_ == CT_MAP)
      {
        *current_value_ = new_map;
        current_container_ = current_value_;
      }
      else if (container_type_ == CT_ARRAY)
      {
        current_container_->PushBack(new_map);
        current_container_ = & (*current_container_).back();
      }
      else
      {
        *current_container_ = new_map;
      }

      container_type_ = CT_MAP;
      stack_.push(current_container_);
      return true;
    }

    bool OnMapKey(const char * str, size_t len)
    {
      current_value_ = & (*current_container_)[std::string(str, len)];
      return true;
    }

    bool OnEndMap()
    {
      stack_.pop();
      if (!stack_.empty())
      {
        current_container_ = stack_.top();
        container_type_ = current_container_->IsDict() ? CT_MAP : CT_ARRAY;
      }
      else
        current_container_ = NULL;
      return true;
    }

    bool OnStartArray()
    {
      Dynamic new_list = Dynamic::List();
      if (container_type_ == CT_MAP)
      {
        *current_value_ = new_list;
        current_container_ = current_value_;
      }
      else if (container_type_ == CT_ARRAY)
      {
        current_container_->PushBack(new_list);
        current_container_ = & (*current_container_).back();
      }
      else
      {
        *current_container_ = new_list;
      }

      container_type_ = CT_ARRAY;
      stack_.push(current_container_);
      return true;
    }

    bool OnEndArray()
    {
      stack_.pop();
      if (!stack_.empty())
      {
        current_container_ = stack_.top();
        container_type_ = current_container_->IsDict() ? CT_MAP : CT_ARRAY;
      }
      else
        current_container_ = NULL;
      return true;
    }

  private:
    //Dynamic * root_;
    Dynamic * current_container_;
    Dynamic * current_value_;
    ContainerType container_type_;
    std::stack<Dynamic *> stack_;
    char numeric_buffer_[NUMERIC_BUFFER_SIZE];
  };

  static yajl_callbacks callbacks = {
    &DynamicConstructor::on_null,
    &DynamicConstructor::on_boolean,
    NULL, // &DynamicConstructor::on_integer,
    NULL, // &DynamicConstructor::on_double,
    &DynamicConstructor::on_number,
    &DynamicConstructor::on_string,
    &DynamicConstructor::on_start_map,
    &DynamicConstructor::on_map_key,
    &DynamicConstructor::on_end_map,
    &DynamicConstructor::on_start_array,
    &DynamicConstructor::on_end_array
  };

  Dynamic DynamicFromYajl(const std::string & json, std::string * error)
  {
    Dynamic result;
    DynamicConstructor constructor(&result);
    yajl_handle hand = yajl_alloc(&callbacks, NULL, (void *) &constructor);
    if (!hand)
    {
      *error = "Could not allocate yajl handler";
      return Dynamic();
    }

    yajl_config(hand, yajl_allow_comments, 1);

    yajl_status st = yajl_parse(hand,
        (unsigned const char *)(json.c_str()), json.size());
    if (st == yajl_status_ok)
      st = yajl_complete_parse(hand);

    if (st != yajl_status_ok)
    {
      unsigned char * message =
          yajl_get_error(hand, 1,
              (unsigned const char *)json.c_str(), json.size());
      *error = std::string((const char *)message);
      yajl_free_error(hand, message);
      result = Dynamic();
    }

    yajl_free(hand);

    return result;
  }

  Dynamic DynamicFromJson(const std::string & json, std::string * error)
  {
    return DynamicFromYajl(json, error);
  }

  Dynamic DynamicFromJsonFile(const std::string & path, std::string * error)
  {
    std::string json;
    if (!path.empty() && !text_file_to_string(path, &json, error))
    {
      *error = "Could not open file: '" + path + "'";
      return Dynamic();
    }
    if (json.empty())
      json = "{}";
    return DynamicFromJson(json, error);
  }

  //--------------------------------------------------------------------------
  void print(std::ostream & out, const Dynamic & v, std::string offset,
      bool newline)
  {
    if (v.IsBool())
      print(out, v.GetBoolean(), offset, newline);
    else if (v.IsDateTime() || v.IsString()
        || v.IsMongodbOID() || v.IsNumber())
      print(out, v.GetString(), offset, newline);
    else if (v.IsList())
    {
      std::string item_offset = offset + "  ";
      size_t size = v.size();
      out << "[\n";
      for(size_t i = 0; i < size; ++i)
      {
        out << item_offset;
        print(out, v[i], item_offset, false);
        if ((i+1) < size)
          out << ",\n";
      }
      out << offset << "\n]\n";
    }
    else if (v.IsDict())
    {
      out << "{\n";
      DDICT_FOREACH(it, v)
      {
        std::string item_offset = offset + "  ";
        print(out, it->first, item_offset, false);
        out << ": ";
        print(out, it->second, item_offset, true);
      }
      out << offset << "}\n";
    }
    else if (v.IsTable())
    {
      size_t shift_size = 0;
      StringVector cols = v.GetColumnNames();
      StringVector::const_iterator beg = cols.begin();

      for (; beg != cols.end(); ++beg)
      {
        if (beg->size() > shift_size)
          shift_size = beg->size();
      }

      beg = cols.begin();
      for (; beg != cols.end(); ++beg)
        std::cout << " | " << *beg;

      out << std::endl;

      const size_t h_size = v.height(), w_size = v.width();
      for (size_t row = 0; row < h_size; ++row)
      {
        for (size_t col = 0; col < w_size; ++col)
        {
          print(out, v.GetCellValue(row, col), " | ", false);
        }
        out << std::endl;
      }
    }
    else if (v.IsNone())
      out << "DYNAMIC_NONE\n";
    else
      out << "DYNAMIC_UNDEF\n";
  }

  std::ostream & operator << (std::ostream & os, const Dynamic & v)
  {
    DynamicToJsonStream(os, v);
    return os;
  }

  std::ostream & operator << (std::ostream & os, const DynamicVector & v)
  {
    DynamicToJsonStream(os, v);
    return os;
  }

  std::ostream & json(std::ostream & ios)
  {
    ios.iword(detail::json::xalloc) = detail::JSON_HR_ONE_LINE;
    return ios;
  }

  std::ostream & json_hr(std::ostream & ios)
  {
    ios.iword(detail::json::xalloc) = detail::JSON_HR_TABLE_AS_DICT;
    return ios;
  }

  std::ostream & json_hr_table(std::ostream & ios)
  {
    ios.iword(detail::json::xalloc) = detail::JSON_HR_TABLE_AS_TABLE;
    return ios;
  }
}  // namespace nkit

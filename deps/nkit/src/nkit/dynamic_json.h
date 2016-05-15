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

#ifndef __DYNAMIC__JSON__H__
#define __DYNAMIC__JSON__H__

#include <sstream>
#include <fstream>
#include <iomanip>

#include <nkit/detail/push_options.h>
#include <nkit/dynamic.h>

#define __NKIT__WRITE__JSON__(src, dst) \
  nkit::detail::JsonWriter<T>::write_json(src, sizeof(src) - 1, dst);

namespace nkit
{
  struct DynamicToJsonOptions
  {
    DynamicToJsonOptions()
      : date_time_format(DATE_TIME_DEFAULT_FORMAT())
    {}

    std::string date_time_format;
    std::string indent_space;
    std::string indent_newline;
  };

  namespace detail
  {
    enum JsonHumanReadable
    {
      JSON_HR_ONE_LINE = 0,
      JSON_HR_TABLE_AS_DICT = 1,
      JSON_HR_TABLE_AS_TABLE = 2
    };

    struct json
    {
      static int xalloc;
    };

    // for streams
    template<typename T>
    struct JsonWriter
    {
      static inline void write_json(const char * s, size_t size, T * t)
      {
        t->write(s, size);
      }
    };

    // for std::string
    template<>
    struct JsonWriter<std::string>
    {
      static inline void write_json(const char * s, size_t size,
          std::string * dst)
      {
        dst->append(s, size);
      }
    };

    template<typename T>
    inline void write_number(const Dynamic & v, T * t,
        const DynamicToJsonOptions & NKIT_UNUSED(options))
    {
      const std::string tmp(v.GetString());
      JsonWriter<T>::write_json(tmp.c_str(), tmp.size(), t);
    }

    template<typename T>
    inline void write_date_time(const Dynamic & v, T * t,
        const DynamicToJsonOptions & options)
    {
      const std::string tmp(v.GetString(options.date_time_format.c_str()));
      __NKIT__WRITE__JSON__("\"", t);
      JsonWriter<T>::write_json(tmp.c_str(), tmp.size(), t);
      __NKIT__WRITE__JSON__("\"", t);
    }

    template<typename T>
    void write_string_or_mongodb_oid(const Dynamic & v, T * t,
        const DynamicToJsonOptions & NKIT_UNUSED(options))
    {
      std::string::const_iterator it = v.GetConstString().begin(), end =
          v.GetConstString().end(), next;
      __NKIT__WRITE__JSON__("\"", t);
      while (it != end)
      {
        next = it;
        ++next;
        char ch = *it;
        if (unlikely((ch == '\\') && (next != end) && (*next == '"')))
        {
          __NKIT__WRITE__JSON__("\\\"", t);
          ++it;
        }
        else
        {
          switch (ch)
          {
          case '\\':
            __NKIT__WRITE__JSON__("\\\\", t)
            ;
            break;
          case '\"':
            __NKIT__WRITE__JSON__("\\\"", t)
            ;
            break;
          case '/':
            __NKIT__WRITE__JSON__("\\/", t)
            ;
            break;
          case '\b':
            __NKIT__WRITE__JSON__("\\b", t)
            ;
            break;
          case '\f':
            __NKIT__WRITE__JSON__("\\f", t)
            ;
            break;
          case '\n':
            __NKIT__WRITE__JSON__("\\n", t)
            ;
            break;
          case '\r':
            __NKIT__WRITE__JSON__("\\r", t)
            ;
            break;
          case '\t':
            __NKIT__WRITE__JSON__("\\t", t)
            ;
            break;
          default:
            if (unlikely(ch > 0 && ch <= 0x1F))
            {
              std::ostringstream oss;
              oss << "\\u" << std::hex << std::uppercase << std::setfill('0')
                  << std::setw(4) << static_cast<int>(ch);
              const std::string str = oss.str();
              JsonWriter<T>::write_json(str.c_str(), str.size() - 1, t);
            }
            else
              JsonWriter<T>::write_json((char *) &ch, 1, t);
            break;
          }
        }
        ++it;
      }

      __NKIT__WRITE__JSON__("\"", t);
    }

    template<typename T>
    bool write_dict_item(const std::string & key, const Dynamic & v, T * t,
        const DynamicToJsonOptions & options)
    {
      __NKIT__WRITE__JSON__("\"", t);
      JsonWriter<T>::write_json(key.c_str(), key.size(), t);
      __NKIT__WRITE__JSON__("\":", t);
      return DynamicToJson(v, t, options);
    }

    template<typename T>
    bool write_table_row(const Dynamic & v, const size_t width,
        const StringVector & column_names, const size_t row, T * t,
        const DynamicToJsonOptions & options)
    {
      __NKIT__WRITE__JSON__("{", t);
      size_t last_col = width - 1;
      for (size_t col = 0; col < last_col; ++col)
      {
        if (unlikely(
            !write_dict_item(column_names[col], v.GetCellValue(row, col), t,
                options)))
          return false;
        __NKIT__WRITE__JSON__(",", t);
      }

      if (unlikely(
          !write_dict_item(column_names[last_col], v.GetCellValue(row, last_col),
              t, options)))
        return false;

      __NKIT__WRITE__JSON__("}", t);
      return true;
    }

    template<typename T>
    bool write_table(const Dynamic & v, T * t,
        const DynamicToJsonOptions & options)
    {
      __NKIT__WRITE__JSON__("[", t);
      size_t height = v.height();
      if (likely(height > 0))
      {
        size_t last_row = height - 1;
        StringVector column_names = v.GetColumnNames();
        size_t width = v.width();

        for (size_t row = 0; row < last_row; ++row)
        {
          if (unlikely(!write_table_row(v, width, column_names, row, t,
              options)))
            return false;
          __NKIT__WRITE__JSON__(",", t);
        }

        if (unlikely(!write_table_row(v, width, column_names, last_row, t,
            options)))
          return false;
      }

      __NKIT__WRITE__JSON__("]", t);
      return true;
    }

    template<typename T>
    bool write_dict(const Dynamic & v, T * t,
        const DynamicToJsonOptions & options)
    {
      Dynamic::DictConstIterator h_it, end_d;
      size_t size = v.size();
      if (unlikely(size == 0))
      {
        __NKIT__WRITE__JSON__("{}", t);
      }
      else
      {
        __NKIT__WRITE__JSON__("{", t);

        h_it = v.begin_d();
        for (; size > 1; ++h_it, --size)
        {
          if (unlikely(!write_dict_item(h_it->first, h_it->second, t,
              options)))
            return false;
          __NKIT__WRITE__JSON__(",", t);
        }

        if (unlikely(!write_dict_item(h_it->first, h_it->second, t,
            options)))
          return false;

        __NKIT__WRITE__JSON__("}", t);
      }

      return true;
    }

    template<typename T>
    bool write_list(const Dynamic & v, T * t,
        const DynamicToJsonOptions & options)
    {
      size_t size = v.size();
      if (unlikely(size == 0))
      {
        __NKIT__WRITE__JSON__("[]", t);
      }
      else
      {
        __NKIT__WRITE__JSON__("[", t);
        Dynamic::ListConstIterator a = v.begin_l();
        for (; size > 1; ++a, --size)
        {
          if (unlikely(!DynamicToJson(*a, t, options)))
            return false;
          __NKIT__WRITE__JSON__(",", t);
        }
        if (unlikely(!DynamicToJson(*a, t, options)))
          return false;
        __NKIT__WRITE__JSON__("]", t);
      }

      return true;
    }
  } // namespace detail

  extern DynamicToJsonOptions DEFAULT_DYNAMIC_TO_JSON_OPTIONS_;

  template<typename T>
  inline bool DynamicToJson(const Dynamic & v, T * t,
      const DynamicToJsonOptions & options = DEFAULT_DYNAMIC_TO_JSON_OPTIONS_)
  {
    switch (v.type())
    {
    case detail::BOOL:
      if (v.GetSignedInteger() == 0)
      {
        __NKIT__WRITE__JSON__("false", t);
      }
      else
      {
        __NKIT__WRITE__JSON__("true", t);
      }
      break;
    case detail::INTEGER:
    case detail::UNSIGNED_INTEGER:
    case detail::FLOAT:
      detail::write_number(v, t, options);
      break;
    case detail::STRING:
    case detail::MONGODB_OID:
      detail::write_string_or_mongodb_oid(v, t, options);
      break;
    case detail::DATE_TIME:
      detail::write_date_time(v, t, options);
      break;
    case detail::LIST:
      if (unlikely(!detail::write_list(v, t, options)))
        return false;
      break;
    case detail::DICT:
      if (unlikely(!detail::write_dict(v, t, options)))
        return false;
      break;
    case detail::TABLE:
      if (unlikely(!detail::write_table(v, t, options)))
        return false;
      break;
    case detail::NONE:
    case detail::UNDEF:
    default:
      __NKIT__WRITE__JSON__("null", t)
      ;
      break;
    }
    return true;
  }

  template<typename T>
  inline bool DynamicToJson(const DynamicVector & v, T * t,
      const DynamicToJsonOptions & options = DEFAULT_DYNAMIC_TO_JSON_OPTIONS_)
  {
    __NKIT__WRITE__JSON__("[", t);
    bool first = true;
    DynamicVector::const_iterator item = v.begin(), end = v.end();
    for (; item != end; ++item)
    {
      if (first)
        first = false;
      else
        __NKIT__WRITE__JSON__(", ", t);

      if (!DynamicToJson(*item, t, options))
        return false;
    }

    __NKIT__WRITE__JSON__("]", t);

    return true;
  }

  template<typename T>
  std::string DynamicToJson(const T & v,
      const DynamicToJsonOptions & options = DEFAULT_DYNAMIC_TO_JSON_OPTIONS_)
  {
    std::string out;
    if (!DynamicToJson(v, &out, options))
      return S_EMPTY_;
    return out;
  }

  template<typename T>
  bool DynamicToJsonFile(const T & v, const std::string & file_path,
      std::string * error,
      const DynamicToJsonOptions & options = DEFAULT_DYNAMIC_TO_JSON_OPTIONS_)
  {
    std::ofstream file_stream(file_path.c_str(),
        std::fstream::trunc | std::fstream::binary);
    if (!file_stream.is_open())
    {
      if (error)
        *error = "Counld not open file : " + file_path;
      return false;
    }

    return DynamicToJson(v, &file_stream, options);
  }

  template <typename T>
  bool DynamicToJsonStream(std::ostream & os, const T & v,
      const DynamicToJsonOptions & options = DEFAULT_DYNAMIC_TO_JSON_OPTIONS_)
  {
    long mode = os.iword(detail::json::xalloc);
    switch (mode)
    {
    case detail::JSON_HR_TABLE_AS_DICT:
    case detail::JSON_HR_TABLE_AS_TABLE:
      print(os, v);
      return true;
    default:
    case detail::JSON_HR_ONE_LINE:
      return DynamicToJson(v, &os, options);
    }
  }

  void print(std::ostream & out, const Dynamic & v,
      std::string offset = S_EMPTY_, bool newline = true);

  inline std::ostream & operator << (std::ostream & os, const struct tm & _tm)
  {
    os << Dynamic::DateTimeFromTm(_tm);
    return os;
  }

  Dynamic DynamicFromJson(const std::string & json, std::string * const error);
  Dynamic DynamicFromJsonFile(const std::string & path,
      std::string * const error);

} // namespace nkit

#undef __NKIT__WRITE__JSON__

#endif // __DYNAMIC__JSON__H__

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

#ifndef __NKIT__TOOLS__H__
#define __NKIT__TOOLS__H__

#include <iostream>
#include <algorithm>
#include <locale>
#include <deque>

#include <nkit/types.h>
#include <nkit/ctools.h>
#include <nkit/detail/push_options.h>

#if HAVE_STD_CXX_11
# define NKIT_STATIC_ASSERT(expr, message) static_assert(expr, #message)
#else
# define NKIT_STATIC_ASSERT__(expr, message, line) \
		typedef char static_assertion_##message##line[(expr)?1:-1]

# define NKIT_STATIC_ASSERT_(expr, message, line) \
		NKIT_STATIC_ASSERT__(expr, message, line)

# define NKIT_STATIC_ASSERT(expr, message) \
		NKIT_STATIC_ASSERT_(expr, message, __LINE__)
#endif

#if defined(__GNUC__)
#  define NKIT_UNUSED(v) v __attribute__ ((unused))
#  define DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#  define NKIT_UNUSED(v)
#  define DEPRECATED(func) __declspec(deprecated) func
#else
#  pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#  define DEPRECATED(func) func
#  define NKIT_UNUSED(v)
#endif

#if defined(NKIT_HAS_LIKELY)
#  define likely(expr)  __builtin_expect(!!(expr), 1)
#  define unlikely(expr)  __builtin_expect(!!(expr), 0)
#else
#  define likely(expr) (expr)
#  define unlikely(expr) (expr)
#endif


namespace nkit
{
  extern const std::string WHITE_SPACES;
  extern const std::string WHITE_SPACES_BUT_TAB;
  extern const std::string WHITE_SPACES_BUT_SPACE;
  //----------------------------------------------------------------------------
  void abort_with_core(const std::string & error);

  std::string get_process_id();
  std::string get_hostname();
  std::string get_username();
  bool copy_file(const std::string & from, const std::string & to,
      std::string * error);
  bool move_file(const std::string & from, const std::string & to,
      std::string * error);
  bool delete_file(const std::string & path, std::string * error);

  enum PathType
  {
    PATH_UNKNOWN = 0,
    PATH_FILE,
    PATH_DIR
  };

  bool is_path_exists(const std::string & path, PathType * pt);
  bool path_is_file(const std::string & path);
  bool path_is_dir(const std::string & path);

  bool text_file_to_string(const std::string & path, std::string * out,
      std::string * error);
  bool string_to_text_file(const std::string & path, const std::string & str,
      std::string * error);

  //----------------------------------------------------------------------------
  bool bool_cast(const std::string & str);
  bool bool_cast(const char * str);

  //----------------------------------------------------------------------------
  std::string string_cast(int8_t i);
  std::string string_cast(uint8_t i);
  std::string string_cast(int16_t i);
  std::string string_cast(uint16_t i);
  std::string string_cast(int32_t i);
  std::string string_cast(uint32_t i);
  std::string string_cast(int64_t i);
  std::string string_cast(uint64_t i);
#ifdef __APPLE__
    std::string string_cast(size_t i);
#endif
#ifdef NKIT_WINNT
    std::string string_cast(unsigned long i);
#endif
  std::string string_cast(double v, size_t precision = 2);

  //----------------------------------------------------------------------------
  inline uint64_t & operator << (uint64_t & uv, const std::string & s)
  {
    uv = NKIT_STRTOULL(s.c_str(), NULL, 10);
    return uv;
  }

  inline int64_t & operator << (int64_t & sv, const std::string & s)
  {
    sv = NKIT_STRTOLL(s.c_str(), NULL, 10);
    return sv;
  }

  inline uint8_t & operator << (uint8_t & uv, const std::string & s)
  {
    uv = static_cast<uint8_t>(NKIT_STRTOULL(s.c_str(), NULL, 10));
    return uv;
  }

  inline int8_t & operator << (int8_t & sv, const std::string & s)
  {
    sv = static_cast<int8_t>(NKIT_STRTOLL(s.c_str(), NULL, 10));
    return sv;
  }

  inline uint16_t & operator << (uint16_t & uv, const std::string & s)
  {
    uv = static_cast<uint16_t>(NKIT_STRTOULL(s.c_str(), NULL, 10));
    return uv;
  }

  inline int16_t & operator << (int16_t & sv, const std::string & s)
  {
    sv = static_cast<int16_t>(NKIT_STRTOLL(s.c_str(), NULL, 10));
    return sv;
  }

  inline uint32_t & operator << (uint32_t & uv, const std::string & s)
  {
    uv = static_cast<uint32_t>(NKIT_STRTOULL(s.c_str(), NULL, 10));
    return uv;
  }

  inline int32_t & operator << (int32_t & sv, const std::string & s)
  {
    sv = static_cast<int32_t>(NKIT_STRTOLL(s.c_str(), NULL, 10));
    return sv;
  }

  inline std::string & operator << (std::string & v, const std::string & s)
  {
    v = s;
    return v;
  }

#if NKIT_POSIX_PLATFORM
  inline void sleep(uint32_t millisecs)
  {
    ::usleep(millisecs * 1000);
  }
#elif NKIT_WINNT_PLATFORM
  inline void sleep(uint32_t millisecs)
  {
    ::Sleep(millisecs);
  }
#endif

  //----------------------------------------------------------------------------
  bool is_hex_lower(const std::string & str);

  //----------------------------------------------------------------------------
  void simple_split(const std::string & src, const std::string & delimeter,
      StringVector * dst, const std::string & white_spaces = WHITE_SPACES);
  bool simple_split(const std::string & src, const std::string & delimeter,
      std::string * key, std::string * value,
      const std::string & white_spaces = WHITE_SPACES);
  std::string ltrim(const std::string & src,
      const std::string & white_spaces);
  std::string rtrim(const std::string & src,
      const std::string & white_spaces);
  std::string trim(const std::string & src,
      const std::string & white_spaces);

  bool starts_with(const std::string & what, const std::string & with);
  bool istarts_with(const std::string & what, const std::string & with,
      const std::locale & loc = std::locale());
  bool starts_with(const std::string & what, const char * with);
  bool istarts_with(const std::string & what, const char * with,
      const std::locale & loc = std::locale());
  bool ends_with(const std::string & what, const std::string & with);
  bool iends_with(const std::string & what, const std::string & with,
      const std::locale & loc = std::locale());
  bool ends_with(const std::string & what, const char * with);
  bool iends_with(const std::string & what, const char * with);

  //----------------------------------------------------------------------------
  template<typename T>
  void join(const T & container, const std::string & delimiter,
      const std::string & prefix, const std::string & postfix,
      std::string * out)
  {
    typename T::const_iterator it = container.begin(), end = container.end();
    bool first(true);
    for (; it != end; ++it)
    {
      if (!first)
        *out += delimiter;
      else
        first = false;
      *out += prefix;
      *out += *it;
      *out += postfix;
    }
  }

  //----------------------------------------------------------------------------
  template<typename T1, typename T2>
  void join_pairs(const T1 & container1, const T2 & container2,
      const std::string & in_pair_delimiter, const std::string & delimiter,
      const std::string & prefix, const std::string & postfix,
      std::string * out)
  {
    typename T1::const_iterator it1 = container1.begin(),
        end1 = container1.end();
    typename T2::const_iterator it2 = container2.begin(),
        end2 = container2.end();
    bool first(true);
    for (; it1 != end1 && it2 != end2; ++it1, ++it2)
    {
      if (!first)
        *out += delimiter;
      else
        first = false;
      *out += prefix;
      *out += *it1 + in_pair_delimiter + *it2;
      *out += postfix;
    }
  }

  //----------------------------------------------------------------------------
  template<typename charT>
  struct icharequal
  {
    icharequal( const std::locale & loc ) : loc_(loc) {}
    bool operator()(charT ch1, charT ch2)
    {
        return std::tolower(ch1, loc_) == std::tolower(ch2, loc_);
    }

  private:
    const std::locale& loc_;
  };

  //----------------------------------------------------------------------------
  // find substring (case insensitive)
  template<typename T>
  typename T::size_type stristr(const T& str1, const T& str2,
      const std::locale & loc = std::locale())
  {
    typename T::const_iterator it = std::search(str1.begin(), str1.end(),
        str2.begin(), str2.end(), icharequal<typename T::value_type>(loc));
    if (it != str1.end())
      return it - str1.begin();
    else
      return T::npos;
  }

  //----------------------------------------------------------------------------
  // compare strings (case insensitive)
  template<typename T>
  bool istrequal(const T& str1, const T& str2,
          const std::locale & loc = std::locale())
  {
    if (str1.size() != str2.size())
      return false;
    return std::equal(str1.begin(), str1.end(),
        str2.begin(), icharequal<typename T::value_type>(loc));
  }

  //----------------------------------------------------------------------------
  template <typename T>
  void clear_stack(T & st)
  {
    while (!st.empty())
      st.pop();
  }

  //----------------------------------------------------------------------------
  class ToolsInitializer;

  class TimeMeter
  {
    friend class ToolsInitializer;

    static uint64_t FREQUENCY;

  public:
    TimeMeter();
    ~TimeMeter();
    void Stop();
    void Start();
    double GetTotal() const;
    void Clear();

  private:
    bool stoped_;
    uint64_t total_;
    uint64_t begin_;
  };

  void print(std::ostream & out, const char *, std::string offset = "",
      bool newline = true);
  void print(std::ostream & out, const std::string &, std::string offset = "",
      bool newline = true);
  void print(std::ostream & out, bool v, std::string offset = "",
      bool newline = true);
  void print(std::ostream & out, int32_t v, std::string offset = "",
      bool newline = true);
  void print(std::ostream & out, uint32_t v, std::string offset = "",
      bool newline = true);
  void print(std::ostream & out, int64_t v, std::string offset = "",
      bool newline = true);
  void print(std::ostream & out, uint64_t v, std::string offset = "",
      bool newline = true);
#if defined(__APPLE__)
  void print(std::ostream & out, size_t v, std::string offset = "",
      bool newline = true);
#endif
  void print(std::ostream & out, double v, std::string offset = "",
      bool newline = true);

  template <typename T>
  struct Printer
  {
    static void _print(std::ostream & out, const T & container,
        std::string offset = "")
    {
      out << "[\n";
      typename T::const_iterator it = container.begin(), end = container.end();
      std::string item_offset = offset + "  ";
      size_t size = container.size(), i = 1;
      for (;it != end; ++it, ++i)
      {
        out << item_offset;
        print(out, *it, item_offset, false);
        if (i != size)
          out << ',';
        out << '\n';
      }
      out << "]\n";
    }
  };

  template <typename T>
  std::ostream & operator << (std::ostream & out,
      const std::list<T> & container)
  {
    Printer<std::list<T> >::_print(out, container, "");
    return out;
  }

  template <typename T>
  std::ostream & operator << (std::ostream & out,
      const std::vector<T> & container)
  {
    Printer<std::vector<T> >::_print(out, container, "");
    return out;
  }

  template <typename T>
  std::ostream & operator << (std::ostream & out,
      const std::set<T> & container)
  {
    Printer<std::set<T> >::_print(out, container, "");
    return out;
  }

  template <typename T>
  std::ostream & operator << (std::ostream & out,
      const std::deque<T> & container)
  {
    Printer<std::deque<T> >::_print(out, container, "");
    return out;
  }

  template <typename K, typename V>
  struct Printer<std::map<K, V> >
  {
    static void _print(std::ostream & out, const std::map<K, V>  & container,
        std::string offset = "")
    {
      out << "{\n";
      typename std::map<K, V>::const_iterator it = container.begin(),
          end = container.end();
      std::string item_offset = offset + "  ";
      size_t size = container.size(), i = 1;
      for (;it != end; ++it, ++i)
      {
        out << item_offset;
        print(out, it->first, item_offset, false);
        out << ": ";
        print(out, it->second, item_offset, false);
        if (i != size)
          out << ',';
        out << '\n';
      }
      out << "}\n";
    }
  };

  template <typename T>
  void print(std::ostream & out, const T & container, std::string offset = "")
  {
    Printer<T>::_print(out, container, offset);
  }

  template <typename K, typename V>
  std::ostream & operator << (std::ostream & out,
      const typename std::map<K, V> & map)
  {
    print(out, map, "");
    return out;
  }

  //----------------------------------------------------------------------------
  /*
    Generator of unique container item combinations

    Usage:

    Uint16Vector initial_aids;
    for (size_t i=1; i <= 5; ++i)
      initial_aids.push_back(i);

    UniqueCombinationGenerator<uint16_t> ucgen(initial_aids);
    while (true)
    {
      Uint16Set combination;
      if (!ucgen.GetNext(&combination))
        break;
      print(combination);
    }

    Output:
    [1, 2, 3, 4, 5]
    [2, 3, 4, 5]
    [1, 3, 4, 5]
    [3, 4, 5]
    [1, 2, 4, 5]
    [2, 4, 5]
    [1, 4, 5]
    [4, 5]
    [1, 2, 3, 5]
    [2, 3, 5]
    [1, 3, 5]
    [3, 5]
    [1, 2, 5]
    [2, 5]
    [1, 5]
    [5]
    [1, 2, 3, 4]
    [2, 3, 4]
    [1, 3, 4]
    [3, 4]
    [1, 2, 4]
    [2, 4]
    [1, 4]
    [4]
    [1, 2, 3]
    [2, 3]
    [1, 3]
    [3]
    [1, 2]
    [2]
    [1]
  */
  //--------------------------------------------------------------------------
  template <typename T>
  class UniqueCombinationGenerator
  {
    static const uint64_t MAX_INT_COUNT = 64;
    static const uint64_t MAX_POWER_OF_2 = 0x8000000000000000ll;

    static uint64_t max_int_[MAX_INT_COUNT];
    static uint64_t power_of_2_[MAX_INT_COUNT];

    struct Filler
    {
      Filler()
      {
        max_int_[0] = 1;
        power_of_2_[0] = 1;
        for (uint64_t i=1; i<MAX_INT_COUNT; ++i)
        {
          power_of_2_[i] = power_of_2_[i-1]*2;
          max_int_[i] = power_of_2_[i] - 1;
        }
      }
    };

    static Filler filler_;

  public:
    UniqueCombinationGenerator(const std::vector<T> & items)
      : items_(items)
      , counter_(static_cast<int64_t>(max_int_[items_.size()]))
    {}

    bool GetNext(std::set<T> * combination)
    {
      Get(combination);
      return counter_-- > 0;
    }

  private:
    void Get(std::set<T> * combination)
    {
      NKIT_FORCE_USED(filler_); // this is forces filler_ initialization

      uint64_t mask(power_of_2_[items_.size() - 1]);
      size_t index = items_.size() - 1;
      while (true)
      {
        if (counter_ & mask)
          combination->insert(items_[index]);
        if (mask == 1)
          break;
        mask /= 2;
        if (index == 0)
          break;
        --index;
      }
    }

  private:
    const std::vector<T> & items_;
    int64_t counter_;
  };

  template <typename T>
  uint64_t UniqueCombinationGenerator<T>::max_int_[
    UniqueCombinationGenerator<T>::MAX_INT_COUNT];

  template <typename T>
  uint64_t UniqueCombinationGenerator<T>::power_of_2_[
    UniqueCombinationGenerator<T>::MAX_INT_COUNT];

  template <typename T>
  typename UniqueCombinationGenerator<T>::Filler
    UniqueCombinationGenerator<T>::filler_;
} // namespace nkit

#define NKIT_CHECK_OR_DIE(expr, message) \
  if (unlikely(!(expr))) nkit::abort_with_core((message));

// UTF tools
// String length:
// - utf8:  1 character has max. 6 Bytes  plus 1 Byte  NULL-terminator.
// - utf16: 1 character has max. 2 Shorts plus 1 Short NULL-terminator.

bool    utf16_to_utf8(char *utf8, const uint16_t *utf16, uint8_t *shorts_read);
bool    utf8_to_utf16(uint16_t *utf16, const char *utf8, uint8_t *bytes_read);
uint8_t utf8_bytes(char first_utf8_byte);

#endif // __NKIT__TOOLS__H__

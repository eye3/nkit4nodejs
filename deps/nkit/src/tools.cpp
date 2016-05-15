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

#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <limits>
#include <cstdio>
#include <iomanip>
#include <fstream>

#include "nkit/tools.h"
#include "nkit/constants.h"
#include "nkit/logger_brief.h"

#if defined(NKIT_WINNT)
# include <LMCons.h>
#endif
namespace nkit
{
  void abort_with_core(const std::string & error)
  {
    std::cerr << error << std::endl;
    std::flush(std::cerr);
    uint64_t * i = 0;
    *i = 1;
  }

  //----------------------------------------------------------------------------
  const std::string WHITE_SPACES(" \n\t\r");
  const std::string WHITE_SPACES_BUT_TAB(" \n\r");
  const std::string WHITE_SPACES_BUT_SPACE("\t\n\r");

  bool is_white_space(char ch, const std::string & white_spaces)
  {
    return strchr(white_spaces.c_str(), ch) != NULL;
    //return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r';
  }

  //----------------------------------------------------------------------------
  size_t skip_space_forward(const char * src, size_t pos,
      const size_t size, const std::string & white_spaces = WHITE_SPACES)
  {
    while ((pos < size) && is_white_space(src[pos], white_spaces))
      ++pos;
    return pos;
  }

  //----------------------------------------------------------------------------
  size_t skip_space_backward(const char* src, size_t pos,
      const std::string & white_spaces = WHITE_SPACES)
  {
    while (is_white_space(src[pos], white_spaces))
    {
      if (pos == 0)
        break;
      --pos;
    }
    return pos;
  }

  //----------------------------------------------------------------------------
  std::string ltrim(const std::string & str, const std::string & white_spaces)
  {
    size_t size = str.size();
    size_t first_pos = skip_space_forward(str.c_str(), 0, size, white_spaces);
    if (first_pos >= size)
      return S_EMPTY_;
    return str.substr(first_pos, size - first_pos);
  }

  //----------------------------------------------------------------------------
  std::string rtrim(const std::string & str, const std::string & white_spaces)
  {
    size_t size = str.size();
    size_t last_pos = skip_space_backward(str.c_str(), size-1, white_spaces);
    return str.substr(0, last_pos + 1);
  }

  //----------------------------------------------------------------------------
  std::string trim(const std::string & str, const std::string & white_spaces)
  {
    size_t size = str.size();
    size_t first_pos = skip_space_forward(str.c_str(), 0, size, white_spaces);
    if (first_pos >= size)
      return S_EMPTY_;
    size_t last_pos = skip_space_backward(str.c_str(), size-1, white_spaces);
    return str.substr(first_pos, last_pos - first_pos + 1);
  }

  //----------------------------------------------------------------------------
  bool bool_cast(const char * str, size_t size)
  {
    if (size == 0)
      return false;

    size_t first_pos = skip_space_forward(str, 0, size);
    if (first_pos >= size)
      return false;

    char * first = const_cast<char *>(str + first_pos);
    switch (*first)
    {
    case 't':
      return strcmp(first, S_TRUE_.c_str()) == 0;
    case 'T':
      return (strcmp(first, S_TRUE_CAP_.c_str()) == 0)
          || (strcmp(first, S_TRUE_CAP_CAP_.c_str()) == 0);
    case 'y':
      return strcmp(first, S_YES_.c_str()) == 0;
    case 'Y':
      return (strcmp(first, S_YES_CAP_.c_str()) == 0)
          || (strcmp(first, S_YES_CAP_CAP_.c_str()) == 0);
    }

    return (*first > 48 && *first <=57); // 1..9
  }

  //----------------------------------------------------------------------------
  bool bool_cast(const char * str)
  {
    return bool_cast(str, strlen(str));
  }

  //----------------------------------------------------------------------------
  bool bool_cast(const std::string & str)
  {
    return bool_cast(str.c_str(), str.size());
  }

  //----------------------------------------------------------------------------
  const int BUF_LEN = 128;

  //----------------------------------------------------------------------------
  std::string string_cast(int8_t i)
  {
    char tmp[BUF_LEN];
    const int rc = NKIT_SNPRINTF(tmp, BUF_LEN, NKIT_FORMAT_I8, i);
    if (unlikely(rc > BUF_LEN || rc < 0))
      return S_NAN_;
    return std::string(tmp);
  }

  //----------------------------------------------------------------------------
  std::string string_cast(uint8_t i)
  {
    char tmp[BUF_LEN];
    const int rc = NKIT_SNPRINTF(tmp, BUF_LEN, NKIT_FORMAT_U8, i);
    if (unlikely(rc > BUF_LEN || rc < 0))
      return S_NAN_;
    return std::string(tmp);
  }

  //----------------------------------------------------------------------------
  std::string string_cast(int16_t i)
  {
    char tmp[BUF_LEN];
    const int rc = NKIT_SNPRINTF(tmp, BUF_LEN, NKIT_FORMAT_I16, i);
    if (unlikely(rc > BUF_LEN || rc < 0))
      return S_NAN_;
    return std::string(tmp);
  }

  //----------------------------------------------------------------------------
  std::string string_cast(uint16_t i)
  {
    char tmp[BUF_LEN];
    const int rc = NKIT_SNPRINTF(tmp, BUF_LEN, NKIT_FORMAT_U16, i);
    if (unlikely(rc > BUF_LEN || rc < 0))
      return S_NAN_;
    return std::string(tmp);
  }

  //----------------------------------------------------------------------------
  std::string string_cast(int32_t i)
  {
    char tmp[BUF_LEN];
    const int rc = NKIT_SNPRINTF(tmp, BUF_LEN, NKIT_FORMAT_I32, i);
    if (unlikely(rc > BUF_LEN || rc < 0))
      return S_NAN_;
    return std::string(tmp);
  }

  //----------------------------------------------------------------------------
  std::string string_cast(uint32_t i)
  {
    char tmp[BUF_LEN];
    const int rc = NKIT_SNPRINTF(tmp, BUF_LEN, NKIT_FORMAT_U32, i);
    if (unlikely(rc > BUF_LEN || rc < 0))
      return S_NAN_;
    return std::string(tmp);
  }

  //----------------------------------------------------------------------------
  std::string string_cast(int64_t i)
  {
    char tmp[BUF_LEN];
    const int rc = NKIT_SNPRINTF(tmp, BUF_LEN, NKIT_FORMAT_I64, i);
    if (unlikely(rc > BUF_LEN || rc < 0))
      return S_NAN_;
    return std::string(tmp);
  }

  //----------------------------------------------------------------------------
    std::string string_cast(uint64_t i)
    {
        char tmp[BUF_LEN];
        const int rc = NKIT_SNPRINTF(tmp, BUF_LEN, NKIT_FORMAT_U64, i);
        if (unlikely(rc > BUF_LEN || rc < 0))
            return S_NAN_;
        return std::string(tmp);
    }
#if defined(__APPLE__)
    std::string string_cast(size_t i)
    {
      char tmp[BUF_LEN];
      const int rc = NKIT_SNPRINTF(tmp, BUF_LEN, "%zd", i);
      if (unlikely(rc > BUF_LEN || rc < 0))
      return S_NAN_;
      return std::string(tmp);
    }
#endif
#if defined(NKIT_WINNT)
    std::string string_cast(unsigned long i)
    {
      return string_cast(static_cast<size_t>(i));
    }
#endif
  //----------------------------------------------------------------------------
  static const char * formats_of_double[] =
  { "%.0f\0", "%.1f\0", "%.2f\0", "%.3f\0", "%.4f\0", "%.5f\0", "%.6f\0",
      "%.7f\0", "%.8f\0", "%.9f\0", "%.10f\0", "%.11f\0", "%.12f\0", "%.13f\0",
      "%.14f\0", "%.15f\0" };

  static const size_t formats_of_double_count =
      sizeof(formats_of_double) / sizeof(formats_of_double[0]);

  std::string string_cast(double v, size_t precision)
  {
    if (precision >= formats_of_double_count)
      return S_NAN_;
    char tmp[BUF_LEN];
    if (unlikely(NKIT_SNPRINTF(tmp, BUF_LEN, formats_of_double[precision], v) >=
        BUF_LEN))
      return S_NAN_;
    return std::string(tmp);
  }

  //----------------------------------------------------------------------------
  bool is_hex_lower(const std::string & str)
  {
    size_t size = str.size();
    for (size_t i=0; i < size; ++i)
    {
      char ch = str[i];
      if ( ((ch < '0') || (ch > '9')) && ((ch < 'a') || (ch > 'f')))
        return false;
    }
    return true;
  }

  //----------------------------------------------------------------------------
  void simple_split(const std::string & src, const std::string & delimeter,
      StringVector * dst, const std::string & white_spaces)
  {
    dst->clear();
    size_t size = src.size();
    if (size == 0)
      return;
    size_t first_pos = skip_space_forward(src.c_str(), 0, size, white_spaces);
    if (first_pos >= size)
      return;

    size_t delimeter_size = delimeter.size();

    size_t pos = src.find(delimeter, first_pos);
    while (pos != src.npos)
    {
      size_t last_pos = pos > 0 ?
          skip_space_backward(src.c_str(), pos-1, white_spaces) : -1;
      dst->push_back(src.substr(first_pos, last_pos - first_pos + 1));
      first_pos = skip_space_forward(src.c_str(), pos + delimeter_size, size,
          white_spaces);
      pos = src.find(delimeter, first_pos);
    }

    dst->push_back(src.substr(first_pos, size - first_pos));
  }

  //----------------------------------------------------------------------------
  bool simple_split(const std::string & src, const std::string & delimeter,
      std::string * key, std::string * value,
      const std::string & white_spaces)
  {
    key->clear();
    value->clear();
    size_t size = src.size();
    if (size == 0)
      return false;
    size_t first_pos = skip_space_forward(src.c_str(), 0, size, white_spaces);
    if (first_pos >= size)
      return false;

    size_t delimeter_size = delimeter.size();

    size_t pos = src.find(delimeter, first_pos);
    if (pos == src.npos)
    {
      size_t last_pos = skip_space_backward(src.c_str(), size - 1, white_spaces);
      *key = src.substr(first_pos, last_pos - first_pos + 1);
      return false;
    }
    else
    {
      size_t last_pos = pos > 0 ?
          skip_space_backward(src.c_str(), pos-1, white_spaces) : -1;
      *key = src.substr(first_pos, last_pos - first_pos + 1);
      first_pos = skip_space_forward(src.c_str(), pos + delimeter_size, size,
          white_spaces);
      if (first_pos < size)
      {
        last_pos = skip_space_backward(src.c_str(), size - 1, white_spaces);
        *value = src.substr(first_pos, size - first_pos + 1);
      }
    }
    return true;
  }

  void print(std::ostream & out, const char * s, std::string _offset,
      bool newline)
  {
    print(out, std::string(s), _offset, newline);
  }

  void print(std::ostream & out, const std::string & s, std::string _offset,
      bool newline)
  {
    _offset = newline ? "" : _offset;
    out << _offset << s;
    if (newline)
      out << '\n';
  }

  void print(std::ostream & out, bool v, std::string _offset, bool newline)
  {
    std::string s(v ? "true" : "false");
    _offset = newline ? "" : _offset;
    out << _offset << s;
    if (newline)
      out << '\n';
  }

  void print(std::ostream & out, int32_t v, std::string _offset, bool newline)
  {
    _offset = newline ? "" : _offset;
    out << _offset << v;
    if (newline)
      out << '\n';
  }

  void print(std::ostream & out, uint32_t v, std::string _offset, bool newline)
  {
    _offset = newline ? "" : _offset;
    out << _offset << v;
    if (newline)
      out << '\n';
  }

  void print(std::ostream & out, int64_t v, std::string _offset, bool newline)
  {
    _offset = newline ? "" : _offset;
    out << _offset << v;
    if (newline)
      out << '\n';
  }

  void print(std::ostream & out, uint64_t v, std::string _offset, bool newline)
  {
    _offset = newline ? "" : _offset;
    out << _offset << v;
    if (newline)
      out << '\n';
  }

#ifdef __APPLE__
  void print(std::ostream & out, size_t v, std::string _offset, bool newline)
  {
    _offset = newline ? "" : _offset;
    out << _offset << v;
    if (newline)
      out << '\n';
  }
#endif

  void print(std::ostream & out, double v, std::string _offset, bool newline)
  {
    _offset = newline ? "" : _offset;
    out << _offset << v;
    if (newline)
      out << '\n';
  }

  bool starts_with(const std::string & what, const std::string & with)
  {
    if (what.size() < with.size())
      return false;
    return std::equal(with.begin(), with.end(), what.begin());
  }

  bool istarts_with(const std::string & what, const std::string & with,
      const std::locale & loc)
  {
    if (what.size() < with.size())
      return false;
    return std::equal(with.begin(), with.end(), what.begin(),
        icharequal<char>(loc));
  }

  bool starts_with(const std::string & what, const char * with)
  {
    size_t len = strlen(with);
    if (what.size() < len)
      return false;
    return std::equal(with, with + len, what.begin());
  }

  bool istarts_with(const std::string & what, const char * with,
      const std::locale & loc)
  {
    size_t len = strlen(with);
    if (what.size() < len)
      return false;
    return std::equal(with, with + len, what.begin(),
        icharequal<char>(loc));
  }

  bool ends_with(const std::string & what, const std::string & with)
  {
    if (what.size() < with.size())
      return false;
    return std::equal(with.rbegin(), with.rend(), what.rbegin());
  }

  bool iends_with(const std::string & what, const std::string & with,
      const std::locale & loc)
  {
    if (what.size() < with.size())
      return false;
    return std::equal(with.rbegin(), with.rend(), what.rbegin(),
        icharequal<char>(loc));
  }

  bool ends_with(const std::string & what, const char * with)
  {
    size_t len = strlen(with);
    if (what.size() < len)
      return false;
    return strncmp(what.c_str() + (what.size() - len), with, len) == 0;
  }

  bool iends_with(const std::string & what, const char * with)
  {
    size_t len = strlen(with);
    if (what.size() < len)
      return false;
    return NKIT_STRNCASECMP(what.c_str() + (what.size() - len), with, len) == 0;
  }

  //----------------------------------------------------------------------------
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

static const time_t __FREQUENCY = 1000000000;

#ifdef NKIT_POSIX_PLATFORM
  uint64_t cputime()
  {
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t t;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &t);
    mach_port_deallocate(mach_task_self(), cclock);
    return t.tv_sec * __FREQUENCY + t.tv_nsec;
#else
    timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * __FREQUENCY + t.tv_nsec;
#endif
  }
#elif NKIT_WINNT_PLATFORM
  uint64_t cputime()
  {
    LARGE_INTEGER counter;
    if (!QueryPerformanceCounter(&counter))
      abort_with_core("QueryPerformanceCounter() error");
    return counter.QuadPart;
  }
#endif

  uint64_t TimeMeter::FREQUENCY;

  TimeMeter::TimeMeter()
    : stoped_(true)
    , total_(0)
    , begin_(0)
  {}

  TimeMeter::~TimeMeter()
  {
    Stop();
  }

  void TimeMeter::Start()
  {
    if (!stoped_)
      return;

    stoped_ = false;
    begin_ = cputime();
  }

  void TimeMeter::Stop()
  {
    if (stoped_)
      return;

    stoped_ = true;
    uint64_t stop_point = cputime();
    total_ += stop_point - begin_;
  }

  double TimeMeter::GetTotal() const
  {
    return (total_*1.0 / FREQUENCY);
  }

  void TimeMeter::Clear()
  {
    total_ = 0;
  }

  class ToolsInitializer
  {
  public:
    ToolsInitializer()
    {
#ifdef NKIT_POSIX_PLATFORM
      TimeMeter::FREQUENCY = __FREQUENCY;
#elif NKIT_WINNT_PLATFORM
      LARGE_INTEGER frequency;
      if (!QueryPerformanceFrequency(&frequency))
        abort_with_core("QueryPerformanceFrequency() error");
      TimeMeter::FREQUENCY = frequency.QuadPart;
#else
#  pragma error("ERROR: unsupported platform")
#endif
    }
  };

  static const char * UNKNOWN = "unknown";

  std::string get_process_id()
  {
#if defined(NKIT_POSIX_PLATFORM)
    return "[" + string_cast((uint32_t)getpid()) + "]";
#elif defined(NKIT_WINNT)
  return "[" + string_cast((uint32_t)::GetCurrentProcessId()) + "]";
#else
#  error "static inline std::string get_process_id() does not supported"
#endif
  }

  std::string get_hostname()
  {
#if defined(NKIT_POSIX_PLATFORM)
    char buf[255];
    if (gethostname(&buf[0], sizeof(buf)) == -1)
      return UNKNOWN;
    return buf;
#elif defined(NKIT_WINNT)
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    char name[MAX_COMPUTERNAME_LENGTH + 1];
    const BOOL ret = ::GetComputerName(name, &size);
    assert(ret == TRUE);
    return name;
#else
#  error "static inline std::string get_hostname() does not supported"
#endif
  }

  std::string get_username()
  {
#if defined(NKIT_POSIX_PLATFORM)
    char * login = getpwuid(getuid())->pw_name;
    if (!login)
      return "";
    return login;
#elif defined(NKIT_WINNT)
  DWORD size = UNLEN + 1;
  char name[UNLEN + 1];
  const BOOL ret = ::GetUserName(name, &size);
  if(ret != TRUE)
    return "";
  return name;
#else
#  error "static inline std::string get_username() does not supported"
#endif
  }

  bool copy_file(const std::string & from, const std::string & to,
      std::string * error)
  {
    char buf[BUFSIZ];
    size_t size;

    FILE * source = std::fopen(from.c_str(), "r");
    if (!source)
    {
      *error = strerror(errno);
      return false;
    }

    FILE * dest = std::fopen(to.c_str(), "w");
    if (!dest)
    {
      *error = strerror(errno);
      return false;
    }

    while ((size = std::fread(buf, 1, BUFSIZ, source)) > 0)
      std::fwrite(buf, 1, size, dest);

    std::fclose(source);
    std::fclose(dest);

    return true;
  }

  bool rename_file(const std::string & from, const std::string & to,
      std::string * error)
  {
    if (std::rename(from.c_str(), to.c_str()) != 0)
    {
      *error = strerror(errno);
      return false;
    }

    return true;
  }

  bool move_file(const std::string & from, const std::string & to,
      std::string * error)
  {
    std::string tmp;
    if (!rename_file(from, to, &tmp))
    {
      if (!copy_file(from, to, error))
        return false;
      if (!delete_file(from, error))
      {
        delete_file(to, &tmp);
        return false;
      }
    }

    return true;
  }

  bool delete_file(const std::string & path, std::string * error)
  {
    if (std::remove(path.c_str()) != 0)
    {
      *error = strerror(errno);
      return false;
    }

    return true;
  }

  bool is_path_exists(const std::string & path, PathType * pt)
  {
    struct stat info;
    if (stat(path.c_str(), &info) == -1)
      return false;
    if (info.st_mode & S_IFDIR)
      *pt = PATH_DIR;
    else if (info.st_mode & S_IFREG)
      *pt = PATH_FILE;
    else
      *pt = PATH_UNKNOWN;
    return true;
  }

  bool path_is_file(const std::string & path)
  {
    PathType pt;
    return is_path_exists(path, &pt) && pt == PATH_FILE;
  }

  bool path_is_dir(const std::string & path)
  {
    PathType pt;
    return is_path_exists(path, &pt) && pt == PATH_DIR;
  }

  bool text_file_to_string(const std::string & path, std::string * out,
      std::string * error)
  {
    FILE * source = std::fopen(path.c_str(), "r");
    if (!source)
    {
      *error = strerror(errno);
      return false;
    }

    char buf[BUFSIZ];
    size_t size;

    while ((size = std::fread(buf, 1, BUFSIZ, source)) > 0)
      out->append(buf, size);

    std::fclose(source);
    return true;
  }

  bool string_to_text_file(const std::string & path, const std::string & str,
      std::string * error)
  {
    FILE * dest = std::fopen(path.c_str(), "w");
    if (!dest)
    {
      *error = strerror(errno);
      return false;
    }

    std::fwrite(str.c_str(), 1, str.size(), dest);
    std::fclose(dest);
    return true;
  }

  static ToolsInitializer tools_initializer;

} // namespace nkit

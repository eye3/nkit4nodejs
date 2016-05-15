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

#include <limits>

#include "nkit/constants.h"
#include "nkit/tools.h"

#define NKIT_DEFINE_CONST_STRING(name, value) \
  const std::string name(value);

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace nkit
{
  const char * GMT_FORMAT_ = "%a, %d %b %Y %H:%M:%S GMT";
  const char * DATE_TIME_DEFAULT_FORMAT()
  {
    static const char * result = "%Y-%m-%d %H:%M:%S";
    return result;
  }

  const char * Y_m_d_H_M_S_ = DATE_TIME_DEFAULT_FORMAT();
  const char * DATE_TIME_DEFAULT_FORMAT_ = DATE_TIME_DEFAULT_FORMAT();

  const std::string S_DATE_TIME_DEFAULT_FORMAT_(DATE_TIME_DEFAULT_FORMAT());
  const std::string S_FLOAT_DEFAULT_(NKIT_FORMAT_DOUBLE);

  uint64_t MAX_UINT64_VALUE = std::numeric_limits<uint64_t>::max();
  uint32_t MAX_UINT32_VALUE = std::numeric_limits<uint32_t>::max();
  uint16_t MAX_UINT16_VALUE = std::numeric_limits<uint16_t>::max();
  int64_t MAX_INT64_VALUE = std::numeric_limits<int64_t>::max();
  int64_t MIN_INT64_VALUE = std::numeric_limits<int64_t>::min();
  int32_t MAX_INT32_VALUE = std::numeric_limits<int32_t>::max();
  int32_t MIN_INT32_VALUE = std::numeric_limits<int32_t>::min();
  int16_t MAX_INT16_VALUE = std::numeric_limits<int16_t>::max();
  int16_t MIN_INT16_VALUE = std::numeric_limits<int16_t>::min();

  NKIT_DEFINE_CONST_STRING(S_LF_, "\n")
  NKIT_DEFINE_CONST_STRING(S_CR_, "\r")
  NKIT_DEFINE_CONST_STRING(S_EMPTY_, "")
  NKIT_DEFINE_CONST_STRING(S_ZERO_, "0")
  NKIT_DEFINE_CONST_STRING(S_FALSE_, "false")
  NKIT_DEFINE_CONST_STRING(S_FALSE_CAP_, "False")
  NKIT_DEFINE_CONST_STRING(S_TRUE_, "true")
  NKIT_DEFINE_CONST_STRING(S_TRUE_CAP_, "True")
  NKIT_DEFINE_CONST_STRING(S_TRUE_CAP_CAP_, "TRUE")
  NKIT_DEFINE_CONST_STRING(S_YES_, "yes")
  NKIT_DEFINE_CONST_STRING(S_YES_CAP_, "Yes")
  NKIT_DEFINE_CONST_STRING(S_YES_CAP_CAP_, "YES")
  NKIT_DEFINE_CONST_STRING(S_NO_, "no")
  NKIT_DEFINE_CONST_STRING(S_NO_CAP_, "No")
  NKIT_DEFINE_CONST_STRING(S_NULL_, "null")
  NKIT_DEFINE_CONST_STRING(S_NAN_, "NaN")
  NKIT_DEFINE_CONST_STRING(S_DOT_, ".")
  NKIT_DEFINE_CONST_STRING(S_STAR_, "*")
  NKIT_DEFINE_CONST_STRING(S_LT_, "<")
  NKIT_DEFINE_CONST_STRING(S_GT_, ">")
  NKIT_DEFINE_CONST_STRING(S_UTF_8_, "UTF-8")
  NKIT_DEFINE_CONST_STRING(S_CDATA_BEGIN_, "<![CDATA[")
  NKIT_DEFINE_CONST_STRING(S_CDATA_END_, "]]>")

  time_t timezone_offset()
  {
    static time_t TIMEZONE_OFFSET = 0;
    if (unlikely(!TIMEZONE_OFFSET))
    {
      time_t t = time(NULL);
      struct tm local_tm;
      struct tm gmt_tm;
      LOCALTIME_R(t, &local_tm);
      GMTIME_R(t, &gmt_tm);
      TIMEZONE_OFFSET = mktime(&gmt_tm) - mktime(&local_tm);
    }
    return TIMEZONE_OFFSET;
  }
}  // namespace nkit

#undef NKIT_DEFINE_CONST_STRING

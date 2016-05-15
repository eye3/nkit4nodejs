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

#ifndef __NKIT__CONSTANTS__INCLUDED__
#define __NKIT__CONSTANTS__INCLUDED__

#include <string>
#include <nkit/types.h>

#define NKIT_DECLARE_CONST_STRING(name) \
  extern const std::string name;

namespace nkit
{
  extern const char * GMT_FORMAT_;               // "%a, %d %b %Y %H:%M:%S GMT"
  extern const char * Y_m_d_H_M_S_;              // "%Y-%m-%d %H:%M:%S"
  const char * DATE_TIME_DEFAULT_FORMAT();       // "%Y-%m-%d %H:%M:%S"
  extern const char * DATE_TIME_DEFAULT_FORMAT_; // "%Y-%m-%d %H:%M:%S"
  extern const std::string S_DATE_TIME_DEFAULT_FORMAT_;

  extern const std::string S_FLOAT_DEFAULT_;

  extern uint64_t MAX_UINT64_VALUE;
  extern uint32_t MAX_UINT32_VALUE;
  extern uint16_t MAX_UINT16_VALUE;
  extern int64_t MAX_INT64_VALUE;
  extern int64_t MIN_INT64_VALUE;
  extern int32_t MAX_INT32_VALUE;
  extern int32_t MIN_INT32_VALUE;
  extern int16_t MAX_INT16_VALUE;
  extern int16_t MIN_INT16_VALUE;
  NKIT_DECLARE_CONST_STRING(S_LF_)
  NKIT_DECLARE_CONST_STRING(S_CR_)
  NKIT_DECLARE_CONST_STRING(S_EMPTY_)
  NKIT_DECLARE_CONST_STRING(S_FALSE_)
  NKIT_DECLARE_CONST_STRING(S_FALSE_CAP_)
  NKIT_DECLARE_CONST_STRING(S_TRUE_)
  NKIT_DECLARE_CONST_STRING(S_TRUE_CAP_)
  NKIT_DECLARE_CONST_STRING(S_TRUE_CAP_CAP_)
  NKIT_DECLARE_CONST_STRING(S_YES_)
  NKIT_DECLARE_CONST_STRING(S_YES_CAP_)
  NKIT_DECLARE_CONST_STRING(S_YES_CAP_CAP_)
  NKIT_DECLARE_CONST_STRING(S_NO_)
  NKIT_DECLARE_CONST_STRING(S_NO_CAP_)
  NKIT_DECLARE_CONST_STRING(S_ZERO_)
  NKIT_DECLARE_CONST_STRING(S_NULL_)
  NKIT_DECLARE_CONST_STRING(S_NAN_)
  NKIT_DECLARE_CONST_STRING(S_DOT_)
  NKIT_DECLARE_CONST_STRING(S_STAR_)
  NKIT_DECLARE_CONST_STRING(S_LT_)
  NKIT_DECLARE_CONST_STRING(S_GT_)
  NKIT_DECLARE_CONST_STRING(S_UTF_8_)
  NKIT_DECLARE_CONST_STRING(S_CDATA_BEGIN_)
  NKIT_DECLARE_CONST_STRING(S_CDATA_END_)

  time_t timezone_offset();
}  // namespace nkit

#undef NKIT_DECLARE_CONST_STRING

#endif // __NKIT__CONSTANTS__INCLUDED__

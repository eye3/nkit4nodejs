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

#ifndef LOGGER_BRIFE_H_
#define LOGGER_BRIFE_H_

#include <nkit/logger.h>

//------------------------------------------------------------------------------
// Shorter alternatives to logger macroses
//------------------------------------------------------------------------------
#ifdef INFO
#undef INFO
#endif
#define INFO(v) NKIT_LOG_INFO(v)

#ifdef CINFO
#undef CINFO
#endif
#define CINFO(v) NKIT_LOG_INFO(::nkit::console_logger << v)

#ifdef ERR
#undef ERR
#endif
#define ERR(v) NKIT_LOG_ERROR(v)

#ifdef CERR
#undef CERR
#endif
#define CERR(v) NKIT_LOG_ERROR(::nkit::console_logger << v)

#ifdef DBG
#undef DBG
#endif
#define DBG(v) NKIT_LOG_DEBUG(v)

#ifdef CDBG
#undef CDBG
#endif
#define CDBG(v) NKIT_LOG_DEBUG(::nkit::console_logger << v)

#ifdef WARN
#undef WARN
#endif
#define WARN(v) NKIT_LOG_WARNING(v)

#ifdef CWARN
#undef CWARN
#endif
#define CWARN(v) NKIT_LOG_WARNING(::nkit::console_logger << v)


#endif /* LOGGER_BRIFE_H_ */

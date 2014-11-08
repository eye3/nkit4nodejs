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

#ifndef __NKIT__DETAIL__PUSH__OPTIONS__H__
#define __NKIT__DETAIL__PUSH__OPTIONS__H__

#include <nkit/detail/config.h>

#if defined(__FreeBSD__) || defined(__APPLE__)
#  define NKIT_MACOS 1
#  define NKIT_POSIX_PLATFORM 1
#  define NKIT_PTHREAD 1
#elif defined(__linux__) || defined(linux) || defined(__CYGWIN__)
#  define NKIT_POSIX_PLATFORM 1
#  define NKIT_PTHREAD 1
#elif defined(_WIN32) || defined(_WIN64)
#  pragma warning(disable : 4786)
#  if defined(_MSC_VER)  &&  _MSC_VER <= 1200 // MSVC 6
// Microsoft Visual Studio 6 only support conversion from __int64 to double
// (no conversion from unsigned __int64).
#    define NKIT_USE_CUSTOM_UINT64_CAST_TO_DOUBLE 1
#  endif // if defined(_MSC_VER)  &&  _MSC_VER < 1200 // MSVC 6
#  define NKIT_WINNT_PLATFORM 1
// TODO Set NT version since NT have compatibility issues
#  define NKIT_WINNT 1
#else
#  error "Unknown platform"
#endif

#if defined(NKIT_POSIX_PLATFORM) && defined(__GNUC__)
#  define NKIT_HAS_LIKELY 1
#endif // NKIT_HAS_LIKELY && __GNUC__

#if defined(USE_REF_COUNT_PTR)
#  define NKIT_USE_REF_COUNT_PTR 1
#elif defined(HAVE_STD_CXX_11) && !defined(USE_BOOST)
#  define NKIT_USE_STD_CXX_11 1
#elif defined(HAVE_BOOST)
#  define NKIT_USE_BOOST 1
#endif

#if defined(_DEBUG) || defined(DEBUG)
#  define NKIT_DEBUG
#else
#  define NKIT_RELEASE
#endif // defined(_DEBUG) || defined(DEBUG)

#endif


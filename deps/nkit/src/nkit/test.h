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

#ifndef __NKIT__TEST__H__
#define __NKIT__TEST__H__

#include <exception>
#include <string>

#include "nkit/tools.h"

namespace nkit
{
  namespace test
  {
    namespace detail
    {
      class AssertError: public std::exception
      {
      public:
        AssertError(const std::string & file, uint32_t line,
            const std::string & text)
        {
          text_ = file + ":" + nkit::string_cast(line) + ": error: " + text + "\n";
        }
        AssertError(const std::string & file, uint32_t line,
            const std::string & text1, const std::string & text2)
        {
          text_ = file + ":" + nkit::string_cast(line) + ": error: " + text1 +
              + ": " + text2 + "\n";
        }
        ~AssertError() throw ()
        {
        }
        const char* what() const throw ()
        {
          return text_.c_str();
        }

      private:
        std::string text_;
      };

      typedef void (*TestFunction)(void);
      void RegisterTestFunction(const std::string & name, TestFunction f);
    } // namespace detail

    int run_all_tests();

  } // namespace test
} // namespace nkit

#ifdef NKIT_TEST_FATAL_ERRORS

#define NKIT_TEST_ASSERT(a) if (!(a)) \
  nkit::abort_with_core(#a)

#define NKIT_TEST_ASSERT_WITH_TEXT(a, s) if (!(a)) \
    nkit::abort_with_core(#a + (s))

#else

#define NKIT_TEST_ASSERT(a) if (!(a)) \
  throw nkit::test::detail::AssertError(__FILE__, __LINE__, #a)

#define NKIT_TEST_ASSERT_WITH_TEXT(a, s) if (!(a)) \
  throw nkit::test::detail::AssertError(__FILE__, __LINE__, #a, (s))

#endif

#define NKIT_TEST_CASE(TestName) class TestName##Class\
  {\
  public:\
    TestName##Class()\
    {\
      nkit::test::detail::RegisterTestFunction(#TestName, Run);\
    }\
    static void Run();\
  };\
  TestName##Class TestName##_test_registrator;\
  void TestName##Class::Run()

#define _NKIT_TEST_CASE(TestName) class TestName##Class\
  {\
  public:\
  TestName##Class()\
  {\
    }\
  static void Run();\
  };\
  void TestName##Class::Run()

#endif // __NKIT__TEST__H__

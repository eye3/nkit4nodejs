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

#include <string>

#include "nkit/dynamic.h"
#include "nkit/logger.h"
#include "nkit/test.h"

namespace nkit
{
  namespace test
  {
    namespace detail
    {
      typedef std::map<std::string, TestFunction> TestFunctions;

      TestFunctions & GetTestFunctions()
      {
        static TestFunctions test_functions;
        return test_functions;
      }

      void RegisterTestFunction(const std::string & name, TestFunction f)
      {
        GetTestFunctions()[name] = f;
      }

    } // namespace detail

    int run_all_tests()
    {
      size_t total(nkit::test::detail::GetTestFunctions().size());
      size_t error_count(total);
      NKIT_LOG_INFO(console_logger <<
          "----------------------------------");
      NKIT_LOG_INFO(console_logger << "Running " << total << " tests.");
      nkit::test::detail::TestFunctions::const_iterator
        it = nkit::test::detail::GetTestFunctions().begin(),
        end = nkit::test::detail::GetTestFunctions().end();
      for(; it != end; ++it)
      {
        std::string name(it->first);
        nkit::test::detail::TestFunction f(it->second);
        try
        {
          f();
          NKIT_LOG_INFO(console_logger << name << "() is passed.");
          --error_count;
        }
        catch(const nkit::test::detail::AbortWithError & e)
        {
          NKIT_LOG_ERROR(console_logger << "\n\n" << name << "(): " << e.what()
              << "\n");
          exit(1);
        }
        catch(const std::exception & e)
        {
          NKIT_LOG_ERROR(console_logger << "\n\n" << name << "(): " << e.what()
              << "\n");
        }
        catch(...)
        {
          NKIT_LOG_ERROR(console_logger <<
              "\n\nUnexpected error occurred while running test case "
              << name << "()\n");
        }
      }
      if (!error_count)
        NKIT_LOG_INFO(console_logger << "\nAll tests are passed.");
      else
      {
        NKIT_LOG_INFO(console_logger <<
            (total - error_count) << " test(s) are passed.");
        NKIT_LOG_INFO(console_logger <<
            error_count << " test(s) are not passed.");
      }
      NKIT_LOG_INFO(console_logger << "----------------------------------");

      return error_count > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
    }
  } // namespace test
} // namespace nkit

/*
   Copyright 2014 Boris T. Darchiev (boris.darchiev@gmail.com)
                  Vasiliy Soshnikov (dedok.mad@gmail.com)

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

#ifndef __NKIT__CONSOLE__LOGGER__H__
#define __NKIT__CONSOLE__LOGGER__H__

#include <nkit/types.h>

#include <cstring>
#include <iostream>

namespace nkit
{
  class ConsoleLogger : public Logger
  {
  public:
    static Logger::Ptr Create()
    {
      return Logger::Ptr(new ConsoleLogger);
    }

    ~ConsoleLogger() {}

  private:
    virtual bool WriteLine(detail::LogLevel level, time_t now, bool put_header)
    {
      std::string header(put_header ? GetHeader(level, now) : "");

      switch (level)
      {
      case detail::LL_INFO:
      case detail::LL_DEBUG:
        std::cout << header;
        std::cout << stream().str() << std::endl;
        std::cout.flush();
        break;
      case detail::LL_ERROR:
      case detail::LL_WARN:
      default:
        std::cerr << header;
        std::cerr << stream().str() << std::endl;
        std::cerr.flush();
        break;
      }
      return true;
    }

    ConsoleLogger()
    {}
  }; // class ConsoleLogger

  extern ConsoleLogger::Ptr console_logger;
} // namespace nkit

#endif

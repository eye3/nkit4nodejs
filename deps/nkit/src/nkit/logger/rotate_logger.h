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

#ifndef __NKIT__ROTATE__LOGGER__H__
#define __NKIT__ROTATE__LOGGER__H__

#include <nkit/types.h>

#include <cstring>
#include <fstream>

namespace nkit
{
  enum RotateInterval
  {
    ROTATE_INTERVAL_DISABLED = 0, // rotate is not depends on time
    ROTATE_INTERVAL_1S = 1,       // one second (this is for tests)
    ROTATE_INTERVAL_2S = 2,       // two seconds (this is for tests)
    ROTATE_INTERVAL_5S = 5,       // five seconds (this is for tests)
    ROTATE_INTERVAL_1H = 60*60,                 // one hour
    ROTATE_INTERVAL_1D = ROTATE_INTERVAL_1H*24  // one day
  };

  enum RotateSize
  {
    ROTATE_SIZE_1K = 1024,
    ROTATE_SIZE_1M = ROTATE_SIZE_1K * ROTATE_SIZE_1K,
    ROTATE_SIZE_100M = 100 * ROTATE_SIZE_1M,
    ROTATE_SIZE_512M = 512 * ROTATE_SIZE_1M,
    ROTATE_SIZE_1G = ROTATE_SIZE_1K * ROTATE_SIZE_1M
  };

  class RotateLogger : public Logger
  {
    static const std::streamoff ROTATE_SIZE_DISABLED = -1;

  public:
    static Logger::Ptr Create(const std::string & file_path,
        RotateInterval rotate_interval = ROTATE_INTERVAL_DISABLED,
        std::streamoff rotate_size = ROTATE_SIZE_DISABLED,
        std::string * error = NULL);
    ~RotateLogger();

  private:
    virtual bool WriteLine(detail::LogLevel level, time_t now, bool);
    void UpdateRotateTimes();

    explicit RotateLogger(const std::string & file_path,
        RotateInterval rotate_interval, std::streamoff rotate_size);
    void Rotate(time_t now, size_t suffix);

  private:
    std::ofstream file_stream_;
    std::string file_path_;
    std::streamoff rotate_size_;
    RotateInterval rotate_interval_;
    time_t prev_rotate_time_;
    time_t next_rotate_time_;
    size_t rotate_by_size_counter_;
  }; // class RotateLogger
} // namespace nkit

#endif

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

#include <nkit/logger.h>
#include <nkit/logger/rotate_logger.h>

#include <nkit/detail/push_options.h>
#include <nkit/tools.h>
#include <nkit/dynamic.h>

#include <cstdio>
#include <iostream>

#if defined(NKIT_POSIX_PLATFORM)
#  include <unistd.h>
#  include <sys/types.h>
#elif defined(NKIT_WINNT)
#  pragma warning(disable : 4996)
#  include <windows.h>
#  include <Lmcons.h>
#endif

namespace nkit
{
  Logger::Ptr Logger::logger_;
  ConsoleLogger::Ptr console_logger = ConsoleLogger::Create();

  namespace detail
  {
    time_t time_offset()
    {
        time_t now = time(NULL);

        struct tm gm;
        GMTIME_R(now, &gm);
        time_t gmt = mktime(&gm);

        struct tm loc;
        LOCALTIME_R(now, &loc);
        time_t local = mktime(&loc);

        return static_cast<time_t>(difftime(local, gmt));
    }

    const std::string & get_log_level_name(detail::LogLevel level)
    {
      static const std::string __INFO("[Info]");
      static const std::string __WARNING("[Warning]");
      static const std::string __ERROR("[Error]");
      static const std::string __DEBUG("[Debug]");
      static const std::string __UNKNOWN("[?]");

      switch (level)
      {
        case detail::LL_DEBUG:
          return __DEBUG;
        case detail::LL_INFO:
          return __INFO;
        case detail::LL_WARN:
          return __WARNING;
        case detail::LL_ERROR:
          return __ERROR;
        default:
          return __UNKNOWN;
      }
    }
  } // namespace

  Logger::Ptr RotateLogger::Create(const std::string & file_path,
      RotateInterval rotate_interval, std::streamoff rotate_size,
      std::string * error)
  {
    // check file can be opened
    std::ofstream file_stream(file_path.c_str(), std::fstream::app);
    if (!file_stream.is_open())
    {
      if (error)
        *error = "RotateLogger cannot open file : " + file_path;
      return Logger::Ptr();
    }
    file_stream.close();

    return Logger::Ptr(new RotateLogger(file_path, rotate_interval,
        rotate_size));
  }

  RotateLogger::RotateLogger(const std::string & file_path,
      RotateInterval rotate_interval, std::streamoff rotate_size)
    : file_stream_(file_path.c_str(), std::fstream::app)
    , file_path_(file_path)
    , rotate_size_(rotate_size)
    , rotate_interval_(rotate_interval)
    , prev_rotate_time_(0)
    , next_rotate_time_(0)
    , rotate_by_size_counter_(-1)
  {
    UpdateRotateTimes();
  }

  RotateLogger::~RotateLogger()
  {
  }

  void print_time(time_t t)
  {
    struct tm ti;
    LOCALTIME_R(t, &ti);

    char iso_time[128];
    strftime(iso_time, sizeof(iso_time), "%Y-%m-%dT%H%M%S", &ti);
    std::cout << iso_time << std::endl;
  }

  void RotateLogger::UpdateRotateTimes()
  {
    if (rotate_interval_ == ROTATE_INTERVAL_DISABLED)
      return;

    if (!prev_rotate_time_)
    {
      Dynamic day_start = Dynamic::DateTimeLocal();
      day_start.SetHour(0);
      day_start.SetMinute(0);
      day_start.SetSecond(0);
      prev_rotate_time_ = day_start.timestamp();
    }
    else
    {
      prev_rotate_time_ = next_rotate_time_;
    }

    next_rotate_time_ = prev_rotate_time_ + rotate_interval_;
  }

  bool RotateLogger::WriteLine(detail::LogLevel level, time_t now, bool)
  {
    bool rotate_by_size = ((rotate_size_ != ROTATE_SIZE_DISABLED)
        && ((std::streamoff)file_stream_.tellp()) >= rotate_size_);

    bool rotate_by_time = (rotate_interval_ != ROTATE_INTERVAL_DISABLED)
        && now >= next_rotate_time_;

    if (rotate_by_time)
    {
      Rotate(prev_rotate_time_, ++rotate_by_size_counter_);
      UpdateRotateTimes();
      rotate_by_size_counter_ = -1;
    }
    else if (rotate_by_size)
    {
      if (rotate_interval_ == ROTATE_INTERVAL_DISABLED)
        Rotate(now, 0);
      else
        Rotate(prev_rotate_time_, ++rotate_by_size_counter_);
    }

    file_stream_ << GetHeader(level, now);
    file_stream_ << stream().str();
    file_stream_ << std::endl;
    file_stream_.flush();
    return true;
  }

  void RotateLogger::Rotate(time_t now, size_t suffix)
  {
    char iso_time[128];
    struct tm ti;
    LOCALTIME_R(now, &ti);

    file_stream_.close();

    std::string error;
    bool moved = false;
    if (unlikely(strftime(iso_time, sizeof(iso_time),
          "%Y-%m-%dT%H%M%S", &ti) == 0))
    {
      moved = move_file(
          file_path_, file_path_ + "." + string_cast((uint64_t)now), &error);
    }
    else
    {
      std::string ssuffix(suffix ? ("-" + string_cast(suffix)) : "");
      moved = move_file(
          file_path_, file_path_ + "." + iso_time + ssuffix, &error);
    }

    if (!moved)
      abort_with_core("Could not rotate log file: " + error);

    file_stream_.open(file_path_.c_str(), std::fstream::app);
    if (unlikely(!file_stream_.is_open()))
      abort_with_core("RotateLogger open file error : " + file_path_);
  }
} // namespace nkit


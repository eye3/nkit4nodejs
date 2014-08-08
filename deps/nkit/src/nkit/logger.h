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

#ifndef __NKIT__LOGGER__H__
#define __NKIT__LOGGER__H__

#include <ctime>
#include <sstream>

#include <nkit/tools.h>
#include <nkit/mutex.h>

namespace nkit
{
  namespace detail
  {
    //--------------------------------------------------------------------------
    enum LogLevel
    {
      LL_DEBUG = 0,
      LL_INFO,
      LL_WARN,
      LL_ERROR,
      LL_MAX
    }; // enum LogLevel

    //--------------------------------------------------------------------------
    const std::string & get_log_level_name(detail::LogLevel level);

  } // namespace detail

  //----------------------------------------------------------------------------
  class LoggerAccessor;

  class Logger
  {
    friend class LoggerAccessor;

    Logger(const Logger &);
    Logger & operator=(const Logger &);

  public:
    typedef NKIT_SHARED_PTR(Logger) Ptr;

  public:
    virtual ~Logger()
    {
      LockGuard<Mutex> guard(mutex_);
    }

    static bool Initialize(const Logger::Ptr & logger)
    {
      if (!logger)
        return false;
      logger_ = logger;
      return true;
    }

    static Logger * Instance()
    {
      return logger_.get();
    }

  protected:
    virtual bool WriteLine(detail::LogLevel level, time_t now,
        bool put_header) = 0;
    const std::ostringstream & stream() const { return stream_; }
    std::string GetHeader(detail::LogLevel level, time_t now) const
    {
      char tp_[128];
      CTIME_R(now, tp_, sizeof(tp_));
      const char * tp = (tp_ + 0) + 4;
      // Head : "day-time host user[pid]: level\t"
      std::string header;
      header.assign(tp, strlen(tp) - 1); // trim '\n'
      header.append(" ");
      header.append(host_);
      header.append(" ");
      header.append(user_);
      header.append(process_id_);
      header.append(": ");
      header.append(detail::get_log_level_name(level));
      header.append("\t ");
      return header;
    }

    Logger()
      : process_id_(get_process_id())
      , host_(get_hostname())
      , user_(get_username())
    {}

  private :
    static Logger::Ptr logger_;
    std::string const process_id_;
    std::string const host_;
    std::string const user_;
    std::ostringstream stream_;
    Mutex mutex_;
  }; // class Logger
} // namespace nkit

#include <nkit/logger/console_logger.h>
#include <nkit/logger/rotate_logger.h>
#include <nkit/logger/rsyslog_logger.h>

namespace nkit
{
  //----------------------------------------------------------------------------
  class LoggerAccessor
  {
  public:
    LoggerAccessor(detail::LogLevel level)
      : level_(level)
      , now_(0)
      , put_header_(false)
      , logger_(NULL)
    {
      ::std::time(&now_);
    }

    ~LoggerAccessor()
    {
      if (likely(logger_))
        EndLine();
    }

    LoggerAccessor & operator << (Logger & logger)
    {
      if (unlikely(logger_))
        EndLine();

      logger_ = &logger;
      put_header_ = (logger_ != console_logger.get());

      if (likely(logger_))
        BeginLine();

      return *this;
    }

    LoggerAccessor & operator << (Logger::Ptr logger)
    {
      return operator << (*logger);
    }

    LoggerAccessor & operator << (Logger * logger)
    {
      return operator << (*logger);
    }

    template <typename T>
    LoggerAccessor & operator << (const T & v)
    {
      if (unlikely(!logger_))
      {
        logger_ = Logger::Instance();
        if (!logger_)
        {
          logger_ = console_logger.get();
          put_header_ = true;
        }
        BeginLine();
      }

      logger_->stream_ << v;

      return *this;
    }

  private:
    LoggerAccessor & operator << (const Logger & logger);

    void BeginLine()
    {
      logger_->mutex_.Lock();
    }

    void EndLine()
    {
      logger_->WriteLine(level_, now_, put_header_);
      logger_->stream_.str("");
      logger_->mutex_.Unlock();
    }

  private:
    detail::LogLevel level_;
    time_t now_;
    bool put_header_;
    Logger * logger_;
  };

} // namespace nkit

//------------------------------------------------------------------------------
#define NKIT_LOG__(v, level)          \
    do                                \
    {                                 \
      nkit::LoggerAccessor a(level);  \
      a << v;                         \
    } while (0)                       \

#define NKIT_LOG_INFO(v)    NKIT_LOG__(v, nkit::detail::LL_INFO)
#define NKIT_LOG_WARNING(v) NKIT_LOG__(v, nkit::detail::LL_WARN)
#define NKIT_LOG_ERROR(v)   NKIT_LOG__(v, nkit::detail::LL_ERROR)

#ifdef NKIT_DEBUG
#define NKIT_LOG_DEBUG(v)   NKIT_LOG__(v, nkit::detail::LL_DEBUG)
#else
#define NKIT_LOG_DEBUG(v)
#endif

#endif

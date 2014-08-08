/*
   Copyright 2014 Vasiliy Soshnikov (dedok.mad@gmail.com)
                  Boris T. Darchiev (boris.darchiev@gmail.com)

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

#ifndef __NKIT__LOGGER__RSYSLOG__RFC3164__H__
#define __NKIT__LOGGER__RSYSLOG__RFC3164__H__

#if defined(NKIT_POSIX_PLATFORM)
# include <syslog.h>
#elif defined(NKIT_WINNT)
# define  LOG_EMERG 0 /* system is unusable */
# define  LOG_ALERT 1 /* action must be taken immediately */
# define  LOG_CRIT  2 /* critical conditions */
# define  LOG_ERR   3 /* error conditions */
# define  LOG_WARNING 4 /* warning conditions */
# define  LOG_NOTICE  5 /* normal but significant condition */
# define  LOG_INFO  6 /* informational */
# define  LOG_DEBUG 7 /* debug-level messages */
# define  LOG_FACMASK 0x03f8  /* mask to extract facility part */
# define  LOG_PRIMASK 0x07  /* mask to extract priority part (internal) */

/* facility codes */
# define  LOG_KERN  (0<<3)  /* kernel messages */
# define  LOG_USER  (1<<3)  /* random user-level messages */
# define  LOG_MAIL  (2<<3)  /* mail system */
# define  LOG_DAEMON  (3<<3)  /* system daemons */
# define  LOG_AUTH  (4<<3)  /* security/authorization messages */
# define  LOG_SYSLOG  (5<<3)  /* messages generated internally by syslogd */
# define  LOG_LPR   (6<<3)  /* line printer subsystem */
# define  LOG_NEWS  (7<<3)  /* network news subsystem */
# define  LOG_UUCP  (8<<3)  /* UUCP subsystem */
# define  LOG_CRON  (9<<3)  /* clock daemon */
# define  LOG_AUTHPRIV  (10<<3) /* security/authorization messages (private) */
# define  LOG_FTP   (11<<3) /* ftp daemon */

#endif
#include <sstream>
#include <nkit/tools.h>
#include <nkit/logger.h>

namespace nkit
{
  //----------------------------------------------------------------------------
  class RSysLoggerDefault : public Logger
  {
    public:
      struct Options
      {
        Options();

        Options(int facility_, const std::string & ident_)
          : facility(facility_)
          , ident(ident_)
        {
        }

        int facility;
        std::string ident;
      }; // struct Options

    public:
      static Logger::Ptr Create(const Options & opt);
      ~RSysLoggerDefault();

    private:
      explicit RSysLoggerDefault(const Options & opt);
      RSysLoggerDefault();

      virtual bool WriteLine(detail::LogLevel level, time_t, bool put_header);
  }; // class RSysLoggerDefault

  //----------------------------------------------------------------------------
  class RSysLoggerBase : public Logger
  {
  protected:
    enum { MAX_HEADER_SIZE = 235 };
    enum { CLOSED_SOCKET = -1 };

  public:
    enum { MAX_MESSAGE_SIZE = 2227 - MAX_HEADER_SIZE };

  protected:
    RSysLoggerBase(int facility, const std::string & ident);

    virtual ~RSysLoggerBase() { CloseConnection(true); }
    virtual bool Connect() = 0;
    void CloseConnection(bool graceful);
    virtual bool WriteLine(detail::LogLevel level, time_t, bool put_header);

  protected:
    std::string pid_;
    std::string tag_;
    int facility_;
    std::string ident_;
    int sockfd_;
    int fac_mask_;
  };

  //----------------------------------------------------------------------------
  class RSysUnixSocketLogger : public RSysLoggerBase
  {
  public:
    static Logger::Ptr Create(int facility, const std::string & ident,
        const std::string & unix_socket, std::string * error);

  private:
    RSysUnixSocketLogger(int facility, const std::string & ident,
        const std::string & unix_socket)
      : RSysLoggerBase(facility, ident)
      , unix_socket_(unix_socket)
    {}

    virtual bool Connect();

  private:
    std::string unix_socket_;
  };

  //----------------------------------------------------------------------------
  class RSysEndpointLogger : public RSysLoggerBase
  {
  public:
    static Logger::Ptr Create(int facility, const std::string & ident,
        const std::string & host, const std::string & port, std::string * error);

  private:
    RSysEndpointLogger(int facility, const std::string & ident,
        const std::string & host, const std::string & port)
      : RSysLoggerBase(facility, ident)
      , host_(host)
      , port_(port)
    {}

    virtual bool Connect();

  private:
    std::string host_;
    std::string port_;
  };
} // namespace nkit

#endif


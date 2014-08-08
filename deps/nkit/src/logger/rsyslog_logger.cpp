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

#include "nkit/types.h"

#if defined(NKIT_WINNT)
#  pragma warning(disable : 4996)
# define MSG_NOSIGNAL 0
#else
# define INVALID_SOCKET -1
# define SOCKET_ERROR -1
# define closesocket close
# define SYSLOG_NAMES 1
# include <syslog.h>
# include <errno.h>
# include <sys/socket.h>
# include <sys/un.h>
# include <netdb.h>
#endif

#include "nkit/logger.h"
#include "nkit/logger/rsyslog_logger.h"

namespace nkit
{
  const char * get_level(detail::LogLevel level, int * prior)
  {
    switch (level)
    {
      case detail::LL_INFO:
        *prior = LOG_INFO;
        return "Info";
      case detail::LL_DEBUG:
        *prior = LOG_DEBUG;
        return "Debug";
        break;
      case detail::LL_WARN:
        *prior = LOG_WARNING;
        return "Warning";
      case detail::LL_ERROR:
        *prior = LOG_ERR;
        return "Error";
      default:
        *prior = LOG_ERR;
        return "?";
    }
  }

#if defined(NKIT_POSIX_PLATFORM)
  //-----------------------------------------------------------------------------
  // RSysLoggerDefault
  //-----------------------------------------------------------------------------
  RSysLoggerDefault::Options::Options()
    : facility(LOG_USER)
    , ident("?")
  {
  }

  Logger::Ptr RSysLoggerDefault::Create(const Options & opt)
  {
    return Logger::Ptr(new RSysLoggerDefault(opt));
  }

  RSysLoggerDefault::~RSysLoggerDefault()
  {
    closelog();
  }

  RSysLoggerDefault::RSysLoggerDefault(const Options & opt)
  {
    openlog(opt.ident.c_str(), (LOG_PID | LOG_CONS), opt.facility);
  }

  bool RSysLoggerDefault::WriteLine(detail::LogLevel level, time_t, bool)
  {
    int prior;
    const char * message_level = get_level(level, &prior);

    std::string message = stream().str();
    syslog(prior, "[%s] %s", message_level, message.c_str());

    return true;
  }
#endif
  //-----------------------------------------------------------------------------
  // RSysLoggerBase
  //-----------------------------------------------------------------------------
  enum
  {
    TYPE_UDP = (1 << 1),
    TYPE_TCP = (1 << 2),
    ALL_TYPES = TYPE_UDP | TYPE_TCP
  };

  static const int socket_type__ = ALL_TYPES;

  //-----------------------------------------------------------------------------
  RSysLoggerBase::RSysLoggerBase(int facility, const std::string & ident)
    : pid_(get_process_id())
    , tag_(ident)
    , facility_(facility)
    , ident_(ident)
    , sockfd_(CLOSED_SOCKET)
    , fac_mask_(facility & LOG_FACMASK)
  {
    std::string login = get_username();
    if (login.empty())
    {
      switch (errno)
      {
        case EIO:
        case EMFILE:
        case ENFILE:
        case ENOMEM:
        case ERANGE:
          ::nkit::abort_with_core(__PRETTY_FUNCTION__ + std::string("():") +
              std::string(strerror(errno)));
          break;
        default:
          errno = 0;
          break;
      }
    }
    else
      tag_ = login + " " + tag_;
  }

  void RSysLoggerBase::CloseConnection(bool graceful)
  {
    if (sockfd_ != CLOSED_SOCKET)
    {
      if (graceful && shutdown(sockfd_, SHUT_RDWR) == -1)
      {
        std::cerr << std::string(strerror(errno)) << std::endl;
        std::cerr.flush();
        errno = 0;
      }
      closesocket(sockfd_);
      sockfd_ = CLOSED_SOCKET;
    }
  }

  bool RSysLoggerBase::WriteLine(detail::LogLevel level, time_t now, bool)
  {
    char buf[MAX_HEADER_SIZE + MAX_MESSAGE_SIZE];
    std::string msg = stream().str();
    int prior;
    const char * message_level = get_level(level, &prior);
    if (level == detail::LL_DEBUG)
      prior = LOG_INFO;
    int syslog_pri = prior & LOG_PRIMASK;
    /*
    switch (level)
    {
    case detail::LL_DEBUG:
    case detail::LL_INFO:
      syslog_pri = fac_mask_ | (LOG_INFO & LOG_PRIMASK);
      break;
    case detail::LL_WARN:
      syslog_pri = fac_mask_ | (LOG_WARNING & LOG_PRIMASK);
      break;
    default:
      syslog_pri = fac_mask_ | (LOG_ERR & LOG_PRIMASK);
      break;
    }
    */
    static const uint16_t CTIME_BUF_LEN = 32;
    char ctime_buf[CTIME_BUF_LEN];
    CTIME_R(now, ctime_buf, CTIME_BUF_LEN);
    int len = NKIT_SNPRINTF(buf, sizeof(buf),
        "<%d>%.15s %.200s%s: [%s] %.1992s",
        syslog_pri, ctime_buf + 4, tag_.c_str(), pid_.c_str(), message_level,
        msg.c_str());

    if (unlikely(sockfd_ == -1))
      if (!Connect())
        return false;

    if (likely(sockfd_ != -1))
    {
      int sent = 0;
      while (sent < len)
      {
#ifdef __APPLE__
        sent = sent + send(sockfd_, buf + sent, len, SO_NOSIGPIPE);
#else
        sent = sent + send(sockfd_, buf + sent, len, MSG_NOSIGNAL);
#endif
        if (unlikely(sent == -1 && errno != 0))
        {
          fprintf(stderr, "%s\n", buf);
          CloseConnection(true);
        }
        else if (sent == 0)
          break;
        len = len - sent;
      }
    }
    else
    {
      switch (level)
      {
      case detail::LL_DEBUG:
      case detail::LL_INFO:
        std::cout << buf << std::endl;
        std::cout.flush();
        break;
      default:
        std::cerr << buf << std::endl;
        std::cerr.flush();
        break;
      }
    }

    return true;
  }

#if defined(NKIT_POSIX_PLATFORM)
  //-----------------------------------------------------------------------------
  // RSysUnixSocketLogger
  //-----------------------------------------------------------------------------
  Logger::Ptr RSysUnixSocketLogger::Create(int facility,
      const std::string & ident, const std::string & unix_socket,
      std::string * error)
  {
    if (unix_socket.empty())
    {
      *error = "'unix_socket' parameter could not be empty";
      return Logger::Ptr();
    }

    struct sockaddr_un unused;
    if (unix_socket.size() >= sizeof(unused.sun_path))
    {
      *error = "'" + unix_socket + "': pathname too long";
      return Logger::Ptr();
    }

    return Logger::Ptr(
        new RSysUnixSocketLogger(facility, ident, unix_socket));
  }

  //----------------------------------------------------------------------------
  bool RSysUnixSocketLogger::Connect()
  {
    int st = -1;
    short step = 2;
    struct sockaddr_un s_addr;

    s_addr.sun_family = AF_UNIX;
    strcpy(s_addr.sun_path, unix_socket_.c_str());

    for (step = 2; step; step--, st = -1)
    {
      if (step == 2 && (socket_type__ & TYPE_UDP))
        st = SOCK_DGRAM;
      if (step == 1 && (socket_type__ & TYPE_TCP))
        st = SOCK_STREAM;
      if (st == -1 || (sockfd_ = socket(AF_UNIX, st, 0)) == INVALID_SOCKET)
        continue;

      if (connect(sockfd_, (struct sockaddr *) &s_addr, sizeof(s_addr)) ==
        SOCKET_ERROR)
      {
        CloseConnection(false);
        continue;
      }

      break;
    }

    if (step == 0)
      sockfd_ = -1;

    errno = 0;
    return true;
  }
#endif

  //-----------------------------------------------------------------------------
  // RSysEndpointLogger
  //-----------------------------------------------------------------------------
  Logger::Ptr RSysEndpointLogger::Create(int facility, const std::string & ident,
      const std::string & host, const std::string & port, std::string * error)
  {
    if (host.empty() || port.empty())
    {
      *error = "'host' & 'port' parameters could not be empty";
      return Logger::Ptr();
    }

    return Logger::Ptr(new RSysEndpointLogger(facility, ident, host, port));
  }

  //----------------------------------------------------------------------------
  bool RSysEndpointLogger::Connect()
  {
    short step = 2;
    struct addrinfo hints, *_addrinfo;

    for (; step; step--)
    {
      memset(&hints, 0, sizeof(hints));
      if (step == 2 && (socket_type__ & TYPE_UDP))
        hints.ai_socktype = SOCK_DGRAM;
      if (step == 1 && (socket_type__ & TYPE_TCP))
        hints.ai_socktype = SOCK_STREAM;
      if (hints.ai_socktype == 0)
        continue;
      hints.ai_family = AF_UNSPEC;
#if defined (NKIT_WINNT)
      int ret = getaddrinfo(host_.c_str(), port_.c_str(), &hints, &_addrinfo);
      if (ret != 0)
      {
        std::cerr << "Could not resolve " << host_ << ":" <<
            port_ << ", code = " << ret << std::endl;
        std::cerr.flush();
        return false;
      }
#else
      if (getaddrinfo(host_.c_str(), port_.c_str(), &hints, &_addrinfo) == -1)
      {
        std::cerr << "Could not resolve " << host_ << ":" <<
            port_ << strerror(errno) << std::endl;
        std::cerr.flush();
        errno = 0;
        return false;
      }
#endif
      sockfd_ = socket(_addrinfo->ai_family, _addrinfo->ai_socktype,
          _addrinfo->ai_protocol);
      if (sockfd_ == INVALID_SOCKET)
      {
          freeaddrinfo(_addrinfo);
          continue;
      }

      if (connect(sockfd_, _addrinfo->ai_addr, _addrinfo->ai_addrlen) ==
        SOCKET_ERROR)
      {
          freeaddrinfo(_addrinfo);
          CloseConnection(false);
          continue;
      }

      freeaddrinfo(_addrinfo);
      break;
    }

    if (step == 0)
      sockfd_ = -1;

    errno = 0;
    return true;
  }
} // namespace nkit

#undef SYSLOG_NAMES

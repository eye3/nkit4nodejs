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

#include <iostream>
#include <iomanip>

#include "nkit/test.h"
#include <nkit/version.h>
#include <nkit/logger.h>
#include <nkit/tools.h>

static std::string big_string;

void write_default(size_t steps = 50, bool sleep_ = false)
{
  for (size_t i = 0; i < steps; ++i)
  {
    if (sleep_ && (i == steps / 2))
      nkit::sleep(1000);

    NKIT_LOG_INFO("const string " << i << " "<< 0.10 << big_string);

    NKIT_LOG_ERROR(
        "const string " << i << " "<< 0.10 << big_string);

    NKIT_LOG_WARNING(
        "const string " << i << " " << 0.10 << big_string);

    NKIT_LOG_DEBUG(
        "const string " << i  << " " << 0.10 << big_string);
  }
}

void write_big_strings_only(size_t steps = 50, bool sleep_ = false)
{
  for (size_t i = 0; i < steps; ++i)
  {
    if (sleep_ && (i == steps / 2))
      nkit::sleep(1000);

    NKIT_LOG_INFO(big_string);

    NKIT_LOG_ERROR(big_string);

    NKIT_LOG_WARNING(big_string);

    NKIT_LOG_DEBUG(big_string);
  }
}

void * second_thread(void *)
{
  nkit::sleep(500);
  write_default(1000, false);
  return NULL;
}

_NKIT_TEST_CASE(TestRSysLog)
{
  std::string error;
  big_string.resize(nkit::RSysLoggerBase::MAX_MESSAGE_SIZE);
  std::fill(big_string.begin(), big_string.end(), '~');
  big_string[nkit::RSysLoggerBase::MAX_MESSAGE_SIZE-1] = '!';
#if defined(NKIT_POSIX_PLATFORM) && 0
  nkit::RSysLoggerDefault::Options rsdopt;
  rsdopt.ident = "nkit-test";
  rsdopt.facility = LOG_MAIL;
  nkit::Logger::Initialize(nkit::RSysLoggerDefault::Create(rsdopt));
  write_default();
#endif
  nkit::Logger::Ptr logger = nkit::RSysEndpointLogger::Create(
            LOG_DAEMON, "nkit-test", "search1.dv.rbc.ru", "514", &error);

  NKIT_TEST_ASSERT_WITH_TEXT(logger, error);
  nkit::Logger::Initialize(logger);

  write_big_strings_only(1);
  /*pthread_t t;
  pthread_create(&t, NULL, second_thread, NULL);
  write_default(1000, true);
  pthread_join(t, NULL);*/
}

_NKIT_TEST_CASE(TestMainRotateLoggerEveryDay)
{
  using namespace nkit;

  big_string.resize(1024);
  std::fill(big_string.begin(), big_string.end(), '~');

  std::string error;
  nkit::Logger::Ptr logger = nkit::RotateLogger::Create("./main.log",
      ROTATE_INTERVAL_1D, 30 * 1024 * 1024, &error);
  NKIT_TEST_ASSERT_WITH_TEXT(logger, error);
  nkit::Logger::Initialize(logger);
  write_default(1000, true);
  NKIT_TEST_ASSERT_WITH_TEXT(false, "qe qw q qwe q");
}

_NKIT_TEST_CASE(TestMainRotateLogger)
{
  using namespace nkit;

  big_string.resize(1024);
  std::fill(big_string.begin(), big_string.end(), '~');

  std::string error;
  nkit::Logger::Ptr logger = nkit::RotateLogger::Create("./main.log",
      ROTATE_INTERVAL_1S, 30 * 1024 * 1024, &error);
  NKIT_TEST_ASSERT_WITH_TEXT(logger, error);
  nkit::Logger::Initialize(logger);
  write_default(1000, true);

#if defined(NKIT_POSIX_PLATFORM)
  pthread_t t;
  pthread_create(&t, NULL, second_thread, NULL);
  write_default(1000, true);
  pthread_join(t, NULL);
#endif
}

_NKIT_TEST_CASE(TestCustomRotateLogger)
{
  using namespace nkit;

  big_string.resize(1024);
  std::fill(big_string.begin(), big_string.end(), '~');

  std::string error;
  nkit::Logger::Ptr logger = nkit::RotateLogger::Create("./custom.log",
      ROTATE_INTERVAL_5S, 3 * 1024 * 1024, &error);
  NKIT_TEST_ASSERT_WITH_TEXT(logger, error);

  for (size_t i = 1; i < 7000; ++i)
  {
    if (i % 500 == 0)
      nkit::sleep(1000);

    NKIT_LOG_INFO(
        logger << "const string " << i << 0.10 << big_string);

    NKIT_LOG_ERROR(
        logger << "const string " << i << 0.10 << big_string);

    NKIT_LOG_WARNING(
        logger << "const string " << i << 0.10 << big_string);

    NKIT_LOG_DEBUG(
        logger << "const string " << i << 0.10 << big_string);
  }
}

_NKIT_TEST_CASE(TestComboRotateLogger)
{
  using namespace nkit;

  big_string.resize(1024);
  std::fill(big_string.begin(), big_string.end(), '~');

  std::string error;
  nkit::Logger::Ptr logger = nkit::RotateLogger::Create("./combo_main.log",
      ROTATE_INTERVAL_5S, 3 * 1024 * 1024, &error);
  NKIT_TEST_ASSERT_WITH_TEXT(logger, error);
  nkit::Logger::Initialize(logger);

  logger = nkit::RotateLogger::Create("./combo_custom.log",
      ROTATE_INTERVAL_5S,
      3 * 1024 * 1024, &error);
  NKIT_TEST_ASSERT_WITH_TEXT(logger, error);

  for (size_t i = 1; i < 7000; ++i)
  {
    if (i % 500 == 0)
      nkit::sleep(1000);

    NKIT_LOG_INFO("const string " << i << 0.10 << big_string
        << logger << "const string " << i << 0.10 << big_string);

    NKIT_LOG_ERROR("const string " << i << 0.10 << big_string
        << logger << "const string " << i << 0.10 << big_string);

    NKIT_LOG_WARNING("const string " << i << 0.10 << big_string
        << logger << "const string " << i << 0.10 << big_string);

    NKIT_LOG_DEBUG("const string " << i << 0.10 << big_string
        << logger << "const string " << i << 0.10 << big_string);
  }
}

_NKIT_TEST_CASE(TestConsoleLogger)
{
  using namespace nkit;

  nkit::Logger::Ptr logger = nkit::ConsoleLogger::Create();
  double d = 0.123456789;

  for (size_t i = 1; i < 10; ++i)
  {
    NKIT_LOG_INFO(
        logger << "const string" << " "
          << i << " "
          << std::setprecision(9) << d);

    NKIT_LOG_DEBUG(
        logger << "const string" << " "
          << i << " "
          << d);

    NKIT_LOG_ERROR(
        logger << "const string" << " "
          << i << " "
          << d);

    NKIT_LOG_WARNING(
        logger << "const string" << " "
          << i << " "
          << d);
  }
}

int main(int NKIT_UNUSED(argc), char ** NKIT_UNUSED(argv))
{
#ifdef _WIN32
#ifdef _DEBUG
  int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag(tmpFlag);
#endif
#endif

#ifdef NKIT_WINNT
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }

/* Confirm that the WinSock DLL supports 2.2.*/
/* Note that if the DLL supports versions greater    */
/* than 2.2 in addition to 2.2, it will still return */
/* 2.2 in wVersion since that is the version we      */
/* requested.                                        */

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return 1;
    }
    else
        printf("The Winsock 2.2 dll was found okay\n");
#endif

    int ret = nkit::test::run_all_tests();

#ifdef NKIT_WINNT
    WSACleanup();
#endif

    return ret;
}

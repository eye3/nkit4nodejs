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

#include "nkit/test.h"
#include "nkit/dynamic_json.h"
#include "nkit/detail/config.h"
#include "nkit/dynamic_getter.h"
#include "nkit/logger_brief.h"
#include "nkit/version.h"

namespace nkit_test
{
  using namespace nkit;

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(DynamicDateTimeFromTimestamp)
  {
    Dynamic dt = Dynamic::DateTimeFromTimestamp(1361873780);
    dt.SetSecond(0);
    dt.SetMinute(0);
    std::cout << dt.hours() << '\n';
    std::cout << dt.GetString() << '\n';
    uint64_t i = 566658777940492288ll;
    uint64_t k = dt.GetUnsignedInteger();
    std::cout << i << '\n';
    std::cout << k << '\n';
  }

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(DynamicDateTimeFromTm)
  {
    struct tm t;
    memset(&t, 0, sizeof(t));
    t.tm_mday = 1;
    t.tm_hour = 1;
    t.tm_min = 2;
    t.tm_sec = 3;
    Dynamic dt = Dynamic::DateTimeFromTm(t);
    NKIT_TEST_ASSERT(dt.IsDateTime());
    NKIT_TEST_ASSERT(dt.GetString("%H:%M:%S") == "01:02:03");
  }

  //----------------------------------------------------------------------------
  _NKIT_TEST_CASE(DynamicDateTimeFromString)
  {
    std::string etalon("2014/01/01 12:13:14");
    const char * FORMAT = "%Y/%m/%d %H:%M:%S";
    Dynamic dt = Dynamic::DateTimeFromString(etalon, FORMAT);
    NKIT_TEST_ASSERT(dt.IsDateTime());
    NKIT_TEST_ASSERT(dt.GetString(FORMAT) == etalon);

    etalon = "Fri, 22 Aug 2014 13:59:06 +0400";
    FORMAT = "%a, %d %b %Y %H:%M:%S %z";
    dt = Dynamic::DateTimeFromString(etalon, FORMAT);
    NKIT_TEST_ASSERT(dt.IsDateTime());
    NKIT_TEST_ASSERT_WITH_TEXT(dt.GetString(FORMAT) == etalon,
    		dt.GetString(FORMAT));

    std::string wrong_string("2014/01/01 12-13-14");
    dt = Dynamic::DateTimeFromString(wrong_string, FORMAT);
    NKIT_TEST_ASSERT(!dt.IsDateTime());
  }

  //----------------------------------------------------------------------------
  inline bool operator ==(const Dynamic & dt, const struct tm * _tm)
  {
    uint32_t year = static_cast<uint32_t>(_tm->tm_year + 1900);
    uint32_t month = static_cast<uint32_t>(_tm->tm_mon + 1);
    uint32_t day = static_cast<uint32_t>(_tm->tm_mday);
    uint32_t hour = static_cast<uint32_t>(_tm->tm_hour);
    uint32_t min = static_cast<uint32_t>(_tm->tm_min);
    uint32_t sec = static_cast<uint32_t>(_tm->tm_sec);

    return (year == dt.year())
        && (month == dt.month())
        && (day == dt.day())
        // it is expected, that this test is running at daylight time
        && (hour == dt.hours() || (hour - 1) == dt.hours())
        && (min == dt.minutes() || (min-1) == dt.minutes()
            || (dt.minutes() == 0 && min == 59))
        && (sec == dt.seconds() || (sec-1) == dt.seconds()
            || (dt.seconds() == 0 && sec == 59));
  }

  inline bool operator ==(const struct tm * _tm, const Dynamic & dt)
  {
    return dt == _tm;
  }

  inline bool operator ==(const struct tm & _tm, const Dynamic & dt)
  {
    return dt == &_tm;
  }

  inline bool operator ==(const Dynamic & dt, const struct tm & _tm)
  {
    return dt == &_tm;
  }

  NKIT_TEST_CASE(DynamicLocalDateTime)
  {
    time_t time_check;
    time(&time_check);
    struct tm tm_check;
    LOCALTIME_R(time_check, &tm_check);
    Dynamic dt = Dynamic::DateTimeLocal();
    CINFO(tm_check);
    CINFO(dt);
    NKIT_TEST_ASSERT(dt == tm_check);
  }

  NKIT_TEST_CASE(DynamicGmtDateTime)
  {
    time_t time_check;
    time(&time_check);
    struct tm tm_check;
    GMTIME_R(time_check, &tm_check);
    Dynamic dt = Dynamic::DateTimeGmt();
    CINFO(&tm_check);
    CINFO(dt);
    NKIT_TEST_ASSERT(dt == tm_check);
  }

  NKIT_TEST_CASE(DynamicDateAddDays)
  {
    Dynamic dt(2012, 7, 8, 0, 0, 1);
    //const std::string GMT_FORMAT("%a, %d %b %Y %H:%M:%S GMT");
    std::cout << dt.GetString(GMT_FORMAT_) << '\n';
    dt.AddDays(-32);
    Dynamic etalon(2012, 6, 6, 0, 0, 1);
    std::cout << dt.GetString(GMT_FORMAT_) << '\n';
    std::cout << etalon.GetString(GMT_FORMAT_) << std::endl;
    NKIT_TEST_ASSERT(etalon == dt);
  }

  NKIT_TEST_CASE(DynamicDateAddSeconds)
  {
    Dynamic dt(2012, 7, 8, 0, 0, 1);

    dt.AddSeconds(60);
    Dynamic etalon(2012, 7, 8, 0, 1, 1);
    NKIT_TEST_ASSERT(etalon == dt);

    dt.AddSeconds(30);
    etalon = Dynamic(2012, 7, 8, 0, 1, 31);
    NKIT_TEST_ASSERT(etalon == dt);
  }

  NKIT_TEST_CASE(DynamicDateAddMinutes)
  {
    Dynamic dt(2012, 12, 31, 23, 0, 1);

    dt.AddMinutes(30);
    Dynamic etalon(2012, 12, 31, 23, 30, 1);
    NKIT_TEST_ASSERT(etalon == dt);

    dt.AddMinutes(30);
    etalon = Dynamic(2013, 1, 1, 0, 0, 1);
    NKIT_TEST_ASSERT(etalon == dt);

    dt.AddMinutes(-30);
    etalon = Dynamic(2012, 12, 31, 23, 30, 1);
    NKIT_TEST_ASSERT(etalon == dt);
  }

  NKIT_TEST_CASE(DynamicDateAddHours)
  {
    Dynamic dt(2012, 12, 30, 0, 0, 1);

    dt.AddHours(24);
    Dynamic etalon(2012, 12, 31, 0, 0, 1);
    NKIT_TEST_ASSERT(etalon == dt);

    dt.AddHours(12);
    etalon = Dynamic(2012, 12, 31, 12, 0, 1);
    NKIT_TEST_ASSERT(etalon == dt);

    dt.AddHours(12);
    etalon = Dynamic(2013, 1, 1, 0, 0, 1);
    NKIT_TEST_ASSERT(etalon == dt);

    dt.AddHours(-12);
    etalon = Dynamic(2012, 12, 31, 12, 0, 1);
    NKIT_TEST_ASSERT(etalon == dt);
  }

  NKIT_TEST_CASE(DynamicDateSetSecond)
  {
    Dynamic dt(2012, 7, 8, 0, 0, 1);
    //const std::string GMT_FORMAT("%a, %d %b %Y %H:%M:%S GMT");
    //std::cout << dt.GetString(GMT_FORMAT.c_str()) << '\n';
    dt.SetSecond(12);
    Dynamic etalon(2012, 7, 8, 0, 0, 12);
    //std::cout << dt.GetString(GMT_FORMAT.c_str()) << '\n';
    NKIT_TEST_ASSERT(etalon == dt);
    NKIT_TEST_ASSERT(!dt.SetSecond(422));
    NKIT_TEST_ASSERT(etalon == dt);
  }

  NKIT_TEST_CASE(DynamicDateSetMinute)
  {
    Dynamic dt(2012, 7, 8, 0, 10, 1);
    dt.SetMinute(12);
    Dynamic etalon(2012, 7, 8, 0, 12, 1);
    NKIT_TEST_ASSERT(etalon == dt);
    NKIT_TEST_ASSERT(!dt.SetMinute(432));
    NKIT_TEST_ASSERT(etalon == dt);
  }

  NKIT_TEST_CASE(DynamicDateSetHour)
  {
    Dynamic dt(2012, 7, 8, 0, 10, 1);
    dt.SetHour(12);
    Dynamic etalon(2012, 7, 8, 12, 10, 1);
    NKIT_TEST_ASSERT(etalon == dt);
    NKIT_TEST_ASSERT(!dt.SetHour(42));
    NKIT_TEST_ASSERT(etalon == dt);
  }

  NKIT_TEST_CASE(DynamicDateTimeDefault)
  {
    std::string error;
    std::string str("2012-07-08 00:12:33");
    Dynamic dt(Dynamic::DateTimeFromDefault(str, &error));
    Dynamic etalon(2012, 7, 8, 0, 12, 33);
    NKIT_TEST_ASSERT(etalon == dt);
  }

  NKIT_TEST_CASE(DynamicDateTimeISO8601)
  {
    std::string error;
    Dynamic etalon(2012, 7, 8, 0, 12, 33);

    std::string str("2012-07-08T00:12:33");
    Dynamic dt(Dynamic::DateTimeFromISO8601(str, &error));
    NKIT_TEST_ASSERT(etalon == dt);

    str = "20120708T001233";
    dt = Dynamic::DateTimeFromISO8601(str, &error);
    NKIT_TEST_ASSERT(etalon == dt);
  }

  NKIT_TEST_CASE(DynamicDateTimeWrongValues)
  {
    Dynamic dt(2014, 2, 28, 0, 12, 33);
    NKIT_TEST_ASSERT(dt.IsDateTime());

    dt = Dynamic(2014, 2, 28, 23, 59, 59);
    NKIT_TEST_ASSERT(dt.IsDateTime());

    dt = Dynamic(2014, 2, 29, 0, 12, 33);
    NKIT_TEST_ASSERT(dt.IsUndef());

    dt = Dynamic(2014, 2, 28, 0, 12, 60);
    NKIT_TEST_ASSERT(dt.IsUndef());

    dt = Dynamic(2014, 2, 28, 0, 60, 0);
    NKIT_TEST_ASSERT(dt.IsUndef());

    dt = Dynamic(2014, 2, 28, 24, 0, 0);
    NKIT_TEST_ASSERT(dt.IsUndef());
  }

  NKIT_TEST_CASE(DynamicDateTime_TimeZoneOffset)
  {
    CINFO(timezone_offset());
  }
}

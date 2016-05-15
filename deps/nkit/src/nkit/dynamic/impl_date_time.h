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

#include <nkit/detail/push_options.h>

#include <ctime>
#include <cstring>

namespace nkit
{
  namespace detail
  {
    template <>
    class Impl<detail::DATE_TIME> : public ImplDefault
    {
      static const uint64_t MAX_VAL = 0xFFFFFFFF;

      static const uint64_t TOTAL_SIZE = sizeof(uint64_t) * 8;

      static const uint64_t YEAR_SIZE = 16;
      static const uint64_t YEAR_SHIFT = TOTAL_SIZE - YEAR_SIZE;
      static const uint64_t YEAR_POS = 0;
      static const uint64_t YEAR_MASK = (MAX_VAL << (TOTAL_SIZE - YEAR_SIZE))
          >> YEAR_POS;
      static const uint64_t YEAR_MAX = (MAX_VAL << (TOTAL_SIZE - YEAR_SIZE))
          >> (TOTAL_SIZE - YEAR_SIZE);

      static const uint64_t MONTH_SIZE = 4;
      static const uint64_t MONTH_SHIFT = YEAR_SHIFT - MONTH_SIZE;
      static const uint64_t MONTH_POS = YEAR_POS + YEAR_SIZE;
      static const uint64_t MONTH_MASK = (MAX_VAL << (TOTAL_SIZE - MONTH_SIZE))
          >> MONTH_POS;

      static const uint64_t DAY_SIZE = 5;
      static const uint64_t DAY_SHIFT = MONTH_SHIFT - DAY_SIZE;
      static const uint64_t DAY_POS = MONTH_POS + MONTH_SIZE;
      static const uint64_t DAY_MASK = (MAX_VAL << (TOTAL_SIZE - DAY_SIZE))
          >> DAY_POS;

      static const uint64_t HH_SIZE = 5;
      static const uint64_t HH_SHIFT = DAY_SHIFT - HH_SIZE;
      static const uint64_t HH_POS = DAY_POS + DAY_SIZE;
      static const uint64_t HH_MASK = (MAX_VAL << (TOTAL_SIZE - HH_SIZE))
          >> HH_POS;

      static const uint64_t MM_SIZE = 6;
      static const uint64_t MM_SHIFT = HH_SHIFT - MM_SIZE;
      static const uint64_t MM_POS = HH_POS + HH_SIZE;
      static const uint64_t MM_MASK = (MAX_VAL << (TOTAL_SIZE - MM_SIZE))
          >> MM_POS;

      static const uint64_t SS_SIZE = 6;
      static const uint64_t SS_SHIFT = MM_SHIFT - SS_SIZE;
      static const uint64_t SS_POS = MM_POS + MM_SIZE;
      static const uint64_t SS_MASK = (MAX_VAL << (TOTAL_SIZE - SS_SIZE))
          >> SS_POS;

      static const uint64_t MICROSEC_SIZE = 20;
      static const uint64_t MICROSEC_SHIFT = SS_SHIFT - MICROSEC_SIZE;
      static const uint64_t MICROSEC_POS = SS_POS + SS_SIZE;
      static const uint64_t MICROSEC_MASK = (MAX_VAL
          << (TOTAL_SIZE - MICROSEC_SIZE)) >> MICROSEC_POS;
      static const uint64_t MICROSEC_MAX = 999999;

    public:
      static void CreateLocal(Dynamic & v)
      {
        uint64_t d(0);
        time_t now;
        ::std::time(&now);
        struct tm _tm;
        LOCALTIME_R(now, &_tm);
        Set(_tm, &d);
        Reset(&v, d);
      }

      static void CreateGmt(Dynamic & v)
      {
        uint64_t d(0);
        time_t now;
        ::std::time(&now);
        struct tm _tm;
        GMTIME_R(now, &_tm);
        Set(_tm, &d);
        Reset(&v, d);
      }

      static void Create(Dynamic & v, const time_t now)
      {
        uint64_t d(0); NKIT_FORCE_USED(d);
        struct tm _tm;
        LOCALTIME_R(now, &_tm);
        Set(_tm, &d);
        Reset(&v, d);
      }

      static void Create(Dynamic & v,
          const uint64_t year, const uint64_t mon, const uint64_t dd,
          const uint64_t hh, const uint64_t mm, const uint64_t ss,
          const uint64_t microsec)
      {
        uint64_t d(0);
        if (!Set(year, mon, dd, hh, mm, ss, microsec, &d))
          v.Reset();
        else
          Reset(&v, d);
      }

      static void OP_CLEAR(Dynamic & v) { v.Reset(); }

      static Dynamic OP_CLONE(const Data & v)
      {
        Dynamic result;
        Reset(&result, v.ui64_);
        return result;
      }

      static void OP_DEC_REF_DYNAMIC(Dynamic & ) {}
      static void OP_INC_REF_DATA(Data & ) {}
      static void OP_DEC_REF_DATA(Data & ) {}

      static int64_t OP_GET_INT(const Data & v)
      {
        return v.i64_;
      }

      static uint64_t OP_GET_UINT(const Data & v)
      {
        return v.ui64_;
      }

      static bool OP_GET_BOOL(const Data & v)
      {
        return v.ui64_ != 0;
      }

      static bool OP_IS_EMPTY(const Data & NKIT_UNUSED(v))
      {
        return false;
      }

      static double GET_DOUBLE(const Data & v)
      {
        return (double) v.i64_;
      }

      static std::string OP_GET_STRING(const Data & v, const char * format)
      {
        struct tm timeinfo;
        memset(&timeinfo, 0, sizeof(timeinfo));

        timeinfo.tm_year = year(v) - 1900;
        timeinfo.tm_mon = month(v) - 1;
        timeinfo.tm_mday = day(v);
        timeinfo.tm_hour = hours(v);
        timeinfo.tm_min = minutes(v);
        timeinfo.tm_sec = seconds(v);
        timeinfo.tm_isdst = -1;
        std::mktime(&timeinfo);

        if (*format == '\0')
          format = DATE_TIME_DEFAULT_FORMAT();

        const int BUFFER_SIZE(1024);
        char buffer [BUFFER_SIZE];

        size_t written = std::strftime(buffer, BUFFER_SIZE, format, &timeinfo);
        if (written == 0)
          return S_EMPTY_;
        return std::string(buffer, written);
      }

      static std::string OP_GET_STRING(const Dynamic & v, const char * format)
      {
        return OP_GET_STRING(v.data_, format);
      }

      static uint32_t year(const Data & v)
      {
        return uint32_t((v.ui64_ & YEAR_MASK) >> YEAR_SHIFT);
      }

      static uint32_t month(const Data & v)
      {
        return uint32_t((v.ui64_ & MONTH_MASK) >> MONTH_SHIFT);
      }

      static uint32_t day(const Data & v)
      {
        return uint32_t((v.ui64_ & DAY_MASK) >> DAY_SHIFT);
      }

      static uint32_t hours(const Data & v)
      {
        return uint32_t((v.ui64_ & HH_MASK) >> HH_SHIFT);
      }

      static uint32_t minutes(const Data & v)
      {
        return uint32_t((v.ui64_ & MM_MASK) >> MM_SHIFT);
      }

      static uint32_t seconds(const Data & v)
      {
        return uint32_t((v.ui64_ & SS_MASK) >> SS_SHIFT);
      }

      static uint32_t microseconds(const Data & v)
      {
        return uint32_t(
            (v.ui64_ & MICROSEC_MASK) >> MICROSEC_SHIFT);
      }

      static int64_t GetTimestump(const Data & v)
      {
        struct tm timeinfo;
        memset(&timeinfo, 0, sizeof(timeinfo));
        timeinfo.tm_year = year(v) - 1900;
        timeinfo.tm_mon = month(v) - 1;
        timeinfo.tm_mday = day(v);
        timeinfo.tm_hour = hours(v);
        timeinfo.tm_min = minutes(v);
        timeinfo.tm_sec = seconds(v);
        timeinfo.tm_isdst = -1;
        return std::mktime(&timeinfo);
      }

      static void AddDays(Data & v, const int32_t days_count)
      {
        struct tm timeinfo;
        memset(&timeinfo, 0, sizeof(timeinfo));
        timeinfo.tm_year = year(v) - 1900;
        timeinfo.tm_mon = month(v) - 1;
        timeinfo.tm_mday = day(v) + days_count;
        timeinfo.tm_hour = hours(v);
        timeinfo.tm_min = minutes(v);
        timeinfo.tm_sec = seconds(v);
        timeinfo.tm_isdst = -1;
        std::mktime(&timeinfo);
        uint64_t d(0);
        Set(timeinfo, &d);
        Reset(&v, d);
      }

      static void AddSeconds(Data & v, const int32_t ss)
      {
        struct tm timeinfo;
        memset(&timeinfo, 0, sizeof(timeinfo));
        timeinfo.tm_year = year(v) - 1900;
        timeinfo.tm_mon = month(v) - 1;
        timeinfo.tm_mday = day(v);
        timeinfo.tm_hour = hours(v);
        timeinfo.tm_min = minutes(v);
        timeinfo.tm_sec = seconds(v) + ss;
        timeinfo.tm_isdst = -1;
        std::mktime(&timeinfo);
        uint64_t d(0);
        Set(timeinfo, &d);
        Reset(&v, d);
      }

      static bool SetHour(Data & v, uint64_t h)
      {
        if (h > 23)
          return false;
        uint64_t old(OP_GET_UINT(v));
        uint64_t mask = ~HH_MASK;
        Reset(&v, (old & mask) + (h <<= HH_SHIFT));
        return true;
      }

      static bool SetMinute(Data & v, uint64_t m)
      {
        if (m > 59)
          return false;
        uint64_t old(OP_GET_UINT(v));
        uint64_t mask = ~MM_MASK;
        Reset(&v, (old & mask) + (m << MM_SHIFT));
        return true;
      }

      static bool SetSecond(Data & v, uint64_t s)
      {
        if (s > 59)
          return false;

        uint64_t old(OP_GET_UINT(v));
        uint64_t mask = ~SS_MASK;
        Reset(&v, (old & mask) + (s << SS_SHIFT));
        return true;
      }

      static bool IsLeap(const Data & v)
      {
        return IsLeap(year(v));
      }

      static Dynamic GetDate(const Data & v)
      {
        return Dynamic(year(v), month(v), day(v), 0, 0, 0, 0);
      }

      static Dynamic GetTime(const Data & v)
      {
        return Dynamic(0, 0, 0, hours(v), minutes(v), seconds(v),
            microseconds(v));
      }

      template<typename T>
      static bool OP_LT(const Dynamic & v, const Dynamic & rv)
      {
        return v.data_.ui64_ < T::OP_GET_UINT(rv.data_);
      }

      template<typename T>
      static bool OP_EQ(const Dynamic & v, const Dynamic & rv)
      {
        return v.data_.ui64_ == T::OP_GET_UINT(rv.data_);
      }

      static Data OP_GET_DEFAULT_DATA()
      {
        return OP_GET_MIN_DATA();
      }

      static Data OP_GET_MAX_DATA()
      {
        Data result;
        result.ui64_ = MAX_UINT64_VALUE;
        return result;
      }

      static Data OP_GET_MIN_DATA()
      {
        Data result;
        result.ui64_ = 0;
        return result;
      }

    private:
      static bool Set(const struct tm & timeinfo, uint64_t * const out)
      {
        return Set(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
            timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min,
            timeinfo.tm_sec, 0, out);
      }

      // return 1 if year is a leap year, else 0
      static bool IsLeap(uint64_t y)
      {
        if (y % 4)
          return false;
        if (y % 100)
          return true;
        if (y % 400)
          return false;
        return true;
      }

      static bool Set(const uint64_t year, const uint64_t month,
          const uint64_t day, const uint64_t hh, const uint64_t mm,
          const uint64_t ss, const uint64_t microsec, uint64_t * const out)
      {
        static const uint64_t DAYS_IN_MONTH[12] = {31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31};

        if (likely((year <= YEAR_MAX) && (month > 0) && (month <= 12)
            && (day > 0)
            && ((day <= DAYS_IN_MONTH[month-1])
                || ((month == 2) && (day == 29) && IsLeap(year)))
            && (hh <= 23) && (mm <= 59) && (ss <= 59)
            && (microsec <= MICROSEC_MAX)))
        {
          *out = (year << YEAR_SHIFT) + (month << MONTH_SHIFT)
              + (day << DAY_SHIFT) + (hh << HH_SHIFT) + (mm << MM_SHIFT)
              + (ss << SS_SHIFT) + (microsec << MICROSEC_SHIFT);
          return true;
        }

        return false;
      }

      static void Reset(Dynamic * v, const uint64_t ui)
      {
        v->type_ = DATE_TIME;
        v->data_.ui64_ = ui;
      }

      static void Reset(Data * v, const uint64_t ui)
      {
        v->ui64_ = ui;
      }
    };
  } // namespace detail
} // namespace nkit


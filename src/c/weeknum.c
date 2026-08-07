#include "weeknum.h"

static bool prv_is_leap(int year) {
  return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

// Weekday of Jan 1 for the tm's year, Monday = 0
static int prv_jan1_wday(const struct tm *t) {
  int wday_mon0 = (t->tm_wday + 6) % 7;
  return ((wday_mon0 - t->tm_yday) % 7 + 7) % 7;
}

static int prv_weeks_in_year(int year, int jan1_wday_mon0) {
  // ISO years have 53 weeks when Jan 1 is a Thursday, or a Wednesday in a leap year
  if (jan1_wday_mon0 == 3 || (jan1_wday_mon0 == 2 && prv_is_leap(year))) {
    return 53;
  }
  return 52;
}

int weeknum_days_in_year(int year) {
  return prv_is_leap(year) ? 366 : 365;
}

WeekInfo weeknum_get(const struct tm *t) {
  int year = 1900 + t->tm_year;
  int wday_mon0 = (t->tm_wday + 6) % 7;
  int week = (t->tm_yday - wday_mon0 + 10) / 7;
  int jan1 = prv_jan1_wday(t);

  if (week < 1) {
    // Days before the first ISO week belong to the last week of the previous year
    int prev = year - 1;
    int prev_jan1 = ((jan1 - (prv_is_leap(prev) ? 2 : 1)) % 7 + 7) % 7;
    week = prv_weeks_in_year(prev, prev_jan1);
  } else if (week > prv_weeks_in_year(year, jan1)) {
    week = 1;
  }

  int quarter_end = ((week + 12) / 13) * 13;
  if (quarter_end > 52) {
    quarter_end = 53;  // ISO week 53 closes Q4 in 53-week years
  }
  return (WeekInfo) { .week = week, .quarter_end = quarter_end };
}

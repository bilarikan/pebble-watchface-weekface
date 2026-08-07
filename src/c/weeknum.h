#pragma once

#include <pebble.h>

typedef struct {
  int week;         // ISO 8601 week number, 1..53
  int quarter_end;  // closing week of the fixed 13-week quarter (13/26/39/52)
} WeekInfo;

WeekInfo weeknum_get(const struct tm *t);

// 365 or 366
int weeknum_days_in_year(int year);

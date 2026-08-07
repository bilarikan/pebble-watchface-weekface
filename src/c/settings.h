#pragma once

#include <pebble.h>

typedef enum {
  TimeFormat12h = 0,       // "1:05", no AM/PM marker
  TimeFormat24h = 1,       // "13:05"
  TimeFormatMilitary = 2,  // "1305"
} TimeFormat;

typedef enum {
  WeekdayFull = 0,    // "Friday, August 07, 2026"
  WeekdayAbbr = 1,    // "Fri, August 07, 2026"
  WeekdayHidden = 2,  // "August 07, 2026"
} WeekdayStyle;

typedef enum {
  MonthFull = 0,  // "August"
  MonthAbbr = 1,  // "Aug"
} MonthStyle;

typedef struct {
  TimeFormat time_format;
  bool hourly_vibe;
  bool conn_vibe;
  bool show_week;  // W<n>/Q<n> indicator
  bool show_yday;  // D<n>/R<n> indicator
  WeekdayStyle weekday_style;
  MonthStyle month_style;
} Settings;

typedef void (*SettingsChangedHandler)(void);

void settings_init(SettingsChangedHandler handler);
const Settings *settings_get(void);

// Translated names. mon0: 0 = January. wday0: 0 = Sunday (struct tm convention).
const char *settings_month_name(int mon0, bool abbreviated);
const char *settings_day_name(int wday0, bool abbreviated);

// Indicator prefix letters: W32/Q39 and D219/R146 by default
typedef enum {
  IndWeek = 0,
  IndQuarter = 1,
  IndDay = 2,
  IndRemaining = 3,
} IndicatorLetter;

const char *settings_ind_letter(IndicatorLetter which);

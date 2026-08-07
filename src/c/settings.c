#include "settings.h"

#include <stdlib.h>
#include <string.h>

// Bumped whenever the persisted struct layout changes
#define SETTINGS_PERSIST_KEY 4
#define PERSIST_MONTHS_FULL 10
#define PERSIST_MONTHS_ABBR 11
#define PERSIST_DAYS_FULL 12
#define PERSIST_DAYS_ABBR 13
#define PERSIST_IND_LETTERS 14

// Persist entries max out at 256 bytes, so each name group must fit on its own:
// 12 * 20 = 240 and 12 * 8 = 96
#define FULL_NAME_BYTES 20
#define ABBR_NAME_BYTES 8
#define IND_LETTER_BYTES 8

typedef struct {
  char months_full[12][FULL_NAME_BYTES];
  char months_abbr[12][ABBR_NAME_BYTES];
  char days_full[7][FULL_NAME_BYTES];
  char days_abbr[7][ABBR_NAME_BYTES];
  char ind_letters[4][IND_LETTER_BYTES];
} Translations;

static Settings s_settings;
static Translations s_tr;
static SettingsChangedHandler s_changed_handler;

static const char *DEFAULT_MONTHS_FULL[12] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December",
};
static const char *DEFAULT_MONTHS_ABBR[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};
static const char *DEFAULT_DAYS_FULL[7] = {
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
};
static const char *DEFAULT_DAYS_ABBR[7] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};
static const char *DEFAULT_IND_LETTERS[4] = {"W", "Q", "D", "R"};

// Copy at most dst_len-1 bytes without splitting a UTF-8 sequence
static void prv_copy_utf8(char *dst, const char *src, size_t dst_len) {
  size_t n = strlen(src);
  if (n > dst_len - 1) {
    n = dst_len - 1;
    while (n > 0 && (src[n] & 0xC0) == 0x80) n--;
  }
  memcpy(dst, src, n);
  dst[n] = '\0';
}

static void prv_set_defaults(void) {
  s_settings = (Settings) {
    .time_format = TimeFormat24h,
    .hourly_vibe = false,
    .conn_vibe = false,
    .show_week = true,
    .show_quarter = true,
    .show_day = true,
    .show_remaining = true,
    .weekday_style = WeekdayFull,
    .month_style = MonthFull,
  };
  for (int i = 0; i < 12; i++) {
    prv_copy_utf8(s_tr.months_full[i], DEFAULT_MONTHS_FULL[i], FULL_NAME_BYTES);
    prv_copy_utf8(s_tr.months_abbr[i], DEFAULT_MONTHS_ABBR[i], ABBR_NAME_BYTES);
  }
  for (int i = 0; i < 7; i++) {
    prv_copy_utf8(s_tr.days_full[i], DEFAULT_DAYS_FULL[i], FULL_NAME_BYTES);
    prv_copy_utf8(s_tr.days_abbr[i], DEFAULT_DAYS_ABBR[i], ABBR_NAME_BYTES);
  }
  for (int i = 0; i < 4; i++) {
    prv_copy_utf8(s_tr.ind_letters[i], DEFAULT_IND_LETTERS[i], IND_LETTER_BYTES);
  }
}

// Clay sends radio-group values as strings, toggles as int32
static int prv_tuple_int(Tuple *t) {
  if (t->type == TUPLE_CSTRING) {
    return atoi(t->value->cstring);
  }
  return (int)t->value->int32;
}

// Read `count` sequential string keys into a name table; empty fields keep
// their current value
static void prv_read_names(DictionaryIterator *iter, uint32_t first_key, int count,
                           char *table, size_t stride) {
  for (int i = 0; i < count; i++) {
    Tuple *t = dict_find(iter, first_key + i);
    if (!t || t->type != TUPLE_CSTRING) continue;
    const char *src = t->value->cstring;
    if (src[0] != '\0') {
      prv_copy_utf8(table + (size_t)i * stride, src, stride);
    }
  }
}

static void prv_inbox_received(DictionaryIterator *iter, void *context) {
  Tuple *t;
  if ((t = dict_find(iter, MESSAGE_KEY_TimeFormat))) {
    s_settings.time_format = (TimeFormat)prv_tuple_int(t);
  }
  if ((t = dict_find(iter, MESSAGE_KEY_HourlyVibe))) {
    s_settings.hourly_vibe = prv_tuple_int(t) != 0;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_ConnVibe))) {
    s_settings.conn_vibe = prv_tuple_int(t) != 0;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_ShowWeek))) {
    s_settings.show_week = prv_tuple_int(t) != 0;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_ShowQuarter))) {
    s_settings.show_quarter = prv_tuple_int(t) != 0;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_ShowDay))) {
    s_settings.show_day = prv_tuple_int(t) != 0;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_ShowRemaining))) {
    s_settings.show_remaining = prv_tuple_int(t) != 0;
  }
  if ((t = dict_find(iter, MESSAGE_KEY_WeekdayStyle))) {
    s_settings.weekday_style = (WeekdayStyle)prv_tuple_int(t);
  }
  if ((t = dict_find(iter, MESSAGE_KEY_MonthStyle))) {
    s_settings.month_style = (MonthStyle)prv_tuple_int(t);
  }
  prv_read_names(iter, MESSAGE_KEY_MonthsFull, 12, &s_tr.months_full[0][0], FULL_NAME_BYTES);
  prv_read_names(iter, MESSAGE_KEY_MonthsAbbr, 12, &s_tr.months_abbr[0][0], ABBR_NAME_BYTES);
  prv_read_names(iter, MESSAGE_KEY_DaysFull, 7, &s_tr.days_full[0][0], FULL_NAME_BYTES);
  prv_read_names(iter, MESSAGE_KEY_DaysAbbr, 7, &s_tr.days_abbr[0][0], ABBR_NAME_BYTES);
  prv_read_names(iter, MESSAGE_KEY_IndLetters, 4, &s_tr.ind_letters[0][0], IND_LETTER_BYTES);

  persist_write_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
  persist_write_data(PERSIST_MONTHS_FULL, s_tr.months_full, sizeof(s_tr.months_full));
  persist_write_data(PERSIST_MONTHS_ABBR, s_tr.months_abbr, sizeof(s_tr.months_abbr));
  persist_write_data(PERSIST_DAYS_FULL, s_tr.days_full, sizeof(s_tr.days_full));
  persist_write_data(PERSIST_DAYS_ABBR, s_tr.days_abbr, sizeof(s_tr.days_abbr));
  persist_write_data(PERSIST_IND_LETTERS, s_tr.ind_letters, sizeof(s_tr.ind_letters));
  if (s_changed_handler) {
    s_changed_handler();
  }
}

void settings_init(SettingsChangedHandler handler) {
  s_changed_handler = handler;
  prv_set_defaults();
  if (persist_exists(SETTINGS_PERSIST_KEY)) {
    persist_read_data(SETTINGS_PERSIST_KEY, &s_settings, sizeof(s_settings));
  }
  if (persist_exists(PERSIST_MONTHS_FULL)) {
    persist_read_data(PERSIST_MONTHS_FULL, s_tr.months_full, sizeof(s_tr.months_full));
  }
  if (persist_exists(PERSIST_MONTHS_ABBR)) {
    persist_read_data(PERSIST_MONTHS_ABBR, s_tr.months_abbr, sizeof(s_tr.months_abbr));
  }
  if (persist_exists(PERSIST_DAYS_FULL)) {
    persist_read_data(PERSIST_DAYS_FULL, s_tr.days_full, sizeof(s_tr.days_full));
  }
  if (persist_exists(PERSIST_DAYS_ABBR)) {
    persist_read_data(PERSIST_DAYS_ABBR, s_tr.days_abbr, sizeof(s_tr.days_abbr));
  }
  if (persist_exists(PERSIST_IND_LETTERS)) {
    persist_read_data(PERSIST_IND_LETTERS, s_tr.ind_letters, sizeof(s_tr.ind_letters));
  }
  app_message_register_inbox_received(prv_inbox_received);
  app_message_open(2048, 64);
}

const Settings *settings_get(void) {
  return &s_settings;
}

const char *settings_month_name(int mon0, bool abbreviated) {
  return abbreviated ? s_tr.months_abbr[mon0] : s_tr.months_full[mon0];
}

const char *settings_day_name(int wday0, bool abbreviated) {
  return abbreviated ? s_tr.days_abbr[wday0] : s_tr.days_full[wday0];
}

const char *settings_ind_letter(IndicatorLetter which) {
  return s_tr.ind_letters[which];
}

#include "time_layer.h"

#define DATE_H 20

static Layer *s_root;
static TextLayer *s_date_text_layer;
static TextLayer *s_time_text_layer;
static char s_time_buf[8];
static char s_date_buf[64];

static TextLayer *prv_make_text_layer(GRect frame, const char *font_key) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_font(layer, fonts_get_system_font(font_key));
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
  layer_add_child(s_root, text_layer_get_layer(layer));
  return layer;
}

Layer *time_layer_create(GRect frame) {
  s_root = layer_create(frame);
  int w = frame.size.w;
  s_date_text_layer = prv_make_text_layer(GRect(0, 0, w, DATE_H), FONT_KEY_GOTHIC_18);
  // Custom translated names can exceed the line; ellipsize rather than clip
  text_layer_set_overflow_mode(s_date_text_layer, GTextOverflowModeTrailingEllipsis);
  s_time_text_layer =
      prv_make_text_layer(GRect(0, DATE_H, w, frame.size.h - DATE_H), FONT_KEY_BITHAM_42_BOLD);
  return s_root;
}

void time_layer_destroy(Layer *layer) {
  text_layer_destroy(s_time_text_layer);
  text_layer_destroy(s_date_text_layer);
  layer_destroy(layer);
  s_root = NULL;
}

void time_layer_set_time(const struct tm *t, const Settings *settings) {
  switch (settings->time_format) {
    case TimeFormat12h:
      strftime(s_time_buf, sizeof(s_time_buf), "%I:%M", t);
      if (s_time_buf[0] == '0') {
        memmove(s_time_buf, s_time_buf + 1, strlen(s_time_buf));
      }
      break;
    case TimeFormatMilitary:
      strftime(s_time_buf, sizeof(s_time_buf), "%H%M", t);
      break;
    case TimeFormat24h:
    default:
      strftime(s_time_buf, sizeof(s_time_buf), "%H:%M", t);
      break;
  }
  text_layer_set_text(s_time_text_layer, s_time_buf);

  // Date line from (possibly translated) names, e.g. "Friday, August 07, 2026"
  const char *mon = settings_month_name(t->tm_mon, settings->month_style == MonthAbbr);
  int year = 1900 + t->tm_year;
  if (settings->weekday_style == WeekdayHidden) {
    snprintf(s_date_buf, sizeof(s_date_buf), "%s %02d, %d", mon, t->tm_mday, year);
  } else {
    const char *wd = settings_day_name(t->tm_wday, settings->weekday_style == WeekdayAbbr);
    snprintf(s_date_buf, sizeof(s_date_buf), "%s, %s %02d, %d", wd, mon, t->tm_mday, year);
  }
  text_layer_set_text(s_date_text_layer, s_date_buf);
}

#include "info_layer.h"

#include "settings.h"

static Layer *s_layer;
static GFont s_font;

static int s_week, s_quarter_end;
static int s_day_of_year, s_days_remaining;
static bool s_show_week = true;
static bool s_show_quarter = true;
static bool s_show_day = true;
static bool s_show_remaining = true;

// "W32/Q39", "W32", or "Q39" depending on which parts are enabled
static void prv_format_pair(char *buf, size_t len,
                            bool first_on, IndicatorLetter first, int first_val,
                            bool second_on, IndicatorLetter second, int second_val) {
  buf[0] = '\0';
  size_t n = 0;
  if (first_on) {
    n = snprintf(buf, len, "%s%d", settings_ind_letter(first), first_val);
  }
  if (second_on && n < len) {
    snprintf(buf + n, len - n, "%s%s%d", first_on ? "/" : "",
             settings_ind_letter(second), second_val);
  }
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  graphics_context_set_text_color(ctx, GColorWhite);

  char left[32];
  prv_format_pair(left, sizeof(left), s_show_week, IndWeek, s_week,
                  s_show_quarter, IndQuarter, s_quarter_end);
  if (left[0]) {
    graphics_draw_text(ctx, left, s_font, GRect(4, 0, b.size.w - 8, b.size.h),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }

  char right[32];
  prv_format_pair(right, sizeof(right), s_show_day, IndDay, s_day_of_year,
                  s_show_remaining, IndRemaining, s_days_remaining);
  if (right[0]) {
    graphics_draw_text(ctx, right, s_font, GRect(4, 0, b.size.w - 8, b.size.h),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  }
}

Layer *info_layer_create(GRect frame) {
  s_layer = layer_create(frame);
  s_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  layer_set_update_proc(s_layer, prv_update_proc);
  return s_layer;
}

void info_layer_destroy(Layer *layer) {
  layer_destroy(layer);
  s_layer = NULL;
}

void info_layer_set(int week, int quarter_end, int day_of_year, int days_remaining) {
  if (s_week == week && s_quarter_end == quarter_end &&
      s_day_of_year == day_of_year && s_days_remaining == days_remaining) {
    return;
  }
  s_week = week;
  s_quarter_end = quarter_end;
  s_day_of_year = day_of_year;
  s_days_remaining = days_remaining;
  if (s_layer) layer_mark_dirty(s_layer);
}

void info_layer_set_visible(bool show_week, bool show_quarter, bool show_day,
                            bool show_remaining) {
  if (s_show_week == show_week && s_show_quarter == show_quarter &&
      s_show_day == show_day && s_show_remaining == show_remaining) {
    return;
  }
  s_show_week = show_week;
  s_show_quarter = show_quarter;
  s_show_day = show_day;
  s_show_remaining = show_remaining;
  if (s_layer) layer_mark_dirty(s_layer);
}

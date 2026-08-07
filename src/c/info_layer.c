#include "info_layer.h"

#include "settings.h"

static Layer *s_layer;
static GFont s_font;

static int s_week, s_quarter_end;
static int s_day_of_year, s_days_remaining;
static bool s_show_week = true;
static bool s_show_yday = true;

static void prv_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  graphics_context_set_text_color(ctx, GColorWhite);

  if (s_show_week) {
    char week_text[32];
    snprintf(week_text, sizeof(week_text), "%s%d/%s%d",
             settings_ind_letter(IndWeek), s_week,
             settings_ind_letter(IndQuarter), s_quarter_end);
    graphics_draw_text(ctx, week_text, s_font, GRect(4, 0, b.size.w - 8, b.size.h),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }

  if (s_show_yday) {
    char yday_text[32];
    snprintf(yday_text, sizeof(yday_text), "%s%d/%s%d",
             settings_ind_letter(IndDay), s_day_of_year,
             settings_ind_letter(IndRemaining), s_days_remaining);
    graphics_draw_text(ctx, yday_text, s_font, GRect(4, 0, b.size.w - 8, b.size.h),
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

void info_layer_set_visible(bool show_week, bool show_yday) {
  if (s_show_week == show_week && s_show_yday == show_yday) return;
  s_show_week = show_week;
  s_show_yday = show_yday;
  if (s_layer) layer_mark_dirty(s_layer);
}

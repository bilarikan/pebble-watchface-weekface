#include <pebble.h>

#include "calendar_layer.h"
#include "info_layer.h"
#include "settings.h"
#include "status_layer.h"
#include "time_layer.h"
#include "weeknum.h"

#define STATUS_BAR_H 22
#define TIME_AREA_H 68
#define INFO_ROW_H 22

static Window *s_window;
static Layer *s_status_layer;
static Layer *s_time_layer;
static Layer *s_info_layer;
static Layer *s_calendar_layer;

static void prv_update_time_and_week(void) {
  const Settings *s = settings_get();
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  time_layer_set_time(t, s);
  WeekInfo wi = weeknum_get(t);
  int day_of_year = t->tm_yday + 1;
  int days_remaining = weeknum_days_in_year(1900 + t->tm_year) - day_of_year;
  info_layer_set_visible(s->show_week, s->show_yday);
  info_layer_set(wi.week, wi.quarter_end, day_of_year, days_remaining);
}

static void prv_update_steps(void) {
#if defined(PBL_HEALTH)
  time_t start = time_start_of_today();
  time_t end = time(NULL);
  HealthServiceAccessibilityMask mask =
      health_service_metric_accessible(HealthMetricStepCount, start, end);
  if (mask & HealthServiceAccessibilityMaskAvailable) {
    status_layer_set_steps((int)health_service_sum_today(HealthMetricStepCount));
  }
#endif
}

#if defined(PBL_HEALTH)
static void prv_health_handler(HealthEventType event, void *context) {
  if (event == HealthEventMovementUpdate || event == HealthEventSignificantUpdate) {
    prv_update_steps();
  }
}
#endif

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  prv_update_time_and_week();
  prv_update_steps();
  if (units_changed & DAY_UNIT) {
    calendar_layer_refresh();
  }
  if (settings_get()->hourly_vibe && tick_time->tm_min == 0 && !quiet_time_is_active()) {
    vibes_short_pulse();
  }
}

static void prv_battery_handler(BatteryChargeState state) {
  status_layer_set_battery(state);
}

static void prv_connection_handler(bool connected) {
  status_layer_set_connected(connected);
  if (settings_get()->conn_vibe && !quiet_time_is_active()) {
    if (connected) {
      vibes_short_pulse();
    } else {
      vibes_double_pulse();
    }
  }
}

static void prv_settings_changed(void) {
  prv_update_time_and_week();
  calendar_layer_refresh();          // header day names may have been retranslated
  layer_mark_dirty(s_info_layer);    // indicator letters may have changed
}

// Timeline Quick View obstructs the bottom of the screen. The calendar is the
// bottom block, so hide it outright rather than showing it clipped mid-row.
static void prv_apply_unobstructed(void) {
  Layer *root = window_get_root_layer(s_window);
  GRect full = layer_get_bounds(root);
  GRect ub = layer_get_unobstructed_bounds(root);
  layer_set_hidden(s_calendar_layer, ub.size.h < full.size.h);
}

static void prv_unobstructed_did_change(void *context) {
  prv_apply_unobstructed();
}

static void prv_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect b = layer_get_bounds(root);

  const int info_y = STATUS_BAR_H + TIME_AREA_H;
  const int cal_y = info_y + INFO_ROW_H;
  s_status_layer = status_layer_create(GRect(0, 0, b.size.w, STATUS_BAR_H));
  s_time_layer = time_layer_create(GRect(0, STATUS_BAR_H, b.size.w, TIME_AREA_H));
  s_info_layer = info_layer_create(GRect(0, info_y, b.size.w, INFO_ROW_H));
  s_calendar_layer = calendar_layer_create(GRect(0, cal_y, b.size.w, b.size.h - cal_y));
  layer_add_child(root, s_status_layer);
  layer_add_child(root, s_time_layer);
  layer_add_child(root, s_info_layer);
  layer_add_child(root, s_calendar_layer);

  status_layer_set_battery(battery_state_service_peek());
  status_layer_set_connected(connection_service_peek_pebble_app_connection());
  prv_update_time_and_week();
  prv_update_steps();

  unobstructed_area_service_subscribe((UnobstructedAreaHandlers) {
    .did_change = prv_unobstructed_did_change,
  }, NULL);
  prv_apply_unobstructed();
}

static void prv_window_unload(Window *window) {
  unobstructed_area_service_unsubscribe();
  status_layer_destroy(s_status_layer);
  time_layer_destroy(s_time_layer);
  info_layer_destroy(s_info_layer);
  calendar_layer_destroy(s_calendar_layer);
}

static void prv_init(void) {
  settings_init(prv_settings_changed);

  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
  battery_state_service_subscribe(prv_battery_handler);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = prv_connection_handler,
  });
#if defined(PBL_HEALTH)
  health_service_events_subscribe(prv_health_handler, NULL);
#endif
}

static void prv_deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}

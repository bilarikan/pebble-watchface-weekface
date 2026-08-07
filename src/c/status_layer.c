#include "status_layer.h"

static Layer *s_layer;
static GFont s_font;

static BatteryChargeState s_battery;
static bool s_connected;
static int s_steps = -1;  // -1: not yet known, draw nothing

static void prv_format_steps(char *buf, size_t len, int steps) {
  if (steps < 1000) {
    snprintf(buf, len, "%d", steps);
  } else {
    snprintf(buf, len, "%d.%dk", steps / 1000, (steps % 1000) / 100);
  }
}

static void prv_draw_battery(GContext *ctx) {
  // Small vertical battery: nub on top, body outline, fill level inside
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(7, 4, 3, 2), 0, GCornerNone);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_rect(ctx, GRect(4, 6, 9, 14));

  GColor fill = GColorWhite;
  if (s_battery.is_charging) {
    fill = COLOR_FALLBACK(GColorGreen, GColorWhite);
  } else if (s_battery.charge_percent <= 20) {
    fill = COLOR_FALLBACK(GColorRed, GColorWhite);
  }
  graphics_context_set_fill_color(ctx, fill);
  const int inner_h = 10;
  int fh = (s_battery.charge_percent * inner_h + 50) / 100;
  graphics_fill_rect(ctx, GRect(6, 8 + (inner_h - fh), 5, fh), 0, GCornerNone);
}

static void prv_draw_bluetooth(GContext *ctx) {
  // Bluetooth rune drawn with lines; shown only while connected
  const int cx = 22, top = 5, bot = 19;
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx, GPoint(cx, top), GPoint(cx, bot));
  graphics_draw_line(ctx, GPoint(cx, top), GPoint(cx + 4, top + 4));
  graphics_draw_line(ctx, GPoint(cx + 4, top + 4), GPoint(cx - 4, bot - 4));
  graphics_draw_line(ctx, GPoint(cx, bot), GPoint(cx + 4, bot - 4));
  graphics_draw_line(ctx, GPoint(cx + 4, bot - 4), GPoint(cx - 4, top + 4));
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);

  prv_draw_battery(ctx);
  if (s_connected) {
    prv_draw_bluetooth(ctx);
  }

  graphics_context_set_text_color(ctx, GColorWhite);

  if (s_steps >= 0) {
    char steps_text[16];
    prv_format_steps(steps_text, sizeof(steps_text), s_steps);
    graphics_draw_text(ctx, steps_text, s_font, GRect(0, 0, b.size.w - 4, 22),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  }
}

Layer *status_layer_create(GRect frame) {
  s_layer = layer_create(frame);
  s_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  layer_set_update_proc(s_layer, prv_update_proc);
  return s_layer;
}

void status_layer_destroy(Layer *layer) {
  layer_destroy(layer);
  s_layer = NULL;
}

void status_layer_set_battery(BatteryChargeState state) {
  s_battery = state;
  if (s_layer) layer_mark_dirty(s_layer);
}

void status_layer_set_connected(bool connected) {
  if (s_connected == connected) return;
  s_connected = connected;
  if (s_layer) layer_mark_dirty(s_layer);
}

void status_layer_set_steps(int steps) {
  if (s_steps == steps) return;
  s_steps = steps;
  if (s_layer) layer_mark_dirty(s_layer);
}

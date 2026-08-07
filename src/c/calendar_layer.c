#include "calendar_layer.h"

#include <string.h>

#include "settings.h"

#define CAL_DAYS 21
#define HEADER_H 16

typedef struct {
  uint8_t mday;
  bool is_today;
  bool is_past;
} CalCell;

static Layer *s_layer;
static CalCell s_cells[CAL_DAYS];
static GFont s_header_font;
static GFont s_day_font;
static GFont s_today_font;

static GRect prv_cell_rect(GRect bounds, int col, int y, int h) {
  int x0 = col * bounds.size.w / 7;
  int x1 = (col + 1) * bounds.size.w / 7;
  return GRect(x0, y, x1 - x0, h);
}

static void prv_draw_centered(GContext *ctx, const char *text, GFont font, GRect cell) {
  graphics_draw_text(ctx, text, font, cell, GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);
}

// First `max_cp` UTF-8 code points of src
static void prv_utf8_prefix(char *dst, const char *src, size_t dst_len, int max_cp) {
  size_t n = 0;
  int cp = 0;
  while (src[n] && n < dst_len - 1) {
    if ((src[n] & 0xC0) != 0x80) {  // lead byte starts a new code point
      if (cp == max_cp) break;
      cp++;
    }
    dst[n] = src[n];
    n++;
  }
  dst[n] = '\0';
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  int row_h = (b.size.h - HEADER_H) / 3;

  graphics_context_set_text_color(ctx, COLOR_FALLBACK(GColorLightGray, GColorWhite));
  for (int c = 0; c < 7; c++) {
    // Monday-first columns; day names come from the translation settings
    char hdr[12];
    prv_utf8_prefix(hdr, settings_day_name((c + 1) % 7, true), sizeof(hdr), 2);
    prv_draw_centered(ctx, hdr, s_header_font, prv_cell_rect(b, c, 0, HEADER_H));
  }

  for (int i = 0; i < CAL_DAYS; i++) {
    int r = i / 7, c = i % 7;
    GRect cell = prv_cell_rect(b, c, HEADER_H + r * row_h, row_h);
    GRect text_rect = GRect(cell.origin.x, cell.origin.y + (row_h - 24) / 2,
                            cell.size.w, 24);
    char txt[4];
    snprintf(txt, sizeof(txt), "%d", s_cells[i].mday);

    if (s_cells[i].is_today) {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx, GRect(cell.origin.x + 1, cell.origin.y + 2,
                                    cell.size.w - 2, row_h - 4), 3, GCornersAll);
      graphics_context_set_text_color(ctx, GColorBlack);
      prv_draw_centered(ctx, txt, s_today_font, text_rect);
    } else {
      graphics_context_set_text_color(
          ctx, s_cells[i].is_past ? COLOR_FALLBACK(GColorDarkGray, GColorWhite) : GColorWhite);
      prv_draw_centered(ctx, txt, s_day_font, text_rect);
    }
  }
}

void calendar_layer_refresh(void) {
  // Anchor at noon so day arithmetic never lands on a DST boundary
  time_t now = time(NULL);
  struct tm noon = *localtime(&now);
  noon.tm_hour = 12;
  noon.tm_min = 0;
  noon.tm_sec = 0;
  noon.tm_isdst = -1;
  time_t base = mktime(&noon);

  int mon_offset = (noon.tm_wday + 6) % 7;  // days since Monday
  int start = -mon_offset - 7;              // Monday of the previous week
  for (int i = 0; i < CAL_DAYS; i++) {
    int off = start + i;
    time_t ti = base + (time_t)off * SECONDS_PER_DAY;
    struct tm *lt = localtime(&ti);
    s_cells[i] = (CalCell) {
      .mday = lt->tm_mday,
      .is_today = off == 0,
      .is_past = off < 0,
    };
  }
  if (s_layer) layer_mark_dirty(s_layer);
}

Layer *calendar_layer_create(GRect frame) {
  s_layer = layer_create(frame);
  s_header_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_day_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  s_today_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  layer_set_update_proc(s_layer, prv_update_proc);
  calendar_layer_refresh();
  return s_layer;
}

void calendar_layer_destroy(Layer *layer) {
  layer_destroy(layer);
  s_layer = NULL;
}

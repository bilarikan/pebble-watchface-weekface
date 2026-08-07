#pragma once

#include <pebble.h>

// 3-week calendar grid: previous week, current week, next week.
// Weeks start on Monday (consistent with the ISO week indicator).
Layer *calendar_layer_create(GRect frame);
void calendar_layer_destroy(Layer *layer);

// Recompute the grid from the current date and redraw.
void calendar_layer_refresh(void);

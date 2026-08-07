#pragma once

#include <pebble.h>

// Indicator row between the time and the calendar:
// W<week>/Q<quarter-end> on the left, <day of year>/<days remaining> on the right.
Layer *info_layer_create(GRect frame);
void info_layer_destroy(Layer *layer);

void info_layer_set(int week, int quarter_end, int day_of_year, int days_remaining);
void info_layer_set_visible(bool show_week, bool show_quarter, bool show_day,
                            bool show_remaining);

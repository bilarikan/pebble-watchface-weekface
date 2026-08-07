#pragma once

#include <pebble.h>

#include "settings.h"

// Full date line with the large time display beneath it.
Layer *time_layer_create(GRect frame);
void time_layer_destroy(Layer *layer);

void time_layer_set_time(const struct tm *t, const Settings *settings);

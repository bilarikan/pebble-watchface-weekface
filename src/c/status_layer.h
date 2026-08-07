#pragma once

#include <pebble.h>

// Top status bar: battery + bluetooth icons (left), step count (right).
Layer *status_layer_create(GRect frame);
void status_layer_destroy(Layer *layer);

void status_layer_set_battery(BatteryChargeState state);
void status_layer_set_connected(bool connected);
void status_layer_set_steps(int steps);

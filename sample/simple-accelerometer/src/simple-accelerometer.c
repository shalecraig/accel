#include <pebble.h>

#include "accel.h"

static Window *window;
static TextLayer *text_layer;
static accel_state *state = NULL;

static int recording_gesture = 0;
static bool recording = false;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  int result = 0;
  if (recording) {
    text_layer_set_text(text_layer, "Done recording");
    result = accel_end_record_gesture(state, recording_gesture);
    if (result == ACCEL_SUCCESS) {
      recording = false;
      text_layer_set_text(text_layer, "End Successfully");
    } else {
      text_layer_set_text(text_layer, "End Unsuccessfully");
    }
  } else {
    result = accel_start_record_gesture(state, &recording_gesture);
    if (result == ACCEL_SUCCESS) {
      recording = true;
      text_layer_set_text(text_layer, "Start Successfully");
    } else {
      text_layer_set_text(text_layer, "Start Unsuccessfully");
    }
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (state == NULL) {
    int result = accel_generate_state(&state, 3, 1);
    if (result == ACCEL_SUCCESS) {
      text_layer_set_text(text_layer, "Allocated Successfuly");
    } else {
      text_layer_set_text(text_layer, "sAllocated Unsuccessfullys");
    }
  } else {
    int result = accel_destroy_state(&state);
    if (result == ACCEL_SUCCESS) {
      text_layer_set_text(text_layer, "Successful deletion");
    } else {
      text_layer_set_text(text_layer, "Non-successful deletion");
    }
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  AccelData data;
  accel_service_peek(&data);

  int accel_data[3] = {data.x, data.y, data.z};

  int result = accel_process_timer_tick(state, accel_data);
  if (result == ACCEL_SUCCESS) {
    text_layer_set_text(text_layer, "Successful tick");

    int gesture_id = 0;
    int distance = 0;
    accel_find_most_likely_gesture(state, &gesture_id, &distance);
    APP_LOG(APP_LOG_LEVEL_INFO, "Distance: %i, gesture: %i", distance, gesture_id);
  } else {
    text_layer_set_text(text_layer, "Non-successful tick");
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  accel_data_service_subscribe(0, NULL);
  window_stack_push(window, animated);
}

static void deinit(void) {
  accel_data_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}

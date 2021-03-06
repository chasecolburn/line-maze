#include <pebble.h>

Window *window;
Layer *leftHourLayer;
Layer *rightHourLayer;
Layer *leftMinuteLayer;
Layer *rightMinuteLayer;

GBitmap *patternOnImage;
GBitmap *patternOffImage;


// messages index
enum MESSAGE_IDS {
  SHAKE_TO_CHEAT,
  SEND_CONFIG,
  JS_INITIALIZED,
};

bool hideBackground;
int hours = 0, minutes = 0;
bool shakeToCheat = true;

// Use the low order first 20 bits to define each number
int numbers[10] = {0xF999F, /* 0 */ 
                   0xE4446, /* 1 */
                   0xF1F8F, /* 2 */ 
                   0xF8E8F, /* 3 */
                   0x88F99, /* 4 */ 
                   0xF8F1F, /* 5 */
                   0xF9F1F, /* 6 */
                   0x8888F, /* 7 */
                   0xF9F9F, /* 8 */
                   0x88F9F  /* 9 */};

//-----------------------------------------------------------------------------
// Interface

void handle_init();
void handle_deinit();
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed);
void draw_layer(Layer *layer, GContext *gctxt);
void draw_time_layer(Layer *layer, GContext *gctxt);
int display_number_for_layer(Layer *layer);
void redraw();
void handle_accel(AccelAxisType axis, int32_t direction);

void trigger_config_fetch();

void in_received_handler(DictionaryIterator *received, void *context);
void in_dropped_handler(AppMessageResult reason, void *context);
void out_sent_handler(DictionaryIterator *sent, void *context);
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);

//-----------------------------------------------------------------------------
// Implementation

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}

void handle_init() {
  // messaging
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);

  const uint32_t inbound_size = 64;
  const uint32_t outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  if(persist_exists(SHAKE_TO_CHEAT)) {
    shakeToCheat = persist_read_bool(SHAKE_TO_CHEAT);
    APP_LOG(APP_LOG_LEVEL_ERROR, "shake-to-cheat config: %d", (int) shakeToCheat);
  }

  patternOnImage = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PATTERN_ON);
  patternOffImage = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PATTERN_OFF);
  
  window = window_create();
  window_stack_push(window, true /* Animated */);
    
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  hours = tick_time->tm_hour;
  minutes = tick_time->tm_min;
  hideBackground = false;

  Layer *root = window_get_root_layer(window);
  layer_set_update_proc(root, draw_layer);
  layer_mark_dirty(root);

  leftHourLayer = layer_create(GRect(12, 12, 48, 60));
  layer_set_update_proc(leftHourLayer, draw_time_layer);
  layer_add_child(root, leftHourLayer);
  layer_mark_dirty(leftHourLayer);

  rightHourLayer = layer_create(GRect(84, 12, 48, 60));
  layer_set_update_proc(rightHourLayer, draw_time_layer);
  layer_add_child(root, rightHourLayer);
  layer_mark_dirty(rightHourLayer);

  leftMinuteLayer = layer_create(GRect(12, 96, 48, 60));
  layer_set_update_proc(leftMinuteLayer, draw_time_layer);
  layer_add_child(root, leftMinuteLayer);
  layer_mark_dirty(leftMinuteLayer);

  rightMinuteLayer = layer_create(GRect(84, 96, 48, 60));
  layer_set_update_proc(rightMinuteLayer, draw_time_layer);
  layer_add_child(root, rightMinuteLayer);
  layer_mark_dirty(rightMinuteLayer);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  accel_tap_service_subscribe(handle_accel);
  
}

void handle_deinit() {
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  gbitmap_destroy(patternOnImage);
  gbitmap_destroy(patternOffImage);
  layer_destroy(leftHourLayer);
  layer_destroy(rightHourLayer);
  layer_destroy(leftMinuteLayer);
  layer_destroy(rightMinuteLayer);
  window_destroy(window);
}

// Any update to the screen causes the entire row to update so might as well mark both numbers dirty at the same time
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)   {
  if(units_changed & HOUR_UNIT) {
    hours = tick_time->tm_hour;
    layer_mark_dirty(leftHourLayer);
    layer_mark_dirty(rightHourLayer);
  }

  if(units_changed & MINUTE_UNIT) {
    minutes = tick_time->tm_min;
    layer_mark_dirty(leftMinuteLayer);
    layer_mark_dirty(rightMinuteLayer);
  }
}

// Draw entire screen
void draw_layer(Layer *layer, GContext *gctxt) {
  GRect destination = GRect(0, 0, 144, 168);
  if(hideBackground) {
    graphics_context_set_fill_color(gctxt, GColorBlack);
    graphics_fill_rect(gctxt, destination, 0, GCornerNone);
  }else {
    graphics_draw_bitmap_in_rect(gctxt, patternOffImage, destination);
  }
}

// Draw layer for each number
void draw_time_layer(Layer *layer, GContext *gctxt) {
  int displayNumber = display_number_for_layer(layer);
  for(int row = 0; row < 5; row++) {
    for(int col = 0; col < 4; col++) {
      GRect destination = GRect(col * 12, row * 12, 12, 12);
      // Draw the tile on
      if(numbers[displayNumber] & (1 << ((row * 4) + col))) {
        graphics_draw_bitmap_in_rect(gctxt, patternOnImage, destination);  
      }
      // Draw the tile off
      else {
        if(hideBackground) {
          graphics_context_set_fill_color(gctxt, GColorBlack);
          graphics_fill_rect(gctxt, destination, 0, GCornerNone);
        }else {
          graphics_draw_bitmap_in_rect(gctxt, patternOffImage, destination);
        }
      }
    }
  }
}

// Return number to display for the target layer
int display_number_for_layer(Layer *layer) {
  // Hour 10s place
  if(layer == leftHourLayer) return hours / 10;
  // Hour 1s pace
  if(layer == rightHourLayer) return hours % 10;
  // Minute 10s place
  if(layer == leftMinuteLayer) return minutes / 10;
  // Minute 1s place
  if(layer == rightMinuteLayer) return minutes % 10;
  // Default (should never reach here)
  return 0;
}

void redraw() {
  Layer *root = window_get_root_layer(window);
  layer_mark_dirty(root);
  layer_mark_dirty(leftHourLayer);
  layer_mark_dirty(rightHourLayer);
  layer_mark_dirty(leftMinuteLayer);
  layer_mark_dirty(rightMinuteLayer);
}

void handle_accel(AccelAxisType axis, int32_t direction) {
  //app_log(APP_LOG_LEVEL_INFO, "", 0, "Mode was toggled");

  if(shakeToCheat) {
    hideBackground = !hideBackground;
    redraw();
  }
}

void trigger_config_fetch() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Triggering config fetch");

  DictionaryIterator *iter;

  int res;

  if( (res = app_message_outbox_begin(&iter)) != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Dict iterator size is %d", sizeof(iter));
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error beginning to write to outbox, error #%d", res);
    return;
  }

  Tuplet send_config = TupletCString(SEND_CONFIG, "send-config"); // the value is a dummy
  if( (res = dict_write_tuplet(iter, &send_config)) != DICT_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error writing tuplet to dictionary, error #%d", res);
  }

  app_message_outbox_send();
}

void in_received_handler(DictionaryIterator *received, void *context) {
  Tuple *shakeToCheatConfig = dict_find(received, SHAKE_TO_CHEAT);

  if(shakeToCheatConfig) {
    shakeToCheat = shakeToCheatConfig->value->int32 == 1 ? true : false;
    persist_write_int(SHAKE_TO_CHEAT, shakeToCheat);
    if(!persist_exists(SHAKE_TO_CHEAT)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error persisting config");
    }
    hideBackground = shakeToCheat;
    redraw();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received SHAKE_TO_CHEAT configuration value: %d", (int) shakeToCheat);
  }

  Tuple *jsInitialized = dict_find(received, JS_INITIALIZED);
  if(jsInitialized) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received JS_INITIALIZED message");
    if(!persist_exists(SHAKE_TO_CHEAT)) {
      trigger_config_fetch();
    }
  }
}

void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message dropped: %d", reason);
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message sent");
}


void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message sending failed: %d", reason);
  // outgoing message failed
}

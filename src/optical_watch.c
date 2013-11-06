#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

// Define application
#define AUTO_GENERATED_UUID { 0xAF, 0x77, 0xC6, 0x54, 0x47, 0x51, 0x42, 0x10, 0xB1, 0x92, 0xB1, 0x69, 0x5A, 0x18, 0xD8, 0x63 }
PBL_APP_INFO(AUTO_GENERATED_UUID, "Line Maze", "Chase Colburn", 1, 0, DEFAULT_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;

Layer leftHourLayer;
Layer rightHourLayer;
Layer leftMinuteLayer;
Layer rightMinuteLayer;

int hours = 0, minutes = 0;
// Use the low order first 20 bits to define each number
int numbers[10] = {0xF999F, /* 0 */ 
                   0xF4446, /* 1 */
                   0xF1F8F, /* 2 */ 
                   0xF8E8F, /* 3 */
                   0x88F99, /* 4 */ 
                   0xF8F1F, /* 5 */
                   0xF9F1F, /* 6 */
                   0x8888F, /* 7 */
                   0xF9F9F, /* 8 */
                   0x88F9F  /* 9 */};

BmpContainer patternOnContainer;
BmpContainer patternOffContainer;

//-----------------------------------------------------------------------------
// Interface

void handle_init(AppContextRef ctx);
void handle_tick(AppContextRef ctxt, PebbleTickEvent *event);
void draw_layer(Layer *layer, GContext *gctxt);
void draw_time_layer(Layer *layer, GContext *gctxt);
int display_number_for_layer(Layer *layer);

//-----------------------------------------------------------------------------
// Implementation

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info =
    {
      .tick_handler = &handle_tick,
      .tick_units = SECOND_UNIT
    }
  };

  app_event_loop(params, &handlers);
}

void handle_init(AppContextRef ctx) {
  window_init(&window, "Line Maze");
  window_stack_push(&window, true /* Animated */);
    
  resource_init_current_app(&APP_RESOURCES);
  
  bmp_init_container(RESOURCE_ID_IMAGE_PATTERN_ON, &patternOnContainer);
  bmp_init_container(RESOURCE_ID_IMAGE_PATTERN_OFF, &patternOffContainer);

  Layer *root = window_get_root_layer(&window);
  layer_set_update_proc(root, draw_layer);
  layer_mark_dirty(root);

  layer_init(&leftHourLayer, GRect(12, 12, 48, 60));
  layer_set_update_proc(&leftHourLayer, draw_time_layer);
  layer_add_child(root, &leftHourLayer);

  layer_init(&rightHourLayer, GRect(84, 12, 48, 60));
  layer_set_update_proc(&rightHourLayer, draw_time_layer);
  layer_add_child(root, &rightHourLayer);

  layer_init(&leftMinuteLayer, GRect(12, 96, 48, 60));
  layer_set_update_proc(&leftMinuteLayer, draw_time_layer);
  layer_add_child(root, &leftMinuteLayer);

  layer_init(&rightMinuteLayer, GRect(84, 96, 48, 60));
  layer_set_update_proc(&rightMinuteLayer, draw_time_layer);
  layer_add_child(root, &rightMinuteLayer);
}

// Any update to the screen causes the entire row to update so might as well mark both numbers dirty at the same time
void handle_tick(AppContextRef ctxt, PebbleTickEvent *event)   {
  PblTm time;
  get_time(&time);

  if(time.tm_hour != hours) {
    hours = time.tm_hour;
    layer_mark_dirty(&leftHourLayer);
    layer_mark_dirty(&rightHourLayer);
  }

  // Since this event is triggered every minute, these will always need to be redrawn
  minutes = time.tm_min;
  layer_mark_dirty(&leftMinuteLayer);
  layer_mark_dirty(&rightMinuteLayer);
}

// Draw entire screen
void draw_layer(Layer *layer, GContext *gctxt) {
  graphics_draw_bitmap_in_rect(gctxt, &patternOffContainer.bmp, GRect(0, 0, 144, 168));
}

// Draw layer for each number
void draw_time_layer(Layer *layer, GContext *gctxt) {
  GRect destination = GRect(0, 0, 48, 60);
  graphics_draw_bitmap_in_rect(gctxt, &patternOffContainer.bmp, destination);

  int displayNumber = display_number_for_layer(layer);
  for(int row = 0; row < 5; row++) {
    for(int col = 0; col < 4; col++) {
      destination = GRect(col * 12, row * 12, 12, 12);
      if(numbers[displayNumber] & (1 << ((row * 4) + col))) {
        graphics_draw_bitmap_in_rect(gctxt, &patternOnContainer.bmp, destination);  
      }else {
        graphics_draw_bitmap_in_rect(gctxt, &patternOffContainer.bmp, destination);
      }
    }
  }
}

// Return number to display for the target layer
int display_number_for_layer(Layer *layer) {
  // Hour 10s place
  if(layer == &leftHourLayer) return hours / 10;
  // Hour 1s place
  if(layer == &rightHourLayer) return hours % 10;
  // Minute 10s place
  if(layer == &leftMinuteLayer) return minutes / 10;
  // Minute 1s place
  if(layer == &rightMinuteLayer) return minutes % 10;
  // Default (should never reach here)
  return 0;
}


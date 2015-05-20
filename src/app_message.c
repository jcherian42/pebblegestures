#include <pebble.h>
#include <time.h>

Window *window;	
static TextLayer *s_output_layer;
	
// Key values for AppMessage Dictionary
enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1
};


// Write message to buffer & send
static void data_handler(AccelData *data, uint32_t num_samples) {
  static char s_buffer[256];
  static char output_buffer[256];

  snprintf(s_buffer, sizeof(s_buffer),
    "%d,%d,%d,%d,%d\n",
    (int)time(NULL), (int)time_ms(NULL, NULL), data[0].x, data[0].y, data[0].z
  );
  snprintf(output_buffer, sizeof(output_buffer),
    "Time(s): %d\nTime(ms): %d\nX: %d\nY: %d\nZ: %d\n",
    (int)time(NULL), (int)time_ms(NULL, NULL), data[0].x, data[0].y, data[0].z
  );
  	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
  dict_write_cstring(iter, STATUS_KEY, s_buffer);
  	
	dict_write_end(iter);
  app_message_outbox_send();

  text_layer_set_text(s_output_layer, output_buffer);
}


// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	
	tuple = dict_find(received, STATUS_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %d", (int)tuple->value->uint32); 
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
	}}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}

void init(void) {
	window = window_create();
  s_output_layer = text_layer_create(GRect(0, 0, 144,154));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_output_layer));
	window_stack_push(window, true);
	
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
  accel_data_service_subscribe(2, data_handler);
}

void deinit(void) {
	app_message_deregister_callbacks();
  text_layer_destroy(s_output_layer);
	window_destroy(window);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}
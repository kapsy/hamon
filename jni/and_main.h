// and_main.h

#ifndef AND_MAIN_H_
#define AND_MAIN_H_

#include <android/asset_manager.h>

#define INTERACTIVE_TTL (3*SEC_IN_US)

void trigger_note(float x, float y);

extern int sles_init_called;
extern int show_gameplay;
extern int touch_enabled;
extern int buttons_activated;
extern int show_help;
extern unsigned long buttons_activated_time;

extern size_t screen_width;
extern size_t screen_height;

extern size_t screen_margin_x_l;
extern size_t screen_margin_x_r;
extern size_t screen_margin_y_t;
extern size_t screen_margin_y_b;

extern AAssetManager* asset_manager;

#endif /* AND_MAIN_H_ */

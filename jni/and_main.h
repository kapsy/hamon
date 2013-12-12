/*
 * and_main.h
 *
 *  Created on: 2013/06/21
 *      Author: Michael
 */

#ifndef AND_MAIN_H_
#define AND_MAIN_H_


#define INTERACTIVE_TTL (4*SEC_IN_US)

void trigger_note(float x, float y);

extern int sles_init_called;
extern int show_gameplay;
extern int touch_enabled;

extern int interactive_mode;


extern int show_help;

extern unsigned long interactive_on_time;


extern size_t screen_width;
extern size_t screen_height;
extern size_t screen_height_reduced;
extern size_t screen_margin_y;
extern size_t screen_margin_x;



//extern unsigned long elapsed_time;


#endif /* AND_MAIN_H_ */

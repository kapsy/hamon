/*
 * gfx_butn.h
 *
 *  Created on: 2013/11/29
 *      Author: Michael
 */

#ifndef GFX_BUTN_H_
#define GFX_BUTN_H_

#include "hon_type.h"
#include "gfx_gles.h"
#include "gfx_asst.h"

#define BTN_W 128
#define BTN_ALPHA_MAX 1.0F
#define BTN_ALPHA_MIN 0.0F
#define BTN_FADE_RATE (0.4F/(float)SEC_IN_US)
//#define BTN_FADE_RATE 1.0f

enum {

	TOUCH_EVENT_BUTTON_0 = 0,
	TOUCH_EVENT_BUTTON_1 = 1,
	TOUCH_EVENT_BUTTON_2 = 2,
	TOUCH_EVENT_GAME = 3,
	TOUCH_EVENT_NULL = 4

};

typedef struct {
	texture_file* main_texture;
	texture_file* pressed_texture;
	float gl_x, gl_y;
	float scale;
	float alpha;

	// bl: x=0, y=max tr: x=max, y=0
	vec2 touch_bl, touch_tr;
	int event_enum;

	int is_touch_anim;
	int fading_in;
	int fading_out;
	float fade_rate;



}button;

//extern int show_buttons;
//extern int btn_fading_in;
//extern int btn_fading_out;

extern button buttons[];
extern vertex btn_quad[];
extern unsigned short btn_quad_index[];

extern int sizeof_button_element;
extern int sizeof_buttons;
extern int sizeof_btn_quad;
extern int sizeof_btn_quad_index;

void calc_btn_quad_verts(int bm_w, int bm_h);
int get_touch_response(float x, float y);
//void btn_anim(button* b);
void btn_anim(button* b, int index);

#endif /* GFX_BUTN_H_ */

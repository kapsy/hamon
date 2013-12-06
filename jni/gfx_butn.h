/*
 * gfx_butn.h
 *
 *  Created on: 2013/11/29
 *      Author: Michael
 */


///* fuga.h */
//#ifndef FUGA_H
//#define FUGA_H
//
//struct hoge_t;Å@/* ëOï˚êÈåæ */
//
//typedef struct fuga_t
//{
//  struct hoge_t* hoge;
//} Fuga;


#ifndef GFX_BUTN_H_
#define GFX_BUTN_H_

//#include "gfx_gles.h"

//#include "hon_type.h"
//#include "gfx/vertex.h"

#define BTN_W 128
#define BTN_ALPHA_MAX 1.0F
#define BTN_ALPHA_MIN 0.0F
#define BTN_FADE_RATE (0.4F/(float)SEC_IN_US)
#define BTN_PRESS_FADE_RATE (7.0F/(float)SEC_IN_US)
#define BTN_PRESS_FADE_RATE_OUT (1.3F/(float)SEC_IN_US)
//#define BTN_FADE_RATE 1.0f

enum {

	TOUCH_EVENT_BUTTON_0 = 0,
	TOUCH_EVENT_BUTTON_1 = 1,
	TOUCH_EVENT_BUTTON_2 = 2,
	TOUCH_EVENT_GAME = 3,
	TOUCH_EVENT_NULL = 4

};

//texture_file;
//struct vertex;

struct texture_file;
struct vec2;

struct button{
	struct texture_file* main_texture;
	struct texture_file* pressed_texture;
	float gl_x, gl_y;
	float scale;
	float alpha;
	float alpha_pt;
	int pressed_peak;

	// bl: x=0, y=max tr: x=max, y=0
	struct vec2 touch_bl, touch_tr;
	int event_enum;

	int is_touch_anim;
	int fading_in;
	int fading_out;
	float fade_rate;



};

//extern int show_buttons;
//extern int btn_fading_in;
//extern int btn_fading_out;

extern struct button buttons[];
extern struct vertex btn_quad[];
extern unsigned short btn_quad_index[];

extern int sizeof_button_array;
extern int sizeof_buttons;
extern int sizeof_btn_quad;
extern int sizeof_btn_quad_index;

void calc_btn_quad_verts(int bm_w, int bm_h);
int get_touch_response(float x, float y);
//void btn_anim(button* b);
void btn_anim(struct button* b, int index);

#endif /* GFX_BUTN_H_ */

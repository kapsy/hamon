/*
 * gfx_butn.c
 *
 *  Created on: 2013/11/29
 *      Author: Michael
 */


#include "gfx_butn.h"


//int show_buttons = FALSE;
//int btn_fading_in = FALSE;
//int btn_fading_out = FALSE;


button buttons[] = {
		{textures + 2, textures + 5,
				-1.0F, -1.0F, 1.0F, 0.0F, 0.0F, FALSE,
				0.0F, 0.0F, 0.0F, 0.0F,
				TOUCH_EVENT_BUTTON_0, FALSE, TRUE, FALSE, BTN_FADE_RATE},
		{textures + 3, textures + 6,
				-1.0F, -1.0F, 1.0F, 0.0F, 0.0F, FALSE,
				0.0F, 0.0F, 0.0F, 0.0F,
				TOUCH_EVENT_BUTTON_1, FALSE, FALSE, FALSE, BTN_FADE_RATE},
		{textures + 4, textures + 7,
				-1.0F, -1.0F, 1.0F, 0.0F, 0.0F, FALSE,
				0.0F, 0.0F, 0.0F, 0.0F,
				TOUCH_EVENT_BUTTON_2, FALSE, FALSE, FALSE, BTN_FADE_RATE}
};

vertex btn_quad[] = {
	{0.0f, 		0.0f, 		0.0f, 		0.0f, 		0.0f, 		0.0f}, //0, 0
	{0.25f, 	0.0f, 		0.0f, 		1.0f, 		0.0f, 		0.0f}, //0,1
	{0.25f, 	0.25f, 	0.0f, 		1.0f,		1.0f, 		0.0f},
	{0.0f, 		0.25f, 	0.0f, 		0.0f,		1.0f, 		0.0f},
};

unsigned short btn_quad_index[] = {
  0, 1, 3, 2
};

int sizeof_button_array = sizeof buttons/sizeof buttons[0];
int sizeof_buttons = sizeof buttons;
int sizeof_btn_quad = sizeof btn_quad;
int sizeof_btn_quad_index = sizeof btn_quad_index;

double btn_fade_rate = BTN_FADE_RATE;


void set_btn_fading(int b);
void set_index_fading(int i);
//int buttons_fading();


void calc_btn_quad_verts(int bm_w, int bm_h) {

	float gl_w = ((float)bm_w/(float)g_sc.width)*2.0F;
	float gl_h = ((float)bm_h/(float)g_sc.height)*2.0F;

	LOGD("calc_btn_quad_verts", "gl_w: %f", gl_w);

	btn_quad[1].x = gl_w;
	btn_quad[2].x = gl_w; 	btn_quad[2].y = gl_h;
									btn_quad[3].y = gl_h;
	int i;

	for (i = 0; i < sizeof_button_array; i++) {
		button* b = buttons + i;

		b->gl_x = -1.0F + (i * gl_w);

		b->touch_bl.x = gl_to_scr(-1.0F+(i * gl_w), 1);
		b->touch_bl.y = gl_to_scr(-1.0F, 0);
		b->touch_tr.x = gl_to_scr(-1.0F+((i+1)*gl_w),1);
		b->touch_tr.y = gl_to_scr(-1.0+gl_h, 0);

		LOGD("calc_btn_quad_verts", "b->touch_bl.x: %f", b->touch_bl.x);
		LOGD("calc_btn_quad_verts", "b->touch_bl.y: %f", b->touch_bl.y);
		LOGD("calc_btn_quad_verts", "b->touch_tr.x: %f", b->touch_tr.x);
		LOGD("calc_btn_quad_verts", "b->touch_tr.y: %f", b->touch_tr.y);
		LOGD("calc_btn_quad_verts", "b->gl_x: %f", b->gl_x);

	}
}

int get_touch_response(float x, float y) {

	LOGD("get_touch_response", "x: %f, y: %f", x, y);


	int response = TOUCH_EVENT_GAME;

	int i;
	for (i = 0; i < sizeof_button_array; i++) {
		button* b = buttons + i;

		LOGD("get_touch_response", "b->touch_bl.x: %f, b->touch_bl.y: %f", b->touch_bl.x, b->touch_bl.y);
		LOGD("get_touch_response", "b->touch_tr.x: %f, b->touch_tr.y: %f", b->touch_tr.x, b->touch_tr.y);


//		11-29 12:26:31.113: D/get_touch_response(30149): x: 67.187500, y: 432.656250
//		11-29 12:26:31.113: D/get_touch_response(30149): b->touch_bl.x: 0.000000, b->touch_bl.y: 480.000000
//		11-29 12:26:31.117: D/get_touch_response(30149): b->touch_tr.x: 128.000000, b->touch_tr.y: 352.000000

//		if (!buttons_fading()) {

		if (!b->fading_in && !b->fading_out && !b->is_touch_anim) {
			if (x>b->touch_bl.x && x<b->touch_tr.x) {
				if (y<b->touch_bl.y && y>b->touch_tr.y) {

					b->is_touch_anim = TRUE;

					response = b->event_enum;
				}
			}
		}
//		else response = TOUCH_EVENT_NULL;

	}

	return response;
}


void btn_anim(button* b, int i) {
//	LOGD("btn_anim", "btn_anim called");

	if (b->fading_in) {
		LOGD("btn_anim", "b->fading_in");
		if (b->alpha < BTN_ALPHA_MAX) {
			b->alpha += (float)frame_delta * b->fade_rate;

			if (b->alpha > 0.57F)	set_index_fading(i+1);

			LOGD("btn_anim", "b->alpha : %f", b->alpha);
		}
		else if (b->alpha >= BTN_ALPHA_MAX) {
			LOGD("btn_anim", "else if (b->alpha >= BTN_ALPHA_MAX)");
			b->alpha = BTN_ALPHA_MAX;
			b->fading_in = FALSE;
//			set_btn_fading(i+1);
		}
	}

//	if (b->fading_out) {
//		if (b->alpha > BTN_ALPHA_MIN) {
//			b->alpha -= BTN_FADE_RATE;
//		} else if (b->alpha <= BTN_ALPHA_MIN) {
//			b->alpha = BTN_ALPHA_MIN;
//			b->fading_out = FALSE;
//		}
//	}

	if (b->is_touch_anim) {

		if (!b->pressed_peak) {
			b->alpha_pt += (float)frame_delta * BTN_PRESS_FADE_RATE;
		}
		if (b->alpha_pt >= 1.0F && !b->pressed_peak) {
			b->pressed_peak = TRUE;
			b->alpha_pt = 1.0F;
		}

		if (b->pressed_peak) {
			b->alpha_pt -= (float)frame_delta * BTN_PRESS_FADE_RATE_OUT;
		}

		if (b->alpha_pt <= 0.0F && b->pressed_peak) {
			b->pressed_peak = FALSE;
			b->is_touch_anim = FALSE;
			b->alpha_pt = 0.0F;
		}
	}
}

void set_btn_fading(int b) {
	if (b < sizeof_button_array)
		buttons[b].fading_in = TRUE;
}
void set_index_fading(int i) {
	if (i < sizeof_button_array) {
		button* b = buttons + i;
		if (!b->fading_in)
			b->fading_in = TRUE;
	}
}

//int buttons_fading() {
//	int r = FALSE;
//	int i;
//	for (i = 0; i < sizeof_button_array; i++) {
//		button* b = buttons + i;
//		if (b->fading_in || b->fading_out) r = TRUE;
//	}
//	return r;
//}












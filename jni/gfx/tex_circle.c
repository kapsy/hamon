/*
 * tex_circle.c
 *
 *  Created on: 2013/12/16
 *      Author: Michael
 */


#include <jni.h>
#include <errno.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
// kapsy
#include <android/asset_manager.h>
#include <android/storage_manager.h>
#include <android/window.h>

#include <unistd.h>  // sleep()‚ð’è‹`
#include <pthread.h>
#include <math.h>
#include <stdlib.h>

#include "gfx/vertex.h"

#include "hon_type.h"
#include "tex_circle.h"

#include <time.h>
#include "gfx/frame_delta.h"


#include "gfx_gles.h"
#include "gfx_asst.h"

#include "game/moods.h"



#define TEXC_SHRINK_RATE (0.1/(float)SEC_IN_US)
#define TEXC_ALPHA_FADE_IN (3.6/(float)SEC_IN_US)
#define TEXC_ALPHA_FADE_OUT (0.205/(float)SEC_IN_US)

#define TEXR_GROW_RATE (1.8/(float)SEC_IN_US) //(2.2/(
#define TEXR_ALPHA_FADE_IN (3.6/(float)SEC_IN_US)
#define TEXR_ALPHA_FADE_OUT 0.94f


//pthread_mutex_t tex_frame_mutex = PTHREAD_MUTEX_INITIALIZER;

struct tex_circle tex_circles[TEX_CIRCLES_MAX];
unsigned int tex_circle_draw_order[TEX_CIRCLES_MAX];
struct tex_circle tex_ripples[TEX_CIRCLES_MAX];


int sizeof_tex_circles_e = sizeof(tex_circles)/sizeof(tex_circles[0]); // e: elements
int sizeof_tex_circles = sizeof(tex_circles);


void step_tex_circle_draw_order();
//void calc_circle_vertex();


//struct vertex tex_circle_v[4];
//unsigned short tex_circle_i[4];


//struct vertex tex_circle_v[] = {
//	{-0.5f, 	-0.5f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{0.5f, 		-0.5f, 	0.0f, 		1.0f, 		0.0f, 		0.0f}, //0,1
//	{0.5f, 		0.5f, 		0.0f, 		1.0f,		1.0f, 		0.0f},
//	{-0.5f, 	0.5f,		0.0f, 		0.0f, 		1.0f,		0.0f},
//};

struct vertex tex_circle_v[] = {
	{0.0f, 		0.0f, 		0.0f, 		0.0f, 		0.0f, 		0.0f},
	{0.0f,		0.0f, 		0.0f,		1.0f, 		0.0f, 		0.0f}, //0,1
	{0.0f, 		0.0f, 		0.0f,		1.0f,		1.0f, 		0.0f},
	{0.0f, 		0.0f, 		0.0f, 		0.0f, 		1.0f,		0.0f},
};


unsigned short tex_circle_i[] = {
  0, 1, 3, 2
};

int sizeof_tex_circle_v = sizeof tex_circle_v;
int sizeof_tex_circle_i = sizeof tex_circle_i;


void init_tex_circles() {
	int i;
	for(i=0;i<sizeof_tex_circles_e;i++) {
		tex_circles[i].is_alive = FALSE;




		tex_circles[i].tex = (textures + 10);
		tex_ripples[i].tex = (textures + 10);

		tex_ripples[i].is_alive = FALSE;
		tex_circle_draw_order[i] = i;

		tex_circles[i].rgb = (moods + selected_mood)->rgb_circ;
		tex_ripples[i].rgb = (moods + selected_mood)->rgb_circ;
//
//		ts->rgb->r = (moods + selected_mood)->rgb_circ->r;
//		ts->rgb->g = (moods + selected_mood)->rgb_circ->g;
//		ts->rgb->b = (moods + selected_mood)->rgb_circ->b;


	}
}

void step_tex_circle_draw_order() {
	int i;
	for(i=0;i<sizeof_tex_circles_e;i++) {
			if (tex_circle_draw_order[i] < sizeof_tex_circles_e)
				tex_circle_draw_order[i]++;
			if (tex_circle_draw_order[i] == sizeof_tex_circles_e)
				tex_circle_draw_order[i] = 0;
	}
}

rgb part_colors_test[] = {
	{0.597993F, 1.000000F, 0.658208F},
	{0.249421F, 0.896186F, 1.000000F},
	{1.000000F, 0.035373F, 0.319272F},
	{0.329373F, 0.094343F, 1.000000F},
	{0.879933F, 0.123191F, 1.000000F},
	{0.404804F, 0.984234F, 1.000000F},
	{0.465192F, 0.537905F, 1.000000F},
	{1.000000F, 0.186882F, 0.480862F}
};
void activate_tex_circle(float x, float y, size_t col, float* vel) {

	pthread_mutex_lock(&frame_mutex);
	step_tex_circle_draw_order();
	struct tex_circle* ts = tex_circles + (tex_circle_draw_order[sizeof_tex_circles_e -1]);
//	LOGI("activate_touch_circle", "t_circle_draw_order[TOUCH_SHAPES_MAX -1] %d",
//			t_circle_draw_order[TOUCH_SHAPES_MAX -1]);

	ts->pos_x = ((x/(float)g_sc.width)*2)-1;
	ts->pos_y = ((1.0F - (y/(float)g_sc.height))*2)-1;
	LOGI("activate_touch_circle", "x: %f ts->pos_x: %f", x, ts->pos_x);
	LOGI("activate_touch_circle", "y: %f ts->pos_y: %f", y, ts->pos_y);

//	ts->rgb[0] = part_colors_test[col].r;
//	ts->rgb[1] = part_colors_test[col].g;
//	ts->rgb[2] = part_colors_test[col].b;


	ts->rgb = (moods + selected_mood)->rgb_circ;

	LOGD("activate_touch_circle", "selected_mood: %d", selected_mood);

//	ts->rgb->g = (moods + selected_mood)->rgb_circ->g;
//	ts->rgb->b = (moods + selected_mood)->rgb_circ->b;


	ts->alpha = 0.0F; // TODO


	ts->scale = *vel * *vel * 1.7; // TODO Šù‚ÉŒvŽZ‚·‚ê‚Î‚¢‚¢‚Ì‚©‚à
	ts->alpha_max = ts->scale / 2.0; // TODO
	ts->fading_in = TRUE;
	ts->is_alive = TRUE;



	struct tex_circle* tr = tex_ripples + (tex_circle_draw_order[sizeof_tex_circles_e -1]);
	tr->pos_x = ((x/(float)g_sc.width)*2)-1;
	tr->pos_y = ((1.0F - (y/(float)g_sc.height))*2)-1;
//	ts->rgb[0] = part_colors_test[col].r;
//	ts->rgb[1] = part_colors_test[col].g;
//	ts->rgb[2] = part_colors_test[col].b;

//	tr->rgb->r = (moods + selected_mood)->rgb_circ->r;
//	tr->rgb->g = (moods + selected_mood)->rgb_circ->g;
//	tr->rgb->b = (moods + selected_mood)->rgb_circ->b;
	tr->rgb = (moods + selected_mood)->rgb_circ;

	tr->alpha = 0.0F; // TODO

	tr->scale = *vel * *vel * 1.7; // TODO Šù‚ÉŒvŽZ‚·‚ê‚Î‚¢‚¢‚Ì‚©‚à
	tr->alpha_max = *vel;

	if (tr->alpha_max >= 1.0) tr->alpha_max = 1.0;
	tr->alpha_delta_factor = 0.000004F;
	tr->fading_in = TRUE;
	tr->is_alive = TRUE;

	pthread_mutex_unlock(&frame_mutex);
}


void tex_circle_alpha_size(struct tex_circle* ts) {

	ts->scale -= (float)frame_delta * TEXC_SHRINK_RATE;

	if (ts->fading_in) {

		ts->alpha += (float)frame_delta * TEXC_ALPHA_FADE_IN;
		if (ts->alpha >= ts->alpha_max) ts->fading_in = FALSE;
	}

	if (!ts->fading_in) {
		ts->alpha -= (float)frame_delta * TEXC_ALPHA_FADE_OUT;
		if (ts->alpha <= 0) ts->is_alive = FALSE;
	}


}

void tex_ripple_alpha_size(struct tex_circle* tr) {

	tr->scale += (float)frame_delta * TEXR_GROW_RATE;

	if (tr->fading_in) {
		tr->alpha += (float)frame_delta *  TEXR_ALPHA_FADE_IN;
		if (tr->alpha >= (tr->alpha_max * 0.95)) tr->fading_in = FALSE;
	}

	if (!tr->fading_in) {

		tr->alpha *= TEXR_ALPHA_FADE_OUT;
//				ts->alpha *= (frame_delta_ratio * 0.98);
		if (tr->alpha < 0.005) tr->is_alive = FALSE;
	}
}

void kill_all_tex_circles() {

	int i;
	for (i=0; i<sizeof_tex_circles_e; i++) {
		struct tex_circle* ts = tex_circles + i;
		ts->is_alive = FALSE;
	}
}
//struct vertex tex_circle_v[] = {
//	{-0.05f, -0.05f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{0.05f, 	-0.05f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{0.05f, 	0.05f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{-0.05f, 	0.05f,		0.0f, 		0.0f, 		0.0f,		0.0f},
//};

void calc_tex_circle_vertex() {

	float tex_h = TEX_TO_W_RATIO/g_sc.hw_ratio;
	float y_b = 0.0f - (tex_h/2.0f);
	float y_t = 0.0f + (tex_h/2.0f);

	float x_l = 0.0f - (TEX_TO_W_RATIO/2.0f);
	float	x_r = 0.0f + (TEX_TO_W_RATIO/2.0f);

	tex_circle_v[0].x = x_l; 		tex_circle_v[0].y = y_b;
	tex_circle_v[1].x = x_r; 		tex_circle_v[1].y = y_b;
	tex_circle_v[2].x = x_r; 		tex_circle_v[2].y = y_t;
	tex_circle_v[3].x = x_l; 		tex_circle_v[3].y = y_t;
}


/*
 * touch_circles.c
 *
 *  Created on: 2013/12/09
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
#include "touch_circle.h"

#include <time.h>
#include "gfx/frame_delta.h"


#include "gfx_gles.h"



#define TC_SHRINK_RATE (0.1/(float)SEC_IN_US)
#define TC_ALPHA_FADE_IN (3.6/(float)SEC_IN_US)
#define TC_ALPHA_FADE_OUT (0.205/(float)SEC_IN_US)

#define TR_GROW_RATE (2.2/(float)SEC_IN_US)
#define TR_ALPHA_FADE_IN (3.6/(float)SEC_IN_US)
#define TR_ALPHA_FADE_OUT 0.94f


pthread_mutex_t frame_mutex = PTHREAD_MUTEX_INITIALIZER;

struct touch_circle t_circles[TOUCH_CIRCLES_MAX];
unsigned int t_circle_draw_order[TOUCH_CIRCLES_MAX];
struct touch_circle t_ripples[TOUCH_CIRCLES_MAX];


int sizeof_t_circles_e = sizeof(t_circles)/sizeof(t_circles[0]); // e: elements
int sizeof_t_circles = sizeof(t_circles);


void step_touch_circle_draw_order();
void calc_circle_vertex();


struct vertex solid_circle_v[CIRCLE_SEGMENTS+1];
unsigned short solid_circle_i[CIRCLE_SEGMENTS+2];

int sizeof_solid_circle_v = sizeof solid_circle_v;
int sizeof_solid_circle_i = sizeof solid_circle_i;


rgb part_colors[] = {
	{0.597993F, 1.000000F, 0.658208F},
	{0.249421F, 0.896186F, 1.000000F},
	{1.000000F, 0.035373F, 0.319272F},
	{0.329373F, 0.094343F, 1.000000F},
	{0.879933F, 0.123191F, 1.000000F},
	{0.404804F, 0.984234F, 1.000000F},
	{0.465192F, 0.537905F, 1.000000F},
	{1.000000F, 0.186882F, 0.480862F}
};



void init_touch_circles() {
	int i;
	for(i=0;i<sizeof_t_circles_e;i++) {
		t_circles[i].is_alive = FALSE;
		t_ripples[i].is_alive = FALSE;
//		touch_no_ammo[i].is_alive = FALSE;
		t_circle_draw_order[i] = i;
//		touch_no_ammo_draw_order[i] = i;
	}
}

void step_touch_circle_draw_order() {
	int i;
	for(i=0;i<sizeof_t_circles_e;i++) {
			if (t_circle_draw_order[i] < sizeof_t_circles_e)
				t_circle_draw_order[i]++;
			if (t_circle_draw_order[i] == sizeof_t_circles_e)
				t_circle_draw_order[i] = 0;
	}
}


void activate_touch_circle(float x, float y, size_t col, float* vel) {

	pthread_mutex_lock(&frame_mutex);
	step_touch_circle_draw_order();
	struct touch_circle* ts = t_circles + (t_circle_draw_order[sizeof_t_circles_e -1]);
//	LOGI("activate_touch_circle", "t_circle_draw_order[TOUCH_SHAPES_MAX -1] %d",
//			t_circle_draw_order[TOUCH_SHAPES_MAX -1]);

	ts->pos_x = ((x/(float)g_sc.width)*2)-1;
	ts->pos_y = ((1.0F - (y/(float)g_sc.height))*2)-1;
	LOGI("activate_touch_circle", "x: %f ts->pos_x: %f", x, ts->pos_x);
	LOGI("activate_touch_circle", "y: %f ts->pos_y: %f", y, ts->pos_y);

	ts->rgb[0] = part_colors[col].r;
	ts->rgb[1] = part_colors[col].g;
	ts->rgb[2] = part_colors[col].b;

//	LOGI("activate_touch_shape", "ts->rgb[0] %f ts->rgb[1] %f ts->rgb[2] %f", ts->rgb[0], ts->rgb[1], ts->rgb[2]);

	ts->alpha = 0.0F; // TODO


	ts->scale = *vel * *vel * 1.7; // TODO Šù‚ÉŒvŽZ‚·‚ê‚Î‚¢‚¢‚Ì‚©‚à
//	LOGI("activate_touch_shape", "ts->scale: %f", ts->scale);

	ts->alpha_max = ts->scale / 2.0; // TODO

//	ts->ttl = TOUCH_SHAPES_TTL;

	ts->fading_in = TRUE;
	ts->is_alive = TRUE;

	struct touch_circle* tr = t_ripples + (t_circle_draw_order[sizeof_t_circles_e -1]);
	tr->pos_x = ((x/(float)g_sc.width)*2)-1;
	tr->pos_y = ((1.0F - (y/(float)g_sc.height))*2)-1;
	tr->rgb[0] = part_colors[col].r;
	tr->rgb[1] = part_colors[col].g;
	tr->rgb[2] = part_colors[col].b;
	tr->alpha = 0.0F; // TODO
	tr->scale = *vel * *vel * 1.7; // TODO Šù‚ÉŒvŽZ‚·‚ê‚Î‚¢‚¢‚Ì‚©‚à

	tr->alpha_max = *vel;
	if (tr->alpha_max >= 1.0) tr->alpha_max = 1.0;
	LOGI("activate_touch_shape", "tr->alpha_max: %f", tr->alpha_max);

	tr->alpha_delta_factor = 0.000004F;
	tr->fading_in = TRUE;
	tr->is_alive = TRUE;

	pthread_mutex_unlock(&frame_mutex);
}

void t_circle_alpha_size(struct touch_circle* ts) {

	ts->scale -= (float)frame_delta * TC_SHRINK_RATE;

	if (ts->fading_in) {

		ts->alpha += (float)frame_delta * TC_ALPHA_FADE_IN;
		if (ts->alpha >= ts->alpha_max) ts->fading_in = FALSE;
	}

	if (!ts->fading_in) {
		ts->alpha -= (float)frame_delta * TC_ALPHA_FADE_OUT;
		if (ts->alpha <= 0) ts->is_alive = FALSE;
	}


}

void t_ripple_alpha_size(struct touch_circle* tr) {

	tr->scale += (float)frame_delta * TR_GROW_RATE;

	if (tr->fading_in) {
		tr->alpha += (float)frame_delta *  TR_ALPHA_FADE_IN;
		if (tr->alpha >= (tr->alpha_max * 0.95)) tr->fading_in = FALSE;
	}

	if (!tr->fading_in) {

		tr->alpha *= TR_ALPHA_FADE_OUT;
//				ts->alpha *= (frame_delta_ratio * 0.98);
		if (tr->alpha < 0.005) tr->is_alive = FALSE;
	}
}



void kill_all_touch_circles() {

	int i;
	for (i=0; i<sizeof_t_circles_e; i++) {
		struct touch_circle* ts = t_circles + i;
		ts->is_alive = FALSE;


		LOGD ("kill_all_touch_shapes", "kill_all_touch_shapes i: %d", i);
	}
}



void calc_circle_vertex() {

	int i, n = CIRCLE_SEGMENTS;
	float r = 0.25F;
	double rate;

	float largest_x = 0.0;
	float largest_y = 0.0;
	float one_factor_x = 1.0;
	float one_factor_y = 1.0;

	solid_circle_v[0].x = 0.0;
	solid_circle_v[0].y = 0.0;

	for (i = 0; i < n; i++) {

		rate = (double) i / n;
		solid_circle_v[i + 1].x = r * g_sc.hw_ratio * sin(2.0 * PI * rate);
		solid_circle_v[i + 1].y = r * cos(2.0 * PI * rate);

		float abs_x = fabsf(solid_circle_v[i + 1].x);
		float abs_y = fabsf(solid_circle_v[i + 1].y);

		if (abs_x > largest_x)
			largest_x = abs_x;
		if (abs_y > largest_y)
			largest_y = abs_y;

	}

	one_factor_x = 1.0F / largest_x;
	one_factor_y = 1.0F / largest_y;

	for (i = 0; i < CIRCLE_SEGMENTS + 1; i++) {
		solid_circle_i[i] = i;

		solid_circle_v[i].r = solid_circle_v[i].x * one_factor_x;
		solid_circle_v[i].g = solid_circle_v[i].y * one_factor_y;
		solid_circle_v[i].b = 0.5;//solid_circle_vertex[i].y * one_factor_y;

		LOGI(
				"calc_circle_vertex",
				"solid_circle_v[i].r %f "
				"solid_circle_v[i].g %f "
				"solid_circle_v[i].b %f ",
				solid_circle_v[i].r, solid_circle_v[i].g, solid_circle_v[i].b);

	}
	solid_circle_i[CIRCLE_SEGMENTS + 1] = 1;

}



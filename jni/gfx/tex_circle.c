// tex_circle.c
#include "common.h"
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
#define TEXC_NO_AMMO_SHRINK_RATE (3.0/(float)SEC_IN_US)
#define TEXC_NO_AMMO_ALPHA_FADE_IN (3.6/(float)SEC_IN_US)
#define TEXC_NO_AMMO_ALPHA_FADE_OUT (4.0/(float)SEC_IN_US)
#define TEXC_NO_AMMO_SCALE 1.2f


struct tex_circle tex_circles[TEX_CIRCLES_MAX];
unsigned int tex_circle_draw_order[TEX_CIRCLES_MAX];
struct tex_circle tex_ripples[TEX_CIRCLES_MAX];

int sizeof_tex_circles_e = sizeof(tex_circles)/sizeof(tex_circles[0]); // e: elements
int sizeof_tex_circles = sizeof(tex_circles);

void step_tex_circle_draw_order();

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

		(tex_circles + i)->rgb = (struct vertex_rgb*)malloc(sizeof(struct vertex_rgb*));
		(tex_ripples + i)->rgb = (struct vertex_rgb*)malloc(sizeof(struct vertex_rgb*));
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

void activate_tex_circle(float x, float y, struct vertex_rgb* rgb_p, float* vel) {
	pthread_mutex_lock(&frame_mutex);

	step_tex_circle_draw_order();
	struct tex_circle* ts = tex_circles + (tex_circle_draw_order[sizeof_tex_circles_e -1]);

	ts->pos_x = ((x/(float)g_sc.width)*2)-1;
	ts->pos_y = ((1.0F - (y/(float)g_sc.height))*2)-1;
//	LOGI("activate_tex_circle", "x: %f ts->pos_x: %f", x, ts->pos_x);
//	LOGI("activate_tex_circle", "y: %f ts->pos_y: %f", y, ts->pos_y);

	struct vertex_rgb* rgb_c = (moods+selected_mood)->rgb_circ;
	struct vertex_rgb* rgb_m = (moods+selected_mood)->rgb_circ_mask;

	ts->rgb->r = rgb_c->r * (1.0f - (rgb_p->r*rgb_m->r));
	ts->rgb->g = rgb_c->g * (1.0f - (rgb_p->g*rgb_m->g));
	ts->rgb->b = rgb_c->b * (1.0f - (rgb_p->b*rgb_m->b));
	LOGD("activate_tex_circle", "rgb_p->r: %f, g: %f, b: %f", rgb_p->r, rgb_p->g, rgb_p->b);
	ts->alpha = 0.0F; // TODO

	ts->scale = *vel * *vel * 1.7; // TODO 既に計算すればいいのかも
	ts->scale_change_rate = TEXC_SHRINK_RATE;

	ts->alpha_max = ts->scale / 2.0; // TODO
	ts->alpha_fade_in = TEXC_ALPHA_FADE_IN;
	ts->alpha_fade_out = TEXC_ALPHA_FADE_OUT;
	ts->fading_in = TRUE;
	ts->is_alive = TRUE;

	struct tex_circle* tr = tex_ripples + (tex_circle_draw_order[sizeof_tex_circles_e -1]);
	tr->pos_x = ((x/(float)g_sc.width)*2)-1;
	tr->pos_y = ((1.0F - (y/(float)g_sc.height))*2)-1;

	tr->rgb = (moods + selected_mood)->rgb_circ;

	tr->alpha = 0.0F; // TODO
	tr->scale = *vel * *vel * 1.7; // TODO 既に計算すればいいのかも
	tr->scale_change_rate = TEXR_GROW_RATE;

	tr->alpha_max = *vel;
	if (tr->alpha_max >= 1.0) tr->alpha_max = 1.0;
	// tr->alpha_delta_factor = 0.000004F;
	tr->alpha_fade_in = TEXR_ALPHA_FADE_IN;
	tr->alpha_fade_out = TEXR_ALPHA_FADE_OUT;
	tr->fading_in = TRUE;
	tr->is_alive = TRUE;

	pthread_mutex_unlock(&frame_mutex);
}

// フェードの大きさと収縮する速度を引く数へ
void activate_tex_no_ammo(float x, float y, float* vel) {
	pthread_mutex_lock(&frame_mutex);

	step_tex_circle_draw_order();
	struct tex_circle* ts = tex_circles + (tex_circle_draw_order[sizeof_tex_circles_e -1]);

	ts->pos_x = ((x/(float)g_sc.width)*2)-1;
	ts->pos_y = ((1.0F - (y/(float)g_sc.height))*2)-1;

	struct vertex_rgb* rgb = no_ammo_touch_rgb;

	ts->rgb->r = rgb->r;
	ts->rgb->g = rgb->g;
	ts->rgb->b = rgb->b;

	LOGD("activate_touch_no_ammo", "rgb->r: %f, g: %f, b: %f", rgb->r, rgb->g, rgb->b);

	ts->alpha = 0.0F; // TODO
	ts->scale = TEXC_NO_AMMO_SCALE; // TODO 既に計算すればいいのかも
	ts->scale_change_rate = TEXC_NO_AMMO_SHRINK_RATE;
	ts->alpha_fade_in = TEXC_NO_AMMO_ALPHA_FADE_IN;
	ts->alpha_fade_out = TEXC_NO_AMMO_ALPHA_FADE_OUT;

	ts->alpha_max = ts->scale / 2.0; // TODO
	ts->fading_in = TRUE;
	ts->is_alive = TRUE;

	pthread_mutex_unlock(&frame_mutex);
}

void tex_circle_alpha_size(struct tex_circle* ts) {
	ts->scale -= (float)frame_delta * ts->scale_change_rate; //TEXC_SHRINK_RATE;
	if (ts->fading_in) {
		ts->alpha += (float)frame_delta * ts->alpha_fade_in;
		if (ts->alpha >= ts->alpha_max) ts->fading_in = FALSE;
	}
	if (!ts->fading_in) {
		ts->alpha -= (float)frame_delta * ts->alpha_fade_out;
		if (ts->alpha <= 0) ts->is_alive = FALSE;
	}
}

void tex_ripple_alpha_size(struct tex_circle* tr) {

	tr->scale += (float)frame_delta * tr->scale_change_rate;
	if (tr->fading_in) {
		tr->alpha += (float)frame_delta *  tr->alpha_fade_in;
		if (tr->alpha >= (tr->alpha_max * 0.95)) tr->fading_in = FALSE;
	}
	if (!tr->fading_in) {
		tr->alpha *= tr->alpha_fade_out;
//		ts->alpha *= (frame_delta_ratio * 0.98);
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

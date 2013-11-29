/*
 * gfx_init.h
 *
 *  Created on: 2013/07/02
 *      Author: Michael
 */

#ifndef GFX_INIT_H_
#define GFX_INIT_H_

#include <EGL/egl.h>
#include "gfx_asst.h"

#define TOTAL_PART_COLORS 8



typedef struct {
	EGLNativeWindowType nativeWin;
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
	EGLint majorVersion;
	EGLint minorVersion;
	int width;
	int height;

//	float		display_ratio;
	float hw_ratio;
} screen_settings;

typedef struct {
    GLfloat x, y, z;
    GLfloat r, g, b;
} vertex;

typedef struct {
	GLfloat r, g, b;
} vertex_rgb;


extern screen_settings  g_sc;

extern unsigned long frame_delta;


//EGLBoolean pi_SurfaceCreate(ANativeWindow* nw);
EGLBoolean create_window_surface(ANativeWindow* nw);
void draw_main();
int gles_init();
void calc_frame_delta_time();
void calc_frame_rate();
//void activate_touch_shape(float x, float y, size_t col);
void activate_touch_shape(float x, float y, size_t col, float* vel);
void activate_touch_no_ammo(float x, float y);

void kill_all_touch_shapes();

//size_t cycle_color();

void start_xfade_bgs();
int bg_fading();

void get_start_time();

void get_elapsed_time(unsigned long* t);


void gles_term_display(screen_settings* e);



int create_gl_texture(texture_type *tt);


float gl_to_scr(float gl, int is_x);

#endif /* GFX_INIT_H_ */

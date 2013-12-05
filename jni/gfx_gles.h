/*
 * gfx_init.h
 *
 *  Created on: 2013/07/02
 *      Author: Michael
 */

#ifndef GFX_INIT_H_
#define GFX_INIT_H_

//full_screen* fs;

struct background;
struct full_screen;

#include <EGL/egl.h>
#include <GLES/gl.h>
#include "gfx_asst.h"
#include "hon_type.h"


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


////struct vertextag;
//typedef struct vertex{
//    GLfloat x, y, z;
//    GLfloat r, g, b;
//} vertex;
//
//typedef struct vertex_rgb{
//	GLfloat r, g, b;
//} vertex_rgb;


extern screen_settings  g_sc;

extern unsigned long frame_delta;

//extern int splash_fading_in;
//extern int splash_fading_out;
//
//extern int show_splash;


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
int create_gl_texture(struct texture_type *tt);

void draw_full_screen_image(struct full_screen* fs);
void draw_background(struct background* bg);

float gl_to_scr(float gl, int is_x);

#endif /* GFX_INIT_H_ */

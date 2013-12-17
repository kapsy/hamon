/*
 * gfx_init.h
 *
 *  Created on: 2013/07/02
 *      Author: Michael
 */

#ifndef GFX_INIT_H_
#define GFX_INIT_H_

#include <EGL/egl.h>
#include <GLES/gl.h>


struct texture_type;
struct full_scr_el;
struct background;
struct tex_circle;

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



extern screen_settings  g_sc;

extern unsigned long frame_delta;


EGLBoolean create_window_surface(ANativeWindow* nw);

void draw_frame();
int gles_init();

void gles_term_display(screen_settings* e);
//int create_gl_texture(struct texture_type *tt);
int create_gl_texture(struct texture_type *tt, GLint param);


void draw_full_screen_image(struct full_scr_el* fs);
void draw_background(struct background* bg);

float gl_to_scr(float gl, int is_x);

#endif /* GFX_INIT_H_ */

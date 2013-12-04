/*
 * gfx_bkgd.h
 *
 *  Created on: 2013/12/04
 *      Author: Michael
 */

#ifndef BACKGROUND_H_
#define BACKGROUND_H_

//#include "gfx/fullscreene.h"
//#include "gfx_gles.h"
//#include "gfx/vertex.h"
#include <GLES/gl.h>

#define BG_FADE_RATE (0.11F/(float)SEC_IN_US)
#define BG_PULSE_FADE_RATE (0.21F/(float)SEC_IN_US)



//
//#ifndef _FOO_H_
//#define _FOO_H_ 1
//
//#define FOO_DEF (0xDEADBABE)
//
//struct bar; /* forward declaration, defined in bar.h*/
//
//struct foo {
//  struct bar *bar;
//};
//
//#endif
//
//

//struct vertex_rgb{
//	GLfloat r, g, b;
//};



struct fullscreen;
struct vertex_rgb;


struct background {

	struct full_screen* fs;

	struct vertex_rgb colors[4];

	float pulse;
	float pulse_size;
	float pulse_dir; // ê≥å∑îgÇÃÇŸÇ§Ç™Ç¢Ç¢ÇÃÇ©Ç‡
//	int selected_scale;

};

extern struct background backgrounds[];
//extern int curr_bg;
extern int sizeof_backgrounds_elements; // îzóÒÇÃå¬ëÃÇÃêî
//extern int selected_scale;


void bg_anim_all();
void bg_pulse(struct background* bg);
void bg_xfade();
int bgs_fading();

#endif /* GFX_BKGD_H_ */

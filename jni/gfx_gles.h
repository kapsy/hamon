/*
 * gfx_init.h
 *
 *  Created on: 2013/07/02
 *      Author: Michael
 */

#ifndef GFX_INIT_H_
#define GFX_INIT_H_

#include <EGL/egl.h>


#define TOTAL_PART_COLORS 8

EGLBoolean pi_SurfaceCreate(ANativeWindow* nw);
void pi_draw();
int init_cmds();
void calc_frame_delta_time();
void calc_frame_rate();
void activate_touch_shape(float x, float y, size_t col);
//size_t cycle_color();

#endif /* GFX_INIT_H_ */

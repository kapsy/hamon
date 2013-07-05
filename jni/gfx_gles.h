/*
 * gfx_init.h
 *
 *  Created on: 2013/07/02
 *      Author: Michael
 */

#ifndef GFX_INIT_H_
#define GFX_INIT_H_


#include <EGL/egl.h>

EGLBoolean pi_SurfaceCreate(ANativeWindow* nw);
void pi_draw();
//int init_cmds();

int init_cmds(int* animating);

#endif /* GFX_INIT_H_ */

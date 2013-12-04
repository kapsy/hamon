/*
 * gfx_pics.h
 *
 *  Created on: 2013/12/02
 *      Author: Michael
 */

#ifndef FULLSCREENE_H_
#define FULLSCREENE_H_

//#include "gfx/vertex.h"

#define SPLASH_FADE_RATE (0.4F/(float)SEC_IN_US)
#define HELP_FADE_RATE (0.4F/(float)SEC_IN_US)
#include <GLES/gl.h>


//struct vertex{
//    GLfloat x, y, z;
//    GLfloat r, g, b;
//};

struct texture_file;

// 全画面の絵を定義するため（例：スプラッシュ）
struct full_screen {

	char* title;
	struct texture_file* main_texture;
	float alpha;
	float fade_rate;
	int fading_in;
	int fading_out;
	int is_showing;

};


extern struct vertex fs_quad[];
extern unsigned short fs_quad_index[];
extern struct full_screen screens[];

extern int sizeof_fs_quad_elements;
extern int sizeof_fs_quad;
extern int sizeof_fs_quad_index_elements;
extern int sizeof_fs_quad_index;
extern int sizeof_screens_array;
extern int sizeof_screens;

void fse_anim(struct full_screen* fs);
void fse_alpha_anim(struct full_screen* fs);
int fse_fading(struct full_screen* fs);



#endif /* GFX_PICS_H_ */

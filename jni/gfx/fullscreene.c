/*
 * fullscreene.c FULL SCREEN ELEMENT
 *
 *  Created on: 2013/12/02
 *      Author: Michael
 */


#include "hon_type.h"
//#include "gfx_gles.h"

#include "gfx/fullscreene.h"


struct vertex fs_quad[] = {
	{-1.0f, 	-1.0f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
	{1.0f, 		-1.0f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
	{1.0f, 		1.0f, 		0.0f, 		0.0f, 		0.0f, 		0.0f},
	{-1.0f, 	1.0f, 		0.0f, 		0.0f, 		0.0f,		0.0f},
};
unsigned short fs_quad_index[] = {
  0, 1, 3, 2
};




struct full_screen screens[] = {

		 {"splash", textures + 0, 0.0F, SPLASH_FADE_RATE, TRUE, FALSE, TRUE },
		 {"splash_bg", textures + 1, 0.0F, SPLASH_FADE_RATE, TRUE, FALSE, TRUE },
		 {"help", textures + 0, 0.0F, HELP_FADE_RATE, FALSE, FALSE, FALSE },

};


int sizeof_fs_quad_elements = sizeof fs_quad/sizeof fs_quad[0];
int sizeof_fs_quad = sizeof fs_quad;

int sizeof_fs_quad_index_elements = sizeof fs_quad_index/sizeof fs_quad_index[0];
int sizeof_fs_quad_index = sizeof fs_quad_index;

int sizeof_screens_elements = sizeof screens/sizeof screens[0];
int sizeof_screens = sizeof screens;

void fse_anim(struct full_screen* fs) {

	fse_alpha_anim(fs);
	draw_full_screen_image(fs);
}


void fse_alpha_anim(struct full_screen* fs) {

	if(fs->is_showing) {

		if (fs->fading_in && fs->alpha < 1.0) {
			fs->alpha += (float)frame_delta *  fs->fade_rate;
		} else if (fs->fading_in && fs->alpha >= 1.0) {
			fs->fading_in = FALSE;
			fs->alpha = 1.0;
		}

		if (fs->fading_out && fs->alpha > 0.0) {
			fs->alpha -= (float)frame_delta * fs->fade_rate;
		} else if (fs->fading_out && fs->alpha <= 0.0) {

			fs->fading_out = FALSE;
			fs->alpha = 0.0;
			fs->is_showing = FALSE;
		}
	}
}


int fse_fading(struct full_screen* fs) {
	int r = FALSE;
	if (fs->is_showing) {
		if (fs->fading_in || fs->fading_out) r = TRUE;
		if (!fs->fading_in && !fs->fading_out) r = FALSE;
	}
	return r;
}



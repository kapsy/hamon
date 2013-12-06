/*
 * full_screen_element.c
 *
 *  Created on: 2013/12/05
 *      Author: Michael
 */
#include <android/log.h>
#include <time.h>

#include "gfx/full_screen_element.h"
#include "hon_type.h"
#include "gfx_asst.h"
#include "gfx_gles.h"
#include "game/moods.h"

//struct vertex fs_quad[] = {
//	{-1.0f, 	-1.0f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{1.0f, 		-1.0f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{1.0f, 		1.0f, 		0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{-1.0f, 	1.0f, 		0.0f, 		0.0f, 		0.0f,		0.0f},
//};
//unsigned short fs_quad_index[] = {
//  0, 1, 3, 2
//};









struct full_screen_element screens[] = {

		 {"splash", textures + 0, 0.0F, SPLASH_FADE_RATE, TRUE, FALSE, TRUE },
		 {"splash_bg", textures + 1, 0.0F, SPLASH_FADE_RATE, TRUE, FALSE, TRUE },
		 {"help", textures + 0, 0.0F, HELP_FADE_RATE, FALSE, FALSE, FALSE },

};

//int sizeof_fs_quad_elements = sizeof fs_quad/sizeof fs_quad[0];
//int sizeof_fs_quad = sizeof fs_quad;
//
//int sizeof_fs_quad_index_elements = sizeof fs_quad_index/sizeof fs_quad_index[0];
//int sizeof_fs_quad_index = sizeof fs_quad_index;

int sizeof_screens_elements = sizeof screens/sizeof screens[0];
int sizeof_screens = sizeof screens;

void fse_anim(struct full_screen_element* fs) {

	fse_alpha_anim(fs);
	draw_full_screen_image(fs);
}


void fse_alpha_anim(struct full_screen_element* fs) {

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


int fse_fading(struct full_screen_element* fs) {
	int r = FALSE;
	if (fs->is_showing) {
		if (fs->fading_in || fs->fading_out) r = TRUE;
		if (!fs->fading_in && !fs->fading_out) r = FALSE;
	}
	return r;
}



void bg_anim_all() {
	int i;

	for(i=0; i<sizeof_moods_elements; i++) {

		LOGD("draw_background", "i: %d", i);
		struct background* bg = (moods + i)->bg;
		bg_pulse(bg);
		fse_alpha_anim(bg->fs);
		draw_background(bg);
	}
}

void bg_pulse(struct background* bg) {

	bg->pulse += (float)frame_delta * BG_PULSE_FADE_RATE * bg->pulse_dir;
	if (bg->pulse_dir == 1.0F &&  bg->pulse_dir > 1.0) bg->pulse_dir = -1.0F;
	if (bg->pulse_dir == -1.0F &&  bg->pulse_dir < 0.2) bg->pulse_dir = 1.0F;

}


void bg_xfade() { //ˆê”­‚ÈŠÖ”


	LOGD("bg_xfade", "selected_mood: %d", selected_mood);

	struct background* bg = (moods+selected_mood)->bg;

	bg->fs->fading_out = TRUE;

	if (selected_mood<(sizeof_moods_elements-1)) bg = (moods+(selected_mood + 1))->bg;
	if (selected_mood==(sizeof_moods_elements-1)) bg = (moods+0)->bg;

	bg->fs->fading_in = TRUE;

}

int bgs_fading() {

	int r = FALSE;
	int i;
	for (i=0; i<sizeof_moods_elements; i++) {

		r = fse_fading((moods + i)->bg->fs);
	}

	return r;
}







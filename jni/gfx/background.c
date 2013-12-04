/*
 * gfx_bkgd.c
 *
 *  Created on: 2013/12/04
 *      Author: Michael
 */


#include "gfx/background.h"
#include "hon_type.h"
//#include "moods.h"


//int curr_bg = 0;
//int selected_scale = 0;
//int sizeof_backgrounds_elements = sizeof backgrounds/sizeof backgrounds[0]; // îzóÒÇÃå¬ëÃÇÃêî


void bg_anim_all() {
	int i;

	for(i=0; i<sizeof_moods_elements; i++) {
		struct background* bg = (moods + i)->bg;
		bg_pulse(bg);
		fse_alpha_anim(bg->fs);
		draw_background(bg);
	}
}

void bg_pulse(struct background* bg) {

	bg->pulse += (float)frame_delta * BG_PULSE_FADE_RATE * bg->pulse_dir;
	if (bg->pulse_dir == 1.0F && bg_pulse > 1.0) bg->pulse_dir = -1.0F;
	if (bg->pulse_dir == -1.0F && bg_pulse < 0.2) bg->pulse_dir = 1.0F;

}


void bg_xfade() { //àÍî≠Ç»ä÷êî

//	LOGD("start_xfade_bgs", "curr_bg: %d", curr_bg);
//	bg_def* bg = bgs + curr_bg;
//	bg->fading_out = TRUE;
//
//	if (curr_bg < bgs_size) curr_bg++;
//	if (curr_bg == bgs_size) curr_bg = 0;
//
//	bg = bgs + curr_bg;
//	bg->selected_scale = selected_scale;
//	bg->fading_in = TRUE;
//



	//-------

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

		r = is_screen_fading((moods + i)->bg->fs);
	}

	return r;
}







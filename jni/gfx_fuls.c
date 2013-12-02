/*
 * gfx_fuls.c
 *
 *  Created on: 2013/12/02
 *      Author: Michael
 */


#include "hon_type.h"
#include "gfx_gles.h"

#include "gfx_fuls.h"


full_screen screens[] = {

		 {"splash", textures + 0, 0.0F, SPLASH_FADE_RATE, TRUE, FALSE, TRUE },
		 {"help", textures + 1, 0.0F, HELP_FADE_RATE, FALSE, FALSE, FALSE },

};


int sizeof_screens_elements = sizeof screens/sizeof screens[0];
int sizeof_screens = sizeof screens;

void screen_anim(full_screen* fs) {









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

		draw_full_screen_image(fs);


	}

}

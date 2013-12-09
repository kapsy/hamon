/*
 * full_screen_element.h
 *
 *  Created on: 2013/12/05
 *      Author: Michael
 */

#ifndef FULL_SCREEN_ELEMENT_H_
#define FULL_SCREEN_ELEMENT_H_

#define SPLASH_FADE_RATE (0.4F/(float)SEC_IN_US)
#define HELP_FADE_RATE (0.4F/(float)SEC_IN_US)
#define BG_FADE_RATE (0.11F/(float)SEC_IN_US)
#define BG_PULSE_FADE_RATE (0.41F/(float)SEC_IN_US)
//#define BG_PULSE_FADE_RATE (0.21F/(float)SEC_IN_US)



struct texture_file;
struct vertex_rgb;


struct full_scr_el {

	char* title;
	struct texture_file* main_texture;
	float alpha;
	float fade_rate;
	int fading_in;
	int fading_out;
	int is_showing;

//	struct vertex_rgb* colors;

	float pulse;
	float pulse_size;
	float pulse_dir; // 正弦波のほうがいいのかも

};


extern struct full_scr_el screens[];
extern struct full_scr_el backgrounds[];

extern int sizeof_backgrounds_elements;
extern int sizeof_backgrounds;
extern int selected_background;

void full_scr_anim(struct full_scr_el* fs);
void full_scr_alpha_anim(struct full_scr_el* fs);
int full_scr_fading(struct full_scr_el* fs);
int all_bgs_fading();


void full_scr_mod(struct full_scr_el* fs);
void full_scr_xfade();
int full_scr_fading();

void background_anim_all();


#endif /* FULL_SCREEN_ELEMENT_H_ */

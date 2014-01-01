/*
 * full_screen_element.h
 *
 *  Created on: 2013/12/05
 *      Author: Michael
 */

#ifndef FULL_SCREEN_ELEMENT_H_
#define FULL_SCREEN_ELEMENT_H_

#define SPLASH_FADE_RATE (0.4F/(float)SEC_IN_US)
#define SPLASH_BG_FADE_RATE (0.36F/(float)SEC_IN_US) //(0.1F (0.3F
#define SPLASH_FADE_RATE_QUICK (0.42F/(float)SEC_IN_US)
#define HELP_FADE_RATE (0.8F/(float)SEC_IN_US)
#define BG_FADE_RATE (0.11F/(float)SEC_IN_US)
#define BG_PULSE_FADE_RATE (0.41F/(float)SEC_IN_US)
//#define BG_PULSE_FADE_RATE (0.21F/(float)SEC_IN_US)

#define DELAY_BEFORE_SPLASH (1 * SEC_IN_US)
#define DELAY_BEFORE_SPLASH_BG (2.3 * SEC_IN_US)
#define SPLASH_COUNT_SECS 10

struct texture_file;
struct vertex_rgb;
struct modulator;

struct full_scr_el {

	char* title;
	struct texture_file* main_texture;


	float alpha;

	float alpha_mod;

	float fade_rate;

	int fading_in;
	int fading_out;
	int is_showing;


	struct modulator* mod_a;
	float pulse;
	float pulse_size;
	float pulse_dir; // 正弦波のほうがいいのかも

	func_ptr fade_in_start;
	func_ptr fade_out_end;

	int color_index;

};


extern struct full_scr_el screens[];
extern struct full_scr_el backgrounds[];

extern int sizeof_backgrounds_elements;
extern int sizeof_backgrounds;
extern int selected_background;

void full_scr_anim(struct full_scr_el* fs);
void full_scr_alpha_anim(struct full_scr_el* fs);

void full_scr_alpha_mod (struct full_scr_el* fs);
int full_scr_fading(struct full_scr_el* fs);
int all_bgs_fading();


void full_scr_mod(struct full_scr_el* fs);
void full_scr_xfade();
int full_scr_fading();

void background_anim_all();
void help_screen_end();

#endif /* FULL_SCREEN_ELEMENT_H_ */

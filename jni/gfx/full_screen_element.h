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
#define BG_PULSE_FADE_RATE (0.21F/(float)SEC_IN_US)



struct texture_file;

struct full_screen_element {

	char* title;
	struct texture_file* main_texture;
	float alpha;
	float fade_rate;
	int fading_in;
	int fading_out;
	int is_showing;

};
struct vertex_rgb;



struct background {

	struct full_screen_element* fs;

//	struct vertex_rgb* colors[4];
	struct vertex_rgb* colors[4];

	float pulse;
	float pulse_size;
	float pulse_dir; // ê≥å∑îgÇÃÇŸÇ§Ç™Ç¢Ç¢ÇÃÇ©Ç‡
//	int selected_scale;

};



extern int sizeof_backgrounds_elements; // îzóÒÇÃå¬ëÃÇÃêî




//extern struct vertex fs_quad[];
//extern unsigned short fs_quad_index[];


extern struct full_screen_element screens[];


//extern int sizeof_fs_quad_elements;
//extern int sizeof_fs_quad;
//extern int sizeof_fs_quad_index_elements;
//extern int sizeof_fs_quad_index;


extern int sizeof_screens_array;
extern int sizeof_screens;


void fse_anim(struct full_screen_element* fs);
void fse_alpha_anim(struct full_screen_element* fs);
int fse_fading(struct full_screen_element* fs);


void bg_anim_all();
void bg_pulse(struct background* bg);
void bg_xfade();
int bgs_fading();

#endif /* FULL_SCREEN_ELEMENT_H_ */

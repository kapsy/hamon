/*
 * moods.c
 *
 *  Created on: 2013/12/04
 *      Author: Michael
 */

#include <android/log.h>
#include <time.h>
#include "game\moods.h"
#include "snd_asst.h"
#include "hon_type.h"
#include "snd_scal.h"
#include "gfx/full_screen_element.h"
//#include "gfx_gles.h"

#include "gfx/vertex.h"





struct vertex_rgb mood_colors[5][4] = {
		{
			{0.0f, 		0.0f, 		0.0f},
			{0.0f, 		0.0f, 		0.0f},
			{0.95f,	0.0f, 		0.35f},
			{0.25f,	0.0f, 		0.25f}
		},
		{
			{0.0f, 		0.0f, 		0.0f},
			{1.0f, 		1.0f, 		1.0f},
			{1.0f, 		1.0f, 		0.35f},
			{0.8f, 		1.0f, 		1.0f}
		},
		{
			{0.2f, 		0.3f, 		0.0f},
			{0.2f, 		0.3f, 		0.0f},
			{1.0f, 		0.0f, 		0.0f},
			{1.0f, 		0.0f, 		0.0f}
		},
		{
			{0.0f, 	 	0.1f, 		0.0f},
			{0.0f, 		0.6f, 		0.0f},
			{0.0f, 		0.2f, 		0.35f},
			{0.0f, 		0.2f, 		0.05f}
		},
		{
			{0.0f, 	 	0.5f, 		0.0f},
			{0.0f, 		0.3f, 		0.0f},
			{1.0f, 		0.2f, 		0.3f},
			{0.0f, 		1.0f, 		1.0f}
		}
};

//gathered_t all = {
//  .both_ptr = &(both_t){
//     .a_ptr = &a,
//     .b_ptr = &b,
//   }
//};

int sizeof_mood_colors_set = sizeof mood_colors/ (sizeof mood_colors/sizeof mood_colors[0]);

//struct full_screen_element fses[] = {
//		{
//				.title = "bg1",
//				.main_texture = NULL,
//				.alpha = 0.0,
//				.fade_rate = BG_FADE_RATE,
//				.fading_in = TRUE,
//				.fading_out = FALSE,
//				.is_showing = TRUE,
//		},
//		{
//				.title = "bg2",
//				.main_texture = NULL,
//				.alpha = 0.0,
//				.fade_rate = BG_FADE_RATE,
//				.fading_in = FALSE,
//				.fading_out = FALSE,
//				.is_showing = FALSE,
//		},
//
//};
//
//
//
//
//struct background backgroundsold[] = {
//
//
//		{
//				.fs = fses + 0,
//				.colors = (mood_colors + 0),
//				.pulse = 0.0,
//				.pulse_size = 1.0,
//				.pulse_dir = 1.0,
//		},
//		{
//				.fs = fses + 1,
//				.colors = (mood_colors + 1),
//				.pulse = 0.0,
//				.pulse_size = 1.0,
//				.pulse_dir = 1.0,
//		}
//};

//struct background backgrounds[] = {
//
//
//		{
//				.fs = &(struct full_screen_element) {
//										.title = "bg1",
//										.main_texture = NULL,
//										.alpha = 0.0,
//										.fade_rate = BG_FADE_RATE,
//										.fading_in = TRUE,
//										.fading_out = FALSE,
//										.is_showing= FALSE,
//								},
//				.colors = (mood_colors + 0),
//				.pulse = 0.0,
//				.pulse_size = 1.0,
//				.pulse_dir = 1.0,
//		},
//		{
//				.fs = &(struct full_screen_element) {
//										.title = "bg2",
//										.main_texture = NULL,
//										.alpha = 0.0,
//										.fade_rate = BG_FADE_RATE,
//										.fading_in = FALSE,
//										.fading_out = FALSE,
//										.is_showing= FALSE,
//								},
//				.colors = (mood_colors + 1),
//				.pulse = 0.0,
//				.pulse_size = 1.0,
//				.pulse_dir = 1.0,
//		}
//};

struct scale scales[] = {

	{
		.midimap = { 48, 48, 50, 50, 52, 53, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83 },
		.looping_sample = (looping_samples + 0),
	},
	{
		.midimap = { 49, 49, 51, 51, 54, 54, 56, 56, 58, 58, 61, 61, 63, 63, 66, 66, 68, 68, 70, 73, 75, 78, 80, 82 },
		.looping_sample = (looping_samples + 1),
	}

};


struct mood moods[] = {

		{
				"major", (mood_colors + 0), 0, (scales + 0)
		},
		{
				"minor", (mood_colors + 1), 1, (scales + 1)
		}

};




int selected_mood = 0;
int sizeof_moods_elements = sizeof moods/sizeof moods[0];

int init_mood() {

//	int success = enqueue_seamless_loop(looping_samples + selected_scale);
	int i = enqueue_seamless_loop((moods+selected_mood)->scale->looping_sample);
	return i;
}

int cycle_mood() {

	// ここでフェードをしてないかを確認しなきゃ
	if(current_voice_fading() || all_bgs_fading()) {
		LOGD("cycle_mood", "current_voice_fading() || bg_fading()");
		return 0;
	}



	full_scr_xfade();


	if (selected_mood < sizeof_moods_elements) selected_mood += 1;
	if (selected_mood == sizeof_moods_elements) selected_mood = 0;

	LOGD("cycle_mood", "debug c, selected_mood: %d", selected_mood);
	int success = enqueue_seamless_loop((moods+selected_mood)->scale->looping_sample);
	LOGD("cycle_mood", "debug d");
//	start_xfade_bgs(); // graphics background crossfade
	return 1;
}



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

			{1.0f, 		0.0f, 		0.0f},
			{1.0f, 		0.0f, 		0.0f},
			{1.0f, 		0.0f, 		0.35f},
			{1.0f, 		0.0f, 		0.25f}
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
//typedef struct {
//  my_type1_t* a_ptr ;
//  my_type2_t* b_ptr ;
//} both_t ;
//
//typedef struct {
//  both_t* both_ptr ;
//} gathered_t

//gathered_t all = {
//  .both_ptr = &(both_t){
//     .a_ptr = &a,
//     .b_ptr = &b,
//   }
//};

struct mood moods[1] = {

		{
				.title = "major",
				.bg = &(struct background) {
					.fs = &(struct full_screen_element) {
							.title = "bg1",
							.main_texture = NULL,
							.alpha = 0.0,
							.fade_rate = BG_FADE_RATE,
							.fading_in = TRUE,
							.fading_out = FALSE,
							.is_showing= FALSE,
					},
					.colors = (mood_colors + 0),
					.pulse = 0.0,
					.pulse_size = 1.0,
					.pulse_dir = 1.0,

//					"bg1", NULL, 0.0, BG_FADE_RATE, TRUE, FALSE, FALSE,	mood_colors + 0, 0.0, 1.0, 1.0,
				},

				.scale = &(struct scale) {
					.midimap = {48, 48, 50, 50, 52, 53, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83},
					.looping_sample = (looping_samples+0),
				}
		},
		{
				.title = "major2",
				.bg = &(struct background) {
					.fs = &(struct full_screen_element) {
							.title = "bg1",
							.main_texture = NULL,
							.alpha = 0.0,
							.fade_rate = BG_FADE_RATE,
							.fading_in = TRUE,
							.fading_out = FALSE,
							.is_showing= FALSE,
					},
					.colors = (mood_colors + 1),
					.pulse = 0.0,
					.pulse_size = 1.0,
					.pulse_dir = 1.0,

//					"bg1", NULL, 0.0, BG_FADE_RATE, TRUE, FALSE, FALSE,	mood_colors + 0, 0.0, 1.0, 1.0,
				},

				.scale = &(struct scale) {
					.midimap = {48, 48, 50, 50, 52, 53, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83},
					.looping_sample = (looping_samples+0),
				}
		}
};













//
//
//struct mood moods[5] = {
//
//		{
//				"major",
//				{
//						"bg1", NULL, 0.0, BG_FADE_RATE, TRUE, FALSE, FALSE,
//						mood_colors + 0,
//						0.0, 1.0, 1.0
//				},
//				{
//								48, 48, 50, 50, 52, 53, 53, 55,
//								57, 59, 60, 62, 64, 65, 67, 69,
//								71, 72, 74, 76, 77, 79, 81, 83,
//										(looping_samples+0)
//				}
//		},
//
//		{
//				"minor",
//				{
//						"bg2", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE,
//						mood_colors + 1,
//						0.0, 1.0, 1.0
//				},
//				{
//								49, 49, 51, 51, 54, 54, 56, 56,
//								58, 58, 61, 61, 63, 63, 66, 66,
//								68, 68, 70, 73, 75, 78, 80, 82,
//							(looping_samples+1)
//				},
//		},
//
//		{
//				"cirrostratus",
//				{
//						"bg3", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE,
//						mood_colors + 2,
//						0.0, 1.0, 1.0
//				},
//				{
//								48, 48, 49, 49, 51, 51, 53, 53,
//								54, 54, 58, 58, 60, 61, 63, 65,
//								66, 70, 72, 73, 75, 77, 78, 82,
//						(looping_samples+2)
//				},
//		},
//
//		{
//				"cumulonimbus",
//				{
//						"bg4", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE,
//						mood_colors + 3,
//						0.0, 1.0, 1.0
//				},
//				{
//								48, 48, 51, 51, 53, 53, 55, 55,
//								57, 57, 58, 58, 60, 63, 65, 67,
//								69, 70, 72, 75, 77, 79, 80, 82,
//						(looping_samples+3)
//				},
//		},
//
//		{
//			 "cirrostratus tense",
//				{
//						"bg5", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE,
//						mood_colors + 4,
//						0.0, 1.0, 1.0
//				},
//				{
//								48, 48, 49, 49, 51, 51, 53, 53,
//								54, 54, 58, 58, 60, 61, 63, 65,
//								66, 70, 72, 73, 75, 77, 78, 82,
//							(looping_samples+4)
//				}
//		}
//};
//





/*
struct mood moods[5] = {

		{
				"major",
				{
						{"bg1", NULL, 0.0, BG_FADE_RATE, TRUE, FALSE, FALSE},
						mood_colors + 0,
						0.0, 1.0, 1.0
				},
				{
						{
								48, 48, 50, 50, 52, 53, 53, 55,
								57, 59, 60, 62, 64, 65, 67, 69,
								71, 72, 74, 76, 77, 79, 81, 83
						},
						(looping_samples+0)
				}
		},

		{
				"minor",
				{
						{"bg2", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE},
						mood_colors + 1,
						0.0, 1.0, 1.0
				},
				{
						{
								49, 49, 51, 51, 54, 54, 56, 56,
								58, 58, 61, 61, 63, 63, 66, 66,
								68, 68, 70, 73, 75, 78, 80, 82
						},
						(looping_samples+1)
				},
		},

		{
				"cirrostratus",
				{
						{"bg3", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE},
						mood_colors + 2,
						0.0, 1.0, 1.0
				},
				{
						{
								48, 48, 49, 49, 51, 51, 53, 53,
								54, 54, 58, 58, 60, 61, 63, 65,
								66, 70, 72, 73, 75, 77, 78, 82
						},
						(looping_samples+2)
				},
		},

		{
				"cumulonimbus",
				{
						{"bg4", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE},
						mood_colors + 3,
						0.0, 1.0, 1.0
				},
				{
						{
								48, 48, 51, 51, 53, 53, 55, 55,
								57, 57, 58, 58, 60, 63, 65, 67,
								69, 70, 72, 75, 77, 79, 80, 82
						},
						(looping_samples+3)
				},
		},

		{
			 "cirrostratus tense",
				{
						{"bg5", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE},
						mood_colors + 4,
						0.0, 1.0, 1.0
				},
				{
						{
								48, 48, 49, 49, 51, 51, 53, 53,
								54, 54, 58, 58, 60, 61, 63, 65,
								66, 70, 72, 73, 75, 77, 78, 82
						},
						(looping_samples+4)
				}
		}

};

*/








/*
struct mood moods[5] = {

		{
				"major",
				{
						{"bg1", NULL, 0.0, BG_FADE_RATE, TRUE, FALSE, FALSE},
						{
								{0.0f, 		0.0f, 		0.0f},
								{0.0f, 		0.0f, 		0.0f},
								{0.95f,	0.0f, 		0.35f},
								{0.25f,	0.0f, 		0.25f}
						},
						0.0, 1.0, 1.0
				},
				{
						{
								48, 48, 50, 50, 52, 53, 53, 55,
								57, 59, 60, 62, 64, 65, 67, 69,
								71, 72, 74, 76, 77, 79, 81, 83
						},
						(looping_samples+0)
				}
		},

		{
				"minor",
				{
						{"bg2", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE},
						{
								{1.0f, 		0.0f, 		0.0f},
								{1.0f, 		0.0f, 		0.0f},
								{1.0f, 		0.0f, 		0.35f},
								{1.0f, 		0.0f, 		0.25f}
						},
						0.0, 1.0, 1.0
				},
				{
						{
								49, 49, 51, 51, 54, 54, 56, 56,
								58, 58, 61, 61, 63, 63, 66, 66,
								68, 68, 70, 73, 75, 78, 80, 82
						},
						(looping_samples+1)
				},
		},

		{
				"cirrostratus",
				{
						{"bg3", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE},
						{
								{0.2f, 		0.3f, 		0.0f},
								{0.2f, 		0.3f, 		0.0f},
								{1.0f, 		0.0f, 		0.0f},
								{1.0f, 		0.0f, 		0.0f}
						},
						0.0, 1.0, 1.0
				},
				{
						{
								48, 48, 49, 49, 51, 51, 53, 53,
								54, 54, 58, 58, 60, 61, 63, 65,
								66, 70, 72, 73, 75, 77, 78, 82
						},
						(looping_samples+2)
				},
		},

		{
				"cumulonimbus",
				{
						{"bg4", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE},
						{
								{0.0f, 	 	0.1f, 		0.0f},
								{0.0f, 		0.6f, 		0.0f},
								{0.0f, 		0.2f, 		0.35f},
								{0.0f, 		0.2f, 		0.05f}
						},
						0.0, 1.0, 1.0
				},
				{
						{
								48, 48, 51, 51, 53, 53, 55, 55,
								57, 57, 58, 58, 60, 63, 65, 67,
								69, 70, 72, 75, 77, 79, 80, 82
						},
						(looping_samples+3)
				},
		},

		{
			 "cirrostratus tense",
				{
						{"bg5", NULL, 0.0, BG_FADE_RATE, FALSE, FALSE, FALSE},
						{
								{0.0f, 	 	0.5f, 		0.0f},
								{0.0f, 		0.3f, 		0.0f},
								{1.0f, 		0.2f, 		0.3f},
								{0.0f, 		1.0f, 		1.0f}
						},
						0.0, 1.0, 1.0
				},
				{
						{
								48, 48, 49, 49, 51, 51, 53, 53,
								54, 54, 58, 58, 60, 61, 63, 65,
								66, 70, 72, 73, 75, 77, 78, 82
						},
						(looping_samples+4)
				}
		}

};
*/

int selected_mood = 0;
int sizeof_moods_elements = sizeof moods/sizeof moods[0];

int init_mood() {

//	int success = enqueue_seamless_loop(looping_samples + selected_scale);
	int i = enqueue_seamless_loop((moods+selected_mood)->scale->looping_sample);
	return i;
}

int cycle_mood() {

	// ここでフェードをしてないかを確認しなきゃ
	if(current_voice_fading() || bgs_fading()) {
		LOGD("cycle_mood", "current_voice_fading() || bg_fading()");
		return 0;
	}


	if (selected_mood < sizeof_moods_elements) selected_mood += 1;
	if (selected_mood == sizeof_moods_elements) selected_mood = 0;

	LOGD("cycle_mood", "debug c, selected_scale: %d", selected_mood);
	int success = enqueue_seamless_loop((moods+selected_mood)->scale->looping_sample);
	LOGD("cycle_mood", "debug d");
//	start_xfade_bgs(); // graphics background crossfade
	bg_xfade();
	return 1;
}

struct sample_def* get_scale_sample(int seg) {

	int sample = moods->scale->midimap[seg];
	sample -= START_NOTE;

	return (oneshot_samples + sample);

}


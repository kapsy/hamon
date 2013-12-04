/*
 * moods.c
 *
 *  Created on: 2013/12/04
 *      Author: Michael
 */


#include "game\moods.h"
//#include "snd_asst.h"
//#include "gfx_bkgd.h"
#include "hon_type.h"
#include "background.h"
#include "snd_scal.h"


struct mood moods[] = {

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
		LOGD("cycle_scale", "current_voice_fading() || bg_fading()");
		return 0;
	}


	if (selected_mood < sizeof_moods_elements) selected_mood += 1;
	if (selected_mood == sizeof_moods_elements) selected_mood = 0;

	LOGD("cycle_scale", "debug c, selected_scale: %d", selected_mood);
	int success = enqueue_seamless_loop((moods+selected_mood)->scale->looping_sample);
	LOGD("cycle_scale", "debug d");
	start_xfade_bgs(); // graphics background crossfade

	return 1;
}

struct sample_def* get_scale_sample(int seg) {

	int sample = moods->scale->midimap[seg];
	sample -= START_NOTE;

	return (oneshot_samples + sample);

}


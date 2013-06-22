/*
 * scales.c
 *
 *  Created on: 2013/06/02
 *      Author: Michael
 */

// 音調の情報を方損するため

#include "snd_asst.h"
#include "snd_sles.h"
#include <android/log.h>

#include "snd_ctrl.h"

//C2-B4
//48 - 83

#define TOTAL_NOTES 24
#define START_NOTE 48// MIDI番号

int selected_scale = 0;
extern sample_def oneshot_samples[];

typedef struct {

	//int id; //バッファーのため
	char* name;

	// 画面の部分をMIDIノートへ
	int midimap[24];
	int chord_loop; // 他の方法があるはずだ

} scale_def;


// 今考えれば一番簡単な方法というのは
// 空白なところは欲しければ、同じノートを入れても構いません。

// この配列の中にループしている音の情報を入るべき！
scale_def scales[] = {

//		{ "test scale",
//				{
//						48, 48, 48, 48, 48, 48, 48, 48,
//						49, 49, 49, 49, 49, 49, 49, 49,
//						50, 50, 50, 50, 50, 50, 50, 50
//				},
//				1
//		},

		{ "major",
				{
						48, 48, 50, 50, 52, 53, 53, 55,
						57, 59, 60, 62, 64, 65, 67, 69,
						71, 72, 74, 76, 77, 79, 81, 83
				},
				0
		},

		{ "minor",
				{
						49, 49, 51, 51, 54, 54, 56, 56,
						58, 58, 61, 61, 63, 63, 66, 66,
						68, 68, 70, 73, 75, 78, 80, 82
				},
				1
		},

		{ "cumulonimbus",
				{
						48, 48, 51, 51, 53, 53, 55, 55,
						57, 57, 58, 58, 60, 63, 65, 67,
						69, 70, 72, 75, 77, 79, 80, 82
				},
				2
		},

		{ "cirrostratus",
				{
						48, 48, 49, 49, 51, 51, 53, 53,
						54, 54, 58, 58, 60, 61, 63, 65,
						66, 70, 72, 73, 75, 77, 78, 82
				},
				3
	}
};


// この関数は必要ないかも
// 初期化をするためだけの関数
void play_loop() {
	int success = enqueue_seamless_loop(looping_samples + selected_scale);
}

void play_note(int segment, float vel) {


		int sample = scales[selected_scale].midimap[segment];
		sample -= START_NOTE;

		int success = enqueue_one_shot(oneshot_samples + sample, vel);

		__android_log_print(ANDROID_LOG_DEBUG, "play_note", "vel: %f", vel);

}

int cycle_scale() {

	// ここでフェードをしてないかを確認しなきゃ
	if(current_voice_fading()) {

		__android_log_write(ANDROID_LOG_DEBUG, "cycle_scale", "current_voice_fading()");
		return 0;
	}

	__android_log_write(ANDROID_LOG_DEBUG, "cycle_scale", "cycle_scale() called");
	int total_scales = sizeof scales / sizeof scales[0];

	if (selected_scale < total_scales)
		selected_scale += 1;

	if (selected_scale == total_scales)
		selected_scale = 0;

	int success = enqueue_seamless_loop(looping_samples + selected_scale);


	return 1;
}

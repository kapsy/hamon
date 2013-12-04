/*
 * scales.c
 *
 *  Created on: 2013/06/02
 *      Author: Michael
 */

// 音調の情報を方損するため

//#include "snd_asst.h"
//#include "snd_sles.h"
//#include <android/log.h>
//
//#include "snd_ctrl.h"
//#include "snd_scal.h"
//#include "hon_type.h"
//C2-B4
//48 - 83



//int selected_scale = 0;
//extern sample_def oneshot_samples[];

//typedef struct {
//
//	//int id; //バッファーのため
//	char* name;
//
//	// 画面の部分をMIDIノートへ
//	int midimap[TOTAL_NOTES];
//	int chord_loop; // 他の方法があるはずだ
//
//} scale_def;


// 今考えれば一番簡単な方法というのは
// 空白なところは欲しければ、同じノートを入れても構いません。

// この配列の中にループしている音の情報を入るべき！
//scale_def scales[] = {
//
//		{ "major",
//				{
//						48, 48, 50, 50, 52, 53, 53, 55,
//						57, 59, 60, 62, 64, 65, 67, 69,
//						71, 72, 74, 76, 77, 79, 81, 83
//				},
//				0
//		},
//
//		{ "minor",
//				{
//						49, 49, 51, 51, 54, 54, 56, 56,
//						58, 58, 61, 61, 63, 63, 66, 66,
//						68, 68, 70, 73, 75, 78, 80, 82
//				},
//				1
//		},
//
//		{ "cirrostratus",
//				{
//						48, 48, 49, 49, 51, 51, 53, 53,
//						54, 54, 58, 58, 60, 61, 63, 65,
//						66, 70, 72, 73, 75, 77, 78, 82
//				},
//				2
//		},
//
//		{ "cumulonimbus",
//				{
//						48, 48, 51, 51, 53, 53, 55, 55,
//						57, 57, 58, 58, 60, 63, 65, 67,
//						69, 70, 72, 75, 77, 79, 80, 82
//				},
//				3
//		},
//
//		{ "cirrostratus tense",
//				{
//						48, 48, 49, 49, 51, 51, 53, 53,
//						54, 54, 58, 58, 60, 61, 63, 65,
//						66, 70, 72, 73, 75, 77, 78, 82
//				},
//				4
//		}
//
//
//
//};


// この関数は必要ないかも
// 初期化をするためだけの関数
//void start_loop() {
//	int success = enqueue_seamless_loop(looping_samples + selected_scale);
//}

//void play_note(int segment, float vel) {
//
//
//		int sample = scales[selected_scale].midimap[segment];
//		sample -= START_NOTE;
//
//		int success = enqueue_one_shot(oneshot_samples + sample, vel, segment);
//
//		__android_log_print(ANDROID_LOG_DEBUG, "play_note", "vel: %f", vel);
//
//}

//sample_def* get_scale_sample(int seg) {
//
//	int sample = scales[selected_scale].midimap[seg];
//	sample -= START_NOTE;
//
//	return (oneshot_samples + sample);
//
//}




//int cycle_scale() {
//
//	// ここでフェードをしてないかを確認しなきゃ
//	if(current_voice_fading() || bg_fading()) {
//
//		LOGD("cycle_scale", "current_voice_fading() || bg_fading()");
//		return 0;
//	}
//
//	LOGD("cycle_scale", "debug a");
//
//	if (selected_scale < TOTAL_SCALES)
//		selected_scale += 1;
//
//	LOGD("cycle_scale", "debug b");
//	if (selected_scale == TOTAL_SCALES)
//		selected_scale = 0;
//
//	LOGD("cycle_scale", "debug c, selected_scale: %d", selected_scale);
//	int success = enqueue_seamless_loop(looping_samples + selected_scale);
//	LOGD("cycle_scale", "debug d");
//	start_xfade_bgs(); // graphics background crossfade
//
//	return 1;
//}

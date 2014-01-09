// snd_scal.c

// 音調の情報を方損するため
#include <android/asset_manager.h>
#include <android/storage_manager.h>

#include "gfx/vertex.h"
#include "snd_asst.h"
#include "snd_scal.h"
#include "game/moods.h"
// C2-B4
// 48 - 83

// この関数は必要ないかも
// 初期化をするためだけの関数
void start_loop() {
	int success = enqueue_seamless_loop(looping_samples + selected_mood);
}

struct sample_def* get_scale_sample(int seg) {

	int sample = (moods+selected_mood)->scale->midimap[seg];
	sample -= START_NOTE;

	return (oneshot_samples + sample);
}

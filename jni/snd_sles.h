// OpenSL ES management




#ifndef SND_SLES_H_
#define SND_SLES_H_

#include "snd_asst.h"
#include "snd_scal.h"


// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
//void load_all_buffers(AAssetManager* mgr);

typedef struct {
	SLObjectItf bqPlayerObject;
	SLPlayItf bqPlayerPlay;
	SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
	SLVolumeItf bqPlayerVolume;
	SLBufferQueueState bqstate;

	size_t current_chunk;
	sample_def* sample;
	int is_playing;
	int timing_test_index;

	// automation
	//int is_fading;

	SLmillibel sl_volume;

	int fading_in;
	int fading_out;

	// Ç±ÇÃílÇ¡ÇƒÇ«Ç§Ç¢Ç§à”ñ°ÅH
	float vol_fade_factor;
//	float vol_auto_factor;

} voice;

void create_sl_engine();
void init_all_voices();


int enqueue_seamless_loop(sample_def * samp);
//int enqueue_one_shot(sample_def * samp, float vel);
int enqueue_one_shot(sample_def * s, SLmillibel vol, SLpermille pan);



SLmillibel float_to_slmillibel(float sender_vel, float sender_range);
SLpermille get_seg_permille(size_t seg);

//init_seg_pan_map();


//void* loop_fade_in(void* args);
void vol_automation();

int current_voice_fading();

//void all_voices_fade_out_exit();
//void quick_fade_on_exit();
//void shutdown_audio();

void pause_all_voices();



int total_tic_counter;

#endif /* SND_SLES_H_ */

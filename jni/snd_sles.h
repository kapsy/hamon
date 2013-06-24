// OpenSL ES management




#ifndef SND_SLES_H_
#define SND_SLES_H_

#include "snd_asst.h"


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


	int fading_out;
	int fading_in;
	SLmillibel sl_volume;



	float vol_fade_factor;
	float vol_auto_factor;



} voice;

void create_sl_engine();
void init_all_voices();

void set_voice_volume(voice* voice, float vol);

int enqueue_seamless_loop(sample_def * samp);
int enqueue_one_shot(sample_def * samp, float vel);


//void* loop_fade_in(void* args);
void vol_automation();

int current_voice_fading();

void shutdown_audio();

int total_tic_counter;

#endif /* SND_SLES_H_ */

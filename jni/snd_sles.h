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
	int is_fading;

} voice;

void create_sl_engine();
void init_all_voices();

SLmillibel float_to_slmillibel(float sender_vel, float sender_range);
void set_voice_volume(voice* voice, float vol);

int enqueue_seamless_loop(oneshot_def * samp);
int enqueue_one_shot(oneshot_def * samp, float vel);


void* loop_fade_out(void* args);
void* loop_fade_in(void* args);

int current_voice_fading();

void shutdown_audio();

#endif /* SND_SLES_H_ */

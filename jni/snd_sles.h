//snd_sles.h

#ifndef SND_SLES_H_
#define SND_SLES_H_

struct sample_def;

typedef struct {

	SLObjectItf bqPlayerObject;
	SLPlayItf bqPlayerPlay;
	SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
	SLVolumeItf bqPlayerVolume;
	SLBufferQueueState bqstate;

	size_t current_chunk;
	struct sample_def* sample;
	int is_playing;
	int timing_test_index;

	SLmillibel sl_volume;

	int fading_in;
	int fading_out;

	// used as a factor for setting volume
	// should probably be renamed to vol_factor
	float vol_fade_factor;

} voice;

void create_sl_engine();
void init_all_voices();

int enqueue_seamless_loop(struct sample_def * samp);
int enqueue_one_shot(struct sample_def * s, SLmillibel vol, SLpermille pan);

SLmillibel float_to_slmillibel(float sender_vel, float sender_range);
SLpermille get_seg_permille(size_t seg);

void vol_automation();
int current_voice_fading();
void pause_all_voices();

#endif /* SND_SLES_H_ */

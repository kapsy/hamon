// snd_sles.c

#include "common.h"

#include "snd_sles.h"
#include "hon_type.h"
#include "snd_ctrl.h"
#include "snd_scal.h"
#include "snd_asst.h"
#include "gfx/vertex.h"
#include "gfx_butn.h"

#define VOICE_COUNT 20
#define LOOPER_COUNT 2
#define SLMILLIBEL_MIN -5000
#define SLMILLIBEL_MAX 0
#define VEL_SLMILLIBEL_MIN -3000
#define VEL_SLMILLIBEL_RANGE 3000
#define PAN_RANGE 1000
#define SEG_PAN_AMT 30
#define FADE_FACTOR_CHG_RATE 0.03F
#define FADE_FACTOR_CHG_RATE_QUICK 0.1F
#define MASTER_VOLUME 1.0f
#define VEL_SLMILLIBEL_OFFSET -700

void cycle_looping_voice();
void cycle_oneshot_voice();
voice* get_next_free_voice();
voice* get_oldest_voice();
void loop_fade_out(voice* v);
void loop_fade_in(voice* v);
void voice_volume_factor(voice* v);
void buffer_chunk_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* voice);
void buffer_chunk_timer_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v);
void timing_test_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v);
void set_voice_vol_zero(voice* v);
void set_voice_volume_pan(voice* v, SLmillibel vol, SLpermille pan);
unsigned short* get_next_data_chunk(voice* voice);
void shutdown_voice(voice* v);
void shutdown_audio();
void init_seg_pan_map();

static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

int initial_loop = TRUE;
static voice poly_sampler[VOICE_COUNT];
extern struct sample_def silence_chunk; // TODO move to snd_sles.h
static const SLEnvironmentalReverbSettings reverbSettings =	SL_I3DL2_ENVIRONMENT_PRESET_ARENA;
static int current_oneshot_voice = 4; // FIXME LOOPER_COUNTのほうが無難かも
static int current_looping_voice = 0;
SLpermille segment_pan_map[TOTAL_NOTES];

//SLEngineItf EngineItf;
//SLAudioIODeviceCapabilitiesItf AudioIODeviceCapabilitiesItf;
//SLAudioOutputDescriptor AudioOutputDescriptor;
/* Get the Audio IO DEVICE CAPABILITIES interface */
//res = (*sl)->GetInterface(sl, SL_IID_AUDIOIODEVICECAPABILITIES, (void*)&AudioIODeviceCapabilitiesItf);
//CheckErr(res);
//numOutputs = MAX_NUMBER_OUTPUT_DEVICES;
//res = (*AudioIODeviceCapabilitiesItf)->GetAvailableAudioOutputs(AudioIODeviceCapabilitiesItf, &numOutputs, OutputDeviceIDs);
//CheckErr(res);

static SLAudioCodecDescriptor AudioCodecDescriptor;
static SLAudioDecoderCapabilitiesItf AudioDecoderCapabilitiesItf;

void create_sl_engine()
{
    SLresult result;

    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);

    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    assert(SL_RESULT_SUCCESS == result);

    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
            &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
    }

    init_seg_pan_map(); // FIXME
    LOGD("create_sl_engine", "init_seg_pan_map()");
    result = (*engineObject)->GetInterface(engineObject, SL_IID_AUDIODECODERCAPABILITIES, (void*)&AudioDecoderCapabilitiesItf);
    assert(SL_RESULT_SUCCESS == result);
    LOGD("create_sl_engine", "GetInterface()");

//    result = (*AudioDecoderCapabilitiesItf)->GetAudioDecoderCapabilities(AudioDecoderCapabilitiesItf, SL_AUDIOCODEC_PCM, NULL, (void*)&AudioCodecDescriptor);
//    assert(SL_RESULT_SUCCESS == result);
//    LOGD("create_sl_engine", "AudioCodecDescriptor.maxChannels: %d", AudioCodecDescriptor.maxChannels);
}


// buffer callback handler
// only required for looping
// no need to register a cb for one shot (might be though if buffers are too small).
void looper_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* samp) {

	LOGD("looper_callback", "looper_callback() called");
	SLresult result;
	result = (*buffer_queue)->Enqueue(buffer_queue, ((struct sample_def*)samp)->buffer_data, ((struct sample_def*)samp)->data_size);
	assert(SL_RESULT_SUCCESS == result);
}

// a single voice dedicated to sample-accurate timing
// timing_voiceは特別なトラックである。正確な時間を守るためのトラック。
void init_voice(voice* v, int timing_voice, int looping_voice) {

	v->vol_fade_factor = 1.0F;
	v->fading_in = FALSE;
	v->fading_out = FALSE;
	v->is_playing = FALSE;
	v->current_chunk = 0;
	v->sample = NULL;

    SLresult result;
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_22_05,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};
    LOGD("init_voice", "SLDataFormat_PCM format_pcm ");

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    LOGD("init_voice", "SLDataSource audioSrc");

    // audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    LOGD("init_voice", "SLDataSink audioSnk");

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    LOGD("init_voice", "const SLboolean req[3]");

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &v->bqPlayerObject, &audioSrc, &audioSnk, 3, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    LOGD("init_voice", " (*engineEngine)->CreateAudioPlayer");

    // realize player
    result = (*v->bqPlayerObject)->Realize(v->bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    LOGD("init_voice", "(*bqPlayerObj)->Realize");

    // play interface
    result = (*v->bqPlayerObject)->GetInterface(v->bqPlayerObject, SL_IID_PLAY, &(v->bqPlayerPlay));
    assert(SL_RESULT_SUCCESS == result);
    LOGD("init_voice", "GetInterface(bqPlayerObj, SL_IID_PLAY, &(voice->bqPlayerPlay))");

    // volume interface
    result = (*v->bqPlayerObject)->GetInterface(v->bqPlayerObject, SL_IID_VOLUME, &(v->bqPlayerVolume));
    assert(SL_RESULT_SUCCESS == result);
    LOGD("init_voice", "(*bqPlayerObj)->GetInterface(bqPlayerObj, SL_IID_VOLUME, &(voice->bqPlayerVolume)");

	// buffer queue interface
    result = (*v->bqPlayerObject)->GetInterface(v->bqPlayerObject, SL_IID_BUFFERQUEUE, &(v->bqPlayerBufferQueue));
    assert(SL_RESULT_SUCCESS == result);
    LOGD("init_voice", "(*bqPlayerObj)->GetInterface(bqPlayerObj, SL_IID_BUFFERQUEUE, &(voice->bqPlayerBufferQueue)");

	if (timing_voice) {
	    result = (*v->bqPlayerBufferQueue)->RegisterCallback(v->bqPlayerBufferQueue, buffer_chunk_timer_callback, (void *)v);
	    assert(SL_RESULT_SUCCESS == result);
	} else {
		result = (*v->bqPlayerBufferQueue)->RegisterCallback(v->bqPlayerBufferQueue, buffer_chunk_callback, (void *)v);
		assert(SL_RESULT_SUCCESS == result);
	}

	LOGD("init_voice", "RegisterCallback() called");

	// set  to playing - ここで問題が起きる
	result = (*v->bqPlayerPlay)->SetPlayState(v->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
	assert(SL_RESULT_SUCCESS == result);
	LOGD("init_voice", "(*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING)");

	v->is_playing = FALSE;
	v->current_chunk = 0;
	v->sample = &silence_chunk;
	v->timing_test_index = 0;

	result = (*v->bqPlayerBufferQueue)->Enqueue(v->bqPlayerBufferQueue,	v->sample->buffer_data, BUFFER_SIZE);
}

void shutdown_voice(voice* v) {

	if (v->bqPlayerObject != NULL) {
		(*v->bqPlayerObject)->Destroy(v->bqPlayerObject);
		v->bqPlayerObject = NULL;
		v->bqPlayerPlay = NULL;
		v->bqPlayerBufferQueue = NULL;
		v->bqPlayerVolume = NULL;
	}
}

// buffer queue players を作り出すため
void init_all_voices() {

	LOGD("init_all_voices", "void init_all_voices() called");
	int i;

	for (i = 0; i < VOICE_COUNT; i++) {
		LOGD("init_all_voices", "void initPolyphony() i: %d", i);
		if (i == LOOPER_COUNT) {
			init_voice(&poly_sampler[i], TRUE, FALSE);
			// TODO set all looper voices vol to min
		} else {
			init_voice(&poly_sampler[i], FALSE, FALSE);
		}

		if (i < LOOPER_COUNT) {
			LOGD("init_all_voices", "(%d < LOOPER_COUNT),set_voice_vol_zero()", i);
			set_voice_vol_zero(&poly_sampler[i]);
		}
	}
	LOGD("init_all_voices", "void init_all_voices() finished");
}

void set_voice_vol_zero(voice* v) {

	SLresult result;

	v->sl_volume = SLMILLIBEL_MIN;
	v->vol_fade_factor = 0.0F;

	SLVolumeItf vol_itf = v->bqPlayerVolume;
	result = (*vol_itf)->SetVolumeLevel(vol_itf, v->sl_volume);
	assert(SL_RESULT_SUCCESS == result);
}


int enqueue_seamless_loop(struct sample_def* s) {

	LOGD("enqueue_seamless_loop", "enqueue_seamless_loop() called");
	LOGD("enqueue_seamless_loop", "	s->vol_factor: %f", s->vol_factor);

	// ここで今の再生中音をフェードアウトし
	// fadeout on the currently playing loop
	SLresult result;
	SLAndroidSimpleBufferQueueState current_queue_state;
	SLuint32 state;
	voice* v = (poly_sampler + current_looping_voice);
	result = (*v->bqPlayerBufferQueue)->GetState(v->bqPlayerBufferQueue, &current_queue_state);
	LOGD("enqueue_seamless_loop", "GetState() called");
	LOGD("enqueue_seamless_loop", "current_queue_state.count: %d", current_queue_state.count);

	if (!initial_loop) {
		LOGD("enqueue_seamless_loop", "(!initial_loop) ");
		// 音量を最大の値を設定するため
		// ensures the loop is fading from 1.0
		v->vol_fade_factor = 1.0F;
		v->fading_in = FALSE;
		v->fading_out = TRUE;
		cycle_looping_voice();
		v = (poly_sampler + current_looping_voice);

	} else {
		// 最初のループをフェードする必要
		// sets initial loop fading on startup
		v->sl_volume = SLMILLIBEL_MIN;
		v->vol_fade_factor = 0.0F;
		SLVolumeItf vol_itf = v->bqPlayerVolume;
		result = (*vol_itf)->SetVolumeLevel(vol_itf, v->sl_volume);
		assert(SL_RESULT_SUCCESS == result);
		initial_loop = FALSE;
	}

    result = (*v->bqPlayerPlay)->SetPlayState(v->bqPlayerPlay, SL_PLAYSTATE_STOPPED);
    assert(SL_RESULT_SUCCESS == result);
	LOGD("enqueue_seamless_loop", "SetPlayState() SL_PLAYSTATE_STOPPED called");

	result = (*v->bqPlayerBufferQueue)->Clear(v->bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    LOGD("enqueue_seamless_loop", "Clear() called");

    result = (*v->bqPlayerBufferQueue)->RegisterCallback(v->bqPlayerBufferQueue, looper_callback, (void *)s);
    assert(SL_RESULT_SUCCESS == result);
    LOGD("enqueue_seamless_loop", "RegisterCallback() called");

	// TODO ここで音の大きさを0に設定して
    result = (*v->bqPlayerPlay)->SetPlayState(v->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    LOGD("enqueue_seamless_loop", "SetPlayState() SL_PLAYSTATE_PLAYING called");

	result =	(*v->bqPlayerBufferQueue)->Enqueue(v->bqPlayerBufferQueue, s->buffer_data,  (int)s->data_size);
	LOGD("enqueue_seamless_loop", "Enqueue() called");

	// ここでフェードインを始めよ
	// new loop voice starts fading in
	v->vol_fade_factor = 0.0F;
	v->fading_in = TRUE;
	v->fading_out = FALSE;
	v->sample = s;

	if (SL_RESULT_SUCCESS != result) {
		return 0;
	}
	return 1;
}

int current_voice_fading() {

	voice* v = (poly_sampler + current_looping_voice);
	if (v->fading_in || v->fading_out) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void vol_automation() {

	int i;
	for (i = 0; i < LOOPER_COUNT; i++) {
		voice* v = &poly_sampler[i];
		if (v->fading_in) {
			loop_fade_in(v);
		}
		if (v->fading_out) {
			loop_fade_out(v);
		}
		if (v->fading_in || v->fading_out) {
			voice_volume_factor(v);
		}
	}
}

void loop_fade_in(voice* v) {

	if (v->vol_fade_factor < 1.0F) {

		v->vol_fade_factor += FADE_FACTOR_CHG_RATE;
	}

	if (v->vol_fade_factor >= 1.0F) {
		v->vol_fade_factor = 1.0F;
		v->fading_in = FALSE;

		buttons[0].busy = FALSE;
		LOGD("loop_fade_in", "buttons[0].busy = FALSE");
	}
}

void loop_fade_out(voice* v) {

	if (v->vol_fade_factor > 0.0F) {
		v->vol_fade_factor -= FADE_FACTOR_CHG_RATE;
	}

	if (v->vol_fade_factor <= 0.0F) {
		v->vol_fade_factor = 0.0F;
		v->fading_out = FALSE;
	}
}

void pause_all_voices() {

	int i;
	for (i = 0; i < VOICE_COUNT; i++) {
		voice* v = &poly_sampler[i];
		SLresult result;
		SLPlayItf play_itf = v->bqPlayerPlay;
		result = (*play_itf)->SetPlayState(play_itf, SL_PLAYSTATE_PAUSED);
						assert(SL_RESULT_SUCCESS == result);
	}

	initial_loop = TRUE;
}


void voice_volume_factor(voice* v) {

	v->sl_volume = (1.0F - (v->vol_fade_factor * v->sample->vol_factor)) * SLMILLIBEL_MIN;

	LOGD("voice_volume_factor",
			"v->vol_fade_factor: %f, v->sample->vol_factor: %f, v->sl_volume: %d",
			v->vol_fade_factor, v->sample->vol_factor, v->sl_volume);

	SLresult result;
	SLVolumeItf vol_itf = v->bqPlayerVolume;

	result = (*vol_itf)->SetVolumeLevel(vol_itf, v->sl_volume);
	assert(SL_RESULT_SUCCESS == result);

}


// set a 0.0 - 1.0 float to the appropriate SLmillibel val
SLmillibel float_to_slmillibel(float sender_vel, float sender_range) {

	if(sender_vel > 1.0f) sender_vel = 1.0f;
	SLmillibel vol =  (sender_vel * (VEL_SLMILLIBEL_RANGE/sender_range)) + VEL_SLMILLIBEL_MIN + VEL_SLMILLIBEL_OFFSET;
	LOGD("float_to_slmillibel", "v: %d", vol);

	return vol;
}

SLpermille get_seg_permille(size_t seg) {
	return segment_pan_map[seg];
}

void init_seg_pan_map() {
	int i;
	for(i=0;i<TOTAL_NOTES; i++) {
		segment_pan_map[i] = ((i * SEG_PAN_AMT) - (((TOTAL_NOTES) * SEG_PAN_AMT) / 2)) + (SEG_PAN_AMT / 2);
		LOGD("init_seg_pan_map", "segment_pan_map[%d]: %d", i, segment_pan_map[i]);
	}
}

int enqueue_one_shot(struct sample_def * s, SLmillibel vol, SLpermille pan) {
	LOGI("play_all_parts", "enqueue_one_shot");
	LOGD("enqueue_one_shot", "vol: %d", vol);

	// この方法は一番いいと思う。最古なボイズを選ぶとボイス全体の数が少ないのに、
	// なんの違和感なくたくさんの音を鳴らすことが出来る
	// this is the best voice selecting method I believe
	// it allows less voices to be used overall
	// if the samples were of different lengths, get_next_free_voice() could be a better choice.
	voice* v = get_oldest_voice();
	// voice* v = get_next_free_voice();

	v->sample = s;
	v->current_chunk = 0;
	v->is_playing = TRUE;

	SLresult result;
	set_voice_volume_pan(v, vol, pan);
	LOGD("enqueue_one_shot", "v->current_chunk: %d", v->current_chunk);
	LOGD("enqueue_one_shot", "v->sample->data_size: %d", v->sample->data_size);
	LOGD("enqueue_one_shot", "v->is_playing: %d", v->is_playing);

	if (SL_RESULT_SUCCESS != result) {
		return 0;
	}

	return 1;
}

// 一発的な音のため
// for setting the pan values of one-shot samples
void set_voice_volume_pan(voice* v, SLmillibel vol, SLpermille pan) { //入力スべき値は0から1の間

	LOGD("set_voice_volume_pan", "vol: %d", vol);
	LOGD("set_voice_volume_pan", "pan: %d", pan);

	SLresult result;
	SLVolumeItf volumeItf = v->bqPlayerVolume;

	if (NULL != volumeItf) {
		result = (*volumeItf)->SetVolumeLevel(volumeItf, vol);
		assert(SL_RESULT_SUCCESS == result);
	}
}

unsigned short* get_next_data_chunk(voice* v) {
	unsigned short* b;

	if (v->current_chunk == v->sample->total_chunks && v->is_playing) {
		LOGD("get_next_data_chunk", "buffer_logic 1");

		v->sample = &silence_chunk;
		b = silence_chunk.buffer_data;
		v->is_playing = FALSE;

	} else if (v->current_chunk < v->sample->total_chunks && v->is_playing) {
		// LOGD("get_next_data_chunk", "buffer_logic 2");

		b = v->sample->buffer_data + (v->current_chunk * BUFFER_SIZE_SHORT);
		v->current_chunk++;

	} else if (!v->is_playing) {
		// LOGD("get_next_data_chunk", "buffer_logic 3");

		v->sample = &silence_chunk;
		b = silence_chunk.buffer_data;
	}
	return b;
}

// main callback for one shot samples
void buffer_chunk_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v) {
	SLresult result;
	result = (*buffer_queue)->Enqueue(buffer_queue, get_next_data_chunk((voice*)v), BUFFER_SIZE);
	assert(SL_RESULT_SUCCESS == result);
}

// used for sample accurate timing
void buffer_chunk_timer_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v) {
	part_tic_counter();
	play_all_parts();
	SLresult result;
	result = (*buffer_queue)->Enqueue(buffer_queue, get_next_data_chunk((voice*)v), BUFFER_SIZE);
	assert(SL_RESULT_SUCCESS == result);
}

void timing_test_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v) {

	SLresult result;
	if (((voice*)v)->timing_test_index == 0) {
		result = (*buffer_queue)->Enqueue(buffer_queue, oneshot_samples[0].buffer_data, BUFFER_SIZE);
		((voice*)v)->timing_test_index++;

	} else if (((voice*)v)->timing_test_index < 1024) {
		result = (*buffer_queue)->Enqueue(buffer_queue, silence_chunk.buffer_data, BUFFER_SIZE);
		((voice*)v)->timing_test_index++;

	} else if (((voice*)v)->timing_test_index == 1024) {
		result = (*buffer_queue)->Enqueue(buffer_queue, silence_chunk.buffer_data, BUFFER_SIZE);
		((voice*)v)->timing_test_index = 0;

	}
	assert(SL_RESULT_SUCCESS == result);
}


voice* get_next_free_voice() {

	int i;
	voice* v = NULL;

	for (i = LOOPER_COUNT; i < VOICE_COUNT; i++) {
		if (!poly_sampler[i].is_playing) {
			v = poly_sampler + i;
			break;
		}
	}
	LOGD("get_next_free_voice", "i: %d", i);
	return v;
}

voice* get_oldest_voice() {

	voice* v = NULL;

	if (current_oneshot_voice < VOICE_COUNT)
		current_oneshot_voice += 1;

	if (current_oneshot_voice == VOICE_COUNT)
		current_oneshot_voice = LOOPER_COUNT;

	v = poly_sampler + current_oneshot_voice;

	LOGD("get_oldest_voice", "i: %d", current_oneshot_voice);

	return v;

}

void cycle_looping_voice() {

	if (current_looping_voice < LOOPER_COUNT)
		current_looping_voice += 1;

	if (current_looping_voice == LOOPER_COUNT)
		current_looping_voice = 0;
}

void cycle_oneshot_voice() {

	if (current_oneshot_voice < VOICE_COUNT)
		current_oneshot_voice+=1;

	if (current_oneshot_voice == VOICE_COUNT)
		current_oneshot_voice=LOOPER_COUNT;
}

void shutdown_audio() {
	LOGD("shutdown_audio", "shutdown_audio() called");

	int i;
	for(i=0;i<VOICE_COUNT;i++) {
		shutdown_voice(&poly_sampler[i]);
	}

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
}

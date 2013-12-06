/*
 * slman.c / OpenSL Manager
 *
 *  Created on: 2013/05/31
 *      Author: Michael
 *      OpenSLの処理のみのファイルです
 */
// OpenSL ES management

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <jni.h>
#include <string.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

// for threading
#include <unistd.h>  // sleep()を定義
#include <pthread.h>
//#include <time.h>
#include <android/log.h>
#include <time.h>


//#include "game/moods.h"



#include "snd_sles.h"
#include "hon_type.h"
#include "snd_ctrl.h"
#include "snd_scal.h"
#include "snd_asst.h"






//int PLYCNT = 10;
// voice count
//#define VOICE_COUNT 10 //フレームレートに影響を与える
//#define VOICE_COUNT 30
#define VOICE_COUNT 20
#define LOOPER_COUNT 2

#define SLMILLIBEL_MIN -4000
#define SLMILLIBEL_MAX 0

//#define VEL_SLMILLIBEL_MIN -1500
//#define VEL_SLMILLIBEL_RANGE 1700

#define VEL_SLMILLIBEL_MIN -3000
#define VEL_SLMILLIBEL_RANGE 3000

#define PAN_RANGE 1000
#define SEG_PAN_AMT 30


#define FADE_FACTOR_CHG_RATE 0.01F
#define FADE_FACTOR_CHG_RATE_QUICK 0.1F


//long elapsed_buffer_tics = 0;


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
//int timing_test_index = 0;



void set_voice_vol_zero(voice* v);
//void set_voice_volume(voice* voice, float vol, int pan);
void set_voice_volume_pan(voice* v, SLmillibel vol, SLpermille pan);
//SLmillibel float_to_slmillibel(float sender_vel, float sender_range);

unsigned short* get_next_data_chunk(voice* voice);

void shutdown_voice(voice* v);
//void shutdown_all_voices();

//void loop_fade_out_exit(voice* v);
void shutdown_audio();
void init_seg_pan_map();

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;




// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

//static SLVolumeItf outputMixVolume = NULL;

// フェードのため
//static SLVolumeItf fade_in_itf;
//static SLVolumeItf fade_out_itf;
int initial_loop = TRUE;


static voice poly_sampler[VOICE_COUNT];

extern struct sample_def silence_chunk;

// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
		SL_I3DL2_ENVIRONMENT_PRESET_ARENA;




static int current_oneshot_voice = 4; // FIXME LOOPER_COUNTのほうが無難かも
static int current_looping_voice = 0;

// 実時間に計算したくないから
SLpermille segment_pan_map[TOTAL_NOTES];



//static int currently_xfading = 0;
// 必要の理由：ループのcallbackを登録するため
//extern oneshot_def looping_samples[];



SLEngineItf EngineItf;
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












    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    assert(SL_RESULT_SUCCESS == result);

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);



//    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_VOLUME, &outputMixVolume);
//    assert(SL_RESULT_SUCCESS == result);
//	result = (*outputMixVolume)->SetVolumeLevel(outputMixVolume, SLMILLIBEL_MIN);
//    assert(SL_RESULT_SUCCESS == result);


    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
            &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
    }
    // ignore unsuccessful result codes for environmental reverb, as it is optional for this example

    init_seg_pan_map(); // FIXME


    LOGD("create_sl_engine", "init_seg_pan_map()");


    result = (*engineObject)->GetInterface(engineObject, SL_IID_AUDIODECODERCAPABILITIES, (void*)&AudioDecoderCapabilitiesItf);
    assert(SL_RESULT_SUCCESS == result);


    LOGD("create_sl_engine", "GetInterface()");


//    result = (*AudioDecoderCapabilitiesItf)->GetAudioDecoderCapabilities(AudioDecoderCapabilitiesItf, SL_AUDIOCODEC_PCM, NULL, (void*)&AudioCodecDescriptor);
//    assert(SL_RESULT_SUCCESS == result);
//
//    LOGD("create_sl_engine", "AudioCodecDescriptor.maxChannels: %d", AudioCodecDescriptor.maxChannels);




}


// this callback handler is called every time a buffer finishes playing
//only required for looping... no need to register a cb for one shot (might be though if buffers are too small).
void looper_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* samp) {

	LOGD("looper_callback", "looper_callback() called");

	//assert(bq == bqPlayerBufferQueue);
	//assert(NULL == context);

	SLresult result;

	result = (*buffer_queue)->Enqueue(buffer_queue, ((struct sample_def*)samp)->buffer_data, ((struct sample_def*)samp)->data_size);

	//result = (*bqPlayerBufferQueue)->GetState(bqPlayerBufferQueue, &bqstate);

/*	__android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag: ",
			"bqstate.count %d", bqstate.count);*/

	// the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
	// which for this code example would indicate a programming error
	assert(SL_RESULT_SUCCESS == result);
}

// timing_voiceは特別なトラックである。正確な時間を守るためのトラック。
void init_voice(voice* v, int timing_voice, int looping_voice) {

	v->vol_fade_factor = 1.0F;
//	v->vol_auto_factor = 1.0F;

	v->fading_in = FALSE;
	v->fading_out = FALSE;

	v->is_playing = FALSE;
	v->current_chunk = 0;
	v->sample = NULL;

    SLresult result;

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
//    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
//        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
//        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_22_05,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

    LOGD("init_voice", "SLDataFormat_PCM format_pcm ");
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    LOGD("init_voice", "SLDataSource audioSrc");

    // configure audio sink
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
    // realize the player
    result = (*v->bqPlayerObject)->Realize(v->bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    LOGD("init_voice", "(*bqPlayerObj)->Realize");
    // get the play interface
    result = (*v->bqPlayerObject)->GetInterface(v->bqPlayerObject, SL_IID_PLAY, &(v->bqPlayerPlay));
    assert(SL_RESULT_SUCCESS == result);
    LOGD("init_voice", "GetInterface(bqPlayerObj, SL_IID_PLAY, &(voice->bqPlayerPlay))");

    // get the volume interface
    result = (*v->bqPlayerObject)->GetInterface(v->bqPlayerObject, SL_IID_VOLUME, &(v->bqPlayerVolume));
    assert(SL_RESULT_SUCCESS == result);
    LOGD("init_voice", "(*bqPlayerObj)->GetInterface(bqPlayerObj, SL_IID_VOLUME, &(voice->bqPlayerVolume)");

// 今パンほしくない
//	SLboolean true = TRUE;
//		// ステレィオを有効する
//	result = (*v->bqPlayerVolume)->EnableStereoPosition(v->bqPlayerVolume, true);
//    assert(SL_RESULT_SUCCESS == result);
//	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "EnableStereoPosition");


	// get the buffer queue interface
    result = (*v->bqPlayerObject)->GetInterface(v->bqPlayerObject, SL_IID_BUFFERQUEUE, &(v->bqPlayerBufferQueue));
    assert(SL_RESULT_SUCCESS == result);
    LOGD("init_voice", "(*bqPlayerObj)->GetInterface(bqPlayerObj, SL_IID_BUFFERQUEUE, &(voice->bqPlayerBufferQueue)");

	if (timing_voice) {
	    result = (*v->bqPlayerBufferQueue)->RegisterCallback(v->bqPlayerBufferQueue, buffer_chunk_timer_callback, (void *)v);
	    assert(SL_RESULT_SUCCESS == result);
	} else {
		result = (*v->bqPlayerBufferQueue)->RegisterCallback(v->bqPlayerBufferQueue, buffer_chunk_callback, (void *)v);
		assert(SL_RESULT_SUCCESS == result);
		// result = (*v->bqPlayerBufferQueue)->RegisterCallback(v->bqPlayerBufferQueue, timing_test_callback, (void *)v);
	}

	LOGD("init_voice", "RegisterCallback() called");
	//if (!looping_voice) {
		// set the player's state to playing - ここで問題が起きる
	result = (*v->bqPlayerPlay)->SetPlayState(v->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
	assert(SL_RESULT_SUCCESS == result);
	//}
	LOGD("init_voice", "(*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING)");

	v->is_playing = FALSE;
	v->current_chunk = 0;
//	v->current_chunk_size = BUFFER_SIZE;
	v->sample = &silence_chunk;
	v->timing_test_index = 0;

//	result = (*v->bqPlayerBufferQueue)->Enqueue(v->bqPlayerBufferQueue,	v->sample->buffer_data, v->current_chunk_size);

	//if (!looping_voice) {
		result = (*v->bqPlayerBufferQueue)->Enqueue(v->bqPlayerBufferQueue,	v->sample->buffer_data, BUFFER_SIZE);
//	}



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

//		if (i < LOOPER_COUNT) {
//			init_voice(&poly_sampler[i], FALSE, TRUE);
//		} else

		if (i == LOOPER_COUNT) {
			init_voice(&poly_sampler[i], TRUE, FALSE);

			// NEED TO SET ALL LOOPER VOICES VOL TO ZERO.
		} else {
			init_voice(&poly_sampler[i], FALSE, FALSE);
		}

		// ゲロみたいな音を防げるため
		if (i < LOOPER_COUNT) {
			LOGD("init_all_voices", "(%d < LOOPER_COUNT),set_voice_vol_zero()", i);
			set_voice_vol_zero(&poly_sampler[i]);

		}

		// バッファーのタイミングテストのため
//		if (i == 4) {
//			SLresult result;
//			voice* v = &poly_sampler[i];
//			result = (*v->bqPlayerPlay)->SetPlayState(v->bqPlayerPlay,
//					SL_PLAYSTATE_PLAYING);
//		}
//
//		if (i == 29) {
//			SLresult result;
//			voice* v = &poly_sampler[i];
//			result = (*v->bqPlayerPlay)->SetPlayState(v->bqPlayerPlay,
//					SL_PLAYSTATE_PLAYING);
//		}
//
//		if (i == 16) {
//			SLresult result;
//			voice* v = &poly_sampler[i];
//			result = (*v->bqPlayerPlay)->SetPlayState(v->bqPlayerPlay,
//					SL_PLAYSTATE_PLAYING);
//		}

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

	SLresult result;

	LOGD("enqueue_seamless_loop", "enqueue_seamless_loop() called");
	LOGD("enqueue_seamless_loop", "	s->vol_factor: %f", s->vol_factor);


	// ここで今の再生中音をフェードアウトし
	SLAndroidSimpleBufferQueueState current_queue_state;
	SLuint32 state;
	LOGD("enqueue_seamless_loop", "debug_a, current looping voice: %d", current_looping_voice);

	voice* v = (poly_sampler + current_looping_voice);
	LOGD("enqueue_seamless_loop", "debug_b");


	// ポインターのポインターかもなぁ
	result = (*v->bqPlayerBufferQueue)->GetState(v->bqPlayerBufferQueue, &current_queue_state);
	LOGD("enqueue_seamless_loop", "debug_c");

	LOGD("enqueue_seamless_loop", "GetState() called");
	LOGD("enqueue_seamless_loop", "current_queue_state.count: %d", current_queue_state.count);

	// 最初のループをフェードする必要
	if (!initial_loop) {

		LOGD("enqueue_seamless_loop", "(!initial_loop) ");

		// 音量を最大の値を設定するため
		v->vol_fade_factor = 1.0F;
		v->fading_in = FALSE;
		v->fading_out = TRUE;

		cycle_looping_voice();

		v = (poly_sampler + current_looping_voice);
		LOGD("enqueue_seamless_loop", "debug_d, current looping voice: %d", current_looping_voice);
	} else {

		// 浮動小数点型なボリューム操作機能を使うべきだ
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

	// ここで音の大きさを0に設定して

    result = (*v->bqPlayerPlay)->SetPlayState(v->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    LOGD("enqueue_seamless_loop", "SetPlayState() SL_PLAYSTATE_PLAYING called");


	result =	(*v->bqPlayerBufferQueue)->Enqueue(v->bqPlayerBufferQueue, s->buffer_data,  (int)s->data_size);
	LOGD("enqueue_seamless_loop", "Enqueue() called");

	// ここでフェードインを始めよ
//	v->sl_volume = SLMILLIBEL_MIN;
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
//	__android_log_write(ANDROID_LOG_DEBUG, "loop_fade_in", "loop_fade_in() called");

	if (v->vol_fade_factor < 1.0F) {
		v->vol_fade_factor += FADE_FACTOR_CHG_RATE;
	}

	if (v->vol_fade_factor >= 1.0F) {
		v->vol_fade_factor = 1.0F;
		v->fading_in = FALSE;
//		__android_log_print(ANDROID_LOG_DEBUG, "loop_fade_in", "v->vol_fade_factor: %f", v->vol_fade_factor);
	}

}

void loop_fade_out(voice* v) {
//	__android_log_write(ANDROID_LOG_DEBUG, "loop_fade_out", "loop_fade_out() called");

	if (v->vol_fade_factor > 0.0F) {
		v->vol_fade_factor -= FADE_FACTOR_CHG_RATE;
	}

	if (v->vol_fade_factor <= 0.0F) {
		v->vol_fade_factor = 0.0F;
		v->fading_out = FALSE;
//		__android_log_print(ANDROID_LOG_DEBUG, "loop_fade_out", "v->vol_fade_factor: %f", v->vol_fade_factor);
	}

}

//void loop_fade_in(voice* v) {
//	float* vf = &v->sample->vol_factor;
//
//	if (v->vol_fade_factor < *vf) {
//		v->vol_fade_factor += (FADE_FACTOR_CHG_RATE * *vf);
//	}
//	if (v->vol_fade_factor >= *vf) {
//		v->vol_fade_factor = *vf;
//		v->fading_in = FALSE;
//	}
//}
//
//void loop_fade_out(voice* v) {
//
//	float* vf = &v->sample->vol_factor;
//	if (v->vol_fade_factor > 0.0F) {
//		v->vol_fade_factor -= (FADE_FACTOR_CHG_RATE * *vf);
//	}
//	if (v->vol_fade_factor <= 0.0F) {
//		v->vol_fade_factor = 0.0F;
//		v->fading_out = FALSE;
//	}
//}



void pause_all_voices() {
	int i;
	for (i = 0; i < VOICE_COUNT; i++) {
		voice* v = &poly_sampler[i];

//		v->fading_in = FALSE;
//		v->fading_out = FALSE;
//
//
//		v->sl_volume = SLMILLIBEL_MIN;

		SLresult result;
//		SLVolumeItf vol_itf = v->bqPlayerVolume;
//		result = (*vol_itf)->SetVolumeLevel(vol_itf, v->sl_volume);
//				assert(SL_RESULT_SUCCESS == result);



		SLPlayItf play_itf = v->bqPlayerPlay;

		result = (*play_itf)->SetPlayState(play_itf, SL_PLAYSTATE_PAUSED);
						assert(SL_RESULT_SUCCESS == result);
	}


	initial_loop = TRUE;
}



// 自動的なフェードのための関数
void voice_volume_factor(voice* v) {

//	v->sl_volume = (1.0F - (v->vol_fade_factor * v->vol_auto_factor)) * SLMILLIBEL_MIN;
	v->sl_volume = (1.0F - (v->vol_fade_factor * v->sample->vol_factor)) * SLMILLIBEL_MIN;
	LOGD("voice_volume_factor",
			"v->vol_fade_factor: %f, v->sample->vol_factor: %f, v->sl_volume: %d",
			v->vol_fade_factor, v->sample->vol_factor, v->sl_volume);

	SLresult result;
	SLVolumeItf vol_itf = v->bqPlayerVolume;

	result = (*vol_itf)->SetVolumeLevel(vol_itf, v->sl_volume);
	assert(SL_RESULT_SUCCESS == result);

}



SLmillibel float_to_slmillibel(float sender_vel, float sender_range) {

	// エラーが発生可能性を確認しなきゃ

	SLmillibel vol = (sender_vel * (VEL_SLMILLIBEL_RANGE/sender_range)) + VEL_SLMILLIBEL_MIN;
// 速度を上げるため、こういう式(VEL_SLMILLIBEL_RANGE/sender_range)を既に計算スべし？

// 0以上なら制御てきに０に設定
if (vol > 0)	vol = 0;

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

//sample_def* get_scale_sample(int seg) {
//
//	int sample = moods->scale->midimap[seg];
//	sample -= START_NOTE;
//
//	return (oneshot_samples + sample);
//
//}


// 書き直すべき
int enqueue_one_shot(struct sample_def * s, SLmillibel vol, SLpermille pan) {


	LOGI("play_all_parts", "enqueue_one_shot");


	LOGD("enqueue_one_shot", "vol: %d", vol);

//	voice* v = get_next_free_voice();

	// この方法は一番いいと思う。最古なボイズを選ぶとボイス全体の数が少ないのに、
	// なんの違和感なくたくさんの音を鳴らすことが出来る
	voice* v = get_oldest_voice();

//	if (v == NULL)
//	{
//		LOGD("enqueue_one_shot", "v == NULL: couldn't find a free buffer");
//		return 0;
//	}


	v->sample = s;
	v->current_chunk = 0;
//	v->current_chunk_size = BUFFER_SIZE;
	v->is_playing = TRUE;

	SLresult result;

//	set_voice_volume(v, vol, pan); // 一回しか必要ない


	set_voice_volume_pan(v, vol, pan);


	LOGD("enqueue_one_shot", "v->current_chunk: %d", v->current_chunk);
//	__android_log_print(ANDROID_LOG_DEBUG, "enqueue_one_shot", "v->current_chunk_size: %d", v->current_chunk_size);
	LOGD("enqueue_one_shot", "v->sample->data_size: %d", v->sample->data_size);
	LOGD("enqueue_one_shot", "v->is_playing: %d", v->is_playing);

	//result = (*v->bqPlayerBufferQueue)->Enqueue(v->bqPlayerBufferQueue, get_next_data_chunk(v), v->current_chunk_size);

	if (SL_RESULT_SUCCESS != result) {
		return 0;
	}

	return 1;

}



// 一発的な音のため
void set_voice_volume_pan(voice* v, SLmillibel vol, SLpermille pan) { //入力スべき値は0から1の間

	LOGD("set_voice_volume_pan", "vol: %d", vol);
	LOGD("set_voice_volume_pan", "pan: %d", pan);

	//SLmillibel sl_millibel = float_to_slmillibel(vol, 1.0F);
//	__android_log_print(ANDROID_LOG_DEBUG, "set_voice_volume", "sl_millibel: %d", sl_millibel);

//	SLpermille sl_permille = segment_pan_vals[pan];

	SLresult result;
	// 書き直す
	// SLVolumeItf volumeItf = poly_sampler[current_oneshot_voice].bqPlayerVolume;

	SLVolumeItf volumeItf = v->bqPlayerVolume;

	if (NULL != volumeItf) {
		result = (*volumeItf)->SetVolumeLevel(volumeItf, vol);
		assert(SL_RESULT_SUCCESS == result);

		// 今は必要ないパン
//		result = (*volumeItf)->SetStereoPosition(volumeItf, pan);
//		assert(SL_RESULT_SUCCESS == result);

	}

}



//unsigned short* get_next_data_chunk(voice* v) {
//
////	__android_log_write(ANDROID_LOG_DEBUG, "get_next_data_chunk", "get_next_data_chunk(voice* v) called");
//
//	unsigned short* b;
//
//	size_t remaining_size = v->sample->data_size - (v->current_chunk * BUFFER_SIZE);
//
//	if (remaining_size <= BUFFER_SIZE && v->is_playing) {
//		__android_log_write(ANDROID_LOG_DEBUG, "get_next_data_chunk", "buffer_logic 1");
////		__android_log_print(ANDROID_LOG_DEBUG, "get_next_data_chunk", "v->current_chunk: %d", v->current_chunk);
//		__android_log_print(ANDROID_LOG_DEBUG, "get_next_data_chunk", "remaining_size: %d", remaining_size);
//
//		// これは問題
//		v->current_chunk_size = remaining_size;
//		b = v->sample->buffer_data + (v->current_chunk * BUFFER_SIZE_SHORT);
//		// v->sample = 0; // 無音なサンプル
//
//		v->is_playing = FALSE;
//
//	} else if (remaining_size > BUFFER_SIZE && v->is_playing) {
//
////		__android_log_write(ANDROID_LOG_DEBUG, "get_next_data_chunk", "buffer_logic 2");
//
//		v->current_chunk_size = BUFFER_SIZE;
//		b = v->sample->buffer_data + (v->current_chunk * BUFFER_SIZE_SHORT);
//		v->current_chunk++;
//
//	} else if (!v->is_playing) {
//
////		__android_log_write(ANDROID_LOG_DEBUG, "get_next_data_chunk", "buffer_logic 3");
//
//		v->sample = &silence_chunk;
//		v->current_chunk_size = BUFFER_SIZE;
//		b = silence_chunk.buffer_data;
//	}
//
//	return b;
//
//}

unsigned short* get_next_data_chunk(voice* v) {
	unsigned short* b;

	if (v->current_chunk == v->sample->total_chunks && v->is_playing) {
		LOGD("get_next_data_chunk", "buffer_logic 1");

		v->sample = &silence_chunk;
		b = silence_chunk.buffer_data;
		v->is_playing = FALSE;

	} else if (v->current_chunk < v->sample->total_chunks && v->is_playing) {
//		LOGD("get_next_data_chunk", "buffer_logic 2");

		b = v->sample->buffer_data + (v->current_chunk * BUFFER_SIZE_SHORT);
		v->current_chunk++;

	} else if (!v->is_playing) {
//		LOGD("get_next_data_chunk", "buffer_logic 3");

		v->sample = &silence_chunk;
		b = silence_chunk.buffer_data;
	}

	return b;

}


void buffer_chunk_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v) {
//	__android_log_write(ANDROID_LOG_DEBUG, "buffer_chunk_callback", "buffer_chunk_callback() called");
//usleep(200000);
	SLresult result;

//	result = (*buffer_queue)->Enqueue(buffer_queue, get_next_data_chunk((voice*)v), ((voice*)v)->current_chunk_size);
	result = (*buffer_queue)->Enqueue(buffer_queue, get_next_data_chunk((voice*)v), BUFFER_SIZE);


	assert(SL_RESULT_SUCCESS == result);
}

//extern pthread_mutex_t mutex;

void buffer_chunk_timer_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v) {
//	__android_log_write(ANDROID_LOG_DEBUG, "buffer_chunk_timer_callback", "buffer_chunk_timer_callback() called");
//total_tic_counter++;
	part_tic_counter();


	play_all_parts();

	SLresult result;
	result = (*buffer_queue)->Enqueue(buffer_queue, get_next_data_chunk((voice*)v), BUFFER_SIZE);
	assert(SL_RESULT_SUCCESS == result);
}

void timing_test_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v) {
//	__android_log_write(ANDROID_LOG_DEBUG, "timing_test_callback", "timing_test_callback() called");

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


// 連続的な音は0から3
void cycle_looping_voice() {

	if (current_looping_voice < LOOPER_COUNT)
		current_looping_voice += 1;

	if (current_looping_voice == LOOPER_COUNT)
		current_looping_voice = 0;
}



// 声部・パートは4から28
void cycle_oneshot_voice() {

	if (current_oneshot_voice < VOICE_COUNT)
		current_oneshot_voice+=1;

	if (current_oneshot_voice == VOICE_COUNT)
		current_oneshot_voice=LOOPER_COUNT;
}



// shut down the native audio system
void shutdown_audio()
{
	LOGD("shutdown_audio", "shutdown_audio() called");

//	shutdown_all_voices();
//

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

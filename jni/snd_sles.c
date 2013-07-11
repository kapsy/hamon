/*
 * slman.c / OpenSL Manager
 *
 *  Created on: 2013/05/31
 *      Author: Michael
 *      OpenSLの処理のみのファイルです
 */
// OpenSL ES management


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


// kapsys includes
//#include <android/log.h>;

#include "snd_sles.h"
#include "snd_asst.h"
#include "hon_type.h"
#include "snd_ctrl.h"
#include "snd_scal.h"


//int PLYCNT = 10;
// voice count
//#define VOICE_COUNT 10 //フレームレートに影響を与える
#define VOICE_COUNT 30
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

void loop_fade_out(voice* v);
void loop_fade_in(voice* v);
void voice_volume_factor(voice* v);

void buffer_chunk_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* voice);
void buffer_chunk_timer_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v);
void timing_test_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v);
//int timing_test_index = 0;



//void set_voice_volume(voice* voice, float vol, int pan);
void set_voice_volume_pan(voice* v, SLmillibel vol, SLpermille pan);
//SLmillibel float_to_slmillibel(float sender_vel, float sender_range);

unsigned short* get_next_data_chunk(voice* voice);
voice* get_next_free_voice();

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

extern sample_def silence_chunk;

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
}


// this callback handler is called every time a buffer finishes playing
//only required for looping... no need to register a cb for one shot (might be though if buffers are too small).
void looper_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* samp) {

	__android_log_write(ANDROID_LOG_DEBUG, "looper_callback", "looper_callback() called");

	//assert(bq == bqPlayerBufferQueue);
	//assert(NULL == context);

	SLresult result;

	result = (*buffer_queue)->Enqueue(buffer_queue, ((sample_def*)samp)->buffer_data, ((sample_def*)samp)->data_size);

	//result = (*bqPlayerBufferQueue)->GetState(bqPlayerBufferQueue, &bqstate);

/*	__android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag: ",
			"bqstate.count %d", bqstate.count);*/

	// the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
	// which for this code example would indicate a programming error
	assert(SL_RESULT_SUCCESS == result);
}


void init_voice(voice* v, int timing_voice, int looping_voice) {

	v->vol_fade_factor = 1.0F;
	v->vol_auto_factor = 1.0F;
//	v->vol_exit_factor = 1.0F;


	v->fading_in = FALSE;
	v->fading_out = FALSE;
//	v->fading_out_exit = FALSE;

	v->is_playing = FALSE;
	v->current_chunk = 0;
	v->sample = NULL;

    SLresult result;

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};


//    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_22_05,
//        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
//        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

    // フレームレートにあまり影響を与えないのかも
//    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_11_025,
//        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
//        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};


	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "SLDataFormat_PCM format_pcm ");
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "SLDataSource audioSrc");

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "SLDataSink audioSnk");

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "const SLboolean req[3]");

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &v->bqPlayerObject, &audioSrc, &audioSnk, 3, ids, req);
    assert(SL_RESULT_SUCCESS == result);
	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", " (*engineEngine)->CreateAudioPlayer");
    // realize the player
    result = (*v->bqPlayerObject)->Realize(v->bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "(*bqPlayerObj)->Realize");
    // get the play interface
    result = (*v->bqPlayerObject)->GetInterface(v->bqPlayerObject, SL_IID_PLAY, &(v->bqPlayerPlay));
    assert(SL_RESULT_SUCCESS == result);
	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "GetInterface(bqPlayerObj, SL_IID_PLAY, &(voice->bqPlayerPlay))");

    // get the volume interface
    result = (*v->bqPlayerObject)->GetInterface(v->bqPlayerObject, SL_IID_VOLUME, &(v->bqPlayerVolume));
    assert(SL_RESULT_SUCCESS == result);
	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "(*bqPlayerObj)->GetInterface(bqPlayerObj, SL_IID_VOLUME, &(voice->bqPlayerVolume)");

// 今パンほしくない
//	SLboolean true = TRUE;
//		// ステレィオを有効する
//	result = (*v->bqPlayerVolume)->EnableStereoPosition(v->bqPlayerVolume, true);
//    assert(SL_RESULT_SUCCESS == result);
//	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "EnableStereoPosition");




	// get the buffer queue interface
    result = (*v->bqPlayerObject)->GetInterface(v->bqPlayerObject, SL_IID_BUFFERQUEUE, &(v->bqPlayerBufferQueue));
    assert(SL_RESULT_SUCCESS == result);
	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "(*bqPlayerObj)->GetInterface(bqPlayerObj, SL_IID_BUFFERQUEUE, &(voice->bqPlayerBufferQueue)");




	if (timing_voice) {
	    result = (*v->bqPlayerBufferQueue)->RegisterCallback(v->bqPlayerBufferQueue, buffer_chunk_timer_callback, (void *)v);
	    assert(SL_RESULT_SUCCESS == result);
	} else {
		result = (*v->bqPlayerBufferQueue)->RegisterCallback(v->bqPlayerBufferQueue, buffer_chunk_callback, (void *)v);
		assert(SL_RESULT_SUCCESS == result);
		// result = (*v->bqPlayerBufferQueue)->RegisterCallback(v->bqPlayerBufferQueue, timing_test_callback, (void *)v);
	}

	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "RegisterCallback() called");


	//if (!looping_voice) {
		// set the player's state to playing - ここで問題が起きる
		result = (*v->bqPlayerPlay)->SetPlayState(v->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
		assert(SL_RESULT_SUCCESS == result);
	//}
	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "(*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING)");

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
	__android_log_write(ANDROID_LOG_DEBUG, "init_all_voices", "void init_all_voices() called");
	int i;

	for (i = 0; i < VOICE_COUNT; i++) {
		__android_log_print(ANDROID_LOG_DEBUG, "init_all_voices", "void initPolyphony() i: %d", i);

//		if (i < LOOPER_COUNT) {
//			init_voice(&poly_sampler[i], FALSE, TRUE);
//		} else

		if (i == LOOPER_COUNT) {
			init_voice(&poly_sampler[i], TRUE, FALSE);
		} else {
			init_voice(&poly_sampler[i], FALSE, FALSE);
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
	__android_log_write(ANDROID_LOG_DEBUG, "init_all_voices", "void init_all_voices() finished");
}



int enqueue_seamless_loop(sample_def* s) {

	SLresult result;

	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "enqueue_seamless_loop() called");

	// ここで今の再生中音をフェードアウトし
	SLAndroidSimpleBufferQueueState current_queue_state ;
	SLuint32 state;

	voice* v = (poly_sampler + current_looping_voice);

	// ポインターのポインターかもなぁ
	result = (*v->bqPlayerBufferQueue)->GetState(v->bqPlayerBufferQueue, &current_queue_state);


	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "GetState() called");
	__android_log_print(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "current_queue_state.count: %d", current_queue_state.count);

	if (!initial_loop) {

		__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "(!initial_loop) ");


		//v->sl_volume = SLMILLIBEL_MAX;
		v->vol_fade_factor = 1.0F;

		v->fading_in = FALSE;
		v->fading_out = TRUE;


		cycle_looping_voice();

		v = (poly_sampler + current_looping_voice);
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

	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "SetPlayState() SL_PLAYSTATE_STOPPED called");

	result = (*v->bqPlayerBufferQueue)->Clear(v->bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "Clear() called");

    result = (*v->bqPlayerBufferQueue)->RegisterCallback(v->bqPlayerBufferQueue, looper_callback, (void *)s);
    assert(SL_RESULT_SUCCESS == result);
	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "RegisterCallback() called");

	// ここで音の大きさを0に設定して

    result = (*v->bqPlayerPlay)->SetPlayState(v->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "SetPlayState() SL_PLAYSTATE_PLAYING called");

	//__android_log_print(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "(int)samp->data_size(): %x", samp->data_size);

	result =	(*v->bqPlayerBufferQueue)->Enqueue(v->bqPlayerBufferQueue, s->buffer_data,  (int)s->data_size);
	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "Enqueue() called");

	// ここでフェードインを始めよ
//	v->sl_volume = SLMILLIBEL_MIN;
	v->vol_fade_factor = 0.0F;
	v->fading_in = TRUE;
	v->fading_out = FALSE;

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
//	for (i = 0; i < VOICE_COUNT; i++) {

		voice* v = &poly_sampler[i];
//		__android_log_print(ANDROID_LOG_DEBUG, "fade_automation", "poly_sampler[%d], in: %d, out: %d", i, v->fading_in, v->fading_out);

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
}



// 自動的なフェードのための関数
void voice_volume_factor(voice* v) {

	v->sl_volume = (1.0F - (v->vol_fade_factor * v->vol_auto_factor)) * SLMILLIBEL_MIN;
	__android_log_print(ANDROID_LOG_DEBUG, "voice_volume_factor", "v->vol_fade_factor: %f, v->sl_volume: %d", v->vol_fade_factor, v->sl_volume);

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

	__android_log_print(ANDROID_LOG_DEBUG, "float_to_slmillibel", "v: %d", vol);

	return vol;
}

SLpermille get_seg_permille(size_t seg) {

	return segment_pan_map[seg];

}

void init_seg_pan_map() {

	int i;
	for(i=0;i<TOTAL_NOTES; i++) {

		segment_pan_map[i] = ((i * SEG_PAN_AMT) - (((TOTAL_NOTES) * SEG_PAN_AMT) / 2)) + (SEG_PAN_AMT / 2);


		__android_log_print(ANDROID_LOG_DEBUG, "init_seg_pan_map", "segment_pan_map[%d]: %d", i, segment_pan_map[i]);
	}

}



// 書き直すべき
int enqueue_one_shot(sample_def * s, SLmillibel vol, SLpermille pan) {

	voice* v = get_next_free_voice();
	if (v == NULL)
	{
		__android_log_write(ANDROID_LOG_DEBUG, "enqueue_one_shot", "v == NULL: couldn't find a free buffer");
		return 0;
	}


	v->sample = s;
	v->current_chunk = 0;
//	v->current_chunk_size = BUFFER_SIZE;
	v->is_playing = TRUE;

	SLresult result;

//	set_voice_volume(v, vol, pan); // 一回しか必要ない


	set_voice_volume_pan(v, vol, pan);


	__android_log_print(ANDROID_LOG_DEBUG, "enqueue_one_shot", "v->current_chunk: %d", v->current_chunk);
//	__android_log_print(ANDROID_LOG_DEBUG, "enqueue_one_shot", "v->current_chunk_size: %d", v->current_chunk_size);
	__android_log_print(ANDROID_LOG_DEBUG, "enqueue_one_shot", "v->sample->data_size: %d", v->sample->data_size);
	__android_log_print(ANDROID_LOG_DEBUG, "enqueue_one_shot", "v->is_playing: %d", v->is_playing);

	//result = (*v->bqPlayerBufferQueue)->Enqueue(v->bqPlayerBufferQueue, get_next_data_chunk(v), v->current_chunk_size);

	if (SL_RESULT_SUCCESS != result) {
		return 0;
	}

	return 1;

}



// 一発的な音のため
void set_voice_volume_pan(voice* v, SLmillibel vol, SLpermille pan) { //入力スべき値は0から1の間

	__android_log_print(ANDROID_LOG_DEBUG, "set_voice_volume_pan", "vol: %d", vol);
	__android_log_print(ANDROID_LOG_DEBUG, "set_voice_volume_pan", "pan: %d", pan);

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
		__android_log_write(ANDROID_LOG_DEBUG, "get_next_data_chunk", "buffer_logic 1");

		v->sample = &silence_chunk;
		b = silence_chunk.buffer_data;
		v->is_playing = FALSE;

	} else if (v->current_chunk < v->sample->total_chunks && v->is_playing) {
//		__android_log_write(ANDROID_LOG_DEBUG, "get_next_data_chunk", "buffer_logic 2");

		b = v->sample->buffer_data + (v->current_chunk * BUFFER_SIZE_SHORT);
		v->current_chunk++;

	} else if (!v->is_playing) {
//		__android_log_write(ANDROID_LOG_DEBUG, "get_next_data_chunk", "buffer_logic 3");

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

	__android_log_print(ANDROID_LOG_DEBUG, "get_next_free_voice", "i: %d", i);

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
	__android_log_write(ANDROID_LOG_DEBUG, "shutdown_audio", "shutdown_audio() called");

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

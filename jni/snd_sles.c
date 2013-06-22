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
#include <android/log.h>;

#include "snd_sles.h"
#include "snd_asst.h"
#include "hon_type.h"
#include "snd_ctrl.h"


//int PLYCNT = 10;
// voice count
#define VOICE_COUNT 30
#define LOOPER_COUNT 2

#define SLMILLIBEL_MIN -4000
#define SLMILLIBEL_MAX 0

//#define VEL_SLMILLIBEL_MIN -1500
//#define VEL_SLMILLIBEL_RANGE 1700

#define VEL_SLMILLIBEL_MIN -3000
#define VEL_SLMILLIBEL_RANGE 3000




//long elapsed_buffer_tics = 0;

void cycle_looping_voice();
void cycle_oneshot_voice();


void loop_fade_out(voice* v);
void loop_fade_in(voice* v);


void buffer_chunk_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* voice);
void buffer_chunk_timer_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v);
void timing_test_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v);
//int timing_test_index = 0;

SLmillibel float_to_slmillibel(float sender_vel, float sender_range);


unsigned short* get_next_data_chunk(voice* voice);
voice* get_next_free_voice();


// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// フェードのため
static SLVolumeItf fade_in_itf;
static SLVolumeItf fade_out_itf;
int initial_loop = TRUE;

//typedef struct {
//	SLObjectItf bqPlayerObject;
//	SLPlayItf bqPlayerPlay;
//	SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
//	SLVolumeItf bqPlayerVolume;
//	SLBufferQueueState bqstate;
//} voice;
//


//static voice loop_poly[VOICE_COUNT];
static voice poly_sampler[VOICE_COUNT];


extern sample_def silence_chunk;

// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
		SL_I3DL2_ENVIRONMENT_PRESET_ARENA;




static int current_oneshot_voice = 4;
static int current_looping_voice = 0;

static int currently_xfading = 0;


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

//	v->is_fading = FALSE;

	v->fading_in = FALSE;
	v->fading_out=FALSE;

	v->is_playing = FALSE;
	v->current_chunk = 0;
	v->sample = NULL;

    SLresult result;

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

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

//		pthread_t fade_out;
		//pthread_create(&fade_out, NULL, loop_fade_out, (void*)voice);

		v->sl_volume = SLMILLIBEL_MAX;
		v->fading_in = FALSE;
		v->fading_out = TRUE;


		cycle_looping_voice();

		v = (poly_sampler + current_looping_voice);
	} else {

		// 浮動小数点型なボリューム操作機能を使うべきだ
		v->sl_volume = SLMILLIBEL_MIN;
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
//	pthread_t fade_in;
//	pthread_create(&fade_in, NULL, loop_fade_in, (void*)(poly_sampler + current_looping_voice));

	v->sl_volume = SLMILLIBEL_MIN;
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

void fade_automation() {

	int i;
	for (i = 0; i < LOOPER_COUNT; i++) {

		voice* v = &poly_sampler[i];
		//__android_log_print(ANDROID_LOG_DEBUG, "fade_automation", "poly_sampler[%d], in: %d, out: %d", i, v->fading_in, v->fading_out);

		if (v->fading_in) {
			loop_fade_in(v);
		}

		if (v->fading_out) {
			loop_fade_out(v);
		}
	}
}

void loop_fade_in(voice* v) {
	//__android_log_write(ANDROID_LOG_DEBUG, "fade_automation", "loop_fade_in() called");

		// これは問題？
		SLresult result;
		SLVolumeItf vol_itf = v->bqPlayerVolume;

		result = (*vol_itf)->SetVolumeLevel(vol_itf, v->sl_volume);
		assert(SL_RESULT_SUCCESS == result);

		v->sl_volume += 50;

		if (v->sl_volume >= SLMILLIBEL_MAX) {
			v->fading_in = FALSE;
		}
}

void loop_fade_out(voice* v) {
	//__android_log_write(ANDROID_LOG_DEBUG, "fade_automation", "loop_fade_out() called");

		SLresult result;
		SLVolumeItf vol_itf = v->bqPlayerVolume;

		result = (*vol_itf)->SetVolumeLevel(vol_itf, v->sl_volume);
		assert(SL_RESULT_SUCCESS == result);

		v->sl_volume -= 50;

		if (v->sl_volume <= SLMILLIBEL_MIN) {
			v->fading_out = FALSE;
		}
}






//
//// 一般的なTICRATEを使えばいいかもな
//void* loop_fade_out(void* looper) {
//
//	((voice*)looper)->is_fading = 1;
//
//	struct timespec ts;
//
//	__android_log_write(ANDROID_LOG_DEBUG, "loop_fade_in", "loop_fade_in() called");
//	SLresult result;
//
//	// ((oneshot_def*)samp)
//	SLVolumeItf fade_out_Itf = ((voice*)looper)->bqPlayerVolume;
//	//__android_log_print(ANDROID_LOG_DEBUG, "loop_fade_out", "current_looping_voice: %d", current_looping_voice);
//
//	SLmillibel out_vol = SLMILLIBEL_MAX;
//
//	// グローバルスレッドのほうが一番簡単
//	while (1) {
//
//		if (NULL != fade_out_Itf) {
//			result = (*fade_out_Itf)->SetVolumeLevel(fade_out_Itf, out_vol);
//			assert(SL_RESULT_SUCCESS == result);
//		}
//		out_vol -= 50;
//		//__android_log_print(ANDROID_LOG_DEBUG, "loop_fade_out", "out_vol: %d", out_vol);
//
//		usleep(100000);
//		if (out_vol <= SLMILLIBEL_MIN) {
//			((voice*)looper)->is_fading = 0;
//			break;
//		}
//	}
//	return NULL;
//}


//void* loop_fade_in(void* looper) {
//
//	((voice*)looper)->is_fading = 1;
//
//	__android_log_write(ANDROID_LOG_DEBUG, "loop_fade_in", "loop_fade_in() called");
//
//		struct timespec ts;
//
//	SLresult result;
//	SLVolumeItf fade_in_Itf =  ((voice*)looper)->bqPlayerVolume;
//	__android_log_print(ANDROID_LOG_DEBUG, "loop_fade_in", "current_looping_voice: %d", current_looping_voice);
//
//	SLmillibel in_vol = SLMILLIBEL_MIN;
//
//		while(1) {
//
//			if (NULL != fade_in_Itf) {
//				result = (*fade_in_Itf)->SetVolumeLevel(fade_in_Itf, in_vol);
//				assert(SL_RESULT_SUCCESS == result);
//			}
//			in_vol += 50;
//			//__android_log_print(ANDROID_LOG_DEBUG, "loop_fade_in", "in_vol: %d", in_vol);
//
//			usleep(100000);
//			if (in_vol >= SLMILLIBEL_MAX) {
//
//				((voice*)looper)->is_fading = 0;
//				break;
//			}
//	}
//	return NULL;
//}



void set_voice_volume(voice* v, float vol) { //入力スべき値は0から1の間

	__android_log_print(ANDROID_LOG_DEBUG, "set_voice_volume", "vol: %f", vol);

	SLmillibel sl_millibel = float_to_slmillibel(vol, 1.0F);
	__android_log_print(ANDROID_LOG_DEBUG, "set_voice_volume", "sl_millibel: %d", sl_millibel);

	SLresult result;
	// 書き直す
	// SLVolumeItf volumeItf = poly_sampler[current_oneshot_voice].bqPlayerVolume;

	SLVolumeItf volumeItf = v->bqPlayerVolume;

	if (NULL != volumeItf) {
		result = (*volumeItf)->SetVolumeLevel(volumeItf, sl_millibel);
		assert(SL_RESULT_SUCCESS == result);
	}

}

//	public static float calcToRangeFM(float sndrval, float sndrrng) {
//		float rtnval =(sndrval * (FM_FADE_RNG/sndrrng)) + FM_FADE_MIN;
//		return rtnval;
//	}
//


SLmillibel float_to_slmillibel(float sender_vel, float sender_range) {

	// エラーが発生可能性を確認しなきゃ

	SLmillibel vol = (sender_vel * (VEL_SLMILLIBEL_RANGE/sender_range)) + VEL_SLMILLIBEL_MIN;
// 速度を上げるため、こういう式(VEL_SLMILLIBEL_RANGE/sender_range)を既に計算スべし？

// 0以上なら制御てきに０に設定
if (vol > 0)	vol = 0;

	__android_log_print(ANDROID_LOG_DEBUG, "float_to_slmillibel", "v: %d", vol);

	return vol;


}

// 書き直すべき
int enqueue_one_shot(sample_def * s, float vel) {

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

	set_voice_volume(v, vel); // 一回しか必要ない

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

	SLresult result;

//	result = (*buffer_queue)->Enqueue(buffer_queue, get_next_data_chunk((voice*)v), ((voice*)v)->current_chunk_size);
	result = (*buffer_queue)->Enqueue(buffer_queue, get_next_data_chunk((voice*)v), BUFFER_SIZE);


	assert(SL_RESULT_SUCCESS == result);
}

void buffer_chunk_timer_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* v) {
//	__android_log_write(ANDROID_LOG_DEBUG, "buffer_chunk_timer_callback", "buffer_chunk_timer_callback() called");
//total_tic_counter++;
	tic_counter();
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
    // destroy buffer queue audio player object, and invalidate all associated interfaces
//    if (bqPlayerObject != NULL) {
//        (*bqPlayerObject)->Destroy(bqPlayerObject);
//        bqPlayerObject = NULL;
//        bqPlayerPlay = NULL;
//        bqPlayerBufferQueue = NULL;/*
//        bqPlayerEffectSend = NULL;
//        bqPlayerMuteSolo = NULL;*/
//        bqPlayerVolume = NULL;
//    }

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

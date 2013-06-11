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


//int PLYCNT = 10;
// voice count
#define VOICE_COUNT 30
#define LOOPER_COUNT 2

#define SLMILLIBEL_MIN -4000
#define SLMILLIBEL_MAX 0

#define VEL_SLMILLIBEL_MIN -1500
#define VEL_SLMILLIBEL_RANGE 1700


void cycle_looping_voice();
void cycle_oneshot_voice();






// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

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



// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
		SL_I3DL2_ENVIRONMENT_PRESET_ARENA;




static int current_oneshot_voice = 4;
static int current_looping_voice = 0;

static int currently_xfading = 0;


// 必要の理由：ループのcallbackを登録するため
//extern oneshot_def looping_samples[];


//void* fadeInThread(void* args);
/*


# define HEADER_SIZE 44

typedef struct {
	char* file_name;
	int midi_number;
	unsigned sample_size;

	size_t data_size;


	unsigned short* buffer_header;
	unsigned short* buffer_data;


} oneshot_def;


// 今の立場4個だけでいいかも
oneshot_def oneshot_samples[] = {

		{"test_p_01.wav", 48, 0, 0, NULL, NULL},
		{"test_p_01.wav", 49, 0, 0, NULL, NULL},
		{"test_p_01.wav", 50, 0, 0, NULL, NULL},

};


void load_all_buffers(AAssetManager* mgr) {


	int success;//必要ない

	int i;

	for (i = 0; i < sizeof(oneshot_samples); i++) {

		success = open_asset(
				mgr,
				oneshot_samples[i].file_name,
				oneshot_samples[i].buffer_header,
				oneshot_samples[i].buffer_data,
				*oneshot_samples[i].data_size);

		if (success == 0) break;
	}
}


*/

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


//// this callback handler is called every time a buffer finishes playing
////only required for looping... no need to register a cb for one shot (might be though if buffers are too small).
//void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
//
//	__android_log_write(ANDROID_LOG_DEBUG, "NDK_debug_tag: ",
//			"bqPlayerCallback() called");
//
//	//assert(bq == bqPlayerBufferQueue);
//	//assert(NULL == context);
//
//	SLresult result;
//	//result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, bufd[0], nextSize[0]);
//
//
//
//
//	result = (*bq)->Enqueue(bq, context->bq[0], nextSize[0]);
//
//	//result = (*bqPlayerBufferQueue)->GetState(bqPlayerBufferQueue, &bqstate);
//
///*	__android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag: ",
//			"bqstate.count %d", bqstate.count);*/
//
//	// the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
//	// which for this code example would indicate a programming error
//	assert(SL_RESULT_SUCCESS == result);
//}

// this callback handler is called every time a buffer finishes playing
//only required for looping... no need to register a cb for one shot (might be though if buffers are too small).
void looper_callback(SLAndroidSimpleBufferQueueItf buffer_queue, void* samp) {

	__android_log_write(ANDROID_LOG_DEBUG, "NDK_debug_tag: ",
			"looper_callback called");

	//assert(bq == bqPlayerBufferQueue);
	//assert(NULL == context);

	SLresult result;

	result = (*buffer_queue)->Enqueue(buffer_queue, ((oneshot_def*)samp)->buffer_data, ((oneshot_def*)samp)->data_size);

	//result = (*bqPlayerBufferQueue)->GetState(bqPlayerBufferQueue, &bqstate);

/*	__android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag: ",
			"bqstate.count %d", bqstate.count);*/

	// the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
	// which for this code example would indicate a programming error
	assert(SL_RESULT_SUCCESS == result);
}





void init_voice(voice* voice) {

voice->is_fading = 0;


	//SLObjectItf* bqTest = &(voice->bqPlayerObject);


	// int * const * - a pointer to a const pointer to an int

	// typedef const struct SLObjectItf_ * const * SLObjectItf;
	// by my understanding SLObjectItf IS a pointer to a const pointer to SLObjectItf_(where SL magic happens?)

/*	Like pretty much everyone pointed out:

	[18.5] What's the difference between "const Fred* p", "Fred* const p" and "const Fred* const p"?

	You have to read pointer declarations right-to-left.

	const Fred* p means "p points to a Fred that is const" — that is, the Fred object can't be changed via p.
	Fred* const p means "p is a const pointer to a Fred" — that is, you can change the Fred object via p, but you can't change the pointer p itself.
	const Fred* const p means "p is a const pointer to a const Fred" — that is, you can't change the pointer p itself, nor can you change the Fred object via p.
	*/
//	SLObjectItf bqPlayerObj = voice->bqPlayerObject;
//	SLAndroidSimpleBufferQueueItf bqPlyBufQ = voice->bqPlayerBufferQueue;
//	SLPlayItf bqPlayerPlay = voice->bqPlayerPlay;


/*	struct SLObjectItf_ {
		SLresult (*Realize) (
			SLObjectItf self,
			SLboolean async
		);*/

    SLresult result;

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "SLDataFormat_PCM format_pcm ");
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

	__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "SLDataSource audioSrc");

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

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &voice->bqPlayerObject, &audioSrc, &audioSnk, 3, ids, req);
    assert(SL_RESULT_SUCCESS == result);


	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", " (*engineEngine)->CreateAudioPlayer");
    // realize the player
    result = (*voice->bqPlayerObject)->Realize(voice->bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "(*bqPlayerObj)->Realize");
    // get the play interface
    result = (*voice->bqPlayerObject)->GetInterface(voice->bqPlayerObject, SL_IID_PLAY, &(voice->bqPlayerPlay));
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "GetInterface(bqPlayerObj, SL_IID_PLAY, &(voice->bqPlayerPlay))");

    // get the volume interface
    result = (*voice->bqPlayerObject)->GetInterface(voice->bqPlayerObject, SL_IID_VOLUME, &(voice->bqPlayerVolume));
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "(*bqPlayerObj)->GetInterface(bqPlayerObj, SL_IID_VOLUME, &(voice->bqPlayerVolume)");




	// get the buffer queue interface
    result = (*voice->bqPlayerObject)->GetInterface(voice->bqPlayerObject, SL_IID_BUFFERQUEUE, &(voice->bqPlayerBufferQueue));
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "initBufferQueuePlayer", "(*bqPlayerObj)->GetInterface(bqPlayerObj, SL_IID_BUFFERQUEUE, &(voice->bqPlayerBufferQueue)");


/*
	// register callback on the buffer queue- ここで問題が起きる
    result = (*voice->bqPlayerBufferQueue)->RegisterCallback(voice->bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "(*voice->bqPlayerBufferQueue)->RegisterCallback(voice->bqPlayerBufferQueue, bqPlayerCallback, NULL);");
*/


    // set the player's state to playing - ここで問題が起きる
    result = (*voice->bqPlayerPlay)->SetPlayState(voice->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "init_voice", "(*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING)");
}

// buffer queue players を作り出すため

void init_all_voices() {

	__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "void init_all_voices() called");

	int i;

	for (i = 0; i < VOICE_COUNT; i++) {


		__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "void initPolyphony() i: %d", i);

	init_voice(&poly_sampler[i]);

	}


	__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "void init_all_voices() finished");

}

/*

// ループのサンプルを登録するために、再生する前にlooper_callbackにbuffer_dataを指示しなきゃ
// 問題は再生中にcallbackバッファーを変更しなきゃ
// なのでenqueue_seamless_loopを呼ぶと再生する前にlooper_callbackを登場新きゃ
void reg_looper_callback_voices() {

	int i;

	for (i = 0; i < LOOPER_COUNT; i++) {

		SLresult result;
		// register callback on the buffer queue- ここで問題が起きる
		result = (&poly_sampler[i]->bqPlayerBufferQueue)->RegisterCallback(
				poly_sampler[i]->bqPlayerBufferQueue, looper_callback, looping_samples[i]);
		assert(SL_RESULT_SUCCESS == result);

		__android_log_write(ANDROID_LOG_DEBUG, "init_voice",
				"(*voice->bqPlayerBufferQueue)->RegisterCallback(voice->bqPlayerBufferQueue, bqPlayerCallback, NULL);");
	}
}

*/



//
///*olean Java_nz_kapsy_yakunitatsuplayer_MainActivity_createAssetOpenRead(
//		JNIEnv* env, jclass clazz, jobject assetManager, jstring filename, jint buffernum) {
//	*/
//int open_asset_old(AAssetManager* mgr, char* filename, int buffernum) {
//
///*	// convert Java string to UTF-8
//	    const char* utf8 = (*env)->GetStringUTFChars(env, filename, NULL);
//	 assert(NULL != utf8);*/
//
//	assert(NULL != mgr);
//	AAsset *asset = AAssetManager_open(mgr, filename, AASSET_MODE_BUFFER);
//
//	__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "AAssetManager_open");
//	/*
//	 size_t length = AAsset_getLength(asset);
//	 __android_log_print(ANDROID_LOG_DEBUG, "ASSET", "length: %d", (int) length);
//	 */
//
//	//char* buffer = (char*) malloc(length);
//	if (NULL == asset) {
//		__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "Asset not found, loading aborted.");
//		return JNI_FALSE;
//	}
//
//	bufh[buffernum] = (unsigned short*) malloc(header_size);
//	AAsset_read(asset, bufh[buffernum] , header_size);
//
//	// 変数の週類はポインターである
//	unsigned short* fmttype;
//	unsigned long* databytes;
//
//	//fmttype =  *(buffer + 10);
//	//fmttype = &buffer[10];
//
//	fmttype = (bufh[buffernum]  + 10);
//	if (*fmttype != 0x1) {
//		__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "*fmttype not PCM, loading aborted.");
//		return JNI_FALSE;
//	}
//
//	databytes = (bufh[buffernum]  + 20);
//	size_t datasize_t = *databytes;
//
//	bufd[buffernum]  = (unsigned short*) malloc(datasize_t);
//	AAsset_seek(asset, header_size, SEEK_SET);
//	AAsset_read(asset, bufd[buffernum] , datasize_t);
//
//	//bufferc = (unsigned char*) malloc(length);
//	/*unsigned char buffer[length];*/
//
//	// release the Java string and UTF-8
//	// (*env)->ReleaseStringUTFChars(env, filename, utf8);
//	// the asset might not be found
//	// Read the file contents in a loop
//	/*    while (((count = AAsset_read(&asset, buffer, sizeof(buffer))) > 0)) {
//	 // buffer will have file contents...display or process data in buffer
//	 }*/
//
//	///*	unsigned long samplingrate;
//	//	memcpy(&samplingrate, header_buf + 24, sizeof(samplingrate));
//	//
//	//	printf("\n%d", samplingrate);*/
//
//	//ファイルがRIFF形式であるか
//	/*	if(strncmp(buffer, "RIFF", 4)){
//	 __android_log_wr
//
//	 if(strncmp(buffer, , 4)){
//	 __android_log_write(ANDROID_LOG_DEBUG, "ASSET", "Not a RIFF file");
//	 } */
//
//	/*	じつは、ポインタ変数名の前にアスタリスク(*)をつけて参照すると
//	 ポインタ変数が格納しているメモリアドレスの内容を参照します
//
//	 アスタリスクをつけない後者のprintf()関数の po では、格納されているメモリアドレスを指します*/
//
//	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "fmttype: %x", fmttype);
//	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "*fmttype: %x", *fmttype);
//	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "databytes: %x", databytes);
//	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "databytes: %x", *databytes);
//
//	AAsset_close(asset);
//
//	nextSize[buffernum] = datasize_t; // nextSizeはそんなに必要なの？
//
//	// データを確認するため
///*	int idx = nextSize / 2;
//	int ct = 0;
//	for (ct = 0; ct < 100; ct++) {
//		__android_log_print(ANDROID_LOG_DEBUG, "ASSET",
//				"bufd[%d] (start) x: %x c: %c", ct, bufd[ct], bufd[ct]);
//	}
//	for (ct = (idx - 64); ct < idx; ct++) {
//		__android_log_print(ANDROID_LOG_DEBUG, "ASSET",
//				"bufd[%d] (finish) x: %x c: %c", ct, bufd[ct], bufd[ct]);
//	}*/
//
//	/*    // open asset as file descriptor
//	 off_t start, length;
//	 int fd = AAsset_openFileDescriptor(asset, &start, &length);
//	 assert(0 <= fd);
//	 AAsset_close(asset); */
//
//	return JNI_TRUE;
//}



//int enqueue_seamless_loop(oneshot_def* samp) {
//
//	SLresult result;
//
//	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "enqueue_seamless_loop() called");
//
//	// ここで今の再生中音をフェードアウトし
//	SLAndroidSimpleBufferQueueState current_queue_state ;
//	SLuint32 state;
//
//	result =  (*poly_sampler[current_looping_voice].bqPlayerBufferQueue)->GetState(poly_sampler[current_looping_voice].bqPlayerBufferQueue, &current_queue_state);
//	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "GetState() called");
//
//	if (current_queue_state.count > 0) {
//
//		__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "(current_queue_state.count > 0)");
//
//		pthread_t fade_out;
//		pthread_create(&fade_out, NULL, loop_fade_out, (void*)(poly_sampler + current_looping_voice));
//
//		cycle_looping_voice();
//	}
//
//    result = (*poly_sampler[current_looping_voice].bqPlayerPlay)->SetPlayState(poly_sampler[current_looping_voice].bqPlayerPlay, SL_PLAYSTATE_STOPPED);
//    assert(SL_RESULT_SUCCESS == result);
//
//	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "SetPlayState() SL_PLAYSTATE_STOPPED called");
//
//	result = (*poly_sampler[current_looping_voice].bqPlayerBufferQueue)->Clear(poly_sampler[current_looping_voice].bqPlayerBufferQueue);
//    assert(SL_RESULT_SUCCESS == result);
//	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "Clear() called");
//
//    result = (*poly_sampler[current_looping_voice].bqPlayerBufferQueue)->RegisterCallback(poly_sampler[current_looping_voice].bqPlayerBufferQueue, looper_callback, (void *)samp);
//    assert(SL_RESULT_SUCCESS == result);
//	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "RegisterCallback() called");
//
//	// ここで音の大きさを0に設定して
//
//    result = (*poly_sampler[current_looping_voice].bqPlayerPlay)->SetPlayState(poly_sampler[current_looping_voice].bqPlayerPlay, SL_PLAYSTATE_PLAYING);
//    assert(SL_RESULT_SUCCESS == result);
//	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "SetPlayState() SL_PLAYSTATE_PLAYING called");
//
//	//__android_log_print(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "(int)samp->data_size(): %x", samp->data_size);
//
//	result =	(*poly_sampler[current_looping_voice].bqPlayerBufferQueue)->Enqueue(poly_sampler[current_looping_voice].bqPlayerBufferQueue, samp->buffer_data,  (int)samp->data_size);
//	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "Enqueue() called");
//
//	// ここでフェードインを始めよ
//	pthread_t fade_in;
//	pthread_create(&fade_in, NULL, loop_fade_in, (void*)(poly_sampler + current_looping_voice));
//
//	if (SL_RESULT_SUCCESS != result) {
//		return 0;
//	}
//
//	return 1;
//}
//

// 今はフェード中なら返り値は1
int current_voice_fading() {
	voice* voice = (poly_sampler + current_looping_voice);
	return voice->is_fading;
}

int enqueue_seamless_loop(oneshot_def* samp) {

	SLresult result;

	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "enqueue_seamless_loop() called");

	// ここで今の再生中音をフェードアウトし
	SLAndroidSimpleBufferQueueState current_queue_state ;
	SLuint32 state;

	voice* voice = (poly_sampler + current_looping_voice);

// ここでも駄目のかな

	// まずは関数一つが必要かも
//	if(voice->is_fading != 0){

//		__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "(voice->is_fading != 0)");
//		return 0;
//	}


	// ポインターのポインターかもなぁ
	result = (*voice->bqPlayerBufferQueue)->GetState(voice->bqPlayerBufferQueue, &current_queue_state);


	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "GetState() called");

	if (current_queue_state.count > 0) {

		__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "(current_queue_state.count > 0)");

		pthread_t fade_out;
		pthread_create(&fade_out, NULL, loop_fade_out, (void*)voice);

		cycle_looping_voice();

		voice = (poly_sampler + current_looping_voice);
	}

    result = (*voice->bqPlayerPlay)->SetPlayState(voice->bqPlayerPlay, SL_PLAYSTATE_STOPPED);
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "SetPlayState() SL_PLAYSTATE_STOPPED called");

	result = (*voice->bqPlayerBufferQueue)->Clear(voice->bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "Clear() called");

    result = (*voice->bqPlayerBufferQueue)->RegisterCallback(voice->bqPlayerBufferQueue, looper_callback, (void *)samp);
    assert(SL_RESULT_SUCCESS == result);
	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "RegisterCallback() called");

	// ここで音の大きさを0に設定して

    result = (*voice->bqPlayerPlay)->SetPlayState(voice->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "SetPlayState() SL_PLAYSTATE_PLAYING called");

	//__android_log_print(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "(int)samp->data_size(): %x", samp->data_size);

	result =	(*voice->bqPlayerBufferQueue)->Enqueue(voice->bqPlayerBufferQueue, samp->buffer_data,  (int)samp->data_size);
	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "Enqueue() called");

	// ここでフェードインを始めよ
	pthread_t fade_in;
	pthread_create(&fade_in, NULL, loop_fade_in, (void*)(poly_sampler + current_looping_voice));

	if (SL_RESULT_SUCCESS != result) {
		return 0;
	}

	return 1;
}

// 一般的なTICRATEを使えばいいかもな
void* loop_fade_out(void* looper) {

	((voice*)looper)->is_fading = 1;

	struct timespec ts;

	__android_log_write(ANDROID_LOG_DEBUG, "loop_fade_in", "loop_fade_in() called");
	SLresult result;

	// ((oneshot_def*)samp)
	SLVolumeItf fade_out_Itf = ((voice*)looper)->bqPlayerVolume;
	//__android_log_print(ANDROID_LOG_DEBUG, "loop_fade_out", "current_looping_voice: %d", current_looping_voice);

	SLmillibel out_vol = SLMILLIBEL_MAX;

	// グローバルスレッドのほうが一番簡単
	while (1) {

		if (NULL != fade_out_Itf) {
			result = (*fade_out_Itf)->SetVolumeLevel(fade_out_Itf, out_vol);
			assert(SL_RESULT_SUCCESS == result);
		}
		out_vol -= 50;
		//__android_log_print(ANDROID_LOG_DEBUG, "loop_fade_out", "out_vol: %d", out_vol);

		usleep(100000);
		if (out_vol <= SLMILLIBEL_MIN) {
			((voice*)looper)->is_fading = 0;
			break;
		}
	}
	return NULL;
}


void* loop_fade_in(void* looper) {

	((voice*)looper)->is_fading = 1;

	__android_log_write(ANDROID_LOG_DEBUG, "loop_fade_in", "loop_fade_in() called");

		struct timespec ts;

	SLresult result;
	SLVolumeItf fade_in_Itf =  ((voice*)looper)->bqPlayerVolume;
	__android_log_print(ANDROID_LOG_DEBUG, "loop_fade_in", "current_looping_voice: %d", current_looping_voice);

	SLmillibel in_vol = SLMILLIBEL_MIN;

		while(1) {

			if (NULL != fade_in_Itf) {
				result = (*fade_in_Itf)->SetVolumeLevel(fade_in_Itf, in_vol);
				assert(SL_RESULT_SUCCESS == result);
			}
			in_vol += 50;
			//__android_log_print(ANDROID_LOG_DEBUG, "loop_fade_in", "in_vol: %d", in_vol);

			usleep(100000);
			if (in_vol >= SLMILLIBEL_MAX) {

				((voice*)looper)->is_fading = 0;
				break;
			}
	}
	return NULL;
}


//void fadeInBqPlayer() {
//	pthread_t th;
//	// スレッド作成と起動
//	pthread_create(&th, NULL, fadeInThread, (void *)NULL);
//
//}
//
//void* fadeInThread(void* args) {
//
//	struct timespec ts;
//
//	SLresult result;
//	SLVolumeItf volumeItf = bqPlayerVolume;
//	SLmillibel currvol = -5000;
//
//
//	int counter = 0;
//	while (1) {
//
//		if (NULL != volumeItf) {
//			result = (*volumeItf)->SetVolumeLevel(volumeItf, currvol);
//			assert(SL_RESULT_SUCCESS == result);
//		}
//		currvol += 50;
//
//		__android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag", "counter cycle: %d", counter);
//		__android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag", "currvol: %d", currvol);
//
//		usleep(100000);
//
//		counter++;
//		if (currvol == 0)
//			break;
//	}
//	return NULL;
//}
//



void set_voice_volume(voice* voice, float vol) { //入力スべき値は0から1の間

	__android_log_print(ANDROID_LOG_DEBUG, "set_voice_volume", "vol: %f", vol);

	SLmillibel sl_millibel = float_to_slmillibel(vol, 1.0F);
	__android_log_print(ANDROID_LOG_DEBUG, "set_voice_volume", "sl_millibel: %d", sl_millibel);

	SLresult result;
	// 書き直す
	// SLVolumeItf volumeItf = poly_sampler[current_oneshot_voice].bqPlayerVolume;

	SLVolumeItf volumeItf = voice->bqPlayerVolume;

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
int enqueue_one_shot(oneshot_def * samp, float vel) {


	__android_log_print(ANDROID_LOG_DEBUG, "enqueue_one_shot", "vel: %f", vel);


	cycle_oneshot_voice();

	voice* voice = (poly_sampler + current_oneshot_voice);


	SLresult result;

//	result = (*voice->bqPlayerBufferQueue)->GetState(voice->bqPlayerBufferQueue, &current_queue_state);

	set_voice_volume(voice, vel);
	result = (*voice->bqPlayerBufferQueue)->Clear(voice->bqPlayerBufferQueue);




	__android_log_print(ANDROID_LOG_DEBUG, "enqueue_one_shot", "data_size x: %x, d: %d", samp->data_size, samp->data_size);

	__android_log_print(ANDROID_LOG_DEBUG, "enqueue_one_shot", "*data_size x: %x, d: %d", &(samp->data_size), &(samp->data_size));

	result = (*voice->bqPlayerBufferQueue)->Enqueue(voice->bqPlayerBufferQueue, samp->buffer_data, samp->data_size); // 何でdata_size？何で*data_sizeは駄目だ？わかんねぇ
	if (SL_RESULT_SUCCESS != result) {
		return 0;
	}

	__android_log_write(ANDROID_LOG_DEBUG, "enqueue_one_shot", "result = (*voice->bqPlayerBufferQueue)");
	return 1;

}





//
//
//// 書き直すべき
//int enqueue_one_shot(unsigned short* buffer_data, size_t data_size) {
//
//	cycle_oneshot_voice();
//
//	SLresult result;
//	result = (*poly_sampler[current_oneshot_voice].bqPlayerBufferQueue)->Clear(poly_sampler[current_oneshot_voice].bqPlayerBufferQueue);
//
//	__android_log_print(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "data_size x: %x, d: %d", data_size, data_size);
//
//	__android_log_print(ANDROID_LOG_DEBUG, "enqueue_seamless_loop", "*data_size x: %x, d: %d", &data_size, &data_size);
//
//	result = (*poly_sampler[current_oneshot_voice].bqPlayerBufferQueue)->Enqueue(poly_sampler[current_oneshot_voice].bqPlayerBufferQueue, buffer_data, data_size); // 何でdata_size？何で*data_sizeは駄目だ？わかんねぇ
//	if (SL_RESULT_SUCCESS != result) {
//		return 0;
//	}
//
//	__android_log_write(ANDROID_LOG_DEBUG, "NDK_debug_tag", "result = (*poly_sampler[current_oneshot_voice].bqPlayerBufferQueue)");
//	return 1;
//
//}


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

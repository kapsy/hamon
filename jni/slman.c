/*
 * slman.c / OpenSL Manager
 *
 *  Created on: 2013/05/31
 *      Author: Michael
 *      OpenSLの処理のみのファイルです
 */


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

#include "slman.h"


//int PLYCNT = 10;
// voice count
#define VOXCNT 15


// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

typedef struct {
	SLObjectItf bqPlayerObject;
	SLPlayItf bqPlayerPlay;
	SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
	SLVolumeItf bqPlayerVolume;
	SLBufferQueueState bqstate;
} voice;



static voice loop_poly[VOXCNT];
static voice shot_poly[VOXCNT];



/*
// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
//static SLEffectSendItf bqPlayerEffectSend;
//static SLMuteSoloItf bqPlayerMuteSolo;
static SLVolumeItf bqPlayerVolume;
static SLBufferQueueState bqstate;

*/


// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
		SL_I3DL2_ENVIRONMENT_PRESET_ARENA;

// one shot samples
static size_t bufhsize = 44;


//going to put everything sample wise in one big buffer array...
/*// looping samples
// header buffer
unsigned short* ls_bufh[3];
// data buffer
unsigned short* ls_ bufd[3];
static unsigned nextSize[3];*/

// one shot samples
// header buffer
unsigned short* bufh[3];
// data buffer
unsigned short* bufd[3];
static unsigned nextSize[3];

static int curroneshotvoice = 0;




void* fadeInThread(void* args);



void loadAllBuffers(AAssetManager* mgr) {


	//int openSLAsset(AAssetManager* mgr, char* filename, int buffernum) {


/*
	int success = 1;

	while(success == 1) {

		success = openSLAsset(mgr, "test_p_01.wav", 0);


		break;


	}

*/

		int success = openSLAsset(mgr, "test_p_01.wav", 0);


}



void createSLEngine()
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
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {

	__android_log_write(ANDROID_LOG_DEBUG, "NDK_debug_tag: ",
			"bqPlayerCallback() called");

	//assert(bq == bqPlayerBufferQueue);
	//assert(NULL == context);

	SLresult result;
	//result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, bufd[0], nextSize[0]);

	result = (*bq)->Enqueue(bq, bufd[0], nextSize[0]);

	//result = (*bqPlayerBufferQueue)->GetState(bqPlayerBufferQueue, &bqstate);

/*	__android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag: ",
			"bqstate.count %d", bqstate.count);*/

	// the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
	// which for this code example would indicate a programming error
	assert(SL_RESULT_SUCCESS == result);
}

void initBufferQueuePlayer(voice* voice) {

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

	__android_log_write(ANDROID_LOG_DEBUG, "initBufferQueuePlayer", "SLDataFormat_PCM format_pcm ");
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

	__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "SLDataSource audioSrc");

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
	__android_log_write(ANDROID_LOG_DEBUG, "initBufferQueuePlayer", "SLDataSink audioSnk");

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};

	__android_log_write(ANDROID_LOG_DEBUG, "initBufferQueuePlayer", "const SLboolean req[3]");

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &voice->bqPlayerObject, &audioSrc, &audioSnk, 3, ids, req);
    assert(SL_RESULT_SUCCESS == result);


	__android_log_write(ANDROID_LOG_DEBUG, "initBufferQueuePlayer", " (*engineEngine)->CreateAudioPlayer");
    // realize the player
    result = (*voice->bqPlayerObject)->Realize(voice->bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "initBufferQueuePlayer", "(*bqPlayerObj)->Realize");
    // get the play interface
    result = (*voice->bqPlayerObject)->GetInterface(voice->bqPlayerObject, SL_IID_PLAY, &(voice->bqPlayerPlay));
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "initBufferQueuePlayer", "GetInterface(bqPlayerObj, SL_IID_PLAY, &(voice->bqPlayerPlay))");

    // get the volume interface
    result = (*voice->bqPlayerObject)->GetInterface(voice->bqPlayerObject, SL_IID_VOLUME, &(voice->bqPlayerVolume));
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "initBufferQueuePlayer", "(*bqPlayerObj)->GetInterface(bqPlayerObj, SL_IID_VOLUME, &(voice->bqPlayerVolume)");




	// get the buffer queue interface
    result = (*voice->bqPlayerObject)->GetInterface(voice->bqPlayerObject, SL_IID_BUFFERQUEUE, &(voice->bqPlayerBufferQueue));
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "initBufferQueuePlayer", "(*bqPlayerObj)->GetInterface(bqPlayerObj, SL_IID_BUFFERQUEUE, &(voice->bqPlayerBufferQueue)");


/*
	// register callback on the buffer queue- ここで問題が起きる
    result = (*voice->bqPlayerBufferQueue)->RegisterCallback(voice->bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "initBufferQueuePlayer", "(*voice->bqPlayerBufferQueue)->RegisterCallback(voice->bqPlayerBufferQueue, bqPlayerCallback, NULL);");
*/


    // set the player's state to playing - ここで問題が起きる
    result = (*voice->bqPlayerPlay)->SetPlayState(voice->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);

	__android_log_write(ANDROID_LOG_DEBUG, "initBufferQueuePlayer", "(*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING)");
}

// populate all voices with buffer queue players
void initPolyphony() {

	__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "void initPolyphony()");

	int i;

	for (i = 0; i < VOXCNT; i++) {


		__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "void initPolyphony() i: %d", i);

		initBufferQueuePlayer(&loop_poly[i]);
		initBufferQueuePlayer(&shot_poly[i]);
	}
}



/*olean Java_nz_kapsy_yakunitatsuplayer_MainActivity_createAssetOpenRead(
		JNIEnv* env, jclass clazz, jobject assetManager, jstring filename, jint buffernum) {
	*/
int openSLAsset(AAssetManager* mgr, char* filename, int buffernum) {

/*	// convert Java string to UTF-8
	    const char* utf8 = (*env)->GetStringUTFChars(env, filename, NULL);
	 assert(NULL != utf8);*/

	// use asset manager to open asset by filename
	//AAssetManager* mgr;// = AAssetManager_fromJava(env, assetManager);
	assert(NULL != mgr);
	AAsset *asset = AAssetManager_open(mgr, filename, AASSET_MODE_BUFFER);

	__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "AAssetManager_open");
	/*
	 size_t length = AAsset_getLength(asset);
	 __android_log_print(ANDROID_LOG_DEBUG, "ASSET", "length: %d", (int) length);
	 */

	//char* buffer = (char*) malloc(length);
	if (NULL == asset) {
		__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "Asset not found, loading aborted.");
		return JNI_FALSE;
	}

	bufh[buffernum] = (unsigned short*) malloc(bufhsize);
	AAsset_read(asset, bufh[buffernum] , bufhsize);

	// 変数の週類はポインターである
	unsigned short* fmttype;
	unsigned long* databytes;

	//fmttype =  *(buffer + 10);
	//fmttype = &buffer[10];

	fmttype = (bufh[buffernum]  + 10);
	if (*fmttype != 0x1) {
		__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "*fmttype not PCM, loading aborted.");
		return JNI_FALSE;
	}

	databytes = (bufh[buffernum]  + 20);
	size_t datasize_t = *databytes;

	bufd[buffernum]  = (unsigned short*) malloc(datasize_t);
	AAsset_seek(asset, bufhsize, SEEK_SET);
	AAsset_read(asset, bufd[buffernum] , datasize_t);

	//bufferc = (unsigned char*) malloc(length);
	/*unsigned char buffer[length];*/

	// release the Java string and UTF-8
	// (*env)->ReleaseStringUTFChars(env, filename, utf8);
	// the asset might not be found
	// Read the file contents in a loop
	/*    while (((count = AAsset_read(&asset, buffer, sizeof(buffer))) > 0)) {
	 // buffer will have file contents...display or process data in buffer
	 }*/

	///*	unsigned long samplingrate;
	//	memcpy(&samplingrate, header_buf + 24, sizeof(samplingrate));
	//
	//	printf("\n%d", samplingrate);*/

	//ファイルがRIFF形式であるか
	/*	if(strncmp(buffer, "RIFF", 4)){
	 __android_log_wr

	 if(strncmp(buffer, , 4)){
	 __android_log_write(ANDROID_LOG_DEBUG, "ASSET", "Not a RIFF file");
	 } */

	/*	じつは、ポインタ変数名の前にアスタリスク(*)をつけて参照すると
	 ポインタ変数が格納しているメモリアドレスの内容を参照します

	 アスタリスクをつけない後者のprintf()関数の po では、格納されているメモリアドレスを指します*/

	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "fmttype: %x", fmttype);
	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "*fmttype: %x", *fmttype);
	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "databytes: %x", databytes);
	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "databytes: %x", *databytes);

	AAsset_close(asset);

	nextSize[buffernum] = datasize_t; // nextSizeはそんなに必要なの？

	// データを確認するため
/*	int idx = nextSize / 2;
	int ct = 0;
	for (ct = 0; ct < 100; ct++) {
		__android_log_print(ANDROID_LOG_DEBUG, "ASSET",
				"bufd[%d] (start) x: %x c: %c", ct, bufd[ct], bufd[ct]);
	}
	for (ct = (idx - 64); ct < idx; ct++) {
		__android_log_print(ANDROID_LOG_DEBUG, "ASSET",
				"bufd[%d] (finish) x: %x c: %c", ct, bufd[ct], bufd[ct]);
	}*/

	/*    // open asset as file descriptor
	 off_t start, length;
	 int fd = AAsset_openFileDescriptor(asset, &start, &length);
	 assert(0 <= fd);
	 AAsset_close(asset); */

	return JNI_TRUE;
}

void fadeInBqPlayer() {
	pthread_t th;
	// スレッド作成と起動
	pthread_create(&th, NULL, fadeInThread, (void *)NULL);

}
/*

void* fadeInThread(void* args) {

	struct timespec ts;

	SLresult result;
	SLVolumeItf volumeItf = bqPlayerVolume;
	SLmillibel currvol = -5000;
//		SLmillibel maxvol;
//
//	 // 返り値は０である
//	 if (NULL != volumeItf) {
//	 result = (*volumeItf)->GetMaxVolumeLevel(volumeItf, maxvol);
//	 assert(SL_RESULT_SUCCESS == result);
//	 }

	int counter = 0;
	while (1) {

		if (NULL != volumeItf) {
			result = (*volumeItf)->SetVolumeLevel(volumeItf, currvol);
			assert(SL_RESULT_SUCCESS == result);
		}
		currvol += 50;

		__android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag", "counter cycle: %d", counter);
		__android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag", "currvol: %d", currvol);

		usleep(100000);

		counter++;
		if (currvol == 0)
			break;
	}
	return NULL;
}
*/



/*

// 一発のメソッド、連続的なファイルを再生するため
// a seamless loop player
int playSeamless()
{
	SLresult result;

    //result = (*bqPlayerBufferQueue)->Clear(bqPlayerBufferQueue);
    __android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag: ", "nextSize[0] %d",	nextSize[0]);

	if (nextSize[0] > 0) {
		// here we only enqueue one buffer because it is a long clip,
		// but for streaming playback we would typically enqueue at least 2 buffers to start
		// SLresult result;
		result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, bufd[0], nextSize[0]);
		if (SL_RESULT_SUCCESS != result) {
			return 0;
		}

//		result = (*bqPlayerBufferQueue)->GetState(bqPlayerBufferQueue, &bqstate);
//		__android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag: ",
//				"bqstate.count %d", bqstate.count);
//		__android_log_print(ANDROID_LOG_DEBUG, "NDK_debug_tag: ",
//				"bqstate.playIndex %d", bqstate.playIndex);
	}

    return 1;
}



*/





int playOneShot() {



	SLresult result;


	//SLAndroidSimpleBufferQueueItf bqPlyBufQ = voice->bqPlayerBufferQueue;

/*	result = (*bqPlyBufQ)->Clear(bqPlyBufQ);
	__android_log_write(ANDROID_LOG_DEBUG, "NDK_debug_tag", "	result = (*bqPlyBufQ)->Clear(bqPlyBufQ);");
	if (SL_RESULT_SUCCESS != result) {
		return 0;
	}*/

	cycleVoice();
	result = (*shot_poly[curroneshotvoice].bqPlayerBufferQueue)->Clear(shot_poly[curroneshotvoice].bqPlayerBufferQueue);




	result = (*shot_poly[curroneshotvoice].bqPlayerBufferQueue)->Enqueue(shot_poly[curroneshotvoice].bqPlayerBufferQueue, bufd[0], nextSize[0]);
	if (SL_RESULT_SUCCESS != result) {
		return 0;
	}
	__android_log_write(ANDROID_LOG_DEBUG, "NDK_debug_tag", "result = (*bqPlyBufQ)->Enqueue(bqPlyBufQ, bufd[0], nextSize[0]);");



	return 1;

}


void cycleVoice() {

	if (curroneshotvoice < VOXCNT)
		curroneshotvoice+=1;

	if (curroneshotvoice == VOXCNT)
		curroneshotvoice=0;


}



// shut down the native audio system
void shutdownAudio()
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

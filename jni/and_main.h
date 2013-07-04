/*
 * and_main.h
 *
 *  Created on: 2013/06/21
 *      Author: Michael
 */

#ifndef AND_MAIN_H_
#define AND_MAIN_H_


//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>


#include <EGL/egl.h>
#include <GLES/gl.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

// kapsy
#include <android/asset_manager.h>
#include <android/storage_manager.h>
#include <android/window.h>


#include <math.h>
#include <time.h>


#include "snd_sles.h"
#include "snd_scal.h"
#include "snd_asst.h"
#include "snd_ctrl.h"
//#include "gfx_gles.h"
#include "hon_type.h"

void play_rec_note(float x, float y);

struct saved_state {
    float angle;
    size_t x;
    size_t y;
};


//struct engine{
//    struct android_app* app;
//
////    ASensorManager* sensorManager;
////    const ASensor* accelerometerSensor;
////    ASensorEventQueue* sensorEventQueue;
//
//    int animating;
//
////    EGLDisplay display;
////    EGLSurface surface;
////    EGLContext context;
////    int32_t width;
////    int32_t height;
//    struct saved_state state;
//};

//struct engine{
//    struct android_app* app;
//
////    ASensorManager* sensorManager;
////    const ASensor* accelerometerSensor;
////    ASensorEventQueue* sensorEventQueue;
//
//    int animating;
//
////    EGLDisplay display;
////    EGLSurface surface;
////    EGLContext context;
////    int32_t width;
////    int32_t height;
//    struct saved_state state;
//};



//struct engine{
//    struct android_app* app;
//
////    ASensorManager* sensorManager;
////    const ASensor* accelerometerSensor;
////    ASensorEventQueue* sensorEventQueue;
//
//    int animating;
//
//    EGLDisplay display;
//    EGLSurface surface;
//    EGLContext context;
//	EGLint majorVersion;
//	EGLint minorVersion;
//    int32_t width;
//    int32_t height;
//    struct saved_state state;
//};

typedef struct{
    struct android_app* app;

//    ASensorManager* sensorManager;
//    const ASensor* accelerometerSensor;
//    ASensorEventQueue* sensorEventQueue;

    int animating;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
	EGLint majorVersion;
	EGLint minorVersion;
    int32_t width;
    int32_t height;
    struct saved_state state;
} engine;










#endif /* AND_MAIN_H_ */

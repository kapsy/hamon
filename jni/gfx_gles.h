/*
 * gfx_init.h
 *
 *  Created on: 2013/07/02
 *      Author: Michael
 */

#ifndef GFX_INIT_H_
#define GFX_INIT_H_


#include <EGL/egl.h>
//#include <GLES/gl.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include "hon_type.h"
#include "and_main.h"


//struct saved_state {
//    float angle;
//    int32_t x;
//    int32_t y;
//};
//
//struct engine {
//    struct android_app* app;
//
////    ASensorManager* sensorManager;
////    const ASensor* accelerometerSensor;
////    ASensorEventQueue* sensorEventQueue;
//
//    int animating;
//    EGLDisplay display;
//    EGLSurface surface;
//    EGLContext context;
//    int32_t width;
//    int32_t height;
//    struct saved_state state;
//
//
//
//    EGLint      majorVersion;
//    EGLint      minorVersion;
//
//};
//
//int gles_init(struct engine* e);
//void gles_draw(struct engine* e);
//
//void gles_term_display(struct engine* e);



//typedef struct {
//
////	    struct android_app* app;
//
//	int animating;
//	EGLNativeWindowType nativeWin;
//	EGLDisplay display;
//	EGLContext context;
//	EGLSurface surface;
//	EGLint majorVersion;
//	EGLint minorVersion;
//	int width;
//	int height;
//} ScreenSettings;


EGLBoolean pi_SurfaceCreate(engine *e);
void pi_draw(engine *e);

int init_cmds();



#endif /* GFX_INIT_H_ */

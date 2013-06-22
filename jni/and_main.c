/*
 * houtounimain.c
 *
 *  Created on: 2013/05/31
 *      Author: Michael
 */

//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>

typedef void* EGLNativeDisplayType;

#include <EGL/egl.h>
#include <GLES/gl.h>

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


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))


static int screen_width;
static int screen_height;

static int TOTAL_SEGMENTS = 24;
static float touch_segment_width;


static int find_screen_segment(float pos_x);
static float find_vel_value(float pos_y);

void play_rec_note(float x, float y);






/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);


 	__android_log_print(ANDROID_LOG_DEBUG, "engine_init_display", "h: %d", h);


    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    // Just fill the screen with a color.
    glClearColor(((float)engine->state.x)/engine->width, engine->state.angle, ((float)engine->state.y)/engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(engine->display, engine->surface);






}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
/*    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);


    	__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_input", "AINPUT_EVENT_TYPE_MOTION");

        return 1;
    }*/






//	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
//
//		int32_t key = AKeyEvent_getKeyCode(event);
//
//
//
//
//	}


    int index = 0;
    int touch_max = 5;

	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {


		int p;
		int action = AKeyEvent_getAction(event);

		switch (action & AMOTION_EVENT_ACTION_MASK) {

		case AMOTION_EVENT_ACTION_DOWN:

			__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_input",
					"AMOTION_EVENT_ACTION_DOWN");

			//play_note(find_screen_segment(AMotionEvent_getX(event, 0)), find_vel_value(AMotionEvent_getY(event, 0)));
			play_rec_note(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0));
			set_parts_active();
        engine->animating = 1;

			break;

		case AMOTION_EVENT_ACTION_POINTER_DOWN:
			__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_input",
					"AMOTION_EVENT_ACTION_POINTER_DOWN");




		    /* Bits in the action code that represent a pointer index, used with
		     * AMOTION_EVENT_ACTION_POINTER_DOWN and AMOTION_EVENT_ACTION_POINTER_UP.  Shifting
		     * down by AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT provides the actual pointer
		     * index where the data for the pointer going up or down can be found.
		     */
		  //  AMOTION_EVENT_ACTION_POINTER_INDEX_MASK  = 0xff00,

			//AMotionEvent_getPointerCount(event);




			/*

for (index = 0; index < touch_max; index++) {
	if (engine->)
}
*/

			// マルチタッチバグを解決するため、こうやれば一番いい。
			size_t pointer_index_mask = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
					>> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		//	play_note(find_screen_segment(AMotionEvent_getX(event, pointer_index_mask)));

			if (pointer_index_mask < 4) {

					//play_note(find_screen_segment(AMotionEvent_getX(event, pointer_index_mask)), find_vel_value(AMotionEvent_getY(event, pointer_index_mask)));
					play_rec_note(AMotionEvent_getX(event, pointer_index_mask), AMotionEvent_getY(event, pointer_index_mask));

					set_parts_active();
				}

						__android_log_print(ANDROID_LOG_DEBUG, "engine_handle_input",
						"pointer_index_mask: %d", pointer_index_mask);

						if (pointer_index_mask == 4) {
							int s = cycle_scale();
							//play_loop();
						}


			// 理由は一発的なタッチ
						// 駄目・バグの原因
//			size_t pointer_index = AMotionEvent_getPointerCount(event);
//			play_note(find_screen_segment(AMotionEvent_getX(event, pointer_index - 1)));
//
//			__android_log_print(ANDROID_LOG_DEBUG, "engine_handle_input",
//						"pointer_index: %d", pointer_index);





			break;

		}

		return 1;
	}


    return 0;
}


void play_rec_note(float x, float y) {
	if (decrease_ammo()) { // AMMOの量を確認するため
		int seg = find_screen_segment(x);
		float vel = find_vel_value(y);
		play_note(seg, vel);
		record_note(x, y, seg, vel);
	}
}


static void calc_segment_width() {

	touch_segment_width = (float)screen_width/(float)TOTAL_SEGMENTS;
	__android_log_print(ANDROID_LOG_DEBUG, "calc_segment_width", "touch_segment_width: %f", touch_segment_width);
}




static int find_screen_segment(float pos_x) {





	int segment = (int)floor(pos_x/touch_segment_width);
	//int seg = (int)floor(x_pos/segsize);


	__android_log_print(ANDROID_LOG_DEBUG, "find_screen_segment", "x_pos: %f", pos_x);
	__android_log_print(ANDROID_LOG_DEBUG, "find_screen_segment", "touch_segment_width: %f", touch_segment_width);
	__android_log_print(ANDROID_LOG_DEBUG, "find_screen_segment", "int seg: %d", segment);

	// 必要ないかも
/*	// x_posとscreen_widthが同じなら
	if (segment == TOTAL_SEGMENTS) {
		segment = TOTAL_SEGMENTS -  1;
	}*/

	return segment;


}

static float find_vel_value(float pos_y) {


	__android_log_print(ANDROID_LOG_DEBUG, "find_vel_value", "pos_y: %f", pos_y);

	//float vel = 1 - (pos_y/(float)screen_height);

	//SLmillibel vol = (sender_vel * (VEL_SLMILLIBEL_RANGE/sender_range)) + VEL_SLMILLIBEL_MIN;

	float vel = (pos_y * (-0.6F/screen_height)) + 1.1F; // 書き直すしなきゃ
	__android_log_print(ANDROID_LOG_DEBUG, "find_vel_value", "vel: %f", vel);

	return vel;
}





static void get_screen_dimensions(struct engine* engine) {

//	ANativeWindow_Buffer buffer;
//
//	int w, h;


	// 縦置きの向き (APad)
	//	06-04 15:47:33.845: D/get_screen_dimensions(13014): ANativeWindow_getWidth: 480
	//	06-04 15:47:33.845: D/get_screen_dimensions(13014): ANativeWindow_getHeight: 752

	// 横置きの向き (APad)
	//	06-04 15:48:38.785: D/get_screen_dimensions(13098): ANativeWindow_getWidth: 800
	//	06-04 15:48:38.785: D/get_screen_dimensions(13098): ANativeWindow_getHeight: 432



	screen_width = (int)ANativeWindow_getWidth(engine->app->window);
	screen_height = (int)ANativeWindow_getHeight(engine->app->window);


/*
	if (ANativeWindow_lock(engine->app->window, &buffer, NULL) < 0) {
	        LOGW("Unable to lock window buffer");
	        return;
	    }

	__android_log_print(ANDROID_LOG_DEBUG, "get_screen_dimensions", "buffer.height: %d", buffer.height);
	__android_log_print(ANDROID_LOG_DEBUG, "get_screen_dimensions", "buffer.width: %d", buffer.width);
*/

	__android_log_print(ANDROID_LOG_DEBUG, "get_screen_dimensions", "ANativeWindow_getWidth: %d", screen_width);
	__android_log_print(ANDROID_LOG_DEBUG, "get_screen_dimensions", "ANativeWindow_getHeight: %d", screen_height);


}


/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {


                engine_init_display(engine);
                engine_draw_frame(engine);



            	get_screen_dimensions(engine);
            	// touch
            	calc_segment_width();



           	 ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);

            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
//            if (engine->accelerometerSensor != NULL) {
//                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
//                        engine->accelerometerSensor);
//                // We'd like to get 60 events per second (in us).
//                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
//                        engine->accelerometerSensor, (1000L/60)*1000);
//            }
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
//            if (engine->accelerometerSensor != NULL) {
//                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
//                        engine->accelerometerSensor);
//            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
            state->looper, LOOPER_ID_USER, NULL, NULL);

    if (state->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

	// audio stuff
	  ANativeActivity* nativeActivity = state->activity;
//	  internal_path = nativeActivity->externalDataPath;
//
//
//		__android_log_print(ANDROID_LOG_DEBUG, "android_main", "nativeActivity->externalDataPath: %s", nativeActivity->externalDataPath);
//		__android_log_print(ANDROID_LOG_DEBUG, "android_main", "nativeActivity->internalDataPath: %s", nativeActivity->internalDataPath);


	init_random_seed();
	init_all_parts();

	AAssetManager* asset_manager = state->activity->assetManager;

	create_sl_engine();
	load_all_assets(asset_manager);
	init_all_voices();

	// snd_ctrlのこと

	init_timing_loop();
	play_loop();

//	struct timeval start_time;
//	struct timeval finish_time;
//	struct timeval diff_time;
//	struct timeval curr_time;
//	struct timezone tzp;

	// loop waiting for stuff to do.

	while (1) {


//		gettimeofday(&start_time, &tzp);



        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) {
                if (engine.accelerometerSensor != NULL) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0) {
/*                        LOGI("accelerometer: x=%f y=%f z=%f",
                                event.acceleration.x, event.acceleration.y,
                                event.acceleration.z);*/
                    }
                }
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine_draw_frame(&engine);
        }


//		gettimeofday(&finish_time, &tzp);
//		timersub(&finish_time, &start_time, &diff_time);
//
//
//		gettimeofday(&curr_time, &tzp);
//		__android_log_print(ANDROID_LOG_DEBUG, "android_main", "gettimeofday: %d %d diff_time: %d %d",
//				curr_time.tv_sec, curr_time.tv_usec, diff_time.tv_sec, diff_time.tv_usec);



    }
}
//END_INCLUDE(all)



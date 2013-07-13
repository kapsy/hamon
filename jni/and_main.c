/*
 * houtounimain.c
 *
 *  Created on: 2013/05/31
 *      Author: Michael
 */

#include <jni.h>
#include <errno.h>

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
#include "gfx_gles.h"
#include "and_main.h"
#include "hon_type.h"

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    size_t x;
    size_t y;
};

/**
 * Shared state for our app.
 */
typedef struct{

    struct android_app* app;
    int animating;
    struct saved_state state;

}engine;


typedef void* EGLNativeDisplayType;
size_t screen_width;
size_t screen_height;
size_t screen_height_reduced; // 既に計算した値・自動的再生のためにここで計算

static float touch_segment_width;
int gfx_initialized = FALSE;

static int find_screen_segment(float pos_x);
static float find_vel_value(float pos_y);



// gfx_init.cに移動した
///**
// * Tear down the EGL context currently associated with the display.
// */
//static void engine_term_display(struct engine* engine) {
//    if (engine->display != EGL_NO_DISPLAY) {
//        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//        if (engine->context != EGL_NO_CONTEXT) {
//            eglDestroyContext(engine->display, engine->context);
//        }
//        if (engine->surface != EGL_NO_SURFACE) {
//            eglDestroySurface(engine->display, engine->surface);
//        }
//        eglTerminate(engine->display);
//    }
//    engine->animating = 0;
//    engine->display = EGL_NO_DISPLAY;
//    engine->context = EGL_NO_CONTEXT;
//    engine->surface = EGL_NO_SURFACE;
//}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	engine* e = (engine*)app->userData;



	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {

		int32_t key = AKeyEvent_getKeyCode(event);
		int32_t key_action = AKeyEvent_getAction(event);

		if (key == AKEYCODE_BACK) {

			LOGDw( "engine_handle_input", "AKEYCODE_BACK");

			if (key_action == AKEY_EVENT_ACTION_UP) {
				LOGDw("engine_handle_input", "AKEY_EVENT_ACTION_UP");

				//                ANativeActivity_finish(state->activity);
			}
		}
	}


    int index = 0;
    int touch_max = 5;



	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		int p;
		int action = AKeyEvent_getAction(event);
		switch (action & AMOTION_EVENT_ACTION_MASK) {
		case AMOTION_EVENT_ACTION_DOWN:
			LOGDw("engine_handle_input", "AMOTION_EVENT_ACTION_DOWN");
//			float x = AMotionEvent_getX(event, 0);
//			float y = AMotionEvent_getY(event, 0);

			trigger_note(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0));
			set_parts_active();

//			activate_touch_shape(x, y);

			e->animating = 1;

			LOGD("LOGD_engine_handle_input", "action, %d", action);

			break;

		case AMOTION_EVENT_ACTION_POINTER_DOWN:
			LOGDw("engine_handle_input", "AMOTION_EVENT_ACTION_POINTER_DOWN");

		    /* Bits in the action code that represent a pointer index, used with
		     * AMOTION_EVENT_ACTION_POINTER_DOWN and AMOTION_EVENT_ACTION_POINTER_UP.  Shifting
		     * down by AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT provides the actual pointer
		     * index where the data for the pointer going up or down can be found.
		     */
		  //  AMOTION_EVENT_ACTION_POINTER_INDEX_MASK  = 0xff00,

			//AMotionEvent_getPointerCount(event);



			// マルチタッチバグを解決するため、こうやれば一番いい。
			size_t pointer_index_mask = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
					>> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		//	play_note(find_screen_segment(AMotionEvent_getX(event, pointer_index_mask)));

			if (pointer_index_mask < 4) {

				trigger_note(AMotionEvent_getX(event, pointer_index_mask), AMotionEvent_getY(event, pointer_index_mask));
				set_parts_active();
			}

			LOGD("engine_handle_input", "pointer_index_mask: %d", pointer_index_mask);

			if (pointer_index_mask == 4) {
				int s = cycle_scale();
				//play_loop();
			}

			break;
		}
		return 1;
	}
    return 0;
}


void trigger_note(float x, float y) {
	activate_touch_shape(x, y);

	if (decrease_ammo()) { // AMMOの量を確認するため
		int seg = find_screen_segment(x);
		float vel = find_vel_value(y);
		//play_note(seg, vel);
		enqueue_one_shot(get_scale_sample(seg), float_to_slmillibel(vel, 1.0F), get_seg_permille(seg));
		record_note(x, y, seg, vel);

	}
}


static void calc_segment_width() {
	touch_segment_width = (float)screen_width/(float)TOTAL_NOTES;
	LOGD("calc_segment_width", "touch_segment_width: %f", touch_segment_width);
}




static int find_screen_segment(float pos_x) {

	int segment = (int)floor(pos_x/touch_segment_width);
	//int seg = (int)floor(x_pos/segsize);

	LOGD("find_screen_segment", "x_pos: %f", pos_x);
	LOGD("find_screen_segment", "touch_segment_width: %f", touch_segment_width);
	LOGD("find_screen_segment", "int seg: %d", segment);

	// 必要ないかも
/*	// x_posとscreen_widthが同じなら
	if (segment == TOTAL_SEGMENTS) {
		segment = TOTAL_SEGMENTS -  1;
	}*/

	return segment;


}

static float find_vel_value(float pos_y) {


	LOGD("find_vel_value", "pos_y: %f", pos_y);

	//float vel = 1 - (pos_y/(float)screen_height);

	//SLmillibel vol = (sender_vel * (VEL_SLMILLIBEL_RANGE/sender_range)) + VEL_SLMILLIBEL_MIN;

	float vel = (pos_y * (-0.6F/screen_height)) + 1.1F; // 書き直すしなきゃ
	LOGD("find_vel_value", "vel: %f", vel);

	return vel;
}




// この関数は必要ない
static void get_screen_dimensions(engine* e) {

	// 縦置きの向き (APad)
	//	06-04 15:47:33.845: D/get_screen_dimensions(13014): ANativeWindow_getWidth: 480
	//	06-04 15:47:33.845: D/get_screen_dimensions(13014): ANativeWindow_getHeight: 752

	// 横置きの向き (APad)
	//	06-04 15:48:38.785: D/get_screen_dimensions(13098): ANativeWindow_getWidth: 800
	//	06-04 15:48:38.785: D/get_screen_dimensions(13098): ANativeWindow_getHeight: 432

	screen_width = (int)ANativeWindow_getWidth(e->app->window);
	screen_height = (int)ANativeWindow_getHeight(e->app->window);

	screen_height_reduced = screen_height * 0.8F;
/*
	if (ANativeWindow_lock(e->app->window, &buffer, NULL) < 0) {
	        LOGW("Unable to lock window buffer");
	        return;
	    }

	__android_log_print(ANDROID_LOG_DEBUG, "get_screen_dimensions", "buffer.height: %d", buffer.height);
	__android_log_print(ANDROID_LOG_DEBUG, "get_screen_dimensions", "buffer.width: %d", buffer.width);
*/

	LOGD("get_screen_dimensions", "ANativeWindow_getWidth: %d", screen_width);
	LOGD("get_screen_dimensions", "ANativeWindow_getHeight: %d", screen_height);
	LOGD("get_screen_dimensions", "screen_height_reduced: %d", screen_height_reduced);

}









/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    engine* e = (engine*)app->userData;



    switch (cmd) {
        case APP_CMD_SAVE_STATE:
        	LOGDw("engine_handle_cmd", "APP_CMD_SAVE_STATE");


            // The system has asked us to save our current state.  Do so.
        	e->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)e->app->savedState) = e->state;
            e->app->savedStateSize = sizeof(struct saved_state);
            break;

        case APP_CMD_INIT_WINDOW:
        	LOGDw("engine_handle_cmd", "APP_CMD_INIT_WINDOW");
            // The window is being shown, get it ready.

			if (e->app->window != NULL) {

				LOGD("call_order", "APP_CMD_INIT_WINDOW");

				int suc = pi_SurfaceCreate(e->app->window);
				LOGD("call_order", "pi_SurfaceCreate, suc: %d", suc);
				suc = init_cmds();
				LOGD("call_order", "init_cmds, suc: %d", suc);




				get_screen_dimensions(e);
				calc_segment_width();

				e->animating = TRUE;

				init_sles_components(app);
			}

            break;
        case APP_CMD_START:
        	LOGDw("engine_handle_cmd", "APP_CMD_START");

			ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);

        	break;
        case APP_CMD_TERM_WINDOW:
        	LOGDw("engine_handle_cmd", "APP_CMD_TERM_WINDOW");
            // The window is being hidden or closed, clean it up.

//        	gles_term_display(engine); // FIXME

            break;
        case APP_CMD_GAINED_FOCUS:
        	LOGDw("engine_handle_cmd", "APP_CMD_GAINED_FOCUS");
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
        	LOGDw("engine_handle_cmd", "APP_CMD_LOST_FOCUS");
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
//            if (engine->accelerometerSensor != NULL) {
//                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
//                        engine->accelerometerSensor);
//            }
            // Also stop animating.
        	e->animating = 0;
//            engine_draw_frame(engine);
//            quick_fade_on_exit();
//            shutdown_audio();
//            all_voices_fade_out_exit();
            pause_all_voices();


    		usleep(1000000); // 100ミリ秒
    		shutdown_audio();
    		join_control_loop();

    		e->app->destroyRequested = 1; // RvA
            //

            break;
        case APP_CMD_STOP:
        	LOGDw("engine_handle_cmd", "APP_CMD_STOP");
        	break;
        case APP_CMD_DESTROY:
        	LOGDw("engine_handle_cmd", "APP_CMD_DESTROY");
        	break;
    }
}

void init_sles_components(struct android_app* state) {

	AAssetManager* asset_manager = state->activity->assetManager;
	  ANativeActivity* nativeActivity = state->activity;

//	  internal_path = nativeActivity->externalDataPath;
//		__android_log_print(ANDROID_LOG_DEBUG, "android_main", "nativeActivity->externalDataPath: %s", nativeActivity->externalDataPath);
//		__android_log_print(ANDROID_LOG_DEBUG, "android_main", "nativeActivity->internalDataPath: %s", nativeActivity->internalDataPath);

	init_random_seed();
	init_all_parts();

	create_sl_engine();
	load_all_assets(asset_manager);
	init_all_voices();
	init_auto_vals();

	// snd_ctrlのこと
	init_control_loop();
	start_loop();
}



void android_main(struct android_app* state) {

    engine e;

    LOGD("android_main", "android_main");

    // Make sure glue isn't stripped.
    app_dummy();
    memset(&e, 0, sizeof(engine));
    state->userData = &e;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    e.app = state;
	LOGD("call_order", "android_main e.app = state");

	int i = 0;

	int animating = FALSE;






//	    struct timeval curr_time;
//	    struct timeval new_time;
//	    struct timeval delta_time;
//	    struct timezone tzp;
	while (1) {

		////            gettimeofday(&start_time, &tzp);


//		if (i < 1) { LOGD("call_order", "while started"); i++; }

		int ident, events;
		struct android_poll_source* source;
		while ((ident = ALooper_pollAll(0, NULL, &events, (void**) &source)) >= 0) {
			LOGD("call_order", "while ((ident=ALooper_pollAll...");
			if (source != NULL) {
				source->process(state, source);
			}

			if (state->destroyRequested != 0) {
				return;
			}
		}




		if (e.animating) {

			calc_frame_delta_time();
//			calc_frame_rate();

			pi_draw();

		}





	}
}

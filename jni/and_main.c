/*
 * houtounimain.c
 *
 *  Created on: 2013/05/31
 *      Author: Michael
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <jni.h>
#include <errno.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

// kapsy
#include <android/asset_manager.h>
#include <android/storage_manager.h>
#include <android/window.h>


#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <EGL/egl.h>
#include <GLES/gl.h>

#include <stddef.h>
#include "and_main.h"
#include "hon_type.h"
#include "snd_sles.h"
#include "snd_asst.h"

#include "gfx/vertex.h"
#include "snd_sles.h"
#include "game/moods.h"
#include "snd_scal.h"
#include "snd_ctrl.h"
#include "gfx_gles.h"
#include "gfx/full_screen_element.h"
#include "gfx_butn.h"

#include "gfx/frame_delta.h"

#include <pthread.h>
#include "gfx/touch_circle.h"
#include "gfx/tex_circle.h"


#include "math/trig_sampler.h"





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


extern screen_settings g_sc;

typedef void* EGLNativeDisplayType;
size_t screen_width;
size_t screen_height;
//size_t screen_height_reduced; // 既に計算した値・自動的再生のためにここで計算
//size_t screen_margin; // 円形を端に切れないように描くための値、画面縦の２０％
//size_t screen_margin_y;
//size_t screen_margin_x;
size_t screen_margin_x_l;
size_t screen_margin_x_r;
size_t screen_margin_y_t;
size_t screen_margin_y_b;


static float touch_segment_width;
//int gfx_initialized = FALSE;

// TIMING SWITCHES
int sles_init_called = FALSE;
int sles_init_finished = FALSE;
int show_gameplay = FALSE;
int touch_enabled = FALSE; // タッチ操作のため
int buttons_activated = FALSE;
int splash_fading_in = FALSE;
int splash_bg_fading_in = FALSE;
int splash_fading_out = FALSE; //used for splash timing
int splash_bg_fading_out = FALSE;
//int gameplay_started = FALSE;

int show_help = FALSE;

int wake_from_paused = FALSE;

unsigned long splash_fadeout_time = 0;
unsigned long buttons_activated_time = 0;
unsigned long touch_enable_time = 0;






//
//unsigned long elapsed_time = 0;

// プロトタイプ
void first_init(engine* e);
static int find_screen_segment(float pos_x);
static float find_vel_value(float pos_y);
void touch_branching(float x, float y);
void create_init_sles_thread(struct android_app* state);
void* init_sles_thread(void* args);

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

			LOGD( "engine_handle_input", "AKEYCODE_BACK");

			if (key_action == AKEY_EVENT_ACTION_UP) {
				LOGD("engine_handle_input", "AKEY_EVENT_ACTION_UP");

				//                ANativeActivity_finish(state->activity);
			}

	        return 1; // <-- prevent default handler
		}

		if (key == AKEYCODE_MENU) {

			LOGD( "engine_handle_input", "AKEYCODE_MENU");

			if (key_action == AKEY_EVENT_ACTION_UP) {
				LOGD("engine_handle_input", "AKEY_EVENT_ACTION_UP");

				//                ANativeActivity_finish(state->activity);
			}

	        return 1; // <-- prevent default handler
		}


/*
		if (key == AKEYCODE_HOME) {
			LOGD( "engine_handle_input", "AKEYCODE_HOME");
			if (key_action == AKEY_EVENT_ACTION_UP) {
				LOGD("engine_handle_input", "AKEY_EVENT_ACTION_UP");
			}
	        return 1; // <-- prevent default handler
		}

		if (key == AKEYCODE_VOLUME_UP) {
			LOGD( "engine_handle_input", "AKEYCODE_VOLUME_UP");
			if (key_action == AKEY_EVENT_ACTION_UP) {
				LOGD("engine_handle_input", "AKEY_EVENT_ACTION_UP");
			}
	        return 1; // <-- prevent default handler
		}

		if (key == AKEYCODE_VOLUME_DOWN) {
			LOGD( "engine_handle_input", "AKEYCODE_VOLUME_DOWN");
			if (key_action == AKEY_EVENT_ACTION_UP) {
				LOGD("engine_handle_input", "AKEY_EVENT_ACTION_UP");
			}
	        return 1; // <-- prevent default handler
		}*/





	}


    int index = 0;
    int touch_max = 5;



	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {

//		if (show_gameplay && !screens[0].fading_out) {

		if (touch_enabled) {


			int p;
			int action = AKeyEvent_getAction(event);
			switch (action & AMOTION_EVENT_ACTION_MASK) {
			case AMOTION_EVENT_ACTION_DOWN:
				LOGDw("engine_handle_input", "AMOTION_EVENT_ACTION_DOWN");



//		if (show_splash) {
//			splash_fading_out = TRUE;
//		}
//		else {
						touch_branching(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0));

//		}


				e->animating = 1;


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

//				if (pointer_index_mask < 4) {

					trigger_note(AMotionEvent_getX(event, pointer_index_mask), AMotionEvent_getY(event, pointer_index_mask));
					set_parts_active();
//				}

				LOGD("engine_handle_input", "pointer_index_mask: %d", pointer_index_mask);

//				if (pointer_index_mask == 4) {
//					int s = cycle_mood();
//				}

				break;
			}
			return 1;
		}
	}
    return 0;
}



void touch_branching(float x, float y) {

	int s;
	switch (get_touch_response(x, y)) {

	case TOUCH_EVENT_BUTTON_0:
		LOGD("touch_branching", "TOUCH_EVENT_BUTTON_0");




		int s = cycle_mood();
		chord_count = 0;
		assign_time(&buttons_activated_time);
		break;
	case TOUCH_EVENT_BUTTON_1:
		LOGD("touch_branching", "TOUCH_EVENT_BUTTON_1");




		init_all_parts();
		ammo_current = AMMO_MAX;
		assign_time(&buttons_activated_time);
		break;
	case TOUCH_EVENT_BUTTON_2:
		LOGD("touch_branching", "TOUCH_EVENT_BUTTON_2");



		buttons[0].fade_out_end = &all_btns_fade_end_deactivate_show_help;

		break;

	case TOUCH_EVENT_INTERACTIVE_ON:
		LOGD("touch_branching", "TOUCH_EVENT_INTERACTIVE_ON");

		buttons[0].fading_in = TRUE;
		buttons_activated = TRUE;

		assign_time(&buttons_activated_time);

		break;

	case TOUCH_EVENT_HELP:

//		if (!screens[2].fading_out) screens[2].fading_out = TRUE;


		if (!screens[2].fading_in && !screens[2].fading_out) {
			screens[2].fading_out = TRUE;
			LOGD("touch_branching", "screens[2].fading_out = TRUE");

		}

		break;

	case TOUCH_EVENT_GAME:
		LOGD("touch_branching", "TOUCH_EVENT_GAME");

		trigger_note(x, y);
		set_parts_active();

		break;

	case TOUCH_EVENT_NULL:
		LOGD("touch_branching", "TOUCH_EVENT_NULL");

		//do nothing

		break;

	}




//void interactive_time() {
//
//
//	if (interactive_mode && interactive_on_time < INTERACTIVE_TTL) {
//	interactive_on_time++;
//	} else if (interactive_mode) {
//		interactive_mode = FALSE;
//		interactive_on_time = 0;
//	}
//
//
//
//
//}








}

void trigger_note(float x, float y) {

	LOGD("trigger_note", "x: %f, y: %f", x, y);

	int seg = find_screen_segment(x);
	float vel = find_vel_value(y);

	if (decrease_ammo()) { // AMMOの量を確認するため
		activate_tex_circle(x, y, (parts + current_rec_part)->rgb, &vel);
		enqueue_one_shot(get_scale_sample(seg), float_to_slmillibel(vel, 1.0F), get_seg_permille(seg));
		record_note(x, y, seg, vel);

	} else {
//		activate_touch_no_ammo(x, y);
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


	screen_margin_x_l = screen_width*0.13f;
	screen_margin_x_r = screen_width*0.110f;
	screen_margin_y_t = screen_height*0.186f;
	screen_margin_y_b = screen_height*0.26f;

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

	LOGD("get_screen_dimensions", "screen_margin_x_l: %d", screen_margin_x_l);
	LOGD("get_screen_dimensions", "screen_margin_x_r: %d", screen_margin_x_r);
	LOGD("get_screen_dimensions", "screen_margin_y_t: %d", screen_margin_y_t);
	LOGD("get_screen_dimensions", "screen_margin_y_b: %d", screen_margin_y_b);

//	LOGD("get_screen_dimensions", "screen_height_reduced: %d", screen_height_reduced);

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



        	// ココらへんは大丈夫
        	// けど、掃除しないと…
        	if (e->app->window != NULL) {

				LOGD("call_order", "APP_CMD_INIT_WINDOW");



//				init_all_trig_samples();
//				int suc = create_window_surface(e->app->window);
//				LOGD("call_order", "create_window_surface, suc: %d", suc);
//				suc = gles_init();
//				LOGD("call_order", "init_cmds, suc: %d", suc);
//				get_start_time();
//				get_screen_dimensions(e);
//				calc_segment_width();
//				e->animating = TRUE;



				first_init(e);



			}






            break;
        case APP_CMD_START:
        	LOGDw("engine_handle_cmd", "APP_CMD_START");

			ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);

        	break;
        case APP_CMD_TERM_WINDOW:
        	LOGDw("engine_handle_cmd", "APP_CMD_TERM_WINDOW");
            // The window is being hidden or closed, clean it up.


			gles_term_display(&g_sc);


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




        	kill_all_touch_circles();
        	draw_frame();
        	e->animating = 1;



            break;
        case APP_CMD_LOST_FOCUS:
        	LOGDw("engine_handle_cmd", "APP_CMD_LOST_FOCUS");
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.






        	kill_all_touch_circles();
        	draw_frame();
        	e->animating = 0;
        	if (sles_init_called)wake_from_paused = TRUE;
//        	show_gameplay = FALSE;

        	// タッチの円形全部無効スべき

            pause_all_voices();
//    		usleep(1000000); // 100ミリ秒
    		usleep(5000000); // 100ミリ秒
    		shutdown_audio();



//    		join_control_loop();

//    		e->app->destroyRequested = 1; // RvA

            break;
        case APP_CMD_STOP:
        	LOGDw("engine_handle_cmd", "APP_CMD_STOP");
        	break;
        case APP_CMD_DESTROY:
        	LOGDw("engine_handle_cmd", "APP_CMD_DESTROY");
        	break;
    }
}





// 最初の初期化するための関数
void first_init(engine* e) {

	init_all_trig_samples();
	int suc = create_window_surface(e->app->window);
	LOGD("call_order", "create_window_surface, suc: %d", suc);
	suc = gles_init();
	LOGD("call_order", "init_cmds, suc: %d", suc);
	get_start_time();
	get_screen_dimensions(e);
	calc_segment_width();
	e->animating = TRUE;

}




pthread_t init_sles;
pthread_attr_t init_sles_t_attr;

void create_init_sles_thread(struct android_app* state) {
	LOGD("init_sles_thread", "create_init_sles_thread");
	pthread_create(&init_sles, NULL, init_sles_thread, (struct android_app*)state);
}

void join_init_sles_thread() {
	LOGD("init_sles_thread", "join_init_sles_thread");
	pthread_join(init_sles, NULL);
	pthread_exit(NULL);
}

void* init_sles_thread(void* args) {

	LOGD("init_sles_thread", "init_sles_thread(void* args)");
	init_sles_components(args);
	return NULL;
}




void init_sles_components(struct android_app* state) {

	AAssetManager* asset_manager = state->activity->assetManager;
//	  ANativeActivity* nativeActivity = state->activity;

//	  internal_path = nativeActivity->externalDataPath;
//		LOGD("android_main", "nativeActivity->externalDataPath: %s", nativeActivity->externalDataPath);
//		LOGD("android_main", "nativeActivity->internalDataPath: %s", nativeActivity->internalDataPath);

	load_all_assets(asset_manager);
	create_sl_engine();
	init_all_voices();
	init_random_seed();
	init_all_parts();
	init_auto_vals();



	// snd_ctrlのこと
//	start_loop();
//	init_control_loop();

	sles_init_finished = TRUE;
	LOGD("init_sles_thread", "sles_init_finished = TRUE");
}






void init_sles_gain_focus(struct android_app* state) {

	create_sl_engine();
	init_all_voices();
	start_loop();


}












//void update_elapsed_time() {
//
//	get_time_long(&curr_time);
//	elapsed_time = curr_time - start_time;
//}


void android_main(struct android_app* state) {

    engine e;

//    LOGD("android_main", "android_main");

    // Make sure glue isn't stripped.
    app_dummy();
    memset(&e, 0, sizeof(engine));
    state->userData = &e;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    e.app = state;
//	LOGD("call_order", "android_main e.app = state");

	int i = 0;

	int animating = FALSE;



	while (1) {


//		if (i < 1) { LOGD("call_order", "while started"); i++; }



		int ident, events;
		struct android_poll_source* source;
		while ((ident = ALooper_pollAll(0, NULL, &events, (void**) &source)) >= 0) {
			LOGD("call_order", "while ((ident=ALooper_pollAll...");
			LOGD("ALooper_pollAll", "ALooper_pollAll");
			if (source != NULL) {
				source->process(state, source);
			}


//			 * 4/ Whenever you receive a LOOPER_ID_MAIN or LOOPER_ID_INPUT event,
//			 *    the returned data will point to an android_poll_source structure.  You
//			 *    can call the process() function on it, and fill in android_app->onAppCmd
//			 *    and android_app->onInputEvent to be called for your own processing
//			 *    of the event.
//			 *
//			 *    Alternatively, you can call the low-level functions to read and process
//			 *    the data directly...  look at the process_cmd() and process_input()
//			 *    implementations in the glue to see how to do this.


//			if (source->id == LOOPER_ID_INPUT) {
//
//				process_cmd();
//			}




//			Solved: to prevent default "Back" button behaivor it is enough to return 1 while handling key event:

//			int32_t app_handle_event(struct android_app* app, AInputEvent* event) {
//				if (AKeyEvent_getKeyCode(event) == AKEYCODE_BACK) {
//					// actions on back key
//					return 1; // <-- prevent default handler
//				};
//				// ...
//				return 0;
//			}
//



//
//
//			if (ident == LOOPER_ID_USER) {
//				if (e.)
//
//
//
//			}
//



//			/**
//			 * Waits for events to be available, with optional timeout in milliseconds.
//			 * Invokes callbacks for all file descriptors on which an event occurred.
//			 *
//			 * If the timeout is zero, returns immediately without blocking.
//			 * If the timeout is negative, waits indefinitely until an event appears.
//			 *
//			 * Returns ALOOPER_POLL_WAKE if the poll was awoken using wake() before
//			 * the timeout expired and no callbacks were invoked and no other file
//			 * descriptors were ready.
//			 *
//			 * Returns ALOOPER_POLL_CALLBACK if one or more callbacks were invoked.
//			 *
//			 * Returns ALOOPER_POLL_TIMEOUT if there was no data before the given
//			 * timeout expired.
//			 *
//			 * Returns ALOOPER_POLL_ERROR if an error occurred.
//			 *
//			 * Returns a value >= 0 containing an identifier if its file descriptor has data
//			 * and it has no callback function (requiring the caller here to handle it).
//			 * In this (and only this) case outFd, outEvents and outData will contain the poll
//			 * events and data associated with the fd, otherwise they will be set to NULL.
//			 *
//			 * This method does not return until it has finished invoking the appropriate callbacks
//			 * for all file descriptors that were signalled.
//			 */
//			int ALooper_pollOnce(int timeoutMillis, int* outFd, int* outEvents, void** outData);
//
//
//






			if (state->destroyRequested != 0) {
				return;
			}
		}




		if (e.animating) {
			if(wake_from_paused)	{
				init_sles_gain_focus(state);
				wake_from_paused = FALSE;
			}


			calc_frame_delta_time();
			update_elapsed_time();
//			calc_frame_rate();
//		    LOGD("android_main", "frame_delta %d", frame_delta);
			draw_frame();




			if(!sles_init_called && elapsed_time > (1*SEC_IN_US)) {

				create_init_sles_thread(state);
				sles_init_called = TRUE;
			    LOGD("android_main", "sles_init_called = TRUE");
			}

			if(!splash_fading_in && elapsed_time > (1*SEC_IN_US	)) {
				splash_fading_in = TRUE;
				screens[0].is_showing = TRUE;
				LOGD("android_main", "splash_fading_in = TRUE");
			}
			if(!splash_bg_fading_in && elapsed_time > (3*SEC_IN_US)) {
				splash_bg_fading_in = TRUE;
				screens[1].is_showing = TRUE;
				LOGD("android_main", "splash_bg_fading_in = TRUE");
			}

			if(sles_init_finished) {
				sles_init_finished = FALSE;
				assign_time(&splash_fadeout_time);
				LOGD("android_main", "sles_init_finished");
			}

			if(!splash_bg_fading_out && compare_times(splash_fadeout_time, (1*SEC_IN_US))) {
				splash_bg_fading_out = TRUE;
				screens[1].fading_in = FALSE;
//				screens[1].fade_rate = SPLASH_FADE_RATE_QUICK;
				screens[1].fading_out = TRUE;
				LOGD("android_main", "splash_bg_fading_out = TRUE");
			}

			if(!splash_fading_out && compare_times(splash_fadeout_time, (4*SEC_IN_US))) {
				splash_fading_out = TRUE;
				screens[0].fading_in = FALSE;
				screens[0].fading_out = TRUE;
				LOGD("android_main", "splash_fading_out = TRUE");
			}

			if(!show_gameplay && compare_times(splash_fadeout_time, (6*SEC_IN_US))) {
				show_gameplay = TRUE;
				assign_time(&touch_enable_time);
				start_loop();
				init_control_loop();
				LOGD("android_main", "show_gameplay = TRUE");
			}

			if(!touch_enabled && compare_times(touch_enable_time, (5*SEC_IN_US))) {
				touch_enabled = TRUE;
				LOGD("android_main", "touch_enabled = TRUE");
			}

			if (show_gameplay && buttons_activated) {
				if (compare_times(buttons_activated_time, INTERACTIVE_TTL)) {
					if (!all_buttons_busy_fading) {
						buttons[2].fading_out = TRUE;
					}
				}
			}


		}
	}
}

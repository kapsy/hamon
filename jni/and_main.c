/*
 * houtounimain.c
 *
 *  Created on: 2013/05/31
 *      Author: Michael
 */

//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>


//#include <EGL/egl.h>
//#include <GLES/gl.h>
//
//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>


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







struct saved_state {
    float angle;
    size_t x;
    size_t y;
};


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
void play_rec_note(float x, float y);

/**
 * Our saved state data.
 */
//struct saved_state {
//    float angle;
//    int32_t x;
//    int32_t y;
//};

/**
 * Shared state for our app.
 */
//struct engine {
//    struct android_app* app;
//
//    ASensorManager* sensorManager;
//    const ASensor* accelerometerSensor;
//    ASensorEventQueue* sensorEventQueue;
//
//    int animating;
//    EGLDisplay display;
//    EGLSurface surface;
//    EGLContext context;
//    int32_t width;
//    int32_t height;
//    struct saved_state state;
//};





///**
// * Just the current frame in the display.
// */
//static void engine_draw_frame(struct engine* e) {
////    if (engine->display == NULL) {
////        // No display.
////        return;
////    }
////
////    // Just fill the screen with a color.
////    glClearColor(((float)engine->state.x)/engine->width, engine->state.angle, ((float)engine->state.y)/engine->height, 1);
////    glClear(GL_COLOR_BUFFER_BIT);
////    eglSwapBuffers(engine->display, engine->surface);
//
//
//	gles_draw(e);
//
//
//
//}


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

			__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_input", "AKEYCODE_BACK");

			if (key_action == AKEY_EVENT_ACTION_UP) {
				__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_input", "AKEY_EVENT_ACTION_UP");

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
			__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_input",
					"AMOTION_EVENT_ACTION_DOWN");
			//play_note(find_screen_segment(AMotionEvent_getX(event, 0)), find_vel_value(AMotionEvent_getY(event, 0)));
			play_rec_note(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0));
			set_parts_active();
			e->animating = 1;

        LOGD("LOGD_engine_handle_input", "action, %d", action);

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
		//play_note(seg, vel);
		enqueue_one_shot(get_scale_sample(seg), float_to_slmillibel(vel, 1.0F), get_seg_permille(seg));

		record_note(x, y, seg, vel);
	}
}


static void calc_segment_width() {

	touch_segment_width = (float)screen_width/(float)TOTAL_NOTES;
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




// この関数は必要ない
static void get_screen_dimensions(engine* engine) {

	// 縦置きの向き (APad)
	//	06-04 15:47:33.845: D/get_screen_dimensions(13014): ANativeWindow_getWidth: 480
	//	06-04 15:47:33.845: D/get_screen_dimensions(13014): ANativeWindow_getHeight: 752

	// 横置きの向き (APad)
	//	06-04 15:48:38.785: D/get_screen_dimensions(13098): ANativeWindow_getWidth: 800
	//	06-04 15:48:38.785: D/get_screen_dimensions(13098): ANativeWindow_getHeight: 432



	screen_width = (int)ANativeWindow_getWidth(engine->app->window);
	screen_height = (int)ANativeWindow_getHeight(engine->app->window);

	screen_height_reduced = screen_height * 0.8F;
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
	__android_log_print(ANDROID_LOG_DEBUG, "get_screen_dimensions", "screen_height_reduced: %d", screen_height_reduced);

}


//static void engine_init(struct android_app* app) {
//	//	struct engine* engine = (struct engine*)app->userData;
//
//
//	//(engine*)app->userData = &main_engine;
//	app->userData = &main_engine;
//main_engine.app = app;
//
//main_engine.animating = 0;
//	main_engine.display = NULL;
//	main_engine.surface = NULL;
//	main_engine.context = NULL;
//	main_engine.majorVersion = 0;
//
//	main_engine.minorVersion = 0;
//
//	main_engine.width = 0;
//	main_engine.height = 0;
//
//
//
//}


/*void android_main(android_app* state) {
	app_dummy();

	engine e;
	state->userData = &e;
	state->onAppCmd = [](android_app* app, int32_t cmd) {
		auto e = static_cast<engine*>(app->userData);
		switch (cmd) {
			case APP_CMD_INIT_WINDOW:
				init(e);
				draw(e);
				break;
		}
	};
	e.app = state;

	while (1) {
		int ident, events;
		android_poll_source* source;
		while ((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
			if (source != NULL) {
				source->process(state, source);
			}
			if (state->destroyRequested != 0) {
				return;
			}
		}
	}
}*/






/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    engine* e = (engine*)app->userData;



    switch (cmd) {
        case APP_CMD_SAVE_STATE:
        	__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_cmd", "APP_CMD_SAVE_STATE");


            // The system has asked us to save our current state.  Do so.
        	e->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)e->app->savedState) = e->state;
            e->app->savedStateSize = sizeof(struct saved_state);
            break;

        case APP_CMD_INIT_WINDOW:
        	__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_cmd", "APP_CMD_INIT_WINDOW");
            // The window is being shown, get it ready.



			if (e->app->window != NULL) {

				LOGD("call_order", "APP_CMD_INIT_WINDOW");

				int suc = pi_SurfaceCreate(e->app->window);
				LOGD("call_order", "pi_SurfaceCreate, suc: %d", suc);
				suc = init_cmds(&e->animating);
				LOGD("call_order", "init_cmds, suc: %d", suc);

				get_screen_dimensions(e);
				calc_segment_width();

					e->animating = TRUE;

					init_sles_components(app);


//				ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);



//				if (gfx_initialized) {
//					e->animating = TRUE;
//						init_sles_components(app);
//				}
//
//
//				gfx_initialized = TRUE;
			}


            break;
        case APP_CMD_START:
        	__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_cmd", "APP_CMD_START");



			ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);





        	break;
        case APP_CMD_TERM_WINDOW:
        	__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_cmd", "APP_CMD_TERM_WINDOW");
            // The window is being hidden or closed, clean it up.

//        	gles_term_display(engine); // FIXME

            break;
        case APP_CMD_GAINED_FOCUS:
        	__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_cmd", "APP_CMD_GAINED_FOCUS");
            // When our app gains focus, we start monitoring the accelerometer.
//            if (engine->accelerometerSensor != NULL) {
//                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
//                        engine->accelerometerSensor);
//                // We'd like to get 60 events per second (in us).
//                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
//                        engine->accelerometerSensor, (1000L/60)*1000);
//            }


//        	e->animating = TRUE;
            break;
        case APP_CMD_LOST_FOCUS:
        	__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_cmd", "APP_CMD_LOST_FOCUS");
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

//        case AINPUT_EVENT_TYPE_KEY:
//        	__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_cmd", "AINPUT_EVENT_TYPE_KEY");
//        	break;
        case APP_CMD_STOP:
        	__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_cmd", "APP_CMD_STOP");
        	break;
        case APP_CMD_DESTROY:
        	__android_log_write(ANDROID_LOG_DEBUG, "engine_handle_cmd", "APP_CMD_DESTROY");
        	break;
    }
}

void init_sles_components(struct android_app* state) {
    // ＝＝＝＝＝＝＝＝＝オーディオ処理＝＝＝＝＝＝＝＝＝

	AAssetManager* asset_manager = state->activity->assetManager;
	  ANativeActivity* nativeActivity = state->activity;
//	  internal_path = nativeActivity->externalDataPath;
//
//
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
	LOGD("call_order", "init_sles_components() finished");
}

///**
// * This is the main entry point of a native application that is using
// * android_native_app_glue.  It runs in its own thread, with its own
// * event loop for receiving input events and doing other things.
// */
//void android_main(struct android_app* state) {
//    engine e;
//
//    LOGD("android_main", "android_main");
//
//    // Make sure glue isn't stripped.
//    app_dummy();
//    memset(&e, 0, sizeof(engine));
//    state->userData = &e;
//    state->onAppCmd = engine_handle_cmd;
//    state->onInputEvent = engine_handle_input;
//    e.app = state;
//	LOGD("call_order", "android_main e.app = state");
//
//
//    if (state->savedState != NULL) {
//        // We are starting with a previous saved state; restore from it.
//    	e.state = *(struct saved_state*)state->savedState;
//    }
//    LOGD("android_main", "(state->savedState != NULL)");
//
//
//
////    // ＝＝＝＝＝＝＝＝＝オーディオ処理＝＝＝＝＝＝＝＝＝
////
////	  ANativeActivity* nativeActivity = state->activity;
//////	  internal_path = nativeActivity->externalDataPath;
//////
//////
//////		__android_log_print(ANDROID_LOG_DEBUG, "android_main", "nativeActivity->externalDataPath: %s", nativeActivity->externalDataPath);
//////		__android_log_print(ANDROID_LOG_DEBUG, "android_main", "nativeActivity->internalDataPath: %s", nativeActivity->internalDataPath);
////
////	init_random_seed();
////	init_all_parts();
////
////	AAssetManager* asset_manager = state->activity->assetManager;
////
////	create_sl_engine();
////	load_all_assets(asset_manager);
////	init_all_voices();
////	init_auto_vals();
////
////	// snd_ctrlのこと
////
////	init_control_loop();
////	start_loop();
////	LOGD("call_order", "start_loop() called");
//
////	struct timeval start_time;
////	struct timeval finish_time;
////	struct timeval diff_time;
////	struct timeval curr_time;
////	struct timezone tzp;
//
//
//int i = 0;
//
//	// loop waiting for stuff to do.
//
//	while (1) {
//		if (i<1){ LOGD("call_order", "while started"); i++;}
//
//
////		gettimeofday(&start_time, &tzp);
//        // Read all pending events.
//        int ident;
//        int events;
//        struct android_poll_source* source;
//
//        // If not animating, we will block forever waiting for events.
//        // If animating, we loop until all events are read, then continue
//        // to draw the next frame of animation.
////        while ((ident=ALooper_pollAll(e.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0) {
//            while ((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
//        	LOGD("call_order", "while ((ident=ALooper_pollAll...");
//            // Process this event.
//            if (source != NULL) {
//                source->process(state, source);
//            }
//
////            // If a sensor has data, process it now.
////            if (ident == LOOPER_ID_USER) {
////                if (engine.accelerometerSensor != NULL) {
////                    ASensorEvent event;
////                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0) {
/////*                        LOGI("accelerometer: x=%f y=%f z=%f",
////                                event.acceleration.x, event.acceleration.y,
////                                event.acceleration.z);*/
////                    }
////                }
////            }
//
//            // Check if we are exiting.
//            if (state->destroyRequested != 0) {
//            	__android_log_write(ANDROID_LOG_DEBUG, "android_main", "(state->destroyRequested != 0)");
////                gles_term_display(&engine);
//                return;
//            }
//        }
//
//
//
//
//        if (!gfx_initialized) {
//
//        	int suc = pi_SurfaceCreate(e.app->window);
//        	suc = init_cmds();
//
//        	LOGD("call_order", "(!gfx_initialized)");
//        	get_screen_dimensions(&e);
//        	calc_segment_width();
//        	init_sles_components(state);
//
//        	gfx_initialized = TRUE;
//        }
//
//
//
//
//
//        if (e.animating) {
////            // Done with events; draw next animation frame.
////            engine.state.angle += .01f;
////            if (engine.state.angle > 1) {
////                engine.state.angle = 0;
////            }
////
////            // Drawing is throttled to the screen update rate, so there
////            // is no need to do timing here.
//
//
//        		pi_draw();
//        }
//
//		LOGD("call_order", "while running");
//
//
//
//
//
////		gettimeofday(&finish_time, &tzp);
////		timersub(&finish_time, &start_time, &diff_time);
////
////
////		gettimeofday(&curr_time, &tzp);
////		__android_log_print(ANDROID_LOG_DEBUG, "android_main", "gettimeofday: %d %d diff_time: %d %d",
////				curr_time.tv_sec, curr_time.tv_usec, diff_time.tv_sec, diff_time.tv_usec);
//
//
//
//	}
//}
//END_INCLUDE(all)


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






//	app_dummy();
//
//	engine e;
//	state->userData = &e;
//	state->onAppCmd = [](android_app* app, int32_t cmd) {
//		auto e = static_cast<engine*>(app->userData);
//		switch (cmd) {
//			case APP_CMD_INIT_WINDOW:
//				init(e);
//				draw(e);
//				break;
//		}
//	};
//	e.app = state;
	int i = 0;

	int animating = FALSE;

	while (1) {

		if (i < 1) { LOGD("call_order", "while started"); i++; }

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



//		if (e.app->window != NULL && !gfx_initialized) {
//
//			LOGD("call_order", "(e->app->window != NULL && !gfx_initialized))");
//
//			int suc = pi_SurfaceCreate(e.app->window);
//			suc = init_cmds();
//
//			LOGD("call_order", "(!gfx_initialized)");
//			get_screen_dimensions(&e);
//			calc_segment_width();
//
//			//animating = TRUE;
//
//			init_sles_components(state);
//
//			gfx_initialized = TRUE;
//		}

//		if (e.app->window != NULL && animating) {
//			pi_draw();
//
//		}

		if (e.animating) {
			pi_draw();
		}





//    	LOGD("call_order", "while (1)");

	}
}






















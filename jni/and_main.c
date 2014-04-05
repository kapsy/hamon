// and_main.c

#include "common.h"
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
#include "gfx_asst.h"

// saved state
struct saved_state {
    float angle;
    size_t x;
    size_t y;
};

// shared state
typedef struct{
    struct android_app* app;
    int animating;
    struct saved_state state;
}engine;

AAssetManager* asset_manager;
extern screen_settings g_sc;
typedef void* EGLNativeDisplayType;
size_t screen_width;
size_t screen_height;
size_t screen_margin_x_l;
size_t screen_margin_x_r;
size_t screen_margin_y_t;
size_t screen_margin_y_b;
static float touch_segment_width;

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
int show_help = FALSE;
int wake_from_paused = FALSE;

unsigned long splash_fadeout_time = 0;
unsigned long buttons_activated_time = 0;
unsigned long touch_enable_time = 0;

// プロトタイプ
void first_init(engine* e);
static int find_screen_segment(float pos_x);
static float find_vel_value(float pos_y);
void touch_branching(float x, float y);
void create_init_sles_thread(struct android_app* state);
void* init_sles_thread(void* args);
void init_sles_components();

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	engine* e = (engine*)app->userData;

	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {

		int32_t key = AKeyEvent_getKeyCode(event);
		int32_t key_action = AKeyEvent_getAction(event);

		if (key == AKEYCODE_BACK) {

			LOGD( "engine_handle_input", "AKEYCODE_BACK");

			if (key_action == AKEY_EVENT_ACTION_UP) {
				LOGD("engine_handle_input", "AKEY_EVENT_ACTION_UP");
			}

	        return 1; // <-- prevent default handler
		}

		if (key == AKEYCODE_MENU) {

			LOGD( "engine_handle_input", "AKEYCODE_MENU");

			if (key_action == AKEY_EVENT_ACTION_UP) {
				LOGD("engine_handle_input", "AKEY_EVENT_ACTION_UP");
			}

	        return 1; // <-- prevent default handler
		}

/*		if (key == AKEYCODE_HOME) {
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

		if (touch_enabled) {

			int p;
			int action = AKeyEvent_getAction(event);
			switch (action & AMOTION_EVENT_ACTION_MASK) {
			case AMOTION_EVENT_ACTION_DOWN:
				LOGDw("engine_handle_input", "AMOTION_EVENT_ACTION_DOWN");
				touch_branching(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0));
				e->animating = 1;
				break;

			case AMOTION_EVENT_ACTION_POINTER_DOWN:
				LOGDw(
						"engine_handle_input", "AMOTION_EVENT_ACTION_POINTER_DOWN");

				/* Bits in the action code that represent a pointer index, used with
				 * AMOTION_EVENT_ACTION_POINTER_DOWN and AMOTION_EVENT_ACTION_POINTER_UP.  Shifting
				 * down by AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT provides the actual pointer
				 * index where the data for the pointer going up or down can be found.
				 */
				//  AMOTION_EVENT_ACTION_POINTER_INDEX_MASK  = 0xff00,
				// マルチタッチバグを解決するため、こうやれば一番いい。
				size_t pointer_index_mask = (action
						& AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
						>> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

				trigger_note(AMotionEvent_getX(event, pointer_index_mask),
						AMotionEvent_getY(event, pointer_index_mask));
				set_parts_active();
				LOGD("engine_handle_input", "pointer_index_mask: %d", pointer_index_mask);
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
}

void trigger_note(float x, float y) {

	LOGD("trigger_note", "x: %f, y: %f", x, y);

	int seg = find_screen_segment(x);
	float vel = find_vel_value(y);

	if (decrease_ammo()) { // AMMOの量を確認するため
		LOGD("trigger_note", "decrease_ammo");
		activate_tex_circle(x, y, (parts + current_rec_part)->rgb, &vel);
		enqueue_one_shot(get_scale_sample(seg), float_to_slmillibel(vel, 1.0F), get_seg_permille(seg));
		record_note(x, y, seg, vel);

	} else {
		LOGD("trigger_note", "!decrease_ammo");
		//activate_tex_circle(x, y, (parts + current_rec_part)->rgb, &vel);
		activate_tex_no_ammo(x, y, &vel);
		// need this to be a non "bubble" like sound
		// ここで泡みたいな音のほうが
		enqueue_one_shot(get_scale_sample(seg), float_to_slmillibel(vel, 1.0F), get_seg_permille(seg));
	}
}

static void calc_segment_width() {
	touch_segment_width = (float)screen_width/(float)TOTAL_NOTES;
	LOGD("calc_segment_width", "touch_segment_width: %f", touch_segment_width);
}

static int find_screen_segment(float pos_x) {
	int segment = (int)floor(pos_x/touch_segment_width);
	LOGD("find_screen_segment", "x_pos: %f", pos_x);
	LOGD("find_screen_segment", "touch_segment_width: %f", touch_segment_width);
	LOGD("find_screen_segment", "int seg: %d", segment);
	return segment;
}

static float find_vel_value(float pos_y) {

	LOGD("find_vel_value", "pos_y: %f", pos_y);

	//float vel = 1 - (pos_y/(float)screen_height);
	//SLmillibel vol = (sender_vel * (VEL_SLMILLIBEL_RANGE/sender_range)) + VEL_SLMILLIBEL_MIN;
	float vel = (pos_y * (-0.6F/screen_height)) + 1.1F; // TODO 書き直すしなきゃ
	LOGD("find_vel_value", "vel: %f", vel);
	return vel;
}

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
	screen_margin_y_t = screen_height*0.187f;
	screen_margin_y_b = screen_height*0.26f;

	LOGD("get_screen_dimensions", "ANativeWindow_getWidth: %d", screen_width);
	LOGD("get_screen_dimensions", "ANativeWindow_getHeight: %d", screen_height);
	LOGD("get_screen_dimensions", "screen_margin_x_l: %d", screen_margin_x_l);
	LOGD("get_screen_dimensions", "screen_margin_x_r: %d", screen_margin_x_r);
	LOGD("get_screen_dimensions", "screen_margin_y_t: %d", screen_margin_y_t);
	LOGD("get_screen_dimensions", "screen_margin_y_b: %d", screen_margin_y_b);
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    engine* e = (engine*)app->userData;

    switch (cmd) {

        case APP_CMD_SAVE_STATE:
        	LOGDw("engine_handle_cmd", "APP_CMD_SAVE_STATE");
        	e->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)e->app->savedState) = e->state;
            e->app->savedStateSize = sizeof(struct saved_state);
            break;

        case APP_CMD_INIT_WINDOW:
        	LOGDw("engine_handle_cmd", "APP_CMD_INIT_WINDOW");
        	if (e->app->window != NULL) {
				LOGD("call_order", "APP_CMD_INIT_WINDOW");
				first_init(e);
			}
            break;

        case APP_CMD_START:
        	LOGDw("engine_handle_cmd", "APP_CMD_START");
			ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
        	break;

        case APP_CMD_TERM_WINDOW:
        	LOGDw("engine_handle_cmd", "APP_CMD_TERM_WINDOW");
			gles_term_display(&g_sc);
            break;

        case APP_CMD_GAINED_FOCUS:
        	LOGDw("engine_handle_cmd", "APP_CMD_GAINED_FOCUS");
        	kill_all_touch_circles();
        	draw_frame();
        	e->animating = 1;
            break;

        case APP_CMD_LOST_FOCUS:
        	LOGDw("engine_handle_cmd", "APP_CMD_LOST_FOCUS");
        	kill_all_touch_circles();
        	draw_frame();
        	e->animating = 0;
        	if (sles_init_called)wake_from_paused = TRUE;

        	// タッチの円形全部無効スべき
            pause_all_voices();
    		usleep(5000000);
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
// the very first initialization call order
void first_init(engine* e) {

	LOGD("call_order", "first_init() called");
	asset_manager = e->app->activity->assetManager;
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
	init_sles_components();
	return NULL;
}

//void init_sles_components(struct android_app* state) { // FIXME conflicting types for 'init_sles_components' [enabled by default]

//	AAssetManager* am = state->activity->assetManager;

void init_sles_components() {

	load_all_assets(asset_manager);// todo only loads sound files... need a to have a seperate Android asset loader that returns buffer data only
	// init_sound_buffers(); // TODO abstracted sound buffer init
	create_sl_engine();
	init_all_voices();
	init_random_seed();
	init_all_parts();
	init_auto_vals();

	sles_init_finished = TRUE;
	LOGD("init_sles_thread", "sles_init_finished = TRUE");
}

void init_sles_gain_focus(struct android_app* state) {

	create_sl_engine();
	init_all_voices();
	start_loop();
	int i;
	for(i=0;i<sizeof_textures_elements;i++) {
		struct texture_file* tf = textures+i;
		setup_texture(tf, 0.0f, asset_manager);
	}
}
void android_main(struct android_app* state) {
	LOGD("android_main", "android_main() called");

    engine e;
    app_dummy();
    memset(&e, 0, sizeof(engine));
    state->userData = &e;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    e.app = state;

	int i = 0;
	int animating = FALSE;

	while (1) {
		int ident, events;
		struct android_poll_source* source;

		while ((ident = ALooper_pollAll(0, NULL, &events, (void**) &source)) >= 0) {

			LOGD("call_order", "while ((ident=ALooper_pollAll...");
			LOGD("ALooper_pollAll", "ALooper_pollAll");
			if (source != NULL) {
				source->process(state, source);
			}
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
			if(!splash_bg_fading_in && elapsed_time > (3*SEC_IN_US)) { //3
				splash_bg_fading_in = TRUE;
				screens[1].is_showing = TRUE;
				LOGD("android_main", "splash_bg_fading_in = TRUE");
			}

			if(sles_init_finished && splash_bg_fading_in && screens[1].alpha == 1.0) {
				sles_init_finished = FALSE;
				assign_time(&splash_fadeout_time);
				LOGD("android_main", "sles_init_finished");
			}

			if(!splash_bg_fading_out && compare_times(splash_fadeout_time, (1*SEC_IN_US))) {
				splash_bg_fading_out = TRUE;
				screens[1].fading_in = FALSE;
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

			if(!touch_enabled && compare_times(touch_enable_time, (2*SEC_IN_US))) {
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

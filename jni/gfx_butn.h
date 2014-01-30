// gfx_butn.h

#ifndef GFX_BUTN_H_
#define GFX_BUTN_H_

#define BTN_W 128 // button width
#define BTN_ALPHA_MAX 1.0F
#define BTN_ALPHA_MIN 0.0F
#define BTN_FADE_IN_RATE (2.0F/(float)SEC_IN_US)
#define BTN_FADE_OUT_RATE (1.7F/(float)SEC_IN_US) //(1.5F
#define BTN_PRESS_FADE_RATE (7.0F/(float)SEC_IN_US)
#define BTN_PRESS_FADE_RATE_OUT (1.2F/(float)SEC_IN_US)//(1.8F/(

enum {

	TOUCH_EVENT_BUTTON_0 = 0,
	TOUCH_EVENT_BUTTON_1 = 1,
	TOUCH_EVENT_BUTTON_2 = 2,
	TOUCH_EVENT_INTERACTIVE_ON = 3,
	TOUCH_EVENT_HELP = 4,
	TOUCH_EVENT_GAME = 5,
	TOUCH_EVENT_NULL = 6

};

struct texture_file;
struct vec2;
struct vertex_rgb;

struct button{

	struct texture_file* main_texture;
	struct texture_file* pressed_texture;
	float gl_x, gl_y;
	float scale;
	float alpha;
	float alpha_pt;
	int pressed_peak;
	int busy;

	// bl: x=0, y=max tr: x=max, y=0
	struct vec2 touch_bl, touch_tr;
	int event_enum;

	int is_touch_anim;
	int fading_in;
	int fading_out;
	float fade_in_rate;
	float fade_out_rate;

	struct button* fade_in_next;
	struct button* fade_out_next;

	func_ptr fade_in_start;
	func_ptr fade_in_end;

	func_ptr fade_out_start;
	func_ptr fade_out_end;

	func_ptr touch_anim_start;
	func_ptr touch_anim_finish;

	struct vertex_rgb* rgb;
};

extern struct button buttons[];
extern struct vertex btn_quad[];
extern unsigned short btn_quad_index[];

extern int sizeof_button_array;
extern int sizeof_buttons;
extern int sizeof_btn_quad;
extern int sizeof_btn_quad_index;
extern int all_buttons_busy_fading;

void calc_btn_quad_verts(int bm_w, int bm_h);
int get_touch_response(float x, float y);
void btn_anim(struct button* b);

void all_btns_fade_start();
void all_btns_fade_end();
void all_btns_fade_end_deactivate();
void all_btns_fade_end_deactivate_show_help();

void touch_anim_start();
void touch_anim_finish();
void touch_anim_finish_help();

#endif /* GFX_BUTN_H_ */

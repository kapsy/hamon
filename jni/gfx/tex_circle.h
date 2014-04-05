// tex_circle.h

#ifndef TEX_CIRCLE_H_
#define TEX_CIRCLE_H_

#define TEX_CIRCLES_MAX 10
#define TEX_TO_W_RATIO 0.3f

struct vertex_rgb;

struct tex_circle {

	int is_alive;
	int fading_in;

	float pos_x;
	float pos_y;

	struct vertex_rgb* rgb;

	float alpha;
	float alpha_max;
//	float alpha_delta_factor;
	float alpha_fade_in;
	float alpha_fade_out;

	float scale;
	float scale_change_rate;

	struct texture_file* tex;
};

void init_tex_circles();
void activate_tex_circle(float x, float y, struct vertex_rgb* rgb_p, float* vel);
void activate_tex_no_ammo(float x, float y, float* vel);
void kill_all_tex_circles();
void tex_circle_alpha_size(struct tex_circle* ts);
void tex_ripple_alpha_size(struct tex_circle* tr);
void calc_tex_circle_vertex();

extern struct tex_circle tex_circles[];
extern unsigned int tex_circle_draw_order[];

extern struct tex_circle tex_ripples[];

extern int sizeof_tex_circles_e;
extern int sizeof_tex_circles;

extern struct vertex tex_circle_v[];
extern unsigned short tex_circle_i[];

extern int sizeof_tex_circle_v;
extern int sizeof_tex_circle_i;

#endif /* TEX_CIRCLE_H_ */

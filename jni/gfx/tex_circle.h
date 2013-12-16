/*
 * tex_circle.h
 *
 *  Created on: 2013/12/16
 *      Author: Michael
 */

#ifndef TEX_CIRCLE_H_
#define TEX_CIRCLE_H_

#define TEX_CIRCLES_MAX 24

#define TEX_TO_W_RATIO 0.5F


struct tex_circle {
	int is_alive;
	int fading_in;
	float pos_x;
	float pos_y;
	float rgb[3];
	float alpha;
	float alpha_max;
	float alpha_delta_factor;
	float scale;
	struct texture_file* tex;
//    GLuint  t_name;
};


void init_tex_circles();
void activate_tex_circle(float x, float y, size_t col, float* vel);
void kill_all_tex_circles();

void tex_circle_alpha_size(struct tex_circle* ts);
void tex_ripple_alpha_size(struct tex_circle* tr);
void calc_tex_circle_vertex();


extern pthread_mutex_t frame_mutex;

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

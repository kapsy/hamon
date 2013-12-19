/*
 * touch_circles.h
 *
 *  Created on: 2013/12/09
 *      Author: Michael
 */

#ifndef TOUCH_CIRCLES_H_
#define TOUCH_CIRCLES_H_


#define TOUCH_CIRCLES_MAX 24
//#define PI 3.14159265358979
//#define CIRCLE_SEGMENTS 24
#define CIRCLE_SEGMENTS 24




struct touch_circle {
	int is_alive;
	int fading_in;
	float pos_x;
	float pos_y;
	float rgb[3];
	float alpha;
	float alpha_max;
	float alpha_delta_factor;
	float scale;
};




void init_touch_circles();
void calc_circle_vertex();
void activate_touch_circle(float x, float y, size_t col, float* vel);
void kill_all_touch_circles();

void t_circle_alpha_size(struct touch_circle* ts);
void t_ripple_alpha_size(struct touch_circle* tr);


//extern pthread_mutex_t frame_mutex;

extern struct touch_circle t_circles[];
extern unsigned int t_circle_draw_order[];


extern struct touch_circle t_ripples[];

extern int sizeof_t_circles_e;
extern int sizeof_t_circles;

extern struct vertex solid_circle_v[];
extern unsigned short solid_circle_i[];

extern int sizeof_solid_circle_v;
extern int sizeof_solid_circle_i;


#endif /* TOUCH_CIRCLES_H_ */

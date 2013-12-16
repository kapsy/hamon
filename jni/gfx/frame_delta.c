/*
 * frame_delta.c
 *
 *  Created on: 2013/12/09
 *      Author: Michael
 */

#include <android/log.h>
#include <android_native_app_glue.h>

#include <unistd.h>  // sleep()‚ð’è‹`
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "hon_type.h"


#define DELTA_AVG_COUNT 6



struct timezone tzp;
struct timeval get_time;

unsigned long start_time = 0;

unsigned long curr_time = 0;
unsigned long new_time = 0;

unsigned long frame_delta = 0;
int frame_delta_avg[DELTA_AVG_COUNT];
size_t frame_delta_cycle = 0;

float frame_delta_ratio = 1.0; // ratio of delta to ideal fps (60)



unsigned long elapsed_time = 0;
unsigned long arbitrary_time = 0;



void get_time_long(unsigned long* t) {

	gettimeofday(&get_time, &tzp);

	*t = (get_time.tv_sec * SEC_IN_US) + get_time.tv_usec;

}

void get_start_time() {

	get_time_long(&start_time);
}


void reset_arb_time() {
	arbitrary_time = elapsed_time;
}

void assign_time(unsigned long* l) {

	*l = curr_time;
}




void update_elapsed_time() {

	elapsed_time = curr_time - start_time;

}

int comp_arb_time(long t) {
	int b = FALSE;
	if ((curr_time - arbitrary_time) > t) b = TRUE;

	return b;
}


int compare_times(unsigned long t, unsigned long u) {

//	LOGD("compare_times", "t: %u, u: %u", t, u);


	int b = FALSE;
	if ((curr_time - t) > u) b = TRUE;


	LOGD("compare_times", "(curr_time - t): %u, u: %u, b: %d", (curr_time - t), u, b);

	return b;
}




void frame_delta_avg_init() {

	int i;
	for (i = 0; i < DELTA_AVG_COUNT; i++) {
		frame_delta_avg[i] = 16000;
	}
}

void frame_delta_avg_calc() {

	if (frame_delta_cycle < DELTA_AVG_COUNT)
		frame_delta_cycle++;

	if (frame_delta_cycle == DELTA_AVG_COUNT)
		frame_delta_cycle = 0;

	frame_delta_avg[frame_delta_cycle] = frame_delta;

	int i;

	unsigned long d_tot;

	for (i = 0; i < DELTA_AVG_COUNT; i++) {
		d_tot += frame_delta_avg[i];

	}


	frame_delta = d_tot / DELTA_AVG_COUNT;

}


void calc_frame_delta_time() {

	if (curr_time <= 0) get_time_long(&curr_time);

	get_time_long(&new_time);
	frame_delta = new_time - curr_time;

//	LOGI("pi_draw", "delta: %d next_time: %u curr_time: %u", delta, next_time, curr_time);

	if (frame_delta > 100000) frame_delta = 100000;

//	LOGI("calc_delta_time", "frame_delta: %u", frame_delta);

	frame_delta_avg_calc();

//	LOGI("calc_delta_time", "frame_delta after frame_delta_avg_calc(): %u", frame_delta);



//	frame_delta_ratio = 18000.0F/(float) frame_delta;
//	if (frame_delta_ratio >= 1.0) frame_delta_ratio = 1.0;
//		LOGI("calc_delta_time", "frame_delta_ratio: %f", frame_delta_ratio);

	curr_time = new_time;

}



void calc_frame_rate() {


		LOGI("calc_frame_rate", "frame rate: %u", SEC_IN_US/frame_delta);

}

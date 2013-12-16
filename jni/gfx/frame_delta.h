/*
 * frame_delta.h
 *
 *  Created on: 2013/12/09
 *      Author: Michael
 */

#ifndef FRAME_DELTA_H_
#define FRAME_DELTA_H_



extern unsigned long frame_delta;
//extern float frame_delta_ratio;


extern unsigned long elapsed_time;

extern unsigned long arbitrary_time;

void frame_delta_avg_init();
void calc_frame_rate();


void reset_arb_time();
void update_elapsed_time();
int comp_arb_time(unsigned long t);



void assign_time(long* l);
int compare_times(unsigned long t, unsigned long u);

#endif /* FRAME_DELTA_H_ */

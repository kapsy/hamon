/*
 * scales.h
 *
 *  Created on: 2013/06/02
 *      Author: Michael
 *
 *      âπäKä«óù
 *
 *      Scale Management
 */

#ifndef SCALES_H_
#define SCALES_H_

//#define TOTAL_NOTES 24
#define TOTAL_NOTES 24

#include "snd_asst.h"

//void play_note(int segment, float vel);

sample_def* get_scale_sample(int seg);



void start_loop();
int cycle_scale();

#endif /* SCALES_H_ */

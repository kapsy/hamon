/*
 * snd_ctrl.h
 *
 *  Created on: 2013/06/10
 *      Author: Michael
 */

#ifndef SND_CTRL_H_
#define SND_CTRL_H_

void init_timing_loop();
void record_note(float x, float y, int seg, float vel);
void init_all_parts();


void tic_counter();
void play_all_parts();



#endif /* SND_CTRL_H_ */

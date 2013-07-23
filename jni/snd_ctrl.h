/*
 * snd_ctrl.h
 *
 *  Created on: 2013/06/10
 *      Author: Michael
 */



#ifndef SND_CTRL_H_
#define SND_CTRL_H_


#include <stdio.h>
#include <stdlib.h>
#include <time.h>



void init_control_loop();
void join_control_loop();

void record_note(float x, float y, int seg, float vel);
void init_all_parts();


void part_tic_counter();
void play_all_parts();

int decrease_ammo();

int obtain_random(int modulus);
void init_random_seed();

void set_parts_active();

//void shutdown_audio_delay();

void init_auto_vals();
//float* get_part_rgb();
size_t current_part_color();


#endif /* SND_CTRL_H_ */

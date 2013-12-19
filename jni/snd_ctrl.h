/*
 * snd_ctrl.h
 *
 *  Created on: 2013/06/10
 *      Author: Michael
 */



#ifndef SND_CTRL_H_
#define SND_CTRL_H_


#define AMMO_MAX 5


#define TOTAL_NOTES_PER_PART 32


struct vertex_rgb;


typedef struct {
	float pos_x;
	float pos_y;
	int seg;
	float vel;
	int tic;
}note;

typedef struct {

	note note_info[TOTAL_NOTES_PER_PART];
	int current_note;
	int total_tics;
	int current_tic;
	int is_recording;

	size_t play_count;
	size_t plays_ttl;
	float fade_out_factor;

	int is_alive;

	struct vertex_rgb* rgb;

}part;



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
//size_t current_part_color();

extern part parts[];


extern int current_rec_part;

extern size_t ammo_current;
extern size_t chord_count;
extern int playback_paused;

//extern int total_tic_counter;

#endif /* SND_CTRL_H_ */

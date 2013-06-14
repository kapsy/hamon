// ノートの再生と録音の管理、自動的な再生（AI)
// 弾いましたオートを全部ここに通さないといけない
//

#include <android/log.h>
#include <time.h>


// for threading
#include <unistd.h>  // sleep()を定義
#include <pthread.h>

#include <stdbool.h>

#include "snd_ctrl.h"
#include "snd_asst.h"
#include "snd_sles.h"
#include "snd_scal.h"



#define MAX_AMMO 1000
#define MIN_AMMO 0
#define AMMO_FACTOR 200 // velは1.0なら項ぐらいの量を減る

#define TICRATE 35

#define TOTAL_NOTES_PER_PART 64
#define TOTAL_PARTS 32


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

}part;

struct {

	// これは問題発生の原因かも
	struct timeval start_time;
	struct timeval finish_time;
	struct timeval proc_time;

	struct timeval sleep_time;
	//sleep_time.tv_usec = 20000;
//	sleep_time.tv_usec = 500000;
	//sleep_time.tv_sec = 2;
	struct timeval adjusted_sleep_time;

	struct timeval curr_time;
	struct timezone tzp;





	struct timespec start_time_s;
	struct timespec finish_time_s;

	struct timespec diff_time_s;



}timing;




part parts[TOTAL_PARTS];

static int current_rec_part = 0;
static int tics_per_part = 2000;
static int tic_increment = 0;

void* timing_loop(void* args);
void tic_counter();
void init_part(part* part, int rec);
void reset_all_notes(part* part);
void play_all_parts();

void add_tic_increment(int inc);




void init_timing_loop() {

	pthread_t fade_in;
	pthread_create(&fade_in, NULL, timing_loop, (void*)NULL);

}

//
//// コリラのほうが性格的に正しい
//void* timing_loop(void* args) {
//
//
//
//	while (1) {
//		gettimeofday(&timing.start_time, &timing.tzp);
//
//
//
//		    clock_gettime(CLOCK_MONOTONIC, &t);
//
//		// ノート再生処理
//
//		tic_counter();
//		play_all_parts();
//
//
//
//		// 処理終了
////		usleep(timing.sleep_time.tv_usec);
//
////
//		gettimeofday(&timing.finish_time, &timing.tzp);
//
//
//		usleep(20000 - (&timing.finish_time.tv_usec - &timing.start_time.tv_usec));
//
////
////		timersub(&timing.finish_time, &timing.start_time, &timing.proc_time);
////		timersub(&timing.sleep_time, &timing.proc_time, &timing.adjusted_sleep_time);
////
////		sleep(timing.adjusted_sleep_time.tv_sec);
////		usleep(timing.adjusted_sleep_time.tv_usec);
////
////		gettimeofday(&timing.curr_time, &timing.tzp);
////		__android_log_print(ANDROID_LOG_DEBUG, "sound_control_lroop", "gettimeofday: %d %d sleep_time: %d %d",
////				timing.curr_time.tv_sec, timing.curr_time.tv_usec, timing.adjusted_sleep_time.tv_sec, timing.adjusted_sleep_time.tv_usec);
//
//		gettimeofday(&timing.curr_time, &timing.tzp);
//		__android_log_print(ANDROID_LOG_DEBUG, "timing_loop", "gettimeofday: %d %d",
//				timing.curr_time.tv_sec, timing.curr_time.tv_usec);
//
//	}
//	return NULL;
//
//}

// コリラのほうが性格的に正しい
void* timing_loop(void* args) {



	while (1) {



		clock_gettime(CLOCK_MONOTONIC, &timing.start_time_s);

		// ノート再生処理

		tic_counter();
		play_all_parts();


	    clock_gettime(CLOCK_MONOTONIC, &timing.finish_time_s);

		// 処理終了
//		usleep(timing.sleep_time.tv_usec);

//

	    timing.diff_time_s.tv_nsec = (5000000 - (timing.finish_time_s.tv_nsec - timing.start_time_s.tv_nsec));


		nanosleep(&timing.diff_time_s, NULL);

//				__android_log_print(ANDROID_LOG_DEBUG, "timing_loop", "timing.diff_time_s.tv_nsec: %d",
//						timing.diff_time_s.tv_nsec);



//		usleep(20000 - (&timing.finish_time.tv_usec - &timing.start_time.tv_usec));

//
//		timersub(&timing.finish_time, &timing.start_time, &timing.proc_time);
//		timersub(&timing.sleep_time, &timing.proc_time, &timing.adjusted_sleep_time);
//
//		sleep(timing.adjusted_sleep_time.tv_sec);
//		usleep(timing.adjusted_sleep_time.tv_usec);
//
//		gettimeofday(&timing.curr_time, &timing.tzp);
//		__android_log_print(ANDROID_LOG_DEBUG, "sound_control_lroop", "gettimeofday: %d %d sleep_time: %d %d",
//				timing.curr_time.tv_sec, timing.curr_time.tv_usec, timing.adjusted_sleep_time.tv_sec, timing.adjusted_sleep_time.tv_usec);
//
//		gettimeofday(&timing.curr_time, &timing.tzp);
//		__android_log_print(ANDROID_LOG_DEBUG, "timing_loop", "gettimeofday: %d %d",
//				timing.curr_time.tv_sec, timing.curr_time.tv_usec);

	}
	return NULL;

}







// mainから呼ぶ
void record_note(float x, float y, int seg, float vel){

	part* p = (parts + current_rec_part);

//	__android_log_print(ANDROID_LOG_DEBUG, "record_note", "current_rec_part %d",
//			current_rec_part);

	int tic = p->current_tic;
	int n = p->current_note;

	p->note_info[n].pos_x = x;
	p->note_info[n].pos_y = y;

	p->note_info[n].seg = seg;
	p->note_info[n].vel = vel;

	p->note_info[n].tic = tic;

	__android_log_print(ANDROID_LOG_DEBUG, "record_note", "current_rec_part %d, current_tic %d, current_note %d",
			current_rec_part,p->current_tic, p->current_note);

	p->current_note++;

}

//// 毎TIC実行しなきゃ //
//// ticを全部進めないといけない
//void tic_counter() {
//	//__android_log_write(ANDROID_LOG_DEBUG, "tic_counter", "tic_counter() called");
//
//	 int p;
//	int total_parts = TOTAL_PARTS;
//
////	for (p = 0; p < TOTAL_PARTS; p++) {
//	for (p = 0; p <= current_rec_part; p++) {
//
//		//part* part = (parts + current_rec_part);
//
//
//		part* part = (parts + p);
//
//
//		__android_log_print(ANDROID_LOG_DEBUG, "tic_counter", "parts + %d, current_tic: %d",
//				p, part->current_tic);
//
//		if (part->current_tic >= part->total_tics) {
//			__android_log_write(ANDROID_LOG_DEBUG, "tic_counterif1", "if (part->current_tic >= part->total_tics)");
//
//			if (part->is_recording) {
//				part->is_recording = 0;
//				part->current_tic = 0;
//				current_rec_part++;
//				init_part(parts + current_rec_part, 1);
//
//			} else {
//				part->current_tic = 0;
//			}
//
//		} else if (part->current_tic < part->total_tics) {
//
//			__android_log_write(ANDROID_LOG_DEBUG, "tic_counterif2", "if (part->current_tic < part->total_tics)");
//
//			if (part->is_recording && part->current_note > 0) {
//				part->current_tic++;
//
//			} else if (!part->is_recording) {
//
//				part->current_tic++;
//			}
//
//		}
//
//	}
//
//}


// 毎TIC実行しなきゃ //
// ticを全部進めないといけない
void tic_counter() {
	//__android_log_write(ANDROID_LOG_DEBUG, "tic_counter", "tic_counter() called");

	 int i;
	int total_parts = TOTAL_PARTS;

//	for (p = 0; p < TOTAL_PARTS; p++) {
//		__android_log_print(ANDROID_LOG_DEBUG, "tic_counter", "current_rec_part: %d", current_rec_part);


	for (i = 0; i <= current_rec_part; i++) {


		part* p = (parts + i);

//		__android_log_print(ANDROID_LOG_DEBUG, "tic_counter", "parts [%d] ", i);
//		__android_log_print(ANDROID_LOG_DEBUG, "tic_counter", "p->current_tic: %d", p->current_tic);
//		__android_log_print(ANDROID_LOG_DEBUG, "tic_counter", "p->is_recording: %d", p->is_recording);
//		__android_log_print(ANDROID_LOG_DEBUG, "tic_counter", "p->total_tics: %d", p->total_tics);


		if (p->current_tic >= p->total_tics && p->is_recording) {

//			__android_log_write(ANDROID_LOG_DEBUG, "tic_counter_logic", "1");

							p->is_recording = 0;
							p->current_tic = 0;
							current_rec_part++;
							init_part(parts + current_rec_part, 1);

		} else if (p->current_tic >= p->total_tics && !p->is_recording) {
//			__android_log_write(ANDROID_LOG_DEBUG, "tic_counter_logic", "2");
							p->current_tic = 0;

		} else if (p->current_tic < p->total_tics && p->is_recording && p->current_note > 0) {
//			__android_log_write(ANDROID_LOG_DEBUG, "tic_counter_logic", "3");
							p->current_tic++;
		} else if (p->current_tic < p->total_tics && !p->is_recording) {
//			__android_log_write(ANDROID_LOG_DEBUG, "tic_counter_logic", "4");
							p->current_tic++;

		}

	}

}

//void init_first_part() {
//
//	init_rec_part(parts+ 0);
//
//}

void init_all_parts() {


	 int i;
	int total_parts = TOTAL_PARTS;

	for (i = 0; i < total_parts; i++) {

		part* p = (parts + i);

		if (i==0) init_part(p, 1);
		else	init_part(p, 0);

	}
}

void init_part(part* p, int rec) {

	p->current_note = 0;
	p->current_tic = 0;
	p->is_recording = rec;
	p->total_tics = tics_per_part + tic_increment;

	add_tic_increment(2);

	__android_log_print(ANDROID_LOG_DEBUG, "init_part", "p->total_tics  %d", p->total_tics);

	reset_all_notes(p);

}

void reset_all_notes(part* p) {

	int n;

	int total_notes = sizeof p->note_info / sizeof p->note_info[0];

	for (n = 0; n < total_notes; n++) {

		note* note = (p->note_info) + n;

		note->pos_x = 0.0F;
		note->pos_y = 0.0F;
		note->vel = 0.0F;
		note->tic = 0;
		note->vel = 0;
	}
}

void add_tic_increment(int inc) {

	tic_increment += inc;


}


void play_all_parts() {

	int i;

	//int total_parts = sizeof parts / sizeof parts[0];

	// int total_parts = current_rec_part; // これまでにチェックすれば十分。

	// もしかすると、必要なのは循環的な機能
	//#define TOTAL_PARTS

	int total_parts = TOTAL_PARTS;

	for (i = 0; i < total_parts; i++) {

		part* p = (parts + i);


		if (!p->is_recording) {

			int n;

			//int total_notes = sizeof part->note_info / sizeof part->note_info[0];

			int total_notes = p->current_note; // この方が一番早い

			// p->total_notes という変数は必要？


			for (n = 0; n < total_notes; n++) {

				note* note = (p->note_info) + n;

				if (note->tic == p->current_tic) {
					play_note(note->seg, note->vel);

				 	__android_log_print(ANDROID_LOG_DEBUG, "play_all_parts", "part: %d tic: %d current_tic: %d", i, note->tic, p->current_tic);


					//draw_note(note->pos_x, note->pos_y);
				}

			}

		}

	}

}










//static int current_ammo;


//// I_GetTime
//// returns time in 1/70th second tics
////
int  I_GetTime (void)
{
    struct timeval	tp;
    struct timezone	tzp;
    int			newtics;
    static int		basetime=0;

    gettimeofday(&tp, &tzp);
    if (!basetime)
	basetime = tp.tv_sec;
    newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
    return newtics;
}

//
//#include <time.h>
//#include <stdio.h>
//
//main()
//{
//        struct timespec tp ;
//        clock_gettime(CLOCK_MONOTONIC, &tp) ;
//        printf("%llu %llu\n", tp.tv_sec, tp.tv_nsec) ;
//}


//
////
//// D-DoomLoop()
//// Not a globally visible function,
////  just included for source reference,
////  called by D_DoomMain, never exits.
//// Manages timing and IO,
////  calls all ?_Responder, ?_Ticker, and ?_Drawer,
////  calls I_GetTime, I_StartFrame, and I_StartTic
////
//void D_DoomLoop (void);
//
//
////
////  D_DoomLoop
////
//extern  boolean         demorecording;
//
//void D_DoomLoop (void)
//{
//    if (demorecording)
//	G_BeginRecording ();
//
//    if (M_CheckParm ("-debugfile"))
//    {
//	char    filename[20];
//	sprintf (filename,"debug%i.txt",consoleplayer);
//	printf ("debug output to: %s\n",filename);
//	debugfile = fopen (filename,"w");
//    }
//
//    I_InitGraphics ();
//
//    while (1)
//    {
//	// frame syncronous IO operations
//	I_StartFrame ();
//
//	// process one or more tics
//	if (singletics)
//	{
//	    I_StartTic ();
//	    D_ProcessEvents ();
//	    G_BuildTiccmd (&netcmds[consoleplayer][maketic%BACKUPTICS]);
//	    if (advancedemo)
//		D_DoAdvanceDemo ();
//	    M_Ticker ();
//	    G_Ticker ();
//	    gametic++;
//	    maketic++;
//	}
//	else
//	{
//	    TryRunTics (); // will run at least one tic
//	}
//
//	S_UpdateSounds (players[consoleplayer].mo);// move positional sounds
//
//	// Update display, next frame, with current state.
//	D_Display ();
//
//#ifndef SNDSERV
//	// Sound mixing for the buffer is snychronous.
//	I_UpdateSound();
//#endif
//	// Synchronous sound output is explicitly called.
//#ifndef SNDINTR
//	// Update sound output.
//	I_SubmitSound();
//#endif
//    }
//}
//
//



//int  I_GetTime (void)
//{
//    struct timeval	tp;
//    struct timezone	tzp;
//    int			newtics;
//    static int		basetime=0;
//
//    gettimeofday(&tp, &tzp);
//    if (!basetime)
//	basetime = tp.tv_sec;
//    newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
//    return newtics;
//}
//
//



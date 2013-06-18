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

#define NS_IN_SEC 1000000000

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

	size_t ammo;

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
//part_buf parts[TOTAL_PARTS];

static int current_rec_part = 0;
static int tics_per_part = 1500;
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



// コリラのほうが性格的に正しい
void* timing_loop(void* args) {





	while (1) {
//		clock_gettime(CLOCK_MONOTONIC, &timing.start_time_s);
//		// 処理
//
//	    clock_gettime(CLOCK_MONOTONIC, &timing.finish_time_s);
//
//	    // StackOverflowからの
////	    (timing.finish_time_s.tv_sec * NS_IN_SEC + timing.finish_time_s.tv_nsec) - (timing.start_time_s.tv_nsec * NS_IN_SEC + timing.start_time_s.tv_nsec)；
//
//	    timing.diff_time_s.tv_nsec = (5000000 - (timing.finish_time_s.tv_nsec - timing.start_time_s.tv_nsec));
//		nanosleep(&timing.diff_time_s, NULL);


		// これだけで十分あまり性格的なタイミングが必要ないかも
		usleep(100000); // 100ミリ秒

		fade_automation();










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



// 毎TIC実行しなきゃ //
// ticを全部進めないといけない
void tic_counter() {

	int i;
	int total_parts = TOTAL_PARTS;

	for (i = 0; i <= current_rec_part; i++) {

		part* p = (parts + i);

		if (p->current_tic >= p->total_tics && p->is_recording) {

							p->is_recording = 0;
							p->current_tic = 0;
							current_rec_part++;
							init_part(parts + current_rec_part, 1);

		} else if (p->current_tic >= p->total_tics && !p->is_recording) {
							p->current_tic = 0;

		} else if (p->current_tic < p->total_tics && p->is_recording && p->current_note > 0) {
							p->current_tic++;

		} else if (p->current_tic < p->total_tics && !p->is_recording) {
							p->current_tic++;


		}

	}

}



void init_all_parts() {

total_tic_counter =0;

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

	add_tic_increment(4);

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

// パートの長さを異なる
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

			int total_notes = p->current_note; // この方が一番早い

			// p->total_notes という変数は必要？


			for (n = 0; n < total_notes; n++) {

				note* note = (p->note_info) + n;

				if (note->tic == p->current_tic) {
					play_note(note->seg, note->vel);

				 	__android_log_print(ANDROID_LOG_DEBUG, "play_all_parts", "total_tic_counter: %d: part: %d tic: %d current_tic: %d", total_tic_counter, i, note->tic, p->current_tic);


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



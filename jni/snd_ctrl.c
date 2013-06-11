// ノートの再生と録音の管理、自動的な再生（AI)
// 弾いましたオートを全部ここに通さないといけない
//

#include <android/log.h>
#include <time.h>

// for threading
#include <unistd.h>  // sleep()を定義
#include <pthread.h>

#include "snd_ctrl.h"
#include "snd_asst.h"
#include "snd_sles.h"



#define MAX_AMMO 1000
#define MIN_AMMO 0
#define AMMO_FACTOR 200 // velは1.0なら項ぐらいの量を減る

#define TICRATE 35





void init_snd_ctrl_loop() {

	pthread_t fade_in;
	pthread_create(&fade_in, NULL, sound_control_loop, (void*)NULL);


}

//
//#include <stdio.h>
//#include <time.h>
//
//int main() {
//  struct timespec delay;
//
//  delay.tv_sec=1;
//  delay.tv_nsec=500000000;
//  printf("sleep(1.5 sec)\n");
//  nanosleep(&delay,NULL);
//  printf("end\n");
//  return 0;
//}



void* sound_control_loop(void* args) {

//	struct timespec start_time ;
//
//
//	// timevalのほうがいいかも
//
//
//	struct timespec finish_time;
//
//	struct timespec sleep_time;

	struct timeval start_time ;
	struct timeval finish_time;
	struct timeval diff_time;

	struct timeval wait_time;

	wait_time.tv_sec = 2;


	struct timeval sleep_time;


struct timeval curr_time;


    struct timezone	tzp;

	while (1) {
		// clock を使うと駄目！不正確な結果

		gettimeofday(&start_time, &tzp);
		//clock_gettime(CLOCK_MONOTONIC, &start_time) ;



		// アプリの処理・録音、再生、フェードなど


		//マイクロ秒(μs)は100万分の1 (10-6) 秒に等しい。

		usleep(1000000);

		gettimeofday(&finish_time, &tzp);
		//clock_gettime(CLOCK_MONOTONIC, &finish_time) ;


		timersub(&finish_time, &start_time, &diff_time);



		timersub(&wait_time, &diff_time, &sleep_time);

		gettimeofday(&curr_time, &tzp);
		__android_log_print(ANDROID_LOG_DEBUG, "sound_control_loop", "gettimeofday: %d %d sleep_time: %d %d",
				curr_time.tv_sec, curr_time.tv_usec, sleep_time.tv_sec, sleep_time.tv_usec);




		usleep(sleep_time.tv_usec);




		//__android_log_print(ANDROID_LOG_DEBUG, "sound_control_loop", "%d %d", diff_time.tv_sec, diff_time.tv_usec);







		//sleep_time.tv_sec=0;
	//	diff_time.tv_nsec= 100000000;



	//	nanosleep(&sleep_time, NULL);

		//usleep(100000);
//		usleep(1000000);

	//	__android_log_print(ANDROID_LOG_DEBUG, "sound_control_loop", "%d %d", start_time.tv_sec, start_time.tv_nsec);
//		__android_log_print(ANDROID_LOG_DEBUG, "sound_control_loop", "%llu %llu\n", tp.tv_sec, tp.tv_nsec);


	}
	return NULL;

}



//static int current_ammo;


//
//
//
////
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



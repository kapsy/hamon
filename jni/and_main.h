/*
 * and_main.h
 *
 *  Created on: 2013/06/21
 *      Author: Michael
 */

#ifndef AND_MAIN_H_
#define AND_MAIN_H_

//extern int sles_init_called;

//#include <jni.h>
//#include <errno.h>
//
//#include <android/sensor.h>
//#include <android/log.h>
//#include <android_native_app_glue.h>
//
//// kapsy
//#include <android/asset_manager.h>
//#include <android/storage_manager.h>
//#include <android/window.h>
//
//#include <math.h>
//#include <time.h>
//#include "snd_sles.h"
//#include "snd_scal.h"
//#include "snd_asst.h"
//#include "snd_ctrl.h"
//#include "gfx_gles.h"
//#include "and_main.h"
//#include "hon_type.h"
//#include "gfx_butn.h"



void trigger_note(float x, float y);

extern int sles_init_called;
extern int show_gameplay;
extern int show_help;


extern size_t screen_width;
extern size_t screen_height;
extern size_t screen_height_reduced;
extern size_t screen_margin_y;
extern size_t screen_margin_x;

#endif /* AND_MAIN_H_ */

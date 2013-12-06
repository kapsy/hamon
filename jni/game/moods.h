/*
 * moods.h
 *
 *  Created on: 2013/12/04
 *      Author: Michael
 */

#ifndef MOODS_H_
#define MOODS_H_

//#include "gfx/background.h"

//#include "snd_scal.h"


struct scale;
struct background;


struct mood {

	char* title;
	struct background* bg;


	struct scale* scale;
////touch color?
//	//touch texture?

};


extern struct mood moods[];
extern int selected_mood;
extern int sizeof_moods_elements;


int init_mood();
int cycle_mood();

#endif /* MOODS_H_ */

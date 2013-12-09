/*
 * moods.h
 *
 *  Created on: 2013/12/04
 *      Author: Michael
 */

#ifndef MOODS_H_
#define MOODS_H_


//#include "gfx/vertex.h"

//#include "gfx/background.h"

//#include "snd_scal.h"

#define TOTAL_MOODS 2

struct scale;
struct background;


struct mood {

	char* title;
//	struct background* bg;

	struct vertex_rgb* colors;
	int color_index;


	struct scale* scale;
////touch color?
//	//touch texture?

};





extern struct mood moods[];
extern int selected_mood;
extern int sizeof_moods_elements;

extern int sizeof_mood_colors_set;



int init_mood();
int cycle_mood();

#endif /* MOODS_H_ */

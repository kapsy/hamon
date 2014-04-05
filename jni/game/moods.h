// moods.h

#ifndef MOODS_H_
#define MOODS_H_

#define TOTAL_MOODS 5

struct scale;
struct background;

struct mood {
	char* title;
	struct vertex_rgb* rgb_bg;
	struct vertex_rgb* rgb_circ;
	struct vertex_rgb* rgb_circ_mask;
	int color_index; //GLES のため
	struct scale* scale;
};

extern struct mood moods[];
extern int selected_mood;
extern int sizeof_moods_elements;
extern int sizeof_mood_colors_set;

extern struct vertex_rgb* no_ammo_touch_rgb;

int init_mood();
int cycle_mood();

#endif /* MOODS_H_ */

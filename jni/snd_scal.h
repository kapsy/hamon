// snd_scal.h

#ifndef SCALES_H_
#define SCALES_H_

#define TOTAL_NOTES 24
#define START_NOTE 48 // MIDI番号, MIDI number

struct sample_def;

struct scale {

	int midimap[TOTAL_NOTES];
	struct sample_def* looping_sample;
};

struct sample_def* get_scale_sample(int seg);
void start_loop();

#endif /* SCALES_H_ */

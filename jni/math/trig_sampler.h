// trig_sampler.h

#ifndef TRIG_SAMPLER_H_
#define TRIG_SAMPLER_H_

#define SAMPLES_PER_CYCLE 44100

struct modulator {
	double* mod_cycle;
	int mod_type;
	int curr_samp;
	double period; // time in sec/cycle
	float amp;
	double samples_per_us;
};

enum {
	MOD_SINE = 0,
	MODE_SAW = 1,
	MOD_SQUARE = 2,
};

extern struct modulator modulators[];

void calc_all_spu();
void init_all_trig_samples();
void cycle_modulator(struct modulator* m);

#endif /* TRIG_SAMPLER_H_ */

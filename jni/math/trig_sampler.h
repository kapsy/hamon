/*
 * trig_sampler.h
 *
 *  Created on: 2013/12/11
 *      Author: Michael
 */

#ifndef TRIG_SAMPLER_H_
#define TRIG_SAMPLER_H_

#define SAMPLES_PER_CYCLE 44100



struct modulator {

	double* mod_cycle;

	int mod_type;
	int curr_samp;
	double period; // time in sec/cycle

//	int us_per_sample;
	double samples_per_us;
	float amp;
};


enum {
	MOD_SINE = 0,
	MODE_SAW = 1,
	MOD_SQUARE = 2,
};


//extern double* hi_res_sin;
extern struct modulator modulators[];

void calc_all_spu();
void init_all_trig_samples();
void cycle_modulator(struct modulator* m);

#endif /* TRIG_SAMPLER_H_ */
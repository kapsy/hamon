/*
 * snd_asst.h
 *
 *  Created on: 2013/06/03
 *      Author: Michael
 */

#ifndef SND_ASST_H_
#define SND_ASST_H_

#include <android/asset_manager.h>
#include <android/storage_manager.h>

typedef struct {
	char* file_name;
	int midi_number;
	//unsigned sample_size;

//unsigned char* buffer_header;
	unsigned short* buffer_header;
	unsigned short* buffer_data;
	size_t data_size;

} oneshot_def;

extern oneshot_def oneshot_samples[];
extern oneshot_def looping_samples[];

extern char* internal_path;


void load_all_assets(AAssetManager* mgr);




#endif /* SND_ASST_H_ */

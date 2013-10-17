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

//	signed short* buffer_header;
//	signed short* buffer_data;
	unsigned short* buffer_header;
	unsigned short* buffer_data;

	size_t data_size;
	size_t total_chunks;

} sample_def;

extern sample_def oneshot_samples[];
extern sample_def looping_samples[];

extern sample_def silence_chunk;

extern char* internal_path;


void load_all_assets(AAssetManager* mgr);

void init_silence_chunk();



void create_file_load_thread(AAssetManager* mgr);




#endif /* SND_ASST_H_ */

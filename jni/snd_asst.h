// snd_asst.h

#ifndef SND_ASST_H_
#define SND_ASST_H_

struct sample_def {

	char* file_name;
	int midi_number;

	unsigned short* buffer_header;
	unsigned short* buffer_data;

	size_t data_size;
	size_t total_chunks;

	float vol_factor;
};

extern struct sample_def oneshot_samples[];
extern struct sample_def looping_samples[];

extern struct sample_def silence_chunk;

extern char* internal_path;

//void load_all_assets(AAssetManager* mgr);
void init_silence_chunk();
//void create_file_load_thread(AAssetManager* mgr);

#endif /* SND_ASST_H_ */

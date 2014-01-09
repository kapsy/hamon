// snd_asst.c

#include "common.h"
#include "snd_asst.h"
#include "hon_type.h"

# define HEADER_SIZE 44

char* internal_path;

char* string_join(const char* a, const char* b, const char* c);
//char* string_join_src(const char* join_a, const char* join_b);
void open_external_file(struct sample_def* sample_def);
void init_silence_chunk();
void malloc_to_buffer_factor(struct sample_def* s);

struct sample_def silence_chunk = {"no_file_name", 00, NULL, NULL, 0, 0, 1.0F};

struct sample_def looping_samples[] = {
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/scale_loop_16_major_001_n22.wav", 48, NULL, NULL, 0, 0, 0.5F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/scale_loop_16_minor_001_n22.wav", 48, NULL, NULL, 0, 0, 0.5F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/scale_loop_16_cirrostratus_004_n22.wav", 48, NULL, NULL, 0, 0, 0.5F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/scale_loop_16_cumulonimbus_003_n22.wav", 48, NULL, NULL, 0, 0, 0.4F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/scale_loop_16_cirrostratus_002_n22.wav", 48, NULL, NULL, 0, 0, 0.4F}
};

struct sample_def oneshot_samples[] = {
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_48_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_49_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_50_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_51_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_52_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_53_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_54_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_55_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_56_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_57_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_58_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_59_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_60_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_61_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_62_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_63_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_64_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_65_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_66_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_67_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_68_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_69_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_70_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_71_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_72_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_73_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_74_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_75_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_76_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_77_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_78_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_79_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_80_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_81_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_82_n22.wav", 48, NULL, NULL, 0, 0, 1.0F},
		{"/mnt/sdcard/Android/data/nz.kapsy.hamon/files/hontouniiioto_heavy_83_n22.wav", 48, NULL, NULL, 0, 0, 1.0F}
};

void load_all_assets(AAssetManager* mgr) { // TODO use open_asset() instead of open_external_file()
	LOGD("sound_load_thread", "load_all_assets");
	init_silence_chunk();
	int success; //必要ない

	int i;
	for (i = 0; i < sizeof oneshot_samples / sizeof oneshot_samples[0]; i++) {

		/*
		 char* path;
		 path = string_join_src(internal_path, oneshot_samples[i].file_name);

		 __android_log_print(ANDROID_LOG_DEBUG, "load_all_assets",
		 "string_join: %s", path);

		 size_t lpath = strlen(path);
		 oneshot_samples[i].file_name = malloc(lpath);

		 memcpy(oneshot_samples[i].file_name, path, lpath);
		 */

		LOGD("load_all_assets", "oneshot_samples[%d].file_name: %s",
				i, oneshot_samples[i].file_name);

		open_external_file(oneshot_samples + i);
		//	open_asset(mgr, oneshot_samples[i].file_name, i);

//		 int d;
//
//			for (d = 0; d < 150; d++) {
//				__android_log_print(ANDROID_LOG_DEBUG, "load_all_assets",
//						"oneshot_samples[%d].buffer_header[%d], (start) x: %x c: %c",
//						i, d, oneshot_samples[i].buffer_header[d],
//						oneshot_samples[i].buffer_header[d]);
//			}
//			for (d = 0; d < 20; d++) {
//				__android_log_print(ANDROID_LOG_DEBUG, "load_all_assets",
//						"oneshot_samples[%d].buffer_data[%d], (start) x: %x c: %c",
//						i, d, oneshot_samples[i].buffer_data[d],
//						oneshot_samples[i].buffer_data[d]);
//			}

		LOGD("load_all_assets", "oneshot_samples[i].data_size: %x",
				oneshot_samples[i].data_size);
		LOGD("load_all_assets", "&(oneshot_samples[i].data_size: %x",
				&(oneshot_samples[i].data_size));
	}

	for (i = 0; i < sizeof looping_samples / sizeof looping_samples[0]; i++) {

		LOGD("load_all_assets", "looping_samples[%d].file_name: %s",
				i, looping_samples[i].file_name);
		open_external_file(looping_samples + i);
		LOGD("load_all_assets", "looping_samples[i].data_size: %x",
				looping_samples[i].data_size);
		LOGD("load_all_assets", "&(looping_samples[i].data_size: %x",
				&(looping_samples[i].data_size));
	}
	LOGD("load_all_assets", "loading finished");
	LOGD("sound_load_thread", "load_all_assets loading finished");
}

void init_silence_chunk() {

	struct sample_def* s = &silence_chunk;
	s->data_size = BUFFER_SIZE;
	s->buffer_data = (unsigned short*) malloc(BUFFER_SIZE);

	int i;
	for (i=0; i<(BUFFER_SIZE/2); i++) {
		s->buffer_data[i] = 0x0000;
	}
}

char* string_join(const char* a, const char* b, const char* c) {

	size_t la = strlen(a);
	size_t lb = strlen(b);
	size_t lc = strlen(c);

	char* p = malloc(la + lb + lc + 1);
	memcpy(p, a, la);
	memcpy(p + la, b, lb);
	memcpy(p + la + lb, c, lc + 1);

	return p;
}

// TODO rename to open_external_wav
void open_external_file(struct sample_def* s) {

	LOGD("open_external_file", "open_external_file(sample_def* s) called");
	FILE* fp;

	s->buffer_header = (unsigned short*) malloc(HEADER_SIZE);
	LOGD("open_external_file", "filepath: %s", s->file_name);

	if ((fp = fopen(s->file_name, "r")) != NULL) {
		LOGD("open_external_file", "fopen()");
		fread(s->buffer_header, 1, HEADER_SIZE, fp);
	}

	// 変数の週類はポインターである
	unsigned short* fmttype;
	unsigned long* databytes;

	fmttype = (s->buffer_header + 10);
	if (*fmttype != 0x1) {
		LOGD("open_external_file", "*fmttype not PCM, loading aborted.");
	}

	databytes = (s->buffer_header + 20);
	s->data_size = *databytes;

	// 必要な処理
	malloc_to_buffer_factor(s);

	//s->buffer_data = (unsigned short*) malloc(*databytes);

	LOGD("open_external_file", "*fmttype: %x", *fmttype);
	LOGD("open_external_file", "*databytes: %x", *databytes);

	fseek(fp , HEADER_SIZE , SEEK_SET);
	fread(s->buffer_data, 1, s->data_size, fp);

	fclose(fp);
	LOGD("open_external_file", "fclose(fp);");
}

// The length of the sample buffer must be a multiple of the buffer chunk size.
// This is done by filling out the end of the sample buffer with silence.
void malloc_to_buffer_factor(struct sample_def* s) {

	size_t buffer_over = s->data_size % BUFFER_SIZE;

	LOGD("open_external_file", "s->data_size: %d", s->data_size);
	LOGD("open_external_file", "buffer_over: %d", buffer_over);

	if (buffer_over > 0 && buffer_over < BUFFER_SIZE) {

		size_t buffer_rem = BUFFER_SIZE - buffer_over;
		LOGD("open_external_file", "buffer_rem: %d", buffer_rem);
		s->buffer_data = (unsigned short*) malloc(s->data_size + buffer_rem );
		s->total_chunks = (s->data_size + buffer_rem) / BUFFER_SIZE;
		int i;
		for (i=s->data_size; i<(buffer_rem/2); i++)	{
			s->buffer_data[i] =  0x0000;
		}
	}
	else if (buffer_over == 0) {
		s->buffer_data = (unsigned short*) malloc(s->data_size);
		s->total_chunks = (s->data_size) / BUFFER_SIZE;
	}
	LOGD("open_external_file", "s->total_chunks: %d", s->total_chunks);
}


// 音声ファイルをロードするため

#include <assert.h>
#include <jni.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <android/log.h>;
#include <android/obb.h>;


#include "snd_asst.h"
#include "hon_type.h"

# define HEADER_SIZE 44

char* internal_path;

char* string_join(const char* a, const char* b, const char* c);

char* string_join_src(const char* join_a, const char* join_b);
//void open_external_file(char* filepath, int samp);
void open_external_file(sample_def* sample_def);
void init_silence_chunk();
void malloc_to_buffer_factor(sample_def* s);

//sample_def test[] = {
//		{"blah", 48, NULL, NULL}
//
//};


sample_def silence_chunk = {"no_file_name", 00, NULL, NULL};

sample_def looping_samples[] = {


		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/scale_loop_16_major_001.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/scale_loop_16_minor_001.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/scale_loop_16_cumulonimbus_002.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/scale_loop_16_cirrostratus_002.wav", 48, NULL, NULL}

};


// 今の立場4個だけでいいかも
sample_def oneshot_samples[] = {
//
//		{"hontouniiioto_heavy_48.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_49.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_50.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_51.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_52.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_53.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_54.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_55.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_56.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_57.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_58.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_59.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_60.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_61.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_62.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_63.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_64.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_65.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_66.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_67.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_68.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_69.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_70.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_71.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_72.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_73.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_74.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_75.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_76.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_77.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_78.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_79.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_80.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_81.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_82.wav", 48, NULL, NULL},
//		{"hontouniiioto_heavy_83.wav", 48, NULL, NULL}

//		06-05 15:37:30.598: D/load_all_assets(29666):
//		string_join: /mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_48.wav


		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_48.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_49.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_50.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_51.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_52.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_53.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_54.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_55.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_56.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_57.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_58.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_59.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_60.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_61.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_62.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_63.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_64.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_65.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_66.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_67.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_68.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_69.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_70.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_71.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_72.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_73.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_74.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_75.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_76.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_77.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_78.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_79.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_80.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_81.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_82.wav", 48, NULL, NULL},
		{"/mnt/sdcard/Android/data/nz.kapsy.hontouniiioto/files/hontouniiioto_heavy_83.wav", 48, NULL, NULL}


/*		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_48.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_49.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_50.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_51.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_52.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_53.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_54.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_55.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_56.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_57.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_58.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_59.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_60.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_61.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_62.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_63.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_64.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_65.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_66.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_67.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_68.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_69.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_70.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_71.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_72.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_73.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_74.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_75.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_76.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_77.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_78.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_79.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_80.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_81.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_82.wav", 48, NULL, NULL},
		{"/mnt/sdcard/hontouniiioto/hontouniiioto_heavy_83.wav", 48, NULL, NULL}*/

};


void load_all_assets(AAssetManager* mgr) {


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

		__android_log_print(ANDROID_LOG_DEBUG, "load_all_assets",
				"oneshot_samples[%d].file_name: %s", i,
				oneshot_samples[i].file_name);

		//open_external_file(oneshot_samples[i].file_name, i);

//		open_external_file(*(oneshot_samples[i]));

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



		__android_log_print(ANDROID_LOG_DEBUG, "load_all_assets",
				"oneshot_samples[i].data_size: %x",
				oneshot_samples[i].data_size);
		__android_log_print(ANDROID_LOG_DEBUG, "load_all_assets",
				"&(oneshot_samples[i].data_size: %x",
				&(oneshot_samples[i].data_size));

		//if (success == 0) break;
	}

	for (i = 0; i < sizeof looping_samples / sizeof looping_samples[0]; i++) {

		__android_log_print(ANDROID_LOG_DEBUG, "load_all_assets",
				"looping_samples[%d].file_name: %s", i,
				looping_samples[i].file_name);

		open_external_file(looping_samples + i);

		__android_log_print(ANDROID_LOG_DEBUG, "load_all_assets",
				"looping_samples[i].data_size: %x",
				looping_samples[i].data_size);
		__android_log_print(ANDROID_LOG_DEBUG, "load_all_assets",
				"&(looping_samples[i].data_size: %x",
				&(looping_samples[i].data_size));

	}

	__android_log_write(ANDROID_LOG_DEBUG, "load_all_assets",
			"loading finished");
}

void init_silence_chunk() {

	sample_def* s = &silence_chunk;

	s->data_size = BUFFER_SIZE;

	s->buffer_data = (unsigned short*) malloc(BUFFER_SIZE);

	int i;

	for (i=0; i<(BUFFER_SIZE/2); i++) {
		//s->buffer_data[i] = 0x7FFF;
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



// so these are copies of the pointer not the pointer itself, just like any other argument
// 復讐しなきゃ・今の立場って言えば、OBB方式のファイルは一番と思う
void open_external_file(sample_def* s) {

	__android_log_write(ANDROID_LOG_DEBUG, "open_external_file", "open_external_file(sample_def* s) called");

	FILE* fp;

	s->buffer_header = (unsigned short*) malloc(HEADER_SIZE);
	//oneshot_samples[samp].buffer_header = (unsigned short*) malloc(HEADER_SIZE);

	__android_log_print(ANDROID_LOG_DEBUG, "open_external_file",
			"filepath: %s", s->file_name);

	if ((fp = fopen(s->file_name, "r")) != NULL) {
		__android_log_write(ANDROID_LOG_DEBUG, "open_external_file", "fopen()");

		//fread(oneshot_samples[samp].buffer_header, sizeof(unsigned short), HEADER_SIZE/2, fp);
		fread(s->buffer_header, 1, HEADER_SIZE, fp);
	}

	// 変数の週類はポインターである
	unsigned short* fmttype;
	unsigned long* databytes;

	fmttype = (s->buffer_header + 10);
	if (*fmttype != 0x1) {
		__android_log_write(ANDROID_LOG_DEBUG, "open_external_file", "*fmttype not PCM, loading aborted.");
		//return JNI_FALSE;
	}

	databytes = (s->buffer_header + 20);

	s->data_size = *databytes;

	// 必要な処理
	malloc_to_buffer_factor(s);

	//s->buffer_data = (unsigned short*) malloc(*databytes);

	__android_log_print(ANDROID_LOG_DEBUG, "open_external_file", "*fmttype: %x", *fmttype);
	__android_log_print(ANDROID_LOG_DEBUG, "open_external_file", "*databytes: %x", *databytes);


	fseek(fp , HEADER_SIZE , SEEK_SET);
	//fread(oneshot_samples[samp].buffer_data, sizeof(unsigned short), oneshot_samples[samp].data_size/2, fp);
	fread(s->buffer_data, 1, s->data_size, fp);

	fclose(fp);
	__android_log_write(ANDROID_LOG_DEBUG, "open_external_file", "fclose(fp);");


}

void malloc_to_buffer_factor(sample_def* s) {

	size_t buffer_over = s->data_size % BUFFER_SIZE;

	__android_log_print(ANDROID_LOG_DEBUG, "open_external_file", "s->data_size: %d", s->data_size);
	__android_log_print(ANDROID_LOG_DEBUG, "open_external_file", "buffer_over: %d", buffer_over);

	if (buffer_over > 0 && buffer_over < BUFFER_SIZE) {

		size_t buffer_rem = BUFFER_SIZE - buffer_over;

		__android_log_print(ANDROID_LOG_DEBUG, "open_external_file", "buffer_rem: %d", buffer_rem);
		s->buffer_data = (unsigned short*) malloc(s->data_size + buffer_rem );

		s->total_chunks = (s->data_size + buffer_rem) / BUFFER_SIZE;

		int i;

		for (i=s->data_size; i<(buffer_rem/2); i++)
		{
			s->buffer_data[i] =  0x0000;
		}


	} else if (buffer_over == 0) {
		s->buffer_data = (unsigned short*) malloc(s->data_size);
		s->total_chunks = (s->data_size) / BUFFER_SIZE;
	}


		__android_log_print(ANDROID_LOG_DEBUG, "open_external_file", "s->total_chunks: %d", s->total_chunks);

}



//// so these are copies of the pointer not the pointer itself, just like any other argument
//int open_asset(AAssetManager* mgr, char* filename, int samp) {
//
//
//
//	assert(NULL != mgr);
//	AAsset *asset = AAssetManager_open(mgr, filename, AASSET_MODE_BUFFER);
//
//	__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "AAssetManager_open");
//	/*
//	 size_t length = AAsset_getLength(asset);
//	 __android_log_print(ANDROID_LOG_DEBUG, "ASSET", "length: %d", (int) length);
//	 */
//
//	//char* buffer = (char*) malloc(length);
//	if (NULL == asset) {
//		__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "Asset not found, loading aborted.");
//		return JNI_FALSE;
//	}
//
////	unsigned short* buffer_header = oneshot_samples[samp].buffer_header;
//
//	// 動かない理由は全くわかねぇ
//
//	// NULL？
////	unsigned short* buffer_data = oneshot_samples[samp].buffer_data;
//
//
//	oneshot_samples[samp].buffer_header = (unsigned short*) malloc(HEADER_SIZE);
//	AAsset_read(asset, oneshot_samples[samp].buffer_header, HEADER_SIZE);
//
//	// 変数の週類はポインターである
//	unsigned short* fmttype;
//	unsigned long* databytes;
//
//	fmttype = (oneshot_samples[samp].buffer_header + 10);
//	if (*fmttype != 0x1) {
//		__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "*fmttype not PCM, loading aborted.");
//		return JNI_FALSE;
//	}
//
//	databytes = (oneshot_samples[samp].buffer_header + 20);
//	//size_t datasize_t = *databytes;
//
//	oneshot_samples[samp].data_size = *databytes;
//
//	oneshot_samples[samp].buffer_data = (unsigned short*) malloc(*databytes);
//	AAsset_seek(asset, HEADER_SIZE, SEEK_SET);
//	AAsset_read(asset, oneshot_samples[samp].buffer_data, oneshot_samples[samp].data_size);
//
//	AAsset_read(asset, oneshot_samples[samp].buffer_data, oneshot_samples[samp].data_size);
//
//
//	/*	じつは、ポインタ変数名の前にアスタリスク(*)をつけて参照すると
//	 ポインタ変数が格納しているメモリアドレスの内容を参照します
//
//	 アスタリスクをつけない後者のprintf()関数の po では、格納されているメモリアドレスを指します*/
//
//	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "fmttype: %x", fmttype);
//	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "*fmttype: %x", *fmttype);
//	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "databytes: %x", databytes);
//	__android_log_print(ANDROID_LOG_DEBUG, "ASSET", "*databytes: %x", *databytes);
//
//	AAsset_close(asset);
//
//	__android_log_write(ANDROID_LOG_DEBUG, "ASSET", "AAsset_close(asset)");
//
//	return JNI_TRUE;
//}





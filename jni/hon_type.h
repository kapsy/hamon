/*
 * hon_type.h
 *
 *  Created on: 2013/06/14
 *      Author: Michael
 */

#ifndef HON_TYPE_H_
#define HON_TYPE_H_

#include <android/log.h>

#define TRUE 1
#define FALSE 0

// APadの最低値、それいかにあまり変化がない（時間差にとって）
// #define BUFFER_SIZE 256

// Galaxy Sの最低限
//#define BUFFER_SIZE 128 // 0.72562358276643990929705215419501ms


#define BUFFER_SIZE 1024 // 5.8049886621315192743764172335601ms
//static int tics_per_part = 1600;

#define BUFFER_SIZE_SHORT (BUFFER_SIZE / 2)

//#define  LOG_TAG    "libgl2jni"
//#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
//#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
//#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
//#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,__VA_ARGS__)
#define LOGDw(...) __android_log_write(ANDROID_LOG_DEBUG,__VA_ARGS__)


#endif /* HON_TYPE_H_ */

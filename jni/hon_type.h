/*
 * hon_type.h
 *
 *  Created on: 2013/06/14
 *      Author: Michael
 */

#ifndef HON_TYPE_H_
#define HON_TYPE_H_


#define TRUE 1
#define FALSE 0

// APadの最低値、それいかにあまり変化がない（時間差にとって）
// #define BUFFER_SIZE 256

// Galaxy Sの最低限
//#define BUFFER_SIZE 128 // 0.72562358276643990929705215419501ms


#define BUFFER_SIZE 1024 // 5.8049886621315192743764172335601ms
//static int tics_per_part = 1600;

#define BUFFER_SIZE_SHORT (BUFFER_SIZE / 2)


#endif /* HON_TYPE_H_ */

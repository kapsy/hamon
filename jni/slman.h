/*
 * slman.h
 *
 *  Created on: 2013/05/31
 *      Author: Michael
 */

#ifndef SLMAN_H_
#define SLMAN_H_



void createSLEngine();

void loadAllBuffers(AAssetManager* mgr);


void initPolyphony();



int playOneShot();
void  playFFS();
//void createBufferQueueAudioPlayer();
//int openSLAsset(AAssetManager* mgr, char* filename, int buffernum);
void fadeInBqPlayer();
int playSeamless();

void shutdownAudio();
#endif /* SLMAN_H_ */

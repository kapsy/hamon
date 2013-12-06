/*
 * full_screen_quad.c
 *
 *  Created on: 2013/12/05
 *      Author: Michael
 */


#include "gfx/vertex.h"
#include "gfx/full_screen_quad.h"





struct vertex fs_quad[] = {
	{-1.0f, 	-1.0f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
	{1.0f, 		-1.0f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
	{1.0f, 		1.0f, 		0.0f, 		0.0f, 		0.0f, 		0.0f},
	{-1.0f, 	1.0f, 		0.0f, 		0.0f, 		0.0f,		0.0f},
};
unsigned short fs_quad_index[] = {
  0, 1, 3, 2
};


int sizeof_fs_quad_elements = sizeof fs_quad/sizeof fs_quad[0];
int sizeof_fs_quad = sizeof fs_quad;

int sizeof_fs_quad_index_elements = sizeof fs_quad_index/sizeof fs_quad_index[0];
int sizeof_fs_quad_index = sizeof fs_quad_index;



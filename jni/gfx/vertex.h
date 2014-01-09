// vertex.h


#ifndef VERTEX_H_
#define VERTEX_H_

#include <GLES/gl.h>

struct vertex{
    GLfloat x, y, z;
    GLfloat r, g, b;
};

struct vertex_rgb{
	float r, g, b;
};

struct vec2{
	float x;
	float y;
};

typedef struct {
	float r, g, b;
} rgb;

#endif /* VERTEX_H_ */

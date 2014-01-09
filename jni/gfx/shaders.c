// shaders.c

#include <stdio.h>
#include <stdlib.h>
#include "gfx/shaders.h"

const char v_shdr_splash[] =
		"attribute vec4 vPosition;	\n"
		"void main()				\n"
		"{							\n"
		"   gl_Position = vPosition;\n"
		"}							\n";

const char f_shdr_splash[] =
		"precision mediump float;		\n"
		"varying vec2 vtex;				\n"
		"uniform sampler2D sTexture;	\n"
		"uniform vec2 display;			\n"
		"uniform float bitmap_ratio;	\n"
		"uniform float alpha;			\n"
		"void main()					\n"
		"{								\n"
			// used to ensure the splash w/h ratio is always kept intact.
			// Only the sides of the bitmap are cropped.
			// This is not a problem as long as the bitmap is of a ratio of 2:1 (the highest w/h ratio of any Android device).
			"    gl_FragColor = texture2D(sTexture, (vec2(gl_FragCoord) + vec2(((display.y * bitmap_ratio) - display.x) / 2.0, 0.0)) / "
			"vec2(display.y * (bitmap_ratio), display.y)   ) * vec4(1.0,1.0,1.0, alpha);\n"
		"}								\n";

// 本体のアプリのシェーダー
const char v_shdr_main[] = // vShaderSrc
		"attribute vec3	aposition;	\n"
		"attribute vec3	atex;     	\n"
		"varying vec3 vtex;			\n"
		"uniform float pos_x;		\n"
		"uniform float pos_y;		\n"
		"uniform float scale;		\n"
//		"uniform 	mat4 	projectionMatrix; \n"
		"void main()				\n"
		"	{                  		\n"
		"		vtex = atex;     	\n"
		"		gl_Position = vec4((aposition.x * scale) + pos_x, (aposition.y*scale) + pos_y, 0, 1);		\n"
		"	}            			\n";

const char f_shdr_main[] = // fShaderSrc
		"precision mediump float;	\n"
		"varying vec3 vtex;			\n"
		"uniform vec3 rgb;			\n"
		"uniform float alpha;		\n"
		"void main()        		\n"
		"	{                  		\n"
		"  		gl_FragColor = vec4(rgb.x * vtex.x, rgb.y * vtex.y, rgb.z * vtex.z, alpha);	\n"
		"	}  						\n";

const char f_shdr_button[] =
		"precision mediump float;	\n"
		"varying vec3 vtex;			\n"
		"uniform vec3 rgb;			\n"
		"uniform sampler2D sTexture;\n"
		"uniform float alpha;		\n"
		"void main()				\n"
		"{							\n"
		"    gl_FragColor = texture2D(sTexture, vec2(vtex.x, vtex.y))  * vec4(rgb.x,rgb.y,rgb.z,alpha);\n"
		"}							\n";

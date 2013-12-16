/*
 * shaders.c
 *
 *  Created on: 2013/12/10
 *      Author: Michael
 */


#include <stdio.h>
#include <stdlib.h>
#include "gfx/shaders.h"

//const char* vertex_shader =
//	"attribute vec4 position;\n"
//	"attribute vec2 texcoord;\n"
//	"varying vec2 texcoordVarying;\n"
//	"void main() {\n"
//		"gl_Position = position;\n"
//		"texcoordVarying = texcoord;\n"
//	"}\n";
//
//const char* fragment_shader =
//	"precision mediump float;\n"
//	"varying vec2 texcoordVarying;\n"
//	"uniform sampler2D texture;\n"
//	"void main() {\n"
//		"gl_FragColor = texture2D(texture, texcoordVarying);\n"
//	"}\n";


// �X�v���b�V����ʂ̃V�F�[�_�[

const char v_shdr_splash[] =
		"attribute vec4 vPosition;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vPosition;\n"
		"}\n";

const char f_shdr_splash[] =
	"precision mediump float;\n"
//	"uniform vec2 uSize;\n"
		"varying		vec2  	vtex;			\n"
	"uniform sampler2D sTexture;\n"
	"uniform vec2 display;\n"
	"uniform float bitmap_ratio;\n"
	"uniform float alpha;\n"

	"void main()\n"
	"{\n"
//	"    gl_FragColor = texture2D(sTexture, vec2(gl_FragCoord) / vec2(256.0, 256.0));\n"


//		"    gl_FragColor = texture2D(sTexture, (vec2(gl_FragCoord) - vec2(60.0, 60.0)) / vec2(256.0, 256.0)    );\n"

		"    gl_FragColor = texture2D(sTexture, (vec2(gl_FragCoord) + vec2(((display.y * bitmap_ratio) - display.x) / 2.0, 0.0)) / "
		"vec2(display.y * (bitmap_ratio), display.y)   ) * vec4(1.0,1.0,1.0, alpha);\n"

	"}\n";


// �{�̂̃A�v���̃V�F�[�_�[

const char v_shdr_main[] = // vShaderSrc
		"attribute 	vec3		aposition;	\n"
		"attribute 	vec3		atex;     		\n"
		"varying   	vec3  	vtex;			\n"
		"uniform	float 		pos_x;			\n"
		"uniform	float		pos_y;			\n"
		"uniform	float 		scale;		\n"
//		"uniform 	mat4 	projectionMatrix; \n"
		"void main()							\n"
		"	{                  						\n"
		"		vtex = atex;     				\n"
		"		gl_Position = vec4((aposition.x * scale) + pos_x, (aposition.y*scale) + pos_y, 0, 1);		\n"
		"	}            \n";



const char f_shdr_main[] = // fShaderSrc
		"precision	mediump float;		\n"
		"varying		vec3  	vtex;			\n"
		"uniform 	float		ured;			\n"
		"uniform 	float		ugrn;			\n"
		"uniform 	float		ublu;			\n"
//		"uniform 	vec3		vcol			\n"
		"uniform	float 		alpha;			\n"

		"void main()        					\n"
		"	{                  						\n"

		"  		gl_FragColor = vec4(ured * vtex.x, ugrn * vtex.y, ublu * vtex.z, alpha);	\n"

		"	}   	               						\n";





const char f_shdr_button[] =
	"precision mediump float;\n"

		"varying	vec3 vtex;\n"
//		"uniform vec2 tex2;\n"

//	"uniform vec2 uSize;\n"
	"uniform sampler2D sTexture;\n"
	"uniform float alpha;\n"

	"void main()\n"
	"{\n"


//	"    gl_FragColor = texture2D(sTexture, vec2(gl_FragCoord) / vec2(256.0, 256.0));\n	"

//		"    gl_FragColor = texture2D(sTexture, vec2(gl_FragCoord) / vec2(128.0, 128.0));\n"

//		"tex2 = vec2(vtex.x, vtex.y);\n"
		"    gl_FragColor = texture2D(sTexture, vec2(vtex.x, vtex.y))  * vec4(1.0,1.0,1.0, alpha);\n"

//		"    gl_FragColor = texture2D(sTexture, tex2);\n"


//		"    gl_FragColor = texture2D(sTexture, (vec2(gl_FragCoord) + vec2(((display.y * bitmap_ratio) - display.x) / 2.0, 0.0)) / "
//		"vec2(display.y * (bitmap_ratio), display.y)   ) * vec4(1.0,1.0,1.0, alpha);\n"

	"}\n";





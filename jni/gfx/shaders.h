/*
 * shaders.h
 *
 *  Created on: 2013/12/10
 *      Author: Michael
 */

#ifndef SHADERS_H_
#define SHADERS_H_


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


// スプラッシュ画面のシェーダー
extern const char v_shdr_splash[];
extern const char f_shdr_splash[];

// 本体のアプリのシェーダー
extern const char v_shdr_main[];
extern const char f_shdr_main[];

extern const char f_shdr_button[];

#endif /* SHADERS_H_ */

/*
 * gfx_init.c
 *
 *  Created on: 2013/07/02
 *      Author: Michael
 */

// OpenGL ES 2.0 code

#include <jni.h>

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#include <unistd.h>  // sleep()を定義
#include <pthread.h>
#include <math.h>
#include <stdlib.h>


#include "gfx/vertex.h"

#include "hon_type.h"
#include "gfx/full_screen_element.h"
#include "gfx/full_screen_quad.h"


#include "gfx_gles.h"
#include "and_main.h"
#include "gfx_asst.h"
#include "gfx_butn.h"
#include "game/moods.h"

#include <time.h>
#include "gfx/frame_delta.h"

#include <pthread.h>
#include "gfx/touch_circle.h"
#include "gfx/shaders.h"

#include "gfx/tex_circle.h"



void draw_background_fse();
void draw_button();
void draw_gameplay();
void draw_touch_circles();
void draw_tex_circles();
void create_gl_buffers();
int init_shaders(GLuint *program, char const *vShSrc, char const *fShSrc);



void orthogonalMatrix(
			float left, 		float right,
			float bottom, float top,
            float near, 	float far,
            GLfloat *matrix);

void perspectiveMatrix(
			float left, 		float right,
			float bottom, float top,
			float near, 	float far,
			GLfloat *matrix);

static GLfloat projectionMatrix[16];
static GLint projectionMatrixLocation;

typedef struct {
	GLint  		aposition;
	GLint  		atex;
//	GLint  		uframe;
	GLint			pos_x;
	GLint 		pos_y;
//	GLint 		rgb[3];
	GLint 		rgb;

	GLint			alpha;
	GLint			scale;

	GLint 		display;
//	GLint			bitmap_ratio;
} shader_params_main;



typedef struct {
	GLint			position;
	GLint 		tex;

	GLint 		alpha;
	GLint			pos_x;
	GLint 		pos_y;
	GLint			scale;

	GLint 		display;
	GLint			bitmap_ratio;

	GLint 		rgb;
} shader_params_tex;


shader_params_main    g_sp_m;
screen_settings  g_sc;


// gl 浮動小数点数から画面の解像度の値へ
float gl_to_scr(float gl, int is_x) {
	float scr;
	if (is_x)
		scr = ((gl+1.0F)/2.0) * (float)g_sc.width;
	else
		scr = (float)g_sc.height - (((gl+1.0F)/2.0) * (float)g_sc.height);
	return scr;
}



GLuint sc_vbo;
GLuint sc_ibo;

GLuint fs_quad_vbo;
GLuint fs_quad_ibo;

GLuint btn_vbo;
GLuint btn_ibo;

GLuint tex_circ_vbo;
GLuint tex_circ_ibo;
shader_params_tex gles_sp_tex_circ;
GLuint gles_prog_tex_circ;



GLuint g_prog_main;
GLuint bg_cols [TOTAL_MOODS];

// テキスチャーのフィールド
shader_params_tex g_sp_t;
GLuint g_prog_splash;

shader_params_tex g_sp_btn;
GLuint g_prog_button;


unsigned int frames = 0;
//float posx = -1.0F; //wtf?
float global_scale = 1.0F;


pthread_mutex_t frame_mutex = PTHREAD_MUTEX_INITIALIZER;

EGLBoolean create_window_surface(ANativeWindow* nw) {

	LOGD("create_window_surface", "create_window_surface() called");
// ScreenSettings *sc = &g_sc;

    memset(&g_sc, 0, sizeof(screen_settings));


	LOGD("create_window_surface", "create_window_surface debug a");

//    LOGD("create_window_surface", "memset");

//	EGLint attrib[] = {
//		EGL_RED_SIZE,       8,
//		EGL_GREEN_SIZE,     8,
//		EGL_BLUE_SIZE,      8,
//		EGL_ALPHA_SIZE,     8,
//		EGL_DEPTH_SIZE,     24,
//		EGL_RENDERABLE_TYPE,
//		EGL_OPENGL_ES2_BIT,
//		EGL_NONE
//	};


	EGLint attrib[] = {
		EGL_RED_SIZE,       		EGL_DONT_CARE,
		EGL_GREEN_SIZE,     	EGL_DONT_CARE,
		EGL_BLUE_SIZE,      	EGL_DONT_CARE,
		EGL_ALPHA_SIZE,     	EGL_DONT_CARE,
		EGL_DEPTH_SIZE,     	EGL_DONT_CARE,
		EGL_RENDERABLE_TYPE,
		EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};


	LOGD("create_window_surface", "create_window_surface debug b");

	EGLint context[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
	EGLint numConfigs;
	EGLConfig config;

	g_sc.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	LOGD("create_window_surface", "create_window_surface debug c");
	if (g_sc.display == EGL_NO_DISPLAY)
		return EGL_FALSE;

	LOGD("create_window_surface", "create_window_surface debug d");

	if (!eglInitialize(g_sc.display, &g_sc.majorVersion, &g_sc.minorVersion))
		return EGL_FALSE;
	LOGD("create_window_surface", "create_window_surface debug e");

	if (!eglChooseConfig(g_sc.display, attrib, &config, 1, &numConfigs))
		return EGL_FALSE;
	LOGD("create_window_surface", "create_window_surface debug f");


	EGLint format;
	eglGetConfigAttrib(g_sc.display, config, EGL_NATIVE_VISUAL_ID, &format);

	LOGD("create_window_surface", "create_window_surface debug g");
	ANativeWindow_setBuffersGeometry(nw, 0, 0, format);

	LOGD("create_window_surface", "create_window_surface debug h");

	g_sc.surface = eglCreateWindowSurface(g_sc.display, config, nw, NULL);

	LOGD("create_window_surface", "create_window_surface debug i");
	if (g_sc.surface == EGL_NO_SURFACE)
		return EGL_FALSE;

	LOGD("create_window_surface", "create_window_surface debug j");

	g_sc.context = eglCreateContext(g_sc.display, config, EGL_NO_CONTEXT, context);

	LOGD("create_window_surface", "create_window_surface debug k");
	if (g_sc.context == EGL_NO_CONTEXT)
		return EGL_FALSE;


	LOGD("create_window_surface", "create_window_surface debug l");

	if (!eglMakeCurrent(g_sc.display, g_sc.surface, g_sc.surface, g_sc.context))
		return EGL_FALSE;


	LOGD("create_window_surface", "create_window_surface debug m");

	EGLint w, h;
	eglQuerySurface(g_sc.display, g_sc.surface, EGL_WIDTH, &w);
	eglQuerySurface(g_sc.display, g_sc.surface, EGL_HEIGHT, &h);


	LOGD("create_window_surface", "create_window_surface debug n");

//	g_sc.width = w;
//	g_sc.height = h;
	g_sc.width = (float)w;
	g_sc.height = (float)h;

  	LOGD("create_window_surface", "g_sc.width: %d", g_sc.width);
  	LOGD("create_window_surface", "g_sc.height: %d", g_sc.height);

  	g_sc.hw_ratio = (float)h / (float)w;

	LOGD("create_window_surface", "create_window_surface debug o");

  	return EGL_TRUE;
}





int gles_init() {

	LOGD("gles_init", "init_cmds() called");

	LOGD("gles_init", "GL_VENDOR: %s", glGetString(GL_VENDOR));
	LOGD("gles_init", "GL_RENDERER: %s", glGetString(GL_RENDERER));
	LOGD("gles_init", "GL_VERSION: %s", glGetString(GL_VERSION));

	frames = 0;
	frame_delta_avg_init();

//	init_touch_circles();
//	calc_circle_vertex();

	calc_tex_circle_vertex();
	init_tex_circles();



//	calc_button_coords();

	calc_btn_quad_verts(BTN_W, BTN_W);

	int res;

	res = init_shaders(&g_prog_splash, v_shdr_splash, f_shdr_splash);
	if (!res)	return 0;


	res = init_shaders(&gles_prog_tex_circ, v_shdr_main, f_shdr_button);
	if (!res)	return 0;


//	res = init_shaders(&g_prog_button, v_shdr_splash, f_shdr_button);
//	if (!res)	return 0;
	res = init_shaders(&g_prog_button, v_shdr_main, f_shdr_button);
	if (!res)	return 0;

	res = init_shaders(&g_prog_main, v_shdr_main, f_shdr_main);
	if (!res) return 0;

	create_gl_buffers();

	g_sp_m.aposition = glGetAttribLocation(g_prog_main, "aposition");
	g_sp_m.atex = glGetAttribLocation(g_prog_main, "atex");
//	g_sp_m.uframe = glGetUniformLocation(g_prog_main, "uframe");
	g_sp_m.pos_x = glGetUniformLocation(g_prog_main, "pos_x");
	g_sp_m.pos_y = glGetUniformLocation(g_prog_main, "pos_y");
	g_sp_m.rgb = glGetUniformLocation(g_prog_main, "rgb");
//	g_sp_m.rgb[0] = glGetUniformLocation(g_prog_main, "ured");
//	g_sp_m.rgb[1]  = glGetUniformLocation(g_prog_main, "ugrn");
//	g_sp_m.rgb[2]  = glGetUniformLocation(g_prog_main, "ublu");
	g_sp_m.alpha = glGetUniformLocation(g_prog_main, "alpha");
	g_sp_m.scale = glGetUniformLocation(g_prog_main, "scale");



	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

//	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glViewport(0, 0, g_sc.width, g_sc.height);

	/* 平行投影変換行列を求める */
	orthogonalMatrix(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, projectionMatrix);

	/* uniform 変数 projectionMatrix の場所を得る */
	projectionMatrixLocation = glGetUniformLocation(g_prog_main, "projectionMatrix");



	// gles2_py_texture からの関数


	g_sp_t.display = glGetUniformLocation(g_prog_splash, "display");
	g_sp_t.bitmap_ratio = glGetUniformLocation(g_prog_splash, "bitmap_ratio");
	g_sp_t.alpha = glGetUniformLocation(g_prog_splash, "alpha");
	g_sp_t.position = glGetAttribLocation(g_prog_splash, "vPosition");
//	g_sp_t.tex = glGetAttribLocation(g_prog_splash, "aTex");
	g_sp_btn.pos_x = glGetUniformLocation(g_prog_button, "pos_x");
	g_sp_btn.pos_y = glGetUniformLocation(g_prog_button, "pos_y");
	g_sp_btn.tex = glGetAttribLocation(g_prog_button, "atex");
	g_sp_btn.alpha = glGetUniformLocation(g_prog_button, "alpha");
	g_sp_btn.position = glGetAttribLocation(g_prog_button, "aposition");
	g_sp_btn.scale = glGetUniformLocation(g_prog_button, "scale");
	g_sp_btn.rgb = glGetUniformLocation(g_prog_button, "rgb");


//	gles_sp_tex_circ

	gles_sp_tex_circ.display = glGetUniformLocation(g_prog_splash, "display");
	gles_sp_tex_circ.bitmap_ratio = glGetUniformLocation(g_prog_splash, "bitmap_ratio");
	gles_sp_tex_circ.alpha = glGetUniformLocation(g_prog_splash, "alpha");
	gles_sp_tex_circ.position = glGetAttribLocation(g_prog_splash, "vPosition");
//	g_sp_t.tex = glGetAttribLocation(g_prog_splash, "aTex");
	gles_sp_tex_circ.pos_x = glGetUniformLocation(g_prog_button, "pos_x");
	gles_sp_tex_circ.pos_y = glGetUniformLocation(g_prog_button, "pos_y");
	gles_sp_tex_circ.tex = glGetAttribLocation(g_prog_button, "atex");
	gles_sp_tex_circ.alpha = glGetUniformLocation(g_prog_button, "alpha");
	gles_sp_tex_circ.position = glGetAttribLocation(g_prog_button, "aposition");
	gles_sp_tex_circ.scale = glGetUniformLocation(g_prog_button, "scale");
	gles_sp_tex_circ.rgb = glGetUniformLocation(g_prog_button, "rgb");




	if(!sles_init_called) {
		int i;


//		アドレスの内容を参照 . 代入
//		*ary_p[n] = var;

		for (i=0; i<sizeof_textures_elements; i++) {
			struct texture_file *tf = textures + i;
			setup_texture(tf, 0.0F);
		}

//		LOGD("gles_init", "sizeof textures: %d", sizeof textures);
//		LOGD("gles_init", "sizeof textures[0]: %d", sizeof textures[0]);
	}




//	init_tex_circles();

//	create_gl_tex_circles();




	LOGD("init_cmds", "init_cmds() finished");
	return TRUE;


}



//// and_main.c からに取ったコード
///**
// * Tear down the EGL context currently associated with the display.
// */
//void gles_term_display(ScreenSettings* e) {
//    if (e->display != EGL_NO_DISPLAY) {
//        eglMakeCurrent(e->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//        if (e->context != EGL_NO_CONTEXT) {
//            eglDestroyContext(e->display, e->context);
//        }
//        if (e->surface != EGL_NO_SURFACE) {
//            eglDestroySurface(e->display, e->surface);
//        }
//        eglTerminate(e->display);
//    }
//    e->animating = 0;
//    e->display = EGL_NO_DISPLAY;
//    e->context = EGL_NO_CONTEXT;
//    e->surface = EGL_NO_SURFACE;
//}



// and_main.c からに取ったコード
/**
 * Tear down the EGL context currently associated with the display.
 */
void gles_term_display(screen_settings* e) {

	LOGD("gles_term_display", "gles_term_display() called");

    if (e->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(e->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (e->context != EGL_NO_CONTEXT) {
            eglDestroyContext(e->display, e->context);
        }
        if (e->surface != EGL_NO_SURFACE) {
            eglDestroySurface(e->display, e->surface);
        }
        eglTerminate(e->display);
    }
//    e->animating = 0;
    e->display = EGL_NO_DISPLAY;
    e->context = EGL_NO_CONTEXT;
    e->surface = EGL_NO_SURFACE;
}


GLuint load_shader(GLenum type, const char *shaderSource)
{
	LOGD("load_shader", "load_shader() called");
	GLuint shader;
	GLint compiled;

	shader = glCreateShader(type);
	if (shader == 0) return 0;

	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) { // compile error
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 0) {
			char* infoLog = malloc(sizeof(char) * infoLen);
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			LOGD("load_shader", "Error compiling shader:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}



int init_shaders(GLuint *program, char const *vShSrc, char const *fShSrc)
{
	LOGD("init_shaders", "init_shaders() called");
	GLuint vShader;
	GLuint fShader;
	GLint  linked;
	GLuint prog;

	vShader = load_shader(GL_VERTEX_SHADER, vShSrc);
	fShader = load_shader(GL_FRAGMENT_SHADER, fShSrc);

	prog = glCreateProgram();
	if (prog == 0) return 0;

	glAttachShader(prog, vShader);
	glAttachShader(prog, fShader);
	glLinkProgram(prog);
	glGetProgramiv (prog, GL_LINK_STATUS, &linked);
	if (!linked) { // error
		GLint infoLen = 0;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 0) {
			char* infoLog = malloc(sizeof(char) * infoLen);
			glGetProgramInfoLog(prog, infoLen, NULL, infoLog);
			LOGD("init_shaders", "Error linking program:\n%s\n", infoLog);
			free ( infoLog );
		}
		glDeleteProgram (prog);
		return GL_FALSE;
	}
	glDeleteShader(vShader);
	glDeleteShader(fShader);

	*program = prog;
	return GL_TRUE;
}



// GPU上のバッファオブジェクトにデータを転送
void create_gl_buffers()
{


	// VBOの生成
	glGenBuffers(1, &tex_circ_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, tex_circ_vbo);
	// データの転送
	glBufferData(GL_ARRAY_BUFFER, sizeof_tex_circle_v, tex_circle_v, GL_STATIC_DRAW);

	// インデックスバッファの作成
	glGenBuffers(1, &tex_circ_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tex_circ_ibo);
	// データの転送
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_tex_circle_i, tex_circle_i, GL_STATIC_DRAW);


	// VBOの生成
	glGenBuffers(1, &btn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, btn_vbo);
	// データの転送
	glBufferData(GL_ARRAY_BUFFER, sizeof_btn_quad, btn_quad, GL_STATIC_DRAW);

	// インデックスバッファの作成
	glGenBuffers(1, &btn_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, btn_ibo);
	// データの転送
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_btn_quad_index, btn_quad_index, GL_STATIC_DRAW);


	// VBOの生成
	glGenBuffers(1, &sc_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, sc_vbo);
	// データの転送
	glBufferData(GL_ARRAY_BUFFER, sizeof_solid_circle_v, solid_circle_v, GL_STATIC_DRAW);

	// インデックスバッファの作成
	glGenBuffers(1, &sc_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sc_ibo);
	// データの転送
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_solid_circle_i, solid_circle_i, GL_STATIC_DRAW);



	// VBOの生成
	glGenBuffers(1, &fs_quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, fs_quad_vbo);
	// データの転送
//	glBufferData(GL_ARRAY_BUFFER, sizeof(fs_quad), fs_quad, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof_fs_quad, fs_quad, GL_STATIC_DRAW);

	// インデックスバッファの作成
	glGenBuffers(1, &fs_quad_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fs_quad_ibo);
	// データの転送
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fs_quad_index), fs_quad_index, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_fs_quad_index, fs_quad_index, GL_STATIC_DRAW);



	int i;
	for (i = 0; i < TOTAL_MOODS; i++) {
		glGenBuffers(1, &bg_cols[i]);
		glBindBuffer(GL_ARRAY_BUFFER, bg_cols[i]);
//		glBufferData(GL_ARRAY_BUFFER, (sizeof(quad_colors))/4, quad_colors[i], GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, sizeof_mood_colors_set,  (moods +i)->rgb_bg, GL_STATIC_DRAW);
	}


	LOGD("create_gl_buffers", "pi_create_buffer() finished");
}

int create_gl_texture(struct texture_type *tt, GLint param)
{
  glGenTextures(1, &tt->texname);
  glBindTexture(GL_TEXTURE_2D, tt->texname);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);


  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tt->BmpWidth, tt->BmpHeight, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, tt->TexData);
  return 1;
}



void draw_frame() {

//	glViewport(0, 0, g_sc.width, g_sc.height);
	glClear(GL_COLOR_BUFFER_BIT);


//	if (screens[0].is_showing) {
//
//		if (elapsed_time > DELAY_BEFORE_SPLASH_BG)
//
//
//
//			full_scr_anim(screens + 1);
//			full_scr_alpha_mod(screens + 1);
//			draw_full_screen_image(screens + 1);
//
//		if (elapsed_time > DELAY_BEFORE_SPLASH)
//
//			full_scr_anim(screens + 0);
//			draw_full_screen_image(screens + 0);
//	}



	// +++++++++++++++++++++++++


//	if(screens[0].is_showing) {
//
//		if(splash_fading_in) {
//			full_scr_anim(screens + 1);
//			full_scr_alpha_mod(screens + 1);
//			draw_full_screen_image(screens + 1);
//		}
//		if(splash_bg_fading_in) {
//			full_scr_anim(screens + 0);
//			draw_full_screen_image(screens + 0);
//
//		}
//
//	}


	if (screens[1].is_showing) {
		full_scr_anim(screens + 1);
		full_scr_alpha_mod(screens + 1);
		draw_full_screen_image(screens + 1);
	}
	if (screens[0].is_showing) {
		full_scr_anim(screens + 0);
		draw_full_screen_image(screens + 0);
	}






	if(show_gameplay) {
//		if (!screens[1].is_showing)
			draw_background_fse();
		draw_gameplay();

//		 if (!screens[0].fading_out)
		if (buttons_activated)
			 draw_button();
	}

	if(screens[2].is_showing) {
		full_scr_anim(screens + 2);
		draw_full_screen_image(screens + 2);
	}

	eglSwapBuffers(g_sc.display, g_sc.surface);

}


void draw_gameplay() {
//	draw_touch_circles();
	draw_tex_circles();

//		usleep(100000);
//		usleep(20000);
}





void draw_full_screen_image(struct full_scr_el* fs) {


	glUseProgram(g_prog_splash);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fs->main_texture->tt.texname);

	glBindBuffer(GL_ARRAY_BUFFER, fs_quad_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fs_quad_ibo);

	glEnableVertexAttribArray(g_sp_t.position);
	glVertexAttribPointer(g_sp_t.position, 3, GL_FLOAT, GL_FALSE, 24, (void*)0);
	glEnableVertexAttribArray(0);
	glUniform2f(g_sp_t.display, g_sc.width, g_sc.height);
	glUniform1f(g_sp_t.bitmap_ratio, fs->main_texture->tt.bitmap_ratio);
//	glUniform3f(g_sp_t.rgb, 1.0f, 1.0f, 1.0f);

	glUniform1f(g_sp_t.alpha, fs->alpha_mod);

	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);


//	glBindTexture(GL_TEXTURE_2D,0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
}



void draw_button() {

	glUseProgram(g_prog_button); // res = init_shaders(&g_prog_button, v_shdr_main, f_shdr_button);
	glBindBuffer(GL_ARRAY_BUFFER, btn_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, btn_ibo);
	glEnableVertexAttribArray(g_sp_btn.position);
	glEnableVertexAttribArray(g_sp_btn.tex);
	glVertexAttribPointer(g_sp_btn.position, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) 0);
	glVertexAttribPointer(g_sp_btn.tex, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) (sizeof(GL_FLOAT) * 3));
	glEnableVertexAttribArray(0);


	int i;
	for (i = 0; i < sizeof_button_array; i++) {

		struct button* b = buttons + i;
		glUniform3f(g_sp_btn.rgb, b->rgb->r, b->rgb->g, b->rgb->b);

		btn_anim(b);

		glUniform1f(g_sp_btn.pos_x, b->gl_x);
		glUniform1f(g_sp_btn.pos_y, b->gl_y);
		glUniform1f(g_sp_btn.scale, b->scale);

		if (b->alpha_pt > 0.0) {
			glUniform1f(g_sp_btn.alpha, b->alpha_pt);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, b->pressed_texture->tt.texname);
			glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
		}

		glUniform1f(g_sp_btn.alpha, b->alpha);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, b->main_texture->tt.texname);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}



void draw_background_fse() {

	glUniform1f(g_sp_m.scale, global_scale);
	glUniform1f(g_sp_m.alpha, 0.8);
//	glUniform1f(g_sp_m.rgb[0], 1.0);
//	glUniform1f(g_sp_m.rgb[1], 1.0);
//	glUniform1f(g_sp_m.rgb[2], 1.0);

	glUseProgram(g_prog_main);

	glBindBuffer(GL_ARRAY_BUFFER, fs_quad_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fs_quad_ibo);
	glEnableVertexAttribArray(g_sp_m.aposition);
	glVertexAttribPointer(g_sp_m.aposition, 3, GL_FLOAT, GL_FALSE,	sizeof(GL_FLOAT) * 6, (void*) 0);

	int i;
	for (i = 0; i < sizeof_backgrounds_elements; i++) {

		struct full_scr_el* fs = backgrounds + i;
		if(fs->is_showing) {
			full_scr_mod(fs);
			full_scr_alpha_anim(fs);

//			glBindBuffer(GL_ARRAY_BUFFER, bg_cols[(moods + i)->color_index]);
			glBindBuffer(GL_ARRAY_BUFFER, bg_cols[(backgrounds + i)->color_index]);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fs_quad_ibo);
			glEnableVertexAttribArray(g_sp_m.atex);
			glVertexAttribPointer(g_sp_m.atex, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*) 0);

			glUniform1f(g_sp_m.pos_x, 0.0);
			glUniform1f(g_sp_m.pos_y, 0.0);


			glUniform3f(g_sp_m.rgb, 1.0f, 1.0f, 1.0f);


//			glUniform1f(g_sp_m.rgb[0], 0.5 * fs->pulse);
//			glUniform1f(g_sp_m.rgb[1], 1.0);
//			glUniform1f(g_sp_m.rgb[2], 1.0);

			glUniform1f(g_sp_m.alpha, fs->alpha);
			glUniform1f(g_sp_m.scale, 1.0);

			glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
		}

	}
}


void draw_tex_circles() {


	glUseProgram(gles_prog_tex_circ);
	glBindBuffer(GL_ARRAY_BUFFER, tex_circ_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tex_circ_ibo);
	glEnableVertexAttribArray(gles_sp_tex_circ.position);
	glEnableVertexAttribArray(gles_sp_tex_circ.tex);

	// 頂点情報のサイズ、オフセットを指定
	glVertexAttribPointer(gles_sp_tex_circ.position, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) 0);
	glVertexAttribPointer(gles_sp_tex_circ.tex, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) (sizeof(GL_FLOAT) * 3));

	glEnableVertexAttribArray(0);
//		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);

	pthread_mutex_lock(&frame_mutex);
	int i;
	for (i=0; i<sizeof_tex_circles_e; i++) {

		struct tex_circle* ts = tex_ripples + (tex_circle_draw_order[i]);
		if (ts->is_alive) {
			glUniform1f(gles_sp_tex_circ.pos_x, ts->pos_x);
			glUniform1f(gles_sp_tex_circ.pos_y, ts->pos_y);
			glUniform3f(gles_sp_tex_circ.rgb, ts->rgb->r, ts->rgb->g, ts->rgb->b);

			tex_ripple_alpha_size(ts);
			glUniform1f(gles_sp_tex_circ.alpha, ts->alpha);
			glUniform1f(gles_sp_tex_circ.scale, ts->scale);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ts->tex->tt.texname);
			glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
		}
	}

	for (i = 0; i < sizeof_tex_circles_e; i++) {

		struct tex_circle* ts = tex_circles + (tex_circle_draw_order[i]);
		if (ts->is_alive) {
			glUniform1f(gles_sp_tex_circ.pos_x, ts->pos_x);
			glUniform1f(gles_sp_tex_circ.pos_y, ts->pos_y);
			glUniform3f(gles_sp_tex_circ.rgb, ts->rgb->r, ts->rgb->g, ts->rgb->b);

			tex_circle_alpha_size(ts);
			glUniform1f(gles_sp_tex_circ.alpha, ts->alpha);
			glUniform1f(gles_sp_tex_circ.scale, ts->scale);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ts->tex->tt.texname);
			glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	pthread_mutex_unlock(&frame_mutex);
}








void draw_touch_circles() {



	glUseProgram(g_prog_main);


	glBindBuffer(GL_ARRAY_BUFFER, sc_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sc_ibo);
	glEnableVertexAttribArray(g_sp_m.aposition);
	glEnableVertexAttribArray(g_sp_m.atex);

	// 頂点情報のサイズ、オフセットを指定
	glVertexAttribPointer(g_sp_m.aposition, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) 0);
	glVertexAttribPointer(g_sp_m.atex, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) (sizeof(GL_FLOAT) * 3));

	glEnableVertexAttribArray(0);
//		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);


	pthread_mutex_lock(&frame_mutex);
	int i;
	for (i=0; i<sizeof_t_circles_e; i++) {

		struct touch_circle* ts = t_ripples + (t_circle_draw_order[i]);
		if (ts->is_alive) {
			glUniform1f(g_sp_m.pos_x, ts->pos_x);
			glUniform1f(g_sp_m.pos_y, ts->pos_y);
			glUniform3f(g_sp_m.rgb, 1.0f, 1.0f, 1.0f);
			t_ripple_alpha_size(ts);
			glUniform1f(g_sp_m.alpha, ts->alpha);
			glUniform1f(g_sp_m.scale, ts->scale);
			glDrawElements(GL_TRIANGLE_FAN, CIRCLE_SEGMENTS + 2, GL_UNSIGNED_SHORT, 0);
		}
	}

	for (i=0; i<sizeof_t_circles_e; i++) {

		struct touch_circle* ts = t_circles + (t_circle_draw_order[i]);
		if (ts->is_alive) {
			glUniform1f(g_sp_m.pos_x, ts->pos_x);
			glUniform1f(g_sp_m.pos_y, ts->pos_y);
			glUniform3f(g_sp_m.rgb, 1.0f, 1.0f, 1.0f);
			t_circle_alpha_size(ts);
			glUniform1f(g_sp_m.alpha, ts->alpha);
			glUniform1f(g_sp_m.scale, ts->scale);
			glDrawElements(GL_TRIANGLE_FAN, CIRCLE_SEGMENTS + 2, GL_UNSIGNED_SHORT, 0);
		}
	}
	pthread_mutex_unlock(&frame_mutex);
}





//GLfloat matrix[] = {
//  m0,  m4,  m8,  m12,
//  m1,  m5,  m9,  m13,
//  m2,  m6,  m10, m14,
//  m3,  m7,  m11, m15,
//};
///*
//** 平行投影変換行列を求める
//*/
//void orthogonalMatrix(
//			float left, 			float right,
//			float bottom, 	float top,
//			float near, 		float far,
//			GLfloat *matrix)
//{
//
//
//  /* この部分を考えましょう */
//}

/*
** 平行投影変換行列を求める
*/
void orthogonalMatrix(
			float left, 		float right,
			float bottom, float top,
            float near, 	float far,
            GLfloat *matrix)
{
  float dx = right - left;
  float dy = top - bottom;
  float dz = far - near;

  matrix[ 0] =  2.0f / dx;
  matrix[ 5] =  2.0f / dy;
  matrix[10] = -2.0f / dz;
  matrix[12] = -(right + left) / dx;
  matrix[13] = -(top + bottom) / dy;
  matrix[14] = -(far + near) / dz;
  matrix[15] =  1.0f;
  matrix[ 1] = matrix[ 2] = matrix[ 3] = matrix[ 4] =
  matrix[ 6] = matrix[ 7] = matrix[ 8] = matrix[ 9] = matrix[11] = 0.0f;
}

/*
** 透視投影変換行列を求める
*/
void perspectiveMatrix(
			float left, 		float right,
			float bottom, float top,
			float near, 	float far,
			GLfloat *matrix)
{
  float dx = right - left;
  float dy = top - bottom;
  float dz = far - near;

  matrix[ 0] =  2.0f * near / dx;
  matrix[ 5] =  2.0f * near / dy;
  matrix[ 8] =  (right + left) / dx;
  matrix[ 9] =  (top + bottom) / dy;
  matrix[10] = -(far + near) / dz;
  matrix[11] = -1.0f;
  matrix[14] = -2.0f * far * near / dz;
  matrix[ 1] = matrix[ 2] = matrix[ 3] = matrix[ 4] =
  matrix[ 6] = matrix[ 7] = matrix[12] = matrix[13] = matrix[15] = 0.0f;
}

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
#include "hon_type.h"


#include "and_main.h"
#include "gfx_gles.h"


#include <unistd.h>  // sleep()を定義
#include <pthread.h>

#define DELTA_AVG_COUNT 6

#define TOUCH_SHAPES_MAX 40
#define TOUCH_SHAPES_TTL 200.0F


typedef struct {

	int is_alive;
	GLfloat pos_x;
	GLfloat pos_y;
GLfloat alpha;
	float ttl;


} touch_shape;

//typedef struct {
//
//} rgba;

touch_shape touch_shapes[TOUCH_SHAPES_MAX];
unsigned int touch_shape_draw_order[TOUCH_SHAPES_MAX];
size_t current_touch_shape = 0;


void init_touch_shapes();
//touch_shape* cycle_touch_shapes();
void draw_touch_shapes();
void step_touch_shape_draw_order();

typedef struct {
	EGLNativeWindowType nativeWin;
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
	EGLint majorVersion;
	EGLint minorVersion;
	int width;
	int height;
} ScreenSettings;


void pi_create_buffer();

const char vShaderSrc[] =
		"attribute 	vec3		aPosition;	\n"
		"attribute 	vec2		aTex;     	\n"
		"varying   	vec2  	vTex;			\n"
		"uniform	float 		uFrame;		\n"
		"uniform	float 		posX;			\n"
		"uniform	float		posY;			\n"
		"void main()							\n"
		"	{                  						\n"
		"		vTex = aTex;     				\n"
//		"		vTex = vec2(0.0, 0.0);		\n"
//		"		gl_Position = vec4(aPosition.x - uFrame, aPosition.y + uFrame, 0, 1);		\n"
		"		gl_Position = vec4(aPosition.x + posX, aPosition.y + posY, 0, 1);		\n"
		"	}                  						\n";

const char fShaderSrc[] =
		"precision	mediump float;		\n"
		"varying		vec2  	vTex;			\n"
		"uniform 	float 		uBlue;		\n"
		"uniform	float 		alpha;			\n"
		"void main()        					\n"
		"	{                  						\n"
		"  		gl_FragColor = vec4(vTex.y, vTex.x, uBlue, alpha);	\n"
		"	}   	               						\n";


//const char fShaderSrcBlue[] =
//  "precision mediump float;\n"\
//  "varying   vec2  vTex;\n"
//  "void main()        \n"
//  "{                  \n"
//  "  gl_FragColor = vec4(vTex.y, 0.0, 1.0, 0.8);\n"
//  "}                  \n";



typedef struct {
  GLint  		aPosition;
  GLint  		aTex;
  GLint  		uFrame;
  GLint		posX;
  GLint 		posY;
  GLint 		uBlue;
  GLint		alpha;
} ShaderParams;

typedef struct {
    GLfloat x, y, z;
    GLfloat u, v;
} VertexType;

VertexType vObj[] = { // VertexBufferObjectを使用スべきか？

  {.x = -0.5f, 	.y = -0.5f, 	.z = 0.0f, 	.u = 0.0f, 	.v = 1.0f},
  {.x =  0.5f, 	.y = -0.5f, 	.z = 0.0f, 	.u = 1.0f, 	.v = 1.0f},
  {.x =  0.0f, 	.y =  0.5f, 	.z = 0.0f, 	.u = 0.5f, 	.v = 0.0f},
};




unsigned short iObj[] = {
  0, 1, 2
};


		// 四角形
//VertexType vObj_sq[] = { // VertexBufferObjectを使用スべきか？
//
//  {.x = -0.25f, 	.y =   0.5f, 	.z = 0.0f, 	.u = 0.0f, 	.v = 1.0f},
//  {.x = 0.25f, 		.y =   0.5f, 	.z = 0.0f, 	.u = 1.0f, 	.v = 1.0f},
//  {.x = 0.25f, 		.y = -0.5f, 	.z = 0.0f, 	.u = 0.5f, 	.v = 0.0f},
//  {.x = -0.25f, 	.y = -0.5f, 	.z = 0.0f, 	.u = 0.5f, 	.v = 0.0f},
//};
VertexType vObj_sq[] = { // VertexBufferObjectを使用スべきか？

  {.x = -0.125f, 	.y =   0.25f, 	.z = 0.0f, 	.u = 0.0f, 	.v = 1.0f},
  {.x = 0.125f, 		.y =   0.25f, 	.z = 0.0f, 	.u = 1.0f, 	.v = 1.0f},
  {.x = 0.125f, 		.y = -0.25f, 	.z = 0.0f, 	.u = 0.5f, 	.v = 0.0f},
  {.x = -0.125f, 	.y = -0.25f, 	.z = 0.0f, 	.u = 0.5f, 	.v = 0.0f},
};
unsigned short iObj_sq[] = {
//  0, 1, 2, 3
//  0, 1, 3, 2
  0, 3, 1, 2
//  0, 2, 3
};



ShaderParams    g_sp;
ScreenSettings  g_sc;

GLuint g_vbo;
GLuint g_ibo;

GLuint g_vbo_2;
GLuint g_ibo_2;

GLuint g_program;
GLuint g_program_purp;

unsigned int frames = 0;
    float posx = -1.0F;

struct timezone tzp;
struct timeval get_time;

unsigned long curr_time = 0;
unsigned long new_time = 0;

unsigned long frame_delta = 0;
int frame_delta_avg[DELTA_AVG_COUNT];
size_t frame_delta_cycle = 0;




EGLBoolean pi_SurfaceCreate(ANativeWindow* nw) {

	LOGD("pi_SurfaceCreate", "pi_SurfaceCreate() called");
// ScreenSettings *sc = &g_sc;

    memset(&g_sc, 0, sizeof(ScreenSettings));
//    LOGD("pi_SurfaceCreate", "memset");

	EGLint attrib[] = {
		EGL_RED_SIZE,       8,
		EGL_GREEN_SIZE,     8,
		EGL_BLUE_SIZE,      8,
		EGL_ALPHA_SIZE,     8,
		EGL_DEPTH_SIZE,     24,
		EGL_RENDERABLE_TYPE,
		EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	EGLint context[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
	EGLint numConfigs;
	EGLConfig config;

	g_sc.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (g_sc.display == EGL_NO_DISPLAY)
		return EGL_FALSE;

	if (!eglInitialize(g_sc.display, &g_sc.majorVersion, &g_sc.minorVersion))
		return EGL_FALSE;

	if (!eglChooseConfig(g_sc.display, attrib, &config, 1, &numConfigs))
		return EGL_FALSE;

	EGLint format;
	eglGetConfigAttrib(g_sc.display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(nw, 0, 0, format);

	g_sc.surface = eglCreateWindowSurface(g_sc.display, config, nw, NULL);
	if (g_sc.surface == EGL_NO_SURFACE)
		return EGL_FALSE;

	g_sc.context = eglCreateContext(g_sc.display, config, EGL_NO_CONTEXT, context);
	if (g_sc.context == EGL_NO_CONTEXT)
		return EGL_FALSE;

	if (!eglMakeCurrent(g_sc.display, g_sc.surface, g_sc.surface, g_sc.context))
		return EGL_FALSE;

	EGLint w, h;
	eglQuerySurface(g_sc.display, g_sc.surface, EGL_WIDTH, &w);
	eglQuerySurface(g_sc.display, g_sc.surface, EGL_HEIGHT, &h);

	g_sc.width = w;
	g_sc.height = h;

  	LOGD("pi_SurfaceCreate", "g_sc.width: %d", g_sc.width);
  	LOGD("pi_SurfaceCreate", "g_sc.height: %d", g_sc.height);

  	return EGL_TRUE;
}





int init_cmds() { // FIXME

	LOGD("init_cmds", "init_cmds() called");

	LOGD("init_cmds", "GL_VENDOR: %s", glGetString(GL_VENDOR));
	LOGD("init_cmds", "GL_RENDERER: %s", glGetString(GL_RENDERER));
	LOGD("init_cmds", "GL_VERSION: %s", glGetString(GL_VERSION));

	frames = 0;
	frame_delta_avg_init();

	init_touch_shapes();

	int res;

	res = InitShaders(&g_program, vShaderSrc, fShaderSrc);
	if (!res)
		return 0;




	pi_create_buffer();

	g_sp.aPosition = glGetAttribLocation(g_program, "aPosition");
	g_sp.aTex = glGetAttribLocation(g_program, "aTex");
	g_sp.uFrame = glGetUniformLocation(g_program, "uFrame");

	g_sp.posX = glGetUniformLocation(g_program, "posX");
	g_sp.posY = glGetUniformLocation(g_program, "posY");



	g_sp.uBlue = glGetUniformLocation(g_program, "uBlue");
	g_sp.alpha = glGetUniformLocation(g_program, "alpha");



	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.3f, 0.0f, 0.5f);

	glViewport(0, 0, g_sc.width, g_sc.height);

	LOGD("init_cmds", "init_cmds() finished");
	return TRUE;
}


GLuint LoadShader(GLenum type, const char *shaderSource)
{
	LOGD("LoadShader", "LoadShader() called");
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
      LOGD("LoadShader", "Error compiling shader:\n%s\n", infoLog);
      free(infoLog);
    }
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}




int InitShaders(GLuint *program, char const *vShSrc, char const *fShSrc)
{
	LOGD("InitShaders", "InitShaders() called");
  GLuint vShader;
  GLuint fShader;
  GLint  linked;
  GLuint prog;

  vShader = LoadShader(GL_VERTEX_SHADER, vShSrc);
  fShader = LoadShader(GL_FRAGMENT_SHADER, fShSrc);

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
      LOGD("InitShaders", "Error linking program:\n%s\n", infoLog);
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
void pi_create_buffer()
{
  // VBOの生成
  glGenBuffers(1, &g_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
  // データの転送
  glBufferData(GL_ARRAY_BUFFER, sizeof(vObj), vObj, GL_STATIC_DRAW);

  // インデックスバッファの作成
  glGenBuffers(1, &g_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
  // データの転送
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iObj), iObj, GL_STATIC_DRAW);



  // VBOの生成
  glGenBuffers(1, &g_vbo_2);
  glBindBuffer(GL_ARRAY_BUFFER, g_vbo_2);
  // データの転送
  glBufferData(GL_ARRAY_BUFFER, sizeof(vObj_sq), vObj_sq, GL_STATIC_DRAW);

  // インデックスバッファの作成
  glGenBuffers(1, &g_ibo_2);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo_2);
  // データの転送
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iObj_sq), iObj_sq, GL_STATIC_DRAW);














	LOGD("pi_create_buffer", "pi_create_buffer() finished");
}

//float delta_average(float dt, int count) {
//
//}


void get_time_long(unsigned long* t) {


	gettimeofday(&get_time, &tzp);

	*t = (get_time.tv_sec * SEC_IN_US) + get_time.tv_usec;

}


void frame_delta_avg_init() {

	int i;
	for (i = 0; i < DELTA_AVG_COUNT; i++) {
		frame_delta_avg[i] = 16000;
	}
}

void frame_delta_avg_calc() {

	if (frame_delta_cycle < DELTA_AVG_COUNT)
		frame_delta_cycle++;

	if (frame_delta_cycle == DELTA_AVG_COUNT)
		frame_delta_cycle = 0;

	frame_delta_avg[frame_delta_cycle] = frame_delta;

	int i;

	unsigned long d_tot;

	for (i = 0; i < DELTA_AVG_COUNT; i++) {
		d_tot += frame_delta_avg[i];

	}


	frame_delta = d_tot / DELTA_AVG_COUNT;

}


void calc_frame_delta_time() {

	if (curr_time <= 0) get_time_long(&curr_time);

	get_time_long(&new_time);
	frame_delta = new_time - curr_time;

//	LOGI("pi_draw", "delta: %d next_time: %u curr_time: %u", delta, next_time, curr_time);

	if (frame_delta > 100000) frame_delta = 100000;

//	LOGI("calc_delta_time", "frame_delta: %u", frame_delta);

	frame_delta_avg_calc();

//	LOGI("calc_delta_time", "frame_delta after frame_delta_avg_calc(): %u", frame_delta);

	curr_time = new_time;

}

void calc_frame_rate() {

		LOGI("calc_frame_rate", "frame rate: %u", SEC_IN_US/frame_delta);

}


void pi_draw() {

//	LOGI("pi_draw", "pi_draw called");

//	posx+=(float)frame_delta * 0.00000015F;

	posx+=(float)frame_delta * 0.000000105F;


//		usleep(10000);


//
//		glViewport(0, 0, g_sc.width, g_sc.height);
		glClear(GL_COLOR_BUFFER_BIT);

		glUniform1f(g_sp.alpha, 0.8);

//		glUniform1f(g_sp.uFrame, (float) (frames % 240) / 150.0 - 0.8);

		glUniform1f(g_sp.uFrame,posx);
		glUniform1f(g_sp.uBlue, 1.0);

		glUniform1f(g_sp.posX, 0.5);
		glUniform1f(g_sp.posY, 0.5);


		// 使用するシェーダを指定
		glUseProgram(g_program);
		// 有効にするバッファオブジェクトを指定
		glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
		// シェーダのアトリビュート変数をアクセス可能にする
		glEnableVertexAttribArray(g_sp.aPosition);
		glEnableVertexAttribArray(g_sp.aTex);

		// 頂点情報のサイズ、オフセットを指定
		glVertexAttribPointer(g_sp.aPosition, 3, GL_FLOAT, GL_FALSE, 20, (void*) 0);
		glVertexAttribPointer(g_sp.aTex, 2, GL_FLOAT, GL_FALSE, 20, (void*) 12);

		glEnableVertexAttribArray(0);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);




		glUniform1f(g_sp.posX, -0.5);
		glUniform1f(g_sp.posY, -0.5);
		glUniform1f(g_sp.uFrame,posx*2.0F);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);



		glUniform1f(g_sp.posX, posx);
		glUniform1f(g_sp.posY, -0.5);
		glUniform1f(g_sp.uFrame,posx*3.0F);
		glUniform1f(g_sp.uBlue, 0.0);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);



		// 四角形
		glUniform1f(g_sp.posX, posx*2.0);
		glUniform1f(g_sp.posY, -0.5);
		glUniform1f(g_sp.uBlue, 0.3);
		glUniform1f(g_sp.uFrame,posx*4.0F);

		glBindBuffer(GL_ARRAY_BUFFER, g_vbo_2);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo_2);
//		glEnableVertexAttribArray(g_sp.aPosition);
//		glEnableVertexAttribArray(g_sp.aTex);

		// 頂点情報のサイズ、オフセットを指定
		glVertexAttribPointer(g_sp.aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 5, (void*) 0);
		glVertexAttribPointer(g_sp.aTex, 2, GL_FLOAT, GL_FALSE, 20, (void*) 12);

		glEnableVertexAttribArray(0);

		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);


		glUniform1f(g_sp.posX, posx*1.5);
		glUniform1f(g_sp.posY, -0.25);
		glUniform1f(g_sp.uFrame,posx*5.0F);
		glUniform1f(g_sp.uBlue, 0.7);
//		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);









		draw_touch_shapes();



		eglSwapBuffers(g_sc.display, g_sc.surface);





//		LOGI("pi_draw", "pi_draw finished");




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

void init_touch_shapes() {

	int i;
	for(i=0;i<TOUCH_SHAPES_MAX;i++) {
		touch_shapes[i].is_alive = FALSE;

		touch_shape_draw_order[i] = i;
	}
}

void step_touch_shape_draw_order() {

	int i;
	for(i=0;i<TOUCH_SHAPES_MAX;i++) {

			if (touch_shape_draw_order[i] < TOUCH_SHAPES_MAX)
				touch_shape_draw_order[i]++;
			if (touch_shape_draw_order[i] == TOUCH_SHAPES_MAX)
				touch_shape_draw_order[i] = 0;
		}
}


void activate_touch_shape(float x, float y) {
	LOGI("activate_touch_shape", "x: %f", x);
	LOGI("activate_touch_shape", "y: %f", y);

//	touch_shape* ts = cycle_touch_shapes();
	step_touch_shape_draw_order();
	touch_shape* ts = touch_shapes + (touch_shape_draw_order[TOUCH_SHAPES_MAX -1]);

	ts->pos_x = ((x/(float)g_sc.width)*2)-1;
	ts->pos_y = ((1.0F - (y/(float)g_sc.height))*2)-1;
//	LOGI("activate_touch_shape", "x: %f ts->pos_x: %f", x, ts->pos_x);
//	LOGI("activate_touch_shape", "y: %f ts->pos_y: %f", y, ts->pos_y);

	ts->alpha = 1.0;
	ts->ttl = TOUCH_SHAPES_TTL;

	ts->is_alive = TRUE;


}

void draw_touch_shapes() {
	int i;
	for (i=0; i<TOUCH_SHAPES_MAX; i++) {

		touch_shape* ts = touch_shapes + (touch_shape_draw_order[i]);

		if (ts->is_alive) {

			glUniform1f(g_sp.posX, ts->pos_x);
			glUniform1f(g_sp.posY, ts->pos_y);

//			glUniform1f(g_sp.uBlue, 0.7);

			ts->alpha -= (float)frame_delta *  0.000000205F;//(float)(SEC_IN_US/25);

			glUniform1f(g_sp.alpha, ts->alpha);
			glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

		}
	}
}

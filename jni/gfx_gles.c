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
#include "snd_scal.h"


#include <unistd.h>  // sleep()を定義
#include <pthread.h>

#include <math.h>
#include <stdlib.h>

#define DELTA_AVG_COUNT 6

//#define TOUCH_SHAPES_MAX 40//0000
#define TOUCH_SHAPES_MAX 24
//#define TOUCH_SHAPES_TTL 200.0F //必要ないよ。アルファでやるなら十分

#define PI 3.14159265358979
#define CIRCLE_SEGMENTS 24





typedef struct {

	int is_alive;

	int fading_in;

	GLfloat pos_x;
	GLfloat pos_y;

	GLfloat rgb[3];

	GLfloat alpha;

	GLfloat scale;




//	float ttl;


} touch_shape;

//typedef struct {
//
//} rgba;


// 取り敢えず
touch_shape* touch_shape_list[TOUCH_SHAPES_MAX];

touch_shape touch_shapes[TOUCH_SHAPES_MAX];
unsigned int touch_shape_draw_order[TOUCH_SHAPES_MAX];
size_t current_touch_shape = 0;


void init_touch_shapes();
//touch_shape* cycle_touch_shapes();
void draw_touch_shapes();
void draw_background();
void step_touch_shape_draw_order();
void calc_circle_vertex();

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

//const char vShaderSrc[] =
//		"attribute 	vec3		aPosition;	\n"
//		"attribute 	vec2		aTex;     	\n"
//		"varying   	vec2  	vTex;			\n"
//		"uniform	float 		uFrame;		\n"
//		"uniform	float 		posX;			\n"
//		"uniform	float		posY;			\n"
//		"uniform	float 		scale;		\n"
//		"uniform 	mat4 	projectionMatrix; \n"
//		"void main()							\n"
//		"	{                  						\n"
//		"		vTex = aTex;     				\n"
////		"		vTex = vec2(0.0, 0.0);		\n"
////		"		gl_Position = vec4(aPosition.x - uFrame, aPosition.y + uFrame, 0, 1);		\n"
////		"		gl_Position = projectionMatrix * vec4(aPosition.x + posX, aPosition.y + posY, 0, 1);		\n"
////		"		aPostion.x *=scale;				\n"
////		"		aPostion.y *=scale;			\n"
//		"		gl_Position = vec4((aPosition.x * scale) + posX, (aPosition.y*scale) + posY, 0, 1);		\n"
//		"	}                  						\n";


const char vShaderSrc[] =
		"attribute 	vec3		aposition;	\n"
		"attribute 	vec3		atex;     		\n"
		"varying   	vec3  	vtex;			\n"
		"uniform	float 		uframe;		\n"
		"uniform	float 		pos_x;			\n"
		"uniform	float		pos_y;			\n"
		"uniform	float 		scale;		\n"
//		"uniform 	mat4 	projectionMatrix; \n"
		"void main()							\n"
		"	{                  						\n"
		"		vtex = atex;     				\n"
		"		gl_Position = vec4((aposition.x * scale) + pos_x, (aposition.y*scale) + pos_y, 0, 1);		\n"
		"	}            \n";

//const char fShaderSrc[] =
//		"precision	mediump float;		\n"
//		"varying		vec2  	vTex;			\n"
//		"uniform 	float		ured;		\n"
//		"uniform 	float		ugrn;		\n"
//		"uniform 	float		ublu;			\n"
////		"uniform 	float 		uBlue;		\n"
//		"uniform	float 		alpha;			\n"
//		"void main()        					\n"
//		"	{                  						\n"
////		"  		gl_FragColor = vec4(vTex.y * u_red, vTex.x * u_grn, u_blu, alpha);	\n"
////		"  		gl_FragColor = vec4(vTex.y, vTex.x, u_blu, alpha);	\n"
//
////		"  		gl_FragColor = vec4(ured, ugrn, ublu, alpha);	\n"
//		"  		gl_FragColor = vec4(vTex.y * ured, vTex.x * ugrn, ublu, alpha);	\n"
//
//		"	}   	               						\n";

//const char fShaderSrc[] =
//		"precision	mediump float;		\n"
//		"varying		vec3  	vtex;			\n"
//		"uniform 	float		ured;			\n"
//		"uniform 	float		ugrn;			\n"
//		"uniform 	float		ublu;			\n"
//
//		"uniform 	vec3		vcol			\n"
//
//		"uniform	float 		alpha;			\n"
//
//
//		"void main()        					\n"
//		"	{                  						\n"
//
//
////		"  		gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);	\n"
//		"  		gl_FragColor = vec4(ured * vtex.x, ugrn * vtex.y, ublu * vtex.z, alpha);	\n"
//
//		"	}   	               						\n";
//


const char fShaderSrc[] =
		"precision	mediump float;		\n"
		"varying		vec3  	vtex;			\n"
		"uniform 	float		ured;			\n"
		"uniform 	float		ugrn;			\n"
		"uniform 	float		ublu;			\n"

//		"uniform 	vec3		vcol			\n"

		"uniform	float 		alpha;			\n"


		"void main()        					\n"
		"	{                  						\n"



//		"  		gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);	\n"
		"  		gl_FragColor = vec4(ured * vtex.x, ugrn * vtex.y, ublu * vtex.z, alpha);	\n"

		"	}   	               						\n";




//const char fShaderSrcBlue[] =
//  "precision mediump float;\n"\
//  "varying   vec2  vTex;\n"
//  "void main()        \n"
//  "{                  \n"
//  "  gl_FragColor = vec4(vTex.y, 0.0, 1.0, 0.8);\n"
//  "}                  \n";

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
  GLint  		uframe;
  GLint		pos_x;
  GLint 		pos_y;

  GLint rgb[3];

  GLint		alpha;
  GLint		scale;

} ShaderParams;

//typedef struct {
//    GLfloat x, y, z;
//    GLfloat u, v;
//} vertex;


typedef struct {
    GLfloat x, y, z;
    GLfloat r, g, b;
} vertex;

typedef struct {
	GLfloat r, g, b;
} vertex_rgb;

//vertex vObj[] = { // VertexBufferObjectを使用スべきか？
//
//  {.x = -0.5f, 	.y = -0.5f, 	.z = 0.0f, 	.u = 0.0f, 	.v = 1.0f},
//  {.x =  0.5f, 	.y = -0.5f, 	.z = 0.0f, 	.u = 1.0f, 	.v = 1.0f},
//  {.x =  0.0f, 	.y =  0.5f, 	.z = 0.0f, 	.u = 0.5f, 	.v = 0.0f},
//};
//
//
//
//
//unsigned short iObj[] = {
//  0, 1, 2
//};
//
//
//		// 四角形
////VertexType vObj_sq[] = { // VertexBufferObjectを使用スべきか？
////
////  {.x = -0.25f, 	.y =   0.5f, 	.z = 0.0f, 	.u = 0.0f, 	.v = 1.0f},
////  {.x = 0.25f, 		.y =   0.5f, 	.z = 0.0f, 	.u = 1.0f, 	.v = 1.0f},
////  {.x = 0.25f, 		.y = -0.5f, 	.z = 0.0f, 	.u = 0.5f, 	.v = 0.0f},
////  {.x = -0.25f, 	.y = -0.5f, 	.z = 0.0f, 	.u = 0.5f, 	.v = 0.0f},
////};
//vertex vObj_sq[] = { // VertexBufferObjectを使用スべきか？
//
//  {.x = -0.125f, 	.y =   0.25f, 	.z = 0.0f, 	.u = 0.0f, 	.v = 1.0f},
//  {.x = 0.125f, 		.y =   0.25f, 	.z = 0.0f, 	.u = 1.0f, 	.v = 1.0f},
//  {.x = 0.125f, 		.y = -0.25f, 	.z = 0.0f, 	.u = 0.5f, 	.v = 0.0f},
//  {.x = -0.125f, 	.y = -0.25f, 	.z = 0.0f, 	.u = 0.5f, 	.v = 0.0f},
//};
//unsigned short iObj_sq[] = {
////  0, 1, 2, 3
////  0, 1, 3, 2
//  0, 3, 1, 2
////  0, 2, 3
//};

vertex solid_circle_vertex[CIRCLE_SEGMENTS+1];
unsigned short solid_circle_index[CIRCLE_SEGMENTS+2];

//vertex bg_quad[] = {
//	{-1.0f, 	-1.0f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{1.0f, 		-1.0f, 	0.0f, 		0.0f, 		0.2f, 		0.0f},
//	{1.0f, 		1.0f, 		0.0f, 		0.95f,		0.0f, 		0.35f},
//	{-1.0f, 	1.0f, 		0.0f, 		0.25f,		0.0f, 		0.25f},
//};


void draw_all_backgrounds();

extern int selected_scale;

float alpha_fade_rate = 0.00000011F;

typedef struct {
	int fading_in;
	int fading_out;
	int selected_scale;
	float alpha;
} bg_def;

bg_def bgs [] = {
		{TRUE, 	FALSE,	0, 	0.0F},
		{FALSE, FALSE,	0, 	0.0F}
};

int curr_bg = 0;
int bgs_size = sizeof(bgs)/sizeof(bgs[0]);



vertex bg_quad[] = {
	{-1.0f, 	-1.0f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
	{1.0f, 		-1.0f, 	0.0f, 		0.0f, 		0.2f, 		0.0f},
	{1.0f, 		1.0f, 		0.0f, 		0.95f,		0.0f, 		0.35f},
	{-1.0f, 	1.0f, 		0.0f, 		0.25f,		0.0f, 		0.25f},
};

int seiseki[2][3] = {{72, 67, 84}, {67, 92, 71}};


vertex_rgb quad_colors[5][4] = {
		{
			{0.0f, 		0.0f, 		0.0f},
			{0.0f, 		0.0f, 		0.0f},
			{0.95f,	0.0f, 		0.35f},
			{0.25f,	0.0f, 		0.25f}
		},
		{

			{1.0f, 		0.0f, 		0.0f},
			{1.0f, 		0.0f, 		0.0f},
			{1.0f, 		0.0f, 		0.35f},
			{1.0f, 		0.0f, 		0.25f}
		},
		{
			{0.2f, 		0.3f, 		0.0f},
			{0.2f, 		0.3f, 		0.0f},
			{1.0f, 		0.0f, 		0.0f},
			{1.0f, 		0.0f, 		0.0f}
		},
		{
			{0.0f, 	 	0.1f, 		0.0f},
			{0.0f, 		0.6f, 		0.0f},
			{0.0f, 		0.2f, 		0.35f},
			{0.0f, 		0.2f, 		0.05f}
		},
		{
			{0.0f, 	 	0.5f, 		0.0f},
			{0.0f, 		0.3f, 		0.0f},
			{1.0f, 		0.2f, 		0.3f},
			{0.0f, 		1.0f, 		1.0f}
		}
};

//unsigned int current color = 0;


unsigned short bg_quad_index[] = {
  0, 1, 3, 2
};


ShaderParams    g_sp;
ScreenSettings  g_sc;

GLuint g_vbo;
GLuint g_ibo;

GLuint g_vbo_2;
GLuint g_ibo_2;

GLuint sc_vbo;
GLuint sc_ibo;

GLuint bg_quad_vbo;
GLuint bg_quad_ibo;

GLuint g_program;
GLuint g_program_purp;



GLuint quad_col_1;

GLuint bg_cols [TOTAL_SCALES];




typedef struct {
	float r, g, b;
} rgb;

rgb part_colors[] = {
	{0.597993F, 1.000000F, 0.658208F},
	{0.249421F, 0.896186F, 1.000000F},
	{1.000000F, 0.035373F, 0.319272F},
	{0.329373F, 0.094343F, 1.000000F},
	{0.879933F, 0.123191F, 1.000000F},
	{0.404804F, 0.984234F, 1.000000F},
	{0.465192F, 0.537905F, 1.000000F},
	{1.000000F, 0.186882F, 0.480862F}
};

//size_t current_part_color = 0; // 必要なのか？



unsigned int frames = 0;
float posx = -1.0F;
float global_scale = 1.0F;

float hw_ratio;

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

  	hw_ratio = (float)h / (float)w;

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
	calc_circle_vertex();

	int res;

	res = InitShaders(&g_program, vShaderSrc, fShaderSrc);
	if (!res)
		return 0;

	pi_create_buffer();

	g_sp.aposition = glGetAttribLocation(g_program, "aposition");
	g_sp.atex = glGetAttribLocation(g_program, "atex");
	g_sp.uframe = glGetUniformLocation(g_program, "uframe");

	g_sp.pos_x = glGetUniformLocation(g_program, "pos_x");
	g_sp.pos_y = glGetUniformLocation(g_program, "pos_y");

	g_sp.rgb[0] = glGetUniformLocation(g_program, "ured");
	g_sp.rgb[1]  = glGetUniformLocation(g_program, "ugrn");
	g_sp.rgb[2]  = glGetUniformLocation(g_program, "ublu");

	g_sp.alpha = glGetUniformLocation(g_program, "alpha");
	g_sp.scale = glGetUniformLocation(g_program, "scale");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glViewport(0, 0, g_sc.width, g_sc.height);

	  /* 平行投影変換行列を求める */
	  orthogonalMatrix(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, projectionMatrix);

	  /* uniform 変数 projectionMatrix の場所を得る */
	  projectionMatrixLocation = glGetUniformLocation(g_program, "projectionMatrix");

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
	glGenBuffers(1, &sc_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, sc_vbo);
	// データの転送
	glBufferData(GL_ARRAY_BUFFER, sizeof(solid_circle_vertex), solid_circle_vertex, GL_STATIC_DRAW);

	// インデックスバッファの作成
	glGenBuffers(1, &sc_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sc_ibo);
	// データの転送
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(solid_circle_index), solid_circle_index, GL_STATIC_DRAW);




	// VBOの生成
	glGenBuffers(1, &bg_quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, bg_quad_vbo);
	// データの転送
	glBufferData(GL_ARRAY_BUFFER, sizeof(bg_quad), bg_quad, GL_STATIC_DRAW);

	// インデックスバッファの作成
	glGenBuffers(1, &bg_quad_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bg_quad_ibo);
	// データの転送
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bg_quad_index), bg_quad_index, GL_STATIC_DRAW);


	int i;
	for (i=0; i<TOTAL_SCALES; i++) {
		glGenBuffers(1, &bg_cols[i]);
		glBindBuffer(GL_ARRAY_BUFFER, bg_cols[i]);
		glBufferData(GL_ARRAY_BUFFER, (sizeof(quad_colors))/4, quad_colors[i], GL_STATIC_DRAW);


	}


//	glGenBuffers(1, &quad_col_1);
//	glBindBuffer(GL_ARRAY_BUFFER, quad_col_1);
//	glBufferData(GL_ARRAY_BUFFER, (sizeof(quad_colors))/4, quad_colors[3], GL_STATIC_DRAW);




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


//	posx+=(float)frame_delta * 0.000000105F;


//
//		glViewport(0, 0, g_sc.width, g_sc.height);
		glClear(GL_COLOR_BUFFER_BIT);

		glUniform1f(g_sp.scale, global_scale);



		glUniform1f(g_sp.alpha, 0.8);
		glUniform1f(g_sp.uframe, posx);


//glUniform3f()
		glUniform1f(g_sp.rgb[0], 1.0);
		glUniform1f(g_sp.rgb[1], 1.0);
		glUniform1f(g_sp.rgb[2], 1.0);

		glUseProgram(g_program);


//		glUniform1f(g_sp.posX, 0.5);
//		glUniform1f(g_sp.posY, 0.5);
//
//
//		// 使用するシェーダを指定
//		glUseProgram(g_program);
//		// 有効にするバッファオブジェクトを指定
//		glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
//		// シェーダのアトリビュート変数をアクセス可能にする
//		glEnableVertexAttribArray(g_sp.aPosition);
//		glEnableVertexAttribArray(g_sp.aTex);
//
//		// 頂点情報のサイズ、オフセットを指定
//
////		http://marina.sys.wakayama-u.ac.jp/~tokoi/?date=20090828
////		第４引数は, データ型が整数型であったときに,
////		それを [0,1] または [-1,1] の範囲に正規化するか否かを指定します.
////		ここでは正規化しないので, GL_FALSE を指定します.
////		第５引数には頂点情報と頂点情報の間隔を指定します.
////		頂点情報が密に (隙間無く) 格納されていれば, 0 を指定します.
////		そして第６引数には, 頂点情報を格納している領域の先頭の位置を指定します.
////		ここでは頂点情報は頂点バッファオブジェクトの先頭から格納されているので, 0 を指定します.
//
////		 頂点情報の格納場所と書式を指定する
////		  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
//
//
//		glVertexAttribPointer(g_sp.aPosition, 3, GL_FLOAT, GL_FALSE, 20, (void*) 0);
//		glVertexAttribPointer(g_sp.aTex, 2, GL_FLOAT, GL_FALSE, 20, (void*) 12);
//
//		glEnableVertexAttribArray(0);
//		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
//
//
//
//
//		glUniform1f(g_sp.posX, -0.5);
//		glUniform1f(g_sp.posY, -0.5);
//		glUniform1f(g_sp.uFrame,posx*2.0F);
//		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
//
//
//
//		glUniform1f(g_sp.posX, posx);
//		glUniform1f(g_sp.posY, -0.5);
//		glUniform1f(g_sp.uFrame,posx*3.0F);
//		glUniform1f(g_sp.u_blu, 0.0);
//		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
//
//
//
//		// 四角形
//		glUniform1f(g_sp.posX, posx*2.0);
//		glUniform1f(g_sp.posY, -0.5);
//		glUniform1f(g_sp.u_blu, 0.3);
//		glUniform1f(g_sp.uFrame,posx*4.0F);
//
//		glBindBuffer(GL_ARRAY_BUFFER, g_vbo_2);
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo_2);
////		glEnableVertexAttribArray(g_sp.aPosition);
////		glEnableVertexAttribArray(g_sp.aTex);
//
//		// 頂点情報のサイズ、オフセットを指定
//		glVertexAttribPointer(g_sp.aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 5, (void*) 0);
//		glVertexAttribPointer(g_sp.aTex, 2, GL_FLOAT, GL_FALSE, 20, (void*) 12);
//
//		glEnableVertexAttribArray(0);
//
//		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
//
//
//		glUniform1f(g_sp.posX, posx*1.5);
//		glUniform1f(g_sp.posY, -0.25);
//		glUniform1f(g_sp.uFrame,posx*5.0F);
//		glUniform1f(g_sp.u_blu, 0.7);
////		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
//		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
//
//
//
//		// 四角形
//		glUniform1f(g_sp.posX, posx*2.0);
//		glUniform1f(g_sp.posY, -0.2);
//		glUniform1f(g_sp.u_blu, 0.5);
//		glUniform1f(g_sp.uFrame,posx*1.23450F);

//========================================================================






		glBindBuffer(GL_ARRAY_BUFFER, bg_quad_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bg_quad_ibo);
		glEnableVertexAttribArray(g_sp.aposition);
//		glEnableVertexAttribArray(g_sp.atex);





		glVertexAttribPointer(g_sp.aposition, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) 0);


		draw_all_backgrounds();






//		glBindBuffer(GL_ARRAY_BUFFER, bg_cols[selected_scale]);
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bg_quad_ibo);
//		glEnableVertexAttribArray(g_sp.atex);
//
//		glVertexAttribPointer(g_sp.atex, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*) 0);
//
//
//		draw_background();
//
//
//
//
//		glBindBuffer(GL_ARRAY_BUFFER, bg_cols[1]);
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bg_quad_ibo);
//		glEnableVertexAttribArray(g_sp.atex);
//
//		glVertexAttribPointer(g_sp.atex, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*) 0);
//
//
//		draw_background();









		glBindBuffer(GL_ARRAY_BUFFER, sc_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sc_ibo);
		glEnableVertexAttribArray(g_sp.aposition);
		glEnableVertexAttribArray(g_sp.atex);

		// 頂点情報のサイズ、オフセットを指定
		glVertexAttribPointer(g_sp.aposition, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) 0);
		glVertexAttribPointer(g_sp.atex, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) (sizeof(GL_FLOAT) * 3));


		glEnableVertexAttribArray(0);


//
//		  glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);



		draw_touch_shapes();




//		usleep(100000);


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

			LOGI("step_touch_shape_draw_order", "touch_shape_draw_order[%d]: %d", i, touch_shape_draw_order[i]);
	}

}


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void activate_touch_shape(float x, float y, size_t col, float* vel) {

	pthread_mutex_lock(&mutex);
	step_touch_shape_draw_order();
	touch_shape* ts = touch_shapes + (touch_shape_draw_order[TOUCH_SHAPES_MAX -1]);
//	LOGI("activate_touch_shape", "touch_shape_draw_order[TOUCH_SHAPES_MAX -1] %d",
//			touch_shape_draw_order[TOUCH_SHAPES_MAX -1]);

	ts->pos_x = ((x/(float)g_sc.width)*2)-1;
	ts->pos_y = ((1.0F - (y/(float)g_sc.height))*2)-1;
//	LOGI("activate_touch_shape", "x: %f ts->pos_x: %f", x, ts->pos_x);
//	LOGI("activate_touch_shape", "y: %f ts->pos_y: %f", y, ts->pos_y);



	ts->rgb[0] = part_colors[col].r;
	ts->rgb[1] = part_colors[col].g;
	ts->rgb[2] = part_colors[col].b;


//	LOGI("activate_touch_shape", "ts->rgb[0] %f ts->rgb[1] %f ts->rgb[2] %f", ts->rgb[0], ts->rgb[1], ts->rgb[2]);

//	ts->alpha = 1.0;
//	ts->scale = 1.0;
//	ts->alpha = *vel * *vel; // TODO
	ts->alpha = 0.0F; // TODO
	ts->scale = *vel * *vel; // TODO 既に計算すればいいのかも

//	ts->ttl = TOUCH_SHAPES_TTL;

	ts->fading_in = TRUE;
	ts->is_alive = TRUE;
	pthread_mutex_unlock(&mutex);
}

float bg_pulse = 0.0F;
float bg_pulse_dir = 1.0F;
float test_alpha = 1.0F;

//void draw_background() {
//
//	bg_pulse += (float)frame_delta * 0.0000001F * bg_pulse_dir;
//	glUniform1f(g_sp.pos_x, 0.0);
//	glUniform1f(g_sp.pos_y, 0.0);
//
//	if (bg_pulse_dir == 1.0F && bg_pulse > 1.0) bg_pulse_dir = -1.0F;
//	if (bg_pulse_dir == -1.0F && bg_pulse < 0.2) bg_pulse_dir = 1.0F;
//
//	glUniform1f(g_sp.rgb[0], 0.5 * bg_pulse);
//	glUniform1f(g_sp.rgb[1], 1.0);
//	glUniform1f(g_sp.rgb[2], 1.0);
//
//	glUniform1f(g_sp.alpha, 0.5);
//	glUniform1f(g_sp.scale, 1.0);
//
//	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
//
//}

void draw_background_alpha(float a) {

	bg_pulse += (float)frame_delta * 0.00000021F * bg_pulse_dir;

	glUniform1f(g_sp.pos_x, 0.0);
	glUniform1f(g_sp.pos_y, 0.0);

	if (bg_pulse_dir == 1.0F && bg_pulse > 1.0) bg_pulse_dir = -1.0F;
	if (bg_pulse_dir == -1.0F && bg_pulse < 0.2) bg_pulse_dir = 1.0F;

	glUniform1f(g_sp.rgb[0], 0.5 * bg_pulse);
	glUniform1f(g_sp.rgb[1], 1.0);
	glUniform1f(g_sp.rgb[2], 1.0);

	glUniform1f(g_sp.alpha, a);
	glUniform1f(g_sp.scale, 1.0);

	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

}

int count =0;

void draw_all_backgrounds() {


	bg_def* bg;
	int i = 0;

		count++;

	for (i=0; i<bgs_size; i++) {

//		bg_def* bg = bgs + i;
		bg = bgs + i;

		if (count%20 == 0)
		LOGD("draw_all_backgrounds", "i: %d, in: %d, out: %d. bg->alpha: %f", i, bg->fading_in, bg->fading_out, bg->alpha);

		if (bg->fading_in) {
			bg->alpha += (float)frame_delta * alpha_fade_rate;
			if (bg->alpha >= 1.0F) {
				bg->fading_in = FALSE;
				bg->alpha = 1.0F;
			}
		}

		if (bg->fading_out) {

			if (count%20 == 0) {
				LOGD("draw_all_backgrounds", "(double)frame_delta * alpha_fade_rate: %f", (float)frame_delta * alpha_fade_rate);
				LOGD("draw_all_backgrounds", "(float)frame_delta: %f", (float)frame_delta);
				LOGD("draw_all_backgrounds", "alpha_fade_rate: %f", alpha_fade_rate);
				LOGD("draw_all_backgrounds", "bg->alpha: %f", bg->alpha);
			}
			bg->alpha -= (float)frame_delta * alpha_fade_rate;
			if (bg->alpha <= 0.0F) {
				bg->fading_out = FALSE;
				bg->alpha = 0.0F;
			}
		}

		if (bg->alpha > 0.0F) {

			glBindBuffer(GL_ARRAY_BUFFER, bg_cols[bg->selected_scale]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bg_quad_ibo);
			glEnableVertexAttribArray(g_sp.atex);
			glVertexAttribPointer(g_sp.atex, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*) 0);

			draw_background_alpha(bg->alpha);
		}
	}

}


void start_xfade_bgs() {

	LOGD("start_xfade_bgs", "curr_bg: %d", curr_bg);
	bg_def* bg = bgs + curr_bg;
	bg->fading_out = TRUE;

	if (curr_bg < bgs_size) curr_bg++;
	if (curr_bg == bgs_size) curr_bg = 0;

	bg = bgs + curr_bg;
	bg->selected_scale = selected_scale;
	bg->fading_in = TRUE;

}

int bg_fading() {

	int rtn = FALSE;
	int i;
	for (i=0; i<bgs_size; i++) {
	bg_def* bg = bgs + i;

		if (bg->fading_in || bg->fading_out) {
			rtn = TRUE;
		}
		if (!bg->fading_in && !bg->fading_out) {
			rtn = FALSE;
		}
	}

	LOGD ("bg_fading", "rtn: %d", rtn);
	return rtn;
}







void draw_touch_shapes() {
	int i;

			pthread_mutex_lock(&mutex);
	for (i=0; i<TOUCH_SHAPES_MAX; i++) {

		touch_shape* ts = touch_shapes + (touch_shape_draw_order[i]);

		if (ts->is_alive) {


			glUniform1f(g_sp.pos_x, ts->pos_x);
			glUniform1f(g_sp.pos_y, ts->pos_y);

			glUniform1f(g_sp.rgb[0], ts->rgb[0]);
			glUniform1f(g_sp.rgb[1], ts->rgb[1]);
			glUniform1f(g_sp.rgb[2], ts->rgb[2]);

			ts->scale += (float)frame_delta * 0.0000003F;

			if (ts->fading_in) {

				ts->alpha += (float)frame_delta *  0.0000039F;//(float)(SEC_IN_US/25);
				if (ts->alpha >= 1.0F) ts->fading_in = FALSE;
			}

			if (!ts->fading_in) {
				ts->alpha -= (float)frame_delta *  0.000000205F;//(float)(SEC_IN_US/25);
				if (ts->alpha <= 0) ts->is_alive = FALSE;
			}

			glUniform1f(g_sp.alpha, ts->alpha);
			glUniform1f(g_sp.scale, ts->scale);

			glDrawElements(GL_TRIANGLE_FAN, CIRCLE_SEGMENTS + 2, GL_UNSIGNED_SHORT, 0);

		}
	}
	pthread_mutex_unlock(&mutex);
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

void calc_circle_vertex() {

	int i, n = CIRCLE_SEGMENTS;
	float r = 0.25F;
	double rate;

	float largest_x = 0.0;
	float largest_y = 0.0;
	float one_factor_x = 1.0;
	float one_factor_y = 1.0;

	solid_circle_vertex[0].x = 0.0;
	solid_circle_vertex[0].y = 0.0;

	for (i = 0; i < n; i++) {

		rate = (double) i / n;
		solid_circle_vertex[i + 1].x = r * hw_ratio * sin(2.0 * PI * rate);
		solid_circle_vertex[i + 1].y = r * cos(2.0 * PI * rate);

		float abs_x = fabsf(solid_circle_vertex[i + 1].x);
		float abs_y = fabsf(solid_circle_vertex[i + 1].y);

		if (abs_x > largest_x)
			largest_x = abs_x;
		if (abs_y > largest_y)
			largest_y = abs_y;

	}

	one_factor_x = 1.0F / largest_x;
	one_factor_y = 1.0F / largest_y;

	for (i = 0; i < CIRCLE_SEGMENTS + 1; i++) {
		solid_circle_index[i] = i;

		solid_circle_vertex[i].r = solid_circle_vertex[i].x * one_factor_x;
		solid_circle_vertex[i].g = solid_circle_vertex[i].y * one_factor_y;
		solid_circle_vertex[i].b = 0.5;//solid_circle_vertex[i].y * one_factor_y;

		LOGI(
				"calc_circle_vertex",
				"solid_circle_vertex[i].r %f "
				"solid_circle_vertex[i].g %f "
				"solid_circle_vertex[i].b %f ",
				solid_circle_vertex[i].r, solid_circle_vertex[i].g, solid_circle_vertex[i].b);

	}
	solid_circle_index[CIRCLE_SEGMENTS + 1] = 1;

}


//
//size_t cycle_part_color() {
//
//	if ()
//
//}


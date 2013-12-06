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


#include "and_main.h"
#include "gfx_gles.h"
#include "gfx_asst.h"
#include "gfx_butn.h"
#include "game/moods.h"





#define DELTA_AVG_COUNT 6

//#define TOUCH_SHAPES_MAX 40//0000
#define TOUCH_SHAPES_MAX 24
//#define TOUCH_SHAPES_TTL 200.0F //必要ないよ。アルファでやるなら十分

#define PI 3.14159265358979
//#define CIRCLE_SEGMENTS 24
#define CIRCLE_SEGMENTS 6


#define SPLASH_COUNT_SECS 10

typedef struct {
	int is_alive;
	int fading_in;
	GLfloat pos_x;
	GLfloat pos_y;
	GLfloat rgb[3];
	GLfloat alpha;
	GLfloat alpha_max;
	GLfloat alpha_delta_factor;
	GLfloat scale;
//	int is_alive_ripple;
//	int fading_in_ripple;
//
//	GLfloat alpha_ripple;
//	GLfloat scale_ripple;

//	float ttl;
} touch_shape;


// 取り敢えず
//touch_shape* touch_shape_list[TOUCH_SHAPES_MAX];

touch_shape touch_shapes[TOUCH_SHAPES_MAX];
unsigned int touch_shape_draw_order[TOUCH_SHAPES_MAX];
size_t current_touch_shape = 0;

touch_shape touch_ripples[TOUCH_SHAPES_MAX];


touch_shape touch_no_ammo[TOUCH_SHAPES_MAX];
unsigned int touch_no_ammo_draw_order[TOUCH_SHAPES_MAX];
size_t current_touch_no_ammo = 0;











void frame_delta_avg_init();
void init_touch_shapes();
//touch_shape* cycle_touch_shapes();
void draw_touch_shapes();
void draw_touch_ripples();
//void draw_background(); //必要ない

void draw_button();
void draw_gameplay();


void step_touch_shape_draw_order();
void calc_circle_vertex();
void create_gl_buffers();


//float gl_square_x_to_y(float w);
//void calc_button_coords();
//void calc_btn_quad_verts(int bm_w, int bm_h);
//float gl_to_scr(float gl, int is_y);

//int create_gl_texture(texture_type *tt);
int init_shaders(GLuint *program, char const *vShSrc, char const *fShSrc);
//void draw_splash();
//void draw_all_backgrounds();



//void draw_full_screen_image(full_screen* fs);

//typedef struct {
//	EGLNativeWindowType nativeWin;
//	EGLDisplay display;
//	EGLContext context;
//	EGLSurface surface;
//	EGLint majorVersion;
//	EGLint minorVersion;
//	int width;
//	int height;
//
////	float		display_ratio;
//	float hw_ratio;
//} screen_settings;




const char* vertex_shader =
	"attribute vec4 position;\n"
	"attribute vec2 texcoord;\n"
	"varying vec2 texcoordVarying;\n"
	"void main() {\n"
		"gl_Position = position;\n"
		"texcoordVarying = texcoord;\n"
	"}\n";

const char* fragment_shader =
	"precision mediump float;\n"
	"varying vec2 texcoordVarying;\n"
	"uniform sampler2D texture;\n"
	"void main() {\n"
		"gl_FragColor = texture2D(texture, texcoordVarying);\n"
	"}\n";


// スプラッシュ画面のシェーダー

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


// 本体のアプリのシェーダー

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


//const char v_shdr_main[] = // vShaderSrc
//		"attribute 	vec3		aposition;	\n"
//		"attribute 	vec3		atex;     		\n"
//		"varying   	vec3  	vtex;			\n"
////		"uniform	float 		uframe;		\n"
//		"uniform	float 		pos_x;			\n"
//		"uniform	float		pos_y;			\n"
//		"uniform	float 		scale;		\n"
////		"uniform 	mat4 	projectionMatrix; \n"
//		"void main()							\n"
//		"	{                  						\n"
//		"		vtex = atex;     				\n"
//		"		gl_Position = vec4((aposition.x * scale) + pos_x, (aposition.y*scale) + pos_y, 0, 1);		\n"
//		"	}            \n";
//



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
	GLint 		rgb[3];
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
} shader_params_tex;




struct vertex solid_circle_vertex[CIRCLE_SEGMENTS+1];
unsigned short solid_circle_index[CIRCLE_SEGMENTS+2];



//extern int selected_scale;
//extern int sles_init_called;

//float alpha_fade_rate = 0.11f / (float)SEC_IN_US;
//
//typedef struct {
//	int fading_in;
//	int fading_out;
//	int selected_scale;
//	float alpha;
//} bg_def;
//
//bg_def bgs [] = {
//		{TRUE, 	FALSE,	0, 	0.0F},
//		{FALSE, FALSE,	0, 	0.0F}
//};

//int curr_bg = 0;
//int bgs_size = sizeof(bgs)/sizeof(bgs[0]);

//vertex bg_quad[] = {
//	{-1.0f, 	-1.0f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{1.0f, 		-1.0f, 	0.0f, 		0.0f, 		0.2f, 		0.0f},
//	{1.0f, 		1.0f, 		0.0f, 		0.95f,		0.0f, 		0.35f},
//	{-1.0f, 	1.0f, 		0.0f, 		0.25f,		0.0f, 		0.25f},
//};

// wtf???
//int seiseki[2][3] = {{72, 67, 84}, {67, 92, 71}};


//vertex_rgb quad_colors[5][4] = {
//		{
//			{0.0f, 		0.0f, 		0.0f},
//			{0.0f, 		0.0f, 		0.0f},
//			{0.95f,	0.0f, 		0.35f},
//			{0.25f,	0.0f, 		0.25f}
//		},
//		{
//
//			{1.0f, 		0.0f, 		0.0f},
//			{1.0f, 		0.0f, 		0.0f},
//			{1.0f, 		0.0f, 		0.35f},
//			{1.0f, 		0.0f, 		0.25f}
//		},
//		{
//			{0.2f, 		0.3f, 		0.0f},
//			{0.2f, 		0.3f, 		0.0f},
//			{1.0f, 		0.0f, 		0.0f},
//			{1.0f, 		0.0f, 		0.0f}
//		},
//		{
//			{0.0f, 	 	0.1f, 		0.0f},
//			{0.0f, 		0.6f, 		0.0f},
//			{0.0f, 		0.2f, 		0.35f},
//			{0.0f, 		0.2f, 		0.05f}
//		},
//		{
//			{0.0f, 	 	0.5f, 		0.0f},
//			{0.0f, 		0.3f, 		0.0f},
//			{1.0f, 		0.2f, 		0.3f},
//			{0.0f, 		1.0f, 		1.0f}
//		}
//};

//unsigned short bg_quad_index[] = {
//  0, 1, 3, 2
//};




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

//float gl_square_x_to_y(float w) {
//	float h = (1.0f/g_sc.hw_ratio) * w;
//  	return h;
//}





GLuint g_vbo;
GLuint g_ibo;
//GLuint g_vbo_2;
//GLuint g_ibo_2;
GLuint sc_vbo;
GLuint sc_ibo;
GLuint fs_quad_vbo;
GLuint fs_quad_ibo;


GLuint btn_vbo;
GLuint btn_ibo;


GLuint g_prog_main;
//GLuint g_program_purp;
GLuint quad_col_1;
//GLuint bg_cols [sizeof_moods_elements];
GLuint bg_cols [5];



// テキスチャーのフィールド
shader_params_tex g_sp_t;
//texture_type      g_tt;
GLuint g_prog_splash;



shader_params_tex g_sp_btn;
GLuint g_prog_button;
//extern button buttons;
//extern vertex btn_quad[];
//extern unsigned short btn_quad_index[];



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


// フェードとタイミング
//int splash_remaining = SPLASH_COUNT_SECS * SEC_IN_US;


//int splash_fading_in = TRUE;
//int splash_fading_out = FALSE;
//int show_splash = TRUE;






unsigned int frames = 0;
//float posx = -1.0F; //wtf?
float global_scale = 1.0F;

//float hw_ratio;

struct timezone tzp;
struct timeval get_time;

unsigned long start_time = 0;

unsigned long curr_time = 0;
unsigned long new_time = 0;




unsigned long frame_delta = 0;
int frame_delta_avg[DELTA_AVG_COUNT];
size_t frame_delta_cycle = 0;

float frame_delta_ratio = 1.0; // ratio of delta to ideal fps (60)

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

	init_touch_shapes();
	calc_circle_vertex();


//	calc_button_coords();

	calc_btn_quad_verts(BTN_W, BTN_W);

	int res;

	res = init_shaders(&g_prog_splash, v_shdr_splash, f_shdr_splash);
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

	g_sp_m.rgb[0] = glGetUniformLocation(g_prog_main, "ured");
	g_sp_m.rgb[1]  = glGetUniformLocation(g_prog_main, "ugrn");
	g_sp_m.rgb[2]  = glGetUniformLocation(g_prog_main, "ublu");

	g_sp_m.alpha = glGetUniformLocation(g_prog_main, "alpha");
	g_sp_m.scale = glGetUniformLocation(g_prog_main, "scale");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

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

	LOGD("init_cmds", "init_cmds() finished");
	return TRUE;


}


//void init_pi(struct engine* e) { // gles2_py_texture からの関数
//
//
////	res = create_window_surface(e->app->window);
////	if (!res)
////	return;
//
////	res = InitShaders(&g_program, vShaderSrc, fShaderSrc);
////	if (!res) return;
//
//	create_gl_buffers();
//
//	g_sp.display = glGetUniformLocation(g_program, "display");
//	g_sp.bitmap_ratio = glGetUniformLocation(g_program, "bitmap_ratio");
//
//	g_sp.position = glGetAttribLocation(g_program, "vPosition");
//	g_sp.tex = glGetAttribLocation(g_program, "aTex");
//
//	int size;
//
//	size = load_bitmap("/mnt/sdcard/Android/data/nz.kapsy.gles2_py_texture/files/splash_test_001_800x400.bmp", (void *)g_bmpbuffer);
//	LOGD("init_pi", "LoadFile %d", size);
//
//	check_bitmap(&g_tt, (void *)&g_bmpbuffer);
//	make_texture(&g_tt, 255);
//	create_gl_texture(&g_tt);
//}




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










//struct vertex fs_quad[];

//struct vertex fs_quad[] = {
//	{-1.0f, 	-1.0f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{1.0f, 		-1.0f, 	0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{1.0f, 		1.0f, 		0.0f, 		0.0f, 		0.0f, 		0.0f},
//	{-1.0f, 	1.0f, 		0.0f, 		0.0f, 		0.0f,		0.0f},
//};
//unsigned short fs_quad_index[] = {
//  0, 1, 3, 2
//};

// GPU上のバッファオブジェクトにデータを転送
void create_gl_buffers()
{

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(solid_circle_vertex), solid_circle_vertex, GL_STATIC_DRAW);

	// インデックスバッファの作成
	glGenBuffers(1, &sc_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sc_ibo);
	// データの転送
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(solid_circle_index), solid_circle_index, GL_STATIC_DRAW);



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



	//vertex_rgb quad_colors[5][4] = {
	//		{
	//			{0.0f, 		0.0f, 		0.0f},
	//			{0.0f, 		0.0f, 		0.0f},
	//			{0.95f,	0.0f, 		0.35f},
	//			{0.25f,	0.0f, 		0.25f}
	//		},


	int i;
	for (i=0; i<sizeof_moods_elements; i++) {
		glGenBuffers(1, &bg_cols[i]);
		glBindBuffer(GL_ARRAY_BUFFER, bg_cols[i]);
//		glBufferData(GL_ARRAY_BUFFER, (sizeof(quad_colors))/4, quad_colors[i], GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, sizeof((moods +i)->bg->colors), (moods +i)->bg->colors, GL_STATIC_DRAW);
	}

	LOGD("create_gl_buffers", "pi_create_buffer() finished");
}

int create_gl_texture(struct texture_type *tt)
{
  glGenTextures(1, &tt->texname);
  glBindTexture(GL_TEXTURE_2D, tt->texname);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tt->BmpWidth, tt->BmpHeight, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, tt->TexData);
  return 1;
}




void get_time_long(unsigned long* t) {


	gettimeofday(&get_time, &tzp);

	*t = (get_time.tv_sec * SEC_IN_US) + get_time.tv_usec;

}

void get_start_time() {

	get_time_long(&start_time);
}

void get_elapsed_time(unsigned long* t) {

//	get_time_long(&curr_time);
	*t = curr_time - start_time;
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


//	frame_delta_ratio = 17000/(float) frame_delta;
//	if (frame_delta_ratio >= 1.0) frame_delta_ratio = 1.0;


	curr_time = new_time;

}



void calc_frame_rate() {

		LOGI("calc_frame_rate", "frame rate: %u", SEC_IN_US/frame_delta);

}




void draw_full_screen_image(struct full_screen_element* fs) {


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

	glUniform1f(g_sp_t.alpha, fs->alpha);

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

		btn_anim(b, i);

		glUniform1f(g_sp_btn.pos_x, b->gl_x);
		glUniform1f(g_sp_btn.pos_y, b->gl_y);
		glUniform1f(g_sp_btn.scale, b->scale);

// alpha_pt>0.0のみ
		glUniform1f(g_sp_btn.alpha, b->alpha_pt);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, b->pressed_texture->tt.texname);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);



		glUniform1f(g_sp_btn.alpha, b->alpha);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, b->main_texture->tt.texname);
		//	glUniform2f(g_sp_btn.position, 1.0F, 1.0F);
//		glUniform1f(g_sp_btn.pos_x, b->gl_x);
//		glUniform1f(g_sp_btn.pos_y, b->gl_y);
//		glUniform1f(g_sp_btn.scale, b->scale);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);




	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}







void draw_gameplay() {

//	glUniform1f(g_sp_m.scale, global_scale);
//	glUniform1f(g_sp_m.alpha, 0.8);
//	glUniform1f(g_sp_m.rgb[0], 1.0);
//	glUniform1f(g_sp_m.rgb[1], 1.0);
//	glUniform1f(g_sp_m.rgb[2], 1.0);
//
//	glUseProgram(g_prog_main);
//
//	glBindBuffer(GL_ARRAY_BUFFER, fs_quad_vbo);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fs_quad_ibo);
//	glEnableVertexAttribArray(g_sp_m.aposition);
//
//
//	glVertexAttribPointer(g_sp_m.aposition, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) 0);
//	draw_all_backgrounds();

	glBindBuffer(GL_ARRAY_BUFFER, sc_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sc_ibo);
	glEnableVertexAttribArray(g_sp_m.aposition);
	glEnableVertexAttribArray(g_sp_m.atex);

	// 頂点情報のサイズ、オフセットを指定
	glVertexAttribPointer(g_sp_m.aposition, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) 0);
	glVertexAttribPointer(g_sp_m.atex, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) (sizeof(GL_FLOAT) * 3));

	glEnableVertexAttribArray(0);
//		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);

	draw_touch_ripples();
	draw_touch_shapes();

//	draw_touch_no_ammo();



//		usleep(100000);
//		usleep(20000);




}






void draw_frame() {

//	glViewport(0, 0, g_sc.width, g_sc.height);
	glClear(GL_COLOR_BUFFER_BIT);


	if (screens[0].is_showing) {
		fse_anim(screens + 1);
		fse_anim(screens + 0);
	}

	if(show_gameplay) {

		bg_anim_all();
		draw_gameplay();

		 if (!screens[0].fading_out)
			 draw_button();
	}



	if(screens[2].is_showing) {
		fse_anim(screens + 2);
	}

	eglSwapBuffers(g_sc.display, g_sc.surface);

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




void init_touch_shapes() {
	int i;
	for(i=0;i<TOUCH_SHAPES_MAX;i++) {
		touch_shapes[i].is_alive = FALSE;
		touch_ripples[i].is_alive = FALSE;
		touch_no_ammo[i].is_alive = FALSE;
		touch_shape_draw_order[i] = i;
		touch_no_ammo_draw_order[i] = i;
	}
}

void step_touch_shape_draw_order() {
	int i;
	for(i=0;i<TOUCH_SHAPES_MAX;i++) {
			if (touch_shape_draw_order[i] < TOUCH_SHAPES_MAX)
				touch_shape_draw_order[i]++;
			if (touch_shape_draw_order[i] == TOUCH_SHAPES_MAX)
				touch_shape_draw_order[i] = 0;
//			LOGI("step_touch_shape_draw_order", "touch_shape_draw_order[%d]: %d", i, touch_shape_draw_order[i]);
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
	LOGI("activate_touch_shape", "x: %f ts->pos_x: %f", x, ts->pos_x);
	LOGI("activate_touch_shape", "y: %f ts->pos_y: %f", y, ts->pos_y);

	ts->rgb[0] = part_colors[col].r;
	ts->rgb[1] = part_colors[col].g;
	ts->rgb[2] = part_colors[col].b;

//	LOGI("activate_touch_shape", "ts->rgb[0] %f ts->rgb[1] %f ts->rgb[2] %f", ts->rgb[0], ts->rgb[1], ts->rgb[2]);

	ts->alpha = 0.0F; // TODO


	ts->scale = *vel * *vel * 1.7; // TODO 既に計算すればいいのかも
//	LOGI("activate_touch_shape", "ts->scale: %f", ts->scale);

	ts->alpha_max = ts->scale / 2.0; // TODO

//	ts->ttl = TOUCH_SHAPES_TTL;

	ts->fading_in = TRUE;
	ts->is_alive = TRUE;


	touch_shape* tr = touch_ripples + (touch_shape_draw_order[TOUCH_SHAPES_MAX -1]);
	tr->pos_x = ((x/(float)g_sc.width)*2)-1;
	tr->pos_y = ((1.0F - (y/(float)g_sc.height))*2)-1;
	tr->rgb[0] = part_colors[col].r;
	tr->rgb[1] = part_colors[col].g;
	tr->rgb[2] = part_colors[col].b;
	tr->alpha = 0.0F; // TODO
	tr->scale = *vel * *vel * 1.7; // TODO 既に計算すればいいのかも



	tr->alpha_max = *vel;
	if (tr->alpha_max >= 1.0) tr->alpha_max = 1.0;
	LOGI("activate_touch_shape", "tr->alpha_max: %f", tr->alpha_max);


	tr->alpha_delta_factor = 0.000004F;


	tr->fading_in = TRUE;
	tr->is_alive = TRUE;


	pthread_mutex_unlock(&mutex);
}





void draw_background(struct background* bg) {

//		glUniform1f(g_sp_m.scale, global_scale);
//		glUniform1f(g_sp_m.alpha, 0.8);
//		glUniform1f(g_sp_m.rgb[0], 1.0);
//		glUniform1f(g_sp_m.rgb[1], 1.0);
//		glUniform1f(g_sp_m.rgb[2], 1.0);
	glUseProgram(g_prog_main);

	glBindBuffer(GL_ARRAY_BUFFER, fs_quad_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fs_quad_ibo);
	glEnableVertexAttribArray(g_sp_m.aposition);
	glVertexAttribPointer(g_sp_m.aposition, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, (void*) 0);

	glBindBuffer(GL_ARRAY_BUFFER, bg_cols[selected_mood]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fs_quad_ibo);
	glEnableVertexAttribArray(g_sp_m.atex);
	glVertexAttribPointer(g_sp_m.atex, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*) 0);

	glUniform1f(g_sp_m.pos_x, 0.0);
	glUniform1f(g_sp_m.pos_y, 0.0);

	glUniform1f(g_sp_m.rgb[0], 0.5 * bg->pulse);
	glUniform1f(g_sp_m.rgb[1], 1.0);
	glUniform1f(g_sp_m.rgb[2], 1.0);

	glUniform1f(g_sp_m.alpha, bg->fs->alpha);
	glUniform1f(g_sp_m.scale, 1.0);

	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

}



void draw_touch_ripples() {
	int i;

	pthread_mutex_lock(&mutex);

	for (i=0; i<TOUCH_SHAPES_MAX; i++) {

		touch_shape* ts = touch_ripples + (touch_shape_draw_order[i]);

		if (ts->is_alive) {
			glUniform1f(g_sp_m.pos_x, ts->pos_x);
			glUniform1f(g_sp_m.pos_y, ts->pos_y);
			glUniform1f(g_sp_m.rgb[0], 1.0);
			glUniform1f(g_sp_m.rgb[1], 1.0);
			glUniform1f(g_sp_m.rgb[2], 1.0);
//			ts->scale += (float)frame_delta * 0.0000003F;
			ts->scale += (float)frame_delta * 0.0000022F;

			if (ts->fading_in) {
				ts->alpha += (float)frame_delta *  0.0000036F;//(float)(SEC_IN_US/25);
				if (ts->alpha >= (ts->alpha_max * 0.95)) ts->fading_in = FALSE;
			}

			if (!ts->fading_in) {

				ts->alpha *= 0.94F;
//				ts->alpha *= (frame_delta_ratio * 0.98);
				if (ts->alpha < 0.005) ts->is_alive = FALSE;
			}

			glUniform1f(g_sp_m.alpha, ts->alpha);
			glUniform1f(g_sp_m.scale, ts->scale);

			glDrawElements(GL_TRIANGLE_FAN, CIRCLE_SEGMENTS + 2, GL_UNSIGNED_SHORT, 0);

		}
	}
	pthread_mutex_unlock(&mutex);
}



void draw_touch_shapes() {
	int i;

	pthread_mutex_lock(&mutex);

	for (i=0; i<TOUCH_SHAPES_MAX; i++) {

		touch_shape* ts = touch_shapes + (touch_shape_draw_order[i]);

		if (ts->is_alive) {


			glUniform1f(g_sp_m.pos_x, ts->pos_x);
			glUniform1f(g_sp_m.pos_y, ts->pos_y);

			glUniform1f(g_sp_m.rgb[0], ts->rgb[0]);
			glUniform1f(g_sp_m.rgb[1], ts->rgb[1]);
			glUniform1f(g_sp_m.rgb[2], ts->rgb[2]);

			ts->scale -= (float)frame_delta * 0.0000001F;

			if (ts->fading_in) {


				ts->alpha += (float)frame_delta *  0.0000036F;//(float)(SEC_IN_US/25);
				if (ts->alpha >= ts->alpha_max) ts->fading_in = FALSE;
			}

			if (!ts->fading_in) {
				ts->alpha -= (float)frame_delta *  0.000000205F;//(float)(SEC_IN_US/25);
				if (ts->alpha <= 0) ts->is_alive = FALSE;
			}

			glUniform1f(g_sp_m.alpha, ts->alpha);
			glUniform1f(g_sp_m.scale, ts->scale);

			glDrawElements(GL_TRIANGLE_FAN, CIRCLE_SEGMENTS + 2, GL_UNSIGNED_SHORT, 0);

		}
	}
	pthread_mutex_unlock(&mutex);
}


















void kill_all_touch_shapes() {

	int i;
	for (i=0; i<TOUCH_SHAPES_MAX; i++) {
//		touch_shape* ts = touch_shapes + (touch_shape_draw_order[i]);
		touch_shape* ts = touch_shapes + i;
		ts->is_alive = FALSE;


		LOGD ("kill_all_touch_shapes", "kill_all_touch_shapes i: %d", i);
	}
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
		solid_circle_vertex[i + 1].x = r * g_sc.hw_ratio * sin(2.0 * PI * rate);
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


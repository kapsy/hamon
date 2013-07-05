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
  "attribute vec3  aPosition;\n"
  "attribute vec2  aTex;     \n"
  "varying   vec2  vTex;\n"
  "uniform   float uFrame;\n"
  "void main()        \n"
  "{                  \n"
  "  vTex = aTex;     \n"
  "  gl_Position = vec4(aPosition.x - uFrame, aPosition.y + uFrame, 0, 1);\n"
  "}                  \n";

const char fShaderSrc[] =
  "precision mediump float;\n"\
  "varying   vec2  vTex;\n"
  "void main()        \n"
  "{                  \n"
  "  gl_FragColor = vec4(vTex.y, vTex.x, 0.5, 1.0);\n"
  "}                  \n";

typedef struct {
  GLint   aPosition;
  GLint   aTex;
  GLint   uFrame;
} ShaderParams;

typedef struct {
    GLfloat x, y, z;
    GLfloat u, v;
} VertexType;

VertexType vObj[] = {
  {.x = -0.5f, .y = -0.5f, .z = 0.0f, .u = 0.0f, .v = 1.0f},
  {.x =  0.5f, .y = -0.5f, .z = 0.0f, .u = 1.0f, .v = 1.0f},
  {.x =  0.0f, .y =  0.5f, .z = 0.0f, .u = 0.5f, .v = 0.0f},
};

unsigned short iObj[] = {
  0, 1, 2
};

ShaderParams    g_sp;
ScreenSettings  g_sc;

GLuint g_vbo;
GLuint g_ibo;
GLuint g_program;

unsigned int frames = 0;

//const char gVertexShader[] = "attribute vec4 vPosition;\n"
//		"void main() {\n"
//		"  gl_Position = vPosition;\n"
//		"}\n";
//
//const char gFragmentShader[] = "precision mediump float;\n"
//		"void main() {\n"
//		"  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
//		"}\n";





EGLBoolean pi_SurfaceCreate(ANativeWindow* nw)
{

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
  if (g_sc.display == EGL_NO_DISPLAY) return EGL_FALSE;

  if (!eglInitialize(g_sc.display, &g_sc.majorVersion, &g_sc.minorVersion))
    return EGL_FALSE;

  if (!eglChooseConfig(g_sc.display, attrib, &config, 1, &numConfigs))
    return EGL_FALSE;


	EGLint format;
	eglGetConfigAttrib(g_sc.display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(nw, 0, 0, format);


	g_sc.surface = eglCreateWindowSurface(g_sc.display, config, nw, NULL);
  if (g_sc.surface == EGL_NO_SURFACE) return EGL_FALSE;


  g_sc.context = eglCreateContext(g_sc.display, config, EGL_NO_CONTEXT, context);
  if (g_sc.context == EGL_NO_CONTEXT) return EGL_FALSE;

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
	LOGD("pi_create_buffer", "pi_create_buffer() finished");
}



void pi_draw() {




//	LOGD("pi_draw", "pi_draw() called");
//	while (frames < 600) {


//		LOGD("pi_draw", "sc: %d", sc);
//		LOGD("pi_draw", "&sc->display): %d", &sc->display);
//		LOGD("pi_draw", "&sc->surface): %d", &sc->surface);
//		LOGD("pi_draw", "sc->surface): %d", sc->width);
//		LOGD("pi_draw", "sc->surface): %d", sc->height);


		glViewport(0, 0, g_sc.width, g_sc.height);
//		LOGD("pi_draw", "glViewport() called");
		glClear(GL_COLOR_BUFFER_BIT);
//		LOGD("pi_draw", "glClear() called");
		glUniform1f(g_sp.uFrame, (float) (frames % 240) / 150.0 - 0.8);
//		LOGD("pi_draw", "glUniform1f() called");




		// 使用するシェーダを指定
		glUseProgram(g_program);
//		LOGD("pi_draw", "glUseProgram() called");
		// 有効にするバッファオブジェクトを指定
		glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
//		LOGD("pi_draw", "glBindBuffer() called");
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
//		LOGD("pi_draw", "glBindBuffer() called");
		// シェーダのアトリビュート変数をアクセス可能にする
		glEnableVertexAttribArray(g_sp.aPosition);
//		LOGD("pi_draw", "glEnableVertexAttribArray() called");
		glEnableVertexAttribArray(g_sp.aTex);
//		LOGD("pi_draw", "glEnableVertexAttribArray() called");


		// 頂点情報のサイズ、オフセットを指定
		glVertexAttribPointer(g_sp.aPosition, 3, GL_FLOAT, GL_FALSE, 20, (void*) 0);
//		LOGD("pi_draw", "glVertexAttribPointer() called");
		glVertexAttribPointer(g_sp.aTex, 2, GL_FLOAT, GL_FALSE, 20, (void*) 12);
//		LOGD("pi_draw", "glVertexAttribPointer() called");





		glEnableVertexAttribArray(0);
//		LOGD("pi_draw", "glEnableVertexAttribArray() called");
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
//		LOGD("pi_draw", "glDrawElements() called");




		eglSwapBuffers(g_sc.display, g_sc.surface);
//		LOGD("pi_draw", "eglSwapBuffers() called");
		frames++;
//	}





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

int init_cmds(int* animating) { // FIXME

	LOGD("init_cmds", "init_cmds() called");

	frames = 0;
	  int res;

	  res = InitShaders(&g_program, vShaderSrc, fShaderSrc);
	  if (!res) return 0;

	  pi_create_buffer();

	  g_sp.aPosition = glGetAttribLocation(g_program, "aPosition");
	  g_sp.aTex = glGetAttribLocation(g_program, "aTex");
	  g_sp.uFrame  = glGetUniformLocation(g_program, "uFrame");

	  glClearColor(0.0f, 0.3f, 0.0f, 0.5f);

//	  *animating = TRUE;

		LOGD("init_cmds", "init_cmds() finished");
	  return TRUE;
}

//int main ( int argc, char *argv[] )
//{
//  unsigned int frames = 0;
//  int res;
//
////  bcm_host_init();
////  res = WinCreate(&g_sc);
////  if (!res) return 0;
//  res = SurfaceCreate(&g_sc);
//  if (!res) return 0;
//  res = InitShaders(&g_program, vShaderSrc, fShaderSrc);
//  if (!res) return 0;
//
//  createBuffer();
//
//  g_sp.aPosition = glGetAttribLocation(g_program, "aPosition");
//  g_sp.aTex = glGetAttribLocation(g_program, "aTex");
//  g_sp.uFrame  = glGetUniformLocation(g_program, "uFrame");
//
//  glClearColor(0.0f, 0.3f, 0.0f, 0.5f);
//
//  /* 600frame / 60fps = 10sec */
//  while(frames < 600) {
//    glViewport(0, 0, g_sc.width, g_sc.height);
//    glClear(GL_COLOR_BUFFER_BIT);
//    glUniform1f(g_sp.uFrame, (float)(frames % 240) / 150.0 - 0.8);
//    Draw();
//    eglSwapBuffers(g_sc.display, g_sc.surface);
//    frames++;
//  }
//  return 0;
//}


/*


void android_main(android_app* state) {
	app_dummy();

	engine e;
	state->userData = &e;
	state->onAppCmd = [](android_app* app, int32_t cmd) {
		auto e = static_cast<engine*>(app->userData);
		switch (cmd) {
			case APP_CMD_INIT_WINDOW:
			init(e);
			draw(e);
			break;
		}
	};
	e.app = state;

	while (1) {
		int ident, events;
		android_poll_source* source;
		while ((ident = ALooper_pollAll(0, NULL, &events, (void**) &source))
				>= 0) {
			if (source != NULL) {
				source->process(state, source);
			}
			if (state->destroyRequested != 0) {
				return;
			}
		}
	}
}

*/

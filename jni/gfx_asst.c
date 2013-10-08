//#include <jni.h>
//#include <EGL/egl.h>
//#include <GLES/gl.h>
//#include <GLES2/gl2.h>
//#include <android/log.h>
//#include <android_native_app_glue.h>
#include <android/bitmap.h>
#include <stdio.h>
#include <math.h>


#include "hon_type.h"
#include "gfx_asst.h"

//#define SEC_IN_NS 1000000000
//#define SEC_IN_US 1000000
//
////#define LOG_TAG ("gles2_py_texture")
//
//#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, __VA_ARGS__)
//#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, __VA_ARGS__)
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, __VA_ARGS__)
//#define LOGDw(...) __android_log_write(ANDROID_LOG_DEBUG, __VA_ARGS__)

//struct engine {
//	struct android_app* app;
//	EGLDisplay display;
//	EGLSurface surface;
//	int animating;
//};

//#3) When you render the fullscreen quad, you definately don't want to use the same projection matrix.
//The easiest is to just use no matrix at all (define input coordinates in normalized device coordinates).
//		Just draw your quad with vertices (+/-1, +/-1, 0) and that will align with the fullscreen
//		without any transform required.
//typedef struct {
//	EGLNativeWindowType  nativeWin;
//	EGLDisplay  display;
//	EGLContext  context;
//	EGLSurface  surface;
//	EGLint      majorVersion;
//	EGLint      minorVersion;
//	float 		width;
//	float		height;
//	float		display_ratio;
//} ScreenSettings;


/*const char vShaderSrc[] =
		"attribute vec4 vPosition;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vPosition;\n"
		"}\n";

const char fShaderSrc[] =
	"precision mediump float;\n"
	"uniform vec2 uSize;\n"
	"uniform sampler2D sTexture;\n"
	"uniform vec2 display;\n"
	"uniform float bitmap_ratio;\n"

	"void main()\n"
	"{\n"		"    gl_FragColor = texture2D(sTexture, (vec2(gl_FragCoord) + vec2(((display.y * bitmap_ratio) - display.x) / 2.0, 0.0)) / "
		"vec2(display.y * (bitmap_ratio), display.y)   );\n"

	"}\n";*/



//typedef struct {
//	GLint		position;
//	GLint 	tex;
////	GLint		time;
////	GLint		resolution;
////	GLint 	mouse;
//	GLint display;
//	GLint		bitmap_ratio;
//} shader_params_t;

//typedef struct {
//	GLfloat x, y, z;
//	GLfloat u, v;
//} vertex;

//#pragma pack(push,1)
//
//typedef struct tagBITMAPFILEHEADER {
//    unsigned short bfType;
//    unsigned int   bfSize;
//    unsigned short bfReserved1;
//    unsigned short bfReserved2;
//    unsigned int   bfOffBits;
//} BITMAPFILEHEADER;
//
//typedef struct tagBITMAPINFOHEADER{
//    unsigned int   biSize;
//    int            biWidth;
//    int            biHeight;
//    unsigned short biPlanes;
//    unsigned short biBitCount;
//    unsigned int   biCompression;
//    unsigned int   biSizeImage;
//    int            biXPixPerMeter;
//    int            biYPixPerMeter;
//    unsigned int   biClrUsed;
//    unsigned int   biClrImporant;
//} BITMAPINFOHEADER;
//
//#pragma pack(pop)

//typedef struct {
//    int  fsize;
//    unsigned char *pdata;    // 画像ファイルのピクセルデータ
//    unsigned char *TexData;  // テクスチャのピクセルデータ
//    BITMAPFILEHEADER *bmpheader;
//    BITMAPINFOHEADER *bmpinfo;
//    int  BmpSize;
//    int  BmpOffBits;
//    int  BmpWidth;           // 画像の幅
//    int  BmpHeight;          // 画像の高さ（負ならば反転）
//
//    float bitmap_ratio;
//
//    int  BmpBit;             // 画像のビット深度
//    int  BmpLine;
//    int  initial_alpha;
//    GLuint  texname;
//} TexureType;

/*
vertex bg_quad[] = {
	{-1.0f, -1.0f, 0.0f, 0.0, 0.0},
	{1.0f, -1.0f, 0.0f, 1.0f, 0.0f},
	{1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
	{-1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
};

unsigned short iObj[] = {
  0, 1, 3, 2
};

ScreenSettings  g_sc;

//shader_params_k sp_k;
shader_params_t    g_sp;
TexureType      g_tt;

GLuint g_vbo;
GLuint g_ibo;
GLuint g_program;
//GLuint g_program_2;
*/

//#define MAXSIZE  1024 * 1024 * 4
//#define MAXSIZE  4096 * 4096 * 4
//
//static unsigned char g_bmpbuffer[MAXSIZE];


int load_bitmap(char *filename, void *buffer)
{
	FILE *fp;
	long fsize;
	int n_read = 0;

	fp = fopen(filename, "rb");
	if (fp > 0) {
		fseek(fp, 0L, SEEK_END);
		fsize = ftell(fp);
		if (fsize >= MAXSIZE) {
			LOGD("LoadFile", "(fsize >= MAXSIZE) ");
			fclose(fp);
			return -1;
		}
		fseek(fp, 0L, SEEK_SET);
		n_read = fread((void *)buffer, 1, fsize, fp);
		fclose(fp);

		//    int count =0;
		//    unsigned int* header = buffer + 14;
		//   	LOGD("LoadFile", "*hd: %d", *header);
		//   	// やっぱ問題は決してLoadFileではない
		//    for (count =0; count < 1200; count++) {
		//    	LOGD("LoadFile", "\t%d: \t%x", count, g_bmpbuffer[count]);
		//    }

		return n_read;
	}
	return -1;
}

int check_bitmap(TexureType *tt, void* buffer)
{
	tt->bmpheader = (BITMAPFILEHEADER *)buffer;
	// bmp フォーマットのシグネチャのチェック
	if (tt->bmpheader->bfType != ('B' | ('M' << 8))) {
		LOGD("bmpCheck", "(tt->bmpheader->bfType != ('B' | ('M' << 8)))");
		return 0;
	}
	tt->BmpSize = tt->bmpheader->bfSize;
	tt->BmpOffBits = tt->bmpheader->bfOffBits;
	tt->bmpinfo = (BITMAPINFOHEADER *)(buffer + sizeof(BITMAPFILEHEADER));


	LOGD("bmpCheck", "(tt->bmpinfo->biSize: %d", tt->bmpinfo->biSize);
	LOGD("bmpCheck", "(tt->bmpinfo->biWidth: %d", tt->bmpinfo->biWidth);
	LOGD("bmpCheck", "(tt->bmpinfo->biHeight: %d", tt->bmpinfo->biHeight);
	LOGD("bmpCheck", "(tt->bmpinfo->biPlanes: %d", tt->bmpinfo->biPlanes);
	LOGD("bmpCheck", "(tt->bmpinfo->biBitCount: %d", tt->bmpinfo->biBitCount);
	LOGD("bmpCheck", "(tt->bmpinfo->biCompression: %d", tt->bmpinfo->biCompression);
	LOGD("bmpCheck", "(tt->bmpinfo->biSizeImage: %d", tt->bmpinfo->biSizeImage);

	LOGD("bmpCheck", "(tt->bmpinfo->biXPixPerMeter: %d", tt->bmpinfo->biXPixPerMeter);
	LOGD("bmpCheck", "(tt->bmpinfo->biYPixPerMeter: %d", tt->bmpinfo->biYPixPerMeter);
	LOGD("bmpCheck", "(tt->bmpinfo->biClrUsed: %d", tt->bmpinfo->biClrUsed);
	LOGD("bmpCheck", "(tt->bmpinfo->biClrImporant: %d", tt->bmpinfo->biClrImporant);

	// bmp フォーマットの形式チェック
	if (tt->bmpinfo->biSize == 40) {
		LOGD("bmpCheck", "(tt->bmpinfo->biSize == 40) ");
		tt->BmpWidth = tt->bmpinfo->biWidth;
		tt->BmpHeight = tt->bmpinfo->biHeight;
		tt->BmpBit = tt->bmpinfo->biBitCount;
		tt->BmpLine = (tt->BmpWidth * tt->BmpBit) / 8;
		LOGD("bmpCheck", "tt->BmpLine %d", tt->BmpLine);
		tt->BmpLine = (tt->BmpLine + 3) / 4 * 4;
		LOGD("bmpCheck", "tt->BmpLine %d", tt->BmpLine);
		// 画像ファイルのピクセルデータの先頭アドレス
		tt->pdata = buffer + tt->bmpheader->bfOffBits;

		int* w = &tt->BmpWidth;
		int* h = &tt->BmpHeight;

		if(*w > *h) {
			tt->bitmap_ratio = (float)*w / (float)*h;
		}

		LOGD("bmpCheck", "return 1");
		return 1;
	} else {
		LOGD("bmpCheck", "(tt->bmpinfo->biSize != 40) ");
		return 0;
	}
}


void make_texture(TexureType *tt, int alpha)
{
	int color, x, y;
//	tt->initial_alpha = alpha;

//  09-20 18:53:19.956: D/makeTexture(23694): malloc(), tex_malloc: 1459280
	size_t tex_malloc = sizeof(char) * tt->BmpWidth * tt->BmpHeight * 4;

	tt->TexData = malloc(tex_malloc);
	LOGD("makeTexture", "malloc(), tex_malloc: %d", tex_malloc);
	if (tt->BmpHeight < 0) tt->BmpHeight = -tt->BmpHeight;
	LOGD("makeTexture", "tt->BmpHeight: %d",  tt->BmpHeight);
	LOGD("makeTexture", "tt->BmpWidth: %d",  tt->BmpWidth);

//  09-18 21:58:11.846: D/makeTexture(1603): === y: 258 ===
//  09-18 21:58:11.846: D/makeTexture(1603): x: 0
//  09-18 21:58:11.846: A/libc(1603): Fatal signal 11 (SIGSEGV) at 0x4a25f00a (code=1)

//  09-18 22:21:20.715: D/makeTexture(3263): offset: 198141
//  09-18 22:21:20.715: D/makeTexture(3263): offset: 198144
//  09-18 22:21:20.715: A/libc(3263): Fatal signal 11 (SIGSEGV) at 0x4a25f00a (code=1)

  for (y=0; y < tt->BmpHeight; y++) {
    for (x=0; x < tt->BmpWidth; x++) {

      int bitdata;
      int offset, n;

//	09-19 18:30:33.408: D/makeTexture(9661): tt->BmpBit: 24
//	09-19 18:30:33.408: D/makeTexture(9661): tt->BmpLine: 768
//	LOGD("makeTexture", "tt->BmpLine: %d", tt->BmpLine);
//	LOGD("makeTexture", "tt->BmpBit: %d", tt->BmpBit);

      offset = (y * tt->BmpLine) + x * (tt->BmpBit / 8);

//		n = ((y*tt->BmpHeight) + x) * 4;
      n = ((y*tt->BmpWidth) + x) * 4;

      switch(tt->BmpBit) {
        case 32 :
//            LOGD("makeTexture", "case 32");
            break;
        case 24 :
//            LOGD("makeTexture", "case 24");
          tt->TexData[n+2] = tt->pdata[offset];    // B
          tt->TexData[n+1] = tt->pdata[offset+1];  // G
          tt->TexData[n+0] = tt->pdata[offset+2];  // R
          tt->TexData[n+3] = alpha;  // A

//          if (y < 3) {
//				LOGD("makeTexture", "y: %d, x: %d", y, x);
//				LOGD("makeTexture", "tt->TexData[n+0]: %x", tt->TexData[n+0]);
//				LOGD("makeTexture", "tt->TexData[n+1]: %x", tt->TexData[n+1]);
//				LOGD("makeTexture", "tt->TexData[n+2]: %x", tt->TexData[n+2]);
//				LOGD("makeTexture", "tt->TexData[n+3]: %x", tt->TexData[n+3]);
//          }
          break;
//        case 16 :
////            LOGD("makeTexture", "case 16");
//          color = tt->pdata[offset]|(tt->pdata[offset+1]<<8);
//          tt->TexData[n+2] = (color & 0x1F) << 3;  // B
//          tt->TexData[n+1] = (color & 0x3E0) >> 2; // G
//          tt->TexData[n+0] = (color & 0x7C00) >> 7;// R
//          tt->TexData[n+3] = alpha;  // A
//          break;
//        case 1 :
////            LOGD("makeTexture", "case 1");
//          bitdata = tt->pdata[offset] & (0x80 >> (x % 8));
//          color = bitdata?255:0;
//          tt->TexData[n+2] = color;  // B
//          tt->TexData[n+1] = color;  // G
//          tt->TexData[n+0] = color;  // R
//          tt->TexData[n+3] = alpha;  // A
//          break;
      }
    }
  }
//  int i;
//  for (i = 0; i < 4400; i++) {
//	  LOGD("makeTexture", "tt->TexData %d: %x", i, *(tt->TexData + i));
//  }
}


//int createTexture(TexureType *tt)
//{
//  glGenTextures(1, &tt->texname);
//  glBindTexture(GL_TEXTURE_2D, tt->texname);
//
//  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//
//  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tt->BmpWidth, tt->BmpHeight, 0,
//                GL_RGBA, GL_UNSIGNED_BYTE, tt->TexData);
//  return 1;
//}

//GLuint LoadShader(GLenum type, const char *shaderSource)
//{
//	GLuint shader;
//	GLint compiled;
//
//	shader = glCreateShader(type);
//	if (shader == 0) return 0;
//
//	glShaderSource(shader, 1, &shaderSource, NULL);
//	glCompileShader(shader);
//	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
//
//	if (!compiled) { // compile error
//		GLint infoLen = 0;
//		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
//
//		if (infoLen > 0) {
//			void* infoLog = malloc(sizeof(char) * infoLen);
//			glGetShaderInfoLog(shader, infoLen, NULL, (char*)infoLog);
//			LOGE("Error compiling shader:\n%s\n", infoLog);
//			//      LOGE("Error compiling shader");
//			free(infoLog);
//		}
//		glDeleteShader(shader);
//		return 0;
//	}
//	return shader;
//}

//int InitShaders(GLuint *program, char const *vShSrc, char const *fShSrc)
//{
//	GLuint vShader;
//	GLuint fShader;
//	GLint  linked;
//	GLuint prog;
//
//	vShader = LoadShader(GL_VERTEX_SHADER, vShSrc);
//	fShader = LoadShader(GL_FRAGMENT_SHADER, fShSrc);
//
//	prog = glCreateProgram();
//	if (prog == 0) return 0;
//
//	glAttachShader(prog, vShader);
//	glAttachShader(prog, fShader);
//	glLinkProgram(prog);
//	glGetProgramiv (prog, GL_LINK_STATUS, &linked);
//
//	if (!linked) { // error
//		GLint infoLen = 0;
//		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLen);
//		if (infoLen > 0) {
//			void* infoLog = malloc(sizeof(char) * infoLen);
//			glGetProgramInfoLog(prog, infoLen, NULL, (char*)infoLog);
//			LOGE("Error linking program:\n%s\n", infoLog);
//			free ( infoLog );
//		}
//		glDeleteProgram (prog);
//		return GL_FALSE;
//	}
//	glDeleteShader(vShader);
//	glDeleteShader(fShader);
//
//	*program = prog;
//	return GL_TRUE;
//}

//void createBuffer()
//{
//	glGenBuffers(1, &g_vbo);
//	// vertex buffer
//	glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(bg_quad), bg_quad, GL_STATIC_DRAW);
//
//	// index buffer
//	glGenBuffers(1, &g_ibo);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iObj), iObj, GL_STATIC_DRAW);
//}
//
//struct timezone tzp;
//struct timeval get_time;
//void get_time_long(unsigned long* t) {
//	gettimeofday(&get_time, &tzp);
//	*t = (get_time.tv_sec * SEC_IN_US) + get_time.tv_usec;
//}
//
//float get_time_s() {
//	gettimeofday(&get_time, &tzp	);
//	return (float)get_time.tv_sec;
//}


//float t = 0.0F;
//void Draw() {
//
//	glUseProgram(g_program);
//
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, g_tt.texname);
//
//	glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
//	glEnableVertexAttribArray(g_sp.position);
//	//	  glEnableVertexAttribArray(g_sp.tex);
//	glVertexAttribPointer(g_sp.position, 3, GL_FLOAT, GL_FALSE, 20, (void*)0);
//	//	  glVertexAttribPointer(g_sp.tex, 2, GL_FLOAT, GL_FALSE, 20, (void*)12);
//	glEnableVertexAttribArray(0);
//
//	glUniform2f(g_sp.display, g_sc.width, g_sc.height);
//	glUniform1f(g_sp.bitmap_ratio, g_tt.bitmap_ratio);
//
//	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
//
//	glBindTexture(GL_TEXTURE_2D,0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//}

//EGLBoolean pi_SurfaceCreate(ANativeWindow* nw) {
//
//	LOGD("pi_SurfaceCreate", "pi_SurfaceCreate() called");
//// ScreenSettings *sc = &g_sc;
//
//    memset(&g_sc, 0, sizeof(ScreenSettings));
////    LOGD("pi_SurfaceCreate", "memset");
//
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
//	EGLint context[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
//	EGLint numConfigs;
//	EGLConfig config;
//
//	g_sc.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
//	if (g_sc.display == EGL_NO_DISPLAY)
//		return EGL_FALSE;
//
//	if (!eglInitialize(g_sc.display, &g_sc.majorVersion, &g_sc.minorVersion))
//		return EGL_FALSE;
//
//	if (!eglChooseConfig(g_sc.display, attrib, &config, 1, &numConfigs))
//		return EGL_FALSE;
//
//	EGLint format;
//	eglGetConfigAttrib(g_sc.display, config, EGL_NATIVE_VISUAL_ID, &format);
//	ANativeWindow_setBuffersGeometry(nw, 0, 0, format);
//
//	g_sc.surface = eglCreateWindowSurface(g_sc.display, config, nw, NULL);
//	if (g_sc.surface == EGL_NO_SURFACE)
//		return EGL_FALSE;
//
//	g_sc.context = eglCreateContext(g_sc.display, config, EGL_NO_CONTEXT, context);
//	if (g_sc.context == EGL_NO_CONTEXT)
//		return EGL_FALSE;
//
//	if (!eglMakeCurrent(g_sc.display, g_sc.surface, g_sc.surface, g_sc.context))
//		return EGL_FALSE;
//
//	EGLint w, h;
//
//	eglQuerySurface(g_sc.display, g_sc.surface, EGL_WIDTH, &w);
//	eglQuerySurface(g_sc.display, g_sc.surface, EGL_HEIGHT, &h);
//
//	g_sc.width = (float)w;
//	g_sc.height = (float)h;
//
//  	LOGD("pi_SurfaceCreate", "g_sc.width: %f", g_sc.width);
//  	LOGD("pi_SurfaceCreate", "g_sc.height: %f", g_sc.height);
//
//  	return EGL_TRUE;
//}

//void init_pi(struct engine* e) {
//
//	unsigned int frames = 0;
//	int res;
//	//	Mat4 viewMat;
//	//	Mat4 rotMat;
//	//	Mat4 modelMat;
//	float aspect;
//
//	//	res = SurfaceCreate(&g_sc);
//	res = pi_SurfaceCreate(e->app->window);
//	if (!res)
//	return;
//
//	res = InitShaders(&g_program, vShaderSrc, fShaderSrc);
//	if (!res) return;
//
//	createBuffer();
//
//	//	glUseProgram(g_program);
//	g_sp.display = glGetUniformLocation(g_program, "display");
//	g_sp.bitmap_ratio = glGetUniformLocation(g_program, "bitmap_ratio");
//
//	g_sp.position = glGetAttribLocation(g_program, "vPosition");
//	g_sp.tex = glGetAttribLocation(g_program, "aTex");
//
//	int size;
//
//	size = LoadFile("/mnt/sdcard/Android/data/nz.kapsy.gles2_py_texture/files/splash_test_001_800x400.bmp", (void *)g_bmpbuffer);
//	LOGD("init_pi", "LoadFile %d", size);
//
//	bmpCheck(&g_tt, (void *)&g_bmpbuffer);
//	makeTexture(&g_tt, 255);
//	createTexture(&g_tt);
//}



///**
// * Process the next main command.
// */
//static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
//    struct engine* e = (struct engine*)app->userData;
//
//    switch (cmd) {
//        case APP_CMD_SAVE_STATE:
//        	LOGD("engine_handle_cmd", "APP_CMD_SAVE_STATE");
//            break;
//
//        case APP_CMD_INIT_WINDOW:
//        	LOGD("engine_handle_cmd", "APP_CMD_INIT_WINDOW");
//
//				init_pi(e);
//				e->animating = 1;
//            break;
//        case APP_CMD_START:
//        	LOGD("engine_handle_cmd", "APP_CMD_START");
//
////			ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
//
//        	break;
//        case APP_CMD_TERM_WINDOW:
//        	LOGD("engine_handle_cmd", "APP_CMD_TERM_WINDOW");
//            break;
//        case APP_CMD_GAINED_FOCUS:
//        	LOGD("engine_handle_cmd", "APP_CMD_GAINED_FOCUS");
//
//            break;
//        case APP_CMD_CONFIG_CHANGED:
//        	LOGD("engine_handle_cmd", "APP_CMD_CONFIG_CHANGED");
//        	break;
//        case APP_CMD_LOST_FOCUS:
//        	LOGD("engine_handle_cmd", "APP_CMD_LOST_FOCUS");
//        	break;
//        case APP_CMD_STOP:
//        	LOGD("engine_handle_cmd", "APP_CMD_STOP");
//        	break;
//        case APP_CMD_DESTROY:
//        	LOGD("engine_handle_cmd", "APP_CMD_DESTROY");
//        	break;
//        case APP_CMD_WINDOW_RESIZED:
//        	LOGD("engine_handle_cmd", "APP_CMD_WINDOW_RESIZED");
//        	break;
//    }
//}

//void android_main(struct android_app* state) {
//	app_dummy();
//
//	struct engine e;
//	state->userData = &e;
//
//	e.app = state;
//
//    // Make sure glue isn't stripped.
//    app_dummy();
//    memset(&e, 0, sizeof(struct engine));
//    state->userData = &e;
//    state->onAppCmd = engine_handle_cmd;
////    state->onInputEvent = engine_handle_input;
//    e.app = state;
//	LOGD("call_order", "android_main e.app = state");
//
//	while (1) {
//
//
//		int ident, events;
//		struct android_poll_source* source;
//		while ((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
//			if (source != NULL) {
//				source->process(state, source);
//			}
//			if (state->destroyRequested != 0) {
//				return;
//			}
//		}
//
//		if(e.animating) {
//
//			glViewport(0, 0, g_sc.width, g_sc.height);
////			glViewport(0, 0, g_sc.width, 232);
//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//			Draw();
//			eglSwapBuffers(g_sc.display, g_sc.surface);
//
//		}
//	}
//}

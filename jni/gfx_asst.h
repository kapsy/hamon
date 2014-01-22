// gfx_asst.h

#ifndef GFX_ASST_H_
#define GFX_ASST_H_

#include <GLES/gl.h>
#include <android/asset_manager.h>

#define MAXSIZE  1024 * 1024 * 4

static unsigned char g_bmpbuffer[MAXSIZE];

#pragma pack(push,1)

struct bitmap_file_header {

    unsigned short bfType;
    unsigned int   bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int   bfOffBits;

};

struct bitmap_info_header {

    unsigned int   biSize;
    int            biWidth;
    int            biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int   biCompression;
    unsigned int   biSizeImage;
    int            biXPixPerMeter;
    int            biYPixPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImporant;

};

#pragma pack(pop)

struct texture_type{

    int  fsize;
    unsigned char *pdata;    // 画像ファイルのピクセルデータ
    unsigned char *TexData;  // テクスチャのピクセルデータ
    struct bitmap_file_header *bmpheader;
    struct bitmap_info_header *bmpinfo;
    int  BmpSize;
    int  BmpOffBits;
    int  BmpWidth;           // 画像の幅
    int  BmpHeight;          // 画像の高さ（負ならば反転）

    float bitmap_ratio;
    float alpha;
    int  BmpBit;             // 画像のビット深度
    int  BmpLine;
    GLuint  texname;

};

struct texture_file{

	int size;
	char* file_name;
	GLint param;
	unsigned char* buffer;
	struct texture_type tt;

};

extern struct texture_file textures[];
extern int sizeof_textures_elements;
extern int sizeof_textures;

void setup_texture(struct texture_file *tf, float init_alpha, AAssetManager* am);


#endif /* GFX_ASST_H_ */

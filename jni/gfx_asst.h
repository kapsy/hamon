/*
 * gfx_asst.h
 *
 *  Created on: 2013/09/30
 *      Author: Michael
 */

#ifndef GFX_ASST_H_
#define GFX_ASST_H_



#include <GLES/gl.h>




#define MAXSIZE  1024 * 1024 * 4

static unsigned char g_bmpbuffer[MAXSIZE];





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
//
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
//    float alpha;
//
//    int  BmpBit;             // 画像のビット深度
//    int  BmpLine;
////    int  initial_alpha;
//    GLuint  texname;
//} texture_type;
//
//
//typedef struct {
//
//	int size;
//	char* path;
//	unsigned char* buffer;
//	texture_type tt;
//} texture_file;


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
//    int  initial_alpha;
    GLuint  texname;
//	GLint param;
};

//struct texture_type;

struct texture_file{

	int size;
	char* path;
	GLint param;
	unsigned char* buffer;
	struct texture_type tt;

};



extern struct texture_file textures[];
extern int sizeof_textures_elements;
extern int sizeof_textures;

void setup_texture(struct texture_file *tf, float init_alpha);
int load_bitmap(char *filename, void *buffer);
int check_bitmap(struct texture_type *tt, void* buffer);
void make_texture(struct texture_type *tt, int alpha);


#endif /* GFX_ASST_H_ */

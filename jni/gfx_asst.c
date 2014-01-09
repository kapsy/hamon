// gfx_asst.c

#include "common.h"
#include "hon_type.h"
#include "gfx_asst.h"

struct texture_file textures[] = {
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/splash_main_002_480x960.bmp", GL_NEAREST},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/splash_bg_002_480x960.bmp", GL_NEAREST},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/but_C_001_128.bmp", GL_NEAREST},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/but_C_002_128.bmp", GL_NEAREST},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/but_C_003_128.bmp", GL_NEAREST},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/but_Ct_001_128.bmp", GL_NEAREST},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/but_Ct_002_128.bmp", GL_NEAREST},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/but_Ct_003_128.bmp", GL_NEAREST},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/test_tex_01.bmp", GL_LINEAR},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/bubble_001.bmp", GL_LINEAR},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/bubble_002_128.bmp", GL_LINEAR},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/bubble_003_128.bmp", GL_LINEAR},
		{0, "/mnt/sdcard/Android/data/nz.kapsy.hamon/files/bubble_004_128.bmp", GL_LINEAR}
};

int sizeof_textures_elements = sizeof textures / sizeof textures[0];
int sizeof_textures = sizeof textures;

void setup_texture(struct texture_file *tf, float init_alpha) {

	LOGD("setup_texture", "tf->path: %c", tf->path);
//	LOGD("setup_texture", "tf->size: %d", tf->path);

	tf->buffer = (unsigned char*) malloc(MAXSIZE);
	tf->size = load_bitmap(tf->path, (void *) tf->buffer);
	LOGD("setup_texture", "tf->size: %d", tf->size);
	check_bitmap(&tf->tt, (void *) tf->buffer);
	make_texture(&tf->tt, 255);
	create_gl_texture(&tf->tt, tf->param);
	tf->tt.alpha = 0.0;
}

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
			LOGD("load_bitmap", "(fsize >= MAXSIZE) ");
			fclose(fp);
			return -1;
		}
		fseek(fp, 0L, SEEK_SET);
		n_read = fread((void *)buffer, 1, fsize, fp);
		fclose(fp);
		LOGD("load_bitmap", "fread((void *)buffer, 1, fsize, fp)");
		return n_read;
	}
	return -1;
}

int check_bitmap(struct texture_type *tt, void* buffer)
{
	tt->bmpheader = (struct bitmap_file_header *)buffer;
	// bmp フォーマットのシグネチャのチェック
	if (tt->bmpheader->bfType != ('B' | ('M' << 8))) {
		LOGD("bmpCheck", "(tt->bmpheader->bfType != ('B' | ('M' << 8)))");
		return 0;
	}
	tt->BmpSize = tt->bmpheader->bfSize;
	tt->BmpOffBits = tt->bmpheader->bfOffBits;
	tt->bmpinfo = (struct bitmap_info_header *)(buffer + sizeof(struct bitmap_file_header));

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
	if (tt->bmpinfo->biSize == 40 || tt->bmpinfo->biSize == 124) {
		LOGD("bmpCheck", "(tt->bmpinfo->biSize == 124) ");
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
		if (*w > *h) {
			tt->bitmap_ratio = (float) *w / (float) *h;
		}
		LOGD("bmpCheck", "return 1");
		return 1;
	} else {
		LOGD("bmpCheck", "(tt->bmpinfo->biSize != 40) ");
		return 0;
	}
}

void make_texture(struct texture_type *tt, int alpha)
{
	int color, x, y;
	size_t tex_malloc = sizeof(char) * tt->BmpWidth * tt->BmpHeight * 4;

	tt->TexData = malloc(tex_malloc);
	LOGD("makeTexture", "malloc(), tex_malloc: %d", tex_malloc);
	if (tt->BmpHeight < 0) tt->BmpHeight = -tt->BmpHeight;
	LOGD("makeTexture", "tt->BmpHeight: %d",  tt->BmpHeight);
	LOGD("makeTexture", "tt->BmpWidth: %d",  tt->BmpWidth);

  for (y=0; y < tt->BmpHeight; y++) {
    for (x=0; x < tt->BmpWidth; x++) {

      int bitdata;
      int offset, n;
//	09-19 18:30:33.408: D/makeTexture(9661): tt->BmpBit: 24
//	09-19 18:30:33.408: D/makeTexture(9661): tt->BmpLine: 768
//	LOGD("makeTexture", "tt->BmpLine: %d", tt->BmpLine);
//	LOGD("makeTexture", "tt->BmpBit: %d", tt->BmpBit);

      offset = (y * tt->BmpLine) + x * (tt->BmpBit / 8);
      n = ((y*tt->BmpWidth) + x) * 4;

      switch(tt->BmpBit) {

        case 32 :
            tt->TexData[n+0] = tt->pdata[offset+3];  // R
            tt->TexData[n+1] = tt->pdata[offset+2];    // G
            tt->TexData[n+2] = tt->pdata[offset+1];  // B
            tt->TexData[n+3] = tt->pdata[offset];  // A

//			if (y < 1) {
//				LOGD("makeTexture", "y: %d, x: %d", y, x);
//				LOGD("makeTexture", "tt->TexData[n+0]: %x", tt->TexData[n+0]);
//				LOGD("makeTexture", "tt->TexData[n+1]: %x", tt->TexData[n+1]);
//				LOGD("makeTexture", "tt->TexData[n+2]: %x", tt->TexData[n+2]);
//				LOGD("makeTexture", "tt->TexData[n+3]: %x", tt->TexData[n+3]);
//			}
				break;

        case 24 :
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
//          break;
      }
    }
  }

// for validating texture data loaded
//  int i;
//  for (i = 0; i < 4400; i++) {
//	  LOGD("makeTexture", "tt->TexData %d: %x", i, *(tt->TexData + i));
//  }

}

/*
 * Copyright 2016 Joey  <joeyye@foxmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <va/va.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <gui/SurfaceControl.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <WindowSurface.h>

extern "C"{
	#include <libavformat/avformat.h>
	#include <libavutil/pixdesc.h>
	#include <libavcodec/vaapi.h>
}

using namespace android;
struct vaglContext{
	struct vaContext{
		VADisplay display;
		VAConfigID configid;
		VAContextID contextid;
		VAConfigAttrib attribs[1];
		VAProfile *profiles;
		uint32_t num_profiles;
		VASurfaceID *surfaces;
		int numsurfaces;
		int surfacesptr;
		int surfacesshowptr;
	}va;
	struct vaapi_context ffva;
	struct glContext{
		EGLDisplay dpy;
		EGLNativeWindowType window;
		EGLContext context;
		EGLSurface surface;
		EGLClientBuffer *clientBuffer_ptr;
		EGLClientBuffer *clientbuffer;
		GraphicBuffer** gfxBuffer_ptr;
		WindowSurface *windowSurface;
		int maxclientBufffer;
		EGLImageKHR *khrimg;
		GLuint *khrtexid;
		GLuint buffertexid;
		int width;
		int height;
		struct {
			GLuint program;
			GLint  vpos;
			GLint  vtex;
			GLint  vnor;
			GLint  sampler;
			GLint  vtrans;
			GLint  ttrans;
			GLint  ntrans;
		}cube;
		struct {
			GLuint program;
			GLint  vpos;
			GLint  vtex;
			GLint  sampler;
			GLint  vtrans;
			GLint  ttrans;
		}background;
	}gl;
	struct ffmepg{
		AVFormatContext *fmtctx;
		AVStream *stream;
		AVCodecContext *avctx;
		AVFrame *frame;
		AVCodec *codec;
		VASurfaceID surface;
		VARectangle crop_rect;
		bool has_crop_rect;
	}ff;
};

int gfxInit(vaglContext *ctx);
void gfxUninit(vaglContext *ctx);
void renderFrame(vaglContext *ctx,VASurfaceID id=-1);
void setupKHRTexture(vaglContext *ctx);
void putKHRTextureInfo(vaglContext *ctx,void *buffer,int num);
void** CreateVAGfxBuffer(vaglContext *ctx,int width,int height,int num,void **clientbuffer);

int vaInit(vaglContext *ctx);
int vaInitSurface(vaglContext *ctx,VAProfile profile, VAEntrypoint entrypoint);
void vaUninit(vaglContext *ctx);
bool checkVAStatus(VAStatus status,const char *func);
int vaGetShowSurface(vaglContext *ctx);

int openavfile(vaglContext *ctx, const char *filename);
void closeavfile(vaglContext *ctx);
int decode_frame(vaglContext *ctx);

#define U_GEN_CONCAT4(a1, a2, a3, a4)   U_GEN_CONCAT4_I(a1, a2, a3, a4)
#define U_GEN_CONCAT4_I(a1, a2, a3, a4) a1 ## a2 ## a3 ## a4
#define U_GEN_CONCAT3(a1, a2, a3)       U_GEN_CONCAT3_I(a1, a2, a3)
#define U_GEN_CONCAT3_I(a1, a2, a3)     a1 ## a2 ## a3
#define DEFINE_PROFILE(CODEC, FFMPEG_PROFILE, VA_PROFILE)       \
    case U_GEN_CONCAT4(FF_PROFILE_,CODEC,_,FFMPEG_PROFILE):     \
        profile = U_GEN_CONCAT3(VAProfile,CODEC,VA_PROFILE);    \
        break

#define FORMAT_NV12  0x100
#define SURFACE_NUM 9


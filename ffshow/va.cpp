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
#include <stdio.h>
#include <va/va.h>
#include <va/va_android.h>
#include <va/va_drmcommon.h>
#include "data.h"

bool checkVAStatus(VAStatus status,const char *func)
{                                  
	if (status != VA_STATUS_SUCCESS && status != VA_STATUS_ERROR_UNIMPLEMENTED) {                                   
		printf("%s:%s (%d) failed,exit\n", __func__, func, __LINE__); 
		return false;
	}
	return true;
}
static  void *win_display;
static  int android_display=0;

static void *open_display()
{
    return &android_display;
}

static void close_display(void *win_display)
{
    return;
}

int vaInit(vaglContext *ctx)
{
    int major_ver, minor_ver;
    VAStatus status;
	win_display = (void *)open_display();
	ctx->va.display = vaGetDisplay(win_display);
	status = vaInitialize(ctx->va.display, &major_ver, &minor_ver);
	checkVAStatus(status, "vaInitialize");
	return status;
}
int vaGetShowSurface(vaglContext *ctx)
{
	int i=0;
	for(i=0;i<ctx->va.numsurfaces;++i) {
       if(ctx->va.surfaces[i]==ctx->ff.surface) break;
	}
	return i;
}
int vaInitSurface(vaglContext *ctx,VAProfile profile, VAEntrypoint entrypoint)
{
	VAStatus status;
	int i;
	VAConfigAttrib *attrib;
	uint32_t num_attribs = 1;


	attrib = ctx->va.attribs;
	attrib->type = VAConfigAttribRTFormat;
	status = vaGetConfigAttributes(ctx->va.display, profile, entrypoint,ctx->va.attribs, num_attribs);
	if (!checkVAStatus(status, "vaGetConfigAttributes()"))
		return status;

	attrib = &ctx->va.attribs[0];
	if (attrib->value == VA_ATTRIB_NOT_SUPPORTED || !(attrib->value & VA_RT_FORMAT_YUV420)){
		printf("unsupported YUV 4:2:0 format\n");
		return ENOTSUP;
	}
	attrib->value = VA_RT_FORMAT_YUV420;

	ctx->va.num_profiles=3;
	status = vaCreateConfig(ctx->va.display, profile, entrypoint,ctx->va.attribs, num_attribs, &ctx->va.configid);
	if (!checkVAStatus(status, "vaCreateConfig()"))
		return status;


	int width=ctx->ff.avctx->coded_width;
	int height=ctx->ff.avctx->coded_height;
	int stride = (width+0x3f) & (~0x3f);
	ctx->va.numsurfaces=SURFACE_NUM; 
	int mNumSurfaces=ctx->va.numsurfaces;
	VASurfaceAttrib attribs[2];
	VASurfaceAttribExternalBuffers *surfExtBuf = new VASurfaceAttribExternalBuffers;
	void **source_buffer;
	void *clientbuffer;
	source_buffer=CreateVAGfxBuffer(ctx,width, height,mNumSurfaces,&clientbuffer);
	ctx->va.surfaces = new VASurfaceID [mNumSurfaces];
    ctx->va.surfacesptr=-1;
	ctx->va.surfacesshowptr=-1;
	putKHRTextureInfo(ctx,clientbuffer,mNumSurfaces);

	surfExtBuf->buffers= new  unsigned long [mNumSurfaces];
	surfExtBuf->num_buffers = mNumSurfaces;
	surfExtBuf->pixel_format = VA_FOURCC_NV12;
	surfExtBuf->width = width;
	surfExtBuf->height = height;
	surfExtBuf->data_size = width * height * 1.5;
	surfExtBuf->num_planes = 2;
	surfExtBuf->pitches[0] = stride;
	surfExtBuf->pitches[1] = stride;
	surfExtBuf->pitches[2] = 0;
	surfExtBuf->pitches[3] = 0;
	surfExtBuf->offsets[0] = 0;
	surfExtBuf->offsets[1] = stride * height;
	surfExtBuf->offsets[2] = 0;
	surfExtBuf->offsets[3] = 0;
	//surfExtBuf->private_data = (void *)mConfigBuffer.nativeWindow;
	surfExtBuf->flags = VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC;
	surfExtBuf->flags |= VA_SURFACE_EXTBUF_DESC_ENABLE_TILING;

	for (i = 0; i < mNumSurfaces; i++) {
		surfExtBuf->buffers[i] = (unsigned long)source_buffer[i];
	}

	attribs[0].type = (VASurfaceAttribType)VASurfaceAttribMemoryType;
	attribs[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
	attribs[0].value.type = VAGenericValueTypeInteger;
	attribs[0].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC;

	attribs[1].type = (VASurfaceAttribType)VASurfaceAttribExternalBufferDescriptor;
	attribs[1].flags = VA_SURFACE_ATTRIB_SETTABLE;
	attribs[1].value.type = VAGenericValueTypePointer;
	attribs[1].value.value.p = (void *)surfExtBuf;

	status = vaCreateSurfaces(ctx->va.display,VA_RT_FORMAT_YUV420,
		width, height, 
		ctx->va.surfaces,mNumSurfaces,attribs, 2);

    status = vaCreateContext(ctx->va.display, ctx->va.configid,ctx->ff.avctx->coded_width, 
	                         ctx->ff.avctx->coded_height, VA_PROGRESSIVE,ctx->va.surfaces, 
							 ctx->va.numsurfaces, &ctx->va.contextid);

    memset(&ctx->ffva, 0, sizeof(ctx->ffva));
    ctx->ffva.config_id = ctx->va.configid;
    ctx->ffva.context_id = ctx->va.contextid;
    ctx->ffva.display = ctx->va.display;

	return status;
}
void vaUninit(vaglContext *ctx)
{
	vaDestroyConfig(ctx->va.display, ctx->va.configid);
	vaDestroyContext(ctx->va.display, ctx->va.contextid);
    vaDestroySurfaces(ctx->va.display,ctx->va.surfaces,ctx->va.numsurfaces);    
    vaTerminate(ctx->va.display);

    close_display(win_display);
}


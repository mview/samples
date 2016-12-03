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
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <cutils/memory.h>

using namespace android;

static sp<SurfaceComposerClient> client0 = NULL;
static sp<SurfaceControl> surface_ctrl0 = NULL;
static sp<ANativeWindow> anw0 = NULL;

static int create_window(int x, int y, int width, int height)
{
    client0 = new SurfaceComposerClient();
    
    surface_ctrl0 = client0->createSurface(
        String8("Test Surface"),
        width, height,
        PIXEL_FORMAT_RGBX_8888, ISurfaceComposerClient::eOpaque);

    SurfaceComposerClient::openGlobalTransaction();
    surface_ctrl0->setLayer(0x7FFFFFFF);
    surface_ctrl0->show();
    SurfaceComposerClient::closeGlobalTransaction();
    
    SurfaceComposerClient::openGlobalTransaction();
    surface_ctrl0->setPosition(x, y);
    SurfaceComposerClient::closeGlobalTransaction();
    
    SurfaceComposerClient::openGlobalTransaction();
    surface_ctrl0->setSize(width, height);
    SurfaceComposerClient::closeGlobalTransaction();
    
    anw0 = surface_ctrl0->getSurface();

    sp<Surface> surface = surface_ctrl0->getSurface();
    printf("surface %p\n",surface.get());
    ANativeWindow_Buffer outBuffer;
    surface->lock(&outBuffer, NULL);
    ssize_t bpr = outBuffer.stride * bytesPerPixel(outBuffer.format);
	android_memset32((uint32_t*)outBuffer.bits, 0xFFFFFFFF, bpr*outBuffer.height);
    surface->unlockAndPost();

    surface->lock(&outBuffer,NULL);
	android_memset32((uint32_t*)outBuffer.bits, 0xFFFF0000, bpr*outBuffer.height);
    surface->unlockAndPost();

    return 0;
}

int main(int argc, char *argv[])
{
	int t;
	if(argc<=1){
		printf("swin secs\n");
		return 0;
	}
	t=atoi(argv[1])*100;
	printf("t %d\n",t);
	create_window(0,0,1,1);
	while(1){
		sp<Surface> surface = surface_ctrl0->getSurface();
		ANativeWindow_Buffer outBuffer;
		surface->lock(&outBuffer, NULL);
		ssize_t bpr = outBuffer.stride * bytesPerPixel(outBuffer.format);
		android_memset32((uint32_t*)outBuffer.bits, 0xFFFFFFFF, bpr*outBuffer.height);
		surface->unlockAndPost();

		surface->lock(&outBuffer,NULL);
		android_memset32((uint32_t*)outBuffer.bits, 0xFFFF0000, bpr*outBuffer.height);
		surface->unlockAndPost();
		
		usleep(10*1000);
		--t;
		if(t==0) break;
	}
	return 0;
}

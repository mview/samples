# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH:= $(call my-dir)
PREBUILD_LIB_DIR:=$(SDK_HOME)/prebuild/$(ANDROID_VER)/lib/$(TARGET_ARCH)/
PREBUILD_3RDPARTY_LIB_DIR:=3rdparty/lib/$(TARGET_ARCH)/

$(info Android Version $(ANDROID_VER))

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	va.cpp \
	gfx.cpp \
	main.cpp \
	ffmpeg.cpp


ifeq ("7.0","$(ANDROID_VER)")
LOCAL_LDLIBS    :=-L$(PREBUILD_LIB_DIR)/share -lva-android -lutils -lui -lgui  -lGLESv2 -lEGL -landroid -lz -lc.0 -ldl.0  -L$(PREBUILD_3RDPARTY_LIB_DIR)/static -lavfilter -lavformat -lavcodec -lavutil -lswscale -lswresample -L$(PREBUILD_LIB_DIR)/static -lbz -lva -lglTest
else
LOCAL_LDLIBS    :=-L$(PREBUILD_LIB_DIR)/share -lva-android -lutils -lui -lgui  -lGLESv2 -lEGL -landroid -lz  -L$(PREBUILD_3RDPARTY_LIB_DIR)/static -lavfilter -lavformat -lavcodec -lavutil -lswscale -lswresample -L$(PREBUILD_LIB_DIR)/static -lbz -lva -lglTest
endif

LOCAL_MODULE:= ffshow

LOCAL_C_INCLUDES +=$(LOCAL_PATH)/3rdparty/include/ffmpeg/ \
		$(LOCAL_PATH)/3rdparty/include/ffmpeg/libavutil \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/external/libva \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/system/core/include \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/frameworks/native/opengl/include \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/frameworks/native/opengl/tests/include \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/frameworks/native/include \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/bionic/libc/include \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/hardware/libhardware/include \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/external/drm/include/drm \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/bionic/libc/kernel/uapi \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/bionic/libc/kernel/uapi/asm-$(TARGET_ARCH_ABI) \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/bionic/libm/include \
		$(SDK_HOME)/dep_files/$(ANDROID_VER)/external/libcxx/include 

		
LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES -DUSE_GLES_VERSION=2 -DUSE_VA_DRM -DUSE_EGL -D__STDC_CONSTANT_MACROS -std=c++0x  
ifeq ("5.0","$(ANDROID_VER)")
LOCAL_CFLAGS +=-DANDROIDVER=5
endif
ifeq ("6.0","$(ANDROID_VER)")
LOCAL_CFLAGS +=-DANDROIDVER=6
endif
ifeq ("7.0","$(ANDROID_VER)")
LOCAL_CFLAGS +=-DANDROIDVER=7 -D_USING_LIBCXX
endif

include $(BUILD_EXECUTABLE)

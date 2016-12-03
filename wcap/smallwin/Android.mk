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

PREBUILD_LIB_DIR:=$(NDK_EXT_HOME)/prebuild/$(ANDROID_VER)/lib/$(TARGET_ARCH)/

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main.cpp \


LOCAL_SHARED_LIBRARIES := 

LOCAL_LDLIBS    :=-L$(PREBUILD_LIB_DIR)/share -lcutils -lutils -lui -lgui  
ifeq ("arm64-v8a","$(TARGET_ARCH_ABI)")
ifeq ("5.0","$(ANDROID_VER)")
LOCAL_LDLIBS    +=-lbacktrace -lstlport -lhardware -lsync -lbinder -lgccdemangle -lunwind -lunwind-ptrace -lEGL.0 -lGLES_trace -lc.0 
endif
ifeq ("6.0","$(ANDROID_VER)")
LOCAL_LDLIBS    +=-lbacktrace -lhardware -lsync -lbinder -lunwind -lEGL.0 -lGLES_trace -lbase -lprotobuf-cpp-lite -ldl.0 -lc++  -lc.0 
endif
ifeq ("7.0","$(ANDROID_VER)")
LOCAL_LDLIBS    +=-lbacktrace -lhardware -lsync -lbinder -lunwind -lEGL.0 -llzma -lbase -lprotobuf-cpp-lite -ldl.0 -lc++  -lc.0 
endif
endif


LOCAL_MODULE:= swin

LOCAL_C_INCLUDES +=$(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/system/core/include \
		$(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/frameworks/native/include \
		$(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/bionic/libc/kernel/uapi \
		$(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/hardware/libhardware/include \
		$(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/bionic/libc/include \
		$(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/bionic/libc/kernel/uapi/asm-$(TARGET_ARCH) \
		$(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/bionic/libm/include \

ifeq ("7.0","$(ANDROID_VER)")
LOCAL_C_INCLUDES += $(NDK_EXT_HOME)/dep_files/7.0/external/libcxx/include 
endif
		
LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES -DUSE_GLES_VERSION=2 -DUSE_VA_DRM -DUSE_EGL -D__STDC_CONSTANT_MACROS -std=c++0x  

include $(BUILD_EXECUTABLE)

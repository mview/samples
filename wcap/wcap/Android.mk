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
include $(CLEAR_VARS)
PREBUILD_LIB_DIR:=$(NDK_EXT_HOME)/prebuild/$(ANDROID_VER)/lib/$(TARGET_ARCH)/

LOCAL_SRC_FILES:= \
	main.cpp \
	interface.cpp 

LOCAL_C_INCLUDES := -I $(LOCAL_PATH)/../lua-5.1.4/src

LOCAL_STATIC_LIBRARIES :=\
    liblua \

LOCAL_LDLIBS    :=-L$(PREBUILD_LIB_DIR)/share -lutils -lcutils -lbinder -lskia -lgui -ldl.0 -lc.0 

ifeq ("arm64-v8a","$(TARGET_ARCH_ABI)")
ifeq ("5.0","$(ANDROID_VER)")
LOCAL_LDLIBS    += -lui -lc++ -lbacktrace -ljpeg -lpng -licuuc -licui18n -lexpat -lft2 -lsync -lhardware \
				   -lunwind -lm.0 -lEGL.0 -lGLES_trace  -lstlport -lgccdemangle -lunwind-ptrace -lgabi++ \
				   -lGLESv2.0
endif
ifeq ("6.0","$(ANDROID_VER)")
LOCAL_LDLIBS    += -lui -lc++ -lbacktrace -ljpeg -lpng -licuuc -licui18n -lexpat -lft2 -lsync -lbase -lhardware \
				   -lunwind -lm.0 -lEGL.0 -lGLES_trace -lprotobuf-cpp-lite 
endif		  
ifeq ("7.0","$(ANDROID_VER)")
LOCAL_LDLIBS    += -lui -lc++ -lbacktrace -ljpeg -lpng -licuuc -licui18n -lexpat -lft2 -lsync -lbase -lhardware \
				   -lunwind -lm.0 -lEGL.0 -ldng_sdk -lpiex -llzma -lbinary_parse -limage_type_recognition -ltiff_directory  
endif		  
endif

	
LOCAL_MODULE:= wcap

LOCAL_CFLAGS := -DLUA_USE_LINUX -DTARGET_ARCH="\"$(TARGET_ARCH)\""

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

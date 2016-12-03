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
LOCAL_MODULE_TAGS := tests
LOCAL_MODULE:= liblua
CORE_C=	src/lapi.c src/lcode.c src/ldebug.c src/ldo.c src/ldump.c src/lfunc.c src/lgc.c src/llex.c src/lmem.c \
	src/lobject.c src/lopcodes.c src/lparser.c src/lstate.c src/lstring.c src/ltable.c src/ltm.c  \
	src/lundump.c src/lvm.c src/lzio.c
LIB_C=	src/lauxlib.c src/lbaselib.c src/ldblib.c src/liolib.c src/lmathlib.c src/loslib.c src/ltablib.c \
	src/lstrlib.c src/loadlib.c src/linit.c

LOCAL_SRC_FILES:= $(CORE_C) $(LIB_C)
LOCAL_C_INCLUDES += $(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/system/extras/tests/include \
    $(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/bionic \
	$(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/bionic/libc/include/ \
	$(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/bionic/libc/kernel/uapi \
	$(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/bionic/libc/kernel/uapi/asm-$(TARGET_ARCH) \
	$(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/bionic/libm/include \
    $(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/bionic/libstdc++/include \
    $(NDK_EXT_HOME)/dep_files/$(ANDROID_VER)/external/stlport/stlport \

LOCAL_CFLAGS := -DLUA_USE_LINUX 

#LOCAL_SHARED_LIBRARIES += libdl
#LOCAL_LDLIBS    :=-L$(PREBUILD_LIB_DIR)/share -lc.0  


include $(BUILD_STATIC_LIBRARY)

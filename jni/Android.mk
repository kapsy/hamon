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
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -DANDROID_NDK \
	-include $(LOCAL_PATH)/common.h

LOCAL_MODULE    := and_main
LOCAL_SRC_FILES := and_main.c
LOCAL_SRC_FILES += snd_scal.c
LOCAL_SRC_FILES += snd_sles.c
LOCAL_SRC_FILES += snd_asst.c
LOCAL_SRC_FILES += snd_ctrl.c

LOCAL_SRC_FILES += gfx/shaders.c

LOCAL_SRC_FILES += gfx_gles.c
LOCAL_SRC_FILES += gfx_asst.c
LOCAL_SRC_FILES += gfx_butn.c
LOCAL_SRC_FILES += gfx/full_screen_quad.c
LOCAL_SRC_FILES += gfx/full_screen_element.c
LOCAL_SRC_FILES += game/moods.c

LOCAL_SRC_FILES += gfx/frame_delta.c
LOCAL_SRC_FILES += gfx/touch_circle.c
LOCAL_SRC_FILES += math/trig_sampler.c
LOCAL_SRC_FILES += gfx/tex_circle.c
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -lc -lm
# for native audio
LOCAL_LDLIBS    += -lOpenSLES
# for native asset manager
LOCAL_LDLIBS    += -landroid
LOCAL_STATIC_LIBRARIES := android_native_app_glue

APP_STL := gnustl_static

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
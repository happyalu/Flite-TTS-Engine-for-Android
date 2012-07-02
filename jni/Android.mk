LOCAL_PATH:= $(call my-dir)

FLITE_BUILD_SUBDIR:=$(TARGET_ARCH_ABI)

ifeq "$(TARGET_ARCH_ABI)" "armeabi-v7a"
  FLITE_BUILD_SUBDIR:="armeabiv7a"
endif

FLITE_LIB_DIR:= $(FLITEDIR)/build/$(FLITE_BUILD_SUBDIR)-android/lib

# wrapper lib will depend on and include the flite libs
#
include $(CLEAR_VARS)

LOCAL_MODULE    := ttsflite 
LOCAL_SRC_FILES := edu_cmu_cs_speech_tts_fliteEngine.cpp \
	edu_cmu_cs_speech_tts_fliteVoices.cpp \
	edu_cmu_cs_speech_tts_String.cpp

LOCAL_C_INCLUDES := $(FLITEDIR)/include

LOCAL_LDLIBS:= -llog \
	$(FLITE_LIB_DIR)/libflite_cmu_us_generic_cg.a \
	$(FLITE_LIB_DIR)/libflite_cmulex.a \
	$(FLITE_LIB_DIR)/libflite_usenglish.a \
	$(FLITE_LIB_DIR)/libflite.a \


include $(BUILD_SHARED_LIBRARY)

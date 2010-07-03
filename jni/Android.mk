LOCAL_PATH:= $(call my-dir)

FLITE_LIB_DIR:= $(FLITEDIR)/build/arm-android/lib

# wrapper lib will depend on and include the flite libs
#
include $(CLEAR_VARS)

LOCAL_MODULE    := ttsflite 
LOCAL_SRC_FILES := edu_cmu_cs_speech_tts_fliteEngine.cpp \
#	edu_cmu_cs_speech_tts_fliteVoices.cpp \
#	edu_cmu_cs_speech_tts_String.cpp

LOCAL_C_INCLUDES := $(FLITEDIR)/include

LOCAL_LDLIBS:= -llog \
#	$(FLITE_LIB_DIR)/libflite_cmu_us_generic_cg.a \
	$(FLITE_LIB_DIR)/libflite_cmulex.a \
	$(FLITE_LIB_DIR)/libflite_usenglish.a \
	$(FLITE_LIB_DIR)/libflite.a \


include $(BUILD_SHARED_LIBRARY)

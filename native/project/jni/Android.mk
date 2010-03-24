LOCAL_PATH:= $(call my-dir)

# wrapper lib will depend on and include the flite libs
#
include $(CLEAR_VARS)

LOCAL_MODULE    := ttsfliteengine 
LOCAL_SRC_FILES := edu_cmu_cs_speech_tts_fliteEngine.cpp \
	edu_cmu_cs_speech_tts_fliteVoices.cpp \
	edu_cmu_cs_speech_tts_String.cpp

LOCAL_LDLIBS:= -llog \
	$(LOCAL_PATH)/../libs/armeabi/libcmu_us_rms_me_18.a \
	$(LOCAL_PATH)/../libs/armeabi/libflite_cmulex.a \
	$(LOCAL_PATH)/../libs/armeabi/libflite_usenglish.a \
	$(LOCAL_PATH)/../libs/armeabi/libflite.a \


include $(BUILD_SHARED_LIBRARY)

/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2010                            */
/*                        All Rights Reserved.                           */
/*                                                                       */
/*  Permission is hereby granted, free of charge, to use and distribute  */
/*  this software and its documentation without restriction, including   */
/*  without limitation the rights to use, copy, modify, merge, publish,  */
/*  distribute, sublicense, and/or sell copies of this work, and to      */
/*  permit persons to whom this work is furnished to do so, subject to   */
/*  the following conditions:                                            */
/*   1. The code must retain the above copyright notice, this list of    */
/*      conditions and the following disclaimer.                         */
/*   2. Any modifications must be clearly marked as such.                */
/*   3. Original authors' names are not deleted.                         */
/*   4. The authors' names are not used to endorse or promote products   */
/*      derived from this software without specific prior written        */
/*      permission.                                                      */
/*                                                                       */
/*  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         */
/*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
/*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
/*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
/*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
/*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
/*  THIS SOFTWARE.                                                       */
/*                                                                       */
/*************************************************************************/
/*             Author:  Alok Parlikar (aup@cs.cmu.edu)                   */
/*               Date:  June 2012                                        */
/*************************************************************************/
/*                                                                       */
/*  Flite TTS as a JNI Service (for TextToSpeechService)                 */
/*                                                                       */
/*************************************************************************/

// Standard headers
#include <jni.h>

// Flite headers
#include <flite.h>

// Local headers
#include "./edu_cmu_cs_speech_tts_logging.h"
#include "./edu_cmu_cs_speech_tts_string.h"
#include "./tts/tts.h"

// Have a different logging tag for the JNI service
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "Flite_Native_JNI_Service"

#define DEBUG_LOG_FUNCTION if (FLITE_DEBUG_ENABLED) LOGV("%s", __FUNCTION__)

jmethodID METHOD_nativeSynthCallback;
jfieldID FIELD_mNativeData;

class SynthJNIData {
 public:
  JNIEnv*                         env_;
  jobject                         tts_ref_;
  android_tts_engine_funcs_t*     flite_engine_;
  int8_t*                         audio_buffer_;
  size_t                          audio_buffer_size_;

  SynthJNIData() {
    DEBUG_LOG_FUNCTION;
    env_ = NULL;
    tts_ref_ = NULL;
    flite_engine_ = NULL;
    audio_buffer_size_ = 2048;
    audio_buffer_ = new int8_t[audio_buffer_size_];
    memset(audio_buffer_, 0, audio_buffer_size_);
  }

  ~SynthJNIData() {
    DEBUG_LOG_FUNCTION;

    if (flite_engine_) {
      flite_engine_->shutdown(flite_engine_);
      flite_engine_ = NULL;
    }

    delete audio_buffer_;
  }
};

/* Callback from flite.  Should call back to the TTS API */
static android_tts_callback_status_t TtsSynthDoneCallback(
    void **user_data, uint32_t rate,
    android_tts_audio_format_t format, int channel_count,
    int8_t **wave_data, size_t *buffer_size,
    android_tts_synth_status_t status) {
  DEBUG_LOG_FUNCTION;


  if (user_data == NULL) {
    LOGE("TtsSynthDoneCallback: userdata == NULL");
    return ANDROID_TTS_CALLBACK_HALT;
  }

  SynthJNIData* pJNIData = reinterpret_cast<SynthJNIData*>(*user_data);
  JNIEnv *env = pJNIData->env_;

  jbyteArray audio_data = env->NewByteArray(*buffer_size);
  env->SetByteArrayRegion(audio_data, 0, *buffer_size,
                          reinterpret_cast<jbyte*>(*wave_data));
  env->CallVoidMethod(pJNIData->tts_ref_,
                      METHOD_nativeSynthCallback, audio_data);

  if (status == ANDROID_TTS_SYNTH_DONE) {
    env->CallVoidMethod(pJNIData->tts_ref_, METHOD_nativeSynthCallback, NULL);
    return ANDROID_TTS_CALLBACK_HALT;
  }

  return ANDROID_TTS_CALLBACK_CONTINUE;
}


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
  JNIEXPORT jint
  JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    DEBUG_LOG_FUNCTION;
    JNIEnv *env;

    if (vm->GetEnv(reinterpret_cast<void **>(&env),
                   JNI_VERSION_1_6) != JNI_OK) {
      LOGE("Failed to get the environment using GetEnv()");
      return -1;
    }

    return JNI_VERSION_1_6;
  }

  JNIEXPORT jboolean
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeClassInit(
      JNIEnv * env, jclass cls) {
    DEBUG_LOG_FUNCTION;
    METHOD_nativeSynthCallback = env->GetMethodID(cls,
                                                  "nativeSynthCallback",
                                                  "([B)V");
    FIELD_mNativeData = env->GetFieldID(cls, "mNativeData", "I");

    return JNI_TRUE;
  }

  JNIEXPORT jboolean
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeCreate(
      JNIEnv *env, jobject object, jstring path) {
    DEBUG_LOG_FUNCTION;

    const char *path_string = env->GetStringUTFChars(path, 0);

    SynthJNIData* jni_data = new SynthJNIData();
    jni_data->flite_engine_ = android_getTtsEngine()->funcs;

    android_tts_result_t result =
        jni_data->flite_engine_->init(jni_data->flite_engine_,
                                     TtsSynthDoneCallback, path_string);

    env->SetIntField(object, FIELD_mNativeData,
                     reinterpret_cast<int>(jni_data));

    env->ReleaseStringUTFChars(path, path_string);
    return result;
  }

  JNIEXPORT jboolean
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeDestroy(
      JNIEnv *env, jobject object) {
    DEBUG_LOG_FUNCTION;

    int jni_data_address = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* jni_data = reinterpret_cast<SynthJNIData*>(jni_data_address);
    android_tts_engine_funcs_t* flite_engine = jni_data->flite_engine_;

    return flite_engine->shutdown(flite_engine);
  }

  JNIEXPORT jint
  JNICALL
  Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeIsLanguageAvailable(
      JNIEnv *env, jobject object, jstring language,
      jstring country, jstring variant) {
    DEBUG_LOG_FUNCTION;

    const char *c_language = env->GetStringUTFChars(language, NULL);
    const char *c_country = env->GetStringUTFChars(country, NULL);
    const char *c_variant = env->GetStringUTFChars(variant, NULL);

    int jni_data_address = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* jni_data = reinterpret_cast<SynthJNIData*>(jni_data_address);
    android_tts_engine_funcs_t* flite_engine = jni_data->flite_engine_;

    android_tts_support_result_t result =
        flite_engine->isLanguageAvailable(flite_engine, c_language,
                                          c_country, c_variant);

    env->ReleaseStringUTFChars(language, c_language);
    env->ReleaseStringUTFChars(country, c_country);
    env->ReleaseStringUTFChars(variant, c_variant);

    return result;
  }

  JNIEXPORT jboolean
  JNICALL
  Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeSetLanguage(
      JNIEnv *env, jobject object, jstring language,
      jstring country, jstring variant) {
    DEBUG_LOG_FUNCTION;

    const char *c_language = env->GetStringUTFChars(language, NULL);
    const char *c_country = env->GetStringUTFChars(country, NULL);
    const char *c_variant = env->GetStringUTFChars(variant, NULL);

    int jni_data_address = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* jni_data = reinterpret_cast<SynthJNIData*>(jni_data_address);
    android_tts_engine_funcs_t* flite_engine = jni_data->flite_engine_;

    android_tts_result_t result =
        flite_engine->setLanguage(flite_engine, c_language,
                                  c_country, c_variant);

    env->ReleaseStringUTFChars(language, c_language);
    env->ReleaseStringUTFChars(country, c_country);
    env->ReleaseStringUTFChars(variant, c_variant);

    if (result == ANDROID_TTS_SUCCESS) {
      return JNI_TRUE;
    } else {
      return JNI_FALSE;
    }
  }

  JNIEXPORT jboolean
  JNICALL
  Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeSetSpeechRate(
      JNIEnv *env, jobject object, jint rate) {
    DEBUG_LOG_FUNCTION;

    int jni_data_address = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* jni_data = reinterpret_cast<SynthJNIData*>(jni_data_address);
    android_tts_engine_funcs_t* flite_engine = jni_data->flite_engine_;

    android_tts_result_t result =
      flite_engine->setSpeechRate(flite_engine, rate);

    if (result == ANDROID_TTS_SUCCESS) {
      return JNI_TRUE;
    } else {
      return JNI_FALSE;
    }
  }


  JNIEXPORT jboolean
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeSynthesize(
      JNIEnv *env, jobject object, jstring text) {
    DEBUG_LOG_FUNCTION;

    int jni_data_address = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* jni_data = reinterpret_cast<SynthJNIData*>(jni_data_address);
    android_tts_engine_funcs_t* flite_engine = jni_data->flite_engine_;

    const char *c_text = env->GetStringUTFChars(text, NULL);

    jni_data->env_ = env;
    jni_data->tts_ref_ = env->NewGlobalRef(object);

    android_tts_result_t result =
        flite_engine->synthesizeText(flite_engine, c_text,
                                     jni_data->audio_buffer_,
                                     jni_data->audio_buffer_size_,
                                     jni_data);

    env->ReleaseStringUTFChars(text, c_text);
    env->DeleteGlobalRef(jni_data->tts_ref_);
    return result;
  }

  JNIEXPORT jboolean
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeStop(
      JNIEnv *env, jobject object) {
    DEBUG_LOG_FUNCTION;

    int jni_data_address = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* jni_data = reinterpret_cast<SynthJNIData*>(jni_data_address);
    android_tts_engine_funcs_t* flite_engine = jni_data->flite_engine_;

    android_tts_result_t result = flite_engine->stop(flite_engine);

    if (result == ANDROID_TTS_SUCCESS) {
      return JNI_TRUE;
    } else {
      return JNI_FALSE;
    }
  }

  JNIEXPORT jfloat
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeGetBenchmark(
      JNIEnv *env, jobject object) {
    DEBUG_LOG_FUNCTION;

    return (jfloat) getBenchmark();
  }

  JNIEXPORT jint
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeGetSampleRate(
      JNIEnv *env, jobject object) {
    DEBUG_LOG_FUNCTION;
    int jni_data_address = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* jni_data = reinterpret_cast<SynthJNIData*>(jni_data_address);
    android_tts_engine_funcs_t* flite_engine = jni_data->flite_engine_;

    LOGV("HERE: %d", flite_engine->getSampleRate(flite_engine));
    return (jint) flite_engine->getSampleRate(flite_engine);
  }

#ifdef __cplusplus
}
#endif  // __cplusplus

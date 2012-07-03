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
#include "./edu_cmu_cs_speech_tts_Common.h"
#include "./edu_cmu_cs_speech_tts_String.h"
#include "./tts/tts.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "FliteService"
#define DEBUG 1

jmethodID METHOD_nativeSynthCallback;
jfieldID FIELD_mNativeData;

/* Callback from flite.  Should call back to the TTS API */
static android_tts_callback_status_t ttsSynthDoneCB(
    void **pUserdata, uint32_t rate,
    android_tts_audio_format_t format, int channelCount,
    int8_t **pWav, size_t *pBufferSize,
    android_tts_synth_status_t status) {
  // LOGV("ttsSynthDoneCallback: %d bytes", bufferSize);

  if (pUserdata == NULL) {
    LOGE("userdata == NULL");
    return ANDROID_TTS_CALLBACK_HALT;
  }
  return ANDROID_TTS_CALLBACK_CONTINUE;
}


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
  JNIEXPORT jint
  JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
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
    if (DEBUG) LOGV("%s", __FUNCTION__);
    METHOD_nativeSynthCallback = env->GetMethodID(cls,
                                                  "nativeSynthCallback",
                                                  "([B)V");
    FIELD_mNativeData = env->GetFieldID(cls, "mNativeData", "I");

    return JNI_TRUE;
  }

  JNIEXPORT jboolean
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeCreate(
      JNIEnv *env, jobject object, jstring path) {
    if (DEBUG) LOGV("%s", __FUNCTION__);

    const char *pathString = env->GetStringUTFChars(path, 0);

    android_tts_engine_t *flite_engine = android_getTtsEngine();

    android_tts_result_t result = flite_engine->funcs->init(
        flite_engine, ttsSynthDoneCB, pathString);

    env->SetIntField(object, FIELD_mNativeData,
                     reinterpret_cast<int>(flite_engine));
    env->ReleaseStringUTFChars(path, pathString);
    return result;
  }

  JNIEXPORT jboolean
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeDestroy(
      JNIEnv *env, jobject object) {
    if (DEBUG) LOGV("%s", __FUNCTION__);
    android_tts_engine_t *flite_engine =
        reinterpret_cast<android_tts_engine_t*>(
            env->GetIntField(object, FIELD_mNativeData));
    return flite_engine->funcs->shutdown(flite_engine);
  }

  JNIEXPORT jint
  JNICALL
  Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeIsLanguageAvailable(
      JNIEnv *env, jobject object, jstring language,
      jstring country, jstring variant) {
    if (DEBUG) LOGV("%s", __FUNCTION__);

    const char *c_language = env->GetStringUTFChars(language, NULL);
    const char *c_country = env->GetStringUTFChars(country, NULL);
    const char *c_variant = env->GetStringUTFChars(variant, NULL);

    android_tts_engine_t *flite_engine =
        reinterpret_cast<android_tts_engine_t*>(
            env->GetIntField(object, FIELD_mNativeData));

    android_tts_support_result_t result =
        flite_engine->funcs->isLanguageAvailable(flite_engine, c_language,
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
    if (DEBUG) LOGV("%s", __FUNCTION__);

    const char *c_language = env->GetStringUTFChars(language, NULL);
    const char *c_country = env->GetStringUTFChars(country, NULL);
    const char *c_variant = env->GetStringUTFChars(variant, NULL);

    android_tts_engine_t *flite_engine =
        reinterpret_cast<android_tts_engine_t*>(
            env->GetIntField(object, FIELD_mNativeData));

    android_tts_result_t result =
        flite_engine->funcs->setLanguage(flite_engine, c_language,
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
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeStop(
      JNIEnv *env, jobject object) {
    if (DEBUG) LOGV("%s", __FUNCTION__);
    android_tts_engine_t *flite_engine =
        reinterpret_cast<android_tts_engine_t*>(
            env->GetIntField(object, FIELD_mNativeData));
    android_tts_result_t result = flite_engine->funcs->stop(flite_engine);
    if (result == ANDROID_TTS_SUCCESS) {
      return JNI_TRUE;
    } else {
      return JNI_FALSE;
    }
  }

#ifdef __cplusplus
}
#endif  // __cplusplus

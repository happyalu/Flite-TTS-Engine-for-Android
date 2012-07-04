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
#define LOG_TAG "Flite_Native_JNI_Service"
#define DEBUG 1

jmethodID METHOD_nativeSynthCallback;
jfieldID FIELD_mNativeData;

class SynthJNIData {
 public:
  JNIEnv*                         env;
  jobject                         tts_ref;
  android_tts_engine_funcs_t*     mFliteEngine;
  int8_t*                         mBuffer;
  size_t                          mBufferSize;

  SynthJNIData() {
    if (DEBUG) LOGV("%s", __FUNCTION__);
    env = NULL;
    tts_ref = NULL;
    mFliteEngine = NULL;
    mBufferSize = 2048;
    mBuffer = new int8_t[mBufferSize];
    memset(mBuffer, 0, mBufferSize);
  }

  ~SynthJNIData() {
    if (DEBUG) LOGV("%s", __FUNCTION__);

    if (mFliteEngine) {
      mFliteEngine->shutdown(mFliteEngine);
      mFliteEngine = NULL;
    }

    delete mBuffer;
  }
};

/* Callback from flite.  Should call back to the TTS API */
static android_tts_callback_status_t ttsSynthDoneCB(
    void **pUserdata, uint32_t rate,
    android_tts_audio_format_t format, int channelCount,
    int8_t **pWav, size_t *pBufferSize,
    android_tts_synth_status_t status) {
  if (DEBUG) LOGV("%s", __FUNCTION__);

  if (pUserdata == NULL) {
    LOGE("ttsSynthDoneCB: userdata == NULL");
    return ANDROID_TTS_CALLBACK_HALT;
  }

  SynthJNIData* pJNIData = reinterpret_cast<SynthJNIData*>(*pUserdata);
  JNIEnv *env = pJNIData->env;

  jbyteArray audioData = env->NewByteArray(*pBufferSize);
  env->SetByteArrayRegion(audioData, 0, *pBufferSize,
                          reinterpret_cast<jbyte*>(*pWav));
  env->CallVoidMethod(pJNIData->tts_ref, METHOD_nativeSynthCallback, audioData);

  if (status == ANDROID_TTS_SYNTH_DONE) {
    env->CallVoidMethod(pJNIData->tts_ref, METHOD_nativeSynthCallback, NULL);
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

    SynthJNIData* pJNIData = new SynthJNIData();
    pJNIData->mFliteEngine = android_getTtsEngine()->funcs;

    android_tts_result_t result =
        pJNIData->mFliteEngine->init(pJNIData->mFliteEngine,
                                     ttsSynthDoneCB, pathString);

    env->SetIntField(object, FIELD_mNativeData,
                     reinterpret_cast<int>(pJNIData));

    env->ReleaseStringUTFChars(path, pathString);
    return result;
  }

  JNIEXPORT jboolean
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeDestroy(
      JNIEnv *env, jobject object) {
    if (DEBUG) LOGV("%s", __FUNCTION__);

    int jniData = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* pJNIData = reinterpret_cast<SynthJNIData*>(jniData);
    android_tts_engine_funcs_t* flite_engine = pJNIData->mFliteEngine;

    return flite_engine->shutdown(flite_engine);
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

    int jniData = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* pJNIData = reinterpret_cast<SynthJNIData*>(jniData);
    android_tts_engine_funcs_t* flite_engine = pJNIData->mFliteEngine;

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
    if (DEBUG) LOGV("%s", __FUNCTION__);

    const char *c_language = env->GetStringUTFChars(language, NULL);
    const char *c_country = env->GetStringUTFChars(country, NULL);
    const char *c_variant = env->GetStringUTFChars(variant, NULL);

    int jniData = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* pJNIData = reinterpret_cast<SynthJNIData*>(jniData);
    android_tts_engine_funcs_t* flite_engine = pJNIData->mFliteEngine;

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
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeSynthesize(
      JNIEnv *env, jobject object, jstring text) {
    if (DEBUG) LOGV("%s", __FUNCTION__);

    int jniData = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* pJNIData = reinterpret_cast<SynthJNIData*>(jniData);
    android_tts_engine_funcs_t* flite_engine = pJNIData->mFliteEngine;

    const char *c_text = env->GetStringUTFChars(text, NULL);

    pJNIData->env = env;
    pJNIData->tts_ref = env->NewGlobalRef(object);

    android_tts_result_t result =
        flite_engine->synthesizeText(flite_engine, c_text, pJNIData->mBuffer,
                                     pJNIData->mBufferSize, pJNIData);

    env->DeleteGlobalRef(pJNIData->tts_ref);
    return result;
  }


  JNIEXPORT jboolean
  JNICALL Java_edu_cmu_cs_speech_tts_flite_NativeFliteTTS_nativeStop(
      JNIEnv *env, jobject object) {
    if (DEBUG) LOGV("%s", __FUNCTION__);

    int jniData = env->GetIntField(object, FIELD_mNativeData);
    SynthJNIData* pJNIData = reinterpret_cast<SynthJNIData*>(jniData);
    android_tts_engine_funcs_t* flite_engine = pJNIData->mFliteEngine;

    android_tts_result_t result = flite_engine->stop(flite_engine);

    if (result == ANDROID_TTS_SUCCESS) {
      return JNI_TRUE;
    } else {
      return JNI_FALSE;
    }
  }

#ifdef __cplusplus
}
#endif  // __cplusplus

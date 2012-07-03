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
/*               Date:  March 2010                                       */
/*************************************************************************/
/*                                                                       */
/*  Library classes to manage available flite voices                     */
/*                                                                       */
/*************************************************************************/
#ifndef JNI_EDU_CMU_CS_SPEECH_TTS_FLITEVOICES_H_
#define JNI_EDU_CMU_CS_SPEECH_TTS_FLITEVOICES_H_

#include "./edu_cmu_cs_speech_tts_String.h"

#include <flite.h>
#include <tts/tts.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

//#define voxdir_path "/sdcard/flite-data"
extern char* flite_voxdir_path;  // Path to flite-data directory

namespace FliteEngine {
  
  // Function pointer type for flite_register_voice
  typedef cst_voice* (*t_voice_register_function)(const char*);

  // Function pointer type for flite_unregister_voice
  typedef void (*t_voice_unregister_function)(cst_voice* v);

  enum VoiceRegistrationMode {
    ONLY_ONE_VOICE_REGISTERED = 0,
    ALL_VOICES_REGISTERED = 1
  };


  class Voice {
 protected:
    String mLanguage;  // ISO3 language
    String mCountry;   // ISO3 country
    String mVariant;   // Short name of the variant
    cst_voice* mFliteVoice;  // Pointer to registered flite voice

 public:
    virtual ~Voice() {}
    const char* getLanguage();
    const char* getCountry();
    const char* getVariant();

    // Returns the currently set flite voice.
    // WARNING: This will *NOT* register the voice.
    cst_voice* getFliteVoice();

    bool isSameLocaleAs(String flang, String fcountry, String fvar);

    // Returns how far the language request is supported.
    virtual android_tts_support_result_t getLocaleSupport(String flang,
                                                          String fcountry,
                                                          String fvar) {
      return ANDROID_TTS_LANG_NOT_SUPPORTED;
    }

    virtual void unregisterVoice() {}
  };


  // Voices that are linked into the library
  class LinkedVoice : public Voice {
 private:
    t_voice_register_function mRegfunc;  // Flite function: register voice
    t_voice_unregister_function mUnregfunc;  // Flite function: unregister voice

 public:
    LinkedVoice(String flang, String fcountry, String fvar,
                t_voice_register_function freg,
                t_voice_unregister_function funreg);

    ~LinkedVoice();

    cst_voice* registerVoice();
    void unregisterVoice();

    android_tts_support_result_t getLocaleSupport(String flang,
                                                  String fcountry, String fvar);
  };

  class ClustergenVoice : public Voice {
 public:
    ClustergenVoice();
    ~ClustergenVoice();

    android_tts_support_result_t getLocaleSupport(String flang,
                                                  String fcountry, String fvar);
    android_tts_result_t setLanguage(String flang,
                                     String fcountry, String fvar);
    void unregisterVoice();
  };

  class Voices {
 private:
    LinkedVoice** mVoiceList;
    Voice* mCurrentVoice;
    ClustergenVoice mCGVoice;
    VoiceRegistrationMode mRMode;  // Registration mode
    int mMaxCount;  // Maximum voice list size
    int mCurrentCount;  // Current occupancy of voice list

 public:
    Voices(int fmaxCount, VoiceRegistrationMode fregistrationMode);
    ~Voices();

    Voice* getCurrentVoice();

    void addLinkedVoice(String flang, String fcountry, String fvar,
                        t_voice_register_function freg,
                        t_voice_unregister_function funreg);

    void setDefaultVoice();

    // Find out if a particular locale is available.
    android_tts_support_result_t isLocaleAvailable(String flang,
                                                   String fcountry,
                                                   String fvar);

    // Register and set the current voice to the one asked for
    Voice* getVoiceForLocale(String flang, String fcountry, String fvar);
  };
}

#endif  // JNI_EDU_CMU_CS_SPEECH_TTS_FLITEVOICES_H_

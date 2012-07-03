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

#include "./edu_cmu_cs_speech_tts_fliteVoices.h"
#include "./edu_cmu_cs_speech_tts_Common.h"
#include <stdint.h>

namespace FliteEngine {
const char* Voice::getLanguage() {
  return mLanguage.c_str();
}

const char* Voice::getCountry() {
  return mCountry.c_str();
}

const char* Voice:: getVariant() {
  return mVariant.c_str();
}

cst_voice* Voice::getFliteVoice() {
  return mFliteVoice;
}

bool Voice::isSameLocaleAs(String flang, String fcountry, String fvar) {
  if ((mLanguage == flang) &&
      (mCountry == fcountry) &&
      (mVariant == fvar))
    return true;
  else
    return false;
}

LinkedVoice::LinkedVoice(const String flang, const String fcountry,
                         const String fvar,
                         t_voice_register_function freg,
                         t_voice_unregister_function funreg) {
  mLanguage = flang;
  mCountry = fcountry;
  mVariant = fvar;
  mFliteVoice = NULL;
  mRegfunc = freg;
  mUnregfunc = funreg;
}

LinkedVoice::~LinkedVoice() {
  LOGI("Voice::~Voice: unregistering voice");
  unregisterVoice();
  LOGI("Voice::~Voice: voice unregistered");
}

android_tts_support_result_t LinkedVoice::getLocaleSupport(String flang,
                                                           String fcountry,
                                                           String fvar) {
  android_tts_support_result_t support = ANDROID_TTS_LANG_NOT_SUPPORTED;

  if (mLanguage == flang) {
    support = ANDROID_TTS_LANG_AVAILABLE;
    if (mCountry == fcountry) {
      support = ANDROID_TTS_LANG_COUNTRY_AVAILABLE;
      if (mVariant == fvar) {
        support = ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE;
      }
    }
  }

  return support;
}


cst_voice* LinkedVoice::registerVoice() {
  LOGI("Voice::registerVoice for %s", mVariant.c_str());
  mFliteVoice = mRegfunc(flite_voxdir_path);
  LOGI("Voice::registerVoice done");
  return mFliteVoice;
}

void LinkedVoice::unregisterVoice() {
  LOGI("Calling flite unregister for %s", mVariant.c_str());
  if (mFliteVoice == NULL) return;  // Voice not registered
  mUnregfunc(mFliteVoice);
  LOGI("Done unregistering voice in flite");
  mFliteVoice = NULL;
}

ClustergenVoice::ClustergenVoice() {
  LOGI("Creating a generic clustergen voice loader.");
  mFliteVoice = NULL;
}

ClustergenVoice::~ClustergenVoice() {
  LOGI("Unloading generic clustergen voice.");

  if (mFliteVoice != NULL) {
    // We have something loaded in there. Let's unregister it.
    unregisterVoice();
  }
}

void ClustergenVoice::unregisterVoice() {
  if (mFliteVoice != NULL) {
    // TODO(aup): Flite 1.5.6 does not support unregistering a linked voice.
    // We do nothing here, but there is potential memory issue.

    LOGW("Not calling unregister for clustergen voice");

    // cst_cg_unload_voice(mFliteVoice);
    // LOGD("Flite voice unregistered.");
    mFliteVoice = NULL;
    mLanguage = "";
    mCountry = "";
    mVariant = "";
  }
}

String get_default_variant_in_countrydir(String dirname) {
  int return_code;
  DIR *dir;
  struct dirent *entry;
  struct dirent *result;

  if ((dir = opendir(dirname.c_str())) == NULL) {
    LOGE("%s could not be opened.\n ", dirname.c_str());
    return "";
  } else {
    entry = (struct dirent *) malloc(
        pathconf(dirname.c_str(), _PC_NAME_MAX) + 1);
    for (return_code = readdir_r(dir, entry, &result);
         result != NULL && return_code == 0;
         return_code = readdir_r(dir, entry, &result)) {
      if (entry->d_type == DT_REG) {
        if (strstr(entry->d_name, "cg.flitevox") ==
            (entry->d_name + strlen(entry->d_name)-11)) {
          char *tmp = new char[strlen(entry->d_name) - 11];
          strncpy(tmp, entry->d_name, strlen(entry->d_name) -12);
          tmp[strlen(entry->d_name) - 12] = '\0';
          String ret = String(tmp);
          delete[] tmp;
          closedir(dir);
          return ret;
        }
      }
    }
  }
  return "";
}

String get_default_country_in_languagedir(String dirname) {
  int return_code;
  DIR *dir;
  struct dirent *entry;
  struct dirent *result;
  String defaultVoice;

  if ((dir = opendir(dirname.c_str())) == NULL) {
    LOGE("%s could not be opened.\n ", dirname.c_str());
    return "";
  } else {
    entry = (struct dirent *) malloc(
        pathconf(dirname.c_str(), _PC_NAME_MAX) + 1);
    for (return_code = readdir_r(dir, entry, &result);
         result != NULL && return_code == 0;
         return_code = readdir_r(dir, entry, &result)) {
      if (strcmp(entry->d_name, ".") == 0 ||
         strcmp(entry->d_name, "..") == 0 )
        continue;
      if (entry->d_type == DT_DIR) {
        String newdir = dirname + "/" + String(entry->d_name);
        defaultVoice = get_default_variant_in_countrydir(newdir);
        if (!(defaultVoice == "")) {
          String ret = String(entry->d_name);
          closedir(dir);
          return ret;
        }
      }
    }
  }
  return "";
}

bool file_exists(String filename) {
  int fd;
  fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0)
    return false;
  close(fd);
  return true;
}

// Check that the required clustergen file is present on disk and
// return the information.

android_tts_support_result_t ClustergenVoice::getLocaleSupport(String flang,
                                                               String fcountry,
                                                               String fvar) {
  LOGI("ClustergenVoice::getLocaleSupport for lang=%s country=%s var=%s",
       flang.c_str(),
       fcountry.c_str(),
       fvar.c_str());

  android_tts_support_result_t languageSupport = ANDROID_TTS_LANG_NOT_SUPPORTED;
  String path = flite_voxdir_path;
  path = path + "/cg/" + flang;
  LOGV("Path: %s", path.c_str());
  if (!(get_default_country_in_languagedir(path)== "")) {
    // language exists
    languageSupport = ANDROID_TTS_LANG_AVAILABLE;
    path = path + "/" + fcountry;
    LOGV("Path: %s", path.c_str());
    if (!(get_default_variant_in_countrydir(path)== "")) {
      LOGV("Path: %s", path.c_str());
      // country exists
      languageSupport = ANDROID_TTS_LANG_COUNTRY_AVAILABLE;
      path = path + "/" + fvar + ".cg.flitevox";
      LOGV("Path: %s", path.c_str());
      if (file_exists(path)) {
        languageSupport = ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE;
        LOGV("%s is available", path.c_str());
      }
    }
  }
  return languageSupport;
}

android_tts_result_t ClustergenVoice::setLanguage(String flang,
                                                  String fcountry,
                                                  String fvar) {
  LOGI("ClustergenVoice::setLanguage: lang=%s country=%s variant=%s",
       flang.c_str(),
       fcountry.c_str(),
       fvar.c_str());

  // But check that the current voice itself isn't being requested.
  if ((mLanguage == flang) &&
     (mCountry == fcountry) &&
     (mVariant == fvar)) {
    LOGW("ClustergenVoice::setLanguage: %s",
         "Voice being requested is already registered. Doing nothing.");
    return ANDROID_TTS_SUCCESS;
  }

  // If some voice is already loaded, unload it.
  unregisterVoice();

  android_tts_support_result_t languageSupport;
  languageSupport = getLocaleSupport(flang, fcountry, fvar);
  String path = flite_voxdir_path;

  if (languageSupport == ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE) {
    path = path + "/cg/" + flang + "/" + fcountry + "/" + fvar + ".cg.flitevox";
    mLanguage = flang;
    mCountry = fcountry;
    mVariant = fvar;

    LOGW("ClustergenVoice::setLanguage: Exact voice found.");
  } else if (languageSupport == ANDROID_TTS_LANG_COUNTRY_AVAILABLE) {
    LOGW("ClustergenVoice::setLanguage: %s",
         "Exact voice not found. Only Language and country available.");
    path = path + "/cg/" + flang + "/" + fcountry;
    String var = get_default_variant_in_countrydir(path);
    path = path + "/" + var + ".cg.flitevox";

    mLanguage = flang;
    mCountry = fcountry;
    mVariant = var;
  } else if (languageSupport == ANDROID_TTS_LANG_AVAILABLE) {
    LOGW("ClustergenVoice::setLanguage: %s",
         "Exact voice not found. Only Language available.");
    path = path + "/cg/" + flang;
    String country = get_default_country_in_languagedir(path);
    path = path + "/" + country;
    String var = get_default_variant_in_countrydir(path);
    path = path + "/" + var + ".cg.flitevox";
    mLanguage = flang;
    mCountry = country;
    mVariant = var;
  } else {
    LOGE("ClustergenVoice::setLanguage: Voice not available.");
    return ANDROID_TTS_FAILURE;
  }

  // Try to load the flite voice given the voxdata file

  // new since fLite 1.5.6
  mFliteVoice = flite_voice_load(path.c_str());

  if (mFliteVoice == NULL) {
    LOGE("ClustergenVoice::setLanguage: %s",
         "Could not set language. File found but could not be loaded");
    return ANDROID_TTS_FAILURE;
  } else {
    LOGE("ClustergenVoice::setLanguage: Voice Loaded in Flite");
  }
  mLanguage = flang;
  mCountry = fcountry;
  mVariant = fvar;

  // Print out voice information from the meta-data.
  const char* lang, *country, *gender, *age, *build_date, *desc;

  lang = flite_get_param_string(mFliteVoice->features, "language", "");
  country = flite_get_param_string(mFliteVoice->features, "country", "");
  gender = flite_get_param_string(mFliteVoice->features, "gender", "");
  age = flite_get_param_string(mFliteVoice->features, "age", "");
  build_date = flite_get_param_string(mFliteVoice->features, "build_date", "");
  desc = flite_get_param_string(mFliteVoice->features, "desc", "");

  LOGV("      Clustergen voice: Voice Language: %s", lang);
  LOGV("      Clustergen voice: Speaker Country: %s", country);
  LOGV("      Clustergen voice: Speaker Gender: %s", gender);
  LOGV("      Clustergen voice: Speaker Age: %s", age);
  LOGV("      Clustergen voice: Voice Build Date: %s", build_date);
  LOGV("      Clustergen voice: Voice Description: %s", desc);

  return ANDROID_TTS_SUCCESS;
}

Voices::Voices(int fmaxCount, VoiceRegistrationMode fregistrationMode) {
  LOGI("Voices being loaded. Maximum Linked voices: %d. Registration mode: %d",
       fmaxCount, fregistrationMode);
  mRMode = fregistrationMode;
  mCurrentVoice = NULL;
  mVoiceList = new LinkedVoice*[fmaxCount];
  mMaxCount = fmaxCount;
  for (int i = 0; i < mMaxCount; i++)
    mVoiceList[i]=NULL;
  mCurrentCount = 0;
}

Voices::~Voices() {
  LOGI("Voices::~Voices Deleting voice list");
  if (mVoiceList != NULL) {
    for (int i = 0; i < mCurrentCount; i++)
      if (mVoiceList[i] != NULL)
        delete mVoiceList[i];  // Delete the individual voices
    delete[] mVoiceList;
    mVoiceList = NULL;
  }
  // clustergen voice will be destroyed automatically.
  LOGI("Voices::~Voices voice list deleted");
}

Voice* Voices::getCurrentVoice() {
  return mCurrentVoice;
}

void Voices::addLinkedVoice(String flang, String fcountry, String fvar,
                            t_voice_register_function freg,
                            t_voice_unregister_function funreg) {
  LOGI("Voices::addLinkedVoice adding %s", fvar.c_str());
  if (mCurrentCount == mMaxCount) {
    LOGE("Could not add linked voice %s_%s_%s. Too many voices",
         flang.c_str(), fcountry.c_str(), fvar.c_str());
    return;
  }

  LinkedVoice* v = new LinkedVoice(flang, fcountry, fvar, freg, funreg);

  /* We must register this voice if the registration mode
     so dictates.
  */
  if (mRMode == ALL_VOICES_REGISTERED)
    v->registerVoice();

  mVoiceList[mCurrentCount] = v;
  mCurrentCount++;
}

void Voices::setDefaultVoice() {
  if (mCurrentVoice != NULL)
    if (mRMode == ONLY_ONE_VOICE_REGISTERED) {
      mCurrentVoice->unregisterVoice();
      mCurrentVoice = NULL;
    }

  // Try to load CMU_US_RMS. If it doesn't exist,
  // then pick the first linked voice, whichever it is.

  android_tts_result_t result = mCGVoice.setLanguage("eng", "USA", "male,rms");
  if (result == ANDROID_TTS_SUCCESS) {
    mCurrentVoice = &mCGVoice;
    return;
  }

  for (int i = 0; i < mCurrentCount; i++) {
    if (mVoiceList[i] != NULL) {
      if (mRMode == ONLY_ONE_VOICE_REGISTERED)
        mVoiceList[i]->registerVoice();
      mCurrentVoice = mVoiceList[i];
      return;
    }
  }
}

android_tts_support_result_t Voices::isLocaleAvailable(String flang,
                                                       String fcountry,
                                                       String fvar) {
  LOGI("Voices::isLocaleAvailable");

  // First loop over the linked-in voices to see the locale match.
  android_tts_support_result_t languageSupport = ANDROID_TTS_LANG_NOT_SUPPORTED;
  android_tts_support_result_t currentSupport;

  for (int i = 0; i < mCurrentCount; i++)  {
    if (mVoiceList[i] == NULL) continue;
    currentSupport = mVoiceList[i]->getLocaleSupport(flang, fcountry, fvar);
    if (currentSupport == ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE) {
      // We found a match, no need to loop any more.
      return ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE;
    } else {
      if (languageSupport < currentSupport)
        // we found a better support than we previously knew
        languageSupport = currentSupport;
    }
  }

  // We need to also look through the cg voices to see if better
  // support available there.

  currentSupport = mCGVoice.getLocaleSupport(flang, fcountry, fvar);
  if (languageSupport < currentSupport)
    // we found a better support than we previously knew
    languageSupport = currentSupport;
  return languageSupport;
}

Voice* Voices::getVoiceForLocale(String flang,
                                 String fcountry, String fvar) {
  LOGI("Voices::getVoiceForLocale: language=%s country=%s variant=%s",
       flang.c_str(), fcountry.c_str(), fvar.c_str());

  /* Check that the voice we currently have set doesn't already
     provide what is requested.
  */
  if ((mCurrentVoice != NULL)
      && (mCurrentVoice->isSameLocaleAs(flang, fcountry, fvar))) {
    LOGW("Voices::getVoiceForLocale: %s",
         "Requested voice is already loaded. Doing nothing.");
    return mCurrentVoice;
  }

  /* If registration mode dictatas that only one voice can be set, 
     this is the right time to unregister currently loaded voice.
  */
  if ((mCurrentVoice != NULL) && (mRMode == ONLY_ONE_VOICE_REGISTERED)) {
    LOGI("Voices::getVoiceForLocale: %s",
         "Request for new voice. Unregistering current voice");
    mCurrentVoice->unregisterVoice();
  }
  mCurrentVoice = NULL;

  Voice* newVoice = NULL;
  android_tts_support_result_t languageSupport = ANDROID_TTS_LANG_NOT_SUPPORTED;
  android_tts_support_result_t currentSupport;

  /* First loop over the linked-in voices to gather best available voice. */

  for (int i = 0; i < mCurrentCount; i++) {
    if (mVoiceList[i] == NULL) continue;
    currentSupport = mVoiceList[i]->getLocaleSupport(flang, fcountry, fvar);
    if (languageSupport < currentSupport) {
      // We found a better support for language than we previously had.
      newVoice = mVoiceList[i];
      languageSupport = currentSupport;
    }
    if (languageSupport == ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE)
      break;  // No point in continuing search if best support is found.
  }
  LOGD("Voices::getVoiceForLocale: Linked voice support: %d.", languageSupport);

  if (languageSupport < ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE) {
    LOGD("Voices::getVoiceForLocale: %s",
         "Exact linked voice not found. Trying cg voices.");

    /* Since we didn't find an exact match, 
     * we should now search in the clustergen voices. */
    currentSupport = mCGVoice.getLocaleSupport(flang, fcountry, fvar);
    if (languageSupport <= currentSupport) {
      /* Clustergen has equal or better support. */
      LOGV("Voices::getVoiceForLocale: %s",
           "Clustergen voice has better support than linked voices.");

      android_tts_result_t result;
      result = mCGVoice.setLanguage(flang, fcountry, fvar);
      if (result == ANDROID_TTS_SUCCESS) {
        LOGI("Voices::getVoiceForLocale: %s",
             "CG voice was found and set correctly.");
        mCurrentVoice = &mCGVoice;
        return mCurrentVoice;
      } else {
        LOGE("Voices::getVoiceForLocale: CG voice could not be used. %s",
             "NO VOICE SET. Synthesis is NOT possible.");
        mCurrentVoice = NULL;  // Requested voice not available!
        return mCurrentVoice;
      }
    } else {
      /* Clustergen doesn't have better support. Go for whatever was
       * previously found.
       */
    }
  }

  if (newVoice != NULL) {
    // Something was found in the linked voices.
    mCurrentVoice = newVoice;
    if (mRMode == ONLY_ONE_VOICE_REGISTERED)
      (reinterpret_cast<LinkedVoice*>(mCurrentVoice))->registerVoice();
    return mCurrentVoice;
  } else {
    LOGE("Voices::getVoiceForLocale: %s",
         "No voice could be used. Synthesis is NOT possible.");
    mCurrentVoice = NULL;
    return mCurrentVoice;
  }
}
}

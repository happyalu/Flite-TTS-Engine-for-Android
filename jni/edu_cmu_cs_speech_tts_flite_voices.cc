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

#include "./edu_cmu_cs_speech_tts_flite_voices.h"
#include "./edu_cmu_cs_speech_tts_logging.h"
#include <stdint.h>

namespace FliteEngine {
const char* Voice::GetLanguage() {
  return language_.c_str();
}

const char* Voice::GetCountry() {
  return country_.c_str();
}

const char* Voice::GetVariant() {
  return variant_.c_str();
}

const int Voice::GetSampleRate() {
  int rate = 16000;
  if (flite_voice_ != NULL) {
    rate = flite_get_param_int(flite_voice_->features, "sample_rate", rate);
  }

  return rate;
}

cst_voice* Voice::GetFliteVoice() {
  return flite_voice_;
}

bool Voice::IsSameLocaleAs(String flang, String fcountry, String fvar) {
  if ((language_ == flang) &&
      (country_ == fcountry) &&
      (variant_ == fvar))
    return true;
  else
    return false;
}

LinkedVoice::LinkedVoice(const String flang, const String fcountry,
                         const String fvar,
                         t_voice_register_function freg,
                         t_voice_unregister_function funreg) {
  language_ = flang;
  country_ = fcountry;
  variant_ = fvar;
  flite_voice_ = NULL;
  voice_register_function_ = freg;
  voice_unregister_function_ = funreg;
}

LinkedVoice::~LinkedVoice() {
  LOGI("Voice::~Voice: unregistering voice");
  UnregisterVoice();
  LOGI("Voice::~Voice: voice unregistered");
}

android_tts_support_result_t LinkedVoice::GetLocaleSupport(String flang,
                                                           String fcountry,
                                                           String fvar) {
  android_tts_support_result_t support = ANDROID_TTS_LANG_NOT_SUPPORTED;

  if (language_ == flang) {
    support = ANDROID_TTS_LANG_AVAILABLE;
    if (country_ == fcountry) {
      support = ANDROID_TTS_LANG_COUNTRY_AVAILABLE;
      if (variant_ == fvar) {
        support = ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE;
      }
    }
  }

  return support;
}


cst_voice* LinkedVoice::RegisterVoice() {
  LOGI("Voice::RegisterVoice for %s", variant_.c_str());
  flite_voice_ = voice_register_function_(flite_voxdir_path);
  LOGI("Voice::RegisterVoice done");
  return flite_voice_;
}

void LinkedVoice::UnregisterVoice() {
  LOGI("Calling flite unregister for %s", variant_.c_str());
  if (flite_voice_ == NULL) return;  // Voice not registered
  voice_unregister_function_(flite_voice_);
  LOGI("Done unregistering voice in flite");
  flite_voice_ = NULL;
}

ClustergenVoice::ClustergenVoice() {
  LOGI("Creating a generic clustergen voice loader.");
  flite_voice_ = NULL;
}

ClustergenVoice::~ClustergenVoice() {
  LOGI("Unloading generic clustergen voice.");

  if (flite_voice_ != NULL) {
    // We have something loaded in there. Let's unregister it.
    UnregisterVoice();
  }
}

void ClustergenVoice::UnregisterVoice() {
  if (flite_voice_ != NULL) {
    // TODO(aup): Flite 1.5.6 does not support unregistering a linked voice.
    // We do nothing here, but there is potential memory issue.

    LOGW("Not calling unregister for clustergen voice");

    // cst_cg_unload_voice(flite_voice_);
    // LOGD("Flite voice unregistered.");
    flite_voice_ = NULL;
    language_ = "";
    country_ = "";
    variant_ = "";
  }
}

String GetDefaultVariantInCountryDirectory(String dirname) {
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

String GetDefaultCountryInLanguageDirectory(String dirname) {
  int return_code;
  DIR *dir;
  struct dirent *entry;
  struct dirent *result;
  String default_voice;

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
        default_voice = GetDefaultVariantInCountryDirectory(newdir);
        if (!(default_voice == "")) {
          String ret = String(entry->d_name);
          closedir(dir);
          return ret;
        }
      }
    }
  }
  return "";
}

bool FileExists(String filename) {
  int fd;
  fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0)
    return false;
  close(fd);
  return true;
}

// Check that the required clustergen file is present on disk and
// return the information.

android_tts_support_result_t ClustergenVoice::GetLocaleSupport(String flang,
                                                               String fcountry,
                                                               String fvar) {
  LOGI("ClustergenVoice::GetLocaleSupport for lang=%s country=%s var=%s",
       flang.c_str(),
       fcountry.c_str(),
       fvar.c_str());

  android_tts_support_result_t
      language_support = ANDROID_TTS_LANG_NOT_SUPPORTED;

  String path = flite_voxdir_path;
  path = path + "/cg/" + flang;
  LOGV("Path: %s", path.c_str());
  if (!(GetDefaultCountryInLanguageDirectory(path)== "")) {
    // language exists
    language_support = ANDROID_TTS_LANG_AVAILABLE;
    path = path + "/" + fcountry;
    LOGV("Path: %s", path.c_str());
    if (!(GetDefaultVariantInCountryDirectory(path)== "")) {
      LOGV("Path: %s", path.c_str());
      // country exists
      language_support = ANDROID_TTS_LANG_COUNTRY_AVAILABLE;
      path = path + "/" + fvar + ".cg.flitevox";
      LOGV("Path: %s", path.c_str());
      if (FileExists(path)) {
        language_support = ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE;
        LOGV("%s is available", path.c_str());
      }
    }
  }
  return language_support;
}

android_tts_result_t ClustergenVoice::SetLanguage(String flang,
                                                  String fcountry,
                                                  String fvar) {
  LOGI("ClustergenVoice::SetLanguage: lang=%s country=%s variant=%s",
       flang.c_str(),
       fcountry.c_str(),
       fvar.c_str());

  // But check that the current voice itself isn't being requested.
  if ((language_ == flang) &&
     (country_ == fcountry) &&
     (variant_ == fvar)) {
    LOGW("ClustergenVoice::SetLanguage: %s",
         "Voice being requested is already registered. Doing nothing.");
    return ANDROID_TTS_SUCCESS;
  }

  // If some voice is already loaded, unload it.
  UnregisterVoice();

  android_tts_support_result_t language_support;
  language_support = GetLocaleSupport(flang, fcountry, fvar);
  String path = flite_voxdir_path;

  if (language_support == ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE) {
    path = path + "/cg/" + flang + "/" + fcountry + "/" + fvar + ".cg.flitevox";
    language_ = flang;
    country_ = fcountry;
    variant_ = fvar;

    LOGW("ClustergenVoice::SetLanguage: Exact voice found.");
  } else if (language_support == ANDROID_TTS_LANG_COUNTRY_AVAILABLE) {
    LOGW("ClustergenVoice::SetLanguage: %s",
         "Exact voice not found. Only Language and country available.");
    path = path + "/cg/" + flang + "/" + fcountry;
    String var = GetDefaultVariantInCountryDirectory(path);
    path = path + "/" + var + ".cg.flitevox";

    language_ = flang;
    country_ = fcountry;
    variant_ = var;
  } else if (language_support == ANDROID_TTS_LANG_AVAILABLE) {
    LOGW("ClustergenVoice::SetLanguage: %s",
         "Exact voice not found. Only Language available.");
    path = path + "/cg/" + flang;
    String country = GetDefaultCountryInLanguageDirectory(path);
    path = path + "/" + country;
    String var = GetDefaultVariantInCountryDirectory(path);
    path = path + "/" + var + ".cg.flitevox";
    language_ = flang;
    country_ = country;
    variant_ = var;
  } else {
    LOGE("ClustergenVoice::SetLanguage: Voice not available.");
    return ANDROID_TTS_FAILURE;
  }

  // Try to load the flite voice given the voxdata file

  // new since fLite 1.5.6
  flite_voice_ = flite_voice_load(path.c_str());

  if (flite_voice_ == NULL) {
    LOGE("ClustergenVoice::SetLanguage: %s",
         "Could not set language. File found but could not be loaded");
    return ANDROID_TTS_FAILURE;
  } else {
    LOGE("ClustergenVoice::SetLanguage: Voice Loaded in Flite");
  }
  language_ = flang;
  country_ = fcountry;
  variant_ = fvar;

  // Print out voice information from the meta-data.
  const char* lang, *country, *gender, *age, *build_date, *desc;

  lang = flite_get_param_string(flite_voice_->features, "language", "");
  country = flite_get_param_string(flite_voice_->features, "country", "");
  gender = flite_get_param_string(flite_voice_->features, "gender", "");
  age = flite_get_param_string(flite_voice_->features, "age", "");
  build_date = flite_get_param_string(flite_voice_->features, "build_date", "");
  desc = flite_get_param_string(flite_voice_->features, "desc", "");

  LOGV("      Clustergen voice: Voice Language: %s", lang);
  LOGV("      Clustergen voice: Speaker Country: %s", country);
  LOGV("      Clustergen voice: Speaker Gender: %s", gender);
  LOGV("      Clustergen voice: Speaker Age: %s", age);
  LOGV("      Clustergen voice: Voice Build Date: %s", build_date);
  LOGV("      Clustergen voice: Voice Description: %s", desc);

  return ANDROID_TTS_SUCCESS;
}

Voices::Voices(int max_count, VoiceRegistrationMode registration_mode) {
  LOGI("Voices being loaded. Maximum Linked voices: %d. Registration mode: %d",
       max_count, registration_mode);
  voice_registration_mode_ = registration_mode;
  current_voice_ = NULL;
  voice_list_ = new LinkedVoice*[max_count];
  max_voice_count_ = max_count;
  for (int i = 0; i < max_voice_count_; i++)
    voice_list_[i]=NULL;
  current_voice_count_ = 0;
}

Voices::~Voices() {
  LOGI("Voices::~Voices Deleting voice list");
  if (voice_list_ != NULL) {
    for (int i = 0; i < current_voice_count_; i++)
      if (voice_list_[i] != NULL)
        delete voice_list_[i];  // Delete the individual voices
    delete[] voice_list_;
    voice_list_ = NULL;
  }
  // clustergen voice will be destroyed automatically.
  LOGI("Voices::~Voices voice list deleted");
}

Voice* Voices::GetCurrentVoice() {
  return current_voice_;
}

void Voices::AddLinkedVoice(String flang, String fcountry, String fvar,
                            t_voice_register_function freg,
                            t_voice_unregister_function funreg) {
  LOGI("Voices::AddLinkedVoice adding %s", fvar.c_str());
  if (current_voice_count_ == max_voice_count_) {
    LOGE("Could not add linked voice %s_%s_%s. Too many voices",
         flang.c_str(), fcountry.c_str(), fvar.c_str());
    return;
  }

  LinkedVoice* v = new LinkedVoice(flang, fcountry, fvar, freg, funreg);

  /* We must register this voice if the registration mode
     so dictates.
  */
  if (voice_registration_mode_ == ALL_VOICES_REGISTERED)
    v->RegisterVoice();

  voice_list_[current_voice_count_] = v;
  current_voice_count_++;
}

void Voices::SetDefaultVoice() {
  if (current_voice_ != NULL)
    if (voice_registration_mode_ == ONLY_ONE_VOICE_REGISTERED) {
      current_voice_->UnregisterVoice();
      current_voice_ = NULL;
    }

  // Try to load CMU_US_RMS. If it doesn't exist,
  // then pick the first linked voice, whichever it is.

  android_tts_result_t result = clustergen_voice_.SetLanguage("eng",
                                                              "USA",
                                                              "male,rms");
  if (result == ANDROID_TTS_SUCCESS) {
    current_voice_ = &clustergen_voice_;
    return;
  }

  for (int i = 0; i < current_voice_count_; i++) {
    if (voice_list_[i] != NULL) {
      if (voice_registration_mode_ == ONLY_ONE_VOICE_REGISTERED)
        voice_list_[i]->RegisterVoice();
      current_voice_ = voice_list_[i];
      return;
    }
  }
}

android_tts_support_result_t Voices::IsLocaleAvailable(String flang,
                                                       String fcountry,
                                                       String fvar) {
  LOGI("Voices::IsLocaleAvailable");

  // First loop over the linked-in voices to see the locale match.
  android_tts_support_result_t
      language_support = ANDROID_TTS_LANG_NOT_SUPPORTED;

  android_tts_support_result_t current_support;

  for (int i = 0; i < current_voice_count_; i++)  {
    if (voice_list_[i] == NULL) continue;
    current_support = voice_list_[i]->GetLocaleSupport(flang, fcountry, fvar);
    if (current_support == ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE) {
      // We found a match, no need to loop any more.
      return ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE;
    } else {
      if (language_support < current_support)
        // we found a better support than we previously knew
        language_support = current_support;
    }
  }

  // We need to also look through the cg voices to see if better
  // support available there.

  current_support = clustergen_voice_.GetLocaleSupport(flang, fcountry, fvar);
  if (language_support < current_support)
    // we found a better support than we previously knew
    language_support = current_support;
  return language_support;
}

Voice* Voices::GetVoiceForLocale(String flang,
                                 String fcountry, String fvar) {
  LOGI("Voices::GetVoiceForLocale: language=%s country=%s variant=%s",
       flang.c_str(), fcountry.c_str(), fvar.c_str());

  /* Check that the voice we currently have set doesn't already
     provide what is requested.
  */
  if ((current_voice_ != NULL)
      && (current_voice_->IsSameLocaleAs(flang, fcountry, fvar))) {
    LOGW("Voices::GetVoiceForLocale: %s",
         "Requested voice is already loaded. Doing nothing.");
    return current_voice_;
  }

  /* If registration mode dictatas that only one voice can be set,
     this is the right time to unregister currently loaded voice.
  */
  if ((current_voice_ != NULL)
      && (voice_registration_mode_ == ONLY_ONE_VOICE_REGISTERED)) {
    LOGI("Voices::GetVoiceForLocale: %s",
         "Request for new voice. Unregistering current voice");
    current_voice_->UnregisterVoice();
  }
  current_voice_ = NULL;

  Voice* newVoice = NULL;
  android_tts_support_result_t
      language_support = ANDROID_TTS_LANG_NOT_SUPPORTED;

  android_tts_support_result_t current_support;

  /* First loop over the linked-in voices to gather best available voice. */

  for (int i = 0; i < current_voice_count_; i++) {
    if (voice_list_[i] == NULL) continue;
    current_support = voice_list_[i]->GetLocaleSupport(flang, fcountry, fvar);
    if (language_support < current_support) {
      // We found a better support for language than we previously had.
      newVoice = voice_list_[i];
      language_support = current_support;
    }
    if (language_support == ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE)
      break;  // No point in continuing search if best support is found.
  }
  LOGD("Voices::GetVoiceForLocale: Linked voice support: %d.",
       language_support);

  if (language_support < ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE) {
    LOGD("Voices::GetVoiceForLocale: %s",
         "Exact linked voice not found. Trying cg voices.");

    /* Since we didn't find an exact match,
     * we should now search in the clustergen voices. */
    current_support = clustergen_voice_.GetLocaleSupport(flang, fcountry, fvar);
    if (language_support <= current_support) {
      /* Clustergen has equal or better support. */
      LOGV("Voices::GetVoiceForLocale: %s",
           "Clustergen voice has better support than linked voices.");

      android_tts_result_t result;
      result = clustergen_voice_.SetLanguage(flang, fcountry, fvar);
      if (result == ANDROID_TTS_SUCCESS) {
        LOGI("Voices::GetVoiceForLocale: %s",
             "CG voice was found and set correctly.");
        current_voice_ = &clustergen_voice_;
        return current_voice_;
      } else {
        LOGE("Voices::GetVoiceForLocale: CG voice could not be used. %s",
             "NO VOICE SET. Synthesis is NOT possible.");
        current_voice_ = NULL;  // Requested voice not available!
        return current_voice_;
      }
    } else {
      /* Clustergen doesn't have better support. Go for whatever was
       * previously found.
       */
    }
  }

  if (newVoice != NULL) {
    // Something was found in the linked voices.
    current_voice_ = newVoice;
    if (voice_registration_mode_ == ONLY_ONE_VOICE_REGISTERED)
      (reinterpret_cast<LinkedVoice*>(current_voice_))->RegisterVoice();
    return current_voice_;
  } else {
    LOGE("Voices::GetVoiceForLocale: %s",
         "No voice could be used. Synthesis is NOT possible.");
    current_voice_ = NULL;
    return current_voice_;
  }
}
}

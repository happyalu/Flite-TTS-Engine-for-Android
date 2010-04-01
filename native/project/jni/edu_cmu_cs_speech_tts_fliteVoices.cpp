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

#include "edu_cmu_cs_speech_tts_fliteVoices.hh"
#include "edu_cmu_cs_speech_tts_Common.hh"

namespace FliteEngine {
  
  const char* Voice::getLanguage() 
  {
    return mLanguage.c_str();
  }
  
  const char* Voice::getCountry() 
  {
    return mCountry.c_str();
  }
  
  const char* Voice:: getVariant() 
  {
    return mVariant.c_str();
  }

  cst_voice* Voice::getFliteVoice()
  {
    return mFliteVoice;
  }

  bool Voice::isSameLocaleAs(String flang, String fcountry, String fvar)
  {
    if((mLanguage == flang) && (mCountry == fcountry) && (mVariant == fvar)) return true;
    else return false;
  }

  LinkedVoice::LinkedVoice(const String flang, const String fcountry, const String fvar, 
			   t_voice_register_function freg, 
			   t_voice_unregister_function funreg)
  {
    mLanguage = flang;
    mCountry = fcountry;
    mVariant = fvar;
    mFliteVoice = NULL;
    mRegfunc = freg;
    mUnregfunc = funreg;
  }
  
  LinkedVoice::~LinkedVoice()
  {
    LOGI("Voice::~Voice: unregistering voice");
    unregisterVoice();
    LOGI("Voice::~Voice: voice unregistered");
  }
  
  android::tts_support_result LinkedVoice::getLocaleSupport(String flang, String fcountry, String fvar)
  {
    android::tts_support_result support = android::TTS_LANG_NOT_SUPPORTED;

    if(mLanguage == flang)
      {
	support = android::TTS_LANG_AVAILABLE;
	if(mCountry == fcountry)
	  {
	    support = android::TTS_LANG_COUNTRY_AVAILABLE;
	    if(mVariant == fvar)
	      {
		support = android::TTS_LANG_COUNTRY_VAR_AVAILABLE;
	      }
	  }
      }

    return support;
  }


  cst_voice* LinkedVoice::registerVoice()
  {
    LOGI("Voice::registerVoice for %s",mVariant.c_str());
    mFliteVoice = mRegfunc(voxdir_path);
    LOGI("Voice::registerVoice done");
    return mFliteVoice;
  }

  void LinkedVoice::unregisterVoice()
  {
    LOGI("Calling flite unregister for %s",mVariant.c_str());
    if(mFliteVoice == NULL) return; // Voice not registered
    mUnregfunc(mFliteVoice);
    LOGI("Done unregistering voice in flite");
    mFliteVoice = NULL;
  }
  


  ClustergenVoice::ClustergenVoice()
  {
    LOGI("Creating a generic clustergen voice loader.");
    mFliteVoice = NULL;
  }

  ClustergenVoice::~ClustergenVoice()
  {
    LOGI("Unloading generic clustergen voice.");
    
    if(mFliteVoice != NULL)
      {
	// We have something loaded in there. Let's unregister it.
	unregisterVoice();
      }
    
  }

  void ClustergenVoice::unregisterVoice()
  {
    if(mFliteVoice != NULL)
      {
        // We have something loaded in there. Let's unregister it.
        LOGD("Calling flite's unregister for cg voice");
        unregister_cmu_us_generic_cg(mFliteVoice);
        LOGD("Flite voice unregistered.");
        mFliteVoice = NULL;
	mLanguage = "";
	mCountry = "";
	mVariant = "";
      }
  }

  bool directory_exists(String dirname)
  {
    struct stat file_info;
    stat(dirname.c_str(), &file_info);
    return S_ISDIR(file_info.st_mode);
    
  }

  bool file_exists(String filename)
  {
    int fd;
    fd = open(filename.c_str(),O_RDONLY);
    if(fd<0)
      return false;
    close(fd);
    return true;
  }

  // Check that the required clustergen file is present on disk and return the information.
  android::tts_support_result ClustergenVoice::getLocaleSupport(String flang, String fcountry, String fvar)
  {
    LOGI("ClustergenVoice::getLocaleSupport for lang=%s country=%s var=%s",flang.c_str(), fcountry.c_str(), fvar.c_str());

    android::tts_support_result languageSupport = android::TTS_LANG_NOT_SUPPORTED;
    String path = voxdir_path; 
    path = path + "/cg/" + flang;
    
    if(directory_exists(path))
      {
	// language exists
	languageSupport = android::TTS_LANG_AVAILABLE;
	path = path + "/" + fcountry;
	if(directory_exists(path))
	  {
	    // country exists
	    languageSupport = android::TTS_LANG_COUNTRY_AVAILABLE;
	    path = path + "/" + fvar + ".cg.voxdata";
	    if(file_exists(path))
	      languageSupport = android::TTS_LANG_COUNTRY_VAR_AVAILABLE;
	  }

      }
    return languageSupport;
  }

  android::tts_result ClustergenVoice::setLanguage(String flang, String fcountry, String fvar)
  {
    LOGI("ClustergenVoice::setLanguage: lang=%s country=%s variant=%s",flang.c_str(), fcountry.c_str(), fvar.c_str());

    // If some voice is already loaded, unload it.
    unregisterVoice();

    String path = voxdir_path;
    path = path + "/cg/" + flang + "/" + fcountry + "/" + fvar + ".cg.voxdata";

    if(!file_exists(path))
      {
	LOGE("ClustergenVoice::setLanguage: Could not set language. Language data file (%s)not available",path.c_str());
	return android::TTS_FAILURE;
      }
    // Try to load the flite voice given the voxdata file
    mFliteVoice = register_cmu_us_generic_cg(path.c_str());
    
    if(mFliteVoice == NULL)
      {
	LOGE("ClustergenVoice::setLanguage: Could not set language. File found but could not be loaded");
	return android::TTS_FAILURE;
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

    LOGV("      Clustergen voice: Voice Language: %s",lang);
    LOGV("      Clustergen voice: Speaker Country: %s",country);
    LOGV("      Clustergen voice: Speaker Gender: %s",gender);
    LOGV("      Clustergen voice: Speaker Age: %s",age);
    LOGV("      Clustergen voice: Voice Build Date: %s",build_date);
    LOGV("      Clustergen voice: Voice Description: %s",desc);

    return android::TTS_SUCCESS;
  }

  Voices::Voices(int fmaxCount, VoiceRegistrationMode fregistrationMode)
  {
    LOGI("Voices are being loaded. Maximum Linked voices: %d. Registration mode: %d",fmaxCount, fregistrationMode);
    mRMode = fregistrationMode;
    mCurrentVoice = NULL;
    mVoiceList = new LinkedVoice*[fmaxCount];
    mMaxCount = fmaxCount;
    for(int i=0;i<mMaxCount;i++)
      mVoiceList[i]=NULL;
    mCurrentCount = 0;
  }

  Voices::~Voices()
  {
    LOGI("Voices::~Voices Deleting voice list");
    if(mVoiceList != NULL)
      {
	for(int i=0;i<mCurrentCount;i++)
	  if(mVoiceList[i] != NULL)
	    delete mVoiceList[i]; // Delete the individual voices
	delete[] mVoiceList;
	mVoiceList = NULL;
      }
    // clustergen voice will be destroyed automatically.
    LOGI("Voices::~Voices voice list deleted");
  }

  Voice* Voices::getCurrentVoice()
  {
    return mCurrentVoice;
  }

  void Voices::addLinkedVoice(String flang, String fcountry, String fvar, 
			      t_voice_register_function freg,
			      t_voice_unregister_function funreg)
  {
    LOGI("Voices::addLinkedVoice adding %s",fvar.c_str());
    if(mCurrentCount==mMaxCount)
      {
	LOGE("Could not add linked voice %s_%s_%s. Too many voices",
	     flang.c_str(),fcountry.c_str(), fvar.c_str());
	return;
      }
    
    LinkedVoice* v = new LinkedVoice(flang, fcountry, fvar, freg, funreg);

    /* We must register this voice if the registration mode
       so dictates.
    */
    if(mRMode == ALL_VOICES_REGISTERED)
      v->registerVoice();

    mVoiceList[mCurrentCount] = v;
    mCurrentCount++;
  }

  void Voices::setDefaultVoice()
  {
    if(mCurrentVoice != NULL)
      if(mRMode == ONLY_ONE_VOICE_REGISTERED)
	{
	  mCurrentVoice->unregisterVoice();
	  mCurrentVoice = NULL;
	}

    // Try to load CMU_US_RMS_ME18. If it doesn't exist, 
    // then pick the first linked voice, whichever it is.

    android::tts_result result = mCGVoice.setLanguage("eng","USA","cmu_us_rms_me18");
    if(result == android::TTS_SUCCESS) 
      {
	mCurrentVoice = &mCGVoice;
	return;
      }

    for(int i=0;i<mCurrentCount;i++)
      {
	if(mVoiceList[i] != NULL)
	  {
	    if(mRMode == ONLY_ONE_VOICE_REGISTERED)
	      mVoiceList[i]->registerVoice();
	    mCurrentVoice = mVoiceList[i];
	    return;
	  }
      }

  }
  
  android::tts_support_result Voices::isLocaleAvailable(String flang, String fcountry, String fvar)
  {
    LOGI("Voices::isLocaleAvailable");

    // First loop over the linked-in voices to see the locale match.
    android::tts_support_result languageSupport = android::TTS_LANG_NOT_SUPPORTED;
    android::tts_support_result currentSupport;

    for(int i=0;i<mCurrentCount;i++)
      {
	if(mVoiceList[i] == NULL) continue;
	currentSupport = mVoiceList[i]->getLocaleSupport(flang, fcountry, fvar);
	if(currentSupport == android::TTS_LANG_COUNTRY_VAR_AVAILABLE)
	  {
	    // We found a match, no need to loop any more.
	    return android::TTS_LANG_COUNTRY_VAR_AVAILABLE;
	  }
	else
	  {
	    if(languageSupport < currentSupport)
	      // we found a better support than we previously knew
	      languageSupport = currentSupport;
	  }
      }

    // We need to also look through the cg voices to see if better support 
    // available there.

    currentSupport = mCGVoice.getLocaleSupport(flang, fcountry, fvar);
    if(languageSupport < currentSupport)
      // we found a better support than we previously knew
      languageSupport = currentSupport;
    return languageSupport;
  }

  Voice* Voices::getVoiceForLocale(String flang, 
				   String fcountry, String fvar)
  {
    LOGI("Voices::getVoiceForLocale: language=%s country=%s variant=%s",flang.c_str(), fcountry.c_str(), fvar.c_str());
    Voice* ptr;

    /* Check that the voice we currently have set doesn't already
       provide what is requested.
    */
    if((mCurrentVoice != NULL) && (mCurrentVoice->isSameLocaleAs(flang, fcountry, fvar)))
      {
	LOGW("Voices::getVoiceForLocale: Requested voice is already loaded. Doing nothing.");
	return mCurrentVoice;
      }

    /* If registration mode dictatas that only one voice can be set, 
       this is the right time to unregister currently loaded voice.
    */
    if((mCurrentVoice != NULL) && (mRMode == ONLY_ONE_VOICE_REGISTERED))
      {
	LOGI("Voices::getVoiceForLocale: Request for new voice. Unregistering current voice");
	mCurrentVoice->unregisterVoice();
      }
    mCurrentVoice = NULL;
	
    LOGD("Voices::getVoiceForLocale: Trying to find linked voices first");

    for(int i=0; i<mCurrentCount;i++)
      {
	ptr = mVoiceList[i];
	if(ptr->getLocaleSupport(flang, fcountry, fvar) == android::TTS_LANG_COUNTRY_VAR_AVAILABLE)
	  {
	    mCurrentVoice = ptr;
	    break;
	  }
      }
    if(mCurrentVoice != NULL)
      {
	if(mRMode == ONLY_ONE_VOICE_REGISTERED)
	  ((LinkedVoice*)mCurrentVoice)->registerVoice();
	return mCurrentVoice;
      }
    LOGD("Voices::getVoiceForLocale: Linked voice unavailable. Trying to load CG voice off file");
    
    android::tts_result result;
    result = mCGVoice.setLanguage(flang, fcountry, fvar);
    if(result == android::TTS_SUCCESS)
      {
	LOGI("Voices::getVoiceForLocale: CG voice was found and set correctly.");
	mCurrentVoice = &mCGVoice;
	return mCurrentVoice;
      }
    else
      {
	LOGE("Voices::getVoiceForLocale: CG voice was also not found. NO VOICE SET. Synthesis is NOT possible.");
	mCurrentVoice = NULL; // Requested voice not available!
	return mCurrentVoice;

      }

  }
}

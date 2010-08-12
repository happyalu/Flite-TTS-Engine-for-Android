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

  String get_default_variant_in_countrydir(String dirname)
  {
    DIR *dp;
    struct dirent *ep;

    dp = opendir(dirname.c_str());
    if (dp == NULL)
      {
        LOGE("%s could not be opened.\n ",dirname.c_str());
        return "";
      }
    else
      {
        while (ep = readdir(dp))
          {
            if(ep->d_type == DT_REG)
              {
                if(strstr(ep->d_name, "cg.voxdata") == ep->d_name+strlen(ep->d_name)-10)
                  {
		    char* tmp = new char[strlen(ep->d_name) - 10];
		    strncpy(tmp, ep->d_name, strlen(ep->d_name) - 11);
		    tmp[strlen(ep->d_name) - 11] = '\0';
		    String ret = String(tmp);
		    delete[] tmp;
                    (void) closedir(dp);
		    return ret;
                  }
              }
          }
      }
    (void) closedir(dp);
    return "";
  }

  String get_default_country_in_languagedir(String dirname)
  {
    DIR *dp;
    struct dirent *ep;
    String defaultVoice;

    dp = opendir(dirname.c_str());
    if (dp == NULL)
      {
        LOGE("%s could not be opened.\n ",dirname.c_str());
        return "";
      }
    else
      {
        while (ep = readdir(dp))
          {
            if(strcmp(ep->d_name,".") == 0 ||
               strcmp(ep->d_name, "..") == 0 )
              continue;

            if(ep->d_type == DT_DIR)
              {
		String dir = dirname + "/" + String(ep->d_name);
		defaultVoice = get_default_variant_in_countrydir(dir);
		if (!(defaultVoice == ""))
		  {
		    String ret = String(ep->d_name);
		    (void) closedir(dp);
		    return ret;
		  }
              }
          }
      }
    (void) closedir(dp);
    return "";
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
    
    if(!(get_default_country_in_languagedir(path)== ""))
      {
	// language exists
	languageSupport = android::TTS_LANG_AVAILABLE;
	path = path + "/" + fcountry;
	if(!(get_default_variant_in_countrydir(path)== ""))
	  {
	    // country exists
	    languageSupport = android::TTS_LANG_COUNTRY_AVAILABLE;
	    path = path + "/" + fvar + ".cg.voxdata";
	    if(file_exists(path))
	      {
		languageSupport = android::TTS_LANG_COUNTRY_VAR_AVAILABLE;
		LOGV("%s is available",path.c_str());
	      }
	  }
      }
    return languageSupport;
  }

  android::tts_result ClustergenVoice::setLanguage(String flang, String fcountry, String fvar)
  {
    LOGI("ClustergenVoice::setLanguage: lang=%s country=%s variant=%s",flang.c_str(), fcountry.c_str(), fvar.c_str());

    // But check that the current voice itself isn't being requested.
    if((mLanguage == flang) &&
       (mCountry == fcountry) &&
       (mVariant == fvar))
      {
	LOGW("ClustergenVoice::setLanguage: Voice being requested is already registered. Doing nothing.");
	return android::TTS_SUCCESS;
      }

    // If some voice is already loaded, unload it.
    unregisterVoice();

    android::tts_support_result languageSupport = getLocaleSupport(flang, fcountry, fvar);
    String path = voxdir_path;

    if(languageSupport == android::TTS_LANG_COUNTRY_VAR_AVAILABLE)
      {
	path = path + "/cg/" + flang + "/" + fcountry + "/" + fvar + ".cg.voxdata";
	mLanguage = flang;
	mCountry = fcountry;
	mVariant = fvar;

	LOGW("ClustergenVoice::setLanguage: Exact voice found.");
      }
    else if(languageSupport == android::TTS_LANG_COUNTRY_AVAILABLE)
      {
	LOGW("ClustergenVoice::setLanguage: Exact voice not found. Only Language and country available.");
	path = path + "/cg/" + flang + "/" + fcountry;
	String var = get_default_variant_in_countrydir(path);
	path = path + "/" + var + ".cg.voxdata";

	mLanguage = flang;
	mCountry = fcountry;
	mVariant = var;
      }
    else if(languageSupport == android::TTS_LANG_AVAILABLE)
      {
	LOGW("ClustergenVoice::setLanguage: Exact voice not found. Only Language available.");
	path = path + "/cg/" + flang;
	String country = get_default_country_in_languagedir(path);
	path = path + "/" + country;
	String var = get_default_variant_in_countrydir(path);
	path = path + "/" + var + ".cg.voxdata";
	mLanguage = flang;
        mCountry = country;
        mVariant = var;
      }
    else
      {
	LOGE("ClustergenVoice::setLanguage: Voice not available.");
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
    
    Voice* newVoice = NULL;
    android::tts_support_result languageSupport = android::TTS_LANG_NOT_SUPPORTED;
    android::tts_support_result currentSupport;

    /* First loop over the linked-in voices to gather best available voice. */

    for(int i=0; i<mCurrentCount; i++)
      {
	if(mVoiceList[i] == NULL) continue;
	currentSupport = mVoiceList[i]->getLocaleSupport(flang, fcountry, fvar);
	if(languageSupport < currentSupport)
	  {
	    // We found a better support for language than we previously had.
	    newVoice = mVoiceList[i];
	    languageSupport = currentSupport;
	  }
	if(languageSupport == android::TTS_LANG_COUNTRY_VAR_AVAILABLE)
	  break; // No point in continuing search if best support is found.
      }
    LOGD("Voices::getVoiceForLocale: Linked voice support: %d.", languageSupport);

    if(languageSupport < android::TTS_LANG_COUNTRY_VAR_AVAILABLE)
      {
	LOGD("Voices::getVoiceForLocale: Exact linked voice not found. Trying cg voices.");

	/* Since we didn't find an exact match, 
	 * we should now search in the clustergen voices. */
	currentSupport = mCGVoice.getLocaleSupport(flang, fcountry, fvar); 
	if(languageSupport <= currentSupport)
	  {
	    /* Clustergen has equal or better support. */
	    LOGV("Voices::getVoiceForLocale: Clustergen voice has better support than linked voices.");
	    
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
		LOGE("Voices::getVoiceForLocale: CG voice could not be used. NO VOICE SET. Synthesis is NOT possible.");
		mCurrentVoice = NULL; // Requested voice not available!
		return mCurrentVoice;
	      }
	  }
	else
	  {
	    /* Clustergen doesn't have better support. Go for whatever was previously found. */
	  }
      }

    if(newVoice != NULL)
      {
	// Something was found in the linked voices.
	mCurrentVoice = newVoice;
	if(mRMode == ONLY_ONE_VOICE_REGISTERED)
	  ((LinkedVoice*)mCurrentVoice)->registerVoice();
	return mCurrentVoice;
      }
    else
      {
	LOGE("Voices::getVoiceForLocale: No voice could be used. Synthesis is NOT possible.");
	mCurrentVoice = NULL;
	return mCurrentVoice;
      }
  }
}

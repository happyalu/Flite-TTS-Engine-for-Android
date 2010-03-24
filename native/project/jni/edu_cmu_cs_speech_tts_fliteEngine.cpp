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
/*  Interface to the eyes-free project for Flite (www.cmuflite.org)      */
/*                                                                       */
/*************************************************************************/

// Standard headers
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// Flite headers
#include <include/flite.h>

// Eyes-free headers
#include <tts/TtsEngine.h>

// Local headers
#include "edu_cmu_cs_speech_tts_Common.hh"
#include "edu_cmu_cs_speech_tts_fliteVoices.hh"
#include "edu_cmu_cs_speech_tts_String.hh"
using namespace FliteEngine;

namespace android {

  static synthDoneCB_t* ttsSynthDoneCBPointer;
  static Voice* currentVoice;
  static int ttsAbort = 0;
  static int ttsStream = 1;
  static Voices* loadedVoices;

  /* BEGIN VOICE SPECIFIC CODE */

  // Declarations

  // RMS CG voice with order reduced to 18
  extern "C" cst_voice* register_cmu_us_rms_me_18(const char* voxdir);
  extern "C" void unregister_cmu_us_rms_me_18(cst_voice* voice);
  
  // RMS CLUNIT voice
  extern "C" cst_voice* register_cmu_us_rms(const char* voxdir); 
  extern "C" void unregister_cmu_us_rms(cst_voice* voice);
  
  void setVoiceList() {
    loadedVoices = new Voices(2, FliteEngine::Voices::ALL_VOICES_REGISTERED); // Max number of voices is the first argument.
    loadedVoices->addVoice("eng","USA","rms-cg-18",&register_cmu_us_rms_me_18,&unregister_cmu_us_rms_me_18);
    loadedVoices->addVoice("eng","USA","rms-clunit",&register_cmu_us_rms,&unregister_cmu_us_rms);
  }

  /* END VOICE SPECIFIC CODE */

  
  /* Callback from flite. Should call back the TTS API */
  static int fliteCallback(const cst_wave *w, int start, int size,
			   int last, void *userdata) 
  {
    int8_t *castedWave = (int8_t *) &w->samples[start];
    size_t bufferSize = size*sizeof(short);
    int num_channels = w->num_channels;
    int sample_rate = w->sample_rate;
    
    LOGI("flite callback received! Start: %d. Size: %d. Last: %d. Channels: %d.", start, size, last, num_channels );
    if(last == 1)
	ttsSynthDoneCBPointer(userdata, sample_rate, AudioSystem::PCM_16_BIT, num_channels, castedWave, bufferSize, TTS_SYNTH_DONE);
    else
      ttsSynthDoneCBPointer(userdata, sample_rate, AudioSystem::PCM_16_BIT, num_channels, castedWave, bufferSize, TTS_SYNTH_PENDING);
    
    LOGI("flite callback processed!");

    if(ttsAbort == 1)
      return CST_AUDIO_STREAM_STOP;

    return CST_AUDIO_STREAM_CONT;
  }

  // Initializes the TTS engine and returns whether initialization succeeded
  tts_result TtsEngine::init(synthDoneCB_t synthDoneCBPtr)
  {
    LOGI("TtsEngine::init");
    ttsSynthDoneCBPointer = synthDoneCBPtr;
    flite_init();
    setVoiceList();
    currentVoice = loadedVoices->getCurrentVoice();
    if (currentVoice->getFliteVoice() == NULL) {
      return TTS_FAILURE;
    }
    ttsAbort = 0;
    return TTS_SUCCESS;
  }

  // Shutsdown the TTS engine. Unload all voices
  tts_result TtsEngine::shutdown( void )
  {
    LOGI("TtsEngine::shutdown");
    delete loadedVoices;
    return TTS_SUCCESS;
  }

  // We load language when we set the language. Thus, this function always succeeds.
  tts_result TtsEngine::loadLanguage(const char *lang, const char *country, const char *variant)
  {
    LOGI("TtsEngine::loadLanguage: lang=%s, country=%s, variant=%s", lang, country, variant);
    return TTS_SUCCESS;
  }

  // Set the language based on locale. We use our voices manager to do this job. 
  tts_result TtsEngine::setLanguage( const char * lang, const char * country, const char * variant )
  {
    LOGI("TtsEngine::setLanguage: lang=%s, country=%s, variant=%s", lang, country, variant);

    // Set streaming or non-streaming via hacky way. There doesn't seem to be Java API to 
    // set properties. Hence this hack
    // Last character of variant = 1 for streaming will be 0 for no streaming.

    int variant_length = strlen(variant);
    char* actual_variant = new char[variant_length];
    strncpy(actual_variant, variant, variant_length-1);
    actual_variant[variant_length] = '\0';

    char streamingEnabled = variant[strlen(variant)-1];
    if(streamingEnabled == '1')
      {
	ttsStream = 1;
	LOGD("TtsEngine::setLanguage : Setting ttsStream to 1");
      }
    else
      {
	ttsStream = 0;
	LOGD("TtsEngine::setLanguage : Setting ttsStream to 0");
      }
    
    // Request the voice to voice-manager
    currentVoice = loadedVoices->getVoiceForLocale(lang, country, actual_variant);
    
    delete actual_variant;
    
    if(currentVoice->getFliteVoice() == NULL)
      return TTS_FAILURE;
    else
      return TTS_SUCCESS; 
  }

  // Language availability check does not use the "streaming" byte, as in setLanguage
  // Also, check is made against the entire locale.
  tts_support_result TtsEngine::isLanguageAvailable(const char *lang, const char *country,
                                                    const char *variant) 
  {
    LOGI("TtsEngine::isLanguageAvailable: lang=%s, country=%s, variant=%s", lang, country, variant);

    if(loadedVoices->isLocaleAvailable(lang, country, variant))
      return TTS_LANG_AVAILABLE;
    else
      return TTS_LANG_NOT_SUPPORTED;
  }

  // Provide information about the currently set language.
  tts_result TtsEngine::getLanguage(char *language, char *country, char *variant)
  {
    LOGI("TtsEngine::getLanguage");
    strcpy(language, currentVoice->getLanguage());
    strcpy(country, currentVoice->getCountry());
    strcpy(variant, currentVoice->getVariant());
    return TTS_SUCCESS;
  }

  // Setting Audio Format is not supported by Flite Engine.
  tts_result TtsEngine::setAudioFormat(AudioSystem::audio_format& encoding, uint32_t& rate,
                                       int& channels)
  {
    LOGI("TtsEngine::setAudioFormat");
    return TTS_FAILURE;
  }

  // Properties are not yet implemented.
  tts_result TtsEngine::setProperty(const char *property, const char *value,
                                    const size_t size)
  {
    LOGW("TtsEngine::setProperty is unimplemented");
    return TTS_PROPERTY_UNSUPPORTED; 
    // Other values hint: TTS_VALUE_INVALID, TTS_SUCCESS
  }

  //Properties are not yet implemented.
  tts_result TtsEngine::getProperty(const char *property, char *value, size_t *iosize)
  {
    LOGW("TtsEngine::getProperty is unimplemented");
    return TTS_PROPERTY_UNSUPPORTED;
  }

  // Support for synthesizing IPA text is not implemented.
  tts_result TtsEngine::synthesizeIpa( const char * ipa, int8_t * buffer, size_t bufferSize, void * userdata )
  {
    LOGI("TtsEngine::synthesizeIpa");
    return TTS_FAILURE;
  }

  // Interrupts synthesis.
  tts_result TtsEngine::stop()
  {
    LOGI("TtsEngine::stop");
    ttsAbort = 1;
    return TTS_SUCCESS;
  }

  // Synthesize Text. Check if streaming is requested, and stream iff so.
  tts_result TtsEngine::synthesizeText( const char * text, int8_t * buffer, size_t bufferSize, void * userdata )
  {
    cst_voice* flite_voice;
    if(currentVoice == NULL)
      {	
        LOGE("Voices not loaded?");
        return TTS_FAILURE;
      }
    flite_voice = currentVoice->getFliteVoice();
    if(flite_voice == NULL)
      {
	LOGE("Voice not available");
	return TTS_FAILURE;
      }
    
    LOGI("TtsEngine::synthesizeText: text=%s, bufferSize=%lu", text, (unsigned long) bufferSize);

    if(ttsStream)
      {
	LOGI("TtsEngine::synthesizeText: streaming is ENABLED");
        ttsAbort = 0;
        cst_audio_streaming_info *asi;
        asi = new_audio_streaming_info();
        asi->min_buffsize = bufferSize;
        asi->asc = fliteCallback;
        asi->userdata = userdata;
        feat_set(flite_voice->features,
                 "streaming_info",
                 audio_streaming_info_val(asi));
        cst_utterance *u = flite_synth_text(text,flite_voice);
        delete_utterance(u);
        feat_remove(flite_voice->features, "streaming_info");

        LOGI("Done flite synthesis.");
        return TTS_SUCCESS;
      }
    else
      {
	LOGI("TtsEngine::synthesizeText: streaming is DISABLED");
        LOGI("Starting Synthesis");

        time_t end,start;
        double dif = 0;
        time ( &start );

        cst_wave* w = flite_text_to_wave(text, flite_voice);

        time ( &end);
        dif = difftime(end, start);
        float wavlen = (float)w->num_samples / w->sample_rate;

        float timesrealtime = dif/wavlen;
	LOGW("A %1.2f second file synthesized in %1.2f seconds: %1.2f x real time.", wavlen, dif, timesrealtime);

        LOGI("Done flite synthesis.");

        size_t bufSize = w->num_samples * sizeof(short);
        int8_t* castedWave = (int8_t *)w->samples;

        ttsSynthDoneCBPointer(userdata, w->sample_rate, AudioSystem::PCM_16_BIT, w->num_channels, castedWave, bufSize, TTS_SYNTH_DONE);

        delete_wave(w);

        return TTS_SUCCESS;
      }
  }
}



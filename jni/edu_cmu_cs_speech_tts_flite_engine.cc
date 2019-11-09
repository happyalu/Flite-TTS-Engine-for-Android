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
#include <math.h>

// Flite headers
#include <flite.h>

// Local headers
#include "./edu_cmu_cs_speech_tts_logging.h"
#include "./edu_cmu_cs_speech_tts_flite_voices.h"
#include "./edu_cmu_cs_speech_tts_string.h"

static android_tts_synth_cb_t ttsSynthDoneCBPointer;
static int ttsAbort = 0;
static int ttsStream = 1;
char* flite_voxdir_path;
FliteEngine::Voices* loadedVoices;
FliteEngine::Voice* currentVoice;

/* BEGIN VOICE SPECIFIC CODE */

// Declarations
extern "C" void usenglish_init(cst_voice *v);
extern "C" cst_lexicon *cmulex_init(void);

extern "C" void cmu_indic_lang_init(cst_voice *v);
extern "C" cst_lexicon *cmu_indic_lex_init(void);

void setVoiceList() {
    if(loadedVoices != NULL)
      {
	LOGW("Voices already initialized!");
	return;
      }
    LOGI("Starting setVoiceList");
    flite_add_lang("eng",usenglish_init,cmulex_init);
    flite_add_lang("cmu_indic_lang",cmu_indic_lang_init,cmu_indic_lex_init);
    loadedVoices = new FliteEngine::Voices(0, FliteEngine::ONLY_ONE_VOICE_REGISTERED); // Max number of voices is the first argument.
    if(loadedVoices == NULL)
      {
	LOGE("Voice list could not be initialized!");
	return;
      }
    LOGI("setVoiceList: list initialized");
    LOGI("setVoiceList done");
  }

/* END VOICE SPECIFIC CODE */

#define sgn(x) (x>0?1:x?-1:0)

  void compress(short *samples,int num_samples,float mu){
    int i=0;
    short limit = 30000;
    short x;
    for(i=0; i < num_samples; i++)
      {
	x=samples[i];
	samples[i]=limit * (sgn(x)*(log(1+(mu/limit)*abs(x))/log(1+mu)));
      }
  }

  /* Callback from flite. Should call back the TTS API */
  static int fliteCallback(const cst_wave *w, int start, int size,
			   int last, cst_audio_streaming_info_struct *asi)
  {

    short *waveSamples = (short *) &w->samples[start];
    compress(waveSamples, size, 5);
    //LOGV("Compressing with 5");

    int8_t *castedWave = (int8_t *) &w->samples[start];
    size_t bufferSize = size*sizeof(short);
    int num_channels = w->num_channels;
    int sample_rate = w->sample_rate;

    // LOGV("flite callback received! Start: %d. Size: %d. Last: %d. Channels: %d. Rate: %d.", start, size, last, num_channels, sample_rate );

    if(ttsSynthDoneCBPointer != NULL)
      {
	if(last == 1)
	  {
	    /* Bug in audio rendering: Short utterances are not played. Fix it by playing silence in addition. */
	    float dur = (start+size)/sample_rate;
	    if(dur < 0.8)
	      {
		// create padding
		size_t padding_length = num_channels*(sample_rate/2);
		int8_t* paddingWave = new int8_t[padding_length]; // Half a second
		for(int i=0;i<(int)padding_length;i++)
		  paddingWave[i] = 0;
		LOGE("Utterance too short. Adding padding to the output to workaround audio rendering bug.");
		ttsSynthDoneCBPointer(&asi->userdata, sample_rate, ANDROID_TTS_AUDIO_FORMAT_PCM_16_BIT, num_channels, &castedWave, &bufferSize, ANDROID_TTS_SYNTH_PENDING);
		// Changed by Alok to still be pending, because in the new
		// streaming mode (via tokenstream), utterance end isn't the end
		// of TTS. There could be more utterances

		/*
		  ttsSynthDoneCBPointer(&asi->userdata, sample_rate, ANDROID_TTS_AUDIO_FORMAT_PCM_16_BIT, num_channels, &paddingWave, &padding_length,
				      ANDROID_TTS_SYNTH_DONE);
		*/
		ttsSynthDoneCBPointer(&asi->userdata, sample_rate, ANDROID_TTS_AUDIO_FORMAT_PCM_16_BIT, num_channels, &paddingWave, &padding_length,
				      ANDROID_TTS_SYNTH_PENDING);
		delete[] paddingWave;
	      }
	    else
	      // See comment above on why this has been changed.
	      /* ttsSynthDoneCBPointer(&asi->userdata, sample_rate, ANDROID_TTS_AUDIO_FORMAT_PCM_16_BIT, num_channels, &castedWave, &bufferSize,
		 ANDROID_TTS_SYNTH_DONE); */
	      ttsSynthDoneCBPointer(&asi->userdata, sample_rate, ANDROID_TTS_AUDIO_FORMAT_PCM_16_BIT, num_channels, &castedWave, &bufferSize,
				    ANDROID_TTS_SYNTH_PENDING);
	  }
	else
	  ttsSynthDoneCBPointer(&asi->userdata, sample_rate, ANDROID_TTS_AUDIO_FORMAT_PCM_16_BIT, num_channels, &castedWave, &bufferSize, ANDROID_TTS_SYNTH_PENDING);
        //	LOGV("flite callback processed!");
      }
    else
      {
	LOGE("flite callback not processed because it's NULL!");
	ttsAbort = 1;
      }

  if(ttsAbort == 1)
    return CST_AUDIO_STREAM_STOP;

  return CST_AUDIO_STREAM_CONT;
}

// Initializes the TTS engine and returns whether initialization succeeded
android_tts_result_t init(void* engine, android_tts_synth_cb_t synthDoneCBPtr, const char *engineConfig)
{
  LOGI("TtsEngine::init start");

  LOGI("Compilation Build Date: %s %s", __DATE__, __TIME__);

  // First make sure we receive the data directory. That's very crucial.
  if ((engineConfig != NULL) && (strlen(engineConfig) > 0)) {
    flite_voxdir_path = reinterpret_cast<char*>(malloc(strlen(engineConfig) + 12));
    snprintf(flite_voxdir_path, strlen(engineConfig) + 12,
             "%s/%s", engineConfig, "flite-data");
  } else {
    LOGE("External storage directory not specified in engineConfig. ERROR.");
    LOGE("TtsEngine::init fail");
    return ANDROID_TTS_FAILURE;
  }

  ttsSynthDoneCBPointer = synthDoneCBPtr;
  flite_init();
  setVoiceList();
  if(loadedVoices == NULL)
    {
      LOGE("TTSEngine::init Could not load voice list");
      return ANDROID_TTS_FAILURE;
    }
  currentVoice = loadedVoices->GetCurrentVoice();
  if(currentVoice == NULL)
    {
      LOGE("TTSEngine::init Voice list error");
      return ANDROID_TTS_FAILURE;
    }
  if (currentVoice->GetFliteVoice() == NULL) {
    return ANDROID_TTS_FAILURE;
  }
  ttsAbort = 0;
  LOGI("TtsEngine::init done");
  return ANDROID_TTS_SUCCESS;
  }

  // Shutsdown the TTS engine. Unload all voices
  android_tts_result_t shutdown(void* engine )
  {
    if (flite_voxdir_path != NULL) {
      free(flite_voxdir_path);
    }

    LOGI("TtsEngine::shutdown");
    if(loadedVoices != NULL)
      delete loadedVoices;
    loadedVoices = NULL;
    return ANDROID_TTS_SUCCESS;
  }

  // We load language when we set the language. Thus, this function always succeeds.
  android_tts_result_t loadLanguage(void* engine, const char *lang, const char *country, const char *variant)
  {
    LOGI("TtsEngine::loadLanguage: lang=%s, country=%s, variant=%s", lang, country, variant);
    return ANDROID_TTS_SUCCESS;
  }

  // Set the language based on locale. We use our voices manager to do this job.
  android_tts_result_t setLanguage( void* engine, const char * lang, const char * country, const char * variant )
  {
    LOGI("TtsEngine::setLanguage: lang=%s, country=%s, variant=%s", lang, country, variant);

    // Request the voice to voice-manager
    currentVoice = loadedVoices->GetVoiceForLocale(lang, country, variant);
    if(currentVoice == NULL)
      {
	LOGE("TtsEngine::setLanguage : Could not set voice");
	return ANDROID_TTS_FAILURE;
      }
    // Request the voice to voice-manager
    currentVoice = loadedVoices->GetVoiceForLocale(lang, country, variant);
    if(currentVoice == NULL)
      {
	LOGE("TtsEngine::setLanguage : Could not set voice");
	return ANDROID_TTS_FAILURE;
      }

    if(currentVoice->GetFliteVoice() == NULL)
      return ANDROID_TTS_FAILURE;
    else
      return ANDROID_TTS_SUCCESS;
  }

  android_tts_result_t setSpeechRate(void* engine, int rate) {
    LOGI("TtsEngine::setSpeechRate : Attempting to set rate %d", rate);
    if(currentVoice == NULL)
      {
	LOGE("TtsEngine::setSpeechRate : Could not set rate");
	return ANDROID_TTS_FAILURE;
      }

    cst_voice* flite_voice = currentVoice->GetFliteVoice();
    if(flite_voice == NULL)
      {
        LOGE("Voice not available to set rate");
        return ANDROID_TTS_FAILURE;
      }

    // Set duration_stretch parameter on the voice. Rate is given as a percentage.
    if (rate == 0) {
      LOGE("Attempt to set speaking rate of zero. Discarding request.");
      return ANDROID_TTS_FAILURE;
    }

    // Each voice has its own default duration stretch parameter. We should not discard this original value.
    if (!feat_present(flite_voice->features, "orig_duration_stretch")) {
      LOGW("Don't have original stretch");

      float orig_dur_stretch = get_param_float(flite_voice->features, "duration_stretch", 1.0);
      LOGW("Original duration stretch: %1.3f", orig_dur_stretch);

      feat_set_float(flite_voice->features, "orig_duration_stretch", orig_dur_stretch);
    }

    // Now update the actual duration stretch as a percentage of the default value.
    feat_set_float(flite_voice->features, "duration_stretch",
    		   feat_float(flite_voice->features, "orig_duration_stretch") * 100.0 / rate);
    return ANDROID_TTS_SUCCESS;
  }

  // Language availability check does not use the "streaming" byte, as in setLanguage
  // Also, check is made against the entire locale.
  android_tts_support_result_t isLanguageAvailable(void* engine, const char *lang, const char *country,
                                                    const char *variant)
  {
    LOGI("TtsEngine::isLanguageAvailable: lang=%s, country=%s, variant=%s", lang, country, variant);

    // The hack for java to make sure flite is available.
    // Only flite can respond "YES" to the language "eng-USA-is_flite_available"
    if ( (strcmp(lang, "eng") == 0) &&
         (strcmp(country, "USA") == 0) &&
         (strcmp(variant, "is_flite_available") == 0) ) {
      return ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE;
    }

    // The hack to set streaming:
    // If language and country are not set, then variant will be
    // interpreted as being "stream" or "nostream" to set the appropriate parameters.
    // The default is to stream.
    if((strcmp(lang,"")==0) && (strcmp(country,"")==0))
      {
	if(strcmp(variant, "stream") == 0)
	  {
	    LOGI("Streaming setting hack: TTS Streaming is ENABLED. Synthesis Benchmarks DISABLED.");
	    ttsStream = 1;
	    return ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE;
	  }
	else if(strcmp(variant, "nostream")==0)
	  {
	    LOGI("Streaming setting hack: TTS Streaming is DISABLED. Synthesis Benchmarks ENABLED.");
	    ttsStream = 0;
	    return ANDROID_TTS_LANG_COUNTRY_VAR_AVAILABLE;
	  }
	else
	  {
	    LOGE("Streaming setting hack: Incorrect setting %s. If you don't specify language and country, variant should be 'stream' or 'nostream'",variant);
	    return ANDROID_TTS_LANG_NOT_SUPPORTED;
	  }
      }

    return loadedVoices->IsLocaleAvailable(lang, country, variant);
  }

  // Provide information about the currently set language.
  android_tts_result_t getLanguage(void* engine, char *language, char *country, char *variant)
  {
    LOGI("TtsEngine::getLanguage");
    if(currentVoice == NULL)
      return ANDROID_TTS_FAILURE;

    strcpy(language, currentVoice->GetLanguage());
    strcpy(country, currentVoice->GetCountry());
    strcpy(variant, currentVoice->GetVariant());
    return ANDROID_TTS_SUCCESS;
  }

  // Provide Sample rate of current Voice
  const int getSampleRate(void* engine)
  {
    int rate = 16000;
    if (currentVoice != NULL)
    {
      rate = currentVoice->GetSampleRate();
    }

    LOGV("getSampleRate: %d", rate);
    return rate;
  }


  // Setting Audio Format is not supported by Flite Engine.
  android_tts_result_t setAudioFormat(void* engine, android_tts_audio_format_t* encoding, uint32_t* rate,
                                       int* channels)
  {
    LOGI("TtsEngine::setAudioFormat");
    cst_voice* flite_voice;
    if(currentVoice == NULL)
      {
        LOGE("Voices not loaded?");
        return ANDROID_TTS_FAILURE;
      }
    flite_voice = currentVoice->GetFliteVoice();
    if(flite_voice == NULL)
      {
        LOGE("Voice not available");
        return ANDROID_TTS_FAILURE;
      }

    *rate = feat_int(flite_voice->features,"sample_rate");
    LOGI("TtsEngine::setAudioFormat: setting Rate to %u", (unsigned int) rate);

    *encoding = ANDROID_TTS_AUDIO_FORMAT_PCM_16_BIT;
    *channels = 1;

    return ANDROID_TTS_FAILURE;
  }

  // Properties are not yet implemented.
  android_tts_result_t setProperty(void* engine, const char *property, const char *value,
                                    const size_t size)
  {
    LOGW("TtsEngine::setProperty is unimplemented");
    return ANDROID_TTS_PROPERTY_UNSUPPORTED;
    // Other values hint: TTS_VALUE_INVALID, TTS_SUCCESS
  }

  //Properties are not yet implemented.
  android_tts_result_t getProperty(void* engine, const char *property, char *value, size_t *iosize)
  {
    LOGW("TtsEngine::getProperty is unimplemented");
    return ANDROID_TTS_PROPERTY_UNSUPPORTED;
  }

  // Support for synthesizing IPA text is not implemented.
  android_tts_result_t synthesizeIpa( void* engine, const char * ipa, int8_t * buffer, size_t bufferSize, void * userdata )
  {
    LOGI("TtsEngine::synthesizeIpa");
    return ANDROID_TTS_FAILURE;
  }

  // Interrupts synthesis.
  android_tts_result_t stop(void* engine)
  {
    LOGI("TtsEngine::stop");
    ttsAbort = 1;
    return ANDROID_TTS_SUCCESS;
  }

  // Synthesize Text. Check if streaming is requested, and stream iff so.
  android_tts_result_t synthesizeText( void* engine, const char * text, int8_t * buffer, size_t bufferSize, void * userdata )
  {
    cst_voice* flite_voice;
    if(currentVoice == NULL)
      {
        LOGE("Voices not loaded?");
        return ANDROID_TTS_FAILURE;
      }
    flite_voice = currentVoice->GetFliteVoice();
    if(flite_voice == NULL)
      {
	LOGE("Voice not available");
	return ANDROID_TTS_FAILURE;
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

	/* SSML support */
	if(!strncmp(text, "<?xml version=\"1.0\"?>", 21))
	  {
	    LOGE("TtsEngine: Using SSML mode for %s", text);
	    // XML string given. Parse as SSML.

	    // For now, just strip off all tags and say whatever remains.
	    int pos1=0, pos2=0;
	    int maxpos = strlen(text);

	    char* textOverride = (char*)text;

	    while(pos2<maxpos)
	      {
		while((pos2<maxpos) && (textOverride[pos2] != '<'))
		  {
		    textOverride[pos1] = textOverride[pos2];
		    pos1++;
		    pos2++;
		  }
		while((pos2<maxpos) && (textOverride[pos2] != '>'))
		  {
		    pos2++;
		  }
	      }
	    textOverride[pos1] = '\0';

	  }

	cst_tokenstream *ts;

	char* padded_text = reinterpret_cast<char*>(malloc(strlen(text) + 2));
	snprintf(padded_text, strlen(text)+2, "%s\n\n", text);

	if ((ts=ts_open_string(padded_text,
			       get_param_string(flite_voice->features, "text_whitespace", cst_ts_default_whitespacesymbols),
			       get_param_string(flite_voice->features, "text_singlecharsymbols", cst_ts_default_singlecharsymbols),
			       get_param_string(flite_voice->features, "text_prepunctuation", cst_ts_default_prepunctuationsymbols),
			       get_param_string(flite_voice->features, "text_postpunctuation", cst_ts_default_postpunctuationsymbols)))==NULL) {

	  LOGE("Unable to open tokenstream");
	  free(padded_text);
	  return ANDROID_TTS_FAILURE;
	}

	free(padded_text);
	flite_ts_to_speech(ts,
			   flite_voice,
			   "stream");

	  //	cst_utterance *u = flite_synth_text(text,flite_voice);
	  //	delete_utterance(u);

        feat_remove(flite_voice->features, "streaming_info");

	// // Mark synthesis as done
	// size_t padding_length = 8000;
	// int8_t* paddingWave = new int8_t[padding_length]; // Half a second
	// for(int i=0;i<(int)padding_length;i++)
	//   paddingWave[i] = 0;
	// LOGE("Finalizing TTS");
	// uint32_t rate = getSampleRate(NULL);
	// ttsSynthDoneCBPointer(&asi->userdata, rate, ANDROID_TTS_AUDIO_FORMAT_PCM_16_BIT, 1, &paddingWave, &padding_length,
	// 		      ANDROID_TTS_SYNTH_DONE);


        LOGI("Done flite synthesis.");
        return ANDROID_TTS_SUCCESS;
      }
    else
      {
	// AUP: This doesn't do the right thing: It will treat all the text as one utterance.
	// The streaming code goes through tokenstreams, therefore does the right thing.

	LOGI("TtsEngine::synthesizeText: streaming is DISABLED");
        LOGI("Starting Synthesis");

        timespec start, end;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

        cst_wave* w = flite_text_to_wave(text, flite_voice);

	compress(w->samples, w->num_samples, 5);
	LOGV("Compressing with 5");

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

        // Calculate time difference
        timespec temp;
        if ((end.tv_nsec-start.tv_nsec)<0)
          {
            temp.tv_sec = end.tv_sec-start.tv_sec-1;
            temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
          }
        else
          {
            temp.tv_sec = end.tv_sec-start.tv_sec;
            temp.tv_nsec = end.tv_nsec-start.tv_nsec;
          }

        float diffmilliseconds = 1000*temp.tv_sec + (temp.tv_nsec)/1000000;

        float wavlen = 1000*((float)w->num_samples / w->sample_rate);
        float timesrealtime = wavlen/diffmilliseconds;
        LOGW("A %1.2f ms file synthesized in %1.2f ms: synthesis is %1.2f times faster than real time.", wavlen, diffmilliseconds, timesrealtime);

        LOGI("Done flite synthesis.");

        size_t bufSize = w->num_samples * sizeof(short);
        int8_t* castedWave = (int8_t *)w->samples;

	if(ttsSynthDoneCBPointer!=NULL)
	    ttsSynthDoneCBPointer(&userdata, w->sample_rate, ANDROID_TTS_AUDIO_FORMAT_PCM_16_BIT, w->num_channels, &castedWave, &bufSize, ANDROID_TTS_SYNTH_DONE);
	else
	  {
	    LOGI("flite callback not processed because it's NULL!");
	  }

        delete_wave(w);

        return ANDROID_TTS_SUCCESS;
      }
  }

  // Function to get TTS Engine
android_tts_engine_t *getTtsEngine()
{
  LOGI("TtsEngine::getTtsEngine");

  android_tts_engine_t* engine;
  engine = (android_tts_engine_t*) malloc(sizeof(android_tts_engine_t));
  android_tts_engine_funcs_t* functable = (android_tts_engine_funcs_t*) malloc(sizeof(android_tts_engine_funcs_t));
  functable->init = &init;
  functable->shutdown = &shutdown;
  functable->stop = &stop;
  functable->isLanguageAvailable = &isLanguageAvailable;
  functable->loadLanguage = &loadLanguage;
  functable->setLanguage = &setLanguage;
  functable->getLanguage = &getLanguage;
  functable->getSampleRate = &getSampleRate;
  functable->setSpeechRate = &setSpeechRate;
  functable->setAudioFormat = &setAudioFormat;
  functable->setProperty = &setProperty;
  functable->getProperty = &getProperty;
  functable->synthesizeText = &synthesizeText;
  engine->funcs = functable;
  return engine;
}

android_tts_engine_t *android_getTtsEngine()
{
  return getTtsEngine();
}


// This function generates a benchmark of how fast the current voice is.
float getBenchmark() {
  cst_voice* flite_voice;
  if(currentVoice == NULL)
  {
    LOGE("Voices not loaded?");
    return -1;
  }

  flite_voice = currentVoice->GetFliteVoice();
  if(flite_voice == NULL)
  {
    LOGE("Voice not available");
    return -1;
  }

  LOGI("TtsEngine Running Benchmark");

  timespec start, end;
  float totalmilliseconds = 0;
  float wavlen;
  int num_trials = 3;

  const char* text =
      "This sentence is the being synthesized for a benchmark computation.";

  for (int i = 0; i < num_trials; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    cst_wave* w = flite_text_to_wave(text, flite_voice);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    // Calculate time difference
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0)
    {
      temp.tv_sec = end.tv_sec-start.tv_sec-1;
      temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    }
    else
    {
      temp.tv_sec = end.tv_sec-start.tv_sec;
      temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }

    float diffmilliseconds = 1000*temp.tv_sec + (temp.tv_nsec)/1000000;
    totalmilliseconds += diffmilliseconds;
    wavlen = 1000*((float)w->num_samples / w->sample_rate);
    delete_wave(w);
  }

  float timesrealtime = (wavlen*num_trials)/totalmilliseconds;
  return timesrealtime;
}

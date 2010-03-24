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
#ifndef __FliteEngine_Voices
#define __FliteEngine_Voices
#include "edu_cmu_cs_speech_tts_String.hh"

#include <include/flite.h>

#define voxdir_path "/sdcard/flite"

namespace FliteEngine {
  // Function pointer type for flite_register_voice
  typedef cst_voice* (*t_voice_register_function)(const char*);
 
  // Function pointer type for flite_unregister_voice
  typedef void (*t_voice_unregister_function)(cst_voice*);

  class Voice {
  private:

    String language;  // ISO3 language
    String country;   // ISO3 country
    String variant;   // Short name of the variant
    cst_voice* fliteVoice; //Pointer to registered flite voice 

    // Flite Function that should be called to register the voice
    t_voice_register_function regfunc; 

    // Flite Function that should be called to unregister the voice=
    t_voice_unregister_function unregfunc; 

  public:
    Voice(String flang, String fcountry, String fvar, 
	  t_voice_register_function freg, 
	  t_voice_unregister_function funreg);
    
    ~Voice();

    // Returns true if the voice belongs to the exact locale asked
    bool isSameLocaleAs(String flang, String fcountry, String fvar);

    cst_voice* registerVoice();
    void unregisterVoice();

    // Returns the currently set flite voice. 
    // WARNING: This will *NOT* register the voice.
    cst_voice* getFliteVoice();
    
  };

  class Voices {
  private:
    Voice** voiceList;
    Voice* currentVoice;
    int rMode; // Registration mode
    int maxCount; // Maximum voice list size
    int currentCount; // Current occupancy of voice list
  public:
    static enum {
      ONLY_ONE_VOICE_REGISTERED = 0,
      ALL_VOICES_REGISTERED = 1
    } VoiceRegistrationMode;

    Voices(int fmaxCount,int fregistrationMode);
    ~Voices();

    cst_voice* getCurrentFliteVoice();

    void addVoice(String flang, String fcountry, String fvar, 
		  t_voice_register_function freg,
		  t_voice_unregister_function funreg);

    // Find out if a particular locale is available.
    bool isLocaleAvailable(String flang, String fcountry, String fvar);

    // Register and set the current voice to the one asked for
    cst_voice* getFliteVoiceForLocale(String flang, String fcountry, String fvar);
  };  

}
#endif

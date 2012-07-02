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
/*               Date:  April 2010                                       */
/*************************************************************************/

package edu.cmu.cs.speech.tts.flite;

import android.speech.tts.SynthesisCallback;
import android.speech.tts.SynthesisRequest;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeechService;
import android.util.Log;

/**
 * Implements the Flite Engine as a TextToSpeechService
 *
 */

public class FliteTtsService extends TextToSpeechService {
  @Override
  public void onCreate() {
    super.onCreate();
  }

  @Override
  protected String[] onGetLanguage() {
    Log.v("FliteTtsService", "onGetLanguage");
    return new String[] {"", "", ""};
  }

  @Override
  protected int onIsLanguageAvailable(String language, String country, String variant) {
    Log.v("FliteTtsService", "onIsLanguageAvailable");
    return TextToSpeech.LANG_NOT_SUPPORTED;
  }

  @Override
  protected int onLoadLanguage(String language, String country, String variant) {
    Log.v("FliteTtsService", "onLoadLanguage");
    return TextToSpeech.LANG_NOT_SUPPORTED;
  }

  @Override
  protected void onStop() {
    Log.v("FliteTtsService", "onStop");
  }

  @Override
  protected synchronized void onSynthesizeText(
      SynthesisRequest request, SynthesisCallback callback) {
    Log.v("FliteTtsService", "onSynthesize");
  }
  
}
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

import java.util.ArrayList;

import android.app.Activity;
import android.os.Bundle;
import android.content.Intent;
import android.speech.tts.TextToSpeech;
import android.os.Environment;
import java.io.File;
import android.util.Log;
import edu.cmu.cs.speech.tts.flite.Utility;
import java.io.IOException;
import java.io.BufferedWriter;
import java.io.FileWriter;


/* Checks if the voice data is installed
 * for flite
 */

public class CheckVoiceData extends Activity {
	private final static String FLITE_DATA_PATH = Environment.getExternalStorageDirectory()
    + "/flite-data/";
	
    
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		int result = TextToSpeech.Engine.CHECK_VOICE_DATA_PASS;
		Intent returnData = new Intent();
		returnData.putExtra(TextToSpeech.Engine.EXTRA_VOICE_DATA_ROOT_DIRECTORY, FLITE_DATA_PATH);

		ArrayList<String> available = new ArrayList<String>();
		ArrayList<String> unavailable = new ArrayList<String>();

		/* First, make sure that the directory structure we need exists
		 * There should be a "cg" folder inside the flite data directory
		 * which will store all the clustergen voice data files.
		 */

		if(!Utility.pathExists(FLITE_DATA_PATH+"cg")) {
			// Create the directory.
			Log.e("Flite.CheckVoiceData", "Flite data directory missing. Trying to create it.");
			boolean success;
			try {
				Log.e("Flite.CheckVoiceData",FLITE_DATA_PATH);
				success = new File(FLITE_DATA_PATH+"cg").mkdirs();
			}
			catch (Exception e) {
				Log.e("Flite.CheckVoiceData","Could not create directory structure. "+e.getMessage());
				success = false;
			}

			if(!success) {
				Log.e("Flite.CheckVoiceData", "Failed");
				// Can't do anything without appropriate directory structure.
				result = TextToSpeech.Engine.CHECK_VOICE_DATA_FAIL;
				setResult(result, returnData);
				finish();
			}
		}

		/* Connect to CMU TTS server and get the list of voices available, 
		 * if we don't already have a file. 
		 */
		String voiceListFile = FLITE_DATA_PATH+"cg/voices.list";
		if(!Utility.pathExists(voiceListFile)) {
			Log.e("Flite.CheckVoiceData", "Voice list file doesn't exist. Try getting it from server.");
			String voiceListURL = "http://tts.speech.cs.cmu.edu/android/vox-flite-1.5.6/voices.list?q=1";

                        FileDownloader fdload = new FileDownloader();
                        fdload.saveUrlAsFile(voiceListURL, voiceListFile);
                        while(!fdload.finished) {}
			boolean savedVoiceList = fdload.success;
			
			if(!savedVoiceList)
				Log.w("Flite.CheckVoiceData","Could not update voice list from server");
			else
				Log.w("Flite.CheckVoiceData","Successfully updated voice list from server");
		}

		/* At this point, we MUST have a voices.list file. If this file is not there,
		 * possibly because internet connection was not available, we must create a dummy
		 * 
		 */
		if(!Utility.pathExists(FLITE_DATA_PATH+"cg/voices.list")) {
			try {
				Log.w("Flite.CheckVoiceData", "Voice list not found, creating dummy list.");
			    BufferedWriter out = new BufferedWriter(new FileWriter(FLITE_DATA_PATH+"cg/voices.list"));
			    out.write("eng-USA-male,rms");
			    out.close();
			} catch (IOException e) {
				Log.e("Flite.CheckVoiceData", "Failed to create voice list dummy file.");
				// Can't do anything without that file.
				result = TextToSpeech.Engine.CHECK_VOICE_DATA_FAIL;
				setResult(result, returnData);
				finish();
			}
		}
                Log.v("Flite.CheckVoiceData", "HERE");				
		/* Go through each line in voices.list file and see
		 * if the data for that voice is installed.
		 */
		
		ArrayList<String> voiceList = null;
		try {
			voiceList = Utility.readLines(FLITE_DATA_PATH+"cg/voices.list");
		} catch (IOException e) {
			Log.e("Flite.CheckVoiceData","Problem reading voices list. This shouldn't happen!");
			result = TextToSpeech.Engine.CHECK_VOICE_DATA_FAIL;
			setResult(result, returnData);
			finish();
		}
		
		for(String strLine:voiceList) {
			String[] voiceParams = strLine.split("-");
			if(voiceParams.length != 3) {
				Log.e("Flite.CheckVoiceData","Incorrect voicename:" + strLine);
				continue;
			}
				
			if(voiceAvailable(voiceParams)) {
				available.add(strLine);
			} else {
				unavailable.add(strLine);
			}
		}
		
		returnData.putStringArrayListExtra("availableVoices", available);
		returnData.putStringArrayListExtra("unavailableVoices", unavailable);
		setResult(result, returnData);
		finish();
	}
    
    private boolean voiceAvailable(String[] voiceParams) {
      Log.v("Flite.CheckVoiceData", "Checking for Voice Available" + voiceParams[0]+"/"+voiceParams[1]+"/"+voiceParams[2]+".cg.flitevox");
    	String voxdataFileName = FLITE_DATA_PATH + "cg/"+voiceParams[0]+"/"+voiceParams[1]+"/"+voiceParams[2]+".cg.flitevox";
    	return Utility.pathExists(voxdataFileName);
    }
    
}  

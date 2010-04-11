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
		
		/* Connect to CMU TTS server and get the list of voices available 
		 * 
		 */
		String voiceListURL = "http://tts.speech.cs.cmu.edu/android/general/voices.list";
		String voiceListFile = FLITE_DATA_PATH+"cg/voices.list";
		boolean savedVoiceList = Utility.saveUrlAsFile(voiceListURL, voiceListFile);
		
		if(!savedVoiceList)
			Log.w("Flite.CheckVoiceData","Could not update voice list from server");
		else
			Log.w("Flite.CheckVoiceData","Successfully updated voice list from server");
		
		/* At this point, we MUST have a voices.list file. If this file is not there,
		 * possibly because internet connection was not available, we must create a dummy
		 * 
		 */
		if(!Utility.pathExists(FLITE_DATA_PATH+"cg/voices.list")) {
			try {
				Log.w("Flite.CheckVoiceData", "Voice list not found, creating dummy list.");
			    BufferedWriter out = new BufferedWriter(new FileWriter(FLITE_DATA_PATH+"cg/voices.list"));
			    out.write("eng-USA-rms");
			    out.close();
			} catch (IOException e) {
				Log.e("Flite.CheckVoiceData", "Failed to create voice list dummy file.");
				// Can't do anything without that file.
				result = TextToSpeech.Engine.CHECK_VOICE_DATA_FAIL;
				setResult(result, returnData);
				finish();
			}
		}

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
    	String voxdataFileName = FLITE_DATA_PATH + "cg/"+voiceParams[0]+"/"+voiceParams[1]+"/"+voiceParams[2]+".cg.voxdata";
    	return Utility.pathExists(voxdataFileName);
    }
    
}  

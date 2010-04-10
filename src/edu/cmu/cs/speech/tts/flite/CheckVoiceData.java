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
import java.io.FileInputStream;
import java.io.DataInputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.InputStreamReader;
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
		if(!pathExists("cg")) {
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
		 * TODO (aup): Create combinations of voices and have that available as a preference.
		 * This will make sure that everybody downloads only required voices.
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
		if(!pathExists("cg/voices.list")) {
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
		
		try{
			FileInputStream fstream = new FileInputStream(FLITE_DATA_PATH+"cg/voices.list");
			DataInputStream in = new DataInputStream(fstream);
			BufferedReader br = new BufferedReader(new InputStreamReader(in),1024);
			String strLine;
			while ((strLine = br.readLine()) != null)   {
				String[] voiceParams = strLine.split("-");
				if(voiceParams.length != 3)
					throw new Exception("Incorrect voice name");
				
				if(voiceAvailable(voiceParams)) {
					available.add(strLine);
				} else {
					unavailable.add(strLine);
				}
			}
			in.close();
		} catch (Exception e){//Catch exception if any
			Log.e("Flite.CheckVoiceData","Error: " + e.getMessage());
		}

		returnData.putStringArrayListExtra("availableVoices", available);
		returnData.putStringArrayListExtra("unavailableVoices", unavailable);
		setResult(result, returnData);
		finish();
	}
    
    private boolean pathExists(String pathname){
        File tempFile = new File(FLITE_DATA_PATH + pathname);
        if ((!tempFile.exists()) ){ 
            return false;
        }
        return true;
    }
    
    private boolean voiceAvailable(String[] voiceParams) {
    	String voxdataFileName = FLITE_DATA_PATH + "cg/"+voiceParams[0]+"/"+voiceParams[1]+"/"+voiceParams[2]+".voxdata";
    	return pathExists(voxdataFileName);
    }
    
}  

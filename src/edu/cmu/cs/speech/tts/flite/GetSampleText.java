package edu.cmu.cs.speech.tts.flite;

import android.app.Activity;
import android.os.Bundle;
import android.content.Intent;
import android.speech.tts.TextToSpeech;

/*
 * Returns the sample text string for the language requested
 */
public class GetSampleText extends Activity {
    
    @Override
	public void onCreate(Bundle savedInstanceState) {
	super.onCreate(savedInstanceState);
	
	int result = TextToSpeech.LANG_AVAILABLE;
	Intent returnData = new Intent();

	Intent i = getIntent();
	String language = i.getExtras().getString("language");
	
	if (language.equals("eng")) {
	    returnData.putExtra("sampleText", getString(R.string.eng_sample));
	} else {
	    result = TextToSpeech.LANG_NOT_SUPPORTED;
	    returnData.putExtra("sampleText", "");
	}
	
	setResult(result, returnData);	

	finish();
    }
}  

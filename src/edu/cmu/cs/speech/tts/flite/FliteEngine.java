package edu.cmu.cs.speech.tts.flite;

import android.app.Activity;
import android.os.Bundle;
import android.content.Intent;


public class FliteEngine extends Activity {
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		Intent data = new Intent();
		setResult(Activity.RESULT_OK, data);
		finish();
	}
}  

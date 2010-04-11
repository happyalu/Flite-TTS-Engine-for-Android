package edu.cmu.cs.speech.tts.flite;

import android.app.Activity;
import android.os.Bundle;

public class Flite extends Activity {
    
    @Override
	public void onCreate(Bundle savedInstanceState) {
	super.onCreate(savedInstanceState);
	// The Java portion of this does nothing.
        // This activity is only here so that everything
        // can be wrapped up inside an apk file.
	finish();
    }
}  

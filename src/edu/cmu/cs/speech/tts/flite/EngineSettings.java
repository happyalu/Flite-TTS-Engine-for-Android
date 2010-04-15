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

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Environment;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.util.Log;
import android.view.WindowManager;

public class EngineSettings extends PreferenceActivity implements Preference.OnPreferenceClickListener{
	private final static String FLITE_DATA_PATH = Environment.getExternalStorageDirectory()
    + "/flite-data/";
	
	private Preference mLiveUpdate;
	private ProgressDialog mProgress;
    
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND,
		        WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
		
		addPreferencesFromResource(R.xml.engine_settings);
		initClickers();
		
			
	}
	
	private void initClickers() {
		mLiveUpdate = findPreference("flite_sync_voicelist");	
        mLiveUpdate.setOnPreferenceClickListener(this);

	}
	
	private boolean syncVoiceList() {
		/* Connect to CMU TTS server and get the list of voices available 
		 * 
		 */
		final String voiceListURL = "http://tts.speech.cs.cmu.edu/android/general/voices.list?q=2";
		final String voiceListFile = FLITE_DATA_PATH+"cg/voices.list";
		
		final FileDownloader fdload = new FileDownloader();
		
		mProgress = new ProgressDialog(this);
		mProgress.setCancelable(true);
		mProgress.setIndeterminate(true);
		mProgress.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
		mProgress.setButton(ProgressDialog.BUTTON_NEGATIVE, "Cancel", new ProgressDialog.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				fdload.abort();
			}
		});
		mProgress.show();
		final Builder syncSuccessStatus = new AlertDialog.Builder(this);
		syncSuccessStatus.setPositiveButton("Ok",null);
		
		new Thread() {
    		public void run() { 
    			fdload.saveUrlAsFile(voiceListURL, voiceListFile);
    			while(fdload.totalFileLength == 0) {
    				if (fdload.finished)
    					break;
    			}
    			runOnUiThread(new Runnable() {

    				@Override
    				public void run() {
    					mProgress.setIndeterminate(false);
    					mProgress.setMax(fdload.totalFileLength);
    				}
    			});
    			int prev = 0;
    			while(!fdload.finished) {
    				if (fdload.finishedFileLength > prev) {
    					prev = fdload.finishedFileLength;
    					runOnUiThread(new Runnable() {

        					@Override
        					public void run() {
        						mProgress.setProgress(fdload.finishedFileLength);
        					}
        				});
    				}
    			}
    			runOnUiThread(new Runnable() {
					
					@Override
					public void run() {
	
		    			mProgress.dismiss();

					}
				});
    			if(!fdload.success) {
    				Log.e("Flite.LiveUpdate", "Voice list download failed!");
    				if(fdload.abortDownload)
    					syncSuccessStatus.setMessage("Live Update aborted.");
    				else
    					syncSuccessStatus.setMessage("Live Update failed! Check your internet settings.");
    			}
    			else {
    				syncSuccessStatus.setMessage("Live update succesful. Voice list synced with server.");
    			}
    			runOnUiThread(new Runnable() {
					
					@Override
					public void run() {
						syncSuccessStatus.show();
					}
				});
    		}
    	}.start();
		return true;
	}

	@Override
	public boolean onPreferenceClick(Preference preference) {
		Log.v("Flite.EngineSettings","Trying to start Live Update");
		if(preference == mLiveUpdate) {
			Log.v("Flite.EngineSettings","Starting Live Update");
			return syncVoiceList();
		}
		
		return false;
	}
}

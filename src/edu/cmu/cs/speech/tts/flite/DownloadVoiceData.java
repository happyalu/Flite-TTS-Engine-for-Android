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

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Locale;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

/* Download user-requested voice data for Flite
 * 
 */
public class DownloadVoiceData extends Activity {
	private final static String LOG_TAG = "Flite_Java_" + DownloadVoiceData.class.getSimpleName();	
	private final static String FLITE_DATA_PATH = CheckVoiceData.getDataPath();

	private ArrayAdapter<CharSequence> mVoiceList;
	private ArrayAdapter<CharSequence> mVoiceDescList;
	private Handler mHandler;
	ProgressDialog pd;


	private static ArrayList<String> availableLanguages = null;
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		try {
			availableLanguages = Utility.readLines(FLITE_DATA_PATH+"cg/voices.list");
		} catch (IOException e) {
			Log.e(LOG_TAG,"Could not read voice list");
			abort("Could not download voice data. Please check your internet connectivity.");			
		}

		setContentView(R.layout.download_voice_data);
		Spinner voiceSpinner = (Spinner) findViewById(R.id.spnVoice);
		mVoiceDescList = new ArrayAdapter<CharSequence>(this,android.R.layout.simple_spinner_item);
		mVoiceList = new ArrayAdapter<CharSequence>(this,android.R.layout.simple_spinner_item);
		voiceSpinner.setAdapter(mVoiceDescList);
		mVoiceDescList.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

		Button bDownload = (Button) findViewById(R.id.bDownload);
		bDownload.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				downloadSelectedVoice();				
			}

		});
		mHandler = new Handler();
		populateVoiceList();

	}

	/* Look at the voices available in flite, 
	 * and compare them to voices already installed.
	 * The remaining voices will be available for download.
	 */
	private void populateVoiceList() {
		mVoiceList.clear();
		mVoiceDescList.clear();
		for(String s: availableLanguages) {
			String[] voiceInfo = s.split("\t");
			if (voiceInfo.length != 2) {
				Log.e(LOG_TAG, "Voice line could not be read: " + s);
				continue;
			}
			String voiceName = voiceInfo[0];
			String voiceMD5 = voiceInfo[1];

			String[] voiceParams = voiceName.split("-");
			if(voiceParams.length != 3) {
				Log.e(LOG_TAG,"Incorrect voicename:" + voiceName);
				continue;
			}

			if(!CheckVoiceData.voiceAvailable(voiceParams, voiceMD5)) {
				// We need to install this voice.
				Locale loc = new Locale(voiceParams[0],voiceParams[1],voiceParams[2]);

				mVoiceDescList.add(loc.getDisplayLanguage() +
						" (" + loc.getISO3Country() + ", " +
						loc.getVariant() + ")");
				mVoiceList.add(voiceName);
			}
		}
		if(mVoiceList.getCount()==0) {
			TextView tv = (TextView) findViewById(R.id.txtInfo);
			tv.setText("All voices correctly installed");

			Button b = (Button) findViewById(R.id.bDownload);
			b.setEnabled(false);
		}
	}

	// Download the chosen voice from CMU server.
	private void downloadSelectedVoice() {
		Spinner sp = (Spinner) findViewById(R.id.spnVoice);
		String selectedVoice = (String) mVoiceList.getItem(sp.getSelectedItemPosition());

		String[] voiceParams = selectedVoice.split("-");
		if(voiceParams.length != 3) {
			Log.e("Flite.CheckVoiceData","Incorrect voicename:" + selectedVoice);
			return;
		}

		String datapath = FLITE_DATA_PATH + "cg/" + voiceParams[0] + "/" + voiceParams[1];
		try {
			if(!Utility.pathExists(datapath))
				if(! new File(datapath).mkdirs()) {
					abort("Could not create directory structure necessary to store the voice");
				}
		} catch (Exception e) {
			abort("Could not create directory structure necessary to store the voice");
		}

		final FileDownloader fdload = new FileDownloader();

		pd = new ProgressDialog(this);
		pd.setCancelable(true);
		pd.setIndeterminate(true);
		pd.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
		pd.setButton(ProgressDialog.BUTTON_NEGATIVE, "Cancel", new OnClickListener() {

			@Override
			public void onClick(DialogInterface dialog, int which) {
				fdload.abort();
			}
		});
		pd.show();

		final Builder voicedownloadSuccessStatus = new AlertDialog.Builder(this);
		voicedownloadSuccessStatus.setPositiveButton("Ok",null);


		final String url = "http://tts.speech.cs.cmu.edu/android/vox-flite-1.5.6/"+selectedVoice+".flitevox";
		final String filename = datapath + "/" + voiceParams[2] + ".cg.flitevox";
		new Thread() {
			public void run() { 
				fdload.saveUrlAsFile(url, filename);
				while(fdload.totalFileLength == 0) {
					if (fdload.finished)
						break;
				}
				runOnUiThread(new Runnable() {

					@Override
					public void run() {
						pd.setIndeterminate(false);
						pd.setMax(fdload.totalFileLength);
					}
				});
				int prev = 0;
				while(!fdload.finished) {
					if (fdload.finishedFileLength > prev) {
						prev = fdload.finishedFileLength;
						runOnUiThread(new Runnable() {

							@Override
							public void run() {
								pd.setProgress(fdload.finishedFileLength);
							}
						});
					}
				}
				runOnUiThread(new Runnable() {

					@Override
					public void run() {

						pd.dismiss();

					}
				});
				if(!fdload.success) {
					Log.e(LOG_TAG, "Voice data download failed!");
					if(fdload.abortDownload)
						voicedownloadSuccessStatus.setMessage("Voice download aborted.");
					else
						voicedownloadSuccessStatus.setMessage("Voice download failed! Check your internet settings.");
				}
				else {
					voicedownloadSuccessStatus.setMessage("Voice download succeeded");
				}
				mHandler.post(new Runnable() {

					@Override
					public void run() {
						voicedownloadSuccessStatus.show();
						populateVoiceList();
					}
				});

			}
		}.start();
	}

	private void abort(String str) {
		Log.e(LOG_TAG, str);
		new AlertDialog.Builder(this)
		.setMessage(str)
		.setPositiveButton("Ok",new OnClickListener() {

			@Override
			public void onClick(DialogInterface dialog, int which) {
				finish();				
			}
		})
		.show();
	}

}

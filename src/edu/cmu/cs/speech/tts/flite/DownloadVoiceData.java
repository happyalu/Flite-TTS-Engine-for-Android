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
import java.util.ArrayList;
import android.app.AlertDialog;
import android.app.DownloadManager;
import android.app.ListActivity;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;
import android.widget.ImageButton;
import android.widget.Toast;

/* Download user-requested voice data for Flite
 * 
 */
public class DownloadVoiceData extends ListActivity {
	private final static String LOG_TAG = "Flite_Java_" + DownloadVoiceData.class.getSimpleName();
	private VoiceListAdapter mListAdapter;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		registerReceiver(onComplete,
				new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
		
		mListAdapter = new VoiceListAdapter(this);
		setListAdapter(mListAdapter);
	}

	@Override
	public void onDestroy() {
		super.onDestroy();

		unregisterReceiver(onComplete);
	}

	

	private class VoiceListAdapter extends BaseAdapter {
		
		private Context mContext;
		private ArrayList<Voice> mVoiceList;
		private LayoutInflater mInflater;

		public VoiceListAdapter(Context context) {
			mContext = context;
			mInflater = LayoutInflater.from(mContext);

			// Get Information about voices
			mVoiceList = CheckVoiceData.getVoices();			
		}
		
		public void refresh() {
			mVoiceList = CheckVoiceData.getVoices();
			notifyDataSetChanged();
		}
		
		@Override
		public int getCount() {
			// TODO Auto-generated method stub
			return mVoiceList.size();
		}

		@Override
		public Object getItem(int position) {
			// TODO Auto-generated method stub
			return position;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public View getView(final int position, View convertView, ViewGroup parent) {
			if (convertView == null) {
				convertView = mInflater.inflate(R.layout.view_voice_manager, parent, false);
			}

			((TextView) convertView.findViewById(R.id.voice_manager_voice_language)).setText(mVoiceList.get(position).getDisplayLanguage());
			((TextView) convertView.findViewById(R.id.voice_manager_voice_variant)).setText(mVoiceList.get(position).getVariant());
			final ImageButton actionButton = (ImageButton) convertView.findViewById(R.id.voice_manager_action_image);
			actionButton.setImageResource(
					mVoiceList.get(position).isAvailable()?R.drawable.ic_action_delete:R.drawable.ic_action_download);
			actionButton.setVisibility(View.VISIBLE);

			actionButton.setOnClickListener(new View.OnClickListener() {

				@Override
				public void onClick(View v) {
					final Voice vox = mVoiceList.get(position);
					if (!vox.isAvailable()) {
						AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
						builder.setMessage("Are you sure you want to download voice " + vox.getDisplayName() + "?");
						builder.setCancelable(false);
						builder.setPositiveButton("Download Voice", new DialogInterface.OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog, int which) {
								String url = Voice.getDownloadURLBasePath() + vox.getName() + ".flitevox";
								DownloadManager.Request request = new DownloadManager.Request(Uri.parse(url));
								request.setDescription("Downloading Flite Voice: " + vox.getName());
								request.setTitle("Flite TTS Voice Download");
								request.setDestinationUri(Uri.fromFile(new File(vox.getPath())));

								DownloadManager manager = (DownloadManager) mContext.getSystemService(Context.DOWNLOAD_SERVICE);
								manager.enqueue(request);
								actionButton.setVisibility(View.INVISIBLE);
								
							}
						});
						builder.setNegativeButton("No", new DialogInterface.OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog, int which) {
								dialog.cancel();
							}
						});
						AlertDialog alert = builder.create();
						alert.show();
					}
					else {
						AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
						builder.setMessage("Are you sure you want to delete " + vox.getDisplayName() + "?");
						builder.setCancelable(false);
						builder.setPositiveButton("Delete Voice", new DialogInterface.OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog, int which) {
								File f = new File(vox.getPath());
								if(f.delete()) {
									refresh();
									Toast toast = Toast.makeText(mContext, "Voice Deleted", Toast.LENGTH_SHORT);
									toast.show();
								}
							}
						});
						builder.setNegativeButton("No", new DialogInterface.OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog, int which) {
								dialog.cancel();
							}
						});
						AlertDialog alert = builder.create();
						alert.show();
					}
				}
			});

			return convertView;
		}

	}
	
	BroadcastReceiver onComplete=new BroadcastReceiver() {
		public void onReceive(Context ctxt, Intent intent) {
			Toast toast = Toast.makeText(ctxt, "Flite TTS Voice  Downloaded!", Toast.LENGTH_SHORT);
			toast.show();
			mListAdapter.refresh();
		}
	};

}

/*
	private final static String LOG_TAG = "Flite_Java_" + DownloadVoiceData.class.getSimpleName();	
	private final static String FLITE_DATA_PATH = Voice.getDataPath();

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

	 Look at the voices available in flite, 
 * and compare them to voices already installed.
 * The remaining voices will be available for download.

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
 */
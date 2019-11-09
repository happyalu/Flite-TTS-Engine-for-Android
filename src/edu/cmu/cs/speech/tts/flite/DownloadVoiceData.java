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
import android.view.Menu;
import android.view.MenuItem;
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
	private Context mContext;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		registerReceiver(onComplete,
				new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));

		mListAdapter = new VoiceListAdapter(this);
		setListAdapter(mListAdapter);
		mContext = this;
	}

	@Override
	public void onDestroy() {
		super.onDestroy();

		unregisterReceiver(onComplete);
	}

	public void onResume() {
		super.onResume();
		mListAdapter.refresh();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(R.string.voice_list_update);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Only works for a single menu option.
		// User must have requested a refresh of the voice list.

		Toast toast = Toast.makeText(mContext, "Downloading Voice List", Toast.LENGTH_SHORT);
		toast.show();

		Thread thread = new Thread() {
			@Override
			public void run() {
				CheckVoiceData.DownloadVoiceList(new Runnable() {
					@Override
					public void run() {
						runOnUiThread(new Runnable() {

							@Override
							public void run() {
								mListAdapter.refresh();

							}
						});
					}
				});

			}
		};

		thread.start();
		return true;

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

			if (mVoiceList.isEmpty()) {
				Intent intent = new Intent(mContext, CheckVoiceData.class);
		        startActivity(intent);
			}
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
						builder.setMessage("Data Alert: Download Size up to 3MB.");
						builder.setCancelable(false);
						builder.setPositiveButton("Download Voice", new DialogInterface.OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog, int which) {
								// Create destination directory
								File f = new File
										(vox.getPath());
								f.mkdirs();
								f.delete();
								String url = Voice.getDownloadURLBasePath() + vox.getName() + ".flitevox";
								DownloadManager.Request request = new DownloadManager.Request(Uri.parse(url));
								request.setDescription("Downloading Flite Voice: " + vox.getName());
								request.setTitle("Flite TTS Voice Download");
								request.setDestinationUri(Uri.fromFile(new File(vox.getPath())));

								DownloadManager manager = (DownloadManager) mContext.getSystemService(Context.DOWNLOAD_SERVICE);
								manager.enqueue(request);
								Toast toast = Toast.makeText(mContext, "Download Started", Toast.LENGTH_SHORT);
								toast.show();
								actionButton.setVisibility(View.INVISIBLE);

							}
						});
						builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
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
						builder.setMessage("Sure? Deleting " + vox.getDisplayName());
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
						builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
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

			convertView.setOnClickListener(new View.OnClickListener() {

				@Override
				public void onClick(View v) {
					actionButton.performClick();
				}
			});

			return convertView;
		}

	}

	BroadcastReceiver onComplete=new BroadcastReceiver() {
		public void onReceive(Context ctxt, Intent intent) {
			Toast toast = Toast.makeText(ctxt, "Flite TTS Voice Data Downloaded!", Toast.LENGTH_SHORT);
			toast.show();
			mListAdapter.refresh();
		}
	};

}

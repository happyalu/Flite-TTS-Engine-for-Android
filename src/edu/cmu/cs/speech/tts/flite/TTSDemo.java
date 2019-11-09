/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2012                            */
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
/*               Date:  July 2012                                        */
/*************************************************************************/
package edu.cmu.cs.speech.tts.flite;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import android.annotation.TargetApi;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.OnInitListener;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnKeyListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;

public class TTSDemo extends ListActivity implements OnClickListener, OnKeyListener, OnInitListener {
	private final static String LOG_TAG = "Flite_Java_" + TTSDemo.class.getSimpleName();

	private EditText mUserText;
	private ImageButton mSendButton;
	private ArrayAdapter<String> mAdapter;
	private ArrayAdapter<String> mVoiceAdapter;
    private ArrayAdapter<String> mRateAdapter;
	private ArrayList<Voice> mVoices;
	private ArrayList<String> mStrings = new ArrayList<String>();
    private ArrayList<String> mRates = new ArrayList<String>();
	private Spinner mVoiceSpinner;
    private Spinner mRateSpinner;
	private TextToSpeech mTts;
	private int mSelectedVoice;

	@TargetApi(14)
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		ArrayList<Voice> allVoices = CheckVoiceData.getVoices();
		mVoices = new ArrayList<Voice>();
		for(Voice vox:allVoices) {
			if (vox.isAvailable()) {
				mVoices.add(vox);
			}
		}

		if (mVoices.isEmpty()) {
			// We can't demo anything if there are no voices installed.
			AlertDialog.Builder builder = new AlertDialog.Builder(this);
			builder.setMessage("Flite voices not installed. Please add voices in order to run the demo");
			builder.setNegativeButton("OK", new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					dialog.cancel();
					finish();
				}
			});
			AlertDialog alert = builder.create();
			alert.show();
		}
		else {
			// Initialize the TTS
			if (android.os.Build.VERSION.SDK_INT >=
					android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
				mTts = new TextToSpeech(this, this, "edu.cmu.cs.speech.tts.flite");
			}
			else {
				mTts = new TextToSpeech(this, this);
			}
			mSelectedVoice = -1;

		}
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		if (mTts != null)
			mTts.shutdown();
	}

	private void buildUI() {

		ArrayList<String> voiceNames = new ArrayList<String>();

		for (Voice vox: mVoices) {
		    voiceNames.add(vox.getDisplayName()); // vox.getVariant());
		}

		mVoiceAdapter = new ArrayAdapter<String>(this,
				android.R.layout.simple_spinner_dropdown_item,
				voiceNames);


		setContentView(R.layout.activity_tts_demo);
		mStrings.add("Click an item here to synthesize, or enter your own text below!");
		mStrings.add("A whole joy was reaping, but they've gone south, go fetch azure mike!");
		mStrings.add("हिन्दी संवैधानिक रूप से भारत की प्रथम राजभाषा और भारत की सबसे अधिक बोली और समझी जाने वाली भाषा है।");
		mStrings.add("महाराष्ट्र आणि गोवा ह्या राज्यांची मराठी ही अधिकृत राजभाषा आहे.");
		mStrings.add("Hello World, नमस्कार, வணக்கம், నమస్కారం");

		mAdapter = new InputHistoryAdapter(this, R.layout.list_tts_history, mStrings);

		setListAdapter(mAdapter);

		mRates.add("Very Slow");
		mRates.add("Slow");
		mRates.add("Normal");
		mRates.add("Fast");
		mRates.add("Very Fast");

		mRateAdapter = new ArrayAdapter<String>(this,
							android.R.layout.simple_spinner_dropdown_item,
							mRates);


		mUserText = (EditText) findViewById(R.id.userText);
		mSendButton = (ImageButton) findViewById(R.id.sendButton);

		mVoiceSpinner = (Spinner) findViewById(R.id.voice);
		mVoiceSpinner.setAdapter(mVoiceAdapter);

		mRateSpinner = (Spinner) findViewById(R.id.speechrate);
		mRateSpinner.setAdapter(mRateAdapter);
		mRateSpinner.setSelection(2);

		mUserText.setOnClickListener(this);
		mSendButton.setOnClickListener(this);
		mUserText.setOnKeyListener(this);
	}

	public void onClick(View v) {
		sendText();
	}

	private void sendText() {
		String text = mUserText.getText().toString();
		if (text.isEmpty())
			return;
		mAdapter.add(text);
		mUserText.setText(null);
		sayText(text);
	}

	private void sayText(String text) {
		Log.v(LOG_TAG, "Speaking: " + text);
		int currentVoiceID = mVoiceSpinner.getSelectedItemPosition();
		if (currentVoiceID != mSelectedVoice) {
			mSelectedVoice = currentVoiceID;
			Voice v = mVoices.get(currentVoiceID);
			mTts.setLanguage(v.getLocale());
		}

		int currentRate = mRateSpinner.getSelectedItemPosition();
		mTts.setSpeechRate((float)(currentRate + 1)/3);

		mTts.speak(text, TextToSpeech.QUEUE_FLUSH, null);
	}

	public boolean onKey(View v, int keyCode, KeyEvent event) {
		if (event.getAction() == KeyEvent.ACTION_DOWN) {
			switch (keyCode) {
			case KeyEvent.KEYCODE_DPAD_CENTER:
			case KeyEvent.KEYCODE_ENTER:
				sendText();
				return true;
			}
		}
		return false;
	}

	private class InputHistoryAdapter extends ArrayAdapter<String> {
		private ArrayList<String> items;

		public InputHistoryAdapter(Context context,
				int textViewResourceId, ArrayList<String> items) {
			super(context, textViewResourceId, items);
			this.items = items;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			if (convertView == null) {
				LayoutInflater vi = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
				convertView = vi.inflate(R.layout.list_tts_history, null);
			}
			String s = items.get(position);
			TextView tt = (TextView) convertView.findViewById(R.id.inputText);
			tt.setText(s);
			return convertView;
		}

	}

	@SuppressWarnings("deprecation")
	@Override
	public void onInit(int status) {
		boolean success = true;
		if (status == TextToSpeech.ERROR) {
			success = false;
		}

		if (success &&
				(android.os.Build.VERSION.SDK_INT >=
				android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH)) {
			status = mTts.setEngineByPackageName("edu.cmu.cs.speech.tts.flite");
		}

		if (status == TextToSpeech.ERROR) {
			success = false;
		}

		// REALLY check that it is flite engine that has been initialized
		// This is done using a hack, for now, since for API < 14
		// there seems to be no way to check which engine is being used.

		if (mTts.isLanguageAvailable(new Locale("eng", "USA", "is_flite_available"))
				!= TextToSpeech.LANG_COUNTRY_VAR_AVAILABLE) {
			success = false;
		}

		if (!success) {
			AlertDialog.Builder builder = new AlertDialog.Builder(this);
			builder.setMessage("Flite TTS Engine could not be initialized. Check that Flite is enabled on your phone!. In some cases, you may have to select flite as the default engine.");
			builder.setNegativeButton("Open TTS Settings", new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					dialog.cancel();
					Intent intent = new Intent();
					intent.setComponent(new ComponentName("com.android.settings", "com.android.settings.TextToSpeechSettings"));
			        startActivity(intent);
					finish();
				}
			});
			AlertDialog alert = builder.create();
			alert.show();
		}
		else {
			buildUI();
		}
	}

	@Override
	public void onListItemClick(ListView parent, View view, int position, long id) {
		String text = (String) parent.getItemAtPosition(position);
		sayText(text);

	}
}

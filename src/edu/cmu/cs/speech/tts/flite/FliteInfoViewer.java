package edu.cmu.cs.speech.tts.flite;

import android.app.Activity;
import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;

public class FliteInfoViewer extends ListActivity {
	private final static String LOG_TAG = "Flite_Java_" + FliteInfoViewer.class.getSimpleName();
	private NativeFliteTTS mFliteEngine;
	private SimpleCursorAdapter mAdapter;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mFliteEngine = new NativeFliteTTS(this, null);
		mFliteEngine.setLanguage("eng", "USA","");
		
		ProgressDialog progress = new ProgressDialog(this);
		progress.setMessage("Benchmarking Flite. Wait a few seconds");
		progress.setCancelable(false);
		new GetInformation(progress).execute();
	}
	
	private class GetInformation extends AsyncTask<Void, Void, Void> {

		private ProgressDialog progress;

		public GetInformation(ProgressDialog progress) {
			this.progress = progress;
		}
		
		@Override
		public void onPreExecute() {
			progress.show();
		}
		
		@Override
		public Void doInBackground(Void... arg0) {
			populateInformation();
			return null;
		}
		
		@Override
		public void onPostExecute(Void unused) {
			progress.dismiss();
		}
		
		
	}

	private void populateInformation() {
		final String[] Info = new String[] {"Build ABI", "Benchmark" };
		final String[] Data = new String[] {mFliteEngine.getNativeABI(),
				mFliteEngine.getNativeBenchmark()+" times faster than real time"};
		
		runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
				setListAdapter(new SettingsArrayAdapter(FliteInfoViewer.this, Info, Data));
			}
		});
		
	}
	
	private class SettingsArrayAdapter extends ArrayAdapter<String> {
		private final Context context;
		private final String[] values;
		private final String[] data;
		
		public SettingsArrayAdapter(Context context, String[] values, String[] data) {
			super(context, R.layout.flite_info, values);
			this.context = context;
			this.values = values;
			this.data = data;
		}
		
		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			LayoutInflater inflater = (LayoutInflater) context
				.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	 
			View rowView = inflater.inflate(R.layout.flite_info, parent, false);
			TextView infoType = (TextView) rowView.findViewById(R.id.infotitle);
			TextView infoDetail = (TextView) rowView.findViewById(R.id.infodetail);
			
			infoType.setText(values[position]);
			infoDetail.setText(data[position]);
	 
			return rowView;
		}
		
	}

}

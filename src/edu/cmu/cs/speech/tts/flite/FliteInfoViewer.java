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
	private float mBenchmark = -1; 

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
		if (mBenchmark <0) {
			mBenchmark = mFliteEngine.getNativeBenchmark();
		}
		final String[] Info = new String[] {
				"Copyright",
				"URL",
				"RUNTIME_HEADER",
				"Android Version",
				"Build ABI", 
				"Phone Model",
				"Benchmark",
				};
		final String[] Data = new String[] {	
				"© (1999-2012) Carnegie Mellon University",
				"www.cmuflite.org",
				"",
				android.os.Build.VERSION.RELEASE,
				android.os.Build.CPU_ABI,
				android.os.Build.MODEL,
				mBenchmark + " times faster than real time",

				};
		
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
        public int getViewTypeCount() {
            return 2;
        }
		
		@Override
        public int getItemViewType(int position) {
            if (values[position] == "RUNTIME_HEADER") {
            	return 0;
            }
            else return 1;
        }
		
		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			LayoutInflater inflater = (LayoutInflater) context
				.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			
			if (convertView == null) {
				convertView = inflater.inflate(R.layout.flite_info, parent, false);
			}
			
			TextView infoType = (TextView) convertView.findViewById(R.id.infotitle);
			TextView infoDetail = (TextView) convertView.findViewById(R.id.infodetail);
			
			if (values[position] == "RUNTIME_HEADER") {
				infoType.setText("Runtime Information");
				infoType.setClickable(false);
				
				infoType.setTextColor(getResources().getColor(R.color.themeblue));
				infoType.setPadding(0,20,0,5);
				infoDetail.setVisibility(View.GONE);
			}
			else {
				infoType.setText(values[position]);
				infoDetail.setText(data[position]);
			}
	 
			return convertView;
		}
		
	}

}

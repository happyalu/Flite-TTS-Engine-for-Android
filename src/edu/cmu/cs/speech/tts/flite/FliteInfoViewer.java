package edu.cmu.cs.speech.tts.flite;

import android.app.Activity;
import android.app.ListActivity;
import android.content.Context;
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
		populateInformation();
	}

	private void populateInformation() {
		final String[] Info = new String[] {"Build ABI", "Benchmark" };
		
		setListAdapter(new SettingsArrayAdapter(this, Info));
	}
	
	private class SettingsArrayAdapter extends ArrayAdapter<String> {
		private final Context context;
		private final String[] values;
	 
		
		public SettingsArrayAdapter(Context context, String[] values) {
			super(context, R.layout.flite_info, values);
			this.context = context;
			this.values = values;
		}
		
		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			LayoutInflater inflater = (LayoutInflater) context
				.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	 
			View rowView = inflater.inflate(R.layout.flite_info, parent, false);
			TextView infoType = (TextView) rowView.findViewById(R.id.infotitle);
			TextView infoDetail = (TextView) rowView.findViewById(R.id.infodetail);
			
			infoType.setText(values[position]);
			
	 
			// Change icon based on name
			String s = values[position];
	 
			if (s.equals("Build ABI")) {
				infoDetail.setText(mFliteEngine.getNativeABI());
			} 
			else if (s.equals("Benchmark")) {
				infoDetail.setText(mFliteEngine.getNativeBenchmark()+" times faster than real time");
			}
	 
			return rowView;
		}
		
	}

}

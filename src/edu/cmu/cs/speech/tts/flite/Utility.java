package edu.cmu.cs.speech.tts.flite;

import java.io.BufferedInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import android.util.Log;

public class Utility {
    
    public static boolean saveUrlAsFile(String url, String filename) {
    	try {
    		//TODO (aup): Improve the exception handling. This is cruel.
    		Log.v("Flite.Utility","Trying to save "+url+" as "+filename);
    		URL u = new URL(url);
    		URLConnection uc = u.openConnection();
    		String contentType = uc.getContentType();
    		int contentLength = uc.getContentLength();

    		InputStream raw = uc.getInputStream();
    		InputStream in = new BufferedInputStream(raw,8000);
    		byte[] data = new byte[contentLength];
    		int bytesRead = 0;
    		int offset = 0;
    		while (offset < contentLength) {
    			bytesRead = in.read(data, offset, data.length - offset);
    			if (bytesRead == -1)
    				break;
    			offset += bytesRead;
    		}
    		in.close();

    		if (offset != contentLength) {
    			throw new IOException("Only read " + offset + " bytes; Expected " + contentLength + " bytes");
    		}

    		FileOutputStream out = new FileOutputStream(filename);
    		out.write(data);
    		out.flush();
    		out.close();
    		return true;
    	}
    	catch (Exception e) {
    		Log.e("Flite Utility","Could not save url as file.: "+e.getMessage());
    		return false;
    	}
    }
}  

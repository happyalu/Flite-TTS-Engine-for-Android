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

import java.io.BufferedInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.List;
import java.util.ArrayList;

import android.util.Log;

public class FileDownloader {
  public int totalFileLength = 0;
  public int finishedFileLength;
	
  public boolean abortDownload;
  public boolean finished;
  public boolean success;
	
  public void saveUrlAsFile(final String url, final String filename) {
    finished = false;
    success = false;
    new Thread() {
      public void run() {
        save(url, filename);
      }
    }.start();
  }
		
  private boolean save(String url, String filename) {
    try {
      //TODO (aup): Improve the exception handling. This is cruel.
    		
      abortDownload = false;
    		
      Log.v("Flite.FileDownloader","Trying to save "+url+" as "+filename);
      URL u = new URL(url);
      URLConnection uc = u.openConnection();

      uc.setDoInput(true);
      uc.setDoOutput(true);
      uc.getOutputStream();
                
      int contentLength = uc.getContentLength();

      totalFileLength = contentLength;
      finishedFileLength = 0;

      InputStream raw = uc.getInputStream();
      InputStream in = new BufferedInputStream(raw,256);
      FileOutputStream out = new FileOutputStream(filename);

      int nextByte = 0;

      while (nextByte != -1) {
        nextByte = in.read();
        if (nextByte == -1)
          break;    			
        finishedFileLength += 1;
        out.write((byte)nextByte);
        if(abortDownload)
          break;
      }
      Log.v("Flite.FileDownloader", "FinishedFileLength: " + finishedFileLength);
      in.close();

      out.flush();
      out.close();
                
      if(abortDownload) {
        Log.e("Flite.FileDownloader", "File download aborted by user");
        success = false;
        finished = true;
        new java.io.File(filename).delete();
        return false;
      }
    		
      if ((contentLength > 0) && (finishedFileLength != contentLength)) {
        throw new IOException("Only read " + finishedFileLength + " bytes; Expected " + contentLength + " bytes");
      }
                
      finished = true;
      success = true;
      return true;
    }
    catch (Exception e) {
      Log.e("Flite Utility","Could not save url as file.: "+e.getMessage());
      finished = true;
      return false;
    }
  }

  public void abort() {
    abortDownload = true;
  }
	
}

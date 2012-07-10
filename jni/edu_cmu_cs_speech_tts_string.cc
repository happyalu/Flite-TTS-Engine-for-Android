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
/*               Date:  March 2010                                       */
/*************************************************************************/
/*                                                                       */
/*  Library classes to provide basic the missing String support          */
/*                                                                       */
/*************************************************************************/

#include "./edu_cmu_cs_speech_tts_string.h"
#include<string.h>
#include<stdio.h>

namespace FliteEngine {

String::String(const char* str) {
  if (str != NULL) {
    str_data_ = new char[strlen(str) + 1];
    if (str_data_ != NULL)
      snprintf(str_data_, strlen(str) + 1, "%s", str);
  } else {
    str_data_ = NULL;
  }
}

String::String(const String& rhs) {
  if (rhs.str_data_ == NULL) {
    str_data_ = NULL;
  } else {
    str_data_ = new char[strlen(rhs.str_data_)+1];
    if (str_data_ != NULL)
      snprintf(str_data_, strlen(rhs.str_data_) + 1, "%s", rhs.str_data_);
  }
}

String::~String() {
  if (str_data_ != NULL) {
    delete[] str_data_;
  }
}

String& String::operator=(const String &rhs) {
  // Don't assign if rhs is the same object!
  if (this != &rhs) {
    if (str_data_ != NULL)
      delete[] str_data_;
    str_data_ = new char[strlen(rhs.str_data_)+1];
    if (str_data_ != NULL)
      snprintf(str_data_, strlen(rhs.str_data_) + 1, "%s", rhs.str_data_);
  }
  return *this;
}

String String::operator+(const String &other) {
  int newlength;
  char* s;

  if (str_data_ == NULL)
    return other;

  if (other.str_data_ == NULL)
    return *this;

  newlength = strlen(str_data_) + strlen(other.str_data_) + 1;

  s = new char[newlength];
  snprintf(s, newlength, "%s%s", str_data_, other.str_data_);

  String addedStr = String(s);
  delete[] s;
  return addedStr;
}

bool String::operator==(const String &other) const {
    if ( (str_data_ == NULL) or (other.str_data_ == NULL) )
      return false;

    if (strcmp(str_data_, other.str_data_) == 0)
      return true;
    else
      return false;
}

const char* String::c_str() {
    return str_data_;
}
}  // End Namespace

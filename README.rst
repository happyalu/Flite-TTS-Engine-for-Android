============================
Flite TTS Engine for Android
============================

:Author: Alok Parlikar

Introduction
============

Flite (festival-lite) is a small, fast runtime speech synthesis engine
developed at Carnegie Mellon University and primarily designed for
small embedded devices. Flite is an opensource synthesis engine that
can be used to provide text-to-speech functionality on smartphones and
similar devices. For more information about Flite, visit www.cmuflite.org

This application wraps the Flite engine so that it can be installed as
an application on Android devices. This app gets installed as a TTS
library, thereby allowing other applications to make use of Flite for
synthesis. 

Installing this Application
===========================

This app is not yet available on the Google Play Store, but you can
generate an apk file by building this source code, or download an APK
file from the Github page and install it on your device. Devices
running Android versions 2.2 (Froyo) or later are supported.

After installing this application, you will have to:

1. Enable Flite from "Text to Speech Settings" on your phone
2. Install voice data
3. Go to "Language" settings and select the voice that you want.

By clicking on "Listen to a sample" you can verify that Flite is
working.

Using Flite for TTS in Your Application
=======================================

If you are developing an application and would like to use Flite for
speech synthesis, you can specify "edu.cmu.cs.speech.tts.flite" as the
package name of the engine to use.

Building this App from Source
=============================

Instructions here are for development on Linux. 

Requirements
------------
In order to build this application, you need the following:

- Flite 1.5.6 or later. This latest release is not available for
  download on the Flite website. A version can be downloaded from
  http://tts.speech.cs.cmu.edu/aup/distr/android/flite-1.5.6-current.tar.bz2

- Android NDK Release 8
- Android SDK Release 20

Application Build Steps
-----------------------

*Export necessary environment variables* ::

    export FLITEDIR=/path/to/flite-1.5.6
    export FLITE_APP_DIR=/path/to/this/application
    export ANDROID_NDK=/path/to/android/ndk
    export ANDROID_SDK=/path/to/android/sdk

*Build Flite Engine for multiple architectures* ::

    cd $FLITEDIR
    ./configure --with-langvox=android --target=armeabi-android
    make
    ./configure --with-langvox=android --target=armeabiv7a-android
    make
    ./configure --with-langvox=android --target=x86-android
    make
    ./configure --with-langvox=android --target=mips-android
    make

*Build installable application package (debug mode)* ::

    cd $FLITE_APP_DIR
    ant debug


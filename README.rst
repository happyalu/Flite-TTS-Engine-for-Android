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
running Android versions 2.3 (Gingerbread) or later are supported.

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

Build this App from Source in 2019
==================================

To build this App from source in 2019, you will have to modify the 10e ndk, 
the sdk, install an older version of JDK (8 will do) and install ant.

---------------------
modify the 10e ndk
---------------------
#the Android build script are tailored for ndk r10d but that ndk is no longer 
#available. You'll have to modify the r10e ndk so that it would work with them. 

#Step 1: download ndk
#Step 2: extract archive
#Step 3: browse to to .../android-ndk-r10e/toolchains. 
#Step 4: or every folder ending with 4.8 (make a link with same name but end it 
#   with 4.6 instead). This can be done via CLI: 

ln -s arm-linux-androideabi-4.8 arm-linux-androideabi-4.6
ln -s mipsel-linux-android-4.8 mipsel-linux-android-4.6
ln -s x86-4.8 x86-4.6

#Step 6: for each of the linked folders, link linux_x86_64 to linux_x86 or you can 
#   execute this command from the toolchains folder (from the same folder as above):

ln -s $PWD/arm-linux-androideabi-4.8/prebuilt/linux-x86_64 $PWD/arm-linux-androideabi-4.6/prebuilt/linux-x86
ln -s $PWD/mipsel-linux-android-4.8/prebuilt/linux-x86_64 $PWD/mipsel-linux-android-4.6/prebuilt/linux-x86
ln -s $PWD/x86-4.8/prebuilt/linux-x86_64 $PWD/x86-4.6/prebuilt/linux-x86

-------------------
modify your sdk
-------------------
#Sdk/tools/ant/build.xml is no longer included in your Sdk/tools folder, you'll need to
#   download it seperately 

#Step 1: 
wget https://dl.google.com/android/repository/tools_r24.0.2-linux.zip

#Step 2: backup unmessed with SDK/tools so that you may restore it after all of this:
cp ~/Android/Sdk/tools ~/Android/Sdk/tools.bak

#Step 3: extract downloaded zip and copy the tools folder within it:
cp ./tools_r24.0.2-linux/tools ~/Android/Sdk/tools

-----------------------------
install missing ant & JDK8
-----------------------------
sudo apt-get install openjdk-8-jdk ant
export PATH=/usr/lib/jvm/java-8-openjdk-amd64/bin:$PATH
#you'll need redo export if you open a new tab or terminal.

Building this App from Source using the Build script
====================================================

Instructions here are for development on Linux.

Requirements
------------
In order to build this application, you need the following:

- Android NDK Release 10e or newer
- Android SDK with the SDK Platform 24.0.2

Application Build Steps
-----------------------

*Export necessary environment variables* ::

    export ANDROID_NDK=/path/to/android/ndk
    export ANDROID_SDK=/path/to/android/sdk
    bash ./build.sh

*Wait for the build script do download, verify, extract, patch and build the Flite Engine and App.*

The final ``apk`` will be placed in ``bin/FliteEngine-debug.apk``.

Building this App from Source manually
======================================

Instructions here are for development on Linux.

Requirements
------------
In order to build this application, you need the following:

- Flite 2.0.0 or later.  A version can be downloaded from
  http://www.festvox.org/flite/packed/flite-2.0/flite-2.0.0-release.tar.bz2

- Android NDK Release 10e
- Android SDK Release 24.0.2

Application Build Steps
-----------------------

*Export necessary environment variables* ::

    export FLITEDIR=/path/to/flite-2.0.0
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

*Build installable application package (release)* ::

    keytool -genkey -v -keystore flite.keystore -alias flite_release -keyalg RSA -keysize 2048 -validity 10000
    ant release
    jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore flite.keystore bin/FliteEngine-release-unsigned.apk flite_release

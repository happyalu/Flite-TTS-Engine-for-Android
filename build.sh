#!/bin/sh
FLITE_APP_DIR="$(dirname $(readlink -f "$0"))"
FLITE_BUILD_DIR="${FLITE_APP_DIR}/flite"
FLITE_ARCHIVE_URL="http://www.festvox.org/flite/packed/flite-2.0/flite-2.0.0-release.tar.bz2"
FLITE_ARCHIVE_MD5="645db96ffc296cbb6d37f231cc1cc6b2"
FLITE_ARCHIVE_NAM="$(basename "${FLITE_ARCHIVE_URL}")"
FLITE_ARCHIVE_DIR="$(basename "${FLITE_ARCHIVE_URL}" .tar.bz2)"

FLITE_PATCH='
--- flite-2.0.0-release/configure.in	2014-12-09 22:39:01.000000000 +0100
+++ flite-2.0.0-release.new/configure.in	2016-06-15 12:36:53.543619329 +0200
@@ -151,16 +151,22 @@
                 # http://developer.android.com/sdk/ndk/index.html
 		shared=false
 
+		if test -z "${ANDROID_NDK}";
+		then
+			ANDROID_NDK="${ANDROID_NDK_HOME}"
+		fi
+
 		# We target our compilation to android-14 (4.0) platform
 		ANDROID_NDK_PLATFORM_PATH="$ANDROID_NDK/platforms/android-14"
 
-		ANDROID_GCC_VERSION=4.6
+		ANDROID_GCC_ARCH="$(uname -m)"
+		ANDROID_GCC_VERSION=4.9
 
 		if test "$target_cpu" = "armeabi" 
                 then
 		   ANDROID_TOOLCHAIN="arm-linux-androideabi-$ANDROID_GCC_VERSION"
 		   ANDROID_NDK_SYSROOT="$ANDROID_NDK_PLATFORM_PATH/arch-arm"
-		   ANDROIDBIN="$ANDROID_NDK/toolchains/arm-linux-androideabi-$ANDROID_GCC_VERSION/prebuilt/linux-x86/bin/arm-linux-androideabi"
+		   ANDROIDBIN="$ANDROID_NDK/toolchains/arm-linux-androideabi-$ANDROID_GCC_VERSION/prebuilt/linux-$ANDROID_GCC_ARCH/bin/arm-linux-androideabi"
       		   CFLAGS="$CFLAGS -fpic -mthumb -O3 -DANDROID --sysroot=$ANDROID_NDK_SYSROOT"
                 fi
 
@@ -168,7 +174,7 @@
                 then
 		   ANDROID_TOOLCHAIN="arm-linux-androideabi-$ANDROID_GCC_VERSION"
 		   ANDROID_NDK_SYSROOT="$ANDROID_NDK_PLATFORM_PATH/arch-arm"
-		   ANDROIDBIN="$ANDROID_NDK/toolchains/arm-linux-androideabi-$ANDROID_GCC_VERSION/prebuilt/linux-x86/bin/arm-linux-androideabi"
+		   ANDROIDBIN="$ANDROID_NDK/toolchains/arm-linux-androideabi-$ANDROID_GCC_VERSION/prebuilt/linux-$ANDROID_GCC_ARCH/bin/arm-linux-androideabi"
       		   CFLAGS="$CFLAGS -fpic -march=armv7-a -mfloat-abi=softfp -O3 -DANDROID --sysroot=$ANDROID_NDK_SYSROOT"
 		   LDFLAGS="$LDFLAGS -Wl,--fix-cortex-a8"
                 fi
@@ -177,7 +183,7 @@
 		then
 		   ANDROID_TOOLCHAIN="x86-$ANDROID_GCC_VERSION"
 		   ANDROID_NDK_SYSROOT="$ANDROID_NDK_PLATFORM_PATH/arch-x86"
-		   ANDROIDBIN="$ANDROID_NDK/toolchains/x86-$ANDROID_GCC_VERSION/prebuilt/linux-x86/bin/i686-linux-android"
+		   ANDROIDBIN="$ANDROID_NDK/toolchains/x86-$ANDROID_GCC_VERSION/prebuilt/linux-$ANDROID_GCC_ARCH/bin/i686-linux-android"
       		   CFLAGS="$CFLAGS -fpic -O3 -DANDROID --sysroot=$ANDROID_NDK_SYSROOT"
 		fi
 
@@ -185,7 +191,7 @@
 		then
 		   ANDROID_TOOLCHAIN="mipsel-linux-android-$ANDROID_GCC_VERSION"
 		   ANDROID_NDK_SYSROOT="$ANDROID_NDK_PLATFORM_PATH/arch-mips"
-		   ANDROIDBIN="$ANDROID_NDK/toolchains/mipsel-linux-android-$ANDROID_GCC_VERSION/prebuilt/linux-x86/bin/mipsel-linux-android"
+		   ANDROIDBIN="$ANDROID_NDK/toolchains/mipsel-linux-android-$ANDROID_GCC_VERSION/prebuilt/linux-$ANDROID_GCC_ARCH/bin/mipsel-linux-android"
       		   CFLAGS="$CFLAGS -fpic -O3 -DANDROID --sysroot=$ANDROID_NDK_SYSROOT"
 		fi
';

# Abort after first error
set -e
OLDPWD="${PWD}"

# Check for required environment vairables
HAVE_REQUIRED_ENVS=true

if [ -z "${ANDROID_NDK}" ] && [ -z "${ANDROID_NDK_HOME}" ];
then
	echo "Missing Android NDK path environment variable: ANDROID_NDK / ANDROID_NDK_HOME" >&2
	HAVE_REQUIRED_ENVS=false
fi

if [ -z "${ANDROID_SDK}" ] && [ -z "${ANDROID_HOME}" ];
then
	echo "Missing Android SDK path environment variable: ANDROID_SDK / ANDROID_HOME" >&2
	HAVE_REQUIRED_ENVS=false
fi

if ! ${HAVE_REQUIRED_ENVS};
then
	exit 1
fi

export FLIGHT_APP_DIR

# Download and patch flight, unless it was provided
if [ -z "${FLITEDIR}" ];
then
	# Check if the actual `flight` build directory exists
	flite_directory="${FLITE_BUILD_DIR}/${FLITE_ARCHIVE_DIR}"
	if ! [ -d "${flite_directory}" ];
	then
		mkdir -p "${FLITE_BUILD_DIR}"
		
		flite_archive="${FLITE_BUILD_DIR}/${FLITE_ARCHIVE_NAM}"
		
		# Download the `flight` file archive
		if ! [ -e "${flite_archive}" ];
		then
			wget "${FLITE_ARCHIVE_URL}" -O "${flite_archive}"
		fi
		
		# Verify the archive's integrity
		echo "${FLITE_ARCHIVE_MD5}  ${flite_archive}" | md5sum --check -
		
		# Extract the `flight` file archive
		tar -C "${FLITE_BUILD_DIR}" -xvf "${flite_archive}"
		
		# Patch `flight` configure script to work with newer versions of the NDK (10e – 11c tested)
		cd "${flite_directory}"
		echo "${FLITE_PATCH}" | patch -p 1
		autoreconf
		cd "${OLDPWD}"
		
	fi
	
	export FLITEDIR="${flite_directory}"
fi

# Build `flight` engine for all supported targets
cd "${FLITEDIR}"
for arch in armeabi armeabiv7a x86 mips;
do
	if ! [ -e "${FLITEDIR}/build/${arch}-android/lib/libflite.a" ];
	then
		./configure --with-langvox=android --target="${arch}-android"
		make -j4
	fi
done
cd "${OLDPWD}"

# Build the Android application package
if [ $# -gt 0 ];
then
	action="${1}"
	shift 1
else
	action="debug"
fi
ant "${action}" "$@"

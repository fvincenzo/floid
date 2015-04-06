## Getting the source code ##

Getting the source code consist on three steps:
  1. download Android source code from Google repository
  1. install the SPEAr1340 specific parts
  1. download and unpack proprietary libraries
  1. patch the Android sources

### 1. Download Android source code ###

To install, initialize, and configure the Android source code repository , follow these steps:

Make sure you have a bin/ directory in your home directory, and that it is included in your path:

<pre>
$ mkdir ~/bin<br>
$ PATH=~/bin:$PATH<br>
</pre>
Download the Repo script and ensure it is executable:

<pre>
$ curl https://dl-ssl.google.com/dl/googlesource/git-repo/repo > ~/bin/repo<br>
$ chmod a+x ~/bin/repo<br>
</pre>

Create an empty directory (`<SPEAR_ANDROID>`) to hold working files and use the repo tool to download AOSP source:
<pre>$ repo init -u https://android.googlesource.com/platform/manifest -b android-4.0.3_r1</pre>
To pull down files to your working directory from the repositories, run:
<pre>$ repo sync</pre>

### 2. Install the SPEAr1340 specific parts ###

Here we will clone specific device tree from git

<pre>
$ cd <SPEAR_ANDROID><br>
$ mkdir -p device/stm<br>
$ cd device/stm<br>
$ git clone https://code.google.com/p/floid/ .<br>
$ git checkout -b android_4.0.3 origin/android_4.0.3<br>
$ cd ../..<br>
$ mkdir -p hardware/stm<br>
$ cd hardware/stm<br>
$ git clone https://code.google.com/p/floid.hardware/ .<br>
$ git checkout -b android_4.0.3 origin/android_4.0.3<br>
</pre>

A BusyBox binary (v1.16.1) is included in the floid project, where the source code can be downloaded from [busybox.net](http://busybox.net/downloads/).

### 3. Download and unpack proprietary libraries ###

Please download the binaries archive directly from [STMicroelectronics server](http://www.stlinux.com/SPEAr1340HW). Once downloaded you can unpack the archive and copy the files to the correct location. The following instructions assume that the archive was downloaded in the home directory of the current user.

<pre>
$ gpg --output /tmp/hwpack_android-4.0.3_1.0.run --decrypt ~/hwpack_android-4.0.3_1.0.run.gpg<br>
$ cd /tmp<br>
$ chmod +x hwpack_android-4.0.3_1.0.run<br>
$ ./hwpack_android-4.0.3_1.0.run<br>
$ cp binaries/arm/* <SPEAR_ANDROID>/device/stm/SPEAr1340/hardware/arm/prebuilt/<br>
$ cp binaries/hantro/* <SPEAR_ANDROID>/hardware/stm/mm/omx/omxil/prebuilt<br>
</pre>

### 4. Patch the Android sources & Install new application ###
  * apply to patch files base on Android ICS source code.
**1. patch wpa\_supplicant\_6**
<pre>
$ cd <SPEAR_ANDROID>/external/wpa_supplicant_6<br>
$ wget http://floid.googlecode.com/files/external_wpa_supplicant_6-2012-08-02.zip<br>
$ unzip external_wpa_supplicant_6-2012-08-02.zip<br>
$ git am -3 0001-Added-ST-Micro-patch-for-Android-private-ioctl.patch<br>
$ git am -3 0002-Update-wpa_states-to-match-ICS-states.patch<br>
$ git am -3 0003-Removed-some-logging.patch<br>
$ git am -3 0004-Disabled-logging-in-build-config.patch<br>
</pre>
**2. patch to external webkit**
<pre>
$ cd <SPEAR_ANDROID>/external/webkit<br>
$ wget http://floid.googlecode.com/files/externel_webkit-2012-08-02.tar.bz2<br>
$ tar xfv externel_webkit-2012-08-02.tar.bz2<br>
$ git am -3 0001-B0155-Youtube-player-controls-work-only-in-FS.patch<br>
$ git am -3 0002-B0155-v2-Fix-browser-crash.patch<br>
</pre>
**3. patch to frameworks base**
<pre>
$ cd <SPEAR_ANDROID>/frameworks/base<br>
$ wget http://floid.googlecode.com/files/frameworks_base-2013-02-08.tar.bz2<br>
$ tar xfv frameworks_base-2013-02-08.tar.bz2<br>
$ git am -3 0001-Added-ST-Micro-calibration-patches.patch<br>
$ git am -3 0002-Enabling-Hantro-OMX-component-support-and-decoding.patch<br>
$ git am -3 0003-B0108-Fixed-testDispatchTouchEvent-fail.patch<br>
$ git am -3 0004-Minor-video-related-fixes.patch<br>
$ git am -3 0005-camera-merged-http-review.omapzoom.org-change-14346-.patch<br>
$ git am -3 0006-Added-support-for-packed-semi-planar-yuv420.patch<br>
$ git am -3 0007-Video-rendering-performance-improvements.patch<br>
$ git am -3 0008-B0093-CTS-failures-in-android.bluetooth.patch<br>
$ git am -3 0009-Fixed-problem-with-no-video-displayed-for-RTSP.patch<br>
$ git am -3 0010-Added-UMS-support.patch<br>
$ git am -3 0011-Added-LUN-file-definition-inside-configuration-xml-f.patch<br>
$ git am -3 0012-Fix-framebuffer-corruption-when-rotate-device.patch<br>
$ git am -3 0013-UMS-support-enable-mass-storage-configuration-menu.patch<br>
$ git am -3 0014-UMS-support-fix-build-issue.patch<br>
$ git am -3 0015-Added-Release-Note-for-STM-Drop-on-1031.patch<br>
</pre>
**4. patch to system core**
<pre>
$ cd <SPEAR_ANDROID>/system/core<br>
$ wget http://floid.googlecode.com/files/system_core-2013-02-08.tar.bz2<br>
$ tar xfv system_core-2013-02-08.tar.bz2<br>
$ git am -3 0001-Add-support-for-32-bit-framebuffer.patch<br>
$ git am -3 0002-Video-rendering-performance-improvements.patch<br>
$ git am -3 0003-Added-STM-vendor-ID-in-the-USB-ID-list.patch<br>
$ git am -3 0004-Added-Release-Note-for-STM-Drop-on-1031.patch<br>
</pre>
**5. patch to system bluetooth**
<pre>
$ cd <SPEAR_ANDROID>/system/bluetooth<br>
$ wget http://floid.googlecode.com/files/system_bluetooth-2012-07-19.tar.bz2<br>
$ tar xfv system_bluetooth-2012-07-19.tar.bz2<br>
$ git am -3 0001-B0093-Fixed-the-android.bluetooth-of-CTS.patch<br>
</pre>
**6. patch to system vold**
<pre>
$ cd <SPEAR_ANDROID>/system/vold<br>
$ wget http://floid.googlecode.com/files/system_vold-2013-02-08.tar.bz2<br>
$ tar xfv system_vold-2013-02-08.tar.bz2<br>
$ git am -3 0001-LUN-file-path-fix-lun-file-path.patch<br>
$ git am -3 0002-Enable-mass-storage-mode.patch<br>
$ git am -3 0003-Added-Release-Note-for-STM-Drop-on-1031.patch<br>
</pre>
**7. install to tscalibration app**
<pre>
$ cd <SPEAR_ANDROID>/packages/apps<br>
$ mkdir tscalibration<br>
$ cd tscalibration<br>
$ git init<br>
$ wget http://floid.googlecode.com/files/packages_apps_tscalibration-2012-08-02.zip<br>
$ unzip packages_apps_tscalibration-2012-08-02.zip<br>
$ git am -3 0001-Adding-tscalibration.patch<br>
$ git am -3 0002-Do-not-build-tscalibration-for-eng.patch<br>
</pre>
**9. patch to Camera app**
<pre>
$ cd <SPEAR_ANDROID>/packages/apps/Camera<br>
$ wget http://floid.googlecode.com/files/packages_apps_Camera-2012-08-02.tar.bz2<br>
$ tar xfv packages_apps_Camera-2012-08-02.tar.bz2<br>
$ git am -3 0001-B0134-Camera-preview-overlaps-the-shot-button.patch<br>
$ git am -3 0002-B0154-No-resolution-option-in-Camera.patch<br>
</pre>
**10. patch to Music app**
<pre>
$ cd <SPEAR_ANDROID>/packages/apps/Music<br>
$ wget http://floid.googlecode.com/files/packages_apps_Music-2012-08-02.tar.bz2<br>
$ tar xfv packages_apps_Music-2012-08-02.tar.bz2<br>
$ git am -3 0001-B0122-MusicPlayer_Launch_Performance-fail.patch<br>
</pre>
**11. patch to Settings app**
<pre>
$ cd <SPEAR_ANDROID>/packages/apps/Settings<br>
$ wget http://floid.googlecode.com/files/packages_apps_Settings-2013-02-08.tar.bz2<br>
$ tar xfv packages_apps_Settings-2013-02-08.tar.bz2<br>
$ git am -3 0001-Added-the-USB-Mass-Storage-entry-into-setting-menu.patch<br>
$ git am -3 0002-Fix-USB-device-mode-selection-logic.patch<br>
$ git am -3 0003-Added-Release-Note-for-STM-Drop-on-1031.patch<br>
</pre>
**12. patch to Ubuntux64 for Building x64**
<pre>
$ cd <SPEAR_ANDROID><br>
$ wget http://floid.googlecode.com/files/Ubuntux64-2012-08-02.tar.bz2<br>
$ tar xfv Ubuntux64-2012-08-02.tar.bz2<br>
$ cd ./build<br>
$ git am -3 ../0001-build-core-combo-fix-Fixed-building-on-Ubuntu-x64.patch<br>
$ cd - ; cd external/gtest<br>
$ git am -3 ../../0001-external-gtest-include-gtest-fix-Fixed-building-on-U.patch<br>
$ cd - ; cd external/llvm<br>
$ git am -3 ../../0001-external-llvm-fix-Fixed-building-on-Ubuntu-x64.patch<br>
$ cd - ; cd external/mesa3d<br>
$ git am -3 ../../0001-external-mesa3d-src-glsl-fix-Fixed-building-on-Ubunt.patch<br>
$ cd - ; cd external/oprofile<br>
$ git am -3 ../../0001-external-oprofile-libpp-fix-Fixed-building-on-Ubuntu.patch<br>
$ cd - ; cd frameworks/compile/slang<br>
$ git am -3 ../../../0001-frameworks-compile-slang-fix-Fixed-building-on-Ubunt.patch<br>
$ cd - ; cd sdk/emulator/qtools<br>
$ git am -3 ../../../0001-sdk-emulator-qtools-fix-Fixed-building-on-Ubuntu-x64.patch<br>
</pre>


### Download the kernel source ###

The kernel source code is available from ST Microelectronics public git repository:

<pre>
$ cd <SPEAR_ANDROID><br>
$ mkdir kernel<br>
$ cd kernel<br>
$ git clone http://git.stlinux.com/spear/android-2.6.git .<br>
$ git checkout -b next origin/next<br>
</pre>
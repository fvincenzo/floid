### Getting the source code ###

Getting the source code consist on three steps:
  1. download Android source code from Google repository
  1. install the SPEAr1340 specific parts
  1. patch the Android sources in order to integrate hardware decoder

#### Download Android source code ####

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
<pre>$ repo  init -u https://android.googlesource.com/platform/manifest -b android-2.3.7_r1</pre>
To pull down files to your working directory from the repositories, run:
<pre>$ repo  sync</pre>

#### Install the SPEAr1340 specific parts ####

Here we will clone specific device tree from git

<pre>
$ cd <SPEAR_ANDROID><br>
$ mkdir -p device/stm<br>
$ cd device/stm<br>
$ git clone https://code.google.com/p/floid/ .<br>
$ cd ../..<br>
$ mkdir -p hardware/stm<br>
$ cd hardware/stm<br>
$ git clone https://code.google.com/p/floid.hardware/ .<br>
</pre>

#### Install the ALSA libraries ####

Here we will clone the ALSA libs (no longer part of AOSP):

<pre>
$ cd <SPEAR_ANDROID><br>
$ mkdir -p external/alsa-utils<br>
$ cd external/alsa-utils<br>
$ git clone https://code.google.com/p/floid.alsa-utils/ .<br>
$ cd ../..<br>
$ mkdir -p external/alsa-lib<br>
$ cd external/alsa-lib<br>
$ git clone https://code.google.com/p/floid.alsa-lib/ .<br>
</pre>

### Download and unpack proprietary libraries ###

Please download the binaries archive directly from [STMicroelectronics server](http://www.stlinux.com/SPEAr1340HW). Once downloaded you can unpack the archive and copy the files to the correct location. The following instructions assume that the archive was downloaded in the home directory of the current user.

<pre>
$ gpg --output /tmp/hwpack_android-2.3.7_1.0.run --decrypt ~/hwpack_android-2.3.7_1.0.run.gpg<br>
$ cd /tmp<br>
$ chmod +x hwpack_android-2.3.7_1.0.run<br>
$ ./hwpack_android-2.3.7_1.0.run<br>
$ cp binaries/arm/* <SPEAR_ANDROID>/device/stm/SPEAr1340/hardware/arm/mali/<br>
$ cp binaries/hantro/* <SPEAR_ANDROID>/hardware/stm/mm/omx/omxil/prebuilt<br>
</pre>

#### Add support for hardware decoder ####

Patch the libstagefright source code:

<pre>
$ cd <SPEAR_ANDROID>/frameworks/base<br>
$ wget http://floid.googlecode.com/files/frameworks_base_20120323.zip<br>
$ unzip frameworks_base_20120323.zip<br>
$ git am -3 0001-Verisilicon-intergation.patch<br>
$ git am -3 0002-Added-VPX-and-H263-HW-codecs.patch<br>
$ git am -3 0003-Reduced-size-of-decoded-image-and-changed.patch<br>
$ git am -3 0004-Increased-the-window-for-early-and-late-frames.patch<br>
$ git am -3 0005-Enabled-the-use-of-Surface-instead-of.patch<br>
$ git am -3 0006-Fixed-problem-with-incorrect-format.patch<br>
$ git am -3 0007-Disabled-push-buffers-for-all-surfaces.-This.patch<br>
$ git am -3 0008-Fixed-problem-with-incorrect-rendering.patch<br>
</pre>

#### Add support for USB mouse for HDMI configuration ####

You will need to apply a patch if you want to use the HDMI configuration. The patch is available in the same archive as above so use the above steps to download and unpack.

<pre>
$ git apply --check 0009-Merge-mouse-support-from-android-x86-project.patch<br>
$ git am -3 0009-Merge-mouse-support-from-android-x86-project.patch<br>
</pre>

#### Add support for resistive touchscreen and inject permission for SoftKey app ####

<pre>
$ git am -3 0010-Add-permission-to-inject-events-to-root.patch<br>
$ git am -3 0011-Add-support-for-resistive-touchscreen.patch<br>
</pre>


#### Add SoftKey and tsCalibration applications ####

<pre>
$ cd <SPEAR_ANDROID>/packages/apps<br>
$ wget http://floid.googlecode.com/files/SoftKey.zip<br>
$ unzip SoftKey.zip<br>
$ wget http://floid.googlecode.com/files/tscalibration.zip<br>
$ unzip tscalibration.zip<br>
</pre>


#### Apply wpa\_supplicant patch for loadable module ####

<pre>
$ cd <SPEAR_ANDROID>/external/wpa_supplicant_6<br>
$ wget http://floid.googlecode.com/files/wpa_supplicant-20120323.zip<br>
$ unzip wpa_supplicant-20120323.zip<br>
$ git am -3 0001-Enable-USB-key-to-work-using-loadable-module.patch<br>
</pre>

#### Download the kernel source ####

The kernel source code is available from ST Microelectronics public git repository:

<pre>
$ cd <SPEAR_ANDROID><br>
$ mkdir kernel<br>
$ cd kernel<br>
$ git clone http://git.stlinux.com/spear/android-2.6.git .<br>
$ git checkout -b next origin/next<br>
$ git checkout master<br>
$ git reset --hard android-2.3.7_1.0<br>
</pre>

Note: The _master_ branch is the validated one. Pay attention to not use _next_, it is an under development branch.
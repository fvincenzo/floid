## Building instructions for USB boot /NAND boot ##

There are 4 steps, the first one only if you need to use your own kernel instead of the pre-built one, and the third one if you need to change any configuration in bootloader.

> #### 1. Select mode for boot mode configuration (for 7", 10", HDMI) ####
> #### 2. Compile kernel source code (and modules) ####
> #### 3. Compile Android source code ####
> #### 4. Copy Build image into USB key/SD card. ####

### < 1. Select mode for boot mode configuration (for 7", 10", HDMI) > ###
The SPEAr1340 platform is supporting 3 configurations:
  1. 10" capacitive touchscreen
  1. 7" resistive touchscreen
  1. HDMI
<pre>
$ cd <SPEAR_ANDROID><br>
$ source ./build/envsetup.sh<br>
$ lunch (select number or name that refer to the below)<br>
* if you want 7inch ENG mode , select '14'<br>
(ex) $ lunch 14<br>
* if you want 10inch ENG mode , select '11'<br>
(ex) $ lunch 11<br>
<br>
</pre>
| Number | Specific Name |
|:-------|:--------------|
| 11. | full\_SPEAr1340\_10inch-eng |
| 12. | full\_SPEAr1340\_10inch-userdebug |
| 13. | full\_SPEAr1340\_10inch-user |
| 14. | full\_SPEAr1340\_7inch-eng |
| 15. | full\_SPEAr1340\_7inch-userdebug |
| 16. | full\_SPEAr1340\_7inch-user |

  * if you want HDMI mode , it'll be okay select one of "11,12,14,15"

### < 2. Compile kernel source (and modules) > ###

<pre>
$ cd <SPEAR_ANDROID><br>
$ cd kernel<br>
$ make clean<br>
$ make ARCH=arm spear13xx_android_defconfig<br>
$ export CROSS_COMPILE=../prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi-<br>
$ make ARCH=arm uImage<br>
$ cd ..<br>
</pre>


After this step you are ready to copy the kernel image to your USB key/SD card 'boot' partition, assuming it is mounted in /media/boot

<pre>
$ cp arch/arm/boot/uImage /media/boot/uImage_Android<br>
</pre>

You will also need to compile the **Mali modules** using these commands:

<pre>
$ cd <SPEAR_ANDROID>/device/stm/SPEAr1340/hardware/arm/driver/<br>
$ ./make_driver.sh<br>
</pre>

### < 3. Compile Android source code > ###

<pre>
$ cd <SPEAR_ANDROID><br>
$ make -j4    --> -j<CPU_COUNT><br>
</pre>
  * When build is finish , go to out folder.
  * If you select mode of 10", it'll make out folder as "SPEAr1340\_10inch"
  * If you select mode of 7", it'll make out folder as "SPEAr1340\_7inch"
  * If you select mode 7,10 of HDMI, it'll make out folder as "SPEAr1340\_7inch" or "SPEAr1340\_10inch"
<pre>
$ cd out/target/product/SPEAr1340_7inch  (according to selection 7" or 10")<br>
$ sudo ./build_root.sh<br>
$ cp ../../../../kernel/arch/arm/boot/uImage boot/uImage_Android<br>
</pre>

### < 4. Copy Build image into USB key/SD card. > ###

After this step you are ready to copy the Android file system to your USB key/SD card 'root' partition, assuming it is mounted in /media/root, and you are ready to copy Android data file in to /media/data/, copy boot parameter img files into  /media/boot/
  * **You must run build root script in out folder as like "sudo ./build\_root.sh" before copy build images into USB key / SD card.**
<pre>
$ cd out/target/product/SPEAr1340_7inch  (according to selection 7" or 10")<br>
$ sudo cp -arf root/* /media/root<br>
$ sudo cp -arf boot/* /media/boot<br>
$ sudo cp -arf data/* /media/data<br>
</pre>

#### Select/Update bootloader parameter img file (For HDMI) ####

In order to have a working build you will need to copy the kernel image and the correct "run.img" files to your boot partition. The "run.img" file contains the configuration for the bootloader.

  * **You must run as such like the below after copy all of the build image into USB key/SD card for HDMI**

| **According to Selection "7" mode , refer to the below**|
|:|
| $ cp (SPEAR\_ANDROID)/out/target/product/SPEAr1340\_7inch/boot/run\_HDMI.img /media/boot/run.img |

| **According to Selection "10" mode , refer to the below**|
|:|
| $ cp (SPEAR\_ANDROID)/out/target/product/SPEAr1340\_10inch/boot/run\_HDMI.img /media/boot/run.img |
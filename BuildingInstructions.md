### Building instructions for USB boot mode ###

There are 3 steps, the first one only if you need to use your own kernel instead of the pre-built one, and the third one if you need to change any configuration in bootloader.

  1. Compile kernel source code (and modules)
  1. Compile Android source code
  1. Select/Update bootloader parameter file

#### Compile kernel source (and modules) ####

<pre>
$ cd <SPEAR_ANDROID><br>
$ . build/envsetup.sh<br>
$ lunch full_SPEAr1340-eng<br>
$ cd kernel<br>
$ make ARCH=arm spear13xx_android_defconfig<br>
$ export CROSS_COMPILE=arm-eabi-<br>
$ make ARCH=arm uImage<br>
</pre>


After this step you are ready to copy the kernel image to your USB key/SD card 'boot' partition, assuming it is mounted in /media/boot

<pre>
$ cp arch/arm/boot/uImage /media/boot/uImage_Android<br>
</pre>

You will also need to compile the Mali and the Wifi kernel modules.

For the Mali modules please edit the `<SPEAR_ANDROID>`/device/stm/SPEAr1340/hardware/arm/mali/make\_driver.sh file and change the ANDROID\_D variable to suit your environment. Then you can compile it using these commands:

<pre>
$ cd <SPEAR_ANDROID>/device/stm/SPEAr1340/hardware/arm/mali/<br>
$ ./make_driver.sh<br>
</pre>

For the Wifi module please edit the `<SPEAR_ANDROID>`/device/stm/SPEAr1340/wifi/rtl8192cu/Makefile file and change the CROSS\_COMPILE and KSRC variables in the CONFIG\_PLATFORM\_ANDROID\_ARM section. Then you are able to build the module by executing:

<pre>
$ cd <SPEAR_ANDROID>/device/stm/SPEAr1340/wifi/rtl8192cu/<br>
$ make<br>
$ cp 8192cu.ko ../<br>
</pre>


#### Compile Android source code ####

<pre>
$ cd <SPEAR_ANDROID><br>
$ . build/envsetup.sh<br>
$ lunch full_SPEAr1340-eng<br>
$ make<br>
$ cd out/target/product/SPEAr1340<br>
$ sudo ./build_root.sh<br>
</pre>

After this step you are ready to copy the Android file system to your USB key/SD card 'root' partition, assuming it is mounted in /media/root

<pre>
$ sudo cp -arf root/* /media/root<br>
</pre>

#### Select/Update bootloader parameter file ####

The SPEAr1340 platform is supporting 3 configurations:
  1. 10" capacitive touchscreen
  1. 7" resistive touchscreen
  1. HDMI

In order to have a working build you will need to copy the kernel image and the correct "run.img" files to your boot partition. The "run.img" file contains the configuration for the bootloader.

<pre>
$ cp <SPEAR_ANDROID>/kernel/arch/arm/boot/uImage /media/boot/uImage_Android<br>
$ cp <SPEAR_ANDROID>/device/stm/SPEAr1340/boot/run.img /media/boot/<br>
</pre>

You will need to select one of:
  * run.img (or run\_10inch.img) for 10" configuration
  * run\_7inch.img for 7" configuration
  * run\_10inch\_HDMI.img for 10" HDMI configuration

You can create your own bootloader configuration files of needed. You will need to use run.txt (located in device/stm/SPEAr1340/boot/) as a starting point. After you made the changes you can create a run.img by executing the following command:

<pre>
$ mkimage -n "Demo Run Script" -A arm  -T script -C none  -d run.txt run.img<br>
</pre>
### You can select two Boot option for Spear1340 ###
#### #1 Boot option is USB key / SD card using. (Recommend) ####
#### #2 Boot option is NAND using. ####

---

#### This page is #2 Boot option for NAND Boot ####

---

### Downloading pre-built images ###

Pre-built images will be soon available from the [Downloads](http://code.google.com/p/floid/downloads/list) section. They are usually named to include the date when they were produced and in case you see more than one set of images the recommendation is to download and use the latest one.

### Using the pre-built images ###

The pre-built images are distributed as a tar.gz archive. You will need to unpack them and then copy the content to the new empty SD card for NAND installer
  * Nand boot is need USB memory stick or SD card

### (Option #2) Preparing the USB Memory / SD card with NAND installer ###
  * **Check the STM Device's NAND is working or not**

  1. Enter into **u-boot** by pressing **SPACE** on Console to stop autoboot.
  1. Result of below command should be **512 KiB**. If it’s **1024 KiB**, it’s not good to use NAND

| u-boot> nand info |
|:------------------|
| Device 0: MT29F16G08CBACAWP   ,, sector size 1024 KiB |


  * **Making USB memory stick with NAND installer**
  1. Format USB memory stick with FAT filesystem.
  1. Build STM Android (Refer to the [Building\_Instruction](http://code.google.com/p/floid/wiki/BuildingInstructions_ICS) section)
  1. Download NAND flasher from [Downloads](http://code.google.com/p/floid/downloads/list) section and extract its contents in the USB key. Create the following directory in the root of USB key (e.g.: /media/key),:
<pre>
$ tar -xvjf vInstaller.tar.bz2<br>
$ cp -r vInstaller /media/USB/<br>
$ cd /media/USB/vInstaller<br>
$ mkdir kernel<br>
$ mkdir rootfs<br>
</pre>
> > In the top directory of Android after finishing to build
<pre>
$ cp kernel/arch/arm/boot/uImage /media/USB/vInstaller/kernel/uImage_android<br>
$ cd out/target/product/stm/SPEAr1340_10inch<br>
$ sudo ./build_root.sh<br>
$ cp -arf data root/<br>
<br>
You shoud modify the init.rc file as refer to the below.<br>
$ vi root/init.rc<br>
...<br>
mount yaffs2 mtd@System /system inband-tags,tags-ecc-off<br>
mount yaffs2 mtd@Data /data nosuid nodev inband-tags,tags-ecc-off<br>
mount yaffs2 mtd@Cache /cache nosuid nodev inband-tags,tags-ecc-off<br>
#mount ext3 /dev/block/sda4 /data nosuid nodev<br>
...<br>
</pre>
<pre>
$ tar cvfps root.tar.gz root/*<br>
$ cp root.tar.gz /media/USB/vInstaller/rootfs<br>
</pre>

### <Flashing NAND (manually)> ###
  1. Connect USB memory stick with STM device and reboot the device.
  1. Enter into u-boot by pressing SPACE on Console to stop autoboot.
  1. Enter below commands to flash NAND:

|u-boot> mw 0x0 0x0|
|:-----------------|
|u-boot> usb start|
|u-boot> fatload usb 0:1 0x0 vInstaller/upgrade.img|
|u-boot> source 0x0|

<pre>
4. Device will install necessary files into NAND.<br>
5. Remove USB memory stick when it asks to remove.<br>
6. Device will be rebooted.<br>
</pre>

### <Boot with Android on NAND> ###
  1. Enter into u-boot by pressing SPACE on Console to stop autoboot.
  1. Enter below commands
| u-boot> setenv bootargs console=ttyAMA0,115200 androidboot.console=ttyAMA0 android.checkjni=0 mem=870M root=/dev/mtdblock8 rw rootfstype=yaffs2 rootflags=inband-tags,tags-ecc-off rootdelay=3 init=/init |
|:----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| u-boot> nand read.jffs2 0x800000 0x500000 0x800000 |
| u-boot> bootm 0x800000 |


### <Setup u-boot to make flashing NAND and booting with NAND automatically> ###
  1. Enter into u-boot by pressing SPACE on Console to stop autoboot.
  1. Input below commands
| u-boot> setenv bootupg mw 0x0 0x0\; fatload usb 0:1 0x0 vInstaller/upgrade.img\; source 0x0 |
|:--------------------------------------------------------------------------------------------|
| u-boot> setenv bootargs console=ttyAMA0,115200 androidboot.console=ttyAMA0 android.checkjni=0 mem=870M root=/dev/mtdblock8 rw rootfstype=yaffs2 rootflags=inband-tags,tags-ecc-off rootdelay=3 init=/init |
| u-boot> setenv bootcmd run bootusb\; run bootupg\; nand read.jffs2 0x800000 0x500000 0x800000\; bootm 0x800000 |
| u-boot> saveenv |

### <Reference #1, The status that is Setup (NAND Boot) > ###
  * **"printenv" result in u-boot env of the STM device that it's normal on NAND Setup.**

|ramboot=setenv bootargs root=/dev/ram rw console=ttyAMA0,115200 $(othbootargs);run bootusb; run bootupg; nand read.jffs2 0x800000 0x500000 0x800000; bootm 0x800000|
|:------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|nfsboot=bootp; setenv bootargs root=/dev/nfs rw nfsroot=$(serverip):$(rootpath) ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask):$(hostname):$(netdev):off console=ttyAMA0,115200 $(othbootargs);bootm;|
|bootdelay=1|
|baudrate=115200|
|usbtty=cdc\_acm|
|bootusb=mw 0x0 0x0; usb start; fatload usb 0:2 0x0 run.img; source 0x0|
|bootupg=mw 0x0 0x0; fatload usb 0:1 0x0 vInstaller/upgrade.img; source 0x0|
|bootargs=console=ttyAMA0,115200 androidboot.console=ttyAMA0 android.checkjni=0 mem=870M root=/dev/mtdblock8 rw rootfstype=yaffs2 rootflags=inband tags,tags-ecc-off rootdelay=3 init=/init|
|bootcmd=run bootusb; run bootupg; nand read.jffs2 0x800000 0x500000 0x800000; bootm 0x800000|
|filesize=3416EC|
|stdin=serial|
|stdout=serial|
|stderr=serial|
|verify=n|


### <Reference #2, The status that is Setup (Not NAND Boot) > ###
  * **"printenv" result in u-boot env of the STM device that it's normal on NAND Setup for Not NAND Boot.**

|ramboot=setenv bootargs root=/dev/ram rw console=ttyAMA0,115200 $(othbootargs);bootm 0xe6050000|
|:----------------------------------------------------------------------------------------------|
|nfsboot=bootp; setenv bootargs root=/dev/nfs rw nfsroot=$(serverip):$(rootpath) ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask):$(hostname):$(netdev):off console=ttyAMA0,115200 $(othbootargs);bootm;|
|bootdelay=1|
|baudrate=115200|
|usbtty=cdc\_acm|
|ethact=mii0|
|ethaddr=00:80:E1:12:61:FE|
|ipaddr=192.168.1.10|
|bootusb=usb start;fatload usb 0:2 0 run.img; source 0x0|
|nandboot=bootm 0xe6050000|
|bootcmd=run bootusb;run nandboot|
|bootargs=console=ttyAMA0,115200 root=/dev/mtdblock4 rootfstype=jffs2|
|linux=usb start; fatload usb 0:1 0x800000 uImage; bootm 0x800000|
|stdin=serial|
|stdout=serial|
|stderr=serial|
|verify=n|
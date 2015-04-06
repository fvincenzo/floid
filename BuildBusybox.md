## Configure and Build ##

Download busybox source from http://busybox.net/downloads/ .


#### Extract the busybox source: ####

$ tar jxf busybox-1.21.1.tar.bz2

#### Configure busybox ####

$ cd busybox-1.21.1/

$ make menuconfig


#### In menuconfig set the following options ####

$ Busybox Settings --> Build Options --> Build Busybox as a static binary (no shared libs) - Enable this option by pressing "Y"

$ Networking Utilities --> Support RPC services - Disable this option


#### Download and extract cross-compiler ####

$ cd ..

$ wget http://impactlinux.com/aboriginal/downloads/binaries/cross-compiler-armv5l.tar.bz2

$ tar xf cross-compiler-armv5l.tar.bz2

$ CROSS\_COMPILE="$PWD/cross-compiler-armv5l/bin/armv5l-"


#### Build busybox ####

$ cd busybox-1.21.1

$ make CROSS\_COMPILE="$CROSS\_COMPILE"


#### Installing Busybox on board ####

$ adb remount,rw

$ adb shell "chmod 0777 /system/xbin"

$ adb push busybox /system/xbin/

$ adb shell "chmod 755 /system/xbin/busybox"

$ adb shell "chmod 0755 /system/xbin"


#### Errors compiling busybox : ####

For make menuconfig, if you received error “ undefined reference to `pmap\_unset' ”, just disable RPC Service: Menuconfig>Networking Utilities>Support RPC services.

For make , if you received error " “checklist.c undefined reference to wmove" " and many such errors , just install libncurses5-dev :
$ sudo apt-get install libncurses5-dev
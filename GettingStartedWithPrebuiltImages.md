### You can select two Boot option for Spear1340 ###
#### #1 Boot option is USB key / SD card using. (Recommend) ####
#### #2 Boot option is NAND using. ####

---

#### This page is #1 Boot option for USB key / SD card Boot ####

---

### Downloading pre-built images ###

Pre-built images will be soon available from the [Downloads](http://code.google.com/p/floid/downloads/list) section. They are usually named to include the date when they were produced and in case you see more than one set of images the recommendation is to download and use the latest one.

### Using the pre-built images ###

The pre-built images are distributed as a tar.gz archive. You will need to unpack them and then copy the content to the USB key/SD card.

#### (Option #1) Preparing the USB key/SD card (Recommend) ####

To create a suitable USB key the first step is to create the appropriate partitions map consisting of:
| Partition Number | Partition Size | Partition Type | Bootable Flag | File System Type| Content |
|:-----------------|:---------------|:---------------|:--------------|:----------------|:--------|
| 1 | 3GB | FAT16 | N | vfat | Media (e.g.: Photos) |
| 2 | 32M | W95 FAT32 (LBA) | Y | vfat | Kernel image and boot script |
| 3 | 600MB | Linux | N | ext3 | Root file system |
| 4 | ~4GB | Linux | N | ext3 | Data file system |

#### Creating and formatting partitions ####

Insert the USB key and in a terminal do:
<pre>$ dmesg | grep "logical blocks"</pre>
that return the list of attached logical blocks. Suppose to consider an 8GB USB key,
the previous command print:
<pre>
sd 0:0:0:0: [sda] 488397168 512­byte logical blocks: (250 GB/232 GiB)<br>
sd 3:0:0:0: [sdb] 2930277168 512­byte logical blocks: (1.50 TB/1.36 TiB)<br>
sd 6:0:0:0: [sdc] 15646720 512­byte logical blocks: (8.01 GB/7.46 GiB)<br>
</pre>

The connected key is mapped as “sdc”. Now start to create partitions with fdisk tool:
<pre>sudo fdisk /dev/sdc</pre>

This command start an interactive shell. The used commands are:
  * p: print the partitions list
  * n: create a partitions
  * d: delete a partitions
  * t: define the partition type
  * l: show the available partitions type
  * m: show help
  * a: set boot flag
  * w: write to disk

Usually a new USB key is shipped with a partitions that covers the entire storage space. To view the partition on key use the 'p' command:
<pre> Device       Boot   Start    End      Blocks     Id     System<br>
/dev/sdc1           1        21270    7816688    b      W95 FAT32</pre>
Then delete this partition:
<pre>>d</pre>
To create a partition, the 'n' command must be used:
<pre>>n</pre>

The needed partition must be of primary type, then select 'primary partition' type when asked:
<pre>>p</pre>
Now fdisk tell you which is the partition number. Select 1 for first partition or press enter key to select the default value that is already 1. The successive step is to select the start and the end cylinder to define the partition geometry/size.

For start cylinder press the enter key to select the first free available cylinder. Follow this for all the three needed partitions.
The last cylinder should be selected defining the partition dimension (e.g.: +31M to create a 31M partition, +3G to create a 3GB partition).
Then for first partition:
  * partition number = 1
  * first cylinder = default (enter key)
  * last cylinder = +3G
For second partition:
  * partition number = 2
  * first cylinder = default (enter key)
  * last cylinder = +31M
For second partition:
  * partition number = 3
  * first cylinder = default (enter key)
  * last cylinder = +600M
For the last partition:
  * partition number = 4
  * first cylinder = default (enter key)
  * last cylinder = default (enter key), to select the entire residual space
Use the 't' command to define the partition type: this command take an
hexadecimal value to define the partition type. To show the list of all available
types, use the command 'l'.
| Partition number | Partition Type |
|:-----------------|:---------------|
| 1 | 6 |
| 2 | c |
| 3 | 83 |
| 4 | 83 |

To make a partition as bootable the boot flag must be selected with command **'a'**, then use this command and select the partition **2** as bootable.
Finally, to write all changes on the USB key use command **'w'**.

After partitions are made, then format it with proper file system:
<pre>
$ sudo mkfs.vfat /dev/sdc1 -­n media<br>
$ sudo mkfs.vfat /dev/sdc2 ­-n boot<br>
$ sudo mkfs.ext3 /dev/sdc3 ­-L root -m 1<br>
$ sudo mkfs.ext3 /dev/sdc4 ­-L data<br>
</pre>
<pre>
If there is some problem for the format , you should un-mount the partition as the below command list.<br>
and If there is no problem for the format , skip the below command list.<br>
</pre>
<pre>
$ sudo umount /dev/sdc1<br>
$ sudo umount /dev/sdc2<br>
$ sudo umount /dev/sdc3<br>
$ sudo umount /dev/sdc4<br>
</pre>
#### Unpacking the images and copy them to USB key/SD card ####

After you have downloaded the pre-built image archive you will need to unpack it:

<pre>$ tar zxf spear-1340-YYMMDD.tar.gz</pre>

If your system does not automatically mount the partitions on the USB key/SD card you will need to mount them manually:

<pre>
$ sudo mount /dev/sdc1 /media/media<br>
$ sudo mount /dev/sdc2 /media/boot<br>
$ sudo mount /dev/sdc3 /media/root<br>
$ sudo mount /dev/sdc4 /media/data<br>
</pre>
or you can re-insert the USB key/SD card.


The last step is to copy Android files inside the correct partitions:

<pre>
$ cd <unpacked images directory><br>
$ sudo cp -­arf ./root/* /media/root<br>
$ sudo cp -­arf ./boot/* /media/boot<br>
$ sudo cp -­arf ./data/* /media/data<br>
</pre>

As a last step you can copy media files on the 'media' partition if you want to use this for video playback testing. or You can external SD-card through Slot on the board. and then you can copy video image into the SD-card for the test video playback.
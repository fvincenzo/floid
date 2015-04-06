### Currently working features on Android 4.0.3 and hardware peripherals information ###

  * CPU
    * Maximum working core frequency (600MHz minimum)
    * SMP for Dual Cortex A9
    * L2 cache
  * 1GB DDR3 memory
  * 2GB FLASH
  * CLCD controller
    * 1024x768@60Hz       10”LCD kit, Capacitive Touch screen through USB (Atmel controller)
    * 800x480@60Hz          7”LCD , Resistive Touch screen through on board device.
    * HDMI, profiles:         Mouse connected to USB Host
      * 1080p
      * 720p
      * 480p
  * Stereo Audio Out
    * Through 3.5mm stereo jack connector
    * Through HDMI port
  * Mono audio In through a 3.5mm mono jack connector
  * Sensors  (accelerometer, magnetometer and gyroscope)
  * Camera
    * picture mode, video mode, panorama mode
  * SDIO
    * SDHC memory card
  * Basic Power Management framework
  * OTG USB
    * USB thumb drive
    * (It'll be available) Android ADB
    * USB Host
    * USB thumb drive
    * USB WiFi adapters. Supported models: Belkin SURF N150: Belkin Surf Micro WLAN USB-Adapter - Hi-Speed USB - 802.11b, 802.11g, 802.11n, Chipset: “RealTek RTL8192CU”.
    * USB Bluetooth adapters. Supported models: Digicom USB Micro Bluetooth Adapter. The chipset is a Cambridge Silicon Radio
  * Hardware Video/jpeg Decoder
    * H.263
    * H.264 AVC, Baseline profile
    * MPEG-4 SP
    * VP8
    * Jpeg images
  * Hardware Video Encoder
    * H.263
    * H.264 AVC, Baseline profile (Higher quality)
  * Video Playback
    * Test case of the Support Video
| Type |  Format | Type | Format |
|:-----|:--------|:-----|:-------|
| Video Playback | Video – 1080p – H264 | Video Streaming | Video – 1080p – H264 |
| Video Playback | Video – 720p – H264 | Video Streaming | Video – 720p – H264 |
| Video Playback | Video – wvga – H264 | Video Streaming | Video – 480p – H264 |
| Video Playback | Video –vga – H264 | Video Streaming | Video – 1080p – MPEG4 |
| Video Playback | Video – 720p – H263 | Video Streaming | Video – 720p – MPEG4 |
| Video Playback | Video – wvga – H263 | Video Streaming | Video – 480p – MPEG4 |
| Video Playback | Video –vga – H263 | Video Streaming | Video –1080p – VP8 |
| Video Playback | Video – 1080p – MPEG4 | Video Streaming | Video –720p – VP8 |
| Video Playback | Video – 720p – MPEG4 | Video Streaming | Video –480p – VP8 |
| Video Playback | Video – wvga – MPEG4 | Video Playback | Video –vga – MPEG4 |
| Video Playback | Video – 1080p – VP8 | Video Playback | Video – 720p – VP8 |
| Video Playback | Video – wvga – VP8 | Video Playback | Video –vga – VP8 |
| Video Playback | Video –Suspend&Resume |

  * GPU integration in Android Surfaceflinger
  * FLASH 10 HW accelerated plug-in
  * USB boot. Suggested USB key is the 8GB SanDisk Cruzer Blade
  * NAND boot: vInstaller procedure, for embedded Flash, could be obtained “on demand”.
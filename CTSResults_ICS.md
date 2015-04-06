## CTS Results ##

Used CTS version: 4.0.3r2

**Failures:**
```
android.graphics
-- testMeasureTextWithLongText

android.mediastress
android.mediastress.cts.MediaRecorderStressTest
-- testStressRecordVideoAndPlayback

android.security
android.security.cts.PackageSignatureTest
-- testPackageSignatures
```


## How to run the tests ##

Follow the official instructions from Google on
  * http://source.android.com/compatibility/cts-intro.html
  * http://source.android.com/compatibility/android-cts-manual-r6.pdf

Used CTS version can be downloaded from https://dl.google.com/dl/android/cts/android-cts-4.0.3_r2-linux_x86-arm.zip

**How to connect to SPEAr1340 board:**
  1. To enable the ADB connection through IP, uncomment the following line "setprop service.adb.tcp.port 5555" from init.rc, which are available in root path.
  1. DTS settings:
    * A jumper on the board (JP27 - RTC battery enable) should be closed to provide power to RTC
    * You should set time at least once and then reboot.
  1. Use WIFI IP for adb connection, `adb connect <ip>`.
  1. For best results CTS must be done in 2 sessions:
    * First session:
      * start cts using: `./android-cts/tools/cts-tradefed`
      * start a full plan of tests using : `run cts –plan CTS`
    * Second session:
      * collect all the failures in one new plan:
        * add derivedplan --plan plane\_name --session/-s session\_id  -r 					            [pass/fail/notExecuted/timeout] (E.g: `add derivedplan  –plan CTS_rerun -s 1 -r fail`)
      * OR run individually the failures:  run cts --class/-c [--method/-m] (E.g: `run cts -c android.os.cts.AsyncTaskTest -m testAsyncTask`)
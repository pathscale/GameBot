Installation
============

Required: OpenCV v3.x (master)
Device-side recorder binary (`arm.out`)
Nvidia Shield Tablet connected over USB
ADB binary in PATH

```
$ cd levelme_coc/CookieRun
$ mkdir build
$ cmake ../
```

Running:
========

In one terminal:

```
$ adb forward tcp:9999 tcp:5645
$ adb push /path/to/arm.out /data/local/tmp/
$ adb shell /data/local/tmp/arm.out
```

Device:

0. USB port and cable must be on the left of the screen
1. Open Cookie Run
2. Navigate to pirate level (first bonus)
3. Stop at the screen with green PLAY button

In another terminal:

```
$ cd levelme_coc/CookieRun
$ ./build/cookie_player /dev/input/event0
```

Adjustments:
============

File `play_level.log` contains actual gameplay input. Replace it and/or use `delay.py` on it.

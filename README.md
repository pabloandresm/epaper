# epaper
WaveShare E-Ink 4.3" driver for Arduino, Linux and Cygwin(Windows).

This device is an excellent piece of hardware!

The official driver sends commands to the display via UART but does not read the answer from it. This is a great disadvantage because it obliges the programmer to add delays to wait for the actions to be performed, crossing fingers the delay was enough.

This driver uses the full capacity of the E-Ink UART protocol, reading the error codes of every command issued. Once the command returns then for sure it is finished, so you don't need to add random delays.

The driver was tested on Arduino, ESP32, ESP8266, STM32, Cygwin, and Linux platforms, but for sure will work on many others, as the code is pretty straightforward.


Differences with the official Waveshare driver:
----------------------------------------------
1 - This driver reads error codes from display after each command is finished, so not only you have the error but also you don't need to add delay()s to guess when it might have finished.

2 - Reduced RAM footprint and FLASH footprint, and speed optimizations.

3 - Added buffer overflow checks for string and bitmap function.

4 - Parameters in separate file epaperconf.h

5 - Multiplatform driver (Arduino, ESP, STM32, Linux, Cygwin(Windows), etc.

6 - Added epaper_set_drawmode() function for transparent fonts, and epaper_draw_arc() function. (UPDATED FIRMWARE NEEDED!)

7 - Added epaper_disp_string_penwidth() function to increase the pen width of the string displayed. (UPDATED FIRMWARE NEEDED!)


Firmware:
--------
This repository contains the latest firmware from WaveShare with 5 additions:

1 - Fixed bug when bitmap to be displayed has a width not divisible by 8. The original firmware truncates the output to the nearest smallest width divisible by 8

2 - Added function epaper_draw_arc() to draw incomplete circles.

3 - Added function epaper_set_drawmode() that allows transparent mode on text output. Transparent text only draws the non-white pixels, so it allows text over other pixels and figures.

4 - This firmware contains the full ASCII-Extended characters. ASCII-Extended includes characters after 128, which includes symbols, letters with accents, etc. etc.

5 - This firmware was compiled with -O2 optimization parameter, so it works faster, it refreshes the screen 5 times faster, which leads to lower overall power consumption.


I will improve it as I test more platforms.


Thank you.

Pablo Martikian

19 APRIL 2021

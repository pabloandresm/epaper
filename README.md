# epaper
WaveShare E-Ink 4.3" driver for Arduino, Linux and cygwin(Windows).

This device is an excellent piece of hardware!

The official driver sends commands to the display via UART but does not read the answer of them. This is a great disadvantage because it obliges the programmer to add delays to wait for the actions to be performed, and cross fingers the delay was enough. This delays can be too long (making the code wait for nothing), or too short (issuing commands without waiting for the to completely finish).

This driver uses the full capacity of the E-Ink UART protocol, reading the error codes of every command. Once the command returns then for sure it is finished, so you don't need to add random delays.

Also you can use it for Arduino, ESP32, or any microcontroller, any version and hardware platform of Linux, and also Cygwin for Windows.

Differences with the official Waveshare driver:
----------------------------------------------
1 - This driver reads error codes from display after each command is finished, so not only you have the error but also you don't need to add delay()s after each command to guess when it might have finished.

2 - Reduced RAM footprint and FLASH footprint, and some speed optimizations.

3 - Added buffer overflow checks for string and bitmap function.

4 - Parameters in separate file epaperconf.h

5 - This driver works on Arduino, and also Linux and Cygwin (Windows)


I will improve it as I test the devices.


Thank you.

Pablo Martikian

5 Nov 2018

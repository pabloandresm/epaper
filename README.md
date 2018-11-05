# epaper
WaveShare E-Ink 4.3" driver for Arduino, Linux and cygwin(Windows).

The official driver sends commands to the display via UART but does not read the answer of them. This is a great disadvantage because it obliges the programmer to add delays to wait for the actions to be performed, and cross fingers the delay was enough. This delays can be too long (making the code wait for nothing), or too short (issuing commands without waiting for the to completely finish).

This driver uses the full capacity of the E-Ink UART protocol, reading the error codes of every command. Once the command returns then for sure it is finished, so you don't need to add random delays.

Also you can use it for Arduino, ESP32, or any microcontroller, any version and hardware platform of Linux, and also Cygwin for Windows.

I will improve it as I test the devices.

Thank you
Pablo Martikian

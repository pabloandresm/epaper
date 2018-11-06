/***************************************************************************
  This is a library for the Waveshare E-Ink UART display.

  The display uses UART serial communication to interface with.

  Written by Pablo Martikian (pablomartikian@hotmail.com)

  Please include this text in any redistribution.
 ***************************************************************************/
#ifndef __CONF_H
#define __CONF_H

#define EPAPER_REPONSE_TIMEOUT		5000

#ifndef __AVR__
// for linux or cygwin

#define EINK_SERIAL	"/dev/ttyUSB0"
// or you could try "/dev/ttyS3" for cygwin

#else
// for Arduino

#define EINK_SERIAL			Serial2
// or you could try SoftwareSerial.h

#define PIN_EINK_WAKEUP		12
#define PIN_EINK_RST		13

#endif

#endif /* __CONF_H */

/***************************************************************************
  This is a library wrapper to use Arduino code in linux/cygwin.

  Written by Pablo Martikian (pablomartikian@hotmail.com)

  Please include this text in any redistribution.
 ***************************************************************************/
#ifndef __AVR__

#ifndef __LINUX_ARDUINO_WRAPPER_H
#define __LINUX_ARDUINO_WRAPPER_H

#include <stdio.h>
#include <unistd.h>

extern unsigned long millis(void);
extern void delay(unsigned long milli);

class SerialC {
public:
void begin(unsigned long baud);
void end();
size_t print(const char s[]);
size_t print(char c);
size_t print(unsigned long  i);
size_t println(unsigned long  i);
size_t println(const char s[]);
size_t println(char c);
size_t println();
};

extern SerialC Serial;

#endif

#endif

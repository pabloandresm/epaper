/***************************************************************************
  This is a library wrapper to use Arduino code in linux/cygwin.

  Written by Pablo Martikian (pablomartikian@hotmail.com)

  Please include this text in any redistribution.
 ***************************************************************************/
#ifndef __AVR__

#include "linux_arduino_wrapper.h"

#include <time.h>

#include <string.h>

#ifdef __linux__
static struct timespec program_start_time;
void __attribute__ ((constructor)) linux_arduino_wrapper_constructor()
{
	clock_gettime(CLOCK_MONOTONIC_RAW,&program_start_time);
}

unsigned long
millis(void)
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
	tp.tv_sec -= program_start_time.tv_sec;
	tp.tv_nsec -= program_start_time.tv_nsec;
	while (tp.tv_nsec < 0) {
		tp.tv_nsec += (long)1000000000;
		tp.tv_sec --;
		}
	return tp.tv_sec*1000 + tp.tv_nsec/1000000.0;
}
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
unsigned long
millis(void)
{
	return GetTickCount();
}
#endif

void
delay(unsigned long milli)
{
	usleep(milli*1000);
}

void SerialC::begin(unsigned long baud)
{
}

void SerialC::end()
{
}

size_t SerialC::print(const char s[])
{
	return printf("%s",s);
}

size_t SerialC::print(char c)
{
	return printf("%c",c);
}

size_t SerialC::print(unsigned long i)
{
	return printf("%lu",i);
}

size_t SerialC::println(unsigned long l)
{
	return printf("%lu\n",l);
}

size_t SerialC::println(const char s[])
{
	return printf("%s\n",s);
}

size_t SerialC::println(char c)
{
	return printf("%c\n",c);
}

size_t SerialC::println()
{
	return printf("\n");
}

SerialC Serial;

#endif


/***************************************************************************
  This is a library for the Waveshare E-Ink UART display.

  The display uses UART serial communication to interface with.

  Written by Pablo Martikian (pablomartikian@hotmail.com)

  Please include this text in any redistribution.
 ***************************************************************************/
#ifndef __EPAPER_H
#define __EPAPER_H

/*
FONT SIZES
*/
#define FONT32						0x01
#define FONT48						0x02
#define FONT64						0x03

/*
MEMORY MODE
*/
#define MEM_NAND					0
#define MEM_TF						1

/*
SCREEN ROTATION
*/
#define EPD_NORMAL					0
#define EPD_90						1
#define EPD_180						2
#define EPD_270						3

/*
COLOR
*/
#define EPAPER_WHITE				0x03
#define EPAPER_GRAY					0x02
#define EPAPER_DARK_GRAY			0x01
#define EPAPER_BLACK				0x00

/*
ERROR CODES
*/
#define EPAPER_OK									0
#define EPAPER_FAILED_TFCARD_INIT					1
#define EPAPER_INVALID_PARAMETERS					2
#define EPAPER_TFCARD_NOT_FOUND						3
#define EPAPER_FILE_NOT_FOUND						4
#define EPAPER_CHECKSUM_FAILED						20
#define EPAPER_INVALID_FRAME_FORMAT					21
#define EPAPER_UNDEFINED_ERROR						250
// display error code 0 redirected to 88
#define EPAPER_INVALID_COMMAND						88

#ifdef __AVR__
extern void epaper_reset(void);					// linux or cygwin don't connect this pin, so it will not be possible to reset
extern int epaper_wakeup(void);					// linux or cygwin don't connect this pin, so it will not be possible to wakeup
#else			// for linux or cygwin
extern int epaper_fini(void);		// in Arduino you don't need epaper_fini(), can do SerialX.end()
#endif

extern int epaper_init(void);		// for all (Arduino, cygwin, linux, etc.)

extern int epaper_handshake(void);
extern int epaper_update(void);
extern void epaper_sleep(void);		// be careful with this command! linux or cygwin will not be able to wakeup as the wakeup pin is not connected

extern int epaper_set_baud(long baud);
extern int epaper_read_baud(void);

extern int epaper_set_memory(unsigned char mode);
extern int epaper_get_memory(void);
extern int epaper_set_screenrotation(unsigned char mode);
extern int epaper_get_screenrotation(void);
extern int epaper_set_color(unsigned char color, unsigned char bkcolor);
extern int epaper_get_color(void);
extern int epaper_set_en_font(unsigned char font);
extern int epaper_set_ch_font(unsigned char font);
extern int epaper_draw_pixel(int x0, int y0);
extern int epaper_draw_line(int x0, int y0, int x1, int y1);
extern int epaper_rect(int fill, int x0, int y0, int x1, int y1);
extern int epaper_circle(int fill, int x0, int y0, int r);
extern int epaper_triangle(int fill, int x0, int y0, int x1, int y1, int x2, int y2);
extern int epaper_clear(void);
extern int epaper_disp_string(const void * p, int x0, int y0);
#ifdef __AVR__
#include <WString.h>
#include <avr/pgmspace.h>
extern int epaper_disp_string(const __FlashStringHelper * p, int x0, int y0);
#endif
extern int epaper_disp_char(const char c, int x0, int y0);
extern int epaper_disp_bitmap(const void * p, int x0, int y0);

#if 0
// for debugging purposes
extern void epaper_dump(unsigned long long timeout);
#endif

#endif /* __EPAPER_H */

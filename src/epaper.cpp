/***************************************************************************
  This is a library for the Waveshare E-Ink UART display.

  The display uses UART serial communication to interface with.

  Written by Pablo Martikian (pablomartikian@hotmail.com)

  Please include this text in any redistribution.
 ***************************************************************************/
#include "epaperconf.h"
#include "epaper.h"

#include <string.h>
#include <stdlib.h>

#define FRAME_B						0xA5
#define FRAME_E0					0xCC
#define FRAME_E1					0x33
#define FRAME_E2					0xC3
#define FRAME_E3					0x3C

#define CMD_HANDSHAKE				0x00
#define CMD_SET_BAUD				0x01
#define CMD_READ_BAUD				0x02
#define CMD_GET_MEMORYMODE			0x06
#define CMD_SET_MEMORYMODE			0x07
#define CMD_STOPMODE				0x08
#define CMD_UPDATE					0x0A
#define CMD_GET_SCREENROTATION		0x0C
#define CMD_SET_SCREENROTATION		0x0D
#define CMD_LOAD_FONT				0x0E
#define CMD_LOAD_PIC				0x0F

#define CMD_SET_COLOR				0x10
#define CMD_GET_COLOR				0x11
#define CMD_SET_EN_FONT				0x1E
#define CMD_SET_CH_FONT				0x1F

#define CMD_DRAW_PIXEL				0x20
#define CMD_DRAW_LINE				0x22
#define CMD_FILL_RECT				0x24
#define CMD_DRAW_RECT				0x25
#define CMD_DRAW_CIRCLE				0x26
#define CMD_FILL_CIRCLE				0x27
#define CMD_DRAW_TRIANGLE			0x28
#define CMD_FILL_TRIANGLE			0x29
#define CMD_DRAW_ARC				0x2D
#define CMD_CLEAR					0x2E
#define CMD_CLEAR_COLOR				0x2F

#define CMD_DRAW_STRING				0x30

#define CMD_DRAW_BITMAP				0x70

#define CMD_SET_DRAWMODE      0x32


#ifndef __AVR__							// linux or cygwin
#include "linux_arduino_wrapper.h"		// wrapper to emulate millis() / delay() / and Serial.
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>

static int epaper_fd = -1;

static int
serial_open(const char *serial_name, speed_t baud)
{
	struct termios newtermios;
	int fd;
	fd = open(serial_name,O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
	if (fd<0) return -1;

	memset(&newtermios,0,sizeof(newtermios));
	tcgetattr(fd,&newtermios);

	cfsetospeed(&newtermios,baud);
	cfsetispeed(&newtermios,baud);

	newtermios.c_cc[VMIN] = 0;			// read doesn't block

	newtermios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	newtermios.c_oflag &= ~OPOST;
	newtermios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	newtermios.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
	newtermios.c_cflag |= CS8;

	tcflush(fd,TCIOFLUSH);

	if (tcsetattr(fd,TCSANOW,&newtermios)==-1) { close(fd); return -1; }

	return fd;
}

int
epaper_init(void)
{
	if (epaper_fd>-1) return epaper_fd;
	epaper_fd = serial_open(EINK_SERIAL,B115200);
	return epaper_fd;
}

int
epaper_fini(void)
{
	if (epaper_fd==-1) return 0;
	fsync(epaper_fd);
	close(epaper_fd);
	epaper_fd = -1;
	return 0;
}

int
epaper_write(unsigned char *v, int s)
{
//	int i;
	if (epaper_fd<0) return -1;
//	for (i=0;i<s;i++) printf("[%02X] ",(int)v[i]);
	return write(epaper_fd,v,s);
}

int
epaper_read(char *c)
{
	if (epaper_fd<0) return -1;
	return read(epaper_fd,c,1);
}
#endif

//--------------------------------------------------------------------

#ifdef __AVR__

#include <Arduino.h>
#include <WString.h>

#define epaper_write(v,s)	EINK_SERIAL.write(v,s)

int
epaper_read(char *c)
{
	char cc;
	if (EINK_SERIAL.available()<=0) return -1;
	cc = EINK_SERIAL.read();
	if (cc==-1) return -1;
	*c = (char)cc;
   	//Serial.print((char)cc);
	return 1;
}

int
epaper_init(void)
{
	EINK_SERIAL.begin(115200);
	pinMode(PIN_EINK_RST,OUTPUT);
	digitalWrite(PIN_EINK_RST, LOW);
	pinMode(PIN_EINK_WAKEUP,OUTPUT);
	digitalWrite(PIN_EINK_WAKEUP, LOW);
	return 1;
}

#endif

#if 0
void
epaper_dump(unsigned long long timeout)
{
	char c;
	unsigned long long t = millis() + timeout;
	while (millis()<t) {
		if (1==epaper_read(&c)) printf("%c",c);
			else delay(1);
		}
}
#endif

//--------------------------------------------------------------------

//static const unsigned char _cmd_load_font[8] = {0xA5, 0x00, 0x09, CMD_LOAD_FONT, 0xCC, 0x33, 0xC3, 0x3C};
//static const unsigned char _cmd_load_pic[8] = {0xA5, 0x00, 0x09, CMD_LOAD_PIC, 0xCC, 0x33, 0xC3, 0x3C};


static unsigned char global_bgcolor = -1;
static unsigned char global_fgcolor = -1;
static unsigned char global_en_font = -1;
static unsigned char global_ch_font = -1;

static unsigned char _cmd_buff[CMD_SIZE];

static unsigned char _verify(const void * ptr, int n)
{
	int i;
	unsigned char * p = (unsigned char *)ptr;
	unsigned char result = 0;
	for(i = 0; i < n; i++) result ^= p[i];
	return result;
}

static int _sendcmd(unsigned char *c, int s)
{
	int r = 0;
	unsigned char cc = _verify(c,s);
	r = epaper_write(c,s);				// send cmd
	return r+epaper_write(&cc,1);		// send checksum
}

static int
epaper_wait_error_getnumber(void)
{
	int n = 0;
	char c;
	do {
		if (epaper_read(&c)!=1) break;
		n = n * 10 + (c-'0');
	} while (1);
	if (n == 0) return 88;	// redirect display error 0 to 88
	return n;
}

static int
epaper_wait_ok_error(int with_enter)
{
    char c;
    int x;
    char buf[8] = {0,0,0,0,0,0,0,0};    // \n E r r o r :
    unsigned long long timeout = millis() + EPAPER_REPONSE_TIMEOUT;
    while (millis() < timeout) {
        if (1==epaper_read(&c)) {
            for (x=0;x<7;x++) buf[x] = buf[x+1];
            buf[x]=c;
			if (with_enter) {
				// for bitmap display there is a \n before the message
#ifdef __AVR__
   				if (memcmp_P(buf+1,F("\nError:"),7)==0) return epaper_wait_error_getnumber();
#else
   				if (memcmp(buf+1,"\nError:",7)==0) return epaper_wait_error_getnumber();
#endif
   	    		if ((buf[5]=='\n') && (buf[6]=='O') && (buf[7]=='K')) return 0;
#ifdef __AVR__
				if (memcmp_P(buf,F(" allowed"),8)==0) return 4;	// "Only .BMP .JPG .JPEG are allowed"
#else
				if (memcmp(buf," allowed",8)==0) return 4;	// "Only .BMP .JPG .JPEG are allowed"
#endif
				} else {
#ifdef __AVR__
					if (memcmp_P(buf+2,F("Error:"),6)==0) return epaper_wait_error_getnumber();
#else
					if (memcmp(buf+2,"Error:",6)==0) return epaper_wait_error_getnumber();
#endif
            		if ((buf[6]=='O') && (buf[7]=='K')) return 0;
					}
            } else delay(1);    // 1 millisecond
        }
    return -1;
}

#ifdef __AVR__

void
epaper_reset(void)
{
	pinMode(PIN_EINK_RST,OUTPUT);
	digitalWrite(PIN_EINK_RST, LOW);
	delayMicroseconds(10);
	digitalWrite(PIN_EINK_RST, HIGH);
	delayMicroseconds(500);
	digitalWrite(PIN_EINK_RST, LOW);
	delay(3000);
}

static void
epaper_wakeup_internal(void)
{
	pinMode(PIN_EINK_WAKEUP,OUTPUT);
	digitalWrite(PIN_EINK_WAKEUP, LOW);
	delayMicroseconds(10);
	digitalWrite(PIN_EINK_WAKEUP, HIGH);
	delayMicroseconds(500);
	digitalWrite(PIN_EINK_WAKEUP, LOW);
	delay(10);
}

int
epaper_wakeup(void)
{
	int retries = EPAPER_REPONSE_TIMEOUT / 1000;
	do {
		epaper_wakeup_internal();
		delay(500);
		if (epaper_handshake()==0) return 0;
		delay(500);
		retries--;
		} while (retries>0);
	return -1;
}

#endif

void
epaper_sleep(void)
{
	const unsigned char _cmd_stopmode[8] = {FRAME_B, 0x00, 0x09, CMD_STOPMODE, FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_sendcmd((unsigned char *)_cmd_stopmode,8);
}

int
epaper_update(void)
{
	const unsigned char _cmd_update[8] = {FRAME_B, 0x00, 0x09, CMD_UPDATE, FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_sendcmd((unsigned char *)_cmd_update, 8);
	return epaper_wait_ok_error(0);
}

int
epaper_handshake(void)
{
	const unsigned char _cmd_handshake[8] = {FRAME_B, 0x00, 0x09, CMD_HANDSHAKE, FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_sendcmd((unsigned char *)_cmd_handshake, 8);
	return epaper_wait_ok_error(0);
}

int
epaper_set_baud(long baud)
{
	unsigned char _cmd_set_baud[12] = { FRAME_B, 0x00, 0x0D, CMD_SET_BAUD, 0,0,0,0,FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_cmd_set_baud[4] = (baud >> 24) & 0xFF;
	_cmd_set_baud[5] = (baud >> 16) & 0xFF;
	_cmd_set_baud[6] = (baud >> 8) & 0xFF;
	_cmd_set_baud[7] = baud & 0xFF;
	_sendcmd(_cmd_set_baud,12);
	delay(10);
	return epaper_wait_ok_error(0);
}

int
epaper_read_baud(void)
{
	int x;
	char buf[8] = {0,0,0,0,0,0,0,0};
	const unsigned char _cmd_read_baud[8] = {FRAME_B, 0x00, 0x09, CMD_READ_BAUD, FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_sendcmd((unsigned char *)_cmd_read_baud, 8);
	delay(500);	// half a second
	for (x=0;x<7;x++)
		if (1==epaper_read(buf+x)) continue;
			else break;
	return atoi(buf);
}

int
epaper_get_memory(void)
{
	char c;
	const unsigned char _cmd_get_memorymode[8] = {FRAME_B, 0x00, 0x09, CMD_GET_MEMORYMODE, FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_sendcmd((unsigned char *)_cmd_get_memorymode, 8);
	delay(500);	// half a second
	if (1==epaper_read(&c)) return c-'0';
	return -1;
}

int
epaper_set_memory(unsigned char mode)
{
	unsigned char _cmd_set_memory[9] = { FRAME_B, 0x00, 0x0A, CMD_SET_MEMORYMODE, 0, FRAME_E0, FRAME_E1, FRAME_E2, FRAME_E3};
	_cmd_set_memory[4] = mode;
	_sendcmd(_cmd_set_memory, 9);
	return epaper_wait_ok_error(0);
}

int
epaper_set_screenrotation(unsigned char mode)
{
	unsigned char _cmd_set_screenrotation[9] = {FRAME_B, 0x00, 0x0A, CMD_SET_SCREENROTATION, 0, FRAME_E0, FRAME_E1, FRAME_E2, FRAME_E3};
	_cmd_set_screenrotation[4] = mode;
	_sendcmd(_cmd_set_screenrotation, 9);
	return epaper_wait_ok_error(0);
}

int
epaper_set_drawmode(unsigned char mode)
{
	unsigned char _cmd_set_drawmode[9] = {FRAME_B, 0x00, 0x0A, CMD_SET_DRAWMODE, 0, FRAME_E0, FRAME_E1, FRAME_E2, FRAME_E3};
	_cmd_set_drawmode[4] = mode;
	_sendcmd(_cmd_set_drawmode, 9);
	return epaper_wait_ok_error(0);
}

int
epaper_get_screenrotation(void)
{
	char c;
	const unsigned char _cmd_get_screenrotation[8] = {FRAME_B, 0x00, 0x09, CMD_GET_SCREENROTATION, FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_sendcmd((unsigned char *)_cmd_get_screenrotation, 8);
	delay(500);	// half a second
	if (1==epaper_read(&c)) return c-'0';
	return -1;
}

int
epaper_set_color(unsigned char colour, unsigned char bkcolour)
{
	unsigned char _cmd_set_color[10] = { FRAME_B, 0x00, 0x0B, CMD_SET_COLOR, 0,0,FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	if ((colour==global_fgcolor) && (bkcolour==global_bgcolor)) return 0;
	global_fgcolor = colour;
	global_bgcolor = bkcolour;
	_cmd_set_color[4]= colour;
	_cmd_set_color[5]= bkcolour;
	_sendcmd(_cmd_set_color,10);
	return epaper_wait_ok_error(0);
}

int
epaper_get_color(void)
{
	char c;
	const unsigned char _cmd_get_color[8] = {FRAME_B, 0x00, 0x09, CMD_GET_COLOR, FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_sendcmd((unsigned char *)_cmd_get_color,8);
	delay(500);	// half a second
	if (1==epaper_read(&c)) return c-'0';
	return -1;
}

int
epaper_set_en_font(unsigned char font)
{
	int res;
	unsigned char _cmd_set_en_font[9] = { FRAME_B, 0x00, 0x0A, CMD_SET_EN_FONT, 0, FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	if (global_en_font==font) return 0;
	global_en_font = font;
	_cmd_set_en_font[4] = font;
	_sendcmd(_cmd_set_en_font, 9);
	res = epaper_wait_ok_error(0);
//	if (!res) CurrentFont = font;
	return res;
}

int
epaper_set_ch_font(unsigned char font)
{
	int res;
	unsigned char _cmd_set_ch_font[9] = { FRAME_B, 0x00, 0x0A, CMD_SET_CH_FONT, 0, FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	if (global_ch_font==font) return 0;
	global_ch_font = font;
	_cmd_set_ch_font[4] = font;
	_sendcmd(_cmd_set_ch_font, 9);
	res = epaper_wait_ok_error(0);
//	if (!res) CurrentFont = font;
	return res;
}

int
epaper_draw_pixel(int x0, int y0)
{
	unsigned char _cmd_draw_pixel[12] = {FRAME_B,0x00,0x0D,CMD_DRAW_PIXEL,0,0,0,0,FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_cmd_draw_pixel[4] = (x0 >> 8) & 0xFF;
	_cmd_draw_pixel[5] = x0 & 0xFF;
	_cmd_draw_pixel[6] = (y0 >> 8) & 0xFF;
	_cmd_draw_pixel[7] = y0 & 0xFF;
	_sendcmd(_cmd_draw_pixel, 12);
    return epaper_wait_ok_error(0);
}

int
epaper_draw_line(int x0, int y0, int x1, int y1)
{
	unsigned char _cmd_draw_line[16]={FRAME_B,0x00,0x11,CMD_DRAW_LINE,0,0,0,0,0,0,0,0,FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_cmd_draw_line[4] = (x0 >> 8) & 0xFF;
	_cmd_draw_line[5] = x0 & 0xFF;
	_cmd_draw_line[6] = (y0 >> 8) & 0xFF;
	_cmd_draw_line[7] = y0 & 0xFF;
	_cmd_draw_line[8] = (x1 >> 8) & 0xFF;
	_cmd_draw_line[9] = x1 & 0xFF;
	_cmd_draw_line[10] = (y1 >> 8) & 0xFF;
	_cmd_draw_line[11] = y1 & 0xFF;
	_sendcmd(_cmd_draw_line, 16);
    return epaper_wait_ok_error(0);
}

int
epaper_rect(int fill, int x0, int y0, int x1, int y1)
{
	unsigned char _cmd_rect[16] ={FRAME_B,0x00,0x11,CMD_FILL_RECT,0,0,0,0,0,0,0,0,FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	if (!fill) _cmd_rect[3] = CMD_DRAW_RECT;
	_cmd_rect[4] = (x0 >> 8) & 0xFF;
	_cmd_rect[5] = x0 & 0xFF;
	_cmd_rect[6] = (y0 >> 8) & 0xFF;
	_cmd_rect[7] = y0 & 0xFF;
	_cmd_rect[8] = (x1 >> 8) & 0xFF;
	_cmd_rect[9] = x1 & 0xFF;
	_cmd_rect[10] = (y1 >> 8) & 0xFF;
	_cmd_rect[11] = y1 & 0xFF;
	_sendcmd(_cmd_rect,16);
	return epaper_wait_ok_error(0);
}

int
epaper_circle(int fill, int x0, int y0, int r)
{
	unsigned char _cmd_circle[14]={FRAME_B,0x00,0x0F,CMD_FILL_CIRCLE,0,0,0,0,0,0,FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	if (!fill) _cmd_circle[3] = CMD_DRAW_CIRCLE;
	_cmd_circle[4] = (x0 >> 8) & 0xFF;
	_cmd_circle[5] = x0 & 0xFF;
	_cmd_circle[6] = (y0 >> 8) & 0xFF;
	_cmd_circle[7] = y0 & 0xFF;
	_cmd_circle[8] = (r >> 8) & 0xFF;
	_cmd_circle[9] = r & 0xFF;
	_sendcmd(_cmd_circle,14);
	return epaper_wait_ok_error(0);
}

int
epaper_circle_width_inner(int x0, int y0, int r, int width_inner)
{
	int x;
	int res;
	for (x=width_inner;x>0;x--) {
		res = epaper_circle(0,x0,y0,r-x+1);			// +1 because width_inner=0 means 1 pixel width
		if (res) return res;
		}
	return EPAPER_OK;
}

int
epaper_triangle(int fill, int x0, int y0, int x1, int y1, int x2, int y2)
{
	unsigned char _cmd_triangle[20]={FRAME_B,0x00,0x15,CMD_FILL_TRIANGLE,0,0,0,0,0,0,0,0,0,0,0,0,FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	if (!fill) _cmd_triangle[3] = CMD_DRAW_TRIANGLE;
	_cmd_triangle[4] = (x0 >> 8) & 0xFF;
	_cmd_triangle[5] = x0 & 0xFF;
	_cmd_triangle[6] = (y0 >> 8) & 0xFF;
	_cmd_triangle[7] = y0 & 0xFF;
	_cmd_triangle[8] = (x1 >> 8) & 0xFF;
	_cmd_triangle[9] = x1 & 0xFF;
	_cmd_triangle[10] = (y1 >> 8) & 0xFF;
	_cmd_triangle[11] = y1 & 0xFF;
	_cmd_triangle[12] = (x2 >> 8) & 0xFF;
	_cmd_triangle[13] = x2 & 0xFF;
	_cmd_triangle[14] = (y2 >> 8) & 0xFF;
	_cmd_triangle[15] = y2 & 0xFF;
	_sendcmd(_cmd_triangle,20);
	return epaper_wait_ok_error(0);
}

int
epaper_draw_arc(int x0, int y0, int rx, int a0, int a1)
{
	unsigned char _cmd_draw_arc[20]={FRAME_B,0x00,0x15,CMD_DRAW_ARC,0,0,0,0,0,0,0,0,0,0,0,0,FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_cmd_draw_arc[4] = (x0 >> 8) & 0xFF;
	_cmd_draw_arc[5] = x0 & 0xFF;
	_cmd_draw_arc[6] = (y0 >> 8) & 0xFF;
	_cmd_draw_arc[7] = y0 & 0xFF;
	_cmd_draw_arc[8] = (rx >> 8) & 0xFF;
	_cmd_draw_arc[9] = rx & 0xFF;
/*	_cmd_draw_arc[10] = (ry >> 8) & 0xFF;
	_cmd_draw_arc[11] = ry & 0xFF;*/
	_cmd_draw_arc[10] = (rx >> 8) & 0xFF;
	_cmd_draw_arc[11] = rx & 0xFF;
	_cmd_draw_arc[12] = (a0 >> 8) & 0xFF;
	_cmd_draw_arc[13] = a0 & 0xFF;
	_cmd_draw_arc[14] = (a1 >> 8) & 0xFF;
	_cmd_draw_arc[15] = a1 & 0xFF;
	_sendcmd(_cmd_draw_arc,20);
	return epaper_wait_ok_error(0);
}

int
epaper_clear(void)
{
	const unsigned char _cmd_clean[8] = {FRAME_B,0x00,0x09,CMD_CLEAR,FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_sendcmd((unsigned char *)_cmd_clean,8);
	return epaper_wait_ok_error(0);
}

int
epaper_clear_color(int c)
{
	unsigned char _cmd_clean_color[9] = {FRAME_B,0x00,0x09,CMD_CLEAR_COLOR,0,FRAME_E0,FRAME_E1,FRAME_E2,FRAME_E3};
	_cmd_clean_color[4]=c;
	_sendcmd((unsigned char *)_cmd_clean_color,9);
	return epaper_wait_ok_error(0);
}

int
epaper_disp_string(const void * p, int x0, int y0)
{
	int strl;
	int cmd_size;

	strl = strlen((const char *)p) + 1;		// +1 to add the ending \0
	if (strl+12 > CMD_SIZE) strl = CMD_SIZE-12;	// avoid buffer overflow
	cmd_size = strl + 12 + 1;	// extra +1 for the checksum
	_cmd_buff[0] = FRAME_B;

	// size of the whole command
	_cmd_buff[1] = (cmd_size >> 8) & 0xFF;
	_cmd_buff[2] = cmd_size & 0xFF;

	_cmd_buff[3] = CMD_DRAW_STRING;

	// pos x
	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	// pos y
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;

	// string
	memcpy((void *)(_cmd_buff+8),p,strl);
	_cmd_buff[cmd_size-6] = 0;	// make sure the last char of the string is always \0

	_cmd_buff[cmd_size-5] = FRAME_E0;
	_cmd_buff[cmd_size-4] = FRAME_E1;
	_cmd_buff[cmd_size-3] = FRAME_E2;
	_cmd_buff[cmd_size-2] = FRAME_E3;

	_sendcmd(_cmd_buff,cmd_size-1);
	return epaper_wait_ok_error(0);
}

int
epaper_disp_string_penwidth(const void * p, int x0, int y0, int w)
{
  int x,y,res=0;
  for (x=0;x<w;x++) for (y=0;y<w;y++) {
    res = epaper_disp_string(p,x0+x,y0+y);
    if (res) return res;
    }
  return res;
}

#ifdef __AVR__
int
epaper_disp_string(const __FlashStringHelper * p, int x0, int y0)
{
	int strl;
	int cmd_size;

	strl = strlen_P((PGM_P)p) + 1;		// +1 to add the ending \0
	if (strl+12 > CMD_SIZE) strl = CMD_SIZE-12;	// avoid buffer overflow
	cmd_size = strl + 12 + 1;	// extra +1 for the checksum
	_cmd_buff[0] = FRAME_B;

	// size of the whole command
	_cmd_buff[1] = (cmd_size >> 8) & 0xFF;
	_cmd_buff[2] = cmd_size & 0xFF;

	_cmd_buff[3] = CMD_DRAW_STRING;

	// pos x
	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	// pos y
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;

	// string
	memcpy_P((void *)(_cmd_buff+8),p,strl);
	_cmd_buff[cmd_size-6] = 0;	// make sure the last char of the string is always \0

	_cmd_buff[cmd_size-5] = FRAME_E0;
	_cmd_buff[cmd_size-4] = FRAME_E1;
	_cmd_buff[cmd_size-3] = FRAME_E2;
	_cmd_buff[cmd_size-2] = FRAME_E3;

	_sendcmd(_cmd_buff,cmd_size-1);
	return epaper_wait_ok_error(0);
}

int
epaper_disp_string_penwidth(const __FlashStringHelper * p, int x0, int y0, int w)
{
  int x,y,res=0;
  for (x=0;x<w;x++) for (y=0;y<w;y++) {
    res = epaper_disp_string((const __FlashStringHelper *)p,x0+x,y0+y);
    if (res) return res;
    }
  return res;
}
#endif

int
epaper_disp_char(const char c, int x0, int y0)
{
	char buf[2] = {0,0};
	buf[0]=c;
	return epaper_disp_string(buf,x0,y0);
}

int
epaper_disp_bitmap(const void * p, int x0, int y0)
{
	int strl;
	int string_size;
	if (strchr((const char *)p,'.')==0) return 4;			// no extension =>>> error 4 (bug in firmware)

	strl = strlen((const char *)p);
	if (strl + 13 > CMD_SIZE) return 2;			// avoid buffer overflow of _cmd_buff

	string_size = strlen((const char *)p);
	string_size += 14;

	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = (string_size >> 8) & 0xFF;
	_cmd_buff[2] = string_size & 0xFF;

	_cmd_buff[3] = CMD_DRAW_BITMAP;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;

	strcpy((char *)(&_cmd_buff[8]), (const char *)p);

	string_size -= 5;

	_cmd_buff[string_size] = FRAME_E0;
	_cmd_buff[string_size + 1] = FRAME_E1;
	_cmd_buff[string_size + 2] = FRAME_E2;
	_cmd_buff[string_size + 3] = FRAME_E3;

	_sendcmd(_cmd_buff,string_size+4);
	return epaper_wait_ok_error(1);
}

#ifdef __AVR__
int
epaper_disp_bitmap(const __FlashStringHelper * p, int x0, int y0)
{
	int strl;
	int string_size;
	if (strchr_P((PGM_P)p,'.')==0) return 4;			// no extension =>>> error 4 (bug in firmware)

	strl = strlen_P((PGM_P)p);
	if (strl + 13 > CMD_SIZE) return 2;			// avoid buffer overflow of _cmd_buff

	string_size = strlen_P((PGM_P)p);
	string_size += 14;

	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = (string_size >> 8) & 0xFF;
	_cmd_buff[2] = string_size & 0xFF;

	_cmd_buff[3] = CMD_DRAW_BITMAP;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;

	strcpy_P((char *)(&_cmd_buff[8]),(PGM_P)p);

	string_size -= 5;

	_cmd_buff[string_size] = FRAME_E0;
	_cmd_buff[string_size + 1] = FRAME_E1;
	_cmd_buff[string_size + 2] = FRAME_E2;
	_cmd_buff[string_size + 3] = FRAME_E3;

	_sendcmd(_cmd_buff,string_size+4);
	return epaper_wait_ok_error(1);
}
#endif

#include "epaper.h"

void setup()
{
	epaper_wakeup();
}

void loop()
{
	epaper_set_screenrotation(EPD_90);
	epaper_clear();
	epaper_set_en_font(FONT48);
	epaper_disp_string("Hello World!",10,10);
	epaper_update();
	while (1) delay(1000);
}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "epaper.h"

int main()
{
    setbuf(stdout,NULL);
    printf("init: %d\n",epaper_init());
    printf("handshake: %d\n",epaper_handshake());

    printf("set_memory: %d\n",epaper_set_memory(MEM_TF));
    printf("screen_rotation: %d\n",epaper_set_screenrotation(3));
    printf("set_color: %d\n",epaper_set_color(EPAPER_BLACK, EPAPER_WHITE));
    printf("set_en_font: %d\n",epaper_set_en_font(FONT32));
    printf("set_ch_font: %d\n",epaper_set_ch_font(FONT32));
    printf("set_baud: %d\n",epaper_set_baud(115200));
    printf("read_baud: %d\n",epaper_read_baud());
    printf("get_memory: %d\n",epaper_get_memory());
    printf("get_screenrotation: %d\n",epaper_get_screenrotation());
    printf("get_color: %d\n",epaper_get_color());

    printf("epaper_clear: %d\n",epaper_clear());
    printf("epaper_disp_bitmap: %d\n",epaper_disp_bitmap("test.bmp",0,0));
    printf("epaper_disp_bitmap: %d\n",epaper_disp_bitmap("test2.bmp",50,50));
    printf("epaper_disp_bitmap: %d\n",epaper_disp_bitmap("test3.bmp",150,150));
    printf("epaper_disp_bitmap: %d\n",epaper_disp_bitmap("test3.bmp",150,150));

    printf("epaper_draw_pixel: %d\n",epaper_draw_pixel(10,10));
    printf("epaper_draw_line: %d\n",epaper_draw_line(15,15,50,30));
    printf("epaper_fill_rect: %d\n",epaper_rect(1,150,150,210,230));
    printf("epaper_draw_rect: %d\n",epaper_rect(0,150+90,150,210+90,230));
    printf("epaper_draw_circle: %d\n",epaper_circle(0,500,500,30));
    printf("epaper_fill_circle: %d\n",epaper_circle(1,0400,400,30));
    printf("epaper_draw_triangle: %d\n",epaper_triangle(0,100,500,110,500,105,480));
    printf("epaper_fill_triangle: %d\n",epaper_triangle(1,200,500,210,500,205,480));
    printf("epaper_disp_string: %d\n",epaper_disp_string("Hola 123",600,200));
    printf("epaper_disp_bitmap: %d\n",epaper_disp_bitmap("Hola 123.bmp",500,200));

    printf("update: %d\n",epaper_update());

    printf("fini: %d\n",epaper_fini());
    return 0;
}


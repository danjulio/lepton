#ifndef FB_H
#define FB_H

#include <stdint.h>

int init_fb(char* fb_path);
void update_fb(uint8_t* pixbuf);
void set_colormap(int n);
uint16_t get_rgb_pixel(uint8_t p);


#endif /* FB_H */

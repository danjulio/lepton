#include "log.h"
#include "fb.h"
#include "colormaps.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char *fbp = 0;
uint8_t* cmap_p = (uint8_t*) colormap_golden;


/**
 * Initialize frame buffer
 */
int init_fb(char* fb_path)
{
	long int screensize = 0;

	// Open the file for reading and writing
	fbfd = open(fb_path, O_RDWR);
	if (fbfd == -1) {
		log_fatal("Error: cannot open framebuffer device");
		exit(1);
	}
	log_info("The framebuffer device was opened successfully.");

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
		log_fatal("Error reading fixed fb information");
		exit(2);
	}

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
		log_fatal("Error reading variable fb information");
		exit(3);
	}

	log_info("FB: %dx%d, %dbpp, length %d, offsets: %d %d", vinfo.xres, vinfo.yres,
		 vinfo.bits_per_pixel, finfo.line_length, vinfo.xoffset, vinfo.yoffset);

	// Figure out the size of the screen in bytes
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

	// Map the device to memory
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if ((int)fbp == -1) {
		log_fatal("Error: failed to map framebuffer device to memory");
	        exit(4);
	}

	log_info("Framebuffer device mapped to memory successfully.");

	return(0);
}


/*
 * Draw the 8-bit data into the frame buffer (x2 in size)
 */
void update_fb(uint8_t* pixbuf)
{
	uint8_t* bP;
	int x, y, offset;
	uint16_t t;

	for (y=0; y < 240; y=y+2) {
		// Line 1
		bP = pixbuf + y/2*160;
		for (x = 0; x < 320; x=x+2) {
			offset = (x * 2) + (y * finfo.line_length);
			t = get_rgb_pixel(*bP++);
			*((uint16_t*)(fbp + offset)) = t;
			*((uint16_t*)(fbp + offset + 2)) = t;
		}
		// Line 2
		bP = pixbuf + y/2*160;
		for (x = 0; x < 320; x=x+2) {
			offset = (x * 2) + ((y+1) * finfo.line_length);
			t = get_rgb_pixel(*bP++);
			*((uint16_t*)(fbp + offset)) = t;
			*((uint16_t*)(fbp + offset + 2)) = t;
		}
	}
}


void set_colormap(int n)
{
	switch (n) {
		case 0:
			cmap_p = (uint8_t*) colormap_golden;
			break;
		case 1:
			cmap_p = (uint8_t*) colormap_rainbow;
			break;
		case 2:
			cmap_p = (uint8_t*) colormap_grayscale;
			break;
		case 3:
			cmap_p = (uint8_t*) colormap_ironblack;
			break;
		default:
			cmap_p = (uint8_t*) colormap_golden;
			break;
	}
}


uint16_t get_rgb_pixel(uint8_t p)
{
	uint8_t r, g, b;
	uint8_t* cP;

	cP = cmap_p + p*3;
	r = *(cP) / 8;
	g = *(++cP) / 4;
	b = *(++cP) / 8;
	return r<<11 | g<<5 | b;
}


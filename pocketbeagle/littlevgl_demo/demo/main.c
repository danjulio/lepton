#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "lv_examples/lv_apps/demo/demo.h"
#include <unistd.h>

#define DISP_BUF_SIZE (320*240*2)

int main(void)
{
	/*LittlevGL init*/
	lv_init();

	/*Linux frame buffer device init*/
	fbdev_init();

	/*Linux touchscreen device init*/
	evdev_init();

	/*A small buffer for LittlevGL to draw the screen's content*/
	static lv_color_t buf1[DISP_BUF_SIZE];
	static lv_color_t buf2[DISP_BUF_SIZE];

	/*Initialize a descriptor for the buffer*/
	static lv_disp_buf_t disp_buf;
	lv_disp_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

	/*Initialize and register a display driver*/
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.buffer = &disp_buf;
	disp_drv.flush_cb = fbdev_flush;
	lv_disp_drv_register(&disp_drv);

	/*Initialize and register an input device*/
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = evdev_read;
	lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv);

	 /*Create demo*/
	demo_create();

	/*Handle LitlevGL tasks (tickless mode)*/
	while(1) {
		lv_tick_inc(5);
		lv_task_handler();
		usleep(5000);
	}

	return 0;
}

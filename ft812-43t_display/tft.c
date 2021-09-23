
#include "EVE.h"
#include "tft_data.h"
#include "images_carbon.h"
#include "images_gauge_scale.h"
#include "images_battery.h"
#include "images_rpm.h"
#include "images_pressure.h"
#include "images_air_intake_temp.h"
#include "images_air_intake.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

/* some pre-definded colors */
#define RED		0xff0000UL
#define ORANGE	0xffa500UL
#define GREEN	0x00ff00UL
#define BLUE	0x0000ffUL
#define BLUE_1	0x5dade2L
#define YELLOW	0xffff00UL
#define PINK	0xff00ffUL
#define PURPLE	0x800080UL
#define WHITE	0xffffffUL
#define BLACK	0x000000UL

#define MEM_DL_STATIC (EVE_RAM_G_SIZE - 4096) /* 0xff000 - start-address of the static part of the display-list, upper 4k of gfx-mem */

uint32_t num_dl_static; /* amount of bytes in the static part of our display-list */
uint8_t tft_active = 0;
uint16_t gauge_val = 210;

_Bool batt_en = false;
_Bool rpm_en = false;
_Bool press_en = false;
_Bool air_tmp_en = false;
_Bool air_en = false;

uint32_t rpm = 0;
uint32_t vlt = 0;
int32_t air_tmp = 0;
uint32_t map = 0;
uint32_t maf = 0;
int32_t boost = 0;

int gauge_buff[8] = {0};
int buff_pos = 0;
long sum = 0;

void touch_calibrate(void)
{

/* activate this if you are using a module for the first time or if you need to re-calibrate it */
/* write down the numbers on the screen and either place them in one of the pre-defined blocks above or make a new block */
#if 1
	/* calibrate touch and displays values to screen */
	EVE_cmd_dl(CMD_DLSTART);
	EVE_cmd_dl(DL_CLEAR_RGB | BLACK);
	EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
	EVE_cmd_text((EVE_HSIZE/2), 50, 26, EVE_OPT_CENTER, "Please tap on the dot.");
	EVE_cmd_calibrate();
	EVE_cmd_dl(DL_DISPLAY);
	EVE_cmd_dl(CMD_SWAP);
	EVE_cmd_execute();

	uint32_t touch_a, touch_b, touch_c, touch_d, touch_e, touch_f;

	touch_a = EVE_memRead32(REG_TOUCH_TRANSFORM_A);
	touch_b = EVE_memRead32(REG_TOUCH_TRANSFORM_B);
	touch_c = EVE_memRead32(REG_TOUCH_TRANSFORM_C);
	touch_d = EVE_memRead32(REG_TOUCH_TRANSFORM_D);
	touch_e = EVE_memRead32(REG_TOUCH_TRANSFORM_E);
	touch_f = EVE_memRead32(REG_TOUCH_TRANSFORM_F);

	EVE_cmd_dl(CMD_DLSTART);
	EVE_cmd_dl(DL_CLEAR_RGB | BLACK);
	EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
	EVE_cmd_dl(TAG(0));

	EVE_cmd_text(5, 15, 26, 0, "TOUCH_TRANSFORM_A:");
	EVE_cmd_text(5, 30, 26, 0, "TOUCH_TRANSFORM_B:");
	EVE_cmd_text(5, 45, 26, 0, "TOUCH_TRANSFORM_C:");
	EVE_cmd_text(5, 60, 26, 0, "TOUCH_TRANSFORM_D:");
	EVE_cmd_text(5, 75, 26, 0, "TOUCH_TRANSFORM_E:");
	EVE_cmd_text(5, 90, 26, 0, "TOUCH_TRANSFORM_F:");

	EVE_cmd_setbase(16L);
	EVE_cmd_number(310, 15, 26, EVE_OPT_RIGHTX|8, touch_a);
	EVE_cmd_number(310, 30, 26, EVE_OPT_RIGHTX|8, touch_b);
	EVE_cmd_number(310, 45, 26, EVE_OPT_RIGHTX|8, touch_c);
	EVE_cmd_number(310, 60, 26, EVE_OPT_RIGHTX|8, touch_d);
	EVE_cmd_number(310, 75, 26, EVE_OPT_RIGHTX|8, touch_e);
	EVE_cmd_number(310, 90, 26, EVE_OPT_RIGHTX|8, touch_f);

	EVE_cmd_dl(DL_DISPLAY);	/* instruct the graphics processor to show the list */
	EVE_cmd_dl(CMD_SWAP); /* make this list active */
	EVE_cmd_execute();
#endif
}

void TFT_init(void)
{
	if(EVE_init() != 0)
	{
		tft_active = 1;

		EVE_memWrite8(REG_PWM_DUTY, 0x80);	/* setup backlight, range is from 0 = off to 0x80 = max */
		touch_calibrate();

		EVE_cmd_inflate(RAM_IMAGES_CARBON, images_carbon, sizeof(images_carbon));
		EVE_cmd_inflate(RAM_IMAGES_GAUGE_SCALE, images_gauge_scale, sizeof(images_gauge_scale));
		EVE_cmd_inflate(RAM_IMAGES_BATTERY, images_battery, sizeof(images_battery));
		EVE_cmd_inflate(RAM_IMAGES_RPM, images_rpm, sizeof(images_rpm));
		EVE_cmd_inflate(RAM_IMAGES_PRESSURE, images_pressure, sizeof(images_pressure));
		EVE_cmd_inflate(RAM_IMAGES_AIR_INTAKE_TEMP, images_air_intake_temp, sizeof(images_air_intake_temp));
		EVE_cmd_inflate(RAM_IMAGES_AIR_INTAKE, images_air_intake, sizeof(images_air_intake));


		EVE_cmd_dl(CMD_DLSTART); /* Start the display list */
		EVE_cmd_dl(VERTEX_FORMAT(0)); /* reduce precision for VERTEX2F to 1 pixel instead of 1/16 pixel default */
		EVE_cmd_dl(DL_COLOR_RGB | WHITE);
		EVE_cmd_dl(DL_BEGIN | EVE_BITMAPS);
		EVE_cmd_setbitmap(RAM_IMAGES_CARBON, EVE_ARGB1555, 480, 272);
		EVE_cmd_dl(VERTEX2F(0, 0));
		EVE_cmd_setbitmap(RAM_IMAGES_GAUGE_SCALE, EVE_ARGB1555, 480, 272);
		EVE_cmd_dl(VERTEX2F(0, 0));

		EVE_color_rgb(0xFFC335UL);
		EVE_cmd_text(136, 168, 29, EVE_OPT_CENTERY, "-0.4");
		EVE_cmd_text(106, 240, 31, EVE_OPT_CENTERY, "-1");
		EVE_cmd_text(212, 134, 30, EVE_OPT_CENTERY, "0.25");
		EVE_cmd_text(298, 167, 29, EVE_OPT_CENTERY, "0.9");
		EVE_cmd_text(314, 237, 31, EVE_OPT_CENTERY, "1.5");
		EVE_cmd_text(120, 195, 26, EVE_OPT_CENTERY, "-0.7");
		EVE_cmd_text(184, 144, 26, EVE_OPT_CENTERY, "0.1");
		EVE_cmd_text(285, 143, 26, EVE_OPT_CENTERY, "0.6");
		EVE_cmd_text(334, 198, 26, EVE_OPT_CENTERY, "1.2");
		EVE_cmd_text(189, 207, 30, EVE_OPT_CENTERY, "TURBO");
		EVE_cmd_text(211, 233, 16, EVE_OPT_CENTERY, "x100kPA");

		EVE_cmd_dl(DL_END);

		while (EVE_busy());
		num_dl_static = EVE_memRead16(REG_CMD_DL);
		EVE_cmd_memcpy(MEM_DL_STATIC, EVE_RAM_DL, num_dl_static);
		while (EVE_busy());
	}
}

static _Bool check_coordinate(uint32_t touch_point, uint16_t x_start, uint16_t y_start,uint16_t x_size, uint16_t y_size)
{
	volatile uint16_t touch_x, touch_y;
	touch_y = touch_point & 0x0000FFFF;
	touch_x = touch_point >> 16;

	if((touch_x > x_start) && (touch_x < (x_size+x_start)) && (touch_y > y_start) && (touch_y < (y_size+y_start)))
	{
		return true;
	}

	return false;
}

/* check for touch events and setup vars for TFT_display() */
void TFT_touch(void)
{
	_Bool tag;
	volatile uint32_t coordinates;
	
	if(EVE_busy()) /* is EVE still processing the last display list? */
	{
		return;
	}

	/*Get the touch xy coordinates*/
	coordinates = EVE_memRead32(REG_TOUCH_SCREEN_XY);

	/*Toggle the state of the battery info*/
	tag = check_coordinate(coordinates, 12, 3, 52, 42);
	if(tag)
	{
		while(tag)
		{
			CyDelay(50);
			coordinates = EVE_memRead32(REG_TOUCH_SCREEN_XY);
			tag = check_coordinate(coordinates, 12, 3, 52, 42);
		}

		batt_en = !batt_en;
	}

	/*Toggle the state of the tachometer info*/
	tag = check_coordinate(coordinates, 98, 3, 67, 40);
	if(tag)
	{
		while(tag)
		{
			CyDelay(50);
			coordinates = EVE_memRead32(REG_TOUCH_SCREEN_XY);
			tag = check_coordinate(coordinates, 98, 3, 67, 40);
		}

		rpm_en = !rpm_en;
	}

	/*Toggle the state of the barometer info*/
	tag = check_coordinate(coordinates, 205, 10, 84, 32);
	if(tag)
	{
		while(tag)
		{
			CyDelay(50);
			coordinates = EVE_memRead32(REG_TOUCH_SCREEN_XY);
			tag = check_coordinate(coordinates, 205, 10, 84, 32);
		}

		press_en = !press_en;
	}

	/*Toggle the state of the intake air thermometer info*/
	tag = check_coordinate(coordinates, 321, 3, 52, 47);
	if(tag)
	{
		while(tag)
		{
			CyDelay(50);
			coordinates = EVE_memRead32(REG_TOUCH_SCREEN_XY);
			tag = check_coordinate(coordinates, 321, 3, 52, 47);
		}

		air_tmp_en = !air_tmp_en;
	}

	/*Toggle the state of the intake air thermometer info*/
	tag = check_coordinate(coordinates, 405, 5, 61, 38);
	if(tag)
	{
		while(tag)
		{
			CyDelay(50);
			coordinates = EVE_memRead32(REG_TOUCH_SCREEN_XY);
			tag = check_coordinate(coordinates, 405, 5, 61, 38);
		}

		air_en = !air_en;
	}
}


static int16_t convert_gauge_scale(int32_t boost)
{
	uint16_t gauge_value = arg_y[0];
	float boost_value = 0;
	uint16_t i = 0;

	boost_value = boost/1000;
	if(boost_value < arg_x[0])
	{
		return arg_y[0];
	}
	else if(boost_value > arg_x[580])
	{
		return arg_y[580];
	}
	else
	{
		for(i = 0; i <= 580; i++)
		{
			if(arg_x[i] >= boost_value)
			{
				return arg_y[i];
			}
		}
	}

	return gauge_value;
}

static int MovingAvg(int *ptrArrNumbers, long *ptrSum, int pos, int len, int nextNum)
{
  /*Subtract the oldest number from the previous sum, add the new number*/
  *ptrSum = *ptrSum - ptrArrNumbers[pos] + nextNum;

  /*Assign the nextNum to the position in the array*/
  ptrArrNumbers[pos] = nextNum;

  /*Return the average*/
  return *ptrSum / len;
}


/*
	dynamic portion of display-handling, meant to be called every 20ms or more
*/
void TFT_display(void)
{
	char output_text[32];
	int gauge_av = 0;
	int buff_len;
	
	if(tft_active != 0)
	{
		EVE_start_cmd_burst(); /* start writing to the cmd-fifo as one stream of bytes, only sending the address once */
		EVE_cmd_dl_burst(CMD_DLSTART); /* start the display list */
		EVE_cmd_dl_burst(DL_CLEAR_RGB | WHITE); /* set the default clear color to white */
		EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG); /* clear the screen - this and the previous prevent artifacts between lists, Attributes are the color, stencil and tag buffers */
		EVE_cmd_dl_burst(TAG(0));
		EVE_cmd_append_burst(MEM_DL_STATIC, num_dl_static); /* insert static part of display-list from copy in gfx-mem */

		/*Filter and draw gauge values*/
		EVE_color_rgb_burst(0xFF0000UL);
		gauge_val = convert_gauge_scale(boost);
		buff_len = sizeof(gauge_buff)/sizeof(int);
		gauge_av = MovingAvg(gauge_buff, &sum, buff_pos, buff_len, (int)gauge_val);
	    buff_pos++;
	    if(buff_pos >= buff_len){buff_pos = 0;}
		EVE_cmd_gauge_burst(240, 262, 229, EVE_OPT_FLAT | EVE_OPT_NOBACK | EVE_OPT_NOTICKS, 10, 1, (uint16_t)gauge_av, 1000);

		/*Draw battery info*/
		if(batt_en)
		{
			EVE_color_rgb_burst(0xFF0000UL);
			EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
			EVE_cmd_setbitmap_burst(RAM_IMAGES_BATTERY, EVE_ARGB1555, 52, 42);
			EVE_cmd_dl_burst(VERTEX2F(12, 3));
			EVE_color_rgb_burst(0xFFFFFFUL);
			memset(output_text, 0x00, sizeof(output_text));
			sprintf(output_text, "%.1fV", (double)vlt);
			EVE_cmd_text_burst(15, 59, 27, EVE_OPT_CENTERY, output_text);
		}
		else
		{
			EVE_color_rgb_burst(0x000000UL);
			EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
			EVE_cmd_setbitmap_burst(RAM_IMAGES_BATTERY, EVE_ARGB1555, 52, 42);
			EVE_cmd_dl_burst(VERTEX2F(12, 3));
		}

		/*Draw RPM info*/
		if(rpm_en)
		{
			EVE_color_rgb_burst(0xFF9D00UL);
			EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
			EVE_cmd_setbitmap_burst(RAM_IMAGES_RPM, EVE_ARGB1555, 67, 40);
			EVE_cmd_dl_burst(VERTEX2F(98, 3));
			EVE_color_rgb_burst(0xFFFFFFUL);
			memset(output_text, 0x00, sizeof(output_text));
			sprintf(output_text, "%drpm", (int)rpm);
			EVE_cmd_text_burst(91, 60, 27, EVE_OPT_CENTERY, output_text);
		}
		else
		{
			EVE_color_rgb_burst(0x000000UL);
			EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
			EVE_cmd_setbitmap_burst(RAM_IMAGES_RPM, EVE_ARGB1555, 67, 40);
			EVE_cmd_dl_burst(VERTEX2F(98, 3));
		}

		/*Draw pressure info*/
		if(press_en)
		{
			EVE_color_rgb_burst(0xD7007DUL);
			EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
			EVE_cmd_setbitmap_burst(RAM_IMAGES_PRESSURE, EVE_ARGB1555, 84, 32);
			EVE_cmd_dl_burst(VERTEX2F(205, 10));
			EVE_color_rgb_burst(0xFFFFFFUL);
			memset(output_text, 0x00, sizeof(output_text));
			sprintf(output_text, "%dkPa", (int)map);
			EVE_cmd_text_burst(212, 59, 27, EVE_OPT_CENTERY, output_text);
		}
		else
		{
			EVE_color_rgb_burst(0x000000UL);
			EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
			EVE_cmd_setbitmap_burst(RAM_IMAGES_PRESSURE, EVE_ARGB1555, 84, 32);
			EVE_cmd_dl_burst(VERTEX2F(205, 10));
		}

		/*Draw intake air temperature info*/
		if(air_tmp_en)
		{
			EVE_color_rgb_burst(0x00B8FFUL);
			EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
			EVE_cmd_setbitmap_burst(RAM_IMAGES_AIR_INTAKE_TEMP, EVE_ARGB1555, 52, 47);
			EVE_cmd_dl_burst(VERTEX2F(321, 3));
			EVE_color_rgb_burst(0xFFFFFFUL);
			memset(output_text, 0x00, sizeof(output_text));
			sprintf(output_text, "%d", (int)air_tmp);
			EVE_cmd_text_burst(352, 58, 27, EVE_OPT_CENTERY|EVE_OPT_RIGHTX, output_text);
			EVE_cmd_text_burst(356, 58, 27, EVE_OPT_CENTERY, "C");
			EVE_cmd_text_burst(367, 53, 26, EVE_OPT_CENTERY, "o");
		}
		else
		{
			EVE_color_rgb_burst(0x000000UL);
			EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
			EVE_cmd_setbitmap_burst(RAM_IMAGES_AIR_INTAKE_TEMP, EVE_ARGB1555, 52, 47);
			EVE_cmd_dl_burst(VERTEX2F(321, 3));
		}

		/*Draw intake air mass info*/
		if(air_en)
		{
			EVE_color_rgb_burst(0x0000FFUL);
			EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
			EVE_cmd_setbitmap_burst(RAM_IMAGES_AIR_INTAKE, EVE_ARGB1555, 61, 38);
			EVE_cmd_dl_burst(VERTEX2F(405, 5));
			EVE_color_rgb_burst(0xFFFFFFUL);
			memset(output_text, 0x00, sizeof(output_text));
			sprintf(output_text, "%dg/sec", (int)maf);
			EVE_cmd_text_burst(398, 58, 27, EVE_OPT_CENTERY, output_text);
		}
		else
		{
			EVE_color_rgb_burst(0x000000UL);
			EVE_cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
			EVE_cmd_setbitmap_burst(RAM_IMAGES_AIR_INTAKE, EVE_ARGB1555, 61, 38);
			EVE_cmd_dl_burst(VERTEX2F(405, 5));
		}

		EVE_cmd_dl_burst(DL_DISPLAY); /* instruct the graphics processor to show the list */
		EVE_cmd_dl_burst(CMD_SWAP); /* make this list active */
		EVE_end_cmd_burst(); /* stop writing to the cmd-fifo, the cmd-FIFO will be executed automatically after this or when DMA is done */
	}
}

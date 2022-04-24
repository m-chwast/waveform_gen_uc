/*
 * lcd.h
 *
 *  Created on: Jan 14, 2022
 *      Author: mchwast
 *
 * /////contains hi-level functions allowing to use lcd
 */

#ifndef INC_LCD_LCD_H_
#define INC_LCD_LCD_H_

#include <string>
#include "lcd/st7920.h"



class point
{
	uint8_t x;
	uint8_t y;
};


class graphic_lcd
{
	private:
		//st7920 driver;
		void set_pixel(point pixel);
		void reset_pixel(point pixel);
	public:
		void line(point start, point end);



};



#endif /* INC_LCD_LCD_H_ */

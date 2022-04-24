/*
 * st7920.h
 *
 *  Created on: Jan 14, 2022
 *      Author: mchwast
 */

#ifndef INC_LCD_ST7920_H_
#define INC_LCD_ST7920_H_

/////includes
#include <cstdio>
#include "main.h"
#include "lcd/lcd.h"
#include <string>
#include <array>

///namespaces
using namespace std;

/////const data
constexpr uint8_t LCD_HEIGHT = 64;
constexpr uint8_t LCD_LENGTH = 128;
constexpr uint16_t BUFFER_SIZE = 100;



/////enums
enum class st7920_disp_type {TEXT, GRAPHIC};
enum class st7920_command {COMMAND, DATA, CLEAR_CMD, TEXT};


////classes definitions
class lcd_cmd
{
	public:
		st7920_command command;
		uint8_t row;
		uint8_t column;
};

class lcd_buffer
{
	public:
		array<lcd_cmd, BUFFER_SIZE> command_buffer;
		uint16_t first_elem_index;
		uint16_t count;
		array<array<uint16_t, LCD_LENGTH / 16>, LCD_HEIGHT> screen_buffer;
};

class st7920
{
	private:
		//data:
		st7920_disp_type display_type;
		GPIO_TypeDef* data_gpio;
		GPIO_TypeDef* e_gpio;
		GPIO_TypeDef* rs_gpio;
		GPIO_TypeDef* reset_gpio;
		uint8_t d4_pin_number, rs_pin_number;
		uint16_t en_pin, rst_pin;

		lcd_buffer buffer;

		//private functions:
		void send_nibble(uint8_t nibble);

		void lcd_send_delay(uint8_t cmd, bool rs);
		void send_string_delay(string str);
		void lcd_set_graphic_mode();

		lcd_cmd* get_transaction();
		uint8_t transaction_add(uint8_t row_or_command, uint8_t column, uint16_t data, st7920_command cmd_type);
		void transaction_remove();

		void start_timer();

	public:
		//constructors
		st7920(GPIO_TypeDef& data, uint16_t data_pin, GPIO_TypeDef& e, uint16_t e_pin, GPIO_TypeDef& rs, uint16_t rs_pin,
				GPIO_TypeDef& reset, uint16_t reset_pin, st7920_disp_type disp_type);
		st7920();
		//public functions:
		void init();
		void set_mode(st7920_disp_type mode);
		void send_byte(uint8_t byte_to_send, bool rs);
		void set_address(uint8_t addr);
		/*! append text*/
		void write(string text)	{write(text, 0, 0);}
		/*! write in selected place */
		void write(string text, uint8_t row)	{write(text, row, 0);}
		void write(string text, uint8_t row, uint8_t column);
		void reverse_line(uint8_t line);

		void clear();

		void send_data(uint8_t row, uint8_t column, uint16_t data);
		void send_command(uint8_t command);



		uint8_t transaction_handler();
		void stop_timer();
		//destructor:
		~st7920() {}
};


////exported objects:
extern st7920 display;



#endif /* INC_LCD_ST7920_H_ */

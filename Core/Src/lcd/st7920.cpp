#include "lcd/st7920.h"
#include "main.h"
#include "tim.h"


/////commands:
//command macros
#define INT_TO_BOOL(X) ((X) ? 1 : 0)
//basic instructions:
#define CLEAR 0b1
#define HOME 0b10
#define ENTRY_MODE(I_D,S) (0b100 + (INT_TO_BOOL((I_D)) << 1) + INT_TO_BOOL((S)))
#define DISPLAY_ON_OFF(D,C,B) (0b1000 + (INT_TO_BOOL((D)) << 2) + (INT_TO_BOOL((C)) << 1) + INT_TO_BOOL((B)))
#define CURSOR_DISPLAY_CONTROL(S_C,R_L) (0b10000 + (INT_TO_BOOL((S_C)) << 3) + (INT_TO_BOOL((R_L)) << 2))
#define FUNCTION_SET(DL,RE) (0b100000 + (INT_TO_BOOL((DL)) << 4) + (INT_TO_BOOL((RE)) << 2))
#define SET_CGRAM_ADDR(ADDR) (0b1000000 + (ADDR))
#define SET_DDRAM_ADDR(ADDR) (0b10000000 + (uint8_t)(ADDR))
//extended instruction set:
#define STAND_BY CLEAR
#define SCROLL 0b11
#define REVERSE(R1,R0) (0b100 + (INT_TO_BOOL((R1)) << 1) + INT_TO_BOOL((R0)))
#define EXTENDED_FUNCTION_SET(DL, G) (0b100100 + (INT_TO_BOOL((DL)) << 4) + (INT_TO_BOOL((G)) << 1))
#define SET_IRAM_ADDR(ADDR) SET_CGRAM_ADDR((ADDR))
#define SET_GDRAM_ADDR(ADDR) SET_DDRAM_ADDR((ADDR))

/////constants:
constexpr uint16_t lcd_wait_time_us = 80;


/////global variables:
st7920 display;


////extern variables
extern TIM_HandleTypeDef htim2;

/////functions:

////constructors
/*
st7920::st7920(GPIO_TypeDef& data, uint16_t data_pin, GPIO_TypeDef& e, uint16_t e_pin, GPIO_TypeDef& rs, uint16_t rs_pin,
		GPIO_TypeDef& reset, uint16_t reset_pin, st7920_disp_type disp_type)
{
	data_gpio = &data;
	e_gpio = &e;
	rs_gpio = &rs;
	reset_gpio = &reset;
	en_pin  = e_pin;
	rst_pin = reset_pin;
	display_type = disp_type;
	d4_pin_number = 0;
	for(; data_pin > 0; data_pin >>= 1)
		d4_pin_number++;
	rs_pin_number = 0;
	for(; rs_pin > 0; rs_pin >>= 1)
		rs_pin_number++;
}*/

st7920::st7920()
{
	data_gpio = LCD_D4_GPIO_Port;
	e_gpio = LCD_E_GPIO_Port;
	rs_gpio = LCD_RS_GPIO_Port;
	reset_gpio = LCD_RESET_GPIO_Port;
	en_pin  = LCD_E_Pin;
	rst_pin = LCD_RESET_Pin;
	display_type = st7920_disp_type::GRAPHIC;
	d4_pin_number = 0;
	uint16_t pin = LCD_D4_Pin;
	if(pin != 0)
	{
		for(; pin > 0; pin >>= 1)
			d4_pin_number++;
		d4_pin_number--;
	}
	rs_pin_number = 0;
	pin = LCD_RS_Pin;
	if(pin != 0)
	{
		for(; pin > 0; pin >>= 1)
			rs_pin_number++;
		rs_pin_number--;
	}
}

////other functions:

inline void st7920::send_nibble(uint8_t nibble)
{
	e_gpio->BSRR = en_pin;	//set E pin
	data_gpio->BSRR = nibble << d4_pin_number;		//set data bits
	e_gpio->BSRR = en_pin << 16;	//reset E pin
	data_gpio->BSRR = (0b1111 << 16) << d4_pin_number;	//reset data bus
}

void st7920::send_byte(uint8_t byte_to_send, bool rs)
{
	rs_gpio->BSRR = rs << rs_pin_number;
	send_nibble(byte_to_send >> 4);
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	send_nibble(byte_to_send & 0x0F);
	rs_gpio->BSRR = (rs << 16) << rs_pin_number;
}

inline void st7920::lcd_send_delay(uint8_t cmd, bool rs)
{
	send_byte(cmd, rs);
	HAL_Delay(1);
}

void st7920::set_address(uint8_t addr)
{
	send_byte(SET_GDRAM_ADDR(addr), 0);
}

void st7920::send_string_delay(string str)
{

	for(char c : str)
		lcd_send_delay(c, 1);
	if((str.length() % 2) != 0)	//because single lcd address contains two characters
		lcd_send_delay(' ', 1);


}

inline void st7920::lcd_set_graphic_mode()
{
	lcd_send_delay(EXTENDED_FUNCTION_SET(0, 0), 0);
	lcd_send_delay(EXTENDED_FUNCTION_SET(0, 1), 0);
}


void st7920::init()
{

	HAL_GPIO_WritePin(reset_gpio, rst_pin, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(reset_gpio, rst_pin, GPIO_PIN_SET);
	HAL_Delay(50);
	HAL_GPIO_WritePin(rs_gpio, 1 << rs_pin_number, GPIO_PIN_RESET);
	send_nibble(0b0010);
	HAL_Delay(1);
	lcd_send_delay(FUNCTION_SET(0,0), 0);
	lcd_send_delay(DISPLAY_ON_OFF(1,0,0), 0);
	lcd_send_delay(CLEAR, 0);
	HAL_Delay(15);
	lcd_send_delay(ENTRY_MODE(1,0), 0);
	HAL_Delay(5);
	lcd_send_delay(HOME, 0);
	HAL_Delay(200);
	lcd_send_delay(CLEAR, 0);
	HAL_Delay(5);
	lcd_set_graphic_mode();
	display_type = st7920_disp_type::GRAPHIC;
	clear();
	HAL_Delay(500);
	set_mode(st7920_disp_type::TEXT);
	HAL_Delay(50);
}

void st7920::set_mode(st7920_disp_type mode)
{
	if(display_type == mode)
		return;
	display_type = mode;
	switch(mode)
	{
		case st7920_disp_type::GRAPHIC:
			send_command(CLEAR);
			send_command(EXTENDED_FUNCTION_SET(0, 0));
			send_command(EXTENDED_FUNCTION_SET(0, 1));
			break;
		case st7920_disp_type::TEXT:
			send_command(EXTENDED_FUNCTION_SET(0, 0));
			send_command(FUNCTION_SET(0, 0));
			break;
		default: break;
	}
}

void st7920::clear()
{
	if(display_type == st7920_disp_type::GRAPHIC)
		transaction_add(1, 1, 1, st7920_command::CLEAR_CMD);
	else if(display_type == st7920_disp_type::TEXT)
		send_command(CLEAR);
}

static inline uint8_t clear_display_exe(uint8_t * first_run_ptr)
{
	static uint16_t clearing_progress = 0;
	if(clearing_progress >= LCD_HEIGHT * LCD_LENGTH / 16)
	{
		clearing_progress = 0;
		return 0;	//end of clearing
	}
	else if(*first_run_ptr == 1 || *first_run_ptr == 0)	//this if will run twice
	{
		display.set_address((1 - *first_run_ptr) * (clearing_progress / 16));
		*first_run_ptr += 1;
	}
	else
	{
		if(*first_run_ptr == 2)
		{
			*first_run_ptr = 3;
			display.send_byte(0xF0, 1);
		}
		else
		{
			*first_run_ptr = 2;
			display.send_byte(0x00, 1);
			clearing_progress++;
			if((clearing_progress % 16) == 0)
				*first_run_ptr = 0;
		}
	}
	return 1;	//clearing not done yet
}

void st7920::send_data(uint8_t row, uint8_t column, uint16_t data)
{
	if((buffer.screen_buffer[row][column] ^ data) == 0)	//return when data is already in the buffer
		return;
	transaction_add(row, column, data, st7920_command::DATA);
}

void st7920::send_command(uint8_t command)
{
	transaction_add(command, 1, 1, st7920_command::COMMAND);
}

void st7920::write(string text, uint8_t row, uint8_t column)
{
	if((column % 2) != 0)
	{
		text = " " + text;
		column--;
	}
	uint8_t addr = column / 2;
	switch(row)
	{
		case 0:	break;
		case 1: addr += 16; break;
		case 2: addr += 8; break;
		case 3: addr += 24; break;
		default: break;
	}

	transaction_add(SET_DDRAM_ADDR(addr), 0, 0, st7920_command::COMMAND);
	for(char c : text)
		transaction_add(c, 0, 0, st7920_command::TEXT);
	if((text.length() % 2) != 0)
		transaction_add(' ', 0, 0, st7920_command::TEXT);
}

void st7920::reverse_line(uint8_t line)
{
	/*send_command(FUNCTION_SET(0, 1));
	send_command(REVERSE(0, 0));
	send_command(FUNCTION_SET(0, 0));*/
}

///////circular buffer handling:

static inline void update_oc_register()	{TIM2->CCR1 = (TIM2->CNT + lcd_wait_time_us) % TIM2->ARR;}
static inline void update_oc_register_long_delay()	{TIM2->CCR1 = (TIM2->CNT + lcd_wait_time_us * 22) % TIM2->ARR;}

void st7920::start_timer()
{

	if(htim2.ChannelState[0] != HAL_TIM_CHANNEL_STATE_BUSY)
	{
		if(HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1) != HAL_OK)
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		update_oc_register();
	}
}

void st7920::stop_timer()
{
	HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
}

lcd_cmd* st7920::get_transaction()
{
	if(buffer.count == 0)
		return NULL;
	else
		return &buffer.command_buffer[buffer.first_elem_index];

}

uint8_t st7920::transaction_add(uint8_t row_or_command, uint8_t column, uint16_t data, st7920_command cmd_type)
{
	if(buffer.count >= BUFFER_SIZE)
			return 0;
	buffer.count++;
	//reentrancy at this point will generate a bug, repair TODO
	uint8_t trans_number = (buffer.first_elem_index + buffer.count - 1) % BUFFER_SIZE;
	lcd_cmd tmp_trans;
	tmp_trans.command = cmd_type;
	tmp_trans.row = row_or_command;
	tmp_trans.column = column;
	if(cmd_type == st7920_command::DATA)
		buffer.screen_buffer[row_or_command][column] = data;
	buffer.command_buffer[trans_number] = tmp_trans;
	start_timer();
	return 1;
}

void st7920::transaction_remove()
{
	if(buffer.count > 0)
	{
		buffer.count--;
		buffer.first_elem_index = (buffer.first_elem_index + 1) % BUFFER_SIZE;
	}
}

uint8_t st7920::transaction_handler()
{
	static lcd_cmd* transaction;
	static uint8_t address_sent[2] = {1, 1}, data_sent[2] = {1, 1};
	if(address_sent[1] == 1 && data_sent[1] == 1)
	{
		transaction = get_transaction();
		if(transaction != NULL)
		{
			address_sent[0] = 0;
			address_sent[1] = 0;
			data_sent[0] = 0;
			data_sent[1] = 0;
		}
	}
	if(transaction != NULL)
	{
		if(transaction->command == st7920_command::COMMAND)
		{
			send_byte(transaction->row, 0);
			transaction_remove();
			address_sent[1] = 1;
			data_sent[1] = 1;
		}
		else if(transaction->command == st7920_command::CLEAR_CMD)	//addres_sent[0] -- flag
		{
			if(clear_display_exe(&address_sent[0]) == 0)
			{
				data_sent[1] = 1;
				address_sent[1] = 1;
				for(int i = 0; i < LCD_HEIGHT; i++)
					for(int j = 0; j < LCD_LENGTH / 16; j++)
						buffer.screen_buffer[i][j] = 0;
				transaction_remove();
			}
		}
		else if(transaction->command == st7920_command::TEXT)
		{
			send_byte(transaction->row, 1);
			transaction_remove();
			address_sent[1] = 1;
			data_sent[1] = 1;
		}
		else if(transaction->command == st7920_command::DATA)
		{
			if(address_sent[1] == 0)
			{
				if(address_sent[0] == 0)
				{
					uint8_t tmp_line = transaction->row;
					tmp_line %= 32;
					set_address(tmp_line);
					address_sent[0] = 1;
				}
				else
				{
					uint8_t tmp_column = transaction->column;
					if(transaction->row >= 32)
						tmp_column += 8;
					set_address(tmp_column);
					address_sent[1] = 1;
				}
			}
			else if(data_sent[1] == 0)
			{
				if(data_sent[0] == 0)
				{
					send_byte(buffer.screen_buffer[transaction->row][transaction->column] >> 8, 1);
					data_sent[0] = 1;
				}
				else
				{
					send_byte(buffer.screen_buffer[transaction->row][transaction->column] & 0xFF, 1);
					data_sent[1] = 1;
					transaction_remove();
				}
			}
		}
		if(transaction->command == st7920_command::COMMAND && transaction->row == CLEAR)
			update_oc_register_long_delay();
		else
			update_oc_register();
		return 1;
	}
	else
	{
		return 0;
	}
}




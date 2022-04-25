#include "uart/uart.h"
#include "usart.h"
#include <string>
#include <cstring>
#include "main.h"
#include "device.h"
#include "wave/waveform.h"
#include "tim.h"

using namespace std;


/////main uart instance:

uart_class uart;



/////extern global variables:
extern device_state_t device_state;
extern wave waveform;
extern TIM_HandleTypeDef htim2;

///new types:
enum class transmission_state_t {NONE, START, DATA};


/////classes functions:

uart_class::uart_class()
{
	abort = false;
}

void uart_class::init()
{
	HAL_UART_Receive_IT(&huart2, &in_buffer[in_buffer_elem_count], 1);
}

void uart_class::init(uint bytes_no)
{
	HAL_UART_Receive_IT(&huart2, &in_buffer[in_buffer_elem_count], bytes_no);
}

void uart_class::start_timeout()
{
	htim2.Instance->CCR4 = (htim2.Instance->CNT + 50000) % htim2.Instance->ARR;	//50 ms timeout
	htim2.Instance->SR &= ~(TIM_SR_CC4IF);	//have to clear interrupt flag
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_4);
}

void uart_class::stop_timeout()
{
	HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_4);
}

void uart_class::abort_reception()
{
	HAL_UART_AbortReceive_IT(&huart2);
	uart.in_buffer_elem_count = 0;
	init();
	abort = true;
}


void uart_class::start_transmission()
{
	if(current_transmission == NULL && out_buffer.size() != 0)
	{
		current_transmission = &out_buffer.front();
		HAL_UART_Transmit_IT(&huart2, &(out_buffer.front())[0], out_buffer.front().size());
	}
}

void uart_class::send(char character)
{
		vector<uint8_t> tmp = {character};
		out_buffer.push_back(tmp);
		start_transmission();
}

void uart_class::send(string text)
{
	vector<uint8_t> tmp_vect(text.size());
	for(uint i = 0; i < text.size(); i++)
		tmp_vect[i] = text[i];
	out_buffer.push_back(tmp_vect);
	start_transmission();
}

void uart_class::send(uint8_t * data, uint16_t size)
{
	vector<uint8_t> tmp_vect(size);
	for(uint i = 0; i < size; i++)
		tmp_vect[i] = data[i];
	out_buffer.push_back(tmp_vect);
	start_transmission();
}

void uart_class::transmit_next()
{
	if(current_transmission != NULL)
	{
		out_buffer.pop_front();
		if(out_buffer.begin() != out_buffer.end())	////need to check if next message is available
		{
			current_transmission = &out_buffer.front();
			HAL_UART_Transmit_IT(&huart2, &(out_buffer.front())[0], out_buffer.front().size());
		}
		else
			current_transmission = NULL;
	}
}

void uart_class::receive()
{
	static transmission_state_t transmission_state = transmission_state_t::NONE;


	if(device_state == device_state_t::LOADING)
	{
		static uint16_t progress = 0;
		if(abort_rx_status() == true)	//timeout occurred
		{
			progress = 0;
			uint8_t tmp = in_buffer[in_buffer_elem_count];
			in_buffer.fill(0);
			in_buffer_elem_count = 1;
			in_buffer[0] = tmp;
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
			transmission_state = transmission_state_t::NONE;
			device_state = device_state_t::IDLE;
			init();
			uart.abort_reset();
			return;
		}

		in_buffer_elem_count = 0;
		if(transmission_state == transmission_state_t::START)
		{
			waveform.dac_divider = ((int16_t)in_buffer[0]) << 8;
			waveform.dac_divider |= (int16_t)in_buffer[1];
			waveform.sample_no = ((int16_t)in_buffer[2]) << 8;
			waveform.sample_no |= (int16_t)in_buffer[3];
			std::string tmpstr = "";
			tmpstr += (char)(waveform.sample_no >> 8);
			tmpstr += (char)(waveform.sample_no & 0xFF);
			tmpstr += '\n';
			send(tmpstr);
			start_timeout();
			transmission_state = transmission_state_t::DATA;
			init(200);
			return;
		}
		else if(transmission_state == transmission_state_t::DATA)
		{
			for(int i = 0; i < 100; i++)
			{
				if(progress >= waveform.sample_no)
					break;
				waveform.samples[progress] = ((int16_t)in_buffer[i * 2]) << 8;
				waveform.samples[progress] |= (int16_t)in_buffer[i * 2 + 1];
				progress++;
			}
			if(progress < waveform.sample_no)
			{
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				send("OK\n");
				start_timeout();
				init(200);
				return;
			}
			else
			{
				stop_timeout();
				send("END\n");
				progress = 0;
				in_buffer.fill(0);
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
				transmission_state = transmission_state_t::NONE;
				device_state = device_state_t::IDLE;
			}
		}
	}
	else if(in_buffer[in_buffer_elem_count] == '\n' && device_state != device_state_t::LOADING)
	{
		char tmp[in_buffer_elem_count + 1];
		for(int i = 0; i < in_buffer_elem_count; i++)
			tmp[i] = in_buffer[i];
		tmp[in_buffer_elem_count] = 0;
		for(int i = 0; i <= in_buffer_elem_count; i++)
			in_buffer[i] = 0;
		if(strcmp(tmp, "wlacz") == 0)
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		else if(strcmp(tmp, "wylacz") == 0)
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		else if(strcmp(tmp, "wave") == 0)
		{
			in_buffer.fill(0);
			device_state = device_state_t::LOADING;
			send("LD\n");
			transmission_state = transmission_state_t::START;
			in_buffer_elem_count = 0;
			start_timeout();
			init(4);
			return;
		}
		in_buffer_elem_count = 0;
	}
	else if(device_state != device_state_t::LOADING)
	{
		in_buffer_elem_count = (in_buffer_elem_count + 1) % UART_BUFFER_SIZE;
		if(in_buffer_elem_count == 0)
			in_buffer.fill(0);
	}
	init();
}

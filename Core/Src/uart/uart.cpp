#include "uart/uart.h"
#include "usart.h"
#include <string>
#include <cstring>
#include "main.h"
#include "device.h"
#include "wave/waveform.h"


using namespace std;


/////main uart instance:

uart_class uart;



/////extern global variables:
extern device_state_t device_state;
extern wave waveform;

///new types:
enum class transmission_state_t {NONE, START, DATA};


/////classes functions:

uart_class::uart_class()
{

}

void uart_class::init()
{
	HAL_UART_Receive_IT(&huart2, &in_buffer[in_buffer_elem_count], 1);
}

void uart_class::init(uint bytes_no)
{
	HAL_UART_Receive_IT(&huart2, &in_buffer[in_buffer_elem_count], bytes_no);
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
	uint16_t tim1 = 0, tim2 = 0;
	tim1 = TIM7->CNT;
	static transmission_state_t transmission_state = transmission_state_t::NONE;


	if(device_state == device_state_t::LOADING)
	{
		static uint16_t progress = 0;

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
			transmission_state = transmission_state_t::DATA;
			init(200);
			return;
		}
		else if(transmission_state == transmission_state_t::DATA)
		{
			int sum = 0;
			for(int i = 0; i < 100; i++)
			{
				if(progress >= waveform.sample_no)
				{
					break;
				}
				waveform.samples[progress] = ((int16_t)in_buffer[i * 2]) << 8;
				waveform.samples[progress] |= (int16_t)in_buffer[i * 2 + 1];
				progress++;
				sum += waveform.samples[progress];
			}
			/*char tmp[5];
			for(int i = 0; i < 4; i++)
				tmp[i] = (sum >> ((3-i) * 8)) & 0xFF;
			tmp[4] = '\n';
			std::string tmpstr = tmp;
			send(tmpstr);*/
			if(progress < waveform.sample_no)
			{
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				send("OK\n");
				init(200);
				return;
			}
			else
			{
				send("END\n");
				progress = 0;
				in_buffer.fill(0);
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
				transmission_state = transmission_state_t::NONE;
				device_state = device_state_t::IDLE;
			}
		}
		/*
		if(progress >= 2 + waveform.sample_no)
		{
			progress = 0;
			device_state = device_state_t::IDLE;
			in_buffer.fill(0);
			waveform.samples.fill(0);
			waveform.dac_divider = 1;
			waveform.sample_no = 0;
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		}
		else
		{
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			in_buffer[0] = 0;
			in_buffer[1] = 0;
			init(2);
			return;
		}*/

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
			init(4);
			return;
		}
		in_buffer_elem_count = 0;
		tim2 = TIM7->CNT;
		//send(to_string(tim2 - tim1) + '\n');
	}
	else if(device_state != device_state_t::LOADING)
	{
		in_buffer_elem_count = (in_buffer_elem_count + 1) % UART_BUFFER_SIZE;
		if(in_buffer_elem_count == 0)
			in_buffer.fill(0);
	}
	init();
}

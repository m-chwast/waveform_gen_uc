#include "tim.h"
#include  "lcd/st7920.h"
#include "lcd/menu.h"
#include "uart/uart.h"





///////TIM interrupt handlers:


void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)	//OC interrupt handling
{
	if(htim == &htim2)
	{
		if((htim->Channel) == HAL_TIM_ACTIVE_CHANNEL_1)	//handling lcd
		{
				//if(display.transaction_handler() != 0)
				//	update_oc_register();
				//else	//transaction pointer was null
			if(display.transaction_handler() == 0)
				display.stop_timer();

		}
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)	//handling pc transmission timeout
		{
			uart.abort_reception();
			uart.stop_timeout();
			uart.send("TOUT\n");
		}
	}

}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim)
{
	if(htim == &htim6)	//encoder handling
	{
		static int16_t old_enc;
		uint16_t current_enc = (htim3.Instance->CNT >> 2);
		int16_t diff = current_enc - old_enc;
		old_enc = current_enc;
		if((int8_t)diff > 0)
			menu.next();
		else if((int8_t)diff < 0)
			menu.prev();

		static bool old_enc_button = true;
		bool current_enc_button = HAL_GPIO_ReadPin(ENC_SW_GPIO_Port, ENC_SW_Pin);
		if((current_enc_button != old_enc_button) && (current_enc_button == false))
			menu.enter();
		old_enc_button = current_enc_button;
		menu.display_values();
	}
}

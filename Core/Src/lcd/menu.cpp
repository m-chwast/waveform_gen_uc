#include "lcd/menu.h"
#include "lcd/st7920.h"
#include "device.h"
#include <array>


//////global menu object
menu_class menu;


////exported variables
extern device_state_t device_state;


//////menu_class functions:
menu_class::menu_class()
{
	pos = 0;
	position_active = false;
	current_menu = &main_menu;
	first_displayed = 0;
	main_menu =
	{
		menu_item_unordered{"Run", NULL, NULL, NULL, true, 0},
		menu_item_unordered{"Waveform", NULL, &waveform_menu},
		menu_item_unordered{"History", NULL, NULL, NULL},
		menu_item_unordered{"Menu2", NULL, NULL, NULL},
		menu_item_unordered{"Settings", NULL, &settings_menu, NULL},
		menu_item_unordered{"Menu3", NULL, NULL, NULL, true, 556},
		menu_item_unordered{"USART", NULL, &usart_menu, NULL}
	};

	settings_menu =
	{
		menu_item_unordered{"Set1", NULL},
		menu_item_unordered{"Set2", NULL},
		menu_item_unordered{"Set3", NULL},
		menu_item_unordered{"Set4", NULL},
		menu_item_unordered{"Set5", NULL},
		menu_item_unordered{"Set6", NULL},
		menu_item_unordered{"Back", &main_menu, NULL}
	};

	usart_menu =
		{
			menu_item_unordered{"BAUD", .has_value = true, .value = 9600},
			menu_item_unordered{"Back", &main_menu}
		};

	waveform_menu =
	{
			menu_item_unordered{"Sine"},
			menu_item_unordered{"Triangle"},
			menu_item_unordered{"Sawtooth"},
			menu_item_unordered{"Square"},
			menu_item_unordered{"Back", &main_menu}
	};

	waveform_settings_menu_sine =
	{
			menu_item_unordered{"Amplitude: ", .has_value = true, .value = 1000},
			menu_item_unordered{"Freq: ", .has_value = true, .value = 10000},
			menu_item_unordered{"Back", &waveform_menu}
	};
}

void menu_class::next()
{
	if(position_active == true)
		change_value(true);
	else if(pos + 1 >= (uint16_t)current_menu->size())
	{
		pos = 0;
		if(current_menu->size() > 4)
			menu_draw(menu_draw_mode::NEW);
		else
			menu_draw(menu_draw_mode::CURSOR_RESET);
	}
	else
	{
		pos++;
		menu_draw(menu_draw_mode::ADVANCE_1);
	}
}

void menu_class::prev()
{
	if(position_active == true)
		change_value(false);
	else
	{
		if(pos == 0)
			pos = (uint8_t)current_menu->size() - 1;
		else
			pos--;
		menu_draw(menu_draw_mode::BACK_1);
	}
}

void menu_class::enter()
{
	if((*current_menu)[pos].has_value == true)
	{
		if(position_active == false)
		{
			position_active = true;
			display.write(">>", pos - first_displayed);
		}
		else
		{
			position_active = false;
			display.write("->", pos - first_displayed);
		}
	}
	else if((*current_menu)[pos].submenu != NULL)
	{
		current_menu = (*current_menu)[pos].submenu;
		pos = 0;
		first_displayed = 0;
		menu_draw(menu_draw_mode::NEW);
		//display_values(true);
	}
	else if((*current_menu)[pos].parent != NULL)
	{
		menu_draw(menu_draw_mode::BACK_TO_PARENT);
		display_values(true);
	}
}

void menu_class::menu_draw()	{menu_draw(menu_draw_mode::NEW);}

void menu_class::menu_draw(menu_draw_mode mode)
{
	uint menu_size = current_menu->size();
	switch(mode)
	{
		case menu_draw_mode::NEW:
		{
			display.clear();
			if(pos == 0)
			{
				first_displayed = 0;
				display.write("->" + (*current_menu)[0].name);
				for(uint i = 1; i < 4; i++)
				{
					if(i < menu_size)
						display.write((*current_menu)[i].name, i, 2);
					else
						break;
				}
				display_values(true);
			}
			/*else	////done in different way
			{
				uint pos_delta = menu_size - pos;
				if(pos_delta >= 3)
					first_displayed = pos;
				else if(menu_size < 4)
					first_displayed = 0;
				else
					first_displayed = menu_size - 4;
				for(uint i = 0; i < 4; i++)
				{
					if((int)i == pos - first_displayed)
						display.write("->" + (*current_menu)[0].name, i);
					else if(i < menu_size - first_displayed)
						display.write((*current_menu)[i].name, i, 2);
					else
						display.write("                ", i);
				}
			}*/
			break;
		}
		case menu_draw_mode::ADVANCE_1:
		{
			uint pos_delta = pos - first_displayed;
			if(pos_delta <= 3)	//only have to move cursor
			{
				display.write("  ", pos_delta - 1);
				display.write("->", pos_delta);
			}
			else	//need to redraw whole menu
			{
				display.clear();
				first_displayed++;
				display.write("->" + (*current_menu)[pos].name, 3);
				for(uint i = 0; i < 3; i++)
					display.write((*current_menu)[first_displayed + i].name, i, 2);
				display_values(true);
			}
			break;
		}
		case menu_draw_mode::CURSOR_RESET:
		{
			display.write("  ", menu_size - 1);
			display.write("->");
			break;
		}
		case menu_draw_mode::BACK_1:
		{
			if(pos - first_displayed >= 0)
			{
				if(pos != menu_size - 1) //only need to redraw cursor
				{
					display.write("  ", pos - first_displayed + 1);
					display.write("->", pos - first_displayed);
				}
				else
				{
					if(menu_size <= 4)
					{
						display.write("  ");
						display.write("->", menu_size - 1);
					}
					else
					{
						first_displayed = menu_size - 4;
						display.clear();
						for(int i = 0; i < 3; i++)
							display.write((*current_menu)[first_displayed + i].name, i, 2);
						display.write("->" + (*current_menu)[pos].name, 3);
						display_values(true);
					}
				}
			}
			else
			{
				display.clear();
				if(pos != menu_size - 1)
				{
					first_displayed--;
					display.write("->" + (*current_menu)[first_displayed].name);
					for(int i = 1; i < 4; i++)
						display.write((*current_menu)[first_displayed + i].name, i, 2);
					display_values(true);
				}
			}
			break;
		}
		case menu_draw_mode::BACK_TO_PARENT:
		{
			if((*current_menu)[pos].parent != NULL)
			{
				display.clear();
				vector<menu_item_unordered> * old_menu = current_menu;
				current_menu = (*current_menu)[pos].parent;
				uint16_t current_size = current_menu->size();
				pos = 0;
				for(; (*current_menu)[pos].submenu != old_menu && pos <= current_size; pos++);
				if(pos >= current_size)
				{
					pos = 0;
					menu_draw(menu_draw_mode::NEW);
				}
				else
				{
					for(int i = 3; i >= 0; i--)	///finds first pos to display
					{
						if(pos + i < current_size)
						{
							first_displayed = pos - 3 + i;
							if(first_displayed >= current_size)	//overflow
								first_displayed = 0;
							break;
						}
					}
					for(int i = 0; i < 4; i++)
					{
						if(first_displayed + i >= current_size)
							break;
						else
							display.write((*current_menu)[first_displayed + i].name, i, 2);
					}
					display.write("->", pos - first_displayed);
				}
			}
			break;
		}
		default:
			break;
	}
}

void menu_class::display_values()	{display_values(false);}

void menu_class::display_values(bool force_display)
{
	static array<int, 4> displayed_values;

	for(uint i = 0; i < 4 && i < current_menu->size(); i++)
	{
		if((*current_menu)[first_displayed + i].has_value == true)
		{
			if((*current_menu)[first_displayed + i].value != displayed_values[i] || force_display == true)
			{
				displayed_values[i] = (*current_menu)[first_displayed + i].value;
				/*switch((int)current_menu)
				{
					case &main_menu:
					{

						break;
					}
					default:
					{
						string value_str = to_string(displayed_values[i]);
						while(value_str.length() < 6)
							value_str += " ";
						display.write(value_str, i, 10);
						break;
					}
				}*/
				if(&(*current_menu)[first_displayed + i] == &main_menu[0])
				{
					if(main_menu[0].value == 1)
						display.write("on ", i, 10);
					else
						display.write("off", i, 10);
				}
				else
				{
					string value_str = to_string(displayed_values[i]);
					while(value_str.length() < 6)
						value_str += " ";
					display.write(value_str, i, 10);
				}
			}
		}
	}
}

void menu_class::change_value(bool change_right)
{
	if(&(*current_menu)[pos] == &usart_menu[0])	//baud
	{
		if(change_right == true)
			(*current_menu)[pos].value+=100;
		else
			(*current_menu)[pos].value-=100;
	}
	else if(&(*current_menu)[pos] == &main_menu[0])	//run
	{
		if((*current_menu)[pos].value == 0)
		{
			if(device_state == device_state_t::IDLE)
			{
				(*current_menu)[pos].value = 1;
				device_state = device_state_t::RUN;
			}
		}
		else
		{
			(*current_menu)[pos].value = 0;
			device_state = device_state_t::IDLE;
		}

	}
	else
	{
		if(change_right == true)
			(*current_menu)[pos].value++;
		else
			(*current_menu)[pos].value--;
	}
}



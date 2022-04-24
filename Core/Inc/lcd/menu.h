#include <string>
#include <array>
#include <vector>


using namespace std;





/////constants
constexpr int MENU_ITEM_COUNT = 20;

////structs:

struct menu_item
{
		string name;
		menu_item * parent;
		menu_item * submenu;
		menu_item * next;
		menu_item * previous;
		void (*callback)(void);
};

struct menu_item_unordered
{
		string name;
		vector<menu_item_unordered> * parent;
		vector<menu_item_unordered> * submenu;
		void (*callback)(void);
		bool has_value;
		int value;
};



/////menu positions:






/////classes:

enum class menu_draw_mode {ADVANCE_1, BACK_1, NEW, CURSOR_RESET, BACK_TO_PARENT};

class menu_class
{
	private:
		vector<menu_item_unordered> main_menu, settings_menu, usart_menu, waveform_menu,
		waveform_settings_menu_sine, waveform_settings_menu_triangle, waveform_settings_menu_sawtooth;

		uint8_t pos;
		vector<menu_item_unordered> * current_menu;
		uint8_t first_displayed;
		bool position_active;
		void menu_draw(menu_draw_mode mode);
		void menu_draw();
		void display_values(bool force_display);
		void change_value(bool change_right);

	public:
		menu_class();

		void init()		{menu_draw();}
		void next();
		void prev();
		void display_values();
		void enter();

		~menu_class() {}
};


extern menu_class menu;


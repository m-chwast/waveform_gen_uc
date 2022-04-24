#include <array>
#include <string>
#include <list>
#include <vector>



//////constants:

constexpr uint16_t UART_BUFFER_SIZE = 200;



////classes:

class uart_class
{
	private:
		std::list<std::vector<uint8_t>> out_buffer;
		std::vector<uint8_t> * current_transmission;
		std::array<uint8_t, UART_BUFFER_SIZE> in_buffer;
		uint16_t in_buffer_elem_count;
		void start_transmission();
	public:
		uart_class();
		void init();
		void init(uint bytes_no);
		void send(std::string text);
		void send(char character);
		void send(uint8_t * data, uint16_t size);
		void receive();

		void transmit_next();
		~uart_class() {}
};




////exported objects:

extern uart_class uart;

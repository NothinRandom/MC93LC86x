#ifndef MC93LC86x_h
#define MC93LC86x_h

#if (ARDUINO >= 100)
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class MC93LC86x
{
	public:
		MC93LC86x(uint8_t csPin, boolean type); //either 1x16b or 2x8b
		void init(); //initialize 93LC86x EWEN
		void begin();
		void ewen(); //enable writing
		void ewds(); //disable writing
		void write(uint16_t value, uint16_t address);
		uint16_t read(uint16_t address);
		void eral(); //erase all
		void erase(uint16_t address);
	private:
		uint8_t _cs;
		boolean _type; //if true, for 2Kx8b
		uint8_t transfer(uint8_t data);
		void send(uint16_t packet);
};
#endif
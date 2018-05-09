#include "MC93LC86x.h"

//SPCR Settings:
// |   7  |   6   |  5   |  4   |  3   |   2  |   1  |   0  |
// | SPIE |  SPE  | DORD | MSTR | CPOL | CPHA | SPR1 | SPR0 |

// SPIE - Enables the SPI interrupt when 1
// SPE - Enables the SPI when 1
// DORD - Sends data least Significant Bit First when 1, most Significant Bit first when 0
// MSTR - Sets the Arduino in master mode when 1, slave mode when 0
// CPOL - Sets the data clock to be idle when high if set to 1, idle when low if set to 0
// CPHA - Samples data on the falling edge of the data clock when 1, rising edge when 0
// SPR1 and SPR0 - Sets the SPI speed, 00 is fastest (4MHz) 11 is slowest (250KHz)

// SPI2X SPR1 SPR0 SCK Freq.  Actual SCK speed @ 16Mhz
// 0     0    0    fosc/4     4Mhz    
// 0     0    1    fosc/16    1Mhz
// 0     1    0    fosc/64    250Khz
// 0     1    1    fosc/128   125Khz
// 1     0    0    fosc/2     8Mhz
// 1     0    1    fosc/8     2Mhz
// 1     1    0    fosc/32    500Khz
// 1     1    1    fosc/64    125Khz

//default is EWDS at startup
//EWEN is required for all programming mode

//false for 1Kx16b 
//true for 2Kx8b
MC93LC86x::MC93LC86x(uint8_t csPin, boolean type)
{
	_cs = csPin;
	_type = type;
	pinMode(SCK, OUTPUT);
	pinMode(MOSI, OUTPUT);
	pinMode(MISO, INPUT);	
	pinMode(_cs, OUTPUT);
	digitalWrite(_cs, LOW); //active HIGH
}

void MC93LC86x::begin()
{
	// digitalWrite(SCK, HIGH);
	// digitalWrite(MOSI, HIGH);	
	// digitalWrite(MISO, HIGH);
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<SPR0); //spi mode0, 125kHz
	//SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA)|(1<<SPR0); //spi mode1
	//SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPOL); //spi mode2
	//SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPOL)|(1<<CPHA); //mode3
	//SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPOL)|(1<<SPR0); //SPCR = B01011001; //enable spi, master, mode2, 1MHz	
}

//set up 93LC86 with erase/write enabled
void MC93LC86x::init() 
{ 
//	ewen();
	// digitalWrite(_cs, HIGH);
	// send(0x8000);
	// digitalWrite(_cs, LOW);
} 

//EWEN 00010011	XXXXXXXX //00010011	00000000b : 1300h : 4864d					11	00000000
void MC93LC86x::ewen()
{
	uint16_t output;
	output = 0x1300;
	if(_type) //2Kx8
		output << 1;
	digitalWrite(_cs, HIGH);
	send(output);
	digitalWrite(_cs, LOW);	
}

//EWDS 100 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0 //00010000	00000000b : 1000h : 4096d
void MC93LC86x::ewds()
{
	uint16_t output;
	output = 0x1000;
	if(_type) //2Kx8
		output << 1;	
	digitalWrite(_cs, HIGH);
	send(output);
	digitalWrite(_cs, LOW);	
}

//WRITE 101 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0 //00010100	00000000b : 1400h : 5120d
void MC93LC86x::write(uint16_t address, uint16_t value)
{
	if(_type) //organized = 0
		address = (0x1400 << 1) | address;
	else
		address |= 0x1400;
	ewen();
	digitalWrite(_cs, HIGH);
	send(address); //send in CMD and address
	if(_type) //true for 8b
		transfer((uint8_t)value);
	else //16b
		send(value);
	digitalWrite(_cs, LOW);
	ewds();	
}

//READ 110 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0 //00011000	00000000b : 1800h : 6144d
uint16_t MC93LC86x::read(uint16_t address)
{ 
	uint16_t value = 0;
	if(_type) 
		address = (0x1800 << 1) | address;
	else
		address |= 0x1800;
	digitalWrite(_cs, HIGH);
	send(address);
	if(!_type) //16b
		value += (transfer(0x00) << 8) & 0xFF00; //grab hi byte from (D15-D8)
	else
		transfer(0x00); //dummy so ignore
	value += transfer(0x00) & 0xFF; //grab lo byte from (D7-D0)
	digitalWrite(_cs, LOW);
	return value;
} 

//ERAL addresses //00010001	XXXXXXXXb : 1100h : 4608d
void MC93LC86x::eral()
{
	uint16_t output = 0;
	output = 0x1100;
	if(_type)
		output << 1;	
	digitalWrite(_cs, HIGH);
	send(output);
	digitalWrite(_cs, LOW);	
}

//ERASE specific address //00011100	00000000b : 1C00h : 7168d
void MC93LC86x::erase(uint16_t address)
{
	address |= 0x1C00;
	if(_type)
		address << 1;	
	digitalWrite(_cs, HIGH);
	send(address);
	digitalWrite(_cs, LOW);	
}

uint8_t MC93LC86x::transfer(volatile byte data)
{
	begin();
	SPDR = data;
	while (!(SPSR & (1 << SPIF))); //wait until transfer is done
	return SPDR; //return received byte
}

void MC93LC86x::send(uint16_t packet)
{
    transfer((uint8_t)(packet >> 8));
    transfer((uint8_t)packet);
}
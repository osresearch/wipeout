/** \file
 * PS1 controller interface for analog sticks.
 *
 * Based on PSX Controller Decoder Library (Psx.cpp)
 * by: Kevin Ahrendt June 22nd, 2008
 *
 * Controller protocol implemented using Andrew J McCubbin's analysis.
 * http://www.gamesx.com/controldata/psxcont/psxcont.htm
 *
 * Be sure to select board type Joystick/Serial
 */

static const int command_pin = 9;
static const int clock_pin = 7;
static const int data_pin = 10;
static const int att_pin = 8;
static const int bit_delay = 100;

uint8_t psx_shift(uint8_t x)
{
	uint8_t in = 0;

	for (int i = 0; i < 8; i++, x >>= 1)
	{
		digitalWrite(command_pin, (x & 1) ? HIGH : LOW);
		digitalWrite(clock_pin, LOW);
		
		delayMicroseconds(bit_delay);

		uint8_t bit = digitalRead(data_pin);
		
		in = (in >> 1) | (bit ? 0 : 0x80);

		digitalWrite(clock_pin, HIGH);
		delayMicroseconds(bit_delay);
	}

	return in;
}


void psx_setup()
{
	pinMode(data_pin, INPUT_PULLUP);
	pinMode(command_pin, OUTPUT);
	pinMode(att_pin, OUTPUT);
	pinMode(clock_pin, OUTPUT);

	digitalWrite(att_pin, HIGH);
	digitalWrite(clock_pin, HIGH);
}

static const uint8_t out_bytes[] = {
	0x01, // 0 read device id
	0x42, // 1 send data command
	0xFF, // 2
	0xFF, // 3
	0xFF, // 4
	0xFF, // 5
	0xFF, // 6
	0xFF, // 7
	0xFF, // 8
};

static const int num_bytes = sizeof(out_bytes) / sizeof(*out_bytes);
uint8_t bytes[num_bytes];


unsigned int psx_read()
{
	digitalWrite(att_pin, LOW);

	for(int i = 0 ; i < num_bytes ; i++)
		bytes[i] = psx_shift(out_bytes[i]);

	digitalWrite(att_pin, HIGH);

	for(int i = 0 ; i < num_bytes ; i++)
	{
		Serial.print(" ");
		Serial.print(bytes[i], HEX);
	}

	Serial.println();
}


void setup()
{
	Joystick.useManualSend(true);
	psx_setup();
}

void loop()
{
	psx_read();

	// we only support the fancy dual stick right now
	// id 0x53, which is ~0xAC
	if (bytes[1] != 0xAC)
		return;

	// analog axes
	Joystick.X(bytes[5] * 4);
	Joystick.Y(bytes[6] * 4);
	Joystick.Z(bytes[7] * 4);
	Joystick.Zrotate(bytes[8] * 4);

	// hat 
	switch(bytes[3] & 0xF0)
	{
	default:
	case 0x00: Joystick.hat(-1); break;
	case 0x10: Joystick.hat(0); break;
	case 0x30: Joystick.hat(45); break;
	case 0x20: Joystick.hat(90); break;
	case 0x60: Joystick.hat(135); break;
	case 0x40: Joystick.hat(180); break;
	case 0xC0: Joystick.hat(225); break;
	case 0x80: Joystick.hat(270); break;
	case 0x90: Joystick.hat(315); break;
	};

	// other buttons
	uint8_t buttons = bytes[4];
	for(int i = 0 ; i < 8 ; i++, buttons >>= 1)
		Joystick.button(i+1, buttons & 1);

	// and some final buttons
	buttons = bytes[3] & 0x0F;
	for(int i = 0 ; i < 4 ; i++, buttons >>= 1)
		Joystick.button(i+9, buttons & 1);

	// flush the packet to the computer
	Joystick.send_now();
}

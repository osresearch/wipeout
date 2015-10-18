/** \file
 * PS1 controller interface for analog sticks.
 *
 * Based on PSX Controller Decoder Library (Psx.cpp)
 * by: Kevin Ahrendt June 22nd, 2008
 *
 * Controller protocol implemented using Andrew J McCubbin's analysis.
 * http://www.gamesx.com/controldata/psxcont/psxcont.htm
 *
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


unsigned int psx_read()
{
	digitalWrite(att_pin, LOW);

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

	for(int i = 0 ; i < num_bytes ; i++)
		bytes[i] = psx_shift(out_bytes[i]);

	digitalWrite(att_pin, HIGH);

	for(int i = 0 ; i < num_bytes ; i++)
	{
		Serial.print(" ");
		Serial.print(bytes[i]);
	}

	Serial.println();
}


void setup()
{
	psx_setup();
}

void loop()
{
	psx_read();
}

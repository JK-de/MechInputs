// QEIx$ simple sketch (c) 2017 by Jochen Krapf
// released under the GPLv3 license to match the rest of the MechInputs library

#include <MechInputs.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Pin numbers for encoder inputs
#define PINA           6
#define PINB           7
#define PINI           8

QEIx4 encoder = QEIx4();
unsigned long millisNext = 0;

void setup() {
	Serial.begin(115200);           // set up Serial library at 115200 bps
	Serial.println("Button test (MechInputs)");

	pinMode(PINA, INPUT_PULLUP);
	pinMode(PINB, INPUT_PULLUP);
	pinMode(PINI, INPUT_PULLUP);

	encoder.begin(4); // This initializes the library.
}

void loop() {

	bool bInA = digitalRead(PINA);
	bool bInB = digitalRead(PINB);
	bool bInI = digitalRead(PINI);

	encoder.doProcess(bInA, bInB, bInI);

	// check for status change 5 times per second
	if (millisNext <= millis())
	{
		millisNext = millis() + 200;

		if (encoder.hasChanged())
		{
			long counter = encoder.read();
			Serial.println(counter);
		}
	}
}

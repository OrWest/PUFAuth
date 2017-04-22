/*
 Name:		Server.ino
 Created:	18.03.2017 12:27:13
 Author:	Alex Motor
*/

#include <SD.h>

#define SIGNUP
#define Client Serial1

void setup() {
	Serial.begin(115200);
	Client.begin(57600);

	while (!Serial) {}
	Serial.println("Start");

	randomSeed(analogRead(0));

	if (!SD.begin(10)) {
		Serial.println("SD error.");
		while (true);
	}
}

// the loop function runs over and over again until power down or reset
void loop() {
#ifdef SIGNUP
	ClientSignUp();
#else
	ClientAuth();
#endif // SIGNUP

}

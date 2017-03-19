/*
 Name:		Server.ino
 Created:	18.03.2017 12:27:13
 Author:	Alex Motor
*/

#include <SD.h>

#define Client Serial1

void setup() {
	Serial.begin(115200);
	Client.begin(57600);

	if (!SD.begin(10)) {
		Serial.println("SD error.");
		while (true);
	}
}

// the loop function runs over and over again until power down or reset
void loop() {
	ClientHandle();
}

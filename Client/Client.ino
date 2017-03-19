/*
 Name:		Client.ino
 Created:	18.03.2017 12:26:58
 Author:	Alex Motor
*/
#include <SoftwareSerial.h>

#define ID F("AN0")

SoftwareSerial Server(3, 4); // RX, TX

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	Server.begin(57600);

	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);

	SaveDumpAddresses();
	TryAuth();
}

// the loop function runs over and over again until power down or reset
void loop() {

}

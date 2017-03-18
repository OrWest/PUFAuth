/*
 Name:		Client.ino
 Created:	18.03.2017 12:26:58
 Author:	Alex Motor
*/
#include <SoftwareSerial.h>

SoftwareSerial Serial1(3, 4); // RX, TX

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	Serial1.begin(57600);

	SignUp();
}

// the loop function runs over and over again until power down or reset
void loop() {

}

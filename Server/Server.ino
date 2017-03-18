/*
 Name:		Server.ino
 Created:	18.03.2017 12:27:13
 Author:	Alex Motor
*/

// the setup function runs once when you press reset or power the board

void setup() {
	Serial.begin(115200);
	Serial1.begin(57600);
}

// the loop function runs over and over again until power down or reset
void loop() {
	ClientHandle();
}

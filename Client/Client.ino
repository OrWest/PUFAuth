	/*
 Name:		Client.ino
 Created:	18.03.2017 12:26:58
 Author:	Alex Motor
*/
#include <SoftwareSerial.h>

SoftwareSerial Server(3, 4); // RX, TX

// Функция, которая вызывается для настройки микроконтроллера. Вызывается один раз.
void setup() {
	// Настройка Serial-соединения
	Serial.begin(115200);
	Server.begin(57600);

	// Настройки светодиода
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);

	SaveDumpAddresses();
	TryAuth();
}

// Функция, которая вызывается после настройки микроконтроллера и повторяется в цикле до выключения устройства.
void loop() {

}

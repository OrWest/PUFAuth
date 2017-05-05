/*
 Name:		Server.ino
 Created:	18.03.2017 12:27:13
 Author:	Alex Motor
*/

#include <SD.h>

#define SIGNUP
#define Client Serial1

// Функция, которая вызывается для настройки микроконтроллера. Вызывается один раз.
void setup() {
	// Настройка Serial-соединения
	Serial.begin(115200);
	Client.begin(57600);

	// Настройки светодиода
	pinMode(13, OUTPUT);

	// Ожидание подключения debug Serial-соединения
	while (!Serial) {}
	Serial.println("Start");

	// Установка начального вектора для псевдослучайного генератора чисел
	randomSeed(analogRead(0));

	// Инициализация SD-хранилища
	if (!SD.begin(10)) {
		Serial.println("SD error.");
		while (true) {
			digitalWrite(13, HIGH);
			delay(500);
			digitalWrite(13, LOW);
			delay(500);
		}
	}

	// Зажигание светодиода
	digitalWrite(13, HIGH);
}

// Функция, которая вызывается после настройки микроконтроллера и повторяется в цикле до выключения устройства.
void loop() {
#ifdef SIGNUP
	ClientSignUp();
#else
	ClientAuth();
#endif // SIGNUP

}

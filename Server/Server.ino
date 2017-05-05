/*
 Name:		Server.ino
 Created:	18.03.2017 12:27:13
 Author:	Alex Motor
*/

#include <SD.h>

#define SIGNUP
#define Client Serial1

// �������, ������� ���������� ��� ��������� ����������������. ���������� ���� ���.
void setup() {
	// ��������� Serial-����������
	Serial.begin(115200);
	Client.begin(57600);

	// ��������� ����������
	pinMode(13, OUTPUT);

	// �������� ����������� debug Serial-����������
	while (!Serial) {}
	Serial.println("Start");

	// ��������� ���������� ������� ��� ���������������� ���������� �����
	randomSeed(analogRead(0));

	// ������������� SD-���������
	if (!SD.begin(10)) {
		Serial.println("SD error.");
		while (true) {
			digitalWrite(13, HIGH);
			delay(500);
			digitalWrite(13, LOW);
			delay(500);
		}
	}

	// ��������� ����������
	digitalWrite(13, HIGH);
}

// �������, ������� ���������� ����� ��������� ���������������� � ����������� � ����� �� ���������� ����������.
void loop() {
#ifdef SIGNUP
	ClientSignUp();
#else
	ClientAuth();
#endif // SIGNUP

}

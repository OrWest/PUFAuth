	/*
 Name:		Client.ino
 Created:	18.03.2017 12:26:58
 Author:	Alex Motor
*/
#include <SoftwareSerial.h>

SoftwareSerial Server(3, 4); // RX, TX

// �������, ������� ���������� ��� ��������� ����������������. ���������� ���� ���.
void setup() {
	// ��������� Serial-����������
	Serial.begin(115200);
	Server.begin(57600);

	// ��������� ����������
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);

	SaveDumpAddresses();
	TryAuth();
}

// �������, ������� ���������� ����� ��������� ���������������� � ����������� � ����� �� ���������� ����������.
void loop() {

}

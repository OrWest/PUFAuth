#include <SD.h>

#define kAuth F("Auth")
#define kMemSize F("MemSize")
#define kMemContext F("MemContext")
#define kNeedReboot F("NeedReboot")
#define kSignedUp F("SignedUp")

String WaitString() {
	while (Serial1.available() == 0);

	return Serial1.readStringUntil('\r');
}

void ReadBytesAndWriteToSDFile(String fileName, int bytesCount) {
	if (!SD.begin(10)) {
		Serial.println("SD error.");
		while (true);
	}
	File file = SD.open(fileName, FILE_WRITE);

	for (int i = 0; i < bytesCount; i++) {
		file.print(Serial1.read());
	}

	file.close();
}

void ClientHandle() {
	Serial.println("Start server. Waiting for client...");
	String command = WaitString();

	Serial.println(command);
	if (command == kAuth) {
		Serial.println(kMemSize);
		Serial1.println(kMemSize);


		while (Serial1.available() == 0);
		int size = Serial1.parseInt();

		int minSize = 800;
		Serial1.println(size);
		if (size > minSize) {
			Serial.println(kMemContext);
			Serial1.println(kMemContext);

			ReadBytesAndWriteToSDFile("test.txt", size);

			Serial.println(kNeedReboot);
			Serial1.println(kNeedReboot);
		}
		else {
			Serial.print(size);
			Serial.print(" less then ");
			Serial.println(minSize);
		}
	}
	else {
		Serial.print(command);
		Serial.print(" not equal to ");
		Serial.println(kAuth);
	}
}

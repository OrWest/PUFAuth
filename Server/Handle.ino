#define kAuth F("Auth@")
#define kMemSize F("MemSize")
#define kMemContext F("MemContext")
#define kNeedReboot F("NeedReboot")
#define kSignedUp F("SignedUp")

#define dumpMaxCount 2
#define byteMinCount 800
#define timeout 5000

inline String WaitString() {
	while (Client.available() == 0);

	String string = Client.readStringUntil('\r');
	Client.read();
	Client.read();

	return string;
}

inline bool WaitByte() {
	long time = millis();
	while (Client.available() == 0) {
		if (millis() - time > timeout) {
			return false;
		}
	}
	return true;
}

inline void ClearRX() {
	while (Client.read() >= 0);
}

void ReadBytesAndWriteToSDFile(String fileName, int bytesCount) {
	File file = SD.open(fileName, FILE_WRITE);

	int i = 0;
	while (i < bytesCount) {
		if (!WaitByte()) {
			Serial.println("Bytes misses. Rollback.");
			file.close();
			SD.remove(fileName);
			return;
		}
		
		uint8_t byte = Client.read();
		file.write(byte);
		i++;
	}

	file.close();
}

String fileNameToSave(String ID) {
	if (!SD.exists(ID)) {
		SD.mkdir(ID);
	}

	int dumpCount = 0;
	String fileName;
	while (dumpCount < dumpMaxCount)
	{
		String peekName = ID + "/dump" + String(dumpCount) + ".txt";
		if (!SD.exists(peekName)) {
			fileName = peekName;
			break;
		}
		dumpCount++;
	}

	Serial.println(fileName);
	return fileName;
}

inline void closeFiles(File files[], int count) {
	for (int i = 0; i < count; i++)
	{
		files[i].close();
	}
}

uint8_t maskFromBytes(uint8_t bytes[]) {

	uint8_t orMask = bytes[0] ^ bytes[1];
	for (byte i = 1; i < dumpMaxCount - 1; i++)
	{
		orMask |= bytes[i] ^ bytes[i + 1];
	}

	return ~orMask;
}

void fillMaskFile(File files[], File maskFile) {
	for (int i = 0; i < files[0].size(); i++) {
		uint8_t bytes[dumpMaxCount];
		for (int j = 0; j < dumpMaxCount; j++) {
			bytes[j] = files[j].read();
		}
		uint8_t maskByte = maskFromBytes(bytes);
		maskFile.write(maskByte);
	}
}

void removeDumpsExceptFirst(String ID) {
	for (int i = 1; i < dumpMaxCount; i++) {
		String fileName = ID + "/dump" + String(i) + ".txt";
		SD.remove(fileName);
	}
}

bool generateXorMask(String ID) {
	File files[dumpMaxCount];

	int dumpCount = 0;
	while (dumpCount < dumpMaxCount)
	{
		String peekName = ID + "/dump" + String(dumpCount) + ".txt";
		if (!SD.exists(peekName)) {
			Serial.println(peekName + " doesn't exist. Sign up hasn't been completed yet.");

			closeFiles(files, dumpCount);
			return false;
		}
		else {
			files[dumpCount] = SD.open(peekName, FILE_READ);
		}
		dumpCount++;
	}

	Serial.println("Create mask file.");
	File maskFile = SD.open(ID + "/mask.txt", FILE_WRITE);
	fillMaskFile(files, maskFile);

	closeFiles(files, dumpMaxCount);
	maskFile.close();

	Serial.println("Remove unnecessary dumps.");
	removeDumpsExceptFirst(ID);

	return true;
}

bool SignUpCompleted(String ID) {

	bool signedUp = generateXorMask(ID);
	if (signedUp) {
		Serial.println("Sign up has completed. Generate xOR mask and remove other dumps.");
	}
	else {
		Serial.println("Sign up hasn't been completed yet.");
	}

	return signedUp;
}

void ClientHandle() {
	Serial.println("Start server. Waiting for client...");
	String request = WaitString();
	String authCommand = request.substring(0, 5);
	String ID = request.substring(5);

	Serial.println(authCommand);
	Serial.println(ID);
	if (authCommand == kAuth) {
		Serial.println(kMemSize);
		Client.println(kMemSize);


		while (Client.available() == 0);
		int size = Client.parseInt();

		Serial.println(size);
		if (size > byteMinCount) {
			ClearRX();

			Serial.println(kMemContext);
			Client.println(kMemContext);

			ReadBytesAndWriteToSDFile(fileNameToSave(ID), size);

			String commandToResponse = SignUpCompleted(ID) ? kSignedUp : kNeedReboot;

			Serial.println(commandToResponse);
			Client.println(commandToResponse);
		}
		else {
			Serial.print(size);
			Serial.print(" less then ");
			Serial.println(byteMinCount);
		}
	}
	else {
		Serial.print(authCommand);
		Serial.print(" not equal to ");
		Serial.println(kAuth);
	}
}

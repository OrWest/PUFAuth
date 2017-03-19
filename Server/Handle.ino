#define kAuthRequest F("Auth@")
#define kAuth F("Auth")
#define kSignUp F("SignUp")
#define kMemSize F("MemSize")
#define kMemContext F("MemContext")
#define kNeedReboot F("NeedReboot")
#define kSignedUp F("SignedUp")
#define kAuthorized F("Authorized")
#define kAccessDenied F("AccessDenied")

#define dumpMaxCount 5
#define byteMinCount 800
#define timeout 5000
#define maxCheckBytes 4
#define requiredCheckBytes 4

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
			Serial.println("Bytes missed. Rollback.");
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

void SignUp(String ID) {
	Serial.println(kSignUp);
	Client.println(kSignUp);

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

uint8_t WaitAndGetByte() {
	while (Client.available() == 0);

	uint8_t value = Client.read();
	Client.read();

	return value;
}

uint8_t SendAddrAndWaitValue(uint8_t addr) {
	ClearRX();

	Serial.println(addr, HEX);
	Client.write(addr);

	uint8_t receivedValue = WaitAndGetByte();
	Serial.println(receivedValue, HEX);
	return receivedValue;
}

uint8_t *BytesFrom(File file, uint8_t addrs[], int count) {
	uint8_t *bytes = (uint8_t *)calloc(sizeof(uint8_t), count);

	for (int i = 0; i < count; i++) {
		file.seek(addrs[i]);
		bytes[i] = file.read();
	}

	return bytes;
}

bool AccessAllowed(uint8_t receivedBytes[], uint8_t dumpBytes[], uint8_t maskBytes[], int count) {
	Serial.println("Start check bytes.");

	int chechBytesCount = 0;
	int missedBytesCount = 0;
	for (int i = 0; i < count; i++) {
		Serial.println(receivedBytes[i], BIN);
		Serial.println(dumpBytes[i], BIN);
		Serial.println(maskBytes[i], BIN);

		int value = receivedBytes[i] ^ dumpBytes[i];
		Serial.print("XOR: ");
		Serial.println(value, BIN);
		int mask = maskBytes[i];

		for (int j = 0; j < 8; j++) {
			if (mask & 0x01 == 1) {
				Serial.print("Mask and value shifting(Mask has 1): i=");
				Serial.print(i);
				Serial.print(" mask=");
				Serial.println(mask, BIN);
				chechBytesCount++;
				Serial.print("Missed += ");
				Serial.println(value & 0x01);
				missedBytesCount += value & 0x01;
			}
			mask >>= 1;
			value >>= 1;
		}
	}

	Serial.print("Bytes count=");
	Serial.print(chechBytesCount);
	Serial.print(" Missed count=");
	Serial.println(missedBytesCount);

	double missedPercent = missedBytesCount / (double)chechBytesCount;
	Serial.print("Missed percent=");
	Serial.println(missedPercent);

	return missedPercent < 0.1;
}

void Auth(String ID) {
	Serial.println(kAuth);
	Client.println(kAuth);

	File dump = SD.open(ID + "/dump0.txt");
	File mask = SD.open(ID + "/mask.txt");
	long fileSize = dump.size();

	uint8_t randomAddrs[6];
	for (int i = 0; i < maxCheckBytes; i++) {
		uint8_t randomAddr = random(fileSize);
		randomAddrs[i] = randomAddr;
	}
	uint8_t *dumpBytes = BytesFrom(dump, randomAddrs, 6);
	uint8_t *maskBytes = BytesFrom(mask, randomAddrs, 6);

	uint8_t recievedBytes[6];
	for (int i = 0; i < requiredCheckBytes; i++) {
		recievedBytes[i] = SendAddrAndWaitValue(randomAddrs[i]);
	}

	bool authorized = AccessAllowed(recievedBytes, dumpBytes, maskBytes, requiredCheckBytes);
	for (int i = requiredCheckBytes; i < maxCheckBytes; i++) {

		recievedBytes[i] = SendAddrAndWaitValue(randomAddrs[i]);
		authorized = AccessAllowed(recievedBytes, dumpBytes, maskBytes, i + 1);
		if (authorized) break;
	}

	if (authorized) {
		Serial.println(kAuthorized);
		Client.println(kAuthorized);
	}
	else {
		Serial.println(kAccessDenied);
		Client.println(kAccessDenied);
	}
}

inline bool SignedUp(String ID) {
	return SD.exists(ID + "/mask.txt");
}

void ClientHandle() {
	Serial.println("Start server. Waiting for client...");
	String request = WaitString();
	String authCommand = request.substring(0, 5);
	String ID = request.substring(5);

	Serial.println(authCommand);
	Serial.println(ID);
	if (authCommand == kAuthRequest) {
		SignedUp(ID) ? Auth(ID) : SignUp(ID);
	}
	else {
		Serial.print(authCommand);
		Serial.print(" not equal to ");
		Serial.println(kAuth);
	}
}

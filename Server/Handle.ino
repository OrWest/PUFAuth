// Протокол
#define kAuth F("Auth")
#define kMemSmall F("MemToSmall")
#define kMemSize F("MemSize")
#define kMemContext F("MemContext")
#define kNeedReboot F("NeedReboot")
#define kSignedUp F("SignedUp")
#define kAuthorized F("Authorized")
#define kAccessDenied F("AccessDenied")

// Конфигурация сервера
#define dumpMaxCount 2
#define byteMinCount 800
#define timeout 5000
#define requiredCheckBytes 4
#define maxMissingPercent 0.1

// Ожидание запроса/ответа по клиента
inline String WaitString() {
	while (Client.available() == 0);

	String string = Client.readStringUntil('\r');
	Client.read();
	Client.read();

	return string;
}

// Ожидание байта памяти от клиента
inline bool WaitByte() {
	long time = millis();
	while (Client.available() == 0) {
		if (millis() - time > timeout) {
			return false;
		}
	}
	return true;
}

// Очистка буфера входные данных по Serial
inline void ClearRX() {
	while (Client.read() >= 0);
}

// Путь папки с дампами нужного размера
String dirNameToDumpStack(String size) {
	int dumpCount = 0;
	String dirName = size + "/" + String(dumpCount);
	while (SD.exists(dirName) && SD.exists(dirName + "/mask")) {
		dumpCount++;
		dirName = size + "/" + String(dumpCount);
	} 

	if (!SD.exists(dirName)) {
		SD.mkdir(dirName);
	}

	return String(dumpCount);
}

// Адрес файла, который нужно использоваться 
// для сохранение снимка памяти клиента
String fileNameToSave(String size) {
	if (!SD.exists(size)) {
		SD.mkdir(size);
		Serial.println("Create new dir: " + size);
	}
	 
	String dirName = dirNameToDumpStack(size);
	String fileName;
	int dumpCount = 0;

	while (dumpCount < dumpMaxCount)
	{
		String peekName = size + "/" + dirName + "/dump" + String(dumpCount);
		if (!SD.exists(peekName)) {
			fileName = peekName;
			break;
		}
		dumpCount++;
	}

	Serial.println(fileName);
	return fileName;
}

// Чтение байтов памяти клиента и запись их в файл на SD-карте
void ReadBytesAndWriteToSDFile(int bytesCount) {
	String fileName = fileNameToSave(String(bytesCount));
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

inline void closeFiles(File files[], int count) {
	for (int i = 0; i < count; i++)
	{
		files[i].close();
	}
}

// Маска, которая показывает стабильные биты памяти.
uint8_t maskFromBytes(uint8_t bytes[]) {

	uint8_t orMask = bytes[0] ^ bytes[1];
	for (byte i = 1; i < dumpMaxCount - 1; i++)
	{
		orMask |= bytes[i] ^ bytes[i + 1];
	}

	return ~orMask;
}

// Заполнение файла маски на основании массива файлов со снимками памяти
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

// Удаление файлов со снимками клиента, кроме первого
void removeDumpsExceptFirst(String dirPath) {

	int dumpNum = 1;
	String dumpPath = dirPath + "/dump" + String(dumpNum);
	while (SD.exists(dumpPath)) 
	{
		SD.remove(dumpPath);
		dumpNum++;
		dumpPath = dirPath + "/dump" + String(dumpNum);
	}
}

// Если снимков хватает - генерируется маска устойчивости бит и возвращается true.
// Иначе просто false.
bool generateXorMask(String size) {
	File files[dumpMaxCount];

	String dirName = dirNameToDumpStack(size);
	int dumpCount = 0;
	while (dumpCount < dumpMaxCount)
	{
		String peekName = size + "/" + dirName + "/dump" + String(dumpCount);
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
	File maskFile = SD.open(size + "/" + dirName + "/mask", FILE_WRITE);
	fillMaskFile(files, maskFile);

	closeFiles(files, dumpMaxCount);
	maskFile.close();

	Serial.println("Remove all dumps except first.");
	removeDumpsExceptFirst(size + "/" + dirName);

	return true;
}

inline bool SignUpCompleted(String size) {
	return generateXorMask(size);
}

// Процедура регистрации нового клиентского устройства
void SignUp() {

	Serial.println(kMemSize);
	Client.println(kMemSize);

	while (Client.available() == 0);
	int size = Client.parseInt();

	Serial.println(size);
	if (size > byteMinCount) {
		ClearRX();

		Serial.println(kMemContext);
		Client.println(kMemContext);

		ReadBytesAndWriteToSDFile(size);

		String commandToResponse = SignUpCompleted(String(size)) ? kSignedUp : kNeedReboot;

		Serial.println(commandToResponse);
		Client.println(commandToResponse);
	}
	else {
		Serial.print(size);
		Serial.print(" less then ");
		Serial.println(byteMinCount);

		Serial.println(kMemSmall);
		Client.println(kMemSmall);
	}
}

// Ожидание байта от клиента
uint8_t WaitAndGetByte() {
	while (Client.available() == 0);

	uint8_t value = Client.read();
	Client.read();

	return value;
}

// Отправка адреса памяти клиенту и
// получение значения памяти по этому адресу от клиента
uint8_t SendAddrAndWaitValue(uint8_t addr) {
	ClearRX();

	Serial.println(addr, HEX);
	Client.write(addr);

	uint8_t receivedValue = WaitAndGetByte();
	Serial.println(receivedValue, HEX);
	return receivedValue;
}

// Возвращает значения памяти из сохраненного снимка памяти по адресам
uint8_t *BytesFrom(File file, uint8_t addrs[], int count) {
	uint8_t *bytes = (uint8_t *)calloc(sizeof(uint8_t), count);

	for (int i = 0; i < count; i++) {
		file.seek(addrs[i]);
		bytes[i] = file.read();
	}

	return bytes;
}

// Проверяет полученные байты от клиента с сохраненными в файлах,
// и возвращает флаг совпадение с определенной погрешностью.
bool AccessAllowed(uint8_t receivedBytes[], uint8_t dumpBytes[], uint8_t maskBytes[], int count) {
	Serial.println("Start check bytes.");

	int chechBytesCount = 0;
	int missedBytesCount = 0;
	for (int i = 0; i < count; i++) {
		//Serial.println(receivedBytes[i], BIN);
		//Serial.println(dumpBytes[i], BIN);
		//Serial.println(maskBytes[i], BIN);

		int value = receivedBytes[i] ^ dumpBytes[i];
		Serial.print("XOR: ");
		Serial.println(value, BIN);
		int mask = maskBytes[i];

		for (int j = 0; j < 8; j++) {
			if (mask & 0x01 == 1) {
				//Serial.print("Mask and value shifting(Mask has 1): i=");
				//Serial.print(i);
				//Serial.print(" mask=");
				//Serial.println(mask, BIN);
				chechBytesCount++;
				//Serial.print("Missed += ");
				//Serial.println(value & 0x01);
				missedBytesCount += value & 0x01;
			}
			mask >>= 1;
			value >>= 1;
		}
	}

	//Serial.print("Bytes count=");
	//Serial.print(chechBytesCount);
	//Serial.print(" Missed count=");
	//Serial.println(missedBytesCount);

	double missedPercent = missedBytesCount / (double)chechBytesCount;
	Serial.print("Missed percent=");
	Serial.println(missedPercent);

	return missedPercent < maxMissingPercent;
}

// Проверяет полученные байты от клиента с сохраненными в файлах, и либо разрешает доступ, либо нет.
bool Auth(uint8_t *addrs, uint8_t *receivedBytes, String size) {
	bool authorized = false;

	if (!SD.exists(size)) {
		return false;
	}

	File sizeDir = SD.open(size);
	File dumpDir = sizeDir.openNextFile();
	while (dumpDir != NULL)
	{
		Serial.print("Search in dumpDir: ");
		Serial.println(dumpDir.name());
		uint8_t *dumpBytes = BytesFrom(SD.open(size + "/" + dumpDir.name() + "/dump0"), addrs, requiredCheckBytes);
		uint8_t *maskBytes = BytesFrom(SD.open(size + "/" + dumpDir.name() + "/mask" ), addrs, requiredCheckBytes);
		authorized = AccessAllowed(receivedBytes, dumpBytes, maskBytes, requiredCheckBytes);

		if (authorized) {
			break;
		}

		dumpDir.close();
		dumpDir = sizeDir.openNextFile();
	}
	
	Serial.println("Rewind directory.");
	sizeDir.rewindDirectory();
	sizeDir.close();
	return authorized;
}

// Процедура идентификации клиентского устройства
void Auth() {
	Serial.println(kMemSize);
	Client.println(kMemSize);

	while (Client.available() == 0);
	int size = Client.parseInt();

	uint8_t randomAddrs[requiredCheckBytes];
	for (int i = 0; i < requiredCheckBytes; i++) {
		uint8_t randomAddr = random(size);
		randomAddrs[i] = randomAddr;
	}

	uint8_t receivedBytes[requiredCheckBytes];
	for (int i = 0; i < requiredCheckBytes; i++) {
		receivedBytes[i] = SendAddrAndWaitValue(randomAddrs[i]);
	}

	if (Auth(randomAddrs, receivedBytes, String(size))) {
		Serial.println(kAuthorized);
		Client.println(kAuthorized);
	}
	else {
		Serial.println(kAccessDenied);
		Client.println(kAccessDenied);
	}
}

// Ожидание подключения клиента и запуск процедуры регистрации при корректном запросе.
void ClientSignUp() {
	Serial.println("Start server(in SignUp mode). Waiting for client...");
	String request = WaitString();

	Serial.println(request);
	if (request == kAuth) {
		SignUp();
	}
	else {
		Serial.print(request);
		Serial.print(" not equal to ");
		Serial.println(kAuth);
	}
}

// Ожидание подключения клиента и запуск процедуры идентификации при корректном запросе.
void ClientAuth() {
	Serial.println("Start server. Waiting for client...");
	String request = WaitString();

	Serial.println(request);
	if (request == kAuth) {
		Auth();
	}
	else {
		Serial.print(request);
		Serial.print(" not equal to ");
		Serial.println(kAuth);
	}
}

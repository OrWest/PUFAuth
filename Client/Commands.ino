#include <avr\io.h>

extern char *const __brkval;
extern char *const __data_start;

#define kAuth F("Auth")
#define kMemSmall F("MemToSmall")
#define kMemSize F("MemSize")
#define kMemContext F("MemContext")
#define kNeedReboot F("NeedReboot")
#define kSignedUp F("SignedUp")
#define kAuthorized F("Authorized")
#define kAccessDenied F("AccessDenied")

#define delayBetweenBytesSending 15

uint8_t *startAddr;
uint8_t *endAddr;

inline uint8_t* freeMemStart()
{
	if (__brkval == 0) {
		return (uint8_t *)__malloc_heap_start;
	}
	else {
		return (uint8_t *)__brkval;
	}
}

void SaveDumpAddresses() {
	startAddr = freeMemStart() + 64;
	endAddr = (uint8_t *)(SP) - 192;
}

int memSize() {
	int size = endAddr - startAddr;

	char buffer[35];
	snprintf(buffer, 35, "p=%p end=%p size=%X", startAddr, endAddr, size);
	Serial.println(buffer);
	return size;
}

void printMemContentToSerial() {
	uint8_t *p = startAddr;

	do
	{
		Server.write(*p);
		p++;
		delay(delayBetweenBytesSending);
	} while (p < endAddr);
}

uint8_t byteByAddr(uint8_t addr) {
	uint8_t *p = startAddr;
	p += addr;

	Serial.print("Offset: ");
	Serial.print(addr, HEX);
	Serial.print(" Addr: ");
	char buffer[6];
	snprintf(buffer, 6, "%p", p);
	Serial.print(buffer);
	Serial.print(" Value: ");
	Serial.println(*p, HEX);

	return *p;
}

inline String WaitString() {
	while (Server.available() == 0);

	String string = Server.readStringUntil('\r');
	Server.read();

	return string;
}

void WaitAddrsAndSendValues() {
	while (Server.available() < 4) // byte \r \n
	{
		while (Server.available() == 0);
		delay(10);

		if (Server.available() > 3) {
			Serial.print(F("Available: "));
			Serial.println(Server.available());
			break;
		}

		uint8_t addr = Server.read();
		Server.read();
		Server.read();

		Serial.print(F("Auth: Received: "));
		Serial.println(addr, HEX);

		Server.write(byteByAddr(addr));
	}
}

void TryAuth() {
	Serial.println(String(kAuth));
	Server.println(String(kAuth));

	String response = WaitString();
	Serial.println(response);

	if (response == kMemSize) {
		Serial.println(F("Send memSize."));
		Server.println(memSize());

		WaitAddrsAndSendValues();

		response = WaitString();
		Serial.print(F("Response: "));
		Serial.println(response);

		if (response == kMemContext) {
			Serial.println(F("Send memContext."));
			printMemContentToSerial();

			response = WaitString();
			Serial.println(response);
			if (response == kNeedReboot) {
				while (true)
				{
					digitalWrite(13, HIGH);
					delay(300);
					digitalWrite(13, LOW);
					delay(300);
				}
			}
			else {
				digitalWrite(13, HIGH);
				while (true);
			}
		}
		else if (response == kAuthorized) {
			Serial.println(F("Device was authorized."));
			while (true)
			{
				digitalWrite(13, HIGH);
				delay(1000);
				digitalWrite(13, LOW);
				delay(1000);
			}
		}
		else if (response == kAccessDenied) {
			Serial.println(F("Access denied."));
			while (true)
			{
				digitalWrite(13, HIGH);
				delay(100);
				digitalWrite(13, LOW);
				delay(1000);
			}
		}
		else {
			Serial.print(F("Received unknown command. "));
			Serial.println(response.length());
		}
	}
	else {
		Serial.print(F("Received unknown command. "));
		Serial.println(response.length());
	}
}

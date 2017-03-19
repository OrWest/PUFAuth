#include <avr\io.h>

extern char *const __brkval;
extern char *const __data_start;

#define kAuth F("Auth@")
#define kMemSize F("MemSize")
#define kMemContext F("MemContext")
#define kNeedReboot F("NeedReboot")
#define kSignedUp F("SignedUp")

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
	startAddr = freeMemStart() + 100;
	endAddr = (uint8_t *)(SP) - 100;
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

	char buffer[20];
	snprintf(buffer, 20, "p=%p end=%p", p, endAddr);
	Serial.println(buffer);
	do
	{
		Server.write(*p);
		p++;
		delay(10);
	} while (p < endAddr);
}

inline String WaitString() {
	while (Server.available() == 0);

	String string = Server.readStringUntil('\r');
	Server.read();
	Server.read();

	return string;
}

void SignUp() {
  Serial.println(String(kAuth) + ID);
  Server.println(String(kAuth) + ID);

  String response = WaitString();
  Serial.println(response);
  if (response == kMemSize) {
    Serial.println(F("Send memSize."));
    Server.println(memSize());

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
			  delay(500);
			  digitalWrite(13, LOW);
			  delay(500);
		  }
	  }
	  else {
		  digitalWrite(13, HIGH);
		  while (true);
	  }
	}
  }
}

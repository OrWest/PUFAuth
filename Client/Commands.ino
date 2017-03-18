#include <avr\io.h>

extern char *const __brkval;
extern char *const __data_start;

#define kAuth F("Auth")
#define kMemSize F("MemSize")
#define kMemContext F("MemContext")
#define kNeedReboot F("NeedReboot")
#define kSignedUp F("SignedUp")

uint8_t * freeMemStart()
{
	if (__brkval == 0) {
		return (uint8_t *)__malloc_heap_start;
	}
	else {
		return (uint8_t *)__brkval;
	}
}

uint8_t * freeMemEnd() {
	return (uint8_t *)(SP);
}

int memSize() {
	uint8_t *startAddr = freeMemStart();
	uint8_t *endAddr = freeMemEnd();
	int size = endAddr - startAddr;

	char buffer[50];
	snprintf(buffer, 50, "p=%p end=%p size=%X", startAddr, endAddr, size);
	Serial.println(buffer);
	return size;
}

void printMemContentToSerial() {
	uint8_t *p = freeMemStart();
	uint8_t *end = freeMemEnd();

	do
	{
		Serial1.print(*p);
		p++;
	} while (p < end);

}

inline String WaitString() {
	while (Serial1.available() == 0);

	return Serial1.readStringUntil('\r');
}

void SignUp() {
  Serial.println(kAuth);
  Serial1.println(kAuth);

  String response = WaitString();
  Serial.println(response);
  if (response == kMemSize) {
    Serial.println(F("Send memSize."));
    Serial1.println(memSize());

    response = WaitString();
	Serial.println(response);
    if (response == kMemContext) {
      Serial.println(F("Send memContext."));
	  printMemContentToSerial();

      response = WaitString();
      Serial.println(response);
	}
  }
}

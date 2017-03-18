
#define kAuth L("Auth")
#define kMemSize L("MemSize")
#define kMemContext L("MemContext")
#define kNeedReboot L("NeedReboot")
#define kSignedUp L("SignedUp")

uint16 memSize() {
  uint16 startAddr;
  uint16 endAddr;
  uint16 size = endAddr - startAddr;

  Serial.print(L("Start: "));
  Serial.println(startAddr, HEX);
  Serial.print(L("End: "));
  Serial.println(endAddr, HEX);
  Serial.print(L("Size: "));
  Serial.println(size, HEX);
  return size;
}

string memContent() {

}

void SignUp() {
  Serial.println(kAuth);
  Serial1.println(kAuth);
  delay(100);
  string response = Serial1.read();
  if (response == kMemSize) {
    Serial.println(L("Send memSize."));
    Serial1.println(memSize());
    delay(100);

    response = Serial1.read();
    if (response == kMemContext) {
      Serial.println(L("Send memContext."));
      Serial1.println(memContent());
      delay(500);

      response = Serial1.read();
      Serial.println(response);
    }
  }

}

#include <SPI.h>
#include <Adafruit_PN532.h>

#define PN532_SS 5

Adafruit_PN532 nfc(PN532_SS);

void setup() {
  Serial.begin(115200);
  delay(100);

  SPI.begin(18, 19, 23); // correct ESP32 SPI bus

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();

  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    while (1);
  }

  Serial.println("PN532 detected!");
}

void loop(void) {
  uint8_t uid[7];
  uint8_t uidLength;

  bool success = nfc.readPassiveTargetID(
    PN532_MIFARE_ISO14443A,
    uid,
    &uidLength
  );

  if (success) {
    Serial.println("Card detected!");

    Serial.print("UID: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println();
    uint8_t data[32];

  // NTAG starts readable data at page 4
  for (uint8_t page = 4; page < 10; page++) {
    if (nfc.ntag2xx_ReadPage(page, data)) {
      Serial.print("Page ");
      Serial.print(page);
      Serial.print(": ");

      for (int i = 0; i < 4; i++) {
        Serial.print((char)data[i]); // try print as text
      }

      Serial.println();
    } else {
      Serial.println("Read failed");
    }
  }
    delay(1000); // prevents spam + re-trigger issues
  }
}
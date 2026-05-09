#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ   -1
#define PN532_RESET -1

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET, &Wire);


// DO_CLEAR: when true, clear the tag.
// DO_WRITE_TEST: when true, write to page 10 and page 24.
bool DO_CLEAR = false;
bool DO_WRITE_TEST = false;

void printUID(const uint8_t *uid, uint8_t uidLength) {
  for (uint8_t i = 0; i < uidLength; i++) {
    if (uid[i] < 0x10) Serial.print("0");
    Serial.print(uid[i], HEX);

    if (i + 1 < uidLength) {
      Serial.print(" ");
    }
  }
}


bool readPageSpan(uint8_t startPage, uint8_t endPage) {
  uint8_t buf[4];

  for (uint8_t page = startPage; page <= endPage; page++) {
    if (page < 0x10) Serial.print("0");
    Serial.print(page, HEX);
    Serial.print(": ");

    bool ok = nfc.ntag2xx_ReadPage(page, buf);

    if (ok) {
      for (uint8_t i = 0; i < 4; i++) {
        if (buf[i] < 0x10) Serial.print("0");
        Serial.print(buf[i], HEX);

        if (i < 3) {
          Serial.print(" ");
        }
      }
      Serial.println();
    } else {
      Serial.println("READ FAIL");
      return false;
    }
  }

  return true;
}


void calculateBCC(const uint8_t *uid, uint8_t uidLength) {
  if (uidLength != 7) {
    Serial.println("no UID");
    return;
  }

  uint8_t bcc0 = 0x88 ^ uid[0] ^ uid[1] ^ uid[2];
  uint8_t bcc1 = uid[3] ^ uid[4] ^ uid[5] ^ uid[6];

  Serial.print("BCC0: ");
  if (bcc0 < 0x10) Serial.print("0");
  Serial.println(bcc0, HEX);

  Serial.print("BCC1: ");
  if (bcc1 < 0x10) Serial.print("0");
  Serial.println(bcc1, HEX);
}


void clearNTAG213UserArea() {
  uint8_t blank[4] = {0x00, 0x00, 0x00, 0x00};

  Serial.println();
  Serial.println("Clearing pages 04 to 27");

  for (uint8_t page = 0x04; page <= 0x27; page++) {
    bool ok = nfc.ntag2xx_WritePage(page, blank);

    if (page < 0x10) Serial.print("0");
    Serial.print(page, HEX);
    Serial.print(": ");

    if (ok) {
      Serial.println("CLEARED");
    } else {
      Serial.println("WRITE FAIL");
    }
  }
}


void writeTestPages() {
  uint8_t page10Data[4] = {0x11, 0x22, 0x33, 0x44};
  uint8_t page24Data[4] = {0xAA, 0xBB, 0xCC, 0xDD};

  Serial.println();
  Serial.println("Writing test values");

  bool ok10 = nfc.ntag2xx_WritePage(10, page10Data);
  Serial.print("Page 10 decimal / 0A hex: ");
  if (ok10) {
    Serial.println("WRITE OK");
  } else {
    Serial.println("WRITE FAIL");
  }

  bool ok24 = nfc.ntag2xx_WritePage(24, page24Data);
  Serial.print("Page 24 decimal / 18 hex: ");
  if (ok24) {
    Serial.println("WRITE OK");
  } else {
    Serial.println("WRITE FAIL");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin();

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();

  if (!versiondata) {
    Serial.println("ERROR: PN532 not found. Check I2C mode and wiring.");
    while (1) delay(10);
  }

  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);

  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print(".");
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  nfc.SAMConfig();

  Serial.println("Hold tag near reader...");
}

void loop() {
  uint8_t uid[7];
  uint8_t uidLength = 0;

  bool success = nfc.readPassiveTargetID(
    PN532_MIFARE_ISO14443A,
    uid,
    &uidLength,
    100
  );

  if (success) {
    Serial.println();
    Serial.print("UID length: ");
    Serial.print(uidLength);
    Serial.print(" bytes");
    Serial.println();
    Serial.print("UID: ");
    printUID(uid, uidLength);
    Serial.println();

    calculateBCC(uid, uidLength);

    Serial.println();
    Serial.println("First 4 pages:");
    readPageSpan(0x00, 0x03);

    Serial.println();
    Serial.println("All NTAG213 pages:");
    readPageSpan(0x00, 0x2C);

    if (DO_CLEAR) {
      clearNTAG213UserArea();

      Serial.println();
      Serial.println("After clearing user pages:");
      readPageSpan(0x04, 0x27);
    }

    if (DO_WRITE_TEST) {
      writeTestPages();

      Serial.println();
      Serial.println("After writing to page 10 and page 24:");
      readPageSpan(0x04, 0x27);
    }

    Serial.println();
    Serial.println("Done. Remove tag or reset to run again.");
    delay(3000);
  }
}
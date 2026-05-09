#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ   -1
#define PN532_RESET -1

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET, &Wire);


const uint8_t N_TRIALS = 30;
const uint32_t POLL_TIMEOUT_MS = 250;
const uint32_t INTER_TRIAL_DELAY_MS = 80;

void printUID(const uint8_t *uid, uint8_t uidLength) {
  for (uint8_t i = 0; i < uidLength; i++) {
    if (uid[i] < 0x10) Serial.print("0");
    Serial.print(uid[i], HEX);
    if (i + 1 < uidLength) Serial.print(" ");
  }
}

bool onePoll(uint32_t &ttf_ms) {
  uint8_t uid[7];
  uint8_t uidLength = 0;

  uint32_t t0 = millis();
  bool ok = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, POLL_TIMEOUT_MS);
  ttf_ms = millis() - t0;

  if (ok) {
    Serial.print("UID: ");
    printUID(uid, uidLength);
    Serial.print(" | ");
  }

  return ok;
}

void runCouplingStudy() {
  uint8_t successCount = 0;
  uint32_t totalSuccessTime = 0;

  Serial.println();
  Serial.println("Starting coupling study...");
  Serial.print("Number of trials: ");
  Serial.println(N_TRIALS);

  for (uint8_t i = 0; i < N_TRIALS; i++) {
    uint32_t ttf_ms = 0;
    bool success = onePoll(ttf_ms);

    Serial.print("Trial ");
    Serial.print(i + 1);
    Serial.print(": ");

    if (success) {
      successCount++;
      totalSuccessTime += ttf_ms;

      Serial.print("SUCCESS");
      Serial.print(" | time = ");
      Serial.print(ttf_ms);
      Serial.println(" ms");
    } else {
      Serial.print("FAIL");
      Serial.print(" | time = ");
      Serial.print(ttf_ms);
      Serial.println(" ms");
    }

    delay(INTER_TRIAL_DELAY_MS);
  }

  float successRate = (float)successCount / N_TRIALS * 100.0;

  Serial.println();
  Serial.println("Coupling study results:");
  Serial.print("Successful calls: ");
  Serial.print(successCount);
  Serial.print(" / ");
  Serial.println(N_TRIALS);

  Serial.print("Success rate: ");
  Serial.print(successRate, 1);
  Serial.println("%");

  if (successCount > 0) {
    float avgSuccessTime = (float)totalSuccessTime / successCount;
    Serial.print("Average successful read time: ");
    Serial.print(avgSuccessTime, 1);
    Serial.println(" ms");
  } else {
    Serial.println("Average successful read time: N/A, no successful reads");
  }

  Serial.println();
  Serial.println("Move tag to new distance/orientation, then reset board to test again.");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin();

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();

  if (!versiondata) {
    Serial.println("ERROR: PN532 not found. Check I2C mode + wiring.");
    while (1) delay(10);
  }

  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);

  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print(".");
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  nfc.SAMConfig();

  Serial.println("Ready for coupling study.");
  Serial.println("Hold the tag in one fixed position/orientation.");
  delay(2000);

  runCouplingStudy();
}

void loop() {

}
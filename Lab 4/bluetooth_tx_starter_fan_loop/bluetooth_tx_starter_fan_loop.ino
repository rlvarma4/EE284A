#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHAR_UUID    "12345678-1234-1234-1234-1234567890ac"

Adafruit_INA219 ina219;
BLECharacteristic *characteristic;

bool connected = false;
bool restartAdvertising = false;

// Fan control
const int FAN_PIN = 13;   

// Battery safety/control thresholds
const float BATTERY_CRITICAL_V = 3.20;  


int fanDutyPercent = 0;

bool isCharging(float current_mA) {
  return current_mA < 0;
}

void setFanDuty(int dutyPercent) {
  dutyPercent = constrain(dutyPercent, 0, 100);

  int pwmValue = map(dutyPercent, 0, 100, 0, 255);
  analogWrite(FAN_PIN, pwmValue);

  fanDutyPercent = dutyPercent;
}

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) {
    connected = true;
    Serial.println("Client connected");
  }

  void onDisconnect(BLEServer*) {
    connected = false;
    restartAdvertising = true;
    Serial.println("Client disconnected");
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
      delay(10);
    }
  }

  Serial.println("INA219 initialized");

  pinMode(FAN_PIN, OUTPUT);
  setFanDuty(0);

  BLEDevice::init("Riya_ESP32_INA219");
  BLEDevice::setPower(ESP_PWR_LVL_P9);

  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);

  characteristic = service->createCharacteristic(
    CHAR_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  characteristic->addDescriptor(new BLE2902());

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);

  BLEDevice::startAdvertising();

  Serial.println("BLE advertising started");
}

void loop() {
  if (restartAdvertising) {
    delay(200);
    BLEDevice::startAdvertising();
    Serial.println("Advertising restarted");
    restartAdvertising = false;
  }

  float shunt_mV = ina219.getShuntVoltage_mV();
  float bus_V = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();

  float battery_V = bus_V + (shunt_mV / 1000.0);
  float power_mW = battery_V * current_mA;

  String controlState;

  // ------------------------------------------------------------
  //CONTROL STRATEGY
  // ------------------------------------------------------------

  if (battery_V <= BATTERY_CRITICAL_V) {
    // Protect battery first
    setFanDuty(0);
    controlState = "PROTECT";
  }
  else if (isCharging(current_mA)) {
    // If solar panel is charging the battery, use more fan power
    setFanDuty(100);
    controlState = "CHARGING_FULL_FAN";
  }
  else {
    // Above critical, but still low: turn fan off to recover
    setFanDuty(0);
    controlState = "LOW_BATT_OFF";
  }

  char message[180];

  snprintf(
    message,
    sizeof(message),
    "Bus: %.3f V  Shunt: %.3f mV  Load: %.3f V  I: %.3f mA  P: %.3f mW  Fan: %d%%  State: %s",
    bus_V,
    shunt_mV,
    battery_V,
    current_mA,
    power_mW,
    fanDutyPercent,
    controlState.c_str()
  );

  Serial.println(message);

  if (connected) {
    characteristic->setValue(message);
    characteristic->notify();
  }

  delay(20);
}
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
const int FAN_PIN = 13;             // Connect this pin to MOSFET gate
const int FAN_DUTY_PERCENT = 0;    // Use 25 for 25%, change to 50 for 50%

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

  // Set fan PWM duty cycle
  pinMode(FAN_PIN, OUTPUT);

  int pwm_value = map(FAN_DUTY_PERCENT, 0, 100, 0, 255);
  analogWrite(FAN_PIN, pwm_value);

  Serial.print("Fan duty cycle set to ");
  Serial.print(FAN_DUTY_PERCENT);
  Serial.println("%");

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

  if (connected) {
    float shunt_mV = ina219.getShuntVoltage_mV();
    float bus_V = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();

    float battery_V = bus_V + (shunt_mV / 1000.0);
    float power_mW = battery_V * current_mA;

    char message[150];

    snprintf(
      message,
      sizeof(message),
      "Bus: %.3f V  Shunt: %.3f mV  Load: %.3f V  I: %.3f mA  P: %.3f mW  Fan: %d%%",
      bus_V,
      shunt_mV,
      battery_V,
      current_mA,
      power_mW,
      FAN_DUTY_PERCENT
    );

    Serial.println(message);

    characteristic->setValue(message);
    characteristic->notify();

    delay(20);
  }
}
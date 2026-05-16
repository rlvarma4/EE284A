#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

void setup(void) 
{
  Serial.begin(115200);

  while (!Serial) {
    delay(1);
  }

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
      delay(10);
    }
  }

  Serial.println("Measuring battery voltage and current with INA219...");
}

void loop(void) 
{
  float shunt_mV = ina219.getShuntVoltage_mV();
  float bus_V = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();

  // Battery/load voltage includes the shunt voltage drop
  float battery_V = bus_V + (shunt_mV / 1000.0);

  Serial.print("Bus: ");
  Serial.print(bus_V, 3);
  Serial.print(" V  ");

  Serial.print("Shunt: ");
  Serial.print(shunt_mV, 3);
  Serial.print(" mV  ");

  Serial.print("Load: ");
  Serial.print(battery_V, 3);
  Serial.print(" V  ");

  Serial.print("I: ");
  Serial.print(current_mA, 3);
  Serial.println(" mA");

  delay(20);
}
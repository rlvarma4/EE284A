#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define SEALEVELPRESSURE_HPA (1013.25)
const int TMP_PIN = A2;      
const int LED_PIN = A0;      
const int BUTTON_PIN = BUTTON;  
const int ADC_MAX = 4095;
const float VREF = 2.1;  
const int N = 10;
float bmeBuffer[N];
float tmpBuffer[N];
int bufIndex = 0;
int samplesFilled = 0;
const float THRESHOLD = 3.0;  
bool systemStarted = false;
Adafruit_BME680 bme(&Wire);

float averageArray(float arr[], int count) {
  float sum = 0.0;
  for (int i = 0; i < count; i++) {
    sum += arr[i];
  }
  return sum / count;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT);
  analogReadResolution(12);

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);
  for (int i = 0; i < N; i++) {
    bmeBuffer[i] = 0.0;
    tmpBuffer[i] = 0.0;
  }
  Serial.println("Press button to start.");
}

void loop() {
  if (!systemStarted) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      systemStarted = true;
      Serial.println("Starting measurements...");
      delay(300); 
    } else {
      return;
    }
  }

  if (!bme.performReading()) {
    Serial.println("Failed to perform BME680 reading");
    delay(500);
    return;
  }
  float bmeTemp = bme.temperature;
  analogSetPinAttenuation(TMP_PIN, ADC_6db);
  delay(50);
  int value = analogRead(TMP_PIN);
  float voltage = (value * VREF) / ADC_MAX;
  float tmpTemp = (voltage - 0.5) / 0.01;
  bmeBuffer[bufIndex] = bmeTemp;
  tmpBuffer[bufIndex] = tmpTemp;

  bufIndex = (bufIndex + 1) % N;
  if (samplesFilled < N) {
    samplesFilled++;
  }

  float bmeAvg = averageArray(bmeBuffer, samplesFilled);
  float tmpAvg = averageArray(tmpBuffer, samplesFilled);

  float diff = abs(bmeAvg - tmpAvg);

  if (diff > THRESHOLD) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  Serial.print("BME Avg: ");
  Serial.print(bmeAvg, 2);
  Serial.print(" C, TMP Avg: ");
  Serial.print(tmpAvg, 2);
  Serial.print(" C, |Diff|: ");
  Serial.print(diff, 2);
  Serial.print(" C, LED: ");
  Serial.println(diff > THRESHOLD ? "ON" : "OFF");

  delay(500);
}
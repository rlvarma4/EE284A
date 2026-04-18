#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <math.h>
Adafruit_BME680 bme;
const int SAMP = 5000;
const int N = 500;
float buffer[N];
float sortedBuffer[N];
int bufferIndex = 0;
int countInWindow = 0;
float minTemp = 10000.0;
float maxTemp = -10000.0;
double sum = 0.0;
double sumSq = 0.0;
int count = 0;
bool done = false;

float computeMedian(float arr[], int size) {
  for (int i = 0; i < size; i++) {
    sortedBuffer[i] = arr[i];
  }
  for (int i = 0; i < size - 1; i++) {
    for (int j = i + 1; j < size; j++) {
      if (sortedBuffer[j] < sortedBuffer[i]) {
        float temp = sortedBuffer[i];
        sortedBuffer[i] = sortedBuffer[j];
        sortedBuffer[j] = temp;
      }
    }
  }
  if (size % 2 == 1) {
    return sortedBuffer[size / 2];
  } else {
    return (sortedBuffer[size / 2 - 1] + sortedBuffer[size / 2]) / 2.0;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Wire.begin();
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  bme.setTemperatureOversampling(BME680_OS_1X);
  bme.setHumidityOversampling(BME680_OS_1X);
  bme.setPressureOversampling(BME680_OS_1X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_0);
  for (int i = 0; i < N; i++) {
    buffer[i] = 0.0;
    sortedBuffer[i] = 0.0;
  }
}

void loop() {
  if (done) return;
  if (!bme.performReading()) {
    Serial.println("Failed to read from BME680");
    return;
  }
  float temp = bme.temperature;
  buffer[bufferIndex] = temp;
  bufferIndex = (bufferIndex + 1) % N;
  if (countInWindow < N) {
    countInWindow++;
  }
  float updatedTemp = computeMedian(buffer, countInWindow);
  Serial.print(count + 1);
  Serial.print(", ");
  Serial.print(temp, 4);
  Serial.print(", ");
  Serial.println(updatedTemp, 4);
  if (updatedTemp < minTemp) minTemp = updatedTemp;
  if (updatedTemp > maxTemp) maxTemp = updatedTemp;
  sum += updatedTemp;
  sumSq += (double)updatedTemp * updatedTemp;
  count++;
  if (count >= SAMP) {
    double mean = sum / SAMP;
    double variance = (sumSq / SAMP) - (mean * mean);
    if (variance < 0) variance = 0;
    double stddev = sqrt(variance);
    Serial.print("Min temp: ");
    Serial.println(minTemp, 4);
    Serial.print("Max temp: ");
    Serial.println(maxTemp, 4);
    Serial.print("Mean temp: ");
    Serial.println(mean, 4);
    Serial.print("Std dev: ");
    Serial.println(stddev, 4);

    done = true;
  }

  delay(10);
}
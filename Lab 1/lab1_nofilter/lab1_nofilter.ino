#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <math.h>

Adafruit_BME680 bme;  // I2C

const int NUM_SAMPLES = 5000;

float minTemp = 10000.0;
float maxTemp = -10000.0;
double sum = 0.0;
double sumSq = 0.0;

int count = 0;
bool done = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin();

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // No internal filtering
  bme.setTemperatureOversampling(BME680_OS_1X);
  bme.setHumidityOversampling(BME680_OS_1X);
  bme.setPressureOversampling(BME680_OS_1X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_0);

  Serial.println("Starting 5000-sample BME680 run with NO filtering...");
  Serial.println("Sample #, Temperature (C)");
}

void loop() {
  if (done) return;

  if (!bme.performReading()) {
    Serial.println("Failed to read from BME680");
    return;
  }

  float temp = bme.temperature;

  // Print each sample
  Serial.print(count + 1);
  Serial.print(", ");
  Serial.println(temp, 4);

  if (temp < minTemp) minTemp = temp;
  if (temp > maxTemp) maxTemp = temp;

  sum += temp;
  sumSq += (double)temp * temp;
  count++;

  if (count >= NUM_SAMPLES) {
    double mean = sum / NUM_SAMPLES;
    double variance = (sumSq / NUM_SAMPLES) - (mean * mean);
    if (variance < 0) variance = 0;
    double stddev = sqrt(variance);

    Serial.println("\n===== BME680 RESULTS: NO FILTERING =====");
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
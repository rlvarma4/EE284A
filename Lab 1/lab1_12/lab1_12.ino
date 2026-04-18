const int ADC_PIN = A2;
const int ADC_MAX = 4095;
const float VREF = 2.1;
const int SAMPLES = 5000;

int count = 0;
float minTemp = 10000.0;
float maxTemp = -10000.0;
double sum = 0.0;
double sumSq = 0.0;
bool done = false;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(460800);
    analogReadResolution(12);
}
 
void loop() {
  // put your main code here, to run repeatedly:
    if (done) {
    return;
  }

  analogSetPinAttenuation(ADC_PIN, ADC_6db);
  delay(50);

  int value = analogRead(ADC_PIN);
  float voltage = (value * VREF) / ADC_MAX;
  float temp = (voltage - 0.5) / 0.01;

  if (temp < minTemp) minTemp = temp;
  if (temp > maxTemp) maxTemp = temp;

  sum += temp;
  sumSq += temp * temp;
  count++;

  Serial.print("Sample ");
  Serial.print(count);
  Serial.print(": ");
  Serial.print(temp, 4);
  Serial.println(" C");

  if (count >= SAMPLES) {
    double mean = sum / SAMPLES;
    double variance = (sumSq / SAMPLES) - (mean * mean);
    if (variance < 0) variance = 0;
    double stddev = sqrt(variance);

    Serial.println("Results");
    Serial.print("Samples: ");
    Serial.println(SAMPLES);
    Serial.print("Min temperature: ");
    Serial.print(minTemp, 4);
    Serial.println(" C");
    Serial.print("Max temperature: ");
    Serial.print(maxTemp, 4);
    Serial.println(" C");
    Serial.print("Mean temperature: ");
    Serial.print(mean, 4);
    Serial.println(" C");
    Serial.print("Standard deviation: ");
    Serial.print(stddev, 4);
    Serial.println(" C");

    done = true;
  }

  delay(10);

}

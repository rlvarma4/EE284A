const int ADC_PIN = A2;
const int ADC_MAX = 4095;
const float VREF = 2.1;
const float THRESHOLD = 23.00;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(460800);
    analogReadResolution(12);
}

void loop() {
  // put your main code here, to run repeatedly:
  analogSetPinAttenuation(ADC_PIN, ADC_6db); 
  delay(50);

  int value = analogRead(ADC_PIN);
  float voltage = (value * VREF) / ADC_MAX;
  float temp = (voltage - 0.5) / 0.01;

  Serial.print("The raw ADC value is ");
  Serial.print(value);
  Serial.print(", which converts to ");
  Serial.print(voltage, 3);
  Serial.print(" V. The temperature is ");
  if (temp >= THRESHOLD) {
    Serial.print("above ");
  } else {
    Serial.print("below ");
  }
  Serial.print("23C, in fact it is ");
  Serial.print(temp, 2);
  Serial.println(" C.");

  delay(500);

}

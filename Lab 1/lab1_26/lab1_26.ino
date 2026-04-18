const int beamPin = 27;
const int servoPin = 13;

const int pwmResolution = 16;
const int pwmFreq = 50;

uint32_t minPulse = 1550;
uint32_t midPulse = 1650;
uint32_t maxPulse = 1750;

uint32_t setPulse = maxPulse;

int lastState = HIGH;
unsigned long edgeTime = 0;
float rpm = 0.0;

uint32_t dutyFromUs(uint32_t pulseUs) {
  const uint32_t maxDuty = (1UL << pwmResolution) - 1;
  return (pulseUs * maxDuty) / 20000UL; 
}

void setup() {
  Serial.begin(115200);

  pinMode(beamPin, INPUT_PULLUP);

  ledcAttach(servoPin, pwmFreq, pwmResolution);
  delay(100);

  ledcWrite(servoPin, dutyFromUs(setPulse));

}

void loop() {
  ledcWrite(servoPin, dutyFromUs(setPulse));

  int state = digitalRead(beamPin);
  unsigned long now = micros();

  if (lastState == HIGH && state == LOW) {
    if (edgeTime != 0) {
      unsigned long period = now - edgeTime; 

      if (period > 0) {
        rpm = 60000000.0 / period; 
      }
      Serial.print("Period (us): ");
      Serial.print(period);
      Serial.print("   RPM: ");
      Serial.println(rpm);
    }

    edgeTime = now;
  }

  lastState = state;
}
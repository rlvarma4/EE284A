const int beamPin = 27;
const int servoPin = 13;

const int pwmResolution = 16;
const int pwmFreq = 50;

uint32_t minPulse = 1550;
uint32_t midPulse = 1650;
uint32_t maxPulse = 1750;

float rpmTarget = 60.0;

float pulseStart = 1650.0;

int lastState = HIGH;
unsigned long edgeTime = 0;
float rpm = 0.0;

float Kp = 0.5;

uint32_t dutyFromUs(uint32_t pulseUs) {
  const uint32_t maxDuty = (1UL << pwmResolution) - 1;
  return (pulseUs * maxDuty) / 20000UL;   
}

void setup() {
  Serial.begin(115200);
  pinMode(beamPin, INPUT_PULLUP);

  ledcAttach(servoPin, pwmFreq, pwmResolution);
  delay(100);

  ledcWrite(servoPin, dutyFromUs((uint32_t)pulseStart));
}

void loop() {
  int state = digitalRead(beamPin);
  unsigned long now = micros();

  if (lastState == HIGH && state == LOW) {
    if (edgeTime != 0) {
      unsigned long period = now - edgeTime;

      if (period > 0) {
        rpm = 60000000.0 / period;
      }

      float error = rpmTarget - rpm;

      pulseStart = pulseStart + Kp * error;

//update
      ledcWrite(servoPin, dutyFromUs((uint32_t)pulseStart));

      Serial.print("RPM: ");
      Serial.print(rpm);
      Serial.print(" | Error: ");
      Serial.print(error);
      Serial.print(" | Pulse: ");
      Serial.println(pulseStart);
    }

    edgeTime = now;
  }

  lastState = state;
}
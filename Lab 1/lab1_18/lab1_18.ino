const int LED1 = A0;
const int LED2 = A1;
const int LED3 = A5;

String input = "";

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);

  Serial.println("Enter a 3-bit code:");
}

void loop() {
  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
    input.trim();

    Serial.println("Received: ");
    Serial.println(input);

    if (input.length() == 3 &&
        (input[0] == '0' || input[0] == '1') &&
        (input[1] == '0' || input[1] == '1') &&
        (input[2] == '0' || input[2] == '1')){

      digitalWrite(LED1, input[0] == '1' ? HIGH : LOW);
      digitalWrite(LED2, input[1] == '1' ? HIGH : LOW);
      digitalWrite(LED3, input[2] == '1' ? HIGH : LOW);

    } else {
      Serial.println("Invalid input. Try again.");
    }

    Serial.println("Enter new code to change LEDs:");
  }
}
#define PIN_FINAL_CARRERA 15

void setup() {
  pinMode(PIN_FINAL_CARRERA, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println("Test final de carrera - Pin 36");
}

void loop() {
  int estado = digitalRead(PIN_FINAL_CARRERA);

  if (estado == LOW) {
    Serial.println("Desactivado");
  } else {
    Serial.println("Activado");
  }

  delay(100);
}

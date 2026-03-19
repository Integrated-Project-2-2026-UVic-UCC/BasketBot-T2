// Test NEMA 17 con TMC2209 - Modo STEP/DIR
// Conexión: STEP=32, DIR=33, EN=25

#define STEP_PIN 32
#define DIR_PIN  33
#define EN_PIN   25

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN,  OUTPUT);
  pinMode(EN_PIN,   OUTPUT);

  // LOW = driver habilitado (activo bajo)
  digitalWrite(EN_PIN, LOW);
  
  Serial.begin(115200);
  Serial.println("Motor listo");
}

void loop() {
  Serial.println("Subiendo...");
  digitalWrite(DIR_PIN, HIGH);
  moverMotor(10000);  // 200 pasos = 1 vuelta (motor 1.8°)
  delay(500);

  Serial.println("Bajando...");
  digitalWrite(DIR_PIN, LOW);
  moverMotor(10000);
  delay(500);
}

void moverMotor(int pasos) {
  for (int i = 0; i < pasos; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000);  // Ajusta esta velocidad (µs entre pulsos)
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);
  }
}
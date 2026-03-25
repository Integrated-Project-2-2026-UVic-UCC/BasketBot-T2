// Control NEMA 17 con TMC2209 via Serial
// Comandos: u = subir, d = bajar
// Pines: STEP=32, DIR=33, EN=25

#define STEP_PIN 32
#define DIR_PIN  33
#define EN_PIN   25

int VELOCIDAD_US = 1000;
long pasos_totales = 0;

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN,  OUTPUT);
  pinMode(EN_PIN,   OUTPUT);
  digitalWrite(EN_PIN, LOW);

  Serial.begin(115200);
  Serial.println("=== Control NEMA 17 ===");
  Serial.println("u = subir 100 pasos");
  Serial.println("d = bajar 100 pasos");
  Serial.println("p = ver posicion");
  Serial.println("r = resetear contador");
  Serial.println("=======================");
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    if (cmd == 'u' || cmd == 'U') {
      digitalWrite(DIR_PIN, HIGH);
      moverMotor(30000);
      pasos_totales += 1000;
      Serial.print(">> Posicion: ");
      Serial.print(pasos_totales);
      Serial.println(" pasos");

    } else if (cmd == 'd' || cmd == 'D') {
      digitalWrite(DIR_PIN, LOW);
      moverMotor(30000);
      pasos_totales -= 1000;
      Serial.print(">> Posicion: ");
      Serial.print(pasos_totales);
      Serial.println(" pasos");

    } else if (cmd == 'p' || cmd == 'P') {
      Serial.print(">> Posicion: ");
      Serial.print(pasos_totales);
      Serial.println(" pasos");

    } else if (cmd == 'r' || cmd == 'R') {
      pasos_totales = 0;
      Serial.println(">> Contador reseteado a 0");
    }
    // ignora \n y \r que llegan tras el comando
  }
}

void moverMotor(int pasos) {
  for (int i = 0; i < pasos; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(VELOCIDAD_US);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(VELOCIDAD_US);
  }
}
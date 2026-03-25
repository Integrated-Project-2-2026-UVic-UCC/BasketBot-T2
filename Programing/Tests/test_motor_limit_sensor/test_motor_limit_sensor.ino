#define STEP_PIN 32
#define DIR_PIN  33
#define EN_PIN   25
#define PIN_FC   15   // cambia al pin que uses

int VELOCIDAD_US = 1000;

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN,  OUTPUT);
  pinMode(EN_PIN,   OUTPUT);
  pinMode(PIN_FC,   INPUT_PULLUP);
  
  digitalWrite(EN_PIN, LOW);
  
  Serial.begin(115200);
  Serial.println("Buscando final de carrera...");
  
  // Bajar hasta que toque el final de carrera
  digitalWrite(DIR_PIN, HIGH);  // dirección bajar
  while (digitalRead(PIN_FC) == LOW) {
    moverMotor(1);
  }
  
  Serial.println("Final de carrera activado!");
  Serial.println("Subiendo un poco para liberarlo...");
  
  // Subir un poco para no aplastarlo
  digitalWrite(DIR_PIN, LOW);  // dirección subir
  moverMotor(300);
  
  Serial.println("Listo - posicion HOME establecida");
}

void loop() {
  // Nada, el homing solo se hace una vez al arrancar
}

void moverMotor(int pasos) {
  for (int i = 0; i < pasos; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(VELOCIDAD_US);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(VELOCIDAD_US);
  }
}
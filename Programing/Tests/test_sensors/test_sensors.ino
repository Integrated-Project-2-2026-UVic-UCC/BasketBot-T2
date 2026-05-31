/*
 * DETECTOR DE CANASTA - ESP32
 * Lógica invertida: LOW = haz libre (recibe IR), HIGH = obstáculo (cortado)
 */

// ==================== ESTRUCTURA PARA LECTURAS ====================
struct LecturaSensores {
  bool arribaDetectado;  // true = OBSTÁCULO detectado (haz cortado)
  bool abajoDetectado;   // true = OBSTÁCULO detectado (haz cortado)
};

// ==================== CONFIGURACIÓN DE PINES ====================
const int PIN_SENSOR_ARRIBA = 15;
const int PIN_SENSOR_ABAJO  = 13;
const int PIN_IR_EMISOR     = 12;
const int PIN_LED_AVISO_1   = 14;
const int PIN_LED_AVISO_2   = 4;

// ==================== CONFIGURACIÓN IR ====================
const int FRECUENCIA_IR = 38000;
const int RESOLUCION_PWM = 8;
const int DUTY_CYCLE = 127;

// ==================== TEMPORIZACIÓN ====================
const unsigned long TIEMPO_MAXIMO_CAIDA = 600;
const unsigned long DELAY_RAFAGA = 2;
const unsigned long DELAY_RECUPERACION = 15;
const unsigned long DELAY_ANTI_REBOTE = 1000;

// ==================== VARIABLES DE ESTADO ====================
bool pelotaEnAro = false;
unsigned long tiempoDeteccionAro = 0;

// Calibración: estado normal sin obstáculos
bool estadoNormalArriba = false;  // true = normalmente recibe IR (LOW)
bool estadoNormalAbajo = false;   // true = normalmente recibe IR (LOW)

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(PIN_SENSOR_ARRIBA, INPUT);
  pinMode(PIN_SENSOR_ABAJO, INPUT);
  pinMode(PIN_LED_AVISO_1, OUTPUT);
  pinMode(PIN_LED_AVISO_2, OUTPUT);
  
  ledcAttach(PIN_IR_EMISOR, FRECUENCIA_IR, RESOLUCION_PWM);
  ledcWrite(PIN_IR_EMISOR, 0);
  
  Serial.println("========================================");
  Serial.println("  SISTEMA DE DETECCION DE CANASTA");
  Serial.println("========================================");
  
  // === CALIBRACIÓN AUTOMÁTICA ===
  Serial.println("Calibrando sensores... NO PONGAS LA PELOTA");
  delay(2000);
  
  // Encender emisor y leer estado base
  ledcWrite(PIN_IR_EMISOR, DUTY_CYCLE);
  delay(5);
  estadoNormalArriba = (digitalRead(PIN_SENSOR_ARRIBA) == LOW);  // LOW = recibe IR
  estadoNormalAbajo = (digitalRead(PIN_SENSOR_ABAJO) == LOW);   // LOW = recibe IR
  ledcWrite(PIN_IR_EMISOR, 0);
  
  Serial.print("Estado normal Arriba: ");
  Serial.println(estadoNormalArriba ? "Recibe IR (LOW)" : "NO recibe IR (HIGH)");
  Serial.print("Estado normal Abajo:  ");
  Serial.println(estadoNormalAbajo ? "Recibe IR (LOW)" : "NO recibe IR (HIGH)");
  Serial.println("========================================\n");
}

// ==================== FUNCIONES ====================

LecturaSensores leerSensoresIR() {
  LecturaSensores lectura;
  
  ledcWrite(PIN_IR_EMISOR, DUTY_CYCLE);
  delay(DELAY_RAFAGA);
  
  // Leer sensores
  bool rawArriba = (digitalRead(PIN_SENSOR_ARRIBA) == LOW);  // LOW = recibe IR
  bool rawAbajo = (digitalRead(PIN_SENSOR_ABAJO) == LOW);    // LOW = recibe IR
  
  ledcWrite(PIN_IR_EMISOR, 0);
  
  // Detectar obstáculo: si el estado cambia respecto al normal
  // Si normalmente recibe IR (LOW) y ahora es HIGH = obstáculo
  lectura.arribaDetectado = (rawArriba != estadoNormalArriba);
  lectura.abajoDetectado = (rawAbajo != estadoNormalAbajo);
  
  return lectura;
}

void actualizarLEDs(bool arriba, bool abajo) {
  digitalWrite(PIN_LED_AVISO_1, arriba ? HIGH : LOW);
  digitalWrite(PIN_LED_AVISO_2, abajo ? HIGH : LOW);
}

// ==================== LOOP ====================
void loop() {
  LecturaSensores lectura = leerSensoresIR();
  
  // --- DEBUG: Mostrar estado crudo y procesado ---
  static bool prevArriba = false;
  static bool prevAbajo = false;
  
  if (lectura.arribaDetectado != prevArriba || lectura.abajoDetectado != prevAbajo) {
    Serial.print("[");
    Serial.print(millis());
    Serial.print(" ms] ");
    
    if (lectura.arribaDetectado && lectura.abajoDetectado) {
      Serial.println("AMBOS cortados (Pelota pasando por aro y red!)");
    } else if (lectura.arribaDetectado) {
      Serial.println("ARO cortado (Pelota en el aro)");
    } else if (lectura.abajoDetectado) {
      Serial.println("RED cortada (Pelota en la red)");
    } else {
      Serial.println("Haz libre en ambos sensores");
    }
    
    prevArriba = lectura.arribaDetectado;
    prevAbajo = lectura.abajoDetectado;
  }
  
  actualizarLEDs(lectura.arribaDetectado, lectura.abajoDetectado);
  
  // --- LÓGICA DE CANASTA ---
  if (lectura.arribaDetectado && !pelotaEnAro) {
    pelotaEnAro = true;
    tiempoDeteccionAro = millis();
    Serial.println("\n>>> Pelota entrando en el aro...");
  }
  
  if (pelotaEnAro) {
    unsigned long transcurrido = millis() - tiempoDeteccionAro;
    
    if (lectura.abajoDetectado) {
      Serial.println(">>> CANASTA!! +2 Puntos");
      Serial.println("========================================\n");
      pelotaEnAro = false;
      delay(DELAY_ANTI_REBOTE);
    } else if (transcurrido > TIEMPO_MAXIMO_CAIDA) {
      Serial.println(">>> Fallo! Timeout (no llego a la red)");
      Serial.println("========================================\n");
      pelotaEnAro = false;
    }
  }
  
  delay(DELAY_RECUPERACION);
}
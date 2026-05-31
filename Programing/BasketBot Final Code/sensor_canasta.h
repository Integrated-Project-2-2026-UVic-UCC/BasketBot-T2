#pragma once

/*
 * sensor_canasta.h — Detección inteligente de canasta con 2 sensores IR
 *
 * Hardware:
 *   GPIO 15 → Sensor TSOP58438 ARRIBA (aro)    — INPUT
 *   GPIO 13 → Sensor TSOP58438 ABAJO  (red)    — INPUT
 *   GPIO 12 → LED emisor IR 38kHz              — PWM output
 *
 * Lógica de detección:
 *   1. Pelota pasa por ARRIBA (aro) → inicia ventana temporal
 *   2. Si pasa por ABAJO (red) dentro del tiempo → CANASTA VÁLIDA
 *   3. Si timeout sin pasar por ABAJO → FALLO (rebote en aro)
 *   4. Si ABAJO detecta sin ARRIBA previo → IGNORADO (entrada desde abajo)
 *   5. Si ARRIBA detecta pero ABAJO ya está activo → IGNORADO (pelota atascada)
 *
 * Estado del sensor:
 *   - LOW  (0) → Haz libre (recibe IR)
 *   - HIGH (1) → Obstáculo detectado (haz cortado)
 *
 * Anti-rebote:
 *   - Cooldown de 500 ms tras cada canasta válida
 *   - Filtrado de detecciones simultáneas (pelota atascada)
 */

// ─── CONFIGURACIÓN DE HARDWARE ───────────────────────────────────────────────
#define SC_PIN_SENSOR_ARRIBA  15
#define SC_PIN_SENSOR_ABAJO   13
#define SC_PIN_IR_EMISOR      12

// ─── CONFIGURACIÓN IR ────────────────────────────────────────────────────────
#define SC_FRECUENCIA_IR      38000    // Hz - frecuencia del TSOP58438
#define SC_DUTY_CYCLE         127      // 50% duty cycle (0-255)
#define SC_DURACION_RAFAGA_US 2000     // Duración de la ráfaga IR en microsegundos

// ─── TIEMPOS DE DETECCIÓN ────────────────────────────────────────────────────
#define SC_TIEMPO_MAX_CAIDA   600      // ms máximo entre aro y red (trayectoria normal)
#define SC_COOLDOWN_CANASTA   500      // ms de bloqueo tras canasta válida (anti-rebote)
#define SC_DELAY_LECTURA      15       // ms de espera entre lecturas (recuperación sensor)

// ─── ESTADOS ─────────────────────────────────────────────────────────────────
enum ScEstado {
  SC_IDLE,              // Esperando detección inicial
  SC_ESPERANDO_RED,     // Aro detectado, esperando que pase por la red
  SC_COOLDOWN           // Canasta confirmada, en periodo de anti-rebote
};

// ─── VARIABLES PRIVADAS ──────────────────────────────────────────────────────
static ScEstado      _sc_estado          = SC_IDLE;
static unsigned long _sc_tiempoAro       = 0;      // Timestamp cuando se detectó el aro
static unsigned long _sc_inicioCooldown  = 0;      // Timestamp inicio del cooldown
static bool          _sc_inicializado    = false;
static unsigned long _sc_ultimaLectura   = 0;      // Control de timing entre lecturas

// Callback opcional para notificar canasta
static void (*_sc_callback_canasta)() = nullptr;

// Estadísticas (opcional, para debug)
static unsigned int  _sc_canastas_validas = 0;
static unsigned int  _sc_fallos_timeout   = 0;
static unsigned int  _sc_entradas_abajo   = 0;    // Intentos desde abajo (inválidos)

// ─── FUNCIONES PRIVADAS ──────────────────────────────────────────────────────

/*
 * Lee un sensor IR con ráfaga de luz IR
 * Retorna true si hay OBSTÁCULO (haz cortado = HIGH)
 *         false si haz libre (recibe IR = LOW)
 */
static bool _sc_leerSensor(int pin) {
  // Activar emisor IR
  ledcWrite(SC_PIN_IR_EMISOR, SC_DUTY_CYCLE);
  delayMicroseconds(SC_DURACION_RAFAGA_US);
  
  // Leer estado del sensor
  bool obstaculo = (digitalRead(pin) == LOW);
  
  // Apagar emisor IR
  ledcWrite(SC_PIN_IR_EMISOR, 0);
  
  return obstaculo;
}

/*
 * Registra una canasta válida
 */
static void _sc_registrarCanasta() {
  _sc_canastas_validas++;
  _sc_estado = SC_COOLDOWN;
  _sc_inicioCooldown = millis();
  
  Serial.println("[SENSOR] *** CANASTA VÁLIDA ***");
  
  // Ejecutar callback si está registrado
  if (_sc_callback_canasta != nullptr) {
    _sc_callback_canasta();
  }
}

/*
 * Procesa el timeout de espera de red
 */
static void _sc_procesarTimeout() {
  _sc_fallos_timeout++;
  _sc_estado = SC_IDLE;
  Serial.println("[SENSOR] Fallo: pelota no llegó a la red (rebote en aro)");
}

/*
 * Filtra entrada inválida desde abajo
 */
static void _sc_filtrarEntradaAbajo() {
  _sc_entradas_abajo++;
  Serial.println("[SENSOR] Ignorado: detección desde abajo sin pasar por aro");
}

// ─── API PÚBLICA ─────────────────────────────────────────────────────────────

/*
 * Inicializa el sistema de sensores
 * Debe llamarse en setup()
 */
void sensor_iniciar() {
  pinMode(SC_PIN_SENSOR_ARRIBA, INPUT);
  pinMode(SC_PIN_SENSOR_ABAJO,  INPUT);
  
  // Configurar PWM para el LED IR (ESP32 Arduino 3.x usa ledcAttach)
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcAttach(SC_PIN_IR_EMISOR, SC_FRECUENCIA_IR, 8);
  #else
    ledcSetup(0, SC_FRECUENCIA_IR, 8);
    ledcAttachPin(SC_PIN_IR_EMISOR, 0);
  #endif
  
  ledcWrite(SC_PIN_IR_EMISOR, 0);  // Emisor apagado inicialmente
  
  _sc_estado          = SC_IDLE;
  _sc_tiempoAro       = 0;
  _sc_inicioCooldown  = 0;
  _sc_ultimaLectura   = 0;
  _sc_inicializado    = true;
  
  // Reset estadísticas
  _sc_canastas_validas = 0;
  _sc_fallos_timeout   = 0;
  _sc_entradas_abajo   = 0;
  
  Serial.println("[SENSOR] Sistema de detección de canasta iniciado");
  Serial.printf("[SENSOR] Tiempo máx caída: %d ms | Cooldown: %d ms\n",
                SC_TIEMPO_MAX_CAIDA, SC_COOLDOWN_CANASTA);
}

/*
 * Registra una función callback que se ejecutará cuando se detecte una canasta
 * Ejemplo: sensor_setCallback(miCallbackCanasta);
 */
void sensor_setCallback(void (*callback)()) {
  _sc_callback_canasta = callback;
}

/*
 * Procesa la lógica de detección
 * Debe llamarse continuamente en loop()
 * 
 * Retorna true si se detectó una canasta en esta llamada
 */
bool sensor_tick() {
  if (!_sc_inicializado) {
    Serial.println("[SENSOR] ERROR: sensor_iniciar() no fue llamado");
    return false;
  }
  
  // Control de timing entre lecturas
  unsigned long ahora = millis();
  if (ahora - _sc_ultimaLectura < SC_DELAY_LECTURA) {
    return false;  // Aún no es tiempo de leer
  }
  _sc_ultimaLectura = ahora;
  
  // Leer ambos sensores
  bool arriba = _sc_leerSensor(SC_PIN_SENSOR_ARRIBA);
  bool abajo  = _sc_leerSensor(SC_PIN_SENSOR_ABAJO);
  
  // ─── MÁQUINA DE ESTADOS ────────────────────────────────────────────────────
  
  switch (_sc_estado) {
    
    // ── ESTADO: IDLE (esperando detección inicial) ──────────────────────────
    case SC_IDLE:
      // Caso 1: Pelota pasa por ARRIBA (aro) sin estar en ABAJO
      if (arriba && !abajo) {
        _sc_estado = SC_ESPERANDO_RED;
        _sc_tiempoAro = ahora;
        Serial.println("[SENSOR] Aro detectado → esperando red...");
      }
      // Caso 2: ARRIBA y ABAJO simultáneos = pelota atascada o muy lenta
      else if (arriba && abajo) {
        Serial.println("[SENSOR] Ignorado: detección simultánea (pelota atascada?)");
      }
      // Caso 3: Solo ABAJO = entrada desde abajo (inválido)
      else if (!arriba && abajo) {
        _sc_filtrarEntradaAbajo();
      }
      // Caso 4: Ninguno = estado normal, continuar esperando
      break;
    
    // ── ESTADO: ESPERANDO_RED (aro detectado, esperando red) ────────────────
    case SC_ESPERANDO_RED: {
      unsigned long transcurrido = ahora - _sc_tiempoAro;
      
      // Canasta válida: red detectada dentro del tiempo
      if (abajo) {
        _sc_registrarCanasta();
        return true;  // ← Retorna true para indicar canasta
      }
      // Timeout: pelota no llegó a la red (rebote en aro)
      else if (transcurrido > SC_TIEMPO_MAX_CAIDA) {
        _sc_procesarTimeout();
      }
      // Continuar esperando...
      break;
    }
    
    // ── ESTADO: COOLDOWN (anti-rebote tras canasta) ─────────────────────────
    case SC_COOLDOWN:
      if (ahora - _sc_inicioCooldown >= SC_COOLDOWN_CANASTA) {
        _sc_estado = SC_IDLE;
        Serial.println("[SENSOR] Cooldown finalizado → listo para nueva detección");
      }
      // Durante cooldown se ignoran todas las detecciones
      break;
  }
  
  return false;
}

/*
 * Retorna el número de canastas válidas detectadas desde el inicio
 */
unsigned int sensor_getCanastasTotales() {
  return _sc_canastas_validas;
}

/*
 * Retorna estadísticas de detección (para debug)
 */
String sensor_getEstadisticas() {
  String stats = "";
  stats += "Canastas válidas: " + String(_sc_canastas_validas) + "\n";
  stats += "Fallos (timeout): " + String(_sc_fallos_timeout) + "\n";
  stats += "Entradas inválidas (desde abajo): " + String(_sc_entradas_abajo) + "\n";
  return stats;
}

/*
 * Resetea los contadores de estadísticas
 */
void sensor_resetEstadisticas() {
  _sc_canastas_validas = 0;
  _sc_fallos_timeout   = 0;
  _sc_entradas_abajo   = 0;
  Serial.println("[SENSOR] Estadísticas reseteadas");
}

/*
 * Retorna el estado actual del sensor (para debug)
 */
String sensor_getEstado() {
  switch (_sc_estado) {
    case SC_IDLE:           return "IDLE";
    case SC_ESPERANDO_RED:  return "ESPERANDO_RED";
    case SC_COOLDOWN:       return "COOLDOWN";
    default:                return "DESCONOCIDO";
  }
}

/*
 * Fuerza un reset del sensor al estado IDLE
 * Útil si se detecta un estado inconsistente
 */
void sensor_reset() {
  _sc_estado = SC_IDLE;
  _sc_tiempoAro = 0;
  _sc_inicioCooldown = 0;
  Serial.println("[SENSOR] Reset manual → estado IDLE");
}

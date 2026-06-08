#pragma once

#define MC_STEP_PIN    32
#define MC_DIR_PIN     33
#define MC_EN_PIN      25
#define MC_FC_PIN      35     // ← CAMBIADO de 15 a 35 (evita conflicto con sensor)

#define MC_PASOS_POR_CM   2000L
#define MC_CM_MAX         20
#define MC_PASOS_MAX      ((long)MC_CM_MAX * MC_PASOS_POR_CM)   // 40 000
#define MC_HOMING_BACKUP  300      // pasos hacia arriba tras tocar FC
#define MC_HOMING_LIMITE  50000L   // si no toca FC en este nº de pasos → error
#define MC_VEL_US         800      // microsegundos entre HIGH y LOW del STEP
                                   // (bajar para ir más rápido, mínimo ~300)

// ─── ESTADOS ─────────────────────────────────────────────────────────────────
enum McEstado { MC_IDLE, MC_HOMING_DOWN, MC_HOMING_UP, MC_MOVING, MC_ERROR };

static McEstado  _mc_estado          = MC_IDLE;
static long      _mc_posActual       = 0;     // pasos desde HOME (0 = posición más baja)
static long      _mc_posObjetivo     = 0;
static int       _mc_cmObjetivo      = 0;     // último cm pedido (0-20)
static long      _mc_homingContador  = 0;     // pasos dados durante homing
static int       _mc_homingBackup    = 0;     // pasos dados en la fase de subida
static unsigned long _mc_ultimoPaso  = 0;     // micros del último paso dado

// ─── HELPERS ─────────────────────────────────────────────────────────────────
static void _mc_step() {
  digitalWrite(MC_STEP_PIN, HIGH);
  delayMicroseconds(MC_VEL_US);
  digitalWrite(MC_STEP_PIN, LOW);
  delayMicroseconds(MC_VEL_US);
}

static bool _mc_fcActivo() {
  // FC activo HIGH (INPUT_PULLUP + señal cierra a VCC — revisa tu hardware)
  // Si tu FC cierra a GND cambia a: return digitalRead(MC_FC_PIN) == LOW;
  return digitalRead(MC_FC_PIN) == HIGH;
}

// ─── API PÚBLICA ─────────────────────────────────────────────────────────────

void motor_iniciar() {
  pinMode(MC_STEP_PIN, OUTPUT);
  pinMode(MC_DIR_PIN,  OUTPUT);
  pinMode(MC_EN_PIN,   OUTPUT);
  pinMode(MC_FC_PIN,   INPUT_PULLUP);

  digitalWrite(MC_EN_PIN, LOW);   // motor activo
  Serial.println("[MOTOR] Iniciado (FC en GPIO 35). Lanzando homing...");
}

// Lanza secuencia de homing (no bloqueante, continúa en motor_tick)
void motor_homing() {
  _mc_estado         = MC_HOMING_DOWN;
  _mc_homingContador = 0;
  _mc_homingBackup   = 0;
  _mc_posActual      = 0;
  _mc_posObjetivo    = 0;
  digitalWrite(MC_DIR_PIN, HIGH);   // bajar
  Serial.println("[MOTOR] Homing: bajando hacia FC...");
}

// Envía motor a posición en cm (0-20). Lanza homing si es necesario primero.
void motor_irACm(int cm) {
  cm = constrain(cm, 0, MC_CM_MAX);
  _mc_cmObjetivo  = cm;
  _mc_posObjetivo = (long)cm * MC_PASOS_POR_CM;

  if (_mc_estado == MC_ERROR || _mc_estado == MC_HOMING_DOWN || _mc_estado == MC_HOMING_UP) {
    Serial.printf("[MOTOR] Pendiente ir a %d cm tras homing\n", cm);
    return;   // motor_tick irá allí automáticamente cuando acabe el homing
  }

  if (_mc_posActual == _mc_posObjetivo) {
    Serial.printf("[MOTOR] Ya en %d cm\n", cm);
    return;
  }

  _mc_estado = MC_MOVING;
  if (_mc_posObjetivo > _mc_posActual) {
    digitalWrite(MC_DIR_PIN, LOW);   // subir (alejarse del FC)
    Serial.printf("[MOTOR] Subiendo a %d cm\n", cm);
  } else {
    digitalWrite(MC_DIR_PIN, HIGH);  // bajar (acercarse al FC)
    Serial.printf("[MOTOR] Bajando a %d cm\n", cm);
  }
}

// Estado para la app
bool motor_ocupado()   { return _mc_estado != MC_IDLE && _mc_estado != MC_ERROR; }
bool motor_error()     { return _mc_estado == MC_ERROR; }
int  motor_cmActual()  { return (int)(_mc_posActual / MC_PASOS_POR_CM); }
int  motor_cmObjetivo(){ return _mc_cmObjetivo; }

String motor_statusJSON() {
  String st;
  switch(_mc_estado) {
    case MC_IDLE:         st="Listo";    break;
    case MC_HOMING_DOWN:
    case MC_HOMING_UP:    st="Homing";   break;
    case MC_MOVING:       st="Moviendo"; break;
    case MC_ERROR:        st="Error";    break;
    default:              st="Desconocido";
  }
  String j = "";
  j += "\"motorState\":\"" + st + "\",";
  j += "\"motorCm\":"      + String(motor_cmActual())   + ",";
  j += "\"motorTarget\":"  + String(_mc_cmObjetivo);
  return j;
}

// ─── TICK — llamar en cada loop() ────────────────────────────────────────────
void motor_tick() {
  // ── HOMING: bajando hasta el final de carrera ────────────────────────────
  if (_mc_estado == MC_HOMING_DOWN) {
    if (_mc_fcActivo()) {
      // Tocó el final de carrera → ahora subir 300 pasos
      _mc_homingBackup = 0;
      _mc_estado = MC_HOMING_UP;
      digitalWrite(MC_DIR_PIN, LOW);  // subir
      Serial.println("[MOTOR] FC activado. Subiendo para liberarlo...");
      return;
    }
    if (_mc_homingContador >= MC_HOMING_LIMITE) {
      _mc_estado = MC_ERROR;
      digitalWrite(MC_EN_PIN, HIGH);  // apagar motor
      Serial.println("[MOTOR] ERROR: FC no encontrado. Motor deshabilitado.");
      return;
    }
    _mc_step();
    _mc_homingContador++;
    return;
  }

  // ── HOMING: subiendo para liberar el FC ──────────────────────────────────
  if (_mc_estado == MC_HOMING_UP) {
    if (_mc_homingBackup >= MC_HOMING_BACKUP) {
      // Homing completo → posición HOME = 0
      _mc_posActual   = 0;
      _mc_estado      = MC_IDLE;
      Serial.println("[MOTOR] Homing completado. Posición HOME = 0 cm.");
      // Si había un objetivo pendiente, ir a él
      if (_mc_posObjetivo > 0) {
        motor_irACm(_mc_cmObjetivo);
      }
      return;
    }
    _mc_step();
    _mc_homingBackup++;
    return;
  }

  // ── MOVING: moviéndose hacia objetivo ────────────────────────────────────
  if (_mc_estado == MC_MOVING) {
    if (_mc_posActual == _mc_posObjetivo) {
      _mc_estado = MC_IDLE;
      Serial.printf("[MOTOR] Posición alcanzada: %d cm (%ld pasos)\n",
                    motor_cmActual(), _mc_posActual);
      return;
    }

    // Seguridad: no pasar de los límites
    if (_mc_posObjetivo > _mc_posActual) {
      // Subiendo
      if (_mc_posActual >= MC_PASOS_MAX) {
        _mc_posObjetivo = MC_PASOS_MAX;
        _mc_estado = MC_IDLE;
        return;
      }
      _mc_step();
      _mc_posActual++;
    } else {
      // Bajando — también comprobamos el FC por seguridad
      if (_mc_fcActivo()) {
        _mc_posActual  = 0;
        _mc_posObjetivo = 0;
        _mc_estado = MC_IDLE;
        Serial.println("[MOTOR] FC inesperado durante movimiento. Parado en HOME.");
        return;
      }
      _mc_step();
      _mc_posActual--;
    }
    return;
  }
}

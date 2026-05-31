#pragma once

/*
 * game_shootout.h — Shoot Out: máximos puntos en el tiempo dado
 *
 * ⚠️ MODIFICADO: Ya NO lee sensor físico directamente.
 *    Las canastas se notifican desde marcador.ino mediante gameShootout_notificarCanasta()
 *
 * Hardware:
 *   GPIO 16 → Botón "Listo"   (INPUT_PULLUP, activo LOW)
 *   GPIO 21 → LED indicador
 *
 * Flujo por turno:
 *   1. SO_ESPERANDO_LISTO  → jugador pulsa GPIO16 (o app) para confirmar que está listo
 *   2. SO_CUENTA_ATRAS     → 3… 2… 1… no bloqueante, 1 s por dígito
 *   3. SO_JUGANDO          → sensor cuenta canastas durante turnoMs
 *   4. SO_FIN_TURNO        → pausa 1.5 s, muestra resultado del turno
 *   5. Siguiente turno, o evaluación de ganador / desempate
 *
 * Anti-rebote botón: detección de flanco descendente + cooldown 300 ms
 */

#define GSO_PIN_BTN_LISTO 16
#define GSO_PIN_LED       21
#define GSO_MAX_JUG       4
#define GSO_BTN_COOLDOWN_MS     300

// ─── ESTADOS ─────────────────────────────────────────────────────────────────
enum SoEstado {
  SO_IDLE,
  SO_ESPERANDO_LISTO,
  SO_CUENTA_ATRAS,
  SO_JUGANDO,
  SO_FIN_TURNO,
  SO_TERMINADO
};

// ─── VARIABLES PRIVADAS ──────────────────────────────────────────────────────
static SoEstado      _so_estado        = SO_IDLE;
static bool          _so_pausado       = false;
static int           _so_numJug        = 2;
static unsigned long _so_turnoMs       = 30000;
static int           _so_puntos[GSO_MAX_JUG] = {0};
static int           _so_turnoActual   = 0;       // índice del jugador en curso
static int           _so_ronda         = 1;
static int           _so_ganador       = -1;

// Lista de jugadores de la ronda actual (índices 0-based)
// En primera ronda = todos; en desempate = solo los empatados
static int           _so_grupo[GSO_MAX_JUG];
static int           _so_nGrupo        = 0;
static int           _so_idxEnGrupo    = 0;       // posición del turnoActual dentro de _so_grupo

// Tiempos
static unsigned long _so_inicioJuego   = 0;
static unsigned long _so_inicioCuenta  = 0;
static int           _so_cuentaActual  = 3;
static unsigned long _so_finTurnoTs    = 0;
static unsigned long _so_pausaTs       = 0;       // timestamp al pausar (para compensar)

// Anti-rebote botón listo
static bool          _so_pinBtn        = HIGH;
static unsigned long _so_ultimoBtn     = 0;

// Log serial
static unsigned long _so_ultimoSerial  = 0;

// ─── FORWARD DECLARATIONS ────────────────────────────────────────────────────
void gameShootout_printSerial();
static void _so_pedirListo();
static void _so_iniciarCuentaAtras();
static void _so_iniciarTurno();
static void _so_finalizarTurno();
static void _so_evaluarFin();

// ─── HELPERS ─────────────────────────────────────────────────────────────────
static void _so_setLED(bool on) { digitalWrite(GSO_PIN_LED, on ? HIGH : LOW); }

// Flanco bajante botón listo
static bool _so_btnFlanco() {
  bool pin = digitalRead(GSO_PIN_BTN_LISTO);
  bool flanco = (_so_pinBtn == HIGH && pin == LOW);
  _so_pinBtn = pin;
  if (flanco && millis() - _so_ultimoBtn >= GSO_BTN_COOLDOWN_MS) {
    _so_ultimoBtn = millis();
    return true;
  }
  return false;
}

static void _so_pedirListo() {
  _so_estado   = SO_ESPERANDO_LISTO;
  _so_setLED(false);
  // Sincronizar estado de pin para no detectar flancos residuales
  _so_pinBtn    = digitalRead(GSO_PIN_BTN_LISTO);
  Serial.printf("[SO] Ronda %d — J%d: pulsa LISTO para empezar\n",
                _so_ronda, _so_turnoActual + 1);
}

static void _so_iniciarCuentaAtras() {
  _so_estado       = SO_CUENTA_ATRAS;
  _so_cuentaActual = 3;
  _so_inicioCuenta = millis();
  _so_setLED(false);
  Serial.println("[SO] 3...");
}

static void _so_iniciarTurno() {
  _so_estado        = SO_JUGANDO;
  _so_inicioJuego   = millis();
  _so_ultimoSerial  = millis();
  _so_setLED(true);
  Serial.printf("[SO] ¡YA! J%d — %lu s\n",
                _so_turnoActual + 1, _so_turnoMs / 1000);
}

static void _so_finalizarTurno() {
  _so_setLED(false);
  _so_estado     = SO_FIN_TURNO;
  _so_finTurnoTs = millis();
  Serial.printf("[SO] Fin turno J%d → %d pts\n",
                _so_turnoActual + 1, _so_puntos[_so_turnoActual]);
  gameShootout_printSerial();
}

static void _so_evaluarFin() {
  // ¿Hay más jugadores en este grupo que no han jugado?
  if (_so_idxEnGrupo < _so_nGrupo - 1) {
    _so_idxEnGrupo++;
    _so_turnoActual = _so_grupo[_so_idxEnGrupo];
    _so_pedirListo();
    return;
  }

  // Todos del grupo han jugado → buscar el máximo
  int maxPts = -1;
  for (int i = 0; i < _so_nGrupo; i++) {
    if (_so_puntos[_so_grupo[i]] > maxPts) maxPts = _so_puntos[_so_grupo[i]];
  }

  // Recolectar empatados
  int empArr[GSO_MAX_JUG];
  int nEmp = 0;
  for (int i = 0; i < _so_nGrupo; i++) {
    if (_so_puntos[_so_grupo[i]] == maxPts) empArr[nEmp++] = _so_grupo[i];
  }

  if (nEmp == 1) {
    // Ganador claro
    _so_ganador = empArr[0];
    _so_estado  = SO_TERMINADO;
    _so_setLED(false);
    Serial.printf("[SO] *** GANADOR: J%d con %d pts ***\n",
                  _so_ganador + 1, _so_puntos[_so_ganador]);
  } else {
    // Desempate: nueva ronda solo entre empatados
    _so_ronda++;
    Serial.printf("[SO] EMPATE! Ronda %d de desempate entre %d jugadores\n",
                  _so_ronda, nEmp);
    // Resetear puntos de los empatados para la ronda extra
    for (int i = 0; i < nEmp; i++) _so_puntos[empArr[i]] = 0;
    // Configurar nuevo grupo
    _so_nGrupo = nEmp;
    for (int i = 0; i < nEmp; i++) _so_grupo[i] = empArr[i];
    _so_idxEnGrupo  = 0;
    _so_turnoActual = _so_grupo[0];
    _so_pedirListo();
  }
}

// ─── API PÚBLICA ─────────────────────────────────────────────────────────────
void gameShootout_iniciar(int numJugadores, unsigned long turnoMs) {
  pinMode(GSO_PIN_BTN_LISTO, INPUT_PULLUP);
  pinMode(GSO_PIN_LED,       OUTPUT);
  _so_setLED(false);

  _so_numJug        = constrain(numJugadores, 1, GSO_MAX_JUG);
  _so_turnoMs       = turnoMs;
  _so_ronda         = 1;
  _so_ganador       = -1;
  _so_pausado       = false;
  _so_pinBtn        = HIGH;
  _so_ultimoBtn     = 0;

  for (int i = 0; i < GSO_MAX_JUG; i++) _so_puntos[i] = 0;

  // Grupo inicial: todos los jugadores en orden
  _so_nGrupo = _so_numJug;
  for (int i = 0; i < _so_numJug; i++) _so_grupo[i] = i;
  _so_idxEnGrupo  = 0;
  _so_turnoActual = 0;

  Serial.printf("=== SHOOT OUT | %d jug | %lu s/turno ===\n",
                _so_numJug, _so_turnoMs / 1000);
  _so_pedirListo();
}

void gameShootout_tick() {
  if (_so_estado == SO_IDLE || _so_estado == SO_TERMINADO) return;
  if (_so_pausado) return;

  // ── Esperando que el jugador pulse Listo ──────────────────────────────────
  if (_so_estado == SO_ESPERANDO_LISTO) {
    if (_so_btnFlanco()) {
      Serial.printf("[SO] J%d listo!\n", _so_turnoActual + 1);
      _so_iniciarCuentaAtras();
    }
    return;
  }

  // ── Cuenta atrás 3-2-1 ───────────────────────────────────────────────────
  if (_so_estado == SO_CUENTA_ATRAS) {
    unsigned long el = millis() - _so_inicioCuenta;
    int digito = 3 - (int)(el / 1000);
    if (digito < _so_cuentaActual && digito > 0) {
      _so_cuentaActual = digito;
      Serial.printf("[SO] %d...\n", digito);
    }
    if (el >= 3000) _so_iniciarTurno();
    return;
  }

  // ── Turno activo ─────────────────────────────────────────────────────────
  if (_so_estado == SO_JUGANDO) {
    unsigned long el = millis() - _so_inicioJuego;

    // Log cada segundo
    if (millis() - _so_ultimoSerial >= 1000) {
      _so_ultimoSerial = millis();
      unsigned long rest = (el < _so_turnoMs) ? (_so_turnoMs - el) / 1000 : 0;
      Serial.printf("[SO] J%d | %lu s | %d pts\n",
                    _so_turnoActual + 1, rest, _so_puntos[_so_turnoActual]);
    }

    // Fin del tiempo
    if (el >= _so_turnoMs) _so_finalizarTurno();
    return;
  }

  // ── Pausa breve entre turnos ──────────────────────────────────────────────
  if (_so_estado == SO_FIN_TURNO) {
    if (millis() - _so_finTurnoTs >= 1500) _so_evaluarFin();
    return;
  }
}

/*
 * NUEVA FUNCIÓN: Notifica que el sensor físico detectó una canasta
 * Llamada desde marcador.ino cuando sensor_tick() retorna true
 */
void gameShootout_notificarCanasta() {
  if (_so_estado != SO_JUGANDO || _so_pausado) return;
  _so_puntos[_so_turnoActual]++;
  Serial.printf("[SO] CANASTA! J%d → %d pts\n",
                _so_turnoActual + 1, _so_puntos[_so_turnoActual]);
}

String gameShootout_statusJSON() {
  String st;
  switch (_so_estado) {
    case SO_TERMINADO:       st = "Fin";       break;
    case SO_JUGANDO:         st = "Jugando";   break;
    case SO_CUENTA_ATRAS:    st = "Cuenta";    break;
    case SO_ESPERANDO_LISTO: st = "Listo";     break;
    case SO_FIN_TURNO:       st = "FinTurno";  break;
    default:                 st = "Esperando"; break;
  }
  if (_so_pausado && _so_estado != SO_TERMINADO) st = "Pausado";

  unsigned long tiempoRestante = 0;
  if (_so_estado == SO_JUGANDO) {
    unsigned long el = millis() - _so_inicioJuego;
    tiempoRestante = (el < _so_turnoMs) ? (_so_turnoMs - el) / 1000 : 0;
  }

  String j = "";
  j += "\"mode\":\"shootout\",";
  j += "\"state\":\""        + st + "\",";
  j += "\"turno\":"          + String(_so_turnoActual) + ",";
  j += "\"ronda\":"          + String(_so_ronda)       + ",";
  j += "\"ganador\":"        + String(_so_ganador)     + ",";
  j += "\"segundos\":"       + String(tiempoRestante)  + ",";
  j += "\"turnoTotalSeg\":"  + String(_so_turnoMs / 1000) + ",";
  j += "\"cuentaAtras\":"    + String(_so_cuentaActual) + ",";
  j += "\"numJugadores\":"   + String(_so_numJug)      + ",";
  j += "\"puntos\":[";
  for (int i = 0; i < _so_numJug; i++) {
    j += String(_so_puntos[i]);
    if (i < _so_numJug - 1) j += ",";
  }
  j += "]";
  return j;
}

// Jugador pulsa "Listo" desde la app
void gameShootout_listoManual() {
  if (_so_estado != SO_ESPERANDO_LISTO || _so_pausado) return;
  Serial.printf("[APP→SO] J%d listo (manual)\n", _so_turnoActual + 1);
  _so_iniciarCuentaAtras();
}

// Canasta manual desde la app (solo durante SO_JUGANDO)
void gameShootout_canastaManual() {
  if (_so_estado != SO_JUGANDO || _so_pausado) return;
  _so_puntos[_so_turnoActual]++;
  Serial.printf("[APP→SO] Canasta manual J%d → %d pts\n",
                _so_turnoActual + 1, _so_puntos[_so_turnoActual]);
}

void gameShootout_setPausa(bool pausar) {
  if (_so_estado == SO_TERMINADO) return;
  if (pausar && !_so_pausado) {
    _so_pausado  = true;
    _so_pausaTs  = millis();
    _so_setLED(false);
    Serial.println("[SO] Pausado.");
  } else if (!pausar && _so_pausado) {
    unsigned long delta = millis() - _so_pausaTs;
    // Compensar el tiempo perdido en pausa
    if (_so_estado == SO_JUGANDO)      _so_inicioJuego  += delta;
    if (_so_estado == SO_CUENTA_ATRAS) _so_inicioCuenta += delta;
    if (_so_estado == SO_FIN_TURNO)    _so_finTurnoTs   += delta;
    _so_pausado = false;
    if (_so_estado == SO_JUGANDO) _so_setLED(true);
    Serial.println("[SO] Reanudado.");
  }
}

void gameShootout_reset() {
  gameShootout_iniciar(_so_numJug, _so_turnoMs);
}

void gameShootout_stop() {
  _so_estado  = SO_TERMINADO;
  _so_pausado = false;
  _so_setLED(false);
  Serial.println("[SO] Detenido.");
}

void gameShootout_printSerial() {
  Serial.println("----- SHOOT OUT -----");
  for (int i = 0; i < _so_numJug; i++)
    Serial.printf("  %s J%d: %d pts\n",
                  (i == _so_turnoActual) ? ">>" : "  ", i + 1, _so_puntos[i]);
  Serial.println("---------------------");
}

// ─── FUNCIONES AUXILIARES PARA MARCADOR.INO ──────────────────────────────────
bool gameShootout_estaJugando() { return _so_estado == SO_JUGANDO; }
bool gameShootout_estaPausado() { return _so_pausado; }
bool gameShootout_haTerminado() { return _so_estado == SO_TERMINADO; }
int  gameShootout_getPuntosTurno() { return _so_puntos[_so_turnoActual]; }
unsigned long gameShootout_getTiempoRestante() {
  if (_so_estado != SO_JUGANDO) return 0;
  unsigned long el = millis() - _so_inicioJuego;
  return (el < _so_turnoMs) ? (_so_turnoMs - el) : 0;
}

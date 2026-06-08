#pragma once

#define G21_PIN_LED       21
#define G21_MAX_JUG       4
#define G21_PUNTOS_WIN    21
#define G21_PUNTOS_PENAL  11

enum G21Estado { G21_IDLE, G21_ESPERANDO_TIRO, G21_TERMINADO };

static G21Estado     _g21_estado        = G21_IDLE;
static bool          _g21_pausado       = false;
static int           _g21_numJug        = 2;
static unsigned long _g21_ventanaMs     = 3000;
static int           _g21_puntos[G21_MAX_JUG] = {0};
static int           _g21_turno         = 0;
static bool          _g21_tiroLibre     = false;
static int           _g21_ganador       = -1;
static unsigned long _g21_inicioTiro    = 0;
static unsigned long _g21_ultimoSerial  = 0;

// ─── FORWARD DECLARATIONS ────────────────────────────────────────────────────
void game21_printSerial();
static void _g21_procesarCanasta();
static void _g21_procesarFallo();

// ─── HELPERS ─────────────────────────────────────────────────────────────────
static void _g21_setLED(bool on) { digitalWrite(G21_PIN_LED, on ? HIGH : LOW); }

static void _g21_iniciarVentana() {
  _g21_inicioTiro  = millis();
  _g21_setLED(true);
  Serial.printf("[21] Turno J%d [%s] | ventana %lums\n",
    _g21_turno + 1, _g21_tiroLibre ? "TIRO LIBRE" : "TIRO NORMAL", _g21_ventanaMs);
  game21_printSerial();
}

static void _g21_avanzarTurno() {
  _g21_tiroLibre = false;
  _g21_turno     = (_g21_turno + 1) % _g21_numJug;
  _g21_iniciarVentana();
}

// ─── LÓGICA DE PUNTUACIÓN ────────────────────────────────────────────────────
static void _g21_procesarCanasta() {
  if (!_g21_tiroLibre) {
    _g21_puntos[_g21_turno] += 2;
    Serial.printf("[21] CANASTA J%d +2 → %d pts\n", _g21_turno+1, _g21_puntos[_g21_turno]);
  } else {
    _g21_puntos[_g21_turno] += 1;
    Serial.printf("[21] TIRO LIBRE J%d +1 → %d pts\n", _g21_turno+1, _g21_puntos[_g21_turno]);
  }

  if (_g21_puntos[_g21_turno] > G21_PUNTOS_WIN) {
    _g21_puntos[_g21_turno] = G21_PUNTOS_PENAL;
    Serial.printf("[21] PASADO! J%d baja a %d. Turno perdido.\n", _g21_turno+1, G21_PUNTOS_PENAL);
    _g21_avanzarTurno();
  } else if (_g21_puntos[_g21_turno] == G21_PUNTOS_WIN) {
    _g21_ganador = _g21_turno;
    _g21_estado  = G21_TERMINADO;
    _g21_setLED(false);
    Serial.printf("[21] *** GANADOR: J%d con 21 pts! ***\n", _g21_ganador + 1);
  } else {
    _g21_tiroLibre = true;
    _g21_iniciarVentana();
  }
}

static void _g21_procesarFallo() {
  Serial.printf("[21] %s J%d. Cambio turno.\n",
    _g21_tiroLibre ? "Tiro libre fallado." : "Fallo/timeout.", _g21_turno + 1);
  _g21_avanzarTurno();
}

// ─── API PÚBLICA ─────────────────────────────────────────────────────────────
void game21_iniciar(int numJugadores, unsigned long ventanaMs) {
  pinMode(G21_PIN_LED, OUTPUT);
  _g21_setLED(false);

  _g21_numJug    = constrain(numJugadores, 1, G21_MAX_JUG);
  _g21_ventanaMs = ventanaMs;
  _g21_turno     = 0;
  _g21_tiroLibre = false;
  _g21_ganador   = -1;
  _g21_pausado   = false;

  for (int i = 0; i < G21_MAX_JUG; i++) _g21_puntos[i] = 0;

  _g21_ultimoSerial  = millis();
  _g21_estado        = G21_ESPERANDO_TIRO;
  _g21_iniciarVentana();

  Serial.printf("=== JUEGO 21 | %d jug | ventana %lums ===\n", _g21_numJug, _g21_ventanaMs);
}

void game21_tick() {
  if (_g21_estado != G21_ESPERANDO_TIRO || _g21_pausado) return;

  // Log cada segundo
  if (millis() - _g21_ultimoSerial >= 1000) {
    _g21_ultimoSerial = millis();
    unsigned long fin = _g21_inicioTiro + _g21_ventanaMs;
    unsigned long rest = (millis() < fin) ? (fin - millis()) / 1000 : 0;
    Serial.printf("[21] J%d | %lus restantes\n", _g21_turno + 1, rest);
  }

  // Timeout ventana
  if (millis() - _g21_inicioTiro >= _g21_ventanaMs) {
    _g21_procesarFallo();
  }
}

/*
 * NUEVA FUNCIÓN: Notifica que el sensor físico detectó una canasta
 * Llamada desde marcador.ino cuando sensor_tick() retorna true
 */
void game21_notificarCanasta() {
  if (_g21_estado != G21_ESPERANDO_TIRO || _g21_pausado) return;
  _g21_procesarCanasta();
}

String game21_statusJSON() {
  String st;
  if      (_g21_estado == G21_TERMINADO)      st = "Fin";
  else if (_g21_pausado)                       st = "Pausado";
  else if (_g21_estado == G21_ESPERANDO_TIRO)  st = "En juego";
  else                                          st = "Esperando";

  unsigned long restante = 0;
  if (_g21_estado == G21_ESPERANDO_TIRO && !_g21_pausado) {
    unsigned long fin = _g21_inicioTiro + _g21_ventanaMs;
    if (millis() < fin) restante = fin - millis();
  }

  String j = "";
  j += "\"mode\":\"21\",";
  j += "\"state\":\"" + st + "\",";
  j += "\"turno\":"            + String(_g21_turno)   + ",";
  j += "\"tiroLibre\":"        + String(_g21_tiroLibre ? "true" : "false") + ",";
  j += "\"ganador\":"          + String(_g21_ganador) + ",";
  j += "\"ventanaRestante\":"  + String(restante / 1000) + ",";
  j += "\"ventanaTotalMs\":"   + String(_g21_ventanaMs)  + ",";
  j += "\"numJugadores\":"     + String(_g21_numJug)  + ",";
  j += "\"puntos\":[";
  for (int i = 0; i < _g21_numJug; i++) {
    j += String(_g21_puntos[i]);
    if (i < _g21_numJug - 1) j += ",";
  }
  j += "]";
  return j;
}

void game21_puntoManual(int player) {
  if (_g21_estado != G21_ESPERANDO_TIRO || _g21_pausado) return;
  int idx = player - 1;
  if (idx == _g21_turno) {
    Serial.printf("[APP→21] Canasta manual J%d\n", player);
    _g21_procesarCanasta();
  }
}

void game21_falloManual() {
  if (_g21_estado != G21_ESPERANDO_TIRO || _g21_pausado) return;
  Serial.println("[APP→21] Fallo manual");
  _g21_procesarFallo();
}

void game21_setPausa(bool pausar) {
  if (_g21_estado == G21_TERMINADO) return;
  _g21_pausado = pausar;
  if (!pausar) _g21_inicioTiro = millis(); // renueva ventana
  _g21_setLED(!pausar && _g21_estado == G21_ESPERANDO_TIRO);
  Serial.printf("[21] %s\n", pausar ? "Pausado" : "Reanudado");
}

void game21_reset() { game21_iniciar(_g21_numJug, _g21_ventanaMs); }

void game21_stop() {
  _g21_estado  = G21_TERMINADO;
  _g21_pausado = false;
  _g21_setLED(false);
  Serial.println("[21] Detenido.");
}

void game21_printSerial() {
  Serial.println("-----------------------------");
  for (int i = 0; i < _g21_numJug; i++)
    Serial.printf("  %s J%d: %d pts\n", (i==_g21_turno)?">>":"  ", i+1, _g21_puntos[i]);
  Serial.println("-----------------------------");
}

// ─── FUNCIONES AUXILIARES PARA MARCADOR.INO ──────────────────────────────────
bool game21_estaEsperando() { return _g21_estado == G21_ESPERANDO_TIRO; }
bool game21_estaPausado()   { return _g21_pausado; }
bool game21_haTerminado()   { return _g21_estado == G21_TERMINADO; }
int  game21_getPuntosTurno() { return _g21_puntos[_g21_turno]; }
unsigned long game21_getTiempoVentana() {
  if (_g21_estado != G21_ESPERANDO_TIRO || _g21_pausado) return 0;
  unsigned long fin = _g21_inicioTiro + _g21_ventanaMs;
  return (millis() < fin) ? (fin - millis()) : 0;
}

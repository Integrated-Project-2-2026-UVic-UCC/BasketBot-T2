#pragma once

#define G1_PIN_BTN_A    16
#define G1_PIN_BTN_B    17
#define G1_PIN_LED_V    21   // Verde  — espera confirmacion
#define G1_PIN_LED_A    19   // Azul   — equipo A anoto
#define G1_PIN_LED_B    18   // Rojo   — equipo B anoto
#define G1_LED_DESTELLO 300  // ms que dura el destello de equipo

static bool          _g1_activo      = false;
static bool          _g1_pausado     = false;
static bool          _g1_terminado   = false;
static int           _g1_puntos[2]   = {0, 0};
static unsigned long _g1_tiempoFinMs = 0;
static unsigned long _g1_ventanaMs   = 0;
static unsigned long _g1_duracionMs  = 0;
static unsigned long _g1_tiempoRestanteMs = 0;

enum _G1Estado { G1_ESPERANDO_CANASTA, G1_ESPERANDO_PULSADOR };
static _G1Estado _g1_estado = G1_ESPERANDO_CANASTA;

static unsigned long _g1_inicioVentana        = 0;
static unsigned long _g1_ultimoMarcadorSerial = 0;

static int           _g1_ledDestelloEquipo = -1;
static unsigned long _g1_ledApagarTs       = 0;

static unsigned long _g1_debounce[2] = {0, 0};
static bool          _g1_anterior[2] = {true, true};
#define G1_DEBOUNCE_MS 50

void game1vs1_printSerial();

static bool _g1_leerPin(int pin, int idx) {
  bool lec = digitalRead(pin);
  if (lec != _g1_anterior[idx]) _g1_debounce[idx] = millis();
  _g1_anterior[idx] = lec;
  return ((millis() - _g1_debounce[idx]) > G1_DEBOUNCE_MS) && (lec == LOW);
}

static void _g1_apagarLeds() {
  digitalWrite(G1_PIN_LED_V, LOW);
  digitalWrite(G1_PIN_LED_A, LOW);
  digitalWrite(G1_PIN_LED_B, LOW);
  _g1_ledDestelloEquipo = -1;
  _g1_ledApagarTs       = 0;
}

static unsigned long _g1_restanteMs() {
  if (_g1_pausado)  return _g1_tiempoRestanteMs;
  if (!_g1_activo)  return 0;
  unsigned long now = millis();
  return (now < _g1_tiempoFinMs) ? (_g1_tiempoFinMs - now) : 0;
}

static void _g1_finPartido() {
  _g1_activo    = false;
  _g1_terminado = true;
  _g1_apagarLeds();
  Serial.println("============================");
  Serial.println("***    FIN DE PARTIDO    ***");
  Serial.printf("  Equipo A: %d pts\n", _g1_puntos[0]);
  Serial.printf("  Equipo B: %d pts\n", _g1_puntos[1]);
  if      (_g1_puntos[0] > _g1_puntos[1]) Serial.println("  GANADOR: Equipo A");
  else if (_g1_puntos[1] > _g1_puntos[0]) Serial.println("  GANADOR: Equipo B");
  else                                     Serial.println("  EMPATE");
  Serial.println("============================");
}

static void _g1_canastaDetectada() {
  Serial.println(">> CANASTA! Esperando confirmacion de equipo...");
  digitalWrite(G1_PIN_LED_V, HIGH);
  _g1_estado        = G1_ESPERANDO_PULSADOR;
  _g1_inicioVentana = millis();
}

static void _g1_confirmarCanasta(int equipo) {
  _g1_puntos[equipo] += 2;
  digitalWrite(G1_PIN_LED_V, LOW);
  digitalWrite(equipo == 0 ? G1_PIN_LED_A : G1_PIN_LED_B, HIGH);
  _g1_ledDestelloEquipo = equipo;
  _g1_ledApagarTs       = millis() + G1_LED_DESTELLO;
  _g1_estado = G1_ESPERANDO_CANASTA;
  Serial.printf(">> Equipo %s anota! +2 pts -> Total: %d\n",
                equipo == 0 ? "A" : "B", _g1_puntos[equipo]);
}

void game1vs1_iniciar(unsigned long duracionMs, unsigned long ventanaMs) {
  pinMode(G1_PIN_BTN_A,  INPUT_PULLUP);
  pinMode(G1_PIN_BTN_B,  INPUT_PULLUP);
  pinMode(G1_PIN_LED_V,  OUTPUT);
  pinMode(G1_PIN_LED_A,  OUTPUT);
  pinMode(G1_PIN_LED_B,  OUTPUT);
  _g1_apagarLeds();

  _g1_puntos[0]        = 0;
  _g1_puntos[1]        = 0;
  _g1_duracionMs       = duracionMs;
  _g1_ventanaMs        = ventanaMs;
  _g1_tiempoFinMs      = millis() + duracionMs;
  _g1_tiempoRestanteMs = duracionMs;
  _g1_estado           = G1_ESPERANDO_CANASTA;
  _g1_activo           = true;
  _g1_pausado          = false;
  _g1_terminado        = false;
  _g1_ultimoMarcadorSerial = millis();

  for (int i = 0; i < 2; i++) {
    _g1_debounce[i] = millis();
    _g1_anterior[i] = true;
  }

  Serial.println("=== PARTIDO 1vs1 INICIADO ===");
  Serial.printf("Duracion: %lu s | Ventana pulsador: %lu ms\n",
                duracionMs / 1000, ventanaMs);
  Serial.println("=============================");
}

void game1vs1_tick() {
  if (!_g1_activo || _g1_pausado) return;

  // Apagar LED equipo tras destello
  if (_g1_ledDestelloEquipo >= 0 && millis() >= _g1_ledApagarTs) {
    digitalWrite(_g1_ledDestelloEquipo == 0 ? G1_PIN_LED_A : G1_PIN_LED_B, LOW);
    _g1_ledDestelloEquipo = -1;
    _g1_ledApagarTs       = 0;
  }

  if (millis() >= _g1_tiempoFinMs) { _g1_finPartido(); return; }

  if (millis() - _g1_ultimoMarcadorSerial >= 1000) {
    _g1_ultimoMarcadorSerial = millis();
    game1vs1_printSerial();
  }

  // Procesar pulsadores de confirmación
  if (_g1_estado == G1_ESPERANDO_PULSADOR) {
    if (millis() - _g1_inicioVentana >= _g1_ventanaMs) {
      digitalWrite(G1_PIN_LED_V, LOW);
      _g1_estado = G1_ESPERANDO_CANASTA;
      Serial.println(">> Tiempo agotado. Canasta anulada.");
      return;
    }
    if (_g1_leerPin(G1_PIN_BTN_A, 0)) _g1_confirmarCanasta(0);
    if (_g1_leerPin(G1_PIN_BTN_B, 1)) _g1_confirmarCanasta(1);
  }
}

/*
 * NUEVA FUNCIÓN: Notifica que el sensor físico detectó una canasta
 * Llamada desde marcador.ino cuando sensor_tick() retorna true
 */
void game1vs1_notificarCanasta() {
  if (!_g1_activo || _g1_pausado || _g1_terminado) return;
  if (_g1_estado == G1_ESPERANDO_CANASTA) {
    _g1_canastaDetectada();
  }
}

String game1vs1_statusJSON() {
  unsigned long restMs = _g1_restanteMs();
  String estado;
  if      (_g1_terminado) estado = "Fin";
  else if (_g1_pausado)   estado = "Pausado";
  else if (_g1_activo)    estado = "En juego";
  else                    estado = "Esperando";

  String j = "";
  j += "\"score1\":"   + String(_g1_puntos[0])      + ",";
  j += "\"score2\":"   + String(_g1_puntos[1])      + ",";
  j += "\"seconds\":"  + String(restMs / 1000)      + ",";
  j += "\"state\":\""  + estado                     + "\"";
  return j;
}

void game1vs1_puntoManual(int player) {
  if (!_g1_activo || _g1_pausado || _g1_terminado) return;
  int idx = (player == 1) ? 0 : 1;
  _g1_puntos[idx] += 2;
  _g1_apagarLeds();
  digitalWrite(idx == 0 ? G1_PIN_LED_A : G1_PIN_LED_B, HIGH);
  _g1_ledDestelloEquipo = idx;
  _g1_ledApagarTs       = millis() + G1_LED_DESTELLO;
  Serial.printf("[APP] Punto manual J%d -> A:%d B:%d\n",
                player, _g1_puntos[0], _g1_puntos[1]);
}

void game1vs1_setPausa(bool pausar) {
  if (!_g1_activo || _g1_terminado) return;
  if (pausar && !_g1_pausado) {
    _g1_tiempoRestanteMs = _g1_restanteMs();
    _g1_pausado = true;
    _g1_apagarLeds();
    Serial.println("[PAUSA] Partido pausado.");
  } else if (!pausar && _g1_pausado) {
    _g1_tiempoFinMs = millis() + _g1_tiempoRestanteMs;
    _g1_pausado = false;
    _g1_ultimoMarcadorSerial = millis();
    Serial.println("[PAUSA] Partido reanudado.");
  }
}

void game1vs1_reset() {
  game1vs1_iniciar(_g1_duracionMs, _g1_ventanaMs);
  Serial.println("[RESET] Partido reiniciado.");
}

void game1vs1_stop() {
  if (_g1_activo) {
    _g1_tiempoFinMs = millis();
    Serial.println("[STOP] Partido detenido manualmente.");
  }
}

void game1vs1_printSerial() {
  unsigned long restMs = _g1_restanteMs();
  unsigned int  min    = (restMs / 1000) / 60;
  unsigned int  s      = (restMs / 1000) % 60;
  Serial.printf("[%02d:%02d]  A: %d pts  |  B: %d pts\n",
                min, s, _g1_puntos[0], _g1_puntos[1]);
}

// ─── FUNCIONES AUXILIARES PARA MARCADOR.INO ──────────────────────────────────
unsigned long game1vs1_getTiempoRestante() { return _g1_restanteMs(); }
bool game1vs1_estaActivo()   { return _g1_activo; }
bool game1vs1_estaPausado()  { return _g1_pausado; }
bool game1vs1_haTerminado()  { return _g1_terminado; }
void game1vs1_getPuntos(int &ptsA, int &ptsB) {
  ptsA = _g1_puntos[0];
  ptsB = _g1_puntos[1];
}

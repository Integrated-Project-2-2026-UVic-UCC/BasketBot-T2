#pragma once

/*
 * game_libre.h — Modo libre: marcador único, canastas de +2/-2 sin límite
 *
 * ⚠️ MODIFICADO: Ya NO lee sensor físico directamente.
 *    Las canastas se notifican desde marcador.ino mediante gameLibre_notificarCanasta()
 *
 * Hardware:
 *   GPIO 21 → LED (destello breve al anotar)
 *
 * El sensor físico suma +2 igual que el botón de la app.
 * El botón "Deshacer" de la app resta -2.
 */

#define GL_PIN_LED      21

static bool          _gl_activo        = false;
static bool          _gl_pausado       = false;
static int           _gl_puntos        = 0;

// LED: destello 200 ms al anotar
static unsigned long _gl_ledApagar     = 0;

// ─── HELPERS ─────────────────────────────────────────────────────────────────
static void _gl_setLED(bool on) { digitalWrite(GL_PIN_LED, on ? HIGH : LOW); }

static void _gl_anotar() {
  _gl_puntos += 2;
  _gl_setLED(true);
  _gl_ledApagar = millis() + 200;
  Serial.printf("[LIBRE] Canasta! +2 → %d pts\n", _gl_puntos);
}

// ─── API PÚBLICA ─────────────────────────────────────────────────────────────
void gameLibre_iniciar() {
  pinMode(GL_PIN_LED, OUTPUT);
  _gl_setLED(false);

  _gl_activo        = true;
  _gl_pausado       = false;
  _gl_puntos        = 0;
  _gl_ledApagar     = 0;

  Serial.println("=== MODO LIBRE INICIADO ===");
}

void gameLibre_tick() {
  if (!_gl_activo || _gl_pausado) return;

  // Apagar LED tras el destello
  if (_gl_ledApagar > 0 && millis() >= _gl_ledApagar) {
    _gl_setLED(false);
    _gl_ledApagar = 0;
  }
}

/*
 * NUEVA FUNCIÓN: Notifica que el sensor físico detectó una canasta
 * Llamada desde marcador.ino cuando sensor_tick() retorna true
 */
void gameLibre_notificarCanasta() {
  if (!_gl_activo || _gl_pausado) return;
  _gl_anotar();
}

String gameLibre_statusJSON() {
  String st = _gl_pausado ? "Pausado" : (_gl_activo ? "En juego" : "Esperando");
  String j  = "";
  j += "\"mode\":\"libre\",";
  j += "\"state\":\"" + st + "\",";
  j += "\"puntos\":" + String(_gl_puntos);
  return j;
}

// +2 desde la app
void gameLibre_sumar() {
  if (!_gl_activo || _gl_pausado) return;
  _gl_anotar();
}

// -2 desde la app (deshacer)
void gameLibre_restar() {
  if (!_gl_activo || _gl_pausado) return;
  _gl_puntos = max(0, _gl_puntos - 2);
  Serial.printf("[LIBRE] Deshacer → %d pts\n", _gl_puntos);
}

void gameLibre_setPausa(bool pausar) {
  _gl_pausado = pausar;
  if (pausar) _gl_setLED(false);
  Serial.printf("[LIBRE] %s\n", pausar ? "Pausado" : "Reanudado");
}

void gameLibre_reset() {
  _gl_puntos        = 0;
  _gl_pausado       = false;
  _gl_ledApagar     = 0;
  _gl_setLED(false);
  Serial.println("[LIBRE] Reset.");
}

void gameLibre_stop() {
  _gl_activo  = false;
  _gl_pausado = false;
  _gl_setLED(false);
  Serial.println("[LIBRE] Detenido.");
}

// ─── FUNCIONES AUXILIARES PARA MARCADOR.INO ──────────────────────────────────
bool gameLibre_estaActivo()  { return _gl_activo; }
bool gameLibre_estaPausado() { return _gl_pausado; }
int  gameLibre_getPuntos()   { return _gl_puntos; }

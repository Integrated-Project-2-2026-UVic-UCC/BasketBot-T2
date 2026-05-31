#pragma once

/*
 * display_timer.h — Display TM1637 de TIEMPO para BasketBot
 *   CLK → GPIO 26  |  DIO → GPIO 4
 *
 * ⚠️ PINES ACTUALIZADOS para evitar conflicto con sensor_canasta.h
 *    (Anteriormente: CLK=13, DIO=12 — ahora usados por el sensor IR)
 *
 * Qué muestra según el modo:
 *   MODO_1VS1     → cuenta atrás  MM:SS  (ej. 05:23)
 *   MODO_SHOOTOUT → tiempo turno  SS     (ej. 0017)
 *   MODO_21       → ventana tiro  SS     (ej. 0003)
 *   MODO_LIBRE    → "----"  (el tiempo no aplica en modo libre)
 *   Sin juego / fin / pausa → "----"
 *
 * Dependencia: TM1637Display by Avishay Orpaz
 */

#include <TM1637Display.h>

#define TIMER_CLK_PIN   26     // ← ACTUALIZADO (era 13)
#define TIMER_DIO_PIN   4      // ← ACTUALIZADO (era 12)
#define TIMER_BRIGHT     5     // 0–7
#define TIMER_UPDATE_MS 250

static TM1637Display _timer_display(TIMER_CLK_PIN, TIMER_DIO_PIN);
static unsigned long _timer_ultimoUpdate = 0;

static const uint8_t TIMER_GUIONES[] = { SEG_G, SEG_G, SEG_G, SEG_G };

// ─── INIT ─────────────────────────────────────────────────────────────────────
void timer_iniciar() {
  _timer_display.setBrightness(TIMER_BRIGHT);
  _timer_display.setSegments(TIMER_GUIONES);
  Serial.println("[TIMER] Display tiempo iniciado (CLK=26, DIO=4)");
}

// ─── HELPERS ─────────────────────────────────────────────────────────────────
static void _timer_showMMSS(unsigned long totalSeg) {
  if (totalSeg > 5999) totalSeg = 5999;
  int valor = (totalSeg / 60) * 100 + (totalSeg % 60);
  _timer_display.showNumberDecEx(valor, 0b01000000, true); // dos puntos centrales ON
}

static void _timer_showNum(int valor) {
  valor = constrain(valor, 0, 9999);
  _timer_display.showNumberDec(valor, true);
}

static void _timer_showGuiones() {
  _timer_display.setSegments(TIMER_GUIONES);
}

// ─── UPDATE ───────────────────────────────────────────────────────────────────
// Llamar desde loop() de marcador.ino
// 0=NINGUNO 1=MODO_1VS1 2=MODO_21 3=MODO_LIBRE 4=MODO_SHOOTOUT
void timer_update(int modoActual,
                  unsigned long seg1vs1,  bool activo1vs1,  bool pausado1vs1, bool fin1vs1,
                  unsigned long segSO,    bool jugandoSO,   bool pausadoSO,   bool finSO,
                  unsigned long segG21,   bool actG21,      bool pausG21,     bool finG21) {

  if (millis() - _timer_ultimoUpdate < TIMER_UPDATE_MS) return;
  _timer_ultimoUpdate = millis();

  switch (modoActual) {

    case 1: // MODO_1VS1
      (fin1vs1 || pausado1vs1 || !activo1vs1) ? _timer_showGuiones() : _timer_showMMSS(seg1vs1);
      break;

    case 4: // MODO_SHOOTOUT
      (finSO || pausadoSO || !jugandoSO) ? _timer_showGuiones() : _timer_showNum((int)segSO);
      break;

    case 2: // MODO_21
      (finG21 || pausG21 || !actG21) ? _timer_showGuiones() : _timer_showNum((int)segG21);
      break;

    default: // MODO_LIBRE y NINGUNO
      _timer_showGuiones();
      break;
  }
}

#pragma once

/*
 * display_score.h — Display TM1637 de PUNTOS para BasketBot
 *   CLK → GPIO 14  |  DIO → GPIO 27
 *
 * Qué muestra según el modo:
 *   MODO_1VS1     → AB:CD  dígitos izq = puntos Equipo A  |  dígitos dcha = puntos Equipo B
 *                   (cada equipo hasta 99 pts; sin los dos puntos centrales)
 *   MODO_SHOOTOUT → puntos del jugador en turno (0000–9999)
 *   MODO_21       → puntos del jugador en turno (0000–9999)
 *   MODO_LIBRE    → puntos totales              (0000–9999)
 *   Sin juego / fin / pausa → "----"
 *
 * Dependencia: TM1637Display by Avishay Orpaz
 */

#include <TM1637Display.h>

#define SCORE_CLK_PIN   14
#define SCORE_DIO_PIN   27
#define SCORE_BRIGHT     5     // 0–7
#define SCORE_UPDATE_MS 250

static TM1637Display _score_display(SCORE_CLK_PIN, SCORE_DIO_PIN);
static unsigned long _score_ultimoUpdate = 0;

static const uint8_t SCORE_GUIONES[] = { SEG_G, SEG_G, SEG_G, SEG_G };

// ─── INIT ─────────────────────────────────────────────────────────────────────
void score_iniciar() {
  _score_display.setBrightness(SCORE_BRIGHT);
  _score_display.setSegments(SCORE_GUIONES);
  Serial.println("[SCORE] Display puntos iniciado.");
}

// ─── HELPERS ─────────────────────────────────────────────────────────────────
// 1vs1: muestra  AA BB  (dos dígitos izq = equipo A, dos dígitos dcha = equipo B)
// sin los dos puntos centrales para que se lean como dos marcadores separados
static void _score_show1vs1(int ptsA, int ptsB) {
  ptsA = constrain(ptsA, 0, 99);
  ptsB = constrain(ptsB, 0, 99);
  int valor = ptsA * 100 + ptsB;
  // showNumberDecEx con máscara 0 = sin puntos; relleno con ceros
  _score_display.showNumberDecEx(valor, 0b00000000, true);
}

static void _score_showNum(int valor) {
  valor = constrain(valor, 0, 9999);
  _score_display.showNumberDec(valor, true);
}

static void _score_showGuiones() {
  _score_display.setSegments(SCORE_GUIONES);
}

// ─── UPDATE ───────────────────────────────────────────────────────────────────
// Llamar desde loop() de marcador.ino
// 0=NINGUNO 1=MODO_1VS1 2=MODO_21 3=MODO_LIBRE 4=MODO_SHOOTOUT
void score_update(int modoActual,
                  int  pts1A,      int  pts1B,      bool activo1vs1, bool pausado1vs1, bool fin1vs1,
                  int  ptsSO,      bool jugandoSO,  bool finSO,      bool pausadoSO,
                  int  ptsG21,     bool actG21,     bool finG21,     bool pausG21,
                  int  ptsLibre,   bool activoLibre, bool pausLibre) {

  if (millis() - _score_ultimoUpdate < SCORE_UPDATE_MS) return;
  _score_ultimoUpdate = millis();

  switch (modoActual) {

    case 1: // MODO_1VS1
      (fin1vs1 || pausado1vs1 || !activo1vs1)
        ? _score_showGuiones()
        : _score_show1vs1(pts1A, pts1B);
      break;

    case 4: // MODO_SHOOTOUT — puntos del jugador en turno
      (finSO || pausadoSO) ? _score_showGuiones() : _score_showNum(ptsSO);
      break;

    case 2: // MODO_21 — puntos del jugador en turno
      (finG21 || pausG21) ? _score_showGuiones() : _score_showNum(ptsG21);
      break;

    case 3: // MODO_LIBRE — puntos totales
      (!activoLibre || pausLibre) ? _score_showGuiones() : _score_showNum(ptsLibre);
      break;

    default:
      _score_showGuiones();
      break;
  }
}

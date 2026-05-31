/*
 * BasketBot — Archivo principal
 *
 * Archivos del proyecto:
 *   marcador.ino         <- este archivo
 *   sensor_canasta.h     <- Detección IR dual (arriba/abajo)
 *   display_timer.h      <- TM1637 tiempo  (CLK 26 / DIO 4)
 *   display_score.h      <- TM1637 puntos  (CLK 14 / DIO 27)
 *   game_1vs1.h          <- partido equipo A vs B por tiempo
 *   game_21.h            <- juego 21 (1-4 jugadores)
 *   game_libre.h         <- marcador libre (+2 / -2)
 *   game_shootout.h      <- shoot out: máximos puntos por turno
 *   motor_canasta.h      <- Control NEMA 17 altura canasta
 *   webpage.h            <- HTML/CSS/JS de la app
 */

#include <WiFi.h>
#include <WebServer.h>
#include "webpage.h"
#include "sensor_canasta.h"  // ← NUEVO: Sistema de detección centralizado
#include "game_1vs1.h"
#include "game_21.h"
#include "game_libre.h"
#include "game_shootout.h"
#include "display_timer.h"   // TM1637 izquierdo — tiempo
#include "display_score.h"   // TM1637 derecho   — puntos
#include "motor_canasta.h"   // NEMA 17 ajuste altura canasta

const char* AP_SSID = "BasketBot";
const char* AP_PASS = "paytowin";

WebServer server(80);

enum ModoJuego { NINGUNO, MODO_1VS1, MODO_21, MODO_LIBRE, MODO_SHOOTOUT };
ModoJuego modoActual = NINGUNO;

// ─── CALLBACK: CANASTA DETECTADA ─────────────────────────────────────────────
/*
 * Esta función se ejecuta automáticamente cuando el sensor detecta una canasta.
 * NO notifica a los modos de juego aquí — eso se hace en loop() para mantener
 * la lógica centralizada y facilitar el debug.
 */
void onCanastaDetectada() {
  Serial.println("[MAIN] 🏀 Canasta física detectada por sensor");
  // Aquí puedes agregar efectos adicionales:
  // - Reproducir sonido
  // - LED de confirmación
  // - Enviar notificación push a la app
}

// ─── HANDLERS HTTP ────────────────────────────────────────────────────────────

void handleRoot() { server.send_P(200, "text/html", INDEX_HTML); }

void handleStart() {
  String mode    = server.arg("mode");
  int    timeSeg = server.arg("time").toInt();
  int    ventana = server.arg("ventana").toInt();
  int    players = server.arg("players").toInt();

  if (mode == "1vs1") {
    modoActual = MODO_1VS1;
    game1vs1_iniciar((unsigned long)timeSeg * 1000UL, (unsigned long)ventana);
    Serial.printf("[START] 1vs1 %ds ventana %dms\n", timeSeg, ventana);

  } else if (mode == "21") {
    modoActual = MODO_21;
    game21_iniciar(constrain(players, 1, 4), (unsigned long)ventana);
    Serial.printf("[START] 21 %djug ventana %dms\n", players, ventana);

  } else if (mode == "libre") {
    modoActual = MODO_LIBRE;
    gameLibre_iniciar();
    Serial.println("[START] Modo libre");

  } else if (mode == "shootout") {
    modoActual = MODO_SHOOTOUT;
    gameShootout_iniciar(constrain(players, 1, 4), (unsigned long)timeSeg * 1000UL);
    Serial.printf("[START] Shootout %djug %ds/turno\n", players, timeSeg);

  } else {
    server.send(400, "text/plain", "Modo desconocido"); return;
  }
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  String json = "{";
  if      (modoActual == MODO_1VS1)     json += game1vs1_statusJSON();
  else if (modoActual == MODO_21)       json += game21_statusJSON();
  else if (modoActual == MODO_LIBRE)    json += gameLibre_statusJSON();
  else if (modoActual == MODO_SHOOTOUT) json += gameShootout_statusJSON();
  else json += "\"state\":\"Esperando\",\"mode\":\"none\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handlePoint() {
  String action = server.arg("action");
  int    player = server.arg("player").toInt();

  if      (modoActual == MODO_1VS1)                   game1vs1_puntoManual(player);
  else if (modoActual == MODO_21)                     game21_puntoManual(player);
  else if (modoActual == MODO_LIBRE && action=="sub") gameLibre_restar();
  else if (modoActual == MODO_LIBRE)                  gameLibre_sumar();
  else if (modoActual == MODO_SHOOTOUT)               gameShootout_canastaManual();
  server.send(200, "text/plain", "OK");
}

void handleFallo() {
  if (modoActual == MODO_21) game21_falloManual();
  server.send(200, "text/plain", "OK");
}

void handleListo() {
  if (modoActual == MODO_SHOOTOUT) gameShootout_listoManual();
  server.send(200, "text/plain", "OK");
}

void handlePause() {
  bool p = server.arg("state").toInt() == 1;
  if      (modoActual == MODO_1VS1)     game1vs1_setPausa(p);
  else if (modoActual == MODO_21)       game21_setPausa(p);
  else if (modoActual == MODO_LIBRE)    gameLibre_setPausa(p);
  else if (modoActual == MODO_SHOOTOUT) gameShootout_setPausa(p);
  server.send(200, "text/plain", "OK");
}

void handleReset() {
  if      (modoActual == MODO_1VS1)     game1vs1_reset();
  else if (modoActual == MODO_21)       game21_reset();
  else if (modoActual == MODO_LIBRE)    gameLibre_reset();
  else if (modoActual == MODO_SHOOTOUT) gameShootout_reset();
  server.send(200, "text/plain", "OK");
}

// /motor?cm=N  → mover canasta a N cm (0-20), lanza homing si es necesario
void handleMotor() {
  int cm = server.arg("cm").toInt();
  cm = constrain(cm, 0, 20);
  motor_irACm(cm);
  server.send(200, "text/plain", "OK");
}

// /motorstatus → JSON con estado del motor
void handleMotorStatus() {
  String json = "{" + motor_statusJSON() + "}";
  server.send(200, "application/json", json);
}

// /homing → fuerza homing manual desde la app
void handleHoming() {
  motor_homing();
  server.send(200, "text/plain", "OK");
}

// ─── SETUP ───────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== BasketBot ===");

  // Inicializar sensor de canasta (PRIMERO — es crítico)
  sensor_iniciar();
  sensor_setCallback(onCanastaDetectada);

  // Displays TM1637
  timer_iniciar();   // TM1637 tiempo
  score_iniciar();   // TM1637 puntos

  // Motor NEMA 17 — homing automático al arrancar
  motor_iniciar();
  motor_homing();

  // WiFi Access Point
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

  // Servidor web
  server.on("/",        HTTP_GET,  handleRoot);
  server.on("/start",   HTTP_POST, handleStart);
  server.on("/status",  HTTP_GET,  handleStatus);
  server.on("/point",   HTTP_POST, handlePoint);
  server.on("/fallo",   HTTP_POST, handleFallo);
  server.on("/listo",   HTTP_POST, handleListo);
  server.on("/pause",   HTTP_POST, handlePause);
  server.on("/reset",   HTTP_POST, handleReset);
  server.on("/motor",       HTTP_POST, handleMotor);
  server.on("/motorstatus", HTTP_GET,  handleMotorStatus);
  server.on("/homing",      HTTP_POST, handleHoming);
  server.begin();

  Serial.println("http://192.168.4.1");
  Serial.println("Cmds: partido,<s>,<ms> | 21,<jug>,<ms> | libre | shootout,<jug>,<s>");
  Serial.println("      listo | stop | reset | status | stats | reset_stats");
}

// ─── LOOP ────────────────────────────────────────────────────────────────────
void loop() {
  server.handleClient();

  // Comandos por serial
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    procesarComandoSerial(cmd);
  }

  // ══════════════════════════════════════════════════════════════════════════
  // 1. SENSOR DE CANASTA — máxima prioridad
  // ══════════════════════════════════════════════════════════════════════════
  bool canasta = sensor_tick();

  // Notificar al modo de juego activo si hay canasta
  if (canasta) {
    switch (modoActual) {
      
      case MODO_1VS1:
        game1vs1_notificarCanasta();
        break;
      
      case MODO_21:
        game21_notificarCanasta();
        break;
      
      case MODO_LIBRE:
        gameLibre_notificarCanasta();
        break;
      
      case MODO_SHOOTOUT:
        gameShootout_notificarCanasta();
        break;
      
      default:
        // Sin modo activo, solo loggear
        Serial.println("[MAIN] Canasta detectada pero no hay modo activo");
        break;
    }
  }

  // ══════════════════════════════════════════════════════════════════════════
  // 2. MOTOR — no bloqueante
  // ══════════════════════════════════════════════════════════════════════════
  motor_tick();

  // ══════════════════════════════════════════════════════════════════════════
  // 3. LÓGICA DE JUEGO — procesa timeouts, LEDs, etc.
  // ══════════════════════════════════════════════════════════════════════════
  if      (modoActual == MODO_1VS1)     game1vs1_tick();
  else if (modoActual == MODO_21)       game21_tick();
  else if (modoActual == MODO_LIBRE)    gameLibre_tick();
  else if (modoActual == MODO_SHOOTOUT) gameShootout_tick();

  // ══════════════════════════════════════════════════════════════════════════
  // 4. DISPLAYS TM1637
  // ══════════════════════════════════════════════════════════════════════════
  
  // ── Display Timer: tiempo ─────────────────────────────────────────────────
  unsigned long seg1vs1  = game1vs1_getTiempoRestante() / 1000;
  bool activo1vs1        = game1vs1_estaActivo();
  bool pausado1vs1       = game1vs1_estaPausado();
  bool fin1vs1           = game1vs1_haTerminado();

  unsigned long segSO    = 0;
  bool jugandoSO         = gameShootout_estaJugando();
  bool pausadoSO         = gameShootout_estaPausado();
  bool finSO             = gameShootout_haTerminado();
  if (jugandoSO && !pausadoSO) {
    segSO = gameShootout_getTiempoRestante() / 1000;
  }

  unsigned long segG21   = 0;
  bool actG21            = game21_estaEsperando();
  bool pausG21           = game21_estaPausado();
  bool finG21            = game21_haTerminado();
  if (actG21 && !pausG21) {
    segG21 = game21_getTiempoVentana() / 1000;
  }

  timer_update((int)modoActual,
               seg1vs1,  activo1vs1,  pausado1vs1, fin1vs1,
               segSO,    jugandoSO,   pausadoSO,   finSO,
               segG21,   actG21,      pausG21,     finG21);

  // ── Display Score: puntos ─────────────────────────────────────────────────
  int ptsA, ptsB;
  game1vs1_getPuntos(ptsA, ptsB);
  
  score_update((int)modoActual,
               ptsA, ptsB, activo1vs1, pausado1vs1, fin1vs1,
               gameShootout_getPuntosTurno(), jugandoSO, finSO, pausadoSO,
               game21_getPuntosTurno(), actG21, finG21, pausG21,
               gameLibre_getPuntos(), gameLibre_estaActivo(), gameLibre_estaPausado());
}

// ─── COMANDOS SERIAL ─────────────────────────────────────────────────────────
void procesarComandoSerial(String cmd) {
  // ── Comandos del sensor ──────────────────────────────────────────────────
  if (cmd == "stats") {
    Serial.println("\n=== ESTADÍSTICAS SENSOR ===");
    Serial.print(sensor_getEstadisticas());
    Serial.println("===========================\n");
    return;
  }
  if (cmd == "reset_stats") {
    sensor_resetEstadisticas();
    return;
  }
  if (cmd == "estado_sensor") {
    Serial.println("Estado sensor: " + sensor_getEstado());
    return;
  }
  if (cmd == "reset_sensor") {
    sensor_reset();
    return;
  }

  // ── Comandos de juegos ───────────────────────────────────────────────────
  if (cmd.startsWith("partido")) {
    int c1=cmd.indexOf(','), c2=cmd.indexOf(',',c1+1);
    if (c1!=-1&&c2!=-1) {
      unsigned long s=cmd.substring(c1+1,c2).toInt(), v=cmd.substring(c2+1).toInt();
      if (s>0&&v>0) { modoActual=MODO_1VS1; game1vs1_iniciar(s*1000UL,v); }
      else Serial.println("Params invalidos.");
    } else Serial.println("Formato: partido,<s>,<ms>");
    return;
  }
  if (cmd.startsWith("21")) {
    int c1=cmd.indexOf(','), c2=cmd.indexOf(',',c1+1);
    if (c1!=-1&&c2!=-1) {
      int j=cmd.substring(c1+1,c2).toInt(); unsigned long v=cmd.substring(c2+1).toInt();
      if (j>=1&&j<=4&&v>0) { modoActual=MODO_21; game21_iniciar(j,v); }
      else Serial.println("Jugadores: 1-4");
    } else Serial.println("Formato: 21,<jug>,<ms>");
    return;
  }
  if (cmd.startsWith("shootout")) {
    int c1=cmd.indexOf(','), c2=cmd.indexOf(',',c1+1);
    if (c1!=-1&&c2!=-1) {
      int j=cmd.substring(c1+1,c2).toInt(); unsigned long s=cmd.substring(c2+1).toInt();
      if (j>=1&&j<=4&&s>0) { modoActual=MODO_SHOOTOUT; gameShootout_iniciar(j,s*1000UL); }
      else Serial.println("Jugadores: 1-4, tiempo > 0");
    } else Serial.println("Formato: shootout,<jug>,<seg>");
    return;
  }
  if (cmd=="libre")  { modoActual=MODO_LIBRE; gameLibre_iniciar(); return; }
  if (cmd=="listo" && modoActual==MODO_SHOOTOUT) { gameShootout_listoManual(); return; }
  if (cmd=="stop") {
    if      (modoActual==MODO_1VS1)     game1vs1_stop();
    else if (modoActual==MODO_21)       game21_stop();
    else if (modoActual==MODO_LIBRE)    gameLibre_stop();
    else if (modoActual==MODO_SHOOTOUT) gameShootout_stop();
    return;
  }
  if (cmd=="reset") {
    if      (modoActual==MODO_1VS1)     game1vs1_reset();
    else if (modoActual==MODO_21)       game21_reset();
    else if (modoActual==MODO_LIBRE)    gameLibre_reset();
    else if (modoActual==MODO_SHOOTOUT) gameShootout_reset();
    return;
  }
  if (cmd=="status") {
    if      (modoActual==MODO_1VS1)     game1vs1_printSerial();
    else if (modoActual==MODO_21)       game21_printSerial();
    else if (modoActual==MODO_SHOOTOUT) gameShootout_printSerial();
    else Serial.println("Sin partido activo.");
    return;
  }
  
  Serial.println("\n=== COMANDOS DISPONIBLES ===");
  Serial.println("JUEGOS:");
  Serial.println("  partido,<s>,<ms>      - Iniciar 1vs1");
  Serial.println("  21,<jug>,<ms>         - Iniciar juego 21");
  Serial.println("  shootout,<jug>,<seg>  - Iniciar shootout");
  Serial.println("  libre                 - Iniciar modo libre");
  Serial.println("  listo                 - Jugador listo (shootout)");
  Serial.println("  stop                  - Detener juego");
  Serial.println("  reset                 - Reiniciar juego");
  Serial.println("  status                - Ver estado");
  Serial.println("\nSENSOR:");
  Serial.println("  stats                 - Estadísticas sensor");
  Serial.println("  reset_stats           - Resetear estadísticas");
  Serial.println("  estado_sensor         - Estado actual sensor");
  Serial.println("  reset_sensor          - Reset manual sensor");
  Serial.println("============================\n");
}

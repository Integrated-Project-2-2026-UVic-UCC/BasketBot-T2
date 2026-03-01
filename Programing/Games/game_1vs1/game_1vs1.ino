// =============================================
//  BasketBot - Partido 1v1 / Equipo vs Equipo
//  Modo simulación: todo se controla por Serial
//
//  Comandos durante el partido:
//    canasta  → simula que el sensor detecta una canasta
//    A        → equipo A confirma (tras canasta detectada)
//    B        → equipo B confirma (tras canasta detectada)
//    stop     → termina el partido manualmente
//
//  Para iniciar:
//    partido,<segundos>,<ventanaPulsadorMs>
//    Ejemplo: partido,120,3000
// =============================================

#define MODO_SIMULACION true  // false para hardware real

#define PIN_SENSOR  2
#define PIN_BTN_A   3
#define PIN_BTN_B   4
#define PIN_LED     13

// =============================================
//  Estado global del partido
// =============================================
bool     partidoActivo    = false;
int      puntos[2]        = {0, 0};
unsigned long tiempoFinMs = 0;
unsigned long ventanaMs   = 0;

// Estados internos
enum EstadoJuego { ESPERANDO_CANASTA, ESPERANDO_PULSADOR };
EstadoJuego estadoActual = ESPERANDO_CANASTA;

unsigned long inicioVentana   = 0;
unsigned long ultimoMarcador  = 0;  // para imprimir el marcador periódicamente

// =============================================
//  Hardware (ignorado en modo simulación)
// =============================================
bool leerSensor() {
  if (MODO_SIMULACION) return false; // en simulación se dispara por Serial
  return digitalRead(PIN_SENSOR) == HIGH;
}

bool leerBotonA() {
  if (MODO_SIMULACION) return false;
  return digitalRead(PIN_BTN_A) == LOW;
}

bool leerBotonB() {
  if (MODO_SIMULACION) return false;
  return digitalRead(PIN_BTN_B) == LOW;
}

void setLED(bool estado) {
  if (!MODO_SIMULACION) digitalWrite(PIN_LED, estado ? HIGH : LOW);
  // En simulación imprimimos el estado del LED
  Serial.print("[LED ");
  Serial.print(estado ? "ON" : "OFF");
  Serial.println("]");
}

// =============================================
//  Imprimir marcador con tiempo restante
// =============================================
void imprimirMarcador() {
  if (!partidoActivo) return;

  unsigned long restanteMs = (millis() < tiempoFinMs) ? (tiempoFinMs - millis()) : 0;
  unsigned int  min = (restanteMs / 1000) / 60;
  unsigned int  s   = (restanteMs / 1000) % 60;

  Serial.println("----------------------------");
  Serial.print("Tiempo restante: ");
  if (min < 10) Serial.print("0");
  Serial.print(min); Serial.print(":");
  if (s < 10) Serial.print("0");
  Serial.println(s);
  Serial.print("  A: "); Serial.print(puntos[0]);
  Serial.print(" pts  |  B: "); Serial.print(puntos[1]);
  Serial.println(" pts");
}

// =============================================
//  Fin del partido
// =============================================
void finPartido() {
  partidoActivo = false;
  setLED(false);

  Serial.println("============================");
  Serial.println("*** TIEMPO AGOTADO ***");
  Serial.print("  Equipo A: "); Serial.print(puntos[0]); Serial.println(" pts");
  Serial.print("  Equipo B: "); Serial.print(puntos[1]); Serial.println(" pts");

  if      (puntos[0] > puntos[1]) Serial.println("*** GANADOR: Equipo A ***");
  else if (puntos[1] > puntos[0]) Serial.println("*** GANADOR: Equipo B ***");
  else                             Serial.println("*** EMPATE ***");

  Serial.println("============================");
  Serial.println("Envía: partido,<segundos>,<ventanaPulsadorMs> para nueva partida.");
}

// =============================================
//  Iniciar partido
// =============================================
void iniciarPartido(unsigned long duracionMs, unsigned long ventanaPulsadorMs) {
  puntos[0]      = 0;
  puntos[1]      = 0;
  tiempoFinMs    = millis() + duracionMs;
  ventanaMs      = ventanaPulsadorMs;
  estadoActual   = ESPERANDO_CANASTA;
  partidoActivo  = true;
  ultimoMarcador = millis();

  Serial.println("=== PARTIDO INICIADO ===");
  Serial.print("Duración: "); Serial.print(duracionMs / 1000); Serial.println(" s");
  Serial.print("Ventana pulsador: "); Serial.print(ventanaMs / 1000); Serial.println(" s");
  Serial.println("Comandos: 'canasta', 'A', 'B', 'stop'");
  Serial.println("========================");
  imprimirMarcador();
  Serial.println("Esperando canasta...");
}

// =============================================
//  Procesar evento de canasta detectada
// =============================================
void canasta_detectada() {
  Serial.println(">> CANASTA detectada! Pulsad A o B para confirmar...");
  setLED(true);
  estadoActual  = ESPERANDO_PULSADOR;
  inicioVentana = millis();
}

// =============================================
//  Procesar pulsador de equipo
// =============================================
void confirmarCanasta(int equipo) {
  const int PUNTOS_CANASTA = 2;
  const char* nombres[2] = {"Equipo A", "Equipo B"};

  setLED(false);
  estadoActual = ESPERANDO_CANASTA;

  puntos[equipo] += PUNTOS_CANASTA;
  Serial.print(">> ");
  Serial.print(nombres[equipo]);
  Serial.print(" anota! +");
  Serial.print(PUNTOS_CANASTA);
  Serial.print(" pts → Total: ");
  Serial.println(puntos[equipo]);

  imprimirMarcador();
  Serial.println("Esperando canasta...");
}

// =============================================
//  Loop principal del partido (no bloqueante)
// =============================================
void tickPartido() {
  if (!partidoActivo) return;

  // ¿Se acabó el tiempo?
  if (millis() >= tiempoFinMs) {
    finPartido();
    return;
  }

  // Imprimir marcador cada 10 segundos
  if (millis() - ultimoMarcador >= 10000) {
    ultimoMarcador = millis();
    imprimirMarcador();
  }

  // Comprobar hardware real (ignorado en simulación)
  if (estadoActual == ESPERANDO_CANASTA) {
    if (leerSensor()) canasta_detectada();

  } else if (estadoActual == ESPERANDO_PULSADOR) {
    // ¿Expiró la ventana?
    if (millis() - inicioVentana >= ventanaMs) {
      setLED(false);
      estadoActual = ESPERANDO_CANASTA;
      Serial.println(">> Tiempo agotado. Canasta anulada.");
      Serial.println("Esperando canasta...");
      return;
    }
    if (leerBotonA()) confirmarCanasta(0);
    if (leerBotonB()) confirmarCanasta(1);
  }
}

// =============================================
//  Parsear comando Serial
// =============================================
void procesarComando(String cmd) {
  cmd.trim();

  // --- Iniciar partido ---
  if (cmd.startsWith("partido")) {
    int c1 = cmd.indexOf(',');
    int c2 = cmd.indexOf(',', c1 + 1);
    if (c1 != -1 && c2 != -1) {
      unsigned long seg       = cmd.substring(c1 + 1, c2).toInt();
      unsigned long ventana   = cmd.substring(c2 + 1).toInt();
      if (seg > 0 && ventana > 0) {
        iniciarPartido(seg * 1000UL, ventana);
      } else {
        Serial.println("Parámetros inválidos.");
      }
    } else {
      Serial.println("Formato: partido,<segundos>,<ventanaPulsadorMs>");
    }
    return;
  }

  // --- Comandos durante el partido ---
  if (!partidoActivo) {
    Serial.println("No hay partido activo. Envía: partido,<seg>,<ventanaMs>");
    return;
  }

  if (cmd == "stop") {
    Serial.println(">> Partido detenido manualmente.");
    tiempoFinMs = millis(); // fuerza fin en el próximo tick
    return;
  }

  if (cmd == "canasta") {
    if (estadoActual == ESPERANDO_CANASTA) {
      canasta_detectada();
    } else {
      Serial.println("Ya hay una canasta pendiente de confirmar.");
    }
    return;
  }

  if (cmd == "A" || cmd == "a") {
    if (estadoActual == ESPERANDO_PULSADOR) {
      confirmarCanasta(0);
    } else {
      Serial.println("No hay canasta pendiente de confirmar.");
    }
    return;
  }

  if (cmd == "B" || cmd == "b") {
    if (estadoActual == ESPERANDO_PULSADOR) {
      confirmarCanasta(1);
    } else {
      Serial.println("No hay canasta pendiente de confirmar.");
    }
    return;
  }

  Serial.println("Comando desconocido. Usa: canasta, A, B, stop");
}

// =============================================
//  Setup & Loop
// =============================================
void setup() {
  Serial.begin(9600);

  if (!MODO_SIMULACION) {
    pinMode(PIN_SENSOR, INPUT);
    pinMode(PIN_BTN_A,  INPUT_PULLUP);
    pinMode(PIN_BTN_B,  INPUT_PULLUP);
    pinMode(PIN_LED,    OUTPUT);
    digitalWrite(PIN_LED, LOW);
  }

  Serial.println("BasketBot listo.");
  Serial.println("Envía: partido,<segundos>,<ventanaPulsadorMs>");
  Serial.println("Ejemplo: partido,120,3000");
}

void loop() {
  // Leer Serial
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    procesarComando(cmd);
  }

  // Tick del partido (no bloqueante)
  tickPartido();
}

// =============================================
//  BasketBot - Juego 21
//  Función principal: juego21(numJugadores, tiempoEspera)
//  numJugadores : int (2..N)
//  tiempoEspera : unsigned long (milisegundos)
// =============================================

// --- Simulación del sensor láser ---
// Reemplaza esta función con la lectura real de tu sensor
bool sensorCanasta() {
  static int contador = 0;
  static unsigned long ultimaVez = 0;

  // Solo avanza el contador cada 500ms
  if (millis() - ultimaVez < 500) return false;
  ultimaVez = millis();

  contador++;
  Serial.print("  [sensor tick: "); Serial.print(contador); Serial.println("]");

  // Canasta en los ticks: 1, 4, 7... (cada 3 ticks)
  // Fallo en los ticks:   2, 3, 5, 6...
  if (contador % 3 == 1) {
    return true;
  }
  return false;
}
// --- Espera un evento del sensor dentro del tiempo límite ---
// Devuelve: 1 = canasta, 0 = fallo/timeout
int esperarTiro(unsigned long tiempoEspera) {
  unsigned long inicio = millis();
  while (millis() - inicio < tiempoEspera) {
    if (sensorCanasta()) {
      delay(300); // debounce
      return 1;   // canasta
    }
  }
  return 0; // timeout = fallo
}

// --- Juego 21 ---
void juego21(int numJugadores, unsigned long tiempoEspera) {

  const int PUNTOS_VICTORIA     = 21;
  const int PUNTOS_PENALIZACION = 11;

  int puntos[8] = {0}; // máx 8 jugadores

  int  turnoActual = 0;
  bool tiroLibre   = false;
  int  ganador     = -1;

  Serial.println("=== JUEGO 21 INICIADO ===");
  Serial.print("Jugadores: ");        Serial.println(numJugadores);
  Serial.print("Tiempo por tiro (ms): "); Serial.println(tiempoEspera);

  while (ganador == -1) {

    // --- Mostrar estado ---
    Serial.println("-----------------------------");
    Serial.print("Turno: Jugador "); Serial.println(turnoActual + 1);
    for (int i = 0; i < numJugadores; i++) {
      Serial.print("  J"); Serial.print(i + 1);
      Serial.print(": "); Serial.print(puntos[i]); Serial.println(" pts");
    }
    if (tiroLibre) Serial.println("[TIRO LIBRE]");
    else           Serial.println("[TIRO NORMAL]");
    Serial.println("Esperando tiro...");

    // --- Leer sensor ---
    int resultado = esperarTiro(tiempoEspera);

    // --- Lógica de puntuación ---
    if (!tiroLibre) {
      // Tiro normal
      if (resultado == 1) {
        puntos[turnoActual] += 2;
        Serial.println(">> CANASTA! +2 puntos. Tiro libre concedido.");

        if (puntos[turnoActual] > PUNTOS_VICTORIA) {
          puntos[turnoActual] = PUNTOS_PENALIZACION;
          Serial.println(">> PASADO de 21! Puntos bajan a 11. Turno perdido.");
          turnoActual = (turnoActual + 1) % numJugadores;
        } else if (puntos[turnoActual] == PUNTOS_VICTORIA) {
          ganador = turnoActual;
        } else {
          tiroLibre = true;
        }

      } else {
        Serial.println(">> FALLO. Turno perdido.");
        turnoActual = (turnoActual + 1) % numJugadores;
      }

    } else {
      // Tiro libre — sigue tirando hasta que falle
      if (resultado == 1) {
        puntos[turnoActual] += 1;
        Serial.println(">> TIRO LIBRE dentro! +1 punto. Sigue tirando...");

        if (puntos[turnoActual] > PUNTOS_VICTORIA) {
          puntos[turnoActual] = PUNTOS_PENALIZACION;
          Serial.println(">> PASADO de 21! Puntos bajan a 11. Turno perdido.");
          tiroLibre = false;
          turnoActual = (turnoActual + 1) % numJugadores;
        } else if (puntos[turnoActual] == PUNTOS_VICTORIA) {
          ganador = turnoActual;
          tiroLibre = false;
        }
        // Si sigue en juego: tiroLibre permanece true, no cambia turno

      } else {
        Serial.println(">> TIRO LIBRE fallado. Cambio de turno.");
        tiroLibre = false;
        turnoActual = (turnoActual + 1) % numJugadores;
      }
    }

  } // fin while

  // --- Fin de partida ---
  Serial.println("=============================");
  Serial.print("*** GANADOR: Jugador ");
  Serial.print(ganador + 1);
  Serial.println(" con 21 puntos exactos! ***");
  Serial.println("=============================");
}

// =============================================
//  Setup & Loop — invocación por Serial
// =============================================
void setup() {
  Serial.begin(9600);
  Serial.println("BasketBot listo.");
  Serial.println("Envía: 21,<numJugadores>,<tiempoEsperaMs>");
  Serial.println("Ejemplo: 21,2,10000");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    // Parsear: "21,2,10000"
    int c1 = cmd.indexOf(',');
    int c2 = cmd.indexOf(',', c1 + 1);

    if (c1 != -1 && c2 != -1) {
      int           juego        = cmd.substring(0, c1).toInt();
      int           numJugadores = cmd.substring(c1 + 1, c2).toInt();
      unsigned long tms          = cmd.substring(c2 + 1).toInt();

      if (juego == 21 && numJugadores >= 2 && tms > 0) {
        juego21(numJugadores, tms);
      } else {
        Serial.println("Parámetros inválidos.");
      }
    } else {
      Serial.println("Formato: 21,<jugadores>,<tiempoMs>");
    }
  }
}

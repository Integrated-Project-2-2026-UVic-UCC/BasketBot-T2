// Prueba en Tinkercad - Monitor Serial
// 'c' = canasta | 'r' = listo

void setup() {
  Serial.begin(9600);

  // Pedir numero de jugadores
  int jugadores = pedirNumero("Cuantos jugadores? (1-4): ", 1, 4);

  // Pedir nombre de cada jugador
  String nombres[4];
  for (int i = 0; i < jugadores; i++) {
    Serial.print("Nombre jugador "); Serial.print(i+1); Serial.println(": ");
    nombres[i] = pedirTexto();
  }

  // Pedir tiempo
  int tiempo = pedirNumero("Tiempo de juego en segundos: ", 5, 300);

  juegoShootOut(jugadores, tiempo, nombres);
}

void loop() {}

// ── Leer un numero por Serial dentro de un rango ──
int pedirNumero(String mensaje, int minVal, int maxVal) {
  while (true) {
    Serial.print(mensaje);
    while (!Serial.available());
    int valor = Serial.parseInt();
    Serial.println(valor);
    if (valor >= minVal && valor <= maxVal) return valor;
    Serial.print("Valor incorrecto, introduce entre ");
    Serial.print(minVal); Serial.print(" y "); Serial.println(maxVal);
  }
}

// ── Leer un texto por Serial ──────────────────────
String pedirTexto() {
  while (!Serial.available());
  String texto = Serial.readStringUntil('\n');
  texto.trim();
  return texto;
}

// ── Juego Shoot Out ───────────────────────────────
void juegoShootOut(int jugadores, int tiempo, String nombres[]) {
  int puntos[4] = {0, 0, 0, 0};

  for (int turno = 0; turno < jugadores; turno++) {

    // Esperar a que el jugador este listo
    Serial.print(nombres[turno]); Serial.println(", escribe 'r' cuando estes listo...");
    while (true) {
      if (Serial.available() && Serial.read() == 'r') break;
    }

    // Cuenta atras
    for (int i = 3; i > 0; i--) {
      Serial.println(i);
      delay(1000);
    }
    Serial.println("YA!");

    // Turno
    int tRestante = tiempo;
    unsigned long tAnterior = millis();
    unsigned long tRebote   = 0;

    while (tRestante > 0) {

      if (millis() - tAnterior >= 1000) {
        tAnterior = millis();
        tRestante--;
        Serial.print("Tiempo: "); Serial.print(tRestante);
        Serial.print("s | Puntos: "); Serial.println(puntos[turno]);
      }

      if (Serial.available()) {
        char c = Serial.read();
        if (c == 'c' && millis() - tRebote > 300) {
          tRebote = millis();
          puntos[turno]++;
          Serial.print("CANASTA! -> "); Serial.println(puntos[turno]);
        }
      }
    }

    Serial.print("Fin turno "); Serial.print(nombres[turno]);
    Serial.print(" -> "); Serial.print(puntos[turno]); Serial.println(" puntos");
  }

  // Resultado final
  Serial.println("--- FIN DEL JUEGO ---");
  int ganador = 0;
  for (int i = 0; i < jugadores; i++) {
    Serial.print(nombres[i]); Serial.print(": "); Serial.println(puntos[i]);
    if (puntos[i] > puntos[ganador]) ganador = i;
  }
  if (jugadores > 1) {
    Serial.print("Ganador: "); Serial.println(nombres[ganador]);
  }
}

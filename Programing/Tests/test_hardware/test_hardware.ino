/*
 * test_hardware.ino — Comprobación LEDs y botones
 *
 * LEDs:
 *   GPIO 21 → Verde
 *   GPIO 19 → Azul
 *   GPIO 18 → Rojo
 *
 * Botones (INPUT_PULLUP, activo LOW):
 *   GPIO 16 → Botón A
 *   GPIO 17 → Botón B
 *
 * Comportamiento:
 *   - Al arrancar: parpadean los 3 LEDs en secuencia (test visual)
 *   - Botón A (GPIO 16) → enciende LED Verde
 *   - Botón B (GPIO 17) → enciende LED Azul
 *   - Ambos a la vez   → enciende LED Rojo
 *   - Sin pulsar nada  → todos apagados
 *   - Serial 115200: muestra eventos
 */

#define PIN_LED_V  21
#define PIN_LED_A  19
#define PIN_LED_B  18
#define PIN_BTN_A  16
#define PIN_BTN_B  17

void setup() {

  pinMode(PIN_LED_V, OUTPUT);
  pinMode(PIN_LED_A, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_BTN_A, INPUT_PULLUP);
  pinMode(PIN_BTN_B, INPUT_PULLUP);

  
}

void loop() {

  digitalWrite(PIN_LED_V, HIGH);
  digitalWrite(PIN_LED_A, HIGH);
  digitalWrite(PIN_LED_B, HIGH);
}

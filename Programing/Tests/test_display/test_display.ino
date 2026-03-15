#include <TM1637Display.h>

// Define los pios de conexión (cámbialos si usaste otros)
#define CLK 13
#define DIO 12

// Crea un objeto para el display
TM1637Display display = TM1637Display(CLK, DIO);

// Variable para llevar la cuenta del tiempo (en segundos)
int counter = 0;

void setup() {
  Serial.begin(115200); // Para depuración

  // Establece el brillo del display (0 = mín, 7 = máx)
  display.setBrightness(7);
  
  // Limpia el display por si acaso
  display.clear();
  
  Serial.println("Iniciando marcador de tiempo...");
}

void loop() {
  // Muestra el valor actual del contador en el display
  // La función showNumberDec() se encarga de formatear el número
  display.showNumberDec(counter, true); // 'false' para no rellenar con ceros a la izquierda

  // Incrementa el contador y lo muestra por el monitor serie
  Serial.print("Tiempo: ");
  Serial.println(counter);
  counter++;

  // Espera 1 segundo (1000 milisegundos)
  delay(1000);
}

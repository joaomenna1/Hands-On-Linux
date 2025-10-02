const int ledPin = 22;  // Define o pino do LED
const int blinkCount = 20;  // Número de vezes que o LED irá piscar

void setup() {
  pinMode(ledPin, OUTPUT);  // Configura o pino do LED como OUTPUT
}

void loop() {
  for (int i = 0; i < blinkCount; i++) {
    digitalWrite(ledPin, HIGH);  // Liga o LED
    delay(2000);                  // Espera 2 segundos
    digitalWrite(ledPin, LOW);   // Desliga o LED
    delay(2000);                  // Espera mais 2 segundos
  }

  // Após piscar, garante que o LED permaneça desligado
  digitalWrite(ledPin, LOW);
  while (true);  // Mantém o loop travado para que nada mais aconteça
}

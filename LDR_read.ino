// Definindo o pino do LDR
const int LDR_PIN = 34;

void setup() {
  // Inicializa comunicação serial
  Serial.begin(115200);
}

void loop() {
  // Lê o valor do pino ADC
  int ldrValue = analogRead(LDR_PIN);
  
  // Imprime o valor lido no Monitor Serial
  Serial.println(ldrValue);
  
  // Adiciona um pequeno atraso
  delay(500); // 500 ms de atraso para facilitar a leitura
}

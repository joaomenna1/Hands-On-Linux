const int LDR_PIN = 34;     // Pino do LDR
const int LED_PIN = 22;     // Pino do LED

unsigned long lastLdrSendTime = 0;   // Controle do tempo para envio automático
const unsigned long ldrInterval = 2000;  // Intervalo de 2 segundos

bool ledState = false;  // Estado atual do LED (false = OFF, true = ON)

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Garante que o LED começa desligado
}

void loop() {
  // --- Processar comandos da Serial ---
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();  // Remove espaços e quebras de linha extras

    if (comando == "ON") {
      digitalWrite(LED_PIN, HIGH);
      ledState = true;
      Serial.println("RES SET_LED ON");
    }
    else if (comando == "OFF") {
      digitalWrite(LED_PIN, LOW);
      ledState = false;
      Serial.println("RES SET_LED OFF");
    }
    else if (comando == "GET_LED") {
      Serial.print("RES GET_LED ");
      Serial.println(ledState ? "ON" : "OFF");
    }
    else if (comando == "GET_LDR") {
      int ldrValue = analogRead(LDR_PIN);
      Serial.print("RES GET_LDR ");
      Serial.println(ldrValue);
    }
    else {
      Serial.println("RES ERROR Unknown command");
    }
  }

  // --- Envio automático do valor do LDR a cada 2 segundos ---
  unsigned long currentTime = millis();
  if (currentTime - lastLdrSendTime >= ldrInterval) {
    int ldrValue = analogRead(LDR_PIN);
    Serial.print("RES GET_LDR ");
    Serial.println(ldrValue);
    lastLdrSendTime = currentTime;
  }
}

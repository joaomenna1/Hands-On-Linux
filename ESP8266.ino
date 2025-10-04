const int LDR_PIN = A0;     // Pino do LDR (No ESP8266, o pino analógico é A0)
const int LED_PIN = D4;     // Pino do LED (PWM)

// Configurações
unsigned long lastLdrSendTime = 0;
const unsigned long ldrInterval = 2000;  // 2 segundos

// Estados do sistema
int ledIntensity = 0;     // Intensidade do LED (0-100)
int threshold = 50;       // Limiar para ativação automática (0-100)

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0); // LED começa desligado
}

void loop() {
  // --- Processar comandos Serial ---
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    // --- SET_LED X ---
    if (comando.startsWith("SET_LED ")) {
      int valor = comando.substring(8).toInt();
      if (valor >= 0 && valor <= 100) {
        ledIntensity = valor;
        analogWrite(LED_PIN, map(ledIntensity, 0, 100, 0, 255));
        Serial.println("RES SET_LED 1");
      } else {
        Serial.println("RES SET_LED -1");
      }
    }

    // --- GET_LED ---
    else if (comando == "GET_LED") {
      Serial.print("RES GET_LED ");
      Serial.println(ledIntensity);
    }

    // --- GET_LDR ---
    else if (comando == "GET_LDR") {
      int ldrValue = analogRead(LDR_PIN);  // Lê o valor do LDR (A0)
      int ldrNormalized = map(ldrValue, 0, 1023, 0, 100);  // Normaliza para 0-100
      Serial.print("RES GET_LDR ");
      Serial.println(ldrNormalized);
    }

    // --- SET_THRESHOLD X ---
    else if (comando.startsWith("SET_THRESHOLD ")) {
      int valor = comando.substring(14).toInt();
      if (valor >= 0 && valor <= 100) {
        threshold = valor;
        Serial.println("RES SET_THRESHOLD 1");
      } else {
        Serial.println("RES SET_THRESHOLD -1");
      }
    }

    // --- GET_THRESHOLD ---
    else if (comando == "GET_THRESHOLD") {
      Serial.print("RES GET_THRESHOLD ");
      Serial.println(threshold);
    }

    // --- Comando desconhecido ---
    else {
      Serial.println("ERR Unknown command.");
    }
  }

  // --- Envio automático do valor do LDR ---
  unsigned long currentTime = millis();
  if (currentTime - lastLdrSendTime >= ldrInterval) {
    lastLdrSendTime = currentTime;

    int ldrValue = analogRead(LDR_PIN);  // Lê o valor do LDR (A0)
    int ldrNormalized = map(ldrValue, 0, 1023, 0, 100);  // Normaliza para 0-100
    Serial.print("RES GET_LDR ");
    Serial.println(ldrNormalized);

    // --- Controle automático do LED com base no threshold ---
    if (ldrNormalized >= threshold) {
      ledIntensity = 100;
    } else {
      ledIntensity = 0;
    }
    analogWrite(LED_PIN, map(ledIntensity, 0, 100, 0, 255));  // Ajusta intensidade do LED
  }
}

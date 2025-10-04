// Pinos para ESP32
const int LDR_PIN = 32;     // GPIO32 para o LDR
const int LED_PIN = 22;     // GPIO22 para o LED

// Configurações
unsigned long lastLdrSendTime = 0;
const unsigned long ldrInterval = 2000;  // 2 segundos

// Estados do sistema
int ledIntensity = 0;     // Intensidade do LED (0-100)
int threshold = 50;       // Limiar para ativação automática (0-100)

// Parâmetros PWM para ESP32
const int ledChannel = 0;
const int pwmFreq = 5000;
const int pwmResolution = 8; // 8 bits: valores de 0 a 255

void setup() {
  Serial.begin(115200);

  // Configuração do PWM no ESP32
  ledcSetup(ledChannel, pwmFreq, pwmResolution);
  ledcAttachPin(LED_PIN, ledChannel);
  ledcWrite(ledChannel, 0); // Começa com o LED desligado
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
        ledcWrite(ledChannel, map(ledIntensity, 0, 100, 0, 255));
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
      int ldrValue = analogRead(LDR_PIN);  // Leitura do ADC
      int ldrNormalized = map(ldrValue, 0, 4095, 0, 100);  // ESP32 usa resolução de 12 bits (0-4095)
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

    int ldrValue = analogRead(LDR_PIN);
    int ldrNormalized = map(ldrValue, 0, 4095, 0, 100);
    Serial.print("RES GET_LDR ");
    Serial.println(ldrNormalized);

    // --- Controle automático do LED com base no threshold ---
    if (ldrNormalized >= threshold) {
      ledIntensity = 100;
    } else {
      ledIntensity = 0;
    }
    ledcWrite(ledChannel, map(ledIntensity, 0, 100, 0, 255));
  }
}

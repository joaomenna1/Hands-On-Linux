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

// #######################################################################################################################
// Código parcial atividade 1.4
#define LED_PIN 15
#define LDR_PIN 34

int ledValue = 0;          
int thresholdValue = 50;   
const int ldrMax = 4095;   
unsigned long lastSend = 0; 
bool autoControlActive = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  // pinMode(LDR_PIN, INPUT);

  Serial.println("SmartLamp inicializada");
  ledUpdate(); 
}

void loop() {
  
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    processCommand(cmd);
  }

  
  if (millis() - lastSend >= 2000) {
    int ldrVal = ldrGetValue();
    Serial.print("RES GET_LDR ");
    Serial.println(ldrVal);
    lastSend = millis();
  }

  int ldrVal = ldrGetValue();
  if (ldrVal > thresholdValue && ledValue == 0) {
    if (!autoControlActive) {
      autoControlActive = true;
      ledValue = 100;
      ledUpdate();
    }
  }  else if (autoControlActive && ldrVal <= thresholdValue) {
    autoControlActive = false;
    ledValue = 0;
    ledUpdate();
  }
}


void processCommand(String cmd) {
  if (cmd.startsWith("SET_LED")) {
    int value = cmd.substring(8).toInt();
    if (value >= 0 && value <= 100) {
      ledValue = value;
      autoControlActive = false;
      ledUpdate();
      Serial.println("RES SET_LED 1");
    } else {
      Serial.println("RES SET_LED -1");
    }
  }
  else if (cmd == "GET_LED") {
    Serial.print("RES GET_LED ");
    Serial.println(ledValue);
  }
  else if (cmd == "GET_LDR") {
    int ldrVal = ldrGetValue();
    Serial.print("RES GET_LDR ");
    Serial.println(ldrVal);
  }
  else if (cmd.startsWith("SET_THRESHOLD")) {
    int value = cmd.substring(14).toInt();
    if (value >= 0 && value <= 100) {
      thresholdValue = value;
      Serial.println("RES SET_THRESHOLD 1");
    } else {
      Serial.println("RES SET_THRESHOLD -1");
    }
  }
  else if (cmd == "GET_THRESHOLD") {
    Serial.print("RES GET_THRESHOLD ");
    Serial.println(thresholdValue);
  }
  else {
    Serial.println("Erro comando desconhecido.");
  }
}

void ledUpdate() {
  int pwm = map(ledValue, 0, 100, 0, 255);
  analogWrite(LED_PIN, pwm);
}

int ldrGetValue() {
  int raw = analogRead(LDR_PIN);
  int norm = map(raw, 0, ldrMax, 0, 100);
  if (norm > 100) return 100;
  if (norm < 0) return 0;
  return norm;
}

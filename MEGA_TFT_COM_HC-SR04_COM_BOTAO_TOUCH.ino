#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <ArduinoJson.h>
#include <TouchScreen.h>  // Adicionada para o toque
#include <HCSR04.h>

// --- CONFIGURAÇÃO DO TOUCH (Pinos comuns em Shields Mega) ---
// Se o touch não responder, esses pinos podem variar (XP, YP, XM, YM)
const int XP = 8, XM = A2, YP = A3, YM = 9;
const int TS_LEFT = 120, TS_RT = 910, TS_TOP = 70, TS_BOT = 890;  // Calibração padrão
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define triggerPin 40
#define echoCount 2
byte* echoPins = new byte[echoCount]{ 30, 32 };

MCUFRIEND_kbv tft;

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

// Cores
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define WHITE 0xFFFF
#define GREY 0x8410

bool esteiraLigada = false;
unsigned long delayLeitura = 0;

// Definição da área do botão (x, y, largura, altura)
int btnX = 20, btnY = 260, btnW = 200, btnH = 50;

void setup(void) {
  Serial.begin(115200);
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(0);
  tft.fillScreen(BLACK);

  desenharLayoutEstatico();
  desenharBotao();  // Desenha o botão pela primeira vez

  // Sensor Ultrasonico
  HCSR04.begin(triggerPin, echoPins, echoCount);
}

void loop(void) {
  // 1. VERIFICA O TOQUE NA TELA
  TSPoint p = ts.getPoint();

  // Restaura os pinos do LCD após o touch (necessário em shields paralelos)
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  // Se houver pressão suficiente no toque
  if (p.z > 10 && p.z < 1000) {
    // Mapeia as coordenadas do touch para os pixels da tela (240x320)
    int pixelX = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
    int pixelY = map(p.y, TS_TOP, TS_BOT, 0, tft.height());

    // Verifica se o toque foi dentro da área do botão inferior
    if (pixelX > btnX && pixelX < (btnX + btnW) && pixelY > btnY && pixelY < (btnY + btnH)) {
      esteiraLigada = !esteiraLigada;  // Inverte o estado (Start/Stop)
      desenharBotao();                 // Atualiza visual do botão
      delay(300);                      // Debounce para não ler múltiplos toques
    }
  }

  // 2. ESCUTA COMANDOS DO NODE-RED (Via Serial)
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    processarComandoSerial(comando);
    desenharBotao();  // Atualiza o botão se o comando veio do Node-RED
  }


  //  LEITURA DOS SENSORES A CADA 50 ms
  if (millis() - delayLeitura > 50) {
    leituraUltrasom();
  }

  // 3. LEITURA DOS SENSORES (A cada 2 segundos)
  // if (millis() - delayLeitura > 2000) {
  //   float t = dht22.getTemperature();
  //   float h = dht22.getHumidity();

  //   if (dht22.getLastError() == dht22.OK) {
  //     atualizarSensoresTFT(t, h);
  //     enviarDadosNodeRed(t, h);
  //   }
  //   delayLeitura = millis();
  // }
}

// --- FUNÇÃO: DESENHA O BOTÃO LIGA/DESLIGA ---
void desenharBotao() {
  if (esteiraLigada) {
    tft.fillRect(btnX, btnY, btnW, btnH, RED);  // Botão fica Vermelho para PARAR
    tft.drawRect(btnX, btnY, btnW, btnH, WHITE);
    tft.setTextColor(WHITE);
    tft.setFont(&FreeSans12pt7b);
    tft.setCursor(btnX + 40, btnY + 35);
    tft.print("PARAR");
  } else {
    tft.fillRect(btnX, btnY, btnW, btnH, GREEN);  // Botão fica Verde para INICIAR
    tft.drawRect(btnX, btnY, btnW, btnH, WHITE);
    tft.setTextColor(BLACK);
    tft.setFont(&FreeSans12pt7b);
    tft.setCursor(btnX + 30, btnY + 35);
    tft.print("INICIAR");
  }
}

void desenharLayoutEstatico() {
  tft.fillRect(0, 0, tft.width(), 40, BLUE);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(WHITE);
  tft.setCursor(10, 28);
  tft.print("CONTROLE DE ESTEIRA");

  // tft.setFont(&FreeSans9pt7b);
  // tft.setTextColor(GREY);
  // tft.setCursor(20, 75);
  // tft.print("Temperatura:");
  // tft.setCursor(20, 150);
  // tft.print("Umidade:");
}

// --- FUNÇÃO DE LEITURA DO SENSOR ULTROSONICO
void leituraUltrasom() {
  long* times = HCSR04.measureMicroseconds();
  static long* lastTime;
  //tft.fillRect(20, 85, 180, 80, BLACK);
  for (int i = 0; i < echoCount; i++) {
    if(times[i]!=lastTime[i]) {
      tft.setTextColor(BLACK, BLACK);
      tft.setCursor(20, 115 + i * 25);
      tft.print(lastTime[i]);

      tft.print(i + 1);
      tft.print(": ");
      tft.print(times[i] + millis());
      tft.println(" μs");
    }
  }
  // Serial.println("---");
  // delay(250);
}

void atualizarSensoresTFT(float t, float h) {
  tft.setFont(&FreeSans12pt7b);

  // Temperatura
  tft.fillRect(20, 85, 180, 40, BLACK);
  tft.setTextColor(RED);
  tft.setCursor(20, 115);
  tft.print(t, 1);
  tft.print(" C");

  // Umidade
  tft.fillRect(20, 160, 180, 40, BLACK);
  tft.setTextColor(BLUE);
  tft.setCursor(20, 190);
  tft.print(h, 1);
  tft.print(" %");
}

void processarComandoSerial(String json) {
  StaticJsonDocument<100> doc;
  DeserializationError error = deserializeJson(doc, json);
  if (!error) {
    const char* cmd = doc["cmd"];
    if (strcmp(cmd, "START") == 0) esteiraLigada = true;
    else if (strcmp(cmd, "STOP") == 0) esteiraLigada = false;
  }
}

void enviarDadosNodeRed(float t, float h) {
  StaticJsonDocument<128> doc;
  doc["temperatura"] = t;
  doc["umidade"] = h;
  doc["esteira"] = esteiraLigada ? 1 : 0;
  serializeJson(doc, Serial);
  Serial.println();
}
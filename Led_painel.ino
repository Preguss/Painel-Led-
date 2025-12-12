//==== GRAXAIM BOTS LED DISPLAY WIFI ====//

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>

// ======== DISPLAY CONFIG ======== //
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

#define CLK_PIN   18
#define DATA_PIN  23
#define CS_PIN     5

#define MAX_DEVICES 8

MD_Parola display = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// ======== Wi-Fi ======== //
const char* ssid = "GRAXAIM";
const char* password = "ifsul2025";

// ======== URL DO JSON ======== //
String url = "https://github.com/Preguss/Painel-Led-/blob/a3d28e06c206543223c4a7983a4c521ce21ea1b1/Led.json";

// Variável de controle
int mostrarTexto = 0;  
bool animacaoAtivada = false;

void iniciarAnimacao() {
  display.displayText(
      "Graxaim Bots",     // Texto
      PA_CENTER,          // Centralizado
      50,                 // Velocidade
      2000,               // Espera 2 segundos
      PA_PRINT,           // Entrada: aparece
      PA_FADE             // Saída: diminui
  );
}

void setup() {
  Serial.begin(115200);

  // ---- Conexão ao Wi-Fi ---- //
  WiFi.begin(ssid, password);
  Serial.println("Conectando ao Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Tentando conectar...");
  }

  Serial.println("Wi-Fi conectado!");

  // ---- Iniciar display ---- //
  display.begin();
  display.setIntensity(4);
  display.displayClear();
}

void loop() {

  // ======== Ler JSON do GitHub ======== //
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      // Ler comando do JSON
      mostrarTexto = doc["mostrar"];   // 1 = ligar animação || 0 = desligar

    } else {
      Serial.println("Erro ao acessar JSON");
    }

    http.end();
  }

  // ======== Controle do Display ======== //

  if (mostrarTexto == 1 && !animacaoAtivada) {
    iniciarAnimacao();
    animacaoAtivada = true;
  }

  if (mostrarTexto == 0) {
    display.displayClear();
    animacaoAtivada = false;
  }

  // Atualizar animação
  if (animacaoAtivada) {
    if (display.displayAnimate()) {
      display.displayReset();   // repete
    }
  }

  delay(3000); // Atualiza a cada 3 segundos
}

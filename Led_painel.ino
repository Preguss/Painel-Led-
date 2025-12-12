//==== GRAXAIM BOTS LED DISPLAY WIFI ====//

#include <WiFi.h>
#include <WebServer.h>

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

// ======== Web Server ======== //
WebServer server(80);

bool animacaoAtivada = false;

// ======== ANIMAÇÃO ======== //
void iniciarAnimacao() {
  display.displayText(
      "Graxaim Bots",
      PA_CENTER,
      50,
      2000,
      PA_PRINT,
      PA_FADE
  );
}

// ======== HTML DO SITE ======== //
String paginaHTML() {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>Graxaim Bots - Painel LED</title>
<style>
body {
  background-color: #111;
  color: white;
  font-family: Arial;
  text-align: center;
  margin-top: 50px;
}
button {
  padding: 20px;
  font-size: 22px;
  margin: 15px;
  border-radius: 10px;
  cursor: pointer;
  border: none;
  width: 220px;
}
.ligar { background-color: #1abc9c; color: black; }
.desligar { background-color: #e74c3c; color: white; }
</style>
</head>
<body>
<h1>Graxaim Bots - Controle do Display</h1>
<h2>Estado atual: )=====";

  html += (animacaoAtivada ? "ATIVADO" : "DESATIVADO");

  html += R"=====(
</h2>

<button class="ligar" onclick="location.href='/ligar'">LIGAR</button>
<button class="desligar" onclick="location.href='/desligar'">DESLIGAR</button>

</body>
</html>
)=====";

  return html;
}

// ======== HANDLERS DO SERVIDOR ======== //
void handleRoot() {
  server.send(200, "text/html", paginaHTML());
}

void handleLigar() {
  animacaoAtivada = true;
  iniciarAnimacao();
  server.send(200, "text/html", paginaHTML());
}

void handleDesligar() {
  animacaoAtivada = false;
  display.displayClear();
  server.send(200, "text/html", paginaHTML());
}

// ======== SETUP ======== //
void setup() {
  Serial.begin(115200);

  // ---- WiFi ---- //
  WiFi.begin(ssid, password);
  Serial.println("Conectando ao Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("Acesse no navegador: http://");
  Serial.println(WiFi.localIP());

  // ---- Display ---- //
  display.begin();
  display.setIntensity(4);
  display.displayClear();

  // ---- Rotas ---- //
  server.on("/", handleRoot);
  server.on("/ligar", handleLigar);
  server.on("/desligar", handleDesligar);

  server.begin();
  Serial.println("Servidor iniciado!");
}

// ======== LOOP ======== //
void loop() {
  server.handleClient();

  if (animacaoAtivada) {
    if (display.displayAnimate()) {
      display.displayReset();
    }
  }
}

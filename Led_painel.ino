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

// VARIÁVEIS CONTROLADAS PELO SITE
String textoPainel = "Graxaim Bots";
uint8_t animacao = PA_SCROLL_LEFT;  // animação padrão
bool animacaoRodando = false;

// ======== EXIBIR TEXTO ======== //
void iniciarAnimacao() {
  display.displayText(
      textoPainel.c_str(),
      PA_CENTER,
      40,
      1000,
      animacao,
      animacao
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
body { background:#111; color:white; font-family:Arial; text-align:center; margin-top:40px; }
input, select { padding:12px; width:300px; font-size:20px; border-radius:10px; margin-top:10px; }
button { padding:15px; font-size:20px; border-radius:10px; margin-top:20px; width:250px; cursor:pointer; }
.enviar { background:#1abc9c; border:none; }
</style>
</head>
<body>

<h1>Controle do Painel LED</h1>

<form action="/set" method="GET">
  <h2>Texto:</h2>
  <input type="text" name="txt" placeholder="Digite o texto" value=")=====";

  html += textoPainel;
  html += R"=====(">

  <h2>Animação:</h2>
  <select name="anim">
    <option value="0">Scroll Left</option>
    <option value="1">Scroll Right</option>
    <option value="2">Fade</option>
    <option value="3">Print</option>
    <option value="4">Blink</option>
    <option value="5">Open</option>
  </select>

  <br><br>
  <button class="enviar" type="submit">ATUALIZAR</button>
</form>

</body>
</html>
)=====";

  return html;
}

// ======== ROTAS ======== //
void handleRoot() {
  server.send(200, "text/html", paginaHTML());
}

void handleSet() {
  
  // pega texto enviado
  if (server.hasArg("txt")) {
    textoPainel = server.arg("txt");
  }

  // pega animação escolhida
  if (server.hasArg("anim")) {
    int mode = server.arg("anim").toInt();
    switch (mode) {
      case 0: animacao = PA_SCROLL_LEFT; break;
      case 1: animacao = PA_SCROLL_RIGHT; break;
      case 2: animacao = PA_FADE; break;
      case 3: animacao = PA_PRINT; break;
      case 4: animacao = PA_BLINK; break;
      case 5: animacao = PA_OPENING; break;
    }
  }

  animacaoRodando = true;
  iniciarAnimacao();

  server.send(200, "text/html", paginaHTML());
}

// ======== SETUP ======== //
void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Conectando ao Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("Acesse: http://");
  Serial.println(WiFi.localIP());

  // Display
  display.begin();
  display.setIntensity(4);
  display.displayClear();

  // Rotas
  server.on("/", handleRoot);
  server.on("/set", handleSet);

  server.begin();
  Serial.println("Servidor iniciado!");
}

// ======== LOOP ======== //
void loop() {
  server.handleClient();

  if (animacaoRodando) {
    if (display.displayAnimate()) {
      display.displayReset();
    }
  }
}

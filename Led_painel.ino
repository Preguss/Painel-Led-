//==== PAINEL LED WI-FI – GRAXAIM BOTS ====//

#include <WiFi.h>
#include <WebServer.h>
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

// ======== VARIÁVEIS ======== //
String texto = "Graxaim Bots";
int animacaoID = 0;

// tempos configuráveis (em segundos)
int tEntrada = 1;
int tPausa   = 1;
int tSaida   = 1;

// Lista de efeitos válidos
textEffect_t efeitos[] = {
  PA_PRINT,
  PA_SCROLL_LEFT,
  PA_SCROLL_RIGHT,
  PA_SCROLL_UP,
  PA_SCROLL_DOWN,
  PA_FADE,
  PA_BLINDS,
  PA_WIPE,
  PA_WIPE_CURSOR,
  PA_OPENING,
  PA_CLOSING,
  PA_GROW_UP,
  PA_GROW_DOWN
};

// Nomes bonitos das animações
String nomesAnim[] = {
  "Aparecer",
  "Rolar para Esquerda",
  "Rolar para Direita",
  "Rolar para Cima",
  "Rolar para Baixo",
  "Desvanecer",
  "Cortinas",
  "Wipe",
  "Wipe com Cursor",
  "Abrir",
  "Fechar",
  "Crescer para Cima",
  "Crescer para Baixo"
};

// Webserver
WebServer server(80);

//=================================================================
// FUNÇÃO: INICIA ANIMAÇÃO DE TEXTO
//=================================================================
void iniciarAnimacaoTexto() {
  
  int speedEntrada = tEntrada * 20;   
  int speedSaida   = tSaida   * 20;
  int pausaMS      = tPausa * 1000;

  textEffect_t efeito = efeitos[animacaoID];

  display.displayText(
    texto.c_str(),
    PA_CENTER,
    speedEntrada,
    pausaMS,
    efeito,
    efeito
  );
}

//=================================================================
// HTML DO SITE
//=================================================================
String htmlPage() {

  String animOps = "";
  for (int i = 0; i < 13; i++) {
    animOps += "<option value='" + String(i) + "'>" + nomesAnim[i] + "</option>";
  }

  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Painel LED</title>
<style>
body { font-family: Arial; background:#111; color:white; text-align:center; }
input, select, button { font-size:20px; padding:8px; margin:10px; }
</style>
</head>
<body>

<h1>Painel LED - Graxaim Bots</h1>

<form action="/set">

  <label>Texto:</label><br>
  <input name="txt" value=")rawliteral" + texto + R"rawliteral("><br>

  <label>Animação:</label><br>
  <select name="anim">)rawliteral" + animOps + R"rawliteral(</select><br>

  <label>Tempo Entrada (segundos):</label><br>
  <input name="entrada" type="number" value="1"><br>

  <label>Pausa (segundos):</label><br>
  <input name="pausa" type="number" value="1"><br>

  <label>Tempo Saída (segundos):</label><br>
  <input name="saida" type="number" value="1"><br>

  <button type="submit">Aplicar</button>

</form>

</body>
</html>
)rawliteral";

  return page;
}

//=================================================================
// HANDLER DO BOTÃO “APLICAR”
//=================================================================
void handleSet() {

  if (server.hasArg("txt")) texto = server.arg("txt");
  if (server.hasArg("anim")) animacaoID = server.arg("anim").toInt();

  if (server.hasArg("entrada")) tEntrada = server.arg("entrada").toInt();
  if (server.hasArg("pausa"))   tPausa   = server.arg("pausa").toInt();
  if (server.hasArg("saida"))   tSaida   = server.arg("saida").toInt();

  iniciarAnimacaoTexto();

  server.send(200, "text/html", htmlPage());
}

//=================================================================
// SETUP
//=================================================================
void setup() {
  Serial.begin(115200);

  display.begin();
  display.setIntensity(5);
  display.displayClear();

  WiFi.begin(ssid, password);
  Serial.println("Conectando ao Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWI-FI CONECTADO!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", [](){ server.send(200, "text/html", htmlPage()); });
  server.on("/set", handleSet);
  server.begin();
}

//=================================================================
// LOOP PRINCIPAL
//=================================================================
void loop() {

  server.handleClient();

  if (display.displayAnimate()) {
    display.displayReset();
  }
}

//==== PAINEL LED WI-FI – GRAXAIM BOTS ====//

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

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

// ======== NTP CLOCK ======== //
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 60000); // UTC-3

// ======== VARIABLES ======== //
String texto = "Graxaim Bots";
int animacaoID = 0;

// Lista de efeitos válidos do Parola
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

// Webserver
WebServer server(80);

//=================================================================
// FUNÇÃO: INICIA ANIMAÇÃO DE TEXTO
//=================================================================
void iniciarAnimacaoTexto() {
  textEffect_t efeito = efeitos[animacaoID];

  display.displayText(
    texto.c_str(),
    PA_CENTER,
    50,
    1000,
    efeito,
    efeito
  );
}

//=================================================================
// MOSTRAR HORA
//=================================================================
String getHora() {
  timeClient.update();
  char buf[6];
  sprintf(buf, "%02d:%02d", timeClient.getHours(), timeClient.getMinutes());
  return String(buf);
}

void animarHora() {
  String hora = getHora();
  display.displayText(hora.c_str(), PA_CENTER, 50, 1500, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

//=================================================================
// TEMPERATURA (FAKE)
//=================================================================
String getTemp() {
  return "25C";
}

void animarTemp() {
  display.displayText("Temp: 25C", PA_CENTER, 50, 1500, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

//=================================================================
// WEBPAGE HTML
//=================================================================
String htmlPage() {
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

<h1>PAINEL LED - GRAXAIM BOTS</h1>

<form action="/set">
  <label>Texto:</label><br>
  <input name="txt" value="Graxaim Bots"><br>

  <label>Animação:</label><br>
  <select name="anim">
    <option value="0">PRINT</option>
    <option value="1">SCROLL LEFT</option>
    <option value="2">SCROLL RIGHT</option>
    <option value="3">SCROLL UP</option>
    <option value="4">SCROLL DOWN</option>
    <option value="5">FADE</option>
    <option value="6">BLINDS</option>
    <option value="7">WIPE</option>
    <option value="8">WIPE CURSOR</option>
    <option value="9">OPENING</option>
    <option value="10">CLOSING</option>
    <option value="11">GROW UP</option>
    <option value="12">GROW DOWN</option>
  </select><br>

  <button type="submit">Mostrar Texto</button>
</form>

<hr>

<button onclick="location.href='/hora'">Mostrar Hora</button>
<button onclick="location.href='/temp'">Mostrar Temp</button>
<button onclick="location.href='/sequencia'">Executar Sequência Completa</button>

</body>
</html>
)rawliteral";
  return page;
}

//=================================================================
// HANDLER PARA SETAR TEXTO E ANIMAÇÃO
//=================================================================
void handleSet() {
  if (server.hasArg("txt")) texto = server.arg("txt");
  if (server.hasArg("anim")) animacaoID = server.arg("anim").toInt();

  iniciarAnimacaoTexto();

  server.send(200, "text/html", htmlPage());
}

//=================================================================
// HANDLERS EXTRAS
//=================================================================
void handleHora() {
  animarHora();
  server.send(200, "text/html", htmlPage());
}

void handleTemp() {
  animarTemp();
  server.send(200, "text/html", htmlPage());
}

void handleSequencia() {
  iniciarAnimacaoTexto();
  delay(3000);
  animarHora();
  delay(3000);
  animarTemp();
  server.send(200, "text/html", htmlPage());
}

//=================================================================
// SETUP
//=================================================================
void setup() {
  Serial.begin(115200);

  // Display
  display.begin();
  display.setIntensity(5);
  display.displayClear();

  // Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Conectando ao Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWI-FI CONECTADO!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();

  // Rotas
  server.on("/", [](){ server.send(200, "text/html", htmlPage()); });
  server.on("/set", handleSet);
  server.on("/hora", handleHora);
  server.on("/temp", handleTemp);
  server.on("/sequencia", handleSequencia);

  server.begin();
}

//=================================================================
// LOOP
//=================================================================
void loop() {
  server.handleClient();

  if (display.displayAnimate()) {
    display.displayReset();
  }
}

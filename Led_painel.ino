//==== PAINEL LED WI-FI – GRAXAIM BOTS + OTA ====//

#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>

#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>

#include <ArduinoOTA.h>   // <<< ADICIONADO

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

// ===== EEPROM ===== //
#define EEPROM_SIZE 300

// ===== VARIÁVEIS ===== //
String texto = "Graxaim Bots";
int animacaoID = 0;
int tPausa = 1;
int velocidade = 40;
bool painelLigado = true;

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

// Nomes bonitos
String nomesAnim[] = {
  "Aparecer",
  "Rolar Esquerda",
  "Rolar Direita",
  "Rolar Cima",
  "Rolar Baixo",
  "Desvanecer",
  "Cortinas",
  "Wipe",
  "Wipe Cursor",
  "Abrir",
  "Fechar",
  "Crescer Cima",
  "Crescer Baixo"
};

WebServer server(80);

//=================================================================
// EEPROM
//=================================================================
void salvarEEPROM() {
  EEPROM.writeString(0, texto);
  EEPROM.write(100, animacaoID);
  EEPROM.write(101, tPausa);
  EEPROM.write(102, velocidade);
  EEPROM.write(103, painelLigado);
  EEPROM.commit();
}

void carregarEEPROM() {
  texto = EEPROM.readString(0);
  animacaoID = EEPROM.read(100);
  tPausa = EEPROM.read(101);
  velocidade = EEPROM.read(102);
  painelLigado = EEPROM.read(103);

  if (texto.length() < 1) texto = "Graxaim Bots";
}

//=================================================================
// FUNÇÃO: INICIA ANIMAÇÃO DE TEXTO
//=================================================================
void iniciarAnimacaoTexto() {
  if (!painelLigado) {
    display.displayClear();
    return;
  }

  int pausaMS = tPausa * 1000;

  display.displayText(
    texto.c_str(),
    PA_CENTER,
    velocidade,
    pausaMS,
    efeitos[animacaoID],
    efeitos[animacaoID]
  );
}

//=================================================================
// HTML – COM APERFEIÇOAMENTO VISUAL
//=================================================================
String htmlPage() {

  // gerar opções
  String animOps = "";
  for (int i = 0; i < 13; i++) {
    animOps += "<option value='" + String(i) + "'";
    if (i == animacaoID) animOps += " selected ";
    animOps += ">" + nomesAnim[i] + "</option>";
  }

  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Painel LED</title>

<style>
body {
  background:#0d0d0d;
  color:white;
  font-family: Arial;
  text-align:center;
}

.card {
  background:#1c1c1c;
  padding:20px;
  width:80%;
  margin:auto;
  border-radius:14px;
  box-shadow:0 0 10px #000;
}

input, select, button {
  font-size:20px;
  padding:8px;
  margin:8px;
  border-radius:8px;
  border:none;
}

.preview {
  margin-top:20px;
  padding:10px;
  font-size:28px;
  border-radius:8px;
  background:#000;
  border:1px solid #444;
  width:80%;
  margin:auto;
}

.btnOn { background:#28a745; }
.btnOff { background:#b30000; }

</style>

</head>
<body>

<h1>Painel LED - Graxaim Bots</h1>

<div class="card">
<form action="/set">

  <label>Texto:</label><br>
  <input name="txt" value=")rawliteral" + texto + R"rawliteral("><br>

  <label>Animação:</label><br>
  <select name="anim">)rawliteral" + animOps + R"rawliteral(</select><br>

  <label>Pausa (segundos):</label><br>
  <input name="pausa" type="number" value=")rawliteral" + String(tPausa) + R"rawliteral("><br>

  <label>Velocidade (ms):</label><br>
  <input name="vel" type="number" value=")rawliteral" + String(velocidade) + R"rawliteral("><br>

  <button type="submit">Aplicar</button>
</form>

<br>

<a href="/on"><button class="btnOn">LIGAR</button></a>
<a href="/off"><button class="btnOff">DESLIGAR</button></a>

<h3>Pré-visualização:</h3>
<div class="preview">)rawliteral" + texto + " — " + nomesAnim[animacaoID] + R"rawliteral(</div>

</div>
</body>
</html>
)rawliteral";

  return page;
}

//=================================================================
// HANDLERS
//=================================================================
void handleSet() {

  if (server.hasArg("txt")) texto = server.arg("txt");
  if (server.hasArg("anim")) animacaoID = server.arg("anim").toInt();
  if (server.hasArg("pausa")) tPausa = server.arg("pausa").toInt();
  if (server.hasArg("vel")) velocidade = server.arg("vel").toInt();

  painelLigado = true;
  iniciarAnimacaoTexto();
  salvarEEPROM();

  server.send(200, "text/html", htmlPage());
}

void handleOn() {
  painelLigado = true;
  salvarEEPROM();
  iniciarAnimacaoTexto();
  server.send(200, "text/html", htmlPage());
}

void handleOff() {
  painelLigado = false;
  salvarEEPROM();
  display.displayClear();
  server.send(200, "text/html", htmlPage());
}

//=================================================================
// SETUP
//=================================================================
void setup() {
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);
  carregarEEPROM();

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

  // ------ OTA CONFIGURADO AQUI ------ //
  ArduinoOTA.setHostname("painel_led_ota");

  ArduinoOTA
    .onStart([]() {
      Serial.println("Iniciando OTA...");
    })
    .onEnd([]() {
      Serial.println("OTA Concluído!");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progresso: %u%%\r", (progress * 100) / total);
    })
    .onError([](ota_error_t error) {
      Serial.printf("Erro OTA [%u]\n", error);
    });

  ArduinoOTA.begin();
  // --------------------------------- //

  server.on("/", [](){ server.send(200, "text/html", htmlPage()); });
  server.on("/set", handleSet);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.begin();

  iniciarAnimacaoTexto();
}

//=================================================================
// LOOP PRINCIPAL
//=================================================================
void loop() {
  server.handleClient();

  ArduinoOTA.handle();   // <<< ESSENCIAL PARA ATUALIZAÇÃO OTA

  if (painelLigado) {
    if (display.displayAnimate()) {
      display.displayReset();
    }
  }
}

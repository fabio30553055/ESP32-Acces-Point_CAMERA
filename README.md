//8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
// AUTORES FABIO RIVADENEIRA_ SANDRA NOPE
// UNIVERSIDAD DEL VALLE-CALI COLOMBIA 2025
// CODIGO DE ACCES POINT
//8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888

#include <WiFi.h>
#include <WebServer.h>
#include "SPIFFS.h"
const char* AP_SSID = "TALLER3_AP_CAM";
const char* AP_PASS = "12345678";  // >=8 caracteres

// ETIQUETA PARA CREAR COMANDOS
String lastLabel = "sin etiqueta";
// COMANDO PARA EL CLIENTE CAM
volatile int CMD_PIN4 = 0;   // 0 = LOW, 1 = HIGH

WebServer server(80);
const char* IMG_PATH = "/IMAGEN_RX.jpg";

void VER_PAGINA_WEB() { 

  String html = F(
    "<!DOCTYPE html><html><head><meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>ESP32 AP - Imagen</title></head><body>"
    "<h2>Última imagen recibida</h2>"
    "<img src='/IMAGEN_RX.jpg' style='max-width:100%;height:auto'/>"
    "<p>......  ESPERANDO IMAGEN .......</p>"
    "</body></html>"
  );
  server.send(200, "text/html", html);
}

// Obtener el archivo más reciente
void OBTENER_IMAGEN() {

  if (!SPIFFS.exists(IMG_PATH)) {
    server.send(404, "text/plain", "Imagen no disponible.");
    return;
  }
  File f = SPIFFS.open(IMG_PATH, "r");
  server.streamFile(f, "image/jpeg");
  f.close();
}

// Archivo
File uploadFile;

void OBTENER_ETIQUETA() {
  
  // Leer etiqueta enviada por el ESP32-CAM
  String label = "";
  if (server.hasArg("label")) {label = server.arg("label");}
  if (label != "a" && label != "b"){ label = " label error ";}
  // Almacenamos en la variable global 
  lastLabel = label;
  Serial.printf("[/upload] etiqueta recibida = %s\n", label.c_str());
  server.send(200, "text/plain", "OK label=" + label);
}

void SUBE_IMAGEN() {

  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
      if (SPIFFS.exists(IMG_PATH)){SPIFFS.remove(IMG_PATH);}
      uploadFile = SPIFFS.open(IMG_PATH, FILE_WRITE);
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile){uploadFile.write(upload.buf, upload.currentSize);}
  } 
  else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile){uploadFile.close();}
    Serial.printf("Imagen recibida: %u bytes\n", upload.totalSize);
  } 
  else if (upload.status == UPLOAD_FILE_ABORTED) {
    if (uploadFile) { uploadFile.close(); SPIFFS.remove(IMG_PATH); }
    Serial.println("Subida abortada.");
  }
}

void setup() {
  Serial.begin(115200);delay(500);

  if (!SPIFFS.begin(true)) {
    Serial.println("Fallo SPIFFS");
    while (true) delay(1000);
  }

  // Iniciar AP
  IPAddress local_IP(192,168,10,1);
  IPAddress gateway(192,168,10,1);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(AP_SSID, AP_PASS);
  if (!ok) {
    Serial.println("Error iniciando softAP");
    while (true) delay(1000);
  }
  //IPAddress ip = WiFi.softAPIP();
  Serial.print("AP SSID: "); Serial.println(AP_SSID);
  Serial.print("AP PASS: "); Serial.println(AP_PASS);
 // Rutas
  server.on("/", HTTP_GET, VER_PAGINA_WEB);
  server.on("/IMAGEN_RX.jpg", HTTP_GET, OBTENER_IMAGEN);
  server.on("/upload", HTTP_POST, OBTENER_ETIQUETA, SUBE_IMAGEN);
//----------------------------------------------------------------------------
// REVISAR SOLICITUD DE  ENCENDIDO DE PIN
  server.on("/set", HTTP_GET, [](){
  if (server.hasArg("pin4")) {
    int valor = server.arg("pin4").toInt();
    if (valor != 0) {CMD_PIN4 = 1;} 
    else {CMD_PIN4 = 0;}
  }
  server.send(200, "text/plain", String("OK pin4=") + CMD_PIN4);
});

// El cliente (ESP32-CAM) consulta el estado para su GPIO4
server.on("/cmd", HTTP_GET, [](){
  server.send(200, "text/plain", String(CMD_PIN4));
});

//consultar última etiqueta recibida en /upload
server.on("/last_label", HTTP_GET, [](){
  server.send(200, "text/plain", lastLabel);
});

server.begin();
Serial.println("HTTP server listo.");
}

void loop() {
  server.handleClient();
}





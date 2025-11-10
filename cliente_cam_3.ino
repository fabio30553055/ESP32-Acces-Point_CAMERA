//8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
// AUTORES FABIO RIVADENEIRA_ SANDRA NOPE
// UNIVERSIDAD DEL VALLE-CALI COLOMBIA 2025
// CODIGO ESP32 CAM CLIENTE STA
//8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888
#include <WiFi.h>
#include "esp_camera.h"
#include <HTTPClient.h>

// DATOS DEL AP (ESP32 servidor) 
const char* WIFI_SSID = "TALLER3_AP_CAM";
const char* WIFI_PASS = "12345678";
// VARIABLE GLOBAL DE ETIQUETA
String CURRENT_LABEL = "a";   // "a" o "b"
// Host del servidor (IP del AP) 
const char* SERVER_HOST = "192.168.10.1";
const uint16_t SERVER_PORT = 80;
const char* SERVER_PATH = "/upload";

// MODELO DE CAMARA AI Thinker ESP32-CAM 
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

static bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;//PIXFORMAT_GRAYSCALE; PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_VGA;   // QVGA/VGA/SVGA...

  /*
  FRAMESIZE_QQVGA 160×120
  FRAMESIZE_QVGA  320×240
  FRAMESIZE_VGA   640×480
  FRAMESIZE_SVGA  800×600
  
  */
  config.jpeg_quality = 12; // calidad (10 alta, 30 baja)
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.fb_count     = 1;//1,2 3

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error init cam: 0x%x\n", err);
    return false;
  }
  sensor_t* s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_VGA);
  return true;
}

//88888888888888888888888888888888888888888888888888888888888888888888888888888
static bool POST_IMG(uint8_t* data, size_t len) {//postImageMultipart

  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClient client;
  client.setTimeout(5000);

  if (!client.connect(SERVER_HOST, SERVER_PORT)) {
    Serial.println("No conecta al servidor");
    return false;
  }
  // Construcción de multipart 
  String boundary = "--ESP32MI_BOUNDARY_UNICO_ABC123";//"----ESP32CamBoundary7MA4YWxkTrZu0gW";
  // Parte 1: campo de texto "label"
  String partLabel =
      "--" + boundary + "\r\n"
      "Content-Disposition: form-data; name=\"label\"\r\n\r\n" +
      CURRENT_LABEL + "\r\n";

  // Parte 2: encabezado del archivo "imagen"
  String partImageHead =
      "--" + boundary + "\r\n"
      "Content-Disposition: form-data; name=\"image\"; filename=\"frame.jpg\"\r\n"
      "Content-Type: image/jpeg\r\n\r\n";
  // Cierre multipart
  String closing = "\r\n--" + boundary + "--\r\n";
  // Longitud total del body: label + head de imagen + bytes JPEG + cierre
  size_t contentLength = partLabel.length() + partImageHead.length() + len + closing.length();

  // Cabeceras HTTP
  client.print(String("POST ") + SERVER_PATH + " HTTP/1.1\r\n");
  client.print(String("Host: ") + SERVER_HOST + "\r\n");
  client.print(String("Content-Type: multipart/form-data; boundary=") + boundary + "\r\n");
  client.print(String("Content-Length: ") + contentLength + "\r\n");
  client.print("Connection: close\r\n\r\n");
  // --- Cuerpo: label -> head imagen -> bytes imagen -> cierre ---
  client.print(partLabel);
  client.print(partImageHead);
  size_t written = client.write(data, len);
  client.print(closing);
  // Leer status line (opcional)
  String status = client.readStringUntil('\n');
  status.trim();
  Serial.println(status);
  uint32_t t0 = millis();
  while (client.connected() && millis() - t0 < 2000) {
    while (client.available()) {
      client.read();
      t0 = millis();
    }
    delay(10);
  }
  client.stop();
  return (written == len);
}

//**************************************************************************
void setup() {

  Serial.begin(115200);
  delay(300);
  pinMode(4, OUTPUT);        // GPIO4 en AI Thinker controla el flash LED
  digitalWrite(4, LOW);

  if (!initCamera()) {
    Serial.println("No se pudo inicializar la cámara.");
    while (true) delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("Conectando a %s", WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(500); }
  Serial.println("\nConectado!");
  Serial.print("IP STA: "); Serial.println(WiFi.localIP());
}

void loop() {

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("fb NULL");
    delay(2);
    return;
  }
  bool ok = POST_IMG(fb->buf, fb->len);
    if (ok) {
      Serial.printf("POST OK (%u bytes)\n", fb->len);
    } else {
      Serial.printf("POST FALLO (%u bytes)\n", fb->len);
    }
  esp_camera_fb_return(fb);
//-------------------------------------------------------------
  int cmd = OBTENER_COMANDO();
  if (cmd == 1) {
    digitalWrite(4, HIGH);
  } else if (cmd == 0) {
    digitalWrite(4, LOW);
  }

//-------------------------------------------------------------
  delay(2); 
}

//888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888

int OBTENER_COMANDO() {
  
  HTTPClient http;
  String url = String("http://") + SERVER_HOST + "/cmd";
  if (!http.begin(url)) return -1;
  int code = http.GET();
  if (code != 200) { http.end(); return -1; }
  String payload = http.getString();
  http.end();
  payload.trim();
  if (payload == "1") {return 1;} 
  else {return 0;}
}

//8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888

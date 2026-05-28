#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ===== KONFIGURASI — ISI SESUAI MILIKMU =====
const char* WIFI_SSID     = "dydx";
const char* WIFI_PASSWORD = "kalkulusParsial";

const char* MQTT_HOST     = "b991b24530db4275a23f9fa3f4fe29f6.s1.eu.hivemq.cloud"; // dari HiveMQ
const int   MQTT_PORT     = 8883;
const char* MQTT_USER     = "TugasBesarAndesis12";
const char* MQTT_PASS     = "TugasBesarAndesis12";
const char* MQTT_TOPIC    = "sensor/hujan";
// ================================================

#define RAIN_SENSOR_PIN 23
#define GREEN_LED 21
#define RED_LED   22
#define BUZZER    2

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// === VARIABEL UNTUK ANTI-SPAM ===
bool lastRainingState = false; 
bool firstRun = true; 
// ================================

void connectWiFi() {
  Serial.print("Menghubungkan WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung! IP: " + WiFi.localIP().toString());
}

void connectMQTT() {
  espClient.setInsecure(); // untuk testing; ganti dengan sertifikat untuk produksi
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  while (!mqttClient.connected()) {
    Serial.print("Menghubungkan MQTT...");
    String clientId = "ESP32-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("Terhubung!");
    } else {
      Serial.print("Gagal, rc=");
      Serial.println(mqttClient.state());
      delay(3000);
    }
  }
}

void setup() {
  pinMode(RAIN_SENSOR_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(115200);
  connectWiFi();
  connectMQTT();
}

void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  int rainDetected = digitalRead(RAIN_SENSOR_PIN);
  bool isRaining = (rainDetected == LOW);

  // Kontrol hardware seperti sebelumnya, responsif realtime
  digitalWrite(GREEN_LED, isRaining ? LOW  : HIGH);
  digitalWrite(RED_LED,   isRaining ? HIGH : LOW);
  digitalWrite(BUZZER,    isRaining ? HIGH : LOW);

  // LOGIKA ANTI-SPAM: Cek apakah status berubah
  if (isRaining != lastRainingState || firstRun) {
    // Kirim data ke MQTT sebagai JSON
    StaticJsonDocument<128> doc;
    doc["status"]    = isRaining ? "OVERLOAD" : "OPTIMAL";
    doc["isRaining"] = isRaining;
    doc["timestamp"] = millis();

    char payload[128];
    serializeJson(doc, payload);

    mqttClient.publish(MQTT_TOPIC, payload);
    Serial.println("Terkirim: " + String(payload));

    // Perbarui status terakhir
    lastRainingState = isRaining;
    firstRun = false;
  }

  delay(50);
}
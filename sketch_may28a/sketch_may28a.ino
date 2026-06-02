#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ===== KONFIGURASI WIFI & MQTT =====
const char* WIFI_SSID  = "dydx";
const char* WIFI_PASSWORD = "kalkulusParsial";

const char* MQTT_HOST  = "b991b24530db4275a23f9fa3f4fe29f6.s1.eu.hivemq.cloud";
const int   MQTT_PORT  = 8883;
const char* MQTT_USER  = "TugasBesarAndesis12";
const char* MQTT_PASS  = "TugasBesarAndesis12";
const char* MQTT_TOPIC = "sensor/tangki";
// ====================================

#define TRIG_PIN 5
#define ECHO_PIN 4

#define GREEN_LED 21
#define RED_LED   22
#define BUZZER    23
#define TINGGI_TOREN 12.0


String statusSebelumnya = "";

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

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
  espClient.setInsecure();
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

float bacaJarak() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long durasi = pulseIn(ECHO_PIN, HIGH);
  float jarak = durasi * 0.034 / 2;
  return jarak;
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  connectWiFi();
  connectMQTT();

  Serial.println("=== SISTEM MONITORING TANGKI AIR ===");
}

void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  float jarak = bacaJarak();
  if (jarak > TINGGI_TOREN) jarak = TINGGI_TOREN;
  if (jarak < 0) jarak = 0;

  int persen = ((TINGGI_TOREN - jarak) / TINGGI_TOREN) * 100;
  persen = constrain(persen, 0, 100);

  String status;

  if (persen <= 20) {
    status = "KOSONG";
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, HIGH);
  }
  else if (persen <= 50) {
    status = "SEDIKIT AIR";
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, HIGH);
  }
  else if (persen <= 80) {
    status = "SUDAH TERISI";
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER, LOW);
  }
  else {
    status = "PENUH";
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER, LOW);
  }

  if (status != statusSebelumnya) {

    Serial.println();
    Serial.println("==============================");
    Serial.print("Status Tangki : "); Serial.println(status);
    Serial.print("Level Air     : "); Serial.print(persen); Serial.println("%");
    Serial.print("Jarak Air     : "); Serial.print(jarak); Serial.println(" cm");

    StaticJsonDocument<128> doc;
    doc["status"] = status;
    doc["level"]  = persen;
    doc["jarak"]  = jarak;
    doc["timestamp"] = millis();

    char payload[128];
    serializeJson(doc, payload);

    mqttClient.publish(MQTT_TOPIC, payload);
    Serial.println("Terkirim ke MQTT: " + String(payload));

    statusSebelumnya = status;
  }

  delay(1000);
}
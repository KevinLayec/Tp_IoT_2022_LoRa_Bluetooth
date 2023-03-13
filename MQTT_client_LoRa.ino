#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LoRa.h>
#define SCK 5 // GPIO5 -- SX127x's SCK
#define MISO 19 // GPIO19 -- SX127x's MISO
#define MOSI 27 // GPIO27 -- SX127x's MOSI
#define SS 18 // GPIO18 -- SX127x's CS
#define RST 14 // GPIO14 -- SX127x's RESET
#define DI0 26 // GPIO26 -- SX127x's IRQ

const long int freq = 8685E5;
const long int sf = 8;
const long int sb = 100E3;

float d1=12.0;
float d2=321.54;

const char* ssid = "Pixel_5969";
const char* password = "azerty1234";
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char message[50];

union pack {
  uint8_t frame[16];
  float data[4];
} sdp;

void setup() {
  Serial.begin(115200);

  pinMode(DI0, INPUT);
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  while (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
  }
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sb);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  client.setServer(mqtt_server, 1883);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT server...");
    if (client.connect("ArduinoClientFJ")) {
      Serial.println("Connected to MQTT server");
    } else {
      Serial.print("Failed to connect to MQTT server, rc=");
      Serial.print(client.state());
      delay(1000);
    }
    
  }

}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT server...");
    if (client.connect("ArduinoClientFJ")) {
      Serial.println("Connected to MQTT server");
    } else {
      Serial.print("Failed to connect to MQTT server, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}


void loop() {
  if (!client.connected()){
    reconnect();
  }
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    sprintf(message,"%ld %ld %ld",freq, sf, sb);
    Serial.print("sent\n");
    client.publish("srt/FJ", message);
    
    delay(1000);   
  }

  Serial.println("New Packet");
  LoRa.beginPacket();
  sdp.data[0]=d1;
  sdp.data[1]=d2;
  LoRa.write(sdp.frame,16);
  LoRa.endPacket();
  d1++;
  d2+=2;
  delay(1000);

  client.loop();
}

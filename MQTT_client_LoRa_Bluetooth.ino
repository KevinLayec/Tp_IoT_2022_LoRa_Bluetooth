#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LoRa.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h> 
int scanTime = 5; //In seconds

BLEScan* pBLEScan;

/*#define SCK 5 // GPIO5 -- SX127x's SCK
#define MISO 19 // GPIO19 -- SX127x's MISO
#define MOSI 27 // GPIO27 -- SX127x's MOSI
#define SS 18 // GPIO18 -- SX127x's CS
#define RST 14 // GPIO14 -- SX127x's RESET
#define DI0 26 // GPIO26 -- SX127x's IRQ*/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

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

/*union pack {
  uint8_t frame[16];
  float data[4];
} sdp;*/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
   void onResult (BLEAdvertisedDevice advertisedDevice) { 
    Serial.printf("Advertised Device: ts \n", 
advertisedDevice.toString().c_str());
   }
};


void setup() {
  Serial.begin(115200);

  /*pinMode(DI0, INPUT);
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  while (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
  }
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sb);*/

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
  Serial.println("Scanning...");
  BLEDevice::init("MyESP32FJ");
  pBLEScan = BLEDevice::getScan (); //create new scan 
  pBLEScan->setAdvertisedDeviceCallbacks (new MyAdvertisedDeviceCallbacks());

pBLEScan->setActiveScan (true); //active scan uses more power, but get resulta faster

pBLEScan->setInterval (100);

pBLEScan->setWindow (99); // less or equal setInterval value
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

  /*Serial.println("New Packet");
  LoRa.beginPacket();
  sdp.data[0]=d1;
  sdp.data[1]=d2;
  LoRa.write(sdp.frame,16);
  LoRa.endPacket();
  d1++;
  d2+=2;
  delay(1000);*/

  BLEScanResults foundDevices = pBLEScan->start (scanTime, false); 
  Serial.print ("Devices found: ");
  Serial.println (foundDevices.getCount()); 
  Serial.println("Scan done!");
  pBLEScan->clearResults(); // delete results fromBLEScan buffer memory delay(2000);


  client.loop();
}

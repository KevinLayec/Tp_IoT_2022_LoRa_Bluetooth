#include <SPI.h> // Bibliothèque SPI pour la communication avec le module LoRa
#include <WiFi.h> // Bibliothèque WiFi pour se connecter à un réseau sans fil
#include <PubSubClient.h> // Bibliothèque MQTT pour la communication avec le serveur MQTT
#include <LoRa.h> // Bibliothèque LoRa pour la communication sans fil à longue portée

#define SCK   5                  // Broche SCK pour la communication SPI avec le module LoRa - GPIO5 -- SX127x's SCK
#define MISO  19                 // Broche MISO pour la communication SPI avec le module LoRa - GPIO19 -- SX127x's MISO
#define MOSI  27                 // Broche MOSI pour la communication SPI avec le module LoRa - GPIO27 -- SX127x's MOSI
#define SS    18                 // Broche SS pour la communication SPI avec le module LoRa - GPIO18 -- SX127x's CS
#define RST   14                 // Broche RST pour la communication SPI avec le module LoRa - GPIO14 -- SX127x's RESET
#define DI0   26                 // Broche DI0 pour détecter l'interruption LoRa - GPIO26 -- SX127x's IRQ

// Paramètres permettant de configurer LoRa
const long int freq = 8685E5; // Fréquence de la communication LoRa
const long int sf = 8; // Facteur d'étalement
const long int sb = 100E3; // Bande passante du signal

float d1=12.0;
float d2=321.54;

const char* ssid = "Pixel_5969"; // Nom du réseau WiFi
const char* password = "azerty1234"; // Mot de passe du réseau WiFi
const char* mqtt_server = "test.mosquitto.org"; // Adresse du broker MQTT

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char message[50];

// Utilisation d'un union pour stocker les données à envoyer en un seul bloc
union pack {
  uint8_t frame[16]; // Tableau de 16 octets
  float data[4]; // Tableau de 4 float
} sdp;

void setup() {
  Serial.begin(115200); // Initialisation de la communication série

  pinMode(DI0, INPUT); // Configuration du pin DI0 en entrée
  SPI.begin(SCK,MISO,MOSI,SS); // Initialisation de la communication SPI
  LoRa.setPins(SS,RST,DI0); // Configuration des pins pour le module LoRa
  while (!LoRa.begin(freq)) { // Tentative de connexion au module LoRa
    Serial.println("Starting LoRa failed!"); // Message d'erreur
  }
  LoRa.setSpreadingFactor(sf); // Configuration du facteur d'étalement pour la communication LoRa
  LoRa.setSignalBandwidth(sb); // Configuration de la bande passante pour la communication LoRa

  WiFi.begin(ssid, password); // Connexion au réseau WiFi
  while (WiFi.status() != WL_CONNECTED) { // Tant que la connexion n'est pas établie
    delay(1000);
    Serial.println("Connecting to WiFi..."); // Message de connexion en cours
  }
  Serial.println("Connected to WiFi"); // Message de connexion établie

  client.setServer(mqtt_server, 1883); // Configuration du serveur MQTT

  while (!client.connected()) { // Tant que la connexion n'est pas établie
    Serial.println("Connecting to MQTT server..."); // Message de connexion en cours
    if (client.connect("ArduinoClientFJ")) { // Tentative de connexion
      Serial.println("Connected to MQTT server"); // Message de connexion établie
    } else {
      Serial.print("Failed to connect to MQTT server, rc="); // Message d'erreur
      Serial.print(client.state());
      delay(1000);
    }
  }
}

// Fonction de reconnexion au serveur MQTT
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
  // Vérifier si le client MQTT est connecté, sinon tenter de se reconnecter
  if (!client.connected()){
    reconnect();
  }
  // Mesurer le temps écoulé depuis le dernier message envoyé
  long now = millis();
  // Si le temps écoulé est supérieur à 5 secondes
  if (now - lastMsg > 5000) {
    // Enregistrer le moment où le message a été envoyé
    lastMsg = now;
    // Créer une chaîne de caractères à partir des valeurs de freq, sf et sb
    sprintf(message,"%ld %ld %ld",freq, sf, sb);
    // Afficher un message de confirmation
    Serial.print("sent\n");
    // Publier le message sur le topic "srt/FJ"
    client.publish("srt/FJ", message);
    
    // Attendre une seconde
    delay(1000);   
  }

  // Afficher un message de confirmation
  Serial.println("New Packet");
  // Préparer un paquet LoRa
  LoRa.beginPacket();
  // Ajouter les valeurs de d1 et d2 à un tableau de float
  sdp.data[0]=d1;
  sdp.data[1]=d2;
  // Écrire le paquet LoRa avec les données du tableau de float
  LoRa.write(sdp.frame,16);
  // Terminer le paquet LoRa
  LoRa.endPacket();
  // Incrémenter la valeur de d1 et ajouter 2 à la valeur de d2
  d1++;
  d2+=2;
  // Attendre une seconde
  delay(1000);

  // Vérifier les messages entrants sur le client MQTT
  client.loop();
}

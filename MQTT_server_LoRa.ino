#include <WiFi.h>                // Bibliothèque WiFi pour se connecter à un réseau sans fil
#include <PubSubClient.h>        // Bibliothèque MQTT pour la communication avec le serveur MQTT
#include <SPI.h>                 // Bibliothèque SPI pour la communication avec le module LoRa
#include <LoRa.h>                // Bibliothèque LoRa pour la communication sans fil à longue portée

#include <stdio.h>               // Bibliothèque standard C pour les entrées/sorties
#include <stdlib.h>              // Bibliothèque standard C pour les fonctions de conversion de type
#include <string.h>              // Bibliothèque standard C pour la manipulation de chaînes de caractères

#define SCK   5                  // Broche SCK pour la communication SPI avec le module LoRa
#define MISO  19                 // Broche MISO pour la communication SPI avec le module LoRa
#define MOSI  27                 // Broche MOSI pour la communication SPI avec le module LoRa
#define SS    18                 // Broche SS pour la communication SPI avec le module LoRa
#define RST   14                 // Broche RST pour la communication SPI avec le module LoRa
#define DI0   26                 // Broche DI0 pour détecter l'interruption LoRa

const char* ssid = "Pixel_5969"; // Nom du réseau WiFi
const char* password = "azerty1234"; // Mot de passe du réseau WiFi
const char* mqtt_server = "test.mosquitto.org"; // Adresse du serveur MQTT
int verif = 0;                   // Variable de vérification pour l'initialisation du module LoRa

float d1;                        // Variable pour stocker la première donnée reçue
float d2;                        // Variable pour stocker la deuxième donnée reçue
int y=0;                         // Compteur pour parcourir le tableau de données reçues
long int freq;                   // Variable pour stocker la fréquence utilisée par le module LoRa
long int sf;                     // Variable pour stocker le facteur d'étalement utilisé par le module LoRa
long int sb;                     // Variable pour stocker la largeur de bande utilisée par le module LoRa

WiFiClient espClient;            // Objet pour la connexion WiFi
PubSubClient client(espClient);  // Objet pour la connexion MQTT


// Union pour stocker les données reçues du module LoRa
union pack {
    uint8_t frame[16];
    float data[4]; 
} rdp; 

// Fonction pour se connecter au réseau WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { // Attente de la connexion WiFi
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Fonction pour initialiser les broches du module LoRa et se connecter au serveur MQTT
void setup() {
  Serial.begin(115200); // Initialisation du port série à 115200 bauds (Fréquence d'affichage)
  setup_wifi(); //Connexion au réseaux WiFi
  client.setServer(mqtt_server, 1883); // Configuration du serveur MQTT
  client.setCallback(callback);

  pinMode(DI0, INPUT);  // Configuration de l'entrée DI0
  SPI.begin(SCK,MISO,MOSI,SS);  // Configuration de l'interface SPI
  LoRa.setPins(SS,RST,DI0);  // Configuration des broches pour LoRa
}

// Boucle reconnect permettant de s'assurer que le client est constamment connecté au serveur MQTT, afin de pouvoir envoyer et recevoir des messages.
void reconnect() {
  while (!client.connected()) { // Tentative de reconnexion
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX); // Génération d'un ID client aléatoire - Pour éviter d'avoir un ID identique à un autre groupe
    if (client.connect(clientId.c_str())) { // Connexion au serveur MQTT
      Serial.println("connected");
      client.subscribe("srt/FJ"); // Abonnement au topic "srt/FJ"
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // Attente de 5 secondes avant la prochaine tentative de connexion - Si la connexion à échoué
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  char* token;
  char* data_lora[3] = {NULL};
  int token_count = 0;

  char payload_str[length + 1];
  for (int i = 0; i < length; i++) {
    payload_str[i] = (char)payload[i]; // Conversion du tableau de bytes en chaîne de caractères
  }
  payload_str[length] = '\0'; // Ajout du caractère nul à la fin de la chaîne

  //Lecture conntinue des données
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(payload_str);

  // Découpage de la chaîne de caractères en tokens en utilisant l'espace comme délimiteur
  if (verif == 0){
    token = strtok(payload_str, " ");
    while (token != NULL) {
      if (token_count >= 3) { // Si nous avons déjà récupéré les trois derniers tokens, sortir de la boucle
        break;
      }
      // Stockage des trois derniers tokens dans last_three_tokens
      Serial.println(token);
      data_lora[token_count] = token;
      token_count++;
      token = strtok(NULL, " ");
  }
  freq = strtol(data_lora[0], NULL, 10);  // Conversion du premier token en long
  //Serial.println(freq); - Test d'affichage des données vérif cohérence - Pas exécuté
  sf = strtol(data_lora[1], NULL, 10); // Conversion du deuxième token en long
  //Serial.println(sf); - Test d'affichage des données vérif cohérence - Pas exécuté
  sb = strtol(data_lora[2], NULL, 10); // Conversion du troisème token en long
  //Serial.println(sb); - Test d'affichage des données vérif cohérence - Pas exécuté
      
  // Initialisation du module LoRa avec la fréquence de communication
  // Si LoRa.begin() renvoie false, cela signifie que l'initialisation a échoué
  if (!LoRa.begin(freq)) {
        Serial.println("Starting LoRa failed!");
  }
  
  // Configuration du facteur d'étalement (spreading factor) et de la bande passante du signal
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sb);
  
  // Initialisation de la variable "verif" à 1 pour indiquer que la transmission LoRa est prête
  verif = 1;
  
  // Affichage des 3 dernières valeurs reçues par LoRa (Test réalisé une fois pour vérifier la cohérence des données)
  // Serial.printf("Les trois dernières valeurs sont : %s %s %s\n", data_lora[0], data_lora[1], data_lora[2]);
}

void loop() {
  if (!client.connected()) {
    Serial.println("Disconnect");
    reconnect();
  }

  // Recherche des paquets LoRa entrants
  int packetLen;
  packetLen = LoRa.parsePacket();
  if(packetLen == 16){
    y = 0;
    while (LoRa.available()){
      rdp.frame[y] = LoRa.read();
      y++;
    }
    // Lecture des données et force du signal en réception
    d1 = rdp.data[0];
    d2 = rdp.data[1];
    int rssi = LoRa.packetRssi(); //Force du signal en réception en dB
    Serial.println(d1);
    Serial.println(d2);
    Serial.println(rssi);
  }

  // Vérification de l'état de la connexion MQTT
  client.loop();
}

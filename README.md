# Tp_IoT_2022_LoRa_Bluetooth

Groupe de travail composé des étudiants suivants : 
- Ronan Bachelot
- Kévin Layec
Nous avons collaboré avec : 
- Pierre Jadaud
- Alexandre Fiandaca
Pour échanger les données entre les deux cartes. 

## Intro et sujet de TP
L'ensemble des scripts présentés ci-dessous représente l'évolution d'un unique Script pour atteindre les objectifs du TP. 

Pour rappel, voici le sujet de TP : 
1. Client
- Récupération d'un paquet Mqtt sur test.mosquitto.org : nomAP/ipaddr
- Se connecter à une carte sur l'AP et faire une requete http donnant les valeurs nécessaire à LoRa.
- Ecouter les données de LoRa.
2. Serveur :
- Faire le serveur servant les données au Client.
-  Ajouter une fonction permettant de switcher entre Serveur et Client
- Sur le serveur, échangez la partie LoRa par du bluetooth (ou BLE)
- Refaire la partie cliente pour répondre au bluetooth

## Les scripts

### MQTT_Server
ce script permet à un ESP32 de se connecter à un réseau Wi-Fi et à un serveur MQTT, de s'abonner à un sujet MQTT spécifique, d'écouter les messages MQTT entrants et d'exécuter une fonction de rappel appropriée pour traiter chaque message reçu. Le script inclut également des fonctions pour gérer la perte de connexion au serveur MQTT et se reconnecter automatiquement, ainsi que pour afficher des messages sur la communication série pour aider à déboguer le code.

### MQTT_client_LoRa
Ce scrip est celui utilisé pour créer le client, il s'agit du groupe qui transmet les informations de connexion LoRa via un serveur MQTT. 
Les données sont séparées par un espace pour les identifiés, elles sont également envoyé en boucle afin de facilité l'accès aux données pour l'autre groupe. 

### MQTT_server_LoRa
Ce script est celui utilisé pour créer le serveur, il s'agit du groupe qui doit ce connecter au serveur MQTT et sélectionner le topic "srt/FJ" pour aller chercher les données permettant de configuré la connexion LoRa envoyé par l'autre groupe (client).
Une fois les données collectés sur le serveur MQTT, le script réalise un traitement des données. Sachant que 3 données doivent être récupéré (freq,sf,sb) et qu'elles sont séparées par un espace.
Une fois les données collectés, le serveur à la possibilité de ce connecter avec l'autre groupe via LoRa. 

### MQTT_Server_Bluetooth
Ce script permet de mettre en place la partie Bluetooth du TP, l'objectif étant de générer un appareils pouvant autorisé la connexion depuis un autre.
Nous n'avons pas eu le temps de terminer toute cette partie. Les données permettant de configuré le Bluetooth ne sont donc pas communiqué via LoRa mais directement incrit dans le code (#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b" et #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8").
Le nom définit pour l'appareil est IoTDevkit-ESP32

Nous avons ainsi pu vérifier le bon fonctionnement de cette partie via l'application Bleutooth Scanner.
![Vérification_Bluetooth_Scanner1](https://github.com/KevinLayec/Tp_IoT_2022_LoRa_Bluetooth/blob/TP_MQTT_LoRa_Bluetooth/Scan_All_Device.png)
![Vérification_Bluetooth_Scanner2](https://github.com/KevinLayec/Tp_IoT_2022_LoRa_Bluetooth/blob/TP_MQTT_LoRa_Bluetooth/Parametre_device.png)
On observe bien que la carte émet un signal Bluetooth (UUID et nom de l'appareil identique au code

###MQTT_client_LoRa_Bluetooth
Ce code ne fonctionne pas.
L'objectif était de ce connnecter au Bluetooth configuré par l'autre carte et d'y récupéré un message. 
Dans notre cas, le message étant : "Hello World!"

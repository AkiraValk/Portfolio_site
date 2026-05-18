#include <WiFi.h>
#include <WiFiManager.h>
#include <ESP32_VS1053_Stream.h>
#include <PubSubClient.h>

// --- Broches VS1053 ---
#define VS1053_CS     32
#define VS1053_DCS    33
#define VS1053_DREQ   15

// --- URLs de stations ---
const char* urls[] = {
  "http://live.radioking.fr/azur-fm-68",
  "http://chisou-02.cdn.eurozet.pl:8112/;",
  "http://streamer01.sti.usherbrooke.ca:8000/cfak.mp3",
  "http://radios.rtbf.be/wr-c21-metal-128.mp3",
  "http://ecoutez.chyz.ca:8000/mp3",
  "http://ice4.somafm.com/seventies-128-mp3",
  "http://lyon1ere.ice.infomaniak.ch/lyon1ere-high.mp3",
  "http://stream03.ustream.ca:8000/cism128.mp3"
};

#define NOMBRECHAINES (sizeof(urls)/sizeof(urls[0]))
int chaine = 0;

// --- Égaliseur ---
int8_t trebleAmp = 0;    // -8 à +7
uint8_t trebleFreq = 8; // 1 à 15
uint8_t bassAmp = 7;    // 0 à 15
uint8_t bassFreq = 8;   // 2 à 15

uint8_t eqSettings[4];

// --- Valeurs par défaut ---
const int8_t DEFAULT_TREBLE_AMP = 0;   // -8 à +7 (milieu)
const uint8_t DEFAULT_TREBLE_FREQ = 8;   // 1 à 15 (milieu)
const uint8_t DEFAULT_BASS_AMP = 7;   // 0 à 15 (milieu)
const uint8_t DEFAULT_BASS_FREQ = 8;  

int chainePrecedente = 0;


int volume = 90;  // 0 = fort, 100 = faible

// --- Spatialisation ---
uint8_t spatialMode = 0; // 0=off, 1=low, 2=medium, 3=high

// --- Mode manuel ou URL personnalisée ---
bool modeManuel = true;


// --- Objet player ---
ESP32_VS1053_Stream player;

// --- MQTT ---
const char* mqtt_server = "10.3.141.2";
const int mqtt_port = 1884;
const char* topic_commands = "tptest/s2/charon";




const char* nomsStations[] = {
  "Azur FM",
  "Chisou",
  "CFAK",
  "RTBF Metal",
  "CHYZ",
  "SomaFM 70s",
  "Lyon 1ère",
  "CISM"
};

const char* codecs[] = {
  "MP3",    // Azur FM
  "MP3",    // Chisou
  "MP3",    // CFAK
  "MP3",    // RTBF Metal
  "MP3",    // CHYZ
  "MP3",    // SomaFM 70s
  "MP3",    // Lyon 1ère
  "MP3"     // CISM
};


const int bitrates[] = {
  128, // Azur FM
  128, // Chisou
  128, // CFAK
  128, // RTBF Metal
  128, // CHYZ
  128, // SomaFM 70s
  128, // Lyon 1ère
  128  // CISM
};


WiFiClient espClient;
PubSubClient client(espClient);

// --- Fonctions audio ---
void applyTone(ESP32_VS1053_Stream &player) {
    // Construction du mot 16 bits pour SCI_BASS :
    // Bits 15-12 : Treble amplitude (-8 à +7, 4 bits signés)
    // Bits 11-8  : Treble frequency (1-15 kHz)
    // Bits 7-4   : Bass amplitude (0-15)
    // Bits 3-0   : Bass frequency (2-15, x10 Hz)
    uint16_t value = ((trebleAmp & 0x0F) << 12)
                   | ((trebleFreq & 0x0F) << 8)
                   | ((bassAmp & 0x0F) << 4)
                   | (bassFreq & 0x0F);
    player.writeCodecRegister(0x02, value); // 0x02 = SCI_BASS
}


// --- Fonction de spatialisation ---
void setSpatialization(ESP32_VS1053_Stream &player, uint8_t mode) {
    uint16_t valeur = player.readCodecRegister(0x00);
    valeur = valeur & 0b1111111101101111;
    if (mode == 1) valeur = valeur | (1 << 4);
    else if (mode == 2) valeur = valeur | (1 << 7);
    else if (mode == 3) valeur = valeur | (1 << 4) | (1 << 7);
    player.writeCodecRegister(0x00, valeur);
    Serial.print("SCI_MODE = "); Serial.println(valeur, BIN);
    Serial.print("Spatialisation mode (EarSpeaker) : "); Serial.println(mode % 4);
}

// --- Publier le statut complet ---
void publishStatus() {
  char status[256];
  snprintf(
    status, sizeof(status),
    "%s",
    (modeManuel ? "MANUEL" : "URL"),
    chaine,
    player.lastUrl(),
    volume
  );
  client.publish(topic_status, status);
}

// Fonction pour publier la station actuelle
void publishCurrentStation() {
  if (modeManuel) {
    client.publish("colmar/radio/current_station", nomsStations[chaine]);
  } else {
    client.publish("colmar/radio/current_station", "URL personnalisée");
  }
}


void publishStationCodec() {
  if (modeManuel) {
    client.publish("colmar/radio/station_codec", codecs[chaine]);
  } else {
    client.publish("colmar/radio/station_codec", "Inconnu");
  }
}


void publishStationBitrate() {
  if (modeManuel) {
    char bitrate[16];
    snprintf(bitrate, sizeof(bitrate), "%d kbps", bitrates[chaine]);
    client.publish("colmar/radio/station_bitrate", bitrate);
  } else {
    client.publish("colmar/radio/station_bitrate", "Inconnu");
  }
}

void publishVolumeDB() {
  // Estimation : 1 unité = 0.5 dB d'atténuation
  float db = volume * 0.5;
  char buf[16];
  snprintf(buf, sizeof(buf), "-%.1f dB", db);
  client.publish("colmar/radio/volume_db", buf);
}





// --- MQTT : callback ---
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
  Serial.print("Commande MQTT reçue : "); Serial.println(message);

  if (String(topic) == topic_url) { //URL !!!
        Serial.println("Traitement de l'URL !");
        player.stopSong();
          if (player.connecttohost(message.c_str())) {
            modeManuel = false;
            Serial.print("URL reçue : ");
            Serial.println(message);
            publishStatus();
          } else {
            Serial.println("Échec de la connexion à l’URL !");
  }}

  if (String(topic) == topic_commands) {  // COMMANDES !!!
    if (message == "n") {
      chainePrecedente = chaine; // mémorise l'actuelle
      chaine = (chaine + 1) % NOMBRECHAINES;
      player.stopSong();
      player.connecttohost(urls[chaine]);
      modeManuel = true;
    } else if (message == "retour") {
      if (chainePrecedente != chaine) { // évite de revenir sur la même
        int temp = chaine;
        chaine = chainePrecedente;
        chainePrecedente = temp; // pour pouvoir revenir encore si besoin
        player.stopSong();
        player.connecttohost(urls[chaine]);
        modeManuel = true;
        Serial.println("Retour à la station précédente !");
      } else {
        Serial.println("Déjà sur la station précédente !");
      }
    } else if (message == "v") {
      chainePrecedente = chaine; // mémorise l'actuelle
      chaine = (chaine - 1 + NOMBRECHAINES) % NOMBRECHAINES;
      player.stopSong();
      player.connecttohost(urls[chaine]);
      modeManuel = true;
    } else if (message.toInt() > 0 || message == "0") {
      int newVolume = message.toInt();
      Serial.print("Nouveau volume reçu (numérique) : ");
      Serial.println(newVolume);
      volume = newVolume;
      player.setVolume(volume);
      publishVolumeDB();
      publishStatus();
    } else if (message == "spatial") {
      spatialMode = (spatialMode+1)%4;
      setSpatialization(player, spatialMode);
    } else if (message == "p") {
      player.stopSong();
      Serial.println("Lecture stoppée (MQTT) !");
    } else if (message == "r") {
      if (!player.connecttohost(urls[chaine])) {
        Serial.println("Échec de la reconnexion à l’URL (MQTT) !");
      } else {
        Serial.println("Lecture reprise (MQTT) !");
      }
    } else if (message == "m") {
      volume = 50; // ou une valeur très élevée pour couper le son
      player.setVolume(volume);
      Serial.println("Son coupé (mute) via MQTT !");
      publishVolumeDB();
      publishStatus();
    } else if (message == "-") {
        if (volume > 0) {
          volume--;
          player.setVolume(volume);
          Serial.print("Volume augmenté via MQTT : ");
          Serial.println(volume);
          publishVolumeDB();
          publishStatus();
      }
    } else if (message == "+") {
      if (volume < 100) {
        volume++;
        player.setVolume(volume);
        Serial.print("Volume baissé via MQTT : ");
        Serial.println(volume);
      } } else if (message == "f") {
        if (bassAmp > 0) {
          bassAmp--;
          applyTone(player);
          Serial.print("Basses moins fortes : ");
          Serial.println(bassAmp);
          // Publier la nouvelle valeur
          char buf[16];
          snprintf(buf, sizeof(buf), "%d", bassAmp);
          client.publish("colmar/radio/bass_state", buf);
        }
      } else if (message == "g") {
        if (bassAmp < 15) {
          bassAmp++;
          applyTone(player);
          Serial.print("Basses plus fortes : ");
          Serial.println(bassAmp);
          // Publier la nouvelle valeur
          char buf[16];
          snprintf(buf, sizeof(buf), "%d", bassAmp);
          client.publish("colmar/radio/bass_state", buf);
        }
      } else if (message == "h") {
        if (trebleAmp > -8) {
          trebleAmp--;
          applyTone(player);
          Serial.print("Aigus moins forts : ");
          Serial.println(trebleAmp);
          char buf[16];
          snprintf(buf, sizeof(buf), "%d", trebleAmp);
          client.publish("colmar/radio/treble_state", buf);
        }
      } else if (message == "j") {
        if (trebleAmp < 7) {
          trebleAmp++;
          applyTone(player);
          Serial.print("Aigus plus forts : ");
          Serial.println(trebleAmp);
          char buf[16];
          snprintf(buf, sizeof(buf), "%d", trebleAmp);
          client.publish("colmar/radio/treble_state", buf);
        }
      }  
      
  } 
}

// --- MQTT : reconnexion ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connexion MQTT...");
    if (client.connect("ESP32RadioColmar")) {
      Serial.println("connecté !");
      client.subscribe(topic_commands);
      client.subscribe(topic_url);
    } else {
      Serial.print("échec, code=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nRadio WiFi");
  Serial.println("Contrôles clavier ET MQTT : n/v (stations), p/- (volume), g/f (basses), j/h (aigus), d (défaut), s (spatialisation)");

  // --- WiFi ---
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  bool res = wm.autoConnect("RadioWiFiConfig");
  if(!res) {
    Serial.println("Échec de connexion WiFi. Redémarrage...");
    delay(3000);
    ESP.restart();
  } else {
    Serial.print("WiFi connecté, IP: ");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.macAddress());
  }

  // --- MQTT ---
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // --- VS1053 ---
  SPI.begin();
  if (!player.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ)) {
    Serial.println("Échec de l’initialisation du VS1053 !");
    while(1);
  } else {
    Serial.println("VS1053 initialisé avec succès !");
  }
  player.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ);
  player.setVolume(volume);
  setSpatialization(player, spatialMode);
  applyTone(player);
  Serial.print("Connexion à la station : ");
  Serial.println(urls[chaine]);

  if (!player.connecttohost(urls[chaine])) {
    Serial.println("Échec de la connexion à l’URL !");
  } else {
  Serial.println("Connexion à l’URL réussie !");
  }
}

void loop() {
  player.loop(); // Nécessaire pour faire tourner la radio

  // --- MQTT ---
  if (!client.connected()) reconnect();
  client.loop();

  // --- Commandes série (clavier) ---

  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'n') {
      chainePrecedente = chaine;
      chaine = (chaine + 1) % NOMBRECHAINES;
      Serial.println("On change de chaine !");
      player.stopSong();
      player.connecttohost(urls[chaine]);
    }
    if (c == 'v') {
      chainePrecedente = chaine;
      chaine = (chaine - 1 + NOMBRECHAINES) % NOMBRECHAINES;
      Serial.println("On reviens à la chaine précédente !");
      player.stopSong();
      player.connecttohost(urls[chaine]);
    }
    if (c == '-') {
      if (volume > 0) {
        volume--;
        Serial.println("on baisse le volume !");
        player.setVolume(volume);
        publishStatus();
      }
    }

    // VOLUME!!!
    if (c == 'p') {
      if (volume < 100) {
        volume = 90 ;
        player.setVolume(volume);
        Serial.println("on augmente le volume !");
        Serial.println(volume);
        publishStatus();
      }
    }
    if (c == 'g') {
      if (bassAmp < 15) {
        bassAmp++;
        Serial.print("Basses plus fortes : ");
        Serial.println(bassAmp);
        applyTone(player);
      } else {
        Serial.println("Basses déjà au maximum !");
      }
    }
    if (c == 'f') {
      if (bassAmp > 0) {
        bassAmp--;
        Serial.print("Basses moins fortes : ");
        Serial.println(bassAmp);
        applyTone(player);
      } else {
        Serial.println("Basses déjà au minimum !");
      }
    }
    if (c == 'j') {
      if (trebleAmp < 7) {
        trebleAmp++;
        Serial.print("Aigus plus forts : ");
        Serial.println(bassAmp);
        applyTone(player);
      } else {
        Serial.println("Aigus déjà au maximum !");
      }
    }
    if (c == 'h') {
      if (trebleAmp > -8) {
        trebleAmp--;
        Serial.print("Aigus moins forts : ");
        Serial.println(bassAmp);
        applyTone(player);
      } else {
        Serial.println("Aigus déjà au minimum !");
      }
    }
    if (c == 'd') {
      trebleAmp = DEFAULT_TREBLE_AMP;
      trebleFreq = DEFAULT_TREBLE_FREQ;
      bassAmp = DEFAULT_BASS_AMP;
      bassFreq = DEFAULT_BASS_FREQ;
      Serial.println("remise à 0");
      applyTone(player);
    }
    if (c == 's') {
      spatialMode = (spatialMode+1)%4;
      setSpatialization(player, spatialMode);
    }
    if (c == 'x') {
      player.stopSong();
      Serial.println("Lecture stoppée !");
    }
    if (c == 'r') {
      if (!player.connecttohost(urls[chaine])) {
        Serial.println("Échec de la reconnexion à l’URL !");
    } else {
        Serial.println("Lecture reprise !");
    }
    if (c == 'm') {
      // Mute : on met le volume à 100 (ou une valeur très élevée)
      volume = 100;
      player.setVolume(volume);
      Serial.println("Son coupé (mute) !");
    }
  }
  }

  // --- Publication périodique du statut ---
  static unsigned long last = 0;
  if (millis() - last > 5000) {
    last = millis();
    publishCurrentStation();
    publishStationCodec();
    publishStationBitrate();
    publishVolumeDB();
    publishStatus();

    char buf[16];
      snprintf(buf, sizeof(buf), "%d", bassAmp);
      client.publish("colmar/radio/bass_state", buf);
      snprintf(buf, sizeof(buf), "%d", trebleAmp);
      client.publish("colmar/radio/treble_state", buf);

    char status[128];
    snprintf(
      status, sizeof(status),
      "%s",
      (modeManuel ? "MANUEL" : "URL"),
      chaine,
      player.lastUrl(),
      volume
    );
    client.publish(topic_status, status);
  }
}

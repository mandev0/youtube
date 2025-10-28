#include <SPI.h>
#include <MFRC522.h>
#include <DFRobotDFPlayerMini.h>
#include <AccelStepper.h>

// --- DFPlayer ---
DFRobotDFPlayerMini myDFPlayer;
HardwareSerial &mp3Serial = Serial3;

// --- RFID ---
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

// --- Step Motor ---
#define IN1 4
#define IN2 5
#define IN3 6
#define IN4 7
AccelStepper myStepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);

// --- Potansiyometreler ---
#define POT_VOLUME A15
#define POT_PERMISSION A14

// --- Değişkenler ---
int lastVolume = -1;
int lastPermValue = 0;
unsigned long lastChangeTime = 0;
const unsigned long stableDelay = 400;

int currentTrack = 1;
int totalTracks = 3;
int folderNumber = 1;

bool isPlaying = false;
bool rfidAuthorized = false;
bool permission = false;

unsigned long lastRFIDCheck = 0;
const unsigned long RFID_INTERVAL = 500; // 200ms’de bir RFID kontrol et

String current_rfid = "";

// --- İzin aralığı ---
int minPermission = 0;
int maxPermission = 10;

// --- RFID Listesi ---
struct CardMap {
  const char* uid;
  int folder;
};
CardMap cards[] = {
  {"0455ca32252090", 1},
  {"0455c932252090", 2},
  {"0455c832252090", 3},
  {"455bf32252090", 4},
  {"455c032252090", 5},
  {"455b832252090", 6},
  {"455c732252090", 7},
  {"455b932252090", 8},
  {"455be32252090", 9},
  {"455bc32252090", 10}
};

int findFolder(const char* uid) {
  for (int i = 0; i < sizeof(cards) / sizeof(cards[0]); i++) {
    if (strcmp(uid, cards[i].uid) == 0)
      return cards[i].folder;
  }
  return -1;
}

// --- Setup ---
void setup() {
  // Serial
  Serial.begin(9600);
  delay(1500);

  // MP3 serial
  mp3Serial.begin(9600);
  delay(1500);

  // Sistem başlatma
  Serial.println("Sistem baslatiliyor...");
  SPI.begin();
  delay(1500);

  // RFID başlatma
  mfrc522.PCD_Init();
  delay(1500);
  Serial.println("RFID hazir...");

  // DFPlayer başlatma
  Serial.println("DFPlayer baglanti bekleniyor...");
  while (!myDFPlayer.begin(mp3Serial)) {
    Serial.println("DFPlayer hazir degil, 1sn bekleniyor...");
    delay(1000);
  }
  Serial.println("DFPlayer baglandi!");
  myDFPlayer.volume(10);

  // --- Step motor ---
  myStepper.setMaxSpeed(800.0);     // Maksimum hız
  myStepper.setAcceleration(200.0); // Hızlanma/yavaşlama oranı

  pinMode(POT_VOLUME, INPUT);
  pinMode(POT_PERMISSION, INPUT);

  Serial.println("Kurulum tamamlandi.");
}

// --- Loop ---
void loop() {
  // Step motor
  myStepper.run();

  // Müzik çalarken rfid kontrolü yapma. Eğer yaparsan aralıklarla yap
  unsigned long now = millis();
  if (now - lastRFIDCheck >= RFID_INTERVAL && !isPlaying) {
    lastRFIDCheck = now;
    handleRFID();
  }

  // --- Ses kontrolü ---
  int potValue = analogRead(POT_VOLUME);
  int volume = map(potValue, 0, 1023, 25, 5); // ters çevrildi
  if (abs(volume - lastVolume) > 1) {
    myDFPlayer.volume(volume);
    lastVolume = volume;
  }

  // --- Pot izin kontrolü ---
  int permValue = analogRead(POT_PERMISSION);
  if (abs(permValue - lastPermValue) > 5) {
    lastChangeTime = millis();
    lastPermValue = permValue;
  }
  if (millis() - lastChangeTime > stableDelay) {
    permission = (permValue >= minPermission && permValue <= maxPermission);
  }

  // --- Motor kontrolü ---
  if (permission) {
    if (isPlaying) {
      myStepper.setSpeed(600);
    } else {
      myStepper.setSpeed(600);
    }
  } else {
    myStepper.setSpeed(0);
  }

  // --- Müzik kontrolü ---
  if (!permission && isPlaying) {
    // pot izin dışına çıktı → müzik dursun
    myDFPlayer.pause();
    isPlaying = false;
    rfidAuthorized = false;  // tekrar RFID gerekir
    Serial.println("Pot izin dışına çıktı → müzik durdu, RFID tekrar gerek.");
  }

  // --- DFPlayer olayları ---
  if (myDFPlayer.available()) {
    uint8_t type = myDFPlayer.readType();
    int value = myDFPlayer.read();
    if (type == DFPlayerPlayFinished) {
      nextTrack();
    }
  }
}

// --- RFID okuma ---
void handleRFID() {
  if (!permission) return; // pot izinli değilse RFID okumayı boşuna yapma
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return;

  char uidStr[32] = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    sprintf(uidStr + strlen(uidStr), "%02x", mfrc522.uid.uidByte[i]);
  }

  Serial.print("Kart okundu: ");
  Serial.println(uidStr);
  
  if (current_rfid == uidStr) {
    // Aynı kart tekrar okutulduysa kaldığı yerden devam
    if (!isPlaying) {
      myDFPlayer.start();
      isPlaying = true;
      Serial.println("Aynı RFID, müzik kaldığı yerden devam ediyor.");
    }
  } else {
    // Yeni kart okutuldu
    myDFPlayer.stop();
    isPlaying = false;
    current_rfid = uidStr;

    int folder = findFolder(uidStr);
    if (folder > 0) {
      folderNumber = folder;
      rfidAuthorized = true;
      playFolder(folderNumber);
    } else {
      Serial.println("Tanimli kart degil!");
      rfidAuthorized = false;
    }
  }

  mfrc522.PICC_HaltA();
}

// --- Yardımcı Fonksiyonlar ---
void playFolder(int folder) {
  Serial.print("Klasör ");
  Serial.print(folder);
  Serial.println(" oynatiliyor...");
  myDFPlayer.playFolder(folder, 1);
  isPlaying = true;
}

void nextTrack() {
  currentTrack++;
  if (currentTrack > totalTracks) {
    myDFPlayer.stop();
    isPlaying = false;
    currentTrack = 1;
    Serial.println("Klasördeki tüm parçalar bitti.");
    return;
  }

  myDFPlayer.playFolder(folderNumber, currentTrack);
  Serial.print("Yeni parça: ");
  Serial.println(currentTrack);
}

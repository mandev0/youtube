#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- ESP32-C3 SUPERMINI PIN AYARLARI ---
#define SDA_PIN 8   // LCD SDA Pini
#define SCL_PIN 9   // LCD SCL Pini
#define REED_PIN 4  // Reed Switch Pini (GPIO 4)

// LCD Nesnesi
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Derin uykuda silinmeyen değişken (RTC Hafıza)
RTC_DATA_ATTR int sonDurum = -1; 

void setup() {
  // Serial başlat (USB CDC On Boot: Enabled olmalı)
  Serial.begin(115200);
  // Açılışta USB'nin oturması için kısa bekleme (C3'e özel)
  delay(1000); 

  // Pin Ayarı
  pinMode(REED_PIN, INPUT_PULLUP);

  // LCD Başlat (Pinleri özel olarak belirtiyoruz)
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();

  // --- UYANMA SEBEBİNİ KONTROL ET ---
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {
    Serial.println("Sebep: Kapi hareketiyle uyandim!");
  } else {
    Serial.println("Sebep: Ilk acilis (Power ON)");
    lcd.setCursor(0,0);
    lcd.print("Sistem Aktif");
    delay(1000);
  }

  // Mevcut durumu kontrol et ve ekrana yaz
  int suankiDurum = digitalRead(REED_PIN);
  durumuEkranaYaz(suankiDurum);

  // --- BİR SONRAKİ UYANMA AYARI (DİNAMİK) ---
  
  // Hangi pini dinleyeceğimizi bitmask olarak hazırlıyoruz
  uint64_t pinMask = (1ULL << REED_PIN); 

  if (suankiDurum == HIGH) {
    // Şu an HIGH (Kapı AÇIK / Mıknatıs YOK)
    // O zaman LOW olunca (Mıknatıs gelince) uyanmalı
    esp_deep_sleep_enable_gpio_wakeup(pinMask, ESP_GPIO_WAKEUP_GPIO_LOW);
    Serial.println("Sonraki uyanma: MIKNATIS GELINCE (LOW)");
  } else {
    // Şu an LOW (Kapı KAPALI / Mıknatıs VAR)
    // O zaman HIGH olunca (Mıknatıs gidince) uyanmalı
    esp_deep_sleep_enable_gpio_wakeup(pinMask, ESP_GPIO_WAKEUP_GPIO_HIGH);
    Serial.println("Sonraki uyanma: MIKNATIS GIDINCE (HIGH)");
  }

  // Hazırlıklar tamam, uykuya geç
  Serial.println("Derin uykuya geciliyor...");
  delay(100); 
  
  lcd.noBacklight();
  lcd.clear();
  
  // Derin Uyku Başlat
  esp_deep_sleep_start();
}

void loop() {
  // Deep Sleep'te burası çalışmaz.
}

void durumuEkranaYaz(int durum) {
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  
  // NC (Normally Closed) Switch Mantığı:
  // Mıknatıs VAR (Kapalı) -> GND -> LOW
  // Mıknatıs YOK (Açık) -> 3.3V -> HIGH
  
  if (durum == HIGH) { 
    lcd.print("KAPI: ACIK [!]");
  } else { 
    lcd.print("KAPI: KAPALI");
  }
  
  // Okumak için süre tanı
  delay(3000);
}
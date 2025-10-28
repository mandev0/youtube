#### Gerekli Parçalar
- Arduino (Mega ya da Uno)
- DF Player Mini MP3
- BYJ-48
- ULN2003A
- RC522
- 2 adet Potansiyometre (10K)
- USB Breakout
- Açma/Kapama
- 1K Direnç
#### İlgili Arduino Kodu
`/src/record_player/record_player.ino`
#### Kablo Diagramı
- DF Player
    - 5V  <-----------> VCC DFPlayer
    - GND <-----------> GND DFPlayer
    - RX3 <-----------> 1K Direnç <-----------> TX DFPlayer
    - TX3 <-----------> RX DFPlayer
- ULN2003A
    - IN1 <----------->	4
    - IN2 <----------->	5
    - IN3 <----------->	6
    - IN4 <----------->	7
    - VCC <----------->	5V
    - GND <----------->	GND
- RC522
    - SDA  <-----------> 10
    - SCK  <-----------> 52
    - MOSI <-----------> 51
    - MISO <-----------> 50
    - GND  <-----------> GND
    - RST  <-----------> 9
    - 3.3V <-----------> 3.3V
- POT1 (Ses Kontrol)
    - A15
- POT2 (Müzik Kontrol)
    - A14
#### 3D Modellerin STL Dosyaları
`/src/modeller/`
- Gövde
- Kapak
- Başlık
- Başlık Adaptörü
- Disk
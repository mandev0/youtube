# Frigate Kurulum Dokümanı

🗺 Youtube: Ev için Kamera Sistemleri, Benim Deneyimlerim, Frigate Aracı ve Kurulumu (https://www.notion.so/Ev-i-in-Kamera-Sistemleri-Benim-Deneyimlerim-Frigate-Arac-ve-Kurulumu-12aa0b8fa0ce804da017c12fe3ff0f80?pvs=21)

# Konu

Bu dokümanda Frigate aracının docker compose ile kurulumu anlatılmıştır.

# Yapı

Ayarlara başlamadan önce klasör/dosya yapımıza bakalım. Ben dosyaları “/opt” altında tutuyorum. Burada “frigate-demo” adında bir klasör, onun altındaysa “config” ve “data” adında iki tane klasör var. “config” klasörünün altında “docker-compose” adında Yaml dosyası, “data” altındaysa Frigate a ait config dosyası var.

```python
- /opt
	- /frigate-demo
		- /config
			- docker-compose.yaml
		- /data
			- config.yml
```

## Docker Compose Dosyası

### Örnek dosya (docker-compose.yaml)

```yaml
name: frigate-demo
services:
  frigate:
    container_name: frigate
    restart: unless-stopped
    image: ghcr.io/blakeblackshear/frigate:stable
    shm_size: "64mb"
    volumes:
      - ../data/config.yml:/config/config.yml:ro
      - /mnt/hdd/cctv/:/media/frigate
    ports:
      - "5002:5000"
      - "1936:1935" # RTMP feeds
    environment:
      FRIGATE_RTSP_PASSWORD: "ChangeME"
      TZ: Europe/Istanbul
```

Yukarıdaki dosyada yorum girilen alanları kendi yapınıza ve ihtiyaçlarınıza göre düzenleyebilirsiniz. Gerekli yorumlar kolonun sağında mevcut. Gerekli değişiklikleri yaptıktan sonra dosyayı kayıt edip sonraki adıma geçebiliriz.

## Frigate Config Dosyası

### Örnek Dosya (config.yml)

```yaml
go2rtc:
  streams:
    entry_cam:
      - rtsp://RTSP_USER:RTSP_PASSWORD@RTSP_IP:RTSP_PORT/stream1
      - "ffmpeg:entry_cam#audio=opus"
    entry_cam_low:
      - rtsp://RTSP_USER:RTSP_PASSWORD@192.168.10.10:554/stream2
      - "ffmpeg:entry_cam_low#audio=opus"

cameras:
  entry_cam:
    ffmpeg:
      output_args:
        record: preset-record-generic-audio-copy
      inputs:
        - path: rtsp://127.0.0.1:8554/entry_cam
          input_args: preset-rtsp-restream
          roles:
            - record
        - path: rtsp://127.0.0.1:8554/entry_cam_low
          input_args: preset-rtsp-restream
          roles:
            - detect

    record:
      enabled: True
      sync_recordings: True
      retain:
        days: 3
        mode: motion
      events:
        retain:
          default: 7
          mode: motion
    snapshots:
      enabled: True
    detect:
      width: 640
      height: 480
```

Şu an gördüğünüz config dosyasında standartın biraz dışında bir yapı var. Kameraları iki adımda işlemeye başlıyoruz yani ne demek istiyorum. İlk bölümde kameraları Frigate e tanımlayıp ikinci kısımda ise ilk kısımdaki tanımlamaları kullanıyoruz. Bunun sebebi aslında kameraları birden fazla defa farklı yayınlarda kullanabilmek. Şu an lazım olmayabilir ancak lazım olduğunda işlerimizi daha kolay hale getirecektir.

Bu dosyayla işimizde bitti şimdi kayıt edip sistemi ayağa kaldırma adımına geçelim.

```yaml
# Öncelikle "config" klasörüne geçelim. Bu klasör "docker-compose.yaml" dosyamızın olduğu klasör. Bu klasördeyken aşağıdaki komutu çalıştırıcaz.
docker compose up -d

# Bu komutla Frigate sistemi ayağa kalmış oldu. Şimdi aşağıdaki URL formatını kullanarak ara yüze erişim sağlayabilirsiniz.
# Ör: http://192.168.10.10:5002
http://Frigate_Sunucu_IP_Adresi:Web_Arayüzü_Port_Bilgisi
```
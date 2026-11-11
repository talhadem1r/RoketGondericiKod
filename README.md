# Roket Uçuş Kontrol Bilgisayarı (UKB)

ATÜ-EBC Bürküt Roket Takımı'nın **TEKNOFEST 2025 Roket Yarışması – Orta İrtifa** kategorisi için geliştirdiği uçuş kontrol bilgisayarı yazılımı. Roketin irtifasını ve duruşunu anlık izler, uçuş evresine göre paraşütleri tetikler ve telemetri verisini LoRa ile yer istasyonuna gönderir.

## Özellikler
- BMP180 basınç sensörüyle irtifa hesaplama
- MPU6050 ile ivme, açısal hız ve eğim açısı ölçümü
- GPS ile enlem, boylam ve GPS irtifası
- Tüm sensör verilerinde Kalman filtresi ile gürültü azaltma
- Uçuş evresi algoritması ile iki aşamalı paraşüt kontrolü
- LoRa E22 ile ~200 ms aralıkla telemetri gönderimi

## Donanım ve Bağlantılar
| Bileşen | Bağlantı |
|---|---|
| [Arduino kart modeli] | Ana kontrolcü |
| BMP180 (basınç/irtifa) | I2C |
| MPU6050 (ivme/jiroskop) | I2C |
| GPS modülü | SoftwareSerial – D6 / D5 |
| Ebyte E22 LoRa | SoftwareSerial – D9 / D8 |
| Sürüklenme paraşütü tetik | D12 |
| Ana paraşüt tetik | D11 |

## Uçuş Algoritması
1. **Yükselme:** İrtifa artarken durum `1` korunur.
2. **Tepe noktası sonrası:** İrtifa düşüyor, roket eğimi ±50° dışına çıkmış ve irtifa 1500 m'nin üzerindeyse, art arda 5 ölçümle doğrulanınca **D12** tetiklenir → durum `2`.
3. **Ana paraşüt:** Durum `2` iken irtifa 550 m'nin altına inince **D11** tetiklenir → durum `4`.

## Telemetri Paketi
İrtifa, ivme (X/Y/Z), açısal hız (X/Y/Z), eğim açısı, GPS irtifası, enlem, boylam ve uçuş durumu tek bir `struct` olarak LoRa üzerinden gönderilir.

## Kullanılan Kütüphaneler
Adafruit BMP085 Unified · Adafruit Unified Sensor · MPU6050_tockn · EByte LoRa E22 · TinyGPS++ · SimpleKalmanFilter

## Takımdaki Rolüm
[Örn: UKB yazılımının geliştirilmesi, sensör entegrasyonu ve uçuş algoritmasının tasarımı]

> Görev yükü yazılımı: [GorevYuku](https://github.com/talhadem1r/GorevYuku)

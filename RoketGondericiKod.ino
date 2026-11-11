//ATÜ-EBC BÜRKÜT ROKET TAKIMI ÖZGÜN UKB YAZILIMI
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>  //bmp
#include <MPU6050_tockn.h>  //mpu
#include "LoRa_E22.h"
#include <SoftwareSerial.h>
SoftwareSerial mySerial(9, 8);  //rx=10
LoRa_E22 e22ttl(&mySerial, UART_BPS_RATE_9600);
MPU6050 mpu6050(Wire);

#include <TinyGPS++.h>
TinyGPSPlus gps; // TinyGPS++ nesnesi
SoftwareSerial gpsSerial(6, 5); // GPS modülünün RX ve TX pinleri

#include <SimpleKalmanFilter.h>
long timer1, timer2;   //mpu için

#define seaLevelPressure 1020

sensors_event_t event;  //basınç sensörü nesnesi

float pitch;
float roll;
float yaw;
float pitch_angle;
float roll_angle;

long timer = 0;       // mpu için
float count = 0;
float count2 = 0;
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(1015);

SimpleKalmanFilter altitudeKalmanFilter(1, 0.01, 0.5);
SimpleKalmanFilter accXKalmanFilter(1, 0.01, 0.5);
SimpleKalmanFilter accYKalmanFilter(1, 0.01, 0.5);
SimpleKalmanFilter accZKalmanFilter(1, 0.01, 0.5);
SimpleKalmanFilter gyroXKalmanFilter(1, 0.01, 0.5);
SimpleKalmanFilter gyroYKalmanFilter(1, 0.01, 0.5);
SimpleKalmanFilter gyroZKalmanFilter(1, 0.01, 0.5);
SimpleKalmanFilter angleKalmanFilter(1, 0.01, 0.5);
SimpleKalmanFilter angle0KalmanFilter(1, 0.01, 0.5);
float Vx ;
float irtifaBefore;
float currentTime;
float prevTime = 0;
float deltaTime;

struct Signal
{

  float irtifa;
  float accX;
  float accY;
  float accZ;
  float gyroX;
  float gyroY;
  float gyroZ;
  float angle;
  float gps_irtifa;
  float enlem;
  float boylam;
  uint8_t durum = 1;

} data_t;

void setup(void)
{
  Serial.begin(9600);
  gpsSerial.begin(9600); // GPS modülü baud hızı
  e22ttl.begin(); //LoRa Modülü Başlat
  Wire.begin();    // i2c iletişimi
  
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  // Initialise the sensor
  if (!bmp.begin())
  {
    Serial.print("BMP180 şimdi çalışmak istemiyor. Kablolarını kontrol et lütfen!");
    while (0);
    irtifaBefore = 0;  //setup içinde tanımlamak gerekir
  }

  mpu6050.begin();   //MPU6050 
  mpu6050.calcGyroOffsets(true);
}

void loop(void)
{
  gpsSerial.listen();
  while (gpsSerial.available()) {
    if (gps.encode(gpsSerial.read())) { // GPS verilerini oku
      if (gps.location.isUpdated()) { // Enlem ve boylam değerleri güncellendi mi?cf

        data_t.enlem = gps.location.lat(); // 6 haneli ondalık gösterim

        data_t.boylam = gps.location.lng(); // 6 haneli ondalık gösterim

        data_t.gps_irtifa = gps.altitude.meters();
      }
    }
  }

  // basınç sensörü verileri alınıyor
  bmp.getEvent(&event);
  float fsiz_irtifa;
  if (event.pressure)
  {
    fsiz_irtifa = bmp.pressureToAltitude(seaLevelPressure, event.pressure);
  }
  data_t.irtifa  = altitudeKalmanFilter.updateEstimate(fsiz_irtifa);

  //----------------------------------mpu 6050----------------------------------------

  mpu6050.update();

  if (millis() - timer > 80)
  {
    float fsiz_accX = mpu6050.getAccX();
    data_t.accX = accXKalmanFilter.updateEstimate(fsiz_accX);

    float fsiz_accY = mpu6050.getAccY();
    data_t.accY = accYKalmanFilter.updateEstimate(fsiz_accY);

    float fsiz_accZ = mpu6050.getAccZ();
    data_t.accZ = accZKalmanFilter.updateEstimate(fsiz_accZ);


    float fsiz_gyroX = mpu6050.getGyroX();
    data_t.gyroX = gyroXKalmanFilter.updateEstimate(fsiz_gyroX);

    float fsiz_gyroY = mpu6050.getGyroY();
    data_t.gyroY = gyroYKalmanFilter.updateEstimate(fsiz_gyroY);

    float fsiz_gyroZ = mpu6050.getGyroZ();
    data_t.gyroZ = gyroZKalmanFilter.updateEstimate(fsiz_gyroZ);

    pitch = atan2(data_t.accX, sqrt(data_t.accY * data_t.accY + data_t.accZ * data_t.accZ)) * 180.0 / PI;
    roll = atan2(data_t.accY, sqrt(data_t.accX * data_t.accX  + data_t.accZ * data_t.accZ)) * 180.0 / PI;
    yaw = atan2(data_t.accZ, sqrt(data_t.accX * data_t.accX + data_t.accY * data_t.accY)) * 180.0 / PI;

    pitch_angle = pitch - 90.0;
    roll_angle = roll - 90.0;

    timer = millis();
  }

  float fsiz_angle = mpu6050.getAngleX() - 90;
  float fsiz_angle0 = mpu6050.getAngleZ() ;
  data_t.angle = angleKalmanFilter.updateEstimate(fsiz_angle);
  float angle0 = angle0KalmanFilter.updateEstimate(fsiz_angle0);
  

//----------------------------------ALGORITMA----------------------------------------

  if (irtifaBefore - data_t.irtifa < -100  )    // roket yükseliyor
  {
    data_t.durum = 1; // durumunu koru
    Serial.println("YUKSELIYOR");
  }

  else if ( irtifaBefore - data_t.irtifa > 4 &&  (!(-50 < data_t.angle && data_t.angle < 50 )||!(-50 < angle0 && angle0 < 50 ) ) && data_t.irtifa > 1500) // roket alçalıyor
  {
  count = count + 1;
    if (count > 4){
      digitalWrite(12, HIGH);
      data_t.durum = 2;
      Serial.println("ALCALIYOR");
    }
  }

 /* else if ( irtifaBefore - data_t.irtifa > 10 && data_t.durum == 1 && data_t.irtifa <= 1500  ) // roket alçalıyor //HAKEMDE AÇ!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  {
    count2 = count2 + 1;
    if (count2 > 4) {
      digitalWrite(11, HIGH);
      data_t.durum = 3;
      Serial.println("ALCALIYOR");
    }
  }*/


  else if (data_t.durum == 2 && data_t.irtifa <= 550)
  {
    digitalWrite(11, HIGH);
    data_t.durum = 4;
    Serial.println("TUM PARASUTLER ACIK");
    exit;
  }

  irtifaBefore = data_t.irtifa;



//----------------------------------LoRa Veri İletimi----------------------------------------
  ResponseStatus rs = e22ttl.sendFixedMessage(0, 8, 47, &data_t, sizeof(Signal));
  Serial.println(data_t.enlem);
  Serial.println(data_t.boylam);
  delay(200);

}

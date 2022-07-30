#include "Wire.h"           //  Подключаем библиотеку для работы с шиной I2C
#include "MAX30105.h"       //  Подключаем библиотеку для работы с модулем
#include "heartRate.h"      //  Подключаем блок для работы с ЧСС (пульс)
MAX30105 PARTICLE_SENSOR;   //  Создаём объект для работы с библиотекой
#include <ESP8266WiFi.h>                                          // esp8266 library
#include <FirebaseArduino.h> 
#define  FIREBASE_HOST "bpm-so2p-android-studio-default-rtdb.firebaseio.com"
#define  FIREBASE_AUTH "MfpXxLrtGqssaQKPIk95Vd1kWChPsSI5fCE0e0rV"
#define WIFI_SSID "Bratsk_5" //provide ssid (wifi name)
#define WIFI_PASSWORD "$c*Wi-SdOP!2745" //wifi password

//--------------------------//
long lastBeat = 0;          //  Время последнего зафиксированного удара
float beatsPerMinute;       //  Создаём переменную для хранения значения ЧСС
int beatsPerMinut;
//----------------------------------------------------//
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  if (!PARTICLE_SENSOR.begin()) {                     //  Инициируем работу с модулем. Если инициализация не прошла, то
    while (1);}
delay(1000);
PARTICLE_SENSOR.setup();                            //  Устанавливаем настройки для сенсора по умолчанию
PARTICLE_SENSOR.setPulseAmplitudeRed(0x0A);         //  Выключаем КРАСНЫЙ светодиод для того, чтобы модуль начал работу
PARTICLE_SENSOR.setPulseAmplitudeGreen(0);          //  Выключаем ЗЕЛЁНЫЙ светодиод
PARTICLE_SENSOR.enableDIETEMPRDY();
  delay(1000);
}
//------------------------------------------------------//
void loop() {
  long irValue = PARTICLE_SENSOR.getIR();               //  Считываем значение отражённого ИК-светодиода (отвечающего за пульс) и
  if (checkForBeat(irValue) == true) {                  //  если пульс был зафиксирован, то
    long delta = millis() - lastBeat;                   //  находим дельту по времени между ударами
    lastBeat = millis();                                //  Обновляем счётчик
    beatsPerMinute = 60 / (delta / 1000.0);             //  Вычисляем количество ударов в минуту
 beatsPerMinut= beatsPerMinute;
  }
  
  if(beatsPerMinute>25){ 
  Firebase.setString("Sensor", String(beatsPerMinut));
  Firebase.setString("Sensor2",String(PARTICLE_SENSOR.readTemperature(),0));
  delay(1500);}                                   //  Выводим в монитор последовательного порта переход на новую строку
}

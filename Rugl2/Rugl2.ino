#define  MAX_BRIGHTNESS 255                         //  Задаём переменную максимальной яркости свечения светодиода
//--------------------------------------------------//
#include "Wire.h"                                   //  Подключаем библиотеку для работы с шиной I2C
#include "MAX30105.h"                               //  Подключаем библиотеку для работы с модулем
#include "spo2_algorithm.h"                         //  Подключаем блок работы с насыщением крови кислородом
#include "heartRate.h"                              //  Подключаем блок для работы с ЧСС (пульс)
MAX30105 PARTICLE_SENSOR;                           //  Создаём объект для работы с библиотекой
#include <ESP8266WiFi.h>                            // esp8266 library
#include <FirebaseArduino.h> 
#define  FIREBASE_HOST "bpm-so2p-android-studio-default-rtdb.firebaseio.com"    // адрес сайта firebase
#define  FIREBASE_AUTH "MfpXxLrtGqssaQKPIk95Vd1kWChPsSI5fCE0e0rV"               // ключ доступа
#define WIFI_SSID "Bratsk_5"                                                    //provide ssid (wifi name)           
#define WIFI_PASSWORD "$c*Wi-SdOP!2745"                                         //wifi password
//#define WIFI_SSID "RuGl_bin2"                                                 //provide ssid (wifi name)
//#define WIFI_PASSWORD "Wi-283!-283"                                           //wifi password
//--------------------------------------------------//
long lastBeat = 0;                                  //  Время последнего зафиксированного удара
float beatsPerMinute;                               //  Создаём переменную для хранения значения ЧСС
int beatsPerMinut;
uint32_t irBuffer[25];                              //  32-битный массив данных от сенсора со значениями от ИК-светодиода
uint32_t redBuffer[25];                             //  32-битный массив данных от сенсора со значениями от красного светодиода
String sost[8]={"on","Conecting...","Conect","Fir con","Got ism","s","s p","clicking?"};    // Переменная с состояними подкючения
String pol="men";
//--------------------------------------------------//
int32_t bufferLength;                               //  длина буфера данных
int32_t spo2;                                       //  значение SpO2 (насыщенности крови кислородом)
int8_t  validSPO2;                                  //  флаг валидности значений сенсора по SpO2
int32_t heartRate;                                  //  значение ЧСС
int8_t  validHeartRate;                             //  флаг валидности значений сенсора по ЧСС
//--------------------------------------------------//
void SendData(String dev, String data)
{
  Serial.print(dev);                                // Отправляем данные dev(номер экрана, название переменной) на Nextion
  Serial.print("=");                                // Отправляем данные =(знак равно, далее передаем сами данные) на Nextion 
  Serial.print(data);                               // Отправляем данные data(данные) на Nextion
  comandEnd();
  dev = "";                                         // Очищаем переменную
  data = "";                                        // Очищаем переменную
}
//----------------------------------------------------//
void comandEnd()
{
  for (int i = 0; i < 3; i++) {
    Serial.write(0xff);}
}
//----------------------------------------------------//
void puls(){

    long irValue = PARTICLE_SENSOR.getIR();           //  Считываем значение отражённого ИК-светодиода (отвечающего за пульс) и
  if (checkForBeat(irValue) == true) {                //  если пульс был зафиксирован, то
    long delta = millis() - lastBeat;                 //  находим дельту по времени между ударами
    lastBeat = millis();                              //  Обновляем счётчик
    beatsPerMinute = 60 / (delta / 1000.0);           //  Вычисляем количество ударов в минуту
     beatsPerMinut= beatsPerMinute;
  }  
}
//----------------------------------------------------//
void spo(){
   bufferLength = 25;                                 //  Устанавливаем длину буфера равным 25 
                                                      //  считываем первые 25 значений и определяем диапазон значений сигнала:
  for (byte i = 0 ; i < bufferLength ; i++) {         //  проходим в цикле по буферу и
    while (PARTICLE_SENSOR.available() == false)      //  отправляем сенсору запрос на получение новых данных
      PARTICLE_SENSOR.check();
    redBuffer[i] = PARTICLE_SENSOR.getRed();          //  Записываем в массив значения сенсора, полученные при работе с КРАСНЫМ светодиодом
    irBuffer[i] = PARTICLE_SENSOR.getIR();            //  Записываем в массив значения сенсора, полученные при работе с ИК      светодиодом
    PARTICLE_SENSOR.nextSample();                     //  Как только в буфер было записано 25 значений - отправляем сенсору команду начать вычислять значения ЧСС и SpO2
  }

  //  Вычисляем значения ЧСС и SpO2 по первым полученным 25 значениям:
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
}
//----------------------------------------------------//
void setup() {
  Serial.begin(9600);                                 //Устанавливаем скорость обмена данными с дисплеем
  Serial.setTimeout(100);                             //Устанавливаем время ожидание получения ответа стандарт(1000)
  delay(200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);               //Производим подключение к WiFi сети
  delay(300);
  SendData("zg.t1.txt","\""+String(sost[1])+"\"");
  delay(100);
    while (WiFi.status() != WL_CONNECTED) { 
    delay(300);
  }
  Serial.print("page log");comandEnd();               //Переходим на главную страницу
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  SendData("log.t1.txt","\""+String(sost[3])+"\"");
  delay(500);
  if (!PARTICLE_SENSOR.begin()) {                     //  инициируем работу с сенсором. Если этого не произошло, то
    while (1);                                        //  останавливаем дальнейшее выполнение скетча
  }
  
//  PARTICLE_SENSOR.setup(60, 4, 2, 100, 411, 4096);
  PARTICLE_SENSOR.setup(); 
  PARTICLE_SENSOR.setPulseAmplitudeRed(0x0A);         //  Выключаем КРАСНЫЙ светодиод для того, чтобы модуль начал работу
  PARTICLE_SENSOR.setPulseAmplitudeGreen(0);          //  Выключаем ЗЕЛЁНЫЙ светодиод 
  PARTICLE_SENSOR.enableDIETEMPRDY();                 //Подключаем считывание температуры
 SendData("log.t1.txt","\""+String(sost[7])+"\"");
Serial.readString();
}
//----------------------------------------------------//
void loop() {
  //----------------------------------------------------//
if(Serial.available()>0){
 String pul = Serial.readString();
  if(pul=="h1"){pol="women";}
  else if(pul=="h0") {pol="men";}
  else {pol="err";}
  //----------------------------------------------------//
SendData("log.t1.txt","\""+String(sost[4])+"\"");
while(spo2<80)spo();
SendData("log.t1.txt","\""+String(sost[5])+"\"");
while(beatsPerMinut<30)puls();
SendData("log.t1.txt","\""+String(sost[6])+"\"");
//if(spo2>80 && beatsPerMinut>30){
float te=(PARTICLE_SENSOR.readTemperature()+2.4);
  Firebase.setString("Sensor", String(beatsPerMinut));
 SendData("log.pu.txt","\""+String(beatsPerMinut)+"\"");
  Firebase.setString("Sensor2",String(spo2));
 SendData("log.sp.txt","\""+String(spo2)+"%"+"\"");
  Firebase.setString("Sensor3",String(te,0));
 SendData("log.kg.txt","\""+String(te,1)+"\"");
 Firebase.setString("pol", String(pol));
 beatsPerMinut=0;
}}

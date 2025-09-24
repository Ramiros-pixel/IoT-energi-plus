struct tm timeinfo;
#define BLYNK_TEMPLATE_ID "TMPL69Zqzpcu3"
#define BLYNK_TEMPLATE_NAME "CAHAYA DARI TIMUR"
#define BLYNK_AUTH_TOKEN "INLSAqCjxg9yYpxhsaQJ2pvbNsRhP_QH"

#include <IRremoteESP8266.h>
#include <IRsend.h>
IRsend irsend(13);
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <MHZ19.h>
#include <HardwareSerial.h>
#include <BH1750.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "DHT.h"
#include "time.h"
#include <ESP32Servo.h>
BlynkTimer timer;
// ====== NTP Server & Zona Waktu ======
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;  // GMT+7 = WIB
const int daylightOffset_sec = 0;
// ====== Servo & Jadwal Otomatisasi ======
int pagiHour = 6;       // mulai 07:30
int pagiMinute = 00;
int siangHour = 11;       // berhenti 21:45
int siangMinute = 30;
int soreHour = 15;
int soreMinute = 00;
int stopHour = 18;
int stopMinute =00;
//-------------------Kode Hex IR AC-----------------------
uint16_t rawNaik[] = {3456, 1724, 424, 472, 400, 1300, 420, 456};  
uint16_t rawTurun[] = {3440, 1720, 420, 470, 402, 1300, 421, 457};  
//-------------------KANOPI-----------------------------

bool ac = false;
unsigned long lastChangeTime = 0;
const unsigned long debounceDelay = 3000; 

int hour ;
int minute;

// ------------------ WIFI DAN BLYNK -------------------
char ssid[] = "ARZ";
char pass[] = "akirey0109";
// ---------define seluruh----------------------
float suhu = 0;
float hum = 0;
float cdd = 0;
float lux = 0;
float co2 = 0;
float temp = 0;
float celcius = 0;
float humadity = 0;
float co2_2 = 0; 
float temp_2 = 0;

// ------------------ MH-Z19 -------------------
#define RX_PIN 16
#define TX_PIN 17
//------------------ MH-Z19-2 -------------------
#define RX_PIN2 32
#define TX_PIN2 33
HardwareSerial mySerial(1);
HardwareSerial mySerial2(2);
MHZ19 mhz19;
MHZ19 mhz19_2;
// ------------------ SENSOR DAN PIN -------------------
#define PINDHT1 15
#define PINDHT  23
#define tipeDHT DHT22
#define relayy 4
#define sensorHujan 34
#define Pin_kanopi 19
DHT dht1(PINDHT1, tipeDHT);
DHT dht(PINDHT, tipeDHT);
BH1750 lightMeter;
Servo kanopi;
bool otomatisasiKanopi = false;

// ------------------Variable CDD -------------------
float datasuhu[100];
int indexsuhu = 0;
bool cekdata = false;
float total = 0.0;
float suhuacuan = 26.0;
int i;

const char spreadsheetId[] = "https://script.google.com/macros/s/AKfycbx0_IaPwqeUzIFju1mWLxe3_BQmBBSBHmwwsXFwa9VVM1guz0b-Q5kEYFriBIA9mLEjpQ/exec";
int otomatisasi = 0;          

// ------------------ FUNGSI -------------------
void kirimKeSpreadsheet(float suhu, float hum, float cdd,float lux,float co2, float temp,float celcius, float humadity, float co2_2, float temp_2) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(spreadsheetId);
    url += "?suhu=" + String(suhu,2);
    url += "&hum=" + String(hum,2);
    url += "&cdd=" + String(cdd,2);
    url += "&lux=" + String(lux,2);
    url  += "&co2=" + String(co2,2);
    url += "&temp=" + String(temp,2);
    url += "&celcius=" + String(celcius,2);
    url += "&humadity=" + String(humadity,2);
    url += "&co2_2=" + String(co2_2,2);
    url += "&temp_2=" + String(temp_2,2);

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) Serial.println("Data terkirim ke Google Sheet!");
    else Serial.printf("Gagal kirim. Kode: %d\n", httpCode);
    http.end();
  } else {
    Serial.println("WiFi belum terkoneksi.");
  }
}
//debug

bool updateTime(int &hour, int &minute) {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("❌ Gagal ambil waktu dari NTP");
    return false;
  }
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  Serial.print("✅ Waktu berhasil: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
  return true;
}

//------------------ Fungsi Co2 -------------
void sendSensorluar(){
  co2_2 = mhz19_2.getCO2();
  temp_2 = mhz19_2.getTemperature();
  int err = mhz19_2.errorCode;
    Serial.print("CO2 luar (ppm): ");
    Serial.print(co2_2);
    Serial.print("Suhu sensor suhu 2 :");
    Serial.print(temp_2);
    Serial.print(err);

    Blynk.virtualWrite(V10, co2_2);
    Blynk.virtualWrite(V11,temp_2);

}
void sendSensor() {
co2 = mhz19.getCO2();
temp = mhz19.getTemperature();
int err = mhz19.errorCode;

  Serial.print("CO2 (ppm): ");
  Serial.print(co2);
  Serial.print(" | Suhu Sensor: ");
  Serial.print(temp);
  Serial.print(" | Kode Error: ");
  Serial.println(err);

  Blynk.virtualWrite(V2, co2);
  Blynk.virtualWrite(V3, temp);
}


//------------------ Fungsi DHT -------------
void suhusuhu() {
 celcius = dht.readTemperature();
 humadity = dht.readHumidity();

  if (isnan(celcius) || isnan(humadity)) {
    Serial.println("sensor dht luar error");
    return;
  }
  Serial.println("sensor dht luar berhasil");
  Blynk.virtualWrite(V5, celcius);
  Blynk.virtualWrite(V6, humadity);
}

void suhu1suhu() {
float celcius1 = dht1.readTemperature();
float humadity1 = dht1.readHumidity();

  if (isnan(celcius1) || isnan(humadity1)) {
    Serial.println("sensor dht dalam error");
    return;
  }
  Serial.println("sensor dht dalam berhasil");
  Blynk.virtualWrite(V0, celcius1);
  Blynk.virtualWrite(V1, humadity1);
}
//------------------ Fungsi CDD -------------
void simpansuhu1() {
suhu = dht1.readTemperature();
hum = dht1.readHumidity();
  if (!isnan(suhu)) {
    datasuhu[indexsuhu++] = suhu;
    if (indexsuhu >= 100) {
      indexsuhu = 0;
      cekdata = true;
    }
  }
}

void coolingdds() {
  if (!cekdata) return;

  float suhumin = datasuhu[0];
  float suhumax = datasuhu[0];
  for (i = 0; i < 100; i++) {
    if (datasuhu[i] < suhumin) suhumin = datasuhu[i];
    if (datasuhu[i] > suhumax) suhumax = datasuhu[i];
  }

  float rata = (suhumin + suhumax) / 2.0;
  cdd = (rata > suhuacuan) ? (rata - suhuacuan) : 0;

  if (cdd > 0) total += cdd;
  Blynk.virtualWrite(V7, cdd);

  float suhu = dht1.readTemperature();
  float hum = dht1.readHumidity();
 
}

//------------------ AC MULAI PROGRAM --------------
void kirimSuhu(uint16_t* rawData, int length) {
  irsend.sendRaw(rawData, length, 38);
  Serial.println("IR dikirim berdasarkan suhu.");
}

BLYNK_WRITE(V12){
  ac = param.asInt();
  if (ac == 0){
    Serial.println("AC Manual");}
  else if (ac ==1){
    Serial.println("AC otomatisasi");
  }
  }
//------Manual Ac -------------
BLYNK_WRITE(V23) {  
  
  bool tombol = param.asInt();
  if (ac == 0) {
    if (tombol == 1){   // hanya manual
    kirimSuhu(rawNaik, sizeof(rawNaik) / sizeof(rawNaik[0]));
    Serial.println("BLYNK -> Kanopi Naik (Manual)");
  }}
}

BLYNK_WRITE(V22) {
  bool button = param.asInt();
  if (ac == 0) {
    if (button == 1){   // hanya manual
    kirimSuhu(rawTurun, sizeof(rawTurun) / sizeof(rawTurun[0]));
    Serial.println("BLYNK -> Kanopi Turun (Manual)");
  }}
}

void otomatisasiAC(){
 float suhu = dht1.readTemperature();
 float suhuPanas = 26.5;
 float suhuDingin = 25.5;
  if (ac == 1){
     if(millis()- lastChangeTime >= 300000){
      if(suhu < suhuDingin){
        Serial.println("suhu akan dinaikan");
        kirimSuhu(rawNaik, sizeof(rawNaik) / sizeof(rawNaik[0]));
        lastChangeTime = millis();
        Blynk.virtualWrite(V21, "suhu AC naik");
      }
      else if(suhu > suhuPanas){
        Serial.println("suhu akan diturunkan");
        kirimSuhu(rawTurun, sizeof(rawTurun) / sizeof(rawTurun[0]));
        lastChangeTime = millis();
        Blynk.virtualWrite(V21, "suhu AC turun");

 }
 }
}}

//------------------ FUNGSI KANOPI -------------
BLYNK_WRITE(V4) {
   otomatisasiKanopi = param.asInt();
}



void luxx() {
  lux = lightMeter.readLightLevel();
  if (lux < 0 || isnan(lux)) {
    Blynk.virtualWrite(V8, "Sensor Error");
    return;
  }
  Serial.print(lux);
  Blynk.virtualWrite(V8, lux);
}

//------------------Manual Kanopi-------------------
BLYNK_WRITE(V20) {
  
  if (otomatisasiKanopi == 0) {   
    int drajat  = param.asInt();
    kanopi.write(drajat);
    Serial.print("Drajat Kanopi (Manual Blynk): ");
    Serial.println(drajat);
  } else {
    Serial.println("Mode otomatis (sensor hujan aktif), kontrol Blynk diabaikan");
  }
}
void getWaktuSekarang(int &hour, int &minute) {
  struct tm now;
  if (getLocalTime(&now)) {
    hour = now.tm_hour;
    minute = now.tm_min;
  } else {
    Serial.println("❌ Gagal ambil waktu dari NTP");
    hour = -1;  // tandai error
    minute = -1;
  }
}

//  Fungsi Sensor Hujan + Logika Kanopi
void Sensorhujan() {
  int hujan_sensor = digitalRead(sensorHujan);
  unsigned long currentMillis = millis();

  int hour, minute;
  getWaktuSekarang(hour, minute);   // selalu update waktu real

  Serial.println("===== Sensor Hujan & Waktu =====");
  Serial.print("Nilai sensor hujan: ");
  Serial.println(hujan_sensor);
  Serial.print("Jam sekarang: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
  Serial.print("Millis: ");
  Serial.println(currentMillis);
  Serial.print("LastChangeTime: ");
  Serial.println(lastChangeTime);

  //  Kirim status hujan ke Blynk
  if (hujan_sensor == 0) {
    Blynk.virtualWrite(V13, "Terdeteksi hujan");
  } else {
    Blynk.virtualWrite(V13, "Terdeteksi tidak hujan");
  }

  //  Kontrol otomatisasi kanopi
  if (otomatisasiKanopi == 1) {
    if (hujan_sensor == 0 && (currentMillis - lastChangeTime > debounceDelay)) {
      Serial.println("Deteksi hujan || Menutup kanopi...");
      kanopi.write(10); 
      Blynk.virtualWrite(V14, "Menutup (hujan)");
      lastChangeTime = currentMillis;
    } 
    else if (hujan_sensor == 1 && (currentMillis - lastChangeTime > debounceDelay)) {
      // Jadwal berdasarkan jam
      if ((hour > pagiHour || (hour == pagiHour && minute >= pagiMinute)) &&
          (hour < siangHour || (hour == siangHour && minute <= siangMinute))) {
        kanopi.write(30);
        Serial.println("pagi");
        Blynk.virtualWrite(V14, "Pagi");
      } 
      else if ((hour > siangHour || (hour == siangHour && minute >= siangMinute)) &&
               (hour < soreHour || (hour == soreHour && minute <= soreMinute))) {
        kanopi.write(60);
        Serial.println("Siang");
        Blynk.virtualWrite(V14, "Siang");
      } 
      else if ((hour > soreHour || (hour == soreHour && minute >= soreMinute)) &&
               (hour < stopHour || (hour == stopHour && minute <= stopMinute))) {
        kanopi.write(90);
        Serial.println("Sore");
        Blynk.virtualWrite(V14, "Sore");
      } 
      else {
        kanopi.write(0);
        Serial.println("Malam");
        Blynk.virtualWrite(V14, "Malam");
      }

      Serial.println(" Kanopi otomatis bergerak sesuai jadwal & cuaca");
      lastChangeTime = currentMillis;
    }
  }
}

void finaldata(){
  celcius = dht.readTemperature();
  humadity = dht.readHumidity();

  kirimKeSpreadsheet(suhu, hum, cdd, lux, co2, temp, celcius, humadity, co2_2, temp_2);
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(21, 22);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  irsend.begin();
  mySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  mySerial2.begin(9600, SERIAL_8N1, RX_PIN2,TX_PIN2 );
  mhz19.begin(mySerial);
  mhz19.autoCalibration(false);
  mhz19_2.begin(mySerial2);

  // Sinkronisasi waktu dari NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Sinkronisasi waktu...");

  dht.begin();
  dht1.begin();

  pinMode(relayy, OUTPUT);
  pinMode(sensorHujan, INPUT);
  kanopi.attach(Pin_kanopi);
  // Inisialisasi Blynk (sekali saja!)
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
     
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Gagal ambil waktu");
    delay(2000);
    return;
  }
  


  // Timer tugas
  timer.setInterval(60000L, finaldata);
  timer.setInterval(5000L, sendSensor); 
  timer.setInterval(5000L, Sensorhujan);
  timer.setInterval(5000L, sendSensorluar);// CO2
  timer.setInterval(5000L, suhu1suhu);     // DHT dalam
  timer.setInterval(5000L, suhusuhu);      // DHT luar
  timer.setInterval(300L, simpansuhu1);    // Ambil data suhu
  timer.setInterval(30000L, coolingdds);   // Hitung CDD
  timer.setInterval(5000L, luxx);          // BH1750
 
}


void loop() {
  Blynk.run();
  timer.run();
  
}
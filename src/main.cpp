/*UNTUK SENSOR SUHU LUAR MENGGUNAKAN KONFIGURASI STREAM UNTUK V 10 DAN 11 
SEBAGAI PEMBACAAN CO2 DAN SUHU PADA SENSOR
PIN 13 UNTUK ESP32

PERHATIKAN UNTUK OTOMATISASI BLYNK BELUM ADA V YANG SESUAI TOLONG SESUAIKAN
KANOPI
*/
#define BLYNK_TEMPLATE_ID "TMPL6fsU0p9IY"
#define BLYNK_TEMPLATE_NAME "Smart Energy Plus Monitoring"
#define BLYNK_AUTH_TOKEN "Z6HkJe8ogi0dMJOCQT3u2evirF-r3m2M"
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
#include <ESP32Servo.h>
BlynkTimer timer;
//-------------------Kode Hex IR AC-----------------------
uint16_t rawNaik[] = {3456, 1724, 424, 472, 400, 1300, 420, 456};  // example
uint16_t rawTurun[] = {3440, 1720, 420, 470, 402, 1300, 421, 457};  // example
//-------------------KANOPI-----------------------------
#define TH_RAIN 2900
#define TH_DRY  3100
const int FILTER_COUNT = 5;
int sensorBuf[FILTER_COUNT];
int bufIdx = 0;
bool status_hujan = false;
bool ac = false;
unsigned long lastChangeTime = 0;
const unsigned long debounceDelay = 3000; 

// ------------------ WIFI DAN BLYNK -------------------
char ssid[] = "Mandala 106";
char pass[] = "Nugraha1";
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
#define PINDHT  2
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

const char spreadsheetId[] = "https://script.google.com/macros/s/AKfycbwa488LMfZ3V59DiYxDYr7m6zrMMdXpRyIhyWgJik9JbPfhoeZJWowqICzkwtlKIzAX3Q/exec";
int otomatisasi = 0;          




// ------------------ FUNGSI -------------------
void kirimKeSpreadsheet(float suhu, float hum, float cdd,float lux,float co2, float temp,float celcius, float humadity, float co2_2, float temp_2) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(spreadsheetId);
    url += "?suhu=" + String(suhu, 2);
    url += "&hum=" + String(hum, 2);
    url += "&cdd=" + String(cdd, 2);
    url += "&lux=" + String(lux,2);
    url  += "&co2=" + String(co2,2);
    url += "&temp=" + String(temp,2);
    url += "&celcius=" + String(celcius,2);
    url += "&humadity=" + String(humadity,2);
    url += "&co2_2=" + String(co2_2, 2);
    url += "&temp_2=" + String(temp_2, 2);

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) Serial.println("Data terkirim ke Google Sheet!");
    else Serial.printf("Gagal kirim. Kode: %d\n", httpCode);
    http.end();
  } else {
    Serial.println("WiFi belum terkoneksi.");
  }
}

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

//------Otomatisasi AC -------------
void kirimSuhu(uint16_t* rawData, int length) {
  irsend.sendRaw(rawData, length, 38);
  Serial.println("IR dikirim berdasarkan suhu.");
}

BLYNK_WRITE(V12){
  ac = param.asInt();
} 

void kirimIRKanopi(uint16_t* rawData, int length) {
  irsend.sendRaw(rawData, length, 38); // 38 kHz biasanya default remote IR
  Serial.println("IR kanopi terkirim.");
}

//------Manual Kanopi -------------
BLYNK_WRITE(V23) {  
  if (ac == 0) {   // hanya manual
    kirimIRKanopi(rawNaik, sizeof(rawNaik) / sizeof(rawNaik[0]));
    Serial.println("BLYNK -> Kanopi Naik (Manual)");
  }
}

BLYNK_WRITE(V22) {  
  if (ac == 0) {   // hanya manual
    kirimIRKanopi(rawTurun, sizeof(rawTurun) / sizeof(rawTurun[0]));
    Serial.println("BLYNK -> Kanopi Turun (Manual)");
  }
}

void otomatisasiAC(){
 float suhu = dht1.readTemperature();
 float acuan_suhu = cdd;
  if (ac == 1){
     if(millis()- lastChangeTime >= 300000){
      if(suhu > acuan_suhu ){
        Serial.println("suhu akan dinaikan");
        kirimSuhu(rawTurun, sizeof(rawTurun) / sizeof(rawTurun[0]));
        lastChangeTime = millis();
      }
      else if(suhu < acuan_suhu){
        Serial.println("suhu akan diturunkan");
        kirimSuhu(rawNaik, sizeof(rawNaik) / sizeof(rawNaik[0]));
        lastChangeTime = millis();

 }
 }
}}

BLYNK_WRITE(V20) { kirimSuhu(rawNaik, sizeof(rawNaik) / sizeof(rawNaik[0])); }
BLYNK_WRITE(V21) { kirimSuhu(rawTurun, sizeof(rawTurun) / sizeof(rawTurun[0])); }
//UNTUK BLYNK BELUM DI SESUAIKAN V nya

BLYNK_WRITE(V10) {
  bool otomatisasiKanopi = param.asInt();
}



void luxx() {
  lux = lightMeter.readLightLevel();
  if (lux < 0 || isnan(lux)) {
    Blynk.virtualWrite(V8, "Sensor Error");
    return;
  }
  Blynk.virtualWrite(V8, lux);
}

//------------------Manual Kanopi-------------------
BLYNK_WRITE(V20) {
  
  if (otomatisasiKanopi == 0) {   // hanya jalan kalau mode manual
    int drajat  = param.asInt();
    kanopi.write(drajat);
    Serial.print("Drajat Kanopi (Manual Blynk): ");
    Serial.println(drajat);
  } else {
    Serial.println("Mode otomatis (sensor hujan aktif), kontrol Blynk diabaikan");
  }
}
void Sensorhujan() {

  int hujan_sensor = analogRead(sensorHujan);
  unsigned long currentMillis = millis();

  Serial.print("Nilai sensor hujan: ");
  Serial.println(hujan_sensor);
  Serial.print("status_hujan: ");
  Serial.println(status_hujan);
  Serial.print("Millis: ");
  Serial.println(currentMillis);
  Serial.print("LastChangeTime: ");
  Serial.println(lastChangeTime);

  // HUJAN - Putar ke arah menutup

  if(otomatisasiKanopi == 1){
    if (hujan_sensor <= 2000 && status_hujan == false && (currentMillis - lastChangeTime > debounceDelay)) {
      Serial.println("Deteksi hujan || Menutup kanopi (putar servo)...");

      kanopi.write(90); 
      delay(2000);     
      kanopi.write(10); 
      Serial.println("Servo berhenti setelah menutup");

      status_hujan = true;
      lastChangeTime = currentMillis;
    }
  
    // CERAH - Putar ke arah membuka
    else if (hujan_sensor > 2000 && status_hujan == true && (currentMillis - lastChangeTime > debounceDelay)) {
      Serial.println("Cerah || Membuka kanopi (putar servo)...");

      kanopi.write(10); 
      delay(2000);       
      kanopi.write(90);  
      Serial.println("Servo berhenti setelah membuka");

      status_hujan = false;
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

  dht.begin();
  dht1.begin();

  pinMode(relayy, OUTPUT);
  pinMode(sensorHujan, INPUT);
  kanopi.attach(Pin_kanopi);

  // Inisialisasi Blynk (sekali saja!)
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

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
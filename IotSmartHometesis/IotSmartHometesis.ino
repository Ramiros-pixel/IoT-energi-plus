/*Beberapa notes code percobaan belum wiring dengan beberapa catatan
untuk pembacaan status sensor Co2 berada pada V2 dan jika diperlukan
dengan temperatur dalam sensor bisa menggunakan pin V3 
||opsi tambahan jika mau ditampikan status sensor
Untuk dht terdapat penggunaan V yaitu pada V0 sebagai suhu dan V1 
sebagai kelembaban
V0
V1
V2
V3
V4
V5
V6
V7 == cooling degree days status
*/

#define BLYNK_TEMPLATE_ID "TMPL66GCr3BNH"
#define BLYNK_TEMPLATE_NAME "Smart Technology Energi Plus"
#define BLYNK_AUTH_TOKEN "-9OboKtlNxrHL4O6Fwk8kPEm3AaAz1h"
// Konfigurasi dengan esp dan blynk serta pemanggilan fungsi wifi
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <BH1750.h>
BH1750 lightMeter;
#include <HTTPClient.h>
#include <MHZ19.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include "DHT.h"
//define dht untuk suhu
#define PINDHT1 15 //PInDalam
#define PINDHT  2 //PinLuar
#define tipeDHT DHT22
#define relayy 4
//parameter cahaya bh1750

//define degree days
float datasuhu[100];
int indexsuhu = 0;
bool cekdata = false;
float total = 0.0;
float suhuacuan = 26.0;
int i;
//int total;
char ssid[] = "";
char pass[] = "";
DHT dht1(PINDHT1,tipeDHT);
DHT dht(PINDHT,tipeDHT);
//otomatisasi
int otomatisasi;
/*Data
char namakolom[] [20] = {"value1","value2","value3"};
const String scriptURL = "https://script.google.com/macros/s/AKfycbwUGXXzKwFyqs_TdHD2x5Z7sfePzlRDmXgWyJjREmN6L8H5x16UcXMrnzcSzUiRCI";
int parameter=3;
*/
const char spreadsheetId[] = "https://script.google.com/macros/s/AKfycbyI9xSdwC4FKMj80JTWPCBMAaa-T1AAnRphRpV-DkvHujKbXNnZc";
HardwareSerial mySerial(1);
MHZ19 myMHZ19;

BlynkTimer timer;
  
//FUNGSI KIRIM KE SPREADSHEET
void kirimKeSpreadsheet(float suhu, float hum, float cdd) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String url = String(spreadsheetId);
    url += "?suhu=" + String(suhu, 2);
    url += "&hum=" + String(hum, 2);
    url += "&cdd=" + String(cdd, 2);

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.println("Data terkirim ke Google Sheet!");
    } else {
      Serial.print("Gagal kirim. Kode: ");
      Serial.println(httpCode);
    }
    http.end();
  } else {
    Serial.println("WiFi belum terkoneksi.");
  }
}



void sendSensor()
{
  int co2 = myMHZ19.getCO2();
  int temp = myMHZ19.getTemperature();
 
  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.print(" ppm | Temp: ");
  Serial.print(temp);
  Serial.println(" °C");

  Blynk.virtualWrite(V2, co2);    
  Blynk.virtualWrite(V3, temp);  
}
//sensor dht
void suhusuhu(){
  float celcius = dht.readTemperature();
  float humadity = dht.readHumidity();

  if (isnan(celcius) || isnan(humadity)){
    String statussuhu = "Sensor Failed";
    Serial.println(statussuhu);
    Blynk.virtualWrite(V4,statussuhu);
    return;
  }
  Serial.println("suhu adalah:");
  Serial.print(celcius);
  String statussuhu1= "Success status sensor";
  Blynk.virtualWrite(V4,statussuhu1);
  Blynk.virtualWrite(V5, celcius);
  Blynk.virtualWrite(V6, humadity);
}

//sensor dht1 DALAM RUANGAN
void suhu1suhu(){
  float celcius1 = dht1.readTemperature();
  float humadity1 = dht1.readHumidity();

  if (isnan(celcius1) || isnan(humadity1)){
    String statussuhu = "Sensor Failed";
    Serial.println(statussuhu);
    Blynk.virtualWrite(V4,statussuhu);
    return;
  }
  Serial.println("suhu adalah:" );
  Serial.print(celcius1);
  String statussuhu1= "Success status sensor";
  Blynk.virtualWrite(V4,statussuhu1);
  Blynk.virtualWrite(V0, celcius1);
  Blynk.virtualWrite(V1, humadity1);
}
//PERHITUNGAN COOLING DEGREE DAYS
void simpansuhu1(){
  float suhu = dht1.readTemperature();
  if (!isnan(suhu)){
    datasuhu[indexsuhu] = suhu;
    indexsuhu++;
  
  if(indexsuhu>= 100){
    indexsuhu=0;
    cekdata = true;
  }
  }
}


//PERCOBAAN
//PERHITUNGAN COOLING DEGREE DAYS FUNGSI2
void coolingdds(){
  if(!cekdata) return;

  float suhumin = datasuhu[0];
  float suhumax = datasuhu[0];
  for(i = 0;i<100;i++){
    if(datasuhu[i]< suhumin) suhumin = datasuhu[i];
    if(datasuhu[i]> suhumax) suhumax = datasuhu[i];
  }

  float rata = ((suhumin + suhumax) / 2.0);
  float cdd = 0;

  if (rata > suhuacuan) {
    cdd = rata - suhuacuan;
    total += cdd;  // perbaiki += bukan =+
  }

  Blynk.virtualWrite(V7, cdd);

  // Kirim ke spreadsheet
  float suhu = dht1.readTemperature();
  float hum = dht1.readHumidity();
  if (!isnan(suhu) && !isnan(hum)) {
    kirimKeSpreadsheet(suhu, hum, cdd);
  }}

//Otomatisasi Cooling degree days

//otomatisasi BH1750 lampu
BLYNK_WRITE(V10){
  otomatisasi = param.asInt();
}void relay() {
  int ldd = lightMeter.readLightLevel();
 
  if (otomatisasi == 1) {
    if (ldd > 0 && ldd<200) {
      digitalWrite(relayy, HIGH);
    } else if(ldd>200) {
      digitalWrite(relayy, LOW);
    }
  }
  if (otomatisasi == 0) {
    BLYNK_WRITE(V11);
  }
}

BLYNK_WRITE(V11) {
  int del = param.asInt();
  digitalWrite(relayy, del);
}


//LUXX
void luxx() {
  float lux = lightMeter.readLightLevel();

  if (lux < 0 || isnan(lux)) {
    Serial.println("[BH1750] Gagal membaca lux");
    Blynk.virtualWrite(V8, "Sensor Error");
    return;
  }

  Serial.print("Lux: ");
  Serial.println(lux);
  Blynk.virtualWrite(V8, lux);
}




void setup()
{
  Serial.begin(115200);
  Wire.begin(21,22);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  // Mulai komunikasi dengan MH-Z19 (GPIO16 RX, GPIO17 TX)
  pinMode(relayy,OUTPUT);

  mySerial.begin(9600, SERIAL_8N1, 16, 17);
  myMHZ19.begin(mySerial);
  myMHZ19.autoCalibration();
  dht.begin();
  dht1.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(5000L, sendSensor);   // CO2
  timer.setInterval(5000L, suhu1suhu);    // DHT dalam
  timer.setInterval(5000L, suhusuhu);     // DHT luar
  timer.setInterval(300L, simpansuhu1);       
  timer.setInterval(30000L, coolingdds);    // hitung setiap 31 detik
  timer.setInterval(5000L,luxx);
  timer.setInterval(3000L, relay); 
  /*Google_Sheets_Init(namakolom,scriptURL,parameter);*/
}
  

void loop()
{
  float celcius1 = dht1.readTemperature();
  float humidity1 = dht1.readHumidity();
  Blynk.run();
  timer.run();
  /*Data_to_Sheets(parameter, celcius1, humidity1, total);*/
  Serial.println();
  delay(1000);
}

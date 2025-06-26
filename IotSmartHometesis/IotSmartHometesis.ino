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

#define BLYNK_TEMPLATE_ID "TMPLxxxxxxx"
#define BLYNK_TEMPLATE_NAME "Monitoring CO2"
#define BLYNK_AUTH_TOKEN "YourAuthToken"
// Konfigurasi dengan esp dan blynk serta pemanggilan fungsi wifi
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <MHZ19.h>
#include <HardwareSerial.h>
#include "DHT.h"
//define dht untuk suhu
#define PINDHT1 * //PInDalam
#define PINDHT  * //PinLuar
#define tipeDHT DHT22

//define degree days
float datasuhu[100];
int indexsuhu = 0;
bool cekdata = false;
float total = 0.0;
float suhuacuan = 26.0;
int i;
char ssid[] = "YourWiFi";
char pass[] = "YourPassword";
DHT dht1(PINDHT1,tipeDHT);
DHT dht(PINDHT,tipeDHT);

HardwareSerial mySerial(1);
MHZ19 myMHZ19;

BlynkTimer timer;

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
  float rata = ((suhumin+suhumax)/2.0);
  float cdd =0;

  if(rata > suhuacuan){
    cdd = rata -suhuacuan;
    total =+ cdd;
      }
//Kualifikasi pemanggilan blynk dengan cooling degree days
Blynk.virtualWrite(V7, total);
}


//Otomatisasi Cooling degree days



void setup()
{
  Serial.begin(115200);
 
  // Mulai komunikasi dengan MH-Z19 (GPIO16 RX, GPIO17 TX)
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
}
  }

void loop()
{
  Blynk.run();
  timer.run();
}

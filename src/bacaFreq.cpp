#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint16_t RECV_PIN = 15;  // IR receiver disambung ke GPIO 15
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();  // Mulai IR Receiver
  Serial.println("Menekan tombol ac.....");
}

void loop() {
  if (irrecv.decode(&results)) {
    // Tampilkan ringkasan
    Serial.println("=== DATA IR DITERIMA ===");
    Serial.println(resultToHumanReadableBasic(&results));

    // Tampilkan rawData[] versi siap tempel
    Serial.println(resultToSourceCode(&results));

    // Tampilkan raw timing info (opsional)
    Serial.println(resultToTimingInfo(&results));

    // Hanya jika ingin tahu kode HEX-nya juga
    Serial.print("Kode HEX: 0x");
    Serial.println(results.value, HEX);
    
    Serial.println("========================");

    irrecv.resume();  // Siapkan untuk sinyal berikutnya
  }
}

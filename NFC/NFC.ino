 #include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h> // Include the LiquidCrystal library for I2C

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

String formattedDate;
String dayStamp;
String timeStamp;
String uid;

const char* ssid = "FM";
const char* password = "pashaejojo123";
const char* Server = "https://api.siiha.id/api/absensi-siswa";

// Define LCD properties
LiquidCrystal_I2C lcd(0x27, 16, 2); // Address 0x27, 16 columns, 2 rows

void setup() {
  Serial.begin(115200);
  pinMode(15, OUTPUT);
  pinMode(5, OUTPUT);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  Serial.printf("connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  timeClient.begin();
  timeClient.setTimeOffset(25200);
  Serial.println("NDEF Reader");

  nfc.begin();
}

void loop() {
  digitalWrite(5, HIGH);
  digitalWrite(15, LOW);
  
  lcd.clear(); // Clear LCD display
  lcd.setCursor(0, 0);
  lcd.print("NO CARD");

  dayStamp = "Kosong";
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    //tag.print();
    uid = tag.getUidString();
    Serial.println(uid);
    digitalWrite(5, LOW);
    digitalWrite(15, HIGH);

    timeClient.update();
    timeStamp = timeClient.getFormattedTime();
    Serial.println(timeStamp);

    lcd.clear(); // Clear LCD display
    lcd.setCursor(0, 0);
    lcd.print("UID: ");
    lcd.setCursor(5, 0);
    lcd.print(uid);

    lcd.setCursor(0, 1);
    lcd.print("Time: ");
    lcd.setCursor(6, 1);
    lcd.print(timeStamp);

    HTTPClient http;

    DynamicJsonDocument jsonDoc(200); // Adjust the size based on your data
    jsonDoc["uid_kartu"] = uid;
    jsonDoc["tanggal"] = dayStamp;
    jsonDoc["waktu"] = timeStamp;
    
    String payload;
    serializeJson(jsonDoc, payload);

    http.begin(Server);      
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(payload);
  
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response Code : " + String(httpResponseCode));
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println("Respon Code : " + String(httpResponseCode));
    }
    http.end();
  }
}

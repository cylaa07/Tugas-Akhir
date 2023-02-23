#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include <FirebaseESP32.h>    //library firebase
#include <WiFi.h>
//library sensor cahaya
#include <BH1750.h>          
#include <Wire.h>
#include <ArduinoJson.h>
#include <ThingESP.h>

ThingESP32 thing("cylarahmania", "Germinator", "sensor");

//inisialisasi sensor soil moisture
#define SOIL_SENSOR_PIN 32  //ESP32 pin GPIO32 
#define RELAY_PIN 4         //ESP32 pin GPIO4
//inisialisasi sensor LDR
#define PIN_RELAY 5         //ESP32 pin GIOP5 (ADC0)

#define FIREBASE_HOST "https://pengamatan-benih-kedelai-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "eF2IU1mEE8DdqkqpVoMmiXKfLoEKJOgIrnB0uw9q"

#define WIFI_SSID "RG LAB KERING"
#define WIFI_PASSWORD ""

BH1750 lightMeter(0x23);
FirebaseData fbdo; //fbdo adalah variabel

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  Wire.begin();
  lightMeter.begin();
  Serial.println(F("BH1750 Advanced begin"));
  
  pinMode(SOIL_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT); 
  
  thing.SetWiFi("RG LAB KERING", "");    
  thing.initDevice(); 
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);   //memulai untuk menghubungkan dengan WiFi
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

  String HandleResponse(String query)
 {
  float kelembaban_persentase;
  int nilai = analogRead(SOIL_SENSOR_PIN);
  kelembaban_persentase = (100 - ((nilai/4095.00) * 100));

  float lux = lightMeter.readLightLevel();
  
  if (query == "/cek kelembaban") {
    return ("Halo\nKelembaban Tanah = " + String(kelembaban_persentase)+" %");
  }
  else if (query == "/cek cahaya") {
    return ("Halo\nIntensitas Cahaya = " + String(lux)+" lx");
  }
  else if (query == "/cek sensor"){
    return ("Halo\nKelembaban Tanah = " + String(kelembaban_persentase)+" %\nIntensitas Cahaya    = " + String(lux)+" lx");
  } 
  else return "Kalimat Perintah Salah...";
 }

void loop() {
  //sensor soil mositure
  float kelembaban_persentase;
  int nilai = analogRead(SOIL_SENSOR_PIN);
  kelembaban_persentase = (100 - ((nilai/4095.00) * 100));
  Serial.print("Nilai : ");
  Serial.println(nilai);
  Serial.print("Kelembaban : ");
  Serial.print(kelembaban_persentase);
  Serial.print("%\n");
  String fireKelembaban = String(kelembaban_persentase) + String("%");
  {
  if (nilai > 1638){
    Serial.print("Tanah Kering, Pompa Hidup");
    Serial.println(" ");    //ke serial monitor

    lcd.setCursor(0,0);
    lcd.print("Kelembaban:"); //menampilkan data pada lcd
    lcd.print(kelembaban_persentase);
    lcd.print("%");
    digitalWrite(RELAY_PIN, LOW);   //pompa hidup
  } else {
    Serial.print("Tanah Basah, Pompa Mati");
    Serial.println(" "); 

    lcd.setCursor(0,0);
    lcd.print("Kelembaban:"); //menampilkan data pada lcd
    lcd.print(kelembaban_persentase);
    lcd.print("%");
    digitalWrite(RELAY_PIN, HIGH); //pompa mati
  }
   
    if (nilai > 1638){
      Firebase.pushString(fbdo, "/Sensor Soil Moisture/Kelembaban Tanah", fireKelembaban);
      Firebase.pushString(fbdo, "/Sensor Soil Moisture/Kondisi", "Tanah Kering");
   } else {
      Firebase.pushString(fbdo, "/Sensor Soil Moisture/Kelembaban Tanah", fireKelembaban);
      Firebase.pushString(fbdo, "/Sensor Soil Moisture/Kondisi", "Tanah Basah");
  }
  
  //sensor Light Intensity
  float lux = lightMeter.readLightLevel();
  Serial.print("Cahaya: ");
  Serial.print(lux);
  Serial.println(" lx");
 
  String fireCahaya = String(lux) + String(" lux");
  
  if (lux < 750) {
    digitalWrite(PIN_RELAY, LOW);              //lampu nyala
    Serial.println("Ruangan Gelap, Lampu Nyala");

    lcd.setCursor(0,1);
    lcd.print("Cahaya: "); //menampilkan data pada lcd
    lcd.print(lux);
    lcd.print(" lx");
    lcd.print("  ");
  } else {
    digitalWrite(PIN_RELAY, HIGH);               //lampu mati
    Serial.println("Ruangan Terang, Lampu Mati");

    lcd.setCursor(0,1);
    lcd.print("Cahaya: "); //menampilkan data pada lcd
    lcd.print(lux);
    lcd.print(" lx");
    lcd.print("  ");
  }

  if (lux < 750) {
    Firebase.pushString(fbdo, "/Sensor Light Intensity/Cahaya", fireCahaya);
    Firebase.pushString(fbdo, "/Sensor Light Intensity/Kondisi", "Gelap");
  } else {
    Firebase.pushString(fbdo, "/Sensor Light Intensity/Cahaya", fireCahaya);
    Firebase.pushString(fbdo, "/Sensor Light Intensity/Kondisi", "Terang");
  }
  delay(5000);
  thing.Handle();
}
}

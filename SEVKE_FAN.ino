#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>

#include <DNSServer.h>         //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>  //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>       //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>  //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
//#include <WiFiUdp.h>

#include <EEPROM.h>

#define LED_PIN D4
#define NUM_LEDS 31
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// int animationIndex = 3;
// const long interval = 100;  // animasyon hızı için süre ayarı (ms)
// int currentPixel = 0;

const int pwmMin = 50;
const int pwmMax = 255;

const char* ssid = "YOUR_WİFİ";
const char* password = "WIFI_PASSWORD";

// IPAddress staticIP(192, 168, 1, 31);  // Sabit IP adresiniz
// IPAddress gateway(192, 168, 1, 2);    // Genellikle modem IP'niz
// IPAddress subnet(255, 255, 255, 0);   // Alt ağ maskesi
// IPAddress dns(192, 168, 1, 2);        // Genellikle modem IP'niz

ESP8266WebServer server(80);

int smokeLED = 0;
bool fanRUN = true;
bool ledRUN = true;
#define fan D5

int fanSpeed = 0;
int oldfanSpeed = 0;
bool fanAuto = false;
//bool fanupt = true;

Adafruit_BMP085 bmp;

#define MQ2_PIN A0  // MQ-2 sensörünün bağlandığı analog pin

int Mgas = 2000;
unsigned long MAgas = 0;
int gasVal = 0;

int freshair = 10;
int badair = 40;
int freshair_fac = 10;
int badair_fac = 40;

int prc, prc2, prc3, prc4;

const int pwmFreq = 20000;    // PWM frekansı 25 kHz
const int pwmResolution = 8;  // 8 bit çözünürlük (0-255 arası değerler)

String globalMetin = "";  // Global değişken, duruma göre güncellenecek

String fullmessage = "";

void setup() {


  pinMode(fan, OUTPUT);
  analogWriteFreq(pwmFreq);  // PWM frekansını ayarla
  analogWriteRange(255);     // PWM çözünürlüğünü ayarla

  updateFAN();

  Serial.begin(9600);

  strip.begin();
  strip.show();



  // if (!WiFi.config(staticIP, gateway, subnet, dns)) {
  //   Serial.println("STA Failed to configure");
  // }
  rainbowLoadingBar2(0, 5);
  WIFImanager();

  //WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    //Serial.println("Connecting to WiFi...");
    prc = random(6, 30);
    rainbowLoadingBar2(6, prc);
  }
  //Serial.println("Connected to WiFi");
  prc2 = random(prc, 40);
  rainbowLoadingBar2(prc + 1, prc2);

  if (!bmp.begin()) {
    //Serial.println("BMP180 sensör bulunamadı. Lütfen bağlantıları kontrol edin.");
    while (1)
      ;
  }

  prc3 = random(prc2, 60);
  rainbowLoadingBar2(prc2 + 1, prc3);

  // #include "OTA.h"

  EEPROM.begin(512);  // EEPROM'un başlatılması

  loadCalibrationData();  // EEPROM'dan kalibrasyon verilerini yükle
  //Serial.println(badair);
  //Serial.println(freshair);
  //Serial.println(fanSpeed);
  prc4 = random(prc3, 75);
  rainbowLoadingBar2(prc3 + 1, prc4);
  //delay(1000);  // 3 second delay for boot recovery, and a moment of silence
  Serial.println(WiFi.localIP());

  // blinkIP();

  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head><title>FAN Kontrol Paneli</title>"
                  "<script>"
                  "function sendRequest(url) {"
                  "  var xhr = new XMLHttpRequest();"
                  "  xhr.open('GET', url, true);"
                  "  xhr.send();"
                  "}"
                  // ... Diğer JavaScript fonksiyonları ...
                  "function refreshData() {"
                  "  var xhr = new XMLHttpRequest();"
                  "  xhr.onreadystatechange = function() {"
                  "    if (this.readyState == 4 && this.status == 200) {"
                  "      document.getElementById('globalMetin').innerHTML = this.responseText;"
                  "    }"
                  "  };"
                  "  xhr.open('GET', '/getGlobalMetin', true);"
                  "  xhr.send();"
                  "}"
                  "function refreshmsg() {"
                  "  var xhr = new XMLHttpRequest();"
                  "  xhr.onreadystatechange = function() {"
                  "    if (this.readyState == 4 && this.status == 200) {"
                  "      document.getElementById('fullmessage').innerHTML = this.responseText;"
                  "    }"
                  "  };"
                  "  xhr.open('GET', '/fullmessage', true);"
                  "  xhr.send();"
                  "}"
                  "setInterval(refreshData, 300);"  // Her 0.3 saniyede bir refreshData fonksiyonunu çağır
                  "setInterval(refreshmsg, 2000);"
                  "</script>"
                  "<script>"
                  "function sendRequest(url) {"
                  "  var xhr = new XMLHttpRequest();"
                  "  xhr.open('GET', url, true);"
                  "  xhr.send();"
                  "}"
                  "function updateSlider(value) {"
                  "  document.getElementById('sliderValue').innerText = value;"
                  "  sendRequest('/setSlider?value=' + value);"
                  "}"
                  "function setFanAuto() {"
                  "  var checkBox = document.getElementById('autoFan');"
                  "  sendRequest('/setFanAuto?state=' + checkBox.checked);"
                  "}"
                  "function ledstate() {"
                  "  var checkBox = document.getElementById('ledRUN');"
                  "  sendRequest('/ledstate?state=' + checkBox.checked);"
                  "}"
                  "function fanstate() {"
                  "  var checkBox = document.getElementById('fanRUN');"
                  "  sendRequest('/fanstate?state=' + checkBox.checked);"
                  "}"
                  "</script>"
                  "</head><body>"
                  "<h1>FAN Kontrol Paneli</h1>"
                  "<button onclick=\"sendRequest('/button1')\">Temiz hava kalibrasyon</button>"
                  "<button onclick=\"sendRequest('/button2')\">Kirli hava kalibrasyon</button>"
                  "<button onclick=\"sendRequest('/button3')\">Fabrika ayarlari</button>"
                  "<br><br>"
                  "<input type='range' id='slider' min='20' max='100' onchange='updateSlider(this.value)' />"
                  "<p>Fan hizi: <span id='sliderValue'>"
                  + String(round(map(fanSpeed, pwmMin, pwmMax, 0, 100))) + "</span></p>"
                                                    "<br>"
                                                    "<input type='checkbox' id='autoFan' onchange='setFanAuto()' /> <label for='autoFan'>OtoFAN</label>"
                                                    "<input type='checkbox' id='ledRUN' onchange='ledstate()' checked /> <label for='ledRUN'>LED ON/OFF</label>"
                                                    "<input type='checkbox' id='fanRUN' onchange='fanstate()' checked /> <label for='fanRUN'>FAN ON/OFF</label>"
                                                    "<h2>Durum</h2>"
                                                    "<p id='globalMetin'>"
                  + globalMetin + "</p>"  // İlk yüklenmede globalMetin değerini göster
                                  "<p id='fullmessage'>"
                  + fullmessage + "</p>"
                                  "</body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/getGlobalMetin", HTTP_GET, []() {
    server.send(200, "text/plain", globalMetin);
  });

  server.on("/fullmessage", HTTP_GET, []() {
    server.send(200, "text/plain", fullmessage);
  });

  server.on("/button1", HTTP_GET, []() {
    freshair = analogRead(MQ2_PIN);
    saveCalibrationData();
    //Serial.println("Temiz hava değeri: " + String(freshair));
    if (abs(badair - freshair) < 10 || badair < freshair) {
      badair = freshair + 10;
      saveCalibrationData();
      globalMetin = "Temiz hava degeri" + freshair;
    }
    // Buton 1 için işlemler
  });

  server.on("/button2", HTTP_GET, []() {
    badair = analogRead(MQ2_PIN);
    saveCalibrationData();
    //Serial.println("Kirli hava değeri: " + String(badair));
    if (abs(badair - freshair) < 10 || badair < freshair) {
      badair = freshair + 10;
      saveCalibrationData();
      globalMetin = "Kirli hava degeri" + badair;
    }
  });

  server.on("/button3", HTTP_GET, []() {
    freshair = freshair_fac;
    badair = badair_fac;
    //Serial.println("Fabrika ayarlarına dönüldü.");

    globalMetin = "FABRIKA AYARLARINA DONULDU";
  });


  server.on("/setSlider", HTTP_GET, []() {
    if (server.hasArg("value")) {
      int sliderValue = server.arg("value").toInt();
      fanSpeed = map(sliderValue, 0, 100, pwmMin, pwmMax);
     // Serial.println("Slider değeri: " + String(fanSpeed));
      //fanupt = true;
      // Slider değeri ile işlemler
    }
  });

  server.on("/setFanAuto", HTTP_GET, []() {
    if (server.hasArg("state")) {
      fanAuto = server.arg("state") == "true";
      //Serial.println("fanAuto durumu: " + String(fanAuto));
      saveCalibrationData();
      if (fanAuto) {
        globalMetin = "FAN OTO";
      } else {
        globalMetin = "FAN MANUEL";
      }
    }
  });

  server.on("/ledstate", HTTP_GET, []() {
    if (server.hasArg("state")) {
      ledRUN = server.arg("state") == "true";
      //Serial.println("LED durumu: " + String(ledRUN));
      if (ledRUN) {
        globalMetin = "LED ON";
      } else {
        globalMetin = "LED OFF";
      }
    }
  });

  server.on("/fanstate", HTTP_GET, []() {
    if (server.hasArg("state")) {
      fanRUN = server.arg("state") == "true";
      //Serial.println("FAN durumu: " + String(fanRUN));
      saveCalibrationData();
      if (fanRUN) {
        globalMetin = "FAN ON";
      } else {
        globalMetin = "FAN OFF";
      }
    }
  });





    server.begin();
    rainbowLoadingBar2(prc4 + 1, 100);
    delay(500);
    clearLEDs();
    delay(1000);
   blinkIP(WiFi.localIP());
  delay (1000);
   tripleFlash();  // 3'lü flaş efektini çalıştır
}

void loop() {

  // while (.available()) {
  //   server.handleClient();
  // }
   for (int i = 0; i < 10000; i++) {
     server.handleClient();
   }
  //Serial.println("dsaf");
  //ArduinoOTA.handle();

   if (millis() - MAgas > Mgas) {
     gasVal = readGas();
     updateFAN();
     updateLED();
       updateBMP();
     MAgas = millis();
   }
}


void updateLED() {
  if (ledRUN) {
    smokeLevelAnimation(gasVal);
  } else {
    clearLEDs();
  }
}

void updateFAN() {

  if (fanRUN) {
    if (fanAuto) {
      fanSpeed = map(readGas(), freshair, badair, pwmMin, pwmMax);
      analogWrite(fan, fanSpeed);
    } else {
      analogWrite(fan, fanSpeed);
      if (fanSpeed != oldfanSpeed) {
        saveCalibrationData();
        oldfanSpeed = fanSpeed;
      }
    }
  } else {
    analogWrite(fan, 0);
  }
}

void smokeLevelAnimation(int mq2_value) {
  int ledCountForSmoke = map(mq2_value, freshair, badair, 0, NUM_LEDS);
  ledCountForSmoke = round(ledCountForSmoke);
  if (smokeLED != ledCountForSmoke) {
    smokeLED = ledCountForSmoke;
    for (int i = 0; i < NUM_LEDS; i++) {
      if (i < ledCountForSmoke) {
        strip.setPixelColor(i, strip.Color(255, 0, 0));  // Kırmızı renk
      } else {
        strip.setPixelColor(i, strip.Color(0, 255, 0));  // Yeşil renk
      }
    }
    strip.show();
  }
}

int readGas() {
  int gasValue = analogRead(MQ2_PIN);
  gasValue = constrain(gasValue, freshair, badair);
  return gasValue;
}

void saveCalibrationData() {
  EEPROM.put(0, freshair);
  EEPROM.put(sizeof(int), badair);
  EEPROM.put(sizeof(int) * 2, fanSpeed);  // fanSpeed'i EEPROM'a kaydet
  EEPROM.put(sizeof(int) * 2 + sizeof(int), fanAuto);  // fanAuto'yu EEPROM'a kaydet
  EEPROM.put(sizeof(int) * 2 + sizeof(int) + sizeof(bool), fanRUN);  // fanRUN'u EEPROM'a kaydet
  EEPROM.commit();
}

void loadCalibrationData() {
  EEPROM.get(0, freshair);
  EEPROM.get(sizeof(int), badair);
  EEPROM.get(sizeof(int) * 2, fanSpeed);  // fanSpeed'i EEPROM'dan yükle
  EEPROM.get(sizeof(int) * 2 + sizeof(int), fanAuto);  // fanAuto'yu EEPROM'dan yükle
  EEPROM.get(sizeof(int) * 2 + sizeof(int) + sizeof(bool), fanRUN);  // fanRUN'u EEPROM'dan yükle
}
void WIFImanager() {

  WiFiManager wifiManager;

  wifiManager.setTimeout(120);

  if (!wifiManager.autoConnect("SEVKE_FAN")) {

     delay(2000);

     ESP.restart();
     delay(500);
  }
}

void blinkIP(IPAddress ip) {
  String ipString = ip.toString();
  int secondLastIndex = ipString.lastIndexOf('.', ipString.lastIndexOf('.') - 1);
  String lastTwoParts = ipString.substring(secondLastIndex + 1);  // Son iki bölümü al

  for (int i = 0; i < lastTwoParts.length(); i++) {
    char c = lastTwoParts.charAt(i);

    if (c == '.') {
      blinkColor(strip.Color(255, 0, 0), 1);  // Nokta için kırmızı
    } else {
      int number = c - '0';                        // Char'ı integer'a dönüştür
      blinkColor(strip.Color(0, 0, 255), number);  // Rakam için mavi

      // Sonraki karakteri kontrol et
      if (i < lastTwoParts.length() - 1) {
        if (lastTwoParts.charAt(i + 1) != '.') {
          blinkColor(strip.Color(255, 255, 0), 1);  // Rakamlar arası için sarı
        }
      }
    }
  }
}



void blinkColor(uint32_t color, int times) {
  for (int i = 0; i < times; i++) {
    strip.fill(color);
    strip.show();
    delay(1000);  // 1 saniye bekle
    strip.fill(strip.Color(0, 0, 0));
    clearLEDs();
    strip.show();
    delay(1000);  // 1 saniye bekle
  }
}

void rainbowLoadingBar(int percentage) {
  int ledCount = (NUM_LEDS * percentage) / 100;  // Yüzdeye göre LED sayısını hesapla

  // LED'leri belirtilen yüzdeye kadar rainbow renklerde doldur
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < ledCount) {
      int pixelHue = i * (65536L / NUM_LEDS);
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    } else {
      strip.setPixelColor(i, strip.Color(0, 0, 0));  // Geri kalan LED'leri kapat
      clearLEDs();
    }
  }
  strip.show();
}

void rainbowLoadingBar2(int startLed, int endLed) {
  if (startLed < 0) startLed = 0;
  if (endLed >= NUM_LEDS) endLed = NUM_LEDS - 1;

  for (int i = startLed; i <= endLed; i++) {
    // LED'leri sırayla rainbow renklerde yak
    for (int j = startLed; j <= i; j++) {
      int pixelHue = j * (65536L / NUM_LEDS);
      strip.setPixelColor(j, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(100);  // Animasyon hızını kontrol et
  }

  // Opsiyonel: Animasyon sonunda, belirtilen aralığı tekrar kapatmak isterseniz bu döngüyü kullanın
  // for (int i = startLed; i <= endLed; i++) {
  //   strip.setPixelColor(i, strip.Color(0, 0, 0));
  // }
  // strip.show();
}


void tripleFlash() {
  for (int i = 0; i < 3; i++) {  // 3 kere yanıp sönme
    // Tüm LED'leri yeşil yap
    for (int j = 0; j < NUM_LEDS; j++) {
      strip.setPixelColor(j, strip.Color(0, 255, 0));  // Yeşil
    }
    strip.show();
    delay(100);  // Yanık durumda ne kadar süre kalacağını kontrol et

    // Tüm LED'leri kapat
    for (int j = 0; j < NUM_LEDS; j++) {
      strip.setPixelColor(j, strip.Color(0, 0, 0));  // Kapalı
    }
    strip.show();
    delay(100);  // Sönük durumda ne kadar süre kalacağını kontrol et
  }
  clearLEDs();
}

void clearLEDs() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));  // Her LED'i siyah yaparak kapat
  }
  strip.show();  // Değişiklikleri LED şeridine uygula
}

void updateBMP() {
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure();
  float altitude = bmp.readAltitude(1013.25);
  String message = "SICAKLIK:" + String(temperature);
  String message2 = message + " C     BASINC:";
  String message3 = message2 + String(pressure);
  fullmessage = message3 + " Pa";
  fullmessage += "  K:" +String(badair);
  fullmessage += "  T:" +String(freshair);
  fullmessage += "  M:" +String(gasVal);
  
  //Serial.println(fullmessage);
}

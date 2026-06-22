#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- إعدادات شبكة الـ Wi-Fi ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// --- إعدادات التليجرام ---
const char* botToken = "YOUR_TELEGRAM_BOT_TOKEN";
const char* chatId = "YOUR_TELEGRAM_CHAT_ID"; 

// --- إعدادات Google Sheets ---
const char* googleScriptDeploymentId = "YOUR_GOOGLE_SCRIPT_ID"; 
const char* host = "script.google.com";

// --- إعدادات وتوصيل الحساسات (SHT I2C Setup) ---
// الحساس الأول والثاني على الـ Hardware I2C الأساسي (SDA=21, SCL=22)
// الحساس الأول: دبوس ADDR واصل بالـ GND (العنوان 0x44)
// الحساس الثاني: دبوس ADDR واصل بالـ VCC 3.3V (العنوان 0x45)
Adafruit_SHT31 sht1 = Adafruit_SHT31();
Adafruit_SHT31 sht2 = Adafruit_SHT31();

// الحساس الثالث: هنشغله على دبابيس I2C منفصلة ومحاكاة برمجية (Software I2C)
// سنستخدم دبابيس GPIO 13 للـ SDA و GPIO 14 للـ SCL
TwoWire I2C_Two = TwoWire(1); 
Adafruit_SHT31 sht3 = Adafruit_SHT31(&I2C_Two);

// --- إعدادات إدارة الطاقة (Deep Sleep) ---
#define TIME_TO_SLEEP  21600  
#define uS_TO_S_FACTOR 1000000ULL  

const float HUMIDITY_THRESHOLD = 70.0; 

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  // تشغيل الـ Hardware I2C للحساس الأول والثاني
  Wire.begin(21, 22); 
  
  // تشغيل الـ Software I2C للحساس الثالث (SDA=13, SCL=14)
  I2C_Two.begin(13, 14, 100000); 

  // تفعيل الحساسات والتأكد من اتصالها
  if (!sht1.begin(0x44)) { Serial.println("Could not find SHT31 sensor 1 (0x44)"); }
  if (!sht2.begin(0x45)) { Serial.println("Could not find SHT31 sensor 2 (0x45)"); }
  if (!sht3.begin(0x44)) { Serial.println("Could not find SHT31 sensor 3 on Wire1"); }

  client.setInsecure(); 
  setup_wifi();

  // قراءة المعطيات ومعالجتها
  processSensorData();

  Serial.println("Going to deep sleep now...");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  // تظل فارغة بسبب الـ Deep Sleep
}

void setup_wifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

void processSensorData() {
  // قراءة البيانات من الحساسات
  float t1 = sht1.readTemperature();
  float h1 = sht1.readHumidity();
  
  float t2 = sht2.readTemperature();
  float h2 = sht2.readHumidity();
  
  float t3 = sht3.readTemperature();
  float h3 = sht3.readHumidity();

  int validCount = 0;
  float sumHum = 0;
  float sumTemp = 0;
  bool isEmergency = false;
  String alertMessage = "";

  // فحص الحساس الأول
  if (!isnan(h1) && !isnan(t1)) {
    sumHum += h1; sumTemp += t1; validCount++;
    if (h1 >= HUMIDITY_THRESHOLD) { isEmergency = true; alertMessage += "⚠️ الحساس السفلي (1): رطوبة حرجة " + String(h1, 1) + "%\n"; }
  }
  // فحص الحساس الثاني
  if (!isnan(h2) && !isnan(t2)) {
    sumHum += h2; sumTemp += t2; validCount++;
    if (h2 >= HUMIDITY_THRESHOLD) { isEmergency = true; alertMessage += "⚠️ الحساس المنتصف (2): رطوبة حرجة " + String(h2, 1) + "%\n"; }
  }
  // فحص الحساس الثالث
  if (!isnan(h3) && !isnan(t3)) {
    sumHum += h3; sumTemp += t3; validCount++;
    if (h3 >= HUMIDITY_THRESHOLD) { isEmergency = true; alertMessage += "⚠️ الحساس العلوي (3): رطوبة حرجة " + String(h3, 1) + "%\n"; }
  }

  float avgHum = (validCount > 0) ? (sumHum / validCount) : 0;
  float avgTemp = (validCount > 0) ? (sumTemp / validCount) : 0;

  if (validCount == 0) {
    sendTelegramMessage("❌ خطأ حرج: جميع حساسات SHT تعطي قراءات خاطئة أو غير متصلة!");
    return;
  }

  // بناء التقرير الدوري للتليجرام
  String report = "🌾 **تقرير مراقبة مخزن الغلة الدوري (SHT Sensors)** 🌾\n\n";
  report += "📊 **المتوسط العام:**\n";
  report += "🔹 رطوبة المخزن: " + String(avgHum, 1) + "%\n";
  report += "🔹 حرارة المخزن: " + String(avgTemp, 1) + "°C\n\n";
  report += "🔍 **تفاصيل الحساسات:**\n";
  report += "1️⃣ حساس السفلي: " + (isnan(h1) ? "عطلان" : String(h1,1)+"% | "+String(t1,1)+"°C") + "\n";
  report += "2️⃣ حساس المنتصف: " + (isnan(h2) ? "عطلان" : String(h2,1)+"% | "+String(t2,1)+"°C") + "\n";
  report += "3️⃣ حساس العلوى: " + (isnan(h3) ? "عطلان" : String(h3,1)+"% | "+String(t3,1)+"°C") + "\n\n";

  if (isEmergency) {
    report = "🚨 **إشعار خطر عاجل - مخزن الغلال** 🚨\n\n" + alertMessage + "\n" + report;
    report += "🛑 **التوصية:** يرجى تشغيل شفاطات التهوية فوراً لمنع تعفن المحصول وتكون الفطريات.";
  } else {
    report += "✅ **الحالة:** مستقرة وضمن الحدود الآمنة.";
  }

  if (WiFi.status() == WL_CONNECTED) {
    sendTelegramMessage(report);
    sendToGoogleSheets(t1, h1, t2, h2, t3, h3, avgTemp, avgHum);
  }
}

void sendTelegramMessage(String message) {
  Serial.println("Sending to Telegram...");
  if (bot.sendMessage(chatId, message, "Markdown")) {
    Serial.println("Telegram message sent successfully.");
  } else {
    Serial.println("Failed to send Telegram message.");
  }
}

void sendToGoogleSheets(float t1, float h1, float t2, float h2, float t3, float h3, float avgT, float avgH) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure sheetClient;
    sheetClient.setInsecure();
    
    String url = "/macros/s/" + String(googleScriptDeploymentId) + "/exec?";
    url += "t1=" + String(t1) + "&h1=" + String(h1);
    url += "&t2=" + String(t2) + "&h2=" + String(h2);
    url += "&t3=" + String(t3) + "&h3=" + String(h3);
    url += "&avgT=" + String(avgT) + "&avgH=" + String(avgH);

    Serial.print("Sending data to Google Sheets... ");
    if (sheetClient.connect(host, 443)) {
      sheetClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
                         "Host: " + host + "\r\n" +
                         "User-Agent: ESP32\r\n" +
                         "Connection: close\r\n\r\n");
      Serial.println("Data Sent.");
    } else {
      Serial.println("Connection to Google Sheets failed.");
    }
    sheetClient.stop();
  }
}

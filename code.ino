#include <Wire.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// -------------------- WiFi + Firebase Credentials --------------------
#define WIFI_SSID "shri"
#define WIFI_PASSWORD "12345687"

#define API_KEY "AIzaSyB3eg0aPR5A-_6kzJUgESutBJMhFrzOIkQ"
#define DATABASE_URL "https://battery-455f1-default-rtdb.asia-southeast1.firebasedatabase.app/"

// -------------------- Firebase Objects --------------------
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// -------------------- INA219 Sensor --------------------
Adafruit_INA219 ina219;

// -------------------- DS18B20 Temperature Sensor --------------------
#define ONE_WIRE_BUS 4     // DATA pin connected to GPIO 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("ESP32 + INA219 + DS18B20 + Firebase Logger");

  // -------------------- WiFi Connection --------------------
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // -------------------- Firebase Setup --------------------
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Anonymous Sign-in
  auth.user.email = "";
  auth.user.password = "";

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // -------------------- INA219 Init --------------------
  if (!ina219.begin()) {
    Serial.println("❌ INA219 Not Found!");
    while (1);
  }
  Serial.println("✔ INA219 Connected");
  ina219.setCalibration_32V_2A();

  // -------------------- DS18B20 Init --------------------
  sensors.begin();
  Serial.println("✔ DS18B20 Temperature Sensor Ready");
}

void loop() {
  // Read INA219
  float busVoltage = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();
  float current_A = current_mA / 1000.0;

  // Read DS18B20 Temperature
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  Serial.print("Voltage: ");
  Serial.print(busVoltage);
  Serial.print(" V | Current: ");
  Serial.print(current_A);
  Serial.print(" A | Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" °C");

  // -------------------- Send to Firebase --------------------
  if (Firebase.ready()) {
    Firebase.RTDB.setFloat(&fbdo, "/Battery/Voltage", busVoltage);
    Firebase.RTDB.setFloat(&fbdo, "/Battery/Current", current_A);
    Firebase.RTDB.setFloat(&fbdo, "/Battery/Temperature", temperatureC);

    Serial.println("✔ Data Uploaded to Firebase");
  }

  delay(1000);
}

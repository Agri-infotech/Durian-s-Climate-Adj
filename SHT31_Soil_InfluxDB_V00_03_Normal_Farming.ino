#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DFRobot_SHT3x.h>
DFRobot_SHT3x sht3x(&Wire,/*address=*/0x44,/*RST=*/4);
//DFRobot_SHT3x sht3x;
//Adafruit_SHT31 sht31 = Adafruit_SHT31();
//#include <Adafruit_BMP280.h>
//#include <AHT10.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"
#elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #define DEVICE "ESP8266"
#endif
///
  // WiFi AP SSID
  #define WIFI_SSID "P9369a73_2.4GHz"
  // WiFi password
  #define WIFI_PASSWORD "Btr89342"
  #define INFLUXDB_URL "http://43.229.135.63:8086"
  #define INFLUXDB_TOKEN "4cNpPwlwFiTh2CVrKWAdv2OOYb_q7eb5yydQNVwTeNOzG4f6yKnOuJGE6v72iPYnAP0ia06E8DD018mllUaksA=="
  #define INFLUXDB_ORG "1978563935982fe0"
  #define INFLUXDB_BUCKET "Farming Environments"
  
// Time zone info
  #define TZ_INFO "UTC7"
// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
#define Station_ID "ID-0001: Normal_Farming"
// Data points
Point sensor("measurements");
// Soil moisture sensor
int _moisture,sensor_analog;
const int sensor_pin = A0;  /* Soil moisture sensor O/P pin */
//
void setup() {
  Serial.begin(115200);
  while (sht3x.begin() != 0) {
Serial.println("Failed to Initialize the chip, please confirm the wire connection");
delay(1000);
}
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    //Initialize the chip
  }
  Serial.print("Chip serial number");
  Serial.println(sht3x.readSerialNumber());
  Serial.println();

   //Init BME280 sensor
 // initBMP();
  // Add tags
  //sensor.addTag("device", DEVICE);
 // sensor.addTag("SSID", WiFi.SSID());
  sensor.addTag("Station Number", Station_ID);
  
timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  // Get latest sensor readings
  //temperature = bmp.readTemperature();
  //humidity = bme.readHumidity();
  //pressure = bmp.readPressure()/100.0F;
  // Soil Moisture Read
   sensor_analog = analogRead(sensor_pin);
  _moisture = ( 100 - ( (sensor_analog/4095.00) * 100 ) ); 
  //moisturePercentage = ( 100.00 - ( (analogRead(moisturePin) / 1023.00) * 100.00 ) ); 
  // Cal Vapor Pressure Deficit ศักย์การคายน้ำ
    float temp = sht3x.getTemperatureC(); // Reading the temperature as Celsius
    float hum = sht3x.getHumidityRH();     // Reading the relative humidity percent
    float VPsat = 610.7 * pow(10, (7.5 * temp / (237.3 + temp)));
    float VPD = ((100.0 - hum) /100.0) * VPsat*0.001;  // Vapor Pressure Deficit in Pascals
  sensor.addField("Temperature(Normal)", temp);
 // sensor.addField("Temperature", sht3x.getTemperatureC());
  sensor.addField("Humidity(Normal)", hum);
  sensor.addField("Vapor Pressure Deficit(Normal)", VPD);
 // sensor.addField("Humidity", sht3x.getHumidityRH());
  sensor.addField("SoilMoisture(Normal)", _moisture);

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(sensor));
  
  // Write point into buffer
  client.writePoint(sensor);

  // Clear fields for next usage. Tags remain the same.
  sensor.clearFields();

  // If no Wifi signal, try to reconnect it
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }
  Serial.print("Chip serial number");
  Serial.println(sht3x.readSerialNumber());
  Serial.println();
  Serial.print("อุณหภูมิภายในทรางพุ่ม ");
  Serial.print (temp);
  Serial.print (" ° C");
  Serial.println();
  Serial.print("ความชื้นสัมพัทธ์ภายในทรางพุ่ม ");
  Serial.print(hum);
  Serial.print(" %RH");
  Serial.println();
  Serial.print("แรงดึงระเหยน้ำของอากาศ ");
  Serial.print(VPD);
  Serial.print(" kPa");
  Serial.println();
  Serial.print("ความชื้นในดินรอบทรงพุ่ม ");
  Serial.print( _moisture);
  Serial.print(" %");
  Serial.println();
  Serial.println("");
  Serial.println("Delay 10s");
  delay(10000);
  
}
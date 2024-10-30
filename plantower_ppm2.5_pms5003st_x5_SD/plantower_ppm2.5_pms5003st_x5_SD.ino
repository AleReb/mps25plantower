/*
 * @file  multi_sensor_air_monitor.ino
 * @brief  Monitors air quality parameters (Temperature, Humidity, PM1.0, PM2.5, PM10, Formaldehyde) from multiple sensors.
 * @n This program is designed to read data from multiple Plantower sensors using an ESP32, saving the data to an SD card in CSV format.
 * @n It was modified by Alejandro Rebolledo to enable data collection for up to 5 sensors, each connected via SoftwareSerial.
 * @n The code can potentially be expanded to read up to 10 sensors using only the read pins and shared connections.
 * @n The data from all sensors is appended to the file /data.csv, creating new lines for each set of readings.
 */


#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

#define SD_MISO 23
#define SD_MOSI 13
#define SD_SCLK 4
#define SD_CS_PIN 5

SPIClass spiSD(VSPI);

char col;
unsigned int PMSa1 = 0, PMSa2_5 = 0, PMSa10 = 0, FMHDSa = 0, TPSa = 0, HDSa = 0, PMSb1 = 0, PMSb2_5 = 0, PMSb10 = 0, FMHDSb = 0, TPSb = 0, HDSb = 0;
unsigned int PMS1 = 0, PMS2_5 = 0, PMS10 = 0, TPS = 0, HDS = 0, CR1 = 0, CR2 = 0;
unsigned char bufferRTT[32] = {};  // serial receive data
char tempStr[15];
float FMHDSB = 0;

SoftwareSerial sensor1Serial(18, 19);  // RX, TX for sensor 1
SoftwareSerial sensor2Serial(16, 17);  // RX, TX for sensor 2
SoftwareSerial sensor3Serial(32, 33);  // RX, TX for sensor 3
SoftwareSerial sensor4Serial(25, 26);  // RX, TX for sensor 4
SoftwareSerial sensor5Serial(27, 14);  // RX, TX for sensor 5

void setup() {
  Serial.begin(115200);
  sensor1Serial.begin(9600);
  sensor2Serial.begin(9600);
  sensor3Serial.begin(9600);
  sensor4Serial.begin(9600);
  sensor5Serial.begin(9600);

  // Initialize SD card
  spiSD.begin(SD_SCLK, SD_MISO, SD_MOSI);
  if (!SD.begin(SD_CS_PIN, spiSD)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("Card initialized.");

  // Write CSV header to the file
  File dataFile = SD.open("/data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Sensor 1: Temp: C, RH: %, PM1.0: ug/m3, PM2.5: ug/m3, PM10: ug/m3, Formaldehyde: mg/m3, "
                     "Sensor 2: Temp: C, RH: %, PM1.0: ug/m3, PM2.5: ug/m3, PM10: ug/m3, Formaldehyde: mg/m3, "
                     "Sensor 3: Temp: C, RH: %, PM1.0: ug/m3, PM2.5: ug/m3, PM10: ug/m3, Formaldehyde: mg/m3, "
                     "Sensor 4: Temp: C, RH: %, PM1.0: ug/m3, PM2.5: ug/m3, PM10: ug/m3, Formaldehyde: mg/m3, "
                     "Sensor 5: Temp: C, RH: %, PM1.0: ug/m3, PM2.5: ug/m3, PM10: ug/m3, Formaldehyde: mg/m3");
    dataFile.close();
  } else {
    Serial.println("Error opening data.csv");
  }
}

void loop() {
  Serial.println("----------------------- All Sensors Data --------------------------");

  String csvData = "";
  csvData += readSensor(sensor1Serial, "Sensor 1");
  csvData += readSensor(sensor2Serial, "Sensor 2");
  csvData += readSensor(sensor3Serial, "Sensor 3");
  csvData += readSensor(sensor4Serial, "Sensor 4");
  csvData += readSensor(sensor5Serial, "Sensor 5");

  Serial.println("-------------------------------------------------------------------\n");

  // Write data to SD card
  File dataFile = SD.open("/data.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.println(csvData);  // Append data for each new line
    Serial.println(csvData);
    dataFile.close();
  } else {
    Serial.println("Error opening data.csv");
  }

  delay(1000); // Delay between readings
}

String readSensor(SoftwareSerial &sensorSerial, const char *sensorName) {
  String dataString = "";

  if (!sensorSerial.available()) return dataString;

  while (sensorSerial.available() > 0) {  // Check whether there is any serial data
    for (int i = 0; i < 32; i++) {
      col = sensorSerial.read();
      bufferRTT[i] = (char)col;
      delay(2);
    }

    sensorSerial.flush();

    CR1 = (bufferRTT[30] << 8) + bufferRTT[31];
    CR2 = 0;
    for (int i = 0; i < 30; i++)
      CR2 += bufferRTT[i];

    if (CR1 == CR2) {  // Check
      PMSa1 = bufferRTT[10];        // Read PM1 high 8-bit data
      PMSb1 = bufferRTT[11];        // Read PM1 low 8-bit data
      PMS1 = (PMSa1 << 8) + PMSb1;  // PM1 data

      PMSa2_5 = bufferRTT[12];            // Read PM2.5 high 8-bit data
      PMSb2_5 = bufferRTT[13];            // Read PM2.5 low 8-bit data
      PMS2_5 = (PMSa2_5 << 8) + PMSb2_5;  // PM2.5 data

      PMSa10 = bufferRTT[14];          // Read PM10 high 8-bit data
      PMSb10 = bufferRTT[15];          // Read PM10 low 8-bit data
      PMS10 = (PMSa10 << 8) + PMSb10;  // PM10 data

      TPSa = bufferRTT[24];      // Read temperature high 8-bit data
      TPSb = bufferRTT[25];      // Read temperature low 8-bit data
      TPS = (TPSa << 8) + TPSb;  // Temperature data

      HDSa = bufferRTT[26];      // Read humidity high 8-bit data
      HDSb = bufferRTT[27];      // Read humidity low 8-bit data
      HDS = (HDSa << 8) + HDSb;  // Humidity data

      unsigned int FMHDSa = bufferRTT[22];  // Read formaldehyde high 8-bit data
      unsigned int FMHDSb = bufferRTT[23];  // Read formaldehyde low 8-bit data
      FMHDSB = (FMHDSa << 8) + FMHDSb;       // Combine high and low bits
      FMHDSB = FMHDSB / 1000.0;
    } else {
      PMS1 = 0;
      PMS2_5 = 0;
      PMS10 = 0;
      TPS = 0;
      HDS = 0;
    }
  }

  Serial.print(sensorName);
  Serial.print(": Temp: ");
  sprintf(tempStr, "%d%d.%d", TPS / 100, (TPS / 10) % 10, TPS % 10);
  Serial.print(tempStr);
  Serial.print(" C, RH: ");
  sprintf(tempStr, "%d%d.%d", HDS / 100, (HDS / 10) % 10, HDS % 10);
  Serial.print(tempStr);
  Serial.print(" %, PM1.0: ");
  Serial.print(PMS1);
  Serial.print(" ug/m3, PM2.5: ");
  Serial.print(PMS2_5);
  Serial.print(" ug/m3, PM10: ");
  Serial.print(PMS10);
  Serial.print(" ug/m3, Formaldehyde: ");
  Serial.print(FMHDSB);
  Serial.println(" mg/m3");

  dataString += String(sensorName) + ": Temp: " + String(TPS / 100) + String((TPS / 10) % 10) + "." + String(TPS % 10) + " C, RH: " +
                String(HDS / 100) + String((HDS / 10) % 10) + "." + String(HDS % 10) + " %, PM1.0: " + String(PMS1) +
                " ug/m3, PM2.5: " + String(PMS2_5) + " ug/m3, PM10: " + String(PMS10) + " ug/m3, Formaldehyde: " + String(FMHDSB) + " mg/m3";

  return dataString;
}

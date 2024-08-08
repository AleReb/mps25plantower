/*!
 * @file  SEN0233.ino
 * @brief Air Quality Monitor (PM 2.5, HCHO, Temperature & Humidity)
 * @n Get the module here: https://www.dfrobot.com/product-1612.html
 * @n This example is to detect formaldehyde, PM2.5, temperature and humidity in the environment.
 * @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license  The MIT License (MIT)
 * @author  [lijun](ju.li@dfrobot.com)
 * @version  V1.0
 * @date  2017-03-01
 Modificado por alejandro rebolledo para ver el formalheidro, que segun el datashet esta en un dato antes de la temperatura
 */

#include <SoftwareSerial.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_NeoPixel.h>
#include "SD.h"
#include "SPI.h"
#include "FS.h"

#define SD_MISO 2
#define SD_MOSI 15
#define SD_SCLK 14
#define SD_CS 13
// Configuración de la pantalla OLED
U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Configuración del RTC
RTC_DS3231 rtc;

// Configuración del SoftwareSerial
SoftwareSerial pms5(18, 19);  // RX, TX

char col;
unsigned int PMSa1 = 0, PMSa2_5 = 0, PMSa10 = 0, FMHDSa = 0, TPSa = 0, HDSa = 0, PMSb1 = 0, PMSb2_5 = 0, PMSb10 = 0, FMHDSb = 0, TPSb = 0, HDSb = 0;
unsigned int PMS1 = 0, PMS2_5 = 0, PMS10 = 0, TPS = 0, HDS = 0, CR1 = 0, CR2 = 0;
unsigned char bufferRTT[32] = {};  //serial receive data
char tempStr[15];
float FMHDSB = 0;
// Configuración del NeoPixel
#define PIN 12  // 23 y 12 pin led interno
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Configuración del botón
#define BUTTON_PIN 5

// Configuración de la tarjeta SD
#define SD_CS_PIN 13
SPIClass spiSD(HSPI);  // HSPI

float tempe;
float humi;
String fecha;
String hora;
bool SDok = true;
int ultimoRun = 0;
void setup() {
  Serial.begin(115200);
  pms5.begin(9600);
  u8g2.begin();
  pixels.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(0, 12);  // y = 96
  u8g2.print("TEST GSM PLANTOWER");
  u8g2.sendBuffer();
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!rtc.begin()) {
    Serial.println("No se encontró el RTC");
  }

  spiSD.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS_PIN);

  if (!SD.begin(SD_CS_PIN, spiSD)) {
    Serial.println("Fallo al inicializar la tarjeta SD");
    SDok = false;
  } else {
    Serial.println("Tarjeta SD inicializada correctamente");
    if (SD.exists("/data.csv") == false) {
      Serial.println("Archivo base no existe...");
      delay(500);
      Serial.println("creando archivos");
      String tosave = "FECHA,HORA,TEMPERATURA,HUMEDAD,PM1,PM2_5,PM10,FORMAL,CONEXION \n";
      writeFile(SD, "/data.csv", tosave.c_str());
    } else {
      Serial.println("dato base es: /data.csv");
    }
  }
  pixels.setPixelColor(0, pixels.Color(100, 255, 0));  // Amarillo
  pixels.show();
  u8g2.clearBuffer();
}

unsigned long ultimoGuardado = 0;              // Variable para llevar un registro del último guardado
const unsigned long intervaloGuardado = 3000;  // Intervalo de guardado (30 segundos en milisegundos)
unsigned long checkpms = 0;                    // Variable para llevar un registro del último guardado
const unsigned long intercheckpms = 3000;      // Intervalo de guardado (30 segundos en milisegundos)

void loop() {

  // while (!pms5.available()) ;  //ojo esto suele bloquerar el resto del codigo, pero puede servir para saver si decta o no el serial
  if (millis() - checkpms >= intercheckpms) {
    // Actualizar el registro del último guardado
    checkpms = millis();
    while (pms5.available() > 0)  //Check whether there is any serial data
    {
      for (int i = 0; i < 32; i++) {
        col = pms5.read();
        bufferRTT[i] = (char)col;
        delay(2);
      }

      pms5.flush();

      CR1 = (bufferRTT[30] << 8) + bufferRTT[31];
      CR2 = 0;
      for (int i = 0; i < 30; i++)
        CR2 += bufferRTT[i];
      if (CR1 == CR2)  //Check
      {
        PMSa1 = bufferRTT[10];        //Read PM1 high 8-bit data
        PMSb1 = bufferRTT[11];        //Read PM1 low 8-bit data
        PMS1 = (PMSa1 << 8) + PMSb1;  //PM1 data

        PMSa2_5 = bufferRTT[12];            //Read PM2.5 high 8-bit data
        PMSb2_5 = bufferRTT[13];            //Read PM2.5 low 8-bit data
        PMS2_5 = (PMSa2_5 << 8) + PMSb2_5;  //PM2.5 data

        PMSa10 = bufferRTT[14];          //Read PM10 high 8-bit data
        PMSb10 = bufferRTT[15];          //Read PM10 low 8-bit data
        PMS10 = (PMSa10 << 8) + PMSb10;  //PM10 data

        TPSa = bufferRTT[24];      //Read temperature high 8-bit data
        TPSb = bufferRTT[25];      //Read temperature low 8-bit data
        TPS = (TPSa << 8) + TPSb;  //Temperature data

        HDSa = bufferRTT[26];      //Read humidity high 8-bit data
        HDSb = bufferRTT[27];      //Read humidity low 8-bit data
        HDS = (HDSa << 8) + HDSb;  //Humidity data

        unsigned int FMHDSa = bufferRTT[22];  // Read formaldehyde high 8-bit data
        unsigned int FMHDSb = bufferRTT[23];  // Read formaldehyde low 8-bit data
        FMHDSB = (FMHDSa << 8) + FMHDSb;      // Combine high and low bits
        FMHDSB = FMHDSB / 1000.0;

      } else {
        PMS1 = 0;
        PMS2_5 = 0;
        PMS10 = 0;
        TPS = 0;
        HDS = 0;
      }
    }
    pixels.setPixelColor(0, pixels.Color(0, 255, 255));  // Calipso
    pixels.show();
  }
  Serial.println("-----------------------pms--------------------------");
  Serial.print("Temp : ");
  sprintf(tempStr, "%d%d.%d", TPS / 100, (TPS / 10) % 10, TPS % 10);
  Serial.print(tempStr);
  Serial.println(" C");  //Display temperature
  Serial.print("RH   : ");
  sprintf(tempStr, "%d%d.%d", HDS / 100, (HDS / 10) % 10, HDS % 10);
  Serial.print(tempStr);  //Display humidity
  Serial.println(" %");   //%
  Serial.print("PM1.0: ");
  Serial.print(PMS1);
  Serial.println(" ug/m3");  //Display PM1.0 unit  ug/m³
  Serial.print("PM2.5: ");
  Serial.print(PMS2_5);
  Serial.println(" ug/m3");  //Display PM2.5 unit  ug/m³
  Serial.print("PM 10: ");
  Serial.print(PMS10);
  Serial.println(" ug/m3");  //Display PM 10 unit  ug/m³
  Serial.print("Formaldehyde: ");
  Serial.print(FMHDSB);
  Serial.println(" mg/m3");  // Display formaldehyde concentration in mg/m³
  Serial.print(SDok);
  Serial.print(" SD");
  Serial.println(ultimoRun);  // Display formaldehyde concentration in mg/m³


  // Leer el botón
  if (digitalRead(BUTTON_PIN) == LOW) {
    u8g2.setCursor(80, 108);  // y = 96
    u8g2.print("BN: ");
    u8g2.println("OK");  // Enviar búfer a la pantalla
    u8g2.sendBuffer();   // Acción del botón
  }
  // Leer y mostrar la fecha y hora del RTC
  DateTime now = rtc.now();
  fecha = getFecha(now);
  hora = getHora(now);
  tempe = TPS / 10.0;
  humi = HDS / 10.0;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  // Mostrar los valores en la pantalla, cada uno separado por 12 píxeles
  u8g2.setCursor(0, 12);  // Comenzar en y = 12
  u8g2.print("Temp: ");
  u8g2.print(tempe);
  u8g2.println(" C");

  u8g2.setCursor(0, 24);  // y = 24
  u8g2.print("RH: ");
  u8g2.print(humi);
  u8g2.println(" %");

  u8g2.setCursor(0, 36);  // y = 36
  u8g2.print("PM1.0: ");
  u8g2.print(PMS1);
  u8g2.println(" ug/m3");

  u8g2.setCursor(0, 48);  // y = 48
  u8g2.print("PM2.5: ");
  u8g2.print(PMS2_5);
  u8g2.println(" ug/m3");

  u8g2.setCursor(0, 60);  // y = 60
  u8g2.print("PM10: ");
  u8g2.print(PMS10);
  u8g2.println(" ug/m3");

  u8g2.setCursor(0, 72);  // y = 72
  u8g2.print("HCHO: ");
  u8g2.print(FMHDSB);
  u8g2.println(" mg/m3");

  u8g2.setCursor(0, 84);  // y = 84
  u8g2.print("Fecha: ");
  u8g2.println(fecha);
  u8g2.setCursor(0, 96);  // y = 96
  u8g2.print("Hora: ");
  u8g2.println(hora);
  u8g2.setCursor(0, 108);  // y = 96
  u8g2.print("Temprtc: ");
  u8g2.println(rtc.getTemperature());  // Enviar búfer a la pantalla
  u8g2.setCursor(0, 120);              // y = 96
  u8g2.print("SD: ");
  u8g2.println(SDok);       // Enviar búfer a la pantalla
  u8g2.setCursor(30, 120);  // y = 96
  u8g2.print("R: ");
  u8g2.println(ultimoRun);
  u8g2.sendBuffer();
  if (millis() - ultimoGuardado >= intervaloGuardado) {
    // Actualizar el registro del último guardado
    ultimoGuardado = millis();
    pixels.setPixelColor(0, pixels.Color(255, 255, 0));  // Calipso
    pixels.show();
    u8g2.setCursor(60, 120);  // y = 96
    if (SDok == true) {
      u8g2.print("Guardando");
      u8g2.sendBuffer();
      // Formatea tus datos en una cadena
      String data = String(fecha) + "," + String(hora) + "," + String(tempe) + "," + String(humi) + "," + String(PMS1) + "," + String(PMS2_5) + "," + String(PMS10) + "," + String(FMHDSB) + "," + String(ultimoRun) + "\n";
      if (SDok) {
        // Guarda los datos en el archivo
        appendFile(SD, "/data.csv", data.c_str());
      }
    }
  }
}

String getFecha(DateTime now) {
  char fecha[11];
  sprintf(fecha, "%02d/%02d/%04d", now.day(), now.month(), now.year());
  return String(fecha);
}

String getHora(DateTime now) {
  char hora[9];
  sprintf(hora, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  return String(hora);
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

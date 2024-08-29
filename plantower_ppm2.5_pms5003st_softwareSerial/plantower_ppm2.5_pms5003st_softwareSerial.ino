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

// Configuración del SoftwareSerial
SoftwareSerial pms5(19, 18);  // RX, TX

char col;
unsigned int PMSa1 = 0, PMSa2_5 = 0, PMSa10 = 0, FMHDSa = 0, TPSa = 0, HDSa = 0, PMSb1 = 0, PMSb2_5 = 0, PMSb10 = 0, FMHDSb = 0, TPSb = 0, HDSb = 0;
unsigned int PMS1 = 0, PMS2_5 = 0, PMS10 = 0, TPS = 0, HDS = 0, CR1 = 0, CR2 = 0;
unsigned char bufferRTT[32] = {};  //serial receive data
char tempStr[15];
float FMHDSB = 0;

void setup() {
  Serial.begin(115200);
  pms5.begin(9600);
 
}
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
  }
}

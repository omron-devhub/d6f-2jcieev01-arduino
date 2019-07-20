/*
 * MIT License
 * Copyright (c) 2019, 2018 - present OMRON Corporation
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* includes */
#include <Wire.h>

#define GPIO_LED_R_PIN 4
#define GPIO_LED_G_PIN 5
#define GPIO_LED_B_PIN 6

/* defines */
#define  D6F_addr   0x6C // D6F-PH Slave address at 7bit expression
//#define  RANGE_MODE  100 // +/-50[Pa] range
#define  RANGE_MODE  250 // 0-250[Pa] range
//#define  RANGE_MODE 1000 // +/-500[Pa] range

byte  rbuf[32];

void  I2C_WR(char sadd, byte *dbuf, byte num){
  int i = 0;
  
  Wire.beginTransmission(sadd); // I2C start
  while (num--){
    Wire.write(dbuf[i]); // I2C write
    i++;
  }
  Wire.endTransmission(); // I2C stop
}

word I2C_RD_16 (char sadd, byte *dbuf, byte num){
  int i = 0;
  word rd_fifo;
  byte rd_buf[2];
  
  Wire.beginTransmission(sadd); // I2C start
  while (num--){
    Wire.write(dbuf[i]); // I2C write
    i++;
  }
  Wire.endTransmission(); // I2C stop

  Wire.requestFrom(sadd, 2);
  i = 0;
  while(Wire.available()){
   rd_buf[i++] = Wire.read(); // first received byte stored here
  }
  rd_fifo = (word)((rd_buf[0] << 8) | rd_buf[1]);

  return rd_fifo;  
}

void setup()
{
    Serial.begin(115200);
    Serial.println("peripherals: GPIO");
    pinMode(GPIO_LED_R_PIN, OUTPUT);
    pinMode(GPIO_LED_G_PIN, OUTPUT);
    pinMode(GPIO_LED_B_PIN, OUTPUT);

    digitalWrite(GPIO_LED_R_PIN, LOW);
    digitalWrite(GPIO_LED_G_PIN, LOW);
    digitalWrite(GPIO_LED_B_PIN, LOW);

    Serial.println("peripherals: I2C");
    Wire.begin();  // master

    Serial.println("sensor: Differential pressure Sensor");
    delay(32);

    Wire.begin(); // i2c master

   // EEPROM Control <= 0x00h
   byte send0[] = {0x0B, 0x00};
   I2C_WR(D6F_addr, send0, 2);

   // MCU mode <= 0x1F(31) //
 //  byte send1[] = {0x00, 0xD0, 0x47, 0x18, 0x1F}; // <= This line maybe needless for mass production sample.
  // I2C_WR(D6F_addr, send1, 5);                    // <= This line maybe needless for mass production sample.

}
    
 /** <!-- loop - Differential pressure sensor {{{1 -->
 * 1. blink LEDs
 * 2. read and convert sensor.
 * 3. output results, format is: [Pa]
 */
void loop()
{
  static bool blink = false;
  int  i,j;
  int  itemp;
  word rd_flow;
  float Flow_rate;
  
  blink = !blink;
    digitalWrite(GPIO_LED_R_PIN, blink ? HIGH: LOW);
    digitalWrite(GPIO_LED_G_PIN, blink ? HIGH: LOW);
    digitalWrite(GPIO_LED_B_PIN, blink ? HIGH: LOW);
    delay(900);

  byte send0[] = {0x00, 0xD0, 0x40, 0x18, 0x06};
  I2C_WR(D6F_addr, send0, 5);

  delay(50); // wait 50ms

  byte send1[] = {0x00, 0xD0, 0x51, 0x2C};
  I2C_WR(D6F_addr, send1, 4);

  byte send2[] = {0x07};
  rd_flow  = I2C_RD_16(D6F_addr, send2, 1); /* read from [07h] */

  
  if(RANGE_MODE == 250){
      Flow_rate = ((float)rd_flow - 1024.0) * RANGE_MODE / 60000.0;
  }
  else{
      Flow_rate = ((float)rd_flow - 1024.0) * RANGE_MODE / 60000.0 - RANGE_MODE/2;
  }
  Serial.print("sensor output:");
  Serial.print(Flow_rate, 2); //print converted flow rate
  Serial.println("[Pa]");
  ; // post operation.
}
// vi: ft=arduino:fdm=marker:et:sw=4:tw=80

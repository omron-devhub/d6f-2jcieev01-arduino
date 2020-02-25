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
#include <Arduino.h>

/* defines for D6T-01 */
#define D6T_ADDR 0x0A  // for I2C 7bit address
#define D6T_CMD 0x4C  // for D6T-44L-06/06H, D6T-8L-09/09H, for D6T-1A-01/02
#define N_ROW 1
#define N_PIXEL 1
#define N_READ ((N_PIXEL + 1) * 2 + 1)
uint8_t rbuf[N_READ];

/* defines for D6F-10 & D6F-PH0505 */
#define D6F_ADDR 0x6C  // D6F-PH I2C client address at 7bit expression

/* defines for B5W-LB */
#if defined(ARDUINO_FEATHER_ESP32)
#define PIN_B5WLB_IN      13
#define PIN_B5WLB_OUT     33
#else
#define PIN_B5WLB_IN      19
#define PIN_B5WLB_OUT     17
#endif

/* defines for B5W-LD */
#if defined(ARDUINO_FEATHER_ESP32)
#define PIN_VOUT1     16
#define PIN_VOUT2     17
#define PIN_VTH       26
#define PIN_EN        21
#else
#define PIN_VOUT1     13
#define PIN_VOUT2     14
#define PIN_VTH       15
#define PIN_EN        A3
#endif

static uint16_t state = 0;

static int counts_idx = 0;
static int counts20sec_vout1 = 0;
static int counts20sec_vout2 = 0;

/** D6T function ********/
uint8_t calc_crc(uint8_t data) {
    int index;
    uint8_t temp;
    for (index = 0; index < 8; index++) {
        temp = data;
        data <<= 1;
        if (temp & 0x80) {data ^= 0x07;}
    }
    return data;
}

/** <!-- D6T_checkPEC {{{ 1--> D6T PEC(Packet Error Check) calculation.
 * calculate the data sequence,
 * from an I2C Read client address (8bit) to thermal data end.
 */
bool D6T_checkPEC(uint8_t buf[], int n) {
    int i;
    uint8_t crc = calc_crc((D6T_ADDR << 1) | 1);  // I2C Read address (8bit)
    for (i = 0; i < n; i++) {
        crc = calc_crc(buf[i] ^ crc);
    }
    bool ret = crc != buf[n];
    if (ret) {
        Serial.print("PEC check failed:");
        Serial.print(crc, HEX);
        Serial.print("(cal) vs ");
        Serial.print(buf[n], HEX);
        Serial.println("(get)");
    }
    return ret;
}


/** D6T function <!-- conv8us_s16_le {{{1 --> convert a 16bit data from the byte stream.*/
int16_t conv8us_s16_le(uint8_t* buf, int n) {
    int ret;
    ret = buf[n];
    ret += buf[n + 1] << 8;
    return (int16_t)ret;   // and convert negative.
}

/**D6F function ************************/
uint8_t conv16_u8_h(int16_t a) {
    return (uint8_t)(a >> 8);
}

uint8_t conv16_u8_l(int16_t a) {
    return (uint8_t)(a & 0xFF);
}

uint16_t conv8us_u16_be(uint8_t* buf) {
    return (uint16_t)(((uint32_t)buf[0] << 8) | (uint32_t)buf[1]);
}

/** D6F <!-- i2c_write_reg16 {{{1 --> I2C write bytes with a 16bit register.*/
bool i2c_write_reg16(uint8_t slave_addr, uint16_t register_addr,
                     uint8_t *write_buff, uint8_t len) {
    Wire.beginTransmission(slave_addr);

    Wire.write(conv16_u8_h(register_addr));
    Wire.write(conv16_u8_l(register_addr));

    if (len != 0) {
        for (uint8_t i = 0; i < len; i++) {
            Wire.write(write_buff[i]);
        }
    }
    Wire.endTransmission();
    return false;
}

/** D6F <!-- i2c_read_reg8 {{{1 --> I2C read bytes with a 8bit register. */
bool i2c_read_reg8(uint8_t slave_addr, uint8_t register_addr,
                   uint8_t *read_buff, uint8_t len) {
    Wire.beginTransmission(slave_addr);
    Wire.write(register_addr);
    Wire.endTransmission();

    Wire.requestFrom(slave_addr, len);

    if (Wire.available() != len) {
        return true;
    }
    for (uint16_t i = 0; i < len; i++) {
        read_buff[i] = Wire.read();
    }
    return false;
}

/** <!-- setup {{{1 -->
 * 1. initialize a Serial port for output.
 * 2. initialize an I2C peripheral.
 */
void setup() {
    Serial.begin(115200);  // Serial baudrate = 115200bps
    Wire.begin();  // i2c master
  
    delay(32);
    // D6F setup: EEPROM Control <= 0x00h
    i2c_write_reg16(D6F_ADDR, 0x0B00, NULL, 0);

    // B5W-LB setup: turn off LED for safe operation.
    pinMode(PIN_B5WLB_IN, OUTPUT);
    pinMode(PIN_B5WLB_OUT, INPUT);
    digitalWrite(PIN_B5WLB_IN, LOW);

    // B5W-LD setup:
    pinMode(PIN_VOUT1, INPUT);
    pinMode(PIN_VOUT2, INPUT);
    // enable level-shifters for OUT1/OUT2 on 2JCIE-EV01
    pinMode(PIN_EN, OUTPUT);
    digitalWrite(PIN_EN, HIGH);
    // setup B5W Threshould Voltage to 0.5[V]
    pinMode(PIN_VTH, OUTPUT);
    #if defined(ARDUINO_FEATHER_ESP32)
        dacWrite(PIN_VTH, 0.5 / (3.3 / 256));
    #else
        analogWriteResolution(10);
        analogWrite(PIN_VTH, 0.5 / (3.3 / 1024));
    #endif
}

/** <!-- loop - Thermal sensor {{{1 -->
 * 1. read sensor.
 * 2. output results, format is: [degC]
 */
void loop() {
    int i, j;
  /**** B5W-LB *******************************/
  int32_t i_low = 0;
  float f_low = 0;
  int32_t i_high = 0;
  float f_high = 0;
  float f_diff = 0;
  /*****************************************/
  /**** D6T *******************************/
  int16_t itemp = 0;
  /*****************************************/
  /**** D6F *******************************/
  uint8_t send0[] = {0x40, 0x18, 0x06};
  uint8_t send1[] = {0x51, 0x2C};
  uint8_t rbuf[2];
  uint16_t rd_flow = 0;
  float flow_rate = 0;
  float pressure = 0; 
  /*****************************************/
  /**** B5W-LD *******************************/
  // VOUT1/2 sampling in 1sec.
	uint8_t prv_gpio1 = LOW;
    uint8_t prv_gpio2 = LOW;
    int now = millis(), prev = now;
    int counts_vout1 = 0;
    int counts_vout2 = 0;
  /*****************************************/
  
  while (state < 5000){
    switch(state){
      /********** B5W-LB ****************************************************/
      case 0: 
        // get Vout(OFF): output voltage when LED is turn off.
        digitalWrite(PIN_B5WLB_IN, LOW);
        break;
      case 1:
      case 2:
      case 3:
      case 4:     
        break;
      case 5: //after 1000us 
        i_low = analogRead(PIN_B5WLB_OUT);
        f_low = (float)i_low * 3.3 / 1024.0;
        break;
      case 6: //after 200us 
        // get Vout(ON): output voltage when LED is turn on.
        digitalWrite(PIN_B5WLB_IN, HIGH);
        break;      
      case 7:
        break;  
      case 8: //after 400us 
        i_high = analogRead(PIN_B5WLB_OUT);
        #if defined(ARDUINO_FEATHER_ESP32)
        f_high = (float)i_high * 3.6 / 4096.0;   // for Feather ESP32
        #else
        f_high = (float)i_high * 3.3 / 1024.0;   // for Arduino MKR
        #endif
        break;  
      case 9:
        break;      
      case 10: //after 400us   should not be exceed 40% ON time of period.
        // turn off to next period.
        digitalWrite(PIN_B5WLB_IN, LOW);
        f_diff = f_high - f_low;  // cancellation processing for external disturbing light interference.
        f_diff = f_diff < 0.0 ? 0.0: f_diff;
        break;      
      /**********************************************************************/
      /********** D6T ****************************************************/
      case 900: 
        memset(rbuf, 0, N_READ);
        Wire.beginTransmission(D6T_ADDR);  // I2C client address
        Wire.write(D6T_CMD);               // D6T register
        Wire.endTransmission();            // I2C repeated start for read
        Wire.requestFrom(D6T_ADDR, N_READ);
        i = 0;
        while (Wire.available()) {
          rbuf[i++] = Wire.read();
        }
        if (D6T_checkPEC(rbuf, N_READ - 1)) {
          return;
        }    
        itemp = conv8us_s16_le(rbuf, 2);
        break;  
     /**********************************************************************/
     /********** D6F ****************************************************/
     case 12:
      i2c_write_reg16(D6F_ADDR, 0x00D0, send0, 3);      
      break;
     case 800:
      i2c_write_reg16(D6F_ADDR, 0x00D0, send1, 2);      
      //if (i2c_read_reg8(D6F_ADDR, 0x07, rbuf, 2)) {  // read from [07h]
      //  return;
      //}
      i2c_read_reg8(D6F_ADDR, 0x07, rbuf, 2);
      rd_flow = conv8us_u16_be(rbuf);
      flow_rate = ((float)rd_flow - 1024.0) * 10.0 / 60000.0;   // Flow sensor D6F-10 :  0-10[L/min] range
      pressure = ((float)rd_flow - 1024.0) * 100.0 / 60000.0 - 50.0; // Differential pressure sensor D6F-PH0505 : +/-50[Pa] range     
      break;  
    /**********************************************************************/
     default:
      break;
    }
    
    delayMicroseconds(200);
    state ++;
    
    /********** B5W-LD ****************************************************/
    uint8_t cur_gpio1 = digitalRead(PIN_VOUT1);
    uint8_t cur_gpio2 = digitalRead(PIN_VOUT2);
    if (cur_gpio1 == HIGH && prv_gpio1 == LOW) {
      counts_vout1++;
    }
    if (cur_gpio2 == HIGH && prv_gpio2 == LOW) {
      counts_vout2++;
    }
    prv_gpio1 = cur_gpio1;
    prv_gpio2 = cur_gpio2;  
    now = millis();
    /**********************************************************************/
  }
  state = 0;
  Serial.print(f_diff, 2);
  Serial.println("  [V]     (B5W-LB)"); 
  Serial.print(itemp / 10.0, 1);  
  Serial.println(" [degC]   (D6T-01)");   
  Serial.print(pressure, 2);  // print converted flow rate
  Serial.println(" [Pa]     (D6F-PH)");     
  Serial.print(flow_rate, 2);  // print converted flow rate
  Serial.println(" [L/min]  (D6F-10)");    
  // output 2sec counts, just for debug.
  Serial.print(counts_vout1 - counts_vout2, 1);
  Serial.print(  "[count]");
  // summarize and output 20sec counts
  if (counts_idx < 19) {   // summarize, output empty data.
    Serial.print(", ----");
    counts20sec_vout1 += counts_vout1;
    counts20sec_vout2 += counts_vout2;
    counts_idx++;
  } 
  else {                 // output 20sec counts.
    Serial.print(", ");
    Serial.print(counts20sec_vout1 - counts20sec_vout2, 1);
    counts_idx = counts20sec_vout1 = counts20sec_vout2 = 0;
  }
  Serial.println("[20s count]   (B5W-LD)");
  Serial.println(" ");       
}
// vi: ft=arduino:fdm=marker:et:sw=4:tw=80

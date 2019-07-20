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

/* defines */
#define D6F_ADDR 0x6C  // D6F-PH I2C client address at 7bit expression
// #define RANGE_MODE 100  // +/-50[Pa] range
#define RANGE_MODE 250      // 0-250[Pa] range
// #define RANGE_MODE 1000     // +/-500[Pa] range


void I2C_WR(char sadd, uint8_t* dbuf, uint8_t num) {
    int i = 0;

    Wire.beginTransmission(sadd);  // I2C start
    while (num--) {
        Wire.write(dbuf[i]);  // I2C write
        i++;
    }
    Wire.endTransmission();  // I2C stop
}

int16_t I2C_RD_16(char sadd, uint8_t* dbuf, uint8_t num) {
    int i = 0;
    int16_t rd_fifo;
    uint8_t rd_buf[2];

    Wire.beginTransmission(sadd);   // I2C start
    while (num--) {
        Wire.write(dbuf[i]);        // I2C write
        i++;
    }
    Wire.endTransmission();         // I2C stop

    Wire.requestFrom(sadd, 2);
    i = 0;
    while (Wire.available()) {
        rd_buf[i++] = Wire.read();  // first received byte stored here
    }
    rd_fifo = (int16_t)((rd_buf[0] << 8) | rd_buf[1]);

    return rd_fifo;
}

void setup() {
    Serial.begin(115200);
    Serial.println("peripherals: I2C");
    Wire.begin();  // i2c master

    Serial.println("sensor: Differential pressure Sensor");
    delay(32);

    // EEPROM Control <= 0x00h
    uint8_t send0[] = {0x0B, 0x00};
    I2C_WR(D6F_ADDR, send0, 2);
}

 /** <!-- loop - Differential pressure sensor {{{1 -->
 * 1. read and convert sensor.
 * 2. output results, format is: [Pa]
 */
void loop() {
    int16_t rd_flow;
    float flow_rate;

    delay(900);

    uint8_t send0[] = {0x00, 0xD0, 0x40, 0x18, 0x06};
    I2C_WR(D6F_ADDR, send0, 5);

    delay(50);  // wait 50ms

    uint8_t send1[] = {0x00, 0xD0, 0x51, 0x2C};
    I2C_WR(D6F_ADDR, send1, 4);

    uint8_t send2[] = {0x07};
    rd_flow = I2C_RD_16(D6F_ADDR, send2, 1);  // read from [07h]


    if (RANGE_MODE == 250) {
        flow_rate = ((float)rd_flow - 1024.0) * RANGE_MODE / 60000.0;
    } else {
        flow_rate = ((float)rd_flow - 1024.0) * RANGE_MODE / 60000.0 - RANGE_MODE / 2;
    }
    Serial.print("sensor output:");
    Serial.print(flow_rate, 2);  // print converted flow rate
    Serial.println("[Pa]");
}
// vi: ft=arduino:fdm=marker:et:sw=4:tw=80

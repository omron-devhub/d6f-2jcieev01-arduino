# d6f-2jcieev01-arduino
It is a sample projects for OMRON **D6F** MEMS Flow Sensors with
evaluation kit **2JCIE-EV01-AR1**,
**2JCIE-EV01-FT1** and some Arduino boards.

## D6F-PH sensor series
A Compact, High-accuracy Differential
Pressure Sensor with Superior Resistance to Environments.


## D6F sensor series
can Measure Low-flow rate and
Reduce to Piping time by quick joint connection


## Description
this Arduino sample projects for acquiring data from sensors on 2JCIE-EV01.
sample projects output the sensor data to USB-Serial ports.

| example    | description           | baord |
|:----------:|:----------------------|:-----------------------|
| d6f-ph0025 | differential pressure | Arduino MKR-WiFi1010/ Adafruit Feather ESP32 |
| d6f-ph0505 | differential pressure | Arduino MKR-WiFi1010/ Adafruit Feather ESP32 |
| d6f-ph0550 | differential pressure | Arduino MKR-WiFi1010/ Adafruit Feather ESP32 |
| d6f-10     | for Air flow          | Arduino MKR-WiFi1010/ Adafruit Feather ESP32 |
| d6f-20     | for Air flow          | Arduino MKR-WiFi1010/ Adafruit Feather ESP32 |
| d6f-50     | for Air flow          | Arduino MKR-WiFi1010/ Adafruit Feather ESP32 |
| d6f-70     | for Air flow          | Arduino MKR-WiFi1010/ Adafruit Feather ESP32 |


## DEMO
sample output from D6F-20A7D

```
sensor output:-0.08[L/min]
sensor output:-0.06[L/min]
sensor output:-0.05[L/min]
sensor output:-0.06[L/min]
sensor output:21.50[L/min]
```


## Installation
see `https://www.arduino.cc/en/guide/libraries`

### Install from Arduino IDE
1. download .zip from this repo [releases](releases)
    or [master](archive/master.zip) .
2. Import the zip from Arduino IDE

    ![install-ide-import-lib](https://user-images.githubusercontent.com/48547675/55043017-9a34e980-5077-11e9-885d-03f9f82e3491.JPG)

    ![install-select-zip](https://user-images.githubusercontent.com/48547675/55043034-a7ea6f00-5077-11e9-99d5-26423fb652b5.JPG)

3. Then, you can see the samples in `File >> Examples` menu.

    ![install-select-examples](https://user-images.githubusercontent.com/48547675/55043028-a28d2480-5077-11e9-8365-6745cda417ff.JPG)

4. Select examples for your favorite sensors, build and program to boards.

### Manual install
1. download this repo

    ```shell
    $ git clone https://github.com/omron-devhub/d6f-2jcieev01-arduino
    ```

2. launch Arduino-IDE and select our sketch to load.
3. build and program to boards.


## Dependencies
None


## Links
- [Arduino samples for 2JCIE-01-AR1/FT1](https://github.com/omron-devhub/2jcieev01-arduino)
- [RaspberryPi samples for 2JCIE-01-RP1](https://github.com/omron-devhub/2jcieev01-raspberrypi)
- [Arduino sample for D6T on 2JCIE-01-AR1/FT1](https://github.com/omron-devhub/d6t-2jcieev01-arduino)
- [RaspberryPi sample for D6T on 2JCIE-01-RP1](https://github.com/omron-devhub/d6t-2jcieev01-raspberrypi)
- [RaspberryPi sample for D6F on 2JCIE-01-RP1](https://github.com/omron-devhub/d6f-2jcieev01-raspberrypi)
- [Arduino sample for B5W on 2JCIE-01-AR1/FT1](https://github.com/omron-devhub/b5w-2jcieev01-arduino)


## Licence
Copyright (c) OMRON Corporation. All rights reserved.

Licensed under the MIT License.


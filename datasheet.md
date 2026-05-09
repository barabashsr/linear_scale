#### Shenzhen QM Smart Panlee Technology Co., Ltd.

# Smart Panlee

# Smart Serial LCD Display

## ZX7D00CE01S


```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
## Revision History:

```
Date Revised by Description
February 9, 2023 Hades First release
```
## Features:

#### 1. Support rapid prototyping

## Core Materials (Tab. 0):

#### No. Name Model Remark

#### 1 ESP32-S3 Module WT32-S3-WROVER-N16R


```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
## Hardware Interface:

### Hardware interface diagram:

#### Fig. 1 Hardware Interface


```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
## Interface Description:

### [1] USB Interface (Tab.1)

#### Note: By default, the USB connector is only used for power supply, and a USB strobe resistor needs

#### to be pasted when using as a USB interface. And be careful not to use the GPIO_19 and GPIO_

#### in the IO expansion interface.

### [2] Debug Interface (Tab.2)

#### Pin Description Module Pin

#### Voltage

#### Range Remark^

1 +5V - 5V (^)
2 +3.3V 3.3V For reference, not for power
input
3 ESP_TXD TXD0 3.3V TTL (^)
4 ESP_RXD RXD0 3.3V TTL (^)
5 EN EN 0- 3.3V (^) Chip enable
6 (^) BOOT GPIO 0 0- 3.3V (^)
7 GND 0V (^) Ground

### [3] Touch Interface (Tab.3)

#### Description Module Pin Remark

TP_SCL (^) GPIO 47 Multiplexed with IIC
TP_SDA (^) GPIO 48 Multiplexed with IIC
TP_INT (^) Not connected
TP_RST (^) AW9523 P11 Connect to the IO expansion chip

### [4] LCD Interface (Tab.4）

#### Description Module Pin Remark

BL_PWM (^) GPIO 45 Backlight IO


```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
##### LCD_RST

##### AW

```
P10 chip
```
```
Connect to the IO expansion chip
```
LCD_RGB_D15 (^) GPIO 14 Backlight IO
LCD_RGB_D14 (^) GPIO 13 Connect to the IO expansion chip
LCD_RGB_D13 (^) GPIO 12
RGB interface
LCD_RGB_D12 (^) GPIO 11
LCD_RGB_D11 (^) GPIO 10
LCD_RGB_D10 (^) GPIO 18
LCD_RGB_D9 (^) GPIO 08
LCD_RGB_D8 (^) GPIO 03
LCD_RGB_D7 (^) GPIO 46
LCD_RGB_D6 (^) GPIO 00
LCD_RGB_D5 (^) GPIO 21
LCD_RGB_D4 (^) GPIO 06
LCD_RGB_D3 (^) GPIO 07
LCD_RGB_D2 (^) GPIO 15
LCD_RGB_D1 (^) GPIO 16
LCD_RGB_D0 (^) GPIO 17
LCD_RGB_PCLK (^) GPIO 09
LCD_RGB_HS (^) GPIO 05
LCD_RGB_VS (^) GPIO 38
LCD_RGB_DE (^) GPIO 39

### [5] Extended IO Interface (Tab.5)

#### Pin Description Module Pin Remark

```
1/2/39/40 +5V 5V power supply for input and output
```

```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
3/4/37/38 +3.3V out 3.3V output power, not input

5~8/33~36 GND Ground

11 EXT_LSIO_0 AW9523 P

```
Connect to the IO expansion chip,
3.3V TTL input/output
```
##### 12 EXT_LSIO_1 AW9523 P

##### 13 EXT_LSIO_2 AW9523 P

##### 14 EXT_LSIO_3 AW9523 P

##### 15 EXT_LSIO_4 AW9523 P

##### 16 EXT_LSIO_5 AW9523 P

##### 17 EXT_LSIO_6 AW9523 P

##### 18 EXT_LSIO_7 AW9523 P

##### 23 EXT_GPIO_0 GPIO 20

```
Connect to ESP32-S3 GPIO,
```
```
3.3V TTL input/output
```
##### 24 EXT_GPIO_1 GPIO 19

##### 25 EXT_GPIO_2 GPIO 41

##### 26 EXT_GPIO_3 GPIO 40

##### 27 EXT_GPIO_4 GPIO 04

##### 28 EXT_GPIO_5 GPIO 42

##### 29 EXT_GPIO_6 GPIO 01

##### 30 EXT_GPIO_7 GPIO 02

##### 31 IIC_SCL GPIO 47

IIC port, 3.3V TTL input/output
32 IIC_SDA GPIO 48


```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
## Interface Encapsulation:

#### Interface Description

#### Interface

#### Encapsulation

#### Remark

```
Debug Interface
```
```
Molex SD- 53261 - 0771
(7Pin) WAFER MX1.25 7P
Horizontal Patch
```
##### MX1.

```
USB Interface - USB-typeC
```
```
Extended IO Interface 1 2.00mm 2×20P Pin
Socket SMT
```
```
2.0mm 2×20P Double-row Pin
Socket
```
```
Extended IO Interface 2
```
```
2.54mm 2×20P Pin
Header/Socket SMT
```
```
2.54mm 2×20P Double-row Pin
Header/Socket
```
## Hardware Peripherals:

#### Peripheral Name Description

```
LCD 7 Inch display with RGB interface
```
```
Touch Panel IIC port, no interrupt, reset using expansion IC
```
```
IO Expansion Chip Model AW9523B, mounted on board IIC bus
```

```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
## Schematic:


```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
## Specification Parameters:

### [1] Display Parameters (Tab.6)

```
Display Type
TN
```
```
Drive IC Model
```
```
Visual Angle
30°
```
```
Resolution
800*
```
```
Interface
RGB
```
```
Color
RGB
```
```
Backlight Mode
LED
```
### [2] Touch Parameters (Tab.7)

**Touchscreen Type** (^) Capacitive touch
**Drive IC Model** (^) -
**Interface** (^) I2C
**Touchscreen Structure** (^) G+F
**Touch Mode** (^) Surface touch
**Surface Hardness** (^) 6H
**Light Transmittance** (^) -


```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
## Outline Dimensional Drawing (Fig.2)


```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
## Firmware Burning:

#### 1. Connect the downloader (ZXACC-ESPDB) via a USB-TypeC cable. And then connect the

#### ZX7D00CE01S board with the downloader (ZXACC-ESPDB) through a data cable. As the

#### downloader (ZXACC-ESPDB) has automatic data flow processing capabilities, the firmware

#### can be downloaded automatically through the ESP32 Flash Download Tools.

#### Fig.

#### 2. As shown in Fig. 4: Select the firmware path at mark 1, and

#### then fill in the burning address, usually 0X00. Note that this

#### checkbox must be checked; Set the crystal frequency to be

#### 40MHz at mark 2; Select 32Mbit for Flash size at mark 3; Select

#### DIO for SPI MODE at mark 4; Select the COM port number

#### recognized by the computer at mark 5; Select the baud rate at

#### mark 6 (the higher the value is, the faster the firmware will be

#### downloaded. Max. 1152000bps).

#### Fig. 4

#### 3. After finishing the previous configuration, click START at

#### mark 7 to start burning the firmware.

#### 4. Complete the above steps, and then press the reset button

#### on the back of the development board to start running the

#### firmware you just burned.

## Software Design Reference:

## URL：https://www.espressif.com.cn/en/support/documents/technical-documents

## Online GUI Designer:

#### Users can use our online GUI designer platform, which is similar to MIT APP Inventor, to

#### achieve the rapid GUI development with building blocks. Currently, the platform has perfected

#### the graphic interface development, and more driver code blocks will be further improved in

#### the future.

#### Login Page: http://8ms.xyz/login

#### User Manual: https://doc.smartpanle.cn/ESP32-S3/index.html


```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
#### Fig.

#### Fig.


```
Shenzhen QM Smart Panlee Technology Co., Ltd.
```
## Contact Us

#### URL: http://www.panel-tag.cn/

#### E-mail: panlee@smartpanle.com

#### Get Samples: https://shop212317088.taobao.com/

#### Wechat Official Account: Smartpanle



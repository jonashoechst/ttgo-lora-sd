# TTGO LoRa + SD Card

The TTGO LoRa boards have some issues when using together with SD cards. Both the SX1278 and SD cards use SPI as communication bus. SPI is a protocol family can work in many different configurations.

## ESP32: Software and Hardware SPI

The ESP32 on the TTGO LoRa board has a builtin **hardware SPI** (HSPI) and can also do **virtual / software SPI** (VSPI) through emulation. Also the VSPI busses as well as the HSPI can be routed to **any** of the pins.

![TTGO LoRa Pinout](TTGO-LoRa.jpg)

In the default configuration both SX1278 and the SD library uses the HSPI and thereby impede each other. 

## Using multiple busses

In order to make the SX1278 and SD card working, two SPI busses need to be used. While experimenting with the board it also became obvious, that the SX1278 is error-prone on the hardware SPI, and did **not** work, when using the SD card on a VSPI.

The final solution uses the **HSPI** bus for communication with SD and a software SPI for the SX1278. It is also an option to use two seperate software SPI busses.

### SD on rerouted HSPI

```c
#define SD_CS 23
#define SD_SCK 17
#define SD_MOSI 12
#define SD_MISO 13

SPIClass sd_spi(HSPI);
sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

if (!SD.begin(SD_CS, sd_spi))
    Serial.println("SD Card: mounting failed.");
else 
    Serial.println("SD Card: mounted.");
``` 

### RH95 on VSPI

```c
RHSoftwareSPI sx1278_spi;
RH_RF95 rf95(LORA_CS, LORA_IRQ, sx1278_spi);

sx1278_spi.setPins(LORA_MISO, LORA_MOSI, LORA_SCK);
if (!rf95.init()) 
    Serial.println("LoRa Radio: init failed.");
else
    Serial.println("LoRa Radio: init OK!");
```
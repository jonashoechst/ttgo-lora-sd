#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <RH_RF95.h>
#include <RHSoftwareSPI.h>

// SD SPI pins can be chosen freely, but cannot overlap with other ports!
#define SD_CS 23
#define SD_SCK 17
#define SD_MOSI 12
#define SD_MISO 13

#define LORA_FREQ 868.0

#define LOG_PATH "/lora_recv.log"

// The sd card can also use a virtual SPI bus
SPIClass sd_spi(HSPI);

// Use a virtual (software) SPI bus for the sx1278
RHSoftwareSPI sx1278_spi;
RH_RF95 rf95(LORA_CS, LORA_IRQ, sx1278_spi);

void setup() {
    // Builtin LED
    pinMode(LED_BUILTIN, OUTPUT);

    // Serial output
    Serial.begin(115200);

    // LoRa: Init
    pinMode(LORA_RST, OUTPUT);
    digitalWrite(LORA_RST, LOW);
    delay(100);
    digitalWrite(LORA_RST, HIGH);

    // the pins for the virtual SPI explicitly to the internal connection
    sx1278_spi.setPins(LORA_MISO, LORA_MOSI, LORA_SCK);

    if (!rf95.init()) 
        Serial.println("LoRa Radio: init failed.");
    else
        Serial.println("LoRa Radio: init OK!");

    // LoRa: set frequency
    if (!rf95.setFrequency(LORA_FREQ))
        Serial.println("LoRa Radio: setFrequency failed.");
    else
        Serial.printf("LoRa Radio: freqency set to %f MHz\n", LORA_FREQ);

    rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128);

    // LoRa: Set max (23 dbm) transmission power. 
    rf95.setTxPower(23, false);

    // SD Card
    sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    if (!SD.begin(SD_CS, sd_spi))
        Serial.println("SD Card: mounting failed.");
    else 
        Serial.println("SD Card: mounted.");

}

uint8_t lora_buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t lora_len;
uint8_t receive_counter = 0;

void loop() {
    // Blink LED
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    lora_len = RH_RF95_MAX_MESSAGE_LEN;
    if (rf95.recv(lora_buf, &lora_len)) {
        receive_counter++;
        Serial.printf("Received LoRa message #%i (%i bytes):\n%s\n", receive_counter, lora_len, lora_buf);

        File test = SD.open(LOG_PATH, FILE_APPEND);
        if (!test) {
            Serial.println("SD Card: writing file failed.");
        } else {
            Serial.printf("SD Card: appending data to %s.\n", LOG_PATH);
            test.write(lora_buf, lora_len);
            test.printf("\n\n");
            test.close();
        }
    }

    delay(100);
}
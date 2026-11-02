![logo](docs/images/ultra-logo.png)

![ultra picture](docs/images/ultra-overview.png)

# Differences from upstream (mmame fork)

This fork adds an SPI slave transport, so a host MCU (e.g. an ESP32) can control the device
without USB, and makes the ISO14443-4 APDU relay more robust. Everything else is upstream.

* **SPI transport** (`firmware/application/src/spi_main.c`), build with `make SPI_TRANSPORT=1`.
  nRF52840 is SPIS2 slave (mode 0, MSB first), carrying the normal Chameleon data frames next to USB/BLE.
  A command is one write transaction starting with a frame (`0x11`); the response is
  clocked out by a read transaction (MOSI all `0x00`) after the IRQ pin went high.
  Default pins: SCK P0.03, MOSI P0.05, MISO P0.07, CSN P0.14, IRQ P0.16
  (override with `-DSPI_PIN_SCK=...` etc.). `SPI_NO_SLEEP=1` disables System OFF,
  because SPI cannot wake the chip. Card data stays in RAM unless `SLOT_DATA_CONFIG_SAVE` is sent.
* **ISO14443-4 relay timeout**: while the host has not answered a relayed APDU the card now
  keeps the reader alive with repeated WTX and answers `6F00` after a configurable host timeout
  (default 10 s, WTXM 59). Late host answers are dropped. New command
  `HF14A_4_SET_HOST_TIMEOUT` (6011): `timeout_ms(2, BE) [wtxm(1)] [apdu_push(1)]`;
  CLI: `emv apdu --host-timeout <ms>`.
* **APDU push over SPI**: with `apdu_push=1` a relayed APDU is delivered as an unsolicited
  `HF14A_4_APDU_RECV` frame (IRQ high) instead of being polled. If IRQ is high, read before sending the next command.

Status: builds for nRF52840; not yet verified on hardware or against a real reader.

# ChameleonUltra Authorized Distributors

Lyon, France: [Lab401](https://lab401.com/)

Santa Ana, United States: [Hackerwarehouse](https://hackerwarehouse.com/)

Hastings, UK: [KSEC](https://labs.ksec.co.uk/product/proxgrind-chameleon-ultra/)

Montreal, Canada: [TechSecurityTools](https://techsecuritytools.com/product/chameleon-ultra/)

Shenzhen, China: [Sneaktechnology](https://sneaktechnology.com)

Guangdong, China: [MTools Tec](https://shop.mtoolstec.com/)

Lazada One, Singapore: [Aliexpress by RRG](https://proxgrind.aliexpress.com/store/1101312023)

# What is it and how to use ?

Read the [available documentation](https://github.com/RfidResearchGroup/ChameleonUltra/wiki).

# Compatible applications

* [ChameleonUltraGUI](https://github.com/GameTec-live/ChameleonUltraGUI)
* [MTools BLE](https://github.com/RfidResearchGroup/ChameleonUltra/wiki/mtoolsble)
* [Mifare Chameleon Tool (iOS only, Beta)](https://apps.apple.com/it/app/mifare-chameleon-tool/id6761231484)
* [Chameleon Ultra (Sailfish OS only)](https://sailfishos-chum.github.io/apps/harbour-chameleon-ultra)
* [Chameleon GO (iOS only)](https://apps.apple.com/us/app/id6802015306)
* [Device console for ChameleonUltra (Web)](https://sec1.dk/phreakbyte.html)

# Videos

*Beware some of the instructions might have changed since recording, check the current documentation when in doubt!*

* [Downloading and compiling the official CLI](https://www.youtube.com/watch?v=VGpAeitNXH0)
* [Downloading ChameleonUltraGUI](https://www.youtube.com/watch?v=rHH7iqbX3nY)
* [ChameleonUltraGUI features overview](https://www.youtube.com/watch?v=YqE8wyVSse4)
* [Using ChameleonUltraGUI and the Chameleon Ultra](https://www.youtube.com/watch?v=9jtKNJ5-kVY)
* [MTools BLE - How to clone a card with ChameleonUltra](https://youtu.be/IvH-xtdW1Wk?si=4exqgAAeJ-kxU3aN)

# Official channels

Where do you find the community?
* [RFID Hacking community discord server](https://t.ly/d4_C)
  * Software/chameleon-dev for firmware and clients development discussions
  * Devices/chameleon-ultra for usage discussions
* [GameTec_live discord server](https://discord.gg/DJ2A4wxncK)

Other notable projects:
* [Fantasi](https://fantasi.cloud)
  * [Discord](https://fantasi.cloud/discord)
* [chameleon-ultra.js](https://github.com/taichunmin/chameleon-ultra.js/) 

###### Searching for the docs repo? Find it [here](https://github.com/RfidResearchGroup/ChameleonUltraDocs)

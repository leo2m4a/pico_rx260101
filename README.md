# pico_rx260101

Raspberry Pi Pico (RP2040) firmware for a LoRa slave node. The device receives RF commands from a master, reads sensor data, controls a relay, and responds with sensor readings over LoRa.

## Target Board

This project is designed for the **宅老大 Pico LoRa** development board, which integrates a Raspberry Pi Pico (RP2040) with an SX1262 LoRa module.

## Features

- LoRa RF communication via SX1262 (915 MHz, SF7, BW 125 kHz)
- Temperature reading via DS18B20 (OneWire)
- Temperature & humidity reading via SHT40 (I2C)
- ADC voltage measurement (GPIO26, with resistor divider)
- Relay output (GPIO28)
- Time-scheduled relay control
- I2C LCD display
- UART debug output (115200 baud)

## Hardware

| Peripheral | Interface | Pins |
|---|---|---|
| SX1262 LoRa | SPI1 | MISO=12, MOSI=11, SCK=10, CS=3 |
| SX1262 control | GPIO | DIO1=20, RST=15, BUSY=2 |
| I2C LCD + SHT40 | I2C1 | SDA=6, SCL=7 |
| DS18B20 | OneWire | GPIO16 |
| UART1 | UART | TX=8, RX=9 |
| Relay K0 | GPIO | GPIO28 |
| ADC input | ADC0 | GPIO26 |

## Dependencies

### Pico SDK

This project requires **Pico SDK 2.2.0**. Follow the [official setup guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) to install the SDK.

### RadioLib (Required — must be cloned manually)

This project uses [RadioLib](https://github.com/jgromes/RadioLib) by Jan Gromeš, licensed under the MIT License.

RadioLib is **not included** in this repository. Clone it into the project root before building:

```bash
git clone https://github.com/jgromes/RadioLib.git
```

The directory structure should look like this:

```
pico_rx260101/
├── RadioLib/          <-- cloned here
├── onewire_library/
├── usr_lib/
├── pico_rx260101.cpp
├── CMakeLists.txt
└── ...
```

## Build

```bash
mkdir build
cd build
cmake ..
make
```

Flash the resulting `.uf2` file to the Pico by holding BOOTSEL and dragging the file onto the USB drive.

## RF Protocol

The device operates as a slave node (SID `0x84`). It listens for 12-byte LoRa packets and replies with sensor data. Command processing is handled in `slv_rf_cmd_proc.cpp`.

## License

Project source code is provided as-is. RadioLib is copyright Jan Gromeš and contributors, used under the [MIT License](https://github.com/jgromes/RadioLib/blob/master/LICENSE).

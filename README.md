# pico_rx260101

Raspberry Pi Pico (RP2040) firmware for a LoRa slave node. The device receives RF commands from a master, reads sensor data, controls a relay, and responds with sensor readings over LoRa.

> **This project is part of a master/slave pair and must be used together with the master node project [pico_mst_260219](https://github.com/leo2m4a/pico_mst_260219).**

## Target Board

This project is designed for the **宅老大 Pico LoRa** development board, which integrates a Raspberry Pi Pico (RP2040) with an SX1262 LoRa module.

- [Purchase](https://www.makdev.net/p/store.html)
- [Pinout](https://www.makdev.net/p/pico-lora-mesh-solar-shield-pinout.html)

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

The full UART/RF packet format is documented in [`docs/pico_lora_cmd2604091.ods`](docs/pico_lora_cmd2604091.ods) (LibreOffice Calc spreadsheet).

### Write commands (`0x78`, subcmd `0x11`)

The slave only acts on a write whose SID (byte 3) matches its own. Byte 5 selects the target:

| Addr (byte 5) | Request data | Effect | Reply |
|---|---|---|---|
| `0x04` | Byte 6 write-enable mask, byte 7 value | Sets the relay bits selected by the mask | 16 bytes: `0x89`, `0x10`, …, `0x11`, `0x04`, sensor data in bytes 6–14, `0xaa` |
| `0x10` | Bytes 6–13 | Stores `gSCH_RLY[0..7]` | 16 bytes, see below |
| `0x11` | Bytes 6–13 | Stores `gSCH_RLY[8..15]` | 16 bytes, see below |
| `0x12` | Bytes 6–13 | Stores `gSCH_RLY[16..23]` | 16 bytes, see below |

The schedule write reply sends back the stored bytes so the master can check them: `0x89`, `0x10`, `0x00`, SID, `0x11`, addr, the 8 stored bytes in bytes 6–13, `0x00`, addr.

### Schedule storage (master EEPROM)

The slave keeps `gSCH_RLY` in RAM only, so schedules written with addr `0x10`–`0x12` are lost on reset. Schedules are stored on the master's AT24C32 EEPROM (I2C `0x57`, 32-byte pages). See [pico_mst_260219](https://github.com/leo2m4a/pico_mst_260219) and the **EEPROM** sheet in the packet format spreadsheet.

| Address | Contents |
|---|---|
| `0x0000` | Reserved for system info |
| `0x0080` | Default relay schedules, 96 bytes (`24 × 4`), loaded by the master at startup |
| `0x0100 + 0x20·n` | Relay n record (relays 0–3), one 32-byte page each |

Relay record layout:

| Offset | Size | Contents |
|---|---|---|
| `+0x00` | 16 | Schedule bytes 0–15 |
| `+0x10` | 8 | Schedule bytes 16–23 |
| `+0x18` | 2 | Signature `0x16`, `0x88`; the record is used only if both match |
| `+0x1A` | 6 | Unused |

At startup the master loads `0x0080`, then each relay with a valid record uses that record instead.

### Poll reply

The poll reply is 20 bytes, with length byte `0x14`: temperature, relay and IO state, ADC, humidity and DS18B20 readings, followed by three `0xaa` bytes.

## License

Project source code is provided as-is. RadioLib is copyright Jan Gromeš and contributors, used under the [MIT License](https://github.com/jgromes/RadioLib/blob/master/LICENSE).

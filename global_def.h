// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi1
#define PIN_MISO 12
#define PIN_CS   13
#define PIN_SCK  10
#define PIN_MOSI 11

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c1
#define I2C_SDA 6
#define I2C_SCL 7

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 8
#define UART_RX_PIN 9

#define UART_EXT 21
// NSS pin
#define RADIO_CS_PIN 3
// DIO1 pin (中斷引腳)
#define RADIO_DIO1_PIN 20
// Reset pin
#define RADIO_RST_PIN 15
// Busy pin
#define RADIO_BUSY_PIN 2

//------ADC
// 分壓電阻配置 (單位: MΩ)
#define VOLTAGE_R1 1.5  // 下電阻 (連接 GND)
#define VOLTAGE_R2 1.0  // 上電阻 (連接外部電壓)
//------
#define PARAM_SID 0x84
// 電壓校正係數 (根據實際測量調整)
// 計算方法: 校正係數 = 實際電壓 / 顯示電壓
// 例如: 實際 3.5V 顯示為 3.37V，則 校正係數 = 3.5 / 3.37 = 1.0386
#define VOLTAGE_CALIBRATION 1.0386  // 預設校正係數

//#define GPIO_BTN_U_PIN 18
#define GPIO_BTN_D_PIN 19

#define GPIO_K0_PIN 28

#define CMD_ECHO 0x55
#define RSP_ECHO 0xaa

#define CMD_SLVWR 0x78
#define RSP_SLVWR 0x89

#define SC_BCN 0
#define SC_PING 0xaa
#define SC_READ 0x35
#define SC_WRITE 0x11

#define VAL_SCH_EN  0x1e
#define VAL_SCH_DIS 0xf1
#define VAL_PWD0 0x16
#define VAL_PWD1 0x08

typedef struct {
    uint8_t temp_h;
    uint8_t temp_l;
    uint8_t humi_h;
    uint8_t humi_l;
    uint8_t adc0_h;
    uint8_t adc0_l;
    uint8_t ds0_h;
    uint8_t ds0_l;
} __attribute__((packed)) SensorData;  // 總共 6 bytes


typedef struct {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} __attribute__((packed)) TimeData;  // 總共 4 bytes

// 感測器資料結構（使用 packed 避免記憶體對齊問題）
typedef struct {
    uint8_t hdr;
    uint8_t length;
    uint8_t mid;
    uint8_t sid;
    uint8_t sec;
    uint8_t data5;//ofs0
    uint8_t data6;
    uint8_t data7;
    uint8_t data8;//ofs3
    uint8_t data9;//ofs4
    uint8_t data10;
    uint8_t data11;
    uint8_t data12;//ofs7
    uint8_t data13;
    uint16_t checksum;       // CRC-16 校驗碼 (2 bytes)
} __attribute__((packed)) RspData;  // 總共 14 bytes
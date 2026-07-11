#include "sensirion_i2c_hal.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

/**
 * Initialize I2C communication on the Raspberry Pi Pico.
 */
void sensirion_i2c_hal_init(void) {
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
}

/**
 * Release I2C communication resources.
 */
void sensirion_i2c_hal_free(void) {
    i2c_deinit(I2C_PORT);
}

/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in data
 * @return        0 on success, error code otherwise
 */
int8_t sensirion_i2c_hal_read(uint8_t address, uint8_t* buffer, uint8_t count) {
    int result = i2c_read_blocking(I2C_PORT, address, buffer, count, false);
    return (result == count) ? 0 : -1;
}

/**
 * Execute one write transaction on the I2C bus, sending a given number of
 * bytes. The bytes in the supplied buffer must be sent to the given address. If
 * the slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from data and send to I2C
 * @return        0 on success, error code otherwise
 */
int8_t sensirion_i2c_hal_write(uint8_t address, const uint8_t* buffer, uint8_t count) {
    int result = i2c_write_blocking(I2C_PORT, address, buffer, count, false);
    return (result == count) ? 0 : -1;
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_i2c_hal_sleep_us(uint32_t useconds) {
    sleep_us(useconds);
}

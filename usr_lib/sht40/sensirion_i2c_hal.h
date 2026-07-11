/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT of THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SENSIRION_I2C_HAL_H
#define SENSIRION_I2C_HAL_H

#include "sensirion_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * sensirion_i2c_hal_init() - initialize I2C communication
 *
 * Initializes the I2C communication on the microcontroller.
 */
void sensirion_i2c_hal_init(void);

/**
 * sensirion_i2c_hal_free() - release I2C communication resources
 */
void sensirion_i2c_hal_free(void);

/**
 * sensirion_i2c_hal_read() - read from a sensor
 * @address:      8-bit I2C address of the sensor
 * @buffer:       buffer to read the data to
 * @count:        number of bytes to read
 *
 * @return:       0 on success, non-zero otherwise
 */
int8_t sensirion_i2c_hal_read(uint8_t address, uint8_t* buffer, uint8_t count);

/**
 * sensirion_i2c_hal_write() - write to a sensor
 * @address:      8-bit I2C address of the sensor
 * @buffer:       buffer with the data to write
 * @count:        number of bytes to write
 *
 * @return:       0 on success, non-zero otherwise
 */
int8_t sensirion_i2c_hal_write(uint8_t address, const uint8_t* buffer,
                               uint8_t count);

/**
 * sensirion_i2c_hal_sleep_us() - sleep for a given duration
 * @useconds:     duration in microseconds
 */
void sensirion_i2c_hal_sleep_us(uint32_t useconds);

#ifdef __cplusplus
}
#endif

#endif /* SENSIRION_I2C_HAL_H */

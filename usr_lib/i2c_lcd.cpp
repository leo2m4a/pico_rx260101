#include <stdio.h>
#include "pico/stdlib.h"

#include "global_def.h"
#include "i2c_lcd.h"
#include "hardware/i2c.h"

//sub function
void i2c_write_byte(uint8_t val) {
//#ifdef i2c_default
//    i2c_write_blocking(i2c_default, addr, &val, 1, false);
//    
//#endif
i2c_write_blocking(I2C_PORT, gPARAM_lcd_i2caddr, &val, 1, false);
}

void lcd_toggle_enable(uint8_t val) {
    // Toggle enable pin on LCD display
    // We cannot do this too quickly or things don't work
#define DELAY_US 600
    sleep_us(DELAY_US);
    i2c_write_byte(val | LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
    i2c_write_byte(val & ~LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
}

// The display is sent a byte as two separate nibble transfers
void lcd_send_byte(uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT;
    uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;

    i2c_write_byte(high);
    lcd_toggle_enable(high);
    i2c_write_byte(low);
    lcd_toggle_enable(low);
}

void lcd_clear(void) {
    lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND);
}

// go to location on LCD
void lcd_set_cursor(int line, int position) {
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(val, LCD_COMMAND);
}

static inline void lcd_char(char val) {
    lcd_send_byte(val, LCD_CHARACTER);
}

void lcd_string(char *s) {
    while (*s!='\0') {
        lcd_char(*s++);
    }
}

void lcd_hex(uint8_t bchar) {
  uint8_t bNum[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  uint8_t bHexh,bHexl;
  
  bHexh = (bchar>>4)&0x0f;
  lcd_char(*(bNum+bHexh));
  bHexl = (bchar>>0)&0x0f;
  lcd_char(*(bNum+bHexl));
}

void lcd_init(void) {
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x02, LCD_COMMAND);

    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
    lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);
    lcd_clear();
}
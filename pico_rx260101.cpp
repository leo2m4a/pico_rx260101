#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/uart.h"
#include "PicoHal.h"
#include <RadioLib.h>
#include <usr_lib/i2c_lcd.h>
#include "usr_lib/sht40/sht4x.h"
#include "usr_lib/sht40/sensirion_i2c_hal.h"

#include "global_def.h"
#include "main.h"
#include "slv_rf_cmd_proc.h"

#include "onewire_library/onewire_library.h"    // onewire library functions
#include "ow_rom.h"             // onewire ROM command codes
#include "ds18b20.h"            // ds18b20 function codes
//#define _LCD_DEBUG_SENSOR
// 分壓電阻配置 (單位: MΩ)
#define VOLTAGE_R1 1.5  // 下電阻 (連接 GND)
#define VOLTAGE_R2 1.0  // 上電阻 (連接外部電壓)

// 電壓校正係數 (根據實際測量調整)
// 計算方法: 校正係數 = 實際電壓 / 顯示電壓
// 例如: 實際 3.5V 顯示為 3.37V，則 校正係數 = 3.5 / 3.37 = 1.0386
#define VOLTAGE_CALIBRATION 1.0386  // 預設校正係數
//5.713649 /4096
//1.429 /1024
#define _EN_ADC
#define _EN_DS
uint8_t rf_state = 0;
//ds
OW ow;
// 全域變數宣告
PicoHal* hal = nullptr;
Module* module = nullptr;
SX1262* radio = nullptr;

// 中斷標誌
volatile bool receivedFlag = false;

//
TimeData Ary_currentTime = {0, 0, 0};
TimeData Ary_receiveTime = {0, 0, 0};
SensorData Ary_sensor;

//
uint8_t sub_min_div10(unsigned char );
// 中斷服務程式
void irqRadio(uint gpio, uint32_t events) {
    // 檢查是否為 DIO1 引腳中斷
    if (gpio == RADIO_DIO1_PIN) {
        if(rf_state == 0)
        {
receivedFlag = true;
        }
        else
        {
            rf_state = 0;
        }
        
    }
}
//
void sub_poll_ds(unsigned char *dsh,unsigned char *dsl)
{
  
    ow_reset (&ow);
    ow_send (&ow, OW_SKIP_ROM);
    ow_send (&ow, DS18B20_CONVERT_T);
    int tmro=0;
    
    while((tmro < 100)&&(ow_read(&ow) == 0)){
      sleep_ms(10);tmro++;
      }
       
      if(tmro < 100)
      {
    ow_reset (&ow);
    ow_send (&ow, OW_SKIP_ROM);
    ow_send (&ow, DS18B20_READ_SCRATCHPAD);
    
    *(dsl+0) = ow_read(&ow) ;//low
    *(dsh+0) = ow_read(&ow);
      }
      else{
        *(dsl+0) = 0x55;
        *(dsh+0) = 0xaa;
      }  
}
int64_t alarm_callback(alarm_id_t id, void *user_data) {
    // Put your timeout handler code in here
    return 0;
}

bool repeating_timer_callback(__unused struct repeating_timer *t) {
//    printf("Repeat at %lld\n", time_us_64());

    if(gPARAM_fcnt_100ms >= 9)
    {
      gPARAM_fcnt_100ms=0;
      
      if(gPARAM_fcnt_1s >= 59)
      {gPARAM_fcnt_1s = 0;
         //sub_updtime(0x8,0,0,0,)
         if(Ary_currentTime.min >=59)
         {  
            Ary_currentTime.min = 0;
            if(Ary_currentTime.hour >=23)
            {Ary_currentTime.hour = 0;}
            else
            {Ary_currentTime.hour++;}
         }
         else
         {  
           Ary_currentTime.min ++;
         }
      }
      else
      {gPARAM_fcnt_1s++;}
      
      //if(gPARAM_fcnt_1s == 10)
      {
        gPARAM_HOUR = Ary_currentTime.hour;
        gPARAM_MIN10 = sub_min_div10(Ary_currentTime.min);
        gPARAM_actflag |= 0x80;//relay
      }
    Ary_currentTime.sec = gPARAM_fcnt_1s;
    
      gPARAM_actflag |= 0x1;  
      if(gPARAM_fcnt_1s % 10 == 1)
      {
        gPARAM_actflag |= 0x10;
      }
      else if(gPARAM_fcnt_1s % 30 == 6)
      {
        gPARAM_actflag |= 0x20;
      }
    }  
    else
    {gPARAM_fcnt_100ms++;}

    return true;
}

void param_init(void)
{
    int i;
    gPARAM_fcnt_100ms = 0;
    gPARAM_fcnt_1s = 0;
    gPARAM_actflag = 0;
    for(i=0;i<32;i++)
    {
      gPARAM_UR1TX_buf[i]=0;
      gPARAM_UR1RX_buf[i]=0;
      gPARAM_RFTX_buf[i]=0;
      gPARAM_RFRX_buf[i]=0;
    }
    
    for(i=0;i<6;i++)
    {
      gSCH_RLY[i] = 0xff;gSCH_RLY[i+24] = 0xff;gSCH_RLY[i+48] = 0xff;gSCH_RLY[i+72] = 0xff;
    }    
    for(i=6;i<17;i++)
    {
      gSCH_RLY[i] = 0;gSCH_RLY[i+24] = 0;gSCH_RLY[i+48] = 0;gSCH_RLY[i+72] = 0;
    }    
    for(i=17;i<23;i++)
    {
      gSCH_RLY[i] = 0xff;gSCH_RLY[i+24] = 0xff;gSCH_RLY[i+48] = 0xff;gSCH_RLY[i+72] = 0xff;
    }    

}

void system_init(void)
{
    // Add your system initialisation code here

}
uint8_t sub_min_div10(unsigned char min_val)
{
       if(min_val>=50)  {return 5;}
  else if(min_val>=40)  {return 4;}
  else if(min_val>=30)  {return 3;}
  else if(min_val>=20)  {return 2;}
  else if(min_val>=10)  {return 1;}
  else                  {return 0;}

}

uint8_t sub_updtime(uint8_t mode,uint8_t hour,uint8_t min,uint8_t sec,TimeData * target_time)
{
  uint8_t mode_temp;
  uint8_t ret=0;
  
  mode_temp = mode;
  if((mode_temp & 0x4)==0x4)
  {
    mode_temp &= ~0x4;
    target_time->hour = hour;
  }
  if((mode_temp & 0x2)==0x2)
  {
    mode_temp &= ~0x2;
    target_time->min = min;
  }
  if((mode_temp & 0x1)==0x1)
  {
    mode_temp &= ~0x1;
    target_time->sec = sec;
  }
  if((mode_temp & 0x8)==0x8)
  {
    mode_temp &= ~0x8;
    if(target_time->sec == 59)
    {
      target_time->sec =0 ;
      ret !=  0x1;
      if(target_time->min == 59)
      {
        target_time->min = 0;
        ret |=  0x2;
        if(target_time->hour == 23)
      {
        target_time->hour = 0;
        ret |=  0x4;
      }
      else
      {target_time->min++;}
      }
      else
      {target_time->min++;}
    }
    else
    {
      target_time->sec++;
    }
  }
  
return ret;
}
int main()
{

    //add_alarm_in_ms(2000, alarm_callback, NULL, false);
    struct repeating_timer timer;
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    int state;
    uint8_t ret=0;
    uint8_t u8temp1;
    unsigned char ds_temp_h,ds_temp_l;
    
    RspData rsp_data;
    
    stdio_init_all();

//ds18b20

    PIO pio = pio0;
    uint gpio_ds =16;

    uint offset;
#ifdef _EN_DS    
    offset = pio_add_program(pio,&onewire_program);
    
   
    ow_init(&ow,pio,offset,gpio_ds);
    
    sub_poll_ds(&ds_temp_h,&ds_temp_l);
//    ow_reset (&ow);
//    ow_send (&ow, OW_SKIP_ROM);
//    ow_send (&ow, DS18B20_CONVERT_T);
//    int tmro=0;
//    
//    while((tmro < 50)&&(ow_read(&ow) == 0)){
//      sleep_ms(10);tmro++;
//      }
//       
//      if(tmro < 50)
//      {
//    ow_reset (&ow);
//    ow_send (&ow, OW_SKIP_ROM);
//    ow_send (&ow, DS18B20_READ_SCRATCHPAD);
//    
//    ds_temp_l = ow_read(&ow) ;
//    ds_temp_h = ow_read(&ow);
//      }
//      else{
//        ds_temp_h = 0x55;
//        ds_temp_l = 0xaa;
//      }
#else
        ds_temp_h = 0xde;
        ds_temp_l = 0xad;
#endif
    //K0
    gpio_init(GPIO_K0_PIN);
    gpio_set_dir(GPIO_K0_PIN, GPIO_OUT);
    gpio_put(GPIO_K0_PIN, 0);
    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    
    // 設定 GPIO 中斷 for radio
    gpio_set_irq_enabled_with_callback(RADIO_DIO1_PIN, GPIO_IRQ_EDGE_RISE, true, &irqRadio);
    
    // Create a new instance of the Pico HAL
    // SPI0, MISO 16, MOSI 19, SCK 18
    hal = new PicoHal(spi1, PIN_MISO, PIN_MOSI, PIN_SCK, 1000 * 1000);
    // Create a new instance of the radio module
    // NSS 17, DIO1 15, RST 21, BUSY 6
    module = new Module(hal, RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN,RADIO_BUSY_PIN);
    // Create a new instance of the SX1262 radio
    radio = new SX1262(module);
    state = radio->begin(915.0,125.0,7,5,0x12,20);
    if (state != RADIOLIB_ERR_NONE) {
        //printf("Failed to initialize radio, code %d\n", state);
        while (true);
    }   
    state = radio->startReceive();    
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c
    sensirion_i2c_hal_init();
    // Initialize the SHT40 sensor
    if (sht4x_init(SHT4X_I2C_ADDR_44) != 0) {
        printf("SHT40 sensor initialization failed\n");
        while (1);
    }

    // Probe the sensor to see if it's connected
    if (sht4x_probe() != 0) {
        printf("SHT40 sensor not found\n");
        while (1);
    }    

    // Timer example code - This example fires off the callback after 2000ms
    //add_alarm_in_ms(2000, alarm_callback, NULL, false);
    
    add_repeating_timer_ms(100, repeating_timer_callback, NULL, &timer);
    // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    
    // Send out a string, with CR/LF conversions
    uart_puts(UART_ID, " Hello, UART!\n");
    
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    adc_init();
    adc_gpio_init(26);  // ADC0 = GPIO 26
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    //param initial
    param_init();
    
    lcd_init();
    lcd_set_cursor(0,8);
    lcd_string("s001");
    lcd_hex(PARAM_SID);
    lcd_set_cursor(1,6);
    lcd_hex(ds_temp_h);
    lcd_hex(ds_temp_l);
    
    while (true) {
      if (receivedFlag) {
            // 關閉中斷，避免處理過程中被打斷
            gpio_set_irq_enabled(RADIO_DIO1_PIN, GPIO_IRQ_EDGE_RISE, false);
            
            // 清除標誌
            receivedFlag = false;
            state = radio->readData(&gPARAM_RFRX_buf[0], 12);
            //lcd_set_cursor(1,8);
            //     lcd_hex(gPARAM_RFRX_buf[0]);
             //    lcd_hex(gPARAM_RFRX_buf[1]);
            if (state == RADIOLIB_ERR_NONE) {
                 //process rx data
                 ret = slv_rf_cmd_proc(&gPARAM_RFRX_buf[0],&gPARAM_RFTX_buf[0],&gPARAM_RFTX_leng);
            lcd_set_cursor(1,0);
            lcd_hex(ret);
                 if(ret >0)
                 {
                   gPARAM_actflag |= 0x40;
                 }
                 #if 0
                 if(gPARAM_RFRX_buf[0] == 0x55)
                 {
                   //set fcnt
                   gPARAM_fcnt_1s = gPARAM_RFRX_buf[7];
                 } 
                 else if(gPARAM_RFRX_buf[0] == 0xa9) 
                    {
                        //send rsp
                        gPARAM_actflag |= 0x40;
                    }      
                 #endif                     
                 
                //printf("Frequency Error: %.2f Hz\n", radio.getFrequencyError());
                //printf("Packet Length: %d bytes\n", radio.getPacketLength());
            } else {
                //printf("Failed to read packet, code %d\n", state);
            }

            state = radio->startReceive();            
            // 重新啟用中斷
            gpio_set_irq_enabled(RADIO_DIO1_PIN, GPIO_IRQ_EDGE_RISE, true);
      }
      
      if((gPARAM_actflag & 0x1) == 0x1)
      {
        gPARAM_actflag &= 0xfffffffe;
        //toggle led
        if((gPARAM_fcnt_1s & 0x1) == 0x1){ gpio_put(LED_PIN, 1); }
        else {gpio_put(LED_PIN, 0);}
        //
        lcd_set_cursor(0,0);
        lcd_hex((uint8_t)(Ary_currentTime.hour&0xff));
        lcd_hex((uint8_t)(Ary_currentTime.min&0xff));
        lcd_hex((uint8_t)(gPARAM_fcnt_1s&0xff));
        
      }
      else if((gPARAM_actflag & 0x2) == 0x2)
      {gPARAM_actflag &= ~0x02;}
      else if((gPARAM_actflag & 0x4) == 0x4)
      {
        gPARAM_actflag &= ~0x04;
        gPARAM_fcnt_1s = Ary_receiveTime.sec;
        Ary_currentTime.sec = Ary_receiveTime.sec;
        Ary_currentTime.min = Ary_receiveTime.min;
        Ary_currentTime.hour = Ary_receiveTime.hour;
      }
      else if((gPARAM_actflag & 0x8) == 0x8)
      {
        gPARAM_actflag &= ~0x08;
        //relay or io action
        if((gVAR_RLY & 0x1) == 0x1)
        {gpio_put(GPIO_K0_PIN, 1);}
        else if((gVAR_RLY & 0x1) == 0x0)
        {gpio_put(GPIO_K0_PIN, 0);}
      }
      else if((gPARAM_actflag & 0x20) == 0x20)
      {
        gPARAM_actflag &= ~0x20;
        //measure sht40
        // Measure temperature and humidity
        sht_error = sht4x_measure_blocking_read_raw(&temp_raw, &humi_raw);
        if (sht_error == 0) {
            // Successfully read data
            #ifdef _LCD_DEBUG_SENSOR
            lcd_set_cursor(1,4);
            lcd_hex((uint8_t)((temp_raw>>8)&0xff));
            lcd_hex((uint8_t)((temp_raw)&0xff));
            lcd_hex((uint8_t)((humi_raw>>8)&0xff));
            lcd_hex((uint8_t)((humi_raw)&0xff));
            #endif
            Ary_sensor.temp_h = ((temp_raw>>8)&0xff);
            Ary_sensor.temp_l = ((temp_raw>>0)&0xff);
            Ary_sensor.humi_h = ((humi_raw>>8)&0xff);
            Ary_sensor.humi_l = ((humi_raw>>0)&0xff);
        } else {
            // Error reading data
            lcd_set_cursor(1,8);
            lcd_string("Err");
        }
        #ifdef _EN_ADC
        // 讀取 ADC0 電壓
        adc_select_input(0);  // 選擇 ADC0
        uint16_t adc_raw = adc_read();
        Ary_sensor.adc0_h = ((adc_raw>>8)&0xff);
        Ary_sensor.adc0_l = ((adc_raw>>0)&0xff);
        //float adc_voltage = adc_raw * 3.3f / 4095.0f;
        
        // 分壓計算: 未校正電壓 = ADC電壓 × (R1+R2)/R1
        //float raw_voltage = adc_voltage * ((VOLTAGE_R1 + VOLTAGE_R2) / VOLTAGE_R1);
        
        // 校正後的電壓
        //float actual_voltage = raw_voltage * VOLTAGE_CALIBRATION;
        #ifdef _LCD_DEBUG_SENSOR
            lcd_hex((uint8_t)((adc_raw>>8)&0xff));
            lcd_hex((uint8_t)((adc_raw)&0xff));
        #endif
        #endif
        #ifdef _EN_DS    
          sub_poll_ds(&ds_temp_h,&ds_temp_l);
          Ary_sensor.ds0_h = ds_temp_h;
          Ary_sensor.ds0_l = ds_temp_l;
        #else
          Ary_sensor.ds0_h = 0xde;
          Ary_sensor.ds0_l = 0xad;
        #endif
      }
      else if((gPARAM_actflag & 0x40) == 0x40)
      {
        gPARAM_actflag &= ~0x40;
        //compose rsp
        #if 0
        rsp_data.hdr = 0x9a;
        rsp_data.length = 0x10;
        rsp_data.mid = 0x01;
        rsp_data.sid = 0x81;
        rsp_data.sec = gPARAM_fcnt_1s;
        rsp_data.data5 = Ary_sensor.temp_h;
        rsp_data.data6 = Ary_sensor.temp_l;
        rsp_data.data7 = 0;
        rf_state = 1;
          state = radio->transmit((uint8_t*)&rsp_data, sizeof(RspData));
        #endif
        rf_state = 1;
          state = radio->transmit(&gPARAM_RFTX_buf[0], gPARAM_RFTX_leng);
          
          if (state == RADIOLIB_ERR_NONE) {
              printf("success!\n");
          } else {
              printf("failed, code %d\n", state);
              rf_state = 0;
          }          
   lcd_set_cursor(1,8);
        lcd_hex((uint8_t)(gPARAM_fcnt_1s&0xff));
          state = radio->startReceive(); 
        
      }
      else if((gPARAM_actflag & 0x80) == 0x80)      
      {
        gPARAM_actflag &= ~0x80;
        
        u8temp1 = gSCH_RLY[gPARAM_HOUR]>>gPARAM_MIN10;
        #if 1
         lcd_set_cursor(1,10);
            lcd_hex((uint8_t)((gPARAM_HOUR)&0xff));
            lcd_hex((uint8_t)((gPARAM_MIN10)&0xff));
            lcd_hex((uint8_t)((u8temp1)&0xff));
            #endif
        if((u8temp1 & 0x1) == 0x1)
          {gpio_put(GPIO_K0_PIN, 1);}
        else
          {gpio_put(GPIO_K0_PIN, 0);}
      }
      //  printf("Hello, world!\n");
        sleep_ms(1);
    }
}

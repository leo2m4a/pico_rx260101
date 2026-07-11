//parameter
uint32_t gPARAM_fcnt_100ms;
uint32_t gPARAM_fcnt_1s;
uint32_t gPARAM_actflag;

uint32_t gVAR_btn_u=0;
uint32_t gVAR_btn_d=0;

uint8_t gVAR_RLY = 0;
uint8_t gVAR_IO = 0;
uint8_t gVAR_ADC0 = 0;

uint8_t gCTRL_SCH_EN = 0;

//volatile bool timer_fired = false;
uint8_t gPARAM_UR1RX_buf[32];
uint8_t gPARAM_UR1RX_buf_lat[32];
uint8_t gPARAM_UR1RX_buf_cnt;
uint8_t gPARAM_UR1RX_buf_leng;
uint8_t gPARAM_UR1RX_buf_ptr;

uint8_t gPARAM_UR1TX_buf[32];

//
int16_t sht_error;

int32_t temp_raw;
uint32_t humi_raw;

//radio
uint8_t gPARAM_RFTX_buf[32];
uint8_t gPARAM_RFRX_buf[32];

uint8_t gPARAM_RFTX_leng;

//
uint8_t gPARAM_MIN10;
uint8_t gPARAM_HOUR;
uint8_t gSCH_RLY[24*4];
//uint8_t gSCH_RLY0[24];
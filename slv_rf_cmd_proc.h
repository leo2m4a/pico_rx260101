#ifndef _SLV_RF_CMD_PROC_
#define _SLV_RF_CMD_PROC_

uint8_t slv_rf_cmd_proc(uint8_t *,uint8_t *,uint8_t *);
uint8_t slv_compose_poll_rsp(uint8_t *);
uint8_t slv_rf_action_wr_rly(uint8_t ,uint8_t );
uint8_t slv_compose_rf_write_a4_rsp(uint8_t *);

extern uint8_t gVAR_RLY;
extern uint8_t gVAR_IO;
extern uint8_t gVAR_ADC0;


extern uint32_t gPARAM_actflag;
extern uint32_t gPARAM_fcnt_1s;

extern SensorData Ary_sensor;
extern TimeData Ary_receiveTime;
#endif
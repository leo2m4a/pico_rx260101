#include <stdio.h>
#include "pico/stdlib.h"
#include "global_def.h"
#include "slv_rf_cmd_proc.h"

extern uint8_t gSCH_RLY[24*4];

uint8_t slv_rf_cmd_proc(uint8_t *inbuf,uint8_t *outbuf,uint8_t *leng)
{
uint8_t ret=0;//rf and other action
uint8_t diff;
int i;
uint8_t val_6,val_7,val_8,val_9;

uint8_t cmd = *(inbuf+0);
uint8_t sid = *(inbuf+3);
uint8_t subcmd = *(inbuf+4);
uint8_t addr = *(inbuf+5);
val_6 = *(inbuf+6);
val_7 = *(inbuf+7);
val_8 = *(inbuf+8);
val_9 = *(inbuf+9);

  if(cmd == CMD_ECHO)
  {
    if(subcmd == SC_BCN)//beacon
    {
      //adjust clock
      Ary_receiveTime.sec = *(inbuf+5);
      Ary_receiveTime.min = *(inbuf+6);
      Ary_receiveTime.hour = *(inbuf+7);
      gPARAM_actflag |= 0x04;
      ret=0;
    }
    else if(subcmd == SC_PING)
    {
      if(sid == PARAM_SID)
      {
        //poll request
        //compose poll response
        *(leng+0) = slv_compose_poll_rsp(outbuf);
        ret |= 0x1;
      }
    }
  }
  else if(cmd == 0x78)
  {
    if(sid == PARAM_SID)
    {
    if(subcmd == SC_READ)//read
    {
      ret = 0;
    }
    else if(subcmd == SC_WRITE)//write
    {
      //write action
      if(addr == 0)
      {
        if((val_6 == VAL_SCH_EN)&& (val_8 == VAL_PWD0) && (val_9 == VAL_PWD1))
        {
          //gCTRL_SCH_EN
        }
        else if((val_6 == VAL_SCH_DIS)&& (val_8 == VAL_PWD0) && (val_9 == VAL_PWD1))
        {
          //gCTRL_SCH_EN
        }
      }
      else if(addr == 4)
      {
        diff = slv_rf_action_wr_rly( *(inbuf+6), *(inbuf+7));
        //need update relay
        if(diff == 1)
        {gPARAM_actflag |= 0x8;}
      
        *(leng+0) = slv_compose_rf_write_a4_rsp(outbuf);
        
        ret |= 0x1;
      }
      else if (addr = 0x10)
      {
        //make sure data length
        for(i=0;i<=7;i++)
        {gSCH_RLY[i] = *(inbuf+6+i);}
      }
      else if (addr = 0x11)
      {
        //make sure data length
        for(i=0;i<=7;i++)
        {gSCH_RLY[i+8] = *(inbuf+6+8+i);}
      }
      else if (addr = 0x12)
      {
        //make sure data length
        for(i=0;i<=7;i++)
        {gSCH_RLY[i+16] = *(inbuf+6+16+i);}
      }

    }
    }
    
  }
  return ret;
}

uint8_t slv_compose_poll_rsp(uint8_t *rspbuf)
{
 int i;
 uint8_t leng = 0x10;
  uint8_t ret=0x14;
  
  *(rspbuf+0) = RSP_ECHO;
  *(rspbuf+1) = leng;
  *(rspbuf+2) = 0;
  *(rspbuf+3) = PARAM_SID;
  *(rspbuf+4) = SC_PING;
  *(rspbuf+5) = 0;
  *(rspbuf+6) = Ary_sensor.temp_h;
  *(rspbuf+7) = Ary_sensor.temp_l;
  *(rspbuf+8) = gVAR_RLY;
  *(rspbuf+9) = gVAR_IO;
  *(rspbuf+10) = gVAR_ADC0;
  *(rspbuf+11) = Ary_sensor.humi_h;
  *(rspbuf+12) = Ary_sensor.humi_l;
  *(rspbuf+13) = Ary_sensor.adc0_h;
  *(rspbuf+14) = Ary_sensor.adc0_l;  
  *(rspbuf+15) = Ary_sensor.ds0_h;
  *(rspbuf+16) = Ary_sensor.ds0_l;
  *(rspbuf+17) = 0xaa;
  *(rspbuf+18) = 0xaa;
  *(rspbuf+19) = 0xaa;
  //for(i=13;i<16;i++)
  //{*(rspbuf+i) = 0;}
  
  return ret;
}

uint8_t slv_rf_action_wr_rly(uint8_t wen,uint8_t wval)
{
  uint8_t uctemp1,uctemp2,uctemp3;
  uint8_t ret=1;
  
  uctemp3 = gVAR_RLY;
  uctemp1 = gVAR_RLY & (~wen);
  uctemp2 = wen & wval;
  gVAR_RLY = uctemp1 | uctemp2;
  if(gVAR_RLY == uctemp3){ret=0;}
  return ret;
}

uint8_t slv_compose_rf_write_a4_rsp(uint8_t *rspbuf)
{
 int i;
 uint8_t leng = 0x10;
  uint8_t ret=0x10;
  
  *(rspbuf+0) = RSP_SLVWR;
  *(rspbuf+1) = leng;
  *(rspbuf+2) = 0;
  *(rspbuf+3) = PARAM_SID;
  *(rspbuf+4) = 0;
  *(rspbuf+5) = 0x4;
  *(rspbuf+6) = Ary_sensor.temp_h;
  *(rspbuf+7) = Ary_sensor.temp_l;
  *(rspbuf+8) = gVAR_RLY;
  *(rspbuf+9) = gVAR_IO;
  *(rspbuf+10) = gVAR_ADC0;
  *(rspbuf+11) = Ary_sensor.humi_h;
  *(rspbuf+12) = Ary_sensor.humi_l;
  *(rspbuf+13) = Ary_sensor.adc0_h;
  *(rspbuf+14) = Ary_sensor.adc0_l;  
  *(rspbuf+15) = 4;
//  for(i=11;i<16;i++)
//  {*(rspbuf+i) = 0;}
  
  return ret;  
}
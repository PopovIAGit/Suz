#ifndef EEPROM_H
#define EEPROM_H

#include "lwip.h"
#include <stdbool.h>

extern osMutexId memmutex_id;

typedef struct{
 struct{
   ip4_addr_t dev_ip;
   ip4_addr_t gw_ip;     
   ip4_addr_t netmask;
   uint64_t mac;
  }eth;   
 struct{
   uint16_t R_offset_GAIN_1;
   uint16_t R_offset_GAIN_2;
   uint16_t R_offset_GAIN_5;
   uint16_t R_offset_GAIN_10;
   uint16_t R_offset_GAIN_20;
   uint16_t R_offset_GAIN_50;
   uint16_t R_offset_GAIN_100;   
  }calibrate;
 struct{
   uint16_t Icoil;
   uint16_t Fcoil;
   uint16_t Irs;
  }testparam;
}devparam_t;

void CreateEEPROMSemaphore();
void ModuleSetParam(devparam_t *data);
void ModuleGetParam(devparam_t *data);

#endif
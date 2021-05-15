#include "stdint.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "string.h"
#include "stdbool.h"
#include <math.h>
#include "eeprom.h"

extern I2C_HandleTypeDef hi2c4;

osSemaphoreId semaphoreId;                         // Semaphore ID
osSemaphoreDef(semaphore);                       // Semaphore definition

void CreateEEPROMSemaphore(){
 semaphoreId = osSemaphoreCreate(osSemaphore(semaphore), 1);
}

void ModuleSetParam(devparam_t *data){
 uint8_t byte, idx; 
  
 osSemaphoreWait(semaphoreId, osWaitForever);
 for(idx = 0; idx < sizeof(devparam_t); idx++){
   byte = ((uint8_t*)data)[idx];
   HAL_I2C_Mem_Write(&hi2c4, 0xa0, idx, 2, &byte, 1, 1000);
   osDelay(15);  
  } 
 osSemaphoreRelease(semaphoreId);
}

void ModuleGetParam(devparam_t *data){ 
 osSemaphoreWait(semaphoreId, osWaitForever);
 HAL_I2C_Mem_Read(&hi2c4, 0xa0, 0x00, 2, (uint8_t*)data, sizeof(devparam_t), 2000);
 osSemaphoreRelease(semaphoreId);
}


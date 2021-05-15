#include "stdint.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "string.h"
#include "stdbool.h"
#include "stm32f7xx_hal_adc.h"
#include <math.h>
#include "analog.h"
#include "main.h"
#include <task.h>

extern DMA_HandleTypeDef hdma_adc1;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern TIM_HandleTypeDef htim1;
extern SPI_HandleTypeDef hspi1;
extern DAC_HandleTypeDef hdac;
extern I2C_HandleTypeDef hi2c1;

#define HBUFSIZE 400

#pragma location = "VARIABLE_MASS"
static uint32_t adc_data_array[HBUFSIZE * 4]; /* dual mode adc 2&1  1@3*/
static uint32_t adcconvrdy_half = 0, adcconvrdy_full = 0; 

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc){
 adcconvrdy_half = 1; 
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
 adcconvrdy_full = 1; 
}

static uint32_t SHUNT_R = 10;
static volatile uint32_t gain = GAIN_10;
static uint32_t rs_current, coil_current;

void ConnectRload(uint32_t Rload){
 HAL_GPIO_WritePin(conn_10R_GPIO_Port, conn_10R_Pin, GPIO_PIN_RESET);    
 HAL_GPIO_WritePin(conn_5R_GPIO_Port, conn_5R_Pin, GPIO_PIN_RESET);    
 HAL_GPIO_WritePin(conn_2400R_GPIO_Port, conn_2400R_Pin, GPIO_PIN_RESET);    
  
 if(Rload == Rload_10R){
   HAL_GPIO_WritePin(conn_10R_GPIO_Port, conn_10R_Pin, GPIO_PIN_SET);       
   SHUNT_R = 10;
  } 
 if(Rload == Rload_5R){
   HAL_GPIO_WritePin(conn_5R_GPIO_Port, conn_5R_Pin, GPIO_PIN_SET);       
   SHUNT_R = 5;
  } 
 if(Rload == Rload_2400R){
   HAL_GPIO_WritePin(conn_2400R_GPIO_Port, conn_2400R_Pin, GPIO_PIN_SET);       
   SHUNT_R = 2400;
  }  
}

static uint32_t TEST_VLT;

void SetRSCurrent(uint32_t curr){
  
 switch(curr){
    case 1 : {
      SetTestVolatge(2500);
      TEST_VLT = 2500;
      ConnectRload(Rload_2400R);
      rs_current = 1;
      break;
     }
    case 50 : {
      SetTestVolatge(500);
      TEST_VLT = 500;
      ConnectRload(Rload_10R);
      rs_current = 50;
      break;
     }
    case 500 : {
      SetTestVolatge(2500);
      TEST_VLT = 2500;
      ConnectRload(Rload_5R);
      rs_current = 500;
      break;
     }
   }
}

uint32_t GetRSCurrent(){
 return  rs_current;
}

void EnableCoilPower(){
 HAL_GPIO_WritePin(EN_COILPWR_GPIO_Port, EN_COILPWR_Pin, GPIO_PIN_SET); 
}

void DisableCoilPower(){
 HAL_GPIO_WritePin(EN_COILPWR_GPIO_Port, EN_COILPWR_Pin, GPIO_PIN_RESET); 
}

void EnableCoilDrivePulses(){
 HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_1);  
}

void DisableCoilDrivePulses(){
 HAL_TIM_PWM_Stop_IT(&htim1, TIM_CHANNEL_1); 
}

void SetCoilPWMFreq(uint16_t freq){ /*in Hz*/
 htim1.Instance->ARR =  40000 / freq;
}

uint32_t GetCoilPWMFreq(){
 if(htim1.Instance->ARR != 0) return (40000 / htim1.Instance->ARR);
 return 0; 
}

void SetCoilCurrent(uint16_t i){ /*in mkA*/
 uint64_t u;
 uint16_t n;
 uint8_t byte;
 
 const uint16_t R = 2;

 u = R * i * 30; /*mkV*/
 n = u * 65535 / 3000000; 

 HAL_Delay(1); 
 HAL_GPIO_WritePin(SPI_nCS_GPIO_Port, SPI_nCS_Pin, GPIO_PIN_RESET); 
 HAL_Delay(1);
 
 byte = 0x00;
 HAL_SPI_Transmit(&hspi1, &byte, 1, 1000);
 byte = n >> 8;
 HAL_SPI_Transmit(&hspi1, &byte, 1, 1000);
 byte = n;
 HAL_SPI_Transmit(&hspi1, &byte, 1, 1000);

 HAL_Delay(1);
 HAL_GPIO_WritePin(SPI_nCS_GPIO_Port, SPI_nCS_Pin, GPIO_PIN_SET); 
 HAL_Delay(1);
 coil_current = i;
}

uint32_t GetCoilCurrent(){
 return coil_current; 
}

void SetTestVolatge(uint16_t v){ /*in mV*/
 uint32_t n;
 const uint32_t Uref = 2500;
 n = v * 4096 / Uref;   
 HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, n); 
}

static uint16_t swap_bytes(uint16_t data){
 uint16_t res;
 
 res = (data & 0x00ff) << 8;
 res |= (data & 0xff00) >> 8;  
 return res;
}

uint16_t MeashureCoilCurrent(){/*in mkA*/
 #define CONFIGURATION_REG  0x00
 #define SHUNT_VLT_REG  0x01 
 #define BUS_VLT_REG  0x02 
 
 int16_t data;
 const uint16_t R = 1;
 
 if(HAL_I2C_Mem_Read(&hi2c1, 0x80, CONFIGURATION_REG, 1, (uint8_t*)&data, 2, 1000) != HAL_OK)
   asm("NOP");
 data = swap_bytes(data);
 if(data != 0x4527){
   data = swap_bytes(0x4527); 
   HAL_I2C_Mem_Write(&hi2c1, 0x80, CONFIGURATION_REG, 1, (uint8_t*)&data, 2, 1000);
  }
 if(HAL_I2C_Mem_Read(&hi2c1, 0x80, SHUNT_VLT_REG, 1, (uint8_t*)&data, 2, 1000) != HAL_OK)
   asm("NOP");
 data = abs(swap_bytes(data));

 data = data * 5 / 2; /*2.5*/ /*in mkv*/
 data = data / R; /*in mkv*/
 return data;
}

void SetSignalGain(uint8_t setgain){
  switch(setgain){
     case GAIN_0 : {
       HAL_GPIO_WritePin(GAIN_0_GPIO_Port, GAIN_0_Pin, GPIO_PIN_RESET); 
       HAL_GPIO_WritePin(GAIN_1_GPIO_Port, GAIN_1_Pin, GPIO_PIN_RESET); 
       HAL_GPIO_WritePin(GAIN_2_GPIO_Port, GAIN_2_Pin, GPIO_PIN_RESET);        
       gain = GAIN_0;
       break;
      }
     case GAIN_1 : {
       HAL_GPIO_WritePin(GAIN_0_GPIO_Port, GAIN_0_Pin, GPIO_PIN_SET); 
       HAL_GPIO_WritePin(GAIN_1_GPIO_Port, GAIN_1_Pin, GPIO_PIN_RESET); 
       HAL_GPIO_WritePin(GAIN_2_GPIO_Port, GAIN_2_Pin, GPIO_PIN_RESET);        
       gain = GAIN_1;
       break;
      }
     case GAIN_2 : {
       HAL_GPIO_WritePin(GAIN_0_GPIO_Port, GAIN_0_Pin, GPIO_PIN_RESET); 
       HAL_GPIO_WritePin(GAIN_1_GPIO_Port, GAIN_1_Pin, GPIO_PIN_SET); 
       HAL_GPIO_WritePin(GAIN_2_GPIO_Port, GAIN_2_Pin, GPIO_PIN_RESET);        
       gain = GAIN_2;
       break;
      }
     case GAIN_5 : {
       HAL_GPIO_WritePin(GAIN_0_GPIO_Port, GAIN_0_Pin, GPIO_PIN_SET); 
       HAL_GPIO_WritePin(GAIN_1_GPIO_Port, GAIN_1_Pin, GPIO_PIN_SET); 
       HAL_GPIO_WritePin(GAIN_2_GPIO_Port, GAIN_2_Pin, GPIO_PIN_RESET);        
       gain = GAIN_5;
       break;
      }
     case GAIN_10 : {
       HAL_GPIO_WritePin(GAIN_0_GPIO_Port, GAIN_0_Pin, GPIO_PIN_RESET); 
       HAL_GPIO_WritePin(GAIN_1_GPIO_Port, GAIN_1_Pin, GPIO_PIN_RESET); 
       HAL_GPIO_WritePin(GAIN_2_GPIO_Port, GAIN_2_Pin, GPIO_PIN_SET);        
       gain = GAIN_10;
       break;
      }
     case GAIN_20 : {
       HAL_GPIO_WritePin(GAIN_0_GPIO_Port, GAIN_0_Pin, GPIO_PIN_SET); 
       HAL_GPIO_WritePin(GAIN_1_GPIO_Port, GAIN_1_Pin, GPIO_PIN_RESET); 
       HAL_GPIO_WritePin(GAIN_2_GPIO_Port, GAIN_2_Pin, GPIO_PIN_SET);        
       gain = GAIN_20;
       break;
      }
     case GAIN_50 : {
       HAL_GPIO_WritePin(GAIN_0_GPIO_Port, GAIN_0_Pin, GPIO_PIN_RESET); 
       HAL_GPIO_WritePin(GAIN_1_GPIO_Port, GAIN_1_Pin, GPIO_PIN_SET); 
       HAL_GPIO_WritePin(GAIN_2_GPIO_Port, GAIN_2_Pin, GPIO_PIN_SET);        
       gain = GAIN_50;
       break;
      }
     case GAIN_100 : {
       HAL_GPIO_WritePin(GAIN_0_GPIO_Port, GAIN_0_Pin, GPIO_PIN_SET); 
       HAL_GPIO_WritePin(GAIN_1_GPIO_Port, GAIN_1_Pin, GPIO_PIN_SET); 
       HAL_GPIO_WritePin(GAIN_2_GPIO_Port, GAIN_2_Pin, GPIO_PIN_SET);        
       gain = GAIN_100;
       break;
      }
    }
}

#pragma location = "VARIABLE_MASS"
static uint16_t R_RS[HBUFSIZE] = {0};
static uint16_t U_HS_avg, U_MR_avg, R_RS_avg, U_CS_avg, U_RS_avg;

/*{ADC2, ACD1}*/

__ramfunc void AnalogMeashure(){
 static uint32_t U_HS_summa_shadow, U_MR_summa_shadow, idx;
 static uint32_t U_CS_summa_shadow = 0, U_RS_summa_shadow = 0, R_RS_summa_shadow = 0;
 static uint32_t I, U, idy;
 uint32_t *ptr;
 const uint32_t UREF = 3000;
 
 //HAL_ADCEx_Calibration_Start(&hadc2);
 //HAL_ADCEx_Calibration_Start(&hadc1);
 //osDelay(1000);
 
 HAL_ADC_Start(&hadc2);
 HAL_ADCEx_MultiModeStart_DMA(&hadc1,(uint32_t*)adc_data_array , HBUFSIZE * 4);
 HAL_DMA_Start_IT(&hdma_adc1, (uint32_t)(ADC1_BASE + 0x308), (uint32_t)adc_data_array, HBUFSIZE * 4); 

 bool data_rdy = false;

 for(;;){
   /*stream transfer half buffer is ready*/
   if(adcconvrdy_half){
     U_HS_summa_shadow = 0; U_MR_summa_shadow = 0; idy = 0;
     U_RS_summa_shadow = 0; U_CS_summa_shadow = 0;
     R_RS_summa_shadow = 0;
     ptr = &adc_data_array[0];
     for(idx = 0; idx < HBUFSIZE * 2; idx = idx + 8){
       U_HS_summa_shadow += ptr[idx] >> 16;
       U_HS_summa_shadow += ptr[idx + 2] >> 16;
       U_MR_summa_shadow += ptr[idx + 1] >> 16;
       U_MR_summa_shadow += ptr[idx + 3] >> 16;
       
       I = (ptr[idx + 1] & 0xffff) * UREF / 4096 * 1000 / SHUNT_R; /*mkA*/
       U = (ptr[idx] & 0xffff) * UREF / 4096 * 1000 / 694 * 100 / gain; /*mkV*/
       U_CS_summa_shadow += (ptr[idx + 1] & 0xffff) * UREF / 4096; /*mV*/
       U_RS_summa_shadow += (ptr[idx] & 0xffff) * UREF / 4096; /*mV*/
       if(I > 0) R_RS[idy++] = U * 1000 / I; /*mOmh*/
       else R_RS[idy++] = 1000;
       I = (ptr[idx + 3] & 0xffff) * UREF / 4096 * 1000 / SHUNT_R; /*mkA*/
       U = (ptr[idx + 2] & 0xffff) * UREF / 4096 * 1000 / 694 * 100 / gain ; /*mkV*/
       U_CS_summa_shadow += (ptr[idx + 3] & 0xffff) * UREF / 4096; /*mV*/
       U_RS_summa_shadow += (ptr[idx + 2] & 0xffff) * UREF / 4096; /*mV*/       
       if(I > 0) R_RS[idy++] = U * 1000 / I; /*mOmh*/
       else R_RS[idy++] = 1000;
      }
     adcconvrdy_half = 0;
     for(idx = 0; idx < HBUFSIZE / 2; idx++) R_RS_summa_shadow += R_RS[idx];
     data_rdy = true;
    }
   /*stream transfer full buffer is ready*/   
   if(adcconvrdy_full){ 
     ptr = &adc_data_array[HBUFSIZE];
     for(idx = 0; idx < HBUFSIZE * 2; idx = idx + 8){
       U_HS_summa_shadow += ptr[idx] >> 16;
       U_HS_summa_shadow += ptr[idx + 2] >> 16;
       U_MR_summa_shadow += ptr[idx + 1] >> 16;
       U_MR_summa_shadow += ptr[idx + 3] >> 16;
       
       I = (ptr[idx + 1] & 0xffff) * UREF / 4096 * 1000 / SHUNT_R; /*mkA*/
       U = (ptr[idx] & 0xffff) * UREF / 4096 * 1000 / 694 * 100 / gain; /*mkV*/
       U_CS_summa_shadow += (ptr[idx + 1] & 0xffff) * UREF / 4096;
       U_RS_summa_shadow += (ptr[idx] & 0xffff) * UREF / 4096; /*mV*/       
       if(I > 0) R_RS[idy++] = U * 1000 / I; /*mOmh*/
       else R_RS[idy++] = 1000;
       I = (ptr[idx + 3] & 0xffff) * UREF / 4096 * 1000 / SHUNT_R; /*mkA*/
       U = (ptr[idx + 2] & 0xffff) * UREF / 4096 * 1000 / 694 * 100 / gain ; /*mkV*/
       U_CS_summa_shadow += (ptr[idx + 3] & 0xffff) * UREF / 4096; /*mV*/
       U_RS_summa_shadow += (ptr[idx + 2] & 0xffff) * UREF / 4096; /*mV*/       
       if(I > 0) R_RS[idy++] = U * 1000 / I; /*mOmh*/
       else R_RS[idy++] = 1000;
      }     
     adcconvrdy_full = 0;
     for(idx = 0; idx < HBUFSIZE / 2; idx++) R_RS_summa_shadow += R_RS[idx];
     taskENTER_CRITICAL();
     R_RS_avg = R_RS_summa_shadow / HBUFSIZE; /*4 samples in one cycle*/  
     U_HS_avg = U_HS_summa_shadow / HBUFSIZE; /*4 samples in one cycle*/
     U_MR_avg = U_MR_summa_shadow / HBUFSIZE; /*4 samples in one cycle*/
     U_CS_avg = U_CS_summa_shadow / HBUFSIZE; /*4 samples in one cycle*/
     U_RS_avg = U_RS_summa_shadow / HBUFSIZE; /*4 samples in one cycle*/
     taskEXIT_CRITICAL();
     data_rdy = true;
    }
   if(data_rdy){
     data_rdy = false;
    }
   osThreadYield(); 
  }
}


uint16_t GetU_HS(){/*in mV*/
  uint32_t U_HS;
  const uint32_t UREF = 3000;
  
  U_HS = (uint32_t)U_HS_avg * UREF / 4096 * 2;
  return (uint16_t)U_HS;
}

uint16_t GetB_HS(){ /*in mT*/
  uint16_t U_HS, B;
  const uint16_t K = 25; /*mV/mT*/
  const uint32_t UREF = 3000;
  
  U_HS = (uint32_t)U_HS_avg * UREF / 4096 * 2;/*in mV*/
  if(U_HS > 600) B = (U_HS - 600) / K;
  else B = 0;
  return (uint16_t)B;
}

uint16_t U_MR(){/*in mV*/
  uint32_t U_MR;
  const uint32_t UREF = 3000;
  
  U_MR = (uint32_t)U_MR_avg * UREF / 4096;
  return (uint16_t)U_MR;
}

uint16_t GetR_ISO(){/*in MOmh*/
  uint32_t U_MR, R_ISO;
  const uint32_t R_izm = 1; /*in MOmh*/
  const uint32_t UREF = 3000;
  
  U_MR = (uint32_t)U_MR_avg * UREF / 4096;    
  if((U_MR < TEST_VLT) && (U_MR !=0)) R_ISO = (TEST_VLT - U_MR) * R_izm / U_MR; /*in MOmh*/
  else R_ISO = 100;
  if(R_ISO > 32000) R_ISO = 100;
  return (uint16_t)R_ISO;
}

uint16_t GetRS_ON(uint16_t offset){/*in mOmh*/
  
  if(R_RS_avg < offset) return 0;
  else return (R_RS_avg - offset);
}

uint16_t GetRS_OFF(){/*in Omh*/
  uint32_t R_OFF;
  
  if((U_CS_avg < 500) && (U_CS_avg != 0)) R_OFF = (500 - U_CS_avg) * SHUNT_R / U_CS_avg; 
  else R_OFF = 50;
  if(R_OFF > 50) R_OFF = 50;
  return (uint16_t)R_OFF;
}

uint16_t GetURS(){/*in mV*/

  return U_RS_avg;
}

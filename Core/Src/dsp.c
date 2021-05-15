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
#include "dsp.h"
#include "eeprom.h"


extern TIM_HandleTypeDef htim1;

enum{SIGNEMODE, CYCLEMODE};

static struct{
 uint8_t mode;
 bool activate_rstest;
 bool float_test;
 bool RISO_test;
 bool calibrateR;
 bool led_blink;
 test_result_T test_result;
 struct{
   double sqr_summa;
   uint32_t cnt;
   double avg;
  }dispertion;
 bool continuously_on;
}dev_cntrl;

static test_result_T test_result_shadow;
static uint8_t current_gain = GAIN_1;
static devparam_t confparam;

__ramfunc void TIM1_UP_TIM10_IRQHandler(void){
  
 if(HAL_GPIO_ReadPin(PWM_GPIO_Port, PWM_Pin) == GPIO_PIN_SET){
 
   switch(current_gain){
     case GAIN_1 : {
       test_result_shadow.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_1);
       break;
      }
     case GAIN_2 : {
       test_result_shadow.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_2);
       break;
      }
     case GAIN_5 : {
       test_result_shadow.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_5);
       break;
      }
     case GAIN_10 : {
       test_result_shadow.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_10);
       break;
      }
     case GAIN_20 : {
       test_result_shadow.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_20);
       break;
      }
     case GAIN_50 : {
       test_result_shadow.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_50);
       break;
      }
     case GAIN_100 : {
       test_result_shadow.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_100);
       break;
      }             
    }
   dev_cntrl.dispertion.avg += dev_cntrl.test_result.RS_R_on;
   if(dev_cntrl.test_result.RS_R_on < 200) dev_cntrl.test_result.good_cnt++; 
   dev_cntrl.dispertion.cnt++;
   dev_cntrl.dispertion.sqr_summa = pow(dev_cntrl.test_result.RS_R_on - dev_cntrl.dispertion.avg / dev_cntrl.dispertion.cnt, 2);
   test_result_shadow.dispertion = sqrt(dev_cntrl.dispertion.sqr_summa / dev_cntrl.dispertion.cnt);
   HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_RESET);    
  }
 else{
   test_result_shadow.RS_R_off = GetRS_OFF();
   test_result_shadow.cycles_cnt++;           
   HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_SET);
  }
  
 HAL_TIM_IRQHandler(&htim1);
}

void Algorithm(){
  static float button_rs = 3.3, button_hs = 3.3;
  static bool buttrs_state = false, butths_state = false;
  uint32_t timer = osKernelSysTick();
  enum{WAIT, CHOOSING_GAIN, TEST, CONTINUOUSLY_ON} fsm = WAIT;
  const uint32_t UREF = 3000;
    
  SetSignalGain(GAIN_1);
  osDelay(100);
  dev_cntrl.activate_rstest = false;
  dev_cntrl.float_test = false;
  dev_cntrl.RISO_test = false;
  dev_cntrl.test_result.RS_R_off = 100;
  dev_cntrl.test_result.RS_R_on = 10;
  dev_cntrl.led_blink = false;
  dev_cntrl.calibrateR = false;
  dev_cntrl.continuously_on = false;

  DisableCoilPower();
  ModuleGetParam(&confparam);   

  SetCoilCurrent(confparam.testparam.Icoil);
  SetRSCurrent(confparam.testparam.Irs);
  SetCoilPWMFreq(confparam.testparam.Fcoil);

  for(;;){
    if(HAL_GPIO_ReadPin(RS_TEST_GPIO_Port, RS_TEST_Pin) == GPIO_PIN_SET) button_rs = button_rs+ (3.3 - button_rs) / 10.0; 
    else button_rs = button_rs + (0 - button_rs) / 10.0;  
    if(button_rs > 2.8) buttrs_state = false;
    else if(button_rs < 0.4) buttrs_state = true;    
    
    if(HAL_GPIO_ReadPin(FLOAT_TEST_GPIO_Port, FLOAT_TEST_Pin) == GPIO_PIN_SET) button_hs = button_hs+ (3.3 - button_hs) / 10.0; 
    else button_hs = button_hs + (0 - button_hs) / 10.0;  
    if(button_hs > 2.8) butths_state = false;
    else if(button_hs < 0.4) butths_state = true;    
    
    if(buttrs_state) dev_cntrl.activate_rstest = true;
    if(butths_state) dev_cntrl.float_test = true;
    
    switch(fsm){
       case WAIT : {
         DisableCoilPower();         
         dev_cntrl.dispertion.sqr_summa = 0;
         dev_cntrl.dispertion.cnt = 0;
         dev_cntrl.dispertion.avg = 0;
         if(dev_cntrl.activate_rstest || dev_cntrl.continuously_on){
           EnableCoilPower();
           SetRSCurrent(confparam.testparam.Irs);
           osDelay(100);
           fsm = CHOOSING_GAIN;
           dev_cntrl.test_result.cycles_cnt = 0;
           dev_cntrl.test_result.good_cnt = 0;
           current_gain = GAIN_1;
          }
         if(dev_cntrl.calibrateR){
           dev_cntrl.calibrateR = false;
           ModuleGetParam(&confparam);
           SetRSCurrent(50);
           SetSignalGain(GAIN_1);
           osDelay(10);
           confparam.calibrate.R_offset_GAIN_1 = GetRS_ON(0);
           SetSignalGain(GAIN_2);
           osDelay(10);
           confparam.calibrate.R_offset_GAIN_2 = GetRS_ON(0);
           SetSignalGain(GAIN_5);
           osDelay(10);
           confparam.calibrate.R_offset_GAIN_5 = GetRS_ON(0);
           SetSignalGain(GAIN_10);
           osDelay(10);
           confparam.calibrate.R_offset_GAIN_10 = GetRS_ON(0);
           SetSignalGain(GAIN_20);
           osDelay(10);
           confparam.calibrate.R_offset_GAIN_20 = GetRS_ON(0);
           SetSignalGain(GAIN_50);
           osDelay(10);
           confparam.calibrate.R_offset_GAIN_50 = GetRS_ON(0);
           SetSignalGain(GAIN_100);
           osDelay(10);
           confparam.calibrate.R_offset_GAIN_100 = GetRS_ON(0);
           ModuleSetParam(&confparam); 
           SetRSCurrent(confparam.testparam.Irs);
          } 
         if(dev_cntrl.float_test){
           osDelay(100);   
           dev_cntrl.test_result.B = GetB_HS(); 
           dev_cntrl.float_test = false;
           button_hs = 3300; /*clear avg filter for button*/
          }         
         break;
        }
       case CHOOSING_GAIN :{
         HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_SET);
         osDelay(1);  
         SetSignalGain(GAIN_1);
         osDelay(10);
         if(GetURS() > (UREF * 90 / 100)){ /*if more than 90%*/
           if(dev_cntrl.continuously_on) fsm = CONTINUOUSLY_ON;
           if(dev_cntrl.activate_rstest) fsm = TEST;           
           HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_RESET);
           current_gain = GAIN_1;
           break;
          }
         SetSignalGain(GAIN_2);
         osDelay(10);
         if(GetURS() > (UREF * 90 / 100)){ /*if more than 90%*/
           if(dev_cntrl.continuously_on) fsm = CONTINUOUSLY_ON;
           if(dev_cntrl.activate_rstest) fsm = TEST;           
           HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_RESET);
           SetSignalGain(GAIN_1);
           current_gain = GAIN_1;
           osDelay(10);
           break;
          }
         SetSignalGain(GAIN_5);
         osDelay(10);
         if(GetURS() > (UREF * 90 / 100)){ /*if more than 90%*/
           if(dev_cntrl.continuously_on) fsm = CONTINUOUSLY_ON;
           if(dev_cntrl.activate_rstest) fsm = TEST;           
           HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_RESET);
           SetSignalGain(GAIN_2);
           current_gain = GAIN_2;
           osDelay(10);
           break;
          }
         SetSignalGain(GAIN_10);
         osDelay(10);
         if(GetURS() > (UREF * 90 / 100)){ /*if more than 90%*/
           if(dev_cntrl.continuously_on) fsm = CONTINUOUSLY_ON;
           if(dev_cntrl.activate_rstest) fsm = TEST;           
           HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_RESET);
           SetSignalGain(GAIN_5);
           current_gain = GAIN_5;
           osDelay(10);
           break;
          }
         SetSignalGain(GAIN_20);
         osDelay(10);
         if(GetURS() > (UREF * 90 / 100)){ /*if more than 90%*/
           if(dev_cntrl.continuously_on) fsm = CONTINUOUSLY_ON;
           if(dev_cntrl.activate_rstest) fsm = TEST;           
           HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_RESET);
           SetSignalGain(GAIN_10);
           current_gain = GAIN_10;
           osDelay(10);
           break;
          }
         SetSignalGain(GAIN_50);
         osDelay(10);
         if(GetURS() > (UREF * 90 / 100)){ /*if more than 90%*/
           if(dev_cntrl.continuously_on) fsm = CONTINUOUSLY_ON;
           if(dev_cntrl.activate_rstest) fsm = TEST;           
           HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_RESET);
           SetSignalGain(GAIN_20);
           current_gain = GAIN_20;
           osDelay(10);
           break;
          }
         SetSignalGain(GAIN_100);
         osDelay(10);
         if(GetURS() > (UREF * 90 / 100)){ /*if more than 90%*/
           if(dev_cntrl.continuously_on) fsm = CONTINUOUSLY_ON;
           if(dev_cntrl.activate_rstest) fsm = TEST;           
           HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_RESET);
           SetSignalGain(GAIN_50);
           current_gain = GAIN_50;
           osDelay(10);
           break;
          }
         SetSignalGain(GAIN_100);         
         current_gain = GAIN_100;
         if(dev_cntrl.continuously_on) fsm = CONTINUOUSLY_ON;
         if(dev_cntrl.activate_rstest) fsm = TEST;           
         osDelay(10);        
         break;
        }
       case CONTINUOUSLY_ON : {
         HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_SET);               
         switch(current_gain){
           case GAIN_1 : {
             dev_cntrl.test_result.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_1);
             break;
            }
           case GAIN_2 : {
             dev_cntrl.test_result.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_2);
             break;
            }
           case GAIN_5 : {
             dev_cntrl.test_result.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_5);
             break;
            }
           case GAIN_10 : {
             dev_cntrl.test_result.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_10);
             break;
            }
           case GAIN_20 : {
             dev_cntrl.test_result.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_20);
             break;
            }
           case GAIN_50 : {
             dev_cntrl.test_result.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_50);
             break;
            }
           case GAIN_100 : {
             dev_cntrl.test_result.RS_R_on = GetRS_ON(confparam.calibrate.R_offset_GAIN_100);
             break;
            }             
          }
         if(!dev_cntrl.continuously_on){
           fsm = WAIT; 
           HAL_GPIO_WritePin(PWM_GPIO_Port, PWM_Pin, GPIO_PIN_RESET);
          }
         break;
        } 
       case TEST : {
         HAL_TIM_Base_Start_IT(&htim1);
         
         taskENTER_CRITICAL();
         dev_cntrl.test_result.RS_R_on = test_result_shadow.RS_R_on;
         dev_cntrl.test_result.dispertion = test_result_shadow.dispertion;
         dev_cntrl.test_result.RS_R_off = test_result_shadow.RS_R_off;
         dev_cntrl.test_result.R_ISO = GetR_ISO();
         dev_cntrl.test_result.cycles_cnt = test_result_shadow.cycles_cnt;
         dev_cntrl.test_result.good_cnt = test_result_shadow.good_cnt; 
         taskEXIT_CRITICAL();
         if(dev_cntrl.mode == SIGNEMODE){
           osDelay(1000);
           HAL_TIM_Base_Stop_IT(&htim1);
           fsm = WAIT;
           dev_cntrl.activate_rstest = false;
           button_rs = 3300; /*clear avg filter for button*/
          }
         else if(!dev_cntrl.activate_rstest){
           fsm = WAIT;         
           HAL_TIM_Base_Stop_IT(&htim1);
          }
         break;
        }        
      }
            
    /*indication*/
    if(abs(timer - osKernelSysTick()) > 500){
       timer = osKernelSysTick();
       HAL_GPIO_WritePin(WRKLED_R_GPIO_Port, WRKLED_R_Pin, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(WRKLED_G_GPIO_Port, WRKLED_G_Pin, GPIO_PIN_SET);                  
      }
    
    osThreadYield();  
   }
}

void StartMagnitTest(){
 dev_cntrl.float_test = true;
}

void SetSignleTestMode(){
 dev_cntrl.mode = SIGNEMODE;  
}

void SetCycleTestMode(){
 dev_cntrl.mode = CYCLEMODE;   
}

void StartRSTest(){
 dev_cntrl.activate_rstest = true; 
}

void StopRSTest(){
 dev_cntrl.activate_rstest = false; 
 dev_cntrl.continuously_on = false; 
}

void StartRISOTest(){
 dev_cntrl.RISO_test = true;
}

void SetOnContinuously(){
 dev_cntrl.continuously_on = true; 
}

test_result_T GetTestResult(){
 return dev_cntrl.test_result; 
}

void StartCalibration(){
 dev_cntrl.calibrateR = true;
}
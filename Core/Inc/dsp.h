#ifndef DSP_H
#define DSP_H

#include <stdint.h>

typedef struct{
 uint16_t RS_R_on; /*in mOmh*/
 double dispertion; /*in mOmh*/
 uint16_t RS_R_off; /*in MOmh*/
 uint16_t R_ISO;
 uint16_t B;
 uint32_t cycles_cnt;
 uint32_t good_cnt; 
}test_result_T;


test_result_T GetTestResult();
void Algorithm();
void SetSignleTestMode();
void SetCycleTestMode();
void StartRSTest();
void StopRSTest();
void StartMagnitTest();
test_result_T GetTestResult();
void StartRISOTest();
void SetOnContinuously();
void StartCalibration();
#endif

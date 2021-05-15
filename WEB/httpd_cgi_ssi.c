#include "debug.h"
#include "tcp.h"
#include "ethernetif.h"
#include "httpd.h"
#include "fs.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include <string.h>
#include "eeprom.h"
#include "dsp.h"
#include "analog.h"


extern struct netif xnetif;

static struct{
 uint8_t web_page[40];
 bool pass_OK;
}WEBFlags = {"/index.shtml", false};

/* we will use character "t" as tag for CGI */
char const* TAGCHAR="t";
char const** TAGS=&TAGCHAR;

const char * WEB_CGI_Handler0(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * WEB_CGI_Handler1(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * WEB_CGI_Handler2(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char * WEB_CGI_Handler3(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

/* Html request for "/leds.cgi" will start WEB_CGI_Handler */
const tCGI WEB_CGI_0={"/web0.cgi", WEB_CGI_Handler0};
const tCGI WEB_CGI_1={"/web1.cgi", WEB_CGI_Handler1};
const tCGI WEB_CGI_2={"/web2.cgi", WEB_CGI_Handler2};
const tCGI WEB_CGI_3={"/web3.cgi", WEB_CGI_Handler3};

/* Cgi call table, only one CGI used */
tCGI CGI_TAB[4];


// -----------------------------------------------------------------------------    
// ------------------------------- SSI Handler ---------------------------------
// -----------------------------------------------------------------------------                   
  
u16_t SSI_Handler(int iIndex, char *pcInsert, int iInsertLen){
 
 uint32_t lenString = 0;
 static devparam_t devparam;
 uint8_t string[30];
 test_result_T test_result;
 
 /*config.shtml*/   
 if(strcmp(WEBFlags.web_page, "/main_conf.shtml") == 0){ 
   if(iIndex == 0){
     test_result = GetTestResult();
     /*R static on*/ 
     sprintf(string, "%d", test_result.RS_R_on);          
     strcpy(pcInsert + lenString, string);
     lenString += strlen(string);
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;
     /*R static off*/ 
     if(test_result.RS_R_off < 50){
       sprintf(string, "%d", test_result.RS_R_off);          
       strcpy(pcInsert + lenString, string);
       lenString += strlen(string);
      }
     else{
       sprintf(string, "> 50");          
       strcpy(pcInsert + lenString, string);
       lenString += strlen(string);
      }
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;     
     /*R static dispertion*/ 
     sprintf(string, "%.2f", test_result.dispertion);          
     strcpy(pcInsert + lenString, string);
     lenString += strlen(string);
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;
    /*R ISO*/ 
     sprintf(string, "%d", test_result.R_ISO);          
     strcpy(pcInsert + lenString, string);
     lenString += strlen(string);
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;       
    /*B*/ 
     sprintf(string, "%d", test_result.B);          
     strcpy(pcInsert + lenString, string);
     lenString += strlen(string);
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;       
    /*rs current*/ 
     sprintf(string, "%d", GetRSCurrent());          
     strcpy(pcInsert + lenString, string);
     lenString += strlen(string);
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;   
    /*frequence*/ 
     sprintf(string, "%d", GetCoilPWMFreq());          
     strcpy(pcInsert + lenString, string);
     lenString += strlen(string);
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;
     /*coil current*/
     sprintf(string, "%d", GetCoilCurrent());          
     strcpy(pcInsert + lenString, string);
     lenString += strlen(string);
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;   
    /*cycles counter*/ 
     sprintf(string, "%d", test_result.cycles_cnt);          
     strcpy(pcInsert + lenString, string);
     lenString += strlen(string);
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;       
    /*good result counter*/ 
     sprintf(string, "%d", test_result.good_cnt);          
     strcpy(pcInsert + lenString, string);
     lenString += strlen(string);
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;       
     /*SW version*/
     strcpy(pcInsert + lenString, __DATE__); 
     lenString += strlen(__DATE__);      
    }
   return strlen(pcInsert); 
  }
 
 if(strcmp(WEBFlags.web_page, "/config_eth.shtml") == 0){ 
   if( iIndex == 0 ){ 
     ModuleGetParam(&devparam);
     /*IP address*/
     sprintf(string, "%d",  (uint8_t)(devparam.eth.dev_ip.addr));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);
     strcpy(pcInsert + lenString, "."); 
     lenString += 1;
     sprintf(string, "%d",  (uint8_t)(devparam.eth.dev_ip.addr >> 8));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);     
     strcpy(pcInsert + lenString, "."); 
     lenString += 1;     
     sprintf(string, "%d",  (uint8_t)(devparam.eth.dev_ip.addr >> 16));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);     
     strcpy(pcInsert + lenString, "."); 
     lenString += 1;     
     sprintf(string, "%d",  (uint8_t)(devparam.eth.dev_ip.addr >> 24));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);     
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;    
     
     /*GW address*/
     sprintf(string, "%d",  (uint8_t)(devparam.eth.gw_ip.addr));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);
     strcpy(pcInsert + lenString, "."); 
     lenString += 1;
     sprintf(string, "%d",  (uint8_t)(devparam.eth.gw_ip.addr >> 8));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);     
     strcpy(pcInsert + lenString, "."); 
     lenString += 1;     
     sprintf(string, "%d",  (uint8_t)(devparam.eth.gw_ip.addr >> 16));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);     
     strcpy(pcInsert + lenString, "."); 
     lenString += 1;     
     sprintf(string, "%d",  (uint8_t)(devparam.eth.gw_ip.addr >> 24));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);     
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;    
     
     /*net mask*/
     sprintf(string, "%d",  (uint8_t)(devparam.eth.netmask.addr));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);
     strcpy(pcInsert + lenString, "."); 
     lenString += 1;
     sprintf(string, "%d",  (uint8_t)(devparam.eth.netmask.addr >> 8));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);     
     strcpy(pcInsert + lenString, "."); 
     lenString += 1;     
     sprintf(string, "%d",  (uint8_t)(devparam.eth.netmask.addr >> 16));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);     
     strcpy(pcInsert + lenString, "."); 
     lenString += 1;     
     sprintf(string, "%d",  (uint8_t)(devparam.eth.netmask.addr >> 24));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);     
     /*new line*/
     strcpy(pcInsert + lenString, "\n");
     lenString++;        
     
     /*MAC address*/
     sprintf(string, "%d",  (uint8_t)(devparam.eth.mac));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);
     strcpy(pcInsert + lenString, ":"); 
     lenString += 1;     
     sprintf(string, "%d",  (uint8_t)(devparam.eth.mac >> 8));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);
     strcpy(pcInsert + lenString, ":"); 
     lenString += 1;          
     sprintf(string, "%d",  (uint8_t)(devparam.eth.mac >> 16));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);
     strcpy(pcInsert + lenString, ":"); 
     lenString += 1;          
     sprintf(string, "%d",  (uint8_t)(devparam.eth.mac >> 24));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);
     strcpy(pcInsert + lenString, ":"); 
     lenString += 1;          
     sprintf(string, "%d",  (uint8_t)(devparam.eth.mac >> 32));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);
     strcpy(pcInsert + lenString, ":"); 
     lenString += 1;          
     sprintf(string, "%d",  (uint8_t)(devparam.eth.mac >> 40));          
     strcpy(pcInsert + lenString, string); 
     lenString += strlen(string);
   /* need to be inserted in html */   
     return strlen(pcInsert);                                      
    }   
  } 
 return 0;
}
// -----------------------------------------------------------------------------    
// ------------------------------- CGI Handler ---------------------------------
// -----------------------------------------------------------------------------

const char * WEB_CGI_Handler0(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
 
 if( iIndex != 0 ) return  "/index.shtml"; 
 
 strcpy(WEBFlags.web_page, "/index.shtml");
 uint8_t pass[16] = {0};
 uint8_t i;
 
 for(i  = 0; i < iNumParams; i++){ 
    
   if(strcmp(pcParam[i] , "pass") == 0 ) memcpy(pass, pcValue[i], strlen(pcValue[i])); 

   if(strcmp(pcParam[i] , "ent_pass") == 0 ){
     if(strcmp(pass, "admin") == 0){
       strcpy(WEBFlags.web_page, "/main_conf.shtml");
       WEBFlags.pass_OK = true;
      }
    } 
  }
 return WEBFlags.web_page;
}

const char * WEB_CGI_Handler1(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
 uint32_t i;
 uint16_t rscurr, freq, coilcurr;
 devparam_t devparam;
 
 if(( iIndex != 1 ) || (!WEBFlags.pass_OK)) return  "/index.shtml"; 
 
 strcpy(WEBFlags.web_page, "/main_conf.shtml");
 
 for(i  = 0; i < iNumParams; i++){ 
   
   if(strcmp(pcParam[i] , "main_config_page") == 0 ) strcpy(WEBFlags.web_page, "/main_conf.shtml");
   if(strcmp(pcParam[i] , "eth_page") == 0 ) strcpy(WEBFlags.web_page, "/config_eth.shtml"); 
   if(strcmp(pcParam[i] , "calib_page") == 0 ) strcpy(WEBFlags.web_page, "/calibrate.shtml"); 
   
   if(strcmp(pcParam[i] , "freq") == 0 ) freq = atoi(pcValue[i]);  
   if(strcmp(pcParam[i] , "rscurr") == 0 ) rscurr = atoi(pcValue[i]);
   if(strcmp(pcParam[i] , "coilcurr") == 0 ) coilcurr = atoi(pcValue[i]);   
   
   if(strcmp(pcParam[i] , "cyc_tststart") == 0 ) {
     SetCycleTestMode();
     StartRSTest();
    }
   if(strcmp(pcParam[i] , "cyc_tststop") == 0 ) StopRSTest();       
   if(strcmp(pcParam[i] , "single_tststart") == 0 ){
     SetSignleTestMode();
     StartRSTest();
    }
   if(strcmp(pcParam[i] , "mag_tststart") == 0 ) StartMagnitTest();     
   if(strcmp(pcParam[i] , "s_rscurr") == 0 ){
     SetRSCurrent(rscurr); 
     ModuleGetParam(&devparam);
     devparam.testparam.Irs = rscurr;          
     ModuleSetParam(&devparam);     
    }
   if(strcmp(pcParam[i] , "s_coilcurr") == 0 ){
     SetCoilCurrent(coilcurr);   
     ModuleGetParam(&devparam);
     devparam.testparam.Icoil = coilcurr;          
     ModuleSetParam(&devparam);          
    }
   if(strcmp(pcParam[i] , "iso_tststart") == 0 ) StartRISOTest(); 
   if(strcmp(pcParam[i] , "sfreq") == 0 ){
     SetCoilPWMFreq(freq);    
     ModuleGetParam(&devparam);
     devparam.testparam.Fcoil = freq;          
     ModuleSetParam(&devparam);               
    }
   if(strcmp(pcParam[i] , "coilon") == 0 ) SetOnContinuously(); 
  }
 return WEBFlags.web_page;
}

const char * WEB_CGI_Handler2(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
 uint32_t i;
 devparam_t devparam;
 uint8_t mac_data[6], dev_data[4], mask_data[4], gw_data[4];
 ip_addr_t ip;


 if(( iIndex != 2 ) || (!WEBFlags.pass_OK)) return  "/index.shtml"; 
 
 strcpy(WEBFlags.web_page, "/config_eth.shtml");
 
 for(i  = 0; i < iNumParams; i++){ 
   
   if(strcmp(pcParam[i] , "main_config_page") == 0 ) strcpy(WEBFlags.web_page, "/main_conf.shtml");
   if(strcmp(pcParam[i] , "eth_page") == 0 ) strcpy(WEBFlags.web_page, "/config_eth.shtml");  
   if(strcmp(pcParam[i] , "calib_page") == 0 ) strcpy(WEBFlags.web_page, "/calibrate.shtml");
   
   if(strcmp(pcParam[i] , "macaddr0") == 0 ) mac_data[0] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "macaddr1") == 0 ) mac_data[1] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "macaddr2") == 0 ) mac_data[2] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "macaddr3") == 0 ) mac_data[3] = atoi(pcValue[i]);     
   if(strcmp(pcParam[i] , "macaddr4") == 0 ) mac_data[4] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "macaddr5") == 0 ) mac_data[5] = atoi(pcValue[i]);     
   if(strcmp(pcParam[i] , "macaddr6") == 0 ) mac_data[5] = atoi(pcValue[i]); 
  
   if(strcmp(pcParam[i] , "dev_ip_ad0") == 0 ) dev_data[0] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "dev_ip_ad1") == 0 ) dev_data[1] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "dev_ip_ad2") == 0 ) dev_data[2] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "dev_ip_ad3") == 0 ) dev_data[3] = atoi(pcValue[i]);   
 
   if(strcmp(pcParam[i] , "mk_ip_ad0") == 0 ) mask_data[0] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "mk_ip_ad1") == 0 ) mask_data[1] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "mk_ip_ad2") == 0 ) mask_data[2] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "mk_ip_ad3") == 0 ) mask_data[3] = atoi(pcValue[i]);      

   if(strcmp(pcParam[i] , "gw_ip_ad0") == 0 ) gw_data[0] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "gw_ip_ad1") == 0 ) gw_data[1] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "gw_ip_ad2") == 0 ) gw_data[2] = atoi(pcValue[i]); 
   if(strcmp(pcParam[i] , "gw_ip_ad3") == 0 ) gw_data[3] = atoi(pcValue[i]);   
   
   
   if(strcmp(pcParam[i] , "eth_sip") == 0 ){
      ModuleGetParam(&devparam);
      IP4_ADDR(&ip, dev_data[0], dev_data[1], dev_data[2], dev_data[3]);
      devparam.eth.dev_ip.addr = ip.addr;         
      ModuleSetParam(&devparam);
     } 
   
   if(strcmp(pcParam[i] , "eth_sgw") == 0 ){
      ModuleGetParam(&devparam);
      IP4_ADDR(&ip, gw_data[0], gw_data[1], gw_data[2], gw_data[3]);
      devparam.eth.gw_ip.addr = ip.addr;         
      ModuleSetParam(&devparam);
     } 
   
   if(strcmp(pcParam[i] , "eth_smk") == 0 ){
      ModuleGetParam(&devparam);
      IP4_ADDR(&ip, mask_data[0], mask_data[1], mask_data[2], mask_data[3]);
      devparam.eth.netmask.addr = ip.addr;         
      ModuleSetParam(&devparam);
     } 
   
   if(strcmp(pcParam[i] , "eth_smac") == 0 ){
      ModuleGetParam(&devparam);
      devparam.eth.mac = (uint64_t)mac_data[0] | ((uint64_t)mac_data[1] << 8)  | ((uint64_t)mac_data[2] << 16) | 
                          ((uint64_t)mac_data[3] << 24) | ((uint64_t)mac_data[4] << 32) | ((uint64_t)mac_data[5] << 40);          
      ModuleSetParam(&devparam);
     }         
  }
 return WEBFlags.web_page;
}

const char * WEB_CGI_Handler3(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
 uint32_t i;
 
 if(( iIndex != 3 ) || (!WEBFlags.pass_OK)) return  "/index.shtml"; 
 
 strcpy(WEBFlags.web_page, "/calibrate.shtml");
 
 for(i  = 0; i < iNumParams; i++){ 
   
   if(strcmp(pcParam[i] , "main_config_page") == 0 ) strcpy(WEBFlags.web_page, "/main_conf.shtml");
   if(strcmp(pcParam[i] , "eth_page") == 0 ) strcpy(WEBFlags.web_page, "/config_eth.shtml"); 
   if(strcmp(pcParam[i] , "calib_page") == 0 ) strcpy(WEBFlags.web_page, "/calibrate.shtml"); 
   
   if(strcmp(pcParam[i] , "calib") == 0 ) StartCalibration();     
  }
 return WEBFlags.web_page;
}

/**
 * Initialize SSI handlers
 */
void httpd_ssi_init(void)
{  
  /* configure SSI handlers */
  http_set_ssi_handler(SSI_Handler, (char const **)TAGS, 1);
}

/**
 * Initialize CGI handlers
 */
void httpd_cgi_init(void){ 
  
  /* configure CGI handlers (LEDs control CGI) */
  CGI_TAB[0] = WEB_CGI_0;
  CGI_TAB[1] = WEB_CGI_1;
  CGI_TAB[2] = WEB_CGI_2;
  CGI_TAB[3] = WEB_CGI_3;
  
  
  http_set_cgi_handlers(CGI_TAB, 4);
}

/*********** Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.*****END OF FILE****/

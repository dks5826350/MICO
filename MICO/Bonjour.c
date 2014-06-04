/**
  ******************************************************************************
  * @file    Bonjour.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   Zero-configuration protocol compatiable with Bonjour from Apple 
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
  ******************************************************************************
  */ 

#include "mico_api.h"
#include "mico_define.h"
#include "mdns.h"
#include "StringUtils.h"
#include "debug.h"
#include "mdns.h"

static int _bonjourStarted = false;

void BonjourNotify_WifiStatusHandler(WiFiEvent event, mico_Context_t * const inContext )
{
  (void)inContext;
  switch (event) {
  case MXCHIP_WIFI_UP:
    suspend_bonjour_service(DISABLE);
    break;
  case MXCHIP_WIFI_DOWN:
    break;
  default:
    break;
  }
  return;
}

void BonjourNotify_SYSWillPoerOffHandler(mico_Context_t * const inContext)
{
  (void)inContext;
  if(_bonjourStarted == true){
    suspend_bonjour_service(ENABLE);
  }
}

OSStatus startBonjourService( mico_Context_t * const inContext )
{
  char temp_txt[500]; 
  char *temp_txt2;
  OSStatus err;
  net_para_st para;
  bonjour_init_t init;

  memset(&init, 0x0, sizeof(bonjour_init_t));

  getNetPara(&para, Station);

  init.service_name = BONJOUR_SERVICE;

  /*   name#xxxxxx.local.  */
  snprintf( temp_txt, 100, "%s#%c%c%c%c%c%c.local.", inContext->flashContentInRam.micoSystemConfig.name, 
                                                     inContext->micoStatus.mac[9], inContext->micoStatus.mac[10], \
                                                     inContext->micoStatus.mac[12], inContext->micoStatus.mac[13], \
                                                     inContext->micoStatus.mac[15], inContext->micoStatus.mac[16] );
  init.host_name = (char*)__strdup(temp_txt);

  /*   name#xxxxxx.   */
  snprintf( temp_txt, 100, "%s#%c%c%c%c%c%c",        inContext->flashContentInRam.micoSystemConfig.name, 
                                                     inContext->micoStatus.mac[9], inContext->micoStatus.mac[10], \
                                                     inContext->micoStatus.mac[12], inContext->micoStatus.mac[13], \
                                                     inContext->micoStatus.mac[15], inContext->micoStatus.mac[16] );
  init.instance_name = (char*)__strdup(temp_txt);

  init.service_port = inContext->flashContentInRam.appConfig.localServerPort;
  init.interface = Station;

  temp_txt2 = __strdup_trans_dot(inContext->micoStatus.mac);
  sprintf(temp_txt, "MAC=%s.", temp_txt2);
  free(temp_txt2);

  temp_txt2 = __strdup_trans_dot(inContext->flashContentInRam.micoSystemConfig.firmwareRevision);
  sprintf(temp_txt, "%sFirmware Rev=%s.", temp_txt, temp_txt2);
  free(temp_txt2);
  
  temp_txt2 = __strdup_trans_dot(inContext->flashContentInRam.micoSystemConfig.hardwareRevision);
  sprintf(temp_txt, "%sHardware Rev=%s.", temp_txt, temp_txt2);
  free(temp_txt2);

  temp_txt2 = __strdup_trans_dot(system_lib_version());
  sprintf(temp_txt, "%sMICO OS Rev=%s.", temp_txt, temp_txt2);
  free(temp_txt2);

  temp_txt2 = __strdup_trans_dot(inContext->flashContentInRam.micoSystemConfig.model);
  sprintf(temp_txt, "%sModel=%s.", temp_txt, temp_txt2);
  free(temp_txt2);

  temp_txt2 = __strdup_trans_dot(inContext->flashContentInRam.micoSystemConfig.protocol);
  sprintf(temp_txt, "%sProtocol=%s.", temp_txt, temp_txt2);
  free(temp_txt2);

  temp_txt2 = __strdup_trans_dot(inContext->flashContentInRam.micoSystemConfig.manufacturer);
  sprintf(temp_txt, "%sManufacturer=%s.", temp_txt, temp_txt2);
  free(temp_txt2);
  
  sprintf(temp_txt, "%sSeed=%u.", temp_txt, inContext->flashContentInRam.micoSystemConfig.seed);
  init.txt_record = (char*)__strdup(temp_txt);

  bonjour_service_init(init);

  free(init.host_name);
  free(init.instance_name);
  free(init.txt_record);

  err = mico_add_notification( mico_notify_WIFI_STATUS_CHANGED, (void *)BonjourNotify_WifiStatusHandler );
  require_noerr( err, exit ); 
  err = mico_add_notification( mico_notify_SYS_WILL_POWER_OFF, (void *)BonjourNotify_SYSWillPoerOffHandler );
  require_noerr( err, exit ); 

  start_bonjour_service();
  _bonjourStarted = true;

exit:
  return err;
}
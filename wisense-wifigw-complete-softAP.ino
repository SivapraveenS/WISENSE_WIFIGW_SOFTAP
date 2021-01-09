#include <MQTT.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 /* wifi manager */
#include "Particle.h"
#include "softap_cred.h"
#include "softap_http.h"
/*
 * File Name: wisense_gateway.ino
 * Author: ram krishnan (rkris@wisense.in)
 * Created: Feb/19/2020f
 *
 *
 * Copyright (c) <2018>, <ram krishnan>
 * All rights reserved.
 *
 * File cannot be copied and/or distributed without the express
 * permission of the author.    
 */
 
/* global declaration */
char str1[32],str2[32],str3[32],str4[50],str5[50];  // global
char mqtt_server[50],mqtt_usn[32],mqtt_pwd[32],mqtt_clientId[50],mqtt_topic[50];
int mqtt_server_sts,mqtt_usn_sts,mqtt_pwd_sts,mqtt_clientId_sts,mqtt_topic_sts,mqtt_server_len,mqtt_usn_len,mqtt_clientId_len;
char mqtt_server_default[32] = "dashboard.wisense.in",mqtt_pwd_default[32]="NULL",mqtt_topic_default[50] = "v1/devices/me/telemetry";
int softAP_flag,softAP_flag_default=3;      //3 for default mqtt method (dashboard.wisense.in)
int mqtt_select_flag=0;          // mqtt_select_flag == 1 (default) mqtt_select_flag == 2(user defined)
char mqtt_server_store[50],mqtt_usn_store[32],mqtt_pwd_store[32],mqtt_clientId_store[50],mqtt_topic_store[50];
int reset_sts_flag=0, setup_status=0, button = D5;                      // button is connected to D5

int addr_size_store[4];
struct Page
{
    const char* url;
    const char* mime_type;
    const char* data;
};

Page myPages[] = {
     { "/index.html", "text/html", index_html },
     { "/rsa-utils/rsa.js", "application/javascript", rsa_js },
     { "/style.css", "text/css", style_css },
     { "/rsa-utils/rng.js", "application/javascript", rng_js },
     { "/rsa-utils/jsbn_2.js", "application/javascript", jsbn_2_js },
     { "/rsa-utils/jsbn_1.js", "application/javascript", jsbn_1_js },
     { "/script.js", "application/javascript", script_js },
     { "/rsa-utils/prng4.js", "application/javascript", prng4_js },
     { nullptr }
};

void myPage(const char* url, ResponseCallback* cb, void* cbArg, Reader* body, Writer* result, void* reserved)
{
    char  _url[strlen(url)+1];
    char* query;
    char* value;
    Log.trace("handling page %s", url);

    strcpy(_url, url);
    if (strtok_r(_url, "?&", &query)) 
    { // is it the control page [0]?
      Log.trace("pre for() - URL: <%s>, *query: <%s>", _url, query);
      while(query) 
      { // and has it got a query string
            Log.trace("in  for() - URL: <%s>, *query: <%s>", _url, query);
            char *token = strtok_r(query, "=", &value);
            Log.trace("*query: <%s>, *value: <%s>, *token: <%s>", query, value, token);
            /*
            To store the data permantely need to store the data in the EEPROM efficiently, althought user entered data length will not remain same
            so the size of the data will not be same. so rightnow we can declare a constant eeprom location too store the data at a allocated location
            */
            int addr=0;
            int size_val_1,size_val_2,size_val_3,size_val_4;
            if(strcmp(token, "str1") == 0) 
            { 
                char* val = strtok_r(NULL, "?&=", &value);
                strncpy(str1, val, sizeof(str1));
            }
            if(strcmp(token, "str2") == 0) 
            { 
                char* val = strtok_r(NULL, "?&=", &value);
                strncpy(str2, val, sizeof(str2));
            }
            if(strcmp(token, "str3") == 0) 
            { 
                char* val = strtok_r(NULL, "?&=", &value);
                strncpy(str3, val, sizeof(str3));
            }
            if(strcmp(token, "str4") == 0) 
            { 
                char* val = strtok_r(NULL, "?&=", &value);
                strncpy(str4, val, sizeof(str4));
            }
            if(strcmp(token, "str5") == 0) 
            { 
                char* val = strtok_r(NULL, "?&=", &value);
                strncpy(str5, val, sizeof(str5));
            }
            if(strcmp(token, "store") == 0) 
            {
                char* val = strtok_r(NULL, "?&=", &value);
                EEPROM.put(250,0);      
                EEPROM.put(0,str1);
                EEPROM.put(50,str2);
                EEPROM.put(100,str3);
                EEPROM.put(150,str4);
                EEPROM.put(200,str5);
                delay(100);
                EEPROM.get(0,mqtt_server);
                EEPROM.get(50,mqtt_usn);
                EEPROM.get(100,mqtt_pwd);
                EEPROM.get(150,mqtt_clientId);
                EEPROM.get(200,mqtt_topic);
                strcpy(mqtt_server_store,mqtt_server);                                   //this is for first time login to this creditinals without this data storing will not happen untill device restarts, because data was fetching before main (constructor)
                strcpy(mqtt_usn_store,mqtt_usn);
                strcpy(mqtt_pwd_store,mqtt_pwd);
                strcpy(mqtt_topic_store,mqtt_topic);
                strcpy(mqtt_clientId_store,mqtt_clientId);
                delay(100);
                reset_sts_flag = 1;
            }
            if(strcmp(token,"save_default") == 0)
            {
                EEPROM.put(250,3);      //3 act as a default status flag
                EEPROM.put(0,"NULL");
                EEPROM.put(50,"NULL");
                EEPROM.put(100,"NULL");
                EEPROM.put(150,"NULL");
                EEPROM.put(200,"NULL");
                delay(100);
                EEPROM.get(250,softAP_flag);    
                strcpy(mqtt_server_store,mqtt_server_default);          //this is for first time login to this creditinals without this data storing will not happen untill device restarts, because data was fetching before main (constructor)
                strcpy(mqtt_usn_store,"NULL");
                strcpy(mqtt_pwd_store,mqtt_pwd_default);
                strcpy(mqtt_clientId_store,"NULL");
                strcpy(mqtt_topic_store,mqtt_topic_default);
                delay(100);
                reset_sts_flag = 1;
            }
            query = value; // continue with the rest
      }
    }

    if (strcmp(url,"/index")==0) {
        Log.trace("sending redirect");
        Header h("Location: /index.html\r\n");
        cb(cbArg, 0, 301, "text/plain", &h);
        return;
    }

    int8_t idx = 0;
    for (;;idx++) {
        Page& p = myPages[idx];
        if (!p.url) {
            idx = -1;
            break;
        }
        else if (strcmp(_url, p.url)==0) {
            break;
        }
    }
    

    Log.trace("refresh page%d <%s>", idx, myPages[idx].url);

    if (idx==-1) {
      cb(cbArg, 0, 404, nullptr, nullptr);
    }
    else {
      cb(cbArg, 0, 200, myPages[idx].mime_type, nullptr);
      result->write(myPages[idx].data);
    }
    /*need of eeprom write fails under user-defined
    EEPROM.put(0,str1);
    EEPROM.put(50,str2);
    EEPROM.put(100,str3);
    EEPROM.put(150,str4);
    */
    EEPROM.get(250,softAP_flag);
    delay(100);
    EEPROM.get(0,mqtt_server);
    EEPROM.get(50,mqtt_usn);
    EEPROM.get(100,mqtt_pwd);
    EEPROM.get(150,mqtt_clientId);
    EEPROM.get(200,mqtt_topic);
    mqtt_server_len = strlen(mqtt_server);
    mqtt_server[mqtt_server_len+1] = '\0';
}

STARTUP(softap_set_application_page_handler(myPage, nullptr));

void serverstore (void) __attribute__ ((constructor)); 

void serverstore (void) 
{ 
    EEPROM.get(250,softAP_flag);
    EEPROM.get(0,mqtt_server);
    EEPROM.get(50,mqtt_usn);
    EEPROM.get(100,mqtt_pwd);
    EEPROM.get(150,mqtt_clientId);
    EEPROM.get(200,mqtt_topic);
    if(mqtt_server == "NULL" || mqtt_server == "dashboard.wisense.in")
    {
        mqtt_server_sts = 1;
        mqtt_usn_sts = 1;
        mqtt_pwd_sts = 1;
        mqtt_clientId_sts = 1;
        mqtt_topic_sts = 1;
    }
    if(softAP_flag == softAP_flag_default)
    {
        mqtt_select_flag = 1;           //default wiSense dashboard
        strcpy(mqtt_server_store,mqtt_server_default);
        strcpy(mqtt_usn_store,"NULL");
        strcpy(mqtt_pwd_store,mqtt_pwd_default);
        strcpy(mqtt_clientId_store,"NULL");
        strcpy(mqtt_topic_store,mqtt_topic_default);
    }
    if(softAP_flag == 0)
    {
        mqtt_select_flag = 2;           //user defined mqtt 
        strcpy(mqtt_server_store,mqtt_server);
        strcpy(mqtt_usn_store,mqtt_usn);
        strcpy(mqtt_pwd_store,mqtt_pwd);
        strcpy(mqtt_topic_store,mqtt_topic);
        strcpy(mqtt_clientId_store,mqtt_clientId);
    }
} 
  
#define LED D7
#define MQTTPORT 1883
//char server[] = "dashboard.wisense.in";
//char server[] = "apps.indriyn.in";                                      //added for customer - don't use this 
//char server[] = "mqtt.aeologic.in";
void callback(char* topic, byte* payload, unsigned int length);

MQTT client(mqtt_server_store, MQTTPORT , callback );           //common access mqtt_server_store

void callback(char* topic, byte* payload, unsigned int length) {
    Particle.publish("MQTT Published" , topic, PRIVATE);    
}

/*
*each dashboard has a seperate access token need to add/replace whenever the new device was created and need to 
*add/replace in the sendtocloud function
*/
char ACCESS_TOKEN_01[] = "09tpyvrmuAUGCSnN4YVA";  //HIH SN - 01         
char ACCESS_TOKEN_02[] = "Df0eDNbFWxhT43ZP0K8o";  //HIH SN - 02
int tspk_rssi,tspk_lqi;

void mqttServerConnect(char* user, char* pass)
{
    if(strcmp(mqtt_clientId_store,"NULL") == 0 || mqtt_clientId_sts == 1 || mqtt_select_flag == 1 && softAP_flag == softAP_flag_default )   //softAP_flag_default = 0;
    {
           client.connect(System.deviceID(), user , pass);
    }
    if(softAP_flag == 0 && mqtt_select_flag == 2 && strcmp(mqtt_clientId,"NULL") != 0 && strcmp(mqtt_clientId_store,"NULL") != 0)         
    {
        client.connect(mqtt_clientId_store,user,pass);
    }
    int sts = 0;
    while(!client.isConnected() && sts<3)
    {
      sts++;
      Particle.publish("Retrying MQTT..." , "wisense broker", PRIVATE);
        if(strcmp(mqtt_clientId_store,"NULL") == 0 || mqtt_clientId_sts == 1 || mqtt_select_flag == 1 && softAP_flag == softAP_flag_default )
         {
            client.connect(System.deviceID(), user , pass); // ClientID, User, PW
         }
        if(softAP_flag == 0 && mqtt_select_flag == 2 && strcmp(mqtt_clientId,"NULL") != 0 && strcmp(mqtt_clientId_store,"NULL") != 0)         
        {
            client.connect(mqtt_clientId_store,user,pass);
        }
      //client.connect(CLIENT_ID_01, user , pass);
    }
    
    return;
}

#include <math.h>
#include "gw.h"
#include "pltfrm.h"
#include "uart.h"
#include "dis.h"
#include "lpwmn.h"
#include "atuat.h"

//#define EVENT_SEND_DATA_TO_CLOUD                            //to send seprate packets data to cloud 
//#define EVENT_FMT_TYPE_MERGED_DATA_STRING                   //to send complete data in string format
#define EVENT_FMT_TYPE_MERGED_DATA_JSON                     //used to send complete node data in one json
uint32_t seq_nr=0;
char GW_macId_1[100]="0x0:0x1:0x70:0xc9";                               //"added for test gateway"
//char GW_macId_2[100]="0x0:0x0:0x97:0x64";                               //added for the customer shipment() dated 15-09-2020
char GW_macId_2[100]="0x0:0x1:0x70:0xc9";
//unsigned long int seq_nr; 
//#define NODE_PACKET_COUNT 4                                //define the packet count from the node

char GW_macBuff[1024];
char GW_snsrIdBuff[16];
char GW_devName[1024];


#define GW_COORD_RESP_TMO_MILLISECS  2000

#define UART_RX_BUFF_LEN   1024

#define GW_COORD_KEEP_ALIVE_INTERVAL_MILLISECS  5000
#define GW_COORD_NO_ACTIVITY_TIME_OUT_SECS  (GW_COORD_KEEP_ALIVE_INTERVAL_MILLISECS * 5)

// #define WIFI_MAC_ADDR_LEN  6

#define LPWMN_CLOUD_CMD_BUFF_LEN  64
char LPWMN_cloudCmdBuff[LPWMN_CLOUD_CMD_BUFF_LEN];

unsigned int GW_buildSendHdr(int msgType, unsigned char *pyldBuff_p, int pyldLen);

int verbose = 0;

int GW_uartRxCnt = 0;
int GW_msgTotLen = UART_FRAME_HDR_LEN;
int GW_msgPyldLen = 0;
int GW_rcvdMsgCnt = 0;
int GW_noPktKAIntvCnt = 0;

int GW_relaySnsrDataToCloud = 1;  // Send sensor data to particle cloud (default behavior)

unsigned char GW_uartRxBuff[UART_RX_BUFF_LEN];
char GW_eventBuff[1024];
char GW_snsrDataBuff[512] = {'\0'};
unsigned char GW_serTxHdrBuff[UART_FRAME_HDR_LEN];
unsigned char GW_serTxPyldBuff[256];

unsigned int GW_lpwmnCoordDownCnt = 0,
             GW_lpwmnCoordUpCnt = 0;
             
unsigned char GW_seqNrSentToCoord = 0x57;  // random value
             
#define GW_ATUAT_ROUTER_ARRAY_ENTRY_CNT  128
int GW_atuatRouterArray[GW_ATUAT_ROUTER_ARRAY_ENTRY_CNT];

unsigned int GW_expMsgTypeFromCoord = LPWMN_GW_MSG_TYPE_INVALID;
unsigned int GW_expHdrSeqNrFromCoord = 0x100;   // Valid range is 0x00 - 0xff
unsigned int GW_pendingTxPyldLength = 0;

typedef struct
{
   unsigned int  tagId;
   unsigned int count;
} GW_atuatTagEntry_s;

#define GW_ATUAT_TAG_ARRAY_ENTRY_CNT  10

GW_atuatTagEntry_s  GW_atuatTagArray[GW_ATUAT_TAG_ARRAY_ENTRY_CNT];

#define GW_ATUAT_BCN_PUBLISH_SKIP_CNT  8

// char WIFI_apRespBuff[512];
// WiFiAccessPoint WIFI_ap[5];

unsigned char USB_GW_APP_rxBuff[128];
int USB_GW_APP_uartRxCnt = 0, USB_GW_APP_msgPyldLen = 0, USB_GW_APP_msgTotLen = 0;

unsigned char USB_GW_APP_txBuff[256];

/*
 * You may specify in code which antenna to use as the default at boot time using the STARTUP() macro.
 * Note that the antenna selection is remembered even after power off or when entering safe mode. This 
 * is to allow your device to be configured once and then continue to function with the selected 
 * antenna when applications are flashed that don't specify which antenna to use.
 */
// STARTUP(WiFi.selectAntenna(ANT_EXTERNAL)); // selects the u.FL antenna


// Every program based on Wiring (programming language used by Arduino, and Particle devices) has two essential parts:
// setup - runs once at the beginning of your program
// loop - runs continuously over and over


/*
 * The "manual" mode puts the device's connectivity completely in the user's control. 
 * This means that the user is responsible for both establishing a connection to the 
 * Particle Cloud and handling communications with the Cloud by calling Particle.process() 
 * on a regular basis.
 */
// SYSTEM_MODE(MANUAL); 

/*
 * At present, System Thread is an opt-in change. To enable system threading for 
 * your application, add to the top of your application code.
 */
SYSTEM_THREAD(ENABLED);

/*
 * The System Thread is a system configuration that helps ensure the application loop is not 
 * interrupted by the system background processing and network management. It does this by 
 * running the application loop and the system loop on separate threads, so they execute in 
 * parallel rather than sequentially.
 *
 *
 * With system threading enabled, the majority of the Particle API continues to run on the calling 
 * thread, as it does for non-threaded mode. For example, when a function, such as Time.now(), is 
 * called, it is processed entirely on the calling thread (typically the application thread when 
 * calling from loop().)
 *
 */
 
#define GW_COORD_STATE_DOWN  1
#define GW_COORD_STATE_UP  2

// First, we're going to make some variables.
// This is our "shorthand" that we'll use throughout the program:

int ledTxToCloud = D2;
int ledRxFromCoord = D1; 
int ledKA = D0;

int ledDbg = D7;

#ifdef GW_COORD_HEALTH_MON
volatile char GW_schedKAMsgPending = 0;
volatile char GW_coordStateEvtPending = 0;
volatile char GW_coordState = GW_COORD_STATE_DOWN;



// The timer callback is similar to an interrupt - it shouldn't block. However, it is less 
// restrictive than an interrupt. If the code does block, the system will not crash - the 
// only consequence is that other software timers that should have triggered will be delayed 
// until the blocking timer callback function returns.
Timer GW_coordKATimer(GW_COORD_KEEP_ALIVE_INTERVAL_MILLISECS, GW_coordKATmrExpCbFunc);

      
/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void GW_restartKATimer(void)
{
   // Serial.printf("Restarting KA Timer .. \r\n");
   GW_noPktKAIntvCnt = 0;
   GW_coordKATimer.reset();  
}


/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void GW_coordKATmrExpCbFunc()
{
   int timeSecs = GW_COORD_KEEP_ALIVE_INTERVAL_MILLISECS;
   
   GW_noPktKAIntvCnt ++;
   
   timeSecs *= GW_noPktKAIntvCnt;
   
   // Serial.printf("KAExpFunc - %d %d/%d \r\n", GW_noPktKAIntvCnt, timeSecs, GW_COORD_NO_ACTIVITY_TIME_OUT_SECS);
  
   if (timeSecs >= GW_COORD_NO_ACTIVITY_TIME_OUT_SECS)
   {
       // Serial.printf("Coord seems to be down !! \r\n");
       
       GW_lpwmnCoordDownCnt ++;
       
       GW_coordState = GW_COORD_STATE_DOWN;
       
       GW_coordStateEvtPending = 1;
       
       // Reset the coord here ....
      
       GW_restartKATimer();
   }
   else
   {
       GW_schedKAMsgPending = 1;
   }
}
#endif


Timer GW_coordRespTmoTimer(GW_COORD_RESP_TMO_MILLISECS, GW_coordRespTmoExpCbFunc);

void GW_coordRespTmoExpCbFunc()
{
    GW_expMsgTypeFromCoord = LPWMN_GW_MSG_TYPE_INVALID;
    GW_expHdrSeqNrFromCoord = 0x100;    
    GW_pendingTxPyldLength = 0;
}


/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void GW_procExpCoordResp(unsigned int rcvdMsgType)
{
    // Serial.printf("rcvd exp msg type <%u> ... pending tx len<%u> \n\r", 
    //               rcvdMsgType, GW_pendingTxPyldLength);
    
    switch (rcvdMsgType)
    {
       case UART_MSG_TYPE_ACK:
            {
               if (GW_pendingTxPyldLength > 0)
               {
                   Serial1.write(GW_serTxPyldBuff, GW_pendingTxPyldLength);  
                   GW_pendingTxPyldLength = 0;   
               }
            }
            break;
          
       default:
            break;
    }

    GW_coordRespTmoTimer.stop();
    GW_expMsgTypeFromCoord = LPWMN_GW_MSG_TYPE_INVALID;
    GW_expHdrSeqNrFromCoord = 0x100;
}


/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
int GW_rebootCoordReq(void)
{
   GW_buildSendHdr(LPWMN_GW_MSG_TYPE_REBOOT_COORD, NULL, 0x0);
   
   return 1;
}


/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
int GW_rebootNodeReq(char *cmdBuff_p)
{
   int idx, rc, off = 0;
   int shortAddr = 0;
   unsigned char *pyld_p = GW_serTxPyldBuff;
   unsigned char extAddr[LPWMN_MAC_EXT_ADDR_LEN];
   
   rc = sscanf(cmdBuff_p, "rstn %x:%x:%x:%x %d %d %d", 
               &extAddr[4], &extAddr[5], &extAddr[6], &extAddr[7]);
   if (rc != 4)
       return -1;
       

   for (idx=4; idx<LPWMN_MAC_EXT_ADDR_LEN; idx++)
   {
       if (!((extAddr[idx] >= 0) && (extAddr[idx] <= 0xff)))
           break;
   }
   
   if (idx < LPWMN_MAC_EXT_ADDR_LEN)
       return -2;
   else
   {
       LPWMN_nwkNodeInfo_s *node_p = LPWMN_lookUpNode(extAddr, 4);
       if (node_p == NULL)
           return -3;
       shortAddr =  node_p->shortAddr;   
   }      
   
   GW_pendingTxPyldLength = LPWMN_MAC_SHORT_ADDR_LEN
                            + DIS_MSG_TYPE_SZ;
                
   GW_htons(pyld_p, shortAddr);
   off = LPWMN_MAC_SHORT_ADDR_LEN;
   pyld_p[off ++] = DIS_MSG_TYPE_NODE_REBOOT_REQ;


   GW_expHdrSeqNrFromCoord = GW_buildSendHdr(LPWMN_GW_MSG_TYPE_RELAY_TO_NODE,
                                             pyld_p, GW_pendingTxPyldLength);
                   
   GW_expMsgTypeFromCoord = UART_MSG_TYPE_ACK;
   
   // Start a timer.
   GW_coordRespTmoTimer.start();
   
   return 1;
}



/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
int GW_cfgDataPushInterval(char *cmdBuff_p)
{
   int idx, rc, off = 0, intvSecs, shortAddr = 0;
   unsigned char *pyld_p = GW_serTxPyldBuff;
   unsigned char extAddr[LPWMN_MAC_EXT_ADDR_LEN];
   
   rc = sscanf(cmdBuff_p, "cfg-dpi %x:%x:%x:%x %d", 
               &extAddr[4], &extAddr[5], &extAddr[6], &extAddr[7],
               &intvSecs);
   if (rc != 5)
       return -1;

   for (idx=4; idx<LPWMN_MAC_EXT_ADDR_LEN; idx++)
   {
       if (!((extAddr[idx] >= 0) && (extAddr[idx] <= 0xff)))
           break;
   }
   
   if (idx < LPWMN_MAC_EXT_ADDR_LEN)
       return -2;
   else
   {
       LPWMN_nwkNodeInfo_s *node_p = LPWMN_lookUpNode(extAddr, 4);
       if (node_p == NULL)
           return -3;
       shortAddr =  node_p->shortAddr;   
   }
   
   if (intvSecs < 1 || intvSecs > 65535)
   {
       return -4;
   }
   
   GW_pendingTxPyldLength = LPWMN_MAC_SHORT_ADDR_LEN
                            + DIS_MSG_TYPE_SZ
                            + DIS_TLV_HDR_SZ
                            + LPWMN_GW_MSG_NODE_DATA_PUSH_INTERVAL_FIELD_LEN;
   
   GW_htons(pyld_p, shortAddr);
   off = LPWMN_MAC_SHORT_ADDR_LEN;
   pyld_p[off ++] = DIS_MSG_TYPE_CFG_NODE_DATA_PUSH_INTERVAL;
   pyld_p[off ++] = DIS_TLV_TYPE_PUSH_INTERVAL;
   pyld_p[off ++] = LPWMN_GW_MSG_NODE_DATA_PUSH_INTERVAL_FIELD_LEN;
   GW_htons(pyld_p + off, intvSecs);

   GW_expHdrSeqNrFromCoord = GW_buildSendHdr(LPWMN_GW_MSG_TYPE_RELAY_TO_NODE,
                                             pyld_p, GW_pendingTxPyldLength);
                   
   GW_expMsgTypeFromCoord = UART_MSG_TYPE_ACK;
  
   // Start a timer.
   GW_coordRespTmoTimer.start();
   
   // Serial.printf("Sent dioc hdr .. \n\r");
   
   
   return 1;
}



/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
int GW_digitalIOCtrl(char *cmdBuff_p)
{
   int idx, rc, off = 0;
   int shortAddr = 0, portId, pinNr, val;
   unsigned char *pyld_p = GW_serTxPyldBuff;
   unsigned char extAddr[LPWMN_MAC_EXT_ADDR_LEN];
   
   rc = sscanf(cmdBuff_p, "dioc %x:%x:%x:%x %d %d %d", 
               &extAddr[4], &extAddr[5], &extAddr[6], &extAddr[7],
               &portId, &pinNr, &val);
   if (rc != 7)
       return -1;
       

   for (idx=4; idx<LPWMN_MAC_EXT_ADDR_LEN; idx++)
   {
       if (!((extAddr[idx] >= 0) && (extAddr[idx] <= 0xff)))
           break;
   }
   
   if (idx < LPWMN_MAC_EXT_ADDR_LEN)
       return -2;
   else
   {
       LPWMN_nwkNodeInfo_s *node_p = LPWMN_lookUpNode(extAddr, 4);
       if (node_p == NULL)
           return -3;
       shortAddr =  node_p->shortAddr;   
   }
   
   if (portId < 0x1 || portId > 16)
       return -4;

   if (pinNr > 7)
       return -5;
 
   if (val != 0 && val != 1)
       return -6;

   GW_pendingTxPyldLength = LPWMN_MAC_SHORT_ADDR_LEN
                            + DIS_MSG_TYPE_SZ
                            + DIS_TLV_HDR_SZ   // DIS_TLV_TYPE_CTRL_DIGITAL_IO
                            + DIS_DIGITAL_IO_PORT_TLV_SZ
                            + DIS_DIGITAL_IO_PIN_TLV_SZ
                            + DIS_DIGITAL_IO_VAL_TLV_SZ;
             
   GW_htons(pyld_p, shortAddr);
   off = LPWMN_MAC_SHORT_ADDR_LEN;
   pyld_p[off ++] = DIS_MSG_TYPE_CTRL_DIGITAL_IO;

   pyld_p[off ++] = DIS_TLV_TYPE_DIGITAL_IO;
   pyld_p[off ++] = (DIS_DIGITAL_IO_PORT_TLV_SZ
                     + DIS_DIGITAL_IO_PIN_TLV_SZ
                     + DIS_DIGITAL_IO_VAL_TLV_SZ);

   pyld_p[off ++] = DIS_TLV_TYPE_DIGITAL_IO_PORT;
   pyld_p[off ++] = DIS_DIGITAL_IO_PORT_FIELD_SZ;
   pyld_p[off ++] = portId;

   pyld_p[off ++] = DIS_TLV_TYPE_DIGITAL_IO_PIN;
   pyld_p[off ++] = DIS_DIGITAL_IO_PIN_FIELD_SZ;
   pyld_p[off ++] = pinNr;

   pyld_p[off ++] = DIS_TLV_TYPE_DIGITAL_IO_VAL;
   pyld_p[off ++] = DIS_DIGITAL_IO_VAL_FIELD_SZ;
   pyld_p[off ++] = val;
   
   GW_expHdrSeqNrFromCoord = GW_buildSendHdr(LPWMN_GW_MSG_TYPE_RELAY_TO_NODE,
                                             pyld_p, GW_pendingTxPyldLength);
                   
   GW_expMsgTypeFromCoord = UART_MSG_TYPE_ACK;
  
   // Start a timer.
   GW_coordRespTmoTimer.start();
   
   // Serial.printf("Sent dioc hdr .. \n\r");

   return 1;
}
   

/*
 *********************************************************************************
 *
 * curl https://api.particle.io/v1/devices/290034000347353137323334/LPWMN_cmd  \
 *      -d access_token=7393265a9fe01fe23229b3a0a0c1bfb548468c1e   -d "args=rstc"
 *
 *********************************************************************************
 */
int LPWMN_procCmd(String cmd)
{
   int rc = 0, cmdStrLen = cmd.length();
   
   /*
    * A cloud function is set up to take one argument of the String datatype. This 
    * argument length is limited to a max of 63 characters.
    */
    
   if (cmdStrLen >= sizeof(LPWMN_cloudCmdBuff))
       return -100;
       
   if (GW_expMsgTypeFromCoord != LPWMN_GW_MSG_TYPE_INVALID)
       return -101;
       
   cmd.toCharArray(LPWMN_cloudCmdBuff, sizeof(LPWMN_cloudCmdBuff));
   
   //  Serial.printf("Received command <%s> of length<%d> from cloud \r\n", 
   //                LPWMN_cloudCmdBuff, cmdStrLen);
   
   if (strstr(LPWMN_cloudCmdBuff, "rstc") != NULL)
   {
       // This could conflict with messages being sent to the coord originating from
       // the local USB port.
       rc = GW_rebootCoordReq();
       goto _end;
   }
   
   if (strstr(LPWMN_cloudCmdBuff, "rstn") != NULL)
   {
       // This could conflict with messages being sent to the coord originating from
       // the local USB port.
       rc = GW_rebootNodeReq(LPWMN_cloudCmdBuff);
       goto _end;
   }   
   
   if (strstr(LPWMN_cloudCmdBuff, "dioc") != NULL)
   {
       rc = GW_digitalIOCtrl(LPWMN_cloudCmdBuff);
       // Serial.printf("GW_digitalIOCtrl() retVal<%d> \r\n", rc);
       goto _end;
   }
  
   if (strstr(LPWMN_cloudCmdBuff, "cfg-dpi") != NULL)
   {
       rc = GW_cfgDataPushInterval(LPWMN_cloudCmdBuff);
       // Serial.printf("GW_digitalIOCtrl() retVal<%d> \r\n", rc);
       goto _end;
   }
   
_end:   
   return rc;
}


// Having declared these variables, let's move on to the setup function.
// The setup function is a standard part of any microcontroller program.
// It runs only once when the device boots up or is reset.
/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
int GW_TimeSync()
{
    Particle.syncTime();
    waitUntil(Particle.syncTimeDone);
    Time.zone(+5.5);
    Particle.process();
}

void setup() 
{
   memset(GW_atuatRouterArray, 0, sizeof(GW_atuatRouterArray));
   memset(GW_atuatTagArray, 0, sizeof(GW_atuatTagArray));
    
   LPWMN_initNodeList();    
    GW_TimeSync();
    
  pinMode(button, INPUT_PULLUP);    // sets pin as input
   pinMode(ledKA, OUTPUT);
   pinMode(ledDbg, OUTPUT);
   pinMode(ledTxToCloud, OUTPUT);
   pinMode(ledRxFromCoord, OUTPUT);
  
   // Serial1: This channel is available via the device's TX and RX pins.
   // begin(): Enables serial channel with specified configuration.
   // Baud rates of 1200 up to 115200 are supported for hardware serial channels.
   // These pins operate at 0V to 3.3V. 
   //Serial1.begin(38400, SERIAL_8N1); // via TX/RX pins, 9600 9N1 mode
  Serial1.begin(38400, SERIAL_8N1); // via TX/RX pins, 9600 9N1 mode
   // This channel communicates through the USB port and when connected to a computer, 
   // will show up as a virtual COM port.
   //Serial.begin(38400);          // via USB port
  Serial.begin(38400);          // via USB port
  
   /*
    * Up to 15 cloud functions may be registered and each function name is limited to a 
    * maximum of 12 characters./
    *
    * Cloud functions registered with Particle.function(). continue to execute on the app
    * thread in between calls to loop(), or when Particle.process() or delay() is called. 
    * A long running cloud function will block the application loop (since it is app code) 
    * but not the system code, so cloud connectivity is maintained.
    */
   Particle.function("LPWMN_cmd", LPWMN_procCmd);

#ifdef GW_COORD_HEALTH_MON
   GW_coordKATimer.start();
#endif
}


/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
unsigned short __crc16(unsigned char *buff_p, unsigned int len)
{
   unsigned int ckSum = 0;

   while (len > 1)
   {
      unsigned short tmp = *buff_p;
      tmp = (tmp << 8) | (*(buff_p + 1));
      ckSum = ckSum + tmp;
      buff_p += 2;
      len -= 2;
   }

   if (len > 0)
       ckSum += (*buff_p);

   while (ckSum >> 16)
   {
      ckSum = (ckSum & 0xffff) + (ckSum >> 16);
   }

   return (~ckSum);
}


/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
unsigned char TLV_get(unsigned char *buff_p, unsigned char len, unsigned char type,
                      unsigned char *pyldLen_p, unsigned char **pyldBuff_pp)
{
    int buffLen = len;
    unsigned char rc = 0;

    if (buffLen < DIS_TLV_HDR_SZ)
        return 0;

    // Get the tlv type

    while (buffLen >= DIS_TLV_HDR_SZ)
    {
        unsigned char tlvPyldLen = *(buff_p + DIS_TLV_TYPE_FIELD_LEN);

        if (*buff_p == type)
        {
            *pyldLen_p = tlvPyldLen;
            *pyldBuff_pp = (buff_p + DIS_TLV_HDR_SZ);
            rc = 1;
            break;
        }
        else
        {
            buff_p += (DIS_TLV_HDR_SZ + tlvPyldLen);
            buffLen -= (DIS_TLV_HDR_SZ + tlvPyldLen);
        }
    }

    return rc;
}


float __latestVcc;
int __latestVccSet = 0;


/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
unsigned short GW_ntohs(unsigned char *buff_p)
{
   short u16Val = *buff_p;
   u16Val = (u16Val << 8) | buff_p[1];  
   return u16Val;
}


/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
void GW_htonl(unsigned char *buff_p, unsigned int val)
{
   buff_p[0] = (val >> 24) & 0xff;
   buff_p[1] = (val >> 16) & 0xff;
   buff_p[2] = (val >> 8) & 0xff;
   buff_p[3] = (val) & 0xff;
}


/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
void GW_htons(unsigned char *buff_p, unsigned short val)
{
   buff_p[0] = (val >> 8) & 0xff;
   buff_p[1] = (val) & 0xff;
}


/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
unsigned int GW_ntohl(unsigned char *buff_p)
{
   unsigned int u32Val = *buff_p;
   u32Val <<= 8;
   u32Val |= buff_p[1];
   u32Val <<= 8;
   u32Val |= buff_p[2];
   u32Val <<= 8;
   u32Val |= buff_p[3];
   return u32Val;
}

/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
int GW_getATUATRouterIdx(unsigned int rtrId)
{
   int idx, rc = -1;
   
   for (idx=0; idx < GW_ATUAT_ROUTER_ARRAY_ENTRY_CNT; idx++)
   {
        if (GW_atuatRouterArray[idx] == 0)
            break;
            
        if (GW_atuatRouterArray[idx] == rtrId)
        {
            rc = idx;
            break;
        }
   }    
   
   return rc;
}

/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
unsigned int GW_getATUATTagBcnCnt(unsigned int tagId)
{
   int idx, found = 0;
   unsigned int rc;
   
   for (idx=0; idx < GW_ATUAT_TAG_ARRAY_ENTRY_CNT; idx++)
   {
        if (GW_atuatTagArray[idx].tagId == 0)
            break;
            
        if (GW_atuatTagArray[idx].tagId == tagId)
        {
            rc = (++ GW_atuatTagArray[idx].count);
            found = 1;
            break;
        }
    }
   
   if (found == 0 && (idx < GW_ATUAT_TAG_ARRAY_ENTRY_CNT))
   {
       GW_atuatTagArray[idx].tagId = tagId;
       GW_atuatTagArray[idx].count = 1;
       rc =  1;
   }   
   
   return rc;
}


/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
int GW_addATUATRouter(unsigned int rtrId)
{
   int idx, rc = -1;
   
   for (idx=0; idx < GW_ATUAT_ROUTER_ARRAY_ENTRY_CNT; idx++)
   {
        if (GW_atuatRouterArray[idx] == 0)
        { 
            GW_atuatRouterArray[idx] = rtrId;
            return idx;
        }
   }    
   
   return 255;
}
//2
#ifdef EVENT_FMT_TYPE_MERGED_DATA_JSON
/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
int GW_DataBufLen=0;
int merge_store[10],merge_count=0;                        //to store each buffer length (for each packet's)
char merge_event[100];
//char GW_macBuff[1024];
//char GW_snsrIdBuff[16];
//char GW_devName[1024];

//#define NODE_PACKET_COUNT 3                               //define the packet count from the node define form 1-n
//char snsrIdStore[]
//int GW_snsrDataBuffLen=0;                     //used to store total data_lenght for spliting use
char snsrIdStore0[16],snsrIdStore1[16],snsrIdStore2[16],snsrIdStore3[16],snsrIdStore4[16];
char GW_snsrDataBuff_0[150]={'\0'};
char GW_snsrDataBuff_1[150]={'\0'};
char GW_snsrDataBuff_2[150]={'\0'};
char GW_snsrDataBuff_3[150]={'\0'};
char GW_snsrDataBuff_4[150]={'\0'};
char GW_get_time[150]={'\0'};
//char snsrId_unit_0[10]={'\0'},snsrId_unit_1[10]={'\0'},snsrId_unit_2{'\0'},snsrId_unit_3{'\0'},snsrId_unit_3{'\0'};

float snsrDataBuff[10]={0};

int snsrIdStore[10];     

/*  for every packet the node data valF respect to the unit_p will differ, remaining the macid= extAddr_p, snsrId will remain same if the data receive from one sensor node 
    storing snsId's at specific rate: id[0] = node_voltage, id[1]=msp temp, id[2]=humdity, id[3]=temp ....n
*/

void GW_appendToEventBufferJson(unsigned char *extAddr_p,
                            int snsrId,
                            char *unit_p,
                            float valF)
{
       
    int strLen = strlen(GW_snsrDataBuff);
    if(merge_count==0)
    {
        merge_store[merge_count] = strLen;
        snsrIdStore[merge_count] = snsrId;
        snsrDataBuff[merge_count]= valF;
    }
    if(merge_count==1)
    {
        merge_store[merge_count] = strLen;
        snsrIdStore[merge_count] = snsrId,
        snsrDataBuff[merge_count]= valF;
    }
    if(merge_count==2)
    {
        merge_store[merge_count] = strLen;   
        snsrIdStore[merge_count] = snsrId;
        snsrDataBuff[merge_count]= valF;
    }
    if(merge_count==3)
    {
        merge_store[merge_count] = strLen;
        snsrIdStore[merge_count] = snsrId;
        snsrDataBuff[merge_count]= valF;
   }
   /* test; add in addition of using multiple sensor respect to data count 
   if(merge_count==4)
   {
       merge_store[merge_count] = strLen;
       snsrIdStore[merge_count] = snsrId;
       snsrDataBuff[merge_count] = valF;
   }
   */

   if(strLen == 0)
   { 
        strLen = sprintf(GW_snsrDataBuff, "ID_%02x%02x%02x%02x",                                //GW_snsrDataBuff will store the merged string here Device macId 
                        extAddr_p[4], extAddr_p[5], extAddr_p[6], extAddr_p[7]);
        merge_store[merge_count] = strLen;  //Node macId length storing in the 0 location of merge_store
        snsrIdStore[merge_count] = snsrId;
        /*
            sprintf(GW_eventBuff, "Total Buffer Length: ");
            sprintf(GW_TotalBuffCount,"Count = %d",GW_DataBufLen);
            Particle.publish(GW_eventBuff, GW_TotalBuffCount);
            
            sprintf(GW_eventBuff, "Merge_store_Check: ");
            sprintf(merge_event,"0=%d,1=%d,2=%d,3=%d,4=%d",merge_store[0],merge_store[1],merge_store[2],merge_store[3],merge_store[4]);
            Particle.publish(GW_eventBuff,merge_event);
            GW_snsrDataBuffLen= GW_DataBufLen;
            GW_DataBufLen=0;
            merge_count=0;
        */
   }
    merge_count++;

    sprintf(GW_devName, "WSN-%02x%02x%02x%02x", extAddr_p[4], extAddr_p[5], extAddr_p[6], extAddr_p[7]);
    sprintf(GW_macBuff,"%02x:%02x:%02x:%02x", extAddr_p[4],extAddr_p[5],extAddr_p[6],extAddr_p[7]);

    GW_DataBufLen = GW_DataBufLen + strLen; 
    
    /*if(merge_count == NODE_PACKET_COUNT)
    {
        merge_count=0;
    }*/
    sprintf(GW_snsrDataBuff + strLen,  "__S%d_V%.3f_U%s", snsrId, valF, unit_p);                         //complete merged string it will store (2_t)
}

/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void GW_appendIntToEventBufferJson(unsigned char *extAddr_p,
                               int snsrId,
                               char *unit_p,
                               int valI)
{
   int strLen = strlen(GW_snsrDataBuff);
   
   if (strLen == 0)
   { 
       strLen = sprintf(GW_snsrDataBuff, "ID_%02x%02x%02x%02x",            
                        extAddr_p[4], extAddr_p[5], extAddr_p[6], extAddr_p[7]);
   }
   sprintf(GW_snsrDataBuff + strLen,  "__S%d_V%d_U%s", snsrId, valI, unit_p);
}

/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */

void GW_sendMergedDataEvtToCloudJson(void)
{
    int i,GW_snsrStoreLen=0;
    
   if (GW_relaySnsrDataToCloud == 0)
       return;
    
   if (strlen(GW_snsrDataBuff) > 0)
   {
       if (Particle.connected())
       {
            //first time only after configuration
            /*reset_sts_flag for system reset */
            if(reset_sts_flag == 1)
            {
                delay(100);
                reset_sts_flag = 0;
                System.reset();
            }
           
            int snsrId_count =4;
            seq_nr++;
            digitalWrite(ledTxToCloud, HIGH);       
            char GW_jsonMsg[1024];
            char GW_mqttMsg[1024];
            StaticJsonBuffer<1024> jsonBuffer;
            JsonObject& root = jsonBuffer.createObject();
            JsonObject& rootMqtt = jsonBuffer.createObject();
            
            rootMqtt["MQTT Server"] = mqtt_server;
            rootMqtt["MQTT username"] = mqtt_usn;
            rootMqtt["MQTT password"] = mqtt_pwd;
            rootMqtt["MQTT clientId"] = mqtt_clientId;
            rootMqtt["MQTT Topic"] = mqtt_topic;
            rootMqtt.printTo(GW_mqttMsg);
            
            /*spilting GW_snsrDataBuff into json file
              GW_snsrDataBuff_0=Node_voltage,  GW_snsrDataBuff_1=msp_temp, GW_snsrDataBuff_2=R_Humidity, GW_snsrDataBuff_3=R_TEMP, GW_snsrDataBuff_4= *** 
            */
            sprintf(GW_snsrDataBuff_0, "%.2f",snsrDataBuff[0]);         //changed to 2 decimal points        
            sprintf(GW_snsrDataBuff_1, "%.2f",snsrDataBuff[1]);
            sprintf(GW_snsrDataBuff_2, "%.2f",snsrDataBuff[2]);
            sprintf(GW_snsrDataBuff_3, "%.2f",snsrDataBuff[3]);     
            sprintf(GW_snsrDataBuff_4, "%.2f",snsrDataBuff[4]);
                        

            //sprintf(snsrIdStore4,"0x%x",snsrIdStore[4]);                //added test
            for(int i=0;i<snsrId_count;i++)
            {
                if(snsrIdStore[i] == 0x8)
                {
                    snsrIdStore[i] = 0x80;
                }
            }
                
            sprintf(snsrIdStore3,"0x%x",snsrIdStore[3]);
            sprintf(snsrIdStore2,"0x%x",snsrIdStore[2]);
            sprintf(snsrIdStore1,"0x%x",snsrIdStore[1]);
            sprintf(snsrIdStore0,"0x%x",snsrIdStore[0]);

            //Time.format(Time.now(), "%I:%M:%S%p");
            sprintf(GW_get_time,Time.format(Time.now(),"%Y-%m-%d %H:%M:%S"));
            
            //sprintf(GW_get_time,"%I:%M:%S%p",get_time);
            
            /*use depends upon the packet_dataValf count                                                                                         
            root["Name"] = GW_devName;          
            root["MacId"] = GW_macBuff;
            root[snsrIdStore0] = GW_snsrDataBuff_0;     
            root[snsrIdStore1] = GW_snsrDataBuff_1;
            root[snsrIdStore2] = GW_snsrDataBuff_2;
            root[snsrIdStore3] = GW_snsrDataBuff_3;
            root["SensorData Push time"] = GW_get_time;
            //root[snsrIdStore4] = GW_snsrDataBuff_4;                     //added test
            */

            root["Name"] = GW_devName; 
            //root["GW MacId"] = GW_macId_1;
            root["GW MacId"] = GW_macId_2;                                  //added for customer() gateway dated- 15/09/2020
            root["MacId"] = GW_macBuff;
            root["RSSI"] = tspk_rssi;
            root["LQI"] = tspk_lqi;
            root[snsrIdStore0] = GW_snsrDataBuff_0;     
            root[snsrIdStore1] = GW_snsrDataBuff_1;
        if(snsrIdStore[2] != 0x0)                                           //condition; if its zero it wont store in json  
        {
            root[snsrIdStore2] = GW_snsrDataBuff_2;
        }
        if(snsrIdStore[3] != 0x0)
        {
            root[snsrIdStore3] = GW_snsrDataBuff_3;
        }
            root["Sequence_number"] = seq_nr;                               // number indicates number of data's pushed to the events, whenever the system restarts, this also start from Zero
            root["SensorData Date&time"] = GW_get_time;
            root.printTo(GW_jsonMsg);           
            
            sprintf(GW_eventBuff, "SENSOR_MERGED_DATA_EVT");
            Particle.publish(GW_devName, GW_jsonMsg,PRIVATE);
    
            // client.connect("wisense");
            /* for customer dated 15SEP2020 -  INDRIYN   (don't use this)
            mqttServerConnect(GW_macBuff, "INDRIYN", "thinksea@123");
            client.publish("wisense/telemetry", GW_jsonMsg);
            client.disconnect();
            */
        
            // particle.publish(GW_jsonMsg,PRIVATE);
            //demo added 18DEC2020 internal HIH sensor node 1 fc:c2:3d:00:00:01:13:24

            if(softAP_flag == softAP_flag_default && strcmp(mqtt_usn_store,"NULL") == 0)
            {
                /* wisense dashboard credentials  */
                if (strcmp(GW_macBuff, "00:01:13:24") == 0) {
                mqttServerConnect(ACCESS_TOKEN_01, mqtt_pwd_store);
                client.publish(mqtt_topic_store, GW_jsonMsg);
                client.disconnect();
                }
             //demo added 18DEC2020 internal HIH sensor node 2 fc:c2:3d:00:00:01:13:31
                if (strcmp(GW_macBuff, "00:01:13:31") == 0) {
                mqttServerConnect(ACCESS_TOKEN_02, mqtt_pwd_store);
                client.publish(mqtt_topic_store, GW_jsonMsg);
                client.disconnect();
                }
            }
            
            if(softAP_flag == 0  && mqtt_select_flag == 2)
            {
                /* example use user-defined mqtt credentials*/
                mqttServerConnect(mqtt_usn_store, mqtt_pwd_store);
                client.publish(mqtt_topic_store, GW_jsonMsg);
                client.disconnect();
            }
            
            Particle.publish("MQTT Creditinals",GW_mqttMsg);        //test
            delay(50);
            /*clearing after pushed */
            snsrIdStore[0] = {0};
            snsrIdStore[1] = {0};
            snsrIdStore[2] = {0};
            snsrIdStore[3] = {0};
            /*
            snsrDataBuff[0] = {0};
            snsrDataBuff[1] = {0};
            snsrDataBuff[2] = {0};
            snsrDataBuff[3] = {0};
            snsrDataBuff[4] = {0};
            */
            
            merge_count=0;                                          //clearing merge count to set to start first
            digitalWrite(ledTxToCloud, LOW);
       }
   }
   
   GW_snsrDataBuff[0] = '\0';
   GW_snsrDataBuff_0[0] = '\0';
   GW_snsrDataBuff_1[0] = '\0';
   GW_snsrDataBuff_2[0] = '\0';
   GW_snsrDataBuff_3[0] = '\0';
   GW_snsrDataBuff_4[0] = '\0';
}

#endif



//1

#ifdef EVENT_FMT_TYPE_MERGED_DATA_STRING

/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
void GW_appendToEventBuffer(unsigned char *extAddr_p,
                            int snsrId,
                            char *unit_p,
                            float valF)
{
   int strLen = strlen(GW_snsrDataBuff);
   
   if (strLen == 0)
   { 
       strLen = sprintf(GW_snsrDataBuff, "ID_%02x%02x%02x%02x",            
                        extAddr_p[4], extAddr_p[5], extAddr_p[6], extAddr_p[7]);
   }
   
   sprintf(GW_snsrDataBuff + strLen,  "__S%d_V%.3f_U%s", snsrId, valF, unit_p);
}


/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void GW_appendIntToEventBuffer(unsigned char *extAddr_p,
                               int snsrId,
                               char *unit_p,
                               int valI)
{
   int strLen = strlen(GW_snsrDataBuff);
   
   if (strLen == 0)
   { 
       strLen = sprintf(GW_snsrDataBuff, "ID_%02x%02x%02x%02x",            
                        extAddr_p[4], extAddr_p[5], extAddr_p[6], extAddr_p[7]);
   }
   

   sprintf(GW_snsrDataBuff + strLen,  "__S%d_V%d_U%s", snsrId, valI, unit_p);
}


/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
void GW_sendMergedDataEvtToCloud(void)
{
   if (GW_relaySnsrDataToCloud == 0)
       return;
    
   if (strlen(GW_snsrDataBuff) > 0)
   {
       if (Particle.connected())
       {
           digitalWrite(ledTxToCloud, HIGH);       

           sprintf(GW_eventBuff, "SENSOR_MERGED_DATA_EVT");
           
           Particle.publish(GW_eventBuff, GW_snsrDataBuff);
           
           digitalWrite(ledTxToCloud, LOW);
           
       }
   }
   
   GW_snsrDataBuff[0] = '\0';
}

#endif

//#else


/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */

#ifdef EVENT_SEND_DATA_TO_CLOUD
void GW_sendDataToCloud(unsigned char *extAddr_p,
                        int snsrId,
                        char *unit_p,
                        float valF)
{
   if (GW_relaySnsrDataToCloud == 0)
       return;
       
   if (Particle.connected())
   {
        digitalWrite(ledTxToCloud, HIGH);
        
        //sprintf(GW_eventBuff, "NSDE_D%02x_U%s", 
               //snsrId, unit_p);
        sprintf(GW_devName, "WSN%02x%02x%02x%02x", 
               extAddr_p[4], extAddr_p[5], extAddr_p[6], extAddr_p[7]);
               
        sprintf(GW_macBuff, "%02x:%02x:%02x:%02x", 
               extAddr_p[4], extAddr_p[5], extAddr_p[6], extAddr_p[7]);
        sprintf(GW_snsrIdBuff, "0x%x" , snsrId);
        sprintf(GW_snsrDataBuff, "%.3f", valF);
        
        char GW_jsonMsg[1024];
        StaticJsonBuffer<1024> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["device"] = GW_devName;
        root["val"] = GW_snsrDataBuff;
        root["unit"] = unit_p;
        root["snsrId"] = GW_snsrIdBuff;
        //root["MacId"] = GW_macBuff;
        root.printTo(GW_jsonMsg); 
        
        
        Particle.publish(GW_devName, GW_jsonMsg , PRIVATE); //(<particle event name>, <json>, permission)
        
        serverConnect(ACCESS_TOKEN_01);
        client.publish("v1/devices/me/telemetry", GW_jsonMsg);
        client.disconnect();
        
        digitalWrite(ledTxToCloud, LOW);
   }
   else
   {
       // Serial.printf("Boron not connected to cloud !! \r\n");
   }
}

/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void GW_sendIntDataToCloud(unsigned char *extAddr_p,
                           int snsrId,
                           char *unit_p,
                           int valI)
{
   if (GW_relaySnsrDataToCloud == 0)
       return;
       
   if (Particle.connected())
   {
       digitalWrite(ledTxToCloud, HIGH);
       
       digitalWrite(ledTxToCloud, LOW);
   }
   else
   {
       // Serial.printf("Boron not connected to cloud !! \r\n");
   }
}

#endif

/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
void GW_sendTagBcnToCloud(unsigned int tagId,
                          int routerIdx,
                          int battV,
                          int bcnCntr,
                          int rssi)
{
   if (Particle.connected())
   {
       
       unsigned int nc = GW_getATUATTagBcnCnt(tagId) - 1;
       unsigned int skipCnt = GW_ATUAT_BCN_PUBLISH_SKIP_CNT;
       
       // Hack !!!
       // This tag's beacon was coming right after the beacon from another tag ... 
       // as a result this beacon was not getting published !!
       if (tagId == 163316)
           skipCnt --;
       
       // At bcn rate of once per  16 secs, too many beacons
       if ((nc % (skipCnt + 1)) != 0)
       {
           // Serial.printf("Skipping publish of bcn %d of %d from tag %d \r\n", 
           //               nc % (GW_ATUAT_BCN_PUBLISH_SKIP_CNT + 1), GW_ATUAT_BCN_PUBLISH_SKIP_CNT, tagId);
           return;
       }
       
       digitalWrite(ledTxToCloud, HIGH);
       sprintf(GW_eventBuff, "ATUAT_tag_battV_%d", tagId);
       sprintf(GW_snsrDataBuff, "%d", battV);
       // Serial.printf("Publishing (%s, %s) \r\n", GW_eventBuff, GW_snsrDataBuff);
       Particle.publish(GW_eventBuff, GW_snsrDataBuff);
       
       sprintf(GW_eventBuff, "ATUAT_tag_cntr_%d", tagId);
       sprintf(GW_snsrDataBuff, "%d", bcnCntr);
       // Serial.printf("Publishing (%s, %s) \r\n", GW_eventBuff, GW_snsrDataBuff);
       Particle.publish(GW_eventBuff, GW_snsrDataBuff);
       
       sprintf(GW_eventBuff, "ATUAT_tag_rtrIdx_%d", tagId);
       sprintf(GW_snsrDataBuff, "%d", routerIdx);
       // Serial.printf("Publishing (%s, %s) \r\n", GW_eventBuff, GW_snsrDataBuff);
       Particle.publish(GW_eventBuff, GW_snsrDataBuff);
       
       sprintf(GW_eventBuff, "ATUAT_tag_rssi_%d", tagId);
       sprintf(GW_snsrDataBuff, "%d", rssi);
       // Serial.printf("Publishing (%s, %s) \r\n", GW_eventBuff, GW_snsrDataBuff);
       Particle.publish(GW_eventBuff, GW_snsrDataBuff);
       
       sprintf(GW_eventBuff, "ATUAT_rtrId_%d", GW_atuatRouterArray[routerIdx]);
       sprintf(GW_snsrDataBuff, "%d", tagId);
       // Serial.printf("Publishing (%s, %s) \r\n", GW_eventBuff, GW_snsrDataBuff);
       Particle.publish(GW_eventBuff, GW_snsrDataBuff);       
       
       digitalWrite(ledTxToCloud, LOW);
   }
   else
   {
       // Serial.printf("Photon not connected to cloud !! \r\n");
   }
}


/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
void GW_processTagBcn(unsigned char *extAddr_p, unsigned char *buff_p, int msgLen)
{
   int tlvType = *(buff_p ++);
   int pyldLen = *(buff_p ++);
   int error = 0;

   if (tlvType == DIS_TLV_TYPE_ATUAT_TAG_BCN_INFO_LIST)
   {
       int idx, tlvCnt = 0;
    
       for (;;)
       {
           if (pyldLen >= (ATUAT_TAG_BCN_PDU_LEN + DIS_TLV_HDR_SZ))
           {
                int tlvType1 = *(buff_p ++);
                int pyldLen1 = *(buff_p ++);

                if (tlvType1 == DIS_TLV_TYPE_ATUAT_TAG_BCN_INFO
                    && pyldLen1 == ATUAT_TAG_BCN_PDU_LEN)
                {
                    int cntr = GW_ntohs(buff_p + ATUAT_TAG_BCN_PDU_CNTR_FIELD_OFF);
                    int vcc = GW_ntohs(buff_p +  ATUAT_TAG_BCN_PDU_VCC_FIELD_OFF);
                    int rssi = *((char *)(buff_p +  ATUAT_TAG_BCN_PDU_RSSI_FIELD_OFF));
                    unsigned int tagId, rtrId;
                    int routerIdx;
                    
                    // int ccaFlrCnt = GW_ntohs(buff_p + ATUAT_TAG_BCN_PDU_CCA_FLR_CNT_FIELD_OFF); 
                    // int intvSecs = GW_ntohs(buff_p + ATUAT_TAG_BCN_PDU_TX_INTERVAL_FIELD_OFF);
                
                    // printf("Bcn from TAG <0x%02x.0x%02x.0x%02x> CNTR<%u> VCC<%u mV> CCA_FLR<%u> INTV<%u secs> \n",
                    //        buff_p[0],  buff_p[1],  buff_p[2],   
                    //        cntr, vcc, ccaFlrCnt, intvSecs);
                    
                    
                    if (rssi > 127)
                        rssi = rssi - 256;

                    // Serial.printf("Bcn from TAG <0x%02x.0x%02x.0x%02x> CNTR<%u> VCC<%u mV> RSSI<%d dBm> \r\n",
                    //               buff_p[0],  buff_p[1],  buff_p[2],   
                    //               cntr, vcc, rssi);
                    
                    tagId = buff_p[0];
                    tagId <<= 8;
                    tagId |= buff_p[1];
                    tagId <<= 8;
                    tagId |= buff_p[2];
                    
                    rtrId = extAddr_p[LPWMN_MAC_EXT_ADDR_LEN - 1];
                    rtrId <<= 8;
                    rtrId |= extAddr_p[LPWMN_MAC_EXT_ADDR_LEN - 2];
                    rtrId <<= 8;
                    rtrId |= extAddr_p[LPWMN_MAC_EXT_ADDR_LEN - 3];
                   
                    routerIdx = GW_getATUATRouterIdx(rtrId);
                    if (routerIdx < 0)
                        routerIdx = GW_addATUATRouter(rtrId);
                    GW_sendTagBcnToCloud(tagId, routerIdx, vcc, cntr, rssi);
    
                    buff_p += ATUAT_TAG_BCN_PDU_LEN;
                    pyldLen -= (ATUAT_TAG_BCN_PDU_LEN + DIS_TLV_HDR_SZ);
                    
                    // Serial.printf("Done with TAG beacon .... \r\n");
                }
                else
                {
                    error = 1;
                    break;
                }
            }
            else
                break;
        }
    }
}

 const double RTD_A = (float)3.9083e-3;
 const double RTD_B = (float)-5.775e-7;
// /*
//  ********************************************************************
//  *
//  * Code from
//  * https://github.com/adafruit/Adafruit_MAX31865/blob/master/Adafruit_MAX31865.cpp
//  *
//  ********************************************************************
//  */
 double RTD_1000_processADCVal(unsigned int adcVal)
 {
   double RTDnominal = 1000, refResistor = 4300;
  double Z1, Z2, Z3, Z4, Rt, temp;

   // printf("\n adcVal : %u \n", adcVal);

   Rt = adcVal;
   Rt /= 32768;
   Rt *= refResistor;

   // printf("\n Rt:%f, RTD_A:%f, RTD_B:%f \n", Rt, RTD_A, RTD_B);

   Z1 = -RTD_A;
   Z2 = RTD_A * RTD_A - (4 * RTD_B);
   Z3 = (4 * RTD_B) / RTDnominal;
   Z4 = 2 * RTD_B;

   temp = Z2 + (Z3 * Rt);
   temp = (sqrt(temp) + Z1) / Z4;

   // printf("\n temp: %f \n", temp);

   if (temp >= 0) return temp;

   // ugh.
   Rt /= RTDnominal;
   Rt *= 100;      // normalize to 100 ohm

   float rpoly = Rt;

   temp = -242.02;
   temp += 2.2228 * rpoly;
   rpoly *= Rt;  // square
   temp += 2.5859e-3 * rpoly;
   rpoly *= Rt;  // ^3
   temp -= 4.8260e-6 * rpoly;
   rpoly *= Rt;  // ^4
   temp -= 2.8183e-8 * rpoly;
   rpoly *= Rt;  // ^5
   temp += 1.5243e-10 * rpoly;

   return temp;
 }
 
/*
 ********************************************************************
 *
 *
 *
 *
 ********************************************************************
 */
void GW_processNodeMsg(unsigned char *buff_p, int msgLen)
{
   int srcShortAddr, off = 0;
   unsigned int disMsgType;
   unsigned char *extAddr_p;
   
   GW_snsrDataBuff[0] = '\0';
   
   GW_rcvdMsgCnt ++;
   
   if (msgLen < LPWMN_MAC_SHORT_ADDR_LEN 
                + LPWMN_MAC_EXT_ADDR_LEN 
                + LPWMN_MSG_RSSI_LEN 
                + LPWMN_MSG_CORR_LQI_LEN)
       return;
       
   srcShortAddr = buff_p[off];
   srcShortAddr = (srcShortAddr << 8) | buff_p[off + 1];
   
   off += LPWMN_MAC_SHORT_ADDR_LEN;
 
#if 0       
   Serial.printf("[%u] Received msg from node <%05u / %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x> \r\n", 
                GW_rcvdMsgCnt,
                srcShortAddr, 
                (unsigned int)buff_p[off],
                (unsigned int)buff_p[off+1],
                (unsigned int)buff_p[off+2],
                (unsigned int)buff_p[off+3],
                (unsigned int)buff_p[off+4],
                (unsigned int)buff_p[off+5],
                (unsigned int)buff_p[off+6],
                (unsigned int)buff_p[off+7]);    
#endif

   extAddr_p = buff_p + off;
   buff_p += LPWMN_MAC_EXT_ADDR_LEN;
   
   LPWMN_updateNodeList(srcShortAddr, extAddr_p, 1);

   {
      int rssi;
      unsigned int lqi_corr;
      
      rssi = (signed char)buff_p[off];
      lqi_corr = buff_p[off + 1];
      
      tspk_rssi = rssi;
      tspk_lqi = lqi_corr;
      // Serial.printf("RSSI %d dBm / LQI %u \r\n", (int)rssi, lqi_corr);
   }
         
   off += (LPWMN_MSG_RSSI_LEN + LPWMN_MSG_CORR_LQI_LEN);

   msgLen -= (LPWMN_MAC_SHORT_ADDR_LEN 
              + LPWMN_MAC_EXT_ADDR_LEN 
              + LPWMN_MSG_RSSI_LEN 
              + LPWMN_MSG_CORR_LQI_LEN);
              
   if (msgLen < 1)
       return;
       
   disMsgType = buff_p[off];

   off += DIS_MSG_TYPE_SZ;
   msgLen -= DIS_MSG_TYPE_SZ;
   buff_p += off;
  
   
   if (msgLen <= 0xff)
   {
       unsigned char rc, tlvLen1, *buff1_p;
  
       if (disMsgType  == DIS_MSG_TYPE_ATUAT_TAG_BCN_INFO)
       {
           GW_processTagBcn(extAddr_p, buff_p, msgLen);
           
           return;
       }  
  
       
       rc = TLV_get(buff_p, msgLen, DIS_TLV_TYPE_SENSOR_OUTPUT_LIST, &tlvLen1, &buff1_p);
       if (rc == 0)
       {
           // Serial.printf("Could not find DIS_TLV_TYPE_SENSOR_OUTPUT_LIST !! \r\n");
           return;
       }        
       else
       {
           // Serial.printf("Found DIS_TLV_TYPE_SENSOR_OUTPUT_LIST \r\n");

           while (1)
           {
              unsigned char tlvLen2, *buff2_p;

              rc = TLV_get(buff1_p, tlvLen1, DIS_TLV_TYPE_SENSOR_OUTPUT, &tlvLen2, &buff2_p);
              if (rc == 0)
              {
                  // Serial.printf("Could not find another DIS_TLV_TYPE_SENSOR_OUTPUT TLV !! \r\n");
                  break;
              } 
              else
              {
                  unsigned char tlvLen3, *buff3_p;
                  int snsrId, scaleFactor = DIS_DATA_SCALE_CENTI;

                  // Make buff1_p point to the end of the current DIS_TLV_TYPE_SENSOR_OUTPUT TLV
                  buff1_p += (tlvLen2 + DIS_TLV_HDR_SZ);
                  if (tlvLen1 >= (tlvLen2 + DIS_TLV_HDR_SZ))
                      tlvLen1 -= (tlvLen2 + DIS_TLV_HDR_SZ);
                  else
                  {
                      // Serial.printf("\nMalformed TLVs in received message <%d/%d> \r\n !! \n", tlvLen1, tlvLen2 + DIS_TLV_HDR_SZ);
                      break;
                  }

                  // Serial.printf("Found DIS_TLV_TYPE_SENSOR_OUTPUT TLV .... val-fld-len<%d> \r\n", tlvLen2);
                  
                  rc = TLV_get(buff2_p, tlvLen2, DIS_TLV_TYPE_SENSOR_ID, &tlvLen3, &buff3_p);
                  if (rc == 0)
                      continue;
                  else
                  {
                      if (tlvLen3 == DIS_SENSOR_ID_FIELD_SZ) 
                      {
                          snsrId = *buff3_p;
                          
                          // Serial.printf("Sensor Id <0x%x> \r\n", snsrId);
                      }
                      else
                         continue;
                  }

                  rc = TLV_get(buff2_p, tlvLen2, DIS_TLV_TYPE_DATA_SCALE_FACTOR, &tlvLen3, &buff3_p);
                  if (rc)
                  {
                      if (tlvLen3 == DIS_DATA_SCALE_FACTOR_FIELD_SZ)
                      {
                          scaleFactor = *buff3_p;
                          // if (verbose)
                          //     Serial.printf("Found Scale factor <%d> \r\n", scaleFactor); 
                          if (!(scaleFactor >= DIS_DATA_SCALE_TERA && scaleFactor <= DIS_DATA_SCALE_FEMTO))
                               scaleFactor = DIS_DATA_SCALE_NONE;
                      }
                  }

                  rc = TLV_get(buff2_p, tlvLen2, DIS_TLV_TYPE_VALUE, &tlvLen3, &buff3_p);
                  if (rc == 0)
                      continue;
                  else
                  {
                      int snsrOp;
                      signed short snsrOp16;
                      char *unit_p = " ";

                      // Serial.printf("Found DIS_TLV_TYPE_VALUE TLV .... val-fld-len<%d> \r\n", tlvLen3);
       
                      switch(tlvLen3)
                      {
                         case 1:
                              snsrOp = (int)(*buff3_p);
                              break;
                    
                         case 2:
                              {
                                snsrOp16 = GW_ntohs(buff3_p);
                                snsrOp = snsrOp16;
                              }
                              break;
                              
                         case 3:
                              {
                                unsigned char lBuff[4];
                                lBuff[0] = 0;
                                memcpy(lBuff + 1, buff3_p, 3);
                                snsrOp = (int)GW_ntohl(lBuff);
                                // printf("\n <1b> snsrOp : %d \n", snsrOp);
                              }
                              break;
                              
                         case 4:
                              snsrOp = (int)GW_ntohl(buff3_p);
                              break;

                         default:
                              break;
                      }
           


                      switch (snsrId)
                      {
                         case PLTFRM_DUMMY_DEV_ID:
                              // Serial.printf("+[Sequence Nr]    ");
                              unit_p = "";
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              break;
                              
                         case PLTFRM_DO_SNSR_1_DEV_ID:      
                         case PLTFRM_ADS1015_1_DEV_ID:
                              // TODO - Remove PLTFRM_ADS1015_1_DEV_ID
                              unit_p = "uVolts";
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              break;
                              
                         case PLTFRM_PH_SNSR_1_DEV_ID:
                              unit_p = "mVolts";
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              break;
                              
                         case PLTFRM_DS18B20_1_DEV_ID:
                              unit_p = "Deg C";
                              scaleFactor = DIS_DATA_SCALE_100MICRO; 
                              break;
                              
                         case PLTFRM_ON_CHIP_VCC_SENSOR_DEV_ID:
                              // Serial.printf("+[Node_Voltage] ");
                              unit_p = "Volts";
                              break;

                         case PLTFRM_GEN_VOLT_MON_DEV_ID:
                              // Serial.printf("+[Ext Voltage]   ");
                              unit_p = "Volts";
                              break;

                         case PLTFRM_GEN_CURRENT_MON_DEV_ID:
                              // Serial.printf("+[Ext Current]   ");
                              unit_p = "mA";
                              break;
                         
                         case PLTFRM_AD7797_1_DEV_ID:
                              // Serial.printf("+[Load Cell]   ");
                              unit_p = "KG";
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              break;
 
                         case PLTFRM_MP3V5050GP_1_DEV_ID:
                              // Serial.printf("+[P_MP3V5050GP]   ");
                              unit_p = "kPa";
                              break;

                         case PLTFRM_MP3V5010_1_DEV_ID:
                              // Serial.printf("+[P_MP3V5010]   ");
                              unit_p = "%";
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              break;
 
                         case PLTFRM_MPXV5010G_1_DEV_ID:
                              // Serial.printf("+[P_MPXV5010G]   ");
                              unit_p = "mm of water";
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              break;

                         case PLTFRM_MP3V5004GP_1_DEV_ID:
                              // Serial.printf("+[P_MP3V5004GP]   ");
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              unit_p = "mv"; // "kPa";
                              break;

                         case PLTFRM_MPL115A2_1_DEV_ID:
                              // Serial.printf("+[P_MPL115A2]   ");
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              unit_p = "Pa";
                              break;

                         case PLTFRM_MS5637_1_DEV_ID:
                              // Serial.printf("+[P_MS5637]     ");
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              unit_p = "mbar";
                              break;

                         case PLTFRM_LLS_1_DEV_ID:
                              // Serial.printf("+[LLS_POT]   ");
                              scaleFactor = DIS_DATA_SCALE_DECI;
                              unit_p = "Ohms";
                              break;

                         case PLTFRM_LM75B_1_DEV_ID:
                              // Serial.printf("+[Temp_LM75B]   ");
                              unit_p = "Deg C";
                              break;

                         case PLTFRM_P43_US_1_DEV_ID:
                              // Serial.printf("+[P43_US]  ");
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              unit_p = "V";
                              break;

                         case PLTFRM_HPM_1_PM2PT5_DEV_ID:
                              // Serial.printf("+[HPM PM 2.5]   ");
                              unit_p = "";
                              break;

                         case PLTFRM_HPM_1_PM10_DEV_ID:
                              // Serial.printf("+[HPM PM 10]   ");
                              unit_p = "";
                              break;

                         case PLTFRM_AS339_1_DIVER_PRESSURE_DEV_ID: 
                              // Serial.printf("+[Diver Pressure x 10]  ");
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              unit_p = "cmH2O";
                              break;

                         case PLTFRM_AS339_1_DIVER_TEMPERATURE_DEV_ID: 
                              // Serial.printf("+[Diver Temperature]  ");
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              unit_p = "Deg C";
                              break;

                         case PLTFRM_AS339_1_DIVER_MOD_PRESSURE_DEV_ID: 
                              // Serial.printf("+[Diver-MOD Pressure x 10]  ");
                              scaleFactor = DIS_DATA_SCALE_DECI;
                              unit_p = "cmH2O";
                              break;

                         case PLTFRM_AS339_1_DIVER_MOD_TEMPERATURE_DEV_ID: 
                              // Serial.printf("+[Diver-MOD Temperature]  ");
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              unit_p = "Deg C";
                              break;

                         case PLTFRM_TEPT5700_1_DEV_ID:
                              // Serial.printf("+[Light_TEPT5700]   ");
                              unit_p = "uA";
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              break;

                         case PLTFRM_CC2D33S_1_RH_DEV_ID:
                              // Serial.printf("+[RH_CC2D33S]   ");
                              unit_p = "%";
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              break;

                         case PLTFRM_CC2D33S_1_TEMP_DEV_ID:
                              // Serial.printf("+[Temp_CC2D33S] ");
                              unit_p = "Deg C";
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              break;

                         case PLTFRM_DUAL_FS_LVL_MON_1_DEV_ID:
                              // Serial.printf("+[Dual_Float_Switch_Tank_Mon]  ");
                              unit_p = "";
                              break;

                         case PLTFRM_MAG3110_1_DEV_ID:
                              // Serial.printf("+[MFS_MAG3110]  ");
                              unit_p = "uT";
                              break;

                         case PLTFRM_WS_VEH_DET_1_DEV_ID:
                              // Serial.printf("+[WISENSE_VEH_MD]     ");
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              unit_p = "";
                              break;

                         case PLTFRM_MDS_1_DEV_ID:
                              // Serial.printf("+[Mains Monitor]  ");
                              unit_p = "";
                              break;
                         
                         case PLTFRM_VIBRATION_SNSR_1_DEV_ID:
                              // Serial.printf("+[Vibration Monitor]  ");
                              unit_p = "";
                              break;

                         case PLTFRM_EKMC160111X_1_DEV_ID:
                              // Serial.printf("+[Motion Sensed Count]     ");
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              unit_p = "";
                              break;
                          
                         case PLTFRM_PULSE_CNTR_2_DEV_ID:
                              // Serial.printf("+[Pulse Count (Rain Gauge)]  ");
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              unit_p = "";
                              break;

                         case PLTFRM_PULSE_CNTR_1_DEV_ID:
                              // Serial.printf("+[Pulse Count (Water Meter)]  ");
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              unit_p = "";
                              break;

                         case PLTFRM_SHT10_1_TEMP_DEV_ID:
                              // Serial.printf("+[Temp (SHT10)] ");
                              unit_p = "Deg C";
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              break;

                         case PLTFRM_SHT15_1_TEMP_DEV_ID:
                              // Serial.printf("+[Temp (SHT15)] ");
                              unit_p = "Deg C";
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              break;

                         case PLTFRM_SHT10_1_RH_DEV_ID:
                              // Serial.printf("+[RH (SHT10)]   ");
                              unit_p = "%";
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              break;

                         case PLTFRM_SHT15_1_RH_DEV_ID:
                              // Serial.printf("+[RH (SHT15)]   ");
                              unit_p = "%";
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              break;

                         case PLTFRM_SOL_PSU_1_VSOL_DEV_ID: 
                              // Serial.printf("+[SOL-PSU PANEL_V");
                              unit_p = "Volts";
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              break;

                         case PLTFRM_SOL_PSU_1_VBATT_DEV_ID: 
                              // Serial.printf("+[SOL-PSU BATT_V");
                              unit_p = "Volts";
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              break;

                         case PLTFRM_SOL_PSU_1_VSYS_DEV_ID: 
                              // Serial.printf("+[SOL-PSU SYS_V");
                              unit_p = "Volts";
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              break;

                         case PLTFRM_SOL_PSU_1_ISOL_DEV_ID: 
                              // Serial.printf("+[SOL-PSU PANEL_I");
                              unit_p = "mA";
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              break;

                         case PLTFRM_SOL_PSU_1_IBATT_DEV_ID: 
                              // Serial.printf("+[SOL-PSU BATT_I");
                              unit_p = "mA";
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              break;

                         case PLTFRM_INA219_1_BV_DEV_ID:
                              // Serial.printf("+[INA219_BUS_V]  ");
                              unit_p = "milli-volts";
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              break;
                         
                         case PLTFRM_INA219_3_BV_DEV_ID:
                              // Serial.printf("+[INA219_BUS_V]  ");
                              unit_p = "milli-volts";
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              break;
                         
                         case PLTFRM_ACS712_1_CURRENT_DEV_ID:
                              // Serial.printf("+[ACS712_1_S_V]  ");
                              unit_p = "milli-volts";
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              break;

                         case PLTFRM_ACS712_2_CURRENT_DEV_ID:
                              // Serial.printf("+[ACS712_2_S_V]  ");
                              unit_p = "milli-volts";
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              break;

                         case PLTFRM_ACS712_3_CURRENT_DEV_ID:
                              // Serial.printf("+[ACS712_3_S_V]     ");
                              unit_p = "milli-volts";
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              break;

                         case PLTFRM_INA219_1_SV_DEV_ID:
                              // Serial.printf("+[INA219_CURRENT]   ");
                              unit_p = "milli-amps";
                              break;

                         case PLTFRM_INA219_3_SV_DEV_ID:
                              // Serial.printf("+[INA219_CURRENT]   ");
                              unit_p = "milli-amps";
                              break;

                         case PLTFRM_NTCALUG02A_1_DEV_ID:
                              // Serial.printf("+[NTC_THERM (Vishay NTCALUG02A)]   ");
                              unit_p = "deg C";
                              break;
                              
                         case PLTFRM_APS_1_DEV_ID:
                              unit_p = "milli-psi";
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              break;

                         case PLTFRM_NXFT15XH103_1_DEV_ID:
                         case PLTFRM_NXFT15XH103_2_DEV_ID:
                              // Serial.printf("+[NTC_THERM (Murata NXFT15XH103)]   ");
                              unit_p = "deg C";
                              break;

                         case PLTFRM_DNAX300R103L040_1_DEV_ID:
                              // Serial.printf("+[NTC_THERM (Deem DNAX300R103L040)]   ");
                              unit_p = "deg C";
                              break;
                         
                         case PLTFRM_NTCALUG02A_2_DEV_ID:
                              // Serial.printf("+[NTC_THERM (Vishay NTCALUG02A)]   ");
                              unit_p = "deg C";
                              break;
                               
                         case PLTFRM_BAR_CODE_SCANNER_1_DEV_ID:
                              // Serial.printf("+[Bar Code]     ");
                              unit_p = "";
                              break;

                         case PLTFRM_MAX_SONAR_1_DEV_ID:
                              // Serial.printf("+[Dist_MaxSonar_1 ]  ");
                              unit_p = "Inches";
                              break;

                         case PLTFRM_MAX_SONAR_2_DEV_ID:
                              // Serial.printf("+[Dist_MaxSonar_2 ]  ");
                              unit_p = "Inches";
                              break;
                         
                         case PLTFRM_MAX_SONAR_3_DEV_ID:
                              // Serial.printf("+[Dist_MaxSonar_3 ]  ");
                              unit_p = "Inches";
                              break;
                         
                         case PLTFRM_CHIRP_PWLA_1_DEV_ID:
                              // Serial.printf("+[Moisture_CHIRP] ");
                              unit_p = "na";
                              break;

                         case PLTFRM_WSMS100_1_DEV_ID:
                              // Serial.printf("+[Moisture_WSMS100] ");
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              unit_p = "%";
                              break;

                         case PLTFRM_ON_CHIP_TEMP_SENSOR_DEV_ID:
                              // Serial.printf("+[Temp_MSP430]  ");
                              unit_p = "deg C";
                              break;

                         case PLTFRM_TSL45315_1_DEV_ID:
                              // Serial.printf("+[Light_TSL45315] ");
                              unit_p = "Lux";
                              break;

                         case PLTFRM_BATT_1_DEV_ID:
                              // Serial.printf("+[Batt Voltage] ");
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              unit_p = "Volts";
                              break;

                         case PLTFRM_SETRA_3100_1_DEV_ID:
                              // Serial.printf("+[SETRA 3100 Op Voltage] ");
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              unit_p = "Volts";
                              break;

                         case PLTFRM_DEV_TYPE_SOLAR_PWR_SRC_VSENSE:
                              // Serial.printf("+[Panel Voltage] ");
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              unit_p = "Volts";
                              break;

                         case PLTFRM_EXT_VOLTAGE_MON_DEV_ID:
                              // Serial.printf("+[Ext Voltage] ");
                              scaleFactor = DIS_DATA_SCALE_MILLI;
                              unit_p = "Volts";
                              break;

                         case PLTFRM_HE055T01_1_DEV_ID:
                              // Serial.printf("+[Current_HE055T01] ");
                              unit_p = "mA";
                              break;

                         case PLTFRM_WPDS_DEV_ID:
                              // Serial.printf("+[WPDS ALARM]  ");
                              break;
                        
                         case PLTFRM_TMP75C_1_DEV_ID:
                              unit_p = "Deg C";
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              break;
                              
                         case PLTFRM_HIH8121_1_RH_DEV_ID:                                       //added
                              //printf("+[RH_HIH8121]   ");
                              unit_p = "%";
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              break;
                              
                         case PLTFRM_HIH8121_1_TEMP_DEV_ID:
                              //printf("+[Temp_HIH8121] ");
                              unit_p = "Deg C";
                              scaleFactor = DIS_DATA_SCALE_CENTI;
                              break;
                        /*
                        case PLTFRM_RTD_1000_1_DEV_ID:
                             // printf("+[Temp_RTD_1000] ");
                              unit_p = "deg C";
                              scaleFactor = DIS_DATA_SCALE_NONE;
                              break;

                        /*case PLTFRM_DS18B20_1_DEV_ID:
                              printf("+[Temp_DS18B20] ");
                              unit_p = "Deg C";
                              scaleFactor = DIS_DATA_SCALE_TENTH_MILLI;
                              break;
                        */
                         default:
                              // Serial.printf("[Unknown]");
                              break;
                      }


                      if (snsrId == PLTFRM_GEN_VOLT_MON_DEV_ID
                          || snsrId == PLTFRM_ON_CHIP_VCC_SENSOR_DEV_ID)
                          scaleFactor = DIS_DATA_SCALE_MILLI;
                      
                      if (snsrId == PLTFRM_GEN_CURRENT_MON_DEV_ID)
                          scaleFactor = DIS_DATA_SCALE_NONE;

                      if (snsrId == PLTFRM_LM75B_1_DEV_ID
                          || snsrId == PLTFRM_ADS1015_1_DEV_ID
                          || snsrId == PLTFRM_DO_SNSR_1_DEV_ID
                          || snsrId == PLTFRM_P43_US_1_DEV_ID
                          || snsrId == PLTFRM_HPM_1_PM2PT5_DEV_ID
                          || snsrId == PLTFRM_HPM_1_PM10_DEV_ID
                          || snsrId == PLTFRM_AS339_1_DIVER_PRESSURE_DEV_ID
                          || snsrId == PLTFRM_AS339_1_DIVER_MOD_PRESSURE_DEV_ID
                          || snsrId == PLTFRM_AS339_1_DIVER_TEMPERATURE_DEV_ID
                          || snsrId == PLTFRM_AS339_1_DIVER_MOD_TEMPERATURE_DEV_ID
                          || snsrId == PLTFRM_BATT_1_DEV_ID
                          || snsrId == PLTFRM_SETRA_3100_1_DEV_ID
                          || snsrId == PLTFRM_AD7797_1_DEV_ID
                          || snsrId == PLTFRM_GEN_CURRENT_MON_DEV_ID
                          || snsrId == PLTFRM_GEN_VOLT_MON_DEV_ID
                          || snsrId == PLTFRM_ON_CHIP_VCC_SENSOR_DEV_ID
                          || snsrId == PLTFRM_ON_CHIP_TEMP_SENSOR_DEV_ID
                          || snsrId == PLTFRM_MP3V5050GP_1_DEV_ID
                          || snsrId == PLTFRM_MP3V5010_1_DEV_ID
                          || snsrId == PLTFRM_MPXV5010G_1_DEV_ID
                          || snsrId == PLTFRM_MP3V5004GP_1_DEV_ID
                          || snsrId == PLTFRM_FC_28_1_DEV_ID
                          || snsrId == PLTFRM_ACS712_1_CURRENT_DEV_ID
                          || snsrId == PLTFRM_ACS712_2_CURRENT_DEV_ID
                          || snsrId == PLTFRM_LLS_1_DEV_ID
                          || snsrId == PLTFRM_ACS712_3_CURRENT_DEV_ID
                          || snsrId == PLTFRM_SHT10_1_RH_DEV_ID
                          || snsrId == PLTFRM_SHT10_1_TEMP_DEV_ID
                          || snsrId == PLTFRM_SHT15_1_TEMP_DEV_ID
                          || snsrId == PLTFRM_SHT15_1_RH_DEV_ID
                          || snsrId == PLTFRM_WSMS100_1_DEV_ID
                          || snsrId == PLTFRM_EXT_VOLTAGE_MON_DEV_ID
                          || snsrId == PLTFRM_DEV_TYPE_SOLAR_PWR_SRC_VSENSE
                          || snsrId == PLTFRM_CC2D33S_1_RH_DEV_ID
                          || snsrId == PLTFRM_CC2D33S_1_TEMP_DEV_ID
                          || snsrId == PLTFRM_TEPT5700_1_DEV_ID
                          || snsrId == PLTFRM_SOL_PSU_1_VSOL_DEV_ID
                          || snsrId == PLTFRM_SOL_PSU_1_VBATT_DEV_ID
                          || snsrId == PLTFRM_SOL_PSU_1_VSYS_DEV_ID
                          || snsrId == PLTFRM_MAX_SONAR_1_DEV_ID
                          || snsrId == PLTFRM_MAX_SONAR_2_DEV_ID
                          || snsrId == PLTFRM_MAX_SONAR_3_DEV_ID
                          || snsrId == PLTFRM_DS18B20_1_DEV_ID
                          || snsrId == PLTFRM_PH_SNSR_1_DEV_ID
                          || snsrId == PLTFRM_APS_1_DEV_ID
                          || snsrId == PLTFRM_TMP75C_1_DEV_ID
                          || snsrId == PLTFRM_HIH8121_1_RH_DEV_ID
                          || snsrId == PLTFRM_HIH8121_1_TEMP_DEV_ID
                          || snsrId == PLTFRM_DS18B20_1_DEV_ID
                          || snsrId == PLTFRM_RTD_1000_1_DEV_ID)
                      {
                          float valF = snsrOp;
                          
                          switch (scaleFactor)
                          {
                             case DIS_DATA_SCALE_MICRO:
                                  valF /= 1000;
                                  valF /= 1000;
                                  break;
                                  
                             case DIS_DATA_SCALE_MILLI:
                                  valF /= 1000;
                                  break;

                             case DIS_DATA_SCALE_CENTI:
                                  valF /= 100;
                                  break;

                             case DIS_DATA_SCALE_DECI:
                                  valF /= 10;
                                  break;
                                  
                             case DIS_DATA_SCALE_100MICRO:
                                  valF /= 10000;
                                  break;

                             default:
                                  break;
                          }

                          if (snsrId == PLTFRM_AD7797_1_DEV_ID)
                          { 
                          }
                          
                          if (snsrId == PLTFRM_RTD_1000_1_DEV_ID)
                            {
                                     // if (tlvLen3 == 4)
                                              //{
                                              int val;
                                              double tempVal = RTD_1000_processADCVal(snsrOp);
                                              valF=tempVal;

#ifdef EVENT_FMT_TYPE_MERGED_DATA_JSON
                                              GW_appendIntToEventBufferJson(extAddr_p, snsrId, "", val);
#endif

#ifdef EVENT_FMT_TYPE_MERGED_DATA_STRING
                                              GW_appendIntToEventBuffer(extAddr_p, snsrId, "", val);
#endif
#ifdef EVENT_SEND_DATA_TO_CLOUD
                                              GW_sendIntDataToCloud(extAddr_p, snsrId, "", val);
#endif
                                              //printf("<%f %s> \n", tempVal, unit_p);
                                              //}
                          }
                                                              
                                                             
                                                             
                          if (snsrId == PLTFRM_MP3V5004GP_1_DEV_ID)
                          {
                              if (__latestVccSet)
                              {
                                  float currRatio, delta;
                                  float startRatio = 0.527289;

                                  // Vout = Vs*(0.2*p + 0.2) +/- (2.5 % of VFSS)

                                  // p = ((Vout / Vs) - 0.2) * 5
                                  // Inches of water = p * 101.97


                                  currRatio = valF;
                                  currRatio /= __latestVcc;

                                  delta = startRatio;
                                  delta -= currRatio;
                                  delta /= startRatio;
                                  delta *= 100;

                                  // temp /= __latestVcc;
                                  // temp -= 0.2187;   // -0.2
                                  // temp /= .176;  // / 0.2
                                  // temp *= 101.97;  // in mm of water

                                  // printf(" <%f mm of water> \n", temp); 
                  
                                  // Serial.printf("Adc: %f V / Vcc: %f V / ratio: %f / delta: %f percent \r\n", 
                                  //               valF, __latestVcc, currRatio, delta);
                              }
                              else
                              {
                                  // Serial.printf(" <%f %s> \r\n", valF, "milli-volts");
                              }
                          }
                          

                          if (snsrId != PLTFRM_AD7797_1_DEV_ID
                              && snsrId != PLTFRM_MP3V5050GP_1_DEV_ID
                              && snsrId != PLTFRM_MP3V5004GP_1_DEV_ID
                              // && snsrId != PLTFRM_ON_CHIP_VCC_SENSOR_DEV_ID
                              )
                          {
                              // Serial.printf(" <%f %s> \r\n", valF, unit_p);
#ifdef EVENT_FMT_TYPE_MERGED_DATA_JSON
                              GW_appendToEventBufferJson(extAddr_p, snsrId, unit_p, valF);  
#endif 

#ifdef EVENT_FMT_TYPE_MERGED_DATA_STRING
                              GW_appendToEventBuffer(extAddr_p, snsrId, unit_p, valF);
#endif
#ifdef EVENT_SEND_DATA_TO_CLOUD
                              GW_sendDataToCloud(extAddr_p, snsrId, unit_p, valF);
#endif
                          }
                          
                          if (snsrId == PLTFRM_ON_CHIP_VCC_SENSOR_DEV_ID)
                          {
                              __latestVcc = valF;
                              __latestVccSet = 1;
                          }
                      }
                      else
                      {
                          if (snsrId == PLTFRM_BAR_CODE_SCANNER_1_DEV_ID)
                          {
                              buff3_p[tlvLen3] = '\0';
                              // Serial.printf(" <%s %s> \r\n", buff3_p, unit_p);
                          }
                          else
                          {
                              if (snsrId == PLTFRM_NTCALUG02A_1_DEV_ID
                                  || snsrId == PLTFRM_NTCALUG02A_2_DEV_ID
                                  || snsrId == PLTFRM_NXFT15XH103_1_DEV_ID
                                  || snsrId == PLTFRM_NXFT15XH103_2_DEV_ID
                                  || snsrId == PLTFRM_DNAX300R103L040_1_DEV_ID)
                              {
                                  double _r, _rl;
                                  double t25 = 25 + 273.15;
                                  double r_t = (double)snsrOp; // 6794;
                                  int b25by85;

                                  switch (snsrId)
                                  {
                                     case PLTFRM_NTCALUG02A_1_DEV_ID:
                                     case PLTFRM_NTCALUG02A_2_DEV_ID:
                                          _r = NTC_THERM_NTCALUG02A_R25_VAL;
                                          b25by85 = NTC_THERM_NTCALUG02A_B_25_85_VAL;
                                          break;

                                     case PLTFRM_DNAX300R103L040_1_DEV_ID:
                                          _r =  NTC_THERM_DNAX300R103L040_4_R25_VAL;
                                          b25by85 = NTC_THERM_DNAX300R103L040_4_B_25_85_VAL;
                                          break;

                                     case PLTFRM_NXFT15XH103_1_DEV_ID:
                                     case PLTFRM_NXFT15XH103_2_DEV_ID:
                                          _r = NTC_THERM_NXFT15XH103_R25_VAL;
                                          b25by85 = NTC_THERM_NXFT15XH103_B_25_85_VAL;
                                          break;

                                     default:
                                          break;
                                  }

                                  // loge(R25/RT) = B * (1/T25 - 1/T)
                                  // 1/T = 1/T25 - loge(R25/RT)/B
                                  // T = 1 / (1/T25 - loge(R25/RT)/B)
    
                                  _r /= r_t;
                                  _rl = log(_r);
                                  _rl /= b25by85;
                                  _r = 1;
                                  _r /= t25;
                                  _r -= _rl;
                                  _r = (1 / _r);
                                  
                                  // Serial.printf(" <%.2f %s> \r\n", _r - 273.15, unit_p);
#ifdef EVENT_FMT_TYPE_MERGED_DATA_JSON
                                  GW_appendToEventBufferJson(extAddr_p, snsrId, unit_p, _r - 273.15);
#endif

#ifdef EVENT_FMT_TYPE_MERGED_DATA_STRING
                                  GW_appendToEventBuffer(extAddr_p, snsrId, unit_p, _r - 273.15);
#endif

#ifdef EVENT_SEND_DATA_TO_CLOUD
                                  GW_sendDataToCloud(extAddr_p, snsrId, unit_p, _r - 273.15);
#endif
                              }
                              else
                              {
                                  if (snsrId == PLTFRM_INA219_1_SV_DEV_ID
                                      || snsrId == PLTFRM_INA219_3_SV_DEV_ID
                                      || snsrId == PLTFRM_SOL_PSU_1_ISOL_DEV_ID
                                      || snsrId == PLTFRM_SOL_PSU_1_IBATT_DEV_ID)
                                  {
                                      // Reported value is voltage in mv * 100
                                      float opVal = (int)snsrOp;
                                      opVal /= 100;
                                      // I = V/R
                                      if (snsrId == PLTFRM_SOL_PSU_1_ISOL_DEV_ID
                                          || snsrId == PLTFRM_SOL_PSU_1_IBATT_DEV_ID)
                                          opVal *= 10; // shunt resistance of 0.1 ohms
                                      else
                                          opVal *= 1;  // shunt resistance of 1 ohms
                                          
                                      // Serial.printf(" <%f %s> \r\n", opVal, unit_p);
#ifdef EVENT_FMT_TYPE_MERGED_DATA_JSON
                                      GW_appendToEventBufferJson(extAddr_p, snsrId, unit_p, opVal); 
#endif

#ifdef EVENT_FMT_TYPE_MERGED_DATA_STRING
                                      GW_appendToEventBuffer(extAddr_p, snsrId, unit_p, opVal);
#endif
#ifdef EVENT_SEND_DATA_TO_CLOUD
                                      GW_sendDataToCloud(extAddr_p, snsrId, unit_p, opVal);
#endif
                                  }
                                  else
                                  {
                                      if (snsrId == PLTFRM_MDS_1_DEV_ID)
                                      {
                                          // Serial.printf(" <%s> \r\n", snsrOp ? "On" : "Off");
                                      }
                                      else
                                      {
                                          if (snsrId == PLTFRM_VIBRATION_SNSR_1_DEV_ID)
                                          {     
                                              // Serial.printf(" <%s> \r\n", snsrOp ? "Yes" : "No");
                                          }
                                          else 
                                          { 
                                              if (snsrId == PLTFRM_MAG3110_1_DEV_ID)
                                              {
                                                  if (tlvLen3 == 6)
                                                  {
                                                      short magFldInt = (short)GW_ntohs(buff3_p);
                                                      float magFldIntF = magFldInt;

                                                      magFldIntF *= 0.1;
                                                      // Serial.printf("x: %f uT, ", magFldIntF);

                                                      magFldInt = (short)GW_ntohs(buff3_p + 2);
                                                      magFldIntF = magFldInt;
                                                      magFldIntF *= 0.1;
                                                      // Serial.printf("y: %f uT, ", magFldIntF);

                                                      magFldInt = (short)GW_ntohs(buff3_p + 4);
                                                      magFldIntF = magFldInt;
                                                      magFldIntF *= 0.1;
                                                      // Serial.printf("z: %f uT ::: \n", magFldIntF);

                                                      // short totalDelta = (short)GW_ntohs(buff3_p + 6);
                                                      // float totalDeltaF = totalDelta;
                                                      // totalDeltaF *= 0.1;

                                                      // printf("tot-abs-delta: %f uT \n", totalDeltaF);
                                                  }
                                                  else 
                                                  {
                                                      // Serial.printf("\n tlvLen3 : %d  \r\n",  tlvLen3);
                                                  }
                                              }
                                              else
                                              {
                                                  if (snsrId == PLTFRM_DUAL_FS_LVL_MON_1_DEV_ID)
                                                  {
                                                      if (tlvLen3 == 1)
                                                      {
                                                          unsigned char byte = *buff3_p;
                                                          unsigned char alertType;
                                                          unsigned char upperSw;
                                                          unsigned char lowerSw;
  
                                                          // printf("\n 0x%02x \n", byte);
                                                          
                                                          alertType = byte & 0x3;
                                                          byte >>= 2;
                                                          lowerSw =  byte & 0x3;
                                                          byte >>= 2;
                                                          upperSw =  byte & 0x3;

                                                          // printf("\n 0x%02x \n", byte);

                                                          Serial.printf("Upper-Sw<%s> Lower-Sw<%s> Alert<%s> \r\n",
                                                                 upperSw == 1 ?  "Open" :  "Closed",             
                                                                 lowerSw == 1 ?  "Open" :  "Closed",      
                                                                 alertType ==  0  ? "None" : \
                                                                 alertType ==  1  ? "High **************" : \
                                                                 alertType ==  2  ? "Low **************" : "Invalid **************");
         
                                                      }
                                                      else 
                                                          Serial.printf("\n tlvLen3 : %d  !!\r\n",  tlvLen3);
                                                                      
                                                  }
                                                  else
                                                  {
                                                      if (snsrId == PLTFRM_WPDS_DEV_ID)
                                                      {                                
                                                          if (tlvLen3 >= (1 + 2 + 1 + 3))
                                                          {
                                                              int channId, battV, rssi;

                                                              channId = *(buff3_p ++);
                                                              battV = GW_ntohs(buff3_p);
                                                              buff3_p += 2;
                                                              rssi = (int)(*((char *)(buff3_p ++)));
                                                              // Serial.printf("<Sender (0x%02x:0x%02x:0x%02x) ",
                                                              //               buff3_p[0], buff3_p[1],  buff3_p[2]);
                                                              // Serial.printf("/ RSSI(%d) / BattV(%d mV) / Channel(%d)> \r\n",
                                                              //               rssi, battV, channId + 1);
                                                          }
                                                          else
                                                              Serial.printf("Malformed event !! \r\n");
                                                      }    
                                                      else                              
                                                      {                                
                                                          if (snsrId == PLTFRM_AUTO_ASSY_TMON_1_DEV_ID)
                                                          {
                                                              int idx;
                                                              for (idx=0; idx<2; idx++)
                                                              {
                                                                   // Serial.printf("Socket [%d] <%s> \r\n", 
                                                                   //     idx + 1, (snsrOp & (1 << idx)) ? "Empty" : "Plugged-In");
                                                              }
                                                          }
                                                          else
                                                          {
                                                                if (snsrId == PLTFRM_RTD_1000_1_DEV_ID)
                                                                {
                                                                 // if (tlvLen3 == 4)
                                                                  //{
                                                                     // double tempVal = RTD_1000_processADCVal(snsrOp);          //add if any rtd related modification in EventBuffer
                                                                      //val=tempVal;
#ifdef EVENT_FMT_TYPE_MERGED_DATA_JSON
                                                                //      GW_appendIntToEventBufferJson(extAddr_p, snsrId, "", val);
#endif

#ifdef EVENT_FMT_TYPE_MERGED_DATA_STRING
                                                                  //    GW_appendIntToEventBuffer(extAddr_p, snsrId, "", val);
#endif
#ifdef EVENT_SEND_DATA_TO_CLOUD
                                                                    //  GW_sendIntDataToCloud(extAddr_p, snsrId, "", val);
#endif
                                                                      //printf("<%f %s> \n", tempVal, unit_p);
                                                                  //}
                                                              }
                                                              
                                                             
                                                              if (snsrId == PLTFRM_GPIO_REMOTE_CTRL_DEV_ID)
                                                              {
                                                                  if (tlvLen3 == 3)
                                                                  {
                                                                      int val = ((*(buff3_p + 0)) + 1)*100;
                                                                      val += ((*(buff3_p + 1))*10);
                                                                      val += (*(buff3_p + 2));


#ifdef EVENT_FMT_TYPE_MERGED_DATA_JSON
                                                                      GW_appendIntToEventBufferJson(extAddr_p, snsrId, "", val); 
#endif

#ifdef EVENT_FMT_TYPE_MERGED_DATA_STRING
                                                                      GW_appendIntToEventBuffer(extAddr_p, snsrId, "", val);
#endif
#ifdef EVENT_SEND_DATA_TO_CLOUD
                                                                      GW_sendIntDataToCloud(extAddr_p, snsrId, "", val);
#endif
                                                                  }
                                                              }
                                                              else
                                                              {
#if 0                                                              
                                                                  if (strlen(unit_p) > 0)  
                                                                      Serial. printf(" <%d %s> \r\n", snsrOp, unit_p);
                                                                  else
                                                                      Serial.printf(" <%d> \r\n", snsrOp);
#endif
                                                              }
                                                          }
                                                      }    
                                                  }
                                              }
                                          }   
                                      }
                                  }
                              }
                          }
                      }
                  }
              } // SENSOR_OP TLV found 
          }  // while (1)
       }
   }

/*   
#ifdef EVENT_FMT_TYPE_MERGED_DATA_STRING
   GW_sendMergedDataEvtToCloud();
#else
    GW_sendMergedDataEvtToCloudJson();
#endif
*/
#ifdef EVENT_FMT_TYPE_MERGED_DATA_STRING
   GW_sendMergedDataEvtToCloud();
#endif
#ifdef EVENT_FMT_TYPE_MERGED_DATA_JSON
    GW_sendMergedDataEvtToCloudJson();
#endif
    



   return;
}


/*
 ****************************************************************************************
 *
 *
 *
 *
 ****************************************************************************************
 */
void GW_processRcvdMsg(unsigned int msgType, unsigned char *pyld_p, int pyldLen)
{
    switch (msgType)
    {
       case LPWMN_GW_MSG_TYPE_RELAY_FROM_NODE:
            {
               digitalWrite(ledRxFromCoord, HIGH);
               GW_processNodeMsg(pyld_p, pyldLen);
               digitalWrite(ledRxFromCoord, LOW);
#ifdef GW_COORD_HEALTH_MON               
               if (GW_coordState != GW_COORD_STATE_UP)
               {
                   GW_coordState = GW_COORD_STATE_UP;
                   GW_coordStateEvtPending = 1;
                   GW_lpwmnCoordUpCnt ++;
               }
               
               GW_restartKATimer();
#endif               
            }
            break;

       case LPWMN_GW_MSG_TYPE_NODE_CNT_REQ:
            {
               if (pyldLen == 2)
               {
                   int nodeCnt = GW_ntohs(pyld_p);
                   Serial.printf("Number of nodes in the network - %d\r\n", nodeCnt);
               }
            }
            break;
            
       default:
            break;
    }
    
    return;
}



/*
 ****************************************************************************************
 *
 *
 *
 *
 ****************************************************************************************
 */
void LOCAL_buildMsgHdr(unsigned short msgType, 
                       unsigned char *buff_p, 
                       unsigned short pyldLen)
{
   static unsigned char __seqNr = 0; 
   unsigned short calcCrc;

   GW_htons(buff_p, msgType);

   buff_p[UART_FRAME_HDR_FLAGS_FIELD_OFF] = 0x0;

   buff_p[UART_FRAME_HDR_SEQ_NR_FIELD_OFF] = __seqNr;
   __seqNr ++;

   GW_htons(buff_p + UART_FRAME_HDR_PYLD_LEN_FIELD_OFF, pyldLen);

   calcCrc = __crc16(buff_p, UART_FRAME_HDR_LEN - UART_FRAME_HDR_CRC_FIELD_LEN*2);
   GW_htons(buff_p + UART_FRAME_HDR_HDR_CRC_FIELD_OFF, calcCrc);

   if (pyldLen > 0)
       calcCrc = __crc16(buff_p + UART_FRAME_HDR_LEN, pyldLen);
   else
       calcCrc = 0x0;
   GW_htons(buff_p + UART_FRAME_HDR_PYLD_CRC_FIELD_OFF, calcCrc);

   return;
}


/*
 ****************************************************************************************
 *
 *
 *
 *
 ****************************************************************************************
 */
void LOCAL_buildSendResp(const unsigned short msgType, 
                         const char *respVal_p, 
                         const unsigned short pyldLen)
{
    unsigned char *buff_p = USB_GW_APP_txBuff;
    
    // Fill in the payload portion
    switch (pyldLen)
    {
        case 1:
           *(buff_p + UART_FRAME_HDR_LEN) = *(respVal_p);
           break;

        case 2:
           GW_htons(buff_p + UART_FRAME_HDR_LEN, *((unsigned short *)respVal_p));
           break;

        case 4:
           GW_htonl(buff_p + UART_FRAME_HDR_LEN, *((unsigned int *)respVal_p));
           break;

        default:
           memcpy(buff_p + UART_FRAME_HDR_LEN, respVal_p, pyldLen);
           break;
    }

    LOCAL_buildMsgHdr(msgType, USB_GW_APP_txBuff, pyldLen);
    
    Serial.write(USB_GW_APP_txBuff, UART_FRAME_HDR_LEN + pyldLen);
}



/*
 ****************************************************************************************
 *
 *
 *
 *
 ****************************************************************************************
 */
void interceptUSBAppCmds(unsigned char rxByte)
{
    USB_GW_APP_rxBuff[USB_GW_APP_uartRxCnt] = rxByte;
    
    // Serial.printf("Read byte idx[%02d] - 0x%02x \r\n", GW_uartRxCnt, rxByte);
    
    USB_GW_APP_uartRxCnt ++;
    
    if (USB_GW_APP_uartRxCnt == UART_FRAME_HDR_PYLD_CRC_FIELD_OFF)
    {
        unsigned short calcCrc16, rxdCrc16;
        
        // Serial.printf("Read %d bytes \r\n", USB_GW_APP_uartRxCnt);
        
        calcCrc16 = __crc16(USB_GW_APP_rxBuff, UART_FRAME_HDR_HDR_CRC_FIELD_OFF);
        rxdCrc16 = USB_GW_APP_rxBuff[UART_FRAME_HDR_HDR_CRC_FIELD_OFF];
        rxdCrc16 = (rxdCrc16 << 8) + USB_GW_APP_rxBuff[UART_FRAME_HDR_HDR_CRC_FIELD_OFF + 1];       
        
        if (calcCrc16 != rxdCrc16)
        {
            int idx;
            // Serial.printf("Hdr CRC mismatch <0x%x/0x%x> !!  \r\n", rxdCrc16, calcCrc16);
            for (idx=0; idx<UART_FRAME_HDR_PYLD_CRC_FIELD_OFF-1; idx++)
                 USB_GW_APP_rxBuff[idx] = USB_GW_APP_rxBuff[idx+1];
            USB_GW_APP_uartRxCnt = UART_FRAME_HDR_PYLD_CRC_FIELD_OFF - 1;
        }
        else
        {
            // Serial.printf("Hdr CRC matches <0x%x/0x%x> \r\n", rxdCrc16, calcCrc16);
            USB_GW_APP_msgPyldLen = USB_GW_APP_rxBuff[UART_FRAME_HDR_PYLD_LEN_FIELD_OFF];
            USB_GW_APP_msgPyldLen = (USB_GW_APP_msgPyldLen << 8);
            USB_GW_APP_msgPyldLen |=  (USB_GW_APP_rxBuff[UART_FRAME_HDR_PYLD_LEN_FIELD_OFF + 1]); 
            USB_GW_APP_msgTotLen = UART_FRAME_HDR_LEN + USB_GW_APP_msgPyldLen;
            // Serial.printf("msg payload length<%d> / total-length<%d> \r\n", USB_GW_APP_msgPyldLen, USB_GW_APP_msgTotLen);
        }
    }
    else
    {
        if (USB_GW_APP_uartRxCnt == USB_GW_APP_msgTotLen)
        {
            unsigned short calcPyldCrc16, rxdPyldCrc16;
            unsigned int currMsgType;
            
            // Payload received ...
            // Serial.printf("msg with total length<%d> received  ... \r\n",  USB_GW_APP_msgTotLen);
            
            calcPyldCrc16 = __crc16(USB_GW_APP_rxBuff + UART_FRAME_HDR_LEN, USB_GW_APP_msgPyldLen);
            rxdPyldCrc16 = USB_GW_APP_rxBuff[UART_FRAME_HDR_PYLD_CRC_FIELD_OFF];
            rxdPyldCrc16 = (rxdPyldCrc16 << 8) + USB_GW_APP_rxBuff[UART_FRAME_HDR_PYLD_CRC_FIELD_OFF + 1];         
            
            
            // Serial.printf("<0x%x/0x%x> Payload length CRC %s \r\n", 
            //               rxdPyldCrc16, calcPyldCrc16, calcPyldCrc16 != rxdPyldCrc16 ? "mismatch !!" : "match");
            
            currMsgType = USB_GW_APP_rxBuff[UART_FRAME_HDR_MSG_TYPE_FIELD_OFF];
            currMsgType = (currMsgType << 8) | USB_GW_APP_rxBuff[UART_FRAME_HDR_MSG_TYPE_FIELD_OFF + 1];       
            
            if (currMsgType >= LPWMN_GW_MSG_TYPE_PHOTON_GET_MAC_ADDR)
            {
                switch (currMsgType)
                {
#if 0
                   case LPWMN_GW_MSG_TYPE_PHOTON_GET_MAC_ADDR:
                        {
                            byte __macAddr[WIFI_MAC_ADDR_LEN];
                            WiFi.macAddress(__macAddr);
                            LOCAL_buildSendResp(currMsgType, (const char *)__macAddr, WIFI_MAC_ADDR_LEN);
                        }
                        break;
                        
                   case LPWMN_GW_MSG_TYPE_PHOTON_GET_SSID:   
                        {
                            const char *ssid_p = WiFi.SSID();
                            LOCAL_buildSendResp(currMsgType, (const char *)ssid_p, strlen(ssid_p) + 1);
                        }
                        break;
#endif
#if 0
                   case LPWMN_GW_MSG_TYPE_GET_BORON_IPV4_ADDR:
                        {
                            unsigned int ipv4 = Cellular.localIP();
                            LOCAL_buildSendResp(currMsgType, (const char *)&ipv4, sizeof(ipv4));
                        }
                        break;
                        
                   case LPWMN_GW_MSG_TYPE_GET_BORON_CELLULAR_CONN_STATE:
                        {
                            unsigned char connected = Cellular.ready();
                            LOCAL_buildSendResp(currMsgType, (const char *)&connected, 1);
                        }
                        break;
                        
                    case LPWMN_GW_MSG_TYPE_SWITCH_ON_BORON_CELL_MOD:
                        {
                            Cellular.on();
                        }
                        break;  
                        
                    case LPWMN_GW_MSG_TYPE_SWITCH_OFF_BORON_CELL_MOD:
                        {
                            Cellular.off();
                        }
                        break;  
#endif 
#if 0
                   
                   case LPWMN_GW_MSG_TYPE_PHOTON_GET_IP_GW_IPV4_ADDR:
                        {
                            unsigned int ipv4 = WiFi.gatewayIP();
                            LOCAL_buildSendResp(currMsgType, (const char *)&ipv4, sizeof(ipv4));
                        }
                        break;

                   case LPWMN_GW_MSG_TYPE_PHOTON_GET_IPV4_MASK:
                        {
                            unsigned int mask = WiFi.subnetMask();
                            LOCAL_buildSendResp(currMsgType, (const char *)&mask, sizeof(mask));
                        }
                        break;
                        
                   case LPWMN_GW_MSG_TYPE_PHOTON_HAS_CREDENTIALS:
                        {
                            char respBuff[32];
                            sprintf(respBuff, "%s", WiFi.hasCredentials() ? "Has creds" : "Does not have creds");    
                            LOCAL_buildSendResp(currMsgType, respBuff, strlen(respBuff) + 1);
                        }
                        break;
                        
                   case LPWMN_GW_MSG_TYPE_PHOTON_CLEAR_ALL_CREDENTIALS:
                        {
                            char respBuff[16];
                            sprintf(respBuff, "%s", WiFi.clearCredentials() ? "Done" : "Failed !!");    
                            LOCAL_buildSendResp(currMsgType, respBuff, strlen(respBuff) + 1);
                        }
                        break;
                        
                   case LPWMN_GW_MSG_TYPE_PHOTON_GET_CREDENTIALS:
                        {
                            int off = 0, idx = 0, found = WiFi.getCredentials(WIFI_ap, 5);

                            if (found)
                            {
                                for (int idx = 0; idx < found; idx++) 
                                     off += sprintf(WIFI_apRespBuff + off, "ssid: %s\n", WIFI_ap[idx].ssid);                          
                            }
                            else
                                sprintf(WIFI_apRespBuff, "None");

                            LOCAL_buildSendResp(currMsgType, WIFI_apRespBuff, strlen(WIFI_apRespBuff) + 1);
                        }
                        break;
#endif
                   case LPWMN_GW_MSG_TYPE_PHOTON_SNSR_DATA_HANDLING_CFG_DROP:
                        GW_relaySnsrDataToCloud = 0;
                        break;
                        
                   case LPWMN_GW_MSG_TYPE_PHOTON_SNSR_DATA_HANDLING_CFG_RELAY_TO_CLOUD:
                        GW_relaySnsrDataToCloud = 1;
                        break;                       
                                                
                   default:
                        {
                            const char *resp_p = "Unknown Cmd";
                            LOCAL_buildSendResp(currMsgType, (const char *)resp_p, strlen(resp_p) + 1);                            
                        }
                        break;
                }
                 
            }
            
            // GW_processRcvsMsg(currMsgType, USB_GW_APP_rxBuff + UART_FRAME_HDR_LEN, USB_GW_APP_msgPyldLen);
            
            USB_GW_APP_uartRxCnt = 0;
        }
    }
}

/*
 * The receive buffer size for hardware serial channels (Serial1, Serial2) is 64 bytes.
 * The receive buffer size for USB serial channels (Serial and USBSerial1) is 256 bytes. 
 */
 
/*
 ********************************************************************************************
 * 
 * called when there is data available from Serial
 *
 *
 ********************************************************************************************
 */
void serialEvent()
{
    unsigned char rxByte = Serial.read();
    
    // Read from USB port (coming from gw app running on pc/laptop) and write
    // to serial port connected to LPWMN Coord
    Serial1.write(&rxByte, 1);
    
    interceptUSBAppCmds(rxByte);
}

/*
 ********************************************************************************************
 * serialEvent1: called when there is data available from Serial1.
 * The serialEvent functions are called in between calls to the application loop(). 
 * This means that if loop() runs for a long time due to delay() calls or other 
 * blocking calls the serial buffer might become full between subsequent calls to 
 * serialEvent and serial characters might be lost. Avoid long delay() calls in your 
 * application if using serialEvent.Since serialEvent functions are an extension of 
 * the application loop, it is ok to call any functions that you would also call from 
 * loop().
 ********************************************************************************************
 */
void serialEvent1()
{
    static int intCnt = 0;
    
    unsigned char rxByte = Serial1.read();
    
    if (intCnt & 0x1)
        digitalWrite(ledDbg, HIGH);
    else
        digitalWrite(ledDbg, LOW);
         
    intCnt ++;
    
    GW_uartRxBuff[GW_uartRxCnt] = rxByte;
    
    // Send to USB port (to gw app running on pc/laptop)
    Serial.write(&rxByte, 1);
    
    // Serial.printf("Read byte idx[%02d] - 0x%02x \r\n", GW_uartRxCnt, rxByte);
    
    GW_uartRxCnt ++;
    
    if (GW_uartRxCnt == UART_FRAME_HDR_PYLD_CRC_FIELD_OFF)
    {
        unsigned short calcCrc16, rxdCrc16;
        
        // Serial.printf("Read %d bytes \r\n", GW_uartRxCnt);
        
        calcCrc16 = __crc16(GW_uartRxBuff, UART_FRAME_HDR_HDR_CRC_FIELD_OFF);
        rxdCrc16 = GW_uartRxBuff[UART_FRAME_HDR_HDR_CRC_FIELD_OFF];
        rxdCrc16 = (rxdCrc16 << 8) + GW_uartRxBuff[UART_FRAME_HDR_HDR_CRC_FIELD_OFF + 1];       
        
        if (calcCrc16 != rxdCrc16)
        {
            int idx;
            // Serial.printf("Hdr CRC mismatch <0x%x/0x%x> !!  \r\n", rxdCrc16, calcCrc16);
            for (idx=0; idx<UART_FRAME_HDR_PYLD_CRC_FIELD_OFF-1; idx++)
                 GW_uartRxBuff[idx] = GW_uartRxBuff[idx+1];
            GW_uartRxCnt = UART_FRAME_HDR_PYLD_CRC_FIELD_OFF - 1;
        }
        else
        {
            // Serial.printf("Hdr CRC matches <0x%x/0x%x> \r\n", rxdCrc16, calcCrc16);
            GW_msgPyldLen = GW_uartRxBuff[UART_FRAME_HDR_PYLD_LEN_FIELD_OFF];
            GW_msgPyldLen = (GW_msgPyldLen << 8);
            GW_msgPyldLen |= (GW_uartRxBuff[UART_FRAME_HDR_PYLD_LEN_FIELD_OFF + 1]); 
            GW_msgTotLen = UART_FRAME_HDR_LEN + GW_msgPyldLen;
            // Serial.printf("msg payload length<%d> / total-length<%d> \r\n", GW_msgPyldLen, GW_msgTotLen);

            // Note that the complete header (10 bytes) are sent irrespective of whether there is a payload or not.
        }
    }
    else
    {
        if (GW_uartRxCnt == GW_msgTotLen)
        {
            unsigned short calcPyldCrc16, rxdPyldCrc16;
            unsigned int currMsgType;
            
            // Payload received ...
            // Serial.printf("msg with total length<%d> received  ... \r\n",  GW_msgTotLen);
            
            if (GW_msgPyldLen > 0)
            {
                calcPyldCrc16 = __crc16(GW_uartRxBuff + UART_FRAME_HDR_LEN, GW_msgPyldLen);
                rxdPyldCrc16 = GW_uartRxBuff[UART_FRAME_HDR_PYLD_CRC_FIELD_OFF];
                rxdPyldCrc16 = (rxdPyldCrc16 << 8) + GW_uartRxBuff[UART_FRAME_HDR_PYLD_CRC_FIELD_OFF + 1];         
            }
            
            if ((GW_msgPyldLen == 0) || (calcPyldCrc16 == rxdPyldCrc16))
            {           
                // Serial.printf("<0x%x/0x%x> Payload length CRC %s \r\n", 
                //              rxdPyldCrc16, calcPyldCrc16, calcPyldCrc16 != rxdPyldCrc16 ? "mismatch !!" : "match");
            
                currMsgType = GW_uartRxBuff[UART_FRAME_HDR_MSG_TYPE_FIELD_OFF];
                currMsgType = (currMsgType << 8) | GW_uartRxBuff[UART_FRAME_HDR_MSG_TYPE_FIELD_OFF + 1];       
                
                
                if (GW_expMsgTypeFromCoord != LPWMN_GW_MSG_TYPE_INVALID
                    && GW_expHdrSeqNrFromCoord != 0x100)
                {
                    unsigned int rcvdSeqNr =  GW_uartRxBuff[UART_FRAME_HDR_SEQ_NR_FIELD_OFF];
                    if ((currMsgType == GW_expMsgTypeFromCoord)
                        && (rcvdSeqNr == GW_expHdrSeqNrFromCoord))
                    {
                        GW_procExpCoordResp((unsigned int)currMsgType);
                    }
                }
            
                digitalWrite(ledRxFromCoord, HIGH);
                GW_processRcvdMsg(currMsgType, GW_uartRxBuff + UART_FRAME_HDR_LEN, GW_msgPyldLen);
                digitalWrite(ledRxFromCoord, LOW);
            }
            
            GW_uartRxCnt = 0;
        }
    }

#if 0
    if (__rcvdCnt % 8 == 0)
        Serial.printf("\r\n");
    Serial.printf("0x%02x ", c);
#endif
}


/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
unsigned int GW_buildSendHdr(int msgType, unsigned char *pyldBuff_p, int pyldLen)
{
   unsigned char *buff_p = GW_serTxHdrBuff, currSeqNr;
   unsigned short calcCrc16;
   
   GW_htons(buff_p, msgType);
   buff_p += UART_FRAME_HDR_MSG_TYPE_FIELD_LEN;

   *buff_p = 0x0;
   buff_p += UART_FRAME_HDR_FLAGS_FIELD_LEN;

   currSeqNr = GW_seqNrSentToCoord;
   *buff_p = GW_seqNrSentToCoord ++;
   buff_p += UART_FRAME_HDR_SEQ_NR_FIELD_LEN;

   GW_htons(buff_p, pyldLen);
   buff_p += UART_FRAME_HDR_PYLD_LEN_FIELD_LEN;

   calcCrc16 = __crc16(GW_serTxHdrBuff, UART_FRAME_HDR_HDR_CRC_FIELD_OFF);
   GW_htons(buff_p, calcCrc16);  // no payload
   buff_p += UART_FRAME_HDR_HDR_CRC_FIELD_LEN;

   if (pyldLen > 0)
   {
       calcCrc16 = __crc16(pyldBuff_p, pyldLen);
       GW_htons(buff_p, calcCrc16);  // payload crc 
   }
   else
       GW_htons(buff_p, 0x0);  // no payload 

#if 0
   if (verbose)
   {
       int idx;
       
       printf("\n -------------------------- \n");

       for (idx=0; idx<UART_FRAME_HDR_LEN; idx++)
            printf(" 0x%02x ", serTxBuff[idx]);

       printf("\n -------------------------- \n");
   }
#endif

   Serial1.write(GW_serTxHdrBuff, UART_FRAME_HDR_LEN);

   // Serial.printf("writePort() done \r\n");

   return (unsigned int)currSeqNr;
}


int _loopCnt = 0;

// Next we have the loop function, the other essential part of a microcontroller program.
// This routine gets repeated over and over, as quickly as possible and as many times as possible, after the setup function is called.
// Note: Code that blocks for too long (like more than 5 seconds), can make weird things happen (like dropping the network connection).  
// The built-in delay function shown below safely interleaves required background activity, so arbitrarily long delays can safely be done
// if you need them.

/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void loop() 
{
  static unsigned int loopCnt = 0;
  static int ledVal = 0;
  static int firstTime = 1;
  static int ledThreshCnt = 10000;
  
  
  #ifdef UART_TEST_ENA
  for (;;)
  {
     //Serial.printf("Hello World %u \r\n", loopCnt);
     //Serial1.printf("UART Hello World %u \r\n", loopCnt);
     delay(500);
     loopCnt ++;
  }
  #endif
  
     
  if (firstTime)
  {
      firstTime = 0;
      digitalWrite(ledKA, LOW);
      digitalWrite(ledRxFromCoord, LOW);
      digitalWrite(ledTxToCloud, LOW);
  }
  
  loopCnt ++;
  
  if (loopCnt > ledThreshCnt)
  {
      digitalWrite(ledKA, ledVal ?  LOW :  HIGH);
      loopCnt = 0;     
      ledVal = ledVal ? 0 : 1;
      if (Particle.connected())
          ledThreshCnt = 80000;
      else
          ledThreshCnt = 10000;      
      // Serial.printf("Net: %s \r\n", Particle.connected() ? "Yes" : "No");
  }
  
  
#ifdef GW_COORD_HEALTH_MON
  if (GW_coordStateEvtPending)
  {
      char evtDataBuff[16];
      char evtBuff[16];
      
      sprintf(evtBuff, "%s", GW_coordState == GW_COORD_STATE_DOWN ? "LPMWN_COORD_DOWN" : "LPWMN_COORD_UP");
      sprintf(evtDataBuff, "%s_%u", GW_coordState == GW_COORD_STATE_DOWN ? "DOWN" : "UP",
              GW_coordState == GW_COORD_STATE_DOWN ? GW_lpwmnCoordDownCnt : GW_lpwmnCoordUpCnt);
              
      // Serial.printf("Sending event %s:%s to the cloud .... \r\n", 
      //               evtBuff, evtDataBuff);
    
      Particle.publish("LPWMN_COORD_STATE_CHANGE", evtDataBuff);          
      GW_coordStateEvtPending = 0;
  }

  if (GW_schedKAMsgPending)
  {
      GW_buildSendHdr(LPWMN_GW_MSG_TYPE_NODE_CNT_REQ, NULL, 0x0);
      GW_schedKAMsgPending = 0;     
      GW_coordKATimer.reset();
  }
#endif
    EEPROM.get(0,mqtt_server);
    EEPROM.get(50,mqtt_usn);
    EEPROM.get(100,mqtt_pwd);
    EEPROM.get(150,mqtt_clientId);
    EEPROM.get(200,mqtt_topic);
    
    setup_status=0;
    while(digitalRead(button) == LOW) {                     //can use 2 while check for time lacking for setup
    setup_status++;
    delay(1000);
    if(setup_status==3)
    {
        //WiFi.clearCredentials();
        WiFi.listen();
        setup_status=0;        
    }
  }

  // To blink the LED, first we'll turn it on...
  // digitalWrite(led1, HIGH);
  // digitalWrite(led2, HIGH);

  // We'll leave it on for 1 second...
  // delay(500);

  // Then we'll turn it off...
  // digitalWrite(led1, LOW);
  // digitalWrite(led2, LOW);
  
  // delay(500);
}

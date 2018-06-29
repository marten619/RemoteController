/*********************************************************************
**  Device:     A9112-F4
**  File:       System_config.h
**  Target:     A9112-F4
**  Tools:      ICE
**  Updated:    2017-02-13
**  Description:
**  This file is setup system configure.
**
**  Copyright (C) 2016 AMICCOM Corp.
**
*********************************************************************/
#ifndef _SYSTEM_CONFIG_H_
#define _SYSTEM_CONFIG_H_

/*********************************************************************
*
*   RF_Band: RF_400_BAND
*
*
*   Xtal_select:    1.  Xtal_12p8MHz
*                   2.  Xtal_16MHz
*                   3.  Xtal_19p2MHz
*
*
*   Datarate:       1.  RF_DR10Kbps_50KIFBW            //@ Xtal_12p8MHz
*                   2.  RF_DR100Kbps_100KIFBW          //@ Xtal_12p8MHz
*                   3.  RF_DR150Kbps_150KIFBW          //@ Xtal_19p2MHz
*                   4.  RF_DR250Kbps_250KIFBW          //@ Xtal_16MHz
*
*
*********************************************************************/


/*setup Xtal*/
#define Xtal_12p8MHz
//#define Xtal_19p2MHz 
//#define Xtal_16MHz 



/*setup Data Rate*/
//#define RF_DR10Kbps_50KIFBW         /*Xtal=12.8M*/
#define   RF_DR100Kbps_100KIFBW       /*Xtal=12.8M*/
//#define   RF_DR150Kbps_150KIFBW       /*Xtal=19.2M*/
//#define   RF_DR250Kbps_250KIFBW       /*Xtal=16M*/


/*TIMER*/
#define TIMEOUT             150     //default packet timeout is 150ms


/*project include header*/
#include <absacc.h>
#include "..\MCU\define.h"
#include "..\RF\A9112F4.h"
#include "..\RF\LIB\A9112F4_FunctionLIB.h"
#include "..\RF\A9112F4_config.h"
#include "..\RF\A9112F4_Function.h"
#include "..\MCU\Uti.h"
#include "..\MCU\MCU.h"
#include "..\MCU\I2C.h"
#include "..\include\main.h"



/*Check Datarate and Xtal chioce item*/
#if defined (Xtal_12p8MHz)

    #if defined (RF_DR150Kbps_150KIFBW) 
        #error **chioce datarate or Xtal error**
    #elif defined (RF_DR250Kbps_250KIFBW)
        #error **chioce datarate or Xtal error**
    #endif
    
#elif defined (Xtal_16MHz)

    #if defined (RF_DR250Kbps_250KIFBW) 
    #else
        #error **chioce datarate or Xtal error**
    #endif 
    
#elif defined (Xtal_19p2MHz)

    #if defined (RF_DR150Kbps_150KIFBW) 
    #else
        #error **chioce datarate or Xtal error**
    #endif

#else
      #error **chioce Xtal error**
#endif
        
#endif

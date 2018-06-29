/*****************************************************************************
**                                                                          **
**                         AMICCOM Proprietary Document                     **
**               Copyright (c) 2004-2015 AMICCOM Electronics Inc.           **
**                                                                          **
**                                                                          **
*****************************************************************************/
#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <stdio.h>
#include <string.h>
#include <intrins.h>

#define Sint8  signed   char
#define Uint8  unsigned char
#define Uint16 unsigned int
#define Uint32 unsigned long
#define Sint32 signed long

#define NO                  0
#define YES                 1

#define CALIBRATE_NUMBER    6

#define LOW                 0
#define HIGH                1

#define ON                  1
#define OFF                 0

#define ENABLE              1
#define DISABLE             0

#define MASTER              1
#define SLAVE               0

#define ONEWAY              0
#define TWOWAY              1

#define FALSE               0
#define TRUE                1

#define PASS                0
#define FAIL                1

#define DIRECT              0
#define FIFO                1

#define BIT0        0x01
#define BIT1        0x02
#define BIT2        0x04
#define BIT3        0x08
#define BIT4        0x10
#define BIT5        0x20
#define BIT6        0x40
#define BIT7        0x80


#define LED1								P1_2
#define LED2								P0_7

// FOB Definition
//#define LED_R							LED1
//#define LED_G							LED2

// DOOR Definition
#define LED_R								LED2
#define LED_G								LED1


#define BUZZER								P1_6
#define BTN1								P3_5
#define BTN2								P3_4
#define BTN3								P3_3
#define BTN4								P3_2

#define Learning_Mode_Seconds			8		// 500ms per count

// Definition of EEPROM & Crypto
#define SW_VERSION						0x11
#define EEP_HEADER_SIZE					8		// EEPROM Header size (bytes)
#define EEP_HEADER_OFFSET				0		// Header offset 
#define EEP_KEY_OFFSET					(EEP_HEADER_OFFSET + EEP_HEADER_SIZE)
#define MAX_KEY_NUM   					8    	// DeviceList[32][8]
#define KEY_SIZE						   8		// Key Size of each FOB

#define EEPROM_DEVICE_NAME				24AA16
#define EEPROM_DEVICE_ADDR				0x50
#define EEPROM_DELAY_TIME				5		// write time of EEPROM (ms)
#define EEPROM_SIZE_BYTE				0x800


/*********************************************************************
**  System State
*********************************************************************/
#define State_Request       0x11
#define State_Challenge     0x22
#define State_Response      0x33
#define State_Standby       0x44
#define State_Learning      0x55

/*********************************************************************
**  System Mode
*********************************************************************/
#define Operation_Mode		 0x0
#define Learning_Mode		 0x1


/*********************************************************************
**  User CMD
*********************************************************************/
#define User_Command_1 		 0x21
#define User_Command_2      0x22
#define User_Command_3      0x23
#define User_Command_4      0x24
#define User_Command_5      0x25
#define User_Command_6      0x26
#define User_Command_7      0x27
#define User_Command_8      0x28

/*********************************************************************
**  Others
*********************************************************************/
// Structure of EEPROM, head discription for FOB key recording
struct EEP_T {
	Uint8	Ver;				// SW version
	Uint8	KeyMax;			// Max number of Key in system
	Uint8	KeySize;			// Size of each key
	Uint8	KeyIndex;		// How many keys are recorded
	Uint8	Crypto[4];		// Keep information for Crypto IC
} ;


#endif
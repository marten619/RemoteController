/*********************************************************************
**  Device:     A9112-F4
**  File:       main.h
**  Target:     A9112-F4
**  Tools:      ICE
**  Updated:    2016-05-13
**  Description:
**  This file is main header.
**
**  Copyright (C) 2016 AMICCOM Corp.
**
*********************************************************************/
#ifndef _MAIN_H_
#define _MAIN_H_


/*********************************************************************
**  Global Variable Declaration
*********************************************************************/

#ifdef _MAIN_
        Uint16  idata   RxCnt;
        Uint32  idata   Err_ByteCnt;
        Uint32  idata   Err_BitCnt;
        const Uint8 code BitCount_Tab[16]={0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
        const Uint8 code PN9_Tab[]=
        {   
                0xFF,0x83,0xDF,0x17,0x32,0x09,0x4E,0xD1,
                0xE7,0xCD,0x8A,0x91,0xC6,0xD5,0xC4,0xC4,
                0x40,0x21,0x18,0x4E,0x55,0x86,0xF4,0xDC,
                0x8A,0x15,0xA7,0xEC,0x92,0xDF,0x93,0x53,
                0x30,0x18,0xCA,0x34,0xBF,0xA2,0xC7,0x59,
                0x67,0x8F,0xBA,0x0D,0x6D,0xD8,0x2D,0x7D,
                0x54,0x0A,0x57,0x97,0x70,0x39,0xD2,0x7A,
                0xEA,0x24,0x33,0x85,0xED,0x9A,0x1D,0xE0
        };  // This table are 64bytes PN9 pseudo random code.
#else
        extern Uint16  idata   RxCnt;
        extern Uint32  idata   Err_ByteCnt;
        extern Uint32  idata   Err_BitCnt;
        extern const Uint8 code BitCount_Tab[16];
        extern const Uint8 code PN9_Tab[];      

#endif

/*********************************************************************
**  function Declaration
*********************************************************************/
extern void A9112_WriteFIFO(void);
extern void RxPacket(void);
extern void RTC_Enable(void);
extern void WOR_enable_by_preamble(void);
extern void WOR_enable_by_sync(void);
extern void WOR_enable_by_carrier(void);
extern void WOT_enable(void);
extern void TWOR_enable(void);
extern void TMR_enable(void);
extern void SAR_ADC_12B(void);
extern void RSSI_measurement(void);
extern void RC_ADC0_24B(void);
extern void RC_ADC1_24B(void);
extern void UserRegister(void);
#endif
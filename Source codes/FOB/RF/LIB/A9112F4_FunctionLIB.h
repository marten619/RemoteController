/*********************************************************************
**  Device:     A9112-F4
**  File:       A9112_FunctionLIB.h
**  Target:     A9112-F4
**  Tools:      ICE
**  Updated:    2017-02-13
**  Description:
**  This file is the RF function header.
**
**  Copyright (C) 2017 AMICCOM Corp.
**
*********************************************************************/
#ifndef _A9112F4_FunctionLIB_H_
#define _A9112F4_FunctionLIB_H_

/*define DR for configure*/
#define 	CONFIG_DR10Kbps_50KIFBW     0    /*Xtal=12.8M*/
#define   CONFIG_DR100Kbps_100KIFBW   1    /*Xtal=12.8M*/
#define   CONFIG_DR150Kbps_150KIFBW   2    /*Xtal=19.2M*/
#define   CONFIG_DR250Kbps_250KIFBW   3    /*Xtal=16M*/

#ifdef _A9112F4_FunctionLIB_C_
				Uint8   xdata A9112Config[107];
#else
				extern Uint8  xdata A9112Config[107];
#endif
    
/*********************************************************************
**  function Declaration
*********************************************************************/      
extern void A9112_Load_Config(Uint8 _sel_);

#endif

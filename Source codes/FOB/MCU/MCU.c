/*********************************************************************
**  File:       MCU.c
**  Updated:    2016-05-27
**  Description:
**  This file is the MCU function.
**
**  Copyright (C) 2016 AMICCOM Corp.
**
*********************************************************************/
#define _MCU_C_
#include "..\include\System_config.h"

/************************************************************************
**  InitMCU
************************************************************************/
void InitMCU(void)
{
    P0      = 0x00;
    P0OE    = 0xFF;
    P0PUN   = 0xFF;
    P0WUN   = 0xFF;

    P1      = 0x00;
    P1OE    = 0xFF;
    P1PUN   = 0xFF;
    P1WUN   = 0xFF;

    P3      = 0xFF;
    P3OE    = 0xC2;     //P3.0=RXD(input), P3.5=Mode Select(input)
    P3PUN   = 0xC2;
    P3WUN   = 0xFF;
    
    // IOSEL     |= 0x08;        //enable RF GPIO pin : GIO1=P0.7, GIO2=P1.2, CKO=P1.3
}
/************************************************************************
**  InitTimer0
************************************************************************/
void InitTimer0(void)
{
    TR0     = 0;
    TMOD    = (TMOD & 0xF0) | 0x01; //Timer0 mode1
    TH0     = (65536-t0hrel)>>8;    //setup Timer0 high byte, low byte
    TL0     = 65536-t0hrel;
    TF0     = 0;                    //Clear any pending Timer0 interrupts
    ET0     = 1;                    //Enable Timer0 interrupt
}

/************************************************************************
**  InitUART0_Timer1
************************************************************************/
void InitUART0_Timer1(void)
{
        IOSEL   |= 0x01;         //enable UART pin

        TR1     = 0;
        TMOD    = (TMOD & 0x0F) | 0x20; //Timer1 mode2
        TH1     = (256 - BaudRate_19200);//Baud Rate
        TL1     = (256 - BaudRate_19200);//Baud Rate
        PCON    = 0x80;                 //SMOD=1
        CKCON   = 0x10;                 //T1M=1
        SCON    = 0x40;                 //UATR0 mode1 : 8bits UART
        T2CON   = 0x00;                 //UART0 use Timer1
        REN     = 1;
        TR1     = 1;
        ES      = 1;                    //enable uart0 interrupt
}

/************************************************************************
**  Timer0ISR
************************************************************************/
void Timer0ISR(void) interrupt 1
{
	// This is a 1ms Timer ISR
    TH0 = (65536-t0hrel)>>8;    //Reload Timer0 high byte, low byte
    TL0 = 65536-t0hrel;
    TF0 = 0;

    timer++;
    if(timer >= TIMEOUT)
    {
        TimeoutFlag=1;
    }

    TimerCnt0++;
    if(TimerCnt0 >= 500)
    {
			TimerCnt0=0;
			if(btn_push_counting > 0) {
				btn_push_counting++;
			}
    }
}

/************************************************************************
**  UART0ISR
************************************************************************/
void UART0ISR(void) interrupt 4 using 3
{
    if(TI)
    {
        TI=0;
        UartSendCnt--;
        if(UartSendCnt !=0)
        {
            Uartptr++;
            SBUF = *Uartptr;
        }
    }

    if(RI)
    {
        RI=0;
    }
}

/************************************************************************
**  Err_State
************************************************************************/
void Err_State(void)
{
    //ERR display
    //Error Proc...
    //...
    while(1);
}

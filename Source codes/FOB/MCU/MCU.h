/*********************************************************************
**	
**	File:		MCU.h
**	Target:		A9113-F4
**	Tools:		ICE
**	Updated:	2016-XX-XX
**	Description:
**	This file is the RF	function.
**
**	Copyright (C) 2016 AMICCOM Corp.
**
*********************************************************************/
#ifndef	_MCU_H_
#define	_MCU_H_
#include "..\include\System_config.h"


/*setup	Xtal*/
#if	defined	(Xtal_12p8MHz)
	
	#define	t0hrel				1067	//1ms @12.8MHz
	#define	BaudRate_2400		83		//crystal=12.8MHz, timer1, SMOD=1, T1M=1
	#define	BaudRate_4800		42		//crystal=12.8MHz, timer1, SMOD=1, T1M=1
	#define	BaudRate_9600		21		//crystal=12.8MHz, timer1, SMOD=1, T1M=1
	#define	BaudRate_19200		11		//crystal=12.8MHz, timer1, SMOD=1, T1M=1
	
#elif defined (Xtal_16MHz)

	#define	t0hrel				1333	//1ms @16MHz
	#define	BaudRate_4800		52		//crystal=16MHz, timer1, SMOD=1, T1M=1
	#define	BaudRate_9600		26		//crystal=16MHz, timer1, SMOD=1, T1M=1
	#define	BaudRate_19200		13		//crystal=16MHz, timer1, SMOD=1, T1M=1	
	
#elif defined (Xtal_19p2MHz)

	#define	t0hrel				1600	//1ms @19.2MHz
	#define	BaudRate_9600		31		//crystal=19.2MHz, timer1, SMOD=1, T1M=1
	#define	BaudRate_19200		16		//crystal=19.2MHz, timer1, SMOD=1, T1M=1
	#define	BaudRate_38400		8		//crystal=19.2MHz, timer1, SMOD=1, T1M=1
			
#else


#endif

/*********************************************************************
**	Constant Declaration
*********************************************************************/	
#ifdef _MCU_C_
	Uint8	data	timer;
	Uint8	data	TimeoutFlag;	
	Uint16	idata	TimerCnt0;
	Uint8	xdata	*Uartptr;
	Uint8	xdata	UartSendCnt;
//	Uint8	data	CmdBuf[11];
//	Uint8	xdata	tmpbuf[64];

	extern Uint8   xdata	 btn_push_counting;
#else
	extern Uint8   data	   timer;
	extern Uint8   data	   TimeoutFlag;	
	extern Uint16  idata   TimerCnt0;
	extern Uint8   xdata	   *Uartptr;
	extern Uint8   xdata	   UartSendCnt;
//	extern Uint8   data	   CmdBuf[11];
//	extern Uint8   xdata   tmpbuf[64];
	
#endif

/*********************************************************************
**	function Declaration
*********************************************************************/	
void InitMCU(void);
void InitTimer0(void);
void InitUART0_Timer1(void);
void Err_State(void);
void Timer0ISR(void);
void UART0ISR(void);


#endif
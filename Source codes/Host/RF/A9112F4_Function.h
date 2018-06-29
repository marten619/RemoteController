/*********************************************************************
**	Device:		A9112-F4
**	File:		A9112_Function.h
**	Target:		A9112-F4
**	Tools:		ICE
**	Updated:	2016-05-13
**	Description:
**	This file is the RF	function header.
**
**	Copyright (C) 2016 AMICCOM Corp.
**
*********************************************************************/
#ifndef	_A9112F4_Function_H_
#define	_A9112F4_Function_H_

/*********************************************************************
**	RF_Interrupt Declaration
*********************************************************************/
#define	Enable_RFINT_TMR	0x01
#define	Enable_RFINT_WTR	0x02
#define	Enable_RFINT_TWWS	0x04
#define	Enable_RFINT_12BADC	0x08
#define	Enable_RFINT_FPF	0x20
#define	Enable_RFINT_FSTNC	0x80

#define	PM1					0xD0
#define	PM2					0xB0
#define	PM3					0xA8

#define	CKO					P1_3		//CKO
#define	GIO1				P0_7		//GIO1
#define	GIO2				P1_2		//GIO2


#ifdef _A9112F4_Function_C_
		Uint8	data	Flag_MASTER;
		Uint8	data	Seq;
		Uint8	data	RF_FLAG;
#else
		extern Uint8	data	Flag_MASTER;
		extern Uint8	data	Seq;
		extern Uint8	data	RF_FLAG;
#endif
	
/*********************************************************************
**	function Declaration
*********************************************************************/		
extern void	A9112_WriteReg(Uint16, Uint8);
extern Uint8 A9112_ReadReg(Uint16);
extern void	A9112_Config(void);
extern void	A9112_WriteID(void);	   
extern void	A9112_Cal(void);   
extern void	InitRF(void);  
extern void	ResetCMD(Uint8);
extern void	StrobeCMD(Uint8);

extern void	A9112_PM(Uint8 _sel_);
extern void	FreqSet(Uint8 ch);
extern void	RCOSC_Cal(void);
extern void	RFISR(void);


#endif

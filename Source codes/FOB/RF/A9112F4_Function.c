/*********************************************************************
**	Device:		A9112-F4
**	File:		A9112_Function.c
**	Target:		A9112-F4
**	Tools:		ICE
**	Updated:	2017-02-13
**	Description:
**	This file is the RF	function.
**
**	Copyright (C) 2016 AMICCOM Corp.
**
*********************************************************************/
#define	_A9112F4_Function_C_

#include "..\include\System_config.h"


/************************************************************************
**	A9112_WriteReg
************************************************************************/
void A9112_WriteReg(Uint16 addr, Uint8 dataByte)
{
	XBYTE[addr]	= dataByte;
	_nop_(); _nop_();
	_nop_(); _nop_();
}

/************************************************************************
**	A9112_ReadReg
************************************************************************/
Uint8 A9112_ReadReg(Uint16 addr)
{
	return (XBYTE[addr]);
}

/************************************************************************
**	Reset Command
************************************************************************/
void ResetCMD(Uint8	cmd)
{
	A9112_WriteReg(RSTCTL_REG, cmd);
	Delay10us(1);
}

/************************************************************************
**	Strobe Command
************************************************************************/
void StrobeCMD(Uint8 cmd)
{
	A9112_WriteReg(MODEC1_REG, cmd);
	Delay10us(1);
}

/************************************************************************
**	InitRF
************************************************************************/
void InitRF(void)
{
	Delay100us(5);		//delay	500us for regulator	stabilized
	ResetCMD(RF_RST);	//reset	A9112 chip
	A9112_Config();		//config A9112 chip
	Delay100us(5);		//delay	500us for crystal(49US type) stabilized
	A9112_WriteID();	//write	ID code
	A9112_Cal();		//IF and VCO calibration
	
	if(Flag_MASTER)	
		A9112_WriteReg(RX_REG, A9112Config[RX_REG -	CONFIG]	& 0xFE);	//Master: ULS=0
	else	
		A9112_WriteReg(RX_REG, A9112Config[RX_REG -	CONFIG]	| 0x01);	//Slave:	ULS=1

}

/************************************************************************
**	A9112_Config
************************************************************************/
void A9112_Config(void)
{
	Uint16 i;
	
		#ifdef RF_DR10Kbps_50KIFBW         /*Xtal=12.8M*/
				A9112_Load_Config(CONFIG_DR10Kbps_50KIFBW);	
    #endif	
	
		#ifdef RF_DR100Kbps_100KIFBW       /*Xtal=12.8M*/
				A9112_Load_Config(CONFIG_DR100Kbps_100KIFBW);
    #endif		

		#ifdef RF_DR150Kbps_150KIFBW       /*Xtal=19.2M*/
				A9112_Load_Config(CONFIG_DR150Kbps_150KIFBW);
    #endif		

		#ifdef RF_DR250Kbps_250KIFBW       /*Xtal=16M*/
				A9112_Load_Config(CONFIG_DR250Kbps_250KIFBW);	
    #endif	
	
	for(i=0; i<sizeof(A9112Config);	i++)
		A9112_WriteReg(i + CONFIG, A9112Config[i]);
}

/************************************************************************
**	A9112_WriteID
************************************************************************/
void A9112_WriteID(void)
{
	Uint8 i;
	Uint8 d[8];

	//Write	ID
	for(i=0; i<8; i++)
		A9112_WriteReg((ID0_REG	- i), ID_Tab[i]);

	//Read ID
	for(i=0; i<8; i++)
		d[i] = A9112_ReadReg(ID0_REG - i);

	//for check
	for(i=0; i<8; i++)
	{
		if(d[i]	!= ID_Tab[i])
		{
			Err_State();
		}
	}
}

/************************************************************************
**	A9112_Cal
************************************************************************/
void A9112_Cal(void)
{
	Uint8 fb, fcd, fbcf;	//IF Filter
	Uint8 vcb, vccf;		//VCO Current
	Uint8 vb, vbcf;			//VCO Band
	Uint8 tmp,i;

	
	StrobeCMD(CMD_STBY);

	//IF calibration procedure @STB	state
	A9112_WriteReg(IFC1_REG, A9112Config[IFC1_REG -	CONFIG]);			//IF Auto Cal.
	A9112_WriteReg(CACL_REG, A9112Config[CACL_REG -	CONFIG]	| 0x0A);	//IF & VCC Calibration
	do{
		tmp	= A9112_ReadReg(CACL_REG);
	}while(tmp & 0x0A);

	//for check(IF Filter)
	tmp		= A9112_ReadReg(IFC1_REG);
	fb		= tmp &	0x0F;
	fbcf	= (tmp >> 4) & 0x01;
	tmp		= A9112_ReadReg(IFC2_REG);
	fcd		= tmp &	0x1F;
	if(fbcf)
	{	
		Err_State();
	}
	
	//manual FB+1 for IFBW=50K or 100KHz
	tmp	= (A9112Config[RX_REG -	CONFIG]	& 0x30)>>4;
	
	if((0 == tmp) || (1	== tmp))
	A9112_WriteReg(IFC1_REG, (fb + 1) |	0x10);	
	
	//for check(VCO	Current)
	tmp		= A9112_ReadReg(VCOCC_REG);
	vcb		= tmp &	0x0F;
	vccf	= (tmp >> 4) & 0x01;
	if(vccf)
	{
		Err_State();
	}
	
	//RSSI Calibration procedure @STB state
	A9112_WriteReg(ADCC_REG, 0xCC);		//set ADC average=64
	A9112_WriteReg(CACL_REG, A9112Config[CACL_REG -	CONFIG]	| 0x01);		//RSSI Calibration
	do{
		tmp	= A9112_ReadReg(CACL_REG);
	}while(tmp & 0x01);
	A9112_WriteReg(ADCC_REG, A9112Config[ADCC_REG -	CONFIG]);
	
		i=0;
		
				
		//VCO calibration procedure	@STB state
		A9112_WriteReg(PLL3_REG, MasterFreq_Tab[3*i]);
		A9112_WriteReg(PLL4_REG, MasterFreq_Tab[3*i+1]);
		A9112_WriteReg(PLL5_REG, MasterFreq_Tab[3*i+2]);
		
		A9112_WriteReg(CACL_REG, A9112Config[CACL_REG -	CONFIG]	| 0x04);		//VCO Calibration
		do{
			tmp	= A9112_ReadReg(CACL_REG);
		}while(tmp & 0x04);
				
		//for check(VCO	Band)
		tmp		= A9112_ReadReg(VCOBC1_REG);
		vb		= tmp &	0x07;
		vbcf	= (tmp >> 3) & 0x01;
		if(vbcf)
		{	
			Err_State();
		}
	
	
}

/************************************************************************
**	A9112_PM
************************************************************************/
void A9112_PM(Uint8	_sel_)
{
	
//	P0OE  &= 0x00;
//	P0PUN &= 0x00;
//	P0    = 0xFF;
//	P0WUN |= 0xFF;
//	
//	P1OE  &= 0x00;
//	P1PUN &= 0x00;
//	P1WUN |= 0xFF;
//	
//	P3OE  &= 0x00;
//	P3PUN &= 0x00;
//	P3WUN =	0xFB;
		//all pin as input and pull high
	IOSEL   = 0x00;

	P0OE  = 0x00;
	P0PUN = 0xFF;
	P0    = 0x00;
	P0WUN = 0xFF;

	P1OE  = 0x00;		// Output: High enable, Bit 1,2,3,6,7 enable
	P1PUN = 0xFF;		// Pullup: Low enable, Bit 1,2,3,6,7 disable
	P1    = 0x00;		// Output Data
	P1WUN = 0xFF;

	P3OE  = 0x00;
	P3PUN = 0x00;
	P3    = 0xC3;
	P3WUN	= 0xC3; 		// Low enable: P3_2 ~ P3_5 are enabled	
	
	A9112_WriteReg(EXT5_REG, 0x08);	
	
	switch(	_sel_ )
	{
		case PM1:
			A9112_WriteReg(PWRCTL_REG, 0xD0);			
		break;
		
		case PM2:
			A9112_WriteReg(PWRCTL_REG, 0xB0);			
		break;
		
		case PM3:
			A9112_WriteReg(PWRCTL_REG, 0xA8);
		break;	
		
	}
	
	StrobeCMD(CMD_SLEEP);
	PCONE =	0x09;
	PCON = PCON	| 0x02;
	_nop_(); _nop_();
	while(PCON & 0x02);	
	
}

/************************************************************************
**	FreqSet
************************************************************************/
void FreqSet(Uint8 ch)
{
	if(Flag_MASTER)
	{
		A9112_WriteReg(PLL3_REG, MasterFreq_Tab[3*ch]);
		A9112_WriteReg(PLL4_REG, MasterFreq_Tab[3*ch+1]);
		A9112_WriteReg(PLL5_REG, MasterFreq_Tab[3*ch+2]);
	}
	else
	{
		A9112_WriteReg(PLL3_REG, SlaveFreq_Tab[3*ch]);
		A9112_WriteReg(PLL4_REG, SlaveFreq_Tab[3*ch+1]);
		A9112_WriteReg(PLL5_REG, SlaveFreq_Tab[3*ch+2]);
	}
}

/************************************************************************
**	RC Oscillator Calibration
************************************************************************/
void RCOSC_Cal(void)
{
	Uint16 tmp;
	
#if	defined	(Xtal_12p8MHz)
	Uint16 TGNUM=781;	//for crystal=12.8MHz
#elif defined (Xtal_16MHz)
	Uint16 TGNUM=977;	//for crystal=16MHz
#elif defined (Xtal_19p2MHz)
	Uint16 TGNUM=1172;	//for crystal=19.2MHz
#endif

		
	StrobeCMD(CMD_STBY);

	//A9112_WriteReg(CKO_REG, 0x50);	//ROSC=32.768KHz
	//A9112_WriteReg(CKO_REG, 0x40);	//WCK=4.096KHz

	A9112_WriteReg(RCOSC7_REG, (TGNUM>>8));	//TGNUM
	A9112_WriteReg(RCOSC8_REG, TGNUM);		//TGNUM
	
	A9112_WriteReg(RCOSC3_REG, A9112Config[RCOSC3_REG -	CONFIG]	| 0x04);	//enable RC	OSC

	while(1)
	{
		A9112_WriteReg(RCOSC4_REG, A9112Config[RCOSC4_REG -	CONFIG]	| 0x01);	//set ENCAL=1 to start RC OSC CAL
		do{
			tmp	= A9112_ReadReg(RCOSC4_REG);
		}while(tmp & 0x01);

		tmp	= (A9112_ReadReg(RCOSC4_REG) & 0xF0);	//read NUMLH[11:8]
		tmp	<<=	4;
		tmp	= tmp +	A9112_ReadReg(RCOSC5_REG);		//read NUMLH[7:0]
		
		if((tmp	> (TGNUM-10)) && (tmp <	(TGNUM+10)))	//NUMLH[11:0]=TGNUM+-10
		{
			break;
		}
	}
}

/************************************************************************
**	RFISR
************************************************************************/
void RFISR(void) interrupt 10
{
	EIF	|= 0x08;	//clear	flag
	RF_FLAG	= 0;
	
	//A9112_WriteReg(TMRCTL_REG, 0xE1);	//TMRCKS=128Hz
	//A9112_WriteReg(TMRCTL_REG, 0xE3);	//TMRCKS=64Hz
	//A9112_WriteReg(TMRCTL_REG, 0xE5);	//TMRCKS=32Hz
	//A9112_WriteReg(TMRCTL_REG, 0xE7);	//TMRCKS=16Hz
	//A9112_WriteReg(TMRCTL_REG, 0xE9);	//TMRCKS=8Hz
	//A9112_WriteReg(TMRCTL_REG, 0xEB);	//TMRCKS=4Hz
	//A9112_WriteReg(TMRCTL_REG, 0xED);	//TMRCKS=2Hz
	//A9112_WriteReg(TMRCTL_REG, 0xEF);	//TMRCKS=1Hz
}

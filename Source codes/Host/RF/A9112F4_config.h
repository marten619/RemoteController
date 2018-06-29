 /********************************************************************
*	A9112F4_CONFIG.h
*	RF Chip-A9112F4	/ A9109-F4	Configure Definitions
*
*	This file provides the constants associated	with the
*	AMICCOM	A9112F4	/ A9109-F4 device.
*
********************************************************************/
#ifndef	_A9112F4_CONFIG_h_
#define	_A9112F4_CONFIG_h_



#ifdef _A9112F4_Function_C_
 
	
		#ifdef RF_DR10Kbps_50KIFBW 
				//433MHz, 10kbps (IFBW = 50KHz, Fdev = 18.75KHz), Crystal=12.8MHz
				const Uint8	code ID_Tab[8]={0x34,0x75,0xC5,0x8C,0xC7,0x33,0x45,0xE7};	//ID code
				const Uint8	HopTab[] = {0,1,2};	//hopping channel
				const Uint8	code MasterFreq_Tab[]=
				{		 
						0x87, 0x68, 0x14,  //CH0, freq 433.301MHz
						0x87, 0x80, 0x14,  //CH1, freq 433.601MHz
						0x87, 0xA0, 0x14   //CH2, freq 434.001MHz
				};
				const Uint8	code SlaveFreq_Tab[]=
				{
						0x87, 0x70, 0x14,  //CH0, freq 433.401MHz
						0x87, 0x88, 0x14,  //CH1, freq 433.701MHz
						0x87, 0xA8, 0x14   //CH2, freq 434.101MHz
				};

		#endif
	

		#ifdef RF_DR100Kbps_100KIFBW
				//433MHz, 100kbps (IFBW = 100KHz, Fdev = 37.5KHz), Crystal=12.8MHz
				const Uint8	code ID_Tab[8]={0x34,0x75,0xC5,0x8C,0xC7,0x33,0x45,0xE7};	//ID code
				const Uint8	HopTab[] = {0,1,2};	//hopping channel
				const Uint8	code MasterFreq_Tab[]=
				{
						0x87, 0x68, 0x14,  //CH0, freq 433.301MHz
						0x87, 0x80, 0x14,  //CH1, freq 433.601MHz
						0x87, 0xA0, 0x14   //CH2, freq 434.001MHz
				};
				const Uint8	code SlaveFreq_Tab[]=
				{
						0x87, 0x78, 0x14,  //CH0, freq 433.501MHz
						0x87, 0x90, 0x14,  //CH1, freq 433.801MHz
						0x87, 0xB0, 0x14   //CH2, freq 434.201MHz
				};

		#endif
				
		#ifdef RF_DR150Kbps_150KIFBW
				 //433MHz, 150kbps (IFBW = 150KHz, Fdev = 56.25KHz), Crystal=19.2MHz
				const Uint8	code ID_Tab[8]={0x34,0x75,0xC5,0x8C,0xC7,0x33,0x45,0xE7};	//ID code
				const Uint8	HopTab[] = {0,1,2};	//hopping channel
				const Uint8	code MasterFreq_Tab[]=
				{
						0x5A, 0x45, 0x62,  //CH0, freq 433.301MHz
						0x5A, 0x55, 0x62,  //CH1, freq 433.601MHz
						0x5A, 0x6A, 0xB8   //CH2, freq 434.001MHz
				};
				const Uint8	code SlaveFreq_Tab[]=
				{
						0x5A, 0x55, 0x62,  //CH0, freq 433.601MHz
						0x5A, 0x65, 0x62,  //CH1, freq 434.901MHz
						0x5A, 0x7A, 0xB8   //CH2, freq 434.301MHz
				};

		#endif
	
		#ifdef RF_DR250Kbps_250KIFBW
				//433MHz, 250kbps (IFBW = 250KHz, Fdev = 93.75KHz), Crystal=16MHz
				const Uint8	code ID_Tab[8]={0x34,0x75,0xC5,0x8C,0xC7,0x33,0x45,0xE7};	//ID code
				const Uint8	HopTab[] = {0,1,2};	//hopping channel
				const Uint8	code MasterFreq_Tab[]=
				{
						0x6C, 0x53, 0x43,  //CH0, freq 433.301MHz
						0x6C, 0x66, 0x76,  //CH1, freq 433.601MHz
						0x6C, 0x80, 0x10   //CH2, freq 434.001MHz
				};
				const Uint8	code SlaveFreq_Tab[]=
				{
						0x6C, 0x73, 0x43,  //CH0, freq 433.801MHz
						0x6C, 0x86, 0x76,  //CH1, freq 434.101MHz
						0x6C, 0xA0, 0x10   //CH2, freq 434.501MHz
				};
		#endif
	


#else
	extern const Uint8 code	ID_Tab[];
	extern const Uint8 HopTab[]; 
	extern const Uint8 code	MasterFreq_Tab[];
	extern const Uint8 code	SlaveFreq_Tab[];	
	
#endif
  
	
	
	
#endif
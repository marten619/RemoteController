/*********************************************************************
**	Device:		A9112-F4
**	File:		main.c
**	Target:		A9112-F4
**	Tools:		ICE
**	Updated:	2016-07-21
**	Description:
**	This file is a sample code for your	reference.
**
**	Copyright (C) 2016 AMICCOM Corp.
**
*********************************************************************
	SN Length = 8Bytes	
	FOB#		XDATA
	6			709
	10			741
	50			1061
	100		1461
*********************************************************************
	@2018-01-31
	Program Size: data=79.4 xdata=1461 code=8388
	creating hex file from "A9112F4"...
	"A9112F4" - 0 Error(s), 1 Warning(s).
	Build Time Elapsed:  00:00:01
	Load "C:\\Projects\\Amicomm\\Remote Control A9112\\keil\\A9112F4" 
	Flash Chip Erase Done.
	Check Blank Done: 0 error(s).
	Flash Write Done: 8388 bytes programmed.
	Flash Verify Done: 8388 bytes verified.
	Flash Load finished at 10:04:00
*/
#define	_MAIN_

#include "..\include\System_config.h"

/********************************************************************/
// Extern Functions
/********************************************************************/
extern void    SHA_I2C_Init(void);
extern void    ShaCmdSendRecv(Uint8 *sbuf, Uint8 sLen, Uint8 *rbuf, Uint8 rLen,Uint8 delay);
extern Uint8   ShaReadSn (Uint8 *rbuf);
extern Uint8 	ShaReadRnd (Uint8 *rbuf);
extern Uint8   I2CSha204Wakeup();
extern Uint8   I2CSha204Sleep();
extern void		step0_mixSnIntoRnd_b();
extern Uint8 	step1_getMACfromSlot0(Uint8 *, Uint8 ,Uint8 *, Uint8 );  

/****************************************
** User definition
****************************************/
Uint8   xdata    uartBuf[64];
Uint8   data    User_FOB;
Uint8   data	 BTN1_Tripped;
Uint8   data    BTN2_Tripped;

Uint8   xdata   DEV_id[16];
Uint8   xdata   DEV_sn[8];      // Device unique SN
Uint8   xdata   RND_a[32];
Uint8   xdata   RND_b[32];       // get from Host
Uint8   xdata   ANS_a[32];
Uint8   xdata   ANS_b[32];
Uint8   xdata   MAC_my[32];      // User's MAC = sha(Slot 0, S/N)
Uint8   xdata   TX_cmd;
Uint8   xdata   ShaBuff[64];
Uint8   xdata   TxBuff[64];
Uint8   xdata   RxBuff[64];
Uint8   xdata   TxLen;
Uint8   xdata   RxLen;
Uint8   xdata	 sysState;
Uint8   xdata	 running_mode;
Uint8   xdata	 numberOfKey;
Uint8   xdata	 btn_push_counting;
Uint8   xdata   DeviceList[MAX_KEY_NUM][KEY_SIZE] = {0};
struct  EEP_T	  xdata	 E2P;

/**********************************************
** Local Function Decoration 
**********************************************/
void UrPrint (Uint8 *_ptr, Uint8 _len);
void KEYISR ( void );
void RF_sendData ( Uint8 _stat, Uint8 *buf, Uint8 bLen );
void RxPacket ( void );
Uint8 isSnInChecklist(Uint8 *ptr, Uint8 );
Uint8 calculateANS_a();
Uint8 calculateANS_b();
void setDoorOpen(void);
void setDoorClose(void);
void setWakeUpKeyIsr (void);
// EEPROM Functions
Uint8 eraseEeprom (void);
Uint8 loadEeprom (void);
Uint8 updateHeaderToEeprom (void);
Uint8 loadKeyFromEeprom(void);
Uint8 writeKeyToEeprom(void);



/*********************************************************************
* ResetState
*********************************************************************/
void ResetState(void) {
	memset(RND_a, 0, sizeof(RND_a));
	memset(RND_b, 0, sizeof(RND_b));
	memset(ANS_a, 0, sizeof(ANS_a));
	memset(ANS_b, 0, sizeof(ANS_b));
	memset(MAC_my, 0, sizeof(MAC_my));
	memset(ShaBuff, 0, sizeof(ShaBuff));
	memset(TxBuff, 0, sizeof(TxBuff));
	memset(RxBuff, 0, sizeof(RxBuff));
	sysState = State_Standby;
	TX_cmd = 0;
}

/*********************************************************************
* Erase Eeprom Header
*********************************************************************/
Uint8 eraseEeprom (void) {
	Uint8		i;
	
	memset(&E2P, 0, EEP_HEADER_SIZE);
	
	for (i=0; i < EEP_HEADER_SIZE; i++) {
		if(PASS != WriteEEPROM( EEPROM_DEVICE_ADDR, (EEP_HEADER_OFFSET+i), (&E2P+i), 1)) {
			return FAIL;
		}	
		Delay1ms (EEPROM_DELAY_TIME);
	}		
	return PASS;	
}


/*********************************************************************
* Load Eeprom
*********************************************************************/
Uint8 loadEeprom (void) {
	// Uint8 *offset = &E2P;
	memset(&E2P, 0, EEP_HEADER_SIZE);
	if(PASS != ReadEEPROM(EEPROM_DEVICE_ADDR, EEP_HEADER_OFFSET, &E2P, EEP_HEADER_SIZE)) {
		return FAIL;
	}
	
	// Check for Empty EEPROM
	if(E2P.Ver != SW_VERSION) {
		E2P.Ver = SW_VERSION;
		E2P.KeyMax = MAX_KEY_NUM;
		E2P.KeySize = KEY_SIZE;
		E2P.KeyIndex = 0;
		memset(DeviceList, 0, sizeof(DeviceList));	
		// TBD: E2P.Crypto[4] = ???;		
		memcpy(&E2P.Crypto, &DEV_id[12], 4);		// copy Crypto S/N last 4 bytes
		
		// Write back to EEPROM
		if(PASS != updateHeaderToEeprom()) {
			return FAIL;
		}
		
	} else {
		// Verify crypto S/N 
		if(memcmp(&E2P.Crypto, &DEV_id[12], 4) != 0 ) {
			return FAIL;
		}
		
		if(PASS != loadKeyFromEeprom()) {
			return FAIL;
		}
	}		
	return PASS;
}

/*********************************************************************
* Update Eeprom
*********************************************************************/
Uint8 updateHeaderToEeprom (void) {
	Uint16	i;

	for (i=0; i < EEP_HEADER_SIZE; i++) {
		if(PASS != WriteEEPROM( EEPROM_DEVICE_ADDR, (EEP_HEADER_OFFSET+i), (&E2P.Ver + i), 1)) {
			return FAIL;
		}	
		Delay1ms (EEPROM_DELAY_TIME);
	}	
		
	return PASS;
}

/*********************************************************************
* writeKeyToEeprom
*********************************************************************/
Uint8 writeKeyToEeprom (void) {
	Uint16	i;
	Uint16 	offset;
	
	offset = EEP_KEY_OFFSET + (E2P.KeyIndex * E2P.KeySize);
	
	for (i=0; i < KEY_SIZE; i++) {
		if(PASS != WriteEEPROM( EEPROM_DEVICE_ADDR, offset++, (DeviceList[E2P.KeyIndex]+i), 1)) {
			return FAIL;
		}	
		Delay1ms (EEPROM_DELAY_TIME);
	}	

	E2P.KeyIndex++;
	updateHeaderToEeprom();
	return PASS;
}


/*********************************************************************
* loadKeyFromEeprom
*********************************************************************/
Uint8 loadKeyFromEeprom (void) {
	// Load the all FOB S/N into list
	if(E2P.KeyIndex > 0) {
		if( PASS != ReadEEPROM(EEPROM_DEVICE_ADDR, EEP_KEY_OFFSET, DeviceList, E2P.KeySize*E2P.KeyIndex ) ) {
			return FAIL;
		}
		Delay1ms (5);	
	}
	return PASS;
}

/*********************************************************************
* main loop
*********************************************************************/
void main(void)
{	
		PCON &=	~0x10;	//PWE=0
  
		InitMCU();
	
		InitTimer0();
	
		InitUART0_Timer1();
	
		SHA_I2C_Init();
	
		Delay100us(10);
	
		TR0=ENABLE;	//Timer0 on
		EA=ENABLE;	//enable interrupt
	
	   // Enable eraseEeprom() to clear Header of All Device Keys
		// eraseEeprom(); while(1);
	
		I2CSha204Wakeup();
		if(ShaReadSn(ShaBuff)) {
			memcpy(DEV_id, &ShaBuff[1], sizeof(DEV_id));  
			UrPrint(DEV_id, sizeof(DEV_id));
			Delay10ms ( 50 );
		}
		I2CSha204Sleep();
				
		ResetState();
		
		Flag_MASTER	= FALSE;
		
	if(Flag_MASTER)
	{
		// User FOB code
		// (1) Initial for User SN 
		memcpy(DEV_sn, &DEV_id[0], 4);  
		memcpy(&DEV_sn[4], &DEV_id[8], 4);  
//		UrPrint(DEV_sn, sizeof(DEV_sn));
//		Delay10ms ( 50);
		
		// (2) Initial RF
		InitRF();	//init RF
		ERFINT=ENABLE;	//enable RF	interrupt
		A9112_WriteReg(INTSW_REG, Enable_RFINT_WTR);	//enable WTR interrupt
		Seq	= 0;
		FreqSet(HopTab[Seq]);
		
		// (3) Set system default state
		sysState = State_Standby;		
		
		while(1)
		{
			if(sysState == State_Standby) {     // User side 
				BTN1_Tripped = 0;
				BTN2_Tripped = 0;
				// Setup KEY ISR before into sleep mode
				setWakeUpKeyIsr();
				// Into PM2 mode
				A9112_PM(PM2);
				
				InitMCU();
				InitTimer0();
				// InitUART0_SBRG();	
				SHA_I2C_Init();

				Delay100us ( 2);  
				TR0 = 1;  //Timer0 on
				EA = 1;   //enable interrupt				
				// LED1 = 0;
			}
			
			if(BTN1_Tripped) {
				BTN1_Tripped = 0;	
				TX_cmd = User_Command_1;
				// Prepare TX buffer and send it out (RF)		  
				RF_sendData ( State_Request, DEV_sn, sizeof(DEV_sn));  //write data to tx fifo   
			}
			
			if(BTN2_Tripped) {
				BTN2_Tripped = 0;	
				TX_cmd = User_Command_2;
				// Prepare TX buffer and send it out (RF)		  
				RF_sendData ( State_Request, DEV_sn, sizeof(DEV_sn));  //write data to tx fifo   
				
			}

			StrobeCMD(CMD_RX);
			timer=0;
			TimeoutFlag=0;
			RF_FLAG=1;
			
			while((RF_FLAG==1) && (TimeoutFlag==0));	//wait receive completed or	timeout
			
			if ( TimeoutFlag ) {
				ResetState();
				StrobeCMD (CMD_STBY);
			} else 	{
				RxPacket();
				// Delay10ms ( 10);
			}					
		}  // end of while(1) loop
	}
	else
	{
		// Door side codes
		running_mode = Operation_Mode;
		
						// P1 = P1 & 0x31;
		
		// (1) Preload User S/N into list
		memset(DeviceList, 0, sizeof(DeviceList));
		if( PASS != loadEeprom()){
			while(1);				// EEPROM Read/Write ERROR
		}
		
		// (2) GPIO 
		LED_G = ON;			
		
		UrPrint(&E2P.Ver, EEP_HEADER_SIZE);
		Delay10ms (50);
		if(E2P.KeyIndex > 0) {
			UrPrint(&DeviceList[0][0], KEY_SIZE * E2P.KeyIndex);
			Delay10ms (50);			
		}
		
		// (3) Initial RF
		InitRF();	//init RF
		ERFINT=ENABLE;	//enable RF	interrupt
		A9112_WriteReg(INTSW_REG, Enable_RFINT_WTR);	//enable WTR interrupt

		Seq	= 0;
		//RxCnt =	0;
		//Err_ByteCnt	= 0;
		//Err_BitCnt = 0;
		FreqSet(HopTab[Seq]);
		// setWakeUpKeyIsr();
		
		while(1)
		{
			// Erase Key
			if(BTN2 ==0) {
				// Wait until BTN be released 
				while (!BTN2) {
					// Do erase EEPROM Header and Data
					eraseEeprom();
					loadEeprom();
					// Success LED
					LED_R = ~LED_R; Delay10ms (5); 
					LED_R = ~LED_R; Delay10ms (10); 
					LED_R = ~LED_R; Delay10ms (5); 
					LED_R = ~LED_R; 
				}
			}
			
			// Learning Key
			if(BTN1 ==0) {
				btn_push_counting = 1;				
				// Wait until BTN be released 
				while (!BTN1) {
					// Let LED_R blink
					if( btn_push_counting >= Learning_Mode_Seconds){
						LED_R = btn_push_counting & 1;
					}
				}
				
				if( running_mode == Operation_Mode ) {
					if(btn_push_counting >= Learning_Mode_Seconds) {
						LED_R = ON; LED_G = OFF;
						running_mode = Learning_Mode;
					}
				} else {
					running_mode = Operation_Mode;
					LED_R = OFF; LED_G = ON;
				}
				btn_push_counting = 0;
			}
			
			StrobeCMD(CMD_RX);
			
			timer=0;
			TimeoutFlag=0;
			RF_FLAG=1;
			
			while((RF_FLAG==1) && (TimeoutFlag==0));	//wait receive completed or	timeout
			
			if ( TimeoutFlag ) {
//				if( running_mode != Learning_Mode) {
//					ResetState();		// No response from FOB, reset state
//				}
				StrobeCMD (CMD_STBY);
			} else 	{
				RxPacket();
				// Delay10ms ( 10);
			}				
		}
	}		// end of else 
}			// end of main()

/************************************************************************
**	UrPrint
************************************************************************/
void UrPrint (Uint8 *_ptr, Uint8 _len) {
	if(_len) {
		memcpy ( uartBuf, _ptr, _len );		
		UartSendCnt = _len;
		Uartptr = &uartBuf[0];
		SBUF = uartBuf[0];
	}
}

/************************************************************************
**	setDoorOpen
************************************************************************/
void setDoorOpen(void) {
	BUZZER =1;
	
	// Debug message on UART
//	RxBuff[0]='O';
//	RxBuff[1]='p';
//	RxBuff[2]='e';
//	RxBuff[3]='n';
//	
//	UrPrint(RxBuff, 4);
//	Delay10ms ( 50); 
}

/************************************************************************
**	setDoorClose
************************************************************************/
void setDoorClose(void) {
	BUZZER = 0;
	
	// Debug message on UART
//	RxBuff[0]='C';
//	RxBuff[1]='l';
//	RxBuff[2]='o';
//	RxBuff[3]='s';
//	RxBuff[4]='e';
//	UrPrint(RxBuff, 5);
//	Delay10ms ( 50); 
}

/************************************************************************
**	A9112_WriteFIFO
************************************************************************/
void A9112_WriteFIFO(void)
{
//	Uint16 i;

//	ResetCMD(TXPOINT_RST);	//TX FIFO address pointer reset
//	for(i=0; i<64; i++)
//	{
//		A9112_WriteReg((i +	TX_FIFO), PN9_Tab[i]);
//	}
}

/************************************************************************
**  A9112_WriteFIFO
************************************************************************/
void RF_sendData ( Uint8 _stat, Uint8 *buf, Uint8 bLen )
{
	Uint16 i;
	memset(TxBuff,0, sizeof(TxBuff));
	TxBuff[0] = bLen+2;    // Add Count & State
	TxBuff[1] = _stat;            
	memcpy(&TxBuff[2], buf, bLen);

	// Dump send data here
//   UrPrint(TxBuff, TxBuff[0]);
//   Delay10ms (50);  

	// RF_sendData ( TxBuff[0]);  //write data to tx fifo          
	ResetCMD(TXPOINT_RST); //TX FIFO address pointer reset
	for ( i = 0; i < TxBuff[0]; i++ ) {
		A9112_WriteReg ( ( i + TX_FIFO ), TxBuff[i] );
	}

	StrobeCMD ( CMD_TX );
	RF_FLAG = 1;
	while ( RF_FLAG );    //wait transmit completed
	Delay100us (2);  //delay 200us for TX ramp down

}

/************************************************************************
**	RxPacket
************************************************************************/
void RxPacket(void)
{
	Uint16  i;
	
	Uint8   idx;
	//Uint8   fault;
	//Uint8		xdata tt [] = { 0xd, 0xa, 1,2,3,4,5,0xd,0xa};

	memset(RxBuff, 0, sizeof(RxBuff));
	RxCnt++;
	ResetCMD (RXPOINT_RST); //RX FIFO address pointer reset
	RxLen =   A9112_ReadReg ( RX_FIFO );

	if(RxLen) {
		sysState = A9112_ReadReg ( 1 + RX_FIFO );
		for ( i = 2; i < RxLen; i++ ) {
			RxBuff[i-2] = A9112_ReadReg ( i + RX_FIFO );
		}
	}

	// Dump RxBuff
//	UrPrint(RxBuff, sizeof(RxBuff));
//	Delay10ms ( 50);  
	
	if(sysState == State_Request) {     // Host(Door) side receive this packet
		// check if SN is accepted, if yes, send RND_b
		if(RxBuff[0] != 0x01) 
			return ;
		idx = isSnInChecklist(RxBuff, sizeof(DEV_sn));
		
		if( running_mode == Learning_Mode) {
			if(idx>=MAX_KEY_NUM) { 
				// Not in the DeviceList, add new SN into List
				// memcpy(DeviceList[numberOfKey++], RxBuff, sizeof(DEV_sn)); 
				memcpy(DeviceList[E2P.KeyIndex], RxBuff, sizeof(DEV_sn)); 
//				UrPrint(RxBuff, sizeof(DEV_sn));
//				Delay10ms (50); 
				if(PASS != writeKeyToEeprom()) {
					// if update failure, remove key data
					memcpy(DeviceList[E2P.KeyIndex], 0, sizeof(DEV_sn)); 
				}
				
				// Success LED
				LED_R = ~LED_R; Delay10ms (5); 
				LED_R = ~LED_R; Delay10ms (10); 
				LED_R = ~LED_R; Delay10ms (5); 
				LED_R = ~LED_R; 
				
			}
		} else {
			if(idx < MAX_KEY_NUM) { 
				// A FOB request is here, reset internal status
				ResetState();	
				
				// if found, copy S/N into DEV_sn
				memcpy(DEV_sn, DeviceList[idx], sizeof(DEV_sn)); 
//				UrPrint(DEV_sn, sizeof(DEV_sn));
//				Delay10ms (50); 
			
				// Get RND_b
				I2CSha204Wakeup();
				if(ShaReadRnd(ShaBuff)) {        // The first byte is size
					ShaBuff[0] = idx;              // Put SN index number			
					// fix RND_b for debugging
					// memset(&ShaBuff[1], 0, 32);	// enable this line to fix the RND_b		
					memcpy(RND_b, &ShaBuff[1], sizeof(RND_b));
					
					// Send idx+RND_b to User			
					RF_sendData ( State_Challenge, ShaBuff, sizeof(RND_b)+1 );   
//					UrPrint(ShaBuff, sizeof(ShaBuff));
//					Delay10ms (50); 
				}				
				I2CSha204Sleep();
			}		// end of if(idx < MAX_KEY_NUM) { 
		}			// end of if( running_mode == Learning_Mode) {
		// LED1 = OFF;
		return;    
	}

	if(sysState == State_Challenge) {     // User side 
		// Get idx and RND_b, then calculate the ANS_a
		LED1 = ON;
		idx = RxBuff[0];
		memcpy(RND_b, &RxBuff[1], sizeof(RND_b));  
	
//		UrPrint(RND_b, sizeof(RND_b));
//		Delay10ms ( 50); 

		if(calculateANS_a()) {
			// Move idx +  CMD + ANS_a to TxBuff, then send it out
			memset(ShaBuff, 0, sizeof(ShaBuff));
			ShaBuff[0] = idx;
			ShaBuff[1] = TX_cmd;
			memcpy(&ShaBuff[2], ANS_a, sizeof(ANS_a));  
			RF_sendData ( State_Response, ShaBuff, sizeof(ANS_a)+2 );   
//			Delay10ms ( 50);  
//			UrPrint(TxBuff, TxBuff[0]);			
		}
		LED1=OFF;
		sysState = State_Standby;
		return;    
	}

	if(sysState == State_Response) {     // Host side 
		// FOB replied idx, TX_cmd & ANS_a to Host
		// So, host gets idx, read S/N from DeviceList, then calculate the ANS_b
		idx = RxBuff[0];
		TX_cmd = RxBuff[1];
		
		if(idx <MAX_KEY_NUM) {
			memcpy(DEV_sn, DeviceList[idx], sizeof(DEV_sn));  
			memcpy(ANS_a, &RxBuff[2], sizeof(ANS_a));       						
			
			// Calculate ANS_b
			if(calculateANS_b()){   
//				UrPrint(ANS_a, sizeof(ANS_a));
//				Delay10ms ( 100);  				
//				UrPrint(ANS_b, sizeof(ANS_b));
//				Delay10ms ( 100); 
				// Compare with ANS_a and ANS_b
				// Delay10ms ( 50);  UrPrint(tt, sizeof(tt));	
				if(!memcmp(ANS_a, ANS_b, sizeof(ANS_b))) {
					// if same, execute TX_cmd here
					if(TX_cmd==User_Command_1) {						
						setDoorOpen();
					}
					if(TX_cmd==User_Command_2) {
						setDoorClose();
					}
				}	
			}
			 
		}  		// end of if(idx ...
		// Reset to Stnadby
		ResetState();
	}    // end of if(stat == State_Response) {   
}

/****************************************************
** Crypto Functions
****************************************************/
Uint8 calculateANS_a() {
	Uint8 Okey =0;
 
//	UrPrint(DEV_sn, sizeof(DEV_sn));
//	Delay10ms ( 50);  
	
	step0_mixSnIntoRnd_b();
	
//	Delay10ms ( 50);  
//	UrPrint(RND_b, sizeof(RND_b));
//	Delay10ms ( 50);  
//	UrPrint(tt, 2);
	
	I2CSha204Wakeup();
	
	// Step 1: Get Mac (Slot0+SN)
	if(step1_getMACfromSlot0(ANS_a, sizeof(ANS_a), RND_b, sizeof(RND_b))){		
			Okey= 1;
	}
	
	I2CSha204Sleep();
		

	return Okey;
		
}

Uint8 calculateANS_b() {
	Uint8 Okey =0;
	//Uint8	xdata tt [] = { 0xd, 0xa};
	
//	UrPrint(DEV_sn, sizeof(DEV_sn));
//	Delay10ms ( 50);  
	
	step0_mixSnIntoRnd_b();
	
//	Delay10ms ( 50);  
//	UrPrint(RND_b, sizeof(RND_b));
	
	I2CSha204Wakeup();
	
	// Step 1: Get Mac (Slot0+SN)  
	if(step1_getMACfromSlot0(ANS_b, sizeof(ANS_b), RND_b, sizeof(RND_b))){		
		Okey = 1;
	}
	I2CSha204Sleep();
	
//	Delay10ms ( 50);  UrPrint(tt, 2);
//	Delay10ms ( 50);  UrPrint(ANS_a, sizeof(ANS_a));
//	Delay10ms ( 50);  UrPrint(tt, 2);
//	Delay10ms ( 50);  UrPrint(ANS_b, sizeof(ANS_b));
//	Delay10ms ( 50);  UrPrint(tt, 2);
	
	
	return Okey;
}

/* Function tested !!*/
Uint8 isSnInChecklist(Uint8 *buff, Uint8 _len){
	Uint8 i;	

	for(i=0; i<MAX_KEY_NUM; i++){
		if(DeviceList[i][0] != 0x01) {
			return MAX_KEY_NUM;
		}
		if(!memcmp(DeviceList[i], buff, _len)) {		
			// Found!!
			return i;
		}
	}
	return MAX_KEY_NUM;
}

void setWakeUpKeyIsr (void){
	// P3WUN=0xF3; // P3_2, P3_3 are enabled 
	EKEYINT=1;	
}

/************************************************************************
**  KEYISR
************************************************************************/
void KEYISR ( void ) interrupt 11
{
	EIF |= 0x10;		// Clear Flag
	EKEYINT=0;
	
	// check which bottom is pressed
	if(BTN1 ==0) {
		BTN1_Tripped = 1;
	}
	if(BTN2 ==0) {
		BTN2_Tripped = 1;
	}	
	/*
	P1OE |= 0x40;
	P1PUN |= 0x40;
	LED2 = 1;*/
}

/************************************************************************
**	RTC_Enable
************************************************************************/
void RTC_Enable(void)
{
	IOSEL		|= 0x10;	//enable RTCIOS
	P3OE		&= 0x3F;
	P3PUN		&= 0x3F;

	//A9112_WriteReg(CKO_REG, 0x50);	//ROSC

	A9112_WriteReg(RCOSC3_REG, A9112Config[RCOSC3_REG -	CONFIG]	| 0x04);	//TMRE=1
	A9112_WriteReg(RTC_REG,	0x01);	//RTCE=1
	A9112_WriteReg(RCOSC4_REG, A9112Config[RCOSC4_REG -	CONFIG]	| 0x80);	//RCOT[2]=1
	
	//delay	for	RTC	stabilized
	//...
	//...
}

/*********************************************************************
** WOR_enable_by_preamble
*********************************************************************/
void WOR_enable_by_preamble(void)
{
	RCOSC_Cal();		//internal RC OSC CAL
	//RTC_Enable();		//use external RC OSC
		
	//A9112_WriteReg(CKO_REG, 0x08);	//CKO=RCK
	//A9112_WriteReg(GPIO1_REG,	(A9112Config[GPIO1_REG - CONFIG] & 0xC0) | 0x0D);	//GPIO1=PMDO
	//A9112_WriteReg(GPIO1_REG,	(A9112Config[GPIO1_REG - CONFIG] & 0xC0) | 0x11);	//GPIO1=TWWS
	//A9112_WriteReg(GPIO2_REG,	(A9112Config[GPIO2_REG - CONFIG] & 0xC0) | 0x01);	//GPIO2=WTR

	//setup	WOR	Sleep time and Rx time
	//WOR Active Period	= (WOR_AC[8:0]+1) x	(1/32768), (30.5us ~ 15.6ms).
	//WOR Sleep	 Period	= (WOR_SL[9:0]+1) x	(32/4096), (7.8ms ~	7.99s).
	A9112_WriteReg(RCOSC1_REG, 0x7F);	//WOR_SL[9:0]=127 :	WOR	Sleep  Period ~	1s 
	A9112_WriteReg(RCOSC2_REG, 0x00);
	A9112_WriteReg(TCODE_REG, 0x80);	//WOR_AC[8:0]=256 :	WOR	Active Period ~	7.84ms

	//WOR_HOLD : WOR hold RX setting when preamble/sync/carrier	detected ok.
	//			[0]: No	hold. [1]: Hold	RX.
	//WOR_SEL: TWWS=1 setting. 
	//			[0]:RX valid packet.  [1]: preamble/sync/carrier  detected ok.
	//WOR_RST: Shall be	set	to [1].
	A9112_WriteReg(WOR_REG,	0x3A);	//by preamble, WOR_HOLD=1, WOR_SEL=1, WOR_RST=1

	StrobeCMD(CMD_STBY);

	ERFINT=ENABLE;	//enable RF	interrupt	
	A9112_WriteReg(INTSW_REG, Enable_RFINT_TWWS);	//enable TWWS INT	

	A9112_WriteReg(MODEC2_REG, A9112Config[MODEC2_REG -	CONFIG]	| 0x08);	//enable WOR function

	A9112_PM(PM1);
	//A9112_PM(PM2);
	
	//WOR_SEL=1	: preamble/sync/carrier	detected ok	-> TWWS	go high	-> wake	up MCU
	//defined process by user
	//...
	//...
	//StrobeCMD(CMD_STBY);	//TWWS reset by	strobe command.
}

/*********************************************************************
** WOR_enable_by_sync
*********************************************************************/
void WOR_enable_by_sync(void)
{
	RCOSC_Cal();		//internal RC OSC CAL
	//RTC_Enable();		//use external RC OSC
		
	//A9112_WriteReg(CKO_REG, 0x08);	//CKO=RCK
	//A9112_WriteReg(GPIO1_REG,	(A9112Config[GPIO1_REG - CONFIG] & 0xC0) | 0x05);	//GPIO1=FSYNC
	//A9112_WriteReg(GPIO1_REG,	(A9112Config[GPIO1_REG - CONFIG] & 0xC0) | 0x11);	//GPIO1=TWWS
	//A9112_WriteReg(GPIO2_REG,	(A9112Config[GPIO2_REG - CONFIG] & 0xC0) | 0x01);	//GPIO2=WTR

	//setup	WOR	Sleep time and Rx time
	//WOR Active Period	= (WOR_AC[8:0]+1) x	(1/32768), (30.5us ~ 15.6ms).
	//WOR Sleep	 Period	= (WOR_SL[9:0]+1) x	(32/4096), (7.8ms ~	7.99s).
	A9112_WriteReg(RCOSC1_REG, 0x7F);	//WOR_SL[9:0]=127 :	WOR	Sleep  Period ~	1s	
	A9112_WriteReg(RCOSC2_REG, 0x00);
	A9112_WriteReg(TCODE_REG, 0x80);	//WOR_AC[8:0]=256 :	WOR	Active Period ~	7.84ms

	//WOR_HOLD : WOR hold RX setting when preamble/sync/carrier	detected ok.
	//			[0]: No	hold. [1]: Hold	RX.
	//WOR_SEL: TWWS=1 setting. 
	//			[0]:RX valid packet.  [1]: preamble/sync/carrier  detected ok.
	//WOR_RST: Shall be	set	to [1].
	A9112_WriteReg(WOR_REG,	0x28);	//by sync, WOR_HOLD=1, WOR_SEL=0, WOR_RST=1

	StrobeCMD(CMD_STBY);

	ERFINT=ENABLE;	 //enable RF interrupt	 
	A9112_WriteReg(INTSW_REG, Enable_RFINT_TWWS);	//enable TWWS INT	

	A9112_WriteReg(MODEC2_REG, A9112Config[MODEC2_REG -	CONFIG]	| 0x08);	//enable WOR function

	A9112_PM(PM1);
	//A9112_PM(PM2);

	//WOR_SEL=0	: RX valid packet -> TWWS go high -> wake up MCU
	//check	data
	RxPacket();
	StrobeCMD(CMD_STBY);	//TWWS reset by	strobe command.
}

/*********************************************************************
** WOR_enable_by_carrier
*********************************************************************/
void WOR_enable_by_carrier(void)
{
	RCOSC_Cal();		//internal RC OSC CAL
	//RTC_Enable();		//use external RC OSC
		
	//A9112_WriteReg(CKO_REG, 0x08);	//CKO=RCK
	//A9112_WriteReg(GPIO1_REG,	(A9112Config[GPIO1_REG - CONFIG] & 0xC0) | 0x09);	//GPIO1=CD
	//A9112_WriteReg(GPIO1_REG,	(A9112Config[GPIO1_REG - CONFIG] & 0xC0) | 0x11);	//GPIO1=TWWS
	//A9112_WriteReg(GPIO2_REG,	(A9112Config[GPIO2_REG - CONFIG] & 0xC0) | 0x01);	//GPIO2=WTR

	//setup	WOR	Sleep time and Rx time
	//WOR Active Period	= (WOR_AC[8:0]+1) x	(1/32768), (30.5us ~ 15.6ms).
	//WOR Sleep	 Period	= (WOR_SL[9:0]+1) x	(32/4096), (7.8ms ~	7.99s).
	A9112_WriteReg(RCOSC1_REG, 0x7F);	//WOR_SL[9:0]=127 :	WOR	Sleep  Period ~	1s	
	A9112_WriteReg(RCOSC2_REG, 0x00);
	A9112_WriteReg(TCODE_REG, 0x80);	//WOR_AC[8:0]=256 :	WOR	Active Period ~	7.84ms

	//WOR_HOLD : WOR hold RX setting when preamble/sync/carrier	detected ok.
	//			[0]: No	hold. [1]: Hold	RX.
	//WOR_SEL: TWWS=1 setting. 
	//			[0]:RX valid packet.  [1]: preamble/sync/carrier  detected ok.
	//WOR_RST: Shall be	set	to [1].
	A9112_WriteReg(WOR_REG,	0x3C);	//by carrier, WOR_HOLD=1, WOR_SEL=1, WOR_RST=1
	
	A9112_WriteReg(CDET1_REG, 0x00);	//RSSI carrier detect
	A9112_WriteReg(ADCC_REG, 0xC5);		//CDM=1, ADC average=16	times
	A9112_WriteReg(RSSI_REG, 0x91);		//RTH=145 (-100dBm)

	StrobeCMD(CMD_STBY);

	ERFINT=ENABLE;	//enable RF	interrupt	
	A9112_WriteReg(INTSW_REG, Enable_RFINT_TWWS);	//enable TWWS INT	

	A9112_WriteReg(MODEC2_REG, A9112Config[MODEC2_REG -	CONFIG]	| 0x48);	//enable WOR function &	ARSSI=1

	A9112_PM(PM1);
	//A9112_PM(PM2);
	
	//WOR_SEL=1	: preamble/sync/carrier	detected ok	-> TWWS	go high	-> wake	up MCU
	//defined process by user
	//...
	//...
	//StrobeCMD(CMD_STBY);	//TWWS reset by	strobe command.
}

/*********************************************************************
** WOT_enable
*********************************************************************/
void WOT_enable(void)
{
	RCOSC_Cal();		//internal RC OSC CAL
	//RTC_Enable();		//use external RC OSC

	//A9112_WriteReg(CKO_REG, 0x00);	//CKO=DCK
	//A9112_WriteReg(GPIO1_REG,	(A9112Config[GPIO1_REG - CONFIG] & 0xC0) | 0x05);	//GPIO1=FSYNC
	//A9112_WriteReg(GPIO1_REG,	(A9112Config[GPIO1_REG - CONFIG] & 0xC0) | 0x11);	//GPIO1=TWWS
	//A9112_WriteReg(GPIO2_REG,	(A9112Config[GPIO2_REG - CONFIG] & 0xC0) | 0x01);	//GPIO2=WTR

	//setup	WOT	Sleep time
	//WOT Sleep	 Period	= (WOR_SL[9:0]+1) x	(32/4096), (7.8ms ~	7.99s).
	A9112_WriteReg(RCOSC1_REG, 0x7F);	//WOR_SL[9:0]=127 :	WOT	Sleep  Period ~	1s 
	A9112_WriteReg(RCOSC2_REG, 0x00);

	//WOR_RST: Shall be	set	to [1].
	A9112_WriteReg(WOR_REG,	0x21);	//WMODE=1, WOR_RST=1
	
	StrobeCMD(CMD_STBY);
	A9112_WriteFIFO();

	ERFINT=ENABLE;	//enable RF	interrupt	
	A9112_WriteReg(INTSW_REG, Enable_RFINT_TWWS);	//enable TWWS INT	

	A9112_WriteReg(MODEC2_REG, A9112Config[MODEC2_REG -	CONFIG]	| 0x08);	//enable WOT function

	A9112_PM(PM1);
	//A9112_PM(PM2);
	
	//Transmit completed ->	TWWS go	high ->	wake up	MCU
	//defined process by user
	//...
	//...
	//A9112_WriteReg(MODEC2_REG, A9112Config[MODEC2_REG	- CONFIG]);			//disable WOT function
	//StrobeCMD(CMD_STBY);	//TWWS reset by	strobe command.
}

/*********************************************************************
** TWOR_enable
*********************************************************************/
void TWOR_enable(void)
{
	RCOSC_Cal();		//internal RC OSC CAL
	//RTC_Enable();		//use external RC OSC

	//A9112_WriteReg(GPIO1_REG,	(A9112Config[GPIO1_REG - CONFIG] & 0xC0) | 0x11);	//GPIO1=TWWS
	//A9112_WriteReg(GPIO2_REG,	(A9112Config[GPIO2_REG - CONFIG] & 0xC0) | 0x11);	//GPIO2=TWWS

	//WOR_AC Period	= (WOR_AC[8:0]+1) x	(1/4096), (244us ~ 125ms).
	//WOR_SL Period	= (WOR_SL[9:0]+1) x	(32/4096), (7.8ms ~	7.99s).
	//Note : TWOR function which enables this device to	output a periodic square wave from GPIO	to a MCU
	//							_____		_____		_____
	//		TWWS signal	: _____|	 |_____|	 |_____|	 |
	//						1s	  1s	1s	  1s	1s	  1s 
	//
	//		TWWS interrupt are activated by	a rising edge.	 
	//		first time,	interrupt interval=1s
	//		other times, interrupt interval=2s
	
	A9112_WriteReg(RCOSC1_REG, 0x7F);	//WOR_SL[9:0]=127 :	WOR_SL Period ~	1s	
	A9112_WriteReg(RCOSC2_REG, 0x19);
	A9112_WriteReg(TCODE_REG, 0xC0);	//WOR_AC[8:0]=409 :	WOR_AC Period ~	0.1s

	ERFINT=ENABLE;	//enable RF	interrupt	
	A9112_WriteReg(INTSW_REG, Enable_RFINT_TWWS);	//enable TWWS INT

	//A9112_WriteReg(RCOSC3_REG, A9112Config[RCOSC3_REG	- CONFIG] |	0x05);	//enable TWOR function by WOR_AC
	A9112_WriteReg(RCOSC3_REG, A9112Config[RCOSC3_REG -	CONFIG]	| 0x07);	//enable TWOR function by WOR_SL

	A9112_PM(PM1);
	//A9112_PM(PM2);
	
	//TWWS go high -> wake up MCU
	//defined process by user
	//...
	//...
	//A9112_WriteReg(RCOSC3_REG, A9112Config[RCOSC3_REG	- CONFIG] |	0x04);	//disable TWOR function
}

/*********************************************************************
** TMR_enable
*********************************************************************/
void TMR_enable(void)
{
	RCOSC_Cal();		//internal RC OSC CAL
	//RTC_Enable();		//use external RC OSC

	//TMR Period = (1 /	(2 ^ (7-TMRCKS[2:0]) ) ) * (TMRINV + 1)
	//ex : TMRCKS=7, TMRINV=0
	//	   TMR Period =	(1 / (2	^ (7-7)	) )	* (0+1)	= 1/1 =	1s

	A9112_WriteReg(TMRINV_REG, 0x00);	//TMRINV=0
 
	//A9112_WriteReg(TMRCTL_REG, 0xE1);	//TMRCKS=128Hz
	//A9112_WriteReg(TMRCTL_REG, 0xE3);	//TMRCKS=64Hz
	//A9112_WriteReg(TMRCTL_REG, 0xE5);	//TMRCKS=32Hz
	//A9112_WriteReg(TMRCTL_REG, 0xE7);	//TMRCKS=16Hz
	//A9112_WriteReg(TMRCTL_REG, 0xE9);	//TMRCKS=8Hz
	//A9112_WriteReg(TMRCTL_REG, 0xEB);	//TMRCKS=4Hz
	//A9112_WriteReg(TMRCTL_REG, 0xED);	//TMRCKS=2Hz
	A9112_WriteReg(TMRCTL_REG, 0xEF);	//TMRCKS=1Hz
	
	ERFINT=ENABLE;	//enable RF	interrupt
	A9112_WriteReg(INTSW_REG, Enable_RFINT_TMR);	//enable TMR INT

	A9112_PM(PM1);
	//A9112_PM(PM2);
	//A9112_PM(PM3);
	
	//defined process by user
	//...
	//...
}

/*********************************************************************
** SAR_ADC_12B
*********************************************************************/
void SAR_ADC_12B(void)
{
	Uint16 MVADC_value;
	//Uint8	tmp;
	
	//ADC Input	Setting
	IOSEL =	(IOSEL & 0x3F)	|0x20;	//enable ADCIOS
	
	//ADC input=P3.2
	IOSEL	&= 0x3F;
	P3OE	&= ~0x04;
	P3PUN	|= 0x04;
	
	///ADC input=P3.3
	//IOSEL	= (IOSEL & 0x3F) | 0x40;
	//P3OE &= ~0x08;
	//P3PUN	|= 0x08;
	
	///ADC input=P3.4
	//IOSEL	= (IOSEL & 0x3F) | 0x80;
	//P3OE &= ~0x10;
	//P3PUN	|= 0x10;
	
	///ADC input=P3.5
	//IOSEL	= (IOSEL & 0x3F) | 0xC0;
	//P3OE &= ~0x20;
	//P3PUN	|= 0x20;

	A9112_WriteReg(ADCC_REG, A9112Config[ADCC_REG -	CONFIG]	| 0x02);	//set XADSR
	
	ERFINT=ENABLE;	//enable RF	interrupt
	A9112_WriteReg(INTSW_REG, Enable_RFINT_12BADC);	//enable ADCE INT
	
	while(1)
	{
		A9112_WriteReg(ADCCTL_REG, A9112Config[ADCCTL_REG -	CONFIG]	| 0x01);	//Start	ADC	conversion,	single mode
		//do{
		//	tmp	= A9112_ReadReg(ADCCTL_REG);	//wait ADC conversion complete
		//}while(tmp & 0x01);
		
		RF_FLAG=1;
		while(RF_FLAG);		//wait ADC conversion complete

		//read MVADC
		MVADC_value	= (A9112_ReadReg(ADCAVG1_REG) &	0xF0);
		MVADC_value	<<=	4;
		MVADC_value	= MVADC_value +	A9112_ReadReg(ADCAVG2_REG);
	}
}

/*********************************************************************
** RSSI_measurement
*********************************************************************/
void RSSI_measurement(void)
{
	Uint8 tmp;

	StrobeCMD(CMD_STBY);
	
	//A9112_WriteReg(GPIO1_REG,	(A9112Config[GPIO1_REG - CONFIG] & 0xC0) | 0x05);	//GPIO1=FSYNC
	//A9112_WriteReg(GPIO2_REG,	(A9112Config[GPIO2_REG - CONFIG] & 0xC0) | 0x01);	//GPIO2=WTR

	A9112_WriteReg(ADCC_REG, 0xC5);		//CDM=1, ADC average=16	times	
	A9112_WriteReg(MODEC2_REG, A9112Config[MODEC2_REG -	CONFIG]	| 0x40);	//ARSSI=1

	ERFINT=ENABLE;	//enable RF	interrupt
	A9112_WriteReg(INTSW_REG, Enable_RFINT_FSTNC);	  //enable FSYNC interrupt
	
	StrobeCMD(CMD_RX);
	
	RF_FLAG=1;
	while(RF_FLAG)		//Stay in RX mode until	receiving ID code(sync ok)		
	{
		tmp	= A9112_ReadReg(RSSI_REG);	//read RSSI	value(environment RSSI)
	}
	tmp	= A9112_ReadReg(RSSI_REG);		//read RSSI	value(wanted signal	RSSI)
}

/*********************************************************************
** RC_ADC0_24B
*********************************************************************/
void RC_ADC0_24B(void)
{
	//Note : Only A9109
	//RC ADC0 Circuit
	//IC Pin2 IN0 :	Oscillator input pin.
	//IC Pin3 CS0 :	Reference capacitor	connection pin.
	//IC Pin4 RS0 :	Reference resistor connection pin.
	//IC Pin5 RT0 :	Resistor sensor	connection pin for measurement.

	Uint32 ADC_value;

	RCOSC_Cal();		//internal RC OSC CAL
	//RTC_Enable();		//use external RC OSC

	ADCCH =	ADCCH &	~0x01;	//Select RC	ADC0
	
	ResetCMD(RCADC_RST);		//Reset	24B	RC ADC

	//AB MODE
	A9112_WriteReg(RADMOD0_REG,	(A9112Config[RADMOD0_REG - CONFIG] | 0x04));	//RC ADC blase clock=32.768K
	A9112_WriteReg(RADWTC0_REG,	0xFF);	//WTC=255
	A9112_WriteReg(RADCA0_2_REG, 0xFF);	//RA=0xFFE000	
	A9112_WriteReg(RADCA0_1_REG, 0xE0);
	A9112_WriteReg(RADCA0_0_REG, 0x00);
	A9112_WriteReg(RADCB0_2_REG, 0x00);	//RB=0x000000
	A9112_WriteReg(RADCB0_1_REG, 0x00);
	A9112_WriteReg(RADCB0_0_REG, 0x00);
	
	A9112_WriteReg(RADCON0_REG,	A9112Config[RADCON0_REG	- CONFIG] |	0x89);		//Start	RC ADC operation & enable RC ADC interrupt

	ERFINT=1;			//enable RF	interrupt

	A9112_PM(PM1);
  //A9112_PM(PM2);

	//RC ADC, AB mode, conversion time ~0.5sec
	//defined process by user
	//...
	//...

	//read ADC value
	ADC_value =	A9112_ReadReg(RADCA0_2_REG);
	ADC_value =	(ADC_value<<8) + A9112_ReadReg(RADCA0_1_REG);
	ADC_value =	(ADC_value<<8) + A9112_ReadReg(RADCA0_0_REG);
}

/*********************************************************************
** RC_ADC1_24B
*********************************************************************/
void RC_ADC1_24B(void)
{
	//Note : Only A9109
	//RC ADC1 Circuit
	//IC Pin26 P1.0/IN1	: Oscillator input pin.
	//IC Pin27 P1.1/CS1	: Reference	capacitor connection pin.
	//IC Pin28 P1.2/RS1	: Reference	resistor connection	pin.
	//IC Pin29 P1.3/RT1	: Resistor sensor connection pin for measurement.

	Uint32 ADC_value;

	RCOSC_Cal();		//internal RC OSC CAL
	//RTC_Enable();		//use external RC OSC

	ADCCH	= ADCCH	| 0x01;	//Select RC	ADC1
	P1OE	= P1OE & 0xF0;		//P1.0~P1.3	as input, pull-high
	P1PUN	= P1PUN	& 0xF0;		//P1.0~P1.3	as input, pull-high
	
	ResetCMD(RCADC_RST);		//Reset	24B	RC ADC

	//AB MODE
	A9112_WriteReg(RADMOD1_REG,	(A9112Config[RADMOD1_REG - CONFIG] | 0x04));	//RC ADC blase clock=32.768K
	A9112_WriteReg(RADWTC1_REG,	0xFF);	//WTC=255
	A9112_WriteReg(RADCA1_2_REG, 0xFF);	//RA=0xFFE000	
	A9112_WriteReg(RADCA1_1_REG, 0xE0);
	A9112_WriteReg(RADCA1_0_REG, 0x00);
	A9112_WriteReg(RADCB1_2_REG, 0x00);	//RB=0x000000
	A9112_WriteReg(RADCB1_1_REG, 0x00);
	A9112_WriteReg(RADCB1_0_REG, 0x00);
	
	A9112_WriteReg(RADCON1_REG,	A9112Config[RADCON1_REG	- CONFIG] |	0x89);		//Start	RC ADC operation & enable RC ADC interrupt

	ERFINT=1;			//enable RF	interrupt

	A9112_PM(PM1);
  //A9112_PM(PM2);

	//RC ADC, AB mode, conversion time ~0.5sec
	//defined process by user
	//...
	//...

	//read ADC value
	ADC_value =	A9112_ReadReg(RADCA1_2_REG);
	ADC_value =	(ADC_value<<8) + A9112_ReadReg(RADCA1_1_REG);
	ADC_value =	(ADC_value<<8) + A9112_ReadReg(RADCA1_0_REG);
}
/************************************************************************
** UserRegister
************************************************************************/
void UserRegister(void)
{	

		#ifdef RF_DR10Kbps_50KIFBW 

		A9112_WriteReg(MODEC2_REG   			,0x02			);
		A9112_WriteReg(CACL_REG     			,0x00			);
		A9112_WriteReg(FIFO1_REG    			,0x3F			);
		A9112_WriteReg(FIFO2_REG    			,0x00			);

		A9112_WriteReg(CKO_REG      			,0x00			);
		A9112_WriteReg(GPIO1_REG    			,0x05			);
		A9112_WriteReg(GPIO2_REG    			,0x01			);

		
		A9112_WriteReg(TMRINV_REG   			,0x00			);
		A9112_WriteReg(TMRCTL_REG   			,0x00			);

		#endif
	

		#ifdef RF_DR100Kbps_100KIFBW

			A9112_WriteReg(MODEC2_REG   			,0x02	);
			A9112_WriteReg(CACL_REG     			,0x00	);
			A9112_WriteReg(FIFO1_REG    			,0x3F	);
			A9112_WriteReg(FIFO2_REG    			,0x00	);
			
			A9112_WriteReg(CKO_REG      			,0x00	);
			A9112_WriteReg(GPIO1_REG    			,0x05	);
			A9112_WriteReg(GPIO2_REG    			,0x01	);

			
			A9112_WriteReg(TMRINV_REG   			,0x00	);
			A9112_WriteReg(TMRCTL_REG   			,0x00	);
			

		#endif
				
		#ifdef RF_DR150Kbps_150KIFBW

			A9112_WriteReg(MODEC2_REG   			,0x02	);
			A9112_WriteReg(CACL_REG     			,0x00	);
			A9112_WriteReg(FIFO1_REG    			,0x3F	);
			A9112_WriteReg(FIFO2_REG    			,0x00	);
			
			A9112_WriteReg(CKO_REG      			,0x00	);
			A9112_WriteReg(GPIO1_REG    			,0x05	);
			A9112_WriteReg(GPIO2_REG    			,0x81	);

			
			
			A9112_WriteReg(TMRINV_REG   			,0x00	);
			A9112_WriteReg(TMRCTL_REG   			,0x00	);
			

		#endif
	
		#ifdef RF_DR250Kbps_250KIFBW

			A9112_WriteReg(MODEC2_REG   			,0x02);
			A9112_WriteReg(CACL_REG     			,0x00);
			A9112_WriteReg(FIFO1_REG    			,0x3F);
			A9112_WriteReg(FIFO2_REG    			,0x00);
			
			A9112_WriteReg(CKO_REG      			,0x00);
			A9112_WriteReg(GPIO1_REG    			,0x45);
			A9112_WriteReg(GPIO2_REG    			,0x41);

			
			A9112_WriteReg(TMRINV_REG   			,0x00);
			A9112_WriteReg(TMRCTL_REG   			,0x00);
			

		#endif
	


}


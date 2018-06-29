#ifndef _SHA_I2C_C_
#define _SHA_I2C_C_

#include "..\include\System_config.h"

#define I2C_ID_C8_SHA204         0x64
#define SHA_WORD_ADDRESS_RESET   0
#define SHA_WORD_ADDRESS_SLEEP   1
#define SHA_WORD_ADDRESS_IDLE    2
#define SHA_WORD_ADDRESS_NORMAL  3


// *************************
// Extern Function
// *************************
extern   void delay(void);
extern 	void I2C_MasterInit(void);
extern 	Uint8 I2CWriteData(Uint8 id, Uint8 *buf, Uint8 length);
extern 	Uint8 I2CReadData(Uint8 id, Uint8* buf, Uint8 length);

extern   UrPrint(Uint8*, Uint8);

// *************************
// Extern Parameters
// *************************
extern   Uint8   xdata   shabuf[64];
extern   Uint8   xdata   DEV_id[16];
extern   Uint8   xdata   DEV_sn[16];      // Device unique SN
extern   Uint8   xdata   RND_a[32];
extern   Uint8   xdata   RND_b[32];       // get from Host
extern   Uint8   xdata   ANS_a[32];
extern   Uint8   xdata   MAC_my[32];      // User's MAC = sha(Slot 0, S/N)
extern   Uint8   xdata   TX_cmd;
extern   Uint8   xdata   TxBuff[64];
extern   Uint8   xdata   RxBuff[64];
extern   Uint8   xdata   TxLen;
extern   Uint8   xdata   RxLen;


// *************************
// Local Function Decoration 
// *************************
void     SHA_I2C_Init(void);
void     ShaCmdSendRecv(Uint8 *sbuf, Uint8 sLen, Uint8 *rbuf, Uint8 rLen,Uint8 delay);
Uint8    ShaReadSn (Uint8 *rbuf);
Uint8 	ShaReadRnd (Uint8 *rbuf);
Uint8    I2CSha204Wakeup();
Uint8    I2CSha204Sleep();
void 		step0_mixSnIntoRnd_b();
Uint8 	step1_getMACfromSlot0(Uint8 *, Uint8 ,Uint8 *, Uint8 );  
Uint8 	step2_writeToSlot7 (Uint8 *buf, Uint8 pLen);  
Uint8 	step3_getMACfromSlot7(Uint8 *buf, Uint8 pLen); 
void		shaCalCrc(Uint8 *, Uint8 length, Uint8 *crc);


/*********************************************************************
* SHA_I2C_Init
*********************************************************************/
void SHA_I2C_Init(void)
{
   I2C_MasterInit();   
}


/*********************************************************************
* SHA_Read
*********************************************************************/
void ShaCmdSendRecv(Uint8 *sbuf, Uint8 sLen, Uint8 *rbuf, Uint8 rLen, Uint8 dly) {
   if(sbuf==NULL | sLen==0)
      return;
  
   I2CWriteData(I2C_ID_C8_SHA204, sbuf,sLen);
   
	memset(rbuf, 0, rLen);
   Delay10ms ( dly );
   
	I2CReadData(I2C_ID_C8_SHA204, rbuf, rLen);
   
   // I2CSha204Sleep();
}

/*********************************************************************
* REV_Read
// CMD1: Read device REV
// I/P: 07 30 00 00 00 03 5D           // expected return size
// O/P: 07 x x x x
*********************************************************************/
#define SHA_CMD_REV_SIZE    0x7        // expected return size
#define SHA_CMD_REV_DELAY   0X1   //4ms
Uint8 ShaReadRev (Uint8 *rbuf) {
   
   Uint8    xdata sbuf[] = { 3, 7, 0x30, 0, 0, 0, 3, 0x5D};   
   ShaCmdSendRecv(sbuf, sizeof(sbuf), \
      rbuf, SHA_CMD_REV_SIZE, SHA_CMD_REV_DELAY );    // Response SN length is 

   if(rbuf[0]==SHA_CMD_REV_SIZE) {
      return 1;
   }
   return 0;
}

/*********************************************************************
* SN_Read
// CMD2: Read device S/N            // expected return size
// I/P: 07 02 80 00 00 09 AD
// O/P: 23 x x x x
*********************************************************************/
#define SHA_CMD_SN_SIZE    0x23
#define SHA_CMD_SN_DELAY   0X1   //4ms
Uint8 ShaReadSn (Uint8 *rbuf) {
   
   Uint8    xdata sbuf[] = { 3, 7, 2, 0x80, 0, 0, 9, 0xAD};
   
   ShaCmdSendRecv(sbuf, sizeof(sbuf), rbuf, \
      SHA_CMD_SN_SIZE, SHA_CMD_SN_DELAY);    // Response SN length is 

   if(rbuf[0]==SHA_CMD_SN_SIZE) {
      return 1;
   }
   return 0;
}

/*********************************************************************
* SN_Read
// CMD2: Read device S/N
// I/P: 07 1B 00 00 00 24 CD 
// O/P: 23 x x x x
*********************************************************************/
#define SHA_CMD_RND_SIZE   0x23        // expected return size
#define SHA_CMD_RND_DELAY  6          //50ms
Uint8 ShaReadRnd (Uint8 *rbuf) {   
   Uint8    xdata sbuf[] = { 3, 7, 0x1B, 0, 0, 0, 0x24, 0xCD};
   
   ShaCmdSendRecv(sbuf, sizeof(sbuf), rbuf, \
      SHA_CMD_RND_SIZE, SHA_CMD_RND_DELAY);    // Response SN length is 
  
   if(rbuf[0]==SHA_CMD_RND_SIZE) {
      return 1;
   }
   return 0;
}

void step0_mixSnIntoRnd_b() {
	Uint8 i;
	for(i=0; i<sizeof(RND_b); i++) {
		RND_b[i] = RND_b[i] ^ DEV_sn[ i & 0x7];		
	}
}

/*********************************************************************
* SHA_Action1
// Result1 = SHA(Slot 0, RND_b)
// 1. MAC Command Sent: MAC (Slot 0 + SN)
// 27 08 00 00 00 01 23 5C C3 00 04 05 00 B4 89 C8 5E EE 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 5C A8 
// O/P Result1 = 32bytes
*********************************************************************/
#define SHA_CMD_MAC_SLOT0_IN_SIZE      0x28     // Full packet will be: Word_address(03) + Count + (Cmd_Data) + CRC1 + CRC2 = Count + 1
#define SHA_CMD_MAC_SLOT0_OUT_SIZE     0x23     // expected return size
#define SHA_CMD_MAC_DELAY              4        // Max. 35ms
Uint8 step1_getMACfromSlot0(Uint8 *obuf, Uint8 oLen,Uint8 *ibuf, Uint8 iLen) {   
   Uint8    xdata header[] = { 0x3, 0x27, 0x8, 0, 0, 0};
   // Copy Header
   memset(TxBuff, 0, sizeof(TxBuff));
   memcpy(TxBuff, header, sizeof(header));
   // 2. put S/N
   memcpy(&TxBuff[sizeof(header)], ibuf, iLen); 
   // 3. Gen CRC (skip) , FAKE CRC  
	shaCalCrc(&TxBuff[1], SHA_CMD_MAC_SLOT0_IN_SIZE-3, &TxBuff[SHA_CMD_MAC_SLOT0_IN_SIZE-2]);
	
   ShaCmdSendRecv(TxBuff, SHA_CMD_MAC_SLOT0_IN_SIZE, RxBuff, \
      SHA_CMD_MAC_SLOT0_OUT_SIZE, SHA_CMD_MAC_DELAY);    // Response SN length is 
  
	
   if(RxBuff[0]==SHA_CMD_MAC_SLOT0_OUT_SIZE) {
      memcpy(obuf, &RxBuff[1], oLen);		
      return 1;
   }
   return 0;
}

///*********************************************************************
//* SHA_Action1
//// Result1 = SHA(Slot 0, S/N)
//// 1. MAC Command Sent: MAC (Slot 0 + SN)
//// 27 08 00 00 00 01 23 5C C3 00 04 05 00 B4 89 C8 5E EE 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 5C A8 
//// O/P Result1 = 32bytes
//*********************************************************************/
//#define SHA_CMD_MAC_SLOT0_IN_SIZE      0x28     // Full packet will be: Word_address(03) + Count + (Cmd_Data) + CRC1 + CRC2 = Count + 1
//#define SHA_CMD_MAC_SLOT0_OUT_SIZE     0x23     // expected return size
//#define SHA_CMD_MAC_DELAY              4        // Max. 35ms
//Uint8 step1_getMACfromSlot0(Uint8 *buf, Uint8 pLen) {   
//   Uint8    xdata header[] = { 0x3, 0x27, 0x8, 0, 0, 0};
//   // Copy Header
//   memset(TxBuff, 0, sizeof(TxBuff));
//   memcpy(TxBuff, header, sizeof(header));
//   // 2. put S/N
//   memcpy(&TxBuff[sizeof(header)], DEV_sn, sizeof(DEV_sn)); 
//   // 3. Gen CRC (skip) , FAKE CRC  
//	shaCalCrc(&TxBuff[1], SHA_CMD_MAC_SLOT0_IN_SIZE-3, &TxBuff[SHA_CMD_MAC_SLOT0_IN_SIZE-2]);
//	
//   ShaCmdSendRecv(TxBuff, SHA_CMD_MAC_SLOT0_IN_SIZE, RxBuff, 
//      SHA_CMD_MAC_SLOT0_OUT_SIZE, SHA_CMD_MAC_DELAY);    // Response SN length is 
//  
//   if(RxBuff[0]==SHA_CMD_MAC_SLOT0_OUT_SIZE) {
//      memcpy(buf, &RxBuff[1], pLen);		
//      return 1;
//   }
//   return 0;
//}

/*********************************************************************
// 2. Write the MAC to Slot 7
// 27 12 82 38 00 48 AD C2 B1 99 B6 EB CD CF 4A C4 4C CD F5 FC E7 4D BF EA BD 24 58 BC 16 F9 D3 45 89 5D BD 73 E5 94 E5                             
//                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// O/P 04 00 03 40
*********************************************************************/
#define SHA_CMD_WRITE_SLOT7_IN_SIZE      0x28     // Full packet will be: Word_address(03) + Count + (Cmd_Data) + CRC1 + CRC2 = Count + 1
#define SHA_CMD_WRITE_SLOT7_OUT_SIZE     0x4     // expected return size
#define SHA_CMD_WRITE_SLOT7_DELAY  5              // Max. 42ms

Uint8 step2_writeToSlot7 (Uint8 *buf, Uint8 pLen) {   
   Uint8    xdata header[] = { 0x3, 0x27, 0x12, 0x82, 0x38, 0x00};
	Uint8 	xdata test1[] = "test 123";
	
   // Copy Header
   memset(TxBuff, 0, sizeof(TxBuff));
   memcpy(TxBuff, header, sizeof(header));
   // 2. put MAC_my
   memcpy(&TxBuff[sizeof(header)], buf, pLen); 
   // 3. Gen CRC (skip)  
	shaCalCrc(&TxBuff[1], SHA_CMD_WRITE_SLOT7_IN_SIZE - 3, &TxBuff[SHA_CMD_WRITE_SLOT7_IN_SIZE-2]);
	
//	Delay10ms(40);
//	UrPrint(TxBuff, TxBuff[0]);
	
   ShaCmdSendRecv(TxBuff, SHA_CMD_WRITE_SLOT7_IN_SIZE, RxBuff, \
      SHA_CMD_WRITE_SLOT7_OUT_SIZE, SHA_CMD_MAC_DELAY);    // Response SN length is 
	
   if(RxBuff[0]==SHA_CMD_WRITE_SLOT7_OUT_SIZE && RxBuff[1]==0x0) {
      // Expecting got "04 00 03 40" means success
//		Delay10ms(40);
//		UrPrint(test1, sizeof(test1));
      return 1;
   }
   return 0;
}

/*********************************************************************
// 3. MAC Command Sent: MAC (Slot 7 + RND_b)
// 27 08 00 07 00 DC EC 42 F6 42 FB 8E 6E AC 1E D3 52 F5 F9 30 40 DC 73 F9 91 88 91 8F 6A 16 2D 50 43 13 97 C3 02 FE 56
//                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// O/P ANS_a = 23 6E 56 00 60 ... CRC1 CRC2
*********************************************************************/
#define SHA_CMD_MAC_SLOT7_IN_SIZE      0x28     // Full packet will be: Word_address(03) + Count + (Cmd_Data) + CRC1 + CRC2 = Count + 1
#define SHA_CMD_MAC_SLOT7_OUT_SIZE     0x23     // expected return size
Uint8 step3_getMACfromSlot7(Uint8 *buf, Uint8 pLen) {   
   Uint8    xdata header[] = { 0x3, 0x27, 0x8, 0, 7, 0};
   // Copy Header
   memset(TxBuff, 0, sizeof(TxBuff));
   memcpy(TxBuff, header, sizeof(header));
   // 2. put RND_b
   memcpy(&TxBuff[sizeof(header)], RND_b, sizeof(RND_b)); 
   // 3. Gen CRC (skip)   
	shaCalCrc(&TxBuff[1], SHA_CMD_MAC_SLOT7_IN_SIZE - 3, &TxBuff[SHA_CMD_MAC_SLOT7_IN_SIZE-2]);
   
   ShaCmdSendRecv(TxBuff, SHA_CMD_MAC_SLOT7_IN_SIZE, RxBuff, \
      SHA_CMD_MAC_SLOT7_OUT_SIZE, SHA_CMD_MAC_DELAY);    // Response SN length is 
  
   if(RxBuff[0]==SHA_CMD_MAC_SLOT7_OUT_SIZE) {
      memcpy(buf, &RxBuff[1], pLen);
      return 1;
   }
   return 0;
}

/*********************************************************************
* Wakeup Command for ATSHA204A
*********************************************************************/
Uint8 I2CSha204Wakeup()
{        
   // write code is here
   
	I2CMBUF = 0;        // set data to 0	
   I2CMCR = START+RUN;    
   Delay100us(1);		// at least 60us
	
   I2CMCR = STOP +RUN;
   Delay1ms (5); 		// at least 2.5ms
   return PASS;
}

/*********************************************************************
* SLEEP Command for ATSHA204A
*********************************************************************/
Uint8 I2CSha204Sleep() {
   
   I2CMSA = (I2C_ID_C8_SHA204 <<1); 
	
   while ( 0 != (I2CMCR & BUS_BUSY) )  ;
    
   
    // write code is here
    I2CMBUF = SHA_WORD_ADDRESS_SLEEP;        // set data to 0
    I2CMCR = START+STOP+RUN;   
    delay();		// delay 10 x "_nop_();"
	// previous is delay(), replace by delay
    while ( 0 != (I2CMCR & BUSY) )  ;                                             

    if ( I2CMCR & (ERROR+ARB_LOST) )
    {
        if ( 0 == (I2CMCR & ARB_LOST) )
            I2CMCR = STOP;

        return FAIL;
    }

    return PASS;
}

/** \brief This function calculates CRC.
 *
 * \param[in] length number of bytes in buffer
 * \param[in] data pointer to data for which CRC should be calculated
 * \param[out] crc pointer to 16-bit CRC
 */
void shaCalCrc(Uint8 * buff, Uint8 length, Uint8 *crc) {
	Uint8 counter;
	Uint16 crc_register = 0;
	Uint16 polynom = 0x8005;
	Uint8 shift_register;
	Uint8 data_bit, crc_bit;

	for (counter = 0; counter < length; counter++) {
	  for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
		 data_bit = (buff[counter] & shift_register) ? 1 : 0;
		 crc_bit = crc_register >> 15;

		 // Shift CRC to the left by 1.
		 crc_register <<= 1;

		 if ((data_bit ^ crc_bit) != 0)
			crc_register ^= polynom;
	  }
	}
	crc[0] = (Uint8) (crc_register & 0x00FF);
	crc[1] = (Uint8) (crc_register >> 8);
}

#endif


#ifndef _I2C_C_
#define _I2C_C_

#include "define.h"

    // I2CMCR write
    #define RSTM        BIT7
    #define SLRST       BIT6
    #define ADDR        BIT5
    #define HS          BIT4
    #define ACK         BIT3
    #define STOP        BIT2
    #define START       BIT1
    #define RUN         BIT0

    // I2CMCR Read
    #define BUS_BUSY    BIT6
    #define IDLE        BIT5
    #define ARB_LOST    BIT4
    #define DATA_ACK    BIT3
    #define ADDR_ACK    BIT2
    #define ERROR       BIT1
    #define BUSY        BIT0

    // I2CSCR write
    #define RSTS        BIT7
    #define DA          BIT6
    #define RECFINCLR   BIT3
    #define SENDFINCLR  BIT2

    // I2CSCR Read
    #define DA          BIT6
    #define BUSACTIVE   BIT4
    #define RECFIN      BIT3
    #define SENDFIN     BIT2
    #define TREQ        BIT1
    #define RREQ        BIT0

    #define I2C_WRITE   0
    #define I2C_READ    1

    #define I2C_ID_50_EEPROM       0x50
    #define I2C_ID_10_MCU          0x10
    

    #define MAX_I2C_BUFFER      20

    #ifdef _MASTER_I2C_C_
        Uint8 data I2C_TxBuffer[MAX_I2C_BUFFER];
        Uint8 data I2C_RxBuffer[MAX_I2C_BUFFER];
        Uint8 data *Ptr_I2C_TxBuffer;
        Uint8 data *Ptr_I2C_RxBuffer;
        Uint8 data TxBufferIndex;
        Uint8 data RxBufferIndex;
        bit bI2C_RxReady;
        bit bI2C_TxReady;
    #else
        extern Uint8 data I2C_TxBuffer[MAX_I2C_BUFFER];
        extern Uint8 data I2C_RxBuffer[MAX_I2C_BUFFER];
        extern Uint8 data *Ptr_I2C_TxBuffer;
        extern Uint8 data *Ptr_I2C_RxBuffer;
        extern Uint8 data TxBufferIndex;
        extern Uint8 data RxBufferIndex;
        extern bit bI2C_RxReady;
        extern bit bI2C_TxReady;
    #endif

    #if defined(_MASTER_I2C_C_)
        
    #else
        // Master I2C
        extern    void  I2C_MasterInit(void);
        extern    Uint8 I2CWriteData(Uint8, Uint8*, Uint8);
        extern    Uint8 I2CReadData( Uint8, Uint8*, Uint8);
        extern    Uint8 WriteEEPROM( Uint8, Uint16,  Uint8*, Uint8);
        extern    Uint8 ReadEEPROM(  Uint8, Uint16,  Uint8*, Uint8);

        // Salve I2C
        extern    void    I2C_SlaveInit(void);
    #endif
	
#endif
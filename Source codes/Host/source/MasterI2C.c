#define _MASTER_I2C_C_

#include "..\include\System_config.h"


/*********************************************************************
* delay
*********************************************************************/
void delay(void)
{
    Uint8 i;

    for ( i = 0; i < 10; i++ )
        _nop_();
}

/*********************************************************************
* I2C_MasterInit
*********************************************************************/
void I2C_MasterInit(void)
{
    IOSEL |= BIT1;					// I2C0_SEL;
    I2CMCR = RSTM;          		// reset master module
	 I2CMTP = 0x05;          		// SHA204(7), set speed : 2 x (1 + 6) x 10 x 78.128ns
											// I2CMTP=6 (80Khz)
											// I2CMTP=5 (101Khz)
											// I2CMTP=1 (316Khz)
    I2CMCR = SLRST + RUN;   	// reset bus
    while ( 0 != (I2CMCR & BUSY) )  ;
    //P3_6 = 0;
}


/*********************************************************************
* I2C_Reset
*********************************************************************/
void I2C_Slave_Reset(void)
{
    // I2CMCR = RSTM;          		// reset master module
    I2CMCR = SLRST + RUN;   		// reset bus
    while ( 0 != (I2CMCR & BUSY) )  ;
    //P3_6 = 0;
}

/*********************************************************************
* bus_busy
*********************************************************************/
void bus_busy(void)
{
	 while ( 0 != (I2CMCR & BUS_BUSY) );
}

/*********************************************************************
* eep_busy
*********************************************************************/
void eep_busy(void)
{
	 while ( 0 == (I2CMCR & BUSY) )  ;
    while ( 1 == (I2CMCR & BUSY) )  ;
}

/*********************************************************************
* write_setup
*********************************************************************/
void write_setup(Uint8 slave_addr)
{
    I2CMSA = (slave_addr<<1);
}

/*********************************************************************
* I2cWrite
*********************************************************************/
Uint8 I2cWrite(Uint8 i2c_data, Uint8 mcr)
{
    I2CMBUF = i2c_data;
    I2CMCR = mcr;
    delay();
   // Oringinal code 
	while ( 0 != (I2CMCR & BUSY) )  ;                                             
	// change to 
	//eep_busy();
	
    if ( I2CMCR & (ERROR+ARB_LOST) )
    {
        if ( 0 == (I2CMCR & ARB_LOST) )
            I2CMCR = STOP;

        return FAIL;
    }

    return PASS;
}

/*********************************************************************
* read_setup
*********************************************************************/
void read_setup(Uint8 slave_addr)
{
    I2CMSA = (slave_addr<<1) | BIT0;
}

/*********************************************************************
* I2cRead
*********************************************************************/
Uint8 I2cRead(Uint8 mcr)
{
    I2CMCR = mcr;
    delay();
    while ( 0 != (I2CMCR & BUSY) )  ;

    if ( I2CMCR & (ERROR+ARB_LOST) )
    {
        if ( 0 == (I2CMCR & ARB_LOST) )
            I2CMCR = STOP;
        return FAIL;
    }

    return PASS;
}

/*********************************************************************
* I2CWriteData
*********************************************************************/
Uint8 I2CWriteData(Uint8 id, Uint8 *buf, Uint8 length)
{
    Uint8 mcr;
    bit bSTART;

    if ( length == 0 )
        return FAIL;

    write_setup( id );
    bus_busy();
    bSTART = 1;

    while(length)
    {
        if ( length == 1 )
        {
            if ( bSTART )
            {
                bSTART = 0;
                mcr = START+STOP+RUN;
            }
            else
            {
                mcr = STOP+RUN;
            }
        }
        else
        {
            if ( bSTART )
            {
                bSTART = 0;
                mcr = START+RUN;
            }
            else
            {
                mcr = RUN;
            }
        }

        if ( PASS != I2cWrite(*buf, mcr) )
            return FAIL;
        buf++; length--;
    }

    return PASS;
}

/*********************************************************************
* I2CReadData
*********************************************************************/
Uint8 I2CReadData(Uint8 id, Uint8* buf, Uint8 length)
{
    Uint8 mcr;
    bit bSTART;

    if ( length == 0 )
        return FAIL;

    read_setup(id);
    bus_busy();
    bSTART = 1;

    while(length)
    {
        if ( length == 1 )
        {
            if ( bSTART )
            {
                bSTART = 0;
                mcr = START+STOP+RUN;
            }
            else
            {
                mcr = STOP+RUN;
            }
        }
        else
        {
            if ( bSTART )
            {
                bSTART = 0;
                mcr = START+ACK+RUN;
            }
            else
            {
                mcr = ACK+RUN;
            }
        }
        if ( PASS != I2cRead(mcr) )
            return FAIL;
        *buf++ = I2CMBUF; length--;
    }
    return PASS;
}


/*********************************************************************
* WriteEEPROM
*
* EEPROM Page Write
*      START ID a ADDR a DATA0 a DATA1 a .... DATA127 a STOP
*********************************************************************/
Uint8 WriteEEPROM(Uint8 id, Uint16 addr, Uint8 *buf, Uint8 length)
{
    Uint8 mcr;
	 Uint8 addr_l, addr_h;
	 // Uint8 dat = *buf;
	
    if ( length > 128 )
        return FAIL;
    if ( length == 0 )
        return FAIL;

	 addr &= (EEPROM_SIZE_BYTE -1);		// Size is 2KB
	 
	 addr_l = addr;
	 addr_h = addr >> 8;
	 
    write_setup( id | addr_h );	 			// Control Code + Block Select bits
	 // I2CMSA = (id << 1);
	 I2CMCR = START | RUN;
	 while ( 1 == (I2CMCR & ADDR_ACK) )  ;
    //bus_busy();
	 

//	 There is no High address byte in 24AA16
//	 I2CMBUF = addr_h;
//	 //I2CMCR = START | RUN;
//    I2CMCR = RUN;
//    //delay();
//	 eep_busy();
	 
	 I2CMBUF = addr_l;
    I2CMCR = RUN;
    //delay();
    eep_busy();
	 
    while(length)
    {
        if ( length == 1 )
            mcr = STOP+RUN;
        else
            mcr = RUN;

        if ( PASS != I2cWrite(*buf, mcr) )
            return FAIL;
        buf++; length--;
    }

    return PASS;
}


/*********************************************************************
* ReadEEPROM
*
* EEPROM Sequential Read
*
*   START ID a ADDR a START ADDRESS1 a DATA a DATA
*      a ... DATA n STOP
*********************************************************************/
Uint8 ReadEEPROM(Uint8 id, Uint16 addr, Uint8* buf, Uint8 length)
{
    Uint8 mcr;
	 Uint8 addr_l, addr_h;
    bit bSTART;

    if ( length == 0 )
        return FAIL;

	 addr &= (EEPROM_SIZE_BYTE -1);		// Size is 2KB
	 addr_l = addr;
	 addr_h = addr >> 8;
	 
	 
    write_setup( id | addr_h );
	 I2CMCR = START | RUN;
	 while ( 1 == (I2CMCR & ADDR_ACK) )  ;
    //bus_busy();
	 
	 if ( PASS != I2cWrite(addr_l, RUN) )
        return FAIL;
    

    read_setup(id| addr_h);
    bSTART = 1;

    while(length)
    {
        if ( length == 1 )
        {
            if ( bSTART )
            {
                bSTART = 0;
                mcr = START+STOP+RUN;
            }
            else
            {
                mcr = STOP+RUN;
            }
        }
        else
        {
            if ( bSTART )
            {
                bSTART = 0;
                mcr = START+ACK+RUN;
            }
            else
            {
                mcr = ACK+RUN;
            }
        }
        if ( PASS != I2cRead(mcr) )
            return FAIL;
        *buf++ = I2CMBUF; length--;
    }
    return PASS;
}

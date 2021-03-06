/*********************************************************************
**	File:		Uti.c
**	Updated:	2016-05-27
**	Description:
**	This file is the delay function.
**
**	Copyright (C) 2016 AMICCOM Corp.
**
*********************************************************************/
#define	_UTI_C_
#include "..\include\SYSTEM_CONFIG.h"

#ifdef Xtal_12p8MHz

/**********************************************************
* Delay10us	@12.8MHz
**********************************************************/
void Delay10us(Uint8 n)
{
	Uint8 i;

			while(n--)
			{
					for(i=0; i<7; i++);
			}

}

/**********************************************************
* Delay100us @12.8MHz
**********************************************************/
void Delay100us(Uint8 n)
{
	Uint8 i;

	while(n--)
	{
		for(i=0; i<103;	i++);
	}
}

/**********************************************************
* Delay1ms @12.8MHz
**********************************************************/
void Delay1ms(Uint8	n)
{
	Uint8 i, j;

	while(n--)
	{
		for(i=0; i<10; i++)
			for(j=0; j<210;	j++);
	}
}

/**********************************************************
* Delay10ms	@12.8MHz
**********************************************************/
void Delay10ms(Uint8 n)
{
	Uint8 i, j;

	while(n--)
	{
		for(i=0; i<100;	i++)
			for(j=0; j<210;	j++);
	}
}

#endif


#ifdef Xtal_16MHz

/**********************************************************
* Delay10us	@16MHz
**********************************************************/
void Delay10us(Uint8 n)
{
	Uint8 i;
	
	while(n--)
	{
		for(i=0; i<10; i++);
	}
}

/**********************************************************
* Delay100us @16MHz
**********************************************************/
void Delay100us(Uint8 n)
{
	Uint8 i;

	while(n--)
	{
		for(i=0; i<130;	i++);
	}
}

/**********************************************************
* Delay1ms @16MHz
**********************************************************/
void Delay1ms(Uint8	n)
{
	Uint8 i, j;

	while(n--)
	{
		for(i=0; i<11; i++)
			for(j=0; j<236;	j++);
	}
}

/**********************************************************
* Delay10ms	@16MHz
**********************************************************/
void Delay10ms(Uint8 n)
{
	Uint8 i, j;

	while(n--)
	{
		for(i=0; i<110;	i++)
			for(j=0; j<236;	j++);
	}
}

#endif


#ifdef Xtal_19p2MHz

/**********************************************************
* Delay10us	@19.2MHz
**********************************************************/
void Delay10us(Uint8 n)
{
	Uint8 i;
	
	while(n--)
	{
		for(i=0; i<14; i++);
	}
}

/**********************************************************
* Delay100us @19.2MHz
**********************************************************/
void Delay100us(Uint8 n)
{
	Uint8 i;

	while(n--)
	{
		for(i=0; i<157;	i++);
	}
}

/**********************************************************
* Delay1ms @19.2MHz
**********************************************************/
void Delay1ms(Uint8	n)
{
	Uint8 i, j;

	while(n--)
	{
		for(i=0; i<13; i++)
			for(j=0; j<242;	j++);
	}
}

/**********************************************************
* Delay10ms	@19.2MHz
**********************************************************/
void Delay10ms(Uint8 n)
{
	Uint8 i, j;

	while(n--)
	{
		for(i=0; i<130;	i++)
			for(j=0; j<242;	j++);
	}
}

#endif

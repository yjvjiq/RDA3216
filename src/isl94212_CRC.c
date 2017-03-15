#include "isl94212_CRC.h"

Bit8Field CRC_Calc_0, CRC_Calc_1;

/*******************************************************************
 *******************************************************************/
void CalcCRC4(U8 *pData, U8 dataLen)
{
	unsigned char i, j, k;
	unsigned char data0;
	unsigned char result;

  CRC_Calc_0.Byte = 0;
	
	for (i=0; i<dataLen; i++)
	{
		data0 = pData[i];		
		(i == (dataLen - 1)) ? (k=3) : (k=8);
		for (j=0; j<k; j++)
		{
			CRCcarry = ((data0 & 0x80) > 0);
			data0 <<= 1;
							
			CRCff0 = CRCcarry ^ CRCbit3;
			CRCff1 = CRCbit0 ^ CRCbit3;
			CRCff2 = CRCbit1;
			CRCff3 = CRCbit2;
            CRC_Calc_0.Byte >>= 4;
		}
	}
		
	result = CRC_Calc_0.Byte & 0x0F;
	
	pData[3] &= 0xE0;
	pData[3] |= (result<<1);		/* ?CRC????pData[dataLen - 1]????? */
}

/*******************************************************************
 *  ????:CheckCRC4
 *  ????:??CRC4
 *  ????:pData - ??CRC???????
 * 		       dataLen - ?????
 *  ????:pData - ??CRC??????? 
 *  ???:1 - ??
 *					0 - ??
 *  
 *  ??:????????????????0,CRC4???????????????
 *        ????????????????????????????????,????
 *******************************************************************/
Bool CheckCRC4(U8 *pData, U8 dataLen)
{
	unsigned char i, j, k;
	unsigned char data0;
	unsigned char result;

	CRC_Calc_0.Byte = 0;
	
	for (i=0; i<dataLen; i++)
	{
		data0 = pData[i];		
		(i == (dataLen - 1)) ? (k=4) : (k=8);
		for (j=0; j<k; j++)
		{
			CRCcarry = ((data0 & 0x80) > 0);
			data0 <<= 1;
							
			CRCff0 = CRCcarry ^ CRCbit3;
			CRCff1 = CRCbit0 ^ CRCbit3;
			CRCff2 = CRCbit1;
			CRCff3 = CRCbit2;

			CRC_Calc_0.Byte >>= 4;
		}
	}
		
	result = CRC_Calc_0.Byte & 0x0F;
	return ((result & pData[dataLen - 1]) == result) ? TRUE : FALSE;
}

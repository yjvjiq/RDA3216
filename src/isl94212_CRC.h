#ifndef ISL94212_CRC_H_
#define ISL94212_CRC_H_

#include "main.h"

//#pragma MESSAGE DISABLE C1106 /* WARNING C1106: Non-standard bitfield type */ 

typedef union
{
	U8 Byte;
    struct  
    {
        U8  bit0         :1;
        U8  bit1         :1;
        U8  bit2         :1;
        U8  bit3         :1;
        U8  bit4         :1;
        U8  bit5         :1;
        U8  bit6         :1;
        U8  bit7         :1;
    } Bits;
} Bit8Field;

extern  Bit8Field CRC_Calc_0, CRC_Calc_1;

#define CRCff3 		CRC_Calc_0.Bits.bit7
#define CRCff2 		CRC_Calc_0.Bits.bit6
#define CRCff1 		CRC_Calc_0.Bits.bit5
#define CRCff0 		CRC_Calc_0.Bits.bit4
#define CRCbit3 	CRC_Calc_0.Bits.bit3
#define CRCbit2 	CRC_Calc_0.Bits.bit2
#define CRCbit1 	CRC_Calc_0.Bits.bit1
#define CRCbit0 	CRC_Calc_0.Bits.bit0
#define CRCcarry	CRC_Calc_1.Bits.bit0
  
void CalcCRC4(U8 *pData, U8 dataLen);
Bool CheckCRC4(U8 *pData, U8 dataLen);


#endif /* ISL94212_CRC_H_ */

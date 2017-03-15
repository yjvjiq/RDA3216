#ifndef _MAIN_H
#define _MAIN_H
#include "string.h"
#include "stdio.h"
#include "config.h"
													
typedef unsigned char      Bool;

typedef unsigned char       U8 ;
typedef unsigned short      U16;
typedef signed short S16;
typedef unsigned long       U32;
typedef unsigned long long  U64;
typedef char                S8 ;
typedef short               S16;
typedef long                S32;
typedef long long           S64;

typedef volatile unsigned char		VU8 ;
typedef volatile unsigned short		VU16;
typedef volatile unsigned long		VU32;
typedef volatile unsigned long long	VU64;
typedef volatile char				VS8 ;
typedef volatile short				VS16;
typedef volatile long				VS32;
typedef volatile long long			VS64;

#define u8  U8
#define u16 U16
#define u32 U32
#define u64 U64
#define s8  S8
#define s16 S16
#define s32 S32
#define s64 S64

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef bool

#define bool U8
#endif

#define DisableInterrupts  __disable_irq()
#define EnableInterrupts   __enable_irq()

#define Set_PosFlag(flag,POS)    (flag |= (1<<POS))
#define Get_PosFlag(flag,POS)    (flag &  (1<<POS))
#define Clear_PosFlag(flag,POS)  (flag &= ~(1<<POS))

#define DATA_SAMPLING_AND_PROCESS_COST_TIME 80
#define MAX_LEN 255


typedef struct
{
	U8 buffer[MAX_LEN+1];
	U16 len;
}data_frame;

typedef struct
{
	U8 buffer[MAX_LEN+1];
	U16 put;
	U16 get;
	Bool full; 
	Bool empty;
}ring_frame;

typedef void(*pt2Func)(void);		  /* Pointer to Functions, void argument */
typedef void(*pt2FuncU8)(U8);	  /* Pointer to Functions, UINT8 argument */

extern U8 g_SoftVersion[4];
extern U8 g_HardWareVersion[4];
void DataSamplingAndProcess(void);
void SetNextFlashDataBlockSaveTimer(void);
#endif


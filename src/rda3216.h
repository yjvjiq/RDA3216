#ifndef RDA3216_H_
#define RDA3216_H_

#include "types.h"

#define WRTCMD_LEN  4
#define RDCMD_LEN  4

//#define SPECIAL_RDCMD 0xF000000C
#define SPECIAL_RDCMD 0x0C0000F0


#define REGREAD_ADDR 0x18
#define REGVOL_BASE 0x00
#define REGAUX_BASE 0x07

#define CHAIN_DEV_NUM 3 
#define CMD_DIFF_MODE 0x8542 // diff mode


#define MAX_SERIALREGNO 16*CHAIN_DEV_NUM


typedef struct {
	U8 regaddr_H	:4;
	U8 devaddr		:4;
	
	U8 data_H		:7;
	U8 regaddr_L	:1;
	
	U8 data_M;
	
	U8 rsvd			:1;
	U8 crc			:4;
	U8 seek_devaddr	:1;
	U8 data_L		:2;
	
}WriteCommand;


typedef union {
	WriteCommand WrtCmd;
	U8 data[4];
}WriteCommand_buf;



typedef struct {
	U8 devaddr	:4;
	U8 regaddr1	:4;
	
	U8 regaddr2	:1;
	U8 data1	:7;
	
	U8 data2;
	
	U8 data3	:2;
	U8 rsvd1	:1;
	U8 crc		:4;
	U8 rsvd2	:1;
}ReadCommand;


typedef union {
	ReadCommand RdCmd;
	U8 data[4];
}ReadCommand_buf;

#define REGCONFIG_ADDR 0x15
#define REGSERIAL_ADDR   0
#define REGSAMPLE_TIME_ADDR 0x1c


typedef enum{
	SevenVol_NineAux	= 0,
	SevenVol_Aux0		= 1,
	Sevenvol			= 2,
	Aux0				= 3,	
}Config_Mode;

typedef enum{
	lowpwr_close,
	soft_close,
}Close_Type;

/*  RDA3216 Error Code */
typedef enum {
    RDA_ErrCode_None = 0,               /*  No Error */
    RDA_ErrCode_CRC = 1,                /*  CRC4 Check Error */
    RDA_ErrCode_ACK = 2,                /*  ACK Error */
    RDA_ErrCode_NAK = 3,                /*  NAK Error */
    RDA_ErrCode_NoResponse = 255,       /*  No Response Error*/
} RDA_ErrCode;

typedef enum{
	READ_ERR_NONE,
	READ_ERR_REG15,
	READ_ERR_REG1C,
	READ_ERR_REG
}CheckErr;

typedef struct{
	U8 pd_form			:1;
	U8 rsvd_11_9		:2;
	U8 rd_conv_mode		:2;
	U8 sel_conv_mode	:2;
	U8 rsvd_16			:1;
	
	U8 rsvd_0			:1;
	U8 inc_dev_addr		:1;
	U8 lock_dev_addr	:1;
	U8 chip_num			:4;
	U8 soft_reset		:1;
}s_RDA_Config_Cmd;

typedef union{
	s_RDA_Config_Cmd cmd;
	U8 data[2];
}u_RDA_Config_Cmd;

void SendWriteCommand(U8 devaddr, U8 regaddr, U32 data, U8 seekall_flg);
void SendReadCommand(U8 devaddr, U8 regaddr, U8 seek_all_flg);
void ReadReg(U8 devaddr,U8 regaddr,U32* data,U8 seek_all_flg);
void RDA_DeviceNumReset(U8 chip_num);
void RDA_DeviceModReset(U8 mode);
void RDA3216_Init(void);


void RDA_DeviceConfig(U8 chip_num,U8 mode);
void RDA_Test(void);

#endif

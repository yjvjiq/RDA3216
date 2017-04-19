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

#define REGCONFIG_ADDR			0x15
#define REGSERIAL_ADDR			0
#define REGSAMPLE_TIME_ADDR		0x1c
#define REG_CONVERT_RESULT_ADDR	0x00

#define CMD_DIFF_MODE 0x8542 // diff mode
#define CHAIN_DEV_NUM 3								/* AFE芯片数量 */
#define CELL_NUMBER_IN_AFE              12          /* 每片AFE芯片上单体电池通道数  */
#define TEMP_NUMBER_IN_AFE              2           /* 每片AFE芯片上连接温度通道数  */
#define CELL_VOLTAGE_CHANNEL            (CELL_NUMBER_IN_AFE * NUMBER_OF_AFE_DEVICES)        /* 单体电压通道数 */
#define CELL_TEMPERATURE_CHANNEL        2           /* (TEMP_NUMBER_IN_AFE * NUMBER_OF_AFE_DEVICES)    单体温度通道数 */


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

#define REGCONFIG_ADDR			0x15
#define REGSERIAL_ADDR			0
#define REGSAMPLE_TIME_ADDR		0x1c
#define REG_CONVERT_RESULT_ADDR	0x00

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

/* ==========================RDA config data struct, regAddr=0x15=========================== */
typedef struct{
	U8 rsvd_16			:1;
	U8 data_rsvd		:7;
	
	U8 pd_form			:1;
	U8 rsvd_11_9		:3;
	U8 rd_conv_mode		:2;
	U8 sel_conv_mode	:2;
	
	U8 rsvd_0			:1;
	U8 inc_dev_addr		:1;
	U8 lock_dev_addr	:1;
	U8 chip_num			:4;
	U8 soft_reset		:1;
}s_RDA_Config_Cmd;

typedef union{
	s_RDA_Config_Cmd cmd;
	U8 data[3];
}u_RDA_Config_Cmd;

/* =============RDA write regAddr = 0x18, data struct =======================*/
typedef struct{
	U8 rsvd_16			:1;
	U8 data_rsvd		:7;
	
	U8 rsvd1;
	
	U8 rsvd_2_02		:3;
	U8 read_type		:5;
}s_regAddr_18;

typedef union{
	s_regAddr_18 cmd;
	U8 data[3];
}u_regAddr_18;

/* =============RDA write regAddr = 0x1a, data struct =======================*/
typedef struct{
	U8 rsvd_16			:1;
	U8 data_rsvd		:7;
	
	U8 rsvd_1_0_4		:5;
	U8 alert_sel		:2;
	U8 soft_reset_en	:1;
	
	U8 rsvd_2;
}s_regAddr_1a;

typedef union{
	s_regAddr_1a cmd;
	U8 data[3];
}u_regAddr_1a;

/* =============RDA write regAddr = 0x1b, data struct =======================*/
typedef struct{
	U8 rsvd_0_0			:1; // bit0
	U8 rsvd_0_1_7		:7; //bit1~bit7
	
	U8 rsvd_1_0_3		:4;
	U8 pdn_timer_sel	:4;
	
	U8 rsvd_2;
}s_regAddr_1b;

typedef union{
	s_regAddr_1b cmd;
	U8 data[3];
}u_regAddr_1b;

/* =============RDA write regAddr = 0x1c, data struct =======================*/
typedef struct{
	U8 rsvd_16			:1;
	U8 data_rsvd		:7;
	
	U8 rsvd_1;
	
	U8 rsvd_1_0			:1;
	U8 diff_mode_sel	:1; // diff_mode_sel=1 means enable the diff mode.
	U8 sample_time_sel	:2; // 00=400ns, 01=800ns, 10=1.2us, 11=1.6us
	U8 rsvd_1_47		:4;
}s_regAddr_1c;

typedef union{
	s_regAddr_1c cmd;
	U8 data[3];
}u_regAddr_1c;

/* =================================data struct=================================================== */
typedef struct {
    U8 devAddress;							/*  device address, master is 0 */
    U16 cellVoltages[CELL_NUMBER_IN_AFE];	/*  cell voltage for each channel */
    U16 batVoltage;							/*  battery voltage */
    U16 extTemps[TEMP_NUMBER_IN_AFE];		/*  external temp No.1 - No.4 */
} FAE_Device;

/*  Battery Pack */
typedef struct {
    FAE_Device devices[CHAIN_DEV_NUM];    
} FAE_Pack;


void SendWriteCommand(U8 devaddr, U8 regaddr, U32 data, U8 seekall_flg);
void SendReadCommand(U8 devaddr, U8 regaddr, U8 seek_all_flg);
void ReadReg(U8 devaddr,U8 regaddr,U32* data,U8 seek_all_flg);
void RDA_DeviceNumReset(U8 chip_num);
void RDA_DeviceModReset(U8 mode);
void RDA3216_Init(void);


void RDA_DeviceConfig(U8 chip_num,U8 mode);
void RDA_Test(void);

#endif

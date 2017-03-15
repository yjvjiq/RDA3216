#ifndef _CAN_SERVICE_H
#define _CAN_SERVICE_H

#include "drivers.h"
#include "w25q64.h"
#include "public.h"
#include "timer.h"

#ifdef CAN_COMMUNICATION
#define CAM_MSG_MAX_NUM  (W25Q64_DATA_BLOCKSIZE/8)
#define BANK_SIZE 16384
#define CAN_SENDBUFF_CAP 30

//BOARDCAST MSG1
#define PackVoltageValue_POS       0
#define NowCurrent_POS             (PackVoltageValue_POS+2)
#define NowSOC_POS                 (NowCurrent_POS+2)
#define PackMeanTemprature_POS     (NowSOC_POS+1)

//BOARDCAST MSG2
#define CellMaxVoltage_POS         0
#define CellMaxVoltageNumber_POS   (CellMaxVoltage_POS+2)
#define CellMinVoltage_POS         (CellMaxVoltageNumber_POS+1)
#define CellMinVoltageNumber_POS   (CellMinVoltage_POS+2)
#define CellMeanVoltageValue_POS   (CellMinVoltageNumber_POS+1)

//BOARDCAST MSG3
#define Cell_T_1_POS		0
#define Cell_T_2_POS		(Cell_T_1_POS+1)
#define AlarmFlag_POS		(Cell_T_2_POS+1)

//CANACK MSG1
#define BMSID_POS                  0

//CANACK MSG2
#define MaxPermitsVoltage_POS      0
#define MaxPermitsCurrent_POS      (MaxPermitsVoltage_POS+2)

//STATIC DATA BMSID 12 BYTE
#define BMSID_HIGH_POS 6
#define BMSID_LOW_POS  0

//COMADDR STAUTS
#define BMSID_LOW_MASK          0x01
#define BMSID_HIGH_MASK         0x02
#define BMSID_FULL_MASK        0x03
#define BMSID_EMPTY_MASK        0x00

#define BMSID_LOW_SHIFT         0
#define BMSID_HIGH_SHIFT        1

//typedef union
//{
//    U32 varu32;
//    U8  varu8[4];
//}u_varform_32u8;

//typedef union
//{
//    U16 baru16;
//    U8  varu8[2];
//}u_varform_16u8;

typedef enum
{
    STOP,
    RUNNING,
    FINISH
}send_state;

typedef struct
{
    U8 *buffer;
    U8 remain_msg;
    U16 send_num;
    send_state state;
}s_canmsg;

typedef struct{
    U32 remain_num;
    send_state state;
}s_popstat;

typedef struct{
    tTimerID timerID;
    U8 src_addr;
    U8 dest_addr;
    U8 code;
    U16 cycle;
    U8 times;
    U8 state;
}s_boardcast;

typedef struct{
U32  UinxTime;
U16  PackVoltageValue;
U16  NowCurrent;
U8   NowSOC;
U8   PackMeanTemprature;
 
U16  CellMaxVoltage;
U8   CellMaxVoltageNumber;
U16  CellMinVoltage;
U8   CellMinVoltageNumber;
U16  CellMeanVoltageValue;
U8   Cell_T_1;
U8   Cell_T_2;
 
U8  AlarmFlag[5];
U16  Crc;
U8  Rev[5];
}s_log_data;

typedef union{
    U16 cell_voltages[CELL_VOLTAGE_CHANNEL];
    U8  data[CELL_VOLTAGE_CHANNEL*2];
}u_cellvoltages;

typedef struct{
    MSCAN_RegisterFrameType buff[CAN_SENDBUFF_CAP+1];
    U8 head;
    U8 tail;
    U8 full;
    U8 empty;
}s_CanSendQueue;

typedef struct{
    U8 Flag;
    U8 ComAddr;
}s_ComAddrStatus;

U32 GetU32From4Bytes(U8 *buff);
void SetU8ToBuffer(U8 data, U8 *buff);
void SetU16ToBuffer(U16 data, U8 *buff);

U8 WriteCanQueue(s_CanSendQueue *queue, MSCAN_RegisterFrameType *canframe);
U8 ReadCanQueue(s_CanSendQueue *queue, MSCAN_RegisterFrameType *canframe);

void InitCanSendBuff(LDD_CAN_TFrame *srcbuff, LDD_CAN_TFrame *sendbuff);
void InitCanMsg(s_canmsg *canmsg, U8 *buff_ptr);
//void InitPopState(LDD_CAN_TFrame *canframe ,s_popstat *popstate);
//void InitFlashDataFrame(flashdata_frame *flashdataframe);

void StopBlockSend(s_canmsg *canmsg);
void StopPopOut(s_popstat *popstate);
void StopPopAndTransimit(void);

void SendStopMSG(LDD_CAN_TFrame *srcbuff,LDD_CAN_TFrame *sendbuff);

U8 FlashDataBlockSend(s_canmsg *canmsg, LDD_CAN_TFrame *sendbuff);
U8 FlashPopOut(s_popstat *popstate , flashdata_frame *dataframe, U8 *buff_ptr);
void Init_AckMSG(s_boardcast *boardmsg, LDD_CAN_TFrame *canframe);
void AckMSG(FlashStatic_Data_Union *staticdata, Battery_Pack_Info *packinfo, s_boardcast *boardmsg);
void Init_BoardCastMSG(s_boardcast *boardmsg, LDD_CAN_TFrame *canframe);
void BoardCastMSG(Pack_Data_Struct *logdata, s_boardcast *boardmsg);
void FlashPopAndSendProc(void);
void CanCmdParsingProc(LDD_CAN_TFrame *canframe);

extern s_boardcast g_boardmsg01;
extern s_boardcast g_boardmsg02;
extern s_boardcast g_boardmsg03;

//extern s_boardcast g_canackmsg01;
//extern s_boardcast g_canackmsg02;
//extern s_boardcast g_canackmsg03;
//extern s_boardcast g_canackmsg04;
//extern s_boardcast g_canackmsg05;
//extern s_boardcast g_canackmsg06;
//extern s_boardcast g_canackmsg07;
//extern s_boardcast g_canackmsg08;
extern s_boardcast g_canackmsg[9];

extern LDD_CAN_TFrame g_can_sendbuff;
extern s_CanSendQueue g_CanSendQueue;
extern U8 g_flashdatablock[W25Q64_DATA_BLOCKSIZE];
extern s_canmsg g_canmsg;
extern s_popstat g_popstate;

extern LDD_CAN_TFrame g_can_recframe;
extern U8 g_can_recdata[8];
#endif 

#endif

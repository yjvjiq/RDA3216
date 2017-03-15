#ifndef __XMODEM_H__
#define __XMODEM_H__
#include "main.h"
#include "derivative.h"

#define MAX_SINGLE_FRAME_LENGTH        8

#define XMD_SOH     0x01 
#define XMD_EOT     0x04 
#define XMD_ACK     0x06 
#define XMD_NAK     0x15 
#define XMD_CAN     0x18 
#define XMD_ESC     0x1b 
#define XMD_REQ     0x52  //'R'
#define XMD_FLW     0x16  

#define BIN_HEADER_16     16
#define PKTLEN_128        128 
#define PKTBASELEN        5 
#define XMODEM_BUFLEN     256
#define XMD_DATA_CAN_LEN  8

#define XMD_CRC_ERR         (1<<0)
#define XMD_FILE_CRC_ERR    (1<<1)
#define XMD_FILE_END        (1<<2)
#define XMD_IDX_ERR         (1<<3)

#define XMD_PUTC_ERR_CNT    (60*10) //1s put 'c',total 10min timeout(max 65535)
#define XMD_MAX_ERR_CNT     10
#define XMD_PACKET_TIMEOUT  200

#define CAN_RX_BUF_LEN    136
#define XMODEM_HEAD_SIZE  (5)
#define XMODEM_BODY_SIZE  (128)

#define XMD_TYPE_POS    0
#define XMD_IDX_POS     1
#define XMD_IDX_INV_POS 2
#define XMD_PACKET_POS  3
#define XMD_CRC_H_POS   131
#define XMD_CRC_L_POS   132
#define FLASH_KEY       0x1234

#define XMD_CMD_MASK_POS       0
#define XMD_CMD_FUNC_POS       1
#define XMD_CMD_TYPE_POS       2
#define XMD_CMD_SUBTYPE0_POS   3
#define XMD_CMD_SUBTYPE1_POS   4
#define XMD_CMD_KEY1_POS       5
#define XMD_CMD_KEY2_POS       6
#define XMD_CMD_KEY3_POS       7

#define XMD_FILE_DEV_TYPE_POS   8
#define XMD_FILE_CODE_TYPE_POS  9

#define BOOT_VERSION_ADDR 0x00001000
#define BOOT_VERSION_LEN  3

#define __SWP16(data)   ((((uint16_t)(data) & 0xff00) >> 8) | (((uint16_t)(data) & 0x00ff) << 8))  
#define __SWP32(data)   ((((uint32_t)(data) & 0xff000000) >> 24) | (((uint32_t)(data) & 0x00ff0000) >> 8) | (((uint32_t)(data) & 0x0000ff00) << 8) | (((uint32_t)(data) & 0x000000ff) << 24)) 

enum{XMODEM_REQ = 1, XMODEM_CAN_REQ,BOOT_CURRENT_VALID_SECTOR,BOOT_ANOTHER_VALID_SECTOR,XMODEM_READ_VERSION};
enum{CMD_BMS_Master=1,CMD_BMS_Slave,CMD_EMS_Master,CMD_EMS_Slave};

typedef enum{
  XMODEM_ICU = 0x10,
  XMODEM_BMS = 0x20,
  XMODEM_VIC = 0x30,
}eBootDeviceType;

typedef enum{
  BMS_Master = 0x10,
  BMS_Slave = 0x20
}eBootCodeType;

typedef enum{
  FLASH_SECTOR_1 = 0,
  FLASH_SECTOR_2,
  FLASH_SECTOR_MAX
}eBootFlashSectorType;

typedef enum{
  BOOT_IDLE = 0,
  BOOT_START,
  BOOT_BUSY,
  BOOT_DONE,
  BOOT_CAN
}eBootSTATE;

typedef enum{
  VIC_XMD_REQ = 0x804,
  VIC_XMD_RSP = 0x805
}eBootCanID;

typedef struct{
  uint8_t type;
  uint8_t idx;
  uint8_t idxInv;
  uint8_t payload[XMODEM_BODY_SIZE];
  uint8_t crc1;
  uint8_t crc2;
}sCANXmodemMsgType;

typedef struct{
  eBootSTATE state;
  uint32_t size;
  uint16_t idx;
  uint8_t subidx;
  uint8_t bincrc;
  uint16_t blocksSize;
  uint8_t blockRemain;
  uint8_t offset;
  uint8_t timeOutCnt;
  uint16_t putC_Cnt;
  uint8_t updateBootFlash;
  eBootDeviceType bootDevice;
  eBootCodeType bootCode;
  eBootFlashSectorType flashSector;
}tXmodemInfoType;

typedef struct
{
  uint8_t rsp_data[8];
  uint8_t length;
  uint16_t idx;
  uint8_t flag;
}tXmodemRspType;

typedef struct{
  uint32_t version;
  uint32_t length;
  uint16_t type;
  uint8_t checkSum;
  uint8_t validFlag;
  uint16_t keyWord;
  uint16_t crc;
}tBootSectorInfoType;

typedef struct{
  tBootSectorInfoType sector1;
  tBootSectorInfoType sector2;
  uint32_t version;
  uint32_t length;
  uint16_t keyWord;
  uint8_t SectorIdx;
  uint8_t SectorIdxInv;
  uint16_t BIN_CRC;
  uint16_t ChkSum;
}tBootFlashType;

typedef struct{
  uint8_t rsp;
  uint16_t idx;
}tSlaveXmodemRsp_Msg;

typedef struct{
  uint16_t mark;
  uint8_t updateflag;
  uint16_t chkSum;
}tBootInfo_Type;

//#pragma location = ".textrw" 

#ifndef ALIGN
/* Compiler Related Definitions */
#ifdef __CC_ARM                         /* ARM Compiler */
    #define ALIGN(n)                    __attribute__((aligned(n)))
#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)                    PRAGMA(data_alignment=n)
#elif defined (__GNUC__)                /* GNU GCC Compiler */
    #define ALIGN(n)                    __attribute__((aligned(n)))
#endif /* Compiler Related Definitions */
#endif

void XmodemInit(void);
uint8_t XmodemMsgHandler(uint8_t *data, uint8_t length);
uint8_t XmodemRequest(uint8_t *data, uint8_t length);
//void ReadFlash(uint8_t* SBuffer, uint32_t Addr, uint16_t Datalen);

void XmodemTimeOutCb(void);
void XmodemPacketTimeOutCb(void);
void XmodemPutCharTimeOutCb(void);
void XmodemSlavePacketTimeOutCb(void);
void XmodemResetTimeOutCb(void);
void GetBootVersion(U8* data);
void XmodemSendVersion(uint8_t* para);

#endif

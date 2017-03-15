#ifndef RF_ARBITRATION_H_
#define RF_ARBITRATION_H_
#include "cc1120.h"
#include "public.h"
#include "config.h"

#define BROADCAST_ADDR      0xff

#define TIME_SEND_WAIT_MS    50
#define TIME_WAIT_ACK_MS     40
#define TIME_DETECT_MS       80
#define TIME_WAIT_MS         160
#define TIME_NEXT_ISR_MS     10
#define TIME_STOP_ISR_MS     0xffffffff

/* Packet Offset*/
#define PKT_LENGTH_OFS           0
#define PKT_DEST_ADDR_POS        (PKT_LENGTH_OFS+1)
#define PKT_CMD_OFS              (PKT_DEST_ADDR_POS+1)
#define PKT_MASTER_ID_OFS        (PKT_CMD_OFS+1)
#define PKT_FREQ_HOP_OFS         (PKT_MASTER_ID_OFS+MASTER_ID_LEN)
#define PKT_DATA_OFS             (PKT_FREQ_HOP_OFS+FREQ_HOP_LIST_LEN)


#define RF_DEBUG_OFFSET 0x08

#define RF_CMD_GET_DATA                      0x01
#define RF_CMD_GET_CELL_VOLTAGE              0x02
#define RF_CMD_GET_DATA_AND_CELL_VOLTAGE     0x03
#define RF_CMD_GET_INFO                      0x04
#define RF_CMD_SET_SLAVE                     0x05
#define RF_CMD_GET_LOG                       0x06
#define RF_CMD_CONNECT                       0x07
#define RF_CMD_GET_CVH_LOG                   0x08
#define RF_CMD_GET_DATA_DEBUG                     (0x01+RF_DEBUG_OFFSET)
#define RF_CMD_GET_CELL_VOLTAGE_DEBUG             (0x02+RF_DEBUG_OFFSET)
#define RF_CMD_GET_DATA_AND_CELL_VOLTAGE_DEBUG    (0x03+RF_DEBUG_OFFSET)
#define RF_CMD_PASS_THROUGH                  0x0d
#define RF_CMD_RTX_TEST                           0x40

#define ACK_FLAG                                    0x80
#define RF_CMD_GET_DATA_ACK                         (RF_CMD_GET_DATA|ACK_FLAG)
#define RF_CMD_GET_CELL_VOLTAGE_ACK                 (RF_CMD_GET_CELL_VOLTAGE|ACK_FLAG)
#define RF_CMD_GET_DATA_AND_CELL_VOLTAGE_ACK        (RF_CMD_GET_DATA_AND_CELL_VOLTAGE|ACK_FLAG)
#define RF_CMD_GET_INFO_ACK                         (RF_CMD_GET_INFO|ACK_FLAG)
#define RF_CMD_SET_SLAVE_ACK                        (RF_CMD_SET_SLAVE|ACK_FLAG)
#define RF_CMD_GET_LOG_ACK                          (RF_CMD_GET_LOG|ACK_FLAG)
#define RF_CMD_CONNECT_ACK                          (RF_CMD_CONNECT|ACK_FLAG)
#define RF_CMD_GET_CVH_LOG_ACK                      (RF_CMD_GET_CVH_LOG|ACK_FLAG)
#define RF_CMD_GET_DATA_DEBUG_ACK                     (RF_CMD_GET_DATA_DEBUG | ACK_FLAG)
#define RF_CMD_GET_CELL_VOLTAGE_DEBUG_ACK             (RF_CMD_GET_CELL_VOLTAGE_DEBUG | ACK_FLAG)
#define RF_CMD_GET_DATA_AND_CELL_VOLTAGE_DEBUG_ACK    (RF_CMD_GET_DATA_AND_CELL_VOLTAGE_DEBUG | ACK_FLAG)
#define RF_CMD_PASS_THROUGH_ACK                     (RF_CMD_PASS_THROUGH | ACK_FLAG)
#define RF_CMD_RTX_TEST_ACK                         (RF_CMD_RTX_TEST|ACK_FLAG)

#ifdef VERSION_BTT
#define RF_CMD_GET_BTT_DATA                  0x0C
#define RF_CMD_GET_BTT_DATA_ACK              (RF_CMD_GET_BTT_DATA|ACK_FLAG)
#endif

#define DATA_CMD_ADDR                   0X01
#define DATA_CMD_UNIXTIME               0X02
#define DATA_CMD_SLAVEID                0X03
#define DATA_CMD_RESET                  0X04
#define DATA_CMD_SLEEP                  0X05
#define DATA_CMD_FACTORY_RESET          0X06
#define DATA_CMD_CURRENT_CALIBRATION    0x07
#define DATA_CMD_SOC_CALIBRATION        0x08
#define DATA_CMD_RELAY_DRIVE            0x09
// #define DATA_CMD_INIT_PROGRAM_DOWNLOAD  0x0a
// #define DATA_CMD_PROGRAM_DOWNLOAD       0x0b
#define DATA_CMD_CURRENT_K_B_CALIBRATION    0x0C
#define DATA_CMD_CAPACITY_SETTING    0x0D

#define DATA_CMD_ADDR_ACK               (DATA_CMD_ADDR|ACK_FLAG)
#define DATA_CMD_UNIXTIME_ACK           (DATA_CMD_UNIXTIME|ACK_FLAG)
#define DATA_CMD_SLAVEID_ACK            (DATA_CMD_SLAVEID|ACK_FLAG)
#define DATA_CMD_RESET_ACK              (DATA_CMD_RESET|ACK_FLAG)
#define DATA_CMD_SLEEP_ACK              (DATA_CMD_SLEEP|ACK_FLAG)
#define DATA_CMD_FACTORY_RESET_ACK      (DATA_CMD_FACTORY_RESET|ACK_FLAG)
#define DATA_CMD_CURRENT_CALIBRATION_ACK (DATA_CMD_CURRENT_CALIBRATION|ACK_FLAG)  
#define DATA_CMD_SOC_CALIBRATION_ACK    (DATA_CMD_SOC_CALIBRATION|ACK_FLAG)     
#define DATA_CMD_RELAY_DRIVE_ACK        (DATA_CMD_RELAY_DRIVE|ACK_FLAG)
// #define DATA_CMD_INIT_PROGRAM_DOWNLOAD_ACK  (DATA_CMD_INIT_PROGRAM_DOWNLOAD|ACK_FLAG)
// #define DATA_CMD_PROGRAM_DOWNLOAD_ACK       (DATA_CMD_PROGRAM_DOWNLOAD|ACK_FLAG)

#define DATA_CMD_LEN                        1
#define DATA_CMD_ADDR_LEN                   1
#define DATA_CMD_UNIXTIME_LEN               4
#define DATA_CMD_SLAVEID_LEN                SLAVE_ID_LEN
#define DATA_CMD_RESET_LEN                  6
#define DATA_CMD_SLEEP_LEN                  2
#define DATA_CMD_FACTORY_RESET_LEN          4
#define DATA_CMD_CURRENT_CALIBRATION_LEN    3
#define DATA_CMD_RELAY_DRIVE_LEN            1
#define DATA_CMD_CURRENT_K_B_CALIBRATION_LEN 7
#define DATA_CMD_CAPACITY_SETTING_LEN        2

#define DATA_CMD_ACK_TOTAL_LEN              ((DATA_CMD_ADDR_LEN + DATA_CMD_LEN) + (DATA_CMD_UNIXTIME_LEN +DATA_CMD_LEN) +\
                                             (DATA_CMD_SLAVEID_LEN + DATA_CMD_LEN) + (DATA_CMD_RESET_LEN + DATA_CMD_LEN) + \
                                             (DATA_CMD_FACTORY_RESET_LEN + DATA_CMD_LEN))+(DATA_CMD_CURRENT_K_B_CALIBRATION_LEN+DATA_CMD_LEN)+\
																						 (DATA_CMD_CAPACITY_SETTING_LEN+DATA_CMD_LEN)\

#define CMD_LEN                             1
#define CRC_LEN                             2
#define SCAN_FREQ_LIST_NUM_MAX              3
#define COMMU_FREQ_LIST_NUM_MAX             5

#define TX_POWER_MAX_VAL 63
#define TX_POWER_MIN_VAL 3

#ifdef VERSION_BTT
#define Clear_BttUpdataFlag(POS)  (FreqHopInfo.FreqHopInfo.PCResponseFlag &= ~(1<<POS))
#define Set_BttUpdataFlag(POS)    (FreqHopInfo.FreqHopInfo.PCResponseFlag |= (1<<POS))
#define Get_BttUpdataFlag(POS)  (FreqHopInfo.FreqHopInfo.PCResponseFlag & (1<<POS))
#define Init_BttUpdataFlag()      (FreqHopInfo.FreqHopInfo.PCResponseFlag = 0)
#endif
//typedef union {
//    U32 varu32;
//    U8  varu8[4];
//} u_varform_32u8;

//typedef union {
//    U16 baru16;
//    U8  varu8[2];
//} u_varform_16u8;

// enum e_RfState {
//     RF_INIT,
//     RF_SEND_READY,
//     RF_SENDING,
//     RF_SEND_SUCCESS,
//     RF_SEND_FAIL,
//     RF_WAIT_ACK,
//     RF_DETECTING,
//     RF_WAIT_END
// };

typedef struct {
    U8 New;
    U8 Old;
} s_RfState;

//TIME MS

#define TIME_WAIT_TIMEOUT_MS    40
enum e_RfWaitAckState {
    RWA_IDLE,
    RWA_WAIT_ACK,
    RWA_WAIT_ACK_SUCCESS,
    RWA_WAIT_ACK_FAIL
};

// #define TX_RF_FREQ_LIST_LEN   5
// #define RX_RF_FREQ_LIST_LEN   1
// typedef __packed struct{
//     U16 TxStartTime;
//     U8  TxOffsetTime;
//     U16 TxIntrvl;
//     U8  TxMaxCount;
//     U8  TxFreqList[TX_RF_FREQ_LIST_LEN];
//     U16 RxSleepTime;
//     U16 RxTimeout;  
//     U8  RxFreqList[RX_RF_FREQ_LIST_LEN];
// }s_FreqHopInfo;

// #define FREQ_HOP_LIST_LEN  (sizeof(s_FreqHopInfo))
// typedef __packed union{
//     s_FreqHopInfo FreqHopInfo;
//     U8            data[FREQ_HOP_LIST_LEN];
// }u_FreqHopInfo;
typedef __packed struct{
    U8 SlaveTxPower :6;
    U8 MosEnable    :1;
    U8 rsvd         :1;
}s_MergeBits1;

typedef __packed struct
{
    U8 MasterNextFreqNomber;
    U8 MasterCurrentTimeSlicePos;
    U8 MasterSendTimeSliceLen;
    U8 SlaveSendTimeSliceLen;
    U8 SlaveAutoResponseTimes;
    U8 SlaveSumCounter;
    U8 ResponseTimeSlicePos;
    s_MergeBits1 MergeBits1;
    // U8 SlaveTxPower;
// #ifdef VERSION_BMS
//     U8 MosEnable;
// #endif
#ifdef VERSION_BTT
    U16 PCResponseFlag;
#endif
}s_FreqHopInfo;

typedef __packed struct
{
    uint16_t K;
    int32_t B;
}tUploadCalibration;

#define FREQ_HOP_LIST_LEN  (sizeof(s_FreqHopInfo))
typedef __packed union{
    s_FreqHopInfo FreqHopInfo;
    U8            data[FREQ_HOP_LIST_LEN];
}u_FreqHopInfo;

typedef __packed struct{
    uint8_t  ID[SLAVE_ID_LEN];
    uint8_t  ComAddr;
    uint8_t SoftVersion[4];
    uint8_t HardWareVersion[4];
    uint32_t FactoryReset;
    uint32_t unixtime;
    uint16_t SleepTime;
    tUploadCalibration CurrentKB[3];
    uint16_t Capacity;
    uint8_t BootVersion[3];
}s_SlaveInfo;

#define SLAVE_INFO_LEN              (sizeof(s_SlaveInfo))
typedef __packed union{
    s_SlaveInfo SlaveInfo;
    U8          data[SLAVE_INFO_LEN];
}u_SlaveInfo;

enum e_RfSyncTimerState {
    FREQ_HOP,
    FREQ_HOP_OFFSET,
    FREQ_HOP_IDLE
};

enum e_RfWorkState {
    RF_SLEEP = 0,
    RF_SCAN_FREQ,
    RF_NORMAL
};

typedef __packed struct{
    U8 SlaveID[SLAVE_ADDR_MAX];
    U8 ComAddr;
}s_ConnectInfo;
#define CONNECT_INFO_LEN (sizeof(s_ConnectInfo))

typedef __packed struct{
    U8 Update;
    U16 Current;
    U16 AdVal;
}s_CurrentCalibrationInfo;
 

void DataPrepareAndSetTxTimerCb(void);
void RfTxCb(void);
void SetScanFreqParam(void);
void SetRfSleepSniffParam(void);
void PacketParse(U8 *buffer);
void SetUplinkParam(void);
void SetDownlinkParam(void);
void RfSleepWaitTimerWake(void);
void RfRxOffsetProc(void);
void RfRxTimeoutProc(void);

//rf pass through interface
void rfGetPassThroughMsg(void* src, const uint8_t len);

#ifdef PRODUCTION_TEST
    void rfInitTestMsg(void);
    void rfTestSetTxParam(uint8_t freq_index, uint8_t tx_power);
    void rfTestSetRxParam(uint8_t freq_index);
    void rfCreatTestTxPacket(void);
    void rfSendTestTxMsg(void);
    void rfTestPacketParse(U8 *buffer);
#endif

#endif

#include "rf_service.h"
//#include "uartdata_service.h"
#include "cc1120.h"
#include "stdlib.h"
#include "string.h"
#include "public.h"
#include "w25q64.h"
#include "debug.h"
#include "timer.h"
#include "data_pack.h"
#include "power_manage.h"
#include "isl28022.h"
#include "soc.h"
#include "config.h"
#include "xmodem.h"
#include "self_check.h"
#define ETC_LEN  2
#define RF_SYNC_TIME   10
#define UNIXTIME_LEN   4

#ifdef PRODUCTION_TEST
#define RF_TEST_KEY_WORDS 0x12345678
typedef __packed struct
{
    uint32_t KeyWords;
    uint32_t TxCount;
}rfTestMsg_t;

rfTestMsg_t g_rfTestMsg = {0};
s_PacketCounter g_rfTestPacketCounter = {0};
extern scRfAck g_scRfAckMsg;

#endif

extern U8 CC1120_rxBuffer[PACKET_RX_MAX_SIZE];

U8 RfWorkState = RF_SCAN_FREQ;

static U8 ConnectedMasterID[4] = {0, 0, 0, 0};
static U8 RfTXBuffer[PACKET_TX_MAX_SIZE] = {0};
static u_FreqHopInfo FreqHopInfo = {0};
static U8 TxCount = 0;
static U8 ScanFreqList[SCAN_FREQ_LIST_NUM_MAX] = {120,130,140};
static U8 WakeUpFreqIndex = 118;
static U8 ScanSyncWord[4] = {0x7e,0xa5,0x70,0x0a};
static U8 WakeSyncWord[4] = {0x7e,0xa5,0xa0,0x70};
static U8 CommuUplinkSyncWord[4] = {0x7E,0xA5,0x5A,0xE7};
static U8 CommuDownlinkSyncWord[4] = {0xE7,0x5A,0x05,0xA0};
static U8 ConnectUpdateFlag = FALSE;
static U32 CurrentFreqIndex = 0;
static U32 CurrentTxPower = 63;
static U8 CmdAckEnable = FALSE;
static U8 RfDebugMsgEnable = FALSE;
static s_RfSignalParam RfSignalParam = {0};
// static s_DownloadInfo DownloadInfo = {0};

const static U8 ResetCode[4] = {0xaa, 0xbb, 0xcc, 0xdd};
const static U8 BroadcastBMSID[SLAVE_ID_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const static U8 FactoryResetCode[4] = {0x0a, 0x0a, 0x0a, 0x0a};
// const static U8 CommuSyncWordMask[4] = {0x7E,0xA5,0xc0,0x00};

s_CurrentCalibrationInfo CurrentCalibrationInfo[CALIBRATION_DOT_NUMBER] = {0};

#define MAX_PASS_THROUGH_BUFFER_LEN 100
extern tXmodemRspType xmdSlaveRspBuffer;

void UploadPassThroughAck(U8 *MasterID, U8 MasterIDlen, void* src, const uint8_t len);

#ifdef PRODUCTION_TEST
const U8 rfTestSyncWord[4] = {0xA5,0xE7,0x36,0x55};
void rfSetBackIdleMode(void);
uint8_t GetCurBGN(void);
void SetRfNormalParam(void);
U32 SortFreqByIndex(U8 Index);

void rfInitTestMsg(void)
{
    memset(&g_rfTestMsg, 0 , sizeof(g_rfTestMsg));
    g_rfTestMsg.KeyWords = RF_TEST_KEY_WORDS;
}

void rfTestSetTxParam(uint8_t freq_index, uint8_t tx_power)
{
    U16 RetryCount = 0xffff;
    U8 StatusByte = GetStatusByte();
    CheackStatusByte(StatusByte);

    while ((GetStatusByte() != CC1120_STATE_IDLE) && (RetryCount--)) {
        ChangeWorkMode(CC1120_WORK_IDLE);
    }

    CC1120_SetTxPower(tx_power);
    CC1120_SetCarrierFreqU32(SortFreqByIndex(freq_index));
    CC1120_SetPreamWord(0x18|0x03);
    CC1120_SetSyncWord(rfTestSyncWord[0], rfTestSyncWord[1], 
                       rfTestSyncWord[2], rfTestSyncWord[3]);
    CC1120_SetDevAddr(0xff);
    manualCalibration();
}

void rfTestSetRxParam(uint8_t freq_index)
{
    U16 RetryCount = 0xffff;
    U8 StatusByte = GetStatusByte();
    CheackStatusByte(StatusByte);

    while ((GetStatusByte() != CC1120_STATE_IDLE) && (RetryCount--)) {
        ChangeWorkMode(CC1120_WORK_IDLE);
    }

    // SetRfNormalParam();
    CC1120_SetCarrierFreqU32(SortFreqByIndex(freq_index));
    CC1120_SetPreamWord(0x18|0x03);
    CC1120_SetSyncWord(rfTestSyncWord[0], rfTestSyncWord[1], 
                       rfTestSyncWord[2], rfTestSyncWord[3]);
    CC1120_SetDevAddr(0xff);
    manualCalibration();
    trxSpiCmdStrobe(CC1120_SFRX);
    ChangeWorkMode(CC1120_WORK_RECEIVING);
}

void rfCreatTestTxPacket(void)
{
    uint8_t packet_len = 0;
    g_rfTestMsg.TxCount++;
    g_rfTestMsg.KeyWords = RF_TEST_KEY_WORDS;
    packet_len = 0;
    g_scRfAckMsg.tx++;

    RfTXBuffer[0] = sizeof(g_rfTestMsg) + PACKET_ADDR_FIELD_SIZE;
    RfTXBuffer[1] = 0xff;
    memcpy(&RfTXBuffer[2], &g_rfTestMsg, sizeof(g_rfTestMsg));

    packet_len = sizeof(g_rfTestMsg) + ETC_LEN;
    CC1120_WriteTxFifo(RfTXBuffer, packet_len);
}

void rfSendTestTxMsg(void)
{
    rfCreatTestTxPacket();
    CC1120_SendTxFifo(TIME_SEND_WAIT_MS);
}

void rfTestPacketParse(U8 *buffer)
{
    U8 BufferLen = 0;
    U8 RxAddrIndex = 0;
    rfTestMsg_t* test_msg;

    BufferLen = buffer[PKT_LENGTH_OFS] + PACKET_LENGTH_FIELD_SIZE + PACKET_RX_METRICS_SIZE;
    RxAddrIndex = buffer[PKT_DEST_ADDR_POS];
    if(RxAddrIndex == 0xff)
    {
        if(buffer[BufferLen - 1] & 0x80) {    
            test_msg = (void *)&buffer[2];
            if(memcmp((void*)&test_msg->KeyWords, (void*)&g_rfTestMsg.KeyWords, sizeof(test_msg->KeyWords)) == 0) 
            {
//                SetRfAckStateSuccess();
                g_scRfAckMsg.expect_rx = test_msg->TxCount;
                g_scRfAckMsg.rssi = buffer[BufferLen - 2];
                g_scRfAckMsg.lqi = (buffer[BufferLen - 1] & 0x7f);
                g_scRfAckMsg.rx++;
                g_scRfAckMsg.offline = g_scRfAckMsg.expect_rx - g_scRfAckMsg.rx;            
            } 
            //handle over
            trxSpiCmdStrobe(CC1120_SFRX);
            ChangeWorkMode(CC1120_WORK_RECEIVING);
            // RfRxCb();
            g_scRfAckMsg.bgn = GetCurBGN();
            g_scRfAckMsg.cinr = g_scRfAckMsg.rssi - g_scRfAckMsg.bgn;
        } else {             
            //CRC Error, detect conflict
            // DisplayBuffer(RxAddrIndex, buffer, BufferLen);
            g_scRfAckMsg.crc_err++;
            trxSpiCmdStrobe(CC1120_SFRX);
            ChangeWorkMode(CC1120_WORK_RECEIVING);
        }
    }else{
        trxSpiCmdStrobe(CC1120_SFRX);
        ChangeWorkMode(CC1120_WORK_RECEIVING);  
    }
    //clear buffer
    memset(buffer, 0, PACKET_RX_MAX_SIZE);
}

#endif

void rfSyncUnixtime(void* unixtime)
{
    memcpy((U8 *)&(g_stateblock_data.stata_union.state_data.UinxTime), unixtime, sizeof(uint32_t));
    W25Q64_SetStateMSG(&g_stateblock_data);
    W25Q64_UnixtimeSyncRecord();
}

bool rfPassThroughAck(void* src, const uint8_t len)
{
    if(len > MAX_PASS_THROUGH_BUFFER_LEN){
        return FALSE;
    }

    UploadPassThroughAck(ConnectedMasterID, MASTER_ID_LEN, src, len);
    return TRUE;
}

void rfGetPassThroughMsg(void* src, const uint8_t len)
{
//    printf("rf recv:");
//    for(uint8_t i =0;i<len;i++)
//    {
//      printf("%02x ",((uint8_t*)src)[i]);
//    }
//    printf("\n");
#ifdef XMODEMBOOT_ENABLE
    XmodemMsgHandler((uint8_t*)src,len);
    if(xmdSlaveRspBuffer.flag){
      rfPassThroughAck(xmdSlaveRspBuffer.rsp_data,xmdSlaveRspBuffer.length);
      if(((xmdSlaveRspBuffer.rsp_data[0] == XMD_EOT)||(xmdSlaveRspBuffer.rsp_data[0] == XMD_CAN))
        &&(xmdSlaveRspBuffer.length == 3))
        xmdSlaveRspBuffer.flag = TRUE;
      else  
        xmdSlaveRspBuffer.flag = FALSE;
      printf("rf send:");
      for(uint8_t i =0;i<xmdSlaveRspBuffer.length;i++)
      {
        printf("%02x ",xmdSlaveRspBuffer.rsp_data[i]);
      }
      printf("\n");
    }
    else
    {
      uint8_t temp[3] ={0};
      rfPassThroughAck(temp,3);
    }
#endif
}

#ifdef MOS_ENABLE
void MosControlProc(U8 FlagBit){
    static uint8_t CmdCount = 0;

    if(FlagBit == TRUE){
        g_battery_pack_info.error_flag.mergedBits.BMS_MosCmd = TRUE;
        W25Q64_SaveStaticErrBlock(&g_static_data);
        if(CmdCount == 1)
        {
            g_battery_pack_info.error_flag.mergedBits.BMS_MosFlt = TRUE;
            W25Q64_SaveStaticErrBlock(&g_static_data);
            MOS_ON;
        }
        else
        {
            CmdCount++;
        }
    }else{
        CmdCount = 0;
        MOS_OFF;
        g_battery_pack_info.error_flag.mergedBits.BMS_MosCmd = FALSE;
        g_battery_pack_info.error_flag.mergedBits.BMS_MosFlt = FALSE;
    }
}
#endif

U32 SortFreqByIndex(U8 Index)
{
    U32 FreqRegVal = 0;
    FreqRegVal = 0x668000 + 8192 * (U32)Index;
    return FreqRegVal;
}

U8 GetCurBGN(void)
{
    U8 Rssi0 = 0, Rssi1 = 0;
    U8 temp_rssi1 = 0;
    int Sum = 0,rssi1_val = 0;
    delay_ms(1);
    for(U16 i = 0; i < 8; i++){
        do{
            cc112xSpiReadReg(CC1120_RSSI0, &Rssi0, 1);
        }while(!(Rssi0 & 0x01));
        cc112xSpiReadReg(CC1120_RSSI1, &temp_rssi1, 1);
        if((temp_rssi1 & 0x80)){
            rssi1_val = (temp_rssi1) | 0xFFFFFF80;
        }else{
            rssi1_val = temp_rssi1;
        }
        Sum += rssi1_val;
    }
    Rssi1 = (U8)(Sum >> 3);

    return Rssi1;
}

void RfSetRxHopOffsetTimer(void)
{
    U32 RxStartTime = 0;

    RxStartTime = ((FreqHopInfo.FreqHopInfo.ResponseTimeSlicePos
        - FreqHopInfo.FreqHopInfo.MasterCurrentTimeSlicePos)
        * FreqHopInfo.FreqHopInfo.MasterSendTimeSliceLen
        + FreqHopInfo.FreqHopInfo.SlaveSendTimeSliceLen
        * FreqHopInfo.FreqHopInfo.SlaveSumCounter
        * FreqHopInfo.FreqHopInfo.SlaveAutoResponseTimes)*10;

    SetTimer1(RF_RX_TIMER_ID, (RxStartTime - RF_SYNC_TIME), TIMER_MODE_SINGLE, NULL);

}

void RfSetRxTimeoutTimer(void)
{
    SetTimer1(RF_RX_TIMEOUT_TIMER_ID,
    (FreqHopInfo.FreqHopInfo.ResponseTimeSlicePos * FreqHopInfo.FreqHopInfo.MasterSendTimeSliceLen
        * 10 + RF_SYNC_TIME), 
    TIMER_MODE_SINGLE, NULL);
}


void RfSleepWaitTimerWake(void)
{
    ChangeWorkMode(CC1120_WORK_SLEEP);
}

void RfRxOffsetProc(void)
{
//		U8 Rssi0 = 0;
    SetDownlinkParam();
    RfSetRxTimeoutTimer();
	// do{
	// 	cc112xSpiReadReg(CC1120_RSSI0, &Rssi0, 1);
	// }while(!(Rssi0 & 0x01));
 //    cc112xSpiReadReg(CC1120_RSSI1, &g_PacketCounter.BGN, 1);
    RfSignalParam.BGN = GetCurBGN();
}

void RfRxTimeoutProc(void)
{
#ifdef STATUS_LED_ENABLE
    LED_SLAVE_RUN_OFF;
#endif
#ifdef MOS_ENABLE
    MOS_OFF;
#endif
    RfWorkState = RF_SCAN_FREQ;
    SetScanFreqParam();
    SetTimer1(RF_SCAN_TIMER_ID, 1000, TIMER_MODE_PERIODIC, NULL);
    SetTimer1(RF_SLEEP_TIMER_ID, RF_SLEEP_TIMER_MS, TIMER_MODE_SINGLE, NULL);
}

void SetUplinkParam(void)
{
    U16 RetryCount = 0xffff;
    U8 StatusByte = GetStatusByte();
    CheackStatusByte(StatusByte);

    while ((GetStatusByte() != CC1120_STATE_IDLE) && (RetryCount--)) {
        ChangeWorkMode(CC1120_WORK_IDLE);
    }
    CC1120_SetTxPower(CurrentTxPower);
    CC1120_SetCarrierFreqU32(SortFreqByIndex(CurrentFreqIndex));
    CC1120_SetSyncWord(CommuUplinkSyncWord[0], CommuUplinkSyncWord[1],
                       CommuUplinkSyncWord[2], CommuUplinkSyncWord[3]);
    CC1120_SetDevAddr(g_stateblock_data.stata_union.state_data.ComAddr);

    CC1120_SetPreamWord(0x18|0x03);
    manualCalibration();
}

void SetDownlinkParam(void)
{
    U16 RetryCount = 0xffff;
    U8 StatusByte = GetStatusByte();
    CheackStatusByte(StatusByte);

    while ((GetStatusByte() != CC1120_STATE_IDLE) && (RetryCount--)) {
        ChangeWorkMode(CC1120_WORK_IDLE);
    }

    CC1120_SetCarrierFreqU32(SortFreqByIndex(CurrentFreqIndex));
    CC1120_SetPreamWord(0x18|0x00);
    CC1120_SetSyncWord(CommuDownlinkSyncWord[0], CommuDownlinkSyncWord[1],
                       CommuDownlinkSyncWord[2], CommuDownlinkSyncWord[3]);
    CC1120_SetDevAddr(g_stateblock_data.stata_union.state_data.ComAddr);
    manualCalibration();
    trxSpiCmdStrobe(CC1120_SFRX);
    ChangeWorkMode(CC1120_WORK_RECEIVING);
}

void SetScanFreqParam(void)
{
    U16 RetryCount = 0xffff;
    static U8 FreqScanIndex = 0;
    U8 StatusByte = GetStatusByte();
    CheackStatusByte(StatusByte);

    while ((GetStatusByte() != CC1120_STATE_IDLE) && (RetryCount--)) {
        ChangeWorkMode(CC1120_WORK_IDLE);
    }
    CC1120_SetPreamWord(0x18|0x00);
    CC1120_SetCarrierFreqU32(SortFreqByIndex(ScanFreqList[FreqScanIndex]));
    FreqScanIndex == (SCAN_FREQ_LIST_NUM_MAX - 1) ? (FreqScanIndex = 0):(FreqScanIndex++);
    CC1120_SetSyncWord(ScanSyncWord[0],
                       ScanSyncWord[1],
                       ScanSyncWord[2],
                       ScanSyncWord[3]);
    CC1120_SetDevAddr(0xff);
    manualCalibration();
    trxSpiCmdStrobe(CC1120_SFRX);
    ChangeWorkMode(CC1120_WORK_RECEIVING);

#ifdef RELAY_ENABLE
            SWITCH_RELAY_LOW_MCU_OFF;
            SWITCH_RELAY_HIGH_MCU_OFF;
#endif
}

void SetRfSleepSniffParam(void){
    uint8 writeByte;
    U16 RetryCount = 0xffff;
    U8 StatusByte = GetStatusByte();
    CheackStatusByte(StatusByte);

    while ((GetStatusByte() != CC1120_STATE_IDLE) && (RetryCount--)) {
        ChangeWorkMode(CC1120_WORK_IDLE);
    }
    // Reset radio
    trxSpiCmdStrobe(CC1120_SRES);

    // Write registers to radio
    for(uint16 i = 0; i < (sizeof  LowPowerSniffSettings / sizeof(registerSetting_t)); i++) {
        writeByte =  LowPowerSniffSettings[i].data;
        cc112xSpiWriteReg( LowPowerSniffSettings[i].addr, &writeByte, 1);
    }

    CC1120_SetPreamWord(0x34|0x00);  //30 byte peamble
    CC1120_SetCarrierFreqU32(SortFreqByIndex(WakeUpFreqIndex));
    CC1120_SetSyncWord(WakeSyncWord[0],
                       WakeSyncWord[1],
                       WakeSyncWord[2],
                       WakeSyncWord[3]);
    CC1120_SetDevAddr(0xff);
    manualCalibration();
    RfWorkState = RF_SLEEP;
    trxSpiCmdStrobe(CC1120_SFRX);
    ChangeWorkMode(CC1120_WORK_SNIFF);
}

void CreatPacket(U8 addr,      U8 cmd,
                 U8* masterid, U8 masterlen,
                 U8* slaveid,  U8 slavelen,
                 U8* hopinfo,  U8 hoplen,
                 U8* src1,     U8 srclen1,
                 U8* src2,     U8 srclen2,
                 U8* dest,     U8 *destlen)
{
    U8 i = 0;
    // u_varform_16u8 varform_16u8;
    // U16 CrcVal16 = 0;
    // uint32_t CrcVal32 = 0;
    // dest[ i++ ] = ETC_LEN + masterlen + slavelen + hoplen + srclen1 + srclen2 + CRC_LEN;
    dest[ i++ ] = ETC_LEN + masterlen + slavelen + hoplen + srclen1 + srclen2;
    //length err
    if(dest[0] > PACKET_TX_MAX_SIZE) 
        return;

    dest[ i++ ] = addr;
    dest[ i++ ]  = cmd;

    if(masterlen > 0) {
        memcpy(&dest[i], masterid, masterlen);
        i += masterlen;
    }
    if(slavelen > 0) {
        memcpy(&dest[i], slaveid, slavelen);
        i += slavelen;
    }
    if(hoplen > 0) {
        memcpy(&dest[i], hopinfo, hoplen);
        i += hoplen;
    }
    if(srclen1 > 0) {
        memcpy(&dest[i], src1, srclen1);
        i += srclen1;
    }
    if(srclen2 > 0) {
        memcpy(&dest[i], src2, srclen2);
        i += srclen2;
    }
    //add crc
    // Crc_MakeCrc(dest, i, &CrcVal32);
    // CrcVal16 = (U16)CrcVal32;
    // varform_16u8.baru16 = CrcVal16;
    // memcpy(&dest[i], varform_16u8.varu8, CRC_LEN);
    // i += CRC_LEN;     
    *destlen = i; 
}

void UploadLogData(U8 *MasterID, U8 len)
{
    U8 PacketLen = 0;
    U8 RfDebugMsgLen = 0;
    U8 AckCmd = RF_CMD_GET_DATA_ACK;

    if(RfDebugMsgEnable == TRUE){
        //move rssi
        RfDebugMsgLen = sizeof(s_PacketCounter);
        AckCmd += RF_DEBUG_OFFSET;
    }

    SetUplinkParam();
	CalcCycleCount();
    g_log_data.log_data.CycleCount = g_stateblock_data.stata_union.state_data.BatteryOfSoc.CycleCount;
    CreatPacket(g_stateblock_data.stata_union.state_data.ComAddr, AckCmd,
                MasterID,                                         len,
                (U8 *)&RfSignalParam,                  sizeof(s_RfSignalParam),
                FreqHopInfo.data,                                 0,
                &g_log_data.data[1],                              29,
                (U8 *)&g_PacketCounter,                           RfDebugMsgLen,
                RfTXBuffer,                                       &PacketLen);
    CC1120_WriteTxFifo(RfTXBuffer, PacketLen);
}

void UploadCellVoltages(U8 *MasterID, U8 len)
{
    U8 PacketLen = 0;
    U8 RfDebugMsgLen = 0;
    U8 AckCmd = RF_CMD_GET_CELL_VOLTAGE_ACK;

    if(RfDebugMsgEnable == TRUE){
        //move rssi
        RfDebugMsgLen = sizeof(s_PacketCounter);
        AckCmd += RF_DEBUG_OFFSET;
    }

    SetUplinkParam();
    CreatPacket(g_stateblock_data.stata_union.state_data.ComAddr, AckCmd,
                MasterID,                                         len,
                (U8 *)&RfSignalParam,                  sizeof(s_RfSignalParam),
                FreqHopInfo.data,                                 0,
                (U8 *)&g_battery_pack_info.sample_data.cell_voltages,   (CELL_VOLTAGE_CHANNEL * 2),
                (U8 *)&g_PacketCounter,                           RfDebugMsgLen,
                RfTXBuffer,                                       &PacketLen);
    CC1120_WriteTxFifo(RfTXBuffer, PacketLen);
}

void UploadLogDataAndCellVoltages(U8 *MasterID, U8 len)
{
    U8 PacketLen = 0;
    U8 RfDebugMsgLen = 0;
    U8 AckCmd = RF_CMD_GET_DATA_AND_CELL_VOLTAGE_ACK;
    if(RfDebugMsgEnable == TRUE){
        //move rssi
        RfDebugMsgLen = sizeof(s_PacketCounter);
        AckCmd += RF_DEBUG_OFFSET;
    }
    SetUplinkParam();
    CreatPacket(g_stateblock_data.stata_union.state_data.ComAddr, AckCmd,
                MasterID,                                         len,
                (U8 *)&RfSignalParam,                  sizeof(s_RfSignalParam),
                &g_log_data.data[1],                              29,
                (U8 *)&g_battery_pack_info.sample_data.cell_voltages,   (CELL_VOLTAGE_CHANNEL * 2),
                (U8 *)&g_PacketCounter,                           RfDebugMsgLen,
                RfTXBuffer,                                       &PacketLen);
    CC1120_WriteTxFifo(RfTXBuffer, PacketLen);
}

void FormatCurrentKB(tCalibration* Calibration, tUploadCalibration* CurrentKB)
{
    CurrentKB->K = Calibration->K;
    CurrentKB->B = Calibration->B;
}

void UploadSlaveInfo(U8 *MasterID, U8 len)
{
    U8 PacketLen = 0;
    U8 tempbuff[1] = {0};
    static u_SlaveInfo SlaveInfo;
    memset(&SlaveInfo, 0, sizeof(u_SlaveInfo));
    SlaveInfo.SlaveInfo.ComAddr = g_stateblock_data.stata_union.state_data.ComAddr;
    memcpy((U8*)&SlaveInfo.SlaveInfo.HardWareVersion, g_HardWareVersion, sizeof(g_HardWareVersion));
    memcpy((U8*)&SlaveInfo.SlaveInfo.SoftVersion, g_SoftVersion, sizeof(g_SoftVersion));
    SlaveInfo.SlaveInfo.FactoryReset = g_static_data.static_data.FactoryReset;
    memcpy(SlaveInfo.SlaveInfo.ID, g_static_data.static_data.BMSID, SLAVE_ID_LEN);
    SlaveInfo.SlaveInfo.unixtime = g_stateblock_data.stata_union.state_data.UinxTime;
    FormatCurrentKB(&g_static_data.static_data.Calibration[CHARGE_40A], &SlaveInfo.SlaveInfo.CurrentKB[0]);
    FormatCurrentKB(&g_static_data.static_data.Calibration[DISCHARGE_10A], &SlaveInfo.SlaveInfo.CurrentKB[1]);
    FormatCurrentKB(&g_static_data.static_data.Calibration[DISCHARGE_100A], &SlaveInfo.SlaveInfo.CurrentKB[2]);
    SlaveInfo.SlaveInfo.Capacity = g_stateblock_data.stata_union.state_data.BatteryOfSoc.NowCapacity;
    #ifdef XMODEMBOOT_ENABLE
    GetBootVersion((void*)&SlaveInfo.SlaveInfo.BootVersion);
    #endif
    SetUplinkParam();
    CreatPacket(g_stateblock_data.stata_union.state_data.ComAddr, RF_CMD_GET_INFO_ACK,
                MasterID,                                         len,
                (U8 *)&RfSignalParam,                  sizeof(s_RfSignalParam),
                FreqHopInfo.data,                                 0,
                SlaveInfo.data,                                   SLAVE_INFO_LEN,
                tempbuff,                                         0,
                RfTXBuffer,                                       &PacketLen);
    CC1120_WriteTxFifo(RfTXBuffer, PacketLen);
}

#ifdef VERSION_BMS
void UploadLogByNum(U8 *MasterID, U8 len, U32 Num, U8 LogType)
{
    U8 PacketLen = 0;
    U8 tempbuff[1] = {0};
    U8 UploadMsgLen = 0;
    static U8 TempUploadBuff[W25Q64_DATA_BLOCKSIZE] = {0};
    if(LogType == RF_CMD_GET_LOG_ACK){
        g_flashdata_frame.tail = g_flashdata_frame.head;
        UploadMsgLen = W25Q64_DATA_BLOCKSIZE;
        if(W25Q64_FindSelectedDataAddr(Num) == TRUE){
            if(W25Q64_DataBlockTop(&g_flashdata_frame, TempUploadBuff) == TRUE){

            }else{
                memset(TempUploadBuff, 0xff, W25Q64_DATA_BLOCKSIZE);
            }
        }else{
            memset(TempUploadBuff, 0xff, W25Q64_DATA_BLOCKSIZE);
        }
    }else if(LogType == RF_CMD_GET_CVH_LOG_ACK){
        UploadMsgLen = sizeof(s_StaticErrBlock);
        U8 Index = g_static_data.static_data.StaticErrBlockFrame.HeadAddr;
		if(Index >= CELL_H_VOLT_BLOCK_NUM){
			Index = CELL_H_VOLT_BLOCK_NUM - 1;
		}
        for(U8 i = Num; i > 0; i--){
            (Index <= 0) ? (Index = CELL_H_VOLT_BLOCK_NUM - 1) : (Index--);
        }
        memcpy(TempUploadBuff, &g_static_data.static_data.StaticErrBlockFrame.StaticErrBlock[Index], UploadMsgLen);
    }

    SetUplinkParam();
    CreatPacket(g_stateblock_data.stata_union.state_data.ComAddr, LogType,
                MasterID,                                         len,
                (U8 *)&RfSignalParam,                  sizeof(s_RfSignalParam),
                FreqHopInfo.data,                                 0,
                TempUploadBuff,                                   UploadMsgLen,
                tempbuff,                                         0,
                RfTXBuffer,                                       &PacketLen);
    CC1120_WriteTxFifo(RfTXBuffer, PacketLen);
}
#endif

void UploadPassThroughAck(U8 *MasterID, U8 MasterIDlen, void* src, const uint8_t len)
{
    U8 PacketLen = 0;
    U8 tempbuff[1] = {0};
    SetUplinkParam();
    CreatPacket(g_stateblock_data.stata_union.state_data.ComAddr, RF_CMD_PASS_THROUGH_ACK,
                MasterID,                                         MasterIDlen,
                (U8 *)&RfSignalParam,                  sizeof(s_RfSignalParam),
                FreqHopInfo.data,                                 0,
                src,                                   len,
                tempbuff,                                         0,
                RfTXBuffer,                                       &PacketLen);
    CC1120_WriteTxFifo(RfTXBuffer, PacketLen);
}

void ConnectCmdAck(U8 *MasterID, U8 len, U8 Flag)
{
    U8 PacketLen = 0;
    U8 tempbuff[1] = {0};
    U8 AckCmd = RF_CMD_CONNECT;
    if(Flag == TRUE){
        AckCmd = RF_CMD_CONNECT_ACK;
    }
    SetUplinkParam();
    CreatPacket(g_stateblock_data.stata_union.state_data.ComAddr, AckCmd,
                MasterID,                                         len,
                (U8 *)&RfSignalParam,                  sizeof(s_RfSignalParam),
                FreqHopInfo.data,                                 0,
                tempbuff, 0,
                tempbuff, 0,
                RfTXBuffer,                                       &PacketLen);
    CC1120_WriteTxFifo(RfTXBuffer, PacketLen);
}

void CreatRTxTestBuffer(U8 StartNum, U8 *dest, U8 len)
{
    U8 num = StartNum;
    for(U8 i = 0; i < len; i++) {
        *dest++ = num++;
    }
}

void UploadRTxTest(U8 *MasterID, U8 len, U8 StartNum)
{
    U8 PacketLen = 0;
    U8 tempbuff[1] = {0};
    static U8 RTxTestBuffer[CELL_VOLTAGE_SIZE] = {0};
    CreatRTxTestBuffer(StartNum, RTxTestBuffer, CELL_VOLTAGE_SIZE);
    SetUplinkParam();
    CreatPacket(g_stateblock_data.stata_union.state_data.ComAddr, RF_CMD_RTX_TEST_ACK,
                MasterID,                                         len,
                (U8 *)&RfSignalParam,                  sizeof(s_RfSignalParam),
                FreqHopInfo.data,                                 0,
                RTxTestBuffer,                                    CELL_VOLTAGE_SIZE,
                tempbuff,                                         0,
                RfTXBuffer,                                       &PacketLen);
    CC1120_WriteTxFifo(RfTXBuffer, PacketLen);
}

void AckSetCmd(U8 *MasterID, U8 MasterIDlen , U8* src, U8 len)
{
    U8 PacketLen = 0;
    U8 tempbuff[1] = {0};
    SetUplinkParam();
    CreatPacket(g_stateblock_data.stata_union.state_data.ComAddr, RF_CMD_SET_SLAVE_ACK,
                MasterID,                                         MasterIDlen,
                (U8 *)&RfSignalParam,                  sizeof(s_RfSignalParam),
                FreqHopInfo.data,                                 0,
                src,                                              len,
                tempbuff,                                         0,
                RfTXBuffer,                                       &PacketLen);
    CC1120_WriteTxFifo(RfTXBuffer, PacketLen);
}

#ifdef VERSION_BTT
#ifdef BTT_RF
void UploadBatteryTestToolData(U8 *MasterID, U8 len)
{
    U8 PacketLen = 0;
    U8 AckCmd = RF_CMD_GET_BTT_DATA_ACK;

    SetUplinkParam();
    CreatPacket(g_stateblock_data.stata_union.state_data.ComAddr, AckCmd,
                MasterID,                                         len,
                (U8 *)&RfSignalParam,                  sizeof(s_RfSignalParam),
                FreqHopInfo.data,                                 0,
                (U8 *)&g_BatteryTestToolFrameBlock.BatteryTestToolFrame,   sizeof(s_BatteryTestToolFrame),
                (U8 *)&g_PacketCounter,                           0,
                RfTXBuffer,                                       &PacketLen);
    CC1120_WriteTxFifo(RfTXBuffer, PacketLen);
}    
#endif
#endif

void SetCmdParseAndMakeAckString(U8* CmdBuffer, U8* CmdLen, U8* Dest, U8 *len)
{
    U8 Cmd = CmdBuffer[0],DotIndex=0;
    u_varform_32u8 varform_32u8;
    // s_BinInfo* BinInfoPtr;
    // U8 BinDownloadLen = 0;
    // U16 TempSeqNum = 0;
    // u_varform_16u8 varform_16u8;
//    U16 SleepTime = 0;

    switch(Cmd) {
        case DATA_CMD_ADDR:
            g_stateblock_data.stata_union.state_data.ComAddr = CmdBuffer[1];
//            W25Q64_SetStateMSG(&g_stateblock_data, &g_flashdata_frame);
            *Dest++  = DATA_CMD_ADDR_ACK;
            *Dest    = g_stateblock_data.stata_union.state_data.ComAddr;
            *CmdLen  = DATA_CMD_ADDR_LEN + DATA_CMD_LEN;
            *len    += *CmdLen;
        break;

        case DATA_CMD_UNIXTIME:
            memcpy(varform_32u8.varu8, &CmdBuffer[1], 4);
            g_stateblock_data.stata_union.state_data.UinxTime = varform_32u8.varu32;
            W25Q64_SetStateMSG(&g_stateblock_data);
            *Dest++  = DATA_CMD_UNIXTIME_ACK;
            memcpy(Dest, varform_32u8.varu8, 4);
            *CmdLen  = DATA_CMD_UNIXTIME_LEN + DATA_CMD_LEN;
            *len    += *CmdLen;
        break;

        case DATA_CMD_SLAVEID:
            memcpy(g_static_data.static_data.BMSID, &CmdBuffer[1], SLAVE_ID_LEN);
            W25Q64_SetStaticMSG(&g_static_data);
            *Dest++  = DATA_CMD_SLAVEID_ACK;
            memcpy(Dest, g_static_data.static_data.BMSID, SLAVE_ID_LEN);
            *CmdLen  = DATA_CMD_SLAVEID_LEN + DATA_CMD_LEN;
            *len    += *CmdLen;
        break;

        case DATA_CMD_RESET:
            if(memcmp(ResetCode, &CmdBuffer[1], 4) == 0) {
                CmdAckEnable = FALSE;
                Cpu_Reset();
            }
        break;

        case DATA_CMD_SLEEP:
            StopTimer1(RF_RX_TIMER_ID);
            StopTimer1(RF_TX_TIMER_ID);
            CmdAckEnable = FALSE;
            SysPowerDown();
            *Dest++  = DATA_CMD_SLEEP_ACK;
            *CmdLen  = DATA_CMD_LEN;
            *len    += *CmdLen;
        break;

        case DATA_CMD_FACTORY_RESET:
            // if(memcmp(FactoryResetCode, &CmdBuffer[1], 4) == 0) {
                // memcpy(varform_32u8.varu8, &CmdBuffer[1], 4);
                // g_static_data.static_data.FactoryReset = varform_32u8.varu32;
                memcpy((U8*)&g_static_data.static_data.FactoryReset, &CmdBuffer[1],
                    DATA_CMD_FACTORY_RESET_LEN);
                W25Q64_SetStaticMSG(&g_static_data);
                if(memcmp(FactoryResetCode, (U8*)&g_static_data.static_data.FactoryReset,
                    DATA_CMD_FACTORY_RESET_LEN) == 0) {
                    g_stateblock_data.stata_union.state_data.BatteryOfSoc.CycleCount = 0;
                    W25Q64_SetStateMSG(&g_stateblock_data);
                }
                //To do FactoryReset()
                *Dest++  = DATA_CMD_FACTORY_RESET_ACK;
                memcpy(Dest, (U8*)&g_static_data.static_data.FactoryReset, DATA_CMD_FACTORY_RESET_LEN);
                *CmdLen  = DATA_CMD_FACTORY_RESET_LEN + DATA_CMD_LEN;
                *len    += *CmdLen;
            // }
        break;

        case DATA_CMD_SOC_CALIBRATION:
            *Dest++  = DATA_CMD_SOC_CALIBRATION_ACK;
            *CmdLen  = DATA_CMD_LEN;
            *len    += *CmdLen;
		    Set_IntFlag(SOC_CALIBRATION_FLAG_POS);
            g_soc_status=SOC_INIT;
				
        break;

        case DATA_CMD_CURRENT_CALIBRATION:
            if(memcmp(FactoryResetCode, (U8*)&g_static_data.static_data.FactoryReset, 4) != 0) {
								DotIndex = (CmdBuffer[1]<CALIBRATION_DOT_NUMBER) ? CmdBuffer[1]:(CALIBRATION_DOT_NUMBER-1);
								memcpy((U8 *)&CurrentCalibrationInfo[DotIndex].Current, &CmdBuffer[2], 2);

								ISL28022_Calibration(CurrentCalibrationInfo[DotIndex].Current,DotIndex);
								W25Q64_SetStaticMSG(&g_static_data);
								*Dest++  = DATA_CMD_CURRENT_CALIBRATION_ACK;

            }else{
                *Dest++  = DATA_CMD_CURRENT_CALIBRATION;
            }
            memcpy(Dest, &CmdBuffer[1], 3);
            *CmdLen  = DATA_CMD_CURRENT_CALIBRATION_LEN + DATA_CMD_LEN;
            *len    += *CmdLen;
        break;
						
        case DATA_CMD_CURRENT_K_B_CALIBRATION:
            if(memcmp(FactoryResetCode, (U8*)&g_static_data.static_data.FactoryReset, 4) != 0) { 
							
							tCalibration *pCalibration=g_static_data.static_data.Calibration;	

							DotIndex = CmdBuffer[1];
							
							if(DotIndex==CHARGE_40A||DotIndex==DISCHARGE_10A||DotIndex==DISCHARGE_100A)
							{
							pCalibration[DotIndex].K=((U16)CmdBuffer[3]<<8)|CmdBuffer[2];
							pCalibration[DotIndex].B=((U32)CmdBuffer[7]<<24)|((U32)CmdBuffer[6]<<16)|((U32)CmdBuffer[5]<<8)|CmdBuffer[4];
              
     
							W25Q64_SetStaticMSG(&g_static_data);
							*Dest++  = DATA_CMD_CURRENT_K_B_CALIBRATION|ACK_FLAG;
							}else
							{
							*Dest++  = DATA_CMD_CURRENT_K_B_CALIBRATION;
							}
      
            }else{
                *Dest++  = DATA_CMD_CURRENT_K_B_CALIBRATION;
            }
            memcpy(Dest, &CmdBuffer[1], DATA_CMD_CURRENT_K_B_CALIBRATION_LEN);
            *CmdLen  = DATA_CMD_CURRENT_K_B_CALIBRATION_LEN + DATA_CMD_LEN;
            *len    += *CmdLen;
        break;
						
        case DATA_CMD_CAPACITY_SETTING:
            if(memcmp(FactoryResetCode, (U8*)&g_static_data.static_data.FactoryReset, 4) != 0) {
			          g_stateblock_data.stata_union.state_data.BatteryOfSoc.NowCapacity=(((U16)CmdBuffer[2])<<8)|CmdBuffer[1];
						 
				
			         W25Q64_SetStateMSG(&g_stateblock_data);
                *Dest++  = DATA_CMD_CAPACITY_SETTING|ACK_FLAG;
      
            }else{
                *Dest++  = DATA_CMD_CAPACITY_SETTING;
            }
            memcpy(Dest, &CmdBuffer[1], DATA_CMD_CAPACITY_SETTING_LEN);
            *CmdLen  = DATA_CMD_CAPACITY_SETTING_LEN + DATA_CMD_LEN;
            *len    += *CmdLen;
        break;						
						
#ifdef RELAY_ENABLE
        case DATA_CMD_RELAY_DRIVE:
            if(CmdBuffer[1] == TRUE){
                SWITCH_RELAY_HIGH_MCU_ON;
                SWITCH_RELAY_LOW_MCU_ON;
                *Dest++  = DATA_CMD_RELAY_DRIVE_ACK;
            }else if(CmdBuffer[1] == FALSE){
                SWITCH_RELAY_HIGH_MCU_OFF;
                SWITCH_RELAY_LOW_MCU_OFF;
                *Dest++  = DATA_CMD_RELAY_DRIVE_ACK;
            }else{
                *Dest++  = DATA_CMD_RELAY_DRIVE;
            }           
            *Dest++  = CmdBuffer[1];
            *CmdLen  = DATA_CMD_LEN + DATA_CMD_RELAY_DRIVE_LEN;
            *len    += *CmdLen;
        break;
#endif

        // case DATA_CMD_INIT_PROGRAM_DOWNLOAD:
        //     BinInfoPtr = (s_BinInfo*)&CmdBuffer[2];
        //     W25Q64_EraseProgramArea(W25Q64_SLAVE_PROGRAM_START_ADDR);
        //     SPI_Flash_Write_NoCheck((U8*)BinInfoPtr, W25Q64_SLAVE_PROGRAM_START_ADDR, sizeof(s_BinInfo));
        //     DownloadInfo.Length = BinInfoPtr->Length;
        //     DownloadInfo.SeqNum = 0;
        //     DownloadInfo.RetryCountMax = CmdBuffer[1];
        //     DownloadInfo.WriteAddr = W25Q64_SLAVE_PROGRAM_START_ADDR + sizeof(s_BinInfo);
        //     *Dest++  = DATA_CMD_INIT_PROGRAM_DOWNLOAD_ACK;
        //     *CmdLen  = DATA_CMD_LEN;
        //     *len    += *CmdLen;
        // break;

        // case DATA_CMD_PROGRAM_DOWNLOAD:
        //     memcpy((U8*)&TempSeqNum, &CmdBuffer[1], sizeof(U16));
        //     if(TempSeqNum == DownloadInfo.SeqNum + 1){
        //     //haven't download
        //         DownloadInfo.SeqNum++;
        //         BinDownloadLen = CmdBuffer[3];
        //         SPI_Flash_Write_NoCheck((U8*)&CmdBuffer[4], DownloadInfo.WriteAddr, BinDownloadLen);
        //         DownloadInfo.WriteAddr += BinDownloadLen;
        //         *Dest++  = DATA_CMD_PROGRAM_DOWNLOAD_ACK;
        //     }else if(TempSeqNum == DownloadInfo.SeqNum){
        //     //have downloaded
        //         *Dest++  = DATA_CMD_PROGRAM_DOWNLOAD_ACK;
        //     }else{
        //     //err download msg
        //         *Dest++  = DATA_CMD_PROGRAM_DOWNLOAD;
        //     }
        //     memcpy(Dest, (U8*)&TempSeqNum, sizeof(U16));
        //     *CmdLen  = DATA_CMD_LEN + sizeof(U16);
        //     *len    += *CmdLen;
        // break;

        default:
        break;
    }
}

void SetCmdSplitAndMakeAckBuffer(U8 *DataBuffer, U8* Dest, U8 *len)
{
    U8 RemainCmdLen = DataBuffer[0];
    U8 CmdLen = 0;
    *Dest++ = DataBuffer[0];
    (*len) ++;
    DataBuffer++;
    if((memcmp(DataBuffer, BroadcastBMSID, SLAVE_ID_LEN) == 0)
        ||(memcmp(DataBuffer, g_static_data.static_data.BMSID, SLAVE_ID_LEN) == 0)){
        memcpy(Dest, DataBuffer, SLAVE_ID_LEN);
        Dest += SLAVE_ID_LEN;
        (*len) += SLAVE_ID_LEN;
        DataBuffer += SLAVE_ID_LEN;
        RemainCmdLen -= SLAVE_ID_LEN;
        //Parse every cmd
        while(RemainCmdLen > 0) {
            SetCmdParseAndMakeAckString(DataBuffer, &CmdLen, Dest, len);
            DataBuffer += CmdLen;
            RemainCmdLen -= CmdLen;
            Dest += CmdLen;
        }
    }else{
        CmdAckEnable = FALSE;
    }
}

U8 CheackPacketCrc(U8 *src, U8 len)
{
    u_varform_16u8 varform_16u8;    
    U16 CrcVal16 = 0;
    uint32_t CrcVal32 = 0;
    Crc_MakeCrc(src,len-4,&CrcVal32);
    CrcVal16 = (U16)CrcVal32;
    memcpy(varform_16u8.varu8,&src[len-4],CRC_LEN);
    if(varform_16u8.baru16 == CrcVal16){
        return TRUE;
    }else{
        return FALSE;
    }

}

void CommandParse(U8 *buffer)
{
    static U8 SetCmdAckBuffer[DATA_CMD_ACK_TOTAL_LEN] = {0};
    static U8 StartNum = 0;
    U8 AckBufferlen = 0;
//    u_varform_32u8 varform_32u8;
    // RfDebugMsgEnable = FALSE;
#ifdef STATUS_LED_ENABLE
    LED_SLAVE_RUN_ON;
#endif
#ifdef VERSION_BMS
		u_varform_32u8 varform_32u8;
		
    // if((buffer[PKT_CMD_OFS] & RF_DEBUG_OFFSET) != 0){
    //     buffer[PKT_CMD_OFS] -= RF_DEBUG_OFFSET;
    if((buffer[PKT_CMD_OFS] == RF_CMD_GET_DATA_DEBUG) ||
        (buffer[PKT_CMD_OFS] == RF_CMD_GET_CELL_VOLTAGE_DEBUG) ||
        (buffer[PKT_CMD_OFS] == RF_CMD_GET_DATA_AND_CELL_VOLTAGE_DEBUG)){
        buffer[PKT_CMD_OFS] -= RF_DEBUG_OFFSET;
        RfDebugMsgEnable = TRUE;
    }
    switch(buffer[PKT_CMD_OFS]) {
        case RF_CMD_GET_DATA:
            // if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0) {
                DataSamplingAndProcess();
                UploadLogData(ConnectedMasterID, MASTER_ID_LEN);
            // }
        break;

        case RF_CMD_GET_CELL_VOLTAGE:
            // if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0) {
                DataSamplingAndProcess();
                UploadCellVoltages(ConnectedMasterID, MASTER_ID_LEN);
            // }
        break;

        case RF_CMD_GET_DATA_AND_CELL_VOLTAGE:
            // if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0) {
                DataSamplingAndProcess();
                UploadLogDataAndCellVoltages(ConnectedMasterID, MASTER_ID_LEN);
            // }
        break;

        case RF_CMD_GET_INFO:
            // if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0) {
                UploadSlaveInfo(ConnectedMasterID, MASTER_ID_LEN);
            // }
        break;

        case RF_CMD_SET_SLAVE:
            // if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0) {
                CmdAckEnable = TRUE;
                SetCmdSplitAndMakeAckBuffer(&buffer[PKT_DATA_OFS], SetCmdAckBuffer, &AckBufferlen);
                if(CmdAckEnable == TRUE){
                     AckSetCmd(ConnectedMasterID, MASTER_ID_LEN, SetCmdAckBuffer, AckBufferlen);
                 }
            // }
        break;

        case RF_CMD_GET_LOG:
            // if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0) {
                StopTimer1(FLASH_DATA_BLOCK_SAVE_ID);
                memcpy(varform_32u8.varu8, &buffer[PKT_DATA_OFS], 4);
                UploadLogByNum(ConnectedMasterID, MASTER_ID_LEN, varform_32u8.varu32, RF_CMD_GET_LOG_ACK);
            // }
        break;

        case RF_CMD_CONNECT:
            if(ConnectUpdateFlag == TRUE){
                ConnectCmdAck(ConnectedMasterID, MASTER_ID_LEN, TRUE);
            }else{
                // ConnectCmdAck(ConnectedMasterID, MASTER_ID_LEN, FALSE);
            }
            ConnectUpdateFlag = FALSE;
        break;

        case RF_CMD_GET_CVH_LOG:
            memcpy(varform_32u8.varu8, &buffer[PKT_DATA_OFS], 4);
            UploadLogByNum(ConnectedMasterID, MASTER_ID_LEN, varform_32u8.varu32, RF_CMD_GET_CVH_LOG_ACK);
        break;

        case RF_CMD_RTX_TEST:
            // if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0) {
                UploadRTxTest(ConnectedMasterID, MASTER_ID_LEN, StartNum);
                StartNum += 5;
            // }
        break;

        case RF_CMD_PASS_THROUGH:
            rfGetPassThroughMsg(&buffer[PKT_DATA_OFS], buffer[PKT_LENGTH_OFS] -
            (MASTER_ID_LEN + FREQ_HOP_LIST_LEN + PACKET_ADDR_FIELD_SIZE + PACKET_CMD_FIELD_SIZE) );
        break;

        default:
            trxSpiCmdStrobe(CC1120_SFRX);
            ChangeWorkMode(CC1120_WORK_RECEIVING);
        break;
    }
#endif

#ifdef VERSION_BTT
#ifdef BTT_RF
    // static U8 FirstRecCmd = TRUE;
    // static U8 UpdataFlag = TRUE;
    // LED_SLAVE_RUN_ON;
    // SetTimer1(RF_COMMU_TIMEOUT_ID, RF_COMMU_TIMEOUT_MS, TIMER_MODE_SINGLE, NULL);

    switch(buffer[PKT_CMD_OFS]) {

        case RF_CMD_GET_BTT_DATA:

            // if(FirstRecCmd == TRUE){
            //     FirstRecCmd = FALSE;
            //     if(W25Q64_DataBlockTop(&g_flashdata_frame, (U8 *)&g_BatteryTestToolFrameBlock) == TRUE){
            //         g_BatteryTestToolFrameBlock.BatteryTestToolFrame.UpdataFlag = TRUE;
            //         FlashPtrUpdataFlag = TRUE;
            //     }else{
            //         g_BatteryTestToolFrameBlock.BatteryTestToolFrame.UpdataFlag = FALSE;
            //         W25Q64_DataBlockPop(&g_flashdata_frame);
            //     }
            // }else{
            //     if(Get_BttUpdataFlag(g_stateblock_data.stata_union.state_data.ComAddr - 1) == FALSE){

            //     }else{
            //         if(FlashPtrUpdataFlag == TRUE){
            //             FlashPtrUpdataFlag = FALSE;
            //             W25Q64_SetStateMSG(&g_stateblock_data, &g_flashdata_frame);
            //             W25Q64_DataBlockPop(&g_flashdata_frame);
            //         }

            //         if(W25Q64_DataBlockTop(&g_flashdata_frame, (U8 *)&g_BatteryTestToolFrameBlock) == TRUE){
            //             g_BatteryTestToolFrameBlock.BatteryTestToolFrame.UpdataFlag = TRUE;
            //             FlashPtrUpdataFlag = TRUE;
            //         }else{
            //             g_BatteryTestToolFrameBlock.BatteryTestToolFrame.UpdataFlag = FALSE;
            //             W25Q64_DataBlockPop(&g_flashdata_frame);
            //         }
            //     }
            // }

            if(Get_BttUpdataFlag(g_stateblock_data.stata_union.state_data.ComAddr - 1) == FALSE){
                W25Q64_DataBlockTop(&g_flashdata_frame, (U8 *)&g_BatteryTestToolFrameBlock);
                if(g_dataptr.FlashDataPtr.DataValidFlag == FALSE){
                    g_BatteryTestToolFrameBlock.BatteryTestToolFrame.UpdataFlag = FALSE;
                }else{
                    // W25Q64_DataBlockTop(&g_flashdata_frame, (U8 *)&g_BatteryTestToolFrameBlock);
                    g_BatteryTestToolFrameBlock.BatteryTestToolFrame.UpdataFlag = TRUE;
                }
            }else{
				g_BatteryTestToolFrameBlock.BatteryTestToolFrame.UpdataFlag = FALSE;
                if(g_flashdata_frame.empty == TRUE){
                    if(g_dataptr.FlashDataPtr.DataValidFlag == TRUE){
                        g_dataptr.FlashDataPtr.DataValidFlag = FALSE;
                        // W25Q64_SetStateMSG(&g_stateblock_data, &g_flashdata_frame);
                        W25Q64_SetDataPtrMSG(&g_dataptr, &g_flashdata_frame);
                    }
                    // UpdataFlag = FALSE;
                }else{
                    // W25Q64_DataBlockPop(&g_flashdata_frame);
                    // W25Q64_SetStateMSG(&g_stateblock_data, &g_flashdata_frame);
                    W25Q64_DataBlockPop(&g_flashdata_frame);
                    g_dataptr.FlashDataPtr.DataValidFlag = TRUE;
                    // W25Q64_SetStateMSG(&g_stateblock_data, &g_flashdata_frame);
                    W25Q64_SetDataPtrMSG(&g_dataptr, &g_flashdata_frame);
					W25Q64_DataBlockTop(&g_flashdata_frame, (U8 *)&g_BatteryTestToolFrameBlock);
					g_BatteryTestToolFrameBlock.BatteryTestToolFrame.UpdataFlag = TRUE;
                    // UpdataFlag = TRUE;
                }
                // if(W25Q64_DataBlockPop(&g_flashdata_frame) == TRUE){
                //     W25Q64_SetStateMSG(&g_stateblock_data, &g_flashdata_frame);
                //     UpdataFlag = TRUE;
                // }else{
                //     UpdataFlag = FALSE;
                // }
            }
        // }
            UploadBatteryTestToolData(ConnectedMasterID, MASTER_ID_LEN);
        break;

        case RF_CMD_GET_INFO:
            // if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0) {
                UploadSlaveInfo(ConnectedMasterID, MASTER_ID_LEN);
            // }
        break;

        case RF_CMD_SET_SLAVE:
            // if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0) {
                CmdAckEnable = TRUE;
                SetCmdSplitAndMakeAckBuffer(&buffer[PKT_DATA_OFS], SetCmdAckBuffer, &AckBufferlen);
                if(CmdAckEnable == TRUE){
                     AckSetCmd(ConnectedMasterID, MASTER_ID_LEN, SetCmdAckBuffer, AckBufferlen);
                 }
            // }
        break;

        // case RF_CMD_GET_LOG:
        //     // if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0) {
        //         memcpy(varform_32u8.varu8, &buffer[PKT_DATA_OFS], 4);
        //         UploadLogByNum(ConnectedMasterID, MASTER_ID_LEN, varform_32u8.varu32);
        //     // }
        // break;

        case RF_CMD_CONNECT:
            if(ConnectUpdateFlag == TRUE){
                ConnectCmdAck(ConnectedMasterID, MASTER_ID_LEN, TRUE);
            }else{
                // ConnectCmdAck(ConnectedMasterID, MASTER_ID_LEN, FALSE);
            }
            ConnectUpdateFlag = FALSE;
        break;

        case RF_CMD_RTX_TEST:
            // if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0) {
                UploadRTxTest(ConnectedMasterID, MASTER_ID_LEN, StartNum);
                StartNum += 5;
            // }
        break;

        default:
            trxSpiCmdStrobe(CC1120_SFRX);
            ChangeWorkMode(CC1120_WORK_RECEIVING);
        break;
    }    
#endif
#endif
}
void WakeCmdParse(U8 *buffer)
{
    if(buffer[PKT_CMD_OFS] == RF_CMD_SET_SLAVE){
        if(buffer[PKT_DATA_OFS + 1 + SLAVE_ID_LEN] == DATA_CMD_RESET ){
            if(memcmp(ResetCode, &buffer[PKT_DATA_OFS + 2 + SLAVE_ID_LEN], 4) == 0) {
                do{
                Cpu_Reset();
                delay_ms(1);
                //wait reset
                }while(1);
            }
        }
    }
    //error cmd
    // trxSpiCmdStrobe(CC1120_SFRX);
    // ChangeWorkMode(CC1120_WORK_SNIFF);
    SysPowerDown();
}

void RfTxCb(void)
{
    g_PacketCounter.Tx++;
    CC1120_SendTxFifo(TIME_SEND_WAIT_MS);
}

void DataPrepareAndSetTxTimerCb(void)
{
    if( TxCount < FreqHopInfo.FreqHopInfo.SlaveAutoResponseTimes)
    {
        SetTimer1(RF_TX_TIMER_ID, DATA_SAMPLING_AND_PROCESS_COST_TIME, TIMER_MODE_SINGLE, NULL);
        SetTimer1(DATA_PREPARE_AND_SET_TX_TIMER_ID, FreqHopInfo.FreqHopInfo.SlaveSendTimeSliceLen
            * FreqHopInfo.FreqHopInfo.SlaveSumCounter * 10, TIMER_MODE_SINGLE, NULL);
        CommandParse(CC1120_rxBuffer);
        // if(CmdAckEnable == TRUE){
        //     DataSamplingAndProcess();
        // }
        TxCount++;
    }
}

void ConnectCmdParse(U8 *buffer, U8 Len)
{
    U8 *buffptr = buffer;
    U8 RestByte = Len;
    s_ConnectInfo* ConnectInfoPtr;
    RestByte -= PACKET_LENGTH_FIELD_SIZE + PACKET_RX_METRICS_SIZE + PACKET_ADDR_FIELD_SIZE
        + PACKET_CMD_FIELD_SIZE + MASTER_ID_LEN + FREQ_HOP_LIST_LEN + UNIXTIME_LEN;
    ConnectInfoPtr = (s_ConnectInfo *)(buffptr + PACKET_LENGTH_FIELD_SIZE + PACKET_ADDR_FIELD_SIZE
        + PACKET_CMD_FIELD_SIZE + MASTER_ID_LEN + FREQ_HOP_LIST_LEN + UNIXTIME_LEN);
    //Search local addr in connect info list 
    while(RestByte > 0){
        if(memcmp(ConnectInfoPtr->SlaveID, g_static_data.static_data.BMSID, SLAVE_ID_LEN) == 0){
            //find local addr
            g_stateblock_data.stata_union.state_data.ComAddr = ConnectInfoPtr->ComAddr;
            // memcpy((U8 *)&g_stateblock_data.stata_union.state_data.UinxTime, &buffer[PKT_DATA_OFS], MASTER_ID_LEN);
            rfSyncUnixtime(&buffer[PKT_DATA_OFS]);
            memcpy(ConnectedMasterID, &buffer[PKT_MASTER_ID_OFS], MASTER_ID_LEN);
            StopTimer1(RF_SCAN_TIMER_ID);
            StopTimer1(RF_SLEEP_TIMER_ID);
            RfWorkState = RF_NORMAL;
            ConnectUpdateFlag = TRUE;
#ifdef RELAY_ENABLE
            SWITCH_RELAY_LOW_MCU_ON;
            SWITCH_RELAY_HIGH_MCU_ON;
#endif
            break;
        }else{
            //get next connect info
            RestByte -= CONNECT_INFO_LEN;
            ConnectInfoPtr++;
        }
    }
}

void PacketParse(U8 *buffer)
{
    U8 BufferLen = 0;
    BufferLen = buffer[PKT_LENGTH_OFS] + PACKET_LENGTH_FIELD_SIZE + PACKET_RX_METRICS_SIZE;
    U32 TxStartTime = 0;
    //CRC Right
   if(buffer[BufferLen - 1] & 0x80) {
        //legal addr
        if((buffer[PKT_DEST_ADDR_POS] == BROADCAST_ADDR)
            ||(buffer[PKT_DEST_ADDR_POS] == g_stateblock_data.stata_union.state_data.ComAddr)){
            // //Stop RX time out timer
            // StopTimer1(RF_RX_TIMEOUT_TIMER_ID);
            g_PacketCounter.Rx++;
            RfSignalParam.Rssi = buffer[BufferLen - 2];
            RfSignalParam.LQI = (buffer[BufferLen - 1] & 0x7f);
            if(RfWorkState == RF_SLEEP){
                WakeCmdParse(buffer);
            }else{

                if(RfWorkState == RF_SCAN_FREQ){
                    if(buffer[PKT_CMD_OFS] == RF_CMD_CONNECT){
                        ConnectCmdParse(buffer, BufferLen);
                    }
                }

                if((RfWorkState == RF_NORMAL)){

                    if(memcmp(&buffer[PKT_MASTER_ID_OFS], &ConnectedMasterID, MASTER_ID_LEN) == 0){
                        //Stop RX time out timer
                        StopTimer1(RF_RX_TIMEOUT_TIMER_ID);
                        RfDebugMsgEnable = FALSE;
                        //Get Hop Info
                        memcpy(FreqHopInfo.data, &buffer[PKT_FREQ_HOP_OFS], FREQ_HOP_LIST_LEN);
                    #ifdef MOS_ENABLE
                        MosControlProc(FreqHopInfo.FreqHopInfo.MergeBits1.MosEnable);
                    #endif
                        CurrentFreqIndex = FreqHopInfo.FreqHopInfo.MasterNextFreqNomber;
                        CurrentTxPower = FreqHopInfo.FreqHopInfo.MergeBits1.SlaveTxPower;
                        TxStartTime = ((FreqHopInfo.FreqHopInfo.ResponseTimeSlicePos - 
                                        FreqHopInfo.FreqHopInfo.MasterCurrentTimeSlicePos) * 
                                        FreqHopInfo.FreqHopInfo.MasterSendTimeSliceLen +
                                        FreqHopInfo.FreqHopInfo.SlaveSendTimeSliceLen *
                                        (g_stateblock_data.stata_union.state_data.ComAddr - 1))*10;
                        TxCount = 0;
                        SetTimer1(DATA_PREPARE_AND_SET_TX_TIMER_ID, 
                                (TxStartTime - DATA_SAMPLING_AND_PROCESS_COST_TIME), TIMER_MODE_SINGLE, NULL);
                        //Set rx start timer
                        RfSetRxHopOffsetTimer();
                    }else{
                        // invalid master id
                        trxSpiCmdStrobe(CC1120_SFRX);
                        ChangeWorkMode(CC1120_WORK_RECEIVING);
                        // RfSetRxTimeoutTimer();
                    }

                }else{
                    //Connect Cmd not match
                    trxSpiCmdStrobe(CC1120_SFRX);
                    ChangeWorkMode(CC1120_WORK_RECEIVING);
                    // RfSetRxTimeoutTimer();
                }
            }
        } else {
            //addr err
            // g_PacketCounter.AddrErr++;
            // trxSpiCmdStrobe(CC1120_SFRX);
            if(RfWorkState == RF_SLEEP){
                SysPowerDown();
                // ChangeWorkMode(CC1120_WORK_SNIFF);
            }else{
                trxSpiCmdStrobe(CC1120_SFRX);
                ChangeWorkMode(CC1120_WORK_RECEIVING);
                // RfSetRxTimeoutTimer();
            }
        }

    } else {
        //CRC ERROR
        g_PacketCounter.CrcErr++;
//        DisplayBuffer(g_stateblock_data.stata_union.state_data.ComAddr, buffer, BufferLen);
        // trxSpiCmdStrobe(CC1120_SFRX);
        if(RfWorkState == RF_SLEEP){
            SysPowerDown();
            // ChangeWorkMode(CC1120_WORK_SNIFF);
        }else{
            trxSpiCmdStrobe(CC1120_SFRX);
            ChangeWorkMode(CC1120_WORK_RECEIVING);
            // RfSetRxTimeoutTimer();
        }
    }

}


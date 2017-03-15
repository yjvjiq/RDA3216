#include "uartdata_service.h"
#include "debug.h"
#include "w25q64.h"
#include "timer.h"
#include "self_check.h"
#include "rda3216.h"


s_uartrecframe g_uartrecframe = {{0}, 0, MSG_STAT_OUT};
#ifdef VERSION_BTT
s_uartrecframe g_uartrecframebtt = {{0}, 0, MSG_STAT_OUT};
extern void DataSamplingAndProcessSave(s_BatteryTestToolFrameBlock* BatteryTestToolFrameBlock);
#endif
#ifdef CAN_COMMUNICATION
ring_frame g_uartsend_ring = {{0}, 0, 0, FALSE, TRUE};
U8 UartDataPullChar(ring_frame *recring, U8 *data)
{
    if(recring->full == TRUE)return FALSE; //full
    recring->buffer[recring->get] = *data;
    (recring->get == MAX_LEN) ? (recring->get = 0) : (recring->get)++;
    if(recring->put == recring->get) {
        recring->full = TRUE;
    } else {
        recring->full = FALSE;
    }
    recring->empty = FALSE;
    return TRUE;
}

void UartDataPullCharWait(ring_frame *recring, U8 *data)
{
    U8 counter = 0xff;
    while((UartDataPullChar(recring, data) == FALSE) && (counter--)) {
        if((UART0->C2 & UART_C2_TIE_MASK) == 0) {
            UART_EnableTxBuffEmptyInt(UART0);
        }

    }
    if((UART0->C2 & UART_C2_TIE_MASK) == 0) {
        UART_EnableTxBuffEmptyInt(UART0);
    }
}

U8 UartDataPullString(ring_frame *recring, U8 *data , U16 len)
{
    U8 tempdata = 0;

    for(int i = 0 ; i < len; i++) {
        tempdata = data[i];
        UartDataPullCharWait(recring, &tempdata);
    }
    return TRUE;
}


U8 UartDataPushChar(ring_frame *recring, U8 *data)
{
    if(recring->empty == TRUE)return FALSE; //full
    *data = recring->buffer[recring->put] ;
    (recring->put == MAX_LEN) ? (recring->put = 0) : (recring->put)++;
    if(recring->put == recring->get) {
        recring->empty = TRUE;
    } else {
        recring->empty = FALSE;
    }
    recring->full = FALSE;
    return TRUE;
}
#endif

void PushAckString(U8 Cmd, U8 *src, U8 len)
{
    U8 PacketHead[3] = {0xfe,0xfe,0X86};
    U8 CheckSum = 0x86;
    U16 MsgLen = len + CHECKSUM_LEN + CMD_LEN;
    DebugPushString(PacketHead, 3);
    DebugPushString((u8 *)&MsgLen, 2);
//    CheckSumAddString(&CheckSum, (u8 *)&MsgLen, 2);
    DebugPushString(&Cmd, CMD_LEN);
//    CheckSumAddString(&CheckSum, &Cmd, CMD_LEN);
    DebugPushString(src, len);
//    CheckSumAddString(&CheckSum, src, len);
    DebugPushString(&CheckSum, CHECKSUM_LEN);
}


void UartDataParse(s_uartrecframe *recframe)
{
    U16 DataLen = 0;

    memcpy((U8 *)&DataLen, &recframe->buff[3], 2);
    switch(recframe->buff[5]) {
        //change master 4 byte id
        case 0x41:
        DataLen -= CHECKSUM_LEN + CMD_LEN;
        if(DataLen == SLAVE_ID_LEN){
            memcpy(g_static_data.static_data.BMSID, &recframe->buff[6], SLAVE_ID_LEN);
            PushAckString(0x41|0x80, &recframe->buff[6], DataLen);
        }else{
            PushAckString(0x41, &recframe->buff[6], DataLen);
        }
        break;

        //get master 4 byte id
        case 0x42:
            PushAckString(0x42|0x80, g_static_data.static_data.BMSID, SLAVE_ID_LEN);
        break;

        //self check
        // case 0x4d:
            // if(0x01 == recframe->buff[5 + 1]){
            //     FlashSelfCheckProc();
            //     SetTimer1(SELF_CHECK_TIMER_ID, SELF_CHECK_TIMER_MS, TIMER_MODE_PERIODIC, NULL);
            // }else if(0x00 == recframe->buff[5 + 1]){
            //     StopTimer1(SELF_CHECK_TIMER_ID);
            // }
        // break;
         #ifdef PRODUCTION_TEST
            case 0x57:
                scSetCmdHandler(&recframe->buff[5 + 1]);
            break;

            case 0x58:
                scRfTestCmdHandler(&recframe->buff[5 + 1]);
            break;
        #endif

        default:
        break;
    }
    UART_EnableRx(UART0);
}

void SplitUartMsg(U8 *recbuff, U16 len, s_uartrecframe *recframe, void (*handler)(s_uartrecframe *))
{
    static U16 remain_num, len_byte0, len_byte1;


    if (recbuff == NULL || len == 0)
        return;
    for (int i = 0; i < len; i++) {
        switch (recframe->state) {
            case MSG_STAT_OUT:
                if(recbuff[i] == ESCAPE_CHAR) {
                    recframe->len = 0;
                    recframe->buff[recframe->len++] = ESCAPE_CHAR;
                    recframe->state = MSG_STAT_EXPECT_HEAD;
                }
                break;
            case MSG_STAT_EXPECT_HEAD:
                if(recbuff[i] == ESCAPE_HEAD) {
                    recframe->buff[recframe->len++] = ESCAPE_HEAD;
                    recframe->state = MSG_STAT_EXPECT_FRAMEHEAD;
                } else {
                    recframe->state = MSG_STAT_OUT;
                }
                break;
            case MSG_STAT_EXPECT_FRAMEHEAD:
                if(recbuff[i] == FRAME_HEAD) {
                    recframe->buff[recframe->len++] = recbuff[i];
                    recframe->state = MSG_STAT_EXPECT_LEN_BYTE0;
                } else {
                    recframe->state = MSG_STAT_OUT;
                }
                break;
            case MSG_STAT_EXPECT_LEN_BYTE0:
                recframe->buff[recframe->len++] = recbuff[i];
                recframe->state = MSG_STAT_EXPECT_LEN_BYTE1;
                break;
            case MSG_STAT_EXPECT_LEN_BYTE1:
                recframe->buff[recframe->len++] = recbuff[i];
                len_byte0 =  recframe->buff[recframe->len - 2];
                len_byte1 =  recframe->buff[recframe->len - 1];
                remain_num = (len_byte1 << 8) + len_byte0;
                recframe->state = MSG_STAT_IN;
                break;
            case MSG_STAT_IN:
                recframe->buff[recframe->len++] = recbuff[i];
                remain_num--;
                if(remain_num == 0) {
//                    if(packcheack(recframe->buff + 2, recframe->len - 3, recframe->buff[recframe->len - 1]) == TRUE) {
//                        recframe->state = MSG_STAT_OUT;
//                        // (*handler)(recframe);
//                        Set_IntFlag(UART0_RXFINISH_POS);
//                        UART_DisableRx(UART0);
//                    } else {
//                        recframe->state = MSG_STAT_OUT;
//                    }
                }
                break;
            case MSG_STAT_ESCAPE:
                break;
            default:
                break;
        }
    }
}

#ifdef VERSION_BTT
#ifdef BTT_RF
void ExchangeIdFormat(s_uartrecframe *recframe, s_BatteryTestToolFrameBlock *BatteryTestToolFrameBlock)
{
    // for (U8 i = recframe->len; i < BATTERTY_ID_LEN_MAX; i++){
    //     recframe->buff[i] = 0;
    // }
    U8 recframe_len = recframe->len;
    for(U8 i = 0; i < BATTERTY_ID_LEN_MAX; i++){
        // BatteryTestToolFrameBlock->BatteryTestToolFrame.BatteryID[i] = recframe->buff[BATTERTY_ID_LEN_MAX - 1 - i];
        if(recframe_len == 0){
            BatteryTestToolFrameBlock->BatteryTestToolFrame.BatteryID[i] = '0';
        }else{
            BatteryTestToolFrameBlock->BatteryTestToolFrame.BatteryID[i] = recframe->buff[recframe_len - 1];
            recframe_len--;
        }
    }
}

U8 CheackRecData(U8 data)
{
    if( ((data >= 'a') && (data <= 'z')) ||
        ((data >= 'A') && (data <= 'Z')) ||
        ((data >= '0') && (data <= '9')) ||
        (data == '\t') ||
        (data == '\r') ||
        (data == '\n') ||
	    (data == '-')){
        return TRUE;
    }else{
        return FALSE;
    }
}

U8 CheackRecID(U8 *src, U8 len)
{
    for(U8 index = 0; index < len; index++){
        if(CheackRecData(src[index]) == FALSE){
            return FALSE;
        }
    }
    return TRUE;
}

void UartDataParseBtt(s_uartrecframe *recframe)
{
    s_BatteryTestToolFrameBlock TempBttBlock;
    TempBttBlock.FlashBlockHead = 0xff;
    if((recframe->len >= BATTERTY_ID_LEN_MIN) && (recframe->len <= BATTERTY_ID_LEN_MAX)){
        // memcpy(TempBttBlock.BatteryTestToolFrame.BatteryID, recframe->buff, BATTERTY_ID_LEN_MAX);
        if(CheackRecID(recframe->buff, recframe->len) == TRUE){
            ExchangeIdFormat(recframe, &TempBttBlock);
            DataSamplingAndProcessSave(&TempBttBlock);
        }
    }

    recframe->len = 0;
    UART_EnableRx(UART0);
}

void RecUartMsg(U8 *recdata, s_uartrecframe *recframe)
{
    recframe->buff[recframe->len++] = *recdata;
    SetTimer1(UART_REC_TIMEOUT_ID, UART_REC_TIMEOUT_MS, TIMER_MODE_SINGLE, NULL);
}
#endif

#ifdef BTT_OFFLINE
void SplitUartBttMsg(U8 *recbuff, U16 len, s_uartrecframe *recframe, void (*handler)(s_uartrecframe *))
{
    static U16 remain_num = 0;;

    if (recbuff == NULL || len == 0)
        return;
    for (int i = 0; i < len; i++) {
        switch (recframe->state) {
            case MSG_STAT_OUT:
                if(recbuff[i] == ESCAPE_BYTE0) {
                    recframe->len = 0;
                    recframe->buff[recframe->len++] = ESCAPE_BYTE0;
                    recframe->state = MSG_STAT_EXPECT_BYTE1;
                }
                break;
            case MSG_STAT_EXPECT_BYTE1:
                if(recbuff[i] == ESCAPE_BYTE1) {
                    recframe->buff[recframe->len++] = ESCAPE_BYTE1;
                    recframe->state = MSG_STAT_EXPECT_BYTE2;
                } else {
                    recframe->state = MSG_STAT_OUT;
                }
                break;
            case MSG_STAT_EXPECT_BYTE2:
                if(recbuff[i] == ESCAPE_BYTE2) {
                    recframe->buff[recframe->len++] = ESCAPE_BYTE2;
                    recframe->state = MSG_STAT_EXPECT_BYTE3;
                } else {
                    recframe->state = MSG_STAT_OUT;
                }
                break;
            case MSG_STAT_EXPECT_BYTE3:
                if(recbuff[i] == ESCAPE_BYTE3) {
                    recframe->buff[recframe->len++] = ESCAPE_BYTE3;
                    recframe->state = MSG_STAT_EXPECT_BYTE4;
                } else {
                    recframe->state = MSG_STAT_OUT;
                }
                break;
            case MSG_STAT_EXPECT_BYTE4:
                if(recbuff[i] == ESCAPE_BYTE4) {
                    recframe->buff[recframe->len++] = ESCAPE_BYTE4;
                    recframe->state = MSG_STAT_EXPECT_BYTE5;
                } else {
                    recframe->state = MSG_STAT_OUT;
                }
                break;
            case MSG_STAT_EXPECT_BYTE5:
                if(recbuff[i] == ESCAPE_BYTE5) {
                    recframe->buff[recframe->len++] = ESCAPE_BYTE5;
                    recframe->state = MSG_STAT_EXPECT_BYTE6;
                } else {
                    recframe->state = MSG_STAT_OUT;
                }
                break;
            case MSG_STAT_EXPECT_BYTE6:
                if(recbuff[i] == ESCAPE_BYTE6) {
                    recframe->buff[recframe->len++] = ESCAPE_BYTE6;
                    recframe->state = MSG_STAT_EXPECT_BYTE7;
                } else {
                    recframe->state = MSG_STAT_OUT;
                }
                break;
            case MSG_STAT_EXPECT_BYTE7:
                if(recbuff[i] == ESCAPE_BYTE7) {
                    recframe->buff[recframe->len++] = ESCAPE_BYTE7;
                    recframe->state = MSG_STAT_IN;
                    remain_num = BATTERTY_ID_LEN;
                } else {
                    recframe->state = MSG_STAT_OUT;
                }
                break;

            case MSG_STAT_IN:
                recframe->buff[recframe->len++] = recbuff[i];
                remain_num--;
                if(remain_num == 0) {
                    recframe->state = MSG_STAT_OUT;
                    // (*handler)(recframe);
                    Set_IntFlag(UART0_RXFINISH_BTT_POS);
                    UART_DisableRx(UART0);
                }
            break;

            default:
                break;
        }
    }
}

void DataSamplingAndReply(s_BatteryTestToolFrame* BatteryTestToolFrame)
{
    DataSamplingAndProcess();
    // InitMetering();
    memcpy((U8 *)&BatteryTestToolFrame->CellVoltage, (U8 *)&g_battery_pack_info.sample_data.cell_voltages,
     (CELL_VOLTAGE_CHANNEL * 2));
    BatteryTestToolFrame->PackVoltage = g_log_data.log_data.PackVoltageValue;
    BatteryTestToolFrame->SOC = g_battery_pack_info.SOC;
    BatteryTestToolFrame->Cell_T[0] = g_log_data.log_data.Cell_T[0];
    BatteryTestToolFrame->Cell_T[1] = g_log_data.log_data.Cell_T[1];
    memcpy((U8 *)&BatteryTestToolFrame->AlarmFlag, g_log_data.log_data.AlarmFlag, 5);
    BatteryTestToolFrame->UpdataFlag = TRUE;
    UpLoadBttData();
}

void UartDataParseBtt(s_uartrecframe *recframe)
{
    U8 OfflineBatteryID[BATTERTY_ID_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    if(memcmp(OfflineBatteryID, &recframe->buff[8], BATTERTY_ID_LEN) == 0){
        memcpy(g_BatteryTestToolFrame.BatteryID, OfflineBatteryID, BATTERTY_ID_LEN);
        DataSamplingAndReply(&g_BatteryTestToolFrame);
    }
    UART_EnableRx(UART0);
}
#endif
#endif

U8 CmpRcvAndCmd(U8* data1,U8* data2,U8 len){

    return !memcmp(data1, data2, len);
}
typedef struct
{
  char message[16];
  void (*pHandler)(U8* para);
}xUARTDBGFunctionHandler;

void DevNumIdentifyHandler(U8* dev_num){
	RDA_DeviceNumReset(*dev_num);
}

void DevModSetHandler(U8* mode){
	RDA_DeviceModReset(*mode);
}

xUARTDBGFunctionHandler UARTDBGCMD[] = {
	{":1dev!", DevNumIdentifyHandler},
	{":2dev!", DevNumIdentifyHandler},
	{":3dev!", DevNumIdentifyHandler},
	{":4dev!", DevNumIdentifyHandler},
	{":5dev!", DevNumIdentifyHandler},
	{":6dev!", DevNumIdentifyHandler},
	{":7dev!", DevNumIdentifyHandler},

	{":0mod!", DevModSetHandler},
	{":1mod!", DevModSetHandler},
	{":2mod!", DevModSetHandler},
	{":3mod!", DevModSetHandler},
};

void UartCmdHandler(UART_Type *pUART){
	U8 UartRcvBuf[16]={0};
	U8 RcvBufLen = 0;
	U8 Cmd_len = sizeof(UARTDBGCMD)/sizeof(xUARTDBGFunctionHandler);
	while('!' != UartRcvBuf[RcvBufLen]){
		UartRcvBuf[RcvBufLen++] = UART_ReadDataReg(UART0);
	}
	
	for (U8 i=0; i<Cmd_len; i++){
		if (CmpRcvAndCmd(UartRcvBuf,(U8*)UARTDBGCMD[i].message,RcvBufLen)){
			UARTDBGCMD[i].pHandler(&UartRcvBuf[1]);
		}
	}
}




#include "can_service.h"
#include "drivers.h"
#include "w25q64.h"
#include "drivers.h"
#include "timer.h"
#include "public.h"

#ifdef CAN_COMMUNICATION
LDD_CAN_TFrame g_can_sendbuff;
s_CanSendQueue g_CanSendQueue = {{0}, 0, 0, FALSE, TRUE};

U8 g_flashdatablock[W25Q64_DATA_BLOCKSIZE] = {0};
s_canmsg g_canmsg = {0, 0, STOP};
s_popstat g_popstate = {0, STOP};

s_boardcast g_boardmsg01 = {BROADCAST_MSG01_ID, 0, 0, 0x01, 0, 0, FALSE};
s_boardcast g_boardmsg02 = {BROADCAST_MSG02_ID, 0, 0, 0x02, 0, 0, FALSE};
s_boardcast g_boardmsg03 = {BROADCAST_MSG03_ID, 0, 0, 0x03, 0, 0, FALSE};

//s_boardcast g_canackmsg01 = {CANACK_MSG01_ID, 0, 0, 0x01, 0, 0, FALSE};
//s_boardcast g_canackmsg02 = {CANACK_MSG02_ID, 0, 0, 0x02, 0, 0, FALSE};
//s_boardcast g_canackmsg03 = {CANACK_MSG03_ID, 0, 0, 0x03, 0, 0, FALSE};
//s_boardcast g_canackmsg04 = {CANACK_MSG04_ID, 0, 0, 0x04, 0, 0, FALSE};
//s_boardcast g_canackmsg05 = {CANACK_MSG05_ID, 0, 0, 0x05, 0, 0, FALSE};
//s_boardcast g_canackmsg06 = {CANACK_MSG06_ID, 0, 0, 0x06, 0, 0, FALSE};
//s_boardcast g_canackmsg07 = {CANACK_MSG07_ID, 0, 0, 0x07, 0, 0, FALSE};
//s_boardcast g_canackmsg08 = {CANACK_MSG08_ID, 0, 0, 0x08, 0, 0, FALSE};

s_boardcast g_canackmsg[9] = {
    {CANACK_MSG01_ID, 0, 0, 0x01, 0, 0, FALSE},
    {CANACK_MSG02_ID, 0, 0, 0x02, 0, 0, FALSE},

    {CANACK_MSG04_ID, 0, 0, 0x04, 0, 0, FALSE},
    {CANACK_MSG05_ID, 0, 0, 0x05, 0, 0, FALSE},
    {CANACK_MSG06_ID, 0, 0, 0x06, 0, 0, FALSE},
    {CANACK_MSG07_ID, 0, 0, 0x07, 0, 0, FALSE},
    {CANACK_MSG08_ID, 0, 0, 0x08, 0, 0, FALSE},
    {CANACK_MSG09_ID, 0, 0, 0x09, 0, 0, FALSE},
    {CANACK_MSG0A_ID, 0, 0, 0x0a, 0, 0, FALSE},
};

LDD_CAN_TFrame g_can_recframe;
U8 g_can_recdata[8];

s_ComAddrStatus g_ComAddrStatus = {0, 0};

U32 GetU32From4Bytes(U8 *buff)
{
    u_varform_32u8 formchanger_32u8;
//    formchanger_32u8.varu8[3] = buff[0];
//    formchanger_32u8.varu8[2] = buff[1];
//    formchanger_32u8.varu8[1] = buff[2];
//    formchanger_32u8.varu8[0] = buff[3];
    memcpy(formchanger_32u8.varu8, buff, 4);
    return formchanger_32u8.varu32;
}

void SetU16ToBuffer(U16 data, U8 *buff)
{
    buff[0] = (U8)(data >> 8);
    buff[1] = (U8)(data);
}

void SetU8ToBuffer(U8 data, U8 *buff)
{
    buff[0] = (U8)(data);
}


void InitCanSendBuff(LDD_CAN_TFrame *srcbuff, LDD_CAN_TFrame *sendbuff)
{
    u_varform_32u8 formchanger_32u8;
    U8 src_addr = 0;
    formchanger_32u8.varu32 = srcbuff->MessageID;
    src_addr = formchanger_32u8.varu8[1];
    formchanger_32u8.varu8[3] = 0x12;
    formchanger_32u8.varu8[2] = 0;
    formchanger_32u8.varu8[1] = 0;
    formchanger_32u8.varu8[0] = src_addr;
    sendbuff->MessageID = formchanger_32u8.varu32;
    sendbuff->MessageID |= LDD_CAN_MESSAGE_ID_EXT;
    sendbuff->FrameType = LDD_CAN_DATA_FRAME;
    sendbuff->Length = 8;
    sendbuff->LocPriority = 0;
}

//start a new datablcok transimit
void InitCanMsg(s_canmsg *canmsg, U8 *buff_ptr)
{
    canmsg->buffer = buff_ptr;
    canmsg->remain_msg = 0;
    canmsg->state = RUNNING;
}

U8 WriteCanQueue(s_CanSendQueue *queue, MSCAN_RegisterFrameType *canframe)
{
    if(queue->full == TRUE) return FALSE;
    memcpy(&queue->buff[queue->head], canframe, sizeof(MSCAN_RegisterFrameType));
    (queue->head == CAN_SENDBUFF_CAP) ? (queue->head = 0) : (queue->head++);
    queue->empty = FALSE;
    if(queue->head == queue->tail)
        queue->full = TRUE;
    else queue ->full = FALSE;
    CAN_TransmitterEmptyIntEn(MSCAN);
    return TRUE;
}

U8 ReadCanQueue(s_CanSendQueue *queue, MSCAN_RegisterFrameType *canframe)
{
    if(queue->empty == TRUE) return FALSE;
    memcpy(canframe, &queue->buff[queue->tail], sizeof(MSCAN_RegisterFrameType));
    (queue->tail == CAN_SENDBUFF_CAP) ? (queue->tail = 0) : (queue->tail++);
    queue->full = FALSE;
    if(queue->head == queue->tail)
        queue->empty = TRUE;
    else queue ->empty = FALSE;
    return TRUE;

}

void StopBlockSend(s_canmsg *canmsg)
{
    canmsg->remain_msg = CAM_MSG_MAX_NUM;
    canmsg->send_num = 0;
    canmsg->state = STOP;
}

void StopPopOut(s_popstat *popstate)
{
    popstate->remain_num = 0;
    popstate->state = STOP;
}

void StopPopAndTransimit(void)
{
    StopBlockSend(&g_canmsg);
    StopPopOut(&g_popstate);
    SendStopMSG(&g_can_recframe, &g_can_sendbuff);
}

void SendStopMSG(LDD_CAN_TFrame *srcbuff, LDD_CAN_TFrame *sendbuff)
{
    u_varform_32u8 formchanger_32u8;
    U8 src_addr = 0;
    U8 buffs[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    formchanger_32u8.varu32 = srcbuff->MessageID;
    src_addr = formchanger_32u8.varu8[1];
    formchanger_32u8.varu8[3] = 0x12;
    formchanger_32u8.varu8[2] = 0xff;
    formchanger_32u8.varu8[1] = 0xff;
    formchanger_32u8.varu8[0] = src_addr;
    sendbuff->MessageID = formchanger_32u8.varu32;
    sendbuff->MessageID |= LDD_CAN_MESSAGE_ID_EXT;
    sendbuff->FrameType = LDD_CAN_DATA_FRAME;
    sendbuff->Length = 8;
    sendbuff->LocPriority = 0;
    sendbuff->Data = buffs;
    U16 counter = 0xffff;
    while((CAN_SendFrame(sendbuff) != ERR_OK) && (counter--));
}

//one datablock send cycle ,send a datablock by 4 can tx interrupt
U8 FlashDataBlockSend(s_canmsg *canmsg, LDD_CAN_TFrame *sendbuff)
{
    u_varform_32u8 formchanger_32u8;
    if(canmsg->state != RUNNING ) {
        return FALSE;
    } else {
        if(canmsg->remain_msg < CAM_MSG_MAX_NUM) {
            sendbuff->Data = &canmsg->buffer[(canmsg->remain_msg) * 8];
            canmsg->remain_msg++;
            if(canmsg->remain_msg >= CAM_MSG_MAX_NUM) {
                canmsg->state = FINISH;
            } else {
                canmsg->state = RUNNING;
            }
            formchanger_32u8.varu32 = sendbuff->MessageID;
            formchanger_32u8.varu8[2] = (U8)(canmsg->send_num >> 8);
            formchanger_32u8.varu8[1] = (U8)canmsg->send_num;
            sendbuff->MessageID = formchanger_32u8.varu32;
            U16 counter = 0xffff;
            while((CAN_SendFrame(sendbuff) != ERR_OK) && (counter--));
            canmsg->send_num++;
//            Uart0_SendBlock(&canmsg->buffer[(canmsg->remain_msg - 1) * 8], 8);
            return TRUE;
        }
        return FALSE;
    }
}

//Get datablocks from flash by number
U8 FlashPopOut(s_popstat *popstate , flashdata_frame *dataframe, U8 *buff_ptr)
{
    if(popstate->state != RUNNING) {
        return FALSE;
    } else {
        if(W25Q64_ReadOneDataBlock(dataframe, buff_ptr) == TRUE) {
            printf("ReadBlock:head:0x%x,tail:0x%x,empty:0x%x,full:0x%x\n", dataframe->head, dataframe->tail + W25Q64_DATA_BLOCKSIZE, dataframe->empty, dataframe->full);
            popstate->remain_num--;
            if(popstate->remain_num <= 0) {
                popstate->state = FINISH;
            } else {
                popstate->state = RUNNING;
            }
            return TRUE;
        } else {
            printf("DataBlockEmpty!\n");
            popstate->state = STOP;
            return FALSE;
        }
    }
}

void FlashPopAndSendProc(void)
{
    if(FlashPopOut(&g_popstate, &g_flashdata_frame, g_flashdatablock) == TRUE) {
        InitCanMsg(&g_canmsg, g_flashdatablock);
        while(g_canmsg.state == RUNNING) {
            FlashDataBlockSend(&g_canmsg, &g_can_sendbuff);
        }
    } else {
        StopPopAndTransimit();
    }

}

void Init_AckMSG(s_boardcast *boardmsg, LDD_CAN_TFrame *canframe)
{
    u_varform_32u8 formchanger_32u8;
    formchanger_32u8.varu32 = canframe->MessageID;
    boardmsg->dest_addr = formchanger_32u8.varu8[1];
//    boardmsg->src_addr = formchanger_32u8.varu8[0];
    boardmsg->src_addr = g_stateblock_data.stata_union.state_data.ComAddr;
    boardmsg->cycle = canframe->Data[2];   //5ms /bit
    boardmsg->times = canframe->Data[1];
    boardmsg->state = TRUE;
    if((boardmsg->times) > 0) {
        SetTimer1(boardmsg->timerID, boardmsg->cycle, TIMER_MODE_PERIODIC, NULL);
    } else {
        boardmsg->state = FALSE;
        StopTimer1(boardmsg->timerID);
    }
}

void AckMSG(FlashStatic_Data_Union *staticdata, Battery_Pack_Info *packinfo, s_boardcast *boardmsg)
{
    U8 msg_buff[8] = {0};
    U16 counter = 0xffff;
    u_varform_32u8 formchanger_32u8;
    LDD_CAN_TFrame ack_canframe;
    if(boardmsg->times <= 0) {
        boardmsg->state = FALSE;
        StopTimer1(boardmsg->timerID);
    } else {
        formchanger_32u8.varu8[3] = 0x1a;
        formchanger_32u8.varu8[2] = boardmsg->code | 0xA0;
        formchanger_32u8.varu8[1] = boardmsg->src_addr;
        formchanger_32u8.varu8[0] = boardmsg->dest_addr;
        ack_canframe.MessageID = formchanger_32u8.varu32;
        ack_canframe.MessageID |= LDD_CAN_MESSAGE_ID_EXT;
        ack_canframe.FrameType = LDD_CAN_DATA_FRAME;
        ack_canframe.LocPriority = 1;
        ack_canframe.Length = 8;
        ack_canframe.Data = msg_buff;
        boardmsg->times--;
        switch(boardmsg->timerID) {
            case CANACK_MSG01_ID:
                memcpy(&(msg_buff[BMSID_POS]), &(staticdata->data[BMSID_HIGH_STATICPOS]), 6);
                counter = 0xffff;
                while((CAN_SendFrame(&ack_canframe) != ERR_OK) && (counter--));
                break;

            case CANACK_MSG02_ID:
                memcpy(&(msg_buff[BMSID_POS]), &(staticdata->data[BMSID_LOW_STATICPOS]), 6);
                counter = 0xffff;
                while((CAN_SendFrame(&ack_canframe) != ERR_OK) && (counter--));
                break;

            case CANACK_MSG04_ID:
                memcpy(&(msg_buff[MaxPermitsVoltage_POS]), &(staticdata->data[MaxPermitsVoltage_STATICPOS]), 2);
                memcpy(&(msg_buff[MaxPermitsCurrent_POS]), &(staticdata->data[MaxPermitsCurrent_STATICPOS]), 2);
                counter = 0xffff;
                while((CAN_SendFrame(&ack_canframe) != ERR_OK) && (counter--));
                break;

            case CANACK_MSG05_ID:
                memcpy(msg_buff, &packinfo->sample_data.cell_voltages[0], 8);
                counter = 0xffff;
                while((CAN_SendFrame(&ack_canframe) != ERR_OK) && (counter--));
                break;

            case CANACK_MSG06_ID:
                memcpy(msg_buff, &packinfo->sample_data.cell_voltages[4], 8);
                counter = 0xffff;
                while((CAN_SendFrame(&ack_canframe) != ERR_OK) && (counter--));
                break;

            case CANACK_MSG07_ID:
                memcpy(msg_buff, &packinfo->sample_data.cell_voltages[8], 8);
                counter = 0xffff;
                while((CAN_SendFrame(&ack_canframe) != ERR_OK) && (counter--));
                break;

            case CANACK_MSG08_ID:
                memcpy(msg_buff, &packinfo->sample_data.cell_voltages[12], 8);
                counter = 0xffff;
                while((CAN_SendFrame(&ack_canframe) != ERR_OK) && (counter--));
                break;

            case CANACK_MSG09_ID:
                memcpy(msg_buff, &packinfo->sample_data.cell_voltages[16], 8);
                counter = 0xffff;
                while((CAN_SendFrame(&ack_canframe) != ERR_OK) && (counter--));
                break;

            case CANACK_MSG0A_ID:
                memcpy(msg_buff, &packinfo->sample_data.cell_voltages[20], 8);
                counter = 0xffff;
                while((CAN_SendFrame(&ack_canframe) != ERR_OK) && (counter--));
                break;



            default:
                break;
        }
    }
}

void Init_BoardCastMSG(s_boardcast *boardmsg, LDD_CAN_TFrame *canframe)
{
    u_varform_32u8 formchanger_32u8;
    formchanger_32u8.varu32 = canframe->MessageID;
    boardmsg->dest_addr = formchanger_32u8.varu8[1];
//    boardmsg->src_addr = formchanger_32u8.varu8[0];
    boardmsg->src_addr = g_stateblock_data.stata_union.state_data.ComAddr;
    boardmsg->cycle = canframe->Data[1] * 5;   //5ms /bit
    boardmsg->state = TRUE;
    SetTimer1(boardmsg->timerID, boardmsg->cycle, TIMER_MODE_PERIODIC, NULL);
}

void BoardCastMSG(Pack_Data_Struct *logdata, s_boardcast *boardmsg)
{
    U8 msg_buff[8] = {0};
    U16 counter = 0xffff;
    u_varform_32u8 formchanger_32u8;
    LDD_CAN_TFrame boardcast_canframe;
    formchanger_32u8.varu8[3] = 0x1a;
    formchanger_32u8.varu8[2] = boardmsg->code | 0xc0;
    formchanger_32u8.varu8[1] = boardmsg->src_addr;
//    formchanger_32u8.varu8[1] = g_stateblock_data.stata_union.state_data.ComAddr;
    formchanger_32u8.varu8[0] = boardmsg->dest_addr;
    boardcast_canframe.MessageID = formchanger_32u8.varu32;
    boardcast_canframe.MessageID |= LDD_CAN_MESSAGE_ID_EXT;
    boardcast_canframe.FrameType = LDD_CAN_DATA_FRAME;
    boardcast_canframe.LocPriority = 1;
    boardcast_canframe.Length = 8;
    boardcast_canframe.Data = msg_buff;
    switch(boardmsg->timerID) {
        case BROADCAST_MSG01_ID:
            memcpy(&(msg_buff[PackVoltageValue_POS]), &(logdata->data[PackVoltageValue_LOGPOS]), 2);
            memcpy(&(msg_buff[NowCurrent_POS]), &(logdata->data[NowCurrent_LOGPOS]), 2);
            memcpy(&(msg_buff[NowSOC_POS]), &(logdata->data[NowSOC_LOGPOS]), 1);
            memcpy(&(msg_buff[PackMeanTemprature_POS]), &(logdata->data[PackMeanTemprature_LOGPOS]), 1);
            counter = 0xffff;
            while((CAN_SendFrame(&boardcast_canframe) != ERR_OK) && (counter--));
            break;

        case BROADCAST_MSG02_ID:
            memcpy(&(msg_buff[CellMaxVoltage_POS]), &(logdata->data[CellMaxVoltage_LOGPOS]), 2);
            memcpy(&(msg_buff[CellMaxVoltageNumber_POS]), &(logdata->data[CellMaxVoltageNumber_LOGPOS]), 1);
            memcpy(&(msg_buff[CellMinVoltage_POS]), &(logdata->data[CellMinVoltage_LOGPOS]), 2);
            memcpy(&(msg_buff[CellMinVoltageNumber_POS]), &(logdata->data[CellMinVoltageNumber_LOGPOS]), 1);
            memcpy(&(msg_buff[CellMeanVoltageValue_POS]), &(logdata->data[CellMeanVoltageValue_LOGPOS]), 2);
            counter = 0xffff;
            while((CAN_SendFrame(&boardcast_canframe) != ERR_OK) && (counter--));
            break;

        case BROADCAST_MSG03_ID:
            memcpy(&(msg_buff[Cell_T_1_POS]), &(logdata->data[Cell_T_1_LOGPOS]), 1);
            memcpy(&(msg_buff[Cell_T_2_POS]), &(logdata->data[Cell_T_2_LOGPOS]), 1);
            memcpy(&(msg_buff[AlarmFlag_POS]), &(logdata->data[AlarmFlag_LOGPOS]), 5);
            counter = 0xffff;
            while((CAN_SendFrame(&boardcast_canframe) != ERR_OK) && (counter--));
            break;

        default:
            break;
    }
}

void CanCmdParsingProc(LDD_CAN_TFrame *canframe)
{
    u_varform_32u8 formchanger_32u8;
    U8 cmd = 0, dest_addr = 0, src_addr = 0, datacmd = 0;
    const U8 c_Zeros[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    const U8 c_KeyWords[4] = {0xaa, 0xbb, 0xcc, 0xdd};
    U16 counter = 0xffff;
    formchanger_32u8.varu32 = canframe->MessageID;
    cmd = formchanger_32u8.varu8[2];
    src_addr = formchanger_32u8.varu8[1];
    dest_addr = formchanger_32u8.varu8[0];
    datacmd = canframe->Data[0];

    if((dest_addr == g_stateblock_data.stata_union.state_data.ComAddr) || (dest_addr == 0xff)) {
        switch(cmd) {
            case 0x01:
                g_popstate.remain_num = BANK_SIZE;
                g_popstate.state = RUNNING;
                g_flashdata_frame.tail = g_flashdata_frame.head;
                g_flashdata_frame.tail = W25Q64_FindSelectedDataAddr(GetU32From4Bytes(canframe->Data) * BANK_SIZE, g_flashdata_frame.tail);
                g_flashdata_frame.empty = FALSE;
                g_canmsg.send_num = 0;
                InitCanSendBuff(canframe, &g_can_sendbuff);
                FlashPopAndSendProc();
                break;

            case 0x10:
                g_popstate.remain_num = 1;
                g_popstate.state = RUNNING;
                g_flashdata_frame.tail = g_flashdata_frame.head;
                g_flashdata_frame.tail = W25Q64_FindSelectedDataAddr(GetU32From4Bytes(canframe->Data) * BANK_SIZE, g_flashdata_frame.tail);
                g_flashdata_frame.tail = W25Q64_FindSelectedDataAddr(GetU32From4Bytes(&(canframe->Data[4])), g_flashdata_frame.tail);
                g_flashdata_frame.empty = FALSE;
                g_canmsg.send_num = 0;
                InitCanSendBuff(canframe, &g_can_sendbuff);
                FlashPopAndSendProc();
                break;

            case 0x11:
                g_popstate.remain_num = GetU32From4Bytes(canframe->Data);
                g_popstate.state = RUNNING;
                g_flashdata_frame.tail = g_flashdata_frame.head;
                g_flashdata_frame.empty = FALSE;
                g_canmsg.send_num = 0;
                InitCanSendBuff(canframe, &g_can_sendbuff);
                FlashPopAndSendProc();
                break;

            case 0xff:  //stop transimit
                if(memcmp(canframe->Data, c_Zeros, 8) == 0) {
                    StopPopAndTransimit();
                }
                break;


            case 0x20:
                switch(datacmd) {
                    case 0x01:
                        Init_AckMSG(&g_canackmsg[0], canframe);
                        break;
                    case 0x02:
                        Init_AckMSG(&g_canackmsg[1], canframe);
                        break;
                    case 0x03:
                        Init_AckMSG(&g_canackmsg[0], canframe);
                        Init_AckMSG(&g_canackmsg[1], canframe);
                        break;
                    case 0x04:
                        Init_AckMSG(&g_canackmsg[2], canframe);
                        break;
                    case 0x05:
                        Init_AckMSG(&g_canackmsg[3], canframe);
                        break;
                    case 0x06:
                        Init_AckMSG(&g_canackmsg[4], canframe);
                        break;
                    case 0x07:
                        Init_AckMSG(&g_canackmsg[5], canframe);
                        break;
                    case 0x08:
                        Init_AckMSG(&g_canackmsg[6], canframe);
                        break;
                    case 0x09:
                        Init_AckMSG(&g_canackmsg[7], canframe);
                        break;
                    case 0x0a:
                        Init_AckMSG(&g_canackmsg[8], canframe);
                        break;
                    case 0x0f:
                        Init_AckMSG(&g_canackmsg[3], canframe);
                        Init_AckMSG(&g_canackmsg[4], canframe);
                        Init_AckMSG(&g_canackmsg[5], canframe);
                        Init_AckMSG(&g_canackmsg[6], canframe);
                        Init_AckMSG(&g_canackmsg[7], canframe);
                        Init_AckMSG(&g_canackmsg[8], canframe);
                        break;

                    default:
                        break;
                }
                break;

            case 0x40:
                switch(datacmd) {
                    case 0x01:
                        Init_BoardCastMSG(&g_boardmsg01, canframe);
                        break;
                    case 0x02:
                        Init_BoardCastMSG(&g_boardmsg02, canframe);
                        break;
                    case 0x03:
                        Init_BoardCastMSG(&g_boardmsg03, canframe);
                        break;
                    case 0x0f:
                        Init_BoardCastMSG(&g_boardmsg01, canframe);
                        Init_BoardCastMSG(&g_boardmsg02, canframe);
                        Init_BoardCastMSG(&g_boardmsg03, canframe);
                        break;
                    default:
                        break;
                }
                break;

            case 0x60:
                switch(datacmd) {
                    case 0x01:
                        g_log_data.log_data.UinxTime = GetU32From4Bytes(&canframe->Data[1]);
                        formchanger_32u8.varu8[0] = src_addr;
                        formchanger_32u8.varu8[1] = dest_addr;
                        formchanger_32u8.varu8[2] = datacmd | 0xE0;
                        formchanger_32u8.varu8[3] = 0x1a;
                        canframe->MessageID = formchanger_32u8.varu32;
                        canframe->MessageID |= LDD_CAN_MESSAGE_ID_EXT;
                        counter = 0xffff;
                        while((CAN_SendFrame(canframe) != ERR_OK) && (counter--));
                        break;

                    case 0x02:
                        if(memcmp((g_static_data.static_data.BMSID + BMSID_HIGH_POS), (canframe->Data + 2), 6) == 0) {
                            Set_PosFlag(g_ComAddrStatus.Flag, BMSID_HIGH_SHIFT);
                            if((g_ComAddrStatus.Flag == BMSID_FULL_MASK) && (g_ComAddrStatus.ComAddr == canframe->Data[1])) {
                                g_stateblock_data.stata_union.state_data.ComAddr = g_ComAddrStatus.ComAddr;
                                g_ComAddrStatus.Flag = BMSID_EMPTY_MASK;
                                W25Q64_SetStateMSG(&g_stateblock_data, &g_flashdata_frame);
                            }
                            g_ComAddrStatus.ComAddr = canframe->Data[1];
                            //ACK
                            formchanger_32u8.varu8[0] = src_addr;
                            formchanger_32u8.varu8[1] = g_stateblock_data.stata_union.state_data.ComAddr;
                            formchanger_32u8.varu8[2] = datacmd | 0xE0;
                            formchanger_32u8.varu8[3] = 0x1a;
                            canframe->MessageID = formchanger_32u8.varu32;
                            canframe->MessageID |= LDD_CAN_MESSAGE_ID_EXT;
                            counter = 0xffff;
                            while((CAN_SendFrame(canframe) != ERR_OK) && (counter--));
                        }
                        break;

                    case 0x03:
                        if(memcmp((g_static_data.static_data.BMSID + BMSID_LOW_POS), (canframe->Data + 2), 6) == 0) {
                            Set_PosFlag(g_ComAddrStatus.Flag, BMSID_LOW_SHIFT);
                            if((g_ComAddrStatus.Flag == BMSID_FULL_MASK) && (g_ComAddrStatus.ComAddr == canframe->Data[1])) {
                                g_stateblock_data.stata_union.state_data.ComAddr = g_ComAddrStatus.ComAddr;
                                g_ComAddrStatus.Flag = BMSID_EMPTY_MASK;
                                W25Q64_SetStateMSG(&g_stateblock_data, &g_flashdata_frame);
                            }
                            g_ComAddrStatus.ComAddr = canframe->Data[1];
                            //ACK
                            formchanger_32u8.varu8[0] = src_addr;
                            formchanger_32u8.varu8[1] = g_stateblock_data.stata_union.state_data.ComAddr;
                            formchanger_32u8.varu8[2] = datacmd | 0xE0;
                            formchanger_32u8.varu8[3] = 0x1a;
                            canframe->MessageID = formchanger_32u8.varu32;
                            canframe->MessageID |= LDD_CAN_MESSAGE_ID_EXT;
                            counter = 0xffff;
                            while((CAN_SendFrame(canframe) != ERR_OK) && (counter--));
                        }
                        break;

                    case 0x04:
                        memcpy((g_static_data.static_data.BMSID + BMSID_HIGH_POS), (canframe->Data + 1), 6);
                        W25Q64_SetStaticMSG(&g_static_data);
                        //ACK
                        formchanger_32u8.varu8[0] = src_addr;
                        formchanger_32u8.varu8[1] = g_stateblock_data.stata_union.state_data.ComAddr;
                        formchanger_32u8.varu8[2] = datacmd | 0xE0;
                        formchanger_32u8.varu8[3] = 0x1a;
                        canframe->MessageID = formchanger_32u8.varu32;
                        canframe->MessageID |= LDD_CAN_MESSAGE_ID_EXT;
                        counter = 0xffff;
                        while((CAN_SendFrame(canframe) != ERR_OK) && (counter--));
                        break;

                    case 0x05:
                        memcpy((g_static_data.static_data.BMSID + BMSID_LOW_POS), (canframe->Data + 1), 6);
                        W25Q64_SetStaticMSG(&g_static_data);
                        //ACK
                        formchanger_32u8.varu8[0] = src_addr;
                        formchanger_32u8.varu8[1] = g_stateblock_data.stata_union.state_data.ComAddr;
                        formchanger_32u8.varu8[2] = datacmd | 0xE0;
                        formchanger_32u8.varu8[3] = 0x1a;
                        canframe->MessageID = formchanger_32u8.varu32;
                        canframe->MessageID |= LDD_CAN_MESSAGE_ID_EXT;
                        counter = 0xffff;
                        while((CAN_SendFrame(canframe) != ERR_OK) && (counter--));
                        break;

                    case 0x06:   //Reset
                        if(memcmp((canframe->Data + 1), c_KeyWords, 4) == 0) {
                            Cpu_Reset();
                        }
                        break;
                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }
}
#endif


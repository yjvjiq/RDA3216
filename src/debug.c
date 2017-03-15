#include "debug.h"
#include "derivative.h"
#include "main.h"
//#include "isl28022.h"
//#include "isl1208.h"
#include "w25q64.h"
#include "drivers.h"
//#include "can_service.h"
#include "public.h"
#include "timer.h"


//******************************Retarget printf*************************************************
#pragma import(__use_no_semihosting_swi)

ring_frame _printf_ring_frame = {{0}, 0, 0, FALSE, TRUE};
#if 1

struct __FILE {
    int handle;
};
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
//  U8 counter = 0xffff;
//    while((_printf_ring_frame.full == TRUE)&&(counter--)) {};
//    while(_printf_ring_frame.full == TRUE) {};
    __disable_irq();
    if(_printf_ring_frame.full == TRUE) return 0;
    if(_printf_ring_frame.full == FALSE) {
        _printf_ring_frame.buffer[_printf_ring_frame.get] = ch;
        _printf_ring_frame.get == MAX_LEN ? (_printf_ring_frame.get = 0) : (_printf_ring_frame.get)++;
        if(_printf_ring_frame.get == _printf_ring_frame.put) {
            _printf_ring_frame.full = TRUE;
        } else {
            _printf_ring_frame.full = FALSE;
        }
        _printf_ring_frame.empty = FALSE;
    }
    if( _printf_ring_frame.empty == FALSE) {
       UART0->C2 |= UART_C2_TE_MASK | UART_C2_TCIE_MASK; //open uart0 send int
			// UART0->C2 |= UART_C2_TIE_MASK; //open uart0 send int
    }
    __enable_irq();
    return ch;
}

int ferror(FILE *f)
{
    /* Your implementation of ferror */
    return EOF;
}

void _sys_exit(int x)
{
    while(1);
}

//******************************Retarget printf*************************************************
#endif
void DebugSendCycle(ring_frame *ringframe)
{
    u8 ch = 0;
    __disable_irq();
    if(ringframe->empty == FALSE) {
        ch = ringframe->buffer[ringframe->put];
        UART0->D = ch;
        ringframe->put == MAX_LEN ? (ringframe->put = 0) : ++(ringframe->put);
        if(ringframe->get == ringframe->put) {
            ringframe->empty = TRUE;
        } else {
            ringframe->empty = FALSE;
        }
        ringframe->full = FALSE;
    } else {
        UART0->C2 &= ~UART_C2_TCIE_MASK;    //close uart0 send int
        // UART0->C2 &= ~UART_C2_TIE_MASK;    //close uart0 send int
    }
    __enable_irq();
}

U8 DebugPushChar(U8 ch)
{
    //  U8 counter = 0xffff;
    //    while((_printf_ring_frame.full == TRUE)&&(counter--)) {};
    //    while(_printf_ring_frame.full == TRUE) {};
    __disable_irq();
    if(_printf_ring_frame.full == TRUE) return 0;
    if(_printf_ring_frame.full == FALSE) {
        _printf_ring_frame.buffer[_printf_ring_frame.get] = ch;
        _printf_ring_frame.get == MAX_LEN ? (_printf_ring_frame.get = 0) : (_printf_ring_frame.get)++;
        if(_printf_ring_frame.get == _printf_ring_frame.put) {
            _printf_ring_frame.full = TRUE;
        } else {
            _printf_ring_frame.full = FALSE;
        }
        _printf_ring_frame.empty = FALSE;
    }
    if( _printf_ring_frame.empty == FALSE) {
        UART0->C2 |= UART_C2_TE_MASK | UART_C2_TCIE_MASK; //open uart0 send int
        // UART0->C2 |= UART_C2_TIE_MASK; //open uart0 send int
    }
    __enable_irq();
    return ch;
}

void DebugPushString(U8 *src, U8 len)
{
    for(U8 i = 0; i < len; i++) {
        DebugPushChar(src[i]);
    }
}

void Debug_Process(void)
{
#if 0
//read data block
    s_Flash_Data_Block_Data TempDataBlock;
    g_flashdata_frame.tail = g_flashdata_frame.head;
    Fault_Info *Fault_Infoptr = (void *)TempDataBlock.AlarmFlag;
    while(W25Q64_DataBlockPop(&g_flashdata_frame) == TRUE){
         W25Q64_DataBlockTop(&g_flashdata_frame, (void *)&TempDataBlock);
		Fault_Info *Fault_Infoptr = (void *)TempDataBlock.AlarmFlag;
		WDOG_Feed();
         if(Fault_Infoptr->mergedBits.BMS_MosFlt == TRUE){
            __nop();
         }
    }
#endif
    // W25Q64_DataBlockPop(&g_flashdata_frame);
    // W25Q64_DataBlockTop(&g_flashdata_frame, &TempDataBlock);
    // SPI_Flash_Erase_Chip();
//    U32 i = 0;
    // printf("Time:%d\n", g_log_data.log_data.UinxTime);
//    LDD_CAN_TFrame canframe;
//    U8 tempc[8] = {1,2,3,4,5,6,7,8};
//    canframe.Data = tempc;
//    canframe.FrameType = LDD_CAN_DATA_FRAME;
//    canframe.MessageID = 0x01;
//    canframe.LocPriority = 0;
//    canframe.Length = 8;
//    CAN_SendFrame(&canframe);
// printf("unixtime:0x%x\n",g_log_data.log_data.UinxTime);
//    g_log_data.log_data.FlashBlockHead= 0xff;
//    g_log_data.log_data.UinxTime = 0x12345678;
//    g_log_data.log_data.PackVoltageValue = 0xabcd;
//    g_log_data.log_data.NowCurrent = 0xef90;
//    g_log_data.log_data.NowSOC = 0x11;
//    g_log_data.log_data.PackMeanTemprature = 0x22;
//    g_log_data.log_data.CellMaxVoltage = 0x3344;
//    g_log_data.log_data.CellMaxVoltageNumber = 0x55;
//    g_log_data.log_data.CellMinVoltage = 0x6677;
//    g_log_data.log_data.CellMinVoltageNumber= 0x88;

//    g_log_data.log_data.CellMeanVoltageValue= 0xddcc;
//    g_log_data.log_data.Cell_T_1 = 0x99;
//    g_log_data.log_data.Cell_T_2 = 0xaa;
//    g_log_data.log_data.AlarmFlag[0] = 0xbb;
//    g_log_data.log_data.AlarmFlag[1] = 0xcc;
//    g_log_data.log_data.AlarmFlag[2] = 0xdd;
//    g_log_data.log_data.AlarmFlag[3] = 0xee;
//    g_log_data.log_data.AlarmFlag[4] = 0xff;
//    g_log_data.log_data.Crc = 0xabcd;
//
// g_stateblock_data.stata_union.state_data.HeadAddr = 0x7f1122;
// g_stateblock_data.stata_union.state_data.FlashBlockHead = 0xff;
// g_stateblock_data.stata_union.state_data.Flags = 0xaa;
// g_stateblock_data.stata_union.state_data.Crc = 0xaabb;
// g_stateblock_data.stata_union.state_data.ComAddr = 0xcd;
// g_stateblock_data.stata_union.state_data.BMSID[0] = 0x11;
// g_stateblock_data.stata_union.state_data.BMSID[1] = 0x22;
// g_stateblock_data.stata_union.state_data.BMSID[2] = 0x33;
// g_stateblock_data.stata_union.state_data.BMSID[3] = 0x44;
// g_stateblock_data.stata_union.state_data.BMSID[4] = 0x55;
// g_stateblock_data.stata_union.state_data.BMSID[5] = 0x66;

//U32 temp;			
//			g_CurrentRegValue=370;//
//			ISL28022_Calibration(1000,0);
//			g_CurrentRegValue=1480;//
//			ISL28022_Calibration(4000,1);	
//			temp= CalibrationCurrent(1480,&g_static_data.static_data.Calibration[CHARGE_40A],1000);	
//            temp--;	
//			g_CurrentRegValue=32;//
//			ISL28022_Calibration(100,2);
//			g_CurrentRegValue=368;//
//			ISL28022_Calibration(1000,3);
//			temp= CalibrationCurrent(368,&g_static_data.static_data.Calibration[DISCHARGE_10A],1000);	
//            temp--;	
//			g_CurrentRegValue=3740;//
//			ISL28022_Calibration(10000,4);

//            temp= CalibrationCurrent(3740,&g_static_data.static_data.Calibration[DISCHARGE_100A],1000);	
//            temp--;	


#if 0 //static data
    U8 addr = 2;
    // g_stateblock_data.stata_union.state_data.ComAddr = addr;
    // W25Q64_SetStateMSG(&g_stateblock_data, &g_flashdata_frame);
    g_static_data.static_data.BMSID[0] = 0x2c;
    g_static_data.static_data.BMSID[1] = 0xd7;
    g_static_data.static_data.BMSID[2] = 0x00;
    g_static_data.static_data.BMSID[3] = 0x00;
    g_static_data.static_data.BMSID[4] = 0x00;
    g_static_data.static_data.BMSID[5] = 0x01;
    g_static_data.static_data.BMSID[6] = 0x00;
    g_static_data.static_data.BMSID[7] = 0x00;
    g_static_data.static_data.BMSID[8] = 0xaa;
    g_static_data.static_data.BMSID[9] = 0xbb;
    g_static_data.static_data.BMSID[10] = 0xcc;
    g_static_data.static_data.BMSID[11] = 0x02;
    // g_static_data.static_data.MaxPermitsCurrent = 0x4321;
    // g_static_data.static_data.MaxPermitsVoltage = 0xabcd;
    // g_static_data.static_data.FactoryReset = 0xffffffff;
    // g_static_data.static_data.CurrentCalibrationParam[0].Ratio = 0x1234;
    // g_static_data.static_data.CurrentCalibrationParam[0].Offset = 0x4321;
    // g_static_data.static_data.CurrentCalibrationParam[1].Ratio = 0x5678;
    // g_static_data.static_data.CurrentCalibrationParam[1].Offset = 0x8765;
    // g_static_data.static_data.CurrentCalibrationParam[2].Ratio = 0x1357;
    // g_static_data.static_data.CurrentCalibrationParam[2].Offset = 0x7531;
    W25Q64_SetStaticMSG(&g_static_data);
    while(1);
#endif

#if 0 //sleep
    printf("%d\n", g_log_data.log_data.UinxTime);
    delay_ms(2000);
    g_mcu_mode = SLEEP;
    Cpu_Sleep_KbiWake();
#endif

#if 0                 //isl28022
    static U8 bufs[2] = {0};
    Isl28022_WrReg(ConfigReg, 0x7909);
    Isl28022_RdReg(ConfigReg, bufs);
    while(!Get_IntFlag(I2C0_RXFINISH_POS));
    Clear_IntFlag(I2C0_RXFINISH_POS);
    printf("Read ISL28022 reg : 0x%x,0x%x\n", bufs[0], bufs[1]);
    while(1);
#endif
#if 0               //isl1208
//               sec   min  hour date mon  year
    U8 bufs[6] = {0x50, 0x59, 0x23, 0x12, 0x11, 0x15};
    U16 i = 0;
    ISL1208_SetTime(bufs);
    while(1) {
        ISL1208_GetTime(bufs);
//    Isl1208_RdReg(ISL1208_REG_SR, bufs);


        printf("sec:0x%x\n", bufs[0]);
        printf("min:0x%x\n", bufs[1]);
        printf("hour:0x%x\n", bufs[2]);
        printf("date:0x%x\n", bufs[3]);
        printf("mon:0x%x\n", bufs[4]);
        printf("year:0x%x\n", bufs[5]);

        delay_ms(1000);
    }
#endif
#if 0  //W25Q64
   // U8 databuffr[512] = {0xff, 2, 3, 4, 5, 6, 7};
   U8 i = 0;


#if 0
    SPI_Flash_Erase_Sector(0x0);
    SPI_Flash_Erase_Sector(0x1000);
    SPI_Flash_Erase_Sector(0x2000);
    SPI_Flash_Erase_Sector(0x3000);
    test_addr = 0x1f00;
    test_num = 0xfc;
    W25Q64_WriteStateBlcok(&test_addr, SBUFF);
    test_addr = 0x2f80;
    test_num = 0xfc;
    SPI_Flash_Write_NoCheck(SBUFF, test_addr, W25Q64_STATE_BLOCKSIZE);
    SPI_Flash_Write_NoCheck(&test_num, test_addr, 1);
    test_addr = 0x2f80;
    test_num = 0xfc;
    SPI_Flash_Write_NoCheck(SBUFF, test_addr, W25Q64_STATE_BLOCKSIZE);
    SPI_Flash_Write_NoCheck(&test_num, test_addr, 1);
    test_addr = 0x2f40;
    test_num = 0xfc;
    SPI_Flash_Write_NoCheck(SBUFF, test_addr, W25Q64_STATE_BLOCKSIZE);
    SPI_Flash_Write_NoCheck(&test_num, test_addr, 1);
    test_addr = 0x2f80;
    test_num = 0xfc;
    SPI_Flash_Write_NoCheck(SBUFF, test_addr, W25Q64_STATE_BLOCKSIZE);
    SPI_Flash_Write_NoCheck(&test_num, test_addr, 1);
#endif

//    SPI_Flash_Erase_Sector(0x0);
//    SPI_Flash_Erase_Sector(0x1000);
//    SPI_Flash_Erase_Sector(0x2000);
//    SPI_Flash_Erase_Sector(0x3000);
//    SPI_Flash_Erase_Sector(0x4000);
//    SPI_Flash_Erase_Sector(0x5000);
//    SPI_Flash_Erase_Sector(0x6000);
//    SPI_Flash_Erase_Chip();



    while(1) {
//        for(i = 0; i < 1; i++) {
//            if(W25Q64_GetStateBlcok(&addr, SBUFF) == TRUE) {
//                printf("GetAddr:0x%x\n", addr);
//                W25Q64_GetFlashStateMsg(&dataframe, SBUFF);
//                printf("head:0x%x,tail:0x%x,empty:0x%x,full:0x%x\n", dataframe.head, dataframe.tail, dataframe.empty, dataframe.full);
//                delay_ms(500);
//            } else { //STATE Area empty init dataframe
//                printf("State Area Empty!\n");
//                printf("GetAddr:0x%x\n", addr);
//                dataframe.head = W25Q64_DATA_START_ADDR;
//                dataframe.tail = W25Q64_DATA_START_ADDR;
//                dataframe.empty = TRUE;
//                dataframe.full = FALSE;
//            }
//        }
#if 1
        for(i = 0; i < 10; i++) {
            g_log_data.data[1]++;
            W25Q64_WriteOneDataBlock(&g_flashdata_frame, g_log_data.data);
            printf("WriteBlock:head:0x%x,tail:0x%x,empty:0x%x,full:0x%x,buffer[1]:0x%x", g_flashdata_frame.head, g_flashdata_frame.tail, g_flashdata_frame.empty, g_flashdata_frame.full, g_log_data.data[1]);
            delay_ms(100);
        }
#endif
#if 0
        test_addr = 0x4e00;
        test_num = 0xfc;
        SPI_Flash_Write_NoCheck(&test_num, 0x4e00 + 1, 1);
        SPI_Flash_Read(&test_num, 0x4e00 + 1, 1);
        printf("\naAddr:0x%x,data:0x%x||\n", 0x4e00 + 1, test_num);
        test_addr = 0x4c00;
        test_num = 0xfc;
        SPI_Flash_Write_NoCheck(&test_num, 0x4c00 + 1, 1);
        SPI_Flash_Read(&test_num, 0x4c00 + 1, 1);
        printf("\naAddr:0x%x,data:0x%x||\n", 0x4c00 + 1, test_num);
#endif
#if 1
        // g_flashdata_frame.tail = g_flashdata_frame.head;
        for(i = 0; i < 10; i++) {
            if(W25Q64_ReadOneDataBlock(&g_flashdata_frame, g_log_data.data) == TRUE) {
                printf("ReadBlock:head:0x%x,tail:0x%x,empty:0x%x,full:0x%x,buffer[1]:0x%x", g_flashdata_frame.head, g_flashdata_frame.tail + W25Q64_DATA_BLOCKSIZE, g_flashdata_frame.empty, g_flashdata_frame.full, g_log_data.data[1]);
                delay_ms(100);
            } else {
                printf("DataBlockEmpty!\n");
            }
        }
#endif
//        for(i = 0; i < 1; i++) {
//            W25Q64_SetFlashStateMsg(&dataframe, SBUFF);
//            for(i = 0; i < 8; i++) {
//                printf("buff[%d]:0x%x\n", i, SBUFF[i]);
//            }
//            W25Q64_WriteStateBlcok(&addr, SBUFF);
//            printf("WriteAddr:0x%x\n", addr);
//            delay_ms(500);
//        }
        W25Q64_SetStateMSG(&g_stateblock_data, &g_flashdata_frame);
        while(1);
    }

#endif



#if 0//can 
//    g_can_recframe.MessageID = 0x0d110102;
//    g_can_recdata[0] = 0;
//    g_can_recdata[1] = 0;
//    g_can_recdata[2] = 0;
//    g_can_recdata[3] = 5;
//   CanCmdParsingProc(&g_can_recframe);

//   while(1)
//   {
    if(g_canmsg.state == FINISH) {
        FlashPopAndSendProc();
    }
//   }
#endif

#if 0 //RTC SLEEP 
    printf("MCU_SLEEP\n");
    delay_ms(50);
    Cpu_Sleep_RtcWake();
    printf("MCU_WAKE\n");
#endif

//  Cpu_Sleep_KbiWake();
#if 0  //CRC
    u32 crc = 0;
    U8 crcbuf[5] = {1, 2, 3, 4, 5};
    Crc_MakeCrc(crcbuf, 5, &crc);
    printf("crc:0x%x\n", crc);
#endif
}

void DisplayBuffer(U8 saddr, U8 *src ,U8 len)
{
    printf("S0x%xCrcErrBuf:", saddr);
    for(U8 i = 0; i < len; i++){
        printf("%02x\0",src[i]);
    }
    printf("\n");
}

#ifdef VERSION_BTT
#ifdef BTT_OFFLINE
void UpLoadBttData()
{
    U8 c_SendHead[3] = {0x01, 0x03, 0x40};
    U8 EptChar = 0xff;
    DebugPushString(c_SendHead, 3);
    if(g_BatteryTestToolFrame.UpdataFlag == TRUE){
        // DebugPushString((U8*)&g_BatteryTestToolFrame, sizeof(s_BatteryTestToolFrame) - 1);
        DebugPushString((U8*)&g_BatteryTestToolFrame.BatteryID, BATTERTY_ID_LEN);
        DebugPushString((U8*)&g_BatteryTestToolFrame.CellVoltage[0], 5*2);
        DebugPushString((U8*)&g_BatteryTestToolFrame.CellVoltage[8], 9*2);
        DebugPushString((U8*)&g_BatteryTestToolFrame.CellVoltage[19], 5*2);
        DebugPushString((U8*)&g_BatteryTestToolFrame.PackVoltage, 5 + 5);
    }else{
        DebugPushString((U8*)&g_BatteryTestToolFrame.BatteryID, BATTERTY_ID_LEN);
        for(U8 i = 0; i < (63 - 3 - BATTERTY_ID_LEN); i++){
            DebugPushString(&EptChar, 1);
        }
    }
}
#endif
#endif


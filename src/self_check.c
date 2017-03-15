#include "self_check.h"
#include "public.h"
#include "w25q64.h"
#include "uartdata_service.h"
#include "rf_service.h"
#include "timer.h"
#include "Data_Sampling.h"

#ifdef PRODUCTION_TEST
// #define SELF_CHECK_STEP1	0
// #define SELF_CHECK_STEP2	1

// s_SelfCheck g_SelfCheckStatus = {0};
static scTestResult test_result = {0};

scRfAck g_scRfAckMsg = {0};
void scRfTestInfoInit(void)
{
	// g_scRfAckMsg.tx = 0;
	// g_scRfAckMsg.expect_rx = 0;
	// g_scRfAckMsg.rx = 0;
	// g_scRfAckMsg.offline = 0;
	// g_scRfAckMsg.crc_err = 0;
	// g_scRfAckMsg.bgn = 0;
	// g_scRfAckMsg.rssi = 0;
	// g_scRfAckMsg.cinr = 0;
	// g_scRfAckMsg.lqi = 0;
	memset(&g_scRfAckMsg, 0, sizeof(g_scRfAckMsg));
}

void scTestInfoInit(void)
{
	memset(&test_result, 0, sizeof(test_result));
	MOS_K1_OFF;
	MOS_K2_GND_OFF;
}

void scInit(void)
{
	User_Init();
	scTestInfoInit();
	scRfTestInfoInit();
	rfInitTestMsg();

	SetTimer1(SELF_CHECK_UPLOAD_TIMER_ID, 500, TIMER_MODE_PERIODIC, NULL);
	SetTimer1(ISL_INIT_ID, ISL_INIT_MS, TIMER_MODE_PERIODIC, NULL);
}

void FlashSelfCheckProc(void)
{
	U8 LastTestByte = 0;
	LastTestByte = g_stateblock_data.stata_union.state_data.TestByte;
	g_stateblock_data.stata_union.state_data.TestByte ++;
	W25Q64_SetStateMSG(&g_stateblock_data);
	W25Q64_GetStateMSG(&g_stateblock_data);
	LastTestByte ++;
	if(LastTestByte == g_stateblock_data.stata_union.state_data.TestByte){
		#if 0
		g_SelfCheckStatus.FlashCheck = 1;
		#endif
		test_result.flash = 1;
	}else{
		#if 0
		g_SelfCheckStatus.FlashCheck = 0;
		#endif
		test_result.flash = 0;
	}
}

void scSetCmdHandler(void* data)
{
	scCmdToSlave *cmd = data;

	scTestInfoInit();
	if(cmd->mos_k1 == TRUE)
	{
		MOS_K1_ON;
	}
	else
	{
		MOS_K1_OFF;
	}

	if(cmd->mos_k2 == TRUE)
	{
		MOS_K2_GND_ON;
	}
	else
	{
		MOS_K2_GND_OFF;
	}

	test_result.flash = 0;
	if(cmd->flash == TRUE)
	{
		FlashSelfCheckProc();
	}

}

void scRfTestCmdHandler(void* data)
{
	scRfSet* rf_set = data;

	scRfTestInfoInit();
	rfInitTestMsg();

	if(rf_set->tr_switch == 1)
	{
		rfTestSetTxParam(rf_set->freq_index, rf_set->tx_power);
		SetTimer1(RF_TEST_TX_TIMER_ID, rf_set->cycle, TIMER_MODE_PERIODIC, NULL);
	}
	else if(rf_set->tr_switch == 0)
	{
		rfTestSetRxParam(rf_set->freq_index);
		StopTimer1(RF_TEST_TX_TIMER_ID);
	}
}

// void SelfCheckProc(void)
// {
// 	static U8 SelfCheckStep = SELF_CHECK_STEP1;

// 	switch(SelfCheckStep){
// 		case SELF_CHECK_STEP1:
// 			MOS_ON;
// 			SelfCheckStep = SELF_CHECK_STEP2;
// 		break;

// 		case SELF_CHECK_STEP2:
// 			MOS_OFF;
// 			SelfCheckStep = SELF_CHECK_STEP1;
// 		break;

// 		default:
// 			SelfCheckStep = SELF_CHECK_STEP1;
// 		break;
// 	}

// 	PushAckString(0x4d|0x80, (U8 *)&g_SelfCheckStatus, sizeof(s_SelfCheck));
// }

void scCheckProc(void)
{
	DataSamplingAndProcess();

	for(uint8_t i = 0; i < 5; i++)
	{
		test_result.cell_volts[i] = g_battery_pack_info.sample_data.cell_voltages[i];
	}

	for(uint8_t i = 0; i < 9; i++)
	{
		test_result.cell_volts[i + 5] = g_battery_pack_info.sample_data.cell_voltages[8 + i];
	}

	for(uint8_t i = 0; i < 5; i++)
	{
		test_result.cell_volts[i + 14] = g_battery_pack_info.sample_data.cell_voltages[19 + i];
	}

	test_result.total_volt = g_log_data.log_data.PackVoltageValue;

	memcpy(test_result.cell_T , g_log_data.log_data.Cell_T, 2);
	test_result.current = g_log_data.log_data.NowCurrent;
	
}

void scCheckResultUpload(void)
{
	PushAckString(0x57|0x80, (U8 *)&test_result, sizeof(test_result));
}

void scRfTestResultUpload(void)
{
	PushAckString(0x58|0x80, (U8 *)&g_scRfAckMsg, sizeof(g_scRfAckMsg));
}

int main(void)
{   
    scInit();
    while(1)
    {
		#ifdef WATCH_DOG_ENABLE
	        if(Get_IntFlag(RTC_FEED_DOG_POS)){
	            Clear_IntFlag(RTC_FEED_DOG_POS);
	            WDOG_Feed();
	        }
		#endif

		if(Get_IntFlag(UART0_RXFINISH_POS)){
		    Clear_IntFlag(UART0_RXFINISH_POS);
		    UartDataParse(&g_uartrecframe);
		}

        if(GetTimer1(SELF_CHECK_UPLOAD_TIMER_ID) == TRUE)
        {
	       	scCheckProc();
	       	scCheckResultUpload();
			delay_ms(1);
	       	scRfTestResultUpload();
        }

        if(GetTimer1(RF_TEST_TX_TIMER_ID) == TRUE)
        {
        	rfSendTestTxMsg();
        }

        if(g_CC1120RxUpdate == TRUE){
            rfTestPacketParse(CC1120_rxBuffer);
            g_CC1120RxUpdate = FALSE;
        }

        if(Get_IntFlag(CC1120_RX_INT_POS)){
            CC1120_RecivePacket();
			Clear_IntFlag(CC1120_RX_INT_POS);
		}

		U8 isl_init_status_t = 0;
        if(GetTimer1(ISL_INIT_ID) == TRUE){
			if(g_isl_init_step_flag != 5){
				isl_init_status_t = ISL_Init();
			}
			
			if(isl_init_status_t == 4){
				g_isl_init_step_flag = 0;
				g_battery_pack_info.error_flag.mergedBits.BMS_Isl_Init_Flt = 1;
			}
			else if(isl_init_status_t == 3){
				g_isl_init_step_flag = 5;
				g_battery_pack_info.error_flag.mergedBits.BMS_Isl_Init_Flt = 0;
			}
		}
    }
}
#endif

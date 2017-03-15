
/**********************************************************************************************
* External objects
**********************************************************************************************/

//#include "derivative.h"
#include "drivers.h"
#include "main.h"
#include "debug.h"
#include "timer.h"
#include "isl28022.h"
#include "w25q64.h"
#include "can_service.h"
#include "public.h"
#include "Data_Sampling.h"
#include "Data_Process.h"
#include "soc.h"
#include "cc1120.h"
#include "rf_service.h"
#include "power_manage.h"
#include "uartdata_service.h"
#include "config.h"
#include "self_check.h"
#include "xmodem.h"
#include "rda3216.h"

U16 volatile g_int_flag = 0; //GLOBAL Interrupt flag

/* version info */

U8 g_SoftVersion[4] = {1, 5, 19, 0};
U8 g_HardWareVersion[4] = {5, 0, 0, 0};
/***********************************************************************************************
*
* @brief    main() - Program entry function
* @param    none
* @return   none
*
************************************************************************************************/

extern U8 g_Serial_RegNO;
static U32 read_voltage_cnt = 0;
int main(void)
{
    User_Init();
	MASTER_SPI_CS_OFF();
    //7 device, 7 voltage and 9 auxilary adc channels.
	RDA_DeviceConfig(CHAIN_DEV_NUM,SevenVol_NineAux);
//	RDA3216_Init();
    g_Serial_RegNO = 16;
	printf("this is serial!\n");
	U32 p[48] = 0;
	for(;;){
		printf("\n\nNO.%d", (int)read_voltage_cnt);
		RDA_Test();
		delay_ms(500);
		read_voltage_cnt++;
	}
}



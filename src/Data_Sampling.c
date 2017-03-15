#include "Data_Sampling.h"
#include "isl94212.h"
#include "types.h"
#include "drivers.h"
#include "public.h"

/** 
 * AFE Sampling Step
 */
typedef enum {
    AFE_Sampling_Step_Prepare = 0,          /* Prepare for Reading */
    AFE_Sampling_Step_ReadCell,          /* Read Voltage */                
    AFE_Sampling_Step_ReadTemperature       /* Read Temperature */
} AFE_Sampling_Step;

//static AFE_Sampling_Step s_sampling_step = AFE_Sampling_Step_Prepare;
U8 g_VoltageSampleErrNumber=0;
#define V_SAMPLE_ERR 3
/*************************************************************/
/*                 Read_Cell_Voltage_Array                   */
/*************************************************************/
void Read_Cell_Voltage_Array(U16 *cell_voltage_array, U16 *outVBAT, U8 devAddress) {
    Bool readResult;
    U8 cellIndex;
    U8 offset = 0;
    offset = (devAddress - 1) * CELL_NUMBER_IN_AFE;

    readResult = Read_All_Voltage(	devAddress, 
									&isl_pack.devices[devAddress - 1].cellVoltages[0], 
									&isl_pack.devices[devAddress - 1].batVoltage);
	if(readResult != FALSE){
		g_VoltageSampleErrNumber=0;
		for(cellIndex = 0; cellIndex < CELL_NUMBER_IN_AFE; cellIndex++){
		#if 0
			if(Is_In_Array(g_busbar_index_array, sizeof(g_busbar_index_array), offset + cellIndex)) {
				*(cell_voltage_array + offset + cellIndex) = 0;
				continue;
			}
		#endif
		*(cell_voltage_array + offset + cellIndex) = isl_pack.devices[devAddress - 1].cellVoltages[cellIndex];
		*(outVBAT + devAddress - 1)= isl_pack.devices[devAddress - 1].batVoltage;
		}
	}
	else{
		g_VoltageSampleErrNumber++;
		if(g_VoltageSampleErrNumber > V_SAMPLE_ERR){
		g_isl_init_step_flag = 0;
		}
	}
}

/*************************************************************/
/*                Read_Cell_Temperature_Array                */
/*************************************************************/
void Read_Cell_Temperature_Array(U8 *cell_temperature_array, 
                                 U8 devAddress) {
    Bool readResult;
    U8 tempIndex;

    readResult = Read_All_Temperature(	devAddress,
										&isl_pack.devices[devAddress - 1].extTemps[0]);
	if(readResult != FALSE){
		for (tempIndex = 0; tempIndex < TEMP_NUMBER_IN_AFE; tempIndex++) {
			*(cell_temperature_array + tempIndex) = isl_pack.devices[devAddress - 1].extTemps[tempIndex];			
		}
	}
	else{
	}
}

U8 ISL_Init(void){
    ISL_ErrCode errCode = ErrCode_NoResponse;
	static U8 s_delay_cnt = 0;
	static U8 s_delay_enset_cnt = 0;
	static U8 s_delay_init_cnt = 0;
	U8 isl_init_status = 0;
	
	switch(g_isl_init_step_flag){
		case 0:{	//delay a time for the first connect with the connector.
			s_delay_cnt++;
			if(s_delay_cnt >= 1){
				OUTPUT_CLEAR(PTE,PTE5); // disable EN port
				s_delay_cnt = 0;
				g_isl_init_step_flag = 1;
			}
			else{
				g_isl_init_step_flag = 0;
			}
			isl_init_status = 1;
			break;
		}
		case 1:{
			s_delay_enset_cnt++;
			if(s_delay_enset_cnt >= 1){
				OUTPUT_SET(PTE,PTE5);	//enable EN port
				s_delay_enset_cnt = 0;
				g_isl_init_step_flag = 2;
			}
			else{
				g_isl_init_step_flag = 1;
			}
			isl_init_status = 2;
			break;
		}
		case 2:{
			s_delay_init_cnt++;
			if(s_delay_init_cnt >= 1){
				s_delay_init_cnt = 0;
				
				for(U8 devNum=NUMBER_OF_AFE_DEVICES;devNum>0;devNum--){
					DaisyChain_Reset(devNum);  // reset from top device to bottom device
				}

				errCode = DaisyChain_Identify(NUMBER_OF_AFE_DEVICES);
				if(errCode == ErrCode_None){
					errCode = DaisyChain_Set_CellSetup(1, 0x0000);
					errCode = DaisyChain_Set_CellSetup(2, 0x0000);
					isl_init_status = 3;
				}
				else{
					isl_init_status = 4;
				}
			}
			
			break;
		}
		default:{
			break;
		}
	}
	return isl_init_status;
}


/******************************************************************************/
/**                                                                           */
/* @brief Function Name:                                                      */
/* @brief Description  :                                                      */
/* @param parameters   :                                                      */
/* @return Value       :                                                      */
/******************************************************************************/  
// #ifdef VERSION_BMS
void Data_Sampling_AFE_Main(Sample_Data *data) {
	if(g_isl_init_step_flag == 5){
		for(U8 i=1;i<=NUMBER_OF_AFE_DEVICES; i++){
			Read_Cell_Voltage_Array(data->cell_voltages, data->total_voltage,i);
		}
		/* just device 1 connected to NTC, 4 channels. */ 
		Read_Cell_Temperature_Array(data->cell_temperatures,1);
	}
	else{
		
	}
#ifndef PRODUCTION_TEST
	#ifdef VERSION_BMS
	    printf("AFE\n");
	#endif
#endif
}
// #endif

// #ifdef VERSION_BTT
// U8 Data_Sampling_AFE_Main(Sample_Data *data) {
//     U8 Ret = FALSE;

//     OUTPUT_SET(PTE,PTE5);       //FGPIOB_PSOR   |= 1<<5; MASTER_EN enable
//     switch(s_sampling_step){
//         case AFE_Sampling_Step_Prepare: {
//             if(ISL_Init() == TRUE){
//                 s_sampling_step = AFE_Sampling_Step_ReadCell;
// #ifdef VERSION_BMS
//                 printf("isl94212 identify success.\n");
// #endif
//                 Ret =  TRUE;
//             }
//             else{
//                 s_sampling_step = AFE_Sampling_Step_Prepare;
// #ifdef VERSION_BMS
//                 printf("isl94212 init err.\n");
// #endif
//                 Ret =  FALSE;
//             }
//         }
//         break;
//         case AFE_Sampling_Step_ReadCell: {
//             for(U8 i=1;i<=NUMBER_OF_AFE_DEVICES; i++){
//                 Read_Cell_Voltage_Array(data->cell_voltages, data->total_voltage,i);
//             }
//             /* just device 1 connected to NTC, 4 channels. */ 
//             Read_Cell_Temperature_Array(data->cell_temperatures,1);
//             s_sampling_step = AFE_Sampling_Step_ReadCell;
//             Ret = TRUE;
//         }
//         break;
//         default:
//             Ret = FALSE;
//         break;
//     }
// #ifdef VERSION_BMS 
//     printf("AFE\n");
// #endif
//     return Ret;
// }
// #endif

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: AFE_Sys_Check                                        */
/* @brief Description  : Read the device address from ISL78600 register to    */
/*                       ensure that the device address is right. if not so   */
/*                       re-init the daisychain.                              */
/* @param parameters   :                                                      */
/* @return Value       : True: System is OK; False: System is error           */
/******************************************************************************/ 
Bool AFE_Sys_Check(volatile Error_Counter *error_counter) {
    U8 devAddress;
    U8 regDevAddress;
    ISL_ErrCode errCode;
    Bool needReset;
    Bool systemOK;
    needReset = FALSE;
    systemOK = TRUE;    
    for(devAddress = 1; devAddress <= NUMBER_OF_AFE_DEVICES; devAddress++){
        errCode = DaisyChain_Get_CommsSetup(devAddress, &isl_pack.devices[devAddress - 1].setup.comms_setup.data);
        if(errCode == ErrCode_None) {
            /* Check the device address setup */
            regDevAddress = (U8)isl_pack.devices[devAddress - 1].setup.comms_setup.mergedBits.ADDR;
            if(regDevAddress != devAddress) {
                needReset = TRUE;
                break;
            }
        } else {
            needReset = TRUE;
            break;    
        }
        if(isl_pack.devices[devAddress - 1].refVoltage == 0 || 
           isl_pack.devices[devAddress - 1].fault.fault_status.mergedBits.REG == 1) {
            needReset = TRUE;
            break;
        }
    }

    if(needReset) {
        errCode = Init_ISL_DaisyChain();
        if(errCode != ErrCode_None) {
            printf("AFE Re-Init Failed with code: %d", errCode);
            systemOK = FALSE;
//            Set_U16_Bit(&Error_Counter->error_flag_BMS_AFE, (U8)ErrIndex_AFE_Init);
        } else {
//            Clear_U16_Bit(&Error_Counter->error_flag_BMS_AFE, (U8)ErrIndex_AFE_Init);
        }
    }
    return systemOK;
}

#include "Data_Process.h"
#include "timer.h"
#include "drivers.h"
#include "public.h"
#include "types.h"
#include "alert.h"
#include "w25q64.h"

/**************************************************************
* Data_Storage_Main, used in Data_Process_Main()
*************************************************************/
void Copy_To_Pack_Data_Struct(Pack_Data_Struct *log_data_t,
								Battery_Pack_Info *battery_pack_info){
	U8 i;
	uint32_t CRC_result;
	
//	log_data_t->log_data.FlashBlockHead = 1;	// need to be assigned further
	log_data_t->log_data.NowSOC = battery_pack_info->SOC;
	log_data_t->log_data.NowCurrent = battery_pack_info->sample_data.total_current;
//	log_data_t->log_data.UinxTime = 1;			// need to be assigned further
	log_data_t->log_data.PackVoltageValue = battery_pack_info->sample_data.total_cell_voltage;
	log_data_t->log_data.PackMeanTemprature = battery_pack_info->sample_result.ave_cell_temperature;
	
	log_data_t->log_data.CellMaxVoltageNumber = battery_pack_info->sample_result.max_cell_voltage_number;
	log_data_t->log_data.CellMaxVoltage = battery_pack_info->sample_result.max_cell_voltage;
	log_data_t->log_data.CellMinVoltage = battery_pack_info->sample_result.min_cell_voltage;
	log_data_t->log_data.CellMeanVoltageValue = battery_pack_info->sample_result.ave_cell_voltage;
	log_data_t->log_data.CellMinVoltageNumber = battery_pack_info->sample_result.min_cell_voltage_number;
	
	log_data_t->log_data.Cell_T[0] = battery_pack_info->sample_data.cell_temperatures[0];
	log_data_t->log_data.Cell_T[1] = battery_pack_info->sample_data.cell_temperatures[1];
	
	for(i=0;i<5;i++){
		log_data_t->log_data.AlarmFlag[i] = battery_pack_info->error_flag.data[i];
	}
	/*reserved data bank */
//	for(i=0;i<sizeof(log_data_t->log_data.Rev);i++){
//		log_data_t->log_data.Rev[i] = 0;
//	}
	Crc_MakeCrc(log_data_t->data, sizeof(log_data_t) - 2, &CRC_result);
	log_data_t->log_data.Crc = (U16)CRC_result;
}

/**************************************************************
* Data_Process_Main, used in main()
*************************************************************/
void Data_Process_Main(	Battery_Pack_Info  *battery_pack_info, 
						FlashStatic_Data_Union *static_data, 
						Pack_Data_Struct *log_data_t,
						Error_Counter  *error_counter) {
	U8 i = 0;
	U32 sum_voltage = 0;
	U32	sum_sampling_voltage = 0;
	U8 cell_number = 0;
	U16 sum_temperature = 0;
	U8 connected_T_num = 0;
	U8 cell_volt_err_flag = 0;
							
	/*  Calculate Max, Min Voltage */
    battery_pack_info->sample_result.max_cell_voltage = 0;
    battery_pack_info->sample_result.max_cell_voltage_number = 0xFF;
    battery_pack_info->sample_result.min_cell_voltage = 0xFFFF;
    battery_pack_info->sample_result.min_cell_voltage_number = 0xFF;
    
    for(i = 0; i < CELL_VOLTAGE_CHANNEL; i++) {
#if 1
        if(Is_In_Array(g_busbar_index_array, sizeof(g_busbar_index_array), i)) {
            continue;
        }
#endif
        if(battery_pack_info->sample_result.max_cell_voltage < battery_pack_info->sample_data.cell_voltages[i]) {
            battery_pack_info->sample_result.max_cell_voltage =  battery_pack_info->sample_data.cell_voltages[i];
            battery_pack_info->sample_result.max_cell_voltage_number  = i + 1;
        }
        
        if(battery_pack_info->sample_result.min_cell_voltage > battery_pack_info->sample_data.cell_voltages[i]) {
            battery_pack_info->sample_result.min_cell_voltage =  battery_pack_info->sample_data.cell_voltages[i];
            battery_pack_info->sample_result.min_cell_voltage_number  = i + 1;
        }
        cell_number += 1;
		
        sum_voltage +=  battery_pack_info->sample_data.cell_voltages[i];
		
		if(battery_pack_info->sample_data.cell_voltages[i] <= 2500 ||
			battery_pack_info->sample_data.cell_voltages[i] >= 4500){
			cell_volt_err_flag = 1;
		}
    }
			
    battery_pack_info->sample_result.diff_cell_voltage = (U16)(battery_pack_info->sample_result.max_cell_voltage - 
															battery_pack_info->sample_result.min_cell_voltage);
	/*  total voltage calculate */
	if(1 == cell_volt_err_flag){
		for(U8 i=0;i<NUMBER_OF_AFE_DEVICES; i++){
			sum_sampling_voltage += (U32)battery_pack_info->sample_data.total_voltage[i];
		}
		sum_sampling_voltage += 200; // calibration value
		battery_pack_info->sample_data.total_cell_voltage = (U16)(sum_sampling_voltage / 10);    // total voltage/10 (mV), 8000 = 80.00V
	}
	else{
		battery_pack_info->sample_data.total_cell_voltage = (U16)(sum_voltage / 10);    // total voltage/10 (mV), 8000 = 80.00V
	}
	
	/* if sampling Vpack - Vcell sum, maybe Vpack sampling circute error, error flag set */
	if(sum_voltage > sum_sampling_voltage){
		if((sum_voltage - sum_sampling_voltage) >= 300){
			battery_pack_info->error_flag.mergedBits.BMS_VpackSampleFlt = 1;
		}
		else{
			battery_pack_info->error_flag.mergedBits.BMS_VpackSampleFlt = 0;
		}
	}
	else{
		if((sum_sampling_voltage - sum_voltage) >= 300){
			battery_pack_info->error_flag.mergedBits.BMS_VpackSampleFlt = 1;
		}
		else{
			battery_pack_info->error_flag.mergedBits.BMS_VpackSampleFlt = 0;
		}
	}
	
	
	battery_pack_info->sample_result.ave_cell_voltage = (U16)(sum_voltage / (U32)cell_number);

    /*  Calculate Max, Min Temperature */
    battery_pack_info->sample_result.max_cell_temperature = 0;
    battery_pack_info->sample_result.max_cell_temperature_number = 0xFF;
    battery_pack_info->sample_result.min_cell_temperature = 0xFF;
    battery_pack_info->sample_result.min_cell_temperature_number = 0xFF;
    
    for(i = 0; i < CELL_TEMPERATURE_CHANNEL; i++) {
		// ignore the too high data or unconnected ones, if one is not connected, the value is 174+40 = 214
		/* 120 degree +40 offset = 160, 160 is max T data in CAN */
		if(battery_pack_info->sample_data.cell_temperatures[i] > 160){
			battery_pack_info->sample_data.cell_temperatures[i] = 215; // 175±íÊ¾´íÎóÖµ
#ifdef VERSION_BMS
			printf("T_%d is too high or unconnected!\n", i+1);
#endif
			continue;
		}
		if(battery_pack_info->sample_result.max_cell_temperature < battery_pack_info->sample_data.cell_temperatures[i]) {
			battery_pack_info->sample_result.max_cell_temperature =  battery_pack_info->sample_data.cell_temperatures[i];
			battery_pack_info->sample_result.max_cell_temperature_number  = i + 1;
		}

		if(battery_pack_info->sample_result.min_cell_temperature > battery_pack_info->sample_data.cell_temperatures[i]) {
			battery_pack_info->sample_result.min_cell_temperature =  battery_pack_info->sample_data.cell_temperatures[i];
			battery_pack_info->sample_result.min_cell_temperature_number  = i + 1;
		}
        
        sum_temperature += battery_pack_info->sample_data.cell_temperatures[i];
		connected_T_num++;
    }
    
    battery_pack_info->sample_result.ave_cell_temperature = (U8)(sum_temperature / connected_T_num);
    battery_pack_info->sample_result.diff_cell_temperature = (U8)(battery_pack_info->sample_result.max_cell_temperature - 
																battery_pack_info->sample_result.min_cell_temperature); 

	/* copy the related data to log data */
	Copy_To_Pack_Data_Struct(log_data_t,
							 battery_pack_info);

#ifndef PRODUCTION_TEST
	#ifdef VERSION_BMS
	    printf("Process\n");
	#endif
#endif
					
}



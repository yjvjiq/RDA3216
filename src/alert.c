#include "alert.h"
#include "timer.h"
#include "types.h"
#include "GPIO.h"

//typedef enum {
//    ErrIndex_Isolation = 0,         /* 0 */
//    ErrIndex_Precharge,             /* 1 */
//    ErrIndex_PositiveRly,           /* 2 */
//    ErrIndex_NegetiveRly,           /* 3 */
//    ErrIndex_PrechargeRly,          /* 4 */
//    ErrIndex_ChargeRly,             /* 5 */
//    ErrIndex_MinCellTemperature,    /* 6 */
//    ErrIndex_MaxCellTemperature,    /* 7 */
//    ErrIndex_CellTemperatureDiff,   /* 8 */
//    ErrIndex_OverChargeCurrent,     /* 9 */
//    ErrIndex_OverDischargeCurrent,  /* 10 */
//    ErrIndex_MaxCellVoltage,        /* 11 */
//    ErrIndex_MinCellVoltage,        /* 12 */
//    ErrIndex_MaxCellVoltageDiff,    /* 13 */
//    ErrIndex_MaxTotalVoltage,       /* 14 */
//    ErrIndex_MinTotalVoltage,       /* 15 */
//    ErrIndex_LowSOC,                /* 16 */
//    ErrIndex_HighSOC,               /* 17 */
//    ErrIndex_BMS_Malfunction,       /* 18 */
//    ErrIndex_VCU_Communication,     /* 19 */
//    ErrIndex_Charger_Communication, /* 20 */
//    ErrIndex_CurrSensor,            /* 21 */
//    ErrIndex_TempSensor,            /* 22 */
//    ErrIndex_MaxErrNum
//} ErrIndex;

// cell_V range = 3.421 V ~ 4.132 V
const U16 default_err_table_lv1[ErrIndex_MaxErrNum] = {
/*0~4*/		39250,		0,		0,		0,		0,	
/*5~9*/		0,			45,		95,		5,		160,
/*10~14*/	400,		4130,	3500,	100,	784,	
/*15~19*/	665,		20,		90,		0,		0,
/*20~24*/	0,			0,		0
};

const U16 default_err_table_lv2[ErrIndex_MaxErrNum] = {
/*0~4*/		7850,		0,		0,		0,		0,	
/*5~9*/		0,			30,		100,	10,		180,
/*10~14*/	600,		4150,	3420,	150,	788,	
/*15~19*/	650,		15,		95,		0,		0,
/*20~24*/	0,			0,		0
};

const U16 default_err_table_lv3[ErrIndex_MaxErrNum] = {
/*0~4*/		7850,		0,		0,		0,		0,	
/*5~9*/		0,			25,		110,	10,		200,
/*10~14*/	800,		4180,	3410,	200,	790,	
/*15~19*/	648,		10,		98,		0,		0,
/*20~24*/	0,			0,		0
};

const U16 default_err_delay_time_lv1[ErrIndex_MaxErrNum] = {
/*0~4*/		2000,		2000,		2000,		2000,		2000,	
/*5~9*/		2000,		2000,		2000,		2000,		2000,
/*10~14*/	2000,		2000,		2000,		2000,		2000,	
/*15~19*/	2000,		2000,		2000,		2000,		2000,
/*20~24*/	2000,		2000,		2000
};

const U16 default_err_delay_time_lv2[ErrIndex_MaxErrNum] = {
/*0~4*/		2000,		2000,		2000,		2000,		2000,	
/*5~9*/		2000,		2000,		2000,		2000,		2000,
/*10~14*/	2000,		2000,		2000,		2000,		2000,	
/*15~19*/	2000,		2000,		2000,		2000,		2000,
/*20~24*/	2000,		2000,		2000
};

const U16 default_err_delay_time_lv3[ErrIndex_MaxErrNum] = {
/*0~4*/		2000,		2000,		2000,		2000,		2000,	
/*5~9*/		2000,		2000,		2000,		2000,		2000,
/*10~14*/	2000,		2000,		2000,		2000,		2000,	
/*15~19*/	2000,		2000,		2000,		2000,		2000,
/*20~24*/	2000,		2000,		2000
};

const U16 default_err_rec_value_lv1[ErrIndex_MaxErrNum] = {
/*0~4*/		40000,		0,		0,		0,		0,	
/*5~9*/		0,			47,		93,		3,		140,
/*10~14*/	350,		4125,	3510,	95,		780,	
/*15~19*/	670,		25,		90,		0,		0,
/*20~24*/	0,			0,		0
};

const U16 default_err_rec_value_lv2[ErrIndex_MaxErrNum] = {
/*0~4*/		9000,		0,		0,		0,		0,	
/*5~9*/		0,			32,		97,		7,		170,
/*10~14*/	550,		4145,	3430,	110,	786,	
/*15~19*/	655,		20,		95,		0,		0,
/*20~24*/	0,			0,		0
};

const U16 default_err_rec_value_lv3[ErrIndex_MaxErrNum] = {
/*0~4*/		9000,		0,		0,		0,		0,	
/*5~9*/		0,			27,		105,	13,		190,
/*10~14*/	750,		4175,	3415,	180,	789,	
/*15~19*/	649,		15,		98,		0,		0,
/*20~24*/	0,			0,		0
};

const U16 default_err_rec_time_lv1[ErrIndex_MaxErrNum] = {
/*0~4*/		2000,		2000,		2000,		2000,		2000,	
/*5~9*/		2000,		2000,		2000,		2000,		2000,
/*10~14*/	2000,		2000,		2000,		2000,		2000,	
/*15~19*/	2000,		2000,		2000,		2000,		2000,
/*20~24*/	2000,		2000,		2000
};

const U16 default_err_rec_time_lv2[ErrIndex_MaxErrNum] = {
/*0~4*/		2000,		2000,		2000,		2000,		2000,	
/*5~9*/		2000,		2000,		2000,		2000,		2000,
/*10~14*/	2000,		2000,		2000,		2000,		2000,	
/*15~19*/	2000,		2000,		2000,		2000,		2000,
/*20~24*/	2000,		2000,		2000
};

const U16 default_err_rec_time_lv3[ErrIndex_MaxErrNum] = {
/*0~4*/		2000,		2000,		2000,		2000,		2000,	
/*5~9*/		2000,		2000,		2000,		2000,		2000,
/*10~14*/	2000,		2000,		2000,		2000,		2000,	
/*15~19*/	2000,		2000,		2000,		2000,		2000,
/*20~24*/	2000,		2000,		2000
};

#ifdef VERSION_BTT
/* v_cell diff too high alert when at differenct V_pack range */
const U16 Default_V_Cell_Diff_Err[4][3] = { {0xFFFF, 0xFFFF,200},	/* V_pack <= 64.5V */
											{100, 150, 200},		/* 64.5 < V_pack <= 67 */
											{100, 150, 200},		/* 67 < V_pack <= 71 */
											{100, 150, 200}};		/* 71 < V_pack <= 79.8 */

#endif

/**************************************************************
* load cell config data to variable in RAM, from flash read 
***************************************************************/
void Load_Default_Cell_Config(Cell_Config_Record *error_table_config_record){
	U8 i;
	
	for(i=0;i<ErrIndex_MaxErrNum;i++){
		/* error table value config */
		error_table_config_record->error_config_table_lv1[i] = default_err_table_lv1[i];
		
		error_table_config_record->error_config_table_lv2[i] = default_err_table_lv2[i];
		
		error_table_config_record->error_config_table_lv3[i] = default_err_table_lv3[i];
		
		/* errpr recovery value table */
		error_table_config_record->error_config_rec_value_lv1[i] = default_err_rec_value_lv1[i];
		
		error_table_config_record->error_config_rec_value_lv2[i] = default_err_rec_value_lv2[i];
		
		error_table_config_record->error_config_rec_value_lv3[i] = default_err_rec_value_lv3[i];
		
		/* error delay time config */
		error_table_config_record->error_config_delay_time_lv1[i] = default_err_delay_time_lv1[i];

		error_table_config_record->error_config_delay_time_lv2[i] = default_err_delay_time_lv2[i];

		error_table_config_record->error_config_delay_time_lv3[i] = default_err_delay_time_lv3[i];

		/* error recovery delay time config */
		error_table_config_record->error_config_rec_time_lv1[i] = default_err_rec_time_lv1[i];

		error_table_config_record->error_config_rec_time_lv2[i] = default_err_rec_time_lv2[i];

		error_table_config_record->error_config_rec_time_lv3[i] = default_err_rec_time_lv3[i];
	}
}

/**
 * error counter for error judgement
 */
void Err_Counter(U16* cnt){
	if(*cnt < U16_MAX - DATA_PROCESS_MS) {
		*cnt += DATA_PROCESS_MS;
	} else {
		*cnt = U16_MAX;
	}
}


/**
* Alert_Main(...)
*/
void Alert_Main(Battery_Pack_Info  *battery_pack_info,
				Error_Counter *error_counter,
				Cell_Config_Record	*error_table_config_record){
#ifdef VERSION_BTT
	U8 V_pack_level = 0xff;
//	Load_Default_Cell_Config(&g_static_data);
	
#endif
	
	Load_Default_Cell_Config(&g_static_data.static_data.cell_config_record);
	
    /*********************************************************************************************************************/
    /*  error counter and flag                                                                                           */
    /*********************************************************************************************************************/    
	/* total voltage high */
	if((battery_pack_info->sample_data.total_cell_voltage/10) >= error_table_config_record->error_config_table_lv3[ErrIndex_MaxTotalVoltage]){
		Err_Counter(&error_counter->error_count_lv3[ErrIndex_MaxTotalVoltage]);
		if(error_counter->error_count_lv3[ErrIndex_MaxTotalVoltage] >= 
			error_table_config_record->error_config_delay_time_lv3[ErrIndex_MaxTotalVoltage]){
			battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt = 3;
			error_counter->error_count_lv3[ErrIndex_MaxTotalVoltage] = 0;
		}
	}
	else if((battery_pack_info->sample_data.total_cell_voltage/10) >= error_table_config_record->error_config_table_lv2[ErrIndex_MaxTotalVoltage]){
		Err_Counter(&error_counter->error_count_lv2[ErrIndex_MaxTotalVoltage]);
		error_counter->error_count_lv3[ErrIndex_MaxTotalVoltage] = 0;
		
		if((battery_pack_info->sample_data.total_cell_voltage/10) >= error_table_config_record->error_config_rec_value_lv3[ErrIndex_MaxTotalVoltage]){
			Err_Counter(&error_counter->error_rec_count_lv3[ErrIndex_MaxTotalVoltage]);
			if(battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt > 2){
				if(error_counter->error_rec_count_lv3[ErrIndex_MaxTotalVoltage] >= error_table_config_record->error_config_rec_time_lv3[ErrIndex_MaxTotalVoltage]
					&& error_counter->error_count_lv2[ErrIndex_MaxTotalVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxTotalVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt = 2;
					error_counter->error_count_lv2[ErrIndex_MaxTotalVoltage] = 0;
					error_counter->error_rec_count_lv3[ErrIndex_MaxTotalVoltage] = 0;
				}
			}
			else if(error_counter->error_count_lv2[ErrIndex_MaxTotalVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxTotalVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MaxTotalVoltage] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MaxTotalVoltage] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv2[ErrIndex_MaxTotalVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxTotalVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MaxTotalVoltage] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MaxTotalVoltage] = 0;
			}
		}
	}
	else if((battery_pack_info->sample_data.total_cell_voltage/10) >= error_table_config_record->error_config_table_lv1[ErrIndex_MaxTotalVoltage]){
		Err_Counter(&error_counter->error_count_lv1[ErrIndex_MaxTotalVoltage]);
		error_counter->error_count_lv2[ErrIndex_MaxTotalVoltage] = 0;
		error_counter->error_count_lv3[ErrIndex_MaxTotalVoltage] = 0;
		error_counter->error_rec_count_lv3[ErrIndex_MaxTotalVoltage] = 0;
		
		if((battery_pack_info->sample_data.total_cell_voltage/10) >= error_table_config_record->error_config_rec_value_lv2[ErrIndex_MaxTotalVoltage]){
			Err_Counter(&error_counter->error_rec_count_lv2[ErrIndex_MaxTotalVoltage]);
			if(battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt > 1){
				if(error_counter->error_rec_count_lv2[ErrIndex_MaxTotalVoltage] >= error_table_config_record->error_config_rec_time_lv2[ErrIndex_MaxTotalVoltage]
					&&error_counter->error_count_lv1[ErrIndex_MaxTotalVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxTotalVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt = 1;
					error_counter->error_count_lv1[ErrIndex_MaxTotalVoltage] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_MaxTotalVoltage] = 0;
				}
			}
			else if(error_counter->error_count_lv1[ErrIndex_MaxTotalVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxTotalVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MaxTotalVoltage] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MaxTotalVoltage] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv1[ErrIndex_MaxTotalVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxTotalVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MaxTotalVoltage] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MaxTotalVoltage] = 0;
			}
		}
	}
	else{
		error_counter->error_count_lv3[ErrIndex_MaxTotalVoltage] = 0;
		error_counter->error_count_lv2[ErrIndex_MaxTotalVoltage] = 0;
		error_counter->error_count_lv1[ErrIndex_MaxTotalVoltage] = 0;
		
		if((battery_pack_info->sample_data.total_cell_voltage/10) >= error_table_config_record->error_config_rec_value_lv1[ErrIndex_MaxTotalVoltage]){
			if(battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt > 0){
				Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_MaxTotalVoltage]);
				if(error_counter->error_rec_count_lv1[ErrIndex_MaxTotalVoltage] >= error_table_config_record->error_config_rec_time_lv1[ErrIndex_MaxTotalVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt = 0;
					error_counter->error_rec_count_lv1[ErrIndex_MaxTotalVoltage] = 0;
				}
			}
			else{
				battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt = 0;
				error_counter->error_rec_count_lv1[ErrIndex_MaxTotalVoltage] = 0;
			}
		}
		else{
			battery_pack_info->error_flag.mergedBits.BMS_PackHVoltFlt = 0;
			error_counter->error_rec_count_lv1[ErrIndex_MaxTotalVoltage] = 0;
		}
	}	

	/* total voltage low */
	if((battery_pack_info->sample_data.total_cell_voltage/10) <= error_table_config_record->error_config_table_lv3[ErrIndex_MinTotalVoltage]){
		Err_Counter(&error_counter->error_count_lv3[ErrIndex_MinTotalVoltage]);
		
		if(error_counter->error_count_lv3[ErrIndex_MinTotalVoltage] >= 
			error_table_config_record->error_config_delay_time_lv3[ErrIndex_MinTotalVoltage]){
			battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt = 3;
			error_counter->error_count_lv3[ErrIndex_MinTotalVoltage] = 0;
		}
	}
	else if((battery_pack_info->sample_data.total_cell_voltage/10) <= error_table_config_record->error_config_table_lv2[ErrIndex_MinTotalVoltage]){
		Err_Counter(&error_counter->error_count_lv2[ErrIndex_MinTotalVoltage]);
		error_counter->error_count_lv3[ErrIndex_MinTotalVoltage] = 0;
		
		if((battery_pack_info->sample_data.total_cell_voltage/10) <= error_table_config_record->error_config_rec_value_lv3[ErrIndex_MinTotalVoltage]){
			Err_Counter(&error_counter->error_rec_count_lv3[ErrIndex_MinTotalVoltage]);
			if(battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt > 2){
				if(error_counter->error_rec_count_lv3[ErrIndex_MinTotalVoltage] >= error_table_config_record->error_config_rec_time_lv3[ErrIndex_MinTotalVoltage]
					&& error_counter->error_count_lv2[ErrIndex_MinTotalVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MinTotalVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt = 2;
					error_counter->error_count_lv2[ErrIndex_MinTotalVoltage] = 0;
					error_counter->error_rec_count_lv3[ErrIndex_MinTotalVoltage] = 0;
				}
			}
			else if(error_counter->error_count_lv2[ErrIndex_MinTotalVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MinTotalVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MinTotalVoltage] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MinTotalVoltage] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv2[ErrIndex_MinTotalVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MinTotalVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MinTotalVoltage] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MinTotalVoltage] = 0;
			}
		}
	}
	else if((battery_pack_info->sample_data.total_cell_voltage/10) <= error_table_config_record->error_config_table_lv1[ErrIndex_MinTotalVoltage]){
		Err_Counter(&error_counter->error_count_lv1[ErrIndex_MinTotalVoltage]);
		error_counter->error_count_lv2[ErrIndex_MinTotalVoltage] = 0;
		error_counter->error_count_lv3[ErrIndex_MinTotalVoltage] = 0;
		
		if((battery_pack_info->sample_data.total_cell_voltage/10) <= error_table_config_record->error_config_rec_value_lv2[ErrIndex_MinTotalVoltage]){
			Err_Counter(&error_counter->error_rec_count_lv2[ErrIndex_MinTotalVoltage]);
			if(battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt > 1){
				if(error_counter->error_rec_count_lv2[ErrIndex_MinTotalVoltage] >= error_table_config_record->error_config_rec_time_lv2[ErrIndex_MinTotalVoltage]
					&& error_counter->error_count_lv1[ErrIndex_MinTotalVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MinTotalVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt = 1;
					error_counter->error_count_lv1[ErrIndex_MinTotalVoltage] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_MinTotalVoltage] = 0;
				}
			}
			else if(error_counter->error_count_lv1[ErrIndex_MinTotalVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MinTotalVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MinTotalVoltage] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MinTotalVoltage] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv1[ErrIndex_MinTotalVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MinTotalVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MinTotalVoltage] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MinTotalVoltage] = 0;
			}
		}
	}
	else{
		error_counter->error_count_lv3[ErrIndex_MinTotalVoltage] = 0;
		error_counter->error_count_lv2[ErrIndex_MinTotalVoltage] = 0;
		error_counter->error_count_lv1[ErrIndex_MinTotalVoltage] = 0;
		
		if((battery_pack_info->sample_data.total_cell_voltage/10) <= error_table_config_record->error_config_rec_value_lv1[ErrIndex_MinTotalVoltage]){
			if(battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt > 0){
				Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_MinTotalVoltage]);
				if(error_counter->error_rec_count_lv1[ErrIndex_MinTotalVoltage] >= error_table_config_record->error_config_rec_time_lv1[ErrIndex_MinTotalVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt = 0;
					error_counter->error_rec_count_lv1[ErrIndex_MinTotalVoltage] = 0;
				}
			}
			else{
				battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt = 0;
				error_counter->error_rec_count_lv1[ErrIndex_MinTotalVoltage] = 0;
			}
		}
		else{
			battery_pack_info->error_flag.mergedBits.BMS_PackLVoltFlt = 0;
			error_counter->error_rec_count_lv1[ErrIndex_MinTotalVoltage] = 0;
		}
	}
	
	/* cell voltage too high */
	if(battery_pack_info->sample_result.max_cell_voltage >= error_table_config_record->error_config_table_lv3[ErrIndex_MaxCellVoltage]){
		Err_Counter(&error_counter->error_count_lv3[ErrIndex_MaxCellVoltage]);
		if(error_counter->error_count_lv3[ErrIndex_MaxCellVoltage] >= 
			error_table_config_record->error_config_delay_time_lv3[ErrIndex_MaxCellVoltage]){
			battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt = 3;
			error_counter->error_count_lv3[ErrIndex_MaxCellVoltage] = 0;
		}
	}
	else if(battery_pack_info->sample_result.max_cell_voltage >= error_table_config_record->error_config_table_lv2[ErrIndex_MaxCellVoltage]){
		Err_Counter(&error_counter->error_count_lv2[ErrIndex_MaxCellVoltage]);
		error_counter->error_count_lv3[ErrIndex_MaxCellVoltage] = 0;
		
		if(battery_pack_info->sample_result.max_cell_voltage >= error_table_config_record->error_config_rec_value_lv3[ErrIndex_MaxCellVoltage]){
			if(battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt > 2){
				Err_Counter(&error_counter->error_rec_count_lv3[ErrIndex_MaxCellVoltage]);
				if(error_counter->error_rec_count_lv3[ErrIndex_MaxCellVoltage] >= error_table_config_record->error_config_rec_time_lv3[ErrIndex_MaxCellVoltage]
					&& error_counter->error_count_lv2[ErrIndex_MaxCellVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxCellVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt = 2;
					error_counter->error_count_lv2[ErrIndex_MaxCellVoltage] = 0;
					error_counter->error_rec_count_lv3[ErrIndex_MaxCellVoltage] = 0;
				}
			}
			else if(error_counter->error_count_lv2[ErrIndex_MaxCellVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxCellVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MaxCellVoltage] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MaxCellVoltage] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv2[ErrIndex_MaxCellVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxCellVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MaxCellVoltage] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MaxCellVoltage] = 0;
			}
		}
	}
	else if(battery_pack_info->sample_result.max_cell_voltage >= error_table_config_record->error_config_table_lv1[ErrIndex_MaxCellVoltage]){
		Err_Counter(&error_counter->error_count_lv1[ErrIndex_MaxCellVoltage]);
		error_counter->error_count_lv2[ErrIndex_MaxCellVoltage] = 0;
		error_counter->error_count_lv3[ErrIndex_MaxCellVoltage] = 0;
		
		if(battery_pack_info->sample_result.max_cell_voltage >= error_table_config_record->error_config_rec_value_lv2[ErrIndex_MaxCellVoltage]){
			if(battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt > 1){
				Err_Counter(&error_counter->error_rec_count_lv2[ErrIndex_MaxCellVoltage]);
				if(error_counter->error_rec_count_lv2[ErrIndex_MaxCellVoltage] >= error_table_config_record->error_config_rec_time_lv2[ErrIndex_MaxCellVoltage]
					&& error_counter->error_count_lv1[ErrIndex_MaxCellVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxCellVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt = 1;
					error_counter->error_count_lv1[ErrIndex_MaxCellVoltage] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_MaxCellVoltage] = 0;
				}
			}
			else if(error_counter->error_count_lv1[ErrIndex_MaxCellVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxCellVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MaxCellVoltage] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MaxCellVoltage] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv1[ErrIndex_MaxCellVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxCellVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MaxCellVoltage] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MaxCellVoltage] = 0;
			}
		}
	}
	else{
		error_counter->error_count_lv3[ErrIndex_MaxCellVoltage] = 0;
		error_counter->error_count_lv2[ErrIndex_MaxCellVoltage] = 0;
		error_counter->error_count_lv1[ErrIndex_MaxCellVoltage] = 0;
		if(battery_pack_info->sample_result.max_cell_voltage >= error_table_config_record->error_config_rec_value_lv1[ErrIndex_MaxCellVoltage]){
			if(battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt > 0){
				Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_MaxCellVoltage]);
				if(error_counter->error_rec_count_lv1[ErrIndex_MaxCellVoltage] >= error_table_config_record->error_config_rec_time_lv1[ErrIndex_MaxCellVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt = 0;
					error_counter->error_rec_count_lv1[ErrIndex_MaxCellVoltage] = 0;
				}
			}
			else{
				battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt = 0;
				error_counter->error_rec_count_lv1[ErrIndex_MaxCellVoltage] = 0;
			}
		}
		else{
			battery_pack_info->error_flag.mergedBits.BMS_CellHVoltFlt = 0;
			error_counter->error_rec_count_lv1[ErrIndex_MaxCellVoltage] = 0;
		}
	}
	
	/* cell voltage low */
	if(battery_pack_info->sample_result.min_cell_voltage <= error_table_config_record->error_config_table_lv3[ErrIndex_MinCellVoltage]){
		Err_Counter(&error_counter->error_count_lv3[ErrIndex_MinCellVoltage]);
		if(error_counter->error_count_lv3[ErrIndex_MinCellVoltage] >= 
			error_table_config_record->error_config_delay_time_lv3[ErrIndex_MinCellVoltage]){
			battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt = 3;
			error_counter->error_count_lv3[ErrIndex_MinCellVoltage] = 0;
		}
	}
	else if(battery_pack_info->sample_result.min_cell_voltage <= error_table_config_record->error_config_table_lv2[ErrIndex_MinCellVoltage]){
		Err_Counter(&error_counter->error_count_lv2[ErrIndex_MinCellVoltage]);
		error_counter->error_count_lv3[ErrIndex_MinCellVoltage] = 0;
		
		if(battery_pack_info->sample_result.min_cell_voltage <= error_table_config_record->error_config_rec_value_lv3[ErrIndex_MinCellVoltage]){
			if(battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt > 2){
				Err_Counter(&error_counter->error_rec_count_lv3[ErrIndex_MinCellVoltage]);
				if(error_counter->error_rec_count_lv3[ErrIndex_MinCellVoltage] >= error_table_config_record->error_config_rec_time_lv3[ErrIndex_MinCellVoltage]
					&& error_counter->error_count_lv2[ErrIndex_MinCellVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MinCellVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt = 2;
					error_counter->error_count_lv2[ErrIndex_MinCellVoltage] = 0;
					error_counter->error_rec_count_lv3[ErrIndex_MinCellVoltage] = 0;
				}
			}
			else if(error_counter->error_count_lv2[ErrIndex_MinCellVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MinCellVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MinCellVoltage] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MinCellVoltage] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv2[ErrIndex_MinCellVoltage] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MinCellVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MinCellVoltage] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MinCellVoltage] = 0;
			}
		}
	}
	else if(battery_pack_info->sample_result.min_cell_voltage <= error_table_config_record->error_config_table_lv1[ErrIndex_MinCellVoltage]){
		Err_Counter(&error_counter->error_count_lv1[ErrIndex_MinCellVoltage]);
		error_counter->error_count_lv2[ErrIndex_MinCellVoltage] = 0;
		error_counter->error_count_lv3[ErrIndex_MinCellVoltage] = 0;
		
		if(battery_pack_info->sample_result.min_cell_voltage <= error_table_config_record->error_config_rec_value_lv2[ErrIndex_MinCellVoltage]){
			if(battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt > 1){
				Err_Counter(&error_counter->error_rec_count_lv2[ErrIndex_MinCellVoltage]);
				if(error_counter->error_rec_count_lv2[ErrIndex_MinCellVoltage] >= error_table_config_record->error_config_rec_time_lv2[ErrIndex_MinCellVoltage]
					&& error_counter->error_count_lv1[ErrIndex_MinCellVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MinCellVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt = 1;
					error_counter->error_count_lv1[ErrIndex_MinCellVoltage] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_MinCellVoltage] = 0;
				}
			}
			else if(error_counter->error_count_lv1[ErrIndex_MinCellVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MinCellVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MinCellVoltage] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MinCellVoltage] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv1[ErrIndex_MinCellVoltage] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MinCellVoltage]){
				battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MinCellVoltage] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MinCellVoltage] = 0;
			}
		}
	}
	else{
		error_counter->error_count_lv3[ErrIndex_MinCellVoltage] = 0;
		error_counter->error_count_lv2[ErrIndex_MinCellVoltage] = 0;
		error_counter->error_count_lv1[ErrIndex_MinCellVoltage] = 0;
		if(battery_pack_info->sample_result.min_cell_voltage <= error_table_config_record->error_config_rec_value_lv1[ErrIndex_MinCellVoltage]){
			if(battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt > 0){
				Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_MinCellVoltage]);
				if(error_counter->error_rec_count_lv1[ErrIndex_MinCellVoltage] >= error_table_config_record->error_config_rec_time_lv1[ErrIndex_MinCellVoltage]){
					battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt = 0;
					error_counter->error_rec_count_lv1[ErrIndex_MinCellVoltage] = 0;
				}
			}
			else{
				battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt = 0;
				error_counter->error_rec_count_lv1[ErrIndex_MinCellVoltage] = 0;
			}
		}
		else{
			battery_pack_info->error_flag.mergedBits.BMS_CellLVoltFlt = 0;
			error_counter->error_rec_count_lv1[ErrIndex_MinCellVoltage] = 0;
		}
	}
	
	/* cell voltage difference high */
	if(battery_pack_info->sample_result.diff_cell_voltage >= error_table_config_record->error_config_table_lv3[ErrIndex_MaxCellVoltageDiff]){
		Err_Counter(&error_counter->error_count_lv3[ErrIndex_MaxCellVoltageDiff]);
		if(error_counter->error_count_lv3[ErrIndex_MaxCellVoltageDiff] >= 
			error_table_config_record->error_config_delay_time_lv3[ErrIndex_MaxCellVoltageDiff]){
			battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 3;
			error_counter->error_count_lv3[ErrIndex_MaxCellVoltageDiff] = 0;
		}
	}
	else if(battery_pack_info->sample_result.diff_cell_voltage >= error_table_config_record->error_config_table_lv2[ErrIndex_MaxCellVoltageDiff]){
		Err_Counter(&error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff]);
		error_counter->error_count_lv3[ErrIndex_MaxCellVoltageDiff] = 0;
		
		if(battery_pack_info->sample_result.diff_cell_voltage >= error_table_config_record->error_config_rec_value_lv3[ErrIndex_MaxCellVoltageDiff]){
			if(battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt > 2){
				Err_Counter(&error_counter->error_rec_count_lv3[ErrIndex_MaxCellVoltageDiff]);
				if(error_counter->error_rec_count_lv3[ErrIndex_MaxCellVoltageDiff] >= error_table_config_record->error_config_rec_time_lv3[ErrIndex_MaxCellVoltageDiff]
					&& error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxCellVoltageDiff]){
					battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 2;
					error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff] = 0;
					error_counter->error_rec_count_lv3[ErrIndex_MaxCellVoltageDiff] = 0;
				}
			}
			else if(error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxCellVoltageDiff]){
				battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MaxCellVoltageDiff] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxCellVoltageDiff]){
				battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MaxCellVoltageDiff] = 0;
			}
		}
	}
	else if(battery_pack_info->sample_result.diff_cell_voltage >= error_table_config_record->error_config_table_lv1[ErrIndex_MaxCellVoltageDiff]){
		Err_Counter(&error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff]);
		error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff] = 0;
		error_counter->error_count_lv3[ErrIndex_MaxCellVoltageDiff] = 0;
		
		if(battery_pack_info->sample_result.diff_cell_voltage >= error_table_config_record->error_config_rec_value_lv2[ErrIndex_MaxCellVoltageDiff]){
			if(battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt > 1){
				Err_Counter(&error_counter->error_rec_count_lv2[ErrIndex_MaxCellVoltageDiff]);
				if(error_counter->error_rec_count_lv2[ErrIndex_MaxCellVoltageDiff] >= error_table_config_record->error_config_rec_time_lv2[ErrIndex_MaxCellVoltageDiff]
					&& error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxCellVoltageDiff]){
					battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 1;
					error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_MaxCellVoltageDiff] = 0;
				}
			}
			else if(error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxCellVoltageDiff]){
				battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MaxCellVoltageDiff] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxCellVoltageDiff]){
				battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MaxCellVoltageDiff] = 0;
			}
		}
	}
	else{
		error_counter->error_count_lv3[ErrIndex_MaxCellVoltageDiff] = 0;
		error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff] = 0;
		error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff] = 0;
		
		if(battery_pack_info->sample_result.diff_cell_voltage >= error_table_config_record->error_config_rec_value_lv1[ErrIndex_MaxCellVoltageDiff]){
			if(battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt > 0){
				Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_MaxCellVoltageDiff]);
				if(error_counter->error_rec_count_lv1[ErrIndex_MaxCellVoltageDiff] >= error_table_config_record->error_config_rec_time_lv1[ErrIndex_MaxCellVoltageDiff]){
					battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 0;
					error_counter->error_rec_count_lv1[ErrIndex_MaxCellVoltageDiff] = 0;
				}
			}
			else{
				battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 0;
				error_counter->error_rec_count_lv1[ErrIndex_MaxCellVoltageDiff] = 0;
			}
		}
		else{
			battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 0;
			error_counter->error_rec_count_lv1[ErrIndex_MaxCellVoltageDiff] = 0;
		}
	}
	
	/* total charge current too high */
	if(battery_pack_info->sample_data.current_direction == 0){
		if((5000 - battery_pack_info->sample_data.total_current) >= error_table_config_record->error_config_table_lv3[ErrIndex_OverChargeCurrent]){
			Err_Counter(&error_counter->error_count_lv3[ErrIndex_OverChargeCurrent]);
			if(error_counter->error_count_lv3[ErrIndex_OverChargeCurrent] >= 
				error_table_config_record->error_config_delay_time_lv3[ErrIndex_OverChargeCurrent]){
				battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt = 3;
				error_counter->error_count_lv3[ErrIndex_OverChargeCurrent] = 0;
			}
		}
		else if((5000 - battery_pack_info->sample_data.total_current) >= error_table_config_record->error_config_table_lv2[ErrIndex_OverChargeCurrent]){
			Err_Counter(&error_counter->error_count_lv2[ErrIndex_OverChargeCurrent]);
			error_counter->error_count_lv3[ErrIndex_OverChargeCurrent] = 0;
			
			if((5000 - battery_pack_info->sample_data.total_current) >= error_table_config_record->error_config_rec_value_lv3[ErrIndex_OverChargeCurrent]){
				if(battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt > 2){
					Err_Counter(&error_counter->error_rec_count_lv3[ErrIndex_OverChargeCurrent]);
					if(error_counter->error_rec_count_lv3[ErrIndex_OverChargeCurrent] >= error_table_config_record->error_config_rec_time_lv3[ErrIndex_OverChargeCurrent]
						&& error_counter->error_count_lv2[ErrIndex_OverChargeCurrent] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_OverChargeCurrent]){
						battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt = 2;
						error_counter->error_count_lv2[ErrIndex_OverChargeCurrent] = 0;
						error_counter->error_rec_count_lv3[ErrIndex_OverChargeCurrent] = 0;
					}
				}
				else if(error_counter->error_count_lv2[ErrIndex_OverChargeCurrent] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_OverChargeCurrent]){
					battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt = 2;
					error_counter->error_count_lv2[ErrIndex_OverChargeCurrent] = 0;
					error_counter->error_rec_count_lv3[ErrIndex_OverChargeCurrent] = 0;
				}
			}
			else{
				if(error_counter->error_count_lv2[ErrIndex_OverChargeCurrent] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_OverChargeCurrent]){
					battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt = 2;
					error_counter->error_count_lv2[ErrIndex_OverChargeCurrent] = 0;
					error_counter->error_rec_count_lv3[ErrIndex_OverChargeCurrent] = 0;
				}
			}
		}
		else if((5000 - battery_pack_info->sample_data.total_current) >= error_table_config_record->error_config_table_lv1[ErrIndex_OverChargeCurrent]){
			Err_Counter(&error_counter->error_count_lv1[ErrIndex_OverChargeCurrent]);
			error_counter->error_count_lv2[ErrIndex_OverChargeCurrent] = 0;
			error_counter->error_count_lv3[ErrIndex_OverChargeCurrent] = 0;
			
			if((5000 - battery_pack_info->sample_data.total_current) >= error_table_config_record->error_config_rec_value_lv2[ErrIndex_OverChargeCurrent]){
				if(battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt > 1){
					Err_Counter(&error_counter->error_rec_count_lv2[ErrIndex_OverChargeCurrent]);
					if(error_counter->error_rec_count_lv2[ErrIndex_OverChargeCurrent] >= error_table_config_record->error_config_rec_time_lv2[ErrIndex_OverChargeCurrent]
						&& error_counter->error_count_lv1[ErrIndex_OverChargeCurrent] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_OverChargeCurrent]){
						battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt = 1;
						error_counter->error_count_lv1[ErrIndex_OverChargeCurrent] = 0;
						error_counter->error_rec_count_lv2[ErrIndex_OverChargeCurrent] = 0;
					}
				}
				else if(error_counter->error_count_lv1[ErrIndex_OverChargeCurrent] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_OverChargeCurrent]){
					battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt = 1;
					error_counter->error_count_lv1[ErrIndex_OverChargeCurrent] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_OverChargeCurrent] = 0;
				}
			}
			else{
				if(error_counter->error_count_lv1[ErrIndex_OverChargeCurrent] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_OverChargeCurrent]){
					battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt = 1;
					error_counter->error_count_lv1[ErrIndex_OverChargeCurrent] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_OverChargeCurrent] = 0;
				}
			}
		}
		else{
			error_counter->error_count_lv3[ErrIndex_OverChargeCurrent] = 0;
			error_counter->error_count_lv2[ErrIndex_OverChargeCurrent] = 0;
			error_counter->error_count_lv1[ErrIndex_OverChargeCurrent] = 0;
			if((5000 - battery_pack_info->sample_data.total_current) >= error_table_config_record->error_config_rec_value_lv1[ErrIndex_OverChargeCurrent]){
				if(battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt > 0){
					Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_OverChargeCurrent]);
					if(error_counter->error_rec_count_lv1[ErrIndex_OverChargeCurrent] >= error_table_config_record->error_config_rec_time_lv1[ErrIndex_OverChargeCurrent]){
						battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt = 0;
						error_counter->error_rec_count_lv1[ErrIndex_OverChargeCurrent] = 0;
					}
				}
				else{
					battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt = 0;
					error_counter->error_rec_count_lv1[ErrIndex_OverChargeCurrent] = 0;
				}
			}
			else{
				battery_pack_info->error_flag.mergedBits.BMS_OvrChgFlt = 0;
				error_counter->error_rec_count_lv1[ErrIndex_OverChargeCurrent] = 0;
			}
		}
	}

	/* total discharge current too high */
	if(battery_pack_info->sample_data.current_direction == 1){
		if((battery_pack_info->sample_data.total_current-5000) >= error_table_config_record->error_config_table_lv3[ErrIndex_OverDischargeCurrent]){
			Err_Counter(&error_counter->error_count_lv3[ErrIndex_OverDischargeCurrent]);
			if(error_counter->error_count_lv3[ErrIndex_OverDischargeCurrent] >= 
				error_table_config_record->error_config_delay_time_lv3[ErrIndex_OverDischargeCurrent]){
				battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt = 3;
				error_counter->error_count_lv3[ErrIndex_OverDischargeCurrent] = 0;
			}
		}
		else if((battery_pack_info->sample_data.total_current-5000) >= error_table_config_record->error_config_table_lv2[ErrIndex_OverDischargeCurrent]){
			Err_Counter(&error_counter->error_count_lv2[ErrIndex_OverDischargeCurrent]);
			error_counter->error_count_lv3[ErrIndex_OverDischargeCurrent] = 0;
			
			if((battery_pack_info->sample_data.total_current-5000) >= error_table_config_record->error_config_rec_value_lv3[ErrIndex_OverDischargeCurrent]){
				if(battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt > 2){
					Err_Counter(&error_counter->error_rec_count_lv3[ErrIndex_OverDischargeCurrent]);
					if(error_counter->error_rec_count_lv3[ErrIndex_OverDischargeCurrent] >= error_table_config_record->error_config_rec_time_lv3[ErrIndex_OverDischargeCurrent]
						&& error_counter->error_count_lv2[ErrIndex_OverDischargeCurrent] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_OverDischargeCurrent]){
						battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt = 2;
						error_counter->error_count_lv2[ErrIndex_OverDischargeCurrent] = 0;
						error_counter->error_rec_count_lv3[ErrIndex_OverDischargeCurrent] = 0;
					}
				}
				else if(error_counter->error_count_lv2[ErrIndex_OverDischargeCurrent] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_OverDischargeCurrent]){
					battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt = 2;
					error_counter->error_count_lv2[ErrIndex_OverDischargeCurrent] = 0;
					error_counter->error_rec_count_lv3[ErrIndex_OverDischargeCurrent] = 0;
				}
			}
			else{
				if(error_counter->error_count_lv2[ErrIndex_OverDischargeCurrent] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_OverDischargeCurrent]){
					battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt = 2;
					error_counter->error_count_lv2[ErrIndex_OverDischargeCurrent] = 0;
					error_counter->error_rec_count_lv3[ErrIndex_OverDischargeCurrent] = 0;
				}
			}
		}
		else if((battery_pack_info->sample_data.total_current-5000) >= error_table_config_record->error_config_table_lv1[ErrIndex_OverDischargeCurrent]){
			Err_Counter(&error_counter->error_count_lv1[ErrIndex_OverDischargeCurrent]);
			error_counter->error_count_lv2[ErrIndex_OverDischargeCurrent] = 0;
			error_counter->error_count_lv3[ErrIndex_OverDischargeCurrent] = 0;
			
			if((battery_pack_info->sample_data.total_current-5000) >= error_table_config_record->error_config_rec_value_lv2[ErrIndex_OverDischargeCurrent]){
				if(battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt > 1){
					Err_Counter(&error_counter->error_rec_count_lv2[ErrIndex_OverDischargeCurrent]);
					if(error_counter->error_rec_count_lv2[ErrIndex_OverDischargeCurrent] >= error_table_config_record->error_config_rec_time_lv2[ErrIndex_OverDischargeCurrent]
						&& error_counter->error_count_lv1[ErrIndex_OverDischargeCurrent] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_OverDischargeCurrent]){
						battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt = 1;
						error_counter->error_count_lv1[ErrIndex_OverDischargeCurrent] = 0;
						error_counter->error_rec_count_lv2[ErrIndex_OverDischargeCurrent] = 0;
					}
				}
				else if(error_counter->error_count_lv1[ErrIndex_OverDischargeCurrent] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_OverDischargeCurrent]){
					battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt = 1;
					error_counter->error_count_lv1[ErrIndex_OverDischargeCurrent] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_OverDischargeCurrent] = 0;
				}
			}
			else{
				if(error_counter->error_count_lv1[ErrIndex_OverDischargeCurrent] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_OverDischargeCurrent]){
					battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt = 1;
					error_counter->error_count_lv1[ErrIndex_OverDischargeCurrent] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_OverDischargeCurrent] = 0;
				}
			}
		}
		else{
			error_counter->error_count_lv3[ErrIndex_OverDischargeCurrent] = 0;
			error_counter->error_count_lv2[ErrIndex_OverDischargeCurrent] = 0;
			error_counter->error_count_lv1[ErrIndex_OverDischargeCurrent] = 0;
			if((battery_pack_info->sample_data.total_current-5000) >= error_table_config_record->error_config_rec_value_lv1[ErrIndex_OverDischargeCurrent]){
				if(battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt > 0){
					Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_OverDischargeCurrent]);
					if(error_counter->error_rec_count_lv1[ErrIndex_OverDischargeCurrent] >= error_table_config_record->error_config_rec_time_lv1[ErrIndex_OverDischargeCurrent]){
						battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt = 0;
						error_counter->error_rec_count_lv1[ErrIndex_OverDischargeCurrent] = 0;
					}
				}
				else{
					battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt = 0;
					error_counter->error_rec_count_lv1[ErrIndex_OverDischargeCurrent] = 0;
				}
			}
			else{
				battery_pack_info->error_flag.mergedBits.BMS_OvrDischgFlt = 0;
				error_counter->error_rec_count_lv1[ErrIndex_OverDischargeCurrent] = 0;
			}
		}
	}
	
	/* cell temperature too low */
	if(battery_pack_info->sample_result.min_cell_temperature <= error_table_config_record->error_config_table_lv3[ErrIndex_MinCellTemperature]){
		Err_Counter(&error_counter->error_count_lv3[ErrIndex_MinCellTemperature]);
		if(error_counter->error_count_lv3[ErrIndex_MinCellTemperature] >= 
			error_table_config_record->error_config_delay_time_lv3[ErrIndex_MinCellTemperature]){
			battery_pack_info->error_flag.mergedBits.BMS_LTempFlt = 3;
			error_counter->error_count_lv3[ErrIndex_MinCellTemperature] = 0;
		}
	}
	else if(battery_pack_info->sample_result.min_cell_temperature <= error_table_config_record->error_config_table_lv2[ErrIndex_MinCellTemperature]){
		Err_Counter(&error_counter->error_count_lv2[ErrIndex_MinCellTemperature]);
		error_counter->error_count_lv3[ErrIndex_MinCellTemperature] = 0;
		
		if(battery_pack_info->sample_result.min_cell_temperature <= error_table_config_record->error_config_rec_value_lv3[ErrIndex_MinCellTemperature]){
			if(battery_pack_info->error_flag.mergedBits.BMS_LTempFlt > 2){
				Err_Counter(&error_counter->error_rec_count_lv3[ErrIndex_MinCellTemperature]);
				if(error_counter->error_rec_count_lv3[ErrIndex_MinCellTemperature] >= error_table_config_record->error_config_rec_time_lv3[ErrIndex_MinCellTemperature]
					&& error_counter->error_count_lv2[ErrIndex_MinCellTemperature] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MinCellTemperature]){
					battery_pack_info->error_flag.mergedBits.BMS_LTempFlt = 2;
					error_counter->error_count_lv2[ErrIndex_MinCellTemperature] = 0;
					error_counter->error_rec_count_lv3[ErrIndex_MinCellTemperature] = 0;
				}
			}
			else if(error_counter->error_count_lv2[ErrIndex_MinCellTemperature] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MinCellTemperature]){
				battery_pack_info->error_flag.mergedBits.BMS_LTempFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MinCellTemperature] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MinCellTemperature] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv2[ErrIndex_MinCellTemperature] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MinCellTemperature]){
				battery_pack_info->error_flag.mergedBits.BMS_LTempFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MinCellTemperature] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MinCellTemperature] = 0;
			}
		}
	}
	else if(battery_pack_info->sample_result.min_cell_temperature <= error_table_config_record->error_config_table_lv1[ErrIndex_MinCellTemperature]){
		Err_Counter(&error_counter->error_count_lv1[ErrIndex_MinCellTemperature]);
		error_counter->error_count_lv2[ErrIndex_MinCellTemperature] = 0;
		error_counter->error_count_lv3[ErrIndex_MinCellTemperature] = 0;
		
		if(battery_pack_info->sample_result.min_cell_temperature <= error_table_config_record->error_config_rec_value_lv2[ErrIndex_MinCellTemperature]){
			if(battery_pack_info->error_flag.mergedBits.BMS_LTempFlt > 1){
				Err_Counter(&error_counter->error_rec_count_lv2[ErrIndex_MinCellTemperature]);
				if(error_counter->error_rec_count_lv2[ErrIndex_MinCellTemperature] >= error_table_config_record->error_config_rec_time_lv2[ErrIndex_MinCellTemperature]
					&& error_counter->error_count_lv1[ErrIndex_MinCellTemperature] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MinCellTemperature]){
					battery_pack_info->error_flag.mergedBits.BMS_LTempFlt = 1;
					error_counter->error_count_lv1[ErrIndex_MinCellTemperature] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_MinCellTemperature] = 0;
				}
			}
			else if(error_counter->error_count_lv1[ErrIndex_MinCellTemperature] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MinCellTemperature]){
				battery_pack_info->error_flag.mergedBits.BMS_LTempFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MinCellTemperature] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MinCellTemperature] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv1[ErrIndex_MinCellTemperature] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MinCellTemperature]){
				battery_pack_info->error_flag.mergedBits.BMS_LTempFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MinCellTemperature] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MinCellTemperature] = 0;
			}
		}
	}
	else{
		error_counter->error_count_lv3[ErrIndex_MinCellTemperature] = 0;
		error_counter->error_count_lv2[ErrIndex_MinCellTemperature] = 0;
		error_counter->error_count_lv1[ErrIndex_MinCellTemperature] = 0;
		if(battery_pack_info->sample_result.min_cell_temperature <= error_table_config_record->error_config_rec_value_lv1[ErrIndex_MinCellTemperature]){
			if(battery_pack_info->error_flag.mergedBits.BMS_LTempFlt > 0){
				Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_MinCellTemperature]);
				if(error_counter->error_rec_count_lv1[ErrIndex_MinCellTemperature] >= error_table_config_record->error_config_rec_time_lv1[ErrIndex_MinCellTemperature]){
					battery_pack_info->error_flag.mergedBits.BMS_LTempFlt = 0;
					error_counter->error_rec_count_lv1[ErrIndex_MinCellTemperature] = 0;
				}
			}
			else{
				battery_pack_info->error_flag.mergedBits.BMS_LTempFlt = 0;
				error_counter->error_rec_count_lv1[ErrIndex_MinCellTemperature] = 0;
			}
		}
		else{
			battery_pack_info->error_flag.mergedBits.BMS_LTempFlt = 0;
			error_counter->error_rec_count_lv1[ErrIndex_MinCellTemperature] = 0;
		}
	}
	
	/* cell temperature too high */
	if(battery_pack_info->sample_result.max_cell_temperature >= error_table_config_record->error_config_table_lv3[ErrIndex_MaxCellTemperature]){
		Err_Counter(&error_counter->error_count_lv3[ErrIndex_MaxCellTemperature]);
		if(error_counter->error_count_lv3[ErrIndex_MaxCellTemperature] >= 
			error_table_config_record->error_config_delay_time_lv3[ErrIndex_MaxCellTemperature]){
			battery_pack_info->error_flag.mergedBits.BMS_HTempFlt = 3;
			error_counter->error_count_lv3[ErrIndex_MaxCellTemperature] = 0;
		}
	}
	else if(battery_pack_info->sample_result.max_cell_temperature >= error_table_config_record->error_config_table_lv2[ErrIndex_MaxCellTemperature]){
		Err_Counter(&error_counter->error_count_lv2[ErrIndex_MaxCellTemperature]);
		error_counter->error_count_lv3[ErrIndex_MaxCellTemperature] = 0;
		
		if(battery_pack_info->sample_result.max_cell_temperature >= error_table_config_record->error_config_rec_value_lv3[ErrIndex_MaxCellTemperature]){
			if(battery_pack_info->error_flag.mergedBits.BMS_HTempFlt > 2){
				Err_Counter(&error_counter->error_rec_count_lv3[ErrIndex_MaxCellTemperature]);
				if(error_counter->error_rec_count_lv3[ErrIndex_MaxCellTemperature] >= error_table_config_record->error_config_rec_time_lv3[ErrIndex_MaxCellTemperature]
					&& error_counter->error_count_lv2[ErrIndex_MaxCellTemperature] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxCellTemperature]){
					battery_pack_info->error_flag.mergedBits.BMS_HTempFlt = 2;
					error_counter->error_count_lv2[ErrIndex_MaxCellTemperature] = 0;
					error_counter->error_rec_count_lv3[ErrIndex_MaxCellTemperature] = 0;
				}
			}
			else if(error_counter->error_count_lv2[ErrIndex_MaxCellTemperature] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxCellTemperature]){
				battery_pack_info->error_flag.mergedBits.BMS_HTempFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MaxCellTemperature] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MaxCellTemperature] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv2[ErrIndex_MaxCellTemperature] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxCellTemperature]){
				battery_pack_info->error_flag.mergedBits.BMS_HTempFlt = 2;
				error_counter->error_count_lv2[ErrIndex_MaxCellTemperature] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_MaxCellTemperature] = 0;
			}
		}
	}
	else if(battery_pack_info->sample_result.max_cell_temperature >= error_table_config_record->error_config_table_lv1[ErrIndex_MaxCellTemperature]){
		Err_Counter(&error_counter->error_count_lv1[ErrIndex_MaxCellTemperature]);
		error_counter->error_count_lv2[ErrIndex_MaxCellTemperature] = 0;
		error_counter->error_count_lv3[ErrIndex_MaxCellTemperature] = 0;
		
		if(battery_pack_info->sample_result.max_cell_temperature >= error_table_config_record->error_config_rec_value_lv2[ErrIndex_MaxCellTemperature]){
			if(battery_pack_info->error_flag.mergedBits.BMS_HTempFlt > 1){
				Err_Counter(&error_counter->error_rec_count_lv2[ErrIndex_MaxCellTemperature]);
				if(error_counter->error_rec_count_lv2[ErrIndex_MaxCellTemperature] >= error_table_config_record->error_config_rec_time_lv2[ErrIndex_MaxCellTemperature]
					&& error_counter->error_count_lv1[ErrIndex_MaxCellTemperature] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxCellTemperature]){
					battery_pack_info->error_flag.mergedBits.BMS_HTempFlt = 1;
					error_counter->error_count_lv1[ErrIndex_MaxCellTemperature] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_MaxCellTemperature] = 0;
				}
			}
			else if(error_counter->error_count_lv1[ErrIndex_MaxCellTemperature] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxCellTemperature]){
				battery_pack_info->error_flag.mergedBits.BMS_HTempFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MaxCellTemperature] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MaxCellTemperature] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv1[ErrIndex_MaxCellTemperature] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxCellTemperature]){
				battery_pack_info->error_flag.mergedBits.BMS_HTempFlt = 1;
				error_counter->error_count_lv1[ErrIndex_MaxCellTemperature] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_MaxCellTemperature] = 0;
			}
		}
	}
	else{
		error_counter->error_count_lv3[ErrIndex_MaxCellTemperature] = 0;
		error_counter->error_count_lv2[ErrIndex_MaxCellTemperature] = 0;
		error_counter->error_count_lv1[ErrIndex_MaxCellTemperature] = 0;
		if(battery_pack_info->sample_result.max_cell_temperature >= error_table_config_record->error_config_rec_value_lv1[ErrIndex_MaxCellTemperature]){
			if(battery_pack_info->error_flag.mergedBits.BMS_HTempFlt > 0){
				Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_MaxCellTemperature]);
				if(error_counter->error_rec_count_lv1[ErrIndex_MaxCellTemperature] >= error_table_config_record->error_config_rec_time_lv1[ErrIndex_MaxCellTemperature]){
					battery_pack_info->error_flag.mergedBits.BMS_HTempFlt = 0;
					error_counter->error_rec_count_lv1[ErrIndex_MaxCellTemperature] = 0;
				}
			}
			else{
				battery_pack_info->error_flag.mergedBits.BMS_HTempFlt = 0;
				error_counter->error_rec_count_lv1[ErrIndex_MaxCellTemperature] = 0;
			}
		}
		else{
			battery_pack_info->error_flag.mergedBits.BMS_HTempFlt = 0;
			error_counter->error_rec_count_lv1[ErrIndex_MaxCellTemperature] = 0;
		}
	}
	
	
	/* cell temperature difference too high */
	if(battery_pack_info->sample_result.diff_cell_temperature >= error_table_config_record->error_config_table_lv3[ErrIndex_CellTemperatureDiff]){
		Err_Counter(&error_counter->error_count_lv3[ErrIndex_CellTemperatureDiff]);
		
		if(error_counter->error_count_lv3[ErrIndex_CellTemperatureDiff] >= 
			error_table_config_record->error_config_delay_time_lv3[ErrIndex_CellTemperatureDiff]){
			battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt = 3;
			error_counter->error_count_lv3[ErrIndex_CellTemperatureDiff] = 0;
		}
	}
	else if(battery_pack_info->sample_result.diff_cell_temperature >= error_table_config_record->error_config_table_lv2[ErrIndex_CellTemperatureDiff]){
		Err_Counter(&error_counter->error_count_lv2[ErrIndex_CellTemperatureDiff]);
		
		if(battery_pack_info->sample_result.diff_cell_temperature >= error_table_config_record->error_config_rec_value_lv3[ErrIndex_CellTemperatureDiff]){
			if(battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt > 2){
				Err_Counter(&error_counter->error_rec_count_lv3[ErrIndex_CellTemperatureDiff]);
				if(error_counter->error_rec_count_lv3[ErrIndex_CellTemperatureDiff] >= error_table_config_record->error_config_rec_time_lv3[ErrIndex_CellTemperatureDiff]
					&& error_counter->error_count_lv2[ErrIndex_CellTemperatureDiff] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_CellTemperatureDiff]){
					battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt = 2;
					error_counter->error_count_lv1[ErrIndex_CellTemperatureDiff] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_CellTemperatureDiff] = 0;
				}
			}
			else if(error_counter->error_count_lv2[ErrIndex_CellTemperatureDiff] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_CellTemperatureDiff]){
				battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt = 2;
				error_counter->error_count_lv1[ErrIndex_CellTemperatureDiff] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_CellTemperatureDiff] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv2[ErrIndex_CellTemperatureDiff] >= error_table_config_record->error_config_delay_time_lv2[ErrIndex_CellTemperatureDiff]){
				battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt = 2;
				error_counter->error_count_lv2[ErrIndex_CellTemperatureDiff] = 0;
				error_counter->error_rec_count_lv3[ErrIndex_CellTemperatureDiff] = 0;
			}
		}
	}
	else if(battery_pack_info->sample_result.diff_cell_temperature >= error_table_config_record->error_config_table_lv1[ErrIndex_CellTemperatureDiff]){
		Err_Counter(&error_counter->error_count_lv1[ErrIndex_CellTemperatureDiff]);
		error_counter->error_count_lv2[ErrIndex_CellTemperatureDiff] = 0;
		
		if(battery_pack_info->sample_result.diff_cell_temperature >= error_table_config_record->error_config_rec_value_lv2[ErrIndex_CellTemperatureDiff]){
			if(battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt > 1){
				Err_Counter(&error_counter->error_rec_count_lv2[ErrIndex_CellTemperatureDiff]);
				if((error_counter->error_rec_count_lv2[ErrIndex_CellTemperatureDiff] >= error_table_config_record->error_config_rec_time_lv2[ErrIndex_CellTemperatureDiff])
					&& (error_counter->error_count_lv1[ErrIndex_CellTemperatureDiff] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_CellTemperatureDiff])){
					battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt = 1;
					error_counter->error_count_lv1[ErrIndex_CellTemperatureDiff] = 0;
					error_counter->error_rec_count_lv2[ErrIndex_CellTemperatureDiff] = 0;
				}
			}
			else if(error_counter->error_count_lv1[ErrIndex_CellTemperatureDiff] >= error_table_config_record->error_config_delay_time_lv1[ErrIndex_CellTemperatureDiff]){
				battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt = 1;
				error_counter->error_count_lv1[ErrIndex_CellTemperatureDiff] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_CellTemperatureDiff] = 0;
			}
		}
		else{
			if(error_counter->error_count_lv1[ErrIndex_CellTemperatureDiff] >= 
				error_table_config_record->error_config_delay_time_lv1[ErrIndex_CellTemperatureDiff]){
				battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt = 1;
				error_counter->error_count_lv1[ErrIndex_CellTemperatureDiff] = 0;
				error_counter->error_rec_count_lv2[ErrIndex_CellTemperatureDiff] = 0;
			}
		}
	}
	else{
		error_counter->error_count_lv3[ErrIndex_CellTemperatureDiff] = 0;
		error_counter->error_count_lv2[ErrIndex_CellTemperatureDiff] = 0;
		error_counter->error_count_lv1[ErrIndex_CellTemperatureDiff] = 0;
		if(battery_pack_info->sample_result.diff_cell_temperature >= 
			error_table_config_record->error_config_rec_value_lv1[ErrIndex_CellTemperatureDiff]){
			if(battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt > 0){
				Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_CellTemperatureDiff]);
				if(error_counter->error_rec_count_lv1[ErrIndex_CellTemperatureDiff] >= 
					error_table_config_record->error_config_rec_time_lv1[ErrIndex_CellTemperatureDiff]){
					battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt = 0;
					error_counter->error_rec_count_lv1[ErrIndex_CellTemperatureDiff] = 0;
				}
			}
			else{
				battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt = 0;
				error_counter->error_rec_count_lv1[ErrIndex_CellTemperatureDiff] = 0;
			}
		}
		else{
			battery_pack_info->error_flag.mergedBits.BMS_DeltaTempFlt = 0;
			error_counter->error_rec_count_lv1[ErrIndex_CellTemperatureDiff] = 0;
		}
	}
	
	
	
	/* SOC low */
	if(battery_pack_info->SOC <= 
        error_table_config_record->error_config_table_lv1[ErrIndex_LowSOC]) {
        Err_Counter(&error_counter->error_count_lv1[ErrIndex_LowSOC]);
		if(error_counter->error_count_lv1[ErrIndex_LowSOC] >= 
			error_table_config_record->error_config_delay_time_lv1[ErrIndex_LowSOC]){
            battery_pack_info->error_flag.mergedBits.BMS_LSOCFlt = 1;
			error_counter->error_count_lv1[ErrIndex_LowSOC] = 0;
        }
    }
	else{
		if(battery_pack_info->SOC <= error_table_config_record->error_config_rec_value_lv1[ErrIndex_LowSOC]){
			if(battery_pack_info->error_flag.mergedBits.BMS_LSOCFlt > 0){
				Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_LowSOC]);
				if(error_counter->error_rec_count_lv1[ErrIndex_LowSOC] >= error_table_config_record->error_config_rec_time_lv1[ErrIndex_LowSOC]){
					battery_pack_info->error_flag.mergedBits.BMS_LSOCFlt = 0;
					error_counter->error_rec_count_lv1[ErrIndex_LowSOC] = 0;
				}
			}
			else{
				battery_pack_info->error_flag.mergedBits.BMS_LSOCFlt = 0;
				error_counter->error_count_lv1[ErrIndex_LowSOC] = 0;
				error_counter->error_rec_count_lv1[ErrIndex_LowSOC] = 0;
			}
		}
		else{
			battery_pack_info->error_flag.mergedBits.BMS_LSOCFlt = 0;
			error_counter->error_count_lv1[ErrIndex_LowSOC] = 0;
			error_counter->error_rec_count_lv1[ErrIndex_LowSOC] = 0;
		}
	}
	
	/* SOC high, regen forbidden */
	if(battery_pack_info->SOC >= 
        error_table_config_record->error_config_table_lv1[ErrIndex_HighSOC]) {
        Err_Counter(&error_counter->error_count_lv1[ErrIndex_HighSOC]);
		if(error_counter->error_count_lv1[ErrIndex_HighSOC] >= 
			error_table_config_record->error_config_delay_time_lv1[ErrIndex_HighSOC]) {
            battery_pack_info->error_flag.mergedBits.BMS_HSOCFlt = 1;
			error_counter->error_count_lv1[ErrIndex_HighSOC] = 0;
        }
    }
	else if(battery_pack_info->SOC >=
		error_table_config_record->error_config_rec_value_lv1[ErrIndex_HighSOC]){
		Err_Counter(&error_counter->error_rec_count_lv1[ErrIndex_HighSOC]);
		if(error_counter->error_rec_count_lv1[ErrIndex_HighSOC] >= 
			error_table_config_record->error_config_rec_time_lv1[ErrIndex_HighSOC]) {
			if(battery_pack_info->error_flag.mergedBits.BMS_HSOCFlt != 1){
				battery_pack_info->error_flag.mergedBits.BMS_HSOCFlt = 0;
			}
		}
	}
	else {
		battery_pack_info->error_flag.mergedBits.BMS_HSOCFlt = 0;
		error_counter->error_rec_count_lv1[ErrIndex_HighSOC] = 0;
		error_counter->error_count_lv1[ErrIndex_HighSOC] = 0;
	}
	if(battery_pack_info->error_flag.mergedBits.BMS_HSOCFlt != 0){
		battery_pack_info->error_flag.mergedBits.BMS_LSOCFlt = 0;
	}
	
	/*current sensor error */
	
	
	/* temperature sensor error */
	
#ifdef VERSION_BTT

	/* cell voltage diff to high alert */
	if(battery_pack_info->sample_data.total_cell_voltage <= 6450){
		V_pack_level = 0;
	}
	else if((battery_pack_info->sample_data.total_cell_voltage > 6450) && 
		(battery_pack_info->sample_data.total_cell_voltage <= 6700)){
		V_pack_level = 1;
	}
	else if((battery_pack_info->sample_data.total_cell_voltage > 6700) && 
		(battery_pack_info->sample_data.total_cell_voltage <= 7100)){
		V_pack_level = 2;
	}
	else if((battery_pack_info->sample_data.total_cell_voltage > 7100) && 
		(battery_pack_info->sample_data.total_cell_voltage <= 7980)){
		V_pack_level = 3;
	}

	if(battery_pack_info->sample_result.diff_cell_voltage >= Default_V_Cell_Diff_Err[V_pack_level][2]){
		#ifdef STATUS_LED_ENABLE
			// LED_FAULT_LV3_ON;
			LED_FAULT_LV2_ON;
			LED_FAULT_LV1_OFF;
		#endif
		Err_Counter(&error_counter->error_count_lv3[ErrIndex_MaxCellVoltageDiff]);
		if(error_counter->error_count_lv3[ErrIndex_MaxCellVoltageDiff] >= 
			error_table_config_record->error_config_delay_time_lv3[ErrIndex_MaxCellVoltageDiff]){
			battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 3;
			error_counter->error_count_lv3[ErrIndex_MaxCellVoltageDiff] = 0;
		}
	}
	else if(battery_pack_info->sample_result.diff_cell_voltage >= Default_V_Cell_Diff_Err[V_pack_level][1]){
		#ifdef	STATUS_LED_ENABLE
			LED_FAULT_LV2_ON;
			// LED_FAULT_LV3_OFF;
			LED_FAULT_LV1_OFF;
		#endif
		Err_Counter(&error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff]);
		if(error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff] >= 
			error_table_config_record->error_config_delay_time_lv2[ErrIndex_MaxCellVoltageDiff]){
			battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 2;
			error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff] = 0;
		}
	}
	else if(battery_pack_info->sample_result.diff_cell_voltage >= Default_V_Cell_Diff_Err[V_pack_level][0]){
		#ifdef	STATUS_LED_ENABLE
				LED_FAULT_LV1_ON;
				// LED_FAULT_LV3_OFF;
				LED_FAULT_LV2_OFF;
		#endif
		Err_Counter(&error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff]);
		if(error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff] >= 
			error_table_config_record->error_config_delay_time_lv1[ErrIndex_MaxCellVoltageDiff]){
			battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 1;
			error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff] = 0;
		}
	}
	else{
		battery_pack_info->error_flag.mergedBits.BMS_CellDeltaVoltFlt = 0;
		error_counter->error_count_lv1[ErrIndex_MaxCellVoltageDiff] = 0;
		error_counter->error_count_lv2[ErrIndex_MaxCellVoltageDiff] = 0;
		error_counter->error_count_lv3[ErrIndex_MaxCellVoltageDiff] = 0;
#ifdef STATUS_LED_ENABLE
		LED_FAULT_LV1_OFF;
		LED_FAULT_LV2_OFF;
		// LED_FAULT_LV3_OFF;
#endif
	}

#endif
	
}



#ifndef _ALERT_H_
#define _ALERT_H_

#include "public.h"

void Load_Default_Cell_Config(Cell_Config_Record *error_table_config_record);

void Alert_Main(Battery_Pack_Info	*battery_pack_info,
				Error_Counter 			*error_counter,
				Cell_Config_Record	*error_table_config_record);


#endif


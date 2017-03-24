#include "public.h"
#include "config.h"
#include "rda3216.h"

/***************************************************************
                    Global Variables
***************************************************************/ 
/* System Runtime  */
VU16 g_task_flag;
VU32 g_system_counter;
volatile SystemMode g_system_mode; 

/* Battery Pack Infomation  */
Battery_Pack_Info g_battery_pack_info; 

/* Failure flag & counters */
Error_Counter g_error_counter;

/* AFE info data */
ISL_Pack isl_pack;
FAE_Pack g_FAE_pack_info;

/* state blockdata */
FlashState_Data_Struct g_stateblock_data;

/* log data */
Pack_Data_Struct g_log_data;

s_FlashDataPtrStruct g_dataptr;

s_Flash_Data_Block_Data g_datablock_data;

/* static data */
FlashStatic_Data_Union g_static_data;

/* mcu state */
McuMode g_mcu_mode = RUN;

s_PacketCounter g_PacketCounter;

#ifdef VERSION_BTT
#ifdef BTT_RF
s_BatteryTestToolFrameBlock g_BatteryTestToolFrameBlock;
#endif
#ifdef BTT_OFFLINE
s_BatteryTestToolFrame g_BatteryTestToolFrame;
#endif
#endif

U8 g_isl_init_step_flag = 0;

Bool Is_In_Array(U8 *array, U8 array_lengh, U8 target_value) {
    U8 i;
    Bool ret;
    ret = FALSE;
    for(i = 0; i < array_lengh; i++) {
        if(*(array+i) == target_value) {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

Bool Get_U8_Bit(U8 data, U8 bit_index) {
    if(data == 0) return FALSE;
    if(data == 0xFF) return TRUE;
    
    if(bit_index >= 8) bit_index = 7;

    return (data >> bit_index) & 0x01;
}

Bool Get_U16_Bit(U16 data, U8 bit_index) {
    if(data == 0) return FALSE;
    if(data == 0xFFFF) return TRUE;
    
    if(bit_index >= 16) bit_index = 15; 
    
    return (data >> bit_index) & 0x0001;   
}

void Set_U8_Bit(U8 *data, U8 bit_index) {
    if(bit_index >= 8) return;
    *data |= (0x01U << bit_index);
}

void Set_U16_Bit(U16 *data, U8 bit_index) {
    if(bit_index >= 16) return;
    *data |= (0x0001U << bit_index);    
}

void Clear_U8_Bit(U8 *data, U8 bit_index) {
    if(bit_index >= 8) return;
    *data &= ~(0x01U << bit_index);
    
}

void Clear_U16_Bit(U16 *data, U8 bit_index) {
    if(bit_index >= 16) return;
    *data &= ~(0x0001U << bit_index);   
}

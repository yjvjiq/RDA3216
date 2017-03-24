#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include "isl94212.h"
#include "rda3216.h"

#define RF_COMMUNICATION
//#define CAN_COMMUNICATION

#define CELL_VOLTAGE_SIZE   48
#define CELL_NUM            24
#define SLAVE_ADDR_MAX      6
#define RF_LOG_DATA_LEN     25
#define MASTER_ID_LEN        4
#define SLAVE_ID_LEN        6
// #define BATTERTY_ID_LEN_MAX     12

// typedef union {
//     U32 varu32;
//     U8  varu8[4];
// } u_varform_32u8;

// typedef union {
//     U16 baru16;
//     U8  varu8[2];
// } u_varform_16u8;

/***************************************************************
              Global Variables
***************************************************************/
/* Battery Pack Infomation */
extern Battery_Pack_Info g_battery_pack_info; 

/* Failure flag & counters */
extern Error_Counter g_error_counter;

/* Fault flag & counters */
extern Fault_Info g_fault_info;

/*AFE info data */
extern ISL_Pack isl_pack;
extern FAE_Pack g_FAE_pack_info;

/* state blockdata */
extern FlashState_Data_Struct g_stateblock_data;

/* log data */
extern Pack_Data_Struct g_log_data;

extern s_FlashDataPtrStruct g_dataptr;
extern s_Flash_Data_Block_Data g_datablock_data;;

/* static data */
extern FlashStatic_Data_Union g_static_data;

/* mcu state */
extern McuMode g_mcu_mode;

/* soc state */
extern tSOC_STATUS g_soc_status;

extern s_PacketCounter g_PacketCounter;

#ifdef VERSION_BTT
#ifdef BTT_RF
extern s_BatteryTestToolFrameBlock g_BatteryTestToolFrameBlock;
#endif
#ifdef BTT_OFFLINE
extern s_BatteryTestToolFrame g_BatteryTestToolFrame;
#endif
#endif

static U8 g_busbar_index_array[BUSBAR_CHANNEL] = {5, 6, 7, 17, 18};

extern U8 g_isl_init_step_flag;

/***************************************************************
                      Global Functions
***************************************************************/
Bool Is_In_Array(U8 *array, U8 array_lengh, U8 target_value);

Bool Get_U8_Bit(U8 data, U8 bit_index);
Bool Get_U16_Bit(U16 data, U8 bit_index);

void Set_U8_Bit(U8 *data, U8 bit_index);
void Set_U16_Bit(U16 *data, U8 bit_index);

void Clear_U8_Bit(U8 *data, U8 bit_index);
void Clear_U16_Bit(U16 *data, U8 bit_index);

#endif

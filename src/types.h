#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "config.h"

#define BIT0    0
#define BIT1    1
#define BIT2    2
#define BIT3    3
#define BIT4    4
#define BIT5    5
#define BIT6    6
#define BIT7    7

#define     ON      TRUE
#define     OFF     FALSE
#define     YES     TRUE
#define     NO      FALSE
#define     U16_MAX 0xFFFF
#define     U32_MAX 0xFFFFFFFF

/****************************************
 * 默认电池包配置数据
 ****************************************/
#define HARDWARE_VER                0x0101
#define SOFTWARE_VER                0x0101
#define NUMBER_OF_CELL              19
#define RATED_VOLTAGE               70
#define RATED_CAPACITY              40
#define SERIAL_NUMBER               20151130


/***************************************************************
                           Constants
***************************************************************/
#define NUMBER_OF_AFE_DEVICES           2           /* AFE芯片数量 */
#define CELL_NUMBER_IN_AFE              12          /* 每片AFE芯片上单体电池通道数  */
#define TEMP_NUMBER_IN_AFE              2           /* 每片AFE芯片上连接温度通道数  */
#define CELL_VOLTAGE_CHANNEL            (CELL_NUMBER_IN_AFE * NUMBER_OF_AFE_DEVICES)        /* 单体电压通道数 */
#define CELL_TEMPERATURE_CHANNEL        2           /* (TEMP_NUMBER_IN_AFE * NUMBER_OF_AFE_DEVICES)    单体温度通道数 */
#define BUSBAR_CHANNEL                  5           /* Busbar 通道数 */

#define MIN(x,y) ((x) > (y) ? (y) : (x))

/***************************************************************
                           log_data_pos
***************************************************************/
#define FlashBlockHead_LOGPOS           0
#define NowSOC_LOGPOS                   (FlashBlockHead_LOGPOS+1)
#define NowCurrent_LOGPOS               (NowSOC_LOGPOS+1)
#define UinxTime_LOGPOS                 (NowCurrent_LOGPOS+2)
#define PackVoltageValue_LOGPOS         (UinxTime_LOGPOS+4)
#define PackMeanTemprature_LOGPOS       (PackVoltageValue_LOGPOS+2)
#define CellMaxVoltageNumber_LOGPOS     (PackMeanTemprature_LOGPOS+1)
#define CellMaxVoltage_LOGPOS           (CellMaxVoltageNumber_LOGPOS+1)
#define CellMinVoltage_LOGPOS           (CellMaxVoltage_LOGPOS+2)
#define CellMeanVoltageValue_LOGPOS     (CellMinVoltage_LOGPOS+2)
#define CellMinVoltageNumber_LOGPOS     (CellMeanVoltageValue_LOGPOS+2)
#define Cell_T_1_LOGPOS					(CellMinVoltageNumber_LOGPOS+1)
#define Cell_T_2_LOGPOS        			(Cell_T_1_LOGPOS+1)
#define AlarmFlag_LOGPOS                (Cell_T_2_LOGPOS+1)
#define Crc_LOGPOS                      (AlarmFlag_LOGPOS+9)

/***************************************************************
                           state_block_data_pos
***************************************************************/
#define FlashBlockHead_STATEPOS         0
#define Flags_STATEPOS                  (FlashBlockHead_STATEPOS+1)
#define ComAddr_STATEPOS                (Flags_STATEPOS+1)
#define HeadAddr_STATEPOS               (ComAddr_STATEPOS+2)
#define Crc_STATEPOS                    (HeadAddr_STATEPOS+10)

//#define BMSID_STATEPOS                  (HeadAddr_STATEPOS+4)
//#define MaxPermitsCurrent_STATEPOS      (BMSID_STATEPOS+6)
//#define MaxPermitsVoltage_STATEPOS      (MaxPermitsCurrent_STATEPOS+2)
//#define Cell_Config_Record_STATEPOS     (MaxPermitsVoltage_STATEPOS+2)
//#define Crc_STATEPOS                    (Cell_Config_Record_STATEPOS+42)
/***************************************************************
                           static_data_pos
***************************************************************/
#define Cell_Config_Record_STATICPOS             0
#define BMSID_LOW_STATICPOS                      (Cell_Config_Record_STATICPOS+552)
#define BMSID_HIGH_STATICPOS                     (BMSID_LOW_STATICPOS+6)
#define MaxPermitsCurrent_STATICPOS              (BMSID_HIGH_STATICPOS+6)
#define MaxPermitsVoltage_STATICPOS              (MaxPermitsCurrent_STATICPOS+2)
#define Crc_STATICPOS                            (MaxPermitsVoltage_STATICPOS+2)

/***************************************************************
                          Enums
***************************************************************/
/**
 * 系统工作模式
 */
typedef enum {
    SystemMode_Unknown = -1,
    SystemMode_Standby,
    SystemMode_Discharge,
    SystemMode_Charge,
    SystemMode_Malfunction,
    SystemMode_Debug
}  SystemMode;

/**
 * 电流方向
 */
typedef enum {
    Current_Direction_None = 0,         /* No current */
    Current_Direction_Charge = 1,       /* Charge current */
    Current_Direction_Discharge = 2     /* Discharge current */
} Current_Direction;

/**
 * 电池包状态
 */
typedef enum {
    Battery_Pack_State_Idle = 0,
    Battery_Pack_State_Discharge = 1,
    Battery_Pack_State_Charge = 3
} Battery_Pack_State;

typedef enum {
	SOC_INIT=1,
	SOC_COUNTER,
	SOC_NONE
} tSOC_STATUS;

/**
 * 故障索引
 */
typedef enum {
    ErrIndex_Isolation = 0,         /* 0 */
    ErrIndex_Precharge,             /* 1 */
    ErrIndex_PositiveRly,           /* 2 */
    ErrIndex_NegetiveRly,           /* 3 */
    ErrIndex_PrechargeRly,          /* 4 */
    ErrIndex_ChargeRly,             /* 5 */
    ErrIndex_MinCellTemperature,    /* 6 */
    ErrIndex_MaxCellTemperature,    /* 7 */
    ErrIndex_CellTemperatureDiff,   /* 8 */
    ErrIndex_OverChargeCurrent,     /* 9 */
    ErrIndex_OverDischargeCurrent,  /* 10 */
    ErrIndex_MaxCellVoltage,        /* 11 */
    ErrIndex_MinCellVoltage,        /* 12 */
    ErrIndex_MaxCellVoltageDiff,    /* 13 */
    ErrIndex_MaxTotalVoltage,       /* 14 */
    ErrIndex_MinTotalVoltage,       /* 15 */
    ErrIndex_LowSOC,                /* 16 */
    ErrIndex_HighSOC,               /* 17 */
    ErrIndex_BMS_Malfunction,       /* 18 */
    ErrIndex_VCU_Communication,     /* 19 */
    ErrIndex_Charger_Communication, /* 20 */
    ErrIndex_CurrSensor,            /* 21 */
    ErrIndex_TempSensor,            /* 22 */
    ErrIndex_MaxErrNum
} ErrIndex;

/**
 * MCU状态
 */
typedef enum {
    RUN,
    SLEEP
}  McuMode;

/***************************************************************
                            Struct
***************************************************************/
/**
 * 时间戳结构体
 */
typedef struct {
    U8 year;
    U8 month;
    U8 day;
    U8 hour;
    U8 minute;
    U8 second;
} Date_Time;

/**
 * 采集原始数据结构体
 */
typedef struct {
    U16 cell_voltages[CELL_VOLTAGE_CHANNEL];			/* 精度0.001V，偏差0, 例如3122为3.122V */
    U8  cell_temperatures[CELL_TEMPERATURE_CHANNEL];	/* 精度1℃，偏差-40，例如65为25℃ */

    U16 total_voltage[NUMBER_OF_AFE_DEVICES];			/* MCU AD采集的总电压，精度0.1V，偏差0, 例如789为78.9V */
    U16 total_cell_voltage;								/* 单体电压累计的总电压， 精度0.1V，偏差0, 例如789为78.9V  */

    U16 total_current;									/* 精度0.1A，偏差0, 例如123为12.3A */
    Current_Direction current_direction;				/* 电流方向 */
} Sample_Data;

/**
 * 采集数据初步分析结果结构体
 */
typedef struct {
    U16 max_cell_voltage;           /* 最高单体电压 */
    U16 min_cell_voltage;           /* 最低单体电压 */
    U16 ave_cell_voltage;           /* 单体平均电压 */
    U16 diff_cell_voltage;          /* 最大单体电压差 */
    U8 max_cell_temperature;        /* 最大单体温度 */
    U8 min_cell_temperature;        /* 最小单体温度 */
    U8 ave_cell_temperature;        /* 单体平均温度  */
    U8 diff_cell_temperature;       /* 最大单体温度差 */
    U8 max_cell_voltage_number;     /* 最大单体电压位置 */
    U8 min_cell_voltage_number;     /* 最小单体电压位置 */
    U8 max_cell_temperature_number; /* 最大单体温度位置 */
    U8 min_cell_temperature_number; /* 最小单体温度位置 */
} Sample_Result;


/**
* Fault Struct, send to CAN
**/
typedef union {
    struct {
        /* the 1st byte */
        U8 rsvd_1_01			: 2;    /* reserved, [bit1:bit0] */
        U8 rsvd_1_2				: 1;    /* reserved */
        U8 rsvd_1_3				: 1;    /* reserved */
        U8 rsvd_1_4				: 1;    /* reserved */
        U8 rsvd_1_5				: 1;    /* reserved */
        U8 rsvd_1_6				: 1;    /* reserved */
        U8 BMS_MosFlt           : 1;    /* mos fuse */

        /* the 2nd byte */
        U8 BMS_LTempFlt         : 2;    /* temperature low fault */
        U8 BMS_HTempFlt         : 2;    /* temperature high fault */
        U8 BMS_DeltaTempFlt     : 2;    /* temperature diff high fault */
        U8 BMS_OvrChgFlt        : 2;    /* over charge fault */

        /* the 3rd byte */
        U8 BMS_OvrDischgFlt     : 2;    /* over discharge fault */
        U8 BMS_CellHVoltFlt     : 2;    /* cell voltage high fault */
        U8 BMS_CellLVoltFlt     : 2;    /* cell voltage low fault */
        U8 BMS_CellDeltaVoltFlt : 2;    /* cell voltage diff high fault */

        /* the 4th byte */
        U8 BMS_PackHVoltFlt     : 2;    /* pack voltage high fault */
        U8 BMS_PackLVoltFlt     : 2;    /* pack voltage low fault */
        U8 BMS_LSOCFlt          : 1;    /* SOC low fault */
        U8 BMS_HSOCFlt          : 1;    /* regen forbidden fault */
        U8 rsvd_4_6				: 1;    /* reserved */
        U8 rsvd_4_7				: 1;    /* reserved */

        /* the 5th byte */
        U8 rsvd_5_0				: 1;    /* reserved */
        U8 BMS_CurrSensorFlt    : 1;    /* current sensor fault */
        U8 BMS_TempSensorFlt    : 1;    /* temperature sensor fault */
		U8 BMS_VpackSampleFlt	: 1;	/* pack voltage sampling error */
        U8 BMS_Isl_Init_Flt		: 1;	/* isl94212 initial error */
        U8 BMS_WdtResetFlt      : 1;    /* Last reset caused by wdt*/
        U8 BMS_MosCmd           : 1;    /* recived mos command*/
		U8 BMS_UnixtimeSync     : 1;    /* unixtime sync*/
    } mergedBits;

    U8 data[5];
} Fault_Info;

/**
 * battery pack information
 */
typedef struct {
    Sample_Data		sample_data;		/* original sampling data */
    Sample_Result	sample_result;		/* max and min value */
	Fault_Info		error_flag;			/* fault info */
	
    U8				SOC;
} Battery_Pack_Info;

/**
 * 错误信息结构体
 */
typedef struct {
    U16 error_count_lv1[ErrIndex_MaxErrNum];			/* 一级故障计数器 */
    U16 error_count_lv2[ErrIndex_MaxErrNum];            /* 二级故障计数器 */
    U16 error_count_lv3[ErrIndex_MaxErrNum];            /* 三级故障计数器 */

    U16 error_rec_count_lv1[ErrIndex_MaxErrNum];		/* 一级故障恢复计数器 */
    U16 error_rec_count_lv2[ErrIndex_MaxErrNum];		/* 二级故障恢复计数器 */
    U16 error_rec_count_lv3[ErrIndex_MaxErrNum];		/* 三级故障恢复计数器 */

} Error_Counter;


/**
 * the struct for alert threshold and related delay time
 */
typedef __packed struct {
    U16 error_config_table_lv1[ErrIndex_MaxErrNum];			/* 1st level alert threshold */
    U16 error_config_table_lv2[ErrIndex_MaxErrNum];			/* 2nd level alert threshold */
    U16 error_config_table_lv3[ErrIndex_MaxErrNum];			/* 3rd level alert threshold */

    U16 error_config_delay_time_lv1[ErrIndex_MaxErrNum];	/* 1st level alert delay time */
    U16 error_config_delay_time_lv2[ErrIndex_MaxErrNum];	/* 2nd level alert delay time */
    U16 error_config_delay_time_lv3[ErrIndex_MaxErrNum];	/* 3rd level alert delay time */

    U16 error_config_rec_value_lv1[ErrIndex_MaxErrNum];		/* 1st level alert recovery threshold */
    U16 error_config_rec_value_lv2[ErrIndex_MaxErrNum];		/* 2nd level alert recovery threshold */
    U16 error_config_rec_value_lv3[ErrIndex_MaxErrNum];		/* 3rd level alert recovery threshold */

    U16 error_config_rec_time_lv1[ErrIndex_MaxErrNum];		/* 1st level alert recovery delay time */
    U16 error_config_rec_time_lv2[ErrIndex_MaxErrNum];		/* 2nd level alert recovery delay time */
    U16 error_config_rec_time_lv3[ErrIndex_MaxErrNum];		/* 3rd level alert recovery delay time */

} Cell_Config_Record;

/**
* pack log data
*
**/
typedef struct {
    U8   FlashBlockHead;
    U8   NowSOC;
    U16  NowCurrent;
    U32  UinxTime;
    U16  PackVoltageValue;
    U8   PackMeanTemprature;

    U8   CellMaxVoltageNumber;
    U16  CellMaxVoltage;
    U16  CellMinVoltage;
    U16  CellMeanVoltageValue;
    U8   CellMinVoltageNumber;

    U8   Cell_T[CELL_TEMPERATURE_CHANNEL];
    U8   AlarmFlag[5];
    U32  CycleCount;
    U16  Crc;
} Log_Data;

/**
 * pack data struct, store into external FLASH
 * 32 bytes per block, the last 5 bytes are reserved for later use.
 */
typedef union {
    Log_Data log_data;
    U8 data[32];
} Pack_Data_Struct;

//flash save log data
typedef __packed struct {
    U8   FlashBlockHead;//flash 地址头
    U32  UinxTime;//时间戳
    U16  PackVoltageValue;//pack电压
    U16  NowCurrent;//pack电流
    U8   NowSOC;//当前SOC
    U8   Cell_T[CELL_TEMPERATURE_CHANNEL];
    U16  CellVoltageValue[19];//19个电芯电压
    U8   AlarmFlag[5];//报警位
    U8   Rsvd[7];
    U16  Crc;//校验位
} s_Flash_Data_Block_Data;

#ifdef VERSION_BTT
#define BATTERTY_ID_LEN_MAX     12
#define BATTERTY_ID_LEN_MIN     6
#define BATTERTY_ID_LEN	BATTERTY_ID_LEN_MAX
typedef __packed struct{
    U8 BatteryID[BATTERTY_ID_LEN_MAX];
    U16 CellVoltage[CELL_VOLTAGE_CHANNEL];
    U16 PackVoltage;     
    U8 SOC;     
    U8 Cell_T[CELL_TEMPERATURE_CHANNEL];
    U8 AlarmFlag[5];
    U8 UpdataFlag;
}s_BatteryTestToolFrame;

typedef __packed struct {
    U8   FlashBlockHead;
    s_BatteryTestToolFrame BatteryTestToolFrame;
    U8 Rsvd[128 - sizeof(s_BatteryTestToolFrame) - sizeof(U16) - 1];
    U16  Crc;
}s_BatteryTestToolFrameBlock;

#endif


typedef __packed struct {
	unsigned int ZeroCurrentCounter;    // 
	unsigned int NowCapacity;     //当前容量(mAh)
	unsigned int InitCapacityPercent_1;     //初始容量(mAh)
	         int ReadCurrentSumDisCharge; //读取到的电流值(mAh)
	         int ReadCurrentSum; //读取到的电流值(mAh)
	unsigned int DischargePercent;      //
	unsigned int CycleCount;       //充放电循环次数	
	unsigned int LastPowerDownTime; //上次掉电时刻	
	unsigned int LastCapacity;//上次的容量
	unsigned int NowOCV;//当前开路电压	
	unsigned int NowTempratrue;//当前温度
	unsigned int SOC;

	
}tBatteryOfSocProperty;

/**
* FlashState data
*
**/

typedef __packed struct {
    U8   FlashBlockHead;
    U8   ComAddr;
    U32  UinxTime;
    tBatteryOfSocProperty BatteryOfSoc;
    U8   TestByte;
    U8   rsvd[64-sizeof(tBatteryOfSocProperty)-9];
    U16  Crc;
} FlashState_Data;


/**
 * FlashState_Data data struct, store into external FLASH
 * 16 bytes per block,
 */

typedef __packed union {
    FlashState_Data state_data;
    U8 data[sizeof(FlashState_Data)];
} FlashState_Data_Union;

typedef struct {
    U32 LastAddr;
    FlashState_Data_Union stata_union;
} FlashState_Data_Struct;

// typedef __packed struct {
//     U16 Ratio;
//     U16 Offset;
// }s_CurrentCalibrationParam;
typedef __packed struct{
   U32 K;//
   S32 B;
   U32 MeasureX;
   U32 RealY;// 
}tCalibration;
/**
 * FlashStatic_Data  struct, store into external FLASH
 */
#define CALIBRATION_DOT_NUMBER 5 //Contained zero

typedef __packed struct{
    U8  FlashBlockHead;
    U8  Flags;
    U32 HeadAddr;
#ifdef VERSION_BTT
    U32  TailAddr;
    U8 DataValidFlag;
    U8 rsvd[3];
#endif
    U16 Crc;
}s_FlashDataPtr;

typedef __packed struct{
    U32 LastAddr;
    s_FlashDataPtr FlashDataPtr;
}s_FlashDataPtrStruct;

typedef __packed struct{
    U32  UinxTime;//时间戳
    U16  PackVoltageValue;//pack电压
    U16  NowCurrent;//pack电流
    U8   NowSOC;//当前SOC
    U8   Cell_T[CELL_TEMPERATURE_CHANNEL];
    U16  CellVoltageValue[19];//19个电芯电压
    U8   AlarmFlag[5];//报警位
}s_StaticErrBlock;

#define CELL_H_VOLT_BLOCK_NUM   5
typedef __packed struct{
    U8 HeadAddr;
    U8 LastCHVFlag;
    s_StaticErrBlock StaticErrBlock[CELL_H_VOLT_BLOCK_NUM];
}s_StaticErrBlockFrame;

typedef __packed struct {
    Cell_Config_Record  cell_config_record;
    U8   BMSID[12];
    U16  MaxPermitsCurrent;
    U16  MaxPermitsVoltage;
    U32  FactoryReset;
    // s_CurrentCalibrationParam CurrentCalibrationParam[CURRENT_CALIBRATION_PARAM_NUM];
    tCalibration Calibration[CALIBRATION_DOT_NUMBER];
    s_StaticErrBlockFrame StaticErrBlockFrame;
    U16  Crc;
} FlashStatic_Data;

typedef __packed union {
    FlashStatic_Data static_data;
    U8 data[(sizeof(FlashStatic_Data))];
} FlashStatic_Data_Union;

typedef __packed struct{
    U32 Tx;
    U32 Rx;
    U32 CrcErr;
    // U32 AddrErr;
    // U8  Rssi;
    // U8  BGN;
}s_PacketCounter;

typedef __packed struct{
    U8  Rssi;
    U8  BGN;
    U8  LQI;
}s_RfSignalParam;

/**
 * Bootloader download etc.
 */
// typedef __packed struct{
//     U32 Version;
//     U32 Length;
//     U32 Type;
//     U32 rsvd;
// }s_BinInfo;

// typedef __packed struct{
//     U32 Length;
//     U16 SeqNum;
//     U8  RetryCountMax;
//     U32 WriteAddr;
// }s_DownloadInfo;
#endif



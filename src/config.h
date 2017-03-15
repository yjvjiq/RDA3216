#ifndef _CONFIG_H_
#define _CONFIG_H_


#define VERSION_BMS
//#define VERSION_BTT /* BMS TEST TOOL */

//#define PRODUCTION_TEST
//#define RF_BANDWIDTH_TEST
#define XMODEMBOOT_ENABLE
#define WATCH_DOG_ENABLE
//#define RELAY_ENABLE
#define MOS_ENABLE
//#define STATUS_LED_ENABLE

#ifdef MOS_ENABLE
#undef STATUS_LED_ENABLE //SLAVE V05 use led2 to control mos ,can't define MOS_ENABLE and STATUS_LED_ENABLE at the same time.
#endif

#ifdef VERSION_BTT
// #define BTT_RF
// #define BTT_OFFLINE
#endif

#define BATTERY_CAPACITY  40 // 25Ah 

#if BATTERY_CAPACITY == 40
// #define BATTERY_TOTAL_CAPACITY_mAH 37390//QCT=37.39AH  39141L //39.141
#define BATTERY_TOTAL_CAPACITY_mAH 40000//QCT=37.39AH  39141L //39.141
#else

#endif

#define CURRENT_CALIBRATION_PARAM_NUM 3

//#define BATTERY_TOTAL_CAPACITY_AH  25 //Ah  

#endif

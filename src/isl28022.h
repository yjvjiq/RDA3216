#ifndef _ISL28022_H_
#define _ISL28022_H_

#include "drivers.h"
#include "types.h"


typedef enum {
    ErrCode_No = 0,               /*  No Error */
    ErrCode_WR = 1,                /*  Read or Write Error */
    ErrCode_NoRSP = 255,       /*  No Response Error*/
    
} ISL28022_ErrCode;






/*ISL28022 Register address*/
#define ConfigReg   0x00
#define ShuntVReg   0x01
#define BusVReg     0x02
#define CalibReg    0x03
#define CurrentReg  0x04
#define PowerReg    0x05
#define ShuntVTReg  0x06
#define BusVTReg    0x07
#define InterTReg   0x08
#define AuxCReg     0x09

/*
CONFIGREG_VALUE :
RST BRNG1 BRNG0 PG1 PG0 BADC3 BADC2 BADC1 BADC0 SADC3 SADC2 SADC1 SADC0 MODE2 MODE1 MODE0 
RST: 1 reset, 0 no reset
BRNGO: 11 60v
PG: 01 is 80mv
ADC:  0001 is 13bit
MODE:111 continus
 
*/
#define CONFIGREG_VALUE         0x6ffd//(0x688f)//0X699f
#define CONFIGREG_VALUE_RESET       (0x6FFF|0X8000)
#define CONFIGREG_VALUE_POWERDOWN   (0x6FF8)

// #define B_OFFSET_CHARGE     20000
// #define B_OFFSET_DISCHARGE  13000
// #define K_CURRENT_ERROR     1007

#define B_OFFSET_CHARGE     0
#define B_OFFSET_DISCHARGE  0
#define K_CURRENT_ERROR     1000


extern pt2Func ISL28022_Callback;		  /* Pointer to Functions, void argument */


void Isl28022_RdReg(U8 reg, U8 *buff);
void Isl28022_WrReg(U8 reg, U16 msg);
int  ISL28022_Init(void);
void ISL28022_Interrupt(void);
void ISL28022_proc(void);
void ISL28022_Init_PowerDown(void);
Current_Direction ISL28022_SOC_proc(void);
void ISL28022_Calibration(U16 RealValue,U8 dotIndex);
U32 CalibrationCurrent(U16 CurrentRegisterValue,tCalibration *Dot,U16 scale);
#define ISL28022_PowerDown() Isl28022_WrReg(ConfigReg,CONFIGREG_VALUE_POWERDOWN)
#define ISL28022_Reset()     Isl28022_WrReg(ConfigReg,CONFIGREG_VALUE_RESET)

//---------------------------------Calibration---------------------------//
// #define CALIBRATION_DOT_NUMBER 5 //Contained zero
typedef enum{
CHARGE_10A=0,
CHARGE_40A,	
ZERO_0A,
DISCHARGE_10A,
DISCHARGE_100A	
}enumCALIBRATION;
//move to types.h
// typedef __packed struct{
//    U32 K;//
//    U32 B;
//    U32 MeasureX;
//    U32 RealY;//	
// }tCalibration;

//---------------------------------Calibration---------------------------//
// extern tCalibration Calibration[];
extern U16 g_CurrentRegValue;
#endif

#ifndef ISL94212_H_
#define ISL94212_H_

#include "types.h"

/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*               Device Config Data                                                                                                         */
/*------------------------------------------------------------------------------------------------------------------------------------------*/
#define MAX_RETRY_COUNT             3           /*  Identify retry count */
#define MAX_RESP_SIZE               46          /*  Max response data length */

#define OV_LIMIT_VALUE              0x1FFF      /*  OverVoltage limit value */
#define UV_LIMIT_VALUE              0x0000      /*  UnderVoltage limit value */
#define EOT_LIMIT_VALUE             0x3FFF      /*  External over-temperature value */

typedef union {
    struct {
        U8 TST0                     : 1;     /*  Test Internal Temperature */
        U8 TST1                     : 1;     /*  Test External Temperature 1 */
        U8 TST2                     : 1;     /*  Test External Temperature 2 */
        U8 TST3                     : 1;     /*  Test External Temperature 3 */
        U8 TST4                     : 1;     /*  Test External Temperature 4 */
        U8 reserved                 : 1;
        U8 dummy                    : 2;

        U8 SCN                      : 4;     /*  Scan continuous mode internal */
        U8 WSCN                     : 1;     /*  Scan wire timing control */
        U8 TOT                      : 3;     /*  Fault totalize code bits */
    } mergedBits;

    U16 data;
} Fault_Setup_t;

typedef union {
    struct {
        U8 OVBAT                    : 1;     /*  Open wire fault on Vbat */
        U8 OVSS                     : 1;     /*  Open wire fault on VSS */
        U8 PAR                      : 1;     /*  Register checksum error */
        U8 REF                      : 1;     /*  Voltage reference fault */
        U8 REG                      : 1;     /*  Voltage regulator fault */
        U8 MUX                      : 1;     /*  Temperature multiplexer error */
        U8 dummy                    : 2;

        U8 reserved                 : 2;     
        U8 OSCILLATOR               : 1;     /*  Oscillator fault */
        U8 WDGF                     : 1;     /*  Watchdog timeout fault */
        U8 OT                       : 1;     /*  Over temperature fault */
        U8 OV                       : 1;     /*  Over voltage fault */
        U8 UV                       : 1;     /*  Under voltage fault */
        U8 OW                       : 1;     /*  Open wire fault */
    } mergedBits;

    U16 data;
} Fault_Status_t;

typedef union {
    struct {
        U8 C9                       : 1;     /*  Enable/Disable cell OV,UV,OW detect */
        U8 C10                      : 1;     /*  Enable/Disable cell OV,UV,OW detect */
        U8 C11                      : 1;     /*  Enable/Disable cell OV,UV,OW detect */
        U8 C12                      : 1;     /*  Enable/Disable cell OV,UV,OW detect */
        U8 FFSP                     : 1;     /*  Force ADC input to fullscale positive */
        U8 FFSN                     : 1;     /*  Force ADC input to fullscale negative */
        U8 dummy                    : 2;    

        U8 C1                       : 1;     /*  Enable/Disable cell OV,UV,OW detect */
        U8 C2                       : 1;     /*  Enable/Disable cell OV,UV,OW detect */
        U8 C3                       : 1;     /*  Enable/Disable cell OV,UV,OW detect */
        U8 C4                       : 1;     /*  Enable/Disable cell OV,UV,OW detect */
        U8 C5                       : 1;     /*  Enable/Disable cell OV,UV,OW detect */
        U8 C6                       : 1;     /*  Enable/Disable cell OV,UV,OW detect */
        U8 C7                       : 1;     /*  Enable/Disable cell OV,UV,OW detect */
        U8 C8                       : 1;     /*  Enable/Disable cell OV,UV,OW detect */
    } mergedBits;

    U16 data;
} Cell_Setup_t;

typedef union {
    struct {
        U16 TFLT0                    : 1;     /*  Internal over-temperature fault. */
        U16 TFLT1                    : 1;     /*  External over-temperature fault. */
        U16 TFLT2                    : 1;     /*  External over-temperature fault. */
        U16 TFLT3                    : 1;     /*  External over-temperature fault. */
        U16 TFLT4                    : 1;     /*  External over-temperature fault. */
        U16 reserved                 : 9;
        U16 dummy                    : 2;

    } mergedBits;

    U16 data;
} OT_Fault_t;

typedef union {
    struct {
        U16 BMD                      : 2;     /*  Balance mode, 0:OFF, 1:Manual, 2:Timed, 3:Auto */
        U16 BWT                      : 3;     /*  Balance wait time */
        U16 BSP                      : 4;     /*  Balance status register pointer */
        U16 BEN                      : 1;     /*  Balance enable */
        U16 reserved                 : 4;
        U16 dummy                    : 2;
    } mergedBits;

    U16 data;
} BAL_Setup_t;

typedef union {
    struct {
        U16 WDG                      : 7;     /*  Watchdog timeout setting */
        U16 BTM                      : 7;     /*  Balance timeout setting */
        U16 dummy                    : 2;
    } mergedBits;

    U16 data;
} WD_BTM_t;

typedef union {
    struct {
        U8 CSEL                     : 2;     /*  Communication setup bits */
        U8 CRAT                     : 2;     /*  Communication rate bits */
        U8 reserved                 : 2;     
        U8 dummy                    : 2;

        U8 ADDR                     : 4;     /*  Device stack address */
        U8 SIZE                     : 4;     /*  Device stack size */

    } mergedBits;

    U16 data;
} Comms_Setup_t;

typedef union {
    struct {
        U8 WP                       : 6;     /*  Watchdog disable password. It must be 0x3A before wdg can be disabled. */
        U8 dummy                    : 2;

        U8 PIN39                    : 1;     /*  Signal level on pin 39 */
        U8 PIN37                    : 1;     /*  Signal level on pin 37 */
        U8 reserved1                : 1;
        U8 EOB                      : 1;     /*  End of balance */
        U8 SCAN                     : 1;     /*  Scan continuous mode */
        U8 ISCN                     : 1;     /*  Set wire scan current source value. 0:150uA, 1:1mA */
        U8 reserved2                : 1;
        U8 BDDS                     : 1;     /*  Balance condition during measurement. set 1 to turnoff balance for 10ms before cellvoltage measurement. */

    } mergedBits;

    U16 data;
} Dev_Setup_t;

/*  ISL94212 Fault */
typedef struct {

    U16 OV_fault;                            /*  Over voltage fault */
    U16 UV_fault;                            /*  Under voltage fault */
    U16 OW_fault;                            /*  Open wire fault */
    Fault_Setup_t fault_setup;                  /*  Fault setup */
    Fault_Status_t fault_status;                /*  Fault status */
    Cell_Setup_t fault_cell_setup;              /*  Cell Fault setup */
    OT_Fault_t OT_fault;                        /*  Over temperature fault */

} ISL_Fault;

/*  ISL94212 Setup */
typedef struct {

    U16 OV_limit;                            /*  Over voltage limit value */
    U16 UV_limit;                            /*  Under voltage limit value */
    U16 EOT_limit;                           /*  External temperature limit value */
    BAL_Setup_t bal_setup;                      /*  Balance setup */
    U16 bal_status;                          /*  Balance status */
    WD_BTM_t wdg_btm_setup;                     /*  Watchdog & BTM setup */
    U16 user_data1;                          /*  User Data1 */
    U16 user_data2;                          /*  User Data2 */
    Comms_Setup_t comms_setup;                  /*  Comms setup */
    Dev_Setup_t dev_setup;                      /*  Devicce setup */
    U16 IOT_limit;                           /*  Internal IC temperature limit value */
    U16 sn1;                                 /*  Serial Number 1 */
    U16 sn2;                                 /*  Serial Number 2 */
    U16 trim_voltage;                        /*  Trim voltage */

} ISL_Setup;

/*  ISL94212 Device Data */
typedef struct {
    U8 devAddress;                           /*  device address, master is 1, top device is ISL_DEVICE_COUNT */
    U16 cellVoltages[CELL_NUMBER_IN_AFE];    /*  cell voltage for each channel */
    U16 batVoltage;                          /*  battery voltage */
    U16 extTemps[TEMP_NUMBER_IN_AFE];        /*  external temp No.1 - No.4 */
    U16 ICTemp;                              /*  internal IC temp */
    U16 refVoltage;                          /*  Reference voltage */
    U8  scanCount;                           /*  Scan Count */

    ISL_Fault fault;                            /*  ISL94212 fault register */
    ISL_Setup setup;                            /*  ISL94212 setup register */

    U16 cellInBalance;                       /*  Cell In Balance */
    U16 BalanceValue[24];                    /*  Balance value , one pair is a group. */
} ISL_Device;

/*  ISL94212 Battery Pack */
typedef struct {
    ISL_Device devices[NUMBER_OF_AFE_DEVICES];    
} ISL_Pack;   

/*  ISL94212 Error Code */
typedef enum {
    ErrCode_None = 0,               /*  No Error */
    ErrCode_CRC = 1,                /*  CRC4 Check Error */
    ErrCode_ACK = 2,                /*  ACK Error */
    ErrCode_NAK = 3,                /*  NAK Error */
    ErrCode_NoResponse = 255,       /*  No Response Error*/
    
} ISL_ErrCode;

/*  global data for ISL battery pack info */
extern ISL_Pack isl_pack;

/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*               Function Definition                                                                                                        */
/*------------------------------------------------------------------------------------------------------------------------------------------*/
ISL_ErrCode Init_ISL_DaisyChain(void);
ISL_ErrCode ISL_DaisyChain_DeviceSetup(U8 devAddress);

/*  Command */
void DaisyChain_ScanVoltages(U8 devAddress);
void DaisyChain_ScanTemps(U8 devAddress);
void DaisyChain_ScanMix(U8 devAddress);
void DaisyChain_ScanWires(U8 devAddress);
void DaisyChain_ScanAll(U8 devAddress);
void DaisyChain_Sleep(U8 devAddress);
void DaisyChain_Wakeup(U8 devAddress);
void DaisyChain_Reset(U8 devAddress);

ISL_ErrCode DaisyChain_Identify(U8 deviceCnt);
ISL_ErrCode DaisyChain_ScanContinuous(U8 devAddress);
ISL_ErrCode DaisyChain_ScanInhibit(U8 devAddress);
ISL_ErrCode DaisyChain_Measure(U8 devAddress, U8 elementAddress);
ISL_ErrCode DaisyChain_Balance_Enable(U8 devAddress);
ISL_ErrCode DaisyChain_Balance_Disable(U8 devAddress);

/*  Read */
ISL_ErrCode DaisyChain_Read_CellVoltage(U8 devAddress, U8 index, U16 *outVoltage);
ISL_ErrCode DaisyChain_Read_BatVoltage(U8 devAddress, U16 *outVoltage);
ISL_ErrCode DaisyChain_Read_ICTemperature(U8 devAddress, U16 *outTemp);
ISL_ErrCode DaisyChain_Read_ExtTemperature(U8 devAddress, U8 index, U16 *outTemp);
ISL_ErrCode DaisyChain_Read_AllVoltages(U8 devAddress, U16 *outVoltages, U16 *outVBAT);
ISL_ErrCode DaisyChain_Read_AllTemperatures(U8 devAddress, U8 *outScanCount, U16 *outRefVoltage, U16 *outExtTemps, U16 *outICTemp);

/*  Config setup */
ISL_ErrCode DaisyChain_Get_OVLimit(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_OVLimit(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_UVLimit(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_UVLimit(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_EOTLimit(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_EOTLimit(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_IOTLimit(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Get_BalanceSetup(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_BalanceSetup(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_BalanceStatus(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_BalanceStatus(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_WD_BalanceTime(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_WD_BalanceTime(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_UserData1(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_UserData1(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_UserData2(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_UserData2(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_CommsSetup(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_CommsSetup(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_DeviceSetup(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_DeviceSetup(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_SN1(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Get_SN2(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Get_TrimVoltage(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Get_AllSetup(U8 devAddress, ISL_Setup *outSetup);

ISL_ErrCode DaisyChain_Get_CellBalanceValue(U8 devAddress, U8 index, U16 *outData1, U16 *outData2);
ISL_ErrCode DaisyChain_Set_CellBalanceValue(U8 devAddress, U8 index, U16 data1, U16 data2);
ISL_ErrCode DaisyChain_Get_RefCoefficientC(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Get_RefCoefficientB(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Get_RefCoefficientA(U8 devAddress, U16 *outData);

/*  Config fault */
ISL_ErrCode DaisyChain_Get_OVFault(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_OVFault(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_UVFault(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_UVFault(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_OWFault(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_OWFault(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_FaultSetup(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_FaultSetup(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_FaultStatus(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_FaultStatus(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_CellSetup(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_CellSetup(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_OTFault(U8 devAddress, U16 *outData);
ISL_ErrCode DaisyChain_Set_OTFault(U8 devAddress, U16 data);
ISL_ErrCode DaisyChain_Get_AllFault(U8 devAddress, ISL_Fault *outFault);

ISL_ErrCode Disable_All_Balance(void);
ISL_ErrCode Enable_All_Balance(void);
Bool Read_Pack_Voltage(U16 *outVBAT);

Bool Read_All_Voltage(U8 devAddress, U16 *outVoltages, U16 *outVBAT);
Bool Read_All_Temperature(U8 devAddress, U16 *outTemp);

void Cell_Monitor_Test(void);

#endif

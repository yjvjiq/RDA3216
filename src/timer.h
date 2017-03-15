//******************************************************************************
//file: timer.h

//******************************************************************************

#ifndef _TIMER_H_
#define _TIMER_H_

#include "main.h"
#include "derivative.h"
#include "public.h"
#include "config.h"
typedef void (*pvoid)(void);
typedef enum
{
	TIMER_MODE_SINGLE = 0,		/*!< Produce an event after a given interval time onece  */
	TIMER_MODE_PERIODIC,			/*!< Produce an event every time the given period is passed  */
}TIMOPT;

typedef struct {
    U32 counter;
    U8  bEnable;
    U32 setval;
    pvoid handle;
    U8 volatile  eventflag;
    TIMOPT  type;
}TimerEvent;

typedef enum{
	// HEART_BEAT_TIMER_ID=0,
	// CELL_MONITOR_ID,
	// DATA_PROCESS_ID,
    DATA_PREPARE_AND_SET_TX_TIMER_ID=0,
	ISL_INIT_ID,
    CVH_FLASH_SAVE_ENABLE_ID,
#ifdef PRODUCTION_TEST
    SELF_CHECK_UPLOAD_TIMER_ID,
    RF_TEST_TX_TIMER_ID,
#endif
#ifdef CAN_COMMUNICATION 
	FLASHPOPSEND_ID,

	BROADCAST_MSG01_ID,
    BROADCAST_MSG02_ID,
    BROADCAST_MSG03_ID,

    CANACK_MSG01_ID,
    CANACK_MSG02_ID,
//    CANACK_MSG03_ID,
    CANACK_MSG04_ID,
    CANACK_MSG05_ID,
    CANACK_MSG06_ID,
    CANACK_MSG07_ID,
    CANACK_MSG08_ID,
    CANACK_MSG09_ID,
    CANACK_MSG0A_ID,
#endif    
	DEBUG_TIMER_ID,
#ifdef VERSION_BMS
    FLASH_DATA_BLOCK_SAVE_ID,
#endif
#ifdef RF_COMMUNICATION 
    RF_SCAN_TIMER_ID,
    RF_SLEEP_TIMER_ID,
    RF_TX_TIMER_ID,
    RF_RX_TIMER_ID,
    RF_RX_TIMEOUT_TIMER_ID,
#endif
#ifdef VERSION_BTT
	START_INIT_TIMER_ID,
#ifdef BTT_RF
    UART_REC_TIMEOUT_ID,
#endif
    // RF_COMMU_TIMEOUT_ID,
#endif
	XModemTimeOut_ID,
  XModemPacket_ID,  
  XModemReset_ID,
	TIMER1_ID_COUNTS_MAX    
}tTimerID;

void drv_timer1_open(void);
void timer1_proc(void);
void InitTimerEvent(TimerEvent *tv,U8 num);
void SetTimer1(U8 eventid,U32 millisecond,TIMOPT opt,pvoid handle);
void StopTimer1(U8 eventid);
U8 GetTimer1(U8 eventid);
extern TimerEvent Timer1Event[];
extern U32 g_10ms_ticks;

#define HEART_BEAT_TIMER_TASK_EN			1
#define PRINTF_TEST_TIMER_TASK_EN			1
#define DEBUG_TIMER_TASK_EN					1
#define CELL_MONITOR_TASK_EN				1
#define DATA_PROCESS_TASK_EN				1
#define FLASHPOPSEND_TASK_EN                1
#define BROADCAST_MSG01_TASK_EN             1                            
#define BROADCAST_MSG02_TASK_EN             1  
#define BROADCAST_MSG03_TASK_EN             1  
#define CANACK_MSG01_TASK_EN                1
#define CANACK_MSG02_TASK_EN                1
//#define CANACK_MSG03_TASK_EN                1
#define CANACK_MSG04_TASK_EN                1
#define CANACK_MSG05_TASK_EN                1
#define CANACK_MSG06_TASK_EN                1
#define CANACK_MSG07_TASK_EN                1
#define CANACK_MSG08_TASK_EN                1


#define HEART_BEAT_TIMER_MS					3000
#define PRINTF_TEST_TIMER_MS				300
#define FLASHPOPSEND_TIMER_MS               100
#define DEBUG_TIMER_MS						200
#define CELL_MONITOR_MS						200
#define DATA_PROCESS_MS						500
#define ISL_INIT_MS							250
#define BROADCAST_MSG01_MS                  500
#define BROADCAST_MSG02_MS                  500
#define BROADCAST_MSG03_MS                  500
#define CANACK_MSG01_MS                     500
#define CANACK_MSG02_MS                     500
//#define CANACK_MSG03_MS                     500
#define CANACK_MSG04_MS                     500
#define CANACK_MSG05_MS                     500
#define CANACK_MSG06_MS                     500
#define CANACK_MSG07_MS                     500
#define CANACK_MSG08_MS                     500
#define CANACK_MSG09_MS                     500
#define CANACK_MSG0A_MS                     500

#define CVH_FLASH_SAVE_ENABLE_MS            10000
#define SELF_CHECK_TIMER_MS                 300

#ifdef VERSION_BTT
#define RF_SCAN_TIMER_MS                    4000
#endif
#ifdef VERSION_BMS
#define RF_SCAN_TIMER_MS                    1000
#define FLASH_DATA_BLOCK_SAVE_NORMAL_MS     30000
#define FLASH_DATA_BLOCK_SAVE_ALARM_MS      3000
#endif
#define RF_SLEEP_TIMER_MS                   10000

#ifdef VERSION_BTT
#define START_INIT_TIMER_MS					10
#define UART_REC_TIMEOUT_MS                 100
#define RF_COMMU_TIMEOUT_MS                 10000
#endif

#define TIMER1_ID_COUNTS	TIMER1_ID_COUNTS_MAX
#define TIMER_EVENT_MAX		(TIMER1_ID_COUNTS)

#endif


/*
 * SOC.h
 * 
 * 
 * 
 */

#ifndef _SOC_H_
#define _SOC_H_
#include "main.h"
#include "public.h"
#include "config.h"

#define BATTERY_TOTAL_CAPACITY  (BATTERY_TOTAL_CAPACITY_mAH*3600*10L)//100ms

#define BATTERY_PERCENT_1    (BATTERY_TOTAL_CAPACITY/100L)

#define BATTERY_PERCENT_MIN    (BATTERY_PERCENT_1*80/100L)

#define BATTERY_PERCENT_MAX    (BATTERY_PERCENT_1*120/100L)




extern tBatteryOfSocProperty g_BatteryOfSoc;

void InitBatteryOfSoc(void);
void InitMetering(void);
void CalcSOC(U32 direction, S32 soc_per_x);

void SelfConsumeProcess(unsigned char runFlag);
void CalcRelativeStateOfCharge(void);
void CalcCycleCount(void);
int FindTableSOCvalue(unsigned int value,char tempc);
void FSM_SOC_Proc(void);

#endif //

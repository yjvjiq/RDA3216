/***************************************************************
 * Data_Sampling.h
 * Eco-EV BMS
 *
 * Created by Hunk on 14-12-16.
 * Copyright (c) 2014 Boston-Power. All rights reserved.
 **************************************************************/

#ifndef _DATA_SAMPLING_H_
#define _DATA_SAMPLING_H_

#include "types.h"

/******************************************************************* 
 * 数据采集，通过SPI从前端采集芯片读取单体电压，单体温度
 * @data[out]    采集结果保存目标
 * @retval  无 
 *******************************************************************/ 
U8 ISL_Init(void);

// #ifdef VERSION_BMS
void Data_Sampling_AFE_Main(Sample_Data *data);
// #endif
// #ifdef VERSION_BTT
// U8 Data_Sampling_AFE_Main(Sample_Data *data);
// #endif

#endif

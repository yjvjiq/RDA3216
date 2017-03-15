#ifndef _SELF_CHECK_H_
#define _SELF_CHECK_H_
#include "drivers.h"
#ifdef PRODUCTION_TEST
// typedef __packed struct{
// 	U8	FlashCheck	:1;
// 	U8	CcCheck		:1;
// 	U8	CpCheck		:1;
// 	U8	AirbagCheck		:1;
// 	U8	IgnCheck		:1;
// 	U8	CanCheck		:1;
// }s_SelfCheck;

// void SelfCheckProc(void);
// void FlashSelfCheckProc(void);
typedef __packed struct{
    uint8_t mos_k1 :1;  // 1: mos on  0:mos off
    uint8_t mos_k2 :1;  // 1: mos on  0:mos off
    uint8_t flash :1; //FLASH自检 1：执行  0：不执行
    uint8_t rsvd : 5;
}scCmdToSlave;

typedef  __packed struct {
    uint16_t cell_volts[19]; //19个电芯电压值
    uint16_t total_volt; //总压值
    uint8_t cell_T[2]; //2个温度值
    uint16_t current;  //电流值
    uint8_t flash :1; //FLASH自检 1：通过  0：未通过
    uint8_t rsvd : 7;
}scTestResult;

typedef  __packed struct{
	uint8_t tx_power; //发射功率
	uint32_t cycle; //发射周期
	uint8_t freq_index;//发送/接收频点
	uint8_t tr_switch; // 1:发射模式 0：接收模式
}scRfSet;

typedef __packed struct{
    uint32_t tx;  //测试板发送帧数
    uint32_t expect_rx;  //测试板期望接收帧数
    uint32_t rx;  //测试板接收帧数
    uint32_t offline; //丢包帧数
    uint32_t crc_err; //CRC校验错误帧数
	int8_t bgn;   //底躁值
	int8_t rssi;  //接收信号强度
	int8_t cinr;  //载波干扰噪声比
    uint8_t lqi;  //连接质量
}scRfAck;

void scSetCmdHandler(void* data);
void scRfTestCmdHandler(void* data);
#endif
#endif

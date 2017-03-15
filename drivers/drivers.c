#include "derivative.h"
#include "drivers.h"
#include "main.h"
#include "timer.h"
#include "public.h"
#include "w25q64.h"
#include "isl28022.h"
#include "can_service.h"
#include "public.h"
#include "Data_Process.h"
#include "soc.h"
#include "kbi.h"
#include "cc1120.h"
#include "rf_service.h"
#include "debug.h"
#include "uartdata_service.h"
#include "alert.h"


void Crc_MakeCrc(U8 *ptr, U32 len, uint32_t *result)
{
    *result = CRC_Cal16(0x1021, ptr, len);
}

void RTC_NormalInit(void)
{
    //RTC init 1s int
    RTC_ConfigType RTC_con;
    RTC_con.bClockSource = 0x3;
    RTC_con.bClockPresaler = 0x02;
    RTC_con.bFlag = 1;
    RTC_con.bInterruptEn = 1;
    RTC_con.bRTCOut = 0;
    RTC_con.u16ModuloValue = 0x7a12;
    RTC_Init(&RTC_con);
    /* NVIC_IPR5: PRI_20=1 */
    NVIC_IPR5 = (uint32_t)((NVIC_IPR5 & (uint32_t)~(uint32_t)(
                                NVIC_IP_PRI_20(0x02)
                            )) | (uint32_t)(
                               NVIC_IP_PRI_20(0x01)
                           ));
    /* NVIC_ISER: SETENA31=0,SETENA30=0,SETENA29=0,SETENA28=0,SETENA27=0,SETENA26=0,SETENA25=0,SETENA24=0,SETENA23=0,SETENA22=0,SETENA21=0,SETENA20=1,SETENA19=0,SETENA18=0,SETENA17=0,SETENA16=0,SETENA15=0,SETENA14=0,SETENA13=0,SETENA12=0,SETENA11=0,SETENA10=0,SETENA9=0,SETENA8=0,SETENA7=0,SETENA6=0,SETENA5=0,SETENA4=0,SETENA3=0,SETENA2=0,SETENA1=0,SETENA0=0 */
    NVIC_ISER = NVIC_ISER_SETENA20_MASK;
    /* NVIC_ICER: CLRENA31=0,CLRENA30=0,CLRENA29=0,CLRENA28=0,CLRENA27=0,CLRENA26=0,CLRENA25=0,CLRENA24=0,CLRENA23=0,CLRENA22=0,CLRENA21=0,CLRENA20=0,CLRENA19=0,CLRENA18=0,CLRENA17=0,CLRENA16=0,CLRENA15=0,CLRENA14=0,CLRENA13=0,CLRENA12=0,CLRENA11=0,CLRENA10=0,CLRENA9=0,CLRENA8=0,CLRENA7=0,CLRENA6=0,CLRENA5=0,CLRENA4=0,CLRENA3=0,CLRENA2=0,CLRENA1=0,CLRENA0=0 */
    NVIC_ICER = 0x00U;
}

void RTC_LowPowerInit(void)
{
    // RTC_MOD = RTC_MOD_MOD(MCU_LOW_POWER_WAKE_INTERVAL);  //interrupt per MCU_LOW_POWER_WAKE_INTERVAL sec
    // RTC_SC = RTC_SC_RTCLKS(0x01) |  //LP0 1KHZ
    //          RTC_SC_RTCPS(0x07) |            //1KHZ/1000 = 1HZ
    //          RTC_SC_RTIF_MASK |
    //          RTC_SC_RTIE_MASK;
    RTC_ConfigType RTC_con;
    RTC_con.bClockSource = 0x01;
    RTC_con.bClockPresaler = 0x07;
    RTC_con.bFlag = 1;
    RTC_con.bInterruptEn = 1;
    RTC_con.bRTCOut = 0;
    RTC_con.u16ModuloValue = MCU_LOW_POWER_WAKE_INTERVAL;
    RTC_Init(&RTC_con);
    /* NVIC_IPR5: PRI_20=1 */
    NVIC_IPR5 = (uint32_t)((NVIC_IPR5 & (uint32_t)~(uint32_t)(
                                NVIC_IP_PRI_20(0x02)
                            )) | (uint32_t)(
                               NVIC_IP_PRI_20(0x01)
                           ));
    /* NVIC_ISER: SETENA31=0,SETENA30=0,SETENA29=0,SETENA28=0,SETENA27=0,SETENA26=0,SETENA25=0,SETENA24=0,SETENA23=0,SETENA22=0,SETENA21=0,SETENA20=1,SETENA19=0,SETENA18=0,SETENA17=0,SETENA16=0,SETENA15=0,SETENA14=0,SETENA13=0,SETENA12=0,SETENA11=0,SETENA10=0,SETENA9=0,SETENA8=0,SETENA7=0,SETENA6=0,SETENA5=0,SETENA4=0,SETENA3=0,SETENA2=0,SETENA1=0,SETENA0=0 */
    NVIC_ISER = NVIC_ISER_SETENA20_MASK;
    /* NVIC_ICER: CLRENA31=0,CLRENA30=0,CLRENA29=0,CLRENA28=0,CLRENA27=0,CLRENA26=0,CLRENA25=0,CLRENA24=0,CLRENA23=0,CLRENA22=0,CLRENA21=0,CLRENA20=0,CLRENA19=0,CLRENA18=0,CLRENA17=0,CLRENA16=0,CLRENA15=0,CLRENA14=0,CLRENA13=0,CLRENA12=0,CLRENA11=0,CLRENA10=0,CLRENA9=0,CLRENA8=0,CLRENA7=0,CLRENA6=0,CLRENA5=0,CLRENA4=0,CLRENA3=0,CLRENA2=0,CLRENA1=0,CLRENA0=0 */
    NVIC_ICER = 0x00U;

}

void Crc_init(void)
{
//    CRC_ConfigType CRC_Con;

//    CRC_Init(&CRC_Con);
    SIM->SCGC |= SIM_SCGC_CRC_MASK;
    CRC0->CTRL = (CRC_CTRL_TOT(0x00) | CRC_CTRL_TOTR(0x00) | CRC_CTRL_WAS_MASK); /* Setup CCITT mode */
    /* CRC_DATA: LU=0xFF,LL=0xFF */
    CRC0->DATA |= (CRC_DATA_LU(0xFF) | CRC_DATA_LL(0xFF)); /* Setup seed */
    /* CRC_GPOLY: LOW=0x1021 */
    CRC0->GPOLY = (uint32_t)((CRC_GPOLY & (uint32_t)~(uint32_t)(
                                  CRC_GPOLY_LOW(0xEFDE)
                              )) | (uint32_t)(
                                 CRC_GPOLY_LOW(0x1021)
                             ));                      /* Setup polynomial */
    /* CRC_CTRL: WAS=0 */
    CRC0->CTRL &= (uint32_t)~(uint32_t)(CRC_CTRL_WAS_MASK); /* Clear seed bit */

}

void I2c_init(void)
{
//    I2C_ConfigType I2C_Con;
//    I2C_Con.sSetting.bI2CEn = 1;
//    I2C_Init(I2C0,&I2C_Con);
    /* SIM_SCGC: I2C0=1 */
    SIM_SCGC |= SIM_SCGC_I2C0_MASK;
    /* I2C0_C1: IICEN=0,IICIE=0,MST=0,TX=0,TXAK=0,RSTA=0,WUEN=0,??=0 */
    I2C0_C1 = 0x00U;                     /* Clear control register */
    I2C0_C1 = I2C_C1_IICEN_MASK;
    /* I2C0_FLT: SHEN=0,STOPF=1,SSIE=0,STARTF=1,FLT=0 */
    I2C0_FLT = (I2C_FLT_STOPF_MASK | I2C_FLT_STARTF_MASK | I2C_FLT_FLT(0x00)); /* Clear bus status interrupt flags */
    /* I2C0_S1: TCF=0,IAAS=0,BUSY=0,ARBL=0,RAM=0,SRW=0,IICIF=1,RXAK=0 */
    I2C0_S1 = I2C_S_IICIF_MASK;          /* Clear interrupt flag */
    /* SIM_PINSEL0: I2C0PS=0 */
    SIM_PINSEL0 &= (uint32_t)~(uint32_t)(SIM_PINSEL_I2C0PS_MASK);
    /* I2C0_C2: GCAEN=0,ADEXT=0,??=0,SBRC=0,RMEN=0,AD=0 */
    I2C0_C2 = I2C_C2_AD(0x00);
    /* I2C0_FLT: SHEN=0,STOPF=0,SSIE=0,STARTF=0,FLT=0 */
    I2C0_FLT = I2C_FLT_FLT(0x00);        /* Set glitch filter register */
    /* I2C0_SMB: FACK=0,ALERTEN=0,SIICAEN=0,TCKSEL=0,SLTF=1,SHTF1=0,SHTF2=0,SHTF2IE=0 */
    I2C0_SMB = I2C_SMB_SLTF_MASK;
    /* I2C0_F: MULT=2,ICR=0 */
    I2C0_F = (I2C_F_MULT(0x02) | I2C_F_ICR(0x00)); /* Set prescaler bits */
}

void Ics_init(void)
{
    //init sys clock 8M
    ICS_ConfigType ICS_con;
    ICS_con.bLPEnable = 1;
    ICS_con.oscConfig.bEnable = 1;
    ICS_con.oscConfig.bRange = 1;
    ICS_con.oscConfig.bStopEnable = 0;
    ICS_con.oscConfig.bGain = 1;
    ICS_con.oscConfig.bWaitInit = 1;
    ICS_con.u32ClkFreq = FREQ_SYS;
    ICS_con.u8ClkMode = ICS_CLK_MODE_FBE_OSC;
    ICS_Init(&ICS_con);
    FBE_to_FBELP(&ICS_con);
    ICS->C1 &= ~(ICS_C1_IREFSTEN_MASK | ICS_C1_IRCLKEN_MASK);

}

void Uart_init(void)
{
    //UART0 38400
    UART_ConfigType UART_Config;
    UART_Config.u32Baudrate = FREQ_UART0;
    UART_Config.u32SysClkHz = FREQ_SYS;
    UART_Config.sSettings.bEnable = 1;
    UART_Init(UART0, &UART_Config);
    // UART0->BDH |= UART_BDH_RXEDGIE_MASK;
    UART_EnableInterrupt(UART0,UART_RxBuffFullInt);
    NVIC_EnableIRQ(UART0_IRQn);
    UART_SetCallback(UartCmdHandler);
    /* NVIC_IPR3: PRI_12=1 */
    NVIC_IPR3 = (uint32_t)((NVIC_IPR3 & (uint32_t)~(uint32_t)(
                                NVIC_IP_PRI_12(0x02)
                            )) | (uint32_t)(
                               NVIC_IP_PRI_12(0x01)
                           ));
    /* NVIC_ISER: SETENA31=0,SETENA30=0,SETENA29=0,SETENA28=0,SETENA27=0,SETENA26=0,SETENA25=0,SETENA24=0,SETENA23=0,SETENA22=0,SETENA21=0,SETENA20=0,SETENA19=0,SETENA18=0,SETENA17=0,SETENA16=0,SETENA15=0,SETENA14=0,SETENA13=0,SETENA12=1,SETENA11=0,SETENA10=0,SETENA9=0,SETENA8=0,SETENA7=0,SETENA6=0,SETENA5=0,SETENA4=0,SETENA3=0,SETENA2=0,SETENA1=0,SETENA0=0 */
    NVIC_ISER = NVIC_ISER_SETENA12_MASK;
    /* NVIC_ICER: CLRENA31=0,CLRENA30=0,CLRENA29=0,CLRENA28=0,CLRENA27=0,CLRENA26=0,CLRENA25=0,CLRENA24=0,CLRENA23=0,CLRENA22=0,CLRENA21=0,CLRENA20=0,CLRENA19=0,CLRENA18=0,CLRENA17=0,CLRENA16=0,CLRENA15=0,CLRENA14=0,CLRENA13=0,CLRENA12=0,CLRENA11=0,CLRENA10=0,CLRENA9=0,CLRENA8=0,CLRENA7=0,CLRENA6=0,CLRENA5=0,CLRENA4=0,CLRENA3=0,CLRENA2=0,CLRENA1=0,CLRENA0=0 */
    NVIC_ICER = 0x00U;

}

void Pit_init(void)
{
    //PIT0 init 100ms int
    PIT_ConfigType PIT0_con;
    PIT0_con.bModuleDis = 0;
    PIT0_con.bFreeze = 1;
    PIT0_con.bFlag = 0;
    PIT0_con.u32LoadValue = 0xc34ff;
    PIT0_con.bTimerEn = 1;
    PIT0_con.bChainMode = 0;
    PIT0_con.bInterruptEn = 0;
    PIT_Init(0, &PIT0_con);
    /* NVIC_IPR5: PRI_22=1 */
    NVIC_IPR5 = (uint32_t)((NVIC_IPR5 & (uint32_t)~(uint32_t)(
                                NVIC_IP_PRI_22(0x02)
                            )) | (uint32_t)(
                               NVIC_IP_PRI_22(0x01)
                           ));
    /* NVIC_ISER: SETENA31=0,SETENA30=0,SETENA29=0,SETENA28=0,SETENA27=0,SETENA26=0,SETENA25=0,SETENA24=0,SETENA23=0,SETENA22=1,SETENA21=0,SETENA20=0,SETENA19=0,SETENA18=0,SETENA17=0,SETENA16=0,SETENA15=0,SETENA14=0,SETENA13=0,SETENA12=0,SETENA11=0,SETENA10=0,SETENA9=0,SETENA8=0,SETENA7=0,SETENA6=0,SETENA5=0,SETENA4=0,SETENA3=0,SETENA2=0,SETENA1=0,SETENA0=0 */
    NVIC_ISER = NVIC_ISER_SETENA22_MASK;
    /* NVIC_ICER: CLRENA31=0,CLRENA30=0,CLRENA29=0,CLRENA28=0,CLRENA27=0,CLRENA26=0,CLRENA25=0,CLRENA24=0,CLRENA23=0,CLRENA22=0,CLRENA21=0,CLRENA20=0,CLRENA19=0,CLRENA18=0,CLRENA17=0,CLRENA16=0,CLRENA15=0,CLRENA14=0,CLRENA13=0,CLRENA12=0,CLRENA11=0,CLRENA10=0,CLRENA9=0,CLRENA8=0,CLRENA7=0,CLRENA6=0,CLRENA5=0,CLRENA4=0,CLRENA3=0,CLRENA2=0,CLRENA1=0,CLRENA0=0 */
    NVIC_ICER = 0x00U;

    //PIT1 init 10ms int
    PIT_ConfigType PIT1_con;
    PIT1_con.bModuleDis = 0;
    PIT1_con.bFreeze = 1;
    PIT1_con.bFlag = 0;
    PIT1_con.u32LoadValue = 0x1387f;
    PIT1_con.bTimerEn = 1;
    PIT1_con.bChainMode = 0;
    PIT1_con.bInterruptEn = 1;
    PIT_Init(1, &PIT1_con);
    /* NVIC_IPR5: PRI_22=1 */
    NVIC_IPR5 = (uint32_t)((NVIC_IPR5 & (uint32_t)~(uint32_t)(
                                NVIC_IP_PRI_23(0x02)
                            )) | (uint32_t)(
                               NVIC_IP_PRI_23(0x01)
                           ));
    /* NVIC_ISER: SETENA31=0,SETENA30=0,SETENA29=0,SETENA28=0,SETENA27=0,SETENA26=0,SETENA25=0,SETENA24=0,SETENA23=0,SETENA22=1,SETENA21=0,SETENA20=0,SETENA19=0,SETENA18=0,SETENA17=0,SETENA16=0,SETENA15=0,SETENA14=0,SETENA13=0,SETENA12=0,SETENA11=0,SETENA10=0,SETENA9=0,SETENA8=0,SETENA7=0,SETENA6=0,SETENA5=0,SETENA4=0,SETENA3=0,SETENA2=0,SETENA1=0,SETENA0=0 */
    NVIC_ISER = NVIC_ISER_SETENA23_MASK;
    /* NVIC_ICER: CLRENA31=0,CLRENA30=0,CLRENA29=0,CLRENA28=0,CLRENA27=0,CLRENA26=0,CLRENA25=0,CLRENA24=0,CLRENA23=0,CLRENA22=0,CLRENA21=0,CLRENA20=0,CLRENA19=0,CLRENA18=0,CLRENA17=0,CLRENA16=0,CLRENA15=0,CLRENA14=0,CLRENA13=0,CLRENA12=0,CLRENA11=0,CLRENA10=0,CLRENA9=0,CLRENA8=0,CLRENA7=0,CLRENA6=0,CLRENA5=0,CLRENA4=0,CLRENA3=0,CLRENA2=0,CLRENA1=0,CLRENA0=0 */
    NVIC_ICER = 0x00U;

}

/*  1 == fall edge, 0 == rising edge */
void SPI0_PhaseSet(U8 FLG)
{
	SPI_Phase_Set(SPI0, FLG);
}

void SPI0_BaudRate_Set(SPI_Baudrate para){
	SPI0->BR = SPI_BR_SPPR(0x00) | SPI_BR_SPR(para);
}
void Spi_init(void)
{
    //SPI0 init
    SPI_ConfigType SPI0_Con;
    SPI0_Con.u32BusClkHz = FREQ_SYS;
    SPI0_Con.u32BitRate = FREQ_SPI0;
    SPI0_Con.sSettings.bBidirectionModeEn = 0;
    SPI0_Con.sSettings.bClkPhase1 = 0;//ÉÏÉý
    //SPI0_Con.sSettings.bClkPhase1 = 1;//ÏÂ½µÑØ
    SPI0_Con.sSettings.bClkPolarityLow = 0;
    SPI0_Con.sSettings.bIntEn = 0;
    SPI0_Con.sSettings.bMasterAutoDriveSS = 0;
    SPI0_Con.sSettings.bMasterMode = 1;
    SPI0_Con.sSettings.bMatchIntEn = 0;
    SPI0_Con.sSettings.bModeFaultEn = 0;
    SPI0_Con.sSettings.bModuleEn = 1;
    SPI0_Con.sSettings.bPinAsOuput = 0;
    SPI0_Con.sSettings.bShiftLSBFirst = 0;
    SPI0_Con.sSettings.bStopInWaitMode = 0;
    SPI0_Con.sSettings.bTxIntEn = 0;
    SPI_Init(SPI0, &SPI0_Con);
	//SPI0->BR = SPI_BR_SPPR(0x00) | SPI_BR_SPR(0x01); // 2MHz
//	SPI0->BR = SPI_BR_SPPR(0x00) | SPI_BR_SPR(0x02); // 1MHz
	SPI0->BR = SPI_BR_SPPR(0x00) | SPI_BR_SPR(0x03); // 500KHz
	//SPI0->BR = SPI_BR_SPPR(0x00) | SPI_BR_SPR(0x05); // 125kHz
	
    SPI0->C1 &= ~SPI_C1_CPHA_MASK;
    //SPI1 init
    SPI_ConfigType SPI1_Con;
    SPI1_Con.u32BusClkHz = FREQ_SYS;
    SPI1_Con.u32BitRate = FREQ_SPI0;
    SPI1_Con.sSettings.bBidirectionModeEn = 0;
    SPI1_Con.sSettings.bClkPhase1 = 0;
    SPI1_Con.sSettings.bClkPolarityLow = 0;
    SPI1_Con.sSettings.bIntEn = 0;
    SPI1_Con.sSettings.bMasterAutoDriveSS = 0;
    SPI1_Con.sSettings.bMasterMode = 1;
    SPI1_Con.sSettings.bMatchIntEn = 0;
    SPI1_Con.sSettings.bModeFaultEn = 0;
    SPI1_Con.sSettings.bModuleEn = 1;
    SPI1_Con.sSettings.bPinAsOuput = 0;
    SPI1_Con.sSettings.bShiftLSBFirst = 0;
    SPI1_Con.sSettings.bStopInWaitMode = 1;
    SPI1_Con.sSettings.bTxIntEn = 0;
    SPI_Init(SPI1, &SPI1_Con);
    SPI1->BR = SPI_BR_SPPR(0x00) | SPI_BR_SPR(0x01);
    SPI1->C1 &= ~SPI_C1_CPHA_MASK;
}

void Can_init(void)
{
//    CAN_SetRxIDFilterTable(0,0X68ffffff);
//    CAN_SetAcceptanceMaskTable(0,0X07ffffff);
//    CAN_SetRxIDFilterTable(1,0X68ffffff);
//    CAN_SetAcceptanceMaskTable(1,0X07ffffff);
    SIM_PINSEL1 |= SIM_PINSEL1_MSCANPS_MASK;
    CAN_STB_CLEAR;
    MSCAN_ConfigType MSCAN_Con;
    MSCAN_Con.sBaudRateSetting.BRP = 0x03;
    MSCAN_Con.sBaudRateSetting.SJW = 0x00;
    MSCAN_Con.sBaudRateSetting.TSEG1 = 0x03;
    MSCAN_Con.sBaudRateSetting.TSEG2 = 0x02;
    MSCAN_Con.sBaudRateSetting.SAMP = 0;
    MSCAN_Con.sSetting.bBusOffUser = 0;
    MSCAN_Con.sSetting.bCanEn = 1;
    MSCAN_Con.sSetting.bCLKSRC = 1;
    MSCAN_Con.sSetting.bListenModeEn = 0;
    MSCAN_Con.sSetting.bLoopModeEn = 0;
    MSCAN_Con.sSetting.bOverRunIEn = 0;
    MSCAN_Con.sSetting.bRxFullIEn = 1;
    MSCAN_Con.sSetting.bRxStatusChangeIEn = 0;
    MSCAN_Con.sSetting.bStatusChangeIEn = 0;
    MSCAN_Con.sSetting.bStopEn = 1;
    MSCAN_Con.sSetting.bTimerEn = 0;
    MSCAN_Con.sSetting.bTxEmptyIEn = 1;
    MSCAN_Con.sSetting.bTxStatusChangeIEn = 0;
    MSCAN_Con.sSetting.bWakeUpEn = 0;
    MSCAN_Con.sSetting.bWakeUpIEn = 0;
    MSCAN_Con.sSetting.bWUPM = 0;
    MSCAN_Con.u32IDAR0 = 0X68ffffff;
    MSCAN_Con.u32IDAR1 = 0X68ffffff;
    MSCAN_Con.u32IDMR0 = 0X07ffffff;
    MSCAN_Con.u32IDMR1 = 0X07ffffff;
    CAN_Init(MSCAN, &MSCAN_Con);
    /* NVIC_IPR7: PRI_31=1 */
    NVIC_IPR7 = (uint32_t)((NVIC_IPR7 & (uint32_t)~(uint32_t)(
                                NVIC_IP_PRI_31(0x02)
                            )) | (uint32_t)(
                               NVIC_IP_PRI_31(0x01)
                           ));
    /* NVIC_ISER: SETENA31=1,SETENA30=0,SETENA29=0,SETENA28=0,SETENA27=0,SETENA26=0,SETENA25=0,SETENA24=0,SETENA23=0,SETENA22=0,SETENA21=0,SETENA20=0,SETENA19=0,SETENA18=0,SETENA17=0,SETENA16=0,SETENA15=0,SETENA14=0,SETENA13=0,SETENA12=0,SETENA11=0,SETENA10=0,SETENA9=0,SETENA8=0,SETENA7=0,SETENA6=0,SETENA5=0,SETENA4=0,SETENA3=0,SETENA2=0,SETENA1=0,SETENA0=0 */
    NVIC_ISER = NVIC_ISER_SETENA31_MASK;
    /* NVIC_ICER: CLRENA31=0,CLRENA30=0,CLRENA29=0,CLRENA28=0,CLRENA27=0,CLRENA26=0,CLRENA25=0,CLRENA24=0,CLRENA23=0,CLRENA22=0,CLRENA21=0,CLRENA20=0,CLRENA19=0,CLRENA18=0,CLRENA17=0,CLRENA16=0,CLRENA15=0,CLRENA14=0,CLRENA13=0,CLRENA12=0,CLRENA11=0,CLRENA10=0,CLRENA9=0,CLRENA8=0,CLRENA7=0,CLRENA6=0,CLRENA5=0,CLRENA4=0,CLRENA3=0,CLRENA2=0,CLRENA1=0,CLRENA0=0 */
    NVIC_ICER = 0x00U;
    /* NVIC_IPR7: PRI_30=1 */
    NVIC_IPR7 = (uint32_t)((NVIC_IPR7 & (uint32_t)~(uint32_t)(
                                NVIC_IP_PRI_30(0x02)
                            )) | (uint32_t)(
                               NVIC_IP_PRI_30(0x01)
                           ));
    /* NVIC_ISER: SETENA31=0,SETENA30=1,SETENA29=0,SETENA28=0,SETENA27=0,SETENA26=0,SETENA25=0,SETENA24=0,SETENA23=0,SETENA22=0,SETENA21=0,SETENA20=0,SETENA19=0,SETENA18=0,SETENA17=0,SETENA16=0,SETENA15=0,SETENA14=0,SETENA13=0,SETENA12=0,SETENA11=0,SETENA10=0,SETENA9=0,SETENA8=0,SETENA7=0,SETENA6=0,SETENA5=0,SETENA4=0,SETENA3=0,SETENA2=0,SETENA1=0,SETENA0=0 */
    NVIC_ISER = NVIC_ISER_SETENA30_MASK;
    /* NVIC_ICER: CLRENA31=0,CLRENA30=0,CLRENA29=0,CLRENA28=0,CLRENA27=0,CLRENA26=0,CLRENA25=0,CLRENA24=0,CLRENA23=0,CLRENA22=0,CLRENA21=0,CLRENA20=0,CLRENA19=0,CLRENA18=0,CLRENA17=0,CLRENA16=0,CLRENA15=0,CLRENA14=0,CLRENA13=0,CLRENA12=0,CLRENA11=0,CLRENA10=0,CLRENA9=0,CLRENA8=0,CLRENA7=0,CLRENA6=0,CLRENA5=0,CLRENA4=0,CLRENA3=0,CLRENA2=0,CLRENA1=0,CLRENA0=0 */
    NVIC_ICER = 0x00U;

}

void Kbi_init(void)
{
    //kbi0
    SIM->SCGC   |= SIM_SCGC_KBI0_MASK;
    CONFIG_PIN_AS_GPIO(PTC, PTC7, INPUT);
    ENABLE_INPUT(PTC, PTC7);
    KBI_DisableInt(KBI0);
    KBI0->ES = 0;
    ENABLE_PULLUP(PTC, PTC7);
    KBI0->PE |= 0x00800000;
    KBI_ClrFlags(KBI0);
    KBI_RstSP(KBI0);
    KBI_EnableInt(KBI0);
    NVIC_EnableIRQ(KBI0_IRQn);
    NVIC_IPR6 = (uint32_t)((NVIC_IPR6 & (uint32_t)~(uint32_t)(
                                NVIC_IP_PRI_24(0x02)
                            )) | (uint32_t)(
                               NVIC_IP_PRI_24(0x01)
                           ));
    NVIC_ISER = NVIC_ISER_SETENA24_MASK;
    NVIC_ICER = 0x00U; 
}

void Irq_init(void)
{
    SIM->SCGC   |= SIM_SCGC_IRQ_MASK;
    SIM->PINSEL |= 0x5;

//    SIM_SCGC |= SIM_SCGC_IRQ_MASK;
    IRQ_SC &= ~IRQ_SC_IRQIE_MASK;
    IRQ_SC |= IRQ_SC_IRQPE_MASK;
    /* Clear interrupt status flag */
    /* IRQ_SC: IRQACK=1 */
    IRQ_SC |= IRQ_SC_IRQACK_MASK;
    /* IRQ_SC: ??=0,IRQPDD=0,IRQEDG=0,IRQPE=1,IRQF=0,IRQACK=0,IRQIE=1,IRQMOD=0 */
    IRQ_SC |=  IRQ_SC_IRQIE_MASK;
//    IRQ->SC |= IRQ_SC_IRQACK_MASK;
    NVIC_EnableIRQ(IRQ_IRQn);
    /* NVIC_IPR1: PRI_7=1 */
    NVIC_IPR1 = (uint32_t)((NVIC_IPR1 & (uint32_t)~(uint32_t)(
                                NVIC_IP_PRI_7(0x02)
                            )) | (uint32_t)(
                               NVIC_IP_PRI_7(0x01)
                           ));
    /* NVIC_ISER: SETENA31=0,SETENA30=0,SETENA29=0,SETENA28=0,SETENA27=0,SETENA26=0,SETENA25=0,SETENA24=0,SETENA23=0,SETENA22=0,SETENA21=0,SETENA20=0,SETENA19=0,SETENA18=0,SETENA17=0,SETENA16=0,SETENA15=0,SETENA14=0,SETENA13=0,SETENA12=0,SETENA11=0,SETENA10=0,SETENA9=0,SETENA8=0,SETENA7=1,SETENA6=0,SETENA5=0,SETENA4=0,SETENA3=0,SETENA2=0,SETENA1=0,SETENA0=0 */
    NVIC_ISER = NVIC_ISER_SETENA7_MASK;
    /* NVIC_ICER: CLRENA31=0,CLRENA30=0,CLRENA29=0,CLRENA28=0,CLRENA27=0,CLRENA26=0,CLRENA25=0,CLRENA24=0,CLRENA23=0,CLRENA22=0,CLRENA21=0,CLRENA20=0,CLRENA19=0,CLRENA18=0,CLRENA17=0,CLRENA16=0,CLRENA15=0,CLRENA14=0,CLRENA13=0,CLRENA12=0,CLRENA11=0,CLRENA10=0,CLRENA9=0,CLRENA8=0,CLRENA7=0,CLRENA6=0,CLRENA5=0,CLRENA4=0,CLRENA3=0,CLRENA2=0,CLRENA1=0,CLRENA0=0 */
    NVIC_ICER = 0x00U;
}

void Wdog_init(void)
{
    WDOG_ConfigType WDOG_Con;
    WDOG_Con.sBits.bIntEnable = 1;
    WDOG_Con.sBits.bDisable = 0;
    WDOG_Con.sBits.bStopEnable = 1;
    WDOG_Con.sBits.bDbgEnable = 0;
    WDOG_Con.sBits.bWinEnable = 0;
    WDOG_Con.sBits.bUpdateEnable = 1;
    WDOG_Con.sBits.bClkSrc = 0x01;
    WDOG_Con.sBits.bPrescaler = 0;
#ifdef VERSION_BMS
    WDOG_Con.u16TimeOut = 5000; //1000ms
#endif
#ifdef VERSION_BTT
	WDOG_Con.u16TimeOut = 2500; //1000ms
#endif
    WDOG_Init(&WDOG_Con);
}

void Wdog_LowpoerInit(void)
{
    WDOG_ConfigType WDOG_Con;
    WDOG_Con.sBits.bIntEnable = 1;
    WDOG_Con.sBits.bDisable = 0;
    WDOG_Con.sBits.bStopEnable = 1;
    WDOG_Con.sBits.bDbgEnable = 0;
    WDOG_Con.sBits.bWinEnable = 0;
    WDOG_Con.sBits.bUpdateEnable = 1;
    WDOG_Con.sBits.bClkSrc = 0x01;
    WDOG_Con.sBits.bPrescaler = 1;
    WDOG_Con.u16TimeOut = 0xffff; //4.5h
    // WDOG_Con.sBits.bPrescaler = 1;
    // WDOG_Con.u16TimeOut = 20; //1000ms
    WDOG_Init(&WDOG_Con);
}

void CPU_Sleep(void)
{
    /* SCB_SCR: SLEEPDEEP=1 */
    SCB_SCR |= SCB_SCR_SLEEPDEEP_MASK; /* STOP mode can be entered after WFI instruction call */
    /* SCB_SCR: SLEEPONEXIT=0 */
    SCB_SCR &= (uint32_t)~(uint32_t)(SCB_SCR_SLEEPONEXIT_MASK); /* Do not enter stop state on ISR exit */
    __wfi();
}

void MCU_Sleep(void)
{
////    __disable_irq();
//    IRQ_SC &= ~IRQ_SC_IRQIE_MASK;
//    RTC_LowPowerInit();
//    g_mcu_mode = SLEEP;
//    CPU_Sleep();
//    while(g_mcu_mode == SLEEP) {};
//    g_log_data.log_data.UinxTime += RTC_CNT;
//    RTC_NormalInit();      //change to normal setting
//    IRQ_SC |= IRQ_SC_IRQIE_MASK;

}

void User_Init(void)
{
#ifdef WATCH_DOG_ENABLE
    WDOG_Feed();
#endif
    __disable_irq();
    SIM_SOPT0 &= ~(uint32_t)SIM_SOPT0_NMIE_MASK; /* disable NMI pin */
    SIM_SOPT0 |= (uint32_t)SIM_SOPT0_RSTPE_MASK; /* Enable Reset pin */
    SIM_SCGC |= (uint32_t)SIM_SCGC_SWD_MASK;  //SWD pins clock gate enable 
    SIM_SOPT0 |= (uint32_t)SIM_SOPT0_SWDE_MASK; /* Enable SWD pins */
    Ics_init();
    Uart_init();
    NVIC_DisableIRQ(UART0_IRQn);
    Spi_init();
    GPIO_Init();
    Pit_init();
    RTC_NormalInit();
    Crc_init();
    I2c_init();
#ifdef CAN_COMMUNICATION
    Can_init();
#endif
#ifdef WATCH_DOG_ENABLE
    WDOG_Feed();
#endif
#ifdef WATCH_DOG_ENABLE
    WDOG_Feed();
#endif
    __enable_irq();
    NVIC_EnableIRQ(UART0_IRQn);
	
}


void delay_ms(U32 ms)
{
    U32 i = 0, j = 0;
    for(j = 0; j < ms; j++) {
        for(i = 0; i < 1024; i++) {
            __nop();
        }
    }
}

/***********************************************************************************************
*
* @brief    Spi_WrString - Send a string on SPI0
* @param    string to send
* @return   none
*
************************************************************************************************/
void Spi_WrString(SPI_MemMapPtr SPIP, unsigned char* pWrBuff, unsigned char uiLength)
{
    unsigned char i;
    volatile char dumay;
    __disable_irq();
	
	SPI0_PhaseSet(0); // rising edge
	delay_ms(10);
	MASTER_SPI_CS_ON();
	for( i = 0; i < uiLength; i++) {
        while(!(SPIP->S & SPI_S_SPTEF_MASK) );               //SPIP_S_SPTEF_MASK=send empty flag
        SPIP->D = pWrBuff[i];
        while(!(SPIP->S & SPI_S_SPRF_MASK) );                   //SPIP_S_SPRF=receive full flag
        dumay = SPIP->D;
    }
	MASTER_SPI_CS_OFF();
    __enable_irq();
}

/***********************************************************************************************
*
* @brief    Spi0_ReadString - Receive a string on SPI0
* @param    none
* @return   TRUE if no error / FALSE if time out
*
************************************************************************************************/
Bool Spi0_ReadBytes(unsigned char* pRdBuff, unsigned char uiLength)
{	
	SPI0_PhaseSet(1); // falling edge
	delay_ms(10);
	
	MASTER_SPI_CS_ON();
	__disable_irq();
	while(!(SPI0_BASE_PTR->S & SPI_S_SPTEF_MASK) );           //SPIP_S_SPTEF_MASK=send empty flag
	SPI0_BASE_PTR->D = 0x00;
	//U8 T = SPI0_BASE_PTR->D;
	while(!(SPI0_BASE_PTR->S & SPI_S_SPRF_MASK) );           //SPIP_S_SPRF=receive full flag
	*pRdBuff++ = SPI0_BASE_PTR->D;
	__enable_irq();
	//MASTER_SPI_CS_OFF();
	return 0;
}

/***********************************************************************************************
*
* @brief    Spi_RdWrByte - Send and receive a byte on SPI
* @param    byte to send
* @return   recieve byte
*
************************************************************************************************/

U8 Spi_RdWrByte(SPI_MemMapPtr SPIP, unsigned char WrByte)
{
    volatile U8 temp;
    U16 RetryCount = 0xfff;
//    __disable_irq();


    while((!(SPIP->S & SPI_S_SPTEF_MASK)) && (RetryCount--) );           //SPIP_S_SPTEF_MASK=send empty flag
    SPIP->D = WrByte;
    while((!(SPIP->S & SPI_S_SPRF_MASK)) && (RetryCount--)  );                   ///SPIP_S_SPRF=receive full flag
    temp = SPIP->D;
	
    return temp;
}

void Spi0_RdBytes(U32 *RdBuf, U8 Len)
{
    volatile U8 temp;
	SPI0_PhaseSet(1); // falling edge
	delay_ms(10);
	
	for(U8 j=0; j<Len; j++){
		*RdBuf = 0;
		MASTER_SPI_CS_ON();
		for(U8 i=0; i<4; i++){
			while(!(SPI0->S & SPI_S_SPTEF_MASK));           //SPIP_S_SPTEF_MASK=send empty flag
			SPI0->D = 0xff;
			while(!(SPI0->S & SPI_S_SPRF_MASK));                   ///SPIP_S_SPRF=receive full flag
			(*RdBuf) |= (SPI0->D<<(8*(3-i)));		
		}
		
		MASTER_SPI_CS_OFF();
		RdBuf++;
	}
}

#ifdef CAN_COMMUNICATION
U8 CAN_SendFrame(LDD_CAN_TFrame *canframe)
{
    MSCAN_RegisterFrameType mscanframe;
    bool IsExtID = 0, IsRTR = 0;
    if((canframe->MessageID & 0x80000000) != 0) {
        IsExtID = 1;
    }
    if(canframe->FrameType == LDD_CAN_REMOTE_FRAME) {
        IsRTR = 1;
    }
    mscanframe.DLR = canframe->Length;
    mscanframe.BPR = canframe->LocPriority;
    memcpy(&mscanframe.EDSR, canframe->Data, 8);
    mscanframe.EIDR0 = (U8)(canframe->MessageID >> 21);
    mscanframe.EIDR1 = (IsExtID << 3) | (((U8)(canframe->MessageID >> 15)) & 0x07) | (((U8)(canframe->MessageID >> 13)) & 0xe0);
    mscanframe.EIDR2 = (U8)(canframe->MessageID >> 7);
    mscanframe.EIDR3 = IsRTR | ((U8)(canframe->MessageID << 1));
    return WriteCanQueue(&g_CanSendQueue,&mscanframe);
//    return (CAN_LoadOneFrameToBuff(MSCAN, &mscanframe));
}

U8 CAN_ReadFrame(LDD_CAN_TFrame *canframe)
{
    MSCAN_FrameType mscanframe;
    mscanframe.ID_Type.ID = 0;
    if (CAN_ReadOneFrameFromBuff(MSCAN, &mscanframe) == FALSE) return FALSE;
    memcpy(canframe->Data, mscanframe.DSR, 8);
    canframe->MessageID = mscanframe.ID_Type.ID;
    canframe->Length = mscanframe.DLR;
    canframe->LocPriority = mscanframe.BPR;
    if(mscanframe.bIsRemoteFrame == 1) {
        canframe->FrameType = LDD_CAN_REMOTE_FRAME;
    }  else canframe->FrameType = LDD_CAN_DATA_FRAME;
    if(mscanframe.bIsExtOrStand == 1) canframe->MessageID |= 0x80000000;
    return TRUE;
}
#endif

void PIT_CH0_IRQHandler(void)
{
//    printf("28022!\n");
//    ISL28022_Interrupt();
//    ISL28022_proc();
    PIT_ChannelClrFlags(0);
}

void PIT_CH1_IRQHandler(void)
{
//    printf("timer1!\n");
    timer1_proc();
//    CC1120_SendTimeoutProc(SetDownlinkParam);
    PIT_ChannelClrFlags(1);
}

void RTC_IRQHandler(void)
{
//    printf("RTC!\n");
#ifdef WATCH_DOG_ENABLE
    Set_IntFlag(RTC_FEED_DOG_POS);
#endif
    if(g_mcu_mode == SLEEP) {
        g_stateblock_data.stata_union.state_data.UinxTime += MCU_LOW_POWER_WAKE_INTERVAL_SET_VAL;
        g_log_data.log_data.UinxTime = g_stateblock_data.stata_union.state_data.UinxTime;
        //re init rf and back sniff mode agian
        // manualCalibration();
        // calibrateRCOsc();
        // SetTimer1(RF_SLEEP_TIMER_ID, 10, TIMER_MODE_SINGLE, NULL);
       // Timer1Event[RF_SLEEP_TIMER_ID].eventflag = TRUE;
    } else if(g_mcu_mode == RUN) {
        g_stateblock_data.stata_union.state_data.UinxTime++;
        g_log_data.log_data.UinxTime = g_stateblock_data.stata_union.state_data.UinxTime;
    }
    RTC_ClrFlags();
}


// void IRQ_IRQHandler(void) //MASTER FAULT int
// {
//     printf("IRQ!\n");
//     IRQ->SC |= IRQ_SC_IRQACK_MASK;
// }

// void KBI1_IRQHandler(void) //28022 int
// {
//     printf("KBI1!\n");
//     KBI1->SC |= KBI_SC_KBACK_MASK;
//     g_mcu_mode = RUN;
// }

void KBI0_IRQHandler(void) //cc1120 int
{
//    printf("KBI0!\n");
    if(!(KBI_GetSP(KBI0) & (0x00800000))){
//        CC1120_ISRHandler();
//        CC1120_SendSuccessHandler(0,RfSleepWaitTimerWake);
//        //    CC1120_SendSuccessHandler(0,SetDownlinkParam);
    }
    KBI_RstSP(KBI0);
    KBI0->SC |= KBI_SC_KBACK_MASK;
}

void UART0_IRQHandler(void)
{
    U8 uart_recdata = 0;

    if(UART_CheckFlag(UART0,UART_FlagOR))
    {
        uart_recdata = UART_ReadDataReg(UART0);     
    }
    if(UART_IsTxComplete(UART0)) {
        DebugSendCycle(&_printf_ring_frame);
    }
    if(UART_IsRxBuffFull(UART0)) {
        uart_recdata = UART_ReadDataReg(UART0);
        SplitUartMsg(&uart_recdata, 1, &g_uartrecframe, NULL);
#ifdef VERSION_BTT
#ifdef BTT_RF
        RecUartMsg(&uart_recdata,&g_uartrecframebtt);
#endif
#ifdef  BTT_OFFLINE
        SplitUartBttMsg(&uart_recdata, 1, &g_uartrecframebtt,NULL);
#endif
#endif
    }
    // UART0->S2 |= UART_S2_RXEDGIF_MASK;
}

void WDOG_IRQHandler()
{
//    W25Q64_SetStateMSG(&g_stateblock_data);
}

#ifdef CAN_COMMUNICATION
void MSCAN_RX_IRQHandler(void)
{
    CAN_ReadFrame(&g_can_recframe);
//    CAN_SendFrame(&g_can_recframe);
    CanCmdParsingProc(&g_can_recframe);
    CAN_ClearRXF_Flag(MSCAN);
    CAN_ClearRXFRM_Flag(MSCAN);
}

void MSCAN_TX_IRQHandler(void)
{
    MSCAN_RegisterFrameType mscanframe;
   if(ReadCanQueue(&g_CanSendQueue,&mscanframe) == TRUE)
   {
    CAN_LoadOneFrameToBuff(MSCAN, &mscanframe);
    }
    else{
    CAN_TransmitterEmptyIntDisable(MSCAN);
    }
}
#endif


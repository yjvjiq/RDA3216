#include "power_manage.h"
#include "drivers.h"
#include "w25q64.h"
#include "isl28022.h"
#include "rf_service.h"

void SysPowerDown(void)
{
    U16 WaitPowerDownCount = 0x0fff;
#ifdef WATCH_DOG_ENABLE
    Wdog_LowpoerInit();
    WDOG_Feed();
#endif
    RTC_LowPowerInit();
    Isl94212_ShutDown();
    SPI_Flash_WAKEUP();
    W25Q64_SetStateMSG(&g_stateblock_data);
    SPI_Flash_PowerDown();
    NVIC_DisableIRQ(PIT_CH0_IRQn);
    NVIC_DisableIRQ(PIT_CH1_IRQn);

    //disable uart0
    NVIC_DisableIRQ(UART0_IRQn);
    //disable i2c0
    NVIC_DisableIRQ(I2C0_IRQn);
    g_mcu_mode = SLEEP;
    CPU_Sleep();
    while(WaitPowerDownCount--);
    do{
		Cpu_Reset();
		delay_ms(1);
		//wait reset
    }while(1);

}

void SysPowerUp(void)
{
    g_mcu_mode = RUN;
    g_stateblock_data.stata_union.state_data.UinxTime += RTC->CNT;
    RTC_NormalInit();
#ifdef WATCH_DOG_ENABLE
    Wdog_init();
    WDOG_Feed();
#endif
    SPI_Flash_WAKEUP();
}


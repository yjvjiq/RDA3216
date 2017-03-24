#include "GPIO.H"

/***********************************************************************************************
*
* @brief    GPIO_Init - Initialize the LEDs as outputs
* @param    none
* @return   none
*
************************************************************************************************/
void GPIO_Init()
{
    SIM_SCGC |= SIM_SCGC_SPI1_MASK;
//	CONFIG_PIN_AS_GPIO(PTE, PTE5, OUTPUT); //MASTER_EN
//	CONFIG_PIN_AS_GPIO(PTD, PTD3, OUTPUT); //FLASH_SPI_CS0
//	CONFIG_PIN_AS_GPIO(PTH, PTH6, OUTPUT); //CAN_STB
//	CONFIG_PIN_AS_GPIO(PTE, PTE2, OUTPUT); //FLASH_SPI_CS1
//	CONFIG_PIN_AS_GPIO(PTE, PTE3, OUTPUT); //CC1120_Reset
//	CONFIG_PIN_AS_GPIO(PTD, PTD2, INPUT); //MISO READ IO
//	ENABLE_INPUT(PTD, PTD2);    //MISO READ IO
    
   

    CONFIG_PIN_AS_GPIO(PTB, PTB5, OUTPUT);//SPI0_CS
    CONFIG_PIN_AS_GPIO(PTF, PTF4, OUTPUT); //RDA3216 conversion pin CNVST
    CONFIG_PIN_AS_GPIO(PTI, PTI4, OUTPUT); //RDA3216 power pin PD_N
    CONFIG_PIN_AS_GPIO(PTE, PTE5, INPUT); //RDA3216 alert pin 

    FGPIOB_PSOR |= 1 << 16;
    FGPIOB_PSOR |= 1 << 17;
    FGPIOB_PSOR |= 1 << 18;

    GPIOB_PDDR |= (1 << 16);
    GPIOB_PCOR = (1 << 16);

	/* RDA related */
	RDA3216_ConvsDone;
	RDA3216_PWRON;
	//RDA3216_ALERTOFF;
}

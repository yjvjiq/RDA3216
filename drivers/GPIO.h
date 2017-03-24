/*
 * GPIO.h
 *
 *  Created on: Sep 24, 2013
 *      Author: B46911
 */

#ifndef GPIO_H_
#define GPIO_H_

/*******************************************************************************
* Includes
*******************************************************************************/
#include "derivative.h"

/*******************************************************************************
* Constants
********************************************************************************/


/*******************************************************************************
* Macros
********************************************************************************/
#define PTA  GPIOA
#define PTB  GPIOA
#define PTC  GPIOA
#define PTD  GPIOA

#define PTE GPIOB
#define PTF GPIOB
#define PTG GPIOB
#define PTH GPIOB

#define PTI GPIOC

#define OUTPUT  1
#define INPUT   0

#define CONFIG_PIN_AS_GPIO(port,port_pin,mode)    XCONFIG_PIN_AS_GPIO(port,port_pin,mode)
#define XCONFIG_PIN_AS_GPIO(port,port_pin,mode)   (mode == 0) ? (port->PDDR |= 0 << port_pin) : (port->PDDR |= 1 << port_pin)

#define ENABLE_INPUT(port,port_pin)                     XENABLE_INPUT(port,port_pin)
#define XENABLE_INPUT(port,port_pin)                    port->PIDR ^= 1<<port_pin

#define ENABLE_PULLUP(port,port_pin)                    XENABLE_PULLUP(port,port_pin)
#define XENABLE_PULLUP(port,port_pin)               (port==GPIOA) ? (PORT->PUE0|= 1<<(uint32_t)(port_pin)) : (PORT->PUE1|= 1<<(uint32_t)(port_pin))

#define OUTPUT_SET(port,register_num)                   XOUTPUT_SET(port,register_num)
#define XOUTPUT_SET(port,register_num)              port->PSOR |=1<<register_num

#define OUTPUT_CLEAR(port,register_num)             XOUTPUT_CLEAR(port,register_num)
#define XOUTPUT_CLEAR(port,register_num)            port->PCOR |=1<<register_num

#define OUTPUT_TOGGLE(port,register_num)            XOUTPUT_TOGGLE(port,register_num)
#define XOUTPUT_TOGGLE(port,register_num)           port->PTOR |=1<<register_num

#define READ_INPUT(port,port_pin)                           XREAD_INPUT(port,port_pin)
#define XREAD_INPUT(port,port_pin)                      (port->PDIR & GPIO_PDIR_PDI(1<<port_pin))

/*******************************************************************************
* Types
********************************************************************************/


/*******************************************************************************
* Variables
********************************************************************************/

/*******************************************************************************
* Global Variables
********************************************************************************/
enum GPIOA_Register {
    PTA0,
    PTA1,
    PTA2,
    PTA3,
    PTA4,
    PTA5,
    PTA6,
    PTA7,
    PTB0,
    PTB1,
    PTB2,
    PTB3,
    PTB4,
    PTB5,
    PTB6,
    PTB7,
    PTC0,
    PTC1,
    PTC2,
    PTC3,
    PTC4,
    PTC5,
    PTC6,
    PTC7,
    PTD0,
    PTD1,
    PTD2,
    PTD3,
    PTD4,
    PTD5,
    PTD6,
    PTD7
};

enum GPIOB_Register {
    PTE0,
    PTE1,
    PTE2,
    PTE3,
    PTE4,
    PTE5,
    PTE6,
    PTE7,
    PTF0,
    PTF1,
    PTF2,
    PTF3,
    PTF4,
    PTF5,
    PTF6,
    PTF7,
    PTG0,
    PTG1,
    PTG2,
    PTG3,
    PTG4,
    PTG5,
    PTG6,
    PTG7,
    PTH0,
    PTH1,
    PTH2,
    PTH3,
    PTH4,
    PTH5,
    PTH6,
    PTH7
};

enum GPIOC_Register{
	PTI0,
	PTI1,
	PTI2,
	PTI3,
	PTI4,
	PTI5,
	PTI6
};

/*******************************************************************************
* Global Functions
********************************************************************************/


#define LED0_TOGGLE     OUTPUT_TOGGLE(PTG,PTG0)
#define LED1_TOGGLE     OUTPUT_TOGGLE(PTG,PTG1)
#define LED2_TOGGLE     OUTPUT_TOGGLE(PTG,PTG2)

/* RDA related */
#define RDA3216_ConvsStart    OUTPUT_CLEAR(PTF,PTF4)
#define RDA3216_ConvsDone    OUTPUT_SET(PTF,PTF4)
#define RDA3216_PWRON  OUTPUT_SET(PTI,PTI4)
#define RDA3216_PWROFF  OUTPUT_CLEAR(PTI,PTI4)
#define RDA3216_ALERTON OUTPUT_SET(PTE,PTE5)
#define RDA3216_ALERTOFF OUTPUT_CLEAR(PTE,PTE5)
//#define SDO_LOW OUTPUT_CLEAR(PTB,PTB5)
/* end of RDA related */


#define LED0_OFF        OUTPUT_SET(PTG,PTG0);
#define LED1_OFF        OUTPUT_SET(PTG,PTG1);
#define LED2_OFF        OUTPUT_SET(PTG,PTG2);


#define LED0_ON         OUTPUT_CLEAR(PTG,PTG0);
#define LED1_ON         OUTPUT_CLEAR(PTG,PTG1);
#define LED2_ON         OUTPUT_CLEAR(PTG,PTG2);

/* BMS TEST TOOL */
#ifdef STATUS_LED_ENABLE
/*  */
#define LED_BLUE_ON			LED2_ON
#define LED_BLUE_OFF		LED2_OFF
#define LED_BLUE_TOGGLE		LED2_TOGGLE

#define LED_RED_ON			OUTPUT_CLEAR(PTH,PTH2)
#define LED_RED_OFF			OUTPUT_SET(PTH,PTH2)
#define LED_RED_TOGGLE		OUTPUT_TOGGLE(PTH,PTH2)

#define LED_GREEN_ON		OUTPUT_CLEAR(PTE,PTE7)
#define LED_GREEN_OFF		OUTPUT_SET(PTE,PTE7)
#define LED_GREEN_TOGGLE	OUTPUT_TOGGLE(PTE,PTE7)

#define LED_YELLOW_ON		OUTPUT_CLEAR(PTH,PTH6)
#define LED_YELLOW_OFF		OUTPUT_SET(PTH,PTH6)
#define LED_YELLOW_TOGGLE	OUTPUT_TOGGLE(PTH,PTH6)

/* re-define the leds' name */
#define LED_SLAVE_RUN_ON		LED_BLUE_ON
#define LED_SLAVE_RUN_OFF		LED_BLUE_OFF
#define LED_SLAVE_RUN_TOGGLE	LED_BLUE_TOGGLE

#define LED_FAULT_LV1_ON		LED_GREEN_ON
#define LED_FAULT_LV1_OFF		LED_GREEN_OFF
#define LED_FAULT_LV1_TOGGLE	LED_GREEN_TOGGLE

#define LED_FAULT_LV2_ON		LED_YELLOW_ON
#define LED_FAULT_LV2_OFF		LED_YELLOW_OFF
#define LED_FAULT_LV2_TOGGLE	LED_YELLOW_TOGGLE

#define LED_FAULT_LV3_ON		LED_RED_ON
#define LED_FAULT_LV3_OFF		LED_RED_OFF
#define LED_FAULT_LV3_TOGGLE	LED_RED_TOGGLE

#endif

#define CAN_STB_SET       OUTPUT_SET(PTH,PTH6)
#define CAN_STB_CLEAR     OUTPUT_CLEAR(PTH,PTH6)

#define POWER_KEEP_ENABLE	OUTPUT_SET(PTC,PTC5)
#define POWER_KEEP_DISABLE	OUTPUT_CLEAR(PTC,PTC5)

#define SWITCH_RELAY_LOW_MCU_ON     OUTPUT_SET(PTA,PTA0)
#define SWITCH_RELAY_LOW_MCU_OFF    OUTPUT_CLEAR(PTA,PTA0)

#define SWITCH_RELAY_HIGH_MCU_ON    OUTPUT_SET(PTA,PTA1)
#define SWITCH_RELAY_HIGH_MCU_OFF   OUTPUT_CLEAR(PTA,PTA1)

#define MOS_K1_OFF OUTPUT_SET(PTF, PTF3);OUTPUT_SET(PTA, PTA7);OUTPUT_CLEAR(PTF, PTF2);OUTPUT_CLEAR(PTA, PTA6);
#define MOS_K1_ON  OUTPUT_CLEAR(PTF, PTF3);OUTPUT_CLEAR(PTA, PTA7);OUTPUT_SET(PTF, PTF2);OUTPUT_SET(PTA, PTA6);

#define MOS_K2_GND_OFF LED2_OFF;
#define MOS_K2_GND_ON  LED2_ON;

#define MOS_OFF      OUTPUT_SET(PTF, PTF3);OUTPUT_SET(PTA, PTA7);OUTPUT_CLEAR(PTF, PTF2);OUTPUT_CLEAR(PTA, PTA6);LED2_ON;  
#define MOS_ON       LED2_OFF;OUTPUT_CLEAR(PTF, PTF3);OUTPUT_CLEAR(PTA, PTA7);OUTPUT_SET(PTF, PTF2);OUTPUT_SET(PTA, PTA6);

void GPIO_Init(void);

#endif /* GPIO_H_ */

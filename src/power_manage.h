#ifndef _POWER_MANAGE_H
#define _POWER_MANAGE_H

#define Isl94212_ShutDown() OUTPUT_CLEAR(PTE,PTE5) // disable EN port

extern void SysPowerDown(void);
extern void SysPowerUp(void);
#endif

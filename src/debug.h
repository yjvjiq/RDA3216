#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "main.h"


void DebugSendCycle(ring_frame *ringframe);
void Debug_Process(void);
extern ring_frame _printf_ring_frame;
void delay_ms(U32 ms);
void DisplayBuffer(U8 saddr, U8 *src ,U8 len);
void DebugSendCycle(ring_frame *ringframe);
void DebugPushString(U8 *src, U8 len);

#ifdef BTT_OFFLINE
void UpLoadBttData(void);
#endif

extern ring_frame _printf_ring_frame;

#endif

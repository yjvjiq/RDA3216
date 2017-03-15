
#include "main.h"
#include "timer.h"
#include "derivative.h"

#define RATIO_1            10UL// 10ms    

TimerEvent Timer1Event[TIMER_EVENT_MAX]= {{0}};
U32 g_10ms_ticks =0;

void timer1_proc(void)
{
    U8 i = 0;
    for(i=0; i<TIMER_EVENT_MAX; i++) {
        if(Timer1Event[i].bEnable==TRUE) {
            if((--Timer1Event[i].counter)==0) {
                if(Timer1Event[i].type==TIMER_MODE_SINGLE) {
                    Timer1Event[i].bEnable=FALSE;
                    }
                Timer1Event[i].counter= Timer1Event[i].setval;
                Timer1Event[i].eventflag=TRUE;
                if(Timer1Event[i].handle!=NULL) {
                    Timer1Event[i].handle();
                    }
                }
            }
        }
        g_10ms_ticks += 10;
}


void InitTimerEvent(TimerEvent *tv,U8 num)
{
    U8 i=0;
    for(i=0; i<num; i++) {
        tv[i].bEnable=FALSE;
        tv[i].counter=0;
        tv[i].eventflag=FALSE;
        tv[i].handle=NULL;
        tv[i].setval=0;
        tv[i].type=TIMER_MODE_SINGLE;
    }
}

void SetTimer1(U8 eventid,U32 millisecond,TIMOPT opt,pvoid handle)
{
    if(eventid<TIMER_EVENT_MAX) {
       __disable_irq();
        Timer1Event[eventid].bEnable=TRUE;
        Timer1Event[eventid].counter=millisecond/RATIO_1;
        Timer1Event[eventid].setval=millisecond/RATIO_1;
        Timer1Event[eventid].handle=handle;
        Timer1Event[eventid].eventflag=FALSE;
        Timer1Event[eventid].type=opt;
       __enable_irq();
    }
}

void StopTimer1(U8 eventid)
{
    if(eventid<TIMER_EVENT_MAX) {
        __disable_irq();
        Timer1Event[eventid].bEnable=FALSE;
        Timer1Event[eventid].handle=NULL;
        Timer1Event[eventid].eventflag=FALSE;
        __enable_irq();
    }

}
U8 GetTimer1(U8 eventid)
{
    U8 status=FALSE;

    if(eventid<TIMER_EVENT_MAX) {
       __disable_irq();
        status = Timer1Event[eventid].eventflag;
        Timer1Event[eventid].eventflag=FALSE;//clear flag;
       __enable_irq();
    }
    return status;
}


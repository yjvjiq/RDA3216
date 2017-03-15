#ifndef _UARTDATA_SERVICE_H
#define _UARTDATA_SERVICE_H
#include "drivers.h"
#include "data_pack.h"


#define REC_BUFF_LEN 256
#define ESCAPE_CHAR  0xfe
#define ESCAPE_HEAD  0xfe
#define FRAME_HEAD   0x68

#define CHECKSUM_LEN    1
#define CMD_LEN         1
#define PACKET_HEAD_LEN 3

#ifdef BTT_OFFLINE
#define ESCAPE_BYTE0  0x01
#define ESCAPE_BYTE1  0x03
#define ESCAPE_BYTE2  0x00
#define ESCAPE_BYTE3  0x60
#define ESCAPE_BYTE4  0x00
#define ESCAPE_BYTE5  0x20
#define ESCAPE_BYTE6  0x44
#define ESCAPE_BYTE7  0x0c
#endif
typedef enum {
    MSG_STAT_OUT = 0,
    MSG_STAT_EXPECT_HEAD,
    MSG_STAT_EXPECT_FRAMEHEAD,
    MSG_STAT_EXPECT_LEN_BYTE0,
    MSG_STAT_EXPECT_LEN_BYTE1,
    MSG_STAT_IN,
    #ifdef BTT_OFFLINE
    MSG_STAT_EXPECT_BYTE0,
    MSG_STAT_EXPECT_BYTE1,
    MSG_STAT_EXPECT_BYTE2,
    MSG_STAT_EXPECT_BYTE3,
    MSG_STAT_EXPECT_BYTE4,
    MSG_STAT_EXPECT_BYTE5,
    MSG_STAT_EXPECT_BYTE6,
    MSG_STAT_EXPECT_BYTE7,
    #endif
    MSG_STAT_ESCAPE
} e_recstate;

typedef struct {
    U8 buff[REC_BUFF_LEN];
    U16 len;
    e_recstate state;
} s_uartrecframe;

#ifdef CAN_COMMUNICATION
U8 UartDataPullChar(ring_frame *recring, U8 *data);
void UartDataPullCharWait(ring_frame *recring, U8 *data);
U8 UartDataPullString(ring_frame *recring, U8 *data , U16 len);
U8 UartDataPushChar(ring_frame *recring, U8 *data);
extern ring_frame g_uartsend_ring;
#endif 
void SplitUartMsg(U8 *recbuff, U16 len, s_uartrecframe *recframe, void (*handler)(s_uartrecframe *));
void UartDataParse(s_uartrecframe *recframe);
void PushAckString(U8 Cmd, U8 *src, U8 len);

#ifdef VERSION_BTT
void UartDataParseBtt(s_uartrecframe *recframe);
void RecUartMsg(U8 *recdata, s_uartrecframe *recframe);
void SplitUartBttMsg(U8 *recbuff, U16 len, s_uartrecframe *recframe, void (*handler)(s_uartrecframe *));
#endif

extern s_uartrecframe g_uartrecframe;

#ifdef VERSION_BTT
extern s_uartrecframe g_uartrecframebtt;
#endif
void UartCmdHandler(UART_Type *pUART);

#endif


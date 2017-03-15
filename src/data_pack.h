#ifndef _DATA_PACK_H
#define _DATA_PACK_H
#include "drivers.h"

#define UF_MSGHEAD_POS          0
#define UF_FRAMEHEAD_POS        (UF_MSGHEAD_POS+2)
#define UF_LEN_POS              (UF_FRAMEHEAD_POS+1)
#define UF_COMMAND_POS          (UF_LEN_POS+2)
#define UF_BUFF_POS             (UF_COMMAND_POS+1)

typedef union {
    U32 varu32;
    U8  varu8[4];
} u_varform_32u8;

typedef union {
    U16 baru16;
    U8  varu8[2];
} u_varform_16u8;

typedef union {
    U32 canid;
    U8  data[4];
} u_canid;

typedef struct {
    U32 canid;
    U8  candata[8];
} s_canmsg;

typedef union {
    s_canmsg canmsg;
    U8       data[12];
} u_canmsg;

typedef __packed struct {
    U16 msghead;
    U8 framehead;
    U16 len;
    U8 command;
//    U8 *buff;
//    U8 checksum;
} s_uartframe;

typedef union {
    s_uartframe uartframe;
    U8          data[6];
} u_uartframe;

void packdata(u_uartframe *uartframe, U8 *srcbuff , U16 bufflen, U8 *destbuff);
U8 packcheack(U8 *buff, U8 len, U8 checksum);
void CheckSumAddString(U8 *checksum , U8 *data, U16 len);



#endif



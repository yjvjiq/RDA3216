#include "data_pack.h"

void packdata(u_uartframe *uartframe, U8 *srcbuff , U16 bufflen, U8 *destbuff)
{
    U8 checksum = 0;
    uartframe->uartframe.len = bufflen + 2;
    *destbuff++ = uartframe->data[0];
    *destbuff++ = uartframe->data[1];
    for(U8 i = 0; i < 4 ; i++) {
        *destbuff++ = uartframe->data[2 + i];
        checksum += uartframe->data[2 + i];
    }
    for(U16 i = 0; i < bufflen; i++) {
        *destbuff++ = *srcbuff;
        checksum += *srcbuff++;
    }
    *destbuff = checksum;
}

U8 packcheack(U8 *buff, U8 len, U8 checksum)
{
    U8 sum = 0;
    for (U16 i = 0; i < len; i++) {
        sum += buff[i];
    }
    if(sum == checksum)
        return TRUE;
    else
        return FALSE;
}

void CheckSumAddString(U8 *checksum , U8 *data, U16 len)
{
    for(U16 i = 0; i < len; i++) {
        *checksum += data[i];
    }
}


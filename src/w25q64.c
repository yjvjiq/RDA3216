#include "w25q64.h"
#include "drivers.h"
#include "public.h"


//flashstate_msg g_flashstate_msg={{0xff},0};
flashdata_frame g_flashdata_frame ={0,0,TRUE,FALSE};

static U8 StaticErrBlockSaveEnable = FALSE;

//#define FLASH_HEADTAIL
//¶ÁÈ¡SPI_FLASHµÄ×´Ì¬¼Ä´æÆ÷
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ä¬ÈÏ0,×´Ì¬¼Ä´æÆ÷±£»¤Î»,ÅäºÏWPÊ¹ÓÃ
//TB,BP2,BP1,BP0:FLASHÇøÓòÐ´±£»¤ÉèÖÃ
//WEL:Ð´Ê¹ÄÜËø¶¨
//BUSY:Ã¦±ê¼ÇÎ»(1,Ã¦;0,¿ÕÏÐ)
//Ä¬ÈÏ:0x00
U8 SPI_Flash_ReadSR(void)
{
    U8 byte = 0;
    FLASH_SPI_CS_ON();
    Spi0_RWByte(W25X_ReadStatusReg);
    byte = Spi0_RWByte(0Xff);
    FLASH_SPI_CS_OFF();
    return byte;
}

//Ð´SPI_FLASH×´Ì¬¼Ä´æÆ÷
//Ö»ÓÐSPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)¿ÉÒÔÐ´!!!
void SPI_FLASH_Write_SR(u8 sr)
{
    FLASH_SPI_CS_ON();
    Spi0_RWByte(W25X_WriteStatusReg);
    Spi0_RWByte(sr);
    FLASH_SPI_CS_OFF();
}

//SPI_FLASHÐ´Ê¹ÄÜ
//½«WELÖÃÎ»
void SPI_FLASH_Write_Enable(void)
{
    FLASH_SPI_CS_ON();
    Spi0_RWByte(W25X_WriteEnable);
    FLASH_SPI_CS_OFF();
}

//SPI_FLASHÐ´½ûÖ¹
//½«WELÇåÁã
void SPI_FLASH_Write_Disable(void)
{
    FLASH_SPI_CS_ON();
    Spi0_RWByte(W25X_WriteDisable);
    FLASH_SPI_CS_OFF();
}

//¶ÁÈ¡SPI FLASH
//ÔÚÖ¸¶¨µØÖ·¿ªÊ¼¶ÁÈ¡Ö¸¶¨³¤¶ÈµÄÊý¾Ý
//pBuffer:Êý¾Ý´æ´¢Çø
//ReadAddr:¿ªÊ¼¶ÁÈ¡µÄµØÖ·(24bit)
//NumByteToRead:Òª¶ÁÈ¡µÄ×Ö½ÚÊý(×î´ó65535)
void SPI_Flash_Read(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
    u16 i;
    FLASH_SPI_CS_ON();
    Spi0_RWByte(W25X_ReadData);
    Spi0_RWByte((u8)((ReadAddr) >> 16));
    Spi0_RWByte((u8)((ReadAddr) >> 8));
    Spi0_RWByte((u8)ReadAddr);
    for(i = 0; i < NumByteToRead; i++) {
       pBuffer[i] = Spi0_RWByte(0XFF);
    }
    FLASH_SPI_CS_OFF();
}

//SPIÔÚÒ»Ò³(0~65535)ÄÚÐ´ÈëÉÙÓÚ256¸ö×Ö½ÚµÄÊý¾Ý
//ÔÚÖ¸¶¨µØÖ·¿ªÊ¼Ð´Èë×î´ó256×Ö½ÚµÄÊý¾Ý
//pBuffer:Êý¾Ý´æ´¢Çø
//WriteAddr:¿ªÊ¼Ð´ÈëµÄµØÖ·(24bit)
//NumByteToWrite:ÒªÐ´ÈëµÄ×Ö½ÚÊý(×î´ó256),¸ÃÊý²»Ó¦¸Ã³¬¹ý¸ÃÒ³µÄÊ£Óà×Ö½ÚÊý!!!
void SPI_Flash_Write_Page(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    u16 i;
    SPI_FLASH_Write_Enable();                  //SET WEL
    FLASH_SPI_CS_ON();
    Spi0_RWByte(W25X_PageProgram);
    Spi0_RWByte((u8)((WriteAddr) >> 16));
    Spi0_RWByte((u8)((WriteAddr) >> 8));
    Spi0_RWByte((u8)WriteAddr);
    for(i = 0; i < NumByteToWrite; i++)Spi0_RWByte(pBuffer[i]);
    FLASH_SPI_CS_OFF();
    SPI_Flash_Wait_Busy();
}

//ÎÞ¼ìÑéÐ´SPI FLASH
//±ØÐëÈ·±£ËùÐ´µÄµØÖ··¶Î§ÄÚµÄÊý¾ÝÈ«²¿Îª0XFF,·ñÔòÔÚ·Ç0XFF´¦Ð´ÈëµÄÊý¾Ý½«Ê§°Ü!
//¾ßÓÐ×Ô¶¯»»Ò³¹¦ÄÜ
//ÔÚÖ¸¶¨µØÖ·¿ªÊ¼Ð´ÈëÖ¸¶¨³¤¶ÈµÄÊý¾Ý,µ«ÊÇÒªÈ·±£µØÖ·²»Ô½½ç!
//pBuffer:Êý¾Ý´æ´¢Çø
//WriteAddr:¿ªÊ¼Ð´ÈëµÄµØÖ·(24bit)
//NumByteToWrite:ÒªÐ´ÈëµÄ×Ö½ÚÊý(×î´ó65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    u16 pageremain;
    pageremain = 256 - WriteAddr % 256; //µ¥Ò³Ê£ÓàµÄ×Ö½ÚÊý
    if(NumByteToWrite <= pageremain)
        pageremain = NumByteToWrite; //²»´óÓÚ256¸ö×Ö½Ú
    while(1) {
        SPI_Flash_Write_Page(pBuffer, WriteAddr, pageremain);
        if(NumByteToWrite == pageremain)break; //Ð´Èë½áÊøÁË
        else { //NumByteToWrite>pageremain
            pBuffer += pageremain;
            WriteAddr += pageremain;
            NumByteToWrite -= pageremain;         //¼õÈ¥ÒÑ¾­Ð´ÈëÁËµÄ×Ö½ÚÊý
            if(NumByteToWrite > 256)pageremain = 256; //Ò»´Î¿ÉÒÔÐ´Èë256¸ö×Ö½Ú
            else pageremain = NumByteToWrite;     //²»¹»256¸ö×Ö½ÚÁË
        }
    };
}

/*
//Ð´SPI FLASH
//ÔÚÖ¸¶¨µØÖ·¿ªÊ¼Ð´ÈëÖ¸¶¨³¤¶ÈµÄÊý¾Ý
//¸Ãº¯Êý´ø²Á³ý²Ù×÷!
//pBuffer:Êý¾Ý´æ´¢Çø
//WriteAddr:¿ªÊ¼Ð´ÈëµÄµØÖ·(24bit)
//NumByteToWrite:ÒªÐ´ÈëµÄ×Ö½ÚÊý(×î´ó65535)
u8 SPI_FLASH_BUFFER[4096];
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
    u32 secpos;
    u16 secoff;
    u16 secremain;
    u16 i;
    u8 * SPI_FLASH_BUF;
    SPI_FLASH_BUF=SPI_FLASH_BUFFER;
    secpos=WriteAddr/4096;//ÉÈÇøµØÖ·
    secoff=WriteAddr%4096;//ÔÚÉÈÇøÄÚµÄÆ«ÒÆ
    secremain=4096-secoff;//ÉÈÇøÊ£Óà¿Õ¼ä´óÐ¡
    //printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//²âÊÔÓÃ
    if(NumByteToWrite<=secremain)
        secremain=NumByteToWrite;//²»´óÓÚ4096¸ö×Ö½Ú
    while(1) {
        SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//¶Á³öÕû¸öÉÈÇøµÄÄÚÈÝ
        for(i=0; i<secremain; i++) { //Ð£ÑéÊý¾Ý
            if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;//ÐèÒª²Á³ý
        }
        if(i<secremain) { //ÐèÒª²Á³ý
            SPI_Flash_Erase_Sector(secpos);//²Á³ýÕâ¸öÉÈÇø
            for(i=0; i<secremain; i++) { //¸´ÖÆ
                SPI_FLASH_BUF[i+secoff]=pBuffer[i];
            }
            SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//Ð´ÈëÕû¸öÉÈÇø

        } else SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);//Ð´ÒÑ¾­²Á³ýÁËµÄ,Ö±½ÓÐ´ÈëÉÈÇøÊ£ÓàÇø¼ä.
        if(NumByteToWrite==secremain)break;//Ð´Èë½áÊøÁË
        else { //Ð´ÈëÎ´½áÊø
            secpos++;//ÉÈÇøµØÖ·Ôö1
            secoff=0;//Æ«ÒÆÎ»ÖÃÎª0

            pBuffer+=secremain;  //Ö¸ÕëÆ«ÒÆ
            WriteAddr+=secremain;//Ð´µØÖ·Æ«ÒÆ
            NumByteToWrite-=secremain;              //×Ö½ÚÊýµÝ¼õ
            if(NumByteToWrite>4096)secremain=4096;  //ÏÂÒ»¸öÉÈÇø»¹ÊÇÐ´²»Íê
            else secremain=NumByteToWrite;          //ÏÂÒ»¸öÉÈÇø¿ÉÒÔÐ´ÍêÁË
        }
    };
}
*/

//²Á³ýÕû¸öÐ¾Æ¬
//µÈ´ýÊ±¼ä³¬³
void SPI_Flash_Erase_Chip(void)
{
    SPI_FLASH_Write_Enable();                  //SET WEL
    SPI_Flash_Wait_Busy();
    FLASH_SPI_CS_ON();
    Spi0_RWByte(W25X_ChipErase);
    FLASH_SPI_CS_OFF();
    SPI_Flash_Wait_Busy();
}

//²Á³ýÒ»¸öÉÈÇø
//Dst_Addr:ÉÈÇøµØÖ· ¸ù¾ÝÊµ¼ÊÈÝÁ¿ÉèÖÃ
//²Á³ýÒ»¸öÉ½ÇøµÄ×îÉÙÊ±¼ä:150ms
void SPI_Flash_Erase_Sector(u32 Dst_Addr)
{
    //?¨¤¨º¨®falsh2¨¢3y?¨¦??,2a¨º?¨®?
//    printf("fe:%x\r\n",Dst_Addr);
//    Dst_Addr*=4096;
    SPI_FLASH_Write_Enable();                  //SET WEL
    SPI_Flash_Wait_Busy();
    FLASH_SPI_CS_ON();
    Spi0_RWByte(W25X_SectorErase);
    Spi0_RWByte((u8)((Dst_Addr) >> 16));
    Spi0_RWByte((u8)((Dst_Addr) >> 8));
    Spi0_RWByte((u8)Dst_Addr);
    FLASH_SPI_CS_OFF();
    SPI_Flash_Wait_Busy();
}

//µÈ´ý¿ÕÏÐ
void SPI_Flash_Wait_Busy(void)
{
    while((SPI_Flash_ReadSR() & 0x01) == 0x01);
}

//½øÈëµôµçÄ£Ê½
void SPI_Flash_PowerDown(void)
{
    FLASH_SPI_CS_ON();
    Spi0_RWByte(W25X_PowerDown);
    FLASH_SPI_CS_OFF();
    delay_ms(1);
}

//»½ÐÑ
void SPI_Flash_WAKEUP(void)
{
    FLASH_SPI_CS_ON();
    Spi0_RWByte(W25X_ReleasePowerDown);   //  send W25X_PowerDown command 0xAB
    FLASH_SPI_CS_OFF();
    delay_ms(1);
}

//U32 W25Q64_GetAddr(U8 *buff)
//{
//    U32 addr = 0, a = 0, b = 0, c = 0;
//    a =  buff[0];
//    b =  buff[1];
//    c =  buff[2];
//    addr = (a << 16) + (b << 8) + c;
//    return addr;
//}

//void W25Q64_SetAddr(U8 *buff, U32 addr)
//{
//    *buff = (U8)(addr >> 16);
//    *(buff + 1) = (U8)(addr >> 8);
//    *(buff + 2) = (U8)addr;
//}

void W25Q64_GetFlags(flashdata_frame *frame, U8 *flags)
{
    if(Get_FlashFlag(*flags, STATE_EMPTY_BITPOS) == FLAG_TRUE) {
        frame->empty = TRUE;
    } else {
        frame->empty = FALSE;
    }
    if(Get_FlashFlag(*flags, STATE_FULL_BITPOS) == FLAG_TRUE) {
        frame->full = TRUE;
    } else {
        frame->full = FALSE;
    }
}

void W25Q64_SetFlags(flashdata_frame *frame, U8 *flags)
{
    if(frame->empty == TRUE) {
        Set_FlashFlag(*flags, STATE_EMPTY_BITPOS);
    } else {
        Clear_FlashFlag(*flags, STATE_EMPTY_BITPOS);
    }

    if(frame->full == TRUE) {
        Set_FlashFlag(*flags, STATE_FULL_BITPOS);
    } else {
        Clear_FlashFlag(*flags, STATE_FULL_BITPOS);
    }
}

void W25Q64_GetFlashDataPtrMsg(flashdata_frame *frame, s_FlashDataPtrStruct *dataptrframe)
{
    frame->head = dataptrframe->FlashDataPtr.HeadAddr;
#ifdef VERSION_BTT
    frame->tail = dataptrframe->FlashDataPtr.TailAddr;
#endif
    W25Q64_GetFlags(frame, &(dataptrframe->FlashDataPtr.Flags));
}

void W25Q64_SetFlashDataPtrMsg(flashdata_frame *frame, s_FlashDataPtrStruct *dataptrframe)
{
    dataptrframe->FlashDataPtr.HeadAddr = frame->head;
#ifdef VERSION_BTT
    dataptrframe->FlashDataPtr.TailAddr = frame->tail;
#endif

    W25Q64_SetFlags(frame, &(dataptrframe->FlashDataPtr.Flags));
}

//  uint8_t *tmpPtr2 = (uint8_t *)Ptr; CRC_DRV.C
void W25Q64_AddCRCToBufferEnd(U8 *buff, U16 len)
{
    uint32_t crc_result = 0;
    Crc_MakeCrc(buff, len - 2, &crc_result);
    buff[len - 1] = (U8)(crc_result >> 8);
    buff[len - 2] = (U8)(crc_result);
}

U8 W25Q64_CheackBufferCRC(U8 *buff, U16 len)
{
    U16 temp_crc = 0, temp_var = 0;
    uint32_t crc_result = 0;
    Crc_MakeCrc(buff, len - 2, &crc_result);
    temp_var = buff[len - 1];
    temp_crc = temp_var << 8;
    temp_crc += buff[len - 2];
    if(crc_result == temp_crc)
        return TRUE;
    else
        return FALSE;
}

U8 W25Q64_IsByteUsed(U32 addr)
{
    U8 data = 0;
    SPI_Flash_Read(&data, addr, 1);
    if(data != 0xff) { //BLOCK uesd
        return TRUE;
    } else {
        return FALSE;
    }
}

U8 W25Q64_CheackSectorsByBlock(U32 start_addr, U32 end_addr, U16 blocksize)
{
    U32 temp_addr = 0;
    for(temp_addr = start_addr; temp_addr <= end_addr; temp_addr += blocksize) {
        if(W25Q64_IsByteUsed(temp_addr) == TRUE) {
            return TRUE; //sector uesd
        }
    }
    return FALSE;
}

U8 W25Q64_CheackThisSectorByBlock(U32 addr, U16 blocksize) //is sector used or not
{
    u32 sector_start_addr = W25Q64_FindSectorAddr(addr);
    u32 temp_addr = sector_start_addr;
    u16 i = 0;
    for(i = 0; i < 4096 / blocksize ; i++) {
        if(W25Q64_IsByteUsed(temp_addr) == TRUE) {
            return TRUE; //sector uesd
        } else {
            temp_addr += blocksize;
        }
    }
    return FALSE;         //sector unused
}

void W25Q64_EraseUsedStateSector(U32 addr)
{
    U32 temp_addr = 0, end_addr = 0, next_addr = 0;
    U16 remain_sectors = 0;
    U16 i = 0;
    if(addr < W25Q64_STATE_AREASIZE) {
        temp_addr = addr & 0x7ff000;
        end_addr = temp_addr + W25Q64_SECTOR_SIZE - W25Q64_STATE_BLOCKSIZE;
        remain_sectors = ( (W25Q64_STATE_AREASIZE - temp_addr) / W25Q64_SECTOR_SIZE) - 1;
        if(W25Q64_CheackSectorsByBlock(addr, end_addr, W25Q64_STATE_BLOCKSIZE) == TRUE) {
            SPI_Flash_Erase_Sector(addr);
        }
        for(i = 0; i < remain_sectors; i++) {
            next_addr += W25Q64_SECTOR_SIZE;
            if(W25Q64_CheackThisSectorByBlock(next_addr, W25Q64_STATE_BLOCKSIZE) == TRUE) {
                SPI_Flash_Erase_Sector(next_addr);
            }
        }
    }
}
U32 W25Q64_FindNextAddr(U32 this_addr, U32 start_addr , U32 end_addr, U16 blocksize)
{
    U32 next_addr = 0;
    if((this_addr < start_addr) || (this_addr > end_addr)) { //addr error
        // return 0xffffffff;
        SPI_Flash_Erase_Sector(start_addr);
        return start_addr;
    } else {
        next_addr = this_addr + blocksize;
        if(next_addr > end_addr) {    //over write
            next_addr = start_addr;
        }
        return next_addr;
    }
}

U32 W25Q64_FindPreviousAddr(U32 this_addr, U32 start_addr , U32 end_addr, U16 blocksize)
{
    U32 next_addr = this_addr;
    if((this_addr > start_addr) || (this_addr < end_addr)) { //addr error
        // return 0xffffffff;
        SPI_Flash_Erase_Sector(start_addr);
        return start_addr;
    } else {
        if(next_addr == end_addr) {    //over write
            next_addr = start_addr;
        } else {
            next_addr -= blocksize;
        }
        return next_addr;
    }
}

U8 W25Q64_FindSelectedDataAddr(U32 num)
{
    U32 i = 0;
    for(i=0;i<num;i++)
    {
    // addr = W25Q64_FindPreviousAddr(addr, W25Q64_DATA_LASTBLOCKADDR, W25Q64_DATA_START_ADDR, W25Q64_DATA_BLOCKSIZE);
        if(W25Q64_DataBlockPop(&g_flashdata_frame) == FALSE){
            return FALSE;
        }
    }
    return TRUE;
}

U8 W25Q64_IsSameSector(U32 addr_1, U32 addr_2)
{
    if(W25Q64_FindSectorAddr(addr_1) == W25Q64_FindSectorAddr(addr_2)) {
        return TRUE;   // same sector
    } else
        return FALSE; //different sector

}

U8 W25Q64_GetStateBlcok(U32 *addr, U8 *buff)
{
    U32 temp_addr = 0;
    U8 BYTE0 = 0, loopcount = 0;
    while(1) {
		WDOG_Feed();
        temp_addr = W25Q64_FindPreviousAddr(temp_addr, W25Q64_STATE_LASTBLOCKADDR, W25Q64_STATE_START_ADDR, W25Q64_STATE_BLOCKSIZE);
        *addr = temp_addr;
        if(temp_addr == W25Q64_STATE_LASTBLOCKADDR) {
            loopcount++;
            if(loopcount == 2) {
                memset(buff, 0xff, W25Q64_STATE_BLOCKSIZE);
                return FALSE;
            }
        }
        SPI_Flash_Read(&BYTE0, temp_addr, 1);
        if( (BYTE0 == 0xff) || (BYTE0 == 0)) {
            continue;
        // } else if(BYTE0 == 0xfe || BYTE0 == 0xfc) {
        }else{
            SPI_Flash_Read(buff, temp_addr, W25Q64_STATE_BLOCKSIZE);
            if(W25Q64_CheackBufferCRC(&buff[1], W25Q64_STATE_BLOCKSIZE - 1) == TRUE) {
                Set_FlashFlag(BYTE0, BLOCK_READ_BITPOS);
                SPI_Flash_Write_NoCheck(&BYTE0, temp_addr, 1);
                return TRUE;
            } else {
                // Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
                // SPI_Flash_Write_NoCheck(&BYTE0, temp_addr, 1);
                continue;
            }
        }
        //  else {
        //     Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
        //     SPI_Flash_Write_NoCheck(&BYTE0, temp_addr, 1);
        //     continue;
        // }
    }
}

U8 W25Q64_WriteStateBlcok(U32 *addr, U8 *buff)
{
    U32 temp_addr = 0, ex_addr = 0;
    U8 BYTE0 = 0xff;
    static U8 temp_buff[W25Q64_STATE_BLOCKSIZE] = {0};
    //clear buffer
    memset(temp_buff, 0, W25Q64_STATE_BLOCKSIZE);
    U32 RemainBlocks = W25Q64_STATE_BLOCKS;
    while(RemainBlocks--) {
		WDOG_Feed();
        ex_addr = *addr;
        temp_addr = W25Q64_FindNextAddr(ex_addr, W25Q64_STATE_START_ADDR, W25Q64_STATE_LASTBLOCKADDR, W25Q64_STATE_BLOCKSIZE);
        *addr = temp_addr;
        if(W25Q64_IsSameSector(ex_addr, temp_addr) == FALSE) {
            SPI_Flash_Erase_Sector(ex_addr);
        }
        // W25Q64_EraseUsedStateSector(temp_addr);
        W25Q64_AddCRCToBufferEnd(&buff[1], W25Q64_STATE_BLOCKSIZE - 1);
        SPI_Flash_Write_NoCheck(buff, temp_addr, W25Q64_STATE_BLOCKSIZE);
        SPI_Flash_Read(temp_buff, temp_addr, W25Q64_STATE_BLOCKSIZE);
        if(W25Q64_CheackBufferCRC(&temp_buff[1], W25Q64_STATE_BLOCKSIZE - 1) == TRUE) {
            Set_FlashFlag(BYTE0, BLOCK_WRITE_BITPOS);
            SPI_Flash_Write_NoCheck(&BYTE0, temp_addr, 1);
            return TRUE;
        } else {
            // Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
            // SPI_Flash_Write_NoCheck(&BYTE0, temp_addr, 1);
            continue;
        }
    }
	return FALSE;
}

U8 W25Q64_GetDataPtrBlcok(U32 *addr, U8 *buff)
{
    U32 temp_addr = 0;
    U8 BYTE0 = 0, loopcount = 0;
    while(1) {
		WDOG_Feed();
        temp_addr = W25Q64_FindPreviousAddr(temp_addr, W25Q64_DATA_PTR_LASTBLOCKADDR, W25Q64_DATA_PTR_START_ADDR, W25Q64_DATA_PTR_BLOCKSIZE);
        *addr = temp_addr;
        if(temp_addr == W25Q64_DATA_PTR_LASTBLOCKADDR) {
            loopcount++;
            if(loopcount == 2) {
                memset(buff, 0xff, W25Q64_DATA_PTR_BLOCKSIZE);
                return FALSE;
            }
        }
        SPI_Flash_Read(&BYTE0, temp_addr, 1);
        if( (BYTE0 == 0xff) || (BYTE0 == 0) ) {
            continue;
        // } else if(BYTE0 == 0xfe || BYTE0 == 0xfc) {
        }else{
            SPI_Flash_Read(buff, temp_addr, W25Q64_DATA_PTR_BLOCKSIZE);
            if(W25Q64_CheackBufferCRC(&buff[1], W25Q64_DATA_PTR_BLOCKSIZE - 1) == TRUE) {
                Set_FlashFlag(BYTE0, BLOCK_READ_BITPOS);
                SPI_Flash_Write_NoCheck(&BYTE0, temp_addr, 1);
                return TRUE;
            } else {
                // Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
                // SPI_Flash_Write_NoCheck(&BYTE0, temp_addr, 1);
                continue;
            }
        }
        //  else {
        //     Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
        //     SPI_Flash_Write_NoCheck(&BYTE0, temp_addr, 1);
        //     continue;
        // }
    }
}

U8 W25Q64_WriteDataPtrBlcok(U32 *addr, U8 *buff)
{
    U32 temp_addr = 0, ex_addr = 0;
    U8 BYTE0 = 0xff;
    static U8 temp_buff[W25Q64_DATA_PTR_BLOCKSIZE] = {0};
    //clear buffer
    memset(temp_buff, 0, W25Q64_DATA_PTR_BLOCKSIZE);
    U32 RemainBlocks = W25Q64_DATA_PTR_BLOCKS;
    while(RemainBlocks--) {
		WDOG_Feed();
        ex_addr = *addr;
        temp_addr = W25Q64_FindNextAddr(ex_addr, W25Q64_DATA_PTR_START_ADDR, W25Q64_DATA_PTR_LASTBLOCKADDR, W25Q64_DATA_PTR_BLOCKSIZE);
        *addr = temp_addr;
        if(W25Q64_IsSameSector(ex_addr, temp_addr) == FALSE) {
            SPI_Flash_Erase_Sector(ex_addr);
        }
        // W25Q64_EraseUsedDATA_PTRSector(temp_addr);
        W25Q64_AddCRCToBufferEnd(&buff[1], W25Q64_DATA_PTR_BLOCKSIZE - 1);
        SPI_Flash_Write_NoCheck(buff, temp_addr, W25Q64_DATA_PTR_BLOCKSIZE);
        SPI_Flash_Read(temp_buff, temp_addr, W25Q64_DATA_PTR_BLOCKSIZE);
        if(W25Q64_CheackBufferCRC(&temp_buff[1], W25Q64_DATA_PTR_BLOCKSIZE - 1) == TRUE) {
            Set_FlashFlag(BYTE0, BLOCK_WRITE_BITPOS);
            SPI_Flash_Write_NoCheck(&BYTE0, temp_addr, 1);
            return TRUE;
        } else {
            // Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
            // SPI_Flash_Write_NoCheck(&BYTE0, temp_addr, 1);
            continue;
        }
    }
	return FALSE;
}

U8 W25Q64_GetStateMSG(FlashState_Data_Struct *stateframe)
{
    if(W25Q64_GetStateBlcok(&stateframe->LastAddr, stateframe->stata_union.data) == TRUE) {
#ifdef VERSION_BMS
        printf("GetAddr:0x%x\n", stateframe->LastAddr);
//         W25Q64_GetFlashDataPtrMsg(dataframe, stateframe);
// #ifdef VERSION_BMS
//         printf("head:0x%x,tail:0x%x,empty:0x%x,full:0x%x\n", dataframe->head, dataframe->tail, dataframe->empty, dataframe->full);
        printf("\nComAddr:0x%d\n",stateframe->stata_union.state_data.ComAddr);
#endif
// #endif
//        delay_ms(500);
        return TRUE;
    } else { //STATE Area empty init dataframe
#ifdef VERSION_BMS
        printf("State Area Empty!\n");
        printf("GetAddr:0x%x\n", stateframe->LastAddr);
#endif
//          delay_ms(500);
//         dataframe->head = W25Q64_DATA_START_ADDR;
// #ifdef VERSION_BTT
//         dataframe->tail = W25Q64_DATA_START_ADDR;
// #endif
//         dataframe->empty = TRUE;
//         dataframe->full = FALSE;
        return FALSE;
    }
}

void W25Q64_SetStateMSG(FlashState_Data_Struct *stateframe)
{
//      U8 i = 0;
    // W25Q64_SetFlashDataPtrMsg(dataframe, stateframe);
//    for(i = 0; i < 8; i++) {
//        printf("buff[%d]:0x%x\n", i, stateframe->stata_union.data[i]);
//    }
    W25Q64_WriteStateBlcok((U32 *)&stateframe->LastAddr, stateframe->stata_union.data);
    printf("WriteAddr:0x%x\n",stateframe->LastAddr);
}

U8 W25Q64_GetDataPtrMSG(s_FlashDataPtrStruct *dataptrframe, flashdata_frame *dataframe)
{
    if(W25Q64_GetDataPtrBlcok((U32 *)&dataptrframe->LastAddr, (U8 *)&dataptrframe->FlashDataPtr) == TRUE) {
#ifdef VERSION_BMS
        printf("GetAddr:0x%x\n", dataptrframe->LastAddr);
#endif
        W25Q64_GetFlashDataPtrMsg(dataframe, dataptrframe);
#ifdef VERSION_BMS
        printf("head:0x%x,tail:0x%x,empty:0x%x,full:0x%x\n", dataframe->head, dataframe->tail, dataframe->empty, dataframe->full);
        // printf("\nComAddr:0x%d\n",dataptrframe->stata_union.state_data.ComAddr);
#endif
//        delay_ms(500);
        return TRUE;
    } else { //STATE Area empty init dataframe
#ifdef VERSION_BMS
        // printf("State Area Empty!\n");
        printf("GetAddr:0x%x\n", dataptrframe->LastAddr);
#endif
//          delay_ms(500);
        dataframe->head = W25Q64_DATA_PTR_START_ADDR;
#ifdef VERSION_BTT
        dataframe->tail = W25Q64_DATA_PTR_START_ADDR;
#endif
        dataframe->empty = TRUE;
        dataframe->full = FALSE;
        return FALSE;
    }
}

void W25Q64_SetDataPtrMSG(s_FlashDataPtrStruct *dataptrframe, flashdata_frame *dataframe)
{
//      U8 i = 0;
    W25Q64_SetFlashDataPtrMsg(dataframe, dataptrframe);
//    for(i = 0; i < 8; i++) {
//        printf("buff[%d]:0x%x\n", i, dataptrframe->stata_union.data[i]);
//    }
    W25Q64_WriteDataPtrBlcok((U32*)&dataptrframe->LastAddr, (U8 *)&dataptrframe->FlashDataPtr);
#ifdef VERSION_BMS
    printf("WriteAddr:0x%x\n",dataptrframe->LastAddr);
#endif
}

//U8 tempbuf[1000]={0};
U8 W25Q64_GetStaticMSG(FlashStatic_Data_Union *staicdata)
{   
    SPI_Flash_Read(staicdata->data, W25Q64_STATIC_START_ADDR, W25Q64_STATIC_BLOCKSIZE);
   // SPI_Flash_Read(tempbuf, W25Q64_STATIC_START_ADDR, W25Q64_STATIC_BLOCKSIZE);

    if(W25Q64_CheackBufferCRC(staicdata->data,W25Q64_STATIC_BLOCKSIZE) == TRUE)
    {
#ifdef VERSION_BMS
    printf("\nBMSID:");
    for(U8 i=0;i<12;i++)
    {
    printf("0x%x ",staicdata->static_data.BMSID[i]);
    }
    printf("\n");
#endif
     return TRUE;
    }else{
    return FALSE;
    
}
}

void W25Q64_SetStaticMSG(FlashStatic_Data_Union *staicdata)
{
    SPI_Flash_Erase_Sector(W25Q64_STATIC_START_ADDR);
    W25Q64_AddCRCToBufferEnd(staicdata->data, W25Q64_STATIC_BLOCKSIZE);
    SPI_Flash_Write_NoCheck(staicdata->data, W25Q64_STATIC_START_ADDR, W25Q64_STATIC_BLOCKSIZE);
}


// #ifdef FLASH_HEADTAIL
U8 W25Q64_WriteOneDataBlock(flashdata_frame *dataframe, U8 *buff_ptr)
{
    U32 next_addr = 0, ex_addr = 0;
    static U8 temp_buff[W25Q64_DATA_BLOCKSIZE] = {0};
    U8 BYTE0 = 0;
    //clear buffer
    memset(temp_buff, 0, W25Q64_DATA_BLOCKSIZE);
    U32 RemainBlocks = W25Q64_DATA_BLOCKS;
    while(RemainBlocks--) {
		WDOG_Feed();
        ex_addr = dataframe->head;
        if(dataframe->full == TRUE) {
        }
        next_addr = W25Q64_FindNextAddr(ex_addr, W25Q64_DATA_START_ADDR, W25Q64_DATA_END_ADDR, W25Q64_DATA_BLOCKSIZE); //addr HEAD+1
        dataframe->head = next_addr;
        if(dataframe->head == dataframe->tail) { //HEAD == TAIL
            dataframe->full = TRUE;
        }
        if(W25Q64_IsSameSector(dataframe->head, ex_addr) == FALSE) { //beyond Sector?
            // if(W25Q64_CheackThisSectorByBlock(dataframe->head, W25Q64_DATA_BLOCKSIZE) == TRUE) { // is Sector used
                SPI_Flash_Erase_Sector(dataframe->head);
            // }
        }
        SPI_Flash_Read(&BYTE0, dataframe->head, 1); //READ BYTE0
        if(BYTE0 == 0xfd) { //write but not read
            continue;
        } else if(BYTE0 == 0xff) { //empty block
            W25Q64_AddCRCToBufferEnd(&buff_ptr[1], W25Q64_DATA_BLOCKSIZE - 1);
            SPI_Flash_Write_NoCheck(buff_ptr, dataframe->head, W25Q64_DATA_BLOCKSIZE);
            dataframe->empty = FALSE;
            SPI_Flash_Read(temp_buff, dataframe->head, W25Q64_DATA_BLOCKSIZE);
            if(W25Q64_CheackBufferCRC(&temp_buff[1], W25Q64_DATA_BLOCKSIZE - 1) == TRUE) {
                Set_FlashFlag(BYTE0, BLOCK_WRITE_BITPOS);
                SPI_Flash_Write_NoCheck(&BYTE0, dataframe->head, 1);
                return TRUE;
            } else {
                Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
                SPI_Flash_Write_NoCheck(&BYTE0, dataframe->head, 1);
                continue;
            }
        } else {
            Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
            SPI_Flash_Write_NoCheck(&BYTE0, dataframe->head, 1);
            continue;
        }
    }
	return FALSE;
}

U8 W25Q64_DataBlockTop(flashdata_frame *dataframe, U8 *buff_ptr)
{
    U8 BYTE0 = 0;
    SPI_Flash_Read(&BYTE0, dataframe->tail, 1);
    if (BYTE0 == 0xfe) {
        SPI_Flash_Read(buff_ptr, dataframe->tail, W25Q64_DATA_BLOCKSIZE);
        if(W25Q64_CheackBufferCRC(&buff_ptr[1], W25Q64_DATA_BLOCKSIZE - 1) == TRUE) {
            // Set_FlashFlag(BYTE0, BLOCK_READ_BITPOS);
            // SPI_Flash_Write_NoCheck(&BYTE0, dataframe->tail, 1);
            return TRUE;
        } else {
            Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
            SPI_Flash_Write_NoCheck(&BYTE0, dataframe->tail, 1);
            return FALSE;
        }
    } else {
        // Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
        // SPI_Flash_Write_NoCheck(&BYTE0, dataframe->tail, 1);
        return FALSE;
    }
}

U8 W25Q64_DataBlockPop(flashdata_frame *dataframe)
{
    if(dataframe->empty == TRUE) {
        return FALSE;
    }
    if(dataframe->full == TRUE) {
        dataframe->tail = dataframe->head;
        dataframe->full = FALSE;
    }
#ifdef BTT_RF
    dataframe->tail = W25Q64_FindNextAddr(dataframe->tail, W25Q64_DATA_START_ADDR, W25Q64_DATA_END_ADDR, W25Q64_DATA_BLOCKSIZE);
#endif
#ifdef VERSION_BMS
    dataframe->tail = W25Q64_FindPreviousAddr(dataframe->tail, W25Q64_DATA_END_ADDR, W25Q64_DATA_START_ADDR, W25Q64_DATA_BLOCKSIZE);
#endif
    if(dataframe->tail == dataframe->head) {
        dataframe->empty = TRUE;
    } else {
        dataframe->empty = FALSE;
    }
    return TRUE;
}

// U8 W25Q64_ReadOneDataBlock(flashdata_frame *dataframe, U8 *buff_ptr)
// {
//     U8 BYTE0 = 0;
//     while(1) {
//         if(dataframe->empty == TRUE) {
//             return FALSE;
//         }
//         if(dataframe->full == TRUE) {
//             dataframe->tail = dataframe->head;
//             dataframe->full = FALSE;
//         }
//         dataframe->tail = W25Q64_FindNextAddr(dataframe->tail, W25Q64_DATA_START_ADDR, W25Q64_DATA_END_ADDR, W25Q64_DATA_BLOCKSIZE);
//         if(dataframe->tail == dataframe->head) {
//             dataframe->empty = TRUE;
//         } else {
//             dataframe->empty = FALSE;
//         }
//         SPI_Flash_Read(&BYTE0, dataframe->tail, 1);
//         if((BYTE0 == 0xff) || (BYTE0 == 0xfc)) {
//             continue;
//         } else if (BYTE0 == 0xfe) {
//             SPI_Flash_Read(buff_ptr, dataframe->tail, W25Q64_DATA_BLOCKSIZE);
//             if(W25Q64_CheackBufferCRC(&buff_ptr[1], W25Q64_DATA_BLOCKSIZE - 1) == TRUE) {
//                 Set_FlashFlag(BYTE0, BLOCK_READ_BITPOS);
//                 SPI_Flash_Write_NoCheck(&BYTE0, dataframe->tail, 1);
//                 return TRUE;
//             } else {
//                 Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
//                 SPI_Flash_Write_NoCheck(&BYTE0, dataframe->tail, 1);
//                 continue;
//             }
//         } else {
//             Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
//             SPI_Flash_Write_NoCheck(&BYTE0, dataframe->tail, 1);
//             continue;
//         }
//     }
// }

#if 0

U8 W25Q64_WriteOneDataBlock(flashdata_frame *dataframe, U8 *buff_ptr)
{
    U32 next_addr = 0, ex_addr = 0;
    U8 temp_buff[W25Q64_DATA_BLOCKSIZE] = {0};
    U8 BYTE0 = 0;
    while(1) {
		WDOG_Feed();
        ex_addr = dataframe->head;
        next_addr = W25Q64_FindNextAddr(ex_addr, W25Q64_DATA_START_ADDR, W25Q64_DATA_END_ADDR, W25Q64_DATA_BLOCKSIZE); //addr HEAD+1
        dataframe->head = next_addr;
        if(W25Q64_IsSameSector(dataframe->head, ex_addr) == FALSE) { //beyond Sector?
            if(W25Q64_CheackThisSectorByBlock(dataframe->head, W25Q64_DATA_BLOCKSIZE) == TRUE) { // is Sector used
                SPI_Flash_Erase_Sector(dataframe->head);
            }
        }
        SPI_Flash_Read(&BYTE0, dataframe->head, 1); //READ BYTE0
        if(BYTE0 == 0xff) { //empty block
            W25Q64_AddCRCToBufferEnd(&buff_ptr[1], W25Q64_DATA_BLOCKSIZE - 1);
            SPI_Flash_Write_NoCheck(buff_ptr, dataframe->head, W25Q64_DATA_BLOCKSIZE);
            SPI_Flash_Read(temp_buff, dataframe->head, W25Q64_DATA_BLOCKSIZE);
            if(W25Q64_CheackBufferCRC(&temp_buff[1], W25Q64_DATA_BLOCKSIZE - 1) == TRUE) {
                Set_FlashFlag(BYTE0, BLOCK_WRITE_BITPOS);
                SPI_Flash_Write_NoCheck(&BYTE0, dataframe->head, 1);
                dataframe->empty = FALSE;
                return TRUE;
            } else {
                Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
                SPI_Flash_Write_NoCheck(&BYTE0, dataframe->head, 1);
                continue;
            }
        } else {
            continue;
        }

    }
}
U8 W25Q64_ReadOneDataBlock(flashdata_frame *dataframe, U8 *buff_ptr)
{
    U8 BYTE0 = 0;
    U8 loop_continue = FALSE;
    while(1) {
		WDOG_Feed();
        loop_continue = FALSE;
        if(dataframe->empty == TRUE) {
            return FALSE;
        }
        SPI_Flash_Read(&BYTE0, dataframe->tail, 1);
        if (BYTE0 == 0xfe) {
            SPI_Flash_Read(buff_ptr, dataframe->tail, W25Q64_DATA_BLOCKSIZE);
            if(W25Q64_CheackBufferCRC(&buff_ptr[1], W25Q64_DATA_BLOCKSIZE - 1) == FALSE) {
                Set_FlashFlag(BYTE0, BLOCK_WRONG_BITPOS);
                SPI_Flash_Write_NoCheck(&BYTE0, dataframe->tail, 1);
                loop_continue = TRUE;
            }
        } else {
            loop_continue = TRUE;
        }
        dataframe->tail = W25Q64_FindPreviousAddr(dataframe->tail, W25Q64_DATA_LASTBLOCKADDR, W25Q64_DATA_START_ADDR, W25Q64_DATA_BLOCKSIZE);
        if(dataframe->tail == dataframe->head) {
            dataframe->empty = TRUE;
        } else {
            dataframe->empty = FALSE;
        }
        if(loop_continue == TRUE) {
            continue;
        } else {
            return TRUE;
        }


    }
}
#endif

void FlashDataBlockSave(void)
{
    g_datablock_data.UinxTime = g_log_data.log_data.UinxTime;
    g_datablock_data.PackVoltageValue = g_log_data.log_data.PackVoltageValue;
    g_datablock_data.NowCurrent = g_log_data.log_data.NowCurrent;
    g_datablock_data.NowSOC = g_log_data.log_data.NowSOC;
    g_datablock_data.Cell_T[0] = g_log_data.log_data.Cell_T[0];
    g_datablock_data.Cell_T[1] = g_log_data.log_data.Cell_T[1];
    for(U8 i = 0; i < 5; i++){
        g_datablock_data.CellVoltageValue[i] = g_battery_pack_info.sample_data.cell_voltages[i];
    }
    for(U8 i = 0; i < 9; i++){
        g_datablock_data.CellVoltageValue[i+5] = g_battery_pack_info.sample_data.cell_voltages[i+8];
    }
    for(U8 i = 0; i < 5; i++){
        g_datablock_data.CellVoltageValue[i+14] = g_battery_pack_info.sample_data.cell_voltages[i+19];
    }
    memcpy(g_datablock_data.AlarmFlag, g_battery_pack_info.error_flag.data, 5);
    W25Q64_WriteOneDataBlock(&g_flashdata_frame, (U8 *)&g_datablock_data);
    W25Q64_SetDataPtrMSG(&g_dataptr, &g_flashdata_frame);
}

void W25Q64_Init(void)
{
    SPI1_C2 &= (U8)~SPI_C2_MODFEN_MASK;
    SPI_Flash_WAKEUP();
}

void W25Q64_EraseProgramArea(U32 StartAddr)
{
    for(U32 i = 0; i < W25Q64_SLAVE_PROGRAM_BLOCKSIZE; i += W25Q64_SECTOR_SIZE){
        SPI_Flash_Erase_Sector(StartAddr + i);
    }
}

void W25Q64_SaveStaticErrBlock(FlashStatic_Data_Union *staicdata){
    (staicdata->static_data.StaticErrBlockFrame.HeadAddr < (CELL_H_VOLT_BLOCK_NUM - 1)) ?
        (staicdata->static_data.StaticErrBlockFrame.HeadAddr++) :
        (staicdata->static_data.StaticErrBlockFrame.HeadAddr = 0);
        
    U8 index = staicdata->static_data.StaticErrBlockFrame.HeadAddr;
    staicdata->static_data.StaticErrBlockFrame.StaticErrBlock[index].UinxTime = g_log_data.log_data.UinxTime;
    staicdata->static_data.StaticErrBlockFrame.StaticErrBlock[index].PackVoltageValue = g_log_data.log_data.PackVoltageValue;
    staicdata->static_data.StaticErrBlockFrame.StaticErrBlock[index].NowCurrent = g_log_data.log_data.NowCurrent;
    staicdata->static_data.StaticErrBlockFrame.StaticErrBlock[index].NowSOC = g_log_data.log_data.NowSOC;
    staicdata->static_data.StaticErrBlockFrame.StaticErrBlock[index].Cell_T[0] = g_log_data.log_data.Cell_T[0];
    staicdata->static_data.StaticErrBlockFrame.StaticErrBlock[index].Cell_T[1] = g_log_data.log_data.Cell_T[1];
    for(U8 i = 0; i < 5; i++){
        staicdata->static_data.StaticErrBlockFrame.StaticErrBlock[index].CellVoltageValue[i] = g_battery_pack_info.sample_data.cell_voltages[i];
    }
    for(U8 i = 0; i < 9; i++){
        staicdata->static_data.StaticErrBlockFrame.StaticErrBlock[index].CellVoltageValue[i+5] = g_battery_pack_info.sample_data.cell_voltages[i+8];
    }
    for(U8 i = 0; i < 5; i++){
        staicdata->static_data.StaticErrBlockFrame.StaticErrBlock[index].CellVoltageValue[i+14] = g_battery_pack_info.sample_data.cell_voltages[i+19];
    }
    memcpy(staicdata->static_data.StaticErrBlockFrame.StaticErrBlock[index].AlarmFlag, &g_battery_pack_info.error_flag, 5);
    W25Q64_SetStaticMSG(&g_static_data);
}

void W25Q64_EnableStaticErrBlockSave(void)
{
    StaticErrBlockSaveEnable = TRUE;
}

void W25Q64_FlashStaticBlockCellHVoltSaveProc(void){
    if(StaticErrBlockSaveEnable == TRUE){
        //Cell high volt fault occur
        if((g_static_data.static_data.StaticErrBlockFrame.LastCHVFlag != 3) &&
            (g_battery_pack_info.error_flag.mergedBits.BMS_CellHVoltFlt == 3)){
            g_static_data.static_data.StaticErrBlockFrame.LastCHVFlag = 
                g_battery_pack_info.error_flag.mergedBits.BMS_CellHVoltFlt;
            W25Q64_SaveStaticErrBlock(&g_static_data);
            // W25Q64_SetStaticMSG(&g_static_data);
        }else
     //     if((g_static_data.static_data.StaticErrBlockFrame.LastCHVFlag != 0) &&
     //                (g_battery_pack_info.error_flag.mergedBits.BMS_CellHVoltFlt == 0)){
     //        //Cell high volt fault remove
    	// 	    g_static_data.static_data.StaticErrBlockFrame.LastCHVFlag = 
     //            g_battery_pack_info.error_flag.mergedBits.BMS_CellHVoltFlt;
     //        W25Q64_SetStaticMSG(&g_static_data);    
    	// }else
        {
            g_static_data.static_data.StaticErrBlockFrame.LastCHVFlag = 
            g_battery_pack_info.error_flag.mergedBits.BMS_CellHVoltFlt;
			// g_battery_pack_info.error_flag.mergedBits.BMS_CellHVoltFlt = 3;
        }
    }
}

void W25Q64_WdtResetRecord(void)
{
    if(SIM->SRSID & SIM_SRSID_WDOG_MASK)
    {
        g_battery_pack_info.error_flag.mergedBits.BMS_WdtResetFlt = TRUE;
        W25Q64_SaveStaticErrBlock(&g_static_data);
        g_battery_pack_info.error_flag.mergedBits.BMS_WdtResetFlt = FALSE;
    }
}

void W25Q64_UnixtimeSyncRecord(void)
{
    g_battery_pack_info.error_flag.mergedBits.BMS_UnixtimeSync = TRUE;
    FlashDataBlockSave();
    g_battery_pack_info.error_flag.mergedBits.BMS_UnixtimeSync = FALSE;
}


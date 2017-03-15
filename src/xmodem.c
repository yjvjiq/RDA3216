#include "xmodem.h"
//#include "flash.h"
#include "crc16.h"
#include "mscan.h"
#include "w25q64.h"
#include "main.h"
#include "debug.h"
#include "timer.h"

#ifdef XMODEMBOOT_ENABLE

static uint32_t appCodeStartAddr;
static uint8_t xmdPacketBuffer[PKTLEN_128+PKTBASELEN];
tXmodemRspType xmdSlaveRspBuffer;

static tXmodemInfoType xmodemInfo;
static tBootFlashType bootFlashInfo;
static tBootSectorInfoType bootSectorInfo;
static U32 XModemTimeOut_MS;
static U32 XModemPacket_MS;
//static U32 XModemPutChar_MS;
//static U32 XModemSlavePacket_MS;

extern U8 g_SoftVersion[4]; 

static void WriteImage2Flash(uint8_t * data, uint16_t data_size);

/*
*XmodemPutChar:xmodem协议回应输出
*rsp:xmodem协议ACK, NAK
*/
static void XmodemPutChar(char rsp)
{

    xmdSlaveRspBuffer.rsp_data[0]=rsp;
    xmdSlaveRspBuffer.rsp_data[1]=xmodemInfo.idx;  
    xmdSlaveRspBuffer.rsp_data[2]=xmodemInfo.idx>>8;
    xmdSlaveRspBuffer.length = 3;
    xmdSlaveRspBuffer.flag = TRUE;
    if((XMD_ACK == rsp)||(XMD_NAK == rsp))
    {
      xmodemInfo.state = BOOT_START;
      xmodemInfo.offset = 0;
      xmodemInfo.subidx = 1;
      SetTimer1(XModemPacket_ID, XModemPacket_MS, TIMER_MODE_SINGLE, NULL);
    }
}

static void XmodemSendMsg(uint8_t* data, uint8_t len)
{
    if(len>MAX_SINGLE_FRAME_LENGTH) len = MAX_SINGLE_FRAME_LENGTH;
    memcpy( xmdSlaveRspBuffer.rsp_data,data,len);
    xmdSlaveRspBuffer.length = len;
    xmdSlaveRspBuffer.flag = TRUE; 
}
void XmodemSendVersion(uint8_t* para)
{
  para[1] |= 0x80; 
  para[3] = g_stateblock_data.stata_union.state_data.ComAddr;
  memcpy(&para[4],g_SoftVersion,3);
  XmodemSendMsg(para,8);
}
static uint16_t XmodemGetCRC(uint8_t *buffer, uint8_t length)
{
    crc16_data_t crcInfo;
    uint16_t crc;
    crc16_init(&crcInfo);
    crc16_update(&crcInfo, buffer, length);
    crc16_finalize(&crcInfo, &crc);
    return crc;
}

static uint8_t XmodemGetCheckSum(uint8_t *buffer, uint8_t length)
{
    uint8_t checkSum = 0;
    for(uint8_t i=0; i<length; i++)    checkSum ^= *buffer++;
    return checkSum;
}

static uint8_t BootFlashValidCheck(tBootFlashType* bootInfo)
{
    #define INVALID_LENGTH  (0xFFFFFFFF)
    uint8_t ret1, ret2;
    
    if((__SWP16(bootInfo->ChkSum) == XmodemGetCRC((uint8_t*)bootInfo, sizeof(tBootFlashType)-2))
        &&(FLASH_KEY == __SWP16(bootInfo->keyWord))&&(bootInfo->SectorIdx == (uint8_t)(bootInfo->SectorIdxInv^0xFF))
        &&(bootInfo->SectorIdx<FLASH_SECTOR_MAX))
    {
      ret1 = XmodemGetCheckSum((uint8_t*)&bootInfo->sector1, sizeof(tBootSectorInfoType));
      if((ret1)||(FLASH_KEY!=__SWP16(bootInfo->sector1.keyWord))||(INVALID_LENGTH==bootInfo->sector1.length))
      {
        bootInfo->sector1.validFlag = FALSE;
        bootInfo->sector1.keyWord   = 0;
      }
      
      ret2 = XmodemGetCheckSum((uint8_t*)&bootInfo->sector2, sizeof(tBootSectorInfoType));
      if((ret2)||(FLASH_KEY!=__SWP16(bootInfo->sector2.keyWord))||(INVALID_LENGTH==bootInfo->sector2.length))
      {
        bootInfo->sector2.validFlag = FALSE;
        bootInfo->sector2.keyWord   = 0;
      }
    }
    else
    {
      bootInfo->sector1.validFlag = FALSE;
      bootInfo->sector1.keyWord   = 0;
      bootInfo->sector2.validFlag = FALSE;
      bootInfo->sector2.keyWord   = 0;
    }
    return ret1|ret2;
}

static uint32_t GetXmodemFlashAddr(void)
{
    uint32_t startAddr = W25Q64_PROGRAM_SECTOR1_START_ADDR;
    xmodemInfo.flashSector = FLASH_SECTOR_1;  
    
    //xSemaphoreTake(xMCP2515Mutex, portMAX_DELAY);
    //NVIC_DisableIRQ(IRQ_IRQn);
    __disable_irq();
    SPI_Flash_Read((uint8_t*)&bootFlashInfo, W25Q64_PROGRAM_INFORMATION_START_ADDR, sizeof(tBootFlashType));
    __enable_irq();
    //NVIC_EnableIRQ(IRQ_IRQn);
    //xSemaphoreGive(xMCP2515Mutex);
    
    xmodemInfo.updateBootFlash = BootFlashValidCheck(&bootFlashInfo);
    
    if((bootFlashInfo.sector1.validFlag)&&(FLASH_SECTOR_1 == bootFlashInfo.SectorIdx))
    {
      xmodemInfo.flashSector = FLASH_SECTOR_2;
      startAddr = W25Q64_PROGRAM_SECTOR2_START_ADDR;
    }
    return startAddr;
}

/*fill data with a 3-bytes value of boot ID*/
void GetBootVersion(U8* data)
{
	for(U8 i=0;i<BOOT_VERSION_LEN;i++){
		*(data+i) = *((uint8_t*)BOOT_VERSION_ADDR+i);
	}
}

void SetBootflag(tBootInfo_Type*bootversion)
{
    bootversion->mark = __SWP16(FLASH_KEY);
    bootversion->updateflag = TRUE; 
    bootversion->chkSum = __SWP16(XmodemGetCRC((uint8_t*)bootversion, sizeof(tBootInfo_Type)-2));
    appCodeStartAddr = W25Q64_BOOTVERSION_INFORMATION_START_ADDR;
    WriteImage2Flash((uint8_t*)bootversion, sizeof(tBootInfo_Type));
}

static uint8_t XmodemUpdateBootFlashPara(void)
{
    appCodeStartAddr = W25Q64_PROGRAM_INFORMATION_START_ADDR;
    WriteImage2Flash((uint8_t*)&bootFlashInfo, sizeof(tBootFlashType));

    return TRUE;
}

static uint8_t XmodemFinished(uint8_t success)
{
    uint8_t flashSector = xmodemInfo.flashSector;
    eBootCodeType bootCode = xmodemInfo.bootCode;
    XmodemInit();
    StopTimer1(XModemTimeOut_ID);
    StopTimer1(XModemPacket_ID);
    if(BMS_Slave != bootCode) return TRUE;

    if(success)
    {
      uint8_t *paraAddr = (uint8_t*)&bootFlashInfo.sector1;
      bootSectorInfo.validFlag = TRUE;
      bootSectorInfo.keyWord  = __SWP16(FLASH_KEY);
      bootSectorInfo.checkSum = 0;
      bootSectorInfo.checkSum = XmodemGetCheckSum((uint8_t*)&bootSectorInfo, sizeof(tBootSectorInfoType));
      if(FLASH_SECTOR_2 == flashSector)  paraAddr = (uint8_t*)&bootFlashInfo.sector2;
      
      memcpy(paraAddr, (uint8_t*)&bootSectorInfo, sizeof(tBootSectorInfoType));
      bootFlashInfo.version = bootSectorInfo.version;
      bootFlashInfo.length  = bootSectorInfo.length;
      bootFlashInfo.keyWord = __SWP16(FLASH_KEY);
      bootFlashInfo.SectorIdx     = flashSector;
      bootFlashInfo.SectorIdxInv  = (uint8_t)(flashSector^0xFF);
      bootFlashInfo.BIN_CRC = bootSectorInfo.crc;
      bootFlashInfo.ChkSum  = __SWP16(XmodemGetCRC((uint8_t*)&bootFlashInfo, sizeof(tBootFlashType)-2));
      
      XmodemUpdateBootFlashPara();
      tBootInfo_Type bootInfo;
      SetBootflag(&bootInfo);

      printf("NVIC_SystemReset\n");
      //for(uint32 i;i<3000;i++)
       // ;
      //NVIC_SystemReset();
      SetTimer1(XModemReset_ID, 1000, TIMER_MODE_SINGLE, NULL);
   }
    
   return TRUE;
}
void UpdataFromSpiFlash(uint8_t cmd)
{ 
    __disable_irq();
    SPI_Flash_Read((uint8_t*)&bootFlashInfo, W25Q64_PROGRAM_INFORMATION_START_ADDR, sizeof(tBootFlashType));
    __enable_irq();
    
  
    tBootInfo_Type bootInfo;
    switch(cmd)
    {
       case BOOT_CURRENT_VALID_SECTOR:
        SetBootflag(&bootInfo);     
        //NVIC_SystemReset();
        SetTimer1(XModemReset_ID, 1000, TIMER_MODE_SINGLE, NULL);
        break;
       case BOOT_ANOTHER_VALID_SECTOR:
        if((bootFlashInfo.sector1.validFlag)&&(FLASH_SECTOR_1 == bootFlashInfo.SectorIdx))
        {
          if(bootFlashInfo.sector2.validFlag)
          {
            bootFlashInfo.version = bootSectorInfo.version;
            bootFlashInfo.length  = bootSectorInfo.length;
            bootFlashInfo.keyWord = __SWP16(FLASH_KEY);
            bootFlashInfo.SectorIdx     = FLASH_SECTOR_2;
            bootFlashInfo.SectorIdxInv  = (uint8_t)(FLASH_SECTOR_2^0xFF);
            bootFlashInfo.BIN_CRC = bootSectorInfo.crc;
            bootFlashInfo.ChkSum  = __SWP16(XmodemGetCRC((uint8_t*)&bootFlashInfo, sizeof(tBootFlashType)-2));

            XmodemUpdateBootFlashPara();
            SetBootflag(&bootInfo);   
            //NVIC_SystemReset();              
            SetTimer1(XModemReset_ID, 1000, TIMER_MODE_SINGLE, NULL);
          }
        } 
        else if((bootFlashInfo.sector2.validFlag)&&(FLASH_SECTOR_2 == bootFlashInfo.SectorIdx))
        {
            if(bootFlashInfo.sector1.validFlag)
            {             
              bootFlashInfo.version = bootSectorInfo.version;
              bootFlashInfo.length  = bootSectorInfo.length;
              bootFlashInfo.keyWord = __SWP16(FLASH_KEY);
              bootFlashInfo.SectorIdx     = FLASH_SECTOR_1;
              bootFlashInfo.SectorIdxInv  = (uint8_t)(FLASH_SECTOR_1^0xFF);
              bootFlashInfo.BIN_CRC = bootSectorInfo.crc;
              bootFlashInfo.ChkSum  = __SWP16(XmodemGetCRC((uint8_t*)&bootFlashInfo, sizeof(tBootFlashType)-2));
          
              XmodemUpdateBootFlashPara();
              SetBootflag(&bootInfo);  
              //NVIC_SystemReset();
              SetTimer1(XModemReset_ID, 1000, TIMER_MODE_SINGLE, NULL);
            }
        }
        break;
       default:
        break;
    }
  
}
void XmodemTimeOutCb(void)
{
    XmodemPutChar(XMD_CAN);
    XmodemFinished(FALSE);
    printf("Boot_TimeOut, total packet: %d, rcv %d\r\n", xmodemInfo.blocksSize, xmodemInfo.idx-1);

}
/*
void XmodemPutCharTimeOutCb(void)
{
    XmodemPutChar('C');
    printf("C");
    if(xmodemInfo.putC_Cnt++ > XMD_PUTC_ERR_CNT)
    {
      xmodemInfo.putC_Cnt = 0;
      xmodemInfo.state = BOOT_IDLE;
      StopTimer1(XModemPutChar_ID);
      printf("put C timeout, Boot Stop!\r\n");
    }
}
*/
void XmodemPacketTimeOutCb(void)
{
    if(xmodemInfo.timeOutCnt++ >= XMD_MAX_ERR_CNT)
    {
     XmodemPutChar(XMD_CAN);
     XmodemFinished(FALSE);
     printf("too many Error, Boot Stop!\r\n");
    }
    else
    {
     XmodemPutChar(XMD_NAK);
     printf("Packet %d TimeOut \r\n", xmodemInfo.idx);
    }
}
void XmodemResetTimeOutCb(void)
{
  NVIC_SystemReset();
}

void XmodemTimerInit(void)
{
  XModemTimeOut_MS = 90*1000;
  XModemPacket_MS = 2*1000;
  //XModemPutChar_MS = 1000;
  //XModemSlavePacket_MS = 1000;
}

static void XmodemInit(void)
{
    memset((void*)&xmodemInfo, 0, sizeof(tXmodemInfoType));
    xmodemInfo.state = BOOT_IDLE;
    xmodemInfo.idx   = 0x01;
    xmodemInfo.subidx = 0x01;
    xmodemInfo.bincrc = FALSE;
    XmodemTimerInit();
}

static void WriteImage2Flash(uint8_t * data, uint16_t data_size)
{
    //xSemaphoreTake(xMCP2515Mutex, portMAX_DELAY);
    //NVIC_DisableIRQ(IRQ_IRQn);
    __disable_irq();
    if(((appCodeStartAddr) & (W25Q64_SECTOR_SIZE-1))==0)
    {
      SPI_Flash_Erase_Sector(appCodeStartAddr);
    }
    else if(((appCodeStartAddr & (W25Q64_SECTOR_SIZE-1)) + data_size) > W25Q64_SECTOR_SIZE)
    {
      SPI_Flash_Erase_Sector((appCodeStartAddr+data_size)&(~(W25Q64_SECTOR_SIZE-1)));
    }
    SPI_Flash_Write_NoCheck(data, appCodeStartAddr, data_size);
    
    appCodeStartAddr += data_size;
    __enable_irq();
    //NVIC_EnableIRQ(IRQ_IRQn);
    //xSemaphoreGive(xMCP2515Mutex);
}


/*
*XmodemPacketMakeCRC：通过xmodem第一帧消息的头16个字节包含的整个文件长度，计算整个包的crc
（在接受xmodem协议每一帧数据时，需要做xmodem crc校验，同时也要开始整个bin文件crc的校验）
*buffer:xmodem协议128字节帧消息的缓存，
*返回值:整个包crc校验状态(置位状态)
*XMD_FILE_END:校验结束
*XMD_FILE_CRC_ERR:在校验结束后，整个包校验错误
*/
static uint8_t XmodemPacketMakeCRC(uint8_t *buffer)
{
    static crc16_data_t crcInfo;
    uint8_t ret = 0;

    if(xmodemInfo.idx == 1)
    {
      crc16_init(&crcInfo);
      //计算xmodem传输的帧数
      uint32_t len = (buffer[4]<<24)|(buffer[5]<<16)|(buffer[6]<<8)|(buffer[7]<<0);
      xmodemInfo.blocksSize = (uint16_t)((len+BIN_HEADER_16+2)/PKTLEN_128);
      xmodemInfo.blockRemain = (uint16_t)((len+BIN_HEADER_16+2)%PKTLEN_128);
      xmodemInfo.blocksSize += xmodemInfo.blockRemain>0 ? 1 : 0;

      if(XMODEM_BMS != buffer[XMD_FILE_DEV_TYPE_POS])
      {
        ret = XMD_FILE_END|XMD_FILE_CRC_ERR;
        printf("BOOT Device Type Error, Expect Device %d but Rcv File Device %d\r\n", XMODEM_BMS, buffer[XMD_FILE_DEV_TYPE_POS]);
        return ret;
      }
      
      xmodemInfo.bootDevice = (eBootDeviceType)buffer[XMD_FILE_DEV_TYPE_POS];
      xmodemInfo.bootCode   = (eBootCodeType)buffer[XMD_FILE_CODE_TYPE_POS];
      if(BMS_Slave == xmodemInfo.bootCode)
      {
        appCodeStartAddr = GetXmodemFlashAddr();  
      }
      memcpy((uint8_t*)&bootSectorInfo, buffer, sizeof(tBootSectorInfoType));

      XModemTimeOut_MS = 50*XMD_PACKET_TIMEOUT*xmodemInfo.blocksSize;
      SetTimer1(XModemTimeOut_ID, XModemTimeOut_MS, TIMER_MODE_SINGLE, NULL);
      
      printf("packet_no: %d %x\n", xmodemInfo.blocksSize, XModemTimeOut_MS);
    }

    if((xmodemInfo.blocksSize > 0) && (xmodemInfo.blocksSize == xmodemInfo.idx))
    {
      uint16_t crc16, verify16;
      ret |= XMD_FILE_END;
            
      crc16_update(&crcInfo, buffer, xmodemInfo.blockRemain-2);
      //得出整个bin文件crc结果
      crc16_finalize(&crcInfo, &verify16);
      crc16 = ((uint16_t)buffer[xmodemInfo.blockRemain-2] << 8)|((uint16_t)buffer[xmodemInfo.blockRemain-1]);

      if(crc16 != verify16){
        ret |= XMD_FILE_CRC_ERR;
      }else{
        xmodemInfo.bincrc = TRUE;
      }
      
      return ret;  
    }

    //计算整个bin文件的crc(含包头)，
    crc16_update(&crcInfo, buffer, PKTLEN_128);
    return ret;
}
/*
*XmodemReadPacket:对xmodem一帧消息128字节的读取，
*buffer: 帧消息存放地址
*返回值:xmodem帧的crc校验(置位状态)
*包含XmodemPacketMakeCRC函数的返回值
*XMD_CRC_ERR xmodem校验错误
*XMD_IDX_ERR 收到的包序号小于xmodemInfo.idx，可能对方没收到ack
*/
static uint8_t XmodemReadPacket(uint8_t *packet)
{
    uint8_t seq[2];
    uint16_t crc16, verify16;
    crc16_data_t crcInfo;
    uint8_t ret = 0;

    seq[0] = packet[XMD_IDX_POS];
    seq[1] = packet[XMD_IDX_INV_POS];
    
    //每帧数据的CRC校验
    crc16_init(&crcInfo);
    crc16_update(&crcInfo, &packet[XMD_PACKET_POS], PKTLEN_128);
    crc16_finalize(&crcInfo, &verify16);

    crc16  = ((uint16_t)packet[XMD_CRC_H_POS] << 8)|packet[XMD_CRC_L_POS];

    if(((crc16 != verify16) || (seq[0] != (uint8_t)(seq[1]^0xff)))||((xmodemInfo.blocksSize == 0)&&(seq[0] != xmodemInfo.idx)))
    {
      ret |= XMD_CRC_ERR;
      printf("__crcerr1\n");
      return ret;
    }
    else if((xmodemInfo.blocksSize > 0)&&(seq[0] != (xmodemInfo.idx%256)))
    {
      if((/*(seq[0] != (xmodemInfo.blocksSize%256))&&*/((seq[0]+1) == (xmodemInfo.idx%256))) || (xmodemInfo.idx == xmodemInfo.blocksSize))      
        ret |= XMD_IDX_ERR;

        
      printf("__crcerr2\n");
      ret |= XMD_CRC_ERR;
      return ret;
    }

    if(TRUE == xmodemInfo.bincrc) {ret|= XMD_IDX_ERR;ret|=XMD_CRC_ERR; return ret;}

    ret |= XmodemPacketMakeCRC(&packet[XMD_PACKET_POS]);
    return ret;
}

/*
*XmodemPacketProcess:根据ReadPacket的返回值写片外flash或异常处理
*ret:ReadPacket函数返回值
*buffer:消息存放地址
*size:收到的字节数
*/
static eBootSTATE XmodemPacketProcess(uint8_t *packetBuffer)
{
    eBootSTATE state = BOOT_START;
    
    uint8_t result = XmodemReadPacket(packetBuffer);
    
    xmodemInfo.offset = 0;
    
    if(!(result&XMD_CRC_ERR))
    {
      if(result&XMD_FILE_END)
      {
        if(result&XMD_FILE_CRC_ERR)
        {
          XmodemPutChar(XMD_CAN); //send CAN or ACK???????????
          XmodemFinished(FALSE);
          printf("makeCRC Failed idx: %d, size: %d\r\n", xmodemInfo.idx, xmodemInfo.size);
        }
        else
        {
          if(BMS_Slave == xmodemInfo.bootCode)
          {
            WriteImage2Flash((uint8_t*)&packetBuffer[XMD_PACKET_POS], PKTLEN_128);
            xmodemInfo.size += PKTLEN_128;
            XmodemPutChar(XMD_ACK);
            printf("makeCRC Successs idx: %d, size: %d\r\n", xmodemInfo.idx, xmodemInfo.size);
          }
          //state = BOOT_DONE;
        }
      }
      else
      {
        if(BMS_Slave == xmodemInfo.bootCode)
        {
          WriteImage2Flash(&packetBuffer[XMD_PACKET_POS], PKTLEN_128);
          xmodemInfo.idx++;
          xmodemInfo.size += PKTLEN_128;
          XmodemPutChar(XMD_ACK);
          printf("xmd Succ idx: %d\r\n", xmodemInfo.idx);
        }
      }
    }
    else if(result&XMD_IDX_ERR)
    {
      printf("maybe lost ack idx: %d \r\n",xmodemInfo.idx);
      XmodemPutChar(XMD_ACK);
    }
    else
    {
      printf("CRC Err idx: %d \r\n",xmodemInfo.idx); 
      XmodemPutChar(XMD_NAK);
    }
    return state;
}

uint8_t cmd_type;
void Slave_UpdataHandler(uint8_t* data)
{
   cmd_type = data[XMD_CMD_TYPE_POS];

   switch(data[XMD_CMD_FUNC_POS])
   {
      case XMODEM_REQ:
       XmodemInit();   
       data[XMD_CMD_FUNC_POS] |= 0x80;
       XmodemSendMsg(data, 8);   
       xmodemInfo.state = BOOT_START;
       //SetTimer1(XModemPutChar_ID,XModemPutChar_MS, TIMER_MODE_PERIODIC, NULL);
       break;
      case XMODEM_CAN_REQ:
        data[XMD_CMD_FUNC_POS] |= 0x80;
        XmodemSendMsg(data, 8);
        XmodemFinished(FALSE);
        break;
      case BOOT_CURRENT_VALID_SECTOR:
      case BOOT_ANOTHER_VALID_SECTOR:
        data[XMD_CMD_FUNC_POS] |= 0x80;        
        XmodemSendMsg(data, 8);
        UpdataFromSpiFlash(data[XMD_CMD_FUNC_POS]&0x0f);
        break;
      case XMODEM_READ_VERSION: 
        XmodemSendVersion(data);
       break;
      default:
       break;
    }
}

void UpdataHandler(uint8_t* data)
{
    switch(data[XMD_CMD_TYPE_POS])
    {
      case CMD_BMS_Slave:
      case CMD_EMS_Slave:
        //rf发送升级命令
        Slave_UpdataHandler(data);
        break;
      default:
        printf("cmd type err\n");
        break;
    }
}
uint8_t XmodemMsgHandler(uint8_t *data, uint8_t length)
{
    #define KEY 0xaabbcc
    uint32_t key = (data[XMD_CMD_KEY1_POS]<<16)|(data[XMD_CMD_KEY2_POS]<<8)|data[XMD_CMD_KEY3_POS];

    printf("xmd state:%d\n",xmodemInfo.state);
    
    switch(xmodemInfo.state)
    {
      case BOOT_IDLE:
        if((XMD_REQ == data[XMD_CMD_MASK_POS])&&(KEY == key)&&(XMD_DATA_CAN_LEN == length))
        {
           UpdataHandler(data);
        }
        break;
      case BOOT_START:
        switch(data[0])
        {
          case 0x01:
            if((XMD_SOH == data[1])&&(data[2]==(uint8_t)(data[3]^0xFF)))
            {
              memcpy(xmdPacketBuffer, &data[1], (length-1));
              xmodemInfo.offset = (length-1);
              xmodemInfo.state  = BOOT_BUSY;
              xmodemInfo.timeOutCnt = 0;
              xmodemInfo.subidx++;
              
              XmodemPutChar(XMD_FLW);
              
              if(1 == xmodemInfo.idx)
              {
                //StopTimer1(XModemPutChar_ID);
                XModemTimeOut_MS = XMD_PACKET_TIMEOUT*512;
                if(CMD_EMS_Slave == cmd_type) XModemPacket_MS = 20*1000;
                SetTimer1(XModemTimeOut_ID, XModemTimeOut_MS, TIMER_MODE_SINGLE, NULL);
                SetTimer1(XModemPacket_ID, XModemPacket_MS, TIMER_MODE_SINGLE, NULL);
              }
            }
            break;
          case XMD_ESC:
          case XMD_CAN:
            if(1 == length)
            {
              XmodemPutChar(XMD_EOT);
              XmodemFinished(FALSE);
            }
            break;
          case XMD_EOT:
            if(1 == length)
            {
              printf("EOT: indx: %d ,size: %d\r\n", xmodemInfo.idx, xmodemInfo.size);
              XmodemPutChar(XMD_EOT);
              XmodemFinished(TRUE);            }
            break;
          case XMD_REQ:
            if((XMODEM_CAN_REQ == data[XMD_CMD_FUNC_POS])&&(KEY == key)&&(XMD_DATA_CAN_LEN == length))
            {
               UpdataHandler(data);
            }
            break;
          default:
            break;
        }
        break;
      case BOOT_BUSY:
        if(data[0] != xmodemInfo.subidx)
        {
          printf("data[0]:%d,xmodemInfo.subidx:%d\n",data[0],xmodemInfo.subidx);
           return FALSE;
        }
     
        memcpy(&xmdPacketBuffer[xmodemInfo.offset], &data[1],(length-1));
        xmodemInfo.offset += (length-1);
        xmodemInfo.subidx++; 
        if(xmodemInfo.offset >= XMODEM_BODY_SIZE + XMODEM_HEAD_SIZE)
        {
          xmodemInfo.subidx = 1;
          StopTimer1(XModemPacket_ID);
          xmodemInfo.state = XmodemPacketProcess(xmdPacketBuffer);
        }
        else
        {
          XmodemPutChar(XMD_FLW);
        }
        break;
      default:
        break;
    }
    return TRUE;
}
#endif

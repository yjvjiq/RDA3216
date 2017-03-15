#ifndef _W25Q64_H_
#define _W25Q64_H_
#include "drivers.h"
#include "public.h"
#include "config.h"
//#define FLASH_SPI_CS_ON()  Gpioa_SetValue(FLASH_SPI_CS,0)
//#define FLASH_SPI_CS_OFF() Gpioa_SetValue(FLASH_SPI_CS,1)
#define FLASH_SPI_CS_ON()  OUTPUT_CLEAR(PTE,PTE2)
#define FLASH_SPI_CS_OFF() OUTPUT_SET(PTE,PTE2)

#define W25X_WriteEnable             0x06
#define W25X_WriteDisable            0x04
#define W25X_ReadStatusReg           0x05
#define W25X_WriteStatusReg          0x01
#define W25X_ReadData                0x03
#define W25X_FastReadData            0x0B
#define W25X_FastReadDual            0x3B
#define W25X_PageProgram             0x02
#define W25X_BlockErase              0xD8
#define W25X_SectorErase             0x20
#define W25X_ChipErase               0xC7
#define W25X_PowerDown               0xB9
#define W25X_ReleasePowerDown        0xAB
#define W25X_DeviceID                0xAB
#define W25X_ManufactDeviceID        0x90
#define W25X_JedecDeviceID           0x9F

#define W25Q64_CHIP_SIZE   0x100000 //1M bytes

#define W25Q64_SECTOR_SIZE 4096 //4k 4096bytes
#define W25Q64_PAGE_SIZE  256  // 256 bytes

#ifdef VERSION_BMS
#define W25Q64_STATE_AREASIZE (W25Q64_SECTOR_SIZE*4)  //16k  16384bytes 0x4000
#endif
#ifdef VERSION_BTT
#define W25Q64_STATE_AREASIZE (W25Q64_SECTOR_SIZE*125)  //16k  16384bytes 0x4000
#endif

#define W25Q64_STATE_BLOCKSIZE 64    //64bytes
#define W25Q64_STATE_BLOCKS  (W25Q64_STATE_AREASIZE/W25Q64_STATE_BLOCKSIZE) //
#define W25Q64_STATE_START_ADDR 0
#define W25Q64_STATE_LASTBLOCKADDR (W25Q64_STATE_AREASIZE - W25Q64_STATE_BLOCKSIZE)
#define W25Q64_STATE_END_ADDR   (W25Q64_STATE_AREASIZE-1) //0x003fff

// #define W25Q64_STATE_WRITEFLAG_POS 0
// #define W25Q64_STATE_FLAGS_POS     1
// #define W25Q64_STATE_HEAD_POS      2
//#define W25Q64_STATE_TAIL_POS      5
// #define W25Q64_STATE_CRC_POS      (W25Q64_STATE_BLOCKSIZE - 2)

#define STATE_EMPTY_BITPOS  0
#define STATE_FULL_BITPOS   1

#define W25Q64_DATA_AND_PTR_REMAIN_BLOCKS	((W25Q64_CHIP_SIZE - W25Q64_STATE_AREASIZE - \
 								 	W25Q64_PROGRAM_SECTOR1_BLOCKSIZE-W25Q64_PROGRAM_SECTOR2_BLOCKSIZE - W25Q64_SECTOR_SIZE) / W25Q64_SECTOR_SIZE)
#define W25Q64_DATA_AND_PTR_RATIO	(W25Q64_DATA_BLOCKSIZE/W25Q64_DATA_PTR_BLOCKSIZE)

#define W25Q64_DATA_PTR_AREASIZE 	((W25Q64_DATA_AND_PTR_REMAIN_BLOCKS / (W25Q64_DATA_AND_PTR_RATIO+1) + \
									W25Q64_DATA_AND_PTR_REMAIN_BLOCKS % (W25Q64_DATA_AND_PTR_RATIO+1)) * W25Q64_SECTOR_SIZE)	

#define W25Q64_DATA_PTR_BLOCKSIZE (sizeof(s_FlashDataPtr))
#define W25Q64_DATA_PTR_BLOCKS  (W25Q64_DATA_PTR_AREASIZE/W25Q64_DATA_PTR_BLOCKSIZE) //
#define W25Q64_DATA_PTR_START_ADDR W25Q64_STATE_AREASIZE
#define W25Q64_DATA_PTR_LASTBLOCKADDR (W25Q64_DATA_PTR_START_ADDR + W25Q64_DATA_PTR_AREASIZE - W25Q64_DATA_PTR_BLOCKSIZE)
#define W25Q64_DATA_PTR_END_ADDR   (W25Q64_DATA_PTR_START_ADDR + W25Q64_DATA_PTR_AREASIZE-1) 

#define W25Q64_STATIC_START_ADDR (W25Q64_PROGRAM_SECTOR2_START_ADDR - W25Q64_SECTOR_SIZE)
#define W25Q64_STATIC_BLOCKSIZE  (sizeof(FlashStatic_Data))

#define W25Q64_DATA_AREASIZE  (W25Q64_STATIC_START_ADDR - W25Q64_STATE_AREASIZE - W25Q64_DATA_PTR_AREASIZE) //0x7c00 
#ifdef VERSION_BMS
#define W25Q64_DATA_BLOCKSIZE  64    //512bytes
#endif
#ifdef VERSION_BTT
#define W25Q64_DATA_BLOCKSIZE  128    //512bytes
#endif
#define W25Q64_DATA_BLOCKS    (W25Q64_DATA_AREASIZE/W25Q64_DATA_BLOCKSIZE) //32
#define W25Q64_DATA_START_ADDR (W25Q64_STATE_AREASIZE +  W25Q64_DATA_PTR_AREASIZE)                   //0x4000
#define W25Q64_DATA_LASTBLOCKADDR (W25Q64_STATIC_START_ADDR-W25Q64_DATA_BLOCKSIZE)
#define W25Q64_DATA_END_ADDR   (W25Q64_STATIC_START_ADDR - 1)                    //0x7fffff
#define W25Q64_DATA_WRITEFLAG_POS  0
#define W25Q64_DATA_CRC_POS      (W25Q64_DATA_BLOCKSIZE - 2)

//#define W25Q64_SLAVE_PROGRAM_START_ADDR     (W25Q64_CHIP_SIZE - W25Q64_SLAVE_PROGRAM_BLOCKSIZE) 
#define W25Q64_SLAVE_PROGRAM_BLOCKSIZE      0x10000

//the Highest 4K byte as program Information sector
#define W25Q64_PROGRAM_INFORMATION_BLOCKSIZE  W25Q64_SECTOR_SIZE
#define W25Q64_PROGRAM_INFORMATION_START_ADDR (W25Q64_CHIP_SIZE - W25Q64_SECTOR_SIZE) 

//the Highest 4K-8K byte as bootversion Information sector
#define W25Q64_BOOTVERSION_INFORMATION_START_ADDR (W25Q64_CHIP_SIZE -W25Q64_SECTOR_SIZE-W25Q64_SECTOR_SIZE )

#define ARM_FALSH_SIZE  0x20000

//the highest 64K byte as the first programe sector
#define W25Q64_PROGRAM_SECTOR1_BLOCKSIZE      ARM_FALSH_SIZE
#define W25Q64_PROGRAM_SECTOR1_START_ADDR     (W25Q64_CHIP_SIZE - W25Q64_PROGRAM_SECTOR1_BLOCKSIZE) 

//the next 64K byte as the sector programe sector
#define W25Q64_PROGRAM_SECTOR2_BLOCKSIZE      ARM_FALSH_SIZE
#define W25Q64_PROGRAM_SECTOR2_START_ADDR     (W25Q64_PROGRAM_SECTOR1_START_ADDR - W25Q64_PROGRAM_SECTOR2_BLOCKSIZE)    

#define BLOCK_WRITE_BITPOS 0
#define BLOCK_READ_BITPOS  1
#define BLOCK_WRONG_BITPOS 2

#define Set_FlashFlag(flag,POS)  (flag &= ~(1<<POS))
#define Clear_FlashFlag(flag,POS)    (flag |= (1<<POS))
#define Get_FlashFlag(flag,POS)  (flag & (1<<POS))

#define FLAG_TRUE  0
#define FLAG_FALSE 1

#define W25Q64_FindSectorAddr(addr) ((U32)addr & 0x000ff000)

typedef struct {
    uint32 head;
    uint32 tail;
    bool empty;
    bool full;
} flashdata_frame;

typedef struct{
    U8 buff[W25Q64_STATE_BLOCKSIZE];
    U32 addr;
}flashstate_msg;

void SPI_Flash_Init(void);
u8   SPI_Flash_ReadSR(void);
void SPI_FLASH_Write_SR(u8 sr);
void SPI_FLASH_Write_Enable(void);
void SPI_FLASH_Write_Disable(void);
void SPI_Flash_Write_NoCheck(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_Flash_Read(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
void SPI_Flash_Write(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_Flash_Erase_Chip(void);
void SPI_Flash_Erase_Sector(u32 Dst_Addr);
void SPI_Flash_Wait_Busy(void);
void SPI_Flash_PowerDown(void);
void SPI_Flash_WAKEUP(void);

void W25Q64_Init(void);
//U32 W25Q64_GetAddr(U8 *buff);
//void W25Q64_SetAddr(U8 *buff, U32 addr);
void W25Q64_GetFlags(flashdata_frame *frame, U8 *flags);
void W25Q64_GetFlashStateMsg(flashdata_frame *frame, FlashState_Data_Struct *stateframe);
void W25Q64_SetFlashStateMsg(flashdata_frame *frame, FlashState_Data_Struct *stateframe);



U8 W25Q64_IsSameSector(U32 addr_1, U32 addr_2);
U8 W25Q64_IsByteUsed(U32 addr);
U8 W25Q64_CheackSectorsByBlock(U32 start_addr, U32 end_addr, U16 blocksize);
U8 W25Q64_CheackThisSectorByBlock(U32 addr, U16 blocksize);
void W25Q64_EraseUsedStateSector(U32 addr);
U32 W25Q64_FindNextAddr(U32 this_addr, U32 start_addr , U32 end_addr, U16 blocksize);
U32 W25Q64_FindPreviousAddr(U32 this_addr, U32 start_addr , U32 end_addr, U16 blocksize);
U8 W25Q64_FindSelectedDataAddr(U32 num);
U8 W25Q64_GetStateBlcok(U32 *addr, U8 *buff);
U8 W25Q64_WriteStateBlcok(U32 *addr, U8 *buff);

void W25Q64_SetDataPtrMSG(s_FlashDataPtrStruct *dataptrframe, flashdata_frame *dataframe);
U8 W25Q64_GetDataPtrMSG(s_FlashDataPtrStruct *dataptrframe, flashdata_frame *dataframe);

void W25Q64_AddCRCToBufferEnd(U8 *buff, U16 len);
U8 W25Q64_CheackBufferCRC(U8 *buff, U16 len);

U8 W25Q64_WriteOneDataBlock(flashdata_frame *dataframe, U8 *buff_ptr);
#ifdef VERSION_BMS
U8 W25Q64_ReadOneDataBlock(flashdata_frame *dataframe, U8 *buff_ptr);
void FlashDataBlockSave(void);
#endif

U8 W25Q64_GetStateMSG(FlashState_Data_Struct *stateframe);
void W25Q64_SetStateMSG(FlashState_Data_Struct *stateframe);

U8 W25Q64_GetStaticMSG(FlashStatic_Data_Union *staicdata);
void W25Q64_SetStaticMSG(FlashStatic_Data_Union *staicdata);

void W25Q64_EraseProgramArea(U32 StartAddr);

U8 W25Q64_DataBlockPop(flashdata_frame *dataframe);
U8 W25Q64_DataBlockTop(flashdata_frame *dataframe, U8 *buff_ptr);

void W25Q64_FlashStaticBlockCellHVoltSaveProc(void);
void W25Q64_EnableStaticErrBlockSave(void);
void W25Q64_SaveStaticErrBlock(FlashStatic_Data_Union *staicdata);
void W25Q64_WdtResetRecord(void);
void W25Q64_UnixtimeSyncRecord(void);

//extern flashstate_msg g_flashstate_msg;
extern flashdata_frame g_flashdata_frame;

#endif

#include "cc1120.h"
//#include "uartdata_service.h"
#include "gpio.h"
#include "stdlib.h"
#include "rf_service.h"
#include "public.h"
/******************************************************************************
* LOCAL VARIABLES
*/
U8 CC1120_rxBuffer[PACKET_RX_MAX_SIZE] = {0};
static U32 RfSendTimeCount = 0xffffffff;
static U8 CC1120_WorkState = CC1120_WORK_IDLE;
static U8 RF_SendState = RS_IDLE;
static U32 CC1120_TxCount = 0;
static U32 CC1120_RxCount = 0;
bool g_CC1120RxUpdate = FALSE;
extern U8 RF_SendState;
extern U8 RfWorkState;
/* basic set of access functions */
//void trxRfSpiInterfaceInit(uint8 clockDivider);

static void trxReadWriteBurstSingle(uint8 addr, uint8 *pData, uint16 len) ;
static void calibrateRCOsc (void);
rfStatus_t trx8BitRegAccess(uint8 accessType, uint8 addrByte, uint8 *pData, uint16 len);
//rfStatus_t trxSpiCmdStrobe(uint8 cmd);

/* CC112X specific prototype function */
rfStatus_t trx16BitRegAccess(uint8 accessType, uint8 extAddr, uint8 regAddr, uint8 *pData, uint8 len);
rfStatus_t cc112xSpiReadReg(uint16 addr, uint8 *data, uint8 len);
rfStatus_t cc112xGetTxStatus(void);
rfStatus_t cc112xGetRxStatus(void);
rfStatus_t cc112xSpiWriteReg(uint16 addr, uint8 *data, uint8 len);
rfStatus_t cc112xSpiWriteTxFifo(uint8 *pWriteData, uint8 len);
rfStatus_t cc112xSpiReadRxFifo(uint8 *pReadData, uint8 len);
U8 CheackMarcStatusIDLE(U8 MarcStatusReg);

/*******************************************************************************
 * @fn          trx8BitRegAccess
 *
 * @brief       This function performs a read or write from/to a 8bit register
 *              address space. The function handles burst and single read/write
 *              as specfied in addrByte. Function assumes that chip is ready.
 *
 * input parameters
 *
 * @param       accessType - Specifies if this is a read or write and if it's
 *                           a single or burst access. Bitmask made up of
 *                           RADIO_BURST_ACCESS/RADIO_SINGLE_ACCESS/
 *                           RADIO_WRITE_ACCESS/RADIO_READ_ACCESS.
 * @param       addrByte - address byte of register.
 * @param       pData    - data array
 * @param       len      - Length of array to be read(TX)/written(RX)
 *
 * output parameters
 *
 * @return      chip status
 */
rfStatus_t trx8BitRegAccess(uint8 accessType, uint8 addrByte, uint8 *pData, uint16 len)
{
    uint8 readValue;
    U16 RetryCount = 0xffff;
    /* Pull CS_N low and wait for SO to go low before communication starts */
    TRXEM_SPI_BEGIN();
    while((READ_INPUT(PTD, PTD2)) && (RetryCount--));
    readValue = Spi1_RWByte(accessType | addrByte);
    trxReadWriteBurstSingle(accessType | addrByte, pData, len);
    TRXEM_SPI_END();
    /* return the status byte value */
    return(readValue);
}

/******************************************************************************
 * @fn          trx16BitRegAccess
 *
 * @brief       This function performs a read or write in the extended adress
 *              space of CC112X.
 *
 * input parameters
 *
 * @param       accessType - Specifies if this is a read or write and if it's
 *                           a single or burst access. Bitmask made up of
 *                           RADIO_BURST_ACCESS/RADIO_SINGLE_ACCESS/
 *                           RADIO_WRITE_ACCESS/RADIO_READ_ACCESS.
 * @param       extAddr - Extended register space address = 0x2F.
 * @param       regAddr - Register address in the extended address space.
 * @param       *pData  - Pointer to data array for communication
 * @param       len     - Length of bytes to be read/written from/to radio
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
rfStatus_t trx16BitRegAccess(uint8 accessType, uint8 extAddr, uint8 regAddr, uint8 *pData, uint8 len)
{
    uint8 readValue;
    U16 RetryCount = 0xffff;
    TRXEM_SPI_BEGIN();
    while((READ_INPUT(PTD, PTD2)) && (RetryCount--));
    /* send extended address byte with access type bits set */
//  /* Storing chip status */
    readValue = Spi1_RWByte(accessType | extAddr);
    Spi1_RWByte(regAddr);
    /* Communicate len number of bytes */
    trxReadWriteBurstSingle(accessType | extAddr, pData, len);
    TRXEM_SPI_END();
    /* return the status byte value */
    return(readValue);
}

/*******************************************************************************
 * @fn          trxSpiCmdStrobe
 *
 * @brief       Send command strobe to the radio. Returns status byte read
 *              during transfer of command strobe. Validation of provided
 *              is not done. Function assumes chip is ready.
 *
 * input parameters
 *
 * @param       cmd - command strobe
 *
 * output parameters
 *
 * @return      status byte
 */
rfStatus_t trxSpiCmdStrobe(uint8 cmd)
{
    uint8 rc;
    U16 RetryCount = 0xffff;
    TRXEM_SPI_BEGIN();
    while((READ_INPUT(PTD, PTD2)) && (RetryCount--));
    rc = Spi1_RWByte(cmd);
    TRXEM_SPI_END();
    return(rc);
}

/*******************************************************************************
 * @fn          trxReadWriteBurstSingle
 *
 * @brief       When the address byte is sent to the SPI slave, the next byte
 *              communicated is the data to be written or read. The address
 *              byte that holds information about read/write -and single/
 *              burst-access is provided to this function.
 *
 *              Depending on these two bits this function will write len bytes to
 *              the radio in burst mode or read len bytes from the radio in burst
 *              mode if the burst bit is set. If the burst bit is not set, only
 *              one data byte is communicated.
 *
 *              NOTE: This function is used in the following way:
 *
 *              TRXEM_SPI_BEGIN();
 *              while(TRXEM_PORT_IN & TRXEM_SPI_MISO_PIN);
 *              ...[Depending on type of register access]
 *              trxReadWriteBurstSingle(uint8 addr,uint8 *pData,uint16 len);
 *              TRXEM_SPI_END();
 *
 * input parameters
 *
 * @param       none
 *
 * output parameters
 *
 * @return      void
 */
static void trxReadWriteBurstSingle(uint8 addr, uint8 *pData, uint16 len)
{
    uint16 i;
    /* Communicate len number of bytes: if RX - the procedure sends 0x00 to push bytes from slave*/
    if(addr & RADIO_READ_ACCESS) {
        if(addr & RADIO_BURST_ACCESS) {
            for (i = 0; i < len; i++) {
                *pData++ =  Spi1_RWByte(0);
            }
        } else {
            *pData =  Spi1_RWByte(0);
        }
    } else {
        if(addr & RADIO_BURST_ACCESS) {
            /* Communicate len number of bytes: if TX - the procedure doesn't overwrite pData */
            for (i = 0; i < len; i++) {
                Spi1_RWByte(*pData++);
            }
        } else {
            Spi1_RWByte(*pData);
        }
    }
    return;
}
/******************************************************************************
 * FUNCTIONS
 */

/******************************************************************************
 * @fn          cc112xSpiReadReg
 *
 * @brief       Read value(s) from config/status/extended radio register(s).
 *              If len  = 1: Reads a single register
 *              if len != 1: Reads len register values in burst mode
 *
 * input parameters
 *
 * @param       addr   - address of first register to read
 * @param       *pData - pointer to data array where read bytes are saved
 * @param       len   - number of bytes to read
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
rfStatus_t cc112xSpiReadReg(uint16 addr, uint8 *pData, uint8 len)
{
    uint8 tempExt  = (uint8)(addr >> 8);
    uint8 tempAddr = (uint8)(addr & 0x00FF);
    uint8 rc;

    /* Checking if this is a FIFO access -> returns chip not ready  */
    if((CC1120_SINGLE_TXFIFO <= tempAddr) && (tempExt == 0)) return STATUS_CHIP_RDYn_BM;

    /* Decide what register space is accessed */
    if(!tempExt) {
        rc = trx8BitRegAccess((RADIO_BURST_ACCESS | RADIO_READ_ACCESS), tempAddr, pData, len);
    } else if (tempExt == 0x2F) {
        rc = trx16BitRegAccess((RADIO_BURST_ACCESS | RADIO_READ_ACCESS), tempExt, tempAddr, pData, len);
    }
    return (rc);
}

/******************************************************************************
 * @fn          cc112xSpiWriteReg
 *
 * @brief       Write value(s) to config/status/extended radio register(s).
 *              If len  = 1: Writes a single register
 *              if len  > 1: Writes len register values in burst mode
 *
 * input parameters
 *
 * @param       addr   - address of first register to write
 * @param       *pData - pointer to data array that holds bytes to be written
 * @param       len    - number of bytes to write
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
rfStatus_t cc112xSpiWriteReg(uint16 addr, uint8 *pData, uint8 len)
{
    uint8 tempExt  = (uint8)(addr >> 8);
    uint8 tempAddr = (uint8)(addr & 0x00FF);
    uint8 rc;

    /* Checking if this is a FIFO access - returns chip not ready */
    if((CC1120_SINGLE_TXFIFO <= tempAddr) && (tempExt == 0)) return STATUS_CHIP_RDYn_BM;

    /* Decide what register space is accessed */
    if(!tempExt) {
        rc = trx8BitRegAccess((RADIO_BURST_ACCESS | RADIO_WRITE_ACCESS), tempAddr, pData, len);
    } else if (tempExt == 0x2F) {
        rc = trx16BitRegAccess((RADIO_BURST_ACCESS | RADIO_WRITE_ACCESS), tempExt, tempAddr, pData, len);
    }
    return (rc);
}

/*******************************************************************************
 * @fn          cc112xSpiWriteTxFifo
 *
 * @brief       Write pData to radio transmit FIFO.
 *
 * input parameters
 *
 * @param       *pData - pointer to data array that is written to TX FIFO
 * @param       len    - Length of data array to be written
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
rfStatus_t cc112xSpiWriteTxFifo(uint8 *pData, uint8 len)
{
    uint8 rc;
    rc = trx8BitRegAccess(0x00, CC1120_BURST_TXFIFO, pData, len);
    return (rc);
}

/*******************************************************************************
 * @fn          cc112xSpiReadRxFifo
 *
 * @brief       Reads RX FIFO values to pData array
 *
 * input parameters
 *
 * @param       *pData - pointer to data array where RX FIFO bytes are saved
 * @param       len    - number of bytes to read from the RX FIFO
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
rfStatus_t cc112xSpiReadRxFifo(uint8 * pData, uint8 len)
{
    uint8 rc;
    rc = trx8BitRegAccess(0x00, CC1120_BURST_RXFIFO, pData, len);
    return (rc);
}

/******************************************************************************
 * @fn      cc112xGetTxStatus(void)
 *
 * @brief   This function transmits a No Operation Strobe (SNOP) to get the
 *          status of the radio and the number of free bytes in the TX FIFO.
 *
 *          Status byte:
 *
 *          ---------------------------------------------------------------------------
 *          |          |            |                                                 |
 *          | CHIP_RDY | STATE[2:0] | FIFO_BYTES_AVAILABLE (free bytes in the TX FIFO |
 *          |          |            |                                                 |
 *          ---------------------------------------------------------------------------
 *
 *
 * input parameters
 *
 * @param   none
 *
 * output parameters
 *
 * @return  rfStatus_t
 *
 */
rfStatus_t cc112xGetTxStatus(void)
{
    return(trxSpiCmdStrobe(CC1120_SNOP));
}

/******************************************************************************
 *
 *  @fn       cc112xGetRxStatus(void)
 *
 *  @brief
 *            This function transmits a No Operation Strobe (SNOP) with the
 *            read bit set to get the status of the radio and the number of
 *            available bytes in the RXFIFO.
 *
 *            Status byte:
 *
 *            --------------------------------------------------------------------------------
 *            |          |            |                                                      |
 *            | CHIP_RDY | STATE[2:0] | FIFO_BYTES_AVAILABLE (available bytes in the RX FIFO |
 *            |          |            |                                                      |
 *            --------------------------------------------------------------------------------
 *
 *
 * input parameters
 *
 * @param     none
 *
 * output parameters
 *
 * @return    rfStatus_t
 *
 */
rfStatus_t cc112xGetRxStatus(void)
{
    return(trxSpiCmdStrobe(CC1120_SNOP | RADIO_READ_ACCESS));
}

/******************************************************************************
 * @fn          CheackMarcStatusIDLE
*/
U8 CheackMarcStatusIDLE(U8 MarcStatusReg)
{
    U8 MarcStatus = (MarcStatusReg & 0x1F);
    if(MarcStatus == MARC_IDLE) {
        return TRUE;
    }
    //clear error state
    switch(MarcStatus) {
        case MARC_TX_FIFO_ERROR:
            trxSpiCmdStrobe(CC1120_SFTX);
            break;

        case MARC_RX_FIFO_ERROR:
            trxSpiCmdStrobe(CC1120_SFRX);
            trxSpiCmdStrobe(CC1120_SRX);
            break;

        default:
            break;
    }
    return FALSE;
}

/******************************************************************************
 * @fn          CC1120_GetStatusByte
*/
U8 GetStatusByte(void)
{
    return ((trxSpiCmdStrobe(CC1120_SNOP) & 0x70));
}

/******************************************************************************
 * @fn          CC1120_GetStatusByte
*/
void ChangeWorkMode(U8 Mode)
{
    switch(Mode) {
        case CC1120_WORK_SENDING:
            trxSpiCmdStrobe(CC1120_STX);
            CC1120_WorkState = CC1120_WORK_SENDING;
            break;

        case CC1120_WORK_RECEIVING:
            trxSpiCmdStrobe(CC1120_SRX);
            CC1120_WorkState = CC1120_WORK_RECEIVING;
            break;

        case CC1120_WORK_SNIFF:
            trxSpiCmdStrobe(CC1120_SWOR);
            CC1120_WorkState = CC1120_WORK_RECEIVING;
            break;

        case CC1120_WORK_IDLE:
            trxSpiCmdStrobe(CC1120_SIDLE);
            CC1120_WorkState = CC1120_WORK_IDLE;
            break;

        case CC1120_WORK_SLEEP:
            trxSpiCmdStrobe(CC1120_SXOFF);
            trxSpiCmdStrobe(CC1120_SPWD);
            CC1120_WorkState = CC1120_WORK_SLEEP; 
            break;
                       
        default:
            break;
    }
}

/******************************************************************************
 * @fn          CheackStatusByte
*/
void CheackStatusByte(U8 StateByte)
{
    switch (StateByte) {
        case CC1120_STATE_TXFIFO_ERROR:
            trxSpiCmdStrobe(CC1120_SFTX);
            ChangeWorkMode(CC1120_WORK_IDLE);
            break;
        case CC1120_STATE_RXFIFO_ERROR:
            trxSpiCmdStrobe(CC1120_SFRX);
            ChangeWorkMode(CC1120_WORK_IDLE);
            break;
        default:
            ChangeWorkMode(CC1120_WORK_IDLE);
            break;
    }
}

/******************************************************************************
 * @fn          CC1120_SetTxPower
 * RaPowerRampVal: 3~63
*/
void CC1120_SetTxPower(U8 RaPowerRampVal)
{
    U8 RegVal = (RaPowerRampVal | 0x40);
    cc112xSpiWriteReg(CC1120_PA_CFG2, &RegVal, 1);
}

/******************************************************************************
 * @fn          CC1120_SetCarrierFreq
*/
void CC1120_SetCarrierFreq(U8 FreqVal0, U8 FreqVal1, U8 FreqVal2)
{
    cc112xSpiWriteReg(CC1120_FREQ0, &FreqVal0, 1);
    cc112xSpiWriteReg(CC1120_FREQ1, &FreqVal1, 1);
    cc112xSpiWriteReg(CC1120_FREQ2, &FreqVal2, 1);
}

/******************************************************************************
 * @fn          CC1120_SetCarrierFreqU32
*/
void CC1120_SetCarrierFreqU32(U32 Freq)
{
    U8 Freq0 = 0, Freq1 = 0, Freq2 = 0;
    U8 *ptr = (U8 *)&Freq;
    Freq0 = *ptr++;
    Freq1 = *ptr++;
    Freq2 = *ptr;
    CC1120_SetCarrierFreq(Freq0,Freq1,Freq2);
}

/******************************************************************************
 * @fn  CC1120_SetPreamWord
 bit: 5:2 num  1:0 type 
 Type:  00  10101010 (0xAA)             //up link
        01  01010101 (0x55)
        10  00110011 (0x33)             //down link
        11  11001100 (0xCC)
*/
void CC1120_SetPreamWord(U8 val)
{
    cc112xSpiWriteReg(CC1120_PREAMBLE_CFG1, &val, 1);
}

/******************************************************************************
 * @fn          CC1120_SetCarrierFreq
*/
void CC1120_SetSyncWord(U8 Sync3, U8 Sync2, U8 Sync1, U8 Sync0)
{
    cc112xSpiWriteReg(CC1120_SYNC3, &Sync3, 1);
    cc112xSpiWriteReg(CC1120_SYNC2, &Sync2, 1);
    cc112xSpiWriteReg(CC1120_SYNC1, &Sync1, 1);
    cc112xSpiWriteReg(CC1120_SYNC0, &Sync0, 1);
}

/******************************************************************************
 * @fn          CC1120_SetDevAddr
*/
void CC1120_SetDevAddr(U8 Addr)
{
    cc112xSpiWriteReg(CC1120_DEV_ADDR, &Addr, 1);
}


/******************************************************************************
 * @fn          CC1120_init
*/
void CC1120_init(void)
{
    OUTPUT_SET(PTE,PTE0);////K_3V3_wireless
    OUTPUT_SET(PTE, PTE3); //Reset High
    registerConfig();
    manualCalibration();
    calibrateRCOsc();
    g_stateblock_data.stata_union.state_data.ComAddr = 0xff;
}

/******************************************************************************
 * @fn          CC1120_SendPacket
 *
 * @brief       Continuously sends packets on button push until button is pushed
 *              again. After the radio has gone into TX the function waits for
 *              interrupt that packet has been sent. Updates packet counter and
 *              display for each packet sent.
 *
 * @param       none
 *
 * @return      none
 */

void CC1120_SendPacket(U8 *Packet , U8 len)
{
    // Write packet to tx fifo
    cc112xSpiWriteTxFifo(Packet, len);

    // Strobe TX to send packet
    ChangeWorkMode(CC1120_WORK_SENDING);
    CC1120_TxCount++;
}

/******************************************************************************
 * @fn          CC1120_SendCheackPacket
*/
U8 CC1120_SendCheackPacket(U8 *Packet , U8 len)
{
    if(len > (PACKET_TX_MAX_SIZE + 1))
        return ERR_RFSEND_SIZE; //beyond max size

    trxSpiCmdStrobe(CC1120_SFTX);
    CC1120_SendPacket(Packet, len);
    return ERR_RFSEND_OK;
}

/******************************************************************************
 * @fn          RfSendAndWaitAckMs
 *              Send packet and make sure packet send success
*/
U8 CC1120_SendWaitSuccess(U8 *Packet , U8 len, U16 WaitTime)
{
    U8 ErrCode;
    if(RF_SendState == RS_SENDING) return FALSE;
    ErrCode = CC1120_SendCheackPacket(Packet, len);
    if(ErrCode == ERR_RFSEND_OK) {
        RfSendTimeCount = WaitTime;
        RF_SendState = RS_SENDING;
        return TRUE;
    } else {
        RF_SendState = RS_SEND_FAIL;
        //CHIP ERROR
        return FALSE;
    }
}

U8 CC1120_WriteTxFifo(U8 *Packet , U8 len)
{
    if(len > (PACKET_TX_MAX_SIZE + 1))return FALSE; //beyond max size
    trxSpiCmdStrobe(CC1120_SFTX);
    cc112xSpiWriteTxFifo(Packet, len);
    return TRUE;
}

void CC1120_SendTxFifo(U8 Timeout)
{
    ChangeWorkMode(CC1120_WORK_SENDING);
    CC1120_TxCount++;
    RfSendTimeCount = Timeout;
    RF_SendState = RS_SENDING;
}
/******************************************************************************
* @fn          CC1120_SendTimeoutProc
*              Put it in Timer ISR
*/
void CC1120_SendTimeoutProc(void (*handler)(void))
{
    if(RF_SendState == RS_SENDING) {
        RfSendTimeCount -= 10;
        if(RfSendTimeCount == 0) {
            RF_SendState = RS_SEND_FAIL;
            (*handler)();
        }
    }
}

/******************************************************************************
* @fn          CC1120_SendSuccessHandler
*/
void CC1120_SendSuccessHandler(void (*handler1)(void), void (*handler2)(void))
{
    if(RF_SendState == RS_SEND_SUCCESS) {
        RF_SendState = RS_IDLE;
//        (*handler1)();
        (*handler2)();
    }
}

/******************************************************************************
 * @fn          CC1120_RecivePacket
 *
 * @brief       puts radio in RX and waits for packets. Function assumes
 *              that status bytes are appended in the RX_FIFO
 *              Update packet counter and display for each packet received.
 *
 * @param       none
 *
 * @return      none
 */
void CC1120_RecivePacket(void)
{
    uint8 rxBytes;

    /* Due a chip bug, the RXBYTES register must read the same value twice
    * in a row to guarantee an accurate value.
    */
    {
        U8 rxBytesVerify;
        cc112xSpiReadReg(CC1120_NUM_RXBYTES, &rxBytesVerify, 1);
        do {
            rxBytes = rxBytesVerify;
            cc112xSpiReadReg(CC1120_NUM_RXBYTES, &rxBytesVerify, 1);
        } while (rxBytes != rxBytesVerify);
    }

    // LED2_TOGGLE;
    U8 StatusByte = GetStatusByte();
    CheackStatusByte(StatusByte);
    
    // Check that we have bytes in fifo
    if((rxBytes > 0) && (rxBytes <= PACKET_RX_MAX_SIZE)) {
        //clear buffer
        memset(CC1120_rxBuffer, 0, PACKET_RX_MAX_SIZE);
        
        cc112xSpiReadRxFifo(CC1120_rxBuffer, rxBytes);
        CC1120_RxCount++;
        g_CC1120RxUpdate = TRUE;

    } else {
        // Set radio back in RX
        trxSpiCmdStrobe(CC1120_SFRX);
        if(RfWorkState == RF_SLEEP){
            ChangeWorkMode(CC1120_WORK_SNIFF);
        }else{
            ChangeWorkMode(CC1120_WORK_RECEIVING);
        }
    }
}

/*******************************************************************************
* @fn          registerConfig
*
* @brief       Write register settings as given by SmartRF Studio found in
*              cc112x_easy_link_reg_config.h
*
* @param       none
*
* @return      none
*/
void registerConfig(void)
{
    uint8 writeByte;

    // Reset radio
    trxSpiCmdStrobe(CC1120_SRES);

    // Write registers to radio
    for(uint16 i = 0; i < (sizeof  preferredSettings / sizeof(registerSetting_t)); i++) {
        writeByte =  preferredSettings[i].data;
        cc112xSpiWriteReg( preferredSettings[i].addr, &writeByte, 1);
    }
}

/******************************************************************************
 * @fn          CC1120_ISRHandler
*/
void CC1120_ISRHandler()
{
    switch(CC1120_WorkState) {
        case CC1120_WORK_SENDING:
            CC1120_WorkState = CC1120_WORK_IDLE;
            if(RF_SendState == RS_SENDING) {
                RF_SendState = RS_SEND_SUCCESS;
            }
            break;
        case CC1120_WORK_RECEIVING:
            CC1120_WorkState = CC1120_WORK_IDLE;
            // CC1120_RecivePacket(PacketParse);
            Set_IntFlag(CC1120_RX_INT_POS);
            break;
        default:
            break;
    }
}
/******************************************************************************
 * @fn          manualCalibration
 *
 * @brief       calibrates radio according to CC112x errata
 *
 * @param       none
 *
 * @return      none
 */
#define VCDAC_START_OFFSET 2
#define FS_VCO2_INDEX 0
#define FS_VCO4_INDEX 1
#define FS_CHP_INDEX 2
void manualCalibration(void)
{

    uint8 original_fs_cal2;
    uint8 calResults_for_vcdac_start_high[3];
    uint8 calResults_for_vcdac_start_mid[3];
    uint8 marcstate;
    uint8 writeByte;
    U16 RetryCount = 0xffff;

    // 1) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    writeByte = 0x00;
    cc112xSpiWriteReg(CC1120_FS_VCO2, &writeByte, 1);

    // 2) Start with high VCDAC (original VCDAC_START + 2):
    cc112xSpiReadReg(CC1120_FS_CAL2, &original_fs_cal2, 1);
    writeByte = original_fs_cal2 + VCDAC_START_OFFSET;
    cc112xSpiWriteReg(CC1120_FS_CAL2, &writeByte, 1);

    // 3) Calibrate and wait for calibration to be done (radio back in IDLE state)
    trxSpiCmdStrobe(CC1120_SCAL);

    RetryCount = 0xffff;
    do {
        cc112xSpiReadReg(CC1120_MARCSTATE, &marcstate, 1);
    } while ((marcstate != 0x41)&&(RetryCount--));

    // 4) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with high VCDAC_START value
    cc112xSpiReadReg(CC1120_FS_VCO2, &calResults_for_vcdac_start_high[FS_VCO2_INDEX], 1);
    cc112xSpiReadReg(CC1120_FS_VCO4, &calResults_for_vcdac_start_high[FS_VCO4_INDEX], 1);
    cc112xSpiReadReg(CC1120_FS_CHP, &calResults_for_vcdac_start_high[FS_CHP_INDEX], 1);

    // 5) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    writeByte = 0x00;
    cc112xSpiWriteReg(CC1120_FS_VCO2, &writeByte, 1);

    // 6) Continue with mid VCDAC (original VCDAC_START):
    writeByte = original_fs_cal2;
    cc112xSpiWriteReg(CC1120_FS_CAL2, &writeByte, 1);

    // 7) Calibrate and wait for calibration to be done (radio back in IDLE state)
    trxSpiCmdStrobe(CC1120_SCAL);

    RetryCount = 0xffff;
    do {
        cc112xSpiReadReg(CC1120_MARCSTATE, &marcstate, 1);
    } while ((marcstate != 0x41)&&(RetryCount--));

    // 8) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with mid VCDAC_START value
    cc112xSpiReadReg(CC1120_FS_VCO2, &calResults_for_vcdac_start_mid[FS_VCO2_INDEX], 1);
    cc112xSpiReadReg(CC1120_FS_VCO4, &calResults_for_vcdac_start_mid[FS_VCO4_INDEX], 1);
    cc112xSpiReadReg(CC1120_FS_CHP, &calResults_for_vcdac_start_mid[FS_CHP_INDEX], 1);

    // 9) Write back highest FS_VCO2 and corresponding FS_VCO and FS_CHP result
    if (calResults_for_vcdac_start_high[FS_VCO2_INDEX] > calResults_for_vcdac_start_mid[FS_VCO2_INDEX]) {
        writeByte = calResults_for_vcdac_start_high[FS_VCO2_INDEX];
        cc112xSpiWriteReg(CC1120_FS_VCO2, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_high[FS_VCO4_INDEX];
        cc112xSpiWriteReg(CC1120_FS_VCO4, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_high[FS_CHP_INDEX];
        cc112xSpiWriteReg(CC1120_FS_CHP, &writeByte, 1);
    } else {
        writeByte = calResults_for_vcdac_start_mid[FS_VCO2_INDEX];
        cc112xSpiWriteReg(CC1120_FS_VCO2, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_mid[FS_VCO4_INDEX];
        cc112xSpiWriteReg(CC1120_FS_VCO4, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_mid[FS_CHP_INDEX];
        cc112xSpiWriteReg(CC1120_FS_CHP, &writeByte, 1);
    }

}

/**********************************************************************************
* @fn calibrateRcOsc
*
* @brief Calibrates the RC oscillator used for the eWOR timer. When this
* function is called, WOR_CFG0.RC_PD must be 0
*
* @param none
*
* @return none
*/
static void calibrateRCOsc (void)
{
    uint8 temp;
// Read current register value
    cc112xSpiReadReg(CC1120_WOR_CFG0, &temp, 1);
// Mask register bit fields and write new values
    temp = (temp & 0xF9) | (0x02 << 1);
// Write new register value
    cc112xSpiWriteReg(CC1120_WOR_CFG0, &temp, 1);
// Strobe IDLE to calibrate the RCOSC
    trxSpiCmdStrobe(CC1120_SIDLE);
// Disable RC calibration
    temp = (temp & 0xF9) | (0x00 << 1);
    cc112xSpiWriteReg(CC1120_WOR_CFG0, &temp, 1);
}


#include "isl94212.h"
#include "isl94212_CRC.h"
#include "drivers.h"
#include "stdio.h"
#include "public.h"
#include "rda3216.h"

U8 g_Serial_RegNO = 0;
U8 g_Chain_DevNO = 0;


/*******************************************************************
 *******************************************************************/
Bit8Field CRC_Calc_0, CRC_Calc_1;
void CalcCRC4(U8 *pData, U8 dataLen)
{
	unsigned char i, j, k;
	unsigned char data0;
	unsigned char result;

	CRC_Calc_0.Byte = 0;
	
	for (i=0; i<dataLen; i++)
	{
		data0 = pData[i];		
		(i == (dataLen - 1)) ? (k=3) : (k=8);
		for (j=0; j<k; j++)
		{
			CRCcarry = ((data0 & 0x80) > 0);
			data0 <<= 1;
							
			CRCff0 = CRCcarry ^ CRCbit3;
			CRCff1 = CRCbit0 ^ CRCbit3;
			CRCff2 = CRCbit1;
			CRCff3 = CRCbit2;
            CRC_Calc_0.Byte >>= 4;
		}
	}
		
	result = CRC_Calc_0.Byte & 0x0F;
	
	pData[3] &= 0xE0;
	pData[3] |= (result<<1);		/* ?CRC????pData[dataLen - 1]????? */
}


void SendWriteCommand(U8 devaddr, U8 regaddr, U32 cmd, U8 seekall_flg)
{
	U8 devaddr_t = 0;
	WriteCommand_buf WrtCmd_buf = {0};
	
	devaddr_t |= ((devaddr & (1<<0)) << 3)
				|((devaddr & (1<<1)) << 1)
				|((devaddr & (1<<2)) >> 1) 
				|((devaddr & (1<<3)) >> 3);
	
	WrtCmd_buf.WrtCmd.devaddr = devaddr_t & 0x0F;
//	WrtCmd_buf.WrtCmd.devaddr = devaddr & 0x0F;
	
	WrtCmd_buf.WrtCmd.regaddr_H = (regaddr & 0x1e) >> 1;
	WrtCmd_buf.WrtCmd.regaddr_L = regaddr & 0x01;
	
	WrtCmd_buf.WrtCmd.data_H = (U8)((cmd & 0x0001FC00)>> 10);
	WrtCmd_buf.WrtCmd.data_M = (U8)((cmd & 0x000003FC)>> 2);
	WrtCmd_buf.WrtCmd.data_L = (U8)((cmd & 0x00000003)>> 0);
	
	WrtCmd_buf.WrtCmd.seek_devaddr = seekall_flg;  //one or a serial registers to be writed
	CalcCRC4(WrtCmd_buf.data, 4);
	WrtCmd_buf.WrtCmd.crc = (WrtCmd_buf.data[3] &0x1E) >> 1;
	WrtCmd_buf.WrtCmd.rsvd = 0;

	Spi0_WrBytes((unsigned char*)(&WrtCmd_buf.data[0]),WRTCMD_LEN);
	
	if (REGCONFIG_ADDR == regaddr)
	{
		U8 rd_conv_mode = (cmd>>12)&0x03;
		switch (rd_conv_mode){
		case 0:
			g_Serial_RegNO = 16;
			break;
		case 1:
			g_Serial_RegNO = 8;
			break;
		case 2:
			g_Serial_RegNO = 7;
			break;
		case 3:
			g_Serial_RegNO = 1;
			break;
		default:
			g_Serial_RegNO = 1;
			break;
		}
		
	}
}


void SendReadCommand(U8 devaddr, U8 regAddr, U8 seek_all_flg)
{
	U8 regAddr_t = 0;
	regAddr_t = regAddr << 3;
	
	SendWriteCommand (devaddr, REGREAD_ADDR, regAddr_t, seek_all_flg);
	SendWriteCommand (0xFF, 0, 0, 0);//send special read command
}

void ReadReg(U8 devAddr,U8 regAddr,U32 *data,U8 seek_all_flg)
{
	U32 ReadDataBuff[MAX_SERIALREGNO] = {0}; // MAX_SERIALREGNO = 48
//	U32 ReadDataBuff[CHAIN_DEV_NUM * 11];
	U8 RegNums = 0;
	
	RDA3216_ConvsStart;
	delay_ms(100);
	SendReadCommand(devAddr,regAddr,seek_all_flg);
	
	if(seek_all_flg == 0){
		RegNums = 1;
	}
	else{
		RegNums = CHAIN_DEV_NUM;
	}
	
	if( 0 == regAddr){	//read a serial registers' content of a device
		Spi0_RdBytes(ReadDataBuff, RegNums);
		for (U8 k = 0; k < RegNums; k++){
			data[k] = (ReadDataBuff[k]>>6) & 0x1FFFF;
		}
	}
	else{
		Spi0_RdBytes(ReadDataBuff,RDCMD_LEN*g_Serial_RegNO);
		data[0] = (ReadDataBuff[0]>>6) & 0x1FFFF;
	}
	
	RDA3216_ConvsDone;
}

void ReadADCVal_SingleDev(U8 devaddr,U32* data){
	ReadReg(devaddr,0,data,0);
}

void ReadADCVal_SerialDevs(U32* data){
	ReadReg(0,0, data,1);
}

Bool Check_Register(U8 regaddr, U32 regcontent){
	U32 readreg_contentbuf = 0;
	ReadReg(0,regaddr,&readreg_contentbuf,0);
	if (regcontent == readreg_contentbuf)
		return TRUE;
	return FALSE;
}


U8 Self_Check(U32 content15,U32 content1C){
	U8 return_val = 0;
	Bool flg = 0;
	
	flg = Check_Register(REGCONFIG_ADDR, content15);
	if (0 == flg) //if register 0x15 self-check failed, set bit0 0f return_val;
		return_val |= READ_ERR_REG15; 
	
	flg = Check_Register(REGSAMPLE_TIME_ADDR,content1C);
	if (0 == flg)//if register 0x1C self-check failed, set bit1 0f return_val;
		return_val |= READ_ERR_REG1C;

	return return_val;
}

void RDA_DeviceNumReset(U8 chip_num){
	U32 reg15buf;
	U8 mode;
	ReadReg(0,REGCONFIG_ADDR,&reg15buf,0);
	mode = (reg15buf>>12)&0x03;
	RDA_DeviceConfig(chip_num,mode);
}

void RDA_DeviceModReset(U8 mode){
	U32 reg15buf;
	U8 chip_num;
	ReadReg(0,REGCONFIG_ADDR,&reg15buf,0);
	chip_num = (reg15buf>>3)&0x0F;
	RDA_DeviceConfig(chip_num,mode);
}

void RDA_DeviceConfig(U8 chip_num,U8 mode)
{
	RDA3216_PWROFF;
	delay_ms(10);
	RDA3216_PWRON;
	
	g_Chain_DevNO = chip_num;
	U32 cmd_reg15 = (mode<<14)|(mode<<12)|((chip_num-1)<<3);
	SPI0_BaudRate_Set(SPI_Baudrate_250K);
	//write a single register -15h, chip_num devices in the chain of mode "mode"
	SendWriteCommand(0, REGCONFIG_ADDR, cmd_reg15, 1);
	
	//set all devices in the chain as expected mode
	U32 cmd_reg1C = CMD_DIFF_MODE;
	//SendWriteCommand(0,REGSAMPLE_TIME_ADDR,cmd_reg1C,1);

	/*U8 selfchk_flg = Self_Check(cmd_reg15,cmd_reg1C);
	if (0 == selfchk_flg)
		printf("device passed selfcheck!\n");
	else
		printf("selfcheck failed!\n");*/
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Convert_CellVoltage                                  */
/* @brief Description  :                                                      */
/* @param parameters   : ADValue => Raw AD sampling value                     */
/* @return Value       : Converted Cell Voltage, 3.001V = 3001                */
/******************************************************************************/

U32 ADCtoVoltage(U32 ADValue){
	//return (((ADValue/128.0/1024.0) - 0.5) * 10.0);
	//return (((ADValue-(5<<17))*10000)>>17);
	return (((ADValue*10-(5<<17))*1000)>>17);
}

void ADCtoVoltage_Serial(U32* data, U32 *Voltage_Value, U8 Len){
	for (U8 i=0; i<Len; i++)
	{
		*(Voltage_Value+i) = ADCtoVoltage(data[i]);
	}
}

void GetChainVolAndAux(void){
	
}

#define voltage_Channal_Num 16
void RDA_Test(void)
{
	U32 transValue[112] =  {0};
	U32 Voltage[112] = {0};
	float voltage_t[CHAIN_DEV_NUM][7] = {{0},{0},{0}};
	
	//RDA_DeviceConfig(g_Chain_DevNO,SevenVol_NineAux);
	
	ReadADCVal_SerialDevs(&transValue[0]);
	ADCtoVoltage_Serial(&transValue[0],Voltage,g_Serial_RegNO * g_Chain_DevNO);
	
	for (U8 i=0; i<16*g_Chain_DevNO; i++){
		if (0 == i%16){
			printf("\n\tdev_%d\t",i/16);
			
			for(U8 j=0;j<7;j++){
				voltage_t[i/16][j] = Voltage[i+j];
				printf("%d\t",(int)(voltage_t[i/16][j]));
				//printf("%d\t",(int)(transValue[i+j]));
			}
			for(U8 j=0;j<7;j++){
				printf("%d\t",(int)(transValue[i+j]));
			}
		}
	}
}


void RDA3216_Init(void){
	U8 devAddr = 0;
	U8 regAddr = 0;
	U32 cmd = 0;
	U8 seekAllCmd = 0;
	u_RDA_Config_Cmd config_cmd_t;
	
	RDA3216_PWROFF;
	delay_ms(10);
	RDA3216_PWRON;

	g_Chain_DevNO 					= CHAIN_DEV_NUM;
	
	config_cmd_t.cmd.sel_conv_mode	= 0x00;
	config_cmd_t.cmd.rd_conv_mode	= 0x00;
	config_cmd_t.cmd.pd_form		= 0;
	config_cmd_t.cmd.soft_reset		= 0;
	config_cmd_t.cmd.chip_num		= CHAIN_DEV_NUM;
	config_cmd_t.cmd.lock_dev_addr	= 1;
	config_cmd_t.cmd.inc_dev_addr	= 1;
	
	devAddr		= 0;
	seekAllCmd	= 1;
	regAddr		= REGCONFIG_ADDR;
	cmd			= (config_cmd_t.data[0] << 8) + config_cmd_t.data[1];
	
	SendWriteCommand(devAddr, regAddr, cmd, seekAllCmd);
	
	cmd = CMD_DIFF_MODE;
	SendWriteCommand(devAddr, REGSAMPLE_TIME_ADDR, cmd, 1); // sample time = 400ns, current detected in diff mode
}

void RDA_Read_Register(U8 devAddr, U8 regAddr, U8 *data, U8 seek_all_flg){
//	U32 cmd = 0;
	U8 seekAllCmd = 0;
	SendReadCommand(devAddr, regAddr, seekAllCmd);
	U32 cnt = 0;
	U32 RdCmd_buf[MAX_SERIALREGNO] = {0}; // MAX_SERIALREGNO = 48

	RDA3216_ConvsStart;
	while( cnt !=0xFFFF) cnt++; // delay
	SendReadCommand(devAddr,regAddr,seek_all_flg);
	
	if( 0 == regAddr)//read a serial registers' content of a device
	{
		Spi0_RdBytes(&RdCmd_buf[0], g_Chain_DevNO*g_Serial_RegNO);
		for (U8 k=0; k<g_Chain_DevNO*g_Serial_RegNO; k++)
		{
			data[k] = (RdCmd_buf[k]>>6) & 0x1FFFF;
		}
	}
	else
	{
		Spi0_RdBytes(&RdCmd_buf[0],RDCMD_LEN*g_Serial_RegNO);
		data[0] = (RdCmd_buf[0]>>6) & 0x1FFFF;
	}
	
	RDA3216_ConvsDone;
}



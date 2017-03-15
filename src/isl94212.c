#include "isl94212.h"
#include "isl94212_CRC.h"
#include "drivers.h"
#include "stdio.h"
#include "public.h"

/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*               isl94212 Register                                                                                                          */
/*------------------------------------------------------------------------------------------------------------------------------------------*/
#define DEV_ADDRESS_IDENTIFY        0x0
#define DEV_ADDRESS_MASTER          0x1
#define DEV_ADDRESS_BROADCAST       0xF

/* R/W Access */
#define ACCESS_READ     0
#define ACCESS_WRITE    1

/* Register Page */
#define PAGE_READ           1
#define PAGE_CONFIG         2
#define PAGE_COMMAND        3
#define PAGE_RESERVE1       4
#define PAGE_RESERVE2       5

/* READ ONLY - PAGE_READ - CELL VOLTAGE  */
#define REG_VOLATAGE_VBAT       0x0
#define REG_VOLATAGE_CELL1      0x1
#define REG_VOLATAGE_CELL2      0x2
#define REG_VOLATAGE_CELL3      0x3
#define REG_VOLATAGE_CELL4      0x4
#define REG_VOLATAGE_CELL5      0x5
#define REG_VOLATAGE_CELL6      0x6
#define REG_VOLATAGE_CELL7      0x7
#define REG_VOLATAGE_CELL8      0x8
#define REG_VOLATAGE_CELL9      0x9
#define REG_VOLATAGE_CELL10     0xA
#define REG_VOLATAGE_CELL11     0xB
#define REG_VOLATAGE_CELL12     0xC
#define REG_VOLATAGE_ALL        0xF

/* READ ONLY - PAGE_READ - TEMPERATURE ETC. */
#define REG_TEMP_INTERNAL       0x10
#define REG_TEMP_EXTERNAL1      0x11
#define REG_TEMP_EXTERNAL2      0x12
#define REG_TEMP_EXTERNAL3      0x13
#define REG_TEMP_EXTERNAL4      0x14
#define REG_VOLTAGE_REF         0x15
#define REG_SCAN_COUNT          0x16  /* READ & WRITE */
#define REG_TEMP_ALL            0x1F 

/* READ/WRITE - PAGE_CONFIG - FAULT DATA */
#define REG_FAULT_OV            0x0
#define REG_FAULT_UV            0x1
#define REG_FAULT_OW            0x2
#define REG_FAULT_SETUP         0x3
#define REG_FAULT_STATUS        0x4
#define REG_FAULT_CELL_SETUP    0x5
#define REG_FAULT_OT            0x6
#define REG_FAULT_ALL           0xF

/* READ/WRITE - PAGE_CONFIG - DEVICE SETUP */
#define REG_SETUP_OV_LIMIT          0x10
#define REG_SETUP_UV_LIMIT          0x11
#define REG_SETUP_EOT_LIMIT         0x12    /* External Over Temperature */
#define REG_SETUP_BALANCE           0x13
#define REG_SETUP_BALANCE_STATUS    0x14
#define REG_SETUP_WD_BALANCE_TIME   0x15
#define REG_SETUP_USER1             0x16
#define REG_SETUP_USER2             0x17
#define REG_SETUP_COMMS             0x18
#define REG_SETUP_DEVICE            0x19
#define REG_SETUP_IOT_LIMIT         0x1A    /* Internal Over Temperature */
#define REG_SETUP_SN1               0x1B    /* READ ONLY */
#define REG_SETUP_SN2               0x1C    /* READ ONLY */
#define REG_SETUP_TRIM_VOLTAGE      0x1D    /* READ ONLY (VALUE SET IN EEPROM) */
#define REG_SETUP_ALL               0x1F    /* READ ONLY */

/* READ/WRITE - PAGE_CONFIG - CELL BALANCE */
#define REG_BV1_L           0x20
#define REG_BV1_H           0x21
#define REG_BV2_L           0x22
#define REG_BV2_H           0x23
#define REG_BV3_L           0x24
#define REG_BV3_H           0x25
#define REG_BV4_L           0x26
#define REG_BV4_H           0x27
#define REG_BV5_L           0x28
#define REG_BV5_H           0x29
#define REG_BV6_L           0x2A
#define REG_BV6_H           0x2B
#define REG_BV7_L           0x2C
#define REG_BV7_H           0x2D
#define REG_BV8_L           0x2E
#define REG_BV8_H           0x2F
#define REG_BV9_L           0x30
#define REG_BV9_H           0x31
#define REG_BV10_L          0x32
#define REG_BV10_H          0x33
#define REG_BV11_L          0x34
#define REG_BV11_H          0x35
#define REG_BV12_L          0x36
#define REG_BV12_H          0x37
#define REG_CELL_IN_BALANCE 0x3B

/* READ ONLY - PAGE_CONFIG - REFERENCE COEFFICIENT */
#define REG_REF_COEFFICIENT_C          0x38
#define REG_REF_COEFFICIENT_B          0x39
#define REG_REF_COEFFICIENT_A          0x3A

/* READ ONLY - PAGE_COMMAND - DEVICE COMMAND */
#define REG_CMD_SCAN_VOLTAGE            0x1
#define REG_CMD_SCAN_TEMP               0x2
#define REG_CMD_SCAN_MIXED              0x3
#define REG_CMD_SCAN_WIRE               0x4
#define REG_CMD_SCAN_ALL                0x5
#define REG_CMD_SCAN_CONTINUOUS         0x6
#define REG_CMD_SCAN_INHIBIT            0x7
#define REG_CMD_SCAN_MEASURE            0x8
#define REG_CMD_IDENTIFY                0x9
#define REG_CMD_SLEEP                   0xA
#define REG_CMD_NAK                     0xB
#define REG_CMD_ACK                     0xC
#define REG_CMD_COMMS_FAILURE           0xE
#define REG_CMD_WAKEUP                  0xF
#define REG_CMD_ENABLE_BALANCE          0x10
#define REG_CMD_DISABLE_BALANCE         0x11
#define REG_CMD_RESET                   0x12
#define REG_CMD_CALC_CRC                0x13
#define REG_CMD_CHECK_CRC               0x14

/* READ ONLY - PAGE_RESERVED1 - MISR */
#define REG_MISR                        0x3F

/* READ ONLY - PAGE_RESERVED2 - MISR SHADDOW */
#define REG_MISR_SHADDOW               	0x0

#define DATA_READY_HIGH 				(FGPIOB_PDIR & (1 << 12))

/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*               Internal Data Type Definition                                                                                              */
/*------------------------------------------------------------------------------------------------------------------------------------------*/
/* 3 byte lenght command request */
typedef struct {

    U8 page              : 3;
    U8 access            : 1;
    U8 devAddress        : 4;
    
    U8 eleAdd1           : 2;
    U8 regAddress        : 6;

    U8 crc               : 4;
    U8 eleAdd2           : 4;

} Request_CommandL3;

/* 4 byte lenght command request */
typedef struct {

    U8 page              : 3;
    U8 access            : 1;
    U8 devAddress        : 4;
    
    U8 eleAdd1           : 2;
    U8 regAddress        : 6;
    
    U16 eleAdd2_1        : 8;
	
    U16 crc              : 4;
	U16 eleAdd2_2		 : 4;

} Request_CommandL4;

/* 3 byte lenght command response */
typedef struct {

    U8 page              : 3;
    U8 access            : 1;
    U8 devAddress        : 4;
    
    U8 eleAdd1           : 2;
    U8 regAddress        : 6;
    
    U8 dummy             : 4;
    U8 eleAdd2           : 4;

} Response_CommandL3;

/* 4 byte lenght command response */
typedef struct {

	U8 page              : 3;
	U8 access            : 1;
	U8 devAddress        : 4;

	U8 eleAdd1           : 2;
	U8 regAddress        : 6;

	U16 eleAdd2_1        : 8;

	U16 crc              : 4;
	U16 eleAdd2_2		 : 4;

} Response_CommandL4;

/* 4 byte lenght data response */
typedef struct {

	U8 page              : 3;
	U8 access            : 1;
	U8 devAddress        : 4;

	U8 data1             : 2;
	U8 regAddress        : 6;

//	U16 crc              : 4;
//	U16 data2			 : 12;

	U16 data2_1			 : 8;
	U16 crc              : 4;
	U16 data2_2          : 4;

} Response_DataL4; 

/* 3 byte lenght data response */
typedef struct {

	U8 data1				: 2;
	U8 dataAddress			: 6;
	U16 data2_1				: 8;
	U16 crc					: 4;
	U16 data2_2				: 4; 

} Response_DataL3; 

/* isl94212 Command */
typedef union {

    Request_CommandL3 L3;
    Request_CommandL4 L4;
    U8  data[4];
    
} ISL_Command;

/* isl94212 Command Response */
typedef union {

    Response_CommandL3 L3;
    Response_CommandL4 L4;
    U8  data[4];
    
} ISL_Command_Response;

/* isl94212 Data Response */
typedef union {
    Response_DataL4 L4;

    struct {
        Response_DataL4 L4Data;
        Response_DataL3 L3Data[6];
    } L22;

    struct {
        Response_DataL4 L4Data;
        Response_DataL3 L3Data[12];
    } L40;

    struct {
        Response_DataL4 L4Data;
        Response_DataL3 L3Data[13];
    } L43;

    U8  data[MAX_RESP_SIZE];
    
} ISL_Data_Response;

/*******************************************************************************************************************
 * the data is from the NTC datasheet, range from -55 to 175 degree, reference is -55 degree,
 * the data unit in the table is ohm*10,when T_Table[80]=10250,means that the resister is 1025 ohm now,
 * and the related temperature is 80 - 55 = 25 degree.
*********************************************************************************************************************/
#define T_TABLE_LENGTH 216
const U16 T_Table[T_TABLE_LENGTH] = {
/* 10 temperatures per line, the data is R*10 */												
///*	-55	 */	5120	,	5172	,	5224	,	5276	,	5328	,	5380	,	5433	,	5486	,	5539	,	5592	,
///*	-45	 */	5645	,	5698	,	5751	,	5804	,	5857	,	
/*	-40	 */	5910	,	5967	,	6024	,	6081	,	6138	,
/*	-35	 */	6195	,	6252	,	6309	,	6366	,	6423	,	6480	,	6541	,	6602	,	6663	,	6724	,
/*	-25	 */	6785	,	6846	,	6907	,	6968	,	7029	,	7090	,	7154	,	7218	,	7282	,	7346	,
/*	-15	 */	7410	,	7474	,	7538	,	7602	,	7666	,	7730	,	7797	,	7864	,	7931	,	7998	,
/*	-5	 */	8065	,	8132	,	8199	,	8266	,	8333	,	8400	,	8472	,	8544	,	8616	,	8688	,
/*	5	 */	8760	,	8832	,	8904	,	8976	,	9048	,	9120	,	9194	,	9268	,	9342	,	9416	,
/*	15	 */	9490	,	9564	,	9638	,	9712	,	9786	,	9860	,	9938	,	10016	,	10094	,	10172	,
/*	25	 */	10250	,	10330	,	10410	,	10490	,	10570	,	10650	,	10731	,	10812	,	10893	,	10974	,
/*	35	 */	11055	,	11136	,	11217	,	11298	,	11379	,	11460	,	11546	,	11632	,	11718	,	11804	,
/*	45	 */	11890	,	11976	,	12062	,	12148	,	12234	,	12320	,	12409	,	12498	,	12587	,	12676	,
/*	55	 */	12765	,	12854	,	12943	,	13032	,	13121	,	13210	,	13302	,	13394	,	13486	,	13578	,
/*	65	 */	13670	,	13762	,	13854	,	13946	,	14038	,	14130	,	14226	,	14322	,	14418	,	14514	,
/*	75	 */	14610	,	14706	,	14802	,	14898	,	14994	,	15090	,	15190	,	15290	,	15390	,	15490	,
/*	85	 */	15590	,	15690	,	15790	,	15890	,	15990	,	16090	,	16193	,	16296	,	16399	,	16502	,
/*	95	 */	16605	,	16708	,	16811	,	16914	,	17017	,	17120	,	17226	,	17332	,	17438	,	17544	,
/*	105	 */	17650	,	17756	,	17862	,	17968	,	18074	,	18180	,	18291	,	18402	,	18513	,	18624	,
/*	115	 */	18735	,	18846	,	18957	,	19068	,	19179	,	19290	,	19402	,	19514	,	19626	,	19738	,
/*	125	 */	19850	,	19964	,	20078	,	20192	,	20306	,	20420	,	20538	,	20656	,	20774	,	20892	,
/*	135	 */	21010	,	21128	,	21246	,	21364	,	21482	,	21600	,	21720	,	21840	,	21960	,	22080	,
/*	145	 */	22200	,	22320	,	22440	,	22560	,	22680	,	22800	,	22925	,	23050	,	23175	,	23300	,
/*	155	 */	23425	,	23550	,	23675	,	23800	,	23925	,	24050	,	24178	,	24306	,	24434	,	24562	,
/*	165	 */	24690	,	24818	,	24946	,	25074	,	25202	,	25330	,	25460	,	25590	,	25720	,	25850	,
/*	175	 */	25980																			
};



/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*               Public Functions                                                                                                           */
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/****************************************************************************/
/**																			*/
/* @brief Function Name:	Find_Temp_Tab									*/
/* @brief Description  :	according to the resister of the NTC, find the	*/
/*							temperature in the table.						*/
/* @param parameters   :													*/
/* @return Value       :	the temperature find result						*/
/****************************************************************************/
U8 Find_Temp_Table(const U16 *list_table, U8 list_length, U32 to_be_find) {
    U8 low = 0;
    U8 hight = list_length - 1;
    U8 mid;
    
    while(low <= hight) {
		if(to_be_find <= list_table[0]){
			return 0;
		}
		
        if((hight-low) == 1) {
            if((to_be_find - list_table[low]) < (list_table[hight] - to_be_find)) {
                return low;
            }
            else {
                return hight;
            }
        }
        
        mid = (U8)((U32)(low + hight) >> 1);
        if(to_be_find > list_table[mid]) {
            low = mid;
        }
        else if(to_be_find < list_table[mid]) {
            hight = mid;
        }
        else {
            return mid;
        }
    }
    return 0;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Convert_CellVoltage                                  */
/* @brief Description  :                                                      */
/* @param parameters   : ADValue => Raw AD sampling value                     */
/* @return Value       : Converted Cell Voltage, 3.001V = 3001                */
/******************************************************************************/
U16 Convert_CellVoltage(U16 ADValue) {
    U16 ret = 0;

    if(ADValue >= 8191) {
        ret = (U16)((U32)((U32)(16384 - ADValue) * (U32)5000) / (U32)8192);
    } else {
        ret = (U16)((U32)(ADValue) * (U32)5000 / (U32)8192);
    }

//    ret = (U16)((U32)(ADValue) * (U32)5000 / (U32)8192);// VREF =2500mV, 13bit AD (2^13 = 8192),hex_code*2*VREF=5000
    return ret;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Convert_BatVoltage                                   */
/* @brief Description  :                                                      */
/* @param parameters   : ADValue => Raw AD sampling value                     */
/* @return Value       : Converted Battery Voltage, 13.001V = 13001           */
/******************************************************************************/
U16 Convert_BatVoltage(U16 ADValue) {
    U16 ret = 0;

//    if(ADValue >= 8191) {
//        ret = (U16)((U32)((U32)(16384 - ADValue) * (U32)39837) / (U32)8192);
//    } else {
//        ret = (U16)((U32)(ADValue * (U32)39837) / (U32)8192);
//    }

    ret = (U16)((U32)(ADValue * (U32)39837) / (U32)8192); // datasheet page62,15.9350784*2.5=39.837
    return ret;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Convert_ADValue_To_Voltage                                 */
/* @brief Description  :                                                      */
/* @param parameters   : ADValue => Raw AD sampling value                     */
/* @return Value       : Converted Temperature Voltage, 3.001V = 3001         */
/******************************************************************************/
U16 Convert_ADValue_To_Voltage(U16 ADValue) {
    U16 ret = 0;
    ret = (U16)((U32)(ADValue) * (U32)2500 / (U32)16384);
    return ret;
}

/********************************************************************************/
/**																				*/
/* @brief Function Name: Convert_ADValue_To_Voltage								*/
/* @brief Description  :														*/
/* @param parameters   : ADValue => Raw AD sampling value						*/
/* @return Value       : Converted Temperature, 135 means 135-55=80 degree		*/
/********************************************************************************/
U16 Convert_ADValue_To_ExtTemp(U16 ADValue) {
    U32 v = 0;
    U32 r;
    U16 t;
    v = (U32)((U32)(ADValue) * (U32)2500 / (U32)16384); /* Ext Temp voltage Uint is mv */
    r = ((U32)10 * 1000 * v * 10) / ((U32)2500 - (U32)v); /* Ext NTC resistance Unit is Ohm*10 */
#if 0
	r = r - (U32)100000;
#endif
    t = (U16)Find_Temp_Table(T_Table, T_TABLE_LENGTH, r);
    return t;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Convert_ADValue_To_ICTemp                            */
/* @brief Description  :                                                      */
/* @param parameters   : ADValue => Raw AD sampling value                     */
/* @return Value       : Converted Temperature Voltage, 3.001V = 3001         */
/******************************************************************************/
U16 Convert_ADValue_To_ICTemp(U16 ADValue) {
    U16 ret = 0;
    ret = (U16)(((U32)(ADValue - 9180) * (U32)10 / (U32)319) + 25);
    return ret;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Merge14BitDataL4                                     */
/* @brief Description  :                                                      */
/* @param parameters   :                                                      */
/* @return Value       : Merged 14bit data, top 2 bit is always 0             */
/******************************************************************************/
U16 Merge14BitDataL4(Response_DataL4 L4Data) {
//    return ((L4Data.data1 & 0x03) << 12) | (L4Data.data2 & 0x0FFF);
	return (((L4Data.data1 & 0x03) << 12) | 
			(L4Data.data2_1 & 0x0FF0) | 
			(L4Data.data2_2 & 0x000F));
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Merge14BitDataL3                                     */
/* @brief Description  :                                                      */
/* @param parameters   :                                                      */
/* @return Value       : Merged 14bit data, top 2 bit is always 0             */
/******************************************************************************/
U16 Merge14BitDataL3(Response_DataL3 L3Data) {
//    return ((L3Data.data1 & 0x03) << 12) | (L3Data.data2 & 0x0FFF);
    return (((L3Data.data1 & 0x03) << 12) | 
			(L3Data.data2_1 & 0x0FF0) | 
			(L3Data.data2_2 & 0x000F));
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: SendCommandL3                                        */
/* @brief Description  : Send 3 byte length disychain command                 */
/* @param parameters   : devAddress = > Device Address                        */
/*                       access = > R:0 / W:1                                 */
/*                       page = > Register Page [1,2,3,4,5]                   */
/*                       regAddress = > Register Address                      */
/*                       eleAddress = > Element Address / Data                */
/* @return Value       : None                                                 */
/******************************************************************************/
void SendCommandL3(U8 devAddress, 
                   U8 access, 
                   U8 page, 
                   U8 regAddress, 
                   U8 eleAddress) {
	U8 data_temp = 0;
	U8 RetryCount = 0xff;
    ISL_Command command;
	
    command.L3.devAddress = devAddress;
    command.L3.access = access;
    command.L3.page = page;
    command.L3.regAddress = regAddress;
    command.L3.eleAdd1 = ((eleAddress & 0x30) >> 4);
    command.L3.eleAdd2 = ((eleAddress & 0x0F) >> 0);
    command.L3.crc = 0;
    CalcCRC4(command.data, 3);
					   
	/* if DATA_READY port is low, means that there is data in isl94212 */
	while((!DATA_READY_HIGH)&&(RetryCount--)){
		data_temp = Spi_RdWrByte(SPI0_BASE_PTR,0x00);
		data_temp = data_temp;	// dummy read to kill the IDE warnings.
	}
	
    Spi0_WrBytes(command.data, 3);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: SendCommandL4                                        */
/* @brief Description  : Send 4 byte length disychain command                 */
/* @param parameters   : devAddress = > Device Address                        */
/*                                                                            */
/*                       access = > R:0 / W:1                                 */
/*                       page = > Register Page [1,2,3,4,5]                   */
/*                       regAddress = > Register Address                      */
/*                       eleAddress = > Element Address / Data                */
/* @return Value       : None                                                 */
/******************************************************************************/
void SendCommandL4(U8 devAddress, 
                   U8 access, 
                   U8 page, 
                   U8 regAddress, 
                   U16 eleAddress) {
    U8 data_temp = 0;
    ISL_Command command;
    command.L4.devAddress = devAddress;
    command.L4.access = access;
    command.L4.page = page;
    command.L4.regAddress = regAddress;
	
    command.L4.eleAdd1 = ((eleAddress & 0x3000) >> 12);	
    command.L4.eleAdd2_1 = (eleAddress & 0x0FF0) >> 4;
	command.L4.eleAdd2_2 = (eleAddress & 0x000F) << 0;

    command.L4.crc = 0;
    CalcCRC4(command.data, 4);
					   
	/* if DATA_READY port is low, means that there is data in isl94212 */
	while(!DATA_READY_HIGH){
		data_temp = Spi_RdWrByte(SPI0_BASE_PTR,0x00);
		data_temp = data_temp;	// dummy read to kill the IDE warnings.
	}
	
    Spi0_WrBytes(command.data, 4);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: SendCommandRequest                                   */
/* @brief Description  : Semd 3 byte lenght daisychain command and Receive &  */
/*                       decode 4 byte length disychain response              */
/*                                                                            */
/* @param parameters   : devAddress => Device Address                         */
/*                       regAddress => Register Address                       */
/*                       eleAddress => Element Address                        */
/*                       outErrCode => ErrCode[out]                           */
/*                                                                            */
/* @return Value       : Return the ISL_Command_Resoponse struct and          */
/*                       output the errCode to pointer outErrCode             */
/******************************************************************************/
ISL_Command_Response SendCommandRequest(U8 devAddress, 
                                        U8 regAddress, 
                                        U8 eleAddress, 
                                        ISL_ErrCode *outErrCode) {
    ISL_Command_Response cmd_response;
    Bool readResult = FALSE;

    /*  Send 3 bytes command */
    SendCommandL3(devAddress, ACCESS_READ, PAGE_COMMAND, regAddress, eleAddress);
	
    /*  Read 4 bytes command response */
    readResult = Spi0_ReadBytes(&cmd_response.data[0], 4);

    /*  Check Response Status */
    if(readResult == FALSE) {
        *outErrCode = ErrCode_NoResponse;
    }
		
    /*  Check Response data CRC */
    else if(CheckCRC4(cmd_response.data, 4) == FALSE) {
        *outErrCode = ErrCode_CRC;
    }
    /*  Check Response for NAK */
    else if(cmd_response.L4.regAddress == REG_CMD_NAK) {
        *outErrCode = ErrCode_NAK;
    }  
    else {
        *outErrCode = ErrCode_None;
    }

    return cmd_response;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: SendReadRequestEx                                    */
/* @brief Description  : Semd 3 byte lenght daisychain command and Receive &  */
/*                       decode 4 byte length disychain response              */
/*                                                                            */
/* @param parameters   : devAddress => Device Address                         */
/*                       regAddress => Regist Address                         */
/*                       eleAddress => Element Address                        */
/*                       respLength => Expect Response Length                 */
/*                       outErrCode => ErrCode[out]                           */
/*                                                                            */
/* @return Value       : Return the ISL_Data_Response struct and              */
/*                       output the errCode to pointer outErrCode             */
/******************************************************************************/
ISL_Data_Response SendReadRequestEx(U8 devAddress, 
                                    U8 regAddress, 
                                    U8 eleAddress, 
                                    U8 respLength,
                                    ISL_ErrCode *outErrCode) {
    ISL_Data_Response data_response;
    Bool readResult = FALSE;

    /*  Send 3 bytes read command */
    SendCommandL3(devAddress, ACCESS_READ, PAGE_READ, regAddress, eleAddress);

    /*  Read [respLength] bytes command response */
    readResult = Spi0_ReadBytes(&data_response.data[0], respLength);

    /*  Check Response Status */
    if(readResult == FALSE) {
        *outErrCode = ErrCode_NoResponse;
    }
    /*  Check Response data CRC (Only check first 4 bytes CRC) */
    else if(CheckCRC4(data_response.data, 4) == FALSE) {
        *outErrCode = ErrCode_CRC;
    }
    /*  Check Response for NAK */
    else if(data_response.L4.regAddress == REG_CMD_NAK) {
        *outErrCode = ErrCode_NAK;
    }  
    else {
        *outErrCode = ErrCode_None;
    }

    return data_response;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: SendReadRequest                                      */
/* @brief Description  : Semd 3 byte lenght read command and Receive & Decode */
/*                       4 byte length disychain response                     */
/*                                                                            */
/* @param parameters   : devAddress => Device Address                         */
/*                       regAddress => Regist Address                         */
/*                       eleAddress => Element Address                        */
/*                       outErrCode => ErrCode[out]                           */
/*                                                                            */
/* @return Value       : Return the ISL_Data_Response struct and              */
/*                       output the errCode to pointer outErrCode             */
/******************************************************************************/
ISL_Data_Response SendReadRequest(U8 devAddress, 
                                  U8 regAddress, 
                                  ISL_ErrCode *outErrCode) {
    return SendReadRequestEx(devAddress, regAddress, 0, 4, outErrCode);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: SendConfigReadRequestEx                              */
/* @brief Description  : Send 3 byte lenght command and Receive & Decode 4    */
/*                       byte length disychain response                       */
/*                                                                            */
/* @param parameters   : devAddress => Device Address                         */
/*                       regAddress => Regist Address                         */
/*                       eleAddress => Element Address                        */
/*                       outErrCode => ErrCode[out]                           */
/*                                                                            */
/* @return Value       : Return the ISL_Data_Response struct and              */
/*                       output the errCode to pointer outErrCode             */
/******************************************************************************/
ISL_Data_Response SendConfigReadRequestEx(U8 devAddress, 
                                          U8 regAddress, 
                                          U8 eleAddress, 
                                          U8 respLength,
                                          ISL_ErrCode *outErrCode) {
    ISL_Data_Response data_response;
    Bool readResult = FALSE;

    /*  Send 3 bytes read config command */
    SendCommandL3(devAddress, ACCESS_READ, PAGE_CONFIG, regAddress, eleAddress);
    
    /*  Read [respLength] bytes response */
    readResult = Spi0_ReadBytes(&data_response.data[0], respLength);

    /*  Check Response Status */
    if(readResult == FALSE) {
        *outErrCode = ErrCode_NoResponse;
    }
    /*  Check Response data CRC (Only check first 4 bytes CRC) */
    else if(CheckCRC4(data_response.data, 4) == FALSE) {
        *outErrCode = ErrCode_CRC;
    }
    /*  Check Response for NAK */
    else if(data_response.L4.regAddress == REG_CMD_NAK) {
        *outErrCode = ErrCode_NAK;
    }  
    else {
        *outErrCode = ErrCode_None;
    }

    return data_response;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: SendConfigReadRequest                                */
/* @brief Description  : Semd 4 byte lenght command and Receive & Decode 4    */
/*                       byte length disychain response                       */
/*                                                                            */
/* @param parameters   : devAddress => Device Address                         */
/*                       regAddress => Regist Address                         */
/*                       eleAddress => Element Address                        */
/*                       outErrCode => ErrCode[out]                           */
/*                                                                            */
/* @return Value       : Return the ISL_Data_Response struct and              */
/*                       output the errCode to pointer outErrCode             */
/******************************************************************************/
ISL_Data_Response SendConfigReadRequest(U8 devAddress, 
                                        U8 regAddress, 
                                        ISL_ErrCode *outErrCode) {
    return SendConfigReadRequestEx(devAddress, regAddress, 0, 4, outErrCode);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: SendConfigWriteRequest                               */
/* @brief Description  : Semd 4 byte lenght command and Receive & Decode 4    */
/*                       byte length disychain response                       */
/*                                                                            */
/* @param parameters   : devAddress => Device Address                         */
/*                       regAddress => Regist Address                         */
/*                       eleAddress => Element Address                        */
/*                       outErrCode => ErrCode[out]                           */
/*                                                                            */
/* @return Value       : Return the ISL_Data_Response struct and              */
/*                       output the errCode to pointer outErrCode             */
/******************************************************************************/
ISL_Data_Response SendConfigWriteRequest(U8 devAddress, 
                                         U8 regAddress, 
                                         U16 eleAddress, 
                                         ISL_ErrCode *outErrCode) {
    ISL_Data_Response data_response;
    Bool readResult = FALSE;

    /*  Send 4 bytes write command */
    SendCommandL4(devAddress, ACCESS_WRITE, PAGE_CONFIG, regAddress, eleAddress);
    
    /*  Read 4 bytes response */
    readResult = Spi0_ReadBytes(&data_response.data[0], 4);

    /*  Check Response Status */
    if(readResult == FALSE) {
        *outErrCode = ErrCode_NoResponse;
    }
    /*  Check Response data CRC (Only check first 4 bytes CRC) */
    else if(CheckCRC4(data_response.data, 4) == FALSE) {
        *outErrCode = ErrCode_CRC;
    }
    /*  Check Response for NAK */
    else if(data_response.L4.regAddress == REG_CMD_NAK) {
        *outErrCode = ErrCode_NAK;
    }  
    else {
        *outErrCode = ErrCode_None;
    }

    return data_response;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_Config                                */
/* @brief Description  : Write config data to register address                */
/*                                                                            */
/* @param parameters   : devAddress => Device Address                         */
/*                       regAddress => Regist Address                         */
/*                       regData => Register Data                             */
/*                                                                            */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_Config(U8 devAddress, U8 regAddress, U16 regData) {
    ISL_ErrCode errCode = ErrCode_NoResponse;

    ISL_Data_Response data_response = SendConfigWriteRequest(devAddress, regAddress, regData, &errCode);
    if(errCode == ErrCode_None) {
        /*  Check ACK */
        if(data_response.L4.regAddress != REG_CMD_ACK) {
            errCode = ErrCode_ACK;
        }
    }
    
    return errCode;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_Config                                */
/* @brief Description  : Read config data from register address               */
/*                                                                            */
/* @param parameters   : devAddress => Device Address                         */
/*                       regAddress => Regist Address                         */
/*                       outRegData => Register Data[out]                     */
/*                                                                            */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_Config(U8 devAddress, U8 regAddress, U16 *outRegData) {
    ISL_ErrCode errCode = ErrCode_NoResponse;

    ISL_Data_Response data_response = SendConfigReadRequest(devAddress, regAddress, &errCode);
    if(errCode == ErrCode_None) {
        *outRegData = Merge14BitDataL4(data_response.L4);
    }
    
    return errCode;
}

/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*               DaisyChain Command                                                                                                         */
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Identify                                  */
/* @brief Description  : Identify disychain devices                           */
/* @param parameters   : deviceCnt => Total device count                      */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Identify(U8 deviceCnt) {
    
    U8 devAddress, retryCnt = 0;
    ISL_Command_Response response;
    ISL_ErrCode errCode = ErrCode_NoResponse;

    /*  Start Identify Device from master Device(address = 1) to top stack Device(address = deviceCnt) */
    for(devAddress = 1; devAddress <= deviceCnt; devAddress++) {
        /*  Master Device (Bottom Stack) */
        if(devAddress == 1) {
            /*  Try reset MAX_RETRY_COUNT times if Identify master failed  */
            while(errCode != ErrCode_None && retryCnt < MAX_RETRY_COUNT) {
                response = SendCommandRequest(DEV_ADDRESS_IDENTIFY, REG_CMD_IDENTIFY, 0, &errCode);
                if(errCode != ErrCode_None) {
                    /*  Try to reset master */
                    DaisyChain_Reset(DEV_ADDRESS_MASTER);
					retryCnt++;
                    delay_ms(1);    
                }
            }
            
            /*  Identify command response OK */
            if(errCode == ErrCode_None) {
                /*  Check ACK */
                if(response.L4.regAddress != REG_CMD_ACK) {
                    errCode = ErrCode_ACK;
                    break;
                }
                /*  Check response passed */
                isl_pack.devices[devAddress - 1].devAddress = devAddress;
            } else {
                break;
            }
        } 
        /*  Top Stack Device */
        else if(devAddress == deviceCnt){ 
            /*  Send Top device identify command */
            response = SendCommandRequest(DEV_ADDRESS_IDENTIFY, REG_CMD_IDENTIFY, devAddress, &errCode);
            if(errCode == ErrCode_None){
                /*  Check response for top device comsel && deviceAddress */
                if(((response.L3.eleAdd1&0x03) != 0x02) || (response.L3.eleAdd2 != devAddress)){
                    errCode = ErrCode_ACK;
                    break;
                }    
            } else {
                break;
            }

            /*  Send Identify End Command */
            response = SendCommandRequest(DEV_ADDRESS_IDENTIFY, REG_CMD_IDENTIFY, 0x3F, &errCode);
            if(errCode == ErrCode_None) {
                /*  Check ACK */
                if(response.L4.regAddress != REG_CMD_ACK) {
                    errCode = ErrCode_ACK;
                    break;
                }
                /*  Check response passed */
                isl_pack.devices[devAddress - 1].devAddress = devAddress;
            } else {
                break;
            }
        }
        /*  Middle Position Device  */
        else { 
            /*  Send Middle device identify command */
            response = SendCommandRequest(DEV_ADDRESS_IDENTIFY, REG_CMD_IDENTIFY, devAddress, &errCode);
            if(errCode == ErrCode_None){
                if(((response.L3.eleAdd1&0x03) != 0x03) || (response.L3.eleAdd2 != devAddress)){
                    errCode = ErrCode_ACK;
                    break;
                }
                /*  Check response passed */
                isl_pack.devices[devAddress - 1].devAddress = devAddress;
            } else{
                break;
            }
        }
    }
    
    return errCode;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_ScanVoltages                              */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
void DaisyChain_ScanVoltages(U8 devAddress) {
    SendCommandL3(devAddress, ACCESS_READ, PAGE_COMMAND, REG_CMD_SCAN_VOLTAGE, 0);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_ScanTemps                                 */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
void DaisyChain_ScanTemps(U8 devAddress) {
    SendCommandL3(devAddress, ACCESS_READ, PAGE_COMMAND, REG_CMD_SCAN_TEMP, 0);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_ScanMix                                   */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
void DaisyChain_ScanMix(U8 devAddress) {
    SendCommandL3(devAddress, ACCESS_READ, PAGE_COMMAND, REG_CMD_SCAN_MIXED, 0);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_ScanWires                                 */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
void DaisyChain_ScanWires(U8 devAddress) {
    SendCommandL3(devAddress, ACCESS_READ, PAGE_COMMAND, REG_CMD_SCAN_WIRE, 0);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_ScanAll                                   */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
void DaisyChain_ScanAll(U8 devAddress) {
    SendCommandL3(devAddress, ACCESS_READ, PAGE_COMMAND, REG_CMD_SCAN_ALL, 0);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_ScanContinuous                            */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_ScanContinuous(U8 devAddress) {
    ISL_ErrCode errCode = ErrCode_NoResponse;
    ISL_Command_Response response = SendCommandRequest(devAddress, REG_CMD_SCAN_CONTINUOUS, 0, &errCode);
    if(errCode == ErrCode_None && response.L4.regAddress != REG_CMD_ACK) {
        errCode = ErrCode_ACK;
    }
    return errCode;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_ScanInhibit                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_ScanInhibit(U8 devAddress) {
    ISL_ErrCode errCode = ErrCode_NoResponse;
    ISL_Command_Response response = SendCommandRequest(devAddress, REG_CMD_SCAN_INHIBIT, 0, &errCode);
    if(errCode == ErrCode_None && response.L4.regAddress != REG_CMD_ACK) {
        errCode = ErrCode_ACK;
    }
    return errCode;   
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Measure                                   */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       elementAddress =>                                    */
/*
                                            00 VBAT Voltage
                                            01 Cell 1 Voltage
                                            02 Cell 2 Voltage
                                            03 Cell 3 Voltage
                                            04 Cell 4 Voltage
                                            05 Cell 5 Voltage
                                            06 Cell 6 Voltage
                                            07 Cell 7 Voltage
                                            08 Cell 8 Voltage
                                            09 Cell 9 Voltage
                                            0A Cell 10 Voltage
                                            0B Cell 11 Voltage
                                            0C Cell 12 Voltage
                                            10 Internal temperature reading
                                            11 External temperature input 1 reading.
                                            12 External temperature input 2 reading.
                                            13 External temperature input 3 reading.
                                            14 External temperature input 4 reading.
                                            15 Reference voltage (raw ADC)value.
*/ 
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Measure(U8 devAddress, U8 elementAddress) {
    ISL_ErrCode errCode = ErrCode_NoResponse;
    ISL_Command_Response response = SendCommandRequest(devAddress, REG_CMD_SCAN_MEASURE, elementAddress, &errCode);
    if(errCode == ErrCode_None && response.L4.regAddress != REG_CMD_ACK) {
        errCode = ErrCode_ACK;
    }
    return errCode;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Sleep                                     */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
void DaisyChain_Sleep(U8 devAddress) {
    SendCommandL3(devAddress, ACCESS_READ, PAGE_COMMAND, REG_CMD_SLEEP, 0);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Wakeup                                    */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
void DaisyChain_Wakeup(U8 devAddress) {
    SendCommandL3(devAddress, ACCESS_READ, PAGE_COMMAND, REG_CMD_WAKEUP, 0);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Reset                                     */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
void DaisyChain_Reset(U8 devAddress) {
    SendCommandL3(devAddress, ACCESS_READ, PAGE_COMMAND, REG_CMD_RESET, 0);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Balance_Enable                            */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Balance_Enable(U8 devAddress) {
    ISL_ErrCode errCode = ErrCode_NoResponse;
    ISL_Command_Response response = SendCommandRequest(devAddress, REG_CMD_ENABLE_BALANCE, 0, &errCode);
    if(errCode == ErrCode_None && response.L4.regAddress != REG_CMD_ACK) {
        errCode = ErrCode_ACK;
    }
    return errCode;  
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Balance_Disable                           */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Balance_Disable(U8 devAddress) {
    ISL_ErrCode errCode = ErrCode_NoResponse;
    ISL_Command_Response response = SendCommandRequest(devAddress, REG_CMD_DISABLE_BALANCE, 0, &errCode);
    if(errCode == ErrCode_None && response.L4.regAddress != REG_CMD_ACK) {
        errCode = ErrCode_ACK;
    }
    return errCode;  
}

/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*               DaisyChain Read Functions                                                                                                  */
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Read_CellVoltage                          */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       index => Cell Index 0...N                            */
/*                       outVoltage => Cell Voltage data[out]                 */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Read_CellVoltage(U8 devAddress, U8 index, U16 *outVoltage) {
    U16 ADValue = 0;
    ISL_ErrCode errCode = ErrCode_NoResponse;

    U8 regArr[12] = {REG_VOLATAGE_CELL1, REG_VOLATAGE_CELL2, REG_VOLATAGE_CELL3, REG_VOLATAGE_CELL4,
                        REG_VOLATAGE_CELL5, REG_VOLATAGE_CELL6, REG_VOLATAGE_CELL7, REG_VOLATAGE_CELL8,
                        REG_VOLATAGE_CELL9, REG_VOLATAGE_CELL10, REG_VOLATAGE_CELL11, REG_VOLATAGE_CELL12};
    U8 regAddress = regArr[index];

    ISL_Data_Response data_response = SendReadRequest(devAddress, regAddress, &errCode);
    if(errCode == ErrCode_None) {
        ADValue = Merge14BitDataL4(data_response.L4); 
        *outVoltage = Convert_CellVoltage(ADValue);
    }
    
    return errCode; 
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Read_BatVoltage                           */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outVoltage => Cell Voltage data[out]                 */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Read_BatVoltage(U8 devAddress, U16 *outVoltage) {
    U16 ADValue = 0;
    ISL_ErrCode errCode = ErrCode_NoResponse;

    ISL_Data_Response data_response = SendReadRequest(devAddress, REG_VOLATAGE_VBAT, &errCode);
    if(errCode == ErrCode_None) {
        ADValue = Merge14BitDataL4(data_response.L4); ;    
        *outVoltage = Convert_BatVoltage(ADValue);
    }

    return errCode; 
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Read_AllVoltages                          */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outVoltage => Cell voltage data array[out]           */
/*                       outVBAT => Battery voltage data[out]                 */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Read_AllVoltages(U8 devAddress, U16 *outVoltages, U16 *outVBAT) {
    ISL_ErrCode errCode = ErrCode_NoResponse;
    U16 realVoltage = 0;
    U16 ADValue = 0;
    U8 i;

    ISL_Data_Response data_response = SendReadRequestEx(devAddress, REG_VOLATAGE_ALL, 0, 40, &errCode);
    if(errCode == ErrCode_None) {
        /*  The response Cell Data construct as below */
        /*  CELL12 | CELL11 | CELL10 | CELL9 | CELL8 | CELL7 | CELL6 | CELL5 | CELL4 | CELL3 | CELL2 | CELL1 | VBAT */

        /*  Read first 4 bytes which contains Cell 12 voltage */
        ADValue = Merge14BitDataL4(data_response.L4);
        realVoltage = Convert_CellVoltage(ADValue);
        *(outVoltages + 11) = realVoltage;

        /*  Loop Read left cell voltage data, cell sequence is decrease */
        for(i = 0; i < 11; i++) {
            ADValue = Merge14BitDataL3(data_response.L40.L3Data[i]);
            realVoltage = Convert_CellVoltage(ADValue);
            *(outVoltages + (10 - i)) = realVoltage;
        }
        
        /*  Read VBAT */
        ADValue = Merge14BitDataL3(data_response.L40.L3Data[11]);
        realVoltage = Convert_BatVoltage(ADValue);
        *(outVBAT) = realVoltage;
    }

    return errCode;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Read_ICTemperature                        */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outTemp => Internal IC temp value[out]               */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Read_ICTemperature(U8 devAddress, U16 *outTemp) {
    U16 ADValue = 0;
    ISL_ErrCode errCode = ErrCode_NoResponse;

    ISL_Data_Response data_response = SendReadRequest(devAddress, REG_TEMP_INTERNAL, &errCode);
    if(errCode == ErrCode_None) {
        ADValue = Merge14BitDataL4(data_response.L4);    
        *outTemp = Convert_ADValue_To_ICTemp(ADValue);
    }

    return errCode; 
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Read_ExtTemperature                       */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       index => Ext Temp Index 0...3                        */
/*                       outTemp => Ext1 to Ext4 Temp[out]                    */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Read_ExtTemperature(U8 devAddress, U8 index, U16 *outTemp) {
    U16 ADValue = 0;
    ISL_ErrCode errCode = ErrCode_NoResponse;

    U8 regArr[12] = {REG_TEMP_EXTERNAL1, REG_TEMP_EXTERNAL2, REG_TEMP_EXTERNAL3, REG_TEMP_EXTERNAL4};
    U8 regAddress = regArr[index];

    ISL_Data_Response data_response = SendReadRequest(devAddress, regAddress, &errCode);
    if(errCode == ErrCode_None) {
        ADValue = Merge14BitDataL4(data_response.L4); 
        *outTemp = Convert_ADValue_To_ExtTemp(ADValue);
    }
    
    return errCode; 
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Read_AllTemperatures                      */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outScanCount => Scan count 0 - 15 [out]              */
/*                       outRefVoltage => Reference voltage[out]              */
/*                       outExtTemps => Ext temperatures [out]                */
/*                       outICTemp => IC temperatures [out]                   */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Read_AllTemperatures(U8 devAddress, 
                                            U8 *outScanCount, 
                                            U16 *outRefVoltage, 
                                            U16 *outExtTemps, 
                                            U16 *outICTemp) {
    ISL_ErrCode errCode = ErrCode_NoResponse;
    U16 ADValue = 0;
    U8 i;

    ISL_Data_Response data_response = SendReadRequestEx(devAddress, REG_TEMP_ALL, 0, 22, &errCode);
    if(errCode == ErrCode_None) {
        /*  The response Temp Data construct as below */
        /*  ScanCount | RefVoltage | EXT4 | EXT3 | EXT2 | EXT1 | ICTemp */

        /*  Read first 4 bytes which contains scan count data */
        ADValue = (data_response.L22.L4Data.data2_2 & 0x000F);    
        *(outScanCount) = (U8)ADValue;

        /*  Read first L3 data which is ref voltage */
        ADValue = Merge14BitDataL3(data_response.L22.L3Data[0]);
        *(outRefVoltage) = Convert_ADValue_To_Voltage(ADValue);

        /*  Loop Read Ext temp */
        for(i = 1; i <= 4; i++) {
            ADValue = Merge14BitDataL3(data_response.L22.L3Data[i]);
            *(outExtTemps + (4 - i)) = Convert_ADValue_To_ExtTemp(ADValue);
        }
        
        /*  Read IC Temp */
        ADValue = Merge14BitDataL3(data_response.L22.L3Data[5]);
        *(outICTemp) = Convert_ADValue_To_ICTemp(ADValue);
    }

    return errCode; 
}

/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*               DaisyChain Setup Register Read/Write                                                                                       */
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_OVLimit                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_OVLimit(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_OV_LIMIT, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_OVLimit                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_OVLimit(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_SETUP_OV_LIMIT, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_UVLimit                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_UVLimit(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_UV_LIMIT, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_UVLimit                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_UVLimit(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_SETUP_UV_LIMIT, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_EOTLimit                              */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_EOTLimit(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_EOT_LIMIT, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_EOTLimit                              */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_EOTLimit(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_SETUP_EOT_LIMIT, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_IOTLimit                              */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_IOTLimit(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_IOT_LIMIT, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_BalanceSetup                          */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_BalanceSetup(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_BALANCE, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_BalanceSetup                          */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_BalanceSetup(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_SETUP_BALANCE, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_BalanceStatus                         */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_BalanceStatus(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_BALANCE_STATUS, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_BalanceStatus                         */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_BalanceStatus(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_SETUP_BALANCE_STATUS, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_WD_BalanceTime                        */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_WD_BalanceTime(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_WD_BALANCE_TIME, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_WD_BalanceTime                        */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_WD_BalanceTime(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_SETUP_WD_BALANCE_TIME, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_UserData1                             */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_UserData1(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_USER1, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_UserData1                             */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_UserData1(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_SETUP_USER1, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_UserData2                             */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_UserData2(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_USER2, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_UserData2                             */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_UserData2(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_SETUP_USER2, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_CommsSetup                            */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_CommsSetup(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_COMMS, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_CommsSetup                            */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_CommsSetup(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_SETUP_COMMS, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_DeviceSetup                           */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_DeviceSetup(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_DEVICE, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_DeviceSetup                           */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_DeviceSetup(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_SETUP_DEVICE, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_SN1                                   */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_SN1(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_SN1, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_SN2                                   */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_SN2(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_SN2, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_TrimVoltage                           */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_TrimVoltage(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_SETUP_TRIM_VOLTAGE, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_AllSetup                              */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outSetup => ISL_Setup register data[out]             */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_AllSetup(U8 devAddress, ISL_Setup *outSetup) {
    ISL_ErrCode errCode = ErrCode_NoResponse;

    ISL_Data_Response data_response = SendConfigReadRequestEx(devAddress, REG_SETUP_ALL, 0, 43, &errCode);
    if(errCode == ErrCode_None) {
        /*  The response data orgnized as below: */
        /*  TrimVoltage | SN2 | SN1 | Internal Temp Limit | DeviceSetup | CommsSetup | User2 | */
        /*  User1 | WD-BTM | BalanceStatus | BalanceSetup | EOT | UV | OV */
        outSetup->trim_voltage = Merge14BitDataL4(data_response.L43.L4Data);
        outSetup->sn2 = Merge14BitDataL3(data_response.L43.L3Data[0]);
        outSetup->sn1 = Merge14BitDataL3(data_response.L43.L3Data[1]);
        outSetup->IOT_limit = Merge14BitDataL3(data_response.L43.L3Data[2]);
        outSetup->dev_setup.data = Merge14BitDataL3(data_response.L43.L3Data[3]);
        outSetup->comms_setup.data = Merge14BitDataL3(data_response.L43.L3Data[4]);
        outSetup->user_data2 = Merge14BitDataL3(data_response.L43.L3Data[5]);
        outSetup->user_data1 = Merge14BitDataL3(data_response.L43.L3Data[6]);
        outSetup->wdg_btm_setup.data = Merge14BitDataL3(data_response.L43.L3Data[7]);
        outSetup->bal_status = Merge14BitDataL3(data_response.L43.L3Data[8]);
        outSetup->bal_setup.data = Merge14BitDataL3(data_response.L43.L3Data[9]);
        outSetup->EOT_limit = Merge14BitDataL3(data_response.L43.L3Data[10]);
        outSetup->UV_limit = Merge14BitDataL3(data_response.L43.L3Data[11]);
        outSetup->OV_limit = Merge14BitDataL3(data_response.L43.L3Data[12]);
    }
    
    return errCode;
}

/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*               DaisyChain Fault Register Read/Write                                                                                       */
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_OVFault                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_OVFault(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_FAULT_OV, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_OVFault                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_OVFault(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_FAULT_OV, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_UVFault                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_UVFault(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_FAULT_UV, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_UVFault                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_UVFault(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_FAULT_UV, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_OWFault                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_OWFault(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_FAULT_OW, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name:  DaisyChain_Set_OWFault                              */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_OWFault(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_FAULT_OW, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_FaultSetup                            */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_FaultSetup(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_FAULT_SETUP, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_FaultSetup                            */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_FaultSetup(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_FAULT_SETUP, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_FaultStatus                           */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_FaultStatus(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_FAULT_STATUS, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_FaultStatus                           */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_FaultStatus(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_FAULT_STATUS, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_CellSetup                             */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_CellSetup(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_FAULT_CELL_SETUP, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_CellSetup                             */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_CellSetup(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_FAULT_CELL_SETUP, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_OTFault                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outData => Register Data[out]                        */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_OTFault(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_FAULT_OT, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_OTFault                               */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       data => Register Data to set                         */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Set_OTFault(U8 devAddress, U16 data) {
    return DaisyChain_Set_Config(devAddress, REG_FAULT_OT, data);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_AllFault                              */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outFault => ISL_Fault register data[out]             */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_AllFault(U8 devAddress, ISL_Fault *outFault) {
    ISL_ErrCode errCode = ErrCode_NoResponse;

    ISL_Data_Response data_response = SendConfigReadRequestEx(devAddress, REG_FAULT_ALL, 0, 22, &errCode);
    if(errCode == ErrCode_None) {
        /*  The response data orgnized as below: */
        /*  OT Fault | CellSetup | FaultStatus | FaultSetup | OW Fault | UV Fault | OV Fault */

        outFault->OT_fault.data = Merge14BitDataL4(data_response.L22.L4Data);
        outFault->fault_cell_setup.data = Merge14BitDataL3(data_response.L22.L3Data[0]);
        outFault->fault_status.data = Merge14BitDataL3(data_response.L22.L3Data[1]);
        outFault->fault_setup.data = Merge14BitDataL3(data_response.L22.L3Data[2]);
        outFault->OW_fault = Merge14BitDataL3(data_response.L22.L3Data[3]);
        outFault->UV_fault = Merge14BitDataL3(data_response.L22.L3Data[4]);
        outFault->OV_fault = Merge14BitDataL3(data_response.L22.L3Data[5]);
    }
    
    return errCode;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_CellBalanceValue                      */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       index => Cell Index 0..N                             */
/*                       outData1 => Balance Value H[out]                     */
/*                       outData2 => Balance Value L[out]                     */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_CellBalanceValue(U8 devAddress, U8 index, U16 *outData1, U16 *outData2) {
    ISL_ErrCode errCode = ErrCode_NoResponse;
    U8 regArr[24] = {REG_BV1_L, REG_BV1_H, REG_BV2_L, REG_BV2_H, REG_BV3_L, REG_BV3_H, REG_BV4_L, REG_BV4_H, 
                        REG_BV5_L, REG_BV5_H, REG_BV6_L, REG_BV6_H, REG_BV7_L, REG_BV7_H, REG_BV8_L, REG_BV8_H,
                        REG_BV9_L, REG_BV9_H, REG_BV10_L, REG_BV10_H, REG_BV11_L, REG_BV11_H, REG_BV12_L, REG_BV12_H};

    errCode = DaisyChain_Get_Config(devAddress, regArr[2 * index + 0], outData1);
    errCode = DaisyChain_Get_Config(devAddress, regArr[2 * index + 1], outData2);

    return errCode;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Set_CellBalanceValue                      */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       index => Cell Index 0..N                             */
/*                       data1 => Balance Value H                             */
/*                       data2 => Balance Value L                             */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/

ISL_ErrCode DaisyChain_Set_CellBalanceValue(U8 devAddress, U8 index, U16 data1, U16 data2) {
    ISL_ErrCode errCode = ErrCode_NoResponse;
    U8 regArr[24] = {REG_BV1_L, REG_BV1_H, REG_BV2_L, REG_BV2_H, REG_BV3_L, REG_BV3_H, REG_BV4_L, REG_BV4_H, 
                        REG_BV5_L, REG_BV5_H, REG_BV6_L, REG_BV6_H, REG_BV7_L, REG_BV7_H, REG_BV8_L, REG_BV8_H,
                        REG_BV9_L, REG_BV9_H, REG_BV10_L, REG_BV10_H, REG_BV11_L, REG_BV11_H, REG_BV12_L, REG_BV12_H};

    errCode = DaisyChain_Set_Config(devAddress, regArr[2 * index + 0], data1);
    errCode = DaisyChain_Set_Config(devAddress, regArr[2 * index + 1], data2);

    return errCode;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_RefCoefficientC                       */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outFault => ISL_Fault register data[out]             */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_RefCoefficientC(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_REF_COEFFICIENT_C, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_RefCoefficientB                       */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outFault => ISL_Fault register data[out]             */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_RefCoefficientB(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_REF_COEFFICIENT_B, outData);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: DaisyChain_Get_RefCoefficientA                       */
/* @brief Description  :                                                      */
/* @param parameters   : devAddress => Device Address                         */
/*                       outFault => ISL_Fault register data[out]             */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode DaisyChain_Get_RefCoefficientA(U8 devAddress, U16 *outData) {
    return DaisyChain_Get_Config(devAddress, REG_REF_COEFFICIENT_A, outData);
}

/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*               DaisyChain Devices Function                                                                                                */
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: ISL_DaisyChain_DeviceSetup                           */
/* @brief Description  : Config Intersil 78600 device                         */
/* @param parameters   : devAddress => Device Address(1...N)                  */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode ISL_DaisyChain_DeviceSetup(U8 devAddress) {
    ISL_ErrCode errCode = ErrCode_NoResponse;

    do {
        /* Clear faults */
        errCode = DaisyChain_Set_OVFault(devAddress, 0);
        errCode = DaisyChain_Set_UVFault(devAddress, 0);
        errCode = DaisyChain_Set_OTFault(devAddress, 0);
        errCode = DaisyChain_Set_OWFault(devAddress, 0);
        errCode = DaisyChain_Set_FaultStatus(devAddress, 0);

        /* Set OV,UV,EOT Limit value */
        errCode = DaisyChain_Set_OVLimit(devAddress, OV_LIMIT_VALUE);
        if(errCode != ErrCode_None) {
            break;
        }

        errCode = DaisyChain_Set_UVLimit(devAddress, UV_LIMIT_VALUE);
        if(errCode != ErrCode_None) {
            break;
        }

        errCode = DaisyChain_Set_EOTLimit(devAddress, EOT_LIMIT_VALUE);
        if(errCode != ErrCode_None) {
            break;
        }

//        /* Get default setup data */
//        errCode = DaisyChain_Get_AllSetup(devAddress, &isl_pack.devices[devAddress - 1].setup);
//        if(errCode != ErrCode_None) {
//            break;
//        }

        /* Set balance to manual mode. BMD=0b01(manual mode), BSP=0b0000(manual & timed) */
        /* See detail on isl94212 datasheet page 27 */
        isl_pack.devices[devAddress - 1].setup.bal_setup.mergedBits.BMD = 0x01; // & = 0xfffc;  //0b01, first clear the last two bits.
//				isl_pack.devices[devAddress - 1].setup.bal_setup.mergedBits.BMD |= 0x0001;  //0b01, then set the last two bits.
        isl_pack.devices[devAddress - 1].setup.bal_setup.mergedBits.BSP = 0x00;	//&= 0xfff0;  //0b0000
        errCode = DaisyChain_Set_BalanceSetup(devAddress, isl_pack.devices[devAddress - 1].setup.bal_setup.data);
        if(errCode != ErrCode_None) {
            break;
        }
                                   
        /* Set continuous scan mode parameter */
        /*  SCN         Voltage         Temp            Wire(WSCN=0)        Wire(WSCN=1)*/
        /* 0b0000       16ms            512ms           512ms               512ms       */
        /* 0b0001       32ms            512ms           512ms               512ms       */
        /* 0b0010       64ms            512ms           512ms               512ms       */
        /* 0b0011       128ms           512ms           512ms               512ms       */
        /* 0b0100       256ms           1024ms          512ms               1024ms      */
        /* 0b0101       512ms           2048ms          512ms               2048ms      */
        /* 0b0110       1024ms          4096ms          1024ms              4096ms      */
        isl_pack.devices[devAddress - 1].fault.fault_setup.mergedBits.SCN = 0x03;	//&= 0xf0; // 0b0011,
//				isl_pack.devices[devAddress - 1].fault.fault_setup.mergedBits.SCN |= 0x03; // 0b0011,
        errCode = DaisyChain_Set_FaultSetup(devAddress, isl_pack.devices[devAddress - 1].fault.fault_setup.data);
        if(errCode != ErrCode_None) {
            break;
        }
		
//		/* cell setup to disable the UV/OV/OW test */
//		errCode = DaisyChain_Set_CellSetup(devAddress, 0x2f);
//        if(errCode != ErrCode_None) {
//            break;
//        }

        /* Enter continuous scan mode */
        errCode = DaisyChain_ScanContinuous(devAddress);
        if(errCode != ErrCode_None) {
            break;
        }
        
    } while(errCode != ErrCode_None);

    return errCode;

}
/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Init_ISL_DaisyChain                                  */
/* @brief Description  : Init Intersil 78600 DaisyChain                       */
/* @param parameters   : None                                                 */
/* @return Value       : Operation error code, return 0 if no error.          */
/******************************************************************************/
ISL_ErrCode Init_ISL_DaisyChain(void) {
    ISL_ErrCode errCode = ErrCode_NoResponse;
//    U8 devAddress = 1;
    
    /* Init DaisyChain & Identify all devices */
    errCode = DaisyChain_Identify(NUMBER_OF_AFE_DEVICES);
    
    /* Identify success */
    if(errCode == ErrCode_None) {
		printf(" \nisl942122 identify success! \n");
//        /* Setup device */
//        for(devAddress = 1; devAddress <= NUMBER_OF_AFE_DEVICES; devAddress++) {
//            errCode = ISL_DaisyChain_DeviceSetup(devAddress);
//        }
    }

    return errCode;        
}


/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*               Convenience Mehtod Wrap                                                                                                */
/*------------------------------------------------------------------------------------------------------------------------------------------*/

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Disable_All_Balance                                  */
/* @brief Description  : Set BEN register to 0 for all                        */
/* @param parameters   : None                                                 */
/* @return Value       : None                                                 */
/******************************************************************************/
ISL_ErrCode Disable_All_Balance(void) {
    return DaisyChain_Balance_Disable(DEV_ADDRESS_BROADCAST);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Enable_All_Balance                                  */
/* @brief Description  : Set BEN register to 1 for all                        */
/* @param parameters   : None                                                 */
/* @return Value       : None                                                 */
/******************************************************************************/
ISL_ErrCode Enable_All_Balance(void) {
    return DaisyChain_Balance_Enable(DEV_ADDRESS_BROADCAST);
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Read_all_Voltage	                                  */
/* @brief Description  : read devAddress's cell and batt voltage              */
/* @param parameters   : device number, pointer to store the data             */
/* @return Value       : true=read ok, false=read error                       */
/******************************************************************************/
Bool Read_All_Voltage(U8 devAddress, U16 *outVoltages, U16 *outVBAT){
	U8 i = 0;
	ISL_Data_Response data_response;
	Bool readResult = FALSE;
	U16 ADValue = 0;
	U16 realVoltage = 0;
    U8 regArr[13] = {	REG_VOLATAGE_VBAT, REG_VOLATAGE_CELL1, REG_VOLATAGE_CELL2, REG_VOLATAGE_CELL3, 
                        REG_VOLATAGE_CELL4, REG_VOLATAGE_CELL5, REG_VOLATAGE_CELL6, REG_VOLATAGE_CELL7, 
                        REG_VOLATAGE_CELL8, REG_VOLATAGE_CELL9, REG_VOLATAGE_CELL10, REG_VOLATAGE_CELL11, 
						REG_VOLATAGE_CELL12};
	
	//printf("\nD%d scan start...\n",devAddress);
	DaisyChain_ScanVoltages(devAddress);
	
	/* 0=pack_voltage, 1~12 = cell_1~cell_12 voltage */
	for(i=0;i<=CELL_NUMBER_IN_AFE;i++){
		SendCommandL3(devAddress, ACCESS_READ, PAGE_READ, regArr[i], 0);
		readResult = Spi0_ReadBytes(&data_response.data[0], 4);
		if(readResult != TRUE){
#ifdef VERSION_BMS
			printf("D%d_V%d spi read no response!\n",devAddress,i);
#endif
			return FALSE;
		}
		/* judge that the received data is the related register number. */
		if(	(data_response.L4.devAddress == devAddress) &&
			(data_response.L4.page == PAGE_READ) &&
			(data_response.L4.regAddress == regArr[i]) ){
			
			ADValue = (	(data_response.data[1]&0x03)<<12 | 
						(data_response.data[2]&0xff)<<4	 |
						((data_response.data[3]>>4)&0x0f) );
			if(i==0){
				realVoltage = Convert_BatVoltage(ADValue);
				*(outVBAT) = realVoltage;
#ifndef PRODUCTION_TEST
    #ifdef VERSION_BMS
    				printf("D%d_VBAT=%d\n",devAddress, *(outVBAT));
    #endif
#endif
			}else{
				realVoltage = Convert_CellVoltage(ADValue);
				*(outVoltages + i-1) = realVoltage;
				//printf("D%d_V%d=%d\n",devAddress, i, *(outVoltages + i-1));
			}
		}
		else{
#ifdef VERSION_BMS
			printf("D%d_V%d err\n", devAddress, i);
#endif
			return FALSE;
		}
	}
	return TRUE;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: Read_Pack_Voltage	                                  */
/* @brief Description  : read batt voltage and calculate total voltage		  */
/* @param parameters   : pointer to store the data				              */
/* @return Value       : true=read ok, false=read error                       */
/******************************************************************************/
Bool Read_Pack_Voltage(U16 *outVBAT){
	ISL_Data_Response data_response;
	Bool readResult = FALSE;
	U16 ADValue = 0;
	U16 realVoltage[2];
	
	for(U8 devAddress=1;devAddress<=2;devAddress++){
		DaisyChain_ScanVoltages(devAddress);
		
		/* 0=pack_voltage */
		SendCommandL3(devAddress, ACCESS_READ, PAGE_READ, REG_VOLATAGE_VBAT, 0);
		readResult = Spi0_ReadBytes(&data_response.data[0], 4);
		if(readResult != TRUE){
#ifdef VERSION_BMS
			printf("pack_V spi read no response!\n");
#endif
			return FALSE;
		}
		/* judge that the received data is the related register number. */
		if(	(data_response.L4.devAddress == devAddress) &&
			(data_response.L4.page == PAGE_READ) &&
			(data_response.L4.regAddress == REG_VOLATAGE_VBAT) ){
			
			ADValue = (	(data_response.data[1]&0x03)<<12 | 
						(data_response.data[2]&0xff)<<4	 |
						((data_response.data[3]>>4)&0x0f) );
			realVoltage[devAddress-1] = Convert_BatVoltage(ADValue);
#ifdef VERSION_BMS
			printf("D%d_VBAT=%d\n",devAddress, *(outVBAT));
#endif
		}
		else{
#ifdef VERSION_BMS
			printf("D%d_V_pack err\n", devAddress);
#endif
			return FALSE;
		}
	}
	*(outVBAT) = (U16)(((U32)realVoltage[0]+(U32)realVoltage[1])/10);
	return TRUE;
}

/********************************************************************************/
/**																				*/
/* @brief Function Name: Read_all_Temperature									*/
/* @brief Description  : read devAddress's temperature							*/
/* @param parameters   : device number, pointer to store the data				*/
/* @return Value       : true=read ok, false=read error							*/
/********************************************************************************/
Bool Read_All_Temperature(U8 devAddress, U16 *outTemp){
	U8 i = 0;
	ISL_ErrCode errCode = ErrCode_NoResponse;
	ISL_Data_Response data_response;
	U16 ADValue = 0;
	U8 regArr[4] = {REG_TEMP_EXTERNAL1, REG_TEMP_EXTERNAL2, REG_TEMP_EXTERNAL3, REG_TEMP_EXTERNAL4};
	
	//printf("\nD%d scan temperature start...\n",devAddress);
	DaisyChain_ScanTemps(devAddress);
	delay_ms(3); // MAX 2958 us for isl94212 to settle interval circuit.
	
	/* 0=pack_voltage, 1~12 = cell_1~cell_12 voltage */
	for(i=0;i<CELL_TEMPERATURE_CHANNEL;i++){
		data_response = SendReadRequest(devAddress, regArr[i], &errCode);
		if(errCode == ErrCode_None) {
			ADValue = (	(data_response.data[1]&0x03)<<12 | 
						(data_response.data[2]&0xff)<<4	 |
						((data_response.data[3]>>4)&0x0f) );
			*(outTemp+i) = Convert_ADValue_To_ExtTemp(ADValue);
			//printf("D%d_T%d=%d\n", devAddress, i, *(outTemp+i)-55);
		}else{
			return FALSE;
		}
	}
	return TRUE;
}

/******************************************************************************/
/**                                                                           */
/* @brief Function Name: cell monitor test method,test code for isl94212      */
/* @brief Description  : just for test and debug	                          */
/* @param parameters   : None                                                 */
/* @return Value       : None                                                 */
/******************************************************************************/
static U8 switch_flag = 1;

void Cell_Monitor_Test(void){
	ISL_ErrCode errCode = ErrCode_NoResponse;
	ISL_Command_Response cmd_response;
	U8 devAddress = 1;
	Bool readResult = FALSE;
	
//	Gpiob_SetValue(MASTER_EN,1);		//FGPIOB_PSOR	|= 1<<5; MASTER_EN enable
	OUTPUT_SET(PTE,PTE5);
	switch(switch_flag){
		case 1:
			errCode  = Init_ISL_DaisyChain();
			if(errCode == ErrCode_None){
				for(devAddress = 1; devAddress <= NUMBER_OF_AFE_DEVICES; devAddress++) {
					
					SendCommandL3(devAddress, ACCESS_READ, PAGE_CONFIG, REG_SETUP_COMMS, 0);
					readResult = Spi0_ReadBytes(&cmd_response.data[0], 4);
					printf("REG_SETUP_COMMS=%x\n", cmd_response.data[2]);
					printf("REG_SETUP_COMMS=%x\n", cmd_response.data[3]);					
				}
				switch_flag = 2;
			}else{
				switch_flag = 1;
			}
			break;
		case 2:
			for(devAddress = 1; devAddress <= NUMBER_OF_AFE_DEVICES; devAddress++) {

				readResult = Read_All_Voltage(devAddress, 
											  &isl_pack.devices[devAddress - 1].cellVoltages[0], 
											  &isl_pack.devices[devAddress - 1].batVoltage);
				if(readResult == FALSE){
					break;
				}
				
				readResult = Read_All_Temperature(devAddress, &isl_pack.devices[devAddress-1].extTemps[0]);
				if(readResult == FALSE){
					break;
				}
			}
			break;
		default:
			break;
		//Gpiob_SetValue(MASTER_EN,0);		//FGPIOB_PCOR |= 1<<5;  MASTER_EN disable
	}
}


#include"isl28022.h"
#include"drivers.h"
#include "debug.h"
#include "types.h"
#include "public.h"
#include "soc.h"
/******************************************************************************
* Global variables
******************************************************************************/

/******************************************************************************
* Constants and macros
******************************************************************************/
#define ISL28022_Addr    0x40

#define CURRENT_FULLSCALL (200*1000L)//mA
#define V_FULLSCALL (75*100L)//75mV *100uV

#define V_SHUNT_FULLSCALL_MIN (6)////uV 5
#define V_SHUNT_FULLSCALL_MAX (V_FULLSCALL)

#define SELF_CONSUME  20//mA
#define CURRENT_OFFSET 5000L//500.0A

#define CURRENT_COEFFICIENT  26667UL//(2686L)clude k   (L)

// tCalibration Calibration[CALIBRATION_DOT_NUMBER];
#define CALI_10A         375 //7500*10/200


U16 g_CurrentRegValue=0;
U32 CapacityPercent_1=BATTERY_PERCENT_1;
/******************************************************************************
* Local types
******************************************************************************/

/******************************************************************************
* Local function prototypes
******************************************************************************/
//#define I2C_READ                        0x01              /*!< I2C read */
//#define I2C_WRITE                       0x00                  /*!< I2C write */
//#define I2C_SLAVE_ADDR                  0X40


/******************************************************************************
* Local variables
******************************************************************************/
pt2Func ISL28022_Callback;        /* Pointer to Functions, void argument */


void Isl28022_WrReg(U8 reg, U16 msg)
{

		UINT8 data[3];


		data[0]=reg;
		data[1]=(U8)(msg >> 8);
		data[2]= (U8)(msg & 0xff);//2 bytes context;

		I2C_MasterSendWait(I2C0,0x40,data,3);
		 
	
	
}

void Isl28022_RdReg(U8 reg, U8 *buff)
{
		UINT8 regbuf[2];
		regbuf[0]=reg;
	    // __disable_irq();
		I2C_MasterSendWait(I2C0,0x40,regbuf,1);
		I2C_MasterReadWait(I2C0,0x40,buff,2);	 
        //__enable_irq();
}
/***********************************************************************************************
*
* @brief    ISL28022_Interrupt(void). Function callback called by the FTM module on each timer interrupt.
* @param    none
* @return   none
*
************************************************************************************************/

void ISL28022_Interrupt(void)
{

		Set_IntFlag(ISL28022_CURRENT_INT_POS);

}

/***********************************************************************************************
*
* @brief    ISL28022_SetCallback - Set the callback function to call on successful matches from any channel
* @param    pointer to function with an UINT8 argument
* @return   none
*
************************************************************************************************/

void ISL28022_SetCallback(pt2Func ptr)
{
    ISL28022_Callback = ptr;
}
/***********************************************************************************************
*
* @brief    ISL28022_Calibration - current calibration coefficient adjustment
* @param    pointer to function with an UINT8 argument
* @return   none
*
************************************************************************************************/
void ISL28022_Calibration(U16 RealValue,U8 dotIndex){
     tCalibration *pCalibration=g_static_data.static_data.Calibration;
     pCalibration[dotIndex].MeasureX=(U32)g_CurrentRegValue*CURRENT_COEFFICIENT/1000;
     pCalibration[dotIndex].RealY=RealValue;
	 
	 if(dotIndex!=0&&dotIndex!=ZERO_0A){
	 
	 pCalibration[dotIndex].K =(pCalibration[dotIndex].RealY-pCalibration[dotIndex-1].RealY)*10*1000;
		 
		 if((pCalibration[dotIndex].MeasureX-pCalibration[dotIndex-1].MeasureX)!=0)
           {
		    pCalibration[dotIndex].K=pCalibration[dotIndex].K/(pCalibration[dotIndex].MeasureX-pCalibration[dotIndex-1].MeasureX);
		   }
	     if(pCalibration[dotIndex].K>1200||pCalibration[dotIndex].K<500)
           {
	        pCalibration[dotIndex].K=1000;//1.000
	       }
	   pCalibration[dotIndex].B =pCalibration[dotIndex].RealY*10*1000-pCalibration[dotIndex].K*pCalibration[dotIndex].MeasureX; //b*1000
 
	 }
	
}
/***********************************************************************************************
*
* @brief    ISL28022_Calibration - current calibration coefficient adjustment
* @param    pointer to function with an UINT8 argument
* @return   none
*
************************************************************************************************/
U32 CalibrationCurrent(U16 CurrentRegisterValue,tCalibration *Dot,U16 scale)
{
	
     U32 CurrentCalibration=0,line_B=0;
	
	 if(scale==0){return 0;}
     if(Dot->K<500||Dot->K>2000){Dot->K=1000;}
	 //if(Dot->B>1000000||Dot->B<-1000000){Dot->B=0;}
	 if(Dot->B<=1000000&&Dot->B>0)
	 { 
	  CurrentCalibration=((S64)CurrentRegisterValue* CURRENT_COEFFICIENT*Dot->K+Dot->B*1000)/1000/scale;
     }else if(Dot->B>=-1000000&&Dot->B<0){
	 line_B=-Dot->B;
	  CurrentCalibration=((S64)CurrentRegisterValue* CURRENT_COEFFICIENT*Dot->K-line_B*1000)/1000/scale; 
	 }else
	 {
	  CurrentCalibration=((S64)CurrentRegisterValue* CURRENT_COEFFICIENT*Dot->K)/1000/scale; 
	 }
	return CurrentCalibration; 
	
}
/*****************************************************************************//*!
   *
   * @brief Read ISL28022 Register.
   * @param[in]  tISL28022Register   register enum type.
   * @param[out]  UINT8* poutbuf  output data.
   * @return error status
   *
*****************************************************************************/
void ISL28022_proc(void)
{

     U8 buf[2] ;
     U16 CurrentValue=0;
	 
	 tBatteryOfSocProperty *pBatteryOfSoc=&g_stateblock_data.stata_union.state_data.BatteryOfSoc;
     Sample_Data *psample = &g_battery_pack_info.sample_data;
     tCalibration *pCalibration=g_static_data.static_data.Calibration;
	
    Isl28022_RdReg(ShuntVReg, buf);
    CurrentValue = buf[0];
	CurrentValue <<=8;
	CurrentValue |= buf[1];
	//printf("\nShuntVReg=%d ",CurrentValue);
	
	  psample->current_direction = Current_Direction_Discharge;
     if(CurrentValue&0XE000)
	{
	     CurrentValue =-CurrentValue;
	     psample->current_direction = Current_Direction_Charge; 
	}
    g_CurrentRegValue=CurrentValue;
  
    if(CurrentValue > V_SHUNT_FULLSCALL_MIN && CurrentValue <V_SHUNT_FULLSCALL_MAX)
			{
         
				if(psample->current_direction == Current_Direction_Discharge)
				{

			
			      if(CurrentValue<CALI_10A)
					{
					psample->total_current =CalibrationCurrent(CurrentValue,&pCalibration[DISCHARGE_10A],1000)/100;	
					pBatteryOfSoc->ReadCurrentSum -=CalibrationCurrent(CurrentValue,&pCalibration[DISCHARGE_10A],1000);//discharge sub	
					}
					else if(CurrentValue>=CALI_10A)
					{
					psample->total_current =CalibrationCurrent(CurrentValue,&pCalibration[DISCHARGE_100A],1000)/100;
					pBatteryOfSoc->ReadCurrentSum  -=CalibrationCurrent(CurrentValue,&pCalibration[DISCHARGE_100A],1000);//discharge sub	
					} 
			        
					
				}
				else
				{

					psample->total_current =CalibrationCurrent(CurrentValue,&pCalibration[CHARGE_40A],1000)/100;// 
			        pBatteryOfSoc->ReadCurrentSum +=CalibrationCurrent(CurrentValue,&pCalibration[CHARGE_40A],1000);//charge add

				}			 
 
		 
    } else {

				psample->current_direction = Current_Direction_None;
				psample->total_current = 0;
				CurrentValue=0;//
    }
	
	   //printf(" total_current=%d\n",psample->total_current);
	  if(psample->current_direction == Current_Direction_Discharge)
			{
			 psample->total_current+=CURRENT_OFFSET;
			}
			else
			{
			 psample->total_current=CURRENT_OFFSET-psample->total_current;
			}
	
 
		
	   Set_IntFlag(SOC_PROC_POS);

}
Current_Direction ISL28022_SOC_proc(void){
	U8  soc;
	Current_Direction diret;
	U16 selfconsume=0;
	Sample_Data *psample = &g_battery_pack_info.sample_data;
	S32 ReadCurrentSum=0;
	
	diret=psample->current_direction;
	if(g_stateblock_data.stata_union.state_data.BatteryOfSoc.ReadCurrentSum==0) return Current_Direction_None;
		
	
	if(g_stateblock_data.stata_union.state_data.BatteryOfSoc.ReadCurrentSum<0)//discharge
		{
			selfconsume=SELF_CONSUME;
			ReadCurrentSum=0-g_stateblock_data.stata_union.state_data.BatteryOfSoc.ReadCurrentSum; 
			diret=Current_Direction_Discharge;	 
		}else{
			ReadCurrentSum=g_stateblock_data.stata_union.state_data.BatteryOfSoc.ReadCurrentSum;	
			diret=Current_Direction_Charge;			 
		}
		 
        soc = (ReadCurrentSum+ selfconsume)/ CapacityPercent_1;
		 
        CalcSOC(diret, soc);
		if(g_stateblock_data.stata_union.state_data.BatteryOfSoc.ReadCurrentSum<0)
			{ 
				g_stateblock_data.stata_union.state_data.BatteryOfSoc.ReadCurrentSum =0-(ReadCurrentSum% CapacityPercent_1);
			}
			else{

				g_stateblock_data.stata_union.state_data.BatteryOfSoc.ReadCurrentSum =ReadCurrentSum% CapacityPercent_1;
			}		 
       
     return diret;



}

/*****************************************************************************//*!
   *
   * @brief Initialize ISL28022 module.

   * @return none
   *
   * @ Pass/ Fail criteria: none
*****************************************************************************/
int ISL28022_Init(void)
{
		U8 buf[2];
		U32 CapacityPercent=0;
		g_battery_pack_info.sample_data.total_current=CURRENT_OFFSET; 
	
		g_battery_pack_info.SOC=g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC; 
		CapacityPercent=g_stateblock_data.stata_union.state_data.BatteryOfSoc.NowCapacity*360L;//NowCapacity*3600*10L/100;
	
		if(CapacityPercent>=BATTERY_PERCENT_MAX||CapacityPercent<=BATTERY_PERCENT_MIN)
		{			
		 CapacityPercent_1=BATTERY_PERCENT_1;		
		}	
		else
		{
		 CapacityPercent_1=CapacityPercent;
		}	
	
	
	
		U16 Value = 0xfff8;//powerdown mode
		Isl28022_WrReg(ConfigReg, Value);
		delay_ms(100);
		ISL28022_Reset();
		delay_ms(100);
		Isl28022_WrReg(ConfigReg, CONFIGREG_VALUE);
		Isl28022_RdReg(ConfigReg, buf);
		Value = ((U16)buf[0]) << 8 | buf[1];
		delay_ms(100);
		if(Value != CONFIGREG_VALUE) 
			{
		    //PRINTF("Isl28022 init error Value=%X",Value);
	
		    return ErrCode_WR;
		   }
		g_soc_status=SOC_COUNTER;	
		ISL28022_SetCallback(ISL28022_proc);
			 
  	NVIC_EnableIRQ(PIT_CH0_IRQn);
	
	  PIT_ChannelEnableInt(0);


			 
		return ErrCode_No;


}

/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2013 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
***************************************************************************//*!
*
* @file i2c.c
*
* @author Freescale
*
* @version 0.0.1
*
* @date Jun. 25, 2013
*
* @brief providing APIs for configuring I2C module (I2C).
*
*******************************************************************************
*
* provide APIs for configuring I2C module (I2C).
******************************************************************************/


#include "isl28022.h"
#include "isl94212.h"
#include "soc.h"
#include "math.h"
#include "debug.h"
#include "public.h"
#include "pit.h"
#include "w25q64.h"
/******************************************************************************
* Global variables
******************************************************************************/
// tBatteryOfSocProperty g_BatteryOfSoc;  move to g_stateblock_data.stata_union.state_data.BatteryOfSoc


tSOC_STATUS g_soc_status;

/******************************************************************************
* Constants and macros
******************************************************************************/
#define VEHICLE_STATIC_TIME_LIMIT    2*3600ul// unit s

#define TEMPRATURE_OFFSET 40
#define Vshunt_FS    75*1000ul //80mV *1000
#define Current_FS    200 //200A *1000
#define Rshunt     Vshunt_FS/Current_FS //uint vohm 


#define SOC_OFFSET  0 //mV maxCellV-offset
//#define RealCellAvg  (g_battery_pack_info.sample_result.ave_cell_voltage-SOC_OFFSET)

#define SOC_ARRAY_RANGE  21
#define SOC_TEMPRATRUE_RANGE 7

#define CURRENT_ZERO_TIMEOUT   3600*10L//1 hours

#define ABS(x,y)  (x)>(y)? ((x)-(y)):((y)-(x))

/******************************************************************************
* Local types
******************************************************************************/

/******************************************************************************
* Local function prototypes
******************************************************************************/

/******************************************************************************
* Local variables
******************************************************************************/

const U16 SOC_OCV[SOC_ARRAY_RANGE][SOC_TEMPRATRUE_RANGE]={
//-10C      0C     15C     25C     30C      45C     55C                                             
{41796,    41977	,41946	,41799	,41975	,41818	,41965 },//100%
{41110	,  41126	,41139	,41143	,41175	,41130	,41145 },// 95%
{40425	,  40434	,40474	,40521	,40488	,40516	,40495 },// 90%
{39870	,  39875	,39957	,39986	,39944	,40029	,39945 },// 85%
{39357	,  39372	,39438	,39425	,39442	,39515	,39420 },// 80%
{38937	,  38910	,38944	,38945	,38949	,38997	,38939 },// 75%
{38442	,  38471	,38546	,38576	,38551	,38551	,38558 },// 70%
{37925	,  38042	,38088	,37975	,38069	,38105	,38135 },// 65%
{37601	,  37597	,37672	,37532	,37717	,37655	,37770 },// 60%
{37198	,  37194	,37252	,37429	,37315	,37277	,37365 },// 55%
{36948	,  36948	,36995	,36936	,37006	,36990	,37092 },// 50%
{36715	,  36753	,36780  ,36734	,36814	,36804	,36871 },// 45%
{36538	,  36572	,36614	,36563	,36602	,36646	,36693 },// 40%
{36437	,  36380  ,36423	,36472	,36474	,36506	,36537 },// 35%
{36243	,  36252	,36307	,36294	,36324	,36351	,36407 },// 30%
{36006	,  36074	,36103	,36101	,36145	,36041	,36188 },// 25%
{35775	,  35813	,35846	,35790	,35870  ,35796	,35897 },// 20%
{35510	,  35690  ,35739	,35509	,35807	,35514	,35654 },// 15%
{35349	,  35177	,35276	,35105	,35337	,35095	,35346 },// 10%
{34891	,  34817	,34848	,34712	,34915	,34711	,34873 },// 05%
{34111	,  34595	,34464	,34520	,34549	,34001	,34694 } //  0%
};
const S8 BatteryTempratrue[] = { -10 + TEMPRATURE_OFFSET, 0 + TEMPRATURE_OFFSET, 15 + TEMPRATURE_OFFSET, 25 + TEMPRATURE_OFFSET, 30 + TEMPRATURE_OFFSET, 45 + TEMPRATURE_OFFSET, 55 + TEMPRATURE_OFFSET};
/******************************************************************************
* Local functions
******************************************************************************/


/******************************************************************************
* Global functions
******************************************************************************/


/***********************************************************************************************
*
* @brief    main() - Program entry function
* @param    none
* @return   none
*
************************************************************************************************/  
void GetPackVoltageProcess(U32 cellVoltage,U32 temp){
	        U8 soc;
			g_stateblock_data.stata_union.state_data.BatteryOfSoc.NowOCV=cellVoltage;
			g_stateblock_data.stata_union.state_data.BatteryOfSoc.NowTempratrue=temp;
			soc=FindTableSOCvalue(cellVoltage,temp);
	        if(g_battery_pack_info.sample_data.current_direction== Current_Direction_Discharge)
			{
			g_stateblock_data.stata_union.state_data.BatteryOfSoc.DischargePercent+=
				soc<=g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC ? g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC-soc:0;
			}
			g_battery_pack_info.SOC=g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC=soc; 
}

/***********************************************************************************************
*
* @brief    main() - Program entry function
* @param    none
* @return   none
*
************************************************************************************************/  
void InitMetering(void){
	U16 RealCellAvg=0,count=3;
	U8 temprature=0;
	while(count--)
		{
	
         if(Read_Pack_Voltage(&RealCellAvg))
		 {
			 temprature=g_battery_pack_info.sample_result.ave_cell_temperature;
		  if(temprature==0){
			  temprature=25+TEMPRATURE_OFFSET;
		     } 
		  GetPackVoltageProcess((U32)RealCellAvg*10/19,temprature);
		  break;
		  }
         }
	NVIC_EnableIRQ(PIT_CH0_IRQn);
	
	PIT_ChannelEnableInt(0);
	
	g_stateblock_data.stata_union.state_data.BatteryOfSoc.ZeroCurrentCounter=0; 
	
	Clear_IntFlag(SOC_CALIBRATION_FLAG_POS);
	
#ifdef VERSION_BMS
	W25Q64_SetStateMSG(&g_stateblock_data);
#endif

}
/***********************************************************************************************
*
* @brief    main() - Program entry function
* @param    none
* @return   none
*
************************************************************************************************/  
void CalcSOC(U32 direction, S32 soc_per_x){
	

	//SOCt=SOC0 - integration(0,t)Idt

		if(direction==Current_Direction_Discharge){

			g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC=g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC>=soc_per_x ? (g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC-soc_per_x): g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC;
			g_stateblock_data.stata_union.state_data.BatteryOfSoc.DischargePercent+=soc_per_x;

		}
		else if (direction==Current_Direction_Charge){

			g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC=g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC<100 ? (g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC+soc_per_x): 100;	
		}
		
		g_battery_pack_info.SOC=g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC; 

		//printf("g_battery_pack_info.SOC=%d\n",g_battery_pack_info.SOC);
}
/***********************************************************************************************
*
* @brief    FSM_SOC_Proc() FSM is funcation sataus machine
* @param    none
* @return   none
*
************************************************************************************************/ 

void FSM_SOC_Proc(void){
	
Current_Direction current_dirt;
static U32 TempSoc = 0;
	
	switch(g_soc_status)
	{
	
		case SOC_INIT:
		if(Get_IntFlag(SOC_CALIBRATION_FLAG_POS)) 
		{
			InitMetering();
			g_soc_status=SOC_COUNTER;
		}
		    
			break;
		
		case SOC_COUNTER:
			
			if(Get_IntFlag(SOC_PROC_POS))
			{
			 Clear_IntFlag(SOC_PROC_POS);
			 current_dirt=ISL28022_SOC_proc();
				
				if(current_dirt==Current_Direction_None)
				{
				
					g_stateblock_data.stata_union.state_data.BatteryOfSoc.ZeroCurrentCounter++;//if current is zero timeout, initial soc is new value;
                    
					if(g_stateblock_data.stata_union.state_data.BatteryOfSoc.ZeroCurrentCounter>=CURRENT_ZERO_TIMEOUT) 
						{
							
						 g_soc_status=SOC_INIT;
						 g_stateblock_data.stata_union.state_data.BatteryOfSoc.ZeroCurrentCounter=0;		
							
						}
          

					
				}else
				{
					
				 g_stateblock_data.stata_union.state_data.BatteryOfSoc.ZeroCurrentCounter=0;
				
				}
			}

			break;	
		default:
			break;
		
	}

	if(g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC != TempSoc){
		TempSoc = g_stateblock_data.stata_union.state_data.BatteryOfSoc.SOC;
		W25Q64_SetStateMSG(&g_stateblock_data);
	}
	
	
}



/***********************************************************************************************
*
* @brief    CalcCycleCount()
* @param    none
* @return   none
*
************************************************************************************************/  
void CalcCycleCount(void){
	
	if(g_stateblock_data.stata_union.state_data.BatteryOfSoc.DischargePercent<999900)//max 9999
	{
		g_stateblock_data.stata_union.state_data.BatteryOfSoc.CycleCount=g_stateblock_data.stata_union.state_data.BatteryOfSoc.DischargePercent/100;
	}
	else{
	    g_stateblock_data.stata_union.state_data.BatteryOfSoc.CycleCount=0;
	}
}
/***********************************************************************************************
*
* @brief    FindTableSOCvalue()
* @param    none
* @return   none
*
************************************************************************************************/  
int FindTableSOCvalue(unsigned int value,char tempc)
{   int mid,start=0,end=SOC_ARRAY_RANGE,soc_val=0;
	  int tp=0,i;
		unsigned int diff=0,diff_temp=0;
	  for(i=0;i<sizeof(BatteryTempratrue);i++)
    {
		 if(tempc<=BatteryTempratrue[i]){tp=i;break;}
		
		}
		if(tempc>BatteryTempratrue[sizeof(BatteryTempratrue)-1]){tp=sizeof(BatteryTempratrue)-1;}
    
	  value*=10;
    while(start<=end)
		{
        mid=start+(end-start)/2;
        if(SOC_OCV[mid][tp]==value)
        return mid;

        else if(value<SOC_OCV[mid][tp])
        {
         start=mid+1; 
        }
        else
        {
         end=mid-1;
        }
		
	  }
		
		soc_val=(100-mid*5);

		if(mid>=0){
		if(value>SOC_OCV[mid][tp])
     {
			  diff=ABS(SOC_OCV[mid][tp],SOC_OCV[mid-1][tp]); //
			
			  diff_temp=(value- SOC_OCV[mid][tp] );
			 
			  soc_val+= diff_temp/(diff/5);
		
		 }else
		 {
		 
		    diff= ABS(SOC_OCV[mid][tp],SOC_OCV[mid+1][tp]);//
		    diff_temp=(SOC_OCV[mid][tp] - value);
			 
			  soc_val-= diff_temp/(diff/5);
			 
		 }					
		}	
	  return soc_val>=0 ? soc_val:0;	
	
}



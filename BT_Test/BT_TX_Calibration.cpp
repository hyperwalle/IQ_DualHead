#include "stdafx.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
//#include <stdio.h>
//#include <time.h>
//Move to stdafx.h
//#include "lp_highrestimer.h"
#include <math.h>
//#include <Windows.h>
#include <algorithm>
#include <functional>

// This global variable is declared in BT_Test_Internal.cpp
extern vDUT_ID      g_BT_Dut;
extern TM_ID        g_BT_Test_ID;

using namespace std;

// Input Parameter Container
map<string, BT_SETTING_STRUCT> l_txCalParamMap;

// Return Value Container 
map<string, BT_SETTING_STRUCT> l_txCalReturnMap;

extern  BT_GLOBAL_SETTING g_BTGlobalSettingParam;

double	l_pathLossRecord = 0;

struct tagParam
{
    // Mandatory Parameters
    double CABLE_LOSS_DB;							/*! The path loss of test system. Default=0 dB */
    double SAMPLING_TIME_US;                        /*! The sampling time to do measurement. Default=50us */ 

    //DUT Parameters
    double TX_CAL_TIMEOUT_MS;                       /*! The timeout to do TX calibration. Default=600s */ 
} l_txCalParam;

struct tagReturn
{
	double   CABLE_LOSS_DB;						/*! The path loss of test system. */
	char     CAL_RESULT[MAX_BUFFER_SIZE];
    char     ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txCalReturn;

#ifndef WIN32
int initTXCalContainers = InitializeTXCalContainers();
#endif

int ClearTxCalReturn(void)
{
	l_txCalParamMap.clear();
	l_txCalReturnMap.clear();
	return 0;
}

//! BT TX Calibration
/*!
* Input Parameters
*
*  - Mandatory 
*      -# CABLE_LOSS_DB (double): The path loss of test system.
*      -# SAMPLING_TIME_US (double): The data rate to do measurement.
*
* Return Values
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/

//#define TX_CALIBRATION_DEBUG
#ifdef TX_CALIBRATION_DEBUG
char g_strMsg[MAX_BUFFER_SIZE] = "";
#endif //TX_CALIBRATION_DEBUG

highrestimer::lp_time_t start_time, stop_time;
int l_iEclipseTimeMs;
typedef struct tagAnalysisInputParameterType
{
	int       FREQ_MHZ;								   /*! The center frequency (MHz). */	
	int       VSA_TRIGGER_TYPE;
    double    TX_POWER_DBM;    
	double	  SAMPLING_TIME_US;
	char	  MEASURE_TYPE[MAX_BUFFER_SIZE];

}tagAnalysisInputParameter;

typedef struct tagAnalysisOutputType
{
    // POWER Test Result 
    double   POWER_AVG;
	double	 FREQUENCY;
    char     ERROR_MESSAGE[MAX_BUFFER_SIZE];
}tagAnalysisOutput;

#define DEFAULT_TIMEOUT_MS 600000                     //default timeout value: 600s 

int TX_Cal_GetPowerAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput);
int TX_Cal_GetCWAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput);

BT_TEST_API int BT_TX_Calibration(void)
{
    int		err = ERR_OK;
    int		dummyValue = 0;    
	char    logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
    * Clear Return Parameters and Container *
    *---------------------------------------*/
    ClearReturnParameters(l_txCalReturnMap);

    /*------------------------*
    * Respond to QUERY_INPUT *
    *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err)
    {
        RespondToQueryInput(l_txCalParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
    * Respond to QUERY_RETURN *
    *-------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err)
    {
        RespondToQueryReturn(l_txCalReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try 
	{
	   /*-----------------------------------------------------------*
		* Both g_BT_Test_ID and g_BT_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_BT_Test_ID<0 || g_BT_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] BT_Test_ID or BT_Dut not valid. BT_Test_ID = %d and BT_Dut = %d.\n", g_BT_Test_ID, g_BT_Dut);
			throw logMessage;
		}
		else
		{
			// do nothing
		}
		TM_ClearReturns(g_BT_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_txCalParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not completed.\n");
			throw logMessage;
		}		
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Get input parameters return OK.\n");
		}


		/*------------------------*
		*   Do Calibration        *
		*-------------------------*/
	#pragma region configure DUT to calibration and begin timer
		err = vDUT_Run(g_BT_Dut, "GET_CAL_SETTING");
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(GET_CAL_SETTING) return error.\n");
			throw logMessage;
		}			
		else
		{			
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_Run(GET_CAL_SETTING) return OK.\n");
			// TODO: Get parameters here
		}

        highrestimer::GetTime(start_time);  
	#pragma endregion	

	#pragma region measurement till done
		int    iCalDone      = 0;
		int    iCalAbort     = 0;
		tagAnalysisInputParameter   tagInputPara;
		tagAnalysisOutput           tagOutputPara;
		tagInputPara.FREQ_MHZ         = 2412; 
		tagInputPara.TX_POWER_DBM     = 0; 
		tagOutputPara.POWER_AVG = NA_NUMBER;

		while(1)
		{
            highrestimer::GetTime (stop_time);

			l_iEclipseTimeMs = highrestimer::GetElapsedMSec(start_time, stop_time);

			if (l_iEclipseTimeMs>l_txCalParam.TX_CAL_TIMEOUT_MS)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] TX power calibration timeout! Abort.\n");				
				throw logMessage;
			}

			//Set calibration point and transmit
			err = vDUT_Run(g_BT_Dut, "SET_CAL_MEASUREMENT");
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(SET_CAL_MEASUREMENT) return error.\n");
				throw logMessage;
			}			
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_Run(SET_CAL_MEASUREMENT) return OK.\n");
			}

			//Get the measure parameter from DUT control layer
			err = ::vDUT_GetIntegerReturn(g_BT_Dut, "CAL_DONE",  (int*)&iCalDone);	
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_GetIntegerReturn(CAL_DONE) return error.\n");
				throw logMessage;
			}				
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_GetIntegerReturn(CAL_DONE=%d) return OK.\n", iCalDone);
			}
			if(iCalDone)
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Dut report Calibration Done.\n");
				break;
			}
			//Get the measure parameter from DUT control layer
			err = ::vDUT_GetIntegerReturn(g_BT_Dut, "CAL_ABORT",  (int*)&iCalAbort);	
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_GetIntegerReturn(CAL_ABORT) return error.\n");
				throw logMessage;
			}				
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_GetIntegerReturn(CAL_ABORT=%d) return OK.\n", iCalAbort);
			}
			if(iCalAbort)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Dut report Abort Calibration.\n");
				throw logMessage;
			}

			/*--------------------------------------------------------*/
			/*    Get the measure parameter from DUT control layer    */
			/*--------------------------------------------------------*/
			//frequency
			err = ::vDUT_GetIntegerReturn(g_BT_Dut, "FREQ_MHZ",  &tagInputPara.FREQ_MHZ);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_GetIntegerReturn(FREQ_MHZ) return error.\n");
				throw logMessage;
			}				
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_GetIntegerReturn(FREQ_MHZ=%d) return OK.\n", tagInputPara.FREQ_MHZ);
			}

			// TX power
			err = ::vDUT_GetDoubleReturn(g_BT_Dut, "TX_POWER_DBM",  &tagInputPara.TX_POWER_DBM);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_GetIntegerReturn(TX_POWER_DBM) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_GetIntegerReturn(TX_POWER_DBM=%.1f) return OK.\n", tagInputPara.TX_POWER_DBM);
			}

			// VSA TRIGGER TYPE
			err = ::vDUT_GetIntegerReturn(g_BT_Dut, "VSA_TRIGGER_TYPE",  &tagInputPara.VSA_TRIGGER_TYPE);
			if ( ERR_OK!=err )
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				tagInputPara.VSA_TRIGGER_TYPE = g_BTGlobalSettingParam.VSA_TRIGGER_TYPE;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[BT] vDut not support parameter (VSA_TRIGGER_TYPE), using default Global setting = %d.\n", g_BTGlobalSettingParam.VSA_TRIGGER_TYPE);
			}	
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_GetIntegerReturn(VSA_TRIGGER_TYPE=%d) return OK.\n", tagInputPara.VSA_TRIGGER_TYPE);
			}

			// SAMPLING TIME US
			err = ::vDUT_GetDoubleReturn(g_BT_Dut, "SAMPLING_TIME_US",  &tagInputPara.SAMPLING_TIME_US);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				tagInputPara.SAMPLING_TIME_US = l_txCalParam.SAMPLING_TIME_US;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[BT] vDut not support parameter (SAMPLING_TIME_US), using flow file setting = %.1f\n", l_txCalParam.SAMPLING_TIME_US);
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_GetIntegerReturn(SAMPLING_TIME_US=%.1f) return OK.\n", tagInputPara.SAMPLING_TIME_US);
			}

			// MEASURE TYPE
			err = ::vDUT_GetStringReturn(g_BT_Dut, "MEASURE_TYPE", tagInputPara.MEASURE_TYPE, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				strcpy_s( tagInputPara.MEASURE_TYPE, MAX_BUFFER_SIZE, "POWER" );
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[BT] vDUT_GetStringReturn(MEASURE_TYPE) return error, set it to default MEASURE_TYPE = POWER.\n");
				//throw logMessage;  // Don't return err, for backward compatible.
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_GetStringReturn(MEASURE_TYPE=%s) return OK.\n", tagInputPara.MEASURE_TYPE);
			}

			// Check path loss (by ant and freq)
			if ( 0==l_txCalParam.CABLE_LOSS_DB )
			{
				err = TM_GetPathLossAtFrequency(g_BT_Test_ID, tagInputPara.FREQ_MHZ, &l_pathLossRecord, 0, TX_TABLE);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to get CABLE_LOSS_DB from path loss table, err = %d.\n", err);
					throw logMessage;
				}						
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] TM_GetPathLossAtFrequency(cableLossDb=%.1f) return OK.\n", l_pathLossRecord);
				}
			}
			else
			{
				l_pathLossRecord = l_txCalParam.CABLE_LOSS_DB;
			}

			/*--------------------------------*/
			/*    Start Do the Measurement    */
			/*--------------------------------*/
			if(strcmp(tagInputPara.MEASURE_TYPE,"POWER")==0)
			{
			   /*-----------------------*
				*   Power Measurement   *
				*-----------------------*/
				err = TX_Cal_GetPowerAnalysisResult(tagInputPara, &tagOutputPara); 
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] TX_Cal_GetPowerAnalysisResult() return error.\n");
					throw logMessage;
				}			
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] TX_Cal_GetPowerAnalysisResult() return OK.\n");
				}

				//input the measure power value to DUT control
				vDUT_ClearParameters(g_BT_Dut);
				err = vDUT_AddDoubleParameter(g_BT_Dut, "MEAS_POWER_DBM",  tagOutputPara.POWER_AVG);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_AddDoubleParameter(MEAS_POWER_DBM) return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_AddDoubleParameter(MEAS_POWER_DBM=%.1f) return OK.\n", tagOutputPara.POWER_AVG);
				}
			}
			if(strcmp(tagInputPara.MEASURE_TYPE,"CW_FREQ")==0)
			{
			   /*----------------------*
				*  CW Frequency Error  *
				*----------------------*/
				err = TX_Cal_GetCWAnalysisResult(tagInputPara, &tagOutputPara); 
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] TX_Cal_GetCWAnalysisResult() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] TX_Cal_GetCWAnalysisResult() return OK.\n");
				}

				//input the measure result to DUT control
				vDUT_ClearParameters(g_BT_Dut);
				err = vDUT_AddDoubleParameter(g_BT_Dut, "FREQUENCY",  tagOutputPara.FREQUENCY);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_AddDoubleParameter(FREQUENCY) return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_AddDoubleParameter(FREQUENCY=%.1f) return OK.\n", tagOutputPara.FREQUENCY);
				}
			}				
		}
	#pragma endregion	

		/*-----------------------*
		*  Return Test Results  *
		*-----------------------*/
		if (ERR_OK==err)
		{
			// Return Path Loss (dB)
			l_txCalReturn.CABLE_LOSS_DB = l_pathLossRecord;
			vDUT_GetStringReturn(g_BT_Dut, "CAL_RESULT", l_txCalReturn.CAL_RESULT,MAX_BUFFER_SIZE);

			sprintf_s(l_txCalReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txCalReturnMap);
		} 
		else
		{
			// do nothing
		}

	}
	catch(char *msg)
    {
		vDUT_Run(g_BT_Dut, "CALIBRATION_FAILED");	 // Clear calibration status. Doesn't need to do error check, in case some other dut control doesn't support this function.
        ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		vDUT_Run(g_BT_Dut, "CALIBRATION_FAILED");	 // Clear calibration status. Doesn't need to do error check, in case some other dut control doesn't support this function.
		ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
    }


    return err;
}

int InitializeTXCalContainers(void)
{
    /*------------------*
    * Input Paramters: *
    * IQTESTER_IP01    *
    *------------------*/
    l_txCalParamMap.clear();

    BT_SETTING_STRUCT setting;

    l_txCalParam.SAMPLING_TIME_US = 50.0;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txCalParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&l_txCalParam.SAMPLING_TIME_US;
        setting.unit        = "uS";
        setting.helpText    = "Capture time in micro-seconds";
        l_txCalParamMap.insert( pair<string,BT_SETTING_STRUCT>("SAMPLING_TIME_US", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txCalParam.CABLE_LOSS_DB = 0.0;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txCalParam.CABLE_LOSS_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_txCalParam.CABLE_LOSS_DB;
        setting.unit        = "dB";
        setting.helpText    = "Cable loss from the DUT antenna port to tester";
        l_txCalParamMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txCalParam.TX_CAL_TIMEOUT_MS = DEFAULT_TIMEOUT_MS;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txCalParam.TX_CAL_TIMEOUT_MS))    // Type_Checking
    {
        setting.value = (void*)&l_txCalParam.TX_CAL_TIMEOUT_MS;
        setting.unit        = "MS";
        setting.helpText    = "timeout when calibration fail";
        l_txCalParamMap.insert( pair<string,BT_SETTING_STRUCT>("TX_CAL_TIMEOUT_MS", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    /*----------------*
    * Return Values: *
    * ERROR_MESSAGE  *
    *----------------*/
    l_txCalReturnMap.clear();

	l_txCalReturn.CABLE_LOSS_DB = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txCalReturn.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txCalReturn.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_txCalReturnMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txCalReturn.CAL_RESULT[0] = '\0';
	setting.type = BT_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txCalReturn.CAL_RESULT))    // Type_Checking
	{
		setting.value       = (void*)l_txCalReturn.CAL_RESULT;
		setting.unit        = "";
		setting.helpText    = "Calibration Result";
		l_txCalReturnMap.insert( pair<string,BT_SETTING_STRUCT>("CAL_RESULT", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_txCalReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txCalReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txCalReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_txCalReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txCalReturn.CAL_RESULT[0] = '\0';
	setting.type = BT_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txCalReturn.CAL_RESULT))    // Type_Checking
	{
		setting.value       = (void*)l_txCalReturn.CAL_RESULT;
		setting.unit        = "";
		setting.helpText    = "Calibration Result";
		l_txCalReturnMap.insert( pair<string,BT_SETTING_STRUCT>("CAL_RESULT", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    return 0;
}

//This is the function to do analysis during TX calibration and provide the result to the DUT control layer
int TX_Cal_GetPowerAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput)
{
    int    err = ERR_OK;
    int    avgIteration = 0;
    double pwr_sum  = 0;
    double meas_pwr = 0;
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};

	tagOutput->POWER_AVG = NA_NUMBER;

	/*--------------------*
	 * Setup IQtester VSA *
	 *--------------------*/
	double peakToAvgRatio = g_BTGlobalSettingParam.IQ_P_TO_A_BDR;

	err = LP_SetVsaBluetooth(  tagInput.FREQ_MHZ*1e6,
							   tagInput.TX_POWER_DBM-l_pathLossRecord+peakToAvgRatio,
							   g_BTGlobalSettingParam.VSA_PORT,
							   g_BTGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
							   g_BTGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
							 );
    if (ERR_OK!=err)
    {
        ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[BT] Fail to setup VSA.\n");
        return err;
    }

   /*----------------------------*
	* Disable VSG output signal  *
	*----------------------------*/
	// make sure no signal is generated by the VSG
	err = ::LP_EnableVsgRF(0);
	if ( ERR_OK!=err )
	{
		ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[BT] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
		return err;
	}

   /*--------------------------------*
    * Start "while" loop for average *
    *--------------------------------*/
    avgIteration = 0;
    int avg_times = 1;
    err = ::vDUT_GetIntegerReturn(g_BT_Dut, "AVG_TIMES",  (int*)&avg_times);				 
    if (ERR_OK!=err)
    {
        ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[BT] vDUT_GetIntegerReturn(AVG_TIMES) return error.\n");
        return err;
    } 

	char    logMessage[MAX_BUFFER_SIZE] = {'\0'};

    while(avgIteration<avg_times)
    {
	   /*----------------------------*
		* Perform normal VSA capture *
		*----------------------------*/
        err = LP_VsaDataCapture(tagInput.SAMPLING_TIME_US/1000000, tagInput.VSA_TRIGGER_TYPE);    
		if(ERR_OK!=err)	// capture is failed
		{
			// Fail Capture
			ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[BT] Fail to capture signal at %d MHz.\n", tagInput.FREQ_MHZ);
			return err;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] LP_VsaDataCapture() at %d MHz return OK.\n", tagInput.FREQ_MHZ);
		}
		
#pragma endregion

		/*--------------*
		 *  Capture OK  *
		 *--------------*/
		if (1==g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
		{
			// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "BT_TX_CAL_SaveAlways", tagInput.FREQ_MHZ);
			BTSaveSigFile(sigFileNameBuffer);
		}
		else
		{
			// do nothing
		}

		double dummy_T_INTERVAL      = 3.2;
		double dummy_MAX_POW_DIFF_DB = 15.0;  
		err = LP_AnalyzePower( dummy_T_INTERVAL/1000000, dummy_MAX_POW_DIFF_DB );

#pragma region Retrieve analysis Results
        if(ERR_OK==err)	// Analyze is ok
        {
            avgIteration++;
            /*------------------*
            * Store the result *
            *------------------*/
			meas_pwr = LP_GetScalarMeasurement("P_av_no_gap_all",0);
            if (meas_pwr)
            {
                meas_pwr = 10 * log10 (meas_pwr);
                meas_pwr += l_pathLossRecord;
                pwr_sum  += pow ((double) 10, meas_pwr/10);
            }
        }
        else
        {   // Fail Analysis, thus save capture (Signal File) for debug    
			err = -1;

			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "BT_TX_POWER_AnalysisFailed", tagInput.FREQ_MHZ);
			BTSaveSigFile(sigFileNameBuffer);
			
			ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[BT] Fail to analyze signal.\n");
            break;
        }
   
#pragma endregion
    }

    if( avgIteration && (err==ERR_OK) )
    {
        meas_pwr = 10 * log10 (pwr_sum/avgIteration);
    }
    else
    {
        meas_pwr = NA_NUMBER;
    }	

    /*-----------------------*
    *  Return Test Results  *
    *-----------------------*/
    tagOutput->POWER_AVG = meas_pwr;

    return err;
}


//This is the function to do analysis during TX calibration and provide the result to the DUT control layer
int TX_Cal_GetCWAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput)
{
    int    err = ERR_OK;

	int    avg_times = 0;
	char   errorMessage[MAX_BUFFER_SIZE] = {'\0'};

	tagOutput->FREQUENCY = NA_NUMBER;
    
    err = ::vDUT_GetIntegerReturn(g_BT_Dut, "AVG_TIMES",  (int*)&avg_times);				 
    if (ERR_OK!=err)
    {
        ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[BT] vDUT_GetIntegerReturn(AVG_TIMES) return error.\n");
        return err;
    } 

	err = AnalyzeCWFrequency( (double)tagInput.FREQ_MHZ,
			  				  tagInput.TX_POWER_DBM-l_pathLossRecord+g_BTGlobalSettingParam.IQ_P_TO_A_CW,
							  tagInput.SAMPLING_TIME_US,
							  avg_times,
							  &(tagOutput->FREQUENCY),
							  errorMessage
							);
    if (ERR_OK!=err)
    {
        ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, errorMessage);
        return err;
    }

    return err;
}

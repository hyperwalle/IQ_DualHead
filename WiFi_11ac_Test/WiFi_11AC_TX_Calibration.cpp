#include "stdafx.h"
#include "IQlite_Logger.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <math.h>
//Move to stdafx.h
//#include "lp_highrestimer.h"

// This global variable is declared in WiFi_Test_Internal.cpp
extern vDUT_ID      g_WiFi_11ac_Dut;
extern TM_ID        g_WiFi_11ac_Test_ID;               
extern int		    g_Tester_Type;
//extern bool         g_txCalibrationEnabled;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACtxCalParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_11ACtxCalReturnMap;

extern  WIFI_GLOBAL_SETTING g_WiFi11ACGlobalSettingParam;
// Return Value Container
extern map<string, WIFI_SETTING_STRUCT> l_11ACtxVerifyPowerReturnMap;
extern map<string, WIFI_SETTING_STRUCT> l_11ACtxVerifyEvmReturnMap;

double	l_pathLossRecord[MAX_DATA_STREAM];

struct tagParam
{
    // Mandatory Parameters
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. Default=0 dB */
    double SAMPLING_TIME_US;                        /*! The sampling time to do measurement. Default=50us */ 
    double TX_CAL_TIMEOUT_MS;                       /*! The timeout to do TX calibration. Default=600s */ 

} l_11ACtxCalParam;

struct tagReturn
{
    double	CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. Default=0 dB */
	char	CAL_RESULT_1[MAX_BUFFER_SIZE];
	char	CAL_RESULT_2[MAX_BUFFER_SIZE];
	char	CAL_RESULT_3[MAX_BUFFER_SIZE];
	char	CAL_RESULT_4[MAX_BUFFER_SIZE];
    char	ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_11ACtxCalReturn;

#ifndef WIN32
int init11ACTXCalContainers = Initialize11ACTXCalContainers();
#endif

//! WiFi TX Calibration
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
int SPATIAL_STREAM=1;
//GET PARAMETER FROM DUTs
typedef struct tagAnalysisInputParameterType
{
    int    BSS_FREQ_MHZ_PRIMARY;                            /*!< For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment */
	int    BSS_FREQ_MHZ_SECONDARY;                          /*!< For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined */
	int    CH_FREQ_MHZ_PRIMARY_20MHz;                       /*!< The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ"   */
	int    CH_FREQ_MHZ;										/*!< The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used  */
	int    NUM_STREAM_11AC;							        /*!< The number of streams 11AC (Only). */
    char   BSS_BANDWIDTH[MAX_BUFFER_SIZE];                  /*!< The RF bandwidth of basic service set (BSS) to verify EVM. */
	char   CH_BANDWIDTH[MAX_BUFFER_SIZE];                   /*!< The RF channel bandwidth to verify EVM. */
    char   DATA_RATE[MAX_BUFFER_SIZE];				/*! The data rate to verify POWER. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT[MAX_BUFFER_SIZE];                  /*!< The packet format of 11AC and 11N. */
	char   GUARD_INTERVAL[MAX_BUFFER_SIZE];				/*!< The guard interval for 11n and 11ac */
	char   STANDARD[MAX_BUFFER_SIZE];				/*!< The standard parameter used for signal analysis option or to discriminate the same data rates/package formats from different standards */
    double TX_POWER_DBM;                            /*! The output power to verify Power. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
    double SAMPLING_TIME_US;                        /*! The sampling time to verify Power. */ 
    //double T_INTERVAL;                              /*! This field is used to specify the interval that is used to determine if power is present or not. */
    //double MAX_POW_DIFF_DB;                         /*! This field is used to specify the maximum power difference between packets that are expected to be detected. */

    // DUT Parameters
    int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on) */
    int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */
    int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */

	//int VSA_PORT;
	char MEASURE_TYPE[MAX_BUFFER_SIZE];

}tagAnalysisInputParameter;

typedef struct tagAnalysisOutputType
{
    double EVM_AVG;                  
    double EVM_MAX;                  
    double EVM_MIN; 
    // POWER Test Result 
    
    double POWER_MAX;                
    double POWER_MIN;                 

	double POWER_AVG_ALL;
    double POWER_AVG[MAX_DATA_STREAM];
    double EVM_AVG_TMP[MAX_DATA_STREAM];
 
    double FREQ_ERROR_AVG;                             
    double FREQ_ERROR_MAX;                            
    double FREQ_ERROR_MIN;  

    double AMP_ERR_DB;								   /*!< IQ Match Amplitude Error in dB. */
    double PHASE_ERR;								   /*!< IQ Match Phase Error. */

	//channel list
	int    CH_FREQ_MHZ_PRIMARY_40MHz;       /*!< The center frequency (MHz) for primary 40 MHZ channel  */
	int    CH_FREQ_MHZ_PRIMARY_80MHz;       /*!< The center frequency (MHz) for primary 80 MHZ channel  */


    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
}tagAnalysisOutput;

#define DEFAULT_TIMEOUT_MS 600000                     //default timeout value: 600s 

int WIFI_11AC_TX_Cal_GetAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb);
int WIFI_11AC_TX_Cal_GetEVMAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb);
int WIFI_11AC_TX_Cal_GetPowerAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb);

WIFI_11AC_TEST_API int WIFI_11AC_TX_Calibration(void)
{

    int    err = ERR_OK;
    int    dummyValue;  
	double cableLossDb = 0;
	char   logMessage[MAX_BUFFER_SIZE] =  {'\0'};
	char   vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	int    previousVsaPort = PORT_LEFT;

    /*---------------------------------------*
    * Clear Return Parameters and Container *
    *---------------------------------------*/
    ClearReturnParameters(l_11ACtxCalReturnMap);

    /*------------------------*
    * Respond to QUERY_INPUT *
    *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err)
    {
        RespondToQueryInput(l_11ACtxCalParamMap);
        return err;
    }

    /*-------------------------*
    * Respond to QUERY_RETURN *
    *-------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err)
    {
        RespondToQueryReturn(l_11ACtxCalReturnMap);
        return err;
    }

    /*-----------------------------------------------------------*
    * Both g_WiFi_11ac_Test_ID and g_WiFi_11ac_Dut need to be valid (>=0) *
    *-----------------------------------------------------------*/
    TM_ClearReturns(g_WiFi_11ac_Test_ID);
    if( g_WiFi_11ac_Test_ID<0 || g_WiFi_11ac_Dut<0 )  
    {
        ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] WiFi_Test_ID and WiFi_Dut not valid.\n");
        return err;
    }

    /*----------------------*
    * Get input parameters *
    *----------------------*/
    err = GetInputParameters(l_11ACtxCalParamMap);
    if(ERR_OK!=err )
    {
        // Input parameters are not completed.  Exit
        ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] Input parameters are not completed.\n");
        return err;
    }

	// Error return of this function is irrelevant
	CheckDutTransmitStatus();

    // Record the path loss for Tx calibration
    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_pathLossRecord[i] = l_11ACtxCalParam.CABLE_LOSS_DB[i];
    }

    /*------------------------*
    *   Do Calibration        *
    *-------------------------*/
#pragma region configure DUT to calibration and begin timer
    //RW-2009-12-11: GET_CAL_SETTING has been merged into TX_CALIBRATION
    //err = vDUT_Run(g_WiFi_11ac_Dut, "GET_CAL_SETTING");
    //if (ERR_OK!=err)
    //{
    //    ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_Run(GET_CAL_SETTING) return error.");
    //}
    //else
    //{
    //    // TODO: Get Return parameters here
    //}
    highrestimer::GetTime(start_time);  
#pragma endregion	

#pragma region measurement till done
    int    iCalDone      = 0;
    int    iCalAbort     = 0;
    tagAnalysisInputParameter   tagInputPara;
    tagAnalysisOutput           tagOutputPara;
    tagInputPara.CH_FREQ_MHZ       = 2412; 
	tagInputPara.BSS_FREQ_MHZ_SECONDARY       = 0; 
	tagInputPara.CH_FREQ_MHZ_PRIMARY_20MHz       = 0; 
	tagInputPara.BSS_FREQ_MHZ_PRIMARY       = 2412;
	tagInputPara.NUM_STREAM_11AC       = 1;
	//tagInputPara.SAMPLING_TIME_US       = 160;
	tagInputPara.SAMPLING_TIME_US       = l_11ACtxCalParam.SAMPLING_TIME_US;
    strcpy_s(tagInputPara.DATA_RATE,MAX_BUFFER_SIZE, ""); 
    strcpy_s(tagInputPara.CH_BANDWIDTH,MAX_BUFFER_SIZE, ""); 
	strcpy_s(tagInputPara.BSS_BANDWIDTH,MAX_BUFFER_SIZE, ""); 
	strcpy_s(tagInputPara.PREAMBLE,MAX_BUFFER_SIZE, ""); 
    strcpy_s(tagInputPara.PACKET_FORMAT,MAX_BUFFER_SIZE, ""); 
	strcpy_s(tagInputPara.STANDARD,MAX_BUFFER_SIZE, ""); 
	strcpy_s(tagInputPara.GUARD_INTERVAL,MAX_BUFFER_SIZE, ""); 

    tagInputPara.TX_POWER_DBM   = 0; 

    for(int i=0;i<MAX_DATA_STREAM;i++)
	{
		tagInputPara.CABLE_LOSS_DB[i] = 0;
		tagOutputPara.POWER_AVG[i] = NA_NUMBER;
	}


	/*---------------------------------------------------------*/
	/* Before start calibrations, keep previous global setting */
	/* Now, just keep VSA_PORT status before calibration       */
	/* TODO: keep all previous global setting                  */
	/*---------------------------------------------------------*/
	previousVsaPort = g_WiFi11ACGlobalSettingParam.VSA_PORT;

    while(1)
    {
        highrestimer::GetTime (stop_time);

        l_iEclipseTimeMs = highrestimer::GetElapsedMSec(start_time, stop_time);
        
        if (l_iEclipseTimeMs>l_11ACtxCalParam.TX_CAL_TIMEOUT_MS)
		{
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] TX power calibration timed out\n");
            return TM_ERR_FUNCTION_ERROR;
        }

        //Set calibration point and transmit

        err = vDUT_Run(g_WiFi_11ac_Dut, "TX_CALIBRATION");
		if(vDUT_ERR_FUNCTION_NOT_SUPPORTED==err || vDUT_ERR_FUNCTION_NOT_DEFINED == err)
		{
			// for backward compability, older DUT controls implemented SET_CAL_MEASUREMENT
			err = vDUT_Run(g_WiFi_11ac_Dut, "SET_CAL_MEASUREMENT");
		}

        if (ERR_OK!=err)
        {
           // Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_Run(TX_CALIBRATION) return error, %s", vErrorMsg);
			}
			else	// Just return normal error message in this case
			{
				ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_Run(TX_CALIBRATION) return error.\n");
			}

			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
			return err;
        } 


        //Get the measure parameter from DUT control layer
        err = ::vDUT_GetIntegerReturn(g_WiFi_11ac_Dut, "CAL_DONE",  (int*)&iCalDone);				 
        if (ERR_OK!=err)
        {
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetIntegerReturn(CAL_DONE) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
        } 
        if(iCalDone)
        {
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Calibration Done.\n");
            break;
        }

        //Get the measure parameter from DUT control layer
        err = ::vDUT_GetIntegerReturn(g_WiFi_11ac_Dut, "CAL_ABORT",  (int*)&iCalAbort);				 
        if (ERR_OK!=err)
        {
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetIntegerReturn(CAL_ABORT) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
        } 

        if(iCalAbort)
        {
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] Abort Calibration.\n");
            return -1;
        }


        //Get the measure parameter from DUT control layer
        //frequency
        err = ::vDUT_GetIntegerReturn(g_WiFi_11ac_Dut, "BSS_FREQ_MHZ_PRIMARY",  &tagInputPara.BSS_FREQ_MHZ_PRIMARY);
        if (ERR_OK!=err)
        {
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetIntegerReturn(BSS_FREQ_MHZ_PRIMARY) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
        }
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(BSS_FREQ_MHZ_PRIMARY) return OK.\n");
		}

        err = ::vDUT_GetIntegerReturn(g_WiFi_11ac_Dut, "CH_FREQ_MHZ",  &tagInputPara.CH_FREQ_MHZ);
        if (ERR_OK!=err)
        {
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetIntegerReturn(CH_FREQ_MHZ) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
        }
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(CH_FREQ_MHZ) return OK.\n");
		}

        //data rate
        err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "DATA_RATE", tagInputPara.DATA_RATE, MAX_BUFFER_SIZE);
        if (ERR_OK!=err)
        {
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetStringReturn(DATA_RATE) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
        } 
		else
		{
			//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
			_strupr_s(tagInputPara.DATA_RATE, MAX_BUFFER_SIZE);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(DATA_RATE) return OK.\n");
		}

		//BSS Bandwidth
		err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "BSS_BANDWIDTH", tagInputPara.BSS_BANDWIDTH, MAX_BUFFER_SIZE);
        if (ERR_OK!=err)
        {
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetStringReturn(BSS_BANDWIDTH) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
        } 
		else
		{
			//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
			_strupr_s(tagInputPara.BSS_BANDWIDTH, MAX_BUFFER_SIZE);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(BSS_BANDWIDTH) return OK.\n");
		}

		err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "CH_BANDWIDTH", tagInputPara.CH_BANDWIDTH, MAX_BUFFER_SIZE);
        if (ERR_OK!=err)
        {
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetStringReturn(CH_BANDWIDTH) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
        } 
		else
		{
			//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
			_strupr_s(tagInputPara.CH_BANDWIDTH, MAX_BUFFER_SIZE);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(CH_BANDWIDTH) return OK.\n");
		}


	    //TX power
        err = ::vDUT_GetDoubleReturn(g_WiFi_11ac_Dut, "TX_POWER_DBM",  &tagInputPara.TX_POWER_DBM);
        if (ERR_OK!=err)
        {
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetIntegerReturn(TX_POWER_DBM) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
        } 	
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetIntegerReturn(TX_POWER_DBM) return OK.\n");
		}
		
        //OLD CALIBRATION: 
		//NOT EXISTED: TX1, TX2, TX3, TX4, PREAMBLE, PACKET_FORMAT_11N
		//TEMP: IF CAN'T GET THESE KEYWORD FROM VDUT, THEN GIVE TX_CALIBRATION INPUT PARAMETER
        //TODO: REMOVE "TX_CALIBRATION INPUT PARAMETER (TX1, TX2, TX3,TX4)"
        err = ::vDUT_GetIntegerReturn(g_WiFi_11ac_Dut, "TX1",  &tagInputPara.TX1);
		if ( ERR_OK!=err )
		{
			///*
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetIntegerReturn(TX1) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
			/*
			tagInputPara.TX1 = l_11ACtxCalParam.TX1;*/
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetIntegerReturn(TX1) return OK.\n");
		}

        err = ::vDUT_GetIntegerReturn(g_WiFi_11ac_Dut, "TX2",  &tagInputPara.TX2);
		if ( ERR_OK!=err )
		{
			///*
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetIntegerReturn(TX2) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
			//*/
			/*tagInputPara.TX2 = l_11ACtxCalParam.TX2;*/

		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetIntegerReturn(TX2) return OK.\n");
		}

        err = ::vDUT_GetIntegerReturn(g_WiFi_11ac_Dut, "TX3",  &tagInputPara.TX3);
		if ( ERR_OK!=err )
		{
			///*
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetIntegerReturn(TX3) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
			//*/
			//tagInputPara.TX3 = l_11ACtxCalParam.TX3;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetIntegerReturn(TX3) return OK.\n");
		}

        err = ::vDUT_GetIntegerReturn(g_WiFi_11ac_Dut, "TX4",  &tagInputPara.TX4);
		if ( ERR_OK!=err )
		{
			///*
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetIntegerReturn(TX4) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
			//*/
			//tagInputPara.TX4 = l_11ACtxCalParam.TX4;

		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetIntegerReturn(TX4) return OK.\n");
		}

		err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "PREAMBLE", tagInputPara.PREAMBLE,MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			/*
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetStringReturn(PREAMBLE) return error.\n");
			return err;
			*/
			sprintf_s(tagInputPara.PREAMBLE , MAX_BUFFER_SIZE, "SHORT");
		}
		else
		{
			//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
			_strupr_s(tagInputPara.PREAMBLE, MAX_BUFFER_SIZE);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(PREAMBLE) return OK.\n");
		}

        //PACKET_FORMAT
        err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "PACKET_FORMAT", tagInputPara.PACKET_FORMAT,MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			/*
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetStringReturn(PACKET_FORMAT_11N) return error.\n");
			return err;
			*/
			sprintf_s(tagInputPara.PACKET_FORMAT , MAX_BUFFER_SIZE, PACKET_FORMAT_HT_MF);
		}
		else
		{
			//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
			_strupr_s(tagInputPara.PACKET_FORMAT, MAX_BUFFER_SIZE);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(PACKET_FORMAT_11N) return OK.\n");
		}

		//STANDARD
        err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "STANDARD", tagInputPara.STANDARD,MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			/*
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetStringReturn(STANDARD) return error.\n");
			return err;
			*/
			sprintf_s(tagInputPara.STANDARD, MAX_BUFFER_SIZE, STANDARD_802_11_AC);
		}
		else
		{
			//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
			_strupr_s(tagInputPara.STANDARD, MAX_BUFFER_SIZE);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(STANDARD) return OK.\n");
		}

		//GUARD_INTERVAL
        err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "GUARD_INTERVAL", tagInputPara.GUARD_INTERVAL,MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			/*
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetStringReturn(GUARD_INTERVAL) return error.\n");
			return err;
			*/
			sprintf_s(tagInputPara.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
		}
		else
		{
			//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
			_strupr_s(tagInputPara.GUARD_INTERVAL, MAX_BUFFER_SIZE);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(GUARD_INTERVAL) return OK.\n");
		}

		/*--------------------------------------------------------------------------------------------
		 CHANGE GLOBAL SETTING:																      
		 While calibration, DUT may change GLOBAL setting, such like port, sampling times, and etc. 
		 Get the GLOBAL seeting from DUT control:                    
		 1. VSA_PORT                                                                             
		 2. ...                                                                                  
		 TODO: deal more VSA setting                                                             
		--------------------------------------------------------------------------------------------*/
		        
		//err = ::vDUT_GetIntegerReturn(g_WiFi_11ac_Dut, "VSA_PORT", &tagInputPara.VSA_PORT);
		//if ( ERR_OK!=err )
		//{
		//	/*
  //          ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetStringReturn(PACKET_FORMAT_11N) return error.\n");
		//	return err;
		//	*/
		//	//If VSA_PORT didn't return from Dut control, VSA_PORT use default global setting.
		//	tagInputPara.VSA_PORT = previousVsaPort;
		//	sprintf_s(tagInputPara.PACKET_FORMAT , MAX_BUFFER_SIZE, "MIXED");
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetIntegerReturn(VSA_PORT) return OK.\n");
		//}

		/*---------------------------------------------------
		 CHANGE GLOBAL SETTING:
		 1. VSA_PORT
		 2. ...
		 TODO: deal more global setting  
		---------------------------------------------------*/
		::TM_ClearParameters(g_WiFi_11ac_Test_ID);
		//::TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "VSA_PORT",      tagInputPara.VSA_PORT);

		/*------------------------*/
		/* CHANGE GLOBAL SETTING  */
		/*------------------------*/
		err = ::TM_Run(g_WiFi_11ac_Test_ID, "GLOBAL_SETTINGS");
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_Run(GLOBAL_SETTINGS) return OK.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_Run(GLOBAL_SETTINGS) return OK.\n");
		}

	    err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "MEASURE_TYPE", tagInputPara.MEASURE_TYPE,MAX_BUFFER_SIZE);
        if (ERR_OK!=err)
        {
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetStringReturn(MEASURE_TYPE) return error.\n");
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
        } 	

        // Check path loss (by ant and freq)
        for (int i=0;i<MAX_DATA_STREAM;i++)
        {
            l_11ACtxCalParam.CABLE_LOSS_DB[i] = l_pathLossRecord[i];
        }		

        err = CheckPathLossTableExt(   g_WiFi_11ac_Test_ID,
									tagInputPara.CH_FREQ_MHZ,
									tagInputPara.TX1,
									tagInputPara.TX2,
									tagInputPara.TX3,
									tagInputPara.TX4,
									l_11ACtxCalParam.CABLE_LOSS_DB,
									l_11ACtxCalReturn.CABLE_LOSS_DB,
									&cableLossDb,
									TX_TABLE
									);
        if (ERR_OK!=err)
        {
            ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] Failed to get TX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
			vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
            return err;
        } 

        if(strcmp(tagInputPara.MEASURE_TYPE,"POWER")==0)
        {
            //err = TX_Cal_GetAnalysisResult(tagInputPara, &tagOutputPara, cableLossDb); 
			err = WIFI_11AC_TX_Cal_GetPowerAnalysisResult(tagInputPara, &tagOutputPara, cableLossDb);
            if (ERR_OK!=err)
            {
				// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] TX_Cal_GetPowerAnalysisResult return error, %s", vErrorMsg);
				}
				else	// Just return normal error message in this case
				{
					ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] TX_Cal_GetPowerAnalysisResult return error.\n");
				}

				vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
				return err;
            } 
        }
        if(strcmp(tagInputPara.MEASURE_TYPE,"EVM")==0)
        {
            err = WIFI_11AC_TX_Cal_GetEVMAnalysisResult(tagInputPara, &tagOutputPara, cableLossDb); 
            if (ERR_OK!=err)
            {
				// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] TX_Cal_GetEVMAnalysisResult return error, %s", vErrorMsg);
				}
				else	// Just return normal error message in this case
				{
					ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] TX_Cal_GetEVMAnalysisResult return error.\n");
				}

				vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
				return err;
            }
        }


		//char keywordStr[MAX_BUFFER_SIZE] = {'\0'};
		
/*-----------------------*/
/*  BACKWARD COMPATIBLE  */
/*-----------------------*/

		//GET SPATIAL_STREAM
		//if(strcmp(tagInputPara.MEASURE_TYPE, "EVM") == 0)
		//{
		//	err = vDUT_GetIntegerParameter(g_WiFi_11ac_Dut, "SPATIAL_STREAM",  &SPATIAL_STREAM);
  //          if (ERR_OK!=err)
  //          {
  //              ReturnErrorMessage(l_11ACtxCalReturn.ERROR_MESSAGE, "[WiFi_11AC] vDUT_GetIntegerParameter(SPATIAL_STREAM) return error.\n");
  //              return err;
		//	}
		//}
		//else
		//{
		//	//POWER ANALYSIS CONDITION	
		//	//NOW: TX_VERIFY_POWER KEYOWRD DIDN'T HAVE MULTI-STREAM KEYWORD
		//	SPATIAL_STREAM = 1;
		//}
	
		//if(strcmp(tagInputPara.MEASURE_TYPE,"EVM")==0)
		//{
		//	for(int i= 0; i<SPATIAL_STREAM;i++)
		//	{

		//		//GET RESULT FRROM TX_VERIFY_EVM RESULT CONTAINER
		//		sprintf_s(keywordStr, MAX_BUFFER_SIZE, "POWER_AVG_%d", i+1);
		//		vDUT_GetDoubleParameter(g_WiFi_11ac_Dut, keywordStr,  &tagOutputPara.POWER_AVG[i]);
		//		sprintf_s(keywordStr, MAX_BUFFER_SIZE, "EVM_AVG_%d", i+1);
		//		vDUT_GetDoubleParameter(g_WiFi_11ac_Dut, keywordStr,  &tagOutputPara.EVM_AVG_TMP[i]);

		//		//BACKWARD COMPATIBLE
		//		//PUT RESULT INTO VDUT CONTAINTER WITH OLD CALIBRATION KEYWORD
		//		sprintf_s(keywordStr, MAX_BUFFER_SIZE, "MEAS_POWER_DBM_%d", i+1);
		//		vDUT_AddDoubleParameter(g_WiFi_11ac_Dut, keywordStr,  tagOutputPara.POWER_AVG[i]);
		//		sprintf_s(keywordStr, MAX_BUFFER_SIZE, "EVM_DB_%d", i+1);
		//		vDUT_AddDoubleParameter(g_WiFi_11ac_Dut, keywordStr,  tagOutputPara.EVM_AVG_TMP[i]);
		//	}
		//	
		//	vDUT_GetDoubleParameter(g_WiFi_11ac_Dut, "POWER_AVG_DBM",  &tagOutputPara.POWER_AVG_ALL);
		//	vDUT_AddDoubleParameter(g_WiFi_11ac_Dut, "MEAS_POWER_DBM",  tagOutputPara.POWER_AVG_ALL);
		//	
		//	vDUT_GetDoubleParameter(g_WiFi_11ac_Dut, "FREQ_ERROR_AVG", &tagOutputPara.FREQ_ERROR_AVG);
		//	vDUT_AddDoubleParameter(g_WiFi_11ac_Dut, "MEAS_FREQ_ERROR",  tagOutputPara.FREQ_ERROR_AVG);
		//	
		//	vDUT_GetDoubleParameter(g_WiFi_11ac_Dut, "EVM_AVG_DB", &tagOutputPara.EVM_AVG);
		//	vDUT_AddDoubleParameter(g_WiFi_11ac_Dut, "MEAS_EVM",  tagOutputPara.EVM_AVG);
		//	
		//	vDUT_GetDoubleParameter(g_WiFi_11ac_Dut, "AMP_ERR_DB", &tagOutputPara.AMP_ERR_DB);
		//	vDUT_AddDoubleParameter(g_WiFi_11ac_Dut, "AMP_MISMATCH",  tagOutputPara.AMP_ERR_DB);


		//	vDUT_GetDoubleParameter(g_WiFi_11ac_Dut, "PHASE_ERR", &tagOutputPara.PHASE_ERR);
		//	vDUT_AddDoubleParameter(g_WiFi_11ac_Dut, "PHASE_MISMATCH",  tagOutputPara.PHASE_ERR);

		//}
		//else
		//{
		//	//GET RESULT FROM TX_VERIFY_POWER RESULT CONTAINER
		//	vDUT_GetDoubleParameter(g_WiFi_11ac_Dut, "POWER_AVERAGE_DBM",  &tagOutputPara.POWER_AVG_ALL);
		//	//PUT RESULT INTO VDUT CONTAINER WITH OLD CALIBRATION KEYWORD
		//	vDUT_AddDoubleParameter(g_WiFi_11ac_Dut, "MEAS_POWER_DBM",  tagOutputPara.POWER_AVG_ALL);

		//}
	}

	/*---------------------------------------------------
	 RESTORE PREVIOUS GLOBAL SETTING:
	 1. VSA_PORT
	 TODO: deal more previous global setting  
	---------------------------------------------------*/
	::TM_ClearParameters(g_WiFi_11ac_Test_ID);
	::TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "VSA_PORT",      previousVsaPort);

	/*------------------------*/
	/* CHANGE GLOBAL SETTING  */
	/*------------------------*/
	err = ::TM_Run(g_WiFi_11ac_Test_ID, "GLOBAL_SETTINGS");
	if ( ERR_OK!=err )
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_Run(GLOBAL_SETTINGS) return OK.\n");
	}
	else
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_Run(GLOBAL_SETTINGS) return OK.\n");
	}

#pragma endregion	

    /*-----------------------*
    *  Return Test Results  *
    *-----------------------*/
    if (ERR_OK==err)
    {
        sprintf_s(l_11ACtxCalReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		//get  return information
		vDUT_GetStringReturn(g_WiFi_11ac_Dut, "CAL_RESULT_1", l_11ACtxCalReturn.CAL_RESULT_1,MAX_BUFFER_SIZE);
		vDUT_GetStringReturn(g_WiFi_11ac_Dut, "CAL_RESULT_2", l_11ACtxCalReturn.CAL_RESULT_2,MAX_BUFFER_SIZE);
		vDUT_GetStringReturn(g_WiFi_11ac_Dut, "CAL_RESULT_3", l_11ACtxCalReturn.CAL_RESULT_3,MAX_BUFFER_SIZE);
		vDUT_GetStringReturn(g_WiFi_11ac_Dut, "CAL_RESULT_4", l_11ACtxCalReturn.CAL_RESULT_4,MAX_BUFFER_SIZE);
        ReturnTestResults(l_11ACtxCalReturnMap);
    } 

	vDUT_Run(g_WiFi_11ac_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.

    return err;
}

int Initialize11ACTXCalContainers(void)
{
    /*------------------*
    * Input Paramters: *
    * IQTESTER_IP01    *
    *------------------*/
    l_11ACtxCalParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    l_11ACtxCalParam.SAMPLING_TIME_US = 150.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxCalParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxCalParam.SAMPLING_TIME_US;
        setting.unit        = "uS";
        setting.helpText    = "Capture time in micro-seconds";
        l_11ACtxCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_11ACtxCalParam.CABLE_LOSS_DB[i] = 0.0;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxCalParam.CABLE_LOSS_DB[i]))    // Type_Checking
        {
            setting.value       = (void*)&l_11ACtxCalParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Cable loss from the DUT antenna port to tester";
            l_11ACtxCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    //l_11ACtxCalParam.TX1 = 1;
    //setting.type = WIFI_SETTING_TYPE_INTEGER;
    //if (sizeof(int)==sizeof(l_11ACtxCalParam.TX1))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_11ACtxCalParam.TX1;
    //    setting.unit        = "";
    //    setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
    //    l_11ACtxCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    //l_11ACtxCalParam.TX2 = 0;
    //setting.type = WIFI_SETTING_TYPE_INTEGER;
    //if (sizeof(int)==sizeof(l_11ACtxCalParam.TX2))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_11ACtxCalParam.TX2;
    //    setting.unit        = "";
    //    setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
    //    l_11ACtxCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    //l_11ACtxCalParam.TX3 = 0;
    //setting.type = WIFI_SETTING_TYPE_INTEGER;
    //if (sizeof(int)==sizeof(l_11ACtxCalParam.TX3))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_11ACtxCalParam.TX3;
    //    setting.unit        = "";
    //    setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
    //    l_11ACtxCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    //l_11ACtxCalParam.TX4 = 0;
    //setting.type = WIFI_SETTING_TYPE_INTEGER;
    //if (sizeof(int)==sizeof(l_11ACtxCalParam.TX4))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_11ACtxCalParam.TX4;
    //    setting.unit        = "";
    //    setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
    //    l_11ACtxCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    //l_11ACtxCalParam.AVG_TIMES = 3;
    //setting.type = WIFI_SETTING_TYPE_INTEGER;
    //if (sizeof(int)==sizeof(l_11ACtxCalParam.AVG_TIMES))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_11ACtxCalParam.AVG_TIMES;
    //    setting.unit        = "";
    //    setting.helpText    = "DUT AVG POWER TIMES DEFULT is 3!";
    //    l_11ACtxCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("AVG_TIMES", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    l_11ACtxCalParam.TX_CAL_TIMEOUT_MS = DEFAULT_TIMEOUT_MS;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxCalParam.TX_CAL_TIMEOUT_MS))    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxCalParam.TX_CAL_TIMEOUT_MS;
        setting.unit        = "MS";
        setting.helpText    = "timeout when calibration fail";
        l_11ACtxCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_CAL_TIMEOUT_MS", setting));
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
    l_11ACtxCalReturnMap.clear();

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_11ACtxCalReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxCalReturn.CABLE_LOSS_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxCalReturn.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Cable loss from the DUT antenna port to tester";
            l_11ACtxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    l_11ACtxCalReturn.CAL_RESULT_1[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxCalReturn.CAL_RESULT_1))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxCalReturn.CAL_RESULT_1;
        setting.unit        = "";
        setting.helpText    = "Calibration Result 1";
        l_11ACtxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_1", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxCalReturn.CAL_RESULT_2[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACtxCalReturn.CAL_RESULT_2))    // Type_Checking
	{
		setting.value       = (void*)l_11ACtxCalReturn.CAL_RESULT_2;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 2";
		l_11ACtxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_2", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxCalReturn.CAL_RESULT_3[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACtxCalReturn.CAL_RESULT_3))    // Type_Checking
	{
		setting.value       = (void*)l_11ACtxCalReturn.CAL_RESULT_3;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 3";
		l_11ACtxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_3", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxCalReturn.CAL_RESULT_4[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACtxCalReturn.CAL_RESULT_4))    // Type_Checking
	{
		setting.value       = (void*)l_11ACtxCalReturn.CAL_RESULT_4;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 4";
		l_11ACtxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_4", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	
    l_11ACtxCalReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxCalReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxCalReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_11ACtxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}


//This is the function to do EVM analysis when TX calibration and put the result to DUT control layer
int WIFI_11AC_TX_Cal_GetEVMAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb)
{
    int  err = ERR_OK;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

    //Step1: Insert Keyword into EVM Container 
    ::TM_ClearParameters(g_WiFi_11ac_Test_ID);

	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "BSS_FREQ_MHZ_PRIMARY",		tagInput.BSS_FREQ_MHZ_PRIMARY);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "BSS_FREQ_MHZ_SECONDARY",		tagInput.BSS_FREQ_MHZ_SECONDARY);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "CH_FREQ_MHZ",					tagInput.CH_FREQ_MHZ);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "CH_FREQ_MHZ_PRIMARY_20MHz",	tagInput.CH_FREQ_MHZ_PRIMARY_20MHz);
	
	TM_AddStringParameter(g_WiFi_11ac_Test_ID, "BSS_BANDWIDTH",		tagInput.BSS_BANDWIDTH);
	TM_AddStringParameter(g_WiFi_11ac_Test_ID, "CH_BANDWIDTH",		tagInput.CH_BANDWIDTH);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "NUM_STREAM_11AC",		tagInput.NUM_STREAM_11AC);
	TM_AddStringParameter (g_WiFi_11ac_Test_ID, "PREAMBLE",			tagInput.PREAMBLE);
	TM_AddStringParameter (g_WiFi_11ac_Test_ID, "PACKET_FORMAT",		tagInput.PACKET_FORMAT);
	TM_AddStringParameter (g_WiFi_11ac_Test_ID, "STANDARD",		tagInput.STANDARD);
	TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "TX_POWER_DBM",		tagInput.TX_POWER_DBM);
	TM_AddStringParameter (g_WiFi_11ac_Test_ID, "DATA_RATE",			tagInput.DATA_RATE);
	TM_AddStringParameter (g_WiFi_11ac_Test_ID, "GUARD_INTERVAL",			tagInput.GUARD_INTERVAL);

	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "TX1",					tagInput.TX1);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "TX2",					tagInput.TX2);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "TX3",					tagInput.TX3);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "TX4",					tagInput.TX4);
    TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "SAMPLING_TIME_US",	tagInput.SAMPLING_TIME_US);

	TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "CABLE_LOSS_DB_1", l_11ACtxCalParam.CABLE_LOSS_DB[0]);
	TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "CABLE_LOSS_DB_2", l_11ACtxCalParam.CABLE_LOSS_DB[1]);
	TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "CABLE_LOSS_DB_3", l_11ACtxCalParam.CABLE_LOSS_DB[2]);
	TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "CABLE_LOSS_DB_4", l_11ACtxCalParam.CABLE_LOSS_DB[3]);


    //Step2: Disable DUT Config in Wifi Tx Verify POWER
    //g_txCalibrationEnabled = false;

    //Step3: Run Tx_Verify_EVM
    err = WIFI_11AC_TX_Verify_Evm();

    //Has done tx_calibration no matter is successfuly or fail.
    //g_txCalibrationEnabled = false;

    if(err != ERR_OK)
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Function WiFi_TX_Verify_Evm() Failed.\n");
	}
	else
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Function WiFi_TX_Verify_Evm() OK.\n");
	}

    //Step4: Get Result from EVM return map
    PassTestResultToDutInputContainer(l_11ACtxVerifyEvmReturnMap);

	//Step5: TODO error code
	return err;
}

//This is the function to do analysis when TX calibration and put the result to DUT control layer
int WIFI_11AC_TX_Cal_GetPowerAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb)
{
	int     err = ERR_OK;
    char    logMessage[MAX_BUFFER_SIZE] = {'\0'};

    //Step1: Insert Keyword into EVM Container 
    ::TM_ClearParameters(g_WiFi_11ac_Test_ID);

	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "BSS_FREQ_MHZ_PRIMARY",		tagInput.BSS_FREQ_MHZ_PRIMARY);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "BSS_FREQ_MHZ_SECONDARY",		tagInput.BSS_FREQ_MHZ_SECONDARY);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "CH_FREQ_MHZ",					tagInput.CH_FREQ_MHZ);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "CH_FREQ_MHZ_PRIMARY_20MHz",	tagInput.CH_FREQ_MHZ_PRIMARY_20MHz);
									             
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "NUM_STREAM_11AC",		tagInput.NUM_STREAM_11AC);

	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "TX1",					tagInput.TX1);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "TX2",					tagInput.TX2);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "TX3",					tagInput.TX3);
	TM_AddIntegerParameter(g_WiFi_11ac_Test_ID, "TX4",					tagInput.TX4);

	TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "TX_POWER_DBM",		tagInput.TX_POWER_DBM);

	TM_AddStringParameter(g_WiFi_11ac_Test_ID, "BSS_BANDWIDTH",		tagInput.BSS_BANDWIDTH);
	TM_AddStringParameter(g_WiFi_11ac_Test_ID, "CH_BANDWIDTH",		tagInput.CH_BANDWIDTH);
	
	TM_AddStringParameter (g_WiFi_11ac_Test_ID, "PREAMBLE",			tagInput.PREAMBLE);
	TM_AddStringParameter (g_WiFi_11ac_Test_ID, "PACKET_FORMAT",		tagInput.PACKET_FORMAT);
	TM_AddStringParameter (g_WiFi_11ac_Test_ID, "STANDARD",		tagInput.STANDARD);
	TM_AddStringParameter (g_WiFi_11ac_Test_ID, "GUARD_INTERVAL",		tagInput.GUARD_INTERVAL);
	TM_AddStringParameter (g_WiFi_11ac_Test_ID, "DATA_RATE",			tagInput.DATA_RATE);
	
	TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "SAMPLING_TIME_US",	tagInput.SAMPLING_TIME_US);

	TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "CABLE_LOSS_DB_1", l_11ACtxCalParam.CABLE_LOSS_DB[0]);
	TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "CABLE_LOSS_DB_2", l_11ACtxCalParam.CABLE_LOSS_DB[1]);
	TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "CABLE_LOSS_DB_3", l_11ACtxCalParam.CABLE_LOSS_DB[2]);
	TM_AddDoubleParameter (g_WiFi_11ac_Test_ID, "CABLE_LOSS_DB_4", l_11ACtxCalParam.CABLE_LOSS_DB[3]);

		/*----------------------*/
    //g_txCalibrationEnabled = true;

    //Step3: Run Tx_Verify_EVM
    err = WIFI_11AC_TX_Verify_Power();

    //Has done tx_calibration no matter is successfuly or fail.
    //g_txCalibrationEnabled = false;

    if(err != ERR_OK)
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Function WiFi_TX_Verify_Power() Failed.\n");
	}
	else
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Function WiFi_TX_Verify_Power() OK.\n");
	}

    //Step4: Get Result from EVM return map
    PassTestResultToDutInputContainer(l_11ACtxVerifyPowerReturnMap);
    //TM_ClearReturns(g_WiFi_11ac_Test_ID);  //don't clear return values, returns will be handled from calling function

	//Step5: TODO error code
	return err;


}
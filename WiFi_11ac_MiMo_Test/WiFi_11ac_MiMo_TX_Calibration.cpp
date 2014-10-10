#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11ac_MiMo_Test.h"
#include "WiFi_11ac_MiMo_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <math.h>
//Move to stdafx.h
//#include "lp_highrestimer.h"

// This global variable is declared in WiFi_Test_Internal.cpp
extern vDUT_ID      g_WiFi_Dut;
extern TM_ID        g_WiFi_Test_ID;
extern bool         g_txCalibrationEnabled;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_txCalParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_txCalReturnMap;

extern map<string, WIFI_SETTING_STRUCT> l_txVerifyPowerReturnMap;
extern map<string, WIFI_SETTING_STRUCT> l_txVerifyEvmReturnMap;
extern  WIFI_GLOBAL_SETTING g_globalSettingParam;

double	l_pathLossRecord[MAX_DATA_STREAM];

struct tagParam
{
	// Mandatory Parameters
	double CABLE_LOSS_DB[MAX_TESTER_NUM];                  /*! The path loss of test system. Default=0 dB */
	double SAMPLING_TIME_US;                                /*! The sampling time to do measurement. Default=50us */ 

    char   VSA_CONNECTION[MAX_TESTER_NUM][MAX_BUFFER_SIZE]; /*!< The vsa connect to which antenn port. */ 
    char   VSA_PORT[MAX_TESTER_NUM][MAX_BUFFER_SIZE];		/*!< IQTester VSAs port setting. Default=PORT_LEFT. */  
	double TX_CAL_TIMEOUT_MS;                               /*! The timeout to do TX calibration. Default=600s */ 

} l_txCalParam;

struct tagReturn
{
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. Default=0 dB */
	char CAL_RESULT_1[MAX_BUFFER_SIZE];
	char CAL_RESULT_2[MAX_BUFFER_SIZE];
	char CAL_RESULT_3[MAX_BUFFER_SIZE];
	char CAL_RESULT_4[MAX_BUFFER_SIZE];
	char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txCalReturn;

#ifndef WIN32
int initTXCalMIMOContainers = InitializeTXCalContainers();
#endif

//! WiFi TX Calibration
/*!
* Input Parameters
*
*  - Mandatory 
*      -# CABLE_LOSS_DB_VSA1 (double): The path loss of VSA 1.
*      -# CABLE_LOSS_DB_VSA2 (double): The path loss of VSA 2.
*      -# CABLE_LOSS_DB_VSA3 (double): The path loss of VSA 3.
*      -# CABLE_LOSS_DB_VSA4 (double): The path loss of VSA 4.
*      -# SAMPLING_TIME_US	 (double): The data rate to do measurement.
*      -# TX_CAL_TIMEOUT_MS  (double): The timeout while doing measurement.
*      -# VSA1_CONNECTION	 (double): The chain # connect to VSA1.
*      -# VSA2_CONNECTION	 (double): The chain # connect to VSA2.
*      -# VSA3_CONNECTION	 (double): The chain # connect to VSA3.
*      -# VSA4_CONNECTION	 (double): The chain # connect to VSA4.
*      -# VSA1_PORT			 (double): The vsa port of tester 1.
*      -# VSA2_PORT			 (double): The vsa port of tester 2.
*      -# VSA3_PORT			 (double): The vsa port of tester 3.
*      -# VSA4_PORT			 (double): The vsa port of tester 4.
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

//struct __timeb64 start_time, stop_time;
//int g_iEclipseTimeMs;

typedef struct tagAnalysisInputParameterType
{
	//int    FREQ_MHZ;                                /*! The center frequency (MHz).*/
	int    BSS_FREQ_MHZ_PRIMARY;                            /*!< For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment */
	int    BSS_FREQ_MHZ_SECONDARY;                          /*!< For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined */
	int    CH_FREQ_MHZ_PRIMARY_20MHz;                       /*!< The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ"   */
	int    CH_FREQ_MHZ;										/*!< The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used  */

	char   DATA_RATE[MAX_BUFFER_SIZE];			    /*! The data rate to verify POWER. */
	double TX_POWER_DBM;      
	//char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify POWER. */
	char   BSS_BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify POWER. */
	char   CH_BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify POWER. */
	char   MEASURE_TYPE[MAX_BUFFER_SIZE];

    char   PREAMBLE[MAX_BUFFER_SIZE];
    char   PACKET_FORMAT[MAX_BUFFER_SIZE];

    //DUT Parameters
    int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on) */
    int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */
    int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */
    int    AVG_TIMES;                               /*!< DUT AVG_TIMES.  Default=3 */

}tagAnalysisInputParameter;

typedef struct tagAnalysisOutputType
{
	double EVM_AVG;                  
	double EVM_MAX;                  
	double EVM_MIN; 
	// POWER Test Result 
	double POWER_AVG;
	double POWER_MAX;                
	double POWER_MIN;                 

	double FREQ_ERROR_AVG;                             
	double FREQ_ERROR_MAX;                            
	double FREQ_ERROR_MIN;  

	double AMP_ERR_DB;								   /*!< IQ Match Amplitude Error in dB. */
	double PHASE_ERR;								   /*!< IQ Match Phase Error. */

	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
}tagAnalysisOutput;

#define DEFAULT_TIMEOUT_MS 600000                     //default timeout value: 600s 

int TX_Cal_GetPowerAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb);
int TX_Cal_GetEVMAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb);

WIFI_11AC_MIMO_TEST_API int WiFi_TX_Calibration(void)
{
	int    err = ERR_OK;
	int    dummyValue = 0;  
	double cableLossDb = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_txCalReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err)
	{
		RespondToQueryInput(l_txCalParamMap);
		return err;
	}

	/*-------------------------*
	* Respond to QUERY_RETURN *
	*-------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_RETURN", &dummyValue);
	if( ERR_OK==err)
	{
		RespondToQueryReturn(l_txCalReturnMap);
		return err;
	}

	try
	{
		/*-----------------------------------------------------------*
		* Both g_WiFi_Test_ID and g_WiFi_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_WiFi_Test_ID<0 || g_WiFi_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] WiFi_Test_ID or WiFi_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		}

		TM_ClearReturns(g_WiFi_Test_ID);

		/*----------------------*
		* Get input parameters *
		*----------------------*/
		err = GetInputParameters(l_txCalParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Get input parameters return OK.\n");
		}

		// Error return of this function is irrelevant
		CheckDutStatus();

		//for(int i=0;i<MAX_TESTER_NUM;i++)
		//{
		//	vsaMappingTx[i] = 0;
		//	vsaPort[i] = PORT_LEFT;
		//}

		////Check VSA port and VSA connect to which TX
		//err = CheckConnectionVsaAndAntennaPort(l_txCalParamMap, vsaMappingTx, vsaPort);
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi_11ac_MiMo] Mapping VSA to antenna port failed\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Mapping VSA to antenna port OK.\n");
		//}

		// Record the path loss for Tx calibration
		for (int i=0;i<MAX_DATA_STREAM;i++)
		{
			l_pathLossRecord[i] = l_txCalParam.CABLE_LOSS_DB[i];
		}
		//for (int i=0;i<MAX_TESTER_NUM;i++)
		//{
		//	switch(vsaPort[i])
		//	{
		//		case 2:
		//			sprintf_s(l_txCalParam.VSA_PORT[i], MAX_BUFFER_SIZE, "LEFT");
		//			break;
		//		case 3:
		//			sprintf_s(l_txCalParam.VSA_PORT[i], MAX_BUFFER_SIZE, "RIGHT");
		//			break;
		//		default:
		//			sprintf_s(l_txCalParam.VSA_PORT[i], MAX_BUFFER_SIZE, "LEFT");
		//			break;
		//	}

		//}


		/*------------------------*
		*   Do Calibration        *
		*-------------------------*/
#pragma region configure DUT to calibration and begin timer
		//err = vDUT_Run(g_WiFi_Dut, "GET_CAL_SETTING");
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(GET_CAL_SETTING) return error.\n");
		//	//throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(GET_CAL_SETTING) return OK.\n");
		//	// TODO: Get Return parameters here
		//}
		//highrestimer::GetTime(start_time);  
		_ftime64_s (&start_time);
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
		strcpy_s(tagInputPara.DATA_RATE,MAX_BUFFER_SIZE, ""); 
		//strcpy_s(tagInputPara.BANDWIDTH,MAX_BUFFER_SIZE, ""); 
		strcpy_s(tagInputPara.BSS_BANDWIDTH,MAX_BUFFER_SIZE, ""); 
		strcpy_s(tagInputPara.CH_BANDWIDTH,MAX_BUFFER_SIZE, ""); 
		tagInputPara.TX_POWER_DBM   = 0; 
		tagOutputPara.POWER_AVG = NA_NUMBER;

		while(1)
		{
			//highrestimer::GetTime (stop_time);
			_ftime64_s (&stop_time);
			l_iEclipseTimeMs = (unsigned int) (((stop_time.time - start_time.time) * 1000) + (stop_time.millitm - start_time.millitm));
			
		//	l_iEclipseTimeMs = highrestimer::GetElapsedMSec(start_time, stop_time);

			if (l_iEclipseTimeMs>l_txCalParam.TX_CAL_TIMEOUT_MS)
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] TX power calibration timeout! Abort.\n");
				throw logMessage;
			}

            err = vDUT_Run(g_WiFi_Dut, "TX_CALIBRATION");
			if(vDUT_ERR_FUNCTION_NOT_SUPPORTED==err || vDUT_ERR_FUNCTION_NOT_DEFINED == err)
			{
				// for backward compability, older DUT controls implemented SET_CAL_MEASUREMENT
				err = vDUT_Run(g_WiFi_Dut, "SET_CAL_MEASUREMENT");
			}

            if (ERR_OK!=err)
            {
                ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[Error] vDUT_Run(TX_CALIBRATION) return error.\n");
                return err;
            } 

			//Get the measure parameter from DUT control layer
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "CAL_DONE",  (int*)&iCalDone);				 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(CAL_DONE) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(CAL_DONE) return OK.\n");
			}
            
            if(iCalDone)
            {
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Calibration Done.\n");
                break;
            }
            else
            {
                // continue next calibration step
            }

			//Get the measure parameter from DUT control layer
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "CAL_ABORT",  (int*)&iCalAbort);				 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(CAL_ABORT) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(CAL_ABORT) return OK.\n");
			}

            if(iCalAbort)
            {
                ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[Error] Abort Calibration.\n");
                return -1;
            }


			//Get the measure parameter from DUT control layer
			//frequency
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "CH_FREQ_MHZ",  &tagInputPara.CH_FREQ_MHZ);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(CH_FREQ_MHZ) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(CH_FREQ_MHZ) return OK.\n");
			} 

			//BSS frequency
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "BSS_FREQ_MHZ_PRIMARY",  &tagInputPara.BSS_FREQ_MHZ_PRIMARY);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(BSS_FREQ_MHZ_PRIMARY) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(BSS_FREQ_MHZ_PRIMARY) return OK.\n");
			}

			//data rate
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "DATA_RATE", tagInputPara.DATA_RATE, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(DATA_RATE) return error.\n");
				throw logMessage;
			}
			else
			{
				//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
				_strupr_s(tagInputPara.DATA_RATE, MAX_BUFFER_SIZE);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(DATA_RATE) return OK.\n");
			}

			//err = ::vDUT_GetStringReturn(g_WiFi_Dut, "CHANNEL_BW", tagInputPara.BANDWIDTH, MAX_BUFFER_SIZE);
			//if ( ERR_OK!=err )
			//{
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(CHANNEL_BW) return error.\n");
			//	throw logMessage;
			//}
			//else
			//{
			//	//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
			//	_strupr_s(tagInputPara.BANDWIDTH, MAX_BUFFER_SIZE);
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(CHANNEL_BW) return OK.\n");
			//}

			//Channel Bandwidth
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "CH_BANDWIDTH", tagInputPara.CH_BANDWIDTH, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(CH_BANDWIDTH) return error.\n");
				throw logMessage;
			}
			else
			{
				//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
				_strupr_s(tagInputPara.CH_BANDWIDTH, MAX_BUFFER_SIZE);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(CH_BANDWIDTH) return OK.\n");
			}

			//BSS Bandwidth
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "BSS_BANDWIDTH", tagInputPara.BSS_BANDWIDTH, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(BSS_BANDWIDTH) return error.\n");
				throw logMessage;
			}
			else
			{
				//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
				_strupr_s(tagInputPara.BSS_BANDWIDTH, MAX_BUFFER_SIZE);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(BSS_BANDWIDTH) return OK.\n");
			}

			//TX power
			err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, "TX_POWER_DBM",  &tagInputPara.TX_POWER_DBM);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(TX_POWER_DBM) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(TX_POWER_DBM) return OK.\n");
			}

            err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "TX1",  &tagInputPara.TX1);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(TX1) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(TX1) return OK.\n");
			}

            err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "TX2",  &tagInputPara.TX2);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(TX2) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(TX2) return OK.\n");
			}

            err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "TX3",  &tagInputPara.TX3);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(TX3) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(TX3) return OK.\n");
			}

            err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "TX4",  &tagInputPara.TX4);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(TX4) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(TX4) return OK.\n");
			}

            //PREAMBLE
            err = ::vDUT_GetStringReturn(g_WiFi_Dut, "PREAMBLE", tagInputPara.PREAMBLE,MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				/*
				ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(PREAMBLE) return error.\n");
				return err;
				*/
				sprintf_s(tagInputPara.PREAMBLE , MAX_BUFFER_SIZE, "SHORT");
			}
			else
			{
				//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
				_strupr_s(tagInputPara.PREAMBLE, MAX_BUFFER_SIZE);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(PREAMBLE) return OK.\n");
			}

            //PACKET_FORMAT
            err = ::vDUT_GetStringReturn(g_WiFi_Dut, "PACKET_FORMAT", tagInputPara.PACKET_FORMAT,MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				/*
				ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(PACKET_FORMAT) return error.\n");
				return err;
				*/
				sprintf_s(tagInputPara.PACKET_FORMAT , MAX_BUFFER_SIZE, "MIXED");
			}
			else
			{
				//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
				_strupr_s(tagInputPara.PACKET_FORMAT, MAX_BUFFER_SIZE);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(PACKET_FORMAT) return OK.\n");
			}
            //AVERAGE_TIMES
   //         err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "AVG_TIMES",  &tagInputPara.AVG_TIMES);
			//if ( ERR_OK!=err )
			//{
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(NUMBER_OF_AVG) return error.\n");
			//	throw logMessage;
			//}
			//else
			//{
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(NUMBER_OF_AVG) return OK.\n");
			//}

			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "MEASURE_TYPE", tagInputPara.MEASURE_TYPE,MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(MEASURE_TYPE) return error.\n");
				throw logMessage;
			}
			else
			{
				//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
				_strupr_s(tagInputPara.DATA_RATE, MAX_BUFFER_SIZE);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(MEASURE_TYPE) return OK.\n");
			}

			// Check path loss (by ant and freq)
			for (int i=0;i<MAX_DATA_STREAM;i++)
			{
				l_txCalParam.CABLE_LOSS_DB[i] = l_pathLossRecord[i];
			}		

			//For re-use Tx_Verify_Power() and Tx_Verify_EVM()
			//CheckPathLossTable() should be removed.
			//TODO: MARKED 
			//err = CheckPathLossTable( g_WiFi_Dut,
			//	                    tagInputPara.FREQ_MHZ,
			//	                    tagInputPara.TX1,
			//	                    tagInputPara.TX2,
			//	                    tagInputPara.TX3,
			//	                    tagInputPara.TX4,
			//	                    l_txCalParam.CABLE_LOSS_DB,
			//	                    l_txCalReturn.CABLE_LOSS_DB,
			//	                    &cableLossDb
			//	                    );
			//if ( ERR_OK!=err )
			//{
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to get CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
			//	throw logMessage;
			//}
			//else
			//{
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] CheckPathLossTable() return OK.\n");
			//}

			if(strcmp(tagInputPara.MEASURE_TYPE,"POWER")==0)
			{
				err = TX_Cal_GetPowerAnalysisResult(tagInputPara, &tagOutputPara, cableLossDb); 
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] TX_Cal_GetPowerAnalysisResult() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TX_Cal_GetPowerAnalysisResult() return OK.\n");
				}
			}
			if(strcmp(tagInputPara.MEASURE_TYPE,"EVM")==0)
			{
				err = TX_Cal_GetEVMAnalysisResult(tagInputPara, &tagOutputPara, cableLossDb); 
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] TX_Cal_GetEVMAnalysisResult() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TX_Cal_GetEVMAnalysisResult() return OK.\n");
				}
			}
			//input the measure power value to DUT control
			//vDUT_ClearParameters(g_WiFi_Dut);
			//vDUT_AddDoubleParameter(g_WiFi_Dut, "MEAS_POWER_DBM",  tagOutputPara.POWER_AVG);
			//vDUT_AddDoubleParameter(g_WiFi_Dut, "MEAS_FREQ_ERROR",  tagOutputPara.FREQ_ERROR_AVG);
			//vDUT_AddDoubleParameter(g_WiFi_Dut, "MEAS_EVM",  tagOutputPara.EVM_AVG);
			//vDUT_AddDoubleParameter(g_WiFi_Dut, "AMP_MISMATCH",  tagOutputPara.AMP_ERR_DB);
			//vDUT_AddDoubleParameter(g_WiFi_Dut, "PHASE_MISMATCH",  tagOutputPara.PHASE_ERR);

		}
#pragma endregion	

		/*-----------------------*
		*  Return Test Results  *
		*-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_txCalReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			//get  return information
			vDUT_GetStringReturn(g_WiFi_Dut, "CAL_RESULT_1", l_txCalReturn.CAL_RESULT_1,MAX_BUFFER_SIZE);
			vDUT_GetStringReturn(g_WiFi_Dut, "CAL_RESULT_2", l_txCalReturn.CAL_RESULT_2,MAX_BUFFER_SIZE);
			vDUT_GetStringReturn(g_WiFi_Dut, "CAL_RESULT_3", l_txCalReturn.CAL_RESULT_3,MAX_BUFFER_SIZE);
			vDUT_GetStringReturn(g_WiFi_Dut, "CAL_RESULT_4", l_txCalReturn.CAL_RESULT_4,MAX_BUFFER_SIZE);
			ReturnTestResults(l_txCalReturnMap);
		} 
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[WiFi_11ac_MiMo] Unknown Error!\n");
	}

	return err;
}

int InitializeTXCalContainers(void)
{
	/*------------------*
	* Input Parameters: *
	* IQTESTER_IP01    *
	*------------------*/
	l_txCalParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	l_txCalParam.SAMPLING_TIME_US = 50.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txCalParam.SAMPLING_TIME_US))    // Type_Checking
	{
		setting.value = (void*)&l_txCalParam.SAMPLING_TIME_US;
		setting.unit        = "uS";
		setting.helpText    = "Capture time in micro-seconds";
		l_txCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txCalParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txCalParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txCalParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}
    //VSG_CONNECTION
    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_txCalParam.VSA_CONNECTION[i], MAX_BUFFER_SIZE, "TX%d", i+1);
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_txCalParam.VSA_CONNECTION[i]))    // Type_Checking
        {
            setting.value       = (void*)l_txCalParam.VSA_CONNECTION[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the actual connection between DUT antenna ports and VSAs port.\r\nValid value is TX1, TX2, TX3, TX4 and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSA%d_CONNECTION", i+1);
            l_txCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }

    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_txCalParam.VSA_PORT[i], MAX_BUFFER_SIZE, "");
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_txCalParam.VSA_PORT[i]))    // Type_Checking
        {
            setting.value       = (void*)l_txCalParam.VSA_PORT[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the VSA port, Default is used global setting.\r\nValid value is Left, Right and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSA%d_PORT", i+1);
            l_txCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }

    }

	//l_txCalParam.TX1 = 1;
	//setting.type = WIFI_SETTING_TYPE_INTEGER;
	//if (sizeof(int)==sizeof(l_txCalParam.TX1))    // Type_Checking
	//{
	//	setting.value       = (void*)&l_txCalParam.TX1;
	//	setting.unit        = "";
	//	setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
	//	l_txCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//l_txCalParam.TX2 = 0;
	//setting.type = WIFI_SETTING_TYPE_INTEGER;
	//if (sizeof(int)==sizeof(l_txCalParam.TX2))    // Type_Checking
	//{
	//	setting.value       = (void*)&l_txCalParam.TX2;
	//	setting.unit        = "";
	//	setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
	//	l_txCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//l_txCalParam.TX3 = 0;
	//setting.type = WIFI_SETTING_TYPE_INTEGER;
	//if (sizeof(int)==sizeof(l_txCalParam.TX3))    // Type_Checking
	//{
	//	setting.value       = (void*)&l_txCalParam.TX3;
	//	setting.unit        = "";
	//	setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
	//	l_txCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//l_txCalParam.TX4 = 0;
	//setting.type = WIFI_SETTING_TYPE_INTEGER;
	//if (sizeof(int)==sizeof(l_txCalParam.TX4))    // Type_Checking
	//{
	//	setting.value       = (void*)&l_txCalParam.TX4;
	//	setting.unit        = "";
	//	setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
	//	l_txCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	l_txCalParam.TX_CAL_TIMEOUT_MS = DEFAULT_TIMEOUT_MS;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txCalParam.TX_CAL_TIMEOUT_MS))    // Type_Checking
	{
		setting.value = (void*)&l_txCalParam.TX_CAL_TIMEOUT_MS;
		setting.unit        = "MS";
		setting.helpText    = "timeout when calibration fail";
		l_txCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_CAL_TIMEOUT_MS", setting));
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

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txCalReturn.CABLE_LOSS_DB[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txCalReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txCalReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txCalReturn.CAL_RESULT_1[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txCalReturn.CAL_RESULT_1))    // Type_Checking
	{
		setting.value       = (void*)l_txCalReturn.CAL_RESULT_1;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 1";
		l_txCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_1", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txCalReturn.CAL_RESULT_2[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txCalReturn.CAL_RESULT_2))    // Type_Checking
	{
		setting.value       = (void*)l_txCalReturn.CAL_RESULT_2;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 2";
		l_txCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_2", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txCalReturn.CAL_RESULT_3[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txCalReturn.CAL_RESULT_3))    // Type_Checking
	{
		setting.value       = (void*)l_txCalReturn.CAL_RESULT_3;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 3";
		l_txCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_3", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txCalReturn.CAL_RESULT_4[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txCalReturn.CAL_RESULT_4))    // Type_Checking
	{
		setting.value       = (void*)l_txCalReturn.CAL_RESULT_4;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 4";
		l_txCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_4", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txCalReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txCalReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_txCalReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_txCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    return 0;
}

//This is the function to do analysis when TX calibration and put the result to DUT control layer
int TX_Cal_GetPowerAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb)
{
	int     err = ERR_OK;
    char    logMessage[MAX_BUFFER_SIZE] = {'\0'};

    //Step1: Insert Keyword into EVM Container 
    ::TM_ClearParameters(g_WiFi_Test_ID);
         
	::TM_AddIntegerParameter(g_WiFi_Test_ID, "BSS_FREQ_MHZ_PRIMARY",      tagInput.BSS_FREQ_MHZ_PRIMARY);
	::TM_AddIntegerParameter(g_WiFi_Test_ID, "BSS_FREQ_MHZ_SECONDARY",    tagInput.BSS_FREQ_MHZ_SECONDARY);
	::TM_AddIntegerParameter(g_WiFi_Test_ID, "CH_FREQ_MHZ",				  tagInput.CH_FREQ_MHZ);
	::TM_AddIntegerParameter(g_WiFi_Test_ID, "CH_FREQ_MHZ_PRIMARY_20MHz", tagInput.CH_FREQ_MHZ_PRIMARY_20MHz);
	::TM_AddIntegerParameter(g_WiFi_Test_ID, "NUM_STREAM_11AC",			  1);
	
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX1",           tagInput.TX1);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX2",           tagInput.TX2);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX3",           tagInput.TX3);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX4",           tagInput.TX4);

    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_VSA1", l_txCalParam.CABLE_LOSS_DB[0]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_VSA2", l_txCalParam.CABLE_LOSS_DB[1]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_VSA3", l_txCalParam.CABLE_LOSS_DB[2]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_VSA4", l_txCalParam.CABLE_LOSS_DB[3]);
    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "SAMPLING_TIME_US", l_txCalParam.SAMPLING_TIME_US);
    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "TX_POWER_DBM",  tagInput.TX_POWER_DBM);

    //::TM_AddStringParameter (g_WiFi_Test_ID, "BANDWIDTH",     tagInput.BANDWIDTH);
	::TM_AddStringParameter (g_WiFi_Test_ID, "BSS_BANDWIDTH",     tagInput.BSS_BANDWIDTH);
	::TM_AddStringParameter (g_WiFi_Test_ID, "CH_BANDWIDTH",     tagInput.CH_BANDWIDTH);

    ::TM_AddStringParameter (g_WiFi_Test_ID, "DATA_RATE",     tagInput.DATA_RATE);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "PACKET_FORMAT", tagInput.PACKET_FORMAT);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "PREAMBLE", tagInput.PREAMBLE);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA1_CONNECTION", l_txCalParam.VSA_CONNECTION[0]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA2_CONNECTION", l_txCalParam.VSA_CONNECTION[1]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA3_CONNECTION", l_txCalParam.VSA_CONNECTION[2]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA4_CONNECTION", l_txCalParam.VSA_CONNECTION[3]);

    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA1_PORT", l_txCalParam.VSA_PORT[0]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA2_PORT", l_txCalParam.VSA_PORT[1]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA3_PORT", l_txCalParam.VSA_PORT[2]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA4_PORT", l_txCalParam.VSA_PORT[3]);

    //Step2: Disable DUT Config in Wifi Tx Verify POWER
    //g_txCalibrationEnabled = true;

    //Step3: Run Tx_Verify_Power
    err = WiFi_TX_Verify_Power();

    //Has done tx_calibration no matter is successfuly or fail.
    //g_txCalibrationEnabled = false;

    if(err != ERR_OK)
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Function WiFi_TX_Verify_Power() Failed.\n");
	}
	else
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Function WiFi_TX_Verify_Power() OK.\n");
	}

    //Step4: Get Result from EVM return map
    PassTestResultToDutInputContainer(l_txVerifyPowerReturnMap);

	//Step5: TODO error code
	return err;




}
//This is the function to do EVM analysis when TX calibration and put the result to DUT control layer
int TX_Cal_GetEVMAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb)
{

	int     err = ERR_OK;
    char    logMessage[MAX_BUFFER_SIZE] = {'\0'};

    //Step1: Insert Keyword into EVM Container 
    ::TM_ClearParameters(g_WiFi_Test_ID);
         
    //::TM_AddIntegerParameter(g_WiFi_Test_ID, "FREQ_MHZ",      tagInput.FREQ_MHZ);
	::TM_AddIntegerParameter(g_WiFi_Test_ID, "BSS_FREQ_MHZ_PRIMARY",      tagInput.BSS_FREQ_MHZ_PRIMARY);
	::TM_AddIntegerParameter(g_WiFi_Test_ID, "BSS_FREQ_MHZ_SECONDARY",    tagInput.BSS_FREQ_MHZ_SECONDARY);
	::TM_AddIntegerParameter(g_WiFi_Test_ID, "CH_FREQ_MHZ",				  tagInput.CH_FREQ_MHZ);
	::TM_AddIntegerParameter(g_WiFi_Test_ID, "CH_FREQ_MHZ_PRIMARY_20MHz", tagInput.CH_FREQ_MHZ_PRIMARY_20MHz);
	::TM_AddIntegerParameter(g_WiFi_Test_ID, "NUM_STREAM_11AC",			  1);

    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX1",           tagInput.TX1);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX2",           tagInput.TX2);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX3",           tagInput.TX3);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX4",           tagInput.TX4);

    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_VSA1", l_txCalParam.CABLE_LOSS_DB[0]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_VSA2", l_txCalParam.CABLE_LOSS_DB[1]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_VSA3", l_txCalParam.CABLE_LOSS_DB[2]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_VSA4", l_txCalParam.CABLE_LOSS_DB[3]);
    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "SAMPLING_TIME_US", l_txCalParam.SAMPLING_TIME_US);
    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "TX_POWER_DBM",  tagInput.TX_POWER_DBM);

    //::TM_AddStringParameter (g_WiFi_Test_ID, "BANDWIDTH",     tagInput.BANDWIDTH);
	::TM_AddStringParameter (g_WiFi_Test_ID, "BSS_BANDWIDTH",     tagInput.BSS_BANDWIDTH);
	::TM_AddStringParameter (g_WiFi_Test_ID, "CH_BANDWIDTH",     tagInput.CH_BANDWIDTH);

    ::TM_AddStringParameter (g_WiFi_Test_ID, "DATA_RATE",     tagInput.DATA_RATE);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "PACKET_FORMAT", tagInput.PACKET_FORMAT);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "PREAMBLE", tagInput.PREAMBLE);

	//TODO: DECIDE "TX_ANALYSIS_ORDER" PUT INTO CALIBRATION INPUT CONTAINER OR RETURN BY VDUT?
	::TM_AddStringParameter (g_WiFi_Test_ID, "TX_ANALYSIS_ORDER", "-1,-1,-1,-1");

    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA1_CONNECTION", l_txCalParam.VSA_CONNECTION[0]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA2_CONNECTION", l_txCalParam.VSA_CONNECTION[1]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA3_CONNECTION", l_txCalParam.VSA_CONNECTION[2]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA4_CONNECTION", l_txCalParam.VSA_CONNECTION[3]);

    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA1_PORT", l_txCalParam.VSA_PORT[0]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA2_PORT", l_txCalParam.VSA_PORT[1]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA3_PORT", l_txCalParam.VSA_PORT[2]);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "VSA4_PORT", l_txCalParam.VSA_PORT[3]);

    //Step2: Disable DUT Config in Wifi Tx Verify POWER
    //g_txCalibrationEnabled = true;

    //Step3: Run Tx_Verify_EVM
    err = WiFi_11ac_MiMo_TX_Verify_Evm();

    //Has done tx_calibration no matter is successfuly or fail.
    //g_txCalibrationEnabled = false;

    if(err != ERR_OK)
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Function WiFi_TX_Verify_Evm() Failed.\n");
	}
	else
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Function WiFi_TX_Verify_Evm() OK.\n");
	}

    //Step4: Get Result from EVM return map
    PassTestResultToDutInputContainer(l_txVerifyEvmReturnMap);

	//Step5: TODO error code
	return err;




}

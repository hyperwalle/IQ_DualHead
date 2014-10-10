#include "stdafx.h"
#include "IQlite_Logger.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <math.h>
#include <Windows.h>


using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_rxCalParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_rxCalReturnMap;


double	l_pathLossRecordRxCal[MAX_DATA_STREAM];

struct tagParam
{
    // Mandatory Parameters
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. Default=0 dB */
    double SAMPLING_TIME_US;                        /*! The sampling time to do measurement. */ 
    double RX_CAL_TIMEOUT_MS;                       /*! The timeout to do RX calibration. Default=600s */ 

} l_rxCalParam;

struct tagReturn
{
    double	CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. Default=0 dB */
	char	CAL_RESULT_1[MAX_BUFFER_SIZE];
	char	CAL_RESULT_2[MAX_BUFFER_SIZE];
	char	CAL_RESULT_3[MAX_BUFFER_SIZE];
	char	CAL_RESULT_4[MAX_BUFFER_SIZE];
    char	ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_rxCalReturn;

void ClearrxCalReturn(void)
{
	l_rxCalParamMap.clear();
	l_rxCalReturnMap.clear();
}

//! WiFi RX Calibration
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

//#define RX_CALIBRATION_DEBUG
#ifdef RX_CALIBRATION_DEBUG
char g_strMsg[MAX_BUFFER_SIZE] = "";
#endif //RX_CALIBRATION_DEBUG

struct __timeb64 rxStartTime, rxStopTime;
int   l_iRxEclipseTimeMs;

//GET PARAMETER FROM DUTs
typedef struct tagAnalysisInputParameterType
{
	// Mandatory Parameters
	int    FREQ_MHZ;                                /*! The center frequency (MHz). */  
	char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify RX. */
	char   DATA_RATE[MAX_BUFFER_SIZE];              /*! The data rate to verify Rx. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];      /*! The packet format of 11N(only). */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];     /*! The guard interval of 11N(only). */
	double RX_POWER_DBM;                            /*! The output power to verify RX. */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	char   WAVE_TYPE[MAX_BUFFER_SIZE];				/*! Support "CW" and "WIFI" signal type */
	//char   WAVEFORM_FILE_NAME[MAX_BUFFER_SIZE];		/*! For RX calibration, a Waveform name (*.mod) is required. If the value equal to AUTO that means using GLOBAL_SETTINGS->WIFI_WAVEFORM_NAME. */

	// Tester Parameters
	int    VSG_PORT;

	// DUT Parameters
	int    RX1;                                     /*! DUT RX1 on/off. Default=1(on)  */
	int    RX2;                                     /*! DUT RX2 on/off. Default=0(off) */
	int    RX3;                                     /*! DUT RX3 on/off. Default=0(off) */
	int    RX4;                                     /*! DUT RX4 on/off. Default=0(off) */

}tagAnalysisInputParameter;

typedef struct tagAnalysisOutputType
{
    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
}tagAnalysisOutput;

#define DEFAULT_TIMEOUT_MS 600000                     //default timeout value: 600s 

const int ciOffset = 1;
int GenerateSignal(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb);

WIFI_TEST_API int WiFi_RX_Calibration(void)
{
    int    err = ERR_OK;
    int    dummyValue;  
	double cableLossDb = 0;
	char   logMessage[MAX_BUFFER_SIZE] =  {'\0'};
	int    defaultVsgPort = PORT_LEFT;

   /*---------------------------------------*
    * Clear Return Parameters and Container *
    *---------------------------------------*/
    ClearReturnParameters(l_rxCalReturnMap);	

   /*------------------------*
    * Respond to QUERY_INPUT *
    *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err)
    {
        RespondToQueryInput(l_rxCalParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

   /*-------------------------*
    * Respond to QUERY_RETURN *
    *-------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err)
    {
        RespondToQueryReturn(l_rxCalReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*-----------------------------------------------------------*
		* Both g_WiFi_Test_ID and g_WiFi_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_WiFi_Test_ID<0 || g_WiFi_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID or WiFi_Dut not valid. WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		}

		TM_ClearReturns(g_WiFi_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_rxCalParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get input parameters return OK.\n");
		}

		// Error return of this function is irrelevant
		CheckDutTransmitStatus();

		// Record the path loss for Rx calibration
		for (int i=0;i<MAX_DATA_STREAM;i++)
		{
			l_pathLossRecordRxCal[i] = l_rxCalParam.CABLE_LOSS_DB[i];
		}

	   /*-----------------------*
		*   Do Calibration      *
		*-----------------------*/
#pragma region configure DUT to calibration and begin timer
		// For backward compatibility, we still call "GET_RX_CAL_SETTING", but won't report error, if failed.
		err = vDUT_Run(g_WiFi_Dut, "GET_RX_CAL_SETTING");
		if (ERR_OK!=err)
		{
			err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] This vDut control not support (GET_RX_CAL_SETTING).\n");
		}
		else
		{			
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(GET_RX_CAL_SETTING) return OK.\n");
			// TODO: Get parameters here
		}

		_ftime64_s (&rxStartTime);  
#pragma endregion	

#pragma region measurement till done
		int    iCalDone      = 0;
		int    iCalAbort     = 0;
		tagAnalysisInputParameter   tagInputPara;
		tagAnalysisOutput           tagOutputPara;
		tagInputPara.FREQ_MHZ       = 2412; 
		tagInputPara.RX_POWER_DBM   = -20; 
		strcpy_s(tagInputPara.DATA_RATE,MAX_BUFFER_SIZE, ""); 
		strcpy_s(tagInputPara.BANDWIDTH,MAX_BUFFER_SIZE, ""); 
		strcpy_s(tagInputPara.PACKET_FORMAT_11N,MAX_BUFFER_SIZE, "");   
		strcpy_s(tagInputPara.GUARD_INTERVAL_11N,MAX_BUFFER_SIZE, "");   


		/*---------------------------------------------------------*/
		/* Before start calibrations, keep previous global setting */
		/* Now, just keep VSG_PORT status before Rx calibration    */
		/* TODO: keep all previous global setting                  */
		/*---------------------------------------------------------*/
		defaultVsgPort = g_WiFiGlobalSettingParam.VSG_PORT;

		while(1)
		{
			_ftime64_s (&rxStopTime);

			l_iRxEclipseTimeMs = (unsigned int) (((rxStopTime.time - rxStartTime.time) * 1000) + (rxStopTime.millitm - rxStartTime.millitm));

			if (l_iRxEclipseTimeMs>l_rxCalParam.RX_CAL_TIMEOUT_MS)
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] RX power calibration timeout! Abort.\n");
				throw logMessage;
			}

			// Set calibration point and transmit
			err = vDUT_Run(g_WiFi_Dut, "RX_CALIBRATION");
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_CALIBRATION) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_CALIBRATION) return OK.\n");
			}

			//Get the measure parameter from DUT control layer
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "CAL_DONE", &iCalDone);				 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetIntegerReturn(CAL_DONE) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(CAL_DONE=%d) return OK.\n", iCalDone);
			}
			if(iCalDone)
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Dut report Calibration Done.\n");
				break;
			}

			//Get the measure parameter from DUT control layer
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "CAL_ABORT", &iCalAbort);				 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetIntegerReturn(CAL_ABORT) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(CAL_ABORT=%d) return OK.\n", iCalAbort);
			}
			if(iCalAbort)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Dut report Abort Calibration.\n");
				throw logMessage;
			}

			/*--------------------------------------------------------*/
			/*    Get the measure parameter from DUT control layer    */
			/*--------------------------------------------------------*/
			// Frequency
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "FREQ_MHZ",  &tagInputPara.FREQ_MHZ);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetIntegerReturn(FREQ_MHZ) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(FREQ_MHZ=%d) return OK.\n", tagInputPara.FREQ_MHZ);
			}

			// MEASURE_TYPE
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "WAVE_TYPE", tagInputPara.WAVE_TYPE, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetStringReturn(WAVE_TYPE) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(WAVE_TYPE=%s) return OK.\n", tagInputPara.WAVE_TYPE);
			}

			if ( 0==strcmp(tagInputPara.WAVE_TYPE, "CW") )
			{
				sprintf_s(tagInputPara.DATA_RATE, MAX_BUFFER_SIZE, "NONE"); 
				sprintf_s(tagInputPara.BANDWIDTH, MAX_BUFFER_SIZE, "NONE");
			}
			else	// WIFI
			{
				// Data rate
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "DATA_RATE", tagInputPara.DATA_RATE, MAX_BUFFER_SIZE);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetStringReturn(DATA_RATE) return error.\n");
					throw logMessage;
				}
				else
				{
					//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
					_strupr_s(tagInputPara.DATA_RATE, MAX_BUFFER_SIZE);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(DATA_RATE=%s) return OK.\n", tagInputPara.DATA_RATE);
				}

				// CHANNEL_BW
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "CHANNEL_BW", tagInputPara.BANDWIDTH, MAX_BUFFER_SIZE);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetStringReturn(CHANNEL_BW) return error.\n");
					throw logMessage;
				}
				else
				{
					//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
					_strupr_s(tagInputPara.BANDWIDTH, MAX_BUFFER_SIZE);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(CHANNEL_BW=%s) return OK.\n", tagInputPara.BANDWIDTH);
				}
			}

			// RX power
			err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, "RX_POWER_DBM",  &tagInputPara.RX_POWER_DBM);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetIntegerReturn(RX_POWER_DBM) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(RX_POWER_DBM=%.1f) return OK.\n", tagInputPara.RX_POWER_DBM);
			}		

			// NOTE: IF CAN'T GET FOLLOWING KEYWORD FROM VDUT, THEN USING DEFAULT RX_CALIBRATION INPUT PARAMETERS.
			//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			// RX1
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "RX1",  &tagInputPara.RX1);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				tagInputPara.RX1 = 1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (RX1), default set the RX1 = ON.\n");
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(RX1=%d) return OK.\n", tagInputPara.RX1);
			}

			// RX2
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "RX2",  &tagInputPara.RX2);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				tagInputPara.RX2 = 0;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (RX2), default set the RX2 = OFF.\n");
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(RX2=%d) return OK.\n", tagInputPara.RX2);
			}

			// RX3
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "RX3",  &tagInputPara.RX3);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				tagInputPara.RX3 = 0;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (RX3), default set the RX3 = OFF.\n");
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(RX3=%d) return OK.\n", tagInputPara.RX3);
			}

			// RX4
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "RX4",  &tagInputPara.RX4);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				tagInputPara.RX4 = 0;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (RX4), default set the RX4 = OFF.\n");
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(RX4=%d) return OK.\n", tagInputPara.RX4);
			}

			// PREAMBLE
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "PREAMBLE", tagInputPara.PREAMBLE, MAX_BUFFER_SIZE);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				sprintf_s(tagInputPara.PREAMBLE , MAX_BUFFER_SIZE, "LONG");
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (PREAMBLE), default set the PREAMBLE = LONG.\n");
			} 
			else
			{
				//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
				_strupr_s(tagInputPara.PREAMBLE, MAX_BUFFER_SIZE);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(PREAMBLE=%s) return OK.\n", tagInputPara.PREAMBLE);
			}

			//PACKET_FORMAT_11N
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "PACKET_FORMAT_11N", tagInputPara.PACKET_FORMAT_11N, MAX_BUFFER_SIZE);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				sprintf_s(tagInputPara.PACKET_FORMAT_11N , MAX_BUFFER_SIZE, "MIXED");
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (PACKET_FORMAT_11N), default set the PACKET_FORMAT_11N = MIXED.\n");
			} 
			else
			{
				//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
				_strupr_s(tagInputPara.PACKET_FORMAT_11N, MAX_BUFFER_SIZE);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(PACKET_FORMAT_11N=%s) return OK.\n", tagInputPara.PACKET_FORMAT_11N);
			}

			//GUARD_INTERVAL_11N
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "GUARD_INTERVAL_11N", tagInputPara.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				sprintf_s(tagInputPara.GUARD_INTERVAL_11N , MAX_BUFFER_SIZE, "LONG");
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (GUARD_INTERVAL_11N), default set the GUARD_INTERVAL_11N = LONG.\n");
			} 
			else
			{
				//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
				_strupr_s(tagInputPara.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(GUARD_INTERVAL_11N=%s) return OK.\n", tagInputPara.GUARD_INTERVAL_11N);
			}

			//WAVEFORM_FILE_NAME
			//err = ::vDUT_GetStringReturn(g_WiFi_Dut, "WAVEFORM_FILE_NAME", tagInputPara.WAVEFORM_FILE_NAME, MAX_BUFFER_SIZE);
			//if (ERR_OK!=err)
			//{
			//	err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
			//	sprintf_s(tagInputPara.WAVEFORM_FILE_NAME , MAX_BUFFER_SIZE, "AUTO");
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (WAVEFORM_FILE_NAME), default set the WAVEFORM_FILE_NAME = AUTO.\n");
			//} 
			//else
			//{
			//	//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
			//	_strupr_s(tagInputPara.WAVEFORM_FILE_NAME, MAX_BUFFER_SIZE);
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(WAVEFORM_FILE_NAME=%s) return OK.\n", tagInputPara.WAVEFORM_FILE_NAME);
			//}
			//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

			// Check path loss (by ant and freq)
			for (int i=0;i<MAX_DATA_STREAM;i++)
			{
				l_rxCalParam.CABLE_LOSS_DB[i] = l_pathLossRecordRxCal[i];
			}		

			err = CheckPathLossTableExt(    g_WiFi_Test_ID,
											tagInputPara.FREQ_MHZ,
											tagInputPara.RX1,
											tagInputPara.RX2,
											tagInputPara.RX3,
											tagInputPara.RX4,
											l_rxCalParam.CABLE_LOSS_DB,
											l_rxCalReturn.CABLE_LOSS_DB,
											&cableLossDb,
											RX_TABLE
										);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get RX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] CheckPathLossTableExt(cableLossDb=%.1f) return OK.\n", cableLossDb);
			}

			/*--------------------------------------------------------------------------------------------
			 CHANGE GLOBAL SETTING:																      
			 While calibration, DUT may change GLOBAL setting, such like port, sampling times, and etc. 
			 Get the GLOBAL seeting from DUT control:                    
			 1. VSG_PORT                                                                             
			 2. ...                                                                                  
			 TODO: deal more VSG setting                                                             
			--------------------------------------------------------------------------------------------*/			        
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "VSG_PORT", &tagInputPara.VSG_PORT);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				//If VSG_PORT didn't return from Dut control, VSG_PORT use default global setting.
				tagInputPara.VSG_PORT = defaultVsgPort;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (VSG_PORT), default set the VSG_PORT = %d.\n", g_WiFiGlobalSettingParam.VSG_PORT);
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(VSG_PORT=%d) return OK.\n", tagInputPara.VSG_PORT);
			}

			/*---------------------------------------------------
			 CHANGE GLOBAL SETTING:
			 1. VSG_PORT
			 2. ...
			 TODO: deal more global setting  
			---------------------------------------------------*/
			::TM_ClearParameters(g_WiFi_Test_ID);
			::TM_AddIntegerParameter(g_WiFi_Test_ID, "VSG_PORT",      tagInputPara.VSG_PORT);

			/*------------------------*/
			/* CHANGE GLOBAL SETTING  */
			/*------------------------*/
			err = ::TM_Run(g_WiFi_Test_ID, "GLOBAL_SETTINGS");
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] TM_Run(GLOBAL_SETTINGS) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_Run(GLOBAL_SETTINGS) return OK.\n");
			}

			/*--------------------------------*/
			/*  Start Do the VSG RX transmit  */
			/*--------------------------------*/
			err = GenerateSignal(tagInputPara, &tagOutputPara, cableLossDb);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] The GenerateSignal( WAVE_TYPE=%s ) return error.\n", tagInputPara.WAVE_TYPE);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Start GenerateSignal( WAVE_TYPE=%s ) return OK.\n", tagInputPara.WAVE_TYPE);
			}

		}	// end of loop

		/*---------------------------------------------------
		 RESTORE PREVIOUS GLOBAL SETTING:
		 1. VSG_PORT
		 TODO: deal more previous global setting  
		---------------------------------------------------*/
		::TM_ClearParameters(g_WiFi_Test_ID);
		::TM_AddIntegerParameter(g_WiFi_Test_ID, "VSG_PORT",      defaultVsgPort);

		/*------------------------*/
		/* CHANGE GLOBAL SETTING  */
		/*------------------------*/
		err = ::TM_Run(g_WiFi_Test_ID, "GLOBAL_SETTINGS");
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] TM_Run(GLOBAL_SETTINGS) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_Run(GLOBAL_SETTINGS) return OK.\n");
		}

#pragma endregion	
	   /*-----------------------*
		*  Return Test Results  *
		*-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_rxCalReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.");
			// Get return information
			vDUT_GetStringReturn(g_WiFi_Dut, "CAL_RESULT_1", l_rxCalReturn.CAL_RESULT_1,MAX_BUFFER_SIZE);
			vDUT_GetStringReturn(g_WiFi_Dut, "CAL_RESULT_2", l_rxCalReturn.CAL_RESULT_2,MAX_BUFFER_SIZE);
			vDUT_GetStringReturn(g_WiFi_Dut, "CAL_RESULT_3", l_rxCalReturn.CAL_RESULT_3,MAX_BUFFER_SIZE);
			vDUT_GetStringReturn(g_WiFi_Dut, "CAL_RESULT_4", l_rxCalReturn.CAL_RESULT_4,MAX_BUFFER_SIZE);
			ReturnTestResults(l_rxCalReturnMap);
		} 
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		vDUT_Run(g_WiFi_Dut, "CALIBRATION_FAILED");	 // Clear calibration status. Doesn't need to do error check, in case some other dut control doesn't support this function.
		ReturnErrorMessage(l_rxCalReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		vDUT_Run(g_WiFi_Dut, "CALIBRATION_FAILED");	 // Clear calibration status. Doesn't need to do error check, in case some other dut control doesn't support this function.
		ReturnErrorMessage(l_rxCalReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
	}

    return err;
}

int InitializeRXCalContainers(void)
{
    /*------------------*
    * Input Paramters: *
    * IQTESTER_IP01    *
    *------------------*/
    l_rxCalParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    l_rxCalParam.SAMPLING_TIME_US = 0.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxCalParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&l_rxCalParam.SAMPLING_TIME_US;
        setting.unit        = "uS";
        setting.helpText    = "Capture time in micro-seconds";
        l_rxCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_rxCalParam.CABLE_LOSS_DB[i] = 0.0;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_rxCalParam.CABLE_LOSS_DB[i]))    // Type_Checking
        {
            setting.value       = (void*)&l_rxCalParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Cable loss from the DUT antenna port to tester";
            l_rxCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    l_rxCalParam.RX_CAL_TIMEOUT_MS = DEFAULT_TIMEOUT_MS;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxCalParam.RX_CAL_TIMEOUT_MS))    // Type_Checking
    {
        setting.value = (void*)&l_rxCalParam.RX_CAL_TIMEOUT_MS;
        setting.unit        = "MS";
        setting.helpText    = "timeout when calibration fail";
        l_rxCalParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX_CAL_TIMEOUT_MS", setting));
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
    l_rxCalReturnMap.clear();

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_rxCalReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_rxCalReturn.CABLE_LOSS_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_rxCalReturn.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Cable loss from the DUT antenna port to tester";
            l_rxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    l_rxCalReturn.CAL_RESULT_1[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxCalReturn.CAL_RESULT_1))    // Type_Checking
    {
        setting.value       = (void*)l_rxCalReturn.CAL_RESULT_1;
        setting.unit        = "";
        setting.helpText    = "Calibration Result 1";
        l_rxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_1", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxCalReturn.CAL_RESULT_2[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxCalReturn.CAL_RESULT_2))    // Type_Checking
	{
		setting.value       = (void*)l_rxCalReturn.CAL_RESULT_2;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 2";
		l_rxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_2", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxCalReturn.CAL_RESULT_3[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxCalReturn.CAL_RESULT_3))    // Type_Checking
	{
		setting.value       = (void*)l_rxCalReturn.CAL_RESULT_3;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 3";
		l_rxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_3", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxCalReturn.CAL_RESULT_4[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxCalReturn.CAL_RESULT_4))    // Type_Checking
	{
		setting.value       = (void*)l_rxCalReturn.CAL_RESULT_4;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 4";
		l_rxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAL_RESULT_4", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	
    l_rxCalReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxCalReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_rxCalReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_rxCalReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

int GenerateSignal(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb)
{
	int  err = ERR_OK;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};
	int  wifiMode = WIFI_11AG;
	int  wifiStreamNum = WIFI_ONE_STREAM;
	double MaxVSGPowerlimit = 0.0;
	double vsgPower = 0.0;

	try
	{
	   /*----------------*
		*  Turn off VSG  *
		*----------------*/
		err = ::LP_EnableVsgRF(0);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_EnableVsgRF(0) return OK.\n");
		}

		if ( 0==strcmp(tagInput.WAVE_TYPE, "WIFI") )
		{
			//if ( (0==strcmp(tagInput.WAVEFORM_FILE_NAME,""))||(0==strcmp(tagInput.WAVEFORM_FILE_NAME,"NULL")) )
			//{
			//	// If WAVE_TYPE = WIFI, but the user won't input the WAVEFORM_FILE_NAME name, then must return an error message.
			//	err = -1;
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] The Rx [WAVEFORM_FILE_NAME] can NOT be empty.\n");
			//	throw logMessage;
			//}
			//else
			{
			   /*---------------------------*
				* Configure DUT to Receive  *
				*---------------------------*/
				vDUT_AddStringParameter (g_WiFi_Dut, "PER_WAVEFORM_DESTINATION_MAC", g_WiFiGlobalSettingParam.PER_WAVEFORM_DESTINATION_MAC);

				// Convert parameter
				err = WiFiTestMode(tagInput.DATA_RATE, tagInput.BANDWIDTH, &wifiMode, &wifiStreamNum);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Convert WiFi test mode function failed.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFiTestMode() return OK.\n");
				}

			   /*------------------------------------*
				* check VSG output power limit first *
				*------------------------------------*/
				if( wifiMode==WIFI_11B )
				{
					MaxVSGPowerlimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11B;
				}
				else if( wifiMode==WIFI_11AG )
				{
					MaxVSGPowerlimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11G; 
				}
				else
				{
					MaxVSGPowerlimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11N; 
				}
			
				vsgPower = tagInput.RX_POWER_DBM + cableLossDb;
				// Check VSG power will exceed VSG_MAX_POWER_WiFi or not
				if ( vsgPower>MaxVSGPowerlimit )
				{					
					err = -1;	//ERR_VSG_POWER_EXCEED_LIMIT
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", vsgPower, MaxVSGPowerlimit);
					throw logMessage;
				}
				else
				{
					// do nothing
				}

			   /*---------------------*
				* Load waveform Files *
				*---------------------*/
				char  modFile[MAX_BUFFER_SIZE] = "\0";
				err = GetWaveformFileName(  "PER", 
											"WAVEFORM_NAME", 
											wifiMode, 
											tagInput.BANDWIDTH, 
											tagInput.DATA_RATE, 
											tagInput.PREAMBLE, 
											tagInput.PACKET_FORMAT_11N,
											tagInput.GUARD_INTERVAL_11N,
											modFile, 
											MAX_BUFFER_SIZE);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get waveform file name, GetWaveformFileName() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] GetWaveformFileName() return OK.\n");
				}

				// Load the whole MOD file for continuous transmit
				err = ::LP_SetVsgModulation( modFile );
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SetVsgModulation( %s ) return error.\n", modFile);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsgModulation( %s ) return OK.\n", modFile);
				}

			   /*--------------------*
				* Setup IQTester VSG *
				*--------------------*/
				// Since the limitation, we assume that all path loss value are very close.
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Set VSG port by g_WiFiGlobalSettingParam.VSG_PORT=[%d].\n", g_WiFiGlobalSettingParam.VSG_PORT);
				err = ::LP_SetVsg(tagInput.FREQ_MHZ*1e6, vsgPower, g_WiFiGlobalSettingParam.VSG_PORT);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to setup VSG, LP_SetVsg() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsg() return OK.\n");
				}
			}	// end - if ( (0==strcmp(tagInput.WAVEFORM_FILE_NAME,""))||(0==strcmp(tagInput.WAVEFORM_FILE_NAME,"NULL")) )

		}
		else	// WAVE_TYPE != WiFi, means => CW signal
		{
		   /*------------------------------------*
			* check VSG output power limit first *
			*------------------------------------*/
			// TODO: Using the "VSG_MAX_POWER_11B" as CW signal "MaxVSGPowerlimit". 
			MaxVSGPowerlimit = g_WiFiGlobalSettingParam.VSG_MAX_POWER_11B;
		
			vsgPower = tagInput.RX_POWER_DBM + cableLossDb;
			// Check VSG power will exceed VSG_MAX_POWER_WiFi or not
			if ( vsgPower>MaxVSGPowerlimit )
			{					
				err = -1;	//ERR_VSG_POWER_EXCEED_LIMIT
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", vsgPower, MaxVSGPowerlimit);
				throw logMessage;
			}
			else
			{
				// do nothing
			}

			// Since there is a bug inside IQapi, the CW signal always can shift -1 MHz offset only in 2.4G.
			// So, we can fix it after using (tagInput.FREQ_MHZ+ciOffset)*1e6 as the center freq. and -ciOffset as the offset freq for LP_SetVsgCw() function.
		    // Or we need to Set VSG twice to fix this issue.
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Set VSG port by g_WiFiGlobalSettingParam.VSG_PORT=[%d].\n", g_WiFiGlobalSettingParam.VSG_PORT);
			err = ::LP_SetVsgCw ( (tagInput.FREQ_MHZ+ciOffset)*1e6, -ciOffset, vsgPower, g_WiFiGlobalSettingParam.VSG_PORT );
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to setup VSG, LP_SetVsgCw(CW signal) at %d MHz return error.\n", tagInput.FREQ_MHZ);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Set LP_SetVsg(CW signal) at %d MHz return OK.\n", tagInput.FREQ_MHZ);
			}
		}

	   /*---------------*
		*  Turn on VSG  *
		*---------------*/
		err = ::LP_EnableVsgRF(1);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn on VSG, LP_EnableVsgRF(1) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_EnableVsgRF(1) return OK.\n");
		}

	   /*----------------------*
		* Send packet for Test *
		*----------------------*/  
		err = ::LP_SetFrameCnt(0);		// continuous transmit
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] VSG send out packet LP_SetFrameCnt(0) return error.\n" );
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] VSG send out packet LP_SetFrameCnt(0) return OK.\n" );
		}
	
	}
	catch(char *msg)
    {
        ReturnErrorMessage(tagOutput->ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(tagOutput->ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
    }


	return err;
}
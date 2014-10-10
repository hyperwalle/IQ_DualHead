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
//Move to stdafx.h
//#include "lp_highrestimer.h"

int	l_vDutEnabled = 1;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_txCalParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_txCalReturnMap;

// Return Value Container
extern  map<string, WIFI_SETTING_STRUCT>		l_txVerifyPowerReturnMap;
extern  map<string, WIFI_SETTING_STRUCT>		l_txVerifyEvmReturnMap;

double	l_pathLossRecord[MAX_DATA_STREAM];

struct tagParam
{
    // Mandatory Parameters
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. Default=0 dB */
    double SAMPLING_TIME_US;                        /*! The sampling time to do measurement. */ 
    double TX_CAL_TIMEOUT_MS;                       /*! The timeout to do TX calibration. Default=600s */ 

} l_txCalParam;

struct tagReturn
{
    double	CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. Default=0 dB */
	char	CAL_RESULT_1[MAX_BUFFER_SIZE];
	char	CAL_RESULT_2[MAX_BUFFER_SIZE];
	char	CAL_RESULT_3[MAX_BUFFER_SIZE];
	char	CAL_RESULT_4[MAX_BUFFER_SIZE];
    char	ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txCalReturn;

void ClearTxCalReturn(void)
{
	l_txCalParamMap.clear();
	l_txCalReturnMap.clear();
}

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

//GET PARAMETER FROM DUTs
typedef struct tagAnalysisInputParameterType
{
    int    FREQ_MHZ;                                /*! The center frequency (MHz).*/
    int    VSA_PORT;
	char   DATA_RATE[MAX_BUFFER_SIZE];			    /*! The data rate to verify POWER. */
    double TX_POWER_DBM;      
    double SAMPLING_TIME_US;                        /*! The sampling time to do measurement. */ 
    char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify POWER. */
    char   PREAMBLE[MAX_BUFFER_SIZE];
    char   MEASURE_TYPE[MAX_BUFFER_SIZE];
    char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];     /*! The guard interval of 11N(only). */


    //DUT Parameters
    int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on) */
    int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */
    int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */


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
       
    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
}tagAnalysisOutput;

#define DEFAULT_TIMEOUT_MS 600000                     //default timeout value: 600s 

int TX_Cal_GetPowerAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb);
int TX_Cal_GetEVMAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb);


WIFI_TEST_API int WiFi_TX_Calibration(void)
{
    int    err = ERR_OK;
    int    dummyValue;  
	double cableLossDb = 0;
	char   logMessage[MAX_BUFFER_SIZE] =  {'\0'};
	char   vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};  // -cfy@sunnyvale, 2012/3/13-
	int    defaultVsaPort = PORT_LEFT;

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
		err = GetInputParameters(l_txCalParamMap);
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

		// Record the path loss for Tx calibration
		for (int i=0;i<MAX_DATA_STREAM;i++)
		{
			l_pathLossRecord[i] = l_txCalParam.CABLE_LOSS_DB[i];
		}

	   /*-----------------------*
		*   Do Calibration      *
		*-----------------------*/
#pragma region configure DUT to calibration and begin timer
		// RW-2009-12-11: GET_CAL_SETTING has been merged into TX_CALIBRATION
		// For backward compatibility, we still call "GET_CAL_SETTING", but won't report error, if failed.
		err = vDUT_Run(g_WiFi_Dut, "GET_CAL_SETTING");
		if (ERR_OK!=err)
		{
			err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] This vDut control not support (GET_CAL_SETTING).\n");
		}
		else
		{			
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(GET_CAL_SETTING) return OK.\n");
			// TODO: Get parameters here
		}

    	highrestimer::GetTime(start_time);  // -cfy@sunnyvale, 2012/3/13-
#pragma endregion	

#pragma region measurement till done
		int    iCalDone      = 0;
		int    iCalAbort     = 0;
		tagAnalysisInputParameter   tagInputPara;
		tagAnalysisOutput           tagOutputPara;
		tagInputPara.FREQ_MHZ       = 2412; 
		tagInputPara.TX_POWER_DBM   = 15; 
		strcpy_s(tagInputPara.DATA_RATE,MAX_BUFFER_SIZE, ""); 
		strcpy_s(tagInputPara.BANDWIDTH,MAX_BUFFER_SIZE, ""); 
		strcpy_s(tagInputPara.PACKET_FORMAT_11N,MAX_BUFFER_SIZE, "");   
		strcpy_s(tagInputPara.GUARD_INTERVAL_11N,MAX_BUFFER_SIZE, "");   

		for(int i=0;i<MAX_DATA_STREAM;i++)    tagOutputPara.POWER_AVG[i] = NA_NUMBER;

		/*---------------------------------------------------------*/
		/* Before start calibrations, keep previous global setting */
		/* Now, just keep VSA_PORT status before calibration       */
		/* TODO: keep all previous global setting                  */
		/*---------------------------------------------------------*/
		defaultVsaPort = g_WiFiGlobalSettingParam.VSA_PORT;

	    for(;;)
		{
			TM_ClearReturns(g_WiFi_Test_ID); // -cfy@sunnyvale, 2012/3/13-

			highrestimer::GetTime (stop_time); // -cfy@sunnyvale, 2012/3/13-

			//org: l_iTxEclipseTimeMs = (unsigned int) (((txStopTime.time - txStartTime.time) * 1000) + (txStopTime.millitm - txStartTime.millitm));
			l_iEclipseTimeMs = highrestimer::GetElapsedMSec(start_time, stop_time); //TBD: use original one or new one?

			if (l_iEclipseTimeMs>l_txCalParam.TX_CAL_TIMEOUT_MS)
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] TX power calibration timeout! Abort.\n");
				throw logMessage;
			}

			// RW-2009-12-11: SET_CAL_MEASUREMENT has been merged into TX_CALIBRATION
			// Set calibration point and transmit
			err = vDUT_Run(g_WiFi_Dut, "TX_CALIBRATION");
			if( ERR_OK!=err || vDUT_ERR_FUNCTION_NOT_SUPPORTED==err || vDUT_ERR_FUNCTION_NOT_DEFINED==err )
			{
				// For backward compatibility, older DUT controls implemented "SET_CAL_MEASUREMENT"
				err = vDUT_Run(g_WiFi_Dut, "SET_CAL_MEASUREMENT");
				if ( ERR_OK!=err )
				{
					// Modified /* -cfy@sunnyvale, 2012/3/13- */
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_CALIBRATION) return error.\n");
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(SET_CAL_MEASUREMENT) return error.\n");
					// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetStringReturn() ERROR_MESSAGE: %s", vErrorMsg);
					}
					vDUT_Run(g_WiFi_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
					/* <><~~ */

					throw logMessage;
				}
				else
				{
					// For backward compatibility, older DUT DLL must turn off vDut control.
					l_vDutEnabled = 0;	// 0: Disabled						
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(SET_CAL_MEASUREMENT) return OK.\n");
				}
			}
			else
			{
				l_vDutEnabled = 1;	// 1: Enabled	
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_CALIBRATION) return OK.\n");
			}

			//Get the measure parameter from DUT control layer
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "CAL_DONE", &iCalDone);				 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetIntegerReturn(CAL_DONE) return error.\n");
				vDUT_Run(g_WiFi_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
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
				vDUT_Run(g_WiFi_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
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
				vDUT_Run(g_WiFi_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(FREQ_MHZ=%d) return OK.\n", tagInputPara.FREQ_MHZ);
			}

			// Data rate
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "DATA_RATE", tagInputPara.DATA_RATE, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetStringReturn(DATA_RATE) return error.\n");
				vDUT_Run(g_WiFi_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
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
				vDUT_Run(g_WiFi_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
				throw logMessage;
			}
			else
			{
				//UPPER CASE, IN CASE OF DUT GIVE LOWER CASE .
				_strupr_s(tagInputPara.BANDWIDTH, MAX_BUFFER_SIZE);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(CHANNEL_BW=%s) return OK.\n", tagInputPara.BANDWIDTH);
			}

			// TX power
			err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, "TX_POWER_DBM",  &tagInputPara.TX_POWER_DBM);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetIntegerReturn(TX_POWER_DBM) return error.\n");
				vDUT_Run(g_WiFi_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(TX_POWER_DBM=%.1f) return OK.\n", tagInputPara.TX_POWER_DBM);
			}
			
			// OLD CALIBRATION: 
			// NOT EXISTED: TX1, TX2, TX3, TX4, PREAMBLE, PACKET_FORMAT_11N, SAMPLING_TIME_US
			// TEMP: IF CAN'T GET THESE KEYWORD FROM VDUT, THEN GIVE TX_CALIBRATION INPUT PARAMETER

			// SAMPLING_TIME_US
			err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, "SAMPLING_TIME_US",  &tagInputPara.SAMPLING_TIME_US);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				tagInputPara.SAMPLING_TIME_US = l_txCalParam.SAMPLING_TIME_US;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (SAMPLING_TIME_US), using flow file setting = %.1f\n", l_txCalParam.SAMPLING_TIME_US);
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(SAMPLING_TIME_US=%.1f) return OK.\n", tagInputPara.SAMPLING_TIME_US);
			}

			// TX1
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "TX1",  &tagInputPara.TX1);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				tagInputPara.TX1 = 1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (TX1), default set the TX1 = ON.\n");
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(TX1=%d) return OK.\n", tagInputPara.TX1);
			}

			// TX2
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "TX2",  &tagInputPara.TX2);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				tagInputPara.TX2 = 0;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (TX2), default set the TX2 = OFF.\n");
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(TX2=%d) return OK.\n", tagInputPara.TX2);
			}

			// TX3
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "TX3",  &tagInputPara.TX3);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				tagInputPara.TX3 = 0;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (TX3), default set the TX3 = OFF.\n");
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(TX3=%d) return OK.\n", tagInputPara.TX3);
			}

			// TX4
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "TX4",  &tagInputPara.TX4);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				tagInputPara.TX4 = 0;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (TX4), default set the TX4 = OFF.\n");
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(TX4=%d) return OK.\n", tagInputPara.TX4);
			}

			// PREAMBLE
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "PREAMBLE", tagInputPara.PREAMBLE, MAX_BUFFER_SIZE);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				sprintf_s(tagInputPara.PREAMBLE , MAX_BUFFER_SIZE, "SHORT");
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

			// MEASURE_TYPE
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "MEASURE_TYPE", tagInputPara.MEASURE_TYPE, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetStringReturn(MEASURE_TYPE) return error.\n");
				vDUT_Run(g_WiFi_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(MEASURE_TYPE=%s) return OK.\n", tagInputPara.MEASURE_TYPE);
			}

			// Check path loss (by ant and freq)
			for (int i=0;i<MAX_DATA_STREAM;i++)
			{
				l_txCalParam.CABLE_LOSS_DB[i] = l_pathLossRecord[i];
			}		

			err = CheckPathLossTableExt(    g_WiFi_Test_ID,
											tagInputPara.FREQ_MHZ,
											tagInputPara.TX1,
											tagInputPara.TX2,
											tagInputPara.TX3,
											tagInputPara.TX4,
											l_txCalParam.CABLE_LOSS_DB,
											l_txCalReturn.CABLE_LOSS_DB,
											&cableLossDb,
											TX_TABLE
										);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get TX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
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
			 1. VSA_PORT                                                                             
			 2. ...                                                                                  
			 TODO: deal more VSA setting                                                             
			--------------------------------------------------------------------------------------------*/			        
			err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "VSA_PORT", &tagInputPara.VSA_PORT);
			if (ERR_OK!=err)
			{
				err = ERR_OK;	// Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				//If VSA_PORT didn't return from Dut control, VSA_PORT use default global setting.
				tagInputPara.VSA_PORT = defaultVsaPort;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] vDut not support parameter (VSA_PORT), default set the VSA_PORT = %d.\n", g_WiFiGlobalSettingParam.VSA_PORT);
			} 
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(VSA_PORT=%d) return OK.\n", tagInputPara.VSA_PORT);
			}

			/*---------------------------------------------------
			 CHANGE GLOBAL SETTING:
			 1. VSA_PORT
			 2. ...
			 TODO: deal more global setting  
			---------------------------------------------------*/
			::TM_ClearParameters(g_WiFi_Test_ID);
			::TM_AddIntegerParameter(g_WiFi_Test_ID, "VSA_PORT",      tagInputPara.VSA_PORT);

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
			/*    Start Do the Measurement    */
			/*--------------------------------*/
			if(strcmp(tagInputPara.MEASURE_TYPE, "POWER")==0)
			{
				err = TX_Cal_GetPowerAnalysisResult(tagInputPara, &tagOutputPara, cableLossDb);
				if (ERR_OK!=err)
				{
					// Modified /* -cfy@sunnyvale, 2012/3/13- */
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] TX_Cal_GetPowerAnalysisResult() return error.\n");	
					// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetStringReturn() ERROR_MESSAGE: %s", vErrorMsg);
					}
					vDUT_Run(g_WiFi_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
					/* <><~~ */

					throw logMessage;
				} 
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TX_Cal_GetPowerAnalysisResult() return OK.\n");
				}
			}

			if(strcmp(tagInputPara.MEASURE_TYPE, "EVM")==0)
			{
				err = TX_Cal_GetEVMAnalysisResult(tagInputPara, &tagOutputPara, cableLossDb); 
				if (ERR_OK!=err)
				{
					// Modified /* -cfy@sunnyvale, 2012/3/13- */
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] TX_Cal_GetEVMAnalysisResult() return error.\n");	
					// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetStringReturn() ERROR_MESSAGE: %s", vErrorMsg);
					}
					vDUT_Run(g_WiFi_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
					/* <><~~ */
					throw logMessage;
				} 
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TX_Cal_GetEVMAnalysisResult() return OK.\n");
				}
			}	

		}	// end of loop

		/*---------------------------------------------------
		 RESTORE PREVIOUS GLOBAL SETTING:
		 1. VSA_PORT
		 TODO: deal more previous global setting  
		---------------------------------------------------*/
		::TM_ClearParameters(g_WiFi_Test_ID);
		::TM_AddIntegerParameter(g_WiFi_Test_ID, "VSA_PORT",      defaultVsaPort);

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
        	sprintf_s(l_txCalReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			// Get return information
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
		vDUT_Run(g_WiFi_Dut, "CLEAR_DUT_STATUS"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
	}
	catch(char *msg)
	{
		vDUT_Run(g_WiFi_Dut, "CALIBRATION_FAILED");	 // Clear calibration status. Doesn't need to do error check, in case some other dut control doesn't support this function.
		ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		vDUT_Run(g_WiFi_Dut, "CALIBRATION_FAILED");	 // Clear calibration status. Doesn't need to do error check, in case some other dut control doesn't support this function.
		ReturnErrorMessage(l_txCalReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
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

    WIFI_SETTING_STRUCT setting;

    l_txCalParam.SAMPLING_TIME_US = 0.0;
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
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
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
        l_txCalReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txCalReturn.CABLE_LOSS_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txCalReturn.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
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

// This is the function to do EVM analysis when TX calibration and put the result to DUT control layer
int TX_Cal_GetEVMAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb)
{
    int  err = ERR_OK;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

    // Step1: Insert Keyword into EVM Container 
    ::TM_ClearParameters(g_WiFi_Test_ID);
         
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "FREQ_MHZ",			tagInput.FREQ_MHZ);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX1",					tagInput.TX1);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX2",					tagInput.TX2);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX3",					tagInput.TX3);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX4",					tagInput.TX4);

    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_1",		l_txCalParam.CABLE_LOSS_DB[0]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_2",		l_txCalParam.CABLE_LOSS_DB[1]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_3",		l_txCalParam.CABLE_LOSS_DB[2]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_4",		l_txCalParam.CABLE_LOSS_DB[3]);

    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "SAMPLING_TIME_US",	tagInput.SAMPLING_TIME_US);
    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "TX_POWER_DBM",		tagInput.TX_POWER_DBM);

    ::TM_AddStringParameter (g_WiFi_Test_ID, "BANDWIDTH",			tagInput.BANDWIDTH);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "DATA_RATE",			tagInput.DATA_RATE);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "PACKET_FORMAT_11N",	tagInput.PACKET_FORMAT_11N);
	::TM_AddStringParameter (g_WiFi_Test_ID, "GUARD_INTERVAL_11N",	tagInput.GUARD_INTERVAL_11N);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "PREAMBLE",			tagInput.PREAMBLE);

    // Step2: Disable DUT Configure in Wifi Tx Verify EVM
	// For backward compatibility, older DUT DLL must turn off vDut control.
    if (l_vDutEnabled==0)	// if vDut = Disabled
	{
		::TM_Run(g_WiFi_Test_ID, "VDUT_DISABLED");
	}
	else
	{
		// do nothing						
	}

    // Step3: Run Tx_Verify_EVM
    err = WiFi_TX_Verify_Evm();

	// For backward compatibility, after calibration must check vDut control status.
    if (l_vDutEnabled==0)	// if vDut = Disabled
	{		
		::TM_Run(g_WiFi_Test_ID, "VDUT_ENABLED");
	}
	else
	{
		// do nothing						
	}

    if(err != ERR_OK)
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function WiFi_TX_Verify_Evm() Failed.\n");
	}
	else
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function WiFi_TX_Verify_Evm() OK.\n");
	}

    // Step4: Get Result from EVM return map
    PassTestResultToDutInputContainer(l_txVerifyEvmReturnMap);

	// For backward compatibility, return the measure evm results back to vDUT control	
	double valueDouble = NA_NUMBER;
	
	/* #LPTW# cfy,-2011/07/7- */
	::vDUT_GetDoubleParameter(g_WiFi_Dut, "POWER_AVG_DBM",	   &valueDouble);
	::vDUT_AddDoubleParameter(g_WiFi_Dut, "MEAS_POWER_DBM",	   valueDouble);

	::vDUT_GetDoubleParameter(g_WiFi_Dut, "EVM_AVG_DB",        &valueDouble);
	::vDUT_AddDoubleParameter(g_WiFi_Dut, "MEAS_EVM",		   valueDouble);

	::vDUT_GetDoubleParameter(g_WiFi_Dut, "FREQ_ERROR_AVG",    &valueDouble); //Unit: ppm 
	::vDUT_AddDoubleParameter(g_WiFi_Dut, "MEAS_FREQ_ERROR",   valueDouble*tagInput.FREQ_MHZ); //Unit: Mhz

	::vDUT_GetDoubleParameter(g_WiFi_Dut, "AMP_ERR_DB",        &valueDouble);
	::vDUT_AddDoubleParameter(g_WiFi_Dut, "AMP_MISMATCH",	   valueDouble);

	::vDUT_GetDoubleParameter(g_WiFi_Dut, "PHASE_ERR",         &valueDouble);
	::vDUT_AddDoubleParameter(g_WiFi_Dut, "PHASE_MISMATCH",	   valueDouble);

	// Step5: TODO error code
	return err;
}

//This is the function to do analysis when TX calibration and put the result to DUT control layer
int TX_Cal_GetPowerAnalysisResult(tagAnalysisInputParameter tagInput, tagAnalysisOutput* tagOutput, double cableLossDb)
{
	int     err = ERR_OK;
    char    logMessage[MAX_BUFFER_SIZE] = {'\0'};

    // Step1: Insert Keyword into EVM Container 
    ::TM_ClearParameters(g_WiFi_Test_ID);
         
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "FREQ_MHZ",			tagInput.FREQ_MHZ);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX1",					tagInput.TX1);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX2",					tagInput.TX2);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX3",					tagInput.TX3);
    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX4",					tagInput.TX4);

    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_1",		l_txCalParam.CABLE_LOSS_DB[0]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_2",		l_txCalParam.CABLE_LOSS_DB[1]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_3",		l_txCalParam.CABLE_LOSS_DB[2]);
	::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_4",		l_txCalParam.CABLE_LOSS_DB[3]);

    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "SAMPLING_TIME_US",	tagInput.SAMPLING_TIME_US);
    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "TX_POWER_DBM",		tagInput.TX_POWER_DBM);

    ::TM_AddStringParameter (g_WiFi_Test_ID, "BANDWIDTH",			tagInput.BANDWIDTH);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "DATA_RATE",			tagInput.DATA_RATE);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "PACKET_FORMAT_11N",	tagInput.PACKET_FORMAT_11N);
	::TM_AddStringParameter (g_WiFi_Test_ID, "GUARD_INTERVAL_11N",	tagInput.GUARD_INTERVAL_11N);
    ::TM_AddStringParameter (g_WiFi_Test_ID, "PREAMBLE",			tagInput.PREAMBLE);

    // Step2: Disable DUT Configure in Wifi Tx Verify POWER
	// For backward compatibility, older DUT DLL must turn off vDut control.
    if (l_vDutEnabled==0)	// if vDut = Disabled
	{
		::TM_Run(g_WiFi_Test_ID, "VDUT_DISABLED");
	}
	else
	{
		// do nothing						
	}

    // Step3: Run Tx_Verify_Power
    err = WiFi_TX_Verify_Power();

	// For backward compatibility, after calibration must check vDut control status.
    if (l_vDutEnabled==0)	// if vDut = Disabled
	{		
		::TM_Run(g_WiFi_Test_ID, "VDUT_ENABLED");
	}
	else
	{
		// do nothing						
	}

    if(err != ERR_OK)
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function WiFi_TX_Verify_Power() Failed.\n");
	}
	else
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function WiFi_TX_Verify_Power() OK.\n");
	}

    // Step4: Get Result from POWER return map
    PassTestResultToDutInputContainer(l_txVerifyPowerReturnMap);

	// For backward compatibility, return the measure power results back to vDUT control	
	double valueDouble = NA_NUMBER;
	
	/* #LPTW# cfy,-2011/07/7- */
	::vDUT_GetDoubleParameter(g_WiFi_Dut, "POWER_AVERAGE_DBM", &valueDouble);
	::vDUT_AddDoubleParameter(g_WiFi_Dut, "MEAS_POWER_DBM",	   valueDouble);

	// Step5: TODO error code
	return err;
}

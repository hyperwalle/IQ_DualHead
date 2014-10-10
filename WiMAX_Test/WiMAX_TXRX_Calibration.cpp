#include "stdafx.h"
#include "TestManager.h"
#include "WIMAX_Test.h"
#include "WIMAX_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
//#include <stdio.h>
//#include <time.h>
#include <sys/timeb.h>
#include <math.h>
//#include <Windows.h>
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"


// This global variable is declared in WIMAX_Test_Internal.cpp
extern vDUT_ID				g_WIMAX_Dut;
extern TM_ID				g_WIMAX_Test_ID;
extern int					g_WIMAX_Test_timer;
extern bool					g_vsgContinuousTxMode;
extern bool					g_vDutTxActived;
extern bool					g_vDutRxActived;
extern bool					g_dutConfigChanged;
extern char					g_defaultFilePath[MAX_BUFFER_SIZE];

extern WIMAX_GLOBAL_SETTING g_WIMAXGlobalSettingParam;

using namespace std;

// Input Parameter Container
map<string, WIMAX_SETTING_STRUCT> l_txrxCalParamMap;

// Return Value Container 
map<string, WIMAX_SETTING_STRUCT> l_txrxCalReturnMap;

struct tagParam
{
    // Mandatory Parameters
    double SAMPLING_TIME_US;                         /*! The sampling time to do measurement. Default=50us */ 
    double CAL_TIMEOUT_MS;							 /*! The timeout to do TX calibration. Default=600s */ 

} l_txrxCalParam;

struct tagReturn
{
	char	CAL_RESULT_1[MAX_BUFFER_SIZE];
	char	CAL_RESULT_2[MAX_BUFFER_SIZE];
	char	CAL_RESULT_3[MAX_BUFFER_SIZE];
	char	CAL_RESULT_4[MAX_BUFFER_SIZE];

    char	ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txrxCalReturn;


//#define TXRX_CALIBRATION_DEBUG
#ifdef TXRX_CALIBRATION_DEBUG
	char g_strMsg[MAX_BUFFER_SIZE] = "";
#endif //TXRX_CALIBRATION_DEBUG


#define DEFAULT_TIMEOUT_MS 600000                     //default timeout value: 600s 
#define CAL_TX_MODE				0
#define CAL_RX_MODE				1
#define CAL_IDLE_MODE			9

typedef struct tagInputParamFromDutType
{
    // Mandatory Parameters
	int    CALIBRATION_MODE;
	int    AVG_TIMES;
	int    SIG_TYPE;								/*! Indicates the type of signal. 1=downlink signal, 2=uplink signal, 0=auto-detect\r\nDefault: 0. */
	int    RATE_ID;									/*! Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively\r\nDefault: 0. */
    double FREQ_MHZ;                                /*! The center frequency (MHz). */  
	double BANDWIDTH_MHZ;							/*! Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20\r\nDefault: 0 */
    double CYCLIC_PREFIX;							/*! Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32\r\nDefault: 0. */
	double TX_POWER_DBM; 
	double SAMPLING_TIME_US;                        /*! The sampling time to verify EVM. */

	char   WAVE_TYPE[MAX_BUFFER_SIZE];				/*! Support "CW" and "WIMAX" signal type */
    char   MEASURE_TYPE[MAX_BUFFER_SIZE];			/*! Support "POWER" measurement type */
	char   MAP_CONFIG_FILE_NAME[MAX_BUFFER_SIZE];	/*! For 802.16e analysis, a map configuration file (*.mcf) is required for non-auto-detect operation. */
	char   WAVEFORM_FILE_NAME[MAX_BUFFER_SIZE];		/*! For 802.16e analysis, a Waveform name (*.mod) is required for Rx test. If the value equal to AUTO that means using GLOBAL_SETTINGS->WIMAX_WAVEFORM_NAME. */

    int    TX1;                                     /*!< DUT TX1 on/off. */
    int    TX2;                                     /*!< DUT TX2 on/off. */
    int    TX3;                                     /*!< DUT TX3 on/off. */ 
    int    TX4;                                     /*!< DUT TX4 on/off. */

	// RX Parameters
	int    FRAME_COUNT;                             /*! Number of count to send data packet */
    double RX_POWER_DBM;                            /*! The power level to verify PER. */

    int    RX1;                                     /*!< DUT RX1 on/off. */
    int    RX2;                                     /*!< DUT RX2 on/off. */
    int    RX3;                                     /*!< DUT RX3 on/off. */ 
    int    RX4;                                     /*!< DUT RX4 on/off. */

}tagAnalysisInputParamFromDut;

typedef struct tagOutputParamToDutType
{
    double EVM_AVG;                  
    double EVM_MAX;                  
    double EVM_MIN; 

    double POWER_AVG;
    double POWER_MAX;                
    double POWER_MIN;   
	
    double FREQ_ERROR_AVG;                             
    double FREQ_ERROR_MAX;                            
    double FREQ_ERROR_MIN;  

    double AMP_ERR_DB;								/*!< IQ Match Amplitude Error in dB. */
    double PHASE_ERR;								/*!< IQ Match Phase Error. */

    double TX_CABLE_LOSS_DB[MAX_DATA_STREAM];       /*! The TX_path loss of test system. TX_CABLE_LOSS_DB_1, TX_CABLE_LOSS_DB_2, TX_CABLE_LOSS_DB_3 and TX_CABLE_LOSS_DB_4 etc. */
    double RX_CABLE_LOSS_DB[MAX_DATA_STREAM];       /*! The RX path loss of test system. RX_CABLE_LOSS_DB_1, RX_CABLE_LOSS_DB_2, RX_CABLE_LOSS_DB_3 and RX_CABLE_LOSS_DB_4 etc. */

    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
	
}tagAnalysisOutputParamToDut;

tagAnalysisInputParamFromDut	l_InputParamFromDut;
tagAnalysisOutputParamToDut		l_OutputParamToDut;


struct __timeb64 start_time, stop_time;
int		l_iEclipseTimeMs;
double	l_cableLossDb[MAX_PATHLOSS_TABLE_NUM];

void ClearTxRxCalReturn(void)
{
	l_txrxCalParamMap.clear();
	l_txrxCalReturnMap.clear();
}


int     CalGetFrequencyErrorResult(tagAnalysisInputParamFromDut tagInput, tagAnalysisOutputParamToDut* tagOutput);
int		CalGetPowerAnalysisResult(tagAnalysisInputParamFromDut tagInput, tagAnalysisOutputParamToDut* tagOutput);
int		GenerateSignal(tagAnalysisInputParamFromDut tagInput);
int		GetMeasureParameterFromDut(void);
void    InitialDutCalParameters(void);


//! WIMAX TXRX Calibration
/*!
* Input Parameters
*
*  - Mandatory 
*      -# SAMPLING_TIME_US (double): The data rate to do measurement.
*      -# CAL_TIMEOUT_MS (double): The timeout to do TX calibration. Default=600s.
*
* Return Values
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/

WIMAX_TEST_API int WIMAX_TXRX_Calibration(void)
{
    int    err = ERR_OK;
    int    dummyValue  = 0;  
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};


   /*---------------------------------------*
    * Clear Return Parameters and Container *
    *---------------------------------------*/
    ClearReturnParameters(l_txrxCalReturnMap);

    /*------------------------*
    * Respond to QUERY_INPUT *
    *------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err)
    {
        RespondToQueryInput(l_txrxCalParamMap);
        return err;
    }

   /*-------------------------*
    * Respond to QUERY_RETURN *
    *-------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err)
    {
        RespondToQueryReturn(l_txrxCalReturnMap);
        return err;
    }

	try
	{
	   /*-----------------------------------------------------------*
		* Both g_WIMAX_Test_ID and g_WIMAX_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_WIMAX_Test_ID<0 || g_WIMAX_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] WIMAX_Test_ID or WIMAX_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] WIMAX_Test_ID = %d and WIMAX_Dut = %d.\n", g_WIMAX_Test_ID, g_WIMAX_Dut);
		}
		
		TM_ClearReturns(g_WIMAX_Test_ID);

	   /*----------------------*
		* Get input parameters *
		*----------------------*/
		err = GetInputParameters(l_txrxCalParamMap);		
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Get input parameters return OK.\n");
		}

		// Error return of this function is irrelevant
		CheckDutTransmitStatus();
	

	   /*---------------------*
		*   Do Calibration    *
		*---------------------*/
		// Start Timer
		_ftime64_s (&start_time); 

		while (true)
		{
		   /*----------------------------*
			*   Initial the Parameters   *
			*----------------------------*/
			int    iCalDone      = 0;
			int    iCalAbort     = 0;

			InitialDutCalParameters();	


			// Stop Timer
			_ftime64_s (&stop_time);

			// Check Timeout 
			l_iEclipseTimeMs = (unsigned int) (((stop_time.time-start_time.time)*1000) + (stop_time.millitm-start_time.millitm));
			if (l_iEclipseTimeMs>l_txrxCalParam.CAL_TIMEOUT_MS)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Calibration timeout (test time: %.2f sec)! Abort.\n", (l_iEclipseTimeMs/1000) );
				throw logMessage;
			}

			// Set DUT calibration procedure
			err = vDUT_Run(g_WIMAX_Dut, "TX_RX_CALIBRATION");
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(TX_RX_CALIBRATION) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(TX_RX_CALIBRATION) return OK.\n");
			}

			// Get the measure parameter from DUT control layer
			err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "CAL_DONE",  &iCalDone);				 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_GetIntegerReturn(CAL_DONE) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetIntegerReturn(CAL_DONE) return OK.\n");
			}
			// If the DUT set the CAL_DONE, means the calibration is finished.
			if ( iCalDone )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] DUT report calibration done.\n");
				break;		// calibration finished
			}	
			else
			{
				// continue the calibration process...
			}
			
			// Get the measure parameter from DUT control layer
			err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "CAL_ABORT",  &iCalAbort);				 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_GetIntegerReturn(CAL_ABORT) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetIntegerReturn(CAL_ABORT) return OK.\n");
			}
			// If the DUT set the CAL_ABORT, means the calibration has some error and need to stop the process.
			if( iCalAbort )
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] DUT abort calibration.\n");
				throw logMessage;
			}
			else
			{
				// continue the calibration process...
			}

		   /*-----------------------------------------*
			*   Get the measure parameter from DUT    *
			*-----------------------------------------*/
			err = GetMeasureParameterFromDut();
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Get the measure parameter from DUT return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Get the measure parameter from DUT return OK.\n");
			}

		   /*-----------------------------*
			*   Case1 - TX Calibration    *
			*-----------------------------*/
			if ( l_InputParamFromDut.CALIBRATION_MODE==CAL_TX_MODE )  // process TX mode: CAL_TX_MODE => int(0)
			{
				if ( (0==strcmp(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE,"LINK"))&&(0==strcmp(l_InputParamFromDut.WAVE_TYPE, "WIMAX")) )
				{
					// The only case of TX_Mode won't need to turn off the VSG
					// do nothing...

					/*-------------------------------*
					 * SETUP VSG FOR SYNCHRONIZATION *
					 *-------------------------------*/	
					// If the WIMAX_TEST_MODE=LINK and WAVE_TYPE=WIMAX, then DUT need VSG transmit the signal for synchronization.
					// Since the IQLite software architecture, the programer must call CALIBRATION_MODE=CAL_RX_MODE first for sync.
					// Then call CALIBRATION_MODE=CAL_TX_MODE for WiMAX TX calibration or there is no way to know the DUT is under synchronization or not.  
				}
				else
				{
					// Must turn off the VSG.
					err = ::LP_EnableVsgRF(0);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
					}
				}

				// Delay for DUT settle
				if (0!=g_WIMAXGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
				{
					Sleep(g_WIMAXGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
				}
				else
				{
					// do nothing
				}

			   /*---------------------*
				*  Start Measurement  *
				*---------------------*/
				if( strcmp(l_InputParamFromDut.MEASURE_TYPE, "POWER")==0 )
				{
					err = CalGetPowerAnalysisResult(l_InputParamFromDut, &l_OutputParamToDut); 
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] CalGetPowerAnalysisResult(POWER_AVG) return error.\n");
						vDUT_Run(g_WIMAX_Dut, "CALIBRATION_FAILED"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] CalGetPowerAnalysisResult(POWER_AVG) return OK.\n");
					}
				}
				else if( strcmp(l_InputParamFromDut.MEASURE_TYPE, "FREQUENCY")==0 )
				{
					err = CalGetFrequencyErrorResult(l_InputParamFromDut, &l_OutputParamToDut); 
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] CalGetFrequencyErrorResult(FREQ_ERROR_AVG) return error.\n");
						vDUT_Run(g_WIMAX_Dut, "CALIBRATION_FAILED"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] CalGetFrequencyErrorResult(FREQ_ERROR_AVG) return OK.\n");
					}
				}
				
				// Clear container
				vDUT_ClearParameters(g_WIMAX_Dut);
				// Input the measure power value to DUT control
				// Note: The power result doesn't contain the cable loss.
				vDUT_AddDoubleParameter (g_WIMAX_Dut, "POWER_AVG",				l_OutputParamToDut.POWER_AVG);
				vDUT_AddDoubleParameter (g_WIMAX_Dut, "FREQ_ERROR_AVG",			l_OutputParamToDut.FREQ_ERROR_AVG);
				vDUT_AddDoubleParameter (g_WIMAX_Dut, "EVM_AVG",				l_OutputParamToDut.EVM_AVG);
				vDUT_AddDoubleParameter (g_WIMAX_Dut, "AMP_ERR_DB",				l_OutputParamToDut.AMP_ERR_DB);
				vDUT_AddDoubleParameter (g_WIMAX_Dut, "PHASE_ERR",				l_OutputParamToDut.PHASE_ERR);
			}  // end of tx process

		   /*-----------------------------*
			*   Case2 - RX Calibration    *
			*-----------------------------*/
			else if ( l_InputParamFromDut.CALIBRATION_MODE==CAL_RX_MODE ) 	// process RX mode: CAL_RX_MODE => int(1)
			{
			   /*------------------------------------------------------*
				* Generating CW or WiMAX signal, depend on "WAVE_TYPE" *
				*------------------------------------------------------*/
				err = GenerateSignal( l_InputParamFromDut );
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] GenerateSignal() return error.\n");
					vDUT_Run(g_WIMAX_Dut, "CALIBRATION_FAILED"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] GenerateSignal() return OK.\n");
				}	
				// Clear container
				vDUT_ClearParameters(g_WIMAX_Dut);

	        }  // end of rx process

		   /*-----------------------*
			*   Case3 - Idle Mode   *
			*-----------------------*/
			else if ( l_InputParamFromDut.CALIBRATION_MODE==CAL_IDLE_MODE )		// Tester Idle: CAL_IDLE_MODE => int(9)
			{
				// In this mode, we just turn off the VSG and do nothing.
				err = ::LP_EnableVsgRF(0);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Under IDLE_MODE, fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
					vDUT_Run(g_WIMAX_Dut, "CALIBRATION_FAILED"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
				}	
				// Clear container
				vDUT_ClearParameters(g_WIMAX_Dut);

			}  // end of Idle Mode
			else
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Unknown clibration mode and not support by IQlite.\n");
				vDUT_Run(g_WIMAX_Dut, "CALIBRATION_FAILED"); // to clear dut status. Does not need to do error check in case some other dut control does not need this function.
				throw logMessage;
			}

	   /*---------------------------------------*
		*   Return the Tx/Rx Path Loss to Dut   *
		*---------------------------------------*/
		// Set to 0, means want to load path loss from table
		for (int i=0;i<MAX_DATA_STREAM;i++)
		{
			l_OutputParamToDut.TX_CABLE_LOSS_DB[i]  = 0.0;
			l_OutputParamToDut.RX_CABLE_LOSS_DB[i]  = 0.0;
		}

		// Set all Tx_Ant to ON 
		err = CheckPathLossTableExt(	g_WIMAX_Test_ID,
										(int)l_InputParamFromDut.FREQ_MHZ,
										1,
										1,
										1,
										1,
										l_OutputParamToDut.TX_CABLE_LOSS_DB,
										l_OutputParamToDut.TX_CABLE_LOSS_DB,
										&l_cableLossDb[TX_TABLE],
										TX_TABLE
									);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] Fail to get TX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
			//throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] CheckPathLossTableExt(TX_TABLE) return OK.\n");
		}

		// Set all Rx_Ant to ON 
		err = CheckPathLossTableExt(	g_WIMAX_Test_ID,
										(int)l_InputParamFromDut.FREQ_MHZ,
										1,
										1,
										1,
										1,
										l_OutputParamToDut.RX_CABLE_LOSS_DB,
										l_OutputParamToDut.RX_CABLE_LOSS_DB,
										&l_cableLossDb[RX_TABLE],
										RX_TABLE
									);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] Fail to get RX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
			//throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] CheckPathLossTableExt(RX_TABLE) return OK.\n");
		}

		if ( NA_NUMBER!=l_OutputParamToDut.TX_CABLE_LOSS_DB[0] )	vDUT_AddDoubleParameter (g_WIMAX_Dut, "TX_CABLE_LOSS_DB_1",		l_OutputParamToDut.TX_CABLE_LOSS_DB[0]);
		if ( NA_NUMBER!=l_OutputParamToDut.TX_CABLE_LOSS_DB[1] )	vDUT_AddDoubleParameter (g_WIMAX_Dut, "TX_CABLE_LOSS_DB_2",		l_OutputParamToDut.TX_CABLE_LOSS_DB[1]);
		if ( NA_NUMBER!=l_OutputParamToDut.TX_CABLE_LOSS_DB[2] )	vDUT_AddDoubleParameter (g_WIMAX_Dut, "TX_CABLE_LOSS_DB_3",		l_OutputParamToDut.TX_CABLE_LOSS_DB[2]);
		if ( NA_NUMBER!=l_OutputParamToDut.TX_CABLE_LOSS_DB[3] )	vDUT_AddDoubleParameter (g_WIMAX_Dut, "TX_CABLE_LOSS_DB_4",		l_OutputParamToDut.TX_CABLE_LOSS_DB[3]);

		if ( NA_NUMBER!=l_OutputParamToDut.RX_CABLE_LOSS_DB[0] )	vDUT_AddDoubleParameter (g_WIMAX_Dut, "RX_CABLE_LOSS_DB_1",		l_OutputParamToDut.RX_CABLE_LOSS_DB[0]);
		if ( NA_NUMBER!=l_OutputParamToDut.RX_CABLE_LOSS_DB[1] )	vDUT_AddDoubleParameter (g_WIMAX_Dut, "RX_CABLE_LOSS_DB_2",		l_OutputParamToDut.RX_CABLE_LOSS_DB[1]);
		if ( NA_NUMBER!=l_OutputParamToDut.RX_CABLE_LOSS_DB[2] )	vDUT_AddDoubleParameter (g_WIMAX_Dut, "RX_CABLE_LOSS_DB_3",		l_OutputParamToDut.RX_CABLE_LOSS_DB[2]);
		if ( NA_NUMBER!=l_OutputParamToDut.RX_CABLE_LOSS_DB[3] )	vDUT_AddDoubleParameter (g_WIMAX_Dut, "RX_CABLE_LOSS_DB_4",		l_OutputParamToDut.RX_CABLE_LOSS_DB[3]);
		
      } // end loop

		/*----------------------*
		 *  Function Completed  *
		 *----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_txrxCalReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.");
		} 
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txrxCalReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txrxCalReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
    }

	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	// Get return information
	vDUT_GetStringReturn(g_WIMAX_Dut, "CAL_RESULT_1",	l_txrxCalReturn.CAL_RESULT_1,MAX_BUFFER_SIZE);
	vDUT_GetStringReturn(g_WIMAX_Dut, "CAL_RESULT_2",	l_txrxCalReturn.CAL_RESULT_2,MAX_BUFFER_SIZE);
	vDUT_GetStringReturn(g_WIMAX_Dut, "CAL_RESULT_3",	l_txrxCalReturn.CAL_RESULT_3,MAX_BUFFER_SIZE);
	vDUT_GetStringReturn(g_WIMAX_Dut, "CAL_RESULT_4",	l_txrxCalReturn.CAL_RESULT_4,MAX_BUFFER_SIZE);
	ReturnTestResults(l_txrxCalReturnMap);


    return err;
}

void InitializeTXRXCalContainers(void)
{
    /*------------------*
    * Input Paramters: *
    * IQTESTER_IP01    *
    *------------------*/
    l_txrxCalParamMap.clear();

    WIMAX_SETTING_STRUCT setting;

    l_txrxCalParam.SAMPLING_TIME_US = 900.0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txrxCalParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&l_txrxCalParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "VSA capture time in micro-seconds. This can be updated by DUT, if DUT set the SAMPLING_TIME_US during the calibration runtime.";
        l_txrxCalParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SAMPLING_TIME_US", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxCalParam.CAL_TIMEOUT_MS = DEFAULT_TIMEOUT_MS;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txrxCalParam.CAL_TIMEOUT_MS))    // Type_Checking
    {
        setting.value = (void*)&l_txrxCalParam.CAL_TIMEOUT_MS;
        setting.unit        = "ms";
        setting.helpText    = "Timeout when calibration fail.";
        l_txrxCalParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CAL_TIMEOUT_MS", setting));
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
    l_txrxCalReturnMap.clear();

    l_txrxCalReturn.CAL_RESULT_1[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txrxCalReturn.CAL_RESULT_1))    // Type_Checking
    {
        setting.value       = (void*)l_txrxCalReturn.CAL_RESULT_1;
        setting.unit        = "";
        setting.helpText    = "Calibration Result 1";
        l_txrxCalReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CAL_RESULT_1", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txrxCalReturn.CAL_RESULT_2[0] = '\0';
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txrxCalReturn.CAL_RESULT_2))    // Type_Checking
	{
		setting.value       = (void*)l_txrxCalReturn.CAL_RESULT_2;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 2";
		l_txrxCalReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CAL_RESULT_2", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txrxCalReturn.CAL_RESULT_3[0] = '\0';
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txrxCalReturn.CAL_RESULT_3))    // Type_Checking
	{
		setting.value       = (void*)l_txrxCalReturn.CAL_RESULT_3;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 3";
		l_txrxCalReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CAL_RESULT_3", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txrxCalReturn.CAL_RESULT_4[0] = '\0';
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txrxCalReturn.CAL_RESULT_4))    // Type_Checking
	{
		setting.value       = (void*)l_txrxCalReturn.CAL_RESULT_4;
		setting.unit        = "";
		setting.helpText    = "Calibration Result 4";
		l_txrxCalReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CAL_RESULT_4", setting));
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	
    l_txrxCalReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txrxCalReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txrxCalReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_txrxCalReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ERROR_MESSAGE", setting));
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}


//This is the function to do analysis when TX calibration and return the results to DUT control layer
int CalGetFrequencyErrorResult(tagAnalysisInputParamFromDut tagInput, tagAnalysisOutputParamToDut* tagOutput)
{
    int    err = ERR_OK;

	int    avgIteration_freq  = 0;  // for frequency
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};
	
	double vsaAmplDb	  = 0.0;

	vector< double >     freqErrorHz (tagInput.AVG_TIMES);

	try
	{
	   /*--------------------*
		* Setup IQTester VSA *
		*--------------------*/	
		err = ::LP_SetVsaAmplitudeTolerance(g_WIMAXGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to set VSA amplitude tolerance in dB.\n");
			throw logMessage;
		}

	   /*------------------------------*
		* Start while loop for average *
		*------------------------------*/    
		while( avgIteration_freq<tagInput.AVG_TIMES )
		{
			if( strcmp(tagInput.WAVE_TYPE, "CW")==0 )   // CW signal, need shit the frequency and do FFT.
			{
				if ( NA_NUMBER!=tagInput.TX_POWER_DBM )
				{
					vsaAmplDb = tagInput.TX_POWER_DBM - l_cableLossDb[TX_TABLE] + g_WIMAXGlobalSettingParam.IQ_P_TO_A_CW;
				}
				else
				{
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Can't retrieve the TX_POWER_DBM for VSA setup.\n");
					throw logMessage;			
				}
	
				//------------------------------//
				//    Analyze frequency Error   //
				//------------------------------//
				err = AnalyzeCWFrequency( tagInput.FREQ_MHZ,
		  								  vsaAmplDb,
										  l_txrxCalParam.SAMPLING_TIME_US,
										  1,
										  &freqErrorHz[avgIteration_freq],
										  logMessage
										);
				if (ERR_OK!=err)
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, logMessage);
					throw logMessage;
				}
				avgIteration_freq++;
			}
			else	// WiMAX signal, do EVM analysis. 
			{
				// TODO: WiMAX EVM analysis
			}

		}

	   /*----------------------------------*
		* Averaging and Saving Test Result *
		*----------------------------------*/
		if( avgIteration_freq>0 )
		{
			// Average frequency result
			::AverageTestResult(&freqErrorHz[0], avgIteration_freq, Linear, tagOutput->FREQ_ERROR_AVG, tagOutput->FREQ_ERROR_MAX, tagOutput->FREQ_ERROR_MIN);
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txrxCalReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txrxCalReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
    }

	// Free memory
	freqErrorHz.clear();


	return err;
}

//This is the function to do analysis when TX calibration and return the results to DUT control layer
int CalGetPowerAnalysisResult(tagAnalysisInputParamFromDut tagInput, tagAnalysisOutputParamToDut* tagOutput)
{
    int    err = ERR_OK;

    int    avgIteration_power = 0;  // for power
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};
	
	bool   useFFTforCW	  = false;
	bool   captureRetry   = false;
	int    vsaTriggerType = 0;
	double peakToAvgRatio = 0.0;
	double vsaAmplDb	  = 0.0;
	double measureFreqMHz = 0.0;

	vector< double >     rxRmsPowerDb(tagInput.AVG_TIMES);

	try
	{
	   /*--------------------*
		* Setup IQTester VSA *
		*--------------------*/
		if( strcmp(tagInput.WAVE_TYPE, "CW")==0 )   // CW signal
		{
			peakToAvgRatio = g_WIMAXGlobalSettingParam.IQ_P_TO_A_CW;	
			vsaTriggerType = IQV_TRIG_TYPE_FREE_RUN;
			//measureFreqMHz = tagInput.FREQ_MHZ + WIMAX_FREQ_SHIFT_FOR_CW_MHZ;
		}
		else //if (strcmp(tagInput.WAVE_TYPE, "WIMAX") == 0 ) // WiMAX signal 
		{
			peakToAvgRatio = g_WIMAXGlobalSettingParam.IQ_P_TO_A_WIMAX;
			vsaTriggerType = g_WIMAXGlobalSettingParam.VSA_TRIGGER_TYPE;
			measureFreqMHz = tagInput.FREQ_MHZ;
		}

		err = ::LP_SetVsaAmplitudeTolerance(g_WIMAXGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to set VSA amplitude tolerance in dB.\n");
			throw logMessage;
		}

		if ( NA_NUMBER!=tagInput.TX_POWER_DBM )
		{
			vsaAmplDb = tagInput.TX_POWER_DBM - l_cableLossDb[TX_TABLE] + peakToAvgRatio;
		}
		else
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Can't retrieve the TX_POWER_DBM for VSA setup.\n");
			throw logMessage;			
		}
		// Check CW power level, if more than MIN_WIMAX_CW_POWER(dBm) then using LP_AnalyzePower(), else using LP_AnalyzeFFT() and search peak power.
		if( 0==strcmp(tagInput.WAVE_TYPE, "CW") )   // CW signal
		{
			if ( vsaAmplDb>MIN_WIMAX_CW_POWER )
			{
				measureFreqMHz = tagInput.FREQ_MHZ;
				useFFTforCW = false;	// using LP_AnalyzePower()
			}
			else
			{
				measureFreqMHz = tagInput.FREQ_MHZ + WIMAX_FREQ_SHIFT_FOR_CW_MHZ;
				useFFTforCW = true;		// using LP_AnalyzeFFT()
			}
		}
		else //if (strcmp(tagInput.WAVE_TYPE, "WIMAX") == 0 ) // WiMAX signal 
		{
			// do nothing
		}
		err = ::LP_SetVsa(	measureFreqMHz*1e6,							 // in Hz unit
							vsaAmplDb,      
							g_WIMAXGlobalSettingParam.VSA_PORT,
							0,
							g_WIMAXGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
							g_WIMAXGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000				  
							);
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to setup VSA.\n");
			throw logMessage;
		}

		//for (int pwrCheckLoop=0;pwrCheckLoop<10;pwrCheckLoop++)
		//{
		//   /*-------------------------------------------*
		//	* Perform Quick VSA capture and Power Check *
		//	*-------------------------------------------*/
		//	err = ::LP_VsaDataCapture(0.005, IQV_TRIG_TYPE_FREE_RUN); 
		//	if ( ERR_OK!=err )
		//	{
		//		// Fail Capture
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to Quick capture signal at %d MHz.\n", measureFreqMHz);
		//		throw logMessage;
		//	}
		//	else
		//	{
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Quick LP_VsaDataCapture() at %d MHz return OK.\n", measureFreqMHz);
		//	}		

		//	/*--------------------------------*
		//	 *  Perform Quick Power Analysis  *
		//	 *--------------------------------*/
		//	err = ::LP_AnalyzePower( );
		//	if ( ERR_OK!=err )
		//	{
		//		// Fail Analysis, thus save capture (Signal File) for debug
		//		sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Analysis_Failed", measureFreqMHz);
		//		WIMAXSaveSigFile(sigFileNameBuffer);
		//		sprintf_s(logMessage, MAX_BUFFER_SIZE, "[WIMAX] Quick LP_AnalyzePower() return error and save the sig file %s.\n", sigFileNameBuffer);
		//		return err;
		//	}
		//	else		// Analyze is ok
		//	{
		//	   /*-----------------------------*
		//		*  Retrieve analysis Results  *
		//		*-----------------------------*/
		//		double pkPowerDbm  = ::LP_GetScalarMeasurement("P_peak_all_dBm", 0);
		//		if ( -99.00 >= (pkPowerDbm)  )
		//		{
		//			pkPowerDbm = NA_NUMBER;				
		//			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Result_Failed", measureFreqMHz);
		//			WIMAXSaveSigFile(sigFileNameBuffer);
		//			err = -1;				
		//			sprintf_s(logMessage, MAX_BUFFER_SIZE, "[WIMAX] Quick LP_GetScalarMeasurement(P_peak_all_dBm) return error and save the sig file %s.\n", sigFileNameBuffer);
		//			return err;
		//		}
		//		else
		//		{			
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Quick power analysis OK, P_peak_all_dBm = %.2f.\n", pkPowerDbm);

		//			// Check if the power over the range, then needs some delay for power settle down
		//			double targetPkPower = vsaAmplDb;
		//			if ( abs(targetPkPower-pkPowerDbm)>g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_TOLERANCE_DB )
		//			{
						//if (pwrCheckLoop==5)
						//{
							//// Retry DUT control, but only check "TX_START" error return
							//vDUT_Run(g_WIMAX_Dut, "TX_STOP");
							//vDUT_Run(g_WIMAX_Dut, "RF_SET_FREQ");
							//vDUT_Run(g_WIMAX_Dut, "TX_SET_BW");
							//vDUT_Run(g_WIMAX_Dut, "TX_SET_DATA_RATE");
							//vDUT_Run(g_WIMAX_Dut, "TX_SET_ANTENNA");
							//vDUT_Run(g_WIMAX_Dut, "TX_PRE_TX");
						//	err = vDUT_Run(g_WIMAX_Dut, "TX_START");
						//	if ( ERR_OK!=err )
						//	{	
						//	   g_vDutTxActived = false;
						//	   // Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
						//	   err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
						//	   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
						//	   {
						//		   err = -1;	// set err to -1, means "Error".
						//		   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						//		   throw logMessage;
						//	   }
						//	   else	// Just return normal error message in this case
						//	   {
						//		   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(TX_START) return error.\n");
						//		   throw logMessage;
						//	   }
						//	}
						//	else
						//	{
						//	   g_vDutTxActived = true;
						//	   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(TX_START) return OK.\n");
						//	}
						//}
						//else
						//{
						//	// do nothing
						//}
		//				// A delay for waitting Power settle down
		//				Sleep(g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_SETTLE_TIME_MS);
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Since [Target_Peak_Power_dBm(%.2f)]-[P_peak_all_dBm(%.2f)] > [%.1f] dB, thus Sleep(%d) for power settle down.\n", targetPkPower, pkPowerDbm, g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_TOLERANCE_DB, g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_SETTLE_TIME_MS);
		//			}
		//			else
		//			{
		//				// Power OK
		//				break;
		//			}
		//		}
		//	}
		//}

	   /*------------------------------*
		* Start while loop for average *
		*------------------------------*/    
		while( avgIteration_power<tagInput.AVG_TIMES )
		{
			if( strcmp(tagInput.WAVE_TYPE, "CW")==0 )   // CW signal, need shit the frequency and do FFT.
			{
				//-----------------------//
				//    Analyze CW Power   //
				//-----------------------//
				// Perform Normal VSA capture 
				err = ::LP_VsaDataCapture(l_txrxCalParam.SAMPLING_TIME_US/1000000, vsaTriggerType);
				if (ERR_OK!=err)	// capture is failed
				{
					double quickPower = NA_NUMBER;					
					err = QuickCaptureRetry( measureFreqMHz, l_txrxCalParam.SAMPLING_TIME_US, vsaTriggerType, &quickPower, logMessage);
					if (ERR_OK!=err)	// QuickCaptureRetry() is failed
					{
						// Fail Capture
						if ( quickPower!=NA_NUMBER )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz.\nThe DUT power (without path loss) = %.1f dBm and QuickCaptureRetry() return error.\n", measureFreqMHz, quickPower);
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz, QuickCaptureRetry() return error.\n", measureFreqMHz);
						}
						throw logMessage;
					}						
					else
					{
						captureRetry = true;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] The QuickCaptureRetry() at %.1f MHz return OK.\n", measureFreqMHz);
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_VsaDataCapture() at %.1f MHz return OK.\n", measureFreqMHz);
				}

				/*--------------*
				 *  Capture OK  *
				 *--------------*/
				if (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
				{
					// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f_%.1fMHz", "WIMAX_CAL_CW_POWER_SaveAlways", measureFreqMHz, tagInput.BANDWIDTH_MHZ);
					WIMAXSaveSigFile(sigFileNameBuffer);
				}
				else
				{
					// do nothing
				}

				if ( !useFFTforCW )
				{
					//-------------------//
					//   Analyze Power   //
					//-------------------//
					//double dummy_T_INTERVAL = 3.2, dummy_MAX_POW_DIFF_DB = 15.0;  
					err = ::LP_AnalyzePower( );
					if(ERR_OK==err)	// Analyze is ok
					{
					   /*------------------*
						* Store the result *
						*------------------*/
						// Note: The power result doesn't contain the cable loss.
						rxRmsPowerDb[avgIteration_power] = ::LP_GetScalarMeasurement("P_av_no_gap_all_dBm", 0);      

						// Check Power result
						if ( (5<fabs((vsaAmplDb-peakToAvgRatio)-rxRmsPowerDb[avgIteration_power]))||
							 (rxRmsPowerDb[avgIteration_power]<=MIN_WIMAX_CW_POWER) )
						{
							useFFTforCW = true;	// means using FFT to analyize CW power again
							vsaAmplDb = rxRmsPowerDb[avgIteration_power] + peakToAvgRatio;
							err = ::LP_SetVsa(	(tagInput.FREQ_MHZ+WIMAX_FREQ_SHIFT_FOR_CW_MHZ)*1e6,							 // in Hz unit
												vsaAmplDb,      
												g_WIMAXGlobalSettingParam.VSA_PORT,
												0,
												g_WIMAXGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
												g_WIMAXGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000				  
												);
							if (ERR_OK!=err)
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to setup VSA.\n");
								throw logMessage;
							}

							// Perform Normal VSA capture 
							err = ::LP_VsaDataCapture(l_txrxCalParam.SAMPLING_TIME_US/1000000, vsaTriggerType);  
							if(ERR_OK!=err)	// capture is failed
							{
								// Fail Capture
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture CW signal at %.1f MHz.\n", measureFreqMHz);
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_VsaDataCapture() at %.1f MHz return OK.\n", measureFreqMHz);
							}

							/*--------------*
							 *  Capture OK  *
							 *--------------*/
							if (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
							{
								// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
								sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_CAL_POWER_2_SaveAlways", measureFreqMHz);
								WIMAXSaveSigFile(sigFileNameBuffer);
							}
							else
							{
								// do nothing
							}
						}
					}
					else
					{		
						// Fail Analysis, thus save capture (Signal File) for debug
						char sigFileName[MAX_BUFFER_SIZE] = {'\0'};
						sprintf_s(sigFileName, "POWER_Failed_Analysis%03d.sig", avgIteration_power);
						err = ::LP_SaveVsaSignalFile(sigFileName);
						if (ERR_OK!=err)
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to save signal file.\n");
							throw logMessage;					
						}

						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to analyze power.\n");
						throw logMessage;
					}
				}	// end - if( !useFFTforCW )

				if ( useFFTforCW )
				{
					/*----------------------------*
					 *  FFT Analysis for CW power *
					 *----------------------------*/
#pragma region Analysis FFT
					// Perform FFT analysis
					int NFFT = 0;
					err = ::LP_AnalyzeFFT( NFFT, 100000 );		// For Calibration, using default RBW = 100 KHz always.
					if ( ERR_OK!=err )
					{
						// Fail Analysis, thus save capture (Signal File) for debug
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f_%.1fMHz", "WIMAX_CAL_FFT_Analysis_Failed", measureFreqMHz, tagInput.BANDWIDTH_MHZ);
						WIMAXSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Analyze FFT at %.1f MHz return error.\n", measureFreqMHz);
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Analyze FFT at %.1f MHz return OK.\n", measureFreqMHz );
					}

#pragma endregion
#pragma region Retrieve analysis Results
					/*-----------------------------*
					 *  Retrieve analysis Results  *
					 *-----------------------------*/
					rxRmsPowerDb[avgIteration_power] = NA_NUMBER;
					// Store the result
					double bufferRealX[MAX_BUFFER_SIZE], bufferImagX[MAX_BUFFER_SIZE];
					double bufferRealY[MAX_BUFFER_SIZE], bufferImagY[MAX_BUFFER_SIZE];
					int    bufferSizeX = ::LP_GetVectorMeasurement("x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE);
					int    bufferSizeY = ::LP_GetVectorMeasurement("y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE);

					if ( (0>=bufferSizeX)||(0>=bufferSizeY) )
					{
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Can not retrieve FFT result, result length = 0.\n");
						throw logMessage;
					}
					else
					{
						/*----------------------------------------------------------------------*
						 *  Since it is a CW signal, so we just need to looking for peak power. *
						 *----------------------------------------------------------------------*/					
						for ( int i=((int)(bufferSizeX/2)-1);i>0;i-- )
						{
							// Only need to check the CW frequency range between +- 2.5 MHz. 
							// Note: Must calculate the WIMAX_FREQ_SHIFT_FOR_CW_MHZ first.
							if ( (bufferRealX[i]>(-1)*(WIMAX_FREQ_SHIFT_FOR_CW_MHZ+2.5)*1e6)&&
								 (bufferRealX[i]<(-1)*(WIMAX_FREQ_SHIFT_FOR_CW_MHZ-2.5)*1e6) )
							{
								if ( bufferRealY[i]>rxRmsPowerDb[avgIteration_power] )  
								{   // store the current peak power and keep searching
									rxRmsPowerDb[avgIteration_power] = bufferRealY[i];
								}
								else
								{
									// keep searching...
								}
							}
							else
							{
								// not in the frequency range, skip and continue...
							}
						} 

					}
				}	// end - if( useFFTforCW )

				avgIteration_power++;
#pragma endregion	

			}
			else	// WiMAX signal, do power analysis. 
			{
				//--------------------------//
				//    Analyze WiMAX Power   //
				//--------------------------//
				captureRetry = false;

				// Perform Normal VSA capture 
				err = ::LP_VsaDataCapture(l_txrxCalParam.SAMPLING_TIME_US/1000000, vsaTriggerType);
				if (ERR_OK!=err)	// capture is failed
				{
					double quickPower = NA_NUMBER;					
					err = QuickCaptureRetry( measureFreqMHz, l_txrxCalParam.SAMPLING_TIME_US, vsaTriggerType, &quickPower, logMessage);
					if (ERR_OK!=err)	// QuickCaptureRetry() is failed
					{
						// Fail Capture
						if ( quickPower!=NA_NUMBER )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz.\nThe DUT power (without path loss) = %.1f dBm and QuickCaptureRetry() return error.\n", measureFreqMHz, quickPower);
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz, QuickCaptureRetry() return error.\n", measureFreqMHz);
						}
						throw logMessage;
					}						
					else
					{
						captureRetry = true;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] The QuickCaptureRetry() at %.1f MHz return OK.\n", measureFreqMHz);
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_VsaDataCapture() at %.1f MHz return OK.\n", measureFreqMHz);
				}

				/*--------------*
				 *  Capture OK  *
				 *--------------*/
				if (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
				{
					// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f_%.1fMHz", "WIMAX_CAL_POWER_1_SaveAlways", measureFreqMHz, tagInput.BANDWIDTH_MHZ);
					WIMAXSaveSigFile(sigFileNameBuffer);
				}
				else
				{
					// do nothing
				}

				//-------------------//
				//   Analyze Power   //
				//-------------------//
				//double dummy_T_INTERVAL = 3.2, dummy_MAX_POW_DIFF_DB = 15.0;  
				err = ::LP_AnalyzePower( );
				if(ERR_OK==err)	// Analyze is ok
				{
				   /*------------------*
					* Store the result *
					*------------------*/
					// Note: The power result doesn't contain the cable loss.
					rxRmsPowerDb[avgIteration_power] = ::LP_GetScalarMeasurement("P_av_no_gap_all_dBm", 0);      

					// Check Power result
					if ( (!captureRetry)&&(5<fabs((vsaAmplDb-peakToAvgRatio)-rxRmsPowerDb[avgIteration_power])) )
					{
						vsaAmplDb = rxRmsPowerDb[avgIteration_power] + peakToAvgRatio;
						err = ::LP_SetVsa(	measureFreqMHz*1e6,							 // in Hz unit
											vsaAmplDb,      
											g_WIMAXGlobalSettingParam.VSA_PORT,
											0,
											g_WIMAXGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
											g_WIMAXGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000				  
											);
						if (ERR_OK!=err)
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Retry - Fail to setup VSA.\n");
							throw logMessage;
						}

						// Perform Normal VSA capture 
						err = ::LP_VsaDataCapture(l_txrxCalParam.SAMPLING_TIME_US/1000000, vsaTriggerType);  
						if(ERR_OK!=err)	// capture is failed
						{
							// Fail Capture
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Retry - Fail to capture WiMAX signal at %.1f MHz.\n", measureFreqMHz);
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Retry - LP_VsaDataCapture() at %.1f MHz return OK.\n", measureFreqMHz);
						}

						/*--------------*
						 *  Capture OK  *
						 *--------------*/
						if (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
						{
							// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
							sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_CAL_POWER_2_SaveAlways", measureFreqMHz);
							WIMAXSaveSigFile(sigFileNameBuffer);
						}
						else
						{
							// do nothing
						}

						//-------------------//
						//   Analyze Power   //
						//-------------------//
						//double dummy_T_INTERVAL = 3.2, dummy_MAX_POW_DIFF_DB = 15.0;  
						err = ::LP_AnalyzePower( );
						if(ERR_OK==err)	// Analyze is ok
						{
						   /*------------------*
							* Store the result *
							*------------------*/
							// Note: The power result doesn't contain the cable loss.
							rxRmsPowerDb[avgIteration_power] = ::LP_GetScalarMeasurement("P_av_no_gap_all_dBm", 0);    
						}
						else
						{		
							// Fail Analysis, thus save capture (Signal File) for debug
							char sigFileName[MAX_BUFFER_SIZE] = {'\0'};
							sprintf_s(sigFileName, "POWER_Failed_Analysis%03d.sig", avgIteration_power);
							err = ::LP_SaveVsaSignalFile(sigFileName);
							if (ERR_OK!=err)
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to save signal file.\n");
								throw logMessage;					
							}

							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to analyze power.\n");
							throw logMessage;
						}
					}
				}
				else
				{   
					// Fail Analysis, thus save capture (Signal File) for debug
					char sigFileName[MAX_BUFFER_SIZE] = {'\0'};
					sprintf_s(sigFileName, "POWER_Failed_Analysis%03d.sig", avgIteration_power);
					err = ::LP_SaveVsaSignalFile(sigFileName);
					if (ERR_OK!=err)
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to save signal file.\n");
						throw logMessage;					
					}

					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to analyze power.\n");
					throw logMessage;					
				}

				avgIteration_power++;					
				
			}

		}

	   /*----------------------------------*
		* Averaging and Saving Test Result *
		*----------------------------------*/
		if( avgIteration_power>0 )
		{
			// Average Power result
			::AverageTestResult(&rxRmsPowerDb[0], avgIteration_power, LOG_10, tagOutput->POWER_AVG, tagOutput->POWER_MAX, tagOutput->POWER_MIN);    
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txrxCalReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txrxCalReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
    }

	// Free memory
	rxRmsPowerDb.clear();


	return err;
}

int	 GenerateSignal(tagAnalysisInputParamFromDut tagInput)
{
	int  err = ERR_OK;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		if ( 0==strcmp(tagInput.WAVE_TYPE,"WIMAX") )
		{
			if ( (0==strcmp(tagInput.WAVEFORM_FILE_NAME,""))||(0==strcmp(tagInput.WAVEFORM_FILE_NAME,"NULL")) )
			{
				// If WAVE_TYPE = WIMAX, but the user won't input the WAVEFORM_FILE_NAME name, then must return an error message.
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Since WIMAX_TEST_MODE=LINK, thus WAVEFORM_FILE_NAME can NOT be empty.\n");
				throw logMessage;
			}
			else
			{
			   /*---------------------*
				* Load waveform Files *
				*---------------------*/
				char   modFile[MAX_BUFFER_SIZE] = {'\0'};
				if ( 0==strcmp(tagInput.WAVEFORM_FILE_NAME,"AUTO") )
				{
					sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, g_WIMAXGlobalSettingParam.WIMAX_WAVEFORM_NAME);
				}
				else
				{
					sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, tagInput.WAVEFORM_FILE_NAME);
				}

				if ( 0!=strcmp(g_defaultFilePath,modFile) )	// Check mod file name first
				{
					// Store the file path
					sprintf_s(g_defaultFilePath, MAX_BUFFER_SIZE, "%s", modFile);

					// Load the whole MOD file for continuous transmit
					err = ::LP_SetVsgModulation( modFile );
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] LP_SetVsgModulation( %s ) return error.\n", modFile);
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_SetVsgModulation( %s ) return OK.\n", modFile);
					}
				}
				else
				{
					// do nothing
				}

			   /*--------------------*
				* Setup IQTester VSG *
				*--------------------*/
				double VSG_POWER = 0.0;
				if ( tagInput.RX_POWER_DBM!=NA_NUMBER )
				{
					VSG_POWER = tagInput.RX_POWER_DBM + l_cableLossDb[RX_TABLE];
				}
				else
				{
					if ( 0==strcmp(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE,"LINK") )	// LINK mode
					{
						VSG_POWER = g_WIMAXGlobalSettingParam.VSG_POWER_FOR_SYNCHRONIZATION_DBM + l_cableLossDb[RX_TABLE];
					}
					else	// NON-LINK mode
					{
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Can't retrieve the RX_POWER_DBM for VSG.\n");
						throw logMessage;	
					}
				}
				// Check VSG power will exceed VSG_MAX_POWER_WIMAX or not
				if ( VSG_POWER>g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX )	
				{					
					err = -1;	//ERR_VSG_POWER_EXCEED_LIMIT
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX);
					throw logMessage;
				}
				else
				{
					// do nothing
				}

				// Select the VSG port by WIMAX_TEST_MODE
				if ( 0==strcmp(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE,"LINK") )
				{
					if (g_WIMAXGlobalSettingParam.VSA_PORT==PORT_LEFT)
					{						
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG to PORT_RIGHT, since VSA already using PORT_LEFT.\n");
						err = ::LP_SetVsg(tagInput.FREQ_MHZ*1e6, VSG_POWER, PORT_RIGHT);
					}
					else	// VSA => PORT_RIGHT, then VSG must => PORT_LEFT 
					{						
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG to PORT_LEFT, since VSA already using PORT_RIGHT.\n");
						err = ::LP_SetVsg(tagInput.FREQ_MHZ*1e6, VSG_POWER, PORT_LEFT);
					}
				}
				else	// If NON-LINK mode, just using the VSG_PORT from Global Settings directly.
				{					
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG port by g_WIMAXGlobalSettingParam.VSG_PORT=[%d].\n", g_WIMAXGlobalSettingParam.VSG_PORT);
					err = ::LP_SetVsg(tagInput.FREQ_MHZ*1e6, VSG_POWER, g_WIMAXGlobalSettingParam.VSG_PORT);
				}	
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to setup VSG, LP_SetVsg(WIMAX signal) return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set LP_SetVsg(WIMAX signal) return OK.\n");
				}


			}	// end - if ( (0==strcmp(tagInput.WAVEFORM_FILE_NAME,""))||(0==strcmp(tagInput.WAVEFORM_FILE_NAME,"NULL")) )

		}
		else	// WAVE_TYPE != WIMAX, means => CW signal
		{
		   /*--------------------*
			* Setup IQTester VSG *
			*--------------------*/
			double VSG_POWER = 0.0;
			if ( tagInput.RX_POWER_DBM!=NA_NUMBER )
			{
				VSG_POWER = tagInput.RX_POWER_DBM + l_cableLossDb[RX_TABLE];
			}
			else
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Can't retrieve the RX_POWER_DBM for VSG.\n");
				throw logMessage;				
			}
			// Check VSG power will exceed VSG_MAX_POWER_WIMAX or not
			if ( VSG_POWER>g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX )	
			{					
				err = -1;	//ERR_VSG_POWER_EXCEED_LIMIT
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX);
				throw logMessage;
			}
			else
			{
				// do nothing
			}

			// Since there is a bug inside IQapi, the CW signal always can shift -1 MHz offset only.
		    // Or we need to Set VSG twice to fix this issue.
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG port by g_WIMAXGlobalSettingParam.VSG_PORT=[%d].\n", g_WIMAXGlobalSettingParam.VSG_PORT);
			double FLOOR_FREQ_MHZ = floor(tagInput.FREQ_MHZ);
			err = ::LP_SetVsgCw ( (FLOOR_FREQ_MHZ-1)*1e6, (tagInput.FREQ_MHZ-FLOOR_FREQ_MHZ+1) , VSG_POWER, g_WIMAXGlobalSettingParam.VSG_PORT );
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to setup VSG, LP_SetVsgCw(CW signal) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set LP_SetVsg(CW signal) return OK.\n");
			}
		}

	   /*---------------*
		*  Turn on VSG  *
		*---------------*/
		err = ::LP_EnableVsgRF(1);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to turn on VSG, LP_EnableVsgRF(1) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_EnableVsgRF(1) return OK.\n");
		}

	   /*----------------------*
		* Send packet for Test *
		*----------------------*/  
		if (g_vsgContinuousTxMode)
		{
			// VSG already under "g_vsgContinuousTxMode=1", thus do nothing.
		}
		else
		{
			err = ::LP_SetFrameCnt(0);		// continuous transmit
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] VSG send out packet LP_SetFrameCnt(0) return error.\n" );
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] VSG send out packet LP_SetFrameCnt(0) return OK.\n" );
			}

			g_vsgContinuousTxMode = true;	// set VSG to "g_vsgContinuousTxMode=true", means ON.
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txrxCalReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txrxCalReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
    }


	return err;
}

int  GetMeasureParameterFromDut(void)
{
	int    err = ERR_OK;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		//Get the measure parameter from DUT control layer

		// Check CALIBRATION_MODE, Tx or Rx mode 
		err	= ::vDUT_GetIntegerReturn (g_WIMAX_Dut, "CALIBRATION_MODE", &l_InputParamFromDut.CALIBRATION_MODE );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_GetIntegerReturn(CALIBRATION_MODE) return error.\n");
			throw logMessage;
		}

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "AVG_TIMES",  &l_InputParamFromDut.AVG_TIMES );				 
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(AVG_TIMES) return error.\n");
			//throw logMessage;
		} 

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "FREQ_MHZ",  &l_InputParamFromDut.FREQ_MHZ);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_GetDoubleReturn(FREQ_MHZ) return error.\n");
			throw logMessage;
		}

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "SIG_TYPE",  &l_InputParamFromDut.SIG_TYPE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(SIG_TYPE) return error.\n");
			//throw logMessage;
		}

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "RATE_ID",  &l_InputParamFromDut.RATE_ID);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(RATE_ID) return error.\n");
			//throw logMessage;
		}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "BANDWIDTH_MHZ", &l_InputParamFromDut.BANDWIDTH_MHZ );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetDoubleReturn(BANDWIDTH_MHZ) return error.\n");
			//throw logMessage;
		}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "CYCLIC_PREFIX", &l_InputParamFromDut.CYCLIC_PREFIX );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetDoubleReturn(CYCLIC_PREFIX) return error.\n");
			//throw logMessage;
		}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "TX_POWER_DBM", &l_InputParamFromDut.TX_POWER_DBM );
		if ( ERR_OK!=err )
		{
			l_InputParamFromDut.TX_POWER_DBM = NA_NUMBER;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetDoubleReturn(TX_POWER_DBM) return error.\n");
			//throw logMessage;
		}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "SAMPLING_TIME_US", &l_InputParamFromDut.SAMPLING_TIME_US );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetDoubleReturn(SAMPLING_TIME_US) return error.\n");
			//throw logMessage;
		}
		else
		{
			// Update the "SAMPLING_TIME_US" 
			l_txrxCalParam.SAMPLING_TIME_US = l_InputParamFromDut.SAMPLING_TIME_US;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(SAMPLING_TIME_US) return OK and update the SAMPLING_TIME_US for WiMAX calibration.\n");
		}

		err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "WAVE_TYPE", l_InputParamFromDut.WAVE_TYPE, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_GetStringReturn(WAVE_TYPE) return error.\n");
			throw logMessage;
		}

		err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "MEASURE_TYPE", l_InputParamFromDut.MEASURE_TYPE, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_GetStringReturn(MEASURE_TYPE) return error.\n");
			throw logMessage;
		}

		err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "MAP_CONFIG_FILE_NAME", l_InputParamFromDut.MAP_CONFIG_FILE_NAME, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetStringReturn(MAP_CONFIG_FILE_NAME) return error.\n");
			//throw logMessage;
		}

		err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "WAVEFORM_FILE_NAME", l_InputParamFromDut.WAVEFORM_FILE_NAME, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetStringReturn(WAVEFORM_FILE_NAME) return error.\n");
			//throw logMessage;
		}

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "TX1",  &l_InputParamFromDut.TX1 );				 
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(TX1) return error.\n");
			//throw logMessage;
		} 

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "TX2",  &l_InputParamFromDut.TX2 );				 
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(TX2) return error.\n");
			//throw logMessage;
		} 

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "TX3",  &l_InputParamFromDut.TX3 );				 
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(TX3) return error.\n");
			//throw logMessage;
		} 

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "TX4",  &l_InputParamFromDut.TX4 );				 
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(TX4) return error.\n");
			//throw logMessage;
		} 

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "RX1",  &l_InputParamFromDut.RX1 );				 
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(RX1) return error.\n");
			//throw logMessage;
		} 

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "RX2",  &l_InputParamFromDut.RX2 );				 
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(RX2) return error.\n");
			//throw logMessage;
		} 

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "RX3",  &l_InputParamFromDut.RX3 );				 
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(RX3) return error.\n");
			//throw logMessage;
		} 

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "RX4",  &l_InputParamFromDut.RX4 );				 
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(RX4) return error.\n");
			//throw logMessage;
		} 

		err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "FRAME_COUNT",  &l_InputParamFromDut.FRAME_COUNT );				 
		if (ERR_OK!=err)
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetIntegerReturn(FRAME_COUNT) return error.\n");
			//throw logMessage;
		} 

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "RX_POWER_DBM", &l_InputParamFromDut.RX_POWER_DBM );
		if ( ERR_OK!=err )
		{
			l_InputParamFromDut.RX_POWER_DBM = NA_NUMBER;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] vDUT_GetDoubleReturn(RX_POWER_DBM) return error.\n");
			//throw logMessage;
		}


		// Set to 0, means want to load path loss from table
		for (int i=0;i<MAX_DATA_STREAM;i++)
		{
			l_OutputParamToDut.TX_CABLE_LOSS_DB[i]  = 0.0;
			l_OutputParamToDut.RX_CABLE_LOSS_DB[i]  = 0.0;
		}

		l_cableLossDb[TX_TABLE]	= NA_NUMBER;
		l_cableLossDb[RX_TABLE]	= NA_NUMBER;

		if ( (l_InputParamFromDut.TX1+l_InputParamFromDut.TX2+l_InputParamFromDut.TX3+l_InputParamFromDut.TX4)>0 )	// means at least some TX ant is set to ON
		{
			// Check Tx path loss (by ant and freq)
			err = CheckPathLossTableExt(	g_WIMAX_Test_ID,
											(int)l_InputParamFromDut.FREQ_MHZ,
											l_InputParamFromDut.TX1,
											l_InputParamFromDut.TX2,
											l_InputParamFromDut.TX3,
											l_InputParamFromDut.TX4,
											l_OutputParamToDut.TX_CABLE_LOSS_DB,
											l_OutputParamToDut.TX_CABLE_LOSS_DB,
											&l_cableLossDb[TX_TABLE],
											TX_TABLE
										);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to get TX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] CheckPathLossTableExt(TX_TABLE) return OK.\n");
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] Since no TX Antenna is set to ON, thus default using TX1 to ON.\n");

			// Default using the TX ant_01 
			err = CheckPathLossTableExt(	g_WIMAX_Test_ID,
											(int)l_InputParamFromDut.FREQ_MHZ,
											1,
											0,
											0,
											0,
											l_OutputParamToDut.TX_CABLE_LOSS_DB,
											l_OutputParamToDut.TX_CABLE_LOSS_DB,
											&l_cableLossDb[TX_TABLE],
											TX_TABLE
										);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to get TX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] CheckPathLossTableExt(TX_TABLE) return OK.\n");
			}
		}

		if ( (l_InputParamFromDut.RX1+l_InputParamFromDut.RX2+l_InputParamFromDut.RX3+l_InputParamFromDut.RX4)>0 )	// means at least some RX ant is set to ON
		{
			// Check RX path loss (by ant and freq)
			err = CheckPathLossTableExt(	g_WIMAX_Test_ID,
											(int)l_InputParamFromDut.FREQ_MHZ,
											l_InputParamFromDut.RX1,
											l_InputParamFromDut.RX2,
											l_InputParamFromDut.RX3,
											l_InputParamFromDut.RX4,
											l_OutputParamToDut.RX_CABLE_LOSS_DB,
											l_OutputParamToDut.RX_CABLE_LOSS_DB,
											&l_cableLossDb[RX_TABLE],
											RX_TABLE
										);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to get RX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] CheckPathLossTableExt(RX_TABLE) return OK.\n");
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] Since no RX Antenna is set to ON, thus default using RX1 to ON.\n");

			// Default using the RX ant_01 
			err = CheckPathLossTableExt(	g_WIMAX_Test_ID,
											(int)l_InputParamFromDut.FREQ_MHZ,
											1,
											0,
											0,
											0,
											l_OutputParamToDut.RX_CABLE_LOSS_DB,
											l_OutputParamToDut.RX_CABLE_LOSS_DB,
											&l_cableLossDb[RX_TABLE],
											RX_TABLE
										);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to get RX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] CheckPathLossTableExt(RX_TABLE) return OK.\n");
			}
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txrxCalReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txrxCalReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
    }


	return	err;
}

void InitialDutCalParameters(void)
{
	l_InputParamFromDut.CALIBRATION_MODE	= CAL_TX_MODE;
	l_InputParamFromDut.AVG_TIMES			= 1;
	l_InputParamFromDut.FREQ_MHZ			= NA_NUMBER;		/*! The center frequency (MHz). */  
	l_InputParamFromDut.SIG_TYPE			= 0;				/*! Indicates the type of signal. */
	l_InputParamFromDut.RATE_ID				= 0;				/*! Indicates signal modulation rate. */
	l_InputParamFromDut.BANDWIDTH_MHZ		= 0;				/*! Indicates signal bandwidth. */
	l_InputParamFromDut.CYCLIC_PREFIX		= 0;				/*! Indicates signal cyclic prefix ratio. */
	l_InputParamFromDut.TX_POWER_DBM		= NA_NUMBER; 
	l_InputParamFromDut.SAMPLING_TIME_US	= 900;              /*! The sampling time to verify TX test. */

	strcpy_s(l_InputParamFromDut.WAVE_TYPE, MAX_BUFFER_SIZE,			"WIMAX"); 
	strcpy_s(l_InputParamFromDut.MEASURE_TYPE, MAX_BUFFER_SIZE,			"POWER");		 
	strcpy_s(l_InputParamFromDut.MAP_CONFIG_FILE_NAME, MAX_BUFFER_SIZE, "AUTO");
	strcpy_s(l_InputParamFromDut.WAVEFORM_FILE_NAME, MAX_BUFFER_SIZE,	"AUTO");

	l_InputParamFromDut.TX1				= 0;                    /*!< DUT TX1 on/off. */
	l_InputParamFromDut.TX2				= 0;                    /*!< DUT TX2 on/off. */
	l_InputParamFromDut.TX3				= 0;                    /*!< DUT TX3 on/off. */ 
	l_InputParamFromDut.TX4				= 0;                    /*!< DUT TX4 on/off. */

	// RX Parameters
	l_InputParamFromDut.FRAME_COUNT		= 1000;                 /*! Number of count to send data packet */
	l_InputParamFromDut.RX_POWER_DBM	= NA_NUMBER;            /*! The power level to verify PER. */

	l_InputParamFromDut.RX1				= 0;                    /*!< DUT RX1 on/off. */
	l_InputParamFromDut.RX2				= 0;                    /*!< DUT RX2 on/off. */
	l_InputParamFromDut.RX3				= 0;                    /*!< DUT RX3 on/off. */ 
	l_InputParamFromDut.RX4				= 0;                    /*!< DUT RX4 on/off. */

	// Result parameters
	l_OutputParamToDut.AMP_ERR_DB		= NA_NUMBER;
	l_OutputParamToDut.EVM_AVG			= NA_NUMBER;
	l_OutputParamToDut.EVM_MAX			= NA_NUMBER;
	l_OutputParamToDut.EVM_MIN			= NA_NUMBER;
	l_OutputParamToDut.FREQ_ERROR_AVG	= NA_NUMBER;
	l_OutputParamToDut.FREQ_ERROR_MAX	= NA_NUMBER;
	l_OutputParamToDut.FREQ_ERROR_MIN	= NA_NUMBER;
	l_OutputParamToDut.PHASE_ERR		= NA_NUMBER;
	l_OutputParamToDut.POWER_AVG		= NA_NUMBER;
	l_OutputParamToDut.POWER_MAX		= NA_NUMBER;
	l_OutputParamToDut.POWER_MIN		= NA_NUMBER;
}
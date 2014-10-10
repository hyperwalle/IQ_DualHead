#include "stdafx.h"
#include "TestManager.h"
#include "WIMAX_Test.h"
#include "WIMAX_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "math.h"

using namespace std;

// These global variables are declared in WIMAX_Test_Internal.cpp
extern TM_ID				g_WIMAX_Test_ID;    
extern vDUT_ID				g_WIMAX_Dut;
extern int					g_Tester_Type;
extern int					g_Tester_Number;
extern bool					g_vsgContinuousTxMode;
extern bool					g_vDutTxActived;
extern bool					g_dutConfigChanged;
extern WIMAX_RECORD_PARAM	g_RecordedParam;
extern char					g_defaultFilePath[MAX_BUFFER_SIZE];

// This global variable is declared in WIMAX_Global_Setting.cpp
extern WIMAX_GLOBAL_SETTING g_WIMAXGlobalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIMAX_SETTING_STRUCT> l_txVerifyPowerParamMap;

// Return Value Container
map<string, WIMAX_SETTING_STRUCT> l_txVerifyPowerReturnMap;

struct tagParam
{
    // Mandatory Parameters
    int    FREQ_MHZ;                                /*! The center frequency (MHz). */  
	int    STEP_NUMBER;								/*! The number of step for Power test, from 1 to MAX_POWER_STEP. Default: 1. */
	double STEP_POWER;								/*! The power of step for adjust the outpower in dB. Default: 1 dB. */
	int    SIG_TYPE;								/*! Indicates the type of signal. 1=downlink signal, 2=uplink signal, 0=auto-detect\r\nDefault: 0. */
	int    RATE_ID;									/*! Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and  correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively\r\nDefault: 0. */
	double BANDWIDTH_MHZ;							/*! Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20\r\nDefault: 0 */
    double CYCLIC_PREFIX;							/*! Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32\r\nDefault: 0. */
	double NUM_SYMBOLS;								/*! Number of OFDM symbols in burst. Default: 0. (auto detect) */
    double MAX_POWER_DBM;                           /*! The MAX output power to verify Power step. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
    double SAMPLING_TIME_US;                        /*! The sampling time to verify POWER. */
	//char   MAP_CONFIG_FILE_NAME[MAX_BUFFER_SIZE];	/*! For 802.16e analysis, a map configuration file (*.mcf) is required for non-auto-detect operation. */
	char   WAVEFORM_FILE_NAME[MAX_BUFFER_SIZE];	/*! For 802.16e, sometime VSG needs a specific mod file to synchronize the test device, if the WIMAX_TEST_MODE=LINK. The value can be AUTO(using GLOBAL_SETTINGS->WIMAX_WAVEFORM_NAME) or NULL(or empty). Default: AUTO. */

	int    VSA_AUTO_GAIN_CONTROL;                   /*! A flag that to trigger the VSA Auto Gain control before doing the signal capture, default is OFF. */
	int    VSA_GAIN_TOLERANCE_DB;	                /*! The VSA gain step tolerance for re-configure the VSA setup, default set to 0, means must configure VSA based on DUT Tx power every time. */

    // DUT Parameters
    int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on)  */
    int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */
    int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */
} l_txVerifyPowerParam;

struct tagReturn
{   
    // POWER Test Result 
    double POWER_AVERAGE_DBM[MAX_POWER_STEP];       /*!< (Average) Average power in dBm. */
    double RELATIVE_POWER_DB[MAX_POWER_STEP];       /*!< The relative power of each step in dB, must equal or less than +- 0.5 dB. */

	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyPowerReturn;

void ClearTxVerifyPowerReturn(void)
{
	l_txVerifyPowerParamMap.clear();
	l_txVerifyPowerReturnMap.clear();
}

#pragma endregion

// These global variables/functions only for WIMAX_TX_Verify_Power.cpp
int CheckTxPowerParameters( double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg );


//! WIMAX TX Verify POWER
/*!
* Input Parameters
*
*  - Mandatory 
*      -# FREQ_MHZ (double): The center frequency (MHz)
*      -# TX_POWER (double): The power (dBm) DUT is going to transmit at the antenna port
*
* Return Values
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/


WIMAX_TEST_API int WIMAX_TX_Verify_Power_Step(void)
{
    int    err = ERR_OK;

    bool   analysisOK = false, captureOK  = false;     
    int    avgIteration = 0, stepIteration = 0, vsaGainIteration = 0;
    int    dummyValue = 0;
	double samplingTimeUs = 0, peakToAvgRatio = 0, cableLossDb = 0;
	char   vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};


    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_txVerifyPowerReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_txVerifyPowerParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_txVerifyPowerReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< vector<double> >
	 * The outer vector is for MAX_POWER_STEP, and the inner vector is for averaging of measurement
	 *---------------------------------------------------------------------------------------------*/
	vector< vector<double> >    powerAvEachBurst(MAX_POWER_STEP, vector<double>(g_WIMAXGlobalSettingParam.PM_AVERAGE));


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
		err = GetInputParameters(l_txVerifyPowerParamMap);
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

#pragma region Prepare input parameters

		err = CheckTxPowerParameters( &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Prepare input parameters CheckTxPowerParameters() return OK.\n");
		}
#pragma endregion


		/*----------------------------------*
		 * Start while loop for # PowerStep *
		 *----------------------------------*/
		vsaGainIteration = 0;
		stepIteration    = 0;	
		while ( stepIteration<l_txVerifyPowerParam.STEP_NUMBER )
		{
			if ( stepIteration>MAX_POWER_STEP )		// Over the step range
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] The step number %d over the MAX step range %d.\n", stepIteration, MAX_POWER_STEP);
				throw logMessage;
			}
			else
			{
				// do nothing
			}

#pragma region SETUP VSG FOR SYNCHRONIZATION
		/*-------------------------------*
		 * SETUP VSG FOR SYNCHRONIZATION *
		 *-------------------------------*/
		if ( 0==strcmp(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE,"LINK") )
		{
			if ( (0==strcmp(l_txVerifyPowerParam.WAVEFORM_FILE_NAME,""))||(0==strcmp(l_txVerifyPowerParam.WAVEFORM_FILE_NAME,"NULL")) )
			{
				// If WIMAX_TEST_MODE = LINK, but the user won't input the WAVEFORM_FILE_NAME name, then must return an error message.
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Since WIMAX_TEST_MODE=LINK, thus WAVEFORM_FILE_NAME can NOT be empty.\n");
				throw logMessage;
			}
			else
			{
				if ( (g_dutConfigChanged==true)||(g_vDutTxActived==false) )
				{
				   /*---------------------*
					* Load waveform Files *
					*---------------------*/
					char   modFile[MAX_BUFFER_SIZE] = {'\0'};
					if ( 0==strcmp(l_txVerifyPowerParam.WAVEFORM_FILE_NAME,"AUTO") )
					{
						sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, g_WIMAXGlobalSettingParam.WIMAX_WAVEFORM_NAME);
					}
					else
					{
						sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, l_txVerifyPowerParam.WAVEFORM_FILE_NAME);
					}

					if ( 0!=strcmp(g_defaultFilePath,modFile) )
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
					double VSG_POWER = g_WIMAXGlobalSettingParam.VSG_POWER_FOR_SYNCHRONIZATION_DBM + cableLossDb;
					if ( VSG_POWER>g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX )
					{
						//ERR_VSG_POWER_EXCEED_LIMIT
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX);
						throw logMessage;
					}
					else
					{
						// do nothing
					}

					if (g_WIMAXGlobalSettingParam.VSA_PORT==PORT_LEFT)
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG to PORT_RIGHT, since VSA already using PORT_LEFT.\n");
						err = ::LP_SetVsg(l_txVerifyPowerParam.FREQ_MHZ*1e6, VSG_POWER, PORT_RIGHT);
					}
					else	// VSA => PORT_RIGHT, then VSG must => PORT_LEFT 
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG to PORT_LEFT, since VSA already using PORT_RIGHT.\n");
						err = ::LP_SetVsg(l_txVerifyPowerParam.FREQ_MHZ*1e6, VSG_POWER, PORT_LEFT);
					}
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to setup VSG, LP_SetVsg() return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_SetVsg() return OK.\n");
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

						g_vsgContinuousTxMode = 1;	// set VSG to "g_vsgContinuousTxMode=true", means ON.
					}
				}
			}
		}
		else
		{
			// WIMAX_TEST_MODE = NON-LINK, then must turn off the VSG.
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

		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna etc.
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WIMAX_Dut);

		vDUT_AddIntegerParameter(g_WIMAX_Dut, "FREQ_MHZ",				l_txVerifyPowerParam.FREQ_MHZ);	
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "NUMBER_OF_STEP",			stepIteration);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "RATE_ID",				l_txVerifyPowerParam.RATE_ID);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "SIG_TYPE",				l_txVerifyPowerParam.SIG_TYPE);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "BANDWIDTH_MHZ",			l_txVerifyPowerParam.BANDWIDTH_MHZ);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "CYCLIC_PREFIX",			l_txVerifyPowerParam.CYCLIC_PREFIX);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "NUM_SYMBOLS",			l_txVerifyPowerParam.NUM_SYMBOLS);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "TX_POWER_DBM",			l_txVerifyPowerParam.MAX_POWER_DBM-(l_txVerifyPowerParam.STEP_POWER*stepIteration) );
		//vDUT_AddStringParameter (g_WIMAX_Dut, "MAP_CONFIG_FILE_NAME",	l_txVerifyPowerParam.MAP_CONFIG_FILE_NAME);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX1",					l_txVerifyPowerParam.TX1);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX2",					l_txVerifyPowerParam.TX2);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX3",					l_txVerifyPowerParam.TX3);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX4",					l_txVerifyPowerParam.TX4);

		
		if ( (g_dutConfigChanged==true)||(g_vDutTxActived==false)||(1!=l_txVerifyPowerParam.STEP_NUMBER) )
		{
			if ( stepIteration==0 )		// If stepIteration==0, means this is the first run.  
			{
				if ( g_vDutTxActived==true )
				{
				   /*-----------*
					*  Tx Stop  *
					*-----------*/
					err = ::vDUT_Run(g_WIMAX_Dut, "TX_STOP");		
					if ( ERR_OK!=err )
					{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
						err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
						if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
						{
							err = -1;	// set err to -1, means "Error".
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
							throw logMessage;
						}
						else	// Just return normal error message in this case
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(TX_STOP) return error.\n");
							throw logMessage;
						}
					}
					else
					{
						g_vDutTxActived = false;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(TX_STOP) return OK.\n");
					}
				}
				else
				{
					// continue Dut configuration				
				}

				err = vDUT_Run(g_WIMAX_Dut, "RF_SET_FREQ");
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				   err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				   {
					   err = -1;	// set err to -1, means "Error".
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					   throw logMessage;
				   }
				   else	// Just return normal error message in this case
				   {
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(RF_SET_FREQ) return error.\n");
					   throw logMessage;
				   }
				}
				else
				{  
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(RF_SET_FREQ) return OK.\n");
				}

				err = vDUT_Run(g_WIMAX_Dut, "TX_SET_BW");
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				   err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				   {
					   err = -1;	// set err to -1, means "Error".
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					   throw logMessage;
				   }
				   else	// Just return normal error message in this case
				   {
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(TX_SET_BW) return error.\n");
					   throw logMessage;
				   }
				}
				else
				{  
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(TX_SET_BW) return OK.\n");
				}
				err = vDUT_Run(g_WIMAX_Dut, "TX_SET_DATA_RATE");
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				   err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				   {
					   err = -1;	// set err to -1, means "Error".
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					   throw logMessage;
				   }
				   else	// Just return normal error message in this case
				   {
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(TX_SET_DATA_RATE) return error.\n");
					   throw logMessage;
				   }
				}
				else
				{  
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(TX_SET_DATA_RATE) return OK.\n");
				}

				err = vDUT_Run(g_WIMAX_Dut, "TX_SET_ANTENNA");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				   err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				   {
					   err = -1;	// set err to -1, means "Error".
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					   throw logMessage;
				   }
				   else	// Just return normal error message in this case
				   {
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(TX_SET_ANTENNA) return error.\n");
					   throw logMessage;
				   }
				}
				else
				{  
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(TX_SET_ANTENNA) return OK.\n");
				}

				err = vDUT_Run(g_WIMAX_Dut, "TX_PRE_TX");
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				   err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				   {
					   err = -1;	// set err to -1, means "Error".
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					   throw logMessage;
				   }
				   else	// Just return normal error message in this case
				   {
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(TX_PRE_TX) return error.\n");
					   throw logMessage;
				   }
				}
				else
				{  
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(TX_PRE_TX) return OK.\n");
				}
			}
			else
			{
				// If stepIteration!=0, means this is not the first run. Then we can skip some vDut steps. 
				// do nothing
			}

			err = vDUT_Run(g_WIMAX_Dut, "TX_START");
			if ( ERR_OK!=err )
			{	
			   g_vDutTxActived = false;
			   // Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(TX_START) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{
			   g_vDutTxActived = true;
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(TX_START) return OK.\n");
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
		}
		else
		{
			// g_dutConfigChanged==false, do nothing
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Since g_dutConfigChanged = false, skip Dut control section.\n");
		}
#pragma endregion

#pragma region Setup LP Tester and Capture

		/*--------------------*
		 * Setup IQTester VSA *
		 *--------------------*/
		double vsaGainSteps       = l_txVerifyPowerParam.MAX_POWER_DBM-(l_txVerifyPowerParam.VSA_GAIN_TOLERANCE_DB*vsaGainIteration);
		double currentGainSetting = l_txVerifyPowerParam.MAX_POWER_DBM-(l_txVerifyPowerParam.STEP_POWER*stepIteration);
		double preGainSetting     = l_txVerifyPowerParam.MAX_POWER_DBM-(l_txVerifyPowerParam.STEP_POWER*(stepIteration-1));

		if ( (l_txVerifyPowerParam.VSA_GAIN_TOLERANCE_DB==0) || (vsaGainSteps>=currentGainSetting && vsaGainSteps<preGainSetting) )
		{
			vsaGainIteration++;

			err = ::LP_SetVsaAmplitudeTolerance(g_WIMAXGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);		
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to set VSA amplitude tolerance in dB.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_SetVsaAmplitudeTolerance(%.3f) return OK.\n", g_WIMAXGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
			}

			err = ::LP_SetVsa(  l_txVerifyPowerParam.FREQ_MHZ*1e6,
								currentGainSetting-cableLossDb+peakToAvgRatio,
								g_WIMAXGlobalSettingParam.VSA_PORT,
								0,
								g_WIMAXGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
								g_WIMAXGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000	
							  );
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WIMAX] Fail to setup VSA, LP_SetVsa() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_SetVsa() return OK.\n");
			}

			for (int pwrCheckLoop=0;pwrCheckLoop<10;pwrCheckLoop++)
			{
			   /*-------------------------------------------*
				* Perform Quick VSA capture and Power Check *
				*-------------------------------------------*/
				err = ::LP_VsaDataCapture(0.005, IQV_TRIG_TYPE_FREE_RUN); 
				if ( ERR_OK!=err )
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to Quick capture signal at %d MHz.\n", l_txVerifyPowerParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Quick LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyPowerParam.FREQ_MHZ);
				}		

				/*--------------------------------*
				 *  Perform Quick Power Analysis  *
				 *--------------------------------*/
				err = ::LP_AnalyzePower( );
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ);
					WIMAXSaveSigFile(sigFileNameBuffer);
					sprintf_s(logMessage, MAX_BUFFER_SIZE, "[WIMAX] Quick LP_AnalyzePower() return error and save the sig file %s.\n", sigFileNameBuffer);
					return err;
				}
				else		// Analyze is ok
				{
				   /*-----------------------------*
					*  Retrieve analysis Results  *
					*-----------------------------*/
					double pkPowerDbm  = ::LP_GetScalarMeasurement("P_peak_all_dBm", 0);
					if ( -99.00 >= (pkPowerDbm)  )
					{
						pkPowerDbm = NA_NUMBER;				
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Result_Failed", l_txVerifyPowerParam.FREQ_MHZ);
						WIMAXSaveSigFile(sigFileNameBuffer);
						err = -1;				
						sprintf_s(logMessage, MAX_BUFFER_SIZE, "[WIMAX] Quick LP_GetScalarMeasurement(P_peak_all_dBm) return error and save the sig file %s.\n", sigFileNameBuffer);
						return err;
					}
					else
					{			
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Quick power analysis OK, P_peak_all_dBm = %.2f.\n", pkPowerDbm);

						// Check if the power over the range, then needs some delay for power settle down
						double targetPkPower = currentGainSetting-cableLossDb+peakToAvgRatio;
						if ( fabs(targetPkPower-pkPowerDbm)>g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_TOLERANCE_DB )
						{
							if (pwrCheckLoop==5)
							{
								// Retry DUT control, but only check "TX_START" error return
								vDUT_Run(g_WIMAX_Dut, "TX_STOP");
								vDUT_Run(g_WIMAX_Dut, "RF_SET_FREQ");
								vDUT_Run(g_WIMAX_Dut, "TX_SET_BW");
								vDUT_Run(g_WIMAX_Dut, "TX_SET_DATA_RATE");
								vDUT_Run(g_WIMAX_Dut, "TX_SET_ANTENNA");
								vDUT_Run(g_WIMAX_Dut, "TX_PRE_TX");
								err = vDUT_Run(g_WIMAX_Dut, "TX_START");
								if ( ERR_OK!=err )
								{	
								   g_vDutTxActived = false;
								   // Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
								   err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
								   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
								   {
									   err = -1;	// set err to -1, means "Error".
									   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
									   throw logMessage;
								   }
								   else	// Just return normal error message in this case
								   {
									   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(TX_START) return error.\n");
									   throw logMessage;
								   }
								}
								else
								{
								   g_vDutTxActived = true;
								   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(TX_START) return OK.\n");
								}
							}
							else
							{
								// do nothing
							}
							// A delay for waitting Power settle down
							Sleep(g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_SETTLE_TIME_MS);
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Since [Target_Peak_Power_dBm(%.2f)]-[P_peak_all_dBm(%.2f)] > [%.1f] dB, thus Sleep(%d) for power settle down.\n", targetPkPower, pkPowerDbm, g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_TOLERANCE_DB, g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_SETTLE_TIME_MS);
						}
						else
						{
							// Power OK
							break;
						}
					}
				}
			}

			if (l_txVerifyPowerParam.VSA_AUTO_GAIN_CONTROL==1)
			{
				double vsaGainFromAgc = NA_NUMBER;
				err = ::LP_Agc(&vsaGainFromAgc);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WIMAX] Fail to call VSA Auto-Gain control, LP_Agc() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_Agc() return OK.\n");
				}
			}
			else
			{
				// no AGC, do nothing.
			}
		}
		else
		{
			// do nothing.
		}
		
		/*------------------------------*
		 * Start while loop for average *
		 *------------------------------*/
		avgIteration = 0;
		while ( avgIteration<g_WIMAXGlobalSettingParam.PM_AVERAGE )
		{
			analysisOK = false;
			captureOK  = false; 

		   /*----------------------------*
			* Perform Normal VSA capture *
			*----------------------------*/
			err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_WIMAXGlobalSettingParam.VSA_TRIGGER_TYPE );
			if (ERR_OK!=err)	// capture is failed
			{
				double quickPower = NA_NUMBER;					
				err = QuickCaptureRetry( l_txVerifyPowerParam.FREQ_MHZ, samplingTimeUs, g_WIMAXGlobalSettingParam.VSA_TRIGGER_TYPE, &quickPower, logMessage);
				if (ERR_OK!=err)	// QuickCaptureRetry() is failed
				{
					// Fail Capture
					if ( quickPower!=NA_NUMBER )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz.\nThe DUT power (without path loss) = %.1f dBm and QuickCaptureRetry() return error.\n", l_txVerifyPowerParam.FREQ_MHZ, quickPower);
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz, QuickCaptureRetry() return error.\n", l_txVerifyPowerParam.FREQ_MHZ);
					}
					throw logMessage;
				}				
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] The QuickCaptureRetry() at %d MHz return OK.\n", l_txVerifyPowerParam.FREQ_MHZ);
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyPowerParam.FREQ_MHZ);
			}

#pragma endregion
			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;
			if (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WIMAX_TX_Power_SaveAlways", l_txVerifyPowerParam.FREQ_MHZ);
				WIMAXSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}

			/*------------------*
			 *  Power Analysis  *
			 *------------------*/
			double dummy_T_INTERVAL      = 3.2;
			double dummy_MAX_POW_DIFF_DB = 15.0;  
			err = ::LP_AnalyzePower( dummy_T_INTERVAL/1000000, dummy_MAX_POW_DIFF_DB );
			if ( ERR_OK!=err )
			{
				// Fail Analysis, thus save capture (Signal File) for debug
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WIMAX_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ);
				WIMAXSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] LP_AnalyzePower() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_AnalyzePower() return OK.\n");
			}

#pragma region Retrieve analysis Results
			/*-----------------------------*
			 *  Retrieve analysis Results  *
			 *-----------------------------*/
			avgIteration++;
			analysisOK = true;

			// Since the limitation, we assume that all path loss value are very close.	

			// powerAvEachBurst  
			powerAvEachBurst[stepIteration][avgIteration-1] = ::LP_GetScalarMeasurement("P_av_no_gap_all_dBm",0);
			if ( -99.00 >= powerAvEachBurst[stepIteration][avgIteration-1] )
			{
				analysisOK = false;
				powerAvEachBurst[stepIteration][avgIteration-1] = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WIMAX_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ);
				WIMAXSaveSigFile(sigFileNameBuffer);
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_av_no_gap_all_dBm) return error.\n");
				throw logMessage;
			}
			else
			{
				//The current code will work correctly with 4x4 MIMO.  
				int antenaOrder = 0;
				err = CheckAntennaOrderByStream(1, l_txVerifyPowerParam.TX1, l_txVerifyPowerParam.TX2, l_txVerifyPowerParam.TX3, l_txVerifyPowerParam.TX4, &antenaOrder);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] CheckAntennaOrderByStream() return error.\n");					
					throw logMessage;
				}

				powerAvEachBurst[stepIteration][avgIteration-1] = powerAvEachBurst[stepIteration][avgIteration-1] + l_txVerifyPowerParam.CABLE_LOSS_DB[antenaOrder-1];					
			}

#pragma endregion
		}   // End - avgIteration

			stepIteration++;

		}   // End - stepIteration
		
#pragma region Averaging and Saving Test Result
	   /*----------------------------------*
		* Averaging and Saving Test Result *
		*----------------------------------*/
		if ( (ERR_OK==err) && captureOK && analysisOK )
		{
			double dummyMax=0.0, dummyMin=0.0;

			for(int i=0;i<stepIteration;i++)
			{
				// Average Power test result
				::AverageTestResult(&powerAvEachBurst[i][0], avgIteration, LOG_10, l_txVerifyPowerReturn.POWER_AVERAGE_DBM[i], dummyMax, dummyMin);
			}

			for (int i=0;i<(stepIteration-1);i++)
			{
				l_txVerifyPowerReturn.RELATIVE_POWER_DB[i]	= l_txVerifyPowerReturn.POWER_AVERAGE_DBM[i+1] - l_txVerifyPowerReturn.POWER_AVERAGE_DBM[i];
			}
		}
		else
		{
			// do nothing
		}
#pragma endregion 

		// This is a special case, only if "DUT_KEEP_TRANSMIT=0" then must do TX_STOP manually.
		if ( (g_WIMAXGlobalSettingParam.DUT_KEEP_TRANSMIT==0)&&(g_vDutTxActived==true) )
		{
		   /*-----------*
			*  Tx Stop  *
			*-----------*/
			err = ::vDUT_Run(g_WIMAX_Dut, "TX_STOP");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(TX_STOP) return OK.\n");
			}
		}

		/*----------------------*
		 *  Function Completed  *
		 *----------------------*/
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			sprintf_s(l_txVerifyPowerReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txVerifyPowerReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txVerifyPowerReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
		err = -1;
    }

	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	ReturnTestResults(l_txVerifyPowerReturnMap);

    // Free memory
    powerAvEachBurst.clear();
    
    return err;
}

void InitializeTXVerifyPowerContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txVerifyPowerParamMap.clear();

    WIMAX_SETTING_STRUCT setting;

    l_txVerifyPowerParam.VSA_AUTO_GAIN_CONTROL = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.VSA_AUTO_GAIN_CONTROL))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.VSA_AUTO_GAIN_CONTROL;
        setting.unit        = "";
        setting.helpText    = "A flag that to trigger the VSA Auto Gain control before doing the signal capture, default is 0 (OFF).";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("VSA_AUTO_GAIN_CONTROL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.VSA_GAIN_TOLERANCE_DB = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.VSA_GAIN_TOLERANCE_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.VSA_GAIN_TOLERANCE_DB;
        setting.unit        = "dB";
        setting.helpText    = "The VSA gain step tolerance for re-configure the VSA setup, default set to 0 dB, means must configure VSA based on DUT Tx power every time.";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("VSA_GAIN_TOLERANCE_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.FREQ_MHZ = 3500;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz.";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("FREQ_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.STEP_NUMBER = 45;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.STEP_NUMBER))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.STEP_NUMBER;
        setting.unit        = "";
        setting.helpText    = "The number of step for Power test, from 1 to MAX_POWER_STEP(100). Default: 1.";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("STEP_NUMBER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.STEP_POWER = 1.00;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyPowerParam.STEP_POWER))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.STEP_POWER;
        setting.unit        = "dB";
        setting.helpText    = "The power of step for adjust the outpower in dB. Default: 1 dB.";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("STEP_POWER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.SIG_TYPE = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.SIG_TYPE))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.SIG_TYPE;
        setting.unit        = "";
        setting.helpText    = "Indicates the type of signal. 1=downlink signal, 2=uplink signal, 0=auto-detect\r\nDefault: 0.";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SIG_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.BANDWIDTH_MHZ = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyPowerParam.BANDWIDTH_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.BANDWIDTH_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20\r\nDefault: 0.";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("BANDWIDTH_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.CYCLIC_PREFIX = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyPowerParam.CYCLIC_PREFIX))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.CYCLIC_PREFIX;
        setting.unit        = "";
        setting.helpText    = "Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32\r\nDefault: 0.";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CYCLIC_PREFIX", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.RATE_ID = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.RATE_ID))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.RATE_ID;
        setting.unit        = "";
        setting.helpText    = "Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and  correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively\r\nDefault: 0.";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RATE_ID", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.NUM_SYMBOLS = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyPowerParam.NUM_SYMBOLS))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.NUM_SYMBOLS;
        setting.unit        = "";
        setting.helpText    = "Number of OFDM symbols in burst. Default: 0 (auto detect).";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("NUM_SYMBOLS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.SAMPLING_TIME_US = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyPowerParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyPowerParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyPowerParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyPowerParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyPowerParam.MAX_POWER_DBM = 20.0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyPowerParam.MAX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.MAX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "MAX power level at DUT antenna port for Power Step test";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("MAX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.TX1 = 1;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX1))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.TX1;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.TX2 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX2))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.TX2;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.TX3 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX3))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.TX3;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.TX4 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX4))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.TX4;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyPowerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
	//strcpy_s(l_txVerifyPowerParam.MAP_CONFIG_FILE_NAME, MAX_BUFFER_SIZE, "AUTO");
	//setting.type = WIMAX_SETTING_TYPE_STRING;
	//if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.MAP_CONFIG_FILE_NAME))    // Type_Checking
 //   {
 //       setting.value = (void*)l_txVerifyPowerParam.MAP_CONFIG_FILE_NAME;
 //       setting.unit  = "";
 //       setting.helpText = "For 802.16e analysis, a map configuration file (*.mcf) is required for non-auto-detect operation.\r\nDefault value is AUTO";
 //       l_txVerifyPowerParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("MAP_CONFIG_FILE_NAME", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

	strcpy_s(l_txVerifyPowerParam.WAVEFORM_FILE_NAME, MAX_BUFFER_SIZE, "AUTO");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.WAVEFORM_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyPowerParam.WAVEFORM_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "For 802.16e, sometime VSG needs a specific mod file to synchronize the test device, if the WIMAX_TEST_MODE=LINK. The value can be AUTO(using GLOBAL_SETTINGS->WIMAX_WAVEFORM_NAME) or NULL(or empty). Default: AUTO.";
        l_txVerifyPowerParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("WAVEFORM_FILE_NAME", setting) );
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
    l_txVerifyPowerReturnMap.clear();

	for (int i=0;i<MAX_POWER_STEP;i++)
	{
		l_txVerifyPowerReturn.POWER_AVERAGE_DBM[i] = NA_NUMBER;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_AVERAGE_DBM[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyPowerReturn.POWER_AVERAGE_DBM[i];
            char tempStr[MAX_BUFFER_SIZE];
			if (i<9)	
			{
				sprintf_s(tempStr, "POWER_AVERAGE_DBM_0%d", i+1);
			}
			else
			{
				sprintf_s(tempStr, "POWER_AVERAGE_DBM_%d", i+1);
			}
			setting.unit        = "dBm";
			setting.helpText    = "Average power in dBm.";
			l_txVerifyPowerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}
    
	for (int i=0;i<MAX_POWER_STEP;i++)
	{
		l_txVerifyPowerReturn.RELATIVE_POWER_DB[i] = NA_NUMBER;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyPowerReturn.RELATIVE_POWER_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyPowerReturn.RELATIVE_POWER_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
			if (i<9)	
			{
				sprintf_s(tempStr, "RELATIVE_POWER_DB_0%d", i+1);
			}
			else
			{
				sprintf_s(tempStr, "RELATIVE_POWER_DB_%d", i+1);
			}            
			setting.unit        = "dB";
			setting.helpText    = "The relative power of each step in dB, must equal or less than +- 0.5 dB.";
			l_txVerifyPowerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}
    
	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyPowerReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyPowerReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyPowerReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyPowerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyPowerReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyPowerReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_txVerifyPowerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}


//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
// 
//-------------------------------------------------------------------------------------
int CheckTxPowerParameters( double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Check path loss (by ant and freq)
		err = CheckPathLossTableExt( g_WIMAX_Test_ID,
								  l_txVerifyPowerParam.FREQ_MHZ,
								  l_txVerifyPowerParam.TX1,
								  l_txVerifyPowerParam.TX2,
								  l_txVerifyPowerParam.TX3,
								  l_txVerifyPowerParam.TX4,
								  l_txVerifyPowerParam.CABLE_LOSS_DB,
								  l_txVerifyPowerReturn.CABLE_LOSS_DB,
								  cableLossDb,
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

		// Check sampling time 
		if (0==l_txVerifyPowerParam.SAMPLING_TIME_US)
		{
			*samplingTimeUs = g_WIMAXGlobalSettingParam.PM_SAMPLE_INTERVAL_US;
		}
		else	// SAMPLING_TIME_US != 0
		{
			*samplingTimeUs = l_txVerifyPowerParam.SAMPLING_TIME_US;
		}
		
		// Peak to Average Ratio
		*peakToAvgRatio = g_WIMAXGlobalSettingParam.IQ_P_TO_A_WIMAX;  

		// Check Dut configuration changed or not
		if (  g_WIMAXGlobalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.			  
			  l_txVerifyPowerParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
			  l_txVerifyPowerParam.SIG_TYPE!=g_RecordedParam.SIG_TYPE ||
			  l_txVerifyPowerParam.BANDWIDTH_MHZ!=g_RecordedParam.BANDWIDTH_MHZ ||
			  l_txVerifyPowerParam.CYCLIC_PREFIX!=g_RecordedParam.CYCLIC_PREFIX ||
			  l_txVerifyPowerParam.RATE_ID!=g_RecordedParam.RATE_ID ||
			  l_txVerifyPowerParam.NUM_SYMBOLS!=g_RecordedParam.NUM_SYMBOLS ||
			  l_txVerifyPowerParam.MAX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
			  l_txVerifyPowerParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_txVerifyPowerParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_txVerifyPowerParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_txVerifyPowerParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  //0!=strcmp(l_txVerifyPowerParam.MAP_CONFIG_FILE_NAME, g_RecordedParam.MAP_CONFIG_FILE_NAME) ||
			  l_txVerifyPowerParam.TX1!=g_RecordedParam.ANT1 ||
			  l_txVerifyPowerParam.TX2!=g_RecordedParam.ANT2 ||
			  l_txVerifyPowerParam.TX3!=g_RecordedParam.ANT3 ||
			  l_txVerifyPowerParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;			
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup

		g_RecordedParam.FREQ_MHZ				= l_txVerifyPowerParam.FREQ_MHZ;	
		g_RecordedParam.POWER_DBM				= l_txVerifyPowerParam.MAX_POWER_DBM;	
	    g_RecordedParam.SIG_TYPE				= l_txVerifyPowerParam.SIG_TYPE;
	    g_RecordedParam.BANDWIDTH_MHZ			= l_txVerifyPowerParam.BANDWIDTH_MHZ;
	    g_RecordedParam.CYCLIC_PREFIX			= l_txVerifyPowerParam.CYCLIC_PREFIX;
	    g_RecordedParam.RATE_ID					= l_txVerifyPowerParam.RATE_ID;
	    g_RecordedParam.NUM_SYMBOLS				= l_txVerifyPowerParam.NUM_SYMBOLS;
		g_RecordedParam.ANT1					= l_txVerifyPowerParam.TX1;
		g_RecordedParam.ANT2					= l_txVerifyPowerParam.TX2;
		g_RecordedParam.ANT3					= l_txVerifyPowerParam.TX3;
		g_RecordedParam.ANT4					= l_txVerifyPowerParam.TX4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txVerifyPowerParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txVerifyPowerParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txVerifyPowerParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txVerifyPowerParam.CABLE_LOSS_DB[3];	
		//sprintf_s(g_RecordedParam.MAP_CONFIG_FILE_NAME, MAX_BUFFER_SIZE, l_txVerifyPowerParam.MAP_CONFIG_FILE_NAME);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] CheckTxPowerParameters() Confirmed.\n");
	}
	catch(char *msg)
    {
        sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
    }
    catch(...)
    {
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] Unknown Error!\n");
		err = -1;
    }

	
	return err;
}

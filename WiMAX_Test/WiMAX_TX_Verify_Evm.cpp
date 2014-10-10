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

// This global variable is declared in WIMAX_GLOBAL_SETTING.cpp
extern WIMAX_GLOBAL_SETTING g_WIMAXGlobalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIMAX_SETTING_STRUCT> l_txVerifyEvmParamMap;

// Return Value Container
map<string, WIMAX_SETTING_STRUCT> l_txVerifyEvmReturnMap;

struct tagParam
{
    // Mandatory Parameters
    int    FREQ_MHZ;                                /*! The center frequency (MHz). */  
	int    SIG_TYPE;								/*! Indicates the type of signal. 1=downlink signal, 2=uplink signal, 0=auto-detect\r\nDefault: 0. */
	int    RATE_ID;									/*! Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively\r\nDefault: 0. */
	double BANDWIDTH_MHZ;							/*! Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20\r\nDefault: 0 */
    double CYCLIC_PREFIX;							/*! Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32\r\nDefault: 0. */
	double NUM_SYMBOLS;								/*! Number of OFDM symbols in burst. Default: 0. (auto detect) */
    double TX_POWER_DBM;                            /*! The output power to verify EVM. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
    double SAMPLING_TIME_US;                        /*! The sampling time to verify EVM. */
	char   MAP_CONFIG_FILE_NAME[MAX_BUFFER_SIZE];	/*! For 802.16e analysis, a map configuration file (*.mcf) is required for non-auto-detect operation. */
	char   WAVEFORM_FILE_NAME[MAX_BUFFER_SIZE];	/*! For 802.16e, sometime VSG needs a specific mod file to synchronize the test device, if the WIMAX_TEST_MODE=LINK. The value can be AUTO(using GLOBAL_SETTINGS->WIMAX_WAVEFORM_NAME) or NULL(or empty). Default: AUTO. */

    // DUT Parameters
    int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on)  */
    int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */ 
    int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */
} l_txVerifyEvmParam;

struct tagReturn
{
    // EVM Test Result 
    double EVM_AVG_ALL_MAX_DB[MAX_DATA_STREAM];		   /*!< (MAX) EVM test results on specific data stream. */
	double EVM_AVG_ALL_MIN_DB[MAX_DATA_STREAM];		   /*!< (MIN) EVM test results on specific data stream. */
    double EVM_AVG_ALL_DB[MAX_DATA_STREAM];            /*!< (Average) EVM test result on specific data stream. */
  
    // POWER Test Result 
 	double AVG_POWER_NO_GAP_MAX_DBM[MAX_DATA_STREAM];  /*!< (MAX) POWER test result on specific data stream. */
	double AVG_POWER_NO_GAP_MIN_DBM[MAX_DATA_STREAM];  /*!< (MIN) POWER test result on specific data stream. */
    double AVG_POWER_NO_GAP_DBM[MAX_DATA_STREAM];      /*!< (Average) POWER test result on specific data stream. */

    // Phase Noise Test Result 
	double PHASE_NOISE_DEG_RMS_ALL_MAX[MAX_DATA_STREAM];/*!< (MAX) Phase Noise test result on specific data stream. */
	double PHASE_NOISE_DEG_RMS_ALL_MIN[MAX_DATA_STREAM];/*!< (MIN) Phase Noise test result on specific data stream. */
    double PHASE_NOISE_DEG_RMS_ALL[MAX_DATA_STREAM];    /*!< (Average) Phase Noise test result on specific data stream. */

    // Frequency Error Test Result 
    double FREQ_ERROR_MAX_KHZ[MAX_DATA_STREAM];		   /*!< (MAX) Frequency Error test result on specific data stream. */
    double FREQ_ERROR_MIN_KHZ[MAX_DATA_STREAM];		   /*!< (MIN) Frequency Error test result on specific data stream. */
    double FREQ_ERROR_KHZ[MAX_DATA_STREAM];			   /*!< (Average) Frequency Error test result on specific data stream. */

    // Symbol Clock Error Test Result 
    double SYM_CLOCK_ERROR_MAX_PPM[MAX_DATA_STREAM];   /*!< (MAX) Symbol Clock Error test result on specific data stream. */
    double SYM_CLOCK_ERROR_MIN_PPM[MAX_DATA_STREAM];   /*!< (MIN) Symbol Clock Error test result on specific data stream. */
    double SYM_CLOCK_ERROR_PPM[MAX_DATA_STREAM];	   /*!< (Average) Symbol Clock Error test result on specific data stream. */

    // IQ Amplitude Imbalance Test Result 
    double IQIMBAL_AMP_MAX_DB[MAX_DATA_STREAM];		   /*!< (MAX) IQ Amplitude Imbalance test result on specific data stream. */
    double IQIMBAL_AMP_MIN_DB[MAX_DATA_STREAM];		   /*!< (MIN) IQ Amplitude Imbalance test result on specific data stream. */
    double IQIMBAL_AMP_DB[MAX_DATA_STREAM];			   /*!< (Average) IQ Amplitude Imbalance test result on specific data stream. */

    // IQ Phase Imbalance Test Result 
    double IQIMBAL_PHASE_MAX_DEG[MAX_DATA_STREAM];	   /*!< (MAX) IQ Phase Imbalance test result on specific data stream. */
    double IQIMBAL_PHASE_MIN_DEG[MAX_DATA_STREAM];	   /*!< (MIN) IQ Phase Imbalance test result on specific data stream. */
    double IQIMBAL_PHASE_DEG[MAX_DATA_STREAM];		   /*!< (Average) IQ Phase Imbalance test result on specific data stream. */

    // DC Leakage Test Result 
    double DC_LEAKAGE_MAX_DBC[MAX_DATA_STREAM];		   /*!< (MAX) DC Leakage test result on specific data stream. */
    double DC_LEAKAGE_MIN_DBC[MAX_DATA_STREAM];		   /*!< (MIN) DC Leakage test result on specific data stream. */
    double DC_LEAKAGE_DBC[MAX_DATA_STREAM];		       /*!< (Average) DC Leakage test result on specific data stream. */

	double CABLE_LOSS_DB[MAX_DATA_STREAM];             /*! The path loss of test system. */

    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyEvmReturn;

void ClearTxVerifyEvmReturn(void)
{
	l_txVerifyEvmParamMap.clear();
	l_txVerifyEvmReturnMap.clear();
}

#pragma endregion

// These global variables/functions only for WIMAX_TX_Verify_Evm.cpp
int CheckTxEvmParameters( double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg );


//! WIMAX TX Verify EVM
/*!
* Input Parameters
*
*  - Mandatory 
*      -# FREQ_MHZ (double): The center frequency (MHz)
*      -# TX_POWER_DBM (double): The power (dBm) DUT is going to transmit at the antenna port
*
* Return Values
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/


WIMAX_TEST_API int WIMAX_TX_Verify_Evm(void)
{
    int    err = ERR_OK;

    bool   analysisOK = false, captureOK  = false;     
    int    avgIteration = 0;
    int    dummyValue = 0;
	double samplingTimeUs = 0, peakToAvgRatio = 0, cableLossDb = 0;
	char   vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};


    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_txVerifyEvmReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_txVerifyEvmParamMap);
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
        RespondToQueryReturn(l_txVerifyEvmReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< vector<double> >
	 * The outer vector is for TX streams, and the inner vector is for averaging of measurement
	 *-----------------------------------------------------------------------------------------*/
	vector< vector<double> >    evmAvgAll(MAX_DATA_STREAM, vector<double>(g_WIMAXGlobalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    avgPowerNoGapDbm(MAX_DATA_STREAM, vector<double>(g_WIMAXGlobalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    phaseNoiseDegRmsAll(MAX_DATA_STREAM, vector<double>(g_WIMAXGlobalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    freqErrorKhz(MAX_DATA_STREAM, vector<double>(g_WIMAXGlobalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    symClockErrorPpm(MAX_DATA_STREAM, vector<double>(g_WIMAXGlobalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    iqImbalAmpDb(MAX_DATA_STREAM, vector<double>(g_WIMAXGlobalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    iqImbalPhaseDeg(MAX_DATA_STREAM, vector<double>(g_WIMAXGlobalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    dcLeakageDbc(MAX_DATA_STREAM, vector<double>(g_WIMAXGlobalSettingParam.EVM_AVERAGE));

	
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
		err = GetInputParameters(l_txVerifyEvmParamMap);
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

		err = CheckTxEvmParameters( &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Prepare input parameters CheckTxEvmParameters() return OK.\n");
		}

#pragma endregion

#pragma region SETUP VSG FOR SYNCHRONIZATION
		/*-------------------------------*
		 * SETUP VSG FOR SYNCHRONIZATION *
		 *-------------------------------*/
		if ( 0==strcmp(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE,"LINK") )
		{
			if ( (0==strcmp(l_txVerifyEvmParam.WAVEFORM_FILE_NAME,""))||(0==strcmp(l_txVerifyEvmParam.WAVEFORM_FILE_NAME,"NULL")) )
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
					if ( 0==strcmp(l_txVerifyEvmParam.WAVEFORM_FILE_NAME,"AUTO") )
					{
						sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, g_WIMAXGlobalSettingParam.WIMAX_WAVEFORM_NAME);
					}
					else
					{
						sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, l_txVerifyEvmParam.WAVEFORM_FILE_NAME);
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
						err = ::LP_SetVsg(l_txVerifyEvmParam.FREQ_MHZ*1e6, VSG_POWER, PORT_RIGHT);
					}
					else	// VSA => PORT_RIGHT, then VSG must => PORT_LEFT 
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG to PORT_LEFT, since VSA already using PORT_RIGHT.\n");
						err = ::LP_SetVsg(l_txVerifyEvmParam.FREQ_MHZ*1e6, VSG_POWER, PORT_LEFT);
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

						g_vsgContinuousTxMode = true;	// set VSG to "g_vsgContinuousTxMode=true", means ON.
					}
				}
				else
				{
					// DUT still transmit, thus do nothing.
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

		vDUT_AddIntegerParameter(g_WIMAX_Dut, "FREQ_MHZ",				l_txVerifyEvmParam.FREQ_MHZ);	
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "SIG_TYPE",				l_txVerifyEvmParam.SIG_TYPE);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "RATE_ID",				l_txVerifyEvmParam.RATE_ID);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "BANDWIDTH_MHZ",			l_txVerifyEvmParam.BANDWIDTH_MHZ);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "CYCLIC_PREFIX",			l_txVerifyEvmParam.CYCLIC_PREFIX);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "NUM_SYMBOLS",			l_txVerifyEvmParam.NUM_SYMBOLS);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "TX_POWER_DBM",			l_txVerifyEvmParam.TX_POWER_DBM);
		vDUT_AddStringParameter (g_WIMAX_Dut, "MAP_CONFIG_FILE_NAME",	l_txVerifyEvmParam.MAP_CONFIG_FILE_NAME);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX1",					l_txVerifyEvmParam.TX1);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX2",					l_txVerifyEvmParam.TX2);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX3",					l_txVerifyEvmParam.TX3);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX4",					l_txVerifyEvmParam.TX4);


		if ( (g_dutConfigChanged==true)||(g_vDutTxActived==false) )
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

		err = ::LP_SetVsa(  l_txVerifyEvmParam.FREQ_MHZ*1e6,
							l_txVerifyEvmParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio,
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to Quick capture signal at %d MHz.\n", l_txVerifyEvmParam.FREQ_MHZ);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Quick LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyEvmParam.FREQ_MHZ);
			}		

			/*--------------------------------*
			 *  Perform Quick Power Analysis  *
			 *--------------------------------*/
			err = ::LP_AnalyzePower( );
			if ( ERR_OK!=err )
			{
				// Fail Analysis, thus save capture (Signal File) for debug
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Analysis_Failed", l_txVerifyEvmParam.FREQ_MHZ);
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
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Result_Failed", l_txVerifyEvmParam.FREQ_MHZ);
					WIMAXSaveSigFile(sigFileNameBuffer);
					err = -1;				
					sprintf_s(logMessage, MAX_BUFFER_SIZE, "[WIMAX] Quick LP_GetScalarMeasurement(P_peak_all_dBm) return error and save the sig file %s.\n", sigFileNameBuffer);
					return err;
				}
				else
				{			
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Quick power analysis OK, P_peak_all_dBm = %.2f.\n", pkPowerDbm);

					// Check if the power over the range, then needs some delay for power settle down
					double targetPkPower = l_txVerifyEvmParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio;
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
		
		/*------------------*
		 * Start While Loop *
		 *------------------*/
		avgIteration = 0;
		while ( avgIteration<g_WIMAXGlobalSettingParam.EVM_AVERAGE )
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
				err = QuickCaptureRetry( l_txVerifyEvmParam.FREQ_MHZ, samplingTimeUs, g_WIMAXGlobalSettingParam.VSA_TRIGGER_TYPE, &quickPower, logMessage);
				if (ERR_OK!=err)	// QuickCaptureRetry() is failed
				{
					// Fail Capture
					if ( quickPower!=NA_NUMBER )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz.\nThe DUT power (without path loss) = %.1f dBm and QuickCaptureRetry() return error.\n", l_txVerifyEvmParam.FREQ_MHZ, quickPower);
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz, QuickCaptureRetry() return error.\n", l_txVerifyEvmParam.FREQ_MHZ);
					}
					throw logMessage;
				}				
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] The QuickCaptureRetry() at %d MHz return OK.\n", l_txVerifyEvmParam.FREQ_MHZ);
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyEvmParam.FREQ_MHZ);
			}
#pragma endregion

			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;
			if (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WIMAX_TX_Evm_SaveAlways", l_txVerifyEvmParam.FREQ_MHZ);
				WIMAXSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}

			/*----------------*
			 *  EVM Analysis  *
			 *----------------*/
#pragma region Analysis_802_16e

			double sigType=-1, bandwidthMHz=-1, cyclicPrefix=-1, rateId=-1, numSymbols=-1;

			if (0==l_txVerifyEvmParam.SIG_TYPE)			sigType			= -1;
				else									sigType			= l_txVerifyEvmParam.SIG_TYPE;	
			if (0==l_txVerifyEvmParam.BANDWIDTH_MHZ)	bandwidthMHz	= -1;
				else									bandwidthMHz	= l_txVerifyEvmParam.BANDWIDTH_MHZ;
			if (0==l_txVerifyEvmParam.CYCLIC_PREFIX)	cyclicPrefix	= -1;
				else									cyclicPrefix	= l_txVerifyEvmParam.CYCLIC_PREFIX;
			if (0==l_txVerifyEvmParam.RATE_ID)			rateId			= l_txVerifyEvmParam.RATE_ID - 1;
				else									rateId			= l_txVerifyEvmParam.RATE_ID - 1;
			if (0==l_txVerifyEvmParam.NUM_SYMBOLS)		numSymbols		= -1;
				else									numSymbols		= l_txVerifyEvmParam.NUM_SYMBOLS;


			if ( 0==strcmp(l_txVerifyEvmParam.MAP_CONFIG_FILE_NAME, "AUTO") )
			{
				err = ::LP_Analyze80216e( sigType,
										  bandwidthMHz*1e6,
										  cyclicPrefix,
										  rateId,
										  numSymbols,
										  g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_PH_CORR_MODE,
										  g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_CH_ESTIMATE,
										  g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_FREQ_SYNC,
										  g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_SYM_TIM_CORR,										  
										  g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_AMPL_TRACK
										);
			}
			else
			{
				char   mapFile[MAX_BUFFER_SIZE] = {'\0'};
				sprintf_s(mapFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, l_txVerifyEvmParam.MAP_CONFIG_FILE_NAME);
				
				err = ::LP_Analyze80216e( sigType,
										  bandwidthMHz*1e6,
										  cyclicPrefix,
										  rateId,
										  numSymbols,
										  g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_PH_CORR_MODE,
										  g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_CH_ESTIMATE,
										  g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_FREQ_SYNC,
										  g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_SYM_TIM_CORR,										  
										  g_WIMAXGlobalSettingParam.ANALYSIS_WIMAX_AMPL_TRACK,
										  0,
										  mapFile
										);

			}

			if ( ERR_OK!=err )
			{
				// Fail Analysis, thus save capture (Signal File) for debug
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WIMAX_TX_Evm_Analyze80216e_Failed", l_txVerifyEvmParam.FREQ_MHZ);
				WIMAXSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] LP_Analyze80216e() return error.\n");					
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_Analyze80216e() return OK.\n");
			}	
#pragma endregion

#pragma region Retrieve analysis Results
			/*-----------------------------*
			 *  Retrieve analysis Results  *
			 *-----------------------------*/
			char sTestDetail[MAX_BUFFER_SIZE] = {'\0'};
			sprintf_s(sTestDetail, MAX_BUFFER_SIZE, "WIMAX_TX_Verify_Evm_%d", l_txVerifyEvmParam.FREQ_MHZ);

			avgIteration++;
			analysisOK = true;

			// Store the result, includes EVM, power etc.              
			if ( g_Tester_Type==IQ_MAX )        // 802.16e, only one tester
			{
				// EVM
				evmAvgAll[0][avgIteration-1] = ::LP_GetScalarMeasurement("evmAvgAll",0); 
				if ( -99.00 >= evmAvgAll[0][avgIteration-1] )
				{
					analysisOK = false;
					evmAvgAll[0][avgIteration-1] = NA_NUMBER;
					WIMAXSaveSigFile(sTestDetail);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(evmAvgAll) return error.\n");
					throw logMessage;
				}
				
				// Power 
				avgPowerNoGapDbm[0][avgIteration-1] = ::LP_GetScalarMeasurement("avgPowerNoGapDb",0);
				if ( -99.00 >= avgPowerNoGapDbm[0][avgIteration-1] )
				{
					analysisOK = false;
					avgPowerNoGapDbm[0][avgIteration-1] = NA_NUMBER;
					WIMAXSaveSigFile(sTestDetail);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(avgPowerNoGapDb) return error.\n");
					throw logMessage;
				}
				else
				{
					// One stream data rate, should be only one antenna is ON	
					int antenaOrder = 0;
					err = CheckAntennaOrderByStream( 1, l_txVerifyEvmParam.TX1, l_txVerifyEvmParam.TX2, l_txVerifyEvmParam.TX3, l_txVerifyEvmParam.TX4, &antenaOrder);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] CheckAntennaOrderByStream() return error.\n");					
						throw logMessage;
					}

					avgPowerNoGapDbm[0][avgIteration-1] = avgPowerNoGapDbm[0][avgIteration-1] + l_txVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
				}

				// Phase noise Error
				phaseNoiseDegRmsAll[0][avgIteration-1] = ::LP_GetScalarMeasurement("phaseNoiseDegRmsAll", 0); 

				// Frequency Error
				freqErrorKhz[0][avgIteration-1] = ::LP_GetScalarMeasurement("freqErrorHz", 0); 
				freqErrorKhz[0][avgIteration-1] = freqErrorKhz[0][avgIteration-1] / 1000.0;	// kHz

				// Symbol clock Error (ppm)
				symClockErrorPpm[0][avgIteration-1] = ::LP_GetScalarMeasurement("symClockErrorPpm", 0); 

				// IQ Amplitude Error
				iqImbalAmpDb[0][avgIteration-1] = ::LP_GetScalarMeasurement("iqImbalAmplDb", 0); 

				// IQ Phase Error.
				iqImbalPhaseDeg[0][avgIteration-1] = ::LP_GetScalarMeasurement("iqImbalPhaseDeg", 0);

				// IQ Phase Error.
				dcLeakageDbc[0][avgIteration-1] = ::LP_GetScalarMeasurement("dcLeakageDbc", 0);

			}
			else    // 802.16e, more than one stream
			{				
				for(int i=0;i<g_Tester_Number;i++)
				{
					// EVM
					evmAvgAll[i][avgIteration-1] = ::LP_GetScalarMeasurement("evmAvgAll", i); 
					if ( -99.00 >= evmAvgAll[i][avgIteration-1] )
					{
						analysisOK = false;
						evmAvgAll[i][avgIteration-1] = NA_NUMBER;
						WIMAXSaveSigFile(sTestDetail);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(evmAvgAll) return error.\n");
						throw logMessage;
					}
					
					// Power 
					avgPowerNoGapDbm[i][avgIteration-1] = ::LP_GetScalarMeasurement("avgPowerNoGapDb", i*(g_Tester_Number+1));
					if ( -99.00 >= avgPowerNoGapDbm[i][avgIteration-1] )
					{
						analysisOK = false;
						avgPowerNoGapDbm[i][avgIteration-1] = NA_NUMBER;
						WIMAXSaveSigFile(sTestDetail);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(avgPowerNoGapDb) return error.\n");
						throw logMessage;
					}
					else
					{
                        //The current code will work correctly with 4x4 MIMO.  
						int antenaOrder = 0;
						err = CheckAntennaOrderByStream(i+1, l_txVerifyEvmParam.TX1, l_txVerifyEvmParam.TX2, l_txVerifyEvmParam.TX3, l_txVerifyEvmParam.TX4, &antenaOrder);
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] CheckAntennaOrderByStream() return error.\n");					
							throw logMessage;
						}

						avgPowerNoGapDbm[i][avgIteration-1] = avgPowerNoGapDbm[i][avgIteration-1] + l_txVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
					}

					// Phase noise Error
					phaseNoiseDegRmsAll[i][avgIteration-1] = ::LP_GetScalarMeasurement("phaseNoiseDegRmsAll", i*(g_Tester_Number+1)); 

					// Frequency Error
					freqErrorKhz[i][avgIteration-1] = ::LP_GetScalarMeasurement("freqErrorHz", i*(g_Tester_Number+1)); 
					freqErrorKhz[i][avgIteration-1] = freqErrorKhz[i][avgIteration-1] / 1000.0;	// kHz

					// Symbol clock Error (ppm)
					symClockErrorPpm[i][avgIteration-1] = ::LP_GetScalarMeasurement("symClockErrorPpm", i*(g_Tester_Number+1)); 

					// IQ Amplitude Error
					iqImbalAmpDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("iqImbalAmplDb", i*(g_Tester_Number+1)); 

					// IQ Phase Error.
					iqImbalPhaseDeg[i][avgIteration-1] = ::LP_GetScalarMeasurement("iqImbalPhaseDeg", i*(g_Tester_Number+1));
					
					// IQ Phase Error.
					dcLeakageDbc[i][avgIteration-1] = ::LP_GetScalarMeasurement("dcLeakageDbc", i);
				}		
				
			}                
#pragma endregion
		}   // End - avgIteration


#pragma region Averaging and Saving Test Result
	   /*----------------------------------*
		* Averaging and Saving Test Result *
		*----------------------------------*/
		if ( (ERR_OK==err) && captureOK && analysisOK )
		{
			for(int i=0;i<g_Tester_Number;i++)
			{
				// Average EVM test result
				::AverageTestResult(&evmAvgAll[i][0], avgIteration, LOG_20, l_txVerifyEvmReturn.EVM_AVG_ALL_DB[i], l_txVerifyEvmReturn.EVM_AVG_ALL_MAX_DB[i], l_txVerifyEvmReturn.EVM_AVG_ALL_MIN_DB[i]);

				// Average Power test result
				::AverageTestResult(&avgPowerNoGapDbm[i][0], avgIteration, LOG_10, l_txVerifyEvmReturn.AVG_POWER_NO_GAP_DBM[i], l_txVerifyEvmReturn.AVG_POWER_NO_GAP_MAX_DBM[i], l_txVerifyEvmReturn.AVG_POWER_NO_GAP_MIN_DBM[i]);

				// Average Phase noise Error result
				::AverageTestResult(&phaseNoiseDegRmsAll[i][0], avgIteration, Linear, l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL[i], l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL_MAX[i], l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL_MIN[i]);

				// Average Frequency Error result
				::AverageTestResult(&freqErrorKhz[i][0], avgIteration, Linear, l_txVerifyEvmReturn.FREQ_ERROR_KHZ[i], l_txVerifyEvmReturn.FREQ_ERROR_MAX_KHZ[i], l_txVerifyEvmReturn.FREQ_ERROR_MIN_KHZ[i]);

				// Average Symbol clock Error (ppm) result
				::AverageTestResult(&symClockErrorPpm[i][0], avgIteration, Linear, l_txVerifyEvmReturn.SYM_CLOCK_ERROR_PPM[i], l_txVerifyEvmReturn.SYM_CLOCK_ERROR_MAX_PPM[i], l_txVerifyEvmReturn.SYM_CLOCK_ERROR_MIN_PPM[i]);

				// Average IQ Amplitude Error result
				::AverageTestResult(&iqImbalAmpDb[i][0], avgIteration, LOG_10, l_txVerifyEvmReturn.IQIMBAL_AMP_DB[i], l_txVerifyEvmReturn.IQIMBAL_AMP_MAX_DB[i], l_txVerifyEvmReturn.IQIMBAL_AMP_MIN_DB[i]);

				// Average IQ Phase Error result
				::AverageTestResult(&iqImbalPhaseDeg[i][0], avgIteration, Linear, l_txVerifyEvmReturn.IQIMBAL_PHASE_DEG[i], l_txVerifyEvmReturn.IQIMBAL_PHASE_MAX_DEG[i], l_txVerifyEvmReturn.IQIMBAL_PHASE_MIN_DEG[i]);
					
				// Average IQ Phase Error result
				::AverageTestResult(&dcLeakageDbc[i][0], avgIteration, LOG_10, l_txVerifyEvmReturn.DC_LEAKAGE_DBC[i], l_txVerifyEvmReturn.DC_LEAKAGE_MAX_DBC[i], l_txVerifyEvmReturn.DC_LEAKAGE_MIN_DBC[i]);

			}
		}
		else
		{
			// do nothing
		}
#pragma endregion 

		/*----------------------------*
  		 * Disable VSG output signal  *
		 *----------------------------*/
		//if ( 0==strcmp(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE,"LINK") )
		//{
		//	// make sure no signal is generated by the VSG
		//	err = ::LP_EnableVsgRF(0);
		//	if ( ERR_OK!=err )
		//	{
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
		//		throw logMessage;
		//	}
		//	else
		//	{
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
		//	}
		//}

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
			sprintf_s(l_txVerifyEvmReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txVerifyEvmReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txVerifyEvmReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
		err = -1;
    }


	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	ReturnTestResults(l_txVerifyEvmReturnMap);

	// Free memory
	evmAvgAll.clear();
	avgPowerNoGapDbm.clear();
	phaseNoiseDegRmsAll.clear();
	freqErrorKhz.clear();
	symClockErrorPpm.clear();
	iqImbalAmpDb.clear();
	iqImbalPhaseDeg.clear();
	dcLeakageDbc.clear();



	return err;
}

void InitializeTXVerifyEvmContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txVerifyEvmParamMap.clear();

    WIMAX_SETTING_STRUCT setting;

    l_txVerifyEvmParam.FREQ_MHZ = 3500;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("FREQ_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.SIG_TYPE = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.SIG_TYPE))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.SIG_TYPE;
        setting.unit        = "";
        setting.helpText    = "Indicates the type of signal. 1=downlink signal, 2=uplink signal, 0=auto-detect\r\nDefault: 0.";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SIG_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.BANDWIDTH_MHZ = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEvmParam.BANDWIDTH_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.BANDWIDTH_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20\r\nDefault: 0.";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("BANDWIDTH_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.CYCLIC_PREFIX = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEvmParam.CYCLIC_PREFIX))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.CYCLIC_PREFIX;
        setting.unit        = "";
        setting.helpText    = "Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32\r\nDefault: 0.";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CYCLIC_PREFIX", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.RATE_ID = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.RATE_ID))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.RATE_ID;
        setting.unit        = "";
        setting.helpText    = "Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and  correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively\r\nDefault: 0.";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RATE_ID", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.NUM_SYMBOLS = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEvmParam.NUM_SYMBOLS))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.NUM_SYMBOLS;
        setting.unit        = "";
        setting.helpText    = "Number of OFDM symbols in burst. Default: 0 (auto detect).";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("NUM_SYMBOLS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.SAMPLING_TIME_US = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEvmParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyEvmParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyEvmParam.TX_POWER_DBM = 15.0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEvmParam.TX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.TX1 = 1;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX1))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.TX1;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.TX2 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX2))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.TX2;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.TX3 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX3))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.TX3;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.TX4 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX4))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.TX4;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyEvmParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
	strcpy_s(l_txVerifyEvmParam.MAP_CONFIG_FILE_NAME, MAX_BUFFER_SIZE, "AUTO");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.MAP_CONFIG_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyEvmParam.MAP_CONFIG_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "For 802.16e analysis, a map configuration file (*.mcf) is required for non-auto-detect operation.\r\nDefault value is AUTO";
        l_txVerifyEvmParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("MAP_CONFIG_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txVerifyEvmParam.WAVEFORM_FILE_NAME, MAX_BUFFER_SIZE, "AUTO");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.WAVEFORM_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyEvmParam.WAVEFORM_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "For 802.16e, sometime VSG needs a specific mod file to synchronize the test device, if the WIMAX_TEST_MODE=LINK. The value can be AUTO(using GLOBAL_SETTINGS->WIMAX_WAVEFORM_NAME) or NULL(or empty). Default: AUTO.";
        l_txVerifyEvmParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("WAVEFORM_FILE_NAME", setting) );
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
    l_txVerifyEvmReturnMap.clear();

	// EVM Test Result 
    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
		l_txVerifyEvmReturn.EVM_AVG_ALL_MAX_DB[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_AVG_ALL_MAX_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.EVM_AVG_ALL_MAX_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "EVM_AVG_ALL_MAX_DB_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "(MAX) EVM test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
		l_txVerifyEvmReturn.EVM_AVG_ALL_MIN_DB[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_AVG_ALL_MIN_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.EVM_AVG_ALL_MIN_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "EVM_AVG_ALL_MIN_DB_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "(MIN) EVM test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.EVM_AVG_ALL_DB[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_AVG_ALL_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.EVM_AVG_ALL_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "EVM_AVG_ALL_DB_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "(Average) EVM test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }
  
	// POWER Test Result 
    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
		l_txVerifyEvmReturn.AVG_POWER_NO_GAP_MAX_DBM[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.AVG_POWER_NO_GAP_MAX_DBM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.AVG_POWER_NO_GAP_MAX_DBM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "AVG_POWER_NO_GAP_MAX_DBM_%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(MAX) POWER test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.AVG_POWER_NO_GAP_MIN_DBM[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.AVG_POWER_NO_GAP_MIN_DBM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.AVG_POWER_NO_GAP_MIN_DBM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "AVG_POWER_NO_GAP_MIN_DBM_%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(MIN) POWER test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
	}

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.AVG_POWER_NO_GAP_DBM[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.AVG_POWER_NO_GAP_DBM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.AVG_POWER_NO_GAP_DBM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "AVG_POWER_NO_GAP_DBM_%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(Average) POWER test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
	}

    // Phase Noise Test Result 
    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
		l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL_MAX[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL_MAX[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL_MAX[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "PHASE_NOISE_DEG_RMS_ALL_MAX_%d", i+1);
            setting.unit        = "degree";
            setting.helpText    = "(MAX) Phase Noise test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL_MIN[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL_MIN[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL_MIN[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "PHASE_NOISE_DEG_RMS_ALL_MIN_%d", i+1);
            setting.unit        = "degree";
            setting.helpText    = "(MIN) Phase Noise test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.PHASE_NOISE_DEG_RMS_ALL[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "PHASE_NOISE_DEG_RMS_ALL_%d", i+1);
            setting.unit        = "degree";
            setting.helpText    = "(Average) Phase Noise test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    // Frequency Error Test Result 
    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.FREQ_ERROR_MAX_KHZ[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.FREQ_ERROR_MAX_KHZ[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.FREQ_ERROR_MAX_KHZ[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "FREQ_ERROR_MAX_KHZ_%d", i+1);
            setting.unit        = "kHz";
            setting.helpText    = "(MAX) Frequency Error test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.FREQ_ERROR_MIN_KHZ[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.FREQ_ERROR_MIN_KHZ[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.FREQ_ERROR_MIN_KHZ[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "FREQ_ERROR_MIN_KHZ_%d", i+1);
            setting.unit        = "kHz";
            setting.helpText    = "(MIN) Frequency Error test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.FREQ_ERROR_KHZ[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.FREQ_ERROR_KHZ[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.FREQ_ERROR_KHZ[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "FREQ_ERROR_KHZ_%d", i+1);
            setting.unit        = "kHz";
            setting.helpText    = "(Average) Frequency Error test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }


    // Symbol Clock Error Test Result 
    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
		l_txVerifyEvmReturn.SYM_CLOCK_ERROR_MAX_PPM[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.SYM_CLOCK_ERROR_MAX_PPM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.SYM_CLOCK_ERROR_MAX_PPM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "SYM_CLOCK_ERROR_MAX_PPM_%d", i+1);
            setting.unit        = "ppm";
            setting.helpText    = "(MAX) Symbol Clock Error test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.SYM_CLOCK_ERROR_MIN_PPM[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.SYM_CLOCK_ERROR_MIN_PPM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.SYM_CLOCK_ERROR_MIN_PPM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "SYM_CLOCK_ERROR_MIN_PPM_%d", i+1);
            setting.unit        = "ppm";
            setting.helpText    = "(MIN) Symbol Clock Error test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.SYM_CLOCK_ERROR_PPM[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.SYM_CLOCK_ERROR_PPM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.SYM_CLOCK_ERROR_PPM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "SYM_CLOCK_ERROR_PPM_%d", i+1);
            setting.unit        = "ppm";
            setting.helpText    = "(Average) Symbol Clock Error test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    // IQ Amplitude Imbalance Test Result 
    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
		l_txVerifyEvmReturn.IQIMBAL_AMP_MAX_DB[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.IQIMBAL_AMP_MAX_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.IQIMBAL_AMP_MAX_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "IQIMBAL_AMP_MAX_DB_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "(MAX) IQ Amplitude Imbalance test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.IQIMBAL_AMP_MIN_DB[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.IQIMBAL_AMP_MIN_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.IQIMBAL_AMP_MIN_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "IQIMBAL_AMP_MIN_DB_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "(MIN) IQ Amplitude Imbalance test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.IQIMBAL_AMP_DB[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.IQIMBAL_AMP_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.IQIMBAL_AMP_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "IQIMBAL_AMP_DB_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "(Average) IQ Amplitude Imbalance test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    // IQ Phase Imbalance Test Result 	
    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
		l_txVerifyEvmReturn.IQIMBAL_PHASE_MAX_DEG[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.IQIMBAL_PHASE_MAX_DEG[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.IQIMBAL_PHASE_MAX_DEG[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "IQIMBAL_PHASE_MAX_DEG_%d", i+1);
            setting.unit        = "degree";
            setting.helpText    = "(MAX) IQ Phase Imbalance test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.IQIMBAL_PHASE_MIN_DEG[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.IQIMBAL_PHASE_MIN_DEG[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.IQIMBAL_PHASE_MIN_DEG[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "IQIMBAL_PHASE_MIN_DEG_%d", i+1);
            setting.unit        = "degree";
            setting.helpText    = "(MIN) IQ Phase Imbalance test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.IQIMBAL_PHASE_DEG[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.IQIMBAL_PHASE_DEG[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.IQIMBAL_PHASE_DEG[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "IQIMBAL_PHASE_DEG_%d", i+1);
            setting.unit        = "degree";
            setting.helpText    = "(Average) IQ Phase Imbalance test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    // DC Leakage Test Result 
    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.DC_LEAKAGE_MAX_DBC[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.DC_LEAKAGE_MAX_DBC[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.DC_LEAKAGE_MAX_DBC[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "DC_LEAKAGE_MAX_DBC_%d", i+1);
            setting.unit        = "dBc";
            setting.helpText    = "(MAX) DC Leakage test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.DC_LEAKAGE_MIN_DBC[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.DC_LEAKAGE_MIN_DBC[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.DC_LEAKAGE_MIN_DBC[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "DC_LEAKAGE_MIN_DBC_%d", i+1);
            setting.unit        = "dBc";
            setting.helpText    = "(MIN) DC Leakage test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.DC_LEAKAGE_DBC[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.DC_LEAKAGE_DBC[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.DC_LEAKAGE_DBC[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "DC_LEAKAGE_DBC_%d", i+1);
            setting.unit        = "dBc";
            setting.helpText    = "(Average) DC Leakage test result on specific data stream.";
            l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

	// Path Loss
	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyEvmReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEvmReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_txVerifyEvmReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
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
int CheckTxEvmParameters( double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Check path loss (by ant and freq)
		err = CheckPathLossTableExt( g_WIMAX_Test_ID,
								  l_txVerifyEvmParam.FREQ_MHZ,
								  l_txVerifyEvmParam.TX1,
								  l_txVerifyEvmParam.TX2,
								  l_txVerifyEvmParam.TX3,
								  l_txVerifyEvmParam.TX4,
								  l_txVerifyEvmParam.CABLE_LOSS_DB,
								  l_txVerifyEvmReturn.CABLE_LOSS_DB,
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
		if (0==l_txVerifyEvmParam.SAMPLING_TIME_US)
		{
			*samplingTimeUs = g_WIMAXGlobalSettingParam.EVM_SAMPLE_INTERVAL_US;
		}
		else	// SAMPLING_TIME_US != 0
		{
			*samplingTimeUs = l_txVerifyEvmParam.SAMPLING_TIME_US;
		}
		
		// Peak to Average Ratio
		*peakToAvgRatio = g_WIMAXGlobalSettingParam.IQ_P_TO_A_WIMAX;  

		// Check Dut configuration changed or not
		if (  g_WIMAXGlobalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.			  
			  l_txVerifyEvmParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
			  l_txVerifyEvmParam.SIG_TYPE!=g_RecordedParam.SIG_TYPE ||
			  l_txVerifyEvmParam.BANDWIDTH_MHZ!=g_RecordedParam.BANDWIDTH_MHZ ||
			  l_txVerifyEvmParam.CYCLIC_PREFIX!=g_RecordedParam.CYCLIC_PREFIX ||
			  l_txVerifyEvmParam.RATE_ID!=g_RecordedParam.RATE_ID ||
			  l_txVerifyEvmParam.NUM_SYMBOLS!=g_RecordedParam.NUM_SYMBOLS ||
			  l_txVerifyEvmParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
			  l_txVerifyEvmParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_txVerifyEvmParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_txVerifyEvmParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_txVerifyEvmParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  0!=strcmp(l_txVerifyEvmParam.MAP_CONFIG_FILE_NAME, g_RecordedParam.MAP_CONFIG_FILE_NAME) ||
			  l_txVerifyEvmParam.TX1!=g_RecordedParam.ANT1 ||
			  l_txVerifyEvmParam.TX2!=g_RecordedParam.ANT2 ||
			  l_txVerifyEvmParam.TX3!=g_RecordedParam.ANT3 ||
			  l_txVerifyEvmParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;			
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup

		g_RecordedParam.FREQ_MHZ				= l_txVerifyEvmParam.FREQ_MHZ;	
		g_RecordedParam.POWER_DBM				= l_txVerifyEvmParam.TX_POWER_DBM;	
	    g_RecordedParam.SIG_TYPE				= l_txVerifyEvmParam.SIG_TYPE;
	    g_RecordedParam.BANDWIDTH_MHZ			= l_txVerifyEvmParam.BANDWIDTH_MHZ;
	    g_RecordedParam.CYCLIC_PREFIX			= l_txVerifyEvmParam.CYCLIC_PREFIX;
	    g_RecordedParam.RATE_ID					= l_txVerifyEvmParam.RATE_ID;
	    g_RecordedParam.NUM_SYMBOLS				= l_txVerifyEvmParam.NUM_SYMBOLS;
		g_RecordedParam.ANT1					= l_txVerifyEvmParam.TX1;
		g_RecordedParam.ANT2					= l_txVerifyEvmParam.TX2;
		g_RecordedParam.ANT3					= l_txVerifyEvmParam.TX3;
		g_RecordedParam.ANT4					= l_txVerifyEvmParam.TX4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txVerifyEvmParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txVerifyEvmParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txVerifyEvmParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txVerifyEvmParam.CABLE_LOSS_DB[3];	
		sprintf_s(g_RecordedParam.MAP_CONFIG_FILE_NAME, MAX_BUFFER_SIZE, l_txVerifyEvmParam.MAP_CONFIG_FILE_NAME);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] CheckTxEvmParameters() Confirmed.\n");
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

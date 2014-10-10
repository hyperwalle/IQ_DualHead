#include "stdafx.h"
#include "TestManager.h"
#include "WIMAX_Test.h"
#include "WIMAX_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "math.h"

#define POWER_STEP_FOR_TEST	 3

using namespace std;

// These global variables are declared in WIMAX_Test_Internal.cpp
extern TM_ID				g_WIMAX_Test_ID;    
extern vDUT_ID				g_WIMAX_Dut;
extern int					g_Tester_Type;
extern int					g_Tester_Number;
extern bool					g_vsgContinuousTxMode;
extern bool					g_vDutTxActived;
extern bool					g_vDutRxActived;
extern bool					g_dutConfigChanged;
extern WIMAX_RECORD_PARAM	g_RecordedParam;
extern char					g_defaultFilePath[MAX_BUFFER_SIZE];

// This global variable is declared in WIMAX_GLOBAL_SETTING.cpp
extern WIMAX_GLOBAL_SETTING g_WIMAXGlobalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIMAX_SETTING_STRUCT> l_txrxVerificationParamMap;

// Return Value Container
map<string, WIMAX_SETTING_STRUCT> l_txrxVerificationReturnMap;

struct tagParam
{
    // Mandatory Parameters
    int    FREQ_MHZ;                                /*! The center frequency (MHz). */  
	int    SIG_TYPE;								/*! Indicates the type of signal. 1=downlink signal, 2=uplink signal, 0=auto-detect\r\nDefault: 0. */
	int    RATE_ID;									/*! Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively\r\nDefault: 0. */
	double BANDWIDTH_MHZ;							/*! Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20\r\nDefault: 0 */
    double CYCLIC_PREFIX;							/*! Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32\r\nDefault: 0. */
	double NUM_SYMBOLS;								/*! Number of OFDM symbols in burst. Default: 0. (auto detect) */
    double HIGH_TX_POWER_DBM;                       /*! The output power to verify EVM. */
	double MID_TX_POWER_DBM;                        /*! The output power to verify EVM. */
    double LOW_TX_POWER_DBM;                        /*! The output power to verify EVM. */
    double TX_CABLE_LOSS_DB[MAX_DATA_STREAM];       /*! The TX_path loss of test system. */
    double RX_CABLE_LOSS_DB[MAX_DATA_STREAM];       /*! The RX path loss of test system. */
	double SAMPLING_TIME_US;                        /*! The sampling time to verify EVM. */
	char   MAP_CONFIG_FILE_NAME[MAX_BUFFER_SIZE];	/*! For 802.16e analysis, a map configuration file (*.mcf) is required for non-auto-detect operation. */
	char   WAVEFORM_FILE_NAME[MAX_BUFFER_SIZE];		/*! For 802.16e analysis, a Waveform name (*.mod) is required for Rx test. If the value equal to AUTO that means using GLOBAL_SETTINGS->WIMAX_WAVEFORM_NAME. */

    // DUT Parameters
    int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on)  */
    int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */ 
    int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */

    int    RX1;                                     /*!< DUT RX1 on/off. Default=1(on)  */
    int    RX2;                                     /*!< DUT RX2 on/off. Default=0(off) */
    int    RX3;                                     /*!< DUT RX3 on/off. Default=0(off) */ 
    int    RX4;                                     /*!< DUT RX4 on/off. Default=0(off) */

	// RX Parameters
	int    ENABLE_RX_PER_TEST;						/*! A flag to enable the RX PER test. Default=1. (ON) */
	int    FRAME_COUNT;                             /*! Number of count to send data packet */
    double RX_POWER_DBM;                            /*! The power level to verify PER. */

} l_txrxVerificationParam;

struct tagReturn
{
    // EVM Test Result 
    double HIGH_EVM_AVG_ALL_DB[MAX_DATA_STREAM];           /*!< (High) EVM test result on specific data stream. */  
    double MID_EVM_AVG_ALL_DB[MAX_DATA_STREAM];            /*!< (Mid)  EVM test result on specific data stream. */  
    double LOW_EVM_AVG_ALL_DB[MAX_DATA_STREAM];            /*!< (Low)  EVM test result on specific data stream. */  

	// POWER Test Result 
    double HIGH_AVG_POWER_NO_GAP_DBM[MAX_DATA_STREAM];     /*!< (High) POWER test result on specific data stream. */
	double MID_AVG_POWER_NO_GAP_DBM[MAX_DATA_STREAM];      /*!< (Mid)  POWER test result on specific data stream. */
    double LOW_AVG_POWER_NO_GAP_DBM[MAX_DATA_STREAM];      /*!< (Low)  POWER test result on specific data stream. */

    // Phase Noise Test Result 
    double HIGH_PHASE_NOISE_DEG_RMS_ALL[MAX_DATA_STREAM];   /*!< (High) Phase Noise test result on specific data stream. */
    double MID_PHASE_NOISE_DEG_RMS_ALL[MAX_DATA_STREAM];    /*!< (Mid)  Phase Noise test result on specific data stream. */
    double LOW_PHASE_NOISE_DEG_RMS_ALL[MAX_DATA_STREAM];    /*!< (Low)  Phase Noise test result on specific data stream. */

	// Frequency Error Test Result 
    double HIGH_FREQ_ERROR_KHZ[MAX_DATA_STREAM];		   /*!< (High) Frequency Error test result on specific data stream. */
    double MID_FREQ_ERROR_KHZ[MAX_DATA_STREAM];			   /*!< (Mid)  Frequency Error test result on specific data stream. */
    double LOW_FREQ_ERROR_KHZ[MAX_DATA_STREAM];			   /*!< (Low)  Frequency Error test result on specific data stream. */

	double TX_CABLE_LOSS_DB[MAX_DATA_STREAM];			   /*! The Tx path loss of test system. */
	double RX_CABLE_LOSS_DB[MAX_DATA_STREAM];			   /*! The Rx path loss of test system. */

	// RX Power Level
	double RX_POWER_LEVEL;								/*!< POWER_LEVEL dBm for the PER test. Format: POWER LEVEL */
	// PER Test Result 
	double PER;											/*!< PER test result. */
	// RSSI Test Result 
	double RSSI[MAX_DATA_STREAM];						/*!< RSSI test result on specific data stream. Format: RSSI[SpecificStream] */
	// PER Test Result
	double CINR;										/*!< The test result of carrier to interference noise ratio. */

    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txrxVerificationReturn;

void ClearTxRxVerificationReturn(void)
{
	l_txrxVerificationParamMap.clear();
	l_txrxVerificationReturnMap.clear();
}

#pragma endregion

// These global variables/functions only for WIMAX_TX_Verify_Evm.cpp
int CheckParameters( double *samplingTimeUs, double *peakToAvgRatio, char* errorMsg );


//! WIMAX TX RX verification
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


WIMAX_TEST_API int WIMAX_TX_RX_Verification(void)
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
	ClearReturnParameters(l_txrxVerificationReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_txrxVerificationParamMap);
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
        RespondToQueryReturn(l_txrxVerificationReturnMap);
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

	// Define 3 dimensional vector by "POWER_STEP_FOR_TEST"
	vector< vector< vector<double> > >  evm_3D			(POWER_STEP_FOR_TEST, evmAvgAll);
	vector< vector< vector<double> > >  power_3D		(POWER_STEP_FOR_TEST, avgPowerNoGapDbm);
	vector< vector< vector<double> > >  phaseNoise_3D	(POWER_STEP_FOR_TEST, phaseNoiseDegRmsAll);
	vector< vector< vector<double> > >  freqError_3D	(POWER_STEP_FOR_TEST, freqErrorKhz);
	
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
		err = GetInputParameters(l_txrxVerificationParamMap);
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

		err = CheckParameters( &samplingTimeUs, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Prepare input parameters CheckParameters() return OK.\n");
		}

#pragma endregion

#pragma region SETUP VSG FOR SYNCHRONIZATION
		/*-------------------------------*
		 * SETUP VSG FOR SYNCHRONIZATION *
		 *-------------------------------*/
		if ( 0==strcmp(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE,"LINK") )
		{
			if ( (0==strcmp(l_txrxVerificationParam.WAVEFORM_FILE_NAME,""))||(0==strcmp(l_txrxVerificationParam.WAVEFORM_FILE_NAME,"NULL")) )
			{
				// If WIMAX_TEST_MODE = LINK, but the user won't input the WAVEFORM_FILE_NAME name, then must return an error message.
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Since WIMAX_TEST_MODE=LINK, thus WAVEFORM_FILE_NAME can NOT be empty.\n");
				throw logMessage;
			}
			else
			{
				if ( (g_dutConfigChanged==true)||(g_vDutTxActived==false)||(l_txrxVerificationParam.ENABLE_RX_PER_TEST) )
				{
				   /*---------------------*
					* Load waveform Files *
					*---------------------*/
					char   modFile[MAX_BUFFER_SIZE] = {'\0'};
					if ( 0==strcmp(l_txrxVerificationParam.WAVEFORM_FILE_NAME,"AUTO") )
					{
						sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, g_WIMAXGlobalSettingParam.WIMAX_WAVEFORM_NAME);
					}
					else
					{
						sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, l_txrxVerificationParam.WAVEFORM_FILE_NAME);
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
					double VSG_POWER = 0.0;
					if (l_txrxVerificationParam.ENABLE_RX_PER_TEST)
					{
						// Check path loss (by ant and freq)
						err = CheckPathLossTableExt( g_WIMAX_Test_ID,
												  l_txrxVerificationParam.FREQ_MHZ,
												  l_txrxVerificationParam.RX1,
												  l_txrxVerificationParam.RX2,
												  l_txrxVerificationParam.RX3,
												  l_txrxVerificationParam.RX4,
												  l_txrxVerificationParam.RX_CABLE_LOSS_DB,
												  l_txrxVerificationReturn.RX_CABLE_LOSS_DB,
												  &cableLossDb,
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
						VSG_POWER = l_txrxVerificationParam.RX_POWER_DBM + cableLossDb;
					}
					else
					{
						// Check path loss (by ant and freq)
						err = CheckPathLossTableExt( g_WIMAX_Test_ID,
												  l_txrxVerificationParam.FREQ_MHZ,
												  l_txrxVerificationParam.TX1,
												  l_txrxVerificationParam.TX2,
												  l_txrxVerificationParam.TX3,
												  l_txrxVerificationParam.TX4,
												  l_txrxVerificationParam.TX_CABLE_LOSS_DB,
												  l_txrxVerificationReturn.TX_CABLE_LOSS_DB,
												  &cableLossDb,
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
						VSG_POWER = g_WIMAXGlobalSettingParam.VSG_POWER_FOR_SYNCHRONIZATION_DBM + cableLossDb;
					}
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
						err = ::LP_SetVsg(l_txrxVerificationParam.FREQ_MHZ*1e6, VSG_POWER, PORT_RIGHT);
					}
					else	// VSA => PORT_RIGHT, then VSG must => PORT_LEFT 
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG to PORT_LEFT, since VSA already using PORT_RIGHT.\n");
						err = ::LP_SetVsg(l_txrxVerificationParam.FREQ_MHZ*1e6, VSG_POWER, PORT_LEFT);
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

		if ( l_txrxVerificationParam.ENABLE_RX_PER_TEST )
		{
#pragma region Rx DUT control
			/*---------------------------*
			 * Configure DUT to Receive  *
			 *---------------------------*/
			// Set DUT RF frquency, Rx power, antenna, data rate
			vDUT_ClearParameters(g_WIMAX_Dut);

			if ( 0!=l_txrxVerificationParam.FRAME_COUNT )
			{
				vDUT_AddIntegerParameter(g_WIMAX_Dut, "FRAME_COUNT",	l_txrxVerificationParam.FRAME_COUNT );  
			}
			else
			{
				vDUT_AddIntegerParameter(g_WIMAX_Dut, "FRAME_COUNT",	g_WIMAXGlobalSettingParam.PER_WIMAX_PACKETS_NUM ); //when zero, means using default global setting value.  
			}
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "FREQ_MHZ",			l_txrxVerificationParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "SIG_TYPE",			l_txrxVerificationParam.SIG_TYPE);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "RATE_ID",			l_txrxVerificationParam.RATE_ID);
		
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX1",			    l_txrxVerificationParam.RX1);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX2",				l_txrxVerificationParam.RX2);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX3",				l_txrxVerificationParam.RX3);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX4",				l_txrxVerificationParam.RX4);
			
			vDUT_AddDoubleParameter (g_WIMAX_Dut, "BANDWIDTH_MHZ",		l_txrxVerificationParam.BANDWIDTH_MHZ);
			vDUT_AddDoubleParameter (g_WIMAX_Dut, "CYCLIC_PREFIX",		l_txrxVerificationParam.CYCLIC_PREFIX);	
			
			vDUT_AddDoubleParameter (g_WIMAX_Dut, "RX_POWER_DBM",		l_txrxVerificationParam.RX_POWER_DBM);
			vDUT_AddStringParameter (g_WIMAX_Dut, "WAVEFORM_FILE_NAME",	l_txrxVerificationParam.WAVEFORM_FILE_NAME);


			if ( (g_vDutTxActived==true)&&(0==strcmp(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE,"NON-LINK")) )
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

			err = vDUT_Run(g_WIMAX_Dut, "RX_SET_BW");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(RX_SET_BW) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(RX_SET_BW) return OK.\n");
			}

			err = vDUT_Run(g_WIMAX_Dut, "RX_SET_DATA_RATE");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(RX_SET_DATA_RATE) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(RX_SET_DATA_RATE) return OK.\n");
			}

			err = vDUT_Run(g_WIMAX_Dut, "RX_SET_ANTENNA");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(RX_SET_ANTENNA) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(RX_SET_ANTENNA) return OK.\n");
			}

			err = vDUT_Run(g_WIMAX_Dut, "RX_SET_FILTER");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(RX_SET_FILTER) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(RX_SET_FILTER) return OK.\n");
			}

#pragma endregion

			if ( 0==strcmp(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE,"LINK") )
			{
				// Since "Link Mode", VSG already transmit the signal for synchronization, thus skip this step and do nothing.
			}
			else
			{
#pragma region Setup LP Tester (RX)

			   /*---------------------*
				* Load waveform Files *
				*---------------------*/
				char   modFile[MAX_BUFFER_SIZE] = {'\0'};
				if ( 0==strcmp(l_txrxVerificationParam.WAVEFORM_FILE_NAME,"AUTO") )
				{
					sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, g_WIMAXGlobalSettingParam.WIMAX_WAVEFORM_NAME);
				}
				else
				{
					sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, l_txrxVerificationParam.WAVEFORM_FILE_NAME);
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

				// Check path loss (by ant and freq)
				err = CheckPathLossTableExt( g_WIMAX_Test_ID,
										  l_txrxVerificationParam.FREQ_MHZ,
										  l_txrxVerificationParam.RX1,
										  l_txrxVerificationParam.RX2,
										  l_txrxVerificationParam.RX3,
										  l_txrxVerificationParam.RX4,
										  l_txrxVerificationParam.RX_CABLE_LOSS_DB,
										  l_txrxVerificationReturn.RX_CABLE_LOSS_DB,
										  &cableLossDb,
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
				
			   /*--------------------*
				* Setup IQTester VSG *
				*--------------------*/
				if( (l_txrxVerificationParam.RX_POWER_DBM+cableLossDb)>g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX )
				{
					//ERR_VSG_POWER_EXCEED_LIMIT
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", l_txrxVerificationParam.RX_POWER_DBM+cableLossDb, g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX);
					throw logMessage;
				}
				else
				{
					// do nothing
				}
			
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG port by g_WIMAXGlobalSettingParam.VSG_PORT=[%d].\n", g_WIMAXGlobalSettingParam.VSG_PORT);
				err = ::LP_SetVsg(l_txrxVerificationParam.FREQ_MHZ*1e6, l_txrxVerificationParam.RX_POWER_DBM+cableLossDb, g_WIMAXGlobalSettingParam.VSG_PORT);
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

#pragma endregion
			}

#pragma region Start RX_TEST
		   /*---------------------*
			*  Do RX pre_RX frst  *
			*---------------------*/
			err = vDUT_Run(g_WIMAX_Dut, "RX_PRE_RX");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(RX_PRE_RX) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(RX_PRE_RX) return OK.\n");
			}

		   /*-------------------------*
			* clear RX packet counter *
			*-------------------------*/
			err = vDUT_Run(g_WIMAX_Dut, "RX_CLEAR_STATS");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(RX_CLEAR_STATS) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(RX_CLEAR_STATS) return OK.\n");
			}

		   /*----------*
			* RX Start *
			*----------*/		
			err = vDUT_Run(g_WIMAX_Dut, "RX_START");		
			if ( ERR_OK!=err )
			{	
				g_vDutRxActived = false;
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(RX_START) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutRxActived = true;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(RX_START) return OK.\n");
			}
			
			// Delay for DUT settle
			if (0!=g_WIMAXGlobalSettingParam.DUT_RX_SETTLE_TIME_MS)
			{
				Sleep(g_WIMAXGlobalSettingParam.DUT_RX_SETTLE_TIME_MS);
			}
			else
			{
				// do nothing
			}
#pragma endregion

		}
		else
		{
			// l_txrxVerificationParam.ENABLE_RX_PER_TEST==0, then do nothing.
		}

		/*----------------------------------*
		 * Start while loop for # PowerStep *
		 *----------------------------------*/
		double dutTxPower[POWER_STEP_FOR_TEST]; 
		dutTxPower[0] = l_txrxVerificationParam.HIGH_TX_POWER_DBM;
		dutTxPower[1] = l_txrxVerificationParam.MID_TX_POWER_DBM;
		dutTxPower[2] = l_txrxVerificationParam.LOW_TX_POWER_DBM;

		for (int stepNum=0;stepNum<POWER_STEP_FOR_TEST;stepNum++)
		{
			if ( NA_NUMBER==dutTxPower[stepNum] )		// "NA_NUMBER" means don't wnat to test it, then must skip it.
			{
				continue;
			}

			/*---------------------------*
			 * Configure DUT to transmit *
			 *---------------------------*/
			// Set DUT RF frquency, tx power, antenna etc.
			// And clear vDut parameters at beginning.
			vDUT_ClearParameters(g_WIMAX_Dut);

			// Fot TX Test
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "FREQ_MHZ",				l_txrxVerificationParam.FREQ_MHZ);	
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "SIG_TYPE",				l_txrxVerificationParam.SIG_TYPE);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "RATE_ID",				l_txrxVerificationParam.RATE_ID);
			vDUT_AddDoubleParameter (g_WIMAX_Dut, "BANDWIDTH_MHZ",			l_txrxVerificationParam.BANDWIDTH_MHZ);
			vDUT_AddDoubleParameter (g_WIMAX_Dut, "CYCLIC_PREFIX",			l_txrxVerificationParam.CYCLIC_PREFIX);
			vDUT_AddDoubleParameter (g_WIMAX_Dut, "NUM_SYMBOLS",			l_txrxVerificationParam.NUM_SYMBOLS);
			vDUT_AddDoubleParameter (g_WIMAX_Dut, "TX_POWER_DBM",			dutTxPower[stepNum]);
			vDUT_AddStringParameter (g_WIMAX_Dut, "MAP_CONFIG_FILE_NAME",	l_txrxVerificationParam.MAP_CONFIG_FILE_NAME);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX1",					l_txrxVerificationParam.TX1);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX2",					l_txrxVerificationParam.TX2);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX3",					l_txrxVerificationParam.TX3);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX4",					l_txrxVerificationParam.TX4);


			if ( (g_dutConfigChanged==true)||(g_vDutTxActived==false) )
			{
				if (l_txrxVerificationParam.ENABLE_RX_PER_TEST==0)	// RX mode is "Off"
				{
					if ( stepNum==0 )		// If stepNum==0, means this is the first run. 
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
						// If stepNum!=0, means this is not the first run. Then we can skip some vDut steps. 
						// do nothing
					}
				}
				else
				{
					// l_txrxVerificationParam.ENABLE_RX_PER_TEST==1, Rx mode is "ON". Then do nothing and skip some TX dut control steps.
					if ( (l_txrxVerificationParam.TX1!=l_txrxVerificationParam.RX1)||
						 (l_txrxVerificationParam.TX2!=l_txrxVerificationParam.RX2)||
						 (l_txrxVerificationParam.TX3!=l_txrxVerificationParam.RX3)||
						 (l_txrxVerificationParam.TX4!=l_txrxVerificationParam.RX4)
						)
					{
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

						// TODO: Do we real need to call "TX_PRE_TX" again?
						//err = vDUT_Run(g_WIMAX_Dut, "TX_PRE_TX");
						//if ( ERR_OK!=err )
						//{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
						//   err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
						//   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
						//   {
						//	   err = -1;	// set err to -1, means "Error".
						//	   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						//	   throw logMessage;
						//   }
						//   else	// Just return normal error message in this case
						//   {
						//	   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(TX_PRE_TX) return error.\n");
						//	   throw logMessage;
						//   }
						//}
						//else
						//{  
						//   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(TX_PRE_TX) return OK.\n");
						//}
					}
					else
					{
						// do nothing
					}

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

			// Check path loss (by ant and freq)
			err = CheckPathLossTableExt( g_WIMAX_Test_ID,
									  l_txrxVerificationParam.FREQ_MHZ,
									  l_txrxVerificationParam.TX1,
									  l_txrxVerificationParam.TX2,
									  l_txrxVerificationParam.TX3,
									  l_txrxVerificationParam.TX4,
									  l_txrxVerificationParam.TX_CABLE_LOSS_DB,
									  l_txrxVerificationReturn.TX_CABLE_LOSS_DB,
									  &cableLossDb,
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

			err = ::LP_SetVsa(  l_txrxVerificationParam.FREQ_MHZ*1e6,
								dutTxPower[stepNum]-cableLossDb+peakToAvgRatio,
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to Quick capture signal at %d MHz.\n", l_txrxVerificationParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Quick LP_VsaDataCapture() at %d MHz return OK.\n", l_txrxVerificationParam.FREQ_MHZ);
				}		

				/*--------------------------------*
				 *  Perform Quick Power Analysis  *
				 *--------------------------------*/
				err = ::LP_AnalyzePower( );
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Analysis_Failed", l_txrxVerificationParam.FREQ_MHZ);
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
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Result_Failed", l_txrxVerificationParam.FREQ_MHZ);
						WIMAXSaveSigFile(sigFileNameBuffer);
						err = -1;				
						sprintf_s(logMessage, MAX_BUFFER_SIZE, "[WIMAX] Quick LP_GetScalarMeasurement(P_peak_all_dBm) return error and save the sig file %s.\n", sigFileNameBuffer);
						return err;
					}
					else
					{			
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Quick power analysis OK, P_peak_all_dBm = %.2f.\n", pkPowerDbm);

						// Check if the power over the range, then needs some delay for power settle down
						double targetPkPower = dutTxPower[stepNum]-cableLossDb+peakToAvgRatio;
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
					err = QuickCaptureRetry( l_txrxVerificationParam.FREQ_MHZ, samplingTimeUs, g_WIMAXGlobalSettingParam.VSA_TRIGGER_TYPE, &quickPower, logMessage);
					if (ERR_OK!=err)	// QuickCaptureRetry() is failed
					{
						// Fail Capture
						if ( quickPower!=NA_NUMBER )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz.\nThe DUT power (without path loss) = %.1f dBm and QuickCaptureRetry() return error.\n", l_txrxVerificationParam.FREQ_MHZ, quickPower);
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz, QuickCaptureRetry() return error.\n", l_txrxVerificationParam.FREQ_MHZ);
						}
						throw logMessage;
					}						
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] The QuickCaptureRetry() at %d MHz return OK.\n", l_txrxVerificationParam.FREQ_MHZ);
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_VsaDataCapture() at %d MHz return OK.\n", l_txrxVerificationParam.FREQ_MHZ);
				}
	#pragma endregion

				/*--------------*
				 *  Capture OK  *
				 *--------------*/
				captureOK = true;

				if (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
				{
					// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WIMAX_TXRX_Test_SaveAlways", l_txrxVerificationParam.FREQ_MHZ);
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

				if (0==l_txrxVerificationParam.SIG_TYPE)		sigType			= -1;
					else										sigType			= l_txrxVerificationParam.SIG_TYPE;	
				if (0==l_txrxVerificationParam.BANDWIDTH_MHZ)	bandwidthMHz	= -1;
					else										bandwidthMHz	= l_txrxVerificationParam.BANDWIDTH_MHZ;
				if (0==l_txrxVerificationParam.CYCLIC_PREFIX)	cyclicPrefix	= -1;
					else										cyclicPrefix	= l_txrxVerificationParam.CYCLIC_PREFIX;
				if (0==l_txrxVerificationParam.RATE_ID)			rateId			= l_txrxVerificationParam.RATE_ID - 1;
					else										rateId			= l_txrxVerificationParam.RATE_ID - 1;
				if (0==l_txrxVerificationParam.NUM_SYMBOLS)		numSymbols		= -1;
					else										numSymbols		= l_txrxVerificationParam.NUM_SYMBOLS;
				

				if ( 0==strcmp(l_txrxVerificationParam.MAP_CONFIG_FILE_NAME, "AUTO") )
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
					sprintf_s(mapFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, l_txrxVerificationParam.MAP_CONFIG_FILE_NAME);
					
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
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WIMAX_TXRX_Test_Analyze80216e_Failed", l_txrxVerificationParam.FREQ_MHZ);
					WIMAXSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] LP_Analyze80216e() return error.\n");					
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_Analyze80216e() return OK.\n");
				}	
	#pragma endregion

	#pragma region Retrieve TX analysis Results
				/*-----------------------------*
				 *  Retrieve analysis Results  *
				 *-----------------------------*/
				char sTestDetail[MAX_BUFFER_SIZE] = {'\0'};
				sprintf_s(sTestDetail, MAX_BUFFER_SIZE, "WIMAX_TXRX_Test_%d", l_txrxVerificationParam.FREQ_MHZ);

				avgIteration++;
				analysisOK = true;

				// Store the result, includes EVM, power etc.              
				if ( g_Tester_Type==IQ_MAX )        // 802.16e, only one tester
				{
					// EVM
					evm_3D[stepNum][0][avgIteration-1] = ::LP_GetScalarMeasurement("evmAvgAll",0); 
					if ( -99.00 >= evm_3D[stepNum][0][avgIteration-1] )
					{
						analysisOK = false;
						evm_3D[stepNum][0][avgIteration-1] = NA_NUMBER;
						WIMAXSaveSigFile(sTestDetail);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(evmAvgAll) return error.\n");
						throw logMessage;
					}
					
					// Power 
					power_3D[stepNum][0][avgIteration-1] = ::LP_GetScalarMeasurement("avgPowerNoGapDb",0);
					if ( -99.00 >= power_3D[stepNum][0][avgIteration-1] )
					{
						analysisOK = false;
						power_3D[stepNum][0][avgIteration-1] = NA_NUMBER;
						WIMAXSaveSigFile(sTestDetail);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(avgPowerNoGapDb) return error.\n");
						throw logMessage;
					}
					else
					{
						// One stream data rate, should be only one antenna is ON	
						int antenaOrder = 0;
						err = CheckAntennaOrderByStream( 1, l_txrxVerificationParam.TX1, l_txrxVerificationParam.TX2, l_txrxVerificationParam.TX3, l_txrxVerificationParam.TX4, &antenaOrder);
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] CheckAntennaOrderByStream() return error.\n");					
							throw logMessage;
						}

						power_3D[stepNum][0][avgIteration-1] = power_3D[stepNum][0][avgIteration-1] + l_txrxVerificationParam.TX_CABLE_LOSS_DB[antenaOrder-1];
					}

					// Phase noise Error
					phaseNoise_3D[stepNum][0][avgIteration-1] = ::LP_GetScalarMeasurement("phaseNoiseDegRmsAll", 0); 

					// Frequency Error
					freqError_3D[stepNum][0][avgIteration-1] = ::LP_GetScalarMeasurement("freqErrorHz", 0); 
					freqError_3D[stepNum][0][avgIteration-1] = freqError_3D[stepNum][0][avgIteration-1] / 1000.0;	// kHz

				}
				else    // 802.16e, more than one stream
				{				
					for(int i=0;i<g_Tester_Number;i++)
					{
						// EVM
						evm_3D[stepNum][i][avgIteration-1] = ::LP_GetScalarMeasurement("evmAvgAll", i); 
						if ( -99.00 >= evm_3D[stepNum][i][avgIteration-1] )
						{
							analysisOK = false;
							evm_3D[stepNum][i][avgIteration-1] = NA_NUMBER;
							WIMAXSaveSigFile(sTestDetail);
							err = -1;
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(evmAvgAll) return error.\n");
							throw logMessage;
						}
						
						// Power 
						power_3D[stepNum][i][avgIteration-1] = ::LP_GetScalarMeasurement("avgPowerNoGapDb", i*(g_Tester_Number+1));
						if ( -99.00 >= power_3D[stepNum][i][avgIteration-1] )
						{
							analysisOK = false;
							power_3D[stepNum][i][avgIteration-1] = NA_NUMBER;
							WIMAXSaveSigFile(sTestDetail);
							err = -1;
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(avgPowerNoGapDb) return error.\n");
							throw logMessage;
						}
						else
						{
							//The current code will work correctly with 4x4 MIMO.  
							int antenaOrder = 0;
							err = CheckAntennaOrderByStream(i+1, l_txrxVerificationParam.TX1, l_txrxVerificationParam.TX2, l_txrxVerificationParam.TX3, l_txrxVerificationParam.TX4, &antenaOrder);
							if ( ERR_OK!=err )
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] CheckAntennaOrderByStream() return error.\n");					
								throw logMessage;
							}

							power_3D[stepNum][i][avgIteration-1] = power_3D[stepNum][i][avgIteration-1] + l_txrxVerificationParam.TX_CABLE_LOSS_DB[antenaOrder-1];
						}

						// Phase noise Error
						phaseNoise_3D[stepNum][i][avgIteration-1] = ::LP_GetScalarMeasurement("phaseNoiseDegRmsAll", i*(g_Tester_Number+1)); 

						// Frequency Error
						freqError_3D[stepNum][i][avgIteration-1] = ::LP_GetScalarMeasurement("freqErrorHz", i*(g_Tester_Number+1)); 
						freqError_3D[stepNum][i][avgIteration-1] = freqError_3D[stepNum][i][avgIteration-1] / 1000.0;	// kHz

					}		
					
				}                
	#pragma endregion
			}   // End - avgIteration
		}


#pragma region Averaging and Saving Test Result
	   /*----------------------------------*
		* Averaging and Saving Test Result *
		*----------------------------------*/
		if ( (ERR_OK==err) && captureOK && analysisOK )
		{
			double dummyMax = 0.0;
			double dummyMin = 0.0;

			for(int i=0;i<g_Tester_Number;i++)
			{
				// Average EVM test result
				::AverageTestResult(&evm_3D[0][i][0], avgIteration, RMS_LOG_20, l_txrxVerificationReturn.HIGH_EVM_AVG_ALL_DB[i], dummyMax, dummyMin);
				::AverageTestResult(&evm_3D[1][i][0], avgIteration, RMS_LOG_20, l_txrxVerificationReturn.MID_EVM_AVG_ALL_DB[i],  dummyMax, dummyMin);
				::AverageTestResult(&evm_3D[2][i][0], avgIteration, RMS_LOG_20, l_txrxVerificationReturn.LOW_EVM_AVG_ALL_DB[i],  dummyMax, dummyMin);

				// Average Power test result
				::AverageTestResult(&power_3D[0][i][0], avgIteration, LOG_10, l_txrxVerificationReturn.HIGH_AVG_POWER_NO_GAP_DBM[i], dummyMax, dummyMin);
				::AverageTestResult(&power_3D[1][i][0], avgIteration, LOG_10, l_txrxVerificationReturn.MID_AVG_POWER_NO_GAP_DBM[i],  dummyMax, dummyMin);
				::AverageTestResult(&power_3D[2][i][0], avgIteration, LOG_10, l_txrxVerificationReturn.LOW_AVG_POWER_NO_GAP_DBM[i],  dummyMax, dummyMin);

				// Average Phase noise Error result
				::AverageTestResult(&phaseNoise_3D[0][i][0], avgIteration, Linear, l_txrxVerificationReturn.HIGH_PHASE_NOISE_DEG_RMS_ALL[i], dummyMax, dummyMin);
				::AverageTestResult(&phaseNoise_3D[1][i][0], avgIteration, Linear, l_txrxVerificationReturn.MID_PHASE_NOISE_DEG_RMS_ALL[i], dummyMax, dummyMin);
				::AverageTestResult(&phaseNoise_3D[2][i][0], avgIteration, Linear, l_txrxVerificationReturn.LOW_PHASE_NOISE_DEG_RMS_ALL[i], dummyMax, dummyMin);

				// Average Frequency Error result
				::AverageTestResult(&freqError_3D[0][i][0], avgIteration, Linear, l_txrxVerificationReturn.HIGH_FREQ_ERROR_KHZ[i], dummyMax, dummyMin);
				::AverageTestResult(&freqError_3D[1][i][0], avgIteration, Linear, l_txrxVerificationReturn.MID_FREQ_ERROR_KHZ[i], dummyMax, dummyMin);
				::AverageTestResult(&freqError_3D[2][i][0], avgIteration, Linear, l_txrxVerificationReturn.LOW_FREQ_ERROR_KHZ[i], dummyMax, dummyMin);
			}
		}
		else
		{
			// do nothing
		}
#pragma endregion 

		if ( l_txrxVerificationParam.ENABLE_RX_PER_TEST )
		{
			/*---------------------------*
			 * Configure DUT to Receive  *
			 *---------------------------*/
			// Set DUT RF frquency, Rx power, antenna, data rate
			vDUT_ClearParameters(g_WIMAX_Dut);

			if ( 0!=l_txrxVerificationParam.FRAME_COUNT )
			{
				vDUT_AddIntegerParameter(g_WIMAX_Dut, "FRAME_COUNT",	l_txrxVerificationParam.FRAME_COUNT );  
			}
			else
			{
				vDUT_AddIntegerParameter(g_WIMAX_Dut, "FRAME_COUNT",	g_WIMAXGlobalSettingParam.PER_WIMAX_PACKETS_NUM ); //when zero, means using default global setting value.  
			}
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "FREQ_MHZ",			l_txrxVerificationParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "SIG_TYPE",			l_txrxVerificationParam.SIG_TYPE);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "RATE_ID",			l_txrxVerificationParam.RATE_ID);
		
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX1",			    l_txrxVerificationParam.RX1);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX2",				l_txrxVerificationParam.RX2);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX3",				l_txrxVerificationParam.RX3);
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX4",				l_txrxVerificationParam.RX4);
			
			vDUT_AddDoubleParameter (g_WIMAX_Dut, "BANDWIDTH_MHZ",		l_txrxVerificationParam.BANDWIDTH_MHZ);
			vDUT_AddDoubleParameter (g_WIMAX_Dut, "CYCLIC_PREFIX",		l_txrxVerificationParam.CYCLIC_PREFIX);				
			vDUT_AddDoubleParameter (g_WIMAX_Dut, "RX_POWER_DBM",		l_txrxVerificationParam.RX_POWER_DBM);

#pragma region Retrieve Rx analysis Results
		   /*--------------------*
			* Get RX PER Result  *
			*--------------------*/
			int totalPackets = 0;
			int goodPackets  = 0;
			int badPackets   = 0;

			err = ::vDUT_Run(g_WIMAX_Dut, "RX_GET_STATS");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(RX_GET_STATS) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(RX_GET_STATS) return OK.\n");
			}

			l_txrxVerificationReturn.RX_POWER_LEVEL = l_txrxVerificationParam.RX_POWER_DBM;

			err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "GOOD_PACKETS", &goodPackets);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_GetIntegerReturn(GOOD_PACKETS) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetIntegerReturn(GOOD_PACKETS) return OK.\n");
			}
			if ( goodPackets<0 )
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Dut report GOOD_PACKETS less than 0.\n");
				throw logMessage;
			}
			else
			{
				//l_txrxVerificationReturn.GOOD_PACKETS = goodPackets;
			}

			err = ::vDUT_GetIntegerReturn(g_WIMAX_Dut, "TOTAL_PACKETS", &totalPackets);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_GetIntegerReturn(TOTAL_PACKETS) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetIntegerReturn(TOTAL_PACKETS) return OK.\n");
			}
			if ( totalPackets<0 )
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Dut report TOTAL_PACKETS less than 0.\n");
				throw logMessage;
			}
			else
			{
				//l_txrxVerificationReturn.TOTAL_PACKETS = totalPackets;
			}

			// Sometime Dut get packets from Air
			// workaround for some dut which returns more ack than packet sent
			if ( goodPackets>totalPackets )	
			{
				goodPackets = totalPackets;
				//l_txrxVerificationReturn.GOOD_PACKETS  = goodPackets;
				//l_txrxVerificationReturn.TOTAL_PACKETS = totalPackets;
			}
			else
			{
				// do nothing	
			}

			badPackets = totalPackets - goodPackets;

			if( totalPackets!=0 )
			{
				l_txrxVerificationReturn.PER = ((double)(badPackets)/(double)(totalPackets)) * 100.0;
			}
			else	// In this case, totalPackets = 0
			{
				l_txrxVerificationReturn.PER = 100.0;
			}

			err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "RSSI_1", &l_txrxVerificationReturn.RSSI[0]); 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the RSSI_1 value.\n");
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(RSSI_1) return OK.\n");
			}

			err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "RSSI_2", &l_txrxVerificationReturn.RSSI[1]); 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the RSSI_2 value.\n");
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(RSSI_2) return OK.\n");
			}

			err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "RSSI_3", &l_txrxVerificationReturn.RSSI[2]); 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the RSSI_3 value.\n");
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(RSSI_3) return OK.\n");
			}

			err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "RSSI_4", &l_txrxVerificationReturn.RSSI[3]); 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the RSSI_4 value.\n");
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(RSSI_4) return OK.\n");
			}

			err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "CINR", &l_txrxVerificationReturn.CINR); 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the CINR value.\n");
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(CINR) return OK.\n");
			}

#pragma endregion

			err = vDUT_Run(g_WIMAX_Dut, "RX_STOP");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(RX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutRxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(RX_STOP) return OK.\n");
			}

		}
		else
		{
			// l_txrxVerificationParam.ENABLE_RX_PER_TEST==0, then skip this step and do nothing.
		}

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
			sprintf_s(l_txrxVerificationReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txrxVerificationReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txrxVerificationReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
		err = -1;
    }

	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	ReturnTestResults(l_txrxVerificationReturnMap);


	// Free memory
	evmAvgAll.clear();
	avgPowerNoGapDbm.clear();
	phaseNoiseDegRmsAll.clear();
	freqErrorKhz.clear();
	evm_3D.clear();
	power_3D.clear();
	phaseNoise_3D.clear();
	freqError_3D.clear();


	return err;
}

void InitializetxrxVerificationContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txrxVerificationParamMap.clear();

    WIMAX_SETTING_STRUCT setting;

    l_txrxVerificationParam.FREQ_MHZ = 3500;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txrxVerificationParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("FREQ_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.SIG_TYPE = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txrxVerificationParam.SIG_TYPE))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.SIG_TYPE;
        setting.unit        = "";
        setting.helpText    = "Indicates the type of signal. 1=downlink signal, 2=uplink signal, 0=auto-detect\r\nDefault: 0.";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SIG_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.BANDWIDTH_MHZ = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txrxVerificationParam.BANDWIDTH_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.BANDWIDTH_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20\r\nDefault: 0.";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("BANDWIDTH_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.CYCLIC_PREFIX = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txrxVerificationParam.CYCLIC_PREFIX))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.CYCLIC_PREFIX;
        setting.unit        = "";
        setting.helpText    = "Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32\r\nDefault: 0.";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CYCLIC_PREFIX", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.RATE_ID = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txrxVerificationParam.RATE_ID))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.RATE_ID;
        setting.unit        = "";
        setting.helpText    = "Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and  correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively\r\nDefault: 0.";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RATE_ID", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.NUM_SYMBOLS = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txrxVerificationParam.NUM_SYMBOLS))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.NUM_SYMBOLS;
        setting.unit        = "";
        setting.helpText    = "Number of OFDM symbols in burst. Default: 0 (auto detect).";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("NUM_SYMBOLS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.SAMPLING_TIME_US = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txrxVerificationParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.HIGH_TX_POWER_DBM = 25.0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txrxVerificationParam.HIGH_TX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.HIGH_TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "(High Power) Expected power level at DUT antenna port.";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("HIGH_TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.MID_TX_POWER_DBM = 0.0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txrxVerificationParam.MID_TX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.MID_TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "(Mid Power) Expected power level at DUT antenna port.";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("MID_TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.LOW_TX_POWER_DBM = -20.0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txrxVerificationParam.LOW_TX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.LOW_TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "(Low Power) Expected power level at DUT antenna port.";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("LOW_TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.TX1 = 1;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txrxVerificationParam.TX1))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.TX1;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.TX2 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txrxVerificationParam.TX2))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.TX2;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.TX3 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txrxVerificationParam.TX3))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.TX3;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.TX4 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txrxVerificationParam.TX4))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.TX4;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.RX1 = 1;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txrxVerificationParam.RX1))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.RX1;
        setting.unit        = "";
        setting.helpText    = "DUT RX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.RX2 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txrxVerificationParam.RX2))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.RX2;
        setting.unit        = "";
        setting.helpText    = "DUT RX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.RX3 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txrxVerificationParam.RX3))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.RX3;
        setting.unit        = "";
        setting.helpText    = "DUT RX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txrxVerificationParam.RX4 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txrxVerificationParam.RX4))    // Type_Checking
    {
        setting.value       = (void*)&l_txrxVerificationParam.RX4;
        setting.unit        = "";
        setting.helpText    = "DUT RX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
	strcpy_s(l_txrxVerificationParam.MAP_CONFIG_FILE_NAME, MAX_BUFFER_SIZE, "AUTO");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txrxVerificationParam.MAP_CONFIG_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)l_txrxVerificationParam.MAP_CONFIG_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "For 802.16e analysis, a map configuration file (*.mcf) is required for non-auto-detect operation.\r\nDefault value is AUTO";
        l_txrxVerificationParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("MAP_CONFIG_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txrxVerificationParam.WAVEFORM_FILE_NAME, MAX_BUFFER_SIZE, "AUTO");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txrxVerificationParam.WAVEFORM_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)l_txrxVerificationParam.WAVEFORM_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "For 802.16e, sometime VSG needs a specific mod file to synchronize the test device, if the WIMAX_TEST_MODE=LINK. The value can be AUTO(using GLOBAL_SETTINGS->WIMAX_WAVEFORM_NAME) or NULL(or empty). Default: AUTO.";
        l_txrxVerificationParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("WAVEFORM_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txrxVerificationParam.TX_CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txrxVerificationParam.TX_CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txrxVerificationParam.TX_CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "TX Cable loss from the DUT antenna port to tester";
			l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txrxVerificationParam.RX_CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txrxVerificationParam.RX_CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txrxVerificationParam.RX_CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "RX_CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "RX Cable loss from the DUT antenna port to tester";
			l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	//---------------//
	// RX parameters //
	//---------------//	
	l_txrxVerificationParam.ENABLE_RX_PER_TEST = 1;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txrxVerificationParam.ENABLE_RX_PER_TEST))    // Type_Checking
	{
		setting.value = (void*)&l_txrxVerificationParam.ENABLE_RX_PER_TEST;
		setting.unit        = "";
		setting.helpText    = "A flag to enable the RX PER test. Default=1. (means ON).";
		l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ENABLE_RX_PER_TEST", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txrxVerificationParam.FRAME_COUNT = 0;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txrxVerificationParam.FRAME_COUNT))    // Type_Checking
	{
		setting.value = (void*)&l_txrxVerificationParam.FRAME_COUNT;
		setting.unit        = "";
		setting.helpText    = "Number of frame count, default=0, means using default global setting value.";
		l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("FRAME_COUNT", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txrxVerificationParam.RX_POWER_DBM = -65.0;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txrxVerificationParam.RX_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_txrxVerificationParam.RX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port";
		l_txrxVerificationParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX_POWER_DBM", setting) );
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
    l_txrxVerificationReturnMap.clear();

	// EVM Test Result 
    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txrxVerificationReturn.HIGH_EVM_AVG_ALL_DB[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.HIGH_EVM_AVG_ALL_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.HIGH_EVM_AVG_ALL_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_HIGH_EVM_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "(High Power) EVM test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txrxVerificationReturn.MID_EVM_AVG_ALL_DB[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.MID_EVM_AVG_ALL_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.MID_EVM_AVG_ALL_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_MID_EVM_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "(Mid Power) EVM test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txrxVerificationReturn.LOW_EVM_AVG_ALL_DB[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.LOW_EVM_AVG_ALL_DB[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.LOW_EVM_AVG_ALL_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_LOW_EVM_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "(Low Power) EVM test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
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
        l_txrxVerificationReturn.HIGH_AVG_POWER_NO_GAP_DBM[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.HIGH_AVG_POWER_NO_GAP_DBM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.HIGH_AVG_POWER_NO_GAP_DBM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_HIGH_POWER_%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(High Power) POWER test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
	}

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txrxVerificationReturn.MID_AVG_POWER_NO_GAP_DBM[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.MID_AVG_POWER_NO_GAP_DBM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.MID_AVG_POWER_NO_GAP_DBM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_MID_POWER_%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(Mid Power) POWER test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
	}

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txrxVerificationReturn.LOW_AVG_POWER_NO_GAP_DBM[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.LOW_AVG_POWER_NO_GAP_DBM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.LOW_AVG_POWER_NO_GAP_DBM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_LOW_POWER_%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(Low Power) POWER test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
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
        l_txrxVerificationReturn.HIGH_PHASE_NOISE_DEG_RMS_ALL[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.HIGH_PHASE_NOISE_DEG_RMS_ALL[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.HIGH_PHASE_NOISE_DEG_RMS_ALL[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_HIGH_PHASE_NOISE_%d", i+1);
            setting.unit        = "degree";
            setting.helpText    = "(High Power) Phase Noise test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txrxVerificationReturn.MID_PHASE_NOISE_DEG_RMS_ALL[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.MID_PHASE_NOISE_DEG_RMS_ALL[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.MID_PHASE_NOISE_DEG_RMS_ALL[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_MID_PHASE_NOISE_%d", i+1);
            setting.unit        = "degree";
            setting.helpText    = "(Mid Power) Phase Noise test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txrxVerificationReturn.LOW_PHASE_NOISE_DEG_RMS_ALL[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.LOW_PHASE_NOISE_DEG_RMS_ALL[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.LOW_PHASE_NOISE_DEG_RMS_ALL[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_LOW_PHASE_NOISE_%d", i+1);
            setting.unit        = "degree";
            setting.helpText    = "(Low Power) Phase Noise test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
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
        l_txrxVerificationReturn.HIGH_FREQ_ERROR_KHZ[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.HIGH_FREQ_ERROR_KHZ[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.HIGH_FREQ_ERROR_KHZ[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_HIGH_FREQ_ERROR_%d", i+1);
            setting.unit        = "kHz";
            setting.helpText    = "(High Power) Frequency Error test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txrxVerificationReturn.MID_FREQ_ERROR_KHZ[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.MID_FREQ_ERROR_KHZ[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.MID_FREQ_ERROR_KHZ[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_MID_FREQ_ERROR_%d", i+1);
            setting.unit        = "kHz";
            setting.helpText    = "(Mid Power) Frequency Error test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txrxVerificationReturn.LOW_FREQ_ERROR_KHZ[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txrxVerificationReturn.LOW_FREQ_ERROR_KHZ[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txrxVerificationReturn.LOW_FREQ_ERROR_KHZ[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TX_LOW_FREQ_ERROR_%d", i+1);
            setting.unit        = "kHz";
            setting.helpText    = "(Low Power) Frequency Error test result on specific data stream.";
            l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
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
		l_txrxVerificationReturn.TX_CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txrxVerificationReturn.TX_CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txrxVerificationReturn.TX_CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "TX_CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "TX Cable loss from the DUT antenna port to tester";
			l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
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
		l_txrxVerificationReturn.RX_CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txrxVerificationReturn.RX_CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txrxVerificationReturn.RX_CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "RX_CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "RX Cable loss from the DUT antenna port to tester";
			l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}


	//---------------//
	//   RX Return   //
	//---------------//	
	l_txrxVerificationReturn.PER = NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txrxVerificationReturn.PER))    // Type_Checking
	{
		setting.value = (void*)&l_txrxVerificationReturn.PER;
		setting.unit        = "%";
		setting.helpText    = "Average PER over received packets.";
		l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX_PER", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txrxVerificationReturn.RSSI[i] = NA_NUMBER;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txrxVerificationReturn.RSSI[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txrxVerificationReturn.RSSI[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "RX_RSSI_%d", i+1);
			setting.unit        = "";
			setting.helpText    = "Average RSSI over received packets.";
			l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_txrxVerificationReturn.CINR = NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txrxVerificationReturn.CINR))    // Type_Checking
	{
		setting.value = (void*)&l_txrxVerificationReturn.CINR;
		setting.unit        = "dB";
		setting.helpText    = "The test result of carrier to interference noise ratio.";
		l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX_CINR", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txrxVerificationReturn.RX_POWER_LEVEL = NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txrxVerificationReturn.RX_POWER_LEVEL))    // Type_Checking
	{
		setting.value = (void*)&l_txrxVerificationReturn.RX_POWER_LEVEL;
		setting.unit        = "dBm";
		setting.helpText    = "RX power level for PER";
		l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX_POWER_LEVEL", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_txrxVerificationReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txrxVerificationReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txrxVerificationReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_txrxVerificationReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
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
int CheckParameters( double *samplingTimeUs, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Check sampling time 
		if (0==l_txrxVerificationParam.SAMPLING_TIME_US)
		{
			*samplingTimeUs = g_WIMAXGlobalSettingParam.EVM_SAMPLE_INTERVAL_US;
		}
		else	// SAMPLING_TIME_US != 0
		{
			*samplingTimeUs = l_txrxVerificationParam.SAMPLING_TIME_US;
		}
		
		// Peak to Average Ratio
		*peakToAvgRatio = g_WIMAXGlobalSettingParam.IQ_P_TO_A_WIMAX;  

		// Check Dut configuration changed or not
		if (  g_WIMAXGlobalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.			  
			  l_txrxVerificationParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
			  l_txrxVerificationParam.SIG_TYPE!=g_RecordedParam.SIG_TYPE ||
			  l_txrxVerificationParam.BANDWIDTH_MHZ!=g_RecordedParam.BANDWIDTH_MHZ ||
			  l_txrxVerificationParam.CYCLIC_PREFIX!=g_RecordedParam.CYCLIC_PREFIX ||
			  l_txrxVerificationParam.RATE_ID!=g_RecordedParam.RATE_ID ||
			  l_txrxVerificationParam.NUM_SYMBOLS!=g_RecordedParam.NUM_SYMBOLS ||
			  l_txrxVerificationParam.HIGH_TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||	    // TODO: note that must compare High Power
			  l_txrxVerificationParam.TX_CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_txrxVerificationParam.TX_CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_txrxVerificationParam.TX_CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_txrxVerificationParam.TX_CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  0!=strcmp(l_txrxVerificationParam.MAP_CONFIG_FILE_NAME, g_RecordedParam.MAP_CONFIG_FILE_NAME) ||
			  l_txrxVerificationParam.TX1!=g_RecordedParam.ANT1 ||
			  l_txrxVerificationParam.TX2!=g_RecordedParam.ANT2 ||
			  l_txrxVerificationParam.TX3!=g_RecordedParam.ANT3 ||
			  l_txrxVerificationParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;			
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup

		g_RecordedParam.FREQ_MHZ				= l_txrxVerificationParam.FREQ_MHZ;	
		g_RecordedParam.POWER_DBM				= l_txrxVerificationParam.LOW_TX_POWER_DBM;		// TODO: note that must record Low Power 
	    g_RecordedParam.SIG_TYPE				= l_txrxVerificationParam.SIG_TYPE;
	    g_RecordedParam.BANDWIDTH_MHZ			= l_txrxVerificationParam.BANDWIDTH_MHZ;
	    g_RecordedParam.CYCLIC_PREFIX			= l_txrxVerificationParam.CYCLIC_PREFIX;
	    g_RecordedParam.RATE_ID					= l_txrxVerificationParam.RATE_ID;
	    g_RecordedParam.NUM_SYMBOLS				= l_txrxVerificationParam.NUM_SYMBOLS;
		g_RecordedParam.ANT1					= l_txrxVerificationParam.TX1;
		g_RecordedParam.ANT2					= l_txrxVerificationParam.TX2;
		g_RecordedParam.ANT3					= l_txrxVerificationParam.TX3;
		g_RecordedParam.ANT4					= l_txrxVerificationParam.TX4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txrxVerificationParam.TX_CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txrxVerificationParam.TX_CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txrxVerificationParam.TX_CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txrxVerificationParam.TX_CABLE_LOSS_DB[3];	
		sprintf_s(g_RecordedParam.MAP_CONFIG_FILE_NAME, MAX_BUFFER_SIZE, l_txrxVerificationParam.MAP_CONFIG_FILE_NAME);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] CheckParameters() Confirmed.\n");
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

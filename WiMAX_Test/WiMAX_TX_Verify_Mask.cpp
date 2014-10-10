#include "stdafx.h"
#include "TestManager.h"
#include "WIMAX_Test.h"
#include "WIMAX_Test_Internal.h"
#include "IQmeasure.h"
#include "StringUtil.h"
#include "math.h" 
#include "vDUT.h"

using namespace std;

// This global variable is declared in WIMAX_Test_Internal.cpp
extern TM_ID                 g_WIMAX_Test_ID;
extern vDUT_ID               g_WIMAX_Dut;
extern bool					 g_vsgContinuousTxMode;
extern bool					 g_vDutTxActived;
extern bool					 g_dutConfigChanged;
extern WIMAX_RECORD_PARAM    g_RecordedParam;
extern char					 g_defaultFilePath[MAX_BUFFER_SIZE];

// This global variable is declared in WIMAX_Global_Setting.cpp
extern WIMAX_GLOBAL_SETTING g_WIMAXGlobalSettingParam;


// Definition of Mask Type of measurement /* #LPTW# cfy,-2010/06/07- */
enum 
{
	MASK_TYPE_REF,		// reference(ref)
	MASK_TYPE_ABS		// absolute(abs)
};

typedef struct tagDefaultMask
{ 
    double  startFrequencyOffset;
    double  stopFrequencyOffset;
	double	startLimit;
	double	stopLimit; 
	int		integrationBW; // In our implementation, integration bandwidth = resolution bandwidth (RBW)
	
} DEFAULT_MASK_STRUCT;

typedef struct averageFFTresult
{
	int		specifiedRBWlevels;						// Recorded in case needed
	int		bufferSize;								// bufferSizeX = bufferSizeY
	double	avgBufferRealX_FFT[MAX_BUFFER_SIZE*3];
	double	avgBufferRealY_FFT[MAX_BUFFER_SIZE*3];

} AVERAGE_FFT_RESULT;

int 						l_maskType = MASK_TYPE_ABS; // Set the value of mask type to "absolute" by default
vector<AVERAGE_FFT_RESULT>	l_averageFFTresults;
vector<DEFAULT_MASK_STRUCT> l_WIMAX_MASK;
vector<int>					l_specifiedRBWlevels;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIMAX_SETTING_STRUCT> l_txVerifyMaskParamMap;

// Return Value Container
map<string, WIMAX_SETTING_STRUCT> l_txVerifyMaskReturnMap;

struct tagParam
{
    // Mandatory Parameters
    int    FREQ_MHZ;                                /*! The center frequency (MHz). */  
	int	   SIG_TYPE;								/*! Indicates the type of signal. 1=downlink signal, 2=uplink signal, 0=auto-detect\r\nDefault: 0. */
	int    RATE_ID;									/*! Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and  correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively\r\nDefault: 0. */
	double BANDWIDTH_MHZ;							/*! Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20\r\nDefault: 0 */
    double CYCLIC_PREFIX;							/*! Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32\r\nDefault: 0. */
	double NUM_SYMBOLS;								/*! Number of OFDM symbols in burst. Default: 0. (auto detect) */
    double TX_POWER_DBM;                            /*! The output power to verify Mask. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
    double SAMPLING_TIME_US;                        /*! The sampling time to verify Mask. */
	char   MASK_LIMITS_FILE_NAME[MAX_BUFFER_SIZE];	/*! For 802.16e Mask analysis, a Mask limits file (*.csv) is required for transmitter spectral mask compliance test. */
	char   WAVEFORM_FILE_NAME[MAX_BUFFER_SIZE];	/*! For 802.16e, sometime VSG needs a specific mod file to synchronize the test device, if the WIMAX_TEST_MODE=LINK. The value can be AUTO(using GLOBAL_SETTINGS->WIMAX_WAVEFORM_NAME) or NULL(or empty). Default: AUTO. */

    // DUT Parameters
    int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on)  */
    int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */
    int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */
} l_txVerifyMaskParam;

struct tagReturn
{
    // POWER Test Result 
    double POWER_AVERAGE_DBM;								/*!< (Average) Average power in dBm. */

    // Mask Test Result 
	char   MASK_TYPE[MAX_BUFFER_SIZE];						/*!< The Mask Type that defined by MaskRequirement.csv, can be reference(ref) or absolute(abs). */
    double MARGIN_DB_POSITIVE[MAX_SEGMENT_NUM];		/*!< Margin to the mask, normally should be a negative number, if pass. */
	double MARGIN_DB_NEGATIVE[MAX_SEGMENT_NUM];		/*!< Margin to the mask, normally should be a negative number, if pass */
	double FREQ_AT_MARGIN_POSITIVE[MAX_SEGMENT_NUM];	/*!< Point out the frequency offset at margin to the mask */
	double FREQ_AT_MARGIN_NEGATIVE[MAX_SEGMENT_NUM];	/*!< Point out the frequency offset at margin to the mask */
	double VIOLATION_PERCENT;								/*!< Percentage which fail the mask */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];					/*!< The path loss of test system. */
	vector <double> SPECTRUM_RAW_DATA_X;					/*!< spectrum X raw data  */
	vector <double> SPECTRUM_RAW_DATA_Y;					/*!< spectrum Y raw data  */

    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyMaskReturn;

#pragma endregion

void ClearTxVerifyMaskReturn(void)
{
	l_WIMAX_MASK.clear();

	l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X.clear();
	l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y.clear();

	l_txVerifyMaskParamMap.clear();
	l_txVerifyMaskReturnMap.clear();
}

// These global variables/functions only for WIMAX_TX_Verify_Mask.cpp
int CheckTxMaskParameters( double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg );


//! WIMAX TX Verify Mask
/*!
* Input Parameters
*
*  - Mandatory 
*      -# FREQ_MHZ (double): The center frequency (MHz)
*      -# DATA_RATE (string): The data rate to verify Mask
*      -# TX_POWER (double): The power (dBm) DUT is going to transmit at the antenna port
*
* Return Values
*      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/


WIMAX_TEST_API int WIMAX_TX_Verify_Mask(void)
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
	ClearReturnParameters(l_txVerifyMaskReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_txVerifyMaskParamMap);
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
        RespondToQueryReturn(l_txVerifyMaskReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< double >
	 *-----------------------------------------------------------------------------------------*/
	vector< double >    powerAvEachBurst(g_WIMAXGlobalSettingParam.MASK_FFT_AVERAGE);

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
		err = GetInputParameters(l_txVerifyMaskParamMap);
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

		err = CheckTxMaskParameters( &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );;
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Prepare input parameters CheckTxMaskParameters() return OK.\n");
		}
#pragma endregion

#pragma region SETUP VSG FOR SYNCHRONIZATION
		/*-------------------------------*
		 * SETUP VSG FOR SYNCHRONIZATION *
		 *-------------------------------*/
		if ( 0==strcmp(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE,"LINK") )
		{
			if ( (0==strcmp(l_txVerifyMaskParam.WAVEFORM_FILE_NAME,""))||(0==strcmp(l_txVerifyMaskParam.WAVEFORM_FILE_NAME,"NULL")) )
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
					if ( 0==strcmp(l_txVerifyMaskParam.WAVEFORM_FILE_NAME,"AUTO") )
					{
						sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, g_WIMAXGlobalSettingParam.WIMAX_WAVEFORM_NAME);
					}
					else
					{
						sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, l_txVerifyMaskParam.WAVEFORM_FILE_NAME);
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
						err = ::LP_SetVsg(l_txVerifyMaskParam.FREQ_MHZ*1e6, VSG_POWER, PORT_RIGHT);
					}
					else	// VSA => PORT_RIGHT, then VSG must => PORT_LEFT 
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG to PORT_LEFT, since VSA already using PORT_RIGHT.\n");
						err = ::LP_SetVsg(l_txVerifyMaskParam.FREQ_MHZ*1e6, VSG_POWER, PORT_LEFT);
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

		vDUT_AddIntegerParameter(g_WIMAX_Dut, "FREQ_MHZ",				l_txVerifyMaskParam.FREQ_MHZ);	
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "SIG_TYPE",				l_txVerifyMaskParam.SIG_TYPE);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "RATE_ID",				l_txVerifyMaskParam.RATE_ID);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "BANDWIDTH_MHZ",			l_txVerifyMaskParam.BANDWIDTH_MHZ);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "CYCLIC_PREFIX",			l_txVerifyMaskParam.CYCLIC_PREFIX);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "NUM_SYMBOLS",			l_txVerifyMaskParam.NUM_SYMBOLS);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "TX_POWER_DBM",			l_txVerifyMaskParam.TX_POWER_DBM);
		//vDUT_AddStringParameter (g_WIMAX_Dut, "MAP_CONFIG_FILE_NAME",	l_txVerifyMaskParam.MAP_CONFIG_FILE_NAME);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX1",					l_txVerifyMaskParam.TX1);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX2",					l_txVerifyMaskParam.TX2);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX3",					l_txVerifyMaskParam.TX3);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "TX4",					l_txVerifyMaskParam.TX4);

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

#pragma region Setup LP Tester

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

		err = ::LP_SetVsa(  l_txVerifyMaskParam.FREQ_MHZ*1e6,
							l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio,
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to Quick capture signal at %d MHz.\n", l_txVerifyMaskParam.FREQ_MHZ);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Quick LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyMaskParam.FREQ_MHZ);
			}		

			/*--------------------------------*
			 *  Perform Quick Power Analysis  *
			 *--------------------------------*/
			err = ::LP_AnalyzePower( );
			if ( ERR_OK!=err )
			{
				// Fail Analysis, thus save capture (Signal File) for debug
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ);
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
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Result_Failed", l_txVerifyMaskParam.FREQ_MHZ);
					WIMAXSaveSigFile(sigFileNameBuffer);
					err = -1;				
					sprintf_s(logMessage, MAX_BUFFER_SIZE, "[WIMAX] Quick LP_GetScalarMeasurement(P_peak_all_dBm) return error and save the sig file %s.\n", sigFileNameBuffer);
					return err;
				}
				else
				{			
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Quick power analysis OK, P_peak_all_dBm = %.2f.\n", pkPowerDbm);

					// Check if the power over the range, then needs some delay for power settle down
					double targetPkPower = l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio;
					if ( abs(targetPkPower-pkPowerDbm)>g_WIMAXGlobalSettingParam.DUT_QUICK_POWER_CHECK_TOLERANCE_DB )
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
#pragma endregion
		/*-----------------------------------------*
		 *  Load Mask requirement from *.csv file  *
		 *-----------------------------------------*/
		err = UpdateMaskLimitsByFile( l_txVerifyMaskParam.MASK_LIMITS_FILE_NAME );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to update Mask limit requirement from file %s.\n", l_txVerifyMaskParam.MASK_LIMITS_FILE_NAME);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] UpdateMaskLimitsByFile() return OK.\n");
		}
		
	    /*-------------------------------*
	     *  Initial the return results   *
	     *-------------------------------*/

		/* #LPTW# cfy,-2010/06/07- */
		l_txVerifyMaskReturn.VIOLATION_PERCENT  = 0;
		l_specifiedRBWlevels.clear();

		// List all different RBW levels that need to be FFT anylasis individually		
		for (int i=0; i<(int)l_WIMAX_MASK.size(); i++)
		{
			bool found = false;
			vector<int>::iterator it = l_specifiedRBWlevels.begin();
			
			do 
			{
				// Add RBW level into the list if it's not yet exist
				if ( it == l_specifiedRBWlevels.end() && !found )
				{
					l_specifiedRBWlevels.push_back(l_WIMAX_MASK[i].integrationBW);
					found = true;
				} 
				else if ( l_WIMAX_MASK[i].integrationBW == *it )					
				{
					found = true;
				} 
				else
				{
					it++;
				}
			} while (!found);
		}

		/* Implement MASK_FFT_AVERAGE for IQapi #LPTW# cfy,-2010/05/18- */
		/*------------------------------*
	     * Start while loop for average *
	     *------------------------------*/
		avgIteration = 0;
		l_averageFFTresults.clear();

		while ( avgIteration<g_WIMAXGlobalSettingParam.MASK_FFT_AVERAGE )
		{
#pragma region VSA Capture
			/*----------------------------*
			 * Perform Normal VSA capture *
			 *----------------------------*/
			err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_WIMAXGlobalSettingParam.VSA_TRIGGER_TYPE );
			if (ERR_OK!=err)	// capture is failed
			{
				double quickPower = NA_NUMBER;					
				err = QuickCaptureRetry( l_txVerifyMaskParam.FREQ_MHZ, samplingTimeUs, g_WIMAXGlobalSettingParam.VSA_TRIGGER_TYPE, &quickPower, logMessage);
				if (ERR_OK!=err)	// QuickCaptureRetry() is failed
				{
					// Fail Capture
					if ( quickPower!=NA_NUMBER )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz.\nThe DUT power (without path loss) = %.1f dBm and QuickCaptureRetry() return error.\n", l_txVerifyMaskParam.FREQ_MHZ, quickPower);
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to capture WiMAX signal at %d MHz, QuickCaptureRetry() return error.\n", l_txVerifyMaskParam.FREQ_MHZ);
					}
					throw logMessage;
				}			
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] The QuickCaptureRetry() at %d MHz return OK.\n", l_txVerifyMaskParam.FREQ_MHZ);
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_VsaDataCapture() at %d MHz return OK.\n", l_txVerifyMaskParam.FREQ_MHZ);
			}

			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;
			if (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WIMAX_TX_Mask_SaveAlways", l_txVerifyMaskParam.FREQ_MHZ);
				WIMAXSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}
#pragma endregion

			/*------------------*
			 *  Power Analysis  *
			 *------------------*/
			double dummy_T_INTERVAL      = 3.2;
			double dummy_MAX_POW_DIFF_DB = 15.0;  
			err = ::LP_AnalyzePower( dummy_T_INTERVAL/1000000, dummy_MAX_POW_DIFF_DB );
			if ( ERR_OK!=err )
			{
				// Fail Analysis, thus save capture (Signal File) for debug
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WIMAX_TX_Power_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ);
				WIMAXSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] LP_AnalyzePower() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] LP_AnalyzePower() return OK.\n");
			}
	
			/*-----------------------------*
			 *  Retrieve analysis Results  *
			 *-----------------------------*/
			avgIteration++;
			analysisOK = true;
	
			// Since the limitation, we assume that all path loss value are very close.	
			// powerAvEachBurst  
			powerAvEachBurst[avgIteration-1]= ::LP_GetScalarMeasurement("P_av_no_gap_all_dBm", 0);
	
			if ( -99.00 >= powerAvEachBurst[avgIteration-1] )
			{
				analysisOK = false;
				powerAvEachBurst[avgIteration-1] = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WIMAX_TX_Power_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ);
				WIMAXSaveSigFile(sigFileNameBuffer);
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_av_no_gap_all_dBm) return error.\n");
				throw logMessage;
			}
			else
			{
	            //The current code will work correctly with 4x4 MIMO.  
				int antenaOrder = 0;
				err = CheckAntennaOrderByStream(1, l_txVerifyMaskParam.TX1, l_txVerifyMaskParam.TX2, l_txVerifyMaskParam.TX3, l_txVerifyMaskParam.TX4, &antenaOrder);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] CheckAntennaOrderByStream() return error.\n");					
					throw logMessage;
				}
				powerAvEachBurst[avgIteration-1] = powerAvEachBurst[avgIteration-1] + l_txVerifyMaskParam.CABLE_LOSS_DB[antenaOrder-1];					
			}

			// Execute FFT anylasis for all different RBW levels /* #LPTW# cfy,-2010/06/07- */
			for (int indexRBW=0; indexRBW<(int)l_specifiedRBWlevels.size(); indexRBW++)
			{
#pragma region Analysis
				/*----------------*
				 *  FFT Analysis  *
				 *----------------*/
				// Perform FFT analysis
				int NFFT = 0;
				err = ::LP_AnalyzeFFT( NFFT, l_specifiedRBWlevels[indexRBW]*1000 );
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WIMAX_TX_Mask_Analysis_Failed", l_txVerifyMaskParam.FREQ_MHZ);
					WIMAXSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Analyze FFT return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Analyze FFT return OK.\n");
				}
#pragma endregion
#pragma region Retrieve analysis Results
				/*-------------------------------*
				 *  Video Averaging the spectrum *
				 *-------------------------------*/
				/* Video Averaging is performed on a point-to-point basis over two or more sweeps,
				   which averaging the display trace point is each display pixel after the log amplifier in traditional SA.
				   Belowing is our implementation as a simulation of VSA for this function. With video averaging,
				   at each display point the new value is averaged in with the previously averaged data. #LPTW# cfy,-2010/05/20- */
				analysisOK = true;
	
				// Store the result
				// TODO, Since when RWB=30kHz, the FFT size will be 8192 > MAX_BUFFER_SIZE
				// Thus, must increase the buffer size.
				double tmpBufferRealX[MAX_BUFFER_SIZE*3], dummyBufferImagX[MAX_BUFFER_SIZE*3];
				double tmpBufferRealY[MAX_BUFFER_SIZE*3], dummyBufferImagY[MAX_BUFFER_SIZE*3];
				int tmpBufferSizeX = 0;
				int tmpBufferSizeY = 0;

				memset(&tmpBufferRealX, 0, sizeof(tmpBufferRealX));
				memset(&tmpBufferRealY, 0, sizeof(tmpBufferRealY));
				tmpBufferSizeX = ::LP_GetVectorMeasurement("x", tmpBufferRealX, dummyBufferImagX, MAX_BUFFER_SIZE*3);
				tmpBufferSizeY = ::LP_GetVectorMeasurement("y", tmpBufferRealY, dummyBufferImagY, MAX_BUFFER_SIZE*3);
					
				// In case we don't have enough buffer size for FFT results
				if ( (tmpBufferSizeX > MAX_BUFFER_SIZE*3)||(tmpBufferSizeY > MAX_BUFFER_SIZE*3) )
				{
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] No enough space to retrieve FFT result (RBW=%dKHz), result length = %d.\n", l_specifiedRBWlevels[indexRBW], tmpBufferSizeX);
					throw logMessage;
				} 
				else if ( (tmpBufferSizeX <= 0)||(tmpBufferSizeY <= 0) )
				{
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Can not retrieve FFT result (RBW=%dKHz)\n", l_specifiedRBWlevels[indexRBW]);
					throw logMessage;
				}

				if (avgIteration==1) // first capture
				{
					AVERAGE_FFT_RESULT tmpResult;
					memset(&tmpResult, 0, sizeof(tmpResult));

					// Capture length is always the same, so just use the first result
					if (tmpBufferSizeX == tmpBufferSizeY)
					{
						tmpResult.bufferSize = tmpBufferSizeX;
					}
					else
					{
						// Should not happend
					}

					// Direct copy from the first result
					tmpResult.specifiedRBWlevels = l_specifiedRBWlevels[indexRBW];
					memcpy(tmpResult.avgBufferRealX_FFT, tmpBufferRealX, sizeof(tmpBufferRealX));	
					memcpy(tmpResult.avgBufferRealY_FFT, tmpBufferRealY, sizeof(tmpBufferRealY));

					l_averageFFTresults.push_back(tmpResult);					
				}
				else // average analysis results each time
				{
					for (int i=0; i<l_averageFFTresults[indexRBW].bufferSize; i++)
					{
						// We only need to perform the averaging for Y real array because X are fixed and images are empty
						l_averageFFTresults[indexRBW].avgBufferRealY_FFT[i] = \
							((l_averageFFTresults[indexRBW].avgBufferRealY_FFT[i] * (avgIteration-1)) + tmpBufferRealY[i])/(avgIteration);			
					}
				}
			}
		} // End - avgIteration
			
#pragma region Averaging and Saving Test Result
	   /*----------------------------------*
		* Averaging and Saving Test Result *
		*----------------------------------*/
		if ( (ERR_OK==err) && captureOK && analysisOK )
		{
			// Average Power test result
			double dummyMax = 0, dummyMin = 0;
			AverageTestResult(&powerAvEachBurst[0], avgIteration, LOG_10, l_txVerifyMaskReturn.POWER_AVERAGE_DBM, dummyMax, dummyMin);
		}
		else
		{
			// do nothing
		}
#pragma endregion 
			
		// Anylasis averaging FFT results for all different RBW levels
		for (int indexRBW=0; indexRBW<(int)l_specifiedRBWlevels.size(); indexRBW++)
		{
			// [TBD] There is only one set of return data by default, so we only return the first RBW result for now
			if (indexRBW == 0)
			{	
				l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X.clear();
				l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y.clear();

				//Return Mask Raw Data
	            for(int i=0; i<l_averageFFTresults[indexRBW].bufferSize; i++) // PS. bufferSizeX == bufferSizeY
	            {
					l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X.push_back(l_averageFFTresults[indexRBW].avgBufferRealX_FFT[i]);
					l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y.push_back(l_averageFFTresults[indexRBW].avgBufferRealY_FFT[i]);
	            }
			}
	
			// Since the limitation, we assume that all path loss value are very close.	
			int antenaOrder = 0;
			err = CheckAntennaOrderByStream(1, l_txVerifyMaskParam.TX1, l_txVerifyMaskParam.TX2, l_txVerifyMaskParam.TX3, l_txVerifyMaskParam.TX4, &antenaOrder);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] CheckAntennaOrderByStream() return error.\n");					
				throw logMessage;
			}

			// Verify MASK
			l_txVerifyMaskReturn.VIOLATION_PERCENT += VerifyPowerMask(	l_averageFFTresults[indexRBW].avgBufferRealX_FFT, \
																		l_averageFFTresults[indexRBW].avgBufferRealY_FFT, \
																		l_averageFFTresults[indexRBW].bufferSize, \
																		l_txVerifyMaskParam.CABLE_LOSS_DB[antenaOrder-1], \
																		l_specifiedRBWlevels[indexRBW]);
		}

		// Adjust percentage if there is a rounding error (Round-off error)
		if (l_txVerifyMaskReturn.VIOLATION_PERCENT > 100.0) l_txVerifyMaskReturn.VIOLATION_PERCENT = 100.0;


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
			sprintf_s(l_txVerifyMaskReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txVerifyMaskReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txVerifyMaskReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
		err = -1;
    }

	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	// Transfer the absolute frequency results to relative frequency offset
	for (int i=0;i<MAX_SEGMENT_NUM;i++)
	{
		if (l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i]!=NA_NUMBER)		
		{
			l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i]-l_txVerifyMaskParam.FREQ_MHZ;
		}
		if (l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i]!=NA_NUMBER)		
		{
			l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i]-l_txVerifyMaskParam.FREQ_MHZ;
		}
	}
	ReturnTestResults(l_txVerifyMaskReturnMap);

	return err;
}


void InitializeTXVerifyMaskContainers(void)
{
    /*-------------------*
     * Input Parameters  *
     *-------------------*/
    l_txVerifyMaskParamMap.clear();

    WIMAX_SETTING_STRUCT setting;

    l_txVerifyMaskParam.FREQ_MHZ = 3500;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("FREQ_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.SIG_TYPE = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.SIG_TYPE))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.SIG_TYPE;
        setting.unit        = "";
        setting.helpText    = "Indicates the type of signal. 1=downlink signal, 2=uplink signal, 0=auto-detect\r\nDefault: 0.";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SIG_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.BANDWIDTH_MHZ = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyMaskParam.BANDWIDTH_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.BANDWIDTH_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20\r\nDefault: 0.";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("BANDWIDTH_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.CYCLIC_PREFIX = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyMaskParam.CYCLIC_PREFIX))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.CYCLIC_PREFIX;
        setting.unit        = "";
        setting.helpText    = "Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32\r\nDefault: 0.";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CYCLIC_PREFIX", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.RATE_ID = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.RATE_ID))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.RATE_ID;
        setting.unit        = "";
        setting.helpText    = "Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and  correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively\r\nDefault: 0.";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RATE_ID", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.NUM_SYMBOLS = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyMaskParam.NUM_SYMBOLS))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.NUM_SYMBOLS;
        setting.unit        = "";
        setting.helpText    = "Number of OFDM symbols in burst. Default: 0 (auto detect).";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("NUM_SYMBOLS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.SAMPLING_TIME_US = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyMaskParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyMaskParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyMaskParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyMaskParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyMaskParam.TX_POWER_DBM = 15.0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyMaskParam.TX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.TX1 = 1;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX1))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.TX1;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.TX2 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX2))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.TX2;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.TX3 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX3))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.TX3;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.TX4 = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX4))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyMaskParam.TX4;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyMaskParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TX4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
	strcpy_s(l_txVerifyMaskParam.MASK_LIMITS_FILE_NAME, MAX_BUFFER_SIZE, "MaskRequirement.csv");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.MASK_LIMITS_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyMaskParam.MASK_LIMITS_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "For 802.16e Mask analysis, a Mask limits file (*.csv) is required for transmitter spectral mask compliance test.";
        l_txVerifyMaskParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("MASK_LIMITS_FILE_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txVerifyMaskParam.WAVEFORM_FILE_NAME, MAX_BUFFER_SIZE, "AUTO");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.WAVEFORM_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyMaskParam.WAVEFORM_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "For 802.16e, sometime VSG needs a specific mod file to synchronize the test device, if the WIMAX_TEST_MODE=LINK. The value can be AUTO(using GLOBAL_SETTINGS->WIMAX_WAVEFORM_NAME) or NULL(or empty). Default: AUTO.";
        l_txVerifyMaskParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("WAVEFORM_FILE_NAME", setting) );
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
    l_txVerifyMaskReturnMap.clear();

    l_txVerifyMaskReturn.POWER_AVERAGE_DBM = NA_NUMBER;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyMaskReturn.POWER_AVERAGE_DBM))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskReturn.POWER_AVERAGE_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "The average power in dBm.";
        l_txVerifyMaskReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("POWER_AVERAGE_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for (int i=0;i<MAX_SEGMENT_NUM;i++)
    {
		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "MARGIN_DB_UPPER_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
            l_txVerifyMaskReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_SEGMENT_NUM;i++)
    {
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "MARGIN_DB_LOWER_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
            l_txVerifyMaskReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_SEGMENT_NUM;i++)
    {
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "FREQ_AT_MARGIN_UPPER_%d", i+1);
            setting.unit        = "MHz";
            setting.helpText    = "Point out the frequency offset at margin to the mask.";
            l_txVerifyMaskReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_SEGMENT_NUM;i++)
    {
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i] = NA_NUMBER;
        setting.type = WIMAX_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "FREQ_AT_MARGIN_LOWER_%d", i+1);
            setting.unit        = "MHz";
            setting.helpText    = "Point out the frequency offset at margin to the mask.";
            l_txVerifyMaskReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }
    
    l_txVerifyMaskReturn.VIOLATION_PERCENT = NA_NUMBER;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyMaskReturn.VIOLATION_PERCENT))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskReturn.VIOLATION_PERCENT;
        setting.unit        = "%";
        setting.helpText    = "Percentage which fail the mask";
        l_txVerifyMaskReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("VIOLATION_PERCENT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyMaskReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyMaskReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyMaskReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyMaskReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X.clear();
    setting.type = WIMAX_SETTING_TYPE_ARRAY_DOUBLE;
    if (0 == l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X.size())    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X;
        setting.unit        = "dBm/<RBW>kHz";
        setting.helpText    = "Spectrum X raw data.";
        l_txVerifyMaskReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SPECTRUM_RAW_DATA_X", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y.clear();
    setting.type = WIMAX_SETTING_TYPE_ARRAY_DOUBLE;
    if (0 == l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y.size())    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y;
        setting.unit        = "dBm/<RBW>kHz";
        setting.helpText    = "Spectrum Y raw data.";
        l_txVerifyMaskReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SPECTRUM_RAW_DATA_Y", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskReturn.MASK_TYPE[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskReturn.MASK_TYPE))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyMaskReturn.MASK_TYPE;
        setting.unit        = "";
        setting.helpText    = "The Mask Type that defined by MaskRequirement.csv, can be reference(ref) or absolute(abs).";
        l_txVerifyMaskReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("MASK_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyMaskReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_txVerifyMaskReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}

//=============================================================================
//	Description:
//		A generic function for interpolating. Find y_val when xVal is present.
//
//                             (yMax - yMin)
//    Returns y_val = yMin + (--------------- * (xVal - xMin))
//                             (xMax - xMin)
//=============================================================================
inline double IQInterpolate( double yMax, double yMin, double xMax, double xMin, double xVal ) 
{
	return (yMin + ((yMax - yMin) / (xMax - xMin) * (xVal - xMin)));
}

//=============================================================================
//	Description:
//		Power mask verification of specified RBW
//
//    Returns Fail Percentage 
//												/* #LPTW# cfy,-2010/06/07- */
//=============================================================================
double VerifyPowerMask ( double *freqBuf, double *maskBuf, int maskBufCnt, double cableLoss, int analysingRBWlevel) 
{
    int		passTest = ERR_OK;
    int		failPoints = 0, segmentPoints = 0;
	double	failPercentage = 0;
	double	deltaPower = 0, xFreqVal = 0, yDeltaPwrdB = 0;
	
    /*--------------------------------------------*
     *  Looking for margin power by each segment  *
     *--------------------------------------------*/
    for ( int i=0; i<maskBufCnt; i++ )
    {
		xFreqVal = fabs( (freqBuf[i]/1000000) );// to MHz

		for ( int segmentInedx=0; segmentInedx<(int)l_WIMAX_MASK.size(); segmentInedx++ )
		{
			// Check xFreqVal(currentFreq) belong to which segment
			if (( l_WIMAX_MASK[segmentInedx].startFrequencyOffset<=xFreqVal )&&( xFreqVal<l_WIMAX_MASK[segmentInedx].stopFrequencyOffset))
			{
				segmentPoints++;

				// Only verify the corresponding segment
				if ( l_WIMAX_MASK[segmentInedx].integrationBW == analysingRBWlevel )
				{
					yDeltaPwrdB = IQInterpolate( l_WIMAX_MASK[segmentInedx].stopLimit, l_WIMAX_MASK[segmentInedx].startLimit, l_WIMAX_MASK[segmentInedx].stopFrequencyOffset, l_WIMAX_MASK[segmentInedx].startFrequencyOffset, xFreqVal );

					if (l_maskType == MASK_TYPE_ABS) 
					{					
						deltaPower = maskBuf[i] - yDeltaPwrdB + cableLoss;	// check Mask ("absolute" type)
					}
					else /* l_maskType == MASK_TYPE_REF */
					{
						deltaPower = maskBuf[i] - (yDeltaPwrdB + l_txVerifyMaskReturn.POWER_AVERAGE_DBM) + cableLoss;	// check Mask ("reference" type)
					}

					if ( deltaPower>0 )  // Mask Failed
					{
						failPoints++;
					}
					else // Mask Pass
					{
						// keep compare...
					}

					// Check Segment 
					if ( i>=(int)(maskBufCnt/2) )	// This is the Upper(positive) segment
					{
						if ( deltaPower>l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[segmentInedx] )
						{
							l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[segmentInedx]      = deltaPower;		// save margin		
							l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[segmentInedx] = l_txVerifyMaskParam.FREQ_MHZ + (freqBuf[i]/1e6);	// save frequency
						}
						else
						{
							// do nothing, keep compare...
						}
					}
					else	// This is the Lower(negative) segment
					{
						if ( deltaPower>l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[segmentInedx] )
						{
							l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[segmentInedx]      = deltaPower;		// save margin		
							l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[segmentInedx] = l_txVerifyMaskParam.FREQ_MHZ + (freqBuf[i]/1e6);	// save frequency
						}
						else
						{
							// do nothing, keep compare...
						}
					}
		
					// Nick add break to speed test					
					break; // If this xFreqVal is already verified, skip checking other segement
				}
			}
			else
			{
				// not in this Mask segment, continue and keep searching
			}
		}
	}

	// Calculate failure percentage only based on available segment points but not all FFT points /* #LPTW# cfy,-2010/06/07- */
	failPercentage = (double)failPoints/segmentPoints * 100.0;

    #if defined(_DEBUG)
		// print out the Mask into file for debug
		FILE *fp;
		fopen_s(&fp, "Log_Mask_Result_test.csv", "w");    
		if (fp)
		{
			fprintf(fp, "Mask_Frequency,Signal,Size: %d\n", maskBufCnt); // print to log file
			for (int x=0;x<maskBufCnt;x++)
			{
				fprintf(fp, "%8.2f,%8.2f\n", freqBuf[x], maskBuf[x]); // print to log file
			}
			fclose(fp);
		}
    #endif

	return failPercentage;
}


int UpdateMaskLimitsByFile( char* maskLimitsName )
{
    int		err = ERR_OK;
    DEFAULT_MASK_STRUCT dummyStruct;

    FILE *maskLimitsFile = NULL;
    fopen_s( &maskLimitsFile, maskLimitsName, "rt" );

    if( NULL!=maskLimitsFile )
    {
        // clear the current mask limits
        l_WIMAX_MASK.clear();

        char buffer[MAX_BUFFER_SIZE] = {'\0'};
        string line;
        vector<string> splits;
        
        while( !feof( maskLimitsFile ) )
        {
			for (int i=0;i<MAX_BUFFER_SIZE;i++)
			{
				buffer[i] = '\0';
			}

            fgets(buffer, MAX_BUFFER_SIZE, maskLimitsFile);

		    int x=0;
		    while (buffer[x])
			{			
			    buffer[x] = toupper(buffer[x]);
			    x++;
		    }

            line = buffer;
            Trim( line );

			if( (0==line.size())||(string::npos!=line.find("#"))||(string::npos!=line.find("//")) )
            {
                // skip blank and comment lines
                continue;
            } 
            else
            {
                splits.clear();
                SplitString(line, splits, ",");
                switch (splits.size()) 
                {	
					case 2: // Format: < reference(ref)/absolute(abs) >
						if ( string::npos!=splits[0].find("MASK_TYPE") )
						{
							if ( (string::npos!=splits[1].find("REFERENCE"))||(string::npos!=splits[1].find("REF")) )
							{
								l_maskType = MASK_TYPE_REF;
								strcpy_s( l_txVerifyMaskReturn.MASK_TYPE, MAX_BUFFER_SIZE, "REFERENCE" ); 
							}
							else if ( (string::npos!=splits[1].find("ABSOLUTE"))||(string::npos!=splits[1].find("ABS")) )
							{
								l_maskType = MASK_TYPE_ABS;
								strcpy_s( l_txVerifyMaskReturn.MASK_TYPE, MAX_BUFFER_SIZE, "ABSOLUTE" ); 
							}
							else // Set the value of mask type to "absolute" by default
							{
								l_maskType = MASK_TYPE_ABS;
								strcpy_s( l_txVerifyMaskReturn.MASK_TYPE, MAX_BUFFER_SIZE, "ABSOLUTE" );
							}
						}
						else 
						{ 
							/* do nothing */
						}
						break;
					
					case 4: // Format: < Start_Frequency_Offset >,< Stop_Frequency_Offset >,< Limit_1 >,< Limit_2 >
						dummyStruct.startFrequencyOffset	= atof(splits[0].c_str());
						dummyStruct.stopFrequencyOffset		= atof(splits[1].c_str());
						dummyStruct.startLimit				= atof(splits[2].c_str());
						dummyStruct.stopLimit				= atof(splits[3].c_str()); 
						dummyStruct.integrationBW			= g_WIMAXGlobalSettingParam.ANALYSIS_FFT_RBW; // for backwards compatibility
						
						l_WIMAX_MASK.push_back( dummyStruct );
						break;
						
					case 5: // Format: < Start_Frequency_Offset >,< Stop_Frequency_Offset >,< Limit_1 >,< Limit_2 >,< Integration_Bandwidth >
						dummyStruct.startFrequencyOffset	= atof(splits[0].c_str());
						dummyStruct.stopFrequencyOffset		= atof(splits[1].c_str());
						dummyStruct.startLimit				= atof(splits[2].c_str());
						dummyStruct.stopLimit				= atof(splits[3].c_str());
						dummyStruct.integrationBW 			= atoi(splits[4].c_str());

						l_WIMAX_MASK.push_back( dummyStruct );
						break;
						
					default: /* do nothing */						
						break;
				}
            }
        }
        fclose(maskLimitsFile);
    }
    else
    {
        // Failed to open the file
        err = TM_ERR_FAILED_TO_OPEN_FILE;
    }
    return err;
}


//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
// 
//-------------------------------------------------------------------------------------
int CheckTxMaskParameters( double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Check path loss (by ant and freq)
		err = CheckPathLossTableExt( g_WIMAX_Test_ID,
								  l_txVerifyMaskParam.FREQ_MHZ,
								  l_txVerifyMaskParam.TX1,
								  l_txVerifyMaskParam.TX2,
								  l_txVerifyMaskParam.TX3,
								  l_txVerifyMaskParam.TX4,
								  l_txVerifyMaskParam.CABLE_LOSS_DB,
								  l_txVerifyMaskReturn.CABLE_LOSS_DB,
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
		if (0==l_txVerifyMaskParam.SAMPLING_TIME_US)
		{
			*samplingTimeUs = g_WIMAXGlobalSettingParam.MASK_SAMPLE_INTERVAL_US;
		}
		else	// SAMPLING_TIME_US != 0
		{
			*samplingTimeUs = l_txVerifyMaskParam.SAMPLING_TIME_US;
		}
		
		// Peak to Average Ratio
		*peakToAvgRatio = g_WIMAXGlobalSettingParam.IQ_P_TO_A_WIMAX;  

		// Check Dut configuration changed or not
		if (  g_WIMAXGlobalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.			  
			  l_txVerifyMaskParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
			  l_txVerifyMaskParam.SIG_TYPE!=g_RecordedParam.SIG_TYPE ||
			  l_txVerifyMaskParam.BANDWIDTH_MHZ!=g_RecordedParam.BANDWIDTH_MHZ ||
			  l_txVerifyMaskParam.CYCLIC_PREFIX!=g_RecordedParam.CYCLIC_PREFIX ||
			  l_txVerifyMaskParam.RATE_ID!=g_RecordedParam.RATE_ID ||
			  l_txVerifyMaskParam.NUM_SYMBOLS!=g_RecordedParam.NUM_SYMBOLS ||
			  l_txVerifyMaskParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
			  l_txVerifyMaskParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_txVerifyMaskParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_txVerifyMaskParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_txVerifyMaskParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  l_txVerifyMaskParam.TX1!=g_RecordedParam.ANT1 ||
			  l_txVerifyMaskParam.TX2!=g_RecordedParam.ANT2 ||
			  l_txVerifyMaskParam.TX3!=g_RecordedParam.ANT3 ||
			  l_txVerifyMaskParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;			
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup

		g_RecordedParam.FREQ_MHZ				= l_txVerifyMaskParam.FREQ_MHZ;	
		g_RecordedParam.POWER_DBM				= l_txVerifyMaskParam.TX_POWER_DBM;	
	    g_RecordedParam.SIG_TYPE				= l_txVerifyMaskParam.SIG_TYPE;
	    g_RecordedParam.BANDWIDTH_MHZ			= l_txVerifyMaskParam.BANDWIDTH_MHZ;
	    g_RecordedParam.CYCLIC_PREFIX			= l_txVerifyMaskParam.CYCLIC_PREFIX;
	    g_RecordedParam.RATE_ID					= l_txVerifyMaskParam.RATE_ID;
	    g_RecordedParam.NUM_SYMBOLS				= l_txVerifyMaskParam.NUM_SYMBOLS;
		g_RecordedParam.ANT1					= l_txVerifyMaskParam.TX1;
		g_RecordedParam.ANT2					= l_txVerifyMaskParam.TX2;
		g_RecordedParam.ANT3					= l_txVerifyMaskParam.TX3;
		g_RecordedParam.ANT4					= l_txVerifyMaskParam.TX4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txVerifyMaskParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txVerifyMaskParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txVerifyMaskParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txVerifyMaskParam.CABLE_LOSS_DB[3];	

		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] CheckTxMaskParameters() Confirmed.\n");
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
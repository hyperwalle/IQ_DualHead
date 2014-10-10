#include "stdafx.h"
#include "TestManager.h"
#include "Wimax_Test.h"
#include "Wimax_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "math.h"

using namespace std;

// This global variable is declared in WIMAX_Test_Internal.cpp
extern TM_ID		g_WIMAX_Test_ID;    
extern vDUT_ID		g_WIMAX_Dut;
extern int			g_Tester_Type;
extern bool         g_vsgContinuousTxMode;
extern bool		    g_vDutTxActived;
extern bool		    g_vDutRxActived;
extern char			g_defaultFilePath[MAX_BUFFER_SIZE];

// This global variable is declared in WIMAX_Global_Setting.cpp
extern WIMAX_GLOBAL_SETTING g_WIMAXGlobalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIMAX_SETTING_STRUCT> l_rxVerifyPerParamMap;

// Return Value Container
map<string, WIMAX_SETTING_STRUCT> l_rxVerifyPerReturnMap;

struct tagParam
{
	// Mandatory Parameters
	int    FREQ_MHZ;                                /*! The center frequency (MHz). */  
	int    FRAME_COUNT;                             /*! Number of count to send data packet */
	int    SIG_TYPE;								/*! Indicates the type of signal. 1=downlink signal, 2=uplink signal, 0=auto-detect\r\nDefault: 0. */
	int    RATE_ID;									/*! Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and  correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively\r\nDefault: 0. */
	double BANDWIDTH_MHZ;							/*! Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20\r\nDefault: 0 */
    double CYCLIC_PREFIX;							/*! Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32\r\nDefault: 0. */
	double NUM_SYMBOLS;								/*! Number of OFDM symbols in burst. Default: 0. (auto detect) */
    double RX_POWER_DBM;                            /*! The power level to verify PER. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	char   WAVEFORM_FILE_NAME[MAX_BUFFER_SIZE];		/*! For 802.16e analysis, a Waveform name (*.mod) is required for Rx test. If the value equal to AUTO that means using GLOBAL_SETTINGS->WIMAX_WAVEFORM_NAME. */

	// DUT Parameters
	int    RX1;                                     /*! DUT TX1 on/off. Default=1(on) */
	int    RX2;                                     /*! DUT TX2 on/off. Default=0(off) */
	int    RX3;                                     /*! DUT TX3 on/off. Default=0(off) */
	int    RX4;                                     /*! DUT TX4 on/off. Default=0(off) */
} l_rxVerifyPerParam;

struct tagReturn
{
	// RX Power Level
	double RX_POWER_LEVEL;							/*!< POWER_LEVEL dBm for the PER test. Format: POWER LEVEL */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */

	// PER Test Result 
	int	   GOOD_PACKETS;							/*!< GOOD_PACKETS is the number of good packets that reported from Dut */
	int    TOTAL_PACKETS;							/*!< TOTAL_PACKETS is the total number of packets. */
	double PER;										/*!< PER test result. */

	// RSSI Test Result 
	double RSSI[MAX_DATA_STREAM];					/*!< RSSI test result on specific data stream. Format: RSSI[SpecificStream] */

	// PER Test Result
	double CINR;									/*!< The test result of carrier to interference noise ratio. */
	//Nick add CINR_1, CINR_2, CINR_3, CINR_4, CINR_DELTA_MAX
	double CINR_1;									/*!< The test result of carrier to interference noise ratio for antenna1. */
	double CINR_2;									/*!< The test result of carrier to interference noise ratio for antenna2. */
	double CINR_3;									/*!< The test result of carrier to interference noise ratio for antenna1. */
	double CINR_4;									/*!< The test result of carrier to interference noise ratio for antenna2. */
	double CINR_DELTA_MAX;							/*!< The test result of carrier to interference noise ratio for the max delta between antenna1 to antenna4. */

	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_rxVerifyPerReturn;

void ClearRxVerifyPerReturn(void)
{
	l_rxVerifyPerParamMap.clear();
	l_rxVerifyPerReturnMap.clear();
}

#pragma endregion


//! WIMAX RX Verify PER
/*!
* Input Parameters
*
*  - Mandatory 
*      -# FREQ_MHZ (double): The center frequency (MHz)
*      -# DATA_RATE (string): The data rate to verify EVM
*      -# RX_POWER (double): The power (dBm) of DUT is going to receive at the antenna port
*
* Return Values
*      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/


WIMAX_TEST_API int WIMAX_RX_Verify_Per(void)
{
	int    err = ERR_OK;   
	#define vDUT_ERR_FUNCTION_NOT_SUPPORTED 3

	int    dummyValue = 0;
	double cableLossDb = 0;
	//Nick add CINR Dek=delta
	double CINRDelta[4];
	double CINRDeltaMax=-9999;
	double CINRDeltaMin=9999;
	char   vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_rxVerifyPerReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_rxVerifyPerParamMap);
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
		RespondToQueryReturn(l_rxVerifyPerReturnMap);
		return err;
	}
	else
	{
		// do nothing
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
		err = GetInputParameters(l_rxVerifyPerParamMap);
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

		// Check path loss (by ant and freq)
		err = CheckPathLossTableExt( g_WIMAX_Test_ID,
								  l_rxVerifyPerParam.FREQ_MHZ,
								  l_rxVerifyPerParam.RX1,
								  l_rxVerifyPerParam.RX2,
								  l_rxVerifyPerParam.RX3,
								  l_rxVerifyPerParam.RX4,
								  l_rxVerifyPerParam.CABLE_LOSS_DB,
								  l_rxVerifyPerReturn.CABLE_LOSS_DB,
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

#pragma region SETUP VSG FOR SYNCHRONIZATION

		/*-------------------------------*
		 * SETUP VSG FOR SYNCHRONIZATION *
		 *-------------------------------*/
		if ( 0==strcmp(g_WIMAXGlobalSettingParam.WIMAX_TEST_MODE,"LINK") )
		{
			if ( (0==strcmp(l_rxVerifyPerParam.WAVEFORM_FILE_NAME,""))||(0==strcmp(l_rxVerifyPerParam.WAVEFORM_FILE_NAME,"NULL")) )
			{
				// If WIMAX_TEST_MODE = LINK, but the user won't input the WAVEFORM_FILE_NAME name, then must return an error message.
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Since WIMAX_TEST_MODE=LINK, thus WAVEFORM_FILE_NAME can NOT be empty.\n");
				throw logMessage;
			}
			else
			{
#pragma region Setup LP Tester
			   /*---------------------*
				* Load waveform Files *
				*---------------------*/
				char   modFile[MAX_BUFFER_SIZE] = {'\0'};
				if ( 0==strcmp(l_rxVerifyPerParam.WAVEFORM_FILE_NAME,"AUTO") )
				{
					sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, g_WIMAXGlobalSettingParam.WIMAX_WAVEFORM_NAME);
				}
				else
				{
					sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, l_rxVerifyPerParam.WAVEFORM_FILE_NAME);
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
				if( (l_rxVerifyPerParam.RX_POWER_DBM+cableLossDb)>g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX )
				{
					//ERR_VSG_POWER_EXCEED_LIMIT
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", l_rxVerifyPerParam.RX_POWER_DBM+cableLossDb, g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX);
					throw logMessage;
				}
				else
				{
					// do nothing
				}
			
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG port by g_WIMAXGlobalSettingParam.VSG_PORT=[%d].\n", g_WIMAXGlobalSettingParam.VSG_PORT);
				err = ::LP_SetVsg(l_rxVerifyPerParam.FREQ_MHZ*1e6, l_rxVerifyPerParam.RX_POWER_DBM+cableLossDb, g_WIMAXGlobalSettingParam.VSG_PORT);
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

			   /*--------------------------*
				* Send packet for PER Test *
				*--------------------------*/  
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
		}
		else
		{
			// WIMAX_TEST_MODE = NON-LINK, then must turn off the VSG.
			//err = ::LP_EnableVsgRF(0);
			//if ( ERR_OK!=err )
			//{
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
			//	throw logMessage;
			//}
			//else
			//{
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
			//}
		}

		/*---------------------------*
		 * Configure DUT to Receive  *
		 *---------------------------*/
		// Set DUT RF frquency, Rx power, antenna, data rate
		vDUT_ClearParameters(g_WIMAX_Dut);

		if ( 0!=l_rxVerifyPerParam.FRAME_COUNT )
		{
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "FRAME_COUNT",	l_rxVerifyPerParam.FRAME_COUNT );  
		}
		else
		{
			vDUT_AddIntegerParameter(g_WIMAX_Dut, "FRAME_COUNT",	g_WIMAXGlobalSettingParam.PER_WIMAX_PACKETS_NUM ); //when zero, means using default global setting value.  
		}
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "FREQ_MHZ",			l_rxVerifyPerParam.FREQ_MHZ);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "SIG_TYPE",			l_rxVerifyPerParam.SIG_TYPE);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "RATE_ID",			l_rxVerifyPerParam.RATE_ID);
	
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX1",			    l_rxVerifyPerParam.RX1);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX2",				l_rxVerifyPerParam.RX2);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX3",				l_rxVerifyPerParam.RX3);
		vDUT_AddIntegerParameter(g_WIMAX_Dut, "RX4",				l_rxVerifyPerParam.RX4);
		
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "BANDWIDTH_MHZ",		l_rxVerifyPerParam.BANDWIDTH_MHZ);
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "CYCLIC_PREFIX",		l_rxVerifyPerParam.CYCLIC_PREFIX);	
		
		vDUT_AddDoubleParameter (g_WIMAX_Dut, "RX_POWER_DBM",		l_rxVerifyPerParam.RX_POWER_DBM);
		vDUT_AddStringParameter (g_WIMAX_Dut, "WAVEFORM_FILE_NAME",	l_rxVerifyPerParam.WAVEFORM_FILE_NAME);


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
#pragma region Setup LP Tester

		   /*---------------------*
			* Load waveform Files *
			*---------------------*/
			char   modFile[MAX_BUFFER_SIZE] = {'\0'};
			if ( 0==strcmp(l_rxVerifyPerParam.WAVEFORM_FILE_NAME,"AUTO") )
			{
				sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, g_WIMAXGlobalSettingParam.WIMAX_WAVEFORM_NAME);
			}
			else
			{
				sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s", g_WIMAXGlobalSettingParam.WAVEFORM_PATH, l_rxVerifyPerParam.WAVEFORM_FILE_NAME);
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
			if( (l_rxVerifyPerParam.RX_POWER_DBM+cableLossDb)>g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX )
			{
				//ERR_VSG_POWER_EXCEED_LIMIT
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", l_rxVerifyPerParam.RX_POWER_DBM+cableLossDb, g_WIMAXGlobalSettingParam.VSG_MAX_POWER_WIMAX);
				throw logMessage;
			}
			else
			{
				// do nothing
			}
		
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Set VSG port by g_WIMAXGlobalSettingParam.VSG_PORT=[%d].\n", g_WIMAXGlobalSettingParam.VSG_PORT);
			err = ::LP_SetVsg(l_rxVerifyPerParam.FREQ_MHZ*1e6, l_rxVerifyPerParam.RX_POWER_DBM+cableLossDb, g_WIMAXGlobalSettingParam.VSG_PORT);
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

		   /*--------------------------*
			* Send packet for PER Test *
			*--------------------------*/  
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

		//if ( 0!=l_rxVerifyPerParam.FRAME_COUNT )	// 0 means continuous transmit, will skip this step
		//{
		//   /*-----------------*
		//	* Wait for TxDone *
		//	*-----------------*/ 
		//	int timeOutInMs = 0, sleepTime = 50;
		//	while ( timeOutInMs<=(1000*(g_WIMAXGlobalSettingParam.PER_VSG_TIMEOUT_SEC)) )
		//	{
		//		err = ::LP_TxDone();
		//		if (ERR_OK!=err)	// Tx not finish yet...
		//		{
		//			Sleep(sleepTime);
		//			timeOutInMs = timeOutInMs + sleepTime;
		//		}
		//		else	
		//		{	// Tx Done, then break the while loop.
		//			break;
		//		}		
		//	}
		//	if ( timeOutInMs>(1000*(g_WIMAXGlobalSettingParam.PER_VSG_TIMEOUT_SEC)) )	// timeout
		//	{
		//		err = -1;
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] IQTester VSG timeout.\n");
		//		throw logMessage;
		//	}
		//	else
		//	{
		//		// no error, do noting.
		//	}
		//}

#pragma region Retrieve analysis Results
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

		l_rxVerifyPerReturn.RX_POWER_LEVEL = l_rxVerifyPerParam.RX_POWER_DBM;

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
			l_rxVerifyPerReturn.GOOD_PACKETS = goodPackets;
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
			l_rxVerifyPerReturn.TOTAL_PACKETS = totalPackets;
		}

		// Sometime Dut get packets from Air
		// workaround for some dut which returns more ack than packet sent
		if ( goodPackets>totalPackets )	
		{
			goodPackets = totalPackets;
			l_rxVerifyPerReturn.GOOD_PACKETS  = goodPackets;
			l_rxVerifyPerReturn.TOTAL_PACKETS = totalPackets;
		}
		else
		{
			// do nothing	
		}

		badPackets = totalPackets - goodPackets;

		if( totalPackets!=0 )
		{
			l_rxVerifyPerReturn.PER = ((double)(badPackets)/(double)(totalPackets)) * 100.0;
		}
		else	// In this case, totalPackets = 0
		{
			l_rxVerifyPerReturn.PER = 100.0;
		}

		//double _fer = 0.0;
		//err = ::vDUT_GetDoubleReturn (g_WIMAX_Dut, "FER", &_fer);
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_GetDoubleReturn(FER) return error.\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(FER) return OK.\n");
		//}
		//if( err==ERR_OK )
		//{
		//	l_rxVerifyPerReturn.PER = _fer;
		//}
		//else	// In this case, totalPackets = 0
		//{
		//	l_rxVerifyPerReturn.PER = 100.0;
		//}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "RSSI_1", &l_rxVerifyPerReturn.RSSI[0]); 
		if ( ERR_OK!=err )
		{
			err = ERR_OK; // TODO: Since we only report "LOGGER_WARNING", thus must reset err
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the RSSI_1 value.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(RSSI_1) return OK.\n");
		}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "RSSI_2", &l_rxVerifyPerReturn.RSSI[1]); 
		if ( ERR_OK!=err )
		{
			err = ERR_OK; // TODO: Since we only report "LOGGER_WARNING", thus must reset err
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the RSSI_2 value.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(RSSI_2) return OK.\n");
		}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "RSSI_3", &l_rxVerifyPerReturn.RSSI[2]); 
		if ( ERR_OK!=err )
		{
			err = ERR_OK; // TODO: Since we only report "LOGGER_WARNING", thus must reset err
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the RSSI_3 value.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(RSSI_3) return OK.\n");
		}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "RSSI_4", &l_rxVerifyPerReturn.RSSI[3]); 
		if ( ERR_OK!=err )
		{
			err = ERR_OK; // TODO: Since we only report "LOGGER_WARNING", thus must reset err
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the RSSI_4 value.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(RSSI_4) return OK.\n");
		}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "CINR", &l_rxVerifyPerReturn.CINR); 
		if ( ERR_OK!=err )
		{
			err = ERR_OK; // TODO: Since we only report "LOGGER_WARNING", thus must reset err
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the CINR value.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(CINR) return OK.\n");
		}
		
		//Nick add CINR_1, CINR_2, CINR_DELTA_MAX
		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "CINR_1", &l_rxVerifyPerReturn.CINR_1); 
		if ( ERR_OK!=err )
		{
			err = ERR_OK; // TODO: Since we only report "LOGGER_WARNING", thus must reset err
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the CINR_1 value.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(CINR_1) return OK.\n");
		}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "CINR_2", &l_rxVerifyPerReturn.CINR_2); 
		if ( ERR_OK!=err )
		{
			err = ERR_OK; // TODO: Since we only report "LOGGER_WARNING", thus must reset err
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the CINR_2 value.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(CINR_2) return OK.\n");
		}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "CINR_3", &l_rxVerifyPerReturn.CINR_3); 
		if ( ERR_OK!=err )
		{
			err = ERR_OK; // TODO: Since we only report "LOGGER_WARNING", thus must reset err
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the CINR_3 value.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(CINR_3) return OK.\n");
		}

		err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "CINR_4", &l_rxVerifyPerReturn.CINR_4); 
		if ( ERR_OK!=err )
		{
			err = ERR_OK; // TODO: Since we only report "LOGGER_WARNING", thus must reset err
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the CINR_4 value.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(CINR_4) return OK.\n");
		}

		//Nick add for picking max & min CINR than have a max Delta
		CINRDelta[0]=l_rxVerifyPerReturn.CINR_1;
		CINRDelta[1]=l_rxVerifyPerReturn.CINR_2;
		CINRDelta[2]=l_rxVerifyPerReturn.CINR_3;
		CINRDelta[3]=l_rxVerifyPerReturn.CINR_4;
		for (int i=0;i<4;i++)
		{
			if(CINRDelta[i]==NA_NUMBER)
			{
			}else
			{
				CINRDeltaMax=max(CINRDeltaMax,CINRDelta[i]);
				CINRDeltaMin=min(CINRDeltaMin,CINRDelta[i]);
			}
			
		}
		
		l_rxVerifyPerReturn.CINR_DELTA_MAX= fabs(CINRDeltaMax-CINRDeltaMin);
		/*
		CINRDelta[0]=abs(l_rxVerifyPerReturn.CINR_1-l_rxVerifyPerReturn.CINR_2);
		CINRDelta[1]=abs(l_rxVerifyPerReturn.CINR_1-l_rxVerifyPerReturn.CINR_3);
		CINRDelta[2]=abs(l_rxVerifyPerReturn.CINR_1-l_rxVerifyPerReturn.CINR_4);
		CINRDelta[3]=abs(l_rxVerifyPerReturn.CINR_2-l_rxVerifyPerReturn.CINR_3);
		CINRDelta[4]=abs(l_rxVerifyPerReturn.CINR_2-l_rxVerifyPerReturn.CINR_4);
		CINRDelta[5]=abs(l_rxVerifyPerReturn.CINR_3-l_rxVerifyPerReturn.CINR_4);
		CINRDeltaTemp=CINRDelta[0];
		for (int i=1;i<6;i++)
		{
			if(CINRDeltaTemp<CINRDelta[i])
			{
				CINRDeltaTemp=CINRDelta[i];
			}
		}
		*/
		
		//err = ::vDUT_GetDoubleReturn(g_WIMAX_Dut, "CINR_DELTA", &l_rxVerifyPerReturn.CINR_DELTA_MAX); 
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] This DUT doesn't report the CINR_DELTA_MAX value.\n");
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetDoubleReturn(CINR_DELTA_MAX) return OK.\n");
		//}

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

		/*----------------------*
		 *  Function Completed  *
		 *----------------------*/
		if ( ERR_OK==err )
		{
			sprintf_s(l_rxVerifyPerReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_rxVerifyPerReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_rxVerifyPerReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
	}

	// This is a special case, only when some error occur before the RX_STOP. 
	// This case will take care by the error handling, but must do RX_STOP manually.
	if ( g_vDutRxActived )
	{
		vDUT_Run(g_WIMAX_Dut, "RX_STOP");
	}
	else
	{
		// do nothing
	}

	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	ReturnTestResults(l_rxVerifyPerReturnMap);


	return err;
}

void InitializeRXVerifyPerContainers(void)
{
   /*-----------------*
	* Input Paramters *
	*-----------------*/
	l_rxVerifyPerParamMap.clear();

	WIMAX_SETTING_STRUCT setting;
	setting.unit = "";
	setting.helpText = "";
	

	l_rxVerifyPerParam.FREQ_MHZ = 3500;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.FREQ_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "Channel center frequency in MHz";
		l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("FREQ_MHZ", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_rxVerifyPerParam.SIG_TYPE = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerifyPerParam.SIG_TYPE))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerifyPerParam.SIG_TYPE;
        setting.unit        = "";
        setting.helpText    = "Indicates the type of signal. 1=downlink signal, 2=uplink signal, 0=auto-detect\r\nDefault: 0.";
        l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("SIG_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerifyPerParam.BANDWIDTH_MHZ = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyPerParam.BANDWIDTH_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerifyPerParam.BANDWIDTH_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20\r\nDefault: 0.";
        l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("BANDWIDTH_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerifyPerParam.CYCLIC_PREFIX = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyPerParam.CYCLIC_PREFIX))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerifyPerParam.CYCLIC_PREFIX;
        setting.unit        = "";
        setting.helpText    = "Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32\r\nDefault: 0.";
        l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CYCLIC_PREFIX", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerifyPerParam.RATE_ID = 0;
    setting.type = WIMAX_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerifyPerParam.RATE_ID))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerifyPerParam.RATE_ID;
        setting.unit        = "";
        setting.helpText    = "Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and  correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively\r\nDefault: 0.";
        l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RATE_ID", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerifyPerParam.NUM_SYMBOLS = 0;
    setting.type = WIMAX_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyPerParam.NUM_SYMBOLS))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerifyPerParam.NUM_SYMBOLS;
        setting.unit        = "";
        setting.helpText    = "Number of OFDM symbols in burst. Default: 0 (auto detect).";
        l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("NUM_SYMBOLS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyPerParam.FRAME_COUNT = 0;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.FRAME_COUNT))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.FRAME_COUNT;
		setting.unit        = "";
		setting.helpText    = "Number of frame count, default=0, means using default global setting value.";
		l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("FRAME_COUNT", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_rxVerifyPerParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_rxVerifyPerParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_rxVerifyPerParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_rxVerifyPerParam.RX_POWER_DBM = -65.0;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerParam.RX_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port";
		l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX_POWER_DBM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX1 = 1;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX1))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX1;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 1 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX1", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX2 = 0;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX2))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX2;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 2 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX2", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX3 = 0;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX3))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX3;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 3 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX3", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX4 = 0;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX4))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX4;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 4 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX4", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxVerifyPerParam.WAVEFORM_FILE_NAME, MAX_BUFFER_SIZE, "AUTO");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.WAVEFORM_FILE_NAME))    // Type_Checking
    {
        setting.value = (void*)l_rxVerifyPerParam.WAVEFORM_FILE_NAME;
        setting.unit  = "";
        setting.helpText = "For 802.16e analysis, a Waveform name (*.mod) is required for Rx test. If the value equal to AUTO that means using GLOBAL_SETTINGS->WIMAX_WAVEFORM_NAME.";
        l_rxVerifyPerParamMap.insert( pair<string, WIMAX_SETTING_STRUCT>("WAVEFORM_FILE_NAME", setting) );
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
	l_rxVerifyPerReturnMap.clear();

	l_rxVerifyPerReturn.TOTAL_PACKETS = (int)NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerReturn.TOTAL_PACKETS))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.TOTAL_PACKETS;
		setting.unit        = "";
		setting.helpText    = " TOTAL_PACKETS is the number of total packets.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("TOTAL_PACKETS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.GOOD_PACKETS = (int)NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerReturn.GOOD_PACKETS))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.GOOD_PACKETS;
		setting.unit        = "";
		setting.helpText    = " GOOD_PACKETS is the number of good packets that reported from Dut.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("GOOD_PACKETS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.PER = NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.PER))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.PER;
		setting.unit        = "%";
		setting.helpText    = "Average PER over received packets.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("PER", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_rxVerifyPerReturn.RSSI[i] = NA_NUMBER;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_rxVerifyPerReturn.RSSI[i]))    // Type_Checking
		{
			setting.value = (void*)&l_rxVerifyPerReturn.RSSI[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "RSSI_%d", i+1);
			setting.unit        = "";
			setting.helpText    = "Average RSSI over received packets.";
			l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_rxVerifyPerReturn.CINR = NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.CINR))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.CINR;
		setting.unit        = "dB";
		setting.helpText    = "The test result of carrier to interference noise ratio.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CINR", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	
	//Nick add CINR_1, CINR_2,& CINR_DELTA
	l_rxVerifyPerReturn.CINR_1 =  NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.CINR_1))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.CINR_1;
		setting.unit        = "dB";
		setting.helpText    = "The test result of carrier to interference noise ratio for antenna1.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CINR_1", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.CINR_2 =  NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.CINR_2))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.CINR_2;
		setting.unit        = "dB";
		setting.helpText    = "The test result of carrier to interference noise ratio for antenna2.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CINR_2", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	
	l_rxVerifyPerReturn.CINR_3 =  NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.CINR_3))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.CINR_3;
		setting.unit        = "dB";
		setting.helpText    = "The test result of carrier to interference noise ratio for antenna3.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CINR_3", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.CINR_4 =  NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.CINR_4))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.CINR_4;
		setting.unit        = "dB";
		setting.helpText    = "The test result of carrier to interference noise ratio for antenna4.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CINR_4", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	
	
	l_rxVerifyPerReturn.CINR_DELTA_MAX = NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.CINR_DELTA_MAX))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.CINR_DELTA_MAX;
		setting.unit        = "dB";
		setting.helpText    = "The test result of carrier to interference noise ratio for the max delta between antenna1 to antenna4.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("CINR_DELTA_MAX", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.RX_POWER_LEVEL = NA_NUMBER;
	setting.type = WIMAX_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.RX_POWER_LEVEL))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.RX_POWER_LEVEL;
		setting.unit        = "dBm";
		setting.helpText    = "RX power level for PER";
		l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("RX_POWER_LEVEL", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_rxVerifyPerReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIMAX_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_rxVerifyPerReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_rxVerifyPerReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_rxVerifyPerReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_rxVerifyPerReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return;
}

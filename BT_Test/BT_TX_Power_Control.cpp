#include "stdafx.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "math.h"

using namespace std;

// These global variables are declared in BT_Test_Internal.cpp
extern TM_ID                 g_BT_Test_ID;    
extern vDUT_ID               g_BT_Dut;

// This global variable is declared in BT_Global_Setting.cpp
extern BT_GLOBAL_SETTING g_BTGlobalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, BT_SETTING_STRUCT> l_txPowerControlParamMap;

// Return Value Container
map<string, BT_SETTING_STRUCT> l_txPowerControlReturnMap;

struct tagParam
{
    // Mandatory Parameters
	int       FREQ_MHZ;								/*! The center frequency (MHz). */				
	char      PACKET_TYPE[MAX_BUFFER_SIZE];			/*! The pack type to verify power control. */
	int		  PACKET_LENGTH;						/*! The number of packet to verify power control. */
	double    CABLE_LOSS_DB;						/*! The path loss of test system. */
	double    SAMPLING_TIME_US;						/*! The sampling time to verify power control. */ 

	double	  STEP_POWER;							/*! The power of step for adjust the outpower in dB. Default: 1 dB. */
    double    MAX_POWER_DBM;                        /*! The MAX output power to verify Power step. */
	int       STEP_NUMBER;							/*! The number of step for Power test, from 1 to MAX_POWER_STEP. Default: 1. */


} l_txPowerControlParam;

struct tagReturn
{   
	// <Perform LP_AnalyzePower>
	double   POWER_AVERAGE[MAX_POWER_STEP];
	double   POWER_STEP_SIZE[MAX_POWER_STEP];
	double   MAX_STEP_SIZE;
	double   MIN_STEP_SIZE;

	//double   POWER_PEAK[MAX_POWER_STEP];
	double   TARGET_POWER[MAX_POWER_STEP];

	double   CABLE_LOSS_DB;						/*! The path loss of the test system. */
    char     ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txPowerControlReturn;
#pragma endregion

#ifndef WIN32
int initTXPowerControlContainers = InitializeTXPowerControlContainers();
#endif

int ClearTxPowerControlReturn(void)
{
	l_txPowerControlParamMap.clear();
	l_txPowerControlReturnMap.clear();
	return 0;
}

//! BT Tx Power Control
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


BT_TEST_API int BT_TX_Power_Control(void)
{
    int    err = ERR_OK;

    bool   analysisOK = false, captureOK  = false, vDutActived = false;       
	int	   packetLength = 0;
	int    avgIteration = 0;
	int    dummyValue   = 0; 
	double dummyMax     = 0, dummyMin = 0;
	double samplingTimeUs = 0, cableLossDb = 0;
	char   vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};   
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_txPowerControlReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_txPowerControlParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_txPowerControlReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< vector<double> >
	 *-----------------------------------------------------------------------------------------*/
	vector< vector<double> >    powerAvEachBurst (MAX_POWER_STEP, vector<double>(g_BTGlobalSettingParam.TX_POWER_CONTROL_AVERAGE));
	vector< vector<double> >    powerPkEachBurst (MAX_POWER_STEP, vector<double>(g_BTGlobalSettingParam.TX_POWER_CONTROL_AVERAGE));
	vector< vector<double> >    targetPower		 (MAX_POWER_STEP, vector<double>(g_BTGlobalSettingParam.TX_POWER_CONTROL_AVERAGE));


	try 
	{
	   /*-----------------------------------------------------------*
		*   Both g_BT_Test_ID and g_BT_Dut need to be valid (>=0)   *
		*-----------------------------------------------------------*/
		TM_ClearReturns(g_BT_Test_ID);
		if( g_BT_Test_ID<0 || g_BT_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] BT_Test_ID or BT_Dut not valid. BT_Test_ID = %d and BT_Dut = %d.\n", g_BT_Test_ID, g_BT_Dut);
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_txPowerControlParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not completed.\n");
			throw logMessage;
		}			

		// Check path loss (by ant and freq)
		if ( 0==l_txPowerControlParam.CABLE_LOSS_DB )
		{
			err = TM_GetPathLossAtFrequency(g_BT_Test_ID, l_txPowerControlParam.FREQ_MHZ, &l_txPowerControlParam.CABLE_LOSS_DB, 0, TX_TABLE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Failed to get CABLE_LOSS_DB from path loss table.\n", err);
				throw logMessage;
			}				
		}
		else
		{
			// do nothing
		}

		for (int i=0;i<MAX_POWER_STEP;i++)
		{
			l_txPowerControlReturn.POWER_AVERAGE[i]   = NA_NUMBER;
			l_txPowerControlReturn.POWER_STEP_SIZE[i] = NA_NUMBER;
			l_txPowerControlReturn.TARGET_POWER[i]    = NA_NUMBER;
		}
		l_txPowerControlReturn.MAX_STEP_SIZE = NA_NUMBER;
		l_txPowerControlReturn.MIN_STEP_SIZE = NA_NUMBER;

		/*------------------------------*
		 * Control Power Level for Test *
		 *------------------------------*/
		for(int levelIndex=0;levelIndex<l_txPowerControlParam.STEP_NUMBER;levelIndex++)
		{
		   /*----------------------------*
			* Disable VSG output signal  *
			*----------------------------*/
			// make sure no signal is generated by the VSG
			err = ::LP_EnableVsgRF(0);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
			}

#pragma region Configure DUT to transmit
			/*-------------------------------------------*
			 * Configure DUT to transmit - PRBS9 Pattern *
			 *-------------------------------------------*/
			// Set DUT RF frequency, tx power, data rate
			// And clear vDUT parameters at the beginning.
			vDUT_ClearParameters(g_BT_Dut);

			vDUT_AddIntegerParameter(g_BT_Dut, "FREQ_MHZ",			l_txPowerControlParam.FREQ_MHZ);	
			 // Description				Enum in the IQFact
			 // 0x00 8-bit Pattern		ZERO =0: all zero
			 // 0xFF 8-bit Pattern      ONE1 =1: all ones
			 // 0xAA 8-bit Pattern      HEX_A=2: 1010
			 // 0xF0 8-bit Pattern		HEX_F0=3: 11110000
			 // PRBS9 Pattern           RANDOM=4;  PRBS=0
			vDUT_AddIntegerParameter(g_BT_Dut, "MODULATION_TYPE",   BT_PATTERN_PRBS);	//{0x04, "PRBS9 Pattern"}
			 // {0x00, "ACL  EDR"},
			 // {0x01, "ACL  Basic"},
			 // {0x02, "eSCO EDR"},
			 // {0x03, "eSCO Basic"},
			 // {0x04, "SCO  Basic"}
			vDUT_AddIntegerParameter(g_BT_Dut, "LOGIC_CHANNEL",     1);	//{0x01, "ACL Basic"}
			vDUT_AddStringParameter (g_BT_Dut, "PACKET_TYPE",	    l_txPowerControlParam.PACKET_TYPE);

			// Check packet length 		
			if (0==l_txPowerControlParam.PACKET_LENGTH)
			{
				GetPacketLength("BER", "PACKETS_LENGTH", l_txPowerControlParam.PACKET_TYPE, &packetLength);
			}
			else	
			{
				packetLength = l_txPowerControlParam.PACKET_LENGTH;
			}
			vDUT_AddIntegerParameter(g_BT_Dut, "PACKET_LENGTH",     packetLength);
			vDUT_AddIntegerParameter(g_BT_Dut, "TX_POWER_LEVEL",	(int)(l_txPowerControlParam.MAX_POWER_DBM-(l_txPowerControlParam.STEP_POWER*levelIndex)) );

			err = vDUT_Run(g_BT_Dut, "QUERY_POWER_DBM");
			if ( ERR_OK!=err )
			{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
				err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					throw logMessage;
				}
				else	// Return error message 
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(QUERY_POWER_DBM) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				// do nothing
			}

			err = vDUT_GetDoubleReturn(g_BT_Dut, "POWER_DBM", &l_txPowerControlReturn.TARGET_POWER[levelIndex]);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_GetDoubleReturn(POWER_DBM) return error.\n");
				throw logMessage;
			}

			err = vDUT_Run(g_BT_Dut, "TX_START");
			if ( ERR_OK!=err )
			{	
				vDutActived = false;
				// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
				err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					throw logMessage;
				}
				else	// Return error message
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_START) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				vDutActived = true;
			}

			// Delay for DUT settle
			if (0!=g_BTGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
			{
				Sleep(g_BTGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
			}
			else
			{
				// do nothing
			}
#pragma endregion

#pragma region Setup LP Tester and Capture
			/*--------------------*
			 * Setup IQtester VSA *
			 *--------------------*/
			double peakToAvgRatio = g_BTGlobalSettingParam.IQ_P_TO_A_BDR;
			cableLossDb = l_txPowerControlParam.CABLE_LOSS_DB;

			err = LP_SetVsaBluetooth(  l_txPowerControlParam.FREQ_MHZ*1e6,
									   l_txPowerControlReturn.TARGET_POWER[levelIndex]-cableLossDb+peakToAvgRatio,
									   g_BTGlobalSettingParam.VSA_PORT,
									   g_BTGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
									   g_BTGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
									 );
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to setup VSA.\n");
				throw logMessage;
			}

			// Check Capture Time 
			if (0==l_txPowerControlParam.SAMPLING_TIME_US)
			{
				samplingTimeUs = PacketTypeToSamplingTimeUs(l_txPowerControlParam.PACKET_TYPE);		
			}
			else	// SAMPLING_TIME_US != 0
			{
				samplingTimeUs = l_txPowerControlParam.SAMPLING_TIME_US;
			}
#pragma endregion

			/*--------------------------------*
			 * Start "while" loop for average *
			 *--------------------------------*/
			avgIteration = 0;
			while ( avgIteration<g_BTGlobalSettingParam.TX_POWER_CONTROL_AVERAGE )
			{				
				analysisOK = false;
				captureOK  = false; 

			   /*----------------------------*
				* Perform Nnormal VSA capture *
				*----------------------------*/
				err = LP_VsaDataCapture( samplingTimeUs/1000000, g_BTGlobalSettingParam.VSA_TRIGGER_TYPE );   
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to capture signal at %d MHz.\n", l_txPowerControlParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] LP_VsaDataCapture() at %d MHz return OK.\n", l_txPowerControlParam.FREQ_MHZ);
				}

				/*--------------*
				 *  Capture OK  *
				 *--------------*/
				captureOK = true;
				if (1==g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
				{
					// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_PWR_CONTROL_SaveAlways", l_txPowerControlParam.FREQ_MHZ, l_txPowerControlParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
				}
				else
				{
					// do nothing
				}

				/*--------------------------*
				 *  Perform Power analysis  *
				 *--------------------------*/
				double dummy_T_INTERVAL      = 3.2;
				double dummy_MAX_POW_DIFF_DB = 15.0;  
				err = LP_AnalyzePower( dummy_T_INTERVAL/1000000, dummy_MAX_POW_DIFF_DB );
				if (ERR_OK!=err)
				{	// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_PWR_CONTROL_AnalysisFailed", l_txPowerControlParam.FREQ_MHZ, l_txPowerControlParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] LP_AnalyzePower() return error.\n");
					throw logMessage;
				}
				else
				{
					// do nothing
				}											

#pragma region Retrieve analysis Results
				/*-----------------------------*
				 *  Retrieve analysis Results  *
				 *-----------------------------*/
				analysisOK = true;

				// powerAvEachBurst  
				powerAvEachBurst[levelIndex][avgIteration] = LP_GetScalarMeasurement("P_av_no_gap_all_dBm", 0);
				if ( -99.00 >= powerAvEachBurst[levelIndex][avgIteration] )
				{
					analysisOK = false;
					l_txPowerControlReturn.POWER_AVERAGE[levelIndex] = NA_NUMBER;
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_PWR_CONTROL_AnalysisFailed", l_txPowerControlParam.FREQ_MHZ, l_txPowerControlParam.PACKET_TYPE);
					BTSaveSigFile(sigFileNameBuffer);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_av_no_gap_all_dBm) return error.\n");
					throw logMessage;
				}
				else
				{
					powerAvEachBurst[levelIndex][avgIteration] = powerAvEachBurst[levelIndex][avgIteration] + cableLossDb;					
				}

				// powerPkEachBurst  
				//powerPkEachBurst[levelIndex][avgIteration] = LP_GetScalarMeasurement("P_pk_each_burst_dBm", 0);
				//if ( -99.00 >= powerPkEachBurst[levelIndex][avgIteration] )
				//{
				//	analysisOK = false;
				//	l_txPowerControlReturn.POWER_PEAK[levelIndex] = NA_NUMBER;
				//	sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s", "BT_TX_PWR_CONTROL_AnalysisFailed", l_txPowerControlParam.FREQ_MHZ, l_txPowerControlParam.PACKET_TYPE);
				//	BTSaveSigFile(sigFileNameBuffer);
				//	err = -1;
				//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_pk_each_burst_dBm) return error.\n");
				//	throw logMessage;
				//}
				//else
				//{
				//	powerPkEachBurst[levelIndex][avgIteration] = powerPkEachBurst[levelIndex][avgIteration] + cableLossDb;
				//}

				avgIteration++;
#pragma endregion
			}	// End - avgIteration

#pragma region Averaging and Saving Test Result
		   /*----------------------------------*
			* Averaging and Saving Test Result *
			*----------------------------------*/
			if ( (ERR_OK==err) && captureOK && analysisOK )
			{
				// Average Power test result
				err = ::AverageTestResult(&powerAvEachBurst[levelIndex][0], avgIteration, LOG_10, l_txPowerControlReturn.POWER_AVERAGE[levelIndex], dummyMax, dummyMin);
				// Peak Power test result
				//err = ::AverageTestResult(&powerPkEachBurst[levelIndex][0], avgIteration, LOG_10, dummyMax, l_txPowerControlReturn.POWER_PEAK[levelIndex], dummyMin);
			}
			else
			{
				// do nothing
			}
#pragma endregion

		   /*-----------*
			*  Tx Stop  *
			*-----------*/
			err = vDUT_Run(g_BT_Dut, "TX_STOP");		
			if ( ERR_OK!=err )
			{	// Check vDUT return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					throw logMessage;
				}
				else	// Return error message
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				vDutActived = false;
			}
		}	// End - For Loop


		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			// Return Path Loss (dB)
			l_txPowerControlReturn.CABLE_LOSS_DB = l_txPowerControlParam.CABLE_LOSS_DB;

			for (int i=0;i<(MAX_POWER_STEP-1);i++)
			{
				l_txPowerControlReturn.POWER_STEP_SIZE[i] = NA_NUMBER;

				if (l_txPowerControlReturn.POWER_AVERAGE[i+1]!=NA_NUMBER)
				{
					// the step size should be absolute value.
					if (l_txPowerControlParam.STEP_POWER < 0) { // In case Step Up
						l_txPowerControlReturn.POWER_STEP_SIZE[i] = l_txPowerControlReturn.POWER_AVERAGE[i+1] - l_txPowerControlReturn.POWER_AVERAGE[i];
					} else {
						l_txPowerControlReturn.POWER_STEP_SIZE[i] = l_txPowerControlReturn.POWER_AVERAGE[i] - l_txPowerControlReturn.POWER_AVERAGE[i+1];
					}
					if ((l_txPowerControlReturn.MAX_STEP_SIZE == NA_NUMBER)||(l_txPowerControlReturn.POWER_STEP_SIZE[i]>l_txPowerControlReturn.MAX_STEP_SIZE)) {
						l_txPowerControlReturn.MAX_STEP_SIZE = l_txPowerControlReturn.POWER_STEP_SIZE[i];
					}
					if ((l_txPowerControlReturn.MIN_STEP_SIZE == NA_NUMBER)||(l_txPowerControlReturn.POWER_STEP_SIZE[i]<l_txPowerControlReturn.MIN_STEP_SIZE)) {
						l_txPowerControlReturn.MIN_STEP_SIZE = l_txPowerControlReturn.POWER_STEP_SIZE[i];
					}
				}
				else
				{
					// do nothing
				}
			}

			sprintf_s(l_txPowerControlReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txPowerControlReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txPowerControlReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txPowerControlReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
    }

	// This is a special case, only when some error occurs before the TX_STOP. 
	// This case will be handled by error handling, but the TX_STOP must be done manually.
	
	if ( vDutActived )
	{
		vDUT_Run(g_BT_Dut, "TX_STOP");
	}
	else
	{
		// do nothing
	}

    // Free memory
    powerAvEachBurst.clear();
	powerPkEachBurst.clear();
	targetPower.clear();


	return err;
}

int InitializeTXPowerControlContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txPowerControlParamMap.clear();

    BT_SETTING_STRUCT setting;

    l_txPowerControlParam.FREQ_MHZ = 2402;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txPowerControlParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txPowerControlParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz";
        l_txPowerControlParamMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	//{0x00, "NULL"},
	//{0x01, "POLL"},
	//{0x02, "FHS"},
	//{0x03, "DM1"},
	//{0x04, "DH1 / 2DH1"},
	//{0x05, "HV1"},
	//{0x06, "HV2 / 2-EV3"},
	//{0x07, "HV3 / EV3 / 3-EV3"},
	//{0x08, "DV / 3DH1"},
	//{0x09, "AUX1 / PS"},
	//{0x0A, "DM3 / 2DH3"},
	//{0x0B, "DH3 / 3DH3"},
	//{0x0C, "EV4 / 2-EV5"},
	//{0x0D, "EV5 / 3-EV5"},
	//{0x0E, "DM5 / 2DH5"},
	//{0x0F, "DH5 / 3DH5"}

    strcpy_s(l_txPowerControlParam.PACKET_TYPE, MAX_BUFFER_SIZE, "1DH1");
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txPowerControlParam.PACKET_TYPE))    // Type_Checking
    {
        setting.value       = (void*)l_txPowerControlParam.PACKET_TYPE;
        setting.unit        = "";
        setting.helpText    = "Sets the packet type, the type can be 1DH1, 1DH3, 1DH5. default is 1DH1";
        l_txPowerControlParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txPowerControlParam.PACKET_LENGTH = 0;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txPowerControlParam.PACKET_LENGTH))    // Type_Checking
    {
        setting.value       = (void*)&l_txPowerControlParam.PACKET_LENGTH;
        setting.unit        = "";
        setting.helpText    = "The number of packet to verify Power_Control.";
        l_txPowerControlParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txPowerControlParam.CABLE_LOSS_DB = 0.0;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txPowerControlParam.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txPowerControlParam.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester.";
		l_txPowerControlParamMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_txPowerControlParam.SAMPLING_TIME_US = 0;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txPowerControlParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&l_txPowerControlParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_txPowerControlParamMap.insert( pair<string,BT_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txPowerControlParam.STEP_NUMBER = 8;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txPowerControlParam.STEP_NUMBER))    // Type_Checking
    {
        setting.value       = (void*)&l_txPowerControlParam.STEP_NUMBER;
        setting.unit        = "";
        setting.helpText    = "The number of step for Power test, from 1 to MAX_POWER_STEP. Default: 1.";
        l_txPowerControlParamMap.insert( pair<string,BT_SETTING_STRUCT>("STEP_NUMBER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txPowerControlParam.STEP_POWER = 1.00;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txPowerControlParam.STEP_POWER))    // Type_Checking
    {
        setting.value       = (void*)&l_txPowerControlParam.STEP_POWER;
        setting.unit        = "dB";
        setting.helpText    = "The power of step for adjust the outpower in dB. Default: 2 dB.";
        l_txPowerControlParamMap.insert( pair<string,BT_SETTING_STRUCT>("STEP_POWER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txPowerControlParam.MAX_POWER_DBM = 4.0;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txPowerControlParam.MAX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txPowerControlParam.MAX_POWER_DBM;
        setting.unit        = "";
        setting.helpText    = "MAX power level at DUT antenna port for Power Step test";
        l_txPowerControlParamMap.insert( pair<string,BT_SETTING_STRUCT>("MAX_POWER_DBM", setting) );
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
    l_txPowerControlReturnMap.clear();

	// <Perform LP_AnalyzePower>
    for (int i=0;i<MAX_POWER_STEP;i++)
    {
		l_txPowerControlReturn.POWER_AVERAGE[i] = NA_NUMBER;
        setting.type = BT_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txPowerControlReturn.POWER_AVERAGE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txPowerControlReturn.POWER_AVERAGE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_AVERAGE_LEVEL_%d", i);
            setting.unit        = "dBm";
            setting.helpText    = "Average power in dBm.";
            l_txPowerControlReturnMap.insert( pair<string,BT_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<(MAX_POWER_STEP-1);i++)
    {
		l_txPowerControlReturn.POWER_STEP_SIZE[i] = NA_NUMBER;
        setting.type = BT_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txPowerControlReturn.POWER_STEP_SIZE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txPowerControlReturn.POWER_STEP_SIZE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_STEP_SIZE_LEVEL_%d", i);
            setting.unit        = "dB";
            setting.helpText    = "The power step size between each level in dB.";
            l_txPowerControlReturnMap.insert( pair<string,BT_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

	l_txPowerControlReturn.MAX_STEP_SIZE = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txPowerControlReturn.MAX_STEP_SIZE))    // Type_Checking
	{
		setting.value = (void*)&l_txPowerControlReturn.MAX_STEP_SIZE;
		setting.unit        = "dB";
		setting.helpText    = "The MAX. power step size between each level in dB.";
		l_txPowerControlReturnMap.insert( pair<string,BT_SETTING_STRUCT>("MAX_STEP_SIZE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txPowerControlReturn.MIN_STEP_SIZE = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txPowerControlReturn.MIN_STEP_SIZE))    // Type_Checking
	{
		setting.value = (void*)&l_txPowerControlReturn.MIN_STEP_SIZE;
		setting.unit        = "dB";
		setting.helpText    = "The MIN. power step size between each level in dB.";
		l_txPowerControlReturnMap.insert( pair<string,BT_SETTING_STRUCT>("MIN_STEP_SIZE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

  //  for (int i=0;i<MAX_POWER_STEP;i++)
  //  {
		//l_txPowerControlReturn.POWER_PEAK[i] = NA_NUMBER;
  //      setting.type = BT_SETTING_TYPE_DOUBLE;
  //      if (sizeof(double)==sizeof(l_txPowerControlReturn.POWER_PEAK[i]))    // Type_Checking
  //      {
  //          setting.value = (void*)&l_txPowerControlReturn.POWER_PEAK[i];
  //          char tempStr[MAX_BUFFER_SIZE];
  //          sprintf_s(tempStr, "POWER_PEAK_LEVEL_%d", i);
  //          setting.unit        = "dBm";
  //          setting.helpText    = "Peak power in dBm.";
  //          l_txPowerControlReturnMap.insert( pair<string,BT_SETTING_STRUCT>(tempStr, setting) );
  //      }
  //      else    
  //      {
  //          printf("Parameter Type Error!\n");
  //          exit(1);
  //      }
  //  }

    for (int i=0;i<MAX_POWER_STEP;i++)
    {
		l_txPowerControlReturn.TARGET_POWER[i] = NA_NUMBER;
        setting.type = BT_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txPowerControlReturn.TARGET_POWER[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txPowerControlReturn.TARGET_POWER[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "TARGET_POWER_LEVEL_%d", i);
            setting.unit        = "dBm";
            setting.helpText    = "Expected target power dBm at DUT antenna port.";
            l_txPowerControlReturnMap.insert( pair<string,BT_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

	l_txPowerControlReturn.CABLE_LOSS_DB = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txPowerControlReturn.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txPowerControlReturn.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_txPowerControlReturnMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
    
	// Error Message Return String
    l_txPowerControlReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txPowerControlReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txPowerControlReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_txPowerControlReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}


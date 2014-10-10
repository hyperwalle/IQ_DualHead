#include "stdafx.h"
#include "TestManager.h"
#include "FM_Test.h"
#include "FM_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "math.h"

using namespace std;

// These global variables are declared in FM_Test_Internal.cpp
extern TM_ID                 g_FM_Test_ID;    
extern vDUT_ID               g_FM_Dut;

// This global variable is declared in FM_Global_Setting.cpp
extern FM_GLOBAL_SETTING g_FMGlobalSettingParam;


#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, FM_SETTING_STRUCT> l_rxVerify_RSSI_ParamMap;

// Return Value Container
map<string, FM_SETTING_STRUCT> l_rxVerify_RSSI_ReturnMap;

struct tagParam
{
    // Mandatory Parameters
	int    CARRIER_FREQ_HZ;							/*! The FM Carrier frequency (Hz). */
	double    CARRIER_POWER_DBM;					/*! The FM Carrier Power (dBm). Max VSG Power Setting = -40 dBm*/
	
	double    CABLE_LOSS_DB;						/*! The path loss of test system. */
} l_rxVerify_RSSI_Param;

struct tagReturn
{   
	double	 RSSI_DBM;

	double   CABLE_LOSS_DB;						/*! The path loss of the test system. */
    char     ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_rxVerify_RSSI_Return;

#pragma endregion

void ClearRxVerify_RSSI_Return(void)
{
	l_rxVerify_RSSI_ParamMap.clear();
	l_rxVerify_RSSI_ReturnMap.clear();
}

FM_TEST_API int FM_RX_Verify_RSSI(void)
{
    int    err = ERR_OK;

    bool   analysisOK = false, captureOK  = false, vDutActive = false, vDUTStopped = false;
    int    dummyValue   = 0;
	double dummyMax     = 0;
	double dummyMin     = 0;
	double samplingTimemS = 0, cableLossDb = 0;
	char   vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Containers *
     *---------------------------------------*/
	ClearReturnParameters(l_rxVerify_RSSI_ReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_FM_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_rxVerify_RSSI_ParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_FM_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_rxVerify_RSSI_ReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try 
	{
	   /*-----------------------------------------------------------*
		* Both g_FM_Test_ID and g_FM_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		TM_ClearReturns(g_FM_Test_ID);
		if( g_FM_Test_ID<0 || g_FM_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] FM_Test_ID or FM_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_rxVerify_RSSI_ParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Input parameters are not completed.\n");
			throw logMessage;
		}		

		// Check path loss (by ant and freq)
		if ( 0==l_rxVerify_RSSI_Param.CABLE_LOSS_DB )
		{
			err = TM_GetPathLossAtFrequency(g_FM_Test_ID, l_rxVerify_RSSI_Param.CARRIER_FREQ_HZ, &l_rxVerify_RSSI_Param.CABLE_LOSS_DB, 0, RX_TABLE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Failed to get CABLE_LOSS_DB from path loss table.\n", err);
				throw logMessage;
			}				
		}
		else
		{
			// do nothing
		}

#pragma region LP FM VSG Setup and transmit 
		/*-----------------------------------*
		 * Configure LP FM VSG  to transmit  *
		 *-----------------------------------*/
		//Setup VSG	
		int totalFmDeviation = 75000;
		int modulationEnable = 0;
		int stereoEnable = 0;
		int stereoChannelRelation = 0;
		int peakPilotDeviation = 7500;
		int audioDeviationPercent = 100;
		int rdsEnable = 0;
		int rdsDeviationHz = 7500;
		int preEmphasisTimeConstant = PRE_EMPHASIS_NONE;
		char* rdsTransmitData = "LitepointStation";

		double audioToneFrequency = 1000;
		double compensatedCarrierPowerdBm;
		

		compensatedCarrierPowerdBm = l_rxVerify_RSSI_Param.CARRIER_POWER_DBM + l_rxVerify_RSSI_Param.CABLE_LOSS_DB;
		
		if(-40 < compensatedCarrierPowerdBm)
		{
			compensatedCarrierPowerdBm = -40;
		}
		else
		{
			//Do Nothing
		}

		// Clear Previously set audio tones, especially needed before setVSG if transition from Stereo to Mono mode test.
		err = LP_FM_DeleteAudioTones();
		if( ERR_OK!=err )	// VSG Audio Tone setup failed
		{
			// Fail Capture
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Failed to Clear Audio Tones\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[FM] LP_FM_DeleteAudioTones()return OK.\n");
		}

		err = LP_FM_SetVsg(l_rxVerify_RSSI_Param.CARRIER_FREQ_HZ,
						   compensatedCarrierPowerdBm,
						   modulationEnable,
						   totalFmDeviation, 
						   stereoEnable, 
						   peakPilotDeviation,
						   rdsEnable, 
						   rdsDeviationHz, 
						   preEmphasisTimeConstant, 
						   rdsTransmitData);

		if( ERR_OK!=err )	// VSG Setup failed
		{
			// Fail VSG Setup
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Failed to Setup LP FM VSG\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[FM] LP_FM_SetVsg()return OK.\n");
		}


		// LP FM Audio tones setup
		err = LP_FM_SetAudioSingleTone(audioToneFrequency, stereoChannelRelation, audioDeviationPercent, stereoEnable);
		
		if( ERR_OK!=err )	// VSG Audio Tone setup failed
		{
			// Fail Capture
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Failed to Setup LP FM VSG Audio Tone\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[FM] LP_FM_SetAudioSingleTone()return OK.\n");
		}		

		// Start VSG Transmit
		err = LP_FM_StartVsg();

		if( ERR_OK!=err )	// VSG Setup failed
		{
			// Fail Capture
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Failed to Start LP FM VSG Transmit\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[FM] LP_FM_StartVsg()return OK.\n");
		}


#pragma endregion

#pragma region Configure DUT to receive
		/*---------------------------*
		 * Configure DUT to Receive  *
		 *---------------------------*/

		// And clear vDUT parameters at the beginning.

		int deEmphasisTimeConstant = 0;

		vDUT_ClearParameters(g_FM_Dut);

		vDUT_AddIntegerParameter(g_FM_Dut, "CARRIER_FREQ_HZ",		l_rxVerify_RSSI_Param.CARRIER_FREQ_HZ);	
		
		vDUT_AddIntegerParameter(g_FM_Dut, "DE_EMPHASIS_FILTER_US",	deEmphasisTimeConstant);	 


		vDUT_AddIntegerParameter (g_FM_Dut, "STEREO_ENABLE",	    stereoEnable);

		vDUT_AddIntegerParameter(g_FM_Dut, "RX_RETRIEVE_RDS_MEASUREMENTS", OFF);	

		vDUT_AddIntegerParameter(g_FM_Dut, "RDS_ENABLE",		     rdsEnable);	 

		vDUT_AddIntegerParameter(g_FM_Dut, "STEREO_CHANNEL_RELATION_SELECT", stereoChannelRelation);

		vDUT_AddIntegerParameter(g_FM_Dut, "DUT_CAPTURE_AUDIO_FLAG", OFF);

		vDUT_AddIntegerParameter(g_FM_Dut, "AUDIO_SAMPLING_RATE_HZ", g_FMGlobalSettingParam.AUDIO_SAMPLING_RATE_HZ);

		
		// Parameters for setting the AIM Capture which is a part of vDUT Interface
		vDUT_AddDoubleParameter(g_FM_Dut, "SAMPLING_TIME_MS", 200);

		// Capture wave file location that is used to load file for LP Analysis
		char audioCaptureFileName[MAX_BUFFER_SIZE] = AIM_AUDIO_CAPTURE_WAVE_LOCATION;

		vDUT_AddStringParameter(g_FM_Dut, "AUDIO_CAPTURE_WAV_LOCATION", audioCaptureFileName);

		vDUT_AddIntegerParameter(g_FM_Dut, "STEREO", stereoEnable);

		// This sets the settling time after DUT Fm RX is set and before the Audio is captured using the AIM Device
		vDUT_AddIntegerParameter(g_FM_Dut, "DUT_RX_SETTLE_TIME_MS", g_FMGlobalSettingParam.DUT_RX_SETTLE_TIME_MS);


		err = vDUT_Run(g_FM_Dut, "RX_START");

		if ( ERR_OK!=err )
		{	
			vDutActive = false;
			// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer
			err = vDUT_GetStringReturn(g_FM_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error"
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Return error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] vDUT_Run(RX_START) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			vDutActive = true;
		}
		// Query DUT for measurement results such as RSSI and RDS. All DUT Specific Measurements will be returned
		// by RX_STATUS 

		
		// Delay for DUT settle
		if (0!=g_FMGlobalSettingParam.DUT_RX_SETTLE_TIME_MS)
		{
			Sleep(g_FMGlobalSettingParam.DUT_RX_SETTLE_TIME_MS);
		}
		else
		{
			// do nothing
		}

		err = vDUT_Run(g_FM_Dut, "RX_STATUS");

		if ( ERR_OK!=err )
		{	
			// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer
			err = vDUT_GetStringReturn(g_FM_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error"
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Return error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] vDUT_Run(RX_Status) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			// Do Nothing
		}



		err = vDUT_GetDoubleReturn(g_FM_Dut, "RX_RSSI_VALUE", &l_rxVerify_RSSI_Return.RSSI_DBM);
		
		if ( ERR_OK!=err )
		{	
			// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer
			err = vDUT_GetStringReturn(g_FM_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error"
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Return error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] vDUT_GetDoubleReturn(RX_RSSI_VALUE) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			// Do Nothing
		}

#pragma endregion

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err)
		{
			// Return Path Loss (dB)
			l_rxVerify_RSSI_Return.CABLE_LOSS_DB = l_rxVerify_RSSI_Param.CABLE_LOSS_DB;

			sprintf_s(l_rxVerify_RSSI_Return.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_rxVerify_RSSI_ReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_rxVerify_RSSI_Return.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_rxVerify_RSSI_Return.ERROR_MESSAGE, "[FM] Unknown Error!\n");
		err = -1;
    }

	// Stop VSG Transmit
	LP_FM_StopVsg();

    return err;
}


void InitializeRXVerify_RSSI_Containers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_rxVerify_RSSI_ParamMap.clear();

    FM_SETTING_STRUCT setting;

    l_rxVerify_RSSI_Param.CARRIER_FREQ_HZ = 98100000;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerify_RSSI_Param.CARRIER_FREQ_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerify_RSSI_Param.CARRIER_FREQ_HZ;
        setting.unit        = "Hz";
        setting.helpText    = "FM Carrier frequency in Hz";
        l_rxVerify_RSSI_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CARRIER_FREQ_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	
	l_rxVerify_RSSI_Param.CARRIER_POWER_DBM = -40.0;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerify_RSSI_Param.CARRIER_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerify_RSSI_Param.CARRIER_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "FM Carrier Peak power in dBm";
        l_rxVerify_RSSI_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CARRIER_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerify_RSSI_Param.CABLE_LOSS_DB = 0.0;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerify_RSSI_Param.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerify_RSSI_Param.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_rxVerify_RSSI_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/////////////////////////////////////////////////////////////////////////////
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_rxVerify_RSSI_ReturnMap.clear();

	l_rxVerify_RSSI_Return.RSSI_DBM = NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerify_RSSI_Return.RSSI_DBM))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerify_RSSI_Return.RSSI_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "RSSI Measurement in dBm";
		l_rxVerify_RSSI_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("RSSI_DBM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	
	l_rxVerify_RSSI_Return.CABLE_LOSS_DB = NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerify_RSSI_Return.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerify_RSSI_Return.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_rxVerify_RSSI_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// Error Message Return String
    l_rxVerify_RSSI_Return.ERROR_MESSAGE[0] = '\0';
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerify_RSSI_Return.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerify_RSSI_Return.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_rxVerify_RSSI_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}


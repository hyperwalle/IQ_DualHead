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
map<string, FM_SETTING_STRUCT> l_rxVerify_RDS_ParamMap;

// Return Value Container
map<string, FM_SETTING_STRUCT> l_rxVerify_RDS_ReturnMap;

struct tagParam
{
    // Mandatory Parameters
	int       CARRIER_FREQ_HZ;						/*! The FM Carrier frequency (Hz). */
	double    CARRIER_POWER_DBM;					/*! The FM Carrier Power (dBm). Max VSG Power Setting = -40 dBm*/
	double	  SAMPLING_TIME_MS;						/*! Specifies the data capture time (ms).*/
	char      STEREO_ENABLE[MAX_BUFFER_SIZE];		/*! Enable/Disable Stereo (0 = OFF / 1 = ON). */
	int		  PEAK_AUDIO_DEVIATION_HZ;				/*! Peak/Maximum Audio Deviation(Hz). */
	int		  PEAK_PILOT_DEVIATION_HZ;				/*! Peak/Maximum Pilot Deviation(Hz).Applies only if Stereo is Enabled*/

	// Mandatory Parameters
	int       RDS_ENABLE;                           /*! Enable/Disable RDS (0 = OFF / 1 = ON). */
	int       PEAK_RDS_DEVIATION_HZ;                /*! Peak/Maximum RDS Deviation(Hz).Applies only if Stereo is Enabled*/ 
	char      RDS_TRANSMIT_DATA[MAX_BUFFER_SIZE];   /*! Specifies the RDS Data that needs to be transmitted by the VSG */

	int		  PRE_EMP_TIME_CONSTANT_US;				/*! Pre-Emphasis Time Constant(0,25us,50us,75us). */
	int		  AUDIO_DEVIATION_PERCENT;				/*! Audio Deviation as a percentage of Peak Audio Deviation (percent). This Parameter scales the input audio stimulus*/
	int       AUDIO_TONE_FREQUENCY_HZ;				/*! The Single tone audio frequency */
	char	  STEREO_CHANNEL_RELATION_SELECT[MAX_BUFFER_SIZE];			/*! Select Single tone mode 0:Left Only, 1: Right Only, 2:Left and Right w/ Random Phase. Applies only if Stereo is enabled  */

	double    CABLE_LOSS_DB;						/*! The path loss of test system. */

} l_rxVerify_RDS_Param;

struct tagReturn
{   
      double   RDS_BLER;                            /*! Block Error Rate Measurement returned by the DUT in percentage. */
      double   CABLE_LOSS_DB;                       /*! The path loss of the test system. */
      char     ERROR_MESSAGE[MAX_BUFFER_SIZE];

} l_rxVerify_RDS_Return;

#pragma endregion

void ClearRxVerify_RDS_Return(void)
{
	l_rxVerify_RDS_ParamMap.clear();
	l_rxVerify_RDS_ReturnMap.clear();
}

FM_TEST_API int FM_RX_Verify_RDS(void)
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
	ClearReturnParameters(l_rxVerify_RDS_ReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_FM_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_rxVerify_RDS_ParamMap);
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
        RespondToQueryReturn(l_rxVerify_RDS_ReturnMap);
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
		err = GetInputParameters(l_rxVerify_RDS_ParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Input parameters are not completed.\n");
			throw logMessage;
		}		


	int stereoEnableInt;
	if(strcmp ("ON", l_rxVerify_RDS_Param.STEREO_ENABLE) == 0)
	{
		stereoEnableInt = 1;
	}
	else
	{
		stereoEnableInt = 0;
	}
	// STEREO_CHANNEL_RELATION_SELECT
	int stereoChannelRelationInt;
	if(strcmp ("LEFT_ONLY", l_rxVerify_RDS_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
	{
		stereoChannelRelationInt = 0;
	}
	else if(strcmp ("RIGHT_ONLY", l_rxVerify_RDS_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
	{
		stereoChannelRelationInt = 1;
	}
	else if(strcmp ("LEFT_RIGHT_RANDOM_PHASE", l_rxVerify_RDS_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
	{
		stereoChannelRelationInt = 2;
	}
	else if(strcmp ("LEFT_EQUALS_RIGHT", l_rxVerify_RDS_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
	{
		stereoChannelRelationInt = 3;
	}
	else if(strcmp ("LEFT_EQUALS_MINUS_RIGHT", l_rxVerify_RDS_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
	{
		stereoChannelRelationInt = 4;
	}
	// All other cases, default to LEFT_ONLY CHANNEL
	else
	{
		stereoChannelRelationInt = 0;
	}

		// Check path loss (by ant and freq)
		if ( 0==l_rxVerify_RDS_Param.CABLE_LOSS_DB )
		{
			err = TM_GetPathLossAtFrequency(g_FM_Test_ID, l_rxVerify_RDS_Param.CARRIER_FREQ_HZ, &l_rxVerify_RDS_Param.CABLE_LOSS_DB, 0, RX_TABLE);
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

	int preEmphasisTc;
	if(75 == l_rxVerify_RDS_Param.PRE_EMP_TIME_CONSTANT_US)
	{
		preEmphasisTc = PRE_EMPHASIS_75US;
	}
	else if(50 == l_rxVerify_RDS_Param.PRE_EMP_TIME_CONSTANT_US)
	{
		preEmphasisTc = PRE_EMPHASIS_50US;
	}
	else if(25 == l_rxVerify_RDS_Param.PRE_EMP_TIME_CONSTANT_US)
	{
		preEmphasisTc = PRE_EMPHASIS_25US;
	}
	else 
	{
		preEmphasisTc = PRE_EMPHASIS_NONE;
	}

#pragma region LP FM VSG Setup and transmit 
		/*-----------------------------------*
		 * Configure LP FM VSG  to transmit  *
		 *-----------------------------------*/
		//Setup VSG	
		int modulationEnable = 1;

		double compensatedCarrierPowerdBm;
		int totalDeviation = 0;


		compensatedCarrierPowerdBm = l_rxVerify_RDS_Param.CARRIER_POWER_DBM + l_rxVerify_RDS_Param.CABLE_LOSS_DB;
		
		if(-40 < compensatedCarrierPowerdBm)
		{
			compensatedCarrierPowerdBm = -40;
		}
		else
		{
			//Do Nothing
		}

		//LP_FM_SetVSG parameter totalFmDeviationHz requires total deviation including all deviation frequencies used
		totalDeviation = l_rxVerify_RDS_Param.PEAK_AUDIO_DEVIATION_HZ;
		if(stereoEnableInt)
			totalDeviation += l_rxVerify_RDS_Param.PEAK_PILOT_DEVIATION_HZ;
		if(l_rxVerify_RDS_Param.RDS_ENABLE)
			totalDeviation += l_rxVerify_RDS_Param.PEAK_RDS_DEVIATION_HZ;


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

		err = LP_FM_SetVsg(l_rxVerify_RDS_Param.CARRIER_FREQ_HZ,
						   compensatedCarrierPowerdBm,
						   modulationEnable,
						   totalDeviation,
						   stereoEnableInt,
						   l_rxVerify_RDS_Param.PEAK_PILOT_DEVIATION_HZ,
						   l_rxVerify_RDS_Param.RDS_ENABLE,
						   l_rxVerify_RDS_Param.PEAK_RDS_DEVIATION_HZ,
						   preEmphasisTc,
						   l_rxVerify_RDS_Param.RDS_TRANSMIT_DATA);

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
		err = LP_FM_SetAudioSingleTone(l_rxVerify_RDS_Param.AUDIO_TONE_FREQUENCY_HZ, stereoChannelRelationInt, l_rxVerify_RDS_Param.AUDIO_DEVIATION_PERCENT, stereoEnableInt);
		
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
		vDUT_ClearParameters(g_FM_Dut);

		vDUT_AddIntegerParameter(g_FM_Dut, "CARRIER_FREQ_HZ",		 l_rxVerify_RDS_Param.CARRIER_FREQ_HZ);	
		//The De-emphasis filter is selected to be the same as the Pre-Emphasis filer applied
		// If the DUT does not have De-emphasis filter, this parameter can be ignored in the DUT Control and
		// De-emphasis filter can be applied while performing Audio Analysis
		vDUT_AddIntegerParameter(g_FM_Dut, "DE_EMPHASIS_FILTER_US",	 l_rxVerify_RDS_Param.PRE_EMP_TIME_CONSTANT_US);	 

		vDUT_AddIntegerParameter (g_FM_Dut, "STEREO_ENABLE",	    stereoEnableInt);

		vDUT_AddIntegerParameter(g_FM_Dut, "RX_RETRIEVE_RDS_MEASUREMENTS", ON);	

		vDUT_AddIntegerParameter(g_FM_Dut, "RDS_ENABLE",		     l_rxVerify_RDS_Param.RDS_ENABLE);	 

		vDUT_AddIntegerParameter(g_FM_Dut, "STEREO_CHANNEL_RELATION_SELECT", stereoChannelRelationInt);
		vDUT_AddIntegerParameter(g_FM_Dut, "AUDIO_SAMPLING_RATE_HZ", g_FMGlobalSettingParam.AUDIO_SAMPLING_RATE_HZ);

		// Check Capture Time 
		if (0==l_rxVerify_RDS_Param.SAMPLING_TIME_MS)
		{
			samplingTimemS = 200; // set capture time to 50 ms if capture time is 0
		}
		else	// SAMPLING_TIME_MS != 0
		{
			samplingTimemS = l_rxVerify_RDS_Param.SAMPLING_TIME_MS;
		}
		
		// Parameters for setting the AIM Capture which is a part of vDUT Interface
		vDUT_AddDoubleParameter(g_FM_Dut, "SAMPLING_TIME_MS", samplingTimemS);

		// Capture wave file location that is used to load file for LP Analysis
		char audioCaptureFileName[MAX_BUFFER_SIZE] = AIM_AUDIO_CAPTURE_WAVE_LOCATION;

		vDUT_AddStringParameter(g_FM_Dut, "AUDIO_CAPTURE_WAV_LOCATION", audioCaptureFileName);

		vDUT_AddIntegerParameter(g_FM_Dut, "STEREO", stereoEnableInt);

		// This sets the settling time after DUT Fm RX is set and before the Audio is captured using the AIM Device
		vDUT_AddIntegerParameter(g_FM_Dut, "DUT_RX_SETTLE_TIME_MS", g_FMGlobalSettingParam.DUT_RX_SETTLE_TIME_MS);

		vDUT_AddIntegerParameter(g_FM_Dut, "DUT_CAPTURE_AUDIO_FLAG", OFF);

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

	    double rdsBLER  = 0.0;
		
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

		err = vDUT_GetDoubleReturn(g_FM_Dut, "RDS_BLER", &rdsBLER);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] vDUT_GetDoubleReturn(RDS_BLER) return error.\n");
			throw logMessage;
		}

		if ( rdsBLER<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Dut report rdsBLER less than 0.\n");
			throw logMessage;
		}
		else
		{
			l_rxVerify_RDS_Return.RDS_BLER = rdsBLER;
		}

	 //  /*-----------*
		//*  Rx Stop  *
		//*-----------*/
		//err = vDUT_Run(g_FM_Dut, "RX_STOP");		
		//if ( ERR_OK!=err )
		//{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
		//	err = vDUT_GetStringReturn(g_FM_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
		//	if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
		//	{
		//		err = -1;	// set err to -1, means "Error".
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
		//		throw logMessage;
		//	}
		//	else	// Just return normal error message in this case
		//	{
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(RX_STOP) return error.\n");
		//		throw logMessage;
		//	}
		//}
		//else
		//{
		//	//vDutActived = false;
		//}
	
		

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err)
		{
			// Return Path Loss (dB)
			l_rxVerify_RDS_Return.CABLE_LOSS_DB = l_rxVerify_RDS_Param.CABLE_LOSS_DB;
			sprintf_s(l_rxVerify_RDS_Return.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_rxVerify_RDS_ReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_rxVerify_RDS_Return.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_rxVerify_RDS_Return.ERROR_MESSAGE, "[FM] Unknown Error!\n");
		err = -1;
    }

	// Stop VSG Transmit
	LP_FM_StopVsg();

    return err;
}


void InitializeRXVerify_RDS_Containers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_rxVerify_RDS_ParamMap.clear();

    FM_SETTING_STRUCT setting;

    l_rxVerify_RDS_Param.CARRIER_FREQ_HZ = 98100000;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerify_RDS_Param.CARRIER_FREQ_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerify_RDS_Param.CARRIER_FREQ_HZ;
        setting.unit        = "Hz";
        setting.helpText    = "FM Carrier frequency in Hz";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CARRIER_FREQ_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	
	l_rxVerify_RDS_Param.CARRIER_POWER_DBM = -40.0;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerify_RDS_Param.CARRIER_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerify_RDS_Param.CARRIER_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "FM Carrier Peak power in dBm";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CARRIER_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_rxVerify_RDS_Param.STEREO_ENABLE, MAX_BUFFER_SIZE, "OFF");
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerify_RDS_Param.STEREO_ENABLE))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerify_RDS_Param.STEREO_ENABLE;
        setting.unit        = " ";
		setting.helpText    = "Enable / Disable Stereo :OFF, ON";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("STEREO_ENABLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerify_RDS_Param.PEAK_AUDIO_DEVIATION_HZ = 67500;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerify_RDS_Param.PEAK_AUDIO_DEVIATION_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerify_RDS_Param.PEAK_AUDIO_DEVIATION_HZ;
        setting.unit        = "Hz ";
		setting.helpText    = "Peak Audio Deviation in Hz";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("PEAK_AUDIO_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerify_RDS_Param.PEAK_PILOT_DEVIATION_HZ = 7500;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerify_RDS_Param.PEAK_PILOT_DEVIATION_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerify_RDS_Param.PEAK_PILOT_DEVIATION_HZ;
        setting.unit        = "Hz";
		setting.helpText    = "Peak Pilot Deviation in Hz. Applies only if Stereo is Enabled";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("PEAK_PILOT_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_rxVerify_RDS_Param.STEREO_CHANNEL_RELATION_SELECT, MAX_BUFFER_SIZE, "LEFT_ONLY");
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerify_RDS_Param.STEREO_CHANNEL_RELATION_SELECT))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerify_RDS_Param.STEREO_CHANNEL_RELATION_SELECT;
        setting.unit        = " ";
		setting.helpText    = "Select Left Right Channel Relation. Valid only if Stereo is enabled";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("STEREO_CHANNEL_RELATION_SELECT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerify_RDS_Param.AUDIO_DEVIATION_PERCENT = 100;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerify_RDS_Param.AUDIO_DEVIATION_PERCENT))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerify_RDS_Param.AUDIO_DEVIATION_PERCENT;
        setting.unit        = "percent";
		setting.helpText    = "Audio Deviation as a percentage of Peak Audio Deviation. This scales the audio input signal to the DUT";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("AUDIO_DEVIATION_PERCENT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerify_RDS_Param.AUDIO_TONE_FREQUENCY_HZ = 1000;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerify_RDS_Param.AUDIO_TONE_FREQUENCY_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerify_RDS_Param.AUDIO_TONE_FREQUENCY_HZ;
        setting.unit        = "Hz ";
		setting.helpText    = "Audio Tone Frequency in Hz";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("AUDIO_TONE_FREQUENCY_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerify_RDS_Param.CABLE_LOSS_DB = 0.0;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerify_RDS_Param.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerify_RDS_Param.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_rxVerify_RDS_Param.SAMPLING_TIME_MS = 200.0;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerify_RDS_Param.SAMPLING_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerify_RDS_Param.SAMPLING_TIME_MS;
        setting.unit        = "ms";
        setting.helpText    = "Capture time in milli-seconds";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("SAMPLING_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerify_RDS_Param.PRE_EMP_TIME_CONSTANT_US = 0;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerify_RDS_Param.PRE_EMP_TIME_CONSTANT_US))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerify_RDS_Param.PRE_EMP_TIME_CONSTANT_US;
        setting.unit        = "us";
        setting.helpText    = "Pre-Emphasis Time Constant in micro-seconds";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("PRE_EMP_TIME_CONSTANT_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    
    l_rxVerify_RDS_Param.RDS_ENABLE = 1;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerify_RDS_Param.RDS_ENABLE))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerify_RDS_Param.RDS_ENABLE;
		setting.unit        = "";
		setting.helpText    = "On:1/Off:0 RDS Enable/Disable Flag";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("RDS_ENABLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerify_RDS_Param.PEAK_RDS_DEVIATION_HZ = 7500;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerify_RDS_Param.PEAK_RDS_DEVIATION_HZ))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerify_RDS_Param.PEAK_RDS_DEVIATION_HZ;
		setting.unit        = "Hz";
		setting.helpText    = "Peak RDS Deviation, unit: (Hz)";
        l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("PEAK_RDS_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_rxVerify_RDS_Param.RDS_TRANSMIT_DATA, MAX_BUFFER_SIZE, "LitepointStation");
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerify_RDS_Param.RDS_TRANSMIT_DATA))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerify_RDS_Param.RDS_TRANSMIT_DATA;
        setting.unit        = "";
        setting.helpText    = "RDS Transmit Data";
       l_rxVerify_RDS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("RDS_TRANSMIT_DATA", setting) );
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
    l_rxVerify_RDS_ReturnMap.clear();
    
	l_rxVerify_RDS_Return.RDS_BLER = NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerify_RDS_Return.RDS_BLER))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerify_RDS_Return.RDS_BLER;
		setting.unit        = "%";
		setting.helpText    = "RDS Block Error Rate, Unit: (%)";
		l_rxVerify_RDS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("RDS_BLER", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerify_RDS_Return.CABLE_LOSS_DB = NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerify_RDS_Return.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerify_RDS_Return.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable Loss, Unit: (dB)";
		l_rxVerify_RDS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	// Error Message Return String
    l_rxVerify_RDS_Return.ERROR_MESSAGE[0] = '\0';
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerify_RDS_Return.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerify_RDS_Return.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_rxVerify_RDS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}


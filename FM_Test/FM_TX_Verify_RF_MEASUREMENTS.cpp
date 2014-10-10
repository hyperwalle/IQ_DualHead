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
map<string, FM_SETTING_STRUCT> l_txVerify_RF_MEASUREMENTS_ParamMap;

// Return Value Container
map<string, FM_SETTING_STRUCT> l_txVerify_RF_MEASUREMENTS_ReturnMap;

struct tagParam
{
    // Mandatory Parameters
	int		  CARRIER_FREQ_HZ;						/*! The FM Carrier frequency (Hz). */
	double    CARRIER_POWER_DBM;					/*! The FM Carrier Power (dBm). */
	double	  SAMPLING_TIME_MS;						/*! Specifies the data capture time (ms).*/
	int		  PEAK_AUDIO_DEVIATION_HZ;				/*! Peak/Maximum Audio Deviation(Hz). */
	char      STEREO_ENABLE[MAX_BUFFER_SIZE];		/*! Enable/Disable Stereo (0 = OFF / 1 = ON). */
	int		  PEAK_PILOT_DEVIATION_HZ;				/*! Peak/Maximum Pilot Deviation(Hz).Applies only if Stereo is Enabled*/
	char	  RDS_ENABLE[MAX_BUFFER_SIZE];							/*! Enable/Disable RDS (0 = OFF / 1 = ON). */
	int		  PEAK_RDS_DEVIATION_HZ;				/*! Peak/Maximum RDS Deviation(Hz).Applies only if Stereo is Enabled*/
	int		  PRE_EMP_TIME_CONSTANT_US;				/*! DUT Supported Pre-Emphasis Time Constant Setting(0,25,50,75). */
	int		  AUDIO_DEVIATION_PERCENT;				/*! Audio Deviation as a percentage of Peak Audio Deviation (percent).*/
	int       AUDIO_TONE_FREQUENCY_HZ;				/*! The Single tone audio frequency */
	char	  STEREO_CHANNEL_RELATION_SELECT[MAX_BUFFER_SIZE];			/*! Select Single tone mode 0:Left Only, 1: Right Only, 2:Left and Right w/ Random Phase. Applies only if Stereo is enabled  */
	double    CABLE_LOSS_DB;						/*! The path loss of test system. */
} l_txVerify_RF_MEASUREMENTS_Param;

struct tagReturn
{   
	//RF Spectrum Results
	double   OCCUPIED_BW_HZ;
	double   TOTAL_POWER_DBM;
	double   CHANNEL_POWER_DBM;

	//RF Demod Results
	double	 AVG_TOTAL_PEAK_DEVIATION_HZ;
	double	 RMS_TOTAL_PEAK_DEVIATION_HZ;
	double	 PLUS_TOTAL_PEAK_DEVIATION_HZ;
	double	 MINUS_TOTAL_PEAK_DEVIATION_HZ;
	double   CARRIER_FREQ_OFFSET_HZ;

	double   CABLE_LOSS_DB;						/*! The path loss of the test system. */
    char     ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerify_RF_MEASUREMENTS_Return;

void ClearTxVerify_RF_MEASUREMENTS_Return(void)
{
	l_txVerify_RF_MEASUREMENTS_ParamMap.clear();
	l_txVerify_RF_MEASUREMENTS_ReturnMap.clear();
}

#pragma endregion

//! FM TX Verify _RF_MEASUREMENTS_
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


FM_TEST_API int FM_TX_Verify_RF_MEASUREMENTS(void)
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
	ClearReturnParameters(l_txVerify_RF_MEASUREMENTS_ReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_FM_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_txVerify_RF_MEASUREMENTS_ParamMap);
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
        RespondToQueryReturn(l_txVerify_RF_MEASUREMENTS_ReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< double >
	 *-----------------------------------------------------------------------------------------*/
	vector< double >		obw_Hz							(g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE);
	vector< double >		total_power_dBm					(g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE);
	vector< double >		channel_power_dBm				(g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE);
	vector< double >		noise_bw_Hz						(g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE);
	vector< double >		res_bw_Hz						(g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE);

	vector< double >		avg_total_peak_deviation_hz		 (g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE);
	vector< double >		plus_total_peak_deviation_hz	 (g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE);
	vector< double >		minus_total_peak_deviation_hz	 (g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE);
	vector< double >		rms_total_peak_deviation_hz		 (g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE);
	vector< double >		carrier_freq_offset_hz			 (g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE);

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
		err = GetInputParameters(l_txVerify_RF_MEASUREMENTS_ParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Input parameters are not completed.\n");
			throw logMessage;
		}		

		int stereoEnableInt;
		if(strcmp ("ON", l_txVerify_RF_MEASUREMENTS_Param.STEREO_ENABLE) == 0)
		{
			stereoEnableInt = 1;
		}
		else
		{
			stereoEnableInt = 0;
		}

		int rdsEnableInt;
		if(strcmp ("ON", l_txVerify_RF_MEASUREMENTS_Param.RDS_ENABLE) == 0)
		{
			rdsEnableInt = 1;
		}
		else
		{
			rdsEnableInt = 0;
		}

		// STEREO_CHANNEL_RELATION_SELECT
		int stereoChannelRelationInt;
		if(strcmp ("LEFT_ONLY", l_txVerify_RF_MEASUREMENTS_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
		{
			stereoChannelRelationInt = 1;
		}
		else if(strcmp ("RIGHT_ONLY", l_txVerify_RF_MEASUREMENTS_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
		{
			stereoChannelRelationInt = 2;
		}
		else if(strcmp ("LEFT_EQUALS_RIGHT", l_txVerify_RF_MEASUREMENTS_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
		{
			stereoChannelRelationInt = 3;
		}
		else if(strcmp ("LEFT_EQUALS_MINUS_RIGHT", l_txVerify_RF_MEASUREMENTS_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
		{
			stereoChannelRelationInt = 4;
		}
		// All other cases, default to LEFT_EQUALS_RIGHT CHANNEL
		else
		{
			stereoChannelRelationInt = 3;
		}


		// Check path loss (by ant and freq)
		if ( 0==l_txVerify_RF_MEASUREMENTS_Param.CABLE_LOSS_DB )
		{
			err = TM_GetPathLossAtFrequency(g_FM_Test_ID, l_txVerify_RF_MEASUREMENTS_Param.CARRIER_FREQ_HZ, &l_txVerify_RF_MEASUREMENTS_Param.CABLE_LOSS_DB, 0, TX_TABLE);
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

#pragma region Configure DUT to transmit
		/*-------------------------------------------*
		 * Configure DUT to transmit - PRBS9 Pattern *
		 *-------------------------------------------*/
		// Set DUT RF frequency, tx power, data rate
		// And clear vDUT parameters at the beginning.
		vDUT_ClearParameters(g_FM_Dut);

		vDUT_AddIntegerParameter(g_FM_Dut, "AUDIO_SAMPLING_RATE_HZ",		g_FMGlobalSettingParam.AUDIO_SAMPLING_RATE_HZ);	

		vDUT_AddDoubleParameter(g_FM_Dut, "DUT_PEAK_AUDIO_LEVEL_VOLTS",   g_FMGlobalSettingParam.DUT_PEAK_VOLTAGE_LEVEL_VOLTS);

		vDUT_AddIntegerParameter (g_FM_Dut, "STEREO_ENABLE",	    stereoEnableInt);

		vDUT_AddIntegerParameter (g_FM_Dut, "LEFT_RIGHT_RELATION",	    stereoChannelRelationInt);

		vDUT_AddIntegerParameter (g_FM_Dut, "STEREO_CHANNEL_RELATION_SELECT",	stereoChannelRelationInt);

		vDUT_AddIntegerParameter (g_FM_Dut, "AUDIO_TONE_FREQUENCY_HZ",	    l_txVerify_RF_MEASUREMENTS_Param.AUDIO_TONE_FREQUENCY_HZ);

		vDUT_AddIntegerParameter(g_FM_Dut, "CARRIER_FREQ_HZ",		l_txVerify_RF_MEASUREMENTS_Param.CARRIER_FREQ_HZ);	

		vDUT_AddDoubleParameter(g_FM_Dut, "CARRIER_POWER_DBM",   l_txVerify_RF_MEASUREMENTS_Param.CARRIER_POWER_DBM);

		vDUT_AddIntegerParameter (g_FM_Dut, "AUDIO_DEVIATION_PERCENT",	    l_txVerify_RF_MEASUREMENTS_Param.AUDIO_DEVIATION_PERCENT);

		vDUT_AddIntegerParameter(g_FM_Dut, "DUT_TX_SETTLE_TIME_MS",	g_FMGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);

		vDUT_AddIntegerParameter (g_FM_Dut, "PEAK_AUDIO_DEVIATION_HZ",	    l_txVerify_RF_MEASUREMENTS_Param.PEAK_AUDIO_DEVIATION_HZ);

		vDUT_AddIntegerParameter (g_FM_Dut, "PEAK_PILOT_DEVIATION_HZ",	    l_txVerify_RF_MEASUREMENTS_Param.PEAK_PILOT_DEVIATION_HZ);
	
		vDUT_AddIntegerParameter(g_FM_Dut, "PRE_EMP_TIME_CONSTANT_US",	l_txVerify_RF_MEASUREMENTS_Param.PRE_EMP_TIME_CONSTANT_US);

		vDUT_AddIntegerParameter (g_FM_Dut, "RDS_ENABLE",	    rdsEnableInt);

		err = vDUT_Run(g_FM_Dut, "TX_START");

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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] vDUT_Run(TX_START) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			vDutActive = true;
		}

		// Delay for DUT settle
		if (0!=g_FMGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
		{
			Sleep(g_FMGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
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
		double marginDb = 2;
		cableLossDb = l_txVerify_RF_MEASUREMENTS_Param.CABLE_LOSS_DB;

		err = LP_FM_SetVsa(l_txVerify_RF_MEASUREMENTS_Param.CARRIER_FREQ_HZ,(l_txVerify_RF_MEASUREMENTS_Param.CARRIER_POWER_DBM));
		// 4/4/12 trunk merge: IQlite TW trunk uses following
		//err = LP_FM_SetVsa_Agc_On(l_txVerify_RF_MEASUREMENTS_Param.CARRIER_FREQ_HZ,(l_txVerify_RF_MEASUREMENTS_Param.CARRIER_POWER_DBM - cableLossDb + marginDb));
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Failed to setup FM VSA.\n");
			throw logMessage;
		}		

	   /*----------------------------*
		* Disable VSG output signal  *
		*----------------------------*/
		// make sure no signal is generated by the VSG
		//err = ::LP_FM_StopVsg();
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Fail to turn off VSG, LP_FM_StopVsg() return error.\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[FM] Turn off VSG LP_FM_StopVsg() return OK.\n");
		//}
#pragma endregion

		// Check Capture Time 
		if (0==l_txVerify_RF_MEASUREMENTS_Param.SAMPLING_TIME_MS)
		{
			samplingTimemS = 200; // set capture time to 50 ms if capture time is 0
			// 4/4/12 trunk merge: IQlite TW trunk uses following
			//samplingTimemS = 500; // set capture time to 500 ms if capture time is 0
		}
		else	// SAMPLING_TIME_MS != 0
		{
			samplingTimemS = l_txVerify_RF_MEASUREMENTS_Param.SAMPLING_TIME_MS;
		}

		/*--------------------------------*
		 * Start "while" loop for average *
		 *--------------------------------*/
		int avgIteration = 0;

		while ( avgIteration<g_FMGlobalSettingParam.TX_RF_MEASUREMENTS_AVERAGE )
		{				
			analysisOK = false;
			captureOK  = false; 

		   /*----------------------------*
			* Perform normal VSA capture *
			*----------------------------*/
			err = LP_FM_VsaDataCapture(samplingTimemS);   
			if( ERR_OK!=err )	// capture is failed
			{
				// Fail Capture
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Failed to capture signal at %d Hz.\n", l_txVerify_RF_MEASUREMENTS_Param.CARRIER_FREQ_HZ);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[FM] LP_FM_VSACapture() at %d Hz return OK.\n", l_txVerify_RF_MEASUREMENTS_Param.CARRIER_FREQ_HZ);
			}	

			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;



			/*------------------------------*
			 *  Perform RF analysis  *
			 *------------------------------*/
			err = LP_FM_Analyze_RF();		
			if (ERR_OK!=err)
			{	//Analysis Failure
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] LP_FM_Analyze_RF() return error.\n");
				throw logMessage;
			}
			else
			{
				// do nothing
			}

#pragma region Retrieve RF analysis Results
			/*-----------------------------*
			 *  Retrieve analysis results  *
			 *-----------------------------*/
			analysisOK = true;


			obw_Hz[avgIteration] = LP_FM_GetScalarMeasurement("obw", 0);
			total_power_dBm[avgIteration] = LP_FM_GetScalarMeasurement("totalPower", 0);
			channel_power_dBm[avgIteration] = LP_FM_GetScalarMeasurement("powerInBw", 0);

			if ( NA_NUMBER == obw_Hz[avgIteration] || NA_NUMBER == total_power_dBm[avgIteration] ||NA_NUMBER == channel_power_dBm[avgIteration])
			{
				analysisOK = false;
				l_txVerify_RF_MEASUREMENTS_Return.OCCUPIED_BW_HZ = NA_NUMBER;
				l_txVerify_RF_MEASUREMENTS_Return.TOTAL_POWER_DBM = NA_NUMBER;
				l_txVerify_RF_MEASUREMENTS_Return.CHANNEL_POWER_DBM = NA_NUMBER;

				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_FM_GetScalarMeasurement(\"RF Spectrum\") Failed.\n");
				throw logMessage;
			}
			else
			{
				obw_Hz[avgIteration] = obw_Hz[avgIteration];	
				// for power measurements, cable loss is added to the measured power.
				total_power_dBm[avgIteration] = total_power_dBm[avgIteration] + l_txVerify_RF_MEASUREMENTS_Param.CABLE_LOSS_DB;
				channel_power_dBm[avgIteration] = channel_power_dBm[avgIteration] + l_txVerify_RF_MEASUREMENTS_Param.CABLE_LOSS_DB;
			}

#pragma endregion

			/*------------------------------*
			 *  Perform Demod analysis  *
			 *------------------------------*/
			err = LP_FM_Analyze_Demod();		// using default param values for the function
			if (ERR_OK!=err)
			{	//Analysis Failure
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] LP_FM_Analyze_Demod() return error.\n");
				throw logMessage;
				analysisOK = false;
			}
			else
			{
				// do nothing
			}

#pragma region Retrieve Demod analysis Results
			/*-----------------------------*
			 *  Retrieve Demod analysis results  *
			 *-----------------------------*/
			analysisOK = true;
					
			avg_total_peak_deviation_hz[avgIteration] = LP_FM_GetScalarMeasurement("demodAveragePeakDeviation", 0);
			rms_total_peak_deviation_hz[avgIteration] = LP_FM_GetScalarMeasurement("demodRmsPeakDeviation", 0);
			plus_total_peak_deviation_hz[avgIteration] = LP_FM_GetScalarMeasurement("demodPlusPeakDeviation", 0);
			minus_total_peak_deviation_hz[avgIteration] = LP_FM_GetScalarMeasurement("demodMinusPeakDeviation", 0);
			carrier_freq_offset_hz[avgIteration] = LP_FM_GetScalarMeasurement("demodCfo", 0);


			if ( NA_NUMBER == avg_total_peak_deviation_hz[avgIteration] || NA_NUMBER == rms_total_peak_deviation_hz[avgIteration] || 
				NA_NUMBER == plus_total_peak_deviation_hz[avgIteration] || NA_NUMBER == minus_total_peak_deviation_hz[avgIteration] ||
				NA_NUMBER == carrier_freq_offset_hz[avgIteration])
			{
				analysisOK = false;
				l_txVerify_RF_MEASUREMENTS_Return.AVG_TOTAL_PEAK_DEVIATION_HZ= NA_NUMBER;
				l_txVerify_RF_MEASUREMENTS_Return.RMS_TOTAL_PEAK_DEVIATION_HZ = NA_NUMBER;
				l_txVerify_RF_MEASUREMENTS_Return.PLUS_TOTAL_PEAK_DEVIATION_HZ= NA_NUMBER;
				l_txVerify_RF_MEASUREMENTS_Return.MINUS_TOTAL_PEAK_DEVIATION_HZ = NA_NUMBER;
				l_txVerify_RF_MEASUREMENTS_Return.CARRIER_FREQ_OFFSET_HZ= NA_NUMBER;

				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_FM_GetScalarMeasurement Failed(\"RF Deviation\") return error.\n");
				throw logMessage;
			}
			else
			{
				avg_total_peak_deviation_hz[avgIteration] = avg_total_peak_deviation_hz[avgIteration]; 
				rms_total_peak_deviation_hz[avgIteration] = rms_total_peak_deviation_hz[avgIteration];
				plus_total_peak_deviation_hz[avgIteration] = plus_total_peak_deviation_hz[avgIteration]; 
				minus_total_peak_deviation_hz[avgIteration] = minus_total_peak_deviation_hz[avgIteration];	
				carrier_freq_offset_hz[avgIteration] = carrier_freq_offset_hz[avgIteration];
			}

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
			// Peak Power test result
			err = ::AverageTestResult(&obw_Hz[0], avgIteration, Linear, l_txVerify_RF_MEASUREMENTS_Return.OCCUPIED_BW_HZ, dummyMax, dummyMin);
			err = ::AverageTestResult(&total_power_dBm[0], avgIteration, LOG_10, l_txVerify_RF_MEASUREMENTS_Return.TOTAL_POWER_DBM, dummyMax, dummyMin);
			err = ::AverageTestResult(&channel_power_dBm[0], avgIteration, LOG_10, l_txVerify_RF_MEASUREMENTS_Return.CHANNEL_POWER_DBM, dummyMax, dummyMin);

			err = ::AverageTestResult(&avg_total_peak_deviation_hz[0], avgIteration, Linear, l_txVerify_RF_MEASUREMENTS_Return.AVG_TOTAL_PEAK_DEVIATION_HZ, dummyMax, dummyMin);
			err = ::AverageTestResult(&rms_total_peak_deviation_hz[0], avgIteration, Linear, l_txVerify_RF_MEASUREMENTS_Return.RMS_TOTAL_PEAK_DEVIATION_HZ, dummyMax, dummyMin);
			err = ::AverageTestResult(&plus_total_peak_deviation_hz[0], avgIteration, Linear, l_txVerify_RF_MEASUREMENTS_Return.PLUS_TOTAL_PEAK_DEVIATION_HZ, dummyMax, dummyMin);
			err = ::AverageTestResult(&minus_total_peak_deviation_hz[0], avgIteration, Linear,l_txVerify_RF_MEASUREMENTS_Return.MINUS_TOTAL_PEAK_DEVIATION_HZ, dummyMax , dummyMin);
			err = ::AverageTestResult(&carrier_freq_offset_hz[0], avgIteration, Linear,l_txVerify_RF_MEASUREMENTS_Return.CARRIER_FREQ_OFFSET_HZ, dummyMax , dummyMin);
		}
		else
		{
			// do nothing
		}
#pragma endregion

	   /*-----------*
	    *  Tx Stop  *
		*-----------*/
		err = vDUT_Run(g_FM_Dut, "TX_STOP");		
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer
			err = vDUT_GetStringReturn(g_FM_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// indicates an error has occurred
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// returns normal error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] vDUT_Run(TX_STOP) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			vDutActive = false;
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			// Return Path Loss (dB)
			l_txVerify_RF_MEASUREMENTS_Return.CABLE_LOSS_DB = l_txVerify_RF_MEASUREMENTS_Param.CABLE_LOSS_DB;

			sprintf_s(l_txVerify_RF_MEASUREMENTS_Return.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerify_RF_MEASUREMENTS_ReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txVerify_RF_MEASUREMENTS_Return.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txVerify_RF_MEASUREMENTS_Return.ERROR_MESSAGE, "[FM] Unknown Error!\n");
		err = -1;
    }

	// This is a special case and happens only when certain errors occur before the TX_STOP. 
	// This is handled by error handling, but TX_STOP must be handled manually.
	
	if ( vDutActive )
	{
		vDUT_Run(g_FM_Dut, "TX_STOP");
	}
	else
	{
		// do nothing
	}

    // Free memory
    obw_Hz.clear();
	total_power_dBm.clear();
	channel_power_dBm.clear();

	avg_total_peak_deviation_hz.clear();
	rms_total_peak_deviation_hz.clear();
    plus_total_peak_deviation_hz.clear();
	minus_total_peak_deviation_hz.clear();
	carrier_freq_offset_hz.clear();
    return err;
}



void InitializeTXVerify_RF_MEASUREMENTS_Containers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txVerify_RF_MEASUREMENTS_ParamMap.clear();

    FM_SETTING_STRUCT setting;

    l_txVerify_RF_MEASUREMENTS_Param.CARRIER_FREQ_HZ = 98100000;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_RF_MEASUREMENTS_Param.CARRIER_FREQ_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Param.CARRIER_FREQ_HZ;
        setting.unit        = "Hz";
        setting.helpText    = "FM Carrier frequency in Hz";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CARRIER_FREQ_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	
	l_txVerify_RF_MEASUREMENTS_Param.CARRIER_POWER_DBM = -40.0;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Param.CARRIER_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Param.CARRIER_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "FM Carrier Peak power in dBm";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CARRIER_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txVerify_RF_MEASUREMENTS_Param.STEREO_ENABLE, MAX_BUFFER_SIZE, "OFF");
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerify_RF_MEASUREMENTS_Param.STEREO_ENABLE))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Param.STEREO_ENABLE;
        setting.unit        = " ";
		setting.helpText    = "Enable / Disable Stereo :OFF, ON";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("STEREO_ENABLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerify_RF_MEASUREMENTS_Param.PEAK_AUDIO_DEVIATION_HZ = 67500;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_RF_MEASUREMENTS_Param.PEAK_AUDIO_DEVIATION_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Param.PEAK_AUDIO_DEVIATION_HZ;
        setting.unit        = "Hz ";
		setting.helpText    = "Peak Audio Deviation in Hz";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("PEAK_AUDIO_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerify_RF_MEASUREMENTS_Param.PEAK_PILOT_DEVIATION_HZ = 7500;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_RF_MEASUREMENTS_Param.PEAK_PILOT_DEVIATION_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Param.PEAK_PILOT_DEVIATION_HZ;
        setting.unit        = "Hz";
		setting.helpText    = "Peak Pilot Deviation in Hz. Applies only if Stereo is Enabled";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("PEAK_PILOT_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txVerify_RF_MEASUREMENTS_Param.STEREO_CHANNEL_RELATION_SELECT, MAX_BUFFER_SIZE, "LEFT_ONLY");
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerify_RF_MEASUREMENTS_Param.STEREO_CHANNEL_RELATION_SELECT))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Param.STEREO_CHANNEL_RELATION_SELECT;
        setting.unit        = " ";
		setting.helpText    = "Select Left Right Channel Relation. Valid only if Stereo is enabled";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("STEREO_CHANNEL_RELATION_SELECT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerify_RF_MEASUREMENTS_Param.AUDIO_DEVIATION_PERCENT = 100;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_RF_MEASUREMENTS_Param.AUDIO_DEVIATION_PERCENT))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Param.AUDIO_DEVIATION_PERCENT;
        setting.unit        = "percent";
		setting.helpText    = "Audio Deviation as a percentage of Peak Audio Deviation. This scales the input audio signal amplitude to the DUT to achieve the % audio deviation specified.";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("AUDIO_DEVIATION_PERCENT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerify_RF_MEASUREMENTS_Param.AUDIO_TONE_FREQUENCY_HZ = 1000;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_RF_MEASUREMENTS_Param.AUDIO_TONE_FREQUENCY_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Param.AUDIO_TONE_FREQUENCY_HZ;
        setting.unit        = "Hz ";
		setting.helpText    = "Audio Tone Frequency in Hz";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("AUDIO_TONE_FREQUENCY_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txVerify_RF_MEASUREMENTS_Param.RDS_ENABLE, MAX_BUFFER_SIZE, "OFF");
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerify_RF_MEASUREMENTS_Param.RDS_ENABLE))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Param.RDS_ENABLE;
        setting.unit        = " ";
		setting.helpText    = "Enable / Disable RDS :OFF, ON";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("RDS_ENABLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerify_RF_MEASUREMENTS_Param.PEAK_RDS_DEVIATION_HZ = 2000;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_RF_MEASUREMENTS_Param.PEAK_RDS_DEVIATION_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Param.PEAK_RDS_DEVIATION_HZ;
        setting.unit        = "Hz";
		setting.helpText    = "Peak RDS Deviation in Hz";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("PEAK_RDS_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerify_RF_MEASUREMENTS_Param.CABLE_LOSS_DB = 0.0;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Param.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Param.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_txVerify_RF_MEASUREMENTS_Param.SAMPLING_TIME_MS = 200.0;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Param.SAMPLING_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_RF_MEASUREMENTS_Param.SAMPLING_TIME_MS;
        setting.unit        = "ms";
        setting.helpText    = "Capture time in milli-seconds";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("SAMPLING_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
    l_txVerify_RF_MEASUREMENTS_Param.PRE_EMP_TIME_CONSTANT_US = 0;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_RF_MEASUREMENTS_Param.PRE_EMP_TIME_CONSTANT_US))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_RF_MEASUREMENTS_Param.PRE_EMP_TIME_CONSTANT_US;
        setting.unit        = "us";
        setting.helpText    = "DUT Supported Pre-emphasis Time Constant in us";
        l_txVerify_RF_MEASUREMENTS_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("PRE_EMP_TIME_CONSTANT_US", setting) );
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
    l_txVerify_RF_MEASUREMENTS_ReturnMap.clear();

    l_txVerify_RF_MEASUREMENTS_Return.OCCUPIED_BW_HZ = NA_DOUBLE;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Return.OCCUPIED_BW_HZ))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_RF_MEASUREMENTS_Return.OCCUPIED_BW_HZ;
        setting.unit        = "Hz";
        setting.helpText    = "Occupied Bandwidth in Hz";
        l_txVerify_RF_MEASUREMENTS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("OCCUPIED_BW_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerify_RF_MEASUREMENTS_Return.TOTAL_POWER_DBM = NA_DOUBLE;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Return.TOTAL_POWER_DBM))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_RF_MEASUREMENTS_Return.TOTAL_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Total Power in dBm";
        l_txVerify_RF_MEASUREMENTS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("TOTAL_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerify_RF_MEASUREMENTS_Return.CHANNEL_POWER_DBM= NA_DOUBLE;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Return.CHANNEL_POWER_DBM))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_RF_MEASUREMENTS_Return.CHANNEL_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Channel Power in dBm";
        l_txVerify_RF_MEASUREMENTS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("CHANNEL_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerify_RF_MEASUREMENTS_Return.AVG_TOTAL_PEAK_DEVIATION_HZ = NA_DOUBLE;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Return.AVG_TOTAL_PEAK_DEVIATION_HZ))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_RF_MEASUREMENTS_Return.AVG_TOTAL_PEAK_DEVIATION_HZ;
        setting.unit        = "Hz";
        setting.helpText    = "Measured Average Total Peak Deviation in Hz";
        l_txVerify_RF_MEASUREMENTS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("AVG_TOTAL_PEAK_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerify_RF_MEASUREMENTS_Return.RMS_TOTAL_PEAK_DEVIATION_HZ = NA_DOUBLE;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Return.RMS_TOTAL_PEAK_DEVIATION_HZ))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_RF_MEASUREMENTS_Return.RMS_TOTAL_PEAK_DEVIATION_HZ;
        setting.unit        = "Hz";
        setting.helpText    = "Measured RMS Total Peak Deviation in Hz";
        l_txVerify_RF_MEASUREMENTS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("RMS_TOTAL_PEAK_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerify_RF_MEASUREMENTS_Return.PLUS_TOTAL_PEAK_DEVIATION_HZ = NA_DOUBLE;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Return.PLUS_TOTAL_PEAK_DEVIATION_HZ))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_RF_MEASUREMENTS_Return.PLUS_TOTAL_PEAK_DEVIATION_HZ;
        setting.unit        = "Hz";
        setting.helpText    = "Measured Plus Total Peak Deviation in Hz";
        l_txVerify_RF_MEASUREMENTS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("PLUS_TOTAL_PEAK_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerify_RF_MEASUREMENTS_Return.MINUS_TOTAL_PEAK_DEVIATION_HZ = NA_DOUBLE;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Return.MINUS_TOTAL_PEAK_DEVIATION_HZ))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_RF_MEASUREMENTS_Return.MINUS_TOTAL_PEAK_DEVIATION_HZ;
        setting.unit        = "Hz";
        setting.helpText    = "Measured Minus Total Deviation in Hz";
        l_txVerify_RF_MEASUREMENTS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("MINUS_TOTAL_PEAK_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerify_RF_MEASUREMENTS_Return.CARRIER_FREQ_OFFSET_HZ = NA_DOUBLE;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Return.CARRIER_FREQ_OFFSET_HZ))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_RF_MEASUREMENTS_Return.CARRIER_FREQ_OFFSET_HZ;
        setting.unit        = "Hz";
        setting.helpText    = "Measured Carrier Frequency Offset in Hz";
        l_txVerify_RF_MEASUREMENTS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("CARRIER_FREQ_OFFSET_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


	l_txVerify_RF_MEASUREMENTS_Return.CABLE_LOSS_DB = NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_RF_MEASUREMENTS_Return.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerify_RF_MEASUREMENTS_Return.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_txVerify_RF_MEASUREMENTS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
    
	// Error Message Return String
    l_txVerify_RF_MEASUREMENTS_Return.ERROR_MESSAGE[0] = '\0';
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerify_RF_MEASUREMENTS_Return.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerify_RF_MEASUREMENTS_Return.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_txVerify_RF_MEASUREMENTS_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}


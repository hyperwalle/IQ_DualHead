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
map<string, FM_SETTING_STRUCT> l_txVerify_AUDIO_ParamMap;

// Return Value Container
map<string, FM_SETTING_STRUCT> l_txVerify_AUDIO_ReturnMap;

struct tagParam
{
    // Mandatory Parameters
	int		  CARRIER_FREQ_HZ;						/*! The FM Carrier frequency (Hz). */
	double    CARRIER_POWER_DBM;					/*! The FM Carrier Power (dBm). */
	double	  SAMPLING_TIME_MS;						/*! Specifies the data capture time (ms).*/
	char      STEREO_ENABLE[MAX_BUFFER_SIZE];		/*! Enable/Disable Stereo (0 = OFF / 1 = ON). */
	int		  PEAK_AUDIO_DEVIATION_HZ;				/*! Peak/Maximum Audio Deviation(Hz). */
	int		  PEAK_PILOT_DEVIATION_HZ;				/*! Peak/Maximum Pilot Deviation(Hz).Applies only if Stereo is Enabled*/
	int		  PRE_EMP_TIME_CONSTANT_US;				/*! DUT Supported Pre-Emphasis Time Constant Setting(0,25,50,75). */
	int		  AUDIO_DEVIATION_PERCENT;				/*! Audio Deviation as a percentage of Peak Audio Deviation (percent).*/
	int       AUDIO_TONE_FREQUENCY_HZ;				/*! The Single tone audio frequency */
	int		  AUDIO_FREQ_LOW_LIMIT_HZ;				/*! Set the Lower bound for analyzed audio range(Hz) */
	int		  AUDIO_FREQ_HI_LIMIT_HZ;				/*! Set the Upper bound for analyzed audio range(Hz) */
	char	  STEREO_CHANNEL_RELATION_SELECT[MAX_BUFFER_SIZE];			/*! Select Single tone mode 0:Left Only, 1: Right Only, 2:Left and Right w/ Random Phase. Applies only if Stereo is enabled  */
	char	  FILTER_TYPE_1[MAX_BUFFER_SIZE];			/*! Select filter 1 to be applied to the audio analysis
													0: NO_FILTER
													1: HPF
													2: LPF
													3:DE_EMPHASIS
													4: CCIT_P53
													5: C_MESSAGE
													6: A_WEIGHTING
													7: C_WEIGHTING
													8: ITU_R_468_WEIGHTED
													9: ITU_R_468_UNWEIGHTED*/
	int		FILTER_PARAMETER_1;						/*Filter parameter value corresponding to FILTER_TYPE_1
													FITER_PARAMETER_1 for HPF = 50(3dB cut-off frequency in Hz)
													FITER_PARAMETER_1 for LPF = 20000(3dB cut-off frequency in Hz)
													Other filters doesn't need Filter parameters to be specified.*/
	char     FILTER_TYPE_2[MAX_BUFFER_SIZE];;						/*! Select filter 2 to be applied to the audio analysis
													0: NO_FILTER
													1: HPF
													2: LPF
													4: CCIT_P53
													5: C_MESSAGE
													6: A_WEIGHTING
													7: C_WEIGHTING
													8: ITU_R_468_WEIGHTED
													9: ITU_R_468_UNWEIGHTED*/
	int		FILTER_PARAMETER_2;						/*Filter parameter value corresponding to FILTER_TYPE_2
													FITER_PARAMETER_2 for HPF = 50(3dB cut-off frequency in Hz)
													FITER_PARAMETER_2 for LPF = 20000(3dB cut-off frequency in Hz)
													Other filters doesn't need Filter parameters to be specified.*/

	double    CABLE_LOSS_DB;						/*! The path loss of test system. */
} l_txVerify_AUDIO_Param;

struct tagReturn
{   
	//Mono SNR Results
	double   MONO_SNR_DB;
	double   MONO_SINAD_DB;

	// Stereo SNR results
	double   LEFT_SNR_DB;
	double   RIGHT_SNR_DB;
	double   LEFT_SINAD_DB;
	double   RIGHT_SINAD_DB;

	double   STEREO_CROSSTALK_DB;
	double   PILOT_FREQ_OFFSET_HZ;

	//Mono THD Results
	double   MONO_THD_DB;
	double   MONO_TNHD_DB;

	// Stereo THD results
	double   LEFT_THD_DB;
	double   RIGHT_THD_DB;
	double   LEFT_TNHD_DB;
	double   RIGHT_TNHD_DB;

	double   CABLE_LOSS_DB;						/*! The path loss of the test system. */
    char     ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerify_AUDIO_Return;

#pragma endregion

void ClearTxVerify_AUDIO_Return(void)
{
	l_txVerify_AUDIO_ParamMap.clear();
	l_txVerify_AUDIO_ReturnMap.clear();
}

FM_TEST_API int FM_TX_Verify_AUDIO(void)
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
	ClearReturnParameters(l_txVerify_AUDIO_ReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_FM_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_txVerify_AUDIO_ParamMap);
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
        RespondToQueryReturn(l_txVerify_AUDIO_ReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< double >
	 *-----------------------------------------------------------------------------------------*/
	vector< double >		mono_snr_db				 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		mono_sinad_db			 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		left_snr_db				 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		right_snr_db		 	 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		left_sinad_db		 	 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		right_sinad_db		 	 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		stereo_crosstalk_db	 	 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		pilot_freq_offset_hz 	 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);

	vector< double >		mono_thd_db				 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		mono_tnhd_db		 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		left_thd_db				 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		right_thd_db			 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		left_tnhd_db			 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);
	vector< double >		right_tnhd_db			 (g_FMGlobalSettingParam.TX_AUDIO_AVERAGE);

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
		err = GetInputParameters(l_txVerify_AUDIO_ParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Input parameters are not completed.\n");
			throw logMessage;
		}	

	int stereoEnableInt;
	if(strcmp ("ON", l_txVerify_AUDIO_Param.STEREO_ENABLE) == 0)
	{
		stereoEnableInt = 1;
	}
	else
	{
		stereoEnableInt = 0;
	}
		int rdsEnableInt;
//		if(strcmp ("ON", l_extTxVerifyTxRfAndAudioParam.RDS_ENABLE) == 0)
//		{
//			rdsEnableInt = 1;
//		}
//		else
//		{
			rdsEnableInt = 0;
//		}

	// STEREO_CHANNEL_RELATION_SELECT
	int stereoChannelRelationInt;
	if(strcmp ("LEFT_ONLY", l_txVerify_AUDIO_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
	{
		stereoChannelRelationInt = 1;
	}
	else if(strcmp ("RIGHT_ONLY", l_txVerify_AUDIO_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
	{
		stereoChannelRelationInt = 2;
	}
	else if(strcmp ("LEFT_EQUALS_RIGHT", l_txVerify_AUDIO_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
	{
		stereoChannelRelationInt = 3;
	}
	else if(strcmp ("LEFT_EQUALS_MINUS_RIGHT", l_txVerify_AUDIO_Param.STEREO_CHANNEL_RELATION_SELECT) == 0)
	{
		stereoChannelRelationInt = 4;
	}
	// All other cases, default to LEFT_EQUALS_RIGHT CHANNEL
	else
	{
		stereoChannelRelationInt = 3;
	}

// Filter Table definition

	map <string, int> filterTypeTable;
	map <string, int>::iterator filterTypeIterator;

	filterTypeTable["NO_FILTER"] = 0;
	filterTypeTable["HPF"] = 1;
	filterTypeTable["LPF"] = 2;
	filterTypeTable["DE_EMPHASIS"] = 3;
	filterTypeTable["CCIT_P53"] = 4;
	filterTypeTable["C_MESSAGE"] = 5;
	filterTypeTable["A_WEIGHTING"] = 6; 
	filterTypeTable["C_WEIGHTING"] = 7;
	filterTypeTable["ITU_R_468_WEIGHTED"] = 8;
	filterTypeTable["ITU_R_468_UNWEIGHTED"] = 9;

	int filterType1Int, filterType2Int;

	filterTypeIterator = filterTypeTable.find(l_txVerify_AUDIO_Param.FILTER_TYPE_1);
	if(filterTypeIterator != filterTypeTable.end())
	{
		filterType1Int = filterTypeIterator->second;
	}
	else
	{
		filterType1Int = 0;
	}
	filterTypeIterator = filterTypeTable.find(l_txVerify_AUDIO_Param.FILTER_TYPE_2);
	if(filterTypeIterator != filterTypeTable.end())
	{
		filterType2Int = filterTypeIterator->second;
	}
	else
	{
		filterType2Int = 0;
	}


		// Check path loss (by ant and freq)
		if ( 0==l_txVerify_AUDIO_Param.CABLE_LOSS_DB )
		{
			err = TM_GetPathLossAtFrequency(g_FM_Test_ID, l_txVerify_AUDIO_Param.CARRIER_FREQ_HZ, &l_txVerify_AUDIO_Param.CABLE_LOSS_DB, 0, TX_TABLE);
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

		vDUT_AddIntegerParameter (g_FM_Dut, "STEREO_CHANNEL_RELATION_SELECT",	    stereoChannelRelationInt);

		vDUT_AddIntegerParameter (g_FM_Dut, "AUDIO_TONE_FREQUENCY_HZ",	    l_txVerify_AUDIO_Param.AUDIO_TONE_FREQUENCY_HZ);

		vDUT_AddIntegerParameter(g_FM_Dut, "CARRIER_FREQ_HZ",		l_txVerify_AUDIO_Param.CARRIER_FREQ_HZ);	

		vDUT_AddDoubleParameter(g_FM_Dut, "CARRIER_POWER_DBM",   l_txVerify_AUDIO_Param.CARRIER_POWER_DBM);

		vDUT_AddIntegerParameter (g_FM_Dut, "AUDIO_DEVIATION_PERCENT",	    l_txVerify_AUDIO_Param.AUDIO_DEVIATION_PERCENT);

		vDUT_AddIntegerParameter(g_FM_Dut, "DUT_TX_SETTLE_TIME_MS",	g_FMGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);

		vDUT_AddIntegerParameter (g_FM_Dut, "PEAK_AUDIO_DEVIATION_HZ",	    l_txVerify_AUDIO_Param.PEAK_AUDIO_DEVIATION_HZ);

		vDUT_AddIntegerParameter (g_FM_Dut, "PEAK_PILOT_DEVIATION_HZ",	    l_txVerify_AUDIO_Param.PEAK_PILOT_DEVIATION_HZ);
	
		vDUT_AddIntegerParameter(g_FM_Dut, "PRE_EMP_TIME_CONSTANT_US",	l_txVerify_AUDIO_Param.PRE_EMP_TIME_CONSTANT_US);

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

#pragma endregion

#pragma region Setup LP Tester and Capture
		/*--------------------*
		 * Setup IQtester VSA *
		 *--------------------*/
		double marginDb = 2;
		cableLossDb = l_txVerify_AUDIO_Param.CABLE_LOSS_DB;
		
		err = LP_FM_SetVsa(l_txVerify_AUDIO_Param.CARRIER_FREQ_HZ,(l_txVerify_AUDIO_Param.CARRIER_POWER_DBM));
		// 4/4/12 trunk merge: IQlite TW trunk uses following
		//err = LP_FM_SetVsa_Agc_On(l_txVerify_AUDIO_Param.CARRIER_FREQ_HZ,(l_txVerify_AUDIO_Param.CARRIER_POWER_DBM - cableLossDb + marginDb));
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Failed to setup FM VSA.\n");
			throw logMessage;
		}		

#pragma endregion

		// Check Capture Time 
		if (0==l_txVerify_AUDIO_Param.SAMPLING_TIME_MS)
		{
			samplingTimemS = 200; // set capture time to 50 ms if capture time is 0
		}
		else	// SAMPLING_TIME_MS != 0
		{
			samplingTimemS = l_txVerify_AUDIO_Param.SAMPLING_TIME_MS;
		}

		/*--------------------------------*
		 * Start "while" loop for average *
		 *--------------------------------*/
		int avgIteration = 0;

		while ( avgIteration<g_FMGlobalSettingParam.TX_AUDIO_AVERAGE )
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Failed to capture signal at %d Hz.\n", l_txVerify_AUDIO_Param.CARRIER_FREQ_HZ);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[FM] LP_FM_VSACapture() at %d Hz return OK.\n", l_txVerify_AUDIO_Param.CARRIER_FREQ_HZ);
			}	

			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;


			/*------------------------------*
			 *  Perform Audio analysis  *
			 *------------------------------*/
			int audioResolutionBandwidth = g_FMGlobalSettingParam.AUDIO_RESOLUTION_BANDWIDTH_HZ;
			int filterCount = 3;
			//The De-emphasis Time Constant is chosen to be the same value as Pre-emphasis time constant
			double deEmphasisTimeConstantUs = (l_txVerify_AUDIO_Param.PRE_EMP_TIME_CONSTANT_US * 1e-6);
			int filterType3 = 3; //De-emphasis filter selection
			double filterParam3 = deEmphasisTimeConstantUs;

			if(0 == stereoEnableInt)
			{
				

				err = LP_FM_Analyze_Audio_Mono(audioResolutionBandwidth,
											   l_txVerify_AUDIO_Param.AUDIO_FREQ_LOW_LIMIT_HZ,
											   l_txVerify_AUDIO_Param.AUDIO_FREQ_HI_LIMIT_HZ,
											   filterCount,
											   filterType1Int, l_txVerify_AUDIO_Param.FILTER_PARAMETER_1,
											   filterType2Int, l_txVerify_AUDIO_Param.FILTER_PARAMETER_2,
											   filterType3, filterParam3);		
				if (ERR_OK!=err)
				{	//Analysis Failure
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] LP_FM_Analyze_Audio_Mono() return error.\n");
					throw logMessage;
				}
				else
				{
					// do nothing
				}
			}
			else
			{
				err = LP_FM_Analyze_Audio_Stereo(audioResolutionBandwidth,
											   l_txVerify_AUDIO_Param.AUDIO_FREQ_LOW_LIMIT_HZ,
											   l_txVerify_AUDIO_Param.AUDIO_FREQ_HI_LIMIT_HZ,
											   filterCount,
											   filterType1Int, l_txVerify_AUDIO_Param.FILTER_PARAMETER_1,
											   filterType2Int, l_txVerify_AUDIO_Param.FILTER_PARAMETER_2,
											   filterType3, filterParam3);	
				if (ERR_OK!=err)
				{	//Analysis Failure
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] LP_FM_Analyze_Audio_Stereo() return error.\n");
					throw logMessage;
				}
				else
				{
					// do nothing
				}
			}

#pragma region Retrieve analysis Results
			/*-----------------------------*
			 *  Retrieve analysis results  *
			 *-----------------------------*/
			analysisOK = true;

				if(0 == stereoEnableInt)
				{
					mono_snr_db[avgIteration] = LP_FM_GetScalarMeasurement("audioAnaSnr", 0);
					mono_sinad_db[avgIteration] = LP_FM_GetScalarMeasurement("audioAnaSinad", 0);

					mono_thd_db[avgIteration] = LP_FM_GetScalarMeasurement("audioAnaThd", 0);
					mono_tnhd_db[avgIteration] = LP_FM_GetScalarMeasurement("audioAnaTnhd", 0);

					if ( NA_NUMBER == mono_snr_db[avgIteration] || NA_NUMBER == mono_sinad_db[avgIteration] ||
					     NA_NUMBER == mono_thd_db[avgIteration] || NA_NUMBER == mono_tnhd_db[avgIteration])
					{
						analysisOK = false;
						l_txVerify_AUDIO_Return.MONO_SNR_DB = NA_NUMBER;
						l_txVerify_AUDIO_Return.MONO_SINAD_DB = NA_NUMBER;
						
						l_txVerify_AUDIO_Return.MONO_THD_DB = NA_NUMBER;
						l_txVerify_AUDIO_Return.MONO_TNHD_DB = NA_NUMBER;

						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_FM_GetScalarMeasurement(\"AUDIO\") return error.\n");
						throw logMessage;
					}
					else
					{
						mono_snr_db[avgIteration] = mono_snr_db[avgIteration];	
						mono_sinad_db[avgIteration] = mono_sinad_db[avgIteration];
						mono_thd_db[avgIteration] = mono_thd_db[avgIteration];	
						mono_tnhd_db[avgIteration] = mono_tnhd_db[avgIteration];
	 
					}
				}
				else
				{
					left_snr_db[avgIteration] = LP_FM_GetScalarMeasurement("leftAudioAnaSnr", 0);
					right_snr_db[avgIteration] = LP_FM_GetScalarMeasurement("rightAudioAnaSnr", 0);

					left_sinad_db[avgIteration] = LP_FM_GetScalarMeasurement("leftAudioAnaSinad", 0);
					right_sinad_db[avgIteration] = LP_FM_GetScalarMeasurement("rightAudioAnaSinad", 0);
					
					stereo_crosstalk_db[avgIteration] = LP_FM_GetScalarMeasurement("crosstalkCrosstalk", 0);
					pilot_freq_offset_hz[avgIteration] = LP_FM_GetScalarMeasurement("pilotFrequencyOffset", 0);

					left_thd_db[avgIteration] = LP_FM_GetScalarMeasurement("leftAudioAnaThd", 0);
					right_thd_db[avgIteration] = LP_FM_GetScalarMeasurement("rightAudioAnaThd", 0);
					
					left_tnhd_db[avgIteration] = LP_FM_GetScalarMeasurement("leftAudioAnaTnhd", 0);
					right_tnhd_db[avgIteration] = LP_FM_GetScalarMeasurement("rightAudioAnaTnhd", 0);
					
					if ( NA_NUMBER == left_snr_db[avgIteration] || NA_NUMBER == right_snr_db[avgIteration] ||
						NA_NUMBER == left_sinad_db[avgIteration] || NA_NUMBER == right_sinad_db[avgIteration] || 
						NA_NUMBER == stereo_crosstalk_db[avgIteration] || NA_NUMBER == pilot_freq_offset_hz[avgIteration] ||
						NA_NUMBER == left_thd_db[avgIteration] || NA_NUMBER == right_thd_db[avgIteration] ||
						NA_NUMBER == left_tnhd_db[avgIteration] || NA_NUMBER == right_tnhd_db[avgIteration])
					{
						analysisOK = false;
						l_txVerify_AUDIO_Return.LEFT_SNR_DB = NA_NUMBER;
						l_txVerify_AUDIO_Return.RIGHT_SNR_DB = NA_NUMBER;
						l_txVerify_AUDIO_Return.LEFT_SINAD_DB = NA_NUMBER;
						l_txVerify_AUDIO_Return.RIGHT_SINAD_DB = NA_NUMBER;
						l_txVerify_AUDIO_Return.STEREO_CROSSTALK_DB = NA_NUMBER;
						l_txVerify_AUDIO_Return.PILOT_FREQ_OFFSET_HZ = NA_NUMBER;

						l_txVerify_AUDIO_Return.LEFT_THD_DB = NA_NUMBER;
						l_txVerify_AUDIO_Return.RIGHT_THD_DB = NA_NUMBER;
						l_txVerify_AUDIO_Return.LEFT_TNHD_DB = NA_NUMBER;
						l_txVerify_AUDIO_Return.RIGHT_TNHD_DB = NA_NUMBER;

						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_FM_GetScalarMeasurement(\"AUDIO\") return error.\n");
						throw logMessage;
					}
					else
					{
						left_snr_db[avgIteration] = left_snr_db[avgIteration];	
						right_snr_db[avgIteration] = right_snr_db[avgIteration];
						left_sinad_db[avgIteration] = left_sinad_db[avgIteration];	
						right_sinad_db[avgIteration] = right_sinad_db[avgIteration];
						stereo_crosstalk_db[avgIteration] = stereo_crosstalk_db[avgIteration];	
						pilot_freq_offset_hz[avgIteration] = pilot_freq_offset_hz[avgIteration];

						left_thd_db[avgIteration] = left_thd_db[avgIteration];	
						right_thd_db[avgIteration] = right_thd_db[avgIteration];
						left_tnhd_db[avgIteration] = left_tnhd_db[avgIteration];	
						right_tnhd_db[avgIteration] = right_tnhd_db[avgIteration];

					}
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
			if(0 == stereoEnableInt)
			{
				err = ::AverageTestResult(&mono_snr_db[0], avgIteration, LOG_20, l_txVerify_AUDIO_Return.MONO_SNR_DB, dummyMax, dummyMin);
				err = ::AverageTestResult(&mono_sinad_db[0], avgIteration, LOG_20, l_txVerify_AUDIO_Return.MONO_SINAD_DB, dummyMax, dummyMin);
				err = ::AverageTestResult(&mono_thd_db[0], avgIteration, LOG_20, l_txVerify_AUDIO_Return.MONO_THD_DB, dummyMax, dummyMin);
				err = ::AverageTestResult(&mono_tnhd_db[0], avgIteration, LOG_20, l_txVerify_AUDIO_Return.MONO_TNHD_DB, dummyMax, dummyMin);
			}
			else
			{
				err = ::AverageTestResult(&left_snr_db[0], avgIteration, LOG_20, l_txVerify_AUDIO_Return.LEFT_SNR_DB, dummyMax, dummyMin);
				err = ::AverageTestResult(&right_snr_db[0], avgIteration, LOG_20, l_txVerify_AUDIO_Return.RIGHT_SNR_DB, dummyMax, dummyMin);
				err = ::AverageTestResult(&left_sinad_db[0], avgIteration, LOG_20, l_txVerify_AUDIO_Return.LEFT_SINAD_DB, dummyMax, dummyMin);
				err = ::AverageTestResult(&right_sinad_db[0], avgIteration, LOG_20, l_txVerify_AUDIO_Return.RIGHT_SINAD_DB, dummyMax, dummyMin);
				err = ::AverageTestResult(&stereo_crosstalk_db[0], avgIteration, LOG_20,l_txVerify_AUDIO_Return.STEREO_CROSSTALK_DB, dummyMax , dummyMin);
				err = ::AverageTestResult(&pilot_freq_offset_hz[0], avgIteration, LOG_20,l_txVerify_AUDIO_Return.PILOT_FREQ_OFFSET_HZ, dummyMax , dummyMin);
				err = ::AverageTestResult(&left_thd_db[0], avgIteration, LOG_20, l_txVerify_AUDIO_Return.LEFT_THD_DB, dummyMax, dummyMin);
				err = ::AverageTestResult(&right_thd_db[0], avgIteration, LOG_20, l_txVerify_AUDIO_Return.RIGHT_THD_DB, dummyMax, dummyMin);
				err = ::AverageTestResult(&left_tnhd_db[0], avgIteration, LOG_20, l_txVerify_AUDIO_Return.LEFT_TNHD_DB, dummyMax, dummyMin);
				err = ::AverageTestResult(&right_tnhd_db[0], avgIteration, LOG_20,l_txVerify_AUDIO_Return.RIGHT_TNHD_DB, dummyMax , dummyMin);
			}
			

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
			l_txVerify_AUDIO_Return.CABLE_LOSS_DB = l_txVerify_AUDIO_Param.CABLE_LOSS_DB;

			sprintf_s(l_txVerify_AUDIO_Return.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerify_AUDIO_ReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txVerify_AUDIO_Return.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_txVerify_AUDIO_Return.ERROR_MESSAGE, "[FM] Unknown Error!\n");
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
    mono_snr_db.clear();
	mono_sinad_db.clear();
	left_snr_db.clear();
	right_snr_db.clear();
	left_sinad_db.clear();
    right_sinad_db.clear();
	stereo_crosstalk_db.clear();
	pilot_freq_offset_hz.clear();

    mono_thd_db.clear();
	mono_tnhd_db.clear();
	left_thd_db.clear();
	right_thd_db.clear();
	left_tnhd_db.clear();
    right_tnhd_db.clear();


    return err;
}


void InitializeTXVerify_AUDIO_Containers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txVerify_AUDIO_ParamMap.clear();

    FM_SETTING_STRUCT setting;

    l_txVerify_AUDIO_Param.CARRIER_FREQ_HZ = 98100000;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_AUDIO_Param.CARRIER_FREQ_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.CARRIER_FREQ_HZ;
        setting.unit        = "Hz";
        setting.helpText    = "FM Carrier frequency in Hz";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CARRIER_FREQ_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	
	l_txVerify_AUDIO_Param.CARRIER_POWER_DBM = -40.0;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_AUDIO_Param.CARRIER_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.CARRIER_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "FM Carrier Peak power in dBm";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CARRIER_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txVerify_AUDIO_Param.STEREO_ENABLE, MAX_BUFFER_SIZE, "OFF");
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerify_AUDIO_Param.STEREO_ENABLE))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.STEREO_ENABLE;
        setting.unit        = " ";
		setting.helpText    = "Enable / Disable Stereo :OFF, ON";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("STEREO_ENABLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerify_AUDIO_Param.PEAK_AUDIO_DEVIATION_HZ = 67500;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_AUDIO_Param.PEAK_AUDIO_DEVIATION_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.PEAK_AUDIO_DEVIATION_HZ;
        setting.unit        = "Hz ";
		setting.helpText    = "Peak Audio Deviation in Hz";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("PEAK_AUDIO_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerify_AUDIO_Param.PEAK_PILOT_DEVIATION_HZ = 7500;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_AUDIO_Param.PEAK_PILOT_DEVIATION_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.PEAK_PILOT_DEVIATION_HZ;
        setting.unit        = "Hz";
		setting.helpText    = "Peak Pilot Deviation in Hz. Applies only if Stereo is Enabled";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("PEAK_PILOT_DEVIATION_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txVerify_AUDIO_Param.STEREO_CHANNEL_RELATION_SELECT, MAX_BUFFER_SIZE, "LEFT_ONLY");
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerify_AUDIO_Param.STEREO_CHANNEL_RELATION_SELECT))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.STEREO_CHANNEL_RELATION_SELECT;
        setting.unit        = " ";
		setting.helpText    = "Select Left Right Channel Relation. Valid only if Stereo is enabled";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("STEREO_CHANNEL_RELATION_SELECT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerify_AUDIO_Param.AUDIO_DEVIATION_PERCENT = 100;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_AUDIO_Param.AUDIO_DEVIATION_PERCENT))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.AUDIO_DEVIATION_PERCENT;
        setting.unit        = "percent";
		setting.helpText    = "Audio Deviation as a percentage of Peak Audio Deviation. This scales the audio input signal to the DUT";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("AUDIO_DEVIATION_PERCENT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerify_AUDIO_Param.AUDIO_TONE_FREQUENCY_HZ = 1000;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_AUDIO_Param.AUDIO_TONE_FREQUENCY_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.AUDIO_TONE_FREQUENCY_HZ;
        setting.unit        = "Hz ";
		setting.helpText    = "Audio Tone Frequency in Hz";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("AUDIO_TONE_FREQUENCY_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerify_AUDIO_Param.AUDIO_FREQ_LOW_LIMIT_HZ = 300;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_AUDIO_Param.AUDIO_FREQ_LOW_LIMIT_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.AUDIO_FREQ_LOW_LIMIT_HZ;
        setting.unit        = "Hz ";
		setting.helpText    = "Set the Lower bound for analyzed audio range(Hz)";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("AUDIO_FREQ_LOW_LIMIT_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	
	l_txVerify_AUDIO_Param.AUDIO_FREQ_HI_LIMIT_HZ = 15000;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_AUDIO_Param.AUDIO_FREQ_HI_LIMIT_HZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.AUDIO_FREQ_HI_LIMIT_HZ;
        setting.unit        = "Hz ";
		setting.helpText    = "Set the Upper bound for analyzed audio range(Hz)";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("AUDIO_FREQ_HI_LIMIT_HZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txVerify_AUDIO_Param.FILTER_TYPE_1, MAX_BUFFER_SIZE, "LPF");
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerify_AUDIO_Param.FILTER_TYPE_1))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.FILTER_TYPE_1;
        setting.unit        = " ";
		setting.helpText    = "Select filter 1 to be applied to the audio analysis 0: NO_FILTER, 1: HPF, 2: LPF,3:DE_EMPHASIS 4: CCIT_P53,5: C_MESSAGE, 6: A_WEIGHTING, 7: C_WEIGHTING, 8: ITU_R_468_WEIGHTED, 9: ITU_R_468_UNWEIGHTED";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("FILTER_TYPE_1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerify_AUDIO_Param.FILTER_PARAMETER_1 = 15000;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_AUDIO_Param.FILTER_PARAMETER_1))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.FILTER_PARAMETER_1;
        setting.unit        = "Hz ";
		setting.helpText    = "Filter parameter value corresponding to FILTER_TYPE_1, FITER_PARAMETER_1 for HPF = 50(3dB cut-off frequency in Hz), FITER_PARAMETER_1 for LPF = 20000(3dB cut-off frequency in Hz), Other filters doesn't need Filter parameters to be specified";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("FILTER_PARAMETER_1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txVerify_AUDIO_Param.FILTER_TYPE_2, MAX_BUFFER_SIZE, "HPF");
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerify_AUDIO_Param.FILTER_TYPE_2))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.FILTER_TYPE_2;
        setting.unit        = " ";
		setting.helpText    = "Select filter 2 to be applied to the audio analysis 0: NO_FILTER, 1: HPF, 2: LPF, 4: CCIT_P53,5: C_MESSAGE, 6: A_WEIGHTING, 7: C_WEIGHTING, 8: ITU_R_468_WEIGHTED, 9: ITU_R_468_UNWEIGHTED";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("FILTER_TYPE_2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


	l_txVerify_AUDIO_Param.FILTER_PARAMETER_2 = 300;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_AUDIO_Param.FILTER_PARAMETER_2))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerify_AUDIO_Param.FILTER_PARAMETER_2;
        setting.unit        = "Hz ";
		setting.helpText    = "Filter parameter value corresponding to FILTER_TYPE_1, FITER_PARAMETER_1 for HPF = 50(3dB cut-off frequency in Hz), FITER_PARAMETER_1 for LPF = 20000(3dB cut-off frequency in Hz), Other filters doesn't need Filter parameters to be specified";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("FILTER_PARAMETER_2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerify_AUDIO_Param.CABLE_LOSS_DB = 0.0;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Param.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerify_AUDIO_Param.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_txVerify_AUDIO_Param.SAMPLING_TIME_MS = 200.0;
    setting.type = FM_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerify_AUDIO_Param.SAMPLING_TIME_MS))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_AUDIO_Param.SAMPLING_TIME_MS;
        setting.unit        = "ms";
        setting.helpText    = "Capture time in milli-seconds";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("SAMPLING_TIME_MS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerify_AUDIO_Param.PRE_EMP_TIME_CONSTANT_US = 0;
    setting.type = FM_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerify_AUDIO_Param.PRE_EMP_TIME_CONSTANT_US))    // Type_Checking
    {
        setting.value = (void*)&l_txVerify_AUDIO_Param.PRE_EMP_TIME_CONSTANT_US;
        setting.unit        = "us";
        setting.helpText    = "DUT Supported Pre-Emphasis Time Constant ";
        l_txVerify_AUDIO_ParamMap.insert( pair<string,FM_SETTING_STRUCT>("PRE_EMP_TIME_CONSTANT_US", setting) );
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
    l_txVerify_AUDIO_ReturnMap.clear();


	l_txVerify_AUDIO_Return.MONO_SNR_DB = NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.MONO_SNR_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.MONO_SNR_DB;
		setting.unit        = "dB";
		setting.helpText    = "SNR in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("MONO_SNR_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerify_AUDIO_Return.MONO_SINAD_DB = NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.MONO_SINAD_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.MONO_SINAD_DB;
		setting.unit        = "dB";
		setting.helpText    = "SINAD in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("MONO_SINAD_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerify_AUDIO_Return.LEFT_SNR_DB= NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.LEFT_SNR_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.LEFT_SNR_DB;
		setting.unit        = "dB";
		setting.helpText    = "LEFT SNR in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("LEFT_SNR_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}



	l_txVerify_AUDIO_Return.RIGHT_SNR_DB= NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.RIGHT_SNR_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.RIGHT_SNR_DB;
		setting.unit        = "dB";
		setting.helpText    = "RIGHT SNR in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("RIGHT_SNR_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerify_AUDIO_Return.LEFT_SINAD_DB= NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.LEFT_SINAD_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.LEFT_SINAD_DB;
		setting.unit        = "dB";
		setting.helpText    = "LEFT SINAD in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("LEFT_SINAD_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}



	l_txVerify_AUDIO_Return.RIGHT_SINAD_DB= NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.RIGHT_SINAD_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.RIGHT_SINAD_DB;
		setting.unit        = "dB";
		setting.helpText    = "RIGHT SINAD in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("RIGHT_SINAD_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerify_AUDIO_Return.STEREO_CROSSTALK_DB = NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.STEREO_CROSSTALK_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.STEREO_CROSSTALK_DB;
		setting.unit        = "dB";
		setting.helpText    = "MAXIMUM CROSS TALK between L and R Channel in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("STEREO_CROSSTALK_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerify_AUDIO_Return.PILOT_FREQ_OFFSET_HZ = NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.PILOT_FREQ_OFFSET_HZ))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.PILOT_FREQ_OFFSET_HZ;
		setting.unit        = "Hz";
		setting.helpText    = "Pilot Frequency Offset in Hz";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("PILOT_FREQ_OFFSET_HZ", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerify_AUDIO_Return.MONO_THD_DB = NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.MONO_THD_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.MONO_THD_DB;
		setting.unit        = "dB";
		setting.helpText    = "MONO Total Harmonic in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("MONO_THD_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerify_AUDIO_Return.MONO_TNHD_DB= NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.MONO_TNHD_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.MONO_TNHD_DB;
		setting.unit        = "dB";
		setting.helpText    = "MONO Total Non Harmonic Distortion in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("MONO_TNHD_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerify_AUDIO_Return.LEFT_THD_DB= NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.LEFT_THD_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.LEFT_THD_DB;
		setting.unit        = "dB";
		setting.helpText    = "LEFT Total Non Harmonic Distortion in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("LEFT_THD_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerify_AUDIO_Return.RIGHT_THD_DB= NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.RIGHT_THD_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.RIGHT_THD_DB;
		setting.unit        = "dB";
		setting.helpText    = "RIGHT Total Non Harmonic Distortion in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("RIGHT_THD_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerify_AUDIO_Return.LEFT_TNHD_DB= NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.LEFT_TNHD_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.LEFT_TNHD_DB;
		setting.unit        = "dB";
		setting.helpText    = "LEFT Total Non Harmonic Distortion in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("LEFT_TNHD_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


    l_txVerify_AUDIO_Return.RIGHT_TNHD_DB= NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.RIGHT_TNHD_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txVerify_AUDIO_Return.RIGHT_TNHD_DB;
		setting.unit        = "dB";
		setting.helpText    = "RIGHT Total Non Harmonic Distortion in dB";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("RIGHT_TNHD_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerify_AUDIO_Return.CABLE_LOSS_DB = NA_DOUBLE;
	setting.type = FM_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerify_AUDIO_Return.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerify_AUDIO_Return.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
    
	// Error Message Return String
    l_txVerify_AUDIO_Return.ERROR_MESSAGE[0] = '\0';
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerify_AUDIO_Return.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerify_AUDIO_Return.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_txVerify_AUDIO_ReturnMap.insert( pair<string,FM_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}


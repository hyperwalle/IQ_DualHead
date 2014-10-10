#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_MiMo_Test.h"
#include "WiFi_MiMo_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include <math.h> // fmod on mac
#include "StringUtil.h"

#define INVALID_EVM		-99.00


using namespace std;

// These global variables are declared in WiFi_Test_Internal.cpp
extern TM_ID				g_WiFi_Test_ID;    
extern vDUT_ID				g_WiFi_Dut;
extern int					g_Tester_Type;
extern int					g_Tester_Number;
extern bool					g_vDutTxActived;
extern bool					g_dutConfigChanged;
extern bool                 g_txCalibrationEnabled;
extern WIFI_RECORD_PARAM	g_RecordedParam;
extern double				g_last_TxPower_dBm_Record;

// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_globalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyEvmParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyEvmReturnMap;

struct tagParam
{
    // Mandatory Parameters
    int    FREQ_MHZ;                                        /*!< The center frequency (MHz). */
    char   BANDWIDTH[MAX_BUFFER_SIZE];                      /*!< The RF bandwidth to verify EVM. */
    char   DATA_RATE[MAX_BUFFER_SIZE];                      /*!< The data rate to verify EVM. */
	char   PREAMBLE[MAX_BUFFER_SIZE];                       /*!< The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];              /*!< The packet format of 11N(only). */
	// IQlite merge; Tracy Yu ; 2012-03-31
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];				/*! The guard interval format of 11N(only). */
    char   TX_ANALYSIS_ORDER[MAX_BUFFER_SIZE];              /*!< The analysis order to verify EVM */
    double TX_POWER_DBM;                                    /*!< The output power to verify EVM. */
    double CABLE_LOSS_DB[MAX_TESTER_NUM];                   /*!< The path loss of test system. */
    double SAMPLING_TIME_US;                                /*!< The sampling time to verify EVM. */

    char   VSA_CONNECTION[MAX_TESTER_NUM][MAX_BUFFER_SIZE]; /*!< The vsa connect to which antenn port. */ 
    char   VSA_PORT[MAX_TESTER_NUM][MAX_BUFFER_SIZE];		/*!< IQTester VSAs port setting. Default=PORT_LEFT. */  

    // DUT Parameters
    int    TX1;                                             /*!< DUT TX1 on/off. Default=1(on) */
    int    TX2;                                             /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                             /*!< DUT TX3 on/off. Default=0(off) */
    int    TX4;                                             /*!< DUT TX4 on/off. Default=0(off) */
} l_txVerifyEvmParam;

struct tagReturn
{
	int    EVM_AVERAGE;												/*!< Number of packets for evm averaging during evm verification */
    // EVM Test Result 
    double EVM_AVG_DB;    
	double EVM_MAX_DB;
	double EVM_MIN_DB;
    double EVM_AVG[MAX_DATA_STREAM];                                /*!< (Average) EVM test result on specific data stream. Format: EVM_AVG[SpecificStream] */
    double EVM_MAX[MAX_DATA_STREAM];                                /*!< (Maximum) EVM test result on specific data stream. Format: EVM_MAX[SpecificStream] */
    double EVM_MIN[MAX_DATA_STREAM];                                /*!< (Minimum) EVM test result on specific data stream. Format: EVM_MIN[SpecificStream] */
  
    // EVM_PK Test Result 
    double EVM_PK_DB;							   	                /*!< (11b only)EVM_PK over captured packets. */                  

    // POWER Test Result 
    double POWER_AVG_DBM;                  
    double POWER_AVG[MAX_DATA_STREAM];                              /*!< (Average) POWER test result on specific data stream. Format: POWER_AVG[SpecificStream] */
    double POWER_MAX[MAX_DATA_STREAM];                              /*!< (Maximum) POWER test result on specific data stream. Format: POWER_MAX[SpecificStream] */
    double POWER_MIN[MAX_DATA_STREAM];                              /*!< (Minimum) POWER test result on specific data stream. Format: POWER_MIN[SpecificStream] */

    double FREQ_ERROR_AVG[MAX_DATA_STREAM];                         /*!< (Average) Frequency Error, unit is ppm */
    double FREQ_ERROR_MAX[MAX_DATA_STREAM];                         /*!< (Maximum) Frequency Error, unit is ppm */
    double FREQ_ERROR_MIN[MAX_DATA_STREAM];                         /*!< (Minimum) Frequency Error, unit is ppm */

	double SYMBOL_CLK_ERR[MAX_DATA_STREAM];

	double AMP_ERR_DB[MAX_DATA_STREAM];							    /*!< IQ Match Amplitude Error in dB on each stream. */
    double AMP_ERR_DB_ALL;                                          /*!< IQ Match Amplitude Error in dB. */
	double PHASE_ERR[MAX_DATA_STREAM];								/*!< IQ Match Phase Error(degree) on each stream. */
	double PHASE_ERR_ALL;
    double PHASE_NOISE_RMS_ALL;                                     /*!< Frequency RMS Phase Noise. */ 


	int	   SPATIAL_STREAM;                                          /*!< Analyzed stream number. */	
	double DATA_RATE;                                               /*!< Analyzed data rate. */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];                          /*!< The path loss of test system. */

    //FOR 802.11N MULTI-TESTER
    vector< double>                      streamRmsAvg;              /*!< (Average) all VSA average RMS power in speicified stream. */
    vector< double>                      streamRmsMax;              /*!< (Maximum) all VSA maximum RMS power in speicified stream. */
    vector< double>                      streamRmsMin;              /*!< (Minimum) all VSA minimum RMS power in speicified stream. */
    vector< double>                      vsaRmsAvg;                 /*!< (Average) all streams average RMS power in speicified VSA. */
    vector< double>                      vsaRmsMax;                 /*!< (Maximum) all streams maximum RMS power in speicified VSA. */
    vector< double>                      vsaRmsMin;                 /*!< (Minimum) all streams minimum RMS power in speicified VSA. */

    double streamAvgAll;
    double streamMaxAll;
    double streamMinAll;

    double vsaAvgAll;
    double vsaMaxAll;
    double vsaMinAll;

	double STREAM_POWER_IN_VSA[MAX_DATA_STREAM][MAX_TESTER_NUM];			/*!< (Average) power of stream n (Sn) measured using VS m (VSAm).*/


	double isolationAvgInTwoStreams[MAX_DATA_STREAM][MAX_DATA_STREAM];/*!< (Average) isoltaion between two streams*/

    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyEvmReturn;
#pragma endregion

#ifndef WIN32
int initTxVerifyEvmMIMOContainer = InitializeTXVerifyEvmContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Evm.cpp
int CheckTxEvmParameters( int *channel, 
						  int *wifiMode, 
						  int *wifiStreamNum, 
						  double *samplingTimeUs, 
						  int *vsaEnabled,
						  double *peakToAvgRatio, 
						  char* errorMsg );


//! WiFi TX Verify EVM
/*!
* Input Parameters
*
*  - Mandatory 
*      -# FREQ_MHZ (double): The center frequency (MHz)
*      -# DATA_RATE (string): The data rate to verify EVM
*      -# TX_POWER_DBM (double): The power (dBm) DUT is going to transmit at the antenna port
*
* Return Values
*      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/

WIFI_MIMO_TEST_API int WiFi_TX_Verify_Evm(void)
{
    int    err = ERR_OK;

    bool   analysisOK = false, captureOK  = false;     
    int    avgIteration = 0;
    int    channel = 0, HT40ModeOn = 0, packetFormat = 0;
    int    dummyValue = 0;
    int    wifiMode = 0, wifiStreamNum = 0;
	int	   antSelection = 0;
    
    double samplingTimeUs = 0, peakToAvgRatio = 0, cableLossDb = 0;
	char   vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};
    int    vsaMappingTx[MAX_CHAIN_NUM]={0};
    int    txEnabled[MAX_CHAIN_NUM]={0};
	int	   vsaPortStatus[MAX_TESTER_NUM];
	int    specifiedAnalysisOrder = 0;
	double measuredPower;
	int				validVsaNum = 0;
	vector<int>		prefOrderSignal;


    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_txVerifyEvmReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
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
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_RETURN", &dummyValue);
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
	vector< vector<double> >    evmAvgAll(MAX_DATA_STREAM, vector<double>(g_globalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    rxRmsPowerDb(MAX_DATA_STREAM, vector<double>(g_globalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    evmPk(MAX_DATA_STREAM, vector<double>(g_globalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    freqErrorHz(MAX_DATA_STREAM, vector<double>(g_globalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    symbolClockErr(MAX_DATA_STREAM, vector<double>(g_globalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    ampErrDb(MAX_DATA_STREAM, vector<double>(g_globalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    phaseErr(MAX_DATA_STREAM, vector<double>(g_globalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    phaseNoiseRms(MAX_DATA_STREAM, vector<double>(g_globalSettingParam.EVM_AVERAGE));
    vector<double>              evmAllPacket(vector<double>(g_globalSettingParam.EVM_AVERAGE));

    vector<string> splits;
    vector<string>::iterator splitsIter;

	/*---------------------------------------*
     * Turn Off VSG parameter			     *
     *---------------------------------------*/
	vector<int>					vsgRFEnabled(MAX_TESTER_NUM, (int)NA_NUMBER);
    /*----------------------------------------------------------------------------------------
     * FOR 802.11N MULTI-TESTER
	 *-----------------------------------------------------------------------------------------*/

	vector<double>				rfAmplDb(MAX_TESTER_NUM, NA_NUMBER);
    vector< vector<doubleV> >   streamRmsInEachBurst(MAX_DATA_STREAM, vector<doubleV>(g_globalSettingParam.EVM_AVERAGE, doubleV(MAX_TESTER_NUM, NA_NUMBER)));
    vector< doubleV >           streamRmsSumInEachBurst(MAX_DATA_STREAM, doubleV(g_globalSettingParam.EVM_AVERAGE, NA_NUMBER));
    vector< vector<doubleV> >   vsaRmsInEachBurst(MAX_TESTER_NUM, vector<doubleV>(g_globalSettingParam.EVM_AVERAGE, doubleV(MAX_DATA_STREAM, NA_NUMBER)));
    vector< doubleV>            vsaRmsSumInEachBurst(MAX_TESTER_NUM, doubleV(g_globalSettingParam.EVM_AVERAGE, NA_NUMBER));
    vector< vector<doubleV> >   isolationInTwoStreams(MAX_DATA_STREAM, vector<doubleV>(MAX_DATA_STREAM, doubleV(g_globalSettingParam.EVM_AVERAGE, NA_NUMBER)));
    vector< vector<doubleV> >   streamInVsa(MAX_DATA_STREAM, vector<doubleV>(MAX_TESTER_NUM, doubleV(g_globalSettingParam.EVM_AVERAGE, NA_NUMBER)));

	
	try
	{
	   /*-----------------------------------------------------------*
		* Both g_WiFi_Test_ID and g_WiFi_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_WiFi_Test_ID<0 || g_WiFi_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID or WiFi_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		}
		
		TM_ClearReturns(g_WiFi_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_txVerifyEvmParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get input parameters return OK.\n");
		}

        // Error return of this function is irrelevant
        CheckDutStatus();

		




		// comment out by Kenual 11/10/2010, to allow NxN to capture SISO signal at Tx2 instead of Tx1
		////Check connection between VSAMaster and antenna port
		//err = CheckConnectionVsaMasterAndAntennaPort(l_txVerifyEvmParamMap);
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Connection between VSAMaster and antenna port is not existed.\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Connection between VSAMaster and antenna port is existed.\n");
		//}
		        
        //Map antenna port to Vsa number
        err = CheckConnectionVsaAndAntennaPort(l_txVerifyEvmParamMap, vsaMappingTx, vsaPortStatus);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Mapping VSA to antenna port failed\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Mapping VSA to antenna port OK.\n");
		}

		//For Support Dut Chain Status, 0 = OFF, 1 = Main Antenna, 2 = Aux 
		txEnabled[0] = (l_txVerifyEvmParam.TX1==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[1] = (l_txVerifyEvmParam.TX2==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[2] = (l_txVerifyEvmParam.TX3==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[3] = (l_txVerifyEvmParam.TX4==0?Dut_Chain_Disabled:Dut_Chain_Enabled);

        validVsaNum = txEnabled[0] + txEnabled[1] + txEnabled[2] + txEnabled[3];

#pragma region Prepare input parameters

		//According Tx enabled status which VSA connect to.
		//By order pass into "CheckTxEvmParameters()
		int vsaEnabled[MAX_TESTER_NUM];
		for(int i=0;i<MAX_TESTER_NUM;i++)
		{
			vsaEnabled[i] = txEnabled[vsaMappingTx[i]-1];
			vsgRFEnabled[i] = 0;
		}

		//Turn Off VSG in case signal out after initial testers
		//Tracy Yu; Move to the front of this function, to enable loop back test ;2012-04-06
		err = ::LP_EnableVsgRFNxN(vsgRFEnabled[0], vsgRFEnabled[1], vsgRFEnabled[2], vsgRFEnabled[3]);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Fail to setup VSA, LP_EnableVsgRFNxN() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_EnableVsgRFNxN() return OK.\n");
		}



		err = CheckTxEvmParameters( &channel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &vsaEnabled[0], &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxEvmParameters() return OK.\n");
		}

        if (0==strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, "MIXED"))
		{
            packetFormat = 1;
		}
        else if (0 == strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, "GREENFIELD"))
		{
            packetFormat = 2; 
		}
        else
		{
            packetFormat = 0;
		}

#pragma endregion

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

		if( wifiMode==WIFI_11N_HT40 )
		{
			HT40ModeOn = 1;   // 1: HT40 mode;
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txVerifyEvmParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyEvmParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyEvmParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode 
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txVerifyEvmParam.FREQ_MHZ);
		}		  
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txVerifyEvmParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txVerifyEvmParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_txVerifyEvmParam.GUARD_INTERVAL_11N);  // IQlite merge; Tracy Yu ; 2012-03-31
		//vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txVerifyEvmParam.TX_POWER_DBM);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txVerifyEvmParam.DATA_RATE);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txVerifyEvmParam.BANDWIDTH);   // IQlite merge; Tracy Yu ; 2012-03-31
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txVerifyEvmParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txVerifyEvmParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txVerifyEvmParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txVerifyEvmParam.TX4);
        vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs);

		// IQlite merge; Tracy Yu ; 2012-03-31		
		if ( (l_txVerifyEvmParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
		{ // Indicate that the Tx target power will be determined by channel and data rate, rather than the value input by users in IQwizard 
			if ( g_dutConfigChanged == true )
			{
				/*-----------*
				*  Tx Stop  *
				*-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
				}

				/*----------------------*
				*  Get Tx Target Power  *
				*-----------------------*/
				double tx_target_power = 0;
				err = ::vDUT_Run(g_WiFi_Dut, "GET_TX_TARGET_POWER");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				   {
					   err = -1;	// set err to -1, means "Error".
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					   throw logMessage;
				   }
				   else	// Just return normal error message in this case
				   {
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(GET_TX_TARGET_POWER) return error.\n");
					   throw logMessage;
				   }
				}
				else
				{  
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(GET_TX_TARGET_POWER) return OK.\n");
				}
				vDUT_GetDoubleParameter (g_WiFi_Dut, "TX_TARGET_POWER_DBM", &tx_target_power);
				l_txVerifyEvmParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txVerifyEvmParam.TX_POWER_DBM;
			}
			else
			{
				l_txVerifyEvmParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM", l_txVerifyEvmParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", l_txVerifyEvmParam.TX_POWER_DBM);


		//Calculate valid Test Number

		
        //For code reuse by TX_Calibration, for skipping dut settings
        //Because in calibration, dut setting is controlled by dut layer
        //if(g_txCalibrationEnabled)
        //{
        //    //Means: Calibration is running, skipping dut config.
        //    g_dutConfigChanged = false;
        //    g_vDutTxActived = true;
        //}
        //else
        //{
        //    //keep g_dutConfigchanged and g_vDutTxActived previous status.
        //}

        if ( (g_dutConfigChanged==true) || (g_vDutTxActived==false))	
		{
			if ( g_vDutTxActived==true )
			{
			   /*-----------*
				*  Tx Stop  *
				*-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
				}
			}
			else
			{
				// continue Dut configuration				
			}

			err = vDUT_Run(g_WiFi_Dut, "RF_SET_FREQ");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RF_SET_FREQ) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RF_SET_FREQ) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_SET_BW");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_BW) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_BW) return OK.\n");
			}
			err = vDUT_Run(g_WiFi_Dut, "TX_SET_DATA_RATE");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_DATA_RATE) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_DATA_RATE) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_SET_ANTENNA");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_ANTENNA) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_ANTENNA) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_PRE_TX");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_PRE_TX) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_PRE_TX) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_START");
			if ( ERR_OK!=err )
			{	
			   g_vDutTxActived = false;
			   // Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_START) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{
			   g_vDutTxActived = true;
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_START) return OK.\n");
			}
			// Delay for DUT settle
			if (0!=g_globalSettingParam.DUT_TX_SETTLE_TIME_MS)
			{
				Sleep(g_globalSettingParam.DUT_TX_SETTLE_TIME_MS);
			}
			else
			{
				// do nothing
			}

		}
		else
		{
			// g_dutConfigChanged==false, do nothing
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Since g_dutConfigChanged = false, skip Dut control section.\n");
		}


#pragma endregion

#pragma region Setup LP Tester and Capture
		/*--------------------*
		 * Setup IQTester VSA *
		 *--------------------*/
		err = ::LP_SetVsaAmplitudeTolerance(g_globalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);		
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to set VSA amplitude tolerance in dB.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsaAmplitudeTolerance(%.3f) return OK.\n", g_globalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		}

		//Calculate each VSA Max Reference Level
		/*err = ::LP_SetVsa(  l_txVerifyEvmParam.FREQ_MHZ*1e6,
							l_txVerifyEvmParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio,
							g_globalSettingParam.VSA_PORT,
							0,
							g_globalSettingParam.VSA_TRIGGER_LEVEL_DB,
							g_globalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000	
						  );
		*/
		for(int i = 0; i<g_Tester_Number;i++)
		{
			//if(l_txVerifyEvmParam.CABLE_LOSS_DB[i] >=0)
			//{
				rfAmplDb[i] = l_txVerifyEvmParam.TX_POWER_DBM-l_txVerifyEvmParam.CABLE_LOSS_DB[i]+peakToAvgRatio;
			//}
			//else
			//{
			//	rfAmplDb[i] = l_txVerifyEvmParam.TX_POWER_DBM-l_txVerifyEvmParam.CABLE_LOSS_DB[0]+peakToAvgRatio;
			//}
            vsgRFEnabled[i] = 0; // Disable VSG

		}

		err = ::LP_SetVsaNxN(  l_txVerifyEvmParam.FREQ_MHZ*1e6,
								&rfAmplDb[0],
								&vsaPortStatus[0],
								0,
								g_globalSettingParam.VSA_TRIGGER_LEVEL_DB,
								g_globalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000	
							  );

		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Fail to setup VSA, LP_SetVsa() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsa() return OK.\n");
		}

		//Turn Off VSG in case signal out after initial testers
		//Tracy Yu; Move to the front of this function, to enable loop back test ; 2012-04-06
		/*err = ::LP_EnableVsgRFNxN(vsgRFEnabled[0], vsgRFEnabled[1], vsgRFEnabled[2], vsgRFEnabled[3]);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Fail to setup VSA, LP_EnableVsgRFNxN() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_EnableVsgRFNxN() return OK.\n");
		}*/


		//Default order: prefOrderSignal={1,2,3,4}
        prefOrderSignal.clear();
		for(int i=0;i<MAX_TESTER_NUM;i++)
		{
			prefOrderSignal.push_back(i+1);
		}

#pragma region Set VSA analysis order
        if(!strcmp(l_txVerifyEvmParam.TX_ANALYSIS_ORDER,"-1,-1,-1,-1"))
        {
		    //Complete prefOrderSignals, to avoid valid signal is not what we want. In useAllSignals = 1.
             prefOrderSignal.clear();
             int vsaOrderRef[MAX_TESTER_NUM];

             for(int i=0;i<MAX_TESTER_NUM;i++)
             {
                 
				 if(txEnabled[vsaMappingTx[i]-1])
				 {
					 prefOrderSignal.push_back(i+1);
					 vsaOrderRef[i] = 1;
				 }
				 else
				 {
					 vsaOrderRef[i] = 0;
				 }
             }

             //Step2: complete prefOrderSignal[]
             for(int i=0;i<MAX_TESTER_NUM;i++)
             {
                 if(prefOrderSignal.size() < MAX_TESTER_NUM && 0==vsaOrderRef[i])
                 {
                     prefOrderSignal.push_back(i+1);
                 }
             }
			
			specifiedAnalysisOrder = 0;

        }
        else  
        {
             splits.clear();
             prefOrderSignal.clear();
             SplitString(l_txVerifyEvmParam.TX_ANALYSIS_ORDER, splits, ",");
             
             //Step 1: put non -1 into prefOrderSignal[]                     
             splitsIter = splits.begin();
             int vsaOrderRef[MAX_TESTER_NUM];
             for(int i=0;i<MAX_TESTER_NUM;i++)
             {
                 vsaOrderRef[i] = 0;
             }
             while(splitsIter != splits.end())
             {
                 if((atoi(splitsIter->c_str()) > 0)  && 
                    (atoi(splitsIter->c_str()) < (MAX_TESTER_NUM+1)))
                 {
                     prefOrderSignal.push_back(atoi(splitsIter->c_str()));
                     vsaOrderRef[atoi(splitsIter->c_str())-1] =1;
                 }
                 else
                 {

                 }
                 splitsIter++;

             }

             //Step2: complete prefOrderSignal[]
             for(int i=0;i<MAX_TESTER_NUM;i++)
             {
                 if(prefOrderSignal.size() < MAX_TESTER_NUM && 0==vsaOrderRef[i])
                 {
                     prefOrderSignal.push_back(i+1);
                 }
             }
			 //record specified analysis order for valid VSA Num
			 specifiedAnalysisOrder = 1;
        }

#pragma endregion

		/*------------------*
		 * Start While Loop *
		 *------------------*/
		avgIteration = 0;
		while ( avgIteration<g_globalSettingParam.EVM_AVERAGE )
		{
			analysisOK = false;
			captureOK  = false;
			char sTestDetail[MAX_BUFFER_SIZE] = {'\0'};
			sprintf_s(sTestDetail, MAX_BUFFER_SIZE, "WiFi_TX_Verify_Evm_%d_%s", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE);


			/*-------------------------------------------------------*/
			/*	before: step1: capture signal
			/*			step2: analysis each mode such like 802.11ag, 802.11b, 802.11n
			/*			step3: retrieve result and store
			/*			step4: go back to step1 until reached average times
			/*-------------------------------------------------------*/
			/*-------------------------------------------------------*/
			/*	After:      step1: capture signal
			/*			for 11a/b/g analysis 
			/*				step2: need to specify VSA 
			/*				step3: retrieve result and store
			/*				step4: back to step2, specify the next VSA, when validVSANum reached, go to step5
			/*				step5: go back to step1 until reached average times
			/*
			/*          for 11n analysis
			/*			    step2: specify "useAllSignals" and "prefOrderSignal"
			/*				step3: retrieve result and store
			/*				step4: go back to step1 until reached average times
			/*-------------------------------------------------------*/

		   /*----------------------------*
			* Perform Normal VSA capture *
			*----------------------------*/

			/*------------------------------------------------------------*/
			/*For EVM Analysis, in HT20/HT40, using normal capture     ---*/
			/*------------------------------------------------------------*/
			HT40ModeOn = 0;

			err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_globalSettingParam.VSA_TRIGGER_TYPE, 80e6, HT40ModeOn );     
			if ( ERR_OK!=err )
			{
				double rxAmpl;
				LP_Agc(&rxAmpl, TRUE);	// do auto range on all testers
				err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_globalSettingParam.VSA_TRIGGER_TYPE, 80e6, HT40ModeOn );     
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to capture signal.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_VsaDataCapture() return OK.\n");
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_VsaDataCapture() return OK.\n");
			}


			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;
			if (1==g_globalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_SaveAlways", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}

#pragma region Analysis_802_11a/b/g And Retrieve Result
			if(wifiMode == WIFI_11B || wifiMode == WIFI_11AG)
			{
					
				l_txVerifyEvmReturn.SPATIAL_STREAM = 1;
				if(specifiedAnalysisOrder)
				{
					validVsaNum = l_txVerifyEvmReturn.SPATIAL_STREAM;
				}
				else
				{
					//avoid stream value capture on unvalid vsa when use analysis order
				}
				avgIteration++;

				/*----------------------------------------------*/
				/* For each VSA, do analysis and retrieve result*/
			    /*----------------------------------------------*/
				for(int i=0;i< validVsaNum;i++)
				{
					if(wifiMode == WIFI_11B)
					{

						err = ::LP_SetAnalysisParameterInteger("Analyze11b", "vsaNum", prefOrderSignal[i]);
						err = ::LP_Analyze80211b(   g_globalSettingParam.ANALYSIS_11B_EQ_TAPS,
													g_globalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG,
													g_globalSettingParam.ANALYSIS_11B_METHOD_11B
													);
						if (ERR_OK!=err)
						{	// Fail Analysis, thus save capture (Signal File) for debug
							sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211b_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
							WiFiSaveSigFile(sigFileNameBuffer);

							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211b() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211b() return OK.\n");
						}	
					}
					else // WIFI_11AG
					{
						err = ::LP_SetAnalysisParameterInteger("AnalyzeOFDM", "vsaNum", prefOrderSignal[i]);

						if (0==strcmp(l_txVerifyEvmParam.BANDWIDTH,"HALF") || 0==strcmp(l_txVerifyEvmParam.BANDWIDTH,"QUAR"))
						{
							printf("h\n");
							err = ::LP_Analyze80211p(  g_globalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
								g_globalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
								g_globalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
								g_globalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
								g_globalSettingParam.ANALYSIS_11AG_AMPL_TRACK,
								(0==strcmp(l_txVerifyEvmParam.BANDWIDTH,"HALF"))?IQV_OFDM_ASTM_DSRC:IQV_OFDM_QUARTER_RATE
								);
							if ( ERR_OK!=err )
							{
								// Fail Analysis, thus save capture (Signal File) for debug
								sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211p_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
								WiFiSaveSigFile(sigFileNameBuffer);
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211p() return error.\n");					
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211p() return OK.\n");
							}
						}
						else
						{
							printf("g\n");
							err = ::LP_Analyze80211ag(  g_globalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
														g_globalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
														g_globalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
														g_globalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
														g_globalSettingParam.ANALYSIS_11AG_AMPL_TRACK
														);
							if ( ERR_OK!=err )
							{
								// Fail Analysis, thus save capture (Signal File) for debug
								sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211ag_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
								WiFiSaveSigFile(sigFileNameBuffer);
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211ag() return error.\n");					
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211ag() return OK.\n");
							}					
						}
					}

					
					analysisOK = true;
					/*--------------------------------------------*/
					/* Retrieve analysis Results for 11b and 11ag */
 				    /*--------------------------------------------*/

					//For EVM, In 11a/b/g, only have one stream
					//There existed a problem:
					//  - For result keyword: 
					//							AMP_ERR_DB_S1            
					//							EVM_DB_AVG_S1
					//							EVM_DB_MAX_S1
					//							EVM_DB_MIN_S1
					//							FREQ_ERROR_PPM_AVG
					//							FREQ_ERROR_PPM_MAX
					//							FREQ_ERROR_PPM_MIN
					//							PHASE_ERR_DEG_S1  
					//
					//  - While specify VSA, those keywords will report a group result for each VSA.
					//  - There existed a little difference:
					//		1.	EVM has +/- 2dB  difference in each VSA. 
					//		2.	Amplitude Error has +/-0.1 dB
					//		3.	FREQ ERROR has +/-100kHz
					//		4.	Phase Error has +/- 1degree
					//
					//  - For existed keyword, it will keep only 1 VSA result. 
					//
					//  TODO: re-define keyword or do mathematical operation

					double dataRate;

					if (wifiMode==WIFI_11B)
						dataRate = ::LP_GetScalarMeasurement("bitRateInMHz", 0);
					else
						dataRate = ::LP_GetScalarMeasurement("dataRate", 0); 

					if (dataRate>0)
					{
						// save valid data rate
						l_txVerifyEvmReturn.DATA_RATE = dataRate;

						// Peak EVM, only for 11b, signal stream
						// IQlite merge; Tracy Yu ; 2012-03-31
						if (wifiMode==WIFI_11B)
						{
							evmPk[i][avgIteration-1] = ::LP_GetScalarMeasurement("evmPk", 0);
							if ( -99.00 >= evmPk[i][avgIteration-1] )
							{
								analysisOK = false;
								evmPk[i][avgIteration-1] = NA_NUMBER;
								WiFiSaveSigFile(sTestDetail);
								err = -1;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(evmPk) return error.\n");
								throw logMessage;
							}
						}
						else
							evmAvgAll[i][avgIteration-1] = ::LP_GetScalarMeasurement("evmAll",0); 

						// Frequency Error
						freqErrorHz[i][avgIteration-1] = ::LP_GetScalarMeasurement("freqErr", 0); 

						// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
						ampErrDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("ampErrDb", 0); 

						// IQ Match Phase Error, per stream		
						phaseErr[i][avgIteration-1] = ::LP_GetScalarMeasurement("phaseErr", 0);

						// RMS Phase Noise, mapping the value "RMS Phase Noise" in IQsignal
						phaseNoiseRms[i][avgIteration-1] = ::LP_GetScalarMeasurement("rmsPhaseNoise",0);
						//phaseErr[0][avgIteration-1] = ::LP_GetScalarMeasurement("phaseErr", 0); // "phaseErr" is over all stream

						symbolClockErr[i][avgIteration-1] = ::LP_GetScalarMeasurement("clockErr", 0); 

						// Power, per VSA 
						double measuredPower = ::LP_GetScalarMeasurement("rmsPowerNoGap",0);
						if (measuredPower!=NA_NUMBER)
							measuredPower += l_txVerifyEvmParam.CABLE_LOSS_DB[prefOrderSignal[i]-1];

						vsaRmsInEachBurst[prefOrderSignal[i]-1][avgIteration-1][0] = measuredPower;
						streamRmsInEachBurst[i][avgIteration-1][prefOrderSignal[i]-1] = measuredPower;
						streamInVsa[i][prefOrderSignal[i]-1][avgIteration-1] = measuredPower;
					}
				}
				/*---------------------------------------------------*/
				/* END (do analysis and retrieve result) For each VSA*/
			    /*---------------------------------------------------*/

			}
#pragma endregion
#pragma region Analysis_802_11n And Retrieve Result
			else if ( (wifiMode==WIFI_11N_HT20)||(wifiMode==WIFI_11N_HT40) )                         
			{   // [Case 03]: MIMO Analysis
				char referenceFileName[MAX_BUFFER_SIZE], analyzeMimoType[MAX_BUFFER_SIZE], analyzeMimoMode[MAX_BUFFER_SIZE];

				switch( g_Tester_Type )
				{
					// WiFi_MiMo is only used for IQnxn

					/*case IQ_View:
						sprintf_s(analyzeMimoType, "EWC");                        
						if ( wifiStreamNum==WIFI_ONE_STREAM )  // MCS0 ~ MCS7
						{        
							sprintf_s(analyzeMimoMode, "nxn");
							sprintf_s(referenceFileName, ""); 
						}
						else // MCS8 ~ MCS15 or more, need MIMO reference File *.ref
						{
							sprintf_s(analyzeMimoMode, "composite");
							err = GetWaveformFileName("EVM", 
													  "REFERENCE_FILE_NAME", 
													  wifiMode, 
													  l_txVerifyEvmParam.BANDWIDTH, 
													  l_txVerifyEvmParam.DATA_RATE, 
													  l_txVerifyEvmParam.PREAMBLE, 
													  l_txVerifyEvmParam.PACKET_FORMAT_11N, 
													  referenceFileName, 
													  MAX_BUFFER_SIZE);							
							if ( ERR_OK!=err )
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get reference file name.\n");
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] GetWaveformFileName(REFERENCE_FILE_NAME) return OK.\n");
							}
						}
						break;*/

					case IQ_nxn:
						sprintf_s(analyzeMimoType, "EWC");
						sprintf_s(analyzeMimoMode, "nxn");
						sprintf_s(referenceFileName, "");
						break; 

					default:
						sprintf_s(analyzeMimoType, "EWC");
						sprintf_s(analyzeMimoMode, "nxn");
						sprintf_s(referenceFileName, "");
						break;
				}
				if(!strcmp(l_txVerifyEvmParam.TX_ANALYSIS_ORDER,"-1,-1,-1,-1"))
				{
					err = ::LP_SetAnalysisParameterInteger("Analyze80211n", "useAllSignals", 1);
         			if ( ERR_OK!=err )
					   {
						   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SetAnalysisParameterInteger() return error.\n");
						   throw logMessage;
					   }
					   else
					   {
						   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetAnalysisParameterInteger() return OK.\n");
					   }
					                     
					err = ::LP_SetAnalysisParameterIntegerArray("Analyze80211n","prefOrderSignals", &prefOrderSignal[0], (int)prefOrderSignal.size());
       				if ( ERR_OK!=err )
					{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SetAnalysisParameterIntegerArray() return error.\n");
							throw logMessage;
					}
					else
					{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetAnalysisParameterIntegerArray() return OK.\n");
					}
				}
				else
				{
					 err = ::LP_SetAnalysisParameterInteger("Analyze80211n", "useAllSignals", 0);
         			 if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SetAnalysisParameterInteger() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetAnalysisParameterInteger() return OK.\n");
						}
					 err = ::LP_SetAnalysisParameterIntegerArray("Analyze80211n","prefOrderSignals", &prefOrderSignal[0], (int)prefOrderSignal.size());
           			 if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SetAnalysisParameterIntegerArray() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetAnalysisParameterIntegerArray() return OK.\n");
						}

				}

				//Set Frequency Correction for 802.11n analysis
				err = ::LP_SetAnalysisParameterInteger("Analyze80211n", "frequencyCorr", g_globalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION);
     			if ( ERR_OK!=err )
			    {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SetAnalysisParameterInteger() return error.\n");
				   throw logMessage;
			    }
				else
				{
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetAnalysisParameterInteger() return OK.\n");
				}

				err = ::LP_Analyze80211n( analyzeMimoType,
										analyzeMimoMode,
										g_globalSettingParam.ANALYSIS_11N_PHASE_CORR,    
										g_globalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR,    
										g_globalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING,  
										g_globalSettingParam.ANALYSIS_11N_DECODE_PSDU, 
										g_globalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST,
										referenceFileName,
									    packetFormat);   
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_AnalyzeMIMO_Failed", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211n() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211n() return OK.\n");
				}

				avgIteration++;
				analysisOK = true;
				
				/*--------------------------------------------*/
				/* Retrieve analysis Results for 11n          */
			    /*--------------------------------------------*/
			
				
				// Number of spatial streams
				l_txVerifyEvmReturn.SPATIAL_STREAM = (int) ::LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);
				if (l_txVerifyEvmReturn.SPATIAL_STREAM == (int)NA_NUMBER)
				{
					err = -1;
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Invalid_Analysis_Results", l_txVerifyEvmParam.FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_GetScalarMeasurement() return error.\n");
					throw logMessage;
				}

				if(specifiedAnalysisOrder)
				{
					validVsaNum = l_txVerifyEvmReturn.SPATIAL_STREAM;
				}
				else
				{
					//avoid stream value capture on unvalid vsa when use analysis order
				}

                //no need to run IQ_view for MIMO
				//if (0) //g_Tester_Type==IQ_View)
				//{
				//	for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++)
				//	{
				//		// Get EVM test result
				//		evmAvgAll[i][avgIteration-1] = ::LP_GetScalarMeasurement("evmAvgAll", 0); // only can get EVM_ALL, means index always = 0

				//		//if ( -99.00 >= evmAvgAll[i][avgIteration-1] )
				//		//{
				//		//	analysisOK = false;
				//		//	evmAvgAll[i][avgIteration-1] = NA_NUMBER;
				//		//	WiFiSaveSigFile(sTestDetail);
				//		//	err = -1;
				//		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(evmAvgAll) return error.\n");
				//		//	throw logMessage;
				//		//}
				//		// Get Power test result
				//		double measuredPower = ::LP_GetScalarMeasurement("rxRmsPowerDb", i);
				//		if (measuredPower!=NA_NUMBER)
				//			measuredPower += l_txVerifyEvmParam.CABLE_LOSS_DB[prefOrderSignal[i]-1]

				//		rxRmsPowerDb[i][avgIteration-1] = measuredPower;

				//		printf("Power %6.2f stream %u iteration %u\n", measuredPower, i, avgIteration);

				//		//if ( -99.00 >= rxRmsPowerDb[i][avgIteration-1] )
				//		//{
				//		//	analysisOK = false;
				//		//	rxRmsPowerDb[i][avgIteration-1] = NA_NUMBER;
				//		//	WiFiSaveSigFile(sTestDetail);
				//		//	err = -1;
				//		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(rxRmsPowerDb) return error.\n");
				//		//	throw logMessage;
				//		//}
				//		//else
				//		{
				//			//The current code will work correctly with 2x2 MIMO.  
				//			//For more than 2x2 MIMO, need to implement more complicated algorithm

				//			l_txVerifyEvmParam.CABLE_LOSS_DB[prefOrderSignal[i]-1]

				//			if (l_txVerifyEvmReturn.SPATIAL_STREAM == 1)
				//			{
				//				// One stream data rate, should be only one antenna is ON	
				//				if      (l_txVerifyEvmParam.TX1==1)	antSelection = 0;
				//				else if (l_txVerifyEvmParam.TX2==1)	antSelection = 1;
				//				else if (l_txVerifyEvmParam.TX3==1)	antSelection = 2;
				//				else if (l_txVerifyEvmParam.TX4==1)	antSelection = 3;
				//				else    							antSelection = 0;

				//				rxRmsPowerDb[i][avgIteration-1] = rxRmsPowerDb[i][avgIteration-1] + l_txVerifyEvmParam.CABLE_LOSS_DB[antSelection];
				//			}
				//			else if (1 == l_txVerifyEvmParam.TX1 && 1 == l_txVerifyEvmParam.TX2)
				//			{
				//				rxRmsPowerDb[i][avgIteration-1] = rxRmsPowerDb[i][avgIteration-1] + l_txVerifyEvmParam.CABLE_LOSS_DB[i];
				//			}
				//			else
				//			{
				//				rxRmsPowerDb[i][avgIteration-1] = rxRmsPowerDb[i][avgIteration-1] + cableLossDb;
				//			}
				//		}  

				//		// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
				//		ampErrDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_amplDb", i); // Index always = 0
				//		phaseErr[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_phaseDeg", i);

				//	}
				//}
				//else	// g_Tester_Type == IQnxn
				{
//					for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++)  
					for(int i=0;i<g_Tester_Number;i++)  //not a good way to implement, what if the return value is changed, we should use spatial_stream. 
					{
						double evm;

						evm = ::LP_GetScalarMeasurement("evmAvgAll",i); 
						if (evm!=INVALID_EVM) 
						{
							// EVM 
							evmAvgAll[i][avgIteration-1] = evm;

							//ISOLATION: NStreams x NStreams
							for(int j=0;j<l_txVerifyEvmReturn.SPATIAL_STREAM;j++)
							{
								if(i != j)
								{
									isolationInTwoStreams[i][j][avgIteration-1] = 0.0 - ::LP_GetScalarMeasurement("isolationDb", (i*validVsaNum)+j); 
								}
								else
								{
									continue;
								}
							}

							//Matrix: Nstream x NVsa, Stream in each VSA, order by prefOrderSignal
							for(int j=0;j<validVsaNum;j++)
							{
								measuredPower = ::LP_GetScalarMeasurement("rxRmsPowerDb", (i*validVsaNum)+j);
								if (measuredPower!=NA_NUMBER)
									measuredPower += l_txVerifyEvmParam.CABLE_LOSS_DB[prefOrderSignal[j]-1];
								streamRmsInEachBurst[i][avgIteration-1][prefOrderSignal[j]-1]= measuredPower;
							}

							//Matrix: Nstream x NVsa, VSA in each stream
							for(int j=0;j<validVsaNum;j++)
							{
								measuredPower = ::LP_GetScalarMeasurement("rxRmsPowerDb", i*validVsaNum+j);
								if (measuredPower!=NA_NUMBER)
									measuredPower += l_txVerifyEvmParam.CABLE_LOSS_DB[prefOrderSignal[j]-1];
								vsaRmsInEachBurst[prefOrderSignal[j]-1][avgIteration-1][i] = measuredPower;
							}

							//STREAM_POWER_IN_VSA, streamInVsa
							for(int j=0;j<g_Tester_Number ;j++)
							{
								measuredPower = ::LP_GetScalarMeasurement("rxRmsPowerDb", i*validVsaNum+j);
								if (measuredPower!=NA_NUMBER)
									measuredPower += l_txVerifyEvmParam.CABLE_LOSS_DB[prefOrderSignal[j]-1];
								streamInVsa[i][prefOrderSignal[j]-1][avgIteration-1] = measuredPower;
							}


							// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
							ampErrDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_amplDb", i);
							phaseErr[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_phaseDeg", i);

							// Frequency Error
							freqErrorHz[i][avgIteration-1] = ::LP_GetScalarMeasurement("freqErrorHz",0);

							// Datarate
							l_txVerifyEvmReturn.DATA_RATE = ::LP_GetScalarMeasurement("rateInfo_dataRateMbps", 0); 

							// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
							phaseNoiseRms[i][avgIteration-1] = ::LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll", 0);

							symbolClockErr[i][avgIteration-1] = ::LP_GetScalarMeasurement("symClockErrorPpm", 0);
						}
					}
				}
			}
#pragma endregion
		}   // End - avgIteration

	 //  /*-----------*
		//*  Tx Stop  *
		//*-----------*/
		//err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");		
		//if ( ERR_OK!=err )
		//{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
		//	err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
		//	if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
		//	{
		//		err = -1;	// set err to -1, means "Error".
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
		//		throw logMessage;
		//	}
		//	else	// Just return normal error message in this case
		//	{
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
		//		throw logMessage;
		//	}
		//}
		//else
		//{
		//	g_vDutTxActived = false;
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
		//}

#pragma region Averaging and Saving Test Result
	   /*----------------------------------*
		* Averaging and Saving Test Result *
		*----------------------------------*/
		if ( (ERR_OK==err) && captureOK && analysisOK )
		{
			double dummyMax, dummyMin;
			
		    double unvalidAvg, unvalidMax, unvalidMin;

//			for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++)
			for(int i=0;i<g_Tester_Number;i++)
			{
				//if (l_txVerifyEvmReturn.SPATIAL_STREAM==1)
				//	i = usedSingleStreamVsa;

				// Average EVM test result
				if (wifiMode==WIFI_11B)
					::AverageTestResult(&evmPk[i][0], avgIteration, LOG_20, l_txVerifyEvmReturn.EVM_AVG[i], l_txVerifyEvmReturn.EVM_MAX[i], l_txVerifyEvmReturn.EVM_MIN[i]);
				else
					::AverageTestResult(&evmAvgAll[i][0], avgIteration, LOG_20, l_txVerifyEvmReturn.EVM_AVG[i], l_txVerifyEvmReturn.EVM_MAX[i], l_txVerifyEvmReturn.EVM_MIN[i]);

				// Average Power test result
				::AverageTestResult(&rxRmsPowerDb[i][0], avgIteration, LOG_10, l_txVerifyEvmReturn.POWER_AVG[i], l_txVerifyEvmReturn.POWER_MAX[i], l_txVerifyEvmReturn.POWER_MIN[i]);

				// Average Amp Error test result
				::AverageTestResult(&ampErrDb[i][0], avgIteration, LOG_10, l_txVerifyEvmReturn.AMP_ERR_DB[i], dummyMax, dummyMin);

				// Average Phase Error
				::AverageTestResult(&phaseErr[i][0], avgIteration, Linear, l_txVerifyEvmReturn.PHASE_ERR[i], dummyMax, dummyMin);
			}

			for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++)
			{

				// for 11b data rate, other streams power are the leakage power on other VSA
				if (wifiMode==WIFI_11B)
				{
					for(int j=0;j<MAX_TESTER_NUM;j++)
						::AverageTestResult(&streamInVsa[j][j][0], avgIteration, LOG_10, l_txVerifyEvmReturn.STREAM_POWER_IN_VSA[0][j], unvalidMax, unvalidMin); 
				}
				else
				{
					for(int j=0;j<MAX_TESTER_NUM;j++)
						::AverageTestResult(&streamInVsa[i][j][0], avgIteration, LOG_10, l_txVerifyEvmReturn.STREAM_POWER_IN_VSA[i][j], unvalidMax, unvalidMin);
				}

			}

            //::AverageTestResult(l_txVerifyEvmReturn.EVM_AVG, l_txVerifyEvmReturn.SPATIAL_STREAM, LOG_20_EVM, l_txVerifyEvmReturn.EVM_AVG_DB, dummyMax, dummyMin);
            //::AverageTestResult(l_txVerifyEvmReturn.EVM_MAX, l_txVerifyEvmReturn.SPATIAL_STREAM, LOG_20_EVM, unvalidAvg, l_txVerifyEvmReturn.EVM_MAX_DB, unvalidMin);
            //::AverageTestResult(l_txVerifyEvmReturn.EVM_MIN, l_txVerifyEvmReturn.SPATIAL_STREAM, LOG_20_EVM, unvalidAvg, unvalidMax, l_txVerifyEvmReturn.EVM_MIN_DB);
            ////Zhiyong debug here
            //printf("Old Method Evm_all: Avg: %0.2f dB, Max: %0.2f dB, Min: %0.2f dB \n",  l_txVerifyEvmReturn.EVM_AVG_DB,l_txVerifyEvmReturn.EVM_MAX_DB,l_txVerifyEvmReturn.EVM_MIN_DB);

//          Added new method to correctly calculate EVM_ALL based on spec, RMS average over streams in one packet first and then linear average over packets, Zhiyong on Feb 1, 2011.
            double evmStreamPacket[MAX_TESTER_NUM];

            for(int packetInteration=0; packetInteration<g_globalSettingParam.EVM_AVERAGE; packetInteration++)
            {
                //for(int i=0; i<g_Tester_Number; i++)
                for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++)
                {
                    evmStreamPacket[i]=evmAvgAll[i][packetInteration];
                }
                ::AverageTestResult(evmStreamPacket, l_txVerifyEvmReturn.SPATIAL_STREAM, LOG_20_EVM, evmAllPacket[packetInteration], dummyMax, dummyMin);
            }
            ::AverageTestResult(&evmAllPacket[0], avgIteration, LOG_20, l_txVerifyEvmReturn.EVM_AVG_DB, l_txVerifyEvmReturn.EVM_MAX_DB, l_txVerifyEvmReturn.EVM_MIN_DB);

            ////Zhiyong debug here
            //printf("New Method Evm_all: Avg: %0.2f dB, Max: %0.2f dB, Min: %0.2f dB \n",  l_txVerifyEvmReturn.EVM_AVG_DB,l_txVerifyEvmReturn.EVM_MAX_DB,l_txVerifyEvmReturn.EVM_MIN_DB);

			::AverageTestResult(l_txVerifyEvmReturn.POWER_AVG, l_txVerifyEvmReturn.SPATIAL_STREAM, LOG_10, l_txVerifyEvmReturn.POWER_AVG_DBM, dummyMax, dummyMin);
			::AverageTestResult(l_txVerifyEvmReturn.AMP_ERR_DB, l_txVerifyEvmReturn.SPATIAL_STREAM, LOG_10, l_txVerifyEvmReturn.AMP_ERR_DB_ALL, dummyMax, dummyMin);
			::AverageTestResult(l_txVerifyEvmReturn.PHASE_ERR, l_txVerifyEvmReturn.SPATIAL_STREAM, Linear, l_txVerifyEvmReturn.PHASE_ERR_ALL, dummyMax, dummyMin);

			for(int i=0;i< validVsaNum ;i++)
			{
				//for(int j=0;j<l_txVerifyEvmReturn.SPATIAL_STREAM;j++)
				//{

					//if (wifiMode==WIFI_11B)
					//{
					//	// (11b only) Average EVM_PK test result
					//	//::AverageTestResult(&evmPk[i][j], avgIteration, LOG_20, dummyMax, l_txVerifyEvmReturn.EVM_PK_DB, dummyMin);
					//	::AverageTestResult(&evmPk[i][0], avgIteration, LOG_20, dummyMax, l_txVerifyEvmReturn.EVM_MAX[i], dummyMin);

					//}

					// use EVM value to check for valid packet results
					if (l_txVerifyEvmReturn.EVM_AVG[i]!=0)
					{
						// Average RMS Phase Noise test result
						::AverageTestResult(&phaseNoiseRms[i][0], avgIteration, Linear, l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL, dummyMax, dummyMin);

						// Average Freq Error test result
						::AverageTestResult(&freqErrorHz[i][0], avgIteration, Linear, l_txVerifyEvmReturn.FREQ_ERROR_AVG[i], l_txVerifyEvmReturn.FREQ_ERROR_MAX[i], l_txVerifyEvmReturn.FREQ_ERROR_MIN[i]);

						// Transfer the Freq Error result unit to ppm
						l_txVerifyEvmReturn.FREQ_ERROR_AVG[i] = l_txVerifyEvmReturn.FREQ_ERROR_AVG[i]/l_txVerifyEvmParam.FREQ_MHZ;
						l_txVerifyEvmReturn.FREQ_ERROR_MAX[i] = l_txVerifyEvmReturn.FREQ_ERROR_MAX[i]/l_txVerifyEvmParam.FREQ_MHZ;
						l_txVerifyEvmReturn.FREQ_ERROR_MIN[i] = l_txVerifyEvmReturn.FREQ_ERROR_MIN[i]/l_txVerifyEvmParam.FREQ_MHZ;

						// Average Symbol clock Error test result
						::AverageTestResult(&symbolClockErr[i][0], avgIteration, Linear, l_txVerifyEvmReturn.SYMBOL_CLK_ERR[i], dummyMax, dummyMin);
					}
				//}
			}


			//FOR MULTI-TESTER
            vector<double> tmpVector;
			int tempCount;
//			for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++) 
			for(int i=0;i<validVsaNum;i++) 
			{
				for(int j=0;j<avgIteration;j++) 
				{
                   //Avoid unvalid Vsa
                    tmpVector.clear();
					tempCount = 0;
                    for(int k=0;k<validVsaNum;k++)
                    {
						//// if single streams, all VSA power together since the signal maybe on other VSA, not always on VSA1
						//if (l_txVerifyEvmReturn.SPATIAL_STREAM==1)
						//	tmpVector.push_back(streamRmsInEachBurst[usedSingleStreamVsa][j][prefOrderSignal[k]-1]);
						//else
							tmpVector.push_back(streamRmsInEachBurst[i][j][prefOrderSignal[k]-1]);
                    }
                    ::AverageTestResult(&tmpVector[0], validVsaNum, Linear_Sum, streamRmsSumInEachBurst[i][j], unvalidMax, unvalidMin);
				}

                for(int j=0;j<l_txVerifyEvmReturn.SPATIAL_STREAM;j++)
                {
                    ::AverageTestResult(&isolationInTwoStreams[i][j][0], avgIteration, LOG_10, l_txVerifyEvmReturn.isolationAvgInTwoStreams[i][j], unvalidMax, unvalidMin);
                }
            }

            for(int i=0;i<g_Tester_Number;i++)
            {
                for(int j=0;j<avgIteration;j++) 
			    {
                    ::AverageTestResult(&vsaRmsInEachBurst[prefOrderSignal[i]-1][j][0], l_txVerifyEvmReturn.SPATIAL_STREAM, Linear_Sum, vsaRmsSumInEachBurst[prefOrderSignal[i]-1][j], unvalidMax, unvalidMin);
                }
            }

			//FOR EACH STREAM
//			for(int i=0;i<l_txVerifyEvmReturn.SPATIAL_STREAM;i++) 
			for(int i=0;i<validVsaNum;i++) 
			{
                ::AverageTestResult(&streamRmsSumInEachBurst[i][0], avgIteration, LOG_10, l_txVerifyEvmReturn.streamRmsAvg[i], l_txVerifyEvmReturn.streamRmsMax[i], l_txVerifyEvmReturn.streamRmsMin[i]);
			}

			//FOR EACH VALID VSA,
			for(int i=0;i<g_Tester_Number;i++)
			{
                ::AverageTestResult(&vsaRmsSumInEachBurst[i][0], avgIteration, LOG_10, l_txVerifyEvmReturn.vsaRmsAvg[i], l_txVerifyEvmReturn.vsaRmsMax[i], l_txVerifyEvmReturn.vsaRmsMin[i]);
			}

			//POWER_RMS_AVG_DBM_STREAM_TOTAL
			//POWER_RMS_MIN_DBM_STREAM_TOTAL
			//POWER_RMS_MAX_DBM_STREAM_TOTAL
			::AverageTestResult(&l_txVerifyEvmReturn.streamRmsAvg[0], l_txVerifyEvmReturn.SPATIAL_STREAM, Linear_Sum, l_txVerifyEvmReturn.streamAvgAll, unvalidMax, unvalidMin);
			::AverageTestResult(&l_txVerifyEvmReturn.streamRmsMax[0], l_txVerifyEvmReturn.SPATIAL_STREAM, LOG_10, unvalidAvg, l_txVerifyEvmReturn.streamMaxAll, unvalidMin);
			::AverageTestResult(&l_txVerifyEvmReturn.streamRmsMin[0], l_txVerifyEvmReturn.SPATIAL_STREAM, LOG_10, unvalidAvg, unvalidMax, l_txVerifyEvmReturn.streamMinAll);
			
			//POWER_RMS_AVG_DBM_VSA_TOTAL
			//POWER_RMS_MIN_DBM_VSA_TOTAL
			//POWER_RMS_MAX_DBM_VSA_TOTAL
                                
            //Avoid unvalid Vsa
            tmpVector.clear();
            for(int i=0;i<g_Tester_Number;i++)
            {
                tmpVector.push_back(l_txVerifyEvmReturn.vsaRmsAvg[prefOrderSignal[i]-1]);
            }
            ::AverageTestResult(&tmpVector[0], validVsaNum, Linear_Sum, l_txVerifyEvmReturn.vsaAvgAll, unvalidMax, unvalidMin);
			//::AverageTestResult(&l_txVerifyEvmReturn.vsaRmsAvg[0], validVsaNum, Linear_Sum, l_txVerifyEvmReturn.vsaAvgAll, unvalidMax, unvalidMin);

			//Avoid unvalid Vsa
            tmpVector.clear();
            for(int i=0;i<g_Tester_Number;i++)
            {
                tmpVector.push_back(l_txVerifyEvmReturn.vsaRmsMax[prefOrderSignal[i]-1]);
            }
            ::AverageTestResult(&tmpVector[0], validVsaNum, LOG_10, unvalidAvg, l_txVerifyEvmReturn.vsaMaxAll, unvalidMin);
            //::AverageTestResult(&l_txVerifyEvmReturn.vsaRmsMax[0], validVsaNum, Linear_Sum, l_txVerifyEvmReturn.vsaMaxAll, unvalidMax, unvalidMin);

			//Avoid unvalid Vsa
            tmpVector.clear();
            for(int i=0;i<g_Tester_Number;i++)
            {
                tmpVector.push_back(l_txVerifyEvmReturn.vsaRmsMin[prefOrderSignal[i]-1]);
            }
            ::AverageTestResult(&tmpVector[0], validVsaNum, LOG_10, unvalidAvg, unvalidMax, l_txVerifyEvmReturn.vsaMinAll);
            //::AverageTestResult(&l_txVerifyEvmReturn.vsaRmsMin[0], validVsaNum, Linear_Sum, l_txVerifyEvmReturn.vsaMinAll, unvalidMax, unvalidMin);
			
	      //END FOR 802.11N MULTI-TESTER

		}
		else
		{
			// do nothing
		}

		l_txVerifyEvmReturn.EVM_AVERAGE = g_globalSettingParam.EVM_AVERAGE;
#pragma endregion 

		// IQlite merge; Tracy Yu ; 2012-03-31
		// This is a special case, only if "DUT_KEEP_TRANSMIT=0" then must do TX_STOP manually.
		if ( (g_globalSettingParam.DUT_KEEP_TRANSMIT==0)&&(g_vDutTxActived==true) )
		{
		   /*-----------*
			*  Tx Stop  *
			*-----------*/
			err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			sprintf_s(l_txVerifyEvmReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyEvmReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txVerifyEvmReturn.ERROR_MESSAGE, msg);

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_MiMo] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
    }
    catch(...)
    {
		ReturnErrorMessage(l_txVerifyEvmReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_MiMo] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
    }

	// This is a special case, only when some error occur before the TX_STOP. 
	// This case will take care by the error handling, but must do TX_STOP manually.
	//if ( g_vDutTxActived )
	//{
	//	vDUT_Run(g_WiFi_Dut, "TX_STOP");
	//}
	//else
	//{
	//	// do nothing
	//}

	// Free memory
	prefOrderSignal;  // IQlite merge; Tracy Yu ; 2012-03-31

	evmAvgAll.clear();
	evmPk.clear();
	rxRmsPowerDb.clear();
	freqErrorHz.clear();
	ampErrDb.clear();  // IQlite merge; Tracy Yu ; 2012-03-31
	phaseErr.clear();  // IQlite merge; Tracy Yu ; 2012-03-31
	phaseNoiseRms.clear();  // IQlite merge; Tracy Yu ; 2012-03-31

	splits.clear();  // IQlite merge; Tracy Yu ; 2012-03-31

    streamRmsInEachBurst.clear();
    streamRmsSumInEachBurst.clear();

    vsaRmsInEachBurst.clear();
    vsaRmsSumInEachBurst.clear();

    isolationInTwoStreams.clear();

	vsgRFEnabled.clear();
	rfAmplDb.clear();


    return err;
}

int InitializeTXVerifyEvmContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txVerifyEvmParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    strcpy_s(l_txVerifyEvmParam.BANDWIDTH, MAX_BUFFER_SIZE, "HT20");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.BANDWIDTH))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEvmParam.BANDWIDTH;
        setting.unit        = "";
        setting.helpText    = "Channel bandwidth\r\nValid options: HT20 or HT40 or HALF or QUAR";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txVerifyEvmParam.DATA_RATE, MAX_BUFFER_SIZE, "OFDM-54");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.DATA_RATE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEvmParam.DATA_RATE;
        setting.unit        = "";
        setting.helpText    = "Data rate names:\r\nDSSS-1,DSSS-2,CCK-5_5,CCK-11\r\nOFDM-6,OFDM-9,OFDM-12,OFDM-18,OFDM-24,OFDM-36,OFDM-48,OFDM-54\r\nMCS0, MCS15, etc.";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txVerifyEvmParam.PREAMBLE, MAX_BUFFER_SIZE, "SHORT");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.PREAMBLE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEvmParam.PREAMBLE;
        setting.unit        = "";
        setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=SHORT.";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txVerifyEvmParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.PACKET_FORMAT_11N))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEvmParam.PACKET_FORMAT_11N;
        setting.unit        = "";
        setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	// IQlite merge; Tracy Yu ; 2012-03-31
	strcpy_s(l_txVerifyEvmParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.GUARD_INTERVAL_11N))    // Type_Checking
    {
		setting.value       = (void*)l_txVerifyEvmParam.GUARD_INTERVAL_11N;
        setting.unit        = "";
        setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    
    strcpy_s(l_txVerifyEvmParam.TX_ANALYSIS_ORDER, MAX_BUFFER_SIZE, "-1,-1,-1,-1");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.TX_ANALYSIS_ORDER))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEvmParam.TX_ANALYSIS_ORDER;
        setting.unit        = "";
        setting.helpText    = "The analysis order while verify EVM, speicified VSA analysis order,Default will scan all valid input signals, valid value: 1,2,3,4";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_ANALYSIS_ORDER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.FREQ_MHZ = 2412;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.SAMPLING_TIME_US = 0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEvmParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyEvmParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyEvmParam.TX_POWER_DBM = 15.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEvmParam.TX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port. -99 is for default target power.";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.TX1 = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX1))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.TX1;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.TX2 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX2))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.TX2;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.TX3 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX3))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.TX3;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmParam.TX4 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmParam.TX4))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmParam.TX4;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    //VSG_CONNECTION
    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_txVerifyEvmParam.VSA_CONNECTION[i], MAX_BUFFER_SIZE, "TX%d", i+1);
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.VSA_CONNECTION[i]))    // Type_Checking
        {
            setting.value       = (void*)l_txVerifyEvmParam.VSA_CONNECTION[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the actual connection between DUT antenna ports and VSAs port.\r\nValid value is TX1, TX2, TX3, TX4 and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSA%d_CONNECTION", i+1);
            l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }

    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_txVerifyEvmParam.VSA_PORT[i], MAX_BUFFER_SIZE, "");
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.VSA_PORT[i]))    // Type_Checking
        {
            setting.value       = (void*)l_txVerifyEvmParam.VSA_PORT[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the VSA port, Default is used global setting.\r\nValid value is Left, Right and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSA%d_PORT", i+1);
            l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }

    }

    
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_txVerifyEvmReturnMap.clear();

    setting.type = WIFI_SETTING_TYPE_INTEGER;
    l_txVerifyEvmReturn.EVM_AVERAGE = g_globalSettingParam.PM_AVERAGE;
    if (sizeof(int)==sizeof(l_txVerifyEvmReturn.EVM_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEvmReturn.EVM_AVERAGE;
        setting.unit  = "";
        setting.helpText = "Averaging times used for Power measurement.\r\nDefault value is 3";
        l_txVerifyEvmReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("NUMBER_OF_AVG", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyEvmReturn.EVM_AVG_DB = NA_DOUBLE;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_AVG_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmReturn.EVM_AVG_DB;
        setting.unit        = "dB";
        setting.helpText    = "EVM average over captured packets and all data streams(MIMO)";
        l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_DB_ALL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    //l_txVerifyEvmReturn.EVM_MAX_DB = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_MAX_DB))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_txVerifyEvmReturn.EVM_MAX_DB;
    //    setting.unit        = "dB";
    //    setting.helpText    = "EVM maximum over captured packets and all data streams(MIMO)";
    //    l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_DB_MAX_STREAM_TOTAL", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}
    //l_txVerifyEvmReturn.EVM_MIN_DB = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_MIN_DB))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_txVerifyEvmReturn.EVM_MIN_DB;
    //    setting.unit        = "dB";
    //    setting.helpText    = "EVM minimum over captured packets and all data streams(MIMO)";
    //    l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_DB_MIN_STREAM_TOTAL", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.EVM_AVG[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_AVG[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.EVM_AVG[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "EVM_DB_AVG_S%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Average EVM over captured packets on individual streams(MIMO)";
            l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.EVM_MAX[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_MAX[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.EVM_MAX[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "EVM_DB_MAX_S%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Maximum EVM over captured packets on individual streams(MIMO)";
            l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txVerifyEvmReturn.EVM_MIN[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_MIN[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyEvmReturn.EVM_MIN[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "EVM_DB_MIN_S%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Minimum EVM over captured packets on individual streams(MIMO)";
            l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    //l_txVerifyEvmReturn.EVM_PK_DB = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyEvmReturn.EVM_PK_DB))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_txVerifyEvmReturn.EVM_PK_DB;
    //    setting.unit        = "dB";
    //    setting.helpText    = "(11b only)Peak EVM over captured packets.";
    //    l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_DB_PK", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

	for(int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.AMP_ERR_DB[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.AMP_ERR_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyEvmReturn.AMP_ERR_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "AMP_ERR_DB_S%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "IQ Match Amplitude Error in dB.";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}
	////AMP_ERR_DB_ALL
	//l_txVerifyEvmReturn.AMP_ERR_DB_ALL = NA_DOUBLE;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyEvmReturn.AMP_ERR_DB_ALL))    // Type_Checking
	//{
	//	setting.value       = (void*)&l_txVerifyEvmReturn.AMP_ERR_DB_ALL;
	//	setting.unit        = "dB";
	//	setting.helpText    = "IQ Match Amplitude Error in dB.";
	//	l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("AMP_ERR_DB_STREAM_TOTAL", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}


	for(int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.PHASE_ERR[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.PHASE_ERR[i] ))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyEvmReturn.PHASE_ERR[i] ;
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "PHASE_ERR_DEG_S%d", i+1);
			setting.unit        = "Degree";
			setting.helpText    = "IQ Match Phase Error.";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

	}


	l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL = NA_DOUBLE;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL ))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmReturn.PHASE_NOISE_RMS_ALL ;
		setting.unit        = "Degree";
        setting.helpText    = "Frequency RMS Phase Noise.";
        l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PHASE_NOISE_RMS_ALL", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


    //l_txVerifyEvmReturn.POWER_AVG_DBM = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyEvmReturn.POWER_AVG_DBM))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_txVerifyEvmReturn.POWER_AVG_DBM;
    //    setting.unit        = "dBm";
    //    setting.helpText    = "Average Power over captured packets";
    //    l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVG_DBM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    //for (int i=0;i<MAX_DATA_STREAM;i++)
    //{
    //    l_txVerifyEvmReturn.POWER_AVG[i] = NA_DOUBLE;
    //    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //    if (sizeof(double)==sizeof(l_txVerifyEvmReturn.POWER_AVG[i]))    // Type_Checking
    //    {
    //        setting.value = (void*)&l_txVerifyEvmReturn.POWER_AVG[i];
    //        char tempStr[MAX_BUFFER_SIZE];
    //        sprintf_s(tempStr, "POWER_AVG_%d", i+1);
    //        setting.unit        = "dBm";
    //        setting.helpText    = "Average Power over captured packets on individual streams";
    //        l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
    //    }
    //    else    
    //    {
    //        printf("Parameter Type Error!\n");
    //        exit(1);
    //    }
    //}

    //for (int i=0;i<MAX_DATA_STREAM;i++)
    //{
    //    l_txVerifyEvmReturn.POWER_MAX[i] = NA_DOUBLE;
    //    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //    if (sizeof(double)==sizeof(l_txVerifyEvmReturn.POWER_MAX[i]))    // Type_Checking
    //    {
    //        setting.value = (void*)&l_txVerifyEvmReturn.POWER_MAX[i];
    //        char tempStr[MAX_BUFFER_SIZE];
    //        sprintf_s(tempStr, "POWER_MAX_%d", i+1);
    //        setting.unit        = "dBm";
    //        setting.helpText    = "Maximum Power over captured packets on individual streams";
    //        l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
    //    }
    //    else    
    //    {
    //        printf("Parameter Type Error!\n");
    //        exit(1);
    //    }
    //}

    //for (int i=0;i<MAX_DATA_STREAM;i++)
    //{
    //    l_txVerifyEvmReturn.POWER_MIN[i] = NA_DOUBLE;
    //    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //    if (sizeof(double)==sizeof(l_txVerifyEvmReturn.POWER_MIN[i]))    // Type_Checking
    //    {
    //        setting.value = (void*)&l_txVerifyEvmReturn.POWER_MIN[i];
    //        char tempStr[MAX_BUFFER_SIZE];
    //        sprintf_s(tempStr, "POWER_MIN_%d", i+1);
    //        setting.unit        = "dBm";
    //        setting.helpText    = "Minimum Power over captured packets on individual streams";
    //        l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
    //    }
    //    else    
    //    {
    //        printf("Parameter Type Error!\n");
    //        exit(1);
    //    }
    //}

	l_txVerifyEvmReturn.DATA_RATE = NA_DOUBLE;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyEvmReturn.DATA_RATE))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmReturn.DATA_RATE;
        setting.unit        = "Mbps";
        setting.helpText    = "Data rate in Mbps, reported by IQAPI.";
        l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyEvmReturn.SPATIAL_STREAM = NA_INTEGER;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyEvmReturn.SPATIAL_STREAM))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyEvmReturn.SPATIAL_STREAM;
        setting.unit        = "";
        setting.helpText    = "Number of spatial stream, reported by IQAPI.";
        l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SPATIAL_STREAM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyEvmReturn.CABLE_LOSS_DB[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester.";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyEvmReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEvmReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
    l_txVerifyEvmReturn.streamRmsAvg.resize(MAX_DATA_STREAM, NA_NUMBER);
	char tempStr[MAX_BUFFER_SIZE];
    for(int i=0;i<MAX_DATA_STREAM;i++)
    {
		l_txVerifyEvmReturn.SYMBOL_CLK_ERR[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.SYMBOL_CLK_ERR[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyEvmReturn.SYMBOL_CLK_ERR[i];
			setting.unit        = "ppm";
			setting.helpText    = "Symbol Clock Error";
			sprintf_s(tempStr, "SYMBOL_CLK_ERR_S%d", i+1);
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyEvmReturn.FREQ_ERROR_AVG[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.FREQ_ERROR_AVG[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyEvmReturn.FREQ_ERROR_AVG[i];
			setting.unit        = "ppm";
			setting.helpText    = "Average frequency error over captured packets";
			sprintf_s(tempStr, "FREQ_ERROR_PPM_AVG_S%d", i+1);
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyEvmReturn.FREQ_ERROR_MAX[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.FREQ_ERROR_MAX[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyEvmReturn.FREQ_ERROR_MAX[i];
			setting.unit        = "ppm";
			setting.helpText    = "Maximum frequency error over captured packets";
			sprintf_s(tempStr, "FREQ_ERROR_PPM_MAX_S%d", i+1);
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyEvmReturn.FREQ_ERROR_MIN[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.FREQ_ERROR_MIN[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyEvmReturn.FREQ_ERROR_MIN[i];
			setting.unit        = "ppm";
			setting.helpText    = "Minimum frequency error over captured packets";
			sprintf_s(tempStr, "FREQ_ERROR_PPM_MIN_S%d", i+1);
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		l_txVerifyEvmReturn.streamRmsAvg[i] = NA_NUMBER;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.streamRmsAvg[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.streamRmsAvg[i];
			sprintf_s(tempStr, "POWER_DBM_RMS_AVG_S%d", i+1);
			setting.unit        = "dBm";
			setting.helpText    = "(Sum) Average RMS power in specified stream for whole VSAs.";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
    }

    l_txVerifyEvmReturn.streamRmsMax.resize(MAX_DATA_STREAM, NA_NUMBER);
    for(int i=0;i<MAX_DATA_STREAM;i++)
    {
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		l_txVerifyEvmReturn.streamRmsMax[i] = NA_NUMBER;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.streamRmsMax[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.streamRmsMax[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "POWER_DBM_RMS_MAX_S%d", i+1);
			setting.unit        = "dBm";
			setting.helpText    = "(Maximum) RMS power in specified stream for whole VSAs.";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
    }

    l_txVerifyEvmReturn.streamRmsMin.resize(MAX_DATA_STREAM, NA_NUMBER);
    for(int i=0;i<MAX_DATA_STREAM;i++)
    {
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		l_txVerifyEvmReturn.streamRmsMin[i] = NA_NUMBER;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.streamRmsMin[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.streamRmsMin[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "POWER_DBM_RMS_MIN_S%d", i+1);
			setting.unit        = "dBm";
			setting.helpText    = "(Minimum) RMS power in specified stream for whole VSAs.";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
    }

    l_txVerifyEvmReturn.vsaRmsAvg.resize(MAX_DATA_STREAM, NA_DOUBLE);
    for(int i=0;i<MAX_DATA_STREAM;i++)
    {
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		l_txVerifyEvmReturn.vsaRmsAvg[i] = NA_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.vsaRmsAvg[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.vsaRmsAvg[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "POWER_DBM_RMS_AVG_VSA%d", i+1);
			setting.unit        = "dBm";
			setting.helpText    = "(Sum) Average RMS power in specified VSA for whole streams.";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
    }
    
    l_txVerifyEvmReturn.vsaRmsMax.resize(MAX_DATA_STREAM, NA_NUMBER);
    for(int i=0;i<MAX_DATA_STREAM;i++)
    {
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		l_txVerifyEvmReturn.vsaRmsMax[i] = NA_NUMBER;
		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.vsaRmsMax[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.vsaRmsMax[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "POWER_DBM_RMS_MAX_VSA%d", i+1);
			setting.unit        = "dBm";
			setting.helpText    = "(Maximum) RMS power in specified VSA for whole streams.";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
    }

    l_txVerifyEvmReturn.vsaRmsMin.resize(MAX_DATA_STREAM, NA_NUMBER);
    for(int i=0;i<MAX_DATA_STREAM;i++)
    {
		setting.type = WIFI_SETTING_TYPE_DOUBLE;

		if (sizeof(double)==sizeof(l_txVerifyEvmReturn.vsaRmsMin[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyEvmReturn.vsaRmsMin[i];
			l_txVerifyEvmReturn.vsaRmsMin[i] = NA_NUMBER;
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "POWER_DBM_RMS_MIN_VSA%d", i+1);
			setting.unit        = "dBm";
			setting.helpText    = "(Minimum) RMS power in specified VSA for whole streams.";
			l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
    }

 //   l_txVerifyEvmReturn.streamAvgAll = NA_DOUBLE;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyEvmReturn.streamAvgAll))    // Type_Checking
	//{
	//	setting.value = (void*)&l_txVerifyEvmReturn.streamAvgAll;
	//	setting.unit        = "dBm";
	//	setting.helpText    = "(Sum) total average RMS power in whole streams.";
	//	l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM_RMS_AVG_STREAM_TOTAL", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

 //   l_txVerifyEvmReturn.streamMaxAll = NA_DOUBLE;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyEvmReturn.streamMaxAll))    // Type_Checking
	//{
	//	setting.value = (void*)&l_txVerifyEvmReturn.streamMaxAll;
	//	setting.unit        = "dBm";
	//	setting.helpText    = "(Maximum) average RMS power in whole streams.";
	//	l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM_RMS_MAX_STREAM_TOTAL", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

 //   l_txVerifyEvmReturn.streamMinAll = NA_DOUBLE;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyEvmReturn.streamMinAll))    // Type_Checking
	//{
	//	setting.value = (void*)&l_txVerifyEvmReturn.streamMinAll;
	//	setting.unit        = "dBm";
	//	setting.helpText    = "(Minimum) average RMS power in whole streams.";
	//	l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM_RMS_MIN_STREAM_TOTAL", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

 //   l_txVerifyEvmReturn.vsaAvgAll = NA_DOUBLE;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyEvmReturn.vsaAvgAll))    // Type_Checking
	//{
	//	setting.value = (void*)&l_txVerifyEvmReturn.vsaAvgAll;
	//	setting.unit        = "dBm";
	//	setting.helpText    = "(Sum) total average RMS power in whole VSAs.";
	//	l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM_RMS_AVG_VSA_TOTAL", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

 //   l_txVerifyEvmReturn.vsaMaxAll = NA_DOUBLE;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyEvmReturn.vsaMaxAll))    // Type_Checking
	//{
	//	setting.value = (void*)&l_txVerifyEvmReturn.vsaMaxAll;
	//	setting.unit        = "dBm";
	//	setting.helpText    = "(Sum) total maximum RMS power in whole VSAs.";
	//	l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM_RMS_MAX_VSA_TOTAL", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

 //   l_txVerifyEvmReturn.vsaMinAll = NA_DOUBLE;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyEvmReturn.vsaMinAll))    // Type_Checking
	//{
	//	setting.value = (void*)&l_txVerifyEvmReturn.vsaMinAll;
	//	setting.unit        = "dBm";
	//	setting.helpText    = "(Sum) total minimum RMS power in whole VSAs.";
	//	l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM_RMS_MIN_VSA_TOTAL", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//
	//for(int i=0;i<MAX_DATA_STREAM;i++)
	//{
	//	for(int j=0;j<MAX_DATA_STREAM;j++)
	//	{
	//		if(i!=j)
	//		{
 //               l_txVerifyEvmReturn.isolationAvgInTwoStreams[i][j] = NA_DOUBLE;
 //               setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//			if (sizeof(double)==sizeof(l_txVerifyEvmReturn.isolationAvgInTwoStreams[i][j]))    // Type_Checking
	//			{
	//				setting.value = (void*)&l_txVerifyEvmReturn.isolationAvgInTwoStreams[i][j];
	//				char tmpStr[MAX_BUFFER_SIZE];
	//				sprintf_s(tmpStr, MAX_BUFFER_SIZE, "ISOLATION_DB_S%d_IN_S%d", i+1, j+1);
	//				setting.unit        = "dB";
	//				setting.helpText    = "Isolation between two streams";
	//				l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tmpStr, setting) );
	//			}
	//			else    
	//			{
	//				printf("Parameter Type Error!\n");
	//				exit(1);
	//			}
	//		}
	//		else
	//		{
	//			//skip, stream # in stream # is 0dbm
	//		}

	//	}

	//}

	for(int i=0;i<MAX_DATA_STREAM;i++)
	{
		for(int j=0;j<MAX_TESTER_NUM;j++)
		{
			l_txVerifyEvmReturn.STREAM_POWER_IN_VSA[i][j] = NA_DOUBLE;
			setting.type = WIFI_SETTING_TYPE_DOUBLE;
			if (sizeof(double)==sizeof(l_txVerifyEvmReturn.STREAM_POWER_IN_VSA[i][j]))    // Type_Checking
			{
				setting.value = (void*)&l_txVerifyEvmReturn.STREAM_POWER_IN_VSA[i][j];
				char tempStr[MAX_BUFFER_SIZE];
				sprintf_s(tempStr, "POWER_DBM_RMS_S%d_VSA%d", i+1, j+1);
				setting.unit        = "dBm";
				setting.helpText    = "(Average) power of stream n (Sn) measured using VS m (VSAm).";
				l_txVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
			}
			else    
			{
				printf("Parameter Type Error!\n");
				exit(1);
			}

		}
	}

    return 0;
}

//double CheckSamplingTime(int wifiMode, char *preamble11B, char *dataRate, char *packetFormat11N)
//{
//	double dataRateInMbps, packetLength;
//	char tempDataRate[100];
//
//	if (wifiMode==WIFI_11N_HT20)
//		sprintf_s(tempDataRate, "HT20_%s", dataRate);
//	else if (wifiMode==WIFI_11N_HT40)
//		sprintf_s(tempDataRate, "HT40_%s", dataRate);
//	else
//		sprintf_s(tempDataRate, "%s", dataRate);
//
//	//This is hard to use in IQwizard as it will assign same value for all data rate, Zhiyong May 17, 2011
//	TM_WiFiConvertDataRateNameToMbps(tempDataRate, &dataRateInMbps);
//
//	packetLength = (g_globalSettingParam.EVM_CAPTURE_PSDU_BYTE*8)/(dataRateInMbps*1e6);		// calculate PSDU packet length time	
// 
//	if ( wifiMode==WIFI_11B )
//	{
//		if (0==strcmp(preamble11B, "LONG"))
//		{
//			packetLength += TIME_11B_LONG_PREAMBLE;
//		}
//		else	// 802.11b preamble = SHORT
//		{
//			if (0==strcmp(dataRate, "DSSS-1"))	//  for 1 Mbps (Long Preamble only)
//			{
//				packetLength += TIME_11B_LONG_PREAMBLE;
//			}
//			else	// Using default sampling time (short preamble)
//			{
//				packetLength += TIME_11B_SHORT_PREAMBLE;
//			}
//		}
//	}
//	else if ( wifiMode==WIFI_11AG )
//	{
//		packetLength += TIME_11G_PREAMBLE;
//	}
//	else	// 802.11n
//	{
//		if (0==strcmp(packetFormat11N, "GREENFIELD"))
//		{			
//			packetLength += TIME_11N_GREEN_FIELD_HEADER;
//		}
//		else	// 802.11n Mixed Mode
//		{
//			packetLength += TIME_11N_MIXED_MODE_HEADER;
//		}
//	}
//
//	packetLength *= 1e6;	// convert packet length to us unit
//	packetLength += g_globalSettingParam.VSA_PRE_TRIGGER_TIME_US;
//	packetLength += 10;		// add 10us trailing space
//
//	return packetLength;
//}

double CheckSamplingTime(int wifiMode, char *preamble11B, char *dataRate, char *packetFormat11N)
{
	double samplingTimeUs = 300;

	if ( wifiMode==WIFI_11B )
	{
		if (0==strcmp(preamble11B, "LONG"))
		{
			samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11B_L_US;
		}
		else    // 802.11b preamble = SHORT
		{
			if (0==strcmp(dataRate, "DSSS-1"))      //  for 1 Mbps (Long Preamble only)
			{
				samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11B_L_US;
			}
			else    // Using default sampling time (short preamble)
			{
				samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11B_S_US;                        
			}
		}
	}
	else if ( wifiMode==WIFI_11AG )
	{
		if (0==strcmp(l_txVerifyEvmParam.BANDWIDTH,"HALF")) {
			samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11P_HALF_US;	
		} else if  (0==strcmp(l_txVerifyEvmParam.BANDWIDTH,"QUAR")) {
			samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11P_QUAR_US;	
		} else {
			samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11AG_US;
		}
	}
	else    // 802.11n
	{
		if (0==strcmp(packetFormat11N, "GREENFIELD"))
		{                       
			samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11N_GREENFIELD_US;
		}
		else    // 802.11n Mixed Mode
		{
			samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11N_MIXED_US;
		}
	}

	return samplingTimeUs;
}

//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
// 
//-------------------------------------------------------------------------------------
int CheckTxEvmParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, int *vsaEnabled, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Checking the Input Parameters
		err = TM_WiFiConvertFrequencyToChannel(l_txVerifyEvmParam.FREQ_MHZ, channel);      
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyEvmParam.FREQ_MHZ);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		err = TM_WiFiConvertDataRateNameToIndex(l_txVerifyEvmParam.DATA_RATE, &dummyInt);      
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_txVerifyEvmParam.DATA_RATE);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		if ( 0!=strcmp(l_txVerifyEvmParam.BANDWIDTH, "HT20") && 0!=strcmp(l_txVerifyEvmParam.BANDWIDTH, "HT40")
			&& 0!=strcmp(l_txVerifyEvmParam.BANDWIDTH, "HALF") && 0!=strcmp(l_txVerifyEvmParam.BANDWIDTH, "QUAR"))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown BANDWIDTH, WiFi bandwidth %s not supported.\n", l_txVerifyEvmParam.BANDWIDTH);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyEvmParam.PREAMBLE, "SHORT") && 0!=strcmp(l_txVerifyEvmParam.PREAMBLE, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_txVerifyEvmParam.PREAMBLE);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, "MIXED") && 0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, "GREENFIELD") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PACKET_FORMAT_11N, WiFi 11n packet format %s not supported.\n", l_txVerifyEvmParam.PACKET_FORMAT_11N);
			throw logMessage;
		}

		// Convert parameter
		err = WiFiTestMode(l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.BANDWIDTH, wifiMode, wifiStreamNum);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Convert WiFi test mode function failed.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFiTestMode() return OK.\n");
		}

		//Check path loss (by ant and freq)
		//temp: declare double *cableLossDb
		//TODO: remove "cableLossDb" in CheckPathLossTable(), each VSA/VSG cable loss will record in CABLE_LOSS_DB of local Param 
		double *cableLossDb= (double *) malloc (sizeof (double));
		err = CheckPathLossTableExt(	g_WiFi_Test_ID,
										l_txVerifyEvmParam.FREQ_MHZ,
										vsaEnabled[0],
										vsaEnabled[1],
										vsaEnabled[2],
										vsaEnabled[3],
										l_txVerifyEvmParam.CABLE_LOSS_DB,
										l_txVerifyEvmReturn.CABLE_LOSS_DB,
										cableLossDb,
										TX_TABLE
									);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] CheckPathLossTableExt() return OK.\n");
		}

		// IQlite merge; Tracy Yu ; 2012-03-31
		double timeUs = CheckSamplingTime(*wifiMode, l_txVerifyEvmParam.PREAMBLE, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.PACKET_FORMAT_11N);
		// Check sampling time 
		if (0==l_txVerifyEvmParam.SAMPLING_TIME_US)
		{
			*samplingTimeUs = CheckSamplingTime(*wifiMode, l_txVerifyEvmParam.PREAMBLE, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.PACKET_FORMAT_11N);
		}
		else	// SAMPLING_TIME_US != 0
		{
			*samplingTimeUs = l_txVerifyEvmParam.SAMPLING_TIME_US;
		}

		// IQlite merge; Tracy Yu ; 2012-03-31
		if ( *wifiMode==WIFI_11B )  
		{
			if ( *channel==14 )
			{
				*peakToAvgRatio = g_globalSettingParam.IQ_P_TO_A_11B_CH14;    // CCK (Channel 14)   
			}
			else
			{
				*peakToAvgRatio = g_globalSettingParam.IQ_P_TO_A_11B_11M;    // CCK 
			}
		}
		else
		{
			*peakToAvgRatio = g_globalSettingParam.IQ_P_TO_A_11AG_54M;   // OFDM 
		}

		// Check Dut configuration changed or not
		if (  g_globalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.
			  0!=strcmp(l_txVerifyEvmParam.BANDWIDTH, g_RecordedParam.BANDWIDTH) ||
			  0!=strcmp(l_txVerifyEvmParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
			  0!=strcmp(l_txVerifyEvmParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
			  0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT_11N, g_RecordedParam.PACKET_FORMAT_11N) ||
			  0!=strcmp(l_txVerifyEvmParam.GUARD_INTERVAL_11N, g_RecordedParam.GUARD_INTERVAL_11N) ||  // IQlite merge; Tracy Yu ; 2012-03-31
			  l_txVerifyEvmParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_txVerifyEvmParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_txVerifyEvmParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_txVerifyEvmParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  l_txVerifyEvmParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
			  l_txVerifyEvmParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
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
		g_RecordedParam.ANT1					= l_txVerifyEvmParam.TX1;
		g_RecordedParam.ANT2					= l_txVerifyEvmParam.TX2;
		g_RecordedParam.ANT3					= l_txVerifyEvmParam.TX3;
		g_RecordedParam.ANT4					= l_txVerifyEvmParam.TX4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txVerifyEvmParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txVerifyEvmParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txVerifyEvmParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txVerifyEvmParam.CABLE_LOSS_DB[3];	
		g_RecordedParam.FREQ_MHZ				= l_txVerifyEvmParam.FREQ_MHZ;	
		g_RecordedParam.POWER_DBM				= l_txVerifyEvmParam.TX_POWER_DBM;	

		sprintf_s(g_RecordedParam.BANDWIDTH, MAX_BUFFER_SIZE, l_txVerifyEvmParam.BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_txVerifyEvmParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_txVerifyEvmParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, l_txVerifyEvmParam.PACKET_FORMAT_11N);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_txVerifyEvmParam.GUARD_INTERVAL_11N);  // IQlite merge; Tracy Yu ; 2012-03-31



		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] CheckTxEvmParameters() Confirmed.\n");
	}
	catch(char *msg)
    {
        sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
    }
    catch(...)
    {
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] Unknown Error!\n");
		err = -1;
    }

	
	return err;
}

// IQlite merge; Tracy Yu ; 2012-03-31
void CleanupTXVerifyEvm()
{
	l_txVerifyEvmParamMap.clear();
	l_txVerifyEvmReturnMap.clear();
}

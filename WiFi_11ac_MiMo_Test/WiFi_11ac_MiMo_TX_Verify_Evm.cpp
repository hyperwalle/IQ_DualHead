#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11ac_MiMo_Test.h"
#include "WiFi_11ac_MiMo_Test_Internal.h"
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
    int    BSS_FREQ_MHZ_PRIMARY;                            /*!< For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment */
	int    BSS_FREQ_MHZ_SECONDARY;                          /*!< For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined */
	int    CH_FREQ_MHZ_PRIMARY_20MHz;                       /*!< The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ"   */
	int    CH_FREQ_MHZ;										/*!< The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used  */
	int    NUM_STREAM_11AC;							        /*!< The number of streams 11AC (Only). */
    char   BSS_BANDWIDTH[MAX_BUFFER_SIZE];                  /*!< The RF bandwidth of basic service set (BSS) to verify EVM. */
	char   CH_BANDWIDTH[MAX_BUFFER_SIZE];                   /*!< The RF channel bandwidth to verify EVM. */
    char   DATA_RATE[MAX_BUFFER_SIZE];                      /*!< The data rate to verify EVM. */
	char   PREAMBLE[MAX_BUFFER_SIZE];                       /*!< The preamble type of 11B(only). */
	char   PACKET_FORMAT[MAX_BUFFER_SIZE];                  /*!< The packet format of 11AC and 11N. */
	char   GUARD_INTERVAL[MAX_BUFFER_SIZE];					/*! The guard interval for 11N and 11AC. */
	char   STANDARD[MAX_BUFFER_SIZE];				        /*!< The standard parameter used for signal analysis option or to discriminate the same data rates/package formats from different standards */
    char   TX_ANALYSIS_ORDER[MAX_BUFFER_SIZE];              /*!< The analysis order to verify EVM */
    double TX_POWER_DBM;                                    /*!< The output power to verify EVM. */
    double CABLE_LOSS_DB[MAX_TESTER_NUM];                   /*!< The path loss of test system. */
    double SAMPLING_TIME_US;                                /*!< The sampling time to verify EVM. */
	//char   GUARD_INTERVAL[MAX_BUFFER_SIZE];					/*!< The guard interval for 11N and 11AC. */



    char   VSA_CONNECTION[MAX_TESTER_NUM][MAX_BUFFER_SIZE]; /*!< The vsa connect to which antenna port. */ 
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

	//channel list
	int    CH_FREQ_MHZ_PRIMARY_40MHz;       /*!< The center frequency (MHz) for primary 40 MHZ channel  */
	int    CH_FREQ_MHZ_PRIMARY_80MHz;       /*!< The center frequency (MHz) for primary 80 MHZ channel  */

    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyEvmReturn;
#pragma endregion

#ifndef WIN32
int initTxVerifyEvmMIMOContainer = InitializeTXVerifyEvmContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Evm.cpp
int CheckTxEvmParameters( int *bssBW, 
						  int *cbw,
						  int *bssPchannel, 
						  int *bssSchannel,
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
*      -# BSS_FREQ_MHZ_PRIMARY: For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment.
*	   -# BSS_FREQ_MHZ_SECONDARY;For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined.
*      -# DATA_RATE (string): The data rate to verify EVM
*      -# TX_POWER_DBM (double): The power (dBm) DUT is going to transmit at the antenna port
*	   -# CH_FREQ_MHZ_PRIMARY_20MHz: The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ".
*	   -# CH_FREQ_MHZ:The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used.
*	   -# BSS_BANDWIDTH[MAX_BUFFER_SIZE]: The RF bandwidth of basic service set (BSS) to verify EVM.
*	   -# CH_BANDWIDTH[MAX_BUFFER_SIZE]: The RF channel bandwidth to verify EVM.
*
* Return Values
*      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/

WIFI_11AC_MIMO_TEST_API int WiFi_11ac_MiMo_TX_Verify_Evm(void)
{
    int    err = ERR_OK;

    bool   analysisOK = false, captureOK  = false;     
    int    avgIteration = 0;
    int    bssPchannel = 0,bssSchannel = 0;
    int    dummyValue = 0;
    int    wifiMode = 0, wifiStreamNum = 0, VSAanylisisFormat = 0;
	int	   antSelection = 0;
	int    bssBW = 0, cbw = 0;
	int    VSAcaptureMode;  //0: 20MHz		1: 40MHz	2:80MHz		3: 160MHz		4: 80_80MHz
    
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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] WiFi_Test_ID or WiFi_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		}
	
		
		TM_ClearReturns(g_WiFi_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_txVerifyEvmParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Get input parameters return OK.\n");
		}

        // Error return of this function is irrelevant
        CheckDutStatus();

		// comment out by Kenual 11/10/2010, to allow NxN to capture SISO signal at Tx2 instead of Tx1
		////Check connection between VSAMaster and antenna port
		//err = CheckConnectionVsaMasterAndAntennaPort(l_txVerifyEvmParamMap);
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi_11ac_MiMo] Connection between VSAMaster and antenna port is not existed.\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Connection between VSAMaster and antenna port is existed.\n");
		//}
		        
        //Map antenna port to Vsa number
        err = CheckConnectionVsaAndAntennaPort(l_txVerifyEvmParamMap, vsaMappingTx, vsaPortStatus);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi_11ac_MiMo] Mapping VSA to antenna port failed\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Mapping VSA to antenna port OK.\n");
		}

		//For Support Dut Chain Status, 0 = OFF, 1 = Main Antenna, 2 = Aux 
		txEnabled[0] = (l_txVerifyEvmParam.TX1==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[1] = (l_txVerifyEvmParam.TX2==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[2] = (l_txVerifyEvmParam.TX3==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[3] = (l_txVerifyEvmParam.TX4==0?Dut_Chain_Disabled:Dut_Chain_Enabled);

        validVsaNum = txEnabled[0] + txEnabled[1] + txEnabled[2] + txEnabled[3];

#pragma region Prepare input parameters

		//Our tester doesn't support BW_80_80MHZ
		if ( 0 ==strcmp(l_txVerifyEvmParam.BSS_BANDWIDTH, "BW-80_80") || 0 ==strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "CBW-80_80") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Current HW version doesn't support BW-80_80 or CBW-80_80!\n");
			throw logMessage;
		}
		else
		{
			//do nothing
		}


		//According Tx enabled status which VSA connect to.
		//By order pass into "CheckTxEvmParameters()
		int vsaEnabled[MAX_TESTER_NUM];
		for(int i=0;i<MAX_TESTER_NUM;i++)
		{
			vsaEnabled[i] = txEnabled[vsaMappingTx[i]-1];
		

		}


		err = CheckTxEvmParameters( &bssBW, &cbw,&bssPchannel,&bssSchannel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &vsaEnabled[0], &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Prepare input parameters CheckTxEvmParameters() return OK.\n");
		}

#pragma endregion

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frequency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

		vDUT_AddIntegerParameter(g_WiFi_Dut, "BSS_FREQ_MHZ_PRIMARY",		l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "BSS_FREQ_MHZ_SECONDARY",		l_txVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ",					l_txVerifyEvmParam.CH_FREQ_MHZ);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_20MHz",	l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "NUM_STREAM_11AC",				l_txVerifyEvmParam.NUM_STREAM_11AC);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",							l_txVerifyEvmParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",							l_txVerifyEvmParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",							l_txVerifyEvmParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",							l_txVerifyEvmParam.TX4);
	
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA1",				l_txVerifyEvmParam.CABLE_LOSS_DB[0]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA2",				l_txVerifyEvmParam.CABLE_LOSS_DB[1]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA3",				l_txVerifyEvmParam.CABLE_LOSS_DB[2]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA4",				l_txVerifyEvmParam.CABLE_LOSS_DB[3]);
        vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",			samplingTimeUs);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",				l_txVerifyEvmParam.TX_POWER_DBM);

		vDUT_AddStringParameter (g_WiFi_Dut, "BSS_BANDWIDTH",				l_txVerifyEvmParam.BSS_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "CH_BANDWIDTH",				l_txVerifyEvmParam.CH_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",					l_txVerifyEvmParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT",				l_txVerifyEvmParam.PACKET_FORMAT);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL",				l_txVerifyEvmParam.GUARD_INTERVAL);
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",					l_txVerifyEvmParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "STANDARD",					l_txVerifyEvmParam.STANDARD);


		// the following parameters are not input parameters, why are they here?  7-16-2012 Jacky 
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_40MHz",	l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz);
		//(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_80MHz",	l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
		if ((wifiMode== WIFI_11N_GF_HT40) || ( wifiMode== WIFI_11N_MF_HT40 ) || (wifiMode == WIFI_11AC_VHT40))
		{
			//vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ",			l_txVerifyEvmParam.CH_FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",		l_txVerifyEvmParam.CH_FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ",		l_txVerifyEvmParam.CH_FREQ_MHZ+10);
		}
		else
		{
			//do nothing
		}		

		vDUT_AddIntegerParameter(g_WiFi_Dut, "BSS_BANDWIDTH",		bssBW);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_BANDWIDTH",		cbw);
		

        

		/*----------------------*
		*   VSA capture mode	*
		*   0: 20MHz			*
		*   0/1: 40MHz			*
		*	0: 80MHz			*
		*	?: 160MHz 			*
		*	?: 80_80MHz			*
		*-----------------------*/
		//if ( cbw == BW_80_80MHZ)			VSAcaptureMode = 4;
		//else if ( cbw == BW_40MHZ)			VSAcaptureMode = 1;
		//else if ( cbw == BW_80MHZ)			VSAcaptureMode = 0;
		//else if ( cbw == BW_160MHZ)			VSAcaptureMode = 3;
		//else								VSAcaptureMode = 0;	

		if ( cbw == BW_80_80MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 2; //not sure about this, need to double check
		else if ( cbw == BW_40MHZ)			VSAcaptureMode = IQV_HT_40_CAPTURE_TYPE;
		else if ( cbw == BW_80MHZ)			VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE; //we don't need wide band capture for EVM analysis.  //IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
		else if ( cbw == BW_160MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //not sure about this, need to double check. 
		else								VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;	

		/*------------------------------------------*
		*	VSA analysis packet format				*					
		*	0: auto detect							*
		*	1: mixed format (11n)					*
		*	2: greenfield format (11n)				*
		*	3: legacy fomat ( 11a/g,11ac Non-HT)	*
		*	4: VHT format ( 11ac, VHT only)			*
		*-------------------------------------------*/

		if ( strstr ( l_txVerifyEvmParam.PACKET_FORMAT, PACKET_FORMAT_HT_MF))	// mixed format, 11n
		{
			VSAanylisisFormat = 1;
		}
		else if ( strstr ( l_txVerifyEvmParam.PACKET_FORMAT, PACKET_FORMAT_HT_GF))	// greeffield format, 11n
		{
			VSAanylisisFormat = 2;
		}
		else if ( strstr ( l_txVerifyEvmParam.PACKET_FORMAT, PACKET_FORMAT_VHT))	// mixed format, 11n
		{
			VSAanylisisFormat = 4;
		}
		else if ( strstr ( l_txVerifyEvmParam.PACKET_FORMAT, PACKET_FORMAT_NON_HT))	// mixed format, 11n
		{
			VSAanylisisFormat = 3;
		}
		else
		{
			VSAanylisisFormat = 0;
		}

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
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(TX_STOP) return OK.\n");
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
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(RF_SET_FREQ) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(RF_SET_FREQ) return OK.\n");
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
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(TX_SET_BW) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(TX_SET_BW) return OK.\n");
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
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(TX_SET_DATA_RATE) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(TX_SET_DATA_RATE) return OK.\n");
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
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(TX_SET_ANTENNA) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(TX_SET_ANTENNA) return OK.\n");
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
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(TX_PRE_TX) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(TX_PRE_TX) return OK.\n");
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
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(TX_START) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{
			   g_vDutTxActived = true;
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(TX_START) return OK.\n");
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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Since g_dutConfigChanged = false, skip Dut control section.\n");
		}


#pragma endregion

#pragma region Setup LP Tester and Capture
		/*--------------------*
		 * Setup IQTester VSA *
		 *--------------------*/
		err = ::LP_SetVsaAmplitudeTolerance(g_globalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);		
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to set VSA amplitude tolerance in dB.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetVsaAmplitudeTolerance(%.3f) return OK.\n", g_globalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		}

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

		err = ::LP_SetVsaNxN(  l_txVerifyEvmParam.CH_FREQ_MHZ*1e6,
								&rfAmplDb[0],
								&vsaPortStatus[0],
								0,
								g_globalSettingParam.VSA_TRIGGER_LEVEL_DB,
								g_globalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000	
							  );

		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi_11ac_MiMo] Fail to setup VSA, LP_SetVsa() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetVsa() return OK.\n");
		}
		


		//mark, for LP_DUT_11AC loop back, enable VSG when VSA is turned on. For Debug.
		for(int i = 0; i<MAX_TESTER_NUM;i++)
		{			
            vsgRFEnabled[i] =  vsaEnabled[i]; 
		}
		//


		//Turn Off VSG in case signal out after initial testers
		err = ::LP_EnableVsgRFNxN(vsgRFEnabled[0], vsgRFEnabled[1], vsgRFEnabled[2], vsgRFEnabled[3]);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi_11ac_MiMo] Fail to setup VSA, LP_EnableVsgRFNxN() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_EnableVsgRFNxN() return OK.\n");
		}


		//Default order: prefOrderSignal={1,2,3,4}
        prefOrderSignal.clear();
		for(int i=0;i<MAX_TESTER_NUM;i++)
		{
			prefOrderSignal.push_back(i+1);
		}
#pragma endregion

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

#pragma region Data capture
		/*------------------*
		 * Start While Loop *
		 *------------------*/
		avgIteration = 0;
		while ( avgIteration<g_globalSettingParam.EVM_AVERAGE )
		{
			analysisOK = false;
			captureOK  = false;
			char sTestDetail[MAX_BUFFER_SIZE] = {'\0'};
			sprintf_s(sTestDetail, MAX_BUFFER_SIZE, "WiFi_11AC_MIMO_TX_Verify_Evm_%d_%s", l_txVerifyEvmParam.CH_FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE);


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


			//g_globalSettingParam.VSA_TRIGGER_TYPE
			err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_globalSettingParam.VSA_TRIGGER_TYPE, 160e6, VSAcaptureMode );     
			if ( ERR_OK!=err )
			{
				double rxAmpl;
				LP_Agc(&rxAmpl, TRUE);	// do auto range on all testers
				err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_globalSettingParam.VSA_TRIGGER_TYPE, 160e6, VSAcaptureMode );     
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to capture signal.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_VsaDataCapture() return OK.\n");
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_VsaDataCapture() return OK.\n");
			}


			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;
			if (1==g_globalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_SaveAlways", l_txVerifyEvmParam.CH_FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.CH_BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}
#pragma endregion

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
							sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211b_Failed", l_txVerifyEvmParam.CH_FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.CH_BANDWIDTH);
							WiFiSaveSigFile(sigFileNameBuffer);

							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_Analyze80211b() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_Analyze80211b() return OK.\n");
						}	
					}
					else // WIFI_11AG
					{
						err = ::LP_SetAnalysisParameterInteger("AnalyzeOFDM", "vsaNum", prefOrderSignal[i]);

						err = ::LP_Analyze80211ag(  g_globalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
													g_globalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
													g_globalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
													g_globalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
													g_globalSettingParam.ANALYSIS_11AG_AMPL_TRACK
													);
						if ( ERR_OK!=err )
						{
							// Fail Analysis, thus save capture (Signal File) for debug
							sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211ag_Failed", l_txVerifyEvmParam.CH_FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.CH_BANDWIDTH);
							WiFiSaveSigFile(sigFileNameBuffer);
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_Analyze80211ag() return error.\n");					
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_Analyze80211ag() return OK.\n");
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
						if (wifiMode==WIFI_11B)
							evmPk[i][avgIteration-1] = ::LP_GetScalarMeasurement("evmPk", 0);
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
			else if ( (wifiMode==WIFI_11N_GF_HT20)||(wifiMode==WIFI_11N_GF_HT40) ||
				(wifiMode==WIFI_11N_MF_HT20)||(wifiMode==WIFI_11N_MF_HT40) )                      
			{   // [Case 03]: MIMO Analysis
				char referenceFileName[MAX_BUFFER_SIZE], analyzeMimoType[MAX_BUFFER_SIZE], analyzeMimoMode[MAX_BUFFER_SIZE];

				switch( g_Tester_Type )
				{
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
													  l_txVerifyEvmParam.CH_BANDWIDTH, 
													  l_txVerifyEvmParam.DATA_RATE, 
													  l_txVerifyEvmParam.PREAMBLE, 
													  l_txVerifyEvmParam.PACKET_FORMAT, 
													  referenceFileName, 
													  MAX_BUFFER_SIZE);							
							if ( ERR_OK!=err )
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to get reference file name.\n");
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] GetWaveformFileName(REFERENCE_FILE_NAME) return OK.\n");
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
						   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return error.\n");
						   throw logMessage;
					   }
					   else
					   {
						   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return OK.\n");
					   }
					                     
					err = ::LP_SetAnalysisParameterIntegerArray("Analyze80211n","prefOrderSignals", &prefOrderSignal[0], (int)prefOrderSignal.size());
       				if ( ERR_OK!=err )
					{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterIntegerArray() return error.\n");
							throw logMessage;
					}
					else
					{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterIntegerArray() return OK.\n");
					}
				}
				else
				{
					 err = ::LP_SetAnalysisParameterInteger("Analyze80211n", "useAllSignals", 0);
         			 if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return OK.\n");
						}
					 err = ::LP_SetAnalysisParameterIntegerArray("Analyze80211n","prefOrderSignals", &prefOrderSignal[0], (int)prefOrderSignal.size());
           			 if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterIntegerArray() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterIntegerArray() return OK.\n");
						}

				}

				//Set Frequency Correction for 802.11n analysis
				err = ::LP_SetAnalysisParameterInteger("Analyze80211n", "frequencyCorr", g_globalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION);
     			if ( ERR_OK!=err )
			    {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return error.\n");
				   throw logMessage;
			    }
				else
				{
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return OK.\n");
				}

				//auto-detect or fixed packet format? discussion with Kenual and Ray.
				//VSAanylisisFormat=0;
				err = ::LP_Analyze80211n( analyzeMimoType,
										analyzeMimoMode,
										g_globalSettingParam.ANALYSIS_11N_PHASE_CORR,    
										g_globalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR,    
										g_globalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING,  
										g_globalSettingParam.ANALYSIS_11N_DECODE_PSDU, 
										g_globalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST,
										referenceFileName,
									    VSAanylisisFormat);   
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_AnalyzeMIMO_Failed", l_txVerifyEvmParam.CH_FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.CH_BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_Analyze80211n() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_Analyze80211n() return OK.\n");
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
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Invalid_Analysis_Results", l_txVerifyEvmParam.CH_FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.CH_BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_GetScalarMeasurement() return error.\n");
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

#pragma region Analysis_802_11ac_mimo And Retrieve Result
			else           
			{   // [Case 04]: 802.11AC Analysis
				
				if(!strcmp(l_txVerifyEvmParam.TX_ANALYSIS_ORDER,"-1,-1,-1,-1"))
				{
					err = ::LP_SetAnalysisParameterInteger("Analyze80211ac", "useAllSignals", 1);
         			if ( ERR_OK!=err )
					   {
						   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return error.\n");
						   throw logMessage;
					   }
					   else
					   {
						   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return OK.\n");
					   }
					                     
					err = ::LP_SetAnalysisParameterIntegerArray("Analyze80211ac","prefOrderSignals", &prefOrderSignal[0], (int)prefOrderSignal.size());
       				if ( ERR_OK!=err )
					{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterIntegerArray() return error.\n");
							throw logMessage;
					}
					else
					{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterIntegerArray() return OK.\n");
					}
				}
				else
				{
					 err = ::LP_SetAnalysisParameterInteger("Analyze80211ac", "useAllSignals", 0);
         			 if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return OK.\n");
						}
					 err = ::LP_SetAnalysisParameterIntegerArray("Analyze80211ac","prefOrderSignals", &prefOrderSignal[0], (int)prefOrderSignal.size());
           			 if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterIntegerArray() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterIntegerArray() return OK.\n");
						}

				}

				//Set Frequency Correction for 802.11ac analysis
				err = ::LP_SetAnalysisParameterInteger("Analyze80211ac", "frequencyCorr", g_globalSettingParam.ANALYSIS_11AC_FREQUENCY_CORRELATION);
     			if ( ERR_OK!=err )
			    {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return error.\n");
				   throw logMessage;
			    }
				else
				{
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetAnalysisParameterInteger() return OK.\n");
				}

				err = ::LP_Analyze80211ac( "nxn",
										g_globalSettingParam.ANALYSIS_11AC_PHASE_CORR,    
										g_globalSettingParam.ANALYSIS_11AC_SYM_TIMING_CORR,    
										g_globalSettingParam.ANALYSIS_11AC_AMPLITUDE_TRACKING,  
										g_globalSettingParam.ANALYSIS_11AC_DECODE_PSDU, 
										g_globalSettingParam.ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST,
										g_globalSettingParam.ANALYSIS_11AC_FREQUENCY_CORRELATION,
										"",
									    VSAanylisisFormat);   

				//err = ::LP_Analyze80211ac();
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_11AC_MIMO_TX_Evm_AnalyzeMIMO_Failed", l_txVerifyEvmParam.CH_FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.CH_BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_Analyze80211ac() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_Analyze80211ac() return OK.\n");
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
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_11AC_MIMO_TX_Evm_Invalid_Analysis_Results", l_txVerifyEvmParam.CH_FREQ_MHZ, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.CH_BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_GetScalarMeasurement() return error.\n");
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
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(TX_STOP) return error.\n");
		//		throw logMessage;
		//	}
		//}
		//else
		//{
		//	g_vDutTxActived = false;
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(TX_STOP) return OK.\n");
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
						l_txVerifyEvmReturn.FREQ_ERROR_AVG[i] = l_txVerifyEvmReturn.FREQ_ERROR_AVG[i]/l_txVerifyEvmParam.CH_FREQ_MHZ;
						l_txVerifyEvmReturn.FREQ_ERROR_MAX[i] = l_txVerifyEvmReturn.FREQ_ERROR_MAX[i]/l_txVerifyEvmParam.CH_FREQ_MHZ;
						l_txVerifyEvmReturn.FREQ_ERROR_MIN[i] = l_txVerifyEvmReturn.FREQ_ERROR_MIN[i]/l_txVerifyEvmParam.CH_FREQ_MHZ;

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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
    }
    catch(...)
    {
		ReturnErrorMessage(l_txVerifyEvmReturn.ERROR_MESSAGE, "[WiFi_11ac_MiMo] Unknown Error!\n");
		err = -1;

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
    }
    return err;

}

int InitializeTXVerifyEvmContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txVerifyEvmParamMap.clear();

    WIFI_SETTING_STRUCT setting;

	strcpy_s(l_txVerifyEvmParam.BSS_BANDWIDTH, MAX_BUFFER_SIZE, "BW-80");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.BSS_BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyEvmParam.BSS_BANDWIDTH;
		setting.unit        = "MHz";
		setting.helpText    = "BSS bandwidth\r\nValid options: BW-20, BW-40, BW-80, BW-80_80 or BW-160";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_BANDWIDTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	strcpy_s(l_txVerifyEvmParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, "0");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.CH_BANDWIDTH))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEvmParam.CH_BANDWIDTH;
        setting.unit        = "MHz";
        setting.helpText    = "Channel bandwidth\r\nValid options:0, CBW-20, CBW-40, CBW-80, CBW-80_80 or CBW-160.\r\nFor 802.11ac, if it is zero,CH_BANDWIDTH equals as BSS_BANDWIDTH. For 802.11/a/b/g/n, it must always have value.";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_BANDWIDTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txVerifyEvmParam.DATA_RATE, MAX_BUFFER_SIZE, "MCS0");
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

	/*strcpy_s(l_txVerifyEvmParam.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.GUARD_INTERVAL))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEvmParam.GUARD_INTERVAL;
        setting.unit        = "";
        setting.helpText    = "The guard interval of 11N or 11AC, can be SHORT or LONG, Default=LONG.";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }*/

    strcpy_s(l_txVerifyEvmParam.PACKET_FORMAT, MAX_BUFFER_SIZE, PACKET_FORMAT_VHT);
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.PACKET_FORMAT))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyEvmParam.PACKET_FORMAT;
        setting.unit        = "";
        setting.helpText    = "The packet format, VHT, HT_MF,HT_GF and NON_HT as defined in standard. Default=VHT.";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

   // "STANDARD"
	strcpy_s(l_txVerifyEvmParam.STANDARD, MAX_BUFFER_SIZE, STANDARD_802_11_AC); 
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.STANDARD))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyEvmParam.STANDARD;
		setting.unit        = "";
		setting.helpText    = "Used for signal analysis option or to discriminating the same data rate or package format from different standards, taking value from 802.11ac, 802.11n, 802.11ag, 802.11b. Default = 802.11ac ";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("STANDARD", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	
	strcpy_s(l_txVerifyEvmParam.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyEvmParam.GUARD_INTERVAL))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyEvmParam.GUARD_INTERVAL;
		setting.unit        = "";
		setting.helpText    = "Packet Guard Interval, Long or Short, default is Long";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL", setting) );
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
        setting.helpText    = "The analysis order while verify EVM, specified VSA analysis order,Default will scan all valid input signals, valid value: 1,2,3,4";
        l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_ANALYSIS_ORDER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

  

	l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY = 5520;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_PRIMARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_SECONDARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz;
		setting.unit        = "MHz";
		setting.helpText    = "The center frequency (MHz) for primary 20 MHZ channel, priority is lower than \"CH_FREQ_MHZ\".";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_20MHz", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyEvmParam.CH_FREQ_MHZ = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyEvmParam.CH_FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.CH_FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "It is the center frequency (MHz) for channel. If it is zero,\"CH_FREQ_MHZ_PRIMARY_20MHz\" will be used for 802.11ac. \r\nFor 802.11/a/b/g/n, it must alway have value.";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerifyEvmParam.NUM_STREAM_11AC = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyEvmParam.NUM_STREAM_11AC))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyEvmParam.NUM_STREAM_11AC;
		setting.unit        = "";
		setting.helpText    = "Number of spatial streams based on 11AC spec";
		l_txVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("NUM_STREAM_11AC", setting) );
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
        setting.helpText    = "Expected power level at DUT antenna port";
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
	l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = 0;
    if (sizeof(int)==sizeof(l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 40 MHZ channel";
        l_txVerifyEvmReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_40MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = 0;
    if (sizeof(int)==sizeof(l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 80 MHZ channel";
        l_txVerifyEvmReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_80MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	


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

double CheckSamplingTime(int wifiMode, char *preamble11B, char *dataRate, char *packetFormat)
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
		samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11AG_US;
	}

	else			// 802.11ac
	{
		if ( 0==strcmp( packetFormat, PACKET_FORMAT_VHT))
		{
			samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11AC_VHT_US;
		}
		else if ( 0==strcmp( packetFormat, PACKET_FORMAT_HT_MF))
		{
			samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11N_MIXED_US;
		}
		else if ( 0==strcmp( packetFormat, PACKET_FORMAT_HT_GF))
		{
			samplingTimeUs = g_globalSettingParam.EVM_CAPTURE_TIME_11N_GREENFIELD_US;
		}
	}

	return samplingTimeUs;
}

//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
// 
//-------------------------------------------------------------------------------------
int CheckTxEvmParameters( int *bssBW, int * cbw,int *bssPchannel,int *bssSchannel,int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, int *vsaEnabled, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
	
	// check common input parameters
          err = CheckCommonParameters_WiFi_11ac_MiMo(l_txVerifyEvmParamMap, 
			                                         l_txVerifyEvmReturnMap, 
												     g_WiFi_Test_ID,
												     vsaEnabled,
												     bssBW, 
												     cbw, 
												     bssPchannel, 
												     bssSchannel, 
												     wifiMode, 
												     wifiStreamNum, 
												     samplingTimeUs);
		  if ( ERR_OK!=err )
		  {
			  LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Check Common Input Parameters failed.\n");
			  throw logMessage;
		  }
		  else
		  {
			  LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Check11ACCommonInputParameters() return OK.\n");
		  }

//		if ( 0!=strcmp(l_txVerifyEvmParam.PREAMBLE, "SHORT") && 0!=strcmp(l_txVerifyEvmParam.PREAMBLE, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_txVerifyEvmParam.PREAMBLE);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0!=strcmp(l_txVerifyEvmParam.GUARD_INTERVAL, "SHORT") && 0!=strcmp(l_txVerifyEvmParam.GUARD_INTERVAL, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown GUARD_INTERVAL, WiFi guard interval %s not supported.\n", l_txVerifyEvmParam.GUARD_INTERVAL);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if (( 0> l_txVerifyEvmParam.NUM_STREAM_11AC) || ( l_txVerifyEvmParam.NUM_STREAM_11AC >8))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] NUM_STREAM_11AC only can be 1~9. Please check input paramter \"NUM_STREAM_11AC\"!\n");
//			throw logMessage;
//		}
//		else
//		{
//			// do nothing
//		}
//		
//		/*------------------------------------------*
//		*   Check data rate	and packet format		*
//		*-------------------------------------------*/
//
//		if ( (0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "11AC_MF_HT")) && (0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "11AC_GF_HT")) &&
//			(0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "11N_MF_HT")) && (0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "11N_GF_HT")) &&
//			(0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "11AC_VHT")) &&( 0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "11AC_NON_HT"))
//			&&( 0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "LEGACY_NON_HT")))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown PACKET_FORMAT, WiFi 11ac packet format %s not supported.\n", l_txVerifyEvmParam.PACKET_FORMAT);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0 == strcmp( l_txVerifyEvmParam.PACKET_FORMAT, "11AC_NON_HT") &&
//			! strstr ( l_txVerifyEvmParam.DATA_RATE, "OFDM"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//				"[WiFi_11ac_MiMo] 11AC_NON_HT, only OFDM-6,9,12,18,24,48,54 supported.\n");
//			throw logMessage;
//		}
//		else
//		{
//		}
//
//		err = TM_WiFiConvertDataRateNameToIndex(l_txVerifyEvmParam.DATA_RATE, &dummyInt);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_txVerifyEvmParam.DATA_RATE);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( 0 == strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "11AC_VHT"))  // Data rate: MCS0 ~MCS9
//		{
//			if (( 14 <= dummyInt ) && (dummyInt <= 23))
//			{
//				// Data rate is right
//				//do nothing
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] PACKET_FORMAT and DATA_RATE don't match! The data rates of VHT_11AC must be MCS0 ~ MCS9, doesn't support %s!\n", &l_txVerifyEvmParam.DATA_RATE);
//				throw logMessage;
//
//			}
//
//			
//		}
//		else
//		{
//		}
//
//		if ( !strstr (l_txVerifyEvmParam.PACKET_FORMAT, "11AC"))    // Legacy signal, CH_BANDWIDTH and CH_FREQ_MHZ must have values
//		{
//			if (( l_txVerifyEvmParam.CH_BANDWIDTH <= 0) || ( l_txVerifyEvmParam.CH_FREQ_MHZ <= 0))
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] For legacy mode,CH_BANDWIDTH and CH_FREQ_MHZ must have values! Please check input parameters! \n");
//				throw logMessage;
//
//			}
//			else
//			{
//				// do nothing
//			}
//		}
//		else
//		{
//			// do nothing
//		}
//
//#pragma region  Check bandwidth and channel list
//
//		/*----------------------*
//		*   Check bandwidth		*
//		*-----------------------*/
//		if ( 0!=strcmp(l_txVerifyEvmParam.BSS_BANDWIDTH, "BW-20") && 0!=strcmp(l_txVerifyEvmParam.BSS_BANDWIDTH, "BW-40") && 
//			 0!=strcmp(l_txVerifyEvmParam.BSS_BANDWIDTH, "BW-80") && 0!=strcmp(l_txVerifyEvmParam.BSS_BANDWIDTH, "BW-160") &&
//			 0!=strcmp(l_txVerifyEvmParam.BSS_BANDWIDTH, "BW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_txVerifyEvmParam.BSS_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_txVerifyEvmParam.BSS_BANDWIDTH, "BW-20"))
//			{
//				*bssBW = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyEvmParam.BSS_BANDWIDTH, "BW-40"))
//			{
//				*bssBW = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyEvmParam.BSS_BANDWIDTH, "BW-80"))
//			{
//				*bssBW = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyEvmParam.BSS_BANDWIDTH, "BW-160"))
//			{
//				*bssBW = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyEvmParam.BSS_BANDWIDTH, "BW-80_80"))
//			{
//				*bssBW = BW_80_80MHZ;
//
//				if ( l_txVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
//				{
//					err = -1;
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//						"[WiFi_11ac_MiMo]  BSS_BANDWIDTH = BW_80_80MHZ,\"BSS_FREQ_MHZ_SECONDARY\" must have value! Please check input parameters!\n" );
//					throw logMessage;
//				}
//				else
//				{
//					// do nothing
//				}
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_txVerifyEvmParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//
//		}
//
//		if ( 0 != strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "0") && 
//			0!=strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "CBW-20") && 0!=strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "CBW-40") && 
//			 0!=strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "CBW-80") && 0!=strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "CBW-160") &&
//			 0!=strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "CBW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown CBW BANDWIDTH, WiFi 11ac CBW bandwidth %s not supported.\n", l_txVerifyEvmParam.CH_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "0"))
//			{
//				sprintf_s(l_txVerifyEvmParam.CH_BANDWIDTH,MAX_BUFFER_SIZE,"C%s",l_txVerifyEvmParam.BSS_BANDWIDTH);
//				*cbw = *bssBW;
//			}
//			else if ( 0 == strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "CBW-20"))
//			{
//				*cbw = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "CBW-40"))
//			{
//				*cbw = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "CBW-80"))
//			{
//				*cbw = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "CBW-160"))
//			{
//				*cbw = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, "CBW-80_80"))
//			{
//				*cbw = BW_80_80MHZ;
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_txVerifyEvmParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//		}
//	
//		if ( strstr ( l_txVerifyEvmParam.PREAMBLE,"11AC"))  //802.11ac, cbw can't be larger than bssBW
//		{
//			if (*bssBW != BW_80_80MHZ)
//			{
//				if (*cbw > *bssBW)
//				{
//					err = -1;
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Channel bandwidth can not be wider than BSS bandwidth.\n");
//					throw logMessage;
//				}
//				else 
//				{
//					// do nothing
//				}
//			}
//			else  //bssBW = BW_80_80MHZ
//			{
//				if ( *cbw == BW_160MHZ)
//				{
//					err = -1;
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] BBS bandwidth = 80+80MHz.Channel bandwidth can't be 160MHz.\n");
//					throw logMessage;
//				}
//				else
//				{
//					// do nothing
//				}
//			}
//		}
//		else   // only need cbw, no use of bssBW
//		{
//			// do nothing
//		}
//
//
//
//		//check cbw
//		if ( strstr( l_txVerifyEvmParam.PACKET_FORMAT, "11N"))  //802.11n, BW= 20,40MHz
//		{
//			if ( *cbw != BW_20MHZ && *cbw != BW_40MHZ)
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//						"[WiFi_11ac_MiMo]  11N bandwidth should be 20MHz or 40Mhz. Please check input parameters!\n" );
//				throw logMessage;
//			}
//			else
//			{
//				// do nothing
//			}
//		}
//		else if ( 0 == strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "LEGACY_NON_HT"))  // 11B and 11ag
//		{
//			if ( *cbw != BW_20MHZ)
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//						"[WiFi_11ac_MiMo]  11B and 11AG bandwidth should be 20MHz. Please check input parameters!\n" );
//				throw logMessage;
//			}
//			else
//			{
//				// do nothing
//			}
//		}
//		else		// 802.11 ac
//		{
//			if ( 0 == strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "11AC_VHT"))
//			{
//				//do nothing
//			}
//			else if ( 0 == strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "11AC_MF_HT") ||
//				0 == strcmp(l_txVerifyEvmParam.PACKET_FORMAT, "11AC_GF_HT"))
//			{
//				if ( *cbw != BW_20MHZ && *cbw != BW_40MHZ)
//				{
//					err = -1;
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//							"[WiFi_11ac_MiMo]  11AC_MF(GF)_HT bandwidth should be 20MHz or 40Mhz. Please check input parameters!\n" );
//					throw logMessage;
//				}
//				else
//				{
//					// do nothing
//				}
//			}
//			else // 11AC_NON_HT
//			{
//				if (*cbw == BW_20MHZ)
//				{
//					err = -1;
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//							"[WiFi_11ac_MiMo]  11AC_NON_HT bandwidth can't be 20MHz. Please check input parameters!\n" );
//					throw logMessage;
//				}
//				else
//				{
//				}
//			}
//		}
//
//		/*--------------------------*
//		*   Check BSS frequency		*
//		*---------------------------*/
//		if (l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY <= 0)
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//					"[WiFi_11ac_MiMo]  \"BSS_FREQ_MHZ_PRIMARY\" must have value! Please check input parameters!\n" );
//			throw logMessage;
//
//		}
//		else
//		{
//			// do nothing
//		}
//
//		err = TM_WiFiConvertFrequencyToChannel(l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY, bssPchannel);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( *bssBW == BW_80_80MHZ)  // Need BSS_FREQ_MHZ_SECONDARY
//		{
//			if ( l_txVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//					"[WiFi_11ac_MiMo]  BSS_BANDWIDTH = BW_80_80MHZ,\"BSS_FREQ_MHZ_SECONDARY\" must have value!\n" );
//				throw logMessage;
//			}
//			else
//			{
//					// do nothing
//			}
//
//			err = TM_WiFiConvertFrequencyToChannel(l_txVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY, bssSchannel);      
//			if ( ERR_OK!=err )
//			{
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY);
//				throw logMessage;
//			}
//			else
//			{
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//			}
//
//		}
//		else   // Don't need BSS_FREQ_MHZ_SECONDARY
//		{
//			bssSchannel = 0;
//		}
//
//		
//		/*-------------------------------------------------------------------*
//		* Find channel list.											 	 *
//		* If CH_FREQ_MHz is not zero, we will directly use it to do caputre. *
//		* if not, we will "CH_FREQ_MHZ_PRIMARY_20" and "BSS_BANDWIDTH"		 *
//		* to calculate the freuqency to capture								 *
//		*--------------------------------------------------------------------*/
//
//		if ( strstr (l_txVerifyEvmParam.PACKET_FORMAT, "11AC"))
//		{
//
//			if ( 0 == l_txVerifyEvmParam.CH_FREQ_MHZ)  
//			{
//				//no CH_FREQ_MHZ, no CH_FREQ_MHZ_PRIMARY_20MHz
//				// Use default values
//				if ( 0 == l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz) 
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi_11ac_MiMo] Don't have input parameters \"CH_FREQ_MHZ\" or \"CH_FREQ_MHZ_PRIMARY_20\", using BBS center freuqency as default!");
//					l_txVerifyEvmParam.CH_FREQ_MHZ = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//
//					// all  use lower frequency for channel list
//					switch (*bssBW)
//					{
//					case BW_20MHZ:
//						l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_40MHZ:
//						l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 10;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_80MHZ:
//						l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 20;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//						break;
//					
//					case BW_160MHZ:			
//						l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//				
//					case BW_80_80MHZ:				
//						l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//
//					default:
//						l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					}
//
//				}
//				else  //no CH_FREQ_MHZ, have CH_FREQ_MHZ_PRIMARY_20MHz. Use input CH_FREQ_MHZ_PRIMARY_20MHz to calculate
//				{
//					err = GetChannelList(*bssBW, l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//					if ( err != ERR_OK) // Wrong channel list
//					{
//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_txVerifyEvmParam.PACKET_FORMAT);
//						throw logMessage;
//					}
//					else  // Get channel list successfully
//					{
//					}
//				}
//
//				if (*cbw != BW_80_80MHZ)
//				{
//					if (*cbw == *bssBW)
//					{
//						l_txVerifyEvmParam.CH_FREQ_MHZ = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//					else if (*cbw == BW_20MHZ)
//					{
//						l_txVerifyEvmParam.CH_FREQ_MHZ = l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz;
//					}
//					else if (*cbw == BW_40MHZ)
//					{
//						l_txVerifyEvmParam.CH_FREQ_MHZ = l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
//					}
//					else if (*cbw == BW_80MHZ)
//					{
//						l_txVerifyEvmParam.CH_FREQ_MHZ = l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
//					}
//					else
//					{
//						l_txVerifyEvmParam.CH_FREQ_MHZ = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//				}
//				else // cbw = BW_80_80MHZ, use BSS primary and secondary
//				{
//					l_txVerifyEvmParam.CH_FREQ_MHZ = NA_INTEGER;
//					// do nothing
//				}
//			}
//			else  // CH_FREQ_MHZ: non-zero 
//			{
//				//Check if input CH_FREQ_MHZ is correct
//				err = CheckChannelFreq(*bssBW, *cbw,
//					l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyEvmParam.CH_FREQ_MHZ);
//				
//				if (err != ERR_OK)
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Input paramter \"CH_FREQ_MHZ\" wrong. Please check!\n");
//					throw logMessage;
//				}
//				else
//				{
//				}
//
//
//				// Check if input CH_FREQ_MHZ_PRIMARY_20MHz.
//				// if zero, calculate by default
//				// If non-zero and correct, use it directly
//				// If not-zero, but incorrect, throw error
//				if ( 0 != l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz)
//				{
//					err = CheckChPrimary20(*bssBW, *cbw,
//							l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyEvmParam.CH_FREQ_MHZ,l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz);
//					if ( err == ERR_OK)  //input CH_FREQ_MHZ_PRIMARY_20MHz is correct
//					{
//						err = GetChannelList(*bssBW, l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//							&l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//						if ( err != ERR_OK) // Wrong channel list
//						{
//							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Can't get channel list. Please check input paramters.\n", l_txVerifyEvmParam.PACKET_FORMAT);
//							throw logMessage;
//						}
//						else  // Get channel list successfully
//						{
//						}
//					}
//					else
//					{
//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Input paramter \"CH_FREQ_MHZ_PRIMARY_20MHz\" wrong. Please check!\n");
//						throw logMessage;
//					}
//				}
//				else //input CH_FREQ_MHZ_PRIMARY_20MHz is zero
//				{
//					//Get channel primary20
//					switch (*bssBW)
//					{
//					case BW_20MHZ:
//						l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					case BW_40MHZ:  //cbw = BW_20.40MHz
//						if (*cbw == BW_20MHZ)
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 10;
//
//						}
//						else  //wrong cbw
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80MHZ:  //cbw = BW_20,40,80MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_txVerifyEvmParam.CH_FREQ_MHZ == l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_txVerifyEvmParam.CH_FREQ_MHZ == l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  //wrong cbw
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz =  NA_INTEGER;
//
//						}
//						break;
//					case BW_160MHZ:  //cbw = BW_20,40,80,160MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_txVerifyEvmParam.CH_FREQ_MHZ == l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 60)
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_txVerifyEvmParam.CH_FREQ_MHZ == l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_txVerifyEvmParam.CH_FREQ_MHZ == l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else if ( l_txVerifyEvmParam.CH_FREQ_MHZ == l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 60)
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 50;
//							}
//							else  //wrong CH_FREQ_MHZ
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							if (l_txVerifyEvmParam.CH_FREQ_MHZ == l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 40)
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_txVerifyEvmParam.CH_FREQ_MHZ == l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 40)
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_160MHZ)
//						{
//							if (l_txVerifyEvmParam.CH_FREQ_MHZ == l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY)
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else  //wring cbw
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80_80MHZ:
//						if ( *cbw == BW_20MHZ)
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if ( l_txVerifyEvmParam.CH_FREQ_MHZ == l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_txVerifyEvmParam.CH_FREQ_MHZ == l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else if ( *cbw == BW_80_80MHZ)
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  // wrong CH_FREQ_MHz
//						{
//							l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					default:
//						l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						break;
//					}
//				}
//
//				err = GetChannelList(*bssBW, l_txVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//				if ( err != ERR_OK)  //Get channel list wrong
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_txVerifyEvmParam.PACKET_FORMAT);
//					throw logMessage;
//				}
//				else  //Get channel list successfully
//				{
//				}
//			}	
//
//			//Check channel list. If all are "NA_INTEGER", return error
//			if (( l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz == NA_INTEGER ) && 
//				(l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz == NA_INTEGER ) &&
//				(l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER))
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20/40/80MHz, please check input parameters.\n");
//				throw logMessage;
//			}
//			else
//			{
//				// do nothing
//			}
//		}
//		else   // non-ac, no use of CH_FREQ_MHZ_PRIMARY_20/40/80MHz
//		{
//			l_txVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//			l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//			l_txVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//		}
//#pragma endregion
//		
//		////VHT, cbw = BW_20MHZ, the stream number of MCS9 only can 2,4,6,8
//		//if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )
//		//{
//		//	if ( (l_txVerifyEvmParam.NUM_STREAM_11AC != 2) ||
//		//		(l_txVerifyEvmParam.NUM_STREAM_11AC != 4) ||
//		//		(l_txVerifyEvmParam.NUM_STREAM_11AC != 6) ||
//		//		(l_txVerifyEvmParam.NUM_STREAM_11AC != 8))
//		//	{
//		//		err = -1;
//		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] 802.11ac CBW = 20MHz, Data Rate = MCS9,NUM_STREAM_11AC only can be 2,4,6,8!\n");
//		//		throw logMessage;
//		//	}
//		//	else
//		//	{
//		//	}
//
//		//}
//		//else
//		//{
//		//	// do nothing
//		//}
//
//		//VHT, cbw = BW_20MHz, the stream number of MCS9 only can 3,6 According to the standard, bguo. 7/17/2012
//		if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )
//		{
//			if ( (l_txVerifyEvmParam.NUM_STREAM_11AC != 3) &&
//				(l_txVerifyEvmParam.NUM_STREAM_11AC != 6) )
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] 802.11ac CBW = 20MHz, Data Rate = MCS9,NUM_STREAM_11AC cannot be 1,2,4,5,7,8!\n");
//				throw logMessage;
//			}
//			else
//			{
//			}
//
//		}
//		else
//		{
//			// do nothing
//		}
//
//		//VHT, cbw = BW_80MHz, the stream number of MCS9 cannot be 6
//		if ( ( *cbw == BW_80MHZ) && ( dummyInt == 23) )
//		{
//			if ( l_txVerifyEvmParam.NUM_STREAM_11AC == 6)
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] 802.11ac CBW = 80MHz, Data Rate = MCS9, NUM_STREAM_11AC cannot be 6!\n");
//				throw logMessage;
//			}
//			else
//			{
//			}
//
//		}
//		else
//		{
//			// do nothing
//		}
//
//		//VHT, cbw = BW_80MHz, the stream number of MCS6 cannot be 3 and 7
//		if ( ( *cbw == BW_80MHZ) && ( dummyInt == 20) )
//		{
//			if ( l_txVerifyEvmParam.NUM_STREAM_11AC == 3 || l_txVerifyEvmParam.NUM_STREAM_11AC == 7)
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] 802.11ac CBW = 80MHz, Data Rate = MCS6, NUM_STREAM_11AC cannot be 3 or 7!\n");
//				throw logMessage;
//			}
//			else
//			{
//			}
//
//		}
//		else
//		{
//			// do nothing
//		}
//
//		// Convert parameter
//		err = WiFi_11ac_TestMode(l_txVerifyEvmParam.DATA_RATE, cbw, wifiMode, wifiStreamNum, l_txVerifyEvmParam.PACKET_FORMAT);
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Convert WiFi test mode function failed.\n");
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] WiFiTestMode() return OK.\n");
//		}
//		
//
//		//Check path loss (by ant and freq)
//		//temp: declare double *cableLossDb
//		//TODO: remove "cableLossDb" in CheckPathLossTable(), each VSA/VSG cable loss will record in CABLE_LOSS_DB of local Param 
//		double *cableLossDb= (double *) malloc (sizeof (double));
//		err = CheckPathLossTableExt(	g_WiFi_Test_ID,
//										l_txVerifyEvmParam.CH_FREQ_MHZ,
//										vsaEnabled[0],
//										vsaEnabled[1],
//										vsaEnabled[2],
//										vsaEnabled[3],
//										l_txVerifyEvmParam.CABLE_LOSS_DB,
//										l_txVerifyEvmReturn.CABLE_LOSS_DB,
//										cableLossDb,
//										TX_TABLE
//									);
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to get CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] CheckPathLossTableExt() return OK.\n");
//		}
//
//		// Check sampling time 
//		if (0==l_txVerifyEvmParam.SAMPLING_TIME_US)
//		{
//			*samplingTimeUs = CheckSamplingTime(*wifiMode, l_txVerifyEvmParam.PREAMBLE, l_txVerifyEvmParam.DATA_RATE, l_txVerifyEvmParam.PACKET_FORMAT);
//		}
//		else	// SAMPLING_TIME_US != 0
//		{
//			*samplingTimeUs = l_txVerifyEvmParam.SAMPLING_TIME_US;
//		}

		  //check unique input parameters used only for TX_EVM test ... 
		  /*-------------------------------------*
		  *  set PAPR values for vsa setting     *
		  *--------------------------------------*/
		if ( *wifiMode==WIFI_11B )  *peakToAvgRatio = g_globalSettingParam.IQ_P_TO_A_11B_11M;    // CCK        
		else                        *peakToAvgRatio = g_globalSettingParam.IQ_P_TO_A_11AG_54M;   // OFDM 


		// Check Dut configuration changed or not
		if (  g_globalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.
			  l_txVerifyEvmParam.CH_FREQ_MHZ != g_RecordedParam.CH_FREQ_MHZ ||
			  0!=strcmp(l_txVerifyEvmParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
			  0!=strcmp(l_txVerifyEvmParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
			  l_txVerifyEvmParam.NUM_STREAM_11AC !=g_RecordedParam.NUM_STREAM_11AC ||
			  0!=strcmp(l_txVerifyEvmParam.PACKET_FORMAT, g_RecordedParam.PACKET_FORMAT) ||
			  0!=strcmp(l_txVerifyEvmParam.GUARD_INTERVAL, g_RecordedParam.GUARD_INTERVAL_11N) ||
			  0!=strcmp(l_txVerifyEvmParam.CH_BANDWIDTH, g_RecordedParam.CH_BANDWIDTH) ||
			  l_txVerifyEvmParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_txVerifyEvmParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_txVerifyEvmParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_txVerifyEvmParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
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
		g_RecordedParam.CH_FREQ_MHZ				= l_txVerifyEvmParam.CH_FREQ_MHZ;
	//	g_RecordedParam.CH_FREQ_MHZ_SECONDARY_80	= l_txVerifyEvmParam.CH_FREQ_MHZ_SECONDARY_80;
		g_RecordedParam.NUM_STREAM_11AC			= l_txVerifyEvmParam.NUM_STREAM_11AC;
		g_RecordedParam.POWER_DBM				= l_txVerifyEvmParam.TX_POWER_DBM;	

		sprintf_s(g_RecordedParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, l_txVerifyEvmParam.CH_BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_txVerifyEvmParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_txVerifyEvmParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT, MAX_BUFFER_SIZE, l_txVerifyEvmParam.PACKET_FORMAT);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_txVerifyEvmParam.GUARD_INTERVAL);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11ac_MiMo] CheckTxEvmParameters() Confirmed.\n");
	}
	catch(char *msg)
    {
        sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
    }
    catch(...)
    {
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11ac_MiMo] Unknown Error!\n");
		err = -1;
    }

	
	return err;
}

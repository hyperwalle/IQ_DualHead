#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11ac_MiMo_Test.h"
#include "WiFi_11ac_MiMo_Test_Internal.h"
#include "IQmeasure.h"
#include "math.h" 
#include "vDUT.h"
#include "StringUtil.h"

using namespace std;

// Carrier number
#define CARRIER_0    0
#define CARRIER_1    1
#define CARRIER_2    2
#define CARRIER_6    6
#define CARRIER_16   16
#define CARRIER_17   17
#define CARRIER_26   26
#define CARRIER_28   28
#define CARRIER_31   31
#define CARRIER_32   32
#define CARRIER_33   33
#define CARRIER_42   42
#define CARRIER_43   43
#define CARRIER_58   58
#define CARRIER_70   70
#define CARRIER_84   84
#define CARRIER_85   85
#define CARRIER_95	 95
#define CARRIER_96	 96
#define CARRIER_97	 97
#define CARRIER_122  122


// This global variable is declared in WiFi_Test_Internal.cpp
extern TM_ID				g_WiFi_Test_ID;
extern vDUT_ID				g_WiFi_Dut;
extern int					g_Tester_Type;
extern int					g_Tester_Number;
extern bool					g_vDutTxActived;
extern bool					g_dutConfigChanged;
extern WIFI_RECORD_PARAM    g_RecordedParam;

int testTimes1 =1000;
// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_globalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyFlatnessParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyFlatnessReturnMap;

struct tagParam
{
    // Mandatory Parameters
    int    BSS_FREQ_MHZ_PRIMARY;                            /*!< For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment */
	int    BSS_FREQ_MHZ_SECONDARY;                          /*!< For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined */
	int    CH_FREQ_MHZ_PRIMARY_20MHz;                       /*!< The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ"   */
	int    CH_FREQ_MHZ;										/*!< The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used  */
	int    NUM_STREAM_11AC;							        /*!< The number of streams 11AC (Only). */
	char   BSS_BANDWIDTH[MAX_BUFFER_SIZE];                  /*!< The RF bandwidth of basic service set (BSS) to verify EVM. */
	char   CH_BANDWIDTH[MAX_BUFFER_SIZE];                  /*!< The RF channel bandwidth to verify spectrum. */
	//char   GUARD_INTERVAL[MAX_BUFFER_SIZE];					/*!< The guard interval for 11N and 11AC. */

	double CORRECTION_FACTOR_11B;                   /*!< The 802.11b(only) LO leakage correction factor. */

    char   DATA_RATE[MAX_BUFFER_SIZE];						/*! The data rate to verify Mask. */
	char   PREAMBLE[MAX_BUFFER_SIZE];						/*! The preamble type of 11B(only). */
	char   PACKET_FORMAT[MAX_BUFFER_SIZE];					/*! The packet format of 11N and 11AC.*/
	char   GUARD_INTERVAL[MAX_BUFFER_SIZE];					/*! The guard interval for 11N and 11AC. */
	char   STANDARD[MAX_BUFFER_SIZE];			        	/*!< The standard parameter used for signal analysis option or to discriminate the same data rates/package formats from different standards */
    double TX_POWER_DBM;									/*! The output power to verify Mask. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];					/*! The path loss of test system. */
    double SAMPLING_TIME_US;								/*! The sampling time to verify Mask. */
	double OBW_PERCENTAGE;									/*! The percentage of OBW, default = 99% */

    char   VSA_CONNECTION[MAX_TESTER_NUM][MAX_BUFFER_SIZE]; /*!< The vsa connect to which antenn port. */ 
    char   VSA_PORT[MAX_TESTER_NUM][MAX_BUFFER_SIZE];		/*!< IQTester VSAs port setting. Default=PORT_LEFT. */  

	char   TX_ANALYSIS_ORDER[MAX_BUFFER_SIZE];              /*!< The analysis order to verify EVM */

    // DUT Parameters
    int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on).  */
    int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off). */
    int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off). */
    int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off). */
} l_txVerifyFlatnessParam;

struct tagReturn
{
    // Flatness Test Result 
    int    CARRIER_NUMBER[MAX_DATA_STREAM];         /*!< Margin at which carrier. */
    double MARGIN_DB[MAX_DATA_STREAM];              /*!< Margin to the flatness, normally should be a plus number if failed.*/
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*!< The path loss of test system. */
    double VALUE_DB[MAX_DATA_STREAM];               /*!< Actually margin value to the average spectral center power. */
    double LO_LEAKAGE_DB[MAX_DATA_STREAM];          /*!< Lo leakage value. */
	int    LEAST_MARGIN_SUBCARRIER_VSA[MAX_TESTER_NUM];    
    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];

    int    CARRIER_NUMBER_VSA[MAX_TESTER_NUM];      /*!< Margin at which carrier. */
	int    SUBCARRIER_LO_A_VSA[MAX_TESTER_NUM];		/*!< Margin at which carrier in lower section A. */
	int    SUBCARRIER_LO_B_VSA[MAX_TESTER_NUM];		/*!< Margin at which carrier in lower section B. */
	int    SUBCARRIER_UP_A_VSA[MAX_TESTER_NUM];		/*!< Margin at which carrier in upper section A. */
	int    SUBCARRIER_UP_B_VSA[MAX_TESTER_NUM];		/*!< Margin at which carrier in upper section B. */

    double MARGIN_DB_VSA[MAX_TESTER_NUM];           /*!< Margin to the flatness, normally should be a plus number if failed. */

	double VALUE_DB_VSA[MAX_TESTER_NUM];            /*!< Actually margin value to the average spectral center power. */
	double VALUE_DB_LO_A_VSA[MAX_TESTER_NUM];       /*!< Actually margin value to the average spectral center power in lower section A. */
	double VALUE_DB_LO_B_VSA[MAX_TESTER_NUM];       /*!< Actually margin value to the average spectral center power in lower section B. */
	double VALUE_DB_UP_A_VSA[MAX_TESTER_NUM];       /*!< Actually margin value to the average spectral center power in upper section A. */
	double VALUE_DB_UP_B_VSA[MAX_TESTER_NUM];       /*!< Actually margin value to the average spectral center power in upper section B. */

	
	double LO_LEAKAGE_DB_VSA[MAX_TESTER_NUM];       /*!< Lo leakage value. */

	//channel list
	int    CH_FREQ_MHZ_PRIMARY_40MHz;       /*!< The center frequency (MHz) for primary 40 MHZ channel  */
	int    CH_FREQ_MHZ_PRIMARY_80MHz;       /*!< The center frequency (MHz) for primary 80 MHZ channel  */

} l_txVerifyFlatnessReturn;
#pragma endregion

#ifndef WIN32
int initTXVerifyFlatnessMIMOContainers = InitializeTXVerifyFlatnessContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Spectrum.cpp
int CheckTxSpectrumParameters( int *bssBW, 
							   int *cbw,
							   int *bssPchannel, 
							   int *bssSchannel,
							   int *wifiMode,
							   int *wifiStreamNum,
							   double *samplingTimeUs,
							   int *vsaEnabled,
							   double *peakToAvgRatio,
							   char* errorMsg );


//! WiFi TX Verify Flatness
/*!
* Input Parameters
*
*  - Mandatory 
*      -# FREQ_MHZ (double): The center frequency (MHz)
*      -# DATA_RATE (string): The data rate to verify Flatness
*      -# TX_POWER_DBM (double): The power (dBm) DUT is going to transmit at the antenna port
*
* Return Values
*      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/


WIFI_11AC_MIMO_TEST_API int WiFi_TX_Verify_Spectrum(void)
{
    int				err = ERR_OK;
    bool			analysisOK = false, captureOK  = false; 
    int				channel = 0;  //VHTMode = 0, packetFormat = 0;
    int				dummyValue = 0;
    int				wifiMode = 0, wifiStreamNum = 0;
	double			samplingTimeUs = 0, peakToAvgRatio = 0;		//, cableLossDb = 0;
	char			vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char			logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char			sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};

	int     		txEnabled[MAX_CHAIN_NUM]={0};
	int     		vsaMappingTx[MAX_CHAIN_NUM]={0};
  	int				validVsaNum = 0;
	int				vsaPortStatus[MAX_TESTER_NUM];


	vector<int>		vsgRFEnabled(MAX_TESTER_NUM, (int)NA_NUMBER);
	vector<int>					prefOrderSignal;
    vector<string>				splits;
    vector<string>::iterator	splitsIter;
    vector<double>				rfAmplDb(MAX_TESTER_NUM, NA_NUMBER);

	//11ac
	int		bssPchannel = 0,bssSchannel = 0;
	int		VSAanylisisFormat = 0;
	int		bssBW = 0, cbw = 0;
	int		VSAcaptureMode;  //0: 20MHz		1: 40MHz	2:80MHz		3: 160MHz		4: 80_80MHz

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_txVerifyFlatnessReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_txVerifyFlatnessParamMap);
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
        RespondToQueryReturn(l_txVerifyFlatnessReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

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

		/*--------------------------*
		 * Get mandatory parameters *
		 *--------------------------*/
		err = GetInputParameters(l_txVerifyFlatnessParamMap);
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
        //Check connection between VSAMaster and antenna port
		//err = CheckConnectionVsaMasterAndAntennaPort(l_txVerifyFlatnessParamMap);
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
        err = CheckConnectionVsaAndAntennaPort(l_txVerifyFlatnessParamMap, vsaMappingTx, vsaPortStatus);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi_11ac_MiMo] Mapping VSA to antenna port failed\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Mapping VSA to antenna port OK.\n");
		}

		txEnabled[0] = (l_txVerifyFlatnessParam.TX1==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[1] = (l_txVerifyFlatnessParam.TX2==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[2] = (l_txVerifyFlatnessParam.TX3==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[3] = (l_txVerifyFlatnessParam.TX4==0?Dut_Chain_Disabled:Dut_Chain_Enabled);

        validVsaNum = txEnabled[0] + txEnabled[1] + txEnabled[2] + txEnabled[3];

#pragma region Prepare input parameters

		//Our tester doesn't support BW_80_80MHZ
		if ( 0 ==strcmp(l_txVerifyFlatnessParam.BSS_BANDWIDTH, "BW-80_80") || 0 ==strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "CBW-80_80") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Current HW version doesn/t support BW-80_80 or CBW-80_80!\n");
			throw logMessage;
		}
		else
		{
			//do nothing
		}


		//According Tx enabled status which VSA connect to.
		//By order pass into "CheckTxSpectrumParameters()
		int vsaEnabled[MAX_TESTER_NUM];
		for(int i=0;i<MAX_TESTER_NUM;i++)
		{
			vsaEnabled[i] = txEnabled[vsaMappingTx[i]-1];

		}



		err = CheckTxSpectrumParameters(&bssBW,&cbw, &bssPchannel,&bssSchannel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &vsaEnabled[0], &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Prepare input parameters CheckTxSpectrumParameters() return OK.\n");
		}

   /*     if (0==strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "MIXED"))
            packetFormat = 1;
        else if (0 == strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "GREENFIELD"))
            packetFormat = 2; 
        else
            packetFormat = 0;*/

#pragma endregion

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

		vDUT_AddIntegerParameter(g_WiFi_Dut, "BSS_FREQ_MHZ_PRIMARY",		l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "BSS_FREQ_MHZ_SECONDARY",		l_txVerifyFlatnessParam.BSS_FREQ_MHZ_SECONDARY);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ",					l_txVerifyFlatnessParam.CH_FREQ_MHZ);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_20MHz",	l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "NUM_STREAM_11AC",				l_txVerifyFlatnessParam.NUM_STREAM_11AC);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",							l_txVerifyFlatnessParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",							l_txVerifyFlatnessParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",							l_txVerifyFlatnessParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",							l_txVerifyFlatnessParam.TX4);
	
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA1",			l_txVerifyFlatnessParam.CABLE_LOSS_DB[0]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA2",			l_txVerifyFlatnessParam.CABLE_LOSS_DB[1]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA3",			l_txVerifyFlatnessParam.CABLE_LOSS_DB[2]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA4",			l_txVerifyFlatnessParam.CABLE_LOSS_DB[3]);
        vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",			samplingTimeUs);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",				l_txVerifyFlatnessParam.TX_POWER_DBM);

		vDUT_AddStringParameter (g_WiFi_Dut, "BSS_BANDWIDTH",				l_txVerifyFlatnessParam.BSS_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "CH_BANDWIDTH",				l_txVerifyFlatnessParam.CH_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",					l_txVerifyFlatnessParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT",				l_txVerifyFlatnessParam.PACKET_FORMAT);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL",				l_txVerifyFlatnessParam.GUARD_INTERVAL);
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",					l_txVerifyFlatnessParam.PREAMBLE);
	    vDUT_AddStringParameter (g_WiFi_Dut, "STANDARD",					l_txVerifyFlatnessParam.STANDARD);
		// the following parameters are not input parameters, why are they here?  7-16-2012 Jacky 
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_40MHz",	l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz);
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_80MHz",	l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
		if ((wifiMode== WIFI_11N_GF_HT40) || ( wifiMode== WIFI_11N_MF_HT40 ) || (wifiMode == WIFI_11AC_VHT40))
		{
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyFlatnessParam.CH_FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyFlatnessParam.CH_FREQ_MHZ+10);
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
		else if ( cbw == BW_80MHZ)			VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE; //we don't need wide band capture for spectrum analysis.  //IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
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

		if ( strstr ( l_txVerifyFlatnessParam.PACKET_FORMAT, PACKET_FORMAT_HT_MF))	// mixed format, 11n
		{
			VSAanylisisFormat = 1;
		}
		else if ( strstr ( l_txVerifyFlatnessParam.PACKET_FORMAT, PACKET_FORMAT_HT_GF))	// greeffield format, 11n
		{
			VSAanylisisFormat = 2;
		}
		else if ( strstr ( l_txVerifyFlatnessParam.PACKET_FORMAT, PACKET_FORMAT_VHT))	// 11ac, VHT only
		{
			VSAanylisisFormat = 4;
		}
		else if ( strstr ( l_txVerifyFlatnessParam.PACKET_FORMAT, PACKET_FORMAT_NON_HT))	// 11a/g,11ac Non-HT
		{
			VSAanylisisFormat = 3;
		}
		else
		{
			VSAanylisisFormat = 0;
		}

 
		//Calculate valid Test Number

		if( (g_dutConfigChanged==true) || (g_vDutTxActived==false))
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


#pragma region Complete prefOrderSignal[]
        //Default order: prefOrderSignal={1,2,3,4}
        prefOrderSignal.clear();
		for(int i=0;i<MAX_TESTER_NUM;i++)
		{
			prefOrderSignal.push_back(i+1);
		}
    

		if(!strcmp(l_txVerifyFlatnessParam.TX_ANALYSIS_ORDER,"-1,-1,-1,-1"))
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
		}
		else
		{
             splits.clear();
             prefOrderSignal.clear();
             SplitString(l_txVerifyFlatnessParam.TX_ANALYSIS_ORDER, splits, ",");
             
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
			rfAmplDb[i] = l_txVerifyFlatnessParam.TX_POWER_DBM-l_txVerifyFlatnessParam.CABLE_LOSS_DB[i]+peakToAvgRatio;
			
			vsgRFEnabled[i] = 0; //Turn Off VSG

		}
		err = ::LP_SetVsaNxN( l_txVerifyFlatnessParam.CH_FREQ_MHZ*1e6,
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


	   /*----------------------------*
		* Perform Normal VSA capture *
		*----------------------------*/
		double sampleFreqHz = 160e6;

		/*------------------------------------------------------------*/
		/*For Spectrum Analysis, in HT20/HT40, using normal capture---*/
		/*------------------------------------------------------------*/

		err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_globalSettingParam.VSA_TRIGGER_TYPE, sampleFreqHz, VSAcaptureMode );     
		if ( ERR_OK!=err )
		{
			double rxAmpl;
			LP_Agc(&rxAmpl, TRUE);	// do auto range on all testers
			err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_globalSettingParam.VSA_TRIGGER_TYPE, sampleFreqHz, VSAcaptureMode );     
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
#pragma endregion

		/*--------------*
		 *  Capture OK  *
		 *--------------*/
		captureOK = true;
		if (1==g_globalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)	// 1: Turn On
		{
			// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_SaveAlways", l_txVerifyFlatnessParam.CH_FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.CH_BANDWIDTH);
			WiFiSaveSigFile(sigFileNameBuffer);
		}
		else
		{
			// do nothing
		}

		/*-------------------------------------------------------*/
		/*	before: step1: capture signal
		/*			step2: analysis each mode such like 802.11ag, 802.11b, 802.11n
		/*			step3: retrieve result and store
		/*-------------------------------------------------------*/
		/*-------------------------------------------------------*/
		/*	After:      step1: capture signal
		/*			for 11b analysis 
		/*				step2: need to specify VSA 
		/*				step3: retrieve result and store
		/*				step4: back to step2, specify the next VSA
		/*			for 11a/g analysis 
		/*				step2: need to specify VSA 
		/*				step3: retrieve result and store
		/*				step4: back to step2, specify the next VSA
		/*          for 11n analysis
		/*			    step2: specify "useAllSignals" and "prefOrderSignal"
		/*				step3: retrieve result and store
		/*-------------------------------------------------------*/


        if ( wifiMode==WIFI_11B )                            
		{


			for(int index=0;index<validVsaNum;index++)
			{
				#pragma region Analysis_802_11b
				if ( (0==strcmp(l_txVerifyFlatnessParam.DATA_RATE, "DSSS-2")) && (1==g_globalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE) )
				{
					// Perform FFT analysis
//					double NFFT   = 0;
//					double res_bw = 0;
//					char   window_type = NULL;

					//Specify the VSA
					err = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", prefOrderSignal[index]);

					err = ::LP_AnalyzeFFT();   
					if ( ERR_OK!=err )
					{
						// Fail Analysis, thus save capture (Signal File) for debug
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txVerifyFlatnessParam.CH_FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.CH_BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_AnalyzeFFT() return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_AnalyzeFFT() return OK.\n");
					}
				}
				else
				{
					// First, Check 11B correction factor  
					if (0==l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B)
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Need correction factor (non-zero) for 802.11B LO leakage measurement.\n");
					}
					else
					{
						//Specify the VSA
						err = ::LP_SetAnalysisParameterInteger("Analyze11b", "vsaNum", prefOrderSignal[index]);
						// Settings: No equalization, DC removal off, classic EVM method
						err = ::LP_Analyze80211b(   1,	//IQV_EQ_OFF,
													0,  //IQV_DC_REMOVAL_OFF,
													1	//IQV_11B_STANDARD_TX_ACC
													); 					
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_Analyze80211b() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_Analyze80211b() return OK.\n");
						}
					}
				}
#pragma endregion
				analysisOK = true;
				#pragma region Retrieve_802_11b_Result

		        if ( (0==strcmp(l_txVerifyFlatnessParam.DATA_RATE, "DSSS-2")) && 
			         (1==g_globalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE) )
		        {
			        // Store the result
			        double bufferRealX[MAX_BUFFER_SIZE], bufferImagX[MAX_BUFFER_SIZE];
			        double bufferRealY[MAX_BUFFER_SIZE], bufferImagY[MAX_BUFFER_SIZE];
			        int    bufferSizeX = ::LP_GetVectorMeasurement("x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE);
			        int    bufferSizeY = ::LP_GetVectorMeasurement("y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE);

			        if ( (0>=bufferSizeX)||(0>=bufferSizeY) )
			        {
				        err = -1;
				        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Can not retrieve FFT result, result length = 0.\n");
				        throw logMessage;
			        }
    				                          					
			        /*--------------------------*
			         *  Looking for peak power  *
			         *--------------------------*/
			        double peakPower = NA_NUMBER;
			        for ( int i=0;i<bufferSizeY;i++ )
			        {
				        if ( bufferRealY[i]>peakPower )  
				        {   // store the current peak power and keep searching
					        peakPower = bufferRealY[i];
				        }
				        else
				        {
					        // keep searching...
				        }
			        }

			        if ( NA_NUMBER>=peakPower )
			        {
				        err = -1;
				        peakPower = NA_NUMBER;
				        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Can not find out the peak power.\n");
				        throw logMessage;
			        }
			        else
			        {
						// Verify DC Leakage 
						int zeroIndex = (int)(bufferSizeX/2) - 3;					
						while(true)
						{						
							if (bufferRealX[zeroIndex]==0)
							{
								// this is the offset "ZERO", break the loop.
								break;
							}
							else
							{
								zeroIndex++;
							}

							if ( zeroIndex>((int)(bufferSizeX/2)+3) )
							{
								err = -1;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to find out the offset zero of the FFT results.\n");
								throw logMessage;
							}
						}
				        // Verify DC Leakage 		
				        double deltaPower = pow(10,(bufferRealY[zeroIndex]/LOG_10)) - pow(10,(peakPower/LOG_10));
				        if (0!=deltaPower)
				        {
							//Avoid negative value in log10()
							if( 0 < deltaPower)
							{
								deltaPower = fabs(deltaPower);
							}
							else
							{
								// There won't exist error in log10(deltaPower) while deltaPower > 0  
							}
					        //l_txVerifyFlatnessReturn.LO_LEAKAGE_DB[0] =  10.0*(log10(deltaPower));					
							l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[prefOrderSignal[index]-1] =  10.0*(log10(deltaPower));
				        }
				        else
				        {
					        err = -1;
					        l_txVerifyFlatnessReturn.LO_LEAKAGE_DB[0] = 0;
					        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to retrieve 11B loLeakageDb.\n");
					        throw logMessage;
				        }
			        }
		        }
		        else
		        {
			        // Store the result
			        double bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
			        int    bufferSize = ::LP_GetVectorMeasurement("loLeakageDb", bufferReal, bufferImag, MAX_BUFFER_SIZE);

			        if (bufferSize>0)
			        {
				        l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[prefOrderSignal[index]-1] =	bufferReal[0] + 
																							l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B;
			        }
			        else
			        {
				        err = -1;
						
						//l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[prefOrderSignal[index]-1] = 0;
						//Fail to retrieve 11B LO LeakageDb, LO_LEAKAGE_DB_VSA should be "NaN"
				        l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[prefOrderSignal[index]-1] = 0;
				        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to retrieve 11B loLeakageDb.\n");
				        throw logMessage;
			        }
		        }
#pragma endregion
			}

		}
		else if(wifiMode == WIFI_11AG)
		{ 
            int index = 0;
            if (l_txVerifyFlatnessParam.TX1 == 1 && l_txVerifyFlatnessParam.TX2 == 0 && 
                l_txVerifyFlatnessParam.TX3 == 0 && l_txVerifyFlatnessParam.TX4 == 0)
                index = 0;
            else if (l_txVerifyFlatnessParam.TX1 == 0 && l_txVerifyFlatnessParam.TX2 == 1 && 
                l_txVerifyFlatnessParam.TX3 == 0 && l_txVerifyFlatnessParam.TX4 == 0)
                index = 1;
            else if (l_txVerifyFlatnessParam.TX1 == 0 && l_txVerifyFlatnessParam.TX2 == 0 && 
                l_txVerifyFlatnessParam.TX3 == 1 && l_txVerifyFlatnessParam.TX4 == 0)
                index = 2;
            else if (l_txVerifyFlatnessParam.TX1 == 0 && l_txVerifyFlatnessParam.TX2 == 0 && 
                l_txVerifyFlatnessParam.TX3 == 0 && l_txVerifyFlatnessParam.TX4 == 1)
                index = 3;
            else
                index = 0;
            //for(int index=0;index<validVsaNum;index++)
			{
				#pragma region Analysis_802_11ag

				//Specify the VSA
				err = ::LP_SetAnalysisParameterInteger("AnalyzeOFDM", "vsaNum", prefOrderSignal[index]);

		        err = ::LP_Analyze80211ag(  g_globalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
									        g_globalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
									        g_globalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
									        g_globalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
									        g_globalSettingParam.ANALYSIS_11AG_AMPL_TRACK
									        );
		        if ( ERR_OK!=err )
		        {
			        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_Analyze80211ag() return error.\n");
			        throw logMessage;
		        }
		        else
		        {
			        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_Analyze80211ag() return OK.\n");
		        }
				#pragma endregion

		        analysisOK = true;

				#pragma region Retrieve_802_11ag_Result
		        err = LoResultSpectralFlatness11AG( &l_txVerifyFlatnessReturn.SUBCARRIER_UP_A_VSA[prefOrderSignal[index]-1], 
													&l_txVerifyFlatnessReturn.SUBCARRIER_UP_B_VSA[prefOrderSignal[index]-1],
													&l_txVerifyFlatnessReturn.SUBCARRIER_LO_A_VSA[prefOrderSignal[index]-1],
													&l_txVerifyFlatnessReturn.SUBCARRIER_LO_B_VSA[prefOrderSignal[index]-1],
											        &l_txVerifyFlatnessReturn.MARGIN_DB_VSA[prefOrderSignal[index]-1], 
													&l_txVerifyFlatnessReturn.LEAST_MARGIN_SUBCARRIER_VSA[prefOrderSignal[index]-1], 
											        &l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[prefOrderSignal[index]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_UP_A_VSA[prefOrderSignal[index]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_UP_B_VSA[prefOrderSignal[index]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_LO_A_VSA[prefOrderSignal[index]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_LO_B_VSA[prefOrderSignal[index]-1]
													);
		        if ( ERR_OK!=err )
		        {
			        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LoResultSpectralFlatness11AG() return error.\n");
			        throw logMessage;
		        }
		        else
		        {
			        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LoResultSpectralFlatness11AG() return OK.\n");
		        }

				#pragma endregion			
			}

		}

        else if ( (wifiMode==WIFI_11N_MF_HT20)||(wifiMode==WIFI_11N_MF_HT40) ||
			(wifiMode==WIFI_11N_GF_HT20)||(wifiMode==WIFI_11N_GF_HT40))                         
        {   
			// [Case 03]: MIMO Analysis
			#pragma region Analysis_802_11n	
			
			//Set Vsa Number
			if(!strcmp(l_txVerifyFlatnessParam.TX_ANALYSIS_ORDER,"-1,-1,-1,-1"))
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

	        err = ::LP_Analyze80211n( "EWC",
							        "nxn",
							        g_globalSettingParam.ANALYSIS_11N_PHASE_CORR,    
							        g_globalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR,    
							        g_globalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING,  
							        g_globalSettingParam.ANALYSIS_11N_DECODE_PSDU, 
							        g_globalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST,
							        "",
							        VSAanylisisFormat);        
	        if ( ERR_OK!=err )
	        {
		        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_Analyze80211n() return error.\n");
		        throw logMessage;
	        }
	        else
	        {
		        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_Analyze80211n() return OK.\n");
	        }
		#pragma endregion

			analysisOK = true;

			#pragma region Retrieve_802_11n_Result
			for (int i=0;i<g_Tester_Number;i++)
	        {
	            err = LoResultSpectralFlatness11N(  1, 
                                                    wifiMode,
													&l_txVerifyFlatnessReturn.SUBCARRIER_UP_A_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.SUBCARRIER_UP_B_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.SUBCARRIER_LO_A_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.SUBCARRIER_LO_B_VSA[prefOrderSignal[i]-1],
									                &l_txVerifyFlatnessReturn.MARGIN_DB_VSA[prefOrderSignal[i]-1], 
													&l_txVerifyFlatnessReturn.LEAST_MARGIN_SUBCARRIER_VSA[prefOrderSignal[i]-1], 
									                &l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_UP_A_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_UP_B_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_LO_A_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_LO_B_VSA[prefOrderSignal[i]-1],
									                i
									               );
	            if ( ERR_OK!=err )
	            {
		            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LoResultSpectralFlatness11N() return error.\n");
		            throw logMessage;
	            }
	            else
	            {
		            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LoResultSpectralFlatness11N() return OK.\n");
	            }
	        }

			#pragma endregion

        }
		else   //802.11 ac
		{
#pragma region Analyze_802_11ac
			//Set Vsa Number
			if(!strcmp(l_txVerifyFlatnessParam.TX_ANALYSIS_ORDER,"-1,-1,-1,-1"))
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
			//Set Frequency Correction for 802.11n analysis
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

	        err = ::LP_Analyze80211ac("nxn",
				g_globalSettingParam.ANALYSIS_11AC_PHASE_CORR,
				g_globalSettingParam.ANALYSIS_11AC_SYM_TIMING_CORR,
				g_globalSettingParam.ANALYSIS_11AC_AMPLITUDE_TRACKING,
				g_globalSettingParam.ANALYSIS_11AC_DECODE_PSDU,
				g_globalSettingParam.ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST,
				g_globalSettingParam.ANALYSIS_11AC_FREQUENCY_CORRELATION,
				"",
				VSAanylisisFormat);


	        if ( ERR_OK!=err )
	        {
		        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_Analyze80211ac() return error.\n");
		        throw logMessage;
	        }
	        else
	        {
		        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_Analyze80211ac() return OK.\n");
	        }
#pragma endregion

			analysisOK = true;

#pragma region Retrieve_802_11ac_Result
			for (int i=0;i<g_Tester_Number;i++)
	        {

				l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[prefOrderSignal[i]-1] = ::LP_GetScalarMeasurement("dcLeakageDbc",i);

	            err = LoResultSpectralFlatness11AC(  1, 
                                                    cbw,
													&l_txVerifyFlatnessReturn.SUBCARRIER_UP_A_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.SUBCARRIER_UP_B_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.SUBCARRIER_LO_A_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.SUBCARRIER_LO_B_VSA[prefOrderSignal[i]-1],
									                &l_txVerifyFlatnessReturn.MARGIN_DB_VSA[prefOrderSignal[i]-1], 
													&l_txVerifyFlatnessReturn.LEAST_MARGIN_SUBCARRIER_VSA[prefOrderSignal[i]-1], 
									                &l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_UP_A_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_UP_B_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_LO_A_VSA[prefOrderSignal[i]-1],
													&l_txVerifyFlatnessReturn.VALUE_DB_LO_B_VSA[prefOrderSignal[i]-1],
									                i
									               );
	            if ( ERR_OK!=err )
	            {
		            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LoResultSpectralFlatness11AC() return error.\n");
		            throw logMessage;
	            }
	            else
	            {
		            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LoResultSpectralFlatness11AC() return OK.\n");
	            }
	        }

#pragma endregion
		}


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

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			sprintf_s(l_txVerifyFlatnessReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyFlatnessReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txVerifyFlatnessReturn.ERROR_MESSAGE, msg);

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
		ReturnErrorMessage(l_txVerifyFlatnessReturn.ERROR_MESSAGE, "[WiFi_11ac_MiMo] Unknown Error!\n");
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

	rfAmplDb.clear();
	vsgRFEnabled.clear();

	testTimes1 ++;

    return err;
}

int InitializeTXVerifyFlatnessContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txVerifyFlatnessParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    strcpy_s(l_txVerifyFlatnessParam.BSS_BANDWIDTH, MAX_BUFFER_SIZE, "BW-80");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.BSS_BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyFlatnessParam.BSS_BANDWIDTH;
		setting.unit        = "MHz";
		setting.helpText    = "BSS bandwidth\r\nValid options: BW-20, BW-40, BW-80, BW-80_80 or BW-160";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_BANDWIDTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	strcpy_s(l_txVerifyFlatnessParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, "0");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.CH_BANDWIDTH))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyFlatnessParam.CH_BANDWIDTH;
        setting.unit        = "MHz";
        setting.helpText    = "Channel bandwidth\r\nValid options:0, CBW-20, CBW-40, CBW-80, CBW-80_80 or CBW-160.\r\nFor 802.11ac, if it is zero,CH_BANDWIDTH equals as BSS_BANDWIDTH. For 802.11/a/b/g/n, it must always have value.";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_BANDWIDTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    sprintf_s(l_txVerifyFlatnessParam.DATA_RATE, MAX_BUFFER_SIZE, "%s", "MCS0");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.DATA_RATE))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyFlatnessParam.DATA_RATE;
        setting.unit        = "";
        setting.helpText    = "Data rate names, such as DSSS-1, CCK-5_5, CCK-11, OFDM-54, MCS0, MCS15";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txVerifyFlatnessParam.PREAMBLE, MAX_BUFFER_SIZE, "SHORT");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.PREAMBLE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyFlatnessParam.PREAMBLE;
        setting.unit        = "";
        setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=SHORT.";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

  /* strcpy_s(l_txVerifyFlatnessParam.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.GUARD_INTERVAL))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyFlatnessParam.GUARD_INTERVAL;
        setting.unit        = "";
        setting.helpText    = "The guard interval of 11N or 11AC, can be SHORT or LONG, Default=LONG.";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }*/

    strcpy_s(l_txVerifyFlatnessParam.PACKET_FORMAT, MAX_BUFFER_SIZE, PACKET_FORMAT_VHT);
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.PACKET_FORMAT))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyFlatnessParam.PACKET_FORMAT;
        setting.unit        = "";
        setting.helpText    = "The packet format, VHT, HT_MF,HT_GF and NON_HT as defined in standard. Default=VHT.";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	// "STANDARD"
	strcpy_s(l_txVerifyFlatnessParam.STANDARD, MAX_BUFFER_SIZE, STANDARD_802_11_AC);
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.STANDARD))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyFlatnessParam.STANDARD;
        setting.unit        = "";
        setting.helpText    = "Used for signal analysis option or to discriminating the same data rate or package format from different standards, taking value from 802.11ac, 802.11n, 802.11ag, 802.11b. Default = 802.11ac.";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("STANDARD", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	
	//"GUARD_INTERVAL" 
	strcpy_s(l_txVerifyFlatnessParam.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.GUARD_INTERVAL))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyFlatnessParam.GUARD_INTERVAL;
		setting.unit        = "";
		setting.helpText    = "Packet Guard Interval, Long or Short, default is Long";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

   l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY = 5520;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_PRIMARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessParam.BSS_FREQ_MHZ_SECONDARY = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.BSS_FREQ_MHZ_SECONDARY))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyFlatnessParam.BSS_FREQ_MHZ_SECONDARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_SECONDARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz;
		setting.unit        = "MHz";
		setting.helpText    = "The center frequency (MHz) for primary 20 MHZ channel, priority is lower than \"CH_FREQ_MHZ\".";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_20MHz", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyFlatnessParam.CH_FREQ_MHZ = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.CH_FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyFlatnessParam.CH_FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "It is the center frequency (MHz) for channel. If it is zero,\"CH_FREQ_MHZ_PRIMARY_20MHz\" will be used for 802.11ac. \r\nFor 802.11/a/b/g/n, it must alway have value.";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerifyFlatnessParam.NUM_STREAM_11AC = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.NUM_STREAM_11AC))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyFlatnessParam.NUM_STREAM_11AC;
		setting.unit        = "";
		setting.helpText    = "Number of spatial streams based on 11AC spec";
		l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("NUM_STREAM_11AC", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_txVerifyFlatnessParam.SAMPLING_TIME_US = 0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyFlatnessParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyFlatnessParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyFlatnessParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyFlatnessParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyFlatnessParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyFlatnessParam.TX_POWER_DBM = 15.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyFlatnessParam.TX_POWER_DBM))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyFlatnessParam.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyFlatnessParam.TX1 = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.TX1))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyFlatnessParam.TX1;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 1 on/off. 1:on; 0:off";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyFlatnessParam.TX2 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.TX2))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyFlatnessParam.TX2;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 2 on/off. 1:on; 0:off";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyFlatnessParam.TX3 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.TX3))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyFlatnessParam.TX3;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 3 on/off. 1:on; 0:off";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyFlatnessParam.TX4 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.TX4))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyFlatnessParam.TX4;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 4 on/off. 1:on; 0:off";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
    l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B = 9.2;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B;
        setting.unit  = "dB";
        setting.helpText  = "802.11b(only) LO leakage correction factor. Chipset specific.";
        l_txVerifyFlatnessParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("CORRECTION_FACTOR_11B_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }



    strcpy_s(l_txVerifyFlatnessParam.TX_ANALYSIS_ORDER, MAX_BUFFER_SIZE, "-1,-1,-1,-1");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.TX_ANALYSIS_ORDER))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyFlatnessParam.TX_ANALYSIS_ORDER;
        setting.unit        = "";
        setting.helpText    = "The analysis order while verify Spectrum, speicified VSA analysis order,Default will scan all valid input signals, valid value: 1,2,3,4";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_ANALYSIS_ORDER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_txVerifyFlatnessParam.VSA_CONNECTION[i], MAX_BUFFER_SIZE, "TX%d", i+1);
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.VSA_CONNECTION[i]))    // Type_Checking
        {
            setting.value       = (void*)l_txVerifyFlatnessParam.VSA_CONNECTION[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the actual connection between DUT antenna ports and VSAs port.\r\nValid value is TX1, TX2, TX3, TX4 and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSA%d_CONNECTION", i+1);
            l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }

    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_txVerifyFlatnessParam.VSA_PORT[i], MAX_BUFFER_SIZE, "");
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.VSA_PORT[i]))    // Type_Checking
        {
            setting.value       = (void*)l_txVerifyFlatnessParam.VSA_PORT[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the VSA port, Default is used global setting.\r\nValid value is Left, Right and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSA%d_PORT", i+1);
            l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
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
    l_txVerifyFlatnessReturnMap.clear();

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz = 0;
    if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 40 MHZ channel";
        l_txVerifyFlatnessReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_40MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz = 0;
    if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 80 MHZ channel";
        l_txVerifyFlatnessReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_80MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    

	//for (int i=0;i<MAX_DATA_STREAM;i++)
 //   {
	//	l_txVerifyFlatnessReturn.CARRIER_NUMBER[i] = NA_INTEGER;
	//	setting.type = WIFI_SETTING_TYPE_INTEGER;
	//	if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.CARRIER_NUMBER[i]))    // Type_Checking
	//	{
	//		setting.value = (void*)&l_txVerifyFlatnessReturn.CARRIER_NUMBER[i];
	//		char tempStr[MAX_BUFFER_SIZE];
 //           sprintf_s(tempStr, "CARRIER_NUMBER_%d", i+1);
	//		setting.unit        = "";
	//		setting.helpText    = "Number of which carrier is failed.";
	//		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
	//	}
	//	else    
	//	{
	//		printf("Parameter Type Error!\n");
	//		exit(1);
	//	}
	//}

	//for (int i=0;i<MAX_DATA_STREAM;i++)
 //   {
	//	l_txVerifyFlatnessReturn.MARGIN_DB[i] = NA_DOUBLE;
	//	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//	if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.MARGIN_DB[i]))    // Type_Checking
	//	{
	//		setting.value = (void*)&l_txVerifyFlatnessReturn.MARGIN_DB[i];
	//		char tempStr[MAX_BUFFER_SIZE];
 //           sprintf_s(tempStr, "MARGIN_DB_%d", i+1);
	//		setting.unit        = "dB";
	//		setting.helpText    = "Margin to the average spectral center power.";
	//		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
	//	}
	//	else    
	//	{
	//		printf("Parameter Type Error!\n");
	//		exit(1);
	//	}
	//}
    
	//for (int i=0;i<MAX_DATA_STREAM;i++)
 //   {
	//	l_txVerifyFlatnessReturn.LO_LEAKAGE_DB[i] = NA_DOUBLE;
	//	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//	if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.LO_LEAKAGE_DB[i]))    // Type_Checking
	//	{
	//		setting.value = (void*)&l_txVerifyFlatnessReturn.LO_LEAKAGE_DB[i];
	//		char tempStr[MAX_BUFFER_SIZE];
 //           sprintf_s(tempStr, "LO_LEAKAGE_DB_%d", i+1);
	//		setting.unit        = "dB";
	//		setting.helpText    = "Lo leakage in dB, normally should be a negative number, if pass.";
	//		l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
	//	}
	//	else    
	//	{
	//		printf("Parameter Type Error!\n");
	//		exit(1);
	//	}
	//}

    //each vsa
    for (int i=0;i<MAX_TESTER_NUM;i++)
    {
		l_txVerifyFlatnessReturn.LEAST_MARGIN_SUBCARRIER_VSA[i] = NA_INTEGER;
		setting.type = WIFI_SETTING_TYPE_INTEGER;
		if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.LEAST_MARGIN_SUBCARRIER_VSA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.LEAST_MARGIN_SUBCARRIER_VSA[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "LEAST_MARGIN_SUBCARRIER_VSA%d", i+1);
			setting.unit        = "";
			setting.helpText    = "Least margin carrier no. in each VSA.";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}


		l_txVerifyFlatnessReturn.SUBCARRIER_LO_A_VSA[i] = NA_INTEGER;
		setting.type = WIFI_SETTING_TYPE_INTEGER;
		if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.SUBCARRIER_LO_A_VSA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.SUBCARRIER_LO_A_VSA[i];
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "SUBCARRIER_LO_A_VSA%d", i+1);
			setting.unit        = "";
			setting.helpText    = "Margin at which carrier of lower section A in each VSA.";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyFlatnessReturn.SUBCARRIER_LO_B_VSA[i] = NA_INTEGER;
		setting.type = WIFI_SETTING_TYPE_INTEGER;
		if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.SUBCARRIER_LO_B_VSA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.SUBCARRIER_LO_B_VSA[i];
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "SUBCARRIER_LO_B_VSA%d", i+1);
			setting.unit        = "";
			setting.helpText    = "Margin at which carrier of lower section B in each VSA.";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyFlatnessReturn.SUBCARRIER_UP_A_VSA[i] = NA_INTEGER;
		setting.type = WIFI_SETTING_TYPE_INTEGER;
		if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.SUBCARRIER_UP_A_VSA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.SUBCARRIER_UP_A_VSA[i];
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "SUBCARRIER_UP_A_VSA%d", i+1);
			setting.unit        = "";
			setting.helpText    = "Margin at which carrier of upper section A in each VSA.";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyFlatnessReturn.SUBCARRIER_UP_B_VSA[i] = NA_INTEGER;
		setting.type = WIFI_SETTING_TYPE_INTEGER;
		if (sizeof(int)==sizeof(l_txVerifyFlatnessReturn.SUBCARRIER_UP_B_VSA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.SUBCARRIER_UP_B_VSA[i];
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "SUBCARRIER_UP_B_VSA%d", i+1);
			setting.unit        = "";
			setting.helpText    = "Margin at which carrier of upper section B in each VSA.";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyFlatnessReturn.MARGIN_DB_VSA[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.MARGIN_DB_VSA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.MARGIN_DB_VSA[i];
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "MARGIN_DB_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Margin to the average spectral center power in each VSA.";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[i];
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "LO_LEAKAGE_DBC_VSA%d", i+1);
			setting.unit        = "dBc";
			setting.helpText    = "Lo leakage in dBc, normally should be a negative number in VSA, if pass.";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyFlatnessReturn.VALUE_DB_LO_A_VSA[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.VALUE_DB_LO_A_VSA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.VALUE_DB_LO_A_VSA[i];
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VALUE_DB_LO_A_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Actually margin value to the average spectral center power of lower section A in each VSA.";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyFlatnessReturn.VALUE_DB_LO_B_VSA[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.VALUE_DB_LO_B_VSA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.VALUE_DB_LO_B_VSA[i];
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VALUE_DB_LO_B_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Actually margin value to the average spectral center power of lower section B in each VSA.";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyFlatnessReturn.VALUE_DB_UP_A_VSA[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.VALUE_DB_UP_A_VSA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.VALUE_DB_UP_A_VSA[i];
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VALUE_DB_UP_A_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Actually margin value to the average spectral center power of upper section A in each VSA.";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_txVerifyFlatnessReturn.VALUE_DB_UP_B_VSA[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.VALUE_DB_UP_B_VSA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.VALUE_DB_UP_B_VSA[i];
			char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VALUE_DB_UP_B_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Actually margin value to the average spectral center power of upper section B in each VSA.";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyFlatnessReturn.CABLE_LOSS_DB[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyFlatnessReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyFlatnessReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyFlatnessReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyFlatnessReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_txVerifyFlatnessReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}


void CheckFlatnessViolation(double deltaPower, int carrier, double lowLimit, double highLimit, double *sectionPower, int *sectionCarrier, double *failMargin, int *sectionCarrier2)
{
	if ( deltaPower>=0 )
	{
		// No matter Failed or OK, we need to save the margin
		if ((deltaPower-highLimit)>(*failMargin))	// Save the margin
		{
			*failMargin = deltaPower-highLimit;
			*sectionPower = deltaPower;
			*sectionCarrier = carrier;
			*sectionCarrier2 = carrier;
		}
	}
	else
	{
		// No matter Failed or OK, we need to save the margin
		if ((lowLimit-deltaPower)>(*failMargin))	// Save the margin
		{
			*failMargin = lowLimit-deltaPower;
			*sectionPower = deltaPower;
			*sectionCarrier = carrier;
			*sectionCarrier2 = carrier;
		}
	}
}


int LoResultSpectralFlatness11AG( int* carrierNoUpA,
								  int* carrierNoUpB,
								  int* carrierNoLoA,
								  int* carrierNoLoB,
								  double* failMargin, 
								  int* leastMarginCarrier,
								  double* loLeakage, 
								  double* valueDbUpSectionA,
								  double* valueDbUpSectionB,
								  double* valueDbLoSectionA,
								  double* valueDbLoSectionB)
{
    int     result = ERR_OK;
   
    int     i, lengthIQ = 0;
    double  *dataQ, *dataI, *data;  
    double  avg_center_power = 0, avg_power = 0;
	int     numberOfEst = 0;
	int     carrierFirst, carrierSecond, carrierThird, carrierFourth;
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	bool    flatnessPass = true;


	/* for non-analysis cas, initialize margin and loLeakage to NaN*/
	*failMargin = NA_DOUBLE;
	*loLeakage = NA_DOUBLE;
	
	*carrierNoUpA = NA_INTEGER;
	*carrierNoUpB = NA_INTEGER;
	*carrierNoLoA = NA_INTEGER;
	*carrierNoLoB = NA_INTEGER;

	/* for non-analysis cas, initialize valueDbUpSectionA/B and valudDbLoSectionA/B to NaN*/
	*valueDbUpSectionA = NA_DOUBLE;
	*valueDbUpSectionB = NA_DOUBLE;
	*valueDbLoSectionA = NA_DOUBLE;
	*valueDbLoSectionB = NA_DOUBLE;

	numberOfEst = ::LP_GetVectorMeasurement("hhEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
    if ( numberOfEst!=64 )	// IQ data length, should be 64
    {
        return -1;
    }
	else
	{
		lengthIQ = numberOfEst; 
	}
    
    dataI    = (double *) malloc (lengthIQ * sizeof (double));
	dataQ    = (double *) malloc (lengthIQ * sizeof (double));
    data     = (double *) malloc (lengthIQ * sizeof (double));

    if (dataQ && dataI && data)
    {
		// Get IQ data.			
		for (int carrier=0;carrier<numberOfEst;carrier++)
		{
			dataI[carrier] = bufferReal[carrier];
			dataQ[carrier] = bufferImag[carrier];

			// 1. Convert I and Q to power: P = I*I + Q*Q (the formula is not 100% correct, but log will take out the ratio)
			// 2. Average P 
			// 3. avg_power = 10 * log (avg_P)

			// Calculate the power at each point from I & Q data.
			data[carrier] = CalculateIQtoP( dataI[carrier], dataQ[carrier] );
		}

		// IEEE 802.11a/g Transmitter flatness
		// The average energy of the constellations in each of the spectral lines ¡V16.. ¡V1 and +1.. +16 will deviate no
		// more than ¡Ó 2 dB from their average energy. The average energy of the constellations in each of the spectral
		// lines ¡V26.. ¡V17 and +17.. +26 will deviate no more than +2/¡V4 dB from the average energy of spectral lines
		// ¡V16.. ¡V1 and +1.. +16. The data for this test shall be derived from the channel estimation step.
		
		carrierFirst  = CARRIER_1;
		carrierSecond = CARRIER_16;
		carrierThird  = CARRIER_17;
		carrierFourth = CARRIER_26;
	
        // Calculate the average energy for the spectral lines
		for (i=carrierFirst;i<=carrierSecond;i++)
		{
			avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
		}
		// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			avg_power = avg_power + data[i] + data[lengthIQ-i];
		}
		avg_power = avg_power + avg_center_power;

		avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
		if (0!=avg_center_power)
		{
			avg_center_power = 10.0 * log10 (avg_center_power);
		}
		else
		{
			avg_center_power = 0;
		}
		avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
		if (0!=avg_power)
		{
			avg_power = 10.0 * log10 (avg_power);
		}
		else
		{
			avg_power = 0;
		}

	    for (int carrier=0;carrier<lengthIQ;carrier++)
	    {
			if (0!=data[carrier])
			{
				data[carrier] = 10.0 * log10 (data[carrier]);
			}
			else
			{
				data[carrier] = 0;
			}
	    }


    // free memory

		// Check the flatness
		double deltaPower = 0;
		double highLimit = 2, lowLimit = -2;
		double sectionMargin[4] = {-99, -99, -99, -99};
		int sectionCarrierNo[4];

		for (i=carrierFirst;i<=carrierSecond;i++)
		{
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
		}

		// Continue, Check the flatness -26 to -17 and +17 to +26.
		lowLimit = -4;
		double sectionBFailMargin = 0;
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
		}

		// get the highest fail margin
		*failMargin = -99;
		for (i=0 ; i<4 ; i++)
		{
			if (sectionMargin[i]>*failMargin)
			{
				*failMargin = sectionMargin[i];
				*leastMarginCarrier = sectionCarrierNo[i];
			}
		}

		// Get the center frequency leakage, data[0] is the center frequency power
		// minus "10 * log10((double)((carrierFourth-carrierFirst+1)*2))" for calculate dBc measurement
		*loLeakage = data[0] - avg_power - 10 * log10((double)((carrierFourth-carrierFirst+1)*2));
//for debug only
		//#if defined(_DEBUG)		
		if (0)
		{
			// print out the Mask into file for debug				
			FILE *fp;  
			fopen_s(&fp, "log/Log_Flatness_Result_11AG.csv", "w");    
			if (fp)
			{
				fprintf(fp, "WiFi Flatness Carrier,Signal,Avg_Center_Power,Size: %d\n", lengthIQ);    // print to log file 
				for (int carrier=0;carrier<lengthIQ;carrier++)
				{
					fprintf( fp, "%d,%8.2f,%8.2f\n", carrier, data[carrier], avg_center_power);    // print to log file 
				}
				fclose(fp);
			}
			else
			{
				// fopen failed
			}
		}
		//#endif
	}
	else
	{
		// if (dataQ && dataI && data)
		result = -1;
	}
    
    if (dataQ)	   free (dataQ);
    if (dataI)	   free (dataI);
    if (data)	   free (data);

    return result;
}

int  LoResultSpectralFlatness11N(int basedOnVsa,
								 int wifiMode,
								 int* carrierNoUpA,
								 int* carrierNoUpB,
								 int* carrierNoLoA,
								 int* carrierNoLoB,
								 double* failMargin,
								 int* leastMarginCarrier,
								 double* loLeakage,
								 double* valueDbUpSectionA,
								 double* valueDbUpSectionB,
								 double* valueDbLoSectionA,
								 double* valueDbLoSectionB,
								 int streamIndex)
// streamIndex = 0, 1, 2, 3 (zero based)
{
    int     result = ERR_OK;
   
    int     i, lengthIQ;     
    double  avg_center_power = 0, avg_power = 0;
	int     numberOfEst = 0, numberOfStream = 0, bandwidthMhz = 20;
	int     carrierFirst, carrierSecond, carrierThird, carrierFourth;
	double  *dataRaw, *data; 
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	bool    flatnessPass = true;
    int		vsaIndex = 0; 


	/* for non-analysis cas, initialize margin and loLeakage to NaN*/
	*failMargin = NA_DOUBLE;
	*loLeakage = NA_DOUBLE;

	*carrierNoUpA = NA_INTEGER;
	*carrierNoUpB = NA_INTEGER;
	*carrierNoLoA = NA_INTEGER;
	*carrierNoLoB = NA_INTEGER;

	/* for non-analysis cas, initialize valuedbUpSectionA/B to NaN*/
	*valueDbUpSectionA = NA_DOUBLE;
	*valueDbUpSectionB = NA_DOUBLE;
	*valueDbLoSectionA = NA_DOUBLE;
	*valueDbLoSectionB = NA_DOUBLE;

	vsaIndex = streamIndex;

    //NOTE: Nstreams x Ntones x NRx
	numberOfEst = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
    if (numberOfEst <= 0)
    {
        return -1;
    }

	numberOfStream = (int)::LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);

    if ((streamIndex >= numberOfStream) && !basedOnVsa)
    {
        return 0;	// special case, need return, but not error.
    }
	//for specify order analysis, capture valid streams, validVsaNum must be <= stream Nums
    if(!strcmp(l_txVerifyFlatnessParam.TX_ANALYSIS_ORDER,"-1,-1,-1,-1"))
	{
		//not specify order analysis
	}
	else
	{
		if(vsaIndex >= numberOfStream)
		{
			return 0; // special case, need return, but not error.
		}
		else
		{
			//valid streams on valid VSA
		}
	}


	bandwidthMhz = (int)::LP_GetScalarMeasurement("rateInfo_bandwidthMhz", 0); 
	if ( (wifiMode==WIFI_11N_MF_HT20 || wifiMode==WIFI_11N_GF_HT20)&&(bandwidthMhz!=20) )
	{
		return -1;
	}
	else if ( (wifiMode==WIFI_11N_MF_HT40 || wifiMode==WIFI_11N_GF_HT40)&&(bandwidthMhz!=40) )
	{
		return -1;
	}
	else
	{
		// IQ data length, should be 64 or 128
		if (bandwidthMhz==20)
		{
			lengthIQ = 64;
		}
		else
		{
			lengthIQ = 128;
		}
	}
 
	dataRaw  = (double *) malloc (numberOfEst * sizeof (double));
    data     = (double *) malloc (lengthIQ * sizeof (double));

    if (dataRaw && data)
    {
		// 1. Convert I and Q to power: P = I*I + Q*Q (the formula is not 100% correct, but log will take out the ratio)
		// 2. Average P 
		// 3. avg_power = 10 * log (avg_P)

		// Calculate the power at each point from I & Q raw data.
		for (int carrier=0;carrier<numberOfEst;carrier++)
		{
			dataRaw[carrier] = CalculateIQtoP( bufferReal[carrier], bufferImag[carrier] );
		}

		// Initial the data[] array
		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			data[carrier] = 0;
		}	

        if(!basedOnVsa)
        {
		    // Get data by stream and each VSA.			
		    for (int carrier=0;carrier<lengthIQ;carrier++)
		    {
			    for (int i=0;i<g_Tester_Number;i++)
			    {
				    int numberOfIndex = (lengthIQ*numberOfStream*i)+(numberOfStream*carrier)+streamIndex;
				    if (numberOfIndex<numberOfEst)
				    {
					    data[carrier] = data[carrier] + dataRaw[numberOfIndex];
				    }
				    else
				    {
					    break;
				    }
			    }		
		    }
        }
        else // based on Vsa
        {
		    for (int carrier=0;carrier<lengthIQ;carrier++)
		    {
			    for (int i=0;i<numberOfStream;i++)
			    {
				    int numberOfIndex = (lengthIQ*numberOfStream*vsaIndex)+(numberOfStream*carrier)+i;
				    if (numberOfIndex<numberOfEst)
				    {
					    data[carrier] = data[carrier] + dataRaw[numberOfIndex];
				    }
				    else
				    {
					    break;
				    }
			    }		
		    }

        }




		// IEEE std 802.11n Transmitter flatness
		// HT20	
		// In a 20 MHz channel and in corresponding 20 MHz transmission in a 40 MHz channel, the average energy
		// of the constellations in each of the spectral lines ¡V16 to ¡V1 and +1 to +16 shall deviate no more than ¡Ó 2 dB
		// from their average energy. The average energy of the constellations in each of the spectral lines ¡V28 to ¡V17
		// and +17 to +28 shall deviate no more than +2/¡V4 dB from the average energy of spectral lines ¡V16 to ¡V1 and +1 to +16.

		// HT40
		// In a 40 MHz transmission (excluding HT duplicate format and non-HT duplicate format) the average energy
		// of the constellations in each of the spectral lines ¡V42 to ¡V2 and +2 to +42 shall deviate no more than ¡Ó 2 dB
		// from their average energy. The average energy of the constellations in each of the spectral lines ¡V43 to ¡V58
		// and +43 to +58 shall deviate no more than +2/¡V4 dB from the average energy of spectral lines ¡V42 to ¡V2 and +2 to +42.

		// In HT duplicate format and non-HT duplicate format the average energy of the constellations in each of the
		// spectral lines -42 to -33, -31 to -6, +6 to +31, and +33 to +42 shall deviate no more than ¡Ó 2 dB from their
		// average energy. The average energy of the constellations in each of the spectral lines -43 to -58 and +43 to
		// +58 shall deviate no more than +2/-4 dB from the average energy of spectral lines -42 to -33, -31 to -6, +6
		// to +31, and +33 to +42.

		if (lengthIQ==64)	// HT20 mode
		{
			carrierFirst  = CARRIER_1;
			carrierSecond = CARRIER_16;
			carrierThird  = CARRIER_17;
			carrierFourth = CARRIER_28;
		}
		else				// HT40 mode, lengthIQ=128
		{
			carrierFirst  = CARRIER_2;
			carrierSecond = CARRIER_42;
			carrierThird  = CARRIER_43;
			carrierFourth = CARRIER_58;
		}		


        /* Carrier number report order:
        * 20MHz: +1 ~ +32, -32 ~ -1
        * 40MHz: +1 ~ +64, -64 ~ -1
        */
        // Calculate the average energy for the spectral lines
		for ( i=carrierFirst;i<=carrierSecond;i++)
		{
			avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
		}
		// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			avg_power = avg_power + data[i] + data[lengthIQ-i];
		}
		avg_power = avg_power + avg_center_power;

		avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
		if (0!=avg_center_power)
		{
			avg_center_power = 10.0 * log10 (avg_center_power);
		}
		else
		{
			avg_center_power= 0;
		}
		avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
		if (0!=avg_power)
		{
			avg_power = 10.0 * log10 (avg_power);
		}
		else
		{
			avg_power = 0;
		}

		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			if (0!=data[carrier])
			{
				data[carrier] = 10.0 * log10 (data[carrier]);
			}
			else
			{
				data[carrier] = 0;
			}
		}

		// Check the flatness
		double deltaPower = 0;
		double highLimit = 2, lowLimit = -2;
		double sectionMargin[4] = {-99, -99, -99, -99};
		int sectionCarrierNo[4];

		for (i=carrierFirst;i<=carrierSecond;i++)
		{
			//UPPER SECTION A
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

			//LOWER SECTION A
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
		}

		// Continue, Check the flatness. (side)
		lowLimit = -4;
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			//UPPER SECTION B
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);
			
			//LOWER SECTION B
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
		}

		// get the highest fail margin
		*failMargin = -99;
		for (i=0 ; i<4 ; i++)
		{
			if (sectionMargin[i]>*failMargin)
			{
				*failMargin = sectionMargin[i];
				*leastMarginCarrier = sectionCarrierNo[i];
			}
		}

		// Get the center frequency leakage 
		*loLeakage = data[0] - avg_power - 10 * log10((double)((carrierFourth-carrierFirst+1)*2));
//for debug only
//		#if defined(_DEBUG)
		if (0)
		{
			// print out the Mask into file for debug				
			FILE *fp;  
			char sigFileName[MAX_BUFFER_SIZE];
			sprintf_s(sigFileName, "log/Log_Flatness_Result_11N_Stream%02d.csv", streamIndex+1);
			fopen_s(&fp, sigFileName, "w");      
			if (fp)
			{
				fprintf(fp, "WiFi Flatness Carrier,Signal,Avg_Center_Power,Size: %d\n", lengthIQ);    // print to log file 
				for (int carrier=0;carrier<lengthIQ;carrier++)
				{
					fprintf( fp, "%d,%8.2f,%8.2f\n", carrier, data[carrier], avg_center_power);    // print to log file 
				}
				fclose(fp);
			}
			else
			{
				// fopen failed
			}    
		}
		//#endif
	}
	else
	{
		// if (dataQ && dataI && data && maxData && minData)
		result = -1;
	}
    
    if (dataRaw)   free (dataRaw);
	if (data)	   free (data);

    return result;
}

int  LoResultSpectralFlatness11AC(int basedOnVsa,
								 int cbw,
								 int* carrierNoUpA,
								 int* carrierNoUpB,
								 int* carrierNoLoA,
								 int* carrierNoLoB,
								 double* failMargin,
								 int* leastMarginCarrier,
								 double* loLeakage,
								 double* valueDbUpSectionA,
								 double* valueDbUpSectionB,
								 double* valueDbLoSectionA,
								 double* valueDbLoSectionB,
								 int streamIndex)
// streamIndex = 0, 1, 2, 3 (zero based)
{
    int     result = ERR_OK;
   
    int     i, lengthIQ;     
    double  avg_center_power = 0, avg_power = 0;
	int     numberOfEst = 0, numberOfStream = 0, bandwidthMhz = 20;
	int     carrierFirst, carrierSecond, carrierThird, carrierFourth;
	
	double  *dataRaw, *data; 
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	bool    flatnessPass = true;
    int		vsaIndex = 0; 

	double sectionMargin[4] = {-99, -99, -99, -99};
	int sectionCarrierNo[4];

	// Added for 802.11ac
	int		carrierPilot;
	int		carrierFifth, carrierSixth, carrierSeventh,	carrierEigth, carrierNinth,carrierTenth;  
	int		carrierNumSection1, carrierNumSection2;
	//END



	/* for non-analysis cas, initialize margin and loLeakage to NaN*/
	*failMargin = NA_DOUBLE;
//	*loLeakage = NA_DOUBLE;

	*carrierNoUpA = NA_INTEGER;
	*carrierNoUpB = NA_INTEGER;
	*carrierNoLoA = NA_INTEGER;
	*carrierNoLoB = NA_INTEGER;

	/* for non-analysis cas, initialize valuedbUpSectionA/B to NaN*/
	*valueDbUpSectionA = NA_DOUBLE;
	*valueDbUpSectionB = NA_DOUBLE;
	*valueDbLoSectionA = NA_DOUBLE;
	*valueDbLoSectionB = NA_DOUBLE;

	vsaIndex = streamIndex;

    //NOTE: Nstreams x Ntones x NRx
	numberOfEst = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
    if (numberOfEst <= 0)
    {
        return -1;
    }

	numberOfStream = (int)::LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);

    if ((streamIndex >= numberOfStream) && !basedOnVsa)
    {
        return 0;	// special case, need return, but not error.
    }
	//for specify order analysis, capture valid streams, validVsaNum must be <= stream Nums
    if(!strcmp(l_txVerifyFlatnessParam.TX_ANALYSIS_ORDER,"-1,-1,-1,-1"))
	{
		//not specify order analysis
	}
	else
	{
		if(vsaIndex >= numberOfStream)
		{
			return 0; // special case, need return, but not error.
		}
		else
		{
			//valid streams on valid VSA
		}
	}


	bandwidthMhz = (int)::LP_GetScalarMeasurement("rateInfo_bandwidthMhz", 0); 
	if ( ( cbw == BW_20MHZ) &&(bandwidthMhz!=20) )
	{
		return -1;
	}
	else if ( ( cbw == BW_40MHZ )&&(bandwidthMhz!=40) )
	{
		return -1;
	}
	else if ( ( cbw == BW_80MHZ )&&(bandwidthMhz!=80) )
	{
		return -1;
	}
	else if ( ( cbw == BW_160MHZ )&&(bandwidthMhz!=160) )
	{
		return -1;
	}
	else if (( cbw == BW_80_80MHZ ) &&(bandwidthMhz!=160) )     // BW_80_80MHZ not supported yet
	{
		return -1;
	}
	else     // bandwidthMHz is correct
	{
		if ( 0 != strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT,PACKET_FORMAT_NON_HT))    //802.11ac VHT, MF_HT, GF_HT
		{
			// IQ data length, should be 64 or 128
			if ( bandwidthMhz == 20)
			{
				lengthIQ = 64;
				carrierFirst  = CARRIER_1;
				carrierSecond = CARRIER_16;
				carrierThird  = CARRIER_17;
				carrierFourth = CARRIER_28;
			}
			else if (bandwidthMhz == 40)
			{
				lengthIQ = 128;
				carrierFirst  = CARRIER_2;
				carrierSecond = CARRIER_42;
				carrierThird  = CARRIER_43;
				carrierFourth = CARRIER_58;
			}
			else if (bandwidthMhz == 80)
			{
				lengthIQ = 256;
				carrierFirst  = CARRIER_2;
				carrierSecond = CARRIER_84;
				carrierThird  = CARRIER_85;
				carrierFourth = CARRIER_122;
			}
			else if (bandwidthMhz == 160)
			{
				// Not supported yet
				return 0;
			}
			else   // 80_80
			{
				// Not supported yet
				return 0;
			}
		}
		else   //802.11ac non-HT duplicate
		{
			if ( bandwidthMhz == 40)
			{
				lengthIQ = 128;
				carrierFirst  = CARRIER_6;
				carrierSecond = CARRIER_42;
				carrierPilot = CARRIER_32;
				carrierThird  = CARRIER_43;
				carrierFourth = CARRIER_58;
			//	carrierSecond = CARRIER_31;
			//	carrierThird  = CARRIER_33;
			//	carrierFourth = CARRIER_42;
			//	carrierFifth  = CARRIER_43;
			//	carrierSixth  = CARRIER_58;

			}
			else if (bandwidthMhz == 80)  // bandwidth = 80
			{
				lengthIQ = 256;
				carrierFirst	= CARRIER_6;
				carrierSecond	= CARRIER_31;
				carrierThird	= CARRIER_33;
				carrierFourth	= CARRIER_58;
				carrierFifth	= CARRIER_70;
				carrierSixth	= CARRIER_84;
				carrierNumSection1 = (carrierSecond - carrierFirst + 1) +
									 (carrierFourth - carrierThird + 1) +
									 (carrierSixth - carrierFifth + 1);

				carrierSeventh  = CARRIER_85;
				carrierEigth	= CARRIER_95;
				carrierNinth	= CARRIER_97;
				carrierTenth	= CARRIER_122;
				carrierNumSection2 = ( carrierEigth - carrierSeventh + 1) +
									( carrierTenth - carrierNinth +1);

			}
			else if ( bandwidthMhz == 160)
			{
				// Not supported yet
			}
			else		// bandwidth == 80_80
			{
				// Not supported yet
			}
		}
	}
 
	dataRaw  = (double *) malloc (numberOfEst * sizeof (double));
    data     = (double *) malloc (lengthIQ * sizeof (double));

    if (dataRaw && data)
    {
		// 1. Convert I and Q to power: P = I*I + Q*Q (the formula is not 100% correct, but log will take out the ratio)
		// 2. Average P 
		// 3. avg_power = 10 * log (avg_P)

		// Calculate the power at each point from I & Q raw data.
		for (int carrier=0;carrier<numberOfEst;carrier++)
		{
			dataRaw[carrier] = CalculateIQtoP( bufferReal[carrier], bufferImag[carrier] );
		}

		// Initial the data[] array
		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			data[carrier] = 0;
		}	

        if(!basedOnVsa)
        {
		    // Get data by stream and each VSA.			
		    for (int carrier=0;carrier<lengthIQ;carrier++)
		    {
			    for (int i=0;i<g_Tester_Number;i++)
			    {
				    int numberOfIndex = (lengthIQ*numberOfStream*i)+(numberOfStream*carrier)+streamIndex;
				    if (numberOfIndex<numberOfEst)
				    {
					    data[carrier] = data[carrier] + dataRaw[numberOfIndex];
				    }
				    else
				    {
					    break;
				    }
			    }		
		    }
        }
        else // based on Vsa
        {
		    for (int carrier=0;carrier<lengthIQ;carrier++)
		    {
			    for (int i=0;i<numberOfStream;i++)
			    {
				    int numberOfIndex = (lengthIQ*numberOfStream*vsaIndex)+(numberOfStream*carrier)+i;
				    if (numberOfIndex<numberOfEst)
				    {
					    data[carrier] = data[carrier] + dataRaw[numberOfIndex];
				    }
				    else
				    {
					    break;
				    }
			    }		
		    }

        }


		if ( 0 != strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT,PACKET_FORMAT_NON_HT) && bandwidthMhz != 160)    //802.11ac VHT, MF_HT, GF_HT, BW = 20,40,80, 80_80
		{
			// Calculate the average energy for the spectral lines
			for ( i=carrierFirst;i<=carrierSecond;i++)
			{
				avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
			}
			// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
			for (i=carrierThird;i<=carrierFourth;i++)
			{
				avg_power = avg_power + data[i] + data[lengthIQ-i];
			}
			avg_power = avg_power + avg_center_power;

			avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
			if (0!=avg_center_power)
			{
				avg_center_power = 10.0 * log10 (avg_center_power);
			}
			else
			{
				avg_center_power= 0;
			}
			avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
			if (0!=avg_power)
			{
				avg_power = 10.0 * log10 (avg_power);
			}
			else
			{
				avg_power = 0;
			}

			for (int carrier=0;carrier<lengthIQ;carrier++)
			{
				if (0!=data[carrier])
				{
					data[carrier] = 10.0 * log10 (data[carrier]);
				}
				else
				{
					data[carrier] = 0;
				}
			}


			// Check the flatness
			double deltaPower = 0;
			double highLimit = 4, lowLimit = -4;
			//double sectionMargin[4] = {-99, -99, -99, -99};
			//int sectionCarrierNo[4];

			for (i=carrierFirst;i<=carrierSecond;i++)
			{
				//UPPER SECTION A
				deltaPower = data[i]-avg_center_power;
				CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

				//LOWER SECTION A
				deltaPower = data[lengthIQ-i]-avg_center_power;
				CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
			}

			// Continue, Check the flatness. (side)
			highLimit = 4, lowLimit = -6;
			for (i=carrierThird;i<=carrierFourth;i++)
			{
				//UPPER SECTION B
				deltaPower = data[i]-avg_center_power;
				CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);
				
				//LOWER SECTION B
				deltaPower = data[lengthIQ-i]-avg_center_power;
				CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
			}

		}
		else if ( 0 != strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT,PACKET_FORMAT_NON_HT) && bandwidthMhz == 160) 
		{
			// BW = 160MHz, not supported yet
		}
		else     //802.11ac non-HT
		{
			if ( bandwidthMhz == 40)
			{
				// Calculate the average energy for the spectral lines
				for ( i=carrierFirst;i<=carrierSecond;i++)
				{
					avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
				}
				// Carrier32 is pilot, not caculated in avg_center_power
				avg_center_power = avg_center_power -data[carrierPilot] - data[lengthIQ-carrierPilot];
			
				// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
				for (i=carrierThird;i<=carrierFourth;i++)
				{
					avg_power = avg_power + data[i] + data[lengthIQ-i];
				}
				avg_power = avg_power + avg_center_power;

				//avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
				// Carrier 32 is pilot, not caculated
				avg_center_power = avg_center_power/((carrierSecond-carrierFirst)*2);
				if (0!=avg_center_power)
				{
					avg_center_power = 10.0 * log10 (avg_center_power);
				}
				else
				{
					avg_center_power= 0;
				}
				avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
				if (0!=avg_power)
				{
					avg_power = 10.0 * log10 (avg_power);
				}
				else
				{
					avg_power = 0;
				}

				for (int carrier=0;carrier<lengthIQ;carrier++)
				{
					if (0!=data[carrier])
					{
						data[carrier] = 10.0 * log10 (data[carrier]);
					}
					else
					{
						data[carrier] = 0;
					}
				}

				// Check the flatness
				double deltaPower = 0;
				double highLimit = 4, lowLimit = -4;
				//double sectionMargin[4] = {-99, -99, -99, -99};
				//int sectionCarrierNo[4];

				for (i=carrierFirst;i<=carrierSecond;i++)
				{
					if ( i != carrierPilot)
					{
						//UPPER SECTION A
						deltaPower = data[i]-avg_center_power;
						CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

						//LOWER SECTION A
						deltaPower = data[lengthIQ-i]-avg_center_power;
						CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
					}
					else
					{
						// do nothing
					}
				}

				// Continue, Check the flatness. (side)
				highLimit = 4, lowLimit = -6;
				for (i=carrierThird;i<=carrierFourth;i++)
				{
					//UPPER SECTION B
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);
					
					//LOWER SECTION B
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
				}
			}
			else if ( bandwidthMhz == 80) 
			{
				// Calculate the average energy for the spectral lines
				for ( i=carrierFirst;i<=carrierSecond;i++)   // carrier 6 ~31
				{
					avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
				}
				for ( i=carrierThird;i<=carrierFourth;i++)   // carrier 33 ~58
				{
					avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
				}
				for ( i=carrierFifth;i<=carrierSixth;i++)   // carrier 70 ~84
				{
					avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
				}
				

			
				// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
				for (i=carrierSeventh;i<=carrierEigth;i++)
				{
					avg_power = avg_power + data[i] + data[lengthIQ-i];
				}
				for (i=carrierTenth;i<=carrierNinth;i++)
				{
					avg_power = avg_power + data[i] + data[lengthIQ-i];
				}

				avg_power = avg_power + avg_center_power;

				//avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
				// Carrier 32 is pilot, not caculated
				avg_center_power = avg_center_power/ ( carrierNumSection1 *2);
				if (0!=avg_center_power)
				{
					avg_center_power = 10.0 * log10 (avg_center_power);
				}
				else
				{
					avg_center_power= 0;
				}
				avg_power = avg_power/(carrierNumSection2 *2);
				if (0!=avg_power)
				{
					avg_power = 10.0 * log10 (avg_power);
				}
				else
				{
					avg_power = 0;
				}

				for (int carrier=0;carrier<lengthIQ;carrier++)
				{
					if (0!=data[carrier])
					{
						data[carrier] = 10.0 * log10 (data[carrier]);
					}
					else
					{
						data[carrier] = 0;
					}
				}

				// Check the flatness
				double deltaPower = 0;
				double highLimit = 4, lowLimit = -4;
				

				for (i=carrierFirst;i<=carrierSecond;i++)
				{
					//UPPER SECTION A
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

					//LOWER SECTION A
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
				}

				for (i=carrierThird;i<=carrierFourth;i++)
				{
					//UPPER SECTION A
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

					//LOWER SECTION A
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
				}

				for (i=carrierSixth;i<=carrierFifth;i++)
				{
					//UPPER SECTION A
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

					//LOWER SECTION A
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
				}

				// Continue, Check the flatness. (side)
				highLimit = 4, lowLimit = -6;
				for (i=carrierSeventh;i<=carrierEigth;i++)
				{
					//UPPER SECTION B
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);
					
					//LOWER SECTION B
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
				}

				for (i=carrierTenth;i<=carrierNinth;i++)
				{
					//UPPER SECTION B
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);
					
					//LOWER SECTION B
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
				}
			}
			else if ( bandwidthMhz == 160)
			{
				// not supported yet
			}
			else	// bandwidth = 80_80
			{
				// not supported yet
			}

		}


		// get the highest fail margin
		*failMargin = -99;
		for (i=0 ; i<4 ; i++)
		{
			if (sectionMargin[i]>*failMargin)
			{
				*failMargin = sectionMargin[i];
				*leastMarginCarrier = sectionCarrierNo[i];
			}
		}

		// Get the center frequency leakage 
	//	*loLeakage = data[0] - avg_power - 10 * log10((double)((carrierFourth-carrierFirst+1)*2));

//for debug only
		if(0)
		{
		//#if defined(_DEBUG)

		double carrierIndex[MAX_BUFFER_SIZE];
		double carrierDelta[MAX_BUFFER_SIZE];

		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			
			carrierIndex[carrier] = carrier;
			carrierDelta[carrier] = data[carrier] - avg_center_power;
			
		}
		

		LP_Plot(testTimes1,carrierIndex,carrierDelta,lengthIQ,"-","Spectrum",l_txVerifyFlatnessParam.DATA_RATE,"Flatness",1);


			// print out the Mask into file for debug				
			FILE *fp;  
			char sigFileName[MAX_BUFFER_SIZE];
			sprintf_s(sigFileName, "log/Log_Flatness_Result_11N_Stream%02d_%d.csv", streamIndex+1,testTimes1);
			fopen_s(&fp, sigFileName, "w");      
			if (fp)
			{
				fprintf(fp, "WiFi Flatness Carrier,Signal,Avg_Center_Power,Size: %d\n", lengthIQ);    // print to log file 
				for (int carrier=0;carrier<lengthIQ;carrier++)
				{
					fprintf( fp, "%d,%8.2f,%8.2f\n", carrier, data[carrier],
						data[carrier]-avg_center_power);    // print to log file 
					
				}
				fclose(fp);
			}
			else
			{
				// fopen failed
			}
		}
		//#endif
	}
	else
	{
		// if (dataQ && dataI && data && maxData && minData)
		result = -1;
	}
    
    if (dataRaw)   free (dataRaw);
	if (data)	   free (data);

    return result;
}


//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
// 
//-------------------------------------------------------------------------------------
int CheckTxSpectrumParameters( int *bssBW, 
							   int *cbw,
							   int *bssPchannel, 
							   int *bssSchannel,
							   int *wifiMode,
							   int *wifiStreamNum,
							   double *samplingTimeUs,
							   int *vsaEnabled,
							   double *peakToAvgRatio,
							   char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
        // check common input parameters
          err = CheckCommonParameters_WiFi_11ac_MiMo(l_txVerifyFlatnessParamMap, 
			                                         l_txVerifyFlatnessReturnMap, 
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
		  
//		if ( 0!=strcmp(l_txVerifyFlatnessParam.PREAMBLE, "SHORT") && 0!=strcmp(l_txVerifyFlatnessParam.PREAMBLE, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_txVerifyFlatnessParam.PREAMBLE);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0!=strcmp(l_txVerifyFlatnessParam.GUARD_INTERVAL, "SHORT") && 0!=strcmp(l_txVerifyFlatnessParam.GUARD_INTERVAL, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown GUARD_INTERVAL, WiFi guard interval %s not supported.\n", l_txVerifyFlatnessParam.GUARD_INTERVAL);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if (( 0> l_txVerifyFlatnessParam.NUM_STREAM_11AC) || ( l_txVerifyFlatnessParam.NUM_STREAM_11AC >8))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] NUM_STREAM_11AC only can be 1~9. Please check input paramter \"NUM_STREAM_11AC\"!\n");
//			throw logMessage;
//		}
//		else
//		{
//			// do nothing
//		}
//		/*------------------------------------------*
//		*   Check data rate	and packet format		*
//		*-------------------------------------------*/
//
//		if ( (0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "11AC_MF_HT")) && (0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "11AC_GF_HT")) &&
//			(0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "11N_MF_HT")) && (0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "11N_GF_HT")) &&
//			(0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "11AC_VHT")) &&( 0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "11AC_NON_HT"))
//			&&( 0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "LEGACY_NON_HT")))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown PACKET_FORMAT, WiFi 11ac packet format %s not supported.\n", l_txVerifyFlatnessParam.PACKET_FORMAT);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0 == strcmp( l_txVerifyFlatnessParam.PACKET_FORMAT, "11AC_NON_HT") &&
//			! strstr ( l_txVerifyFlatnessParam.DATA_RATE, "OFDM"))
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
//		err = TM_WiFiConvertDataRateNameToIndex(l_txVerifyFlatnessParam.DATA_RATE, &dummyInt);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_txVerifyFlatnessParam.DATA_RATE);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( 0 == strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "11AC_VHT"))  // Data rate: MCS0 ~MCS9
//		{
//			if (( 14 <= dummyInt ) && (dummyInt <= 23))
//			{
//				// Data rate is right
//				//do nothing
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] PACKET_FORMAT and DATA_RATE don't match! The data rates of VHT_11AC must be MCS0 ~ MCS9, doesn't support %s!\n", &l_txVerifyFlatnessParam.DATA_RATE);
//				throw logMessage;
//
//			}
//
//		}
//		else
//		{
//		}
//
//		if ( !strstr (l_txVerifyFlatnessParam.PACKET_FORMAT, "11AC"))    // Legacy signal, CH_BANDWIDTH and CH_FREQ_MHZ must have values
//		{
//			if (( l_txVerifyFlatnessParam.CH_BANDWIDTH <= 0) || ( l_txVerifyFlatnessParam.CH_FREQ_MHZ <= 0))
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
//		if ( 0!=strcmp(l_txVerifyFlatnessParam.BSS_BANDWIDTH, "BW-20") && 0!=strcmp(l_txVerifyFlatnessParam.BSS_BANDWIDTH, "BW-40") && 
//			 0!=strcmp(l_txVerifyFlatnessParam.BSS_BANDWIDTH, "BW-80") && 0!=strcmp(l_txVerifyFlatnessParam.BSS_BANDWIDTH, "BW-160") &&
//			 0!=strcmp(l_txVerifyFlatnessParam.BSS_BANDWIDTH, "BW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_txVerifyFlatnessParam.BSS_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_txVerifyFlatnessParam.BSS_BANDWIDTH, "BW-20"))
//			{
//				*bssBW = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyFlatnessParam.BSS_BANDWIDTH, "BW-40"))
//			{
//				*bssBW = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyFlatnessParam.BSS_BANDWIDTH, "BW-80"))
//			{
//				*bssBW = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyFlatnessParam.BSS_BANDWIDTH, "BW-160"))
//			{
//				*bssBW = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyFlatnessParam.BSS_BANDWIDTH, "BW-80_80"))
//			{
//				*bssBW = BW_80_80MHZ;
//
//				if ( l_txVerifyFlatnessParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
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
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_txVerifyFlatnessParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//
//		}
//
//		if ( 0 != strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "0") && 
//			0!=strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "CBW-20") && 0!=strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "CBW-40") && 
//			 0!=strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "CBW-80") && 0!=strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "CBW-160") &&
//			 0!=strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "CBW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown CBW BANDWIDTH, WiFi 11ac CBW bandwidth %s not supported.\n", l_txVerifyFlatnessParam.CH_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "0"))
//			{
//				sprintf_s(l_txVerifyFlatnessParam.CH_BANDWIDTH,MAX_BUFFER_SIZE,"C%s",l_txVerifyFlatnessParam.BSS_BANDWIDTH);
//				*cbw = *bssBW;
//			}
//			else if ( 0 == strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "CBW-20"))
//			{
//				*cbw = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "CBW-40"))
//			{
//				*cbw = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "CBW-80"))
//			{
//				*cbw = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "CBW-160"))
//			{
//				*cbw = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, "CBW-80_80"))
//			{
//				*cbw = BW_80_80MHZ;
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_txVerifyFlatnessParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//		}
//	
//		if ( strstr ( l_txVerifyFlatnessParam.PREAMBLE,"11AC"))  //802.11ac, cbw can't be larger than bssBW
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
//		//check cbw
//		if ( strstr( l_txVerifyFlatnessParam.PACKET_FORMAT, "11N"))  //802.11n, BW= 20,40MHz
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
//		else if ( 0 == strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "LEGACY_NON_HT"))  // 11B and 11ag
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
//			if ( 0 == strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "11AC_VHT"))
//			{
//				//do nothing
//			}
//			else if ( 0 == strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "11AC_MF_HT") ||
//				0 == strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, "11AC_GF_HT"))
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
//		if (l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY <= 0)
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
//		err = TM_WiFiConvertFrequencyToChannel(l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY, bssPchannel);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( *bssBW == BW_80_80MHZ)  // Need BSS_FREQ_MHZ_SECONDARY
//		{
//			if ( l_txVerifyFlatnessParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
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
//			err = TM_WiFiConvertFrequencyToChannel(l_txVerifyFlatnessParam.BSS_FREQ_MHZ_SECONDARY, bssSchannel);      
//			if ( ERR_OK!=err )
//			{
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyFlatnessParam.BSS_FREQ_MHZ_SECONDARY);
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
//		if ( strstr (l_txVerifyFlatnessParam.PACKET_FORMAT, "11AC"))
//		{
//
//			if ( 0 == l_txVerifyFlatnessParam.CH_FREQ_MHZ)  
//			{
//				//no CH_FREQ_MHZ, no CH_FREQ_MHZ_PRIMARY_20MHz
//				// Use default values
//				if ( 0 == l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz) 
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi_11ac_MiMo] Don't have input parameters \"CH_FREQ_MHZ\" or \"CH_FREQ_MHZ_PRIMARY_20\", using BBS center freuqency as default!");
//					l_txVerifyFlatnessParam.CH_FREQ_MHZ = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY;
//
//					// all  use lower frequency for channel list
//					switch (*bssBW)
//					{
//					case BW_20MHZ:
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_40MHZ:
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 10;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_80MHZ:
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 20;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY;
//						break;
//					
//					case BW_160MHZ:			
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//				
//					case BW_80_80MHZ:				
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//
//					default:
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					}
//
//				}
//				else  //no CH_FREQ_MHZ, have CH_FREQ_MHZ_PRIMARY_20MHz. Use input CH_FREQ_MHZ_PRIMARY_20MHz to calculate
//				{
//					err = GetChannelList(*bssBW, l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//					if ( err != ERR_OK) // Wrong channel list
//					{
//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_txVerifyFlatnessParam.PACKET_FORMAT);
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
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//					else if (*cbw == BW_20MHZ)
//					{
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ = l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz;
//					}
//					else if (*cbw == BW_40MHZ)
//					{
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ = l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
//					}
//					else if (*cbw == BW_80MHZ)
//					{
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ = l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
//					}
//					else
//					{
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//				}
//				else // cbw = BW_80_80MHZ, use BSS primary and secondary
//				{
//					l_txVerifyFlatnessParam.CH_FREQ_MHZ = NA_INTEGER;
//					// do nothing
//				}
//			}
//			else  // CH_FREQ_MHZ: non-zero 
//			{
//				//Check if input CH_FREQ_MHZ is correct
//				err = CheckChannelFreq(*bssBW, *cbw,
//					l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyFlatnessParam.CH_FREQ_MHZ);
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
//				if ( 0 != l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz)
//				{
//					err = CheckChPrimary20(*bssBW, *cbw,
//							l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyFlatnessParam.CH_FREQ_MHZ,l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz);
//					if ( err == ERR_OK)  //input CH_FREQ_MHZ_PRIMARY_20MHz is correct
//					{
//						err = GetChannelList(*bssBW, l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//							&l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//						if ( err != ERR_OK) // Wrong channel list
//						{
//							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Can't get channel list. Please check input paramters.\n", l_txVerifyFlatnessParam.PACKET_FORMAT);
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
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					case BW_40MHZ:  //cbw = BW_20.40MHz
//						if (*cbw == BW_20MHZ)
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 10;
//
//						}
//						else  //wrong cbw
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80MHZ:  //cbw = BW_20,40,80MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_txVerifyFlatnessParam.CH_FREQ_MHZ == l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_txVerifyFlatnessParam.CH_FREQ_MHZ == l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  //wrong cbw
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz =  NA_INTEGER;
//
//						}
//						break;
//					case BW_160MHZ:  //cbw = BW_20,40,80,160MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_txVerifyFlatnessParam.CH_FREQ_MHZ == l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 60)
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_txVerifyFlatnessParam.CH_FREQ_MHZ == l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_txVerifyFlatnessParam.CH_FREQ_MHZ == l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else if ( l_txVerifyFlatnessParam.CH_FREQ_MHZ == l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY + 60)
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY + 50;
//							}
//							else  //wrong CH_FREQ_MHZ
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							if (l_txVerifyFlatnessParam.CH_FREQ_MHZ == l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 40)
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_txVerifyFlatnessParam.CH_FREQ_MHZ == l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY + 40)
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_160MHZ)
//						{
//							if (l_txVerifyFlatnessParam.CH_FREQ_MHZ == l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY)
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else  //wring cbw
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80_80MHZ:
//						if ( *cbw == BW_20MHZ)
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if ( l_txVerifyFlatnessParam.CH_FREQ_MHZ == l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_txVerifyFlatnessParam.CH_FREQ_MHZ == l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else if ( *cbw == BW_80_80MHZ)
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  // wrong CH_FREQ_MHz
//						{
//							l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					default:
//						l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						break;
//					}
//				}
//
//				err = GetChannelList(*bssBW, l_txVerifyFlatnessParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//				if ( err != ERR_OK)  //Get channel list wrong
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_txVerifyFlatnessParam.PACKET_FORMAT);
//					throw logMessage;
//				}
//				else  //Get channel list successfully
//				{
//				}
//			}	
//
//			//Check channel list. If all are "NA_INTEGER", return error
//			if (( l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz == NA_INTEGER ) && 
//				(l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz == NA_INTEGER ) &&
//				(l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER))
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
//			l_txVerifyFlatnessParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//			l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//			l_txVerifyFlatnessReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//		}
//#pragma endregion
//		
//
//		////VHT, cbw = BW_20MHZ, the stream number of MCS9 only can 2,4,6,8
//		//if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )
//		//{
//		//	if ( (l_txVerifyFlatnessParam.NUM_STREAM_11AC != 2) ||
//		//		(l_txVerifyFlatnessParam.NUM_STREAM_11AC != 4) ||
//		//		(l_txVerifyFlatnessParam.NUM_STREAM_11AC != 6) ||
//		//		(l_txVerifyFlatnessParam.NUM_STREAM_11AC != 8))
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
//	//VHT, cbw = BW_20MHz, the stream number of MCS9 only can 3,6 According to the standard, bguo. 7/17/2012
//		if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )
//		{
//			if ( (l_txVerifyFlatnessParam.NUM_STREAM_11AC != 3) &&
//				(l_txVerifyFlatnessParam.NUM_STREAM_11AC != 6) )
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
//			if ( l_txVerifyFlatnessParam.NUM_STREAM_11AC == 6)
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
//			if ( l_txVerifyFlatnessParam.NUM_STREAM_11AC == 3 || l_txVerifyFlatnessParam.NUM_STREAM_11AC == 7)
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
//
//		// Convert parameter
//		err = WiFi_11ac_TestMode(l_txVerifyFlatnessParam.DATA_RATE, cbw, wifiMode, wifiStreamNum, l_txVerifyFlatnessParam.PACKET_FORMAT);
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
//										l_txVerifyFlatnessParam.CH_FREQ_MHZ,
//										vsaEnabled[0],
//										vsaEnabled[1],
//										vsaEnabled[2],
//										vsaEnabled[3],
//										l_txVerifyFlatnessParam.CABLE_LOSS_DB,
//										l_txVerifyFlatnessReturn.CABLE_LOSS_DB,
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
//		if (0==l_txVerifyFlatnessParam.SAMPLING_TIME_US)
//		{
//			*samplingTimeUs = CheckSamplingTime(*wifiMode, l_txVerifyFlatnessParam.PREAMBLE, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.PACKET_FORMAT);
//		}
//		else	// SAMPLING_TIME_US != 0
//		{
//			*samplingTimeUs = l_txVerifyFlatnessParam.SAMPLING_TIME_US;
//		}

		  /*-------------------------------------*
		  *  set PAPR values for vsa setting     *
		  *--------------------------------------*/
		if ( *wifiMode==WIFI_11B )  *peakToAvgRatio = g_globalSettingParam.IQ_P_TO_A_11B_11M;    // CCK        
		else                        *peakToAvgRatio = g_globalSettingParam.IQ_P_TO_A_11AG_54M;   // OFDM 


		// Check Dut configuration changed or not
		if (  g_globalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.
			  l_txVerifyFlatnessParam.CH_FREQ_MHZ != g_RecordedParam.CH_FREQ_MHZ ||
			  0!=strcmp(l_txVerifyFlatnessParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
			  0!=strcmp(l_txVerifyFlatnessParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
			  l_txVerifyFlatnessParam.NUM_STREAM_11AC != g_RecordedParam.NUM_STREAM_11AC ||
			  0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT, g_RecordedParam.PACKET_FORMAT) ||
			  0!=strcmp(l_txVerifyFlatnessParam.GUARD_INTERVAL, g_RecordedParam.GUARD_INTERVAL_11N) ||
			  0!=strcmp(l_txVerifyFlatnessParam.CH_BANDWIDTH, g_RecordedParam.CH_BANDWIDTH) ||
			  l_txVerifyFlatnessParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_txVerifyFlatnessParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_txVerifyFlatnessParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_txVerifyFlatnessParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  l_txVerifyFlatnessParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
			  l_txVerifyFlatnessParam.TX1!=g_RecordedParam.ANT1 ||
			  l_txVerifyFlatnessParam.TX2!=g_RecordedParam.ANT2 ||
			  l_txVerifyFlatnessParam.TX3!=g_RecordedParam.ANT3 ||
			  l_txVerifyFlatnessParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;			
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup
		g_RecordedParam.ANT1					= l_txVerifyFlatnessParam.TX1;
		g_RecordedParam.ANT2					= l_txVerifyFlatnessParam.TX2;
		g_RecordedParam.ANT3					= l_txVerifyFlatnessParam.TX3;
		g_RecordedParam.ANT4					= l_txVerifyFlatnessParam.TX4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txVerifyFlatnessParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txVerifyFlatnessParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txVerifyFlatnessParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txVerifyFlatnessParam.CABLE_LOSS_DB[3];	
		g_RecordedParam.CH_FREQ_MHZ				= l_txVerifyFlatnessParam.CH_FREQ_MHZ;
	//	g_RecordedParam.CH_FREQ_MHZ_SECONDARY_80	= l_txVerifyFlatnessParam.CH_FREQ_MHZ_SECONDARY_80;
		g_RecordedParam.NUM_STREAM_11AC			= l_txVerifyFlatnessParam.NUM_STREAM_11AC;
		g_RecordedParam.POWER_DBM				= l_txVerifyFlatnessParam.TX_POWER_DBM;	

		sprintf_s(g_RecordedParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.CH_BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.PACKET_FORMAT);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.GUARD_INTERVAL);


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
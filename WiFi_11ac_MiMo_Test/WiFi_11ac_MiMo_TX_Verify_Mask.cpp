#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11ac_MiMo_Test.h"
#include "WiFi_11ac_MiMo_Test_Internal.h"
#include "IQmeasure.h"
#include "math.h" 
#include "vDUT.h"

using namespace std;

// This global variable is declared in WiFi_Test_Internal.cpp
extern TM_ID                 g_WiFi_Test_ID;
extern vDUT_ID               g_WiFi_Dut;
extern bool					 g_vDutTxActived;
extern bool					 g_dutConfigChanged;
extern WIFI_RECORD_PARAM     g_RecordedParam;
extern int					 g_Tester_Number;
extern bool					 g_Tester_EnableVHT80;

int testTimes =1;

// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_globalSettingParam;

typedef struct tagDefaultMask
{ 
    double  freqOffset;
    double  deltaMaskPwr;
} DEFAULT_MASK_STRUCT;

vector<DEFAULT_MASK_STRUCT> l_wifiMask_11b;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11ag;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11nHt20;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11nHt40;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11acBss20;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11acBss40;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11acBss80;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11acBss160;
vector<DEFAULT_MASK_STRUCT> l_wifiMask_11acBss80_80;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyMaskParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyMaskReturnMap;

struct tagParam
{
    // Mandatory Parameters
    int    BSS_FREQ_MHZ_PRIMARY;                            /*!< For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment */
	int    BSS_FREQ_MHZ_SECONDARY;                          /*!< For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined */
	int    CH_FREQ_MHZ_PRIMARY_20MHz;                       /*!< The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ"   */
	int    CH_FREQ_MHZ;										/*!< The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used  */
	int    NUM_STREAM_11AC;							        /*!< The number of streams 11AC (Only). */
	char   BSS_BANDWIDTH[MAX_BUFFER_SIZE];                  /*!< The RF bandwidth of basic service set (BSS) to verify EVM. */
	char   CH_BANDWIDTH[MAX_BUFFER_SIZE];                  /*!< The RF channel bandwidth to verify mask. */
	//char   GUARD_INTERVAL[MAX_BUFFER_SIZE];					/*!< The guard interval for 11N and 11AC. */

    char   DATA_RATE[MAX_BUFFER_SIZE];						/*! The data rate to verify Mask. */
	char   PREAMBLE[MAX_BUFFER_SIZE];						/*! The preamble type of 11B(only). */
	char   PACKET_FORMAT[MAX_BUFFER_SIZE];					/*! The packet format of 11N and 11AC.*/
	char   GUARD_INTERVAL[MAX_BUFFER_SIZE];					/*! The guard interval for 11N and 11AC. */
    char   STANDARD[MAX_BUFFER_SIZE];				        /*!< The standard parameter used for signal analysis option or to discriminate the same data rates/package formats from different standards */
   
    double TX_POWER_DBM;									/*! The output power to verify Mask. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];					/*! The path loss of test system. */
    double SAMPLING_TIME_US;								/*! The sampling time to verify Mask. */
	double OBW_PERCENTAGE;									/*! The percentage of OBW, default = 99% */

    char   VSA_CONNECTION[MAX_TESTER_NUM][MAX_BUFFER_SIZE]; /*!< The vsa connect to which antenn port. */ 
    char   VSA_PORT[MAX_TESTER_NUM][MAX_BUFFER_SIZE];		/*!< IQTester VSAs port setting. Default=PORT_LEFT. */  

    // DUT Parameters
    int    TX1;												/*!< DUT TX1 on/off. Default=1(on).  */
    int    TX2;												/*!< DUT TX2 on/off. Default=0(off). */
    int    TX3;												/*!< DUT TX3 on/off. Default=0(off). */
    int    TX4;												/*!< DUT TX4 on/off. Default=0(off). */

	int    ARRAY_HANDLING_METHOD;                   /*! The flag to handle array result. Default: 0; 0: Do nothing, 1: Display result on screen, 2: Display result and log to file */

	char   MASK_TEMPLATE[MAX_BUFFER_SIZE];                  /*! Mask template */
} l_txVerifyMaskParam;

struct tagReturn
{
    // Mask Test Result 
    double MARGIN_DB_POSITIVE[MAX_POSITIVE_SECTION];		/*!< Margin to the mask, normally should be a negative number, if pass. */
	double MARGIN_DB_NEGATIVE[MAX_NEGATIVE_SECTION];		/*!< Margin to the mask, normally should be a negative number, if pass. */
	double FREQ_AT_MARGIN_POSITIVE[MAX_POSITIVE_SECTION];	/*!< Point out the frequency at margin to the mask. */
	double FREQ_AT_MARGIN_NEGATIVE[MAX_NEGATIVE_SECTION];	/*!< Point out the frequency at margin to the mask. */
	double VIOLATION_PERCENT;								/*!< Percentage which fail the mask. */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];					/*!< The path loss of test system. */
	vector <double> *SPECTRUM_RAW_DATA_X_VSA[MAX_TESTER_NUM];	/*!< spectrum X raw data in each VSA.  */
	vector <double> *SPECTRUM_RAW_DATA_Y_VSA[MAX_TESTER_NUM];	/*!< spectrum Y raw data in each VSA.  */

	// OBW Test Result
	double OBW_FREQ_START_MHZ;                              /*!< The start frequency in OBW test item. */
	double OBW_FREQ_STOP_MHZ;                               /*!< The stop frequency in OBW test item. */
	double OBW_MHZ;                                         /*!< The occupid bandwidth. */

    //Supported for Multi-tester
    double MARGIN_DB_POSITIVE_VSA[MAX_TESTER_NUM][MAX_POSITIVE_SECTION];		/*!< Margin to the mask for specified chain, normally should be a negative number, if pass. */
	double MARGIN_DB_NEGATIVE_VSA[MAX_TESTER_NUM][MAX_NEGATIVE_SECTION];		/*!< Margin to the mask for specified chain, normally should be a negative number, if pass. */
	double FREQ_AT_MARGIN_POSITIVE_VSA[MAX_TESTER_NUM][MAX_POSITIVE_SECTION];   /*!< Point out the frequency for specified chain at margin to the mask. */
	double FREQ_AT_MARGIN_NEGATIVE_VSA[MAX_TESTER_NUM][MAX_NEGATIVE_SECTION];   /*!< Point out the frequency for specified chain at margin to the mask. */
	double VIOLATION_PERCENT_VSA[MAX_TESTER_NUM];							    /*!< Percentage which fail the mask for specified chain. */

    // OBW Test Result
	double OBW_FREQ_START_MHZ_VSA[MAX_TESTER_NUM];                              /*!< The start frequency in OBW test item for specified chain. */ 
	double OBW_FREQ_STOP_MHZ_VSA[MAX_TESTER_NUM];                               /*!< The stop frequency in OBW test item for specified chain. */ 
	double OBW_MHZ_VSA[MAX_TESTER_NUM];                                         /*!< The occupid bandwidth for specified chain. */
    
    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];


	//channel list
	int    CH_FREQ_MHZ_PRIMARY_40MHz;       /*!< The center frequency (MHz) for primary 40 MHZ channel  */
	int    CH_FREQ_MHZ_PRIMARY_80MHz;       /*!< The center frequency (MHz) for primary 80 MHZ channel  */


} l_txVerifyMaskReturn;
#pragma endregion

#ifndef WIN32 
int initTXVerifyMaskMIMOContainers = InitializeTXVerifyMaskContainers();
#endif
// These global variables/functions only for WiFi_TX_Verify_Mask.cpp
int CheckTxMaskParameters( int *bssBW, 
						   int *cbw,
						   int *bssPchannel, 
						   int *bssSchannel,
						   int *wifiMode,
						   int *wifiStreamNum,
						   double *samplingTimeUs,
						   int *vsaEnabled,
						   double *peakToAvgRatio,
						   char* errorMsg );


//! WiFi TX Verify Mask
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


WIFI_11AC_MIMO_TEST_API int WiFi_TX_Verify_Mask(void)
{
    int				err = ERR_OK;

    bool			analysisOK = false, captureOK  = false; 
    int				channel = 0, VHTMode = 0;
    int				dummyValue = 0;
    int				wifiMode = 0, wifiStreamNum = 0;
	double			samplingTimeUs = 0, peakToAvgRatio = 0;	//, cableLossDb = 0;
	char			vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char			logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char			sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};


	int				txEnabled[MAX_CHAIN_NUM]={0};
	int				vsaMappingTx[MAX_CHAIN_NUM]={0};
	int				vsaPortStatus[MAX_TESTER_NUM];

	vector<double>	rfAmplDb(MAX_TESTER_NUM, NA_NUMBER);
	vector<int>		vsgRFEnabled(MAX_TESTER_NUM, (int)NA_NUMBER);

	//11ac
	int		bssPchannel = 0,bssSchannel = 0;
	int		VSAanylisisFormat = 0;
	int		bssBW = 0, cbw = 0;
	int		VSAcaptureMode;  //0: 20MHz		1: 40MHz	2:80MHz		3: 160MHz		4: 80_80MHz

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/

	

	ClearReturnParameters(l_txVerifyMaskReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
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
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_txVerifyMaskReturnMap);
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
		err = GetInputParameters(l_txVerifyMaskParamMap);
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

        //Map antenna port to Vsa number
        err = CheckConnectionVsaAndAntennaPort(l_txVerifyMaskParamMap, vsaMappingTx, vsaPortStatus);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi_11ac_MiMo] Mapping VSA to antenna port failed\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Mapping VSA to antenna port OK.\n");
		}

        txEnabled[0] = l_txVerifyMaskParam.TX1;
        txEnabled[1] = l_txVerifyMaskParam.TX2;
        txEnabled[2] = l_txVerifyMaskParam.TX3;
        txEnabled[3] = l_txVerifyMaskParam.TX4;

#pragma region Prepare input parameters

		//Our tester doesn't support BW_80_80MHZ
		if ( 0 ==strcmp(l_txVerifyMaskParam.BSS_BANDWIDTH, "BW-80_80") || 0 ==strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "CBW-80_80") )
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
		//By order pass into "CheckTxMaskParameters()
		int vsaEnabled[MAX_TESTER_NUM];
		for(int i=0;i<MAX_TESTER_NUM;i++)
		{
           vsaEnabled[i] = txEnabled[vsaMappingTx[i]-1];

		}


		err = CheckTxMaskParameters(&bssBW,&cbw, &bssPchannel,&bssSchannel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &vsaEnabled[0], &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Prepare input parameters CheckTxMaskParameters() return OK.\n");
		}
#pragma endregion

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

		vDUT_AddIntegerParameter(g_WiFi_Dut, "BSS_FREQ_MHZ_PRIMARY",		l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "BSS_FREQ_MHZ_SECONDARY",		l_txVerifyMaskParam.BSS_FREQ_MHZ_SECONDARY);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ",					l_txVerifyMaskParam.CH_FREQ_MHZ);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_20MHz",	l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "NUM_STREAM_11AC",				l_txVerifyMaskParam.NUM_STREAM_11AC);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",							l_txVerifyMaskParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",							l_txVerifyMaskParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",							l_txVerifyMaskParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",							l_txVerifyMaskParam.TX4);
	
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA1",			l_txVerifyMaskParam.CABLE_LOSS_DB[0]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA2",			l_txVerifyMaskParam.CABLE_LOSS_DB[1]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA3",			l_txVerifyMaskParam.CABLE_LOSS_DB[2]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSA4",			l_txVerifyMaskParam.CABLE_LOSS_DB[3]);
        vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",			samplingTimeUs);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",				l_txVerifyMaskParam.TX_POWER_DBM);

		vDUT_AddStringParameter (g_WiFi_Dut, "BSS_BANDWIDTH",				l_txVerifyMaskParam.BSS_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "CH_BANDWIDTH",				l_txVerifyMaskParam.CH_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",					l_txVerifyMaskParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT",				l_txVerifyMaskParam.PACKET_FORMAT);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL",				l_txVerifyMaskParam.GUARD_INTERVAL);
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",					l_txVerifyMaskParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "STANDARD",					l_txVerifyMaskParam.STANDARD);
		// the following parameters are not input parameters, why are they here?  7-16-2012 Jacky 
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_40MHz",	l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz);
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_80MHz",	l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
		if ((wifiMode== WIFI_11N_GF_HT40) || ( wifiMode== WIFI_11N_MF_HT40 ) || (wifiMode == WIFI_11AC_VHT40))
		{
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyMaskParam.CH_FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyMaskParam.CH_FREQ_MHZ+10);
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
		*   1: 40MHz			*
		*	2: 80MHz			*
		*	3: 160MHz			*
		*	4: 80_80MHz			*
		*-----------------------*/
		if ( g_Tester_EnableVHT80)   // 802.11ac ProtoType
		{
			if ( !g_globalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW) // use BSS BW for spec mask
			{
				if ( bssBW == BW_80MHZ)					VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
				else if ( bssBW == BW_160MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //temporary 
				else if ( bssBW == BW_80_80MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 2; //temporary 
				else									VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;	
			}
			else		// use cbw for spec mask
			{
				if ( cbw == BW_80MHZ)					VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
				else if ( cbw == BW_160MHZ)				VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //temporary 
				else if ( cbw == BW_80_80MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 2; //temporary 
				else									VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;	
			}
		}
		else  // other testers
		{
			//if ( strstr (l_txVerifyMaskParam.PACKET_FORMAT,"11AC")) //not sure how other tester support 11ac
			if ( strstr (l_txVerifyMaskParam.PACKET_FORMAT, PACKET_FORMAT_VHT)) //not sure how other tester support 11ac
			{
				if ( !g_globalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW) // use BSS BW for spec mask
				{
					if ( bssBW == BW_80_80MHZ)				VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 2; //temporary 
					else if ( bssBW == BW_40MHZ)			VSAcaptureMode = IQV_HT_40_CAPTURE_TYPE;
					else if ( bssBW == BW_80MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE; // for  hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisVHT80);
					else if ( bssBW == BW_160MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //temporary 
					else									VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;			
				}
				else
				{
					if ( cbw == BW_80_80MHZ)				VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //temporary 
					else if ( cbw == BW_40MHZ)				VSAcaptureMode = IQV_HT_40_CAPTURE_TYPE;
					else if ( cbw == BW_80MHZ)				VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
					else if ( cbw == BW_160MHZ)				VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //temporary 
					else									VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;	
				}
			}
			else
			{
				if (cbw == BW_40MHZ)					VSAcaptureMode = IQV_HT_40_CAPTURE_TYPE;
				else									VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;
			}
		}



		/*------------------------------------------*
		*	VSA analysis packet format				*					
		*	0: auto detect							*
		*	1: mixed format (11n)					*
		*	2: greenfield format (11n)				*
		*	3: legacy fomat ( 11a/g,11ac Non-HT)	*
		*	4: VHT format ( 11ac, VHT only)			*
		*-------------------------------------------*/

		if ( strstr ( l_txVerifyMaskParam.PACKET_FORMAT, PACKET_FORMAT_HT_MF))	// mixed format, 11n
		{
			VSAanylisisFormat = 1;
		}
		else if ( strstr ( l_txVerifyMaskParam.PACKET_FORMAT, PACKET_FORMAT_HT_GF))	// greeffield format, 11n
		{
			VSAanylisisFormat = 2;
		}
		else if ( strstr ( l_txVerifyMaskParam.PACKET_FORMAT, PACKET_FORMAT_VHT))	// 11ac, VHT only
		{
			VSAanylisisFormat = 4;
		}
		else if ( strstr ( l_txVerifyMaskParam.PACKET_FORMAT, PACKET_FORMAT_NON_HT))	// 11a/g,11ac Non-HT
		{
			VSAanylisisFormat = 3;
		}
		else
		{
			VSAanylisisFormat = 0;
		}

		//-----------------------------------------------------
		// check mask template if specified
		//-----------------------------------------------------

		if (strlen(l_txVerifyMaskParam.MASK_TEMPLATE)>0)
		{
			char *ptr, buffer[MAX_BUFFER_SIZE];
			vector<DEFAULT_MASK_STRUCT> *maskTemplate;
			DEFAULT_MASK_STRUCT inputMask;
			int dataRate;

			TM_WiFiConvertDataRateNameToIndex(l_txVerifyMaskParam.DATA_RATE, &dataRate);      

			strcpy_s(buffer, l_txVerifyMaskParam.MASK_TEMPLATE);

			// check data rate to use the right mask template
			if (wifiMode == WIFI_11B)
			{
				maskTemplate = &l_wifiMask_11b;
			}
			else if (wifiMode == WIFI_11AG)
			{
				maskTemplate = &l_wifiMask_11ag;
			}
			else if (( wifiMode == WIFI_11N_MF_HT20 ) ||
				(wifiMode == WIFI_11N_GF_HT20))    //11n_HT20
			{
				maskTemplate = &l_wifiMask_11nHt20;
			}
			else if (( wifiMode == WIFI_11N_MF_HT40 ) ||
				(wifiMode == WIFI_11N_GF_HT40))          //11n_HT40
			{
				maskTemplate = &l_wifiMask_11nHt40;
			}
			else //11ac
			{
				if ( bssBW == BW_20MHZ)
				{
					maskTemplate = &l_wifiMask_11acBss20;
				}
				else if ( bssBW == BW_40MHZ)
				{
					maskTemplate = &l_wifiMask_11acBss40;
				}
				else if ( bssBW == BW_80MHZ)
				{
					maskTemplate = &l_wifiMask_11acBss80;
				}
				else if ( bssBW == BW_160MHZ)
				{
					maskTemplate = &l_wifiMask_11acBss160;
				}
				else 
				{
					maskTemplate = &l_wifiMask_11acBss80_80;
				}
			}

			// initialize mask template
			maskTemplate->clear();
			inputMask.deltaMaskPwr = 0;
			inputMask.freqOffset = 0;
			maskTemplate->push_back(inputMask);

			// add point to the template
			ptr = strtok(buffer, " \t");
			while(ptr)
			{
				inputMask.freqOffset = atof(ptr);
				ptr = strtok(NULL, " \t");
				if (ptr)
				{
					inputMask.deltaMaskPwr = atof(ptr);
					ptr = strtok(NULL, " \t");
					maskTemplate->push_back(inputMask);
				}
			}
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

        //TODO: put tx enabled condition into vector txEnabled
       

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

		/*err = ::LP_SetVsa(  l_txVerifyMaskParam.FREQ_MHZ*1e6,
							l_txVerifyMaskParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio,
							g_globalSettingParam.VSA_PORT,
							0,
							g_globalSettingParam.VSA_TRIGGER_LEVEL_DB,
							g_globalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000	
						  );*/

		for(int i = 0; i<g_Tester_Number;i++)
		{
			
			rfAmplDb[i] = l_txVerifyMaskParam.TX_POWER_DBM-l_txVerifyMaskParam.CABLE_LOSS_DB[i]+peakToAvgRatio;
			
			vsgRFEnabled[i] = 0; // Disable VSG

		}

		//if ( strstr(l_txVerifyMaskParam.PACKET_FORMAT, "11AC") && (!g_globalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW))  //802.11ac, use BSS frequency to capture
		if ( strstr(l_txVerifyMaskParam.PACKET_FORMAT, PACKET_FORMAT_VHT) && (!g_globalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW))  //802.11ac, use BSS frequency to capture
		{
			err = ::LP_SetVsaNxN(  l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY*1e6,
									&rfAmplDb[0],
									&vsaPortStatus[0],
									0,
									g_globalSettingParam.VSA_TRIGGER_LEVEL_DB,
									g_globalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000	
								  );
		}
		else   //non-AC, use channel frequency to capture
		{
			err = ::LP_SetVsaNxN(  l_txVerifyMaskParam.CH_FREQ_MHZ*1e6,
									&rfAmplDb[0],
									&vsaPortStatus[0],
									0,
									g_globalSettingParam.VSA_TRIGGER_LEVEL_DB,
									g_globalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000	
								  );
		}

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
 	    //VSAcaptureMode=0;
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
		if (1==g_globalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
		{
			// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_SaveAlways", l_txVerifyMaskParam.CH_FREQ_MHZ, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BSS_BANDWIDTH);
			WiFiSaveSigFile(sigFileNameBuffer);
		}
		else
		{
			// do nothing
		}

		double *bufferRealX = NULL, *bufferImagX = NULL, *bufferRealY=NULL, *bufferImagY=NULL; 
		bufferRealX = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));
		bufferImagX = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));
		bufferRealY = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));
		bufferImagY = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));

		//double bufferRealX[MAX_BUFFER_SIZE*4], bufferImagX[MAX_BUFFER_SIZE*4]; 
		//double bufferRealY[MAX_BUFFER_SIZE*4], bufferImagY[MAX_BUFFER_SIZE*4];
		int    bufferSizeX;
		int    bufferSizeY;
        int    startIndex, stopIndex, pass;
        //supported multi-tester
        int    firstValidSignal = 0; // for backward compatible
        int    vsaAnalysisIndex = 0;
        

		// Current module only support tester 802.11ac prototype
		while(vsaAnalysisIndex < MAX_TESTER_NUM)
		{
			//if (strstr(l_txVerifyMaskParam.PACKET_FORMAT,"11AC") ) // 11AC
			if (strstr(l_txVerifyMaskParam.PACKET_FORMAT, PACKET_FORMAT_VHT) ) // 11AC
			{
				if ( !g_globalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW )  // use BSS BW for spec mask
				{
					
					if (( bssBW== BW_40MHZ) && ( ! g_Tester_EnableVHT80))
					{
						//AnalyzeHT40
						err = ::LP_SetAnalysisParameterInteger("AnalyzeHT40", "vsaNum", vsaAnalysisIndex+1);
					}
					// VHT8160,VHT80_80 are not supported yet
		
					else if ( bssBW == BW_80MHZ)
					{
						//AnalyzeVHT80
						err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT80Mask", "vsaNum", vsaAnalysisIndex+1);
					}
					else if (bssBW == BW_160MHZ)
					{
						//AnalyzeVHT160
						err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT160Mask", "vsaNum", vsaAnalysisIndex+1);
					}
					else if ( bssBW == BW_80_80MHZ)
					{
						//AnalyzeVHT80_80
						err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT80_80Mask", "vsaNum", vsaAnalysisIndex+1);
					}
					else   //bssBW = 20,40MHz
					{
						err = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", vsaAnalysisIndex+1);
						
					}
				}
				else		// use CBW for spec mask
				{
					if (( cbw== BW_40MHZ) && ( ! g_Tester_EnableVHT80))
					{
						//AnalyzeHT40
						err = ::LP_SetAnalysisParameterInteger("AnalyzeHT40", "vsaNum", vsaAnalysisIndex+1);
					}
					// VHT8160,VHT80_80 are not supported yet
		
					else if ( cbw == BW_80MHZ)
					{
						//AnalyzeVHT80
						err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT80Mask", "vsaNum", vsaAnalysisIndex+1);
					}
					else if (cbw == BW_160MHZ)
					{
						//AnalyzeVHT160
						err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT160Mask", "vsaNum", vsaAnalysisIndex+1);
					}
					else if ( cbw == BW_80_80MHZ)
					{
						//AnalyzeVHT80_80
						err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT80_80Mask", "vsaNum", vsaAnalysisIndex+1);
					}
					else   //bssBW = 20,40MHz
					{
						err = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", vsaAnalysisIndex+1);
						
					}
				}
			}
			else   // 802.11 ab/b/g/n.  mask according to cbw
			{
				if ((cbw == BW_40MHZ)&& ( ! g_Tester_EnableVHT80))
				{
					//AnalyzeHT40
					err = ::LP_SetAnalysisParameterInteger("AnalyzeHT40", "vsaNum", vsaAnalysisIndex+1);
				}
				else
				{
					err = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", vsaAnalysisIndex+1);
				}
			}

			if ( ERR_OK!=err )
			{
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WiFi_Tx_Mask_Set_Analysis_Param_Failed", l_txVerifyMaskParam.CH_FREQ_MHZ);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Set Analyze FFT parameter return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Set Analyze FFT parameter return OK.\n");
			}
			//Step2: Perform Analysis   --802.11 ac Prototype

			//if (strstr(l_txVerifyMaskParam.PACKET_FORMAT,"11AC")) // 11AC, mask according to BSS BW
			if (strstr(l_txVerifyMaskParam.PACKET_FORMAT, PACKET_FORMAT_VHT) ) // 11AC  
			{
				if ( !g_globalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW )  // use BSS BW for spec mask
				{

					if ( bssBW == BW_80MHZ)
					{
						//AnalyzeVHT80
						err = ::LP_AnalyzeVHT80Mask();
					}
					/*else if (bssBW == BW_160MHZ)
					{
						//AnalyzeVHT160
						err = ::LP_AnalyzeVHT160Mask();
					}
					else if ( bssBW== BW_80_80MHZ)
					{
						//AnalyzeVHT80
						err = ::LP_AnalyzeVHT80_80Mask();
					}*/
				
					else  // BW20, BW40
					{
						err = ::LP_AnalyzeFFT();	
					}
				}
				else   
				{
					if ( cbw == BW_80MHZ)
					{
						//AnalyzeVHT80
						err = ::LP_AnalyzeVHT80Mask();
					}
					/*else if (cbw == BW_160MHZ)
					{
						//AnalyzeVHT160
						err = ::LP_AnalyzeVHT160Mask();
					}
					else if ( cbw == BW_80_80MHZ)
					{
						//AnalyzeVHT80
						err = ::LP_AnalyzeVHT80_80Mask();
					}*/
				
					else  // BW20
					{
						err = ::LP_AnalyzeFFT();	
					}
				}
			}
			else		// 802.11 ab/b/g/n.  mask according to cbw
			{
				if ((cbw == BW_40MHZ) && ( ! g_Tester_EnableVHT80))
				{
					//AnalyzeHT40
					err = ::LP_AnalyzeHT40Mask();
				}
				else
				{
					err = ::LP_AnalyzeFFT();	
				}
			}
		
			//-------------------------------------------------------------------------
			// if Tx port is disable and analysis fail, then just fill NA values
			//-------------------------------------------------------------------------
			if (!txEnabled[vsaMappingTx[vsaAnalysisIndex]-1] && err!=ERR_OK)
			{
				for (int x=0;x<MAX_POSITIVE_SECTION;x++)
				{
					l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[x]		= NA_DOUBLE;
					l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[x]	= NA_DOUBLE;
					l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[x]		= NA_DOUBLE;
					l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[x] = NA_DOUBLE;
				}
				err = ERR_OK;
			}
			else
			{
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.BSS_BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Analyze FFT return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Analyze FFT  return OK.\n");
				}

				//Step3: Retrieve analysis results
				analysisOK = true;
				bufferSizeX = ::LP_GetVectorMeasurement("x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE*8);
				bufferSizeY = ::LP_GetVectorMeasurement("y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE*8);
				//bufferSizeX = ::LP_GetVectorMeasurement("x", bufferRealX, bufferRealX, MAX_BUFFER_SIZE*8); //temporay work around
				//bufferSizeY = ::LP_GetVectorMeasurement("y", bufferRealY, bufferRealY, MAX_BUFFER_SIZE*8);

				
				
				//************************ HT40 HACK START ****************************
				//May not needed for 802.11ac prototype; Tracy Yu; 2011/12/13
				/*
				if ((cbw == BW_40MHZ) || (bssBW == BW_40MHZ && strstr (l_txVerifyMaskParam.PACKET_FORMAT,"11AC")))
				{
					

					if(l_txVerifyMaskParam.CH_FREQ_MHZ != 2462)
					{
						// +60MHz offset, right most index of the "spike band" is bufferSizeX-1
						EliminateSpike(bufferRealY, 4, bufferSizeX-1);
					}
					else
					{
						// +55MHz offset, right most index of the "spike band" is...
						// 120MHz span, 5MHz from the right... bufferSizeX * 115 / 120 + half of the bandWidth
						EliminateSpike(bufferRealY, 8, bufferSizeX * 115 / 120 + 4);
					}

					// -40MHz offset, right most index of the "spike band" is...
					// 120MHz span, 20MHz from the left... bufferSizeX * 20 / 120 + half of the bandWidth
					EliminateSpike(bufferRealY, 8, bufferSizeX * 20 / 120 + 4);


					//::LP_Plot(l_txVerifyMaskParam.FREQ_MHZ+1, bufferRealX, bufferRealY, bufferSizeX, ".-", "After", "Freq", "Power", 1);
				}*/
				//************************ HT40 HACK END ****************************

				if ( (0!=bufferSizeX)&&(0!=bufferSizeY) )
				{
					//TODO: Mask Raw Data for specified chain, remain original part for backward compatible
					//Return Mask Raw Data
					l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[vsaAnalysisIndex]->clear();
					l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[vsaAnalysisIndex]->clear();

					for(int i=0;i<bufferSizeX;i++)
					{
						/*l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X->push_back(bufferRealX[i]); */
						l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[vsaAnalysisIndex]->push_back(bufferRealX[i]); 

					}
					for(int i=0;i<bufferSizeY;i++)
					{
						/*l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y->push_back(bufferRealY[i]);  */
						l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[vsaAnalysisIndex]->push_back(bufferRealY[i]);  

					}

			

					// TODO: Verify MASK for specified chain
					pass = VerifyPowerMask( vsaAnalysisIndex, bufferRealX, bufferRealY, bufferSizeY, wifiMode ,bssBW, cbw);

					// Verify OBW for specified chain
					startIndex = 0;
					stopIndex  = 0;

					pass = VerifyOBW(bufferRealY, bufferSizeY, (l_txVerifyMaskParam.OBW_PERCENTAGE/100), startIndex, stopIndex);

					l_txVerifyMaskReturn.OBW_FREQ_START_MHZ_VSA[vsaAnalysisIndex] = bufferRealX[startIndex] / 1000000;
					l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ_VSA[vsaAnalysisIndex]  = bufferRealX[stopIndex]  / 1000000;
					l_txVerifyMaskReturn.OBW_MHZ_VSA[vsaAnalysisIndex]  = l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ_VSA[vsaAnalysisIndex] - l_txVerifyMaskReturn.OBW_FREQ_START_MHZ_VSA[vsaAnalysisIndex];

					//for backward compatible
					if(txEnabled[vsaAnalysisIndex] && !firstValidSignal)
					{
						firstValidSignal = 1;
						//for MASK
						for(int i=0;i<MAX_POSITIVE_SECTION;i++)
						{
							l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[i] = l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[vsaAnalysisIndex][i];
							l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[vsaAnalysisIndex][i];
						}

						for(int i=0;i<MAX_NEGATIVE_SECTION;i++)
						{
							l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[i] = l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[vsaAnalysisIndex][i];
							l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[vsaAnalysisIndex][i];
						}

						l_txVerifyMaskReturn.VIOLATION_PERCENT = l_txVerifyMaskReturn.VIOLATION_PERCENT_VSA[vsaAnalysisIndex];

						//for OBW

						l_txVerifyMaskReturn.OBW_FREQ_START_MHZ = l_txVerifyMaskReturn.OBW_FREQ_START_MHZ_VSA[vsaAnalysisIndex];
						l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ  = l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ_VSA[vsaAnalysisIndex];
						l_txVerifyMaskReturn.OBW_MHZ = l_txVerifyMaskReturn.OBW_MHZ_VSA[vsaAnalysisIndex];
					}
					else
					{
						//do nothing
					}


				}
				else
				{
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Can not retrieve FFT result, result length = 0.\n");
					throw logMessage;
				}
			}
			vsaAnalysisIndex ++;
		}
		if (bufferRealX) free(bufferRealX);
		if (bufferImagX) free(bufferImagX);
		if (bufferRealY) free(bufferRealY);
		if (bufferImagY) free(bufferImagY);


		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/    
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			sprintf_s(l_txVerifyMaskReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyMaskReturnMap);
		}
		else
		{
			// do nothing
		}


		testTimes++;
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txVerifyMaskReturn.ERROR_MESSAGE, msg);

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
		ReturnErrorMessage(l_txVerifyMaskReturn.ERROR_MESSAGE, "[WiFi_11ac_MiMo] Unknown Error!\n");
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

	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i]->clear();
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->clear();
	}

    return err;
}

//************************ HT40 HACK START ****************************
void EliminateSpike(double bufferRealY[], int spikeWidth, int spikeIndex)
{
	//int spikeWidth = 5; 
	double spikeAve = 0.0;
	double adjacentAve = 0.0;
				
	// Check power in +60MHz area, which is end of the array. 
	//int spikeIndex = bufferSizeX - 1;
	for(int i=0; i<spikeWidth; i++)
	{
		spikeAve += bufferRealY[spikeIndex - i];
	}
	spikeAve /= spikeWidth;

	// Check power in adjacent area
	int adjacentIndex = spikeIndex - spikeWidth;
	for(int i=0; i<spikeWidth; i++)
	{
		adjacentAve += bufferRealY[adjacentIndex - i];
	}
	adjacentAve /= spikeWidth;

	// if difference is too big, copy adjacent area to the spike area.
	double difference = spikeAve - adjacentAve;
	if(difference > 3) //dB
	{
		for(int i=0; i<spikeWidth; i++)
		{
			bufferRealY[spikeIndex -i] = bufferRealY[adjacentIndex - i];
		}
	}
}
//************************ HT40 HACK END ****************************

int InitializeTXVerifyMaskContainers(void)
{
    DEFAULT_MASK_STRUCT dummyStruct;

	//CleanupTXVerifyMaskContainers();
	//l_wifiMask_11b.clear(); 
	//l_wifiMask_11ag.clear();
	//l_wifiMask_11nHt20.clear();
	//l_wifiMask_11nHt40.clear();
	//l_wifiMask_11acBss20.clear();
	//l_wifiMask_11acBss40.clear();
	//l_wifiMask_11acBss80.clear();
	//l_wifiMask_11acBss80_80.clear();
	//l_wifiMask_11acBss160.clear();

    /*----------------------*
     *  Construct the Mask  *
     *----------------------*/
    // Default Mask of 802.11b 
    dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11b.push_back(dummyStruct);
    dummyStruct.freqOffset   = 11;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11b.push_back(dummyStruct);
    dummyStruct.freqOffset   = 11;
    dummyStruct.deltaMaskPwr = -30;
    l_wifiMask_11b.push_back(dummyStruct);
    dummyStruct.freqOffset   = 22;
    dummyStruct.deltaMaskPwr = -30;
    l_wifiMask_11b.push_back(dummyStruct);
    dummyStruct.freqOffset   = 22;
    dummyStruct.deltaMaskPwr = -50;
    l_wifiMask_11b.push_back(dummyStruct);

    // Default Mask of 802.11ag 
    dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11ag.push_back(dummyStruct);
    dummyStruct.freqOffset   = 9;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11ag.push_back(dummyStruct);
    dummyStruct.freqOffset   = 11;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11ag.push_back(dummyStruct);
    dummyStruct.freqOffset   = 20;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11ag.push_back(dummyStruct);
    dummyStruct.freqOffset   = 30;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11ag.push_back(dummyStruct);

    // Default Mask of 802.11n HT20
    dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11nHt20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 9;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11nHt20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 11;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11nHt20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 20;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11nHt20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 30;
    dummyStruct.deltaMaskPwr = -45;
    l_wifiMask_11nHt20.push_back(dummyStruct);

    // Default Mask of 802.11n HT40
    dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11nHt40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 19;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11nHt40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 21;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11nHt40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 40;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11nHt40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 60;
    dummyStruct.deltaMaskPwr = -45;
    l_wifiMask_11nHt40.push_back(dummyStruct);

	//Default Mask of 802.11ac BSS20MHz
	dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss20.push_back(dummyStruct);
	dummyStruct.freqOffset   = 9;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 11;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11acBss20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 20;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11acBss20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 30;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11acBss20.push_back(dummyStruct);

	//Default Mask of 802.11ac BSS40MHz
	dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss40.push_back(dummyStruct);
	dummyStruct.freqOffset   = 19;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 21;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11acBss40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 40;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11acBss40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 60;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11acBss40.push_back(dummyStruct);

	//Default Mask of 802.11ac BSS80MHz
	dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss80.push_back(dummyStruct);
	dummyStruct.freqOffset   = 39;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 41;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11acBss80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 80;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11acBss80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 120;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11acBss80.push_back(dummyStruct);


	//Default Mask of 802.11ac BSS160MHz
	dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss160.push_back(dummyStruct);
	dummyStruct.freqOffset   = 79;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss160.push_back(dummyStruct);
    dummyStruct.freqOffset   = 81;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11acBss160.push_back(dummyStruct);
    dummyStruct.freqOffset   = 160;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11acBss160.push_back(dummyStruct);
    dummyStruct.freqOffset   = 240;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11acBss160.push_back(dummyStruct);

	//Default Mask of 802.11ac BSS80_80MHz. Not final, need to discuss about it later.
	dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = -25;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
	dummyStruct.freqOffset   = 39;
    dummyStruct.deltaMaskPwr = 20;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 41;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
	dummyStruct.freqOffset   = 119;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 121;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
	dummyStruct.freqOffset   = 160;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 200;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);


    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txVerifyMaskParamMap.clear();

    WIFI_SETTING_STRUCT setting;

	strcpy_s(l_txVerifyMaskParam.BSS_BANDWIDTH, MAX_BUFFER_SIZE, "BW-80");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.BSS_BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyMaskParam.BSS_BANDWIDTH;
		setting.unit        = "MHz";
		setting.helpText    = "BSS bandwidth\r\nValid options: BW-20, BW-40, BW-80, BW-80_80 or BW-160";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_BANDWIDTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	strcpy_s(l_txVerifyMaskParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, "0");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.CH_BANDWIDTH))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyMaskParam.CH_BANDWIDTH;
        setting.unit        = "MHz";
        setting.helpText    = "Channel bandwidth\r\nValid options:0, CBW-20, CBW-40, CBW-80, CBW-80_80 or CBW-160.\r\nFor 802.11ac, if it is zero,CH_BANDWIDTH equals as BSS_BANDWIDTH. For 802.11/a/b/g/n, it must always have value.";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_BANDWIDTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txVerifyMaskParam.DATA_RATE, MAX_BUFFER_SIZE, "MCS0");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.DATA_RATE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyMaskParam.DATA_RATE;
        setting.unit        = "";
        setting.helpText    = "Data rate names:\r\nDSSS-1,DSSS-2,CCK-5_5,CCK-11\r\nOFDM-6,OFDM-9,OFDM-12,OFDM-18,OFDM-24,OFDM-36,OFDM-48,OFDM-54\r\nMCS0, MCS15, etc.";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


	/*strcpy_s(l_txVerifyMaskParam.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.DATA_RATE))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyMaskParam.DATA_RATE;
        setting.unit        = "";
        setting.helpText    = "Data rate names, such as DSSS-1, CCK-5_5, CCK-11, OFDM-54, MCS0, MCS15";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }*/

    strcpy_s(l_txVerifyMaskParam.PREAMBLE, MAX_BUFFER_SIZE, "SHORT");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.PREAMBLE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyMaskParam.PREAMBLE;
        setting.unit        = "";
        setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=SHORT.";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	// "PACKET_FORMAT"
    strcpy_s(l_txVerifyMaskParam.PACKET_FORMAT, MAX_BUFFER_SIZE, PACKET_FORMAT_VHT);
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.PACKET_FORMAT))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyMaskParam.PACKET_FORMAT;
        setting.unit        = "";
        setting.helpText    = "The packet format, VHT, HT_MF,HT_GF and NON_HT as defined in standard. Default=VHT.";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	// "STANDARD"
	strcpy_s(l_txVerifyMaskParam.STANDARD, MAX_BUFFER_SIZE, STANDARD_802_11_AC); 
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.STANDARD))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyMaskParam.STANDARD;
		setting.unit        = "";
		setting.helpText    = "Used for signal analysis option or to discriminating the same data rate or package format from different standards, taking value from 802.11ac, 802.11n, 802.11ag, 802.11b. Default = 802.11ac ";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("STANDARD", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	
	strcpy_s(l_txVerifyMaskParam.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.GUARD_INTERVAL))    // Type_Checking
	{
		setting.value       = (void*)l_txVerifyMaskParam.GUARD_INTERVAL;
		setting.unit        = "";
		setting.helpText    = "Packet Guard Interval, Long or Short, default is Long";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY = 5520;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_PRIMARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.BSS_FREQ_MHZ_SECONDARY = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyMaskParam.BSS_FREQ_MHZ_SECONDARY))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyMaskParam.BSS_FREQ_MHZ_SECONDARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_SECONDARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz;
		setting.unit        = "MHz";
		setting.helpText    = "The center frequency (MHz) for primary 20 MHZ channel, priority is lower than \"CH_FREQ_MHZ\".";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_20MHz", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txVerifyMaskParam.CH_FREQ_MHZ = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyMaskParam.CH_FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyMaskParam.CH_FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "It is the center frequency (MHz) for channel. If it is zero,\"CH_FREQ_MHZ_PRIMARY_20MHz\" will be used for 802.11ac. \r\nFor 802.11/a/b/g/n, it must alway have value.";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_txVerifyMaskParam.NUM_STREAM_11AC = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txVerifyMaskParam.NUM_STREAM_11AC))    // Type_Checking
	{
		setting.value       = (void*)&l_txVerifyMaskParam.NUM_STREAM_11AC;
		setting.unit        = "";
		setting.helpText    = "Number of spatial streams based on 11AC spec";
		l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("NUM_STREAM_11AC", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_txVerifyMaskParam.SAMPLING_TIME_US = 0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyMaskParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyMaskParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyMaskParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyMaskParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyMaskParam.TX_POWER_DBM = 15.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyMaskParam.TX_POWER_DBM))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskParam.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyMaskParam.OBW_PERCENTAGE = 99.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyMaskParam.OBW_PERCENTAGE))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskParam.OBW_PERCENTAGE;
        setting.unit        = "%";
        setting.helpText    = "The percentage of power for OBW measurement, default = 99%";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_PERCENTAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.TX1 = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX1))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskParam.TX1;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 1 on/off. 1:on; 0:off";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.TX2 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX2))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskParam.TX2;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 2 on/off. 1:on; 0:off";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.TX3 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX3))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskParam.TX3;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 3 on/off. 1:on; 0:off";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyMaskParam.TX4 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.TX4))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskParam.TX4;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 4 on/off. 1:on; 0:off";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txVerifyMaskParam.ARRAY_HANDLING_METHOD = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyMaskParam.ARRAY_HANDLING_METHOD))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskParam.ARRAY_HANDLING_METHOD;
        setting.unit        = "";
        setting.helpText    = "Use the flag to handle array result. Default: 0; 0: Do nothing, 1: Display result on screen, 2: Display result and log to file";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ARRAY_HANDLING_METHOD", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_txVerifyMaskParam.VSA_CONNECTION[i], MAX_BUFFER_SIZE, "TX%d", i+1);
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.VSA_CONNECTION[i]))    // Type_Checking
        {
            setting.value       = (void*)l_txVerifyMaskParam.VSA_CONNECTION[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the actual connection between DUT antenna ports and VSAs port.\r\nValid value is TX1, TX2, TX3, TX4 and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSA%d_CONNECTION", i+1);
            l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }

    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_txVerifyMaskParam.VSA_PORT[i], MAX_BUFFER_SIZE, "");
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.VSA_PORT[i]))    // Type_Checking
        {
            setting.value       = (void*)l_txVerifyMaskParam.VSA_PORT[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the VSA port, Default is used global setting.\r\nValid value is Left, Right and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSA%d_PORT", i+1);
            l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }

    }    

	strcpy_s(l_txVerifyMaskParam.MASK_TEMPLATE, MAX_BUFFER_SIZE, "");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskParam.MASK_TEMPLATE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyMaskParam.MASK_TEMPLATE;
        setting.unit        = "";
        setting.helpText    = "Mask template, not use if blank";
        l_txVerifyMaskParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("MASK_TEMPLATE", setting) );
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

  //  for (int i=0;i<MAX_POSITIVE_SECTION;i++)
  //  {
		//l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[i] = NA_DOUBLE;
  //      setting.type = WIFI_SETTING_TYPE_DOUBLE;
  //      if (sizeof(double)==sizeof(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[i]))    // Type_Checking
  //      {
  //          setting.value = (void*)&l_txVerifyMaskReturn.MARGIN_DB_POSITIVE[i];
  //          char tempStr[MAX_BUFFER_SIZE];
  //          sprintf_s(tempStr, "MARGIN_DB_UPPER_%d", i+1);
  //          setting.unit        = "dB";
  //          setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
  //          l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
  //      }
  //      else    
  //      {
  //          printf("Parameter Type Error!\n");
  //          exit(1);
  //      }
  //  }

  //  for (int i=0;i<MAX_NEGATIVE_SECTION;i++)
  //  {
		//l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[i] = NA_DOUBLE;
  //      setting.type = WIFI_SETTING_TYPE_DOUBLE;
  //      if (sizeof(double)==sizeof(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[i]))    // Type_Checking
  //      {
  //          setting.value = (void*)&l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE[i];
  //          char tempStr[MAX_BUFFER_SIZE];
  //          sprintf_s(tempStr, "MARGIN_DB_LOWER_%d", i+1);
  //          setting.unit        = "dB";
  //          setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
  //          l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
  //      }
  //      else    
  //      {
  //          printf("Parameter Type Error!\n");
  //          exit(1);
  //      }
  //  }

  //  for (int i=0;i<MAX_POSITIVE_SECTION;i++)
  //  {
		//l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i] = NA_DOUBLE;
  //      setting.type = WIFI_SETTING_TYPE_DOUBLE;
  //      if (sizeof(double)==sizeof(l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i]))    // Type_Checking
  //      {
  //          setting.value = (void*)&l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE[i];
  //          char tempStr[MAX_BUFFER_SIZE];
  //          sprintf_s(tempStr, "FREQ_AT_MARGIN_UPPER_%d", i+1);
  //          setting.unit        = "MHz";
  //          setting.helpText    = "Point out the frequency at margin to the mask.";
  //          l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
  //      }
  //      else    
  //      {
  //          printf("Parameter Type Error!\n");
  //          exit(1);
  //      }
  //  }

  //  for (int i=0;i<MAX_NEGATIVE_SECTION;i++)
  //  {
		//l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i] = NA_DOUBLE;
  //      setting.type = WIFI_SETTING_TYPE_DOUBLE;
  //      if (sizeof(double)==sizeof(l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i]))    // Type_Checking
  //      {
  //          setting.value = (void*)&l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE[i];
  //          char tempStr[MAX_BUFFER_SIZE];
  //          sprintf_s(tempStr, "FREQ_AT_MARGIN_LOWER_%d", i+1);
  //          setting.unit        = "MHz";
  //          setting.helpText    = "Point out the frequency at margin to the mask.";
  //          l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
  //      }
  //      else    
  //      {
  //          printf("Parameter Type Error!\n");
  //          exit(1);
  //      }
  //  }
    
    //l_txVerifyMaskReturn.VIOLATION_PERCENT = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyMaskReturn.VIOLATION_PERCENT))    // Type_Checking
    //{
    //    setting.value = (void*)&l_txVerifyMaskReturn.VIOLATION_PERCENT;
    //    setting.unit        = "%";
    //    setting.helpText    = "Percentage which fail the mask";
    //    l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VIOLATION_PERCENT", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

	//l_txVerifyMaskReturn.OBW_FREQ_START_MHZ = NA_DOUBLE;
 //   setting.type = WIFI_SETTING_TYPE_DOUBLE;
 //   if (sizeof(double)==sizeof(l_txVerifyMaskReturn.OBW_FREQ_START_MHZ))    // Type_Checking
 //   {
 //       setting.value = (void*)&l_txVerifyMaskReturn.OBW_FREQ_START_MHZ;
 //       setting.unit        = "MHz";
 //       setting.helpText    = "";
 //       l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_FREQ_START_MHZ", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

	//l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ = NA_DOUBLE;
 //   setting.type = WIFI_SETTING_TYPE_DOUBLE;
 //   if (sizeof(double)==sizeof(l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ))    // Type_Checking
 //   {
 //       setting.value = (void*)&l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ;
 //       setting.unit        = "MHz";
 //       setting.helpText    = "";
 //       l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_FREQ_STOP_MHZ", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

	//l_txVerifyMaskReturn.OBW_MHZ = NA_DOUBLE;
 //   setting.type = WIFI_SETTING_TYPE_DOUBLE;
 //   if (sizeof(double)==sizeof(l_txVerifyMaskReturn.OBW_MHZ))    // Type_Checking
 //   {
 //       setting.value = (void*)&l_txVerifyMaskReturn.OBW_MHZ;
 //       setting.unit        = "MHz";
 //       setting.helpText    = "OBW for the specified power percentage";
 //       l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_MHZ", setting) );
 //   }
 //   else    
 //   {
 //       printf("Parameter Type Error!\n");
 //       exit(1);
 //   }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyMaskReturn.CABLE_LOSS_DB[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyMaskReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyMaskReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}


	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i] = new vector<double>();
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i]->clear();
		setting.type = WIFI_SETTING_TYPE_ARRAY_DOUBLE;
		if (0 == l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i]->size())    // Type_Checking
		{
			setting.value = (void*)l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i];
			setting.unit        = "Hz";
			setting.helpText    = "Spectrum X raw data.";
			char tmpStr[MAX_BUFFER_SIZE];
			sprintf_s(tmpStr, MAX_BUFFER_SIZE, "RAW_DATA_FREQ_VSA%d", i+1);
			l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tmpStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}


	}

	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i] = new vector<double>();
		l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->clear();
		setting.type = WIFI_SETTING_TYPE_ARRAY_DOUBLE;
		if (0 == l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->size())    // Type_Checking
		{
			setting.value = (void*)l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i];
			setting.unit        = "dBm/100kHz";
			setting.helpText    = "Spectrum Y raw data.";
			char tmpStr[MAX_BUFFER_SIZE];
			sprintf_s(tmpStr, MAX_BUFFER_SIZE, "RAW_DATA_POWER_VSA%d", i+1);
		    l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tmpStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}



    //for supported multi-tester

	for (int i=0;i<MAX_TESTER_NUM;i++)
	{            
        for (int j=0;j<MAX_POSITIVE_SECTION;j++)
        {
		    l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[i][j] = NA_DOUBLE;
            setting.type = WIFI_SETTING_TYPE_DOUBLE;
            if (sizeof(double)==sizeof(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[i][j]))    // Type_Checking
            {
                setting.value = (void*)&l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[i][j];
                char tempStr[MAX_BUFFER_SIZE];
				switch(j)
				{
				case 0:
					  sprintf_s(tempStr, "MARGIN_DB_UP_A_VSA%d", i+1);
					  break;
				case 1:
					  sprintf_s(tempStr, "MARGIN_DB_UP_B_VSA%d", i+1);
					  break;
				case 2:
					  sprintf_s(tempStr, "MARGIN_DB_UP_C_VSA%d", i+1);
					  break;
				case 3:
					  sprintf_s(tempStr, "MARGIN_DB_UP_D_VSA%d", i+1);
					  break;
				default:
					break;
				}
                setting.unit        = "dB";
                setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
                l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
            }
            else    
            {
                printf("Parameter Type Error!\n");
                exit(1);
            }
        }
	}

	for (int i=0;i<MAX_TESTER_NUM;i++)
	{            
        for (int j=0;j<MAX_NEGATIVE_SECTION;j++)
        {
		    l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[i][j] = NA_DOUBLE;
            setting.type = WIFI_SETTING_TYPE_DOUBLE;
            if (sizeof(double)==sizeof(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[i][j]))    // Type_Checking
            {
                setting.value = (void*)&l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[i][j];
                char tempStr[MAX_BUFFER_SIZE];
				switch(j)
				{
				case 0:
					  sprintf_s(tempStr, "MARGIN_DB_LO_A_VSA%d", i+1);
					  break;
				case 1:
					  sprintf_s(tempStr, "MARGIN_DB_LO_B_VSA%d", i+1);
					  break;
				case 2:
					  sprintf_s(tempStr, "MARGIN_DB_LO_C_VSA%d", i+1);
					  break;
				case 3:
					  sprintf_s(tempStr, "MARGIN_DB_LO_D_VSA%d", i+1);
					  break;
				default:
					break;
				}                
				setting.unit        = "dB";
                setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
                l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
            }
            else    
            {
                printf("Parameter Type Error!\n");
                exit(1);
            }
        }
	}

   	for (int i=0;i<MAX_TESTER_NUM;i++)
	{            
         for (int j=0;j<MAX_POSITIVE_SECTION;j++)
        {
		    l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[i][j] = NA_DOUBLE;
            setting.type = WIFI_SETTING_TYPE_DOUBLE;
            if (sizeof(double)==sizeof(l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[i][j]))    // Type_Checking
            {
                setting.value = (void*)&l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[i][j];
                char tempStr[MAX_BUFFER_SIZE];
				switch(j)
				{
				case 0:
					  sprintf_s(tempStr, "FREQ_MHZ_UP_A_VSA%d", i+1);
					  break;
				case 1:
					  sprintf_s(tempStr, "FREQ_MHZ_UP_B_VSA%d", i+1);
					  break;
				case 2:
					  sprintf_s(tempStr, "FREQ_MHZ_UP_C_VSA%d", i+1);
					  break;
				case 3:
					  sprintf_s(tempStr, "FREQ_MHZ_UP_D_VSA%d", i+1);
					  break;
				default:
					break;
				}                
                setting.unit        = "MHz";
                setting.helpText    = "Point out the frequency at margin to the mask.";
                l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
            }
            else    
            {
                printf("Parameter Type Error!\n");
                exit(1);
            }
        }
    }

   	for (int i=0;i<MAX_TESTER_NUM;i++)
	{            
         for (int j=0;j<MAX_NEGATIVE_SECTION;j++)
        {
		    l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[i][j] = NA_DOUBLE;
            setting.type = WIFI_SETTING_TYPE_DOUBLE;
            if (sizeof(double)==sizeof(l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[i][j]))    // Type_Checking
            {
                setting.value = (void*)&l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[i][j];
                char tempStr[MAX_BUFFER_SIZE];
				switch(j)
				{
				case 0:
					  sprintf_s(tempStr, "FREQ_MHZ_LO_A_VSA%d", i+1);
					  break;
				case 1:
					  sprintf_s(tempStr, "FREQ_MHZ_LO_B_VSA%d", i+1);
					  break;
				case 2:
					  sprintf_s(tempStr, "FREQ_MHZ_LO_C_VSA%d", i+1);
					  break;
				case 3:
					  sprintf_s(tempStr, "FREQ_MHZ_LO_D_VSA%d", i+1);
					  break;
				default:
					break;
				}                         
                setting.unit        = "MHz";
                setting.helpText    = "Point out the frequency at margin to the mask.";
                l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
            }
            else    
            {
                printf("Parameter Type Error!\n");
                exit(1);
            }
        }
    }
    
    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
	    l_txVerifyMaskReturn.OBW_FREQ_START_MHZ_VSA[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyMaskReturn.OBW_FREQ_START_MHZ_VSA[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyMaskReturn.OBW_FREQ_START_MHZ_VSA[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "OBW_FREQ_START_MHZ_VSA_%d", i+1);
            setting.unit        = "MHz";
            setting.helpText    = "";
            //l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
	    l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ_VSA[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ_VSA[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyMaskReturn.OBW_FREQ_STOP_MHZ_VSA[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "OBW_FREQ_STOP_MHZ_VSA_%d", i+1);
            setting.unit        = "MHz";
            setting.helpText    = "";
            //l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
	    l_txVerifyMaskReturn.OBW_MHZ_VSA[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyMaskReturn.OBW_MHZ_VSA[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyMaskReturn.OBW_MHZ_VSA[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "OBW_MHZ_VSA_%d", i+1);
            setting.unit        = "MHz";
            setting.helpText    = "OBW for the specified power percentage";
            l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        l_txVerifyMaskReturn.VIOLATION_PERCENT_VSA[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyMaskReturn.VIOLATION_PERCENT_VSA[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyMaskReturn.VIOLATION_PERCENT_VSA[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VIOLATION_PERCENT_VSA_%d", i+1);
            setting.unit        = "%";
            setting.helpText    = "Percentage which fail the mask";
            l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }

    }



    l_txVerifyMaskReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyMaskReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyMaskReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_txVerifyMaskReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz = 0;
    if (sizeof(int)==sizeof(l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 40 MHZ channel";
        l_txVerifyMaskReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_40MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz = 0;
    if (sizeof(int)==sizeof(l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 80 MHZ channel";
        l_txVerifyMaskReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_80MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


    return 0;
}

void CleanupTXVerifyMaskContainers(void)
{
	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		if( NULL!=l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i] )
		{
			delete l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i];
			l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_X_VSA[i] = NULL;
		}

		if( NULL!=l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i] )
		{
			delete l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i];
			l_txVerifyMaskReturn.SPECTRUM_RAW_DATA_Y_VSA[i] = NULL;
		}


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

int VerifyPowerMask (int txIndex, double *freqBuf, double *maskBuf, int maskBufCnt, int wifiMode, int bssBW, int cbw)		
{
    int passTest = ERR_OK;

    //int    maskTopStart = 999999, maskTopStop = 0;
    double peakPower = NA_NUMBER;
	double *maskWiFi = new double[maskBufCnt];

    double firstRefFreq =  freqBuf[0]/1000000;				// to MHz
    double deltaFreq    = (freqBuf[1]-freqBuf[0])/1000000;	// to MHz
    

    vector<DEFAULT_MASK_STRUCT>::iterator maskBegin_Iter;
    vector<DEFAULT_MASK_STRUCT>::iterator maskEnd_Iter;
    vector<DEFAULT_MASK_STRUCT>::iterator dummy_Iter;

    /*--------------------------*
     *  Looking for peak power  *
     *--------------------------*/
    for (int i=0;i<maskBufCnt;i++)
    {
        if ( maskBuf[i]>peakPower )  
        {   // store the current peak power and keep searching
            peakPower = maskBuf[i];
        }
        else
        {
            // keep searching...
        }
    }

    /*-----------------------*
     *  Select default Mask  *
     *-----------------------*/
	if (wifiMode == WIFI_11B)    //11b
	{
		maskBegin_Iter = l_wifiMask_11b.begin();
        maskEnd_Iter = l_wifiMask_11b.end();
	}
	else if (wifiMode == WIFI_11AG)
	{
		maskBegin_Iter = l_wifiMask_11ag.begin();
        maskEnd_Iter = l_wifiMask_11ag.end();
	}
	else if ((wifiMode == WIFI_11N_MF_HT20) || (wifiMode == WIFI_11N_GF_HT20) )
	{
        maskBegin_Iter = l_wifiMask_11nHt20.begin();
        maskEnd_Iter = l_wifiMask_11nHt20.end();
	}
	else if ((wifiMode == WIFI_11N_MF_HT40) || (wifiMode == WIFI_11N_GF_HT40) )
	{
        maskBegin_Iter = l_wifiMask_11nHt40.begin();
        maskEnd_Iter = l_wifiMask_11nHt40.end();
	}
	else // 11ac
	{
		int tempBW = 0;

		if (  !g_globalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW)
		{
			// Use bssBW to calculate standard mask
			tempBW = bssBW;
		}
		else
		{
			// Use CBW to calculate standard mask
			tempBW = cbw;
		}
		switch (tempBW)
		{
		case BW_20MHZ:
			maskBegin_Iter = l_wifiMask_11acBss20.begin();
            maskEnd_Iter = l_wifiMask_11acBss20.end();
			break;
		case BW_40MHZ:
			maskBegin_Iter = l_wifiMask_11acBss40.begin();
            maskEnd_Iter = l_wifiMask_11acBss40.end();
			break;
		case BW_80MHZ:
			maskBegin_Iter = l_wifiMask_11acBss80.begin();
            maskEnd_Iter = l_wifiMask_11acBss80.end();
			break;
		case BW_160MHZ:
			maskBegin_Iter = l_wifiMask_11acBss160.begin();
            maskEnd_Iter = l_wifiMask_11acBss160.end();
			break;
		case BW_80_80MHZ:
			maskBegin_Iter = l_wifiMask_11acBss80_80.begin();
            maskEnd_Iter = l_wifiMask_11acBss80_80.end();
			break;
		default:
			maskBegin_Iter = l_wifiMask_11acBss20.begin();
            maskEnd_Iter = l_wifiMask_11acBss20.end();
			break;
		}

	}

    /*-------------------------------*
     *  Construct the absolute Mask  *
     *-------------------------------*/
    int    sectionIndex = 0;
	double currentFreq  = 0, xVal = 0, yDeltaPwrdB = 0;

	// Initial the return results
	l_txVerifyMaskReturn.VIOLATION_PERCENT_VSA[txIndex]  = 0;
	for (int x=0;x<MAX_POSITIVE_SECTION;x++)
	{
		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][x]		 = NA_DOUBLE;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[txIndex][x] = NA_DOUBLE;
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][x]		 = NA_DOUBLE;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[txIndex][x] = NA_DOUBLE;
	}

    currentFreq = firstRefFreq;	 // point to first one

    for (int i=0;i<maskBufCnt;i++)
    {
        xVal = fabs(currentFreq);
        dummy_Iter = maskBegin_Iter;
		sectionIndex = 0;

        while ( dummy_Iter!=maskEnd_Iter ) // not end of mask vector
        {
            if ( xVal >= dummy_Iter->freqOffset )
            {
                dummy_Iter++;
				sectionIndex++;

                if ( dummy_Iter!=maskEnd_Iter ) // not end of mask vector
                {
                    if ( xVal < dummy_Iter->freqOffset )    // find out the range
                    {   
                        double xMax = dummy_Iter->freqOffset;
                        double yMax = dummy_Iter->deltaMaskPwr;                        
                        dummy_Iter--;
						sectionIndex--;
                        double xMin = dummy_Iter->freqOffset;
                        double yMin = dummy_Iter->deltaMaskPwr;                        
                        
                        yDeltaPwrdB = IQInterpolate( yMax, yMin, xMax, xMin, xVal );

                        //if (dummy_Iter->deltaMaskPwr==0) // This is the top of spectrum
                        //{
                        //    if (i<maskTopStart) maskTopStart = i;
                        //    if (i>maskTopStop)  maskTopStop  = i;
                        //}
                        //else
                        //{
                        //    // do nothing, not top
                        //}
                        break;
                    }
                    else
                    {
                        // not in this Mask range, continue and keep searching
                        continue;
                    }
                }
                else    // already end of vector, thus direct assign mask_Iter->deltaMaskPwr to yDeltaPwrdB
                {
                    dummy_Iter--;
					sectionIndex--;
                    yDeltaPwrdB = dummy_Iter->deltaMaskPwr;   
                    break;
                }
            }
            else
            {
                // no this case, because fabs(currentFreq) should >= 0
            }
        }         

        // Construct the absolute Mask and store into Array
        maskWiFi[i] = peakPower + yDeltaPwrdB;

		/*---------------------------------*
		 *  Report Margin by each section  *
		 *---------------------------------*/		
		if ( sectionIndex==0 )		// This is the "TOP" of spectral, just skip this section.
		{
			// do nothing, moving to next frequency step
		}
		else
		{
			double deltaPower = maskBuf[i] - maskWiFi[i];	// check Mask

			if ( i>(maskBufCnt/2) )		// Upper section
			{
				if ( deltaPower>l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][sectionIndex-1] )	// need to save this as margin
				{
					l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][sectionIndex-1]      = deltaPower;		// save margin		
//					l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[txIndex][sectionIndex-1] = (freqBuf[i]/1e6) + l_txVerifyMaskParam.FREQ_MHZ;	// save frequency
					l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[txIndex][sectionIndex-1] = (freqBuf[i]/1e6);	// save frequency, modified to report the same value as WiFi test
				
				}
				else
				{
					// do nothing
				}			
			}
			else						// Lower section
			{
				if ( deltaPower>l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][sectionIndex-1] )	// need to save this as margin
				{
					l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][sectionIndex-1]      = deltaPower;		// save margin
//					l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[txIndex][sectionIndex-1] = (freqBuf[i]/1e6) + l_txVerifyMaskParam.FREQ_MHZ;	// save frequency
					l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[txIndex][sectionIndex-1] = (freqBuf[i]/1e6);	// save frequency, modified to report the same value as WiFi test
				}
				else
				{
					// do nothing
				}	
			}

			if ( deltaPower>0 )  // Mask Failed
			{
				l_txVerifyMaskReturn.VIOLATION_PERCENT_VSA[txIndex]++;
			}
			else    // Mask Pass
			{
				// keep compare...
			}
		}

        // moving to next frequency step
        currentFreq = currentFreq + deltaFreq;
    }   // end - for (int i=0;i<maskBufCnt;i++)

	if ( wifiMode==WIFI_11B )	// 802.11b only have -2,-1,+1,+2 sections
	{
		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][0]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][1]);		// save margin		
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[txIndex][0] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[txIndex][1];	// save frequency
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][0]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][1]);		// save margin
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[txIndex][0] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[txIndex][1];	// save frequency

		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][1]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][3]);		// save margin		
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[txIndex][1] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[txIndex][3];	// save frequency
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][1]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][3]);		// save margin
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[txIndex][1] = l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[txIndex][3];	// save frequency

		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][2]      = NA_DOUBLE;				
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[txIndex][2] = NA_DOUBLE;
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][2]      = NA_DOUBLE;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[txIndex][2] = NA_DOUBLE;

		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][3]      = NA_DOUBLE;				
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_POSITIVE_VSA[txIndex][3] = NA_DOUBLE;
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][3]      = NA_DOUBLE;
		l_txVerifyMaskReturn.FREQ_AT_MARGIN_NEGATIVE_VSA[txIndex][3] = NA_DOUBLE;
	}
	else
	{
		// Turn Values to Positive
		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][0]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][0]);     
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][0]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][0]);     
		
		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][1]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][1]);     
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][1]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][1]);     
														  
		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][2]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][2]);     
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][2]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][2]);     
														  
		l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][3]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_POSITIVE_VSA[txIndex][3]);     
		l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][3]      = -1*(l_txVerifyMaskReturn.MARGIN_DB_NEGATIVE_VSA[txIndex][3]);     
		
	}


    // Percentage, which fail the mask
	l_txVerifyMaskReturn.VIOLATION_PERCENT_VSA[txIndex] = (l_txVerifyMaskReturn.VIOLATION_PERCENT_VSA[txIndex]/maskBufCnt) * 100.0;

    #if defined(_DEBUG)

	::LP_Plot(testTimes, freqBuf, maskBuf, maskBufCnt, "-", "Mask",  l_txVerifyMaskParam.DATA_RATE, "Power",1);
	::LP_Plot(testTimes, freqBuf, maskWiFi, maskBufCnt, "-r", "Mask",  l_txVerifyMaskParam.DATA_RATE, "Power", 1);
	

		//print out the Mask into file for debug
		FILE *fp;  
		fopen_s(&fp, "Log_Mask_Result.csv", "wt+");    
		if (fp)
		{
			fprintf(fp, "Frequency,WiFi Mask,Signal,Size: %d\n", maskBufCnt);    // print to log file 
			for (int x=0;x<maskBufCnt;x++)
			{ 
				fprintf(fp, "%8.2f,%8.2f,%8.2f\n", freqBuf[x], maskWiFi[x], maskBuf[x]);    // print to log file 
			}
			fclose(fp);
		}
    #endif

    // free memory
	delete [] maskWiFi;

    if (l_txVerifyMaskReturn.VIOLATION_PERCENT_VSA[txIndex]!=0)
    {
        passTest = -1;
    }
    else  
    {
        passTest = ERR_OK; 
    }

 	return passTest;
}

int VerifyOBW(double *maskBuf, int maskBufCnt, double obwPercentage, int &startIndex, int &stopIndex)
{

	int passTest = ERR_OK;

	double  totalPower = 0;
//	double  obwPowerLimit = 0;
    double  obwPowerExceed = 0.0;
    double  sumPower = 0.0;


	// Step 1, calculate the total power
	for (int i=0;i<maskBufCnt;i++)
	{
		totalPower = totalPower + pow( 10,(maskBuf[i]/10) );
	}

	// Step 2, calculate the exceed part( >99%) by OBW percantage
    obwPowerExceed = totalPower * (1.0 - obwPercentage);
    obwPowerExceed /= 2.0;

    //Step 3, Set flag from left sideband and right sideband
    startIndex = 0;
    stopIndex  = maskBufCnt - 1;

    //Find the point in 99%, start from left sideband, increase frequency
    sumPower = pow( 10,(maskBuf[startIndex]/10) );
    while((sumPower <= obwPowerExceed) && (startIndex < (maskBufCnt/2)))
    {
        startIndex++;
        sumPower += pow( 10,(maskBuf[startIndex]/10) );
    }

    //Find the point in 99%, start from right sideband, decrease frequency
    sumPower = pow( 10,(maskBuf[stopIndex]/10) );
    while((sumPower <= obwPowerExceed) && (stopIndex > (maskBufCnt/2)))
    {
        stopIndex --;
        sumPower += pow( 10,(maskBuf[stopIndex]/10) );
    }

	return passTest;
}


//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
// 
//-------------------------------------------------------------------------------------
int CheckTxMaskParameters( int *bssBW, 
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
          err = CheckCommonParameters_WiFi_11ac_MiMo(l_txVerifyMaskParamMap, 
			                                         l_txVerifyMaskReturnMap, 
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

		

//		if ( 0!=strcmp(l_txVerifyMaskParam.PREAMBLE, "SHORT") && 0!=strcmp(l_txVerifyMaskParam.PREAMBLE, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_txVerifyMaskParam.PREAMBLE);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0!=strcmp(l_txVerifyMaskParam.GUARD_INTERVAL, "SHORT") && 0!=strcmp(l_txVerifyMaskParam.GUARD_INTERVAL, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown GUARD_INTERVAL, WiFi guard interval %s not supported.\n", l_txVerifyMaskParam.GUARD_INTERVAL);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if (( 0> l_txVerifyMaskParam.NUM_STREAM_11AC) || ( l_txVerifyMaskParam.NUM_STREAM_11AC >8))
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
//		if ( (0!=strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "11AC_MF_HT")) && (0!=strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "11AC_GF_HT")) &&
//			(0!=strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "11N_MF_HT")) && (0!=strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "11N_GF_HT")) &&
//			(0!=strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "11AC_VHT")) &&( 0!=strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "11AC_NON_HT"))
//			&&( 0!=strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "LEGACY_NON_HT")))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown PACKET_FORMAT, WiFi 11ac packet format %s not supported.\n", l_txVerifyMaskParam.PACKET_FORMAT);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0 == strcmp( l_txVerifyMaskParam.PACKET_FORMAT, "11AC_NON_HT") &&
//			! strstr ( l_txVerifyMaskParam.DATA_RATE, "OFDM"))
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
//	/*	err = TM_WiFiConvertFrequencyToChannel(l_txVerifyMaskParam.FREQ_MHZ, channel);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyMaskParam.FREQ_MHZ);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}*/
//
//		err = TM_WiFiConvertDataRateNameToIndex(l_txVerifyMaskParam.DATA_RATE, &dummyInt);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_txVerifyMaskParam.DATA_RATE);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( 0 == strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "11AC_VHT"))  // Data rate: MCS0 ~MCS9
//		{
//			if (( 14 <= dummyInt ) && (dummyInt <= 23))
//			{
//				// Data rate is right
//				//do nothing
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] PACKET_FORMAT and DATA_RATE don't match! The data rates of VHT_11AC must be MCS0 ~ MCS9, doesn't support %s!\n", &l_txVerifyMaskParam.DATA_RATE);
//				throw logMessage;
//
//			}
//
//		}
//		else
//		{
//		}
//
//		if ( !strstr (l_txVerifyMaskParam.PACKET_FORMAT, "11AC"))    // Legacy signal, CH_BANDWIDTH and CH_FREQ_MHZ must have values
//		{
//			if (( l_txVerifyMaskParam.CH_BANDWIDTH <= 0) || ( l_txVerifyMaskParam.CH_FREQ_MHZ <= 0))
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
//
//#pragma region  Check bandwidth and channel list
//
//		/*----------------------*
//		*   Check bandwidth		*
//		*-----------------------*/
//		if ( 0!=strcmp(l_txVerifyMaskParam.BSS_BANDWIDTH, "BW-20") && 0!=strcmp(l_txVerifyMaskParam.BSS_BANDWIDTH, "BW-40") && 
//			 0!=strcmp(l_txVerifyMaskParam.BSS_BANDWIDTH, "BW-80") && 0!=strcmp(l_txVerifyMaskParam.BSS_BANDWIDTH, "BW-160") &&
//			 0!=strcmp(l_txVerifyMaskParam.BSS_BANDWIDTH, "BW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_txVerifyMaskParam.BSS_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_txVerifyMaskParam.BSS_BANDWIDTH, "BW-20"))
//			{
//				*bssBW = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyMaskParam.BSS_BANDWIDTH, "BW-40"))
//			{
//				*bssBW = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyMaskParam.BSS_BANDWIDTH, "BW-80"))
//			{
//				*bssBW = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyMaskParam.BSS_BANDWIDTH, "BW-160"))
//			{
//				*bssBW = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyMaskParam.BSS_BANDWIDTH, "BW-80_80"))
//			{
//				*bssBW = BW_80_80MHZ;
//
//				if ( l_txVerifyMaskParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
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
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_txVerifyMaskParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//
//		}
//
//		if ( 0 != strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "0") && 
//			0!=strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "CBW-20") && 0!=strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "CBW-40") && 
//			 0!=strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "CBW-80") && 0!=strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "CBW-160") &&
//			 0!=strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "CBW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown CBW BANDWIDTH, WiFi 11ac CBW bandwidth %s not supported.\n", l_txVerifyMaskParam.CH_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "0"))
//			{
//				sprintf_s(l_txVerifyMaskParam.CH_BANDWIDTH,MAX_BUFFER_SIZE,"C%s",l_txVerifyMaskParam.BSS_BANDWIDTH);
//				*cbw = *bssBW;
//			}
//			else if ( 0 == strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "CBW-20"))
//			{
//				*cbw = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "CBW-40"))
//			{
//				*cbw = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "CBW-80"))
//			{
//				*cbw = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "CBW-160"))
//			{
//				*cbw = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, "CBW-80_80"))
//			{
//				*cbw = BW_80_80MHZ;
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_txVerifyMaskParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//		}
//	
//		if ( strstr ( l_txVerifyMaskParam.PREAMBLE,"11AC"))  //802.11ac, cbw can't be larger than bssBW
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
//		if ( strstr( l_txVerifyMaskParam.PACKET_FORMAT, "11N"))  //802.11n, BW= 20,40MHz
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
//		else if ( 0 == strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "LEGACY_NON_HT"))  // 11B and 11ag
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
//			if ( 0 == strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "11AC_VHT"))
//			{
//				//do nothing
//			}
//			else if ( 0 == strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "11AC_MF_HT") ||
//				0 == strcmp(l_txVerifyMaskParam.PACKET_FORMAT, "11AC_GF_HT"))
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
//		if (l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY <= 0)
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
//		err = TM_WiFiConvertFrequencyToChannel(l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY, bssPchannel);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( *bssBW == BW_80_80MHZ)  // Need BSS_FREQ_MHZ_SECONDARY
//		{
//			if ( l_txVerifyMaskParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
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
//			err = TM_WiFiConvertFrequencyToChannel(l_txVerifyMaskParam.BSS_FREQ_MHZ_SECONDARY, bssSchannel);      
//			if ( ERR_OK!=err )
//			{
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyMaskParam.BSS_FREQ_MHZ_SECONDARY);
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
//		if ( strstr (l_txVerifyMaskParam.PACKET_FORMAT, "11AC"))
//		{
//
//			if ( 0 == l_txVerifyMaskParam.CH_FREQ_MHZ)  
//			{
//				//no CH_FREQ_MHZ, no CH_FREQ_MHZ_PRIMARY_20MHz
//				// Use default values
//				if ( 0 == l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz) 
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi_11ac_MiMo] Don't have input parameters \"CH_FREQ_MHZ\" or \"CH_FREQ_MHZ_PRIMARY_20\", using BBS center freuqency as default!");
//					l_txVerifyMaskParam.CH_FREQ_MHZ = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY;
//
//					// all  use lower frequency for channel list
//					switch (*bssBW)
//					{
//					case BW_20MHZ:
//						l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_40MHZ:
//						l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 10;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_80MHZ:
//						l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 20;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY;
//						break;
//					
//					case BW_160MHZ:			
//						l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//				
//					case BW_80_80MHZ:				
//						l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//
//					default:
//						l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					}
//
//				}
//				else  //no CH_FREQ_MHZ, have CH_FREQ_MHZ_PRIMARY_20MHz. Use input CH_FREQ_MHZ_PRIMARY_20MHz to calculate
//				{
//					err = GetChannelList(*bssBW, l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//					if ( err != ERR_OK) // Wrong channel list
//					{
//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_txVerifyMaskParam.PACKET_FORMAT);
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
//						l_txVerifyMaskParam.CH_FREQ_MHZ = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//					else if (*cbw == BW_20MHZ)
//					{
//						l_txVerifyMaskParam.CH_FREQ_MHZ = l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz;
//					}
//					else if (*cbw == BW_40MHZ)
//					{
//						l_txVerifyMaskParam.CH_FREQ_MHZ = l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
//					}
//					else if (*cbw == BW_80MHZ)
//					{
//						l_txVerifyMaskParam.CH_FREQ_MHZ = l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
//					}
//					else
//					{
//						l_txVerifyMaskParam.CH_FREQ_MHZ = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//				}
//				else // cbw = BW_80_80MHZ, use BSS primary and secondary
//				{
//					l_txVerifyMaskParam.CH_FREQ_MHZ = NA_INTEGER;
//					// do nothing
//				}
//			}
//			else  // CH_FREQ_MHZ: non-zero 
//			{
//				//Check if input CH_FREQ_MHZ is correct
//				err = CheckChannelFreq(*bssBW, *cbw,
//					l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyMaskParam.CH_FREQ_MHZ);
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
//				if ( 0 != l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz)
//				{
//					err = CheckChPrimary20(*bssBW, *cbw,
//							l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyMaskParam.CH_FREQ_MHZ,l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz);
//					if ( err == ERR_OK)  //input CH_FREQ_MHZ_PRIMARY_20MHz is correct
//					{
//						err = GetChannelList(*bssBW, l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//							&l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//						if ( err != ERR_OK) // Wrong channel list
//						{
//							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Can't get channel list. Please check input paramters.\n", l_txVerifyMaskParam.PACKET_FORMAT);
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
//						l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					case BW_40MHZ:  //cbw = BW_20.40MHz
//						if (*cbw == BW_20MHZ)
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 10;
//
//						}
//						else  //wrong cbw
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80MHZ:  //cbw = BW_20,40,80MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_txVerifyMaskParam.CH_FREQ_MHZ == l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_txVerifyMaskParam.CH_FREQ_MHZ == l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  //wrong cbw
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz =  NA_INTEGER;
//
//						}
//						break;
//					case BW_160MHZ:  //cbw = BW_20,40,80,160MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_txVerifyMaskParam.CH_FREQ_MHZ == l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 60)
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_txVerifyMaskParam.CH_FREQ_MHZ == l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_txVerifyMaskParam.CH_FREQ_MHZ == l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else if ( l_txVerifyMaskParam.CH_FREQ_MHZ == l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY + 60)
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY + 50;
//							}
//							else  //wrong CH_FREQ_MHZ
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							if (l_txVerifyMaskParam.CH_FREQ_MHZ == l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 40)
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_txVerifyMaskParam.CH_FREQ_MHZ == l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY + 40)
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_160MHZ)
//						{
//							if (l_txVerifyMaskParam.CH_FREQ_MHZ == l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY)
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else  //wring cbw
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80_80MHZ:
//						if ( *cbw == BW_20MHZ)
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if ( l_txVerifyMaskParam.CH_FREQ_MHZ == l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_txVerifyMaskParam.CH_FREQ_MHZ == l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else if ( *cbw == BW_80_80MHZ)
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  // wrong CH_FREQ_MHz
//						{
//							l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					default:
//						l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						break;
//					}
//				}
//
//				err = GetChannelList(*bssBW, l_txVerifyMaskParam.BSS_FREQ_MHZ_PRIMARY,l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//				if ( err != ERR_OK)  //Get channel list wrong
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_txVerifyMaskParam.PACKET_FORMAT);
//					throw logMessage;
//				}
//				else  //Get channel list successfully
//				{
//				}
//			}	
//
//			//Check channel list. If all are "NA_INTEGER", return error
//			if (( l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz == NA_INTEGER ) && 
//				(l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz == NA_INTEGER ) &&
//				(l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER))
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
//			l_txVerifyMaskParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//			l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//			l_txVerifyMaskReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//		}
//#pragma endregion
//
//		//VHT, cbw = BW_20MHZ, the stream number of MCS9 only can 2,4,6,8
//		//if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )
//		//{
//		//	if ( (l_txVerifyMaskParam.NUM_STREAM_11AC != 2) ||
//		//		(l_txVerifyMaskParam.NUM_STREAM_11AC != 4) ||
//		//		(l_txVerifyMaskParam.NUM_STREAM_11AC != 6) ||
//		//		(l_txVerifyMaskParam.NUM_STREAM_11AC != 8))
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
////VHT, cbw = BW_20MHz, the stream number of MCS9 only can 3,6 According to the standard, bguo. 7/17/2012
//		if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )
//		{
//			if ( (l_txVerifyMaskParam.NUM_STREAM_11AC != 3) &&
//				(l_txVerifyMaskParam.NUM_STREAM_11AC != 6) )
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
//			if ( l_txVerifyMaskParam.NUM_STREAM_11AC == 6)
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
//			if ( l_txVerifyMaskParam.NUM_STREAM_11AC == 3 || l_txVerifyMaskParam.NUM_STREAM_11AC == 7)
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
//		err = WiFi_11ac_TestMode(l_txVerifyMaskParam.DATA_RATE, cbw, wifiMode, wifiStreamNum, l_txVerifyMaskParam.PACKET_FORMAT);
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
//			l_txVerifyMaskParam.CH_FREQ_MHZ,
//										vsaEnabled[0],
//										vsaEnabled[1],
//										vsaEnabled[2],
//										vsaEnabled[3],
//										l_txVerifyMaskParam.CABLE_LOSS_DB,
//										l_txVerifyMaskReturn.CABLE_LOSS_DB,
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
//		// Check Capture Time 
//		// Check sampling time 
//		if (0 == l_txVerifyMaskParam.SAMPLING_TIME_US)
//		{
//			*samplingTimeUs = CheckSamplingTime(*wifiMode, l_txVerifyMaskParam.PREAMBLE, l_txVerifyMaskParam.DATA_RATE, l_txVerifyMaskParam.PACKET_FORMAT);
//		}
//		else	// SAMPLING_TIME_US != 0
//		{
//			*samplingTimeUs = l_txVerifyMaskParam.SAMPLING_TIME_US;
//		}

		  //check unique input parameters used only for TX_MASK test ... 
		  /*-------------------------------------*
		  *  set PAPR values for vsa setting     *
		  *--------------------------------------*/

		if ( *wifiMode==WIFI_11B )  *peakToAvgRatio = g_globalSettingParam.IQ_P_TO_A_11B_11M;    // CCK        
		else                        *peakToAvgRatio = g_globalSettingParam.IQ_P_TO_A_11AG_54M;   // OFDM 


		// Check Dut configuration changed or not
		if (  g_globalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.
			0!=strcmp(l_txVerifyMaskParam.CH_BANDWIDTH, g_RecordedParam.CH_BANDWIDTH) ||
			  0!=strcmp(l_txVerifyMaskParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
			  0!=strcmp(l_txVerifyMaskParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
			  0!=strcmp(l_txVerifyMaskParam.PACKET_FORMAT, g_RecordedParam.PACKET_FORMAT) ||
			  0!=strcmp(l_txVerifyMaskParam.GUARD_INTERVAL, g_RecordedParam.GUARD_INTERVAL_11N) ||
			  l_txVerifyMaskParam.NUM_STREAM_11AC != g_RecordedParam.NUM_STREAM_11AC ||
			  l_txVerifyMaskParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_txVerifyMaskParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_txVerifyMaskParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_txVerifyMaskParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  l_txVerifyMaskParam.CH_FREQ_MHZ!=g_RecordedParam.CH_FREQ_MHZ ||
			  l_txVerifyMaskParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
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
		g_RecordedParam.ANT1					= l_txVerifyMaskParam.TX1;
		g_RecordedParam.ANT2					= l_txVerifyMaskParam.TX2;
		g_RecordedParam.ANT3					= l_txVerifyMaskParam.TX3;
		g_RecordedParam.ANT4					= l_txVerifyMaskParam.TX4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txVerifyMaskParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txVerifyMaskParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txVerifyMaskParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txVerifyMaskParam.CABLE_LOSS_DB[3];	
		g_RecordedParam.CH_FREQ_MHZ				= l_txVerifyMaskParam.CH_FREQ_MHZ;	
		g_RecordedParam.POWER_DBM				= l_txVerifyMaskParam.TX_POWER_DBM;	
		g_RecordedParam.NUM_STREAM_11AC			= l_txVerifyMaskParam.NUM_STREAM_11AC;

		sprintf_s(g_RecordedParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, l_txVerifyMaskParam.CH_BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_txVerifyMaskParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_txVerifyMaskParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT, MAX_BUFFER_SIZE, l_txVerifyMaskParam.PACKET_FORMAT);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_txVerifyMaskParam.GUARD_INTERVAL);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11ac_MiMo] CheckTxMaskParameters() Confirmed.\n");
	}
	catch(char *msg)
    {
        sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
    }
    catch(...)
    {
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11ac_MiMo] Unknown Error!\n");
    }

	
	return err;
}
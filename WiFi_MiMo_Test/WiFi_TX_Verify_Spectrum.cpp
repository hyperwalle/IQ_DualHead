#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_MiMo_Test.h"
#include "WiFi_MiMo_Test_Internal.h"
#include "IQmeasure.h"
#include "math.h" 
#include "vDUT.h"
#include "StringUtil.h"

using namespace std;

// Carrier number
#define CARRIER_0    0
#define CARRIER_1    1
#define CARRIER_2    2
#define CARRIER_16   16
#define CARRIER_17   17
#define CARRIER_26   26
#define CARRIER_28   28
#define CARRIER_42   42
#define CARRIER_43   43
#define CARRIER_58   58


// This global variable is declared in WiFi_Test_Internal.cpp
extern TM_ID				g_WiFi_Test_ID;
extern vDUT_ID				g_WiFi_Dut;
extern int					g_Tester_Type;
extern int					g_Tester_Number;
extern bool					g_vDutTxActived;
extern bool					g_dutConfigChanged;
extern WIFI_RECORD_PARAM    g_RecordedParam;
extern double				g_last_TxPower_dBm_Record;  // IQlite merge; Tracy Yu ; 2012-03-31


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
    int    FREQ_MHZ;                                /*!< The center frequency (MHz). */ 
    char   BANDWIDTH[MAX_BUFFER_SIZE];              /*!< The RF bandwidth to verify Flatness. */
    char   DATA_RATE[MAX_BUFFER_SIZE];              /*!< The data rate to verify Flatness. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*!< The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];      /*!< The packet format of 11N(only). */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];				/*! The guard interval format of 11N(only). */  // IQlite merge; Tracy Yu ; 2012-03-31
    double TX_POWER_DBM;                            /*!< The output power to verify Flatness. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*!< The path loss of test system. */
    double SAMPLING_TIME_US;                        /*!< The sampling time to verify Flatness. */
	double CORRECTION_FACTOR_11B;                   /*!< The 802.11b(only) LO leakage correction factor. */

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

} l_txVerifyFlatnessReturn;
#pragma endregion

#ifndef WIN32
int initTXVerifyFlatnessMIMOContainers = InitializeTXVerifyFlatnessContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Spectrum.cpp
int CheckTxSpectrumParameters( int *channel,
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


WIFI_MIMO_TEST_API int WiFi_TX_Verify_Spectrum(void)
{
    int				err = ERR_OK;
    bool			analysisOK = false, captureOK  = false; 
    int				channel = 0, HT40ModeOn = 0, packetFormat = 0;
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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID or WiFi_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		}
		
		TM_ClearReturns(g_WiFi_Test_ID);

		/*--------------------------*
		 * Get mandatory parameters *
		 *--------------------------*/
		err = GetInputParameters(l_txVerifyFlatnessParamMap);
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
        //Check connection between VSAMaster and antenna port
		//err = CheckConnectionVsaMasterAndAntennaPort(l_txVerifyFlatnessParamMap);
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
        err = CheckConnectionVsaAndAntennaPort(l_txVerifyFlatnessParamMap, vsaMappingTx, vsaPortStatus);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Mapping VSA to antenna port failed\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Mapping VSA to antenna port OK.\n");
		}

		txEnabled[0] = (l_txVerifyFlatnessParam.TX1==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[1] = (l_txVerifyFlatnessParam.TX2==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[2] = (l_txVerifyFlatnessParam.TX3==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[3] = (l_txVerifyFlatnessParam.TX4==0?Dut_Chain_Disabled:Dut_Chain_Enabled);

        validVsaNum = txEnabled[0] + txEnabled[1] + txEnabled[2] + txEnabled[3];

#pragma region Prepare input parameters

		//According Tx enabled status which VSA connect to.
		//By order pass into "CheckTxSpectrumParameters()
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

		err = CheckTxSpectrumParameters( &channel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &vsaEnabled[0], &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxSpectrumParameters() return OK.\n");
		}

        if (0==strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, "MIXED"))
            packetFormat = 1;
        else if (0 == strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, "GREENFIELD"))
            packetFormat = 2; 
        else
            packetFormat = 0;

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
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txVerifyFlatnessParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyFlatnessParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyFlatnessParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode 
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txVerifyFlatnessParam.FREQ_MHZ);
		}		  
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txVerifyFlatnessParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txVerifyFlatnessParam.PACKET_FORMAT_11N);
		// IQlite merge; Tracy Yu ; 2012-03-30
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_txVerifyFlatnessParam.GUARD_INTERVAL_11N);  // IQlite merge; Tracy Yu ; 2012-03-31
		//vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txVerifyFlatnessParam.TX_POWER_DBM);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txVerifyFlatnessParam.DATA_RATE);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txVerifyFlatnessParam.BANDWIDTH);   // IQlite merge; Tracy Yu ; 2012-03-31
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txVerifyFlatnessParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txVerifyFlatnessParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txVerifyFlatnessParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txVerifyFlatnessParam.TX4);
        vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs);

		// IQlite merge; Tracy Yu ; 2012-03-31
		if ( (l_txVerifyFlatnessParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
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
				l_txVerifyFlatnessParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txVerifyFlatnessParam.TX_POWER_DBM;
			}
			else
			{
				l_txVerifyFlatnessParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM", l_txVerifyFlatnessParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", l_txVerifyFlatnessParam.TX_POWER_DBM);
		// IQlite merge; Tracy Yu ; 2012-03-31; End5

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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to set VSA amplitude tolerance in dB.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsaAmplitudeTolerance(%.3f) return OK.\n", g_globalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		}

		/*err = ::LP_SetVsa(  l_txVerifyFlatnessParam.FREQ_MHZ*1e6,
							l_txVerifyFlatnessParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio,
							g_globalSettingParam.VSA_PORT,
							0,
							g_globalSettingParam.VSA_TRIGGER_LEVEL_DB,
							g_globalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000	
						  );*/
		for(int i = 0; i<g_Tester_Number;i++)
		{
			//if(l_txVerifyFlatnessParam.CABLE_LOSS_DB[i] >=0)
			//{
				rfAmplDb[i] = l_txVerifyFlatnessParam.TX_POWER_DBM-l_txVerifyFlatnessParam.CABLE_LOSS_DB[i]+peakToAvgRatio;
			//}
			//else
			//{
			//	rfAmplDb[i] = l_txVerifyFlatnessParam.TX_POWER_DBM-l_txVerifyFlatnessParam.CABLE_LOSS_DB[0]+peakToAvgRatio;
			//}
			vsgRFEnabled[i] = 0; //Turn Off VSG

		}
		err = ::LP_SetVsaNxN( l_txVerifyFlatnessParam.FREQ_MHZ*1e6,
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
		//Tracy Yu; Move to the front of this function, to enable loop back test ;2012-04-06
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


	   /*----------------------------*
		* Perform Normal VSA capture *
		*----------------------------*/
		double sampleFreqHz = 80e6;

		/*------------------------------------------------------------*/
		/*For Spectrum Analysis, in HT20/HT40, using normal capture---*/
		/*------------------------------------------------------------*/
		HT40ModeOn = 0;

		err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_globalSettingParam.VSA_TRIGGER_TYPE, sampleFreqHz, HT40ModeOn );     
		if ( ERR_OK!=err )
		{
			double rxAmpl;
			LP_Agc(&rxAmpl, TRUE);	// do auto range on all testers
			err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_globalSettingParam.VSA_TRIGGER_TYPE, sampleFreqHz, HT40ModeOn );     
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
#pragma endregion

		/*--------------*
		 *  Capture OK  *
		 *--------------*/
		captureOK = true;
		if (1==g_globalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)	// 1: Turn On
		{
			// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_SaveAlways", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
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
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txVerifyFlatnessParam.FREQ_MHZ, l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_AnalyzeFFT() return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_AnalyzeFFT() return OK.\n");
					}
				}
				else
				{
					// First, Check 11B correction factor  
					if (0==l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B)
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Need correction factor (non-zero) for 802.11B LO leakage measurement.\n");
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
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211b() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211b() return OK.\n");
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
				        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Can not retrieve FFT result, result length = 0.\n");
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
				        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Can not find out the peak power.\n");
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
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to find out the offset zero of the FFT results.\n");
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
					        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to retrieve 11B loLeakageDb.\n");
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
				        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to retrieve 11B loLeakageDb.\n");
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

				if (0==strcmp(l_txVerifyFlatnessParam.BANDWIDTH,"HALF") || 0==strcmp(l_txVerifyFlatnessParam.BANDWIDTH,"QUAR"))
				{
					err = ::LP_Analyze80211p(  g_globalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
						g_globalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
						g_globalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
						g_globalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
						g_globalSettingParam.ANALYSIS_11AG_AMPL_TRACK,
						(0==strcmp(l_txVerifyFlatnessParam.BANDWIDTH,"HALF"))?IQV_OFDM_ASTM_DSRC:IQV_OFDM_QUARTER_RATE
						);
					if ( ERR_OK!=err )
					{
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
					err = ::LP_Analyze80211ag(  g_globalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
												g_globalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
												g_globalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
												g_globalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
												g_globalSettingParam.ANALYSIS_11AG_AMPL_TRACK
												);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211ag() return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211ag() return OK.\n");
					}
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
			        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LoResultSpectralFlatness11AG() return error.\n");
			        throw logMessage;
		        }
		        else
		        {
			        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LoResultSpectralFlatness11AG() return OK.\n");
		        }

				#pragma endregion			
			}

		}

        else if ( (wifiMode==WIFI_11N_HT20)||(wifiMode==WIFI_11N_HT40) )                         
        {   
			// [Case 03]: MIMO Analysis
			#pragma region Analysis_802_11n	
			char referenceFileName[MAX_BUFFER_SIZE] = {'\0'}, analyzeMimoType[MAX_BUFFER_SIZE] = {'\0'}, analyzeMimoMode[MAX_BUFFER_SIZE] = {'\0'};

	        switch( g_Tester_Type )
	        {
				//case IQ_View:
				//	sprintf_s(analyzeMimoType, "EWC");                        
				//	if ( wifiStreamNum==WIFI_ONE_STREAM )  // MCS0 ~ MCS7
				//	{        
				//		sprintf_s(analyzeMimoMode, "nxn");
				//		sprintf_s(referenceFileName, ""); 
				//	}
				//	else // MCS8 ~ MCS15 or more, need MIMO reference File *.ref
				//	{
				//		sprintf_s(analyzeMimoMode, "composite");
				//		err = GetWaveformFileName("EVM", 
				//			"REFERENCE_FILE_NAME", 
				//			wifiMode, 
				//			l_txVerifyFlatnessParam.BANDWIDTH, 
				//			l_txVerifyFlatnessParam.DATA_RATE, 
				//			l_txVerifyFlatnessParam.PREAMBLE, 
				//			l_txVerifyFlatnessParam.PACKET_FORMAT_11N, 
				//			referenceFileName, 
				//			MAX_BUFFER_SIZE);
				//		if ( ERR_OK!=err )
				//		{
				//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get reference file name.\n");
				//			throw logMessage;
				//		}
				//		else
				//		{
				//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] GetWaveformFileName(REFERENCE_FILE_NAME) return OK.\n");
				//		}
				//	}
				//	break;

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

			//Set Vsa Number
			if(!strcmp(l_txVerifyFlatnessParam.TX_ANALYSIS_ORDER,"-1,-1,-1,-1"))
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
		        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_Analyze80211n() return error.\n");
		        throw logMessage;
	        }
	        else
	        {
		        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_Analyze80211n() return OK.\n");
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
		            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LoResultSpectralFlatness11N() return error.\n");
		            throw logMessage;
	            }
	            else
	            {
		            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LoResultSpectralFlatness11N() return OK.\n");
	            }
	        }

			#pragma endregion

        }
		else
		{
           
		}

#pragma region Retrieve analysis Results
//
//
//    #pragma region Analysis_802_11b
//	        if ( wifiMode==WIFI_11B )                            
//	        {   // [Case 01]: 802.11b Analysis
//		        if ( (0==strcmp(l_txVerifyFlatnessParam.DATA_RATE, "DSSS-2")) && 
//			         (1==g_globalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE) )
//		        {
//			        // Store the result
//			        double bufferRealX[MAX_BUFFER_SIZE], bufferImagX[MAX_BUFFER_SIZE];
//			        double bufferRealY[MAX_BUFFER_SIZE], bufferImagY[MAX_BUFFER_SIZE];
//			        int    bufferSizeX = ::LP_GetVectorMeasurement("x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE);
//			        int    bufferSizeY = ::LP_GetVectorMeasurement("y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE);
//
//			        if ( (0>=bufferSizeX)||(0>=bufferSizeY) )
//			        {
//				        err = -1;
//				        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Can not retrieve FFT result, result length = 0.\n");
//				        throw logMessage;
//			        }
//    				                          					
//			        /*--------------------------*
//			         *  Looking for peak power  *
//			         *--------------------------*/
//			        double peakPower = NA_NUMBER;
//			        for ( int i=0;i<bufferSizeY;i++ )
//			        {
//				        if ( bufferRealY[i]>peakPower )  
//				        {   // store the current peak power and keep searching
//					        peakPower = bufferRealY[i];
//				        }
//				        else
//				        {
//					        // keep searching...
//				        }
//			        }
//
//			        if ( NA_NUMBER>=peakPower )
//			        {
//				        err = -1;
//				        peakPower = NA_NUMBER;
//				        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Can not find out the peak power.\n");
//				        throw logMessage;
//			        }
//			        else
//			        {
//				        // Verify DC Leakage 		
//				        double deltaPower = pow(10,(bufferRealY[(int)(bufferSizeY/2)]/LOG_10)) - pow(10,(peakPower/LOG_10));
//				        if (0!=deltaPower)
//				        {
//					        l_txVerifyFlatnessReturn.LO_LEAKAGE_DB[0] =  10.0*(log10(deltaPower));					
//				        }
//				        else
//				        {
//					        err = -1;
//					        l_txVerifyFlatnessReturn.LO_LEAKAGE_DB[0] = 0;
//					        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to retrieve 11B loLeakageDb.\n");
//					        throw logMessage;
//				        }
//			        }
//		        }
//		        else
//		        {
//			        // Store the result
//			        double bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
//			        int    bufferSize = ::LP_GetVectorMeasurement("loLeakageDb", bufferReal, bufferImag, MAX_BUFFER_SIZE);
//
//			        if (bufferSize>0)
//			        {
//				        l_txVerifyFlatnessReturn.LO_LEAKAGE_DB[0] = bufferReal[0] + l_txVerifyFlatnessParam.CORRECTION_FACTOR_11B;
//			        }
//			        else
//			        {
//				        err = -1;
//				        l_txVerifyFlatnessReturn.LO_LEAKAGE_DB[0] = 0;
//				        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to retrieve 11B loLeakageDb.\n");
//				        throw logMessage;
//			        }
//		        }
//	        }
//    #pragma endregion
//    #pragma region Analysis_802_11ag
//	        else if ( wifiMode==WIFI_11AG )  
//	        {   // [Case 02]: 802.11a/g Analysis
//		        err = LoResultSpectralFlatness11AG( &l_txVerifyFlatnessReturn.CARRIER_NUMBER[0], 
//											        &l_txVerifyFlatnessReturn.MARGIN_DB[0], 
//											        &l_txVerifyFlatnessReturn.LO_LEAKAGE_DB[0],
//                                                    &l_txVerifyFlatnessReturn.VALUE_DB[0]
//											        );
//		        if ( ERR_OK!=err )
//		        {
//			        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LoResultSpectralFlatness11AG() return error.\n");
//			        throw logMessage;
//		        }
//		        else
//		        {
//			        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LoResultSpectralFlatness11AG() return OK.\n");
//		        }
//	        }
//    #pragma endregion
//    #pragma region Analysis_802_11n
//	        else if ( (wifiMode==WIFI_11N_HT20)||(wifiMode==WIFI_11N_HT40) )                         
//	        {   
//          //      // [Case 03]: MIMO Analysis(based on Stream) (original way)
//		        //for (int i=0;i<MAX_DATA_STREAM;i++)
//		        //{
//			       // err = LoResultSpectralFlatness11N( 0, 
//          //                                              wifiMode,
//										//	           &l_txVerifyFlatnessReturn.CARRIER_NUMBER[i], 
//										//	           &l_txVerifyFlatnessReturn.MARGIN_DB[i], 
//										//	           &l_txVerifyFlatnessReturn.LO_LEAKAGE_DB[i],
//          //                                             &l_txVerifyFlatnessReturn.VALUE_DB[i],
//										//	           i
//										//	           );
//			       // if ( ERR_OK!=err )
//			       // {
//				      //  LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LoResultSpectralFlatness11N() return error.\n");
//				      //  throw logMessage;
//			       // }
//			       // else
//			       // {
//				      //  LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LoResultSpectralFlatness11N() return OK.\n");
//			       // }
//		        //}
//
//                // [Case 03]: MIMO Analysis(Supported truely mimo)
//		        for (int i=0;i<validVsaNum;i++)
//		        {
//			            err = LoResultSpectralFlatness11N(  1, 
//                                                            wifiMode,
//															&l_txVerifyFlatnessReturn.SUBCARRIER_UP_A_VSA[prefOrderSignal[i]-1],
//															&l_txVerifyFlatnessReturn.SUBCARRIER_UP_B_VSA[prefOrderSignal[i]-1],
//															&l_txVerifyFlatnessReturn.SUBCARRIER_LO_A_VSA[prefOrderSignal[i]-1],
//															&l_txVerifyFlatnessReturn.SUBCARRIER_LO_B_VSA[prefOrderSignal[i]-1],
//											                &l_txVerifyFlatnessReturn.MARGIN_DB_VSA[prefOrderSignal[i]-1], 
//											                &l_txVerifyFlatnessReturn.LO_LEAKAGE_DB_VSA[prefOrderSignal[i]-1],
//															&l_txVerifyFlatnessReturn.VALUE_DB_UP_A_VSA[prefOrderSignal[i]-1],
//															&l_txVerifyFlatnessReturn.VALUE_DB_UP_B_VSA[prefOrderSignal[i]-1],
//															&l_txVerifyFlatnessReturn.VALUE_DB_LO_A_VSA[prefOrderSignal[i]-1],
//															&l_txVerifyFlatnessReturn.VALUE_DB_LO_B_VSA[prefOrderSignal[i]-1],
//											                i
//											               );
//			            if ( ERR_OK!=err )
//			            {
//				            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LoResultSpectralFlatness11N() return error.\n");
//				            throw logMessage;
//			            }
//			            else
//			            {
//				            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LoResultSpectralFlatness11N() return OK.\n");
//			            }
//		        }
//
//	        }
//
//
//    #pragma endregion 
//
#pragma endregion

// IQlite merge; Tracy Yu ; 2012-03-30
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
		// IQlite merge; Tracy Yu ; 2012-03-30 End

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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_MiMo] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
    }
    catch(...)
    {
		ReturnErrorMessage(l_txVerifyFlatnessReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
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

	rfAmplDb.clear();
	vsgRFEnabled.clear();

    return err;
}

int InitializeTXVerifyFlatnessContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txVerifyFlatnessParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    sprintf_s(l_txVerifyFlatnessParam.BANDWIDTH, MAX_BUFFER_SIZE, "%s", "HT20");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.BANDWIDTH))    // Type_Checking
    {
        setting.value = (void*)l_txVerifyFlatnessParam.BANDWIDTH;
        setting.unit        = "";
        setting.helpText    = "Channel bandwidth. Valid options: QUAR, HALF, HT20 or HT40";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    sprintf_s(l_txVerifyFlatnessParam.DATA_RATE, MAX_BUFFER_SIZE, "%s", "OFDM-54");
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

    strcpy_s(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.PACKET_FORMAT_11N))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyFlatnessParam.PACKET_FORMAT_11N;
        setting.unit        = "";
        setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

// IQlite merge; Tracy Yu ; 2012-03-30
	strcpy_s(l_txVerifyFlatnessParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
    setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyFlatnessParam.GUARD_INTERVAL_11N))    // Type_Checking
    {
		setting.value       = (void*)l_txVerifyFlatnessParam.GUARD_INTERVAL_11N;
        setting.unit        = "";
        setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	// IQlite merge; Tracy Yu ; 2012-03-30 End

    l_txVerifyFlatnessParam.FREQ_MHZ = 2412;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyFlatnessParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyFlatnessParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz";
        l_txVerifyFlatnessParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
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
        setting.helpText    = "Expected power level at DUT antenna port. -99 is for default target power.";
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

	}
    for (int i=0;i<MAX_TESTER_NUM;i++)  // IQlite merge; Tracy Yu ; 2012-03-30
    {
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

	}
    for (int i=0;i<MAX_TESTER_NUM;i++)         // IQlite merge; Tracy Yu ; 2012-03-30
    {
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

	}
    for (int i=0;i<MAX_TESTER_NUM;i++)        // IQlite merge; Tracy Yu ; 2012-03-30
    {
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

	}

	for (int i=0;i<MAX_TESTER_NUM;i++)          // IQlite merge; Tracy Yu ; 2012-03-30
    {
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
	}
    
	for (int i=0;i<MAX_TESTER_NUM;i++)            // IQlite merge; Tracy Yu ; 2012-03-30
    {
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
	}

	for (int i=0;i<MAX_TESTER_NUM;i++)         // IQlite merge; Tracy Yu ; 2012-03-30
    {
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
	}
	for (int i=0;i<MAX_TESTER_NUM;i++)        // IQlite merge; Tracy Yu ; 2012-03-30
    {
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
	}
	for (int i=0;i<MAX_TESTER_NUM;i++)             // IQlite merge; Tracy Yu ; 2012-03-30
    {
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
	}
	for (int i=0;i<MAX_TESTER_NUM;i++)          // IQlite merge; Tracy Yu ; 2012-03-30
    {
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

		#if defined(_DEBUG)		
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
		#endif
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
	if ( (wifiMode==WIFI_11N_HT20)&&(bandwidthMhz!=20) )
	{
		return -1;
	}
	else if ( (wifiMode==WIFI_11N_HT40)&&(bandwidthMhz!=40) )
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

		// IQlite merge; Tracy Yu ; 2012-03-30
		// [Roman said] Since carrier 0 value are all the same cross all tester,
		// Thus, only need to retrieve once.
		data[0] = data[0] + dataRaw[0];

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

		#if defined(_DEBUG)
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
		#endif
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
int CheckTxSpectrumParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, int *vsaEnabled, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{  
		// Checking the Input Parameters
		err = TM_WiFiConvertFrequencyToChannel(l_txVerifyFlatnessParam.FREQ_MHZ, channel);      
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyFlatnessParam.FREQ_MHZ);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		err = TM_WiFiConvertDataRateNameToIndex(l_txVerifyFlatnessParam.DATA_RATE, &dummyInt);      
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_txVerifyFlatnessParam.DATA_RATE);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		if ( 0!=strcmp(l_txVerifyFlatnessParam.BANDWIDTH, "HT20") && 0!=strcmp(l_txVerifyFlatnessParam.BANDWIDTH, "HT40")
			&& 0!=strcmp(l_txVerifyFlatnessParam.BANDWIDTH, "QUAR") && 0!=strcmp(l_txVerifyFlatnessParam.BANDWIDTH, "HALF") ) 
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown BANDWIDTH, WiFi bandwidth %s not supported.\n", l_txVerifyFlatnessParam.BANDWIDTH);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyFlatnessParam.PREAMBLE, "SHORT") && 0!=strcmp(l_txVerifyFlatnessParam.PREAMBLE, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_txVerifyFlatnessParam.PREAMBLE);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, "MIXED") && 0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, "GREENFIELD") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PACKET_FORMAT_11N, WiFi 11n packet format %s not supported.\n", l_txVerifyFlatnessParam.PACKET_FORMAT_11N);
			throw logMessage;
		}
		
		// IQlite merge; Tracy Yu ; 2012-03-30
		if ( 0!=strcmp(l_txVerifyFlatnessParam.GUARD_INTERVAL_11N, "LONG") && 0!=strcmp(l_txVerifyFlatnessParam.GUARD_INTERVAL_11N, "SHORT") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown GUARD_INTERVAL_11N, WiFi 11n guard interval format %s not supported.\n", l_txVerifyFlatnessParam.GUARD_INTERVAL_11N);
			throw logMessage;
		}
		// IQlite merge; Tracy Yu ; 2012-03-30 End

		// Convert parameter
		err = WiFiTestMode(l_txVerifyFlatnessParam.DATA_RATE, l_txVerifyFlatnessParam.BANDWIDTH, wifiMode, wifiStreamNum);
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
										l_txVerifyFlatnessParam.FREQ_MHZ,
										vsaEnabled[0],
										vsaEnabled[1],
										vsaEnabled[2],
										vsaEnabled[3],
										l_txVerifyFlatnessParam.CABLE_LOSS_DB,
										l_txVerifyFlatnessReturn.CABLE_LOSS_DB,
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
		
		// Check Capture Time 
		if (0==l_txVerifyFlatnessParam.SAMPLING_TIME_US)
		{
			if ( *wifiMode==WIFI_11B )
			{
				*samplingTimeUs = g_globalSettingParam.SPECTRUM_SMP_TM_DSSS_US;
			}
			else if ( *wifiMode==WIFI_11AG )
			{
				if (0==strcmp(l_txVerifyFlatnessParam.BANDWIDTH,"HALF")) {
					*samplingTimeUs = g_globalSettingParam.SPECTRUM_HALF_SAMPLE_INTERVAL_US;
				} else if (0==strcmp(l_txVerifyFlatnessParam.BANDWIDTH,"QUAR")) {
					*samplingTimeUs = g_globalSettingParam.SPECTRUM_QUAR_SAMPLE_INTERVAL_US;
				} else {
					*samplingTimeUs = g_globalSettingParam.SPECTRUM_SMP_TM_OFDM_US;
				}
			}
			else	// 802.11n
			{		
				// TODO
				*samplingTimeUs = g_globalSettingParam.SPECTRUM_SMP_TM_OFDM_US;
			}
		}
		else	// SAMPLING_TIME_US != 0
		{
			*samplingTimeUs = l_txVerifyFlatnessParam.SAMPLING_TIME_US;
		}
	
	
		// IQlite merge; Tracy Yu ; 2012-03-30
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
		// IQlite merge; Tracy Yu ; 2012-03-30 End

		// Check Dut configuration changed or not
		if (  g_globalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.
			  0!=strcmp(l_txVerifyFlatnessParam.BANDWIDTH, g_RecordedParam.BANDWIDTH) ||
			  0!=strcmp(l_txVerifyFlatnessParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
			  0!=strcmp(l_txVerifyFlatnessParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
			  0!=strcmp(l_txVerifyFlatnessParam.PACKET_FORMAT_11N, g_RecordedParam.PACKET_FORMAT_11N) ||
			  0!=strcmp(l_txVerifyFlatnessParam.GUARD_INTERVAL_11N, g_RecordedParam.GUARD_INTERVAL_11N) ||   // IQlite merge; Tracy Yu ; 2012-03-30
			  l_txVerifyFlatnessParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_txVerifyFlatnessParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_txVerifyFlatnessParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_txVerifyFlatnessParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  l_txVerifyFlatnessParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
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
		g_RecordedParam.FREQ_MHZ				= l_txVerifyFlatnessParam.FREQ_MHZ;	
		g_RecordedParam.POWER_DBM				= l_txVerifyFlatnessParam.TX_POWER_DBM;	

		sprintf_s(g_RecordedParam.BANDWIDTH, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.PACKET_FORMAT_11N);
		// IQlite merge; Tracy Yu ; 2012-03-30
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_txVerifyFlatnessParam.GUARD_INTERVAL_11N);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] CheckTxSpectrumParameters() Confirmed.\n");
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

// IQlite merge; Tracy Yu ; 2012-03-30
void CleanupTXVerifySpectrum()
{
	l_txVerifyFlatnessParamMap.clear();
	l_txVerifyFlatnessReturnMap.clear();
}
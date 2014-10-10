#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_MiMo_Test.h"
#include "WiFi_MiMo_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;

// These global variables are declared in WiFi_Test_Internal.cpp
extern TM_ID                 g_WiFi_Test_ID;    
extern vDUT_ID               g_WiFi_Dut;
extern bool					 g_vDutTxActived;
extern bool					 g_dutConfigChanged;
extern double				 g_last_TxPower_dBm_Record;  // IQlite merge; Tracy Yu ; 2012-03-31
extern WIFI_RECORD_PARAM     g_RecordedParam;
extern int					 g_Tester_Number;
extern bool					 g_txCalibrationEnabled;

// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_globalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyPowerParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_txVerifyPowerReturnMap;

struct tagParam
{
    // Mandatory Parameters
    int    FREQ_MHZ;                                        /*!< The center frequency (MHz). */
    char   BANDWIDTH[MAX_BUFFER_SIZE];                      /*!< The RF bandwidth to verify POWER. */
    char   DATA_RATE[MAX_BUFFER_SIZE];				        /*!< The data rate to verify POWER. */
	char   PREAMBLE[MAX_BUFFER_SIZE];                       /*!< The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];              /*!< The packet format of 11N(only). */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];				/*! The guard interval format of 11N(only). */  // IQlite merge; Tracy Yu ; 2012-03-31
    double TX_POWER_DBM;                                    /*!< The output power to verify Power. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];                  /*!< The path loss of test system. */
    double SAMPLING_TIME_US;                                /*!< The sampling time to verify Power. */ 
    
    //double T_INTERVAL;                                    /*!< This field is used to specify the interval that is used to determine if power is present or not. */
    //double MAX_POW_DIFF_DB;                               /*!< This field is used to specify the maximum power difference between packets that are expected to be detected. */

    char   VSA_CONNECTION[MAX_TESTER_NUM][MAX_BUFFER_SIZE]; /*!< The vsa connect to which antenn port. */ 
    char   VSA_PORT[MAX_TESTER_NUM][MAX_BUFFER_SIZE];		/*!< IQTester VSAs port setting. Default=PORT_LEFT. */  

    // DUT Parameters
    int    TX1;                                             /*!< DUT TX1 on/off. Default=1(on) */
    int    TX2;                                             /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                             /*!< DUT TX3 on/off. Default=0(off) */
    int    TX4;                                             /*!< DUT TX4 on/off. Default=0(off) */
} l_txVerifyPowerParam;

struct tagReturn
{   
    // POWER Test Result 
    double POWER_AVERAGE_DBM;                       /*!< (Average) Average power in dBm. */
    double POWER_AVERAGE_MAX_DBM;                   /*!< (Maximum) Average power in dBm. */
    double POWER_AVERAGE_MIN_DBM;                   /*!< (Minimum) Average power in dBm. */

    double POWER_PEAK_DBM;                          /*!< (Average) Peak power in dBm. */
    double POWER_PEAK_MAX_DBM;					    /*!< (Maximum) Peak power in dBm. */
    double POWER_PEAK_MIN_DBM;                      /*!< (Minimum) Peak power in dBm. */

    // FOR 802.11n Truely NxN

    double POWER_RMS_AVG[MAX_TESTER_NUM];           /*!< (Average) RMS average power in specified VSA with dBm. */
    double POWER_RMS_MAX[MAX_TESTER_NUM];           /*!< (Average) RMS max power in specified VSA with dBm. */
    double POWER_RMS_MIN[MAX_TESTER_NUM];           /*!< (Average) RMS min power in specified VSA with dBm. */
    double POWER_AVG_PK[MAX_TESTER_NUM];            /*!< (Average) PEAK power in specified VSA with dBm. */
    double POWER_MAX_PK[MAX_TESTER_NUM];            /*!< (Maximum) PEAK power in specified VSA with dBm. */
    double POWER_MIN_PK[MAX_TESTER_NUM];            /*!< (Minimum) PEAK power in specified VSA with dBm. */

	double POWER_RMS_AVG_ALL;                             
    double POWER_RMS_MAX_ALL;
    double POWER_RMS_MIN_ALL;
    double POWER_PK_ALL;     


    int    PM_AVERAGE;                              /*!< Number of packets for power averaging during power verification */

	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*!< The path loss of test system. */
    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txVerifyPowerReturn;
#pragma endregion

#ifndef WIN32
int initTXVerifyPowerMIMOContainers = InitializeTXVerifyPowerContainers();
#endif

//FOR AVERAGE POWER IN WHOLE N-CHAINs


// These global variables/functions only for WiFi_TX_Verify_Power.cpp
int CheckTxPowerParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, int *vsaEnabled, double *peakToAvgRatio, char* errorMsg );


//! WiFi TX Verify POWER
/*!
* Input Parameters
*
*  - Mandatory 
*      -# FREQ_MHZ (double): The center frequency (MHz)
*      -# DATA_RATE (string): The data rate to verify POWER
*      -# TX_POWER (double): The power (dBm) DUT is going to transmit at the antenna port
*
* Return Values
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/


WIFI_MIMO_TEST_API int WiFi_TX_Verify_Power(void)
{
    int				err = ERR_OK;

    bool			analysisOK = false, captureOK  = false;     
    int				avgIteration = 0;
    int				channel = 0, HT40ModeOn = 0;
    int				dummyValue = 0;
    int				wifiMode = 0, wifiStreamNum = 0;
	double			samplingTimeUs = 0, peakToAvgRatio = 0;		//, cableLossDb = 0;
	char			vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char			logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char			sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};

	int				txEnabled[MAX_CHAIN_NUM]={0};
	int				vsaMappingTx[MAX_CHAIN_NUM]={0};
    int				validVsaNum = 0;
	int				vsaPortStatus[MAX_TESTER_NUM];


    vector<double>	rfAmplDb(MAX_TESTER_NUM, NA_NUMBER);
	vector<int>		vsgRFEnabled(MAX_TESTER_NUM, (int)NA_NUMBER);


    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_txVerifyPowerReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_txVerifyPowerParamMap);
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
        RespondToQueryReturn(l_txVerifyPowerReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< double >
	 *-----------------------------------------------------------------------------------------*/
	vector< double >    powerAvEachBurst(g_globalSettingParam.PM_AVERAGE);
	vector< double >    powerPkEachBurst(g_globalSettingParam.PM_AVERAGE);

    //TODO: while analysis power in multi-tester
    vector< vector<double> > powerRMSEachBurst(MAX_TESTER_NUM, vector<double>(g_globalSettingParam.PM_AVERAGE,NA_NUMBER));
    vector< vector<double> > powerPKEachBurst(MAX_TESTER_NUM,  vector<double>(g_globalSettingParam.PM_AVERAGE,NA_NUMBER));
    
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
		err = GetInputParameters(l_txVerifyPowerParamMap);
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
		//err = CheckConnectionVsaMasterAndAntennaPort(l_txVerifyPowerParamMap);
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
        err = CheckConnectionVsaAndAntennaPort(l_txVerifyPowerParamMap, vsaMappingTx, vsaPortStatus);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Mapping VSA to antenna port failed\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Mapping VSA to antenna port OK.\n");
		}

		txEnabled[0] = (l_txVerifyPowerParam.TX1==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[1] = (l_txVerifyPowerParam.TX2==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[2] = (l_txVerifyPowerParam.TX3==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        txEnabled[3] = (l_txVerifyPowerParam.TX4==0?Dut_Chain_Disabled:Dut_Chain_Enabled);

        validVsaNum = txEnabled[0] + txEnabled[1] + txEnabled[2] + txEnabled[3];

#pragma region Prepare input parameters

		//According Tx enabled status which VSA connect to.
		//By order pass into "CheckTxPowerParameters()
		int vsaEnabled[MAX_TESTER_NUM];
		for(int i=0;i<MAX_TESTER_NUM;i++)
		{
           vsaEnabled[i] = txEnabled[vsaMappingTx[i]-1];
		   vsgRFEnabled[i] =0;
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

		err = CheckTxPowerParameters( &channel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &vsaEnabled[0], &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxPowerParameters() return OK.\n");
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
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txVerifyPowerParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txVerifyPowerParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txVerifyPowerParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode 
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txVerifyPowerParam.FREQ_MHZ);
		}		  
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txVerifyPowerParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txVerifyPowerParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_txVerifyPowerParam.GUARD_INTERVAL_11N);  // IQlite merge; Tracy Yu ; 2012-03-31
		//vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txVerifyPowerParam.TX_POWER_DBM);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txVerifyPowerParam.DATA_RATE);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txVerifyPowerParam.BANDWIDTH);   // IQlite merge; Tracy Yu ; 2012-03-31
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txVerifyPowerParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txVerifyPowerParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txVerifyPowerParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txVerifyPowerParam.TX4);
        vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs);
		
		// IQlite merge; Tracy Yu ; 2012-03-31
		if ( (l_txVerifyPowerParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
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
				l_txVerifyPowerParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txVerifyPowerParam.TX_POWER_DBM;
			}
			else
			{
				l_txVerifyPowerParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM", l_txVerifyPowerParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", l_txVerifyPowerParam.TX_POWER_DBM);
		
		// IQlite merge; Tracy Yu ; 2012-03-31 End

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

        //TODO: put tx enabled condition into vector txEnabled
       


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

		/*err = ::LP_SetVsa(  l_txVerifyPowerParam.FREQ_MHZ*1e6,
							l_txVerifyPowerParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio,
							g_globalSettingParam.VSA_PORT,
							0,
							g_globalSettingParam.VSA_TRIGGER_LEVEL_DB,
							g_globalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000	
						  );*/
		for(int i = 0; i<g_Tester_Number;i++)
		{
			//if(l_txVerifyPowerParam.CABLE_LOSS_DB[i] >=0)
			//{
				rfAmplDb[i] = l_txVerifyPowerParam.TX_POWER_DBM-l_txVerifyPowerParam.CABLE_LOSS_DB[i]+peakToAvgRatio;
			//}
			//else
			//{
			//	rfAmplDb[i] = l_txVerifyPowerParam.TX_POWER_DBM-l_txVerifyPowerParam.CABLE_LOSS_DB[0]+peakToAvgRatio;
			//}
			vsgRFEnabled[i] = 0; // Turn Off VSG

		}
		  err = ::LP_SetVsaNxN( l_txVerifyPowerParam.FREQ_MHZ*1e6,
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

		
		/*------------------------------*
		 * Start while loop for average *
		 *------------------------------*/
		avgIteration = 0;
		while ( avgIteration< g_globalSettingParam.PM_AVERAGE/*g_globalSettingParam.PM_AVERAGE*/ )
		{
			analysisOK = false;
			captureOK  = false; 

		   /*----------------------------*
			* Perform Normal VSA capture *
			*----------------------------*/
			double sampleFreqHz = 80e6;

			/*------------------------------------------------------------*/
			/*For Power Analysis, in HT20/HT40, using normal capture---*/
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
			if (1==g_globalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_SaveAlways", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}

			/*------------------*
			 *  Power Analysis  *
			 *------------------*/
			double dummy_T_INTERVAL      = 3.2;
			double dummy_MAX_POW_DIFF_DB = 15.0;  

            //TODO: power RMS and Peak AVG, specified VSA
            int vsaAnalysisIndex = 0;
   			analysisOK = true;

            while(vsaAnalysisIndex < MAX_TESTER_NUM)
            {

				/*------------------*
				*  Power Analysis  *
				*------------------*/
				err = ::LP_SetAnalysisParameterInteger("AnalyzePower", "vsaNum", vsaAnalysisIndex+1);

				err = ::LP_AnalyzePower( dummy_T_INTERVAL/1000000, dummy_MAX_POW_DIFF_DB );

				//---------------------------------------------------------------------------------
				// if Tx port is diabled and analysis is fail, then just fill invalid number
				//---------------------------------------------------------------------------------
				if (!txEnabled[vsaMappingTx[vsaAnalysisIndex]-1] && err!=ERR_OK)
				{
					powerRMSEachBurst[vsaAnalysisIndex][avgIteration]= NA_NUMBER;
					powerPKEachBurst[vsaAnalysisIndex][avgIteration]= NA_NUMBER;
					err = ERR_OK;
				}
				else
				{
					if ( ERR_OK!=err )
					{
						// Fail Analysis, thus save capture (Signal File) for debug
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_AnalyzePower() return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_AnalyzePower() return OK.\n");
					}

					/*-----------------------------*
					*  Retrieve analysis Results  *
					*-----------------------------*/
					//powerRMSEachBurst
					powerRMSEachBurst[vsaAnalysisIndex][avgIteration] = ::LP_GetScalarMeasurement("P_av_each_burst_dBm",0);
					if (powerRMSEachBurst[vsaAnalysisIndex][avgIteration]!=NA_NUMBER)
					{
						powerRMSEachBurst[vsaAnalysisIndex][avgIteration] += l_txVerifyPowerParam.CABLE_LOSS_DB[vsaAnalysisIndex];
					}

					// powerPKEachBurst  
					powerPKEachBurst[vsaAnalysisIndex][avgIteration] = ::LP_GetScalarMeasurement("P_pk_each_burst_dBm",0);
					if (powerPKEachBurst[vsaAnalysisIndex][avgIteration]!=NA_NUMBER)
					{
						powerPKEachBurst[vsaAnalysisIndex][avgIteration] += l_txVerifyPowerParam.CABLE_LOSS_DB[vsaAnalysisIndex];
					}
				}
				vsaAnalysisIndex ++;
			}

            


			//err = ::LP_AnalyzePower( dummy_T_INTERVAL/1000000, dummy_MAX_POW_DIFF_DB );
			//if ( ERR_OK!=err )
			//{
			//	// Fail Analysis, thus save capture (Signal File) for debug
			//	sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
			//	WiFiSaveSigFile(sigFileNameBuffer);
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_AnalyzePower() return error.\n");
			//	throw logMessage;
			//}
			//else
			//{
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_AnalyzePower() return OK.\n");
			//}

#pragma region Retrieve analysis Results
			/*-----------------------------*
			 *  Retrieve analysis Results  *
			 *-----------------------------*/
			avgIteration++;
			//analysisOK = true;

			// Since the limitation, we assume that all path loss value are very close.	
			//int  antSelection = 0;
			//if      (l_txVerifyPowerParam.TX1==1)	antSelection = 0;
			//else if (l_txVerifyPowerParam.TX2==1)	antSelection = 1;
			//else if (l_txVerifyPowerParam.TX3==1)	antSelection = 2;
			//else if (l_txVerifyPowerParam.TX4==1)	antSelection = 3;
			//else    							    antSelection = 0;	

			// powerAvEachBurst  
			//powerAvEachBurst[avgIteration-1] = ::LP_GetScalarMeasurement("P_av_each_burst_dBm",0);
			//if ( -99.00 >= powerAvEachBurst[avgIteration-1] )
			//{
			//	analysisOK = false;
			//	powerAvEachBurst[avgIteration-1] = NA_NUMBER;
			//	sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
			//	WiFiSaveSigFile(sigFileNameBuffer);
			//	err = -1;
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_av_each_burst_dBm) return error.\n");
			//	throw logMessage;
			//}
			//else
			//{
			//	powerAvEachBurst[avgIteration-1] = powerAvEachBurst[avgIteration-1] + l_txVerifyPowerParam.CABLE_LOSS_DB[antSelection];					
			//}

			//// powerPkEachBurst  
			//powerPkEachBurst[avgIteration-1] = ::LP_GetScalarMeasurement("P_pk_each_burst_dBm",0);
			//if ( -99.00 >= powerPkEachBurst[avgIteration-1] )
			//{
			//	analysisOK = false;
			//	powerPkEachBurst[avgIteration-1] = NA_NUMBER;
			//	sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txVerifyPowerParam.FREQ_MHZ, l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH);
			//	WiFiSaveSigFile(sigFileNameBuffer);
			//	err = -1;
			//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(P_pk_each_burst_dBm) return error.\n");
			//	throw logMessage;
			//}
			//else
			//{
			//	powerPkEachBurst[avgIteration-1] = powerPkEachBurst[avgIteration-1] + l_txVerifyPowerParam.CABLE_LOSS_DB[antSelection];
			//}
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
   //         // Justin code:
			//// Average Power test result
			//::AverageTestResult(&powerAvEachBurst[0], avgIteration, LOG_10, l_txVerifyPowerReturn.POWER_AVERAGE_DBM, l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM, l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM);

			//// Peak Power test result
			//::AverageTestResult(&powerPkEachBurst[0], avgIteration, LOG_10, l_txVerifyPowerReturn.POWER_PEAK_DBM, l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM, l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM);
//            int firstValidSignal = 0;
            double unvalidMax, unvalidMin;
            for(int i=0;i<MAX_TESTER_NUM;i++)
            {
                
                //::AverageTestResult(&powerRMSEachBurst[i][0], avgIteration, LOG_10, l_txVerifyPowerReturn.POWER_RMS_AVG[i], l_txVerifyPowerReturn.POWER_RMS_MAX[i], l_txVerifyPowerReturn.POWER_RMS_MIN[i]);
                ::AverageTestResult(&powerRMSEachBurst[i][0], avgIteration, LOG_10, l_txVerifyPowerReturn.POWER_RMS_AVG[i], l_txVerifyPowerReturn.POWER_RMS_MAX[i], l_txVerifyPowerReturn.POWER_RMS_MIN[i]);
                ::AverageTestResult(&powerPKEachBurst[i][0], avgIteration, LOG_10, l_txVerifyPowerReturn.POWER_AVG_PK[i],l_txVerifyPowerReturn.POWER_MAX_PK[i], l_txVerifyPowerReturn.POWER_MIN_PK[i]);
              //  //backward compatible 
              //  if(txEnabled[i] && !firstValidSignal)
              //  {
              //      firstValidSignal = 1;
        		    //// Average Power test result
              //      l_txVerifyPowerReturn.POWER_AVERAGE_DBM = l_txVerifyPowerReturn.POWER_RMS_AVG[i];
              //      l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM = l_txVerifyPowerReturn.POWER_RMS_MAX[i];
              //      l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM = l_txVerifyPowerReturn.POWER_RMS_MIN[i];

              //      // Peak Power test result(not directly copy new keyword, because meaning is a little different.
              //      ::AverageTestResult(&powerPKEachBurst[i][0], avgIteration, LOG_10, l_txVerifyPowerReturn.POWER_PEAK_DBM, l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM, l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM);
              //  }

            }

			//POWER_RMS_AVG_DBM_VSA_ALL
			//POWER_RMS_MIN_DBM_VSA_ALL
			//POWER_RMS_MAX_DBM_VSA_ALL

            //Avoid unvalid Vsa
            vector<double> tmpVector;
            tmpVector.clear();
            for(int i=0;i<MAX_TESTER_NUM;i++)
            {
                if(txEnabled[vsaMappingTx[i]-1])
                {
                    tmpVector.push_back(l_txVerifyPowerReturn.POWER_RMS_AVG[i]);
                }
            }
            ::AverageTestResult(&tmpVector[0], validVsaNum, Linear_Sum, l_txVerifyPowerReturn.POWER_RMS_AVG_ALL, unvalidMax, unvalidMin);
            //::AverageTestResult(&l_txVerifyPowerReturn.POWER_RMS_AVG[0], MAX_TESTER_NUM, Linear_Sum, l_txVerifyPowerReturn.POWER_RMS_AVG_ALL, unvalidMax, unvalidMin);
            tmpVector.clear();
            for(int i=0;i<MAX_TESTER_NUM;i++)
            {
                if(txEnabled[vsaMappingTx[i]-1])
                {
                    tmpVector.push_back(l_txVerifyPowerReturn.POWER_RMS_MAX[i]);
                }
            }
            ::AverageTestResult(&tmpVector[0], validVsaNum, Linear_Sum, l_txVerifyPowerReturn.POWER_RMS_MAX_ALL, unvalidMax, unvalidMin);
            //::AverageTestResult(&l_txVerifyPowerReturn.POWER_RMS_MAX[0], MAX_TESTER_NUM, Linear_Sum, unvalidAvg, l_txVerifyPowerReturn.POWER_RMS_MAX_ALL, unvalidMin);
            
            tmpVector.clear();
            for(int i=0;i<MAX_TESTER_NUM;i++)
            {
                if(txEnabled[vsaMappingTx[i]-1])
                {
                    tmpVector.push_back(l_txVerifyPowerReturn.POWER_RMS_MIN[i]);
                }
            }
            ::AverageTestResult(&tmpVector[0], validVsaNum, Linear_Sum, l_txVerifyPowerReturn.POWER_RMS_MIN_ALL, unvalidMax, unvalidMin);
            //::AverageTestResult(&l_txVerifyPowerReturn.POWER_RMS_MIN[0], MAX_TESTER_NUM, Linear_Sum, unvalidAvg, unvalidMax, l_txVerifyPowerReturn.POWER_RMS_MIN_ALL);

			//POWER_PEAK_DBM_VSA_ALL
			tmpVector.clear();   // IQlite merge; Tracy Yu ; 2012-03-31


		}
		else
		{
			// do nothing
		}

        l_txVerifyPowerReturn.PM_AVERAGE = g_globalSettingParam.PM_AVERAGE;
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
		// IQlite merge; Tracy Yu ; 2012-03-31 End
		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			sprintf_s(l_txVerifyPowerReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txVerifyPowerReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txVerifyPowerReturn.ERROR_MESSAGE, msg);

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
		ReturnErrorMessage(l_txVerifyPowerReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
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
	rfAmplDb.clear();  // IQlite merge; Tracy Yu ; 2012-03-31
	vsgRFEnabled.clear();  // IQlite merge; Tracy Yu ; 2012-03-31
	powerAvEachBurst.clear();
	powerPkEachBurst.clear();
	powerRMSEachBurst.clear();
	powerPKEachBurst.clear();  // IQlite merge; Tracy Yu ; 2012-03-31
    
    return err;
}

int InitializeTXVerifyPowerContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txVerifyPowerParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    strcpy_s(l_txVerifyPowerParam.BANDWIDTH, MAX_BUFFER_SIZE, "HT20");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.BANDWIDTH))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyPowerParam.BANDWIDTH;
        setting.unit        = "";
        setting.helpText    = "Channel bandwidth\r\nValid options: QUAR, HALF,HT20 or HT40";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txVerifyPowerParam.DATA_RATE, MAX_BUFFER_SIZE, "OFDM-54");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.DATA_RATE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyPowerParam.DATA_RATE;
        setting.unit        = "";
        setting.helpText    = "Data rate names:\r\nDSSS-1,DSSS-2,CCK-5_5,CCK-11\r\nOFDM-6,OFDM-9,OFDM-12,OFDM-18,OFDM-24,OFDM-36,OFDM-48,OFDM-54\r\nMCS0, MCS15, etc.";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txVerifyPowerParam.PREAMBLE, MAX_BUFFER_SIZE, "SHORT");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.PREAMBLE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyPowerParam.PREAMBLE;
        setting.unit        = "";
        setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=SHORT.";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txVerifyPowerParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.PACKET_FORMAT_11N))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyPowerParam.PACKET_FORMAT_11N;
        setting.unit        = "";
        setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


	// IQlite merge; Tracy Yu ; 2012-03-31
	strcpy_s(l_txVerifyPowerParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
    setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.GUARD_INTERVAL_11N))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyPowerParam.GUARD_INTERVAL_11N;
        setting.unit        = "";
        setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	// IQlite merge; Tracy Yu ; 2012-03-31 End

    l_txVerifyPowerParam.FREQ_MHZ = 2412;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    //l_txVerifyPowerParam.T_INTERVAL = 3.2;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyPowerParam.T_INTERVAL))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_txVerifyPowerParam.T_INTERVAL;
    //    setting.unit        = "uS";
    //    setting.helpText    = "Capture time in micro-seconds";
    //    l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("T_INTERVAL", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    //l_txVerifyPowerParam.MAX_POW_DIFF_DB = 15.0;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyPowerParam.MAX_POW_DIFF_DB))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_txVerifyPowerParam.MAX_POW_DIFF_DB;
    //    setting.unit        = "dBm";
    //    setting.helpText    = "TThe maximum power difference between packets that are expected to be detected.";
    //    l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("MAX_POW_DIFF_DB", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    l_txVerifyPowerParam.TX_POWER_DBM = 15.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyPowerParam.TX_POWER_DBM))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyPowerParam.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port. -99 is for default target power.";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.SAMPLING_TIME_US = 0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txVerifyPowerParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyPowerParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyPowerParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyPowerParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txVerifyPowerParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    

    l_txVerifyPowerParam.TX1 = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX1))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.TX1;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.TX2 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX2))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.TX2;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.TX3 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX3))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.TX3;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txVerifyPowerParam.TX4 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txVerifyPowerParam.TX4))    // Type_Checking
    {
        setting.value       = (void*)&l_txVerifyPowerParam.TX4;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_txVerifyPowerParam.VSA_CONNECTION[i], MAX_BUFFER_SIZE, "TX%d", i+1);
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.VSA_CONNECTION[i]))    // Type_Checking
        {
            setting.value       = (void*)l_txVerifyPowerParam.VSA_CONNECTION[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the actual connection between DUT antenna ports and VSAs port.\r\nValid value is TX1, TX2, TX3, TX4 and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSA%d_CONNECTION", i+1);
            l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }

    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_txVerifyPowerParam.VSA_PORT[i], MAX_BUFFER_SIZE, "");
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerParam.VSA_PORT[i]))    // Type_Checking
        {
            setting.value       = (void*)l_txVerifyPowerParam.VSA_PORT[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the VSA port, Default is used global setting.\r\nValid value is Left, Right and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSA%d_PORT", i+1);
            l_txVerifyPowerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
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
    l_txVerifyPowerReturnMap.clear();

    //l_txVerifyPowerReturn.POWER_AVERAGE_DBM = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_AVERAGE_DBM))    // Type_Checking
    //{
    //    setting.value = (void*)&l_txVerifyPowerReturn.POWER_AVERAGE_DBM;
    //    setting.unit        = "dBm";
    //    setting.helpText    = "Average power of each burst in dBm.";
    //    l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVERAGE_DBM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}
    
    //l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM))    // Type_Checking
    //{
    //    setting.value = (void*)&l_txVerifyPowerReturn.POWER_AVERAGE_MAX_DBM;
    //    setting.unit        = "dBm";
    //    setting.helpText    = "(Maximum) Average power of each burst in dBm.";
    //    l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVERAGE_MAX_DBM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}    

    //l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM))    // Type_Checking
    //{
    //    setting.value = (void*)&l_txVerifyPowerReturn.POWER_AVERAGE_MIN_DBM;
    //    setting.unit        = "dBm";
    //    setting.helpText    = "(Minimum) Average power of each burst in dBm.";
    //    l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVERAGE_MIN_DBM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}    

    //l_txVerifyPowerReturn.POWER_PEAK_DBM = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_PEAK_DBM))    // Type_Checking
    //{
    //    setting.value = (void*)&l_txVerifyPowerReturn.POWER_PEAK_DBM;
    //    setting.unit        = "dBm";
    //    setting.helpText    = "Peak power of each burst in dBm.";
    //    l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_PEAK_DBM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}
    
    //l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM))    // Type_Checking
    //{
    //    setting.value = (void*)&l_txVerifyPowerReturn.POWER_PEAK_MAX_DBM;
    //    setting.unit        = "dBm";
    //    setting.helpText    = "(Maximum) Peak power of each burst in dBm.";
    //    l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_PEAK_MAX_DBM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}    

    //l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM))    // Type_Checking
    //{
    //    setting.value = (void*)&l_txVerifyPowerReturn.POWER_PEAK_MIN_DBM;
    //    setting.unit        = "dBm";
    //    setting.helpText    = "(Minimum) Peak power of each burst in dBm.";
    //    l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_PEAK_MIN_DBM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}    

    //l_txVerifyPowerReturn.POWER_PK_ALL = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_PK_ALL))    // Type_Checking
    //{
    //    setting.value = (void*)&l_txVerifyPowerReturn.POWER_PK_ALL;
    //    setting.unit        = "dBm";
    //    setting.helpText    = "(Average) PEAK average power in whole NChains with dBm.";
    //    l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM_PEAK_VSA_ALL", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //} 

    //l_txVerifyPowerReturn.POWER_PK_MAX_DBM = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_PK_MAX_DBM))    // Type_Checking
    //{
    //    setting.value = (void*)&l_txVerifyPowerReturn.POWER_PK_MAX_DBM;
    //    setting.unit        = "dBm";
    //    setting.helpText    = "(Average) PEAK max power in whole NChains with dBm.";
    //    l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_PK_MAX_DBM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //} 

    //l_txVerifyPowerReturn.POWER_PK_MIN_DBM = NA_DOUBLE;
    //setting.type = WIFI_SETTING_TYPE_DOUBLE;
    //if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_PK_MIN_DBM))    // Type_Checking
    //{
    //    setting.value = (void*)&l_txVerifyPowerReturn.POWER_PK_MIN_DBM;
    //    setting.unit        = "dBm";
    //    setting.helpText    = "(Average) PEAK min power in whole NChains with dBm.";
    //    l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_PK_MIN_DBM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    for (int i=0;i<MAX_TESTER_NUM;i++)
    {
        l_txVerifyPowerReturn.POWER_RMS_AVG[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_RMS_AVG[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyPowerReturn.POWER_RMS_AVG[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_DBM_RMS_AVG_VSA%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(Average) RMS power in specified VSA with dBm.";
            l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_TESTER_NUM;i++)
    {
        l_txVerifyPowerReturn.POWER_RMS_MAX[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_RMS_MAX[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyPowerReturn.POWER_RMS_MAX[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_DBM_RMS_MAX_VSA%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(Maximum) RMS power in specified VSA with dBm.";
            l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_TESTER_NUM;i++)
    {
        l_txVerifyPowerReturn.POWER_RMS_MIN[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_RMS_MIN[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyPowerReturn.POWER_RMS_MIN[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_DBM_RMS_MIN_VSA%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(Minimum) RMS power in specified VSA with dBm.";
            l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_TESTER_NUM;i++)
    {
        l_txVerifyPowerReturn.POWER_AVG_PK[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_AVG_PK[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyPowerReturn.POWER_AVG_PK[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_DBM_PEAK_AVG_VSA%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(Average) Peak power in specified VSA with dBm.";
            l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }
        
	for (int i=0;i<MAX_TESTER_NUM;i++)
    {
        l_txVerifyPowerReturn.POWER_MAX_PK[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_MAX_PK[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyPowerReturn.POWER_MAX_PK[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_DBM_PEAK_MAX_VSA%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(Maximum) Peak power in specified VSA with dBm.";
            l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

	for (int i=0;i<MAX_TESTER_NUM;i++)
    {
        l_txVerifyPowerReturn.POWER_MIN_PK[i] = NA_DOUBLE;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_MIN_PK[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txVerifyPowerReturn.POWER_MIN_PK[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_DBM_PEAK_MIN_VSA%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "(Minimum) Peak power in specified VSA with dBm.";
            l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }
 //	l_txVerifyPowerReturn.POWER_RMS_AVG_ALL = NA_DOUBLE;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_RMS_AVG_ALL))    // Type_Checking
	//{
	//	setting.value = (void*)&l_txVerifyPowerReturn.POWER_RMS_AVG_ALL;
	//	setting.unit        = "dBm";
	//	setting.helpText    = "(All) Average RMS power in whole VSAs";
	//	l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM_RMS_AVG_VSA_ALL", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}
	//l_txVerifyPowerReturn.POWER_RMS_MAX_ALL = NA_DOUBLE;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_RMS_MAX_ALL))    // Type_Checking
	//{
	//	setting.value = (void*)&l_txVerifyPowerReturn.POWER_RMS_MAX_ALL;
	//	setting.unit        = "dBm";
	//	setting.helpText    = "(All) Minimum RMS power in whole VSAs";
	//	l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM_RMS_MAX_VSA_ALL", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}

	//l_txVerifyPowerReturn.POWER_RMS_MIN_ALL = NA_DOUBLE;
	//setting.type = WIFI_SETTING_TYPE_DOUBLE;
	//if (sizeof(double)==sizeof(l_txVerifyPowerReturn.POWER_RMS_MIN_ALL))    // Type_Checking
	//{
	//	setting.value = (void*)&l_txVerifyPowerReturn.POWER_RMS_MIN_ALL;
	//	setting.unit        = "dBm";
	//	setting.helpText    = "(All) Maximum RMS power in whole VSAs";
	//	l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM_RMS_MIN_VSA_ALL", setting) );
	//}
	//else    
	//{
	//	printf("Parameter Type Error!\n");
	//	exit(1);
	//}




    setting.type = WIFI_SETTING_TYPE_INTEGER;
    l_txVerifyPowerReturn.PM_AVERAGE = g_globalSettingParam.PM_AVERAGE;
    if (sizeof(int)==sizeof(l_txVerifyPowerReturn.PM_AVERAGE))    // Type_Checking
    {
        setting.value = (void*)&l_txVerifyPowerReturn.PM_AVERAGE;
        setting.unit  = "";
        setting.helpText = "Averaging times used for Power measurement.\r\nDefault value is 3";
        l_txVerifyPowerReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("NUMBER_OF_AVG", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txVerifyPowerReturn.CABLE_LOSS_DB[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txVerifyPowerReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txVerifyPowerReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_VSA%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txVerifyPowerReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txVerifyPowerReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txVerifyPowerReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_txVerifyPowerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}


//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
// 
//-------------------------------------------------------------------------------------
int CheckTxPowerParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, int *vsaEnabled, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Checking the Input Parameters
		err = TM_WiFiConvertFrequencyToChannel(l_txVerifyPowerParam.FREQ_MHZ, channel);      
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_txVerifyPowerParam.FREQ_MHZ);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		err = TM_WiFiConvertDataRateNameToIndex(l_txVerifyPowerParam.DATA_RATE, &dummyInt);      
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_txVerifyPowerParam.DATA_RATE);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		if ( 0!=strcmp(l_txVerifyPowerParam.BANDWIDTH, "HT20") && 0!=strcmp(l_txVerifyPowerParam.BANDWIDTH, "HT40")
			&& 0!=strcmp(l_txVerifyPowerParam.BANDWIDTH, "QUAR") && 0!=strcmp(l_txVerifyPowerParam.BANDWIDTH, "HALF") ) 
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown BANDWIDTH, WiFi bandwidth %s not supported.\n", l_txVerifyPowerParam.BANDWIDTH);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyPowerParam.PREAMBLE, "SHORT") && 0!=strcmp(l_txVerifyPowerParam.PREAMBLE, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_txVerifyPowerParam.PREAMBLE);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txVerifyPowerParam.PACKET_FORMAT_11N, "MIXED") && 0!=strcmp(l_txVerifyPowerParam.PACKET_FORMAT_11N, "GREENFIELD") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PACKET_FORMAT_11N, WiFi 11n packet format %s not supported.\n", l_txVerifyPowerParam.PACKET_FORMAT_11N);
			throw logMessage;
		}
		
		// IQlite merge; Tracy Yu ; 2012-03-31
		if ( 0!=strcmp(l_txVerifyPowerParam.GUARD_INTERVAL_11N, "LONG") && 0!=strcmp(l_txVerifyPowerParam.GUARD_INTERVAL_11N, "SHORT") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown GUARD_INTERVAL_11N, WiFi 11n guard interval format %s not supported.\n", l_txVerifyPowerParam.GUARD_INTERVAL_11N);
			throw logMessage;
		}
		// IQlite merge; Tracy Yu ; 2012-03-31 End


		// Convert parameter
		err = WiFiTestMode(l_txVerifyPowerParam.DATA_RATE, l_txVerifyPowerParam.BANDWIDTH, wifiMode, wifiStreamNum);
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
										l_txVerifyPowerParam.FREQ_MHZ,
										vsaEnabled[0],
										vsaEnabled[1],
										vsaEnabled[2],
										vsaEnabled[3],
										l_txVerifyPowerParam.CABLE_LOSS_DB,
										l_txVerifyPowerReturn.CABLE_LOSS_DB,
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
		if (0==l_txVerifyPowerParam.SAMPLING_TIME_US)
		{
			if ( *wifiMode==WIFI_11B )
			{
				*samplingTimeUs = g_globalSettingParam.PM_TRIGGER_DSSS_US;
			}
			else if ( *wifiMode==WIFI_11AG )
			{
				if (0==strcmp(l_txVerifyPowerParam.BANDWIDTH,"HALF")) {
					*samplingTimeUs = g_globalSettingParam.PM_HALF_SAMPLE_INTERVAL_US;
				} else if (0==strcmp(l_txVerifyPowerParam.BANDWIDTH,"QUAR")) {
					*samplingTimeUs = g_globalSettingParam.PM_QUAR_SAMPLE_INTERVAL_US;
				} else {
					*samplingTimeUs = g_globalSettingParam.PM_TRIGGER_OFDM_US;
				}
			}
			else	// 802.11n
			{		
				// TODO
				*samplingTimeUs = g_globalSettingParam.PM_TRIGGER_OFDM_US;
			}
		}
		else	// SAMPLING_TIME_US != 0
		{
			*samplingTimeUs = l_txVerifyPowerParam.SAMPLING_TIME_US;
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
		// IQlite merge; Tracy Yu ; 2012-03-31 End

		// Check Dut configuration changed or not
		if (  g_globalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.
			  0!=strcmp(l_txVerifyPowerParam.BANDWIDTH, g_RecordedParam.BANDWIDTH) ||
			  0!=strcmp(l_txVerifyPowerParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
			  0!=strcmp(l_txVerifyPowerParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
			  0!=strcmp(l_txVerifyPowerParam.PACKET_FORMAT_11N, g_RecordedParam.PACKET_FORMAT_11N) ||
			  0!=strcmp(l_txVerifyPowerParam.GUARD_INTERVAL_11N, g_RecordedParam.GUARD_INTERVAL_11N) ||  // IQlite merge; Tracy Yu ; 2012-03-31
			  l_txVerifyPowerParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_txVerifyPowerParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_txVerifyPowerParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_txVerifyPowerParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  l_txVerifyPowerParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
			  l_txVerifyPowerParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
			  l_txVerifyPowerParam.TX1!=g_RecordedParam.ANT1 ||
			  l_txVerifyPowerParam.TX2!=g_RecordedParam.ANT2 ||
			  l_txVerifyPowerParam.TX3!=g_RecordedParam.ANT3 ||
			  l_txVerifyPowerParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;			
		}
		else
		{
			g_dutConfigChanged = false;			
		}

		// Save the current setup
		g_RecordedParam.ANT1					= l_txVerifyPowerParam.TX1;
		g_RecordedParam.ANT2					= l_txVerifyPowerParam.TX2;
		g_RecordedParam.ANT3					= l_txVerifyPowerParam.TX3;
		g_RecordedParam.ANT4					= l_txVerifyPowerParam.TX4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txVerifyPowerParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txVerifyPowerParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txVerifyPowerParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txVerifyPowerParam.CABLE_LOSS_DB[3];	
		g_RecordedParam.FREQ_MHZ				= l_txVerifyPowerParam.FREQ_MHZ;	
		g_RecordedParam.POWER_DBM				= l_txVerifyPowerParam.TX_POWER_DBM;	

		sprintf_s(g_RecordedParam.BANDWIDTH, MAX_BUFFER_SIZE, l_txVerifyPowerParam.BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_txVerifyPowerParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_txVerifyPowerParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, l_txVerifyPowerParam.PACKET_FORMAT_11N);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_txVerifyPowerParam.GUARD_INTERVAL_11N);   // IQlite merge; Tracy Yu ; 2012-03-31


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] CheckTxPowerParameters() Confirmed.\n");
	}
	catch(char *msg)
    {
        sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
    }
    catch(...)
    {
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] Unknown Error!\n");
    }

	
	return err;
}

// IQlite merge; Tracy Yu ; 2012-03-31
void CleanupTXVerifyPower()
{
	l_txVerifyPowerParamMap.clear();
	l_txVerifyPowerReturnMap.clear();
}
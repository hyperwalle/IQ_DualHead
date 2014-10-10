#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11ac_MiMo_Test.h"
#include "WiFi_11ac_MiMo_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;

// This global variable is declared in WiFi_Test_Internal.cpp
extern TM_ID      g_WiFi_Test_ID;
extern vDUT_ID    g_WiFi_Dut;
extern int		  g_Tester_Type;
extern int        g_Tester_Number;
extern bool		  g_vDutTxActived;
extern bool		  g_vDutRxActived;

// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_globalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_rxVerifyPerParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_rxVerifyPerReturnMap;

struct tagParam
{
	// Mandatory Parameters
	int    BSS_FREQ_MHZ_PRIMARY;                            /*!< For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment */
	int    BSS_FREQ_MHZ_SECONDARY;                          /*!< For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined */
	int    CH_FREQ_MHZ_PRIMARY_20MHz;                       /*!< The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ"   */
	int    CH_FREQ_MHZ;										/*!< The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used  */
	int    NUM_STREAM_11AC;							        /*!< The number of streams 11AC (Only). */
    char   BSS_BANDWIDTH[MAX_BUFFER_SIZE];                  /*!< The RF bandwidth of basic service set (BSS) to verify PER. */
	char   CH_BANDWIDTH[MAX_BUFFER_SIZE];					/*!< The RF bandwidth of channle set to verify PER. */
	//char   GUARD_INTERVAL[MAX_BUFFER_SIZE];					/*!< The guard interval for 11N and 11AC. */


	int    FRAME_COUNT;                                     /*! Number of count to send data packet */
	char   DATA_RATE[MAX_BUFFER_SIZE];                      /*! The data rate to verify PER. */
	char   PREAMBLE[MAX_BUFFER_SIZE];                       /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT[MAX_BUFFER_SIZE];					/*! The packet format of 11N and 11AC. */
	char   GUARD_INTERVAL[MAX_BUFFER_SIZE];					/*! The guard interval format of 11N/AC(only). */
    char   STANDARD[MAX_BUFFER_SIZE];				/*!< The standard parameter used for signal analysis option or to discriminate the same data rates/package formats from different standards */
	double RX_POWER_DBM;                                    /*! The output power to verify PER. */

    char   VSG_CONNECTION[MAX_TESTER_NUM][MAX_BUFFER_SIZE]; /*!< The vsg connect to which antenn port. */ 
    char   VSG_PORT[MAX_TESTER_NUM][MAX_BUFFER_SIZE];		/*!< The vsg port is right/left. */ 

    double CABLE_LOSS_DB[MAX_BUFFER_SIZE];                  /*! The path loss of test system. */


	// DUT Parameters
	int    RX1;                                     /*! DUT TX1 on/off. Default=1(on) */
	int    RX2;                                     /*! DUT TX2 on/off. Default=0(off) */
	int    RX3;                                     /*! DUT TX3 on/off. Default=0(off) */
	int    RX4;                                     /*! DUT TX4 on/off. Default=0(off) */

	// waveform paramters
	char   WAVEFORM_NAME[MAX_BUFFER_SIZE]; 						    /*! File name of waveform to verify PER */
	

} l_rxVerifyPerParam;

struct tagReturn
{
	// RX Power Level
	double RX_POWER_LEVEL[MAX_CHAIN_NUM];			/*!< POWER_LEVEL dBm for the PER test. Format: POWER LEVEL */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */

	// PER Test Result 
	int	   GOOD_PACKETS;							/*!< GOOD_PACKETS is the number of good packets that reported from Dut */
	int    TOTAL_PACKETS;							/*!< TOTAL_PACKETS is the total number of packets. */
	double PER;										/*!< PER test result on specific data stream. Format: PER[SpecificStream] */

	// RSSI Test Result 
	double RSSI_RX[MAX_CHAIN_NUM];					/*!< (Average) RSSI test result on specific chain(RX). Format: RSSI_RX[SpecificChain] */
    double RSSI;									/*!< (Average) RSSI overall test result. Format: RSSI */
	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];

	//channel list
	int    CH_FREQ_MHZ_PRIMARY_40MHz;       /*!< The center frequency (MHz) for primary 40 MHZ channel  */
	int    CH_FREQ_MHZ_PRIMARY_80MHz;       /*!< The center frequency (MHz) for primary 80 MHZ channel  */

	char   WAVEFORM_NAME[MAX_BUFFER_SIZE]; 						    /*! File name of waveform to verify PER */

} l_rxVerifyPerReturn;
#pragma endregion

#ifndef WIN32 
int initRXVerifyPerMIMOContainers = InitializeRXVerifyPerContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Power.cpp
int ConfirmRxPerParameters( int *bssBW, int * cbw,int *bssPchannel,int *bssSchannel, int *wifiMode, int *wifiStreamNum, int* vsgEnabled, char* errorMsg );

//! WiFi RX Verify PER
/*!
* Input Parameters
*
*  - Mandatory 
*      -# BSS_FREQ_MHZ_PRIMARY: For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment.
*	   -# BSS_FREQ_MHZ_SECONDARY;For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined.
*      -# DATA_RATE (string): The data rate to verify RX_PER
*      -# TX_POWER_DBM (double): The power (dBm) DUT is going to transmit at the antenna port
*	   -# CH_FREQ_MHZ_PRIMARY_20MHz: The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ".
*	   -# CH_FREQ_MHZ:The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used.
*	   -# BSS_BANDWIDTH[MAX_BUFFER_SIZE]: The RF bandwidth of basic service set (BSS) to verify RX_PER.
*	   -# CH_BANDWIDTH[MAX_BUFFER_SIZE]: The RF channel bandwidth to verify RX_PER.
*
* Return Values
*      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/


WIFI_11AC_MIMO_TEST_API int WiFi_RX_Verify_Per(void)
{
	int    err = ERR_OK;    

	int    channel = 0, VHTMode = 0;
	int    dummyValue = 0;
	int    wifiMode = 0, wifiStreamNum = 0;
	int    packetNumber = 0;
//	double cableLossDb = 0;
	double MaxVSGPowerlimit = 0;
	char   vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
    int    vsgMappingRx[MAX_CHAIN_NUM]={0};
	int    vsgPortStatus[MAX_CHAIN_NUM]={0};
	int    rxEnabled[MAX_CHAIN_NUM] = {0};
	double rfPowerLeveldBm[MAX_TESTER_NUM] = {0};

	int    bssPchannel = 0,bssSchannel = 0;
	int    bssBW = 0, cbw = 0;
//	int	   vsaEnabled[MAX_TESTER_NUM];

	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_rxVerifyPerReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_rxVerifyPerParamMap);
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
		RespondToQueryReturn(l_rxVerifyPerReturnMap);
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
		err = GetInputParameters(l_rxVerifyPerParamMap);
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

		rxEnabled[0] = (l_rxVerifyPerParam.RX1==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
        rxEnabled[1] = (l_rxVerifyPerParam.RX2==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
		rxEnabled[2] = (l_rxVerifyPerParam.RX3==0?Dut_Chain_Disabled:Dut_Chain_Enabled);
		rxEnabled[3] = (l_rxVerifyPerParam.RX4==0?Dut_Chain_Disabled:Dut_Chain_Enabled);

		//Map antenna port to Vsg number, and Get vsg port status on/off
		// vsgMappingRx[] will be set:
		// 1-4 if specified one of RX1, RX2, RX3, and RX4
		// 0   if specified OFF to "disable" VSG RF power
		// -1  if specified invalid string, or empty
		for(int i=0; i<MAX_TESTER_NUM; i++)
		{
			vsgMappingRx[i] = -1;
		}
	
        err = CheckConnectionVsgAndAntennaPort(l_rxVerifyPerParamMap, vsgMappingRx, vsgPortStatus);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi_11ac_MiMo] Mapping VSG to antenna port failed\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Mapping VSG to antenna port OK.\n");
		}

#pragma region Prepare input parameters

		//According Rx enabled status which VSG connect to.
		//By order pass into "ConfirmRxPerParameters()
		int vsgEnabled[MAX_TESTER_NUM];
		for(int i=0;i<MAX_TESTER_NUM;i++)
		{

			if( vsgMappingRx[i]-1 >= 0)
			{
				//VSG#_CONNECTION is RX1 ~ RX4
				vsgEnabled[i] = rxEnabled[vsgMappingRx[i]-1];
			}
			else
			{
				//VSG_CONNECTION is OFF, empty, or unvalid String.
				vsgEnabled[i] = 0;
			}
		}

		err = ConfirmRxPerParameters( &bssBW, &cbw, &bssPchannel,&bssSchannel,&wifiMode, &wifiStreamNum, &vsgEnabled[0], vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Prepare input parameters ConfirmRxPerParameters() return OK.\n");
		}
#pragma endregion

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to Receive  *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		vDUT_ClearParameters(g_WiFi_Dut);

		vDUT_AddIntegerParameter(g_WiFi_Dut, "BSS_FREQ_MHZ_PRIMARY",		l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "BSS_FREQ_MHZ_SECONDARY",		l_rxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ",					l_rxVerifyPerParam.CH_FREQ_MHZ);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_20MHz",	l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "NUM_STREAM_11AC",				l_rxVerifyPerParam.NUM_STREAM_11AC);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX1",							l_rxVerifyPerParam.RX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX2",							l_rxVerifyPerParam.RX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX3",							l_rxVerifyPerParam.RX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX4",							l_rxVerifyPerParam.RX4);
	
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSG1",			l_rxVerifyPerParam.CABLE_LOSS_DB[0]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSG2",			l_rxVerifyPerParam.CABLE_LOSS_DB[1]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSG3",			l_rxVerifyPerParam.CABLE_LOSS_DB[2]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "CABLE_LOSS_DB_VSG4",			l_rxVerifyPerParam.CABLE_LOSS_DB[3]);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "RX_POWER_DBM",				l_rxVerifyPerParam.RX_POWER_DBM);

		vDUT_AddStringParameter (g_WiFi_Dut, "BSS_BANDWIDTH",				l_rxVerifyPerParam.BSS_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "CH_BANDWIDTH",				l_rxVerifyPerParam.CH_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",					l_rxVerifyPerParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT",				l_rxVerifyPerParam.PACKET_FORMAT);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL",			l_rxVerifyPerParam.GUARD_INTERVAL);
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",					l_rxVerifyPerParam.PREAMBLE);
        vDUT_AddStringParameter (g_WiFi_Dut, "STANDARD",					l_rxVerifyPerParam.STANDARD);


		// the following parameters are not input parameters, why are they here?  7-16-2012 Jacky 
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_40MHz",	l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz);
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ_PRIMARY_80MHz",	l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
		if ((wifiMode== WIFI_11N_GF_HT40) || ( wifiMode== WIFI_11N_MF_HT40 ) || (wifiMode == WIFI_11AC_VHT40))
		{
			//vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_FREQ_MHZ",			l_rxVerifyPerParam.CH_FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",		l_rxVerifyPerParam.CH_FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ",		l_rxVerifyPerParam.CH_FREQ_MHZ+10);
		}
		else
		{
			//do nothing
		}		

		vDUT_AddIntegerParameter(g_WiFi_Dut, "BSS_BANDWIDTH",		bssBW);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CH_BANDWIDTH",		cbw);







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

		err = vDUT_Run(g_WiFi_Dut, "RX_SET_ANTENNA");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(RX_SET_ANTENNA) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(RX_SET_ANTENNA) return OK.\n");
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

		err = vDUT_Run(g_WiFi_Dut, "RX_SET_BW");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(RX_SET_BW) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(RX_SET_BW) return OK.\n");
		}

		err = vDUT_Run(g_WiFi_Dut, "RX_SET_DATA_RATE");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(RX_SET_DATA_RATE) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(RX_SET_DATA_RATE) return OK.\n");
		}

		err = vDUT_Run(g_WiFi_Dut, "RX_SET_FILTER");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(RX_SET_FILTER) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(RX_SET_FILTER) return OK.\n");
		}
#pragma endregion



#pragma region Setup LP Tester
		/*----------------------------*
  		 * Disable VSG output signal  *
		 *----------------------------*/
		// make sure no signal coming out VSG
		//err = ::LP_EnableVsgRF(0); // only for 1 tester
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
		//}

        /*----------------------------*/
		/*Turn off VSG if not enabled */
		/*----------------------------*/
		err = ::LP_EnableVsgRFNxN(l_rxVerifyPerParam.RX1,
			l_rxVerifyPerParam.RX2,
			l_rxVerifyPerParam.RX3,
			l_rxVerifyPerParam.RX4); // 
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to turn off VSG, LP_EnableVsgRFNxN(0) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
		}


		/*---------------------*
		* Load waveform Files *
		*---------------------*/
		char   modFile[MAX_BUFFER_SIZE];

		if ( strlen(l_rxVerifyPerParam.WAVEFORM_NAME))	// Use the waveform customer defines
		{
			sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s",g_globalSettingParam.PER_WAVEFORM_PATH, l_rxVerifyPerParam.WAVEFORM_NAME);

			// Wave file checking
			FILE *waveFile;
			fopen_s(&waveFile, modFile, "r");
			if (!waveFile)  // No such file exist
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, 
					"[WiFi_11ac_MiMo] Wrong waveform file name: %s. Please check input paramter!\n",l_rxVerifyPerParam.WAVEFORM_NAME);
				throw logMessage;
				
			}
			else
			{
				strcpy_s(l_rxVerifyPerReturn.WAVEFORM_NAME,MAX_BUFFER_SIZE, modFile);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Customer defines waveform file name: %s!\n",
					l_rxVerifyPerReturn.WAVEFORM_NAME,MAX_BUFFER_SIZE);

				fclose(waveFile);
				waveFile = NULL;
			}

		}
		else		// Use default waveform name
		{
			err = GetDefaultWaveformFileName(g_globalSettingParam.PER_WAVEFORM_PATH,
									"iqvsg", //only iqvsg file support in daytona. might need more robust way if we allow IQ2010 etc to support 11ac signal
									wifiMode,
									l_rxVerifyPerParam.NUM_STREAM_11AC,
									cbw,
									l_rxVerifyPerParam.DATA_RATE,
									l_rxVerifyPerParam.PREAMBLE,
									l_rxVerifyPerParam.PACKET_FORMAT,
									l_rxVerifyPerParam.GUARD_INTERVAL,
									modFile, 
									MAX_BUFFER_SIZE);
			strcpy_s(l_rxVerifyPerReturn.WAVEFORM_NAME,MAX_BUFFER_SIZE, modFile);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
					"[WiFi_11ac_MiMo] Fail to get waveform file name: %s, GetDefaultWaveformFileName() return error.\n",l_rxVerifyPerReturn.WAVEFORM_NAME);

				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION,
					"[WiFi_11ac_MiMo] GetDefaultWaveformFileName() return OK. Waveform file Name: %s\n",l_rxVerifyPerReturn.WAVEFORM_NAME);

			}
		}
		

		// Load the whole MOD file for continuous transmit
		err = ::LP_SetVsgModulation( modFile );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] LP_SetVsgModulation( %s ) return error.\n", modFile);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetVsgModulation( %s ) return OK.\n", modFile);
		}

		/*-------------------*
		* Setup IQTester VSG *
		*--------------------*/
		// Since the limitation, we assume that all path loss value are very close.
		int  antSelection = 0;
		if      (l_rxVerifyPerParam.RX1==1)	antSelection = 0;
		else if (l_rxVerifyPerParam.RX2==1)	antSelection = 1;
		else if (l_rxVerifyPerParam.RX3==1)	antSelection = 2;
		else if (l_rxVerifyPerParam.RX4==1)	antSelection = 3;
		else    							antSelection = 0;	

		//check VSG output power limit first
		if( wifiMode==WIFI_11B )
		{
			MaxVSGPowerlimit = g_globalSettingParam.VSG_MAX_POWER_11B;
		}
		else if( wifiMode==WIFI_11AG )
		{
			MaxVSGPowerlimit = g_globalSettingParam.VSG_MAX_POWER_11G; 
		}
		else if ( strstr (l_rxVerifyPerParam.PACKET_FORMAT, "11N"))
		{
			MaxVSGPowerlimit = g_globalSettingParam.VSG_MAX_POWER_11N; 
		}
		else	// 802.11ac
		{
			MaxVSGPowerlimit = g_globalSettingParam.VSG_MAX_POWER_11AC; 
		}

		if((l_rxVerifyPerParam.RX_POWER_DBM+l_rxVerifyPerParam.CABLE_LOSS_DB[antSelection])>MaxVSGPowerlimit)
		{
			//ERR_VSG_POWER_EXCEED_LIMIT
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", l_rxVerifyPerParam.RX_POWER_DBM+l_rxVerifyPerParam.CABLE_LOSS_DB[antSelection],MaxVSGPowerlimit);
			throw logMessage;
		}
		else
		{
			// do nothing
		}
	
		/*err = ::LP_SetVsg(l_rxVerifyPerParam.FREQ_MHZ*1e6,
						  l_rxVerifyPerParam.RX_POWER_DBM+l_rxVerifyPerParam.CABLE_LOSS_DB[antSelection],
						  g_globalSettingParam.VSG_PORT);*/

		for(int i=0;i<MAX_TESTER_NUM;i++)
		{
			// For "disabled" VSG, we need to set power level really low
			if( 0==vsgMappingRx[i] )
			{
				rfPowerLeveldBm[i] = VERY_LOW_VSG_POWER_DBM;
			}
			else if( vsgMappingRx[i]>=1 && vsgMappingRx[i]<=4 )
			{
				rfPowerLeveldBm[i] = l_rxVerifyPerParam.RX_POWER_DBM +l_rxVerifyPerParam.CABLE_LOSS_DB[i];

				if(rfPowerLeveldBm[i] < VERY_LOW_VSG_POWER_DBM)
				{
					rfPowerLeveldBm[i] = VERY_LOW_VSG_POWER_DBM;
				}
				else
				{
					//do nothing
				}
				
			}
			else
			{
				//if specified invalid string or empty in VSG#_CONNECTION
				//throw error
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi_11ac_MiMo] Mapping VSG to antenna port failed\n");
				throw logMessage;

			}
		}
		err = ::LP_SetVsgNxN(l_rxVerifyPerParam.CH_FREQ_MHZ*1e6,
								&rfPowerLeveldBm[0],
								&vsgPortStatus[0]);

		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to setup VSG, LP_SetVsg() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetVsg() return OK.\n");
		}

		/*---------------*
		*  Turn on VSG  *
		*---------------*/

		//err = ::LP_EnableVsgRF(1); // only for 1 tester
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to turn on VSG, LP_EnableVsgRF(1) return error.\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_EnableVsgRF(1) return OK.\n");
		//}
		

		//err = ::LP_EnableVsgRFNxN(vsgPortStatus[0], vsgPortStatus[1], vsgPortStatus[2], vsgPortStatus[3]); // 
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to turn on VSG, LP_EnableVsgRFNxN() return error.\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Turn on VSG LP_EnableVsgRFNxN() return OK.\n");
		//}

		/*-----------------------------*
		* Send one packet for warm up *
		*-----------------------------*/           
		err = ::LP_SetFrameCnt(1);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] VSG send out packet LP_SetFrameCnt(1) failed.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] LP_SetFrameCnt(1) return OK.\n");
		}
		// After warm up, no signal coming out from VSG
#pragma endregion

#pragma region Start RX_TEST
		/*---------------------*
		*  Do RX pre_RX frst  *
		*---------------------*/
		err = vDUT_Run(g_WiFi_Dut, "RX_PRE_RX");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(RX_PRE_RX) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(RX_PRE_RX) return OK.\n");
		}

		/*-------------------------*
		* clear RX packet counter *
		*-------------------------*/
		err = vDUT_Run(g_WiFi_Dut, "RX_CLEAR_STATS");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(RX_CLEAR_STATS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(RX_CLEAR_STATS) return OK.\n");
		}

		/*----------*
		* RX Start *
		*----------*/		
		err = vDUT_Run(g_WiFi_Dut, "RX_START");		
		if ( ERR_OK!=err )
		{	
			g_vDutRxActived = false;
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(RX_START) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			g_vDutRxActived = true;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(RX_START) return OK.\n");
		}

		// Delay for DUT settle
		if (0!=g_globalSettingParam.DUT_RX_SETTLE_TIME_MS)
		{
			Sleep(g_globalSettingParam.DUT_RX_SETTLE_TIME_MS);
		}
		else
		{
			// do nothing
		}


		/*--------------------------*
		* Send packet for PER Test *
		*--------------------------*/           
		if (0==l_rxVerifyPerParam.FRAME_COUNT)
		{
			err = GetPacketNumber(wifiMode, 
									l_rxVerifyPerParam.PACKET_FORMAT, 
									&packetNumber);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to get the number of packet for PER test.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Get the number of packet GetPacketNumber() return OK.\n");
			}
		}
		else
		{
			packetNumber = l_rxVerifyPerParam.FRAME_COUNT;
		}
		err = ::LP_SetFrameCnt(packetNumber);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] VSG send out packet LP_SetFrameCnt(%d) return error.\n", packetNumber);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] VSG send out packet LP_SetFrameCnt(%d) return OK.\n", packetNumber);
		}
#pragma endregion

		/*-----------------*
		* Wait for TxDone *
		*-----------------*/ 
		int timeOutInMs = 0, sleepTime = 50;
		while ( timeOutInMs<=(1000*(g_globalSettingParam.PER_VSG_TIMEOUT_SEC)) )
		{
			err = ::LP_TxDone();
			if (ERR_OK!=err)	// Tx not finish yet...
			{
				Sleep(sleepTime);
				timeOutInMs = timeOutInMs + sleepTime;
			}
			else	
			{	// Tx Done, then break the while loop.
				break;
			}		
		}
		if ( timeOutInMs>(1000*(g_globalSettingParam.PER_VSG_TIMEOUT_SEC)) )	// timeout
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] IQTester VSG timeout.\n");
			throw logMessage;
		}
		else
		{
			// no error, do noting.
		}

#pragma region Retrieve analysis Results
		/*--------------------*
		* Get RX PER Result  *
		*--------------------*/
		int totalPackets = 0;
		int goodPackets  = 0;
		int badPackets   = 0;

		err = ::vDUT_Run(g_WiFi_Dut, "RX_GET_STATS");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(RX_GET_STATS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(RX_GET_STATS) return OK.\n");
		}

		//TODO: RX_POWER_LEVEL for each chain
		for(int i=0;i<MAX_CHAIN_NUM;i++)
		{
			if(rxEnabled[i])
			{
				l_rxVerifyPerReturn.RX_POWER_LEVEL[i] = l_rxVerifyPerParam.RX_POWER_DBM;
			}
			else
			{
               continue;
			}
		}

		err = ::vDUT_GetIntegerReturn(g_WiFi_Dut, "GOOD_PACKETS", &goodPackets);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(GOOD_PACKETS) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(GOOD_PACKETS) return OK.\n");
		}
		if ( goodPackets<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Dut report GOOD_BITS less than 0.\n");
			throw logMessage;
		}
		else
		{
			l_rxVerifyPerReturn.GOOD_PACKETS = goodPackets;
		}

		// Using packet sent as the total packets
		totalPackets = packetNumber;	
		l_rxVerifyPerReturn.TOTAL_PACKETS = totalPackets;

		// Sometime Dut get packets from Air
		// workaround for some dut which returns more ack than packet sent
		if ( goodPackets>totalPackets )	
		{
			goodPackets = totalPackets;
			l_rxVerifyPerReturn.GOOD_PACKETS  = goodPackets;
			l_rxVerifyPerReturn.TOTAL_PACKETS = totalPackets;
		}
		else
		{
			// do nothing	
		}

		badPackets = totalPackets - goodPackets;

		if( totalPackets!=0 )
		{
			l_rxVerifyPerReturn.PER = ((double)(badPackets)/(double)(totalPackets)) * 100.0;
		}
		else	// In this case, totalPackets = 0
		{
			l_rxVerifyPerReturn.PER = 100.0;
		}	
		
		/*-----------------------
		WIFI_TEST, DUT return keyword:			
			- RSSI(double)
		WIFI_MIMO_TEST, DUT return keyword:
			- RSSI_ALL(double)
			- RSSI_RX1(double)
			- RSSI_RX2(double)
			- RSSI_RX3(double)
			- RSSI_RX4(double)
		For compatible in DUT layer:
		WIFI_MIMO_TEST will try to get return keyword defined in WIFI_TEST while can't get return keyword from DUT layer.
		-------------------------*/
        
		double rssiValue = NA_NUMBER;
		err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, "RSSI_ALL", &rssiValue); 
		if ( ERR_OK!=err )
		{
			//LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetDoubleReturn(RSSI_ALL) return error.\n");
			err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, "RSSI", &rssiValue); 
			if ( ERR_OK!=err )
			{
// 				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetDoubleReturn(RSSI) return error.\n");
// 				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetDoubleReturn(RSSI) return OK.");
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetDoubleReturn(RSSI_ALL) return OK.");
		}
		l_rxVerifyPerReturn.RSSI = (double)rssiValue;
		

		//TODO: DUT need to suport RSSI_RX1 ~ RX4
		for(int i=0;i<MAX_CHAIN_NUM;i++)
		{
			rssiValue = NA_NUMBER;
			if(rxEnabled[i])
			{
				char tmpStr[MAX_BUFFER_SIZE];
				sprintf_s(tmpStr, MAX_BUFFER_SIZE, "RSSI_RX%d", i+1);
				err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, tmpStr, &rssiValue); 
				if ( ERR_OK!=err )
				{
					//TODO: DUT need to suport RSSI_RX1 ~ RSSI_RX4
					//LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_GetIntegerReturn(RSSI) return error.\n");
					//throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetDoubleReturn(%s) return OK.", tmpStr);
				}
				l_rxVerifyPerReturn.RSSI_RX[i] = (double)rssiValue;
			}
			else
			{
				continue; 

			}
		}


#pragma endregion

		/*-----------*
		*  Rx Stop  *
		*-----------*/
		err = vDUT_Run(g_WiFi_Dut, "RX_STOP");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(RX_STOP) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			g_vDutRxActived = false;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(RX_STOP) return OK.\n");
		}

		/*-----------------------*
		*  Return Test Results  *
		*-----------------------*/
		if ( ERR_OK==err )
		{
			sprintf_s(l_rxVerifyPerReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_rxVerifyPerReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_rxVerifyPerReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_rxVerifyPerReturn.ERROR_MESSAGE, "[WiFi_11ac_MiMo] Unknown Error!\n");
		err = -1;
	}

	// This is a special case, only when some error occur before the RX_STOP. 
	// This case will take care by the error handling, but must do RX_STOP manually.
	if ( g_vDutRxActived )
	{
		vDUT_Run(g_WiFi_Dut, "RX_STOP");
	}
	else
	{
		// do nothing
	}

	return err;
}

int InitializeRXVerifyPerContainers(void)
{
	/*-----------------*
	* Input Parameters *
	*-----------------*/
	l_rxVerifyPerParamMap.clear();

	WIFI_SETTING_STRUCT setting;
	setting.unit = "";
	setting.helpText = "";


	l_rxVerifyPerParam.WAVEFORM_NAME[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.WAVEFORM_NAME))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerParam.WAVEFORM_NAME;
		setting.unit        = "";
		setting.helpText    = "File name of waveform to verify PER. if no value, use default waveform name defined in the program.";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	strcpy_s(l_rxVerifyPerParam.BSS_BANDWIDTH, MAX_BUFFER_SIZE, "BW-80");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.BSS_BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerParam.BSS_BANDWIDTH;
		setting.unit        = "MHz";
		setting.helpText    = "BSS bandwidth\r\nValid options: BW-20, BW-40, BW-80, BW-80_80 or BW-160";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_BANDWIDTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	strcpy_s(l_rxVerifyPerParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, "0");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.CH_BANDWIDTH))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerifyPerParam.CH_BANDWIDTH;
        setting.unit        = "MHz";
       setting.helpText    = "Channel bandwidth\r\nValid options:0, CBW-20, CBW-40, CBW-80, CBW-80_80 or CBW-160.\r\nFor 802.11ac, if it is zero,CH_BANDWIDTH equals as BSS_BANDWIDTH. For 802.11/a/b/g/n, it must always have value.";
        l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_BANDWIDTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_rxVerifyPerParam.DATA_RATE, MAX_BUFFER_SIZE, "MCS0");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.DATA_RATE))    // Type_Checking
	{
		setting.value = (void*)l_rxVerifyPerParam.DATA_RATE;
		setting.unit        = "";
		setting.helpText    = "Data rate names, such as DSSS-1, CCK-5_5, CCK-11, OFDM-54, MCS0, MCS15";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxVerifyPerParam.PREAMBLE, MAX_BUFFER_SIZE, "SHORT");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.PREAMBLE))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerParam.PREAMBLE;
		setting.unit        = "";
		setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=SHORT.";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxVerifyPerParam.PACKET_FORMAT, MAX_BUFFER_SIZE, PACKET_FORMAT_VHT);
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.PACKET_FORMAT))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerifyPerParam.PACKET_FORMAT;
        setting.unit        = "";
        setting.helpText    = "The packet format, VHT, HT_MF,HT_GF and NON_HT as defined in standard. Default=VHT.";
        l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_rxVerifyPerParam.STANDARD, MAX_BUFFER_SIZE, STANDARD_802_11_AC);
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.STANDARD))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerifyPerParam.STANDARD;
        setting.unit        = "";
        setting.helpText    = "Used for signal analysis option or to discriminating the same data rate or package format from different standards, taking value from 802.11ac, 802.11n, 802.11ag, 802.11b. Default = 802.11ac.";
        l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("STANDARD", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	
	strcpy_s(l_rxVerifyPerParam.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.GUARD_INTERVAL))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerParam.GUARD_INTERVAL;
		setting.unit        = "";
		setting.helpText    = "Packet Guard Interval, Long or Short, default is Long";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}



	l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY = 5520;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_PRIMARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_SECONDARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz;
		setting.unit        = "MHz";
		setting.helpText    = "The center frequency (MHz) for primary 20 MHZ channel, priority is lower than \"CH_FREQ_MHZ\".";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_20MHz", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.CH_FREQ_MHZ = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.CH_FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyPerParam.CH_FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "It is the center frequency (MHz) for channel. If it is zero,\"CH_FREQ_MHZ_PRIMARY_20MHz\" will be used for 802.11ac. \r\nFor 802.11/a/b/g/n, it must alway have value.";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_rxVerifyPerParam.NUM_STREAM_11AC = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.NUM_STREAM_11AC))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyPerParam.NUM_STREAM_11AC;
		setting.unit        = "";
		setting.helpText    = "Number of spatial streams based on 11AC spec";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("NUM_STREAM_11AC", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.FRAME_COUNT = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.FRAME_COUNT))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.FRAME_COUNT;
		setting.unit        = "";
		setting.helpText    = "Number of frame count, default=0, means using default global setting value.";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FRAME_COUNT", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_rxVerifyPerParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_rxVerifyPerParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_rxVerifyPerParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_VSG%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_rxVerifyPerParam.RX_POWER_DBM = -65.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerParam.RX_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX_POWER_DBM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX1 = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX1))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX1;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 1 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX1", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX2 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX2))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX2;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 2 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX2", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX3 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX3))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX3;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 3 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX3", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.RX4 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.RX4))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.RX4;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 4 on/off. 1:on; 0:off";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX4", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    //VSG_CONNECTION
    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_rxVerifyPerParam.VSG_CONNECTION[i], MAX_BUFFER_SIZE, "RX%d", i+1);
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.VSG_CONNECTION[i]))    // Type_Checking
        {
            setting.value       = (void*)l_rxVerifyPerParam.VSG_CONNECTION[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the actual connection between DUT antenna ports and VSGs port.\r\nValid value is RX1, RX2, RX3, RX4 and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSG%d_CONNECTION", i+1);
            l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }

    }

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
        sprintf_s(l_rxVerifyPerParam.VSG_PORT[i], MAX_BUFFER_SIZE, "");
        setting.type = WIFI_SETTING_TYPE_STRING;
        if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.VSG_PORT[i]))    // Type_Checking
        {
            setting.value       = (void*)l_rxVerifyPerParam.VSG_PORT[i];
            setting.unit        = "";
            setting.helpText    = "Indicate the VSG port, Default is used global setting.\r\nValid value is Left, Right and OFF";
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "VSG%d_PORT", i+1);
            l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
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
	l_rxVerifyPerReturnMap.clear();

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = 0;
    if (sizeof(int)==sizeof(l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 40 MHZ channel";
        l_rxVerifyPerReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_40MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = 0;
    if (sizeof(int)==sizeof(l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 80 MHZ channel";
        l_rxVerifyPerReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_80MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


	l_rxVerifyPerReturn.TOTAL_PACKETS = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerReturn.TOTAL_PACKETS))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.TOTAL_PACKETS;
		setting.unit        = "";
		setting.helpText    = " TOTAL_PACKETS is the number of total packets.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TOTAL_PACKETS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.GOOD_PACKETS = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerReturn.GOOD_PACKETS))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.GOOD_PACKETS;
		setting.unit        = "";
		setting.helpText    = " GOOD_PACKETS is the number of good packets that reported from Dut.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("GOOD_PACKETS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.PER = NA_DOUBLE;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.PER))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.PER;
		setting.unit        = "%";
		setting.helpText    = "Average PER over received packets.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PER", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.RSSI = NA_DOUBLE;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyPerReturn.RSSI))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerReturn.RSSI;
		setting.unit        = "";
		setting.helpText    = "Average RSSI over received packets.";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("RSSI_ALL", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for(int i=0;i<MAX_CHAIN_NUM;i++)
	{
		l_rxVerifyPerReturn.RSSI_RX[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_rxVerifyPerReturn.RSSI_RX[i]))    // Type_Checking
		{
			setting.value = (void*)&l_rxVerifyPerReturn.RSSI_RX[i];
			setting.unit        = "";
			setting.helpText    = "Average RSSI over received packets on each chain.";
			char tmpStr[MAX_BUFFER_SIZE];
			sprintf_s(tmpStr, MAX_BUFFER_SIZE, "RSSI_RX%d", i+1);
			l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tmpStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for(int i=0;i<MAX_CHAIN_NUM;i++)
	{
		l_rxVerifyPerReturn.RX_POWER_LEVEL[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_rxVerifyPerReturn.RX_POWER_LEVEL[i]))    // Type_Checking
		{
			setting.value = (void*)&l_rxVerifyPerReturn.RX_POWER_LEVEL[i];
			setting.unit        = "dBm";
			setting.helpText    = "RX power level for PER";
			char tmpStr[MAX_BUFFER_SIZE];
			sprintf_s(tmpStr, MAX_BUFFER_SIZE, "RX_POWER_LEVEL_DBM_RX%d", i+1);
			l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tmpStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}


	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_rxVerifyPerReturn.CABLE_LOSS_DB[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_rxVerifyPerReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_rxVerifyPerReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_VSG%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_rxVerifyPerReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerReturn.WAVEFORM_NAME[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerReturn.WAVEFORM_NAME))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerReturn.WAVEFORM_NAME;
		setting.unit        = "";
		setting.helpText    = "The waveform file used to verify PER";
		l_rxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("WAVEFORM_NAME", setting) );
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
int ConfirmRxPerParameters( int *bssBW, int * cbw,int *bssPchannel,int *bssSchannel, int *wifiMode, int *wifiStreamNum, int* vsgEnabled, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	//double local_cableLossDb;
	double *samplingTimeUs = NULL;
	try
	{
        // check common input parameters
          err = CheckCommonParameters_WiFi_11ac_MiMo(l_rxVerifyPerParamMap, 
			                                         l_rxVerifyPerReturnMap, 
												     g_WiFi_Test_ID,
												     vsgEnabled,
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

//		if ( 0!=strcmp(l_rxVerifyPerParam.PREAMBLE, "SHORT") && 0!=strcmp(l_rxVerifyPerParam.PREAMBLE, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_rxVerifyPerParam.PREAMBLE);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0!=strcmp(l_rxVerifyPerParam.GUARD_INTERVAL, "SHORT") && 0!=strcmp(l_rxVerifyPerParam.GUARD_INTERVAL, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown GUARD_INTERVAL, WiFi guard interval %s not supported.\n", l_rxVerifyPerParam.GUARD_INTERVAL);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if (( 0> l_rxVerifyPerParam.NUM_STREAM_11AC) || ( l_rxVerifyPerParam.NUM_STREAM_11AC >8))
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
//		if ( (0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "11AC_MF_HT")) && (0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "11AC_GF_HT")) &&
//			(0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "11N_MF_HT")) && (0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "11N_GF_HT")) &&
//			(0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "11AC_VHT")) &&( 0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "11AC_NON_HT"))
//			&&( 0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "LEGACY_NON_HT")))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown PACKET_FORMAT, WiFi 11ac packet format %s not supported.\n", l_rxVerifyPerParam.PACKET_FORMAT);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0 == strcmp( l_rxVerifyPerParam.PACKET_FORMAT, "11AC_NON_HT") &&
//			! strstr ( l_rxVerifyPerParam.DATA_RATE, "OFDM"))
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
//		err = TM_WiFiConvertDataRateNameToIndex(l_rxVerifyPerParam.DATA_RATE, &dummyInt);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_rxVerifyPerParam.DATA_RATE);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( 0 == strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "11AC_VHT"))  // Data rate: MCS0 ~MCS9
//		{
//			if (( 14 <= dummyInt ) && (dummyInt <= 23))
//			{
//				// Data rate is right
//				//do nothing
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] PACKET_FORMAT and DATA_RATE don't match! The data rates of VHT_11AC must be MCS0 ~ MCS9, doesn't support %s!\n", &l_rxVerifyPerParam.DATA_RATE);
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
//		if ( !strstr (l_rxVerifyPerParam.PACKET_FORMAT, "11AC"))    // Legacy signal, CH_BANDWIDTH and CH_FREQ_MHZ must have values
//		{
//			if (( l_rxVerifyPerParam.CH_BANDWIDTH <= 0) || ( l_rxVerifyPerParam.CH_FREQ_MHZ <= 0))
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
//		if ( 0!=strcmp(l_rxVerifyPerParam.BSS_BANDWIDTH, "BW-20") && 0!=strcmp(l_rxVerifyPerParam.BSS_BANDWIDTH, "BW-40") && 
//			 0!=strcmp(l_rxVerifyPerParam.BSS_BANDWIDTH, "BW-80") && 0!=strcmp(l_rxVerifyPerParam.BSS_BANDWIDTH, "BW-160") &&
//			 0!=strcmp(l_rxVerifyPerParam.BSS_BANDWIDTH, "BW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_rxVerifyPerParam.BSS_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_rxVerifyPerParam.BSS_BANDWIDTH, "BW-20"))
//			{
//				*bssBW = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_rxVerifyPerParam.BSS_BANDWIDTH, "BW-40"))
//			{
//				*bssBW = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_rxVerifyPerParam.BSS_BANDWIDTH, "BW-80"))
//			{
//				*bssBW = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_rxVerifyPerParam.BSS_BANDWIDTH, "BW-160"))
//			{
//				*bssBW = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_rxVerifyPerParam.BSS_BANDWIDTH, "BW-80_80"))
//			{
//				*bssBW = BW_80_80MHZ;
//
//				if ( l_rxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
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
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_rxVerifyPerParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//
//		}
//
//		if ( 0 != strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "0") && 
//			0!=strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "CBW-20") && 0!=strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "CBW-40") && 
//			 0!=strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "CBW-80") && 0!=strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "CBW-160") &&
//			 0!=strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "CBW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown CBW BANDWIDTH, WiFi 11ac CBW bandwidth %s not supported.\n", l_rxVerifyPerParam.CH_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "0"))
//			{
//				sprintf_s(l_rxVerifyPerParam.CH_BANDWIDTH,MAX_BUFFER_SIZE,"C%s",l_rxVerifyPerParam.BSS_BANDWIDTH);
//				*cbw = *bssBW;
//			}
//			else if ( 0 == strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "CBW-20"))
//			{
//				*cbw = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "CBW-40"))
//			{
//				*cbw = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "CBW-80"))
//			{
//				*cbw = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "CBW-160"))
//			{
//				*cbw = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_rxVerifyPerParam.CH_BANDWIDTH, "CBW-80_80"))
//			{
//				*cbw = BW_80_80MHZ;
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_rxVerifyPerParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//		}
//	
//		if ( strstr ( l_rxVerifyPerParam.PREAMBLE,"11AC"))  //802.11ac, cbw can't be larger than bssBW
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
//		if ( strstr( l_rxVerifyPerParam.PACKET_FORMAT, "11N"))  //802.11n, BW= 20,40MHz
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
//		else if ( 0 == strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "LEGACY_NON_HT"))  // 11B and 11ag
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
//			if ( 0 == strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "11AC_VHT"))
//			{
//				//do nothing
//			}
//			else if ( 0 == strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "11AC_MF_HT") ||
//				0 == strcmp(l_rxVerifyPerParam.PACKET_FORMAT, "11AC_GF_HT"))
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
//		if (l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY <= 0)
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
//		err = TM_WiFiConvertFrequencyToChannel(l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY, bssPchannel);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( *bssBW == BW_80_80MHZ)  // Need BSS_FREQ_MHZ_SECONDARY
//		{
//			if ( l_rxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
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
//			err = TM_WiFiConvertFrequencyToChannel(l_rxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY, bssSchannel);      
//			if ( ERR_OK!=err )
//			{
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_rxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY);
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
//		if ( strstr (l_rxVerifyPerParam.PACKET_FORMAT, "11AC"))
//		{
//
//			if ( 0 == l_rxVerifyPerParam.CH_FREQ_MHZ)  
//			{
//				//no CH_FREQ_MHZ, no CH_FREQ_MHZ_PRIMARY_20MHz
//				// Use default values
//				if ( 0 == l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz) 
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi_11ac_MiMo] Don't have input parameters \"CH_FREQ_MHZ\" or \"CH_FREQ_MHZ_PRIMARY_20\", using BBS center freuqency as default!");
//					l_rxVerifyPerParam.CH_FREQ_MHZ = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
//
//					// all  use lower frequency for channel list
//					switch (*bssBW)
//					{
//					case BW_20MHZ:
//						l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_40MHZ:
//						l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 10;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_80MHZ:
//						l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 20;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
//						break;
//					
//					case BW_160MHZ:			
//						l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//				
//					case BW_80_80MHZ:				
//						l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//
//					default:
//						l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					}
//
//				}
//				else  //no CH_FREQ_MHZ, have CH_FREQ_MHZ_PRIMARY_20MHz. Use input CH_FREQ_MHZ_PRIMARY_20MHz to calculate
//				{
//					err = GetChannelList(*bssBW, l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY,l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//					if ( err != ERR_OK) // Wrong channel list
//					{
//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_rxVerifyPerParam.PACKET_FORMAT);
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
//						l_rxVerifyPerParam.CH_FREQ_MHZ = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//					else if (*cbw == BW_20MHZ)
//					{
//						l_rxVerifyPerParam.CH_FREQ_MHZ = l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz;
//					}
//					else if (*cbw == BW_40MHZ)
//					{
//						l_rxVerifyPerParam.CH_FREQ_MHZ = l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
//					}
//					else if (*cbw == BW_80MHZ)
//					{
//						l_rxVerifyPerParam.CH_FREQ_MHZ = l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
//					}
//					else
//					{
//						l_rxVerifyPerParam.CH_FREQ_MHZ = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//				}
//				else // cbw = BW_80_80MHZ, use BSS primary and secondary
//				{
//					l_rxVerifyPerParam.CH_FREQ_MHZ = NA_INTEGER;
//					// do nothing
//				}
//			}
//			else  // CH_FREQ_MHZ: non-zero 
//			{
//				//Check if input CH_FREQ_MHZ is correct
//				err = CheckChannelFreq(*bssBW, *cbw,
//					l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY,l_rxVerifyPerParam.CH_FREQ_MHZ);
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
//				if ( 0 != l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz)
//				{
//					err = CheckChPrimary20(*bssBW, *cbw,
//							l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY,l_rxVerifyPerParam.CH_FREQ_MHZ,l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz);
//					if ( err == ERR_OK)  //input CH_FREQ_MHZ_PRIMARY_20MHz is correct
//					{
//						err = GetChannelList(*bssBW, l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY,l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//							&l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//						if ( err != ERR_OK) // Wrong channel list
//						{
//							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Can't get channel list. Please check input paramters.\n", l_rxVerifyPerParam.PACKET_FORMAT);
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
//						l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					case BW_40MHZ:  //cbw = BW_20.40MHz
//						if (*cbw == BW_20MHZ)
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 10;
//
//						}
//						else  //wrong cbw
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80MHZ:  //cbw = BW_20,40,80MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_rxVerifyPerParam.CH_FREQ_MHZ == l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_rxVerifyPerParam.CH_FREQ_MHZ == l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  //wrong cbw
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz =  NA_INTEGER;
//
//						}
//						break;
//					case BW_160MHZ:  //cbw = BW_20,40,80,160MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_rxVerifyPerParam.CH_FREQ_MHZ == l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 60)
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_rxVerifyPerParam.CH_FREQ_MHZ == l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_rxVerifyPerParam.CH_FREQ_MHZ == l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else if ( l_rxVerifyPerParam.CH_FREQ_MHZ == l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 60)
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 50;
//							}
//							else  //wrong CH_FREQ_MHZ
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							if (l_rxVerifyPerParam.CH_FREQ_MHZ == l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 40)
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_rxVerifyPerParam.CH_FREQ_MHZ == l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 40)
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_160MHZ)
//						{
//							if (l_rxVerifyPerParam.CH_FREQ_MHZ == l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY)
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else  //wring cbw
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80_80MHZ:
//						if ( *cbw == BW_20MHZ)
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if ( l_rxVerifyPerParam.CH_FREQ_MHZ == l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_rxVerifyPerParam.CH_FREQ_MHZ == l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else if ( *cbw == BW_80_80MHZ)
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  // wrong CH_FREQ_MHz
//						{
//							l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					default:
//						l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						break;
//					}
//				}
//
//				err = GetChannelList(*bssBW, l_rxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY,l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//				if ( err != ERR_OK)  //Get channel list wrong
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_rxVerifyPerParam.PACKET_FORMAT);
//					throw logMessage;
//				}
//				else  //Get channel list successfully
//				{
//				}
//			}	
//
//			//Check channel list. If all are "NA_INTEGER", return error
//			if (( l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz == NA_INTEGER ) && 
//				(l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz == NA_INTEGER ) &&
//				(l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER))
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
//			l_rxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//			l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//			l_rxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//		}
//#pragma endregion
//		
//		////VHT, cbw = BW_20MHZ, the stream number of MCS9 only can 2,4,6,8
//		//if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )
//		//{
//		//	if ( (l_rxVerifyPerParam.NUM_STREAM_11AC != 2) ||
//		//		(l_rxVerifyPerParam.NUM_STREAM_11AC != 4) ||
//		//		(l_rxVerifyPerParam.NUM_STREAM_11AC != 6) ||
//		//		(l_rxVerifyPerParam.NUM_STREAM_11AC != 8))
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
//			if ( (l_rxVerifyPerParam.NUM_STREAM_11AC != 3) &&
//				(l_rxVerifyPerParam.NUM_STREAM_11AC != 6) )
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
//			if ( l_rxVerifyPerParam.NUM_STREAM_11AC == 6)
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
//			if ( l_rxVerifyPerParam.NUM_STREAM_11AC == 3 || l_rxVerifyPerParam.NUM_STREAM_11AC == 7)
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
//		err = WiFi_11ac_TestMode(l_rxVerifyPerParam.DATA_RATE, cbw, wifiMode, wifiStreamNum, l_rxVerifyPerParam.PACKET_FORMAT);
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
//		//Check path loss (by ant and freq)
//		//temp: declare double *cableLossDb
//		//TODO: remove "cableLossDb" in CheckPathLossTable(), each VSA/VSG cable loss will record in CABLE_LOSS_DB of local Param 
////		double *cableLossDb = (double *) malloc (sizeof (double));
//		err = CheckPathLossTableExt(	g_WiFi_Test_ID,
//										l_rxVerifyPerParam.CH_FREQ_MHZ,
//										vsgEnabled[0],
//										vsgEnabled[1],
//										vsgEnabled[2],
//										vsgEnabled[3],
//										l_rxVerifyPerParam.CABLE_LOSS_DB,
//										l_rxVerifyPerReturn.CABLE_LOSS_DB,
//										&local_cableLossDb,
//										RX_TABLE
//								);
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to get CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] CheckPathLossTableExt() return OK.\n");
//		}

		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11ac_MiMo] ConfirmRxPerParameters() Confirmed.\n");
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
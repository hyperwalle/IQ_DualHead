#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_MiMo_Test.h"
#include "WiFi_MiMo_Test_Internal.h"
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
	int    FREQ_MHZ;                                        /*! The center frequency (MHz). */  
	int    FRAME_COUNT;                                     /*! Number of count to send data packet */
	char   BANDWIDTH[MAX_BUFFER_SIZE];                      /*! The RF bandwidth to verify EVM. */
	char   DATA_RATE[MAX_BUFFER_SIZE];                      /*! The data rate to verify EVM. */
	char   PREAMBLE[MAX_BUFFER_SIZE];                       /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];              /*! The packet format of 11N(only). */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];				/*! The guard interval format of 11N(only). */ // IQlite merge; Tracy Yu ; 2012-03-31
	double RX_POWER_DBM;                                    /*! The output power to verify EVM. */

    char   VSG_CONNECTION[MAX_TESTER_NUM][MAX_BUFFER_SIZE]; /*!< The vsg connect to which antenn port. */ 
    char   VSG_PORT[MAX_TESTER_NUM][MAX_BUFFER_SIZE];		/*!< The vsg port is right/left. */ 

    double CABLE_LOSS_DB[MAX_BUFFER_SIZE];                  /*! The path loss of test system. */


	// DUT Parameters
	int    RX1;                                     /*! DUT TX1 on/off. Default=1(on) */
	int    RX2;                                     /*! DUT TX2 on/off. Default=0(off) */
	int    RX3;                                     /*! DUT TX3 on/off. Default=0(off) */
	int    RX4;                                     /*! DUT TX4 on/off. Default=0(off) */
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
} l_rxVerifyPerReturn;
#pragma endregion

#ifndef WIN32 
int initRXVerifyPerMIMOContainers = InitializeRXVerifyPerContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Power.cpp
int ConfirmRxPerParameters( int *channel, int *wifiMode, int *wifiStreamNum, int* vsgEnabled, char* errorMsg );


//! WiFi RX Verify PER
/*!
* Input Parameters
*
*  - Mandatory 
*      -# FREQ_MHZ (double): The center frequency (MHz)
*      -# DATA_RATE (string): The data rate to verify EVM
*      -# RX_POWER (double): The power (dBm) of DUT is going to receive at the antenna port
*
* Return Values
*      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/


WIFI_MIMO_TEST_API int WiFi_RX_Verify_Per(void)
{
	int    err = ERR_OK;    

	int    channel = 0, HT40ModeOn = 0;
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
		err = GetInputParameters(l_rxVerifyPerParamMap);
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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Mapping VSG to antenna port failed\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Mapping VSG to antenna port OK.\n");
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

		err = ConfirmRxPerParameters( &channel, &wifiMode, &wifiStreamNum, &vsgEnabled[0], vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters ConfirmRxPerParameters() return OK.\n");
		}
#pragma endregion

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to Receive  *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		vDUT_ClearParameters(g_WiFi_Dut);

		if( wifiMode==WIFI_11N_HT40 )
		{
			HT40ModeOn = 1;   // 1: HT40 mode;
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_rxVerifyPerParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_rxVerifyPerParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_rxVerifyPerParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode 
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",	  l_rxVerifyPerParam.FREQ_MHZ);
		}
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);    
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_rxVerifyPerParam.BANDWIDTH);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",		  l_rxVerifyPerParam.FREQ_MHZ);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_rxVerifyPerParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_rxVerifyPerParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_rxVerifyPerParam.PACKET_FORMAT_11N);
		// IQlite merge; Tracy Yu ; 2012-03-31	
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_rxVerifyPerParam.GUARD_INTERVAL_11N);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX1",			      l_rxVerifyPerParam.RX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX2",				  l_rxVerifyPerParam.RX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX3",				  l_rxVerifyPerParam.RX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "RX4",				  l_rxVerifyPerParam.RX4);


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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_ANTENNA) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_ANTENNA) return OK.\n");
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_BW) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_BW) return OK.\n");
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_DATA_RATE) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_DATA_RATE) return OK.\n");
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_SET_FILTER) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_SET_FILTER) return OK.\n");
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
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn off VSG, LP_EnableVsgRFNxN(0) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
		}


		/*---------------------*
		* Load waveform Files *
		*---------------------*/
		char   modFile[MAX_BUFFER_SIZE];
		err = GetWaveformFileName(  "PER", 
									"WAVEFORM_NAME", 
									wifiMode, 
									l_rxVerifyPerParam.BANDWIDTH, 
									l_rxVerifyPerParam.DATA_RATE, 
									l_rxVerifyPerParam.PREAMBLE, 
									l_rxVerifyPerParam.PACKET_FORMAT_11N, 
									l_rxVerifyPerParam.GUARD_INTERVAL_11N,  // IQlite merge; Tracy Yu ; 2012-03-31
									modFile, 
									MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get waveform file name, GetWaveformFileName() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] GetWaveformFileName() return OK.\n");
		}

		// Load the whole MOD file for continuous transmit
		err = ::LP_SetVsgModulation( modFile );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SetVsgModulation( %s ) return error.\n", modFile);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsgModulation( %s ) return OK.\n", modFile);
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
		else
		{
			MaxVSGPowerlimit = g_globalSettingParam.VSG_MAX_POWER_11N; 
		}

		if((l_rxVerifyPerParam.RX_POWER_DBM+l_rxVerifyPerParam.CABLE_LOSS_DB[antSelection])>MaxVSGPowerlimit)
		{
			//ERR_VSG_POWER_EXCEED_LIMIT
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", l_rxVerifyPerParam.RX_POWER_DBM+l_rxVerifyPerParam.CABLE_LOSS_DB[antSelection],MaxVSGPowerlimit);
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Mapping VSG to antenna port failed\n");
				throw logMessage;

			}
		}
		  err = ::LP_SetVsgNxN(l_rxVerifyPerParam.FREQ_MHZ*1e6,
								&rfPowerLeveldBm[0],
								&vsgPortStatus[0]);

		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to setup VSG, LP_SetVsg() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsg() return OK.\n");
		}

		/*---------------*
		*  Turn on VSG  *
		*---------------*/

		//err = ::LP_EnableVsgRF(1); // only for 1 tester
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn on VSG, LP_EnableVsgRF(1) return error.\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_EnableVsgRF(1) return OK.\n");
		//}
		

		//err = ::LP_EnableVsgRFNxN(vsgPortStatus[0], vsgPortStatus[1], vsgPortStatus[2], vsgPortStatus[3]); // 
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn on VSG, LP_EnableVsgRFNxN() return error.\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Turn on VSG LP_EnableVsgRFNxN() return OK.\n");
		//}

		/*-----------------------------*
		* Send one packet for warm up *
		*-----------------------------*/           
		err = ::LP_SetFrameCnt(1);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] VSG send out packet LP_SetFrameCnt(1) failed.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetFrameCnt(1) return OK.\n");
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_PRE_RX) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_PRE_RX) return OK.\n");
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_CLEAR_STATS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_CLEAR_STATS) return OK.\n");
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_START) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			g_vDutRxActived = true;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_START) return OK.\n");
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

		// Added the work around below to fix IQ2010 NXN out of sync issue
		err = ::LP_SetVsg(l_rxVerifyPerParam.FREQ_MHZ*1e6, rfPowerLeveldBm[0], vsgPortStatus[0], IQV_GAP_POWER_ON);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to setup VSG, LP_SetVsg() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsg() return OK.\n");
		}
		err = ::LP_SetVsg(l_rxVerifyPerParam.FREQ_MHZ*1e6, rfPowerLeveldBm[0], vsgPortStatus[0], IQV_GAP_POWER_OFF);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to setup VSG, LP_SetVsg() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsg() return OK.\n");
		}
		//*****************************************************************************
		/*--------------------------*
		* Send packet for PER Test *
		*--------------------------*/           
		if (0==l_rxVerifyPerParam.FRAME_COUNT)
		{
			err = GetPacketNumber(wifiMode, 
									l_rxVerifyPerParam.BANDWIDTH, 
									l_rxVerifyPerParam.DATA_RATE,  
									l_rxVerifyPerParam.PACKET_FORMAT_11N, 
									l_rxVerifyPerParam.GUARD_INTERVAL_11N,  // IQlite merge; Tracy Yu ; 2012-03-31
									&packetNumber);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get the number of packet for PER test.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get the number of packet GetPacketNumber() return OK.\n");
			}
		}
		else
		{
			packetNumber = l_rxVerifyPerParam.FRAME_COUNT;
		}
		err = ::LP_SetFrameCnt(packetNumber);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] VSG send out packet LP_SetFrameCnt(%d) return error.\n", packetNumber);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] VSG send out packet LP_SetFrameCnt(%d) return OK.\n", packetNumber);
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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQTester VSG timeout.\n");
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_GET_STATS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_GET_STATS) return OK.\n");
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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetIntegerReturn(GOOD_PACKETS) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetIntegerReturn(GOOD_PACKETS) return OK.\n");
		}
		if ( goodPackets<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Dut report GOOD_BITS less than 0.\n");
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
			err = ::vDUT_GetDoubleReturn(g_WiFi_Dut, "RSSI", &rssiValue); 
			if ( ERR_OK!=err )
			{
				err = ERR_OK;	// TODO: Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_GetDoubleReturn(RSSI) return error.\n");
				throw logMessage;
			}
			else
			{
				l_rxVerifyPerReturn.RSSI = (double)rssiValue;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetDoubleReturn(RSSI) return OK.");
			}
		}
		else
		{
			l_rxVerifyPerReturn.RSSI = (double)rssiValue;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetDoubleReturn(RSSI_ALL) return OK.");
		}
		

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
					// IQlite merge; Tracy Yu ; 2012-03-31
					err = ERR_OK;	// TODO: Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] WARNING, This DUT does not support RSSI_RX%d measurement.\n", i+1 );
					//throw logMessage;
				}
				else
				{
					l_rxVerifyPerReturn.RSSI_RX[i] = (double)rssiValue;  // IQlite merge; Tracy Yu ; 2012-03-31
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetDoubleReturn(%s) return OK.", tmpStr);
				}
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_STOP) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			g_vDutRxActived = false;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_STOP) return OK.\n");
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
		ReturnErrorMessage(l_rxVerifyPerReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
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

	strcpy_s(l_rxVerifyPerParam.BANDWIDTH, MAX_BUFFER_SIZE, "HT20");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.BANDWIDTH))    // Type_Checking
	{
		setting.value = (void*)l_rxVerifyPerParam.BANDWIDTH;
		setting.unit        = "";
		setting.helpText    = "Channel bandwidth. Valid options: QUAR, HALF, HT20 or HT40";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_rxVerifyPerParam.DATA_RATE, MAX_BUFFER_SIZE, "OFDM-54");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.DATA_RATE))    // Type_Checking
	{
		setting.value = (void*)l_rxVerifyPerParam.DATA_RATE;
		setting.unit        = "";
		setting.helpText    = "Data rate names, such as DSSS-1, CCK-5_5, CCK-11, QUAR1_5, Half-27, OFDM-54, MCS0, MCS15";
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

	strcpy_s(l_rxVerifyPerParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.PACKET_FORMAT_11N))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerParam.PACKET_FORMAT_11N;
		setting.unit        = "";
		setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
 
 	// IQlite merge; Tracy Yu ; 2012-03-31
	strcpy_s(l_rxVerifyPerParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyPerParam.GUARD_INTERVAL_11N))    // Type_Checking
	{
		setting.value       = (void*)l_rxVerifyPerParam.GUARD_INTERVAL_11N;
		setting.unit        = "";
		setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyPerParam.FREQ_MHZ = 2412;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyPerParam.FREQ_MHZ))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyPerParam.FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "Channel center frequency in MHz";
		l_rxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
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

	return 0;
}


//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
// 	
//-------------------------------------------------------------------------------------
int ConfirmRxPerParameters( int *channel, int *wifiMode, int *wifiStreamNum, int* vsgEnabled, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Checking the Input Parameters
		err = TM_WiFiConvertFrequencyToChannel(l_rxVerifyPerParam.FREQ_MHZ, channel);      
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_rxVerifyPerParam.FREQ_MHZ);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}
		err = TM_WiFiConvertDataRateNameToIndex(l_rxVerifyPerParam.DATA_RATE, &dummyInt);      
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_rxVerifyPerParam.DATA_RATE);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertDataRateNameToIndex() return OK.\n");
		}
		if ( 0!=strcmp(l_rxVerifyPerParam.BANDWIDTH, "HT20") && 0!=strcmp(l_rxVerifyPerParam.BANDWIDTH, "HT40")
			&& 0!=strcmp(l_rxVerifyPerParam.BANDWIDTH, "QUAR") && 0!=strcmp(l_rxVerifyPerParam.BANDWIDTH, "HALF") ) 
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown BANDWIDTH, WiFi bandwidth %s not supported.\n", l_rxVerifyPerParam.BANDWIDTH);
			throw logMessage;
		}
		if ( 0!=strcmp(l_rxVerifyPerParam.PREAMBLE, "SHORT") && 0!=strcmp(l_rxVerifyPerParam.PREAMBLE, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_rxVerifyPerParam.PREAMBLE);
			throw logMessage;
		}
		if ( 0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT_11N, "MIXED") && 0!=strcmp(l_rxVerifyPerParam.PACKET_FORMAT_11N, "GREENFIELD") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PACKET_FORMAT_11N, WiFi 11n packet format %s not supported.\n", l_rxVerifyPerParam.PACKET_FORMAT_11N);
			throw logMessage;
		}
		
		// IQlite merge; Tracy Yu ; 2012-03-31
		if ( 0!=strcmp(l_rxVerifyPerParam.GUARD_INTERVAL_11N, "LONG") && 0!=strcmp(l_rxVerifyPerParam.GUARD_INTERVAL_11N, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown GUARD_INTERVAL_11N, WiFi 11n packet format %s not supported.\n", l_rxVerifyPerParam.GUARD_INTERVAL_11N);
			throw logMessage;
		}


		// Convert parameter
		err = WiFiTestMode(l_rxVerifyPerParam.DATA_RATE, l_rxVerifyPerParam.BANDWIDTH, wifiMode, wifiStreamNum);
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
		double *cableLossDb = (double *) malloc (sizeof (double));
		err = CheckPathLossTableExt(	g_WiFi_Test_ID,
										l_rxVerifyPerParam.FREQ_MHZ,
										vsgEnabled[0],
										vsgEnabled[1],
										vsgEnabled[2],
										vsgEnabled[3],
										l_rxVerifyPerParam.CABLE_LOSS_DB,
										l_rxVerifyPerReturn.CABLE_LOSS_DB,
										cableLossDb,
										RX_TABLE
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

		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] ConfirmRxPerParameters() Confirmed.\n");
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
void CleanupRXVerifyPer()
{
	l_rxVerifyPerParamMap.clear();
	l_rxVerifyPerReturnMap.clear();
}
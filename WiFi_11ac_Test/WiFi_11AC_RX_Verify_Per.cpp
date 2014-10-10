#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;

// This global variable is declared in WiFi_Test_Internal.cpp
extern TM_ID      g_WiFi_11ac_Test_ID;
extern vDUT_ID    g_WiFi_11ac_Dut;
extern int		  g_Tester_Type;
extern bool		  g_vDutTxActived;
extern bool		  g_vDutRxActived;

// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_WiFi11ACGlobalSettingParam;
extern int      g_Tester_DualHead;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACrxVerifyPerParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_11ACrxVerifyPerReturnMap;

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
	int    FRAME_COUNT;                             /*! Number of count to send data packet */
	char   DATA_RATE[MAX_BUFFER_SIZE];              /*! The data rate */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT[MAX_BUFFER_SIZE];					/*! The packet format of 11N and 11AC. */
	char   GUARD_INTERVAL[MAX_BUFFER_SIZE];			/*! The guard interval format of 11N/AC(only). */
	char   STANDARD[MAX_BUFFER_SIZE];				/*!< The standard parameter used for signal analysis option or to discriminate the same data rates/package formats from different standards */
	double RX_POWER_DBM;                            /*! The output power to verify PER. */

	double CABLE_LOSS_DB[MAX_BUFFER_SIZE];                  /*! The path loss of test system. */

	// DUT Parameters
	int    RX1;                                     /*! DUT TX1 on/off. Default=1(on) */
	int    RX2;                                     /*! DUT TX2 on/off. Default=0(off) */
	int    RX3;                                     /*! DUT TX3 on/off. Default=0(off) */
	int    RX4;                                     /*! DUT TX4 on/off. Default=0(off) */

	// waveform paramters
	char   WAVEFORM_NAME[MAX_BUFFER_SIZE]; 						    /*! File name of waveform to verify PER */


} l_11ACrxVerifyPerParam;

struct tagReturn
{
	// RX Power Level
	double RX_POWER_LEVEL;							/*!< POWER_LEVEL dBm for the PER test. Format: POWER LEVEL */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */

	// PER Test Result
	int	   GOOD_PACKETS;							/*!< GOOD_PACKETS is the number of good packets that reported from Dut */
	int    TOTAL_PACKETS;							/*!< TOTAL_PACKETS is the total number of packets. */
	double PER;										/*!< PER test result on specific data stream. Format: PER[SpecificStream] */

	// RSSI Test Result
	double RSSI;									/*!< (Average) RSSI test result on specific data stream. Format: RSSI[SpecificStream] */

	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
	//channel list
	int    CH_FREQ_MHZ_PRIMARY_40MHz;       /*!< The center frequency (MHz) for primary 40 MHZ channel  */
	int    CH_FREQ_MHZ_PRIMARY_80MHz;       /*!< The center frequency (MHz) for primary 80 MHZ channel  */

	char   WAVEFORM_NAME[MAX_BUFFER_SIZE]; 						    /*! File name of waveform to verify PER */

} l_11ACrxVerifyPerReturn;
#pragma endregion

#ifndef WIN32
int init11ACRXVerifyPerContainers = Initialize11ACRXVerifyPerContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Power.cpp
int Confirm11ACRxVerifyPerParameters( int *bssBW, int * cbw,int *bssPchannel,int *bssSchannel, int *wifiMode, int *wifiStreamNum, double *cableLossDb, char* errorMsg );


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


WIFI_11AC_TEST_API int WIFI_11AC_RX_Verify_Per(void)
{
	int    err = ERR_OK;

	int    channel = 0;
	int    VHTMode = 0;
	int    dummyValue = 0;
	int    wifiMode = 0;
	int    wifiStreamNum = 0;
	int    packetNumber = 0;
	double cableLossDb = 0;
	double MaxVSGPowerlimit = 0;
	char   vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	int    bssPchannel = 0;
	int    bssSchannel = 0;
	int    bssBW = 0, cbw = 0;

	/*---------------------------------------*
	 * Clear Return Parameters and Container *
	 *---------------------------------------*/
	ClearReturnParameters(l_11ACrxVerifyPerReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_11ACrxVerifyPerParamMap);
		return err;
	}
	else
	{
		// do nothing
	}

	/*-------------------------*
	 * Respond to QUERY_RETURN *
	 *-------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_RETURN", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryReturn(l_11ACrxVerifyPerReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}

	try
	{
		/*-----------------------------------------------------------*
		 * Both g_WiFi_11ac_Test_ID and g_WiFi_11ac_Dut need to be valid (>=0) *
		 *-----------------------------------------------------------*/
		if( g_WiFi_11ac_Test_ID<0 || g_WiFi_11ac_Dut<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] WiFi_Test_ID or WiFi_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_11ac_Test_ID, g_WiFi_11ac_Dut);
		}

		TM_ClearReturns(g_WiFi_11ac_Test_ID);

		/*--------------------------*
		 * Get mandatory parameters *
		 *--------------------------*/
		err = GetInputParameters(l_11ACrxVerifyPerParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get input parameters return OK.\n");
		}

		// Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters
		err = Confirm11ACRxVerifyPerParameters(  &bssBW, &cbw, &bssPchannel,&bssSchannel,&wifiMode, &wifiStreamNum, &cableLossDb, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Prepare input parameters ConfirmRxPerParameters() return OK.\n");
		}
#pragma endregion
#if 0 //ddxx
		/*----------------------------*
		 * Disable VSG output signal  *
		 *----------------------------*/
		// make sure no signal is generated by the VSG
		err = ::LP_EnableVsgRF(0);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
		}
#endif
#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to Receive  *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		vDUT_ClearParameters(g_WiFi_11ac_Dut);

		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_FREQ_MHZ_PRIMARY",		l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_FREQ_MHZ_SECONDARY",		l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ",				l_11ACrxVerifyPerParam.CH_FREQ_MHZ);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_20MHz",	l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "NUM_STREAM_11AC",			l_11ACrxVerifyPerParam.NUM_STREAM_11AC);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "RX1",						l_11ACrxVerifyPerParam.RX1);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "RX2",						l_11ACrxVerifyPerParam.RX2);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "RX3",						l_11ACrxVerifyPerParam.RX3);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "RX4",						l_11ACrxVerifyPerParam.RX4);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_1",			l_11ACrxVerifyPerParam.CABLE_LOSS_DB[0]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_2",			l_11ACrxVerifyPerParam.CABLE_LOSS_DB[1]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_3",			l_11ACrxVerifyPerParam.CABLE_LOSS_DB[2]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_4",			l_11ACrxVerifyPerParam.CABLE_LOSS_DB[3]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "RX_POWER_DBM",				l_11ACrxVerifyPerParam.RX_POWER_DBM);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "BSS_BANDWIDTH",				l_11ACrxVerifyPerParam.BSS_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "CH_BANDWIDTH",				l_11ACrxVerifyPerParam.CH_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "DATA_RATE",					l_11ACrxVerifyPerParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "PACKET_FORMAT",				l_11ACrxVerifyPerParam.PACKET_FORMAT);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "GUARD_INTERVAL",			l_11ACrxVerifyPerParam.GUARD_INTERVAL);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "PREAMBLE",					l_11ACrxVerifyPerParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "STANDARD",					l_11ACrxVerifyPerParam.STANDARD);


		// the following parameters are not input parameters, why are they here?  6-20-2012 Jacky
		//vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_40MHz",	l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz);
		//vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_80MHz",	l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz);

		if ((wifiMode== WIFI_11N_GF_HT40) || ( wifiMode== WIFI_11N_MF_HT40 ))
		{
			//vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ",			l_11ACrxVerifyPerParam.CH_FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "PRIMARY_FREQ",		l_11ACrxVerifyPerParam.CH_FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "SECONDARY_FREQ",		l_11ACrxVerifyPerParam.CH_FREQ_MHZ+10);
		}
		else
		{
			//do nothing
		}
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_BANDWIDTH",		bssBW);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_BANDWIDTH",		cbw);


		if ( g_vDutTxActived==true )
		{
			/*-----------*
			 *  Tx Stop  *
			 *-----------*/
			err = ::vDUT_Run(g_WiFi_11ac_Dut, "TX_STOP");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
		else
		{
			// continue Dut configuration
		}

		err = vDUT_Run(g_WiFi_11ac_Dut, "RX_SET_ANTENNA");
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_SET_ANTENNA) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_SET_ANTENNA) return OK.\n");
		}

		err = vDUT_Run(g_WiFi_11ac_Dut, "RF_SET_FREQ");
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RF_SET_FREQ) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RF_SET_FREQ) return OK.\n");
		}

		err = vDUT_Run(g_WiFi_11ac_Dut, "RX_SET_BW");
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_SET_BW) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_SET_BW) return OK.\n");
		}

		err = vDUT_Run(g_WiFi_11ac_Dut, "RX_SET_DATA_RATE");
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_SET_DATA_RATE) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_SET_DATA_RATE) return OK.\n");
		}

		err = vDUT_Run(g_WiFi_11ac_Dut, "RX_SET_FILTER");
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_SET_FILTER) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_SET_FILTER) return OK.\n");
		}
#pragma endregion
#if 1 //ddxx
#pragma region Setup LP Tester
		if(g_Tester_DualHead)
		{
			while(1)
			{
				err = LP_DualHead_ObtainControl(50,0);
				if ( ERR_OK!=err)
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_DualHead_ObtainControl() Failed.\n");
					//throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_DualHead_ObtainControl() return OK.\n");
				}
				Sleep(50);
			}
		}

		/*----------------------------*
		 * Disable VSG output signal  *
		 *----------------------------*/
		// make sure no signal is generated by the VSG
		err = ::LP_EnableVsgRF(0);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
		}

		/*---------------------*
		 * Load waveform Files *
		 *---------------------*/
		char   modFile[MAX_BUFFER_SIZE];

		if ( strlen(l_11ACrxVerifyPerParam.WAVEFORM_NAME))	// Use the waveform customer defines
		{
			sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s",g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PATH, l_11ACrxVerifyPerParam.WAVEFORM_NAME);

			// Wave file checking
			FILE *waveFile;
			fopen_s(&waveFile, modFile, "r");
			if (!waveFile)  // No such file exist
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
						"[WiFi_11AC] Wrong waveform file name: %s. Please check input paramter!\n",l_11ACrxVerifyPerParam.WAVEFORM_NAME);
				throw logMessage;

			}
			else
			{
				strcpy_s(l_11ACrxVerifyPerReturn.WAVEFORM_NAME,MAX_BUFFER_SIZE, modFile);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Customer defines waveform file name: %s!\n",
						l_11ACrxVerifyPerReturn.WAVEFORM_NAME,MAX_BUFFER_SIZE);

				fclose(waveFile);
				waveFile = NULL;
			}

		}
		else		// Use default waveform name
		{
			err = GetDefaultWaveformFileName(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PATH,
					"iqvsg", //only iqvsg file support in daytona. might need more robust way if we allow IQ2010 etc to support 11ac signal
					wifiMode,
					l_11ACrxVerifyPerParam.NUM_STREAM_11AC,
					cbw,
					l_11ACrxVerifyPerParam.DATA_RATE,
					l_11ACrxVerifyPerParam.PREAMBLE,
					l_11ACrxVerifyPerParam.PACKET_FORMAT,
					l_11ACrxVerifyPerParam.GUARD_INTERVAL,
					modFile,
					MAX_BUFFER_SIZE);
			strcpy_s(l_11ACrxVerifyPerReturn.WAVEFORM_NAME,MAX_BUFFER_SIZE, modFile);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
						"[WiFi_11AC] Fail to get waveform file name: %s, GetDefaultWaveformFileName() return error.\n",l_11ACrxVerifyPerReturn.WAVEFORM_NAME);

				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION,
						"[WiFi_11AC] GetDefaultWaveformFileName() return OK. Waveform file Name: %s\n",l_11ACrxVerifyPerReturn.WAVEFORM_NAME);

			}
		}

		// Load the whole MOD file for continuous transmit
		err = ::LP_SetVsgModulation( modFile );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_SetVsgModulation( %s ) return error.\n", modFile);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_SetVsgModulation( %s ) return OK.\n", modFile);
		}

		/*-------------------*
		 * Setup IQTester VSG *
		 *--------------------*/
		// Since the limitation, we assume that all path loss value are very close.

		// check VSG output power limit first
		if( wifiMode==WIFI_11B )
		{
			MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11B;
		}
		else if( wifiMode==WIFI_11AG )
		{
			MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11G;
		}
		else if ( strstr (l_11ACrxVerifyPerParam.PACKET_FORMAT, PACKET_FORMAT_VHT)) // changed by bguo, 8/6/2012. Check 11ac first then 11n.
		{
			MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11AC;
		}
		else	// 802.11N
		{
			MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11N;
		}
		//else if ( strstr (l_11ACrxVerifyPerParam.PACKET_FORMAT, "11N"))     // commented by bguo 8/6/2012
		//{
		//	MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11N;
		//}
		//else	// 802.11ac
		//{
		//	MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11AC;
		//}

		double VSG_POWER = l_11ACrxVerifyPerParam.RX_POWER_DBM+cableLossDb;
		if ( VSG_POWER>MaxVSGPowerlimit )
		{
			//ERR_VSG_POWER_EXCEED_LIMIT
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, MaxVSGPowerlimit);
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Set VSG port by g_WiFi11ACGlobalSettingParam.VSG_PORT=[%d].\n", g_WiFi11ACGlobalSettingParam.VSG_PORT);
		err = ::LP_SetVsg(l_11ACrxVerifyPerParam.CH_FREQ_MHZ*1e6, VSG_POWER, g_WiFi11ACGlobalSettingParam.VSG_PORT);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to setup VSG, LP_SetVsg() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_SetVsg() return OK.\n");
		}

		/*---------------*
		 *  Turn on VSG  *
		 *---------------*/
		err = ::LP_EnableVsgRF(1);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to turn on VSG, LP_EnableVsgRF(1) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_EnableVsgRF(1) return OK.\n");
		}

		/*-----------------------------*
		 * Send one packet for warm up *
		 *-----------------------------*/
		err = ::LP_SetFrameCnt(1);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] VSG send out packet LP_SetFrameCnt(1) failed.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_SetFrameCnt(1) return OK.\n");
		}
		// After warm up, no signal coming out from VSG
#pragma endregion
#endif
#pragma region Start RX_TEST
		/*---------------------*
		 *  Do RX pre_RX frst  *
		 *---------------------*/
		err = vDUT_Run(g_WiFi_11ac_Dut, "RX_PRE_RX");
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_PRE_RX) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_PRE_RX) return OK.\n");
		}

		/*-------------------------*
		 * clear RX packet counter *
		 *-------------------------*/
		err = vDUT_Run(g_WiFi_11ac_Dut, "RX_CLEAR_STATS");
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_CLEAR_STATS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_CLEAR_STATS) return OK.\n");
		}

		/*----------*
		 * RX Start *
		 *----------*/
		err = vDUT_Run(g_WiFi_11ac_Dut, "RX_START");
		if ( ERR_OK!=err )
		{
			g_vDutRxActived = false;
			// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_START) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			g_vDutRxActived = true;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_START) return OK.\n");
		}

		// Delay for DUT settle
		if (0!=g_WiFi11ACGlobalSettingParam.DUT_RX_SETTLE_TIME_MS)
		{
			Sleep(g_WiFi11ACGlobalSettingParam.DUT_RX_SETTLE_TIME_MS);
		}
		else
		{
			// do nothing
		}

		/*--------------------------*
		 * Send packet for PER Test *
		 *--------------------------*/
		if (0==l_11ACrxVerifyPerParam.FRAME_COUNT)
		{
			err = GetPacketNumber(wifiMode,
					l_11ACrxVerifyPerParam.PACKET_FORMAT,
					&packetNumber);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to get the number of packet for PER test.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get the number of packet GetPacketNumber() return OK.\n");
			}
		}
		else
		{
			packetNumber = l_11ACrxVerifyPerParam.FRAME_COUNT;
		}
		err = ::LP_SetFrameCnt(packetNumber);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] VSG send out packet LP_SetFrameCnt(%d) return error.\n", packetNumber);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] VSG send out packet LP_SetFrameCnt(%d) return OK.\n", packetNumber);
		}
#pragma endregion

		/*-----------------*
		 * Wait for TxDone *
		 *-----------------*/
		int timeOutInMs = 0, sleepTime = 50;
		while ( timeOutInMs<=(1000*(g_WiFi11ACGlobalSettingParam.PER_VSG_TIMEOUT_SEC)) )
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
		if ( timeOutInMs>(1000*(g_WiFi11ACGlobalSettingParam.PER_VSG_TIMEOUT_SEC)) )	// timeout
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] IQTester VSG timeout.\n");
			throw logMessage;
		}
		else
		{
			// no error, do noting.
		}
		if(g_Tester_DualHead)
		{
			err = LP_DualHead_ReleaseControl();
		}		
#pragma region Retrieve analysis Results
		/*--------------------*
		 * Get RX PER Result  *
		 *--------------------*/
		int totalPackets = 0;
		int goodPackets  = 0;
		int badPackets   = 0;

		err = ::vDUT_Run(g_WiFi_11ac_Dut, "RX_GET_STATS");
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_GET_STATS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_GET_STATS) return OK.\n");
		}

		l_11ACrxVerifyPerReturn.RX_POWER_LEVEL = l_11ACrxVerifyPerParam.RX_POWER_DBM;

		err = ::vDUT_GetIntegerReturn(g_WiFi_11ac_Dut, "GOOD_PACKETS", &goodPackets);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_GetIntegerReturn(GOOD_PACKETS) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetIntegerReturn(GOOD_PACKETS) return OK.\n");
		}
		if ( goodPackets<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Dut report GOOD_BITS less than 0.\n");
			throw logMessage;
		}
		else
		{
			l_11ACrxVerifyPerReturn.GOOD_PACKETS = goodPackets;
		}

		// Using packet sent as the total packets
		totalPackets = packetNumber;
		l_11ACrxVerifyPerReturn.TOTAL_PACKETS = totalPackets;

		// Sometime Dut get packets from Air
		// workaround for some dut which returns more ack than packet sent
		if ( goodPackets>totalPackets )
		{
			goodPackets = totalPackets;
			l_11ACrxVerifyPerReturn.GOOD_PACKETS  = goodPackets;
			l_11ACrxVerifyPerReturn.TOTAL_PACKETS = totalPackets;
		}
		else
		{
			// do nothing
		}

		badPackets = totalPackets - goodPackets;

		if( totalPackets!=0 )
		{
			l_11ACrxVerifyPerReturn.PER = ((double)(badPackets)/(double)(totalPackets)) * 100.0;
		}
		else	// In this case, totalPackets = 0
		{
			l_11ACrxVerifyPerReturn.PER = 100.0;
		}

		double rssiValue = 0;
		err = ::vDUT_GetDoubleReturn(g_WiFi_11ac_Dut, "RSSI_ALL", &rssiValue);
		if ( ERR_OK!=err )
		{
			//LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_GetDoubleReturn(RSSI_ALL) return error.\n");
			err = ::vDUT_GetDoubleReturn(g_WiFi_11ac_Dut, "RSSI", &rssiValue);
			if ( ERR_OK!=err )
			{
				// 				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_GetDoubleReturn(RSSI) return error.\n");
				// 				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetDoubleReturn(RSSI) return OK.");
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetDoubleReturn(RSSI_ALL) return OK.");
		}
		l_11ACrxVerifyPerReturn.RSSI = (double)rssiValue;

#pragma endregion

		/*-----------*
		 *  Rx Stop  *
		 *-----------*/
		err = vDUT_Run(g_WiFi_11ac_Dut, "RX_STOP");
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_STOP) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			g_vDutRxActived = false;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_STOP) return OK.\n");
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err )
		{
			sprintf_s(l_11ACrxVerifyPerReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_11ACrxVerifyPerReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_11ACrxVerifyPerReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_11ACrxVerifyPerReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
		err = -1;
	}

	// This is a special case, only when some error occur before the RX_STOP.
	// This case will take care by the error handling, but must do RX_STOP manually.
	if ( g_vDutRxActived )
	{
		vDUT_Run(g_WiFi_11ac_Dut, "RX_STOP");
	}
	else
	{
		// do nothing
	}

	return err;
}

int Initialize11ACRXVerifyPerContainers(void)
{
	/*-----------------*
	 * Input Parameters *
	 *-----------------*/
	l_11ACrxVerifyPerParamMap.clear();

	WIFI_SETTING_STRUCT setting;
	setting.unit = "";
	setting.helpText = "";


	l_11ACrxVerifyPerParam.WAVEFORM_NAME[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACrxVerifyPerParam.WAVEFORM_NAME))    // Type_Checking
	{
		setting.value       = (void*)l_11ACrxVerifyPerParam.WAVEFORM_NAME;
		setting.unit        = "";
		setting.helpText    = "File name of waveform to verify PER. if no value, use default waveform name defined in the program.";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("WAVEFORM_NAME", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	strcpy_s(l_11ACrxVerifyPerParam.BSS_BANDWIDTH, MAX_BUFFER_SIZE, "BW-80");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACrxVerifyPerParam.BSS_BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_11ACrxVerifyPerParam.BSS_BANDWIDTH;
		setting.unit        = "MHz";
		setting.helpText    = "BSS bandwidth\r\nValid options: BW-20, BW-40, BW-80, BW-80_80 or BW-160";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_BANDWIDTH", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	strcpy_s(l_11ACrxVerifyPerParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, "0");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACrxVerifyPerParam.CH_BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_11ACrxVerifyPerParam.CH_BANDWIDTH;
		setting.unit        = "MHz";
		setting.helpText    = "Channel bandwidth\r\nValid options:0, CBW-20, CBW-40, CBW-80, CBW-80_80 or CBW-160.\r\nFor 802.11ac, if it is zero,CH_BANDWIDTH equals as BSS_BANDWIDTH. For 802.11/a/b/g/n, it must always have value.";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_BANDWIDTH", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_11ACrxVerifyPerParam.DATA_RATE, MAX_BUFFER_SIZE, "MCS0");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACrxVerifyPerParam.DATA_RATE))    // Type_Checking
	{
		setting.value = (void*)l_11ACrxVerifyPerParam.DATA_RATE;
		setting.unit        = "";
		setting.helpText    = "Data rate names, such as DSSS-1, CCK-5_5, CCK-11, OFDM-54, MCS0, MCS15";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_11ACrxVerifyPerParam.PREAMBLE, MAX_BUFFER_SIZE, "SHORT");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACrxVerifyPerParam.PREAMBLE))    // Type_Checking
	{
		setting.value       = (void*)l_11ACrxVerifyPerParam.PREAMBLE;
		setting.unit        = "";
		setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=SHORT.";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_11ACrxVerifyPerParam.PACKET_FORMAT, MAX_BUFFER_SIZE, PACKET_FORMAT_VHT);
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACrxVerifyPerParam.PACKET_FORMAT))    // Type_Checking
	{
		setting.value       = (void*)l_11ACrxVerifyPerParam.PACKET_FORMAT;
		setting.unit        = "";
		setting.helpText    = "The packet format, VHT, HT_MF,HT_GF and NON_HTas defined in standard. Default=VHT.";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// "STANDARD"
	strcpy_s(l_11ACrxVerifyPerParam.STANDARD, MAX_BUFFER_SIZE, STANDARD_802_11_AC);
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACrxVerifyPerParam.STANDARD))    // Type_Checking
	{
		setting.value       = (void*)l_11ACrxVerifyPerParam.STANDARD;
		setting.unit        = "";
		setting.helpText    = "Used for signal analysis option or to discriminating the same data rate or package format from different standards, taking value from 802.11ac, 802.11n, 802.11ag, 802.11b. Default = 802.11ac ";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("STANDARD", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// "GUARD_INTERVAL"
	strcpy_s(l_11ACrxVerifyPerParam.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACrxVerifyPerParam.GUARD_INTERVAL))    // Type_Checking
	{
		setting.value       = (void*)l_11ACrxVerifyPerParam.GUARD_INTERVAL;
		setting.unit        = "";
		setting.helpText    = "Packet Guard Interval, Long or Short, default is Long";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY = 5520;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_PRIMARY", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_SECONDARY", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz;
		setting.unit        = "MHz";
		setting.helpText    = "The center frequency (MHz) for primary 20 MHZ channel, priority is lower than \"CH_FREQ_MHZ\".";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_20MHz", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerParam.CH_FREQ_MHZ = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerParam.CH_FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACrxVerifyPerParam.CH_FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "It is the center frequency (MHz) for channel. If it is zero,\"CH_FREQ_MHZ_PRIMARY_20MHz\" will be used for 802.11ac. \r\nFor 802.11/a/b/g/n, it must alway have value.";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_11ACrxVerifyPerParam.NUM_STREAM_11AC = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerParam.NUM_STREAM_11AC))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACrxVerifyPerParam.NUM_STREAM_11AC;
		setting.unit        = "";
		setting.helpText    = "Number of spatial streams based on 11AC spec";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("NUM_STREAM_11AC", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerParam.FRAME_COUNT = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerParam.FRAME_COUNT))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerParam.FRAME_COUNT;
		setting.unit        = "";
		setting.helpText    = "Number of frame count, default=0, means using default global setting value.";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FRAME_COUNT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_11ACrxVerifyPerParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_11ACrxVerifyPerParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_11ACrxVerifyPerParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_11ACrxVerifyPerParam.RX_POWER_DBM = -65.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACrxVerifyPerParam.RX_POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerParam.RX_POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX_POWER_DBM", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerParam.RX1 = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerParam.RX1))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerParam.RX1;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 1 on/off. 1:on; 0:off";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX1", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerParam.RX2 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerParam.RX2))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerParam.RX2;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 2 on/off. 1:on; 0:off";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX2", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerParam.RX3 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerParam.RX3))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerParam.RX3;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 3 on/off. 1:on; 0:off";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX3", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerParam.RX4 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerParam.RX4))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerParam.RX4;
		setting.unit        = "";
		setting.helpText    = "DUT RX path 4 on/off. 1:on; 0:off";
		l_11ACrxVerifyPerParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX4", setting) );
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
	l_11ACrxVerifyPerReturnMap.clear();

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = 0;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
		setting.unit  = "MHz";
		setting.helpText = "The center frequency (MHz) for PRIMARY 40 MHZ channel";
		l_11ACrxVerifyPerReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_40MHz", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = 0;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
		setting.unit  = "MHz";
		setting.helpText = "The center frequency (MHz) for PRIMARY 80 MHZ channel";
		l_11ACrxVerifyPerReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_80MHz", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerReturn.TOTAL_PACKETS))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerReturn.TOTAL_PACKETS;
		setting.unit        = "";
		setting.helpText    = " TOTAL_PACKETS is the number of total packets.";
		l_11ACrxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TOTAL_PACKETS", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerReturn.GOOD_PACKETS = (int)NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACrxVerifyPerReturn.GOOD_PACKETS))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerReturn.GOOD_PACKETS;
		setting.unit        = "";
		setting.helpText    = " GOOD_PACKETS is the number of good packets that reported from Dut.";
		l_11ACrxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("GOOD_PACKETS", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerReturn.PER = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACrxVerifyPerReturn.PER))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerReturn.PER;
		setting.unit        = "%";
		setting.helpText    = "Average PER over received packets.";
		l_11ACrxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PER", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerReturn.RSSI = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACrxVerifyPerReturn.RSSI))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerReturn.RSSI;
		setting.unit        = "dBm";
		setting.helpText    = "Average RSSI over received packets.";
		l_11ACrxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("RSSI", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerReturn.RX_POWER_LEVEL = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACrxVerifyPerReturn.RX_POWER_LEVEL))    // Type_Checking
	{
		setting.value = (void*)&l_11ACrxVerifyPerReturn.RX_POWER_LEVEL;
		setting.unit        = "dBm";
		setting.helpText    = "RX power level for PER";
		l_11ACrxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("RX_POWER_LEVEL", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_11ACrxVerifyPerReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_11ACrxVerifyPerReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_11ACrxVerifyPerReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_11ACrxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_11ACrxVerifyPerReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACrxVerifyPerReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_11ACrxVerifyPerReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_11ACrxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACrxVerifyPerReturn.WAVEFORM_NAME[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACrxVerifyPerReturn.WAVEFORM_NAME))    // Type_Checking
	{
		setting.value       = (void*)l_11ACrxVerifyPerReturn.WAVEFORM_NAME;
		setting.unit        = "";
		setting.helpText    = "The waveform file used to verify PER";
		l_11ACrxVerifyPerReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("WAVEFORM_NAME", setting) );
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
int Confirm11ACRxVerifyPerParameters( int *bssBW, int * cbw,int *bssPchannel,int *bssSchannel, int *wifiMode, int *wifiStreamNum, double *cableLossDb, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	double *samplingTimeUs = NULL;

	try
	{

		// check common input parameters
		err = CheckCommonParameters_WiFi_11ac(l_11ACrxVerifyPerParamMap,
				l_11ACrxVerifyPerReturnMap,
				g_WiFi_11ac_Test_ID,
				bssBW,
				cbw,
				bssPchannel,
				bssSchannel,
				wifiMode,
				wifiStreamNum,
				samplingTimeUs,
				cableLossDb);

		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Check Common Input Parameters failed.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Check11ACCommonInputParameters() return OK.\n");
		}
		//		if ( 0!=strcmp(l_11ACrxVerifyPerParam.PREAMBLE, "SHORT") && 0!=strcmp(l_11ACrxVerifyPerParam.PREAMBLE, "LONG") )
		//		{
		//			err = -1;
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_11ACrxVerifyPerParam.PREAMBLE);
		//			throw logMessage;
		//		}
		//		else
		//		{
		//			//do nothing
		//		}
		//
		//		if ( 0!=strcmp(l_11ACrxVerifyPerParam.GUARD_INTERVAL, "SHORT") && 0!=strcmp(l_11ACrxVerifyPerParam.GUARD_INTERVAL, "LONG") )
		//		{
		//			err = -1;
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown GUARD_INTERVAL, WiFi guard interval %s not supported.\n", l_11ACrxVerifyPerParam.GUARD_INTERVAL);
		//			throw logMessage;
		//		}
		//		else
		//		{
		//			//do nothing
		//		}
		//
		//		if (( 0> l_11ACrxVerifyPerParam.NUM_STREAM_11AC) || ( l_11ACrxVerifyPerParam.NUM_STREAM_11AC >8))
		//		{
		//			err = -1;
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] NUM_STREAM_11AC only can be 1~9. Please check input paramter \"NUM_STREAM_11AC\"!\n");
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
		//		if ( (0!=strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "11AC_MF_HT")) && (0!=strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "11AC_GF_HT")) &&
		//			(0!=strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "11N_MF_HT")) && (0!=strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "11N_GF_HT")) &&
		//			(0!=strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "11AC_VHT")) &&( 0!=strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "11AC_NON_HT"))
		//			&&( 0!=strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "LEGACY_NON_HT")))
		//		{
		//			err = -1;
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown PACKET_FORMAT, WiFi 11ac packet format %s not supported.\n", l_11ACrxVerifyPerParam.PACKET_FORMAT);
		//			throw logMessage;
		//		}
		//		else
		//		{
		//			//do nothing
		//		}
		//
		//		if ( 0 == strcmp( l_11ACrxVerifyPerParam.PACKET_FORMAT, "11AC_NON_HT") &&
		//			! strstr ( l_11ACrxVerifyPerParam.DATA_RATE, "OFDM"))
		//		{
		//			err = -1;
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
		//				"[WiFi_11AC] 11AC_NON_HT, only OFDM-6,9,12,18,24,48,54 supported.\n");
		//			throw logMessage;
		//		}
		//		else
		//		{
		//		}
		//
		//		err = TM_WiFiConvertDataRateNameToIndex(l_11ACrxVerifyPerParam.DATA_RATE, &dummyInt);
		//		if ( ERR_OK!=err )
		//		{
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_11ACrxVerifyPerParam.DATA_RATE);
		//			throw logMessage;
		//		}
		//		else
		//		{
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		//		}
		//
		//		if ( 0 == strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "11AC_VHT"))  // Data rate: MCS0 ~MCS9
		//		{
		//			if (( 14 <= dummyInt ) && (dummyInt <= 23))
		//			{
		//				// Data rate is right
		//				//do nothing
		//			}
		//			else
		//			{
		//				err = -1;
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] PACKET_FORMAT and DATA_RATE don't match! The data rates of VHT_11AC must be MCS0 ~ MCS9, doesn't support %s!\n", &l_11ACrxVerifyPerParam.DATA_RATE);
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
		//		if ( !strstr (l_11ACrxVerifyPerParam.PACKET_FORMAT, "11AC"))    // Legacy signal, CH_BANDWIDTH and CH_FREQ_MHZ must have values
		//		{
		//			if (( l_11ACrxVerifyPerParam.CH_BANDWIDTH <= 0) || ( l_11ACrxVerifyPerParam.CH_FREQ_MHZ <= 0))
		//			{
		//				err = -1;
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] For legacy mode,CH_BANDWIDTH and CH_FREQ_MHZ must have values! Please check input parameters! \n");
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
		//		if ( 0!=strcmp(l_11ACrxVerifyPerParam.BSS_BANDWIDTH, "BW-20") && 0!=strcmp(l_11ACrxVerifyPerParam.BSS_BANDWIDTH, "BW-40") &&
		//			 0!=strcmp(l_11ACrxVerifyPerParam.BSS_BANDWIDTH, "BW-80") && 0!=strcmp(l_11ACrxVerifyPerParam.BSS_BANDWIDTH, "BW-160") &&
		//			 0!=strcmp(l_11ACrxVerifyPerParam.BSS_BANDWIDTH, "BW-80_80"))
		//		{
		//			err = -1;
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACrxVerifyPerParam.BSS_BANDWIDTH);
		//			throw logMessage;
		//		}
		//		else
		//		{
		//			if ( 0 == strcmp(l_11ACrxVerifyPerParam.BSS_BANDWIDTH, "BW-20"))
		//			{
		//				*bssBW = BW_20MHZ;
		//			}
		//			else if ( 0 == strcmp(l_11ACrxVerifyPerParam.BSS_BANDWIDTH, "BW-40"))
		//			{
		//				*bssBW = BW_40MHZ;
		//			}
		//			else if ( 0 == strcmp(l_11ACrxVerifyPerParam.BSS_BANDWIDTH, "BW-80"))
		//			{
		//				*bssBW = BW_80MHZ;
		//			}
		//			else if ( 0 == strcmp(l_11ACrxVerifyPerParam.BSS_BANDWIDTH, "BW-160"))
		//			{
		//				*bssBW = BW_160MHZ;
		//			}
		//			else if ( 0 == strcmp(l_11ACrxVerifyPerParam.BSS_BANDWIDTH, "BW-80_80"))
		//			{
		//				*bssBW = BW_80_80MHZ;
		//
		//				if ( l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
		//				{
		//					err = -1;
		//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
		//						"[WiFi_11AC]  BSS_BANDWIDTH = BW_80_80MHZ,\"BSS_FREQ_MHZ_SECONDARY\" must have value! Please check input parameters!\n" );
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
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACrxVerifyPerParam.BSS_BANDWIDTH);
		//				throw logMessage;
		//			}
		//
		//		}
		//
		//		if ( 0 != strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "0") &&
		//			0!=strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "CBW-20") && 0!=strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "CBW-40") &&
		//			 0!=strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "CBW-80") && 0!=strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "CBW-160") &&
		//			 0!=strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "CBW-80_80"))
		//		{
		//			err = -1;
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown CBW BANDWIDTH, WiFi 11ac CBW bandwidth %s not supported.\n", l_11ACrxVerifyPerParam.CH_BANDWIDTH);
		//			throw logMessage;
		//		}
		//		else
		//		{
		//			if ( 0 == strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "0"))
		//			{
		//				sprintf_s(l_11ACrxVerifyPerParam.CH_BANDWIDTH,MAX_BUFFER_SIZE,"C%s",l_11ACrxVerifyPerParam.BSS_BANDWIDTH);
		//				*cbw = *bssBW;
		//			}
		//			else if ( 0 == strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "CBW-20"))
		//			{
		//				*cbw = BW_20MHZ;
		//			}
		//			else if ( 0 == strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "CBW-40"))
		//			{
		//				*cbw = BW_40MHZ;
		//			}
		//			else if ( 0 == strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "CBW-80"))
		//			{
		//				*cbw = BW_80MHZ;
		//			}
		//			else if ( 0 == strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "CBW-160"))
		//			{
		//				*cbw = BW_160MHZ;
		//			}
		//			else if ( 0 == strcmp(l_11ACrxVerifyPerParam.CH_BANDWIDTH, "CBW-80_80"))
		//			{
		//				*cbw = BW_80_80MHZ;
		//			}
		//			else
		//			{
		//				err = -1;
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACrxVerifyPerParam.BSS_BANDWIDTH);
		//				throw logMessage;
		//			}
		//		}
		//
		//		if ( strstr ( l_11ACrxVerifyPerParam.PREAMBLE,"11AC"))  //802.11ac, cbw can't be larger than bssBW
		//		{
		//			if (*bssBW != BW_80_80MHZ)
		//			{
		//				if (*cbw > *bssBW)
		//				{
		//					err = -1;
		//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Channel bandwidth can not be wider than BSS bandwidth.\n");
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
		//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] BBS bandwidth = 80+80MHz.Channel bandwidth can't be 160MHz.\n");
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
		//		if ( strstr( l_11ACrxVerifyPerParam.PACKET_FORMAT, "11N"))  //802.11n, BW= 20,40MHz
		//		{
		//			if ( *cbw != BW_20MHZ && *cbw != BW_40MHZ)
		//			{
		//				err = -1;
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
		//						"[WiFi_11AC]  11N bandwidth should be 20MHz or 40Mhz. Please check input parameters!\n" );
		//				throw logMessage;
		//			}
		//			else
		//			{
		//				// do nothing
		//			}
		//		}
		//		else if ( 0 == strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "LEGACY_NON_HT"))  // 11B and 11ag
		//		{
		//			if ( *cbw != BW_20MHZ)
		//			{
		//				err = -1;
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
		//						"[WiFi_11AC]  11B and 11AG bandwidth should be 20MHz. Please check input parameters!\n" );
		//				throw logMessage;
		//			}
		//			else
		//			{
		//				// do nothing
		//			}
		//		}
		//		else		// 802.11 ac
		//		{
		//			if ( 0 == strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "11AC_VHT"))
		//			{
		//				//do nothing
		//			}
		//			else if ( 0 == strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "11AC_MF_HT") ||
		//				0 == strcmp(l_11ACrxVerifyPerParam.PACKET_FORMAT, "11AC_GF_HT"))
		//			{
		//				if ( *cbw != BW_20MHZ && *cbw != BW_40MHZ)
		//				{
		//					err = -1;
		//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
		//							"[WiFi_11AC]  11AC_MF(GF)_HT bandwidth should be 20MHz or 40Mhz. Please check input parameters!\n" );
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
		//							"[WiFi_11AC]  11AC_NON_HT bandwidth can't be 20MHz. Please check input parameters!\n" );
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
		//		if (l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY <= 0)
		//		{
		//			err = -1;
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
		//					"[WiFi_11AC]  \"BSS_FREQ_MHZ_PRIMARY\" must have value! Please check input parameters!\n" );
		//			throw logMessage;
		//
		//		}
		//		else
		//		{
		//			// do nothing
		//		}
		//
		//		err = TM_WiFiConvertFrequencyToChannel(l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY, bssPchannel);
		//		if ( ERR_OK!=err )
		//		{
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY);
		//			throw logMessage;
		//		}
		//		else
		//		{
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		//		}
		//
		//		if ( *bssBW == BW_80_80MHZ)  // Need BSS_FREQ_MHZ_SECONDARY
		//		{
		//			if ( l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
		//			{
		//				err = -1;
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
		//					"[WiFi_11AC]  BSS_BANDWIDTH = BW_80_80MHZ,\"BSS_FREQ_MHZ_SECONDARY\" must have value!\n" );
		//				throw logMessage;
		//			}
		//			else
		//			{
		//					// do nothing
		//			}
		//
		//			err = TM_WiFiConvertFrequencyToChannel(l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY, bssSchannel);
		//			if ( ERR_OK!=err )
		//			{
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_SECONDARY);
		//				throw logMessage;
		//			}
		//			else
		//			{
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_WiFiConvertFrequencyToChannel() return OK.\n");
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
		//		if ( strstr (l_11ACrxVerifyPerParam.PACKET_FORMAT, "11AC"))
		//		{
		//
		//			if ( 0 == l_11ACrxVerifyPerParam.CH_FREQ_MHZ)
		//			{
		//				//no CH_FREQ_MHZ, no CH_FREQ_MHZ_PRIMARY_20MHz
		//				// Use default values
		//				if ( 0 == l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz)
		//				{
		//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi_11AC] Don't have input parameters \"CH_FREQ_MHZ\" or \"CH_FREQ_MHZ_PRIMARY_20\", using BBS center freuqency as default!");
		//					l_11ACrxVerifyPerParam.CH_FREQ_MHZ = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
		//
		//					// all  use lower frequency for channel list
		//					switch (*bssBW)
		//					{
		//					case BW_20MHZ:
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
		//						break;
		//
		//					case BW_40MHZ:
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 10;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
		//						break;
		//
		//					case BW_80MHZ:
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 20;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
		//						break;
		//
		//					case BW_160MHZ:
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 70;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 60;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 40;
		//						break;
		//
		//					case BW_80_80MHZ:
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 70;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 60;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 40;
		//						break;
		//
		//					default:
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
		//						break;
		//					}
		//
		//				}
		//				else  //no CH_FREQ_MHZ, have CH_FREQ_MHZ_PRIMARY_20MHz. Use input CH_FREQ_MHZ_PRIMARY_20MHz to calculate
		//				{
		//					err = GetChannelList(*bssBW, l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY,l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz,
		//						&l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
		//					if ( err != ERR_OK) // Wrong channel list
		//					{
		//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_11ACrxVerifyPerParam.PACKET_FORMAT);
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
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
		//					}
		//					else if (*cbw == BW_20MHZ)
		//					{
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ = l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz;
		//					}
		//					else if (*cbw == BW_40MHZ)
		//					{
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ = l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
		//					}
		//					else if (*cbw == BW_80MHZ)
		//					{
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ = l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
		//					}
		//					else
		//					{
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
		//					}
		//				}
		//				else // cbw = BW_80_80MHZ, use BSS primary and secondary
		//				{
		//					l_11ACrxVerifyPerParam.CH_FREQ_MHZ = NA_INTEGER;
		//					// do nothing
		//				}
		//			}
		//			else  // CH_FREQ_MHZ: non-zero
		//			{
		//				//Check if input CH_FREQ_MHZ is correct
		//				err = CheckChannelFreq(*bssBW, *cbw,
		//					l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY,l_11ACrxVerifyPerParam.CH_FREQ_MHZ);
		//
		//				if (err != ERR_OK)
		//				{
		//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input paramter \"CH_FREQ_MHZ\" wrong. Please check!\n");
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
		//				if ( 0 != l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz)
		//				{
		//					err = CheckChPrimary20(*bssBW, *cbw,
		//							l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY,l_11ACrxVerifyPerParam.CH_FREQ_MHZ,l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz);
		//					if ( err == ERR_OK)  //input CH_FREQ_MHZ_PRIMARY_20MHz is correct
		//					{
		//						err = GetChannelList(*bssBW, l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY,l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz,
		//							&l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
		//						if ( err != ERR_OK) // Wrong channel list
		//						{
		//							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Can't get channel list. Please check input paramters.\n", l_11ACrxVerifyPerParam.PACKET_FORMAT);
		//							throw logMessage;
		//						}
		//						else  // Get channel list successfully
		//						{
		//						}
		//					}
		//					else
		//					{
		//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input paramter \"CH_FREQ_MHZ_PRIMARY_20MHz\" wrong. Please check!\n");
		//						throw logMessage;
		//					}
		//				}
		//				else //input CH_FREQ_MHZ_PRIMARY_20MHz is zero
		//				{
		//					//Get channel primary20
		//					switch (*bssBW)
		//					{
		//					case BW_20MHZ:
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
		//						l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
		//						break;
		//					case BW_40MHZ:  //cbw = BW_20.40MHz
		//						if (*cbw == BW_20MHZ)
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.CH_FREQ_MHZ;
		//						}
		//						else if ( *cbw == BW_40MHZ)
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 10;
		//
		//						}
		//						else  //wrong cbw
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
		//						}
		//						break;
		//					case BW_80MHZ:  //cbw = BW_20,40,80MHz
		//						if ( *cbw == BW_20MHZ)
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.CH_FREQ_MHZ;
		//						}
		//						else if ( *cbw == BW_40MHZ)
		//						{
		//							if (l_11ACrxVerifyPerParam.CH_FREQ_MHZ == l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 20)
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
		//							}
		//							else if (l_11ACrxVerifyPerParam.CH_FREQ_MHZ == l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 20)
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 10;
		//							}
		//							else  // wrong CH_FREQ_MHZ
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
		//							}
		//						}
		//						else if ( *cbw == BW_80MHZ)
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
		//						}
		//						else  //wrong cbw
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz =  NA_INTEGER;
		//
		//						}
		//						break;
		//					case BW_160MHZ:  //cbw = BW_20,40,80,160MHz
		//						if ( *cbw == BW_20MHZ)
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.CH_FREQ_MHZ;
		//						}
		//						else if ( *cbw == BW_40MHZ)
		//						{
		//							if (l_11ACrxVerifyPerParam.CH_FREQ_MHZ == l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 60)
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 70;
		//							}
		//							else if (l_11ACrxVerifyPerParam.CH_FREQ_MHZ == l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 20)
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
		//							}
		//							else if (l_11ACrxVerifyPerParam.CH_FREQ_MHZ == l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 20)
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 10;
		//							}
		//							else if ( l_11ACrxVerifyPerParam.CH_FREQ_MHZ == l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 60)
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 50;
		//							}
		//							else  //wrong CH_FREQ_MHZ
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
		//							}
		//						}
		//						else if ( *cbw == BW_80MHZ)
		//						{
		//							if (l_11ACrxVerifyPerParam.CH_FREQ_MHZ == l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 40)
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 70;
		//							}
		//							else if (l_11ACrxVerifyPerParam.CH_FREQ_MHZ == l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 40)
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 10;
		//							}
		//							else // wrong CH_FREQ_MHZ
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
		//							}
		//						}
		//						else if ( *cbw == BW_160MHZ)
		//						{
		//							if (l_11ACrxVerifyPerParam.CH_FREQ_MHZ == l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY)
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 70;
		//							}
		//							else // wrong CH_FREQ_MHZ
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
		//							}
		//						}
		//						else  //wring cbw
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
		//						}
		//						break;
		//					case BW_80_80MHZ:
		//						if ( *cbw == BW_20MHZ)
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.CH_FREQ_MHZ;
		//						}
		//						else if ( *cbw == BW_40MHZ)
		//						{
		//							if ( l_11ACrxVerifyPerParam.CH_FREQ_MHZ == l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 20)
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
		//							}
		//							else if (l_11ACrxVerifyPerParam.CH_FREQ_MHZ == l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 20)
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY + 10;
		//							}
		//							else  // wrong CH_FREQ_MHZ
		//							{
		//								l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
		//							}
		//						}
		//						else if ( *cbw == BW_80MHZ)
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
		//						}
		//						else if ( *cbw == BW_80_80MHZ)
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY - 30;
		//						}
		//						else  // wrong CH_FREQ_MHz
		//						{
		//							l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
		//						}
		//						break;
		//					default:
		//						l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
		//						break;
		//					}
		//				}
		//
		//				err = GetChannelList(*bssBW, l_11ACrxVerifyPerParam.BSS_FREQ_MHZ_PRIMARY,l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz,
		//						&l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
		//				if ( err != ERR_OK)  //Get channel list wrong
		//				{
		//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_11ACrxVerifyPerParam.PACKET_FORMAT);
		//					throw logMessage;
		//				}
		//				else  //Get channel list successfully
		//				{
		//				}
		//			}
		//
		//			//Check channel list. If all are "NA_INTEGER", return error
		//			if (( l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz == NA_INTEGER ) &&
		//				(l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz == NA_INTEGER ) &&
		//				(l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER))
		//			{
		//				err = -1;
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20/40/80MHz, please check input parameters.\n");
		//				throw logMessage;
		//			}
		//			else
		//			{
		//				// do nothing
		//			}
		//		}
		//		else   // non-ac, no use of CH_FREQ_MHZ_PRIMARY_20/40/80MHz
		//		{
		//			l_11ACrxVerifyPerParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
		//			l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
		//			l_11ACrxVerifyPerReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
		//		}
		//#pragma endregion
		//
		//		//VHT, cbw = BW_20MHZ, the stream number of MCS9 only can 3,6
		//		if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )
		//		{
		//			if ( (l_11ACrxVerifyPerParam.NUM_STREAM_11AC != 3) &&
		//				(l_11ACrxVerifyPerParam.NUM_STREAM_11AC != 6) )
		//			{
		//				err = -1;
		//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] 802.11ac CBW = 20MHz, Data Rate = MCS9,NUM_STREAM_11AC only can be 3,6!\n");
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
		//		err = WiFi_11ac_TestMode(l_11ACrxVerifyPerParam.DATA_RATE, cbw, wifiMode, wifiStreamNum, l_11ACrxVerifyPerParam.PACKET_FORMAT);
		//		if ( ERR_OK!=err )
		//		{
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Convert WiFi test mode function failed.\n");
		//			throw logMessage;
		//		}
		//		else
		//		{
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] WiFiTestMode() return OK.\n");
		//		}
		//
		//
		//		//Check path loss (by ant and freq)
		//		//temp: declare double *cableLossDb
		//		//TODO: remove "cableLossDb" in CheckPathLossTable(), each VSA/VSG cable loss will record in CABLE_LOSS_DB of local Param
		//		//double *cableLossDb= (double *) malloc (sizeof (double));
		//		err = CheckPathLossTableExt(	g_WiFi_11ac_Test_ID,
		//										l_11ACrxVerifyPerParam.CH_FREQ_MHZ,
		//										  l_11ACrxVerifyPerParam.RX1,
		//										  l_11ACrxVerifyPerParam.RX2,
		//										  l_11ACrxVerifyPerParam.RX3,
		//										  l_11ACrxVerifyPerParam.RX4,
		//										l_11ACrxVerifyPerParam.CABLE_LOSS_DB,
		//										l_11ACrxVerifyPerReturn.CABLE_LOSS_DB,
		//										cableLossDb,
		//										RX_TABLE
		//									);
		//		if ( ERR_OK!=err )
		//		{
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to get CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
		//			throw logMessage;
		//		}
		//		else
		//		{
		//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] CheckPathLossTableExt() return OK.\n");
		//		}

		// check unique input parameters used only for Rx_verify_Per test ...
		// to do...
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11AC] ConfirmRxPerParameters() Confirmed.\n");
	}
	catch(char *msg)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
	}
	catch(...)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11AC] Unknown Error!\n");
		err = -1;
	}


	return err;
}

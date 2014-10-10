#include "stdafx.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;

// This global variable is declared in BT_Test_Internal.cpp
extern TM_ID      g_BT_Test_ID;
extern vDUT_ID    g_BT_Dut;
extern int		  g_Tester_Type;

// This global variable is declared in BT_Global_Setting.cpp
extern BT_GLOBAL_SETTING g_BTGlobalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, BT_SETTING_STRUCT> l_rxVerifyBdrParamMap;

// Return Value Container
map<string, BT_SETTING_STRUCT> l_rxVerifyBdrReturnMap;

struct tagParam
{
    // Mandatory Parameters
	int       FREQ_MHZ;								/*! The center frequency (MHz). */				
	char      PACKET_TYPE[MAX_BUFFER_SIZE];			/*! The pack type to verify BDR. */
	int		  PACKET_LENGTH;						/*! The number of payload length to verify BDR. */
	double    RX_POWER_LEVEL;						/*! The output power to verify BDR. */
	double    RX_BIT_NUMBER;						/*! The minimum number of payload bits. */ 
	double    CABLE_LOSS_DB;						/*! The path loss of test system. */
	int		  TIMEOUT;								/*! The timeout setting for receive process. 0 forever waiting*/
} l_rxVerifyBdrParam;

struct tagReturn
{
    // RX Power Level
    double     RX_POWER_LEVEL;     /*!< POWER_LEVEL dBm for the Ber test. Format: POWER LEVEL */

    // BER Test Result 
    double	   GOOD_BITS;		   /*!< GOOD_BITS is the number of good bits that reported from Dut */
	double     TOTAL_BITS;		   /*!< TOTAL_BITS is the total number of bits that reported from Dut. */
	double	   BER;                /*!< BER test result on specific data stream. */
	double	   RSSI;                                /*!< RSSI test result on specific data stream. */
	    
	double     CABLE_LOSS_DB;						/*! The path loss of test system. */
    char       ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_rxVerifyBdrReturn;
#pragma endregion

#ifndef WIN32
int initRXVerifyBDRContainers = InitializeRXVerifyBDRContainers();
#endif

int ClearRxVerifyBdrReturn(void)
{
	l_rxVerifyBdrParamMap.clear();
	l_rxVerifyBdrReturnMap.clear();
	return 0;
}

//! BT RX Verify BER
/*!
* Input Parameters
*
*  - Mandatory 
*      -# FREQ_MHZ (double): The center frequency (MHz)
*
* Return Values
*      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/


BT_TEST_API int BT_RX_Verify_BDR(void)
{
    int    err = ERR_OK;

//    bool   analysisOK = false, captureOK  = false;
	bool   vDutActived = false;     
    int    dummyValue = 0;
	int	   packetLength = 0;
//	double samplingTimeUs = 0;
	double cableLossDb = 0;
	char   vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_rxVerifyBdrReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_rxVerifyBdrParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_rxVerifyBdrReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*-----------------------------------------------------------*
		* Both g_BT_Test_ID and g_BT_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		TM_ClearReturns(g_BT_Test_ID);
		if( g_BT_Test_ID<0 || g_BT_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] BT_Test_ID or BT_Dut not valid. BT_Test_ID = %d and BT_Dut = %d.\n", g_BT_Test_ID, g_BT_Dut);
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		/*--------------------------*
		 * Get mandatory parameters *
		 *--------------------------*/
		err = GetInputParameters(l_rxVerifyBdrParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not completed.\n");
			throw logMessage;
		}

		// Check path loss (by ant and freq)
		if ( 0==l_rxVerifyBdrParam.CABLE_LOSS_DB )
		{
			err = TM_GetPathLossAtFrequency(g_BT_Test_ID, l_rxVerifyBdrParam.FREQ_MHZ, &l_rxVerifyBdrParam.CABLE_LOSS_DB, 0, RX_TABLE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Failed to get CABLE_LOSS_DB from path loss table.\n", err);
				throw logMessage;
			}
		}
		else
		{
			// do nothing
		}

#pragma region Setup LP Tester
	   /*----------------------------*
		* Disable VSG output signal  *
		*----------------------------*/
		// make sure no signal is generated by the VSG
		err = ::LP_EnableVsgRF(0);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
		}

	   /*---------------------*
		* Load waveform Files *
		*---------------------*/
		char   modFile[MAX_BUFFER_SIZE] = {'\0'};
		err = GetWaveformFileName("BER",		
								  "WAVEFORM_NAME", 
								  l_rxVerifyBdrParam.PACKET_TYPE,
								  modFile, 
								  MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to get waveform file name.\n");
			throw logMessage;
		}
	    
		// Load the whole MOD file for continuous transmit
		err = LP_SetVsgModulation( modFile );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] LP_SetVsgModulation( %s ) return error.\n", modFile);
			throw logMessage;
		}

        g_BTGlobalSettingParam.MULTI_WAVE_FILE_CHANGED = true;
	   /*--------------------*
		* Setup IQTester VSG *
		*--------------------*/
		cableLossDb = l_rxVerifyBdrParam.CABLE_LOSS_DB;

		double VSG_POWER = l_rxVerifyBdrParam.RX_POWER_LEVEL + cableLossDb;
		if ( VSG_POWER>g_BTGlobalSettingParam.VSG_MAX_POWER_BT )
		{
			//ERR_VSG_POWER_EXCEED_LIMIT
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, g_BTGlobalSettingParam.VSG_MAX_POWER_BT);
			throw logMessage;
		}
		else
		{
			// do nothing
		}		

		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Set VSG port by g_BTGlobalSettingParam.VSG_PORT=[%d].\n", g_BTGlobalSettingParam.VSG_PORT);
		err = LP_SetVsg(l_rxVerifyBdrParam.FREQ_MHZ*1e6, VSG_POWER, g_BTGlobalSettingParam.VSG_PORT);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to setup VSG.\n");
			throw logMessage;
		}		

	   /*---------------*
		*  Turn on VSG  *
		*---------------*/
		err = LP_EnableVsgRF(1);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to turn on VSG.\n");
			throw logMessage;
		}				
        /*----------------------------------------------------------------*
        * Send one packet for warm up and make sure that no TX in the VSG *
        *-----------------------------------------------------------------*/           
        err = ::LP_SetFrameCnt(1);
        if ( ERR_OK!=err )
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] VSG send out packet LP_SetFrameCnt(1) failed.\n");
            throw logMessage;
        }
        else
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] LP_SetFrameCnt(1) return OK.\n");
        }
        // After warm up, no signal coming out from VSG


#pragma endregion

#pragma region Configure DUT to Receive
		/*------------------------------------------*
		 * Configure DUT to Receive - PRBS9 Pattern *
		 *------------------------------------------*/
		// Set DUT RF frequency...
		// And clear vDUT parameters at the beginning.
		vDUT_ClearParameters(g_BT_Dut);

		vDUT_AddStringParameter (g_BT_Dut, "REMOTE_BD_ADDRESS", g_BTGlobalSettingParam.BER_WAVEFORM_BD_ADDRESS);
		vDUT_AddIntegerParameter(g_BT_Dut, "FREQ_MHZ",			l_rxVerifyBdrParam.FREQ_MHZ);	
		 // Description				Enum in the IQFact
		 // 0x00 8-bit Pattern		ZERO =0:   all zero
		 // 0xFF 8-bit Pattern      ONE1 =1:   all ones
		 // 0xAA 8-bit Pattern      HEX_A=2:   1010
		 // 0xF0 8-bit Pattern		HEX_F0=3:  11110000
		 // PRBS9 Pattern           RANDOM=4;  PRBS=0
		vDUT_AddIntegerParameter(g_BT_Dut, "MODULATION_TYPE",   BT_PATTERN_PRBS);	//{0x04, "PRBS9 Pattern"}
		 // {0x00, "ACL  EDR"},
		 // {0x01, "ACL  Basic"},
		 // {0x02, "eSCO EDR"},
		 // {0x03, "eSCO Basic"},
		 // {0x04, "SCO  Basic"}
		vDUT_AddIntegerParameter(g_BT_Dut, "LOGIC_CHANNEL",     1);	//{0x01, "ACL Basic"}
		vDUT_AddStringParameter (g_BT_Dut, "PACKET_TYPE",	    l_rxVerifyBdrParam.PACKET_TYPE);
		// Check packet length 		
		if (0==l_rxVerifyBdrParam.PACKET_LENGTH)
		{
			GetPacketLength("BER", "PACKETS_LENGTH", l_rxVerifyBdrParam.PACKET_TYPE, &packetLength);
		}
		else	
		{
			packetLength = l_rxVerifyBdrParam.PACKET_LENGTH;
		}
		vDUT_AddIntegerParameter(g_BT_Dut, "PACKET_LENGTH",     packetLength);
		vDUT_AddDoubleParameter (g_BT_Dut, "RX_BIT_NUMBER",		l_rxVerifyBdrParam.RX_BIT_NUMBER);
		vDUT_AddIntegerParameter(g_BT_Dut, "TIMEOUT",     l_rxVerifyBdrParam.TIMEOUT);


		err = vDUT_Run(g_BT_Dut, "RX_CLEAR_STATS");		
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(RX_CLEAR_STATS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			// do nothing
		}

		err = vDUT_Run(g_BT_Dut, "RX_START");		
		if ( ERR_OK!=err )
		{	
			vDutActived = false;
			//Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.			
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				LP_EnableVsgRF(0);
				throw logMessage;
			}
			else	// Return error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(RX_START) return error.\n");
				LP_EnableVsgRF(0);
				throw logMessage;
			}
		}
		else
		{
			vDutActived = true;
		}

	   /*--------------------------*
		* Send packet for BER Test *
		*--------------------------*/           
		err = LP_SetFrameCnt(0);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] VSG send out packet failed.\n");
			vDUT_Run(g_BT_Dut, "RX_STOP");		
			LP_EnableVsgRF(0);
			throw logMessage;
		}		

		// Delay for DUT settle
		if (0!=g_BTGlobalSettingParam.DUT_RX_SETTLE_TIME_MS)
		{
			Sleep(g_BTGlobalSettingParam.DUT_RX_SETTLE_TIME_MS);
		}
		else
		{
			// do nothing
		}
#pragma endregion

#pragma region Retrieve analysis Results
	   /*--------------------*
		* Get Rx BER Result  *
		*--------------------*/
		double totalPackets = 0;
		double goodPackets  = 0;
		double badPackets   = 0;

		err = vDUT_Run(g_BT_Dut, "RX_GET_STATS");		
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				vDUT_Run(g_BT_Dut, "RX_STOP");		
				LP_EnableVsgRF(0);
				throw logMessage;
			}
			else	// Return error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(RX_GET_STATS) return error.\n");
				vDUT_Run(g_BT_Dut, "RX_STOP");		
				LP_EnableVsgRF(0);
				throw logMessage;
			}
		}
		else
		{
			// do nothing
		}

		l_rxVerifyBdrReturn.RX_POWER_LEVEL = l_rxVerifyBdrParam.RX_POWER_LEVEL;

		err = vDUT_GetDoubleReturn(g_BT_Dut, "GOOD_BITS", &goodPackets);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_GetDoubleReturn(GOOD_BITS) return error.\n");
			vDUT_Run(g_BT_Dut, "RX_STOP");		
			LP_EnableVsgRF(0);
			throw logMessage;
		}		
		if ( goodPackets<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Dut report GOOD_BITS less than 0.\n");
			vDUT_Run(g_BT_Dut, "RX_STOP");		
			LP_EnableVsgRF(0);
			throw logMessage;
		}
		else
		{
			l_rxVerifyBdrReturn.GOOD_BITS = goodPackets;
		}

		err = vDUT_GetDoubleReturn(g_BT_Dut, "TOTAL_BITS", &totalPackets);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_GetDoubleReturn(TOTAL_BITS) return error.\n");
			vDUT_Run(g_BT_Dut, "RX_STOP");		
			LP_EnableVsgRF(0);
			throw logMessage;
		}
		if ( totalPackets<0 )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Dut report TOTAL_BITS less than 0.\n");
			err = -1;
			vDUT_Run(g_BT_Dut, "RX_STOP");		
			LP_EnableVsgRF(0);
			throw logMessage;
		}
		else
		{
			l_rxVerifyBdrReturn.TOTAL_BITS = totalPackets;
		}

		// Sometimes the DUT can get packets through other means, such as from the air
		// This is a workaround for some DUTs that return more ack than the number of packets sent
		if ( goodPackets>totalPackets )	
		{
			goodPackets = totalPackets;
			l_rxVerifyBdrReturn.GOOD_BITS  = goodPackets;
			l_rxVerifyBdrReturn.TOTAL_BITS = totalPackets;
		}
		else
		{
			// do nothing	
		}
		
		badPackets = totalPackets - goodPackets;

		if( totalPackets!=0 )
		{
			l_rxVerifyBdrReturn.BER = (badPackets/totalPackets) * 100.0;
		}
		else	// In this case, totalPackets = 0
		{
			l_rxVerifyBdrReturn.BER = 100.0;
		}

		err = ::vDUT_GetDoubleReturn(g_BT_Dut, "RSSI", &l_rxVerifyBdrReturn.RSSI); 
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[BT] WARNING, This DUT does not support RSSI measurement.\n");
			l_rxVerifyBdrReturn.RSSI = NA_NUMBER;
			//throw logMessage;
		}


#pragma endregion

	   /*-----------*
		*  Rx Stop  *
		*-----------*/
		err = vDUT_Run(g_BT_Dut, "RX_STOP");		
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Returns error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(RX_STOP) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			vDutActived = false;
		}

	   /*----------------------------*
		* Disable VSG output signal  *
		*----------------------------*/
		err = LP_EnableVsgRF(0);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to turn off VSG.\n");
			throw logMessage;
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err )
		{
			// Return Path Loss (dB)
			l_rxVerifyBdrReturn.CABLE_LOSS_DB = l_rxVerifyBdrParam.CABLE_LOSS_DB;

			sprintf_s(l_rxVerifyBdrReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_rxVerifyBdrReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_rxVerifyBdrReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_rxVerifyBdrReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
    }

	// This is a special case and happens only when certain errors occur before the RX_STOP. 
	// This is handled by error handling, but RX_STOP must be handled manually.
	
	if ( vDutActived )
	{
		vDUT_Run(g_BT_Dut, "RX_STOP");
	}
	else
	{
		// do nothing
	}

    return err;
}

int InitializeRXVerifyBDRContainers(void)
{
    /*-----------------*
     * Input Parameters *
     *-----------------*/
    l_rxVerifyBdrParamMap.clear();

    BT_SETTING_STRUCT setting;

	l_rxVerifyBdrParam.RX_POWER_LEVEL = -70.0;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyBdrParam.RX_POWER_LEVEL))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyBdrParam.RX_POWER_LEVEL;
        setting.unit        = "dBm";
        setting.helpText    = "The output power to verify BDR.";
        l_rxVerifyBdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("RX_POWER_LEVEL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerifyBdrParam.FREQ_MHZ = 2402;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerifyBdrParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerifyBdrParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz";
        l_rxVerifyBdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	//{0x00, "NULL"}
	//{0x01, "POLL"}
	//{0x02, "FHS"}
	//{0x03, "DM1"}
	//{0x04, "DH1 / 2DH1"}
	//{0x05, "HV1"}
	//{0x06, "HV2 / 2-EV3"}
	//{0x07, "HV3 / EV3 / 3-EV3"}
	//{0x08, "DV / 3DH1"}
	//{0x09, "AUX1 / PS"}
	//{0x0A, "DM3 / 2DH3"}
	//{0x0B, "DH3 / 3DH3"}
	//{0x0C, "EV4 / 2-EV5"}
	//{0x0D, "EV5 / 3-EV5"}
	//{0x0E, "DM5 / 2DH5"}
	//{0x0F, "DH5 / 3DH5"}

    strcpy_s(l_rxVerifyBdrParam.PACKET_TYPE, MAX_BUFFER_SIZE, "1DH1");
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyBdrParam.PACKET_TYPE))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerifyBdrParam.PACKET_TYPE;
        setting.unit        = "";
        setting.helpText    = "Sets the packet type, the type can be 1DH1, 1DH3, 1DH5. default is 1DH1";
        l_rxVerifyBdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyBdrParam.PACKET_LENGTH = 0;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerifyBdrParam.PACKET_LENGTH))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerifyBdrParam.PACKET_LENGTH;
        setting.unit        = "";
        setting.helpText    = "The number of payload length to verify BDR.";
        l_rxVerifyBdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyBdrParam.TIMEOUT = 20;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyBdrParam.TIMEOUT))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyBdrParam.TIMEOUT;
		setting.unit        = "sec";
		setting.helpText    = "The timeout setting for receive. Set to 0 for forever waiting.";
		l_rxVerifyBdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("TIMEOUT", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyBdrParam.RX_BIT_NUMBER = 1600000;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyBdrParam.RX_BIT_NUMBER))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyBdrParam.RX_BIT_NUMBER;
        setting.unit        = "";
        setting.helpText    = "The minimum number of payload bits. Default value is 1,600,000.";
        l_rxVerifyBdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("RX_BIT_NUMBER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyBdrParam.CABLE_LOSS_DB = 0.0;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyBdrParam.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyBdrParam.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_rxVerifyBdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
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
    l_rxVerifyBdrReturnMap.clear();

	l_rxVerifyBdrReturn.TOTAL_BITS = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyBdrReturn.TOTAL_BITS))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyBdrReturn.TOTAL_BITS;
        setting.unit        = "bits";
        setting.helpText    = "TOTAL_BITS is the total number of bits that reported from Dut.";
        l_rxVerifyBdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("TOTAL_BITS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyBdrReturn.GOOD_BITS = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyBdrReturn.GOOD_BITS))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyBdrReturn.GOOD_BITS;
        setting.unit        = "bits";
        setting.helpText    = "GOOD_BITS is the number of good bits that reported from Dut.";
        l_rxVerifyBdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("GOOD_BITS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerifyBdrReturn.BER = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyBdrReturn.BER))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyBdrReturn.BER;
        setting.unit        = "%";
        setting.helpText    = "BER over received packets.";
        l_rxVerifyBdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("BER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	l_rxVerifyBdrReturn.RSSI = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyBdrReturn.RSSI))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyBdrReturn.RSSI;
		setting.unit        = "";
		setting.helpText    = "RSSI result when received packets.";
		l_rxVerifyBdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("RSSI", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyBdrReturn.RX_POWER_LEVEL = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyBdrReturn.RX_POWER_LEVEL))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyBdrReturn.RX_POWER_LEVEL;
        setting.unit        = "dBm";
        setting.helpText    = "RX power level for BER";
        l_rxVerifyBdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("RX_POWER_LEVEL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyBdrReturn.CABLE_LOSS_DB = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyBdrReturn.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyBdrReturn.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_rxVerifyBdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_rxVerifyBdrReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyBdrReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerifyBdrReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_rxVerifyBdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}


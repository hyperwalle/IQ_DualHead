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
map<string, BT_SETTING_STRUCT> l_rxVerifyEdrParamMap;

// Return Value Container
map<string, BT_SETTING_STRUCT> l_rxVerifyEdrReturnMap;

struct tagParam
{
    // Mandatory Parameters
	int       FREQ_MHZ;								/*! The center frequency (MHz). */				
	int		  PACKET_LENGTH;						/*! The number of payload length to verify EDR. */
	char      PACKET_TYPE[MAX_BUFFER_SIZE];			/*! The pack type to verify EDR. */	
	double	  RX_BER_THRESHOLD_LIMIT_LEVEL1;		/*! A threshold of BER test for EDR only, BER result must pass this limit or tester need to continue send out packets until reach RX_BIT_NUMBER_LEVEL2 bits. */   
	double    RX_BIT_NUMBER_LEVEL1;					/*! The minimum number of payload bits. */ 
	double    RX_BIT_NUMBER_LEVEL2;					/*! The minimum number of payload bits. */ 
	double    RX_POWER_LEVEL;						/*! The output power to verify EDR. */
	double    CABLE_LOSS_DB;						/*! The path loss of test system. */
	int		  ENABLE_RX_LEVEL2_TEST;				/*! A flag to enable the RX EDR Level-2 test. Default = 1 (Enable). */
	int		  TIMEOUT;								/*! The timeout setting for receive process.Default 0 for forever waiting*/
} l_rxVerifyEdrParam;

struct tagReturn
{
    // RX Power Level
    double     RX_POWER_LEVEL;     /*!< POWER_LEVEL dBm for the Ber test. Format: POWER LEVEL */

    // Ber Test Result 
    double	   GOOD_BITS;		   /*!< GOOD_BITS is the number of good bits that reported from Dut */
	double     TOTAL_BITS;		   /*!< TOTAL_BITS is the total number of bits that reported from Dut. */
	double	   BER;                /*!< BER test result on specific data stream. */
	    
	double	   RSSI;                                /*!< RSSI test result on specific data stream. */

	double     CABLE_LOSS_DB;						/*! The path loss of test system. */
    char       ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_rxVerifyEdrReturn;
#pragma endregion

#ifndef WIN32
int initRXVerifyEDRContainers = InitializeRXVerifyEDRContainers();
#endif

int ClearRxVerifyEdrReturn(void)
{
	l_rxVerifyEdrParamMap.clear();
	l_rxVerifyEdrReturnMap.clear();
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


BT_TEST_API int BT_RX_Verify_EDR(void)
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
	ClearReturnParameters(l_rxVerifyEdrReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_rxVerifyEdrParamMap);
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
        RespondToQueryReturn(l_rxVerifyEdrReturnMap);
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
		err = GetInputParameters(l_rxVerifyEdrParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not completed.\n");
			throw logMessage;
		}		

		// Check path loss (by ant and freq)
		if ( 0==l_rxVerifyEdrParam.CABLE_LOSS_DB )
		{
			err = TM_GetPathLossAtFrequency(g_BT_Test_ID, l_rxVerifyEdrParam.FREQ_MHZ, &l_rxVerifyEdrParam.CABLE_LOSS_DB, 0, RX_TABLE);
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
								  l_rxVerifyEdrParam.PACKET_TYPE,
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
		cableLossDb = l_rxVerifyEdrParam.CABLE_LOSS_DB;

		double VSG_POWER = l_rxVerifyEdrParam.RX_POWER_LEVEL + cableLossDb;
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
		err = LP_SetVsg(l_rxVerifyEdrParam.FREQ_MHZ*1e6, VSG_POWER, g_BTGlobalSettingParam.VSG_PORT);
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
		// Set DUT RF frquency...
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_BT_Dut);

		vDUT_AddStringParameter (g_BT_Dut, "REMOTE_BD_ADDRESS", g_BTGlobalSettingParam.BER_WAVEFORM_BD_ADDRESS);
		vDUT_AddIntegerParameter(g_BT_Dut, "FREQ_MHZ",			l_rxVerifyEdrParam.FREQ_MHZ);	
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
		vDUT_AddIntegerParameter(g_BT_Dut, "LOGIC_CHANNEL",     0);	//{0x00, "ACL  EDR"}
		vDUT_AddStringParameter (g_BT_Dut, "PACKET_TYPE",	    l_rxVerifyEdrParam.PACKET_TYPE);
		// Check packet length 		
		if (0==l_rxVerifyEdrParam.PACKET_LENGTH)
		{
			GetPacketLength("BER", "PACKETS_LENGTH", l_rxVerifyEdrParam.PACKET_TYPE, &packetLength);
		}
		else	
		{
			packetLength = l_rxVerifyEdrParam.PACKET_LENGTH;
		}
		vDUT_AddIntegerParameter(g_BT_Dut, "PACKET_LENGTH",     packetLength);
		vDUT_AddDoubleParameter (g_BT_Dut, "RX_BIT_NUMBER",		l_rxVerifyEdrParam.RX_BIT_NUMBER_LEVEL1);
		vDUT_AddIntegerParameter(g_BT_Dut, "TIMEOUT",			l_rxVerifyEdrParam.TIMEOUT);


		err = vDUT_Run(g_BT_Dut, "RX_CLEAR_STATS");		
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
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
			// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				LP_EnableVsgRF(0);
				throw logMessage;
			}
			else	// Just return normal error message in this case
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
		* Get RX BER Result  *
		*--------------------*/
		double totalPackets = 0;
		double goodPackets  = 0;
		double badPackets   = 0;

		err = vDUT_Run(g_BT_Dut, "RX_GET_STATS");		
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				vDUT_Run(g_BT_Dut, "RX_STOP");		
				LP_EnableVsgRF(0);
				throw logMessage;
			}
			else	// Just return normal error message in this case
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

		l_rxVerifyEdrReturn.RX_POWER_LEVEL = l_rxVerifyEdrParam.RX_POWER_LEVEL;

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
			l_rxVerifyEdrReturn.GOOD_BITS = goodPackets;
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
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Dut report TOTAL_BITS less than 0.\n");
			vDUT_Run(g_BT_Dut, "RX_STOP");		
			LP_EnableVsgRF(0);
			throw logMessage;
		}
		else
		{
			l_rxVerifyEdrReturn.TOTAL_BITS = totalPackets;
		}

		// Sometime Dut get packets from Air
		// workaround for some dut which returns more ack than packet sent
		if ( goodPackets>totalPackets )	
		{
			goodPackets = totalPackets;
			l_rxVerifyEdrReturn.GOOD_BITS  = goodPackets;
			l_rxVerifyEdrReturn.TOTAL_BITS = totalPackets;
		}
		else
		{
			// do nothing	
		}
		
		badPackets = totalPackets - goodPackets;

		if( totalPackets!=0 )
		{
			l_rxVerifyEdrReturn.BER = (badPackets/totalPackets) * 100.0;
		}
		else	// In this case, totalPackets = 0
		{
			l_rxVerifyEdrReturn.BER = 100.0;
		}
#pragma endregion

	   /*--------------------------------------------------------------------------*
		* For EDR_BER test, must check the result can pass the first limits or not *
		*--------------------------------------------------------------------------*/
		// check if received totalpackets reach the RX_BIT_NUMBER
		if ( (l_rxVerifyEdrParam.ENABLE_RX_LEVEL2_TEST)&&
			((l_rxVerifyEdrReturn.BER>=l_rxVerifyEdrParam.RX_BER_THRESHOLD_LIMIT_LEVEL1) ||
			(totalPackets<l_rxVerifyEdrParam.RX_BIT_NUMBER_LEVEL1))
			)	// BER over the EDR LV1_limits
		{
			totalPackets = 0;
			goodPackets  = 0;
			badPackets   = 0;
		   /*----------------------------------------*
			* Get RX BER Result, Configure DUT again *
			*----------------------------------------*/
			// Set DUT RF frquency...
			// And clear vDut parameters at beginning.
			vDUT_ClearParameters(g_BT_Dut);

			vDUT_AddStringParameter (g_BT_Dut, "REMOTE_BD_ADDRESS", g_BTGlobalSettingParam.BER_WAVEFORM_BD_ADDRESS);
			vDUT_AddIntegerParameter(g_BT_Dut, "FREQ_MHZ",			l_rxVerifyEdrParam.FREQ_MHZ);	
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
			vDUT_AddIntegerParameter(g_BT_Dut, "LOGIC_CHANNEL",     0);	//{0x00, "ACL  EDR"}
			vDUT_AddStringParameter (g_BT_Dut, "PACKET_TYPE",	    l_rxVerifyEdrParam.PACKET_TYPE);
			// Check packet length 		
			if (0==l_rxVerifyEdrParam.PACKET_LENGTH)
			{
				GetPacketLength("BER", "PACKETS_LENGTH", l_rxVerifyEdrParam.PACKET_TYPE, &packetLength);
			}
			else	
			{
				packetLength = l_rxVerifyEdrParam.PACKET_LENGTH;
			}
			vDUT_AddIntegerParameter(g_BT_Dut, "PACKET_LENGTH",     packetLength);
			vDUT_AddDoubleParameter (g_BT_Dut, "RX_BIT_NUMBER",		l_rxVerifyEdrParam.RX_BIT_NUMBER_LEVEL2);
			vDUT_AddIntegerParameter(g_BT_Dut, "TIMEOUT",			l_rxVerifyEdrParam.TIMEOUT);

			err = vDUT_Run(g_BT_Dut, "RX_GET_STATS");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					vDUT_Run(g_BT_Dut, "RX_STOP");		
					LP_EnableVsgRF(0);
					throw logMessage;
				}
				else	// Just return normal error message in this case
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

			l_rxVerifyEdrReturn.RX_POWER_LEVEL = l_rxVerifyEdrParam.RX_POWER_LEVEL;

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
				l_rxVerifyEdrReturn.GOOD_BITS = goodPackets;
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
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Dut report TOTAL_BITS less than 0.\n");
				vDUT_Run(g_BT_Dut, "RX_STOP");		
				LP_EnableVsgRF(0);
				throw logMessage;
			}
			else
			{
				l_rxVerifyEdrReturn.TOTAL_BITS = totalPackets;
			}

			// Sometime Dut get packets from Air
			// workaround for some dut which returns more ack than packet sent
			if ( goodPackets>totalPackets )	
			{
				goodPackets = totalPackets;
				l_rxVerifyEdrReturn.GOOD_BITS  = goodPackets;
				l_rxVerifyEdrReturn.TOTAL_BITS = totalPackets;
			}
			else
			{
				// do nothing	
			}
			
			badPackets = totalPackets - goodPackets;

			if( totalPackets!=0 )
			{
				l_rxVerifyEdrReturn.BER = (badPackets/totalPackets) * 100.0;
			}
			else	// In this case, totalPackets = 0
			{
				l_rxVerifyEdrReturn.BER = 100.0;
			}
			// check if received totalpackets reach the RX_BIT_NUMBER
			if (totalPackets<l_rxVerifyEdrParam.RX_BIT_NUMBER_LEVEL2) {
				err = -1;
				ReturnTestResults(l_rxVerifyEdrReturnMap);
				ReturnErrorMessage(l_rxVerifyEdrReturn.ERROR_MESSAGE, "Receive timeout!(LV2) Totalbits: %02.f",totalPackets);
				vDUT_Run(g_BT_Dut, "RX_STOP");		
				LP_EnableVsgRF(0);
				throw logMessage;
			}
		}
		else
		{
			// check if received totalpackets reach the RX_BIT_NUMBER
			if (!l_rxVerifyEdrParam.ENABLE_RX_LEVEL2_TEST) {
				if (totalPackets<l_rxVerifyEdrParam.RX_BIT_NUMBER_LEVEL1) {
					err = -1;
					ReturnTestResults(l_rxVerifyEdrReturnMap);
					ReturnErrorMessage(l_rxVerifyEdrReturn.ERROR_MESSAGE, "Receive timeout!(LV1) Totalbits: %02.f",totalPackets);
					vDUT_Run(g_BT_Dut, "RX_STOP");		
					LP_EnableVsgRF(0);
					throw logMessage;
				}
			}
		}

		/*--------------------*
		* Get RX RSSI Result  *
		*--------------------*/
		err = ::vDUT_GetDoubleReturn(g_BT_Dut, "RSSI", &l_rxVerifyEdrReturn.RSSI); 
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// TODO: Since we only report "LOGGER_WARNING", thus must reset it to "ERR_OK".
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[BT] WARNING, This DUT does not support RSSI measurement.\n");
			l_rxVerifyEdrReturn.RSSI = NA_NUMBER;
			//throw logMessage;
		}

	   /*-----------*
		*  Rx Stop  *
		*-----------*/
		err = vDUT_Run(g_BT_Dut, "RX_STOP");		
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
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
			l_rxVerifyEdrReturn.CABLE_LOSS_DB = l_rxVerifyEdrParam.CABLE_LOSS_DB;

			sprintf_s(l_rxVerifyEdrReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_rxVerifyEdrReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_rxVerifyEdrReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_rxVerifyEdrReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
    }

	// This is a special case, only when some error occur before the RX_STOP. 
	// This case will take care by the error handling, but must do RX_STOP manually.
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

int InitializeRXVerifyEDRContainers(void)
{
    /*-----------------*
     * Input Paramters *
     *-----------------*/
    l_rxVerifyEdrParamMap.clear();

    BT_SETTING_STRUCT setting;

    l_rxVerifyEdrParam.ENABLE_RX_LEVEL2_TEST = 1;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerifyEdrParam.ENABLE_RX_LEVEL2_TEST))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerifyEdrParam.ENABLE_RX_LEVEL2_TEST;
        setting.unit        = "";
        setting.helpText    = "A flag to enable the RX EDR Level-2 test. Default = 1 (Enable).";
        l_rxVerifyEdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("ENABLE_RX_LEVEL2_TEST", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyEdrParam.RX_POWER_LEVEL = -70.0;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyEdrParam.RX_POWER_LEVEL))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyEdrParam.RX_POWER_LEVEL;
        setting.unit        = "dBm";
        setting.helpText    = "The output power to verify EDR.";
        l_rxVerifyEdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("RX_POWER_LEVEL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerifyEdrParam.FREQ_MHZ = 2402;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerifyEdrParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerifyEdrParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz";
        l_rxVerifyEdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_MHZ", setting) );
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

    strcpy_s(l_rxVerifyEdrParam.PACKET_TYPE, MAX_BUFFER_SIZE, "2DH1");
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyEdrParam.PACKET_TYPE))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerifyEdrParam.PACKET_TYPE;
        setting.unit        = "";
        setting.helpText    = "Sets the packet type, the type can be 2DH1, 2DH3, 2DH5, 3DH1, 3DH3, 3DH5. default is 2DH1";
        l_rxVerifyEdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyEdrParam.PACKET_LENGTH = 0;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerifyEdrParam.PACKET_LENGTH))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerifyEdrParam.PACKET_LENGTH;
        setting.unit        = "";
        setting.helpText    = "The number of payload length to verify EDR.";
        l_rxVerifyEdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_LENGTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyEdrParam.TIMEOUT = 20;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_rxVerifyEdrParam.TIMEOUT))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyEdrParam.TIMEOUT;
		setting.unit        = "sec";
		setting.helpText    = "The timeout setting for receive. Set to 0 for forever waiting";
		l_rxVerifyEdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("TIMEOUT", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyEdrParam.RX_BER_THRESHOLD_LIMIT_LEVEL1 = 0.007;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyEdrParam.RX_BER_THRESHOLD_LIMIT_LEVEL1))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyEdrParam.RX_BER_THRESHOLD_LIMIT_LEVEL1;
        setting.unit        = "%";
        setting.helpText    = "A threshold of BER test for EDR only, BER result must pass this limit or tester need to continue send out packets until reach RX_BIT_NUMBER_LEVEL2 bits.";
        l_rxVerifyEdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("RX_BER_THRESHOLD_LIMIT_LEVEL1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


	l_rxVerifyEdrParam.RX_BIT_NUMBER_LEVEL1 = 1600000;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyEdrParam.RX_BIT_NUMBER_LEVEL1))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyEdrParam.RX_BIT_NUMBER_LEVEL1;
        setting.unit        = "";
        setting.helpText    = "The minimum number of payload bits. Default value is 1,600,000.";
        l_rxVerifyEdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("RX_BIT_NUMBER_LEVEL1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyEdrParam.RX_BIT_NUMBER_LEVEL2 = 16000000;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyEdrParam.RX_BIT_NUMBER_LEVEL2))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyEdrParam.RX_BIT_NUMBER_LEVEL2;
        setting.unit        = "";
        setting.helpText    = "The minimum number of payload bits. Default value is 16,000,000.";
        l_rxVerifyEdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("RX_BIT_NUMBER_LEVEL2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyEdrParam.CABLE_LOSS_DB = 0.0;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyEdrParam.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyEdrParam.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_rxVerifyEdrParamMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
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
    l_rxVerifyEdrReturnMap.clear();

	l_rxVerifyEdrReturn.TOTAL_BITS = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyEdrReturn.TOTAL_BITS))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyEdrReturn.TOTAL_BITS;
        setting.unit        = "bits";
        setting.helpText    = "TOTAL_BITS is the total number of bits that reported from Dut.";
        l_rxVerifyEdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("TOTAL_BITS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyEdrReturn.GOOD_BITS = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyEdrReturn.GOOD_BITS))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyEdrReturn.GOOD_BITS;
        setting.unit        = "bits";
        setting.helpText    = "GOOD_BITS is the number of good bits that reported from Dut.";
        l_rxVerifyEdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("GOOD_BITS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerifyEdrReturn.BER = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyEdrReturn.BER))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyEdrReturn.BER;
        setting.unit        = "%";
        setting.helpText    = "BER over received packets.";
        l_rxVerifyEdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("BER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyEdrReturn.RSSI = NA_DOUBLE;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyEdrReturn.RSSI))    // Type_Checking
	{
		setting.value = (void*)&l_rxVerifyEdrReturn.RSSI;
		setting.unit        = "";
		setting.helpText    = "RSSI result when received packets.";
		l_rxVerifyEdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("RSSI", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_rxVerifyEdrReturn.RX_POWER_LEVEL = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyEdrReturn.RX_POWER_LEVEL))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyEdrReturn.RX_POWER_LEVEL;
        setting.unit        = "dBm";
        setting.helpText    = "RX power level for BER";
        l_rxVerifyEdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("RX_POWER_LEVEL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyEdrReturn.CABLE_LOSS_DB = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyEdrReturn.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyEdrReturn.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_rxVerifyEdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_rxVerifyEdrReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyEdrReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerifyEdrReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_rxVerifyEdrReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}


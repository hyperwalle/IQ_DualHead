#include "stdafx.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;

// These global variables are declared in BT_Test_Internal.cpp
extern TM_ID                 g_BT_Test_ID;    
extern vDUT_ID               g_BT_Dut;
extern bool					 g_vDutTxActived;
extern bool					 g_vDutRxActived;

// This global variable is declared in BT_Global_Setting.cpp
extern BT_GLOBAL_SETTING g_BTGlobalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, BT_SETTING_STRUCT> l_CurrentTestParamMap;

// Return Value Container
map<string, BT_SETTING_STRUCT> l_CurrentTestReturnMap;


struct tagParam
{
	// Mandatory Parameters
	char   DUT_POWER_MODE[MAX_BUFFER_SIZE];            /*! DUT power mode.                     */
	char   DUT_OPERATION[MAX_BUFFER_SIZE];             /*! DUT operation, start or stop        */
	//for general parameters
	int    FREQ_MHZ;								   /*! The center frequency (MHz).         */
	char   PACKET_TYPE[MAX_BUFFER_SIZE];			   /*! The pack type for tx mode.          */
	int    PACKET_LENGTH;						       /*! The number of packet for tx mode.   */
	int    TX_POWER_LEVEL;						       /*! The output power for tx mode.       */
	double CABLE_LOSS_DB;						       /*! The path loss of test system.       */
	double SAMPLING_TIME_US;						   /*! The sampling time for tx mode.      */ 
	double RX_POWER_LEVEL;						       /*! The output power for rx mode.       */
} l_CurrentTestParam;

struct tagReturn
{   
	double CABLE_LOSS_DB;                              /*! The path loss of test system.       */
	double TARGET_POWER_DBM;
	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_CurrentTestReturn;
#pragma endregion

#ifndef WIN32
int initPowerModeDutContainers = InitializePowerModeDutContainers();
#endif

int ClearCurrentTestReturn(void)
{
	l_CurrentTestParamMap.clear();
	l_CurrentTestReturnMap.clear();
	return 0;
}

//! BT TX Power Mode DUT
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

enum E_CURR_TEST_MODE{E_CURR_TEST_TX, E_CURR_TEST_RX,E_CURR_TEST_SLEEP,E_CURR_TEST_IDLE,E_CURR_TEST_ERROR};
enum E_CURR_TEST_OPERATION{E_OPERATION_START, E_OPERATION_STOP,E_OPERATION_ERROR};

BT_TEST_API int BT_Power_Mode_Dut(void)
{
	int    err = ERR_OK;
	int    dummyValue  = 0;
	double cableLossDb = 0;
	char   vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
//	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};
	int	   packetLength = 0;

	int  current_test_mode = E_CURR_TEST_ERROR;
	int  current_operation_mode = E_OPERATION_ERROR;
	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_CurrentTestReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_CurrentTestParamMap);
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
		RespondToQueryReturn(l_CurrentTestReturnMap);
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
		if( g_BT_Test_ID<0 || g_BT_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] BT_Test_ID or BT_Dut not valid. BT_Test_ID = %d and BT_Dut = %d.\n", g_BT_Test_ID, g_BT_Dut);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] BT_Test_ID = %d and BT_Dut = %d.\n", g_BT_Test_ID, g_BT_Dut);
		}

		TM_ClearReturns(g_BT_Test_ID);

		/*----------------------*
		* Get input parameters *
		*----------------------*/
		err = GetInputParameters(l_CurrentTestParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Get input parameters return OK.\n");
		}

		// Check path loss (by ant and freq), default using TX_TABLE.
		if ( 0==l_CurrentTestParam.CABLE_LOSS_DB )
		{
			err = TM_GetPathLossAtFrequency(g_BT_Dut, l_CurrentTestParam.FREQ_MHZ, &l_CurrentTestParam.CABLE_LOSS_DB, 0, TX_TABLE);
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

#pragma region Prepare input parameters
		//check Current Test mode
		if(strcmp(l_CurrentTestParam.DUT_POWER_MODE,"TX") == 0)	
		{
			current_test_mode = E_CURR_TEST_TX;

			// Check path loss (by ant and freq)
			if ( 0==l_CurrentTestParam.CABLE_LOSS_DB )
			{
				err = TM_GetPathLossAtFrequency(g_BT_Dut, l_CurrentTestParam.FREQ_MHZ, &l_CurrentTestParam.CABLE_LOSS_DB, 0, TX_TABLE);
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
		}
		else if(strcmp(l_CurrentTestParam.DUT_POWER_MODE,"RX") == 0)
		{
			current_test_mode = E_CURR_TEST_RX;		

			// Check path loss (by ant and freq)
			if ( 0==l_CurrentTestParam.CABLE_LOSS_DB )
			{
				err = TM_GetPathLossAtFrequency(g_BT_Dut, l_CurrentTestParam.FREQ_MHZ, &l_CurrentTestParam.CABLE_LOSS_DB, 0, RX_TABLE);
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
		}	
		else if(strcmp(l_CurrentTestParam.DUT_POWER_MODE,"SLEEP") == 0)
		{
			current_test_mode = E_CURR_TEST_SLEEP;
		}
		else if(strcmp(l_CurrentTestParam.DUT_POWER_MODE,"IDLE") == 0)
		{
			current_test_mode = E_CURR_TEST_IDLE;
		}
		if(current_test_mode == E_CURR_TEST_ERROR)
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Invalid CURRENT TEST MODE %s.\n", l_CurrentTestParam.DUT_POWER_MODE);
			throw logMessage;		
		}
		else
		{
			// do nothing
		}

		if(strcmp(l_CurrentTestParam.DUT_OPERATION,"START") == 0)
		{
			current_operation_mode = E_OPERATION_START;
		}
		else if(strcmp(l_CurrentTestParam.DUT_OPERATION,"STOP") == 0)
		{
			current_operation_mode = E_OPERATION_STOP;		
		}
		if(current_operation_mode == E_CURR_TEST_ERROR)
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Invalid OPERATION MODE %s.\n", l_CurrentTestParam.DUT_OPERATION);
			throw logMessage;		
		}
		else
		{
			// do nothing
		}

#pragma endregion

#pragma region Start TX current test			
		//#pragma region Configure DUT to transmit
		/*---------------------------*
		* Configure DUT to transmit *
		*---------------------------*/
		// Set DUT RF frequency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_BT_Dut);
		if((current_test_mode == E_CURR_TEST_TX)&&(current_operation_mode == E_OPERATION_START))
		{
			if( g_vDutRxActived==true )
			{
				err = ::LP_SetFrameCnt(1);		//stop VSG
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Set VSG send out one packet LP_SetFrameCnt(1) failed.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Set VSG send out one packet LP_SetFrameCnt(1) return OK.\n");
				}

				err = vDUT_Run(g_BT_Dut, "RX_STOP");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
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
					g_vDutRxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_Run(RX_STOP) return OK.\n");
				}
			}			

			/*-------------------------------------------*
			* Configure DUT to transmit - PRBS9 Pattern *
			*-------------------------------------------*/
			// Set DUT RF frequency, tx power, data rate
			// And clear vDut parameters at beginning.
			vDUT_ClearParameters(g_BT_Dut);

			vDUT_AddIntegerParameter(g_BT_Dut, "FREQ_MHZ",			l_CurrentTestParam.FREQ_MHZ);	
			// Description				Enum in the IQFact
			// 0x00 8-bit Pattern		ZERO =0: all zero
			// 0xFF 8-bit Pattern       ONE1 =1: all ones
			// 0xAA 8-bit Pattern       HEX_A=2: 1010
			// 0xF0 8-bit Pattern		HEX_F0=3: 11110000
			// PRBS9 Pattern            RANDOM=4;  PRBS=0
			vDUT_AddIntegerParameter(g_BT_Dut, "MODULATION_TYPE",   BT_PATTERN_PRBS);	//{0x04, "PRBS9 Pattern"}
			// {0x00, "ACL  EDR"},
			// {0x01, "ACL  Basic"},
			// {0x02, "eSCO EDR"},
			// {0x03, "eSCO Basic"},
			// {0x04, "SCO  Basic"}
			vDUT_AddIntegerParameter(g_BT_Dut, "LOGIC_CHANNEL",     0);	//{0x00, "ACL  EDR"}
			vDUT_AddStringParameter (g_BT_Dut, "PACKET_TYPE",	    l_CurrentTestParam.PACKET_TYPE);
			// Check packet length 		
			if (0==l_CurrentTestParam.PACKET_LENGTH)
			{
				GetPacketLength("BER", "PACKETS_LENGTH", l_CurrentTestParam.PACKET_TYPE, &packetLength);
			}
			else	
			{
				packetLength = l_CurrentTestParam.PACKET_LENGTH;
			}
			vDUT_AddIntegerParameter(g_BT_Dut, "PACKET_LENGTH",     packetLength);
			vDUT_AddIntegerParameter(g_BT_Dut, "TX_POWER_LEVEL",	l_CurrentTestParam.TX_POWER_LEVEL);

			err = vDUT_Run(g_BT_Dut, "QUERY_POWER_DBM");
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(QUERY_POWER_DBM) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				// do nothing
			}

			err = vDUT_GetDoubleReturn(g_BT_Dut, "POWER_DBM", &l_CurrentTestReturn.TARGET_POWER_DBM);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_GetDoubleReturn(POWER_DBM) return error.\n");
				throw logMessage;
			}

			err = vDUT_Run(g_BT_Dut, "TX_START");
			if ( ERR_OK!=err )
			{	
				// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_START) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = true;
			}
		}	
#pragma endregion

#pragma region STOP DUT TX CURRENT test
		else if((current_test_mode == E_CURR_TEST_TX) &&(current_operation_mode == E_OPERATION_STOP))
		{
			/*-----------*
			*  Tx Dtop  *
			*-----------*/
			err = vDUT_Run(g_BT_Dut, "TX_STOP");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = false;
			}
		}

#pragma endregion

#pragma region Start RX CURRENT test

		else if((current_test_mode == E_CURR_TEST_RX)&&(current_operation_mode == E_OPERATION_START))
		{
			if ( g_vDutTxActived==true )
			{
				/*-----------*
				*  Tx Stop  *
				*-----------*/
				err = ::vDUT_Run(g_BT_Dut, "TX_STOP");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_Run(TX_STOP) return OK.\n");
				}
			}
			else
			{
				// continue Dut configuration				
			}
#pragma region Setup LP Tester
			/*----------------------------*
			* Disable VSG output signal  *
			*----------------------------*/
			// make sure no signal coming out VSG
			err = LP_EnableVsgRF(0);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to turn off VSG.\n");
				throw logMessage;
			}	

			/*---------------------*
			* Load waveform Files *
			*---------------------*/
			char   modFile[MAX_BUFFER_SIZE] = {'\0'};
			err = GetWaveformFileName("BER",		
				"WAVEFORM_NAME", 
				l_CurrentTestParam.PACKET_TYPE,
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

			/*--------------------*
			* Setup IQTester VSG *
			*--------------------*/
			cableLossDb = l_CurrentTestParam.CABLE_LOSS_DB;

			err = LP_SetVsg(l_CurrentTestParam.FREQ_MHZ*1e6, l_CurrentTestParam.RX_POWER_LEVEL+cableLossDb, g_BTGlobalSettingParam.VSG_PORT);
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

			/*--------------------------*
			* Send packet for BER Test *
			*--------------------------*/           
			err = LP_SetFrameCnt(0);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] VSG send out packet failed.\n");
				throw logMessage;
			}		

#pragma endregion

#pragma region Configure DUT to Receive
			/*------------------------------------------*
			* Configure DUT to Receive - PRBS9 Pattern *
			*------------------------------------------*/
			// Set DUT RF frequency...
			// And clear vDut parameters at beginning.
			vDUT_ClearParameters(g_BT_Dut);

			vDUT_AddStringParameter (g_BT_Dut, "REMOTE_BD_ADDRESS", g_BTGlobalSettingParam.BER_WAVEFORM_BD_ADDRESS);
			vDUT_AddIntegerParameter(g_BT_Dut, "FREQ_MHZ",			l_CurrentTestParam.FREQ_MHZ);	
			// Description				Enum in the IQFact
			// 0x00 8-bit Pattern		ZERO =0:   all zero
			// 0xFF 8-bit Pattern       ONE1 =1:   all ones
			// 0xAA 8-bit Pattern       HEX_A=2:   1010
			// 0xF0 8-bit Pattern		HEX_F0=3:  11110000
			// PRBS9 Pattern            RANDOM=4;  PRBS=0
			vDUT_AddIntegerParameter(g_BT_Dut, "MODULATION_TYPE",   BT_PATTERN_PRBS);	//{0x04, "PRBS9 Pattern"}
			// {0x00, "ACL  EDR"},
			// {0x01, "ACL  Basic"},
			// {0x02, "eSCO EDR"},
			// {0x03, "eSCO Basic"},
			// {0x04, "SCO  Basic"}
			vDUT_AddIntegerParameter(g_BT_Dut, "LOGIC_CHANNEL",     0);	//{0x00, "ACL  EDR"}
			vDUT_AddStringParameter (g_BT_Dut, "PACKET_TYPE",	    l_CurrentTestParam.PACKET_TYPE);
			// Check packet length 		
			if (0==l_CurrentTestParam.PACKET_LENGTH)
			{
				GetPacketLength("BER", "PACKETS_LENGTH", l_CurrentTestParam.PACKET_TYPE, &packetLength);
			}
			else	
			{
				packetLength = l_CurrentTestParam.PACKET_LENGTH;
			}
			vDUT_AddIntegerParameter(g_BT_Dut, "PACKET_LENGTH",     packetLength);

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
				// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(RX_START) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutRxActived = true;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_Run(RX_START) return OK.\n");
			}
#pragma endregion
		}
#pragma endregion

#pragma region Stop RX CURRENT test
		/*-----------*
		*  Rx Stop  *
		*-----------*/
		else if((current_test_mode == E_CURR_TEST_RX)&&(current_operation_mode == E_OPERATION_STOP))
		{
			err = ::LP_SetFrameCnt(1);		//stop VSG
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Set VSG send out one packet LP_SetFrameCnt(1) failed.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Set VSG send out one packet LP_SetFrameCnt(1) return OK.\n");
			}

			err = vDUT_Run(g_BT_Dut, "RX_STOP");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
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
				g_vDutRxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] vDUT_Run(RX_STOP) return OK.\n");
			}
		}
#pragma endregion			

#pragma region START SLEEP CURRENT test
		//John: 8688 don't support SLEEP mode ( at least 8686 dont support, maybe supported by 8688?
		//anyway, just return an error code for now
		else if(current_test_mode == E_CURR_TEST_SLEEP)			
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] SLEEP MODE not implemented for this chip.\n");
			throw logMessage;
		}
#pragma endregion	

#pragma region START IDLE CURRENT test
		else if(current_test_mode == E_CURR_TEST_IDLE)
		{
			//for IDLE test, do nothing

		}
#pragma endregion	
		/*-----------------------*
		*  Return Test Results  *
		*-----------------------*/
		if ( ERR_OK==err )
		{
			sprintf_s(l_CurrentTestReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults( l_CurrentTestReturnMap);
		}
		else
		{
			// do nothing
		}

	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_CurrentTestReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_CurrentTestReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
	}


	return err;
}

int InitializePowerModeDutContainers(void)
{
	/*------------------*
	* Input Parameters  *
	*------------------*/
	l_CurrentTestParamMap.clear();

	BT_SETTING_STRUCT setting;

	strcpy_s(l_CurrentTestParam.DUT_POWER_MODE, MAX_BUFFER_SIZE, "TX");
	setting.type = BT_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestParam.DUT_POWER_MODE))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestParam.DUT_POWER_MODE;
		setting.unit        = "";
		setting.helpText    = "DUT POWER MODE: TX , RX, SLEEP or IDLE";
		l_CurrentTestParamMap.insert( pair<string,BT_SETTING_STRUCT>("DUT_POWER_MODE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_CurrentTestParam.DUT_OPERATION, MAX_BUFFER_SIZE, "START");
	setting.type = BT_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestParam.DUT_OPERATION))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestParam.DUT_OPERATION;
		setting.unit        = "";
		setting.helpText    = "DUT OPERATION: START or STOP";
		l_CurrentTestParamMap.insert( pair<string,BT_SETTING_STRUCT>("DUT_OPERATION", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_CurrentTestParam.FREQ_MHZ = 2402;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_CurrentTestParam.FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_CurrentTestParam.FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "Channel center frequency in MHz";
		l_CurrentTestParamMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_MHZ", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	//{0x00, "NULL"},
	//{0x01, "POLL"},
	//{0x02, "FHS"},
	//{0x03, "DM1"},
	//{0x04, "DH1 / 2DH1"},
	//{0x05, "HV1"},
	//{0x06, "HV2 / 2-EV3"},
	//{0x07, "HV3 / EV3 / 3-EV3"},
	//{0x08, "DV / 3DH1"},
	//{0x09, "AUX1 / PS"},
	//{0x0A, "DM3 / 2DH3"},
	//{0x0B, "DH3 / 3DH3"},
	//{0x0C, "EV4 / 2-EV5"},
	//{0x0D, "EV5 / 3-EV5"},
	//{0x0E, "DM5 / 2DH5"},
	//{0x0F, "DH5 / 3DH5"}

	strcpy_s(l_CurrentTestParam.PACKET_TYPE, MAX_BUFFER_SIZE, "1DH1");
	setting.type = BT_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestParam.PACKET_TYPE))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestParam.PACKET_TYPE;
		setting.unit        = "";
		setting.helpText    = "Sets the packet type, the type can be 2DH1, 2DH3, 2DH5, 3DH1, 3DH3, 3DH5. default is 2DH1";
		l_CurrentTestParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_TYPE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_CurrentTestParam.PACKET_LENGTH = 0;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_CurrentTestParam.PACKET_LENGTH))    // Type_Checking
	{
		setting.value       = (void*)&l_CurrentTestParam.PACKET_LENGTH;
		setting.unit        = "";
		setting.helpText    = "The number of packet to verify EDR.";
		l_CurrentTestParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_LENGTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_CurrentTestParam.TX_POWER_LEVEL = 0;
	setting.type = BT_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_CurrentTestParam.TX_POWER_LEVEL))    // Type_Checking
	{
		setting.value = (void*)&l_CurrentTestParam.TX_POWER_LEVEL;
		setting.unit        = "Level";
		setting.helpText    = "Expected power level at DUT antenna port. Level can be 0, 1, 2, 3, 4, 5, 6 and 7 (MaxPower => MinPower, if supported.)";
		l_CurrentTestParamMap.insert( pair<string,BT_SETTING_STRUCT>("TX_POWER_LEVEL", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_CurrentTestParam.CABLE_LOSS_DB = 0.0;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_CurrentTestParam.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_CurrentTestParam.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_CurrentTestParamMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
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
	l_CurrentTestReturnMap.clear();

	l_CurrentTestReturn.CABLE_LOSS_DB = NA_DOUBLE;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_CurrentTestReturn.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_CurrentTestReturn.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_CurrentTestReturnMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_CurrentTestReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = BT_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_CurrentTestReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return 0;
}


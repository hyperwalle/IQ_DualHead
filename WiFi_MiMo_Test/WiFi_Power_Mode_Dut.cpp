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

// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_globalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_CurrentTestParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_CurrentTestReturnMap;


struct tagParam
{
	// Mandatory Parameters
	
	char DUT_POWER_MODE[MAX_BUFFER_SIZE];            /*! DUT power mode. */
	char DUT_OPERATION[MAX_BUFFER_SIZE];             /*! DUT operation, start or stop */
	//for general parameters
	int    FREQ_MHZ;                                /*! The center frequency (MHz). */
	char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify POWER. */
	char   DATA_RATE[MAX_BUFFER_SIZE];				/*! The data rate to verify POWER. */
	double POWER_DBM;                                /*! The output power . */

	//for TX current test
	
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];      /*! The packet format of 11N(only). */
	
	// IQlite merge; Tracy Yu ; 2012-03-31
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];     /*! The guard interval format of 11N(only). */
	
	//double T_INTERVAL;                              /*! This field is used to specify the interval that is used to determine if power is present or not. */
	//double MAX_POW_DIFF_DB;                         /*! This field is used to specify the maximum power difference between packets that are expected to be detected. */

	// DUT Parameters
	int    ATTEN1;                                     /*!< DUT attenuator1 on/off. Default=1(on) */
	int    ATTEN2;                                     /*!< DUT attenuator2 on/off. Default=0(off) */
	int    ATTEN3;                                     /*!< DUT attenuator3 on/off. Default=0(off) */
	int    ATTEN4;                                     /*!< DUT attenuator4 on/off. Default=0(off) */
} l_CurrentTestParam;

struct tagReturn
{   
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_CurrentTestReturn;
#pragma endregion

#ifndef WIN32
int initPowerModeDutMIMOContainers = InitializePowerModeDutContainers();
#endif

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

enum E_CURR_TEST_MODE{E_CURR_TEST_TX, E_CURR_TEST_RX,E_CURR_TEST_SLEEP,E_CURR_TEST_IDLE,E_CURR_TEST_ERROR};
enum E_CURR_TEST_OPERATION{E_OPERATION_START, E_OPERATION_STOP,E_OPERATION_ERROR};

WIFI_MIMO_TEST_API int WiFi_Power_Mode_Dut(void)
{
	int    err = ERR_OK;

//	bool   analysisOK = false, captureOK  = false;     
//	int    avgIteration = 0;
	int    channel = 0, HT40ModeOn = 0;
	int    dummyValue = 0;
	int    wifiMode = 0, wifiStreamNum = 0;
//	double samplingTimeUs = 0;
	double cableLossDb = 0;
	char   vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
//	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};

    int  current_test_mode = E_CURR_TEST_ERROR;
	int  current_operation_mode = E_OPERATION_ERROR;
	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_CurrentTestReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
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
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_RETURN", &dummyValue);
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
		err = GetInputParameters(l_CurrentTestParamMap);
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

#pragma region Prepare input parameters
		// Convert freq to channel number
		err = ::TM_WiFiConvertFrequencyToChannel(l_CurrentTestParam.FREQ_MHZ, &channel);      
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Convert WiFi frequency to channel function failed.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		err = WiFiTestMode(l_CurrentTestParam.DATA_RATE, l_CurrentTestParam.BANDWIDTH, &wifiMode, &wifiStreamNum);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Convert WiFi test mode function failed.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFiTestMode() return OK.\n");
		}

		// Check path loss (by ant and freq)
		err = CheckPathLossTableExt(	g_WiFi_Test_ID,
										l_CurrentTestParam.FREQ_MHZ,
										l_CurrentTestParam.ATTEN1,
										l_CurrentTestParam.ATTEN2,
										l_CurrentTestParam.ATTEN3,
										l_CurrentTestParam.ATTEN4,
										l_CurrentTestParam.CABLE_LOSS_DB,
										l_CurrentTestReturn.CABLE_LOSS_DB,			
										&cableLossDb,
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

		//check Current Test mode
		if(strcmp(l_CurrentTestParam.DUT_POWER_MODE,"TX") == 0)			
			current_test_mode = E_CURR_TEST_TX;
		else if(strcmp(l_CurrentTestParam.DUT_POWER_MODE,"RX") == 0)
			current_test_mode = E_CURR_TEST_RX;		
		else if(strcmp(l_CurrentTestParam.DUT_POWER_MODE,"SLEEP") == 0)
			current_test_mode = E_CURR_TEST_SLEEP;
		else if(strcmp(l_CurrentTestParam.DUT_POWER_MODE,"IDLE") == 0)
			current_test_mode = E_CURR_TEST_IDLE;
		if(current_test_mode == E_CURR_TEST_ERROR)
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Invalid CURRENT TEST MODE %s.\n", l_CurrentTestParam.DUT_POWER_MODE);
			throw logMessage;		
		}
		else
		{
			// do nothing
		}

		if(strcmp(l_CurrentTestParam.DUT_OPERATION,"START") == 0)
			current_operation_mode = E_OPERATION_START;
		else if(strcmp(l_CurrentTestParam.DUT_OPERATION,"STOP") == 0)
			current_operation_mode = E_OPERATION_STOP;		
		if(current_operation_mode == E_CURR_TEST_ERROR)
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Invalid OPERATION MODE %s.\n", l_CurrentTestParam.DUT_OPERATION);
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
		// Set DUT RF frquency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);
	   if((current_test_mode == E_CURR_TEST_TX)&&(current_operation_mode == E_OPERATION_START))
	   {
		   if( wifiMode==WIFI_11N_HT40 )
		   {
			   HT40ModeOn = 1;   // 1: HT40 mode;
			   vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_CurrentTestParam.FREQ_MHZ);
			   vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_CurrentTestParam.FREQ_MHZ-10);
			   vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_CurrentTestParam.FREQ_MHZ+10);
		   }
		   else
		   {
			   HT40ModeOn = 0;   // 0: Normal 20MHz mode 
			   vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",			l_CurrentTestParam.FREQ_MHZ);
		   }		  
		   vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",				l_CurrentTestParam.PREAMBLE);
		   vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N",	l_CurrentTestParam.PACKET_FORMAT_11N);
		   // IQlite merge; Tracy Yu ; 2012-03-31
		   vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N",	l_CurrentTestParam.GUARD_INTERVAL_11N);
		   vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",			l_CurrentTestParam.POWER_DBM);
		   vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",			l_CurrentTestParam.DATA_RATE);
		   vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",			HT40ModeOn);
		   vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				l_CurrentTestParam.ATTEN1);
		   vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				l_CurrentTestParam.ATTEN2);
		   vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				l_CurrentTestParam.ATTEN3);
		   vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				l_CurrentTestParam.ATTEN4);

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
	   }	
#pragma endregion

#pragma region STOP DUT TX CURRENT test
	   else if((current_test_mode == E_CURR_TEST_TX) &&(current_operation_mode == E_OPERATION_STOP))
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
		

#pragma endregion
			
#pragma region Start RX CURRENT test

	   else if((current_test_mode == E_CURR_TEST_RX)
		   &&(current_operation_mode == E_OPERATION_START))
	   {
			if( wifiMode==WIFI_11N_HT40 )
			{
				HT40ModeOn = 1;   // 1: HT40 mode;
				vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_CurrentTestParam.FREQ_MHZ);
				vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_CurrentTestParam.FREQ_MHZ-10);
				vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_CurrentTestParam.FREQ_MHZ+10);
			}
			else
			{
				HT40ModeOn = 0;   // 0: Normal 20MHz mode 
				vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",	  l_CurrentTestParam.FREQ_MHZ);
			}
			vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);    
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",		  l_CurrentTestParam.FREQ_MHZ);
			vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_CurrentTestParam.DATA_RATE);
			vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_CurrentTestParam.PREAMBLE);
			vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_CurrentTestParam.PACKET_FORMAT_11N);
			// IQlite merge; Tracy Yu ; 2012-03-31
			vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N",l_CurrentTestParam.GUARD_INTERVAL_11N);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "RX1",			      l_CurrentTestParam.ATTEN1);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "RX2",				  l_CurrentTestParam.ATTEN2);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "RX3",				  l_CurrentTestParam.ATTEN3);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "RX4",				  l_CurrentTestParam.ATTEN4);

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
#pragma region Setup LP Tester
			/*----------------------------*
			* Disable VSG output signal  *
			*----------------------------*/
			// make sure no signal coming out VSG
			err = ::LP_EnableVsgRF(0);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn off VSG.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
			}

			/*---------------------*
			* Load waveform Files *
			*---------------------*/
			char   modFile[MAX_BUFFER_SIZE] = {'\0'};
			err = GetWaveformFileName(  "PER", 
										"WAVEFORM_NAME", 
										wifiMode, 
										l_CurrentTestParam.BANDWIDTH, 
										l_CurrentTestParam.DATA_RATE, 
										l_CurrentTestParam.PREAMBLE, 
										l_CurrentTestParam.PACKET_FORMAT_11N, 
										l_CurrentTestParam.GUARD_INTERVAL_11N,  // IQlite merge; Tracy Yu ; 2012-03-31
										modFile, 
										MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get waveform file name.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get waveform file name GetWaveformFileName() return OK.\n");
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
			if      (l_CurrentTestParam.ATTEN1==1)	antSelection = 0;
			else if (l_CurrentTestParam.ATTEN2==1)	antSelection = 1;
			else if (l_CurrentTestParam.ATTEN3==1)	antSelection = 2;
			else if (l_CurrentTestParam.ATTEN4==1)	antSelection = 3;
			else    							    antSelection = 0;	
			err = ::LP_SetVsg(l_CurrentTestParam.FREQ_MHZ*1e6, l_CurrentTestParam.POWER_DBM+l_CurrentTestParam.CABLE_LOSS_DB[antSelection], g_globalSettingParam.VSG_PORT);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to setup VSG.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Setup VSG LP_SetVsg() return OK.\n");
			}

			/*---------------*
			*  Turn on VSG  *
			*---------------*/
			err = ::LP_EnableVsgRF(1);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn on VSG.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Turn on LP_EnableVsgRF(1) return OK.\n");
			}

			/*-----------------------------*
			* Send one packet for warm up *
			*-----------------------------*/           
			err = ::LP_SetFrameCnt(1);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] VSG send out packet failed.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] VSG send out one packet for warm up LP_SetFrameCnt(1) return OK.\n");
			}
			// After warm up, no signal coming out from VSG
#pragma endregion

#pragma region Start RX receiving
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_START) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = true;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_START) return OK.\n");
			}
			
			err = ::LP_SetFrameCnt(0);  // continious transmit
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Set VSG continious transmit LP_SetFrameCnt(0) failed.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Set VSG continious transmit LP_SetFrameCnt(0) return OK.\n");
			}
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Set VSG send out one packet LP_SetFrameCnt(1) failed.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Set VSG send out one packet LP_SetFrameCnt(1) return OK.\n");
			}

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
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_STOP) return OK.\n");
			}
		}
#pragma endregion			

#pragma region START SLEEP CURRENT test
			//John: 8688 don't support SLEEP mode ( at least 8686 dont support, maybe supported by 8688?
			//anyway, just return an error code for now
		 else if(current_test_mode == E_CURR_TEST_SLEEP)			
		 {
			 err = -1;
			 LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] SLEEP MODE not implemented for this chip.\n");
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
		ReturnErrorMessage(l_CurrentTestReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
	}

	
	return err;
}

int InitializePowerModeDutContainers(void)
{
	/*------------------*
	* Input Parameters  *
	*------------------*/
	l_CurrentTestParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	strcpy_s(l_CurrentTestParam.DUT_POWER_MODE, MAX_BUFFER_SIZE, "TX");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestParam.DUT_POWER_MODE))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestParam.DUT_POWER_MODE;
		setting.unit        = "";
		setting.helpText    = "DUT POWER MODE: TX , RX, SLEEP or IDLE";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DUT_POWER_MODE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_CurrentTestParam.DUT_OPERATION, MAX_BUFFER_SIZE, "START");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestParam.DUT_OPERATION))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestParam.DUT_OPERATION;
		setting.unit        = "";
		setting.helpText    = "DUT OPERATION: START or STOP";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DUT_OPERATION", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	strcpy_s(l_CurrentTestParam.BANDWIDTH, MAX_BUFFER_SIZE, "HT20");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestParam.BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestParam.BANDWIDTH;
		setting.unit        = "";
		setting.helpText    = "Channel bandwidth\r\nValid options: HT20 or HT40";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_CurrentTestParam.DATA_RATE, MAX_BUFFER_SIZE, "OFDM-54");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestParam.DATA_RATE))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestParam.DATA_RATE;
		setting.unit        = "";
		setting.helpText    = "Data rate names:\r\nDSSS-1,DSSS-2,CCK-5_5,CCK-11\r\nOFDM-6,OFDM-9,OFDM-12,OFDM-18,OFDM-24,OFDM-36,OFDM-48,OFDM-54\r\nMCS0, MCS15, etc.";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_CurrentTestParam.PREAMBLE, MAX_BUFFER_SIZE, "SHORT");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestParam.PREAMBLE))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestParam.PREAMBLE;
		setting.unit        = "";
		setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=SHORT.";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_CurrentTestParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestParam.PACKET_FORMAT_11N))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestParam.PACKET_FORMAT_11N;
		setting.unit        = "";
		setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	// IQlite merge; Tracy Yu ; 2012-03-31
	strcpy_s(l_CurrentTestParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestParam.GUARD_INTERVAL_11N))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestParam.GUARD_INTERVAL_11N;
		setting.unit        = "";
		setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_CurrentTestParam.FREQ_MHZ = 2412;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_CurrentTestParam.FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_CurrentTestParam.FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "Channel center frequency in MHz";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	

	l_CurrentTestParam.POWER_DBM = 15.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_CurrentTestParam.POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_CurrentTestParam.POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_CurrentTestParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_CurrentTestParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_CurrentTestParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_CurrentTestParam.ATTEN1 = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_CurrentTestParam.ATTEN1))    // Type_Checking
	{
		setting.value       = (void*)&l_CurrentTestParam.ATTEN1;
		setting.unit        = "";
		setting.helpText    = "DUT attenuator path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ATTEN1", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_CurrentTestParam.ATTEN2 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_CurrentTestParam.ATTEN2))    // Type_Checking
	{
		setting.value       = (void*)&l_CurrentTestParam.ATTEN2;
		setting.unit        = "";
		setting.helpText    = "DUT attenuator path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ATTEN2", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_CurrentTestParam.ATTEN3 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_CurrentTestParam.ATTEN3))    // Type_Checking
	{
		setting.value       = (void*)&l_CurrentTestParam.ATTEN3;
		setting.unit        = "";
		setting.helpText    = "DUT attenuator path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ATTEN3", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_CurrentTestParam.ATTEN4 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_CurrentTestParam.ATTEN4))    // Type_Checking
	{
		setting.value       = (void*)&l_CurrentTestParam.ATTEN4;
		setting.unit        = "";
		setting.helpText    = "DUT attenuator path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_CurrentTestParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ATTEN4", setting) );
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

	   

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_CurrentTestReturn.CABLE_LOSS_DB[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_CurrentTestReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_CurrentTestReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_CurrentTestReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_CurrentTestReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_CurrentTestReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_CurrentTestReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_CurrentTestReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return 0;
}

// IQlite merge; Tracy Yu ; 2012-03-31
void CleanupPowerModeDut(void)
{
	l_CurrentTestParamMap.clear();
	l_CurrentTestReturnMap.clear();
}

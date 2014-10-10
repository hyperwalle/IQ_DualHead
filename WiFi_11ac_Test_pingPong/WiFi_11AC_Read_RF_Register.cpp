#include "stdafx.h"
#include "WiFi_11AC_Test.h"
#include "IQmeasure.h"

using namespace std;

// This variable is declared in WiFi_Test_Internal.cpp
extern vDUT_ID      g_WiFi_11ac_Dut;
extern TM_ID        g_WiFi_11ac_Test_ID;
extern bool			g_vDutTxActived;

#pragma region Define Input and Return structures (two containers and two structs)

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACreadRFRegisterParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_11ACreadRFRegisterReturnMap;

struct tagParam
{
	char ADDRESS[MAX_BUFFER_SIZE];       /*!< A command string that store the address that used to read RF Register data.*/
	int  LENGTH;                         /*!< A command string that is used to define the length read.              */
} l_11ACreadRFRegisterParam;

struct tagReturn
{
	char RF_REGISTER[MAX_BUFFER_SIZE];           /*!< A string contains RF Register value.  */
	char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message. */
} l_11ACreadRFRegisterReturn;
#pragma endregion

#ifndef WIN32
int init11ACReadRFRegisterContainers = Initialize11ACReadRFRegisterContainers();
#endif

// These global variables/functions only for WiFi_Write_RF_Register.cpp
int Check11ACReadRFRegisterParameters(char *ADDRESS, int LENGTH, char* errorMsg);

//! WiFi_Read_RF_Register
/*!
* Input Parameters
*
*  - None 
*
* Return Values
*      -# A string contains the generated MAC address
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Failed
*/


WIFI_11AC_TEST_API int WIFI_11AC_Read_RF_Register(void)
{
	int  err = ERR_OK;
	int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_11ACreadRFRegisterReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_11ACreadRFRegisterParamMap);
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
		RespondToQueryReturn(l_11ACreadRFRegisterReturnMap);
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
			// no need for TX_STOP				
		}

		/*----------------------*
		* Get input parameters *
		*----------------------*/
		err = GetInputParameters(l_11ACreadRFRegisterParamMap);
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
		//CheckDutTransmitStatus();  //don't need to check the DUT's transmit status

		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_11ac_Dut);

		vDUT_AddStringParameter (g_WiFi_11ac_Dut,  "ADDRESS",   l_11ACreadRFRegisterParam.ADDRESS);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut,  "LENGTH",    l_11ACreadRFRegisterParam.LENGTH);

#pragma region Prepare input parameters
		err = Check11ACReadRFRegisterParameters( l_11ACreadRFRegisterParam.ADDRESS, l_11ACreadRFRegisterParam.LENGTH, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Prepare input parameters CheckReadRFRegisterParameters() return OK.\n");
		}
#pragma endregion	

		err = vDUT_Run(g_WiFi_11ac_Dut, "READ_RF_REGISTER");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(READ_RF_REGISTER) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(READ_RF_REGISTER) return OK.\n");
		}

		// TODO: Example, get Return parameters here
		err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "RF_REGISTER", l_11ACreadRFRegisterReturn.RF_REGISTER, MAX_BUFFER_SIZE);		
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// This is an optional return parameter, thus always return OK
			sprintf_s(l_11ACreadRFRegisterReturn.RF_REGISTER, MAX_BUFFER_SIZE, "");
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(RF_REGISTER) return error.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(RF_REGISTER) return OK.\n");
		}

		/*-----------------------*
		*  Return Test Results  *
		*-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_11ACreadRFRegisterReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_11ACreadRFRegisterReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_11ACreadRFRegisterReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_11ACreadRFRegisterReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
	}

	return err;
}



int Initialize11ACReadRFRegisterContainers(void)
{
	/*------------------*
	* Input Parameters: *
	*------------------*/
	l_11ACreadRFRegisterParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	sprintf_s(l_11ACreadRFRegisterParam.ADDRESS, MAX_BUFFER_SIZE, "%s", "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACreadRFRegisterParam.ADDRESS))    // Type_Checking
	{
		setting.value = (void*)l_11ACreadRFRegisterParam.ADDRESS;
		setting.unit        = "";
		setting.helpText    = "A command string that is used to Read EEPROM data.";
		l_11ACreadRFRegisterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ADDRESS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACreadRFRegisterParam.LENGTH = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACreadRFRegisterParam.LENGTH))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACreadRFRegisterParam.LENGTH;
		setting.unit        = "";
		setting.helpText    = "Length to read.";
		l_11ACreadRFRegisterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("LENGTH", setting) );
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
	l_11ACreadRFRegisterReturnMap.clear();

	l_11ACreadRFRegisterReturn.RF_REGISTER[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACreadRFRegisterReturn.RF_REGISTER))    // Type_Checking
	{
		setting.value       = (void*)l_11ACreadRFRegisterReturn.RF_REGISTER;
		setting.unit        = "";
		setting.helpText    = "RF register value read from DUT.";
		l_11ACreadRFRegisterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("RF_REGISTER", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACreadRFRegisterReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACreadRFRegisterReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_11ACreadRFRegisterReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_11ACreadRFRegisterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
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
//-------------------------------------------------------------------------------------
int Check11ACReadRFRegisterParameters(char *ADDRESS, int LENGTH, char* errorMsg)
{
	int    err = ERR_OK;
	//int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		if((LENGTH==0)||(LENGTH<0)||(0==strlen(ADDRESS)))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Invalid ADDRESS or LENGTH.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11AC] CheckReadRFRegisterParameters() Confirmed.\n");
	}
	catch(char *msg)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
	}
	catch(...)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11AC] Unknown Error!\n");
	}
	return err;
}
#include "stdafx.h"
#include "WiFi_11ac_MiMo_Test.h"
#include "IQmeasure.h"

using namespace std;

// This variable is declared in WiFi_Test_Internal.cpp
extern vDUT_ID      g_WiFi_Dut;
extern TM_ID        g_WiFi_Test_ID;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_readSocRegisterParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_readSocRegisterReturnMap;

struct tagParam
{
	char ADDRESS[MAX_BUFFER_SIZE];       /*!< A command string that store the address that used to read Soc Register data.*/
	int  LENGTH;                         /*!< A command string that is used to define the length read.              */
} l_readSocRegisterParam;

struct tagReturn
{
	char SOC_REGISTER[MAX_BUFFER_SIZE];          /*!< A string contains Soc Register value.  */
	char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message. */
} l_readSocRegisterReturn;
#pragma endregion

#ifndef WIN32
int initReadSocRegisterMIMOContainers = InitializeReadSocRegisterContainers();
#endif

// These global variables/functions only for WiFi_Write_Soc_Register.cpp
int CheckReadSocRegisterParameters(char *ADDRESS, int LENGTH, char* errorMsg);

//! WiFi_Read_Soc_Register
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

WIFI_11AC_MIMO_TEST_API int WiFi_Read_Soc_Register(void)
{
	int  err = ERR_OK;
	int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_readSocRegisterReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_readSocRegisterParamMap);
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
		RespondToQueryReturn(l_readSocRegisterReturnMap);
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

		/*----------------------*
		* Get input parameters *
		*----------------------*/
		err = GetInputParameters(l_readSocRegisterParamMap);
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

		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

		vDUT_AddStringParameter (g_WiFi_Dut,  "ADDRESS",   l_readSocRegisterParam.ADDRESS);
		vDUT_AddIntegerParameter(g_WiFi_Dut,  "LENGTH",    l_readSocRegisterParam.LENGTH);

#pragma region Prepare input parameters
		err = CheckReadSocRegisterParameters( l_readSocRegisterParam.ADDRESS, l_readSocRegisterParam.LENGTH, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] Prepare input parameters CheckReadSocRegisterParameters() return OK.\n");
		}
#pragma endregion	

		err = vDUT_Run(g_WiFi_Dut, "READ_SOC_REGISTER");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(READ_SOC_REGISTER) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(READ_SOC_REGISTER) return OK.\n");
		}

		// TODO: Example, get Return parameters here
		err = ::vDUT_GetStringReturn(g_WiFi_Dut, "SOC_REGISTER", l_readSocRegisterReturn.SOC_REGISTER, MAX_BUFFER_SIZE);		
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// This is an optional return parameter, thus always return OK
			sprintf_s(l_readSocRegisterReturn.SOC_REGISTER, MAX_BUFFER_SIZE, "");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_GetStringReturn(SOC_REGISTER) return OK.\n");
		}

		/*-----------------------*
		*  Return Test Results  *
		*-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_readSocRegisterReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_readSocRegisterReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_readSocRegisterReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_readSocRegisterReturn.ERROR_MESSAGE, "[WiFi_11ac_MiMo] Unknown Error!\n");
		err = -1;
	}

	return err;
}


int InitializeReadSocRegisterContainers(void)
{
	/*------------------*
	* Input Parameters: *
	*------------------*/
	l_readSocRegisterParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	sprintf_s(l_readSocRegisterParam.ADDRESS, MAX_BUFFER_SIZE, "%s", "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_readSocRegisterParam.ADDRESS))    // Type_Checking
	{
		setting.value = (void*)l_readSocRegisterParam.ADDRESS;
		setting.unit        = "";
		setting.helpText    = "A command string that is used to Read EEPROM data.";
		l_readSocRegisterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ADDRESS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_readSocRegisterParam.LENGTH = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_readSocRegisterParam.LENGTH))    // Type_Checking
	{
		setting.value       = (void*)&l_readSocRegisterParam.LENGTH;
		setting.unit        = "";
		setting.helpText    = "Length to read.";
		l_readSocRegisterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("LENGTH", setting) );
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
	l_readSocRegisterReturnMap.clear();

	l_readSocRegisterReturn.SOC_REGISTER[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_readSocRegisterReturn.SOC_REGISTER))    // Type_Checking
	{
		setting.value       = (void*)l_readSocRegisterReturn.SOC_REGISTER;
		setting.unit        = "";
		setting.helpText    = "Soc register value read from DUT.";
		l_readSocRegisterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SOC_REGISTER", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_readSocRegisterReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_readSocRegisterReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_readSocRegisterReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_readSocRegisterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
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
int CheckReadSocRegisterParameters(char *ADDRESS, int LENGTH, char* errorMsg)
{
	int    err = ERR_OK;
	//int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		if((LENGTH==0)||(LENGTH<0)||(0==strlen(ADDRESS)))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Invalid ADDRESS or LENGTH.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11ac_MiMo] CheckReadSocRegisterParameters() Confirmed.\n");
	}
	catch(char *msg)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
	}
	catch(...)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11ac_MiMo] Unknown Error!\n");
	}
	return err;
}


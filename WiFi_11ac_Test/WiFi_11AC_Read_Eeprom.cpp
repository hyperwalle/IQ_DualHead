#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

// This variable is declared in WiFi_Test_Internal.cpp
extern vDUT_ID      g_WiFi_11ac_Dut;
extern TM_ID        g_WiFi_11ac_Test_ID;

using namespace std;

#pragma region Define Input and Return structures (two containers and two structs)

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACreadEepromParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_11ACreadEepromReturnMap;

struct tagParam
{
	char ADDRESS[MAX_BUFFER_SIZE];       /*!< A command string that store the address that used to read EEPROM data.*/
	int  LENGTH;                         /*!< A command string that is used to define the length read.              */
	char FILE[MAX_BUFFER_SIZE];          /*!< A command string that is used to define the file to read.             */
} l_11ACreadEepromParam;

struct tagReturn
{
    char EEPROM_RETURN[MAX_BUFFER_SIZE];         /*!< A string contains information that return from read address value from EEPROM.  */
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message. */
} l_11ACreadEepromReturn;

#ifndef WIN32
int init11ACReadEepromContainers = Initialize11ACReadEepromContainers();
#endif

#pragma endregion

// These global variables/functions only for WiFi_Read_Eeprom.cpp
int Check11ACReadEepromParameters(char *ADDRESS, int LENGTH, char *FILE, char* errorMsg);

//! WiFi_Read_Eeprom
/*!
* Input Parameters
*
*  - Mandatory 
*      -# A command string that is used to read EEPROM data
*
* Return Values
*      -# A string contains information that return from EEPROM
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 DUT failed to insert.  Please see the returned error message for details
*/


WIFI_11AC_TEST_API int WIFI_11AC_Read_Eeprom(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_11ACreadEepromReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_11ACreadEepromParamMap);
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
		RespondToQueryReturn(l_11ACreadEepromReturnMap);
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

		/*----------------------*
		* Get input parameters *
		*----------------------*/
		err = GetInputParameters(l_11ACreadEepromParamMap);
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

		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_11ac_Dut);

		// READ_EEPROM
		vDUT_AddStringParameter (g_WiFi_11ac_Dut,  "ADDRESS",   l_11ACreadEepromParam.ADDRESS);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut,  "LENGTH",    l_11ACreadEepromParam.LENGTH);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut,  "FILE",      l_11ACreadEepromParam.FILE);	
		
#pragma region Prepare input parameters
		err = Check11ACReadEepromParameters( l_11ACreadEepromParam.ADDRESS, l_11ACreadEepromParam.LENGTH, l_11ACreadEepromParam.FILE, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Prepare input parameters CheckReadEepromParameters() return OK.\n");
		}
#pragma endregion	

		err = vDUT_Run(g_WiFi_11ac_Dut, "READ_EEPROM");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(READ_EEPROM) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(READ_EEPROM) return OK.\n");
		}

		err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "EEPROM_RETURN", l_11ACreadEepromReturn.EEPROM_RETURN, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] No Return, Unknown EEPROM read to address result.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_GetStringReturn(EEPROM_RETURN) return OK.\n");
		}

		/*-----------------------*
		*  Return Test Results  *
		*-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_11ACreadEepromReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_11ACreadEepromReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_11ACreadEepromReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_11ACreadEepromReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
		err = -1;
    }

	return err;
}

int Initialize11ACReadEepromContainers(void)
{
	/*------------------*
	* Input Parameters: *
	*------------------*/
	l_11ACreadEepromParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	sprintf_s(l_11ACreadEepromParam.ADDRESS, MAX_BUFFER_SIZE, "%s", "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACreadEepromParam.ADDRESS))    // Type_Checking
	{
		setting.value = (void*)l_11ACreadEepromParam.ADDRESS;
		setting.unit        = "";
		setting.helpText    = "A command string that is used to Read EEPROM data.";
		l_11ACreadEepromParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ADDRESS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACreadEepromParam.LENGTH = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACreadEepromParam.LENGTH))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACreadEepromParam.LENGTH;
		setting.unit        = "";
		setting.helpText    = "Length to read from EEPROM or Max length to read.";
		l_11ACreadEepromParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("LENGTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	sprintf_s(l_11ACreadEepromParam.FILE, MAX_BUFFER_SIZE, "%s", "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACreadEepromParam.FILE))    // Type_Checking
	{
		setting.value = (void*)l_11ACreadEepromParam.FILE;
		setting.unit        = "";
		setting.helpText    = "A command string that is used to read EEPROM data.";
		l_11ACreadEepromParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FILE", setting) );
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
	l_11ACreadEepromReturnMap.clear();

	// TODO: Example, add Return parameters here
	l_11ACreadEepromReturn.EEPROM_RETURN[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACreadEepromReturn.EEPROM_RETURN))    // Type_Checking
	{
		setting.value       = (void*)l_11ACreadEepromReturn.EEPROM_RETURN;
		setting.unit        = "";
		setting.helpText    = "A string contains information that return from EEPROM.";
		l_11ACreadEepromReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EEPROM_RETURN", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACreadEepromReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACreadEepromReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_11ACreadEepromReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_11ACreadEepromReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
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
int Check11ACReadEepromParameters(char *ADDRESS, int LENGTH, char *FILE, char* errorMsg)
{
	int    err = ERR_OK;
	//int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		if((0==strlen(FILE))&&((LENGTH==0)||(LENGTH<0)||(0==strlen(ADDRESS))))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Invalid FILE, ADDRESS or LENGTH.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		if(((0!=strlen(ADDRESS))&&(LENGTH>0))&&(0!=strlen(FILE)))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Can't read file and read bytes in one item.\n");
			throw logMessage;
		}

		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11AC] CheckReadEepromParameters() Confirmed.\n");
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


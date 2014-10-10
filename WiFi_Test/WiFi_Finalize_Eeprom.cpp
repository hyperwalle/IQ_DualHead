#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;

#pragma region Define Input and Return structures (two containers and two structs)

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_finalizeEepromParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_finalizeEepromReturnMap;

struct tagReturn
{
    char EEPROM_RETURN[MAX_BUFFER_SIZE];         /*!< A string contains information that return from EEPROM.  */
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message. */
} l_finalizeEepromReturn;

void ClearFinalizeEepromReturn(void)
{
	l_finalizeEepromParamMap.clear();
	l_finalizeEepromReturnMap.clear();
}

#pragma endregion


//! WiFi_Finalize_Eeprom
/*!
* Input Parameters
*
*  - Mandatory 
*      -# None
*
* Return Values
*      -# A string contains information that return from EEPROM
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 DUT failed to insert.  Please see the returned error message for details
*/


WIFI_TEST_API int WiFi_Finalize_Eeprom(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_finalizeEepromReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_finalizeEepromParamMap);
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
		RespondToQueryReturn(l_finalizeEepromReturnMap);
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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID or WiFi_Dut not valid. WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		}
		
		TM_ClearReturns(g_WiFi_Test_ID);

        // Error return of this function is irrelevant
        CheckDutTransmitStatus();

		err = vDUT_Run(g_WiFi_Dut, "FINALIZE_EEPROM");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(FINALIZE_EEPROM) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(FINALIZE_EEPROM) return OK.\n");
		}

		err = ::vDUT_GetStringReturn(g_WiFi_Dut, "EEPROM_RETURN", l_finalizeEepromReturn.EEPROM_RETURN, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] No Return, Unknown EEPROM return result.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(EEPROM_RETURN) return OK.\n");
		}

		/*-----------------------*
		*  Return Test Results  *
		*-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_finalizeEepromReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_finalizeEepromReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_finalizeEepromReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_finalizeEepromReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;
    }

	return err;
}

int InitializeFinalizeEepromContainers(void)
{
	/*------------------*
	* Input Parameters: *
	*------------------*/
	l_finalizeEepromParamMap.clear();

    WIFI_SETTING_STRUCT setting;
    setting.unit = "";
    setting.helpText = "";


	/*----------------*
	* Return Values: *
	* ERROR_MESSAGE  *
	*----------------*/
	l_finalizeEepromReturnMap.clear();

	// TODO: Example, add Return parameters here
	l_finalizeEepromReturn.EEPROM_RETURN[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_finalizeEepromReturn.EEPROM_RETURN))    // Type_Checking
	{
		setting.value       = (void*)l_finalizeEepromReturn.EEPROM_RETURN;
		setting.unit        = "";
		setting.helpText    = "A string contains information that return from EEPROM.";
		l_finalizeEepromReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EEPROM_RETURN", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_finalizeEepromReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_finalizeEepromReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_finalizeEepromReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_finalizeEepromReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return 0;
}

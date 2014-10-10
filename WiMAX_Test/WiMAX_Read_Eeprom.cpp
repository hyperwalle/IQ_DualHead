#include "stdafx.h"
#include "TestManager.h"
#include "WIMAX_Test.h"
#include "WIMAX_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

// This variable is declared in WIMAX_Test_Internal.cpp
extern vDUT_ID      g_WIMAX_Dut;
extern TM_ID        g_WIMAX_Test_ID;

using namespace std;

#pragma region Define Input and Return structures (two containers and two structs)

// Input Parameter Container
map<string, WIMAX_SETTING_STRUCT> l_readEepromParamMap;

// Return Value Container 
map<string, WIMAX_SETTING_STRUCT> l_readEepromReturnMap;

struct tagParam
{
	char ADDRESS[MAX_BUFFER_SIZE];       /*!< A command string that store the address that used to read EEPROM data.*/
	int  LENGTH;                         /*!< A command string that is used to define the length read.              */
	char FILE[MAX_BUFFER_SIZE];          /*!< A command string that is used to define the file to read.             */
} l_readEepromParam;

struct tagReturn
{
    char EEPROM_RETURN[MAX_BUFFER_SIZE];         /*!< A string contains information that return from read address value from EEPROM.  */
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message. */
} l_readEepromReturn;

void ClearReadEepromReturn(void)
{
	l_readEepromParamMap.clear();
	l_readEepromReturnMap.clear();
}

#pragma endregion

// These global variables/functions only for WIMAX_Read_Eeprom.cpp
int CheckReadEepromParameters(char *ADDRESS, int LENGTH, char *FILE, char* errorMsg);

//! WIMAX_Read_Eeprom
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


WIMAX_TEST_API int WIMAX_Read_Eeprom(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_readEepromReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_readEepromParamMap);
		return err;
	}
	else
	{
		// do nothing
	}

	/*-------------------------*
	* Respond to QUERY_RETURN *
	*-------------------------*/
	err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_RETURN", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryReturn(l_readEepromReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}

	try
	{
		/*-----------------------------------------------------------*
		* Both g_WIMAX_Test_ID and g_WIMAX_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_WIMAX_Test_ID<0 || g_WIMAX_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] WIMAX_Test_ID or WIMAX_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] WIMAX_Test_ID = %d and WIMAX_Dut = %d.\n", g_WIMAX_Test_ID, g_WIMAX_Dut);
		}
		
		TM_ClearReturns(g_WIMAX_Test_ID);

		/*----------------------*
		* Get input parameters *
		*----------------------*/
		err = GetInputParameters(l_readEepromParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Get input parameters return OK.\n");
		}

        // Error return of this function is irrelevant
        CheckDutTransmitStatus();

		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WIMAX_Dut);

		// READ_EEPROM
		vDUT_AddStringParameter (g_WIMAX_Dut,  "ADDRESS",   l_readEepromParam.ADDRESS);
		vDUT_AddIntegerParameter(g_WIMAX_Dut,  "LENGTH",    l_readEepromParam.LENGTH);
		vDUT_AddStringParameter (g_WIMAX_Dut,  "FILE",      l_readEepromParam.FILE);	
		
#pragma region Prepare input parameters
		err = CheckReadEepromParameters( l_readEepromParam.ADDRESS, l_readEepromParam.LENGTH, l_readEepromParam.FILE, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Prepare input parameters CheckReadEepromParameters() return OK.\n");
		}
#pragma endregion	

		err = vDUT_Run(g_WIMAX_Dut, "READ_EEPROM");		
		if ( ERR_OK!=err )
		{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(READ_EEPROM) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(READ_EEPROM) return OK.\n");
		}

		err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "EEPROM_RETURN", l_readEepromReturn.EEPROM_RETURN, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] No Return, Unknown EEPROM read to address result.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_GetStringReturn(EEPROM_RETURN) return OK.\n");
		}

		/*----------------------*
		 *  Function Completed  *
		 *----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_readEepromReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_readEepromReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_readEepromReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
		err = -1;
    }

	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	ReturnTestResults(l_readEepromReturnMap);



	return err;
}

void InitializeReadEepromContainers(void)
{
	/*------------------*
	* Input Parameters: *
	*------------------*/
	l_readEepromParamMap.clear();

	WIMAX_SETTING_STRUCT setting;

	sprintf_s(l_readEepromParam.ADDRESS, MAX_BUFFER_SIZE, "%s", "");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_readEepromParam.ADDRESS))    // Type_Checking
	{
		setting.value = (void*)l_readEepromParam.ADDRESS;
		setting.unit        = "";
		setting.helpText    = "A command string that is used to Read EEPROM data.";
		l_readEepromParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ADDRESS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_readEepromParam.LENGTH = 0;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_readEepromParam.LENGTH))    // Type_Checking
	{
		setting.value       = (void*)&l_readEepromParam.LENGTH;
		setting.unit        = "";
		setting.helpText    = "Length to read from EEPROM or Max length to read.";
		l_readEepromParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("LENGTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	sprintf_s(l_readEepromParam.FILE, MAX_BUFFER_SIZE, "%s", "");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_readEepromParam.FILE))    // Type_Checking
	{
		setting.value = (void*)l_readEepromParam.FILE;
		setting.unit        = "";
		setting.helpText    = "A command string that is used to read EEPROM data.";
		l_readEepromParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("FILE", setting) );
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
	l_readEepromReturnMap.clear();

	// TODO: Example, add Return parameters here
	l_readEepromReturn.EEPROM_RETURN[0] = '\0';
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_readEepromReturn.EEPROM_RETURN))    // Type_Checking
	{
		setting.value       = (void*)l_readEepromReturn.EEPROM_RETURN;
		setting.unit        = "";
		setting.helpText    = "A string contains information that return from EEPROM.";
		l_readEepromReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("EEPROM_RETURN", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_readEepromReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_readEepromReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_readEepromReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_readEepromReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return;
}

//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
//-------------------------------------------------------------------------------------
int CheckReadEepromParameters(char *ADDRESS, int LENGTH, char *FILE, char* errorMsg)
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		if((0==strlen(FILE))&&((LENGTH==0)||(LENGTH<0)||(0==strlen(ADDRESS))))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Invalid FILE, ADDRESS or LENGTH.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		if(((0!=strlen(ADDRESS))&&(LENGTH>0))&&(0!=strlen(FILE)))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Can't read file and read bytes in one item.\n");
			throw logMessage;
		}

		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] CheckReadEepromParameters() Confirmed.\n");
	}
	catch(char *msg)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
	}
	catch(...)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] Unknown Error!\n");
	}
	return err;
}


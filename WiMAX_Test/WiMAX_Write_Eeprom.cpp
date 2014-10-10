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
map<string, WIMAX_SETTING_STRUCT> l_writeEepromParamMap;

// Return Value Container 
map<string, WIMAX_SETTING_STRUCT> l_writeEepromReturnMap;

struct tagParam
{
	char ADDRESS[MAX_BUFFER_SIZE];       /*!< A command string that is used to store the address of the EEPROM written. */
	char VALUE[MAX_BUFFER_SIZE];         /*!< A command string that is used to store the value to write.                */
	int  LENGTH;                         /*!< A command string that is used to define the length to write or max length.*/
	char FILE[MAX_BUFFER_SIZE];          /*!< A command string that is used to file to write to EEPROM.                 */
} l_writeEepromParam;

struct tagReturn
{
    char EEPROM_RETURN[MAX_BUFFER_SIZE];         /*!< A string contains information that return from EEPROM.        */
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message.                                   */
} l_writeEepromReturn;

void ClearWriteEepromReturn(void)
{
	l_writeEepromParamMap.clear();
	l_writeEepromReturnMap.clear();
}

#pragma endregion

// These global variables/functions only for WIMAX_Write_Eeprom.cpp
int CheckWriteEepromParameters(char *ADDRESS, char *VALUE, int LENGTH, char *FILE, char* errorMsg);

//! WIMAX_Write_Eeprom
/*!
* Input Parameters
*
*  - Mandatory 
*      -# A command string that is used to Write EEPROM data
*
* Return Values
*      -# A string contains information that return from EEPROM
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 DUT failed to insert.  Please see the returned error message for details
*/
WIMAX_TEST_API int WIMAX_Write_Eeprom(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};


	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_writeEepromReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WIMAX_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_writeEepromParamMap);
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
		RespondToQueryReturn(l_writeEepromReturnMap);
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
		err = GetInputParameters(l_writeEepromParamMap);
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

		// WRITE_COMMAND
		vDUT_AddStringParameter (g_WIMAX_Dut,  "ADDRESS",   l_writeEepromParam.ADDRESS);
		vDUT_AddStringParameter (g_WIMAX_Dut,  "VALUE",     l_writeEepromParam.VALUE);
		vDUT_AddIntegerParameter(g_WIMAX_Dut,  "LENGTH",    l_writeEepromParam.LENGTH);
		vDUT_AddStringParameter (g_WIMAX_Dut,  "FILE",      l_writeEepromParam.FILE);	

#pragma region Prepare input parameters
		err = CheckWriteEepromParameters( l_writeEepromParam.ADDRESS, l_writeEepromParam.VALUE, l_writeEepromParam.LENGTH, l_writeEepromParam.FILE, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] Prepare input parameters CheckWriteEepromParameters() return OK.\n");
		}
#pragma endregion

		err = vDUT_Run(g_WIMAX_Dut, "WRITE_EEPROM");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] vDUT_Run(WRITE_EEPROM) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WIMAX] vDUT_Run(WRITE_EEPROM) return OK.\n");
		}

		// TODO: Example, get Return parameters here
		err = ::vDUT_GetStringReturn(g_WIMAX_Dut, "EEPROM_RETURN", l_writeEepromReturn.EEPROM_RETURN, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] No Return, Unknown EEPROM write to address result.\n");
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
			sprintf_s(l_writeEepromReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_writeEepromReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_writeEepromReturn.ERROR_MESSAGE, "[WIMAX] Unknown Error!\n");
		err = -1;
    }

	/*-----------------------*
	 *  Return Test Results  *
	 *-----------------------*/
	ReturnTestResults(l_writeEepromReturnMap);


	return err;
}

void InitializeWriteEepromContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_writeEepromParamMap.clear();

	WIMAX_SETTING_STRUCT setting;

	sprintf_s(l_writeEepromParam.ADDRESS, MAX_BUFFER_SIZE, "%s", "");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_writeEepromParam.ADDRESS))    // Type_Checking
	{
		setting.value = (void*)l_writeEepromParam.ADDRESS;
		setting.unit        = "";
		setting.helpText    = "A command string that is used to Write EEPROM data.";
		l_writeEepromParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("ADDRESS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	sprintf_s(l_writeEepromParam.VALUE, MAX_BUFFER_SIZE, "%s", "");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_writeEepromParam.VALUE))    // Type_Checking
	{
		setting.value = (void*)l_writeEepromParam.VALUE;
		setting.unit        = "";
		setting.helpText    = "A command string that is used to Write EEPROM data.";
		l_writeEepromParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("VALUE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_writeEepromParam.LENGTH = 0;
	setting.type = WIMAX_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_writeEepromParam.LENGTH))    // Type_Checking
	{
		setting.value       = (void*)&l_writeEepromParam.LENGTH;
		setting.unit        = "";
		setting.helpText    = "Length to write in to EEPROM!";
		l_writeEepromParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("LENGTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	sprintf_s(l_writeEepromParam.FILE, MAX_BUFFER_SIZE, "%s", "");
	setting.type = WIMAX_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_writeEepromParam.FILE))    // Type_Checking
	{
		setting.value = (void*)l_writeEepromParam.FILE;
		setting.unit        = "";
		setting.helpText    = "A command string that is used to write EEPROM data.";
		l_writeEepromParamMap.insert( pair<string,WIMAX_SETTING_STRUCT>("FILE", setting) );
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
	l_writeEepromReturnMap.clear();

    // TODO: Example, add Return parameters here
    l_writeEepromReturn.EEPROM_RETURN[0] = '\0';
    setting.type = WIMAX_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_writeEepromReturn.EEPROM_RETURN))    // Type_Checking
    {
        setting.value       = (void*)l_writeEepromReturn.EEPROM_RETURN;
        setting.unit        = "";
        setting.helpText    = "A string contains information that return from EEPROM.";
        l_writeEepromReturnMap.insert( pair<string,WIMAX_SETTING_STRUCT>("EEPROM_RETURN", setting) );
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
// 
//-------------------------------------------------------------------------------------
int CheckWriteEepromParameters(char *ADDRESS, char *VALUE, int LENGTH, char *FILE, char* errorMsg)
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		if((0==strlen(FILE))&&((LENGTH==0)||(LENGTH<0)||(0==strlen(ADDRESS))||(0==strlen(VALUE))))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Invalid FILE, ADDRESS, LENGTH or VALUE.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		if(((0!=strlen(ADDRESS))&&(0!=strlen(VALUE))&&(LENGTH>0))&&(0!=strlen(FILE)))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] Can't write file and write bytes in one item.\n");
			throw logMessage;
		}

		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] CheckWriteEepromParameters() Confirmed.\n");
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

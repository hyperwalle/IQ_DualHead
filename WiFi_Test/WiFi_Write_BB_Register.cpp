#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;


#pragma region Define Input and Return structures (two containers and two structs)

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_writeBBRegisterParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_writeBBRegisterReturnMap;

struct tagParam
{
	char ADDRESS[MAX_BUFFER_SIZE];       /*!< A command string that is used to store the address of the EEPROM written. */
	char VALUE[MAX_BUFFER_SIZE];         /*!< A command string that is used to store the value to write.                */
} l_writeBBRegisterParam;

struct tagReturn
{
	char WRITE_REGISTER[MAX_BUFFER_SIZE];        /*!< A string contains information that write to register.         */
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message.                                   */
} l_writeBBRegisterReturn;
#pragma endregion

void ClearWriteBBRegisterReturn(void)
{
	l_writeBBRegisterParamMap.clear();
	l_writeBBRegisterReturnMap.clear();
}

#ifndef WIN32
int initWriteBBRegisterContainers = InitializeWriteBBRegisterContainers();
#endif

// These global variables/functions only for WiFi_Write_BB_Register.cpp
int CheckWriteBBRegisterParameters(char *ADDRESS, char *VALUE, char* errorMsg);

//! WiFi_Write_BB_Register
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
WIFI_TEST_API int WiFi_Write_BB_Register(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};


    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_writeBBRegisterReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_writeBBRegisterParamMap);
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
        RespondToQueryReturn(l_writeBBRegisterReturnMap);
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

		// -cfy@sunnyvale, 2012/3/13-
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
			// no need for TX_STOP				
		}
		/* <><~~ */

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_writeBBRegisterParamMap);
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
        //CheckDutTransmitStatus();  //don't need to check the DUT's transmit status

		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

		// WRITE_COMMAND
		vDUT_AddStringParameter (g_WiFi_Dut,  "ADDRESS",   l_writeBBRegisterParam.ADDRESS);
		vDUT_AddStringParameter (g_WiFi_Dut,  "VALUE",     l_writeBBRegisterParam.VALUE);

#pragma region Prepare input parameters
		err = CheckWriteBBRegisterParameters( l_writeBBRegisterParam.ADDRESS, l_writeBBRegisterParam.VALUE, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckWriteBBRegisterParameters() return OK.\n");
		}
#pragma endregion

		err = vDUT_Run(g_WiFi_Dut, "WRITE_BB_REGISTER");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(WRITE_BB_REGISTER) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			// TODO: Example, get Return parameters here
			err = ::vDUT_GetStringReturn(g_WiFi_Dut, "WRITE_REGISTER", l_writeBBRegisterReturn.WRITE_REGISTER, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] No Return, Unknown EEPROM write to address result.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(WRITE_REGISTER) return OK.\n");
			}

			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(WRITE_BB_REGISTER) return OK.\n");
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_writeBBRegisterReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_writeBBRegisterReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_writeBBRegisterReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_writeBBRegisterReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
    }

    return err;
}

int InitializeWriteBBRegisterContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_writeBBRegisterParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    sprintf_s(l_writeBBRegisterParam.ADDRESS, MAX_BUFFER_SIZE, "%s", "");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_writeBBRegisterParam.ADDRESS))    // Type_Checking
    {
        setting.value = (void*)l_writeBBRegisterParam.ADDRESS;
        setting.unit        = "";
        setting.helpText    = "The address that is used to write BB Register.";
        l_writeBBRegisterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ADDRESS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
	sprintf_s(l_writeBBRegisterParam.VALUE, MAX_BUFFER_SIZE, "%s", "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_writeBBRegisterParam.VALUE))    // Type_Checking
	{
		setting.value = (void*)l_writeBBRegisterParam.VALUE;
		setting.unit        = "";
		setting.helpText    = "The value that is used to write BB Register.";
		l_writeBBRegisterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE", setting) );
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
    l_writeBBRegisterReturnMap.clear();

    // TODO: Example, add Return parameters here
	l_writeBBRegisterReturn.WRITE_REGISTER[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_writeBBRegisterReturn.WRITE_REGISTER))    // Type_Checking
	{
		setting.value       = (void*)l_writeBBRegisterReturn.WRITE_REGISTER;
		setting.unit        = "";
		setting.helpText    = "The value had been written to BB Register.";
		l_writeBBRegisterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("WRITE_REGISTER", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_writeBBRegisterReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_writeBBRegisterReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_writeBBRegisterReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_writeBBRegisterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
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
// 
//-------------------------------------------------------------------------------------
int CheckWriteBBRegisterParameters(char *ADDRESS, char *VALUE, char* errorMsg)
{
	int    err = ERR_OK;
	//int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		if((0==strlen(ADDRESS))||(0==strlen(VALUE)))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Invalid ADDRESS or LENGTH.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] CheckWriteBBRegisterParameters() Confirmed.\n");
	}
	catch(char *msg)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
	}
	catch(...)
	{
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] Unknown Error!\n");
	}

	return err;
}
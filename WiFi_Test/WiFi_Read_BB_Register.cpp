#include "stdafx.h"
#include "WiFi_Test.h"
#include "IQmeasure.h"

using namespace std;


#pragma region Define Input and Return structures (two containers and two structs)

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_readBBRegisterParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_readBBRegisterReturnMap;

struct tagParam
{
	char ADDRESS[MAX_BUFFER_SIZE];       /*!< A command string that store the address that used to read BB Register data.*/
	int  LENGTH;                         /*!< A command string that is used to define the length read.                   */
} l_readBBRegisterParam;

struct tagReturn
{
    char BB_REGISTER[MAX_BUFFER_SIZE];           /*!< A string contains BB Register value.  */
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message. */
} l_readBBRegisterReturn;
#pragma endregion

void ClearReadBBRegisterReturn(void)
{
	l_readBBRegisterParamMap.clear();
	l_readBBRegisterReturnMap.clear();
}

#ifndef WIN32
int initReadBBRegisterContainers = InitializeReadBBRegisterContainers();
#endif

// These global variables/functions only for WiFi_Write_BB_Register.cpp
int CheckReadBBRegisterParameters(char *ADDRESS, int LENGTH, char* errorMsg);

//! WiFi_Read_BB_Register
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

WIFI_TEST_API int WiFi_Read_BB_Register(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_readBBRegisterReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_readBBRegisterParamMap);
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
        RespondToQueryReturn(l_readBBRegisterReturnMap);
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
		err = GetInputParameters(l_readBBRegisterParamMap);
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
	    
		vDUT_AddStringParameter (g_WiFi_Dut,  "ADDRESS",   l_readBBRegisterParam.ADDRESS);
		vDUT_AddIntegerParameter(g_WiFi_Dut,  "LENGTH",    l_readBBRegisterParam.LENGTH);

#pragma region Prepare input parameters
		err = CheckReadBBRegisterParameters( l_readBBRegisterParam.ADDRESS, l_readBBRegisterParam.LENGTH, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckReadBBRegisterParameters() return OK.\n");
		}
#pragma endregion	

		err = vDUT_Run(g_WiFi_Dut, "READ_BB_REGISTER");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(READ_BB_REGISTER) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(READ_BB_REGISTER) return OK.\n");
		}

		// TODO: Example, get Return parameters here
		err = ::vDUT_GetStringReturn(g_WiFi_Dut, "BB_REGISTER", l_readBBRegisterReturn.BB_REGISTER, MAX_BUFFER_SIZE);		
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// This is an optional return parameter, thus always return OK
			sprintf_s(l_readBBRegisterReturn.BB_REGISTER, MAX_BUFFER_SIZE, "");
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(BB_REGISTER) return error.\n");
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_GetStringReturn(BB_REGISTER) return OK.\n");
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_readBBRegisterReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_readBBRegisterReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_readBBRegisterReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_readBBRegisterReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
    }

    return err;
}

int InitializeReadBBRegisterContainers(void)
{
	/*------------------*
	* Input Parameters: *
	*------------------*/
	l_readBBRegisterParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	sprintf_s(l_readBBRegisterParam.ADDRESS, MAX_BUFFER_SIZE, "%s", "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_readBBRegisterParam.ADDRESS))    // Type_Checking
	{
		setting.value = (void*)l_readBBRegisterParam.ADDRESS;
		setting.unit        = "";
		setting.helpText    = "A command string that is used to Read EEPROM data.";
		l_readBBRegisterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ADDRESS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_readBBRegisterParam.LENGTH = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_readBBRegisterParam.LENGTH))    // Type_Checking
	{
		setting.value       = (void*)&l_readBBRegisterParam.LENGTH;
		setting.unit        = "";
		setting.helpText    = "Length to read.";
		l_readBBRegisterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("LENGTH", setting) );
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
	l_readBBRegisterReturnMap.clear();

	l_readBBRegisterReturn.BB_REGISTER[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_readBBRegisterReturn.BB_REGISTER))    // Type_Checking
	{
		setting.value       = (void*)l_readBBRegisterReturn.BB_REGISTER;
		setting.unit        = "";
		setting.helpText    = "BB register value read from DUT.";
		l_readBBRegisterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("BB_REGISTER", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_readBBRegisterReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_readBBRegisterReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_readBBRegisterReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_readBBRegisterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
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
int CheckReadBBRegisterParameters(char *ADDRESS, int LENGTH, char* errorMsg)
{
	int    err = ERR_OK;
	//int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		if((LENGTH==0)||(LENGTH<0)||(0==strlen(ADDRESS)))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Invalid ADDRESS or LENGTH.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] CheckReadBBRegisterParameters() Confirmed.\n");
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
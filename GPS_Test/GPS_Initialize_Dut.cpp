#include "stdafx.h"
#include "TestManager.h"
#include "GPS_Test.h"
#include "GPS_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

// This variable is declared in GPS_Test_Internal.cpp
extern vDUT_ID      g_GPS_Dut;
extern TM_ID        g_GPS_Test_ID;

using namespace std;

// Input Parameter Container
map<string, GPS_SETTING_STRUCT> l_initializeDutParamMap;

// Return Value Container 
map<string, GPS_SETTING_STRUCT> l_initializeDutReturnMap;


struct tagReturn
{
	char DUT_NAME[MAX_BUFFER_SIZE];				/*!< DUT name. */
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_initializeDutReturn;

void ClearInitializeDutReturn(void)
{
	l_initializeDutParamMap.clear();
	l_initializeDutReturnMap.clear();
}

//! Initialize GPS DUT
/*!
 * Input Parameters
 *
 *  - Mandatory 
 *      -# A string that is used to determine which DUT register function will be used
 *
 * Return Values
 *      -# A string that contains all DUT information, such as FW version, driver version, chip revision, etc.
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 DUT failed to insert.  Please see the returned error message for details
 */
GPS_TEST_API int GPS_Initialize_Dut(void)
{
    int		err = ERR_OK;	
    int		dummyValue = 0;
	char	logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char	vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};

#pragma region Step 1
    /*
    1. Clear Return Parameters and Container
    2. Respond to QUERY_INPUT
    3. Respond to QUERY_RETURN
    */

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_initializeDutReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_initializeDutParamMap);
        return err;
    }
	else{}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_initializeDutReturnMap);
        return err;
    }
	else{}
#pragma endregion

	try
	{
#pragma region Step 2
        /*
        1. Both g_GPS_Test_ID and g_GPS_Dut need to be valid
        2. Get input parameters
        */

	   /*-----------------------------------------------------------*
		* Both g_GPS_Test_ID and g_GPS_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		TM_ClearReturns(g_GPS_Test_ID);
		if( g_GPS_Test_ID<0 || g_GPS_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] GPS_Test_ID or GPS_Dut not valid.\n");
			throw logMessage;
		}
		else{}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_initializeDutParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Input parameters are not complete.\n");
			throw logMessage;
		}
		else{}
#pragma endregion

#pragma region Step 4 : Main Step

		// Initialize DUT
		err = vDUT_Run(g_GPS_Dut, "INITIALIZE_DUT");
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
			err = vDUT_GetStringReturn(g_GPS_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1; indicates there is an "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else
			{	// Return error message			
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] vDUT_Run(INITIALIZE_DUT) return error.\n");
				throw logMessage;
			}
		}
		else{}
	        
		// For retrieval of the DUT information
		err = vDUT_GetStringReturn(g_GPS_Dut, "DUT_NAME", l_initializeDutReturn.DUT_NAME, MAX_BUFFER_SIZE);
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// This is an optional return parameter, thus always return OK
			sprintf_s(l_initializeDutReturn.DUT_NAME, MAX_BUFFER_SIZE, "No Return, Unknown Dut Name.\n");
		}
		else{}

#pragma endregion

#pragma region Step 5
		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_initializeDutReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_initializeDutReturnMap);
		}
		else{}

#pragma endregion
	}
	catch(char *msg)
	{
        ReturnErrorMessage(l_initializeDutReturn.ERROR_MESSAGE, msg);
    }
	catch(...)
	{
		ReturnErrorMessage(l_initializeDutReturn.ERROR_MESSAGE, "[GPS] Unknown Error!\n");
		err = -1;
    }

    return err;
}

void InitializeInitializeDutContainers(void)
{
    
    GPS_SETTING_STRUCT setting;

#pragma region Step 1 : Setup & Init InputParm
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_initializeDutParamMap.clear();

#pragma endregion

#pragma region Step 2 : Setup & Init ReturnValue 
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_initializeDutReturnMap.clear();

    /* return:DUT_NAME  */
    l_initializeDutReturn.DUT_NAME[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_initializeDutReturn.DUT_NAME))    // Type_Checking
    {
        setting.value       = (void*)l_initializeDutReturn.DUT_NAME;
        setting.unit        = "";
        setting.helpText    = "DUT name.";
        l_initializeDutReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("DUT_NAME", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }

    /* return:error_message  */
    l_initializeDutReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_initializeDutReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_initializeDutReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_initializeDutReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }

#pragma endregion
}


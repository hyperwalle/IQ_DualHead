#include "stdafx.h"
#include "BT_Test.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"
#include "TestManager.h"
#include "vDUT.h"

using namespace std;

// This variable is declared in BT_Test_Internal.cpp
extern vDUT_ID      g_BT_Dut;
extern TM_ID        g_BT_Test_ID;


// Input Parameter Container
map<string, BT_SETTING_STRUCT> l_writeBDAddressParamMap;

// Return Value Container 
map<string, BT_SETTING_STRUCT> l_writeBDAddressReturnMap;

struct tagParam
{
} l_writeBDAddressParam;

struct tagReturn
{
    char BD_ADDRESS[MAX_BUFFER_SIZE];            /*!< A string contains BD address processed by DUT control.  */
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message. */
} l_writeBDAddressReturn;

#ifndef WIN32
int initWriteBDAddressContainers = InitializeWriteBDAddressContainers();
#endif

int ClearWriteBDAddressReturn(void)
{
	l_writeBDAddressParamMap.clear();
	l_writeBDAddressReturnMap.clear();
	return 0;
}

//! BT_Write_BD_Address
/*!
 * Input Parameters
 *
 *  - None 
 *
 * Return Values
 *      -# A string contains the generated BD address
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 Failed
 */
BT_TEST_API int BT_Write_BD_Address(void)
{
    int		err = ERR_OK;
    int		dummyValue = 0;    
	char    logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char	vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Containers *
     *---------------------------------------*/
	ClearReturnParameters(l_writeBDAddressReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_writeBDAddressParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_writeBDAddressReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*-----------------------------------------------------------*
		* Both g_BT_Test_ID and g_BT_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_BT_Test_ID<0 || g_BT_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] BT_Test_ID or BT_Dut not valid. BT_Test_ID = %d and BT_Dut = %d.\n", g_BT_Test_ID, g_BT_Dut);
			throw logMessage;
		}
		else
		{
			// do nothing
		}
		TM_ClearReturns(g_BT_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_writeBDAddressParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not complete.\n");
			throw logMessage;
		}		

		// And clear vDUT parameters at the beginning
		vDUT_ClearParameters(g_BT_Dut);
	    
		err = vDUT_Run(g_BT_Dut, "WRITE_BD_ADDRESS");		
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
			err = ::vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// indicates an error has occurred
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Returns error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(WRITE_BD_ADDRESS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			// do nothing
		}

		// TODO: Example, get Return parameters here
		err = ::vDUT_GetStringReturn(g_BT_Dut, "BD_ADDRESS", l_writeBDAddressReturn.BD_ADDRESS, MAX_BUFFER_SIZE);
		
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// Optional return parameter; always returns OK
			sprintf_s(l_writeBDAddressReturn.BD_ADDRESS, MAX_BUFFER_SIZE, "");
		}
		else
		{
			// do nothing
		}

		/*-----------------------*
		 *  Returns Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_writeBDAddressReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_writeBDAddressReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_writeBDAddressReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_writeBDAddressReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int InitializeWriteBDAddressContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_writeBDAddressParamMap.clear();

    BT_SETTING_STRUCT setting;
    
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_writeBDAddressReturnMap.clear();

    l_writeBDAddressReturn.BD_ADDRESS[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_writeBDAddressReturn.BD_ADDRESS))    // Type_Checking
    {
        setting.value       = (void*)l_writeBDAddressReturn.BD_ADDRESS;
        setting.unit        = "";
        setting.helpText    = "BD address generated and programmed by DUT control.";
        l_writeBDAddressReturnMap.insert( pair<string,BT_SETTING_STRUCT>("BD_ADDRESS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_writeBDAddressReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_writeBDAddressReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_writeBDAddressReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_writeBDAddressReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}


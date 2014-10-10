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
map<string, BT_SETTING_STRUCT> l_writeDutInfoParamMap;

// Return Value Container 
map<string, BT_SETTING_STRUCT> l_writeDutInfoReturnMap;

struct tagParam
{
} l_writeDutInfoParam;

struct tagReturn
{
    char DUT_INFO[MAX_BUFFER_SIZE];              /*!< A string contains DUT INFO processed by DUT control.  */
    char ERROR_MESSAGE[MAX_BUFFER_SIZE];         /*!< A string for error message. */
} l_writeDutInfoReturn;

#ifndef WIN32
int initWriteDutInfoContainers = InitializeWriteDutInfoContainers();
#endif

int ClearwriteDutInfoReturn(void)
{
	l_writeDutInfoParamMap.clear();
	l_writeDutInfoReturnMap.clear();
	return 0;
}

//! BT_Write_DUT_INFO
/*!
 * Input Parameters
 *
 *  - None 
 *
 * Return Values
 *      -# A string contains the DUT INFO
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 Failed
 */
BT_TEST_API int BT_Write_DUT_Info(void)
{
    int		err = ERR_OK;
    int		dummyValue = 0;    
	char    logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char	vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Containers *
     *---------------------------------------*/
	ClearReturnParameters(l_writeDutInfoReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_writeDutInfoParamMap);
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
        RespondToQueryReturn(l_writeDutInfoReturnMap);
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
		err = GetInputParameters(l_writeDutInfoParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not complete.\n");
			throw logMessage;
		}		

		// And clear vDUT parameters at the beginning
		vDUT_ClearParameters(g_BT_Dut);
	    
		err = vDUT_Run(g_BT_Dut, "WRITE_DUT_INFO");		
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(WRITE_DUT_INFO) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			// do nothing
		}

		// TODO: Example, get Return parameters here
		err = ::vDUT_GetStringReturn(g_BT_Dut, "DUT_INFO", l_writeDutInfoReturn.DUT_INFO, MAX_BUFFER_SIZE);
		
		if ( ERR_OK!=err )
		{
			err = ERR_OK;	// Optional return parameter; always returns OK
			sprintf_s(l_writeDutInfoReturn.DUT_INFO, MAX_BUFFER_SIZE, "");
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
			sprintf_s(l_writeDutInfoReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_writeDutInfoReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_writeDutInfoReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_writeDutInfoReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int InitializeWriteDutInfoContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_writeDutInfoParamMap.clear();

    BT_SETTING_STRUCT setting;
    
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_writeDutInfoReturnMap.clear();

    l_writeDutInfoReturn.DUT_INFO[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_writeDutInfoReturn.DUT_INFO))    // Type_Checking
    {
        setting.value       = (void*)l_writeDutInfoReturn.DUT_INFO;
        setting.unit        = "";
        setting.helpText    = "BD address generated and programmed by DUT control.";
        l_writeDutInfoReturnMap.insert( pair<string,BT_SETTING_STRUCT>("DUT_INFO", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_writeDutInfoReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_writeDutInfoReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_writeDutInfoReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_writeDutInfoReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}


#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;



#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_TemplateParamMap;
// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_TemplateReturnMap;

struct tagParam
{
    // Mandatory Parameters
    int    INPUT_INTEGER;                               /*! An example of input parameter */
    double INPUT_DOUBLE;								/*! An example of input parameter */ 
	char   INPUT_STRING[MAX_BUFFER_SIZE];				/*! An example of input parameter */

} l_TemplateParam;

struct tagReturn
{   
    // Template Test Result 
    int    RETURN_INTEGER;                              /*! An example of return parameter */
    double RETURN_DOUBLE;								/*! An example of return parameter */ 
    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];				/*! An example of return parameter, a string for error message. */
} l_TemplateReturn;
#pragma endregion

void ClearTemplateReturn(void)
{
	l_TemplateParamMap.clear();
	l_TemplateReturnMap.clear();
}

#ifndef WIN32
int initTemplateContainers = InitializeTemplateContainers();
#endif


//! WiFi Template Function
/*!
* Input Parameters
*
*  - Mandatory 
*      -# INPUT_INTEGER (integer): An example of input parameter
*      -# INPUT_DOUBLE  (double) : An example of input parameter
*      -# INPUT_STRING  (string) : An example of input parameter
*
* Return Values
*      -# RETURN_INTEGER (integer): An example of return parameter
*      -# RETURN_DOUBLE  (double) : An example of return parameter
*      -# ERROR_MESSAGE  (string) : An example of return parameter, a string of error message.
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/

/********************************************************************************************************
 * This function must be declared in "WiFi_Test.h" .
 * And must add two sentences as following,
 * <1> In "TestManager.cpp":
 *     "g_testFunctions[WIFI].insert( functionPair("TEMPLATE_FUNCTION", callBack) );"
 * <2> In "WiFi_Test.cpp":
 *     "TM_InstallCallbackFunction(technologyID, "TEMPLATE_FUNCTION", WiFi_Template_Function);"
 *
 * The return value of this function must follow the general rule.
 *
 * If there are no errors, the function returns a value of 0  (0 indicates that no errors were returned)
 * If errors occur, the function always returns a value that is NOT equal to ZERO
 *******************************************************************************************************/
WIFI_TEST_API int WiFi_Template_Function(void)
{
    /*-----------------------------------------------------------------
     * Step 1:  Initially, set the err = ERR_OK. 
     *----------------------------------------------------------------*/
    int err = ERR_OK;

    /*-----------------------------------------------------------------
     * Step 2:  Declare local parameters for internal used.
     *----------------------------------------------------------------*/
	int		dummyValue = 0;
	char	logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*-----------------------------------------------------------------
     * Step 3: Clear Return Parameters and Container 
     *----------------------------------------------------------------*/
	ClearReturnParameters(l_TemplateReturnMap);

    /*-----------------------------------------------------------------
     * Step 4: Respond to QUERY_INPUT 
     *----------------------------------------------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_TemplateParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-----------------------------------------------------------------
     * Step 5: Respond to QUERY_RETURN 
     *----------------------------------------------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_TemplateReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try 
	{

    /*----------------------------------------------------------------------
     * Step 6: Check both g_WiFi_Test_ID and g_WiFi_Dut must be valid (>=0)
     *---------------------------------------------------------------------*/
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
		
    /*----------------------------------------------------------------------
     * Step 7: Clear Return map by "Technology ID".
     *---------------------------------------------------------------------*/
		TM_ClearReturns(g_WiFi_Test_ID);

    /*----------------------------------------------------------------------
     * Step 8: Get input parameters
     *---------------------------------------------------------------------*/
		err = GetInputParameters(l_TemplateParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get input parameters return OK.\n");
		}

    /*----------------------------------------------------------------------
     * Step 9: Prepare input parameters
	 *		   For example, sometime user need to convert the "DATA_RATE"
	 *		   string to integer, likes datarate index.
     *---------------------------------------------------------------------*/

		// Your code here...


    /*----------------------------------------------------------------------
     * Step 10: Now, user can configure DUT into Tx or Rx mode, if needed.
     *---------------------------------------------------------------------*/

		// Your code here...

		// For Example:
		// Set DUT RF frquency, antenna, data rate, Tx start etc.

	//-----------------------------------------------
	// (1) Clear containers
	//-----------------------------------------------
		//vDUT_ClearParameters(g_WiFi_Dut);
	//-----------------------------------------------
	// (2) Add input parameters into containers.
	//-----------------------------------------------
		//if( wifiMode==WIFI_11N_HT40 )
		//{
		//	HT40ModeOn = 1;   // 1: HT40 mode;
		//	vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_TemplateParam.FREQ_MHZ);
		//	vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_TemplateParam.FREQ_MHZ-10);
		//	vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_TemplateParam.FREQ_MHZ+10);
		//}
		//else
		//{
		//	HT40ModeOn = 0;   // 0: Normal 20MHz mode 
		//	vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",		l_TemplateParam.FREQ_MHZ);
		//}		  
		//vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",			l_TemplateParam.PREAMBLE);
		//vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_TemplateParam.PACKET_FORMAT_11N);
		//vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_Template_DBM",	l_TemplateParam.TX_Template_DBM);
		//vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",			l_TemplateParam.DATA_RATE);
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		HT40ModeOn);
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				l_TemplateParam.TX1);
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				l_TemplateParam.TX2);
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				l_TemplateParam.TX3);
		//vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				l_TemplateParam.TX4);
	//-----------------------------------------------
	// (3) User can run the specific "Keyword" Dut
	//     control function.
	//-----------------------------------------------
		//err = vDUT_Run(g_WiFi_Dut, "TX_START");
		//if ( ERR_OK!=err )
		//{	
		//   g_vDutTxActived = false;
		//   // Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
		//   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
		//   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
		//   {
		//	   err = -1;	// set err to -1, means "Error".
		//	   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
		//	   throw logMessage;
		//   }
		//   else	// Just return normal error message in this case
		//   {
		//	   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_START) return error.\n");
		//	   throw logMessage;
		//   }
		//}
		//else
		//{
		//   g_vDutTxActived = true;
		//   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_START) return OK.\n");
		//}

		// Delay for DUT settle
		if (0!=g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
		{
			Sleep(g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
		}
		else
		{
			// do nothing
		}

    /*-----------------------------------------------------------------------
     * Step 11: Now, user can configure IQ tester for measurement, if needed.
     *----------------------------------------------------------------------*/

		// Your code here...

		// For example: 
	//-----------------------------------------------
	// (1) Setup VSA.
	// (2) Perform VSA capture.
	// (3) Perform data Analysis.
	// (4) Retrieve analysis Results. 
	// (5) Averaging and Saving Test Result.
	//-----------------------------------------------


    /*----------------------------------------------------------------------
     * Step 12: Return Test Results into return container
     *---------------------------------------------------------------------*/
		if ( ERR_OK==err )
		{
			sprintf_s(l_TemplateReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_TemplateReturnMap);
		}
		else
		{
			// do nothing
		}

	}

    /*----------------------------------------------------------------------
     * Step 13: Return error message
     *---------------------------------------------------------------------*/
	catch(char *msg)
    {
        ReturnErrorMessage(l_TemplateReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_TemplateReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;
    }

    /*----------------------------------------------------------------------
     * Step 14:  At the end, return the error code to the upper layer
     *---------------------------------------------------------------------*/
    return err;
}


/************************************************************************
 * This void funtion can initialize the input and return parameters.
 *
 * The function must be declared in "WiFi_Test_Internal.h" and been called
 * when load WiFi_Test dll, automatically. 
 ************************************************************************/
int InitializeTemplateContainers(void)
{
    /*------------------*
     * Input Parameters *
     *------------------*/
    /*----------------------------------------------------------------------
     * Step 1: Clear input parameter map
     *---------------------------------------------------------------------*/
    l_TemplateParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    /*----------------------------------------------------------------------
     * Step 2: Initialize the input parameters
     *---------------------------------------------------------------------*/
	l_TemplateParam.INPUT_INTEGER = 99;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_TemplateParam.INPUT_INTEGER))    // Type_Checking
    {
        setting.value       = (void*)&l_TemplateParam.INPUT_INTEGER;
        setting.unit        = "Non";
        setting.helpText    = "An example of input parameter";
        l_TemplateParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("INPUT_INTEGER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_TemplateParam.INPUT_DOUBLE = 99.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_TemplateParam.INPUT_DOUBLE))    // Type_Checking
    {
        setting.value = (void*)&l_TemplateParam.INPUT_DOUBLE;
        setting.unit        = "Non";
        setting.helpText    = "An example of input parameter";
        l_TemplateParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("INPUT_DOUBLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_TemplateParam.INPUT_STRING, MAX_BUFFER_SIZE, "INPUT_STRING");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_TemplateParam.INPUT_STRING))    // Type_Checking
    {
        setting.value       = (void*)l_TemplateParam.INPUT_STRING;
        setting.unit        = "Non";
        setting.helpText    = "An example of input parameter";
        l_TemplateParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("INPUT_STRING", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    /*----------------*
     * Return Values  *
     *----------------*/
    /*----------------------------------------------------------------------
     * Step 3: Clear return parameter map
     *---------------------------------------------------------------------*/
    l_TemplateReturnMap.clear();

    /*----------------------------------------------------------------------
     * Step 4: Initialize the return parameters
     *---------------------------------------------------------------------*/
	l_TemplateReturn.RETURN_INTEGER = (int)NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_TemplateReturn.RETURN_INTEGER))    // Type_Checking
    {
        setting.value = (void*)&l_TemplateReturn.RETURN_INTEGER;
        setting.unit        = "Non";
        setting.helpText    = "An example of return parameter";
        l_TemplateReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("RETURN_INTEGER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }    

	l_TemplateReturn.RETURN_DOUBLE = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_TemplateReturn.RETURN_DOUBLE))    // Type_Checking
    {
        setting.value = (void*)&l_TemplateReturn.RETURN_DOUBLE;
        setting.unit        = "Non";
        setting.helpText    = "An example of return parameter";
        l_TemplateReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("RETURN_DOUBLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }    

    l_TemplateReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_TemplateReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_TemplateReturn.ERROR_MESSAGE;
        setting.unit        = "Non";
        setting.helpText    = "An example of return parameter";
       l_TemplateReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

#include "stdafx.h"
#include "TestManager.h"
#include "GPS_Test.h"
#include "GPS_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"

// This variable is declared in GPS_Test_Internal.cpp
extern vDUT_ID      g_GPS_Dut;
extern bool         g_ReloadDutDll;
extern TM_ID        g_GPS_Test_ID;
extern int          g_GPS_Test_timer;
extern const char  *g_vDUT_Version;
extern GPS_GLOBAL_SETTING g_GPSGlobalSettingParam;


using namespace std;

// Input Parameter Container
map<string, GPS_SETTING_STRUCT> l_insertDutParamMap;

// Return Value Container 
map<string, GPS_SETTING_STRUCT> l_insertDutReturnMap;

struct tagParam
{
    char    DUT_DLL_FILENAME[MAX_BUFFER_SIZE];		/*!< Specify which DUT control DLL is to be used. */
	char    CONNECTION_NAME[MAX_BUFFER_SIZE];	    /*!< Specify which DUT device name is to be used; for example,"com5@115200" or "usb0". */
    int     RELOAD_DUT_DLL;							/*!< Specify whether the DUT control DLL will be loaded into memory every time INSERT_DUT runs. Options are 1(reload), or 0(not reload) */
} l_insertDutParam;

struct tagReturn
{
	char    VDUT_VERSION[MAX_BUFFER_SIZE];			/*!< Information on the vDUT DLL, such as version. */
    char    DUT_VERSION[MAX_BUFFER_SIZE];			/*!< Information on the DUT control DLL, such as version. */
    char    ERROR_MESSAGE[MAX_BUFFER_SIZE];			
} l_insertDutReturn;

void ClearInsertDutReturn(void)
{
	l_insertDutParamMap.clear();
	l_insertDutReturnMap.clear();
}

//! Insert GPS DUT
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
 * \return -1 DUT failed to insert.  Please see the returned error message for details.
 */
GPS_TEST_API int GPS_Insert_Dut(void)
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
	ClearReturnParameters(l_insertDutReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_insertDutParamMap);
        return err;
    }
	else{}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_GPS_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_insertDutReturnMap);
        return err;
    }
	else{}
#pragma endregion

	try
	{
#pragma region Step 2
        /*
        1. g_GPS_Test_ID needs to be valid (>=0)
        2. Get input parameters
        */

	   /*---------------------------------------*
		* g_GPS_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		TM_ClearReturns(g_GPS_Test_ID);
		if( g_GPS_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] GPS_Test_ID not valid.\n");
			throw logMessage;
		}
		else{}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_insertDutParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Input parameters are not complete.\n");
			throw logMessage;
		}
		else{}
#pragma endregion

#pragma region Step 4 : Main Step
		g_ReloadDutDll = l_insertDutParam.RELOAD_DUT_DLL==0 ? false : true;
		if( 0!=strlen(l_insertDutParam.DUT_DLL_FILENAME) ) 
		{
			// If the DUT DLL filename is specified, load the DUT DLL
			// If the DUT DLL must be loaded everytime, or if the DUT DLL has never been loaded
			if( 0!=l_insertDutParam.RELOAD_DUT_DLL || -1==g_GPS_Dut )
			{
				// Get Dut ID 
				err = vDUT_RegisterDutDll( "GPS", l_insertDutParam.DUT_DLL_FILENAME, &g_GPS_Dut );
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Fail to load Dut Dll.\n");
					throw logMessage;
				}
			}
			else{}
		}
		else
		{			
			// No DUT DLL name is specified, return error
			// Log the error message
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] %s is not a valid DUT DLL name.\n", l_insertDutParam.DUT_DLL_FILENAME);	
			throw logMessage;
		}

		if( (0==err)&&(g_GPS_Dut>=0) )  // Status is OK
		{
			err = vDUT_AddStringParameter (g_GPS_Dut, "CONNECTION_NAME", l_insertDutParam.CONNECTION_NAME);
			
			err = ::vDUT_AddStringParameter(g_GPS_Dut, "GPS_HOST_PATH", g_GPSGlobalSettingParam.GPS_HOST_PATH);
			err = ::vDUT_AddStringParameter(g_GPS_Dut, "GPS_HOST_SHELL_SCRIPT", g_GPSGlobalSettingParam.GPS_HOST_SHELL_SCRIPT);
			err = ::vDUT_AddStringParameter(g_GPS_Dut, "CONFIG_TEMPLATE_XML", g_GPSGlobalSettingParam.CONFIG_TEMPLATE_XML);
			
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] vDUT_AddStringParameter(CONNECTION_NAME) return error.\n", l_insertDutParam.DUT_DLL_FILENAME);
				throw logMessage;
			}

			// INSERT_DUT
			err = vDUT_Run(g_GPS_Dut, "INSERT_DUT");	
			if ( ERR_OK!=err )
			{	//  Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer
			
				err = vDUT_GetStringReturn(g_GPS_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
				{
					err = -1;	// set err to -1; indicates "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					throw logMessage;
				}
				else
				{	// Return an error message				
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] %s failed to run INSERT_DUT.\n", l_insertDutParam.DUT_DLL_FILENAME);
					throw logMessage;
				}
			}
			else{}


			// For retrieval of the vDUT version information
			err = vDUT_GetVersion(l_insertDutReturn.VDUT_VERSION, MAX_BUFFER_SIZE); 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] Failed to get vDut version info.\n");
				throw logMessage;
			}

			// For retrieval of the DUT information
			err = vDUT_GetStringReturn(g_GPS_Dut, "DUT_VERSION", l_insertDutReturn.DUT_VERSION, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				err = ERR_OK;	// This is an optional return parameter, thus always return OK
				sprintf_s(l_insertDutReturn.DUT_VERSION, MAX_BUFFER_SIZE, "No Return, Unknown Dut Version.\n");
			}
			else{}
		}
		else
		{
			// Registration failed
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[GPS] %s failed to register GPS Technology.\n", l_insertDutParam.DUT_DLL_FILENAME);
			throw logMessage;
		}
#pragma endregion

#pragma region Step 5
		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_insertDutReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.");
			ReturnTestResults(l_insertDutReturnMap);
		}
		else{}
#pragma endregion

	}
	catch(char *msg)
	{
        ReturnErrorMessage(l_insertDutReturn.ERROR_MESSAGE, msg);
    }
	catch(...)
	{
		ReturnErrorMessage(l_insertDutReturn.ERROR_MESSAGE, "[GPS] Unknown Error!\n");
		err = -1;
    }

    return err;
}

void InitializeInsertDutContainers(void)
{
    GPS_SETTING_STRUCT setting;

#pragma region Step 1 : Setup & Init InputParm
    /*------------------*
     * Input Paramters: *
     *------------------*/
    l_insertDutParamMap.clear();

    /* input:DUT_DLL_FILENAME */
    strcpy_s(l_insertDutParam.DUT_DLL_FILENAME, MAX_BUFFER_SIZE, "TemplateDut.DLL");
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_insertDutParam.DUT_DLL_FILENAME))    // Type_Checking
    {
        setting.value       = (void*)l_insertDutParam.DUT_DLL_FILENAME;
        setting.unit        = "";
        setting.helpText    = "Specify which DUT control DLL to be used\r\nIf no path with the specified DLL filename, the DLL will be loaded from the same folder where vDUT.DLL resides.";
        l_insertDutParamMap.insert( pair<string,GPS_SETTING_STRUCT>("DUT_DLL_FILENAME", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }   

    /* input:CONNECTION_NAME */
    strcpy_s(l_insertDutParam.CONNECTION_NAME, MAX_BUFFER_SIZE, "");
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_insertDutParam.CONNECTION_NAME))    // Type_Checking
    {
        setting.value       = (void*)l_insertDutParam.CONNECTION_NAME;
        setting.unit        = "";
        setting.helpText    = "Specify which DUT device name to be used, such as com1@115200 or usb0.";
        l_insertDutParamMap.insert( pair<string,GPS_SETTING_STRUCT>("CONNECTION_NAME", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }

    /* input:DUT_DLL_FILENAME */
    l_insertDutParam.RELOAD_DUT_DLL = 1;
    setting.type = GPS_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_insertDutParam.RELOAD_DUT_DLL))    // Type_Checking
    {
        setting.value       = (void*)&l_insertDutParam.RELOAD_DUT_DLL;
        setting.unit        = "";
        setting.helpText    = "Specify whether the DUT control DLL will be loaded into memory every time INSERT_DUT runs.\r\nOptions are 1(reload), or 0(not reload)\r\nIf set to 1(reload), REMOVE_DUT will unload the DLL from memory";
        l_insertDutParamMap.insert( pair<string,GPS_SETTING_STRUCT>("RELOAD_DUT_DLL", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }
#pragma endregion

#pragma region Step 2 : Setup & Init ReturnValue    
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_insertDutReturnMap.clear();

    /* return:VDUT_VERSION  */
    l_insertDutReturn.VDUT_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_insertDutReturn.VDUT_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_insertDutReturn.VDUT_VERSION;
        setting.unit        = "";
        setting.helpText    = "Information of the vDUT DLL, such as version.";
        l_insertDutReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("VDUT_VERSION", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }

    /* return:DUT_VERSION  */
    l_insertDutReturn.DUT_VERSION[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_insertDutReturn.DUT_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_insertDutReturn.DUT_VERSION;
        setting.unit        = "";
        setting.helpText    = "Information of the DUT control DLL, such as version.";
        l_insertDutReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("DUT_VERSION", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }

    /* return:error_message  */
    l_insertDutReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = GPS_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_insertDutReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_insertDutReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred.";
        l_insertDutReturnMap.insert( pair<string,GPS_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
	else
	{
        printf("Parameter Type Error!\n");
        exit(1);
    }

#pragma endregion
}

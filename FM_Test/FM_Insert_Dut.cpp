#include "stdafx.h"
#include "TestManager.h"
#include "FM_Test.h"
#include "FM_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"

// This variable is declared in FM_Test_Internal.cpp
extern vDUT_ID      g_FM_Dut;
extern bool         g_ReloadDutDll;
extern TM_ID        g_FM_Test_ID;
extern int          g_FM_Test_timer;
extern const char  *g_vDUT_Version;

using namespace std;

// Input Parameter Container
map<string, FM_SETTING_STRUCT> l_insertDutParamMap;

// Return Value Container 
map<string, FM_SETTING_STRUCT> l_insertDutReturnMap;

struct tagParam
{
    char    DUT_DLL_FILENAME[MAX_BUFFER_SIZE];		/*!< Specify which DUT control DLL is to be used. */
	char    CONNECTION_NAME[MAX_BUFFER_SIZE];	    /*!< Specify which DUT device name is to be used; for example,"com5@115200" or "usb0". */
    int     RELOAD_DUT_DLL;							/*!< Specify whether the DUT control DLL will be loaded into memory every time INSERT_DUT runs. Options are 1(reload), or 0(not reload) */
	int		FM_MODE_ANALOG_DIGITAL;					/*!< Specify whether the DUT control mode is analog or digital. */
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

//! Insert FM DUT
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
FM_TEST_API int FM_Insert_Dut(void)
{
    int		err = ERR_OK;	
    int		dummyValue = 0;
	char	logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char	vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_insertDutReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_FM_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_insertDutParamMap);
        return err;
    }
	else
	{
		// do nothing
        // Nothing to do
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_FM_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_insertDutReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*---------------------------------------*
		* g_FM_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		TM_ClearReturns(g_FM_Test_ID);
		if( g_FM_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] FM_Test_ID not valid.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_insertDutParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Input parameters are not complete.\n");
			throw logMessage;
		}

		g_ReloadDutDll = l_insertDutParam.RELOAD_DUT_DLL==0 ? false : true;
		if( 0!=strlen(l_insertDutParam.DUT_DLL_FILENAME) ) 
		{
			// If the DUT DLL filename is specified, load the DUT DLL
			// If the DUT DLL must be loaded everytime, or if the DUT DLL has never been loaded
			if( 0!=l_insertDutParam.RELOAD_DUT_DLL || -1==g_FM_Dut )
			{
				err = vDUT_RegisterDutDll( "FM", l_insertDutParam.DUT_DLL_FILENAME, &g_FM_Dut );
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Fail to load Dut Dll.\n");
					throw logMessage;
				}
			}
			else
			{
				// do nothing
			}
		}
		else
		{			
			// No DUT DLL name is specified, return error
			// Log the error message
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] %s is not a valid DUT DLL name.\n", l_insertDutParam.DUT_DLL_FILENAME);	
			throw logMessage;
		}

		if( (0==err)&&(g_FM_Dut>=0) )  // Status is OK
		{
			err = vDUT_AddStringParameter (g_FM_Dut, "CONNECTION_NAME", l_insertDutParam.CONNECTION_NAME);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] vDUT_AddStringParameter(CONNECTION_NAME) return error.\n", l_insertDutParam.DUT_DLL_FILENAME);
				throw logMessage;
			}

			vDUT_AddIntegerParameter(g_FM_Dut, "FM_MODE", l_insertDutParam.FM_MODE_ANALOG_DIGITAL);

			// INSERT_DUT
			err = vDUT_Run(g_FM_Dut, "INSERT_DUT");			
			if ( ERR_OK!=err )
			{	//  Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer
			
				err = vDUT_GetStringReturn(g_FM_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
				{
					err = -1;	// set err to -1; indicates "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
					throw logMessage;
				}
				else	// Return an error message
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] %s failed to run INSERT_DUT.\n", l_insertDutParam.DUT_DLL_FILENAME);
					throw logMessage;
				}
			}
			else
			{
				// do nothing
			}


			// For retrieval of the vDUT version information
			err = vDUT_GetVersion(l_insertDutReturn.VDUT_VERSION, MAX_BUFFER_SIZE); 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] Failed to get vDut version info.\n");
				throw logMessage;
			}

			// For retrieval of the DUT information
			err = vDUT_GetStringReturn(g_FM_Dut, "DUT_VERSION", l_insertDutReturn.DUT_VERSION, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				err = ERR_OK;	// This is an optional return parameter, thus always return OK
				sprintf_s(l_insertDutReturn.DUT_VERSION, MAX_BUFFER_SIZE, "No Return, Unknown Dut Version.\n");
			}
			else
			{
				// do nothing
			}
		}
		else
		{
			// Registration failed
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[FM] %s failed to register FM Technology.\n", l_insertDutParam.DUT_DLL_FILENAME);
			throw logMessage;
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_insertDutReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_insertDutReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_insertDutReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_insertDutReturn.ERROR_MESSAGE, "[FM] Unknown Error!\n");
		err = -1;
    }

    return err;
}

void InitializeInsertDutContainers(void)
{
    /*------------------*
     * Input Paramters: *
     *------------------*/
    l_insertDutParamMap.clear();

    FM_SETTING_STRUCT setting;

    setting.unit        = "";
    setting.helpText    = "Specify which DUT control DLL to be used\r\nIf no path with the specified DLL filename, the DLL will be loaded from the same folder where vDUT.DLL resides.";
    setting.type = FM_SETTING_TYPE_STRING;
    strcpy_s(l_insertDutParam.DUT_DLL_FILENAME, MAX_BUFFER_SIZE, "TemplateDut.DLL");
    if (MAX_BUFFER_SIZE==sizeof(l_insertDutParam.DUT_DLL_FILENAME))    // Type_Checking
    {
        setting.value       = (void*)l_insertDutParam.DUT_DLL_FILENAME;
        l_insertDutParamMap.insert( pair<string,FM_SETTING_STRUCT>("DUT_DLL_FILENAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }   

    setting.unit        = "";
    setting.helpText    = "Specify which DUT device name to be used, such as com1@115200 or usb0.";
    setting.type = FM_SETTING_TYPE_STRING;
    strcpy_s(l_insertDutParam.CONNECTION_NAME, MAX_BUFFER_SIZE, "com1@115200");
    if (MAX_BUFFER_SIZE==sizeof(l_insertDutParam.CONNECTION_NAME))    // Type_Checking
    {
        setting.value       = (void*)l_insertDutParam.CONNECTION_NAME;
        l_insertDutParamMap.insert( pair<string,FM_SETTING_STRUCT>("CONNECTION_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.unit        = "";
    setting.helpText    = "Specify whether the DUT control DLL will be loaded into memory every time INSERT_DUT runs.\r\nOptions are 1(reload), or 0(not reload)\r\nIf set to 1(reload), REMOVE_DUT will unload the DLL from memory";
    setting.type = FM_SETTING_TYPE_INTEGER;
    l_insertDutParam.RELOAD_DUT_DLL = 0;
    if (sizeof(int)==sizeof(l_insertDutParam.RELOAD_DUT_DLL))    // Type_Checking
    {
        setting.value       = (void*)&l_insertDutParam.RELOAD_DUT_DLL;
        l_insertDutParamMap.insert( pair<string,FM_SETTING_STRUCT>("RELOAD_DUT_DLL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.unit        = "";
	setting.helpText    = "Specify whether the DUT control mode is analog or digital.\r\nOptions are 0:Analog Mode, or 1:Digital Mode";
	setting.type = FM_SETTING_TYPE_INTEGER;
	l_insertDutParam.FM_MODE_ANALOG_DIGITAL = 0;
	if (sizeof(int)==sizeof(l_insertDutParam.FM_MODE_ANALOG_DIGITAL))    // Type_Checking
	{
		setting.value       = (void*)&l_insertDutParam.FM_MODE_ANALOG_DIGITAL;
		l_insertDutParamMap.insert( pair<string,FM_SETTING_STRUCT>("FM_MODE", setting) );
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
    l_insertDutReturnMap.clear();

    l_insertDutReturn.VDUT_VERSION[0] = '\0';
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_insertDutReturn.VDUT_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_insertDutReturn.VDUT_VERSION;
        setting.unit        = "";
        setting.helpText    = "Information of the vDUT DLL, such as version.";
        l_insertDutReturnMap.insert( pair<string,FM_SETTING_STRUCT>("VDUT_VERSION", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_insertDutReturn.DUT_VERSION[0] = '\0';
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_insertDutReturn.DUT_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_insertDutReturn.DUT_VERSION;
        setting.unit        = "";
        setting.helpText    = "Information of the DUT control DLL, such as version.";
        l_insertDutReturnMap.insert( pair<string,FM_SETTING_STRUCT>("DUT_VERSION", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_insertDutReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = FM_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_insertDutReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_insertDutReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred.";
        l_insertDutReturnMap.insert( pair<string,FM_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return;
}

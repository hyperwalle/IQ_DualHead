#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"

// This variable is declared in WiFi_Test_Internal.cpp
extern vDUT_ID      g_WiFi_11ac_Dut;
extern bool         g_ReloadDutDll;
extern TM_ID        g_WiFi_11ac_Test_ID;
extern int          g_WiFi_11AC_Test_timer;
extern const char  *g_vDUT_Version;
int					g_useLpDut = FALSE;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACinsertDutParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_11ACinsertDutReturnMap;

struct tagParam
{
    char    DUT_DLL_FILENAME[MAX_BUFFER_SIZE];		/*!< Specify which DUT control DLL to be used. */
	char    CONNECTION_STRING[MAX_BUFFER_SIZE];		/*!< Specify the DUT connection string. */
    int     RELOAD_DUT_DLL;							/*!< Specify whether the DUT control DLL will be loaded into memory every time INSERT_DUT runs. Options are 1(reload), or 0(not reload) */
    char    EEPROM_FILENAME[MAX_BUFFER_SIZE];       /*!< Specify the DUT EEPROM file string. */
    char    OPTION_STRING[MAX_BUFFER_SIZE];		    /*!< Specify the DUT option string. */
} l_11ACinsertDutParam;

struct tagReturn
{
	char    VDUT_VERSION[MAX_BUFFER_SIZE];			/*!< Information of the vDUT DLL, such as version. */
    char    DUT_VERSION[MAX_BUFFER_SIZE];			/*!< Information of the DUT control DLL, such as version. */
    char    ERROR_MESSAGE[MAX_BUFFER_SIZE];			
} l_11ACinsertDutReturn;

//#ifndef WIN32 
int init11ACInsertDutContainers = Initialize11ACInsertDutContainers();
//#endif

//! Insert WiFi DUT
/*!
 * Input Parameters
 *
 *  - Mandatory 
 *      -# A string that is used to determine which DUT register function will be used
 *
 * Return Values
 *      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
 *      -# A string for error message
 *
 * \return 0 No error occurred
 * \return -1 DUT failed to insert.  Please see the returned error message for details
 */
WIFI_11AC_TEST_API int WIFI_11AC_Insert_Dut(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char tempBuff[MAX_BUFFER_SIZE] = {'\0'};


    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_11ACinsertDutReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_11ACinsertDutParamMap);
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
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_11ACinsertDutReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*---------------------------------------*
		* g_WiFi_11ac_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		if( g_WiFi_11ac_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] WiFi_Test_ID not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] WiFi_Test_ID = %d.\n", g_WiFi_11ac_Test_ID);
		}

		TM_ClearReturns(g_WiFi_11ac_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_11ACinsertDutParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get input parameters return OK.\n");
		}

        // And clear vDut parameters at beginning.
        vDUT_ClearParameters(g_WiFi_11ac_Dut);

		g_ReloadDutDll = l_11ACinsertDutParam.RELOAD_DUT_DLL==0 ? false : true;
		if( 0!=strlen(l_11ACinsertDutParam.DUT_DLL_FILENAME) ) 
		{
			// check if the LP virtual DUT is used
			for (size_t i=0; i<=strlen(l_11ACinsertDutParam.DUT_DLL_FILENAME) ; i++)
				tempBuff[i] = toupper(l_11ACinsertDutParam.DUT_DLL_FILENAME[i]);
			if (strcmp(tempBuff, "LP_DUT.DLL")==0 || strcmp(tempBuff, "LIBLP_DUT.DYLIB")==0)
				g_useLpDut = TRUE;

			if( 0!=l_11ACinsertDutParam.RELOAD_DUT_DLL || -1==g_WiFi_11ac_Dut )
			{
				err = ::vDUT_RegisterDutDll( "WIFI_11AC", l_11ACinsertDutParam.DUT_DLL_FILENAME, &g_WiFi_11ac_Dut );
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to load Dut Dll.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_RegisterDutDll() load Dut Dll return OK.\n");
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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] %s is not a valid DUT DLL name.\n", l_11ACinsertDutParam.DUT_DLL_FILENAME);	
			throw logMessage;
		}

		if(l_11ACinsertDutParam.CONNECTION_STRING!=NULL)
		{
			vDUT_AddStringParameter (g_WiFi_11ac_Dut, "CONNECTION_STRING",	l_11ACinsertDutParam.CONNECTION_STRING);
		}
		else
		{
			// do nothing
		}		

        if(l_11ACinsertDutParam.OPTION_STRING!=NULL)
        {
            vDUT_AddStringParameter (g_WiFi_11ac_Dut, "OPTION_STRING",	l_11ACinsertDutParam.OPTION_STRING);
        }
        else
        {
            // do nothing
        }		

        if(l_11ACinsertDutParam.EEPROM_FILENAME!=NULL)
        {
            vDUT_AddStringParameter (g_WiFi_11ac_Dut, "EEPROM_FILENAME",	l_11ACinsertDutParam.EEPROM_FILENAME);
        }
        else
        {
            // do nothing
        }		

        if( (0==err)&&(g_WiFi_11ac_Dut>=0) )  // Status is OK
		{
			// INSERT_DUT
			err = vDUT_Run(g_WiFi_11ac_Dut, "INSERT_DUT");			
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] %s fail to run INSERT_DUT.\n", l_11ACinsertDutParam.DUT_DLL_FILENAME);
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(INSERT_DUT) return OK.\n");
			}


			// For retrieval of the vDUT version info
			err = ::vDUT_GetVersion(l_11ACinsertDutReturn.VDUT_VERSION, MAX_BUFFER_SIZE); 
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to get vDut version information.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get vDut version information return OK.\n");
			}

			// For retrieval of the DUT info
			err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "DUT_VERSION", l_11ACinsertDutReturn.DUT_VERSION, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				err = ERR_OK;	// This is an optional return parameter, thus always return OK
				sprintf_s(l_11ACinsertDutReturn.DUT_VERSION, MAX_BUFFER_SIZE, "No Return, Unknown Dut Version.\n");
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
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] %s failed to register WIFI Technology.\n", l_11ACinsertDutParam.DUT_DLL_FILENAME);
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			Initialize11ACInternalParameters();	
			sprintf_s(l_11ACinsertDutReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_11ACinsertDutReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_11ACinsertDutReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_11ACinsertDutReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
		err = -1;
    }

    return err;
}

int Initialize11ACInsertDutContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_11ACinsertDutParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    setting.unit        = "";
    setting.helpText    = "Specify which DUT control DLL to be used\r\nIf no path with the specified DLL filename, the DLL will be loaded from the same folder where vDUT.DLL resides.";
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(l_11ACinsertDutParam.DUT_DLL_FILENAME, MAX_BUFFER_SIZE, "TemplateDut.DLL");
    if (MAX_BUFFER_SIZE==sizeof(l_11ACinsertDutParam.DUT_DLL_FILENAME))    // Type_Checking
    {
        setting.value       = (void*)l_11ACinsertDutParam.DUT_DLL_FILENAME;
        l_11ACinsertDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DUT_DLL_FILENAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.unit        = "";
	setting.helpText    = "Specify the connection string for the DUT";
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(l_11ACinsertDutParam.CONNECTION_STRING, MAX_BUFFER_SIZE, "");
	if (MAX_BUFFER_SIZE==sizeof(l_11ACinsertDutParam.CONNECTION_STRING))    // Type_Checking
	{
		setting.value       = (void*)l_11ACinsertDutParam.CONNECTION_STRING;
		l_11ACinsertDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CONNECTION_STRING", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    setting.unit        = "";
    setting.helpText    = "Specify the EEPROM file name for the DUT; if leave empty, no EEPROM file will be used";
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(l_11ACinsertDutParam.EEPROM_FILENAME, MAX_BUFFER_SIZE, "");
    if (MAX_BUFFER_SIZE==sizeof(l_11ACinsertDutParam.EEPROM_FILENAME))    // Type_Checking
    {
        setting.value       = (void*)l_11ACinsertDutParam.EEPROM_FILENAME;
        l_11ACinsertDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("EEPROM_FILENAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.unit        = "";
    setting.helpText    = "Specify the option string for the DUT";
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(l_11ACinsertDutParam.OPTION_STRING, MAX_BUFFER_SIZE, "");
    if (MAX_BUFFER_SIZE==sizeof(l_11ACinsertDutParam.OPTION_STRING))    // Type_Checking
    {
        setting.value       = (void*)l_11ACinsertDutParam.OPTION_STRING;
        l_11ACinsertDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("OPTION_STRING", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.unit        = "";
    setting.helpText    = "Specify whether the DUT control DLL will be loaded into memory every time INSERT_DUT runs.\r\nOptions are 1(reload), or 0(not reload)\r\nIf set to 1(reload), REMOVE_DUT will unload the DLL from memory";
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    l_11ACinsertDutParam.RELOAD_DUT_DLL = 0;
    if (sizeof(int)==sizeof(l_11ACinsertDutParam.RELOAD_DUT_DLL))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACinsertDutParam.RELOAD_DUT_DLL;
        l_11ACinsertDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RELOAD_DUT_DLL", setting) );
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
    l_11ACinsertDutReturnMap.clear();

    l_11ACinsertDutReturn.VDUT_VERSION[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACinsertDutReturn.VDUT_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_11ACinsertDutReturn.VDUT_VERSION;
        setting.unit        = "";
        setting.helpText    = "Information of the vDUT DLL, such as version.";
        l_11ACinsertDutReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VDUT_VERSION", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACinsertDutReturn.DUT_VERSION[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACinsertDutReturn.DUT_VERSION))    // Type_Checking
    {
        setting.value       = (void*)l_11ACinsertDutReturn.DUT_VERSION;
        setting.unit        = "";
        setting.helpText    = "Information of the DUT control DLL, such as version.";
        l_11ACinsertDutReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("DUT_VERSION", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACinsertDutReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACinsertDutReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACinsertDutReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred.";
        l_11ACinsertDutReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

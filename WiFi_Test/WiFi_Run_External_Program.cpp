#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"


using namespace std;

#define MAX_RESULT_GROUP_NUM  3
#define LP_RESULT_FORMAT      "LitePoint_Data_Format"

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_runExternalProgramParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_runExternalProgramReturnMap;

struct tagParam
{
    char    PROGRAM_NAME[MAX_BUFFER_SIZE];
    char    ARGUMENTS[MAX_BUFFER_SIZE];
	char    RESULT_FILE_NAME[MAX_BUFFER_SIZE];
	char    PROGRAM_WORKING_DIRECTORY[MAX_BUFFER_SIZE];
	int     WAIT_TIME_MS;
} l_runExternalProgramParam;

struct tagReturn
{
	double  CURRENT_MA[MAX_RESULT_GROUP_NUM];
	double  VOLTAGE_V[MAX_RESULT_GROUP_NUM];
	double  POWER_MW[MAX_RESULT_GROUP_NUM];
	double  MEASUREMENT[MAX_RESULT_GROUP_NUM];
	char    EXE_CHECK[MAX_BUFFER_SIZE];	
	//char    OUTPUT_MESSAGE[MAX_BUFFER_SIZE];
	char    ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_runExternalProgramReturn;
#ifndef WIN32
int initRunExternalProgramContainers = InitializeRunExternalProgramContainers();
#endif


void ClearRunExternalProgramReturn(void)
{
	l_runExternalProgramParamMap.clear();
	l_runExternalProgramReturnMap.clear();
}

// These global variables/functions only for WiFi_Run_External_Program.cpp
int CheckRunExternalProgramParameters(char* PROGRAM_NAME, char* ARGUMENTS, char* RESULT_FILE_NAME, int WAIT_TIME_MS, char* errorMsg);

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

WIFI_TEST_API int WiFi_Run_External_Program(void)
{
    int  err = ERR_OK;
    int  dummyValue = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	char previousWorkingDirectory[MAX_BUFFER_SIZE] = {'\0'};
	char currentWorkingDirectory[MAX_BUFFER_SIZE] = {'\0'};
    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_runExternalProgramReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_runExternalProgramParamMap);
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
        RespondToQueryReturn(l_runExternalProgramReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	 //  /*-----------------------------------------------------------*
		//* Both g_WiFi_Test_ID and g_WiFi_Dut need to be valid (>=0) *
		//*-----------------------------------------------------------*/
		//if( g_WiFi_Test_ID<0 || g_WiFi_Dut<0 )  
		//{
		//	err = -1;
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID or WiFi_Dut not valid. WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		//}
		//
		//TM_ClearReturns(g_WiFi_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_runExternalProgramParamMap);
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
		//CheckDutStatus();  //don't need to check the DUT's transmit status

		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

#pragma region Prepare input parameters
		err = CheckRunExternalProgramParameters( l_runExternalProgramParam.PROGRAM_NAME, 
			l_runExternalProgramParam.ARGUMENTS, 
			l_runExternalProgramParam.RESULT_FILE_NAME,
			l_runExternalProgramParam.WAIT_TIME_MS, 
			vErrorMsg);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckWriteMacRegisterParameters() return OK.\n");
		}
#pragma endregion

#pragma region Execute external program
		// Delete the old result file first
		string delcmd = "del ";
		delcmd += l_runExternalProgramParam.RESULT_FILE_NAME;
		system(delcmd.c_str());

		// Then execute the program to generate the result file
		string cmd;
		cmd = "";
		if(strlen(l_runExternalProgramParam.PROGRAM_WORKING_DIRECTORY)>1)
		{

			//Get Current Working director
			_getcwd(previousWorkingDirectory, MAX_BUFFER_SIZE);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Current Working Directory:[%s]", previousWorkingDirectory);

			cmd += l_runExternalProgramParam.PROGRAM_WORKING_DIRECTORY;
			
			//Change to user spcified working directory
			_chdir(l_runExternalProgramParam.PROGRAM_WORKING_DIRECTORY);
			
			//Get Current Working director
			_getcwd(currentWorkingDirectory, MAX_BUFFER_SIZE);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Switch Working Directory to [%s]", currentWorkingDirectory);
		}
		else
		{
			//do nothing
		}
		
		cmd = "";
		cmd += l_runExternalProgramParam.PROGRAM_NAME; 

		if(strlen(l_runExternalProgramParam.ARGUMENTS)>1)
		{
			cmd += " ";
			cmd += l_runExternalProgramParam.ARGUMENTS;
		}

		if (strlen(l_runExternalProgramParam.RESULT_FILE_NAME)>1)
		{
			cmd += " ";
			cmd += l_runExternalProgramParam.RESULT_FILE_NAME;
		}
		//cmd += " > ";
		//cmd += l_runExternalProgramParam.RESULT_FILE_NAME;
		system(cmd.c_str());
#pragma endregion	   

#pragma region Execute sleep enough time till external program over
		Sleep(l_runExternalProgramParam.WAIT_TIME_MS);
#pragma endregion	

#pragma region Read result and return

		if (strlen(l_runExternalProgramParam.RESULT_FILE_NAME)>1)
		{
			FILE   *log = NULL;
			bool   isLitePointDataFormat = false;
			unsigned int    iSize = 0;

			string strSplitteVal;
			char   strLineBuf[MAX_BUFFER_SIZE] = "\0";
			// If the result file cannot open, something is wrong so it should fail
			if(fopen_s(&log, l_runExternalProgramParam.RESULT_FILE_NAME, "rt"))
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Could not open the result file.\n");
				err = 6;
				throw logMessage;
			}
			while(!feof(log))
			{
				if (fgets(strLineBuf, MAX_BUFFER_SIZE, log))
				{
					strSplitteVal += strLineBuf;
				}
				if(strstr(strLineBuf, LP_RESULT_FORMAT)!=NULL)
				{
					isLitePointDataFormat = true;
				}				
			}
			fclose(log);

			if (!isLitePointDataFormat)
			{
				iSize = (unsigned int)strSplitteVal.size();
				if (iSize<MAX_BUFFER_SIZE)
				{
					//If format is not right, there have to be an error!!!
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Incorrect result file format.\n");
					err = 6;
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] The result file exceed max length(4096 bytes).\n");
					err = 6;
					throw logMessage;
				}
			}
			else
			{
				map<string, string> versionMap;
				char tempStr[MAX_BUFFER_SIZE];
				ParseSplitString(strSplitteVal, "=", versionMap);

				for (int i=0; i<MAX_RESULT_GROUP_NUM; i++)
				{
					strcpy_s(tempStr, "");
					sprintf_s(tempStr, "CURRENT_%d_MA", i+1);
					if (strlen(versionMap[tempStr].c_str())>0)
					{
						l_runExternalProgramReturn.CURRENT_MA[i] = atof( versionMap[tempStr].c_str() );
					}
					sprintf_s(tempStr, "VOLTAGE_%d_V", i+1);
					if (strlen(versionMap[tempStr].c_str())>0)
					{
						l_runExternalProgramReturn.VOLTAGE_V[i] = atof( versionMap[tempStr].c_str() );
					}
					sprintf_s(tempStr, "POWER_%d_MW", i+1);
					if (strlen(versionMap[tempStr].c_str())>0)
					{
						l_runExternalProgramReturn.POWER_MW[i] = atof( versionMap[tempStr].c_str() );
					}
					sprintf_s(tempStr, "MEASUREMENT_%d", i+1);
					if (strlen(versionMap[tempStr].c_str())>0)
					{
						l_runExternalProgramReturn.MEASUREMENT[i] = atof( versionMap[tempStr].c_str() );
					}
				}

				strcpy_s(tempStr, "EXE_CHECK");
				if (strlen(versionMap[tempStr].c_str())>0)
				{
					strcpy_s(l_runExternalProgramReturn.EXE_CHECK, versionMap[tempStr].c_str());
					if (strcmp(l_runExternalProgramReturn.EXE_CHECK, "PASS") == 0)
					{
						;	
					}
					else if (strcmp(l_runExternalProgramReturn.EXE_CHECK, "FAIL") == 0)
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] The external program failed.\n");
						err = -1;
						throw logMessage;
					}
				}				
				//strcpy_s(l_runExternalProgramReturn.OUTPUT_MESSAGE, "[WiFi] The results had been extracted.\n");
			}
		}
#pragma endregion	

		if(strlen(l_runExternalProgramParam.PROGRAM_WORKING_DIRECTORY)>1)
		{
			//Switch to previous working directory
			_chdir(previousWorkingDirectory);

			_getcwd(currentWorkingDirectory, MAX_BUFFER_SIZE);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Current Working Directory to [%s]", currentWorkingDirectory);
		}
		else
		{
			//do nothing
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_runExternalProgramReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_runExternalProgramReturnMap);
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_runExternalProgramReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_runExternalProgramReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
    }

    return err;
}

int InitializeRunExternalProgramContainers(void)
{
    /*------------------*
     * Input Parameters: *
     *------------------*/
    l_runExternalProgramParamMap.clear();

    WIFI_SETTING_STRUCT setting;
    setting.unit        = "";
    setting.helpText    = "Specify which external program to run\r\nSuch as: ping.exe";
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(l_runExternalProgramParam.PROGRAM_NAME, MAX_BUFFER_SIZE, "ping.exe");
    if (MAX_BUFFER_SIZE==sizeof(l_runExternalProgramParam.PROGRAM_NAME))    // Type_Checking
    {
        setting.value       = (void*)l_runExternalProgramParam.PROGRAM_NAME;
        l_runExternalProgramParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PROGRAM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    setting.unit        = "";
    setting.helpText    = "Specify the command line arguments\r\nSuch as: 127.0.0.1";
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(l_runExternalProgramParam.ARGUMENTS, MAX_BUFFER_SIZE, "127.0.0.1");
    if (MAX_BUFFER_SIZE==sizeof(l_runExternalProgramParam.ARGUMENTS))    // Type_Checking
    {
        setting.value       = (void*)l_runExternalProgramParam.ARGUMENTS;
        l_runExternalProgramParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ARGUMENTS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.unit        = "";
	setting.helpText    = "Specify the file name that store the result.";
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(l_runExternalProgramParam.RESULT_FILE_NAME, MAX_BUFFER_SIZE, "output.log");
	if (MAX_BUFFER_SIZE==sizeof(l_runExternalProgramParam.RESULT_FILE_NAME))    // Type_Checking
	{
		setting.value       = (void*)l_runExternalProgramParam.RESULT_FILE_NAME;
		l_runExternalProgramParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RESULT_FILE_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	setting.unit        = "";
	setting.helpText    = "Specify the working directory of external program";
	setting.type = WIFI_SETTING_TYPE_STRING;
	strcpy_s(l_runExternalProgramParam.PROGRAM_WORKING_DIRECTORY, MAX_BUFFER_SIZE, "./");
	if (MAX_BUFFER_SIZE==sizeof(l_runExternalProgramParam.PROGRAM_WORKING_DIRECTORY))    // Type_Checking
	{
		setting.value       = (void*)l_runExternalProgramParam.PROGRAM_WORKING_DIRECTORY;
		l_runExternalProgramParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PROGRAM_WORKING_DIRECTORY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	
	l_runExternalProgramParam.WAIT_TIME_MS = 100;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_runExternalProgramParam.WAIT_TIME_MS))    // Type_Checking
	{
		setting.value = (void*)&l_runExternalProgramParam.WAIT_TIME_MS;
		setting.unit        = "ms";
		setting.helpText    = "Specify the wait time before extract the value from result file.";
		l_runExternalProgramParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("WAIT_TIME_MS", setting) );
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
    l_runExternalProgramReturnMap.clear();
	l_runExternalProgramReturn.EXE_CHECK[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_runExternalProgramReturn.EXE_CHECK))    // Type_Checking
	{
		setting.value       = (void*)l_runExternalProgramReturn.EXE_CHECK;
		setting.unit        = "";
		setting.helpText    = "Execute passed or failed.";
		l_runExternalProgramReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EXE_CHECK", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_runExternalProgramReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_runExternalProgramReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_runExternalProgramReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_runExternalProgramReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    //l_runExternalProgramReturn.OUTPUT_MESSAGE[0] = '\0';
    //setting.type = WIFI_SETTING_TYPE_STRING;
    //if (MAX_BUFFER_SIZE==sizeof(l_runExternalProgramReturn.OUTPUT_MESSAGE))    // Type_Checking
    //{
    //    setting.value       = (void*)l_runExternalProgramReturn.OUTPUT_MESSAGE;
    //    setting.unit        = "";
    //    setting.helpText    = "The output of the external program";
    //    l_runExternalProgramReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("OUTPUT_MESSAGE", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

	for (int i=0; i<MAX_RESULT_GROUP_NUM; i++)
	{
		char tempStr[MAX_BUFFER_SIZE];

		l_runExternalProgramReturn.CURRENT_MA[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_runExternalProgramReturn.CURRENT_MA[i]))    // Type_Checking
		{
			setting.value = (void*)&l_runExternalProgramReturn.CURRENT_MA[i];	
			sprintf_s(tempStr, "CURRENT_%d_MA", i+1);
			setting.unit        = "mA";
			setting.helpText    = "The measure current in mA.";
			l_runExternalProgramReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_runExternalProgramReturn.VOLTAGE_V[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_runExternalProgramReturn.VOLTAGE_V[i]))    // Type_Checking
		{
			setting.value = (void*)&l_runExternalProgramReturn.VOLTAGE_V[i];	
			sprintf_s(tempStr, "VOLTAGE_%d_V", i+1);
			setting.unit        = "V";
			setting.helpText    = "The measure voltage in V.";
			l_runExternalProgramReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_runExternalProgramReturn.POWER_MW[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_runExternalProgramReturn.POWER_MW[i]))    // Type_Checking
		{
			setting.value = (void*)&l_runExternalProgramReturn.POWER_MW[i];	
			sprintf_s(tempStr, "POWER_%d_MW", i+1);
			setting.unit        = "mW";
			setting.helpText    = "The measure power in mW.";
			l_runExternalProgramReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}

		l_runExternalProgramReturn.MEASUREMENT[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_runExternalProgramReturn.MEASUREMENT[i]))    // Type_Checking
		{
			setting.value = (void*)&l_runExternalProgramReturn.MEASUREMENT[i];	
			sprintf_s(tempStr, "MEASUREMENT_%d", i+1);
			setting.unit        = "";
			setting.helpText    = "The external measurement result.";
			l_runExternalProgramReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    return 0;
}

//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
//-------------------------------------------------------------------------------------
int CheckRunExternalProgramParameters(char* PROGRAM_NAME, char* ARGUMENTS, char* RESULT_FILE_NAME, int WAIT_TIME_MS, char* errorMsg)
{
	int    err = ERR_OK;
	//int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		if(0==strlen(PROGRAM_NAME))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Invalid PROGRAM_NAME.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] CheckRunExternalProgramParameters() Confirmed.\n");
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
// FM_Test.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
//#include "vDUT.h"
//#include "TestManager.h"
#include "FM_Test.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"
#include "FM_Test_Internal.h"
#include "IQmeasure.h"

// This global variable is declared in FM_Test_Internal.cpp
extern TM_ID        g_FM_Test_ID;
extern vDUT_ID      g_FM_Dut;
extern int			g_Logger_ID;
extern int          g_FM_Test_timer;
extern int			g_Tester_Type;
extern int			g_Tester_Number;
extern int			g_Tester_Reconnect;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

using namespace std;

BOOL APIENTRY DllMain( HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        InitializeAllContainers();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


FM_TEST_API int GetFMTestID(TM_ID* fmTestID)
{
	*fmTestID = g_FM_Test_ID;
	return ERR_OK;
}

FM_TEST_API int   GetFMDutID(vDUT_ID* fmDutID)
{
	*fmDutID = g_FM_Dut;
	return ERR_OK;
}

FM_TEST_API int   GetFMGlobalSetting(FM_GLOBAL_SETTING** fmGlobleSetting)
{
	*fmGlobleSetting = &g_FMGlobalSettingParam;
	return ERR_OK;
}

FM_TEST_API int FM_technologyRegister()
{
    /*---------------------*
     * Create the timer ID *
     *---------------------*/
    if( -1==g_FM_Test_timer )
    {
        TIMER_CreateTimer("FM_Test", &g_FM_Test_timer);
    }
    else
    {
        // Timer ID is already created; no need to do it again
    }

    TM_ID technologyID=-1;
    TM_RegisterTechnology("FM", &technologyID);
    if( technologyID>=0 )
    {  
        TM_InstallCallbackFunction(technologyID, "GLOBAL_SETTINGS",				FM_Global_Setting);
        TM_InstallCallbackFunction(technologyID, "CONNECT_IQ_TESTER",			FM_Connect_IQTester);
        TM_InstallCallbackFunction(technologyID, "DISCONNECT_IQ_TESTER",		FM_Disconnect_IQTester);
        TM_InstallCallbackFunction(technologyID, "INSERT_DUT",					FM_Insert_Dut);
        TM_InstallCallbackFunction(technologyID, "INITIALIZE_DUT",				FM_Initialize_Dut);
        TM_InstallCallbackFunction(technologyID, "REMOVE_DUT",					FM_Remove_Dut);
		TM_InstallCallbackFunction(technologyID, "FM_LOAD_PATH_LOSS_TABLE",		FM_LoadPathLossTable);
		TM_InstallCallbackFunction(technologyID, "RUN_EXTERNAL_PROGRAM",		FM_Run_External_Program);
        
		TM_InstallCallbackFunction(technologyID, "TX_VERIFY_RF_MEASUREMENTS",	 FM_TX_Verify_RF_MEASUREMENTS);
		TM_InstallCallbackFunction(technologyID, "TX_VERIFY_AUDIO",				 FM_TX_Verify_AUDIO);

		TM_InstallCallbackFunction(technologyID, "RX_VERIFY_AUDIO",				 FM_RX_Verify_AUDIO);
		TM_InstallCallbackFunction(technologyID, "RX_VERIFY_RSSI",			     FM_RX_Verify_RSSI);
   		TM_InstallCallbackFunction(technologyID, "RX_VERIFY_RDS",			     FM_RX_Verify_RDS);
		TM_InstallCallbackFunction(technologyID, "LOAD_PATH_LOSS_TABLE",		FM_LoadPathLossTable);

        g_FM_Test_ID = technologyID;
		
		/*----------------------*
		 * Create the logger ID *
		 *----------------------*/
		if( -1==g_Logger_ID )
		{
			LOGGER_CreateLogger("FM_Test_Log", &g_Logger_ID);
			// TODO: if possible, must do the error handling here
		}
		else
		{
			// Logger ID created already. No need to do it again
		}
    }
	else
	{
		// do nothing
	}

    return technologyID;
}

FM_TEST_API int AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult)
{
    int err = ERR_OK;
    // Averaging after all measurements are done, by an averaging function
    int i = 0;

    averageResult = 0.0;
    maxResult = minResult = resultArray[0];

	if (0==averageTimes)
	{
		return -1;	// Error, can not divided by zero
	}
	else
	{
		if (logType==Linear)
		{
			for (i=0;i<averageTimes;i++)    
			{
				maxResult = max(maxResult, resultArray[i]);
				minResult = min(minResult, resultArray[i]);
				averageResult = averageResult + resultArray[i];
			}

			averageResult = averageResult / averageTimes;        
		}
		else
		{
			if(LOG_20 == logType)
			{
				for (i=0;i<averageTimes;i++)    
				{
					maxResult = max(maxResult, resultArray[i]);
					minResult = min(minResult, resultArray[i]);
					averageResult = averageResult + pow(pow( 10,(resultArray[i]/logType)), 2);
				}
				averageResult = sqrt(averageResult / averageTimes);
			}
			else
			{
				 for (i=0;i<averageTimes;i++)    
				{
					maxResult = max(maxResult, resultArray[i]);
					minResult = min(minResult, resultArray[i]);
					averageResult = averageResult + pow( 10,(resultArray[i]/logType) );
				}
				 averageResult = averageResult / averageTimes;
			}       			

			if (0!=averageResult)
			{
				averageResult = logType*(log10(averageResult));
			}
			else
			{
				averageResult = 0;
			}
		}
	}

    return err;
}

FM_TEST_API int  RespondToQueryInput( map<string, FM_SETTING_STRUCT>& inputMap)
{
    int err = ERR_OK;

    map<string, FM_SETTING_STRUCT>::iterator inputMap_Iter;

    int     *integerValue;
    double  *doubleValue;
    char    *stringValue;

    TM_ClearReturns(g_FM_Test_ID);
    inputMap_Iter = inputMap.begin();
    while( inputMap_Iter!=inputMap.end() )
    {
        if( NULL!=inputMap_Iter->second.unit )
        {
            TM_AddUnitReturn(g_FM_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (TM_STR)inputMap_Iter->second.unit);
        }
        if( NULL!=inputMap_Iter->second.helpText )
        {
            TM_AddHelpTextReturn(g_FM_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (TM_STR)inputMap_Iter->second.helpText);
        }
        switch( inputMap_Iter->second.type )
        {
        case FM_SETTING_TYPE_INTEGER:
            integerValue = reinterpret_cast<int *>(inputMap_Iter->second.value);
            TM_AddIntegerReturn(g_FM_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), *integerValue);
            break;
        case FM_SETTING_TYPE_DOUBLE:
            doubleValue = reinterpret_cast<double *>(inputMap_Iter->second.value);
            TM_AddDoubleReturn(g_FM_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), *doubleValue);
            break;
        case FM_SETTING_TYPE_STRING:
            stringValue = reinterpret_cast<char *>(inputMap_Iter->second.value);
            TM_AddStringReturn(g_FM_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), stringValue);
            break;
        default:
            // Unknown type.  Ignore it.
            break;
        }
        inputMap_Iter++;
    }

    return err;
}


FM_TEST_API int  RespondToQueryReturn( map<string, FM_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, FM_SETTING_STRUCT>::iterator returnMap_Iter;

    TM_ClearReturns(g_FM_Test_ID);
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( NULL!=returnMap_Iter->second.unit )
        {
            TM_AddUnitReturn(g_FM_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.unit);
        }
        if( NULL!=returnMap_Iter->second.helpText )
        {
            TM_AddHelpTextReturn(g_FM_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.helpText);
        }
        switch( returnMap_Iter->second.type )
        {
        case FM_SETTING_TYPE_INTEGER:
            TM_AddIntegerReturn(g_FM_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), 0);
            break;
        case FM_SETTING_TYPE_DOUBLE:
            TM_AddDoubleReturn(g_FM_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), 0.0);
            break;
        case FM_SETTING_TYPE_STRING:
            TM_AddStringReturn(g_FM_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), "");
            break;
        default:
            // Unknown type.  Ignore it.
            break;
        }
        returnMap_Iter++;
    }

    return err;
}

FM_TEST_API int  ReturnTestResults( map<string, FM_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, FM_SETTING_STRUCT>::iterator returnMap_Iter;

    TM_ClearReturns(g_FM_Test_ID);
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( NULL!=returnMap_Iter->second.unit )
        {
            TM_AddUnitReturn(g_FM_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.unit);
        }
        if( NULL!=returnMap_Iter->second.helpText )
        {
            TM_AddHelpTextReturn(g_FM_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.helpText);
        }

        if( returnMap_Iter->second.type==FM_SETTING_TYPE_INTEGER )
        {
            int *valueInteger = reinterpret_cast<int *>(returnMap_Iter->second.value);
            if ( (NA_INTEGER)!=(*valueInteger) )
            {
                err = TM_AddIntegerReturn(g_FM_Test_ID, (char*)returnMap_Iter->first.c_str(), *valueInteger);
                if (TM_ERR_OK!=err)    
                {
                    // If TM_AddIntegerReturn fails, no need to continue
                    break;
                }
            }
            else
            {
                // do nothing
            }
        }
        else if(returnMap_Iter->second.type==FM_SETTING_TYPE_DOUBLE)
        {
            double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            if ( (NA_DOUBLE)!=(*valueDouble) )
            {
                err = TM_AddDoubleReturn(g_FM_Test_ID, (char*)returnMap_Iter->first.c_str(), *valueDouble);
                if (TM_ERR_OK!=err)    
                {
                    // If TM_AddDoubleReturn fails, no need to continue
                    break;
                }
            }
            else
            {
                // do nothing
            }
        }
        else    // returnMap_Iter->second.type==FM_SETTING_TYPE_STRING
        {   
            char *valueString = reinterpret_cast<char *>(returnMap_Iter->second.value);
            err = TM_AddStringReturn(g_FM_Test_ID, (char*)returnMap_Iter->first.c_str(), valueString);
            if (TM_ERR_OK!=err)    
            {
                // If TM_AddStringReturn fails, no need to continue
                break;
            }
        }
        returnMap_Iter++;
    }

    return err;
}

FM_TEST_API int  ClearReturnParameters( map<string, FM_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, FM_SETTING_STRUCT>::iterator returnMap_Iter;

	// clear container
    TM_ClearReturns(g_FM_Test_ID);
	
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( returnMap_Iter->second.type==FM_SETTING_TYPE_INTEGER )
        {
			int *valueInteger = reinterpret_cast<int *>(returnMap_Iter->second.value);
			*valueInteger = NA_INTEGER;
        }
        else if(returnMap_Iter->second.type==FM_SETTING_TYPE_DOUBLE)
        {
			double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            *valueDouble = NA_DOUBLE;
        }
        else    // returnMap_Iter->second.type==FM_SETTING_TYPE_STRING
        {   
			char *valueString = reinterpret_cast<char *>(returnMap_Iter->second.value);
			*valueString = '\0';

        }
        returnMap_Iter++;
    }

    return err;
}

FM_TEST_API int  ReturnErrorMessage(char *errParameter, const char *format, ...)
{
    int err = ERR_OK;

    // Log message format ... variable argument list
    va_list ap;
    va_start(ap, format);

    vsprintf_s(errParameter, MAX_BUFFER_SIZE, format, ap);
    err = TM_AddStringReturn(g_FM_Test_ID, "ERROR_MESSAGE", errParameter);

    va_end(ap);

    return err;
}

FM_TEST_API int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...)
{
    int  err = ERR_OK;
	
	// Log message format ... variable argument list
	va_list ap;
	va_start(ap, format);
	vsprintf_s(errMsg, sizeOfBuf, format, ap);
	va_end(ap);

	// Write error message to log file
	if (g_Logger_ID>=0)
	{
		err = ::LOGGER_Write(g_Logger_ID, level, errMsg);
		// TODO: if possible, must do the error handling here
	}
	else
	{
		// do nothing
		err = -1;
		sprintf_s(errMsg, sizeOfBuf, "[FM] LogReturnMessage() return error, Logger_ID < 0.\n");
	}

    return err;
}

FM_TEST_API int  GetInputParameters( map<string, FM_SETTING_STRUCT>& inputMap)
{
    int     err = ERR_OK;
	char    stringParameter[MAX_BUFFER_SIZE] = {'\0'};

    map<string, FM_SETTING_STRUCT>::iterator inputMap_Iter;

    inputMap_Iter = inputMap.begin();
    while( inputMap_Iter!=inputMap.end() )
    {
		int strError = ERR_OK;
        switch( inputMap_Iter->second.type )
        {
        case FM_SETTING_TYPE_INTEGER:
            err += TM_GetIntegerParameter(g_FM_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (int*)inputMap_Iter->second.value);
            break;
        case FM_SETTING_TYPE_DOUBLE:
            err += TM_GetDoubleParameter(g_FM_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (double*)inputMap_Iter->second.value);
            break;
        case FM_SETTING_TYPE_STRING:			
            stringParameter[0] = 0;
            strError = ::TM_GetStringParameter(g_FM_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), stringParameter, MAX_BUFFER_SIZE);
			err += strError;
            if( ERR_OK==strError )
            {
				if (0!=stringParameter[0])	// got input string
				{
					strcpy_s((char*)inputMap_Iter->second.value, MAX_BUFFER_SIZE, stringParameter);
				}
				else	// got "NULL" input string
				{
					strcpy_s((char*)inputMap_Iter->second.value, MAX_BUFFER_SIZE, "");
				}
            }
			else
			{
				// Do nothing. Sometime, the Global_Setting's input parameters will get into this case.
			}
            break;
        default:
            // Unknown type.  Ignore it.
            break;
        }
        inputMap_Iter++;
    }

    return err;
}

FM_TEST_API int  FMSaveSigFile(char* fileName)
{
    int    err = ERR_OK;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   logInfoMessage[MAX_BUFFER_SIZE] = {'\0'};

	if (1==g_FMGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[FM] The VSA_SAVE_CAPTURE_ALWAYS is set to ON, it may slow down the test time.\n");
	}

	if ( (1==g_FMGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED)||(1==g_FMGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS))  
	{	  
		// Get system time
		SYSTEMTIME   sysTime;  
		char c_time[MAX_BUFFER_SIZE], c_path[MAX_BUFFER_SIZE];

		::GetLocalTime(&sysTime);   

		sprintf_s(c_time, "%d.%d.%d-%d.%d.%d.%d", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);		    
		sprintf_s(c_path, MAX_BUFFER_SIZE, ".\\log\\%s-%s.sig", fileName, c_time);

		err = ::LP_SaveVsaSignalFile(c_path);

		sprintf_s(logInfoMessage, "[FM] FMSaveSigFile(\"%s\") return OK.\n", c_path);

		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, logInfoMessage);
	}
	else
	{
		// do nothing...
	}

	return err;
}

FM_TEST_API int   SetTesterNumber(int testerNumber)
{
    g_Tester_Number = testerNumber;
    return ERR_OK;
}

FM_TEST_API int   GetTesterNumber(int *testerNumber)
{
    *testerNumber = g_Tester_Number;
    return ERR_OK;
}

FM_TEST_API int   SetTesterType(int testerType)
{
    g_Tester_Type = testerType;
    return ERR_OK;
}

FM_TEST_API int   GetTesterType(int *testerType)
{
    *testerType = g_Tester_Type;
    return ERR_OK;
}

FM_TEST_API int   SetTesterReconnect(int flag)
{
    g_Tester_Reconnect = flag;
    return ERR_OK;
}

FM_TEST_API int   GetTesterReconnect(int *flag)
{
    *flag = g_Tester_Reconnect;
    return ERR_OK;
}


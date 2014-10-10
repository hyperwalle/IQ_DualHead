#include "stdafx.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "IQlite_Logger.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "math.h"
//Move to stdafx.h
//#include "lp_time.h"


using namespace std;

// global variable 
TM_ID            g_BT_Test_ID	    = -1;
vDUT_ID          g_BT_Dut		    = -1;
bool             g_ReloadDutDll		= false;
bool			 g_vDutTxActived    = false;
bool			 g_vDutRxActived    = false;
int				 g_Logger_ID        = -1;
int              g_BT_Test_timer    = -1;
int				 g_Tester_Type      = IQ_View;
int				 g_Tester_Number    = 0;
int				 g_Tester_Reconnect = 0;

const char      *g_BT_Test_Version = "1.6.0 (2010-09-17)\n";
char			 g_defaultFilePath[MAX_BUFFER_SIZE] = {'\0'};

// This global variable is declared in BT_Global_Setting.cpp
// Input Parameter Container
extern map<string, BT_SETTING_STRUCT> g_BTGlobalSettingParamMap;
extern BT_GLOBAL_SETTING g_BTGlobalSettingParam;
extern int       g_iTesterName;

// end global


//! BT Test Internal Functions
/*!
 * Input Parameters
 *
 *  - None
 *
 * Return Values
 *      -# A string for possible error message
 *
 * \return 0 No error occurred
 * \return -1 DUT failed to insert.  Please see the returned error message for details
 */

int InitializeAllContainers(void)
{
    InitializeConnectTesterContainers();            // Needed by BT_Connect_IQTester
    InitializeDisconnectTesterContainers();         // Needed by BT_Disconnect_IQTester
    InitializeInsertDutContainers();                // Needed by BT_Insert_Dut
    InitializeInitializeDutContainers();            // Needed by BT_Initialize_Dut
    InitializeRemoveDutContainers();                // Needed by BT_Remove_Dut
    InitializeTXVerifyBDRContainers();              // Needed by BT_TX_Verify_BDR
	InitializeTXVerifyEDRContainers();              // Needed by BT_TX_Verify_EDR
    InitializeRXVerifyBDRContainers();              // Needed by BT_RX_Verify_BDR
	InitializeRXVerifyEDRContainers();              // Needed by BT_RX_Verify_EDR
	InitializeTXPowerControlContainers();

    InitializeTXVerifyLEContainers();              // Needed by BT_TX_Verify_LE
    InitializeRXVerifyLEContainers();              // Needed by BT_RX_Verify_LE
	InitializebtleInitializeContainers();		   // Needed by BT_LE_INITIALIZE

	InitializeWriteBDAddressContainers();           // Needed by BT_WRITE_BD_ADDRESS   
	InitializeReadBDAddressContainers();            // Needed by BT_READ_BD_ADDRESS   
  
    InitializeGlobalSettingContainers();            // Needed by BT_Global_Setting
	InitializeTXCalContainers();
    InitializeloadPathLossTableContainers();
	InitializeWriteDutInfoContainers();             //Needed by BT_WRITE_DUT_INFO

	InitializeRunExternalProgramContainers();       // Needed by BT_Run_External_Program
	InitializePowerModeDutContainers();				// Needed by BT_Power_Mode_Dut
	InitializeResetDutContainers();					// Needed by BT_RESET_DUT
    return 0;
}
int CleanupAllBTContainers(void)
{
	ClearConnectLPReturn();
	ClearDisconnectLPReturn();
	ClearGlobalSettingReturn();
	ClearInitializeDutReturn();
	ClearInsertDutReturn();
	ClearLoadPathLossTableReturn();
	ClearCurrentTestReturn();
	ClearReadBDAddressReturn();
	ClearRemoveDutReturn();
	ClearRunExternalProgramReturn();
	ClearRxVerifyBdrReturn();
	ClearRxVerifyEdrReturn();
	ClearTxCalReturn();
	ClearTxPowerControlReturn();
	ClearTxVerifyBDRReturn();
	ClearTxVerifyEDRReturn();
	ClearWriteBDAddressReturn();
	ClearwriteDutInfoReturn();
	//BT LE 
	ClearTxVerifyLEReturn();
	ClearRxVerifyLEReturn();
	ClearbtleInitializeReturn();
	return 0;
}

BT_TEST_API int AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult)
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
			if( logType==RMS_LOG_20 )
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
			if (averageResult>0)
			{
				averageResult = logType*(log10(averageResult));
			}
			else
			{
				averageResult = NA_NUMBER;
			}
		}
	}

    return err;
}

BT_TEST_API int  RespondToQueryInput( map<string, BT_SETTING_STRUCT>& inputMap)
{
    int err = ERR_OK;

    map<string, BT_SETTING_STRUCT>::iterator inputMap_Iter;

    int     *integerValue;
    double  *doubleValue;
    char    *stringValue;

    TM_ClearReturns(g_BT_Test_ID);
    inputMap_Iter = inputMap.begin();
    while( inputMap_Iter!=inputMap.end() )
    {
        if( NULL!=inputMap_Iter->second.unit )
        {
            TM_AddUnitReturn(g_BT_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (TM_STR)inputMap_Iter->second.unit);
        }
        if( NULL!=inputMap_Iter->second.helpText )
        {
            TM_AddHelpTextReturn(g_BT_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (TM_STR)inputMap_Iter->second.helpText);
        }
        switch( inputMap_Iter->second.type )
        {
        case BT_SETTING_TYPE_INTEGER:
            integerValue = reinterpret_cast<int *>(inputMap_Iter->second.value);
            TM_AddIntegerReturn(g_BT_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), *integerValue);
            break;
        case BT_SETTING_TYPE_DOUBLE:
            doubleValue = reinterpret_cast<double *>(inputMap_Iter->second.value);
            TM_AddDoubleReturn(g_BT_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), *doubleValue);
            break;
        case BT_SETTING_TYPE_STRING:
            stringValue = reinterpret_cast<char *>(inputMap_Iter->second.value);
            TM_AddStringReturn(g_BT_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), stringValue);
            break;
        default:
            // Unknown type.  Ignore it.
            break;
        }
        inputMap_Iter++;
    }

    return err;
}


BT_TEST_API int  RespondToQueryReturn( map<string, BT_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, BT_SETTING_STRUCT>::iterator returnMap_Iter;

    TM_ClearReturns(g_BT_Test_ID);
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( NULL!=returnMap_Iter->second.unit )
        {
            TM_AddUnitReturn(g_BT_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.unit);
        }
        if( NULL!=returnMap_Iter->second.helpText )
        {
            TM_AddHelpTextReturn(g_BT_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.helpText);
        }
        switch( returnMap_Iter->second.type )
        {
        case BT_SETTING_TYPE_INTEGER:
            TM_AddIntegerReturn(g_BT_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), 0);
            break;
        case BT_SETTING_TYPE_DOUBLE:
            TM_AddDoubleReturn(g_BT_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), 0.0);
            break;
        case BT_SETTING_TYPE_STRING:
            TM_AddStringReturn(g_BT_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), "");
            break;
        case BT_SETTING_TYPE_ARRAY_DOUBLE:
            ::TM_AddArrayDoubleReturn(g_BT_Test_ID,(TM_STR)returnMap_Iter->first.c_str(),NULL, 0);
            break;
        default:
            // Unknown type.  Ignore it.
            break;
        }
        returnMap_Iter++;
    }

    return err;
}

BT_TEST_API int  ReturnTestResults( map<string, BT_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, BT_SETTING_STRUCT>::iterator returnMap_Iter;

    TM_ClearReturns(g_BT_Test_ID);
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( NULL!=returnMap_Iter->second.unit )
        {
            TM_AddUnitReturn(g_BT_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.unit);
        }
        if( NULL!=returnMap_Iter->second.helpText )
        {
            TM_AddHelpTextReturn(g_BT_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.helpText);
        }

        if( returnMap_Iter->second.type==BT_SETTING_TYPE_INTEGER )
        {
            int *valueInteger = reinterpret_cast<int *>(returnMap_Iter->second.value);
            if ( (*valueInteger)>(NA_NUMBER+1) )
            {
                err = TM_AddIntegerReturn(g_BT_Test_ID, (char*)returnMap_Iter->first.c_str(), *valueInteger);
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
        else if(returnMap_Iter->second.type==BT_SETTING_TYPE_DOUBLE)
        {
            double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            if ( (*valueDouble)>(NA_NUMBER+1) )
            {
                err = TM_AddDoubleReturn(g_BT_Test_ID, (char*)returnMap_Iter->first.c_str(), *valueDouble);
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
        else if(returnMap_Iter->second.type==BT_SETTING_TYPE_STRING)
        {   
            char *valueString = reinterpret_cast<char *>(returnMap_Iter->second.value);
            err = TM_AddStringReturn(g_BT_Test_ID, (char*)returnMap_Iter->first.c_str(), valueString);
            if (TM_ERR_OK!=err)    
            {
                // If TM_AddStringReturn fails, no need to continue
                break;
            }
        }
        else //returnMap_Iter->second.type==BT_SETTING_TYPE_ARRAY_DOUBLE
        {
            vector<double> doubleVector = * reinterpret_cast<vector<double> *>(returnMap_Iter->second.value);
            if (0 != doubleVector.size())
            {
                double *doubleArray = new double[(int) doubleVector.size()];
                for (unsigned int i=0; i<doubleVector.size(); i++)
                {
                    doubleArray[i] = doubleVector[i];
                }
                err = TM_AddArrayDoubleReturn(g_BT_Test_ID, (char*)returnMap_Iter->first.c_str(), doubleArray, (int)doubleVector.size());

                delete [] doubleArray;

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

        returnMap_Iter++;
    }

    return err;
}

BT_TEST_API int  ClearReturnParameters( map<string, BT_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, BT_SETTING_STRUCT>::iterator returnMap_Iter;

	// clear container
    TM_ClearReturns(g_BT_Test_ID);
	
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( returnMap_Iter->second.type==BT_SETTING_TYPE_INTEGER )
        {
			int *valueInteger = reinterpret_cast<int *>(returnMap_Iter->second.value);
			*valueInteger = (int)NA_NUMBER;
        }
        else if(returnMap_Iter->second.type==BT_SETTING_TYPE_DOUBLE)
        {
			double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            *valueDouble = NA_NUMBER;
        }
        else    // returnMap_Iter->second.type==BT_SETTING_TYPE_STRING
        {   
			char *valueString = reinterpret_cast<char *>(returnMap_Iter->second.value);
			*valueString = '\0';

        }
        returnMap_Iter++;
    }

    return err;
}

BT_TEST_API int  ReturnErrorMessage(char *errParameter, const char *format, ...)
{
    int err = ERR_OK;

    // Log message format ... variable argument list
    va_list ap;
    va_start(ap, format);

    vsprintf_s(errParameter, MAX_BUFFER_SIZE, format, ap);
    err = TM_AddStringReturn(g_BT_Test_ID, "ERROR_MESSAGE", errParameter);
	// Ignore error handling

	err = ::vDUT_Run( g_BT_Dut, "FUNCTION_FAILED" );
	// Ignore error handling

    va_end(ap);

    return err;
}

BT_TEST_API int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...)
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
		err = ::LOGGER_Write_Ext(LOG_IQLITE_CORE, g_Logger_ID, level, errMsg);
		// TODO: if possible, must do the error handling here
	}
	else
	{
		// do nothing
		err = -1;
		sprintf_s(errMsg, sizeOfBuf, "[BT] LogReturnMessage() return error, Logger_ID < 0.\n");
	}

    return err;
}

BT_TEST_API int  GetInputParameters( map<string, BT_SETTING_STRUCT>& inputMap)
{
    int     err = ERR_OK;
	char    stringParameter[MAX_BUFFER_SIZE] = {'\0'};

    map<string, BT_SETTING_STRUCT>::iterator inputMap_Iter;

    inputMap_Iter = inputMap.begin();
    while( inputMap_Iter!=inputMap.end() )
    {
		int strError = ERR_OK;
        switch( inputMap_Iter->second.type )
        {
        case BT_SETTING_TYPE_INTEGER:
            err += TM_GetIntegerParameter(g_BT_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (int*)inputMap_Iter->second.value);
            break;
        case BT_SETTING_TYPE_DOUBLE:
            err += TM_GetDoubleParameter(g_BT_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (double*)inputMap_Iter->second.value);
            break;
        case BT_SETTING_TYPE_STRING:			
            stringParameter[0] = 0;
            strError = ::TM_GetStringParameter(g_BT_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), stringParameter, MAX_BUFFER_SIZE);
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

BT_TEST_API int  BTSaveSigFile(char* fileName)
{
    int    err = ERR_OK;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   logInfoMessage[MAX_BUFFER_SIZE] = {'\0'};

	if (1==g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[BT] The VSA_SAVE_CAPTURE_ALWAYS is set to ON, it may slow down the test time.\n");
	}

	if ( (1==g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED)||(1==g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS))  
	{	  
		char c_time[MAX_BUFFER_SIZE], c_path[MAX_BUFFER_SIZE];
#ifdef WIN32
		// Get system time
		SYSTEMTIME   sysTime;  
		::GetLocalTime(&sysTime);   
		sprintf_s(c_time, "%d.%d.%d-%d.%d.%d.%d", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);		       
#else
        time_t rawtime;
        struct tm timeInfo;
        time(&rawtime);
        localtime_s(&timeInfo, &rawtime);
        // Note this does not have milliseconds, as the win32 version does.
        strftime(c_time, MAX_BUFFER_SIZE, "%y.%m.%d-%H.%M.%S", &timeInfo);
#endif

		char cFileExt[10] = {'\0'};
		if( g_iTesterName == IQTYPE_2010 )
			sprintf_s(cFileExt, 10, "%s", "sig");
		else if( g_iTesterName == IQTYPE_XEL )
			sprintf_s(cFileExt, 10, "%s", "iqvsa");

		sprintf_s(c_path, MAX_BUFFER_SIZE, "./log/%s-%s.%s", fileName, c_time, cFileExt);

		err = ::LP_SaveVsaSignalFile(c_path);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] LP_SaveVsaSignalFile(\"%s\") return error.\n", c_path);
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[BT] Save Signal File (\"%s\").\n", c_path );
		}
	}
	else
	{
		// do nothing...
	}

	return err;
}

BT_TEST_API int  GetWaveformFileName(char* perfix, char* postfix, char* packetType, char* waveformFileName, int bufferSize)
{
    int  err = ERR_OK;

	char keyword[MAX_BUFFER_SIZE] = {'\0'};

	map<string, BT_SETTING_STRUCT>::iterator inputMap_Iter;

	// Find out the keyword of the waveform file name
	sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_%s_%s", perfix, packetType, postfix);			

    // Searching the keyword in g_BTGlobalSettingParamMap
	inputMap_Iter = g_BTGlobalSettingParamMap.find(keyword);
    if( inputMap_Iter!=g_BTGlobalSettingParamMap.end() )
    {
		char dummyString[MAX_BUFFER_SIZE];
		strcpy_s (dummyString, MAX_BUFFER_SIZE, (char*)inputMap_Iter->second.value);

		if ( strlen(g_defaultFilePath)>0 )		// default waveform file path not empty
		{
			sprintf_s(waveformFileName, bufferSize, "%s/%s", g_defaultFilePath, dummyString);
		}
		else	// if default waveform file path is empty, then use g_BTGlobalSettingParam.BER_WAVEFORM_PATH
		{
			sprintf_s(waveformFileName, bufferSize, "%s/%s", g_BTGlobalSettingParam.BER_WAVEFORM_PATH, dummyString);
		}	


		//if Tester is IQxel, change the waveform filename extension from .mod to .iqvsg 
		if( g_iTesterName == IQTYPE_XEL )
		{
			char *pch = NULL;
			pch = strstr(waveformFileName,".mod\0");
			if(pch!= NULL)
				strncpy (pch,".iqvsg\0",7);
		}


		// Wave file checking
		FILE *waveFile;
		fopen_s(&waveFile, waveformFileName, "r");
		if (!waveFile)
		{
			if ( strlen(g_defaultFilePath)>0 )		// Default waveform file path is not empty, but still can't find the waveformfile, then return error.
			{
				err = TM_ERR_PARAM_DOES_NOT_EXIST;
			}
			else	// Try to find the file in higher-level folders
			{
				char tempPath[MAX_BUFFER_SIZE] = {'\0'};
				sprintf_s(tempPath, bufferSize, "../%s", waveformFileName);

				fopen_s(&waveFile, tempPath, "r");
				if (!waveFile)
				{
					err = TM_ERR_PARAM_DOES_NOT_EXIST;
				}
				else
				{
					// Save the path to default path name
					sprintf_s(g_defaultFilePath, bufferSize, "../%s", g_BTGlobalSettingParam.BER_WAVEFORM_PATH);
					// And use the new path
					sprintf_s(waveformFileName, bufferSize, "%s", tempPath);

					fclose(waveFile);
					waveFile = NULL;
				}
			}
		}
		else
		{
			fclose(waveFile);
			waveFile = NULL;
		}
	}
    else
    {
        err = TM_ERR_PARAM_DOES_NOT_EXIST;
    }

	return err;
}


BT_TEST_API int  AddWaveformPath( char* waveformName, char* waveformFileName, int bufferSize)
{
    int  err = ERR_OK;
  
        if ( strlen(g_defaultFilePath)>0 )		// default waveform file path not empty
        {
            sprintf_s(waveformFileName, bufferSize, "%s/%s", g_defaultFilePath, waveformName);
        }
        else	// if default waveform file path is empty, then use g_BTGlobalSettingParam.PER_WAVEFORM_PATH
        {
            sprintf_s(waveformFileName, bufferSize, "%s/%s", g_BTGlobalSettingParam.BER_WAVEFORM_PATH, waveformName);
        }		

		//if Tester is IQxel, change the waveform filename extension from .mod to .iqvsg 
		if( g_iTesterName == IQTYPE_XEL )
		{
			char *pch = NULL;
			pch = strstr(waveformFileName,".mod\0");
			if(pch!= NULL)
				strncpy (pch,".iqvsg\0",7);
		}


        // Wave file checking
        FILE *waveFile;
        fopen_s(&waveFile, waveformFileName, "r");
        if (!waveFile)
        {
            if ( strlen(g_defaultFilePath)>0 )		// Default waveform file path is not empty, but still can't find the waveformfile, then return error.
            {
                err = TM_ERR_PARAM_DOES_NOT_EXIST;
            }
            else	// Try to find the file in higher-level folders
            {
                char tempPath[MAX_BUFFER_SIZE] = {'\0'};
                sprintf_s(tempPath, bufferSize, "../%s", waveformFileName);

                fopen_s(&waveFile, tempPath, "r");
                if (!waveFile)
                {
                    err = TM_ERR_PARAM_DOES_NOT_EXIST;
                }
                else
                {
                    // Save the path to default path name
                    sprintf_s(g_defaultFilePath, bufferSize, "../%s", g_BTGlobalSettingParam.BER_WAVEFORM_PATH);
                    // And use the new path
                    sprintf_s(waveformFileName, bufferSize, "%s", tempPath);

                    fclose(waveFile);
                    waveFile = NULL;
                }
            }
        }
        else
        {
            fclose(waveFile);
            waveFile = NULL;
        }

        return err;
}

BT_TEST_API int  GetPacketLength(char* perfix, char* postfix, char* packetType, int *packetLength)
{
    int  err = ERR_OK;

	char keyword[MAX_BUFFER_SIZE] = {'\0'};

	map<string, BT_SETTING_STRUCT>::iterator inputMap_Iter;
	

	// Find out the keyword of the waveform file name	
	sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_%s_%s", perfix, packetType, postfix);			

    // Search the keyword in g_BTGlobalSettingParamMap
	inputMap_Iter = g_BTGlobalSettingParamMap.find(keyword);
    if( inputMap_Iter!=g_BTGlobalSettingParamMap.end() )
    {
		*packetLength = *((int*)inputMap_Iter->second.value);
	}
    else
    {
        err = TM_ERR_PARAM_DOES_NOT_EXIST;
    }

	return err;
}

BT_TEST_API double PacketTypeToSamplingTimeUs(char *packetType)
{
	double timeUs = 0;

    if ( 0==strcmp(packetType, "1LE"))
    {
        timeUs = g_BTGlobalSettingParam.BT_1LE_SAMPLE_INTERVAL_US;	// 1LE => 625 us
    }
    else if ( (0==strcmp(packetType, "1DH1"))||(0==strcmp(packetType, "2DH1"))||(0==strcmp(packetType, "3DH1")))
	{
		timeUs = g_BTGlobalSettingParam.BT_DH1_SAMPLE_INTERVAL_US;	// DH1 => 625 us
	}
	else if ( (0==strcmp(packetType, "1DH3"))||(0==strcmp(packetType, "2DH3"))||(0==strcmp(packetType, "3DH3")) )
	{
		timeUs = g_BTGlobalSettingParam.BT_DH3_SAMPLE_INTERVAL_US;	// DH3 => 1875 us
	}
	else	// DH5
	{
		timeUs = g_BTGlobalSettingParam.BT_DH5_SAMPLE_INTERVAL_US;	// DH5 => 3125 us
	}

	return timeUs;
}

BT_TEST_API int  AnalyzeCWFrequency(double centerFreqMHz, double vsaAmpPowerDbm, double samplingTimeUs, int avgTimes, double *cwFreqResultHz, char *errorMsg)
{
    int  err = ERR_OK;

	int    avgIteration = 0;
	double cwFreqHz = 0.0;
	double cwFreqHz_sum = 0.0;
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};


	/*--------------------*
	 * Setup IQtester VSA *
	 *--------------------*/
	err = LP_SetVsaBluetooth(  (centerFreqMHz+BT_FREQ_SHIFT_FOR_CW_MHZ)*1e6,
							   vsaAmpPowerDbm,
							   g_BTGlobalSettingParam.VSA_PORT,
							   g_BTGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
							   g_BTGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
							 );
    if (ERR_OK!=err)
    {
        sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[BT] Fail to setup VSA.\n");
        return err;
    }

   /*----------------------------*
	* Disable VSG output signal  *
	*----------------------------*/
	// make sure no signal is generated by the VSG
	//err = ::LP_EnableVsgRF(0);
	//if ( ERR_OK!=err )
	//{
	//	sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[BT] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
	//	return err;
	//}

   /*--------------------------------*
    * Start "while" loop for average *
    *--------------------------------*/
    avgIteration = 0;

	while(avgIteration<avgTimes)
    {
	   /*----------------------------*
		* Perform normal VSA capture *
		*----------------------------*/
        err = LP_VsaDataCapture(samplingTimeUs/1000000, IQV_TRIG_TYPE_FREE_RUN);              
        if(ERR_OK==err)	// capture is ok
        {
			sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[BT] LP_VsaDataCapture() at %.1f MHz return OK.\n", (centerFreqMHz+BT_FREQ_SHIFT_FOR_CW_MHZ));
				
			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			if (1==g_BTGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "BT_CW_FREQ_SaveAlways", (centerFreqMHz+BT_FREQ_SHIFT_FOR_CW_MHZ));
				BTSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}

			/*---------------*
			 *  CW Analysis  *
			 *---------------*/
			err = LP_AnalyzeCW();	
			if ( ERR_OK!=err )
            {   // Fail Analysis, thus save capture (Signal File) for debug    
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "BT_CW_FREQ_AnalysisFailed", (centerFreqMHz+BT_FREQ_SHIFT_FOR_CW_MHZ));
				BTSaveSigFile(sigFileNameBuffer);				
				sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[BT] Fail to analyze signal at %.1f MHz.\n", (centerFreqMHz+BT_FREQ_SHIFT_FOR_CW_MHZ));
                break;
			}
			else	// Analyze is ok
			{                
			   /*-----------------------------*
			    *  Retrieve analysis Results  *
			    *-----------------------------*/
				avgIteration++;

				// Store the result
				cwFreqHz = LP_GetScalarMeasurement("frequency", 0);
				cwFreqHz = cwFreqHz + BT_FREQ_SHIFT_FOR_CW_MHZ*1e6;
				cwFreqHz_sum = cwFreqHz_sum + cwFreqHz;				
			}

        }
        else
        {   
            // Fail Capture
			err = -1;
            sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[BT] Fail to capture signal at %.1f MHz.\n", (centerFreqMHz+BT_FREQ_SHIFT_FOR_CW_MHZ));
            break;
        }
    }

   /*-----------------------*
    *  Return Test Results  *
    *-----------------------*/
    if( avgIteration && (err==ERR_OK) )
    {
        *cwFreqResultHz = cwFreqHz_sum/avgIteration;
    }
    else
    {
        *cwFreqResultHz = NA_NUMBER;
    }	

	return err;
}

BT_TEST_API int GetBTTestID(TM_ID* BTTestID)
{
	*BTTestID = g_BT_Test_ID;
    return ERR_OK;
}

BT_TEST_API int   GetBTDutID(vDUT_ID* BTDutID)
{
    *BTDutID = g_BT_Dut;
    return ERR_OK;
}

BT_TEST_API int   GetBTGlobalSetting(BT_GLOBAL_SETTING** BTGlobleSetting)
{
    *BTGlobleSetting = &g_BTGlobalSettingParam;
    return ERR_OK;
}

BT_TEST_API int   SetTesterNumber(int testerNumber)
{
    g_Tester_Number = testerNumber;
    return ERR_OK;
}

BT_TEST_API int   GetTesterNumber(int *testerNumber)
{
    *testerNumber = g_Tester_Number;
    return ERR_OK;
}

BT_TEST_API int   SetTesterType(int testerType)
{
    g_Tester_Type = testerType;
    return ERR_OK;
}

BT_TEST_API int   GetTesterType(int *testerType)
{
    *testerType = g_Tester_Type;
    return ERR_OK;
}

BT_TEST_API int   SetTesterReconnect(int flag)
{
    g_Tester_Reconnect = flag;
    return ERR_OK;
}

BT_TEST_API int   GetTesterReconnect(int *flag)
{
    *flag = g_Tester_Reconnect;
    return ERR_OK;
}

BT_TEST_API int   GetTesterName(int *flag)
{
	*flag = g_iTesterName;
	return ERR_OK;
}

BT_TEST_API int   SetTesterName(int flag)
{
	g_iTesterName = flag;
	return ERR_OK;
}
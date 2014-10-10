#include "stdafx.h"
#include "TestManager.h"
#include "GPS_Test.h"
#include "IQlite_Logger.h"
#include "GPS_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "math.h"


using namespace std;

// global variable 
TM_ID            g_GPS_Test_ID		= -1;
vDUT_ID          g_GPS_Dut			= -1;
bool             g_ReloadDutDll		= false;
int				 g_Logger_ID        = -1;
int              g_GPS_Test_timer   = -1;
int				 g_Tester_Type      = IQ_View;
int				 g_Tester_Number    = 0;
int				 g_Tester_Reconnect = 0;

const char      *g_GPS_Test_Version = "1.7.0 (2011-03-04)\n";
char			 g_defaultFilePath[MAX_BUFFER_SIZE] = {'\0'};

// This global variable is declared in GPS_Global_Setting.cpp
// Input Parameter Container
extern map<string, GPS_SETTING_STRUCT> g_GPSGlobalSettingParamMap;
extern GPS_GLOBAL_SETTING g_GPSGlobalSettingParam;

// end global


//! GPS Test Internal Functions
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

void InitializeAllGPSContainers(void)
{
    InitializeConnectTesterContainers();            // Needed by GPS_Connect_IQTester
    InitializeDisconnectTesterContainers();         // Needed by GPS_Disconnect_IQTester
    InitializeInsertDutContainers();                // Needed by GPS_Insert_Dut
    InitializeInitializeDutContainers();            // Needed by GPS_Initialize_Dut
    InitializeRemoveDutContainers();                // Needed by GPS_Remove_Dut

    InitializeGlobalSettingContainers();            // Needed by GPS_Global_Setting
    InitializeloadPathLossTableContainers();

    InitializeGPSTransmitScenarioContainers();
    InitializeGPSModulatedModeContainers();
    InitializeGPSContinueWaveContainers();
}

void CleanupAllGPSContainers(void)
{
	ClearConnectLPReturn();
	ClearDisconnectLPReturn();
	ClearGlobalSettingReturn();
	ClearInsertDutReturn();
	ClearInitializeDutReturn();
	ClearRemoveDutReturn();
	ClearLoadPathLossTableReturn();
    ClearTransmitScenarioReturn();
    ClearModulatedModeReturn();
    ClearContinueWaveReturn();
}

GPS_TEST_API int AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult)
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

GPS_TEST_API int  RespondToQueryInput( map<string, GPS_SETTING_STRUCT>& inputMap)
{
    int err = ERR_OK;

    map<string, GPS_SETTING_STRUCT>::iterator inputMap_Iter;

    int     *integerValue;
    double  *doubleValue;
    char    *stringValue;

    TM_ClearReturns(g_GPS_Test_ID);
    inputMap_Iter = inputMap.begin();
    while( inputMap_Iter!=inputMap.end() )
    {
        if( NULL!=inputMap_Iter->second.unit )
        {
            TM_AddUnitReturn(g_GPS_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (TM_STR)inputMap_Iter->second.unit);
        }
        if( NULL!=inputMap_Iter->second.helpText )
        {
            TM_AddHelpTextReturn(g_GPS_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (TM_STR)inputMap_Iter->second.helpText);
        }
        switch( inputMap_Iter->second.type )
        {
        case GPS_SETTING_TYPE_INTEGER:
            integerValue = reinterpret_cast<int *>(inputMap_Iter->second.value);
            TM_AddIntegerReturn(g_GPS_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), *integerValue);
            break;
        case GPS_SETTING_TYPE_DOUBLE:
            doubleValue = reinterpret_cast<double *>(inputMap_Iter->second.value);
            TM_AddDoubleReturn(g_GPS_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), *doubleValue);
            break;
        case GPS_SETTING_TYPE_STRING:
            stringValue = reinterpret_cast<char *>(inputMap_Iter->second.value);
            TM_AddStringReturn(g_GPS_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), stringValue);
            break;
        default:
            // Unknown type.  Ignore it.
            break;
        }
        inputMap_Iter++;
    }

    return err;
}


GPS_TEST_API int  RespondToQueryReturn( map<string, GPS_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, GPS_SETTING_STRUCT>::iterator returnMap_Iter;

    TM_ClearReturns(g_GPS_Test_ID);
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( NULL!=returnMap_Iter->second.unit )
        {
            TM_AddUnitReturn(g_GPS_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.unit);
        }
        if( NULL!=returnMap_Iter->second.helpText )
        {
            TM_AddHelpTextReturn(g_GPS_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.helpText);
        }
        switch( returnMap_Iter->second.type )
        {
        case GPS_SETTING_TYPE_INTEGER:
            TM_AddIntegerReturn(g_GPS_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), 0);
            break;
        case GPS_SETTING_TYPE_DOUBLE:
            TM_AddDoubleReturn(g_GPS_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), 0.0);
            break;
        case GPS_SETTING_TYPE_STRING:
            TM_AddStringReturn(g_GPS_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), "");
            break;
        case GPS_SETTING_TYPE_ARRAY_DOUBLE:
            ::TM_AddArrayDoubleReturn(g_GPS_Test_ID,(TM_STR)returnMap_Iter->first.c_str(),NULL, 0);
            break;
        default:
            // Unknown type.  Ignore it.
            break;
        }
        returnMap_Iter++;
    }

    return err;
}

GPS_TEST_API int  ReturnTestResults( map<string, GPS_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, GPS_SETTING_STRUCT>::iterator returnMap_Iter;

    TM_ClearReturns(g_GPS_Test_ID);
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( NULL!=returnMap_Iter->second.unit )
        {
            TM_AddUnitReturn(g_GPS_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.unit);
        }
        if( NULL!=returnMap_Iter->second.helpText )
        {
            TM_AddHelpTextReturn(g_GPS_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.helpText);
        }

        if( returnMap_Iter->second.type==GPS_SETTING_TYPE_INTEGER )
        {
            int *valueInteger = reinterpret_cast<int *>(returnMap_Iter->second.value);
            if ( (*valueInteger)>(NA_NUMBER+1) )
            {
                err = TM_AddIntegerReturn(g_GPS_Test_ID, (char*)returnMap_Iter->first.c_str(), *valueInteger);
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
        else if(returnMap_Iter->second.type==GPS_SETTING_TYPE_DOUBLE)
        {
            double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            if ( (*valueDouble)>(NA_NUMBER+1) )
            {
                err = TM_AddDoubleReturn(g_GPS_Test_ID, (char*)returnMap_Iter->first.c_str(), *valueDouble);
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
        else if(returnMap_Iter->second.type==GPS_SETTING_TYPE_STRING)
        {   
            char *valueString = reinterpret_cast<char *>(returnMap_Iter->second.value);
            err = TM_AddStringReturn(g_GPS_Test_ID, (char*)returnMap_Iter->first.c_str(), valueString);
            if (TM_ERR_OK!=err)    
            {
                // If TM_AddStringReturn fails, no need to continue
                break;
            }
        }
        else //returnMap_Iter->second.type==GPS_SETTING_TYPE_ARRAY_DOUBLE
        {
            vector<double> doubleVector = * reinterpret_cast<vector<double> *>(returnMap_Iter->second.value);
            if (0 != doubleVector.size())
            {
                double *doubleArray = new double[(int) doubleVector.size()];
                for (unsigned int i=0; i<doubleVector.size(); i++)
                {
                    doubleArray[i] = doubleVector[i];
                }
                err = TM_AddArrayDoubleReturn(g_GPS_Test_ID, (char*)returnMap_Iter->first.c_str(), doubleArray, (int)doubleVector.size());

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

GPS_TEST_API int  ClearReturnParameters( map<string, GPS_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, GPS_SETTING_STRUCT>::iterator returnMap_Iter;

	// clear container
    TM_ClearReturns(g_GPS_Test_ID);
	
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( returnMap_Iter->second.type==GPS_SETTING_TYPE_INTEGER )
        {
			int *valueInteger = reinterpret_cast<int *>(returnMap_Iter->second.value);
			*valueInteger = (int)NA_NUMBER;
        }
        else if(returnMap_Iter->second.type==GPS_SETTING_TYPE_DOUBLE)
        {
			double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            *valueDouble = NA_NUMBER;
        }
        else    // returnMap_Iter->second.type==GPS_SETTING_TYPE_STRING
        {   
			char *valueString = reinterpret_cast<char *>(returnMap_Iter->second.value);
			*valueString = '\0';

        }
        returnMap_Iter++;
    }

    return err;
}

GPS_TEST_API int  ReturnErrorMessage(char *errParameter, const char *format, ...)
{
    int err = ERR_OK;

    // Log message format ... variable argument list
    va_list ap;
    va_start(ap, format);

    vsprintf_s(errParameter, MAX_BUFFER_SIZE, format, ap);
    err = TM_AddStringReturn(g_GPS_Test_ID, "ERROR_MESSAGE", errParameter);
	// Ignore error handling

	err = ::vDUT_Run( g_GPS_Dut, "FUNCTION_FAILED" );
	// Ignore error handling

    va_end(ap);

    return err;
}

GPS_TEST_API int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...)
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
		sprintf_s(errMsg, sizeOfBuf, "[GPS] LogReturnMessage() return error, Logger_ID < 0.\n");
	}

    return err;
}

GPS_TEST_API int  GetInputParameters( map<string, GPS_SETTING_STRUCT>& inputMap)
{
    int     err = ERR_OK;
	char    stringParameter[MAX_BUFFER_SIZE] = {'\0'};

    map<string, GPS_SETTING_STRUCT>::iterator inputMap_Iter;

    inputMap_Iter = inputMap.begin();
    while( inputMap_Iter!=inputMap.end() )
    {
		int strError = ERR_OK;
        switch( inputMap_Iter->second.type )
        {
        case GPS_SETTING_TYPE_INTEGER:
            err += TM_GetIntegerParameter(g_GPS_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (int*)inputMap_Iter->second.value);
            break;
        case GPS_SETTING_TYPE_DOUBLE:
            err += TM_GetDoubleParameter(g_GPS_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (double*)inputMap_Iter->second.value);
            break;
        case GPS_SETTING_TYPE_STRING:			
            stringParameter[0] = 0;
            strError = ::TM_GetStringParameter(g_GPS_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), stringParameter, MAX_BUFFER_SIZE);
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

GPS_TEST_API int  GetPacketLength(char* perfix, char* postfix, char* packetType, int *packetLength)
{
    int  err = ERR_OK;

	char keyword[MAX_BUFFER_SIZE] = {'\0'};

	map<string, GPS_SETTING_STRUCT>::iterator inputMap_Iter;
	

	// Find out the keyword of the waveform file name	
	sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_%s_%s", perfix, packetType, postfix);			

    // Search the keyword in g_GPSGlobalSettingParamMap
	inputMap_Iter = g_GPSGlobalSettingParamMap.find(keyword);
    if( inputMap_Iter!=g_GPSGlobalSettingParamMap.end() )
    {
		*packetLength = *((int*)inputMap_Iter->second.value);
	}
    else
    {
        err = TM_ERR_PARAM_DOES_NOT_EXIST;
    }

	return err;
}

GPS_TEST_API int GetGPSTestID(TM_ID* GPSTestID)
{
	*GPSTestID = g_GPS_Test_ID;
    return ERR_OK;
}

GPS_TEST_API int   GetGPSDutID(vDUT_ID* GPSDutID)
{
    *GPSDutID = g_GPS_Dut;
    return ERR_OK;
}

GPS_TEST_API int   GetGPSGlobalSetting(GPS_GLOBAL_SETTING** GPSGlobleSetting)
{
    *GPSGlobleSetting = &g_GPSGlobalSettingParam;
    return ERR_OK;
}

GPS_TEST_API int   SetTesterNumber(int testerNumber)
{
    g_Tester_Number = testerNumber;
    return ERR_OK;
}

GPS_TEST_API int   GetTesterNumber(int *testerNumber)
{
    *testerNumber = g_Tester_Number;
    return ERR_OK;
}

GPS_TEST_API int   SetTesterType(int testerType)
{
    g_Tester_Type = testerType;
    return ERR_OK;
}

GPS_TEST_API int   GetTesterType(int *testerType)
{
    *testerType = g_Tester_Type;
    return ERR_OK;
}

GPS_TEST_API int   SetTesterReconnect(int flag)
{
    g_Tester_Reconnect = flag;
    return ERR_OK;
}

GPS_TEST_API int   GetTesterReconnect(int *flag)
{
    *flag = g_Tester_Reconnect;
    return ERR_OK;
}

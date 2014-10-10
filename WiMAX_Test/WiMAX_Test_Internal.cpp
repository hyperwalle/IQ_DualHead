#include "stdafx.h"
#include "WIMAX_Test.h"
#include "IQlite_Logger.h"
#include "IQmeasure.h"
#include "math.h"

using namespace std;

// global variable 
TM_ID            g_WIMAX_Test_ID		= -1;
vDUT_ID          g_WIMAX_Dut			= -1;
bool             g_ReloadDutDll			= false;
bool			 g_vDutTxActived		= false;
bool			 g_vDutRxActived		= false;
bool			 g_dutConfigChanged		= false;
bool		     g_vsgContinuousTxMode  = false;
int				 g_Logger_ID			= -1;
int              g_WIMAX_Test_timer		= -1;
int				 g_Tester_Type			= IQ_MAX;
int				 g_Tester_Number		= 0;
int				 g_Tester_Reconnect		= 0;

const char      *g_WIMAX_Test_Version = "1.6.0 (2010-09-17)\n";
char			 g_defaultFilePath[MAX_BUFFER_SIZE] = {'\0'};

WIMAX_RECORD_PARAM g_RecordedParam;

// This global variable is declared in WIMAX_Global_Setting.cpp
// Input Parameter Container
extern map<string, WIMAX_SETTING_STRUCT> g_WIMAXGlobalSettingParamMap;
extern WIMAX_GLOBAL_SETTING g_WIMAXGlobalSettingParam;

// end global

using namespace std;


//! WIMAX Test Internal Functions
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

void InitializeAllContainers(void)
{
    InitializeConnectTesterContainers();            // Needed by WIMAX_Connect_IQTester
    InitializeDisconnectTesterContainers();         // Needed by WIMAX_Disconnect_IQTester
		
    InitializeInsertDutContainers();                // Needed by WIMAX_Insert_Dut
    InitializeInitializeDutContainers();            // Needed by WIMAX_Initialize_Dut
    InitializeRemoveDutContainers();                // Needed by WIMAX_Remove_Dut
	InitializeResetDutContainers();                 // Needed by WIMAX_Reset_Dut
	InitializetxrxVerificationContainers();			// Needed by WIMAX_TX_RX_Verification
    InitializeTXVerifyEvmContainers();              // Needed by WIMAX_TX_Verify_Evm
    InitializeTXVerifyMaskContainers();             // Needed by WIMAX_TX_Verify_Mask
    InitializeRXVerifyPerContainers();              // Needed by WIMAX_RX_Verify_PER
	InitializeTXVerifyPowerContainers();			// Needed by WIMAX_TX_Verify_Power_Step
	InitializeTXRXCalContainers();					// Needed by WIMAX_TXRX_Calibration

	InitializeInternalParameters();
	InitializeGlobalSettingContainers();            // Needed by WIMAX_Global_Setting
	InitializeloadPathLossTableContainers();

	InitializeReadMacAddressContainers();
	InitializeWriteMacAddressContainers();

	InitializeReadEepromContainers();
	InitializeWriteEepromContainers();
}

void CleanupAllWiMaxContainers(void)
{
	ClearConnectLPReturn();
	ClearDisconnectLPReturn();
	ClearGlobalSettingReturn();
	ClearInitializeDutReturn();
	ClearInsertDutReturn();
	ClearLoadPathLossTableReturn();
	ClearReadEepromReturn();
	ClearReadMacAddressReturn();
	ClearRemoveDutReturn();
	ClearResetDutReturn();
	ClearRxVerifyPerReturn();
	ClearTxVerifyEvmReturn();
	ClearTxVerifyMaskReturn();
	ClearTxVerifyPowerReturn();
	ClearTxRxCalReturn();
	ClearTxRxVerificationReturn();
	ClearWriteEepromReturn();
	ClearWriteMacAddressReturn();
}

WIMAX_TEST_API void InitializeInternalParameters(void)
{
	// Initial the internal parameters
	g_RecordedParam.ANT1 = (int)NA_NUMBER;
	g_RecordedParam.ANT2 = (int)NA_NUMBER;
	g_RecordedParam.ANT3 = (int)NA_NUMBER;
	g_RecordedParam.ANT4 = (int)NA_NUMBER;		
	g_RecordedParam.CABLE_LOSS_DB[0] = NA_NUMBER;
	g_RecordedParam.CABLE_LOSS_DB[1] = NA_NUMBER;
	g_RecordedParam.CABLE_LOSS_DB[2] = NA_NUMBER;
	g_RecordedParam.CABLE_LOSS_DB[3] = NA_NUMBER;	
	g_RecordedParam.BANDWIDTH_MHZ	 = NA_NUMBER;
	g_RecordedParam.CYCLIC_PREFIX	 = NA_NUMBER;
	g_RecordedParam.FRAME_COUNT		 = (int)NA_NUMBER;
	g_RecordedParam.FREQ_MHZ		 = (int)NA_NUMBER;
	g_RecordedParam.NUM_SYMBOLS		 = NA_NUMBER;
	g_RecordedParam.POWER_DBM		 = NA_NUMBER;
	g_RecordedParam.RATE_ID			 = (int)NA_NUMBER;
	g_RecordedParam.SIG_TYPE		 = (int)NA_NUMBER;

	g_RecordedParam.MAP_CONFIG_FILE_NAME[0]	 = '\0';
}

WIMAX_TEST_API int AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult)
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
		if ( logType==Linear )
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

WIMAX_TEST_API int  RespondToQueryInput( std::map<std::string, WIMAX_SETTING_STRUCT>& inputMap)
{
    int err = ERR_OK;

    map<string, WIMAX_SETTING_STRUCT>::iterator inputMap_Iter;

    int     *integerValue;
    double  *doubleValue;
    char    *stringValue;

    ::TM_ClearReturns(g_WIMAX_Test_ID);
    inputMap_Iter = inputMap.begin();
    while( inputMap_Iter!=inputMap.end() )
    {
        if( NULL!=inputMap_Iter->second.unit )
        {
            ::TM_AddUnitReturn(g_WIMAX_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (TM_STR)inputMap_Iter->second.unit);
        }
        if( NULL!=inputMap_Iter->second.helpText )
        {
            ::TM_AddHelpTextReturn(g_WIMAX_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (TM_STR)inputMap_Iter->second.helpText);
        }
        switch( inputMap_Iter->second.type )
        {
        case WIMAX_SETTING_TYPE_INTEGER:
            integerValue = reinterpret_cast<int *>(inputMap_Iter->second.value);
            ::TM_AddIntegerReturn(g_WIMAX_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), *integerValue);
            break;
        case WIMAX_SETTING_TYPE_DOUBLE:
            doubleValue = reinterpret_cast<double *>(inputMap_Iter->second.value);
            ::TM_AddDoubleReturn(g_WIMAX_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), *doubleValue);
            break;
        case WIMAX_SETTING_TYPE_STRING:
            stringValue = reinterpret_cast<char *>(inputMap_Iter->second.value);
            ::TM_AddStringReturn(g_WIMAX_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), stringValue);
            break;
        default:
            // Unknown type.  Ignore it.
            break;
        }
        inputMap_Iter++;
    }

    return err;
}


WIMAX_TEST_API int  RespondToQueryReturn( map<string, WIMAX_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, WIMAX_SETTING_STRUCT>::iterator returnMap_Iter;

    ::TM_ClearReturns(g_WIMAX_Test_ID);
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( NULL!=returnMap_Iter->second.unit )
        {
            ::TM_AddUnitReturn(g_WIMAX_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.unit);
        }
        if( NULL!=returnMap_Iter->second.helpText )
        {
            ::TM_AddHelpTextReturn(g_WIMAX_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.helpText);
        }
        switch( returnMap_Iter->second.type )
        {
        case WIMAX_SETTING_TYPE_INTEGER:
            ::TM_AddIntegerReturn(g_WIMAX_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), 0);
            break;
        case WIMAX_SETTING_TYPE_DOUBLE:
            ::TM_AddDoubleReturn(g_WIMAX_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), 0.0);
            break;
        case WIMAX_SETTING_TYPE_STRING:
            ::TM_AddStringReturn(g_WIMAX_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), "");
            break;
        case WIMAX_SETTING_TYPE_ARRAY_DOUBLE:
            ::TM_AddArrayDoubleReturn(g_WIMAX_Test_ID,(TM_STR)returnMap_Iter->first.c_str(),NULL, 0);
            break;
        default:
            // Unknown type.  Ignore it.
            break;
        }
        returnMap_Iter++;
    }

    return err;
}

WIMAX_TEST_API int  ReturnTestResults( map<string, WIMAX_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, WIMAX_SETTING_STRUCT>::iterator returnMap_Iter;

    ::TM_ClearReturns(g_WIMAX_Test_ID);
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( NULL!=returnMap_Iter->second.unit )
        {
            ::TM_AddUnitReturn(g_WIMAX_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.unit);
        }
        if( NULL!=returnMap_Iter->second.helpText )
        {
            ::TM_AddHelpTextReturn(g_WIMAX_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.helpText);
        }

        if( returnMap_Iter->second.type==WIMAX_SETTING_TYPE_INTEGER )
        {
            int *valueInteger = reinterpret_cast<int *>(returnMap_Iter->second.value);
            if ( (*valueInteger)>(NA_NUMBER+1) )
            {
                err = ::TM_AddIntegerReturn(g_WIMAX_Test_ID, (char*)returnMap_Iter->first.c_str(), *valueInteger);
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
        else if(returnMap_Iter->second.type==WIMAX_SETTING_TYPE_DOUBLE)
        {
            double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            if ( (*valueDouble)>(NA_NUMBER+1) )
            {
                err = ::TM_AddDoubleReturn(g_WIMAX_Test_ID, (char*)returnMap_Iter->first.c_str(), *valueDouble);
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
        else  if(returnMap_Iter->second.type==WIMAX_SETTING_TYPE_STRING) 
        {   
            char *valueString = reinterpret_cast<char *>(returnMap_Iter->second.value);
            err = ::TM_AddStringReturn(g_WIMAX_Test_ID, (char*)returnMap_Iter->first.c_str(), valueString);
            if (TM_ERR_OK!=err)    
            {
                // If TM_AddStringReturn fails, no need to continue
                break;
            }
        }
        else //returnMap_Iter->second.type==WIMAX_SETTING_TYPE_ARRAY_DOUBLE
        {
            vector<double> doubleVector = * reinterpret_cast<vector<double> *>(returnMap_Iter->second.value);
            if (0 != doubleVector.size())
            {
                double *doubleArray = new double[(int) doubleVector.size()];
                for (unsigned int i=0; i<doubleVector.size(); i++)
                {
                    doubleArray[i] = doubleVector[i];
                }
                err = TM_AddArrayDoubleReturn(g_WIMAX_Test_ID, (char*)returnMap_Iter->first.c_str(), doubleArray, (int)doubleVector.size());

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

WIMAX_TEST_API int  ClearReturnParameters( map<string, WIMAX_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, WIMAX_SETTING_STRUCT>::iterator returnMap_Iter;

	// clear container
    ::TM_ClearReturns(g_WIMAX_Test_ID);
	
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( returnMap_Iter->second.type==WIMAX_SETTING_TYPE_INTEGER )
        {
			int *valueInteger = reinterpret_cast<int *>(returnMap_Iter->second.value);
			*valueInteger = (int)NA_NUMBER;
        }
        else if(returnMap_Iter->second.type==WIMAX_SETTING_TYPE_DOUBLE)
        {
			double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            *valueDouble = NA_NUMBER;
        }
        else    // returnMap_Iter->second.type==WIMAX_SETTING_TYPE_STRING
        {   
			char *valueString = reinterpret_cast<char *>(returnMap_Iter->second.value);
			*valueString = '\0';

        }
        returnMap_Iter++;
    }

    return err;
}

WIMAX_TEST_API int  ReturnErrorMessage(char *errParameter, const char *format, ...)
{
    int err = ERR_OK;

    // Log message format ... variable argument list
    va_list ap;
    va_start(ap, format);

    vsprintf_s(errParameter, MAX_BUFFER_SIZE, format, ap);
    err = ::TM_AddStringReturn(g_WIMAX_Test_ID, "ERROR_MESSAGE", errParameter);
	// Ignore error handling

	err = ::vDUT_Run( g_WIMAX_Dut, "FUNCTION_FAILED" );
	// Ignore error handling

    va_end(ap);

    return err;
}

WIMAX_TEST_API int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...)
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
		sprintf_s(errMsg, sizeOfBuf, "[WIMAX] LogReturnMessage() return error, Logger_ID < 0.\n");
	}

    return err;
}

WIMAX_TEST_API int  GetInputParameters( map<string, WIMAX_SETTING_STRUCT>& inputMap)
{
    int     err = ERR_OK;
	char    stringParameter[MAX_BUFFER_SIZE] = {'\0'};

    map<string, WIMAX_SETTING_STRUCT>::iterator inputMap_Iter;

    inputMap_Iter = inputMap.begin();
    while( inputMap_Iter!=inputMap.end() )
    {
        switch( inputMap_Iter->second.type )
        {
        case WIMAX_SETTING_TYPE_INTEGER:
            err += ::TM_GetIntegerParameter(g_WIMAX_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (int*)inputMap_Iter->second.value);
            break;
        case WIMAX_SETTING_TYPE_DOUBLE:
            err += ::TM_GetDoubleParameter(g_WIMAX_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (double*)inputMap_Iter->second.value);
            break;
        case WIMAX_SETTING_TYPE_STRING:
            stringParameter[0] = 0;
            err += ::TM_GetStringParameter(g_WIMAX_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), stringParameter, MAX_BUFFER_SIZE);
            if(0!=stringParameter[0])
            {
                strcpy_s((char*)inputMap_Iter->second.value, MAX_BUFFER_SIZE, stringParameter);
            }
			else
			{
				//strcpy_s((char*)inputMap_Iter->second.value, MAX_BUFFER_SIZE, "");
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


WIMAX_TEST_API int  CheckDutTransmitStatus(void)
{
    int err = ERR_OK;
	int	ant01 = 0, ant02 = 0, ant03 = 0, ant04 = 0;

	::TM_GetIntegerParameter(g_WIMAX_Test_ID, "TX1", &ant01);
	::TM_GetIntegerParameter(g_WIMAX_Test_ID, "TX2", &ant02);
	::TM_GetIntegerParameter(g_WIMAX_Test_ID, "TX3", &ant03);
	::TM_GetIntegerParameter(g_WIMAX_Test_ID, "TX4", &ant04);

	if ( (ant01==1)||(ant02==1)||(ant03==1)||(ant04==1) )	// This is a Tx function
	{
		    // do nothing
	}
	else	// This is NOT a Tx function
	{
		if ( g_vDutTxActived )		// If Dut still under Tx mode
		{
			   /*-----------*
				*  Tx Stop  *
				*-----------*/
				err = ::vDUT_Run(g_WIMAX_Test_ID, "TX_STOP");		
				if ( ERR_OK!=err )
				{	
					err = -1;	// vDUT_Run(TX_STOP) return error.
				}
				else
				{
					g_vDutTxActived = false;
				}
		}
		else
		{
			// do nothing
		}
	}

    return err;
}

WIMAX_TEST_API int  WIMAXSaveSigFile(char* fileName)
{
    int    err = ERR_OK;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   logInfoMessage[MAX_BUFFER_SIZE] = {'\0'};

	if (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] The VSA_SAVE_CAPTURE_ALWAYS is set to ON, it may slow down the test time.\n");
	}

	if ( (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED)||(1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS))  
	{	  
		// Get system time
		SYSTEMTIME   sysTime;  
		char c_time[MAX_BUFFER_SIZE], c_path[MAX_BUFFER_SIZE];

		::GetLocalTime(&sysTime);   

		sprintf_s(c_time, "%d.%d.%d-%d.%d.%d.%d", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);		    
		sprintf_s(c_path, MAX_BUFFER_SIZE, ".\\log\\%s-%s.sig", fileName, c_time);

		err = ::LP_SaveVsaSignalFile(c_path);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WIMAX] LP_SaveVsaSignalFile(\"%s\") return error.\n", c_path);
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WIMAX] Save Signal File (\"%s\").\n", c_path );
		}
	}
	else
	{
		// do nothing...
	}

	return err;
}

WIMAX_TEST_API int  GetWaveformFileName(char* perfix, char* postfix, int WIMAXMode, char* bandwidth, char* datarate, char* preamble, char* packetFormat11N, char* waveformFileName, int bufferSize)
{
    int  err = ERR_OK;


	return err;
}

WIMAX_TEST_API int  GetPacketNumber(int WIMAXMode, char* bandwidth, char* datarate, char* packetFormat11N, int *packetNumber)
{
    int  err = ERR_OK;


	return err;
}

// This function is for backward compatible
WIMAX_TEST_API int CheckPathLossTable(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb)
{
	int  err = ERR_OK;

	err = CheckPathLossTableExt(testID, freqMHz, ant01, ant02, ant03, ant04, cableLoss, cableLossReturn, cableLossDb, TX_TABLE);

	return err;
}

WIMAX_TEST_API int CheckPathLossTableExt(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb, int indicatorTxRx)
{
	int  err = ERR_OK;

	try
	{
		int ant[4] = { ant01, ant02, ant03, ant04 };
		
		for ( int i=0;i<MAX_DATA_STREAM;i++)
		{
			if ( ant[i] )	// Path enabled 
			{
				if ( 0==cableLoss[i] )	// 0: means read path loss from TM_TABLE
				{
					err = TM_GetPathLossAtFrequency(testID, freqMHz, &cableLoss[i], i, indicatorTxRx);
					if (err!=ERR_OK)
					{
						cableLoss[i]	   = NA_NUMBER;
						cableLossReturn[i] = NA_NUMBER;
						throw (i+1);
					}
					else
					{
						cableLossReturn[i] = cableLoss[i];
					}
				}
				else	// using user_defined path loss value
				{
					cableLossReturn[i] = cableLoss[i];				
				}			
			}
			else	// This path not enabled
			{
				cableLoss[i]	   = NA_NUMBER;
				cableLossReturn[i] = NA_NUMBER;
			}
		}

		*cableLossDb = CalcCableLossDb( ant01, ant02, ant03, ant04, cableLoss[0], cableLoss[1], cableLoss[2], cableLoss[3]);
	}
	catch(int errReturn)
    {
        return errReturn;
    }
    catch(...)
    {
		return -1;
    }

	return err;
}

WIMAX_TEST_API double CalcCableLossDb(int ant1, int ant2, int ant3, int ant4, double cableLoss1, double cableLoss2, double cableLoss3, double cableLoss4)
{
	double resultDb = 0.0;

	int antNumber = ant1 + ant2 + ant3 + ant4;
	
	if (ant1==1)	// antenna 1 is ON
	{
		resultDb = resultDb + pow( 10, (cableLoss1/10) );
	}
	if (ant2==1)	// antenna 2 is ON
	{
		resultDb = resultDb + pow( 10, (cableLoss2/10) );
	}
	if (ant3==1)	// antenna 3 is ON
	{
		resultDb = resultDb + pow( 10, (cableLoss3/10) );
	}
	if (ant4==1)	// antenna 4 is ON
	{
		resultDb = resultDb + pow( 10, (cableLoss4/10) );
	}

	if ( (antNumber==0)||(resultDb==0) )
	{
		resultDb = 0.0;
	}
	else
	{
		resultDb = 10 * log10(resultDb/antNumber);
	}

	return resultDb;
}

WIMAX_TEST_API int CheckAntennaOrderByStream(int streamOrder, int ant1, int ant2, int ant3, int ant4, int *antOrder)
{
    int err = ERR_OK;

	int count = ant1;

	if (streamOrder==count)
	{
		*antOrder = 1;
		return err;
	}
	else
	{
		// do nothing
	}

	count = count + ant2;
	if (streamOrder==count)
	{
		*antOrder = 2;
		return err;
	}
	else
	{
		// do nothing
	}

	count = count + ant3;
	if (streamOrder==count)
	{
		*antOrder = 3;
		return err;
	}
	else
	{
		// do nothing
	}

	count = count + ant4;
	if (streamOrder==count)
	{
		*antOrder = 4;
		return err;
	}
	else
	{
		// Now, only support 4 stream and 4 antenna
		// If go into this case, means unknown error.
		err = 1;
	}

    return err;
}

WIMAX_TEST_API int GetWIMAXTestID(TM_ID* WIMAXTestID)
{
	*WIMAXTestID = g_WIMAX_Test_ID;
    return ERR_OK;
}

WIMAX_TEST_API int   GetWIMAXDutID(vDUT_ID* WIMAXDutID)
{
    *WIMAXDutID = g_WIMAX_Dut;
    return ERR_OK;
}

WIMAX_TEST_API int   GetWIMAXGlobalSetting(WIMAX_GLOBAL_SETTING** WIMAXGlobleSetting)
{
    *WIMAXGlobleSetting = &g_WIMAXGlobalSettingParam;
    return ERR_OK;
}

WIMAX_TEST_API int   SetTesterNumber(int testerNumber)
{
    g_Tester_Number = testerNumber;
    return ERR_OK;
}

WIMAX_TEST_API int   GetTesterNumber(int *testerNumber)
{
    *testerNumber = g_Tester_Number;
    return ERR_OK;
}

WIMAX_TEST_API int   SetTesterType(int testerType)
{
    g_Tester_Type = testerType;
    return ERR_OK;
}

WIMAX_TEST_API int   GetTesterType(int *testerType)
{
    *testerType = g_Tester_Type;
    return ERR_OK;
}

WIMAX_TEST_API int   SetTesterReconnect(int flag)
{
    g_Tester_Reconnect = flag;
    return ERR_OK;
}

WIMAX_TEST_API int   GetTesterReconnect(int *flag)
{
    *flag = g_Tester_Reconnect;
    return ERR_OK;
}

WIMAX_TEST_API int   SetDutConfigChanged(bool changed)
{
    g_dutConfigChanged = changed;
    return ERR_OK;
}

WIMAX_TEST_API int   GetDutConfigChanged(bool *changed)
{
	*changed = g_dutConfigChanged;
	return ERR_OK;
}

WIMAX_TEST_API int GetDutTxActived(bool* vDutTxActived)
{
	*vDutTxActived = g_vDutTxActived;
    return ERR_OK;
}

WIMAX_TEST_API int SetDutTxActived(bool vDutTxActived)
{
	g_vDutTxActived = vDutTxActived;
    return ERR_OK;
}

WIMAX_TEST_API int   GetVsgContinuousTxMode(bool* vsgContinuousTxMode)
{
	*vsgContinuousTxMode = g_vsgContinuousTxMode;
    return ERR_OK;
}

WIMAX_TEST_API int   SetVsgContinuousTxMode(bool* vsgContinuousTxMode)
{
	g_vsgContinuousTxMode = *vsgContinuousTxMode;
    return ERR_OK;
}

WIMAX_TEST_API int   GetDefaultFilePath(char* defaultFilePath, int sizeOfBuf)
{
	sprintf_s(defaultFilePath, sizeOfBuf, g_defaultFilePath);
    return ERR_OK;
}

WIMAX_TEST_API int   SetDefaultFilePath(char* defaultFilePath)
{
	sprintf_s(g_defaultFilePath, MAX_BUFFER_SIZE, defaultFilePath);
    return ERR_OK;
}

WIMAX_TEST_API int  AnalyzeCWFrequency(double centerFreqMHz, double vsaAmpPowerDbm, double samplingTimeUs, int avgTimes, double *cwFreqResultHz, char *errorMsg)
{
    int  err = ERR_OK;

	int    avgIteration = 0;
	double cwFreqHz = 0.0;
	double cwFreqHz_sum = 0.0;
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};

	/*--------------------*
	 * Setup IQtester VSA *
	 *--------------------*/
	err = LP_SetVsa(  (centerFreqMHz+WIMAX_FREQ_SHIFT_FOR_CW_MHZ)*1e6,
					   vsaAmpPowerDbm,
					   g_WIMAXGlobalSettingParam.VSA_PORT,
					   0,
					   g_WIMAXGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
					   g_WIMAXGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
					 );
    if (ERR_OK!=err)
    {
        sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] Fail to setup VSA.\n");
        return err;
    }

   /*----------------------------*
	* Disable VSG output signal  *
	*----------------------------*/
	//// make sure no signal is generated by the VSG
	//err = ::LP_EnableVsgRF(0);
	//if ( ERR_OK!=err )
	//{
	//	sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
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
			sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] LP_VsaDataCapture() at %.1f MHz return OK.\n", (centerFreqMHz+WIMAX_FREQ_SHIFT_FOR_CW_MHZ));

			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			if (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_CW_FREQ_SaveAlways", (centerFreqMHz+WIMAX_FREQ_SHIFT_FOR_CW_MHZ));
				WIMAXSaveSigFile(sigFileNameBuffer);
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
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_CW_FREQ_AnalysisFailed", (centerFreqMHz+WIMAX_FREQ_SHIFT_FOR_CW_MHZ));
				WIMAXSaveSigFile(sigFileNameBuffer);				
				sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] Fail to analyze signal at %.1f MHz.\n", (centerFreqMHz+WIMAX_FREQ_SHIFT_FOR_CW_MHZ));
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
				cwFreqHz = cwFreqHz + WIMAX_FREQ_SHIFT_FOR_CW_MHZ*1e6;
				cwFreqHz_sum = cwFreqHz_sum + cwFreqHz;				
			}

        }
        else
        {   
            // Fail Capture
			err = -1;
            sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] Fail to capture signal at %.1f MHz.\n", (centerFreqMHz+WIMAX_FREQ_SHIFT_FOR_CW_MHZ));
            break;
        }
    }

   /*-----------------------*
    *  Return Test Results  *
    *-----------------------*/
    if( avgIteration && (err==ERR_OK) )
    {
        *cwFreqResultHz = cwFreqHz_sum/avgIteration;
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WIMAX] CW frequency analysis OK.\n");
    }
    else
    {
        *cwFreqResultHz = NA_NUMBER;
    }	

	return err;
}

WIMAX_TEST_API int QuickCaptureRetry(double centerFreqMHz, double samplingTimeUS, int triggerType, double *PowerDbm, char *errorMsg)
{
    int  err = ERR_OK;
	char sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};

	double peakPowerDbm = NA_NUMBER;
	double avgPowerDbm  = NA_NUMBER;


	LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiMAX] First capture failed, start doing the capture retry.\n");

   /*----------------------------*
	* Perform normal VSA capture *
	*----------------------------*/
    err = LP_VsaDataCapture(0.01, IQV_TRIG_TYPE_FREE_RUN);              
    if( ERR_OK==err )	// capture is ok
    {
		LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiMAX] (Retry) LP_VsaDataCapture(IQV_TRIG_TYPE_FREE_RUN) at %.1f MHz return OK.\n", centerFreqMHz);

		/*--------------*
		 *  Capture OK  *
		 *--------------*/
		//if (1==g_WIMAXGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
		{
			// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_SaveAlways", centerFreqMHz);
			WIMAXSaveSigFile(sigFileNameBuffer);
		}
		//else
		{
			// do nothing
		}

		/*------------------*
		 *  Power Analysis  *
		 *------------------*/
		err = ::LP_AnalyzePower( );
		if ( ERR_OK!=err )
		{
			// Fail Analysis, thus save capture (Signal File) for debug
			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Analysis_Failed", centerFreqMHz);
			WIMAXSaveSigFile(sigFileNameBuffer);
			LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiMAX] (Retry) LP_AnalyzePower() return error and save the sig file %s.\n", sigFileNameBuffer);
			return err;
		}
		else		// Analyze is ok
		{
		   /*-----------------------------*
		    *  Retrieve analysis Results  *
		    *-----------------------------*/
			peakPowerDbm  = ::LP_GetScalarMeasurement("P_peak_all_dBm", 0);
			if ( -99.00 >= (peakPowerDbm)  )
			{
				peakPowerDbm = NA_NUMBER;				
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Result_Failed", centerFreqMHz);
				WIMAXSaveSigFile(sigFileNameBuffer);
				err = -1;				
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiMAX] (Retry) LP_GetScalarMeasurement(P_peak_all_dBm) return error and save the sig file %s.\n", sigFileNameBuffer);
				return err;
			}
			else
			{			
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiMAX] (Retry) Quick power analysis OK, (without path loss) P_peak_all_dBm = %.1f dBm.\n", peakPowerDbm);
			}

			avgPowerDbm  = ::LP_GetScalarMeasurement("P_av_no_gap_all_dBm", 0);
			if ( -99.00 >= (avgPowerDbm)  )
			{
				avgPowerDbm = NA_NUMBER;				
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WIMAX_Quick_Power_Result_Failed", centerFreqMHz);
				WIMAXSaveSigFile(sigFileNameBuffer);
				err = -1;				
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiMAX] (Retry) LP_GetScalarMeasurement(P_av_no_gap_all_dBm) return error and save the sig file %s.\n", sigFileNameBuffer);
				return err;
			}
			else
			{	
				*PowerDbm = avgPowerDbm;
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiMAX] (Retry) Quick power analysis OK, (without path loss) P_av_no_gap_all_dBm = %.1f dBm.\n", avgPowerDbm);
			}
		}
    }
    else
    {   
        // Fail Capture
		err = -1;
        LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiMAX] (Retry) Fail to capture signal at %.1f MHz.\n", centerFreqMHz);
        return err;
    }

   /*-----------------*
    *  Retry Capture  *
    *-----------------*/
	err = LP_SetVsa(   centerFreqMHz*1e6,
					   peakPowerDbm+2,
					   g_WIMAXGlobalSettingParam.VSA_PORT,
					   g_WIMAXGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
					   g_WIMAXGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
					 );
    if (ERR_OK!=err)
    {
        LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiMAX] (Retry) Fail to setup VSA.\n");
        return err;
    }
	else
	{
		LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiMAX] (Retry) LP_SetVsa() with peak PowerLevel = %.1f +2 return OK.\n", peakPowerDbm);
	}

	// Perform Normal VSA capture 
	//err = ::LP_VsaDataCapture( samplingTimeUS/1000000, triggerType );
	err = LP_VsaDataCapture(0.01, IQV_TRIG_TYPE_FREE_RUN); 
	if (ERR_OK!=err)
	{
		LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiMAX] (Retry) Fail to capture WIMAX signal at %.1f MHz with VSA TriggerType = %d.\n", centerFreqMHz, triggerType);
		return err;
	}
	else
	{
		LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiMAX] (Retry) LP_VsaDataCapture(TriggerType=%d) at %.1f MHz return OK.\n", triggerType, centerFreqMHz);
	}
	

	return err;
}

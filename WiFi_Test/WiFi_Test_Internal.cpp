#include "stdafx.h"
#include "WiFi_Test.h"
#include "IQlite_Logger.h"
#include "IQmeasure.h"
#include "math.h"
//Move to stdafx.h
//#include "lp_time.h"

using namespace std;

// global variable 
TM_ID            g_WiFi_Test_ID		= -1;
vDUT_ID          g_WiFi_Dut			= -1;

bool             g_ReloadDutDll		= false;
bool			 g_vDutTxActived	= false;
bool			 g_vDutRxActived	= false;
bool			 g_dutConfigChanged = false;
bool			 g_dutRxConfigChanged  = false;
bool			 g_vDutControlDisabled = false;
bool			 g_useLpDut			= false;	// -cfy@sunnyvale, 2012/3/13-
int				 g_Logger_ID        = -1;
int              g_WiFi_Test_timer  = -1;
int				 g_Tester_Type      = IQ_View;
int				 g_Tester_Number    = 0;
int				 g_Tester_Reconnect = 0;


double			 g_last_TxPower_dBm_Record = 0; // Record the Tx power used in last test item. // -cfy@sunnyvale, 2012/3/13-

const char      *g_WiFi_Test_Version = "1.6.0 (2010-09-17)\n";
char			 g_defaultFilePath[MAX_BUFFER_SIZE] = {'\0'};

WIFI_RECORD_PARAM g_RecordedParam;
WIFI_RECORD_PARAM g_RecordedRxParam;

// end global

//using namespace std;


//! WiFi Test Internal Functions
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
    InitializeConnectTesterContainers();            // Needed by WiFi_Connect_IQTester
    InitializeDisconnectTesterContainers();         // Needed by WiFi_Disconnect_IQTester
    InitializeInsertDutContainers();                // Needed by WiFi_Insert_Dut
    InitializeInitializeDutContainers();            // Needed by WiFi_Initialize_Dut
    InitializeRemoveDutContainers();                // Needed by WiFi_Remove_Dut
	InitializeTXMultiVerificationContainers();		// Needed by WiFi_TX_Multi_Verification
    InitializeTXVerifyEvmContainers();              // Needed by WiFi_TX_Verify_Evm
    InitializeRXVerifyPerContainers();              // Needed by WiFi_RX_Verify_Per
    InitializeSweepPerContainers();					// Needed by WiFi_RX_Sweep_Per
    InitializeTXVerifyMaskContainers();             // Needed by WiFi_TX_Verify_Mask
    InitializeTXVerifyPowerContainers();            // Needed by WiFi_TX_Verify_Power
    InitializeGlobalSettingContainers();            // Needed by WiFi_Global_Setting
    InitializeTXVerifyFlatnessContainers();         // Needed by WiFi_TX_Verify_Flatness
    InitializeReadEepromContainers();               // Needed by WiFi_Read_Eeprom
    InitializeWriteEepromContainers();              // Needed by WiFi_Write_Eeprom
	InitializeWriteBBRegisterContainers();          // Needed by WiFi_Write_BB_Register.cpp
	InitializeReadBBRegisterContainers();           // Needed by WiFi_Read_BB_Register.cpp
	InitializeWriteRFRegisterContainers();          // Needed by WiFi_Write_RF_Register.cpp
	InitializeReadRFRegisterContainers();           // Needed by WiFi_Read_RF_Register.cpp
	InitializeWriteMacRegisterContainers();         // Needed by WiFi_Write_Mac_Register.cpp
	InitializeReadMacRegisterContainers();          // Needed by WiFi_Read_Mac_Register.cpp
	InitializeWriteSocRegisterContainers();         // Needed by WiFi_Write_Soc_Register.cpp
	InitializeReadSocRegisterContainers();          // Needed by WiFi_Read_Soc_Register.cpp
    InitializeRunExternalProgramContainers();       // Needed by WiFi_Run_External_Program
    InitializeGetSerialNumberContainers();          // Needed by WiFi_Get_Serial_Number
    InitializeWriteMacAddressContainers();          // Needed by WiFi_Write_Mac_Address
    InitializeReadMacAddressContainers();           // Needed by WiFi_Read_Mac_Address
    InitializeTXCalContainers();                    // Needed by WiFi_TX_Calibration
	InitializeRXCalContainers();                    // Needed by WiFi_RX_Calibration
	InitializePowerModeDutContainers();				// Needed by WiFi Current test
	InitializeRunDutCommandContainers();            // Needed by WiFi_Run_Dut_Command // -cfy@sunnyvale, 2012/3/13-

	InitializeloadPathLossTableContainers();
	InitializeloadMultiSegmentWaveformContainers();
	InitializeInternalTxParameters();	
	InitializeInternalRxParameters();
	InitializeTemplateContainers();
	InitializeDutDisabledContainers();
	InitializeDutEnabledContainers();
	InitializeFinalizeEepromContainers();

	return TRUE;
}

void CleanupAllWiFiContainers(void)
{
	ClearConnectLPReturn();
	ClearDisconnectLPReturn();
	ClearGetSerialNumberReturn();
	ClearGlobalSettingReturn();
	ClearGetSerialNumberReturn();
	ClearInitializeDutReturn();
	ClearInsertDutReturn();
	ClearLoadMultiSegmentWaveformReturn();
	ClearLoadPathLossTableReturn();
	ClearCurrentTestReturn();
	ClearReadBBRegisterReturn();
	ClearReadEepromReturn();
	ClearReadMacAddressReturn();
	ClearReadMacRegisterReturn();
	ClearReadRFRegisterReturn();
	ClearReadSocRegisterReturn();
	ClearRemoveDutReturn();
	ClearRunExternalProgramReturn();
	ClearRxSweepPerReturn();
	ClearRxPerReturn();
	ClearTemplateReturn();
	ClearTxCalReturn();
	ClearTxMultiVerificationReturn();
	ClearTxEvmReturn();
	ClearTxPowerReturn();
	ClearTxMaskReturn();
	ClearTxSpectrumReturn();
	ClearWriteBBRegisterReturn();
	ClearWriteEepromReturn();
	ClearWriteMacAddressReturn();
	ClearWriteMacRegisterReturn();
	ClearWriteRFRegisterReturn();
	ClearWriteSocRegisterReturn();
	ClearDutDisabledReturn();
	ClearFinalizeEepromReturn();
}

WIFI_TEST_API void InitializeInternalTxParameters(void)
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
	g_RecordedParam.FREQ_MHZ         = (int)NA_NUMBER;	
	g_RecordedParam.POWER_DBM		 = NA_NUMBER;		
	g_RecordedParam.FRAME_COUNT      = (int)NA_NUMBER;	
	g_RecordedParam.BANDWIDTH[0]	 = '\0';
	g_RecordedParam.DATA_RATE[0]     = '\0';
	g_RecordedParam.PREAMBLE[0]      = '\0';
	g_RecordedParam.PACKET_FORMAT_11N[0] = '\0';
	g_RecordedParam.GUARD_INTERVAL_11N[0] = '\0';
}

WIFI_TEST_API void InitializeInternalRxParameters(void)
{
	// Initial the internal parameters
	g_RecordedRxParam.ANT1 = (int)NA_NUMBER;
	g_RecordedRxParam.ANT2 = (int)NA_NUMBER;
	g_RecordedRxParam.ANT3 = (int)NA_NUMBER;
	g_RecordedRxParam.ANT4 = (int)NA_NUMBER;		
	g_RecordedRxParam.CABLE_LOSS_DB[0] = NA_NUMBER;
	g_RecordedRxParam.CABLE_LOSS_DB[1] = NA_NUMBER;
	g_RecordedRxParam.CABLE_LOSS_DB[2] = NA_NUMBER;
	g_RecordedRxParam.CABLE_LOSS_DB[3] = NA_NUMBER;	
	g_RecordedRxParam.FREQ_MHZ         = (int)NA_NUMBER;	
	g_RecordedRxParam.POWER_DBM		   = NA_NUMBER;		
	g_RecordedRxParam.FRAME_COUNT      = (int)NA_NUMBER;	
	g_RecordedRxParam.BANDWIDTH[0]	   = '\0';
	g_RecordedRxParam.DATA_RATE[0]     = '\0';
	g_RecordedRxParam.PREAMBLE[0]      = '\0';
	g_RecordedRxParam.PACKET_FORMAT_11N[0] = '\0';
	g_RecordedRxParam.GUARD_INTERVAL_11N[0] = '\0';
}

WIFI_TEST_API int AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult)
{
    int err = ERR_OK;
    // Averaging after all measurements are done, by an averaging function
    int i = 0;

    averageResult = NA_NUMBER;
    maxResult = minResult = resultArray[0];

	if (0==averageTimes)
	{
		err = -1;	// Error, can not divided by zero
	}
	else
	{
		if ( logType==Linear )
		{
			averageResult = 0.0;
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
				averageResult = 0.0;
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
				averageResult = 0.0;
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

WIFI_TEST_API int  RespondToQueryInput( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap)
{
    int err = ERR_OK;

    map<string, WIFI_SETTING_STRUCT>::iterator inputMap_Iter;

    int     *integerValue = NULL;
    double  *doubleValue  = NULL;
    char    *stringValue  = NULL;
	char    logMessage[MAX_BUFFER_SIZE] = {'\0'};

    ::TM_ClearReturns(g_WiFi_Test_ID);
    inputMap_Iter = inputMap.begin();
    while( inputMap_Iter!=inputMap.end() )
    {
        if( NULL!=inputMap_Iter->second.unit )
        {
            ::TM_AddUnitReturn(g_WiFi_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (TM_STR)inputMap_Iter->second.unit);
        }
        if( NULL!=inputMap_Iter->second.helpText )
        {
            ::TM_AddHelpTextReturn(g_WiFi_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (TM_STR)inputMap_Iter->second.helpText);
        }
        switch( inputMap_Iter->second.type )
        {
        case WIFI_SETTING_TYPE_INTEGER:
            integerValue = reinterpret_cast<int *>(inputMap_Iter->second.value);
            ::TM_AddIntegerReturn(g_WiFi_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), *integerValue);
            break;
        case WIFI_SETTING_TYPE_DOUBLE:
            doubleValue = reinterpret_cast<double *>(inputMap_Iter->second.value);
            ::TM_AddDoubleReturn(g_WiFi_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), *doubleValue);
            break;
        case WIFI_SETTING_TYPE_STRING:
            stringValue = reinterpret_cast<char *>(inputMap_Iter->second.value);
            ::TM_AddStringReturn(g_WiFi_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), stringValue);
            break;
        default:
            // Unknown type.  Ignore it.
            break;
        }
        inputMap_Iter++;
    }

	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] RespondToQueryInput() return OK.\n");

    return err;
}


WIFI_TEST_API int  RespondToQueryReturn( map<string, WIFI_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, WIFI_SETTING_STRUCT>::iterator returnMap_Iter;

	char    logMessage[MAX_BUFFER_SIZE] = {'\0'};

    ::TM_ClearReturns(g_WiFi_Test_ID);
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( NULL!=returnMap_Iter->second.unit )
        {
            ::TM_AddUnitReturn(g_WiFi_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.unit);
        }
        if( NULL!=returnMap_Iter->second.helpText )
        {
            ::TM_AddHelpTextReturn(g_WiFi_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.helpText);
        }
        switch( returnMap_Iter->second.type )
        {
        case WIFI_SETTING_TYPE_INTEGER:
            ::TM_AddIntegerReturn(g_WiFi_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), 0);
            break;
        case WIFI_SETTING_TYPE_DOUBLE:
            ::TM_AddDoubleReturn(g_WiFi_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), 0.0);
            break;
        case WIFI_SETTING_TYPE_STRING:
            ::TM_AddStringReturn(g_WiFi_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), "");
            break;
        case WIFI_SETTING_TYPE_ARRAY_DOUBLE:
            ::TM_AddArrayDoubleReturn(g_WiFi_Test_ID,(TM_STR)returnMap_Iter->first.c_str(),NULL, 0);
            break;
        default:
            // Unknown type.  Ignore it.
            break;
        }
        returnMap_Iter++;
    }

	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] RespondToQueryReturn() return OK.\n");

    return err;
}

WIFI_TEST_API int  ReturnTestResults( map<string, WIFI_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, WIFI_SETTING_STRUCT>::iterator returnMap_Iter;

    ::TM_ClearReturns(g_WiFi_Test_ID);
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( NULL!=returnMap_Iter->second.unit )
        {
            ::TM_AddUnitReturn(g_WiFi_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.unit);
        }
        if( NULL!=returnMap_Iter->second.helpText )
        {
            ::TM_AddHelpTextReturn(g_WiFi_Test_ID, (TM_STR)returnMap_Iter->first.c_str(), (TM_STR)returnMap_Iter->second.helpText);
        }

        if( returnMap_Iter->second.type==WIFI_SETTING_TYPE_INTEGER )
        {
            int *valueInteger = reinterpret_cast<int *>(returnMap_Iter->second.value);
            if ( (*valueInteger)>(NA_NUMBER+1) )
            {
                err = ::TM_AddIntegerReturn(g_WiFi_Test_ID, (char*)returnMap_Iter->first.c_str(), *valueInteger);
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
        else if(returnMap_Iter->second.type==WIFI_SETTING_TYPE_DOUBLE)
        {
            double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            if ( (*valueDouble)>(NA_NUMBER+1) )
            {
                err = ::TM_AddDoubleReturn(g_WiFi_Test_ID, (char*)returnMap_Iter->first.c_str(), *valueDouble);
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
        else  if(returnMap_Iter->second.type==WIFI_SETTING_TYPE_STRING) 
        {   
            char *valueString = reinterpret_cast<char *>(returnMap_Iter->second.value);
            err = ::TM_AddStringReturn(g_WiFi_Test_ID, (char*)returnMap_Iter->first.c_str(), valueString);
            if (TM_ERR_OK!=err)    
            {
                // If TM_AddStringReturn fails, no need to continue
                break;
            }
        }
        else //returnMap_Iter->second.type==WIFI_SETTING_TYPE_ARRAY_DOUBLE
        {
            vector<double> doubleVector = * reinterpret_cast<vector<double> *>(returnMap_Iter->second.value);
            if (0 != doubleVector.size())
            {
                double *doubleArray = new double[(int) doubleVector.size()];
                for (unsigned int i=0; i<doubleVector.size(); i++)
                {
                    doubleArray[i] = doubleVector[i];
                }
                err = TM_AddArrayDoubleReturn(g_WiFi_Test_ID, (char*)returnMap_Iter->first.c_str(), doubleArray, (int)doubleVector.size());

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

			doubleVector.clear();        
        }
        returnMap_Iter++;
    }

    return err;
}

WIFI_TEST_API int  ClearReturnParameters( map<string, WIFI_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, WIFI_SETTING_STRUCT>::iterator returnMap_Iter;

	// clear container
    ::TM_ClearReturns(g_WiFi_Test_ID);
	
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter!=returnMap.end() )
    {
        if( returnMap_Iter->second.type==WIFI_SETTING_TYPE_INTEGER )
        {
			int *valueInteger = reinterpret_cast<int *>(returnMap_Iter->second.value);
			*valueInteger = (int)NA_NUMBER;
        }
        else if(returnMap_Iter->second.type==WIFI_SETTING_TYPE_DOUBLE)
        {
			double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            *valueDouble = NA_NUMBER;
        }
        else    // returnMap_Iter->second.type==WIFI_SETTING_TYPE_STRING
        {   
			char *valueString = reinterpret_cast<char *>(returnMap_Iter->second.value);
			*valueString = '\0';

        }
        returnMap_Iter++;
    }

    return err;
}

WIFI_TEST_API int  ReturnErrorMessage(char *errParameter, const char *format, ...)
{
    int err = ERR_OK;

    // Log message format ... variable argument list
    va_list ap;
    va_start(ap, format);

    vsprintf_s(errParameter, MAX_BUFFER_SIZE, format, ap);
    err = ::TM_AddStringReturn(g_WiFi_Test_ID, "ERROR_MESSAGE", errParameter);
	// Ignore error handling

	err = ::vDUT_Run( g_WiFi_Dut, "FUNCTION_FAILED" );
	// Ignore error handling

    va_end(ap);

    return err;
}

WIFI_TEST_API int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...)
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
		sprintf_s(errMsg, sizeOfBuf, "[WiFi] LogReturnMessage() return error, Logger_ID < 0.\n");
	}

    return err;
}

WIFI_TEST_API int  GetInputParameters( map<string, WIFI_SETTING_STRUCT>& inputMap)
{
	int     err = ERR_OK;
	char    stringParameter[MAX_BUFFER_SIZE] = {'\0'};

	map<string, WIFI_SETTING_STRUCT>::iterator inputMap_Iter;

    inputMap_Iter = inputMap.begin();
    while( inputMap_Iter!=inputMap.end() )
    {
		int strError = ERR_OK;
        switch( inputMap_Iter->second.type )
        {
        case WIFI_SETTING_TYPE_INTEGER:
            err += ::TM_GetIntegerParameter(g_WiFi_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (int*)inputMap_Iter->second.value);
            break;
        case WIFI_SETTING_TYPE_DOUBLE:
            err += ::TM_GetDoubleParameter(g_WiFi_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), (double*)inputMap_Iter->second.value);
            break;
        case WIFI_SETTING_TYPE_STRING:			
            stringParameter[0] = 0;
            strError = ::TM_GetStringParameter(g_WiFi_Test_ID, (TM_STR)inputMap_Iter->first.c_str(), stringParameter, MAX_BUFFER_SIZE);
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
				// do nothing. Sometime, the Global_Setting's input parameters will get into this case.
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

WIFI_TEST_API int  CheckDutTransmitStatus(void)
{
	// Modified /* -cfy@sunnyvale, 2012/3/13- */
	int err = ERR_OK;
	int	tx01 = 0, tx02 = 0, tx03 = 0, tx04 = 0;
	int	rx01 = 0, rx02 = 0, rx03 = 0, rx04 = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	TM_GetIntegerParameter(g_WiFi_Test_ID, "TX1", &tx01);
	TM_GetIntegerParameter(g_WiFi_Test_ID, "TX2", &tx02);
	TM_GetIntegerParameter(g_WiFi_Test_ID, "TX3", &tx03);
	TM_GetIntegerParameter(g_WiFi_Test_ID, "TX4", &tx04);

	TM_GetIntegerParameter(g_WiFi_Test_ID, "RX1", &rx01);
	TM_GetIntegerParameter(g_WiFi_Test_ID, "RX2", &rx02);
	TM_GetIntegerParameter(g_WiFi_Test_ID, "RX3", &rx03);
	TM_GetIntegerParameter(g_WiFi_Test_ID, "RX4", &rx04);

	if ( (tx01==1)||(tx02==1)||(tx03==1)||(tx04==1) ) // This is a Tx function
	{			
		InitializeInternalRxParameters();

		if ( g_vDutRxActived )		// If Dut still under Rx mode
		{
		   /*-----------*
			*  Rx Stop  *
			*-----------*/
			err = vDUT_Run(g_WiFi_Dut, "RX_STOP");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					return err;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RX_STOP) return error.\n");
					return err;
				}
			}
			else
			{
				g_vDutRxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RX_STOP) return OK.\n");
			}
		}
	}
	else // workaround for fix xtal_calibration failed issue Jim
	//if ( (rx01==1)||(rx02==1)||(rx03==1)||(rx04==1) ) // This is a Rx function
	{
		InitializeInternalTxParameters();

		if ( g_vDutTxActived )		// If Dut still under Tx mode
		{
		   /*-----------*
			*  Tx Stop  *
			*-----------*/
			err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					return err;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
					return err;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}

		}
		else
		{
			// do nothing
		}
	}

    return err;
}

// -cfy@sunnyvale, 2012/3/13-
WIFI_TEST_API int CheckTesterConsistentStatus( std::map<std::string, std::string> &versionMap)
{
	int err = ERR_OK;
    vector<string> testersFW;
	vector<string> testersVsaFpga;
	vector<string> testersVsgFpga;
	char version[MAX_BUFFER_SIZE]={'\0'};

	testersFW.clear();
	testersVsaFpga.clear();
	testersVsgFpga.clear();

	map<string, string>::iterator versionMapIter;
	versionMapIter = versionMap.begin();
	while (versionMapIter != versionMap.end())
	{
		if(string::npos != versionMapIter->first.find("FW") )
		{
			testersFW.push_back(versionMapIter->second.c_str());
		}
		else
		{
			//do nothing
		}

		if(string::npos != versionMapIter->first.find("VSAFPGA") )
		{
			testersVsaFpga.push_back(versionMapIter->second.c_str());
		}
		else
		{
			//do nothing
		}

		if(string::npos != versionMapIter->first.find("VSGFPGA") )
		{
			testersVsgFpga.push_back(versionMapIter->second.c_str());
		}
		else
		{
			//do nothing
		}
		versionMapIter ++;
	}

	//Check Tester consistent
	// 1. FW
	// 2. FPGA (VSA/VSG)

	std::vector<std::string>::iterator versionIter;

	versionIter = testersFW.begin();
	sprintf_s(version,MAX_BUFFER_SIZE, "%s", versionIter->c_str());
    versionIter ++;
	while(versionIter != testersFW.end())
	{
		if(strcmp(version,versionIter->c_str()) !=0)
		{
			err = -1;
			break;
		}
		versionIter ++;
	}

	versionIter = testersVsaFpga.begin();
	sprintf_s(version,MAX_BUFFER_SIZE, "%s", versionIter->c_str());
    versionIter ++;
	while(versionIter != testersVsaFpga.end())
	{
		if(strcmp(version,versionIter->c_str()) !=0)
		{
			err = -1;
			break;
		}
		versionIter ++;
	}

	versionIter = testersVsgFpga.begin();
	sprintf_s(version,MAX_BUFFER_SIZE, "%s", versionIter->c_str());
    versionIter ++;
	while(versionIter != testersVsgFpga.end())
	{
		if(strcmp(version,versionIter->c_str()) !=0)
		{
			err = -1;
			break;
		}
		versionIter ++;
	}

	testersFW.clear();
	testersVsaFpga.clear();
	testersVsgFpga.clear();

	return err;
}
/* <><~~ */



WIFI_TEST_API int  WiFiTestMode(char* strDataRate, char* strBandwidth, int* wifiMode, int* wifiStreamNum)
{
    int    err = ERR_OK;
    int    dataRateIndex, dataRatePBCC22, dataRateOFDM6, dataRateOFDM54, dataRateMCS7, dataRateMCS15, dataRateMCS23, dataRateHALF3, dataRateQUAR13_5; 


    err = ::TM_WiFiConvertDataRateNameToIndex(strDataRate, &dataRateIndex);   // Convert data rate name to index
    if (ERR_OK!=err)
    {
        return err;
    }
    err = ::TM_WiFiConvertDataRateNameToIndex("PBCC-22", &dataRatePBCC22);    // Convert data rate name to index
    if (ERR_OK!=err)
    {
        return err;
    }
    err = ::TM_WiFiConvertDataRateNameToIndex("OFDM-6",  &dataRateOFDM6);     // Convert data rate name to index
    if (ERR_OK!=err)
    {
        return err;
    }
    err = ::TM_WiFiConvertDataRateNameToIndex("OFDM-54", &dataRateOFDM54);    // Convert data rate name to index
    if (ERR_OK!=err)
    {
        return err;
    }
    err = ::TM_WiFiConvertDataRateNameToIndex("MCS7", &dataRateMCS7);      // Convert data rate name to index
    if (ERR_OK!=err)
    {
        return err;
    }
    err = ::TM_WiFiConvertDataRateNameToIndex("MCS15", &dataRateMCS15);    // Convert data rate name to index
    if (ERR_OK!=err)
    {
        return err;
    }
    err = ::TM_WiFiConvertDataRateNameToIndex("MCS23", &dataRateMCS23);    // Convert data rate name to index
    if (ERR_OK!=err)
    {
        return err;
    }
    err = ::TM_WiFiConvertDataRateNameToIndex("HALF-3", &dataRateHALF3);    // Convert data rate name to index
    if (ERR_OK!=err)
    {
        return err;
    }    
    err = ::TM_WiFiConvertDataRateNameToIndex("QUAR-13_5", &dataRateQUAR13_5);    // Convert data rate name to index
    if (ERR_OK!=err)
    {
        return err;
    }
   
    if ( dataRateIndex<=dataRatePBCC22 ) 
    {   //  [Case 01]: 802.11b
        *wifiMode = WIFI_11B;
        *wifiStreamNum = WIFI_ONE_STREAM;
    }
    else if ( (dataRateIndex>=dataRateOFDM6)&&(dataRateIndex<=dataRateOFDM54) ) 
    {   //  [Case 02]: 802.11a/g
        *wifiMode = WIFI_11AG;
        *wifiStreamNum = WIFI_ONE_STREAM;
    }
    else if ( (dataRateIndex>=dataRateHALF3)&&(dataRateIndex<=dataRateQUAR13_5) ) 
    {   //  [Case 03]: 802.11p
        *wifiMode = WIFI_11AG;
        *wifiStreamNum = WIFI_ONE_STREAM;
    }
    else // [Case 03]: 802.11n
    {
        if ( 0==strcmp(strBandwidth, "HT20") ) *wifiMode = WIFI_11N_HT20;
        else                                   *wifiMode = WIFI_11N_HT40;

        if (dataRateIndex<=dataRateMCS7)       *wifiStreamNum = WIFI_ONE_STREAM;
        else if (dataRateIndex<=dataRateMCS15) *wifiStreamNum = WIFI_TWO_STREAM;
        else if (dataRateIndex<=dataRateMCS23) *wifiStreamNum = WIFI_THREE_STREAM; 
		else								   *wifiStreamNum = WIFI_FOUR_STREAM;
    }
    
    return err;
}

WIFI_TEST_API int  WiFiSaveSigFile(char* fileName)
{
    int    err = ERR_OK;

	char    logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char    logInfoMessage[MAX_BUFFER_SIZE] = {'\0'};

	if (1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] The VSA_SAVE_CAPTURE_ALWAYS is set to ON, it may slow down the test time.\n");
	}

	if ( (1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED)||(1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS))  
	{	  
		// -cfy@sunnyvale, 2012/3/13-
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
		/* <><~~ */

		err = ::LP_SaveVsaSignalFile(c_path);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SaveVsaSignalFile(\"%s\") return error.\n", c_path);
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] Save Signal File (\"%s\").\n", c_path );
		}
	}
	else
	{
		// do nothing...
	}

	return err;
}

int  GetMultiWaveformFileName(char* perfix, char* postfix, int wifiMode, char* bandwidth, char* datarate, char* preamble, char* packetFormat11N, char* guardInterval11N, char* waveformFileName, int bufferSize)
{
    int  err = ERR_OK;

	char datarateString[MAX_BUFFER_SIZE] = {'\0'};
	int  rateIndexBegin = 0, rateIndexEnd = 0;

	// Find out the datarate index
	if ( (wifiMode==WIFI_11B)||(wifiMode==WIFI_11AG) )
	{
		if ((strcmp(bandwidth,"HALF")==0) || (strcmp(bandwidth,"QUAR")==0)) {
			// 11p 
			// do nothing
		} else {
			for (int i=0;i<MAX_BUFFER_SIZE;i++)
			{
				if (datarate[i] == '-')
				{
					rateIndexBegin = i+1;
				}
				else if (datarate[i] == '\0')
				{
					rateIndexEnd = i;
					break;
				}
				else
				{	
					// do nothing, keep searching...
				}
			}
			// Copy the datarate 
			int rateIndex = 0;
			for (int i=rateIndexBegin;i<=rateIndexEnd;i++)
			{
				datarateString[rateIndex] = datarate[i];
				rateIndex++;
			}
		}
	}
	else	// WIFI_11N_HT20 or WIFI_11N_HT40
	{
		// do nothing
	}

	// Find out the keyword of the waveform file name
	if (wifiMode==WIFI_11B)
	{
		if (0==strcmp(preamble, "LONG"))
		{	
            if (0==strcmp(datarateString, "1"))	// Waveform name for 1 Mbps (Long Preamble only)
            {
                sprintf_s(waveformFileName, bufferSize, "%s_11B_%s_%s", perfix, datarateString, postfix);
            }
            else
            {
			    sprintf_s(waveformFileName, bufferSize, "%s_11B_%sL_%s", perfix, datarateString, postfix);			
            }
		}
		else	// 802.11b preamble = SHORT
		{
			if (0==strcmp(datarateString, "1"))	// Waveform name for 1 Mbps (Long Preamble only)
			{
				sprintf_s(waveformFileName, bufferSize, "%s_11B_%s_%s", perfix, datarateString, postfix);
			}
			else
			{
				sprintf_s(waveformFileName, bufferSize, "%s_11B_%sS_%s", perfix, datarateString, postfix);			
			}
		}
	}
	else if (wifiMode==WIFI_11AG)
	{
		if ((strcmp(bandwidth,"HALF")==0) || (strcmp(bandwidth,"QUAR")==0)) {
			sprintf_s(waveformFileName, bufferSize, "%s_11P_%s_%s", perfix, datarateString, postfix);
		} else {
			sprintf_s(waveformFileName, bufferSize, "%s_11AG_%s_%s", perfix, datarateString, postfix);
		}
	}
	else	// WIFI_11N_HT20 or WIFI_11N_HT40
	{		
		/*****************************************************************
		* For backward compatibility,									 *			  
		*we use old waveform names for mixed, long guard interval format *
		******************************************************************/
		
		if (0==strcmp(guardInterval11N, "LONG") && 0==strcmp(packetFormat11N, "MIXED"))  // 802.11n, mixed format, long preamble, use old default waveform name
		{
			sprintf_s(waveformFileName, bufferSize, "%s_11N_%s_%s_%s_%s", perfix, bandwidth, packetFormat11N, datarate, postfix);	
		}
		else if (0==strcmp(guardInterval11N, "LONG") && 0==strcmp(packetFormat11N, "GREENFIELD"))  //802.11n long guard interval, greefield
		{
			sprintf_s(waveformFileName, bufferSize, "%s_11N_%s_%s_L_GI_%s_%s", perfix, bandwidth, packetFormat11N, datarate, postfix);	
		}
		else      //802.11n short guard interval, mixed/greenfield
		{
			sprintf_s(waveformFileName, bufferSize, "%s_11N_%s_%s_S_GI_%s_%s", perfix, bandwidth, packetFormat11N, datarate, postfix);	
		}
	}

	return err;
}


WIFI_TEST_API int  GetWaveformFileName(char* perfix, char* postfix, int wifiMode, char* bandwidth, char* datarate, char* preamble, char* packetFormat11N, char* guardInterval11N, char* waveformFileName, int bufferSize)
{
    int  err = ERR_OK;

	char keyword[MAX_BUFFER_SIZE] = {'\0'}, datarateString[MAX_BUFFER_SIZE] = {'\0'};
	int  rateIndexBegin = 0, rateIndexEnd = 0;

	map<string, WIFI_SETTING_STRUCT>::iterator inputMap_Iter;

	// Find out the datarate index
	if ( (wifiMode==WIFI_11B)||(wifiMode==WIFI_11AG) )
	{
		if ((strcmp(bandwidth,"HALF")==0) || (strcmp(bandwidth,"QUAR")==0)) {
			// 11p 
			// do nothing
		} else {
			for (int i=0;i<MAX_BUFFER_SIZE;i++)
			{
				if (datarate[i] == '-')
				{
					rateIndexBegin = i+1;
				}
				else if (datarate[i] == '\0')
				{
					rateIndexEnd = i;
					break;
				}
				else
				{	
					// do nothing, keep searching...
				}
			}
			// Copy the datarate 
			int rateIndex = 0;
			for (int i=rateIndexBegin;i<=rateIndexEnd;i++)
			{
				datarateString[rateIndex] = datarate[i];
				rateIndex++;
			}
		}
	}
	else	// WIFI_11N_HT20 or WIFI_11N_HT40
	{
		// do nothing
	}

	// Find out the keyword of the waveform file name
	if (wifiMode==WIFI_11B)
	{
		if (0==strcmp(preamble, "LONG"))
		{	
            if (0==strcmp(datarateString, "1"))	// Waveform name for 1 Mbps (Long Preamble only)
            {
                sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_11B_%s_%s", perfix, datarateString, postfix);
            }
            else
            {
			    sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_11B_%sL_%s", perfix, datarateString, postfix);			
            }
		}
		else	// 802.11b preamble = SHORT
		{
			if (0==strcmp(datarateString, "1"))	// Waveform name for 1 Mbps (Long Preamble only)
			{
				sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_11B_%s_%s", perfix, datarateString, postfix);
			}
			else
			{
				sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_11B_%sS_%s", perfix, datarateString, postfix);			
			}
		}
	}
	else if (wifiMode==WIFI_11AG)
	{
		if ((strcmp(bandwidth,"HALF")==0) || (strcmp(bandwidth,"QUAR")==0)) {
			sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_11P_%s_%s", perfix, datarateString, postfix); // Modified /* -cfy@sunnyvale, 2012/3/13- */
		} else {
			sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_11AG_%s_%s", perfix, datarateString, postfix);
		}
	}
	else	// WIFI_11N_HT20 or WIFI_11N_HT40
	{		
		/*****************************************************************
		* For backward compatibility,									 *			  
		*we use old waveform names for mixed, long guard interval format *
		******************************************************************/
		
		if (0==strcmp(guardInterval11N, "LONG") && 0==strcmp(packetFormat11N, "MIXED"))  // 802.11n, mixed format, long preamble, use old default waveform name
		{
			sprintf_s(keyword, bufferSize, "%s_11N_%s_%s_%s_%s", perfix, bandwidth, packetFormat11N, datarate, postfix);	
		}
		else if (0==strcmp(guardInterval11N, "LONG") && 0==strcmp(packetFormat11N, "GREENFIELD"))  //802.11n long guard interval, greefield
		{
			sprintf_s(keyword, bufferSize, "%s_11N_%s_%s_L_GI_%s_%s", perfix, bandwidth, packetFormat11N, datarate, postfix);	
		}
		else      //802.11n short guard interval, mixed/greenfield
		{
			sprintf_s(keyword, bufferSize, "%s_11N_%s_%s_S_GI_%s_%s", perfix, bandwidth, packetFormat11N, datarate, postfix);	
		}

	
	}

    // Searching the keyword in g_WiFiGlobalSettingParamMap
	inputMap_Iter = g_WiFiGlobalSettingParamMap.find(keyword);
    if( inputMap_Iter!=g_WiFiGlobalSettingParamMap.end() )
    {
		char dummyString[MAX_BUFFER_SIZE];
		strcpy_s (dummyString, MAX_BUFFER_SIZE, (char*)inputMap_Iter->second.value);

		if ( strlen(g_defaultFilePath)>0 )		// default waveform file path not empty
		{
			sprintf_s(waveformFileName, bufferSize, "%s/%s", g_defaultFilePath, dummyString);
		}
		else	// default waveform file path is empty, then using g_WiFiGlobalSettingParam.PER_WAVEFORM_PATH
		{
			sprintf_s(waveformFileName, bufferSize, "%s/%s", g_WiFiGlobalSettingParam.PER_WAVEFORM_PATH, dummyString);
		}		

		//if Tester is IQxel and file extension is .ref change file extension from .ref to .iqref
		if( g_iTesterName == IQTYPE_XEL )
		{
			char *pch = NULL;
			pch = strstr(waveformFileName,".ref\0");
			if(pch!= NULL)
				strncpy (pch,".iqref\0",7);
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
			if ( strlen(g_defaultFilePath)>0 )		// Default waveform file path not empty, but still can't find the waveformfile, then return error.
			{
				err = TM_ERR_PARAM_DOES_NOT_EXIST;
			}
			else	// Try to find the file in upper folder
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
					sprintf_s(g_defaultFilePath, bufferSize, "../%s", g_WiFiGlobalSettingParam.PER_WAVEFORM_PATH);
					// And using the new path
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

WIFI_TEST_API int  GetPacketNumber(int wifiMode, char* bandwidth, char* datarate, char* packetFormat11N, char* guardInterval11N, int *packetNumber)
{
    int  err = ERR_OK;

	char keyword[MAX_BUFFER_SIZE] = {'\0'}, datarateString[MAX_BUFFER_SIZE] = {'\0'};
	int  rateIndexBegin = 0, rateIndexEnd = 0;

	map<string, WIFI_SETTING_STRUCT>::iterator inputMap_Iter;

	// Find out the datarate index
	if ( (wifiMode==WIFI_11B)||(wifiMode==WIFI_11AG) )
	{
		if ((strcmp(bandwidth,"HALF")==0) || (strcmp(bandwidth,"QUAR")==0)) {
			// 11p 
			// do nothing
		} else {
			for (int i=0;i<MAX_BUFFER_SIZE;i++)
			{
				if (datarate[i] == '-')
				{
					rateIndexBegin = i+1;
				}
				else if (datarate[i] == '\0')
				{
					rateIndexEnd = i;
					break;
				}
				else
				{	
					// do nothing, keep searching...
				}
			}
			// Copy the datarate 
			int rateIndex = 0;
			for (int i=rateIndexBegin;i<=rateIndexEnd;i++)
			{
				datarateString[rateIndex] = datarate[i];
				rateIndex++;
			}
		}
	}
	else	// WIFI_11N_HT20 or WIFI_11N_HT40
	{
		// do nothing
	}

	// Find out the keyword of the waveform file name
	if (wifiMode==WIFI_11B)
	{	
		sprintf_s(keyword, MAX_BUFFER_SIZE, "PER_11B_%s_PACKETS_NUM", datarateString);			
	}
	else if (wifiMode==WIFI_11AG)
	{
		if ((strcmp(bandwidth,"HALF")==0) || (strcmp(bandwidth,"QUAR")==0)) {
			sprintf_s(keyword, MAX_BUFFER_SIZE, "PER_11P_%s_PACKETS_NUM", datarateString); // Modified /* -cfy@sunnyvale, 2012/3/13- */
		} else {
			sprintf_s(keyword, MAX_BUFFER_SIZE, "PER_11AG_%s_PACKETS_NUM", datarateString);
		}
	}
	else	// WIFI_11N_HT20 or WIFI_11N_HT40
	{		

		/*****************************************************************
		* For backward compatibility,									 *			  
		*we use old waveform names for mixed, long guard interval format *
		******************************************************************/
		
		if (0==strcmp(guardInterval11N, "LONG") && 0==strcmp(packetFormat11N, "MIXED"))  // 802.11n, mixed format, long preamble, use old default keyword
		{
			sprintf_s(keyword, MAX_BUFFER_SIZE, "PER_11N_%s_%s_%s_PACKETS_NUM", bandwidth,packetFormat11N, datarate);	
		}
		else if (0==strcmp(guardInterval11N, "LONG") && 0==strcmp(packetFormat11N, "GREENFIELD"))  //802.11n long guard interval, greefield
		{
			sprintf_s(keyword, MAX_BUFFER_SIZE, "PER_11N_%s_%s_L_GI_%s_PACKETS_NUM", bandwidth, packetFormat11N, datarate);	
		}
		else      //802.11n short guard interval, mixed/greenfield
		{
			sprintf_s(keyword, MAX_BUFFER_SIZE, "PER_11N_%s_%s_S_GI_%s_PACKETS_NUM", bandwidth, packetFormat11N, datarate);	
		}

	}

    // Searching the keyword in g_WiFiGlobalSettingParamMap
	inputMap_Iter = g_WiFiGlobalSettingParamMap.find(keyword);
    if( inputMap_Iter!=g_WiFiGlobalSettingParamMap.end() )
    {
		*packetNumber = *((int*)inputMap_Iter->second.value);
	}
    else
    {
        err = TM_ERR_PARAM_DOES_NOT_EXIST;
    }

	return err;
}

// This function is for backward compatible
WIFI_TEST_API int CheckPathLossTable(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb)
{
	int  err = ERR_OK;

	err = CheckPathLossTableExt(testID, freqMHz, ant01, ant02, ant03, ant04, cableLoss, cableLossReturn, cableLossDb, TX_TABLE);

	return err;
}

WIFI_TEST_API int CheckPathLossTableExt(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb, int indicatorTxRx)
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

WIFI_TEST_API double CalcCableLossDb(int ant1, int ant2, int ant3, int ant4, double cableLoss1, double cableLoss2, double cableLoss3, double cableLoss4)
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


WIFI_TEST_API int CheckAntennaOrderByStream(int streamOrder, int ant1, int ant2, int ant3, int ant4, int *antOrder)
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

WIFI_TEST_API int GetWiFiTestID(TM_ID* wifiTestID)
{
	*wifiTestID = g_WiFi_Test_ID;
    return ERR_OK;
}

// -cfy@sunnyvale, 2012/3/13-
WIFI_TEST_API int SetWiFiTestID(TM_ID wifiTestID)
{
	g_WiFi_Test_ID = wifiTestID;
	return ERR_OK;
}
/* <><~~ */

WIFI_TEST_API int   GetWiFiDutID(vDUT_ID* wifiDutID)
{
    *wifiDutID = g_WiFi_Dut;
    return ERR_OK;
}

WIFI_TEST_API int   GetWiFiGlobalSetting(WIFI_GLOBAL_SETTING** wifiGlobleSetting)
{
    *wifiGlobleSetting = &g_WiFiGlobalSettingParam;
    return ERR_OK;
}

WIFI_TEST_API int   GetWiFiRecordedParam(WIFI_RECORD_PARAM** wifiRecordedParam)
{
    *wifiRecordedParam = &g_RecordedParam;
    return ERR_OK;
}

WIFI_TEST_API int   SetTesterNumber(int testerNumber)
{
    g_Tester_Number = testerNumber;
    return ERR_OK;
}

WIFI_TEST_API int   GetTesterNumber(int *testerNumber)
{
    *testerNumber = g_Tester_Number;
    return ERR_OK;
}

WIFI_TEST_API int   SetTesterType(int testerType)
{
    g_Tester_Type = testerType;
    return ERR_OK;
}

WIFI_TEST_API int   GetTesterType(int *testerType)
{
    *testerType = g_Tester_Type;
    return ERR_OK;
}

WIFI_TEST_API int   GetTesterName(int *flag)
{
	*flag = g_iTesterName;
	return ERR_OK;
}

WIFI_TEST_API int   SetTesterName(int flag)
{
	g_iTesterName = flag;
	return ERR_OK;
}

WIFI_TEST_API int   SetTesterReconnect(int flag)
{
    g_Tester_Reconnect = flag;
    return ERR_OK;
}

WIFI_TEST_API int   GetTesterReconnect(int *flag)
{
	*flag = g_Tester_Reconnect;
	return ERR_OK;
}

WIFI_TEST_API int   SetDutConfigChanged(bool changed)
{
    g_dutConfigChanged = changed;
    return ERR_OK;
}

WIFI_TEST_API int   GetDutConfigChanged(bool *changed)
{
     *changed = g_dutConfigChanged;
    return ERR_OK;
}


WIFI_TEST_API int GetDutTxActived(bool* vDutTxActived)
{
	*vDutTxActived = g_vDutTxActived;
    return ERR_OK;
}

WIFI_TEST_API int SetDutTxActived(bool vDutTxActived)
{
	g_vDutTxActived = vDutTxActived;
    return ERR_OK;
}

// -cfy@sunnyvale, 2012/3/13-
WIFI_TEST_API int GetDutRxActived(bool* vDutRxActived)
{
	*vDutRxActived = g_vDutRxActived;
    return ERR_OK;
}

WIFI_TEST_API int SetDutRxActived(bool vDutRxActived)
{
	g_vDutRxActived = vDutRxActived;
    return ERR_OK;
}
/* <><~~ */

WIFI_TEST_API int  PassTestResultToDutInputContainer( map<string, WIFI_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;

    map<string, WIFI_SETTING_STRUCT>::iterator returnMap_Iter;

    ::vDUT_ClearParameters(g_WiFi_Dut);
    returnMap_Iter = returnMap.begin();
    while( returnMap_Iter != returnMap.end() )
    {
        if( returnMap_Iter->second.type==WIFI_SETTING_TYPE_INTEGER )
        {
            int *valueInteger = reinterpret_cast<int *>(returnMap_Iter->second.value);
            err = ::vDUT_AddIntegerParameter(g_WiFi_Dut, (char*)returnMap_Iter->first.c_str(), *valueInteger);
            if (vDUT_ERR_OK!=err)    
            {
                // If TM_AddIntegerReturn fails, no need to continue
                break;
            }
        }
        else if(returnMap_Iter->second.type==WIFI_SETTING_TYPE_DOUBLE)
        {
            double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            err = ::vDUT_AddDoubleParameter(g_WiFi_Dut, (char*)returnMap_Iter->first.c_str(), *valueDouble);
            if (vDUT_ERR_OK!=err)    
            {
                // If TM_AddDoubleReturn fails, no need to continue
                break;
            }
        }
        else  if(returnMap_Iter->second.type==WIFI_SETTING_TYPE_STRING) 
        {   
            char *valueString = reinterpret_cast<char *>(returnMap_Iter->second.value);
            err = ::vDUT_AddStringParameter(g_WiFi_Dut, (char*)returnMap_Iter->first.c_str(), valueString);
            if (vDUT_ERR_OK!=err)    
            {
                // If TM_AddStringReturn fails, no need to continue
                break;
            }
        }
        //else if( returnMap_Iter->second.type==WIFI_SETTING_TYPE_ARRAY_DOUBLE )
        //{
        //    vector<double> doubleVector = * reinterpret_cast<vector<double> *>(returnMap_Iter->second.value);
        //    if (0 != doubleVector.size())
        //    {
        //        double *doubleArray = new double[(int) doubleVector.size()];
        //        for (unsigned int i=0; i<doubleVector.size(); i++)
        //        {
        //            doubleArray[i] = doubleVector[i];
        //        }
        //        err = ::vDUT_AddDoubleParameters(g_WiFi_Test_ID, (char*)returnMap_Iter->first.c_str(), doubleArray, (int)doubleVector.size());

        //        delete [] doubleArray;

        //        if (vDUT_ERR_OK!=err)    
        //        {
        //            // If TM_AddDoubleReturn fails, no need to continue
        //            break;
        //        }
        //    }
        //    else
        //    {
        //        // do nothing
        //    }
        //}
         else 
        {
            // Unknown type
            //err = -1;
        }

        returnMap_Iter++;
    }

    return err;

}


WIFI_TEST_API int  AnalyzeCWFrequency(double centerFreqMHz, double vsaAmpPowerDbm, double samplingTimeUs, int avgTimes, double *cwFreqResultHz, char *errorMsg)
{
    int  err = ERR_OK;

	int    avgIteration = 0;
	double cwFreqHz = 0.0;
	double cwFreqHz_sum = 0.0;
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};


	/*--------------------*
	 * Setup IQtester VSA *
	 *--------------------*/
	err = LP_SetVsa(  (centerFreqMHz+WIFI_FREQ_SHIFT_FOR_CW_MHZ)*1e6,
					   vsaAmpPowerDbm,
					   g_WiFiGlobalSettingParam.VSA_PORT,
					   0,
					   g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
					   g_WiFiGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
					 );
    if (ERR_OK!=err)
    {
        sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] Fail to setup VSA.\n");
        return err;
    }

   /*----------------------------*
	* Disable VSG output signal  *
	*----------------------------*/
	// make sure no signal is generated by the VSG
	//err = ::LP_EnableVsgRF(0);
	//if ( ERR_OK!=err )
	//{
	//	sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
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
		/**** workaround for CW analyze issue, shall be passed via input parameter Jim *****/
		samplingTimeUs = 220;
		/***********************************************************************************/
        err = LP_VsaDataCapture(samplingTimeUs/1000000, IQV_TRIG_TYPE_FREE_RUN);     
        if(ERR_OK==err)	// capture is ok
        {
			sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] LP_VsaDataCapture() at %.1f MHz return OK.\n", (centerFreqMHz+WIFI_FREQ_SHIFT_FOR_CW_MHZ));

			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			if (1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WiFi_CW_FREQ_SaveAlways", (centerFreqMHz+WIFI_FREQ_SHIFT_FOR_CW_MHZ));
				WiFiSaveSigFile(sigFileNameBuffer);
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
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WiFi_CW_FREQ_AnalysisFailed", (centerFreqMHz+WIFI_FREQ_SHIFT_FOR_CW_MHZ));
				WiFiSaveSigFile(sigFileNameBuffer);				
				sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] Fail to analyze signal at %.1f MHz.\n", (centerFreqMHz+WIFI_FREQ_SHIFT_FOR_CW_MHZ));
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
				cwFreqHz = cwFreqHz + WIFI_FREQ_SHIFT_FOR_CW_MHZ*1e6;
				cwFreqHz_sum = cwFreqHz_sum + cwFreqHz;				
			}

        }
        else
        {   
            // Fail Capture
			err = -1;
            sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] Fail to capture signal at %.1f MHz.\n", (centerFreqMHz+WIFI_FREQ_SHIFT_FOR_CW_MHZ));
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


WIFI_TEST_API int QuickCaptureRetry(double centerFreqMHz, double samplingTimeUS, int triggerType, int ht40Mode, double *PowerDbm, char *errorMsg)
{
    int  err = ERR_OK;
	char sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};

	double peakPowerDbm = NA_NUMBER;
	double avgPowerDbm  = NA_NUMBER;


	LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] First capture failed, start doing the capture retry.\n");

   /*----------------------------*
	* Perform normal VSA capture *
	*----------------------------*/
    err = LP_VsaDataCapture(0.01, IQV_TRIG_TYPE_FREE_RUN);              
    if( ERR_OK==err )	// capture is ok
    {
		LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] (Retry) LP_VsaDataCapture(IQV_TRIG_TYPE_FREE_RUN) at %.1f MHz return OK.\n", centerFreqMHz);

		/*--------------*
		 *  Capture OK  *
		 *--------------*/
		//if (1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
		{
			// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WiFi_Quick_Power", centerFreqMHz);
			WiFiSaveSigFile(sigFileNameBuffer);
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
			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WiFi_Quick_Power_Analysis_Failed", centerFreqMHz);
			WiFiSaveSigFile(sigFileNameBuffer);
			LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] (Retry) LP_AnalyzePower() return error and save the sig file %s.\n", sigFileNameBuffer);
			return err;
		}
		else		// Analyze is ok
		{
		   /*-----------------------------*
		    *  Retrieve analysis Results  *
		    *-----------------------------*/
			peakPowerDbm  = ::LP_GetScalarMeasurement("P_peak_all_dBm", 0);
			peakPowerDbm = ((int)(peakPowerDbm*100))/100; // drop any digits after 0.00
			// make sure peakPowerDbm is within the meaningful range
			if (peakPowerDbm <= -20) peakPowerDbm = -20;
			else if (peakPowerDbm >= 50) peakPowerDbm = 50;
			if ( -99.00 >= (peakPowerDbm)  )
			{
				peakPowerDbm = NA_NUMBER;				
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WiFi_Quick_Power_Result_Failed", centerFreqMHz);
				WiFiSaveSigFile(sigFileNameBuffer);
				err = -1;				
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] (Retry) LP_GetScalarMeasurement(P_peak_all_dBm) return error and save the sig file %s.\n", sigFileNameBuffer);
				return err;
			}
			else
			{			
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] (Retry) Quick power analysis OK, (without path loss) P_peak_all_dBm = %.1f dBm.\n", peakPowerDbm);
			}

			avgPowerDbm  = ::LP_GetScalarMeasurement("P_av_no_gap_all_dBm", 0);
			if ( -99.00 >= (avgPowerDbm)  )
			{
				avgPowerDbm = NA_NUMBER;				
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WiFi_Quick_Power_Result_Failed", centerFreqMHz);
				WiFiSaveSigFile(sigFileNameBuffer);
				err = -1;				
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] (Retry) LP_GetScalarMeasurement(P_av_no_gap_all_dBm) return error and save the sig file %s.\n", sigFileNameBuffer);
				return err;
			}
			else
			{	
				*PowerDbm = ((int)(avgPowerDbm*100))/100; // to drop any digits after 0.00
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] (Retry) Quick power analysis OK, (without path loss) P_av_no_gap_all_dBm = %.1f dBm.\n", avgPowerDbm);
			}
		}
    }
    else
    {   
        // Fail Capture
		err = -1;
        LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] (Retry) Fail to capture signal at %.1f MHz.\n", centerFreqMHz);
        return err;
    }

   /*-----------------*
    *  Retry Capture  *
    *-----------------*/
	err = LP_SetVsa(   centerFreqMHz*1e6,
					   peakPowerDbm+2,
					   g_WiFiGlobalSettingParam.VSA_PORT,
					   0,
					   g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
					   g_WiFiGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
					 );
    if (ERR_OK!=err)
    {
        LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] (Retry) Fail to setup VSA.\n");
        return err;
    }
	else
	{
		LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] (Retry) LP_SetVsa() with peak PowerLevel = %.1f +2 return OK.\n", peakPowerDbm);
	}

	// Perform Normal VSA capture 
	err = ::LP_VsaDataCapture( samplingTimeUS/1000000, triggerType, 80e6, ht40Mode );
	if (ERR_OK!=err)
	{
		LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] (Retry) Fail to capture WiFi signal at %.1f MHz with VSA TriggerType = %d.\n", centerFreqMHz, triggerType);
		return err;
	}
	else
	{
		LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] (Retry) LP_VsaDataCapture(TriggerType=%d) at %.1f MHz return OK.\n", triggerType, centerFreqMHz);
	}
	

	return err;
}

WIFI_TEST_API int QuickCapturePower(double centerFreqMHz, double samplingTimeUS, int triggerType, int ht40Mode, double *PowerDbm, char *errorMsg)
{
    int  err = ERR_OK;
	char sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};

	//double peakPowerDbm = NA_NUMBER;
	double avgPowerDbm  = NA_NUMBER;



   /*----------------------------*
	* Perform normal VSA capture *
	*----------------------------*/
    err = LP_VsaDataCapture(0.01, IQV_TRIG_TYPE_FREE_RUN);
	
    if( ERR_OK==err )	// capture is ok
    {
		LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi]  LP_VsaDataCapture(IQV_TRIG_TYPE_FREE_RUN) at %.1f MHz return OK.\n", centerFreqMHz);

		/*--------------*
		 *  Capture OK  *
		 *--------------*/
		if (1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
		{
			// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WiFi_Quick_Power_SaveAlways", centerFreqMHz);
			WiFiSaveSigFile(sigFileNameBuffer);
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
			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WiFi_Quick_Power_Analysis_Failed", centerFreqMHz);
			WiFiSaveSigFile(sigFileNameBuffer);
			LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi]  LP_AnalyzePower() return error and save the sig file %s.\n", sigFileNameBuffer);
			return err;
		}
		else		// Analyze is ok
		{
		   /*-----------------------------*
		    *  Retrieve analysis Results  *
		    *-----------------------------*/
			/*peakPowerDbm  = ::LP_GetScalarMeasurement("P_peak_all_dBm", 0);
			if ( -99.00 >= (peakPowerDbm)  )
			{
				peakPowerDbm = NA_NUMBER;				
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WiFi_Quick_Power_Result_Failed", centerFreqMHz);
				WiFiSaveSigFile(sigFileNameBuffer);
				err = -1;				
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi]  LP_GetScalarMeasurement(P_peak_all_dBm) return error and save the sig file %s.\n", sigFileNameBuffer);
				return err;
			}
			else
			{			
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi]  Quick power analysis OK, (without path loss) P_peak_all_dBm = %.1f dBm.\n", peakPowerDbm);
			}*/

			avgPowerDbm  = ::LP_GetScalarMeasurement("P_av_no_gap_all_dBm", 0);
			if ( -99.00 >= (avgPowerDbm)  )
			{
				avgPowerDbm = NA_NUMBER;				
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%.1f", "WiFi_Quick_Power_Result_Failed", centerFreqMHz);
				WiFiSaveSigFile(sigFileNameBuffer);
				err = -1;				
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi]  LP_GetScalarMeasurement(P_av_no_gap_all_dBm) return error and save the sig file %s.\n", sigFileNameBuffer);
				return err;
			}
			else
			{	
				*PowerDbm = ((int)(avgPowerDbm*100))/100; // to drop any digits after 0.00
				// make sure PowerDbm is within the meaningful range
				if (*PowerDbm <= -20) *PowerDbm = -20;
				else if (*PowerDbm >= 50) *PowerDbm = 50;
				LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi]  Quick power analysis OK, (without path loss) P_av_no_gap_all_dBm = %.1f dBm.\n", avgPowerDbm);
			}
		}
	}
    else
    {   
        // Fail Capture
		err = -1;
        LogReturnMessage(errorMsg, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi]  Fail to capture signal at %.1f MHz.\n", centerFreqMHz);
        return err;
    }
	return err;
	
}

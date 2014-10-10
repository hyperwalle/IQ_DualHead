#include "stdafx.h"
#include "WiFi_11ac_MiMo_Test.h"
#include "IQlite_Logger.h"
#include "IQmeasure.h"
#include "math.h"

using namespace std;

// global variable 
TM_ID            g_WiFi_Test_ID		= -1;
vDUT_ID          g_WiFi_Dut			= -1;
bool             g_ReloadDutDll		= false;
bool			 g_vDutTxActived	= false;
bool			 g_vDutRxActived	= false;
bool			 g_dutConfigChanged = false;
bool             g_txCalibrationEnabled = false;
int				 g_Logger_ID        = -1;
int              g_WiFi_Test_timer  = -1;
int              g_WiFi_Test_logger = -1;
int				 g_Tester_Type      = IQ_View;
int				 g_Tester_Number    = 0;
int				 g_Tester_Reconnect = 0;
bool			 g_Tester_EnableVHT80 = FALSE;

const char      *g_WiFi_Test_Version = "1.5.1 (2010-05-24)\n";
char			 g_defaultFilePath[MAX_BUFFER_SIZE] = {'\0'};



WIFI_RECORD_PARAM g_RecordedParam;

// This global variable is declared in WiFi_Global_Setting.cpp
// Input Parameter Container
extern map<string, WIFI_SETTING_STRUCT> g_globalSettingParamMap;
extern WIFI_GLOBAL_SETTING g_globalSettingParam;

// end global

using namespace std;


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

int InitializeAll11acMIMOContainers(void)
{

    InitializeConnectTesterContainers();            // Needed by WiFi_Connect_IQTester
    InitializeDisconnectTesterContainers();         // Needed by WiFi_Disconnect_IQTester
    InitializeInsertDutContainers();                // Needed by WiFi_Insert_Dut
    InitializeInitializeDutContainers();            // Needed by WiFi_Initialize_Dut
    InitializeRemoveDutContainers();                // Needed by WiFi_Remove_Dut
    InitializeGlobalSettingContainers();            // Needed by WiFi_Global_Setting
    InitializeTXVerifyEvmContainers();              // Needed by WiFi_TX_Verify_Evm
    InitializeRXVerifyPerContainers();              // Needed by WiFi_RX_Verify_Per
    InitializeTXVerifyMaskContainers();             // Needed by WiFi_TX_Verify_Mask
    InitializeTXVerifyPowerContainers();            // Needed by WiFi_TX_Verify_Power
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
	InitializePowerModeDutContainers();				// Needed by WiFi Current test
	InitializeRunDutCommandContainers();            // Needed by WiFi_Run_Dut_Command

	InitializeloadPathLossTableContainers();
	InitializeInternalParameters();	

	return 0;
}


void CleanupAll11acMIMOContainers(void)
{
	CleanupTXVerifyMaskContainers();				// Needed by WiFi_TX_Verify_Mask
	return;
}

int InitializeInternalParameters(void)
{
	// Initial the internal parameters
	g_RecordedParam.ANT1 = NA_INTEGER;
	g_RecordedParam.ANT2 = NA_INTEGER;
	g_RecordedParam.ANT3 = NA_INTEGER;
	g_RecordedParam.ANT4 = NA_INTEGER;		
	g_RecordedParam.CABLE_LOSS_DB[0] = NA_DOUBLE;
	g_RecordedParam.CABLE_LOSS_DB[1] = NA_DOUBLE;
	g_RecordedParam.CABLE_LOSS_DB[2] = NA_DOUBLE;
	g_RecordedParam.CABLE_LOSS_DB[3] = NA_DOUBLE;	
	g_RecordedParam.CH_FREQ_MHZ         = NA_INTEGER;	
	g_RecordedParam.POWER_DBM		 = NA_DOUBLE;		
	g_RecordedParam.FRAME_COUNT      = NA_INTEGER;	
	g_RecordedParam.BANDWIDTH[0]	 = '\0';
	g_RecordedParam.DATA_RATE[0]     = '\0';
	g_RecordedParam.PREAMBLE[0]      = '\0';
	g_RecordedParam.PACKET_FORMAT[0] = '\0';

	return 0;
}

int AverageTestResult(double *resultArray, int averageTimes, int logType, double &averageResult, double &maxResult, double &minResult)
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
        else if (logType == Linear_Sum)
        {
            logType = LOG_10;
                        
            for (i=0;i<averageTimes;i++)    
			{
                //TODO: avoid unenabled VSA or streams
                    maxResult = max(maxResult, resultArray[i]);
		            minResult = min(minResult, resultArray[i]);
				    averageResult = averageResult + pow( 10,(resultArray[i]/logType) );

			}
            
		    if (0!=averageResult)
		    {
			    averageResult = logType*(log10(averageResult));
		    }
		    else
		    {
			    averageResult = NA_DOUBLE;
		    }

        }
        else if (logType == LOG_20_EVM)		// for EVM average
        {
			logType = LOG_20;

            for (i=0;i<averageTimes;i++)    
			{
                maxResult = max(maxResult, resultArray[i]);
	            minResult = min(minResult, resultArray[i]);
			    averageResult = averageResult + pow(pow( 10,(resultArray[i]/logType)), 2);
			}
            averageResult = sqrt(averageResult / averageTimes);
			
		    if (0!=averageResult)
		    {
			    averageResult = logType*(log10(averageResult));
		    }
		    else
		    {
			    averageResult = NA_DOUBLE;
		    }
        }
		else // LOG_10 and LOG_20
		{
			for (i=0;i<averageTimes;i++)    
			{
				maxResult = max(maxResult, resultArray[i]);
				minResult = min(minResult, resultArray[i]);
				averageResult = averageResult + pow( 10,(resultArray[i]/logType) );
			}
			averageResult = averageResult / averageTimes;


			if (0!=averageResult)
			{
				averageResult = logType*(log10(averageResult));
			}
			else
			{
				averageResult = NA_DOUBLE;
			}
		}


	}
    return err;
}
WIFI_11AC_MIMO_TEST_API int  RespondToQueryInput( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap)
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

	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] RespondToQueryInput() return OK.\n");

    return err;
}


int  RespondToQueryReturn( map<string, WIFI_SETTING_STRUCT>& returnMap)
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

	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] RespondToQueryReturn() return OK.\n");

    return err;
}

int  ReturnTestResults( map<string, WIFI_SETTING_STRUCT>& returnMap)
{
    int err = ERR_OK;
	bool putTMReturnResult = false;

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
            if ((*valueDouble)>(NA_NUMBER+1) )
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
				
				//Default doubleArray won't put into TM result container. (for Sweep functiion)
				putTMReturnResult = false;

                for (unsigned int i=0; i<doubleVector.size(); i++)
                {
                    doubleArray[i] = doubleVector[i];
					if(doubleArray[i] >(NA_NUMBER+1))
					{
						putTMReturnResult = true;
					}
					else
					{
						//not put double array into TM result container
					}
                }
				if (putTMReturnResult)
				{
					err = TM_AddArrayDoubleReturn(g_WiFi_Test_ID, (char*)returnMap_Iter->first.c_str(), doubleArray, (int)doubleVector.size());
				}
				else
				{
					//not put double array into TM result container
				}

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

int  ClearReturnParameters( map<string, WIFI_SETTING_STRUCT>& returnMap)
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
			*valueInteger = NA_INTEGER;
        }
        else if(returnMap_Iter->second.type==WIFI_SETTING_TYPE_DOUBLE)
        {
			double *valueDouble = reinterpret_cast<double *>(returnMap_Iter->second.value);
            *valueDouble = NA_DOUBLE;
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

int  ReturnErrorMessage(char *errParameter, const char *format, ...)
{
    int err = ERR_OK;

    // Log message format ... variable argument list
    va_list ap;
    va_start(ap, format);

    vsprintf_s(errParameter, MAX_BUFFER_SIZE, format, ap);
    err = ::TM_AddStringReturn(g_WiFi_Test_ID, "ERROR_MESSAGE", errParameter);

    va_end(ap);

    return err;
}

int  LogReturnMessage(char *errMsg, int sizeOfBuf, LOGGER_LEVEL level, const char *format, ...)
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
		sprintf_s(errMsg, sizeOfBuf, "[WiFi_11ac_MiMo] LogReturnMessage() return error, Logger_ID < 0.\n");
	}

    return err;
}

int  GetInputParameters( map<string, WIFI_SETTING_STRUCT>& inputMap)
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

int  CheckDutStatus(void)
{
    int err = ERR_OK;
	int	ant01 = 0, ant02 = 0, ant03 = 0, ant04 = 0;
	char vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	::TM_GetIntegerParameter(g_WiFi_Test_ID, "TX1", &ant01);
	::TM_GetIntegerParameter(g_WiFi_Test_ID, "TX2", &ant02);
	::TM_GetIntegerParameter(g_WiFi_Test_ID, "TX3", &ant03);
	::TM_GetIntegerParameter(g_WiFi_Test_ID, "TX4", &ant04);

	if ( (ant01+ant02+ant03+ant04)>0 )	// This is a Tx function
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] vDUT_Run(TX_STOP) return error.\n");
					return err;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] vDUT_Run(TX_STOP) return OK.\n");
			}

		}
		else
		{
			// do nothing
		}
	}

    return err;
}
int CheckTesterConsistentStatus( std::map<std::string, std::string> &versionMap)
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


int  WiFi_11ac_TestMode(char* strDataRate, int* cbw, int* wifiMode, int* wifiStreamNum, char* pktFormat)
{
    int    err = ERR_OK;
    int    dataRateIndex, dataRatePBCC22, dataRateOFDM6, dataRateOFDM54, dataRateMCS7, dataRateMCS15, dataRateMCS23; 


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
	
	// changed original logic
	
	if (strstr (pktFormat, PACKET_FORMAT_VHT))									// if pktFormat is the same as "VHT" , then its 11AC_VHT
	{  
		if ( *cbw == BW_20MHZ)			*wifiMode = WIFI_11AC_VHT20;
		else if (*cbw == BW_40MHZ)		*wifiMode = WIFI_11AC_VHT40;
		else if (*cbw == BW_80MHZ)		*wifiMode = WIFI_11AC_VHT80;
		else if (*cbw == BW_160MHZ)		*wifiMode = WIFI_11AC_VHT160;
		else							*wifiMode = WIFI_11AC_VHT80_80;

	}
	else																			//Non 11AC_VHT
	{
		if ( dataRateIndex<=dataRatePBCC22 )										// [Case 01]: 802.11b
		{   
			*wifiMode = WIFI_11B;
			*wifiStreamNum = WIFI_ONE_STREAM;
		}
		else if ( (dataRateIndex>=dataRateOFDM6)&&(dataRateIndex<=dataRateOFDM54) )	// [Case 02]: 802.11a/g 
		{   
			*wifiMode = WIFI_11AG;
			*wifiStreamNum = WIFI_ONE_STREAM;
		}
		else																		// [Case 03]: 802.11n
		{
			if ( strstr(pktFormat,PACKET_FORMAT_HT_MF))								//1) "11N_MF_HT"
			{
				if (*cbw == BW_20MHZ)			
					*wifiMode = WIFI_11N_MF_HT20;
				else							
					*wifiMode = WIFI_11N_MF_HT40;
			}
			else																	//2) "11N_GF_HT"
			{
				if (*cbw == BW_20MHZ)			
					*wifiMode = WIFI_11N_GF_HT20;
				else							
					*wifiMode = WIFI_11N_GF_HT40;
			}

			if (dataRateIndex<=dataRateMCS7)       
				*wifiStreamNum = WIFI_ONE_STREAM;
			else if (dataRateIndex<=dataRateMCS15) 
				*wifiStreamNum = WIFI_TWO_STREAM;
			else if (dataRateIndex<=dataRateMCS23) 
				*wifiStreamNum = WIFI_THREE_STREAM; 
			else								   
				*wifiStreamNum = WIFI_FOUR_STREAM;
		}
	}

	//if ( !strstr ( pktFormat, "11AC"))
	//{  

	//	if ( dataRateIndex<=dataRatePBCC22 ) 
	//	{   //  [Case 01]: 802.11b
	//		*wifiMode = WIFI_11B;
	//		*wifiStreamNum = WIFI_ONE_STREAM;
	//	}
	//	else if ( (dataRateIndex>=dataRateOFDM6)&&(dataRateIndex<=dataRateOFDM54) ) 
	//	{   //  [Case 02]: 802.11a/g
	//		*wifiMode = WIFI_11AG;
	//		*wifiStreamNum = WIFI_ONE_STREAM;
	//	}
	//	else // [Case 03]: 802.11n
	//	{
	//		if ( strstr(pktFormat,PACKET_FORMAT_HT_MF)) // originally, it's the 11N_MF_HT
	//		{
	//			if (*cbw == BW_20MHZ)			*wifiMode = WIFI_11N_MF_HT20;
	//			else							*wifiMode = WIFI_11N_MF_HT40;
	//		}
	//		else //"11N_GF_HT"
	//		{
	//			if (*cbw == BW_20MHZ)			*wifiMode = WIFI_11N_GF_HT20;
	//			else							*wifiMode = WIFI_11N_GF_HT40;
	//		}
	//		

	//		if (dataRateIndex<=dataRateMCS7)       *wifiStreamNum = WIFI_ONE_STREAM;
	//		else if (dataRateIndex<=dataRateMCS15) *wifiStreamNum = WIFI_TWO_STREAM;
	//		else if (dataRateIndex<=dataRateMCS23) *wifiStreamNum = WIFI_THREE_STREAM; 
	//		else								   *wifiStreamNum = WIFI_FOUR_STREAM;
	//	}
	//}
	//else   //11AC:  VHT, HT_MF, HT_GF, NON_HT
	//{
	//	if(strstr(pktFormat, PACKET_FORMAT_VHT))  // VHT
	//	{
	//		if ( *cbw == BW_20MHZ)			*wifiMode = WIFI_11AC_VHT20;
	//		else if (*cbw == BW_40MHZ)		*wifiMode = WIFI_11AC_VHT40;
	//		else if (*cbw == BW_80MHZ)		*wifiMode = WIFI_11AC_VHT80;
	//		else if (*cbw == BW_160MHZ)		*wifiMode = WIFI_11AC_VHT160;
	//		else							*wifiMode = WIFI_11AC_VHT80_80;

	//	}
	//	else if( strstr(pktFormat, PACKET_FORMAT_HT_MF))
	//	{
	//		if (*cbw == BW_20MHZ)			*wifiMode = WIFI_11AC_MF_HT20;
	//		else							*wifiMode = WIFI_11AC_MF_HT40;

	//		if (dataRateIndex<=dataRateMCS7)       *wifiStreamNum = WIFI_ONE_STREAM;
	//		else if (dataRateIndex<=dataRateMCS15) *wifiStreamNum = WIFI_TWO_STREAM;
	//		else if (dataRateIndex<=dataRateMCS23) *wifiStreamNum = WIFI_THREE_STREAM; 
	//		else								   *wifiStreamNum = WIFI_FOUR_STREAM;
	//	}
	//	else if (strstr(pktFormat, PACKET_FORMAT_HT_GF))
	//	{
	//		if (*cbw == BW_20MHZ)			*wifiMode = WIFI_11AC_GF_HT20;
	//		else							*wifiMode = WIFI_11AC_GF_HT40;

	//		if (dataRateIndex<=dataRateMCS7)       *wifiStreamNum = WIFI_ONE_STREAM;
	//		else if (dataRateIndex<=dataRateMCS15) *wifiStreamNum = WIFI_TWO_STREAM;
	//		else if (dataRateIndex<=dataRateMCS23) *wifiStreamNum = WIFI_THREE_STREAM; 
	//		else								   *wifiStreamNum = WIFI_FOUR_STREAM;
	//	}
	//	else
	//	{
	//		*wifiMode = WIFI_11AC_NON_HT;

	//		*wifiStreamNum = WIFI_ONE_STREAM;
	//	}
	//}
    return err;
}

WIFI_11AC_MIMO_TEST_API int  WiFiTestMode(char* strDataRate, char* bssBandwidth, int* wifiMode, int* wifiStreamNum)
{
    int    err = ERR_OK;
    int    dataRateIndex, dataRatePBCC22, dataRateOFDM6, dataRateOFDM54, dataRateMCS7, dataRateMCS15, dataRateMCS23; 


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
    else // [Case 03]: 802.11n
    {
       /* if ( 0==strcmp(strBandwidth, "HT20") ) *wifiMode = WIFI_11N_HT20;
        else                                   *wifiMode = WIFI_11N_HT40;

        if (dataRateIndex<=dataRateMCS7)       *wifiStreamNum = WIFI_ONE_STREAM;
        else if (dataRateIndex<=dataRateMCS15) *wifiStreamNum = WIFI_TWO_STREAM;
        else if (dataRateIndex<=dataRateMCS23) *wifiStreamNum = WIFI_THREE_STREAM; 
		else								   *wifiStreamNum = WIFI_FOUR_STREAM;*/
    }
    
    return err;
}

int  WiFiSaveSigFile(char* fileName)
{
    int    err = ERR_OK;

	char    logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char    logInfoMessage[MAX_BUFFER_SIZE] = {'\0'};

	if ( (1==g_globalSettingParam.VSA_SAVE_CAPTURE_ON_FAILED)||(1==g_globalSettingParam.VSA_SAVE_CAPTURE_ALWAYS))  
	{	  
		//Get system time
		//CTime loggerTime = CTime::GetCurrentTime();   
		//CString cStr = loggerTime.Format(_T("%Y.%B.%d-%H.%M.%S"));

		char c_time[MAX_BUFFER_SIZE], c_path[MAX_BUFFER_SIZE];
		//size_t i;
		//int sizeOfString = cStr.GetLength() + 1; 
		//LPTSTR pStrTmp = new TCHAR[sizeOfString];
		//_tcscpy_s( pStrTmp, sizeOfString, cStr );
		//wcstombs_s( &i, c_time, sizeOfString , pStrTmp, sizeOfString );
		//delete pStrTmp;

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
	    
		//sprintf_s(c_path, MAX_BUFFER_SIZE, "./log/%s-%s.sig", fileName, c_time);
		//to do need to save based on tester type when we merge them to one trunk. 
		sprintf_s(c_path, MAX_BUFFER_SIZE, "./log/%s-%s.iqvsa", fileName, c_time);

		err = ::LP_SaveVsaSignalFile(c_path);

		sprintf_s(logInfoMessage, "[WiFi_11ac_MiMo] WiFiSaveSigFile(\"%s\") return OK.\n", c_path);

		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, logInfoMessage);
	}
	else
	{
		// do nothing...
	}

	return err;
}

int GetDefaultWaveformFileName(char* filePath,char* fileType,int wifiMode,  int streamNum_11AC,int chBW,
							char* datarate, char* preamble, char* packetFormat, char* guardInterval, 
							 char* waveformFileName, int bufferSize)
 {
	 int err = ERR_OK;
	 char tempWaveformFileName[MAX_BUFFER_SIZE] = {'\0'};
	 string sGuardInterval = (strcmp( guardInterval, "SHORT") == 0 ? "_S_GI_" : "_");
	 string sFileType = fileType;

	 //if Tester is IQ2010, change the waveform filename extension from .iqvsg to .mod 
	 if( g_iTesterName == IQTYPE_2010 )
	 {
		 if(sFileType == "iqvsg")
			 fileType = "mod";
		 else if(sFileType == "iqref")
			 fileType = "ref";
	 }

	 if ( wifiMode == WIFI_11B)		// 802.11b
	 {
		 if ( 0 == strcmp( datarate, "DSSS-1"))
		 {
			 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_%s.%s", datarate, fileType);
		 }
		 else if ( (strstr(datarate, "DSSS")!=NULL)||(strstr(datarate, "CCK")!=NULL) )
		 {
			 char tmpPreamble[2]; 
			 tmpPreamble[0] = preamble[0];
			 tmpPreamble[1] = '\0';
			 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_%s%s.%s", datarate, tmpPreamble, fileType);
		 }
		 else
		 {
			 // Wrong data rate for 802.11b
			 err = -1;
			 return err;
		 }

		 if ( strlen( g_globalSettingParam.PER_WAVEFORM_PREFIX_11B))
		 {
			 sprintf_s ( waveformFileName, MAX_BUFFER_SIZE, "%s/%s_%s",filePath,g_globalSettingParam.PER_WAVEFORM_PREFIX_11B,tempWaveformFileName);
		 }
		 else
		 {
			 sprintf_s( waveformFileName, MAX_BUFFER_SIZE, "%s/%s",filePath,tempWaveformFileName);
		 }
	 }
	 else if ( wifiMode ==  WIFI_11AG)		//802.11ag
	 {
		 if ( (strstr(datarate, "OFDM")!=NULL) )
		 {
			 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_%s.%s", datarate, fileType);
		 }
		 else
		 {
			 // Wrong data rate for 802.11ag
			 err = -1;
			 return err;
		 }

		 if ( strlen(g_globalSettingParam.PER_WAVEFORM_PREFIX_11AG))
		 {
			 sprintf_s ( waveformFileName, MAX_BUFFER_SIZE, "%s/%s_%s",filePath,g_globalSettingParam.PER_WAVEFORM_PREFIX_11AG,tempWaveformFileName);
		 }
		 else
		 {
			 sprintf_s( waveformFileName, MAX_BUFFER_SIZE, "%s/%s",filePath,tempWaveformFileName);
		 }

	 }
	 //else if ( strstr ( packetFormat, "11N"))		//802.11n
	 //{
		// if ( wifiMode == WIFI_11N_MF_HT20)
		// {
		//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_HT20%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
		// }
		// else if ( wifiMode == WIFI_11N_MF_HT40)
		// {
		//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_HT40%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
		// }
		// else if ( wifiMode == WIFI_11N_GF_HT20)
		// {
		//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_GF_HT20%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
		// }
		// else if ( wifiMode == WIFI_11N_GF_HT40)
		// {
		//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_GF_HT40%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
		// }
		// else
		// {
		//	 // Wrong data rate for 802.11n
		//	 err = -1;
		//	 return err;
		// }

		// if ( strlen(g_globalSettingParam.PER_WAVEFORM_PREFIX_11N))
		// {
		//	 sprintf_s ( waveformFileName, MAX_BUFFER_SIZE, "%s/%s_%s",filePath,g_globalSettingParam.PER_WAVEFORM_PREFIX_11N,tempWaveformFileName);
		// }
		// else
		// {
		//	 sprintf_s( waveformFileName, MAX_BUFFER_SIZE, "%s/%s",filePath,tempWaveformFileName);
		// }
	 //}
	 //else		//802.11ac
	 //{
	 else if ( strstr ( packetFormat, PACKET_FORMAT_VHT) && 
		                strstr ( datarate, "MCS") &&
			            streamNum_11AC >0 )   // 802.11ac VHT, MCS0 ~MCS9   		//802.11ac
		 {
			 if ( chBW == BW_20MHZ)
			 {
				 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_11AC_%s20_S%d%s%s.%s", packetFormat,streamNum_11AC, sGuardInterval.c_str(), datarate, fileType);
			 }
			 else if (chBW == BW_40MHZ)
			 {
				 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_11AC_%s40_S%d%s%s.%s", packetFormat,streamNum_11AC, sGuardInterval.c_str(), datarate, fileType);
			 }
			 else if (chBW == BW_80MHZ)
			 {
				 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_11AC_%s80_S%d%s%s.%s", packetFormat,streamNum_11AC, sGuardInterval.c_str(), datarate, fileType);
			 }
			 else if (chBW == BW_160MHZ)
			 {
				 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_11AC_%s160_S%d%s%s.%s", packetFormat,streamNum_11AC, sGuardInterval.c_str(), datarate, fileType);
			 }
			 else if (chBW == BW_80_80MHZ)
			 {
				 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_11AC_%s80_80_S%d%s%s.%s", packetFormat,streamNum_11AC, sGuardInterval.c_str(), datarate, fileType);
			 }
			// else
			// {
			//	 err = -1;
			//	 return err;
			// }
		 //}
		 //else if (strstr (packetFormat,"11AC_MF_HT")  && strstr(datarate, "MCS")!=NULL) //11ac HT_MF, same waveform name as 802.11n
		 //{
			// if ( chBW == BW_20MHZ)
			// {
			//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_HT20%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
			// }
			// else if ( chBW == BW_40MHZ)
			// {
			//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_HT40%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
			// }
			// else
			// {
			//	 err = -1;
			//	 return err;
		 //	 }
		 //}
		 //else if (strstr (packetFormat,"11AC_GF_HT")  && (strstr(datarate, "MCS")!=NULL)) //11ac HT_GF, same waveform name as 802.11n
		 //{
			// if ( chBW == BW_20MHZ)
			// {
			//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_GF_HT20%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
			// }
			// else if ( chBW == BW_40MHZ)
			// {
			//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_GF_HT40%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
			// }
			// else
			// {
			//	 err = -1;
			//	 return err;
		 //	 }
		 //}
		 //else if ( strstr(packetFormat ,"11AC_NON_HT") && strstr( datarate, "OFDM")) //WIFI_11AC_NON_HT
		 //{
			// if (chBW == BW_40MHZ)
			// {
			//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_%s40_%s.%s", packetFormat, datarate, fileType);
			// }
			// else if (chBW == BW_80MHZ)
			// {
			//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_%s80_%s.%s", packetFormat, datarate, fileType);
			// }
			// else if (chBW == BW_160MHZ)
			// {
			//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_%s160_%s.%s", packetFormat, datarate, fileType);
			// }
			// else if (chBW == BW_80_80MHZ)
			// {
			//	 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_%s80_80_%s.%s", packetFormat, datarate, fileType);
			// }
			// else      // BW != 20MHz
			// {
			//	 err = -1;
			//	 return err;
			// }
		
			 //}
		 else     //wrong wifiMode
		 {
			 err = -1;
			 return err;
		 }
		 if ( strlen(g_globalSettingParam.PER_WAVEFORM_PREFIX_11AC))
		 {
			 sprintf_s ( waveformFileName, MAX_BUFFER_SIZE, "%s/%s_%s",filePath,g_globalSettingParam.PER_WAVEFORM_PREFIX_11AC,tempWaveformFileName);
		 }
		 else
		 {
			 sprintf_s( waveformFileName, MAX_BUFFER_SIZE, "%s/%s",filePath,tempWaveformFileName);
		 }
	 }
		
	 else 
	 {
		 if (strstr (packetFormat,PACKET_FORMAT_HT_MF)  && strstr(datarate, "MCS")!=NULL) //11ac HT_MF, same waveform name as 802.11n
		 {
			 if ( chBW == BW_20MHZ)
			 {
				  sprintf_s(tempWaveformFileName, bufferSize, "WiFi_HT20%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
			 }
			 else if ( chBW == BW_40MHZ)
			 {
				 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_HT40%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
			 }
			 else
			 {
				 err = -1;
				 return err;
			 }
		 }
		 else if (strstr (packetFormat,PACKET_FORMAT_HT_GF)  && (strstr(datarate, "MCS")!=NULL)) //11ac HT_GF, same waveform name as 802.11n
		 {
			 if ( chBW == BW_20MHZ)
			 {
				 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_GF_HT20%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
			 }
			 else if ( chBW == BW_40MHZ)
			 {
				 sprintf_s(tempWaveformFileName, bufferSize, "WiFi_GF_HT40%s%s.%s", sGuardInterval.c_str(), datarate, fileType);
			 }
			 else
			 {
				 err = -1;
				 return err;
			 }
		 }
		 else     //wrong wifiMode
		 {
			 err = -1;
			 return err;
		 }
		 if ( strlen(g_globalSettingParam.PER_WAVEFORM_PREFIX_11N))
		 {
			 sprintf_s ( waveformFileName, MAX_BUFFER_SIZE, "%s/%s_%s",filePath,g_globalSettingParam.PER_WAVEFORM_PREFIX_11N,tempWaveformFileName);
		 }
		 else
		 {
			 sprintf_s( waveformFileName, MAX_BUFFER_SIZE, "%s/%s",filePath,tempWaveformFileName);
		 }
		
	 }


	 // Wave file checking
	FILE *waveFile;
	fopen_s(&waveFile, waveformFileName, "r");
	if (!waveFile)
	{
		return -1;
	}
	else
	{
		fclose(waveFile);
		waveFile = NULL;
		err = 0;
	}


	 return err;
 }




int  GetWaveformFileName(char* perfix, char* postfix, int wifiMode, char* bandwidth, char* datarate, 
						 char* preamble, char* packetFormat, char* waveformFileName, int bufferSize)
{
    int  err = ERR_OK;

	char keyword[MAX_BUFFER_SIZE] = {'\0'}, datarateString[MAX_BUFFER_SIZE] = {'\0'};
	int  rateIndexBegin = 0, rateIndexEnd = 0;

	map<string, WIFI_SETTING_STRUCT>::iterator inputMap_Iter;

	// Find out the datarate index
/*	if ( (wifiMode==WIFI_11B)||(wifiMode==WIFI_11AG) )
	{
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
		sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_11AG_%s_%s", perfix, datarateString, postfix);
	}
	else if (( wifiMode ==  WIFI_11N_MF_HT20) || ( wifiMode ==  WIFI_11N_MF_HT40))
	{		
		sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_11N_%s_MIXED_%s_%s", perfix, bandwidth, datarate, postfix);			
	}
	else if (( wifiMode ==  WIFI_11N_GF_HT20) || ( wifiMode ==  WIFI_11N_GF_HT40))
	{		
		sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_11N_%s_GREENFIELD_%s_%s", perfix, bandwidth, datarate, postfix);			
	}
	else		// 802.11ac
	{
		if ((strstr (packetFormat,PACKET_FORMAT_VHT )) && (strstr(datarate, "MCS")!=NULL) &&
			(streamNum_11AC >0)) //11ac VHT
		{
			if ( chBW == BW_20MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s20_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_40MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s40_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_160MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s160_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_80_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_80_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else
			{
				return -1;
			}
			
		}
		else if ((strstr (packetFormat,PACKET_FORMAT_HT_MF) || strstr (packetFormat,PACKET_FORMAT_HT_GF))
			&& (strstr(datarate, "MCS")!=NULL)) //11ac HT
		{
			if ( chBW == BW_20MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_HT20_%s.%s", waveFilePath, datarate, fileType);
			}
			else if ( chBW == BW_40MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_HT40_%s.%s", waveFilePath, datarate, fileType);
			}
			else
			{
				return -1;
			}
		}
		else if ( strstr(packetFormat ,PACKET_FORMAT_NON_HT)) //WIFI_11AC_NON_HT
		{
			if (chBW == BW_40MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s40_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else if (chBW == BW_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else if (chBW == BW_160MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s160_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else if (chBW == BW_80_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_80_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else      // BW != 20MHz
			{
				return -1;
			}
		
		}
		else     //wrong wifiMode
		{
			return -1;
		}
	}

    // Searching the keyword in g_globalSettingParamMap
	inputMap_Iter = g_globalSettingParamMap.find(keyword);
    if( inputMap_Iter!=g_globalSettingParamMap.end() )
    {
		char dummyString[MAX_BUFFER_SIZE];
		strcpy_s (dummyString, MAX_BUFFER_SIZE, (char*)inputMap_Iter->second.value);

		if ( strlen(g_defaultFilePath)>0 )		// default waveform file path not empty
		{
			sprintf_s(waveformFileName, bufferSize, "%s/%s", g_defaultFilePath, dummyString);
		}
		else	// default waveform file path is empty, then using g_globalSettingParam.PER_WAVEFORM_PATH
		{
			sprintf_s(waveformFileName, bufferSize, "%s/%s", g_globalSettingParam.PER_WAVEFORM_PATH, dummyString);
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
					sprintf_s(g_defaultFilePath, bufferSize, "../%s", g_globalSettingParam.PER_WAVEFORM_PATH);
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
    }*/

	return err;
}


/*int  GetWaveformFileName(char* filePath, char* fileType, int streamNum_11AC, int chBW, char* datarate, 
						 char* preamble, char* packetFormat, char* waveformFileName, int bufferSize)
{
    int  err = -1;
	char waveFilePath[MAX_BUFFER_SIZE] = {'\0'};


	::vDUT_ClearReturns(g_LP_mimo_id);	

	if ( strlen(filePath)>0 )		// user defined path is not empty
	{
		sprintf_s(waveFilePath, MAX_BUFFER_SIZE, "%s", filePath);
	}
	else if ( strlen(g_defaultFilePath)>0 )		// default waveform file path not empty
	{
		sprintf_s(waveFilePath, MAX_BUFFER_SIZE, "%s", g_defaultFilePath);
	}
	else	// both are empty
	{
		sprintf_s(waveFilePath, MAX_BUFFER_SIZE, "../Mod");
	}

	if ( !strstr(packetFormat, "11AC")) //Legacy mode
	{

		if ( 0==strcmp(datarate, "DSSS-1") )
		{
			sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s.%s", waveFilePath, datarate, fileType);
		}
		else if ( (strstr(datarate, "DSSS")!=NULL)||(strstr(datarate, "CCK")!=NULL) )
		{
			char tmpPreamble[2]; 
			tmpPreamble[0] = preamble[0];
			tmpPreamble[1] = '\0';
			sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s%s.%s", waveFilePath, datarate, tmpPreamble, fileType);
		}
		else if ( (strstr(datarate, "OFDM")!=NULL) )
		{
			sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s.%s", waveFilePath, datarate, fileType);
		}
		else if ( (strstr(datarate, "MCS")!=NULL) )
		{
			if (chBW == BW_20MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s_%s.%s", waveFilePath, "HT20", datarate, fileType);
			}
			else
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s_%s.%s", waveFilePath, "HT40", datarate, fileType);
			}
		}
		else
		{
			// do nothing
			return -1;
		}
	}
	else    // 11AC mode
	{
		if ((strstr (packetFormat,PACKET_FORMAT_VHT )) && (strstr(datarate, "MCS")!=NULL) &&
			(streamNum_11AC >0)) //11ac VHT
		{
			if ( chBW == BW_20MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s20_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_40MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s40_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_160MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s160_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_80_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_80_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else
			{
				return -1;
			}
			
		}
		else if ((strstr (packetFormat,PACKET_FORMAT_HT_MF) || strstr (packetFormat,PACKET_FORMAT_HT_GF))
			&& (strstr(datarate, "MCS")!=NULL)) //11ac HT
		{
			if ( chBW == BW_20MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_HT20_%s.%s", waveFilePath, datarate, fileType);
			}
			else if ( chBW == BW_40MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_HT40_%s.%s", waveFilePath, datarate, fileType);
			}
			else
			{
				return -1;
			}
		}
		else if ( strstr(packetFormat ,PACKET_FORMAT_NON_HT)) //WIFI_11AC_NON_HT
		{
			if (chBW == BW_40MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s40_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else if (chBW == BW_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else if (chBW == BW_160MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s160_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else if (chBW == BW_80_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_80_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else      // BW != 20MHz
			{
				return -1;
			}
		
		}
		else     //wrong wifiMode
		{
			return -1;
		}

	}


	// Wave file checking
	FILE *waveFile;
	fopen_s(&waveFile, waveformFileName, "r");
	if (!waveFile)
	{
		if ( strlen(g_defaultFilePath)>0 )		// Default waveform file path not empty, but still can't find the waveformfile, then return error.
		{
			return -1;	//TM_ERR_PARAM_DOES_NOT_EXIST
		}
		else	// Try to find the file in upper folder
		{
			char tempPath[MAX_BUFFER_SIZE] = {'\0'};
			sprintf_s(tempPath, bufferSize, "../%s", waveformFileName);

			fopen_s(&waveFile, tempPath, "r");
			if (!waveFile)
			{
				return -1;	//TM_ERR_PARAM_DOES_NOT_EXIST
			}
			else
			{
				// Save the path to default path name
				sprintf_s(g_defaultFilePath, bufferSize, "../%s", waveFilePath);
				// And using the new path
				sprintf_s(waveformFileName, bufferSize, "%s", tempPath);

				fclose(waveFile);
				waveFile = NULL;
				err = 0;
			}
		}
	}
	else
	{
		fclose(waveFile);
		waveFile = NULL;
		err = 0;
	}

	return err;
}*/

int  GetPacketNumber(int wifiMode,char* packetFormat, int *packetNumber)
{
    int  err = ERR_OK;

	if ( wifiMode == WIFI_11B)
	{
		*packetNumber = g_globalSettingParam.PER_FRAME_COUNT_11B;
	}
	else if ( wifiMode == WIFI_11AG)
	{
		*packetNumber = g_globalSettingParam.PER_FRAME_COUNT_11AG;
	}
	else if ( strstr (packetFormat, PACKET_FORMAT_VHT))  
	{
		*packetNumber = g_globalSettingParam.PER_FRAME_COUNT_11AC;
	}
	else
	{
		*packetNumber = g_globalSettingParam.PER_FRAME_COUNT_11N;
	}
	//else if ( strstr (packetFormat, "11N"))
	//{
	//	*packetNumber = g_globalSettingParam.PER_FRAME_COUNT_11N;
	//}
	//else
	//{
	//	*packetNumber = g_globalSettingParam.PER_FRAME_COUNT_11AC;
	//}

	return err;
}

int CheckPathLossTable(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb)
{
	int  err = ERR_OK;

	err = CheckPathLossTableExt(testID, freqMHz, ant01, ant02, ant03, ant04, cableLoss, cableLossReturn, cableLossDb, TX_TABLE);

	return err;

}
int CheckPathLossTableExt(int testID, int freqMHz, int ant01, int ant02, int ant03, int ant04, double *cableLoss, double *cableLossReturn, double *cableLossDb, int indicatorTxRx)
{
	int  err = ERR_OK;

	try
	{
		int ant[4] = { ant01, ant02, ant03, ant04 };
		
		for ( int i=0;i<MAX_DATA_STREAM;i++)
		{
			//if ( ant[i] )	// Path enabled 
			//{
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
			//}
			//else	// This path not enabled
			//{
			//	cableLoss[i]	   = 0;
			//	cableLossReturn[i] = 0;
			//}
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


double CalcCableLossDb(int ant1, int ant2, int ant3, int ant4, double cableLoss1, double cableLoss2, double cableLoss3, double cableLoss4)
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

WIFI_11AC_MIMO_TEST_API int GetWiFiTestID(TM_ID* wifiTestID)
{
	*wifiTestID = g_WiFi_Test_ID;
    return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int SetWiFiTestID(TM_ID wifiTestID)
{
	g_WiFi_Test_ID = wifiTestID;
	return ERR_OK;
}


WIFI_11AC_MIMO_TEST_API int   GetWiFiDutID(vDUT_ID* wifiDutID)
{
    *wifiDutID = g_WiFi_Dut;
    return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   GetWiFiGlobalSetting(WIFI_GLOBAL_SETTING** wifiGlobleSetting)
{
    *wifiGlobleSetting = &g_globalSettingParam;
    return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   GetWiFiRecordParam(WIFI_RECORD_PARAM** wifiRecordParam)
{
	*wifiRecordParam = &g_RecordedParam;
	return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   SetTesterNumber(int testerNumber)
{
    g_Tester_Number = testerNumber;
    return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   GetTesterNumber(int *testerNumber)
{
    *testerNumber = g_Tester_Number;
    return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   SetTesterType(int testerType)
{
    g_Tester_Type = testerType;
    return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   GetTesterType(int *testerType)
{
    *testerType = g_Tester_Type;
    return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   GetTesterName(int *flag)
{
	*flag = g_iTesterName;
	return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   SetTesterName(int flag)
{
	g_iTesterName = flag;
	return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   SetTesterReconnect(int flag)
{
    g_Tester_Reconnect = flag;
    return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   GetTesterReconnect(int *flag)
{
    *flag = g_Tester_Reconnect;
    return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   SetDutConfigChanged(bool changed)
{
    g_dutConfigChanged = changed;
    return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int   GetDutConfigChanged(bool *changed)
{
	*changed = g_dutConfigChanged;
	return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int GetDutTxActived(bool* vDutTxActived)
{
	*vDutTxActived = g_vDutTxActived;
    return ERR_OK;
}

WIFI_11AC_MIMO_TEST_API int SetDutTxActived(bool vDutTxActived)
{
	g_vDutTxActived = vDutTxActived;
    return ERR_OK;
}
int  CheckConnectionVsaMasterAndAntennaPort( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap)
{
    int err = 0;
    char vsaValue[MAX_BUFFER_SIZE] ={'\0'};
    int txEnabledCondition = 0;
    int *integerValue;
    std::map<std::string, WIFI_SETTING_STRUCT>::iterator  inputMapIter;
    
    inputMapIter = inputMap.find("VSA1_CONNECTION");
   
    if(inputMapIter != inputMap.end())
    {
        if(!strcmp((char *)inputMapIter->second.value, "OFF"))
        {
            //FIRST CONDITION: VSAMASTER IS OFF
            err = -1;
        }
        else
        {
            sprintf_s(vsaValue, MAX_BUFFER_SIZE, "%s", inputMapIter->second.value);
            if(!strcmp(vsaValue, "TX1")) // match
            {
                inputMapIter = inputMap.find("TX1");
                integerValue = reinterpret_cast<int *>(inputMapIter->second.value);
                txEnabledCondition = *integerValue;
            }
            else if(!strcmp(vsaValue, "TX2"))
            {
                inputMapIter = inputMap.find("TX2");
                integerValue = reinterpret_cast<int *>(inputMapIter->second.value);
                txEnabledCondition = *integerValue;
            }
            else if(!strcmp(vsaValue, "TX3"))
            {
                inputMapIter = inputMap.find("TX3");
                integerValue = reinterpret_cast<int *>(inputMapIter->second.value);
                txEnabledCondition = *integerValue;
            }
            else if(!strcmp(vsaValue, "TX4"))
            {
                inputMapIter = inputMap.find("TX4");
                integerValue = reinterpret_cast<int *>(inputMapIter->second.value);
                txEnabledCondition = *integerValue;
            }
            else
            {                
                txEnabledCondition = 0;
            }

            // 6: IQV_TRIG_TYPE_IF2_NO_CAL
            if(!txEnabledCondition && (6 == g_globalSettingParam.VSA_TRIGGER_TYPE))
            {
                //MEANS: EXISTED CONNECTION BETWEEN VSAMASTER AND ANTENNA PORTS
                err = -1;
            }
            else
            {
                err = 0;
            }
        }
    }
    else
    {
        //CAN'T FIND "VSA1_CONNECTION"
        err = -1; 
    }
    
    return err;
}

int CheckConnectionVsaAndAntennaPort( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap, int *vsaMappingTx, int *portStatus)
{
    int err = 0;
    char tmpStr[MAX_BUFFER_SIZE];

    std::map<std::string, WIFI_SETTING_STRUCT>::iterator  inputMapIter;

    for(int i=0;i<MAX_TESTER_NUM;i++)//get all valid/unvald tester connection
    {
		//find port status of vsg
		sprintf_s(tmpStr, MAX_BUFFER_SIZE, "VSA%d_PORT", i+1);
		inputMapIter = inputMap.find(tmpStr);

		if(!_stricmp((char *)inputMapIter->second.value, "LEFT"))
		{
			portStatus[i] = PORT_LEFT;
		}
		else if(!_stricmp((char *)inputMapIter->second.value, "RIGHT"))
		{
			portStatus[i] = PORT_RIGHT;
		}
		else
		{
			portStatus[i] = g_globalSettingParam.VSAs_PORT[i];
		}

		//find connection between vsa and antenna port
        sprintf_s(tmpStr, MAX_BUFFER_SIZE, "VSA%d_CONNECTION", i+1);
        inputMapIter = inputMap.find(tmpStr);

        if(inputMapIter != inputMap.end())
        {
			for(int j=0;j<MAX_CHAIN_NUM;j++)
			{
				sprintf_s(tmpStr, MAX_BUFFER_SIZE, "TX%d", j+1);
	
				if(!_stricmp((char *)inputMapIter->second.value, tmpStr))
				{
		             vsaMappingTx[i] = j+1;
					 break;
				}
				else
				{
					continue;
				}
			}

        }
        else
        {
            continue;
        }

    }

    return err;
}

int CheckConnectionVsgAndAntennaPort( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap, int *vsgMappingRx, int *portStatus)
{
    int err = 0;
    char tmpStr[MAX_BUFFER_SIZE];

    std::map<std::string, WIFI_SETTING_STRUCT>::iterator  inputMapIter;
	std::map<std::string, WIFI_SETTING_STRUCT>::iterator  rxChainIter;

    

    for(int i=0;i<MAX_TESTER_NUM;i++)
    {
		//find port status of vsg
		sprintf_s(tmpStr, MAX_BUFFER_SIZE, "VSG%d_PORT", i+1);
		inputMapIter = inputMap.find(tmpStr);

		if(!_stricmp((char *)inputMapIter->second.value, "LEFT"))
		{
			portStatus[i] = PORT_LEFT;
		}
		else if(!_stricmp((char *)inputMapIter->second.value, "RIGHT"))
		{
			portStatus[i] = PORT_RIGHT;
		}
		else
		{
			portStatus[i] = g_globalSettingParam.VSGs_PORT[i];
		}

		//Find connection between vsg and antenna port
        sprintf_s(tmpStr, MAX_BUFFER_SIZE, "VSG%d_CONNECTION", i+1);
        inputMapIter = inputMap.find(tmpStr);

		if(inputMapIter != inputMap.end())
        {
			for(int j=0;j<MAX_CHAIN_NUM;j++)
			{
				sprintf_s(tmpStr, MAX_BUFFER_SIZE, "RX%d", j+1);
	
				if(!_stricmp((char *)inputMapIter->second.value, tmpStr))
				{
					// VSG[i]_CONNECTION = RX[j]
		             vsgMappingRx[i] = j+1;
					 //rxChainIter = inputMap.find(tmpStr);
					 //if(rxChainIter != inputMap.end())
					 //{

						// chainStatus = reinterpret_cast<int *>(rxChainIter->second.value);
						// if(0 == *chainStatus)
						// {
						//	portStatus[i] = PORT_OFF;
						// }
						// else
						// {
						//	 //keep portStatus
						// }

					 //}
					 break;
				}
				else
				{
					continue;
				}
			}

			//how do we know VSG[i]_CONNECTION is set to 
			// - either one of RX1, RX2, RX3 and RX4
			// - or OFF
			// - or something else, which is invalid 
			if( vsgMappingRx[i]>=1 && vsgMappingRx[i] <=4)
			{
				// - either one of RX1, RX2, RX3 and RX4
			}
			else if(!_stricmp((char *)inputMapIter->second.value, "OFF"))
			{
				// We need to "disalbe" RF power of VSG
				vsgMappingRx[i] = 0;
			}
			else
			{
				//TODO: error needs to be generated
				// vsgMappingRx[i] remains -1
			}

        }
        else
        {
            continue;
        }

	
		

    }

    return err;

}
int  PassTestResultToDutInputContainer( map<string, WIFI_SETTING_STRUCT>& returnMap)
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
double CalculateIQtoP(double data_i, double data_q)
{
	return (data_i*data_i + data_q*data_q);
}

int GetChannelList(int bssBW, int bssPrimary, int chPrimary20,int *chPrimary40, int *chPrimary80)
{
	int err = ERR_OK;

	if (chPrimary20 == NA_INTEGER)
	{
		chPrimary20 = NA_INTEGER;
		*chPrimary40 = NA_INTEGER;
		*chPrimary80 = NA_INTEGER;
	}
	else
	{
		switch (bssBW)
		{
		case BW_20MHZ:
			chPrimary20 = bssPrimary;
			*chPrimary40 = NA_INTEGER;
			*chPrimary80 = NA_INTEGER;
			break;
		case BW_40MHZ:
			if (( chPrimary20 != bssPrimary + 10) && (chPrimary20!= bssPrimary - 10))
			{
				chPrimary20 = NA_INTEGER;
				*chPrimary40 = NA_INTEGER;
				*chPrimary80 = NA_INTEGER;
			}
			else
			{
				*chPrimary40 = bssPrimary;
				*chPrimary80 = NA_INTEGER;
			}
		
			break;
		case BW_80MHZ:
			if ((chPrimary20 == bssPrimary - 30) || (chPrimary20 == bssPrimary - 10))

			{
				*chPrimary40 = bssPrimary - 20;
				*chPrimary80 = bssPrimary;
			}
			else if ((chPrimary20 == bssPrimary + 30) || (chPrimary20 == bssPrimary + 10))
			{
				*chPrimary40= bssPrimary + 20;
				*chPrimary80 = bssPrimary;
			}
			else  //Wrong CH_FREQ_MHZ_PRIMARY_20MHz
			{
				chPrimary20 = NA_INTEGER;
				*chPrimary40 = NA_INTEGER;
				*chPrimary80 = NA_INTEGER;
			}
			break;
		case BW_160MHZ:
			if ((chPrimary20 == bssPrimary - 30) ||( chPrimary20 == bssPrimary - 10))
			{
				*chPrimary40 = bssPrimary - 20;
				*chPrimary80 = bssPrimary - 40;
			}
			else if ((chPrimary20 == bssPrimary - 50) || (chPrimary20 == bssPrimary - 70))
			{
				*chPrimary40 = bssPrimary - 60;
				*chPrimary80 = bssPrimary - 40;
			}
			else if ((chPrimary20 == bssPrimary + 50) || (chPrimary20 == bssPrimary + 70))
			{
				*chPrimary40 = bssPrimary + 60;
				*chPrimary80 = bssPrimary + 40;
			}
			else if ((chPrimary20 == bssPrimary + 10) || (chPrimary20 == bssPrimary + 30))
			{
				*chPrimary40 = bssPrimary + 20;
				*chPrimary80 = bssPrimary + 40;
			}
			else  //Wrong CH_FREQ_MHZ_PRIMARY_20MHz
			{
				chPrimary20 = NA_INTEGER;
				*chPrimary40 = NA_INTEGER;
				*chPrimary80 = NA_INTEGER;
			}
			break;
		case BW_80_80MHZ:
			if (( chPrimary20 == bssPrimary + 10) || (chPrimary20 == bssPrimary + 30))  // Primary segment
			{
				*chPrimary40 = bssPrimary + 20;
				*chPrimary80 = bssPrimary;

			}
			else if (( chPrimary20 == bssPrimary - 10) || (chPrimary20 == bssPrimary - 30))  // Primary segment
			{
				*chPrimary40 = bssPrimary - 20;
				*chPrimary80 = bssPrimary;

			}
			else  //Wrong CH_FREQ_MHZ_PRIMARY_20MHz
			{
				chPrimary20 = NA_INTEGER;
				*chPrimary40 = NA_INTEGER;
				*chPrimary80 = NA_INTEGER;
			}
			break;
		default:
				chPrimary20 = NA_INTEGER;
				*chPrimary40 = NA_INTEGER;
				*chPrimary80 = NA_INTEGER;
			break;
		}
	}
	

	if ( (chPrimary20 == NA_INTEGER) &&
		(*chPrimary40 == NA_INTEGER) &&
		(*chPrimary80 == NA_INTEGER))
	{
		err = -1;
	}
	else
	{
		err = ERR_OK;
	}
	return err;
}

int CheckChannelFreq(int bssBW, int cbw, int bssPrimary, int chFreq)
{
	int err = ERR_OK;
	if (cbw != BW_80_80MHZ)
	{
		if (cbw == bssBW)
		{
			if ( chFreq != bssPrimary)
			{
				err = -1;
			}
			else
			{
				err = ERR_OK;
			}
		}
		else if (bssBW > cbw)
		{
			switch (bssBW)
			{
			case BW_20MHZ:
				if (chFreq != bssPrimary)
				{
					err = -1;
				}
				else
				{
					err = ERR_OK;
				}
				break;
			case BW_40MHZ:  //cbw = BW_20MHZ
				if ((chFreq == bssPrimary - 10) ||
					(chFreq == bssPrimary + 10))
				{
					err = ERR_OK;
				}
				else
				{
					err = -1;
				}
				break;
			case BW_80MHZ:  //cbw = BW_20,40MHz
				if (cbw == BW_20MHZ)
				{
					if ((chFreq == bssPrimary - 10) || (chFreq == bssPrimary - 30) ||
						(chFreq == bssPrimary + 10) || (chFreq == bssPrimary + 30))
					{
						err  = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else if (cbw == BW_40MHZ)
				{
					if ((chFreq == bssPrimary - 20) || (chFreq == bssPrimary + 20))
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else
				{
					err = -1;
				}
				break;
			case BW_160MHZ:  //cbw = BW_20,40,80MHz
				if (cbw == BW_20MHZ)
				{
					if ((chFreq == bssPrimary -10) || (chFreq == bssPrimary - 30) || ( chFreq == bssPrimary -50) ||
						(chFreq == bssPrimary - 70) || (chFreq == bssPrimary + 10) || (chFreq == bssPrimary + 30) ||
						(chFreq == bssPrimary + 50) || (chFreq == bssPrimary + 70))
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}

				}
				else if (cbw == BW_40MHZ)
				{
					if ((chFreq == bssPrimary - 20) || (chFreq == bssPrimary - 60) ||
						(chFreq == bssPrimary + 20) || (chFreq == bssPrimary+60))
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else if (cbw == BW_80MHZ)
				{
					if ((chFreq == bssPrimary + 40) || ( chFreq == bssPrimary - 40))
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else 
				{
					err = -1;
				}
				break;
			default:
				break;
			}
		}
	}
	else  //BW_80_80MHZ, use BSS primary and secondary to caputure signal
	{
		// do nothing
	}

	return err;
}
int CheckChPrimary20(int bssBW, int cbw, int bssPrimary, int chFreq, int chPrimary20)
{
	int err = ERR_OK;

    if (( cbw > bssBW) || (bssPrimary <= 0) || (chFreq <= 0) || (chPrimary20 <= 0))
	{
		err = -1;
	}
	else
	{
		switch (bssBW)
		{
			case BW_20MHZ:  //cbw = BW_20MHZ
				if ( chPrimary20 == chFreq)
				{
					err = ERR_OK;
				}
				else
				{
					err = -1;
				}
				break;
			case BW_40MHZ:  //cbw = BW_20,40MHz
				if (cbw == BW_20MHZ)
				{
					if (chPrimary20 == chFreq )
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else if ( cbw == BW_40MHZ)
				{
					if ((chPrimary20 == bssPrimary -10) || (chPrimary20 == bssPrimary +10))
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else
				{
					err = -1;
				}
				break;
			case BW_80MHZ:  //cbw = BW_20,40,80MHz
				if (cbw == BW_20MHZ)
				{
					if (chPrimary20 == chFreq )
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else if (cbw == BW_40MHZ)
				{
					if ( chFreq == bssPrimary - 20)
					{
						if ((chPrimary20 == bssPrimary - 10) || (chPrimary20 == bssPrimary - 30))
						{
							err = ERR_OK;
						}
						else
						{
							err = -1;
						}
					}
					else
					{
						if ((chPrimary20 == bssPrimary + 10) || (chPrimary20 == bssPrimary + 30))
						{
							err = ERR_OK;
						}
						else
						{
							err = -1;
						}
					}
				}
				else if (cbw == BW_80MHZ)
				{
					if ((chPrimary20 == bssPrimary + 10) || (chPrimary20 == bssPrimary + 30) ||
						(chPrimary20 == bssPrimary - 10) || (chPrimary20 == bssPrimary - 30))
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else
				{
					err = -1;
				}
				break;
			case BW_160MHZ:
				if (cbw == BW_20MHZ)
				{
					if (chPrimary20 == chFreq )
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else if (cbw == BW_40MHZ)
				{
					if (chFreq == bssPrimary - 60)
					{
						if (( chPrimary20 == bssPrimary - 70) || (chPrimary20 == bssPrimary - 50))
						{
							err = ERR_OK;
						}
						else
						{
							err = -1;
						}
					}
					else if ( chFreq == bssPrimary - 20)
					{
						if (( chPrimary20 == bssPrimary - 10) || (chPrimary20 == bssPrimary - 30))
						{
							err = ERR_OK;
						}
						else
						{
							err = -1;
						}
					}
					else if (chFreq == bssPrimary + 20)
					{
						if (( chPrimary20 == bssPrimary + 10) || (chPrimary20 == bssPrimary + 30))
						{
							err = ERR_OK;
						}
						else
						{
							err = -1;
						}
					}
					else if (chFreq == bssPrimary +60)
					{
						if (( chPrimary20 == bssPrimary + 70) || (chPrimary20 == bssPrimary + 50))
						{
							err = ERR_OK;
						}
						else
						{
							err = -1;
						}
					}
					else
					{
						err = -1;
					}
				}
				else if (cbw == BW_80MHZ)
				{
					if ( chFreq == bssPrimary -40)
					{
						if ((chPrimary20 == bssPrimary -10) || ( chPrimary20 == bssPrimary-30) ||
							(chPrimary20 == bssPrimary -50) || (chPrimary20 == bssPrimary -70))
						{
							err = ERR_OK;
						}
						else
						{
							err = -1;
						}
					}
					else
					{
						if ((chPrimary20 == bssPrimary +10) || ( chPrimary20 == bssPrimary+30) ||
							(chPrimary20 == bssPrimary +50) || (chPrimary20 == bssPrimary +70))
						{
							err = ERR_OK;
						}
						else
						{
							err = -1;
						}
					}
				}
				else if (cbw == BW_160MHZ)
				{
					if ((chPrimary20 == bssPrimary +10) || ( chPrimary20 == bssPrimary+30) ||
							(chPrimary20 == bssPrimary +50) || (chPrimary20 == bssPrimary +70) ||
							(chPrimary20 == bssPrimary -10) || ( chPrimary20 == bssPrimary-30) ||
							(chPrimary20 == bssPrimary -50) || (chPrimary20 == bssPrimary -70))
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else
				{
					err = -1;
				}
				break;
			case BW_80_80MHZ:
				if (cbw == BW_20MHZ)
				{
					if (chPrimary20 == chFreq )
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else if (cbw == BW_40MHZ)
				{
					if ( chFreq == bssPrimary -20)
					{
						if ((chPrimary20 == bssPrimary - 10) || (chPrimary20 == bssPrimary -30))
						{
							err = ERR_OK;
						}
						else
						{
							err = -1;
						}
					}
					else if (chFreq == bssPrimary + 20)
					{
						if ((chPrimary20 == bssPrimary + 10) || (chPrimary20 == bssPrimary + 30))
						{
							err = ERR_OK;
						}
						else
						{
							err = -1;
						}
					}
					else
					{
						err = -1;
					}
				}
				else if (cbw == BW_80MHZ)
				{
					if (chFreq == bssPrimary)
					{
						if (( chPrimary20 == bssPrimary -10) || ( chPrimary20 == bssPrimary -30) ||
							(chPrimary20 == bssPrimary + 10) || ( chPrimary20 == bssPrimary +30))
						{
							err = ERR_OK;
						}
						else
						{
							err = -1;
						}
					}
					else
					{
						err = -1;
					}
				}
				else if ( cbw == BW_80_80MHZ)
				{
					if (( chPrimary20 == bssPrimary -10) || ( chPrimary20 == bssPrimary -30) ||
							(chPrimary20 == bssPrimary + 10) || ( chPrimary20 == bssPrimary +30))
					{
						err = ERR_OK;
					}
					else
					{
						err = -1;
					}
				}
				else
				{
					err = -1;
				}
				break;
			default:
				err = -1;
				break;
			}
	}

	return err;
}

WIFI_11AC_MIMO_TEST_API int	CheckCommonParameters_WiFi_11ac_MiMo(map<string, WIFI_SETTING_STRUCT>& inputMap, 
															map<string, WIFI_SETTING_STRUCT>& returnMap, 
															TM_ID WiFi_11ac_Test_ID, 
															int *VsaOrVsgEnabled, 
															int *bssBW, 
															int *cbw, 
															int *bssPchannel, 
															int *bssSchannel,
															int *wifiMode, 
															int *wifiStreamNum, 
															double *samplingTimeUs
															//double *cableLossDb, 
															)
{
	int    err		= ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	double local_cableLossDb = 0;
	map<string, WIFI_SETTING_STRUCT>::iterator inputMap_Iter;
	map<string, WIFI_SETTING_STRUCT>::iterator returnMap_Iter;

	struct tagCommonParam								//	points to common input parameters 
	{
		int		*p_BSS_FREQ_MHZ_PRIMARY;				//
		int		*p_BSS_FREQ_MHZ_SECONDARY;				//
		int		*p_CH_FREQ_MHZ_PRIMARY_20MHz;			//
		int		*p_CH_FREQ_MHZ;							//
		int		*p_NUM_STREAM_11AC;						//
		char	*p_BSS_BANDWIDTH;						// char[MAX_BUFFER_SIZE]
		char	*p_CH_BANDWIDTH;						// char[MAX_BUFFER_SIZE]
		char	*p_DATA_RATE;							// char[MAX_BUFFER_SIZE]
		char	*p_PREAMBLE;							// char[MAX_BUFFER_SIZE]
		char	*p_PACKET_FORMAT;						// char[MAX_BUFFER_SIZE]
		char	*p_STANDARD;							// char[MAX_BUFFER_SIZE]
		double	*p_CABLE_LOSS_DB;	                    // [MAX_DATA_STREAM]
		double	*p_SAMPLING_TIME_US;                   //
	}commonParam;

	struct tagCommonReturn								//	points to common return variables 
	{
		int     *p_CH_FREQ_MHZ_PRIMARY_40MHz;       		//
		int     *p_CH_FREQ_MHZ_PRIMARY_80MHz;       		//
		double	*p_CABLE_LOSS_DB;							// [MAX_DATA_STREAM]
	}commonReturn;

	/*-----------------------------------------------------------------*
	*   Link all input parameters, from inputMap to local structure    *
	*------------------------------------------------------------------*/
	//copy the values from the map to local structure
	inputMap_Iter=inputMap.find("BSS_FREQ_MHZ_PRIMARY");
	commonParam.p_BSS_FREQ_MHZ_PRIMARY = (inputMap.end()==inputMap_Iter)? NULL: ((int*)inputMap_Iter->second.value);

	inputMap_Iter=inputMap.find("BSS_FREQ_MHZ_SECONDARY");
	commonParam.p_BSS_FREQ_MHZ_SECONDARY =(inputMap.end()==inputMap_Iter)? NULL: ((int*)inputMap_Iter->second.value);

	inputMap_Iter=inputMap.find("CH_FREQ_MHZ_PRIMARY_20MHz");
	commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = (inputMap.end()==inputMap_Iter)? NULL: ((int*)inputMap_Iter->second.value);

	inputMap_Iter=inputMap.find("CH_FREQ_MHZ");
	commonParam.p_CH_FREQ_MHZ = (inputMap.end()==inputMap_Iter)? NULL: ((int*)inputMap_Iter->second.value);

	inputMap_Iter=inputMap.find("NUM_STREAM_11AC");
	commonParam.p_NUM_STREAM_11AC = (inputMap.end()==inputMap_Iter)? NULL: ((int*)inputMap_Iter->second.value);

	inputMap_Iter=inputMap.find("BSS_BANDWIDTH");
	commonParam.p_BSS_BANDWIDTH = (inputMap.end()==inputMap_Iter)? NULL: ((char*)inputMap_Iter->second.value);

	inputMap_Iter=inputMap.find("CH_BANDWIDTH");
	commonParam.p_CH_BANDWIDTH = (inputMap.end()==inputMap_Iter)? NULL: ((char*)inputMap_Iter->second.value);

	inputMap_Iter=inputMap.find("DATA_RATE");
	commonParam.p_DATA_RATE = (inputMap.end()==inputMap_Iter)? NULL: ((char*)inputMap_Iter->second.value);

	inputMap_Iter=inputMap.find("PREAMBLE");
	commonParam.p_PREAMBLE = (inputMap.end()==inputMap_Iter)? NULL: ((char*)inputMap_Iter->second.value);

	inputMap_Iter=inputMap.find("PACKET_FORMAT");
	commonParam.p_PACKET_FORMAT = (inputMap.end()==inputMap_Iter)? NULL: ((char*)inputMap_Iter->second.value);

	inputMap_Iter=inputMap.find("STANDARD");
	commonParam.p_STANDARD = (inputMap.end()==inputMap_Iter)? NULL: ((char*)inputMap_Iter->second.value);


	inputMap_Iter=inputMap.find("CABLE_LOSS_DB_VSA1");
	if(inputMap.end()==inputMap_Iter)
	{		
		inputMap_Iter=inputMap.find("CABLE_LOSS_DB_VSG1");
	}		
	commonParam.p_CABLE_LOSS_DB = (inputMap.end()==inputMap_Iter)? NULL: ((double *)inputMap_Iter->second.value);

	inputMap_Iter=inputMap.find("SAMPLING_TIME_US");
	commonParam.p_SAMPLING_TIME_US = (inputMap.end()==inputMap_Iter)? NULL: ((double *)inputMap_Iter->second.value);

		

	/*-----------------------------------------------------------------*
	*   Link all return values, from returnMap to local structure    *
	*------------------------------------------------------------------*/
	returnMap_Iter = returnMap.find("CH_FREQ_MHZ_PRIMARY_40MHz");
	commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz = (returnMap.end()==returnMap_Iter)? NULL: ((int*)returnMap_Iter->second.value);
	
	returnMap_Iter = returnMap.find("CH_FREQ_MHZ_PRIMARY_80MHz");
	commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz = (returnMap.end()==returnMap_Iter)? NULL: ((int*)returnMap_Iter->second.value);

	returnMap_Iter = returnMap.find("CABLE_LOSS_DB_VSA1");
	if(returnMap.end()==returnMap_Iter)
	{
		returnMap_Iter = returnMap.find("CABLE_LOSS_DB_VSG1");
	}
	commonReturn.p_CABLE_LOSS_DB = (returnMap.end()==returnMap_Iter)? NULL: ((double*)returnMap_Iter->second.value);


	/*----------------------*
	*   Check PREAMBLE		*
	*-----------------------*/
	if (0!= strcmp(commonParam.p_PREAMBLE, "SHORT") && 
		0!=strcmp(commonParam.p_PREAMBLE, "LONG") )
	{
		err = -1;
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown PREAMBLE, WiFi preamble %s not supported.\n", commonParam.p_PREAMBLE);
		throw logMessage;
	}
	else
	{
		//do nothing
	}

	/*--------------------------*
	*   Check NUM_STREAM_11AC	*
	*---------------------------*/
	if (( 0> *commonParam.p_NUM_STREAM_11AC) || ( *commonParam.p_NUM_STREAM_11AC >8))
	{
		err = -1;
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] NUM_STREAM_11AC only can be 1~8. Please check input parameter \"NUM_STREAM_11AC\"!\n");
		throw logMessage;
	}
	else
	{
		// do nothing
	}

	/*------------------*
	*   Check STANDARD	*
	*-------------------*/
	if ((0!= strcmp(commonParam.p_STANDARD, STANDARD_802_11_AC))	&&	
		(0!= strcmp(commonParam.p_STANDARD, STANDARD_802_11_N))		&&
		(0!= strcmp(commonParam.p_STANDARD, STANDARD_802_11_AG))	&&
		(0!= strcmp(commonParam.p_STANDARD, STANDARD_802_11_B))) 
	{
		err = -1;
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown \"STANDARD\": can not take value \"%s\". \n", commonParam.p_STANDARD);
		throw logMessage;
	}


	/*--------------------------*
	*   Check packet_format		*
	*---------------------------*/

	if ((0!= strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_HT_MF)) && 
		(0!= strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_HT_GF)) &&
		(0!= strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_VHT))   &&
		(0!= strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_NON_HT)))
	{
		err = -1;
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown \"PACKET_FORMAT\" can not take value \"%s\". \n", commonParam.p_PACKET_FORMAT);
		throw logMessage;
	}


#pragma region check DATA_RATE
	/*--------------------------*
	*   Check data rate         *
	*---------------------------*/
	err = TM_WiFiConvertDataRateNameToIndex(commonParam.p_DATA_RATE, &dummyInt);      
	if ( ERR_OK!=err )
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown \"DATA_RATE\", convert WiFi data rate %s to index failed.\n", commonParam.p_DATA_RATE);
		throw logMessage;
	}
	else
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
	}

	if( 0 == strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_VHT))				// VHT 
	{
		if (( 14 <= dummyInt ) && (dummyInt <= 23))
		{
			// Data rate: MCS0 ~MCS9, do nothing
		}
		else
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unsupported DATA_RATE \"%s\": \"%s\" packet format supports only MCS0, MCS1 ... MCS9! \n", &commonParam.p_DATA_RATE, PACKET_FORMAT_VHT);
			throw logMessage;
		}
	}
	else if ( 0 == strcmp( commonParam.p_PACKET_FORMAT, PACKET_FORMAT_NON_HT) )			// NON_HT
	{
		if (!strstr ( commonParam.p_DATA_RATE, "OFDM") &&
			!strstr ( commonParam.p_DATA_RATE, "DSSS") &&
			!strstr ( commonParam.p_DATA_RATE, "CCK")  &&
			!strstr ( commonParam.p_DATA_RATE, "PBCC"))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unsupported DATA_RATE \"%s\": \"%s\" packet format doesn't support MCS data rate! \n", &commonParam.p_DATA_RATE, PACKET_FORMAT_NON_HT);
			throw logMessage;
		}
	}
	else if (	( 0 == strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_HT_MF))	// HT_MF,
		||	( 0 == strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_HT_GF)))	// HT_GF													
	{
		//do thing with data rate checking 
	}

	else
	{
		// do nothing.
	}
#pragma endregion 


#pragma region  Check BSS_BANDWIDTH,BSS_FREQ_MHZ_SECONDARY, CH_BANDWIDTH& channel list
	/*-----------------------------------------------*
	*   Check BSS_BANDWIDTH, BSS_FREQ_MHZ_SECONDARY	 *
	*-----------------------------------------------*/
	if ( 0!=strcmp(commonParam.p_BSS_BANDWIDTH, "BW-20")	&& 
		0!=strcmp(commonParam.p_BSS_BANDWIDTH, "BW-40")	&& 
		0!=strcmp(commonParam.p_BSS_BANDWIDTH, "BW-80")	&& 
		0!=strcmp(commonParam.p_BSS_BANDWIDTH, "BW-160")	&&
		0!=strcmp(commonParam.p_BSS_BANDWIDTH, "BW-80_80")	)
	{
		err = -1;
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS_BANDWIDTH! WiFi 11ac BSS bandwidth %s not supported.\n", commonParam.p_BSS_BANDWIDTH);
		throw logMessage;
	}
	else
	{
		if ( 0 == strcmp(commonParam.p_BSS_BANDWIDTH, "BW-20"))
		{
			*bssBW = BW_20MHZ;
		}
		else if ( 0 == strcmp(commonParam.p_BSS_BANDWIDTH, "BW-40"))
		{
			*bssBW = BW_40MHZ;
		}
		else if ( 0 == strcmp(commonParam.p_BSS_BANDWIDTH, "BW-80"))
		{
			*bssBW = BW_80MHZ;
		}
		else if ( 0 == strcmp(commonParam.p_BSS_BANDWIDTH, "BW-160"))
		{
			*bssBW = BW_160MHZ;
		}
		else if ( 0 == strcmp(commonParam.p_BSS_BANDWIDTH, "BW-80_80"))
		{
			*bssBW = BW_80_80MHZ;

			if ( *commonParam.p_BSS_FREQ_MHZ_SECONDARY <= 0 )
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,	"[WiFi_11ac_MiMo] BSS_BANDWIDTH = BW_80_80MHZ,\"BSS_FREQ_MHZ_SECONDARY\" must have value! Please check input parameters!\n" );
				throw logMessage;
			}
			else
			{
				// do nothing
			}
		}
		else
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown BSS_BANDWIDTH, \"%s\" is not supported! \n", commonParam.p_BSS_BANDWIDTH);
			throw logMessage;
		}

	}

	/*-----------------------------------------------*
	*   Check CH_BANDWIDTH input                  	 *
	*-----------------------------------------------*/
	if ( 0 != strcmp(commonParam.p_CH_BANDWIDTH, "0") && 
		0 != strcmp(commonParam.p_CH_BANDWIDTH, "CBW-20") && 
		0 != strcmp(commonParam.p_CH_BANDWIDTH, "CBW-40") && 
		0 != strcmp(commonParam.p_CH_BANDWIDTH, "CBW-80") && 
		0 != strcmp(commonParam.p_CH_BANDWIDTH, "CBW-160") &&
		0 != strcmp(commonParam.p_CH_BANDWIDTH, "CBW-80_80"))
	{
		err = -1;
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Unknown CBW_BANDWIDTH, \"%s\" is not supported! \n", commonParam.p_CH_BANDWIDTH);
		throw logMessage;
	}
	else
	{
		if ( 0 == strcmp(commonParam.p_CH_BANDWIDTH, "0"))
		{
			sprintf_s(commonParam.p_CH_BANDWIDTH,MAX_BUFFER_SIZE,"C%s",commonParam.p_BSS_BANDWIDTH);
			*cbw = *bssBW;
		}
		else if ( 0 == strcmp(commonParam.p_CH_BANDWIDTH, "CBW-20"))
		{
			*cbw = BW_20MHZ;
		}
		else if ( 0 == strcmp(commonParam.p_CH_BANDWIDTH, "CBW-40"))
		{
			*cbw = BW_40MHZ;
		}
		else if ( 0 == strcmp(commonParam.p_CH_BANDWIDTH, "CBW-80"))
		{
			*cbw = BW_80MHZ;
		}
		else if ( 0 == strcmp(commonParam.p_CH_BANDWIDTH, "CBW-160"))
		{
			*cbw = BW_160MHZ;
		}
		else if ( 0 == strcmp(commonParam.p_CH_BANDWIDTH, "CBW-80_80"))
		{
			*cbw = BW_80_80MHZ;
		}
	}

	/*-------------------------------------------------------------------*
	*   Check the fact that "CH_BANDWIDTH <= BSS_BANDWIDTH "          	 *
	*--------------------------------------------------------------------*/
	if ( strstr ( commonParam.p_PACKET_FORMAT,PACKET_FORMAT_VHT))  //802.11ac, cbw can't be larger than bssBW. 
	{
		if (*bssBW != BW_80_80MHZ)
		{
			if (*cbw > *bssBW)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] CBW_BANDWIDTH can not be wider than BSS_BANDWIDTH.\n");
				throw logMessage;
			}
		}
		else  //bssBW = BW_80_80MHZ
		{
			if ( *cbw == BW_160MHZ)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] BBS bandwidth = 80+80MHz.Channel bandwidth can't be 160MHz.\n");
				throw logMessage;
			}
		}
	}
	else																// if not 802.11ac, use only CBW_BANDWIDTH (*cbw), not BSS_BANDWIDTH (*bssBW)
	{
		// do nothing
	}

	/*-----------------------------------------------*
	*   Validate CH_BANDWIDTH value cbw            	 *
	*-----------------------------------------------*/

	if (0== strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_VHT))				// 802.11ac VHT 
	{
		//do nothing
	}
	else if (0== strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_HT_MF) ||		// 802.11n HT 
		0== strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_HT_GF))
	{
		if ( *cbw != BW_20MHZ && *cbw != BW_40MHZ)
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] 11n HT_MF(GF) bandwidth can only be 20MHz or 40Mhz. Please check input parameters!\n" );
			throw logMessage;
		}
	}
	else if (0== strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_NON_HT) )		// 802.11ag/b NON_HT 
	{
		if (*cbw != BW_20MHZ)
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo]11ag/b NON_HT bandwidth can only be 20MHz. Please check input parameters!\n" );
			throw logMessage;
		}
	}
	else
	{
		//do nothing ... 
	}


	/*------------------------------*
	*   Check BSS_FREQ_MHZ_PRIMARY	*
	*-------------------------------*/
	if (commonParam.p_BSS_FREQ_MHZ_PRIMARY!=NULL && *commonParam.p_BSS_FREQ_MHZ_PRIMARY <= 0)
	{
		err = -1;
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong parameter %d, \"BSS_FREQ_MHZ_PRIMARY\" must have a non-zero value! \n" ,*commonParam.p_BSS_FREQ_MHZ_PRIMARY);
		throw logMessage;

	}
	else
	{
		// do nothing
	}

	err = TM_WiFiConvertFrequencyToChannel(*commonParam.p_BSS_FREQ_MHZ_PRIMARY, bssPchannel);      
	if ( ERR_OK!=err )
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong BSS_FREQ_MHZ_PRIMARY, convert WiFi frequency %d to channel failed.\n", *commonParam.p_BSS_FREQ_MHZ_PRIMARY);
		throw logMessage;
	}
	else
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
	}


	/*-----------------------------------*
	*   Check CH_FREQ_MHZ				*
	*----------------------------------*/
	bssSchannel = 0;
	if ( *bssBW == BW_80_80MHZ)				// Need BSS_FREQ_MHZ_SECONDARY
	{
		if ( *commonParam.p_BSS_FREQ_MHZ_SECONDARY <= 0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] BSS_BANDWIDTH = BW_80_80MHZ,\"BSS_FREQ_MHZ_SECONDARY\" must have value!\n" );
			throw logMessage;
		}
		else
		{
			err = TM_WiFiConvertFrequencyToChannel(*commonParam.p_BSS_FREQ_MHZ_SECONDARY, bssSchannel);      
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong BSS_FREQ_MHZ_SECONDARY, convert WiFi frequency %d to channel failed.\n", *commonParam.p_BSS_FREQ_MHZ_SECONDARY);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] TM_WiFiConvertFrequencyToChannel() return OK.\n");
			}
		}
	}


	/*-------------------------------------*
	*  Check CH_FREQ_MHZ & CH_BANDWIDTH    *
	*--------------------------------------*/		
	if (( 0 != strcmp(commonParam.p_PACKET_FORMAT, PACKET_FORMAT_VHT)))	// HT_MF, HT_GF, or  NON_HT
	{
		//if (( *commonParam.p_CH_FREQ_MHZ <= 0) ||( commonParam.p_CH_BANDWIDTH <= 0))
		if (( *commonParam.p_CH_FREQ_MHZ <= 0) ||0 == strcmp(commonParam.p_CH_BANDWIDTH, "0"))
			
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong \"CH_FREQ_MHZ\" or \"CH_BANDWIDTH\"! They must have non-zero values when packet format is not %s ! \n", PACKET_FORMAT_VHT);
			throw logMessage;
		}
	}

	//EM comment out this check.  we still want to get the PRIMARY_20MHZ frequency even if BW is HT40.
	//if ( strstr (commonParam.p_PACKET_FORMAT, PACKET_FORMAT_VHT))  /// VHT ...........
	//{

		if ( 0 == *commonParam.p_CH_FREQ_MHZ)  
		{
			if ( 0 == *commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz)  // CH_FREQ_MHZ = 0 and CH_FREQ_MHZ_PRIMARY_20MHz =0
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi_11ac_MiMo] zero value for both \"CH_FREQ_MHZ\" and \"CH_FREQ_MHZ_PRIMARY_20\", use BBS center frequency as default! \n");
				*commonParam.p_CH_FREQ_MHZ = *commonParam.p_BSS_FREQ_MHZ_PRIMARY;
				// all  use lower frequency for channel list
				switch (*bssBW)
				{
				case BW_20MHZ:
					*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
					break;
				case BW_40MHZ:
					*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 10;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
					break;
				case BW_80MHZ:
					*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 30;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 20;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY;
					break;
				case BW_160MHZ:			
					*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 70;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 60;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 40;
					break;
				case BW_80_80MHZ:				
					*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 70;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 60;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 40;
					break;
				default:
					*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
					break;
				}
			}
			else if (0<*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz)  // CH_FREQ_MHZ = 0 and CH_FREQ_MHZ_PRIMARY_20MHz > 0, Use input CH_FREQ_MHZ_PRIMARY_20MHz to calculate
			{
				err = GetChannelList(*bssBW, *commonParam.p_BSS_FREQ_MHZ_PRIMARY,*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz,
					commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz,commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz);
				if ( err != ERR_OK) // Wrong channel list
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value %d. Please check input paramters.\n", *commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz);
					throw logMessage;
				}
				else  // Get channel list successfully
				{
				}
			}
			else 
			{
				//do nothing
			}

			// assign cbw value 
			if (*cbw != BW_80_80MHZ)
			{
				if (*cbw == *bssBW)
				{
					*commonParam.p_CH_FREQ_MHZ = *commonParam.p_BSS_FREQ_MHZ_PRIMARY;

				}
				else if (*cbw == BW_20MHZ)
				{
					*commonParam.p_CH_FREQ_MHZ = *commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz;
				}
				else if (*cbw == BW_40MHZ)
				{
					*commonParam.p_CH_FREQ_MHZ = *commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz;

				}
				else if (*cbw == BW_80MHZ)
				{
					*commonParam.p_CH_FREQ_MHZ = *commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz;
				}
				else
				{
					*commonParam.p_CH_FREQ_MHZ = *commonParam.p_BSS_FREQ_MHZ_PRIMARY;
				}
			}
			else // cbw = BW_80_80MHZ, use BSS primary and secondary
			{
				*commonParam.p_CH_FREQ_MHZ = NA_INTEGER;
			}
		}
		else if ( 0!= *commonParam.p_CH_FREQ_MHZ)  // CH_FREQ_MHZ: non-zero 
		{
			//Check if input CH_FREQ_MHZ is correct
			err = CheckChannelFreq(*bssBW, *cbw, *commonParam.p_BSS_FREQ_MHZ_PRIMARY, *commonParam.p_CH_FREQ_MHZ);

			if (err != ERR_OK)
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Input paramter \"CH_FREQ_MHZ\" wrong. Please check!\n");
				throw logMessage;
			}
			else
			{
			}


			// Check if input CH_FREQ_MHZ_PRIMARY_20MHz.
			// if zero, calculate by default
			// If non-zero and correct, use it directly
			// If not-zero, but incorrect, throw error
			if ( 0 != *commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz)
			{
				err = CheckChPrimary20(*bssBW, *cbw,
					*commonParam.p_BSS_FREQ_MHZ_PRIMARY,*commonParam.p_CH_FREQ_MHZ,*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz);
				if ( err == ERR_OK)  //input CH_FREQ_MHZ_PRIMARY_20MHz is correct
				{
					err = GetChannelList(*bssBW, *commonParam.p_BSS_FREQ_MHZ_PRIMARY,*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz,
						commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz,commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz);
					if ( err != ERR_OK) // Wrong channel list
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Can't get channel list. Please check input paramters.\n", commonParam.p_PACKET_FORMAT);
						throw logMessage;
					}
					else  // Get channel list successfully
					{
					}
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Input paramter \"CH_FREQ_MHZ_PRIMARY_20MHz\" wrong. Please check!\n");
					throw logMessage;
				}
			}
			else //input CH_FREQ_MHZ_PRIMARY_20MHz is zero
			{
				//Get channel primary20
				switch (*bssBW)
				{
				case BW_20MHZ:
					*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
					*commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
					break;
				case BW_40MHZ:  //cbw = BW_20.40MHz
					if (*cbw == BW_20MHZ)
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_CH_FREQ_MHZ;
					}
					else if ( *cbw == BW_40MHZ)
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 10;

					}
					else  //wrong cbw
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;

					}
					break;
				case BW_80MHZ:  //cbw = BW_20,40,80MHz
					if ( *cbw == BW_20MHZ)
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_CH_FREQ_MHZ;

					}
					else if ( *cbw == BW_40MHZ)
					{
						if (*commonParam.p_CH_FREQ_MHZ == *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 20)
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 30;

						}
						else if (*commonParam.p_CH_FREQ_MHZ == *commonParam.p_BSS_FREQ_MHZ_PRIMARY + 20)
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY + 10;
						}
						else  // wrong CH_FREQ_MHZ
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
						}
					}
					else if ( *cbw == BW_80MHZ)
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 30;
					}
					else  //wrong cbw
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz =  NA_INTEGER;
					}
					break;
				case BW_160MHZ:  //cbw = BW_20,40,80,160MHz
					if ( *cbw == BW_20MHZ)
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_CH_FREQ_MHZ;
					}
					else if ( *cbw == BW_40MHZ)
					{
						if (*commonParam.p_CH_FREQ_MHZ == *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 60)
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 70;
						}
						else if (*commonParam.p_CH_FREQ_MHZ == *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 20)
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 30;
						}
						else if (*commonParam.p_CH_FREQ_MHZ == *commonParam.p_BSS_FREQ_MHZ_PRIMARY + 20)
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY + 10;
						}
						else if ( *commonParam.p_CH_FREQ_MHZ == *commonParam.p_BSS_FREQ_MHZ_PRIMARY + 60)
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY + 50;
						}
						else  //wrong CH_FREQ_MHZ
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
						}
					}
					else if ( *cbw == BW_80MHZ)
					{
						if (*commonParam.p_CH_FREQ_MHZ == *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 40)
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 70;
						}
						else if (*commonParam.p_CH_FREQ_MHZ == *commonParam.p_BSS_FREQ_MHZ_PRIMARY + 40)
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY + 10;
						}
						else // wrong CH_FREQ_MHZ
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
						}
					}
					else if ( *cbw == BW_160MHZ)
					{
						if (*commonParam.p_CH_FREQ_MHZ == *commonParam.p_BSS_FREQ_MHZ_PRIMARY)
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 70;
						}
						else // wrong CH_FREQ_MHZ
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
						}
					}
					else  //wring cbw
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
					}
					break;
				case BW_80_80MHZ:
					if ( *cbw == BW_20MHZ)
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_CH_FREQ_MHZ;
					}
					else if ( *cbw == BW_40MHZ)
					{
						if ( *commonParam.p_CH_FREQ_MHZ == *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 20)
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 30;
						}
						else if (*commonParam.p_CH_FREQ_MHZ == *commonParam.p_BSS_FREQ_MHZ_PRIMARY + 20)
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY + 10;
						}
						else  // wrong CH_FREQ_MHZ
						{
							*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
						}
					}
					else if ( *cbw == BW_80MHZ)
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 30;
					}
					else if ( *cbw == BW_80_80MHZ)
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = *commonParam.p_BSS_FREQ_MHZ_PRIMARY - 30;
					}
					else  // wrong CH_FREQ_MHz
					{
						*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
					}
					break;
				default:
					*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
					break;
				}
			}

			err = GetChannelList(*bssBW, *commonParam.p_BSS_FREQ_MHZ_PRIMARY,*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz,
				commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz,commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz);
			if ( err != ERR_OK)  //Get channel list wrong
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", commonParam.p_PACKET_FORMAT);
				throw logMessage;
			}
			else  //Get channel list successfully
			{
			}
		}	

		//Check channel list. If all are "NA_INTEGER", return error
		if (( *commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz == NA_INTEGER ) && 
			(*commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz == NA_INTEGER ) &&
			(*commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER))
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Wrong CH_FREQ_MHZ_PRIMARY_20/40/80MHz, please check input parameters.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}
	//}
	//else   // non-ac, no use of CH_FREQ_MHZ_PRIMARY_20/40/80MHz
	//{
	//	*commonParam.p_CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
	//	*commonReturn.p_CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
	//	*commonReturn.p_CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
	//}
#pragma endregion

	/*-------------------------------------*
	*  Check STREAM NUMBER FOR VHT         *
	*--------------------------------------*/		
	if(strstr (commonParam.p_PACKET_FORMAT, PACKET_FORMAT_VHT))
	{
		if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )		//VHT, cbw = BW_20MHZ, the stream number of MCS9 only can 3,6
		{
			if ( (*commonParam.p_NUM_STREAM_11AC != 3) &&
				(*commonParam.p_NUM_STREAM_11AC != 6) )
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] 802.11ac CBW = 20MHz, Data Rate = MCS9,NUM_STREAM_11AC can only be 3 or 6!\n");
				throw logMessage;
			}
			else
			{
			}
		}
		else if ( ( *cbw == BW_80MHZ) && ( dummyInt == 23) )	//VHT, cbw = BW_80MHz, the stream number of MCS9 cannot be 6
		{
			if ( *commonParam.p_NUM_STREAM_11AC == 6)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] 802.11ac CBW = 80MHz, Data Rate = MCS9, NUM_STREAM_11AC cannot be 6!\n");
				throw logMessage;
			}
			else
			{
			}
		}
		else if ( ( *cbw == BW_80MHZ) && ( dummyInt == 20) )	//VHT, cbw = BW_80MHz, the stream number of MCS6 cannot be 3 and 7
		{
			if ( *commonParam.p_NUM_STREAM_11AC == 3 || *commonParam.p_NUM_STREAM_11AC == 7)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] 802.11ac CBW = 80MHz, Data Rate = MCS6, NUM_STREAM_11AC cannot be 3 or 7!\n");
				throw logMessage;
			}
			else
			{
			}
		}
		else
		{
			// do nothing
		}
	}

	/*-------------------------------------*
	*  Check path loss (by ant and freq)   *
	*--------------------------------------*/	

	//temp: declare double *cableLossDb
	//TODO: remove "cableLossDb" in CheckPathLossTable(), each VSA/VSG cable loss will record in CABLE_LOSS_DB of local Param 
	//double *cableLossDb= (double *) malloc (sizeof (double));
	if(NULL != commonReturn.p_CABLE_LOSS_DB)
	{
		inputMap_Iter=inputMap.find("TX1");
		if(inputMap.end()!= inputMap_Iter)
		{		
			err = CheckPathLossTableExt(WiFi_11ac_Test_ID,
				*commonParam.p_CH_FREQ_MHZ,
				VsaOrVsgEnabled[0],
				VsaOrVsgEnabled[1],
				VsaOrVsgEnabled[2],
				VsaOrVsgEnabled[3],
				commonParam.p_CABLE_LOSS_DB,
				commonReturn.p_CABLE_LOSS_DB,
				&local_cableLossDb,
				TX_TABLE
				);
		}	
		else
		{   
			inputMap_Iter=inputMap.find("RX1");
			if(inputMap.end()!= inputMap_Iter)
			{
				err = CheckPathLossTableExt(WiFi_11ac_Test_ID,
					*commonParam.p_CH_FREQ_MHZ,
					VsaOrVsgEnabled[0],
					VsaOrVsgEnabled[1],
					VsaOrVsgEnabled[2],
					VsaOrVsgEnabled[3],
					commonParam.p_CABLE_LOSS_DB,
					commonReturn.p_CABLE_LOSS_DB,
					&local_cableLossDb,
					RX_TABLE
					);
			}
		}
	}
	else
	{
		double return_cable_loss[MAX_BUFFER_SIZE]; 
		inputMap_Iter = inputMap.find("TX1");
		if(inputMap.end()!= inputMap_Iter)
		{		
			err = CheckPathLossTableExt(WiFi_11ac_Test_ID,
				*commonParam.p_CH_FREQ_MHZ,
				VsaOrVsgEnabled[0],
				VsaOrVsgEnabled[1],
				VsaOrVsgEnabled[2],
				VsaOrVsgEnabled[3],
				commonParam.p_CABLE_LOSS_DB,
				return_cable_loss,
				&local_cableLossDb,
				TX_TABLE
				);
		}	
		else
		{   
			inputMap_Iter=inputMap.find("RX1");
			if(inputMap.end()!= inputMap_Iter)
			{
				err = CheckPathLossTableExt(WiFi_11ac_Test_ID,
					*commonParam.p_CH_FREQ_MHZ,
					VsaOrVsgEnabled[0],
				    VsaOrVsgEnabled[1],
				    VsaOrVsgEnabled[2],
				    VsaOrVsgEnabled[3],
					commonParam.p_CABLE_LOSS_DB,
					return_cable_loss,
					&local_cableLossDb,
					RX_TABLE
					);
			}
		}

	}

	if ( ERR_OK!=err )
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Fail to get CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
		throw logMessage;
	}
	else
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] CheckPathLossTableExt return OK.\n");
	}

	// Convert parameter
	err = WiFi_11ac_TestMode(commonParam.p_DATA_RATE, cbw, wifiMode, wifiStreamNum, commonParam.p_PACKET_FORMAT);
	if ( ERR_OK!=err )
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac_MiMo] Convert WiFi test mode function failed.\n");
		throw logMessage;
	}
	else
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac_MiMo] WiFiTestMode() return OK.\n");
	}

	/*-------------------------------------*
	*  Check sampling time				   *
	*--------------------------------------*/	
	if(NULL != commonParam.p_SAMPLING_TIME_US) // do only when SAMPLING_TIME_US is included in input parameters 
	{
		if (0==*commonParam.p_SAMPLING_TIME_US)
		{
			*samplingTimeUs = CheckSamplingTime(*wifiMode, commonParam.p_PREAMBLE, commonParam.p_DATA_RATE, commonParam.p_PACKET_FORMAT);
		}
		else	// SAMPLING_TIME_US != 0
		{
			*samplingTimeUs = *commonParam.p_SAMPLING_TIME_US;
		}
	}

	return err;
}
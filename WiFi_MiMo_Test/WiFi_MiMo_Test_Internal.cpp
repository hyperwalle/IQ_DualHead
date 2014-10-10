#include "stdafx.h"
#include "WiFi_MiMo_Test.h"
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
bool             g_txCalibrationEnabled = false;
int				 g_Logger_ID        = -1;
int              g_WiFi_Test_timer  = -1;
int              g_WiFi_Test_logger = -1;
int				 g_Tester_Type      = IQ_View;
int				 g_Tester_Number    = 0;
int				 g_Tester_Reconnect = 0;
double			 g_last_TxPower_dBm_Record = 0; // Record the Tx power used in last test item.


const char      *g_WiFi_Test_Version = "1.5.1 (2010-05-24)\n";
char			 g_defaultFilePath[MAX_BUFFER_SIZE] = {'\0'};


WIFI_RECORD_PARAM g_RecordedParam;

// This global variable is declared in WiFi_Global_Setting.cpp
// Input Parameter Container
extern map<string, WIFI_SETTING_STRUCT> g_globalSettingParamMap;
extern WIFI_GLOBAL_SETTING g_globalSettingParam;

extern int       g_iTesterName;
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

int InitializeAllMIMOContainers(void)
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


void CleanupAllMIMOContainers(void)
{
	// IQlite merge; Tracy Yu ; 2012-03-31
	CleanupTXConnectIQTester();                     // Needed by WiFi_Connect_IQTester
    CleanupTXDisconnectIQTester();                  // Needed by WiFi_Disconnect_IQTester
    CleanupGetSerialNumber();                       // Needed by WiFi_Get_Serial_Number
	CleanupGlobalSetting();                         // Needed by WiFi_Global_Setting
    CleanupInitializeDut();                         // Neede by WiFi_Initialize_Dut 
    CleanupInsertDut();                             // Needed by WiFi_Insert_Dut
    CleanupLoadPathLossTable();                     // Needed by WiFi_Load_Path_Loss_Table
    CleanupPowerModeDut();                          // Needed by WiFi_Power_Mode_Dut
    CleanupReadBBRegister();                        // Needed by WiFi_Read_BB_Register
    CleanupReadEeprom();                            // Needed by WiFi_Read_Eeprom
	CleanupReadMacAddress();						// Needed by WiFi_Read_Mac_Address
	CleanupReadMacRegister();    					// Needed by WiFi_Read_Mac_Register
	CleanupReadRfRegister();     					// Needed by WiFi_Read_RF_Register
	CleanupReadSocRegister();    					// Needed by WiFi_Read_Soc_Register
	CleanupRemoveDut();          					// Needed by WiFi_Remove_Dut
	CleanupRunExternalProgram(); 					// Needed by WiFi_Run_External_Program
	CleanupRXVerifyPer();        					// Needed by WiFi_RX_Verify_Per
	CleanupTXCalibration();      					// Needed by WiFi_TX_Calibration
	CleanupTXVerifyEvm();        					// Needed by WiFi_TX_Verify_Evm
	CleanupTXVerifyMaskContainers();				// Needed by WiFi_TX_Verify_Mask
	CleanupTXVerifyPower();      					// Needed by WiFi_TX_Verify_Power
	CleanupTXVerifySpectrum();   					// Needed by WiFi_TX_Verify_Spectrum
	CleanupWriteBBRegister();    					// Needed by WiFi_Write_BB_Register
	CleanupWriteEeprom();        					// Needed by WiFi_Write_Eeprom
	CleanupWriteMacAddress();    					// Needed by WiFi_Write_Mac_Address
	CleanupWriteMacRegister();   					// Needed by WiFi_Write_Mac_Register
	CleanupWriteRFRegister();    					// Needed by WiFi_Write_RF_Register
	CleanupWriteSocRegister();   					// Needed by WiFi_Write_Soc_Register
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
	g_RecordedParam.FREQ_MHZ         = NA_INTEGER;	
	g_RecordedParam.POWER_DBM		 = NA_DOUBLE;		
	g_RecordedParam.FRAME_COUNT      = NA_INTEGER;	
	g_RecordedParam.BANDWIDTH[0]	 = '\0';
	g_RecordedParam.DATA_RATE[0]     = '\0';
	g_RecordedParam.PREAMBLE[0]      = '\0';
	g_RecordedParam.PACKET_FORMAT_11N[0] = '\0';
	g_RecordedParam.GUARD_INTERVAL_11N[0] = '\0';  // IQlite merge; Tracy Yu ; 2012-03-31
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
WIFI_MIMO_TEST_API int  RespondToQueryInput( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap)
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

	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] RespondToQueryReturn() return OK.\n");

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
				doubleVector.clear();  // IQlite merge; Tracy Yu ; 2012-03-31

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
			doubleVector.clear();  // IQlite merge; Tracy Yu ; 2012-03-31
        
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
	// Ignore error handling

	// IQlite merge; Tracy Yu ; 2012-03-31
	err = ::vDUT_Run( g_WiFi_Dut, "FUNCTION_FAILED" );
	// Ignore error handling

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
		sprintf_s(errMsg, sizeOfBuf, "[WiFi] LogReturnMessage() return error, Logger_ID < 0.\n");
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

	if(testersFW.size() >0)
	{
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
	}
	else
	{
		/*---------------------------------------------
		Version Map didn't contain any FW version info.
		----------------------------------------------*/
		//TODO: define error code
		err = -1;
	}
	

	if(testersVsaFpga.size() >0)
	{
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
	}
	else
	{
	    /*---------------------------------------------
		Version Map didn't contain any VSA FPGA version info.
		----------------------------------------------*/
		//TODO: define error code
		err = -1;

	}

	if(testersVsgFpga.size() >0)
	{
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
	}
	else
	{
	    /*---------------------------------------------
		Version Map didn't contain any VSG FPGA version info.
		----------------------------------------------*/
		//TODO: define error code
		err = -1;
	}

	testersFW.clear();
	testersVsaFpga.clear();
	testersVsgFpga.clear();

	return err;
}




int  WiFiTestMode(char* strDataRate, char* strBandwidth, int* wifiMode, int* wifiStreamNum)
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

int  WiFiSaveSigFile(char* fileName)
{
    int    err = ERR_OK;

	char    logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char    logInfoMessage[MAX_BUFFER_SIZE] = {'\0'};

	// IQlite merge; Tracy Yu ; 2012-03-31
	if (1==g_globalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
	{
		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi] The VSA_SAVE_CAPTURE_ALWAYS is set to ON, it may slow down the test time.\n");
	}

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
		char cFileExt[10] = {'\0'};
		if( g_iTesterName == IQTYPE_2010 )
			sprintf_s(cFileExt, 10, "%s", "sig");
		else if( g_iTesterName == IQTYPE_XEL )
			sprintf_s(cFileExt, 10, "%s", "iqvsa");

		sprintf_s(c_path, MAX_BUFFER_SIZE, "./log/%s-%s.%s", fileName, c_time, cFileExt);

		err = ::LP_SaveVsaSignalFile(c_path);

		sprintf_s(logInfoMessage, "[WiFi] WiFiSaveSigFile(\"%s\") return OK.\n", c_path);

		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, logInfoMessage);
	}
	else
	{
		// do nothing...
	}

	return err;
}

int  GetWaveformFileName(char* perfix, char* postfix, int wifiMode, char* bandwidth, char* datarate, char* preamble, char* packetFormat11N, char* guardInterval11N, char* waveformFileName, int bufferSize)
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
			sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_11P_%s_%s", perfix, datarate, postfix);
		} else {
			sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_11AG_%s_%s", perfix, datarateString, postfix);
		}
	}
	else	// WIFI_11N_HT20 or WIFI_11N_HT40
	{	
		/*****************************************************************
		* For backward compatibility,									 *			  
		*we use old waveform names for mixed, long guard interval format *
		*  IQlite merge; Tracy Yu ; 2012-03-31                           *
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

		//if Tester is IQxel and file extension is .ref change file extension from .ref to .iqref
		if( g_iTesterName == IQTYPE_XEL )
		{
			char *pch = NULL;
			pch = strstr(waveformFileName,".ref\0");
			if(pch!= NULL)
				strncpy (pch,".iqref\0",7);
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
    }

	return err;
}

// IQlite merge; Tracy Yu ; 2012-03-31
int  GetPacketNumber(int wifiMode, char* bandwidth, char* datarate, char* packetFormat11N, char* guardInterval11N, int *packetNumber)
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
			sprintf_s(keyword, MAX_BUFFER_SIZE, "PER_11P_%s_PACKETS_NUM", datarate);
		} else {
			sprintf_s(keyword, MAX_BUFFER_SIZE, "PER_11AG_%s_PACKETS_NUM", datarateString);
		}
	}
	else	// WIFI_11N_HT20 or WIFI_11N_HT40
	{		
		/*****************************************************************
		* For backward compatibility,									 *			  
		*we use old waveform names for mixed, long guard interval format *
		*   IQlite merge; Tracy Yu ; 2012-03-31					         *		
		******************************************************************/
		
		if (0==strcmp(guardInterval11N, "LONG") && 0==strcmp(packetFormat11N, "MIXED"))  // 802.11n, mixed format, long preamble, use old default waveform name
		{
			sprintf_s(keyword, MAX_BUFFER_SIZE, "PER_11N_%s_%s_%s_PACKETS_NUM", bandwidth, packetFormat11N, datarate);	
		}
		else if (0==strcmp(guardInterval11N, "LONG") && 0==strcmp(packetFormat11N, "GREENFIELD"))  //802.11n long guard interval, greefield
		{
			sprintf_s(keyword, MAX_BUFFER_SIZE, "PER_11N_%s_%s_L_GI_%s_PACKETS_NUM", bandwidth, packetFormat11N, datarate);	
		}
		else      //802.11n short guard interval, mixed/greenfield
		{
			sprintf_s(keyword, MAX_BUFFER_SIZE, "PER__11N_%s_%s_S_GI_%s_PACKETS_NUM", bandwidth, packetFormat11N, datarate);	
		}	
	}

    // Searching the keyword in g_globalSettingParamMap
	inputMap_Iter = g_globalSettingParamMap.find(keyword);
    if( inputMap_Iter!=g_globalSettingParamMap.end() )
    {
		*packetNumber = *((int*)inputMap_Iter->second.value);
	}
    else
    {
        err = TM_ERR_PARAM_DOES_NOT_EXIST;
    }

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
			if ( ant[i] )	// Path enabled  // IQlite merge; Tracy Yu ; 2012-03-31
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
			else	// This path not enabled;   // IQlite merge; Tracy Yu ; 2012-03-31
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

WIFI_MIMO_TEST_API int GetWiFiTestID(TM_ID* wifiTestID)
{
	*wifiTestID = g_WiFi_Test_ID;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int SetWiFiTestID(TM_ID wifiTestID)
{
	g_WiFi_Test_ID = wifiTestID;
	return ERR_OK;
}


WIFI_MIMO_TEST_API int   GetWiFiDutID(vDUT_ID* wifiDutID)
{
    *wifiDutID = g_WiFi_Dut;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int   GetWiFiGlobalSetting(WIFI_GLOBAL_SETTING** wifiGlobleSetting)
{
    *wifiGlobleSetting = &g_globalSettingParam;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int   SetTesterNumber(int testerNumber)
{
    g_Tester_Number = testerNumber;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int   GetTesterNumber(int *testerNumber)
{
    *testerNumber = g_Tester_Number;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int   SetTesterType(int testerType)
{
    g_Tester_Type = testerType;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int   GetTesterType(int *testerType)
{
    *testerType = g_Tester_Type;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int   SetTesterReconnect(int flag)
{
    g_Tester_Reconnect = flag;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int   GetTesterReconnect(int *flag)
{
    *flag = g_Tester_Reconnect;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int   SetDutConfigChanged(bool changed)
{
    g_dutConfigChanged = changed;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int   GetDutConfigChanged(bool *changed)
{
	*changed = g_dutConfigChanged;
	return ERR_OK;
}

WIFI_MIMO_TEST_API int GetDutTxActived(bool* vDutTxActived)
{
	*vDutTxActived = g_vDutTxActived;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int SetDutTxActived(bool vDutTxActived)
{
	g_vDutTxActived = vDutTxActived;
    return ERR_OK;
}

WIFI_MIMO_TEST_API int GetTesterName(int *flag)
{
	*flag = g_iTesterName;
	return ERR_OK;
}

WIFI_MIMO_TEST_API int SetTesterName(int flag)
{
	g_iTesterName = flag;
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
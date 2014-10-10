// IQlite_Demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TestManager.h"

#define STRICT
#include <windows.h>
#include <iostream>

using namespace std;


/*------------------*
 * global variables *
 *------------------*/
TM_ID   WiFi_Test = -1;
TM_ID   tmID[MAX_TECHNOLOGY_NUM] = {-1, -1, -1, -1, -1, -1, -1};

int  ListAllFunction();
int  SimpleWiFiTestFlow(_TCHAR* ipAddress);
void GetTestID(int technologyIndex, int* testID);

const char  *g_IQlite_Demo_Version = "1.5.0 RC-1 (2010-04-26)\n";


int _tmain(int argc, _TCHAR* argv[])
{
    if( argc>1 )
    {
        if( 0==wcscmp(argv[1],_T("-info")) )
        {
			// <I> This is an example to show how "Query Input" and "Query Return" works.
            ListAllFunction();
            printf("Press ENTER to exit...\n");
            _getch();
            return 0;
        }

        if( 0==wcscmp(argv[1],_T("-test")) )
        {
            if( argc>2 )
            {
				// <II> This is an example of how to use IQlite.
                SimpleWiFiTestFlow(argv[2]);
            }
            else
            {
                printf("\n A valid IP address has to be specified.\n\n");
            }
            printf("Press ENTER to exit...\n");
            _getch();
            return 0;
        }
    }

    printf("options:\n\n\t-info : list of all implemented test functions along with input parameters and return values");
	printf("\n\t-test ip_address[ex:192.168.100.254]: to run a simple WiFi test\n\n");
    printf("Press ENTER to exit...\n");
    _getch();

	return 0;
}



//---------------------------------------------------------------------------------
// <I> This is an example to show how "Query Input" and "Query Return" work.
//---------------------------------------------------------------------------------
int ListAllFunction()
{
    TM_RETURN ret;
    TM_ID testID;

    char techName[MAX_BUFFER_SIZE];
    int  technologyIndex, functionNum;
    char functionKeyword[MAX_BUFFER_SIZE];
    char paramName[MAX_BUFFER_SIZE];
    BOOL callbackInstalled;

	//--------------------------------------------------------------------------
	// Get Technology Information managed by Test Manager
	//--------------------------------------------------------------------------	
    ret = ::TM_GetTechnologyInfo(techName, MAX_BUFFER_SIZE, &technologyIndex, &functionNum, FIRST);
    while( TM_ERR_OK==ret )
    {
        printf("%s(%d):\n", techName, functionNum);

        testID = -1;
        if(0<functionNum)
        {
            GetTestID( technologyIndex, &testID );
            if( 0>testID )
            {
                printf("\nFailed to load the %s_Test.DLL\n", techName);
				ret = ::TM_GetTechnologyInfo(techName, MAX_BUFFER_SIZE, &technologyIndex, &functionNum, NEXT);
                continue;
            }
			// Get Function kyeword
            ret = ::TM_GetFunctionKeyword(techName, functionKeyword, MAX_BUFFER_SIZE, &callbackInstalled, FIRST);
            while( TM_ERR_OK==ret )
            {
                //printf("\t%s(%s)\n", functionKeyword, (TRUE==callbackInstalled)?"Yes":"No");
                if( TRUE==callbackInstalled )
                {
                    printf("\n\t%s\n", functionKeyword);

					//--------------------------------------------------------------------------
					// Query input parameters and return values
					//--------------------------------------------------------------------------   
                    bool commentLineIsNeeded;
                    commentLineIsNeeded = true;
                    if( testID>=0 && testID<MAX_TECHNOLOGY_NUM )
                    {
					/*--------------*
					 * Query inputs *
					 *--------------*/

					//--------------------------------------------------------------------------
					// (1) Clear all input parameter containers for the specified technology.
					//--------------------------------------------------------------------------
                        ::TM_ClearParameters(testID);

					//--------------------------------------------------------------------------
					// (2) Add input parameters into containers.
					//--------------------------------------------------------------------------
                        ret = ::TM_AddIntegerParameter(testID, "QUERY_INPUT", 1);   // QUERY_INPUT can ignores the integer value

					//--------------------------------------------------------------------------
					// (3) Run the "QUERY_INPUT" function.
					//--------------------------------------------------------------------------
                        ret = ::TM_Run(testID, functionKeyword);

					//--------------------------------------------------------------------------
					// (4) List all integer parameters.
					//--------------------------------------------------------------------------
                        ret = ::TM_GetIntegerReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, FIRST);
                        while( TM_ERR_OK==ret )
                        {
                            if( commentLineIsNeeded )
                            {
                                printf("\t\t#Input Parameters:\n");
                                commentLineIsNeeded = false;
                            }
                            printf("\t\t>%s [Integer] = \n", paramName);
                            ret = ::TM_GetIntegerReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, NEXT);
                        }
					//--------------------------------------------------------------------------
					// (5) List all double parameters.
					//--------------------------------------------------------------------------              
                        ret = ::TM_GetDoubleReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, FIRST);
                        while( TM_ERR_OK==ret )
                        {
                            if( commentLineIsNeeded )
                            {
                                printf("\t\t#Input Parameters:\n");
                                commentLineIsNeeded = false;
                            }
                            printf("\t\t>%s [Double] = \n", paramName);
                            ret = ::TM_GetDoubleReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, NEXT);
                        }
					//--------------------------------------------------------------------------
					// (6) List all string parameters.
					//-------------------------------------------------------------------------- 
                        ret = ::TM_GetStringReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, 0, FIRST);
                        while( TM_ERR_OK==ret )
                        {
                            if( commentLineIsNeeded )
                            {
                                printf("\t\t#Input Parameters:\n");
                                commentLineIsNeeded = false;
                            }
                            printf("\t\t>%s [String] = \n", paramName);
                            ret = ::TM_GetStringReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, 0, NEXT);
                        }



                    /*---------------*
                     * Query Returns *
                     *---------------*/
                        commentLineIsNeeded = true;

					//--------------------------------------------------------------------------
					// (1) Clear all input parameter containers for the specified technology.
					//--------------------------------------------------------------------------
                        ::TM_ClearParameters(testID);

					//--------------------------------------------------------------------------
					// (2) Add input parameters into containers.
					//--------------------------------------------------------------------------
                        ret = ::TM_AddIntegerParameter(testID, "QUERY_RETURN", 1);   // QUERY_RETURN can ignores the integer value

					//--------------------------------------------------------------------------
					// (3) Run the "QUERY_RETURN" function.
					//--------------------------------------------------------------------------
                        ret = ::TM_Run(testID, functionKeyword);

					//--------------------------------------------------------------------------
					// (4) List all Integer returns.
					//--------------------------------------------------------------------------
                        ret = ::TM_GetIntegerReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, FIRST);
                        while( TM_ERR_OK==ret )
                        {
                            if( commentLineIsNeeded )
                            {
                                printf("\t\t#Return Values:\n");
                                commentLineIsNeeded = false;
                            }
                            printf("\t\t<%s [Integer] = <,>\n", paramName);
                            ret = ::TM_GetIntegerReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, NEXT);
                        }

					//--------------------------------------------------------------------------
					// (5) List all Double returns.
					//--------------------------------------------------------------------------
                        ret = ::TM_GetDoubleReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, FIRST);
                        while( TM_ERR_OK==ret )
                        {
                            if( commentLineIsNeeded )
                            {
                                printf("\t\t#Return Values:\n");
                                commentLineIsNeeded = false;
                            }
                            printf("\t\t<%s [Double] = <,>\n", paramName);
                            ret = ::TM_GetDoubleReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, NEXT);
                        }

					//--------------------------------------------------------------------------
					// (6) List all String returns.
					//--------------------------------------------------------------------------
                        ret = ::TM_GetStringReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, 0, FIRST);
                        while( TM_ERR_OK==ret )
                        {
                            if( commentLineIsNeeded )
                            {
                                printf("\t\t#Return Values:\n");
                                commentLineIsNeeded = false;
                            }
                            printf("\t\t<%s [String] = <,>\n", paramName);
                            ret = ::TM_GetStringReturnPair(testID, paramName, MAX_BUFFER_SIZE, NULL, 0, NULL, 0, NULL, 0, NEXT);
                        }

                    }
                }
                ret = ::TM_GetFunctionKeyword(techName, functionKeyword, MAX_BUFFER_SIZE, &callbackInstalled, NEXT);
            }
        }
        ret = ::TM_GetTechnologyInfo(techName, MAX_BUFFER_SIZE, &technologyIndex, &functionNum, NEXT);
    }
    return 0;
}

void GetTestID(int technologyIndex, int* testID)
{
    *testID = -1;

    int ret = -1;

    switch(technologyIndex)
    {
    case WIFI:
        if( -1>=tmID[WIFI] )
        {
            ret = ::TM_RegisterTechnologyDll("WIFI", "WiFi_Test.DLL", &tmID[WIFI]);
            if( 0>tmID[WIFI] )
            {
                //MessageBox::Show( "Failed to install WiFi_Test.DLL", "IQlite",
                //    MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
            }
            else
            {
                //printf("OK to register WiFi Test\n");
                *testID = tmID[WIFI];
            }
        }
        else
        {
            // WiFi Test has registered somewhere else
            *testID = tmID[WIFI];
        }
        break;
    case WIFI_MIMO:
        if( -1>=tmID[WIFI_MIMO] )
        {
            ret = ::TM_RegisterTechnologyDll("WIFI_MIMO", "WIFI_MIMO_Test.DLL", &tmID[WIFI_MIMO]);
            if( 0>tmID[WIFI_MIMO] )
            {
                //MessageBox::Show( "Failed to install WIFI_MIMO_Test.DLL", "IQlite",
                //    MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
            }
            else
            {
                //printf("OK to register WIFI_MIMO Test\n");
                *testID = tmID[WIFI_MIMO];
            }
        }
        else
        {
            // WIFI_MIMO Test has registered somewhere else
            *testID = tmID[WIFI_MIMO];
        }
        break;
    case BT:
		 if( -1>=tmID[BT] )
        {
            ret = ::TM_RegisterTechnologyDll("BT", "BT_Test.DLL", &tmID[BT]);
            if( 0>tmID[BT] )
            {
                //MessageBox::Show( "Failed to install WiFi_Test.DLL", "IQlite",
                //    MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
            }
            else
            {
                //printf("OK to register WiFi Test\n");
                *testID = tmID[BT];
            }
        }
        else
        {
            // WiFi test has registered somewhere else
            *testID = tmID[BT];
        }
        break;		
    case IQREPORT:
        break;
    case WIMAX:
        if( -1>=tmID[WIMAX] )
        {
            ret = ::TM_RegisterTechnologyDll("WIMAX", "WIMAX_Test.DLL", &tmID[WIMAX]);
            if( 0>tmID[WIMAX] )
            {
                //MessageBox::Show( "Failed to install WIMAX_Test.DLL", "IQlite",
                //    MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
            }
            else
            {
                //printf("OK to register WIMAX Test\n");
                *testID = tmID[WIMAX];
            }
        }
        else
        {
            // WIMAX Test has registered somewhere else
            *testID = tmID[WIMAX];
        }
        break;
    case GPS:
        if( -1>=tmID[GPS] )
        {
            ret = ::TM_RegisterTechnologyDll("GPS", "GPS_Test.DLL", &tmID[GPS]);
            if( 0>tmID[GPS] )
            {
                //MessageBox::Show( "Failed to install GPS_Test.DLL", "IQlite",
                //    MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
            }
            else
            {
                //printf("OK to register GPS Test\n");
                *testID = tmID[GPS];
            }
        }
        else
        {
            // GPS Test has registered somewhere else
            *testID = tmID[GPS];
        }
        break;
    case FM:
        if( -1>=tmID[FM] )
        {
            ret = ::TM_RegisterTechnologyDll("FM", "FM_Test.DLL", &tmID[FM]);
            if( 0>tmID[FM] )
            {
                //MessageBox::Show( "Failed to install FM_Test.DLL", "IQlite",
                //    MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
            }
            else
            {
                //printf("OK to register FM Test\n");
                *testID = tmID[FM];
            }
        }
        else
        {
            // FM Test has registered somewhere else
            *testID = tmID[FM];
        }
        break;
    default:
        break;
    }
}


//---------------------------------------------------------------------------------
// <II> This is an example of how to use IQlite.
//---------------------------------------------------------------------------------
int SimpleWiFiTestFlow(_TCHAR* ipAddress)
{
    int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};

    try
    {
        // Step 1: Register the WiFi Test functions with TestManager
        //         This ensures that all WiFi Test functions are made available to the  Test Manager
		run_RegisterTechnologyDll();


        // Step 2: Seetup the global settings
        //         Global settings are used by all test functions
        //         They do not belong to any specific test function
		run_GLOBAL_SETTINGS();


        // Step 3: Load path loss table from file
		run_LOAD_PATH_LOSS_TABLE();


        // Step 4: Connect IQTester (IP address needed)
		run_CONNECT_IQ_TESTER(ipAddress);


        // Step 5: Insert DUT for test
		run_INSERT_DUT();


        // Step 6: Initial DUT for test
		run_INITIALIZE_DUT();


        // Step 7: TX Verify EVM Test     
		run_TX_VERIFY_EVM();


        // Step 8: TX Verify Power Test     
		run_TX_VERIFY_POWER();


        // Step 9: TX Verify Mask Test     
		//run_TX_VERIFY_MASK();


        // Step 10: TX Verify Spectrum Test     
		run_TX_VERIFY_SPECTRUM();


        // Step 11: RX Verify PER Test   
		run_RX_VERIFY_PER();   


        // Step 12: Remove DUT
		run_REMOVE_DUT();


        // Step 13: Disconnect IQTester
		run_DISCONNECT_IQ_TESTER();        
	}
	catch(char *msg)
    {
        printf(msg);
    }
    catch(...)
    {
		printf("[Error] Unknown Error!\n");
    }


    return err;   
}


//---------------------------------------------------------------------------------
// Step 1: Register the WiFi Test functions with TestManager
//         This ensures that all WiFi Test functions are made available to the Test Manager
//---------------------------------------------------------------------------------
void run_RegisterTechnologyDll(void)
{
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};

	printf("\n\n__________________________________________________________________\n");
	printf("[Step 1] Register the WiFi Test functions with TestManager.\n");
	//--------------------------------------------------------------------------
	// (1) Register the WiFi Test functions with TestManager.
	//--------------------------------------------------------------------------
    ::TM_RegisterTechnologyDll("WIFI", "WiFi_Test.DLL", &WiFi_Test);
    if( WiFi_Test<0 ) 
	{
		sprintf_s(errMsg, MAX_BUFFER_SIZE, "[Error] Fail to register the WiFi Test function with WiFi_Test.DLL.\n");
		throw errMsg;
	}
    else
	{
        printf("\t[Info] TM_RegisterTechnologyDll() return OK.\n");
	}
	
	return;
}


//---------------------------------------------------------------------------------
// Step 2: Setup the global settings
//         Global settings are used by all test functions
//         They do not belong to any specific test function
//---------------------------------------------------------------------------------
void run_GLOBAL_SETTINGS(void)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	int		valueInteger = 0;
	double	valueDouble = 0.0;
	char	valueString[MAX_BUFFER_SIZE] = {'\0'};

	printf("\n\n__________________________________________________________________\n");
	printf("[Step 2] Setup the global settings.\n");

	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------
    TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
    TM_AddIntegerParameter(WiFi_Test, "DUT_KEEP_TRANSMIT", 0); 
	TM_AddIntegerParameter(WiFi_Test, "VSA_PORT", 2); 
	TM_AddIntegerParameter(WiFi_Test, "VSG_PORT", 3); 
	TM_AddIntegerParameter(WiFi_Test, "IQ2010_EXT_ENABLE", 0);	

	//--------------------------------------------------------------------------
	// (3) Users can now run the specific "Keyword" function.
	//--------------------------------------------------------------------------
	err = TM_Run(WiFi_Test, "GLOBAL_SETTINGS");  
    if( 0!=err ) 
    {	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{
		printf("\t[Info] TM_Run(GLOBAL_SETTINGS) return OK.\n");
	}

	return;
}

LPSTR UnicodeToAnsi(LPCWSTR str)
{
	if (str==NULL) 
	{
		return NULL;
	}
	else
	{
		int cw = lstrlenW(str);
		if (cw==0) 
		{
			CHAR *psz=new CHAR[1];
			*psz='\0';
			return psz;
		}
		else
		{
			int cc = WideCharToMultiByte(CP_ACP,0,str,cw,NULL,0,NULL,NULL);
			if (cc==0) return NULL;

			CHAR *psz = new CHAR[cc+1];
			cc = WideCharToMultiByte(CP_ACP,0,str,cw,psz,cc,NULL,NULL);
			if (cc==0) 
			{
				delete[] psz;
				return NULL;
			}
			else
			{
				psz[cc]='\0';
				return psz;
			}
		}
	}	
}


//---------------------------------------------------------------------------------
// Step 3: Load path loss table from file
//---------------------------------------------------------------------------------
void run_LOAD_PATH_LOSS_TABLE(void)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};

	printf("\n\n__________________________________________________________________\n");
	printf("[Step 3] LOAD_PATH_LOSS_TABLE Test.\n");

	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
	TM_AddStringParameter (WiFi_Test, "TX_PATH_LOSS_FILE", "path_loss.csv");
	TM_AddStringParameter (WiFi_Test, "RX_PATH_LOSS_FILE", "path_loss.csv");

	//--------------------------------------------------------------------------
	// (3) Users can now run the specific "Keyword" function.
	//--------------------------------------------------------------------------
	err = TM_Run(WiFi_Test, "LOAD_PATH_LOSS_TABLE");
	if( 0!=err ) 
	{	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{
		printf("\t[Info] TM_Run(LOAD_PATH_LOSS_TABLE) return OK.\n");
	}

	return;
}


//---------------------------------------------------------------------------------
// Step 4: Connect IQTester (IP address needed)
//---------------------------------------------------------------------------------
void run_CONNECT_IQ_TESTER(_TCHAR* ipAddress)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	char	valueString[MAX_BUFFER_SIZE] = {'\0'};
    
	printf("\n\n__________________________________________________________________\n");
	printf("[Step 4] Connect IQTester (IP address needed).\n");

	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
	LPSTR ip = UnicodeToAnsi(ipAddress);
    TM_AddStringParameter(WiFi_Test, "IQTESTER_IP01", ip); 
    TM_AddStringParameter(WiFi_Test, "IQTESTER_IP02", "");
    TM_AddStringParameter(WiFi_Test, "IQTESTER_IP03", "");
    TM_AddStringParameter(WiFi_Test, "IQTESTER_IP04", "");
    TM_AddIntegerParameter(WiFi_Test, "IQTESTER_RECONNECT", 0);
	TM_AddIntegerParameter(WiFi_Test, "IQTESTER_TYPE", 0);
    //TM_AddIntegerParameter(WiFi_Test, "DH_ENABLE", 0);
    //TM_AddIntegerParameter(WiFi_Test, "DH_OBTAIN_CONTROL_TIMEOUT_MS", 0);
    //TM_AddIntegerParameter(WiFi_Test, "DH_PROBE_TIME_MS", 100);
    //TM_AddIntegerParameter(WiFi_Test, "DH_TOKEN_ID", 1);

	//--------------------------------------------------------------------------
	// (3) Users can now run the specific "Keyword" function.
	//--------------------------------------------------------------------------
    err = TM_Run(WiFi_Test, "CONNECT_IQ_TESTER");
    if( 0!=err ) 
    {	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{		
	//--------------------------------------------------------------------------
	// (4) If no error occurs, users can retrieve results as follows:
	//--------------------------------------------------------------------------
		// An Example to Retrieve results
		TM_GetStringReturn(WiFi_Test, "IQTESTER_INFO", valueString, MAX_BUFFER_SIZE);
		printf("\t[#] IQTESTER_INFO \t\t: \n%s\n", valueString);
		TM_GetStringReturn(WiFi_Test, "TEST_MANAGER_VERSION", valueString, MAX_BUFFER_SIZE);
		printf("\t[#] TEST_MANAGER_VERSION \t: %s", valueString);
		
		printf("\t[Info] TM_Run(CONNECT_IQ_TESTER) return OK.\n");
	}
	delete[] ip;

	return;
}


//---------------------------------------------------------------------------------
// Step 5: Insert DUT for test
//---------------------------------------------------------------------------------
void run_INSERT_DUT(void)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	char	valueString[MAX_BUFFER_SIZE] = {'\0'};
    
	printf("\n\n__________________________________________________________________\n");
	printf("[Step 5] Insert DUT for test.\n");
	
	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------	
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
	TM_AddIntegerParameter(WiFi_Test, "RELOAD_DUT_DLL", 1);
    TM_AddStringParameter (WiFi_Test, "DUT_DLL_FILENAME", "LP_Dut.dll");
	TM_AddStringParameter (WiFi_Test, "CONNECTION_STRING", "");
	TM_AddStringParameter (WiFi_Test, "EEPROM_FILENAME", "");
	TM_AddStringParameter (WiFi_Test, "OPTION_STRING", "");
	//--------------------------------------------------------------------------
	// (3) Users can now run the specific "Keyword" function.
	//--------------------------------------------------------------------------
    err = TM_Run(WiFi_Test, "INSERT_DUT");
    if( 0!=err ) 
    {	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{
	//--------------------------------------------------------------------------
	// (4) If no error occurs, users can retrieve results as follows:
	//--------------------------------------------------------------------------
		// An Example to Retrieve results
		TM_GetStringReturn(WiFi_Test, "DUT_VERSION", valueString, MAX_BUFFER_SIZE);
		printf("\t[#] DUT_VERSION \t\t: %s", valueString);
		TM_GetStringReturn(WiFi_Test, "VDUT_VERSION", valueString, MAX_BUFFER_SIZE);
		printf("\t[#] VDUT_VERSION \t\t: %s", valueString);

		printf("\t[Info] TM_Run(INSERT_DUT) return OK.\n");
	}

	return;
}


//---------------------------------------------------------------------------------
// Step 6: Initialize DUT for testing
//---------------------------------------------------------------------------------
void run_INITIALIZE_DUT(void)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	char	valueString[MAX_BUFFER_SIZE] = {'\0'};
    
	printf("\n\n__________________________________________________________________\n");
	printf("[Step 6] Initial DUT for test.\n");

	//--------------------------------------------------------------------------
	// (1) Users can run the specific "Keyword" function directly.
	//--------------------------------------------------------------------------
    err = TM_Run(WiFi_Test, "INITIALIZE_DUT");
    if( 0!=err ) 
    {	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{
	//--------------------------------------------------------------------------
	// (2) If no error occurs, user can retrieve results as follows:	
	//--------------------------------------------------------------------------
		// An example to retrieve results
		TM_GetStringReturn(WiFi_Test, "DUT_NAME", valueString, MAX_BUFFER_SIZE);
		printf("\t[#] DUT_NAME \t\t\t: %s\n", valueString);
		TM_GetStringReturn(WiFi_Test, "DUT_DRIVER_INFO", valueString, MAX_BUFFER_SIZE);
		printf("\t[#] DUT_DRIVER_INFO \t\t: \n%s\n", valueString);

		printf("\t[Info] TM_Run(INITIALIZE_DUT) return OK.\n");
	}

	return;
}


//---------------------------------------------------------------------------------
// Step 7: TX Verify EVM Test
//---------------------------------------------------------------------------------
void run_TX_VERIFY_EVM(void)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	double	valueDouble = 0.0;
        
	printf("\n\n__________________________________________________________________\n");
	printf("[Step 7] TX Verify EVM Test.\n");
    
	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------	
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
    TM_AddIntegerParameter(WiFi_Test, "FREQ_MHZ", 2437);
    TM_AddIntegerParameter(WiFi_Test, "TX1", 1);
    TM_AddIntegerParameter(WiFi_Test, "TX2", 0);
    TM_AddIntegerParameter(WiFi_Test, "TX3", 0);
    TM_AddIntegerParameter(WiFi_Test, "TX4", 0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_1", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_2", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_3", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_4", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",100);
	TM_AddDoubleParameter (WiFi_Test, "TX_POWER_DBM", -20);
    TM_AddStringParameter (WiFi_Test, "BANDWIDTH", "HT20");
    TM_AddStringParameter (WiFi_Test, "DATA_RATE", "OFDM-54");
	TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT_11N", "MIXED");
	TM_AddStringParameter (WiFi_Test, "PREAMBLE", "LONG");
	TM_AddStringParameter (WiFi_Test, "GUARD_INTERVAL_11N", "LONG");

	//--------------------------------------------------------------------------
	// (3) Users can now run the specific "Keyword" function.
	//--------------------------------------------------------------------------
    err = TM_Run(WiFi_Test, "TX_VERIFY_EVM");
    if( 0!=err ) 
    {	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{
	//--------------------------------------------------------------------------
	// (4) If no error occurs, users can retrieve results as follows:
	//--------------------------------------------------------------------------
		// An Example to Retrieve results
		TM_GetDoubleReturn(WiFi_Test, "AMP_ERR_DB", &valueDouble);
		printf("\t[#] AMP_ERR_DB \t\t\t: %.2f dB\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "DATA_RATE", &valueDouble);
		printf("\t[#] DATA_RATE \t\t\t: %.2f Mbps\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "EVM_AVG_DB", &valueDouble);
		printf("\t[#] EVM_AVG_DB \t\t\t: %.2f dB\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "EVM_PK_DB", &valueDouble);
		printf("\t[#] EVM_PK_DB \t\t\t: %.2f dB\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "FREQ_ERROR_AVG", &valueDouble);
		printf("\t[#] FREQ_ERROR_AVG \t\t: %.2f ppm\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "FREQ_ERROR_MAX", &valueDouble);
		printf("\t[#] FREQ_ERROR_MAX \t\t: %.2f ppm\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "FREQ_ERROR_MIN", &valueDouble);
		printf("\t[#] FREQ_ERROR_MIN \t\t: %.2f ppm\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "PHASE_ERR", &valueDouble);
		printf("\t[#] PHASE_ERR \t\t\t: %.2f Degree\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "POWER_AVG_DBM", &valueDouble);
		printf("\t[#] POWER_AVG_DBM \t\t: %.2f dBm\n", valueDouble);


		printf("\t[Info] TM_Run(TX_VERIFY_EVM) return OK.\n");
	}

	return;
}


//---------------------------------------------------------------------------------
// Step 8: TX Verify Power Test
//---------------------------------------------------------------------------------
void run_TX_VERIFY_POWER(void)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	double	valueDouble = 0.0;
	     
	printf("\n\n__________________________________________________________________\n");
	printf("[Step 8] TX Verify Power Test.\n");
    
	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------	
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
    TM_AddIntegerParameter(WiFi_Test, "FREQ_MHZ", 2437);
    TM_AddIntegerParameter(WiFi_Test, "TX1", 1);
    TM_AddIntegerParameter(WiFi_Test, "TX2", 0);
    TM_AddIntegerParameter(WiFi_Test, "TX3", 0);
    TM_AddIntegerParameter(WiFi_Test, "TX4", 0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_1", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_2", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_3", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_4", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",100);
	TM_AddDoubleParameter (WiFi_Test, "TX_POWER_DBM", -20);
    TM_AddStringParameter (WiFi_Test, "BANDWIDTH", "HT20");
    TM_AddStringParameter (WiFi_Test, "DATA_RATE", "OFDM-54");
	TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT_11N", "MIXED");
	TM_AddStringParameter (WiFi_Test, "PREAMBLE", "LONG");
	TM_AddStringParameter (WiFi_Test, "GUARD_INTERVAL_11N", "LONG");
	//--------------------------------------------------------------------------
	// (3) Users can now run the specific "Keyword" function.
	//--------------------------------------------------------------------------
    err = TM_Run(WiFi_Test, "TX_VERIFY_POWER");
    if( 0!=err ) 
    {	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{
	//--------------------------------------------------------------------------
	// (4) If no error occurs, users can retrieve results as follows:
	//--------------------------------------------------------------------------
		// An example to retrieve the results
		TM_GetDoubleReturn(WiFi_Test, "POWER_AVERAGE_DBM", &valueDouble);
		printf("\t[#] POWER_AVERAGE_DBM \t\t: %.2f dBm\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "POWER_AVERAGE_MAX_DBM", &valueDouble);
		printf("\t[#] POWER_AVERAGE_MAX_DBM \t: %.2f dBm\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "POWER_AVERAGE_MIN_DBM", &valueDouble);
		printf("\t[#] POWER_AVERAGE_MIN_DBM \t: %.2f dBm\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "POWER_PEAK_DBM", &valueDouble);
		printf("\t[#] POWER_PEAK_DBM \t\t: %.2f dBm\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "POWER_PEAK_MAX_DBM", &valueDouble);
		printf("\t[#] POWER_PEAK_MAX_DBM \t\t: %.2f dBm\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "POWER_PEAK_MIN_DBM", &valueDouble);
		printf("\t[#] POWER_PEAK_MIN_DBM \t\t: %.2f dBm\n", valueDouble);

		printf("\t[Info] TM_Run(TX_VERIFY_POWER) return OK.\n");
	}

	return;
}


//---------------------------------------------------------------------------------
// Step 9: TxVerify Mask Test
//---------------------------------------------------------------------------------
void run_TX_VERIFY_MASK(void)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	double	valueDouble = 0.0;
	int		arraySize = 0;
	double* arrayDouble = NULL;
	char    strQuery[MAX_BUFFER_SIZE] = {'\0'};
	char    strResult[MAX_BUFFER_SIZE] = {'\0'};
	     
	printf("\n\n__________________________________________________________________\n");
	printf("[Step 9] TX Verify Mask Test.\n");
    
	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------	
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
    TM_AddIntegerParameter(WiFi_Test, "FREQ_MHZ", 2437);
    TM_AddIntegerParameter(WiFi_Test, "TX1", 1);
    TM_AddIntegerParameter(WiFi_Test, "TX2", 0);
    TM_AddIntegerParameter(WiFi_Test, "TX3", 0);
    TM_AddIntegerParameter(WiFi_Test, "TX4", 0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_1", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_2", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_3", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_4", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "OBW_PERCENTAGE", 99.0);
	TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",400);
	TM_AddDoubleParameter (WiFi_Test, "TX_POWER_DBM", -20);
    TM_AddStringParameter (WiFi_Test, "BANDWIDTH", "HT20");
    TM_AddStringParameter (WiFi_Test, "DATA_RATE", "OFDM-54");
	TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT_11N", "MIXED");
	TM_AddStringParameter (WiFi_Test, "PREAMBLE", "LONG");
	TM_AddStringParameter (WiFi_Test, "GUARD_INTERVAL_11N", "LONG");

	//--------------------------------------------------------------------------
	// (3) Users can now run the specific "Keyword" function.
	//--------------------------------------------------------------------------
    err = TM_Run(WiFi_Test, "TX_VERIFY_MASK");
    if( 0!=err ) 
    {	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{
	//--------------------------------------------------------------------------
	// (4) If no error occurs, user can retrieve results as follows:
	//--------------------------------------------------------------------------
		// An example to retrieve results
		//for (int iIndexX = 1; iIndexX < 5; iIndexX++)
		//{
		//	sprintf_s(strQuery, "FREQ_AT_MARGIN_LOWER_%d", iIndexX);
		//	TM_GetDoubleReturn(WiFi_Test, strQuery, &valueDouble);
		//	sprintf_s(strResult, "\t[#] %s \t: %.2f MHz\n", strQuery);
		//	printf(strResult);
		//	
		//	sprintf_s(strQuery, "FREQ_AT_MARGIN_UPPER_%d", iIndexX);
		//	TM_GetDoubleReturn(WiFi_Test, strQuery, &valueDouble);
		//	sprintf_s(strResult, "\t[#] %s \t: %.2f MHz\n", strQuery);
		//	printf(strResult);

		//	sprintf_s(strQuery, "MARGIN_DB_LOWER_%d", iIndexX);
		//	TM_GetDoubleReturn(WiFi_Test, strQuery, &valueDouble);
		//	sprintf_s(strResult, "\t[#] %s \t\t: %.2f dB\n", strQuery);
		//	printf(strResult);

		//	sprintf_s(strQuery, "MARGIN_DB_UPPER_%d", iIndexX);
		//	TM_GetDoubleReturn(WiFi_Test, strQuery, &valueDouble);
		//	sprintf_s(strResult, "\t[#] %s \t\t: %.2f dB\n", strQuery);
		//	printf(strResult);
		//}

		TM_GetDoubleReturn(WiFi_Test, "OBW_FREQ_START_MHZ", &valueDouble);
		printf("\t[#] OBW_FREQ_START_MHZ \t\t: %.2f MHZ\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "OBW_FREQ_STOP_MHZ", &valueDouble);
		printf("\t[#] OBW_FREQ_STOP_MHZ \t\t: %.2f MHZ\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "OBW_MHZ", &valueDouble);
		printf("\t[#] OBW_MHZ \t\t\t: %.2f MHZ\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "VIOLATION_PERCENT", &valueDouble);
		printf("\t[#] VIOLATION_PERCENT \t\t: %.2f %c\n", valueDouble, '%');

		printf("\t[Info] TM_Run(TX_VERIFY_MASK) return OK.\n");
	}

	return;
}


//---------------------------------------------------------------------------------
// Step 10: Tx Verify Spectrum Test
//---------------------------------------------------------------------------------
void run_TX_VERIFY_SPECTRUM(void)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	int		valueInteger = 0;
	double	valueDouble = 0.0;
         
	printf("\n\n__________________________________________________________________\n");
	printf("[Step 10] TX Verify Spectrum Test.\n");
    
	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------	
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
    TM_AddIntegerParameter(WiFi_Test, "FREQ_MHZ", 2437);
    TM_AddIntegerParameter(WiFi_Test, "TX1", 1);
    TM_AddIntegerParameter(WiFi_Test, "TX2", 0);
    TM_AddIntegerParameter(WiFi_Test, "TX3", 0);
    TM_AddIntegerParameter(WiFi_Test, "TX4", 0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_1", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_2", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_3", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_4", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CORRECTION_FACTOR_11B", 0);
	TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",100);
	TM_AddDoubleParameter (WiFi_Test, "TX_POWER_DBM", -20);
    TM_AddStringParameter (WiFi_Test, "BANDWIDTH", "HT20");
    TM_AddStringParameter (WiFi_Test, "DATA_RATE", "OFDM-54");
	TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT_11N", "MIXED");
	TM_AddStringParameter (WiFi_Test, "PREAMBLE", "LONG");
	TM_AddStringParameter (WiFi_Test, "GUARD_INTERVAL_11N", "LONG");

	//--------------------------------------------------------------------------
	// (3) Users can now run the specific "Keyword" function.
	//--------------------------------------------------------------------------
    err = TM_Run(WiFi_Test, "TX_VERIFY_SPECTRUM");
    if( 0!=err ) 
    {	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{
	//--------------------------------------------------------------------------
	// (4) If no error occurs, users can retrieve results as follows:
	//--------------------------------------------------------------------------
		// An example to retrieve the results:		
		TM_GetIntegerReturn(WiFi_Test, "CARRIER_NUMBER_1", &valueInteger);
		printf("\t[#] CARRIER_NUMBER_1 \t\t: %d\n", valueInteger);

		TM_GetDoubleReturn(WiFi_Test, "MARGIN_DB_1", &valueDouble);
		printf("\t[#] MARGIN_DB_1 \t\t: %.2f dB\n", valueDouble);

		TM_GetDoubleReturn(WiFi_Test, "LO_LEAKAGE_DB_1", &valueDouble);
		printf("\t[#] LO_LEAKAGE_DB_1 \t\t: %.2f dB\n", valueDouble);

		printf("\t[Info] TM_Run(TX_VERIFY_SPECTRUM) return OK.\n");
	}

	return;
}


//---------------------------------------------------------------------------------
// Step 11: Rx Verify PER Test
//---------------------------------------------------------------------------------
void run_RX_VERIFY_PER(void)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	double	valueDouble;
       
	printf("\n\n__________________________________________________________________\n");
	printf("[Step 11] RX Verify PER Test.\n");
    
	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------	
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
	TM_AddIntegerParameter(WiFi_Test, "FRAME_COUNT", 0);
    TM_AddIntegerParameter(WiFi_Test, "FREQ_MHZ", 2437);
    TM_AddIntegerParameter(WiFi_Test, "RX1", 1);
    TM_AddIntegerParameter(WiFi_Test, "RX2", 0);
    TM_AddIntegerParameter(WiFi_Test, "RX3", 0);
    TM_AddIntegerParameter(WiFi_Test, "RX4", 0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_1", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_2", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_3", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_4", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "RX_POWER_DBM", -65);
    TM_AddStringParameter (WiFi_Test, "BANDWIDTH", "HT20");
    TM_AddStringParameter (WiFi_Test, "DATA_RATE", "OFDM-54"); 
	TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT_11N", "MIXED");
	TM_AddStringParameter (WiFi_Test, "PREAMBLE", "LONG");
	TM_AddStringParameter (WiFi_Test, "GUARD_INTERVAL_11N", "LONG");
	//--------------------------------------------------------------------------
	// (3) Users can now run the specific "Keyword" function.
	//--------------------------------------------------------------------------
    err = TM_Run(WiFi_Test, "RX_VERIFY_PER");
    if( 0!=err ) 
    {	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{
	//--------------------------------------------------------------------------
	// (4) If no error occurs, users can retrieve results as follows:
	//--------------------------------------------------------------------------
	// An example to retrieve results:
		TM_GetDoubleReturn(WiFi_Test, "PER", &valueDouble);
		printf("\t[#] PER \t\t\t: %.2f %c\n", valueDouble, '%');

		TM_GetDoubleReturn(WiFi_Test, "RX_POWER_LEVEL", &valueDouble);
		printf("\t[#] RX_POWER_LEVEL \t\t: %.2f dBm\n", valueDouble);

		printf("\t[Info] TM_Run(RX_VERIFY_PER) return OK.\n");
	}

	return;
}


//---------------------------------------------------------------------------------
// Step 12: Remove DUT
//---------------------------------------------------------------------------------
void run_REMOVE_DUT(void)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};

    printf("\n\n__________________________________________________________________\n");
	printf("[Step 12] Remove DUT.\n");

	//--------------------------------------------------------------------------
	// (1) Users can run the specific "Keyword" function directly.
	//--------------------------------------------------------------------------
    err = TM_Run(WiFi_Test, "REMOVE_DUT");
    if( 0!=err ) 
    {	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{
		printf("\t[Info] TM_Run(REMOVE_DUT) return OK.\n");
	}

	return;
}


//---------------------------------------------------------------------------------
// Step 13: Disconnect IQTester
//---------------------------------------------------------------------------------
void run_DISCONNECT_IQ_TESTER(void)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
    
	printf("\n\n__________________________________________________________________\n");
	printf("[Step 13] Disconnect IQTester.\n");	

	//--------------------------------------------------------------------------
	// (1) User can run the specific "Keyword" function directly.
	//--------------------------------------------------------------------------
    err = TM_Run(WiFi_Test, "DISCONNECT_IQ_TESTER");
    if( 0!=err ) 
    {	// Check the error message from the return containers
		TM_GetStringReturn(WiFi_Test, "ERROR_MESSAGE", errMsg, MAX_BUFFER_SIZE);
		throw errMsg;
	}
	else
	{
		printf("\t[Info] TM_Run(DISCONNECT_IQ_TESTER) return OK.\n");
	}

	return;
}


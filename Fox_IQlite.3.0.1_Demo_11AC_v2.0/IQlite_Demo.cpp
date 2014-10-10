// IQlite_Demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TestManager.h"
#include "global_settings.h"
//#define STRICT
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
void printProgramInfo();
void SetSolutionIndex(char *pszIndex);
void ReadRegistry();
void WriteRegistry();
char szVsport[256]="2";

LPSTR UnicodeToAnsi(LPCWSTR str);

const char  *g_IQlite_Demo_Version = "1.5.0 RC-1 (2010-04-26)\n";
TS_Global_Setting g_tsGlobalSetting;
map<string, TX_PARAM_IN> g_TxParamMap;
map<string, RX_PARAM_IN> g_RxParamMap;
map<string, bool> g_TestItemResultMap;
typedef pair<string, bool> PairResult;
STRING_TESTITEM_VECTOR strTestItemVector;

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
	else
	{
		ReadRegistry();
		printProgramInfo();
		// read test flow 
		if(!PathFileExists(_T(".\\log\\")))
		{
			CreateDirectory(_T(".\\log\\"),NULL);
		}
		TCHAR wszFolder[BUFFER_SIZE]=_T("");
		TCHAR wszFilename[BUFFER_SIZE]=_T("");
		GetCurrentDirectory(BUFFER_SIZE,wszFolder);


		TCHAR wszTesterCtrlFilename[BUFFER_SIZE]=_T("");
		swprintf_s(wszTesterCtrlFilename,BUFFER_SIZE,L"%s\\%s",wszFolder,_T("TestStartUp.ini"));
		WritePrivateProfileString(_T("TESTER_CONTROL"),_T("CTRL_MODE"),_T("-1"),wszTesterCtrlFilename);
		printf("\nWaiting for testing (set to 0) or exit (set to 999) ...\r\n");
		clock_t start, finish;
		double duration=0.0;
		while(1)
		{			
			int nTestRetVal=-1;
			int nCtrlVal=GetPrivateProfileInt(_T("TESTER_CONTROL"),_T("CTRL_MODE"),-1,wszTesterCtrlFilename);
			if(nCtrlVal==0)
			{
				start=clock();
				swprintf_s(wszFilename,BUFFER_SIZE,L"%s\\%s",wszFolder,_T("LP_Script.ini"));
				LPSTR pScriptFile = UnicodeToAnsi(wszFilename);
				strTestItemVector.clear();
				ParseTestItem(pScriptFile);
				g_TestItemResultMap.clear();
				try
				{
					WritePrivateProfileString(_T("TESTER_CONTROL"),_T("CTRL_MODE"),_T("1"),wszTesterCtrlFilename);
				}
				catch(char *msg)
				{
					printf("Error Message: %s\n",msg);
				}
				catch(...)
				{
					printf("Error occured!\n");
				}

				ofstream os;
				os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);
				os<<"\nTesting start ..."<<endl;
				os<<"__________________________________________________________________"<<endl;				
				os.close();

				int nret = SimpleWiFiTestFlow(g_tsGlobalSetting.TestCtrl.IQ_TESTER_IP1);
				if(nret !=0 && nret != 3)
				{
					TCHAR wszVal[4]=_T("");
					_itot_s(nret,wszVal,4,10);
					WritePrivateProfileString(_T("TESTER_CONTROL"),_T("CTRL_MODE"),wszVal,wszTesterCtrlFilename);
					printf("\nWaiting for testing (set to 0) or exit (set to 999) ...\r\n");
					continue;
				}
			}
			else if(nCtrlVal == 999)
			{
				exit(0);
			}
			else
			{
				Sleep(1000);
				continue;
			}

			// print out test summary
			printf("\n__________________________________________________________________\n\n");
			bool bFinalResult=true;
			map<string, bool>::iterator iterResult;

			ofstream os;
			os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);
			
			for(iterResult=g_TestItemResultMap.begin();iterResult!=g_TestItemResultMap.end();iterResult++)
			{
				if(iterResult->second == false)
				{
					bFinalResult=false;
					break;
				}
			}

			if(bFinalResult)
			{
				printf("\n                     * * * * * * * * * * * *                     ");
				printf("\n                     * * * P A S S E D * * *                    ");
				printf("\n                     * * * * * * * * * * * *                     ");
				printf("\n__________________________________________________________________");
				printf("\nFinal Result: P A S S E D                                       \n");


				os<<"                     * * * * * * * * * * * *                     "<<endl;
				os<<"                     * * * P A S S E D * * *                    "<<endl;
				os<<"                     * * * * * * * * * * * *                     "<<endl;
				os<<"__________________________________________________________________"<<endl;
				os<<"Final Result: P A S S E D                                         "<<endl;

			}
			else
			{
				printf("\n                     * * * * * * * * * * * *                     ");
				printf("\n                     * * * F A I L E D * * *                    ");
				printf("\n                     * * * * * * * * * * * *                     ");
				printf("\n__________________________________________________________________");
				printf("\nFinal Result: F A I L E D                                        \n");


				os<<"                     * * * * * * * * * * * *                     "<<endl;
				os<<"                     * * * F A I L E D * * *                     "<<endl;
				os<<"                     * * * * * * * * * * * *                     "<<endl;
				os<<"__________________________________________________________________"<<endl;
				os<<"Final Result: F A I L E D                                         "<<endl;			
			}
			printf("\nTesting Result Summary:\n");
			os<<"\nTesting Result Summary:"<<endl;
			os<<"__________________________________________________________________"<<endl;			
			for(iterResult=g_TestItemResultMap.begin();iterResult!=g_TestItemResultMap.end();iterResult++)
			{
				TCHAR tszItem[BUFFER_SIZE]=_T("");
				TCHAR tszTrim[]=_T(" \r\n\0\t");
				char szVal[BUFFER_SIZE]="";


				Char2WChar(iterResult->first.c_str(),strlen(iterResult->first.c_str()),tszItem,BUFFER_SIZE);
				if(iterResult->second)
				{
					StrTrim(tszItem,tszTrim);
					WChar2Char(tszItem,BUFFER_SIZE,szVal,BUFFER_SIZE);
					cout<<szVal<<"    \t<-- Passed"<<endl;
					os<<szVal<<"    \t<-- Passed"<<endl;
				}
				else
				{				
					StrTrim(tszItem,tszTrim);
					WChar2Char(tszItem,BUFFER_SIZE,szVal,BUFFER_SIZE);
					cout<<szVal<<"    \t<-- Failed"<<endl;
					os<<szVal<<"    \t<-- Failed"<<endl;
				}				
			}
			
			finish=clock();
			duration = (double)(finish - start) / CLOCKS_PER_SEC;
			printf("Total Test time: %.2f sec\n", duration );
			os<<"Total Test time: "<<setprecision(5)<<duration<<endl;
			os.close();

			// write final test result into TestStartUp.ini
			try
			{
				if(bFinalResult)
				{
					WritePrivateProfileString(_T("TESTER_CONTROL"),_T("CTRL_MODE"),_T("2"),wszTesterCtrlFilename);
				}
				else
				{
					WritePrivateProfileString(_T("TESTER_CONTROL"),_T("CTRL_MODE"),_T("3"),wszTesterCtrlFilename);
				}

			}
			catch(char *msg)
			{
				printf("Error Message: %s\n",msg);
			}
			catch(...)
			{
				printf("Error occured!\n");
			}
			printf("\nWaiting for testing (set to 0) or exit (set to 999) ...\r\n");
			// Exit testing after test flow run finish
			if(g_tsGlobalSetting.TestCtrl.ExitWhenDone)
			{
				WriteRegistry();
				exit(0);
			}

		}	
	}

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
	int     nRunResult=0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};

	// Step 1: Register the WiFi Test functions with TestManager
    //         This ensures that all WiFi Test functions are made available to the  Test Manager

   clock_t start, finish;
   double  duration;

    try
    {        
		start=clock();
		run_RegisterTechnologyDll();
		finish=clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		printf("Test time: %2.2f sec\n", duration );

	}
	catch(char *msg)
    {
        printf(msg);
		err = err_RegisterTechnologyDll;
		goto __exit_test__; 
    }
    catch(...)
    {
		printf("[Error] Unknown Error!\n");
		err = err_RegisterTechnologyDll;
		goto __exit_test__; 
    }
	
    // Step 2: Setup the global settings
    //         Global settings are used by all test functions
    //         They do not belong to any specific test function
    try
    {        
		start=clock();
		run_GLOBAL_SETTINGS();
		finish=clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		printf("Test time: %2.2f sec\n", duration );
	}
	catch(char *msg)
    {
        printf(msg);
		err = err_GLOBAL_SETTINGS;
		goto __exit_test__; 
    }
    catch(...)
    {
		printf("[Error] Unknown Error!\n");
		err = err_GLOBAL_SETTINGS;
		goto __exit_test__; 
    }

    // Step 3: Load path loss table from file
    try
    {        
		start=clock();
		run_LOAD_PATH_LOSS_TABLE();
		finish=clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		printf("Test time: %2.2f sec\n", duration );
	}
	catch(char *msg)
    {
        printf(msg);
		err = err_LOAD_PATH_LOSS_TABLE;
		goto __exit_test__; 
    }
    catch(...)
    {
		printf("[Error] Unknown Error!\n");
		err = err_LOAD_PATH_LOSS_TABLE;
		goto __exit_test__;     
	}
		
    // Step 4: Insert DUT for test
    try
    {        
		start=clock();
		run_INSERT_DUT();
		finish=clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		printf("Test time: %2.2f sec\n", duration );
	}
	catch(char *msg)
    {
        printf(msg);
		err = err_INSERT_DUT;
		goto __exit_test__; 
    }
    catch(...)
    {
		printf("[Error] Unknown Error!\n");
		err = err_INSERT_DUT;
		goto __exit_test__; 
    }

	// Step 5: Initial DUT for test	
    try
    {        
		start=clock();
		run_INITIALIZE_DUT();
		finish=clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		printf("Test time: %2.2f sec\n", duration );
	}
	catch(char *msg)
    {
        printf(msg);
		err = err_INITIALIZE_DUT;
		goto __exit_test__; 
    }
    catch(...)
    {
		printf("[Error] Unknown Error!\n");
		err = err_INITIALIZE_DUT;
		goto __exit_test__; 
	}
	// Here add your code



    // Step 6: Connect IQTester (IP address needed)
    try
    {        
		start=clock();
		run_CONNECT_IQ_TESTER(ipAddress);
		finish=clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		printf("Test time: %2.2f sec\n", duration );
	}
	catch(char *msg)
    {
        printf(msg);
		err = err_CONNECT_IQ_TESTER;
		goto __exit_test__; 
    }
    catch(...)
    {
		printf("[Error] Unknown Error!\n");
		err = err_CONNECT_IQ_TESTER;
		goto __exit_test__; 
    }
#if 0
	// Step 5: Initial DUT for test	
    try
    {        
		start=clock();
		run_INITIALIZE_DUT();
		finish=clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		printf("Test time: %2.2f sec\n", duration );
	}
	catch(char *msg)
    {
        printf(msg);
		err = err_INITIALIZE_DUT;
		goto __exit_test__; 
    }
    catch(...)
    {
		printf("[Error] Unknown Error!\n");
		err = err_INITIALIZE_DUT;
		goto __exit_test__; 
	}
	// Here add your code
#endif
	// start to execute test flow	

	int iRepeatTime = g_tsGlobalSetting.TestCtrl.RepeatTimes;
	for(iRepeatTime ; iRepeatTime > 0; iRepeatTime-- )
	{
		
		int iTestNo=6;
		map<string, TX_PARAM_IN>::iterator iter;
		map<string, RX_PARAM_IN>::iterator rx_iter;
		STRING_TESTITEM_VECTOR::iterator item_iter;
		for(item_iter=strTestItemVector.begin();item_iter!=strTestItemVector.end();item_iter++)
		{
			start=clock();
			iTestNo++;
			myprintf(item_iter->c_str());
			if( strstr(item_iter->c_str(),"TX_VERIFY_EVM_") ||
				strstr(item_iter->c_str(),"TX_VERIFY_POWER_") ||
				strstr(item_iter->c_str(),"TX_VERIFY_MASK_") ||
				strstr(item_iter->c_str(),"RX_VERIFY_PER_"))
			{
				char szTemp[BUFFER_SIZE]="";
				strcpy_s(szTemp,BUFFER_SIZE,item_iter->c_str());

				char *pToken=NULL, *pNextToken=NULL;
				pToken = strtok_s(szTemp," ",&pNextToken);

				char *pszIndex=strrchr(pToken,'_');
				SetSolutionIndex(pszIndex+1);
			}
			else
			{
				SetSolutionIndex("1");	//add by daixin 2012-11-20
			}

			if(strstr(item_iter->c_str(),"TX_VERIFY_EVM"))
			{
				iter = g_TxParamMap.find(item_iter->c_str());
				TX_PARAM_RETURN m_txParamReturn;
				printf("\n\n__________________________________________________________________\n");
				printf("[Step %03d] %s\n",iTestNo,iter->first.c_str());	
				char szTestItemMap[BUFFER_SIZE]="";
				sprintf_s(szTestItemMap,sizeof(szTestItemMap),"[Step %03d] %s",iTestNo,iter->first.c_str());
				int iRetry=g_tsGlobalSetting.TestCtrl.nRetryTimes;
				while(iRetry--)
				{
					printf("Retry Times=%d\n",g_tsGlobalSetting.TestCtrl.nRetryTimes-iRetry);

					bool bPass=false;
					try
					{        
						run_TX_VERIFY_EVM(&iter->second,&m_txParamReturn);
						bPass = ShowTestResultEVM(&iter->second,&m_txParamReturn,iRetry);
					}
					catch(char *msg)
					{
						printf(msg);
						bPass = false;
					}
					catch(...)
					{
						printf("[Error] Unknown Error!\n");
						bPass = false;
					}				

					if( (iRetry == 0) && g_tsGlobalSetting.TestCtrl.RunMode==0 && bPass == false) // Stop On Fail
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,false));
						finish=clock();
						duration = (double)(finish - start) / CLOCKS_PER_SEC;
						printf("Test time: %2.2f sec\n", duration );
						ofstream os;
						os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);
						os<<"Test time: "<<setprecision(4)<<duration<<endl;
						os.close();
						err = 3; // for wifi control
						goto __exit_test__;
					}
					else if((iRetry == 0) && g_tsGlobalSetting.TestCtrl.RunMode==1&& bPass == false)
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,false));
					}
					else if(bPass)
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,true));
						break;
					}
					else
					{
						//
					}
				}

			}
			else if(strstr(item_iter->c_str(),"TX_VERIFY_POWER"))
			{
				iter = g_TxParamMap.find(item_iter->c_str());
				TX_PARAM_RETURN m_txParamReturn;
				printf("\n\n__________________________________________________________________\n");
				printf("[Step %03d] %s\n",iTestNo,iter->first.c_str());
				char szTestItemMap[BUFFER_SIZE]="";
				sprintf_s(szTestItemMap,sizeof(szTestItemMap),"[Step %03d] %s",iTestNo,iter->first.c_str());
				int iRetry=g_tsGlobalSetting.TestCtrl.nRetryTimes;
				while(iRetry--)
				{
					printf("Retry Times=%d\n",g_tsGlobalSetting.TestCtrl.nRetryTimes-iRetry);

					bool bPass=false;
					try
					{        
						run_TX_VERIFY_POWER(&iter->second,&m_txParamReturn);
						bPass = ShowTestResultPower(&iter->second,&m_txParamReturn,iRetry);
					}
					catch(char *msg)
					{
						printf(msg);
						bPass = false;
					}
					catch(...)
					{
						printf("[Error] Unknown Error!\n");
						bPass = false;
					}				

					if( (iRetry == 0) && g_tsGlobalSetting.TestCtrl.RunMode==0 && bPass == false) // Stop On Fail
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,false));
						finish=clock();
						duration = (double)(finish - start) / CLOCKS_PER_SEC;
						printf("Test time: %2.2f sec\n", duration );
						ofstream os;
						os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);
						os<<"Test time: "<<setprecision(4)<<duration<<endl;
						os.close();
						err = 3; // for wifi control
						goto __exit_test__;
					}
					else if( (iRetry == 0) && g_tsGlobalSetting.TestCtrl.RunMode==1&& bPass == false)
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,false));
					}
					else if(bPass)
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,true));
						break;
					}
					else
					{
						//
					}
				}
			}
			else if(strstr(item_iter->c_str(),"TX_VERIFY_MASK"))
			{
				iter = g_TxParamMap.find(item_iter->c_str());
				TX_PARAM_RETURN m_txParamReturn;
				printf("\n\n__________________________________________________________________\n");
				printf("[Step %03d] %s\n",iTestNo,iter->first.c_str());
				char szTestItemMap[BUFFER_SIZE]="";
				sprintf_s(szTestItemMap,sizeof(szTestItemMap),"[Step %03d] %s",iTestNo,iter->first.c_str());
				int iRetry=g_tsGlobalSetting.TestCtrl.nRetryTimes;
				while(iRetry--)
				{
					printf("Retry Times=%d\n",g_tsGlobalSetting.TestCtrl.nRetryTimes-iRetry);

					bool bPass=false;
					try
					{        
						run_TX_VERIFY_MASK(&iter->second,&m_txParamReturn);
						bPass = ShowTestResultMask(&iter->second,&m_txParamReturn,iRetry);
					}
					catch(char *msg)
					{
						printf(msg);
						bPass = false;
					}
					catch(...)
					{
						printf("[Error] Unknown Error!\n");
						bPass = false;
					}				

					if( (iRetry == 0) && g_tsGlobalSetting.TestCtrl.RunMode==0 && bPass == false) // Stop On Fail
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,false));
						finish=clock();
						duration = (double)(finish - start) / CLOCKS_PER_SEC;
						printf("Test time: %2.2f sec\n", duration );
						ofstream os;
						os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);
						os<<"Test time: "<<setprecision(4)<<duration<<endl;
						os.close();
						//return err;
						err = 3; // for wifi control
						goto __exit_test__;
					}
					else if( (iRetry == 0) && g_tsGlobalSetting.TestCtrl.RunMode==1&& bPass == false)
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,false));
					}
					else if(bPass)
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,true));
						break;
					}
					else
					{
						//
					}
				}
			}
			else if(strstr(item_iter->c_str(),"RX_VERIFY_PER"))
			{
				rx_iter = g_RxParamMap.find(item_iter->c_str());
				RX_PARAM_RETURN m_rxParamReturn;
				printf("\n\n__________________________________________________________________\n");
				printf("[Step %03d] %s\n",iTestNo,rx_iter->first.c_str());
				char szTestItemMap[BUFFER_SIZE]="";
				sprintf_s(szTestItemMap,sizeof(szTestItemMap),"[Step %03d] %s",iTestNo,rx_iter->first.c_str());
				int iRetry=g_tsGlobalSetting.TestCtrl.nRetryTimes;
				while(iRetry--)
				{
					printf("Retry Times=%d\n",g_tsGlobalSetting.TestCtrl.nRetryTimes-iRetry);

					bool bPass=false;
					try
					{        
						run_RX_VERIFY_PER(&rx_iter->second,&m_rxParamReturn); 
						bPass = ShowTestResultPER(&rx_iter->second,&m_rxParamReturn,iRetry);
					}
					catch(char *msg)
					{
						printf(msg);
						bPass = false;
					}
					catch(...)
					{
						printf("[Error] Unknown Error!\n");
						bPass = false;
					}				

					if( (iRetry == 0) && g_tsGlobalSetting.TestCtrl.RunMode==0 && bPass == false) // Stop On Fail
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,false));
						finish=clock();
						duration = (double)(finish - start) / CLOCKS_PER_SEC;
						printf("Test time: %2.2f sec\n", duration );
						ofstream os;
						os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);
						os<<"Test time: "<<setprecision(4)<<duration<<endl;
						os.close();
						//return err;
						err = 3; // for wifi control
						goto __exit_test__;
					}
					else if( (iRetry == 0) && g_tsGlobalSetting.TestCtrl.RunMode==1&& bPass == false)
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,false));
					}
					else if(bPass)
					{
						g_TestItemResultMap.insert(PairResult(szTestItemMap,true));
						break;
					}
					else
					{
						//
					}
				}
			}
			else
			{
				//
				printf("not supported yet!\n");
			}
			finish=clock();
			duration = (double)(finish - start) / CLOCKS_PER_SEC;
			printf("Test time: %2.2f sec\n", duration );
			ofstream os;
			os.open(_T(".\\log\\Log_all.txt"),ios_base::app|ios_base::out);
			os<<"Test time: "<<setprecision(4)<<duration<<endl;
			os.close();
		}
	}

__exit_test__:

		// here add to control the Disable_WiFi flag in LP_Dut_Setup.ini
		// run_REMOVE_DUT() function will read this parameter before excution.
		if(err == 0)
		{
			TCHAR wszFolder[BUFFER_SIZE]=_T("");
			TCHAR wszFilename[BUFFER_SIZE]=_T("");
			GetCurrentDirectory(BUFFER_SIZE,wszFolder);
			TCHAR wszTesterCtrlFilename[BUFFER_SIZE]=_T("");
			swprintf_s(wszTesterCtrlFilename,BUFFER_SIZE,L"%s\\%s",wszFolder,_T("LP_DUT_setup.ini"));
			WritePrivateProfileString(_T("Configuration"),_T("Disable_WiFi"),_T("1"),wszTesterCtrlFilename);
		}
		else
		{
			TCHAR wszFolder[BUFFER_SIZE]=_T("");
			TCHAR wszFilename[BUFFER_SIZE]=_T("");
			GetCurrentDirectory(BUFFER_SIZE,wszFolder);
			TCHAR wszTesterCtrlFilename[BUFFER_SIZE]=_T("");
			swprintf_s(wszTesterCtrlFilename,BUFFER_SIZE,L"%s\\%s",wszFolder,_T("LP_DUT_setup.ini"));
			WritePrivateProfileString(_T("Configuration"),_T("Disable_WiFi"),_T("0"),wszTesterCtrlFilename);		
		}
		// add end
		// Step 12: Remove DUT
		run_REMOVE_DUT();

        // Step 13: Disconnect IQTester
		run_DISCONNECT_IQ_TESTER();        

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
	printf("[Step 001] Register the WiFi Test functions with TestManager.\n");
	//--------------------------------------------------------------------------
	// (1) Register the WiFi Test functions with TestManager.
	//--------------------------------------------------------------------------
    ::TM_RegisterTechnologyDll("WIFI_11AC", "WiFi_11ac_Test.dll", &WiFi_Test);
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
	printf("[Step 002] Setup the global settings.\n");

	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------
    TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
	run_Global_Settings_AddParam(WiFi_Test);
    /*TM_AddIntegerParameter(WiFi_Test, "DUT_KEEP_TRANSMIT", 0); 
	TM_AddIntegerParameter(WiFi_Test, "VSA_PORT", 2); 
	TM_AddIntegerParameter(WiFi_Test, "VSG_PORT", 3); 
	TM_AddIntegerParameter(WiFi_Test, "IQ2010_EXT_ENABLE", 0);*/	

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
	printf("[Step 003] LOAD_PATH_LOSS_TABLE Test.\n");

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
	printf("[Step 006] Connect IQTester (IP address needed).\n");

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
	TM_AddIntegerParameter(WiFi_Test, "IQTESTER_RECONNECT", g_tsGlobalSetting.TestCtrl.IQ_TESTER_RECONNECT);
	TM_AddIntegerParameter(WiFi_Test, "IQTESTER_TYPE", 1);
	TM_AddIntegerParameter(WiFi_Test, "IQTESTER_CONTROL_METHOD", 0);
	TM_AddIntegerParameter(WiFi_Test, "DH_ENABLE", g_tsGlobalSetting.TestCtrl.DH_ENABLE);
    TM_AddIntegerParameter(WiFi_Test, "DH_OBTAIN_CONTROL_TIMEOUT_MS", g_tsGlobalSetting.TestCtrl.DH_OBTAIN_CONTROL_TIMEOUT_MS);
    TM_AddIntegerParameter(WiFi_Test, "DH_PROBE_TIME_MS", g_tsGlobalSetting.TestCtrl.DH_PROBE_TIME_MS);
    TM_AddIntegerParameter(WiFi_Test, "DH_TOKEN_ID", g_tsGlobalSetting.TestCtrl.DH_TOKEN_ID);

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
	printf("[Step 004] Insert DUT for test.\n");
	
	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------	
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
	LPSTR szDutDllFilename = UnicodeToAnsi(g_tsGlobalSetting.TestCtrl.szDUT_DLL_FILENAME);
	LPSTR szConStr = UnicodeToAnsi(g_tsGlobalSetting.TestCtrl.szConnectionString);
	TM_AddIntegerParameter(WiFi_Test, "RELOAD_DUT_DLL", g_tsGlobalSetting.TestCtrl.ReloadDutDLL);
    TM_AddStringParameter (WiFi_Test, "DUT_DLL_FILENAME", szDutDllFilename);
	TM_AddStringParameter (WiFi_Test, "CONNECTION_STRING", szConStr);
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
	printf("[Step 005] Initial DUT for test.\n");

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
void run_TX_VERIFY_EVM(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	double	valueDouble = 0.0;
        
	/*printf("\n\n__________________________________________________________________\n");
	printf("[Step 7] TX Verify EVM Test.\n");*/
    
	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------	
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
	TM_AddIntegerParameter(WiFi_Test, "BSS_FREQ_MHZ_PRIMARY", txParam->BSS_FREQ_MHZ_PRIMARY);
	TM_AddIntegerParameter(WiFi_Test, "BSS_FREQ_MHZ_SECONDARY", txParam->BSS_FREQ_MHZ_SECONDARY);
	TM_AddIntegerParameter(WiFi_Test, "CH_FREQ_MHZ", txParam->CH_FREQ_MHZ);
	TM_AddIntegerParameter(WiFi_Test, "CH_FREQ_MHZ_PRIMARY_20MHz", txParam->CH_FREQ_MHZ_PRIMARY_20MHz);
	TM_AddIntegerParameter(WiFi_Test, "NUM_STREAM_11AC", txParam->NUM_STREAM_11AC);
	TM_AddIntegerParameter(WiFi_Test, "TX1", txParam->nTx1);
	TM_AddIntegerParameter(WiFi_Test, "TX2", txParam->nTx2);
	TM_AddIntegerParameter(WiFi_Test, "TX3", txParam->nTx3);
	TM_AddIntegerParameter(WiFi_Test, "TX4", txParam->nTx4);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_1", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_2", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_3", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_4", 0.0);
	//TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",100);
	TM_AddDoubleParameter (WiFi_Test, "TX_POWER_DBM", txParam->dTargetPower);
	TM_AddStringParameter (WiFi_Test, "CH_BANDWIDTH", /*"CBW-80"*/txParam->CH_BANDWIDTH);
	TM_AddStringParameter (WiFi_Test, "BSS_BANDWIDTH", /*"BW-80"*/txParam->BSS_BANDWIDTH);
	TM_AddStringParameter (WiFi_Test, "DATA_RATE", txParam->DATA_RATE);
	TM_AddStringParameter (WiFi_Test, "GUARD_INTERVAL", "LONG"); // by default value.
	//TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11ac"); // by default value.
	// if 11ac, VHT, else, MIXED or GREENFIELD.
	//LPSTR szPacketFormat
	if(strstr(txParam->BSS_BANDWIDTH,"80"))
	{
		TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", "VHT");	// 11ac, CBW-80
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_VHT_US);	
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11ac");
	}
	else 
	{
		if(strstr(txParam->DATA_RATE,"MCS"))
		{
			LPSTR szPacketFormat = UnicodeToAnsi(g_tsGlobalSetting.TestCtrl.PACKET_FORMAT);
			TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", szPacketFormat); // read from global_settings.ini, 
		}
		else	// OFDM, DSSS
		{
			TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", "NON_HT"); // read from global_settings.ini, 
		}
	
	}
	
	if(strstr(txParam->DATA_RATE,"DSSS") || strstr(txParam->DATA_RATE,"CCK") || strstr(txParam->DATA_RATE,"PBCC"))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "LONG");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11B_L_US);
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11b");
	}
	else if(strstr(txParam->DATA_RATE,"OFDM"))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_NON_HT_US);
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11ag");
	}
	else if(strstr(txParam->DATA_RATE,"MCS") && wcsstr(g_tsGlobalSetting.TestCtrl.PACKET_FORMAT,_T("HT_MF")))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_HT_MF_US);
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11n");
	}
	else if(strstr(txParam->DATA_RATE,"MCS") && wcsstr(g_tsGlobalSetting.TestCtrl.PACKET_FORMAT,_T("HT_GF")))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_HT_GF_US);	
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11n");
	}
	else
	{
		//
	}
	
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
		TM_GetDoubleReturn(WiFi_Test, "EVM_AVG_DB", &valueDouble);
		//printf("\t[#] EVM_AVG_DB \t\t\t: %.2f dB\n", valueDouble);
		txReturn->EVM_AVG_ALL=valueDouble;

		TM_GetDoubleReturn(WiFi_Test, "FREQ_ERROR_AVG", &valueDouble);
		//printf("\t[#] FREQ_ERROR_AVG \t\t: %.2f ppm\n", valueDouble);
		txReturn->FREQ_ERROR=valueDouble;

		int nAntSum = txParam->nTx1+txParam->nTx2+txParam->nTx3+txParam->nTx4;
		switch (nAntSum)
		{
		case 1:
			TM_GetDoubleReturn(WiFi_Test, "POWER_AVG_1", &valueDouble);
			txReturn->POWER_AVG_1=valueDouble;
			break;
		case 2:
			TM_GetDoubleReturn(WiFi_Test, "POWER_AVG_1", &valueDouble);
			txReturn->POWER_AVG_1=valueDouble;
			TM_GetDoubleReturn(WiFi_Test, "POWER_AVG_2", &valueDouble);
			txReturn->POWER_AVG_2=valueDouble;
			break;
		case 3:
			TM_GetDoubleReturn(WiFi_Test, "POWER_AVG_1", &valueDouble);
			txReturn->POWER_AVG_1=valueDouble;
			TM_GetDoubleReturn(WiFi_Test, "POWER_AVG_2", &valueDouble);
			txReturn->POWER_AVG_2=valueDouble;
			TM_GetDoubleReturn(WiFi_Test, "POWER_AVG_3", &valueDouble);
			txReturn->POWER_AVG_3=valueDouble;
			break;
		case 4:
			TM_GetDoubleReturn(WiFi_Test, "POWER_AVG_1", &valueDouble);
			txReturn->POWER_AVG_1=valueDouble;
			TM_GetDoubleReturn(WiFi_Test, "POWER_AVG_2", &valueDouble);
			txReturn->POWER_AVG_2=valueDouble;
			TM_GetDoubleReturn(WiFi_Test, "POWER_AVG_3", &valueDouble);
			txReturn->POWER_AVG_3=valueDouble;
			TM_GetDoubleReturn(WiFi_Test, "POWER_AVG_4", &valueDouble);
			txReturn->POWER_AVG_4=valueDouble;
			break;
		default:
			break;
		}
#if 0		
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
#endif
	}

	return;
}


//---------------------------------------------------------------------------------
// Step 8: TX Verify Power Test
//---------------------------------------------------------------------------------
void run_TX_VERIFY_POWER(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn)
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
	TM_AddIntegerParameter(WiFi_Test, "BSS_FREQ_MHZ_PRIMARY", txParam->BSS_FREQ_MHZ_PRIMARY);
	TM_AddIntegerParameter(WiFi_Test, "BSS_FREQ_MHZ_SECONDARY", txParam->BSS_FREQ_MHZ_SECONDARY);
	TM_AddIntegerParameter(WiFi_Test, "CH_FREQ_MHZ", txParam->CH_FREQ_MHZ);
	TM_AddIntegerParameter(WiFi_Test, "CH_FREQ_MHZ_PRIMARY_20MHz", txParam->CH_FREQ_MHZ_PRIMARY_20MHz);
	TM_AddIntegerParameter(WiFi_Test, "NUM_STREAM_11AC", txParam->NUM_STREAM_11AC);
	TM_AddIntegerParameter(WiFi_Test, "TX1", txParam->nTx1);
	TM_AddIntegerParameter(WiFi_Test, "TX2", txParam->nTx2);
	TM_AddIntegerParameter(WiFi_Test, "TX3", txParam->nTx3);
	TM_AddIntegerParameter(WiFi_Test, "TX4", txParam->nTx4);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_1", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_2", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_3", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_4", 0.0);
	//TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",100);
	TM_AddDoubleParameter (WiFi_Test, "TX_POWER_DBM", txParam->dTargetPower);
	TM_AddStringParameter (WiFi_Test, "CH_BANDWIDTH", txParam->CH_BANDWIDTH);
	TM_AddStringParameter (WiFi_Test, "BSS_BANDWIDTH", txParam->BSS_BANDWIDTH);
	TM_AddStringParameter (WiFi_Test, "DATA_RATE", txParam->DATA_RATE);
	TM_AddStringParameter (WiFi_Test, "GUARD_INTERVAL", "LONG"); // by default value.
	//TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11ac"); // by default value.
	// if 11ac, VHT, else, MIXED or GREENFIELD.
	if(strstr(txParam->BSS_BANDWIDTH,"-80"))
	{
		TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", "VHT");	// 11ac, CBW-80
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11ac");
	}
	else if(strstr(txParam->DATA_RATE,"MCS"))
	{
		LPSTR szPacketFormat = UnicodeToAnsi(g_tsGlobalSetting.TestCtrl.PACKET_FORMAT);
		TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", szPacketFormat); // read from global_settings.ini, 
	}
	else	// OFDM, DSSS
	{
		TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", "NON_HT"); // read from global_settings.ini, 
	}

	if(strstr(txParam->DATA_RATE,"DSSS") || strstr(txParam->DATA_RATE,"CCK") || strstr(txParam->DATA_RATE,"PBCC"))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "LONG");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11B_L_US);
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11b");
	}
	else if(strstr(txParam->DATA_RATE,"OFDM"))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_NON_HT_US);
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11ag");
	}
	else if(strstr(txParam->DATA_RATE,"MCS") && wcsstr(g_tsGlobalSetting.TestCtrl.PACKET_FORMAT,_T("HT_MF")))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_HT_MF_US);
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11n");
	}
	else if(strstr(txParam->DATA_RATE,"MCS") && wcsstr(g_tsGlobalSetting.TestCtrl.PACKET_FORMAT,_T("HT_GF")))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_HT_GF_US);	
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11n");
	}
	else
	{
		//
	}

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
		txReturn->POWER_AVG_ALL=valueDouble;
#if 0
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
#endif
	}

	return;
}


//---------------------------------------------------------------------------------
// Step 9: TxVerify Mask Test
//---------------------------------------------------------------------------------
void run_TX_VERIFY_MASK(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	double	valueDouble = 0.0;
	int		arraySize = 0;
	double* arrayDouble = NULL;
	char    strQuery[MAX_BUFFER_SIZE] = {'\0'};
	char    strResult[MAX_BUFFER_SIZE] = {'\0'};
	     
	/*printf("\n\n__________________________________________________________________\n");
	printf("[Step 9] TX Verify Mask Test.\n");*/
    
	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------	
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
	TM_AddIntegerParameter(WiFi_Test, "BSS_FREQ_MHZ_PRIMARY", txParam->BSS_FREQ_MHZ_PRIMARY);
	TM_AddIntegerParameter(WiFi_Test, "BSS_FREQ_MHZ_SECONDARY", txParam->BSS_FREQ_MHZ_SECONDARY);
	TM_AddIntegerParameter(WiFi_Test, "CH_FREQ_MHZ", txParam->CH_FREQ_MHZ);
	TM_AddIntegerParameter(WiFi_Test, "CH_FREQ_MHZ_PRIMARY_20MHz", txParam->CH_FREQ_MHZ_PRIMARY_20MHz);
	TM_AddIntegerParameter(WiFi_Test, "NUM_STREAM_11AC", txParam->NUM_STREAM_11AC);
	TM_AddIntegerParameter(WiFi_Test, "TX1", txParam->nTx1);
	TM_AddIntegerParameter(WiFi_Test, "TX2", txParam->nTx2);
	TM_AddIntegerParameter(WiFi_Test, "TX3", txParam->nTx3);
	TM_AddIntegerParameter(WiFi_Test, "TX4", txParam->nTx4);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_1", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_2", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_3", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_4", 0.0);
	//TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",100);
	TM_AddDoubleParameter (WiFi_Test, "TX_POWER_DBM", txParam->dTargetPower);
	TM_AddStringParameter (WiFi_Test, "CH_BANDWIDTH", /*"CBW-80"*/txParam->CH_BANDWIDTH);
	TM_AddStringParameter (WiFi_Test, "BSS_BANDWIDTH", /*"BW-80"*/txParam->BSS_BANDWIDTH);
	TM_AddStringParameter (WiFi_Test, "DATA_RATE", txParam->DATA_RATE);
	TM_AddStringParameter (WiFi_Test, "GUARD_INTERVAL", "LONG"); // by default value.
	//TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11ac"); // by default value.
	TM_AddDoubleParameter (WiFi_Test, "OBW_PERCENTAGE", 99);
	TM_AddStringParameter (WiFi_Test, "MASK_TEMPLATE", "");
	// if 11ac, VHT, else, MIXED or GREENFIELD.
	//LPSTR szPacketFormat
	if(strstr(txParam->BSS_BANDWIDTH,"80"))
	{
		TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", "VHT");	// 11ac, CBW-80
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_VHT_US);	
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11ac");
	}
	else 
	{
		if(strstr(txParam->DATA_RATE,"MCS"))
		{
			LPSTR szPacketFormat = UnicodeToAnsi(g_tsGlobalSetting.TestCtrl.PACKET_FORMAT);
			TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", szPacketFormat); // read from global_settings.ini, 
		}
		else	// OFDM, DSSS
		{
			TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", "NON_HT"); // read from global_settings.ini, 
		}
	
	}
	
	if(strstr(txParam->DATA_RATE,"DSSS") || strstr(txParam->DATA_RATE,"CCK") || strstr(txParam->DATA_RATE,"PBCC"))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "LONG");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11B_L_US);
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11b");
	}
	else if(strstr(txParam->DATA_RATE,"OFDM"))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_NON_HT_US);
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11ag");
	}
	else if(strstr(txParam->DATA_RATE,"MCS") && wcsstr(g_tsGlobalSetting.TestCtrl.PACKET_FORMAT,_T("HT_MF")))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_HT_MF_US);
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11n");
	}
	else if(strstr(txParam->DATA_RATE,"MCS") && wcsstr(g_tsGlobalSetting.TestCtrl.PACKET_FORMAT,_T("HT_GF")))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");
		TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME_US",g_tsGlobalSetting.EVM_CAPTURE_TIME_11AC_HT_GF_US);	
		TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11n");
	}
	else
	{
		//
	}
	
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
		TM_GetDoubleReturn(WiFi_Test, "VIOLATION_PERCENT", &valueDouble);
		txReturn->MASK_PERCENT=valueDouble;

		TM_GetDoubleReturn(WiFi_Test, "POWER_AVERAGE_DBM", &valueDouble);
		txReturn->POWER_AVG_ALL=valueDouble;
#if 0
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
#endif
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
void run_RX_VERIFY_PER(RX_PARAM_IN *rxParam, RX_PARAM_RETURN *rxReturn)
{
	int		err = 0;
	char	errMsg[MAX_BUFFER_SIZE] = {'\0'};
	double	valueDouble;
       
	/*printf("\n\n__________________________________________________________________\n");
	printf("[Step 11] RX Verify PER Test.\n");*/
    
	//--------------------------------------------------------------------------
	// (1) Clear all input parameter containers for the specified technology.
	//--------------------------------------------------------------------------	
	TM_ClearParameters(WiFi_Test);

	//--------------------------------------------------------------------------
	// (2) Add input parameters into containers.
	//--------------------------------------------------------------------------
	TM_AddIntegerParameter(WiFi_Test, "BSS_FREQ_MHZ_PRIMARY", rxParam->BSS_FREQ_MHZ_PRIMARY);
	TM_AddIntegerParameter(WiFi_Test, "BSS_FREQ_MHZ_SECONDARY", rxParam->BSS_FREQ_MHZ_SECONDARY);
	TM_AddIntegerParameter(WiFi_Test, "CH_FREQ_MHZ", rxParam->CH_FREQ_MHZ);
	TM_AddIntegerParameter(WiFi_Test, "CH_FREQ_MHZ_PRIMARY_20MHz", rxParam->CH_FREQ_MHZ_PRIMARY_20MHz);
	TM_AddIntegerParameter(WiFi_Test, "FRAME_COUNT",rxParam->FRAME_COUNT);
	TM_AddIntegerParameter(WiFi_Test, "NUM_STREAM_11AC",rxParam->NUM_STREAM_11AC);
	TM_AddIntegerParameter(WiFi_Test, "RX1", rxParam->nRx1);
	TM_AddIntegerParameter(WiFi_Test, "RX2", rxParam->nRx2);
	TM_AddIntegerParameter(WiFi_Test, "RX3", rxParam->nRx3);
	TM_AddIntegerParameter(WiFi_Test, "RX4", rxParam->nRx4);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_1", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_2", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_3", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS_DB_4", 0.0);
	TM_AddDoubleParameter (WiFi_Test, "RX_POWER_DBM", rxParam->dRxPowerLevel);
	TM_AddStringParameter (WiFi_Test, "BSS_BANDWIDTH", rxParam->BSS_BANDWIDTH);
	TM_AddStringParameter (WiFi_Test, "CH_BANDWIDTH", rxParam->CH_BANDWIDTH);
	TM_AddStringParameter (WiFi_Test, "DATA_RATE", rxParam->DATA_RATE); 
	TM_AddStringParameter (WiFi_Test, "GUARD_INTERVAL", "LONG");
	TM_AddStringParameter (WiFi_Test, "STANDARD", "802.11ac");
	TM_AddStringParameter (WiFi_Test, "WAVEFORM_NAME", "");

	// if 11ac, VHT, else, MIXED or GREENFIELD.
	if(strstr(rxParam->BSS_BANDWIDTH,"-80"))
	{
		TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", "VHT");	// 11ac, CBW-80
	}
	else if(strstr(rxParam->DATA_RATE,"MCS"))
	{
		LPSTR szPacketFormat = UnicodeToAnsi(g_tsGlobalSetting.TestCtrl.PACKET_FORMAT);
		TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", szPacketFormat); // read from global_settings.ini, 
	}
	else	// OFDM, DSSS
	{
		TM_AddStringParameter (WiFi_Test, "PACKET_FORMAT", "NON_HT"); // read from global_settings.ini, 
	}

	if(strstr(rxParam->DATA_RATE,"DSSS") || strstr(rxParam->DATA_RATE,"CCK") || strstr(rxParam->DATA_RATE,"PBCC"))
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "LONG");
	}
	else 
	{
		TM_AddStringParameter (WiFi_Test, "PREAMBLE", "SHORT");		
	}

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
		TM_GetDoubleReturn(WiFi_Test, "PER", &valueDouble);
		rxReturn->dPerPercent = valueDouble; 

		TM_GetDoubleReturn(WiFi_Test, "RX_POWER_LEVEL", &valueDouble);
		rxReturn->dRxPowerLevel = valueDouble; 
#if 0
		// An example to retrieve results:
		TM_GetDoubleReturn(WiFi_Test, "PER", &valueDouble);
		printf("\t[#] PER \t\t\t: %.2f %c\n", valueDouble, '%');

		TM_GetDoubleReturn(WiFi_Test, "RX_POWER_LEVEL", &valueDouble);
		printf("\t[#] RX_POWER_LEVEL \t\t: %.2f dBm\n", valueDouble);

		printf("\t[Info] TM_Run(RX_VERIFY_PER) return OK.\n");
#endif
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
	printf("[Step 998] Remove DUT.\n");

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
	printf("[Step 999] Disconnect IQTester.\n");	

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

void printProgramInfo()
{
	printf("Program version: \t%s\n",__this__version__);
	printf("Programmed By:   \t%s\n",__this__author__);
	printf("Built Date:      \t%s\n",__DATE__);
	printf("Built Time:      \t%s\n",__TIME__);
	printf("Code Lines:      \t%d\n",__LINE__);

}

void SetSolutionIndex(char *pszIndex)
{
	TCHAR wszFolder[BUFFER_SIZE]=_T("");
	GetCurrentDirectory(BUFFER_SIZE,wszFolder);

	TCHAR wszTesterCtrlFilename[BUFFER_SIZE]=_T("");
	swprintf_s(wszTesterCtrlFilename,BUFFER_SIZE,L"%s\\%s",wszFolder,_T("LP_DUT_Setup.ini"));
	TCHAR wszIndex[12]=_T("");
	Char2WChar(pszIndex,strlen(pszIndex),wszIndex,12);
	WritePrivateProfileString(_T("Configuration"),_T("SolutionIndex"),wszIndex,wszTesterCtrlFilename);
}


void ReadRegistry()
{
	//Tony 2012.12.14
	//Read registry to set VSport left is 2, VSport right is 3
	//char szVsport[256]="Left";
	//sprintf(szVsport,"%s","Left");
	//char szRemoteIP[256]="";
	HKEY   hk;
	DWORD  dwDisp;
	TCHAR RegBuf[256] = _T("SOFTWARE\\Netgear\\STATION\\IQServer");
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,
					RegBuf,
					0,
					NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_WRITE | KEY_QUERY_VALUE,
					NULL,
					&hk,
					&dwDisp))
	{
		printf("Open registry fail\r\n");
	}
	else
	{
		if(dwDisp == REG_CREATED_NEW_KEY)
		{
			printf("Create new key\r\n");
			
			if(RegSetValueEx(hk,_T("VSPORT"),0,REG_SZ,(LPBYTE)szVsport,(DWORD)strlen(szVsport)+1)	)	
			//||RegSetValueEx(hk,_T("RemoteIP"),0,REG_NONE,(LPBYTE)szRemoteIP,(DWORD)strlen(szRemoteIP)+1))	
				
			{
				RegCloseKey(hk); 
			}
		}
		//else
		//{
			//printf("Read old key\r\n");		
			DWORD dwBuf1 = 256;
			if(RegQueryValueEx(hk,_T("VSPORT"),NULL,NULL,(LPBYTE)szVsport,&dwBuf1)	)
			   //||RegQueryValueEx(hk,_T("RemoteIP"),NULL,NULL,(LPBYTE)szRemoteIP,&dwBuf3)
						
			{
				printf("Get Info from registry fail\r\n");
				RegCloseKey(hk);
			}
			printf("**********Version IQ_V1.1************\r\n");
			if(strstr(szVsport,"2"))
			{
				WritePrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSA_PORT"),_T("2"),_T(".\\global_settings.ini"));
				WritePrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSG_PORT"),_T("2"),_T(".\\global_settings.ini"));
				WritePrivateProfileString(_T("TEST_CONTROL"),_T("DH_TOKEN_ID"),_T("2"),_T(".\\global_settings.ini"));
				printf("VSPORT :\t\t Left\n");
			}
			else
			{
				WritePrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSA_PORT"),_T("3"),_T(".\\global_settings.ini"));
				WritePrivateProfileString(_T("GLOBAL_SETTINGS"),_T("VSG_PORT"),_T("3"),_T(".\\global_settings.ini"));
				WritePrivateProfileString(_T("TEST_CONTROL"),_T("DH_TOKEN_ID"),_T("3"),_T(".\\global_settings.ini"));
				printf("VSPORT :\t\t Right\n");
			}
			RegCloseKey(hk);
			
		//}
	}

}
void WriteRegistry()
{
	 HKEY   hk;
	DWORD  dwDisp;
	TCHAR RegBuf[256] = _T("SOFTWARE\\Netgear\\STATION\\IQServer");
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,
					RegBuf,
					0,
					NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_WRITE | KEY_QUERY_VALUE,
					NULL,
					&hk,
					&dwDisp))
	{
		printf("Open registry fail\r\n");
	}
	else
	{
		if(dwDisp == REG_CREATED_NEW_KEY)
		{
			printf("Create new key\r\n");
			
			if(RegSetValueEx(hk,_T("VSPORT"),0,REG_SZ,(LPBYTE)szVsport,(DWORD)strlen(szVsport)+1)	)	
			//||RegSetValueEx(hk,_T("RemoteIP"),0,REG_NONE,(LPBYTE)szRemoteIP,(DWORD)strlen(szRemoteIP)+1))	
				
			{
				RegCloseKey(hk); 
			}
		}
		//else
		//{
			//printf("Read old key\r\n");		
			DWORD dwBuf1 = 256;
			if(RegQueryValueEx(hk,_T("VSPORT"),NULL,NULL,(LPBYTE)szVsport,&dwBuf1)	)
			   //||RegQueryValueEx(hk,_T("RemoteIP"),NULL,NULL,(LPBYTE)szRemoteIP,&dwBuf3)
						
			{
				printf("Get Info from registry fail\r\n");
				RegCloseKey(hk);
			}

			RegSetValueEx(hk,_T("VSPORT"),0,REG_SZ,(LPBYTE)szVsport,(DWORD)strlen(szVsport)+1);		
				
			
			RegCloseKey(hk);
			
		//}
	}
}
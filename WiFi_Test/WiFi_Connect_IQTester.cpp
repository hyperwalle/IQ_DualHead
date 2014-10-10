#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "StringUtil.h"


bool            l_IQ201xFound = false;
int             g_iTesterName = IQTYPE_2010;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_connectTesterParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_connectTesterReturnMap;

struct tagParam
{
	char    IQTESTER_IP01[MAX_BUFFER_SIZE];				/*!< Tester number 1 IP Address */
	char    IQTESTER_IP02[MAX_BUFFER_SIZE];				/*!< Tester number 2 IP Address */
	char    IQTESTER_IP03[MAX_BUFFER_SIZE];				/*!< Tester number 3 IP Address */
	char    IQTESTER_IP04[MAX_BUFFER_SIZE];				/*!< Tester number 4 IP Address */
	int     IQTESTER_RECONNECT;							/*!< A flag to control reconnect of IQ Tester */
	int	    IQTESTER_TYPE;								/* < Tester type */
} l_connectTesterParam;

struct tagReturn
{
	char    IQLITE_IQTESTER_INFO[MAX_BUFFER_SIZE];			   /*!< IQTester info string, such as HW version, firmware version, etc. */
	char    IQLITE_IQTESTER_SERIAL_NUM_01[MAX_BUFFER_SIZE];    /*!< Tester number 1 serial number */
	char    IQLITE_IQTESTER_SERIAL_NUM_02[MAX_BUFFER_SIZE];    /*!< Tester number 2 serial number */
	char    IQLITE_IQTESTER_SERIAL_NUM_03[MAX_BUFFER_SIZE];    /*!< Tester number 3 serial number */
	char    IQLITE_IQTESTER_SERIAL_NUM_04[MAX_BUFFER_SIZE];    /*!< Tester number 4 serial number */
	char    IQLITE_IQAPI_VERSION[MAX_BUFFER_SIZE];             /*!< IQapi version */
	char    IQLITE_IQAPITC_VERSION[MAX_BUFFER_SIZE];           /*!< IQapitc library version */
	char    IQLITE_IQMEASURE_VERSION[MAX_BUFFER_SIZE];         /*!< IQmeasure library version */
	char    IQLITE_IQV_ANALYSIS_VERSION[MAX_BUFFER_SIZE];      /*!< IQv_analysis library version */
	char    IQLITE_IQV_MW_VERSION[MAX_BUFFER_SIZE];            /*!< IQv middle wear version */
	char    IQLITE_IQ_NXN_VERSION[MAX_BUFFER_SIZE];            /*!< IQnxn version */
	char    IQLITE_IQ_NXN_SIGNAL_VERSION[MAX_BUFFER_SIZE];     /*!< IQnxn signal library version */
	char    IQLITE_IQ_MAX_SIGNAL_VERSION[MAX_BUFFER_SIZE];     /*!< IQmax signal library version */
	char    IQLITE_IQ_ZIGBEE_SIGNAL_VERSION[MAX_BUFFER_SIZE];  /*!< IQzigbee signal library version */
	char    IQLITE_BLUETOOTH_VERSION[MAX_BUFFER_SIZE];         /*!< Bluetooth library version */
	char    IQLITE_TEST_MANAGER_VERSION[MAX_BUFFER_SIZE];	   /*!< Information of the TestManager DLL, such as version. */
	char	IQLITE_ERROR_MESSAGE[MAX_BUFFER_SIZE];			   /*!< Error msg while connecting Tester. */
	char    IQLITE_IQTESTER_HARDWARE_VER[4][MAX_BUFFER_SIZE];  /*!< Testers HW version */
} l_connectTesterReturn;

// -cfy@sunnyvale, 2012/3/13-
#ifndef WIN32
int initConnectTesterContainer = InitializeConnectTesterContainers();
#endif
/* <><~~ */

void ClearConnectLPReturn(void)
{
	l_connectTesterParamMap.clear();
	l_connectTesterReturnMap.clear();
}

//! Connect to Tester (IQview/flex, IQmax, IQnxn, IQmimo, etc.)
/*!
 * Input Parameters
 *
 *  - Mandatory
 *		-# IQTESTER_IP01 (char): Tester number 1 IP address.
 *		-# IQTESTER_IP02 (char): Tester number 2 IP address.
 *		-# IQTESTER_IP03 (char): Tester number 3 IP address.
 *		-# IQTESTER_IP04 (char): Tester number 4 IP address.
 *      -# IQTESTER_RECONNECT (int):   A flag to control reconnect of IQ tester. 0: No reconnect; 1: Reconnect; Default=0
 *
 * Return Values
 *      -# IQTESTER_INFO (char):			A string that contains all tester info, such as s/w versions, h/w serial number, etc.
 *      -# IQTESTER_SERIAL_NUM_01 (char):	Tester number 1 serial number
 *      -# IQTESTER_SERIAL_NUM_02 (char):	Tester number 2 serial number
 *      -# IQTESTER_SERIAL_NUM_03 (char):	Tester number 3 serial number
 *      -# IQTESTER_SERIAL_NUM_04 (char):	Tester number 4 serial number
 *      -# IQAPI_VERSION (char):			IQapi version
 *      -# IQAPITC_VERSION (char):			IQapitc library version
 *      -# IQMEASURE_VERSION (char):		IQmeasure library version
 *      -# IQV_ANALYSIS_VERSION (char):		IQv_analysis library version
 *      -# IQV_MW_VERSION (char):			IQv middle wear version
 *      -# IQ_NXN_VERSION (char):			IQnxn version
 *      -# IQ_NXN_SIGNAL_VERSION (char):	IQnxn signal library version
 *      -# IQ_MAX_SIGNAL_VERSION (char):	IQmax signal library version
 *      -# IQ_ZIGBEE_SIGNAL_VERSION (char):	IQzigbee signal library version
 *      -# BLUETOOTH_VERSION (char):		Bluetooth library version
 *      -# TEST_MANAGER_VERSION (char):     Information of the TestManager DLL, such as version
 *      -# ERROR_MESSAGE (char):			A string for error message
 *
 * \return 0 No error occurred
 * \return -1 DUT failed to insert.  Please see the returned error message for details
 */

WIFI_TEST_API int WiFi_Connect_IQTester(void)
{
	int  err = ERR_OK;
	int  dummyValue = 0;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	vector<double>	    rfAmplDb(MAX_TESTER_NUM, NA_NUMBER);
	vector<int>		    vsgRFEnabled(MAX_TESTER_NUM, (int)NA_NUMBER);


	/*---------------------------------------*
	 * Clear Return Parameters and Container *
	 *---------------------------------------*/
	ClearReturnParameters(l_connectTesterReturnMap);

	/*------------------------*
	 * Respond to QUERY_INPUT *
	 *------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_connectTesterParamMap);
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
		RespondToQueryReturn(l_connectTesterReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}

	try
	{

		/*---------------------------------------*
		 * g_WiFi_Test_ID need to be valid (>=0) *
		 *---------------------------------------*/
		if( g_WiFi_Test_ID<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFi_Test_ID = %d.\n", g_WiFi_Test_ID);
		}

		TM_ClearReturns(g_WiFi_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_connectTesterParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get input parameters return OK.\n");
		}

		g_Tester_Reconnect = l_connectTesterParam.IQTESTER_RECONNECT;

		if( (0==g_Tester_Number) || g_Tester_Reconnect )
		{
			// Connect to IQ tester(s) if no connection, or reconnect every time

			// At least IP01 must be valid, otherwise return error directly.
			err = ValidateIPAddress(l_connectTesterParam.IQTESTER_IP01);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Invalid IP address (%s).\n", l_connectTesterParam.IQTESTER_IP01);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] IP address = (%s).\n", l_connectTesterParam.IQTESTER_IP01);
			}

			// To initialize the IQTester environment
			// check tester type; added IQlite Merge 2012-04-13

			if (l_connectTesterParam.IQTESTER_TYPE == IQTYPE_2010)
			{
				g_iTesterName = IQTYPE_2010;
				LP_SetTesterName("IQView");
				err = LP_Init(IQTYPE_2010);
			}
			else
			{
				g_iTesterName = IQTYPE_XEL;
				LP_SetTesterName("IQxel");
				err = LP_Init(IQTYPE_XEL);
			}
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQapi LP_Init() Failed.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] IQapi LP_Init() return OK.\n");
			}


			// Initialize ok, then valid other IP address
			if ( (ERR_OK!=ValidateIPAddress(l_connectTesterParam.IQTESTER_IP02))&&
					(ERR_OK!=ValidateIPAddress(l_connectTesterParam.IQTESTER_IP03))&&
					(ERR_OK!=ValidateIPAddress(l_connectTesterParam.IQTESTER_IP04)) )
			{
				// IP 01: valid, IP 02: invalid, IP 03: invalid, IP 04: invalid
				err = LP_InitTester(l_connectTesterParam.IQTESTER_IP01);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function LP_InitTester(%s) Failed.\n", l_connectTesterParam.IQTESTER_IP01);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function LP_InitTester(%s) return OK.\n", l_connectTesterParam.IQTESTER_IP01);
				}
				g_Tester_Type   = IQ_View;
				g_Tester_Number = 1;
			}
			else if ( (ERR_OK==ValidateIPAddress(l_connectTesterParam.IQTESTER_IP02))&&
					(ERR_OK!=ValidateIPAddress(l_connectTesterParam.IQTESTER_IP03))&&
					(ERR_OK!=ValidateIPAddress(l_connectTesterParam.IQTESTER_IP04)) )
			{
				// IP 01: valid, IP 02: valid, IP 03: invalid, IP 04: invalid
				err = LP_InitTester2(l_connectTesterParam.IQTESTER_IP01, l_connectTesterParam.IQTESTER_IP02);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function LP_InitTester2(%s, %s) Failed.\n", l_connectTesterParam.IQTESTER_IP01, l_connectTesterParam.IQTESTER_IP02);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function LP_InitTester2(%s, %s) return OK.\n", l_connectTesterParam.IQTESTER_IP01, l_connectTesterParam.IQTESTER_IP02);
				}
				g_Tester_Type   = IQ_nxn;
				g_Tester_Number = 2;
			}
			else if ( (ERR_OK==ValidateIPAddress(l_connectTesterParam.IQTESTER_IP02))&&
					(ERR_OK==ValidateIPAddress(l_connectTesterParam.IQTESTER_IP03))&&
					(ERR_OK!=ValidateIPAddress(l_connectTesterParam.IQTESTER_IP04)) )
			{
				// IP 01: valid, IP 02: valid, IP 03: valid, IP 04: invalid
				err = LP_InitTester3(l_connectTesterParam.IQTESTER_IP01,
						l_connectTesterParam.IQTESTER_IP02,
						l_connectTesterParam.IQTESTER_IP03);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function LP_InitTester3(%s, %s, %s) Failed.\n", l_connectTesterParam.IQTESTER_IP01, l_connectTesterParam.IQTESTER_IP02, l_connectTesterParam.IQTESTER_IP03);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function LP_InitTester3(%s, %s, %s) return OK.\n", l_connectTesterParam.IQTESTER_IP01, l_connectTesterParam.IQTESTER_IP02, l_connectTesterParam.IQTESTER_IP03);
				}
				g_Tester_Type   = IQ_nxn;
				g_Tester_Number = 3;
			}
			else if ( (ERR_OK==ValidateIPAddress(l_connectTesterParam.IQTESTER_IP02))&&
					(ERR_OK==ValidateIPAddress(l_connectTesterParam.IQTESTER_IP03))&&
					(ERR_OK==ValidateIPAddress(l_connectTesterParam.IQTESTER_IP04)) )
			{
				// IP 01: valid, IP 02: valid, IP 03: valid, IP 04: valid
				err = LP_InitTester4(l_connectTesterParam.IQTESTER_IP01,
						l_connectTesterParam.IQTESTER_IP02,
						l_connectTesterParam.IQTESTER_IP03,
						l_connectTesterParam.IQTESTER_IP04);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function LP_InitTester4(%s, %s, %s, %s) Failed.\n", l_connectTesterParam.IQTESTER_IP01, l_connectTesterParam.IQTESTER_IP02, l_connectTesterParam.IQTESTER_IP03, l_connectTesterParam.IQTESTER_IP04);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function LP_InitTester4(%s, %s, %s, %s) return OK.\n", l_connectTesterParam.IQTESTER_IP01, l_connectTesterParam.IQTESTER_IP02, l_connectTesterParam.IQTESTER_IP03, l_connectTesterParam.IQTESTER_IP04);
				}
				g_Tester_Type   = IQ_nxn;
				g_Tester_Number = 4;
			}
			else
			{
				// Invalid IP address sequence
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Invalid IP address sequence for IQnxn.\n");
				throw logMessage;
			}

			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] IQtester initiation accomplished.\n");

			if ( g_Tester_Number==1 )
			{
				// Call LP_SetVsa() for apply VSA and VSG port setting
				err = ::LP_SetVsa(  2412*1e6,
						0,
						g_WiFiGlobalSettingParam.VSA_PORT,
						0,
						g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
						g_WiFiGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
						);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Fail to setup VSA, LP_SetVsa() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsa() return OK.\n");
				}

				/*----------------------------*
				 * Disable VSG output signal  *
				 *----------------------------*/
				// make sure no signal is generated by the VSG
				err = ::LP_EnableVsgRF(0);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
				}
			}
			else
			{
				// Call LP_SetVsaNxN() for apply VSA and VSG port setting from global setting
				for(int i = 0; i < MAX_TESTER_NUM; i++)
				{
					rfAmplDb[i] = 0.0;
					vsgRFEnabled[i] = 0; // Disable VSG
				}

				err = ::LP_SetVsaNxN(   2412*1e6,
						&rfAmplDb[0],
						&g_WiFiGlobalSettingParam.VSA_PORT,
						0,
						g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
						g_WiFiGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
						);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Fail to setup VSA, LP_SetVsaNxN() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsaNxN() return OK.\n");
				}

				//Turn Off VSG in case signal out after initial testers
				err = ::LP_EnableVsgRFNxN(vsgRFEnabled[0], vsgRFEnabled[1], vsgRFEnabled[2], vsgRFEnabled[3]);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Fail to setup VSA, LP_EnableVsgRFNxN() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_EnableVsgRFNxN() return OK.\n");
				}
			}

			// For retrieval of the IQTester version info
			err = ::LP_GetVersion(l_connectTesterReturn.IQLITE_IQTESTER_INFO, MAX_BUFFER_SIZE);
			if (false==err)
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get IQTester version information.\n");
				throw logMessage;
			}
			else
			{
				err = 0;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get IQTester version information return OK.\n");
			}

			// Parse version string to break out the different
			map<string, string> versionMap;
			ParseSplitString(l_connectTesterReturn.IQLITE_IQTESTER_INFO, ":", versionMap);

			strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_01, MAX_BUFFER_SIZE, versionMap["Tester 1 SN"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_02, MAX_BUFFER_SIZE, versionMap["Tester 2 SN"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_03, MAX_BUFFER_SIZE, versionMap["Tester 3 SN"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_04, MAX_BUFFER_SIZE, versionMap["Tester 4 SN"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQAPI_VERSION, MAX_BUFFER_SIZE, versionMap["iqapi"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQAPITC_VERSION, MAX_BUFFER_SIZE, versionMap["iqapitc"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQMEASURE_VERSION, MAX_BUFFER_SIZE, versionMap["IQmeasure"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQV_ANALYSIS_VERSION, MAX_BUFFER_SIZE, versionMap["iqv_analysis"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQV_MW_VERSION, MAX_BUFFER_SIZE, versionMap["IQV_mw"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQ_NXN_VERSION, MAX_BUFFER_SIZE, versionMap["iq_nxn"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQ_NXN_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_nxn_signal"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQ_MAX_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_max_signal"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQ_ZIGBEE_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_zigbee_signal"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_BLUETOOTH_VERSION, MAX_BUFFER_SIZE, versionMap["bluetooth"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_HARDWARE_VER[0], MAX_BUFFER_SIZE, versionMap["Tester 1 hardware version"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_HARDWARE_VER[1], MAX_BUFFER_SIZE, versionMap["Tester 2 hardware version"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_HARDWARE_VER[2], MAX_BUFFER_SIZE, versionMap["Tester 3 hardware version"].c_str() );
			strcpy_s( l_connectTesterReturn.IQLITE_IQTESTER_HARDWARE_VER[3], MAX_BUFFER_SIZE, versionMap["Tester 4 hardware version"].c_str() );

			// -cfy@sunnyvale, 2012/3/13-
			//determine tester type by hardware version
			char *ptr;
			double hardwareVer = 0.0;

			ptr = strtok( l_connectTesterReturn.IQLITE_IQTESTER_HARDWARE_VER[0], ".");
			if (ptr)
			{
				hardwareVer= atof(ptr);
				{
					ptr = strtok(NULL, ".");
					if (ptr)
					{
						hardwareVer = hardwareVer + atof(ptr)/10;
						err = ::SetTesterHwVersion(hardwareVer);
						if(err != ERR_OK)
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function SetTesterHwVersion() Failed.\n");
							throw logMessage;
						}
						else
						{
							// do nothing /*LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function SetTesterHwVersion() OK.\n");*/
						}
						//printf( " %f\n", hardwareVer);
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown hardware version format. Quit.\n");
						err=ERR_NO_CONNECTION;
						throw logMessage;
					}
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown hardware version format. Quit.\n");
				err=ERR_NO_CONNECTION;
				throw logMessage;
			}


			//although WiFi technology does not support multiple testers, but old code enable it. For safety, add version check just in case.
			//Check whether testers FW/VSA/VSG version are consistent or not
			if(g_Tester_Number >1)
			{
				for (int testerIndex =1; testerIndex < g_Tester_Number; testerIndex++)
				{
					string tempStr = l_connectTesterReturn.IQLITE_IQTESTER_HARDWARE_VER[testerIndex];
					char tempChar [MAX_BUFFER_SIZE]={"1.5"};
					sprintf_s(tempChar, MAX_BUFFER_SIZE,"%0.1f", hardwareVer);
					if(tempStr.find(tempChar)!= string::npos)
					{
						//do nothing
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Testers do not have same hardware versions. Quit.\n");
						err=ERR_NO_CONNECTION;
						throw logMessage;
					}
				}

				err = ::CheckTesterConsistentStatus(versionMap);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Versions of testers are not consistent.\n");
					err=ERR_NO_CONNECTION;
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi]  Versions of testers are consistent.\n");
				}
			}
			else
			{
				//do nothing
			}
			/* <><~~ */

			// IQ2010 trigger setting tweak
			if (strstr(l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_01, "IQP"))
			{
				l_IQ201xFound = true;
				LP_SetTesterName("IQ2010");
				if ( g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE==6 )   g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE = 13; //IQV_TRIG_TYPE_FOR_IQ2010
				if ( g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB < -20 )
				{
					g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB = -20;  // some 2010 cannot trigger with -25 dB, so -20 will be more safe with 2010
				}
				else
				{
					//do nothing, using global setting input
				}
			}
			else
			{
				//do nothing, using the global setting input from user
			}

			if ( (strstr(l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_01, "IQP"))&&(g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE==1) )
			{
				g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE = 1;
				err = LP_IQ2010EXT_Init();
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to initial the IQ2010Ext feature, LP_IQ2010EXT_Init() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Enable the IQ2010Ext feature.\n");
				}
			}
			else
			{
				g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE = 0;
			}

			// For retrieval of the TestManager version info
			err = ::TM_GetVersion(l_connectTesterReturn.IQLITE_TEST_MANAGER_VERSION, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get TestManager version information.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get TestManager version information return OK.\n");
			}
		}
		else
		{
			// Reconnect=0
			err = ERR_OK;
		}

		// This case take care that "IQTESTER_RECONNECT=0", but "VSA_TRIGGER_TYPE=6" from "Global Settings".
		if ( l_IQ201xFound&&(g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE==6) )
		{
			g_WiFiGlobalSettingParam.VSA_TRIGGER_TYPE = 13;		//IQV_TRIG_TYPE_FOR_IQ2010
		}
		else
		{
			// do nothing
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_connectTesterReturn.IQLITE_ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_connectTesterReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		g_WiFi_Test_ID = -1;
		ReturnErrorMessage(l_connectTesterReturn.IQLITE_ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		g_WiFi_Test_ID = -1;
		ReturnErrorMessage(l_connectTesterReturn.IQLITE_ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;
	}

	rfAmplDb.clear();
	vsgRFEnabled.clear();

	return err;
}

int InitializeConnectTesterContainers(void)
{
	/*------------------*
	 * Input Parameters: *
	 * IQTESTER_IP01    *
	 *------------------*/
	l_connectTesterParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	strcpy_s(l_connectTesterParam.IQTESTER_IP01, MAX_BUFFER_SIZE, "192.168.100.254");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterParam.IQTESTER_IP01))    // Type_Checking
	{
		setting.value       = (void*)l_connectTesterParam.IQTESTER_IP01;
		setting.unit        = "";
		setting.helpText    = "IP address of LitePoint tester";
		l_connectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_IP01", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*------------------*
	 * Input Parameters: *
	 * IQTESTER_IP02    *
	 *------------------*/

	strcpy_s(l_connectTesterParam.IQTESTER_IP02, MAX_BUFFER_SIZE, "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterParam.IQTESTER_IP02))    // Type_Checking
	{
		setting.value       = (void*)l_connectTesterParam.IQTESTER_IP02;
		setting.unit        = "";
		setting.helpText    = "IP address of LitePoint tester";
		l_connectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_IP02", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*------------------*
	 * Input Parameters: *
	 * IQTESTER_IP03    *
	 *------------------*/

	strcpy_s(l_connectTesterParam.IQTESTER_IP03, MAX_BUFFER_SIZE, "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterParam.IQTESTER_IP03))    // Type_Checking
	{
		setting.value       = (void*)l_connectTesterParam.IQTESTER_IP03;
		setting.unit        = "";
		setting.helpText    = "IP address of LitePoint tester";
		l_connectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_IP03", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*------------------*
	 * Input Parameters: *
	 * IQTESTER_IP04    *
	 *------------------*/

	strcpy_s(l_connectTesterParam.IQTESTER_IP04, MAX_BUFFER_SIZE, "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterParam.IQTESTER_IP04))    // Type_Checking
	{
		setting.value       = (void*)l_connectTesterParam.IQTESTER_IP04;
		setting.unit        = "";
		setting.helpText    = "IP address of LitePoint tester";
		l_connectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_IP04", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*-------------------*
	 * Input Parameters:  *
	 * IQTESTER_RECONNECT*
	 *-------------------*/

	l_connectTesterParam.IQTESTER_RECONNECT = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_connectTesterParam.IQTESTER_RECONNECT))    // Type_Checking
	{
		setting.value       = (void*)&l_connectTesterParam.IQTESTER_RECONNECT;
		setting.unit        = "";
		setting.helpText    = "A flag to control reconnect of IQ tester.\r\n1-Reconnect;0-No reconnect; Default=0";
		l_connectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_RECONNECT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*-------------------*
	 * Input Parameters:  *
	 * IQTESTER_TYPE*
	 *-------------------*/

	l_connectTesterParam.IQTESTER_TYPE = IQTYPE_2010;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_connectTesterParam.IQTESTER_TYPE))    // Type_Checking
	{
		setting.value       = (void*)&l_connectTesterParam.IQTESTER_TYPE;
		setting.unit        = "";
		setting.helpText    = "Tester type.0: non-IQXel;  1: IQXel.\r\n Default vaule: 0 ";
		l_connectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_TYPE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*----------------*
	 * Return Values: *
	 * IQTester Info  *
	 *----------------*/
	l_connectTesterReturnMap.clear();

	l_connectTesterReturn.IQLITE_IQTESTER_INFO[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQTESTER_INFO))    // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQTESTER_INFO;
		setting.unit        = "";
		setting.helpText    = "IQTester info, such as HW version, firmware version, etc.";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_INFO", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*----------------------------*
	 * Return Values:             *
	 * IQTester 1 Serial Number   *
	 *----------------------------*/
	l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_01[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_01))  // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_01;
		setting.unit        = "";
		setting.helpText    = "Tester number 1 serial number";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_SERIAL_NUM_01", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*----------------------------*
	 * Return Values:             *
	 * IQTester 2 Serial Number   *
	 *----------------------------*/
	l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_02[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_02))  // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_02;
		setting.unit        = "";
		setting.helpText    = "Tester number 2 serial number";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_SERIAL_NUM_02", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*----------------------------*
	 * Return Values:             *
	 * IQTester 3 Serial Number   *
	 *----------------------------*/

	l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_03[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_03))  // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_03;
		setting.unit        = "";
		setting.helpText    = "Tester number 3 serial number";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_SERIAL_NUM_03", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*----------------------------*
	 * Return Values:             *
	 * IQTester 4 Serial Number   *
	 *----------------------------*/
	l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_04[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_04))  // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQTESTER_SERIAL_NUM_04;
		setting.unit        = "";
		setting.helpText    = "Tester number 4 serial number";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_SERIAL_NUM_04", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*---------------------*
	 * Return Values:      *
	 * IQAPI Version Info  *
	 *---------------------*/

	l_connectTesterReturn.IQLITE_IQAPI_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQAPI_VERSION))   // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQAPI_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQapi version";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQAPI_VERSION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*-------------------------------*
	 * Return Values:                *
	 * IQAPItc library version Info  *
	 *-------------------------------*/

	l_connectTesterReturn.IQLITE_IQAPITC_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQAPITC_VERSION)) // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQAPITC_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQapitc library version";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQAPITC_VERSION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*--------------------------------*
	 * Return Values:                 *
	 * IQmeasure library version Info *
	 *--------------------------------*/
	l_connectTesterReturn.IQLITE_IQMEASURE_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQMEASURE_VERSION))   // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQMEASURE_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQmeasure library version";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQMEASURE_VERSION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*---------------------------------*
	 * Return Values:                  *
	 * IQanalysis library version Info *
	 *---------------------------------*/
	l_connectTesterReturn.IQLITE_IQV_ANALYSIS_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQV_ANALYSIS_VERSION))    // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQV_ANALYSIS_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQv_analysis library version";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQV_ANALYSIS_VERSION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*------------------------------*
	 * Return Values:               *
	 * IQv middle wear version Info *
	 *------------------------------*/
	l_connectTesterReturn.IQLITE_IQV_MW_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQV_MW_VERSION))  // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQV_MW_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQv middle wear version";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQV_MW_VERSION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*--------------------*
	 * Return Values:     *
	 * IQnxn version Info *
	 *--------------------*/
	l_connectTesterReturn.IQLITE_IQ_NXN_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQ_NXN_VERSION))  // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQ_NXN_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQnxn version";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQ_NXN_VERSION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*-----------------------------------*
	 * Return Values:                    *
	 * IQnxn signal library version Info *
	 *-----------------------------------*/
	l_connectTesterReturn.IQLITE_IQ_NXN_SIGNAL_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQ_NXN_SIGNAL_VERSION))   // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQ_NXN_SIGNAL_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQnxn signal library version";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQ_NXN_SIGNAL_VERSION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*-----------------------------------*
	 * Return Values:                    *
	 * IQmax signal library version Info *
	 *-----------------------------------*/
	l_connectTesterReturn.IQLITE_IQ_MAX_SIGNAL_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQ_MAX_SIGNAL_VERSION))   // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQ_MAX_SIGNAL_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQmax signal library version";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQ_MAX_SIGNAL_VERSION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*--------------------------------------*
	 * Return Values:                       *
	 * IQzigbee signal library version Info *
	 *--------------------------------------*/
	l_connectTesterReturn.IQLITE_IQ_ZIGBEE_SIGNAL_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_IQ_ZIGBEE_SIGNAL_VERSION))    // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_IQ_ZIGBEE_SIGNAL_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQzigbee signal library version";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQ_ZIGBEE_SIGNAL_VERSION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*--------------------------------*
	 * Return Values:                 *
	 * Bluetooth library version Info *
	 *--------------------------------*/
	l_connectTesterReturn.IQLITE_BLUETOOTH_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_BLUETOOTH_VERSION))   // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_BLUETOOTH_VERSION;
		setting.unit        = "";
		setting.helpText    = "Bluetooth library version";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("BLUETOOTH_VERSION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*----------------------*
	 * Return Values:       *
	 * TestManager DLL Info *
	 *----------------------*/
	l_connectTesterReturn.IQLITE_TEST_MANAGER_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_TEST_MANAGER_VERSION))    // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_TEST_MANAGER_VERSION;
		setting.unit        = "";
		setting.helpText    = "Information of the TestManager DLL, such as version.";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TEST_MANAGER_VERSION", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*--------------------------------*
	 * Return Values:                 *
	 * Error Msg while connect Tester *
	 *--------------------------------*/
	l_connectTesterReturn.IQLITE_ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_connectTesterReturn.IQLITE_ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_connectTesterReturn.IQLITE_ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_connectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	return 0;
}

int ValidateIPAddress(char* ipAddress)
{
	int err = ERR_OK;

	if ( ipAddress!=NULL )
	{
		if ( 0!=strcmp(ipAddress,"") )
		{
			// -cfy@sunnyvale, 2012/3/13-
			string			serialNumber = ipAddress;
			vector<string>	splits;


			splits.clear();
			SplitString(ipAddress, splits, ":");
			if( splits.size()==3 )
			{
				//Format is assumed to be right for LP_InitTester with IQ2010 testers.

			}
			else if(serialNumber.find("IQ") == 0)
			{
				//Serial number.  Add the correct formating to the string.
				string fomatString = "127.0.0.1:4001:" + serialNumber;
				strcpy_s(ipAddress, fomatString.size()+1, fomatString.c_str());
			}
			else
			{
				// Assume it is a valid IP address, don't need to check the IP format here.
				// Because the LP_InitTester function will take care the error return.
			}
			/* <><~~ */
		}
		else
		{
			// IP = "", return error
			err = ERR_INVALID_IP_ADDRESS;
		}
	}
	else
	{
		// IP = NULL, return error
		err = ERR_INVALID_IP_ADDRESS;
	}

	return err;
}

void ParseSplitString(const std::string &splitteValue, const std::string spliteString, std::map<std::string, std::string> &spliteMap)
{
	/*
IQmeasure: 1.7.12.1.2 (2009-05-29)
iqapi:      1.7.12.1 (May 19  2009)
iqapitc:     1.2.28 May 15, 2009
iq_nxn:      1.1.5g (2009-05-13)
IQV_mw:      3.2.5.f Sep 08, 2008
Tester 1 SN: IQV08441
Tester 2 SN: IQV08185
iq_nxn_signal:    1.1.14h (2008-08-07)
iq_max_signal:    1.0.46 (2008-08-26)
iq_zigbee_signal:    1.0.7 (2009-01-23)
bluetooth: 1.1.1.a, Febr 5, 2009
iqv_analysis: 1.4.0.v Jan 29, 2009
*/
	string::size_type start = 0;
	string::size_type end = splitteValue.find( '\n', start );
	while( string::npos != end )
	{
		string line = splitteValue.substr(start, end - start);

		string::size_type delimiter = line.find(spliteString);
		string ver = line.substr(0, delimiter);
		string val = line.substr(delimiter + 1);

		spliteMap[Trim( ver )] = Trim( val );

		start = end + 1;
		end = splitteValue.find( '\n', start );
	}
}


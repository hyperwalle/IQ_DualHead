#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "StringUtil.h"

// This global variable is declared in WiFi_Test_Internal.cpp
extern TM_ID    g_WiFi_11ac_Test_ID;
extern int		g_Tester_Type;
extern int      g_Tester_Number;
extern int      g_Tester_Reconnect;
extern int      g_Tester_DualHead;	// add by daixin
extern bool		g_Tester_EnableVHT80;
int             g_iTesterName = IQTYPE_XEL;
// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_WiFi11ACGlobalSettingParam;

using namespace std;

// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACconnectTesterParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_11ACconnectTesterReturnMap;

struct tagParam
{
	char    IQTESTER_IP01[MAX_BUFFER_SIZE];				/*!< Tester number 1 IP Address */
	char    IQTESTER_IP02[MAX_BUFFER_SIZE];				/*!< Tester number 2 IP Address */
	char    IQTESTER_IP03[MAX_BUFFER_SIZE];				/*!< Tester number 3 IP Address */
	char    IQTESTER_IP04[MAX_BUFFER_SIZE];				/*!< Tester number 4 IP Address */
	int     IQTESTER_RECONNECT;							/*!< A flag to control reconnect of IQ Tester */
	int	    IQTESTER_TYPE;								/* < Tester type */
	int 	IQTESTER_CONTROL_METHOD;					/* IQAPI 0, Direct SCPI 1 */

	// add by daixin for dual-head test, 2012-12-10
	int	    DH_ENABLE;									/*!< A flag to control use dual-head test */
	int     DH_OBTAIN_CONTROL_TIMEOUT_MS;				/*!< Dual-head obtain query timeout value */
	int     DH_PROBE_TIME_MS;							/*!< Dual-head obtain query time interval */
	int     DH_TOKEN_ID;								/*!< Dual-head obtain Token ID */
	// add end
} l_11ACconnectTesterParam;

struct tagReturn
{
	char    IQTESTER_INFO[MAX_BUFFER_SIZE];				/*!< IQTester info string, such as HW version, firmware version, etc. */
	char    IQTESTER_SERIAL_NUM_01[MAX_BUFFER_SIZE];    /*!< Tester number 1 serial number */
	char    IQTESTER_SERIAL_NUM_02[MAX_BUFFER_SIZE];    /*!< Tester number 2 serial number */
	char    IQTESTER_SERIAL_NUM_03[MAX_BUFFER_SIZE];    /*!< Tester number 3 serial number */
	char    IQTESTER_SERIAL_NUM_04[MAX_BUFFER_SIZE];    /*!< Tester number 4 serial number */
	char    IQAPI_VERSION[MAX_BUFFER_SIZE];             /*!< IQapi version */
	char    IQAPITC_VERSION[MAX_BUFFER_SIZE];           /*!< IQapitc library version */
	char    IQMEASURE_VERSION[MAX_BUFFER_SIZE];         /*!< IQmeasure library version */
	char    IQV_ANALYSIS_VERSION[MAX_BUFFER_SIZE];      /*!< IQv_analysis library version */
	char    IQV_MW_VERSION[MAX_BUFFER_SIZE];            /*!< IQv middle wear version */
	char    IQ_NXN_VERSION[MAX_BUFFER_SIZE];            /*!< IQnxn version */
	char    IQ_NXN_SIGNAL_VERSION[MAX_BUFFER_SIZE];     /*!< IQnxn signal library version */
	char    IQ_MAX_SIGNAL_VERSION[MAX_BUFFER_SIZE];     /*!< IQmax signal library version */
	char    IQ_ZIGBEE_SIGNAL_VERSION[MAX_BUFFER_SIZE];  /*!< IQzigbee signal library version */
	char    BLUETOOTH_VERSION[MAX_BUFFER_SIZE];         /*!< Bluetooth library version */
	char    TEST_MANAGER_VERSION[MAX_BUFFER_SIZE];		/*!< Information of the TestManager DLL, such as version. */
	char	ERROR_MESSAGE[MAX_BUFFER_SIZE];				/*!< Error msg while connecting Tester. */
	char    IQTESTER_HARDWARE_VER[4][MAX_BUFFER_SIZE];  /*!< Testers HW version */
} l_11ACconnectTesterReturn;


#ifndef WIN32
int init11ACConnectTesterContainer = Initialize11ACConnectTesterContainers();
#endif

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

WIFI_11AC_TEST_API int WIFI_11AC_Connect_IQTester(void)
{
	int  err = ERR_OK;
	int  dummyValue = 0;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};

	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_11ACconnectTesterReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_11ACconnectTesterParamMap);
		return err;
	}
	else
	{
		// do nothing
	}

	/*-------------------------*
	* Respond to QUERY_RETURN *
	*-------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_RETURN", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryReturn(l_11ACconnectTesterReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}

	try
	{


		/*---------------------------------------*
		* g_WiFi_11ac_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		if( g_WiFi_11ac_Test_ID<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] WiFi_Test_ID not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] WiFi_Test_ID = %d.\n", g_WiFi_11ac_Test_ID);
		}

		TM_ClearReturns(g_WiFi_11ac_Test_ID);

		/*----------------------*
		* Get input parameters *
		*----------------------*/
		err = GetInputParameters(l_11ACconnectTesterParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get input parameters return OK.\n");
		}

		g_Tester_Reconnect = l_11ACconnectTesterParam.IQTESTER_RECONNECT;

		if( (0==g_Tester_Number) || g_Tester_Reconnect )
		{
			// Connect to IQ tester(s) if no connection, or reconnect every time

			// At least IP01 must be valid, otherwise return error directly.
			err = ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP01);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Invalid IP address (%s).\n", l_11ACconnectTesterParam.IQTESTER_IP01);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] IP address = (%s).\n", l_11ACconnectTesterParam.IQTESTER_IP01);
			}

			// To initialize the IQTester environment
			// modified for merge trunk 6/7/2012 buddy
			if (l_11ACconnectTesterParam.IQTESTER_TYPE == IQTYPE_2010)
			{
				g_iTesterName = IQTYPE_2010;
				LP_SetTesterName("IQView");
				err = LP_Init(IQTYPE_2010);
			}
			else
			{
				g_iTesterName = IQTYPE_XEL;
				LP_SetTesterName("IQxel");
				err = LP_Init(IQTYPE_XEL, l_11ACconnectTesterParam.IQTESTER_CONTROL_METHOD);
			}
			//err = LP_Init();
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] IQapi LP_Init() Failed.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] IQapi LP_Init() return OK.\n");
			}

			// Initialize ok, then valid other IP address
			if ( (ERR_OK!=ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP02))&&
				(ERR_OK!=ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP03))&&
				(ERR_OK!=ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP04)) )
			{
				// add by daixin to support dual-head test, 2012-12-10
				if(l_11ACconnectTesterParam.DH_ENABLE)
				{
					g_Tester_DualHead=1;
					printf("LP_DualHead_ConOpen, %d, %s\n",l_11ACconnectTesterParam.DH_TOKEN_ID,l_11ACconnectTesterParam.IQTESTER_IP01);
					err = LP_DualHead_ConOpen(l_11ACconnectTesterParam.DH_TOKEN_ID,l_11ACconnectTesterParam.IQTESTER_IP01);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_DualHead_ConOpen() Failed.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_DualHead_ConOpen() return OK.\n");
					}

					int iTokenID=-1;
					err = LP_DualHead_GetTokenID(&iTokenID);
					if ( ERR_OK!=err || iTokenID != l_11ACconnectTesterParam.DH_TOKEN_ID)
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_DualHead_GetTokenID() Failed.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_DualHead_GetTokenID() return OK.\n");
					}

				}
				// add end
				else
				{
					// IP 01: valid, IP 02: invalid, IP 03: invalid, IP 04: invalid
					err = LP_InitTester(l_11ACconnectTesterParam.IQTESTER_IP01);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Function LP_InitTester(%s) Failed.\n", l_11ACconnectTesterParam.IQTESTER_IP01);
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Function LP_InitTester(%s) return OK.\n", l_11ACconnectTesterParam.IQTESTER_IP01);
					}
				}
				g_Tester_Type   = IQ_View;
				g_Tester_Number = 1;
			}
			else if ( (ERR_OK==ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP02))&&
				(ERR_OK!=ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP03))&&
				(ERR_OK!=ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP04)) )
			{
				// IP 01: valid, IP 02: valid, IP 03: invalid, IP 04: invalid
				err = LP_InitTester2(l_11ACconnectTesterParam.IQTESTER_IP01, l_11ACconnectTesterParam.IQTESTER_IP02);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Function LP_InitTester2(%s, %s) Failed.\n", l_11ACconnectTesterParam.IQTESTER_IP01, l_11ACconnectTesterParam.IQTESTER_IP02);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Function LP_InitTester2(%s, %s) return OK.\n", l_11ACconnectTesterParam.IQTESTER_IP01, l_11ACconnectTesterParam.IQTESTER_IP02);
				}
				g_Tester_Type   = IQ_nxn;
				g_Tester_Number = 2;
			}
			else if ( (ERR_OK==ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP02))&&
				(ERR_OK==ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP03))&&
				(ERR_OK!=ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP04)) )
			{
				// IP 01: valid, IP 02: valid, IP 03: valid, IP 04: invalid
				err = LP_InitTester3(l_11ACconnectTesterParam.IQTESTER_IP01,
					l_11ACconnectTesterParam.IQTESTER_IP02,
					l_11ACconnectTesterParam.IQTESTER_IP03);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Function LP_InitTester3(%s, %s, %s) Failed.\n", l_11ACconnectTesterParam.IQTESTER_IP01, l_11ACconnectTesterParam.IQTESTER_IP02, l_11ACconnectTesterParam.IQTESTER_IP03);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Function LP_InitTester3(%s, %s, %s) return OK.\n", l_11ACconnectTesterParam.IQTESTER_IP01, l_11ACconnectTesterParam.IQTESTER_IP02, l_11ACconnectTesterParam.IQTESTER_IP03);
				}
				g_Tester_Type   = IQ_nxn;
				g_Tester_Number = 3;
			}
			else if ( (ERR_OK==ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP02))&&
				(ERR_OK==ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP03))&&
				(ERR_OK==ValidateIPAddress(l_11ACconnectTesterParam.IQTESTER_IP04)) )
			{
				// IP 01: valid, IP 02: valid, IP 03: valid, IP 04: valid
				err = LP_InitTester4(l_11ACconnectTesterParam.IQTESTER_IP01,
					l_11ACconnectTesterParam.IQTESTER_IP02,
					l_11ACconnectTesterParam.IQTESTER_IP03,
					l_11ACconnectTesterParam.IQTESTER_IP04);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Function LP_InitTester4(%s, %s, %s, %s) Failed.\n", l_11ACconnectTesterParam.IQTESTER_IP01, l_11ACconnectTesterParam.IQTESTER_IP02, l_11ACconnectTesterParam.IQTESTER_IP03, l_11ACconnectTesterParam.IQTESTER_IP04);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Function LP_InitTester4(%s, %s, %s, %s) return OK.\n", l_11ACconnectTesterParam.IQTESTER_IP01, l_11ACconnectTesterParam.IQTESTER_IP02, l_11ACconnectTesterParam.IQTESTER_IP03, l_11ACconnectTesterParam.IQTESTER_IP04);
				}
				g_Tester_Type   = IQ_nxn;
				g_Tester_Number = 4;
			}
			else
			{
				// Invalid IP address sequence
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Invalid IP address sequence for IQnxn.\n");
				throw logMessage;
			}

			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] IQtester initiation accomplished.\n");
		}
		else
		{
			// Reconnect=0
			err = ERR_OK;
		}

		if(l_11ACconnectTesterParam.DH_ENABLE)		
		{
			while(1)
			{
				err = LP_DualHead_ObtainControl(l_11ACconnectTesterParam.DH_PROBE_TIME_MS,l_11ACconnectTesterParam.DH_OBTAIN_CONTROL_TIMEOUT_MS);
				if ( ERR_OK!=err)
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_DualHead_ObtainControl() waiting.\n");
					//throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_DualHead_ObtainControl() return OK.\n");
					break;
				}
				Sleep(1000);
			}		
		}			
		// Call LP_SetVsa() for apply VSA and VSG port setting
		err = ::LP_SetVsa(  2412*1e6,
			0,
			g_WiFi11ACGlobalSettingParam.VSA_PORT,
			0,
			g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
			g_WiFi11ACGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000
			);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi_11AC] Fail to setup VSA, LP_SetVsa() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_SetVsa() return OK.\n");
		}

		/*----------------------------*
		* Disable VSG output signal  *
		*----------------------------*/
		// make sure no signal is generated by the VSG
		err = ::LP_EnableVsgRF(0);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
		}


		// For retrieval of the IQTester version info
		err = ::LP_GetVersion(l_11ACconnectTesterReturn.IQTESTER_INFO, MAX_BUFFER_SIZE);
		if (false==err)
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to get IQTester version information.\n");
			throw logMessage;
		}
		else
		{
			// Check if IQXS
			if ( strstr ( l_11ACconnectTesterReturn.IQTESTER_INFO, "SN: IQXS") || strstr ( l_11ACconnectTesterReturn.IQTESTER_INFO, "IQXEL") || strstr ( l_11ACconnectTesterReturn.IQTESTER_INFO, "DTNA"))
			{
				g_Tester_EnableVHT80 = TRUE;
			}
			else
			{
				g_Tester_EnableVHT80 = FALSE;
			}

			err = 0;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get IQTester version information return OK.\n");
		}
		//err = LP_DualHead_ReleaseControl();
		// Parse version string to break out the different
		map<string, string> versionMap;
		ParseSplitString(l_11ACconnectTesterReturn.IQTESTER_INFO, ":", versionMap);

		strcpy_s( l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_01, MAX_BUFFER_SIZE, versionMap["Tester 1 SN"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_02, MAX_BUFFER_SIZE, versionMap["Tester 2 SN"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_03, MAX_BUFFER_SIZE, versionMap["Tester 3 SN"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_04, MAX_BUFFER_SIZE, versionMap["Tester 4 SN"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQAPI_VERSION, MAX_BUFFER_SIZE, versionMap["iqapi"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQAPITC_VERSION, MAX_BUFFER_SIZE, versionMap["iqapitc"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQMEASURE_VERSION, MAX_BUFFER_SIZE, versionMap["IQmeasure"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQV_ANALYSIS_VERSION, MAX_BUFFER_SIZE, versionMap["iqv_analysis"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQV_MW_VERSION, MAX_BUFFER_SIZE, versionMap["IQV_mw"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQ_NXN_VERSION, MAX_BUFFER_SIZE, versionMap["iq_nxn"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQ_NXN_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_nxn_signal"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQ_MAX_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_max_signal"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQ_ZIGBEE_SIGNAL_VERSION, MAX_BUFFER_SIZE, versionMap["iq_zigbee_signal"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.BLUETOOTH_VERSION, MAX_BUFFER_SIZE, versionMap["bluetooth"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQTESTER_HARDWARE_VER[0], MAX_BUFFER_SIZE, versionMap["Tester 1 hardware version"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQTESTER_HARDWARE_VER[1], MAX_BUFFER_SIZE, versionMap["Tester 2 hardware version"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQTESTER_HARDWARE_VER[2], MAX_BUFFER_SIZE, versionMap["Tester 3 hardware version"].c_str() );
		strcpy_s( l_11ACconnectTesterReturn.IQTESTER_HARDWARE_VER[3], MAX_BUFFER_SIZE, versionMap["Tester 4 hardware version"].c_str() );


		//determine tester type by hardware version
		char *ptr;
		double hardwareVer = 0.0;

		ptr = strtok( l_11ACconnectTesterReturn.IQTESTER_HARDWARE_VER[0], ".");
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
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Function SetTesterHwVersion() Failed.\n");
						throw logMessage;
					}
					else
					{
						// do nothing /*LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Function SetTesterHwVersion() OK.\n");*/
					}
					//printf( " %f\n", hardwareVer);
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown hardware version format. Quit.\n");
					err=ERR_NO_CONNECTION;
					throw logMessage;
				}
			}
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown hardware version format. Quit.\n");
			err=ERR_NO_CONNECTION;
			throw logMessage;
		}


		//although WiFi technology does not support multiple testers, but old code enable it. For safety, add version check just in case.
		//Check whether testers FW/VSA/VSG version are consistent or not
		if(g_Tester_Number >1)
		{
			for (int testerIndex =1; testerIndex < g_Tester_Number; testerIndex++)
			{
				string tempStr = l_11ACconnectTesterReturn.IQTESTER_HARDWARE_VER[testerIndex];
				char tempChar [MAX_BUFFER_SIZE]={"0.3"};  //{"1.5"}; Change default value to 0.3 is the current Hardware version.
				sprintf_s(tempChar, MAX_BUFFER_SIZE,"%0.1f", hardwareVer);
				if(tempStr.find(tempChar)!= string::npos)
				{
					//do nothing
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Testers do not have same hardware versions. Quit.\n");
					err=ERR_NO_CONNECTION;
					throw logMessage;
				}
			}

			err = ::CheckTesterConsistentStatus(versionMap);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Versions of testers are not consistent.\n");
				err=ERR_NO_CONNECTION;
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC]  Versions of testers are consistent.\n");
			}
			// For retrieval of the TestManager version info
			err = ::TM_GetVersion(l_11ACconnectTesterReturn.TEST_MANAGER_VERSION, MAX_BUFFER_SIZE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to get TestManager version information.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get TestManager version information return OK.\n");
			}
			//if(l_11ACconnectTesterParam.DH_ENABLE)		
			//{
			//	printf("...%d...%d..\r\n",l_11ACconnectTesterParam.DH_PROBE_TIME_MS,l_11ACconnectTesterParam.DH_OBTAIN_CONTROL_TIMEOUT_MS);
			//	err = LP_DualHead_ObtainControl(l_11ACconnectTesterParam.DH_PROBE_TIME_MS,l_11ACconnectTesterParam.DH_OBTAIN_CONTROL_TIMEOUT_MS);
			//	if ( ERR_OK!=err)
			//	{
			//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_DualHead_ObtainControl() Failed.\n");
			//		throw logMessage;
			//	}
			//	else
			//	{
			//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_DualHead_ObtainControl() return OK.\n");
			//	}
			//	/*err = LP_DualHead_ReleaseControl();*/
			//}				
			/*-----------------------*
			*  Return Test Results  *
			*-----------------------*/
			if (ERR_OK==err)
			{
				sprintf_s(l_11ACconnectTesterReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
				ReturnTestResults(l_11ACconnectTesterReturnMap);
			}
			else
			{
				// do nothing
			}
		}
	}
	catch(char *msg)
	{
		g_WiFi_11ac_Test_ID = -1;
		ReturnErrorMessage(l_11ACconnectTesterReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		g_WiFi_11ac_Test_ID = -1;
		ReturnErrorMessage(l_11ACconnectTesterReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
		err = -1;
	}

	return err;
}

int Initialize11ACConnectTesterContainers(void)
{
	/*------------------*
	* Input Parameters: *
	* IQTESTER_IP01    *
	*------------------*/
	l_11ACconnectTesterParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	strcpy_s(l_11ACconnectTesterParam.IQTESTER_IP01, MAX_BUFFER_SIZE, "192.168.100.254");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterParam.IQTESTER_IP01))    // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterParam.IQTESTER_IP01;
		setting.unit        = "";
		setting.helpText    = "IP address of LitePoint tester";
		l_11ACconnectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_IP01", setting) );
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

	strcpy_s(l_11ACconnectTesterParam.IQTESTER_IP02, MAX_BUFFER_SIZE, "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterParam.IQTESTER_IP02))    // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterParam.IQTESTER_IP02;
		setting.unit        = "";
		setting.helpText    = "IP address of LitePoint tester";
		l_11ACconnectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_IP02", setting) );
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

	strcpy_s(l_11ACconnectTesterParam.IQTESTER_IP03, MAX_BUFFER_SIZE, "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterParam.IQTESTER_IP03))    // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterParam.IQTESTER_IP03;
		setting.unit        = "";
		setting.helpText    = "IP address of LitePoint tester";
		l_11ACconnectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_IP03", setting) );
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

	strcpy_s(l_11ACconnectTesterParam.IQTESTER_IP04, MAX_BUFFER_SIZE, "");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterParam.IQTESTER_IP04))    // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterParam.IQTESTER_IP04;
		setting.unit        = "";
		setting.helpText    = "IP address of LitePoint tester";
		l_11ACconnectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_IP04", setting) );
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

	l_11ACconnectTesterParam.IQTESTER_RECONNECT = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACconnectTesterParam.IQTESTER_RECONNECT))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACconnectTesterParam.IQTESTER_RECONNECT;
		setting.unit        = "";
		setting.helpText    = "A flag to control reconnect of IQ tester.\r\n1-Reconnect;0-No reconnect; Default=0";
		l_11ACconnectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_RECONNECT", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACconnectTesterParam.IQTESTER_TYPE = IQTYPE_XEL;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACconnectTesterParam.IQTESTER_TYPE))	// Type_Checking
	{
		setting.value		= (void*)&l_11ACconnectTesterParam.IQTESTER_TYPE;
		setting.unit		= "";
		setting.helpText	= "Tester type.0: non-IQXel;  1: IQXel.\r\n Default vaule: 0 ";
		l_11ACconnectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_TYPE", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACconnectTesterParam.IQTESTER_CONTROL_METHOD = TESTER_CONTROL_IQAPI;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACconnectTesterParam.IQTESTER_CONTROL_METHOD))	// Type_Checking
	{
		setting.value		= (void*)&l_11ACconnectTesterParam.IQTESTER_CONTROL_METHOD;
		setting.unit		= "";
		setting.helpText	= "Tester Control Method. 0: IQAPI; 1: SCPI.\r\n Default vaule: 0 ";
		l_11ACconnectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_CONTROL_METHOD", setting) );
	}
	else
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/*-------------------*
	* Input Parameters:  *
	* DH_ENABLE*
	*-------------------*/

	l_11ACconnectTesterParam.DH_ENABLE = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACconnectTesterParam.DH_ENABLE))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACconnectTesterParam.DH_ENABLE;
		setting.unit        = "";
		setting.helpText    = "A flag to control Dual Head test of IQ tester.\r\n1-Yes;0-No; Default=0";
		l_11ACconnectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DH_ENABLE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*-------------------*
	* Input Parameters:  *
	* DH_OBTAIN_CONTROL_TIMEOUT_MS*
	*-------------------*/

	l_11ACconnectTesterParam.DH_OBTAIN_CONTROL_TIMEOUT_MS = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACconnectTesterParam.DH_OBTAIN_CONTROL_TIMEOUT_MS))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACconnectTesterParam.DH_OBTAIN_CONTROL_TIMEOUT_MS;
		setting.unit        = "";
		setting.helpText    = "Timeout when obtain IQFlex control.\r\n0-INFINATE;non 0-wait the given timeout; Default=0";
		l_11ACconnectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DH_OBTAIN_CONTROL_TIMEOUT_MS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*-------------------*
	* Input Parameters:  *
	* DH_PROBE_TIME_MS*
	*-------------------*/

	l_11ACconnectTesterParam.DH_PROBE_TIME_MS = 100;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACconnectTesterParam.DH_PROBE_TIME_MS))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACconnectTesterParam.DH_PROBE_TIME_MS;
		setting.unit        = "";
		setting.helpText    = "Time interval when detect IQFlex control permit.\r\n Default=100 ms";
		l_11ACconnectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DH_PROBE_TIME_MS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	/*-------------------*
	* Input Parameters:  *
	* DH_TOKEN_ID*
	*-------------------*/

	l_11ACconnectTesterParam.DH_TOKEN_ID = 2;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACconnectTesterParam.DH_TOKEN_ID))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACconnectTesterParam.DH_TOKEN_ID;
		setting.unit        = "";
		setting.helpText    = "The testers token ID, two tester should has different ID.\r\n Default= 2";
		l_11ACconnectTesterParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DH_TOKEN_ID", setting) );
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
	l_11ACconnectTesterReturnMap.clear();

	l_11ACconnectTesterReturn.IQTESTER_INFO[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQTESTER_INFO))    // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQTESTER_INFO;
		setting.unit        = "";
		setting.helpText    = "IQTester info, such as HW version, firmware version, etc.";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_INFO", setting) );
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
	l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_01[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_01))  // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_01;
		setting.unit        = "";
		setting.helpText    = "Tester number 1 serial number";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_SERIAL_NUM_01", setting) );
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
	l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_02[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_02))  // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_02;
		setting.unit        = "";
		setting.helpText    = "Tester number 2 serial number";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_SERIAL_NUM_02", setting) );
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

	l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_03[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_03))  // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_03;
		setting.unit        = "";
		setting.helpText    = "Tester number 3 serial number";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_SERIAL_NUM_03", setting) );
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
	l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_04[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_04))  // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQTESTER_SERIAL_NUM_04;
		setting.unit        = "";
		setting.helpText    = "Tester number 4 serial number";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQTESTER_SERIAL_NUM_04", setting) );
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

	l_11ACconnectTesterReturn.IQAPI_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQAPI_VERSION))   // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQAPI_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQapi version";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQAPI_VERSION", setting) );
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

	l_11ACconnectTesterReturn.IQAPITC_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQAPITC_VERSION)) // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQAPITC_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQapitc library version";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQAPITC_VERSION", setting) );
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
	l_11ACconnectTesterReturn.IQMEASURE_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQMEASURE_VERSION))   // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQMEASURE_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQmeasure library version";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQMEASURE_VERSION", setting) );
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
	l_11ACconnectTesterReturn.IQV_ANALYSIS_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQV_ANALYSIS_VERSION))    // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQV_ANALYSIS_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQv_analysis library version";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQV_ANALYSIS_VERSION", setting) );
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
	l_11ACconnectTesterReturn.IQV_MW_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQV_MW_VERSION))  // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQV_MW_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQv middle wear version";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQV_MW_VERSION", setting) );
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
	l_11ACconnectTesterReturn.IQ_NXN_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQ_NXN_VERSION))  // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQ_NXN_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQnxn version";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQ_NXN_VERSION", setting) );
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
	l_11ACconnectTesterReturn.IQ_NXN_SIGNAL_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQ_NXN_SIGNAL_VERSION))   // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQ_NXN_SIGNAL_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQnxn signal library version";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQ_NXN_SIGNAL_VERSION", setting) );
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
	l_11ACconnectTesterReturn.IQ_MAX_SIGNAL_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQ_MAX_SIGNAL_VERSION))   // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQ_MAX_SIGNAL_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQmax signal library version";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQ_MAX_SIGNAL_VERSION", setting) );
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
	l_11ACconnectTesterReturn.IQ_ZIGBEE_SIGNAL_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.IQ_ZIGBEE_SIGNAL_VERSION))    // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.IQ_ZIGBEE_SIGNAL_VERSION;
		setting.unit        = "";
		setting.helpText    = "IQzigbee signal library version";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("IQ_ZIGBEE_SIGNAL_VERSION", setting) );
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
	l_11ACconnectTesterReturn.BLUETOOTH_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.BLUETOOTH_VERSION))   // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.BLUETOOTH_VERSION;
		setting.unit        = "";
		setting.helpText    = "Bluetooth library version";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("BLUETOOTH_VERSION", setting) );
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
	l_11ACconnectTesterReturn.TEST_MANAGER_VERSION[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.TEST_MANAGER_VERSION))    // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.TEST_MANAGER_VERSION;
		setting.unit        = "";
		setting.helpText    = "Information of the TestManager DLL, such as version.";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TEST_MANAGER_VERSION", setting) );
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
	l_11ACconnectTesterReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACconnectTesterReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_11ACconnectTesterReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_11ACconnectTesterReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
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
			// Assume it is a valid IP address, don't need to check the IP format here.
			// Because the LP_InitTester function will take care the error return.
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


#include "stdafx.h"
#include "TestManager.h"
#include "FM_Test.h"
#include "IQlite_Logger.h"
#include "FM_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include "math.h"


using namespace std;

// global variable 
TM_ID            g_FM_Test_ID = -1;
vDUT_ID          g_FM_Dut     = -1;
bool             g_ReloadDutDll = false;

int				 g_Logger_ID        = -1;
int              g_FM_Test_timer    = -1;
int				 g_Tester_Type      = IQ_View;
int				 g_Tester_Number    = 0;
int				 g_Tester_Reconnect = 0;

const char      *g_FM_Test_Version = "1.6.0 (2010-09-17)\n";
char			 g_defaultFilePath[MAX_BUFFER_SIZE] = {'\0'};

// This global variable is declared in FM_Global_Setting.cpp
// Input Parameter Container
extern map<string, FM_SETTING_STRUCT> g_FMGlobalSettingParamMap;
extern FM_GLOBAL_SETTING g_FMGlobalSettingParam;

// end global


//! FM Test Internal Functions
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
    InitializeConnectTesterContainers();            // Needed by FM_Connect_IQTester
    InitializeDisconnectTesterContainers();         // Needed by FM_Disconnect_IQTester
    InitializeInsertDutContainers();                // Needed by FM_Insert_Dut
    InitializeInitializeDutContainers();            // Needed by FM_Initialize_Dut
    InitializeRemoveDutContainers();                // Needed by FM_Remove_Dut
	InitializeTXVerify_RF_MEASUREMENTS_Containers();	// Needed by FM_TX_Verify_RF_MEASUREMENTS
	InitializeTXVerify_AUDIO_Containers();              // Needed by FM_TX_Verify_AUDIO
	
	InitializeRXVerify_RSSI_Containers();				 // Needed by FM_RX_Verify_RSSI
	InitializeRXVerify_AUDIO_Containers();              // Needed by FM_RX_Verify_AUDIO
	InitializeRXVerify_RDS_Containers();                // Needed by FM_RX_Verify_RDS

	InitializeGlobalSettingContainers();            // Needed by FM_Global_Setting
    InitializeloadPathLossTableContainers();
	InitializeRunExternalProgramContainers();
}


//int  GetWaveformFileName(char* perfix, char* postfix, char* packetType, char* waveformFileName, int bufferSize)
//{
//    int  err = ERR_OK;
//
//	char keyword[MAX_BUFFER_SIZE] = {'\0'};
//
//	map<string, FM_SETTING_STRUCT>::iterator inputMap_Iter;
//
//	// Find out the keyword of the waveform file name
//	sprintf_s(keyword, MAX_BUFFER_SIZE, "%s_%s_%s", perfix, packetType, postfix);			
//
//    // Searching the keyword in g_FMGlobalSettingParamMap
//	inputMap_Iter = g_FMGlobalSettingParamMap.find(keyword);
//    if( inputMap_Iter!=g_FMGlobalSettingParamMap.end() )
//    {
//		char dummyString[MAX_BUFFER_SIZE];
//		strcpy_s (dummyString, MAX_BUFFER_SIZE, (char*)inputMap_Iter->second.value);
//
//		if ( strlen(g_defaultFilePath)>0 )		// default waveform file path not empty
//		{
//			sprintf_s(waveformFileName, bufferSize, "%s/%s", g_defaultFilePath, dummyString);
//		}
//		else	// if default waveform file path is empty, then use g_FMGlobalSettingParam.PER_WAVEFORM_PATH
//		{
//			sprintf_s(waveformFileName, bufferSize, "%s/%s", g_FMGlobalSettingParam.BER_WAVEFORM_PATH, dummyString);
//		}		
//
//		// Wave file checking
//		FILE *waveFile;
//		fopen_s(&waveFile, waveformFileName, "r");
//		if (!waveFile)
//		{
//			if ( strlen(g_defaultFilePath)>0 )		// Default waveform file path is not empty, but still can't find the waveformfile, then return error.
//			{
//				err = TM_ERR_PARAM_DOES_NOT_EXIST;
//			}
//			else	// Try to find the file in higher-level folders
//			{
//				char tempPath[MAX_BUFFER_SIZE] = {'\0'};
//				sprintf_s(tempPath, bufferSize, "../%s", waveformFileName);
//
//				fopen_s(&waveFile, tempPath, "r");
//				if (!waveFile)
//				{
//					err = TM_ERR_PARAM_DOES_NOT_EXIST;
//				}
//				else
//				{
//					// Save the path to default path name
//					sprintf_s(g_defaultFilePath, bufferSize, "../%s", g_FMGlobalSettingParam.BER_WAVEFORM_PATH);
//					// And use the new path
//					sprintf_s(waveformFileName, bufferSize, "%s", tempPath);
//
//					fclose(waveFile);
//					waveFile = NULL;
//				}
//			}
//		}
//		else
//		{
//			fclose(waveFile);
//			waveFile = NULL;
//		}
//	}
//    else
//    {
//        err = TM_ERR_PARAM_DOES_NOT_EXIST;
//    }
//
//	return err;
//}
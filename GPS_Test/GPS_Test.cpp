// GPS_Test.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "vDUT.h"
#include "TestManager.h"
#include "GPS_Test.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"
#include "GPS_Test_Internal.h"
#include "IQmeasure.h"

// This global variable is declared in GPS_Test_Internal.cpp
extern TM_ID        g_GPS_Test_ID;
extern vDUT_ID      g_GPS_Dut;
extern int			g_Logger_ID;
extern int          g_GPS_Test_timer;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        InitializeAllGPSContainers();
        break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		CleanupAllGPSContainers();
        break;
    }

    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


GPS_TEST_API int GPS_technologyRegister()
{
    /*---------------------*
     * Create the timer ID *
     *---------------------*/
    if( -1==g_GPS_Test_timer )
    {
        TIMER_CreateTimer("GPS_Test", &g_GPS_Test_timer);
    }
    else
    {
        // Timer ID is already created; no need to do it again
    }

    TM_ID technologyID=-1;
    TM_RegisterTechnology("GPS", &technologyID);
    if( technologyID>=0 )
    {  
        TM_InstallCallbackFunction(technologyID, "GLOBAL_SETTINGS",      GPS_Global_Setting);
        TM_InstallCallbackFunction(technologyID, "CONNECT_IQ_TESTER",    GPS_Connect_IQTester);
        TM_InstallCallbackFunction(technologyID, "DISCONNECT_IQ_TESTER", GPS_Disconnect_IQTester);
        TM_InstallCallbackFunction(technologyID, "TRANSMIT_SCENARIO",    GPS_Transmit_Scenario);
        TM_InstallCallbackFunction(technologyID, "MODULATED_MODE",       GPS_ModulatedMode);
        TM_InstallCallbackFunction(technologyID, "CONTINUE_WAVE",        GPS_ContinueWave);
        TM_InstallCallbackFunction(technologyID, "INSERT_DUT",           GPS_Insert_Dut);
        TM_InstallCallbackFunction(technologyID, "INITIALIZE_DUT",       GPS_Initialize_Dut);
        TM_InstallCallbackFunction(technologyID, "REMOVE_DUT",           GPS_Remove_Dut);
		TM_InstallCallbackFunction(technologyID, "LOAD_PATH_LOSS_TABLE", GPS_LoadPathLossTable);

        g_GPS_Test_ID = technologyID;
		
		/*----------------------*
		 * Create the logger ID *
		 *----------------------*/
		if( -1==g_Logger_ID )
		{
			LOGGER_CreateLogger("GPS_Test_Log", &g_Logger_ID);
			// TODO: if possible, must do the error handling here
		}
		else
		{
			// Logger ID created already. No need to do it again
		}
    }
	else
	{
		// do nothing
	}

    return technologyID;
}

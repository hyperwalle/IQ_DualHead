// BT_Test.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "vDUT.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"

// This global variable is declared in BT_Test_Internal.cpp
extern TM_ID        g_BT_Test_ID;
extern vDUT_ID      g_BT_Dut;
extern int			g_Logger_ID;
extern int          g_BT_Test_timer;

#ifdef _MANAGED
#pragma managed(push, off)
#endif


#ifdef WIN32
BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		InitializeAllContainers();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		CleanupAllBTContainers();
		break;
	}
	return TRUE;
}
#else

static int init_dll =  InitializeAllContainers();

#endif


#ifdef _MANAGED
#pragma managed(pop)
#endif


BT_TEST_API int BT_technologyRegister()
{
    /*---------------------*
     * Create the timer ID *
     *---------------------*/
    if( -1==g_BT_Test_timer )
    {
        TIMER_CreateTimer("BT_Test", &g_BT_Test_timer);
    }
    else
    {
        // Timer ID is already created; no need to do it again
    }

    TM_ID technologyID=-1;
    TM_RegisterTechnology("BT", &technologyID);
    if( technologyID>=0 )
    {  
        TM_InstallCallbackFunction(technologyID, "GLOBAL_SETTINGS",      BT_Global_Setting);
        TM_InstallCallbackFunction(technologyID, "CONNECT_IQ_TESTER",    BT_Connect_IQTester);
        TM_InstallCallbackFunction(technologyID, "DISCONNECT_IQ_TESTER", BT_Disconnect_IQTester);
        TM_InstallCallbackFunction(technologyID, "INSERT_DUT",           BT_Insert_Dut);
        TM_InstallCallbackFunction(technologyID, "INITIALIZE_DUT",       BT_Initialize_Dut);
        TM_InstallCallbackFunction(technologyID, "REMOVE_DUT",           BT_Remove_Dut);
        TM_InstallCallbackFunction(technologyID, "RESET_DUT",            BT_Reset_Dut); 

        TM_InstallCallbackFunction(technologyID, "TX_BDR",				 BT_TX_Verify_BDR);
        TM_InstallCallbackFunction(technologyID, "TX_EDR",				 BT_TX_Verify_EDR);
        TM_InstallCallbackFunction(technologyID, "TX_LE",				 BT_TX_Verify_LE);
        TM_InstallCallbackFunction(technologyID, "RX_BDR",				 BT_RX_Verify_BDR);
        TM_InstallCallbackFunction(technologyID, "RX_EDR",				 BT_RX_Verify_EDR);
        TM_InstallCallbackFunction(technologyID, "RX_LE",				 BT_RX_Verify_LE);
		TM_InstallCallbackFunction(technologyID, "TX_POWER_CONTROL",	 BT_TX_Power_Control);
		TM_InstallCallbackFunction(technologyID, "TX_CALIBRATION",       BT_TX_Calibration); 

		TM_InstallCallbackFunction(technologyID, "WRITE_BD_ADDRESS",	 BT_Write_BD_Address);
		TM_InstallCallbackFunction(technologyID, "READ_BD_ADDRESS",	     BT_Read_BD_Address);
		TM_InstallCallbackFunction(technologyID, "WRITE_DUT_INFO",	     BT_Write_DUT_Info);

		//WRITE_DUT_INFO

		TM_InstallCallbackFunction(technologyID, "LOAD_PATH_LOSS_TABLE", BT_LoadPathLossTable);
		TM_InstallCallbackFunction(technologyID, "RUN_EXTERNAL_PROGRAM", BT_Run_External_Program);
		TM_InstallCallbackFunction(technologyID, "POWER_MODE_DUT",       BT_Power_Mode_Dut);
		TM_InstallCallbackFunction(technologyID, "BTLE_INITIALIZE",      BT_LE_Initialize);

        g_BT_Test_ID = technologyID;
		
		/*----------------------*
		 * Create the logger ID *
		 *----------------------*/
		if( -1==g_Logger_ID )
		{
			LOGGER_CreateLogger("BT_Test_Log", &g_Logger_ID);
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

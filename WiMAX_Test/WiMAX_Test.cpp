// WIMAX_Test.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "WIMAX_Test.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"
#include "WIMAX_Test_Internal.h"
#include "IQmeasure.h"

// This global variable is declared in WIMAX_Test_Internal.cpp
extern TM_ID        g_WIMAX_Test_ID;
extern vDUT_ID      g_WIMAX_Dut;
extern int			g_Logger_ID;
extern int          g_WIMAX_Test_timer;

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
        InitializeAllContainers();
        break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		CleanupAllWiMaxContainers();
        break;
    }
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


WIMAX_TEST_API int WIMAX_technologyRegister()
{
    /*---------------------*
     * Create the timer ID *
     *---------------------*/
    if( -1==g_WIMAX_Test_timer )
    {
        ::TIMER_CreateTimer("WIMAX_Test", &g_WIMAX_Test_timer);
    }
    else
    {
        // Timer ID created already. No need to do it again
    }

    TM_ID technologyID=-1;
    TM_RegisterTechnology("WIMAX", &technologyID);
    if( technologyID>=0 )
    {          
        TM_InstallCallbackFunction(technologyID, "CONNECT_IQ_TESTER",    WIMAX_Connect_IQTester);
        TM_InstallCallbackFunction(technologyID, "DISCONNECT_IQ_TESTER", WIMAX_Disconnect_IQTester);
	
		TM_InstallCallbackFunction(technologyID, "GLOBAL_SETTINGS",      WIMAX_Global_Setting);
		TM_InstallCallbackFunction(technologyID, "LOAD_PATH_LOSS_TABLE", WIMAX_LoadPathLossTable);

        TM_InstallCallbackFunction(technologyID, "INSERT_DUT",           WIMAX_Insert_Dut);
        TM_InstallCallbackFunction(technologyID, "INITIALIZE_DUT",       WIMAX_Initialize_Dut);
        TM_InstallCallbackFunction(technologyID, "REMOVE_DUT",           WIMAX_Remove_Dut);
		TM_InstallCallbackFunction(technologyID, "RESET_DUT",			 WIMAX_Reset_Dut); 

		TM_InstallCallbackFunction(technologyID, "READ_EEPROM",          WIMAX_Read_Eeprom);
        TM_InstallCallbackFunction(technologyID, "WRITE_EEPROM",         WIMAX_Write_Eeprom);	
        TM_InstallCallbackFunction(technologyID, "READ_MAC_ADDRESS",     WIMAX_Read_Mac_Address);
        TM_InstallCallbackFunction(technologyID, "WRITE_MAC_ADDRESS",    WIMAX_Write_Mac_Address);


		TM_InstallCallbackFunction(technologyID, "TXRX_CALIBRATION",     WIMAX_TXRX_Calibration);		
		TM_InstallCallbackFunction(technologyID, "TXRX_VERIFICATION",    WIMAX_TX_RX_Verification);
		TM_InstallCallbackFunction(technologyID, "TX_VERIFY_EVM",        WIMAX_TX_Verify_Evm);
		TM_InstallCallbackFunction(technologyID, "TX_VERIFY_MASK",       WIMAX_TX_Verify_Mask);
		TM_InstallCallbackFunction(technologyID, "TX_VERIFY_POWER_STEP", WIMAX_TX_Verify_Power_Step);
		TM_InstallCallbackFunction(technologyID, "RX_VERIFY_PER",        WIMAX_RX_Verify_Per);
			
        g_WIMAX_Test_ID = technologyID;
		
		/*----------------------*
		 * Create the logger ID *
		 *----------------------*/
		if( -1==g_Logger_ID )
		{
			::LOGGER_CreateLogger("WIMAX_Test_Log", &g_Logger_ID);
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

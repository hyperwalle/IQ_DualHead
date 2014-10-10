// WiFi_Test.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "WiFi_11ac_MiMo_Test.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"
#include "WiFi_11ac_MiMo_Test_Internal.h"
#include "IQmeasure.h"




// This global variable is declared in WiFi_Test_Internal.cpp
extern TM_ID        g_WiFi_Test_ID;
extern vDUT_ID      g_WiFi_Dut;
extern int			g_Logger_ID;
extern int          g_WiFi_Test_timer;
extern int          g_WiFi_Test_logger;

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
        InitializeAll11acMIMOContainers();
        break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		CleanupAll11acMIMOContainers();
        break;
    }
    return TRUE;
}
#else
namespace {
	struct WiFiMIMOTestInit
	{
		WiFiMIMOTestInit() { InitializeAllMIMOContainers(); }
		~WiFiMIMOTestInit() { CleanupAllMIMOContainers(); } 
	} mimoTestInit;
}
#endif


#ifdef _MANAGED
#pragma managed(pop)
#endif


WIFI_11AC_MIMO_TEST_API int WIFI_11AC_MIMO_technologyRegister()
{
    /*---------------------*
     * Create the timer ID *
     *---------------------*/
    if( -1==g_WiFi_Test_timer )
    {
        ::TIMER_CreateTimer("WiFi_11ac_MiMo_Test", &g_WiFi_Test_timer);
    }
    else
    {
        // Timer ID created already. No need to do it again
    }

    /*---------------------*
     * Create the logger ID *
     *---------------------*/
    if( -1==g_WiFi_Test_logger )
    {
        ::LOGGER_CreateLogger("WiFi_11ac_MiMo_Test", &g_WiFi_Test_logger);
    }
    else
    {
        // Logger ID created already. No need to do it again
    }

    TM_ID technologyID=-1;
    TM_RegisterTechnology("WIFI_11AC_MIMO", &technologyID);
    if( technologyID>=0 )
    {  
        TM_InstallCallbackFunction(technologyID, "GLOBAL_SETTINGS",      WiFi_Global_Setting);
        TM_InstallCallbackFunction(technologyID, "CONNECT_IQ_TESTER",    WiFi_Connect_IQTester);
        TM_InstallCallbackFunction(technologyID, "DISCONNECT_IQ_TESTER", WiFi_Disconnect_IQTester);
        TM_InstallCallbackFunction(technologyID, "INSERT_DUT",           WiFi_Insert_Dut);
        TM_InstallCallbackFunction(technologyID, "INITIALIZE_DUT",       WiFi_Initialize_Dut);
        TM_InstallCallbackFunction(technologyID, "REMOVE_DUT",           WiFi_Remove_Dut);
		TM_InstallCallbackFunction(technologyID, "RUN_DUT_COMMAND",      WiFi_Run_DUT_Command);
        TM_InstallCallbackFunction(technologyID, "TX_VERIFY_EVM",        WiFi_11ac_MiMo_TX_Verify_Evm);
        TM_InstallCallbackFunction(technologyID, "TX_VERIFY_MASK",       WiFi_TX_Verify_Mask);
        TM_InstallCallbackFunction(technologyID, "RX_VERIFY_PER",        WiFi_RX_Verify_Per);
        TM_InstallCallbackFunction(technologyID, "TX_VERIFY_POWER",      WiFi_TX_Verify_Power);
        TM_InstallCallbackFunction(technologyID, "TX_VERIFY_SPECTRUM",   WiFi_TX_Verify_Spectrum);

        TM_InstallCallbackFunction(technologyID, "TX_CALIBRATION",       WiFi_TX_Calibration);        
        //TM_InstallCallbackFunction(technologyID, "RX_RSSI_CAL",          WiFi_Rssi_Cal);
        //TM_InstallCallbackFunction(technologyID, "RX_VERIFY_PER_MAX",    WiFi_RX_Verify_Per_Max);
        //TM_InstallCallbackFunction(technologyID, "RX_SWEEP_PER",         WiFi_Sweep_Per);
        
		TM_InstallCallbackFunction(technologyID, "GET_SERIAL_NUMBER",    WiFi_Get_Serial_Number);     
        
		TM_InstallCallbackFunction(technologyID, "READ_EEPROM",          WiFi_Read_Eeprom);
        TM_InstallCallbackFunction(technologyID, "WRITE_EEPROM",         WiFi_Write_Eeprom);	

        TM_InstallCallbackFunction(technologyID, "READ_MAC_ADDRESS",     WiFi_Read_Mac_Address);
        TM_InstallCallbackFunction(technologyID, "WRITE_MAC_ADDRESS",    WiFi_Write_Mac_Address);

		TM_InstallCallbackFunction(technologyID, "WRITE_BB_REGISTER",    WiFi_Write_BB_Register);
		TM_InstallCallbackFunction(technologyID, "READ_BB_REGISTER",     WiFi_Read_BB_Register);

		TM_InstallCallbackFunction(technologyID, "WRITE_RF_REGISTER",    WiFi_Write_RF_Register);
		TM_InstallCallbackFunction(technologyID, "READ_RF_REGISTER",     WiFi_Read_RF_Register);

		TM_InstallCallbackFunction(technologyID, "WRITE_MAC_REGISTER",   WiFi_Write_Mac_Register);
		TM_InstallCallbackFunction(technologyID, "READ_MAC_REGISTER",    WiFi_Read_Mac_Register);

		TM_InstallCallbackFunction(technologyID, "WRITE_SOC_REGISTER",   WiFi_Write_Soc_Register);
		TM_InstallCallbackFunction(technologyID, "READ_SOC_REGISTER",    WiFi_Read_Soc_Register);

        TM_InstallCallbackFunction(technologyID, "RUN_EXTERNAL_PROGRAM", WiFi_Run_External_Program);
		TM_InstallCallbackFunction(technologyID, "POWER_MODE_DUT",       WiFi_Power_Mode_Dut);

		TM_InstallCallbackFunction(technologyID, "LOAD_PATH_LOSS_TABLE", WiFi_LoadPathLossTable);
        
        g_WiFi_Test_ID = technologyID;
		
		/*----------------------*
		 * Create the logger ID *
		 *----------------------*/
		if( -1==g_Logger_ID )
		{
			::LOGGER_CreateLogger("WiFi_11ac_Test_Log", &g_Logger_ID);
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

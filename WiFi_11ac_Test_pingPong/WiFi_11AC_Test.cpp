// WiFi_Test.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "WiFi_11AC_Test.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"

// This global variable is declared in WiFi_Test_Internal.cpp
extern TM_ID        g_WiFi_11ac_Test_ID;
extern vDUT_ID      g_WiFi_11ac_Dut;
extern int			g_WiFi_11AC_Logger_ID;
extern int          g_WiFi_11AC_Test_timer;
extern int          g_WiFi_11AC_Test_logger;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

// dll startup code

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
        break;
    }
    return TRUE;
}
#else

  static int init_WiFi_11AC_dll =  InitializeAllContainers();

#endif

 
#ifdef _MANAGED
#pragma managed(pop)
#endif


WIFI_11AC_TEST_API int WIFI_11AC_technologyRegister()
{
    /*---------------------*	
     * Create the timer ID *
     *---------------------*/
    if( -1== g_WiFi_11AC_Test_timer )
    {
        ::TIMER_CreateTimer("WiFi_11AC_Test", &g_WiFi_11AC_Test_timer);
    }
    else
    {
        // Timer ID created already. No need to do it again
    }

    TM_ID technologyID=-1;
    TM_RegisterTechnology("WIFI_11AC", &technologyID);
    if( technologyID>=0 )
    {  
        TM_InstallCallbackFunction(technologyID, "GLOBAL_SETTINGS",      WIFI_11AC_Global_Setting);
        TM_InstallCallbackFunction(technologyID, "CONNECT_IQ_TESTER",    WIFI_11AC_Connect_IQTester);
        TM_InstallCallbackFunction(technologyID, "DISCONNECT_IQ_TESTER", WIFI_11AC_Disconnect_IQTester);
        TM_InstallCallbackFunction(technologyID, "INSERT_DUT",           WIFI_11AC_Insert_Dut);
        TM_InstallCallbackFunction(technologyID, "INITIALIZE_DUT",       WIFI_11AC_Initialize_Dut);
        TM_InstallCallbackFunction(technologyID, "REMOVE_DUT",           WIFI_11AC_Remove_Dut);
		TM_InstallCallbackFunction(technologyID, "RUN_DUT_COMMAND",      WIFI_11AC_Run_DUT_Command);

		TM_InstallCallbackFunction(technologyID, "TX_MULTI_VERIFICATION",	WIFI_11AC_TX_Multi_Verification);
        TM_InstallCallbackFunction(technologyID, "TX_VERIFY_EVM",        WIFI_11AC_TX_Verify_Evm);
        TM_InstallCallbackFunction(technologyID, "TX_VERIFY_MASK",       WIFI_11AC_TX_Verify_Mask);
        TM_InstallCallbackFunction(technologyID, "RX_VERIFY_PER",        WIFI_11AC_RX_Verify_Per);
        TM_InstallCallbackFunction(technologyID, "TX_VERIFY_POWER",      WIFI_11AC_TX_Verify_Power);
        TM_InstallCallbackFunction(technologyID, "TX_VERIFY_SPECTRUM",   WIFI_11AC_TX_Verify_Spectrum);

        TM_InstallCallbackFunction(technologyID, "TX_CALIBRATION",       WIFI_11AC_TX_Calibration);        
        
		TM_InstallCallbackFunction(technologyID, "GET_SERIAL_NUMBER",    WIFI_11AC_Get_Serial_Number);     
        
		TM_InstallCallbackFunction(technologyID, "READ_EEPROM",          WIFI_11AC_Read_Eeprom);
        TM_InstallCallbackFunction(technologyID, "WRITE_EEPROM",         WIFI_11AC_Write_Eeprom);	

        TM_InstallCallbackFunction(technologyID, "READ_MAC_ADDRESS",     WIFI_11AC_Read_Mac_Address);
        TM_InstallCallbackFunction(technologyID, "WRITE_MAC_ADDRESS",    WIFI_11AC_Write_Mac_Address);

		TM_InstallCallbackFunction(technologyID, "WRITE_BB_REGISTER",    WIFI_11AC_Write_BB_Register);
		TM_InstallCallbackFunction(technologyID, "READ_BB_REGISTER",     WIFI_11AC_Read_BB_Register);

		TM_InstallCallbackFunction(technologyID, "WRITE_RF_REGISTER",    WIFI_11AC_Write_RF_Register);
		TM_InstallCallbackFunction(technologyID, "READ_RF_REGISTER",     WIFI_11AC_Read_RF_Register);

		TM_InstallCallbackFunction(technologyID, "WRITE_MAC_REGISTER",   WIFI_11AC_Write_Mac_Register);
		TM_InstallCallbackFunction(technologyID, "READ_MAC_REGISTER",    WIFI_11AC_Read_Mac_Register);

		TM_InstallCallbackFunction(technologyID, "WRITE_SOC_REGISTER",   WIFI_11AC_Write_Soc_Register);
		TM_InstallCallbackFunction(technologyID, "READ_SOC_REGISTER",    WIFI_11AC_Read_Soc_Register);

        TM_InstallCallbackFunction(technologyID, "RUN_EXTERNAL_PROGRAM", WIFI_11AC_Run_External_Program);
		TM_InstallCallbackFunction(technologyID, "POWER_MODE_DUT",       WIFI_11AC_Power_Mode_Dut);

		TM_InstallCallbackFunction(technologyID, "LOAD_PATH_LOSS_TABLE", WIFI_11AC_LoadPathLossTable);
		
		TM_InstallCallbackFunction(technologyID, "TEMPLATE_FUNCTION",    WIFI_11AC_Template_Function);
        
        g_WiFi_11ac_Test_ID = technologyID;
		
		/*----------------------*
		 * Create the logger ID *
		 *----------------------*/
		if( -1==g_WiFi_11AC_Logger_ID )
		{
			::LOGGER_CreateLogger("WiFi_11AC_Test_Log", &g_WiFi_11AC_Logger_ID);
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

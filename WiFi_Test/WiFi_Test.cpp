// WiFi_Test.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "WiFi_Test.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"


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
		CleanupAllWiFiContainers();
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


WIFI_TEST_API int WIFI_technologyRegister()
{
    /*---------------------*
     * Create the timer ID *
     *---------------------*/
    if( -1==g_WiFi_Test_timer )
    {
        ::TIMER_CreateTimer("WiFi_Test", &g_WiFi_Test_timer);
    }
    else
    {
        // Timer ID created already. No need to do it again
    }

    TM_ID technologyID=-1;
    TM_RegisterTechnology("WIFI", &technologyID);
    if( technologyID>=0 )
    {  
        TM_InstallCallbackFunction(technologyID, "GLOBAL_SETTINGS",			WiFi_Global_Setting);
        TM_InstallCallbackFunction(technologyID, "CONNECT_IQ_TESTER",		WiFi_Connect_IQTester);
        TM_InstallCallbackFunction(technologyID, "DISCONNECT_IQ_TESTER",	WiFi_Disconnect_IQTester);
        TM_InstallCallbackFunction(technologyID, "INSERT_DUT",				WiFi_Insert_Dut);
        TM_InstallCallbackFunction(technologyID, "INITIALIZE_DUT",			WiFi_Initialize_Dut);
        TM_InstallCallbackFunction(technologyID, "REMOVE_DUT",				WiFi_Remove_Dut);
		TM_InstallCallbackFunction(technologyID, "RUN_DUT_COMMAND",      	WiFi_Run_DUT_Command); // -cfy@sunnyvale, 2012/3/13-

        TM_InstallCallbackFunction(technologyID, "TX_MULTI_VERIFICATION",	WiFi_TX_Multi_Verification);
		TM_InstallCallbackFunction(technologyID, "TX_VERIFY_EVM",			WiFi_TX_Verify_Evm);
        TM_InstallCallbackFunction(technologyID, "TX_VERIFY_MASK",			WiFi_TX_Verify_Mask);
        TM_InstallCallbackFunction(technologyID, "RX_VERIFY_PER",			WiFi_RX_Verify_Per);
		TM_InstallCallbackFunction(technologyID, "RX_SWEEP_PER",			WiFi_RX_Sweep_Per);
        TM_InstallCallbackFunction(technologyID, "TX_VERIFY_POWER",			WiFi_TX_Verify_Power);
        TM_InstallCallbackFunction(technologyID, "TX_VERIFY_SPECTRUM",		WiFi_TX_Verify_Spectrum);
        
		TM_InstallCallbackFunction(technologyID, "TX_CALIBRATION",			WiFi_TX_Calibration);  
		TM_InstallCallbackFunction(technologyID, "RX_CALIBRATION",			WiFi_RX_Calibration);  

		TM_InstallCallbackFunction(technologyID, "GET_SERIAL_NUMBER",		WiFi_Get_Serial_Number);  
    	TM_InstallCallbackFunction(technologyID, "READ_EEPROM",				WiFi_Read_Eeprom);
        TM_InstallCallbackFunction(technologyID, "WRITE_EEPROM",			WiFi_Write_Eeprom);	
		TM_InstallCallbackFunction(technologyID, "FINALIZE_EEPROM",			WiFi_Finalize_Eeprom);	

        TM_InstallCallbackFunction(technologyID, "READ_MAC_ADDRESS",		WiFi_Read_Mac_Address);
        TM_InstallCallbackFunction(technologyID, "WRITE_MAC_ADDRESS",		WiFi_Write_Mac_Address);
		TM_InstallCallbackFunction(technologyID, "WRITE_BB_REGISTER",		WiFi_Write_BB_Register);
		TM_InstallCallbackFunction(technologyID, "READ_BB_REGISTER",		WiFi_Read_BB_Register);
		TM_InstallCallbackFunction(technologyID, "WRITE_RF_REGISTER",		WiFi_Write_RF_Register);
		TM_InstallCallbackFunction(technologyID, "READ_RF_REGISTER",		WiFi_Read_RF_Register);
		TM_InstallCallbackFunction(technologyID, "WRITE_MAC_REGISTER",		WiFi_Write_Mac_Register);
		TM_InstallCallbackFunction(technologyID, "READ_MAC_REGISTER",		WiFi_Read_Mac_Register);
		TM_InstallCallbackFunction(technologyID, "WRITE_SOC_REGISTER",		WiFi_Write_Soc_Register);
		TM_InstallCallbackFunction(technologyID, "READ_SOC_REGISTER",		WiFi_Read_Soc_Register);

        TM_InstallCallbackFunction(technologyID, "RUN_EXTERNAL_PROGRAM",	WiFi_Run_External_Program);
		TM_InstallCallbackFunction(technologyID, "POWER_MODE_DUT",			WiFi_Power_Mode_Dut);

		TM_InstallCallbackFunction(technologyID, "LOAD_PATH_LOSS_TABLE",		WiFi_LoadPathLossTable);
		TM_InstallCallbackFunction(technologyID, "LOAD_MULTI_SEGMENT_WAVEFORM", WiFi_Load_Multi_Segment_Waveform);
				
		TM_InstallCallbackFunction(technologyID, "TEMPLATE_FUNCTION",		WiFi_Template_Function);

		TM_InstallCallbackFunction(technologyID, "VDUT_DISABLED",			WiFi_vDut_Disabled);
		TM_InstallCallbackFunction(technologyID, "VDUT_ENABLED",			WiFi_vDut_Enabled);
        
        g_WiFi_Test_ID = technologyID;
		
		/*----------------------*
		 * Create the logger ID *
		 *----------------------*/
		if( -1==g_Logger_ID )
		{
			::LOGGER_CreateLogger("WiFi_Test_Log", &g_Logger_ID);
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

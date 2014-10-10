// LP_Dut_11ac.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "IQlite_Timer.h"
#include "vDUT.h"
#include "IQmeasure.h"
#include "IQlite_Logger.h"
#include "LP_Dut_11ac.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#ifdef WIN32
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif


#ifdef _MANAGED
#pragma managed(pop)
#endif

//#define BT_POWER		-10

char   BT_DATA_RATE[MAX_BUFFER_SIZE], DATA_RATE[MAX_BUFFER_SIZE], PREAMBLE[MAX_BUFFER_SIZE], PACKET_FORMAT[MAX_BUFFER_SIZE];

// Global variables ------------------------------------------------------
double	    g_rxBitNumber                       = 0;
int         g_LP_connected                      = 0;
int         g_logger_id                         = -1;
vDUT_ID     g_LP_DUT_11ac_id                  	= -1;
const char  *g_LP_Dut_Version                   = "1.5.2 (2012-03-13)\n";
char	    g_defaultFilePath[MAX_BUFFER_SIZE]  = {0};
char        g_strMsg[MAX_BUFFER_SIZE]           = {0};

// For LP_Dut Setup ------------------------------------------------------
int	        g_HT40ModeOn                        = 0;                    // HT40 mode on/off                                 
int         g_freqMHz                           = 0;
int			g_freqMHz_Secondary					= 0; 
int			g_freqMHz_Primary_20MHz				= 0;
int			g_freqMHz_Primary_40MHz				= 0;
int			g_freqMHz_Primary_80MHz				= 0;
int			g_freqMHz_BSSPrimary 				= 0;
int			g_freqMHz_BSSSecondary 				= 0;
int			g_BSS_bandwidth						= 0;
int 		g_CH_bandwidth						= 0;
int         g_num_streams_11ac                  = 0; 
int	        g_currentAnt1                       = 0;                    // current ant1
int	        g_currentAnt2                       = 0;                    // current ant2
int	        g_currentAnt3                       = 0;                    // current ant3
int	        g_currentAnt4                       = 0;                    // current ant4
//int			g_bt_powerLevel                     = 0;                    // BT power level
//double      g_bt_power_expected                 = 0.00;                 // BT Tx power expected 
double      g_wifi_tx_power                     = 0.00;                 // WiFi Tx power 
char        g_preModFile[MAX_BUFFER_SIZE]       = {0};                  //
//char        g_bt_data_rate[MAX_BUFFER_SIZE]     = {0};                  // BT data rate 
char        g_wifi_data_rate[MAX_BUFFER_SIZE]   = {0};                  // WiFi data rate
char        g_preamble[MAX_BUFFER_SIZE]         = {0};                  //
char        g_packet_format[MAX_BUFFER_SIZE]= {0};                  //


//typedef enum tagBWMode
//{
//    BW_20MHZ,
//	BW_40MHZ,
//	BW_80MHZ,
//	BW_160MHZ,
//	BW_80_80MHZ,
//	BW_NON
//} WIFI_BW_MODE;




int  CheckReturnError(int errorCode, const char *format, ...)
{
    int  err = 0;
	char errMsg[MAX_BUFFER_SIZE];

    if( 0!=errorCode )	// Error code return failed
    {
		// Log message format ... variable argument list
		va_list ap;
		va_start(ap, format);
		vsprintf_s(errMsg, MAX_BUFFER_SIZE, format, ap);
		va_end(ap);

		// Write error message to log file
		if (g_logger_id>=0)
		{
			if (0!=::LOGGER_Write(g_logger_id, LOGGER_ERROR, errMsg))
			{	// fail to log into file, disable the logger ID
				g_logger_id = -1;
			}
			else
			{
				// do nothing
			}
		}
		else
		{
			// do nothing
		}

        throw errMsg;
    }
	else
	{
		// do nothing
	}

    return err;
}


/*LP_DUT_11AC_API int BT_DutRegister(void)
{
	vDUT_ID dutID = -1;
	vDUT_RegisterTechnology("BT", &dutID);
	if( dutID>=0 )
	{
		vDUT_InstallCallbackFunction(dutID, "INSERT_DUT"            ,LP_insert_dut);        
		vDUT_InstallCallbackFunction(dutID, "REMOVE_DUT"            ,LP_remove_dut);
		vDUT_InstallCallbackFunction(dutID, "INITIALIZE_DUT"        ,LP_initialize_dut);
		vDUT_InstallCallbackFunction(dutID, "RESET_DUT"             ,NULL);

		vDUT_InstallCallbackFunction(dutID, "SET_BT_ADDR"           ,NULL);
		vDUT_InstallCallbackFunction(dutID, "GET_BT_ADDR"           ,NULL);
		vDUT_InstallCallbackFunction(dutID, "READ_EEPROM"           ,NULL);
		vDUT_InstallCallbackFunction(dutID, "WRITE_EEPROM"          ,NULL);

		vDUT_InstallCallbackFunction(dutID, "TX_START"              ,LP_bt_tx_start);
		vDUT_InstallCallbackFunction(dutID, "TX_STOP"               ,LP_tx_stop);

		vDUT_InstallCallbackFunction(dutID, "RX_CLEAR_STATS"        ,LP_bt_clear_rx_pckts);
		vDUT_InstallCallbackFunction(dutID, "RX_GET_STATS"          ,LP_bt_get_rx_stats);
		vDUT_InstallCallbackFunction(dutID, "RX_START"              ,LP_rx_start);
		vDUT_InstallCallbackFunction(dutID, "RX_STOP"               ,LP_rx_stop);

		vDUT_InstallCallbackFunction(dutID, "QUERY_POWER_DBM"       ,LP_query_power_dbm);

		g_LP_DUT_11ac_id = dutID;
	}
	else
	{
		// do nothing
	}
	return dutID;
}*/


/*
 * The return value of this function does not follow the general rule.
 * If no errors, it returns a value >=0 (0 being the first ID)
 * If errors occur, it always return -1
 */
LP_DUT_11AC_API int WIFI_11AC_MIMO_DutRegister(void)
{
    vDUT_ID dutID = -1;
    vDUT_RegisterTechnology("WIFI_11AC", &dutID);
    if( dutID>=0 )
    {
        vDUT_InstallCallbackFunction(dutID, "INSERT_DUT"            ,LP_insert_dut);
        vDUT_InstallCallbackFunction(dutID, "INITIALIZE_DUT"        ,LP_initialize_dut);
        vDUT_InstallCallbackFunction(dutID, "REMOVE_DUT"            ,LP_remove_dut);

        vDUT_InstallCallbackFunction(dutID, "WRITE_MAC_ADDRESS"     ,LP_write_mac_address);
        vDUT_InstallCallbackFunction(dutID, "READ_MAC_ADDRESS"      ,NULL);
        vDUT_InstallCallbackFunction(dutID, "READ_EEPROM"           ,NULL);
        vDUT_InstallCallbackFunction(dutID, "WRITE_EEPROM"          ,NULL);
        vDUT_InstallCallbackFunction(dutID, "GET_SERIAL_NUMBER"     ,LP_get_serial_number);

        vDUT_InstallCallbackFunction(dutID, "RF_SET_FREQ"           ,LP_set_freq);

        vDUT_InstallCallbackFunction(dutID, "TX_PRE_TX"             ,LP_prepare_tx);
        vDUT_InstallCallbackFunction(dutID, "TX_SET_BW"             ,LP_tx_set_bw);
        vDUT_InstallCallbackFunction(dutID, "TX_SET_DATA_RATE"      ,LP_tx_set_data_rate);
        vDUT_InstallCallbackFunction(dutID, "TX_SET_ANTENNA"        ,LP_tx_set_antenna);
        vDUT_InstallCallbackFunction(dutID, "TX_START"              ,LP_tx_start);
        vDUT_InstallCallbackFunction(dutID, "TX_STOP"               ,LP_tx_stop);

        vDUT_InstallCallbackFunction(dutID, "TX_SET_MISC1"          ,NULL);
        vDUT_InstallCallbackFunction(dutID, "TX_SET_MISC2"          ,NULL);
        vDUT_InstallCallbackFunction(dutID, "TX_SET_MISC3"          ,NULL);

        vDUT_InstallCallbackFunction(dutID, "RX_PRE_RX"             ,LP_prepare_rx);
        vDUT_InstallCallbackFunction(dutID, "RX_SET_BW"             ,LP_rx_set_bw);
        vDUT_InstallCallbackFunction(dutID, "RX_SET_DATA_RATE"      ,LP_rx_set_data_rate);
        vDUT_InstallCallbackFunction(dutID, "RX_SET_ANTENNA"        ,LP_set_rx_antenna);
        vDUT_InstallCallbackFunction(dutID, "RX_SET_FILTER"         ,LP_set_bssid_filter);
        vDUT_InstallCallbackFunction(dutID, "RX_CLEAR_STATS"        ,LP_clear_rx_pckts);
        vDUT_InstallCallbackFunction(dutID, "RX_GET_STATS"          ,LP_get_rx_stats);
        vDUT_InstallCallbackFunction(dutID, "RX_START"              ,LP_rx_start);
        vDUT_InstallCallbackFunction(dutID, "RX_STOP"               ,LP_rx_stop);

        vDUT_InstallCallbackFunction(dutID, "RX_SET_MISC1"          ,NULL);
        vDUT_InstallCallbackFunction(dutID, "RX_SET_MISC2"          ,NULL);
        vDUT_InstallCallbackFunction(dutID, "RX_SET_MISC3"          ,NULL);

        g_LP_DUT_11ac_id = dutID;

		/*----------------------*
		 * Create the logger ID *
		 *----------------------*/
		if( -1==g_logger_id )
		{
			::LOGGER_CreateLogger("LP_Dut", &g_logger_id, "LP_Dut");
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

    return dutID;
}

int LP_insert_dut(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

	try
	{
		api_status=::vDUT_AddStringReturn(g_LP_DUT_11ac_id, "DUT_VERSION", (char*)g_LP_Dut_Version);
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(DUT_VERSION) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }    

    return api_status;
}

int LP_remove_dut(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

	try 
	{
		//g_LP_DUT_11ac_id = -1;	
		g_LP_connected = 0;		// LP_Status = disconnected now
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }   

    return api_status;
}

int LP_initialize_dut(void)
{
    int  api_status= 0 ;
	char dutInfo[MAX_BUFFER_SIZE];

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

	try
	{
		// clear the previous mod file name
		sprintf_s(g_preModFile, MAX_BUFFER_SIZE, "%s", "");

		api_status = ::LP_GetVersion(dutInfo, MAX_BUFFER_SIZE);		
		if (false==api_status)
		{
			api_status = -1;
			CheckReturnError(api_status, "[Error] LP_GetVersion() return error.\n");
		}
		else
		{
			api_status = 0;
			g_LP_connected = 1;		// LP_Status = connected now
		}

		api_status=::vDUT_AddStringReturn(g_LP_DUT_11ac_id, "DUT_NAME", "Litepoint Loop-Back Test");
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(DUT_NAME) return error.\n");

		//api_status=::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "IQTESTER_CONTROL_METHOD", &testerType); // Added by Jarir , 12/9/11 to make LP_dut understand what type of tester, for waveform loading
		//CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(DUT_NAME) return error.\n");

		api_status=::vDUT_AddStringReturn(g_LP_DUT_11ac_id, "DUT_DRIVER_INFO", dutInfo);
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(DUT_DRIVER_INFO) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_prepare_tx(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

    return api_status;
}

int LP_tx_set_bw(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

	try
	{
		//api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "CHANNEL_BW", &g_HT40ModeOn);
		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "BSS_BANDWIDTH", &g_BSS_bandwidth);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(BSS_BANDWIDTH) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "CH_BANDWIDTH", &g_CH_bandwidth);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CH_BANDWIDTH) return error.\n");


		
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_tx_set_data_rate(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

	try
	{
		api_status = ::vDUT_GetStringParameter(g_LP_DUT_11ac_id, "DATA_RATE", g_wifi_data_rate, MAX_BUFFER_SIZE);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(DATA_RATE) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "NUM_STREAM_11AC", &g_num_streams_11ac);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(NUM_STREAM_11AC) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_set_freq(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

	try
	{
		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "BSS_FREQ_MHZ_PRIMARY", &g_freqMHz_BSSPrimary);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(BSS_FREQ_MHZ_PRIMARY) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "BSS_FREQ_MHZ_SECONDARY", &g_freqMHz_BSSSecondary);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(BSS_FREQ_MHZ_SECONDARY) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "CH_FREQ_MHZ", &g_freqMHz);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CH_FREQ_MHZ) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "CH_FREQ_MHZ_PRIMARY_20MHz", &g_freqMHz_Primary_20MHz);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CH_FREQ_MHZ_PRIMARY_20MHz) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "CH_FREQ_MHZ_PRIMARY_40MHz", &g_freqMHz_Primary_40MHz);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CH_FREQ_MHZ_PRIMARY_40MHz) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "CH_FREQ_MHZ_PRIMARY_80MHz", &g_freqMHz_Primary_80MHz);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CH_FREQ_MHZ_PRIMARY_80MHz) return error.\n");




		api_status = ::vDUT_GetStringParameter (g_LP_DUT_11ac_id, "PREAMBLE", g_preamble, MAX_BUFFER_SIZE);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(PREAMBLE) return error.\n");

		api_status = ::vDUT_GetStringParameter (g_LP_DUT_11ac_id, "PACKET_FORMAT", g_packet_format, MAX_BUFFER_SIZE);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(PACKET_FORMAT) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_tx_set_antenna(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}
	
	try
	{
		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "TX1", &g_currentAnt1);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(TX1) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "TX2", &g_currentAnt2);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(TX2) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "TX3", &g_currentAnt3);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(TX3) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "TX4", &g_currentAnt4);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(TX4) return error.\n");

		api_status = ::vDUT_GetDoubleParameter (g_LP_DUT_11ac_id, "TX_POWER_DBM", &g_wifi_tx_power);
		CheckReturnError(api_status, "[Error] vDUT_GetDoubleParameter(TX_POWER_DBM) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_tx_start(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}
	
	try
	{
	   /*--------------*
		* Turn off VSG *
		*--------------*/
		api_status = ::LP_EnableVsgRF(0);
		CheckReturnError(api_status, "[Error] LP_EnableVsgRF(0) return error.\n");

	   /*--------------------*
		* Setup IQTester VSG *
		*--------------------*/ 

		if (( g_BSS_bandwidth == BW_80_80MHZ) && ( g_CH_bandwidth == BW_80_80MHZ))
		{
			api_status = 0;
			vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "Current HW doesn't support BW_80_80MHZ");
			return api_status;

		}
		else
		{
			api_status = ::LP_SetVsg(g_freqMHz*1e6, g_wifi_tx_power, PORT_RIGHT);
			CheckReturnError(api_status, "[Error] LP_SetVsg() return error.\n");

		}
		
	   /*---------------------*
		* Load waveform Files *
		*---------------------*/
		char   modFile[MAX_BUFFER_SIZE];
		api_status = GetWaveformFileName("../mod/", 
										 "iqvsg",	
										 g_num_streams_11ac,
										 g_CH_bandwidth, 
										 g_wifi_data_rate, 
										 g_preamble,  
										 g_packet_format, 
										 modFile, 
										 MAX_BUFFER_SIZE);
 		CheckReturnError(api_status, "[Error] GetWaveformFileName(%s) return error.\n",modFile);

		if ( 0!=strcmp(modFile, g_preModFile) )	// mod file name != g_preModFile
		{
			sprintf_s(g_preModFile, MAX_BUFFER_SIZE, "%s", modFile);
			api_status = ::LP_SetVsgModulation( modFile );
			CheckReturnError(api_status, "[Error] LP_SetVsgModulation(%s) return error.\n", modFile);
		}
		else
		{
			// do nothing
		}	

	   /*-------------*
		* Turn on VSG *
		*-------------*/
		api_status = ::LP_EnableVsgRF(1);
		CheckReturnError(api_status, "[Error] LP_EnableVsgRF(1) return error.\n");

	   /*--------------------------------------*
		* Send packet, continuous transmission *
		*--------------------------------------*/           
		api_status = ::LP_SetFrameCnt(0);	
		CheckReturnError(api_status, "[Error] LP_SetFrameCnt(0) return error.\n");

		// delay time to wait for power settle
		Sleep(DELAY_FOR_POWER_SETTLE_MS);
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}


int LP_tx_stop(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

	try
	{
	   /*----------------------------*
		* Disable VSG output signal  *
		*----------------------------*/
		api_status = ::LP_EnableVsgRF(0);
		CheckReturnError(api_status, "[Error] LP_EnableVsgRF(0) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_dut_info(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

    return api_status;
}

int LP_prepare_rx(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

    return api_status;
}

int LP_rx_set_bw(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

    return api_status;
}

int LP_rx_set_data_rate(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

    return api_status;
}

int LP_set_bssid_filter(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

    return api_status;
}

int LP_set_rx_antenna(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

	try
	{	
		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "RX1", &g_currentAnt1);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(RX1) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "RX2", &g_currentAnt2);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(RX2) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "RX3", &g_currentAnt3);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(RX3) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "RX4", &g_currentAnt4);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(RX4) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_clear_rx_pckts(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

    return api_status;
}

int LP_bt_clear_rx_pckts(void)
{
	int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
	if( !g_LP_connected )
	{
		return -1;
	}
	else
	{
		// do nothing
	}

	try
	{	
		api_status = ::vDUT_GetDoubleParameter(g_LP_DUT_11ac_id, "RX_BIT_NUMBER", &g_rxBitNumber);
		CheckReturnError(api_status, "[Error] vDUT_GetDoubleParameter(RX_BIT_NUMBER) return error.\n");
	}
	catch(char *msg)
	{
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
	}
	catch(...)
	{
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
	}
	return api_status;
}


int LP_get_rx_stats(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	unsigned long cntRx=0, cntOk=0, cntKo=0;
	double rssi=0.0;	//, nf=0;

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}
       
	try
	{			
		api_status = ::vDUT_AddIntegerReturn(g_LP_DUT_11ac_id, "TOTAL_PACKETS", cntRx);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(TOTAL_PACKETS) return error.\n");

		api_status = ::vDUT_AddIntegerReturn(g_LP_DUT_11ac_id, "GOOD_PACKETS", cntOk);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(GOOD_PACKETS) return error.\n");

		api_status = ::vDUT_AddIntegerReturn(g_LP_DUT_11ac_id, "BAD_PACKETS", cntKo);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(BAD_PACKETS) return error.\n");
		
		api_status = ::vDUT_AddDoubleReturn(g_LP_DUT_11ac_id, "RSSI_ALL", rssi);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(RSSI) return error.\n");

		//api_status = ::vDUT_AddIntegerReturn(g_LP_DUT_11ac_id, "RSSI", rssi);
		//CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(RSSI) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_bt_get_rx_stats(void)
{
	int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
	if( !g_LP_connected )
	{
		return -1;
	}
	else
	{
		// do nothing
	}
 
	try
	{			
		api_status = ::vDUT_AddDoubleReturn(g_LP_DUT_11ac_id, "GOOD_BITS", 0);
		CheckReturnError(api_status, "[Error] vDUT_AddDoubleReturn(GOOD_BITS) return error.\n");

		api_status = ::vDUT_AddDoubleReturn(g_LP_DUT_11ac_id, "TOTAL_BITS", g_rxBitNumber);
		CheckReturnError(api_status, "[Error] vDUT_AddDoubleReturn(TOTAL_BITS) return error.\n");

		api_status = ::vDUT_AddDoubleReturn(g_LP_DUT_11ac_id, "RSSI", 0);
		CheckReturnError(api_status, "[Error] vDUT_AddDoubleReturn(RSSI) return error.\n");
	}
	catch(char *msg)
	{
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
	}
	catch(...)
	{
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
	}

	return api_status;
}


int LP_rx_start(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

    return api_status;
}


int LP_rx_stop(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

    return api_status;
}

int  GetWaveformFileName(char* filePath, char* fileType, int streamNum_11AC, int chBW, char* datarate, 
						 char* preamble, char* packetFormat, char* waveformFileName, int bufferSize)
{
    int  api_status = -1;
	char waveFilePath[MAX_BUFFER_SIZE] = {'\0'};


	::vDUT_ClearReturns(g_LP_DUT_11ac_id);	

	if ( strlen(filePath)>0 )		// user defined path is not empty
	{
		sprintf_s(waveFilePath, MAX_BUFFER_SIZE, "%s", filePath);
	}
	else if ( strlen(g_defaultFilePath)>0 )		// default waveform file path not empty
	{
		sprintf_s(waveFilePath, MAX_BUFFER_SIZE, "%s", g_defaultFilePath);
	}
	else	// both are empty
	{
		sprintf_s(waveFilePath, MAX_BUFFER_SIZE, "../Mod");
	}

	if ( !strstr(packetFormat, "11AC")) //Legacy mode
	{

		if ( 0==strcmp(datarate, "DSSS-1") )
		{
			sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s.%s", waveFilePath, datarate, fileType);
		}
		else if ( (strstr(datarate, "DSSS")!=NULL)||(strstr(datarate, "CCK")!=NULL) )
		{
			char tmpPreamble[2]; 
			tmpPreamble[0] = preamble[0];
			tmpPreamble[1] = '\0';
			sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s%s.%s", waveFilePath, datarate, tmpPreamble, fileType);
		}
		else if ( (strstr(datarate, "OFDM")!=NULL) )
		{
			sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s.%s", waveFilePath, datarate, fileType);
		}
		else if ( (strstr(datarate, "MCS")!=NULL) )
		{
			if (chBW == BW_20MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s_%s.%s", waveFilePath, "HT20", datarate, fileType);
			}
			else
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s_%s.%s", waveFilePath, "HT40", datarate, fileType);
			}
		}
		else
		{
			// do nothing
			return -1;
		}
	}
	else    // 11AC mode
	{
		if ((strstr (packetFormat,"11AC_VHT" )) && (strstr(datarate, "MCS")!=NULL) &&
			(streamNum_11AC >0)) //11ac VHT
		{
			if ( chBW == BW_20MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s20_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_40MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s40_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_160MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s160_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else if (chBW == BW_80_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_80_S%d_%s.%s", waveFilePath, packetFormat,streamNum_11AC, datarate, fileType);
			}
			else
			{
				return -1;
			}
			
		}
		else if ((strstr (packetFormat,"11AC_MF_HT") || strstr (packetFormat,"11AC_GF_HT"))
			&& (strstr(datarate, "MCS")!=NULL)) //11ac HT
		{
			if ( chBW == BW_20MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_HT20_%s.%s", waveFilePath, datarate, fileType);
			}
			else if ( chBW == BW_40MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_HT40_%s.%s", waveFilePath, datarate, fileType);
			}
			else
			{
				return -1;
			}
		}
		else if ( strstr(packetFormat ,"11AC_NON_HT")) //WIFI_11AC_NON_HT
		{
			if (chBW == BW_40MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s40_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else if (chBW == BW_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else if (chBW == BW_160MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s160_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else if (chBW == BW_80_80MHZ)
			{
				sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s80_80_%s.%s", waveFilePath, packetFormat, datarate, fileType);
			}
			else      // BW != 20MHz
			{
				return -1;
			}
		
		}
		else     //wrong wifiMode
		{
			return -1;
		}

	}


	// Wave file checking
	FILE *waveFile;
	fopen_s(&waveFile, waveformFileName, "r");
	if (!waveFile)
	{
		if ( strlen(g_defaultFilePath)>0 )		// Default waveform file path not empty, but still can't find the waveformfile, then return error.
		{
			return -1;	//TM_ERR_PARAM_DOES_NOT_EXIST
		}
		else	// Try to find the file in upper folder
		{
			char tempPath[MAX_BUFFER_SIZE] = {'\0'};
			sprintf_s(tempPath, bufferSize, "../%s", waveformFileName);

			fopen_s(&waveFile, tempPath, "r");
			if (!waveFile)
			{
				return -1;	//TM_ERR_PARAM_DOES_NOT_EXIST
			}
			else
			{
				// Save the path to default path name
				sprintf_s(g_defaultFilePath, bufferSize, "../%s", waveFilePath);
				// And using the new path
				sprintf_s(waveformFileName, bufferSize, "%s", tempPath);

				fclose(waveFile);
				waveFile = NULL;
				api_status = 0;
			}
		}
	}
	else
	{
		fclose(waveFile);
		waveFile = NULL;
		api_status = 0;
	}

	return api_status;
}

int LP_get_serial_number(void)
{
    int  api_status= 0 ;
	char sn[MAX_BUFFER_SIZE];

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

	try
	{
		sprintf_s(sn, MAX_BUFFER_SIZE, "090722-123456");

		api_status=::vDUT_AddStringReturn(g_LP_DUT_11ac_id, "SERIAL_NUMBER", sn);
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(SERIAL_NUMBER) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_write_mac_address(void)
{
    int  api_status= 0 ;
	char mac[MAX_BUFFER_SIZE];

	::vDUT_ClearReturns(g_LP_DUT_11ac_id);

    //First, check LP tester status
    //if( !g_LP_connected )
    //{
    //    return -1;
    //}
    //else
    //{
    //    //do nothing
    //}

	try
	{
		sprintf_s(mac, MAX_BUFFER_SIZE, "01:00:00:C0:FF:EE");

        api_status=::vDUT_AddStringReturn(g_LP_DUT_11ac_id, "MAC_ADDRESS", mac);
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(MAC_ADDRESS) return error.\n");

	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_DUT_11ac_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

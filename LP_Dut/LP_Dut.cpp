// LP_Dut.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#include "IQlite_Timer.h"
#include "vDUT.h"
#include "IQmeasure.h"
#include "IQlite_Logger.h"
#include "LP_Dut.h"
#include <string>
#include <iostream>
using namespace std;



#define BT_POWER		-10


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


// Global variables
double		 g_rxBitNumber = 0;
int          g_LP_connected = 0;
int          g_logger_id   = -1;
vDUT_ID      g_LP_mimo_id  = -1;
const char  *g_LP_Dut_Version = "1.5.0 (2010-04-30)\n";
char	     g_defaultFilePath[MAX_BUFFER_SIZE] = {'\0'};
char         g_strMsg[MAX_BUFFER_SIZE];

#define		 TESTER_TYPE_IQ2010_LEGACY		0
#define		 TESTER_TYPE_IQXEL_TYPE			1
int			 g_testerType = TESTER_TYPE_IQ2010_LEGACY;

size_t		g_numWaveFiles = 0;
char		*g_waveFiles[64];


// For LP_Dut Setup
int	   HT40ModeOn, FREQ_MHZ;
int	   TX1, TX2, TX3, TX4;
int    RX1, RX2, RX3, RX4;
double TX_POWER_DBM;
char   preModFile[MAX_BUFFER_SIZE] = {""};
char   BT_DATA_RATE[MAX_BUFFER_SIZE], DATA_RATE[MAX_BUFFER_SIZE], PREAMBLE[MAX_BUFFER_SIZE], PACKET_FORMAT_11N[MAX_BUFFER_SIZE];


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


LP_DUT_API int BT_DutRegister(void)
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

		vDUT_InstallCallbackFunction(dutID, "TX_BTLE_START"         ,LP_btle_tx_start);

		vDUT_InstallCallbackFunction(dutID, "RX_CLEAR_STATS"        ,LP_bt_clear_rx_pckts);
		vDUT_InstallCallbackFunction(dutID, "RX_GET_STATS"          ,LP_bt_get_rx_stats);
		vDUT_InstallCallbackFunction(dutID, "RX_START"              ,LP_rx_start);
		vDUT_InstallCallbackFunction(dutID, "RX_STOP"               ,LP_rx_stop);

		vDUT_InstallCallbackFunction(dutID, "QUERY_POWER_DBM"       ,LP_query_power_dbm);

		g_LP_mimo_id = dutID;
	}
	else
	{
		// do nothing
	}
	return dutID;
}


/*
 * The return value of this function does not follow the general rule.
 * If no errors, it returns a value >=0 (0 being the first ID)
 * If errors occur, it always return -1
 */
LP_DUT_API int WIFI_DutRegister(void)
{
    vDUT_ID dutID = -1;
    vDUT_RegisterTechnology("WIFI", &dutID);
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

        g_LP_mimo_id = dutID;

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

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		api_status=::vDUT_AddStringReturn(g_LP_mimo_id, "DUT_VERSION", (char*)g_LP_Dut_Version);
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(DUT_VERSION) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }    

    return api_status;
}

int LP_remove_dut(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		//g_LP_mimo_id = -1;	
		g_LP_connected = 0;		// LP_Status = disconnected now
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }   

    return api_status;
}

int LP_initialize_dut(void)
{
    int  api_status= 0 ;
	char dutInfo[MAX_BUFFER_SIZE];

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		sprintf_s(preModFile, MAX_BUFFER_SIZE, "%s", "");

		api_status = ::LP_GetVersion(dutInfo, MAX_BUFFER_SIZE);		
		if (false==api_status)
		{
			api_status = -1;
			CheckReturnError(api_status, "[Error] LP_GetVersion() return error.\n");
		}
		else
		{
			if ( strstr( dutInfo, "IQXEL" ) ) {
					// this vsg is an IQxel tester; otherwise assume legacy tester
				g_testerType = TESTER_TYPE_IQXEL_TYPE;
			}
			api_status = 0;
			g_LP_connected = 1;		// LP_Status = connected now
		}

		api_status=::vDUT_AddStringReturn(g_LP_mimo_id, "DUT_NAME", "Litepoint Loop-Back Test");
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(DUT_NAME) return error.\n");

		api_status=::vDUT_AddStringReturn(g_LP_mimo_id, "DUT_DRIVER_INFO", dutInfo);
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(DUT_DRIVER_INFO) return error.\n");

	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_prepare_tx(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "CHANNEL_BW", &HT40ModeOn);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CHANNEL_BW) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_tx_set_data_rate(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		api_status = ::vDUT_GetStringParameter(g_LP_mimo_id, "DATA_RATE", DATA_RATE, MAX_BUFFER_SIZE);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(DATA_RATE) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_set_freq(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "FREQ_MHZ", &FREQ_MHZ);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(FREQ_MHZ) return error.\n");

		api_status = ::vDUT_GetStringParameter (g_LP_mimo_id, "PREAMBLE", PREAMBLE, MAX_BUFFER_SIZE);
		if ( api_status == vDUT_ERR_PARAM_DOES_NOT_EXIST ) {
			// no preamble, then set to LONG
			strcpy_s( PREAMBLE, MAX_BUFFER_SIZE, "LONG" );
		} else if ( api_status != 0 ) {
			CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(PREAMBLE) return error.\n");
		}

		api_status = ::vDUT_GetStringParameter (g_LP_mimo_id, "PACKET_FORMAT_11N", PACKET_FORMAT_11N, MAX_BUFFER_SIZE);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(PACKET_FORMAT_11N) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_tx_set_antenna(void)
{
    int api_status1 = 0;
    int api_status2 = 0;
	int func_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		api_status1 = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "TX1", &TX1);
		api_status2 = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "ANT1", &TX1);
		if ( ( api_status1 != 0 ) && ( api_status2 != 0 ) ) {
			func_status = 99;
			CheckReturnError(api_status1, "[Error] vDUT_GetIntegerParameter(TX1) return error.\n");
		}

		api_status1 = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "TX2", &TX2);
		api_status1 = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "ANT2", &TX2);
		if ( ( api_status1 != 0 ) && ( api_status2 != 0 ) ) {
			func_status = 99;
			CheckReturnError(api_status1, "[Error] vDUT_GetIntegerParameter(TX2) return error.\n");
		}

		api_status1 = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "TX3", &TX3);
		api_status2 = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "ANT3", &TX3);
		if ( ( api_status1 != 0 ) && ( api_status2 != 0 ) ) {
			func_status = 99;
			CheckReturnError(api_status1, "[Error] vDUT_GetIntegerParameter(TX3) return error.\n");
		}

		api_status1 = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "TX4", &TX4);
		api_status2 = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "ANT4", &TX4);
		if ( ( api_status1 != 0 ) && ( api_status2 != 0 ) ) {
			func_status = 99;
			CheckReturnError(api_status1, "[Error] vDUT_GetIntegerParameter(TX4) return error.\n");
		}

		api_status1 = ::vDUT_GetDoubleParameter (g_LP_mimo_id, "TX_POWER_DBM", &TX_POWER_DBM);
		api_status2 = ::vDUT_GetDoubleParameter (g_LP_mimo_id, "TX_POWER_DBM", &TX_POWER_DBM);
		if ( ( api_status1 != 0 ) && ( api_status2 != 0 ) ) {
			func_status = 99;
			CheckReturnError(api_status1, "[Error] vDUT_GetDoubleParameter(TX_POWER_DBM) return error.\n");
		}
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return func_status;
}

int LP_tx_start(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		api_status = ::LP_SetVsg(FREQ_MHZ*1e6, TX_POWER_DBM, PORT_RIGHT);
		CheckReturnError(api_status, "[Error] LP_SetVsg() return error.\n");

	   /*---------------------*
		* Load waveform Files *
		*---------------------*/
		char   modFile[MAX_BUFFER_SIZE];
		api_status = GetWaveformFileName("../mod/", 
										 "mod", 									  
										 HT40ModeOn, 
										 DATA_RATE, 
										 PREAMBLE, 
										 PACKET_FORMAT_11N, 
										 modFile, 
										 MAX_BUFFER_SIZE);
		CheckReturnError(api_status, "[Error] GetWaveformFileName(mod_file) return error.\n");

		if ( 0!=strcmp(modFile, preModFile) )	// mod file name != preModFile
		{
			sprintf_s(preModFile, MAX_BUFFER_SIZE, "%s", modFile);
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
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}



int LP_bt_tx_start(void)
{
	int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		TX_POWER_DBM = BT_POWER;
		api_status = ::LP_SetVsg(FREQ_MHZ*1e6, TX_POWER_DBM, PORT_RIGHT);
		CheckReturnError(api_status, "[Error] LP_SetVsg() return error.\n");

		char dataRate[10]; 
		int modulation;
		// get packet type
		api_status = vDUT_GetStringParameter(g_LP_mimo_id, "PACKET_TYPE", dataRate, sizeof(dataRate));
		if (api_status!=ERR_OK)
			CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(PACKET_TYPE) return error.\n");
		else
			strcpy_s(BT_DATA_RATE, dataRate);
		
		api_status = vDUT_GetIntegerParameter(g_LP_mimo_id, "MODULATION_TYPE", &modulation);
		
		if (api_status!=ERR_OK)
			CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(MODULATION_TYPE) return error.\n");

		/*---------------------*
		* Load waveform Files *
		*---------------------*/
		char   modFile[MAX_BUFFER_SIZE];
		/*
		api_status = GetWaveformFileName("../mod/", 
			"iqvsg", 									  
			HT40ModeOn, 
			BT_DATA_RATE, 
			PREAMBLE, 
			PACKET_FORMAT_11N, 
			modFile, 
			MAX_BUFFER_SIZE); 
		CheckReturnError(api_status, "[Error] GetWaveformFileName(mod_file) return error.\n");
		*/
		if ( (strstr(dataRate, "DH")!=NULL) )
		{		
			string temp;
			temp+=dataRate;
			
			switch(modulation)
			{
				
				case BT_PATTERN_AA:			// 0xAA 8-bit Pattern	   HEX_A : 1010
					sprintf_s(modFile, MAX_BUFFER_SIZE, "..\\mod\\BT_%s_1010_Fs80M.iqvsg", temp.c_str());
					break;
				case BT_PATTERN_PRBS:		// PRBS9 Pattern		   RANDOM ;  PRBS9
					sprintf_s(modFile, MAX_BUFFER_SIZE, "..\\mod\\BT_%s_prbs9_Fs80M.iqvsg",  temp.c_str());		
					break;
				case BT_PATTERN_F0: 		// 0xF0 8-bit Pattern	   HEX_F0 : 11110000
					sprintf_s(modFile, MAX_BUFFER_SIZE, "..\\mod\\BT_%s_00001111_Fs80M.iqvsg",  temp.c_str());		
					break;
			}		
			
		}
		else if ( (strstr(dataRate, "1LE")!=NULL) )
		{
			switch(modulation)
			{
				case BT_LE_PATTERN_AA:			// 0xAA 8-bit Pattern	   HEX_A : 1010
					sprintf_s(modFile, MAX_BUFFER_SIZE, "..\\mod\\BT_1LE_1010_Fs80M.iqvsg");		
					break;
				case BT_LE_PATTERN_PRBS9:		// PRBS9 Pattern		   RANDOM ;  PRBS9
					sprintf_s(modFile, MAX_BUFFER_SIZE, "..\\mod\\BT_1LE_prbs9_Fs80M.iqvsg");		
					break;
				case BT_LE_PATTERN_F0: 		// 0xF0 8-bit Pattern	   HEX_F0 : 11110000
					sprintf_s(modFile, MAX_BUFFER_SIZE, "..\\mod\\BT_1LE_11110000_Fs80M.iqvsg");		
					break;
			}	
		}

		if ( 0!=strcmp(modFile, preModFile) )	// mod file name != preModFile
		{
			sprintf_s(preModFile, MAX_BUFFER_SIZE, "%s", modFile);
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
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
	}
	catch(...)
	{
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
	}

	return api_status;
}

int LP_btle_tx_start(void)
{
	int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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

		char dataRate[10] = "1LE";
		// get packet type
		api_status = vDUT_GetStringParameter(g_LP_mimo_id, "PACKET_TYPE", dataRate, sizeof(dataRate));
		if (api_status!=ERR_OK)
			CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(PACKET_TYPE) return error.\n");
		else
			strcpy_s(BT_DATA_RATE, dataRate);

		/*--------------------*
		* Setup IQTester VSG *
		*--------------------*/
		TX_POWER_DBM = BT_POWER;
		api_status = ::LP_SetVsg(FREQ_MHZ*1e6, TX_POWER_DBM, PORT_RIGHT);
		CheckReturnError(api_status, "[Error] LP_SetVsg() return error.\n");

		/*---------------------*
		* Load waveform Files *
		*---------------------*/
		char   modFile[MAX_BUFFER_SIZE];
		api_status = GetWaveformFileName("../mod/", 
			"mod", 									  
			HT40ModeOn, 
			BT_DATA_RATE, 
			PREAMBLE, 
			PACKET_FORMAT_11N, 
			modFile, 
			MAX_BUFFER_SIZE); 
		CheckReturnError(api_status, "[Error] GetWaveformFileName(mod_file) return error.\n");

		if ( 0!=strcmp(modFile, preModFile) )	// mod file name != preModFile
		{
			sprintf_s(preModFile, MAX_BUFFER_SIZE, "%s", modFile);
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
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
	}
	catch(...)
	{
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
	}

	return api_status;
}


int LP_tx_stop(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_dut_info(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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

	::vDUT_ClearReturns(g_LP_mimo_id);

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

	::vDUT_ClearReturns(g_LP_mimo_id);

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

	::vDUT_ClearReturns(g_LP_mimo_id);

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

	::vDUT_ClearReturns(g_LP_mimo_id);

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

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "RX1", &RX1);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(RX1) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "RX2", &RX2);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(RX2) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "RX3", &RX3);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(RX3) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "RX4", &RX4);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(RX4) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_clear_rx_pckts(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		api_status = ::vDUT_GetDoubleParameter(g_LP_mimo_id, "RX_BIT_NUMBER", &g_rxBitNumber);
		CheckReturnError(api_status, "[Error] vDUT_GetDoubleParameter(RX_BIT_NUMBER) return error.\n");
	}
	catch(char *msg)
	{
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
	}
	catch(...)
	{
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
	}
	return api_status;
}


int LP_get_rx_stats(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

	unsigned long cntRx=0, cntOk=0, cntKo=0;
	int rssi=0;	//, nf=0;

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
		api_status = ::vDUT_AddIntegerReturn(g_LP_mimo_id, "TOTAL_PACKETS", cntRx);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(TOTAL_PACKETS) return error.\n");

		api_status = ::vDUT_AddIntegerReturn(g_LP_mimo_id, "GOOD_PACKETS", cntOk);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(GOOD_PACKETS) return error.\n");

		api_status = ::vDUT_AddIntegerReturn(g_LP_mimo_id, "BAD_PACKETS", cntKo);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(BAD_PACKETS) return error.\n");
		
		api_status = ::vDUT_AddDoubleReturn(g_LP_mimo_id, "RSSI", rssi);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(RSSI) return error.\n");

		//api_status = ::vDUT_AddIntegerReturn(g_LP_mimo_id, "RSSI", rssi);
		//CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(RSSI) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_bt_get_rx_stats(void)
{
	int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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
		api_status = ::vDUT_AddDoubleReturn(g_LP_mimo_id, "GOOD_BITS", 0);
		CheckReturnError(api_status, "[Error] vDUT_AddDoubleReturn(GOOD_BITS) return error.\n");

		api_status = ::vDUT_AddDoubleReturn(g_LP_mimo_id, "TOTAL_BITS", g_rxBitNumber);
		CheckReturnError(api_status, "[Error] vDUT_AddDoubleReturn(TOTAL_BITS) return error.\n");

		api_status = ::vDUT_AddDoubleReturn(g_LP_mimo_id, "RSSI", 0);
		CheckReturnError(api_status, "[Error] vDUT_AddDoubleReturn(RSSI) return error.\n");
	}
	catch(char *msg)
	{
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
	}
	catch(...)
	{
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
	}

	return api_status;
}


int LP_rx_start(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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

int LP_query_power_dbm(void)
{
	int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

	// First, check LP tester status
	if( !g_LP_connected )
	{
		return -1;
	}
	else
	{
		double powerDbm = BT_POWER;
		char dataRate[10]; 

		// get packet type
		api_status = vDUT_GetStringParameter(g_LP_mimo_id, "PACKET_TYPE", dataRate, sizeof(dataRate));
		if (api_status!=ERR_OK)
			CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(PACKET_TYPE) return error.\n");
		else
			strcpy_s(BT_DATA_RATE, dataRate);

		// get frequency
		api_status = vDUT_GetIntegerParameter(g_LP_mimo_id, "FREQ_MHZ", &FREQ_MHZ);
		if (api_status!=ERR_OK)
			CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(FREQ_MHZ) return error.\n");

		// return expected power
		api_status = vDUT_AddDoubleReturn(g_LP_mimo_id, "POWER_DBM", powerDbm);
		CheckReturnError(api_status, "[Error] vDUT_AddDoubleParameter(POWER_DBM) return error.\n");
		// do nothing
	}

	return api_status;
}


int LP_rx_stop(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

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

int  GetWaveformFileName(char* filePath, char* fileType, int HT40ModeOn, char* datarate, char* preamble, char* packetFormat11N, char* waveformFileName, int bufferSize)
{
    int  api_status = -1;
	char waveFilePath[MAX_BUFFER_SIZE] = {'\0'};


	::vDUT_ClearReturns(g_LP_mimo_id);	

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
		if (HT40ModeOn==0)
		{
			sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s_%s.%s", waveFilePath, "HT20", datarate, fileType);
		}
		else
		{
			sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s_%s.%s", waveFilePath, "HT40", datarate, fileType);
		}
	}
	else if ( (strstr(datarate, "DH")!=NULL) )
	{
		sprintf_s(waveformFileName, bufferSize, "%s%s_000088C0FFEE.%s", waveFilePath, datarate, fileType);
	}
	else if ( (strstr(datarate, "1LE")!=NULL) )
	{
		sprintf_s(waveformFileName, bufferSize, "%s%s.%s", waveFilePath, datarate, fileType);
	}
	else
	{
		// do nothing
		return -1;
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

	::vDUT_ClearReturns(g_LP_mimo_id);

	try
	{
		sprintf_s(sn, MAX_BUFFER_SIZE, "090722-123456");

		api_status=::vDUT_AddStringReturn(g_LP_mimo_id, "SERIAL_NUMBER", sn);
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(SERIAL_NUMBER) return error.\n");
	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

int LP_write_mac_address(void)
{
    int  api_status= 0 ;
	char mac[MAX_BUFFER_SIZE];

	::vDUT_ClearReturns(g_LP_mimo_id);

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

        api_status=::vDUT_AddStringReturn(g_LP_mimo_id, "MAC_ADDRESS", mac);
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(MAC_ADDRESS) return error.\n");

	}
	catch(char *msg)
    {
        vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", msg);
    }
    catch(...)
    {
		vDUT_AddStringReturn(g_LP_mimo_id, "ERROR_MESSAGE", "[Error] Unknown Error!\n");
    }

    return api_status;
}

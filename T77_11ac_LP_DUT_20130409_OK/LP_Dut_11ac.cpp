// LP_Dut_11ac.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "IQlite_Timer.h"
#include "vDUT.h"
#include "IQmeasure.h"
#include "IQlite_Logger.h"
#include "LP_Dut_11ac.h"
#include "GetAdapterInfo.h"	// IP_Arp/IP_Ping/IP_Telnetenable
#include "PeerSocket.h"  // for DUT control
#include <string>	// string type
#include "vDUT.h"	// vDUT_WiFiConvertFrequencyToChannel
#include "assert.h" // assert
using namespace std;

// global variables
extern SOCKET g_socket;
extern string strSocketBuf;
extern HANDLE h_socketThread;
extern bool ShowDebugInfor;
extern bool bRequestExit;

int SolutionIndex = 1;
char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use
int g_channel=-1;
map <string,string> g_rates,g_acrates, g_5grate;
/*Define struct mark same dut parameter*/
struct SameDut
{
	int			nOldfreq ;
	int			nOldBandWidth;	
	string		szOldDataRate;	
}IsSameDut={-1,-1,""};

struct 
{
	char szDutIP[24];
	char szKeyWord[128];
	char szCmdPrefix2g[16];
	char szCmdPrefix5g[16];
	char szCmdPrefixReserved[16];
	bool bRunTelnetEnabled;
}csSocket;

#define BUFSIZE 256

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
        Initialize_WSA();
        break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		if(g_socket!=INVALID_SOCKET)
		{
			closesocket(g_socket);
		}
		ambit_WSACleanup();
        break;
    }
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
LP_DUT_11AC_API int WIFI_11AC_DutRegister(void)
{
	// add rates list
	g_rates.clear();
	g_rates.insert( pair<string, string>("DSSS-1",   "-r 1 -s 0") );
	g_rates.insert( pair<string, string>("DSSS-2",   "-r 2 -s 0") );
	g_rates.insert( pair<string, string>("CCK-5_5",  "-r 5.5 -s 0") );
	g_rates.insert( pair<string, string>("CCK-11",   "-r 11 -s 0") );
	g_rates.insert( pair<string, string>("PBCC-22",  "-r 22 -s 0") );
	g_rates.insert( pair<string, string>("OFDM-6",   "-r 6 -s 0") );
	g_rates.insert( pair<string, string>("OFDM-9",   "-r 9 -s 0") );
	g_rates.insert( pair<string, string>("OFDM-12",  "-r 12 -s 0") );
	g_rates.insert( pair<string, string>("OFDM-18",  "-r 18 -s 0") );
	g_rates.insert( pair<string, string>("OFDM-24",  "-r 24 -s 0") );
	g_rates.insert( pair<string, string>("OFDM-36", "-r 36 -s 0") );
	g_rates.insert( pair<string, string>("OFDM-48", "-r 48 -s 0") );
	g_rates.insert( pair<string, string>("OFDM-54", "-r 54 -s 0") );
	g_rates.insert( pair<string, string>("MCS0",    "-m 0 -s 0") );
	g_rates.insert( pair<string, string>("MCS1",    "-m 1 -s 0") );
	g_rates.insert( pair<string, string>("MCS2",    "-m 2 -s 0") );
	g_rates.insert( pair<string, string>("MCS3",    "-m 3 -s 0") );
	g_rates.insert( pair<string, string>("MCS4",    "-m 4 -s 0") );
	g_rates.insert( pair<string, string>("MCS5",    "-m 5 -s 0") );
	g_rates.insert( pair<string, string>("MCS6",    "-m 6 -s 0") );
	g_rates.insert( pair<string, string>("MCS7",    "-m 7 -s 0") );
	g_rates.insert( pair<string, string>("MCS8",    "-m 8 -s 1") );
	g_rates.insert( pair<string, string>("MCS9",    "-m 9 -s 1") );
	g_rates.insert( pair<string, string>("MCS10",   "-m 10 -s 1") );
	g_rates.insert( pair<string, string>("MCS11",   "-m 11 -s 1") );
	g_rates.insert( pair<string, string>("MCS12",   "-m 12 -s 1") );
	g_rates.insert( pair<string, string>("MCS13",   "-m 13 -s 1") );
	g_rates.insert( pair<string, string>("MCS14",   "-m 14 -s 1") );
	g_rates.insert( pair<string, string>("MCS15",   "-m 15 -s 1") );
	g_rates.insert( pair<string, string>("MCS16",   "-m 16 -s 1") );
	g_rates.insert( pair<string, string>("MCS17",   "-m 17 -s 1") );
	g_rates.insert( pair<string, string>("MCS18",   "-m 18 -s 1") );
	g_rates.insert( pair<string, string>("MCS19",   "-m 19 -s 1") );
	g_rates.insert( pair<string, string>("MCS20",   "-m 20 -s 1") );
	g_rates.insert( pair<string, string>("MCS20",   "-m 21 -s 1") );
	g_rates.insert( pair<string, string>("MCS22",   "-m 22 -s 1") );
	g_rates.insert( pair<string, string>("MCS23",   "-m 23 -s 1") );
	g_rates.insert( pair<string, string>("MCS24",   "-m 24 -s 1") );
	g_rates.insert( pair<string, string>("MCS25",   "-m 25 -s 1") );
	g_rates.insert( pair<string, string>("MCS26",   "-m 26 -s 1") );
	g_rates.insert( pair<string, string>("MCS28",   "-m 28 -s 1") );
	g_rates.insert( pair<string, string>("MCS29",   "-m 29 -s 1") );
	g_rates.insert( pair<string, string>("MCS30",   "-m 30 -s 1") );
	g_rates.insert( pair<string, string>("MCS31",   "-m 31 -s 1") );

	// for 5g 11n rate
	g_5grate.clear();
	g_5grate.insert( pair<string, string>("MCS0",   "5g_rate -h 0 -b %d") );
	g_5grate.insert( pair<string, string>("MCS1",   "5g_rate -h 1 -b %d") );
	g_5grate.insert( pair<string, string>("MCS2",   "5g_rate -h 2 -b %d") );
	g_5grate.insert( pair<string, string>("MCS3",   "5g_rate -h 3 -b %d") );
	g_5grate.insert( pair<string, string>("MCS4",   "5g_rate -h 4 -b %d") );
	g_5grate.insert( pair<string, string>("MCS5",   "5g_rate -h 5 -b %d") );
	g_5grate.insert( pair<string, string>("MCS6",   "5g_rate -h 6 -b %d") );
	g_5grate.insert( pair<string, string>("MCS7",   "5g_rate -h 7 -b %d") );
	g_5grate.insert( pair<string, string>("MCS8",   "5g_rate -h 8 -b %d") );
	g_5grate.insert( pair<string, string>("MCS9",   "5g_rate -h 9 -b %d") );
	g_5grate.insert( pair<string, string>("MCS10",   "5g_rate -h 10 -b %d") );
	g_5grate.insert( pair<string, string>("MCS11",   "5g_rate -h 11 -b %d") );
	g_5grate.insert( pair<string, string>("MCS12",   "5g_rate -h 12 -b %d") );
	g_5grate.insert( pair<string, string>("MCS13",   "5g_rate -h 13 -b %d") );
	g_5grate.insert( pair<string, string>("MCS14",   "5g_rate -h 14 -b %d") );
	g_5grate.insert( pair<string, string>("MCS15",   "5g_rate -h 15 -b %d") );
	g_5grate.insert( pair<string, string>("MCS16",   "5g_rate -h 16 -b %d") );
	g_5grate.insert( pair<string, string>("MCS17",   "5g_rate -h 17 -b %d") );
	g_5grate.insert( pair<string, string>("MCS18",   "5g_rate -h 18 -b %d") );
	g_5grate.insert( pair<string, string>("MCS19",   "5g_rate -h 19 -b %d") );
	g_5grate.insert( pair<string, string>("MCS20",   "5g_rate -h 20 -b %d") );
	g_5grate.insert( pair<string, string>("MCS21",   "5g_rate -h 21 -b %d") );
	g_5grate.insert( pair<string, string>("MCS22",   "5g_rate -h 22 -b %d") );
	g_5grate.insert( pair<string, string>("MCS23",   "5g_rate -h 23 -b %d") );
	// for 5g 11ac rate
	g_acrates.clear();
	g_acrates.insert( pair<string, string>("MCS0",   "5g_rate -v 0x1 -b 80") );
	g_acrates.insert( pair<string, string>("MCS1",   "5g_rate -v 1x1 -b 80") );
	g_acrates.insert( pair<string, string>("MCS2",   "5g_rate -v 2x1 -b 80") );
	g_acrates.insert( pair<string, string>("MCS3",   "5g_rate -v 3x1 -b 80") );
	g_acrates.insert( pair<string, string>("MCS4",   "5g_rate -v 4x1 -b 80") );
	g_acrates.insert( pair<string, string>("MCS5",   "5g_rate -v 5x1 -b 80") );
	g_acrates.insert( pair<string, string>("MCS6",   "5g_rate -v 6x1 -b 80") );
	g_acrates.insert( pair<string, string>("MCS7",   "5g_rate -v 7x1 -b 80") );
	g_acrates.insert( pair<string, string>("MCS8",   "5g_rate -v 8x1 -b 80") );
	g_acrates.insert( pair<string, string>("MCS9",   "5g_rate -v 9x1 -b 80") );

	
	// add end
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

		api_status = 0;
		g_LP_connected = 1;		// LP_Status = connected now

		// new add
		char pathStr[MAX_BUFFER_SIZE];
		api_status=LP_read_ini(pathStr);
		CheckReturnError(api_status, "[Error] LP_read_ini(pathStr) return error.\n");

		int ErrorCode=0;
		if(g_socket!=INVALID_SOCKET)
		{
			closesocket(g_socket);
			g_socket=INVALID_SOCKET;
		}

		//Add "arp -d"		
		TCHAR szArpCommand[128] = "";
		sprintf_s(szArpCommand,"arp -d");
		RunSpecifyExeAndRead(szArpCommand , false); 
		//printf("send command \"arp -d \" ok!!!\r\n");
		memset(szArpCommand,0,128);
		//Add end

		DeleteSpecifyExe("ping.exe");

		if(!PingSpecifyIP(csSocket.szDutIP,ErrorCode))
		{
			api_status = ErrorCode; 
			switch(ErrorCode)
			{
			case 1:
				CheckReturnError(api_status, "[Error] Run ping return error.\n");
				break;
			case 2:
				CheckReturnError(api_status, "[Error] Create pipe return error.\n");
				break;
			case 3:
				CheckReturnError(api_status, "[Error] Read pipe return error.\n");
				break;
			};
		}
		DeleteSpecifyExe("ping");
		if(api_status != 0)
		{
			return api_status;
		}

		//char szDutMac[30]="000000000001";
		char szDutMac[30]="00904C012001";
		if(csSocket.bRunTelnetEnabled)
		{
			if (!IP_ArpMac(csSocket.szDutIP,szDutMac))
			{
				api_status=4;
				CheckReturnError(api_status, "[Error] Failed to get dut mac.\n");			
				return api_status;
			}
		}

		ErrorCode=0;

		int nTelnetNum=6;
		bool IsTelnet=false;
		strSocketBuf.clear();
		while(nTelnetNum--)
		{
			if(csSocket.bRunTelnetEnabled && !IP_TelnetEnable(csSocket.szDutIP,szDutMac))
			{
				ErrorCode=5;
				CheckReturnError(api_status, "[Error] Failed to get dut mac.\n");
				continue;
			}
			printf("beging to Init Socket");
			if(InitSocket(csSocket.szDutIP,ErrorCode))
			{
				Sleep(1000);
				if(strSocketBuf.find(csSocket.szKeyWord))	
				{
					printf("socket connected!\n");
					IsTelnet=true;
					break;
				}
			}
			Sleep(500);
		}

		if(!IsTelnet)
		{
			api_status = 6;
			CheckReturnError(api_status, "[Error] Create socket return error.\n");
		}
		else
		{

		}

		// initialize the DUT at the very beginning

		char szCmd[BUFSIZE];
		char CmdPrefix[16]="wl -i eth2 ";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use
		// run these commands once.
		SendDutCmd(CmdPrefix,"down");
		SendDutCmd(CmdPrefix,"mpc 0");
		SendDutCmd(CmdPrefix,"up");
		SendDutCmd(CmdPrefix,"pkteng_stop tx");
		SendDutCmd(CmdPrefix,"pkteng_stop rx");
		SendDutCmd(CmdPrefix,"down");
		SendDutCmd(CmdPrefix,"country ALL");
		SendDutCmd(CmdPrefix,"mimo_bw_cap 1");
		SendDutCmd(CmdPrefix,"phy_percal 3");
		SendDutCmd(CmdPrefix,"phy_watchdog 0");
		SendDutCmd(CmdPrefix,"interference 0");
		SendDutCmd(CmdPrefix,"scansuppress 1");
		SendDutCmd(CmdPrefix,"glacial_timer 150000");
		SendDutCmd(CmdPrefix,"fast_timer 150000");
		SendDutCmd(CmdPrefix,"slow_timer 150000");
		SendDutCmd(CmdPrefix,"bi 60000");
		SendDutCmd(CmdPrefix,"up");
		SendDutCmd(CmdPrefix,"ssid \"\"");
		SendDutCmd(CmdPrefix,"txpwr1 -o -q 60");
		SendDutCmd(CmdPrefix,"down");

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

		char szCmd[256]="";
		double dBase = 2.0;

		SendDutCmd(CmdPrefix,"down");
		SendDutCmd(CmdPrefix,"mimo_preamble 1");
		SendDutCmd(CmdPrefix,"phy_percal 3");
		SendDutCmd(CmdPrefix,"phy_watchdog 0");
		SendDutCmd(CmdPrefix,"interference 0");
		SendDutCmd(CmdPrefix,"scansuppress 1");
		SendDutCmd(CmdPrefix,"tempsense_disable 1");
		SendDutCmd(CmdPrefix,"mimo_bw_cap 1");
		SendDutCmd(CmdPrefix,"up");
		SendDutCmd(CmdPrefix,"down");
		SendDutCmd(CmdPrefix,"band a");

		sprintf(szCmd,"chanspec %d/%d",g_channel-2,int(pow(dBase,g_CH_bandwidth+1)*10));
		SendDutCmd(CmdPrefix,"up");
		SendDutCmd(CmdPrefix,szCmd);
		
//# wl -i eth2 up
//# wl -i eth2 chanspec 40/80
		
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

		/*api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "NUM_STREAM_11AC", &g_num_streams_11ac);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(NUM_STREAM_11AC) return error.\n");*/
		
		// Here add my codes
		char szCmd[256]="";
		//printf("%s\n",g_wifi_data_rate);
		map<string,string>::iterator it_rate = g_acrates.find(g_wifi_data_rate);
		assert(it_rate != g_acrates.end());

		SendDutCmd(CmdPrefix,"5g_rate auto");
		SendDutCmd(CmdPrefix,"phy_forcecal 1");
		sprintf_s(szCmd,"%s" , it_rate->second.c_str());
		SendDutCmd(CmdPrefix,szCmd);


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
		/*api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "BSS_FREQ_MHZ_PRIMARY", &g_freqMHz_BSSPrimary);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(BSS_FREQ_MHZ_PRIMARY) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "BSS_FREQ_MHZ_SECONDARY", &g_freqMHz_BSSSecondary);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(BSS_FREQ_MHZ_SECONDARY) return error.\n");*/

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "CH_FREQ_MHZ", &g_freqMHz);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CH_FREQ_MHZ) return error.\n");

		/*api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "CH_FREQ_MHZ_PRIMARY_20MHz", &g_freqMHz_Primary_20MHz);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CH_FREQ_MHZ_PRIMARY_20MHz) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "CH_FREQ_MHZ_PRIMARY_40MHz", &g_freqMHz_Primary_40MHz);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CH_FREQ_MHZ_PRIMARY_40MHz) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "CH_FREQ_MHZ_PRIMARY_80MHz", &g_freqMHz_Primary_80MHz);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CH_FREQ_MHZ_PRIMARY_80MHz) return error.\n");*/

		// Here add by daixin
		::vDUT_WiFiConvertFrequencyToChannel(g_freqMHz,&g_channel);
		memset(CmdPrefix,'\0',sizeof(CmdPrefix));
		GetCmdPrefix(g_channel,CmdPrefix);
		// Add end.

		/*api_status = ::vDUT_GetStringParameter (g_LP_DUT_11ac_id, "PREAMBLE", g_preamble, MAX_BUFFER_SIZE);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(PREAMBLE) return error.\n");

		api_status = ::vDUT_GetStringParameter (g_LP_DUT_11ac_id, "PACKET_FORMAT", g_packet_format, MAX_BUFFER_SIZE);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(PACKET_FORMAT) return error.\n");*/
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

		// Here add my codes.
		char szCmd[256]="";
		SendDutCmd(CmdPrefix,"ssid \"\"");
		SendDutCmd(CmdPrefix,"phy_txpwrctrl 1");
		sprintf_s(szCmd,"txpwr1 -o -q %.1f", g_wifi_tx_power*4);
		SendDutCmd(CmdPrefix, szCmd);
		int nAntNum=g_currentAnt1+g_currentAnt2+g_currentAnt3+g_currentAnt4;

		switch (nAntNum)
		{
		case 1:
			if(g_currentAnt1)
			{
				SendDutCmd(CmdPrefix,"txcore -s 1 -c 0x1");	
			}
			else if(g_currentAnt2)
			{
				SendDutCmd(CmdPrefix,"txcore -s 1 -c 0x2");				
			}
			else if(g_currentAnt3)
			{
				SendDutCmd(CmdPrefix,"txcore -s 1 -c 0x4");	
			}
			else
			{
				SendDutCmd(CmdPrefix,"txcore -s 1 -c 0x8");	
			}
			break;
		case 2:
			//reseverd.
			break;
		case 3:
			//reseverd.
			break;
		case 4:
			//reseverd.
			break;
		default:
			break;
		}
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
		SendDutCmd(CmdPrefix,"plcphdr auto");
	   	SendDutCmd(CmdPrefix,"pkteng_start aa.bb.cc.dd.ee.ff tx sync 50 100 250");
		SendDutCmd(CmdPrefix,"pkteng_stop tx");
		SendDutCmd(CmdPrefix,"pkteng_start 00:00:00:c0:ff:ee tx 250 1740 0");
		//SendDutCmd(CmdPrefix,"pkteng_start aa.bb.cc.dd.ee.ff tx 100 1000 0");
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
		/*api_status = ::LP_EnableVsgRF(0);
		CheckReturnError(api_status, "[Error] LP_EnableVsgRF(0) return error.\n");*/

		// Here add my codes
		SendDutCmd(CmdPrefix,"pkteng_stop tx");
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

int LP_read_ini (char* pathStr )
{
// by LJ
 
	int  ini_status = 0 ;
	
	char iniPath[MAX_BUFFER_SIZE] = "";
	
	if (GetCurrentDirectory (MAX_BUFFER_SIZE, iniPath) != NULL)  
	{  
		strcat_s(iniPath,sizeof(iniPath), "\\");
		strcat_s(iniPath,sizeof(iniPath), LP_DUT_INI_FILE); 
	}
	else
	{
	   ini_status = 1;
	   return ini_status;
	}

	// check if the .ini exists
    FILE *iniFile;
	fopen_s(&iniFile, iniPath, "r");
	if (!iniFile)
	{   
		ini_status = 1;
		return ini_status;
	}
	else
	{
		// if the .ini exists
		int stringLength;
		fclose(iniFile);
		stringLength = GetPrivateProfileString("Configuration","DUT_IP", "192.168.1.1", csSocket.szDutIP, sizeof(csSocket.szDutIP), iniPath);
		stringLength = GetPrivateProfileString("Configuration","KEY_WORD", "#", csSocket.szKeyWord, sizeof(csSocket.szKeyWord), iniPath);
		stringLength = GetPrivateProfileString("Configuration","CMD_PREFIX_2G", "wl", csSocket.szCmdPrefix2g, sizeof(csSocket.szCmdPrefix2g), iniPath);
		stringLength = GetPrivateProfileString("Configuration","CMD_PREFIX_5G", "wl -i eth2", csSocket.szCmdPrefix5g, sizeof(csSocket.szCmdPrefix5g), iniPath);
		stringLength = GetPrivateProfileString("Configuration","CMD_PREFIX", "#", csSocket.szCmdPrefixReserved, sizeof(csSocket.szCmdPrefixReserved), iniPath);
		stringLength = GetPrivateProfileString("Configuration","Mod File Path", "Null", pathStr, MAX_BUFFER_SIZE, iniPath);
        csSocket.bRunTelnetEnabled = GetPrivateProfileInt("Configuration","RunTelnetEnable", false, iniPath);
		// check if the path ends up with "/" or "\"
        if ((pathStr[stringLength - 1]!='\\')&&(pathStr[stringLength - 1]!='/'))
		{
			strcat(pathStr, "\\");
			return ini_status;
          //  return pathStr;
		}
		else 
		{
			return ini_status;
		  // do nothing	
		}
	}

}
bool SetSolutionIndex(void)
{
	//int  ini_status = 0 ;
	char iniPath[MAX_BUFFER_SIZE] = "";
	
	if (GetCurrentDirectory (MAX_BUFFER_SIZE, iniPath) != NULL)  
	{  
		strcat_s(iniPath,sizeof(iniPath), "\\");
		strcat_s(iniPath,sizeof(iniPath), LP_DUT_INI_FILE); 
	}
	else
	{
	  // ini_status = 1;
	   return false;
	}

	// check if the .ini exists
    FILE *iniFile;
	fopen_s(&iniFile, iniPath, "r");
	if (!iniFile)
	{   		
		return false;
	}
	else
	{
		// if the .ini exists			
		SolutionIndex = GetPrivateProfileInt("Configuration","SolutionIndex",1,iniPath);	
		fclose(iniFile);
	}
	return true;
}
void GetCmdPrefix(int freq, char* cmdPrefix)
{

	if(freq < 14 )
	{
		if(SetSolutionIndex())
		{
			if( SolutionIndex == 1 )
			{
				//sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
				strcpy_s(cmdPrefix,sizeof(csSocket.szCmdPrefix2g)+1,csSocket.szCmdPrefix2g);
			}
			else if( SolutionIndex == 2 )
			{
				//sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
				strcpy_s(cmdPrefix,sizeof(csSocket.szCmdPrefix5g)+1,csSocket.szCmdPrefix5g);
			}
			else
			{
				//sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefixReserved);
				strcpy_s(cmdPrefix,sizeof(csSocket.szCmdPrefixReserved)+1,csSocket.szCmdPrefixReserved);
			}
		}
	}
	else
	{
		if(SetSolutionIndex())
		{
			if( SolutionIndex == 1 )
			{
				//sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
				strcpy_s(cmdPrefix,sizeof(csSocket.szCmdPrefix5g)+1,csSocket.szCmdPrefix5g);
			}
			else if( SolutionIndex == 2 )
			{
				//sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
				strcpy_s(cmdPrefix,sizeof(csSocket.szCmdPrefix2g)+1,csSocket.szCmdPrefix2g);
			}
			else
			{
				//sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefixReserved);
				strcpy_s(cmdPrefix,sizeof(csSocket.szCmdPrefixReserved)+1,csSocket.szCmdPrefixReserved);
			}
		}
	}		
}
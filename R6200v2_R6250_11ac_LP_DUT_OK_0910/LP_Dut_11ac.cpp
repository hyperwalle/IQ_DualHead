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
#include <fstream>	// ifstream / ofstream type
#include "vDUT.h"	// vDUT_WiFiConvertFrequencyToChannel
#include "assert.h" // assert
#include "shlwapi.h" // PathFileExists()
using namespace std;

// global variables
extern SOCKET g_socket;
extern string strSocketBuf;
extern HANDLE h_socketThread;
extern bool ShowDebugInfor;
extern bool bRequestExit;

int SolutionIndex = 1;
char CmdPrefix[16]="wl";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use
int g_channel=-1;
int g_nBandWidth=0;
int rx_start=0, rx_end=0;
map <string,string> g_rates,g_5grates,g_acrates;
/*Define struct mark same dut parameter*/
struct DisableWiFi
{
  char szCmd1[256];
  char szCmd2[256];
  char szCmd3[256];
  char szCmd4[256];
  char szCmd5[256];
}stDisableWiFi;
/*Define struct mark same dut parameter*/
struct SameDut
{
	int			nOldfreq ;
	int			nOldBandWidth;	
	string		szOldDataRate;	
}IsSameDut={-1,-1,""},RxSameDut={-1,-1,""};
/*Define a static parameter mark the same dut*/
static bool	bOldSameDut = false;
/*End*/		
bool bSocketConnected=false;
struct 
{
	char szDutIP[24];
	char szKeyWord[128];
	char szCmdPrefix2g[16];
	char szCmdPrefix5g[16];
	char szCmdPrefixReserved[16];
	bool bRunTelnetEnabled;
	char szUsbHwID[64];
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
const char  *g_LP_Dut_Version                   = "2.5.2 (2012-08-19)\n";
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
	// 2g
	g_rates.clear();
	g_rates.insert( pair<string, string>("DSSS-1",   "-r 1") );
	g_rates.insert( pair<string, string>("DSSS-2",   "-r 2") );
	g_rates.insert( pair<string, string>("CCK-5_5",  "-r 5.5") );
	g_rates.insert( pair<string, string>("CCK-11",   "-r 11") );
	g_rates.insert( pair<string, string>("PBCC-22",  "-r 22 ") );
	g_rates.insert( pair<string, string>("OFDM-6",   "-r 6 ") );
	g_rates.insert( pair<string, string>("OFDM-9",   "-r 9 ") );
	g_rates.insert( pair<string, string>("OFDM-12",  "-r 12 ") );
	g_rates.insert( pair<string, string>("OFDM-18",  "-r 18 ") );
	g_rates.insert( pair<string, string>("OFDM-24",  "-r 24 ") );
	g_rates.insert( pair<string, string>("OFDM-36", "-r 36 ") );
	g_rates.insert( pair<string, string>("OFDM-48", "-r 48 ") );
	g_rates.insert( pair<string, string>("OFDM-54", "-r 54 ") );
	g_rates.insert( pair<string, string>("MCS0",    "-m 0 ") );
	g_rates.insert( pair<string, string>("MCS1",    "-m 1 ") );
	g_rates.insert( pair<string, string>("MCS2",    "-m 2 ") );
	g_rates.insert( pair<string, string>("MCS3",    "-m 3 ") );
	g_rates.insert( pair<string, string>("MCS4",    "-m 4 ") );
	g_rates.insert( pair<string, string>("MCS5",    "-m 5 ") );
	g_rates.insert( pair<string, string>("MCS6",    "-m 6 ") );
	g_rates.insert( pair<string, string>("MCS7",    "-m 7 ") );
	g_rates.insert( pair<string, string>("MCS8",    "-m 8 ") );
	g_rates.insert( pair<string, string>("MCS9",    "-m 9 ") );
	g_rates.insert( pair<string, string>("MCS10",   "-m 10 ") );
	g_rates.insert( pair<string, string>("MCS11",   "-m 11 ") );
	g_rates.insert( pair<string, string>("MCS12",   "-m 12 ") );
	g_rates.insert( pair<string, string>("MCS13",   "-m 13 ") );
	g_rates.insert( pair<string, string>("MCS14",   "-m 14 ") );
	g_rates.insert( pair<string, string>("MCS15",   "-m 15 ") );
	g_rates.insert( pair<string, string>("MCS16",   "-m 16 ") );
	g_rates.insert( pair<string, string>("MCS17",   "-m 17 ") );
	g_rates.insert( pair<string, string>("MCS18",   "-m 18 ") );
	g_rates.insert( pair<string, string>("MCS19",   "-m 19 ") );
	g_rates.insert( pair<string, string>("MCS20",   "-m 20 ") );
	g_rates.insert( pair<string, string>("MCS20",   "-m 21 ") );
	g_rates.insert( pair<string, string>("MCS22",   "-m 22 ") );
	g_rates.insert( pair<string, string>("MCS23",   "-m 23 ") );

	// 5g 11n
	g_5grates.clear();
	g_5grates.insert( pair<string, string>("DSSS-1",   "-r 1 -b 20") );
	g_5grates.insert( pair<string, string>("DSSS-2",   "-r 2 -b 20") );
	g_5grates.insert( pair<string, string>("CCK-5_5",  "-r 5.5 -b 20") );
	g_5grates.insert( pair<string, string>("CCK-11",   "-r 11 -b 20") );
	g_5grates.insert( pair<string, string>("PBCC-22",  "-r 22 -b 20") );
	g_5grates.insert( pair<string, string>("OFDM-6",   "-r 6 -b 20") );
	g_5grates.insert( pair<string, string>("OFDM-9",   "-r 9 -b 20") );
	g_5grates.insert( pair<string, string>("OFDM-12",  "-r 12 -b 20") );
	g_5grates.insert( pair<string, string>("OFDM-18",  "-r 18 -b 20") );
	g_5grates.insert( pair<string, string>("OFDM-24",  "-r 24 -b 20") );
	g_5grates.insert( pair<string, string>("OFDM-36", "-r 36 -b 20") );
	g_5grates.insert( pair<string, string>("OFDM-48", "-r 48 -b 20") );
	g_5grates.insert( pair<string, string>("OFDM-54", "-r 54 -b 20") );
	g_5grates.insert( pair<string, string>("MCS0",    "-h 0 -b %d") );
	g_5grates.insert( pair<string, string>("MCS1",    "-h 1 -b %d") );
	g_5grates.insert( pair<string, string>("MCS2",    "-h 2 -b %d") );
	g_5grates.insert( pair<string, string>("MCS3",    "-h 3 -b %d") );
	g_5grates.insert( pair<string, string>("MCS4",    "-h 4 -b %d") );
	g_5grates.insert( pair<string, string>("MCS5",    "-h 5 -b %d") );
	g_5grates.insert( pair<string, string>("MCS6",    "-h 6 -b %d") );
	g_5grates.insert( pair<string, string>("MCS7",    "-h 7 -b %d") );
	g_5grates.insert( pair<string, string>("MCS8",    "-h 8 -b %d") );
	g_5grates.insert( pair<string, string>("MCS9",    "-h 9 -b %d") );
	g_5grates.insert( pair<string, string>("MCS10",   "-h 10 -b %d") );
	g_5grates.insert( pair<string, string>("MCS11",   "-h 11 -b %d") );
	g_5grates.insert( pair<string, string>("MCS12",   "-h 12 -b %d") );
	g_5grates.insert( pair<string, string>("MCS13",   "-h 13 -b %d") );
	g_5grates.insert( pair<string, string>("MCS14",   "-h 14 -b %d") );
	g_5grates.insert( pair<string, string>("MCS15",   "-h 15 -b %d") );
	g_5grates.insert( pair<string, string>("MCS16",   "-h 16 -b %d") );
	g_5grates.insert( pair<string, string>("MCS17",   "-h 17 -b %d") );
	g_5grates.insert( pair<string, string>("MCS18",   "-h 18 -b %d") );
	g_5grates.insert( pair<string, string>("MCS19",   "-h 19 -b %d") );
	g_5grates.insert( pair<string, string>("MCS20",   "-h 20 -b %d") );
	g_5grates.insert( pair<string, string>("MCS20",   "-h 21 -b %d") );
	g_5grates.insert( pair<string, string>("MCS22",   "-h 22 -b %d") );
	g_5grates.insert( pair<string, string>("MCS23",   "-h 23 -b %d") );
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
		
#ifdef __CARD__
		// add code to disable wifi card here. 
		// for usb device. A6200
		char szCmd[256]="";
		sprintf_s(szCmd,sizeof(szCmd),"devctl.exe dis %s",csSocket.szUsbHwID);
		SendDutCmd("szCmd",": Disabled",5000);
#endif
#ifdef __AP__

		char iniPath[MAX_BUFFER_SIZE] = "";	 
		if (GetCurrentDirectory (MAX_BUFFER_SIZE, iniPath) != NULL)  
		{  
			strcat_s(iniPath,sizeof(iniPath), "\\");
			strcat_s(iniPath,sizeof(iniPath), LP_DUT_INI_FILE); 
		}

		// add here to burn HH sn
		if(PathFileExists(".\\HH.txt"))
		{
			char szHH[128]="";
			GetPrivateProfileString("DUTTE_HH","TE_HH","N/A",szHH,sizeof(szHH),".\\HH.txt");
			if(strcmp(szHH,"N/A") != 0)
			{
				char szLocalCmd[128]="";
				sprintf_s(szLocalCmd,sizeof(szLocalCmd),"nvram set TE_HH=%s;nvram commit",szHH);
				SendDutCmd("",szLocalCmd);
			}		
			DeleteFile(".\\HH.txt");
		}

		// add end

		char szDisablewf[4]="";
		char szDisablewfABA[4]="";
		GetPrivateProfileString("Configuration","Disable_WiFi","0", szDisablewf,sizeof(szDisablewf),iniPath);
		GetPrivateProfileString("Configuration","Disable_WiFi1","0", szDisablewfABA,sizeof(szDisablewfABA),iniPath);
		if(strstr(szDisablewf,"1") && strstr(szDisablewfABA,"1"))
		{
			char szCmdFile[256]=".\\DisWiFi.txt";
			
			ifstream is_cmd;
			is_cmd.open(szCmdFile,ios_base::in);
			while(!is_cmd.eof())
			{
				char szCmdLine[256]="";
				is_cmd.getline(szCmdLine,sizeof(szCmdLine));
				SendDutCmd("",szCmdLine); // send cmd one by one.				
			}
			is_cmd.close();
		} 
		//End
		if(h_socketThread != NULL)
		{			
			TerminateThread(h_socketThread,0);
		}

		bRequestExit = true;
		SendSocketCmd("wl out\r\n","",500);
		SendSocketCmd("wl -i eth2 out\r\n","",500);
		SendSocketCmd("exit\r\n","",500);
		//bOldSameDut = false;
		IsSameDut.nOldBandWidth = -1;
		IsSameDut.nOldfreq = -1;
		IsSameDut.szOldDataRate = "";
		RxSameDut.nOldfreq=-1;
		bSocketConnected=false;
		//g_LP_mimo_id = -1;	
		
#endif
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

		//// new add
		char pathStr[MAX_BUFFER_SIZE];
		char szCmd[BUFSIZE];
		api_status=LP_read_ini(pathStr);
		CheckReturnError(api_status, "[Error] LP_read_ini(pathStr) return error.\n");
#ifdef __AP__
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
		bSocketConnected=false;
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
					bSocketConnected=true;
					break;
				}
			}
			Sleep(500);
		}

		if(!bSocketConnected)
		{
			api_status = 6;
			CheckReturnError(api_status, "[Error] Create socket return error.\n");
		}
		else
		{

		}
		//initialize the DUT at the very beginning

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
		SendDutCmd(CmdPrefix,"txpwr1 -o 0x1");
		SendDutCmd(CmdPrefix,"down");
		SendDutCmd(CmdPrefix,"pkteng_stop tx");
		SendDutCmd(CmdPrefix,"txbf 0");
		SendDutCmd(CmdPrefix,"radio on");
		// initialize 2.4g
		SendDutCmd("","wl mpc 0");
		SendDutCmd("","wl pkteng_stop tx"); 
		SendDutCmd("","wl country ALL"); 
		SendDutCmd("","wl glacial_timer 150000"); 
		SendDutCmd("","wl fast_timer 150000"); 
		SendDutCmd("","wl slow_timer 150000"); 
		SendDutCmd("","wl bi 60000"); 
		SendDutCmd("","wl down");
		SendDutCmd("","wl mimo_preamble 0");
		SendDutCmd("","wl phy_percal 3");
		SendDutCmd("","wl phy_watchdog 0");
		SendDutCmd("","wl interference 0");
		SendDutCmd("","wl scansuppress 1");
		SendDutCmd("","wl tempsense_disable 1");
		SendDutCmd("","wl mimo_bw_cap 1");
		SendDutCmd("","wl band b");
		SendDutCmd("","wl up");
		SendDutCmd("","wl chanspec -c 7 -b 2 -w 20 -s 0");
		SendDutCmd("","wl mimo_txbw 2");
		SendDutCmd("","wl phy_forcecal 1");
		SendDutCmd("","wl nrate -r 54");
		SendDutCmd("","wl ssid \"\"");
		SendDutCmd("","wl phy_txpwrctrl 1");
		SendDutCmd("","wl txpwr1 -o -q 60");
		SendDutCmd("","wl txcore -o 0x1");
		SendDutCmd("","wl plcphdr auto");
		SendDutCmd("","wl radio on");
		//SendDutCmd("","wl txbf 0");
		
#else

		
		// Add code to enable wifi card.
		//1. Check the device existence
		printf("Device HWID: %s\n",csSocket.szUsbHwID);
		if(!SendDutCmd("devctl.exe list usb",csSocket.szUsbHwID,10000))
		{
			printf("\nFailed to found the usb device\n");
			return -1; // failed to found the device.
		}
		//2. Enable usb device
		sprintf_s(szCmd,sizeof(szCmd),"devctl.exe en %s",csSocket.szUsbHwID);
		if(!SendDutCmd(szCmd,": Enabled",10000))
		{
			printf("\nFailed to enable the usb device\n");
			return -1; // failed to enable the usb device
		}


		//char CmdPrefix[16]="wl ";//2G and 5G command use the same prefix
		// run these commands once.
		SendDutCmd("wl down");
		SendDutCmd("wl mpc 0");
		SendDutCmd("wl up");
		SendDutCmd("wl pkteng_stop tx");
		SendDutCmd("wl pkteng_stop rx");
		SendDutCmd("wl down");
		SendDutCmd("wl country ALL");
		SendDutCmd("wl wsec 0");
		SendDutCmd("wl obss_coex 0");
		SendDutCmd("wl stbc_tx 0");
		SendDutCmd("wl stbc_rx 0");
		SendDutCmd("wl band auto");
		SendDutCmd("wl txpwr1 -1");
		SendDutCmd("wl spect 0");
		SendDutCmd("wl ibss_gmode -1");
		SendDutCmd("wl mimo_bw_cap 1");
		SendDutCmd("wl frameburst 1");
		SendDutCmd("wl ampdu 0");
		SendDutCmd("wl txchain 0x3");
		SendDutCmd("wl rxchain 0x3");
		SendDutCmd("wl up");
		SendDutCmd("wl PM 0");
		SendDutCmd("wl lrl 4");
		SendDutCmd("wl srl 7");
		SendDutCmd("wl ver");
#endif
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

		if(g_channel<=14) // 2g
		{
			if(IsSameDut.nOldfreq > 14) // last items is 5g, we need send the test command.
			{
				SendDutCmd(CmdPrefix,"down");
				SendDutCmd(CmdPrefix,"mpc 0");
				SendDutCmd(CmdPrefix,"down");
				SendDutCmd(CmdPrefix,"mimo_preamble 0");
				SendDutCmd(CmdPrefix,"phy_percal 3");
				SendDutCmd(CmdPrefix,"phy_watchdog 0");
				SendDutCmd(CmdPrefix,"interference 0");
				SendDutCmd(CmdPrefix,"scansuppress 1");
				SendDutCmd(CmdPrefix,"tempsense_disable 1");
				SendDutCmd(CmdPrefix,"mimo_bw_cap 1");
				SendDutCmd(CmdPrefix,"up");
			}

			SendDutCmd(CmdPrefix,"down");
			SendDutCmd(CmdPrefix,"band b");
			SendDutCmd(CmdPrefix,"up");
			//SendDutCmd(CmdPrefix,"chanspec -c 11 -b 2 -w 20 -s 0");
			//SendDutCmd(CmdPrefix,"mimo_txbw 2");

			g_nBandWidth = int(pow(dBase,g_CH_bandwidth+1)*10); // nBandWidth is global
			sprintf_s(szCmd,sizeof(szCmd),"chanspec -c %d -b 2 -w %d -s %d",g_channel,g_nBandWidth,(g_nBandWidth/40));
			SendDutCmd(CmdPrefix,szCmd);
			if(g_nBandWidth == 20)
				SendDutCmd(CmdPrefix,"mimo_txbw 2");
			else
				SendDutCmd(CmdPrefix,"mimo_txbw 4");
		}
		else // 5g
		{		
			//SendDutCmd(CmdPrefix,"mpc 0");
			if(IsSameDut.nOldfreq < 36 && IsSameDut.nOldfreq > 0)
			{
				SendDutCmd(CmdPrefix,"down");
				SendDutCmd(CmdPrefix,"phy_percal 3");
				SendDutCmd(CmdPrefix,"phy_watchdog 0");
				SendDutCmd(CmdPrefix,"interference 0");
				SendDutCmd(CmdPrefix,"scansuppress 1");
				SendDutCmd(CmdPrefix,"tempsense_disable 1");
				SendDutCmd(CmdPrefix,"mimo_bw_cap 1");
				SendDutCmd(CmdPrefix,"up");
			}
			
			SendDutCmd(CmdPrefix,"down");

			g_nBandWidth = int(pow(dBase,g_CH_bandwidth+1)*10); // nBandWidth is global
			SendDutCmd(CmdPrefix,"band a");
			SendDutCmd(CmdPrefix,"up");
			if(g_nBandWidth >= 40)
			{
				sprintf_s(szCmd,sizeof(szCmd),"chanspec %d/%d",g_channel-2,int(pow(dBase,g_CH_bandwidth+1)*10));
			}
			else
			{
				sprintf_s(szCmd,sizeof(szCmd),"chanspec %d/%d",g_channel,int(pow(dBase,g_CH_bandwidth+1)*10));
			}
			SendDutCmd(CmdPrefix,szCmd);
		}
		IsSameDut.nOldfreq = g_channel;

		/*if(g_nBandWidth == 20)
			SendDutCmd(CmdPrefix,"mimo_txbw 2");
		else
			SendDutCmd(CmdPrefix,"mimo_txbw 4");*/

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
		/*map<string,string>::iterator it_rate = g_acrates.find(g_wifi_data_rate);
		assert(it_rate != g_acrates.end());*/

		if(g_channel<=14)
		{
			map<string,string>::iterator it_rate = g_rates.find(g_wifi_data_rate);
			assert(it_rate != g_rates.end());

			//SendDutCmd(CmdPrefix,"2g_rate auto");
			//SendDutCmd(CmdPrefix,"phy_forcecal 1");
			sprintf_s(szCmd,sizeof(szCmd),"nrate %s" , it_rate->second.c_str());
			//sprintf_s(szCmd,sizeof(szCmd),szCmd,g_nBandWidth);
			SendDutCmd(CmdPrefix,szCmd);	
		}
		else
		{
			if(g_nBandWidth == 80)
			{
				map<string,string>::iterator it_rate = g_acrates.find(g_wifi_data_rate);
				assert(it_rate != g_acrates.end());

				SendDutCmd(CmdPrefix,"5g_rate auto");
				SendDutCmd(CmdPrefix,"phy_forcecal 1");
				sprintf_s(szCmd,sizeof(szCmd),"%s" , it_rate->second.c_str());
				SendDutCmd(CmdPrefix,szCmd);				
			}
			else	// bandwidth == 40 or 20
			{
				map<string,string>::iterator it_rate = g_5grates.find(g_wifi_data_rate);
				assert(it_rate != g_rates.end());

				SendDutCmd(CmdPrefix,"5g_rate auto");
				SendDutCmd(CmdPrefix,"phy_forcecal 1");
				sprintf_s(szCmd,sizeof(szCmd),"5g_rate %s" , it_rate->second.c_str());
				sprintf_s(szCmd,sizeof(szCmd),szCmd,g_nBandWidth);
				SendDutCmd(CmdPrefix,szCmd);				
			}
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

		strset(szCmd,'\0');
		sprintf_s(szCmd,"txpwr1 -o -q %.1f", g_wifi_tx_power*4);
		SendDutCmd(CmdPrefix, szCmd);

		int nAntNo=-1;
		int nAntNum=g_currentAnt1+g_currentAnt2+g_currentAnt3+g_currentAnt4;
		if(g_channel <= 14) // 2g
		{
			SendDutCmd(CmdPrefix,"txchain 3");
			SendDutCmd(CmdPrefix,"down");
			/*SendDutCmd(CmdPrefix,"rxchain 0x1
			SendDutCmd(CmdPrefix,"up");
			SendDutCmd(CmdPrefix,"txchain 0x1");*/
			switch (nAntNum)
			{
			case 1:
				if(g_currentAnt1)
				{
					SendDutCmd(CmdPrefix,"rxchain 0x1");
					SendDutCmd(CmdPrefix,"up");
					SendDutCmd(CmdPrefix,"txchain 0x1");
				}
				else if(g_currentAnt2)
				{
					SendDutCmd(CmdPrefix,"rxchain 0x2");
					SendDutCmd(CmdPrefix,"up");
					SendDutCmd(CmdPrefix,"txchain 0x2");				
				}
				else
				{
					;
				}
				break;
			case 2:
				SendDutCmd(CmdPrefix,"rxchain 0x3");
				SendDutCmd(CmdPrefix,"up");
				SendDutCmd(CmdPrefix,"txchain 0x3");
				break;
			default:
				break;
			}	
			SendDutCmd(CmdPrefix,"plcphdr long");
		}
		else // 5g
		{
			if(strstr(g_wifi_data_rate,"MCS"))
			{
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
					nAntNo = GetAntCombineNum(g_currentAnt1,g_currentAnt2,g_currentAnt3,g_currentAnt4);
					sprintf_s(szCmd,sizeof(szCmd),"txcore -s 2 -c 0x%d",nAntNo);
					SendDutCmd(CmdPrefix,szCmd);
					//reseverd.
					break;
				case 3:
					nAntNo = GetAntCombineNum(g_currentAnt1,g_currentAnt2,g_currentAnt3,g_currentAnt4);
					sprintf_s(szCmd,sizeof(szCmd),"txcore -s 3 -c 0x%d",nAntNo);
					SendDutCmd(CmdPrefix,szCmd);
					//reseverd.
					break;
				case 4:
					nAntNo = GetAntCombineNum(g_currentAnt1,g_currentAnt2,g_currentAnt3,g_currentAnt4);
					sprintf_s(szCmd,sizeof(szCmd),"txcore -s 4 -c 0x%d",nAntNo);
					SendDutCmd(CmdPrefix,szCmd);
					//reseverd.
					break;
				default:
					break;
				}		
			}
			else if(strstr(g_wifi_data_rate,"OFDM"))	// OFDM, DSSS, support one ant only.
			{
				switch (nAntNum)
				{
				case 1:
					if(g_currentAnt1)
					{					
						SendDutCmd(CmdPrefix,"txcore -o 0x1");	
					}
					else if(g_currentAnt2)
					{
						SendDutCmd(CmdPrefix,"txcore -o 0x2");				
					}
					else if(g_currentAnt3)
					{
						SendDutCmd(CmdPrefix,"txcore -o 0x4");	
					}
					else
					{
						SendDutCmd(CmdPrefix,"txcore -o 0x8");	
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
			else	// CCK, DSSS, support one ant only.
			{
				switch (nAntNum)
				{
				case 1:
					if(g_currentAnt1)
					{					
						SendDutCmd(CmdPrefix,"txcore -k 0x1");	
					}
					else if(g_currentAnt2)
					{
						SendDutCmd(CmdPrefix,"txcore -k 0x2");				
					}
					else if(g_currentAnt3)
					{
						SendDutCmd(CmdPrefix,"txcore -k 0x4");	
					}
					else
					{
						SendDutCmd(CmdPrefix,"txcore -k 0x8");	
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
			SendDutCmd(CmdPrefix,"plcphdr auto");
		}
		

		/*if(g_channel <= 14)
			SendDutCmd(CmdPrefix,"plcphdr long");
		else
			SendDutCmd(CmdPrefix,"plcphdr auto");*/

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
		//SendDutCmd(CmdPrefix,"plcphdr auto");
		if(g_channel <= 14)
		{
			SendDutCmd(CmdPrefix,"phy_forcecal 1");
			SendDutCmd(CmdPrefix,"pkteng_start aa.bb.cc.dd.ee.ff tx sync 50 100 250");
			SendDutCmd(CmdPrefix,"pkteng_stop tx");
			SendDutCmd(CmdPrefix,"pkteng_start aa.bb.cc.dd.ee.ff tx 100 1000 0");
		}
		else
		{
			SendDutCmd(CmdPrefix,"pkteng_start aa.bb.cc.dd.ee.ff tx sync 50 100 30");
			SendDutCmd(CmdPrefix,"pkteng_stop tx");
			SendDutCmd(CmdPrefix,"pkteng_start 00:00:00:c0:ff:ee tx 30 1740 0");		
		}

		//SendDutCmd(CmdPrefix,"pkteng_start 00:00:00:c0:ff:ee tx 250 1740 0");
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
		if(bSocketConnected)
		{
			SendDutCmd(CmdPrefix,"pkteng_stop tx");
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
		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "BSS_BANDWIDTH", &g_BSS_bandwidth);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(BSS_BANDWIDTH) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "CH_BANDWIDTH", &g_CH_bandwidth);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CH_BANDWIDTH) return error.\n");

		if((RxSameDut.nOldfreq == -1) || (RxSameDut.nOldfreq < 36 && g_channel >= 36) || (RxSameDut.nOldfreq >= 36 && g_channel <= 14 ))
		{
			// do nothing
			SendDutCmd(CmdPrefix,"down");
			SendDutCmd(CmdPrefix,"phy_percal 3");
			SendDutCmd(CmdPrefix,"phy_watchdog 0");
			SendDutCmd(CmdPrefix,"interference 0");
			SendDutCmd(CmdPrefix,"scansuppress 1");
			SendDutCmd(CmdPrefix,"tempsense_disable 1");
			SendDutCmd(CmdPrefix,"mimo_bw_cap 1");
			SendDutCmd(CmdPrefix,"up");
			SendDutCmd(CmdPrefix,"down");
		}
		RxSameDut.nOldfreq = g_channel; // record the channel of last test items.

		char szCmd[256]="";
		double dBase = 2.0;
		g_nBandWidth = int(pow(dBase,g_CH_bandwidth+1)*10); // nBandWidth is global
		if(g_channel<=14)
		{
			SendDutCmd(CmdPrefix,"band b");
			SendDutCmd(CmdPrefix,"up");
			sprintf(szCmd,"chanspec -c %d -b 2 -w %d -s %d",g_channel,g_nBandWidth,g_nBandWidth/40);
			SendDutCmd(CmdPrefix,szCmd);
			if(g_nBandWidth == 20)
			{
				SendDutCmd(CmdPrefix,"mimo_txbw 2");
			}
			else
			{
				SendDutCmd(CmdPrefix,"mimo_txbw 4");
			}
			//SendDutCmd(CmdPrefix,"2g_rate auto");
		}
		else
		{
			SendDutCmd(CmdPrefix,"band a");
			SendDutCmd(CmdPrefix,"up");
			if(g_nBandWidth >= 40)
			{
				sprintf(szCmd,"chanspec %d/%d",g_channel-2,int(pow(dBase,g_CH_bandwidth+1)*10));
			}
			else
			{
				sprintf(szCmd,"chanspec %d/%d",g_channel,int(pow(dBase,g_CH_bandwidth+1)*10));
			}
			SendDutCmd(CmdPrefix,szCmd);
			SendDutCmd(CmdPrefix,"5g_rate auto");
			SendDutCmd(CmdPrefix,"down");
		}
		

		// Here add my codes. SET_ANTENNA
		//char szCmd[256]="";
		int nAntNo=-1;
		int nAntNum=g_currentAnt1+g_currentAnt2+g_currentAnt3+g_currentAnt4;
		if(g_channel <= 14)
		{
			switch (nAntNum)
			{
			case 1:
				if(g_currentAnt1)
				{
					SendDutCmd(CmdPrefix,"phy_enrxcore 0x1");
					SendDutCmd(CmdPrefix,"rxchain 0x1");	
				}
				else if(g_currentAnt2)
				{
					SendDutCmd(CmdPrefix,"phy_enrxcore 0x2");
					SendDutCmd(CmdPrefix,"rxchain 0x2");				
				}
				break;
			case 2:
				SendDutCmd(CmdPrefix,"phy_enrxcore 0x3");
				SendDutCmd(CmdPrefix,"rxchain 0x3");	
				//reseverd.
				break;
			default:
				break;
			}		

			SendDutCmd(CmdPrefix,"down");
			SendDutCmd(CmdPrefix,"phy_forcecal 1");	
			SendDutCmd(CmdPrefix,"mimo_bw_cap 1");
			SendDutCmd(CmdPrefix,"up");

		}
		else
		{
			switch (nAntNum)
			{
			case 1:
				if(g_currentAnt1)
				{
					SendDutCmd(CmdPrefix,"rxchain 0x1");	
				}
				else if(g_currentAnt2)
				{
					SendDutCmd(CmdPrefix,"rxchain 0x2");				
				}
				else if(g_currentAnt3)
				{
					SendDutCmd(CmdPrefix,"rxchain 0x3");	
				}
				else
				{
					SendDutCmd(CmdPrefix,"rxchain 0x4");	
				}
				break;			
			default:
				break;
			}		

			SendDutCmd(CmdPrefix,"up");
			SendDutCmd(CmdPrefix,"phy_forcecal 1");
		}
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
		api_status = ::vDUT_GetStringParameter(g_LP_DUT_11ac_id, "DATA_RATE", g_wifi_data_rate, MAX_BUFFER_SIZE);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(DATA_RATE) return error.\n");

		/*api_status = ::vDUT_GetIntegerParameter(g_LP_DUT_11ac_id, "NUM_STREAM_11AC", &g_num_streams_11ac);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(NUM_STREAM_11AC) return error.\n");*/
		
		// Here add my codes
		char szCmd[256]="";
		//printf("%s\n",g_wifi_data_rate);
		/*map<string,string>::iterator it_rate = g_acrates.find(g_wifi_data_rate);
		assert(it_rate != g_acrates.end());*/

		if(g_channel<=14)
		{
			map<string,string>::iterator it_rate = g_rates.find(g_wifi_data_rate);
			assert(it_rate != g_rates.end());

			//SendDutCmd(CmdPrefix,"2g_rate auto");
			/*sprintf_s(szCmd,sizeof(szCmd),"2g_rate %s" , it_rate->second.c_str());
			sprintf_s(szCmd,sizeof(szCmd),szCmd,g_nBandWidth);
			SendDutCmd(CmdPrefix,szCmd);*/	
		}
		else
		{
			SendDutCmd(CmdPrefix,"5g_rate auto");
			//if(g_nBandWidth == 80)
			//{
			//	map<string,string>::iterator it_rate = g_acrates.find(g_wifi_data_rate);
			//	assert(it_rate != g_acrates.end());

			//	SendDutCmd(CmdPrefix,"wl 5g_rate auto");
			//	sprintf_s(szCmd,sizeof(szCmd),"wl %s" , it_rate->second.c_str());
			//	SendDutCmd(CmdPrefix,szCmd);				
			//}
			//else	// bandwidth == 40 or 20
			//{
			//	map<string,string>::iterator it_rate = g_rates.find(g_wifi_data_rate);
			//	assert(it_rate != g_rates.end());

			//	SendDutCmd(CmdPrefix,"wl 5g_rate auto");
			//	sprintf_s(szCmd,sizeof(szCmd),"wl 5g_rate %s" , it_rate->second.c_str());
			//	sprintf_s(szCmd,sizeof(szCmd),szCmd,g_nBandWidth);
			//	SendDutCmd(/*CmdPrefix,*/szCmd);				
			//}
		}
		//SendDutCmd(/*CmdPrefix,*/"wl down");

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

		SendDutCmd(CmdPrefix,"counters");

		int index = (int)strSocketBuf.find("rxdfrmmcast");

		if(index == -1)
		{
			rx_start = 0;
		}
		else
		{
			char c[MAX_BUFFER_SIZE] = "";
			const size_t size =  strlen("rxdfrmmcast ");
			for(size_t j = 0, i = index+size; strSocketBuf[i] != ' ' ;++i,++j)
			{
				c[j] = strSocketBuf[i];
			}
			rx_start = atoi(c);
		}

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
		SendDutCmd(CmdPrefix,"counters");

		int index = (int)strSocketBuf.find("rxdfrmmcast");

		if(index == -1)
		{
			rx_end = 0;
		}
		else
		{
			char c[MAX_BUFFER_SIZE] = "";
			const size_t size =  strlen("rxdfrmmcast ");
			for(size_t j = 0, i = index+size; strSocketBuf[i] != ' ' ;++i,++j)
			{
				c[j] = strSocketBuf[i];
			}
			rx_end = atoi(c);
		}

		cntOk = rx_end - rx_start;
		/*SendDutCmd("wl pkteng_stop rx");
		SendDutCmd("wl rssi");
		Sleep(100);
		rssi = atoi(strSocketBuf.c_str());*/

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
		//SendDutCmd("wl pkteng_start 00:00:00:C0:FF:EE rxwithack");
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
		//SendDutCmd("","wl down");
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
		stringLength = GetPrivateProfileString("Configuration","USB_HWID", "", csSocket.szUsbHwID, MAX_BUFFER_SIZE, iniPath);
		stringLength = GetPrivateProfileString("Configuration","Mod File Path", "Null", pathStr, MAX_BUFFER_SIZE, iniPath);
        csSocket.bRunTelnetEnabled = GetPrivateProfileInt("Configuration","RunTelnetEnable", false, iniPath);

		// check if the usb device id is null.
		if(csSocket.szUsbHwID[0]=='\0')
		{
			ini_status = 1;
			printf("USB device HWID is not set in LP_DUT_setup.ini\n");
			return ini_status;
		}
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

	if(freq <= 14 )
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
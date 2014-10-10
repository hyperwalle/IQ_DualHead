// LP_Dut.cpp : Defines the entry point for the DLL application.
//
#include <string.h>
#include "stdafx.h"
#include "vDUT.h"
#include "IQmeasure.h"
#include "IQlite_Logger.h"
#include "LP_Dut.h"
#include "windows.h"
#include "PeerSocket.h"  // for DUT control
#include <string>
#include <cassert>
#include "shlwapi.h" // PathFileExists()
using namespace std;

// global variables
extern SOCKET g_socket;
extern string strSocketBuf;
extern HANDLE h_socketThread;
extern bool ShowDebugInfor;

char szDisablewf[256];
char szDisablewf1[256];
struct DisableWiFi
{
  char szCmd1[256];
  char szCmd2[256];
  char szCmd3[256];
  char szCmd4[256];
}stDisableWiFi;
/*Define struct mark same dut parameter*/
struct SameDut
{
	int			nOldfreq ;
	int			nOldBandWidth;	
	string		szOldDataRate;	
}IsSameDut={-1,-1,""};

/*Define a static parameter mark the same dut*/
static bool	bOldSameDut = false;
/*End*/		

int rx_start = 0;
int rx_end = 0;

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

#ifdef _MANAGED
#pragma managed(pop)
#endif

struct 
{
	char szDutIP[24];
	char szKeyWord[128];
	char szCmdPrefix2g[16];
	char szCmdPrefix5g[16];
	char szCmdPrefixReserved[16];
	int nCheckWiFiStatus;
	int nCheckWiFiSleep;
	char szCmdCheckWiFi[32];
	char szKeyCheckWiFi[32];
}csSocket;


#define BUFSIZE 256
// Global variables
int          g_LP_connected = 0;
int          g_logger_id   = -1;
vDUT_ID      g_LP_mimo_id  = -1;
const char  *g_LP_Dut_Version = "Foxconn WiFi DUT Control version : LP_1.0.7 (2011-09-09)\n";
char         g_strMsg[MAX_BUFFER_SIZE];

// For LP_Dut Setup
int	   HT40ModeOn, FREQ_MHZ;
int	   TX1, TX2, TX3, TX4;
int    RX1, RX2, RX3, RX4;
double TX_POWER_DBM;
char   preModFile[MAX_BUFFER_SIZE] = {""};
char   DATA_RATE[MAX_BUFFER_SIZE], PREAMBLE[MAX_BUFFER_SIZE], PACKET_FORMAT_11N[MAX_BUFFER_SIZE];
map <string,string> g_rates,g_nrate;
int freq ;

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

/*
 * The return value of this function does not follow the general rule.
 * If no errors, it returns a value >=0 (0 being the first ID)
 * If errors occur, it always return -1
 */
LP_DUT_API int WIFI_DutRegister(void)
{

	/* DataRate list*/

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
	g_rates.insert( pair<string, string>("MCS0",    "-m 0") );
	g_rates.insert( pair<string, string>("MCS1",    "-m 1") );
	g_rates.insert( pair<string, string>("MCS2",    "-m 2") );
	g_rates.insert( pair<string, string>("MCS3",    "-m 3") );
	g_rates.insert( pair<string, string>("MCS4",    "-m 4") );
	g_rates.insert( pair<string, string>("MCS5",    "-m 5") );
	g_rates.insert( pair<string, string>("MCS6",    "-m 6") );
	g_rates.insert( pair<string, string>("MCS7",    "-m 7 -s 0") );
	//g_rates.insert( pair<string, string>("MCS8",    "-m 8 -s 1") );
	g_rates.insert( pair<string, string>("MCS8",    "-m 8 ") );//mike add 2013-6-21
	g_rates.insert( pair<string, string>("MCS9",    "-m 9 -s 1") );
	g_rates.insert( pair<string, string>("MCS10",   "-m 10 -s 1") );
	g_rates.insert( pair<string, string>("MCS11",   "-m 11 -s 1") );
	g_rates.insert( pair<string, string>("MCS12",   "-m 12 -s 1") );
	g_rates.insert( pair<string, string>("MCS13",   "-m 13 -s 1") );
	g_rates.insert( pair<string, string>("MCS14",   "-m 14 -s 1") );
	//g_rates.insert( pair<string, string>("MCS15",   "-m 15 -s 1") );
	//g_rates.insert( pair<string, string>("MCS15",   "-m 15  ") );////mike add 2013-6-21 for 275 mcs8
	g_rates.insert( pair<string, string>("MCS15",   "-m 15 -s 3 ") );//mike modify 2013-7-2  for mcs15
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

	/*End*/

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
		//Add function to disable wireless

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
				sprintf_s(szLocalCmd,sizeof(szLocalCmd),"param set TE_HH=%s;param save;sync",szHH);
				SendDutCmd("",szLocalCmd);
			}		
			DeleteFile(".\\HH.txt");
		}

		// add end
		GetPrivateProfileString("Configuration","Disable_WiFi","0",    szDisablewf,sizeof(szDisablewf),iniPath);
		GetPrivateProfileString("Configuration","Disable_WiFi1","0",    szDisablewf1,sizeof(szDisablewf),iniPath);
		if(strstr(szDisablewf,"1") && strstr(szDisablewf1,"1"))
		{
			GetPrivateProfileString("DISWIFI","cmd1","",stDisableWiFi.szCmd1,sizeof(stDisableWiFi.szCmd1),".\\LP_DUT_setup.ini");
			GetPrivateProfileString("DISWIFI","cmd2","",stDisableWiFi.szCmd2,sizeof(stDisableWiFi.szCmd2),".\\LP_DUT_setup.ini");
			GetPrivateProfileString("DISWIFI","cmd3","",stDisableWiFi.szCmd3,sizeof(stDisableWiFi.szCmd3),".\\LP_DUT_setup.ini");
			GetPrivateProfileString("DISWIFI","cmd4","",stDisableWiFi.szCmd4,sizeof(stDisableWiFi.szCmd4),".\\LP_DUT_setup.ini");
			SendDutCmd("",stDisableWiFi.szCmd1);
			SendDutCmd("",stDisableWiFi.szCmd2);
			SendDutCmd("",stDisableWiFi.szCmd3);
			SendDutCmd("",stDisableWiFi.szCmd4);
		} 
		//End
		bOldSameDut = false;
		IsSameDut.nOldBandWidth = -1;
		IsSameDut.nOldfreq = -1;
		IsSameDut.szOldDataRate = "";
		//g_LP_mimo_id = -1;
		SendSocketCmd("sync\n","#",1000);
		Sleep(1000);
		g_LP_connected = 0;		// LP_Status = disconnected now
		
		SendSocketCmd("exit\n","",100);  //mike modify this for off wifi

		if(h_socketThread != NULL)
		{			
			TerminateThread(h_socketThread,0);
			Sleep(1000);
		}
		//;
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
			api_status = 0;
			g_LP_connected = 1;		// LP_Status = connected now
		}

		api_status=::vDUT_AddStringReturn(g_LP_mimo_id, "DUT_NAME", "Litepoint Loop-Back Test");
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(DUT_NAME) return error.\n");

		api_status=::vDUT_AddStringReturn(g_LP_mimo_id, "DUT_DRIVER_INFO", dutInfo);
		CheckReturnError(api_status, "[Error] vDUT_AddStringReturn(DUT_DRIVER_INFO) return error.\n");
		
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
		printf("send command \"arp -d \" ok!!!\r\n ");
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
			
		
		char szDutMac[30]="000000000001";
		if (!IP_ArpMac(csSocket.szDutIP,szDutMac))
		{
			api_status=4;
			CheckReturnError(api_status, "[Error] Failed to get dut mac.\n");			
			return api_status;
		}


		ErrorCode=0;
		
		int nTelnetNum=10;
		bool IsTelnet=false;
		strSocketBuf.clear();
		while(nTelnetNum--)
		{
			//WinExec("Telnet275.exe",SW_HIDE);
			if(!IP_TelnetEnable(csSocket.szDutIP,szDutMac))
			{
				ErrorCode=5;
				CheckReturnError(api_status, "[Error] Failed to get dut mac.\n");
				continue;
			}

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
			Sleep(1000);
			
		}

		if(!IsTelnet)
		{
			api_status = 6;
			CheckReturnError(api_status, "[Error] Create socket return error.\n");
		}
		else
		{
			// wait till wifi status ok.
			if(csSocket.nCheckWiFiStatus)
			{
				char szCmd[64]="";
				sprintf_s(szCmd,sizeof(szCmd),"%s\n",csSocket.szCmdCheckWiFi);
				while(!SendSocketCmd(szCmd,csSocket.szKeyCheckWiFi))
				{
					Sleep(10);
				}

				//int i=20;
				//while(i--)
				//{
				//	if(SendSocketCmd(szCmd,csSocket.szKeyCheckWiFi)) break;
				//	else
				//	{
				//		Sleep(2000);
				//		/*char szCmd1[64];
				//		sprintf_s(szCmd1,sizeof(szCmd1),"%s 1\n",csSocket.szCmdCheckWiFi);
				//		SendSocketCmd(szCmd1);*/
				//	}
				//	
				//}
				//if(i<1)
				//{
				//	api_status=6; 
				//	CheckReturnError(api_status, "[Error] Send Wifi command return error.\n");
				//}



				Sleep(2000);
				// initialize 2g here
				char CmdPrefix2[16]="wl -i wl1 ";//2G command just Prefix different,use CmdPrefix mark command prefix before command use
				// run these commands once.
				SendDutCmd(CmdPrefix2,"mpc 0");
				SendDutCmd(CmdPrefix2,"ssid \"\"");
				SendDutCmd(CmdPrefix2,"down");
				SendDutCmd(CmdPrefix2,"country ALL");
				SendDutCmd(CmdPrefix2,"wsec 0");
				SendDutCmd(CmdPrefix2,"obss_coex 0");
				SendDutCmd(CmdPrefix2,"stbc_tx 0");
				SendDutCmd(CmdPrefix2,"stbc_rx 1");
				SendDutCmd(CmdPrefix2,"spect 0");
				SendDutCmd(CmdPrefix2,"bw_cap 2g 3");
				SendDutCmd(CmdPrefix2,"mbss 0");
				SendDutCmd(CmdPrefix2,"up");
				SendDutCmd(CmdPrefix2,"frameburst 0");
				SendDutCmd(CmdPrefix2,"down");
				SendDutCmd(CmdPrefix2,"ampdu 0");
				SendDutCmd(CmdPrefix2,"wd_disable 1");
				SendDutCmd(CmdPrefix2,"mimo_preamble 0");
				SendDutCmd(CmdPrefix2,"up");
				SendDutCmd(CmdPrefix2,"mimo_txbw 4");
				//SendDutCmd(CmdPrefix2,"scansuppress 1");
				SendDutCmd(CmdPrefix2,"mimo_txbw 4");
				SendDutCmd(CmdPrefix2,"pkteng_stop rx");
				SendDutCmd(CmdPrefix2,"pkteng_stop tx");
				SendDutCmd(CmdPrefix2,"stbc_rx 0");
				SendDutCmd(CmdPrefix2,"tempsense_disable 1");

				// initialize 5g here
				char CmdPrefix[16]="wl -i wl0 ";//5G command just Prefix different,use CmdPrefix mark command prefix before command use
				// run these commands once.
				SendDutCmd(CmdPrefix,"down");

				SendDutCmd(CmdPrefix,"mpc 0");
				SendDutCmd(CmdPrefix,"up");
				SendDutCmd(CmdPrefix,"pkteng_stop tx");
				SendDutCmd(CmdPrefix,"pkteng_stop rx");
				SendDutCmd(CmdPrefix,"down");
				SendDutCmd(CmdPrefix,"country ALL");
				SendDutCmd(CmdPrefix,"mimo_bw_cap 1");
				SendDutCmd(CmdPrefix,"stbc_tx 0");
				SendDutCmd(CmdPrefix,"stbc_rx 1");
				SendDutCmd(CmdPrefix,"mimo_preamble 0");
				SendDutCmd(CmdPrefix,"phy_percal 3");
				//SendDutCmd(CmdPrefix,"phy_watchdog 0");
				SendDutCmd(CmdPrefix,"interference 0");
				//SendDutCmd(CmdPrefix,"scansuppress 1");
				SendDutCmd(CmdPrefix,"glacial_timer 150000");
				SendDutCmd(CmdPrefix,"fast_timer 150000");
				SendDutCmd(CmdPrefix,"slow_timer 150000");
				SendDutCmd(CmdPrefix,"bi 100");
				SendDutCmd(CmdPrefix,"up");
				SendDutCmd(CmdPrefix,"ssid \"\"");
				SendDutCmd(CmdPrefix,"txpwr1 -o -q 60");
				SendDutCmd(CmdPrefix,"down");
			}
			else
			{
				Sleep(csSocket.nCheckWiFiSleep);
			}
		}

//Tony add 2014.2.19 : compare HH label between DUT and SFIS to avoid scan A test B 
		char szhh[256]="";
		GetPrivateProfileString("DUTTE_HH","TE_HH","",szhh,sizeof(szhh),".\\HH.txt");//When begin test PT program will write SN value to config file
		

		//SendSocketCmd("nvram get TE_HH\r\n","#",5000);
		SendSocketCmd("param get TE_HH\r\n","/",5000);
		// to check the last letter of return value
		
		if(strSocketBuf.find("\r\n\r\n/ #") != -1)
		{
			// this means the last letter is #, new DUT
		}
		else
		{
			if(strSocketBuf.find(szhh) == -1)
			{
				api_status = -1;
				CheckReturnError(api_status, "[Error] Scan product A but test product B\n");
			}
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
	
	try
	{
		// Your code here...
		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "CHANNEL_BW", &HT40ModeOn);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CHANNEL_BW) return error.\n");


		
		// errReturn = -1;		// Set it to a non-zero value, if needed. If set to non-zero, it throws a string such as 
								// throw "This is an example of an error message" to the upper layer.
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
		
		char szCmd[BUFSIZE];
		char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use

		if(freq < 14 )
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
		}
		else
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
		}		
		SendDutCmd(CmdPrefix,"pkteng_stop tx");
		/*Same Dut don't send this command */
		//if( !bOldSameDut || abs(IsSameDut.nOldfreq - freq) > 20) 
		{
			SendDutCmd(CmdPrefix,"radio on");
			SendDutCmd(CmdPrefix,"mpc 0");
			
			SendDutCmd(CmdPrefix,"down");
			SendDutCmd(CmdPrefix,"country ALL");
			SendDutCmd(CmdPrefix,"ssid");
			SendDutCmd(CmdPrefix,"wsec 0");
			SendDutCmd(CmdPrefix,"interference 0");
			if(freq<14)
			{
				SendDutCmd(CmdPrefix,"band b");
			}
			else
			{
				SendDutCmd(CmdPrefix,"band a");		
			}
			SendDutCmd(CmdPrefix,"bi 100");
			SendDutCmd(CmdPrefix,"frameburst 0");
			SendDutCmd(CmdPrefix,"mimo_bw_cap 1");			
			SendDutCmd(CmdPrefix,"up");	
		}
		SendDutCmd(CmdPrefix,"down");//2		

		if(HT40ModeOn == 1)//HT40
		{			
			SendDutCmd(CmdPrefix,"mimo_txbw 4");			
		}
		else
		{			
			SendDutCmd(CmdPrefix,"mimo_txbw 2");//3			
		}	
		//if( (IsSameDut.nOldfreq != freq) || (IsSameDut.nOldBandWidth != HT40ModeOn))
		//{
			if(HT40ModeOn == 1)
			{
				if(freq<14)
				{
					sprintf_s(szCmd,"chanspec -c %d -b 2 -w 40 -s 1",freq);
					SendDutCmd(CmdPrefix,szCmd);
				}
				else
				{
					//sprintf_s(szCmd,"chanspec -c %d -b 5 -w 40 -s 1",freq);
					if(freq == 36 || freq ==44 || freq == 52 || freq == 60 || freq ==100 || freq == 108 || freq ==116 || freq == 124|| freq ==132|| freq ==149|| freq ==157)
					{	sprintf_s(szCmd,"chanspec %dl",freq);
						SendDutCmd(CmdPrefix,szCmd);			
					}
					else if (freq == 40 || freq ==48 || freq ==56 || freq ==64 || freq ==104 || freq ==112 || freq == 120 || freq ==128 || freq ==136 || freq ==153 || freq ==161)
					{	sprintf_s(szCmd,"chanspec %du",freq);
						SendDutCmd(CmdPrefix,szCmd);			
					}
					else
					{
						sprintf_s(szCmd,"chanspec -c %d -b 5 -w 40 -s 1",freq);
						SendDutCmd(CmdPrefix,szCmd);
					}
				}
			}
			else
			{
				if(freq<14)
				{
					sprintf_s(szCmd,"chanspec -c %d -b 2 -w 20 -s 0",freq);//4
					SendDutCmd(CmdPrefix,szCmd);
				}
				else
				{
					sprintf_s(szCmd,"chanspec -c %d -b 5 -w 20 -s 0",freq);
					SendDutCmd(CmdPrefix,szCmd);			
				}
			}						
		//}
		SendDutCmd(CmdPrefix,"up");//5
		//SendDutCmd(CmdPrefix,"join EVM_Test_41 imode infra");//6	
		IsSameDut.nOldBandWidth = HT40ModeOn;
		IsSameDut.nOldfreq = freq;
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

		map<string,string>::iterator it_rate = g_rates.find(DATA_RATE);
		assert(it_rate != g_rates.end());
		char szCmd[BUFSIZE];
		char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use

		if(freq < 14 )
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
		}
		else
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
		}

		SendDutCmd(CmdPrefix,"txchain 3");//7
		SendDutCmd(CmdPrefix,"rxchain 3");//8
		//if(strcmp(IsSameDut.szOldDataRate.c_str(), DATA_RATE))
		{
			sprintf_s(szCmd,"nrate %s\n" , it_rate->second.c_str());
			SendDutCmd(CmdPrefix,szCmd);//9
			IsSameDut.szOldDataRate = DATA_RATE;
		}
		SendDutCmd(CmdPrefix,"rate");//10
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
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(PREAMBLE) return error.\n");

		api_status = ::vDUT_GetStringParameter (g_LP_mimo_id, "PACKET_FORMAT_11N", PACKET_FORMAT_11N, MAX_BUFFER_SIZE);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(PACKET_FORMAT_11N) return error.\n");		
				
		::vDUT_WiFiConvertFrequencyToChannel(FREQ_MHZ,&freq);


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
		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "TX1", &TX1);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(TX1) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "TX2", &TX2);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(TX2) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "TX3", &TX3);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(TX3) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "TX4", &TX4);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(TX4) return error.\n");

		api_status = ::vDUT_GetDoubleParameter (g_LP_mimo_id, "TX_POWER_DBM", &TX_POWER_DBM);
		CheckReturnError(api_status, "[Error] vDUT_GetDoubleParameter(TX_POWER_DBM) return error.\n");
		/*Add*/
		int	nAntNum=TX1+TX2+TX3+TX4;
		char szCmd[BUFSIZE];
		char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use

		if(freq < 14 )
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
		}
		else
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
		}
		memset(szCmd , 0 , BUFSIZE);


		if(ShowDebugInfor)
		{
			printf("Test ant:TX1=%d , TX2=%d , TX3=%d , TX4=%d \r\n",TX1 , TX2 , TX3 , TX4);//11
		}

		int iRate=-1;
		if(/*HT40ModeOn*/strstr(DATA_RATE,"MCS"))
		{
			iRate=atoi(DATA_RATE+3);
		}

		if(iRate>=15)
		{   

			SendDutCmd(CmdPrefix," down");
			SendDutCmd(CmdPrefix," up");
			SendDutCmd(CmdPrefix,"ssid \"\"");
			SendDutCmd(CmdPrefix," down");
			SendDutCmd(CmdPrefix,"spect 0");
			SendDutCmd(CmdPrefix,"bw_cap 5g 3");
			SendDutCmd(CmdPrefix,"bw_cap 2g 3");
			SendDutCmd(CmdPrefix,"mbss 0");
			SendDutCmd(CmdPrefix,"frameburst 0");
			SendDutCmd(CmdPrefix,"ampdu 0");
			SendDutCmd(CmdPrefix,"up");
			SendDutCmd(CmdPrefix,"stbc_tx 0");
			SendDutCmd(CmdPrefix,"down");
			SendDutCmd(CmdPrefix,"bi 65535");
			SendDutCmd(CmdPrefix,"rate -1");
			SendDutCmd(CmdPrefix,"ampdu 1");
			SendDutCmd(CmdPrefix,"frameburst 1");
			SendDutCmd(CmdPrefix,"txchain 3");
			SendDutCmd(CmdPrefix,"rxchain 3");
			if(freq < 14) 
			{
				SendDutCmd(CmdPrefix,"band b");	
			}
			else 
			{
				SendDutCmd(CmdPrefix,"band a");	
			}
#if 0
			if(HT40ModeOn == 1)//HT40
			{
				if(freq<14)//2.4G
				{
					sprintf_s(szCmd,"chanspec -c %d -b 2 -w 40 -s 1",freq);
					//sprintf_s(szCmd,"chanspec %d ",freq);//mike modify 2013-7-4
					SendDutCmd(CmdPrefix,szCmd);
				}
				else   //5G
				{
					sprintf_s(szCmd,"chanspec -c %d -b 5 -w 40 -s 1",freq);// not necessary
					SendDutCmd(CmdPrefix,szCmd);			
				}
			}
			else  //HT20
			{
				if(freq<14) //2.4G
				{
					sprintf_s(szCmd,"chanspec -c %d -b 2 -w 20 -s 0",freq);//4
					//sprintf_s(szCmd,"chanspec %d ",freq);//mike modify 2013-7-4
					SendDutCmd(CmdPrefix,szCmd);
				}
				else
				{
					sprintf_s(szCmd,"chanspec -c %d -b 5 -w 20 -s 0",freq);
					SendDutCmd(CmdPrefix,szCmd);			
				}
			}						
#endif


			SendDutCmd(CmdPrefix,"up");
			SendDutCmd(CmdPrefix,"phy_forcecal 1");
			//SendDutCmd(CmdPrefix,"phy_activecal");
			//SendDutCmd(CmdPrefix,"ap");
			SendDutCmd(CmdPrefix,"ssid \"\"");

			//SendDutCmd(CmdPrefix,"nrate -m 15 -s 3");//mike modify 2013-7-4 for test
			//************************************************************************
			api_status = ::vDUT_GetStringParameter(g_LP_mimo_id, "DATA_RATE", DATA_RATE, MAX_BUFFER_SIZE);
			CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(DATA_RATE) return error.\n");		
			map<string,string>::iterator it_rate = g_rates.find(DATA_RATE);
			assert(it_rate != g_rates.end());
			char szCmd[BUFSIZE];
			//char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use
			sprintf_s(szCmd,"nrate %s\n" , it_rate->second.c_str());
			SendDutCmd(CmdPrefix,szCmd);//9
			IsSameDut.szOldDataRate = DATA_RATE;
			//*************************************************************************	
			SendDutCmd(CmdPrefix,"rate ");//mike modify 2013-7-4 for test
			SendDutCmd(CmdPrefix,"phy_txpwrctrl 1");
		}

		else if(iRate>=8&&iRate<15)
		{
          SendDutCmd(CmdPrefix,"txchain 3");
		  SendDutCmd(CmdPrefix,"rxchain 3");
        }
		else  //mike modify
		{
			if(TX1 == 1)
			{
				//SendDutCmd(CmdPrefix,"txchain 1");
				SendDutCmd(CmdPrefix,"txant 0");
				SendDutCmd(CmdPrefix,"antdiv 0");

			}
			else if(TX2 == 1)
			{
				//SendDutCmd(CmdPrefix,"txchain 2");
				SendDutCmd(CmdPrefix,"txant 1");
				SendDutCmd(CmdPrefix,"antdiv 1");

			}
			else
			{
				//do nothing
			}
		}
		SendDutCmd(CmdPrefix,"join EVM_Test_41 imode infra");
		SendDutCmd(CmdPrefix,"ssid \"\"");

		sprintf_s(szCmd,"txpwr1 -o -q %.1f", TX_POWER_DBM*4);
		SendDutCmd(CmdPrefix, szCmd);
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
		bOldSameDut = true;

		char   modFile[MAX_BUFFER_SIZE];
		char   pathStr[MAX_BUFFER_SIZE];
		int    ini_status; 
        ini_status = LP_read_ini(pathStr);
		//CheckReturnError(api_status, "[Error] LP_DUT_setup.ini does not exist.\n");

		//printf("pathStr in ini file is %s, api status = %d", pathStr,api_status);
		//api_status = GetWaveformFileName(pathStr, 
		//								 "mod", 									  
		//								 HT40ModeOn, 
		//								 DATA_RATE, 
		//								 PREAMBLE, 
		//								 PACKET_FORMAT_11N, 
		//								 modFile, 
		//								 MAX_BUFFER_SIZE);
		//CheckReturnError(api_status, "[Error] LP_DUT_setup.ini existing error code %d. GetWaveformFileName(mod_file) return error.\n", ini_status);

		//if ( 0!=strcmp(modFile, preModFile) )	// mod file name != preModFile
		//{
		//	sprintf_s(preModFile, MAX_BUFFER_SIZE, "%s", modFile);
		//	api_status = ::LP_SetVsgModulation( modFile );
		//	CheckReturnError(api_status, "[Error] LP_SetVsgModulation(%s) return error.", modFile);
		//}
		//else
		//{
		//	// do nothing
		//}	
		char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use
		if(freq < 14 )
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
		}
		else
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
		}
		//SendDutCmd(CmdPrefix,"pkteng_start aa.bb.cc.dd.ee.ff tx sync 50 100 250");
		//SendDutCmd(CmdPrefix,"pkteng_start 00:90:4c:21:00:8e tx 100 1024 0");

		//SendDutCmd(CmdPrefix,"pkteng_start aa.bb.cc.dd.ee.ff tx 50 100 1000");
		SendDutCmd(CmdPrefix,"up");
		SendDutCmd(CmdPrefix,"isup");
		SendDutCmd(CmdPrefix,"interference_override 3");
		SendDutCmd(CmdPrefix,"mpc 0");
		SendDutCmd(CmdPrefix,"interference_override 0");
		SendDutCmd(CmdPrefix,"ssid \"\"");
		if(freq<=14) // band b
		{
			SendDutCmd(CmdPrefix,"phy_forcecal 1");
			SendDutCmd(CmdPrefix,"pkteng_start 00:11:22:33:44:55 tx 100 1000 0 ff:ee:dd:cc:bb:aa");//MIKE MODIFY 2013-7-3		
		}
		else
		{
			SendDutCmd(CmdPrefix,"phy_forcecal 2");
			SendDutCmd(CmdPrefix,"pkteng_start 00:11:22:33:44:55 tx 150 1000 0 ff:ee:dd:cc:bb:aa");//MIKE MODIFY 2013-7-3		
		
		}
		
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
		//char szCmd[BUFSIZE];
		char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use

		if(freq < 14 )
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
		}
		else
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
		}
		SendDutCmd(CmdPrefix,"pkteng_stop tx");	

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

	char szCmd[BUFSIZE];
	char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use

	if(freq < 14 )
	{
		sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
	}
	else
	{
		sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
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
	api_status = ::vDUT_GetStringParameter(g_LP_mimo_id, "DATA_RATE", DATA_RATE, MAX_BUFFER_SIZE);
	CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(DATA_RATE) return error.\n");		


	map<string,string>::iterator it_rate = g_rates.find(DATA_RATE);
	assert(it_rate != g_rates.end());

	char szCmd[BUFSIZE];	
	char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use

	if(freq < 14 )
	{
		sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
	}
	else
	{
		sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
	}
	SendDutCmd(CmdPrefix,"up");	
	//if(strcmp(IsSameDut.szOldDataRate.c_str(), DATA_RATE))
	{
		sprintf_s(szCmd,"nrate %s\n" , it_rate->second.c_str());
		SendDutCmd(CmdPrefix,szCmd);
		IsSameDut.szOldDataRate = DATA_RATE;
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

		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "FREQ_MHZ", &FREQ_MHZ);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(FREQ_MHZ) return error.\n");

		api_status = ::vDUT_GetIntegerParameter(g_LP_mimo_id, "CHANNEL_BW", &HT40ModeOn);
		CheckReturnError(api_status, "[Error] vDUT_GetIntegerParameter(CHANNEL_BW) return error.\n");

		api_status = ::vDUT_GetStringParameter(g_LP_mimo_id, "DATA_RATE", DATA_RATE, MAX_BUFFER_SIZE);
		CheckReturnError(api_status, "[Error] vDUT_GetStringParameter(DATA_RATE) return error.\n");	



		::vDUT_WiFiConvertFrequencyToChannel(FREQ_MHZ,&freq);

		int nAnt = RX1 + RX2 + RX3 + RX4 ; 

		char szCmd[BUFSIZE];
		char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use

		if(freq < 14 )
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
		}
		else
		{
			sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
		}

		//change for kim debug 


		SendDutCmd(CmdPrefix,"radio on");
		SendDutCmd(CmdPrefix,"mpc 0");
		SendDutCmd(CmdPrefix,"down");
		SendDutCmd(CmdPrefix,"country ALL");
		SendDutCmd(CmdPrefix,"ssid");
		//SendDutCmd(CmdPrefix,"ssid \"NETGEAR\"");
		SendDutCmd(CmdPrefix,"wsec 0");
		SendDutCmd(CmdPrefix,"interference 0");
		if(freq < 14) 
		{
			SendDutCmd(CmdPrefix,"band b");	
		}
		else 
		{
			SendDutCmd(CmdPrefix,"band a");	
		}

		SendDutCmd(CmdPrefix,"bi 65535");
		SendDutCmd(CmdPrefix,"frameburst 0");
		SendDutCmd(CmdPrefix,"mimo_bw_cap 1");
		SendDutCmd(CmdPrefix,"up");

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
		SendDutCmd(CmdPrefix,"down");
		if(HT40ModeOn)
		{
			SendDutCmd(CmdPrefix,"mimo_txbw 4");	
		}
		else
		{
			SendDutCmd(CmdPrefix,"mimo_txbw 2");			
		}

		char szChanCmd[128]="";			
		if(freq<14)
		{
			//SendDutCmd(CmdPrefix,"band b");
			if(HT40ModeOn)
			{
				//SendDutCmd(CmdPrefix,"mimo_txbw 4");
				sprintf_s(szChanCmd,sizeof(szChanCmd),"chanspec -c %d -b 2 -w 40 -s 1",freq);				
			}
			else
			{
				//SendDutCmd(CmdPrefix,"mimo_txbw 2");
				sprintf_s(szChanCmd,sizeof(szChanCmd),"chanspec -c %d -b 2 -w 20 -s 0",freq);	
			}
			SendDutCmd(CmdPrefix,szChanCmd);
		}
		else
		{
			//SendDutCmd(CmdPrefix,"band a");
			if(HT40ModeOn)
			{
				//SendDutCmd(CmdPrefix,"mimo_txbw 4");
				sprintf_s(szChanCmd,sizeof(szChanCmd),"chanspec -c %d -b 5 -w 40 -s 1",freq);				
			}
			else
			{
				//SendDutCmd(CmdPrefix,"mimo_txbw 2");
				sprintf_s(szChanCmd,sizeof(szChanCmd),"chanspec -c %d -b 5 -w 20 -s 0",freq);	
			}
			SendDutCmd(CmdPrefix,szChanCmd);
		}
		SendDutCmd(CmdPrefix,"up");

		map<string,string>::iterator it_rate_1 = g_rates.find(DATA_RATE);
		assert(it_rate_1 != g_rates.end());
		memset(szCmd , 0 , 256);
		sprintf_s(szCmd,"nrate %s\n" , it_rate_1->second.c_str());
		SendDutCmd(CmdPrefix,szCmd);	
		SendDutCmd(CmdPrefix,"join EVM_Test_41 imode infra");
		SendDutCmd(CmdPrefix,"down");
		if(ShowDebugInfor)
		{
			printf("Test ant:RX1=%d , RX2=%d , RX3=%d , RX4=%d \r\n",RX1 , RX2 , RX3 , RX4);
		}
		if(nAnt == 1)
		{
			if(RX1 == 1)
			{
				SendDutCmd(CmdPrefix,"rxchain 1");				
				SendDutCmd(CmdPrefix,"txchain 1");
			}
			else if(RX2 == 1)
			{
				SendDutCmd(CmdPrefix,"rxchain 2");				
				SendDutCmd(CmdPrefix,"txchain 2");
			}
			//else if(RX3 == 1)
			//{
			//	SendDutCmd(CmdPrefix,"mimo_txbw 2");
			//}
			//else if(RX4 == 1)
			//{
			//	SendDutCmd(CmdPrefix,"mimo_txbw 2");
			//}
			else
			{
				//do nothing
			}		
		}
		else
		{
			SendDutCmd(CmdPrefix,"rxchain 3");			
			SendDutCmd(CmdPrefix,"txchain 3");
		}
		SendDutCmd(CmdPrefix,"up");
		SendDutCmd(CmdPrefix,"join Sensitivity_Test_41 imode infra");
		SendDutCmd(CmdPrefix,"down");
		if(RX1 == 1)
			{
				//SendDutCmd(CmdPrefix,"rxchain 1");	

				SendDutCmd(CmdPrefix,"txchain 1");
				SendDutCmd(CmdPrefix,"rxchain 1");
				
			}
			else if(RX2 == 1)
			{
				//SendDutCmd(CmdPrefix,"rxchain 2");				
				SendDutCmd(CmdPrefix,"txchain 2");
				SendDutCmd(CmdPrefix,"rxchain 2");
			}
		SendDutCmd(CmdPrefix,"up");
		SendDutCmd(CmdPrefix,"txchain");
		SendDutCmd(CmdPrefix,"rxchain");
		

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

int LP_get_rx_stats(void)
{
    int api_status = 0;

	::vDUT_ClearReturns(g_LP_mimo_id);

	unsigned long cntRx=0, cntOk=0, cntKo=0;
	int rssi=0, nf=0;

	// First, check LP tester status
    if( !g_LP_connected )
    {
		return -1;
	}
	else
	{
		// do nothing
	}

	char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use

	if(freq < 14 )
	{
		sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
	}
	else
	{
		sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
	}
	
	SendDutCmd(CmdPrefix,"counters");

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
	try
	{			
		api_status = ::vDUT_AddIntegerReturn(g_LP_mimo_id, "TOTAL_PACKETS", cntRx);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(TOTAL_PACKETS) return error.\n");

		api_status = ::vDUT_AddIntegerReturn(g_LP_mimo_id, "GOOD_PACKETS", cntOk);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(GOOD_PACKETS) return error.\n");

		api_status = ::vDUT_AddIntegerReturn(g_LP_mimo_id, "BAD_PACKETS", cntKo);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(BAD_PACKETS) return error.\n");
		
		api_status = ::vDUT_AddIntegerReturn(g_LP_mimo_id, "RSSI", rssi);
		CheckReturnError(api_status, "[Error] vDUT_AddIntegerReturn(RSSI) return error.\n");
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
	char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use
	if(freq < 14 )
	{
		sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
	}
	else
	{
		sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
	}

	//SendDutCmd(CmdPrefix,"pkteng_start 00:00:00:00:00:01 rx 220 1500 0");

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
	char CmdPrefix[16]="";//2G and 5G command just Prefix different,use CmdPrefix mark command prefix before command use

	if(freq < 14 )
	{
		sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix2g);
	}
	else
	{
		sprintf_s(CmdPrefix,sizeof(CmdPrefix),"%s",csSocket.szCmdPrefix5g);
	}
	SendDutCmd(CmdPrefix,"pkteng_stop rx");	

    return api_status;
}

int  GetWaveformFileName(char* filePath, char* fileType, int HT40ModeOn, char* datarate, char* preamble, char* packetFormat11N, char* waveformFileName, int bufferSize)
{
    int  api_status = -1;
	char waveFilePath[MAX_BUFFER_SIZE] = {'\0'};
	char modFileName[MAX_BUFFER_SIZE] = {'\0'};
 

	::vDUT_ClearReturns(g_LP_mimo_id);	

	
	sprintf_s(waveFilePath, MAX_BUFFER_SIZE, "%s", filePath);
	

	if ( 0==strcmp(datarate, "DSSS-1") )
	{
		sprintf_s(modFileName, bufferSize, "WiFi_%s.%s", datarate, fileType);
		sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s.%s", waveFilePath, datarate, fileType);
	}
	else if ( (strstr(datarate, "DSSS")!=NULL)||(strstr(datarate, "CCK")!=NULL) )
	{
		char tmpPreamble[2]; 
		tmpPreamble[0] = preamble[0];
		tmpPreamble[1] = '\0';
		sprintf_s(modFileName, bufferSize, "WiFi_%s%s.%s", datarate, tmpPreamble, fileType);
		sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s%s.%s", waveFilePath, datarate, tmpPreamble, fileType);
	}
	else if ( (strstr(datarate, "OFDM")!=NULL) )
	{
		sprintf_s(modFileName, bufferSize, "WiFi_%s.%s", datarate, fileType);
		sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s.%s", waveFilePath, datarate, fileType);
	}
	else if ( (strstr(datarate, "MCS")!=NULL) )
	{
		if (HT40ModeOn==0)
		{
			sprintf_s(modFileName, bufferSize, "WiFi_%s_%s.%s", "HT20", datarate, fileType);
			sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s_%s.%s", waveFilePath, "HT20", datarate, fileType);
		}
		else
		{
			sprintf_s(modFileName, bufferSize, "WiFi_%s_%s.%s", "HT40", datarate, fileType);
			sprintf_s(waveformFileName, bufferSize, "%sWiFi_%s_%s.%s", waveFilePath, "HT40", datarate, fileType);
		}
	}
	else
	{
		// do nothing
		return -1;
	}
 printf("\nwaveformFileName is %s\n",waveformFileName);
	// Wave file checking
	FILE *waveFile;
	fopen_s(&waveFile, waveformFileName, "r");
	if (!waveFile)
	{
		
			char tempPath[MAX_BUFFER_SIZE] = {'\0'};
			sprintf_s(tempPath, bufferSize, "./mod/%s", modFileName);

			fopen_s(&waveFile, tempPath, "r");
			if (!waveFile)
			{
               sprintf_s(tempPath, bufferSize, "../mod/%s", modFileName); 
			   fopen_s(&waveFile, tempPath, "r");
                 	if (!waveFile)
			        {
						sprintf_s(tempPath, bufferSize, "../../mod/%s", modFileName); 
			            fopen_s(&waveFile, tempPath, "r");
                            	if (!waveFile)
			                      {
			                        return -1;	//TM_ERR_PARAM_DOES_NOT_EXIST
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
			 }
			 else
			 {
                 // do nothing
				
			 }
	             
				
			
				sprintf_s(waveformFileName, bufferSize, "%s", tempPath);
				fclose(waveFile);
				waveFile = NULL;
				api_status = 0;
			
		 
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
int LP_read_ini (char* pathStr )
{
// by LJ
 
	int  ini_status = 0 ;
//	char pathStr[MAX_BUFFER_SIZE];
	
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
		stringLength = GetPrivateProfileString("Configuration","CheckWiFiCmd", "ls -l /tmp/wlan_time", csSocket.szCmdCheckWiFi, MAX_BUFFER_SIZE, iniPath);
		stringLength = GetPrivateProfileString("Configuration","CheckWiFiKeyWord", "root", csSocket.szKeyCheckWiFi, MAX_BUFFER_SIZE, iniPath);		
		csSocket.nCheckWiFiStatus = GetPrivateProfileInt("Configuration","CheckWiFiStatus",0,iniPath);
		csSocket.nCheckWiFiSleep = GetPrivateProfileInt("Configuration","CheckWiFiSleep",0,iniPath);        
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

//by LJ
}

// PeerSocket.cpp: implementation of the PeerSocket.
//
//////////////////////////////////////////////////////////////////////
//[Version]
/*
2012-11-30
1. Reset bRequestExit to false @ InitSocket
*/


#include "stdafx.h"
#include "PeerSocket.h"
#include "Tlhelp32.h"
#include "stdio.h"
#include <string>
using namespace std;
#define MAXBUFSIZE 2048

SOCKET g_socket; // dut socket
string strSocketBuf;
char DutIP[16]="";
bool ShowDebugInfor=false;
bool bRequestExit = false;
HANDLE h_socketThread=NULL;
//extern csSocket szcsSocket;
//////////////////////////////////////////////////////////////////////
void ambit_WSACleanup()
{
	WSACleanup();
}

bool Initialize_WSA()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return false;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) {
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
			WSACleanup( );
			return false; 
	}
	return true;
}
DWORD WINAPI _socket_receive(
  LPVOID lpParameter
)
{
	try
	{
		SOCKET *socket = (SOCKET*)lpParameter;
		while(*socket != INVALID_SOCKET)
		{	
			if(bRequestExit)
			{
				break;
			}
			char szBuf[2048+1]="";
			recv(*socket,szBuf,2048,0);
			if(ShowDebugInfor)
			{
				if(strlen(szBuf)>=1)
				{
					printf("%s",szBuf);
				}
			}
			strSocketBuf+=szBuf;

		}	
	}
	catch(char *msg)
	{
		printf(msg);
	}
	catch(...)
	{
	
	}

	return 0;
}
bool InitSocket(char *ip,int &ErrorType)
{
	Read_ini();
	//Add "arp -d"		
	char szArpCommand[128] = "";
	sprintf_s(szArpCommand,"arp -d");
	RunSpecifyExeAndRead(szArpCommand , false); 
	printf("send command \"arp -d \" ok!!!\r\n ");
	memset(szArpCommand,0,128);

	setsockopt(g_socket,SO_DONTLINGER,true,NULL,0);
	closesocket(g_socket);
	Sleep(300);
	try
	{
		g_socket=NULL;
		g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
    catch(...)
    {
        return FALSE;
    }  

	if(INVALID_SOCKET ==g_socket)
	{
		//		DisplayRunTimeInfo("Create socket fail\n");
		ErrorType=1;
		return FALSE;
	}
    
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	int iMode = 0;
	ioctlsocket(g_socket, FIONBIO, (u_long FAR*) &iMode);

	sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr( ip );
	clientService.sin_port = htons( 23 );

	if(h_socketThread!=NULL)
	{
		TerminateThread(h_socketThread,0);
	}
	bRequestExit = false;
	h_socketThread=CreateThread(NULL,0,_socket_receive,&g_socket,0,NULL);
	//----------------------
	// Connect to server.
	int revt=connect( g_socket, (SOCKADDR*) &clientService, sizeof(clientService) );

	if(SOCKET_ERROR==revt)
	{
		ErrorType=2;
		return FALSE;
	}

	ErrorType=0;

	return TRUE;
}

bool PingSpecifyIP(TCHAR* IP,int &ErrorType,int nFailCount)
{
	HANDLE hWritePipe  = NULL;
	HANDLE hReadPipe   = NULL;

	TCHAR  szPing[500] = "";
	sprintf_s(szPing, 500, "ping.exe %s -t", IP);
	TCHAR   szReadFromPipeData[500] = "";
	DWORD  byteRead    = 0;

	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle =true;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	if(!CreatePipe(&hReadPipe,&hWritePipe,&sa,0))
	{
//		DisplayRunTimeInfo("Create pipe fail\r\n");
		ErrorType=2;
		return false;
	}

	PROCESS_INFORMATION pi;
	STARTUPINFO        si;
	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
	si.hStdError  = hWritePipe;
	si.hStdOutput = hWritePipe;
	si.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow=SW_HIDE;

	if(!CreateProcess(NULL,szPing,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
	{
		ErrorType=1;
		return false;
	}

	CloseHandle(pi.hThread);
	CloseHandle(hWritePipe);
	CloseHandle(pi.hProcess);
	
	DWORD dwPingFailCount=nFailCount;
	bool  pingOK=false;
	while(dwPingFailCount--)
	{
		memset(szReadFromPipeData,0,500);
		Sleep(100);
		int bResult = ReadFile(hReadPipe,szReadFromPipeData,500,&byteRead,NULL);
		
		if(!bResult)
		{
			ErrorType=3;
			return false;
		}
		Sleep(200);
		if(strstr(szReadFromPipeData,"Reply from")!=NULL)
		{
			pingOK=true;
			break;
		}
	}
	CloseHandle(hReadPipe);
	if(pingOK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool PeerCreateSocket(SOCKET *pNewSocket,int iSockType)
{
	SOCKET socket;
	if((socket=WSASocket(AF_INET,iSockType,0,NULL,0,WSA_FLAG_OVERLAPPED))==INVALID_SOCKET)
	{
		DWORD dw = WSAGetLastError();
		printf("WSASocket LastError = %d" , dw );
		return false;
	}
	else
	{
		*pNewSocket=socket;
		return true;
	}
	
//	return ((socket=WSASocket(AF_INET,iSockType,0,NULL,0,WSA_FLAG_OVERLAPPED))==INVALID_SOCKET)?
//		false:*pNewSocket=socket,true;
}

bool PeerBindSocket(SOCKET BindSocket,char *szHostAddr,int iHostPort)
{
	struct sockaddr_in inAddr;
	inAddr.sin_addr.S_un.S_addr=inet_addr(szHostAddr);
	inAddr.sin_family=AF_INET;
	inAddr.sin_port=htons(iHostPort);
	return (bind(BindSocket,(PSOCKADDR)&inAddr,sizeof(inAddr))==SOCKET_ERROR)?
		false:true;
}
bool PeerConnectSocket(SOCKET BindSocket,char *szHostAddr,int iHostPort)
{
	struct sockaddr_in inAddr;
	inAddr.sin_addr.S_un.S_addr=inet_addr(szHostAddr);
	inAddr.sin_family=AF_INET;
	inAddr.sin_port=htons(iHostPort);
	return (connect(BindSocket,(PSOCKADDR)&inAddr,sizeof(inAddr))==SOCKET_ERROR)?
		false:true;
}
bool PeerListenSocket(SOCKET ListenSocket)
{
	return (listen(ListenSocket, 5))?
		false:true;
}

bool PeerShutDownSocket(SOCKET nowSocket)
{
	return shutdown(nowSocket,SD_BOTH)?false:true;
}

bool PeerCloseSocket(SOCKET nowSocket)
{
	return (closesocket(nowSocket)==SOCKET_ERROR)?
		false:true;
}
//重叠模式发送数据
bool PeerSendData(SOCKET socket,char *data,DWORD len,DWORD *retlen,WSAEVENT hSendEvent,WSAEVENT hExitEvent,DWORD time)
{
	WSABUF DataBuf;
	WSAEVENT hEvents[2];
	WSAOVERLAPPED SendOverLapp;
	DWORD flag;

	hEvents[0]=hExitEvent;
	hEvents[1]=hSendEvent;
	DataBuf.buf=data;
	DataBuf.len=len;
	memset(&SendOverLapp,0,sizeof(WSAOVERLAPPED));
	SendOverLapp.hEvent=hSendEvent;
	flag=0;
	/////////////////////////////////////
	int ret;
	if((ret=WSASend(socket,&DataBuf,1,retlen,flag,&SendOverLapp,NULL))==0)
		return true;
	else if((ret==SOCKET_ERROR)&&(WSAGetLastError()==WSA_IO_PENDING))
	{
		DWORD EventCaused=WSAWaitForMultipleEvents(2,hEvents,FALSE,time,FALSE);
		WSAResetEvent(hSendEvent);
		if(EventCaused == WSA_WAIT_FAILED || EventCaused == WAIT_OBJECT_0)
		{
			return false;
		}
		flag=0;
		return (WSAGetOverlappedResult(socket,&SendOverLapp,retlen,false,&flag))?
			true:false;
	}
	else
		return false;
}
//重叠模式接受数据
bool PeerRecvData(SOCKET socket,char *data,DWORD len,DWORD *retlen,WSAEVENT hRecvEvent,WSAEVENT hExitEvent,DWORD time)
{
	WSABUF DataBuf;
	WSAEVENT hEvents[2];
	WSAOVERLAPPED RecvOverLapp;
	DWORD flag;

	hEvents[0]=hExitEvent;
	hEvents[1]=hRecvEvent;
	DataBuf.buf=data;
	DataBuf.len=len;
	memset(&RecvOverLapp,0,sizeof(WSAOVERLAPPED));
	RecvOverLapp.hEvent=hRecvEvent;
	flag=0;
	/////////////////////////////////////
	int ret;
	if((ret=WSARecv(socket,&DataBuf,1,retlen,&flag,&RecvOverLapp,NULL))==0)
		return true;
	else if((ret==SOCKET_ERROR)&&(WSAGetLastError()==WSA_IO_PENDING))
	{
		DWORD EventCaused=WSAWaitForMultipleEvents(2,hEvents,FALSE,time,FALSE);
		WSAResetEvent(hRecvEvent);
		if(EventCaused == WSA_WAIT_FAILED || EventCaused == WAIT_OBJECT_0)
		{
			return false;
		}
		flag=0;
		return (WSAGetOverlappedResult(socket,&RecvOverLapp,retlen,false,&flag))?
			true:false;
	}
	else
		return false;
}
bool PeerSendDataS(SOCKET socket,char *data,DWORD len,DWORD *retlen,WSAEVENT hSendEvent,WSAEVENT hExitEvent,DWORD time)
{
	DWORD left,idx,thisret;
	left=len;
	idx=0;
	int oflag=0;
	while(left>0)
	{
		if(!PeerSendData(socket,&data[idx],left,&thisret,hSendEvent,hExitEvent,time))
		{
			*retlen=0;
			return false;
		}
		left-=thisret;
		idx+=thisret;
		if(thisret==0)
		{
			oflag++;
			if(oflag>5)
				break;
		}
	}
	*retlen=idx;
	return (idx==len)?true:false;

}
bool PeerRecvDataS(SOCKET socket,char *data,DWORD len,DWORD *retlen,WSAEVENT hRecvEvent,WSAEVENT hExitEvent,DWORD time)
{
	DWORD left,idx,thisret;
	left=len;
	idx=0;
	int oflag=0;
	while(left>0)
	{
		if(!PeerRecvData(socket,&data[idx],left,&thisret,hRecvEvent,hExitEvent,time))
		{
			*retlen=0;
			return false;
		}
		left-=thisret;
		idx+=thisret;
		if(thisret==0)
		{
			oflag++;
			if(oflag>5)
				break;
		}
	}
	*retlen=idx;
	return (idx==len)?true:false;
}

void DeleteSpecifyExe(TCHAR* exeName)
{
	if(exeName == NULL)
		return;
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(INVALID_HANDLE_VALUE == hProcess)
		return;
	BOOL fPOK = Process32First(hProcess,&pe);
	for(; fPOK;fPOK = Process32Next(hProcess,&pe))
	{
		if(strstr(pe.szExeFile,exeName) != NULL)
		{
			HANDLE hPr = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID );
			if( hPr == NULL )
				return;
			else
			{
				TerminateProcess(hPr,0);
			}
		}
	}
	if(INVALID_HANDLE_VALUE != hProcess)
		CloseHandle(hProcess);
}

bool RunTelnetExe(char *ip)
{
	//vDUT_RETURN ret = vDUT_ERR_OK;

	char szRunTelnetExeFullName[MAXBUFSIZE] = "";

	sprintf_s(szRunTelnetExeFullName, MAXBUFSIZE, "telnetenable %s 000000000001 Gearguy Geardog", ip);
	
	STARTUPINFO si;
	GetStartupInfo(&si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi;

	if(!CreateProcess(NULL,szRunTelnetExeFullName,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
	{
		printf("Create process for %s failed\n",szRunTelnetExeFullName);
		return false;
	}

	if(WAIT_TIMEOUT == WaitForSingleObject(pi.hProcess,5000))
	{
		TerminateProcess(pi.hProcess,1);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		printf("WaitForSingleObject %s failed\n",szRunTelnetExeFullName);
		return false;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}
bool SendSocketCmd(char *cmd, char *ret, int timeout)
{
	try
	{
		strSocketBuf.clear();
		int bytesSent;
		int nTryNum = timeout/20 ;	
		bytesSent = send(g_socket,cmd,strlen(cmd),0);
		while(nTryNum--)
		{			
			if(bytesSent == SOCKET_ERROR)
			{
				printf("SOCKET_ERROR !\n");
				break;
			}
			if(strSocketBuf.find(ret) != -1)
			{
				return true;
			}
			Sleep(20);			
		}
		if(nTryNum == 0)
		{
			return false;
		}		
	}
	catch(...)
	{
		printf("send command failed!\n");
		return false;
	}
	
	return false;
}
#ifdef __AP__
bool SendDutCmd(char *prefix, char *cmd)
{
	char szCmd[256]="";
	sprintf_s(szCmd,sizeof(szCmd),"%s %s\n",prefix,cmd);
	if(SendSocketCmd(szCmd))
	{
		return true;
	}
	printf( "Command:%s send failed!" , szCmd );
	return false;
}
#endif
int Read_ini()
{
	int  ini_status ;
	char iniPath[MAXBUFSIZE];

	GetCurrentDirectory (MAXBUFSIZE, iniPath);
	sprintf_s(iniPath , MAXBUFSIZE , "%s\\%s" , iniPath , LP_DUT_INI_FILE);

	FILE *iniFile;
	fopen_s(&iniFile, iniPath, "r");
	if (!iniFile)
	{  
		ini_status = 0 ;
	}
	else
	{
		// if the .ini exists	

		GetPrivateProfileString("Configuration","DUT_IP", "192.168.1.1", DutIP, sizeof(DutIP), iniPath);
		ShowDebugInfor=GetPrivateProfileInt("Configuration","DEBUG_SHOW",0,iniPath);

		ini_status = 1;  
	}
	fclose(iniFile);
	return ini_status;
}

int RunSpecifyExeAndRead(char* RunFileName,bool bIsShow)
{
	//strArpOutInfo.Empty();
	SECURITY_ATTRIBUTES sat;
	STARTUPINFO startupinfo;
	PROCESS_INFORMATION pinfo;
	char buffer[1024] = "";
	DWORD byteRead =0;
	HANDLE hReadPipeARP,hWritePipeARP;

	sat.nLength=sizeof(SECURITY_ATTRIBUTES);
	sat.bInheritHandle=true;
	sat.lpSecurityDescriptor=NULL;
	
	if(!CreatePipe(&hReadPipeARP,&hWritePipeARP,&sat,NULL))
	{
		printf("Create pipe fail");
		return 0;
	}
	startupinfo.cb=sizeof(STARTUPINFO);
	GetStartupInfo(&startupinfo);
	startupinfo.hStdError =hWritePipeARP;
	startupinfo.hStdOutput=hWritePipeARP;
	startupinfo.hStdInput = hReadPipeARP;
	startupinfo.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startupinfo.wShowWindow=SW_HIDE;
	
	if(!CreateProcess(NULL,RunFileName,NULL, NULL, TRUE, NULL, NULL, NULL,&startupinfo,&pinfo))
	{
		char error[128] = "";
		sprintf_s(error, 128, "Run %s application fail", RunFileName);
		printf(error);
		return 0;
	}

	if(WAIT_TIMEOUT == WaitForSingleObject(pinfo.hProcess,20000))
	{
		TerminateProcess(pinfo.hProcess,1);
		CloseHandle(hWritePipeARP);
		CloseHandle(pinfo.hProcess);
		CloseHandle(pinfo.hThread);
		CloseHandle(hReadPipeARP);
		char error[128] = "";
		sprintf_s(error, 128, "Run %s application can't complete", RunFileName);
		printf(error);
		return -1;
	}
	CloseHandle(hWritePipeARP);
	CloseHandle(pinfo.hProcess);
	CloseHandle(pinfo.hThread);

	while(TRUE)
	{
		memset(buffer, 0, 1024);
		Sleep(100);
		//BOOL bResult = ReadFile(hReadPipeARP,buffer,1024,&byteRead,NULL);
		if( byteRead == 0 ) 
			break;
		//strArpOutInfo+=buffer;
		Sleep(200);
		if(bIsShow)
			printf(buffer);	
	}
	
	CloseHandle(hReadPipeARP);
	return 1;
}

#ifdef __CARD__
bool SendDutCmd(char *cmd, char *ret, int iTimeout)
{
	HANDLE hPipeRead, hPipeWrite;
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa,sizeof(SECURITY_ATTRIBUTES));
	sa.nLength=sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor=NULL;
	sa.bInheritHandle=true;
	bool bRet = false;
	strSocketBuf.clear();
	char g_szBuf[4096]="";

	try
	{
		BOOL bRet = CreatePipe(&hPipeRead,&hPipeWrite,&sa,0);
		if(!bRet)
		{
			printf("CreatePipe failed!\n");
			return 0;
		}
	}
	catch(char *msg)
	{
		printf("%s\n",msg);
		return 0;
	}     

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	ZeroMemory(&si,sizeof(STARTUPINFO));
	ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));
	GetStartupInfo(&si);

	si.cb=sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdOutput=hPipeWrite;
	si.hStdError=hPipeWrite;
	si.wShowWindow=SW_SHOWNORMAL;

	bRet = CreateProcess(NULL,
		cmd,
		NULL,
		NULL,
		true,
		0,
		NULL,
		NULL,
		&si,
		&pi);
	if(!bRet)
	{
		printf("CreateProcess failed! GetLastError()=%d\n",GetLastError());
		return 0;
	}
	printf("%s\n",cmd);	// show the test commands.
	WaitForSingleObject(pi.hProcess, INFINITE);
	
	CloseHandle(pi.hThread);
	CloseHandle(hPipeWrite);
	CloseHandle(pi.hProcess);
	DWORD dwRead;
	strSocketBuf.clear();
	//int iTimeout = 1000;

	while(true)
	{
		if(ReadFile(hPipeRead,g_szBuf,sizeof(g_szBuf), &dwRead,NULL))
		{
			//Form1->MsgShow((AnsiString)g_szBuf);
			printf("%s",g_szBuf);	// print the 
			strSocketBuf+=g_szBuf;
		}
		else
		{
			break;
		}
		//Sleep(100);
	}

	if(ret == NULL)
		return true;

	if(-1 != strSocketBuf.find(ret))
		return true;
	else
	{
		return false;
	}
}
#endif
int GetAntCombineNum(int Ant1, int Ant2, int Ant3, int Ant4)
{
	int nSum=0;
	if(Ant1)	nSum+=1;
	if(Ant2)	nSum+=2;
	if(Ant3)	nSum+=4;
	if(Ant4)	nSum+=8;

	return nSum;

}
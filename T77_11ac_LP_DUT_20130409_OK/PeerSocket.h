// PeerSocket.h: interface for the PeerSocket.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
//#include "GetAdapterInfo.h"
#if !defined(AFX_PEERSOCKET_H__674A623F_4C93_4AE9_84A7_568658FA724E__INCLUDED_)
#define AFX_PEERSOCKET_H__674A623F_4C93_4AE9_84A7_568658FA724E__INCLUDED_


#ifndef LP_DUT_INI_FILE
	#define LP_DUT_INI_FILE "LP_DUT_setup.ini"
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define PEER_STREAM SOCK_STREAM
#define PEER_DGRAM  SOCK_DGRAM
#define PEER_RAW	SOCK_RAW
void ambit_WSACleanup();
bool Initialize_WSA();
bool InitSocket(TCHAR *ip,int &ErrorType);
bool PingSpecifyIP(TCHAR* IP,int &ErrorType,int nFailCount=35);
bool PeerCreateSocket(SOCKET *pNewSocket,int iSockType);
bool PeerBindSocket(SOCKET BindSocket,TCHAR *szHostAddr,int iHostPort);
bool PeerConnectSocket(SOCKET BindSocket,TCHAR *szHostAddr,int iHostPort);
bool PeerListenSocket(SOCKET ListenSocket);
bool PeerShutDownSocket(SOCKET nowSocket);
bool PeerCloseSocket(SOCKET nowSocket);
bool PeerSendData(SOCKET socket,TCHAR *data,DWORD len,DWORD *retlen,WSAEVENT hSendEvent,WSAEVENT hExitEvent,DWORD time=WSA_INFINITE);
bool PeerRecvData(SOCKET socket,TCHAR *data,DWORD len,DWORD *retlen,WSAEVENT hRecvEvent,WSAEVENT hExitEvent,DWORD time=WSA_INFINITE);
bool PeerSendDataS(SOCKET socket,TCHAR *data,DWORD len,DWORD *retlen,WSAEVENT hSendEvent,WSAEVENT hExitEvent,DWORD time=WSA_INFINITE);
bool PeerRecvDataS(SOCKET socket,TCHAR *data,DWORD len,DWORD *retlen,WSAEVENT hRecvEvent,WSAEVENT hExitEvent,DWORD time=WSA_INFINITE);
bool RunTelnetExe(char *ip);
void DeleteSpecifyExe(TCHAR* exeName);
bool SendSocketCmd(char *cmd, char *ret="#", int timeout=10000);
bool SendDutCmd(char *prefix, char *cmd);
int  Read_ini ();
int RunSpecifyExeAndRead(char* RunFileName,bool bIsShow);
#endif // !defined(AFX_PEERSOCKET_H__674A623F_4C93_4AE9_84A7_568658FA724E__INCLUDED_)

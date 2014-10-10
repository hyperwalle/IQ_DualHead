#include "stdafx.h"
#include "IQxstream_API.h"
#include "IQlite_Logger.h"

//#define SCPI_LOG_CONSOLE_PRINT false


// supporting functions only used in this file  ----------------------------------------------------
extern int splitAsDoubles(char* stringToSplit, double *doubleArray, int dataNumExpected = 0, char * delimiters = ",");

#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }



static void set_color(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);
}


CIQxstream::CIQxstream(void)
{
	testerConnected         = false;            //
	connectSocket           = INVALID_SOCKET;   //
	statusSocket            = 0;
	bEnableDebugMessage     = false;            //
	bEnableScpiKeyPrint     = false;             // print original SCPI key input
	bEnableTesterStatePrint = true;             // print Tester state before updating or adding
	bLastScpiSkipped        = false;            //
	bBypassCheckLocal       = false;            // bypass tester state checking from local map
	testerType              = "NA";             //
	testerSerialNumber      = "";               //
	testerSoftwareVersion   = "";               //
	scpiLogFileName         = "log_scpi.txt";   // save scpi related log to this file

	CreatScpiMap();                             // create SCPI map
	testerStateMap.clear();                     // clear testerStateMap
	resultHistory.clear();                      // clear test results

	scpiRxBuffer = (char*)malloc(SCPI_RX_BUFFER_SIZE*sizeof(char)); // allocate memory for scpiRxBuffer

	int scpiSeparated       = 0;                // whether the scpi log should be saved separted
	LOGGER_GetLoggerSourceLevel(LOG_SCPI_SEPARATED, &scpiSeparated);
	if(scpiSeparated == 0)
		scpiLogFileName ="log_all.txt";
	else
		scpiLogFileName = scpiLogFileName;      //do nothing, means that scpiLogFileName = "log_scpi.txt"

	m_iCmdDataSize = 0;

}

CIQxstream::~CIQxstream(void)
{
	free(scpiRxBuffer);       // clear memory
	resultHistory.clear();
	testerStateMap.clear();
	ScpiMap.clear();

	if (connectSocket!=INVALID_SOCKET)
	{
		// shutdown the winsock connection
		shutdown(connectSocket, SD_SEND);

		// cleanup
		closesocket(connectSocket);
		WSACleanup();
	}
}

void CIQxstream::LogPrintf(const char *format, ...)
{
	if (!bEnableDebugMessage)       // en/disable IQxstream scpi log print
		return;

	va_list ap;
	va_start(ap, format);

	_mkdir("log");

	// create log file
	FILE *fp;
	string tmpFilePath = "log/";
	tmpFilePath.append(scpiLogFileName);
	fp = fopen(tmpFilePath.c_str(), "a+");
	//fp = fopen("log/log_scpi.txt", "a+");
	if (fp)
	{
		vfprintf(fp, format, ap);    // print to log file
		fclose(fp);
	}
	vprintf(format, ap);            // print to console screen
	va_end(ap);
}


int CIQxstream::SendCommand(char *command, char* cmd2, bool scpiKeyUsed, bool checkStatus, int iCmdSize)
{
	int             rxLen = 0;
	int             rxLenTot = 0;
	//char            buffer[MAX_COMMAND_LEN]         = {0};
	char			*buffer = NULL;
	buffer = new char [iCmdSize];


	char            sockBuffer[SOCK_BUFFER_SIZE]    = {0};
	unsigned int    timeDiff  = 0;
	int             have2Send = 1; // Whether current scpi cmd will be sent to the tester; 0 = no, 1 = yes, 2 = yes and first user

	bLastScpiSkipped = false;

	// ---------------------------------------
	// used to bypass the status check from local map
	// ---------------------------------------
	if(bBypassCheckLocal)
		checkStatus = false;

	//-----------------------------------------
	// check tester state from stateMap
	//-----------------------------------------
	if(scpiKeyUsed&&checkStatus)
	{
		have2Send = CheckLocalState(command,cmd2,checkStatus);  // check whether the scpi command should be sent to the tester
		if(have2Send < 0)                                       // error in function call, and report error
		{
			set_color(RED);
			LogPrintf("ERROR in CheckLocalState()!!! \n\n");
			set_color(WHITE);
			return have2Send;
		}
		else if (have2Send == 0)                                // don't need to change tester status, and don't need to send scpi command
		{
			//LogPrintf("Info: the same state, no need to change! \n");
			bLastScpiSkipped =  true;                           // update class member variable to indicate that last SCPI cmd is skipped!
			return have2Send;
		}
		else
		{
			//do nothing!                                          // tester status needs to be changed, and program will go on
		}
	}

	//-----------------------------------------
	// processing input scpi/key to one string
	//-----------------------------------------
	if(scpiKeyUsed)
	{
		string strCmd  = ConvertToScpi(command);
		strcpy(buffer,strCmd.c_str());
	}
	else
	{
		if( strstr(command, "MMEM:DATA") )
		{
			memcpy(buffer, command, m_iCmdDataSize);
		}
		else
		{
			strcpy(buffer, command);
		}
	}

	if (strlen(buffer)==0)
	{
		return 0;                                                   // return if the input SCPI command is empty
	}

	if (strstr(cmd2,"?"))
	{
		strcat(buffer,cmd2);
	}
	else if (!strcmp(cmd2,""))
	{	// do nothing
	}
	else
	{
		strcat(buffer, " ");
		strcat(buffer, cmd2);
	}


	if( strstr(command, "MMEM:DATA") )
	{
		//Add carriage return
		if( buffer[m_iCmdDataSize-1] != '\n' )
		{
			char cCarriageReturn[2];
			sprintf_s(cCarriageReturn, "%c", '\n' );
			memcpy(buffer + m_iCmdDataSize, cCarriageReturn, 1);
		}

	}
	else
	{
		if (buffer[strlen(buffer)-1]!='\n')
		{
			strcat(buffer, "\n");                                       // if last character is not a carriage return, adding one
		}
	}
	//-----------------------------------------
	// Send SCPI command
	//-----------------------------------------
	if (bEnableDebugMessage)
	{
		if (scpiKeyUsed && bEnableScpiKeyPrint)                     // print the orignal SCPI keyword format inputs
		{
			set_color(YELLOW);
			LogPrintf("[SCPI] [KEY] : %s %s\n", command,cmd2);
		}

		if(scpiKeyUsed&&checkStatus&&bLastScpiSkipped)              // skip printing SCPI commands
		{//do nothing
		}
		else
		{
			set_color(MEGNETA);
			LogPrintf("[SCPI] INPUT : %s", buffer);                 // print SCPI Commands
		}

		set_color(WHITE);
	}
	_ftime64(&timerStart);


	if( strstr(command, "MMEM:DATA") )
		statusSocket = send( connectSocket, buffer, m_iCmdDataSize+1, 0 ); // command, data, and carriage return
	else
		statusSocket = send( connectSocket, buffer, (int)strlen(buffer), 0 );
	if (statusSocket == SOCKET_ERROR)
	{
		LogPrintf("Winsock send failed with error: %d\n", WSAGetLastError());
		memset (scpiRxBuffer,'\0',SCPI_RX_BUFFER_SIZE);             // clear response buffer
		return FAIL;
	}
	// printf("  SCPI buffer is %s %s::%s::%d \n", buffer, __FILE__, __FUNCTION__, __LINE__);

	//-----------------------------------------
	// Get response message
	//-----------------------------------------

	//Terence: It will result in beep in QA_11AC @@
	if ( strstr(buffer, "?") && !strstr(command, "MMEM:DATA"))
		//if ( strstr(buffer, "?") )
	{
		int readN =0;
		//Sleep(200);
		//rxLen = recv(connectSocket, scpiRxBuffer, SCPI_RX_BUFFER_SIZE, 0);
		_ftime64(&timerStart);
		unsigned int timeOut = 10000;                               // a timeout for reading scpi return from sockeet

		memset (scpiRxBuffer,'\0',SCPI_RX_BUFFER_SIZE);             // clear response buffer
		while (timeDiff <= timeOut )
		{
			readN++;
			memset (sockBuffer,'\0',SOCK_BUFFER_SIZE);
			rxLen = recv(connectSocket, sockBuffer, SOCK_BUFFER_SIZE-1, 0);
			//printf("readN= %d, rxLen = %d,-->[%s]\n",readN, rxLen,buffer);
			rxLenTot += rxLen;
			if (rxLenTot>SCPI_RX_BUFFER_SIZE)
			{
				printf("\n\nSCPI_RX_BUFFER_SIZE is too small !!!\n\n");
				return rxLenTot - rxLen;
			}
			strcat(scpiRxBuffer,sockBuffer);

			if(strstr(sockBuffer,"\n"))                              // stop reading socket until a '\n' is caught
			{
				//if (bEnableDebugMessage)
				//    printf("FOUND: readN= %d, rxLen = %d \n",readN, rxLen);
				break;
			}

			_ftime64(&timerStop);
			timeDiff = (unsigned int) ((timerStop.time - timerStart.time) * 1000 + (timerStop.millitm - timerStart.millitm));   // current past time
		}

		_ftime64(&timerStop);
		timeDiff = (unsigned int) ((timerStop.time - timerStart.time) * 1000 + (timerStop.millitm - timerStart.millitm));       // get execution time

		if ( strlen(scpiRxBuffer) > 0 )
		{
			if (bEnableDebugMessage)
			{
				set_color(CYAN);
				LogPrintf("[SCPI] OUTPUT (%4u) : %s\n", timeDiff, scpiRxBuffer);
			}
		}
		else if ( rxLen == 0 )
		{
			set_color(RED);
			LogPrintf("Connection closed\n");
		}
		else
		{
			set_color(RED);
			LogPrintf("recv failed with error: %d\n", WSAGetLastError());
		}
		set_color(WHITE);
	}
	else
	{
		memset (scpiRxBuffer,'\0',SCPI_RX_BUFFER_SIZE);// clear response buffer
	}

	//-----------------------------------------
	// Update tester stateMap
	//-----------------------------------------
	if(scpiKeyUsed&checkStatus)
	{
		UpdateLocalState(command,cmd2,checkStatus,scpiRxBuffer);
	}

	SAFE_DELETE_ARRAY(buffer);
	return rxLenTot;
}

// Jarir added this to send only commands, no receiving, need to improve, this is only needed when issuing "VSA; CAPT:DATA?", the return is processed by new functions inside IQmeasure_SCPI.cpp which should move here
int CIQxstream::SendCommand_only(char *command, char* cmd2, bool scpiKeyUsed)
{
	int rxLen = 0;
	int rxLenTot = 0;
	char buffer[MAX_COMMAND_LEN];
	unsigned int timeDiff = 0;
	strcpy(buffer, command);
	if(scpiKeyUsed)
	{
		string strCmd  = ConvertToScpi(buffer);
		strcpy(buffer,strCmd.c_str());
	}

	if (strlen(buffer)==0)
		return 0;

	strcat(buffer,cmd2);

	// if last character is not a carriage return, adding one
	if (buffer[strlen(buffer)-1]!='\n')
		strcat(buffer, "\n");

	//-----------------------------------------
	// Send SCPI command
	//-----------------------------------------
	set_color(MEGNETA);
	if (bEnableDebugMessage)
		LogPrintf("[SCPI] INPUT : %s", buffer);

	statusSocket = send( connectSocket, buffer, (int)strlen(buffer), 0 );
	if (statusSocket == SOCKET_ERROR)
	{
		printf("Winsock send failed with error: %d\n", WSAGetLastError());
		return FAIL;
	}
	return rxLenTot;
}


bool CIQxstream::ConnectTester(char *ipAddress)
{
	// printf("  ipAddress is %s %s::%s::%d \n", ipAddress,__FILE__, __FUNCTION__, __LINE__);
	WSADATA wsaData;
	struct addrinfo *result = NULL, *ptr, hints;

	// Initialize Winsock
	statusSocket = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (statusSocket != 0)
	{
		printf("WSAStartup failed with error: %d\n", statusSocket);
		return FAIL;
	}

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and portxxx
	statusSocket = getaddrinfo(ipAddress, IQXSTREAM_PORT, &hints, &result);
	if ( statusSocket != 0 )
	{
		printf("getaddrinfo failed with error: %d\n", statusSocket);
		WSACleanup();
		return FAIL;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr=result; ptr != NULL ;ptr=ptr->ai_next)
	{

		// Create a SOCKET for connecting to server
		connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);

		//int iOptVal = 0;
		//int iOptLen = sizeof(int);

		BOOL bOptVal = TRUE;
		int bOptLen = sizeof(BOOL);

		// Jarir 12/12/11 setting socket options here to remove any network delay
		//status = getsockopt(connectSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &iOptVal, &iOptLen);
		statusSocket = setsockopt(connectSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &bOptVal, bOptLen);
		//status = getsockopt(connectSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &iOptVal, &iOptLen);
		if (statusSocket == SOCKET_ERROR)
		{
			printf("Failed to set socket options with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return FAIL;
		}

		if (connectSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return FAIL;
		}
		else
		{
			statusSocket = connect( connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);   // Connect to server.
			if (statusSocket != SOCKET_ERROR)
				break;
			else
			{
				closesocket(connectSocket);
				connectSocket = INVALID_SOCKET;
			}
		}
	}

	// frees address information that the getaddrinfo function dynamically allocates in its addrinfo structures
	freeaddrinfo(result);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		return FAIL;
	}
	else
	{
		if (SendCommand("*IDN?\n"))                         // Get Tester version string
		{
			char *pch = NULL;
			pch = strtok(scpiRxBuffer,",");                 // first section is always "LitePoint"!
			if(pch!= NULL)
			{
				if(strstr(pch,"LitePoint")!= NULL)
				{
					testerConnected = true;
				}
			}
			else
			{
				testerConnected = false;
				return FAIL;
			}

			pch = strtok(NULL,",");                         // second section is IQtester TYPE
			if(pch!= NULL)
				testerType = pch;
			else
			{
				SetColor(RED);
				printf("\n\n Error in getting tester type !!!\n\n");
				SetColor(WHITE);
			}

			pch = strtok(NULL,",");                         // third section is IQtester serial number
			if(pch!= NULL)
				testerSerialNumber = pch;
			else
			{
				SetColor(RED);
				printf("\n\n Error in getting tester serial number !!!\n\n");
				SetColor(WHITE);
			}

			pch = strtok(NULL,",");                         // fourth section is software version number
			if(pch!= NULL)
				testerSoftwareVersion = pch;
			else
			{
				SetColor(RED);
				printf("\n\n Error in getting tester serial number !!!\n\n");
				SetColor(WHITE);
			}
		}
	}

	return testerConnected;
}

// Jarir added DisconnectTester to close socket connection, 12/14/11
int CIQxstream::DisconnectTester()
{
	//int status = 0;
	statusSocket = closesocket(connectSocket);

	if (statusSocket != 0)
	{
		printf("Socket close failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return FAIL;
	}
	else
	{
		printf("Socket close successful\n");
		testerConnected = false;
	}
	return testerConnected;

	//WSADATA wsaData;
	//struct addrinfo *result = NULL, *ptr, hints;

	//// Initialize Winsock
	//status = WSAStartup(MAKEWORD(2,2), &wsaData);
	//if (status != 0)
	//{
	//	printf("WSAStartup failed with error: %d\n", status);
	//	return FAIL;
	//}

	//ZeroMemory( &hints, sizeof(hints) );
	//hints.ai_family = AF_UNSPEC;
	//hints.ai_socktype = SOCK_STREAM;
	//hints.ai_protocol = IPPROTO_TCP;

	//// Resolve the server address and portxxx
	//status = getaddrinfo(ipAddress, IQXSTREAM_PORT, &hints, &result);
	//if ( status != 0 )
	//{
	//	printf("getaddrinfo failed with error: %d\n", status);
	//	WSACleanup();
	//	return FAIL;
	//}

	//// Attempt to connect to an address until one succeeds
	//for (ptr=result; ptr != NULL ;ptr=ptr->ai_next)
	//{

	//	// Create a SOCKET for connecting to server
	//	connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
	//		ptr->ai_protocol);

	//	//int iOptVal = 0;
	//	//int iOptLen = sizeof(int);

	//	BOOL bOptVal = TRUE;
	//	int bOptLen = sizeof(BOOL);

	//	// Jarir 12/12/11 setting socket options here to remove any network delay
	//	//status = getsockopt(connectSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &iOptVal, &iOptLen);
	//	status = setsockopt(connectSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &bOptVal, bOptLen);
	//	//status = getsockopt(connectSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &iOptVal, &iOptLen);
	//	if (status == SOCKET_ERROR)
	//	{
	//		printf("Failed to set socket options with error: %ld\n", WSAGetLastError());
	//		WSACleanup();
	//		return FAIL;
	//	}

	//	if (connectSocket == INVALID_SOCKET)
	//	{
	//		printf("socket failed with error: %ld\n", WSAGetLastError());
	//		WSACleanup();
	//		return FAIL;
	//	}
	//	else
	//	{
	//		// Connect to server.
	//		status = connect( connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	//		if (status != SOCKET_ERROR)
	//			break;
	//		else
	//		{
	//			closesocket(connectSocket);
	//			connectSocket = INVALID_SOCKET;
	//		}
	//	}
	//}

	// frees address information that the getaddrinfo function dynamically allocates in its addrinfo structures
	//freeaddrinfo(result);

	//if (connectSocket == INVALID_SOCKET)
	//{
	//	printf("Unable to connect to server!\n");
	//	WSACleanup();
	//	return FAIL;
	//}
	//else
	//{
	//	// Get Tester version string
	//	if (SendCommand("*IDN?\n"))
	//	{
	//		if (strstr(scpiRxBuffer, "IQxstream"))
	//			testerConnected = TRUE;
	//	}
	//}

	//return testerConnected;
}


string CIQxstream::ConvertToScpi(char *cmdKey)
{
	string scpiCmd = "";

	map<string, string>::iterator cmdIter;
	cmdIter = ScpiMap.find(cmdKey);

	if(cmdIter != ScpiMap.end())
		scpiCmd = cmdIter->second;

	return scpiCmd;
}

//int CIQxstream::getLocalState(char *scpiCmdKey, char *parameters)
//{
//    int retErr = -1;
//    return retErr;
//}


// 12/29/12, added by Jacky,
// Check from the local map to find whether the tester status need to be changed based on the current input SCPI,
// if needed, return 1, otherwise 0;
// If the scpi keyword corresponding satus cannot be found in the map, it will return 2 (instead of 1).
// If other exceptions happen, return negative values, -1 or less than -1;
int  CIQxstream::CheckLocalState(char *scpiCmdKey, char *parameters, bool checkStatus)
{
	int retErr              = -1;                   /* 0 = cmd state found and the same as expected;
	/* 1 = cmd state found, but not the same as expected;
	/* 2 = cmd state cannot be found;
	/* -1,-2,... for exceptions        */

	int rxLen               = 0;                    // scpi return length
	char * pch              = NULL;                 //
	string testerStateName  = scpiCmdKey;
	string targetParaStr    = parameters;
	string currentParaStr   = "";
	string queryStr         = "?";

	if(!checkStatus)
		return 1;                                   // bypass the following operation and make sure the scpi is sent

	if (strstr(scpiCmdKey, "ROUT_PORT_RES"))
	{
		testerStateName     += "_Q";
		queryStr            =  "";
	}
	else
	{
		queryStr = "?";                             // query string remains "?"
	}

	map<string, string>::iterator it;
	it = testerStateMap.find(testerStateName);      // find input cmd tester state in the tester state map


	if (it==testerStateMap.end())                   // tester state doesn't exist in local map, return 1
	{
		retErr = 2;
	}
	else                                            // tester state exists in stateMapp, then compare it with expected state
	{
		string currentParaStr = it->second;         // erase '\n' in currentParaStr
		size_t found = currentParaStr.find('\n');
		if(found !=string::npos)
			currentParaStr.erase(found,found+1);

		found = targetParaStr.find('\n');           // erase '\n' in targetParaStr
		if(found !=string::npos)
			targetParaStr.erase(found,found+1);

		if(targetParaStr.compare(currentParaStr)== 0) // the same
		{
			retErr = 0;
			sprintf_s(scpiRxBuffer,SCPI_RX_BUFFER_SIZE,"%s",currentParaStr.c_str()); // copy to scpiRxBuffer return
		}
		else if(atof(currentParaStr.c_str()) && atof(targetParaStr.c_str()) ) // check double value are the same
		{
			double currVal = atof(currentParaStr.c_str());
			double targVal = atof( targetParaStr.c_str());
			if (currVal == targVal)
			{
				retErr = 0; // double values match,
				sprintf_s(scpiRxBuffer,SCPI_RX_BUFFER_SIZE,"%s",currentParaStr.c_str()); // copy to scpiRxBuffer return
			}
			else
				retErr = 1;    // double values don't match,
		}
		else
		{
			retErr = 1;
		}
	}

	return retErr;
}

// 12/29/12, added by Jacky,
// update the local tester status table
int CIQxstream::UpdateLocalState(char *scpiCmdKey, char *parameters, bool checkStatus, char *scpiReturn)
{
	// in two conditions, it is required to update the local state map
	// 1) scpi state name exists, but has different values
	// 2) scpi state name doesnot exist, need to insert
	// Before updating the local state, need to query from the tester directly

	int     retErr          = 0;
	int     rxLen           = 0;                                                    // scpi return length
	char    *pch            = NULL;                                                 //
	string  testerStateName = scpiCmdKey;
	string  targetParaStr   = parameters;
	string  currentParaStr  = "";
	string  queryStr        = "?";

	if(!checkStatus)
		return 0;                                                                   // bypass all following operation if checkStatus is disable


	if (strstr(scpiCmdKey, "ROUT_PORT_RES"))
	{
		testerStateName     += "_Q";                                                //
		queryStr            =  "";                                                  //
	}
	else
	{
		queryStr = "?";                                                              // query string remains "?"
	}

	//** query state from the tester directly
	SendCommand((char*)testerStateName.c_str(),(char*)queryStr.c_str(),true,false); // cmd directly to tester
	if(targetParaStr.empty())
	{
		if(0==strcmp(scpiReturn,""))
			targetParaStr = scpiReturn;
		else
			targetParaStr = scpiRxBuffer;
	}

	//** find insert point in stateMap
	map<string, string>::iterator it;
	it = testerStateMap.find(testerStateName);                                      // find input cmd tester state in the tester state map
	if (it==testerStateMap.end())                                                   // tester state doesn't exist in local map, insert a new one
	{
		testerStateMap.insert(pair<string,string>(testerStateName, targetParaStr));
		if (bEnableDebugMessage && bEnableTesterStatePrint)                           // print updated tester state
		{
			set_color(LIGHT_GREEN);
			LogPrintf("[SCPI] [%s] is added to testerStateMap! SIZE(%d) \n", testerStateName.c_str(),testerStateMap.size());
			LogPrintf("[SCPI] [%s] = %s \n", testerStateName.c_str(),targetParaStr.c_str());
			set_color(WHITE);
		}
	}
	else                                                                            // tester state exists, and update its value
	{
		if (bEnableDebugMessage && bEnableTesterStatePrint)                           // print tester state, before updating
		{
			set_color(LIGHT_GREEN);
			LogPrintf("[SCPI] [%s] = %s \n", testerStateName.c_str(),it->second.c_str());
		}
		it->second.assign(targetParaStr.c_str());                                   // update tester status in local state map


		if (bEnableDebugMessage && bEnableTesterStatePrint)                           // print tester state, after updating
		{
			LogPrintf("[SCPI] [%s] is updated to: %s \n", testerStateName.c_str(),it->second.c_str());
			set_color(WHITE);
		}
	}

	// double check from the map, which is not necessary
	int statusTmp = CheckLocalState(scpiCmdKey, parameters,true);                      // double check the status
	if(statusTmp != 0)                                                                 // update status fails!
	{
		set_color(RED);
		LogPrintf(("ERROR:UpdateLocalState() fails !!!\n"));
		set_color(WHITE);
		retErr = -1;
	}

	return retErr;
}


void CIQxstream::ClearTesterStateMap(void)
{
	testerStateMap.clear();     // clear testerStateMap
}

void CIQxstream::CreatScpiMap(void)
{
	ScpiMap.clear();

	// SYS related SCPI commands //////////////////////////////////////////////////////////////////////////////////////////////
	ScpiMap.insert(pair<string, string>("SYS_MIDN",			        "SYS;MIDN"));               // RETURN 1 STRING [MODULE_IDEN_STRING]
	ScpiMap.insert(pair<string, string>("SYS_IDN",			        "*IDN"));                   // RETURN 1 STRING
	ScpiMap.insert(pair<string, string>("SYS_OPC",			        "*OPC"));                   // RETURN 1 BOOL
	ScpiMap.insert(pair<string, string>("SYS_WAI",			        "*WAI"));                   // NO RETURN
	ScpiMap.insert(pair<string, string>("SYS_CLS",			        "*CLS"));                   // NO RETURN
	ScpiMap.insert(pair<string, string>("SYS_RST",			        "*RST"));                   // NO RETURN
	ScpiMap.insert(pair<string, string>("SYS_ERR",			        "SYST:ERR:ALL"));           // RETURN 2 [INT, STRING]
	//ScpiMap.insert(pair<string, string>("DEVICE_CLEAR",		    "DCL:HMOD"));               // NO RETURN  < ... to be deleted .... >
	ScpiMap.insert(pair<string, string>("SYS_CLEAR_MODULE",	        "SYS;DCL:HMOD"));           // NO RETURN
	ScpiMap.insert(pair<string, string>("SYS_FORM_DATA",	        "SYS;FORM:READ:DATA"));     // INPUT: 1 STRING, [(ASC/PACK)]
	ScpiMap.insert(pair<string, string>("SYS_CAPT_STOR",	        "SYS;CAPT:STOR"));          // INPUT: 1 STRING [FILENAME_IN_QUOTES]
	ScpiMap.insert(pair<string, string>("SYS_AMOD",			        "SYS;AMOD"));               // NO INPUT
	ScpiMap.insert(pair<string, string>("SYS_CAPT_STORE",			"SYS;CAPT:STOR"));          // eg: CAPT:STOR "File_Name" , "Description_of_Capture"

	//ROUT related SCPI commands //////////////////////////////////////////////////////////////////////////////////////////////
	ScpiMap.insert(pair<string, string>("ROUT_MIDN",			    "ROUT;MIDN"));              // RETURN 1 STRING [MODULE_IDEN_STRING]
	ScpiMap.insert(pair<string, string>("ROUT_MSAV",		        "ROUT;MSAV"));              // INPUT: [INT]
	ScpiMap.insert(pair<string, string>("ROUT_MRCL",		        "ROUT;MRCL"));              // INPUT: [INT]
	ScpiMap.insert(pair<string, string>("ROUT_PORT_RES_RF1A",		"ROUT;PORT:RES RF1A,"));    // INPUT: 2 STRING, [RF_PORT, VSA_VSG_MODULE]
	ScpiMap.insert(pair<string, string>("ROUT_PORT_RES_RF2A",		"ROUT;PORT:RES RF2A,"));    // INPUT: 2 STRING, [RF_PORT, VSA_VSG_MODULE]
	ScpiMap.insert(pair<string, string>("ROUT_PORT_RES_RF3A",		"ROUT;PORT:RES RF3A,"));    // INPUT: 2 STRING, [RF_PORT, VSA_VSG_MODULE]
	ScpiMap.insert(pair<string, string>("ROUT_PORT_RES_RF4A",		"ROUT;PORT:RES RF4A,"));    // INPUT: 2 STRING, [RF_PORT, VSA_VSG_MODULE]
	ScpiMap.insert(pair<string, string>("ROUT_PORT_RES_STRM1A",		"ROUT;PORT:RES STRM1A,"));  // INPUT: 2 STRING, [RF_PORT, VSA_VSG_MODULE]
	ScpiMap.insert(pair<string, string>("ROUT_PORT_RES_RF1A_Q",		"ROUT;PORT:RES? RF1A"));    // INPUT: 2 STRING, [RF_PORT, VSA_VSG_MODULE]
	ScpiMap.insert(pair<string, string>("ROUT_PORT_RES_RF2A_Q",		"ROUT;PORT:RES? RF2A"));    // INPUT: 2 STRING, [RF_PORT, VSA_VSG_MODULE]
	ScpiMap.insert(pair<string, string>("ROUT_PORT_RES_RF3A_Q",		"ROUT;PORT:RES? RF3A"));    // INPUT: 2 STRING, [RF_PORT, VSA_VSG_MODULE]
	ScpiMap.insert(pair<string, string>("ROUT_PORT_RES_RF4A_Q",		"ROUT;PORT:RES? RF4A"));    // INPUT: 2 STRING, [RF_PORT, VSA_VSG_MODULE]
	ScpiMap.insert(pair<string, string>("ROUT_PORT_RES_STRM1A_Q",	"ROUT;PORT:RES? STRM1A"));  // INPUT: 2 STRING, [RF_PORT, VSA_VSG_MODULE]

	//VSG related SCPI commands //////////////////////////////////////////////////////////////////////////////////////////////
	ScpiMap.insert(pair<string, string>("VSG_MIDN",			        "VSG1;MIDN"));              // RETURN 1 STRING [MODULE_IDEN_STRING]
	ScpiMap.insert(pair<string, string>("VSG_FREQ_LOOF",	        "VSG1;FREQ:LOOF"));         // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSG_FREQ",			        "VSG1;FREQ"));              // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSG_TRIG_SOUR",	        "VSG1;TRIG:SOUR"));         // INPUT 1 STRING
	ScpiMap.insert(pair<string, string>("VSG_TRIG_MODE",	        "VSG1;TRIG:MODE"));         // INPUT 1 STRING
	ScpiMap.insert(pair<string, string>("VSG_TRIG_SLOPE",	        "VSG1;TRIG:SLOP"));         // INPUT 1 STRING
	ScpiMap.insert(pair<string, string>("VSG_TRIG_TYPE",	        "VSG1;TRIG:TYPE"));         // INPUT 1 STRING
	ScpiMap.insert(pair<string, string>("VSG_SRATE",		        "VSG1;SRAT"));              // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSG_WAVE_EXEC",	        "VSG1;WAVE:EXEC"));         // INPUT 1(2) [ON/OFF (, WSEG#)]
	ScpiMap.insert(pair<string, string>("VSG_WAVE_DEL_ALL",	        "VSG1;WAVE:DEL:ALL"));      // NO INPUT
	ScpiMap.insert(pair<string, string>("VSG_WAVE_GEN_CW",	        "VSG1;WAVE:GEN:CWAV"));     // INPUT 2 DOUBLE
	ScpiMap.insert(pair<string, string>("VSG_WAVE_LOAD",	        "VSG1;WAVE:LOAD"));         // INPUT 1 STRING [FILE_NAME_IN_QUOTES]
	ScpiMap.insert(pair<string, string>("VSG_WLIS_WSEG1_DATA",	    "VSG1;WLIS:WSEG1:DATA"));   // USE WSEG1 NOW, CHANGE LATER? // INPUT 1 STRING [FILE_NAME_IN_QUOTES]
	ScpiMap.insert(pair<string, string>("VSG_WLIS_WSEG1_NEXT",	    "VSG1;WLIS:WSEG1:NEXT"));   // USE WSEG1 NOW, CHANGE LATER? // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSG_WLIS_WSEG1_LENG",	    "VSG1;WLIS:WSEG1:LENG"));   // USE WSEG1 NOW, CHANGE LATER? // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSG_WLIS_WSEG1_START",	    "VSG1;WLIS:WSEG1:STAR"));   // USE WSEG1 NOW, CHANGE LATER? // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSG_WLIS_WSEG1_REPEAT",	"VSG1;WLIS:WSEG1:REP"));    //
	ScpiMap.insert(pair<string, string>("VSG_WLIS_WSEG1_SAVE",	    "VSG1;WLIS:WSEG1:SAVE"));   // USE WSEG1 NOW, CHANGE LATER? // NO INPUT
	ScpiMap.insert(pair<string, string>("VSG_WLIS_COUNT",		    "VSG1;WLIS:COUNT"));        // INPUT 1 INT
	ScpiMap.insert(pair<string, string>("VSG_WLIS_COUNT_ENABLE",    "VSG1;WLIS:COUNT:ENABLE")); // INPUT 1 STRING [WSEG#]
	ScpiMap.insert(pair<string, string>("VSG_WLIS_COUNT_DISABLE",   "VSG1;WLIS:COUNT:DISABLE"));// INPUT 1 STRING [WSEG#]
	ScpiMap.insert(pair<string, string>("VSG_WLIS_CAT_DEL_ALL",	    "VSG1;WLIS:CAT:DEL:ALL"));  // NO INPUT
	ScpiMap.insert(pair<string, string>("VSG_POW_STATUS",		    "VSG1;POW:STAT"));          // INPUT 1 STRING [ON/OFF]
	ScpiMap.insert(pair<string, string>("VSG_POW_LEV",			    "VSG1;POW:LEV"));           // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSG_POW_ALC",			    "VSG1;POW:ALC"));           // INPUT 1 STRING [SING/AUTO/OFF]
	ScpiMap.insert(pair<string, string>("VSG_POW_ALC_CORR",			"VSG1;POW:ALC:CORR"));      // INPUT 1 DOUBLE


	//VSA related SCPI commands //////////////////////////////////////////////////////////////////////////////////////////////
	ScpiMap.insert(pair<string, string>("VSA_MIDN",			        "VSA1;MIDN"));              // RETURN 1 STRING [MODULE_IDEN_STRING]
	ScpiMap.insert(pair<string, string>("VSA_FREQ_LOOF",	        "VSA1;FREQ:LOOF"));         // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSA_FREQ",			        "VSA1;FREQ"));              // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSA_TRIG_SOUR",	        "VSA1;TRIG:SOUR"));         // INPUT 1 STRING
	ScpiMap.insert(pair<string, string>("VSA_TRIG_MODE",	        "VSA1;TRIG:MODE"));         // INPUT 1 STRING
	ScpiMap.insert(pair<string, string>("VSA_TRIG_SLOPE",	        "VSA1;TRIG:SLOP"));         // INPUT 1 STRING
	ScpiMap.insert(pair<string, string>("VSA_TRIG_TYPE",	        "VSA1;TRIG:TYPE"));         // INPUT 1 STRING
	ScpiMap.insert(pair<string, string>("VSA_SRATE",		        "VSA1;SRAT"));              // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSA_RLEV",			        "VSA1;RLEV"));              // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSA_RLEV_AUTO",			"VSA1;RLEV:AUTO"));         // NO INPUT
	ScpiMap.insert(pair<string, string>("VSA_TRIG_LEV",		        "VSA1;TRIG:LEV"));          // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSA_TRIG_TIME",	        "VSA1;TRIG:TIM"));          // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSA_TRIG_OFFS",	        "VSA1;TRIG:OFFS"));         // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSA_CAPT_TIME",	        "VSA1;CAPT:TIME"));         // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("VSA_INIT",			        "VSA1;INIT"));              // NO INPUT
	ScpiMap.insert(pair<string, string>("VSA_INIT_SPEC_WIDE",       "VSA1;INIT:SPEC:WIDE"));    // NO INPUT
	ScpiMap.insert(pair<string, string>("VSA_CAPT_DATA",	        "VSA1;CAPT:DATA"));         // OUTPUT BINARY ARB
	ScpiMap.insert(pair<string, string>("VSA_MEAS_COUP",	        "VSA1;MEAS:COUP"));         // OUTPUT BINARY ARB

	//VSA MPS related SCPI commands /////////////////////////////////////////////////////////////////////////////////////////////
	ScpiMap.insert(pair<string, string>("VSA_MPS_SPC",				"VSA1;MPS:SPC"));			// INPUT 1 UNSIGNED INTEGER
	// VSA MPS Skip Packet Count
	ScpiMap.insert(pair<string, string>("VSA_MPS_CCO",				"VSA1;MPS:CCO"));			// INPUT 1 UNSIGNED INTEGER
	// VSA MPS Capture Packet Count
	ScpiMap.insert(pair<string, string>("VSA_MPS_CTIM",				"VSA1;MPS:CTIM"));			// INPUT 1 DOUBLE
	// VSA MPS Capture Time

	//VSG MPS related SCPI commands /////////////////////////////////////////////////////////////////////////////////////////////
	ScpiMap.insert(pair<string, string>("VSG_MPS_TPC",				"VSG1;MPS:TPC"));			// INPUT 1 UNSIGNED INTEGER
	// MPS Transmit Packet Count
	ScpiMap.insert(pair<string, string>("VSG_MPS_PAT",				"VSG1;MPS:PAT"));			// INPUT 1 DOUBLE
	// MPS Packet Acknowledge Timeout
	ScpiMap.insert(pair<string, string>("VSG_MPS_TIME",				"VSG1;MPS:TIME"));			// INPUT 1 DOUBLE
	// MPS Mode Specific Time
	ScpiMap.insert(pair<string, string>("VSG_MPS_PDEL",				"VSG1;MPS:PDEL"));			// INPUT 1 DOUBLE
	// MPS Packet Delay Time

	//BT related SCPI commands //////////////////////////////////////////////////////////////////////////////////////////////
	ScpiMap.insert(pair<string, string>("BT_MIDN",			        "BT;MIDN"));                // RETURN 1 STRING [MODULE_IDEN_STRING]
	ScpiMap.insert(pair<string, string>("BT_MRST",			        "BT;MRST"));                // RETURN 1 STRING [MODULE_IDEN_STRING]
	ScpiMap.insert(pair<string, string>("BT_CONF_PDET_GAP",		    "BT;CONF:PDET:GAP"));       // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("BT_CONF_PDET_THR",		    "BT;CONF:PDET:THR"));       // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("BT_CONF_IFBW",			    "BT;CONF:IFBW"));           // INPUT 1 DOUBLE, FOLLOWED BY KHZ
	ScpiMap.insert(pair<string, string>("BT_CONF_LESW",			    "BT;CONF:LESW"));           // INPUT 1 HEX STRING, FOLLOWED BY #h
	ScpiMap.insert(pair<string, string>("BT_CONF_DRAT",			    "BT;CONF:DRAT"));           // INPUT 1 (AUTO/1MBPS/2MBPS/4MBPS/LEN)
	ScpiMap.insert(pair<string, string>("BT_CLEAR_ALL",			    "BT;CLE:ALL"));             // NO INPUT
	ScpiMap.insert(pair<string, string>("BT_CALC_POW",			    "BT;CALC:POW"));            // INPUT 2 INT
	ScpiMap.insert(pair<string, string>("BT_CALC_SPEC",			    "BT;CALC:SPEC"));           // INPUT 2 INT
	ScpiMap.insert(pair<string, string>("BT_CALC_TXQ",			    "BT;CALC:TXQ"));            // INPUT 2 INT
	ScpiMap.insert(pair<string, string>("BT_CALC_ACP",			    "BT;CALC:ACP"));            // INPUT 2 INT
	ScpiMap.insert(pair<string, string>("BT_CALC_ALL",			    "BT;CALC:ALL"));            // INPUT 2 INT
	ScpiMap.insert(pair<string, string>("BT_FETC_SYNC",			    "BT;FETC:SYNC"));           // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_DDR",		    "BT;FETC:TXQ:DDR"));        // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_POW",			    "BT;FETC:POW"));            // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_POW_PEAK",		    "BT;FETC:POW:PEAK"));       // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_POW_PEAK_MAX",	    "BT;FETC:POW:PEAK:MAX"));   // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_POW_AVER",		    "BT;FETC:POW:AVER"));       // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_ACP",			    "BT;FETC:ACP"));            // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_ACP_CNGP",		    "BT;FETC:ACP:CNGP"));       // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_ACP_COFF",		    "BT;FETC:ACP:COFF"));       // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_SYNC_PCOM",	    "BT;FETC:SYNC:PCOM"));      // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_SYNC_PST",		    "BT;FETC:SYNC:PST"));       // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_SYNC_PET",		    "BT;FETC:SYNC:PET"));       // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_SPEC_20BW",	    "BT;FETC:SPEC:20BW"));      // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_EDR",		    "BT;FETC:TXQ:EDR"));        // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_EDR_FTIM",	    "BT;FETC:TXQ:EDR:FTIM"));   // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_EDR_EVMT",	    "BT;FETC:TXQ:EDR:EVMT"));   // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_CLAS",		    "BT;FETC:TXQ:CLAS"));       // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_LEN",		    "BT;FETC:TXQ:LEN"));        // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_LEN_ISOF",	    "BT;FETC:TXQ:LEN:ISOF"));   // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_LEN_DF2M",	    "BT;FETC:TXQ:LEN:DF2M"));   // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_LEN_PAYL",	    "BT;FETC:TXQ:LEN:PAYL"));   // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_LEN_PRE",	    "BT;FETC:TXQ:LEN:PRE"));    // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_LEN_SWOR",	    "BT;FETC:TXQ:LEN:SWOR"));   // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_LEN_PDUH",	    "BT;FETC:TXQ:LEN:PDUH"));   // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_LEN_PDUL",	    "BT;FETC:TXQ:LEN:PDUL"));   // OUTPUT
	ScpiMap.insert(pair<string, string>("BT_FETC_TXQ_LEN_CRC",	    "BT;FETC:TXQ:LEN:CRC"));    // OUTPUT

	//WIFI related SCPI commands //////////////////////////////////////////////////////////////////////////////////////////////
	ScpiMap.insert(pair<string, string>("WIFI_CONF_PDET_GAP",	    "WIFI;CONF:PDET:GAP"));     // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("WIFI_CONF_PDET_THR",	    "WIFI;CONF:PDET:THR"));     // INPUT 1 DOUBLE
	ScpiMap.insert(pair<string, string>("WIFI_FETC_POW",		    "WIFI;FETC:POW"));          // OUTPUT
	ScpiMap.insert(pair<string, string>("WIFI_FETC_POW_PEAK",	    "WIFI;FETC:POW:PEAK"));     // OUTPUT
	ScpiMap.insert(pair<string, string>("WIFI_FETC_POW_PEAK_MAX",   "WIFI;FETC:POW:PEAK:MAX")); // OUTPUT
	ScpiMap.insert(pair<string, string>("WIFI_FETC_POW_AVER",	    "WIFI;FETC:POW:AVER"));     // OUTPUT
	ScpiMap.insert(pair<string, string>("WIFI_CALC_POW",		    "WIFI;CALC:POW"));          // INPUT 2 INT
	ScpiMap.insert(pair<string, string>("WIFI_CLEAR_ALL",		    "WIFI;CLE:ALL"));           // NO INPUT
	ScpiMap.insert(pair<string, string>("WIFI_FETC_SYNC",		    "WIFI;FETC:SYNC"));         // OUTPUT

	//GPRF related SCPI commands //////////////////////////////////////////////////////////////////////////////////////////////
	ScpiMap.insert(pair<string, string>("GPRF_CLEAR_ALL",	        "GPRF;CLE:ALL"));           //
	ScpiMap.insert(pair<string, string>("GPRF_CONF_MLEN",	        "GPRF;CONF:MLEN"));         //
	ScpiMap.insert(pair<string, string>("GPRF_CONF_ILEN",	        "GPRF;CONF:ILEN"));         //
	ScpiMap.insert(pair<string, string>("GPRF_CALC_PHAS_INT",	    "GPRF;CALC:PHAS:INT"));     //
	ScpiMap.insert(pair<string, string>("GPRF_FETC_PHAS_FERR",	    "GPRF;FETC:PHAS:FERR"));    //

	//other SCPI commands //////////////////////////////////////////////////////////////////////////////////////////////
	ScpiMap.insert(pair<string, string>("DBUF_MODE",		        "DBUF:MODE"));              // INPUT: [ON/OFF] < ... not used, to be deleted >
	ScpiMap.insert(pair<string, string>("MOD",			            "MOD"));                    // RETURN 1 STRING [(BT/WIFI/SYS/ROUT)]

}

void CIQxstream::DebugEnable(bool enable)
{
	bEnableDebugMessage = enable;
}

bool CIQxstream::GetLogDebug(void)
{
	return bEnableDebugMessage;
}

bool CIQxstream::GetLogScpiKey(void)
{
	return bEnableScpiKeyPrint;
}

bool CIQxstream::GetLogTesterState(void)
{
	return bEnableTesterStatePrint ;
}

bool CIQxstream::IsLastScpiSkipped(void)
{
	return bLastScpiSkipped;
}

string CIQxstream::GetTesterType(void)
{
	return testerType;
}
string CIQxstream::GetTesterSerialNumber(void)
{
	return testerSerialNumber;
}

string CIQxstream::GetTesterSoftwareVersion(void)
{
	return testerSoftwareVersion;
}

void CIQxstream::SetColor(int color)
{
	set_color(color);
}

double CIQxstream::GetScalars(char *measurement, int index, int *status,int *numValues, int numExpected,  bool keyUsed)
	//                          char *measurement, int index, int *status, int *numValues, int numExpected, bool keyUsed
{
	int     rxLen               = 0;
	double  retDoubleVal        = NA_NUMBER;
	string  currentMeasurement  = measurement;

	map<string, vector<double>>::iterator it1;

	it1 = resultHistory.find(currentMeasurement);
	if ((it1!=resultHistory.end()))
	{
		//printf("[GetScalars()] ------------> Fetch command found in history\n");
		int fetchLen = (int) it1->second.size();
		if (fetchLen > (index+1))
		{
			*status = (int) it1->second.at(0);
			*numValues = fetchLen;
			retDoubleVal = it1->second.at(index+1);
		}
		else
		{
			//printf("[GetScalars()] ------------> Upto %d results available, requested index = %d\n", fetchLen, index);
			*numValues = fetchLen;
			*status = -1;
		}
	}
	else
	{
		vector<double> doubleVector;
		if (keyUsed)
			SendCommand(measurement, "?", keyUsed);
		else
			SendCommand(measurement);
		static double doubleArray[MAX_DATA_LENGTH] = {0.0};
		int parsedValues = splitAsDoubles(scpiRxBuffer, doubleArray, numExpected);
		doubleVector.assign(doubleArray, doubleArray+parsedValues);
		resultHistory.insert(pair<string, vector<double>>(measurement, doubleVector));
		*status = (int) doubleArray[0];
		*numValues = parsedValues;
		if (parsedValues > (index+1))
			retDoubleVal = doubleArray[index+1];
		else
			retDoubleVal = NA_NUMBER;
	}
	return retDoubleVal;
}


void CIQxstream::ClearResultHistory(void)
{
	resultHistory.clear();
}

void CIQxstream::RemoveTesterState(char *scpiCmdKey)
{
	map <string, string>::iterator it;
	it = testerStateMap.find(scpiCmdKey);
	if(it!=testerStateMap.end())
	{
		testerStateMap.erase(it);
	}
}

int CIQxstream::SaveVsaCaptureToLocal(char *fileNameNoExtension, char *fileDIR )
{
	int             retErr      = 1; // PASS
	int             rxLen       = 0;
	char            buffer[SOCK_BUFFER_SIZE] =  {0};
	unsigned int    timeDiff    = 0;
	string          fileNameOnTester = "\"capture/_store4copy.iqvsa\"";             // a temp file will be saved to tester

	//----------------------------------------------------------------------
	// save current capture to tester first
	//----------------------------------------------------------------------
	SendCommand("SYS_CAPT_STORE", (char*)fileNameOnTester.c_str(),true);    // save the current capture inside tester as "_store4copy.iqvsa"

	//----------------------------------------------------------------------
	// create a local file for write
	//----------------------------------------------------------------------
	string captureFileFullName = "";
	if(strlen(fileDIR)!=0)
	{
		captureFileFullName.append(fileDIR);
		captureFileFullName.append("/");
	};
	captureFileFullName.append(fileNameNoExtension);    //adding file name

	size_t found = captureFileFullName.find(".iqvsa");
	if(found == string::npos)
	{
		found = captureFileFullName.find(".sig");
		if(found!=string::npos)
			captureFileFullName.replace(found, strlen(".sig"),".iqvsa");           //adding file extension
		else
			captureFileFullName.append(".iqvsa");           //adding file extension
	}

	FILE* fp = NULL;
	fp = fopen(captureFileFullName.c_str(), "wb+");
	if(!fp)
	{
		LogPrintf("\n ERROR in creating the new file!!! [insded function CIQxstream::SaveVsaCaptureToLocal()]\n\n");
		return FAIL;
	}

	//----------------------------------------------------------------------
	// read the temp file on tester and write to the local file
	//----------------------------------------------------------------------
	char scpiCmd[MAX_COMMAND_LEN] = {0};
	sprintf_s(scpiCmd,MAX_COMMAND_LEN,"%s %s\n", "SYS;MMEM:DATA?" ,fileNameOnTester.c_str()); //MMEM:DATA? "File_Name"

	//SendCommand(scpiCmd);
	LogPrintf("[SCPI] INPUT: %s\n", scpiCmd);
	statusSocket = send( connectSocket, scpiCmd, (int)strlen(scpiCmd), 0 );
	if (statusSocket == SOCKET_ERROR)
	{
		LogPrintf("Winsock send failed with error: %d\n", WSAGetLastError());
		return FAIL;                                                        // 0 indicates FAIL
	}

	// Get response message and save to a local file ///////////////////
	unsigned int timeOut = 10000;                                   // a timeout for reading scpi return from sockeet
	_ftime64(&timerStart);
	memset (buffer,'\0',SOCK_BUFFER_SIZE);
	rxLen = recv(connectSocket, buffer, SOCK_BUFFER_SIZE-1, 0);
	int firstDigit      = (int)(buffer[1]-'0');                     // get the first digit
	int numChar2Copy    = atoi(buffer+2);                           // number of characters to copy
	int headerLen       = firstDigit+2;                             // number of characters in the header section: eg: #6901359
	int rxLenLeft       = numChar2Copy;                             // used for control reading
	if(rxLen>headerLen)
	{
		fwrite(buffer+headerLen, sizeof(char),rxLen-headerLen, fp);
		rxLenLeft = rxLenLeft-(rxLen-headerLen);
	}
	while(rxLenLeft>0  && timeDiff <= timeOut)
	{
		memset(buffer,0,SOCK_BUFFER_SIZE);
		int lenRead = rxLenLeft>(SOCK_BUFFER_SIZE-1)?(SOCK_BUFFER_SIZE-1):rxLenLeft;
		rxLen = recv(connectSocket, buffer, lenRead, 0);
		fwrite(buffer, sizeof(char),rxLen, fp);
		rxLenLeft = rxLenLeft-rxLen;
		_ftime64(&timerStop);
		timeDiff = (unsigned int) ((timerStop.time - timerStart.time) * 1000 + (timerStop.millitm - timerStart.millitm));   // current past time
	}
	fclose(fp); //close iqvsa file

	_ftime64(&timerStop);
	timeDiff = (unsigned int) ((timerStop.time - timerStart.time) * 1000 + (timerStop.millitm - timerStart.millitm));       // get execution time
	if(timeDiff >timeOut)
	{
		set_color(RED);
		LogPrintf("Winsock read timeout error !!!\n");
		set_color(WHITE);
		return FAIL;
	}

	LogPrintf("Capture file is saved as %s\n", (char*)captureFileFullName.c_str());

	return retErr;
}


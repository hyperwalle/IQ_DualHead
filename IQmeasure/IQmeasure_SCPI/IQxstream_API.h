
#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <direct.h>
#include <sys/timeb.h>
#include <time.h>
#include "string"
using namespace std;
#include "IQlite_Logger.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma warning(disable : 4996)		// turn off deprecated warning for remaining ones


//#define BREAK_ON_ERROR(err) {ViStatus ec = err; if(ec != VI_SUCCESS){ break; } }

#define SCPI_RX_BUFFER_SIZE		40960000
#define MAX_COMMAND_LEN			1024
#define SOCK_BUFFER_SIZE        4096
#define IQXSTREAM_PORT			"24000"

#define MAX_CMD_DATA_SIZE       204800000

#ifndef MAX_DATA_LENGTH
#define MAX_DATA_LENGTH     60000
#endif
#ifndef NA_NUMBER
#define NA_NUMBER   -99999.99
#endif

enum {FAIL, PASS};
enum {BLACK, BLUE, GREEN, CYAN, RED, MEGNETA, YELLOW, WHITE, GREY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, LIGHT_RED, LIGHT_MAGENTA, LIGHT_YELLOW, LIGHT_WHITE};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CIQxstream                                                                        // class defined to control testers with scpi commands
{
	public:     //** public functions here please ! **
		CIQxstream(void);                                                                   //
		~CIQxstream(void);                                                                  //
		bool    ConnectTester(char *ipAddress);                                             // Connect IQxstream tester through IP address.
		int     DisconnectTester(void);                                                     // Close socket connection to the tester.
		//int   SendCommand(char *command);                                                 // Send SCPI commands to the tester.
		int     SendCommand(char *command       ,                                           // Send SCPI commands to the tester, direct scpi or scpi keyword based.
				char *cmd2       = "",                                          //
				bool scpiKeyUsed = false,                                       // -- whether to use SCPI key input
				bool checkStatus = false,                                       // -- whether to check tester status table (maintained locally) before sending commands,
				int iCmdSize = MAX_COMMAND_LEN
				);                                                              //
		int     SendCommand_only(char *command, char* cmd2 ="", bool scpiKeyUsed= false);   // Jarir included this function only for SaveVsaSignalFile, can be removed later, after some functions moved from IQmeasure_SCPI.cpp to IQxstream_API.cpp, 12/9/11
		void    LogPrintf(const char *format, ...);                                         // Print log information to log file and console screen.
		void    ClearTesterStateMap(void);                                                  // Clear testerStateMap
		void    RemoveTesterState(char *scpiCmdKey);                                        // remove a center tester state from local tester state map
		void    DebugEnable(bool enable = true);                                            // turn on SCPI related debug information if enable = true, otherwise disable
		bool    GetLogDebug(void);                                                          // return a bool flag that controls log
		bool    GetLogScpiKey(void);                                                        // return a bool flag that controls Scpi-key log
		bool    GetLogTesterState(void);                                                    // return a bool flag that controls tester-state log
		bool    IsLastScpiSkipped(void);                                                    // return private member variable "bLastScpiSkipped"
		void    SetColor(int color);                                                        // set console printing color
		string  GetTesterType(void);                                                        // get the current tester's tester TYPE
		string  GetTesterSerialNumber(void);                                                // get the current tester's serial number
		string  GetTesterSoftwareVersion(void);                                             // get the current tester's software version
		double  GetScalars( char    *measurement,                                           //
				int     index,                                                  //
				int     *status,                                                //
				int     *numValues,                                             //
				int     numExpected     = 0,                                    //
				bool    keyUsed         = false                                 //
				);                                                              //
		void    ClearResultHistory(void);                                                   // clear test results history
		int     SaveVsaCaptureToLocal(char *fileNameNoExtension ="vsaCapture",              // save vsa capture to a local pc with the input file name
				char *fileDIR = "");                               // file directory
	public:     //** public variables here please! **
		SOCKET  connectSocket;                                                              //
		char    *scpiRxBuffer;                                                              // Save most recent SCPI command return values, its size is [SCPI_RX_BUFFER_SIZE]
		map <string, string> testerStateMap;                                                // a map that store current tester status
		int     m_iCmdDataSize;

	private:    //** private functions here! **
		void    CreatScpiMap(void);                                                         // Create a map that stores all supported SCPI commands and their corresponding SCPI_keywords.
		string  ConvertToScpi(char *cmdKey);                                                // Convert SCPI key to SCPI command.
		int     UpdateLocalState(char *scpiCmdKey,                                          // update tester state map
				char *parameters,                                          //
				bool checkStatus = false,                                  //
				char *scpiReturn = ""                                      // the scpi return for current scpi command
				);                                                          //
		int     CheckLocalState(char *scpiCmdKey,                                           // check from the tester state map to see whether the tester status need to be changed
				char *parameters,                                           //
				bool checkStatus = false                                    //
				) ;                                                         //
		//int     getLocalState(char *scpiCmdKey, char *parameters);                        // get tester status from local state map, it may change to a PUBLIC:

	private:    //** private member variables here: **
		int     statusSocket;                                                                     // used only in scpi socket communications
		bool    testerConnected;                                                            // Denote whether there is a socket-connection to the tester.
		bool    bEnableDebugMessage;                                                        // flag to enable/disable scpi related log print to console/file, true to enable.
		bool    bEnableScpiKeyPrint;                                                        // flag to enable/disable scpi KEY input log to file/console , (1) to enable.
		bool    bEnableTesterStatePrint;                                                    // print tester state before/after adding/updating tester state map
		bool    bLastScpiSkipped;                                                           // whether the last scpi command is bypassed by local-status-check;
		bool    bBypassCheckLocal;                                                          // bypass checking tester status from local status map
		struct  __timeb64   timerStart;                                                     //
		struct  __timeb64   timerStop;                                                      //
		map<string, string> ScpiMap;                                                        // SCPI command map that stores all supported SCPI commands and their corresponding keys.
		map<string, vector<double>>  resultHistory;                                         // measurement result history
		string  testerType;                                                                 // current tester type used
		string  testerSerialNumber;                                                         // current tester serial number
		string  testerSoftwareVersion;                                                      // current tester software version
		string  scpiLogFileName;

};
///////////////////////// END OF IQxstream_API.h////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// TODO: remove load vsg waveform workaround which bypasses error -200
// TODO: check all double print to string, because %f may not be suitable for many, explicitly mention %.5f etc. or use %e
// TODO: check all temporary strings to see if it is memset to NULL before new string is printed to it


#include "stdafx.h"
//#include "LP_Driver_Lib.h"
//Move to stdafx.h
//#include "lp_stdio.h"
#include "iqapi.h"
#include "..\IQmeasure.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"
#include "StringUtil.h"
//Move to stdafx.h
//#include "lp_string.h"
#include <float.h> // DBL_MAX on Mac
#include "IQmeasure_Scpi.h"
#include "IQxstream_API.h"
//#include "IQmeasure_IQAPI.h"

//#if IQMEASURE_SCPI_CONTROL_ENABLE
//#ifdef INSTRUMENT_DRIVER_EXPLICIT_LINK
//    HINSTANCE g_insDrvDllHndl;
//#else
//    #include "LP_DLL.h"
//    #pragma comment(lib, "LP_DLL.lib")
//#endif

#ifndef MAX_BUFFER_LEN
#define MAX_BUFFER_LEN 4096
#endif
#ifndef MAX_COMMAND_LEN
#define MAX_COMMAND_LEN 1024
#endif

#ifndef SCPI_NA_NUMBER
#define SCPI_NA_NUMBER 9.91e37
#endif
#ifndef SCPI_INF
#define SCPI_INF 9.9e37
#endif
#ifndef SCPI_NINF
#define SCPI_NINF -9.9e37
#endif



#define CONNECTION_PORT "24000"
#define CONNECTION_TIMEOUT 1000
#define MAX_TESTER_NUMBER 4
#ifndef MAX_DATA_LENGTH
#define MAX_DATA_LENGTH 6000
#endif

#define IQXS_MAX_SAMPLING_RATE  150e6
#define IQXS_BT_SAMPLING_RATE   37.5e6
#define DTNA_MAX_SAMPLING_RATE  160e6
#define DTNA_BT_SAMPLING_RATE   80e6

// Jarir added 12/9/11 for reading LE bit-stream length; if IQxstream is using 64 bit system, this needs to be 64: TODO
#define BIT_NUM 32

int g_session = 0;
//extern double *bufferReal;   // JK, don't know why it was here .... removed 01/12/12
//extern double *bufferImag;    // JK, don't know why it was here .... removed 01/12/12

extern iqapiHndl *hndl;
// #define __FN_TEST(_LP_FN)											\
//     rc = ##_LP_FN;													\
//     printf("  %d = ",rc);											\
//     printf(#_LP_FN);												\
//     printf("\n");													\
//     rc = LP_DLL_Error_Retrieve(session, cval);						\
//     memset(resp_string, 0x0, LP_LENGTH_IDENTITY);					\

CIQxstream  iqx;                                // obj to manage scpi communications with IQxtream
bool        g_checkLocal = false;                // always check tester state from local map before sending out scpi command to tester


// Jarir, 12/9/11, removed trigger type map, now relevant functions take in iqapi compatible trigger values and set Trigger with a switch statement
// map<int,string> triggerSourceMap;

// Vsg port connection to which RF port, maintained globally to avoid resetting vsg port
string g_strVsgPort;  // can be RF1A,RF2A,RF3A,RF4A

// Vsa port connection to which RF port, maintained globally to avoid resetting vsg port
string g_strVsaPort;  // can be RF1A,RF2A,RF3A,RF4A

// Jarir added global to indicate if last capture failed, 12/9/11
// to avoid any analysis if capture failed, on success return 0;
// TODO: only BT analysis is being avoided if capture failed; no need to worry if this is not checked before calling any other analysis function!
int g_lastCaptureFailed = -1;

// Jarir added global 12/9/11, to indicate current BT analysis type, cleared each time new analysis starts;
// TODO: maybe improve process and remove this global later.
char analysisTypeBt[50] = {'\0'};

//// Jarir added global map for fetch history, 12/9/11, to minimize fetch commands. Cleared each time a new analysis is performed after which new results can be fetched.
//map <string, vector<double>> g_bt_fetchHistory;

//// Jarir added 12/13/11, global map to store current instrument state in the code, to avoid any unnecessary new setting of modules
//map <string, string> g_instrumentState;

// Jarir added global BT analysis data rate, 12/9/11; to allow BT auto-detected data rate to work properly with FETC commands; since FETC commands have dependency on rate
// cleared or set each time new analysis takes place; TODO: improve and remove if possible?
double g_btAnalysisDataRate = -1;

// Jarir added global enum 12/9/11; being used by vsadatacapture, analyzepower and other functions which need to know what technology/sampling rate to use for analysis
// TODO: remove when these functions do not need to know sampling rate
LP_TECHNOLOGY_TYPE_ENUM g_technologyForPowerAndSampRate = TECHNOLOGY_80211AG;

// Jarir added global module name for power analysis since analyzepower function needs to know a technology module to perform power analysis
// TODO: remove when power analysis function does not need to know sampling rate or technology
char *g_moduleForPowerAnalysis = "WIFI";

// convert character array to lower case, added by Jarir; TODO: use either this or the other one below; not needed if using SCPI key; remove when everything is converted to key
char* toLowerCase(char* str);

// convert character array to lower case, added by Jacky; TODO: use either this or the other one below; not needed if using SCPI key; remove when everything is converted to key
string& tLowerCase(string& str);

// Function added by Jarir to check very large number returned from tester and return NA_NUMBER in that case
double checkDoubleValue(double retVal);

//// Function added by Jarir to check if a fetch has been performed before; if it was, then get value from current fetch history map and return
//// otherwise query tester with new fetch command, store it in fetch history and return value
//double fetchGetScalars(char *measurement, int index, int *status, int *parsedValues, int numExpected = 0, bool keyUsed = false);

//Pay extra attention, dataNumExpected includes the first value, status. thus the real "data" is dataNumExpected-1;
// Parsing function to parse as doubles from ascii return from tester
int splitAsDoubles(char* stringToSplit, double *doubleArray, int dataNumExpected = 0, char * delimiters = ",");

// Parsing function to parse as integers from ascii return from tester
int splitAsIntegers(char* stringToSplit, int *intArray, int dataNumExpected = 0,char * delimiters = ",");

// Jarir 12/9/11, adding following functions for different types of parsing double
// TODO: remove functions that are not needed, current dependencies are on BT get scalar, vector and string measurements only; not sure about WiFi
// added by Jarir, 12/2/11
int splitAsDoubles2(char* stringToSplit, double *doubleArray, int *intArray, int dataNumExpected = 0, char * delimiters = ",");
// added by Jarir, 12/2/11
int splitAsDoubles3(char* stringToSplit, double *doubleArray, int dataNumExpected = 0, char * delimiters = ",");
// added by Jarir, 12/2/11
int splitAsDoubles4(char* stringToSplit, double *doubleArray, int *intArray, int *statusCode, int key, int numContained, int dataNumExpected = 0, char *delimiters = ",");

// Jarir 12/9/11, adding following functions for different types of parsing integers
// TODO: remove functions that are not needed, current dependencies are on BT get scalar, vector and string measurements only; not sure about WiFi
int splitAsIntegers2(char* stringToSplit, int *intArray, int dataNumExpected = 0,char * delimiters = ",");

int CreateMemInTester(char *referenceFile, int iFileType, char ** ppcCmd);

//// Jarir 12/9/11, added following functions to save VSA data capture to local computer
//// TODO: Move some functions to IQxstream_API.cpp
////int TesterCommunication_RecieveBinary(char ** buffer, int *buffer_length, const char * whoami);
////static int internal_waiter_TesterCommunication_Recieve(char **buffer, int *buffer_length, const char * whoami, int binary);
////static int internal_TesterCommunication_Recieve(char ** return_buffer, int *buffer_length, int binary);
////int LP_SCPI_Bin2File(   char * filename);
//int LP_getArbArrayInfo( char * data, char ** ppData, int * pnData);

/* TODO: will remove these functions evetually *//////////////////////////////////////////////////////////////
//// Jarir 12/13/11, added following functions to set/get/check instrument state with currently requested value
//// TODO: now only supporting SCPI keywords, improve to allow explicit SCPI commands too
//// setInstrumentState function: input: 'scpiCommand' as SCPI keyword, 'parameters' is desired new setting as string, scpiKeyused indicates if keyword used (12/15/11, only supporting keywords now)
//int setInstrumentState(char *scpiCommand, char *parameters, bool scpiKeyUsed = true);
//// getInstrumentState function: input: 'scpiCommand' as SCPI keyword, 'parameters' is where current state will return string, scpiKeyused indicates if keyword used (12/15/11, only supporting keywords now)
//int getInstrumentState(char *scpiCommand, char *parameters, bool scpiKeyUsed = true);
//// checkInstrumentState function: input: 'scpiCommand' as SCPI keyword, 'parameters' is desired new setting as string to check with local map, scpiKeyused indicates if keyword used (12/15/11, only supporting keywords now)
//int checkInstrumentState(char *scpiCommand, char *parameters, bool scpiKeyUsed = true);

extern bool  bIQxstreamFound;
extern bool  bIQxelFound;

bool g_11nPacketCheck = false; //if false call txq to check status, save to g_11nPacketValid
int g_11nPacketValid = 0;

#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

CIQmeasure_Scpi::CIQmeasure_Scpi(void)
{
	int isIQmeasureLog = 0;
	LOGGER_GetLoggerSourceLevel(LOG_IQMEASURE, &isIQmeasureLog);
	// LOGGER_GetLogLevel() will crash. Mark it first.
	//bool flagEnable = ((LOGGER_GetLogLevel() == LOGGER_INFORMATION) && isIQmeasureLog == 1);

	//iqx.DebugEnable(flagEnable);
	//iqx.DebugEnable(1);
}
CIQmeasure_Scpi::~CIQmeasure_Scpi(void)
{
}

int CIQmeasure_Scpi::Term(void)
{
	int err = 0;
	//err = LP_DLL_Close_Session(g_session);
	int status = iqx.DisconnectTester(); // Jarir add this here for socket closing, 12/14/11

	return err;
}

int CIQmeasure_Scpi::InitTester(char *ipAddress)
{
	int     err = 0;
	string	serialNumber = "";
	int     rxLen = 0;
	//int		status = 0;		                                        // used to get status of set instrument state through setInstrumentState function
	g_strVsgPort = "";		                                        // clear vsg port setting maintained globally
	g_strVsaPort = "";		                                        // clear vsa port setting maintained globally

	//g_instrumentState.clear();                                      // clear current instrument state table that is maintained locally
	iqx.ClearTesterStateMap();                                      // clear tester state map

	bool testConnected = iqx.ConnectTester(ipAddress);              // connect to tester
	if (!testConnected)
	{
		nTesters = 0;                                               //
		err     = ERR_NO_CONNECTION;                                // changed to right error type tag
		return err;                                                 // returning error right away if no connection
	}
	else
	{
		////rxLen = iqx.SendCommand("*IDN?\n");                       // check tester serial number
		//rxLen = iqx.SendCommand("SYS_MIDN","?",true);                // check tester serial number
		//string strScpiReturn = iqx.scpiRxBuffer;

		////need error handling
		//size_t pos = strScpiReturn.find("IQXS");
		//if (pos > 0)
		//{
		//    serialNumber = strScpiReturn.substr (pos,8); // 8 bit IQxstream serial number
		//    bIQxstreamFound = true;
		//    iqx.LogPrintf("\nIQXSTREAM SN: %s\n",serialNumber.c_str());
		//}
		//else if (pos = strScpiReturn.find("DTNA"))
		//{
		//    serialNumber = strScpiReturn.substr (pos,10); // 8 bit IQxstream serial number
		//    bIQxelFound = true;
		//    iqx.LogPrintf("\nDAYTONA SN: %s\n",serialNumber.c_str());
		//}
		//else //There might be IQxel in the future
		//{
		//    err = ERR_UNKNOWN_TESTER_TYPE; // changed to right error type tag,
		//    return err;
		//}
		string testerType = iqx.GetTesterType();        // get tester type
		string testerSN = iqx.GetTesterSerialNumber();  // get tester SN
		if(testerType.find("IQxstream")!=string::npos)
		{
			if(testerSN.find("IQXS")!=string::npos)
			{
				bIQxstreamFound = true;
				bIQxelFound   = false;
			}
			else
			{
				bIQxstreamFound = false;
				bIQxelFound   = true;
			}
		}
		else if (testerType.find("IQXFLEX")!=string::npos || testerType.find("IQXEL")!=string::npos)
		{
			bIQxstreamFound = false;
			bIQxelFound   = true;
		}
		else if (testerSN.find("DTNA")!=string::npos || testerSN.find("DAYTONA")!=string::npos)
		{
			bIQxstreamFound = false;
			bIQxelFound   = true;
		}
		else
		{
			bIQxstreamFound = false;
			bIQxelFound   = false;
			err = ERR_UNKNOWN_TESTER_TYPE; // changed to right error type tag,
			return err;
		}

		nTesters += 1;
	}

	// Clear Device modules
	rxLen = iqx.SendCommand("SYS_CLEAR_MODULE", "", true);		// DCL:HMOD
	// Save Routing Table
	rxLen = iqx.SendCommand("ROUT_MSAV", "1", true);			// ROUT:MSAV 1
	// Wait for last operations to complete
	rxLen = iqx.SendCommand("SYS_WAI", "", true);				// *WAI
	// Clear status register
	rxLen = iqx.SendCommand("SYS_CLS", "", true);				// *CLS
	// Reset Instrument
	rxLen = iqx.SendCommand("SYS_RST", "", true);				// *RST
	// Wait for last operations to complete
	rxLen = iqx.SendCommand("SYS_WAI", "", true);				// *WAI
	// Recall saved routing table
	rxLen = iqx.SendCommand("ROUT_MRCL", "1", true);			// ROUT:MRCL 1

	//Z: 12/30/2011, This will break Daytona. work around only used for IQxs for now.
	if(true==bIQxstreamFound)
	{
		//Z: 12/19/2011, this should not be the final verion, Need Engineering's clarification
		// Set VSA LO Offset to 35 MHz

		//status += setInstrumentState("VSA_FREQ_LOOF", "3.5e7", true); // VSA;FREQ:LOOF 3.5E7
		rxLen = iqx.SendCommand("VSA_FREQ_LOOF", "3.5e7", true, g_checkLocal); // VSA;FREQ:LOOF 3.5E7
		// Set VSG LO Offset to 35 MHz
		//status += setInstrumentState("VSG_FREQ_LOOF", "-3.5e7", true); // VSG;FREQ:LOOF -3.5E7
		rxLen = iqx.SendCommand("VSG_FREQ_LOOF", "-3.5e7", true, g_checkLocal); // VSG;FREQ:LOOF -3.5E7
	}

	// Set tester output data format to ASCII
	//status += setInstrumentState("SYS_FORM_DATA", "ASC", true); // SYS;FORM:READ:DATA ASC
	rxLen = iqx.SendCommand("SYS_FORM_DATA", "ASC", true, g_checkLocal); // SYS;FORM:READ:DATA ASC

	//// DBUF MODE OFF, don't know what this really does!!! -- Jarir 12/15/11
	//status += setInstrumentState("DBUF_MODE", "OFF", true);		// DBUF:MODE OFF
	//rxLen = iqx.SendCommand("DBUF_MODE", "OFF", true, g_checkLocal);		// DBUF:MODE OFF

	// Turn off all RF ports and STRM port
	//status += setInstrumentState("ROUT_PORT_RES_RF1A", "OFF", true);	// ROUT;PORT:RES RF1A, OFF
	//status += setInstrumentState("ROUT_PORT_RES_RF2A", "OFF", true);	// ROUT;PORT:RES RF2A, OFF
	//status += setInstrumentState("ROUT_PORT_RES_RF3A", "OFF", true);	// ROUT;PORT:RES RF3A, OFF
	//status += setInstrumentState("ROUT_PORT_RES_RF4A", "OFF", true);	// ROUT;PORT:RES RF4A, OFF
	//status += setInstrumentState("ROUT_PORT_RES_STRM1A", "OFF", true);	// ROUT;PORT:RES STRM1A, OFF
	rxLen = iqx.SendCommand("ROUT_PORT_RES_RF1A", "OFF", true, g_checkLocal);	// ROUT;PORT:RES RF1A, OFF
	rxLen = iqx.SendCommand("ROUT_PORT_RES_RF2A", "OFF", true, g_checkLocal);	// ROUT;PORT:RES RF2A, OFF


	if(true == bIQxstreamFound)
	{
		rxLen = iqx.SendCommand("ROUT_PORT_RES_RF3A", "OFF", true, g_checkLocal);	// ROUT;PORT:RES RF3A, OFF
		rxLen = iqx.SendCommand("ROUT_PORT_RES_RF4A", "OFF", true, g_checkLocal);	// ROUT;PORT:RES RF4A, OFF
		rxLen = iqx.SendCommand("ROUT_PORT_RES_STRM1A", "OFF", true, g_checkLocal);	// ROUT;PORT:RES STRM1A, OFF
	}
	// Turn off waveform play
	//status += setInstrumentState("VSG_WAVE_EXEC", "OFF", true);			// VSG;WAVE:EXEC OFF
	rxLen = iqx.SendCommand("VSG_WAVE_EXEC", "OFF", true, g_checkLocal);			// VSG;WAVE:EXEC OFF

	// Jarir, 12/9/11, added to remove all loaded waveforms from vsg;
	rxLen = iqx.SendCommand("VSG_WAVE_DEL_ALL", "", true);				// VSG;WAVE:DEL:ALL

	// Jarir remove condition, 01/11/12, WLIST works for Daytona starting 47336
	//if(true == bIQxstreamFound) // not work for Dayton now.
	//{
	// Jarir, 12/9/11, added to clear wave list
	rxLen = iqx.SendCommand("VSG_WLIS_CAT_DEL_ALL", "", true);			// VSG;WLIS:CAT:DEL:ALL
	//}

	// Jarir, 12/9/11, added to turn off RF after tester init
	//status += setInstrumentState("VSG_POW_STATUS", "OFF", true);		// VSG;POW:STAT OFF
	rxLen = iqx.SendCommand("VSG_POW_STATUS", "OFF", true, g_checkLocal);		// VSG;POW:STAT OFF

	// TODO: should loop here until OPC is 1
	// Check for previous operation complete flag to be 1
	rxLen = iqx.SendCommand("SYS_OPC", "?", true);						// SYS;*OPC?

	// Check tester identity again
	// TODO: is this really needed again? Jarir 12/15/11
	rxLen = iqx.SendCommand("SYS_IDN", "?", true);						// SYS;*IDN?

	// Get module list from tester, not used anywhere, maybe useful for version information
	rxLen = iqx.SendCommand("SYS_AMOD","?", true);						// AMOD?

	// Wait for previous operations to complete
	rxLen = iqx.SendCommand("SYS_WAI","",true);							// SYS;*WAI

	//// TODO: Remove as appropriate when possible, Jarir 12/15/11
	//// BT analysis parameters initialization, only needed to match with default iqapi parameters
	//// Match BT Packet Detect Gap Time with iqapi default
	//status += setInstrumentState("BT_CONF_PDGT","4e-6",true);			// BT;CONF:PDGT 4E-6
	//// Match BT IF BW to match with iqapi default
	//status += setInstrumentState("BT_CONF_IFBW","1300KHZ",true);		// BT;CONF:IFBW 1300KHZ
	//// Match BT LE Sync Word to match with iqapi default
	//double leSyncWord = 0x94826E8E;	// iqapi default value
	//char temp[MAX_COMMAND_LEN] = {'\0'};
	//sprintf_s(temp, MAX_COMMAND_LEN, "%.0f", leSyncWord);	// convert to double, to transmit hex must use #h header, ex: #h94826E8E
	//status += setInstrumentState("BT_CONF_LESW",temp,true);	//		BT;CONF:LESW #h94826E8E

	// Setup default VSA trigger type on signal, singleshot, positive edge-triggered, Jarir, 12/9/11
	// to match with iqapi
	//status += setInstrumentState("VSA_TRIG_SOUR","VID",true);		// VSA;TRIG:SOUR VID
	//status += setInstrumentState("VSA_TRIG_MODE","SSH",true);		// VSA;TRIG:MODE SSH
	//status += setInstrumentState("VSA_TRIG_SLOPE","POS",true);		// VSA;TRIG:SLOP POS
	//status += setInstrumentState("VSA_TRIG_TYPE","EDGE",true);		// VSA;TRIG:TYPE EDGE
	rxLen = iqx.SendCommand("VSA_TRIG_SOUR","VID",true, g_checkLocal);		// VSA;TRIG:SOUR VID
	rxLen = iqx.SendCommand("VSA_TRIG_MODE","SSH",true, g_checkLocal);		// VSA;TRIG:MODE SSH
	rxLen = iqx.SendCommand("VSA_TRIG_SLOPE","POS",true, g_checkLocal);		// VSA;TRIG:SLOP POS
	rxLen = iqx.SendCommand("VSA_TRIG_TYPE","EDGE",true, g_checkLocal);		// VSA;TRIG:TYPE EDGE

	// TODO: possibly check error any way, if no new setting to take effect, still check for any error, Jarir 12/15/11
	//if (status>0) // status is returned as 9999 if set value did not match get value from tester after setting instrument state, so error checking is done by default in that case
	//{
	// Wait for operations to complete
	rxLen = iqx.SendCommand("SYS_WAI", "", true);				// SYS; *WAI
	// Read whole error queue
	rxLen = iqx.SendCommand("SYS_ERR","?",true);

	// tokenize response to read only first error code; if no error, this will be 0 - because "0, No error" will be returned from tester
	char * pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
		err = ERR_NOT_INITIALIZED;
	//}
	return err;
}

int CIQmeasure_Scpi::InitTester2(char *ipAddress1, char *ipAddress2)
{
	int err = 0;
	nTesters = 2;

	return err;
}

int CIQmeasure_Scpi::InitTester3(char *ipAddress1, char *ipAddress2, char *ipAddress3)
{
	int err = 0;
	nTesters = 3;

	return err;
}

int CIQmeasure_Scpi::InitTester4(char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4)
{
	int err = 0;
	nTesters = 4;

	return err;
}

int CIQmeasure_Scpi::DualHead_ConOpen(int tokenID, char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4)
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::ConOpen(char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4)
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::DualHead_GetTokenID(int *tokenID)
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::DualHead_ConClose()
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::DualHead_ObtainControl(unsigned int probeTimeMS, unsigned int timeOutMS)
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::DualHead_ReleaseControl(void)
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::SetVsg(double rfFreqHz, double rfPowerLeveldBm, int port, bool setGapPowerOff, double dFreqShiftHz)
{

	int err     = ERR_OK;
	int rxLen   = 0;
	//int status = 0;
	char scpiCommand[MAX_COMMAND_LEN]   = {0};
	char tmpStr[MAX_COMMAND_LEN]        = {0};

	sprintf_s(tmpStr,MAX_COMMAND_LEN,"RF%d%s",port,"A");
	//vsgPort = tmpStr;
	g_strVsgPort = tmpStr;

	// TODO: Check if RF#a is already connected as VSG, if connected don't do anything
	//       If different port connected as VSG, turn it off and then set the right port as VSG
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand,MAX_COMMAND_LEN,"ROUT_PORT_RES_RF%d%s", port, "A");
	//status += setInstrumentState(scpiCommand, "OFF", true);                   // ROUT;PORT:RES RF#A, OFF
	rxLen = iqx.SendCommand(scpiCommand, "OFF", true, g_checkLocal);	        // ROUT;PORT:RES RF#A, OFF
	//status += setInstrumentState(scpiCommand, "VSG1", true);			        // ROUT;PORT:RES RF#A, VSG1
	rxLen = iqx.SendCommand(scpiCommand, "VSG1", true, g_checkLocal);	        // ROUT;PORT:RES RF#A, VSG1

	rxLen = iqx.SendCommand("VSG_MIDN","?", true);		                        // VSG1;MIDN? -- get vsg module version etc.
	//iqx.LogPrintf(iqx.scpiRxBuffer);	// print on screen,

	//status += setInstrumentState("VSG_WAVE_EXEC", "OFF", true);		        // VSG1;WAVE:EXEC OFF -- turn of waveform play
	rxLen = iqx.SendCommand("VSG_WAVE_EXEC", "OFF", true, g_checkLocal);        // VSG1;WAVE:EXEC OFF -- turn of waveform play

	// TODO: Identify technology for correct sampling rate and input here; for now, just set to max sampling rate (150e6)
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if(true==bIQxstreamFound)
	{
		sprintf_s(scpiCommand,MAX_COMMAND_LEN,"%f", IQXS_MAX_SAMPLING_RATE);
	}
	else  //assume all future tester follow Daytona
	{
		sprintf_s(scpiCommand,MAX_COMMAND_LEN,"%f", DTNA_MAX_SAMPLING_RATE);
	}
	//status += setInstrumentState("VSG_SRATE", scpiCommand, true);
	rxLen = iqx.SendCommand("VSG_SRATE", scpiCommand, true, g_checkLocal);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand,MAX_COMMAND_LEN,"%f", rfFreqHz);
	//status += setInstrumentState("VSG_FREQ", scpiCommand, true);	            // VSG1;FREQ rfFreqHz -- set VSG1 center frequency
	rxLen = iqx.SendCommand("VSG_FREQ", scpiCommand, true, g_checkLocal);	    // VSG1;FREQ rfFreqHz -- set VSG1 center frequency

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand,MAX_COMMAND_LEN,"%f", rfPowerLeveldBm);
	//status  += setInstrumentState("VSG_POW_LEV", scpiCommand, true);	        // VSG1;POW:LEV rfPowerLeveldBm -- set VSG1 power output level in dBm
	rxLen = iqx.SendCommand("VSG_POW_LEV", scpiCommand, true, g_checkLocal);	// VSG1;POW:LEV rfPowerLeveldBm -- set VSG1 power output level in dBm

	//rxLen = setInstrumentState("VSG_POW_STATUS", "ON", true);		            // VSG1;POW:STAT ON -- turn on VSG1 RF
	rxLen = iqx.SendCommand("VSG_POW_STATUS", "ON", true,g_checkLocal);		    // VSG1;POW:STAT ON -- turn on VSG1 RF

	//rxLen = iqx.SendCommand("VSG_POW_ALC", "SING", true,g_checkLocal);		    // VSG1;POW:ALC Single -- turn on ALC once

	//only use in development phase.
	//rxLen = iqx.SendCommand("SYS_WAI", "", true);                               // SYS;*WAI -- wait for previous operations
	rxLen = iqx.SendCommand("VSG_POW_ALC_CORR","?",true);	                    // VSG_POW_ALC_CORR? -- read ALC correction
	//pch = NULL;
	//pch = strtok(iqx.scpiRxBuffer, ",");
	//if (NULL == pch)
	//{
	//	err = ERR_NO_RESPONSE_FROM_TESTER;
	//	return err;
	//}
	//else
	//{
	//	atof(pch);
	//}


	char *pch = NULL;
	// Jarir remove following condition, 01/11/12, WLIST works for Daytona starting 47336
	//if (true == bIQxstreamFound) //work around for Daytona now, need to discuss with Jarir or engineering team.
	//{
	rxLen = iqx.SendCommand("VSG_WLIS_WSEG1_DATA","?", true);		        // VSG1; WLIS:WSEG1:DATA? -- Check if wavelist has any segment with waveform saved
	//char *pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, "\"");
	if ( pch != NULL ) {
		int wseg_initialized = isprint(pch[0]);
		if (wseg_initialized)
		{
			// Workaround, Jarir: if we do wave:exec on with an empty wave list, VSG outputs CW, so don't turn on wave execution
			// if nothing in wavelist, otherwise BER etc will be screwed up
			//status += setInstrumentState("VSG_WAVE_EXEC", "ON, WSEG1", true);  // VSG1;WAVE:EXEC ON, WSEG1
			rxLen = iqx.SendCommand("VSG_WAVE_EXEC", "ON, WSEG1", true,g_checkLocal);  // VSG1;WAVE:EXEC ON, WSEG1
		}
	}
	//}

	// TODO: should really do error checking, even if instrument state did not change in any way? -- Jarir 12/15/11
	//if (status>0)
	//{
	rxLen = iqx.SendCommand("SYS_WAI", "", true);                               // SYS;*WAI -- wait for previous operations
	rxLen = iqx.SendCommand("SYS_ERR","?",true);	                            // SYST:ERR:ALL? -- read error queue
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		// TODO : remove when wave files converted from mod to iqvsg do not trigger tester to throw -200 error
		// Workaround: Jarir include this part because mod files converted to iqvsg throw an error on IQxstream saying length not multiple of 8, and pads 0's
		if (atoi(pch)== -200)
		{
			g_vsgMode = ::VSG_SINGLE_MOD_FILE;
		}
		else
		{
			err = ERR_SET_TX_FAILED;
		}
	}
	else
	{
		g_vsgMode = ::VSG_SINGLE_MOD_FILE;
	}
	//}

	return err;

}

int CIQmeasure_Scpi::SetVsg_triggerType(double rfFreqHz, double rfPowerLeveldBm, int port, int triggerType)
{
	// updated by Jarir, 12/15/11, use of set/get/check instrument state local table before doing any actual setting of tester by issuing scpi command
	// updated by Jarir, 12/9/11: changed trigger type setting, instead of using previous triggermap for IQXStream, now use iqapi trigger values and map inside here
	// IMPORTANT: trigger values in technology dlls must be consistent with iqapi trigger values
	// 11/23/11 updated by Jacky

	int err             = ERR_OK;
	int rxLen           = 0;
	int status			= 0;
	char scpiCommand[MAX_COMMAND_LEN];
	char tmpStr[MAX_COMMAND_LEN];

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);

	//* check vsg port
	memset(tmpStr,'\0',MAX_COMMAND_LEN);
	sprintf_s(tmpStr,MAX_COMMAND_LEN,"RF%d%s",port,"A");
	if(g_strVsgPort.size()==0 ||strstr(g_strVsgPort.c_str(),tmpStr)==NULL)
	{
		memset(scpiCommand,'\0',MAX_COMMAND_LEN);
		// TODO: define module identifier // add strm1a/b
		sprintf_s(scpiCommand,MAX_COMMAND_LEN,"ROUT_PORT_RES_RF%d%s",port,"A");
		//status += setInstrumentState(scpiCommand, "VSG1", true);				// ROUT;PORT:RES RF#A, VG1
		rxLen = iqx.SendCommand(scpiCommand, "VSG1", true, g_checkLocal);				// ROUT;PORT:RES RF#A, VG1
		g_strVsgPort = tmpStr;			// Update global VSG port indicator
	}

	//* check vsg frequency
	if (g_bDisableFreqFilter == true)
	{
		memset(scpiCommand,'\0',MAX_COMMAND_LEN);
		sprintf_s(scpiCommand,MAX_COMMAND_LEN,"%f",rfFreqHz);
		//status += setInstrumentState("VSG_FREQ", scpiCommand, true);		// VSG1;FREQ rfFreqHz
		rxLen = iqx.SendCommand("VSG_FREQ", scpiCommand, true, g_checkLocal);		// VSG1;FREQ rfFreqHz
		g_bDisableFreqFilter = false;
	}

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand,MAX_COMMAND_LEN,"%f",rfPowerLeveldBm);
	//status += setInstrumentState("VSG_POW_LEV", scpiCommand, true);		// VSG1;POW:LEV rfPowerLeveldBm
	rxLen = iqx.SendCommand("VSG_POW_LEV", scpiCommand, true, g_checkLocal);		// VSG1;POW:LEV rfPowerLeveldBm

	// trigger type functions modified by Jarir, 12/7/11
	// Below four will be default trigger settings
	string targetTrigType = "EDGE";
	string targetTrigMode = "SSH";
	string targetTrigSlope = "POS";
	string targetTrigSource = "VID";

	// below cases are mapped with iqapi trigger type value
	/* // iqapi enum values:
	   IQV_VSG_TRIG_FREE_RUN   = 0,      //!< Indicates immediate VSG transmission after wave download
	   IQV_VSG_TRIG_EXT_2      = 2,      //!< Indicates VSG transmission triggered by external trigger input 2
	   IQV_VSG_TRIG_EXT_1      = 1       //!< Indicates VSG transmission triggered by external trigger input 1
	   */
	// TODO: use actual iqapi trigger type enum values?
	switch (triggerType)
	{
		case 0:
			targetTrigType		= "EDGE";
			targetTrigMode		= "SSH";
			targetTrigSlope		= "POS";
			targetTrigSource	= "IMM";
			break;
		case 1:
			targetTrigType		= "EDGE";
			targetTrigMode		= "SSH";
			targetTrigSlope		= "POS";
			targetTrigSource	= "EXT1";
			break;
		case 2:
			targetTrigType		= "EDGE";
			targetTrigMode		= "SSH";
			targetTrigSlope		= "POS";
			targetTrigSource	= "EXT2";
			break;
		default:
			break;
	}

	//status += setInstrumentState("VSG_TRIG_SOUR", (char*)targetTrigSource.c_str(), true);		// VSG1;TRIG:SOUR [source]
	//status += setInstrumentState("VSG_TRIG_MODE", (char*)targetTrigMode.c_str(), true);			// VSG1;TRIG:MODE [mode]
	//status += setInstrumentState("VSG_TRIG_SLOPE", (char*)targetTrigSlope.c_str(), true);		// VSG1;TRIG:SLOP [slope]
	//status += setInstrumentState("VSG_TRIG_TYPE", (char*)targetTrigType.c_str(), true);			// VSG1;TRIG:TYPE [type]

	rxLen = iqx.SendCommand("VSG_TRIG_SOUR", (char*)targetTrigSource.c_str(), true, g_checkLocal);		// VSG1;TRIG:SOUR [source]
	rxLen = iqx.SendCommand("VSG_TRIG_MODE", (char*)targetTrigMode.c_str(), true, g_checkLocal);			// VSG1;TRIG:MODE [mode]
	rxLen = iqx.SendCommand("VSG_TRIG_SLOPE", (char*)targetTrigSlope.c_str(), true, g_checkLocal);		// VSG1;TRIG:SLOP [slope]
	rxLen = iqx.SendCommand("VSG_TRIG_TYPE", (char*)targetTrigType.c_str(), true, g_checkLocal);			// VSG1;TRIG:TYPE [type]


	// TODO: should really do error checking?, even if instrument state did not change in any way? -- Jarir 12/15/11
	//if (status>0)
	//{
	rxLen = iqx.SendCommand("SYS_WAI", "", true);	// SYS;*WAI
	rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL?
	char *pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
		err = ERR_SET_RX_FAILED;
	//}

	return err;
}

int CIQmeasure_Scpi::SetVsg_GapPower(double rfFreqHz, double rfPowerLeveldBm, int port, int gapPowerOff)
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::SetVsgNxN(double rfFreqHz, double rfPowerLeveldBm[], int port[], double dFreqShiftHz)
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::SetVsgCw(double rfFreqHz, double offsetFrequencyMHz, double rfPowerLeveldBm, int port)
{
	// updated by Jarir 12/15/11, to use set/get/check instrument state local table before doing any actual tester setting with SCPI command
	//modified on 11/23/11 by Jacky
	int err     = 0;
	int rxLen   = 0;
	int status = 0;
	char *pch   = NULL;
	char scpiCommand[MAX_COMMAND_LEN];
	char tmpStr[MAX_COMMAND_LEN];

	//* check vsg port
	memset(tmpStr,'\0',MAX_COMMAND_LEN);
	sprintf_s(tmpStr,MAX_COMMAND_LEN,"RF%d%s",port,"A");
	if(g_strVsgPort.size()==0 ||strstr(g_strVsgPort.c_str(),tmpStr)==NULL)
	{
		memset(scpiCommand,'\0',MAX_COMMAND_LEN);
		// TODO: define module identifier // add strm1a/b
		sprintf_s(scpiCommand,MAX_COMMAND_LEN,"ROUT_PORT_RES_RF%d%s",port,"A");
		//status += setInstrumentState(scpiCommand, "VSG1", true);				  // ROUT;PORT:RES RF#A, VSG1
		rxLen = iqx.SendCommand(scpiCommand, "VSG1", true, g_checkLocal);				  // ROUT;PORT:RES RF#A, VSG1
		g_strVsgPort = tmpStr;				// Update global VSG port indicator
	}

	double targetFreq = 0.00;
	targetFreq = rfFreqHz + offsetFrequencyMHz*1e6;

	if (g_bDisableFreqFilter == true)
	{
		memset(scpiCommand,'\0',MAX_COMMAND_LEN);
		sprintf_s(scpiCommand,MAX_COMMAND_LEN,"%f",targetFreq);
		//status += setInstrumentState("VSG_FREQ", scpiCommand, true);	// VSG1;FREQ targetFreq -- set VSG1 center frequency
		rxLen = iqx.SendCommand("VSG_FREQ", scpiCommand, true, g_checkLocal);	// VSG1;FREQ targetFreq -- set VSG1 center frequency
		g_bDisableFreqFilter = false;
	}

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand,MAX_COMMAND_LEN,"%f",rfPowerLeveldBm);
	//status += setInstrumentState("VSG_POW_LEV", scpiCommand, true);		// VSG1;POW:LEV	 rfPowerLeveldBm -- set VSG1 output power level
	rxLen = iqx.SendCommand("VSG_POW_LEV", scpiCommand, true, g_checkLocal);		// VSG1;POW:LEV	 rfPowerLeveldBm -- set VSG1 output power level

	int phaseOffset = 0;
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f, %d", targetFreq, phaseOffset);
	// TODO: should this be saved to instrument state -- Jarir, 12/15/11
	rxLen = iqx.SendCommand("VSG_WAVE_GEN_CW", scpiCommand, true);		// VSG1;WAVE:GEN:CW targetFreq, phaseOffset -- configure CW wave

	//status += setInstrumentState("VSG_WAVE_EXEC", "ON", true);		// VSG1;WAVE:EXEC ON -- turn on waveform play
	rxLen = iqx.SendCommand("VSG_WAVE_EXEC", "ON", true, g_checkLocal);		// VSG1;WAVE:EXEC ON -- turn on waveform play

	// TODO: should really do error checking?, even if instrument state did not change in any way? -- Jarir 12/15/11
	//if (status>0)
	//{
	rxLen = iqx.SendCommand("SYS_WAI", "", true);	// SYS;*WAI
	rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL?
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
		err = ERR_SET_WAVE_FAILED;
	//}

	return err;
}

int CIQmeasure_Scpi::SetVsgModulation(char *modFileName, int loadInternalWaveform)
{
	// 06/28/12, Eric, TODO: add scpi command to load from internal wavefiles
	// 12/30/11, Jacky, repalce all setInstrumentState() with iqx.SendCommand()
	// Jarir removed old function body, 12/15/11, after verifying this works
	// updated by Jarir, 12/15/11, use of set/get/check instrument state local table; use of keywords
	// updated by Jarir, 12/9/11, totally changed the way VSG file is set, wavelist segments now being used, makes life a lot easier for SetFrameCnt
	// for old function, scroll below to commented section
	// updated by Jarir, 11/22/11
	// updated by Jacky, 11/22/11
	int   err = ERR_OK;
	//int status = 0;
	char scpiCommand[MAX_COMMAND_LEN] = {0};

	int rxLen = 0;
	char *pch = NULL;

	//status += setInstrumentState("VSG_WAVE_EXEC", "OFF", true);	// VSG;WAVE:EXEC OFF -- turn off waveform play
	rxLen = iqx.SendCommand("VSG_WAVE_EXEC", "OFF", true, g_checkLocal);	// VSG;WAVE:EXEC OFF -- turn off waveform play

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "\"");
	string modFileNameString = modFileName;

	string fileExt = ".mod";

	size_t found = modFileNameString.find(fileExt);
	if (found != string::npos)
	{
		modFileNameString.replace(modFileNameString.find(fileExt), fileExt.length(), ".iqvsg");
	}


	strcat(scpiCommand,modFileNameString.c_str());
	strcat(scpiCommand,"\"");
	//status += setInstrumentState("VSG_WAVE_LOAD", scpiCommand, true); // VSG;WAVE:LOAD "<filename.iqvsg>" -- load waveform on VSG1


	char * pcSCPIDataCommand = NULL;
	pcSCPIDataCommand =  new char [MAX_CMD_DATA_SIZE];
	if( strlen(modFileName) != 0 )
	{
		CreateMemInTester(modFileName, IQVSG, &pcSCPIDataCommand);
		int iCmdSize = iqx.m_iCmdDataSize+100;
		rxLen = iqx.SendCommand(pcSCPIDataCommand, "", false, false, iCmdSize);
	}
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "\"%s\"", "Signal.iqvsg");

	rxLen = iqx.SendCommand("VSG_WAVE_LOAD", scpiCommand, true, g_checkLocal); // VSG;WAVE:LOAD "<filename.iqvsg>" -- load waveform on VSG1

	// Jarir remove following condition, 01/11/12, WLIST works for Daytona starting 47336
	//if (true == bIQxstreamFound) //daytona does not support wave segment yet
	//{
	//// Jarir, 12/15/11: maintain two statuses: status will accumulate all setting change true flags; status2 will accumulate all wavelist setting change true flags
	//int status2 = 0;
	//int status1 = 0;
	//status1 = setInstrumentState("VSG_WLIS_WSEG1_DATA", scpiCommand, true);   // VSG1;WLIS:WSEG1:DATA "<filename.iqvsg>" -- put the waveform into wavelist as WSEG1
	//status += status1;
	//status2 += status1;
	//status1 = setInstrumentState("VSG_WLIS_WSEG1_NEXT", "0", true);	        // VSG1;WLIS:WSEG1:NEXT 0	-- configure WSEG1
	//status += status1;
	//status2 += status1;
	//status1 = setInstrumentState("VSG_WLIS_WSEG1_LENG", "0", true);	        // VSG1;WLIS:WSEG1:LENG 0	-- configure WSEG1
	//status += status1;
	//status2 += status1;
	//status1 = setInstrumentState("VSG_WLIS_WSEG1_START", "0", true);          // VSG1;WLIS:WSEG1:STAR 0	-- configure WSEG1
	//status += status1;
	//status2 += status1;

	// **replacing the above set ------------------------------------
	bool anyCmdSent = false;
	rxLen = iqx.SendCommand("VSG_WLIS_WSEG1_DATA", scpiCommand, true, g_checkLocal); // VSG1;WLIS:WSEG1:DATA "<filename.iqvsg>" -- put the waveform into wavelist as WSEG1
	anyCmdSent = anyCmdSent|| (!iqx.IsLastScpiSkipped());
	rxLen = iqx.SendCommand("VSG_WLIS_WSEG1_NEXT", "0", true, g_checkLocal);	    // VSG1;WLIS:WSEG1:NEXT 0	-- configure WSEG1
	anyCmdSent = anyCmdSent|| (!iqx.IsLastScpiSkipped());
	rxLen = iqx.SendCommand("VSG_WLIS_WSEG1_LENG", "0", true, g_checkLocal);	    // VSG1;WLIS:WSEG1:LENG 0	-- configure WSEG1
	anyCmdSent = anyCmdSent|| (!iqx.IsLastScpiSkipped());
	rxLen = iqx.SendCommand("VSG_WLIS_WSEG1_START", "0", true, g_checkLocal);       // VSG1;WLIS:WSEG1:STAR 0	-- configure WSEG1
	anyCmdSent = anyCmdSent|| (!iqx.IsLastScpiSkipped());
	//rxLen = iqx.SendCommand("VSG_WLIS_WSEG1_REPEAT", "0", true, g_checkLocal);
	//anyCmdSent = anyCmdSent|| (!iqx.IsLastScpiSkipped());

	// if any wavelist setting is changed, then save it for that wave segment, otherwise don't save -- Jarir 12/15/11
	if (anyCmdSent)
		rxLen = iqx.SendCommand("VSG_WLIS_WSEG1_SAVE", "", true);	// VSG1;WLIS:WSEG1:SAVE  -- save WSEG1

	//error checking is not done here now.
	//iqx.SendCommand("*wai;syst:err:all?\n");
	//pch = strtok(iqx.scpiRxBuffer, ",");
	// for debug purpose only, the waveform created by converting mod to iqvsg doesn't guarantee multiple of 8 samples.
	////if (atoi(pch)!=0 )
	//if (atoi(pch)!=0 && atoi(pch)!=-200)
	//   {
	//       err = ERR_SET_WAVE_FAILED;
	//   }
	//   else
	//   {
	//       // Mark as single MOD file mod
	//       g_vsgMode = ::VSG_SINGLE_MOD_FILE;
	//   }

	//status += setInstrumentState("VSG_WAVE_EXEC", "ON, WSEG1", true);  // VSG1;WAVE:EXEC ON, WSEG1 -- play WSEG1
	rxLen = iqx.SendCommand("VSG_WAVE_EXEC", "ON, WSEG1", true, g_checkLocal);  // VSG1;WAVE:EXEC ON, WSEG1 -- play WSEG1
	//}
	//else  // for non-iqxstream tester
	//{
	//	//status += setInstrumentState("VSG_WAVE_EXEC", "ON, WSEG1", true);  // VSG1;WAVE:EXEC ON
	//	rxLen = iqx.SendCommand("VSG_WAVE_EXEC", "ON", true, g_checkLocal);  // VSG1;WAVE:EXEC ON
	//}

	// TODO: do a loop until *OPC? returns 1 -- may be not needed, since *WAI is used afterwards?
	rxLen = iqx.SendCommand("SYS_OPC","?", true); // SYS;*OPC? -- Check operation complete status -- Jarir 12/15/11

	// TODO: should really do error checking?, even if instrument state did not change in any way? -- Jarir 12/15/11
	//if (status>0)
	//{
	rxLen = iqx.SendCommand("SYS_WAI", "", true);	// SYS;*WAI -- wait for operations to complete
	rxLen = iqx.SendCommand("SYS_ERR","?",true);    // SYST:ERR:ALL? -- get all errors
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		// TODO : remove when wave files converted from mod to iqvsg do not trigger tester to throw -200 error
		// Workaround: Jarir include this part because mod files converted to iqvsg throw an error on IQxstream saying length not multiple of 8, and pads 0's
		if (atoi(pch)== -200)
		{
			g_vsgMode = ::VSG_SINGLE_MOD_FILE;
		}
		else
		{
			err = ERR_SET_WAVE_FAILED;
		}
	}
	else
	{
		// Mark as single MOD file mod
		g_vsgMode = ::VSG_SINGLE_MOD_FILE;
	}
	//}

	SAFE_DELETE_ARRAY(pcSCPIDataCommand);
	return err;

}

int CIQmeasure_Scpi::LoadVsaSignalFile(char *sigFileName)
{
	int err = 0;
	int rxLen           = 0;
	char scpiCommand[MAX_COMMAND_LEN];

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);

	char * pcSCPIDataCommand = NULL;
	pcSCPIDataCommand =  new char [MAX_CMD_DATA_SIZE];
	if( strlen(sigFileName) != 0 )
	{
		CreateMemInTester(sigFileName, IQVSA, &pcSCPIDataCommand);
		int iCmdSize = iqx.m_iCmdDataSize+100;
		rxLen = iqx.SendCommand(pcSCPIDataCommand, "", false, false, iCmdSize);
	}

	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "CAPT:LOAD \"%s\"", "Capture.iqvsa");
	rxLen = iqx.SendCommand(scpiCommand);

	SAFE_DELETE_ARRAY(pcSCPIDataCommand);
	return err;
}

int CIQmeasure_Scpi::SetVsaBluetooth(double rfFreqHz, double rfAmplDb, int port, double triggerLevelDb, double triggerPreTime)
{
	// Updated by Jarir, 12/15/11, to use set/get/check instrument state local table; also use scpi keywords
	// Removed old function body, Jarir 12/15/11
	// Updated by Jarir 12/9/11, BT sampling rate value is set by defined value, not explicitly, VSA LO offset set to BT_SHIFT_HZ
	// Also setting a global variable here to indicate that BT technology is being used; needed for setting sampling rate in VsaDataCapture and for choosing right module for Power measurement
	// remove this global when possible, look at JIRA DAYTONA-25
	//updated by Jarir, 11/28/11, BT sampling rate is 37.5e6. rfAmplDb is rlev in dBm.
	// Note: triggerPreTime in Seconds
	int err = ERR_OK;
	int rxLen = 0;
	//int status = 0;
	char scpiCommand[MAX_COMMAND_LEN] = {0};

	// Jarir 12/15/11 -- currently using following global to identify what technology is being used
	// to identify the sampling rate for capture, power analysis etc.
	// TODO: remove when the above functions do not need to know technology
	g_technologyForPowerAndSampRate = TECHNOLOGY_BLUETOOTH;

	//* check vsa port
	char tmpStr[MAX_COMMAND_LEN] = {0};
	sprintf_s(tmpStr, MAX_COMMAND_LEN, "RF%d%s", port, "A");
	if (g_strVsaPort.size() == 0 || strstr(g_strVsaPort.c_str(), tmpStr) == NULL)
	{
		memset(scpiCommand, '\0', MAX_COMMAND_LEN);
		// TODO: define module identifier // add strm1a/b
		sprintf_s(scpiCommand,MAX_COMMAND_LEN,"ROUT_PORT_RES_RF%d%s",port,"A");
		//status += setInstrumentState(scpiCommand, "VSA1", true);	// ROUT;PORT:RES:RF#A, VSA1
		rxLen = iqx.SendCommand(scpiCommand, "VSA1", true, g_checkLocal);	// ROUT;PORT:RES:RF#A, VSA1
		g_strVsaPort = tmpStr;		// updated global variable with new RF port connected to VSA
	}

	//* check VSA frequency
	if (g_bDisableFreqFilter == true)
	{
		memset(scpiCommand, '\0', MAX_COMMAND_LEN);
		sprintf_s(scpiCommand,MAX_COMMAND_LEN,"%f",rfFreqHz);
		//status += setInstrumentState("VSA_FREQ", scpiCommand, true);	// VSA1;FREQ rfFreqHz -- set VSA center frequency
		rxLen = iqx.SendCommand("VSA_FREQ", scpiCommand, true, g_checkLocal);	// VSA1;FREQ rfFreqHz -- set VSA center frequenc

		if(true==bIQxstreamFound)
		{
			memset(scpiCommand, '\0', MAX_COMMAND_LEN);
			sprintf_s(scpiCommand,MAX_COMMAND_LEN,"%f",BT_SHIFT_HZ);
			//status += setInstrumentState("VSA_FREQ_LOOF", scpiCommand, true); // VSA1;FREQ:LOOF BT_SHIFT_HZ -- set VSA LO offset
			rxLen = iqx.SendCommand("VSA_FREQ_LOOF", scpiCommand, true, g_checkLocal); // VSA1;FREQ:LOOF BT_SHIFT_HZ -- set VSA LO offset
		}
	}

	rxLen = iqx.SendCommand("VSA_MEAS_COUP", "DC", true, g_checkLocal);	// Sets the DC coupling mode.


	double rfAmplDbBar = -5.00;
	if(bIQxelFound)
		rfAmplDbBar = -20.00;
	else
		rfAmplDbBar = -5.00;             // IQxstream supports a minimum -5 dBm ref level
	double rfAmplDbUsed = 0.00;
	rfAmplDbUsed = rfAmplDb<rfAmplDbBar? rfAmplDbBar:rfAmplDb;
	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", rfAmplDbUsed);
	//status += setInstrumentState("VSA_RLEV", scpiCommand, true);	// VSA;RLEV [-5/rfAmplDb] -- whichever is appropriate
	rxLen = iqx.SendCommand("VSA_RLEV", scpiCommand, true, g_checkLocal);	// VSA;RLEV [-5/rfAmplDb] -- whichever is appropriate


	//* set Bluetooth VSA sampling rate to 37.5MHz
	//char currentSampRate[MAX_COMMAND_LEN] = {'\0'};
	//getStatus = getInstrumentState("VSA_SRATE", currentSampRate, true);		// VSA1;SRATE?  -- get current VSA sampling rate
	//double currentSRate = atof(currentSampRate);
	//if ( currentSRate != BT_SAMPLING_RATE)
	//{
	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", DTNA_BT_SAMPLING_RATE); // VSA1;SRATE BT_SAMPLING_RATE -- set VSA sampling rate for BT

	//status += setInstrumentState("VSA_SRATE", scpiCommand, true);
	rxLen = iqx.SendCommand("VSA_SRATE", scpiCommand, true, g_checkLocal);
	//}

	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", triggerLevelDb);
	//status += setInstrumentState("VSA_TRIG_LEV", scpiCommand, true);	// VSA1;TRIG:LEV triggerLevelDb -- set VSA trigger level relative to ref level
	rxLen = iqx.SendCommand("VSA_TRIG_LEV", scpiCommand, true, g_checkLocal);	// VSA1;TRIG:LEV triggerLevelDb -- set VSA trigger level relative to ref level

	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", -1*triggerPreTime);	// -1 * -- because negative means PRE trigger, positive means POST trigger
	//status += setInstrumentState("VSA_TRIG_OFFS", scpiCommand, true);	// VSA1; TRIG:OFFS -triggerPreTime -- set trigger offset
	rxLen = iqx.SendCommand("VSA_TRIG_OFFS", scpiCommand, true, g_checkLocal);	// VSA1; TRIG:OFFS -triggerPreTime -- set trigger offset

	// TODO: should really do error checking?, even if instrument state did not change in any way? -- Jarir 12/15/11
	//if (status>0)
	//{
	rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
	rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
	char *pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
		err = ERR_SET_RX_FAILED;
	//}

	//TODO: rf used not used? //bool   RFused = true; baseband? -- iqmeasure for iqapi specific?
	//TODO: choose baseband port? -- can IQxstream do baseband?

	//TODO: load port compensation table? // 	double extAttenDb = 0;

	return err;
}

int CIQmeasure_Scpi::SetVsaTriggerTimeout(double triggerTimeoutSec)
{
	//JK, 01/09/12, removed set/get/check instrument state local table
	// updated by Jarir, 12/15/11, use of set/get/check instrument state local table; use of SCPI keywords
	//updated by Jarir, 11/23/11
	int err     = ERR_OK;
	int rxLen   = 0;
	//int status  = 0;
	char scpiCommand[MAX_COMMAND_LEN] = {0};

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", triggerTimeoutSec);
	//status += setInstrumentState("VSA_TRIG_TIME", scpiCommand, true);	// VSA1;TRIG:TIME triggerTimeoutSec -- set VSA trigger timeout
	rxLen = iqx.SendCommand("VSA_TRIG_TIME", scpiCommand, true, g_checkLocal);	// VSA1;TRIG:TIME triggerTimeoutSec -- set VSA trigger timeout

	if(!iqx.IsLastScpiSkipped())                            // check only when last scpi is really sent to the tester
	{
		rxLen = iqx.SendCommand("SYS_WAI", "", true);       // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	    // SYST:ERR:ALL? -- check all accumulated errors
	}

	char *pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT);                          //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);       // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	    // SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}
	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		err = ERR_SET_RX_FAILED;
	}

	return err;
}

int CIQmeasure_Scpi::SetVsa(double rfFreqHz, double rfAmplDb, int port, double extAttenDb, double triggerLevelDb, double triggerPreTimeSecs, double dFreqShiftHz)
{
	int err = ERR_OK;
	int rxLen = 0;
	//int status = 0;
	char scpiCommand[MAX_COMMAND_LEN]   = {0};
	char tmpStr[MAX_COMMAND_LEN]        = {0};

	string vsaStr = "";
	sprintf_s(tmpStr, MAX_COMMAND_LEN, "RF%d%s", port, "A");
	vsaStr = tmpStr;
	g_strVsaPort = vsaStr;		// update global variable indicating RF port number for VSA

	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "ROUT_PORT_RES_%s", vsaStr.c_str());
	//status += setInstrumentState(scpiCommand, "OFF", true);			// ROUT;PORT:RES RF#A, OFF -- turn off the port
	rxLen = iqx.SendCommand(scpiCommand, "OFF", true, g_checkLocal);			// ROUT;PORT:RES RF#A, OFF -- turn off the port

	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "ROUT_PORT_RES_%s", vsaStr.c_str());
	//status += setInstrumentState(scpiCommand, "VSA1", true);		// ROUT;PORT:RES RF#A, VSA1 -- turn on port as VSA
	rxLen = iqx.SendCommand(scpiCommand, "VSA1", true, g_checkLocal);		// ROUT;PORT:RES RF#A, VSA1 -- turn on port as VSA

	rxLen = iqx.SendCommand("VSA_MIDN", "?", true);		// VSA;MIDN? -- query VSA module identity
	iqx.LogPrintf(iqx.scpiRxBuffer);	// TODO: have some meaningful use of this information to report to log, and don't print to screen?

	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
	if (bIQxstreamFound)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", IQXS_MAX_SAMPLING_RATE);
	}else  //assume all future tester follow Daytona
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", DTNA_MAX_SAMPLING_RATE);
	}

	rxLen = iqx.SendCommand("VSA_SRATE", scpiCommand, true, g_checkLocal);	// VSA;SRATE MAX_SAMPLING_RATE -- set up VSA sampling rate to max

	//char temp[MAX_COMMAND_LEN] = {'\0'};
	//int getStatus = getInstrumentState("VSA_FREQ",temp, true);	// Get current VSA center frequency
	//double currentFreq = atof(temp);
	//// TODO: check VSA LOOF, skipped so far, set to 0?
	//if ((currentFreq != rfFreqHz) || g_bDisableFreqFilter == true)	// if current values different
	//{
	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", rfFreqHz);
	//status += setInstrumentState("VSA_FREQ", scpiCommand, true);	// VSA1;FREQ rfFreqHz
	rxLen = iqx.SendCommand("VSA_FREQ", scpiCommand, true, g_checkLocal);	// VSA1;FREQ rfFreqHz
	g_bDisableFreqFilter = false;


	double rfAmplDbBar = -5.00;
	if(bIQxelFound)
		rfAmplDbBar = -20.00;
	else
		rfAmplDbBar = -5.00;             // IQxstream supports a minimum -5 dBm ref level
	double rfAmplDbUsed = 0.00;
	rfAmplDbUsed = rfAmplDb<rfAmplDbBar? rfAmplDbBar:rfAmplDb;
	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", rfAmplDbUsed);
	//status += setInstrumentState("VSA_RLEV", scpiCommand, true);	// VSA;RLEV [-5/rfAmplDb] -- whichever is appropriate
	rxLen = iqx.SendCommand("VSA_RLEV", scpiCommand, true, g_checkLocal);	// VSA;RLEV [-5/rfAmplDb] -- whichever is appropriate


	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", triggerLevelDb);
	//status += setInstrumentState("VSA_TRIG_LEV", scpiCommand, true);	// VSA;TRIG:LEV triggerLevelDbm -- Set VSA trigger level in dB relative to ref level
	rxLen = iqx.SendCommand("VSA_TRIG_LEV", scpiCommand, true, g_checkLocal);	// VSA;TRIG:LEV triggerLevelDbm -- Set VSA trigger level in dB relative to ref level

	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", -1 * triggerPreTimeSecs);
	//status += setInstrumentState("VSA_TRIG_OFFS", scpiCommand, true);	// VSA;TRIG:OFFS -triggerPreTimeSecs -- for IQXS, negative means PRE trigger, positive means POST trigger
	rxLen = iqx.SendCommand("VSA_TRIG_OFFS", scpiCommand, true, g_checkLocal);	// VSA;TRIG:OFFS -triggerPreTimeSecs -- for IQXS, negative means PRE trigger, positive means POST trigger

	// TODO: should really do error checking?, even if instrument state did not change in any way? -- Jarir 12/15/11
	//if (status>0)
	//{
	// if instrument state changed, then check error
	rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
	rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
	char *pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if ( atoi(pch)!=0)
		err = ERR_SET_RX_FAILED;
	//}

	return err;
}

int CIQmeasure_Scpi::VsaDataCapture(double samplingTimeSecs, int triggerType, double sampleFreqHz, int htMode)
{
	// updated by Jarir, 12/15/11, use of set/get/check instrument state local table; use of SCPI keywords
	// updated by Jarir 12/9/11: totally changed previous trigger type mapping, using IQapi compatible trigger values to set trigger inside here with a switch statement;
	// two globals being used here: technology type for determining sampling rate (JIRA DAYTONA-25);
	//another global is set to indicate whether capture failed to avoid analysis on failure
	//remove globals when possible
	// IMPORTANT: trigger values in technology dlls must be consistent with iqapi trigger values
	//updated by Jarir 11/23/11
	int err             = 0;
	int rxLen           = 0;
	//int status = 0;
	char scpiCommand[MAX_COMMAND_LEN] = {0};
	//char tmpStr[MAX_COMMAND_LEN];
	char *pch = NULL;

	// Jarir talked to Z 12/16/11 to reset the following globals here during new capture taking place
	g_btAnalysisDataRate = -1;		// reset any analysis data rate that was configured before
	memset(analysisTypeBt, '\0', 50); // reset any analysis type that was requested before
	//g_bt_fetchHistory.clear();		// clear any previous fetch history
	iqx.ClearResultHistory();       // clear any previous fetch history


	// TODO: Jarir 12/15/11 -- need to know sampling rate, hence need to know technology for data capture
	// improve when this function does not need to know these
	if (g_technologyForPowerAndSampRate == TECHNOLOGY_BLUETOOTH)
	{
		if(bIQxstreamFound)
		{
			sampleFreqHz = IQXS_BT_SAMPLING_RATE;
		}
		else
		{
			sampleFreqHz = DTNA_BT_SAMPLING_RATE;
		}
	}
	else
	{
		if(bIQxstreamFound)
		{
			sampleFreqHz = IQXS_MAX_SAMPLING_RATE;
		}
		else //assume all future tester follow Daytona
		{
			sampleFreqHz = DTNA_MAX_SAMPLING_RATE;
		}

	}

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	//memset(tmpStr,'\0',MAX_COMMAND_LEN);

	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", sampleFreqHz);
	//status += setInstrumentState("VSA_SRATE", scpiCommand, true);	// VSA;SRATE sampleFreqHz -- set sampling frequency
	rxLen = iqx.SendCommand("VSA_SRATE", scpiCommand, true, g_checkLocal);	// VSA;SRATE sampleFreqHz -- set sampling frequency

	// following trigger types are default from IQAPI
	// below switch statement uses same values from IQAPI to update needed trigger settings
	// TODO: instead of using numbers, use IQAPI enum values
	string targetTrigType = "EDGE";
	string targetTrigMode = "SSH";
	string targetTrigSlope = "POS";
	string targetTrigSource = "VID";
	/*
	   IQV_TRIG_TYPE_AUTO       = -6,     //!< Indicates that a free run capture is done if IF Trigger times out

	   IQV_TRIG_TYPE_FREE_RUN   = 1,    //!< Indicates free-run capture mode
	   IQV_TRIG_TYPE_EXT        = 2,    //!< Indicates that the trigger type is external
	   IQV_TRIG_TYPE_EXT_1      = 2,    //!< Indicates External Trigger port 1 is selected at the positive edge (rising edge)
	   IQV_TRIG_TYPE_IF2_NO_CAL = 6,    //!< Indicates IF Trigger is selected (triggered by signal)
	   IQV_TRIG_TYPE_EXT_NEG    = 7,    //!< Negative
	   IQV_TRIG_TYPE_EXT_1_NEG  = 7,    //!< Indicates External Trigger is selected - negative (falling edge) - for port 1
	   IQV_TRIG_TYPE_EXT_2      = 8,    //!< Indicates External trigger is selected - positive (rising edge) - for port 2
	   IQV_TRIG_TYPE_EXT_2_NEG  = 9,    //!< Indicates External Trigger is selected - negative (falling edge) - for port 2

	   IQV_TRIG_TYPE_WB_PORT1_FOR_IQ2020 = 11,  //!< Wide band port 1 trigger type for IQ2020
	   IQV_TRIG_TYPE_WB_PORT2_FOR_IQ2020 = 12,  //!< Wide band port 2 trigger type for IQ2020

	   IQV_TRIG_TYPE_FOR_IQ2010            = 13,    //!< Trigger for IQ 2010 / Pebble Beach
	   IQV_TRIG_TYPE_FOR_IQ2010_DIG        = 13,    //!< Trigger for IQ 2010 / Digital trigger in FPGA
	   IQV_TRIG_TYPE_FOR_IQ2010_DIG_I      = 14,    //!< Trigger for IQ 2010 / Digital trigger in FPGA I only
	   IQV_TRIG_TYPE_FOR_IQ2010_DIG_Q      = 15     //!< Trigger for IQ 2010 / Digital trigger in FPGA Q only
	   */

	switch (triggerType)
	{
		case -1:
			break;
		case IQV_TRIG_TYPE_FREE_RUN:
			targetTrigType		= "EDGE";
			targetTrigMode		= "SSH";
			targetTrigSlope		= "POS";
			targetTrigSource	= "IMM";
			break;
		case IQV_TRIG_TYPE_EXT:
			targetTrigType		= "EDGE";
			targetTrigMode		= "SSH";
			targetTrigSlope		= "POS";
			targetTrigSource	= "EXT1";
			break;
		case IQV_TRIG_TYPE_IF2_NO_CAL:
			targetTrigType		= "EDGE";
			targetTrigMode		= "SSH";
			targetTrigSlope		= "POS";
			targetTrigSource	= "VID";
			break;
		case IQV_TRIG_TYPE_EXT_NEG:
			targetTrigType		= "EDGE";
			targetTrigMode		= "SSH";
			targetTrigSlope		= "NEG";
			targetTrigSource	= "VID";
			break;
		case IQV_TRIG_TYPE_EXT_2:
			targetTrigType		= "EDGE";
			targetTrigMode		= "SSH";
			targetTrigSlope		= "POS";
			targetTrigSource	= "EXT2";
			break;
		case IQV_TRIG_TYPE_EXT_2_NEG:
			targetTrigType		= "EDGE";
			targetTrigMode		= "SSH";
			targetTrigSlope		= "NEG";
			targetTrigSource	= "EXT2";
			break;
		case IQV_TRIG_TYPE_FOR_IQ2010:
			targetTrigType		= "EDGE";
			targetTrigMode		= "SSH";
			targetTrigSlope		= "POS";
			targetTrigSource	= "VID";
			break;
		default:
			break;
	}

	//status += setInstrumentState("VSA_TRIG_SOUR",(char*)targetTrigSource.c_str(),true);	// VSA;TRIG:SOUR [source]
	//status += setInstrumentState("VSA_TRIG_MODE",(char*)targetTrigMode.c_str(),true);	    // VSA;TRIG:MODE [mode]
	//status += setInstrumentState("VSA_TRIG_SLOPE",(char*)targetTrigSlope.c_str(),true);	// VSA;TRIG:SLOP [slope]
	//status += setInstrumentState("VSA_TRIG_TYPE",(char*)targetTrigType.c_str(),true);	    // VSA;TRIG:TYPE [type]

	rxLen = iqx.SendCommand("VSA_TRIG_SOUR",(char*)targetTrigSource.c_str(),true, g_checkLocal);	// VSA;TRIG:SOUR [source]
	rxLen = iqx.SendCommand("VSA_TRIG_MODE",(char*)targetTrigMode.c_str(),true, g_checkLocal);	    // VSA;TRIG:MODE [mode]
	rxLen = iqx.SendCommand("VSA_TRIG_SLOPE",(char*)targetTrigSlope.c_str(),true, g_checkLocal);	// VSA;TRIG:SLOP [slope]
	rxLen = iqx.SendCommand("VSA_TRIG_TYPE",(char*)targetTrigType.c_str(),true, g_checkLocal);	    // VSA;TRIG:TYPE [type]

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%f", samplingTimeSecs);
	//status += setInstrumentState("VSA_CAPT_TIME", scpiCommand, true);	// VSA;CAPT:TIME samplingTimeSecs -- set capture duration
	rxLen = iqx.SendCommand("VSA_CAPT_TIME", scpiCommand, true, g_checkLocal);	// VSA;CAPT:TIME samplingTimeSecs -- set capture duration

	if (htMode==2) //vht80mode
	{
		rxLen = iqx.SendCommand("VSA_INIT_SPEC_WIDE", "", true);		// VSA;INIT -- perform wide spectrum capture
	}
	else
	{
		rxLen = iqx.SendCommand("VSA_INIT", "", true);		// VSA;INIT -- perform capture
	}


	// TODO: loop until *OPC? returns 1
	rxLen = iqx.SendCommand("SYS_OPC", "?", true);		// SYS;*OPC? -- check for operation complete status

	// TODO: do we need to check status here to see if any setting changed and then check error only?
	rxLen = iqx.SendCommand("SYS_WAI", "", true);		// SYS;*WAI -- wait for previous operations
	rxLen = iqx.SendCommand("SYS_ERR","?",true);		// SYST:ERR:ALL? -- check error queue
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		g_lastCaptureFailed = -1;			// global variable updated indicating capture failed!!! to avoid any subsequent analysis
		err = ERR_CAPTURE_FAILED;
	}
	else
	{
		g_lastCaptureFailed = 0;			// global variable updated indicating capture was okay, allow analysis in this case
	}

	return err;
}



int CIQmeasure_Scpi::SetVsaNxN(double rfFreqHz, double rfAmplDb[], int port[], double extAttenDb, double triggerLevelDb, double triggerPreTime, double dFreqShiftHz)
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::Agc(double *rfAmplDb, bool allTesters)
{
	// JK, 01/09/12, removed set/get/check instrument status, need a clear definition of what should be done in Agc
	// updated by Jarir, 12/15/11, use of set/get/check instrument state local table; use of SCPI keywords
	// added by Jarir: 12/9/11, only for one tester, for now allTesters is not used
	int err     = ERR_OK ;
	int rxLen   = 0;

	int status = -1;
	//status = iqx.SendCommand("VSA_RLEV_AUTO", "", true);	// VSA;RLEV:AUTO
	rxLen = iqx.SendCommand("VSA_RLEV_AUTO", "", true, g_checkLocal);	// VSA;RLEV:AUTO

	//char temp[MAX_COMMAND_LEN] = {'\0'};
	//map <string, string>::iterator it;
	//it = g_instrumentState.find("VSA_RLEV");	// get current locally stored ref level

	//if (it!=g_instrumentState.end())
	//	g_instrumentState.erase(it);			// erase current locally stored ref level

	//int getStatus = getInstrumentState("VSA_RLEV", temp, true); // again get current ref level from tester, and update local instrument state table

	//double currentRefLev = atof(temp);
	//rfAmplDb[0] = currentRefLev;

	//// TODO: do we force a status check here or only do if anything changed?
	//if (status > 0)
	//{
	//	rxLen = iqx.SendCommand("SYS_WAI", "", true);		// SYS;*WAI -- wait for previous operations
	//	rxLen = iqx.SendCommand("SYS_ERR","?",true);		// SYST:ERR:ALL? -- check error queue
	//	char *pch = NULL;
	//	pch = strtok(iqx.scpiRxBuffer, ",");
	//	if (NULL == pch)
	//	{
	//		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
	//		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
	//		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
	//		pch = strtok(iqx.scpiRxBuffer, ",");
	//	}

	//	if (NULL == pch)
	//	{
	//		err = ERR_NO_RESPONSE_FROM_TESTER;
	//	}
	//	else if (atoi(pch)!=0)
	//		err = ERR_SET_RX_FAILED;
	//}

	return err;
}

int CIQmeasure_Scpi::SetFrameCnt(int frameCnt)
{
	// JK, 01/09/12, removed set/get/check instrument state local table;
	// updated by Jarir, 12/15/11, use of set/get/check instrument state local table; use of SCPI keywords
	// added by Jarir for setting frame count, 12/9/11, corresponding change made to SetVsg and SetVsgModulation, otherwise will not work, check those functions for consistency; sometimes plays one less than frames requested; JIRA STANDREWS-1653
	int     err             = ERR_OK;
	int     rxLen           = 0;
	char    scpiCommand[MAX_COMMAND_LEN] = {0};
	//int     status          = 0;
	//int     status1         = 0;
	//int     status2         = 0;
	char    *pch            = NULL;
	bool    stateChanged    = false;

	//status += setInstrumentState("VSG_WAVE_EXEC", "OFF", true);		// VSG;WAVE:EXEC OFF -- turn off waveform play
	rxLen = iqx.SendCommand("VSG_WAVE_EXEC", "OFF", true, g_checkLocal);		// VSG;WAVE:EXEC OFF -- turn off waveform play

	if (frameCnt !=0) // play given number of times
	{
		stateChanged = false;
		memset(scpiCommand,'\0',MAX_COMMAND_LEN);
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%d", frameCnt+1); // Workaround, Jarir 01/11/12, frameCnt+1 because Daytona plays one less than requested, TODO: Jarir found issue with IQxstream that it plays one less than request, 12/8/11
		//status1 = setInstrumentState("VSG_WLIS_COUNT", scpiCommand, true); // VSG;WLIS:COUNT frameCnt -- update count for wavelist to play
		rxLen = iqx.SendCommand("VSG_WLIS_COUNT", scpiCommand, true, g_checkLocal); // VSG;WLIS:COUNT frameCnt -- update count for wavelist to play
		//// Jarir, 12/15/11: maintain two statuses: status will accumulate all setting change true flags; status2 will accumulate all wavelist setting change true flags
		//status += status1;
		//status2+= (int)(rxLen>0);
		stateChanged = stateChanged &&(!(iqx.IsLastScpiSkipped()));
		//status1 = setInstrumentState("VSG_WLIS_COUNT_ENABLE", "WSEG1", true);	// VSG;WLIS:COUNT:ENABLE WSEG1 -- enable count for WSEG1
		rxLen = iqx.SendCommand("VSG_WLIS_COUNT_ENABLE", "WSEG1", true, g_checkLocal);	// VSG;WLIS:COUNT:ENABLE WSEG1 -- enable count for WSEG1
		//status += status1;
		//status2+= (int)(rxLen>0);
		stateChanged = stateChanged &&(!(iqx.IsLastScpiSkipped()));
		//if (status2>0)	// save if WSEG1 settings were changed
		if(stateChanged)
		{
			rxLen = iqx.SendCommand("VSG_WLIS_WSEG1_SAVE","",true);	            // VSG1;WLIS:WSEG1:SAVE -- save WSEG1
		}
	}
	else
	{
		stateChanged = false;
		//status1 = setInstrumentState("VSG_WLIS_COUNT", "0", true);		    // VSG1;WLIS:COUNT 0  -- set count to 0
		rxLen = iqx.SendCommand("VSG_WLIS_COUNT", "0", true, g_checkLocal);		// VSG1;WLIS:COUNT 0  -- set count to 0
		// Jarir, 12/15/11: maintain two statuses: status will accumulate all setting change true flags; status2 will accumulate all wavelist setting change true flags
		//status += status1;
		//status2+= (int)(rxLen>0);
		stateChanged = stateChanged &&(!(iqx.IsLastScpiSkipped()));

		rxLen = iqx.SendCommand("VSG_WLIS_COUNT_DISABLE", "WSEG1", true);	    // VSG1; WLIS:COUNT:DISABLE WSEG1 -- disable count on WSEG1
		//map <string, string>::iterator it;
		////it = g_instrumentState.find("VSG_WLIS_COUNT_ENABLE");	            // find current count number and erase it from locally stored instrument state
		//it = iqx.testerStateMap.find("VSG_WLIS_COUNT_ENABLE");	            // find current count number and erase it from locally stored instrument state
		//if (it != iqx.testerStateMap.end())
		//	iqx.testerStateMap.erase(it);
		iqx.RemoveTesterState("VSG_WLIS_COUNT_ENABLE");                         // remove current count number from the local tester state map

		//if (status2>0) // if anything changed for WSEG1
		if(stateChanged)
			rxLen = iqx.SendCommand("VSG_WLIS_WSEG1_SAVE","",true);	            // VSG;WLIS:WSEG1:SAVE -- save WSEG1
	}

	//status += setInstrumentState("VSG_WAVE_EXEC", "ON, WSEG1", true);	// VSG;WAVE:EXEC ON, WSEG1 -- turn on playing WSEG1 for given number of times
	rxLen = iqx.SendCommand("VSG_WAVE_EXEC", "ON, WSEG1", true, g_checkLocal);	// VSG;WAVE:EXEC ON, WSEG1 -- turn on playing WSEG1 for given number of times

	// TODO: loop until *OPC? returns 1
	rxLen = iqx.SendCommand("SYS_OPC","?",true);	// SYS;*OPC? -- check for previous operations complete, TODO: do we need this here, since *WAI may be following

	// TODO: do we force a status check here or only do if anything changed?
	//if (status>0)
	//{
	rxLen = iqx.SendCommand("SYS_WAI", "", true);		// SYS;*WAI -- wait for previous operations
	rxLen = iqx.SendCommand("SYS_ERR","?",true);		// SYST:ERR:ALL? -- check error queue
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		if (atoi(pch)== -200)
		{
			// TODO : remove when wave files converted from mod to iqvsg do not trigger tester to throw -200 error
			// Workaround: Jarir include this part because mod files converted to iqvsg throw an error on IQxstream saying length not multiple of 8, and pads 0's
			g_vsgMode = ::VSG_SINGLE_MOD_FILE;
		}
		else
		{
			err = ERR_SET_WAVE_FAILED;
		}
	}
	else
	{
		// Mark as single MOD file mod
		g_vsgMode = ::VSG_SINGLE_MOD_FILE;
	}
	//}

	return err;
}

int CIQmeasure_Scpi::TxDone(void)
{
	int err = 0;
	int rxLen = 0;

	rxLen = iqx.SendCommand("WAVE:RCO", "?", false);	// VSG;POW:STAT [ON/OFF]

	if (atof(iqx.scpiRxBuffer) == 0)
	{
		err = ERR_OK;
	}
	else
	{
		err = ERR_TX_NOT_DONE;
	}

	return err;
}

int CIQmeasure_Scpi::EnableVsgRF(int enabled)
{
	// 12/30/11, Jacky, repalce all setInstrumentState() with iqx.SendCommand()
	// updated by Jarir, 12/15/11, use of set/get/check instrument state local table; use of SCPI keywords
	int err             = ERR_OK;
	//int status = 0;
	const char *vsgOn   = "ON";
	const char *vsgOff  = "OFF";
	//IQV_RF_ENABLE_ENUM rfEnabled; // TODO: is this something related to IQxs? if not, remove
	//rfEnabled = (enabled==0?IQV_RF_DISABLED:IQV_RF_ENABLED);
	//char scpiCommand[MAX_COMMAND_LEN];
	int rxLen   = 0;
	char *pch   = NULL;

	if(enabled)
		pch = (char*)vsgOn;
	else
		pch = (char*)vsgOff;

	//status += setInstrumentState("VSG_POW_STATUS", pch, true);	// VSG;POW:STAT [ON/OFF]
	rxLen = iqx.SendCommand("VSG_POW_STATUS", pch, true,g_checkLocal);	// VSG;POW:STAT [ON/OFF]

	// TODO: do we force a status check here or only do if anything changed?
	//if (status > 0)
	if (!iqx.IsLastScpiSkipped()) // if previous scpi sent, then need to check status
	{
		rxLen = iqx.SendCommand("SYS_WAI", "", true);		// SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);		// SYST:ERR:ALL? -- check error queue
		char *pch = NULL;
		pch = strtok(iqx.scpiRxBuffer, ",");
		if (NULL == pch)
		{
			Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
			rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
			rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
			pch = strtok(iqx.scpiRxBuffer, ",");
		}

		if (NULL == pch)
		{
			err = ERR_NO_RESPONSE_FROM_TESTER;
		}
		else if (atoi(pch)!=0)
			err = ERR_SET_WAVE_FAILED;
	}

	return err;
}

int CIQmeasure_Scpi::EnableVsgRFNxN(int vsg1Enabled, int vsg2Enabled, int vsg3Enabled, int vsg4Enabled)
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::EnableSpecifiedVsgRF(int enabled, int vsgNumber)
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::EnableSpecifiedVsaRF(int enabled, int vsaNumber)
{
	int err = 0;

	return err;
}

int CIQmeasure_Scpi::GetVsaSettings(double *freqHz, double *ampl, IQAPI_PORT_ENUM *port, int *rfEnabled, double *triggerLevel)
{
	int err = 0;

	return err;
}

// Jarir add power analysis function, 12/9/11 need to know technology, remove technology-dependency with Engineering's feedback JIRA Daytona-25, currently need a global to indicate technology, remove global variable when possible
int CIQmeasure_Scpi::AnalyzePower(double T_interval, double max_pow_diff_dB)
{
	// JK, 01/09/12, removed set/get/check instrument state local table;
	// updated by Jarir, 12/15/11, use of set/get/check instrument state local table; use of SCPI keywords
	// TODO: Detect technology being used and use right sampling rate and technology for power analysis
	// Jarir Talked to Fei about Generic Power Analysis, 12/5/11
	// Fei's suggestion is to use Technology-specific power analysis for this part
	// Whether results will be same or not depends on default values being different from explicitly set values
	// Also power measurement including gaps is not supported by SCPI currently, and is not being currently used by IQfact+
	// Even though they are defined in IQmeasure
	/*	// Input parameters
		double	T_interval;	        //!< This field is used to specify the interval that is used to determine if power is present or not (sec). Default: 3.2e-6
		double	max_pow_diff_dB;    //!< This filed is used to specify the maximum power difference between packets that are expected to be detected.  Default: 15

	// IQmeasure is not currently using these, so don't use these for now

*/
	int     err                             = ERR_OK;
	//int     status                        = 0;
	int     rxLen                           = 0;
	char    *pch                            = NULL;
	char    scpiCommand[MAX_COMMAND_LEN]    = {0};
	char    scpiParam[MAX_COMMAND_LEN]      = {0};

	if (g_lastCaptureFailed != 0)
	{
		//g_bt_fetchHistory.clear();		// if capture failed, delete locally stored fetch history
		iqx.ClearResultHistory();
		return ERR_NO_CAPTURE_DATA;		// return error right away without any analysis
	}

	// this function needs to know technology module for power analysis
	// TODO: improve when this function doesn't need to know technology module
	if (g_technologyForPowerAndSampRate == TECHNOLOGY_BLUETOOTH)
	{
		g_moduleForPowerAnalysis = "BT";
	}
	else
	{
		g_moduleForPowerAnalysis = "WIFI";
	}

	g_lastPerformedAnalysisType = ANALYSIS_POWER; // set global variable indicating last analysis performed is power
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%s_CONF_PDET_GAP", g_moduleForPowerAnalysis); // choose the right module
	rxLen = iqx.SendCommand(scpiCommand,"?", true,g_checkLocal);

	//char temp[MAX_COMMAND_LEN] = {'\0'};
	//int getStatus = getInstrumentState(scpiCommand, temp, true);	// get current setting and store to change back after analysis
	double currentPacketDetectGapTime = NA_NUMBER;
	currentPacketDetectGapTime = atof(iqx.scpiRxBuffer);

	memset(scpiParam,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiParam, MAX_COMMAND_LEN, "%e", T_interval);
	//status += setInstrumentState(scpiCommand, scpiParam, true);	// [WIFI/BT];CONF:PDGT T_interval -- set power detect gap time
	rxLen = iqx.SendCommand(scpiCommand, scpiParam, true, g_checkLocal);	// [WIFI/BT];CONF:PDGT T_interval -- set power detect gap time

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	memset(scpiParam,'\0',MAX_COMMAND_LEN);
	//memset(temp, '\0', MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%s_CONF_PDET_THR", g_moduleForPowerAnalysis); // choose the right module and set packet detect power difference
	rxLen = iqx.SendCommand(scpiCommand,"?",true,g_checkLocal);
	//getStatus = getInstrumentState(scpiCommand, temp, true);	// get current setting and store to change back after analysis
	double currentPacketDetectPowerDiff = NA_NUMBER;
	currentPacketDetectPowerDiff = atof(iqx.scpiRxBuffer);
	sprintf_s(scpiParam, MAX_COMMAND_LEN, "%f", max_pow_diff_dB);
	//status += setInstrumentState(scpiCommand, scpiParam, true);// [WIFI/BT];CONF:PDGT max_pow_diff_dB -- set power detect gap time
	rxLen = iqx.SendCommand(scpiCommand, scpiParam, true, g_checkLocal);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN,  "%s_CLEAR_ALL", g_moduleForPowerAnalysis);	// [WIFI/BT];CLE:ALL -- clear all previous analysis results
	rxLen = iqx.SendCommand(scpiCommand, "", true);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN,  "%s_CALC_POW", g_moduleForPowerAnalysis);
	rxLen = iqx.SendCommand(scpiCommand, "0, 1", true);					// [WIFI/BT];CALC:POW 0, 1 -- perform power analysis for first packet only

	int fetchStatus         = -1;
	int parsedValues        = -1;
	int numPackets          = 0;	// number of all packets
	int numCompletePackets  = 0;	// number of complete
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN,  "%s_FETC_SYNC", g_moduleForPowerAnalysis);	// [WIFI/BT];FETC:SYNC? -- get how many complete and incomplete packets
	numPackets = (int) iqx.GetScalars(scpiCommand, 0, &fetchStatus, &parsedValues, 5, true);	// number of all packets

	numCompletePackets = (int) iqx.GetScalars(scpiCommand, 1, &fetchStatus, &parsedValues, 5, true);	// number of complete

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN,  "%s_CLEAR_ALL", g_moduleForPowerAnalysis);	// [WIFI/BT];CLE:ALL -- clear all analysis results again
	rxLen = iqx.SendCommand(scpiCommand, "", true);



	////no such thing for WiFi. Jarir need to explain why we did that, the current power analysis should be able to handle multiple packet
	//if(	g_moduleForPowerAnalysis == "BT")
	//{
	//	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	//	sprintf_s(scpiCommand, MAX_COMMAND_LEN,  "%s_CALC_ALL", g_moduleForPowerAnalysis);	 //no such thing for WiFi.
	//	memset(scpiParam,'\0',MAX_COMMAND_LEN);
	//	sprintf_s(scpiParam, MAX_COMMAND_LEN,  "0, %d", numPackets);
	//	rxLen = iqx.SendCommand(scpiCommand, scpiParam, true);	// [WIFI/BT];CALC:ALL 0, numPackets -- perform power analysis for all packets
	//}
	//else
	//{
	//	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	//	sprintf_s(scpiCommand, MAX_COMMAND_LEN,  "%s_CALC_POW", g_moduleForPowerAnalysis);	 //no such thing for WiFi.
	//	memset(scpiParam,'\0',MAX_COMMAND_LEN);
	//	sprintf_s(scpiParam, MAX_COMMAND_LEN,  "0, %d", numPackets);
	//	rxLen = iqx.SendCommand(scpiCommand, scpiParam, true);	// [WIFI/BT];CALC:ALL 0, numPackets -- perform power analysis for all packets
	//}

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN,  "%s_CALC_POW", g_moduleForPowerAnalysis);	 //no such thing for WiFi.
	memset(scpiParam,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiParam, MAX_COMMAND_LEN,  "0, %d", numPackets);
	rxLen = iqx.SendCommand(scpiCommand, scpiParam, true);	// [WIFI/BT];CALC:ALL 0, numPackets -- perform power analysis for all packets


	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	memset(scpiParam,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN,  "%s_CONF_PDET_THR", g_moduleForPowerAnalysis);
	sprintf_s(scpiParam, MAX_COMMAND_LEN,  "%f", currentPacketDetectPowerDiff);
	//status += setInstrumentState(scpiCommand, scpiParam, true);	// [WIFI/BT];CONF:PDPD currentPacketDetectPowerDiff -- change back to previous setting
	rxLen = iqx.SendCommand(scpiCommand, scpiParam, true, g_checkLocal);	// [WIFI/BT];CONF:PDPD currentPacketDetectPowerDiff -- change back to previous setting

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	memset(scpiParam,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN,  "%s_CONF_PDET_GAP", g_moduleForPowerAnalysis);
	sprintf_s(scpiParam, MAX_COMMAND_LEN,  "%e", currentPacketDetectGapTime);
	//status += setInstrumentState(scpiCommand, scpiParam, true);	// [WIFI/BT];CONF:PDGT currentPacketDetectGapTime -- change back to previous setting
	rxLen = iqx.SendCommand(scpiCommand, scpiParam, true, g_checkLocal);	// [WIFI/BT];CONF:PDGT currentPacketDetectGapTime -- change back to previous setting

	// Force an error check since CALC happened which is not instrument state
	//if (status > 0)
	//{
	rxLen = iqx.SendCommand("SYS_WAI", "", true);		// SYS;*WAI -- wait for previous operations
	rxLen = iqx.SendCommand("SYS_ERR","?",true);		// SYST:ERR:ALL? -- check error queue
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		//g_bt_fetchHistory.clear();		// clear fetch history, since this is new analysis
		iqx.ClearResultHistory();       // clear fetch history, since this is new analysis
		err = ERR_ANALYSIS_FAILED;
	}
	else
	{
		//g_bt_fetchHistory.clear();	// clear fetch history
		iqx.ClearResultHistory();
	}
	//}


	return err;

}



// 12/12/11, Jarir updated based on Response from Roman on 12/7/11
// Jarir added CW analysis 12/9/11udpate based on Roman's response 12/7/11
int CIQmeasure_Scpi::AnalyzeCW(double intervalLength, double measurementLength)
{
	// JK, 01/09/12, removed set/get/check instrument state local table;
	// updated by Jarir, 12/15/11, use of set/get/check instrument state local table; use of SCPI keywords

	int err             = ERR_OK;
	int rxLen           = 0;
	//int status          = 0;
	char *pch           = NULL;
	char scpiCommand[MAX_COMMAND_LEN] = {0};

	if (g_lastCaptureFailed != 0)		                                        // if capture failed, then return error and don't do analysis
		return ERR_NO_CAPTURE_DATA;

	g_lastPerformedAnalysisType = ANALYSIS_CW;                                  // set analysis type to analysis CW

	if(intervalLength == 0.0 )                                                  // (intervalLength >=measurementLength)
	{
		char captureTimeStr[MAX_COMMAND_LEN] = {0};
		//int getStatus = getInstrumentState("VSA_CAPT_TIME", captureTimeStr, true);		// get vsa capture time to perform analysis over whole capture : VSA1;CAPT:TIME?
		rxLen = iqx.SendCommand("VSA_CAPT_TIME", "?",true,g_checkLocal);        // get vsa capture time
		intervalLength = atof(iqx.scpiRxBuffer);                                // set interval length equal vsa capture time
		measurementLength = intervalLength;                                     // make measurement length equal interval lenght
	}

	rxLen = iqx.SendCommand("GPRF_CLEAR_ALL","",true);	                        // GPRF;CLE:ALL -- clear all previous GPRF analysis
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%e", intervalLength);	            //
	rxLen = iqx.SendCommand("GPRF_CONF_MLEN", scpiCommand, true, g_checkLocal);	// GPRF;CONF:MLEN captTime -- set measurement length to whole capture duration
	rxLen = iqx.SendCommand("GPRF_CONF_ILEN", scpiCommand, true, g_checkLocal); // GPRF;CONF:ILEN captTime -- set interval length to whole capture duration, so there is only 1 interval

	rxLen = iqx.SendCommand("GPRF_CALC_PHAS_INT", "0, 1", true, g_checkLocal);	// GPRF;CALC:PHAS:INT 0, 1 -- do a GPRF phase calculation to extract frequency error in specified interval

	// Force an error check because CALC happened, which is not instrument state
	//if (status > 0)
	//{
	rxLen = iqx.SendCommand("SYS_WAI", "", true);		// SYS;*WAI -- wait for previous operations
	rxLen = iqx.SendCommand("SYS_ERR","?",true);		// SYST:ERR:ALL? -- check error queue
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else  if (atoi(pch)!=0)
		err = ERR_ANALYSIS_FAILED;
	//}

	return err;

}

// TODO: Convert Analyze80211b  to optimized form -- 1. use SCPI keyword (insert into map in IQxstream_API.cpp as necessary; 2. use set/get/check instrument state functions
int CIQmeasure_Scpi::Analyze80211ag(int ph_corr_mode, int ch_estimate, int sym_tim_corr, int freq_sync, int ampl_track)
{
	int err             = 0;
	int rxLen           = 0;
	char scpiCommand[MAX_COMMAND_LEN];
	char * pch          = NULL;
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);

	g_lastPerformedAnalysisType = ANALYSIS_80211AG; // set global variable;

	rxLen = iqx.SendCommand("wifi");            // Set "wifi" mode
	rxLen = iqx.SendCommand("clear:all");       // Clear all previous measurement results if there

	rxLen = iqx.SendCommand("conf:stan?");      // What wireless standard will be analyzed ?
	pch = strtok(iqx.scpiRxBuffer, ",");

	if(NULL == strstr(toLowerCase(pch),"ofdm"))
	{

		rxLen = iqx.SendCommand("conf:stan OFDM"); // Set 11a/g/n
		rxLen = iqx.SendCommand("conf:stan?");  // check
		if(NULL == strstr(toLowerCase(pch),"ofdm"))
		{
			err = ERR_INVALID_ANALYSIS_TYPE;
			return err;
		}
	}

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if (IQV_PH_CORR_OFF == ph_corr_mode)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:phas OFF");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:phas ON"); //no moving average now
	}
	rxLen = iqx.SendCommand(scpiCommand);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if(IQV_CH_EST_RAW_FULL == ch_estimate)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:cest DATA");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:cest LTF"); //2nd Order Polyfit is not implemented
	}
	rxLen = iqx.SendCommand(scpiCommand);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if (IQV_SYM_TIM_OFF == sym_tim_corr)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:scl OFF");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:scl ON");
	}
	rxLen = iqx.SendCommand(scpiCommand);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	switch (freq_sync)
	{
		case 2: //IQV_FREQ_SYNC_LONG_TRAIN:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:freq LTF");
			break;
		case 3: //IQV_FREQ_SYNC_FULL_PACKET:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:freq DATA");
			break;
		case 4: // IQV_FREQ_SYNC_LTF_SIG: // this is not defined in iqapidefine.h
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:freq SIG");
			break;
		default:     //short training is not available yet.
			break;
	}

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if (IQV_AMPL_TRACK_ON == ampl_track)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:ampl ON");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:phas OFF");
	}
	rxLen = iqx.SendCommand(scpiCommand);

	// Calculations are done here
	int frameOffset = 0;        // how many frames will be skipped in the calculation from the beginning of current capture segment
	int frameCount  = 1;
	// - Calculate power on the current capture segment in the current active virtual channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:pow %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Calculates Power Ramp On and Off on the most recent capture segment in the current active instrument channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:ramp %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Calculates a spectrum measurement on the most recent capture segment in the current active instrument channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:spec %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Performs a TX quality measurement on the current capture segment in the current active virtual channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:txq %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);

	iqx.SendCommand("*wai;syst:err:all?");
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("*wai;syst:err:all?");
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		err = ERR_INVALID_ANALYSIS_TYPE;
	}

	return err;
}
// TODO: Convert Analyze80211n  to optimized form -- 1. use SCPI keyword (insert into map in IQxstream_API.cpp as necessary; 2. use set/get/check instrument state functions
int CIQmeasure_Scpi::Analyze80211ac(char *mode, int enablePhaseCorr, int enableSymTimingCorr, int enableAmplitudeTracking, int decodePSDU, int enableFullPacketChannelEst, int frequencyCorr, char *referenceFile, int packetFormat)
{
	int err             = 0;
	int rxLen           = 0;

	char scpiCommand[MAX_COMMAND_LEN];

	char * pch          = NULL;
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);

	g_lastPerformedAnalysisType = ANALYSIS_80211AC; // set global variable;

	rxLen = iqx.SendCommand("wifi");            // Set "wifi" mode
	rxLen = iqx.SendCommand("clear:all");       // Clear all previous measurement results if there

	rxLen = iqx.SendCommand("conf:stan?");      // What wireless standard will be analyzed ?
	pch = strtok(iqx.scpiRxBuffer, ",");
	//if(NULL == strstr(toLowerCase(pch),"ac"))
	if(NULL == strstr(toLowerCase(pch),"ofdm")) //Note: DSSS covers 802.11b, and OFDM covers 802.11a/g/n/ac.
	{
		rxLen = iqx.SendCommand("conf:stan OFDM"); // Set 11ac
		rxLen = iqx.SendCommand("conf:stan?");  // check
		pch = strtok(iqx.scpiRxBuffer, ",");
		if(NULL == strstr(toLowerCase(pch),"ofdm"))
		{
			err = ERR_INVALID_ANALYSIS_TYPE;
			return err;
		}
	}

	char * pcSCPIDataCommand = NULL;
	pcSCPIDataCommand =  new char [MAX_CMD_DATA_SIZE];
	// if( referenceFile != NULL )
	// {
	// 	CreateMemInTester(referenceFile, IQREF, &pcSCPIDataCommand);
	// 	int iCmdSize = iqx.m_iCmdDataSize+100;
	// 	rxLen = iqx.SendCommand(pcSCPIDataCommand, "", false, false, iCmdSize);

	// 	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "WIFI;REF:LOAD \"%s\"", "Reference.iqref");
	// 	rxLen = iqx.SendCommand(scpiCommand);
	// }

	if( strstr(mode, "nxn") != NULL )
	{
		rxLen = iqx.SendCommand("conf:ofdm:mimo nxn");
		rxLen = iqx.SendCommand("conf:ofdm:mimo?");  // check
		pch = strtok(iqx.scpiRxBuffer, ",");
		if(NULL == strstr(toLowerCase(pch),"nxn"))
		{
			err = ERR_INVALID_ANALYSIS_TYPE;
			return err;
		}
	}
	else if( strstr(mode, "composite") != NULL )
	{
		rxLen = iqx.SendCommand("conf:ofdm:mimo comp");
		rxLen = iqx.SendCommand("conf:ofdm:mimo?");  // check
		pch = strtok(iqx.scpiRxBuffer, ",");
		if(NULL == strstr(toLowerCase(pch),"comp"))
		{
			err = ERR_INVALID_ANALYSIS_TYPE;
			return err;
		}
	}


	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if (0 == enablePhaseCorr)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:phas OFF");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:phas ON"); //no moving average now
	}
	rxLen = iqx.SendCommand(scpiCommand);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if(1 == enableFullPacketChannelEst)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:cest DATA");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:cest LTF"); //2nd Order Polyfit is not implemented
	}
	rxLen = iqx.SendCommand(scpiCommand);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if (0 == enableSymTimingCorr)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:scl OFF");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:scl ON");
	}
	rxLen = iqx.SendCommand(scpiCommand);

	//default value is LTE in SCPI, same as GUI. discuss whether we need this in software with Ray and Kenual.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	//int freq_sync = IQV_FREQ_SYNC_LONG_TRAIN;
	switch (frequencyCorr)
	{
		//case IQV_FREQUENCY_CORR_STF
		//	break;
		case IQV_FREQUENCY_CORR_LTF:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:freq LTF");
			break;
		case IQV_FREQUENCY_CORR_DATA:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:freq DATA");
			break;
		case IQV_FREQUENCY_CORR_SIG:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:freq SIG");
			break;
		default:     //short training is not available yet.
			break;
	}

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if (1 == enableAmplitudeTracking)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:ampl ON");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:ampl OFF");
	}
	rxLen = iqx.SendCommand(scpiCommand);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	switch (packetFormat)
	{
		case 1:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:pfor MIX");
			break;
		case 2:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:pfor GRE");
			break;
		case 0:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:pfor AUTO");
			break;
		default:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:pfor AUTO"); //unknown packet format, use auto.
	}

	// Calculations are done here
	int frameOffset = 0;        // how many frames will be skipped in the calculation from the beginning of current capture segment
	int frameCount  = 1;
	// - Calculate power on the current capture segment in the current active virtual channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:pow %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Calculates Power Ramp On and Off on the most recent capture segment in the current active instrument channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:ramp %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Calculates a spectrum measurement on the most recent capture segment in the current active instrument channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:spec %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Performs a TX quality measurement on the current capture segment in the current active virtual channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:txq %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);

	iqx.SendCommand("*wai;syst:err:all?");
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");

	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("*wai;syst:err:all?");
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		err = ERR_INVALID_ANALYSIS_TYPE;
	}

	SAFE_DELETE_ARRAY(pcSCPIDataCommand);

	return err;
}

// TODO: Convert Analyze80211b  to optimized form -- 1. use SCPI keyword (insert into map in IQxstream_API.cpp as necessary; 2. use set/get/check instrument state functions
int CIQmeasure_Scpi::Analyze80211b(int eq_taps, int DCremove11b_flag, int method_11b)
{
	int err             = 0;
	int rxLen           = 0;
	char scpiCommand[MAX_COMMAND_LEN];
	char * pch          = NULL;
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);

	g_lastPerformedAnalysisType = ANALYSIS_80211B; // set globle variable;

	rxLen = iqx.SendCommand("wifi");            // Set "wifi" mode
	rxLen = iqx.SendCommand("clear:all");       // Clear all previous measurement results if there

	rxLen = iqx.SendCommand("conf:stan?");      // What wireless standard will be analyzed ?
	pch = strtok(iqx.scpiRxBuffer, ",");

	if(pch!=NULL && NULL == strstr(toLowerCase(pch),"dsss"))
	{
		rxLen = iqx.SendCommand("conf:stan DSSS"); // Set 11b
		rxLen = iqx.SendCommand("conf:stan?");  // check
		if(NULL == strstr(toLowerCase(pch),"dsss"))
		{
			err = ERR_INVALID_ANALYSIS_TYPE;
			return err;
		}
	}

	//configure 11b analysis parameter:
	//rxLen = iqx.SendCommand("conf:dsss:etap?");
	//pch = strtok(iqx.scpiRxBuffer, ",");
	//switch (eq_taps)
	//{
	//case IQV_EQ_OFF:
	//	if(NULL == strstr(pch,"OFF"))
	//	{
	//		memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	//		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:etap OFF");
	//		rxLen = iqx.SendCommand(scpiCommand);
	//	}
	//	break;
	//case IQV_EQ_5_TAPS:
	//	if(NULL == strstr(pch,"TAP5"))
	//	{
	//		memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	//		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:etap TAP5");
	//		rxLen = iqx.SendCommand(scpiCommand);
	//	}
	//	break;
	//case IQV_EQ_7_TAPS:
	//	if(NULL == strstr(pch,"TAP7"))
	//	{
	//		memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	//		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:etap TAP7");
	//		rxLen = iqx.SendCommand(scpiCommand);
	//	}
	//	break;
	//case IQV_EQ_9_TAPS:
	//	if(NULL == strstr(pch,"TAP9"))
	//	{
	//		memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	//   		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:etap TAP9");
	//		rxLen = iqx.SendCommand(scpiCommand);
	//	}
	//	break;

	//default:
	//	break;
	//}

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	switch (eq_taps)
	{
		case IQV_EQ_OFF:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:etap OFF");
			break;
		case IQV_EQ_5_TAPS:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:etap TAP5");
			break;
		case IQV_EQ_7_TAPS:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:etap TAP7");
			break;
		case IQV_EQ_9_TAPS:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:etap TAP9");
			break;
		default:
			break;
	}
	rxLen = iqx.SendCommand(scpiCommand);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if (IQV_DC_REMOVAL_ON == DCremove11b_flag)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:evm:dcr ON");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:evm:dcr OFF"); //default
	}
	rxLen = iqx.SendCommand(scpiCommand);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if (IQV_11B_RMS_ERROR_VECTOR == method_11b)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:evm:meth RMS");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:dsss:evm:meth STANDARD"); //default
	}
	rxLen = iqx.SendCommand(scpiCommand);

	// Calculations are done here
	int frameOffset = 0;        // how many frames will be skipped in the calcuation from the begining of current capture segment
	int frameCount  = 1;
	// - Calculate power on the current capture segment in the current active virtual channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:pow %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Calculates Power Ramp On and Off on the most recent capture segment in the current active instrument channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:ramp %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Calculates Side lobe analysis for 802.11b on the most recent capture segment in the current active instrument channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:slob %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Calculates a spectrum measurement on the most recent capture segment in the current active instrument channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:spec %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Performs a TX quality measurement on the current capture segment in the current active virtual channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:txq %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);

	iqx.SendCommand("*wai;syst:err:all?");
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("*wai;syst:err:all?");
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		err = ERR_INVALID_ANALYSIS_TYPE;
	}

	return err;
}

// TODO: Convert Analyze80211n  to optimized form -- 1. use SCPI keyword (insert into map in IQxstream_API.cpp as necessary; 2. use set/get/check instrument state functions
int CIQmeasure_Scpi::Analyze80211n(char *type,
		char *mode,
		int enablePhaseCorr,
		int enableSymTimingCorr,
		int enableAmplitudeTracking,
		int decodePSDU,
		int enableFullPacketChannelEst,
		char *referenceFile,
		int packetFormat,
		int frequencyCorr)
{
	int err             = 0;
	int rxLen           = 0;
	char scpiCommand[MAX_COMMAND_LEN];
	char * pch          = NULL;
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);

	g_lastPerformedAnalysisType = ANALYSIS_MIMO; // set global variable;

	rxLen = iqx.SendCommand("wifi");            // Set "wifi" mode
	rxLen = iqx.SendCommand("clear:all");       // Clear all previous measurement results if there

	rxLen = iqx.SendCommand("conf:stan?");      // What wireless standard will be analyzed ?
	pch = strtok(iqx.scpiRxBuffer, ",");

	if(NULL == strstr(toLowerCase(pch),"ofdm"))
	{
		rxLen = iqx.SendCommand("conf:stan OFDM"); // Set 11a/g/n
		rxLen = iqx.SendCommand("conf:stan?");  // check
		pch = strtok(iqx.scpiRxBuffer, ",");
		if(NULL == strstr(toLowerCase(pch),"ofdm"))
		{
			err = ERR_INVALID_ANALYSIS_TYPE;
			return err;
		}
	}

	//type might not needed any more
	//mode should be needed.
	if( strstr(mode, "nxn") != NULL )
	{
		rxLen = iqx.SendCommand("conf:ofdm:mimo nxn");
		rxLen = iqx.SendCommand("conf:ofdm:mimo?");  // check
		pch = strtok(iqx.scpiRxBuffer, ",");
		if(NULL == strstr(toLowerCase(pch),"nxn"))
		{
			err = ERR_INVALID_ANALYSIS_TYPE;
			return err;
		}
	}
	else if( strstr(mode, "composite") != NULL )
	{
		rxLen = iqx.SendCommand("conf:ofdm:mimo comp");
		rxLen = iqx.SendCommand("conf:ofdm:mimo?");  // check
		pch = strtok(iqx.scpiRxBuffer, ",");
		if(NULL == strstr(toLowerCase(pch),"comp"))
		{
			err = ERR_INVALID_ANALYSIS_TYPE;
			return err;
		}
	}

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if (IQV_OFF == enablePhaseCorr)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:phas OFF");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:phas ON"); //no moving average now
	}
	rxLen = iqx.SendCommand(scpiCommand);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if(IQV_ON == enableFullPacketChannelEst)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:cest DATA");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:cest LTF"); //2nd Order Polyfit is not implemented
	}
	rxLen = iqx.SendCommand(scpiCommand);

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if (IQV_OFF == enableSymTimingCorr)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:scl OFF");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:scl ON");
	}
	rxLen = iqx.SendCommand(scpiCommand);

	//default value is LTE in SCPI, same as GUI. discuss whether we need this in software with Ray and Kenual.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	//int freq_sync = IQV_FREQ_SYNC_LONG_TRAIN;
	switch (frequencyCorr)
	{
		case 2:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:freq LTF");
			break;
		case 3:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:freq DATA");
			break;
		case 4:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:freq SIG");
			break;
		default:     //short training is not available yet.
			break;
	}

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	if (1 == enableAmplitudeTracking)
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:ampl ON");
	}
	else
	{
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:trac:phas OFF");
	}
	rxLen = iqx.SendCommand(scpiCommand);

	// *referenceFile in the future.

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	switch (packetFormat)
	{
		case 1:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:pfor MIX");
			break;
		case 2:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:pfor GRE");
			break;
		case 0:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:pfor AUTO");
			break;
		default:
			sprintf_s(scpiCommand, MAX_COMMAND_LEN, "conf:ofdm:pfor AUTO"); //unknown packet format, use auto.
	}

	// Calculations are done here
	int frameOffset = 0;        // how many frames will be skipped in the calculation from the beginning of current capture segment
	int frameCount  = 1;
	// - Calculate power on the current capture segment in the current active virtual channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:pow %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Calculates Power Ramp On and Off on the most recent capture segment in the current active instrument channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:ramp %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Calculates a spectrum measurement on the most recent capture segment in the current active instrument channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:spec %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);
	// - Performs a TX quality measurement on the current capture segment in the current active virtual channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:txq %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);

	iqx.SendCommand("*wai;syst:err:all?");
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");

	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("*wai;syst:err:all?");
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		err = ERR_INVALID_ANALYSIS_TYPE;
	}

	return err;
}

int CIQmeasure_Scpi::AnalyzeBluetooth(double data_rate, char *analysis_type)
{
	// updated by Jarir, 12/15/11, use of set/get/check instrument state local table; use of SCPI keywords
	// function added, 11/28/11, Jarir // Jarir updated 12/9/11
	int err             = ERR_OK;
	int rxLen           = 0;
	int status = 0;
	char scpiCommand[MAX_COMMAND_LEN];
	char *pch = NULL;


	if (g_lastCaptureFailed != 0)		// if last capture failed
	{
		g_btAnalysisDataRate = -1;		// reset any analysis data rate that was configured before
		memset(analysisTypeBt, '\0', 50); // reset any analysis type that was requested before
		//g_bt_fetchHistory.clear();		// clear any previous fetch history
		iqx.ClearResultHistory();   // clear any previous fetch history
		return ERR_NO_CAPTURE_DATA;		// return error
	}

	g_lastPerformedAnalysisType = ANALYSIS_BLUETOOTH; // set global variable indicator for current analysis, for future fetch commands

	rxLen = iqx.SendCommand("BT_MRST","",true);         // Resets the current active module to factory defaults.
	rxLen = iqx.SendCommand("BT_CLEAR_ALL","",true);	// BT;CLE:ALL -- clear all previous analysis


	if (data_rate <=4)			// if requested analysis data rate valid
	{
		if (data_rate == 0)		// requested analysis data rate is auto
		{
			//status += setInstrumentState("BT_CONF_DRAT","AUTO",true);	            // BT;CONF:DRAT AUTO -- auto-detect data rate
			rxLen = iqx.SendCommand("BT_CONF_DRAT", "AUTO", true, g_checkLocal);    // BT;CONF:DRAT AUTO -- auto-detect data rate

		}
		else if (data_rate == 4)
		{
			//status += setInstrumentState("BT_CONF_DRAT","LEN",true);	            // BT;CONF:DRAT LEN -- use low energy analysis
			rxLen = iqx.SendCommand("BT_CONF_DRAT", "LEN", true, g_checkLocal);	    // BT;CONF:DRAT LEN -- use low energy analysis
		}
		else
		{
			memset(scpiCommand,'\0',MAX_COMMAND_LEN);
			sprintf_s(scpiCommand, MAX_COMMAND_LEN,  "%dMBPS", (int) data_rate);		// BT;CONF:DRAT <data_rate>MBPS -- use BDR/EDR
			//status += setInstrumentState("BT_CONF_DRAT",scpiCommand, true);
			rxLen = iqx.SendCommand("BT_CONF_DRAT", scpiCommand, true, g_checkLocal);
		}
	}
	else
	{
		//status += setInstrumentState("BT_CONF_DRAT","AUTO",true);	// choose if data_rate is anything else; TODO: throw error if invalid data_rate is requested
		rxLen = iqx.SendCommand("BT_CONF_DRAT","AUTO",true);	// choose if data_rate is anything else; TODO: throw error if invalid data_rate is requested
	}

	int numPackets = 1; // just one packet for now // don't change this!!!! If you change it, make sure GetScalarMeasurement_Bluetooth will work
	// TODO: convert effectively to analyze all complete packets instead of just first one

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN,  "0, %d", numPackets);

	if (!strcmp(analysis_type, "PowerOnly"))
	{
		rxLen = iqx.SendCommand("BT_CALC_POW",scpiCommand, true);	// BT;CALC:POW 0, numPackets --- analyze power only
	}
	else if (!strcmp(analysis_type, "20dbBandwidthOnly"))
	{
		rxLen = iqx.SendCommand("BT_CALC_POW",scpiCommand, true);	// BT;CALC:POW 0, numPackets --- analyze power
		rxLen = iqx.SendCommand("BT_CALC_SPEC",scpiCommand, true);	// BT;CALC:SPEC 0, numPackets --- analyze spectrum
	}
	else if (!strcmp(analysis_type, "PowerAndFreq"))
	{
		rxLen = iqx.SendCommand("BT_CALC_POW",scpiCommand, true);	// BT;CALC:POW 0, numPackets --- analyze power
		rxLen = iqx.SendCommand("BT_CALC_TXQ",scpiCommand, true);	// BT;CALC:TXQ 0, numPackets --- analyze Tx quality
	}
	else if (!strcmp(analysis_type, "All"))
	{
		rxLen = iqx.SendCommand("BT_CALC_POW",scpiCommand, true);	// BT;CALC:POW 0, numPackets --- analyze power
		rxLen = iqx.SendCommand("BT_CALC_TXQ",scpiCommand, true);	// BT;CALC:TXQ 0, numPackets --- analyze Tx quality
		rxLen = iqx.SendCommand("BT_CALC_SPEC",scpiCommand, true);	// BT;CALC:SPEC 0, numPackets --- analyze spectrum
	}
	else if (!strcmp(analysis_type, "ACP"))
	{
		rxLen = iqx.SendCommand("BT_CALC_POW",scpiCommand, true);	// BT;CALC:POW 0, numPackets --- analyze power
		rxLen = iqx.SendCommand("BT_CALC_ACP",scpiCommand, true);	// BT;CALC:ACP 0, numPackets --- analyze ACP

	}
	else if (!strcmp(analysis_type, "AllPlus"))
	{
		rxLen = iqx.SendCommand("BT_CALC_ALL",scpiCommand, true);	// BT;CALC:ALL 0, numPackets --- analyze everything
	}
	else
	{
		// do nothing, invalid measurement type; TODO throw error?
	}

	// Force an error check since CALC happened, which is not instrument state
	// TODO: reduce following Blocks 1, 2, 3
	//if (status > 0)
	//{
	rxLen = iqx.SendCommand("SYS_WAI", "", true);		// SYS;*WAI -- wait for previous operations
	rxLen = iqx.SendCommand("SYS_ERR","?",true);		// SYST:ERR:ALL? -- check error queue
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");

	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
		rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		g_btAnalysisDataRate = -1;				// Block 1: if analysis failed, reset all associated globals again and return error, TODO: check if needed at all
		memset(analysisTypeBt, '\0', 50);
		//g_bt_fetchHistory.clear();
		iqx.ClearResultHistory();
		err = ERR_ANALYSIS_FAILED;
	}
	else
	{
		g_btAnalysisDataRate = data_rate;			// Block 2: if analysis okay, reset all associated globals, TODO: check if needed at all
		memset(analysisTypeBt, '\0', 50);
		strcpy_s(analysisTypeBt, 50, analysis_type);
		//g_bt_fetchHistory.clear();
		iqx.ClearResultHistory();
	}

	//}
	//else
	//{
	//	g_btAnalysisDataRate = data_rate;		// Block 3: if no setting change, reset all associated globals, TODO: check if needed at all
	//	memset(analysisTypeBt, '\0', 50);
	//	strcpy_s(analysisTypeBt, 50, analysis_type);
	//	g_bt_fetchHistory.clear();
	//}

	return err;

}
int CIQmeasure_Scpi::AnalyzeVHT80Mask()
{
	int err             = 0;
	int rxLen           = 0;
	char scpiCommand[MAX_COMMAND_LEN];
	char * pch          = NULL;

	g_lastPerformedAnalysisType = ANALYSIS_VHT80; // set global variable;

	rxLen = iqx.SendCommand("wifi");            // Set "wifi" mode
	//? shall we clear all here?
	//rxLen = iqx.SendCommand("clear:all");       // Clear all previous measurement results if there
	rxLen = iqx.SendCommand("cle:spec");

	// Calculations are done here
	int frameOffset = 0;        // how many frames will be skipped in the calculation from the beginning of current capture segment
	int frameCount  = 1;
	//NFFT, Res_BW and window_type is not available.

	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	// sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:spec:wide %d,%d", frameOffset, frameCount);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:spec %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);

	iqx.SendCommand("*wai;syst:err:all?");
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("*wai;syst:err:all?");
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		err = ERR_INVALID_ANALYSIS_TYPE;
	}

	return err;

}

// Workaround, Jarir 1/13/12, add following AnalyzeHT20Mask() and AnalyzeHT40Mask() functions to differentiate mask
// calculation between HT20 and HT40 signal, only a global is set and then
// AnalyzeFFT() function is called. HT20 spectrum needs truncation.

int CIQmeasure_Scpi::AnalyzeHT20Mask(double NFFT, double res_bw, char *window_type)
{
	int err = 0;
	g_lastPerformedAnalysisType = ANALYSIS_FFT; // set global variable;
	err = AnalyzeFFT(NFFT, res_bw, window_type);
	return err;
}
int CIQmeasure_Scpi::AnalyzeHT40Mask(double NFFT, double res_bw, char *window_type)
{
	int err = 0;
	g_lastPerformedAnalysisType = ANALYSIS_HT40; // set global variable;
	err = AnalyzeFFT(NFFT, res_bw, window_type);
	return err;
}
//

int CIQmeasure_Scpi::AnalyzeFFT(double NFFT, double res_bw, char *window_type)
{
	int err             = 0;
	int rxLen           = 0;
	char scpiCommand[MAX_COMMAND_LEN];
	char * pch          = NULL;

	// Workaround, Jarir, 1/13/12, remove setting global here, this is set in
	// AnalyzeHT20 and AnalyzeHT40 functions to differentiate
	// Between HT20 and HT40 for workaround. HT20 spectrum needs truncation.
	//g_lastPerformedAnalysisType = ANALYSIS_FFT; // set global variable;

	//shall we use WiFI module or GPRF module?
	rxLen = iqx.SendCommand("wifi");            // Set "wifi" mode
	//? shall we clear all here?
	//rxLen = iqx.SendCommand("clear:all");       // Clear all previous measurement results if there
	rxLen = iqx.SendCommand("cle:spec");

	// Calculations are done here
	int frameOffset = 0;        // how many frames will be skipped in the calculation from the beginning of current capture segment
	int frameCount  = 1;
	//NFFT, Res_BW and window_type is not available.

	// - Calculates a spectrum measurement on the most recent capture segment in the current active instrument channel.
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "calc:spec %d,%d", frameOffset, frameCount);
	rxLen = iqx.SendCommand(scpiCommand);

	iqx.SendCommand("*wai;syst:err:all?");
	pch = NULL;
	pch = strtok(iqx.scpiRxBuffer, ",");
	if (NULL == pch)
	{
		Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
		rxLen = iqx.SendCommand("*wai;syst:err:all?");
		pch = strtok(iqx.scpiRxBuffer, ",");
	}

	if (NULL == pch)
	{
		err = ERR_NO_RESPONSE_FROM_TESTER;
	}
	else if (atoi(pch)!=0)
	{
		err = ERR_INVALID_ANALYSIS_TYPE;
	}

	return err;
}

double CIQmeasure_Scpi::GetScalarMeasurement(char *measurement, int index)
{
	double doubleValue = 0.00;

	switch(g_lastPerformedAnalysisType) // based on
	{
		case ANALYSIS_80211AC:
			doubleValue = GetScalarMeasurement_80211ac(measurement,index);
			break;
		case ANALYSIS_80211AG:
			doubleValue = GetScalarMeasurement_80211ag(measurement,index);
			break;
		case ANALYSIS_80211B:
			doubleValue = GetScalarMeasurement_80211b(measurement,index);
			break;
		case ANALYSIS_MIMO:
			doubleValue = GetScalarMeasurement_80211n(measurement,index);
			break;
		case ANALYSIS_BLUETOOTH: // Jarir add BT scalar measurement function, 12/9/11
			doubleValue = GetScalarMeasurement_Bluetooth(measurement,index);
			break;
		case ANALYSIS_CW:  // Jarir add CW scalar measurement function, 12/9/11
			doubleValue = GetScalarMeasurement_CW(measurement, index);
			break;
		case ANALYSIS_POWER:  // Jarir add Power scalar measurement function, 12/9/11
			doubleValue = GetScalarMeasurement_Power(measurement, index);
			break;

		default : break;
	}

	return doubleValue;
}
int CIQmeasure_Scpi::GetVectorMeasurement(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	int err = 0;
	switch(g_lastPerformedAnalysisType) // based on
	{
		case ANALYSIS_80211AC:
			err = GetVectorMeasurement_80211ac(measurement, bufferReal, bufferImag, bufferLength);
			break;
		case ANALYSIS_80211AG:
			err = GetVectorMeasurement_80211ag(measurement, bufferReal, bufferImag, bufferLength);
			break;
		case ANALYSIS_80211B:
			err = GetVectorMeasurement_80211b(measurement, bufferReal, bufferImag, bufferLength);
			break;
		case ANALYSIS_MIMO:
			err = GetVectorMeasurement_80211n(measurement, bufferReal, bufferImag, bufferLength);
			break;
		case ANALYSIS_BLUETOOTH: // Jarir add BT vector measurement function, 12/9/11
			err = GetVectorMeasurement_Bluetooth(measurement, bufferReal, bufferImag, bufferLength);
			break;
		case ANALYSIS_FFT:
			err = GetVectorMeasurement_FFT(measurement, bufferReal, bufferImag, bufferLength);
			break;
		case ANALYSIS_HT40:
			err = GetVectorMeasurement_FFT(measurement, bufferReal, bufferImag, bufferLength);
			break;
		case ANALYSIS_VHT80:
			err = GetVectorMeasurement_FFT(measurement, bufferReal, bufferImag, bufferLength);
			break;

		default : break;
	}

	return err;
}

// Jarir Add following function to get string measurement, 12/9/11

int CIQmeasure_Scpi::GetStringMeasurement(char *measurement, char bufferChar[], int bufferLength)
{
	int err = 0;
	switch(g_lastPerformedAnalysisType) // based on
	{
		case ANALYSIS_BLUETOOTH:
			err = GetStringMeasurement_Bluetooth(measurement, bufferChar, bufferLength);
			break;

		default : break;
	}

	return err;
}

bool CIQmeasure_Scpi::GetVersion(char *buffer, int buf_size)
{
	bool    err                         = true;

	string  strCompany                  = "";  // eg: "LitePoint"
	string  strTesterType               = "";  // eg: "IQXEL 80"
	string  strTesterSN                 = "";  // eg: "DTNA-00009"
	string  strTesterFirmwareRevision   = "";  // eg: "0.9.2.48145"

	strTesterType   = iqx.GetTesterType();
	strTesterSN     = iqx.GetTesterSerialNumber();

	sprintf_s(buffer,MAX_BUFFER_LEN,"Tester 1 hardware version: %s\nTester 1 SN: %s",strTesterType.c_str(),strTesterSN.c_str());
	buf_size = (int)strlen(buffer);

	return err;
}

double CIQmeasure_Scpi::GetScalarMeasurement_80211ag(char *measurement, int index)
{
	double doubleValue  = NA_NUMBER;
	int  intValue = (int)NA_NUMBER;
	char *pch   = NULL;
	int key     = -1;
	int num     = 0;
	int intArray[MAX_BUFFER_LEN] = {0};
	double doubleArray[MAX_BUFFER_LEN] = {0.00};

	//not implemented yet.
	if (strstr(measurement, "psduCrcFail")!=0) //int
	{
		iqx.SendCommand("fetch:ofdm:psdu:crc?");
		num = 0;
		num =splitAsIntegers(iqx.scpiRxBuffer, intArray);
		if(intArray[0]!= 0)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = intArray[1];
		return doubleValue;
	}

	if (!strcmp(measurement, "plcpCrcPass"))
	{
		return doubleValue;
	}


	if (!strcmp(measurement, "dataRate"))
	{
		iqx.SendCommand("fetch:txq:ofdm:info:drat?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}

	if (strstr(measurement, "numSymbols")!=0)
	{
		iqx.SendCommand("fetch:txq:ofdm:info:nsym?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}

	if (!strcmp(measurement, "numPsduBytes"))
	{
		iqx.SendCommand("fetch:txq:ofdm:psdu:nbyt?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		//doubleValue = ceil(doubleArray[1]); //just in case
		doubleValue = doubleArray[1];
		return doubleValue;
	}

	if (!strcmp(measurement, "evmData"))
	{
		return doubleValue;
	}
	//{
	//	if (resultOfdm->evmData && resultOfdm->evmData->length > index)
	//		return(resultOfdm->evmData->real[index]);
	//	else
	//		return NA_NUMBER;
	//}
	if (!strcmp(measurement, "evmPilot"))
	{
		return doubleValue;
	}
	/*{
	  if (resultOfdm->evmPilot && resultOfdm->evmPilot->length > index)
	  return(resultOfdm->evmPilot->real[index]);
	  else
	  return NA_NUMBER;
	  }*/
	if (!strcmp(measurement, "codingRate"))
	{
		iqx.SendCommand("fetch:txq:ofdm:info:crat?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		//Coding rate index: 0 = 1/2, 1 = 3/4, 2 = 2/3, 3 = 5/6, -1 = invalid.
		switch((int)doubleArray[1])
		{
			case 0: doubleValue = 1/2.0;
				break;
			case 1: doubleValue = 3/4.0;
				break;
			case 2: doubleValue = 2/3.0;
				break;
			case 3: doubleValue = 5/6.0;
				break;
			case -1:
			default:
				doubleValue = NA_NUMBER;
				break;
		}
		return doubleValue;
	}

	if (strstr(measurement, "rmsPower")!=0)  //all power is "rmsPowerNoGap" now. need to split these two when it is available as they are different in IQapi.
	{
		iqx.SendCommand("fetch:pow?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}

	if (strstr(measurement, "pkPower")!=0)  //float
	{
		iqx.SendCommand("fetch:pow:peak?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}

	if (strstr(measurement, "rmsMaxAvgPower")!=0)
	{
		iqx.SendCommand("fetch:pow:peak:aver?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}


	if(-1 == key )
	{
		if(strstr(measurement, "evmAll")!=0)//dB
		{
			key = 1; //
		}
		else if (strstr(measurement, "rmsPhaseNoise")!=0)//Deg
		{
			key = 2; //
		}
		else if (strstr(measurement, "freqErr")!=0)//Hz
		{
			key = 3; //
		}
		else if (strstr(measurement, "clockErr")!=0) // Hz
		{
			key = 4; //
		}
		else if (strstr(measurement, "dcLeakageDbc")!=0) // dB
		{
			key = 5; //
		}
		else if (strstr(measurement, "ampErr")!=0)  //else if (strstr(measurement, "ampErrDb")!=0) // Amplitude Imbalance (dB)
		{
			key = 6;
		}
		else if (strstr(measurement, "phaseErr")!=0) // Amplitude Imbalance (dB)
		{
			key = 7;
		}
		else
		{
		}

		if( key > -1) // fetch results
		{
			double doubleArray10[10] = {0};      // there are only 9 elements needed
			iqx.SendCommand("fetch:txq:ofdm?");  // use "FETC:TXQ:DSSS?" or "FETC:TXQ:DSSS:AVER?"
			splitAsDoubles(iqx.scpiRxBuffer, doubleArray10);
			if(doubleArray10[0]!= 0.0 )
			{
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				return NA_NUMBER;
			}
			doubleValue = doubleArray10[key];
			if(key ==6 && (!strcmp(measurement, "ampErr")) )
			{
				//TO DO: dB to linear
				doubleValue = pow(10,0.1*doubleValue);
			}
			return doubleValue;
		}
	}
	printf("\n\nERROR! Input keyword \"%s\" cannot be found in GetScalarMeasurement_80211ag()\n\n", measurement);
	return doubleValue;
}

// TODO: Use optimization: 1. use SCPI keywords 2. use set/get/check instrument state functions
double CIQmeasure_Scpi::GetScalarMeasurement_80211b(char *measurement, int index)
{
	double doubleValue  = NA_NUMBER;
	int  intValue = (int)NA_NUMBER;
	char *pch   = NULL;
	int key     = -1;
	int num     = 0;
	//dangerous to have only 10 in array size.
	int intArray[MAX_BUFFER_LEN] = {0};
	double doubleArray[MAX_BUFFER_LEN] = {0.00};


	if (strstr(measurement, "psduCrcFail")!=0) //int
	{
		iqx.SendCommand("fetch:dsss:psdu:crc?");
		splitAsIntegers(iqx.scpiRxBuffer, intArray);
		if(intArray[0]!= 0)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		else
		{
			doubleValue = intArray[1];
			return doubleValue;
		}
	}

	if (strstr(measurement, "on_time")!=0)  //float
	{
		iqx.SendCommand("fetch:ramp:on:trise?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		else
		{
			doubleValue = doubleArray[1];
			return doubleValue;
		}
	}

	if (strstr(measurement, "off_time")!=0)  //float
	{
		iqx.SendCommand("fetch:ramp:off:trise?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		else
		{
			doubleValue = doubleArray[1];
			return doubleValue;
		}
	}

	if (strstr(measurement, "rmsPower")!=0)   //all power is "rmsPowerNoGap" now. need to split these two when it is available as they are different in IQapi.
	{
		iqx.SendCommand("fetch:pow?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		else
		{
			doubleValue = doubleArray[1];
			return doubleValue;
		}
	}

	if (strstr(measurement, "pkPower")!=0)  //float
	{
		iqx.SendCommand("fetch:pow:peak?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		else
		{
			doubleValue = doubleArray[1];
			return doubleValue;
		}
	}

	//startPointers and stop pointers should not be here. it is a vector
	//  if (strstr(measurement, "startPointers")!=0)  //float
	//  {
	//iqx.SendCommand("fetch:sync?");
	//num = 0;
	//num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
	//if(doubleArray[0]!= 0.00)
	//{
	//	printf("\n\n -- Fetch results ERROR!!! \n\n");
	//	return NA_NUMBER;
	//}
	//else
	//{
	//	intValue = (int)doubleArray[1];
	//	//return doubleValue;
	//
	//	iqx.SendCommand("fetch:sync:pst?");
	//	num = 0;
	//	//has to be dynamic generated as number of packets can be larger than MAX_BUFFER_LEN
	//	double *startPointers = new double[intValue+1];
	//	num = splitAsDoubles(iqx.scpiRxBuffer, startPointers);
	//	if(startPointers[0]!= 0.00)
	//	{
	//		printf("\n\n -- Fetch results ERROR!!! \n\n");
	//		delete [] startPointers;
	//		return NA_NUMBER;
	//	}
	//	else
	//	{
	//		//need to convert back to samples as in IQapi.
	//		iqx.SendCommand("capt:data:srat?");
	//		//iqx.SendCommand("vsa1;srat?");
	//		num = 0;
	//		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
	//		intValue = (int)doubleArray[0];

	//		doubleValue = floor (startPointers[1]*intValue+0.5);
	//		delete [] startPointers;
	//		return doubleValue;
	//	}
	//}
	//  }

	//  if (strstr(measurement, "stopPointers")!=0)  //float
	//  {
	//iqx.SendCommand("fetch:sync?");
	//num = 0;
	//num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
	//if(doubleArray[0]!= 0.00)
	//{
	//	printf("\n\n -- Fetch results ERROR!!! \n\n");
	//	return NA_NUMBER;
	//}
	//else
	//{
	//	intValue = (int)doubleArray[1];

	//	iqx.SendCommand("fetch:sync:pet?");
	//	num = 0;
	//	double *stopPointers = new double[intValue+1];
	//	num = splitAsDoubles(iqx.scpiRxBuffer, stopPointers);
	//	if(stopPointers[0]!= 0.00)
	//	{
	//		printf("\n\n -- Fetch results ERROR!!! \n\n");
	//		delete [] stopPointers;
	//		return NA_NUMBER;
	//	}
	//	else
	//	{
	//		//need to convert back to samples as in IQapi.
	//		iqx.SendCommand("capt:data:srat?");
	//		//iqx.SendCommand("vsa1;srat?");
	//		num = 0;
	//		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
	//		intValue = (int)doubleArray[0];
	//
	//		doubleValue = floor(stopPointers[1]*intValue+0.5);
	//		delete [] stopPointers;
	//		return doubleValue;
	//	}
	//}
	//  }

	// - TX Quality "fetch:txq:dsss?"
	if(-1 == key )
	{
		if(strstr(measurement, "evmAll")!=0)//dB
		{
			key = 1; //
		}
		else if (strstr(measurement, "evmPk")!=0)//dB
		{
			key = 2; //
		}
		else if (strstr(measurement, "phaseErr")!=0)//Deg
		{
			key = 3; //
		}
		else if (strstr(measurement, "freqErr")!=0)//Hz
		{
			key = 4; //
		}
		else if (strstr(measurement, "maxFreqErr")!=0) // Hz
		{
			key = 5; //
		}
		//new build from Dec 8, 2011 (45912) used new format
		else if (strstr(measurement, "clockErr")!=0) // Hz
		{
			key = 6; //
		}
		else if (strstr(measurement, "loLeakageDb")!=0) // dB
		{
			key = 7; //
		}
		else if (strstr(measurement, "ampErr")!=0) // Amplitude Imbalance (dB)
		{
			key = 8; // (!strcmp(measurement, "ampErrDb")) SCPI return dB values, but IQapi return both. for backward compatible consideration
		}
		else if (strstr(measurement, "rmsPhaseNoise")!=0) // Amplitude Imbalance (dB)
		{
			key = 9;
		}
		else
		{
		}

		if( key > -1) // fetch results
		{
			double doubleArray10[10] = {0};      // there are only 9 elements needed
			iqx.SendCommand("fetch:txq:dsss?");  // use "FETC:TXQ:DSSS?" or "FETC:TXQ:DSSS:AVER?"
			splitAsDoubles(iqx.scpiRxBuffer, doubleArray10);
			if(doubleArray10[0]!= 0.0 )
			{
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				return NA_NUMBER;
			}
			else
			{
				doubleValue = doubleArray10[key];
				if(key ==8 && (!strcmp(measurement, "ampErr")) )
				{
					//TO DO: dB to linear
					doubleValue = pow(10,0.1*doubleValue);
				}
				return doubleValue;
			}
		}
	}

	//	new build from Dec 8, 2011 (45912) used new format to get datarate etc.
	if (strstr(measurement, "bitRate")!=0)  //float
	{
		iqx.SendCommand("fetch:dsss:info:drat?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		else
		{
			doubleValue = doubleArray[1];
			return doubleValue;
		}

	}

	if (strstr(measurement, "modType")!=0)  //float
	{
		iqx.SendCommand("fetch:dsss:info:mod?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		else
		{
			doubleValue = doubleArray[1];
			return doubleValue;
		}
	}

	if (strstr(measurement, "plcpCrcFail")!=0)  //float
	{
		iqx.SendCommand("fetch:dsss:plcp:crc?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		else
		{
			doubleValue = doubleArray[1];
			return doubleValue;
		}

	}

	if (strstr(measurement, "numPsduBytes")!=0)  //float
	{
		iqx.SendCommand("fetch:dsss:psdu:len?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		else
		{
			doubleValue = doubleArray[1];
			return doubleValue;
		}
	}

	if (strstr(measurement, "lockedClock")!=0)  //float
	{
		iqx.SendCommand("fetch:dsss:info:cloc?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		else
		{
			doubleValue = doubleArray[1];
			return doubleValue;
		}
	}

	if (strstr(measurement, "longPreamble")!=0)  //float
	{
		iqx.SendCommand("fetch:dsss:info:pre?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		else
		{
			doubleValue = doubleArray[1];
			return doubleValue;
		}
	}


	// - Sidelobe analysis results: "fetch:slobe?", float values
	if(-1 == key )
	{
		if(strstr(measurement, "peak_2_left")!=0)
		{
			key = 1; //
		}
		else if (strstr(measurement, "peak_1_left")!=0)
		{
			key = 2; //
		}
		else if (strstr(measurement, "peak_center")!=0)
		{
			key = 3; //
		}
		else if (strstr(measurement, "peak_1_right")!=0)
		{
			key = 4; //
		}
		else if (strstr(measurement, "peak_2_right")!=0)
		{
			key = 5; //
		}
		else
		{
			key = -1;
		}

		if( key > -1) // fetch results
		{
			iqx.SendCommand("fetch:slobe?");
			num = 0;
			num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
			if(doubleArray[0]!= 0.00)
			{
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				return NA_NUMBER;
			}
			else
			{
				doubleValue = doubleArray[key];
				return doubleValue;
			}
		}
	}

	printf("\n\nERROR! Input keyword \"%s\" cannot be found in GetScalarMeasurement_80211b()\n\n", measurement);
	return doubleValue;
}

// TODO: Use optimization: 1. use SCPI keywords 2. use set/get/check instrument state functions
double CIQmeasure_Scpi::GetScalarMeasurement_80211ac(char *measurement, int index)
{
	double doubleValue  = NA_NUMBER;
	int  intValue = (int)NA_NUMBER;
	char *pch   = NULL;
	int key     = -1;
	int num     = 0;
	int intArray[MAX_BUFFER_LEN] = {0};
	double doubleArray[MAX_BUFFER_LEN] = {0.00};

	//rxRmsPowerDb, all power is "rmsPowerNoGap" now. need to split these two when it is available as they are different in IQapi. it is not the LTS portion that we have in IQapi, need to update
	if (strstr(measurement, "rxRmsPower")!=0)
	{
		iqx.SendCommand("fetch:pow?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}

	if(-1 == key )
	{
		if(strstr(measurement, "evmAvgAll")!=0)//dB    //no consideration of nxn?
		{
			key = 1; //
		}
		else if (strstr(measurement, "PhaseNoiseDeg_RmsAll")!=0)//Deg
		{
			key = 2; //
		}
		else if (strstr(measurement, "freqErrorHz")!=0)//Hz
		{
			key = 3; //
		}
		else if (strstr(measurement, "symClockErrorPpm")!=0) // Hz
		{
			key = 4; //
		}
		else if (strstr(measurement, "dcLeakageDbc")!=0) // dB
		{
			key = 5; //
		}
		else if (strstr(measurement, "IQImbal_amplDb")!=0)  //else if (strstr(measurement, "ampErrDb")!=0) // Amplitude Imbalance (dB)
		{
			key = 6;
		}
		else if (strstr(measurement, "IQImbal_phaseDeg")!=0) // Amplitude Imbalance (dB)
		{
			key = 7;
		}
		else
		{
		}
	}

	if( key > -1) // fetch results
	{
		double doubleArray10[10] = {0};      // there are only 7 elements needed
		iqx.SendCommand("fetch:txq:ofdm?");  //
		splitAsDoubles(iqx.scpiRxBuffer, doubleArray10);
		g_11nPacketCheck = true;
		if(doubleArray10[0]!= 0.0 )
		{
			g_11nPacketValid = 0;
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		g_11nPacketValid = 1;
		doubleValue = doubleArray10[key];
		return doubleValue;
	}

	if (!strcmp(measurement, "packetDetection") || !strcmp(measurement, "acquisition")||!strcmp(measurement, "demodulation") ) //use TXQ status to return these values now. cannot tell the difference between 11a/g and 11n
	{
		if(true == g_11nPacketCheck)
		{
			doubleValue = g_11nPacketValid;
		}
		else
		{
			double doubleArray10[10] = {0.0};
			iqx.SendCommand("fetch:txq:ofdm?");
			num = 0;
			num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray10);
			g_11nPacketCheck = true;
			if(doubleArray10[0]!= 0.0 )
			{
				g_11nPacketValid = 0;
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				return NA_NUMBER;
			}
			g_11nPacketValid = 1;
			doubleValue = g_11nPacketValid;
		}
		return doubleValue;
	}

	if (strstr(measurement, "psduCRC")!=0) //int
	{
		iqx.SendCommand("fetch:ofdm:psdu:crc?");
		num = 0;
		num =splitAsIntegers(iqx.scpiRxBuffer, intArray);
		if(intArray[0]!= 0)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = intArray[1];
		return doubleValue;
	}


	//	else if (!strcmp(measurement, "isolationDb"))
	//	{
	//		if (resultMimo->isolationDb && resultMimo->isolationDb->length > index)
	//		{
	//			return(resultMimo->isolationDb->real[index]);
	//		}
	//		else
	//			return NA_NUMBER;
	//	}


	if (!strcmp(measurement, "rateInfo_bandwidthMhz"))
	{
		iqx.SendCommand("fetch:txq:ofdm:info:cbw?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		// doubleValue = doubleArray[1];
		doubleValue = doubleArray[1]/1e6; // previous SCPI returned in MHz, now returns Hz, JMF, 01/19/12
		return doubleValue;
	}

	if (!strcmp(measurement, "rateInfo_dataRateMbps"))
	{
		iqx.SendCommand("fetch:txq:ofdm:info:drat?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}

	if (!strcmp(measurement, "rateInfo_spatialStreams"))
	{
		iqx.SendCommand("fetch:txq:ofdm:info:nsst?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}
	if (!strcmp(measurement, "rateInfo_spaceTimeStreams"))
	{
		iqx.SendCommand("fetch:txq:ofdm:info:nsts?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}
	if (!strcmp(measurement, "VHTSigBFieldCRC"))
	{
		iqx.SendCommand("fetch:txq:ofdm:sigb:crc?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}


	//	else if (!strcmp(measurement, "analyzedRange")) //need to check how it is use now. using startingsec and stoppingsec
	//	{
	//		if (resultMimo->analyzedRange && resultMimo->analyzedRange->length > index)
	//			return(resultMimo->analyzedRange->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else if (!strcmp(measurement, "htSig1_htLength"))
	//	{
	//		if (resultMimo->htSig1_htLength && resultMimo->htSig1_htLength->length > index)
	//			return(resultMimo->htSig1_htLength->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else if (!strcmp(measurement, "htSig1_mcsIndex"))
	//	{
	//		if (resultMimo->htSig1_mcsIndex && resultMimo->htSig1_mcsIndex->length > index)
	//			return(resultMimo->htSig1_mcsIndex->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else if (!strcmp(measurement, "htSig1_bandwidth"))
	//	{
	//		if (resultMimo->htSig1_bandwidth && resultMimo->htSig1_bandwidth->length > index)
	//			return(resultMimo->htSig1_bandwidth->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else if (!strcmp(measurement, "htSig2_advancedCoding"))
	//	{
	//		if (resultMimo->htSig2_advancedCoding && resultMimo->htSig2_advancedCoding->length > index)
	//			return(resultMimo->htSig2_advancedCoding->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else if (!strcmp(measurement, "rateInfo_spaceTimeStreams"))
	//	{
	//		if (resultMimo->rateInfo_spaceTimeStreams && resultMimo->rateInfo_spaceTimeStreams->length > index)
	//			return(resultMimo->rateInfo_spaceTimeStreams->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else
	//	{
	//		return NA_NUMBER;
	//	}
	printf("\n\nERROR! Input keyword \"%s\" cannot be found in GetScalarMeasurement_80211ac()\n\n", measurement);
	return doubleValue;
}

// TODO: Use optimization: 1. use SCPI keywords 2. use set/get/check instrument state functions
double CIQmeasure_Scpi::GetScalarMeasurement_80211n(char *measurement, int index)
{
	double doubleValue  = NA_NUMBER;
	int  intValue = (int)NA_NUMBER;
	char *pch   = NULL;
	int key     = -1;
	int num     = 0;
	int intArray[MAX_BUFFER_LEN] = {0};
	double doubleArray[MAX_BUFFER_LEN] = {0.00};

	//rxRmsPowerDb, all power is "rmsPowerNoGap" now. need to split these two when it is available as they are different in IQapi. it is not the LTS portion that we have in IQapi, need to update
	if (strstr(measurement, "rxRmsPower")!=0)
	{
		iqx.SendCommand("fetch:pow?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}

	if(-1 == key )
	{
		if(strstr(measurement, "evmAvgAll")!=0)//dB    //no consideration of nxn?
		{
			key = 1; //
		}
		else if (strstr(measurement, "PhaseNoiseDeg_RmsAll")!=0)//Deg
		{
			key = 2; //
		}
		else if (strstr(measurement, "freqErrorHz")!=0)//Hz
		{
			key = 3; //
		}
		else if (strstr(measurement, "symClockErrorPpm")!=0) // Hz
		{
			key = 4; //
		}
		else if (strstr(measurement, "dcLeakageDbc")!=0) // dB
		{
			key = 5; //
		}
		else if (strstr(measurement, "IQImbal_amplDb")!=0)  //else if (strstr(measurement, "ampErrDb")!=0) // Amplitude Imbalance (dB)
		{
			key = 6;
		}
		else if (strstr(measurement, "IQImbal_phaseDeg")!=0) // Amplitude Imbalance (dB)
		{
			key = 7;
		}
		else
		{
		}
	}

	if( key > -1) // fetch results
	{
		double doubleArray10[10] = {0};      // there are only 7 elements needed
		iqx.SendCommand("fetch:txq:ofdm?");  //
		splitAsDoubles(iqx.scpiRxBuffer, doubleArray10);
		g_11nPacketCheck = true;
		if(doubleArray10[0]!= 0.0 )
		{
			g_11nPacketValid = 0;
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		g_11nPacketValid = 1;
		doubleValue = doubleArray10[key];
		return doubleValue;
	}

	if (!strcmp(measurement, "packetDetection") || !strcmp(measurement, "acquisition")||!strcmp(measurement, "demodulation") ) //use TXQ status to return these values now. cannot tell the difference between 11a/g and 11n
	{
		if(true == g_11nPacketCheck)
		{
			doubleValue = g_11nPacketValid;
		}
		else
		{
			double doubleArray10[10] = {0.0};
			iqx.SendCommand("fetch:txq:ofdm?");
			num = 0;
			num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray10);
			g_11nPacketCheck = true;
			if(doubleArray10[0]!= 0.0 )
			{
				g_11nPacketValid = 0;
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				return NA_NUMBER;
			}
			g_11nPacketValid = 1;
			doubleValue = g_11nPacketValid;
		}
		return doubleValue;
	}

	if (strstr(measurement, "psduCRC")!=0) //int
	{
		iqx.SendCommand("fetch:ofdm:psdu:crc?");
		num = 0;
		num =splitAsIntegers(iqx.scpiRxBuffer, intArray);
		if(intArray[0]!= 0)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = intArray[1];
		return doubleValue;
	}


	//	else if (!strcmp(measurement, "isolationDb"))
	//	{
	//		if (resultMimo->isolationDb && resultMimo->isolationDb->length > index)
	//		{
	//			return(resultMimo->isolationDb->real[index]);
	//		}
	//		else
	//			return NA_NUMBER;
	//	}


	if (!strcmp(measurement, "rateInfo_bandwidthMhz"))
	{
		iqx.SendCommand("fetch:txq:ofdm:info:cbw?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		//doubleValue = doubleArray[1];
		doubleValue = doubleArray[1]/1e6; // previous SCPI returned in MHz, now returns Hz, JMF, 01/19/12
		return doubleValue;
	}

	if (!strcmp(measurement, "rateInfo_dataRateMbps"))
	{
		iqx.SendCommand("fetch:txq:ofdm:info:drat?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}

	if (!strcmp(measurement, "rateInfo_spatialStreams"))
	{
		iqx.SendCommand("fetch:txq:ofdm:info:nsst?");
		num = 0;
		num = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return NA_NUMBER;
		}
		doubleValue = doubleArray[1];
		return doubleValue;
	}

	//	else if (!strcmp(measurement, "analyzedRange")) //need to check how it is use now. using startingsec and stoppingsec
	//	{
	//		if (resultMimo->analyzedRange && resultMimo->analyzedRange->length > index)
	//			return(resultMimo->analyzedRange->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else if (!strcmp(measurement, "htSig1_htLength"))
	//	{
	//		if (resultMimo->htSig1_htLength && resultMimo->htSig1_htLength->length > index)
	//			return(resultMimo->htSig1_htLength->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else if (!strcmp(measurement, "htSig1_mcsIndex"))
	//	{
	//		if (resultMimo->htSig1_mcsIndex && resultMimo->htSig1_mcsIndex->length > index)
	//			return(resultMimo->htSig1_mcsIndex->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else if (!strcmp(measurement, "htSig1_bandwidth"))
	//	{
	//		if (resultMimo->htSig1_bandwidth && resultMimo->htSig1_bandwidth->length > index)
	//			return(resultMimo->htSig1_bandwidth->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else if (!strcmp(measurement, "htSig2_advancedCoding"))
	//	{
	//		if (resultMimo->htSig2_advancedCoding && resultMimo->htSig2_advancedCoding->length > index)
	//			return(resultMimo->htSig2_advancedCoding->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else if (!strcmp(measurement, "rateInfo_spaceTimeStreams"))
	//	{
	//		if (resultMimo->rateInfo_spaceTimeStreams && resultMimo->rateInfo_spaceTimeStreams->length > index)
	//			return(resultMimo->rateInfo_spaceTimeStreams->real[index]);
	//		else
	//			return NA_NUMBER;
	//	}
	//	else
	//	{
	//		return NA_NUMBER;
	//	}
	printf("\n\nERROR! Input keyword \"%s\" cannot be found in GetScalarMeasurement_80211n()\n\n", measurement);
	return doubleValue;
}

int CIQmeasure_Scpi::GetVectorMeasurement_80211ag(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	int     err             = 0;
	int     rxLen           = 0;
	int     numResult       = 0;
	double  *doubleArray    = NULL; //[MAX_DATA_LENGTH] = {0.00};
	int     *intArray       = NULL; //[MAX_DATA_LENGTH] = {0};

	//Allocate memory, workaround only, JK 2012/01/12
	doubleArray = (double *)malloc((bufferLength+1)*sizeof(double));
	intArray    = (int *)   malloc((bufferLength+1)*sizeof(int));
	if(NULL==doubleArray || NULL ==intArray)
	{
		iqx.LogPrintf("\nERROR in malloc() in function GetVectorMeasurement_80211ag() !!! \n");
		err = -2;  // memory alloation error
	}


	//not implemented yet.
	if (!strcmp(measurement, "hhEst"))
	{
		rxLen = iqx.SendCommand("fetch:ofdm:sfl?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR in GetVectorMeasurement_80211ag()!!! \n\n");
			err = -1 ; //return 0;
		}
		else
		{
			double doubleArrayTemp[64] = {0.00};
			//double hhEstP =0.0;
			int carrierNum = 0;
			int numTone = numResult -1;
			if (numTone == 64)
			{
				carrierNum = 26;
			}else
			{
				//it is not 11a/g mode, force error return, maybe need to support 11p, half rate etc in the future
				printf("\n\n -- Num of tones, %d, is not an expected value, 64!!! \n\n", numTone);
				//return 0;
			}

			//for(int i=0; i<numTone+1; i++)
			//	printf("SFL[%2d] = %.6f \n",i,doubleArray[i]);
			for(int i=0; i<numTone/2; i++)
			{
				doubleArrayTemp[i] = doubleArray[i+numTone/2+1];
				if (SCPI_INF >= doubleArrayTemp[i] && SCPI_NINF <= doubleArrayTemp[i] )
				{
					doubleArrayTemp[i] = pow(10, 0.05*checkDoubleValue(doubleArrayTemp[i])); //convert it back to linear amp but not power, thus 0.05.
					//	hhEstP = hhEstP+doubleArrayTemp[i]* doubleArrayTemp[i];
				}
				//keep SCPI values
				//else
				//{
				//	doubleArrayTemp[i] = NA_NUMBER;
				//}
			}
			for(int i=numTone/2; i<numTone; i++)
			{
				doubleArrayTemp[i] = doubleArray[i-numTone/2+1];
				if (SCPI_INF >= doubleArrayTemp[i] && SCPI_NINF <= doubleArrayTemp[i] )
				{
					doubleArrayTemp[i] = pow(10, 0.05*checkDoubleValue(doubleArrayTemp[i])); //convert it back to linear amp but not power, thus 0.05.
					//	hhEstP = hhEstP+doubleArrayTemp[i]* doubleArrayTemp[i];
				}
				//keep SCPI values
				//else
				//{
				//	doubleArrayTemp[i] = NA_NUMBER;
				//}
			}
			//use loleakagedbc to calculate buffeReal[0] for backward compatible, SFL is normalized to 0 dB of total power
			double doubleArray10[10] = {0.0};      // there are only 7 elements needed
			iqx.SendCommand("fetch:txq:ofdm?");
			splitAsDoubles(iqx.scpiRxBuffer, doubleArray10);
			if(doubleArray10[0]!= 0.0 )
			{
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				err = -1; //return 0;
			}
			doubleArrayTemp[0] = pow(10, 0.05* checkDoubleValue(doubleArray10[5]+10 * log10((double)(carrierNum*2))));

			//double temp = 10.0*log10(doubleArrayTemp[0]*doubleArrayTemp[0])-10.0*log10(hhEstP/carrierNum/2)-10 * log10((double)(carrierNum*2));
			//printf("mapLO is %0.4f, scpiLO is %0.4f ", temp, doubleArray10[5]);

			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1; //

			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArrayTemp[i];
				bufferImag[i] = 0;
			}
		}

		err = 0; //return bufferLength;
	}
	else if (!strcmp(measurement, "psdu"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:psdu:bits?");
		numResult = splitAsIntegers(iqx.scpiRxBuffer, intArray, bufferLength+1);
		if(intArray[0]!= 0)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; //return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = intArray[i+1];
			}
		}
		err = 0; //return bufferLength;
	}

	else if (strstr(measurement, "startPointers")!=0 ||strstr(measurement, "startSeconds")!=0)  // float array, added new result keyword, startSeconds for future used.
	{
		double tempArray1[MAX_BUFFER_LEN] = {0.00};
		double tempArray2[MAX_BUFFER_LEN] = {0.00};
		iqx.SendCommand("fetch:sync?");
		splitAsDoubles(iqx.scpiRxBuffer, tempArray1);
		if(tempArray1[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; //return 0;
		}
		else
		{
			int numPacket =0; // numCompletePacket = 0;
			numPacket = (int)tempArray1[1];
			//numCompletePacket = tempArray1[2];
			if (numPacket>MAX_DATA_LENGTH && bufferLength>MAX_DATA_LENGTH)
			{
				printf("\n\n -- Number of Packets is %.1f, larger than buffer size, %d. ERROR! \n\n", doubleArray[1], MAX_DATA_LENGTH);
				err = -1; //return 0;
			}
			else
			{

				int packetSamplingRate = 0;
				if(strstr(measurement, "startPointers")!=0 )
				{
					//need to convert back to samples as in IQapi.
					iqx.SendCommand("capt:data:srat?");
					//iqx.SendCommand("vsa1;srat?");
					err = splitAsDoubles(iqx.scpiRxBuffer, tempArray2);
					if(tempArray2[0]== 0.00)
					{
						printf("\n\n -- VSA Sampling Rate is  %.0f. Incorrect!!! \n\n", tempArray2[0]);
						err = -1; //return 0;
					}
					else
					{
						packetSamplingRate = (int)tempArray2[0];
					}
				}
				else
				{
					//do nothing
				}

				rxLen = iqx.SendCommand("fetch:sync:pst?");
				numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
				if(doubleArray[0]!= 0.00)
				{
					printf("\n\n -- Fetch results ERROR!!! \n\n");
					err = -1; //return 0;
				}
				else
				{
					if ((numResult-1)<bufferLength)
						bufferLength=numResult-1;
					for(int i=0; i<bufferLength; i++)
					{
						if(strstr(measurement, "startPointers")!=0 )
						{
							bufferReal[i] = floor(doubleArray[i+1]*packetSamplingRate+0.5); //convert it to pointer of sample
						}
						else
						{
							bufferReal[i] = doubleArray[i+1]; //unit in second from SCPI
						}

					}
				}
				err = 0; //return bufferLength;

			}
		}
	}

	else if (strstr(measurement, "stopPointers")!=0 || strstr(measurement, "stopSeconds")!=0 )  // float array, added new result keyword, stopSeconds for future used.
	{
		double tempArray1[MAX_BUFFER_LEN] = {0.00};
		double tempArray2[MAX_BUFFER_LEN] = {0.00};
		iqx.SendCommand("fetch:sync?");
		splitAsDoubles(iqx.scpiRxBuffer, tempArray1);
		if(tempArray1[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			int numPacket =0; // numCompletePacket = 0;
			numPacket = (int)tempArray1[1];
			//SCPI returns the incomplete packets' ending point anyway.
			//numCompletePacket = tempArray1[2];
			//if (numCompletePacket < numPacket) //incomplete packet does not have ending point
			//	numPacket=numCompletePacket;
			if (numPacket>MAX_DATA_LENGTH && bufferLength>MAX_DATA_LENGTH)
			{
				printf("\n\n -- Number of Packets is %.1f, larger than buffer size, %d. ERROR! \n\n", doubleArray[1], MAX_DATA_LENGTH);
				err = -1; // return 0;
			}
			else
			{
				int packetSamplingRate = 0;
				//need to convert back to samples as in IQapi.
				if  (strstr(measurement, "stopPointers")!=0)
				{
					iqx.SendCommand("capt:data:srat?");
					//iqx.SendCommand("vsa1;srat?");
					err = splitAsDoubles(iqx.scpiRxBuffer, tempArray2);
					if(tempArray2[0]== 0.00)
					{
						printf("\n\n -- VSA Sampling Rate is  %.0f. Incorrect!!! \n\n", tempArray2[0]);
						err = -1; //return 0;
					}
					else
					{
						packetSamplingRate = (int)tempArray2[0];
					}
				}
				else
				{
					//do nothing
				}

				rxLen = iqx.SendCommand("fetch:sync:pet?");
				numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
				if(doubleArray[0]!= 0.00)
				{
					printf("\n\n -- Fetch results ERROR!!! \n\n");
					err = -1; //return 0;
				}
				else
				{
					if ((numResult-1)<bufferLength)
						bufferLength=numResult-1;
					for(int i=0; i<bufferLength; i++)
					{
						if (strstr(measurement, "stopPointers")!=0)
						{
							bufferReal[i] = floor(doubleArray[i+1]*packetSamplingRate+0.5); //convert it to pointer of sample
						}else
						{
							bufferReal[i] = doubleArray[i+1]; //unit in second from SCPI
						}

					}
				}
				err = 0; //return bufferLength;

			}
		}
	}

	////not ready yet
	else if (strstr(measurement, "plcp")!=0)  // int array
	{
		bufferLength =0;
		err = 0; //return bufferLength;
	}
	//{
	//	rxLen = iqx.SendCommand("fetch:ofdm:plcp:data?");
	//	err = splitAsIntegers(iqx.scpiRxBuffer, intArray, bufferLength);
	//	if(intArray[0]!= 0.00)
	//	{
	//		printf("\n\n -- Fetch results ERROR!!! \n\n");
	//      return 0;
	//	}
	//	else
	//	{
	//		for(int i=0; i<bufferLength; i++)
	//		{
	//			bufferReal[i] = intArray[i+1];
	//		}
	//	}
	//	return bufferLength;
	//}
	else
	{
		printf("\n\nERROR! Input keyword \"%s\" cannot be found in GetVectorMeasurement_80211ag()\n\n", measurement);
		err = -1;
	}

	//clear allocated memory before exist function
	if(doubleArray) free(doubleArray);
	if(intArray)    free (intArray);


	if(err == 0)
	{
		return bufferLength;        // normal return
	}
	else
	{
		return 0;                 //error happens
	}
}

int CIQmeasure_Scpi::GetVectorMeasurement_80211b(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	int err = 0;
	int rxLen   = 0;
	int numResult = 0;
	double *doubleArray = NULL; //[MAX_DATA_LENGTH] = {0.00};
	int	   *intArray	= NULL; //[MAX_DATA_LENGTH] = {0};

	// Allocate memory, workaround only, JMF 2012/01/12
	doubleArray = (double *)malloc((bufferLength+1)*sizeof(double));
	intArray    = (int *)   malloc((bufferLength+1)*sizeof(int));
	if(NULL==doubleArray || NULL ==intArray)
	{
		iqx.LogPrintf("\nERROR in malloc() in function GetVectorMeasurement_80211b() !!! \n");
		err = -2;  // memory alloation error
	}

	if (strstr(measurement, "startPointers")!=0 ||strstr(measurement, "startSeconds")!=0)  // float array, added new result keyword, startSeconds for future used.
	{
		double tempArray1[MAX_BUFFER_LEN] = {0.00};
		double tempArray2[MAX_BUFFER_LEN] = {0.00};
		iqx.SendCommand("fetch:sync?");
		splitAsDoubles(iqx.scpiRxBuffer, tempArray1);
		if(tempArray1[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; //return 0;
		}
		else
		{
			int numPacket =0; // numCompletePacket = 0;
			numPacket = (int) tempArray1[1];
			//numCompletePacket = tempArray1[2];
			if (numPacket>MAX_DATA_LENGTH && bufferLength>MAX_DATA_LENGTH)
			{
				printf("\n\n -- Number of Packets is %.1f, larger than buffer size, %d. ERROR! \n\n", doubleArray[1], MAX_DATA_LENGTH);
				err = -1; //return 0;
			}
			else
			{

				int packetSamplingRate = 0;
				if(strstr(measurement, "startPointers")!=0 )
				{
					//need to convert back to samples as in IQapi.
					iqx.SendCommand("capt:data:srat?");
					//iqx.SendCommand("vsa1;srat?");
					err = splitAsDoubles(iqx.scpiRxBuffer, tempArray2);
					if(tempArray2[0]== 0.00)
					{
						printf("\n\n -- VSA Sampling Rate is  %.0f. Incorrect!!! \n\n", tempArray2[0]);
						err = -1; // return 0;
					}
					else
					{
						packetSamplingRate = (int)tempArray2[0];
					}
				}
				else
				{
					//do nothing
				}

				rxLen = iqx.SendCommand("fetch:sync:pst?");
				numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
				if(doubleArray[0]!= 0.00)
				{
					printf("\n\n -- Fetch results ERROR!!! \n\n");
					err = -1; //return 0;
				}
				else
				{
					if ((numResult-1)<bufferLength)
						bufferLength=numResult-1;
					for(int i=0; i<bufferLength; i++)
					{
						if(strstr(measurement, "startPointers")!=0 )
						{
							bufferReal[i] = floor(doubleArray[i+1]*packetSamplingRate+0.5); //convert it to pointer of sample
						}
						else
						{
							bufferReal[i] = doubleArray[i+1]; //unit in second from SCPI
						}

					}
				}
				err = 0; //return bufferLength;

			}
		}
	}

	else if (strstr(measurement, "stopPointers")!=0 || strstr(measurement, "stopSeconds")!=0 )  // float array, added new result keyword, stopSeconds for future used.
	{
		double tempArray1[MAX_BUFFER_LEN] = {0.00};
		double tempArray2[MAX_BUFFER_LEN] = {0.00};
		iqx.SendCommand("fetch:sync?");
		splitAsDoubles(iqx.scpiRxBuffer, tempArray1);
		if(tempArray1[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			int numPacket =0; // numCompletePacket = 0;
			numPacket = (int)tempArray1[1];
			//SCPI returns the incomplete packets' ending point anyway.
			//numCompletePacket = tempArray1[2];
			//if (numCompletePacket < numPacket) //incomplete packet does not have ending point
			//	numPacket=numCompletePacket;
			if (numPacket>MAX_DATA_LENGTH && bufferLength>MAX_DATA_LENGTH)
			{
				printf("\n\n -- Number of Packets is %.1f, larger than buffer size, %d. ERROR! \n\n", doubleArray[1], MAX_DATA_LENGTH);
				err = -1; // return 0;
			}
			else
			{
				int packetSamplingRate = 0;
				//need to convert back to samples as in IQapi.
				if  (strstr(measurement, "stopPointers")!=0)
				{
					iqx.SendCommand("capt:data:srat?");
					//iqx.SendCommand("vsa1;srat?");
					err = splitAsDoubles(iqx.scpiRxBuffer, tempArray2);
					if(tempArray2[0]== 0.00)
					{
						printf("\n\n -- VSA Sampling Rate is  %.0f. Incorrect!!! \n\n", tempArray2[0]);
						err = -1; // return 0;
					}
					else
					{
						packetSamplingRate = (int)tempArray2[0];
					}
				}
				else
				{
					//do nothing
				}

				rxLen = iqx.SendCommand("fetch:sync:pet?");
				numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
				if(doubleArray[0]!= 0.00)
				{
					printf("\n\n -- Fetch results ERROR!!! \n\n");
					err = -1; // return 0;
				}
				else
				{
					if ((numResult-1)<bufferLength)
						bufferLength=numResult-1;
					for(int i=0; i<bufferLength; i++)
					{
						if (strstr(measurement, "stopPointers")!=0)
						{
							bufferReal[i] = floor(doubleArray[i+1]*packetSamplingRate+0.5); //convert it to pointer of sample
						}else
						{
							bufferReal[i] = doubleArray[i+1]; //unit in second from SCPI
						}

					}
				}
				err = 0; // return bufferLength;

			}
		}
	}

	//there is no evmInPlcp anymore
	else if (strstr(measurement, "symConst")!=0)  // new keyword definded by Zhiyong, Dec. 2011
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:dsss:cons:real?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}

		rxLen = iqx.SendCommand("fetch:dsss:cons:imag?");
		tempNumResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if (tempNumResult!=numResult) //real and imag must have same length
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{

			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferImag[i] = doubleArray[i+1];
			}
		}

		err = 0; // return bufferLength;
	}

	//not supported in SCPI.
	//if (strstr(measurement, "evmInPreamble")!=0)  // float array
	//{
	//    return bufferLength;
	//}


	//evmInPsdu is not implemented.
	//  if (strstr(measurement, "evmInPsdu")!=0)  // float array
	//  {
	//return bufferLength;
	//  }

	else if (strstr(measurement, "evmErr")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:dsss:evmt?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}


	else if (!strcmp(measurement, "loLeakageDb"))
	{
		iqx.SendCommand("fetch:txq:dsss?");  // use "FETC:TXQ:DSSS?" or "FETC:TXQ:DSSS:AVER?"
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.0 )
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; //return (int)NA_NUMBER;
		}
		else
		{
			bufferLength = 1;
			bufferReal[0] = doubleArray[7];
		}
		err = 0; // return bufferLength;
	}

	else if (strstr(measurement, "freqErrTimeVect")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:dsss:fevt:time?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; //return bufferLength;
	}

	else if (strstr(measurement, "off_power_inst")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:ramp:off:inst?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}

	else if (strstr(measurement, "off_power_peak")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:ramp:off:peak?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}

	else if (strstr(measurement, "off_time_vect")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:ramp:off:time?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}
	else if (strstr(measurement, "off_mask_x")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:ramp:off:xmask?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}
	else if (strstr(measurement, "off_mask_y")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:ramp:off:ymask?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}
	else if (strstr(measurement, "on_power_inst")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:ramp:on:inst?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}
	else if (strstr(measurement, "on_power_peak")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:ramp:on:peak?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}
	else if (strstr(measurement, "on_time_vect")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:ramp:on:time?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}
	else if (strstr(measurement, "on_mask_x")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:ramp:on:xmask?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}
	else if (strstr(measurement, "on_mask_y")!=0)  // float array
	{
		rxLen = iqx.SendCommand("fetch:ramp:on:ymask?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}


	//** original int array return
	else if (strstr(measurement, "scramblerInit")!=0)  // int array
	{
		rxLen = iqx.SendCommand("fetch:dsss:scr?");
		numResult = splitAsIntegers(iqx.scpiRxBuffer, intArray, bufferLength+1);
		if(intArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err  = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = intArray[i+1];
			}
		}
		err = 0 ; // return bufferLength;
	}

	else if (strstr(measurement, "psdu")!=0)  // int array
	{
		rxLen = iqx.SendCommand("fetch:dsss:psdu?");
		numResult = splitAsIntegers(iqx.scpiRxBuffer, intArray, bufferLength+1);
		if(intArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = intArray[i+1];
			}
		}
		err = 0; //return bufferLength;
	}

	else if (strstr(measurement, "plcp")!=0)  // int array
	{
		rxLen = iqx.SendCommand("fetch:dsss:plcp:data?");
		numResult = splitAsIntegers(iqx.scpiRxBuffer, intArray, bufferLength+1);
		if(intArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = intArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}

	//additional
	else if (!strcmp(measurement, "x"))  // float array
	{
		rxLen = iqx.SendCommand("fetch:spec:ofr?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}

	else if (!strcmp(measurement, "y"))  // float array
	{
		rxLen = iqx.SendCommand("fetch:spec:aver?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}
	else
	{
		printf("\n\nERROR! Input keyword \"%s\" cannot be found in GetVectorMeasurement_80211b()\n\n", measurement);
		err = -1;
	}

	if (doubleArray)	free(doubleArray);
	if (intArray)		free(intArray);
	if (err == 0)
	{
		return bufferLength;  // normal return
	}
	else
	{
		return 0; // error happens
	}
	//return err;
}
int CIQmeasure_Scpi::GetVectorMeasurement_80211ac(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	int err = 0;
	int rxLen   = 0;
	int numResult = 0;
	static double doubleArray[MAX_DATA_LENGTH] = {0.00};
	static int intArray[MAX_DATA_LENGTH] = {0};

	if (!strcmp(measurement, "channelEst")) //signal one/ one tester only now.
	{

		rxLen = iqx.SendCommand("fetch:ofdm:sfl?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			static double doubleArrayTemp[512] = {0.00};
			int carrierNum = 0;
			int numTone = numResult-1;
			if (numTone == 64) //VHT20 FFT size
			{
				carrierNum = 28;
			}
			else if (numTone == 128) //VHT40 FFT size //non-HT duplicate is not considered here
			{
				//get packet format ...  non-HT 58-6 = 26*2
				carrierNum = 58-2+1;
			}
			else if (numTone == 256) //VHT80 FFT size //non-HT duplicate is not considered here
			{
				//get packet format ...... =26*4
				carrierNum = 122-2+1;
			}
			else if (numTone == 512) //VHT160, 80+80 FFT size  //get packet format ...... =26*8
			{
				carrierNum = 250-6+1;
			}
			else
			{
				//it is not VHT mode, force error return
				printf("\n\n -- Num of tones, %d, is not an expected value, either 64 or 128!!! \n\n", numTone);
				return 0;
			}
			for(int i=0; i<numTone/2; i++)
			{
				doubleArrayTemp[i] = doubleArray[i+numTone/2+1];
				if (SCPI_INF >= doubleArrayTemp[i] && SCPI_NINF <= doubleArrayTemp[i] )
				{
					doubleArrayTemp[i] = pow(10, 0.05*checkDoubleValue(doubleArrayTemp[i])); //convert it back to linear amp but not power, thus 0.05.
				}
				//keep SCPI values
				//else
				//{
				//	doubleArrayTemp[i] = NA_NUMBER;
				//}
			}
			for(int i=numTone/2; i<numTone; i++)
			{
				doubleArrayTemp[i] = doubleArray[i-numTone/2+1];
				if (SCPI_INF >= doubleArrayTemp[i] && SCPI_NINF <= doubleArrayTemp[i] )
				{
					doubleArrayTemp[i] = pow(10, 0.05*checkDoubleValue(doubleArrayTemp[i])); //convert it back to linear amp but not power, thus 0.05.
				}
				//keep SCPI values
				//else
				//{
				//	doubleArrayTemp[i] = NA_NUMBER;
				//}
			}
			//use loleakagedbc to calculate buffeReal[0] for backward compatible, SFL is normalized to 0 dB of total power
			double doubleArray10[10] = {0.0};      // there are only 7 elements needed
			iqx.SendCommand("fetch:txq:ofdm?");
			splitAsDoubles(iqx.scpiRxBuffer, doubleArray10);
			if(doubleArray10[0]!= 0.0 )
			{
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				return 0;
			}
			doubleArrayTemp[0] = pow(10, 0.05* checkDoubleValue(doubleArray10[5]+10 * log10((double)(carrierNum*2))));

			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1; //

			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArrayTemp[i];
				bufferImag[i] = 0;
			}

		}
		return bufferLength;
	}

	if (!strcmp(measurement, "rxRmsPowerDb"))
	{
		bufferLength =0;
		return bufferLength;
	}
	if (!strcmp(measurement, "isolationDb"))

	{
		bufferLength =0;
		return bufferLength;
	}

	//need to interleave the streams result for MIMO according to Roman, nightmare ..., shall return the composite? FETC:OFDM:EVMS:COMP?
	if (!strcmp(measurement, "evmSymbols"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:evms:str1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		return bufferLength;
	}

	//per stream will be a better approach, "evmSymbolsStr1", "evmSymbolsStr2" ... Str8.
	if (!strcmp(measurement, "evmSymbolsStr1"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:evms:str1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		return bufferLength;
	}

	//need to talk to Roman on details for MIMO, might be same as evmSymbols,  shall return the composite? FETC:OFDM:EVMT:COMP?
	if (!strcmp(measurement, "evmTones"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:evmt:str1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		return bufferLength;
	}

	//per stream will be a better approach, "evmTonesStr1", "evmTonesStr2" ... Str8.
	if (!strcmp(measurement, "evmTonesStr1"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:evmt:str1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		return bufferLength;
	}

	//per signal/vsa
	//need to talk to Roman on details for MIMO, might be same as evmSymbols, shall return the composite? FETC:OFDM:EVMS:COMP?
	if (!strcmp(measurement, "PhaseNoiseDeg_Symbols"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:pes:sign1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		return bufferLength;
	}

	//per signal will be a better approach, "PhaseNoiseDeg_SymbolsSign1", "PhaseNoiseDeg_SymbolsSign2" ... Sign8.
	if (!strcmp(measurement, "PhaseNoiseDeg_SymbolsSign1"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:pes:sign1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		return bufferLength;
	}

	if (!strcmp(measurement, "demodSymbols"))
	{
		bufferLength =0;
		return bufferLength;
	}

	printf("\n\nERROR! Input keyword \"%s\" cannot be found in GetVectorMeasurement_80211ac()\n\n", measurement);
	return err;
}

int CIQmeasure_Scpi::GetVectorMeasurement_80211n(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	int err = 0;
	int rxLen   = 0;
	int numResult = 0;
	double *doubleArray = NULL; // [MAX_DATA_LENGTH] = {0.00};
	int *intArray = NULL; // [MAX_DATA_LENGTH] = {0};

	// Allocate memory, workaround only, JMF 2012/01/12
	doubleArray = (double *)malloc((bufferLength+1)*sizeof(double));
	intArray    = (int *)   malloc((bufferLength+1)*sizeof(int));
	if(NULL==doubleArray || NULL ==intArray)
	{
		iqx.LogPrintf("\nERROR in malloc() in function GetVectorMeasurement_80211n() !!! \n");
		err = -2;  // memory alloation error
	}

	if (!strcmp(measurement, "channelEst")) //signal one/ one tester only now.
	{

		rxLen = iqx.SendCommand("fetch:ofdm:sfl?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			double doubleArrayTemp[128] = {0.00};
			int carrierNum = 0;
			int numTone = numResult-1;
			if (numTone == 64) //HT20 FFT size
			{
				carrierNum = 28;
			}
			else if (numTone == 128) //HT40 FFT size
			{
				carrierNum = 58-2+1;
			}else
			{
				//it is not HT40 or HT20 mode, force error return
				printf("\n\n -- Num of tones, %d, is not an expected value, either 64 or 128!!! \n\n", numTone);
				err = -1; //return 0;
			}
			for(int i=0; i<numTone/2; i++)
			{
				doubleArrayTemp[i] = doubleArray[i+numTone/2+1];
				if (SCPI_INF >= doubleArrayTemp[i] && SCPI_NINF <= doubleArrayTemp[i] )
				{
					doubleArrayTemp[i] = pow(10, 0.05*checkDoubleValue(doubleArrayTemp[i])); //convert it back to linear amp but not power, thus 0.05.
				}
				//keep SCPI values
				//else
				//{
				//	doubleArrayTemp[i] = NA_NUMBER;
				//}
			}
			for(int i=numTone/2; i<numTone; i++)
			{
				doubleArrayTemp[i] = doubleArray[i-numTone/2+1];
				if (SCPI_INF >= doubleArrayTemp[i] && SCPI_NINF <= doubleArrayTemp[i] )
				{
					doubleArrayTemp[i] = pow(10, 0.05*checkDoubleValue(doubleArrayTemp[i])); //convert it back to linear amp but not power, thus 0.05.
				}
				//keep SCPI values
				//else
				//{
				//	doubleArrayTemp[i] = NA_NUMBER;
				//}
			}
			//use loleakagedbc to calculate buffeReal[0] for backward compatible, SFL is normalized to 0 dB of total power
			double doubleArray10[10] = {0.0};      // there are only 7 elements needed
			iqx.SendCommand("fetch:txq:ofdm?");
			splitAsDoubles(iqx.scpiRxBuffer, doubleArray10);
			if(doubleArray10[0]!= 0.0 )
			{
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				err = -1; // return 0;
			}
			doubleArrayTemp[0] = pow(10, 0.05* checkDoubleValue(doubleArray10[5]+10 * log10((double)(carrierNum*2))));

			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1; //

			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArrayTemp[i];
				bufferImag[i] = 0;
			}

		}
		err = 0; // return bufferLength;
	}

	else if (!strcmp(measurement, "rxRmsPowerDb"))
	{
		bufferLength =0;
		err = 0; // return bufferLength;
	}
	else if (!strcmp(measurement, "isolationDb"))

	{
		bufferLength =0;
		err = 0; // return bufferLength;
	}

	//need to interleave the streams result for MIMO according to Roman, nightmare ..., shall return the composite? FETC:OFDM:EVMS:COMP?
	else if (!strcmp(measurement, "evmSymbols"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:evms:str1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}

	//per stream will be a better approach, "evmSymbolsStr1", "evmSymbolsStr2" ... Str8.
	else if (!strcmp(measurement, "evmSymbolsStr1"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:evms:str1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}

	//need to talk to Roman on details for MIMO, might be same as evmSymbols,  shall return the composite? FETC:OFDM:EVMT:COMP?
	else if (!strcmp(measurement, "evmTones"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:evmt:str1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}

	//per stream will be a better approach, "evmTonesStr1", "evmTonesStr2" ... Str8.
	else if (!strcmp(measurement, "evmTonesStr1"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:evmt:str1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}

	//per signal/vsa
	//need to talk to Roman on details for MIMO, might be same as evmSymbols, shall return the composite? FETC:OFDM:EVMS:COMP?
	else if (!strcmp(measurement, "PhaseNoiseDeg_Symbols"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:pes:sign1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}

	//per signal will be a better approach, "PhaseNoiseDeg_SymbolsSign1", "PhaseNoiseDeg_SymbolsSign2" ... Sign8.
	else if (!strcmp(measurement, "PhaseNoiseDeg_SymbolsSign1"))
	{
		int tempNumResult = 0;
		rxLen = iqx.SendCommand("fetch:ofdm:pes:sign1?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			err = -1; // return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		err = 0; // return bufferLength;
	}

	else if (!strcmp(measurement, "demodSymbols"))
	{
		bufferLength =0;
		err = 0; // return bufferLength;
	}

	else
	{
		printf("\n\nERROR! Input keyword \"%s\" cannot be found in GetVectorMeasurement_80211b()\n\n", measurement);
		err = -1;
	}

	if (doubleArray)	free(doubleArray);
	if (intArray)		free(intArray);
	if (err == 0)
	{
		return bufferLength;  // normal return
	}
	else
	{
		return 0; // error happens
	}
	//return err;
}

// Jarir add BT vector measurement fetch, TODO: really need to improve and remove needto call so many differenty parsing functions, 12/9/11
// TODO: 1. use SCPI keywords 2. use set/get/check instrument state functions
int CIQmeasure_Scpi::GetVectorMeasurement_Bluetooth(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	int     err                 = 0;                            // Return error code
	int     rxLen               = 0;                            // scpi command return character length
	int     key                 = -1;                           // keyword index in the scpi return values
	int     fetchStatus         = -1;                           //
	char    *pch                = NULL;                         //
	double  doubleValue         = NA_NUMBER;                    //
	double  detected_data_rate  = NA_NUMBER;                    //

	rxLen = iqx.SendCommand("bt");
	rxLen = iqx.SendCommand("fetc:txq:ddr?");                   // Returns Detected Data Rate
	int eachFetchStatus[MAX_DATA_LENGTH] = {0};
	double eachDetectedDataRate[MAX_DATA_LENGTH] = {0.0};
	int numberOfPackets = splitAsDoubles2(iqx.scpiRxBuffer, eachDetectedDataRate, eachFetchStatus);

	if (strstr(measurement, "P_av_each_burst")!=0)
	{
		double doubleArray[1000] = {0.0};
		int statusCode[1000] = {0};
		iqx.SendCommand("fetc:pow?");
		int numValues = splitAsDoubles2(iqx.scpiRxBuffer, doubleArray, statusCode, 2*numberOfPackets);
		if (numValues > 0)
		{
			if (bufferLength > numValues)
			{
				bufferLength = numValues;
			}
			for(int i=0; i<bufferLength; i++)
			{
				if (statusCode[i] != 0.0)
				{
					printf("\n\n -- Fetch results ERROR!!! \n\n");
					return 0;
				}

				bufferReal[i] = checkDoubleValue(doubleArray[i]);
				if (!strcmp(measurement, "P_av_each_burst"))
				{
					bufferReal[i] = pow(10, 0.1*checkDoubleValue(doubleArray[i]));
				}
			}
			return (bufferLength);
		}
		else
			return 0;

	}
	else if (strstr(measurement, "P_pk_each_burst"))
	{
		double doubleArray[1000] = {0.0};
		int statusCode[1000] = {0};
		iqx.SendCommand("fetc:pow:peak?");
		int numValues = splitAsDoubles2(iqx.scpiRxBuffer, doubleArray, statusCode, 2*numberOfPackets);
		if (numValues > 0)
		{
			if (bufferLength > numValues)
			{
				bufferLength = numValues;
			}
			for(int i=0; i<bufferLength; i++)
			{
				if (statusCode[i] != 0.0)
				{
					printf("\n\n -- Fetch results ERROR!!! \n\n");
					return 0;
				}

				bufferReal[i] = checkDoubleValue(doubleArray[i]);
				if (!strcmp(measurement, "P_pk_each_burst"))
				{
					bufferReal[i] = pow(10, 0.1*checkDoubleValue(doubleArray[i]));
				}
			}
			return (bufferLength);
		}
		else
			return 0;

	}
	else if (!strcmp(measurement, "complete_burst"))
	{
		double doubleArray[1000] = {0};
		rxLen = iqx.SendCommand("fetc:sync:pcom?");

		int numValues = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if (numValues > 1)
			{
				if (bufferLength > (numValues-1))
				{
					bufferLength = numValues-1;
				}
				for(int i=0; i<bufferLength; i++)
				{
					bufferReal[i] = checkDoubleValue(doubleArray[i+1]);
				}
				return (bufferLength);
			}
			else
				return 0;
		}
	}
	else if (!strcmp(measurement, "start_sec"))
	{
		double doubleArray[1000] = {0};
		rxLen = iqx.SendCommand("fetc:sync:pst?");

		int numValues = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if (numValues > 1)
			{
				if (bufferLength > (numValues-1))
				{
					bufferLength = numValues-1;
				}
				for(int i=0; i<bufferLength; i++)
				{
					bufferReal[i] = checkDoubleValue(doubleArray[i+1]);
				}
				return (bufferLength);
			}
			else
				return 0;
		}
	}
	else if (!strcmp(measurement, "stop_sec"))
	{
		double doubleArray[1000] = {0};
		rxLen = iqx.SendCommand("fetc:sync:pet?");

		int numValues = splitAsDoubles(iqx.scpiRxBuffer, doubleArray);

		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if (numValues > 1)
			{
				if (bufferLength > (numValues-1))
				{
					bufferLength = numValues-1;
				}
				for(int i=0; i<bufferLength; i++)
				{
					bufferReal[i] = checkDoubleValue(doubleArray[i+1]);
				}
				return (bufferLength);
			}
			else
				return 0;
		}
	}
	else if ((!strcmp(measurement, "freq_est")) || (!strcmp(measurement,"freqEstPacketPointer")))
	{
		// for both EDR and BDR, not for LE? check iqapi.h
		int key = -1;
		int numContained = -1;
		double doubleArray[1000] = {0};
		int packetIndex[1000] = {0};
		int statusCode[1000] = {0};

		if ((eachDetectedDataRate[0] == 2.0) || (eachDetectedDataRate[0] == 3.0))
		{
			rxLen = iqx.SendCommand("fetc:txq:edr?");
			key = 8;
			numContained = 13;
		}
		else if (eachDetectedDataRate[0] == 1.0)
		{
			rxLen = iqx.SendCommand("fetc:txq:clas?");
			key = 8;
			numContained = 13;
		}
		else
		{
			return 0;
		}

		int numValues = splitAsDoubles4(iqx.scpiRxBuffer, doubleArray, packetIndex, statusCode, key, numContained);

		for (int packetIter = 0; packetIter<numValues; packetIter++)
		{
			if (statusCode[packetIter] != 0.0)
			{
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				return 0;
			}
			else
			{
				if (numValues > 0)
				{
					if (bufferLength > numValues)
						bufferLength = numValues;
					if (!strcmp(measurement,"freq_est"))
					{
						for (int i = 0; i<bufferLength; i++)
						{
							bufferReal[i] = checkDoubleValue(doubleArray[i]);
						}
					}
					else
					{
						for (int i = 0; i<bufferLength; i++)
						{
							bufferReal[i] = checkDoubleValue((double) packetIndex[i]);
						}
					}
					return bufferLength;
				}
				else
					return 0;
			}
		}

	}
	else if ((!strcmp(measurement, "freq_drift")) || (!strcmp(measurement, "freqDriftPacketPointer")) )
	{
		// only for BDR, not for LE/EDR? check iqapi.h
		int key = -1;
		int numContained = -1;
		double doubleArray[1000] = {0};
		int packetIndex[1000] = {0};
		int statusCode[1000] = {0};

		if (eachDetectedDataRate[0] == 1.0)
		{
			rxLen = iqx.SendCommand("fetc:txq:clas?");
			key = 3;
			numContained = 13;
		}
		else
		{
			return 0;
		}

		int numValues = splitAsDoubles4(iqx.scpiRxBuffer, doubleArray, packetIndex, statusCode, key, numContained);

		for (int packetIter = 0; packetIter<numValues; packetIter++)
		{
			if (statusCode[packetIter] != 0.0)
			{
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				return 0;
			}
			else
			{
				if (numValues > 0)
				{
					if (bufferLength > numValues)
						bufferLength = numValues;
					if (!strcmp(measurement,"freq_drift"))
					{
						for (int i = 0; i<bufferLength; i++)
						{
							bufferReal[i] = checkDoubleValue(doubleArray[i]);
						}
					}
					else
					{
						for (int i = 0; i<bufferLength; i++)
						{
							bufferReal[i] = checkDoubleValue((double) packetIndex[i]);
						}
					}
					return bufferLength;
				}
				else
					return 0;
			}
		}

	}
	else if ((!strcmp(measurement, "freq_deviation")) || (!strcmp(measurement, "freqDeviationPointer")))
	{
		// for both EDR and BDR, not for LE? check iqapi.h
		int key = -1;
		int numContained = -1;
		double doubleArray[1000] = {0};
		int packetIndex[1000] = {0};
		int statusCode[1000] = {0};

		if ((eachDetectedDataRate[0] == 2.0) || (eachDetectedDataRate[0] == 3.0))
		{
			rxLen = iqx.SendCommand("fetc:txq:edr?");
			key = 9;
			numContained = 13;
		}
		else if (eachDetectedDataRate[0] == 1.0)
		{
			rxLen = iqx.SendCommand("fetc:txq:clas?");
			key = 4;
			numContained = 13;
		}
		else
		{
			return 0;
		}

		int numValues = splitAsDoubles4(iqx.scpiRxBuffer, doubleArray, packetIndex, statusCode, key, numContained);

		for (int packetIter = 0; packetIter<numValues; packetIter++)
		{
			if (statusCode[packetIter] != 0.0)
			{
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				return 0;
			}
			else
			{
				if (numValues > 0)
				{
					if (bufferLength > numValues)
						bufferLength = numValues;
					if (!strcmp(measurement,"freq_deviation"))
					{
						for (int i = 0; i<bufferLength; i++)
						{
							bufferReal[i] = checkDoubleValue(doubleArray[i]);
						}
					}
					else
					{
						for (int i = 0; i<bufferLength; i++)
						{
							bufferReal[i] = checkDoubleValue((double) packetIndex[i]);
						}
					}
					return bufferLength;
				}
				else
					return 0;
			}
		}

	}
	else if (!strcmp(measurement, "EdrFreqvsTime"))
	{
		double doubleArray[1000] = {0}; // longest can be upto 16 segments, 17 elements return from fetc:txq:edr:evmt?
		iqx.SendCommand("fetc:txq:edr:ftim?");
		int numValues = splitAsDoubles3(iqx.scpiRxBuffer, doubleArray);
		if (doubleArray[0] != 0.0)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if (numValues > 0)
			{
				if (bufferLength > (numValues-1))
					bufferLength = numValues - 1;
				for (int i = 0; i<bufferLength; i++)
				{
					bufferReal[i] = checkDoubleValue(doubleArray[i+1]);
				}
				return bufferLength;
			}
			else
				return 0;

		}
		return 0;
	}
	else if (strstr(measurement, "EdrExtremeOmegaI0"))
	{
		// for both EDR and BDR, not for LE? check iqapi.h
		int key = -1;
		int numContained = -1;
		double doubleArray[1000] = {0};
		int packetIndex[1000] = {0};
		int statusCode[1000] = {0};

		if ((eachDetectedDataRate[0] == 2.0) || (eachDetectedDataRate[0] == 3.0))
		{
			rxLen = iqx.SendCommand("fetc:txq:edr?");
			key = 6;
			numContained = 13;
		}
		else
		{
			return 0;
		}

		int numValues = splitAsDoubles4(iqx.scpiRxBuffer, doubleArray, packetIndex, statusCode, key, numContained);

		for (int packetIter = 0; packetIter<numValues; packetIter++)
		{
			if (statusCode[packetIter] != 0.0)
			{
				printf("\n\n -- Fetch results ERROR!!! \n\n");
				return 0;
			}
			else
			{
				if (numValues > 0)
				{
					if (bufferLength > numValues)
						bufferLength = numValues;
					for (int i = 0; i<bufferLength; i++)
					{
						bufferReal[i] = checkDoubleValue(doubleArray[i]);
					}
					return bufferLength;
				}
				else
					return 0;
			}
		}

	}
	else
	{
	}

	printf("\n\nERROR! Input keyword \"%s\" cannot be found in GetVectorMeasurement_Bluetooth()\n\n", measurement);
	return err;
}

int CIQmeasure_Scpi::GetVectorMeasurement_FFT(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	int err = 0;
	int rxLen   = 0;
	int numResult = 0;
	static double doubleArray[MAX_DATA_LENGTH] = {0.00};

	if (!strcmp(measurement, "x"))  // float array
	{
		rxLen = iqx.SendCommand("fetch:spec:ofr?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		return bufferLength;
	}

	if (!strcmp(measurement, "y"))  // float array
	{
		rxLen = iqx.SendCommand("fetch:spec:aver?");
		numResult = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, bufferLength+1);
		if(doubleArray[0]!= 0.00)
		{
			printf("\n\n -- Fetch results ERROR!!! \n\n");
			return 0;
		}
		else
		{
			if ((numResult-1)<bufferLength)
				bufferLength=numResult-1;
			for(int i=0; i<bufferLength; i++)
			{
				bufferReal[i] = doubleArray[i+1];
			}
		}
		return bufferLength;
	}
	printf("\n\nERROR! Input keyword \"%s\" cannot be found in GetVectorMeasurement_FFT()\n\n", measurement);
	return err;
}

// Jarir added BT scalar measurements, TODO: improve more as much as possible, 12/9/11

double CIQmeasure_Scpi::GetScalarMeasurement_Bluetooth(char *measurement, int index)
{
	// Updated by Jarir 12/15/11, use of set/get/check instrument state; use of SCPI keywords
	// Added by Jarir, 11/29/11
	int rxLen           = 0;
	double doubleValue = NA_NUMBER;
	char *pch = NULL;
	double detected_data_rate = NA_NUMBER;
	int key = -1;
	int status = -1;
	int parsedValues = -1;
	double multiplier = 1.0; // some measurements are returned as percents, change multipler to 0.01 then to convert, Jarir 12/15/11
	char scpiCommand[MAX_COMMAND_LEN] = {0};
	memset(scpiCommand,'\0',MAX_COMMAND_LEN);

	if (!strcmp(measurement, "valid") )
	{
		// Comment by Jarir 12/15/11 :- Difficult to map to all fetch status, many to one mapping, use fetch power for now
		//fetchGetScalars("BT_FETC_POW", index, &status, &parsedValues, 2, true);		// BT;FETC:POW? -- get average power for each packet
		iqx.GetScalars("BT_FETC_POW", index, &status, &parsedValues, 2, true);		// BT;FETC:POW? -- get average power for each packet

		if (0.0 == status)		// if fetch status okay, return ok
		{
			return 1;
		}
		else		// if fetch status not okay, return -1; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return -1;
		}
	}
	else if (!strcmp(measurement, "validInput") )
	{
		// TODO: is it possible somehow to check if valid input went for CALC SCPI command in AnalyzeBluetooth functions;
		//		if everything went okay there, this should return 1
		//		otherwise return error
		// no validinput checking in Upper Layer, return 1 for now
		return 1;
	}
	else if (!strcmp(measurement, "acpErrValid") )
	{
		//fetchGetScalars("BT_FETC_ACP", index, &status, &parsedValues, 12, true);   // BT;FETC:ACP?  -- get ACP results
		iqx.GetScalars("BT_FETC_ACP", index, &status, &parsedValues, 12, true);   // BT;FETC:ACP?  -- get ACP results
		if (0.0 == status)		// if fetch status okay, return ok
		{
			return 1;
		}
		else		// if fetch status not okay, return -1; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return -1;
		}
	}
	else if (!strcmp(measurement, "leValid") )
	{
		//fetchGetScalars("BT_FETC_TXQ_LEN", index, &status, &parsedValues, 12, true); // BT;FETC:TXQ:LEN? -- get LE tx quality
		iqx.GetScalars("BT_FETC_TXQ_LEN", index, &status, &parsedValues, 12, true); // BT;FETC:TXQ:LEN? -- get LE tx quality
		if (0.0 == status)		// if fetch status okay, return ok
		{
			return 1;
		}
		else		// if fetch status not okay, return -1; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return -1;
		}
	}
	else if (!strcmp(measurement, "EdrEVMvalid") )
	{
		//fetchGetScalars("BT_FETC_TXQ_EDR", index, &status, &parsedValues, 13, true); // BT;FETC:TXQ:EDR? -- get EDR tx quality
		iqx.GetScalars("BT_FETC_TXQ_EDR", index, &status, &parsedValues, 13, true); // BT;FETC:TXQ:EDR? -- get EDR tx quality
		if (0.0 == status)		// if fetch status okay, return ok
		{
			return 1;
		}
		else		// if fetch status not okay, return -1; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return -1;
		}
	}

	if ( g_btAnalysisDataRate == 0) // for auto detecte data rate mode, detect data rate first and set it globally (to be reset by new analysis) , only for first packet
	{
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars("BT_FETC_TXQ_DDR", 0, &status, &parsedValues, 2, true);  // BT;FETC:TXQ:DDR?  -- get detected data rate
		retVal = iqx.GetScalars("BT_FETC_TXQ_DDR", 0, &status, &parsedValues, 2, true);  // BT;FETC:TXQ:DDR?  -- get detected data rate
		if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else	// else set global variable, because each time new measurement is requested, no need to set analysis data rate again and again
		{
			detected_data_rate = checkDoubleValue(retVal);
			g_btAnalysisDataRate = detected_data_rate;	// set global variable for analysis data rate indicator
		}
	}
	else
	{
		// else use user-requested analysis data rate
	}

	if (strstr(measurement, "dataRateDetect") )
	{
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars("BT_FETC_TXQ_DDR", 0, &status, &parsedValues, 2, true);  // BT;FETC:TXQ:DDR?  -- Get detected data rate
		retVal = iqx.GetScalars("BT_FETC_TXQ_DDR", 0, &status, &parsedValues, 2, true);  // BT;FETC:TXQ:DDR?  -- Get detected data rate
		if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else	// else return value if right index
		{
			if (index==0)		// analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
			{
				return checkDoubleValue(retVal);
			}
			else			// if index > 0 is requested, then give warning; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index = 0 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (strstr(measurement, "bandwidth20dB"))
	{
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars("BT_FETC_SPEC_20BW", 0, &status, &parsedValues, 2,true);   // BT;FETC:SPEC:20BW? -- get 20dB Bandwidth
		retVal = iqx.GetScalars("BT_FETC_SPEC_20BW", 0, &status, &parsedValues, 2,true);   // BT;FETC:SPEC:20BW? -- get 20dB Bandwidth
		if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else	// else return value if right index
		{
			if (index==0)		// analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
			{
				return checkDoubleValue(retVal);
			}
			else			// if index > 0 is requested, then give warning; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index = 0 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (strstr(measurement, "P_av_each_burst") )
	{
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars("BT_FETC_POW", 0, &status, &parsedValues, 2, true);   // BT;FETC:POW? -- get average power of each packet
		retVal = iqx.GetScalars("BT_FETC_POW", 0, &status, &parsedValues, 2, true);   // BT;FETC:POW? -- get average power of each packet
		if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else	// else return value if right index
		{
			if (index==0)		// analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
			{
				if (!strcmp(measurement, "P_av_each_burst"))
				{
					return pow(10, 0.1*checkDoubleValue(retVal));		// convert to linear
				}
				return checkDoubleValue(retVal);
			}
			else			// if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index = 0 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (strstr(measurement, "P_pk_each_burst") )
	{
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars("BT_FETC_POW_PEAK", 0, &status, &parsedValues, 2, true);   // BT;FETC:POW:PEAK? -- get peak power of each packet
		retVal = iqx.GetScalars("BT_FETC_POW_PEAK", 0, &status, &parsedValues, 2, true);   // BT;FETC:POW:PEAK? -- get peak power of each packet
		if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else	// else return value if right index
		{
			if (index==0)		// analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
			{
				if (!strcmp(measurement, "P_pk_each_burst"))
				{
					return pow(10, 0.1*checkDoubleValue(retVal));
				}
				return checkDoubleValue(retVal);
			}
			else			// if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index = 0 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (strstr(measurement, "P_av_no_gap_all") )
	{
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars("BT_FETC_POW_AVER", 0, &status, &parsedValues, 2, true);	// BT;FETC:POW:AVER? -- get average of all packet's average powers
		retVal = iqx.GetScalars("BT_FETC_POW_AVER", 0, &status, &parsedValues, 2, true);	// BT;FETC:POW:AVER? -- get average of all packet's average powers
		if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else
		{
			if (index==0)	// analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
			{
				if (!strcmp(measurement, "P_av_no_gap_all"))
				{
					return pow(10, 0.1*checkDoubleValue(retVal));	// convert to linear
				}
				return checkDoubleValue(retVal);
			}
			else			// if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index = 0 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (strstr(measurement, "P_peak_all") )
	{
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars("BT_FETC_POW_PEAK_MAX", 0, &status, &parsedValues, 2, true);	// BT;FETC:POW:PEAK:MAX? -- get max of all packet's peak powers
		retVal = iqx.GetScalars("BT_FETC_POW_PEAK_MAX", 0, &status, &parsedValues, 2, true);	// BT;FETC:POW:PEAK:MAX? -- get max of all packet's peak powers
		if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else
		{
			if (index==0)	// analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
			{
				if (!strcmp(measurement, "P_peak_all"))
				{
					return pow(10, 0.1*checkDoubleValue(retVal));  // covnert to linear
				}
				return checkDoubleValue(retVal);
			}
			else			// if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index = 0 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (strstr(measurement, "P_av_all") )
	{
		// not supported by SCPI
		// SCPI does not return average over whole capture including gap, return NA_NUMBER
		return doubleValue;
	}

	else if (strstr(measurement, "meanNoGapPowerCenterDbm"))
	{
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars("BT_FETC_ACP_CNGP", 0, &status, &parsedValues, 2, true);	// BT;FETC:ACP:CNGP?  -- get mean no gap power at center offset for first packet only
		retVal = iqx.GetScalars("BT_FETC_ACP_CNGP", 0, &status, &parsedValues, 2, true);	// BT;FETC:ACP:CNGP?  -- get mean no gap power at center offset for first packet only
		if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else
		{
			if (index==0)	// analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
			{
				return checkDoubleValue(retVal);
			}
			else		// if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index = 0 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (strstr(measurement, "sequenceDefinition"))
	{
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars("BT_FETC_ACP_COFF", index, &status, &parsedValues, 12, true);	// BT;FETC:ACP:COFF? -- get frequency offset index for ACP calculation
		retVal = iqx.GetScalars("BT_FETC_ACP_COFF", index, &status, &parsedValues, 12, true);	// BT;FETC:ACP:COFF? -- get frequency offset index for ACP calculation
		if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else
		{
			if (index>=0 && index<=10)	// analysis was done for first packet only! so, must use index between 0 to 10 -- 11 values returned in SCPI for first packet
			{
				return checkDoubleValue(retVal);
			}
			else		// if different index is requested, then return NA_NUMBER; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index>= 0 && index<=10 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (!strcmp(measurement, "leMaxPowerDbm") || !strcmp(measurement, "maxPowerAcpDbm") || !strcmp(measurement, "maxPowerEdrDbm") )
	{
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars("BT_FETC_ACP", index, &status, &parsedValues, 12, true);	// BT;FETC:ACP? -- get ACP values
		retVal = iqx.GetScalars("BT_FETC_ACP", index, &status, &parsedValues, 12, true);	// BT;FETC:ACP? -- get ACP values
		if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else
		{
			if (index>=0 && index<=10)	// analysis was done for first packet only! so, must use index between 0 to 10 -- 11 values returned in SCPI for first packet
			{
				return checkDoubleValue(retVal);
			}
			else		// if different index is requested, then return NA_NUMBER; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index>= 0 && index<=10 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}

	switch ((int) g_btAnalysisDataRate) // switch on configured or auto-detected data rate for analysis
	{
		case 0:		// if auto-detected data rate was 0 (unknown), then return NA_NUMBER; TODO: remove printf's; throw error
			printf("\n\n -- Data Rate Unknown ERROR!!! \n\n");
			printf("Configured Data Rate for fetch: %f\n", detected_data_rate);
			return checkDoubleValue(doubleValue);
			break;
		case 1:		// if auto-detected or configured data rate is 1 (BDR)
			if (-1 == key)	// use key, because the fetch command returns an array containing all following measurements;
			// TODO: use a map to store which measurement associated with which key index
			{
				if (!strcmp(measurement,"freq_est"))
				{
					key = 1;
				}
				else if (!strcmp(measurement,"freq_estHeader"))
				{
					key = 2;
				}
				else if (!strcmp(measurement,"freq_drift")) // only valid for 1010 pattern
				{
					key = 3;
				}
				else if (!strcmp(measurement,"freq_deviation"))
				{
					key = 4;
				}
				else if (!strcmp(measurement,"freq_deviationpktopk"))
				{
					key = 5;
				}
				else if (!strcmp(measurement,"deltaF2AvAccess"))
				{
					key = 6;
				}
				else if (!strcmp(measurement,"deltaF2MaxAccess"))
				{
					key = 7;
				}
				else if (!strcmp(measurement,"deltaF1Average"))  // only valid for 11110000 pattern
				{
					key = 8;
				}
				else if (!strcmp(measurement,"deltaF2Average"))  // only valid for 10101010 pattern
				{
					key = 9;
				}
				else if (!strcmp(measurement,"deltaF2Max"))  // only valid for 10101010 pattern
				{
					key = 10;
				}
				else if (!strcmp(measurement,"maxfreqDriftRate"))  // only valid for 10101010 pattern
				{
					key = 11;
				}
				else if (!strcmp(measurement,"payloadErrors"))
				{
					key = 12;
				}
				else
				{
				}
				if (key > -1)
				{
					double retVal = NA_NUMBER;
					//retVal = fetchGetScalars("BT_FETC_TXQ_CLAS", key-1, &status, &parsedValues, 13, true);  // BT;FETC:TXQ:CLAS?  -- get Tx quality for BDR according to key
					retVal = iqx.GetScalars("BT_FETC_TXQ_CLAS", key-1, &status, &parsedValues, 13, true);  // BT;FETC:TXQ:CLAS?  -- get Tx quality for BDR according to key
					if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
					{
						printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
						printf("Returned Fetch Status Code: %f\n", status);
						return checkDoubleValue(doubleValue);
					}
					else
					{
						if (index==0)	// analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
						{
							return checkDoubleValue(retVal);
						}
						else	// if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
						{
							printf("Analysis must be done with index=0 for now, requested index = %d\n", index);
							return NA_NUMBER;
						}
					}
				}
			}


			break;
		case 2:		// case 2 and 3 are EDR data rates
		case 3:
			if (!strcmp(measurement, "EdrEVMvsTime"))
			{
				// TODO: what if requested index is greater than values returned? Do error handling!!
				double retVal = NA_NUMBER;
				//retVal = fetchGetScalars("BT_FETC_TXQ_EDR_EVMT", index, &status, &parsedValues, true);	// BT;FETC:TXQ:EDR:EVMT?  -- get EDR EVM vs. Time as an array
				retVal = iqx.GetScalars("BT_FETC_TXQ_EDR_EVMT", index, &status, &parsedValues, true);	// BT;FETC:TXQ:EDR:EVMT?  -- get EDR EVM vs. Time as an array
				if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
				{
					printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
					printf("Returned Fetch Status Code: %f\n", status);
					return checkDoubleValue(doubleValue);
				}
				else
				{
					return checkDoubleValue(retVal);
				}
			}

			if (-1 == key)	// use key, because the fetch command returns an array containing all following measurements;
			// TODO: use a map to store which measurement associated with which key index
			{
				if (!strcmp(measurement,"EdrEVMAv"))
				{
					key = 1;
					multiplier = 0.01;		// use this multiplier during return because SCPI EVM is percent, IQAPI returns ratio
				}
				else if (!strcmp(measurement,"EdrEVMpk"))
				{
					key = 2;
					multiplier = 0.01;
				}
				else if (!strcmp(measurement,"EdrprobEVM99pass"))
				{
					key = 3;
				}
				else if (!strcmp(measurement,"EdrOmegaI") || !strcmp(measurement,"freq_estHeader"))
					// Jarir added freq_estHeader here on 12/13/11, according iqapi.h: these two are same
				{
					key = 4;
				}
				else if (!strcmp(measurement,"EdrFreqExtremeEdronly"))
				{
					key = 5;
				}
				else if (!strcmp(measurement,"EdrExtremeOmegaI0"))
				{
					key = 6;
				}
				else if (!strcmp(measurement,"EdrPowDiffdB"))
				{
					key = 7;
				}
				else if (!strcmp(measurement,"EdrExtremeOmega0")) // same as EdrFreqExtremeEdronly according to Fei's doc
				{
					key = 5;
				}
				else if (!strcmp(measurement,"freq_est"))
				{
					key = 8;
				}
				else if (!strcmp(measurement,"freq_deviation"))
				{
					key = 9;
				}
				else if (!strcmp(measurement,"freq_deviationpktopk"))
				{
					key = 10;
				}
				else if (!strcmp(measurement,"deltaF2AvAccess"))
				{
					key = 11;
				}
				else if (!strcmp(measurement,"deltaF2MaxAccess"))
				{
					key = 12;
				}
				else
				{
				}
				if (key > -1)
				{
					double retVal = NA_NUMBER;
					//retVal = fetchGetScalars("BT_FETC_TXQ_EDR", key-1, &status, &parsedValues, 13, true);	// BT;FETC:TXQ:EDR? -- get EDR Tx quality
					retVal = iqx.GetScalars("BT_FETC_TXQ_EDR", key-1, &status, &parsedValues, 13, true);	// BT;FETC:TXQ:EDR? -- get EDR Tx quality
					if (0.0 != status)	// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
					{
						printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
						printf("Returned Fetch Status Code: %f\n", status);
						return checkDoubleValue(doubleValue);
					}
					else
					{
						if (index==0)	// analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
						{
							return (multiplier*checkDoubleValue(retVal));	 // returned multiplied by 0.01 -- since IQAPI returns ratio, SCPI returns percent
						}
						else	// if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
						{
							printf("Analysis must be done with index=0 for now, requested index = %d\n", index);
							return NA_NUMBER;
						}
					}
				}
			}

			break;
		case 4:		// case 4 is Low Energy
			if (!strcmp(measurement, "leFn"))	// valid only for 10101010 pattern, otherwise returns fetch status -21 (ANALYSIS_FAILED)
			{
				double retVal = NA_NUMBER;
				//retVal = fetchGetScalars("BT_FETC_TXQ_LEN_ISOF", index, &status, &parsedValues, true);	// BT;FETC:TXQ:LEN:ISOF?  -- get integral sum of frequencies as an array
				retVal = iqx.GetScalars("BT_FETC_TXQ_LEN_ISOF", index, &status, &parsedValues, true);	// BT;FETC:TXQ:LEN:ISOF?  -- get integral sum of frequencies as an array
				// TODO: what if requested index is greater than values returned? Do error handling!!
				if (0.0 != status)	// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
				{
					printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
					printf("Returned Fetch Status Code: %f\n", status);
					return checkDoubleValue(doubleValue);
				}
				else
				{
					return checkDoubleValue(retVal);
				}
			}
			else if (!strcmp(measurement, "lePduLength"))
			{
				int intArray[2] = {0};
				// TODO: careful with following!!! what if rx buffer gets filled up, and next receive will begin from where this left off!!
				//int setStatus = setInstrumentState("SYS_FORM_DATA", "PACK", true);	// SYS;FORM:READ:DATA PACK -- set tester output format to binary block
				int setStatus = iqx.SendCommand("SYS_FORM_DATA", "PACK", true);	// SYS;FORM:READ:DATA PACK -- set tester output format to binary block

				iqx.SendCommand("BT_FETC_TXQ_LEN_PAYL","?", true);					// BT;FETC:TXQ:LEN:PAYL?   -- get the payload as a binary block

				splitAsIntegers2(iqx.scpiRxBuffer, intArray, 2);				// split only first two values, first value is status code,
				//		second value contains payload length in bits
				//		special split function takes care of extracting payload length in bytes

				if (intArray[0] != 0)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
				{
					printf("\n\n -- Fetch results ERROR!!! \n\n");
					return doubleValue;
				}

				// TODO: improve following; must set to format which was before the above PACK format; don't just set to ASC
				//setStatus = setInstrumentState("SYS_FORM_DATA","ASC", true);     // SYS;FORM:READ:DATA ASC -- set tester outut format back to ASCII
				setStatus = iqx.SendCommand("SYS_FORM_DATA","ASC", true);     // SYS;FORM:READ:DATA ASC -- set tester outut format back to ASCII
				doubleValue = (double) (intArray[1]);
				return doubleValue;

			}
			else if (!strcmp(measurement, "lePreambleSeq"))
			{
				double retVal = NA_NUMBER;
				//retVal = fetchGetScalars("BT_FETC_TXQ_LEN_PRE", index, &status, &parsedValues, 9, true);		// BT;FETC:TXQ:LEN:PRE? --- get LE preamble sequence as array
				retVal = iqx.GetScalars("BT_FETC_TXQ_LEN_PRE", index, &status, &parsedValues, 9, true);		// BT;FETC:TXQ:LEN:PRE? --- get LE preamble sequence as array
				// TODO: what if requested index is greater than values returned (8 bits + 1 status)? Do error handling!!
				if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
				{
					printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
					printf("Returned Fetch Status Code: %f\n", status);
					return checkDoubleValue(doubleValue);
				}
				else
				{
					return checkDoubleValue(retVal);
				}
			}
			else if (!strcmp(measurement, "leSyncWordSeq"))
			{
				double retVal = NA_NUMBER;
				//retVal = fetchGetScalars("BT_FETC_TXQ_LEN_SWOR", index, &status, &parsedValues, 33, true); 	// BT;FETC:TXQ:LEN:SWOR? --- get LE sync word sequence as array
				retVal = iqx.GetScalars("BT_FETC_TXQ_LEN_SWOR", index, &status, &parsedValues, 33, true); 	// BT;FETC:TXQ:LEN:SWOR? --- get LE sync word sequence as array
				// TODO: what if requested index is greater than values returned (32 bits + 1 status)? Do error handling!!
				if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
				{
					printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
					printf("Returned Fetch Status Code: %f\n", status);
					return checkDoubleValue(doubleValue);
				}
				else
				{
					return checkDoubleValue(retVal);
				}
			}
			else if (!strcmp(measurement, "lePduHeaderSeq"))
			{
				double retVal = NA_NUMBER;
				//retVal = fetchGetScalars("BT_FETC_TXQ_LEN_PDUH", index, &status, &parsedValues, 9, true); 	// BT;FETC:TXQ:LEN:PDUH? --- get PDU header sequence as array
				retVal = iqx.GetScalars("BT_FETC_TXQ_LEN_PDUH", index, &status, &parsedValues, 9, true); 	// BT;FETC:TXQ:LEN:PDUH? --- get PDU header sequence as array
				// TODO: what if requested index is greater than values returned (8 bits + 1 status)? Do error handling!!
				if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
				{
					printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
					printf("Returned Fetch Status Code: %f\n", status);
					return checkDoubleValue(doubleValue);
				}
				else
				{
					return checkDoubleValue(retVal);
				}
			}
			else if (!strcmp(measurement, "lePduLengthSeq"))
			{
				double retVal = NA_NUMBER;
				//retVal = fetchGetScalars("BT_FETC_TXQ_LEN_PDUL", index, &status, &parsedValues, 9, true); 	// BT;FETC:TXQ:LEN:PDUL? --- get PDU length sequence as array
				retVal = iqx.GetScalars("BT_FETC_TXQ_LEN_PDUL", index, &status, &parsedValues, 9, true); 	// BT;FETC:TXQ:LEN:PDUL? --- get PDU length sequence as array
				// TODO: what if requested index is greater than values returned (8 bits + 1 status)? Do error handling!!
				if (0.0 != status)  	// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
				{
					printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
					printf("Returned Fetch Status Code: %f\n", status);
					return checkDoubleValue(doubleValue);
				}
				else
				{
					return checkDoubleValue(retVal);
				}
			}
			else if (!strcmp(measurement, "lePayloadSeq")) // checked
			{

				int intArray[2] = {0};
				// TODO: careful with following!!! what if rx buffer gets filled up, and next receive will begin from where this left off!!
				//int setStatus = setInstrumentState("SYS_FORM_DATA", "PACK", true);	// SYS;FORM:READ:DATA PACK -- set tester output format to binary block
				int setStatus = iqx.SendCommand("SYS_FORM_DATA", "PACK", true);	// SYS;FORM:READ:DATA PACK -- set tester output format to binary block
				iqx.SendCommand("BT_FETC_TXQ_LEN_PAYL", "?", true);					// BT;FETC:TXQ:LEN:PAYL?   -- get the payload as a binary block

				splitAsIntegers2(iqx.scpiRxBuffer, intArray, 2);				// split only first two values, first value is status code,
				//		second value contains payload length in bits
				//		special split function takes care of extracting payload length in bytes

				if (intArray[0] != 0)
				{
					printf("\n\n -- Fetch results ERROR!!! \n\n");
					return doubleValue;
				}
				int numToRead = intArray[1];		// get how many bits to read


				double doubleArray[300] = {0}; // LE payload is max 296 bits, 297 elements returned from fetc:txq:len:payl? TODO: improve memory allocation
				// TODO: careful with following!!! what if rx buffer gets filled up, and next receive will begin from where this left off!!
				iqx.SendCommand("BT_FETC_TXQ_LEN_PAYL", "?", true);

				// TODO: improve following; must set to format which was before the above PACK format; don't just set to ASC
				//setStatus = setInstrumentState("SYS_FORM_DATA", "ASC", true);	     // SYS;FORM:READ:DATA ASC -- set tester outut format back to ASCII
				setStatus = iqx.SendCommand("SYS_FORM_DATA", "ASC", true);	     // SYS;FORM:READ:DATA ASC -- set tester outut format back to ASCII
				// TODO: what if packet is shorter or more than one packet in capture?? cannot use 0 to detect packet separation; improve parsing function
				int numValues = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, numToRead+1);

				if (doubleArray[0] != 0.0)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
				{
					printf("\n\n -- Fetch results ERROR!!! \n\n");
					return doubleValue;
				}
				if (numValues > (index+1) )		// if requested index within number of bits returned, then return that value, otherwise return NA_NUMBER
				{
					doubleValue = checkDoubleValue(doubleArray[index + 1]);
				}
				return doubleValue;

			}
			else if (!strcmp(measurement, "leCrcSeq"))
			{
				double retVal = NA_NUMBER;
				//retVal = fetchGetScalars("BT_FETC_TXQ_LEN_CRC", index, &status, &parsedValues, 25, true);	// BT;FETC:TXQ:LEN:CRC? -- get LE CRC field bits as an array
				retVal = iqx.GetScalars("BT_FETC_TXQ_LEN_CRC", index, &status, &parsedValues, 25, true);	// BT;FETC:TXQ:LEN:CRC? -- get LE CRC field bits as an array
				// TODO: what if requested index is greater than values returned (24 bits + 1 status)? Do error handling!!
				if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
				{
					printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
					printf("Returned Fetch Status Code: %f\n", status);
					return checkDoubleValue(doubleValue);
				}
				else
				{
					return checkDoubleValue(retVal);
				}
			}
			else if (!strcmp(measurement, "leDeltaF2Max")) // only valid for 10101010
			{
				double dF2temp = -NA_NUMBER;
				int temp = -1;

				double retVal = NA_NUMBER;
				//retVal = fetchGetScalars("BT_FETC_TXQ_LEN_DF2M", 0, &status, &parsedValues, 9999, true);	// BT;FETC:TXQ:LEN:DF2M? -- get all LE delta_F2_max values as array
				retVal = iqx.GetScalars("BT_FETC_TXQ_LEN_DF2M", 0, &status, &parsedValues, 9999, true);	// BT;FETC:TXQ:LEN:DF2M? -- get all LE delta_F2_max values as array
				if (0.0 != status)		// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
				{
					printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
					printf("Returned Fetch Status Code: %f\n", status);
					return checkDoubleValue(doubleValue);
				}
				else // search for the minimum of all max, according to standard
				{
					dF2temp = retVal;
					for (int valIter = 1; valIter<(parsedValues-1); valIter++)
					{
						//retVal = fetchGetScalars("BT_FETC_TXQ_LEN_DF2M",valIter, &status, &temp, 9999, true);	// BT;FETC:TXQ:LEN:DF2M? -- get each LE delta_F2_max value from fetch history table and compare
						retVal = iqx.GetScalars("BT_FETC_TXQ_LEN_DF2M",valIter, &status, &temp, 9999, true);	// BT;FETC:TXQ:LEN:DF2M? -- get each LE delta_F2_max value from fetch history table and compare
						if (dF2temp >  checkDoubleValue(retVal))
						{
							dF2temp = checkDoubleValue(retVal);	// this will eventually contain the minimum
						}
					}
					return dF2temp;
				}
			}
			if (-1 == key)	// use key, because the fetch command returns an array containing all following measurements;
			// TODO: use a map to store which measurement associated with which key index
			{
				if (!strcmp(measurement,"leFreqOffset"))
				{
					key = 1;
				}
				else if (!strcmp(measurement,"leDeltaF1Avg")) // only valid for 00001111
				{
					key = 2;
				}
				else if (!strcmp(measurement,"leDeltaF2Avg")) // only valid for 10101010
				{
					key = 3;
				}
				else if (!strcmp(measurement,"leDeltaF2MaxMinValue")) // only valid for 10101010
				{
					key = 4;
				}
				else if (!strcmp(measurement,"leFreqDevSyncAv"))
				{
					key = 5;
				}
				else if (!strcmp(measurement,"leFnMax")) // only valid for 10101010
				{
					key = 6;
				}
				else if (!strcmp(measurement,"leDeltaF0FnMax")) // only valid for 10101010
				{
					key = 7;
				}
				else if (!strcmp(measurement,"leDeltaF1F0")) // only valid for 10101010
				{
					key = 8;
				}
				else if (!strcmp(measurement,"leDeltaFnFn_5Max")) // only valid for 10101010
				{
					key = 9;
				}
				else if (!strcmp(measurement,"leBelow185F2Max")) // only valid for 10101010?
				{
					key = 10;
				}
				else if (!strcmp(measurement,"leIsCrcOk"))
				{
					key = 11;
				}
				else
				{
				}
				if (key > -1)
				{
					double retVal = NA_NUMBER;
					//retVal = fetchGetScalars("BT_FETC_TXQ_LEN", key-1, &status, &parsedValues, 12, true);	// BT;FETC:TXQ:LEN? -- get LE Tx quality as array
					retVal = iqx.GetScalars("BT_FETC_TXQ_LEN", key-1, &status, &parsedValues, 12, true);	// BT;FETC:TXQ:LEN? -- get LE Tx quality as array
					if (0.0 != status)	// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
					{
						printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
						printf("Returned Fetch Status Code: %f\n", status);
						return checkDoubleValue(doubleValue);
					}
					else
					{
						if (index==0)	// analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
						{
							return checkDoubleValue(retVal);
						}
						else	// if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
						{
							printf("Analysis must be done with index=0 for now, requested index = %d\n", index);
							return NA_NUMBER;
						}
					}
				}
			}


			break;
	}


	return doubleValue;
}


// Jarir added following function to get power results, which are generic in IQapi, but for SCPI, have to specify what technology module to use, Engineering needs to give a way to do this in a generic way, a global variable is being used to indicate technology, 12/9/11

double CIQmeasure_Scpi::GetScalarMeasurement_Power(char *measurement, int index)
{
	// JK, 01/10/12, removed set/get/check instrument state;
	// Updated by Jarir 12/15/11, use of set/get/check instrument state; use of SCPI keywords
	// "P_av_all" // including gap not supported with SCPI
	// "P_av_all_dBm"  // including gap not supported with SCPI

	int     rxLen           = 0;
	double  doubleValue     = NA_NUMBER;
	char    *pch            = NULL;
	int     status          = -1;
	int     parsedValues    = -1;
	char    scpiCommand[MAX_COMMAND_LEN] ={0};

	if (!strcmp(measurement, "valid") )
	{
		// Comment by Jarir 12/15/11 :- Difficult to map to all fetch status, many to one mapping, use fetch power for now
		memset(scpiCommand, '\0', MAX_COMMAND_LEN);
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%s_FETC_POW", g_moduleForPowerAnalysis);	// use globally configured module for Power analysis, [WIFI/BT];FETC:POW? -- get average power for each packet

		//fetchGetScalars(scpiCommand, index, &status, &parsedValues, 2, true);
		iqx.GetScalars(scpiCommand, index, &status, &parsedValues, 2, true);
		if (0 == status)
			return 1;                                                                       // if fetch status okay, return ok
		else
		{
			printf("\n\n -- Fetch ERROR in GetScalarMeasurement_Power()!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return -1;                                                                      // if fetch status not okay, return -1; TODO: remove printf's; improve?
		}
	}
	if (strstr(measurement, "P_av_each_burst") )
	{
		memset(scpiCommand, '\0', MAX_COMMAND_LEN);
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%s_FETC_POW", g_moduleForPowerAnalysis);	// use globally configured module for Power analysis, [WIFI/BT];FETC:POW? -- get average power for each packet
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars(scpiCommand, 0, &status, &parsedValues, 2, true);
		retVal = iqx.GetScalars(scpiCommand, 0, &status, &parsedValues, 2, true);
		if (0 != status)		                                                            // if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else
		{
			if (index==0)			                                                        // analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
			{
				if (!strcmp(measurement, "P_av_each_burst"))
				{
					return pow(10, 0.1*checkDoubleValue(retVal));                           // convert to linear
				}
				return checkDoubleValue(retVal);
			}
			else			                                                                // if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index = 0 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (strstr(measurement, "P_pk_each_burst") )
	{
		memset(scpiCommand, '\0', MAX_COMMAND_LEN);
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%s_FETC_POW_PEAK", g_moduleForPowerAnalysis);	// use globally configured module for Power analysis, [WIFI/BT];FETC:POW:PEAK? -- get peak of power for each packet
		double retVal = NA_NUMBER;
		//retVal= fetchGetScalars(scpiCommand, 0, &status, &parsedValues, 2, true);
		retVal= iqx.GetScalars(scpiCommand, 0, &status, &parsedValues, 2, true);
		if (0 != status)		                                                            // if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else
		{
			if (index==0)			                                                        // analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
			{
				if (!strcmp(measurement, "P_pk_each_burst"))
				{
					return pow(10, 0.1*checkDoubleValue(retVal));                           // convert to linear
				}
				return checkDoubleValue(retVal);
			}
			else			                                                                // if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index = 0 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (strstr(measurement, "P_av_no_gap_all") )
	{
		memset(scpiCommand, '\0', MAX_COMMAND_LEN);
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%s_FETC_POW_AVER", g_moduleForPowerAnalysis);	// use globally configured module for Power analysis, [WIFI/BT];FETC:POW:AVER? -- get average of power for each packet
		double retVal = NA_NUMBER;
		//retVal = fetchGetScalars(scpiCommand, 0, &status, &parsedValues, 2, true);
		retVal = iqx.GetScalars(scpiCommand, 0, &status, &parsedValues, 2, true);
		if (0 != status) 	                                                                // if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else
		{
			if (index==0)			                                                        // analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
			{
				if (!strcmp(measurement, "P_av_no_gap_all"))
				{
					return pow(10, 0.1*checkDoubleValue(retVal));                           // convert to linear
				}
				return checkDoubleValue(retVal);
			}
			else			                                                                // if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index = 0 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (strstr(measurement, "P_peak_all") )
	{
		memset(scpiCommand, '\0', MAX_COMMAND_LEN);
		sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%s_FETC_POW_PEAK_MAX", g_moduleForPowerAnalysis);  // use globally configured module for Power analysis, [WIFI/BT];FETC:POW:PEAK:MAX? -- get max of peak power for each packet
		double retVal = NA_NUMBER;
		retVal = iqx.GetScalars(scpiCommand, 0, &status, &parsedValues, 2, true);
		if (0 != status) 	                                                                // if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
		{
			printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
			printf("Returned Fetch Status Code: %f\n", status);
			return checkDoubleValue(doubleValue);
		}
		else
		{
			if (index==0)			                                                        // analysis was done for first packet only! so, must use index 0 (check analyzebluetooth function)
			{
				if (!strcmp(measurement, "P_peak_all"))
				{
					return pow(10, 0.1*checkDoubleValue(retVal));                           // convert to linear
				}
				return checkDoubleValue(retVal);
			}
			else			                                                                // if index > 0 is requested, then return NA_NUMBER; TODO: remove printf; throw error
			{
				printf("Analysis must be done with index = 0 for now, requested index = %d\n", index);
				return NA_NUMBER;
			}
		}
	}
	else if (strstr(measurement, "P_av_all") )
	{
		// not supported by SCPI
		return doubleValue;                                                                 // SCPI does not return average over whole capture including gap, return NA_NUMBER
	}
	else
	{
		// do nothing
	}

	return doubleValue;
}

double CIQmeasure_Scpi::GetScalarMeasurement_CW(char *measurement, int index)
{
	// Updated by Jarir 12/15/11, use of set/get/check instrument state; use of SCPI keywords
	// 12/12/11, Jarir Updated based on Response from Roman on 12/7/11
	// Added by Jarir, 12/05/11
	double doubleValue = NA_NUMBER;
	int rxLen = 0;

	rxLen = iqx.SendCommand("GPRF_FETC_PHAS_FERR","?", true);	// GRPF;FETC:PHAS:FERR? -- return frequency error over whole capture (see analyzeCW function)

	int fetchStatus[1] = {0};
	double frequency[1] = {0.0};
	int numberValues = splitAsDoubles2(iqx.scpiRxBuffer, frequency, fetchStatus, 2);
	if (fetchStatus[0] !=0)	// if fetch status not okay, return NA_NUMBER; TODO: remove printf's; improve?
	{
		printf("\n\n -- Fetch data rate results ERROR!!! \n\n");
		return checkDoubleValue(doubleValue);
	}

	return frequency[0];

}


// Jarir added following function to save VSA data capture to local machine, 12/9/11, TODO: improve LP_SCPI_Bin2File()
int CIQmeasure_Scpi::SaveVsaSignalFile(char *sigFileName)
{   // save the current capture to tester as a temparary file, and then get it and save it to local
	// The temparary file name is hard-coded to save memory, the sigFileName

	// JK, 01/10/12
	// Added by Jarir, 12/05/11
	// strip only filename from sigFileName and use with remoteFileName
	// use whole sigFileName as localFileName
	int     err		= ERR_OK;
	char    scpiCommand[MAX_COMMAND_LEN] = {0};


	int isPass = iqx.SaveVsaCaptureToLocal(sigFileName);   // save vsa capture to a local file

	if(!isPass)
		err = ERR_SAVE_WAVE_FAILED;

	//memset(scpiCommand,'\0',MAX_COMMAND_LEN);
	//LP_SCPI_Bin2File(sigFileName);			// TODO: strip .sig extension and clean up filename to remove characters unsupported by IQxstream

	//rxLen = iqx.SendCommand("SYS_WAI", "", true);		// SYS;*WAI -- wait for previous operation to complete
	//rxLen = iqx.SendCommand("SYS_ERR","?",true);		// SYST:ERR:ALL? -- check error queue

	//// get status code and return error if fail
	//char * pch = NULL;
	//pch = strtok(iqx.scpiRxBuffer, ",");
	//if (NULL == pch)
	//{
	//	Sleep(CONNECTION_TIMEOUT); //wait CONNECTION_TIMEOUT and try it again.
	//	rxLen = iqx.SendCommand("SYS_WAI", "", true);   // SYS;*WAI -- wait for previous operations
	//	rxLen = iqx.SendCommand("SYS_ERR","?",true);	// SYST:ERR:ALL? -- check all accumulated errors
	//	pch = strtok(iqx.scpiRxBuffer, ",");
	//}

	//if (NULL == pch)
	//{
	//	err = ERR_NO_RESPONSE_FROM_TESTER;
	//}
	//else if (atoi(pch)!=0)
	//{
	//	err = ERR_SAVE_WAVE_FAILED;
	//	return err;
	//}

	return err;

}

// Jarir add BT vector measurement fetch, TODO: really need to improve and remove need to call so many different parsing functions, 12/9/11
// TODO: 1. use SCPI keywords 2. use set/get/check instrument state functions
// Jarir added following function to get BT string measurement, 12/9/11, TODO: improve parsing
int CIQmeasure_Scpi::GetStringMeasurement_Bluetooth(char *measurement, char bufferChar[], int bufferLength)
{
	int err = 0;
	int rxLen = 0;
	rxLen = iqx.SendCommand("bt");

	if (!strcmp(measurement, "analysisType"))
	{
		if (strlen(analysisTypeBt)!=0)
		{
			int numberOfCharToCopy = 0;
			int tempLen = (int) strlen(analysisTypeBt);
			if ( bufferLength > tempLen )
				numberOfCharToCopy = tempLen;
			else
				numberOfCharToCopy = bufferLength-1;
			strncpy_s(bufferChar, bufferLength, analysisTypeBt, numberOfCharToCopy);
			return(numberOfCharToCopy);
		}
		else
		{
			return 0;
		}
	}
	else if (!strcmp(measurement, "versionString"))
	{
		rxLen = iqx.SendCommand("midn?");
		int numberOfCharToCopy = 0;
		int tempLen = (int) strlen(iqx.scpiRxBuffer);
		if ( bufferLength > tempLen )
			numberOfCharToCopy = tempLen;
		else
			numberOfCharToCopy = bufferLength-1;
		strncpy_s(bufferChar, bufferLength, iqx.scpiRxBuffer, numberOfCharToCopy);
		return(numberOfCharToCopy);

	}
	else if (!strcmp(measurement, "acpErrMsg"))
	{
		// TODO: Don't know what SCPI returns and how; not being used by IQfact+

		// How do we get acpErrMsg from fetch results
		//rxLen = iqx.SendCommand("midn?");
		//int numberOfCharToCopy = 0;
		//int tempLen = (int) strlen(iqx.scpiRxBuffer);
		//if ( bufferLength > tempLen )
		//	numberOfCharToCopy = tempLen;
		//else
		//	numberOfCharToCopy = bufferLength-1;
		//strncpy_s(bufferChar, bufferLength, iqx.scpiRxBuffer, numberOfCharToCopy);
		//return(numberOfCharToCopy);

	}
	else if (!strcmp(measurement, "leMsg"))
	{
		// TODO: Don't know what SCPI returns and how; not being used by IQfact+

		// How do we get leMsg from fetch results
		//rxLen = iqx.SendCommand("midn?");
		//int numberOfCharToCopy = 0;
		//int tempLen = (int) strlen(iqx.scpiRxBuffer);
		//if ( bufferLength > tempLen )
		//	numberOfCharToCopy = tempLen;
		//else
		//	numberOfCharToCopy = bufferLength-1;
		//strncpy_s(bufferChar, bufferLength, iqx.scpiRxBuffer, numberOfCharToCopy);
		//return(numberOfCharToCopy);

	}
	else
	{
	}
	return err;

}

//int CIQmeasure_Scpi::InitInstrumentDrvFunctions(void)
//{
//#ifdef INSTRUMENT_DRIVER_EXPLICIT_LINK
//    g_insDrvDllHndl = NULL;
//    g_insDrvDllHndl = LoadLibrary((LPCTSTR)("LP_DLL.dll"));
//
//    if (g_insDrvDllHndl)
//    {
//        LP_DLL_Open_Session = (int (__cdecl *)(int*, int, char*, char*, int))GetProcAddress(g_insDrvDllHndl, "LP_DLL_Open_Session");
//        LP_DLL_Error_Check_Set = (int (__cdecl *)(int, int))GetProcAddress(g_insDrvDllHndl, "LP_DLL_Error_Check_Set");
//        LP_DLL_SCPI_Set_Storage = (int (__cdecl *)(int))GetProcAddress(g_insDrvDllHndl, "LP_DLL_SCPI_Set_Storage");
//        LP_DLL_Driver_Set_Storage = (int (__cdecl *)(int))GetProcAddress(g_insDrvDllHndl, "LP_DLL_Driver_Set_Storage");
//        LP_DLL_Rout_Port_Resource_Set = (int (__cdecl *)(int, int, int, unsigned int))GetProcAddress(g_insDrvDllHndl, "LP_DLL_Rout_Port_Resource_Set");
//
//        LP_DLL_Sys_Reset = (int (__cdecl *)(int))GetProcAddress(g_insDrvDllHndl, "LP_DLL_Sys_Reset");
//        LP_DLL_Sys_Wait = (int (__cdecl *)(int))GetProcAddress(g_insDrvDllHndl, "LP_DLL_Sys_Wait");
//        LP_DLL_Sys_Data_Format_Set = (int (__cdecl *)(int, int))GetProcAddress(g_insDrvDllHndl, "LP_DLL_Sys_Data_Format_Set");
//        LP_DLL_Sys_Capture_Segment_Data = (int (__cdecl *)(int, unsigned int, unsigned int, float, unsigned int*))GetProcAddress(g_insDrvDllHndl, "LP_DLL_Sys_Capture_Segment_Data");
//
//        LP_DLL_VSA_Initiate = (int (__cdecl *)(int, int))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Initiate");
//        LP_DLL_VSA_Sampling_Rate_Set = (int (__cdecl *)(int, int, unsigned int))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Sampling_Rate_Set");
//        LP_DLL_VSA_Sampling_Rate_Get = (int (__cdecl *)(int, int, unsigned int*))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Sampling_Rate_Get");
//        LP_DLL_VSA_Frequency_Set = (int (__cdecl *)(int, int, double))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Frequency_Set");
//        LP_DLL_VSA_Frequency_Get = (int (__cdecl *)(int, int, double*))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Frequency_Get");
//        LP_DLL_VSA_PowerLevel_Set = (int (__cdecl *)(int, int, double))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_PowerLevel_Set");
//        LP_DLL_VSA_PowerLevel_Get = (int (__cdecl *)(int, int, double*))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_PowerLevel_Get");
//        LP_DLL_VSA_Trigger_Source_Set = (int (__cdecl *)(int, int, int))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Trigger_Source_Set");
//        LP_DLL_VSA_Trigger_Source_Get = (int (__cdecl *)(int, int, int*))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Trigger_Source_Get");
//        LP_DLL_VSA_Trigger_Level_Set = (int (__cdecl *)(int, int, double))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Trigger_Level_Set");
//        LP_DLL_VSA_Trigger_Level_Get = (int (__cdecl *)(int, int, double*))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Trigger_Level_Get");
//        LP_DLL_VSA_Capture_Time_Set = (int (__cdecl *)(int, int, double))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Capture_Time_Set");
//        LP_DLL_VSA_Capture_Time_Get = (int (__cdecl *)(int, int, double*))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Capture_Time_Get");
//        LP_DLL_VSA_Pretrigger_Time_Set = (int (__cdecl *)(int, int, double))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Pretrigger_Time_Set");
//        LP_DLL_VSA_Pretrigger_Time_Get = (int (__cdecl *)(int, int, double*))GetProcAddress(g_insDrvDllHndl, "LP_DLL_VSA_Pretrigger_Time_Get");
//    }
//
//    return 0;
//#else
//    return 0;
//#endif
//}

IQMEASURE_API int LpGetSysError (char *errMsg)
{
	int err = 0;
	unsigned int actualSize = 0;

	//LP_DLL_Sys_Error_Get_All(g_session, MAX_BUFFER_SIZE, errMsg, &actualSize);

	return err;
}

//IQMEASURE_API int LpDllGetVersion (char *manufacturer, char* modelNumber, char* serialNumber, char *firmwareRevision)
IQMEASURE_API int LpDllGetVersion (char *verInfo)
{
	int err = 0;
	//    char response[MAX_PATH] = {0};
	//    unsigned int actualSize = 0;
	//    char manufacturer[MAX_BUFFER_LEN];
	//    char modelNumber[MAX_BUFFER_LEN];
	//    char serialNumber[MAX_BUFFER_LEN];
	//    char firmwareRevision[MAX_BUFFER_LEN];
	//    char ver[MAX_BUFFER_LEN],  specifiedVer[MAX_BUFFER_LEN];
	//
	//    err = LP_DLL_Identify_System_Query (g_session, MAX_PATH, MAX_PATH, response, &actualSize, manufacturer, modelNumber, serialNumber, firmwareRevision);
	//    strcpy_s(ver, MAX_BUFFER_LEN, "");
	//    sprintf_s(specifiedVer, "\nAC Model Number:\t%s", modelNumber);
	//    strcat_s(verInfo, MAX_BUFFER_LEN, specifiedVer);
	//    strcpy_s(ver, MAX_BUFFER_LEN, "");
	//    sprintf_s(specifiedVer, "\nAC Serial Number:\t%s", serialNumber);
	//    strcat_s(verInfo, MAX_BUFFER_LEN, specifiedVer);
	//    strcpy_s(ver, MAX_BUFFER_LEN, "");
	//    sprintf_s(specifiedVer, "\nAC Firmware Revision:\t%s", firmwareRevision);
	//    strcat_s(verInfo, MAX_BUFFER_LEN, specifiedVer);
	//    strcpy_s(ver, MAX_BUFFER_LEN, "");
	//
	////     err = LP_DLL_VSA_Identify_Query(g_session, MAX_PATH, MAX_PATH, response, &actualSize, manufacturer, modelNumber, serialNumber, firmwareRevision);
	////     strcpy_s(ver, MAX_BUFFER_LEN, "");
	////     sprintf_s(specifiedVer, "\nAC Model Number:\t%s", modelNumber);
	////     strcat_s(verInfo, MAX_BUFFER_LEN, specifiedVer);
	////     strcpy_s(ver, MAX_BUFFER_LEN, "");
	////     sprintf_s(specifiedVer, "\nAC Serial Number:\t%s", serialNumber);
	////     strcat_s(verInfo, MAX_BUFFER_LEN, specifiedVer);
	////     strcpy_s(ver, MAX_BUFFER_LEN, "");
	////     sprintf_s(specifiedVer, "\nAC Firmware Revision:\t%s", firmwareRevision);
	////     strcat_s(verInfo, MAX_BUFFER_LEN, specifiedVer);
	////     strcpy_s(ver, MAX_BUFFER_LEN, "");
	////
	////     err = LP_DLL_VSG_Identify_Query (g_session, MAX_PATH, MAX_PATH, response, &actualSize, manufacturer, modelNumber, serialNumber, firmwareRevision);
	////     strcpy_s(ver, MAX_BUFFER_LEN, "");
	////     sprintf_s(specifiedVer, "\nAC Model Number:\t%s", modelNumber);
	////     strcat_s(verInfo, MAX_BUFFER_LEN, specifiedVer);
	////     strcpy_s(ver, MAX_BUFFER_LEN, "");
	////     sprintf_s(specifiedVer, "\nAC Serial Number:\t%s", serialNumber);
	////     strcat_s(verInfo, MAX_BUFFER_LEN, specifiedVer);
	////     strcpy_s(ver, MAX_BUFFER_LEN, "");
	////     sprintf_s(specifiedVer, "\nAC Firmware Revision:\t%s", firmwareRevision);
	////     strcat_s(verInfo, MAX_BUFFER_LEN, specifiedVer);
	////     strcpy_s(ver, MAX_BUFFER_LEN, "");

	return err;
}

IQMEASURE_API int LpDllGetVsaVersion (char *manufacturer, char* modelNumber, char* serialNumber, char *firmwareRevision)
{
	int err = 0;
	//char response[MAX_PATH] = {0};
	//unsigned int actualSize = 0;

	//err = LP_DLL_Identify_System_Query (g_session, MAX_PATH, MAX_PATH, response, &actualSize, manufacturer, modelNumber, serialNumber, firmwareRevision);
	return err;
}

IQMEASURE_API int LpDllGetVsgVersion (char *manufacturer, char* modelNumber, char* serialNumber, char *firmwareRevision)
{
	int err = 0;
	//char response[MAX_PATH] = {0};
	//unsigned int actualSize = 0;

	//err = LP_DLL_Identify_System_Query (g_session, MAX_PATH, MAX_PATH, response, &actualSize, manufacturer, modelNumber, serialNumber, firmwareRevision);
	return err;
}

char* toLowerCase(char* str) // modified 11/22/11
{
	int differ = 'A'-'a';
	char ch;
	int ii = (int)strlen(str);
	for (int i=0; i <ii;i++)
	{
		strncpy(&ch,str+i,1);
		if (ch>='A' && ch<='Z')
		{
			ch = ch - differ;
			memcpy(str+i,&ch,1);
		}
	}
	return str;
}
string& tLowerCase(string& str)
{
	int differ  = 'A'-'a';
	for (int i=0; i< (int) str.size() ;i++)
	{
		if (str[i] >= 'A' && str[i] <= 'Z')
			str[i] = str[i] - differ;
	}
	return str;
}

//char* toUpperCase(char* str) // modified 12/27/11
//{
//	int differ = 'A'-'a';
//	char ch;
//	int ii = (int)strlen(str);
//	for (int i=0; i <ii;i++)
//	{
//		strncpy(&ch,str+i,1);
//		if (ch>='a' && ch<='z')
//		{
//			ch = ch+differ;
//			memcpy(str+i,&ch,1);
//		}
//	}
//	return str;
//}
//string tUpperCase(string str)
//{
//	for (int i=0;i<strlen(str.c_str());i++)
//		if (str[i] >= 0x61 && str[i] <= 0x7A)
//			str[i] = str[i] - 0x20;
//	return str;
//}


int splitAsDoubles(char* stringToSplit, double *doubleArray, int dataNumExpected, char * delimiters)
{
	//split an input string, interpret each segment as a double, and return all double values to doubleArray[]

	char *XP = NULL;
	int num  = 0;       // how many sub_string obtained
	XP = strtok(stringToSplit, delimiters);
	while (XP != NULL)
	{
		//printf ("\n [--> debug: %s <--]\n",XP); // debug only
		doubleArray[num] = atof(XP);
		XP = strtok (NULL, ",");
		num++;
		if ((dataNumExpected>0) && (num == dataNumExpected)) break;
	}
	return num;
}

// TODO: remove if possible
// Jarir added following function for parsing doubles, being used by 3 BT vector and CW scalar measurement, remove when these functions are improved? TODO, 12/9/11
int splitAsDoubles2(char* stringToSplit, double *doubleArray, int *intArray, int dataNumExpected, char * delimiters)
{
	//split an input string, interprate each segment as a double, and return all double values to doubleArray[]
	// Added by Jarir, to get scalar measurements for multiple packets
	// each measurement is separated by status code
	// both status code for each measurement and the measurement itself are returned
	char *XP = NULL;
	int num  = 0;       // how many sub_string obtained
	XP = strtok(stringToSplit, delimiters);
	int num1 = 0;
	int num2 = 0;
	while (XP != NULL)
	{
		//printf ("\n [--> debug: %s <--]\n",XP); // debug only
		if ((num%2)==0)
		{
			intArray[num1] = atoi(XP);
			num1++;
		}
		if ((num%2)==1)
		{
			doubleArray[num2] = atof(XP);
			num2++;
		}
		XP = strtok (NULL, ",");
		num++;
		if (dataNumExpected>0 && num == dataNumExpected) break;
	}
	if (num1 == num2)
	{
		return num1;
	}
	else
	{
		return -1;
	}

}

// TODO: remove if possible
// Jarir added following function to parse fetched values, being used by one BT getvector measurement, remove when getvector function gets improved? TODO, 12/9/11
int splitAsDoubles3(char* stringToSplit, double *doubleArray, int dataNumExpected, char * delimiters)
{
	// NEED TO IMRPOVE THIS FUNCTION
	// this function splits values for one packet from multiple packets captured
	// uses status code 0 to break parsing
	// split an input string, interprate each segment as a double, and return all double values to doubleArray[]
	// Added by Jarir, function for getting vector measurement for first packet only
	// parsing stops when a fetch status of 0 is obtained for the second packet
	// improve later
	char *XP = NULL;
	int num  = 0;       // how many sub_string obtained
	XP = strtok(stringToSplit, delimiters);
	while (XP != NULL)
	{
		//printf ("\n [--> debug: %s <--]\n",XP); // debug only
		if ((num>0) && (atof(XP)==0.0))
			break;
		doubleArray[num] = atof(XP);
		XP = strtok (NULL, ",");
		num++;
		if (dataNumExpected>0 && num == dataNumExpected) break;
	}
	return num;
}


// TODO: remove if possible
// Jarir added following function, used for parsing doubles, remove when BT getvector function gets improved? TODO 9/12/11
int splitAsDoubles4(char* stringToSplit, double *doubleArray, int *intArray, int *statusCode, int key, int numContained, int dataNumExpected, char * delimiters)
{
	char *XP = NULL;
	int num = 0;
	XP = strtok(stringToSplit, delimiters);
	int count1 = 0;
	int count = 0;
	while (XP!=NULL)
	{
		if (count1 == 0)
		{
			statusCode[count] = atoi(XP);
			count++;
		}
		if (count1 == key)
		{
			doubleArray[count-1] = atof(XP);
			intArray[count-1] = count-1;
		}
		XP = strtok (NULL, ",");
		count1++;
		if (count1 == numContained)
		{
			count1 = 0;
		}
		if (dataNumExpected>0 && count == dataNumExpected) break;

	}
	return count;
}

int splitAsIntegers(char* stringToSplit, int *intArray, int dataNumExpected,char * delimiters)
{
	//split an input string, interprate each segment as an integer, and return all values to intArray[]

	char *XP = NULL;
	int num  = 0;       // how many sub_string obtained
	XP = strtok(stringToSplit, delimiters);
	while (XP != NULL)
	{
		//printf ("\n [--> debug: %s <--]\n",XP); // debug only
		intArray[num] = atoi(XP);
		XP = strtok (NULL, ",");
		num++;
		if (dataNumExpected>0 && num == dataNumExpected) break;
	}
	return num;
}

// TODO: remove if possible
// Jarir Add following function to get bitstream , used by two BT scalar measurements, remove after scalar parsing improved? TODO, 12/9/11
int splitAsIntegers2(char* stringToSplit, int *intArray, int dataNumExpected,char * delimiters)
{
	//split an input string, interprate each segment as an integer, and return all values to intArray[]

	char *XP = NULL;
	int num  = 0;       // how many sub_string obtained
	XP = strtok(stringToSplit, delimiters);
	while (XP != NULL)
	{
		//printf ("\n [--> debug: %s <--]\n",XP); // debug only
		if ((num%2)==0)
			intArray[num] = atoi(XP);
		else
		{
			char numVal[2] = {'\0'};
			strncpy(numVal, XP+1, 1);
			int numValues = atoi(numVal);
			char pch[10] = {'\0'};
			strncpy(pch, XP+2, numValues);
			intArray[num] = (int) (atoi(pch)/(BIT_NUM/8));  // 32-bit reading
		}
		XP = strtok (NULL, ",");
		num++;
		if (dataNumExpected>0 && num == dataNumExpected) break;
	}
	return num;
}

// Jarir using this function to check returned values from SCPI, SCPI returns a very large number when a measurement is not valid
double checkDoubleValue(double inVal)
{
	if (inVal > 1e37)
		return NA_NUMBER;
	else
		return inVal;
}

int CreateMemInTester(char *referenceFile, int iFileType, char ** ppcCmd)
{
	int err = 0;

	ifstream ifsSignalFile;
	ifsSignalFile.open (referenceFile, ios::binary);
	if ( ifsSignalFile.is_open() == false )
	{
		//sprintf_s(lastErr, MAX_LEN_ERR_TXT, "Error: Fail to open the file '%s'\n", referenceFile);
		//Terence
		err = ERR_NO_MOD_FILE_LOADED;
		return err;
	}
	// get length of file:
	ifsSignalFile.seekg (0, ios::end);
	int length = ifsSignalFile.tellg();
	ifsSignalFile.seekg (0, ios::beg);

	// allocate memory:
	char *buffer = new char [length];

	// read data as a block:
	ifsSignalFile.read (buffer,length);
	ifsSignalFile.close();

	if( iFileType == IQREF )
		sprintf_s(*ppcCmd, MAX_BUFFER_SIZE, "SYS;MMEM:DATA \"%s\",#", "Reference.iqref");
	else if( iFileType == IQVSA )
		sprintf_s(*ppcCmd, MAX_BUFFER_SIZE, "SYS;MMEM:DATA \"%s\",#", "Capture.iqvsa");
	else if( iFileType == IQVSG )
		sprintf_s(*ppcCmd, MAX_BUFFER_SIZE, "SYS;MMEM:DATA \"%s\",#", "Signal.iqvsg");


	char cDigit[100];
	sprintf_s(cDigit, "%d", length);
	sprintf_s(cDigit, "%d", strlen(cDigit));
	strcat(*ppcCmd, cDigit);

	sprintf_s(cDigit, "%d", length);
	strcat(*ppcCmd, cDigit);

	iqx.m_iCmdDataSize = strlen(*ppcCmd) + length;
	memcpy(*ppcCmd+ strlen(*ppcCmd), buffer, length);


	return 0;
}

//// Jarir 12/15/11 -- this function is used to store fetch output in a local map; so that a fetch command does not need to be sent again if the values are already there
//// The Fetch History map gets reset during every new analysis
//// Jarir add following function to reduce Fetching from tester, 12/9/11
//double fetchGetScalars(char *measurement, int index, int *status, int *numValues, int numExpected, bool keyUsed)
//{
//	int rxLen = 0;
//	string currentMeasurement = measurement;
//	double returnValue = NA_NUMBER;
//	map<string, vector<double>>::iterator it1;
//	it1 = g_bt_fetchHistory.find(currentMeasurement);
//	if ((it1!=g_bt_fetchHistory.end()))
//	{
//		//printf("[Jarir Debug] ------------> Fetch command found in history\n");
//		int fetchLen = (int) it1->second.size();
//		if (fetchLen > (index+1))
//		{
//			*status = (int) it1->second.at(0);
//			*numValues = fetchLen;
//			returnValue = it1->second.at(index+1);
//		}
//		else
//		{
//			//printf("[Jarir Debug] ------------> Upto %d results available, requested index = %d\n", fetchLen, index);
//			*numValues = fetchLen;
//			*status = -1;
//		}
//	}
//	else
//	{
//		vector<double> doubleVector;
//		if (keyUsed)
//			rxLen = iqx.SendCommand(measurement, "?", keyUsed);
//		else
//			rxLen = iqx.SendCommand(measurement);
//		double doubleArray[MAX_DATA_LENGTH] = {0.0};
//		int parsedValues = splitAsDoubles(iqx.scpiRxBuffer, doubleArray, numExpected);
//		doubleVector.assign(doubleArray, doubleArray+parsedValues);
//		g_bt_fetchHistory.insert(pair<string, vector<double>>(measurement, doubleVector));
//		*status = (int) doubleArray[0];
//		*numValues = parsedValues;
//		if (parsedValues > (index+1))
//			returnValue = doubleArray[index+1];
//		else
//			returnValue = NA_NUMBER;
//	}
//	return returnValue;
//
//}


//// Jarir 12/15/11 -- Jarir add this function to check if current instrument state is same as requested by matching with local table
//// if not same, then update the global instrument state map after issuing successful SCPI command
//// if same, then do no do anything
//// only use with SCPI key for now; TODO: use explicit SCPI commands
//int setInstrumentState(char *scpiCommand, char *parameters, bool scpiKeyUsed)
//{
//	int rxLen = 0;
//	bool stateChange = false;
//	int status = -1;
//	char *pch = NULL;
//
//	char currentParams[MAX_COMMAND_LEN] = {'\0'};
//	status = getInstrumentState(scpiCommand, currentParams, true);	// get current state stored locally
//
//	string currentParamString = currentParams;
//	string targetParamString = parameters;
//
//	string::size_type currentLastPos = currentParamString.find_first_not_of(", ", 0);	// get current parameters as string with comma separation
//	string::size_type targetLastPos  = targetParamString.find_first_not_of(", ", 0);	// get requested parameters as string with comma separation
//
//	string::size_type currentPos = currentParamString.find_first_of(", ", currentLastPos);
//	string::size_type targetPos  = targetParamString.find_first_of(", ", targetLastPos);
//
//	// TODO: instead of doing the loop here; can we used the checkInstrumentState function?
//	// parse and check
//	do
//	{
//		if ( string::npos == currentLastPos ) // if the length of each parameter array are not same, then state must change
//		{
//			stateChange = true;
//			break;
//		}
//		if ( string::npos == targetLastPos ) // if the length of each parameter array are not same, then state must change
//		{
//			stateChange = true;
//			break;
//		}
//
//		string currentSubStr = currentParamString.substr(currentLastPos, currentPos - currentLastPos);
//		string targetSubStr  = targetParamString.substr(targetLastPos , targetPos  - targetLastPos );
//
//		if ( currentSubStr.compare(targetSubStr) !=0 )	// if each parameter array are not identical, then state may change
//		{
//			if ( atof(currentSubStr.c_str()) && atof( targetSubStr.c_str() ) )	// if values are double, then convert to double first
//			{
//				double currVal = atof(currentSubStr.c_str());
//				double targVal = atof( targetSubStr.c_str());
//				if (currVal != targVal)		// if double values are not same, then state must change
//				{
//					stateChange = true;
//					break;
//				}
//			}
//			else
//			{
//				stateChange = true;
//				break;
//			}
//		}
//
//		currentLastPos = currentParamString.find_first_not_of(", ", currentPos);
//		targetLastPos  = targetParamString.find_first_not_of(", ", currentPos);
//
//		currentPos = currentParamString.find_first_of(", ", currentLastPos);
//		targetPos  = targetParamString.find_first_of(", ", targetLastPos);
//
//	} while ( (string::npos != currentLastPos) || (string::npos != targetLastPos) );
//
//
//	if (stateChange == true)
//	{
//		string currentCommand = scpiCommand;
//		rxLen = iqx.SendCommand(scpiCommand, parameters, true);	// since new settings must take effect, send the actual SCPI command
//
//		if (strstr(scpiCommand,"ROUT_PORT_RES"))	// ROUT;PORT:RES? <RF#A> -- is special because the question mark is in the middle, not at the end
//		{
//			currentCommand += "_Q";		// ROUT;PORT:RES? <RF#A>		-- queries which module RF#A port is connected to -- VSA/VSG/OFF?
//			rxLen = iqx.SendCommand((char*)currentCommand.c_str(), "", true);	// send command
//		}
//		else
//		{
//			rxLen = iqx.SendCommand((char*)currentCommand.c_str(), "?", true); // any other SCPI command is sent as query with question mark at end
//		}
//		pch  = NULL;
//		pch = strtok(iqx.scpiRxBuffer,"\n");
//		if(!pch) return 9999;
//		string currentParams = pch;
//		// update instrument state map
//		map<string,string>::iterator it;
//		it = g_instrumentState.find(currentCommand);
//		g_instrumentState.erase(it);			// erase current state
//		g_instrumentState.insert(pair<string,string>(currentCommand, currentParams));		// update local table with new state obtained from instrument, NOT with requested state
//
//		// for debug only here: added by Jacky 01/03/2012
//		if(TRUE == iqx.GetLogTesterState() )
//		{   iqx.SetColor(LIGHT_GREEN);
//		printf("g_instrumentState.size() = %d\n", g_instrumentState.size());
//		printf("\n>>[%s] = %s\n",currentCommand.c_str(),currentParams.c_str());
//		iqx.SetColor(WHITE);
//		}
//		// check if settings were effective and return relevant error
//		status = checkInstrumentState(scpiCommand, parameters, true); // this is done to check if local table matches with parameters requested
//		if (status == 0)
//			return 1; // return 1 to let caller know new settings were sent
//		else
//		{
//			//printf("***********************************************************\n");
//			//printf("**** WARNING:     DESIRED VALUE COULD NOT BE SET!!!! ******\n");
//			//printf("***********************************************************\n");
//			return 9999;	// if somehow checkInstrumentState returns a different string than requested, return very large number to trigger an error check
//		}
//	}
//	else
//	{
//		return 0; // return 0 to let caller know no settings changed
//	}
//	return -1;	// this must never happen, all returns happen above, TODO: if this happens, this will screw up error checking
//
//}
//
//// Jarir 12/15/11 -- Jarir add this function to check if current instrument state is same as requested
//// return 0 if same, return -1 if not same, i.e. local table and requested state don't match
//// only use with SCPI key for now; TODO: use explicit SCPI commands
//int checkInstrumentState(char *scpiCommand, char *parameters, bool scpiKeyUsed)
//{
//	int rxLen = 0;
//	bool checkPassed = true;
//	int status = -1;
//
//	char *pch = NULL;
//
//	char currentParams[MAX_COMMAND_LEN] = {'\0'};
//	status = getInstrumentState(scpiCommand, currentParams, true);
//
//	string currentParamString = currentParams;	// get current state from local map as comma-separated string
//	string targetParamString = parameters;		// get requested state as comma-separated string
//
//	string::size_type currentLastPos = currentParamString.find_first_not_of(", ", 0);
//	string::size_type targetLastPos  = targetParamString.find_first_not_of(", ", 0);
//
//	string::size_type currentPos = currentParamString.find_first_of(",", currentLastPos);
//	string::size_type targetPos  = targetParamString.find_first_of(",", targetLastPos);
//
//	// parse and check
//	do
//	{
//		if ( string::npos == currentLastPos ) // if the length of each parameter array are not same, do not pass
//		{
//			checkPassed = false;
//			break;
//		}
//		if ( string::npos == targetLastPos ) // if the length of each parameter array are not same, do not pass
//		{
//			checkPassed = false;
//			break;
//		}
//
//		string currentSubStr = currentParamString.substr(currentLastPos, currentPos - currentLastPos);
//		string targetSubStr  = targetParamString.substr(targetLastPos , targetPos  - targetLastPos );
//
//		if ( currentSubStr.compare(targetSubStr) !=0 ) // if the contents of each parameter array are not same, check if double values first
//		{
//			if ( atof(currentSubStr.c_str()) && atof( targetSubStr.c_str() ) )	// convert to double
//			{
//				double currVal = atof(currentSubStr.c_str());
//				double targVal = atof( targetSubStr.c_str());
//				if (currVal != targVal)							// double values don't match, do not pass
//				{
//					checkPassed = false;
//					break;
//				}
//			}
//			else
//			{
//				checkPassed = false;
//				break;
//			}
//		}
//		currentLastPos = currentParamString.find_first_not_of(", ", currentPos);
//		targetLastPos  = targetParamString.find_first_not_of(", ", currentPos);
//
//		currentPos = currentParamString.find_first_of(", ", currentLastPos);
//		targetPos  = targetParamString.find_first_of(", ", targetLastPos);
//
//
//	} while ( (string::npos != currentLastPos) || (string::npos != targetLastPos) );
//
//	if (checkPassed)
//		return 0;
//	else
//		return -1;
//
//	return -1;
//
//}
//
//// Jarir 12/15/11 -- Jarir add this function to get instrument state from local map, if it exists
//// if instrument state is not in local map, query instrument and push into local map
//// only use with SCPI key for now; TODO: use explicit SCPI commands
//int getInstrumentState(char *scpiCommand, char *parameters, bool scpiKeyUsed) // only use with SCPI key for now
//{
//	int rxLen = 0;
//	char * pch = NULL;
//
//	string currentCommand = scpiCommand;
//	string currentParams = "";
//
//	string queryString = "?";
//
//	map<string, string>::iterator it;
//
//	if (strstr(scpiCommand, "ROUT_PORT_RES"))
//	{
//		currentCommand += "_Q";
//		queryString = "";
//	}
//	else
//	{
//		// query string remains "?"
//	}
//	it = g_instrumentState.find(currentCommand);
//
//
//
//	if ( (it==g_instrumentState.end()) ) // instrument state doesn't exist in local map
//	{
//		// fetch instrument values and return in parameters, return ok
//		// TODO: if error, return not ok and reason code
//		rxLen = iqx.SendCommand((char*)currentCommand.c_str(), (char*)queryString.c_str(), true);
//		pch = strtok(iqx.scpiRxBuffer, "\n");
//		if(!pch) return -1;
//		currentParams = pch;
//		g_instrumentState.insert(pair<string, string>(currentCommand, currentParams));		// insert into local map
//
//		// for debug only here:
//		if(TRUE == iqx.GetLogTesterState() )
//		{   iqx.SetColor(LIGHT_GREEN);
//		printf("g_instrumentState.size() = %d\n", g_instrumentState.size());
//		printf("\n>>[%s] = %s\n",currentCommand.c_str(),currentParams.c_str());
//		iqx.SetColor(WHITE);
//		}
//		memcpy(parameters, pch, strlen(pch));
//		return 0;
//	}
//	else // instrument state exists in local map
//	{
//		// return from local map, return ok
//		memcpy(parameters, it->second.c_str(), strlen(it->second.c_str()) );
//		return 0;
//	}
//	return -1;		// TODO: remove this? this never happens if no change above (always return 0)
//}

// Updated by Jarir 12/15/11, use of set/get/check instrument state; use of SCPI keywords
// Jarir add following function to save VSA data capture to local machine, improve later, TODO, 12/9/11
//int LP_SCPI_Bin2File( char * filename)
//{
//	int rxLen = 0;
//	int dataLen = 0;
//	char * pch = NULL;
//	char temp[MAX_COMMAND_LEN] = {'\0'};
//	char strippedFileName[MAX_COMMAND_LEN] = {'\0'};	// strip away path name, IQxstream will only allows storing in 'Capture' folder
//	char localFileName[MAX_COMMAND_LEN] = {'\0'};		// to store local filename
//	char currentModule[MAX_COMMAND_LEN] = {'\0'};		// to store current module and change it back to what it was
//	char currentFormat[MAX_COMMAND_LEN] = {'\0'};		// to store current data output format and change it back to what it was
//
//	FILE *fp;
//	char *fp2;
//	char *firstChunk;
//	int read = 0;
//
//	memcpy(temp, filename, strlen(filename));
//	pch = strtok(temp, "/\\");
//	while (pch!=NULL)
//	{
//		memset(strippedFileName, '\0', MAX_COMMAND_LEN);
//		memcpy(strippedFileName, pch, strlen(pch));
//		pch = strtok(NULL, "/\\");
//	}
//
//	rxLen = iqx.SendCommand("MOD?");			// get current module and store for change back later
//	memcpy(currentModule, iqx.scpiRxBuffer, strlen(iqx.scpiRxBuffer));
//
//	rxLen = iqx.SendCommand("SYS;FORM:READ:DATA?"); // get current data output format and store for change back later
//	memcpy(currentFormat, iqx.scpiRxBuffer, strlen(iqx.scpiRxBuffer));
//
//	// TODO: any need to do 'Change Directory' on IQxstream?
//	//rxLen = iqx.SendCommand("SYS;MMEM:CDIR?");
//	//memcpy(currentPath, iqx.scpiRxBuffer, strlen(iqx.scpiRxBuffer));
//
//	char scpiCommand[MAX_COMMAND_LEN];
//	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
//	// TODO: change to SCPI keyword
//	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "SYS; CAPT:STOR \"Capture\\%s.iqvsa\"", strippedFileName);	// SYS;CAPT:STORE 'Capture\<strippedFileName>.iqvsa' -- save file into Capture folder on IQxstream
//	rxLen = iqx.SendCommand(scpiCommand);
//
//	// TODO: change to SCPI keyword
//	rxLen = iqx.SendCommand("*WAI");		// wait for last command to execute
//
//	// Jarir, 01/10/2012, Commenting out the following block which was used to get binary IQ data from tester using SCPI
//	// Because of some bug, syst:err:all? right after the following block does not return 0, "no error" ; but consequent SCPI input output are shifted by one, may cause problem for other test items
//	/*
//	// Comment start, Jarir 01/10/2012
//	sprintf_s(localFileName, MAX_COMMAND_LEN, "%s.iqvsa", filename);	// construct local filename
//
//	fp = fopen(localFileName, "wb+");
//	if (fp)
//	{
//		// TODO: change to SCPI keyword
//		rxLen = iqx.SendCommand("FORM:READ:DATA PACK");    //  change output data format to binary data
//
//		int sizeof_firstChunk = 20000;
//		firstChunk = (char*) malloc(sizeof_firstChunk);
//
//		// TODO: change to SCPI keyword
//		rxLen = iqx.SendCommand_only("VSA;CAPT:DATA?");		// get VSA captured data in binary format
//       // rxLen = iqx.SendCommand("VSA;CAPT:DATA?");		// get VSA captured data in binary format
//
//		int firstRead = TesterCommunication_RecieveBinary(&firstChunk,&sizeof_firstChunk,__FUNCTION__);		// receive binary stream on socket
//
//		LP_getArbArrayInfo(firstChunk, &fp2, &dataLen);	// get header info, containing total length in bytes of the stream and status code
//
//		// TODO: check status code, if not 0 , return error
//
//		int bytesLeft = (&firstChunk[firstRead] - fp2);	// decrement bytescounter
//		fwrite(fp2, 1, bytesLeft, fp);
//		int dataLeftToWrite = dataLen - bytesLeft;
//
//		read=0;
//		while(dataLeftToWrite > 0)		// continue reading
//		{
//
//			int toRead = sizeof(firstChunk);
//			if(toRead > dataLeftToWrite)
//				toRead = dataLeftToWrite+1; // The unfortunate API spec of TesterCommunication_RecieveBinary is that it requires one more byte then the data to fit a terminating 0
//
//
//			read = TesterCommunication_RecieveBinary(&firstChunk,&toRead,__FUNCTION__);
//
//			if(read > 0)	// write to local file
//				fwrite(firstChunk, 1, read, fp);
//			else {
//				break;
//			}
//
//			dataLeftToWrite-=read;
//		}
//		fclose(fp);
//	}
//
//
//	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
//	// TODO: use SCPI keyword
//	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "SYS;FORM:READ:DATA %s", currentFormat);		// change back output data format to original
//	iqx.SendCommand(scpiCommand);
//
//	// TODO: need to change back directory on IQxstream?
//	//memset(scpiCommand, '\0', MAX_COMMAND_LEN);
//	//sprintf_s(scpiCommand, MAX_COMMAND_LEN, "SYS;MMEM:CDIR %s", currentPath);
//	//iqx.SendCommand(scpiCommand);
//
//
//	memset(scpiCommand, '\0', MAX_COMMAND_LEN);
//	sprintf_s(scpiCommand, MAX_COMMAND_LEN, "%s", currentModule);		// change back to previous module
//	iqx.SendCommand(scpiCommand);
//
//	free(firstChunk);
//
//	if(read < 0)
//		return read;
//	*/
//	// Comment End, Jarir 01/10/2012
//	return 0;
//}

//// Jarir add following function for VSA data capture saving to local machine, maybe useful for other functions too, 12/9/11
//int LP_getArbArrayInfo( char * data, char ** ppData, int * pnData)
//{
//	// Start with setting default out-data in case we have to bail out of the
//	// function.
//	*ppData = data;
//	if(pnData)
//		*pnData = 0;
//
//	// Find the first occurance of the #, which indicates the ARB array data start.
//	char * it = data;
//	while(*it && *it++!= '#')
//		;
//
//	// If we failed to find the #, and instead found the null termination, log that and return with error
//	if(!*it)
//	{
//		return -1;
//	}
//
//	// Calculate how many digits that are used to describe the data length.
//	int digLen = (*it++)-'0';
//	int dataLen = 0;
//	while(*it && digLen--)
//		dataLen=(dataLen*10) + ((*it++)-'0');
//
//	// Test that where able to get the full length without finding a NULL.
//	// If digLen is zero, its ok, because we have read the entire header.
//	if(!*it && digLen!=0)
//	{
//		return -1;
//	}
//
//
//	*ppData = it;
//	if(pnData)
//		*pnData = dataLen;
//	return 0;
//}


//// TODO: move this function to IQmeasure_IQAPI.cpp
//// Jarir add this function for saving vsa capture to local machine, should be moved to iqxstream_api.cpp, TODO
//static int internal_TesterCommunication_Recieve(char ** return_buffer, int *buffer_length, int binary)
//{
//	int resize = 1024;
//	int trycount = 20000;
//	int readdata=0;
//	int err = 0;
//	int res;
//	char c=0;
//	int logNothingAfter=0;
//
//	char *buffer = *return_buffer;
//
//	if(binary)
//	{
//		res = recv(iqx.connectSocket, buffer,(*buffer_length)-1,0);
//		if(res >0)      readdata = res;
//	}
//	else
//	{
//		do
//		{
//			//int bReadingArrayLengthLength = 0;
//			while((c!='\n') && 1 == (res=recv(iqx.connectSocket, &c, 1,0)))
//			{
//
//				if (readdata >= (*buffer_length)-12)
//				{
//					char * new_buffer = (char *) malloc(sizeof(char) * ((*buffer_length)+resize));
//					strncpy(new_buffer, buffer, (*buffer_length));
//					*buffer_length = (*buffer_length) + resize;
//					free(buffer);
//					buffer = new_buffer;
//					//IQxstream_Logging_Info("[SCPI]-RE-ALLOC buffer length = %d", *buffer_length);
//				}
//
//				if(readdata==0 && c=='\n' )
//				{
//					c = 0;
//					// Skip leading \n's
//				}
//				else
//				{
//					buffer[readdata++] = c;
//
//					if(c == '#' )                                   // We should add verificaiton that the # is not in a string "#"
//					{
//						// Here is an ARB array coming, lets read it fast.
//
//						recv(iqx.connectSocket, &c, 1,0);
//						buffer[readdata++]=c;
//
//						if ( c < '0' || c > '9' )
//						{
//							if(c == '\n' )
//							{
//								trycount = 0;
//								break;
//							}
//							continue;
//						}
//						int digitsInARBLength=c-'0';
//						int arb_delta_add = 0;
//						logNothingAfter=readdata+digitsInARBLength;
//
//						readdata += recv(iqx.connectSocket, &buffer[readdata], digitsInARBLength,0);
//						// Read the rest of the ARB length header
//
//						if (buffer[readdata-digitsInARBLength] < '0' || buffer[readdata-digitsInARBLength] > '9')
//							// If the next two characters after # are not numbers, then it is not really and ARB array.
//						{
//							if(buffer[readdata-digitsInARBLength] == '\n' )
//								// If the character was a newline, then done reading
//							{
//								trycount = 0;
//								break;
//							}
//							continue;
//							// If this is really not an ARB read, continue at the top of the loop
//						}
//
//						buffer[readdata]= 0;
//						// NULL terminate the ARB header read so ATOI will work
//						int arbDataLength = atoi(&buffer[readdata-digitsInARBLength]);
//						// Translate ARB header to integer describing length of the bianary read
//						int arbRead = 0;
//
//						if(arbDataLength  > ((*buffer_length)-1) -readdata)
//							// If the buffer is too small to hold the ARB array, then grow the buffer by what is needed
//						{
//							arb_delta_add = arbDataLength - (((*buffer_length)-1) -readdata);
//
//							char * new_buffer = (char*) malloc(sizeof(char) * ((*buffer_length)+arb_delta_add));
//							strncpy(new_buffer, buffer, (*buffer_length));
//							*buffer_length = (*buffer_length) + arb_delta_add;
//							free(buffer);
//							buffer = new_buffer;
//						}
//
//						while(arbRead < arbDataLength && readdata < (*buffer_length)-1)
//						{
//							int r=recv(iqx.connectSocket, &buffer[readdata], arbDataLength-arbRead ,0);
//							// r = what is returned from the socket, not what is requested. Loop until we get what we want.
//							arbRead  +=r;
//							readdata +=r;
//						}
//					}
//					else if(c == '\n' )
//					{
//						trycount = 0; // Make sure we dont try to read more data. Its enough to read one line of response.
//						break;
//					}
//				}
//
//			}  // end while
//
//			if(res < 0 )
//			{
//				err = -1;
//			}
//			else
//				err = 0;
//		}
//		while (err == WSAETIMEDOUT && trycount-- > 0) ;
//
//	}
//
//	buffer[readdata] = 0;
//	*return_buffer = buffer;
//
//	if(readdata > 0)
//	{
//		if (binary)
//		{
//			// Do not log binary block receives
//		}
//	}
//
//	return readdata;
//}

// TODO: move this function to IQmeasure_IQAPI.cpp
// Jarir add following function for saving VSA data capture to local machine, must be moved to IQxstream_API.cpp, TODO, 12/9/11
//static int internal_waiter_TesterCommunication_Recieve(char **buffer, int *buffer_length, const char * whoami, int binary)
//{
//	int r = 0;
//	int i = 0;
//	int logging = 0;
//
//	struct __timeb64 timeStart, timeStop;
//
//	_ftime64(&timeStart);
//
//	unsigned int timeElapsed = 0;
//
//	while( 0 == (r = internal_TesterCommunication_Recieve(buffer, buffer_length,binary)) && timeElapsed < 60*1000) // DEFINE TIMEOUT (NOW 60s)
//	{
//		Sleep(500);
//		i++;
//		_ftime64(&timeStop);
//		timeElapsed = (unsigned int) ((timeStop.time - timeStart.time) * 1000 + (timeStop.millitm - timeStart.millitm));
//	}
//
//	if(timeElapsed >= 60*1000)  // DEFINE TIMEOUT (NOW 60s)
//	{
//		// Since we have failed to talk to the box for 60 seconds, we can now
//		// assume that the connection is broken. Disconnect so that all future calls to
//		// TesterCommunication_IsConnected() returns false;
//		//TesterCommunication_Close();
//		//IQxstream_Logging_Info("[ERROR] IQxstream read timed out");
//		//              while ((GetTimeSinceBootInMilliseconds() - starttime) <= IQXSTREAM_READTIMEOUT_SECONDS*2000)
//		//              {
//		//                      AudioServicesPlayAlertSound(1);
//		//                      IQxstream_Snooze(100);
//		//              }
//
//		return -1; // Unable to read data.
//	}
//	else
//		return r;
//}
//
//// TODO: move this function to IQmeasure_IQAPI.cpp
//// Jarir add following function to save VSA capture to local machine, must be moved to IQxstream_API.cpp, TODO, 12/9/11
//int TesterCommunication_RecieveBinary(char ** buffer, int *buffer_length, const char * whoami)
//{
//	return internal_waiter_TesterCommunication_Recieve(buffer, buffer_length, whoami, 1);
//}


//#endif // #if IQMEASURE_SCPI_CONTROL_ENABLE

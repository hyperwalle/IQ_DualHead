// IQmeasure Library in IQlite

#include "stdafx.h"
//Move to stdafx.h
//#include "lp_stdio.h"
#include "iqapi.h"
#include "iqapiCommon.h"
#include "..\IQmeasure.h"
//#include "..\DebugAlloc.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"
#include "StringUtil.h"
#include "IQ2010Ext.h"
#ifdef WIN32
#include "aimcontrol.h"
#endif
//Move to stdafx.h
//#include "lp_string.h"
#include <float.h> // DBL_MAX on Mac
#include "Version.h"
#include "iqapi11ac.h"


/*
#pragma comment(lib, "iqapi.lib")
#pragma comment(lib, "IQlite_Timer.lib")
#pragma comment(lib, "IQlite_Logger.lib")
#pragma comment(lib, "Util.lib")
#pragma comment(lib, "IQ2010Ext.lib")
*/

#ifndef WIN32
inline double abs(double x)
{ return (fabs(x)); }
#endif

// BEGIN GLOBALS

char *IQmeasure_Version = IQMEASURE_VERSION;

iqapiHndl				*hndl = NULL;
int                     timerIQmeasure  = -1;
int						loggerIQmeasure = -1;
highrestimer::lp_time_t timeStart, timeStop;
//double					timeStart, timeStop;
double                  timeDuration;
double					g_testerHwVersion = 0;
bool					g_bDisableFreqFilter = true;

iqapiAnalysis80216		*analysis80216 = NULL;
iqapiAnalysisMimo		*analysisMimo = NULL;
iqapiAnalysisOFDM		*analysisOfdm = NULL;
iqapiAnalysis11b		*analysis11b = NULL;
iqapiAnalysisPowerRamp	*analysisPowerRampOfdm = NULL;
iqapiAnalysisPowerRamp	*analysisPowerRamp11b = NULL;
iqapiAnalysisCW			*analysisCW = NULL;
iqapiAnalysisWave		*analysisWave = NULL;
iqapiAnalysisSidelobe	*analysisSidelobe = NULL;
iqapiAnalysisCCDF		*analysisCCDF = NULL;
iqapiAnalysisFFT		*analysisFFT = NULL;
iqapiAnalysisPower		*analysisPower = NULL;
iqapiAnalysisBluetooth	*analysisBluetooth = NULL;
iqapiAnalysisZigbee		*analysisZigbee = NULL;
#if !defined(IQAPI_1_5_X)
// iqapiAnalysisHT40 is supported in IQapi 1.6.x and beyond
iqapiAnalysisHT40		*analysisHT40 = NULL;
#endif

iqapiAnalysisWifiOfdm     *analysisWifiOfdm = NULL;
//iqapiAnalysis11ac       *analysis11ac = NULL;
iqapiAnalysisVHT80WideBand *analysisVHT80 = NULL;

//FM Analysis

iqapiAnalysisAudioStereo *analysisAudioStereo = NULL;
iqapiAnalysisAudioMono *analysisAudioMono = NULL;
iqapiAnalysisFmAudioBase *analysisFmAudioBase = NULL;
iqapiAnalysisFmRf *analysisFmRf = NULL;
iqapiAnalysisFmDemodSpectrum *analysisFmDemodSpectrum = NULL;
iqapiAnalysisFmMono *analysisFmMono = NULL;
iqapiAnalysisFmStereo *analysisFmStereo = NULL;
iqapiAnalysisFmAuto *analysisFmAuto = NULL;
iqapiAnalysisRds *analysisRds = NULL;
iqapiAnalysisRdsMono *analysisRdsMono = NULL;
iqapiAnalysisRdsStereo *analysisRdsStereo = NULL;
iqapiAnalysisRdsAuto *analysisRdsAuto = NULL;

iqapiResult80216		*result80216 = NULL;
iqapiResultMimo			*resultMimo = NULL;
iqapiResultOFDM			*resultOfdm = NULL;
iqapiResult11b			*result11b = NULL;
iqapiResultPowerRamp	*resultPowerRamp = NULL;
iqapiResultSidelobe		*resultSidelobe = NULL;
iqapiResultCW			*resultCW = NULL;
iqapiResultWave			*resultwave = NULL;
iqapiResultCCDF			*resultCCDF = NULL;
iqapiResultFFT			*resultFFT = NULL;
iqapiResultPower		*resultPower = NULL;
iqapiResultBluetooth	*resultBluetooth = NULL;
iqapiResultZigbee		*resultZigbee = NULL;
//Z-99
//iqapiResult11ac         *result11ac = NULL;
iqapiResultWifiOfdm		*result11ac = NULL;
iqapiResultVHT80WideBand     *resultVHT80Wideband = NULL;


//
// FM Results
iqapiResultAudioStereo  *resultAudioStereo = NULL;
iqapiResultAudioMono  *resultAudioMono = NULL;
iqapiResultFmRf  *resultFmRf = NULL;
iqapiResultFmDemodSpectrum  *resultFmDemodSpectrum = NULL;
iqapiResultFmMono  *resultFmMono = NULL;
iqapiResultFmStereo *resultFmStereo = NULL;
iqapiResultFmAuto *resultFmAuto = NULL;
iqapiResultRds *resultRds = NULL;
iqapiResultRdsMono *resultRdsMono = NULL;
iqapiResultRdsStereo *resultRdsStereo = NULL;
iqapiResultRdsAuto *resultRdsAuto = NULL;

//	FM Scalar Results container
map<string, double> g_FmRfScalarResults;
map<string, double> g_FmMonoScalarResults;
map<string, double> g_FmStereoScalarResults;

//	FM Vector Results container
map<string, vector <double> > g_FmRfVectorResults;
map<string, vector <double> > g_FmMonoVectorResults;
map<string, vector <double> > g_FmStereoVectorResults;

#if !defined(IQAPI_1_5_X)
// iqapiResultHT40 is supported in IQapi 1.6.x and beyond
iqapiResultHT40		    *resultHT40 = NULL;
#endif

#ifndef MAX_TESTER_NUM
#define MAX_TESTER_NUM 4
#endif

iqapiCapture			*capture = NULL;

bool					g_audioCaptureOnly = false;
bool					LibsInitialized = false;
bool                    setDefaultCalled = false;
int						nTesters = 0;
bool                    bIQ201xFound = false;
bool                    bIQxsFound = false;
bool                    bIQxelFound = false;


// FM Initialization
bool					FmInitialized = false;

// int    g_fast_buffSize  = 0;
// double *g_fast_Real_Ptr = NULL;
// double *g_fast_Imag_Ptr = NULL;
//static double *g_fastcal_pwr_i = NULL;
//static double *g_fastcal_pwr_q = NULL;
//static unsigned int g_fastcal_length_i;
//static unsigned int g_fastcal_length_q;

// This global variable indicates either a single waveform MOD file, or multi-segment MOD file
// has been loaded to the VSG memory.
enum LP_MEASUREMENT_VSG_MODE g_vsgMode;   //0-no MOD file loaded; 1-single MOD file; 2-multi-segment MOD file


// This variable remembers the last performed analysis
enum LP_ANALYSIS_TYPE_ENUM g_lastPerformedAnalysisType = ANALYSIS_MAX_NUM;

//TODO: The variable below is for working aroung the problem that
//      TX stopped working on IQnxn after RX
//      The root cause needs to be sorted out, and then remove this global variable
bool	g_previousOperationIsRX = false;

//This variable remebers whethere NxN function is called or not, used by:
// LP_SetVsgNxN(), LP_SetVsaNxN(), LP_EnableVsgRFNxN()
bool	g_nxnFunctionCalled     = false;

// The structure below is used internally, and for IQnxn only
struct _tagIQnxnSettings
{
	double	vsaAmplDBm;
	double	vsgPowerDBm;
	enum	IQV_PORT_ENUM vsaPort;
	enum	IQV_PORT_ENUM vsgPort;
	int		vsgRFEnabled;
} g_IQnxnSettings[N_MAX_TESTERS];

//This variable indicate whether the SCPI command is used to control the tester
bool    g_useScpi  = false;

//This variable indicate whether the IQapi is used to control the tester
bool    g_useIQapi = false;

#ifndef MAX_BUFFER_LEN
#define MAX_BUFFER_LEN 4096
#endif

#ifndef BT_SHIFT_HZ
/***** workaround temporary for BT TX capture issue Jim *****/
#define BT_SHIFT_HZ    0e6
/*
#define BT_SHIFT_HZ    7e6
/************************************************************/
#endif

double g_amplitudeToleranceDb = 3;

// This global variable is needed for selecting partial capture for analysis
// LP_SelectCaptureRangeForAnalysis() will set this variable
// LP_VsaDataCapture() will reset this variable to NULL
// Note: g_userData does not allocate memory at all.  It points to a portion of hndl->data
iqapiCapture *g_userData = NULL;

// Internal functions
// Functions declared below are not supposed to go in IQmeasure.h
void  InstantiateAnalysisClasses();
int   LP_Analyze(void);
void  InitializeTesterSettings(void);
// End of Internal functions

namespace {
	struct IQMeasureInit
	{
		IQMeasureInit() { LP_StartIQmeasureTimer(); }
		~IQMeasureInit() { LP_FreeMemory(); }
	} measInit;
}


IQMEASURE_API char *LP_GetErrorString(int err)
{
	::TIMER_StartTimer(timerIQmeasure, "LP_GetErrorString", &timeStart);

	switch(err)
	{
		case ERR_OK:								return "No errors";
		case ERR_NO_CONNECTION:						return "No connection to tester.";
		case ERR_NOT_INITIALIZED:					return "Library is not initialized. Call Init first.";
		case ERR_SAVE_WAVE_FAILED:					return "SaveWave failed. Check filename and/or path.";
		case ERR_LOAD_WAVE_FAILED:					return "LoadWave failed. Check filename and/or path.";
		case ERR_SET_TX_FAILED:						return "SetTx failed. Check tx input parameters.";
		case ERR_SET_WAVE_FAILED:					return "SetWave failed. Check filename and/or path.";
		case ERR_SET_RX_FAILED:						return "SetRx failed. Check rx input parameters.";
		case ERR_CAPTURE_FAILED:					return "Data capture failed.";
		case ERR_NO_CAPTURE_DATA:					return "No data to analyze in buffer.";
		case ERR_VSA_NUM_OUT_OF_RANGE:				return "VSA number is out of range. Try 1-4.";
		case ERR_ANALYSIS_FAILED:					return "Analysis failed.";
		case ERR_NO_VALID_ANALYSIS:					return "No valid analysis results.";
		case ERR_GENERAL_ERR:						return "A general error occurred.";
		case ERR_VSG_PORT_IS_OFF:					return "VSG port is turned off";
		case ERR_NO_MOD_FILE_LOADED:				return "No MOD file is loaded";
		case ERR_NO_CONT_FOR_MULTI_SEGMENT:			return "Multi-segment MOD cannot be in continuous play";
		case ERR_MEASUREMENT_NAME_NOT_FOUND:		return "The specified measurement name if not defined";
		case ERR_INVALID_ANALYSIS_TYPE:				return "Invalid analysis type";
		case ERR_NO_ANALYSIS_RESULT_AVAILABLE:		return "No Analysis results available";
		case ERR_NO_MEASUREMENT_RESULT:				return "No result for the specified measurement name";
		case ERR_SET_TOKEN_FAILED:					return "SetTokenID failed";
		case ERR_TOKEN_RETRIEVAL_TIMEDOUT:			return "TokenRetrieveTimeout timed out";
		case ERR_ANALYSIS_NULL_POINTER:             return "Analysis pointer is null";
		case ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME:   return "Analysis param name is not supported";
		case ERR_ANALYSIS_INVALID_PARAM_VALUE:      return "Analysis value is invalid";
		case ERR_INVALID_DATA_CAPTURE_RANGE:        return "Capture range slection is beyond valid range";

							    //FM Error codes
		case ERR_FM_NOT_INITIALIZED:			return "FM Technology is not initialized. FM Handle is not available.";
		case ERR_FM_SET_TX_FAILED:				return "Failed to set the FM VSG Parameters. Please check the input parameters";
		case ERR_FM_GET_TX_FAILED:				return "Failed to query the tester for FM VSG Parameters";
		case ERR_FM_SET_AUDIO_TONE_FAILED:		return "Failed to set the VSG Audio Tone. Please check the input parameters";
		case ERR_FM_GET_AUDIO_TONE_FAILED:		return "Failed to get the VSG Audio Tones.";
		case ERR_FM_NO_CAPTURE_DATA:			return "Failed to read Capture Data";
		case ERR_FM_VSA_CAPTURE_FAILED:			return "VSA failed to capture signal.";

		case ERR_FM_SET_RX_FAILED:				return "Failed to set the FM VSA parameters. Please check the input parameters";
		case ERR_FM_GET_RX_FAILED:				return "Failed to query the tester for FM VSA Parameters";
		case ERR_FM_ANALYSIS_FAILED:			return "FM Analysis failed";

		case ERR_FM_CAPTURE_AUDIO_FAILED:		return "Failed to capture Audio using AIM Device";
		case ERR_FM_GENERATE_AUDIO_FAILED:		return "Failed to generate Audio";
		case ERR_FM_QUERY_AIM_FAILED:			return "Failed to Query AIM Device";
		case ERR_FM_SAVE_WAV_AUDIO_FAILED:		return "Falied to Save Audio";
		case ERR_FM_PLAY_AUDIO_FAILED:			return "Failed to Play Audio using AIM Device";
		case ERR_FM_LOAD_AUDIO_FAILED:			return "Failed to Load Audio";
		case ERR_FM_STOP_AUDIO_PLAY_FAILED:	    return "Failed to stop Audio Play";
		default:									return "Unknown error condition.";
	}

	//    ::TIMER_StopTimer(timerIQmeasure, "LP_GetErrorString", &timeDuration, &timeStop);
	//	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_GetErrorString", timeDuration);
}

extern "C" __declspec(dllimport) int iqapiSetAnalysisMode(const char *lock_unlock_key);  //!< Internal use only

IQMEASURE_API int SetTesterHwVersion(double hwVersion)
{
	int err  = ERR_OK;
	g_testerHwVersion = hwVersion;
	return err;
}

IQMEASURE_API int GetTesterHwVersion(double *hwVersion)
{
	int err  = ERR_OK;
	*hwVersion = g_testerHwVersion;
	return err;
}

int LP_Analyze(void)
{
	int err = 0;

	//::TIMER_StartTimer(timerIQmeasure, "LP_SetAnalysisMode", &timeStart);
	//if  ( iqapiSetAnalysisMode("UNLOCK_KEY_1147014E-95E0-440d-A752-5F7C08A2FEE9") != IQAPI_ERR_OK )
	//{
	//	throw hndl->lastErr;
	//}
	//::TIMER_StopTimer(timerIQmeasure, "LP_SetAnalysisMode", &timeDuration, &timeStop);
	//   printf("\nTime to unlock analysis in LP_Analyze is %0.3f ms\n", timeDuration);

	// g_userData takes priority
	if( g_userData!=NULL )
	{
		err = hndl->Analyze( g_userData );
		if( ERR_OK!=err )
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{
			// nothing
		}
	}
	else if( hndl->data != NULL )
	{
		err = hndl->Analyze();
		if( ERR_OK!=err )
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{
			// nothing
		}
	}
	else
	{
		err = ERR_NO_CAPTURE_DATA;
	}

	//if  ( iqapiSetAnalysisMode("LOCK_KEY_431A4A5B-90A1-499c-B70F-11078C7C5954") != IQAPI_ERR_OK )
	//{
	//	throw hndl->lastErr;
	//}
	return err;
}

// FM Analyze hndl
int LP_Analyze_FM_Hndl(void)
{
	int err = 0;
	if( hndl->hndlFm->dataFm != NULL )
	{
		err = hndl->hndlFm->Analyze();
		if( ERR_OK != err)
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{
			// nothing
		}
	}
	else
	{
		err = ERR_NO_CAPTURE_DATA;
	}

	return err;
}

int LP_FreeMemory(void)
{
	int err = 0;

	::TIMER_StartTimer(timerIQmeasure, "LP_FreeMemory", &timeStart);

	if (NULL != hndl)	{ hndl->analysis = NULL; }

	if (analysis80216) {delete analysis80216; analysis80216 = NULL; }
	if (analysisMimo) {delete analysisMimo; analysisMimo = NULL; }
	if (analysisOfdm) {delete analysisOfdm; analysisOfdm = NULL; }
	if (analysis11b) {delete analysis11b; analysis11b = NULL; }
	if (analysisPowerRampOfdm) {delete analysisPowerRampOfdm; analysisPowerRampOfdm = NULL; }
	if (analysisPowerRamp11b) {delete analysisPowerRamp11b; analysisPowerRamp11b = NULL; }
	if (analysisCW) {delete analysisCW; analysisCW = NULL; }
	if (analysisSidelobe) {delete analysisSidelobe; analysisSidelobe = NULL; }
	if (analysisWave) {delete analysisWave; analysisWave = NULL;}
	if (analysisCCDF) {delete analysisCCDF; analysisCCDF = NULL; }
	if (analysisFFT) {delete analysisFFT; analysisFFT = NULL; }
	if (analysisPower) {delete analysisPower; analysisPower = NULL; }
	if (analysisBluetooth) {delete analysisBluetooth; analysisBluetooth = NULL; }
	if (analysisZigbee) {delete analysisZigbee; analysisZigbee = NULL; }
#if !defined(IQAPI_1_5_X)
	// iqapiResultHT40 is supported in IQapi 1.6.x and beyond
	if (analysisHT40) {delete analysisHT40; analysisHT40 = NULL; }
#endif
	if (analysisWifiOfdm) {delete analysisWifiOfdm; analysisWifiOfdm = NULL; }
	if (analysisVHT80) {delete analysisVHT80; analysisVHT80 = NULL; }

	//if (FmInitialized == true)
	//    if (hndl->hndlFm) {hndl->hndlFm->analysis = NULL;}

	//free FM
	if (analysisAudioStereo) {delete analysisAudioStereo; analysisAudioStereo = NULL; }
	if (analysisAudioMono) {delete analysisAudioMono; analysisAudioMono = NULL; }
	if (analysisFmAudioBase) {delete analysisFmAudioBase; analysisFmAudioBase = NULL; }
	if (analysisFmRf) {delete analysisFmRf; analysisFmRf = NULL; }
	if (analysisFmDemodSpectrum) {delete analysisFmDemodSpectrum; analysisFmDemodSpectrum = NULL; }
	if (analysisFmMono) {delete analysisFmMono; analysisFmMono = NULL; }
	if (analysisFmStereo) {delete analysisFmStereo; analysisFmStereo = NULL; }
	if (analysisFmAuto) {delete analysisFmAuto; analysisFmAuto = NULL; }
	if (analysisRds) {delete analysisRds; analysisRds = NULL; }
	if (analysisRdsMono) {delete analysisRdsMono; analysisRdsMono = NULL; }
	if (analysisRdsStereo) {delete analysisRdsStereo; analysisRdsStereo = NULL; }
	if (analysisRdsAuto) {delete analysisRdsAuto; analysisRdsAuto = NULL; }

	if (hndl) { delete hndl; hndl = NULL; }

	::TIMER_StopTimer(timerIQmeasure, "LP_FreeMemory", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FreeMemory", timeDuration);


	return err;
}


IQMEASURE_API char *LP_GetIQapiHndlLastErrMsg()
{
	return hndl->lastErr;
}


IQMEASURE_API int LP_Init(int IQtype, int testControlMethod)
{
	int err = ERR_OK;
	string objName = "";

	switch(testControlMethod)      //choose tester control method
	{
		case 0:
			g_useIQapi  = true;
			break;
		case 1:
			g_useScpi   = true;
			break;
		default:
			g_useIQapi  = true;
	}
	FmInitialized = false;
	LibsInitialized = false;

	::TIMER_StartTimer(timerIQmeasure, "LP_Init", &timeStart);

	if(true == g_useIQapi)
	{
#ifndef WIN32
		err = iqapiInit((bool)FALSE);
#else
		err = iqapiInit((bool)TRUE);  //to enable plotting figure in Windows
#endif

		if (hndl) { delete hndl; hndl = NULL; }
		hndl = new iqapiHndl();

		if(err == ERR_OK)
		{
			LibsInitialized = true;
		}

	}
	else if(true == g_useScpi)
	{
		LibsInitialized = true;
	}
	::TIMER_StopTimer(timerIQmeasure, "LP_Init", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_Init", timeDuration);

	return err;
}

IQMEASURE_API bool LP_GetVersion(char *buffer, int buf_size)
{
	bool err = true;
	char ver[MAX_BUFFER_LEN],  specifiedVer[MAX_BUFFER_LEN], str[MAX_BUFFER_LEN] = "IQmeasure: ";
	string tmpStr1 = "", tmpStr2 = "";
	//char strDelimit[]   = "\r\t\n";
	//char *token = NULL, *nextToken = NULL;

	::TIMER_StartTimer(timerIQmeasure, "LP_GetVersion", &timeStart);

	// Append the IQmeasure version
	tmpStr1 = IQmeasure_Version;
	tmpStr1 += " ";
	tmpStr1 += SVN_REVISION;
	tmpStr1 += "\n";
	strcat_s(str, MAX_BUFFER_LEN, tmpStr1.c_str());

	//In IQAPI 1.8.x, if use GetAllVersions, will crash while tesers >=3
	//TODO:
	if (hndl)
	{
		hndl->GetAllVersions(&ver[0], MAX_BUFFER_LEN);
	}
	else
	{
		if(LibsInitialized)
			iqapiVersion(&ver[0], MAX_BUFFER_LEN);
		else
			/*sprintf_s(buffer, MAX_BUFFER_LEN, "LP_Dut.dll");*/
			memset(buffer, '\0', buf_size);
		return err;
	}

	//iqapiVersion(&ver[0], MAX_BUFFER_LEN);
	strcat_s(str, MAX_BUFFER_LEN, ver);
	strcpy_s(ver, MAX_BUFFER_LEN, "");

	//Get Version:
	// 1. FM Version
	// 2. FPGA Version
	for(int testerNumber = 1; testerNumber <= nTesters;testerNumber++)
	{
		hndl->GetFirmwareVersion(&ver[0], MAX_BUFFER_LEN, testerNumber);
		sprintf_s(specifiedVer, "Tester %d FW:\t%s\n", testerNumber, ver);
		strcat_s(str, MAX_BUFFER_LEN, specifiedVer);
		strcpy_s(ver, MAX_BUFFER_LEN, "");

		hndl->GetFpgaVersion(&ver[0], MAX_BUFFER_LEN, IQV_FPGA_VSA, testerNumber);
		sprintf_s(specifiedVer, "Tester %d VSAFPGA:\t%s\n", testerNumber, ver);
		strcat_s(str, MAX_BUFFER_LEN, specifiedVer);
		strcpy_s(ver, MAX_BUFFER_LEN, "");

		hndl->GetFpgaVersion(&ver[0], MAX_BUFFER_LEN, IQV_FPGA_VSG, testerNumber);
		sprintf_s(specifiedVer, "Tester %d VSGFPGA:\t%s\n", testerNumber, ver);
		strcat_s(str, MAX_BUFFER_LEN, specifiedVer);
		strcpy_s(ver, MAX_BUFFER_LEN, "");

		hndl->GetHardwareVersion(&ver[0], MAX_BUFFER_LEN, testerNumber);
		//if(strlen(&ver[0])==0)strcpy_s(ver,"0.0.0.0");  // to adapt for daytona box
		sprintf_s(specifiedVer, "Tester %d hardware version:\t%s\n", testerNumber, ver);
		strcat_s(str, MAX_BUFFER_LEN, specifiedVer);
		strcpy_s(ver, MAX_BUFFER_LEN, "");

		////work around as the getallversions does not report the SN now
		hndl->GetSerialNumber(&ver[0], MAX_BUFFER_LEN, testerNumber);
		sprintf_s(specifiedVer, "Tester %d SN:\t%s\n", testerNumber, ver);
		//sprintf_s(specifiedVer, "Tester%d SN:\t\t%s", testerNumber, ver);
		strcat_s(str, MAX_BUFFER_LEN, specifiedVer);
		strcpy_s(ver, MAX_BUFFER_LEN, "");

	}


	//For IQ201X Tester, bIQ201xFound = true , in order to set trigger type = 13.
	//IQ201X Serial Number: IQP-----
	tmpStr1 = str;
	if(string::npos != tmpStr1.find("SN: IQP"))
	{
		bIQ201xFound = true;
		bIQxelFound = false;
		bIQxsFound = false;
	}
	else
	{
		bIQ201xFound = false;
	}

	//for future use.
	if(string::npos != tmpStr1.find("SN: IQXS")) //Prototype tester.
	{
		bIQxsFound = true;
		bIQxelFound = false;
		bIQ201xFound = false;
	}
	else
	{
		bIQxsFound = false;
	}


	if(string::npos != tmpStr1.find("IQXEL") || string::npos != tmpStr1.find("DTNA") || string::npos != tmpStr1.find("IQXL")) //DTNA 11ac tester.
	{
		bIQxelFound = true;
		bIQxsFound = false;
		bIQ201xFound = false;
	}
	else
	{
		bIQxelFound = false;
	}

	int len = (int) strlen(str);

	memset(buffer, '\0', buf_size);
	if ( len > buf_size )
	{
		strncpy_s(buffer, buf_size, str, buf_size-1);
		err = false;
	}
	else
	{
		strncpy_s(buffer, buf_size, str, len);
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_GetVersion", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_GetVersion", timeDuration);


	return err;
}


IQMEASURE_API int LP_GetIQapiHndl(void **getHndl, bool *testerInitialized)
{
	*getHndl = (void *)hndl;
	*testerInitialized = LibsInitialized;
	return ERR_OK;
}


IQMEASURE_API int LP_Term(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_Term", &timeStart);

	if( LibsInitialized )
	{
		//For dynamic-link usage, check handl is null or not
		if( hndl == NULL)
		{
			return err;
		}
		else
		{
			//do nothing
		}

		hndl->analysis = NULL;
		//	if (hndl->ConValid())   ConValid was for IQ2010 tester, not applicable for DTNA
		hndl->ConClose();

		LP_FreeMemory();

		nTesters = 0;

		if (hndl) { delete hndl; hndl = NULL; }
		///	iqapiTerm();
		LibsInitialized = false;
		FmInitialized = false;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_Term", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_Term", timeDuration);

	return err;
}

IQMEASURE_API int LP_InitTester(char *ipAddress)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_InitTester", &timeStart);

	if (LibsInitialized)
	{
		//2010-04-15: when connecting to IQ2010, ipAddress has one of the two formats:
		//            1. 127.0.0.1 - connecting to the first one, or the only one IQ2010
		//                           using port 4000
		//            2. 127.0.0.1:<port_number>:<serial_number> - connecting to the IQ2010
		//                                       with specified serial_number and specified port_number
		string			serialNumber = "";
		string			ipAddressAndPort = "";
		vector<string>	splits;


		splits.clear();
		SplitString(ipAddress, splits, ":");
		if( splits.size()==3 )
		{
			// both Serial# and Port# are specified
			serialNumber = Trim( splits[2] );
			ipAddressAndPort = Trim( splits[0] ) + ":" + Trim( splits[1] );
			err = hndl->ConInitBySerialNumber((char*)ipAddressAndPort.c_str(), (char*)serialNumber.c_str());
		}
		else
		{
			err = hndl->ConInit(ipAddress, IQV_CONNECTION_DAYTONA);
			//err = hndl->ConInit(ipAddress,IQV_CONNECTION_IQ201X);
			//to do, it is a hack now, how to determine the tester/connection type?

			//         char ver[MAX_BUFFER_LEN];
			//if ( hndl->GetSerialNumber(ver, MAX_BUFFER_LEN) != IQAPI_ERR_OK )
			//	throw hndl->lastErr;
			//else
			//	printf("Serial number       : %s\n", ver);



		}

		if ( 0 < err )
		{
			err = ERR_NO_CONNECTION;
		}
		else
		{
			nTesters = 1;
			InstantiateAnalysisClasses();
			InitializeTesterSettings();
			if(NULL != hndl->hndlFm)
			{
				FmInitialized = true;
			}
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_InitTester", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_InitTester", timeDuration);

	return err;
}

IQMEASURE_API int LP_InitTester2(char *ipAddress1, char *ipAddress2)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_InitTester2", &timeStart);

	if (LibsInitialized)
	{
		if(hndl->ConInit(ipAddress1, ipAddress2, IQV_CONNECTION_DAYTONA)) //to do, it is a hack now, how to determine the tester/connection type?
		{
			err = ERR_NO_CONNECTION;
		}
		else
		{
			nTesters = 2;
			InstantiateAnalysisClasses();
			InitializeTesterSettings();
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_InitTester2", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_InitTester2", timeDuration);

	return err;
}

IQMEASURE_API int LP_InitTester3(char *ipAddress1, char *ipAddress2, char *ipAddress3)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_InitTester3", &timeStart);

	if (LibsInitialized)
	{
		if (hndl->ConInit(ipAddress1, ipAddress2, ipAddress3, NULL, IQV_CONNECTION_DAYTONA))//to do, it is a hack now, how to determine the tester/connection type?
		{
			err = ERR_NO_CONNECTION;
		}
		else
		{
			nTesters = 3;
			InstantiateAnalysisClasses();
			InitializeTesterSettings();

		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_InitTester3", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_InitTester3", timeDuration);

	return err;
}

IQMEASURE_API int LP_InitTester4(char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_InitTester4", &timeStart);

	if (LibsInitialized)
	{
		if (hndl->ConInit(ipAddress1, ipAddress2, ipAddress3, ipAddress4,IQV_CONNECTION_DAYTONA))//to do, it is a hack now, how to determine the tester/connection type?
		{
			err = ERR_NO_CONNECTION;
		}
		else
		{
			nTesters = 4;
			InstantiateAnalysisClasses();
			InitializeTesterSettings();

		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_InitTester4", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_InitTester4", timeDuration);

	return err;
}


IQMEASURE_API int LP_ConClose(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_ConClose", &timeStart);

	if (LibsInitialized)
	{
		int iqapiError = hndl->ConClose();
		if (iqapiError)
		{
			err = ERR_GENERAL_ERR;
		}
		else
		{

		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_ConClose", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_ConClose", timeDuration);

	return err;
}

IQMEASURE_API int LP_ConOpen(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_ConOpen", &timeStart);

	if (LibsInitialized)
	{
		if (hndl->ConOpen())
		{
			err = ERR_GENERAL_ERR;
		}
		else
		{

		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	::TIMER_StopTimer(timerIQmeasure, "LP_ConOpen", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_ConOpen", timeDuration);

	return err;
}

IQMEASURE_API int LP_Agc(double *rfAmplDb, bool allTesters)
{
	int err = ERR_OK;
	//bool setupChanged = false;

	::TIMER_StartTimer(timerIQmeasure, "LP_Agc", &timeStart);

	if (LibsInitialized)
	{
		hndl->Agc(allTesters);
		*rfAmplDb = hndl->rx->vsa[0]->rfAmplDb;
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_Agc", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_Agc", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetFrameCnt(int frameCnt)
{
	int err = ERR_OK;
	//bool setupChanged = false;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetFrameCnt", &timeStart);

	if (LibsInitialized)
	{
		hndl->FrameTx(frameCnt);
		//TODO: to work arount the problem that TX stopped working on IQnxn after RX
		g_previousOperationIsRX = true;
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}
	::TIMER_StopTimer(timerIQmeasure, "LP_SetFrameCnt", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetFrameCnt", timeDuration);

	return err;
}
IQMEASURE_API int LP_SetDefault(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetDefault", &timeStart);

	if (LibsInitialized)
	{
		err = hndl->SetDefault();
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SetDefault", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetDefault", timeDuration);

	return err;
}
IQMEASURE_API int LP_ScpiCommandSet(char *scpiCommand)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_ScpiCommandSet", &timeStart);

	if (LibsInitialized)
	{
		err = hndl->ScpiCommandSet(scpiCommand);
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_ScpiCommandSet", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_ScpiCommandSet", timeDuration);

	return err;
}
IQMEASURE_API int LP_ScpiCommandQuery(char query[], unsigned int max_size, char response[], unsigned int *actual_size)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_ScpiCommandQuery", &timeStart);

	if (LibsInitialized)
	{
		err = hndl->ScpiCommandQuery(query,max_size,response,actual_size);
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_ScpiCommandQuery", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_ScpiCommandQuery", timeDuration);

	return err;
}
IQMEASURE_API int LP_TxDone(void)
{
	int err = ERR_OK;
	int err_code;

	::TIMER_StartTimer(timerIQmeasure, "LP_TxDone", &timeStart);

	if (LibsInitialized)
	{
		if( hndl->TxDone(&err_code) )
		{
			err = ERR_OK;
		}
		else
		{
			err = ERR_TX_NOT_DONE;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_TxDone", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_TxDone", timeDuration);

	return err;
}


/*! @defgroup group_vector_measurement Available Measurement Names for LP_GetStringMeasurement()
 *
 * Available measurement names vary for various analysis.  After an analysis has been performed successfully, by calling
 * one of the following functions:
 *   - LP_Analyze80211ag();
 *   - LP_AnalyzeMimo();
 *   - LP_Analyze80211b();
 *   - LP_Analyze80216d();
 *   - LP_Analyze80216e();
 *   - LP_AnalyzePower();
 *   - LP_AnalyzeFFT();
 *   - LP_AnalyzeCCDF();
 *   - LP_AnalyzeCW();
 *   - LP_AnalysisWave();
 *   - LP_AnalyzeSidelobe();
 *   - LP_AnalyzePowerRampOFDM();
 *   - LP_AnalyzePowerRamp80211b();
 *   - LP_AnalyzeBluetooth();
 *   - LP_AnalyzeZigbee();
 */
IQMEASURE_API int LP_GetStringMeasurement_NoTimer(char *measurement, char bufferChar[], int bufferLength)
{
	if (LibsInitialized)
	{
		if (!hndl->results || !bufferChar)
		{
			return 0;
		}
		else
		{
			//Z-99
			//if (dynamic_cast<iqapiResult11ac *>(hndl->results))
			if (dynamic_cast<iqapiResultWifiOfdm *>(hndl->results))
			{
				//Z-99
				//result11ac = dynamic_cast<iqapiResult11ac *>(hndl->results);
				result11ac = dynamic_cast<iqapiResultWifiOfdm *>(hndl->results);

				if (!strcmp(measurement, "rateInfo_modulation"))
				{
					if (result11ac->rateInfo_modulation)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(result11ac->rateInfo_modulation);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, result11ac->rateInfo_modulation, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "rateInfo_codingRate"))
				{
					if (result11ac->rateInfo_codingRate)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(result11ac->rateInfo_codingRate);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, result11ac->rateInfo_codingRate, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
			}
			if (dynamic_cast<iqapiResultMimo *>(hndl->results))
			{
				resultMimo = dynamic_cast<iqapiResultMimo *>(hndl->results);

				if (!strcmp(measurement, "rateInfo_modulation"))
				{
					if (resultMimo->rateInfo_modulation)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultMimo->rateInfo_modulation);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultMimo->rateInfo_modulation, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "rateInfo_codingRate"))
				{
					if (resultMimo->rateInfo_codingRate)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultMimo->rateInfo_codingRate);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultMimo->rateInfo_codingRate, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
			}
			if (dynamic_cast<iqapiResultBluetooth *>(hndl->results))
			{
				resultBluetooth = dynamic_cast<iqapiResultBluetooth*>(hndl->results);

				if (!strcmp(measurement, "analysisType"))
				{
					if (resultBluetooth->analysisType)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultBluetooth->analysisType);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultBluetooth->analysisType, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "versionString"))
				{
					if (resultBluetooth->versionString)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultBluetooth->versionString);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultBluetooth->versionString, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "acpErrMsg"))
				{
					if (resultBluetooth->acpErrMsg)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultBluetooth->acpErrMsg);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultBluetooth->acpErrMsg, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "leMsg"))
				{
					if (resultBluetooth->leMsg)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultBluetooth->leMsg);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultBluetooth->leMsg, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}

			}
		}
	}
	return 0;
}

// Keep this function with typo for backward compatibility
IQMEASURE_API int LP_GetStringMeasurment(char *measurement, char bufferChar[], int bufferLength)
{
	return LP_GetStringMeasurement(measurement, bufferChar, bufferLength);
}

IQMEASURE_API int LP_GetStringMeasurement(char *measurement, char bufferChar[], int bufferLength)
{
	::TIMER_StartTimer(timerIQmeasure, "LP_GetStringMeasurement", &timeStart);

	int ret = LP_GetStringMeasurement_NoTimer(measurement, bufferChar, bufferLength);

	::TIMER_StopTimer(timerIQmeasure, "LP_GetStringMeasurement", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_GetStringMeasurement", timeDuration);

	return ret;
}

IQMEASURE_API int LP_EnableVsgRFNxN(int vsg1Enabled, int vsg2Enabled, int vsg3Enabled, int vsg4Enabled)
{

	g_IQnxnSettings[0].vsgRFEnabled = vsg1Enabled;
	g_IQnxnSettings[1].vsgRFEnabled = vsg2Enabled;
	g_IQnxnSettings[2].vsgRFEnabled = vsg3Enabled;
	g_IQnxnSettings[3].vsgRFEnabled = vsg4Enabled;

	//Remembers NxN function is called
	g_nxnFunctionCalled = true;

	return LP_EnableVsgRF(vsg1Enabled);
}

IQMEASURE_API int LP_EnableVsgRF(int enabled)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_EnableVsgRF", &timeStart);

	IQV_RF_ENABLE_ENUM rfEnabled;

	if (LibsInitialized)
	{
		for(int i=0; i<nTesters; i++)
		{
			// For rest testers of IQnxn
			if( i>0 )
			{
				//If NxN function is called,     apply g_IQnxnSettings to testers.
				//If NxN function is not called, apply 1st tester setting to all testers
				if (g_nxnFunctionCalled)
				{
					enabled = g_IQnxnSettings[i].vsgRFEnabled;
				}
				else
				{
					//apply 1st tester setting to all testers
				}
			}

			rfEnabled = (enabled==0?IQV_RF_DISABLED:IQV_RF_ENABLED);
			if( hndl->tx->vsg[i]->enabled != rfEnabled)
			{
				hndl->tx->vsg[i]->enabled = rfEnabled;
				if (hndl->SetTx())
				{
					err = ERR_SET_WAVE_FAILED;
				}
				else
				{
					hndl->tx->vsg[i]->enabled = IQV_RF_DISABLED;
				}
				//if (hndl->SetTxRx())
				//{
				//	err = ERR_SET_WAVE_FAILED;
				//}
				
			}
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	//After dealing with NxN function, restore g_nxnFunctionCalled to false
	g_nxnFunctionCalled = false;

	::TIMER_StopTimer(timerIQmeasure, "LP_EnableVsgRF", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_EnableVsgRF", timeDuration);

	return err;
}

IQMEASURE_API int LP_EnableSpecifiedVsgRF(int enabled, int vsgNumber)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_EnableSpecifiedVsgRF", &timeStart);

	IQV_RF_ENABLE_ENUM rfEnabled = (enabled==0?IQV_RF_DISABLED:IQV_RF_ENABLED);

	if (LibsInitialized)
	{
		hndl->tx->vsg[vsgNumber]->enabled = rfEnabled;


		if (hndl->SetTx())
		{
			err = ERR_SET_WAVE_FAILED;
		}
		else
		{
			err = ERR_OK;
		}

	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_EnableSpecifiedVsgRF", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_EnableSpecifiedVsgRF", timeDuration);

	return err;
}

IQMEASURE_API int LP_EnableSpecifiedVsaRF(int enabled, int vsaNumber)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_EnableSpecifiedVsaRF", &timeStart);

	IQV_RF_ENABLE_ENUM rfEnabled = (enabled==0?IQV_RF_DISABLED:IQV_RF_ENABLED);

	if (LibsInitialized)
	{
		hndl->rx->vsa[vsaNumber]->enabled = rfEnabled;


		if (hndl->SetRx())
		{
			err = ERR_SET_WAVE_FAILED;
		}
		else
		{
			err = ERR_OK;
		}

	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_EnableSpecifiedVsaRF", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_EnableSpecifiedVsaRF", timeDuration);

	return err;
}
IQMEASURE_API int LP_GetVsaSettings(double *freqHz, double *ampl, IQAPI_PORT_ENUM *port, int *rfEnabled, double *triggerLevel)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_GetVsaSettings", &timeStart);

	if (LibsInitialized)
	{
		*freqHz = hndl->rx->rfFreqHz;
		*ampl   = hndl->rx->vsa[0]->rfAmplDb;
		if( hndl->rx->rxMode == IQV_INPUT_ADC_BB )
		{
			*port = PORT_BB;
		}
		else
		{
			*port = (IQAPI_PORT_ENUM)hndl->rx->vsa[0]->port;
		}
		*rfEnabled = hndl->rx->vsa[0]->enabled;
		*triggerLevel = hndl->rx->triggerLevelDb;
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_GetVsaSettings", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_GetVsaSettings", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetVsgCw(double rfFreqHz, double offsetFrequencyMHz, double rfPowerLeveldBm, int port)
{
	int err = ERR_OK;
	bool setupChanged  = false;
	bool offsetChanged = false;
	bool RFused        = true;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsgCw", &timeStart);

	if (LibsInitialized)
	{
		for (int i=0; i<nTesters; i++)
		{
			if (hndl->tx->vsg[i]->source != IQV_SOURCE_SIGNAL_GENERATOR)
			{
				hndl->tx->vsg[i]->source = IQV_SOURCE_SIGNAL_GENERATOR;
				setupChanged = true;
			}

			if (hndl->tx->rfFreqHz != rfFreqHz || hndl->tx->vsg[i]->sineFreqHz!=offsetFrequencyMHz*1e6 || hndl->tx->freqShiftHz != 0 || g_bDisableFreqFilter == true)
			{
				hndl->tx->rfFreqHz = rfFreqHz;
				hndl->rx->rfFreqHz = rfFreqHz;
				hndl->tx->freqShiftHz = 0;
				hndl->rx->freqShiftHz = 0;
				hndl->tx->vsg[i]->sineFreqHz = offsetFrequencyMHz*1e6;
				hndl->tx->sampleFreqHz = 80.0e6;
				hndl->tx->modulationMode = IQV_WAVE_DL_MOD_ENABLE;
				hndl->tx->gapPowerOff = IQV_DISABLE_MARKER_SIGNAL;
				setupChanged = true;
				g_bDisableFreqFilter = false;
			}

			if (RFused && hndl->tx->vsg[i]->port != (IQV_PORT_ENUM)port)
			{
				if ((IQV_PORT_ENUM)port == IQV_PORT_LEFT)
				{
					hndl->tx->vsg[i]->port = IQV_PORT_LEFT;
					hndl->rx->vsa[i]->port = IQV_PORT_RIGHT;
					setupChanged = true;
				}
				else if ((IQV_PORT_ENUM)port == IQV_PORT_RIGHT)
				{
					hndl->tx->vsg[i]->port = IQV_PORT_RIGHT;
					hndl->rx->vsa[i]->port = IQV_PORT_LEFT;
					setupChanged = true;
				}
				else
				{
					hndl->tx->vsg[i]->port = (IQV_PORT_ENUM)port;
					setupChanged = true;
				}
			}

			if(RFused)
				hndl->tx->vsg[i]->rfGainDb = rfPowerLeveldBm;
			else
			{
				hndl->tx->vsg[i]->bbGainDb = rfPowerLeveldBm;
			}
			hndl->tx->vsg[0]->enabled = IQV_RF_ENABLED;
			setupChanged = true;
		}

		if (setupChanged)
		{
			if (hndl->SetTxRx())
				err = ERR_SET_TX_FAILED;
		}

		// Since there is a bug inside IQapi, the CW signal always shift -1 MHz offset only.
		// Or we need to set VSG twice to fix this issue.
		for (int i=0; i<nTesters; i++)
		{
			if ( hndl->tx->vsg[i]->sineFreqHz!=offsetFrequencyMHz*1e6 )
			{
				hndl->tx->vsg[i]->sineFreqHz = offsetFrequencyMHz*1e6;
				offsetChanged = true;
			}
			else
			{
				// do nothing
			}
		}

		if ( offsetChanged )
		{
			if (hndl->SetTxRx())
				err = ERR_SET_TX_FAILED;
		}
		else
		{
			// do nothing
		}
	}
	else
		err = ERR_NOT_INITIALIZED;


	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsgCw", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetVsgCw", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetVsgNxN(double rfFreqHz, double rfPowerLeveldBm[], int port[], double dFreqShiftHz)
{
	for( int i=0; i<N_MAX_TESTERS; i++)
	{
		g_IQnxnSettings[i].vsgPort			= (IQV_PORT_ENUM)port[i];
		g_IQnxnSettings[i].vsgPowerDBm		= rfPowerLeveldBm[i];
	}

	//Remembers NxN function is called
	g_nxnFunctionCalled = true;

	return LP_SetVsg(rfFreqHz, rfPowerLeveldBm[0], port[0], true, dFreqShiftHz);
}


IQMEASURE_API int LP_SetVsg(double rfFreqHz, double rfPowerLeveldBm, int port, bool setGapPowerOff, double dFreqShiftHz)
{
	int err = ERR_OK;
	bool setupChanged = false;
	bool RFused = true;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsg", &timeStart);

	if (LibsInitialized)
	{
		if ( (hndl->tx->rfFreqHz!=rfFreqHz)||(hndl->tx->freqShiftHz!=dFreqShiftHz) || g_bDisableFreqFilter == true )
		{
			hndl->tx->rfFreqHz = rfFreqHz;
			hndl->rx->rfFreqHz = rfFreqHz;

			hndl->tx->freqShiftHz = dFreqShiftHz;
			hndl->rx->freqShiftHz = dFreqShiftHz;
			setupChanged = true;
			g_bDisableFreqFilter = false;
		}
		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_BB && port != PORT_BB)
		{
			hndl->tx->txMode = IQV_INPUT_MOD_DAC_RF;
			RFused = true;
			setupChanged = true;
		}

		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_RF && port == PORT_BB)
		{
			hndl->tx->txMode = IQV_INPUT_MOD_DAC_BB;
			//port = (int)PORT_RIGHT;
			RFused = false;
			setupChanged = true;
		}

		if (setGapPowerOff == true)
		{
			if (hndl->tx->gapPowerOff == IQV_GAP_POWER_ON)
			{
				hndl->tx->gapPowerOff = IQV_GAP_POWER_OFF;
				setupChanged = true;
			}
		}
		else
		{
			if (hndl->tx->gapPowerOff == IQV_GAP_POWER_OFF)
			{
				hndl->tx->gapPowerOff = IQV_GAP_POWER_ON;
				setupChanged = true;
			}
		}

		for (int i=0; i<nTesters; i++)
		{
			//		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_BB && port != PORT_BB)
			//		{
			//			hndl->tx->txMode = IQV_INPUT_MOD_DAC_RF;
			//			RFused = true;
			//			setupChanged = true;
			//		}

			//		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_RF && port == PORT_BB)
			//		{
			//			hndl->tx->txMode = IQV_INPUT_MOD_DAC_BB;
			//			//port = (int)PORT_RIGHT;
			//			RFused = false;
			//			setupChanged = true;
			//		}

			// For rest testers of IQnxn
			if( i>0 )
			{
				//If NxN function is called,     apply g_IQnxnSettings to testers.
				//If NxN function is not called, apply 1st tester setting to all testers
				if(g_nxnFunctionCalled)
				{
					port				= g_IQnxnSettings[i].vsgPort;
					rfPowerLeveldBm		= g_IQnxnSettings[i].vsgPowerDBm;
				}
				else
				{
					//apply 1st tester setting to all testers
				}

			}
			if( VERY_LOW_VSG_POWER_DBM < rfPowerLeveldBm )
			{
				if(hndl->tx->vsg[i]->enabled != (IQV_RF_ENABLE_ENUM)IQV_RF_ENABLED)
				{
					hndl->tx->vsg[i]->enabled = IQV_RF_ENABLED;
					setupChanged = true;
				}
			}
			else
			{
				if (hndl->tx->vsg[i]->enabled != (IQV_RF_ENABLE_ENUM)IQV_RF_DISABLED)
				{
					hndl->tx->vsg[i]->enabled = IQV_RF_DISABLED;
					setupChanged = true;
				}
				else
				{
					//do nothing
				}
			}

			if (RFused && hndl->tx->vsg[i]->port != (IQV_PORT_ENUM)port)
			{
				if ((IQV_PORT_ENUM)port == IQV_PORT_LEFT)
				{
					hndl->tx->vsg[i]->port = IQV_PORT_LEFT;
					hndl->rx->vsa[i]->port = IQV_PORT_RIGHT;
					setupChanged = true;
				}
				else if ((IQV_PORT_ENUM)port == IQV_PORT_RIGHT)
				{
					hndl->tx->vsg[i]->port = IQV_PORT_RIGHT;
					hndl->rx->vsa[i]->port = IQV_PORT_LEFT;
					setupChanged = true;
				}
				else
				{
					hndl->tx->vsg[i]->port = (IQV_PORT_ENUM)port;
					setupChanged = true;
				}
			}

			if (hndl->tx->vsg[i]->rfGainDb != rfPowerLeveldBm)
			{
				if(RFused)
					hndl->tx->vsg[i]->rfGainDb = rfPowerLeveldBm;
				else
					hndl->tx->vsg[i]->bbGainDb = rfPowerLeveldBm;
				setupChanged = true;
			}

		}
		if (setupChanged)
		{
			if (hndl->SetTxRx())
			{
				err = ERR_SET_TX_FAILED;
				throw hndl->lastErr; //Zhiyong debug here
			}
		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	//After dealing with NxN function, restore g_nxnFunctionCalled to false
	g_nxnFunctionCalled = false;

	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsg", &timeDuration, &timeStop);
	char temp[MAX_BUFFER_SIZE];
	sprintf_s(temp, "LP_SetVsg %4.0f %6.2f", rfFreqHz/1e6, rfPowerLeveldBm);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", temp, timeDuration);

	return err;
}

IQMEASURE_API int LP_SetVsg_triggerType(double rfFreqHz, double rfPowerLeveldBm, int port, int triggerType)
{
	int err = ERR_OK;
	bool setupChanged = false;
	bool RFused = true;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsg", &timeStart);

	if (LibsInitialized)
	{
		if ( (hndl->tx->rfFreqHz!=rfFreqHz)||(hndl->tx->freqShiftHz!=0) || g_bDisableFreqFilter == true)
		{
			hndl->tx->rfFreqHz = rfFreqHz;
			hndl->rx->rfFreqHz = rfFreqHz;

			hndl->tx->freqShiftHz = 0;
			hndl->rx->freqShiftHz = 0;
			setupChanged = true;
			g_bDisableFreqFilter = false;
		}
		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_BB && port != PORT_BB)
		{
			hndl->tx->txMode = IQV_INPUT_MOD_DAC_RF;
			RFused = true;
			setupChanged = true;
		}

		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_RF && port == PORT_BB)
		{
			hndl->tx->txMode = IQV_INPUT_MOD_DAC_BB;
			//port = (int)PORT_RIGHT;
			RFused = false;
			setupChanged = true;
		}

		if (hndl->tx->gapPowerOff == IQV_GAP_POWER_ON)
		{
			hndl->tx->gapPowerOff = IQV_GAP_POWER_OFF;
			setupChanged = true;
		}

		if(hndl->tx->triggerType != (IQV_VSG_TRIG)triggerType)
		{
			hndl->tx->triggerType = (IQV_VSG_TRIG)triggerType;
			setupChanged = true;
		}

		for (int i=0; i<nTesters; i++)
		{
			//		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_BB && port != PORT_BB)
			//		{
			//			hndl->tx->txMode = IQV_INPUT_MOD_DAC_RF;
			//			RFused = true;
			//			setupChanged = true;
			//		}

			//		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_RF && port == PORT_BB)
			//		{
			//			hndl->tx->txMode = IQV_INPUT_MOD_DAC_BB;
			//			//port = (int)PORT_RIGHT;
			//			RFused = false;
			//			setupChanged = true;
			//		}

			// For rest testers of IQnxn
			if( i>0 )
			{
				//If NxN function is called,     apply g_IQnxnSettings to testers.
				//If NxN function is not called, apply 1st tester setting to all testers
				if(g_nxnFunctionCalled)
				{
					port				= g_IQnxnSettings[i].vsgPort;
					rfPowerLeveldBm		= g_IQnxnSettings[i].vsgPowerDBm;
				}
				else
				{
					//apply 1st tester setting to all testers
				}

			}
			if( VERY_LOW_VSG_POWER_DBM < rfPowerLeveldBm )
			{
				if(hndl->tx->vsg[i]->enabled != (IQV_RF_ENABLE_ENUM)IQV_RF_ENABLED)
				{
					hndl->tx->vsg[i]->enabled = IQV_RF_ENABLED;
					setupChanged = true;
				}
			}
			else
			{
				if (hndl->tx->vsg[i]->enabled != (IQV_RF_ENABLE_ENUM)IQV_RF_DISABLED)
				{
					hndl->tx->vsg[i]->enabled = IQV_RF_DISABLED;
					setupChanged = true;
				}
				else
				{
					//do nothing
				}
			}

			if (RFused && hndl->tx->vsg[i]->port != (IQV_PORT_ENUM)port)
			{
				if ((IQV_PORT_ENUM)port == IQV_PORT_LEFT)
				{
					hndl->tx->vsg[i]->port = IQV_PORT_LEFT;
					hndl->rx->vsa[i]->port = IQV_PORT_RIGHT;
					setupChanged = true;
				}
				else if ((IQV_PORT_ENUM)port == IQV_PORT_RIGHT)
				{
					hndl->tx->vsg[i]->port = IQV_PORT_RIGHT;
					hndl->rx->vsa[i]->port = IQV_PORT_LEFT;
					setupChanged = true;
				}
				else
				{
					hndl->tx->vsg[i]->port = (IQV_PORT_ENUM)port;
					setupChanged = true;
				}
			}

			if (hndl->tx->vsg[i]->rfGainDb != rfPowerLeveldBm)
			{
				if(RFused)
					hndl->tx->vsg[i]->rfGainDb = rfPowerLeveldBm;
				else
					hndl->tx->vsg[i]->bbGainDb = rfPowerLeveldBm;
				setupChanged = true;
			}

		}
		if (setupChanged)
		{
			if (hndl->SetTxRx())
				err = ERR_SET_TX_FAILED;
		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	//After dealing with NxN function, restore g_nxnFunctionCalled to false
	g_nxnFunctionCalled = false;

	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsg", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetVsg", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetVsg_GapPower(double rfFreqHz, double rfPowerLeveldBm, int port, int gapPowerOff)
{
	int err = ERR_OK;
	bool setupChanged = false;
	bool RFused = true;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsg", &timeStart);

	if (LibsInitialized)
	{
		if ( (hndl->tx->rfFreqHz!=rfFreqHz)||(hndl->tx->freqShiftHz!=0)||g_bDisableFreqFilter == true )
		{
			hndl->tx->rfFreqHz = rfFreqHz;
			hndl->rx->rfFreqHz = rfFreqHz;

			hndl->tx->freqShiftHz = 0;
			hndl->rx->freqShiftHz = 0;
			setupChanged = true;
			g_bDisableFreqFilter = false;
		}
		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_BB && port != PORT_BB)
		{
			hndl->tx->txMode = IQV_INPUT_MOD_DAC_RF;
			RFused = true;
			setupChanged = true;
		}

		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_RF && port == PORT_BB)
		{
			hndl->tx->txMode = IQV_INPUT_MOD_DAC_BB;
			//port = (int)PORT_RIGHT;
			RFused = false;
			setupChanged = true;
		}

		if(IQV_GAP_POWER_ON == (IQV_GAP_POWER)gapPowerOff)
		{
			if (hndl->tx->gapPowerOff == IQV_GAP_POWER_ON)
			{
				//do nothing
				setupChanged = false;
			}
			else
			{
				hndl->tx->gapPowerOff = IQV_GAP_POWER_ON;
				setupChanged = true;
			}
		}
		else
		{
			if (hndl->tx->gapPowerOff == IQV_GAP_POWER_ON)
			{
				hndl->tx->gapPowerOff = IQV_GAP_POWER_OFF;
				setupChanged = true;
			}
			else
			{
				//do nothing
				setupChanged = false;
			}
		}

		for (int i=0; i<nTesters; i++)
		{
			//		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_BB && port != PORT_BB)
			//		{
			//			hndl->tx->txMode = IQV_INPUT_MOD_DAC_RF;
			//			RFused = true;
			//			setupChanged = true;
			//		}

			//		if (hndl->tx->txMode == IQV_INPUT_MOD_DAC_RF && port == PORT_BB)
			//		{
			//			hndl->tx->txMode = IQV_INPUT_MOD_DAC_BB;
			//			//port = (int)PORT_RIGHT;
			//			RFused = false;
			//			setupChanged = true;
			//		}

			// For rest testers of IQnxn
			if( i>0 )
			{
				//If NxN function is called,     apply g_IQnxnSettings to testers.
				//If NxN function is not called, apply 1st tester setting to all testers
				if(g_nxnFunctionCalled)
				{
					port				= g_IQnxnSettings[i].vsgPort;
					rfPowerLeveldBm		= g_IQnxnSettings[i].vsgPowerDBm;
				}
				else
				{
					//apply 1st tester setting to all testers
				}

			}
			if( VERY_LOW_VSG_POWER_DBM < rfPowerLeveldBm )
			{
				if(hndl->tx->vsg[i]->enabled != (IQV_RF_ENABLE_ENUM)IQV_RF_ENABLED)
				{
					hndl->tx->vsg[i]->enabled = IQV_RF_ENABLED;
					setupChanged = true;
				}
			}
			else
			{
				if (hndl->tx->vsg[i]->enabled != (IQV_RF_ENABLE_ENUM)IQV_RF_DISABLED)
				{
					hndl->tx->vsg[i]->enabled = IQV_RF_DISABLED;
					setupChanged = true;
				}
				else
				{
					//do nothing
				}
			}

			if (RFused && hndl->tx->vsg[i]->port != (IQV_PORT_ENUM)port)
			{
				if ((IQV_PORT_ENUM)port == IQV_PORT_LEFT)
				{
					hndl->tx->vsg[i]->port = IQV_PORT_LEFT;
					hndl->rx->vsa[i]->port = IQV_PORT_RIGHT;
					setupChanged = true;
				}
				else if ((IQV_PORT_ENUM)port == IQV_PORT_RIGHT)
				{
					hndl->tx->vsg[i]->port = IQV_PORT_RIGHT;
					hndl->rx->vsa[i]->port = IQV_PORT_LEFT;
					setupChanged = true;
				}
				else
				{
					hndl->tx->vsg[i]->port = (IQV_PORT_ENUM)port;
					setupChanged = true;
				}
			}

			if (hndl->tx->vsg[i]->rfGainDb != rfPowerLeveldBm)
			{
				if(RFused)
					hndl->tx->vsg[i]->rfGainDb = rfPowerLeveldBm;
				else
					hndl->tx->vsg[i]->bbGainDb = rfPowerLeveldBm;
				setupChanged = true;
			}

		}
		if (setupChanged)
		{
			if (hndl->SetTxRx())
				err = ERR_SET_TX_FAILED;
		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	//After dealing with NxN function, restore g_nxnFunctionCalled to false
	g_nxnFunctionCalled = false;

	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsg", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetVsg", timeDuration);

	return err;
}


IQMEASURE_API int LP_SetVsgModulation(char *modFileName, int loadInternalWaveform)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsgModulation", &timeStart);

	//change file extension from .mod to .iqvsg
	char *pch = NULL;
	pch = strstr(modFileName,".mod\0");
	if(pch!= NULL)
		strncpy (pch,".iqvsg\0",7);

	if (LibsInitialized)
	{
		if(loadInternalWaveform == 0)
		{
			if (hndl->SetWave(modFileName))
			{
				err = ERR_SET_WAVE_FAILED;
				//throw hndl->lastErr; //Zhiyong debug
			}
			else
			{
				// Mark as single MOD file mod
				g_vsgMode = ::VSG_SINGLE_MOD_FILE;
			}
		}
		else
		{
			if (hndl->SetWaveScpi(modFileName))
			{
				err = ERR_SET_WAVE_FAILED;
				//throw hndl->lastErr; //Zhiyong debug
			}
			else
			{
				// Mark as single MOD file mod
				g_vsgMode = ::VSG_SINGLE_MOD_FILE;
			}
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsgModulation", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetVsgModulation", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetVsgModulation_SetPlayCondition(char *modFileName, bool autoPlay, int loadInternalWaveform)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsgModulation_SetPlayCondition", &timeStart);

	//change file extension from .mod to .iqvsg
	char *pch = NULL;
	pch = strstr(modFileName,".mod\0");
	if(pch!= NULL)
		strncpy (pch,".iqvsg\0",7);

	if (LibsInitialized)
	{
		if(loadInternalWaveform == 0)
		{
			if (hndl->SetWave(modFileName, autoPlay))
			{
				err = ERR_SET_WAVE_FAILED;
				//throw hndl->lastErr; //Zhiyong debug
			}
			else
			{
				// Mark as single MOD file mod
				g_vsgMode = ::VSG_SINGLE_MOD_FILE;
			}
		}
		else
		{
			if (hndl->SetWaveScpi(modFileName, autoPlay))
			{
				err = ERR_SET_WAVE_FAILED;
				//throw hndl->lastErr; //Zhiyong debug
			}
			else
			{
				// Mark as single MOD file mod
				g_vsgMode = ::VSG_SINGLE_MOD_FILE;
			}
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsgModulation_SetPlayCondition", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetVsgModulation_SetPlayCondition", timeDuration);

	return err;
}
/*! @defgroup group_analysis_name Available Analysis Name for LP_SetAnalysisParameterInteger()
 *
 * Available analysis name vary for various analysis.  Before doing analysis action, by calling
 * one of the following functions:
 *   - LP_AnalyzeMimo();
 *   - LP_Analyze80211b();
 *   - LP_Analyze80211ag();
 *   - LP_AnalyzePower();
 *   - LP_AnalyzeFFT();
 *   - LP_AnalyzeCCDF();
 *
 */
IQMEASURE_API int LP_SetAnalysisParameterInteger(char *measurement, char *parameter, int value)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetAnalysisParameterInteger", &timeStart);

	if (LibsInitialized)
	{
		if (NULL==analysis80216 &&
				NULL==analysisMimo &&
				NULL==analysisOfdm &&
				NULL==analysis11b &&
				NULL==analysisPowerRampOfdm &&
				NULL==analysisPowerRamp11b &&
				NULL==analysisCW &&
				NULL==analysisWave &&
				NULL==analysisSidelobe &&
				NULL==analysisCCDF &&
				NULL==analysisFFT &&
				NULL==analysisPower &&
				NULL==analysisBluetooth &&
				NULL==analysisZigbee &&
				NULL==analysisHT40&&
				NULL==analysisWifiOfdm&&    //need to implement 11AC specific parameters, might be close to MIMO
				NULL==analysisVHT80)
		{
			err = ERR_ANALYSIS_NULL_POINTER;
		}
		else
		{
			if(!strcmp(measurement, "AnalyzePower"))
			{
				if(!strcmp(parameter, "vsaNum"))
				{
					if( (IQV_VSA_NUM_ENUM)value >= IQV_VSA_NUM_1 &&
							(IQV_VSA_NUM_ENUM)value <= IQV_VSA_NUM_4)
					{
						analysisPower->vsaNum = (IQV_VSA_NUM_ENUM)value;
					}
					else
					{
						err = ERR_ANALYSIS_INVALID_PARAM_VALUE;
					}
				}
				else
				{
					err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME;
				}

			}
			else if(!strcmp(measurement, "Analyze11b"))
			{
				if(!strcmp(parameter, "vsaNum"))
				{
					if( (IQV_VSA_NUM_ENUM)value >= IQV_VSA_NUM_1 &&
							(IQV_VSA_NUM_ENUM)value <= IQV_VSA_NUM_4)
					{
						analysis11b->vsaNum = (IQV_VSA_NUM_ENUM)value;
					}
					else
					{
						err = ERR_ANALYSIS_INVALID_PARAM_VALUE;
					}
				}
				else
				{
					err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME;
				}

			}
			else if(!strcmp(measurement, "AnalyzeOFDM"))
			{
				if(!strcmp(parameter, "vsaNum"))
				{
					if( (IQV_VSA_NUM_ENUM)value >= IQV_VSA_NUM_1 &&
							(IQV_VSA_NUM_ENUM)value <= IQV_VSA_NUM_4)
					{
						analysisOfdm->vsaNum = (IQV_VSA_NUM_ENUM)value;
					}
					else
					{
						err = ERR_ANALYSIS_INVALID_PARAM_VALUE;
					}
				}
				else
				{
					err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME;
				}

			}
			else if(!strcmp(measurement, "AnalyzeFFT"))
			{
				if(!strcmp(parameter, "vsaNum"))
				{
					if( (IQV_VSA_NUM_ENUM)value >= IQV_VSA_NUM_1 &&
							(IQV_VSA_NUM_ENUM)value <= IQV_VSA_NUM_4)
					{
						analysisFFT->vsaNum = (IQV_VSA_NUM_ENUM)value;
					}
					else
					{
						err = ERR_ANALYSIS_INVALID_PARAM_VALUE;
					}

				}
				else
				{
					err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME;
				}
			}
			//iqapiAnalysisHT40
			else if(!strcmp(measurement, "AnalyzeHT40"))
			{
				if(!strcmp(parameter, "vsaNum"))
				{
					if( (IQV_VSA_NUM_ENUM)value >= IQV_VSA_NUM_1 &&
							(IQV_VSA_NUM_ENUM)value <= IQV_VSA_NUM_4)
					{
						analysisHT40->vsaNum = (IQV_VSA_NUM_ENUM)value;
					}
					else
					{
						err = ERR_ANALYSIS_INVALID_PARAM_VALUE;
					}

				}
				else
				{
					err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME;
				}
			}
			//iqapiAnalysisVHT80WideBand
			else if(!strcmp(measurement, "AnalyzeVHT80Mask"))
			{
				if(!strcmp(parameter, "vsaNum"))
				{
					if( (IQV_VSA_NUM_ENUM)value >= IQV_VSA_NUM_1 &&
							(IQV_VSA_NUM_ENUM)value <= IQV_VSA_NUM_4)
					{
						analysisVHT80->vsaNum = (IQV_VSA_NUM_ENUM)value;
					}
					else
					{
						err = ERR_ANALYSIS_INVALID_PARAM_VALUE;
					}

				}
				else
				{
					err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME;
				}
			}
			else if(!strcmp(measurement, "AnalyzeCCDF"))
			{
				if(!strcmp(parameter, "vsaNum"))
				{
					if( (IQV_VSA_NUM_ENUM)value >= IQV_VSA_NUM_1 &&
							(IQV_VSA_NUM_ENUM)value <= IQV_VSA_NUM_4)
					{
						analysisCCDF->vsaNum = (IQV_VSA_NUM_ENUM)value;
					}
					else
					{
						err = ERR_ANALYSIS_INVALID_PARAM_VALUE;
					}
				}
				else
				{
					err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME;
				}
			}
			else if(!strcmp(measurement, "Analyze80211n"))
			{
				if(!strcmp(parameter, "useAllSignals"))
				{
					if(0!= value && 1!= value)
					{
						err = ERR_ANALYSIS_INVALID_PARAM_VALUE;
					}
					else
					{
						analysisMimo->useAllSignals = (int)value;
					}
				}
				else if(!strcmp(parameter, "frequencyCorr"))
				{
					if(2!= value && 3!= value && 4!= value)
					{
						err = ERR_ANALYSIS_INVALID_PARAM_VALUE;
					}
					else
					{
						analysisMimo->frequencyCorr = (int)value;
					}
				}
				else
				{
					err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME;
				}

			}
			else if(!strcmp(measurement, "Analyze80211ac"))
			{
				if(!strcmp(parameter, "useAllSignals"))
				{
					if(0!= value && 1!= value)
					{
						err = ERR_ANALYSIS_INVALID_PARAM_VALUE;
					}
					else
					{
						//temp,  Z-99
						//analysisWifiOfdm->useAllSignals = (int)value;
					}
				}
				else if(!strcmp(parameter, "frequencyCorr"))
				{
					if(2!= value && 3!= value && 4!= value)
					{
						err = ERR_ANALYSIS_INVALID_PARAM_VALUE;
					}
					else
					{
						//analysis11ac->frequencyCorr = (int)value;
						analysisWifiOfdm->frequencyCorr = (IQV_FREQUENCY_CORR)value;
					}
				}
				else
				{
					err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME;
				}

			}
			else
			{
				err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME; //??

			}
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SetAnalysisParameterInteger", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetAnalysisParameterInteger", timeDuration);

	return err;

}

/*! @defgroup group_analysis_name Available Analysis Name for LP_SetAnalysisParameterIntegerArray()
 *
 * Available analysis name vary for various analysis.  Before doing analysis action, by calling
 * one of the following functions:
 *   - LP_AnalyzeMimo();
 *   - LP_AnalyzePower();
 *   - LP_AnalyzeFFT();
 *   - LP_AnalyzeCCDF();
 *
 */
IQMEASURE_API int LP_SetAnalysisParameterIntegerArray(char *measurement, char *parameter, int *value, int valuesize)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetAnalysisParameterIntegerArray", &timeStart);

	if (LibsInitialized)
	{

		if (NULL==analysis80216 &&
				NULL==analysisMimo &&
				NULL==analysisOfdm &&
				NULL==analysis11b &&
				NULL==analysisPowerRampOfdm &&
				NULL==analysisPowerRamp11b &&
				NULL==analysisCW &&
				NULL==analysisWave &&
				NULL==analysisSidelobe &&
				NULL==analysisCCDF &&
				NULL==analysisFFT &&
				NULL==analysisPower &&
				NULL==analysisBluetooth &&
				NULL==analysisZigbee)
		{
			err = ERR_ANALYSIS_NULL_POINTER;
		}
		else
		{
			if(!strcmp(measurement, "Analyze80211n"))
			{
				if(!strcmp(parameter, "prefOrderSignals"))
				{
					for(int i=0;i<valuesize;i++)
					{
						analysisMimo->prefOrderSignals[i] = value[i];
					}

				}
				else
				{
					err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME;
				}

			}
			else if(!strcmp(measurement, "Analyze80211ac"))
			{
				if(!strcmp(parameter, "prefOrderSignals"))
				{
					for(int i=0;i<valuesize;i++)
					{
						//temp,  Z-99
						//analysisWifiOfdm->prefOrderSignals[i] = value[i];
					}

				}
				else
				{
					err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME;
				}

			}
			else
			{
				err = ERR_ANALYSIS_UNSUPPORTED_PARAM_NAME; //??
			}
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SetAnalysisParameterIntegerArray", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetAnalysisParameterIntegerArray", timeDuration);


	return err;

}


IQMEASURE_API int LP_CopyVsaCaptureData(int fromVsaNum, int toVsaNum)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_CopyVsaCaptureData", &timeStart);

	if (LibsInitialized)
	{
		if( (IQV_VSA_NUM_ENUM)fromVsaNum >= IQV_VSA_NUM_1 &&
				(IQV_VSA_NUM_ENUM)fromVsaNum <= IQV_VSA_NUM_4 &&
				(IQV_VSA_NUM_ENUM)toVsaNum >= IQV_VSA_NUM_1 &&
				(IQV_VSA_NUM_ENUM)toVsaNum <= IQV_VSA_NUM_4)
		{
			if( NULL!=hndl->data &&
					hndl->data->length[toVsaNum-1]>0 &&
					hndl->data->length[toVsaNum-1]==hndl->data->length[fromVsaNum-1])
			{
				memcpy(hndl->data->real[toVsaNum-1], hndl->data->real[fromVsaNum-1], hndl->data->length[fromVsaNum-1]*sizeof(double));
			}
			else
			{
				err = ERR_NO_CAPTURE_DATA;
			}
		}
		else
		{
			err = ERR_VSA_NUM_OUT_OF_RANGE;
		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	::TIMER_StopTimer(timerIQmeasure, "LP_CopyVsaCaptureData", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_CopyVsaCaptureData", timeDuration);


	return err;
}

IQMEASURE_API int LP_SaveSignalArrays(double *real[N_MAX_TESTERS],
		double *imag[N_MAX_TESTERS],
		int length[N_MAX_TESTERS],
		double sampleFreqHz[N_MAX_TESTERS],
		char *fileNameToSave)
{
	int err = ERR_OK;

	iqapiCapture *capture = new iqapiCapture();

	for(int i=0; i<N_MAX_TESTERS; i++)
	{
		capture->real[i] = real[i];
		capture->imag[i] = imag[i];
		capture->length[i] = length[i];
		capture->sampleFreqHz[i] = sampleFreqHz[i];
	}

	// Just load to iqapiHandle if the file name is null
	if(fileNameToSave == NULL)
	{
		if(hndl->data != NULL)
		{
			delete hndl->data;
		}

		hndl->data = capture;
	}
	// If file name is there, save it to the file
	else
	{
		if(capture->Save(fileNameToSave))
		{
			err = ERR_SAVE_WAVE_FAILED;
		}
	}

	return err;
}

IQMEASURE_API int LP_SaveIQDataToModulationFile(double *real,
		double *imag,
		int length[N_MAX_TESTERS],
		char *modFileName,
		int  normalization,
		int loadIQDataToVsg)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SaveIQDataToModulationFile", &timeStart);

	if (LibsInitialized)
	{
		// iqData is used for its SaveNormalize()
		// iqWave is used for setting the IQ data to VSG memory
		iqapiCapture        *iqData = new iqapiCapture();
		iqapiModulationWave *iqWave = new iqapiModulationWave();

		int offset = 0;
		for(int i=0; i<N_MAX_TESTERS; i++)
		{
			if(length[i]>0)
			{
				iqData->real[i]         = real + offset;
				iqData->imag[i]         = imag + offset;
				iqWave->real[i]         = real + offset;
				iqWave->imag[i]         = imag + offset;
			}
			else
			{
				iqData->real[i]         = NULL;
				iqData->imag[i]         = NULL;
				iqWave->real[i]         = NULL;
				iqWave->imag[i]         = NULL;
			}
			iqData->length[i]       = length[i];
			iqWave->length[i]       = length[i];
			iqData->sampleFreqHz[i] = 80e6;
			iqWave->sampleFreqHz[i] = 80e6;

			offset += length[i];
		}

		if( NULL!=modFileName && strlen(modFileName)>0 )
		{
			if(0==normalization)
			{
				err = iqData->Save( modFileName );
			}
			else
			{
				err = iqData->SaveNormalize( modFileName );
			}
			if(ERR_OK!=err)
			{
				err = ERR_SAVE_WAVE_FAILED;
			}
			else
			{
				// nothing to be done
			}
		}
		else
		{
			// nothing to be done
		}

		if( 0!=loadIQDataToVsg )
		{
			err = hndl->SetWave( iqWave );
			if(ERR_OK!=err)
			{
				err = ERR_SET_WAVE_FAILED;
			}
			else
			{
				// nothing to be done
			}
		}
		else
		{
			// nothing to be done
		}

		// We have to set all pointers back to NULL, otherwise the destructor will try to
		// free the memory which does not belong to it.
		for(int i=0; i<N_MAX_TESTERS; i++)
		{
			iqData->real[i]         = NULL;
			iqData->imag[i]         = NULL;
			iqData->length[i]       = 0;
			iqWave->real[i]         = NULL;
			iqWave->imag[i]         = NULL;
			iqWave->length[i]       = 0;
		}
		delete iqData;
		delete iqWave;
		iqData = NULL;
		iqWave = NULL;
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SaveIQDataToModulationFile", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SaveIQDataToModulationFile", timeDuration);

	return err;
}


IQMEASURE_API int LP_SaveVsaSignalFile(char *sigFileName)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SaveVsaSignalFile", &timeStart);

	if (LibsInitialized)
	{

		if(NULL!=hndl->data)
		{
			//z-99 no data filed in IQapi now. need to improve
			//if (hndl->data->Save(sigFileName))
			if (hndl->SaveSignalFile(sigFileName))
				err = ERR_SAVE_WAVE_FAILED;
		}
		else
		{
			err = ERR_NO_CAPTURE_DATA;
		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	::TIMER_StopTimer(timerIQmeasure, "LP_SaveVsaSignalFile", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SaveVsaSignalFile", timeDuration);

	return err;
}

IQMEASURE_API int LP_SaveVsaSignalFileText(char *txtFileName)
{
	int err = ERR_OK;
	char tmpbuff[MAX_PATH];
	FILE *fp;

	TIMER_StartTimer(timerIQmeasure, "LP_SaveVsaSignalFileText", &timeStart);

	if (LibsInitialized)
	{
		if(NULL!=hndl->data)
		{

			sprintf_s(tmpbuff, MAX_PATH, "%s", txtFileName);
			err = fopen_s(&fp, tmpbuff, "w");
			if (err == 0)
			{
				for(int i = 0; i < hndl->data->length[0]; i++)
				{
					fprintf(fp, "%15e %15e\n", hndl->data->real[0][i], hndl->data->imag[0][i]);	// print to debug log file
				}

				fclose(fp);
			}
			else
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_ERROR, "[IQMEASURE],[%s],\nfopen_s \"%s\" erred with code: %i\n", "LP_SaveVsaSignalFileText", tmpbuff, err);
				err = ERR_SAVE_WAVE_FAILED;
			}
		}
		else
		{
			err = ERR_NO_CAPTURE_DATA;
		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	::TIMER_StopTimer(timerIQmeasure, "LP_SaveVsaSignalFileText", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SaveVsaSignalFileText", timeDuration);


	return err;
}


IQMEASURE_API int LP_PlotDataCapture()
{
	int err = ERR_OK;
	int i;

	::TIMER_StartTimer(timerIQmeasure, "LP_PlotDataCapture", &timeStart);

	if (LibsInitialized)
	{
		for(i=0; i<hndl->nTesters; i++)
		{
			hndl->data->PlotPower(i+1, "", i);
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_PlotDataCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_PlotDataCapture", timeDuration);

	return err;
}

IQMEASURE_API int LP_Plot(int figNum, double *x, double *y, int length, char *plotArgs, char *title, char *xtitle, char *ytitle, int keepPlot)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_Plot", &timeStart);

	if (LibsInitialized)
	{
		iqapiPlot(figNum, x, y, length, plotArgs, title, xtitle, ytitle, keepPlot);
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_Plot", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_Plot", timeDuration);

	return err;
}

IQMEASURE_API int LP_SaveVsaGeneratorFile(char *modFileName)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SaveVsaGeneratorFile", &timeStart);

	if (LibsInitialized)
	{
		if (hndl->data->SaveNormalize(modFileName))
			err = ERR_SAVE_WAVE_FAILED;
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SaveVsaGeneratorFile", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SaveVsaGeneratorFile", timeDuration);

	return err;
}
IQMEASURE_API int LP_LoadVsaSignalFile(char *sigFileName)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_LoadVsaSignalFile", &timeStart);

	if (LibsInitialized)
	{
		if (hndl->LoadSignalFile(sigFileName))
			err = ERR_LOAD_WAVE_FAILED;
		//either work around or change the structure of IQmeasure, don't check data length anymore, put weird value to stand out from now capture, Z-99
		else
		{
			hndl->data->length[0]=99;
			hndl->data->length[1]=99;
			hndl->data->length[2]=99;
			hndl->data->length[3]=99;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_LoadVsaSignalFile", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_LoadVsaSignalFile", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetVsaBluetooth(double rfFreqHz, double rfAmplDb, int port, double triggerLevelDb, double triggerPreTime)
{
	int    err = ERR_OK;
	bool   setupChanged = false;
	bool   RFused = true;
	double extAttenDb = 0;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsaBluetooth", &timeStart);

	if (LibsInitialized)
	{
		if (hndl->rx->rfFreqHz != (rfFreqHz+BT_SHIFT_HZ) || (hndl->tx->rfFreqHz != (rfFreqHz + BT_SHIFT_HZ))
				|| (hndl->rx->freqShiftHz != BT_SHIFT_HZ) || (hndl->tx->freqShiftHz != -BT_SHIFT_HZ)
				|| g_bDisableFreqFilter == true )
		{
			hndl->rx->rfFreqHz = rfFreqHz + BT_SHIFT_HZ;
			hndl->tx->rfFreqHz = rfFreqHz + BT_SHIFT_HZ;
			hndl->rx->freqShiftHz = BT_SHIFT_HZ;
			hndl->tx->freqShiftHz = -BT_SHIFT_HZ;

			//hndl->rx->rfFreqHz = rfFreqHz;
			//hndl->tx->rfFreqHz = rfFreqHz;
			//hndl->rx->freqShiftHz = 0;
			//hndl->tx->freqShiftHz = 0;

			setupChanged = true;
			g_bDisableFreqFilter = false;
		}
		if (hndl->rx->triggerPreTime != triggerPreTime)
		{
			hndl->rx->triggerPreTime = triggerPreTime;
			setupChanged = true;
		}
		if (hndl->rx->triggerLevelDb != triggerLevelDb)
		{
			hndl->rx->triggerLevelDb = triggerLevelDb;
			setupChanged = true;
		}
		if (hndl->rx->rxMode == IQV_INPUT_ADC_BB && port != PORT_BB)
		{
			hndl->rx->rxMode = IQV_INPUT_ADC_RF;
			RFused = true;
			setupChanged = true;
		}

		if (hndl->rx->rxMode != IQV_INPUT_ADC_BB && port == PORT_BB)
		{
			hndl->rx->rxMode = IQV_INPUT_ADC_BB;
			//		port = (int)PORT_LEFT;
			RFused = false;
			setupChanged = true;
		}
		for (int i=0; i<nTesters; i++)
		{
			//		if (hndl->rx->rxMode == IQV_INPUT_ADC_BB && port != PORT_BB)
			//		{
			//			hndl->rx->rxMode = IQV_INPUT_ADC_RF;
			//			RFused = true;
			//			setupChanged = true;
			//		}

			//			if (hndl->rx->rxMode != IQV_INPUT_ADC_BB && port == PORT_BB)
			//			{
			//				hndl->rx->rxMode = IQV_INPUT_ADC_BB;
			//			port = (int)PORT_LEFT;
			//				RFused = false;
			//				setupChanged = true;
			//			}

			if (RFused && hndl->rx->vsa[i]->port != (IQV_PORT_ENUM)port)
			{
				if ((IQV_PORT_ENUM)port == IQV_PORT_LEFT)
				{
					hndl->rx->vsa[i]->port = IQV_PORT_LEFT;
					hndl->tx->vsg[i]->port = IQV_PORT_RIGHT;
					setupChanged = true;
				}
				else if ((IQV_PORT_ENUM)port == IQV_PORT_RIGHT)
				{
					hndl->rx->vsa[i]->port = IQV_PORT_RIGHT;
					hndl->tx->vsg[i]->port = IQV_PORT_LEFT;
					setupChanged = true;
				}
				else
				{
					hndl->rx->vsa[i]->port = (IQV_PORT_ENUM)port;
					setupChanged = true;
				}
			}

			if (hndl->rx->vsa[i]->extAttenDb != extAttenDb)
			{
				hndl->rx->vsa[i]->extAttenDb = extAttenDb;
				setupChanged = true;
			}

			// Workaround due to IQxel can not set reference level lower than -20 dBm
			if (rfAmplDb < -20)
				rfAmplDb = -20;
			if ( abs(hndl->rx->vsa[i]->rfAmplDb-rfAmplDb)>abs(g_amplitudeToleranceDb) )
				//if (hndl->rx->vsa[i]->rfAmplDb != rfAmplDb)
			{
				if(RFused)
					hndl->rx->vsa[i]->rfAmplDb = rfAmplDb;
				else
				{
					hndl->rx->vsa[i]->bbAmplDbv = rfAmplDb;
					hndl->rx->vsa[i]->bbGainDb = -1 * rfAmplDb;
				}
				setupChanged = true;
			}
		}

		if (setupChanged)
		{
			if (hndl->SetTxRx())
				err = ERR_SET_RX_FAILED;
		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsaBluetooth", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetVsaBluetooth", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetVsaNxN(double rfFreqHz,
		double rfAmplDb[],
		int port[],
		double extAttenDb,
		double triggerLevelDb,
		double triggerPreTime,
		double dFreqShiftHz)
{
	for( int i=0; i<N_MAX_TESTERS; i++)
	{
		g_IQnxnSettings[i].vsaAmplDBm	= rfAmplDb[i];
		g_IQnxnSettings[i].vsaPort		= (IQV_PORT_ENUM)port[i];
	}

	//Remembers NxN function is called
	g_nxnFunctionCalled = true;

	return LP_SetVsa(rfFreqHz, rfAmplDb[0], port[0], extAttenDb, triggerLevelDb, triggerPreTime, dFreqShiftHz);
}

IQMEASURE_API int LP_SetVsa(double rfFreqHz, double rfAmplDb, int port, double extAttenDb, double triggerLevelDb, double triggerPreTime, double dFreqShiftHz)
{
	int err = ERR_OK;
	bool setupChanged = false;
	bool RFused = true;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsa", &timeStart);

	if (LibsInitialized)
	{
		if( g_previousOperationIsRX && nTesters>1 && bIQxsFound == false && bIQxelFound == false )
		{
			//TODO: this is to work around the problem on IQnxn that TX stops working after RX
			hndl->SetDefault();
			g_previousOperationIsRX = false;
		}

		if ( (hndl->rx->rfFreqHz!=rfFreqHz)||(hndl->rx->freqShiftHz!=dFreqShiftHz)||(hndl->tx->rfFreqHz!=rfFreqHz) || g_bDisableFreqFilter == true)
		{
			hndl->rx->rfFreqHz = rfFreqHz;
			hndl->tx->rfFreqHz = rfFreqHz;
			hndl->rx->freqShiftHz = dFreqShiftHz;
			hndl->tx->freqShiftHz = dFreqShiftHz;
			setupChanged = true;
			g_bDisableFreqFilter = false;
		}

		if (hndl->rx->triggerPreTime != triggerPreTime)
		{
			hndl->rx->triggerPreTime = triggerPreTime;
			setupChanged = true;
		}
		if (hndl->rx->triggerLevelDb != triggerLevelDb)
		{
			hndl->rx->triggerLevelDb = triggerLevelDb;
			setupChanged = true;
		}
		if (hndl->rx->rxMode == IQV_INPUT_ADC_BB && port != PORT_BB)
		{
			hndl->rx->rxMode = IQV_INPUT_ADC_RF;
			RFused = true;
			setupChanged = true;
		}

		if (hndl->rx->rxMode != IQV_INPUT_ADC_BB && port == PORT_BB)
		{
			hndl->rx->rxMode = IQV_INPUT_ADC_BB;
			//		port = (int)PORT_LEFT;
			RFused = false;
			setupChanged = true;
		}
		for (int i=0; i<nTesters; i++)
		{
			//		if (hndl->rx->rxMode == IQV_INPUT_ADC_BB && port != PORT_BB)
			//		{
			//			hndl->rx->rxMode = IQV_INPUT_ADC_RF;
			//			RFused = true;
			//			setupChanged = true;
			//		}

			//			if (hndl->rx->rxMode != IQV_INPUT_ADC_BB && port == PORT_BB)
			//			{
			//				hndl->rx->rxMode = IQV_INPUT_ADC_BB;
			//			port = (int)PORT_LEFT;
			//				RFused = false;
			//				setupChanged = true;
			//			}

			// For rest testers of IQnxn
			if( i>0 )
			{
				//If NxN function is called,     apply g_IQnxnSettings to testers.
				//If NxN function is not called, apply 1st tester setting to all testers
				if(g_nxnFunctionCalled)
				{
					port		= g_IQnxnSettings[i].vsaPort;
					rfAmplDb	= g_IQnxnSettings[i].vsaAmplDBm;
				}
				else
				{
					//apply 1st tester setting to all testers
				}
			}
			if (RFused && hndl->rx->vsa[i]->port != (IQV_PORT_ENUM)port)
			{
				if ((IQV_PORT_ENUM)port == IQV_PORT_LEFT)
				{
					hndl->rx->vsa[i]->port = IQV_PORT_LEFT;
					hndl->tx->vsg[i]->port = IQV_PORT_RIGHT;
					setupChanged = true;
				}
				else if ((IQV_PORT_ENUM)port == IQV_PORT_RIGHT)
				{
					hndl->rx->vsa[i]->port = IQV_PORT_RIGHT;
					hndl->tx->vsg[i]->port = IQV_PORT_LEFT;
					setupChanged = true;
				}
				else
				{
					hndl->rx->vsa[i]->port = (IQV_PORT_ENUM)port;
					setupChanged = true;
				}
			}

			if (hndl->rx->vsa[i]->extAttenDb != extAttenDb)
			{
				hndl->rx->vsa[i]->extAttenDb = extAttenDb;
				setupChanged = true;
			}

			bool rfAmplChanged = false;
			// Workaround due to IQxel can not set reference level lower than -20 dBm
			if (rfAmplDb < -20)
				rfAmplDb = -20;
			if (g_amplitudeToleranceDb>=0)
			{
				if (hndl->rx->vsa[i]->rfAmplDb-rfAmplDb > g_amplitudeToleranceDb || hndl->rx->vsa[i]->rfAmplDb < rfAmplDb )
				{
					rfAmplChanged = true;
				}else
				{
					rfAmplChanged = false;
				}
			}else //this should not happen but our software allow customer to enter negative values
			{
				if (hndl->rx->vsa[i]->rfAmplDb-rfAmplDb < g_amplitudeToleranceDb || hndl->rx->vsa[i]->rfAmplDb > rfAmplDb )
				{
					rfAmplChanged = true;
				}else
				{
					rfAmplChanged = false;
				}
			}
			if (true == rfAmplChanged)
			{
				if(RFused)
					hndl->rx->vsa[i]->rfAmplDb = rfAmplDb;
				else
				{
					hndl->rx->vsa[i]->bbAmplDbv = rfAmplDb;
					hndl->rx->vsa[i]->bbGainDb = -1 * rfAmplDb;
				}
				setupChanged = true;
			}
			//			hndl->tx->vsg[i]->source = IQV_SOURCE_WAVE;
		}

		if (setupChanged)
		{
			if (hndl->SetTxRx())
			{
				err = ERR_SET_RX_FAILED;
				//throw hndl->lastErr;
			}
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	//After dealing with NxN function, restore g_nxnFunctionCalled to false
	g_nxnFunctionCalled = false;

	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsa", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetVsa", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetVsaTriggerTimeout(double triggerTimeoutSec)
{
	int err = ERR_OK;

	TIMER_StartTimer(timerIQmeasure, "LP_SetVsaTriggerTimeout", &timeStart);

	if (LibsInitialized)
	{
		hndl->rx->triggerTimeOut = triggerTimeoutSec;
		if (hndl->SetTxRx())
		{
			err = ERR_SET_RX_FAILED;
		}
		else
		{
			err = ERR_OK;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}
	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsaTriggerTimeout", &timeDuration, &timeStop);
	return err;
}


IQMEASURE_API int LP_SetVsaAmplitudeTolerance(double amplitudeToleranceDb)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsaAmplitudeTolerance", &timeStart);

	g_amplitudeToleranceDb = amplitudeToleranceDb;

	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsaAmplitudeTolerance", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetVsaAmplitudeTolerance", timeDuration);

	return err;
}

IQMEASURE_API int LP_VsaDataCapture(double samplingTimeSecs, int triggerType, double sampleFreqHz,
		int captureType,IQMEASURE_CAPTURE_NONBLOCKING_STATES nonBlockingState)
{
	//ht40Mode meaning is changed as VHT80, VHT160 and CVHT80 mode is added.
	int err = ERR_OK;
	bool setupChanged = false;
	int  finalTriggerType = 0;

	::TIMER_StartTimer(timerIQmeasure, "LP_VsaDataCapture", &timeStart);

	//might need to specify the right one for bIQxsFound in the future.
	/* if (triggerType == IQV_TRIG_TYPE_IF2_NO_CAL && bIQ201xFound == true)				// if it is a IQ201x, then use IQ201x trigger
	   {
	   finalTriggerType = IQV_TRIG_TYPE_FOR_IQ2010_DIG;
	   }
	   else if (triggerType == IQV_TRIG_TYPE_FOR_IQ2010_DIG && bIQ201xFound == false)		// if it is not a 201x, then use back IQview trigger
	   {
	   finalTriggerType = IQV_TRIG_TYPE_IF2_NO_CAL;
	   }
	   else
	   {
	   finalTriggerType = triggerType;
	   }*/

	//Tracy Yu; 2012-03-22; For IQXel capture
	if (triggerType == IQV_TRIG_TYPE_IF2_NO_CAL && (bIQ201xFound == true || bIQxelFound == true))				// if it is a IQ201x, then use IQ201x trigger
	{
		finalTriggerType = IQV_TRIG_TYPE_FOR_IQ2010_DIG;
	}
	else if (triggerType == IQV_TRIG_TYPE_FOR_IQ2010_DIG && bIQ201xFound == false && bIQxelFound == false)		// if it is not a 201x, then use back IQview trigger
	{
		finalTriggerType = IQV_TRIG_TYPE_IF2_NO_CAL;
	}
	else
	{
		finalTriggerType = triggerType;
	}

	if ( LibsInitialized )
	{
		// This capture uses hndl->data
		if (hndl->rx->samplingTimeSecs != samplingTimeSecs)
		{
			hndl->rx->samplingTimeSecs = samplingTimeSecs;
			setupChanged = true;
		}

		if (hndl->rx->triggerType != (IQV_TRIG_TYPE_ENUM)finalTriggerType)
		{
			hndl->rx->triggerType = (IQV_TRIG_TYPE_ENUM)finalTriggerType;
			setupChanged = true;
		}

		//work around to handle 80MHz sampling rate.
		//force the sampling frequency to the right one in HT40 wideband to make sure that backward compatible.
		/*if( IQV_HT_40_CAPTURE_TYPE==captureType ||IQV_HT_40_WIDE_BAND_HI_ACCURACY_CAPTURE_TYPE==captureType || IQV_HT_40_WIDE_BAND_LO_ACCURACY_CAPTURE_TYPE == captureType)
		  {*/
		if (bIQxelFound==true )
		{
			sampleFreqHz=160e6;
		}else if (bIQxsFound==true)
		{
			sampleFreqHz=150e6;
		}
		else
		{
			//do nothing
		}
		//}
		if (hndl->rx->sampleFreqHz != sampleFreqHz)
		{
			hndl->rx->sampleFreqHz = sampleFreqHz;
			setupChanged = true;
		}

		if (setupChanged)
		{
			if (hndl->SetTxRx())
				err = ERR_SET_TX_FAILED;
		}

		if(hndl->GetCaptureDataHandling()!=IQV_DATA_IN_IQAPI) //there should be a smarter way to do this, revisit this later Z-99, March 23, 2012
		{
			hndl->SetCaptureDataHandling(IQV_DATA_IN_IQAPI);
		}
		// g_userData is used for selecting a portion of capture for analysis
		// Any new capture will reset this variable to NULL
		if( NULL!=g_userData )
		{
			for(int j = 0 ; j < MAX_TESTER_NUM ; j++){
				g_userData->real[j] = NULL;
				g_userData->imag[j] = NULL;
			}

			delete g_userData;
			g_userData = NULL;
		}

		//if( ON==captureType ) //need to add support for the other capture type when it is ready.
		if( IQV_HT_40_CAPTURE_TYPE==captureType )
		{
			if(bIQxsFound==true || bIQxelFound==true )
			{
				err = hndl->Capture();
			}
			else
			{
				err = hndl->Capture( IQV_HT_40_CAPTURE_TYPE );
			}

		}
		else
		{

			//if (ON_VHT80 == captureType )
			if (IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE == captureType )
			{
				err = hndl->Capture(IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE);
			}
			else
			{
				err = hndl->Capture();
			}
		}

		if (0!=err)
		{
			err = ERR_CAPTURE_FAILED;
		}
		else	// Copy hndl->data
		{
			// do nothing
		}

		if( IQV_HT_40_CAPTURE_TYPE==captureType || IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE==captureType ) //actually all capture don't have real data field for Daytona
		{
			//TODO: HT40 capture does not use hndl->data
			//if (!hndl->data)
			//	err = ERR_CAPTURE_FAILED;
			//else
			//{
			//	if (!hndl->data->length[0])
			//		err = ERR_CAPTURE_FAILED;
			//}
		}
		else
		{
			if (!hndl->data)
				err = ERR_CAPTURE_FAILED;
			else
			{
				if (!hndl->data->length[0])
					err = ERR_CAPTURE_FAILED;
			}
		}

	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_VsaDataCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_VsaDataCapture", timeDuration);

	return err;
}


IQMEASURE_API int LP_GetSampleData(int vsaNum, double bufferReal[], double bufferImag[], int bufferLength)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_GetSampleData", &timeStart);

	if (vsaNum < 0 || vsaNum > 3)
	{
		return ERR_VSA_NUM_OUT_OF_RANGE;
	}


	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[vsaNum])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		if (bufferLength > hndl->data->length[vsaNum])
			bufferLength = hndl->data->length[vsaNum];

		if (bufferLength >= 1)
		{
			memcpy(bufferReal, hndl->data->real[vsaNum], (sizeof(double)*bufferLength));
			memcpy(bufferImag, hndl->data->imag[vsaNum], (sizeof(double)*bufferLength));
		}
		else
		{
			err = ERR_GENERAL_ERR;
		}

	}
	else
		err = ERR_NOT_INITIALIZED;

	::TIMER_StopTimer(timerIQmeasure, "LP_GetSampleData", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_GetSampleData", timeDuration);

	return err;
}


IQMEASURE_API int LP_SelectCaptureRangeForAnalysis(double startPositionUs, double lengthUs,
												   int packetsOffset, int packetsLength)
{

	// note: packetsOffset and packetsLength are ignored for IQ2010/IQapi

	int err = ERR_OK;

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
		}
		else
		{
			int j = 0;
			if( NULL!=g_userData )
			{
				for(j = 0 ; j < MAX_TESTER_NUM ; j++){
					g_userData->real[j] = NULL;
					g_userData->imag[j] = NULL;
				}
				delete g_userData;
				g_userData = NULL;
			}
			g_userData = new iqapiCapture();

			int i = 0;
			int	startOffsetArray[MAX_TESTER_NUM] = {0};
			int	lengthArray[MAX_TESTER_NUM] = {0};
			for(i = 0 ; i < MAX_TESTER_NUM ; i++){
				if(!hndl->data->length[i]){
					err = ERR_NO_CAPTURE_DATA;
				}else{
					g_userData->sampleFreqHz[i] = hndl->data->sampleFreqHz[i];
					startOffsetArray[i] = (int)(g_userData->sampleFreqHz[i]*1.0e-6*startPositionUs);
					lengthArray[i]      = (int)(g_userData->sampleFreqHz[i]*1.0e-6*lengthUs);

					if( startOffsetArray[i] >= hndl->data->length[i] || (startOffsetArray[i]+lengthArray[i]) > hndl->data->length[i] )
					{
						err = ERR_INVALID_DATA_CAPTURE_RANGE;
					}
					else
					{
						g_userData->real[i]   = hndl->data->real[i] + startOffsetArray[i];
						g_userData->imag[i]   = hndl->data->imag[i] + startOffsetArray[i];
						g_userData->length[i] = lengthArray[i];

					}
				}
			}
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	return err;
}

IQMEASURE_API int LP_SaveTruncateCapture(char *sigFileName)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SaveTruncateCapture", &timeStart);

	if (LibsInitialized)
	{
		if(0 != g_userData)
		{
			if(0 !=g_userData->length[0])
			{
				if (g_userData->Save(sigFileName))
					err = ERR_SAVE_WAVE_FAILED;
			}
			else
			{
				err = ERR_NO_CAPTURE_DATA;
			}
		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	::TIMER_StopTimer(timerIQmeasure, "LP_SaveTruncateCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SaveTruncateCapture", timeDuration);

	return err;
}


IQMEASURE_API int LP_Analyze80216e( double sigType		,
		double bandwidthHz	,
		double cyclicPrefix	,
		double rateId		,
		double numSymbols	,
		int ph_corr     ,
		int ch_corr     ,
		int freq_corr   ,
		int timing_corr ,
		int ampl_track  ,
		double decode   ,
		char *map_conf_file
		)
{

	int err = ERR_OK;


	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		analysis80216->type = "80216e-2005";
		analysis80216->mode = "80216e-2005";
		analysis80216->mapConfigFile = map_conf_file;

	}

	return LP_Analyze80216d(sigType, bandwidthHz, cyclicPrefix, rateId, numSymbols, ph_corr, ch_corr, freq_corr, timing_corr, ampl_track, decode);
}


IQMEASURE_API int LP_Analyze80216d( double sigType		,
		double bandwidthHz	,
		double cyclicPrefix	,
		double rateId		,
		double numSymbols	,
		int ph_corr     ,
		int ch_corr     ,
		int freq_corr   ,
		int timing_corr ,
		int ampl_track  ,
		double decode
		)
{

	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_Analyze80216", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		analysis80216->Phy.sigType		= sigType;
		analysis80216->Phy.bandwidthHz	= bandwidthHz;
		analysis80216->Phy.cyclicPrefix = cyclicPrefix;
		analysis80216->Phy.rateId		= rateId;
		analysis80216->Phy.numSymbols	= numSymbols;
		analysis80216->loggingMode		= 0;	// disable message from analysis API

		analysis80216->Acq.phaseCorrect		= (IQV_PH_CORR_ENUM) ph_corr;
		analysis80216->Acq.channelCorrect	= (IQV_CH_EST_ENUM) ch_corr;
		analysis80216->Acq.freqCorrect		= (IQV_FREQ_SYNC_ENUM) freq_corr;
		analysis80216->Acq.timingCorrect	= (IQV_SYM_TIM_ENUM) timing_corr;
		analysis80216->Acq.amplitudeTrack	= (IQV_AMPL_TRACK_ENUM) ampl_track;
		analysis80216->Dec.decode			= decode;

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysis80216);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_Analyze80216", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_Analyze80216", timeDuration);


	return err;

}

IQMEASURE_API int LP_Analyze80211p(int ph_corr_mode, int ch_estimate, int sym_tim_corr, int freq_sync, int ampl_track, int ofdm_mode)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_Analyze80211p", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		analysisOfdm->ph_corr_mode = (IQV_PH_CORR_ENUM) ph_corr_mode;
		analysisOfdm->ch_estimate = (IQV_CH_EST_ENUM) ch_estimate;
		analysisOfdm->sym_tim_corr = (IQV_SYM_TIM_ENUM) sym_tim_corr;
		analysisOfdm->freq_sync = (IQV_FREQ_SYNC_ENUM) freq_sync;
		analysisOfdm->ampl_track = (IQV_AMPL_TRACK_ENUM) ampl_track;
		analysisOfdm->OFDM_mode = (IQV_OFDM_MODE_ENUM)ofdm_mode;


		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisOfdm);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_Analyze80211p", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_Analyze80211p", timeDuration);


	return err;
}

IQMEASURE_API int LP_Analyze80211ag(int ph_corr_mode, int ch_estimate, int sym_tim_corr, int freq_sync, int ampl_track,double prePowStartSec,
		double prePowStopSec)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_Analyze80211ag", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		analysisOfdm->ph_corr_mode = (IQV_PH_CORR_ENUM) ph_corr_mode;
		analysisOfdm->ch_estimate = (IQV_CH_EST_ENUM) ch_estimate;
		analysisOfdm->sym_tim_corr = (IQV_SYM_TIM_ENUM) sym_tim_corr;
		analysisOfdm->freq_sync = (IQV_FREQ_SYNC_ENUM) freq_sync;
		analysisOfdm->ampl_track = (IQV_AMPL_TRACK_ENUM) ampl_track;
		analysisOfdm->prePowStartSec = prePowStartSec;
		analysisOfdm->prePowStopSec = prePowStopSec;
		//Workaround due to IQapi bug
		hndl->ScpiCommandSet("WIFI;Clear:All");

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisOfdm);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_Analyze80211ag", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_Analyze80211ag", timeDuration);

	return err;
}

// LP_AnalyzeMimo kept for backward compability
IQMEASURE_API int LP_AnalyzeMimo(char *type,
		char *mode,
		int enablePhaseCorr,
		int enableSymTimingCorr,
		int enableAmplitudeTracking,
		int decodePSDU,
		int enableFullPacketChannelEst,
		char *referenceFile)
{
	return LP_Analyze80211n(type,
			mode,
			enablePhaseCorr,
			enableSymTimingCorr,
			enableAmplitudeTracking,
			decodePSDU,
			enableFullPacketChannelEst,
			referenceFile);
}

IQMEASURE_API int LP_Analyze80211n(char *type,
		char *mode,
		int enablePhaseCorr,
		int enableSymTimingCorr,
		int enableAmplitudeTracking,
		int decodePSDU,
		int enableFullPacketChannelEst,
		char *referenceFile,
		int packetFormat,
		int frequencyCorr,
		double prePowStartSec,
		double prePowStopSec)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzeMimo", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		if( NULL==analysisMimo )
		{
			InstantiateAnalysisClasses();
		}
		//analysisMimo->type = type; //temporarily unsupported, IQapi should fix this.
		analysisMimo->mode = mode;
		analysisMimo->enablePhaseCorr = enablePhaseCorr;
		analysisMimo->enableSymTimingCorr = enableSymTimingCorr;
		analysisMimo->enableAmplitudeTracking = enableAmplitudeTracking;
		analysisMimo->decodePSDU = decodePSDU;
		analysisMimo->enableFullPacketChannelEst = enableFullPacketChannelEst;
		analysisMimo->frequencyCorr = frequencyCorr;
		analysisMimo->referenceFile = referenceFile;
		analysisMimo->packetFormat = packetFormat;
		analysisMimo->prePowStartSec = prePowStartSec;
		analysisMimo->prePowStopSec = prePowStopSec;
		//debug z-99, work around
		//analysisMimo->packetFormat = IQV_PACKET_FORMAT_AUTO;
		//if( 0==strcmp("sequential_mimo",mode) )
		//{
		//	analysisMimo->SequentialMimo.numSections = g_numSections;
		//	analysisMimo->SequentialMimo.sectionLenSec = g_sectionLenSec;
		//	analysisMimo->SequentialMimo.interSectionGapSec = 0;
		//}

		//Workaround due to IQapi bug
		hndl->ScpiCommandSet("WIFI;Clear:All");
		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisMimo);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{
			// MIMO analysis succeeded

			resultMimo = dynamic_cast<iqapiResultMimo *>(hndl->results);

			if( NULL!=resultMimo )
			{
				g_lastPerformedAnalysisType = ANALYSIS_MIMO;
				// Measurement results are ready for retrieval
			}

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzeMimo", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzeMimo", timeDuration);

	return err;
}

IQMEASURE_API int LP_Analyze80211ac(char *mode,
		int enablePhaseCorr,
		int enableSymTimingCorr,
		int enableAmplitudeTracking,
		int decodePSDU,
		int enableFullPacketChannelEst,
		int frequencyCorr,
		char *referenceFile,
		int packetFormat,
		int numberOfPacketToAnalysis,
		double prePowStartSec,
		double prePowStopSec)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_Analyze80211ac", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		if( NULL==analysisWifiOfdm )
		{
			InstantiateAnalysisClasses();
		}

		// no need to set the type anymore as 11ac only has one type.
		//analysisWifiOfdm->type = type;
		analysisWifiOfdm->mode = mode;
		analysisWifiOfdm->enablePhaseCorr = (IQV_ON_OFF) enablePhaseCorr;
		analysisWifiOfdm->enableSymTimingCorr = (IQV_ON_OFF) enableSymTimingCorr;
		analysisWifiOfdm->enableAmplitudeTracking = (IQV_ON_OFF)enableAmplitudeTracking;
		analysisWifiOfdm->numberOfPacketToAnalysis = numberOfPacketToAnalysis;
		//temp,  Z-99
		//analysisWifiOfdm->decodePSDU = decodePSDU;
		analysisWifiOfdm->enableFullPacketChannelEst = (IQV_CHAN_ESTIMATION) enableFullPacketChannelEst;
		// Work around for Freq Corr Enum
		if(frequencyCorr == 1)
				analysisWifiOfdm->frequencyCorr =  (IQV_FREQUENCY_CORR)(0);
		if(frequencyCorr == 2)
				analysisWifiOfdm->frequencyCorr =  (IQV_FREQUENCY_CORR)(1);
		if(frequencyCorr == 3)
				analysisWifiOfdm->frequencyCorr =  (IQV_FREQUENCY_CORR)(3);
		if(frequencyCorr == 4)
				analysisWifiOfdm->frequencyCorr =  (IQV_FREQUENCY_CORR)(2);
		
		//analysisWifiOfdm->frequencyCorr = (IQV_FREQUENCY_CORR)(frequencyCorr); 
		analysisWifiOfdm->referenceFile = referenceFile;
		analysisWifiOfdm->packetFormat = (IQV_PACKET_FORMAT)packetFormat;
		analysisWifiOfdm->prePowStartSec = prePowStartSec;
		analysisWifiOfdm->prePowStopSec = prePowStopSec;
		//if( 0==strcmp("sequential_mimo",mode) )
		//{
		//	analysisWifiOfdm->SequentialMimo.numSections = g_numSections;
		//	analysisWifiOfdm->SequentialMimo.sectionLenSec = g_sectionLenSec;
		//	analysisWifiOfdm->SequentialMimo.interSectionGapSec = 0;
		//}

		//err=hndl->LoadSignalFile("./log/FastTrack_Capture-WIFI_AC_BW80_SS1_MCS4_BCC_Fs160M-1.iqvsa");
		//Workaround due to IQapi bug
		hndl->ScpiCommandSet("WIFI;Clear:All");
		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisWifiOfdm);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{
			// 11ac analysis succeeded

			//Z-99
			//result11ac = dynamic_cast<iqapiResult11ac *>(hndl->results);
			result11ac = dynamic_cast<iqapiResultWifiOfdm *>(hndl->results);

			if( NULL!=result11ac )
			{
				g_lastPerformedAnalysisType = ANALYSIS_80211AC;
				// Measurement results are ready for retrieval
			}

		}

	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_Analyze80211ac", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_Analyze80211ac", timeDuration);

	return err;
}

IQMEASURE_API int LP_Analyze80211b(int eq_taps, int DCremove11b_flag, int method_11b, double prePowStartSec,
		double prePowStopSec)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_Analyze80211b", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		if (!analysis11b) {
			return ERR_ANALYSIS_FAILED;
		}

		analysis11b->eq_taps = (IQV_EQ_ENUM) eq_taps;
		analysis11b->DCremove11b_flag = (IQV_DC_REMOVAL_ENUM) DCremove11b_flag;
		analysis11b->method_11b = (IQV_11B_METHOD_ENUM) method_11b;
		analysis11b->prePowStartSec = prePowStartSec;
		analysis11b->prePowStopSec = prePowStopSec;


		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysis11b);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_Analyze80211b", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_Analyze80211b", timeDuration);

	return err;
}

IQMEASURE_API int LP_AnalyzePower(double T_interval, double max_pow_diff_dB)
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzePower", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		//analysisPower->T_interval = T_interval;
		//analysisPower->max_pow_diff_dB = max_pow_diff_dB;


		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisPower);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzePower", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzePower", timeDuration);


	return err;
}

IQMEASURE_API int LP_AnalyzeFFT(double NFFT, double res_bw, char *window_type)
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzeFFT", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		//analysisFFT->F_sample = hndl->rx->sampleFreqHz;
		analysisFFT->NFFT = NFFT;
		analysisFFT->res_bw = res_bw;
		analysisFFT->window_type = window_type;

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisFFT);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{
			g_lastPerformedAnalysisType = ANALYSIS_FFT;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzeFFT", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzeFFT", timeDuration);


	return err;
}

IQMEASURE_API int LP_AnalyzeCCDF()
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzeCCDF", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisCCDF);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzeCCDF", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzeCCDF", timeDuration);


	return err;
}
// Kept for backward compability
IQMEASURE_API int LP_AnalyzeCW()
{
	return LP_AnalyzeCWFreq();
}

IQMEASURE_API int LP_AnalyzeCWFreq()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzeCW", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}


		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisCW);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzeCW", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzeCW", timeDuration);


	return err;
}

IQMEASURE_API int LP_AnalyzePowerRamp80211b()
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzePowerRamp80211b", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisPowerRamp11b);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzePowerRamp80211b", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzePowerRamp80211b", timeDuration);


	return err;
}

IQMEASURE_API int LP_AnalyzePowerRampOFDM()
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzePowerRampOFDM", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisPowerRampOfdm);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzePowerRampOFDM", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzePowerRampOFDM", timeDuration);


	return err;
}
IQMEASURE_API int LP_AnalyzeSidelobe()
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzeSidelobe", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisSidelobe);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzeSidelobe", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzeSidelobe", timeDuration);


	return err;
}

IQMEASURE_API int LP_AnalysisWave()
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalysisWave", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisWave);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalysisWave", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalysisWave", timeDuration);

	return err;
}

IQMEASURE_API int LP_AnalyzeBluetooth( double data_rate, char *analysis_type )
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzeBluetooth", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}
		analysisBluetooth->dataRate = data_rate;
		analysisBluetooth->analysis_type = analysis_type;

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisBluetooth);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}


	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzeBluetooth", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzeBluetooth", timeDuration);


	return err;
}


IQMEASURE_API int LP_AnalyzeZigbee()
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzeZigbee", &timeStart);

	if (LibsInitialized)
	{
		if (!hndl->data)
		{
			err = ERR_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->data->length[0])
			{
				err = ERR_NO_CAPTURE_DATA;
				return err;
			}
		}

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisZigbee);

		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzeZigbee", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzeZigbee", timeDuration);


	return err;
}

IQMEASURE_API int LP_AnalyzeVHT80Mask()
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzeVHT80Mask", &timeStart);

	if (LibsInitialized)
	{
		if(true == bIQxsFound || true == bIQxelFound)
		{
			hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisVHT80);
			if (LP_Analyze())
			{
				err = ERR_ANALYSIS_FAILED;
			}
			else
			{

			}
		}
		else
		{
			err = ERR_ANALYSIS_FAILED; //only IQxs/IQxel can support 80MHz
		}

	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzeVHT80Mask", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzeVHT0Mask", timeDuration);


	return err;
}

IQMEASURE_API int LP_AnalyzeHT40Mask()
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzeHT40Mask", &timeStart);

	if (LibsInitialized)
	{
		//if (!hndl->data)
		//{
		//	err = ERR_NO_CAPTURE_DATA;
		//	return err;
		//}
		//else
		//{
		//	if (!hndl->data->length[0])
		//	{
		//		err = ERR_NO_CAPTURE_DATA;
		//		return err;
		//	}
		//}

#if !defined(IQAPI_1_5_X)
		// iqapiResultHT40 is supported in IQapi 1.6.x and beyond
		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisHT40);
#endif
		if (LP_Analyze())
		{
			err = ERR_ANALYSIS_FAILED;
		}
		else
		{

		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzeHT40Mask", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzeHT40Mask", timeDuration);


	return err;
}

// Keep this function with typo for backward compatibility
IQMEASURE_API double LP_GetScalarMeasurment(char *measurement, int index)
{
	return LP_GetScalarMeasurement(measurement, index);
}


/*! @defgroup group_scalar_measurement Available Measurement Names for LP_GetScalarMeasurement()
 *
 * Available measurement names vary for various analysis.  After an analysis has been performed successfully, by calling
 * one of the following functions:
 *   - LP_Analyze80211ag();
 *   - LP_AnalyzeMimo();
 *   - LP_Analyze80211b();
 *   - LP_Analyze80216d();
 *   - LP_Analyze80216e();
 *   - LP_AnalyzePower();
 *   - LP_AnalyzeFFT();
 *   - LP_AnalyzeCCDF();
 *   - LP_AnalyzeCW();
 *   - LP_AnalysisWave();
 *   - LP_AnalyzeSidelobe();
 *   - LP_AnalyzePowerRampOFDM();
 *   - LP_AnalyzePowerRamp80211b();
 *   - LP_AnalyzeBluetooth();
 *   - LP_AnalyzeZigbee();
 *
 * \section analysis_80211ag Measurement Names for LP_Analyze80211ag()
 *  - evmAll: EVM of all symbols in the capture. index value: 0
 *
 * \section analysis_mimo Measurement Names for LP_AnalyzeMimo()
 *  - evmAvgAll: EVM of all symbols in the capture. Each stream will have a corresponding value, so index value: [0 - (StreamNum-1)]
 */
IQMEASURE_API double LP_GetScalarMeasurement_NoTimer(char *measurement, int index)
{
	if (LibsInitialized)
	{
		if (!hndl->results)
		{
			return NA_NUMBER;
		}
		else
		{
			if (dynamic_cast<iqapiResultOFDM *>(hndl->results))
			{
				resultOfdm = dynamic_cast<iqapiResultOFDM *>(hndl->results);

				if (!strcmp(measurement, "psduCrcFail"))
				{
					return((double)resultOfdm->psduCrcFail);
				}
				else if (!strcmp(measurement, "plcpCrcPass"))
				{
					return((double)resultOfdm->plcpCrcPass);
				}
				else if (!strcmp(measurement, "dataRate"))
				{
					return((double)resultOfdm->dataRate);
				}
				else if (!strcmp(measurement, "numSymbols"))
				{
					return((double)resultOfdm->numSymbols);
				}
				else if (!strcmp(measurement, "numPsduBytes"))
				{
					return((double)resultOfdm->numPsduBytes);
				}
				else if (!strcmp(measurement, "evmAll"))
				{
					if (resultOfdm->evmAll && resultOfdm->evmAll->length > index)
						return(resultOfdm->evmAll->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "evmData"))
				{
					if (resultOfdm->evmData && resultOfdm->evmData->length > index)
						return(resultOfdm->evmData->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "evmPilot"))
				{
					if (resultOfdm->evmPilot && resultOfdm->evmPilot->length > index)
						return(resultOfdm->evmPilot->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "codingRate"))
				{
					if (resultOfdm->codingRate && resultOfdm->codingRate->length > index)
						return(resultOfdm->codingRate->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "freqErr"))
				{
					if (resultOfdm->freqErr && resultOfdm->freqErr->length > index)
						return(resultOfdm->freqErr->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "clockErr"))
				{
					if (resultOfdm->clockErr && resultOfdm->clockErr->length > index)
						return(resultOfdm->clockErr->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "ampErr"))
				{
					if (resultOfdm->ampErr && resultOfdm->ampErr->length > index)
						return(resultOfdm->ampErr->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "ampErrDb"))
				{
					if (resultOfdm->ampErrDb && resultOfdm->ampErrDb->length > index)
						return(resultOfdm->ampErrDb->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "phaseErr"))
				{
					if (resultOfdm->phaseErr && resultOfdm->phaseErr->length > index)
						return(resultOfdm->phaseErr->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "clockErr"))
				{
					if (resultOfdm->clockErr && resultOfdm->clockErr->length > index)
						return(resultOfdm->clockErr->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rmsPhaseNoise"))
				{
					if (resultOfdm->rmsPhaseNoise && resultOfdm->rmsPhaseNoise->length > index)
						return(resultOfdm->rmsPhaseNoise->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "powerPreambleDbm"))
				{
					if (resultOfdm->powerPreambleDbm && resultOfdm->powerPreambleDbm->length > index)
						return(resultOfdm->powerPreambleDbm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rmsPowerNoGap"))
				{
					if (resultOfdm->rmsPowerNoGap && resultOfdm->rmsPowerNoGap->length > index)
						return(resultOfdm->rmsPowerNoGap->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rmsPower"))
				{
					if (resultOfdm->rmsPower && resultOfdm->rmsPower->length > index)
						return(resultOfdm->rmsPower->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "pkPower"))  // OFDM
				{
					if (resultOfdm->pkPower && resultOfdm->pkPower->length > index)
						return(resultOfdm->pkPower->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rmsMaxAvgPower"))
				{
					if (resultOfdm->rmsMaxAvgPower && resultOfdm->rmsMaxAvgPower->length > index)
						return(resultOfdm->rmsMaxAvgPower->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "dcLeakageDbc"))
				{
					if (resultOfdm->dcLeakageDbc && resultOfdm->dcLeakageDbc->length > index)
						return (resultOfdm->dcLeakageDbc->real[index]);
					else
						return NA_NUMBER;
				}
				else
				{
					return NA_NUMBER;
				}

			}

			else if (dynamic_cast<iqapiResult80216 *>(hndl->results))
			{
				result80216 = dynamic_cast<iqapiResult80216 *>(hndl->results);

				if(!strcmp(measurement,"packetDetection"))
				{
					return ((double) result80216->packetDetection);
				}

				else if (!strcmp(measurement, "acquisition"))
				{
					return((double) result80216->acquisition);
				}

				else if (!strcmp(measurement, "demodulation"))
				{
					return((double)result80216->demodulation);
				}

				else if (!strcmp(measurement, "completePacket"))
				{
					return((double)result80216->completePacket);
				}

				else if (!strcmp(measurement, "fchHcs"))
				{
					return((double)result80216->fchHcs);
				}

				else if (!strcmp(measurement, "numberOfZone"))
				{
					return((double)result80216->numberOfZone);
				}

				else if (!strcmp(measurement, "sigType"))
				{
					if (result80216->sigType && result80216->sigType->length > index)
						return (result80216->sigType->real[index]);
					else
						return NA_NUMBER;
				}
				//				else if (!strcmp(measurement, "numSymbols"))
				//				{
				//					if (result80216->numSymbols && result80216->numSymbols->length > index)
				//						return (result80216->numSymbols->real[index]);
				//					else
				//						return NA_NUMBER;
				//				}
				else if (!strcmp(measurement, "bandwidthHz"))
				{
					if (result80216->bandwidthHz && result80216->bandwidthHz->length > index)
						return (result80216->bandwidthHz->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "cyclicPrefix"))
				{
					if (result80216->cyclicPrefix && result80216->cyclicPrefix->length > index)
						return (result80216->cyclicPrefix->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "evmAvgAll"))
				{
					if (result80216->evmAvgAll && result80216->evmAvgAll->length > index)
						return (result80216->evmAvgAll->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "evmAvgData"))
				{
					if (result80216->evmAvgData && result80216->evmAvgData->length > index)
						return (result80216->evmAvgData->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "evmAvgPilot"))
				{
					if (result80216->evmAvgPilot && result80216->evmAvgPilot->length > index)
						return (result80216->evmAvgPilot->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "evmCinrDb"))
				{
					if (result80216->evmCinrDb && result80216->evmCinrDb->length > index)
						return (result80216->evmCinrDb->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "avgUnmodData"))
				{
					if (result80216->avgUnmodData && result80216->avgUnmodData->length > index)
						return (result80216->avgUnmodData->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "dcLeakageDbc"))
				{
					if (result80216->dcLeakageDbc && result80216->dcLeakageDbc->length > index)
						return (result80216->dcLeakageDbc->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "avgPowerNoGapDb"))
				{
					if (result80216->avgPowerNoGapDb && result80216->avgPowerNoGapDb->length > index)
						return (result80216->avgPowerNoGapDb->real[index]);
					else
						return NA_NUMBER;

				}

				else if (!strcmp(measurement, "rxPreambleRmsPowerDb"))
				{
					if (result80216->rxPreambleRmsPowerDb && result80216->rxPreambleRmsPowerDb->length > index)
						return (result80216->rxPreambleRmsPowerDb->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "freqErrorHz"))
				{
					if (result80216->freqErrorHz && result80216->freqErrorHz->length > index)
						return(result80216->freqErrorHz->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "freqOffsetTotalHz"))
				{
					if (result80216->freqOffsetTotalHz && result80216->freqOffsetTotalHz->length > index)
						return(result80216->freqOffsetTotalHz->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "symClockErrorPpm"))
				{
					if (result80216->symClockErrorPpm && result80216->symClockErrorPpm->length > index)
						return(result80216->symClockErrorPpm->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "phaseNoiseDegRmsAll"))
				{
					if (result80216->phaseNoiseDegRmsAll && result80216->phaseNoiseDegRmsAll->length > index)
						return(result80216->phaseNoiseDegRmsAll->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "iqImbalAmplDb"))
				{
					if (result80216->iqImbalAmplDb && result80216->iqImbalAmplDb->length > index)
						return(result80216->iqImbalAmplDb->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "iqImbalPhaseDeg"))
				{
					if (result80216->iqImbalPhaseDeg && result80216->iqImbalPhaseDeg->length > index)
						return(result80216->iqImbalPhaseDeg->real[index]);
					else
						return NA_NUMBER;
				}

				else
				{
					return NA_NUMBER;
				}
			}
			//since the 11AC and MIMO structure is so close, it is better to put 11AC in front of MIMO, there might be a better way, Zhiyong On Aug 14, 2011
			//else if (dynamic_cast<iqapiResult11ac *>(hndl->results))
			else if (dynamic_cast<iqapiResultWifiOfdm *>(hndl->results))
			{
				double attn[4]={0.0};//, attnError[4]={0.0};

				//result11ac = dynamic_cast<iqapiResult11ac *>(hndl->results);
				result11ac = dynamic_cast<iqapiResultWifiOfdm *>(hndl->results);

				//not supported until customer really need this info
				//if (!strcmp(measurement, "VHTSigA1Bits"))
				//{
				//}
				if (!strcmp(measurement, "VHTSigA1Bandwidth") || !strcmp(measurement, "vhtSigA1Bandwidth")) //for backward compatible
				{
					if (result11ac->vhtSigA1Bandwidth && result11ac->vhtSigA1Bandwidth->length > index)
						return(result11ac->vhtSigA1Bandwidth->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "VHTSigA1Stbc") || !strcmp(measurement, "vhtSigA1Stbc") )
				{
					if (result11ac->vhtSigA1Stbc && result11ac->vhtSigA1Stbc->length > index)
						return(result11ac->vhtSigA1Stbc->real[index]);
					else
						return NA_NUMBER;
				}

				// For VHTSigA2
				//not supported until customer really need this info
				//if (!strcmp(measurement, "VHTSigA2Bits"))
				//{
				//}
				else if (!strcmp(measurement, "VHTSigA2ShortGI") || !strcmp(measurement, "vhtSigA2ShortGI") )
				{
					if (result11ac->vhtSigA2ShortGI && result11ac->vhtSigA2ShortGI->length > index)
						return(result11ac->vhtSigA2ShortGI->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "VHTSigA2AdvancedCoding") || !strcmp(measurement, "vhtSigA2AdvancedCoding"))
				{
					if (result11ac->vhtSigA2AdvancedCoding && result11ac->vhtSigA2AdvancedCoding->length > index)
						return(result11ac->vhtSigA2AdvancedCoding->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "VHTSigA2McsIndex") || !strcmp(measurement, "vhtSigA2McsIndex"))
				{
					if (result11ac->vhtSigA2McsIndex && result11ac->vhtSigA2McsIndex->length > index)
						return(result11ac->vhtSigA2McsIndex->real[index]);
					else
						return NA_NUMBER;
				}

				// For VHTSigB
				//not supported until customer really need this info
				//if (!strcmp(measurement, "VHTSigBBits"))
				//{
				//}

				else if (!strcmp(measurement, "VHTSigBFieldCRC") || !strcmp(measurement, "vhtSigBFieldCRC"))
				{
					return((double)result11ac->vhtSigBFieldCRC);
				}

				else if (!strcmp(measurement, "evmAvgAll"))
				{
					if (result11ac->evmAvgAll && result11ac->evmAvgAll->length > index)
						return(result11ac->evmAvgAll->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "packetDetection"))
				{
					return((double)result11ac->packetDetection);
				}
				else if (!strcmp(measurement, "psduCRC"))
				{
					return((double)result11ac->psduCRC);
				}
				else if (!strcmp(measurement, "numPsduBytes"))
				{
					return((double)result11ac->numPsduBytes);
				}
				else if (!strcmp(measurement, "acquisition"))
				{
					return((double)result11ac->acquisition);
				}

				else if (!strcmp(measurement, "demodulation"))
				{
					return((double)result11ac->demodulation);
				}
				else if (!strcmp(measurement, "dcLeakageDbc"))
				{
					if (result11ac->dcLeakageDbc && result11ac->dcLeakageDbc->length > index)
					{
						//printf("dc_leakage is %0.2f\n", result11ac->dcLeakageDbc->real[index+1]);
						//printf("dc_leakage is %0.2f\n", result11ac->dcLeakageDbc->real[index+2]);
						return(result11ac->dcLeakageDbc->real[index]);
					}
					else
						return NA_NUMBER;
				}
				if (!strcmp(measurement, "powerPreambleDbm"))
				{
					if (result11ac->powerPreambleDbm && result11ac ->powerPreambleDbm->length > index)
					{
						return(result11ac->powerPreambleDbm->real[index]);
					}
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rxRmsPowerDb"))
				{
					if (result11ac->rxRmsPowerDb && result11ac ->rxRmsPowerDb->length > index)
						//no sequential mode any more.
						//if( analysisWifiOfdm->SequentialMimo.numSections>0 )
						//{
						//	return(result11ac->rxRmsPowerDb->real[index] +
						//			attn[index/analysisWifiOfdm->SequentialMimo.numSections] );
						//}
						//else
					{
						return(result11ac->rxRmsPowerDb->real[index]);
					}
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "isolationDb"))
				{
					if (result11ac->isolationDb && result11ac->isolationDb->length > index)
					{
						return(result11ac->isolationDb->real[index]);
					}
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "freqErrorHz"))
				{
					if (result11ac->freqErrorHz && result11ac->freqErrorHz->length > index)
						return(result11ac->freqErrorHz->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "symClockErrorPpm"))
				{
					if (result11ac->symClockErrorPpm && result11ac->symClockErrorPpm->length > index)
						return(result11ac->symClockErrorPpm->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "PhaseNoiseDeg_RmsAll"))
				{
					if (result11ac->PhaseNoiseDeg_RmsAll && result11ac->PhaseNoiseDeg_RmsAll->length > index)
						return(result11ac->PhaseNoiseDeg_RmsAll->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "IQImbal_amplDb"))
				{
					if (result11ac->IQImbal_amplDb && result11ac->IQImbal_amplDb->length > index)
						return(result11ac->IQImbal_amplDb->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "IQImbal_phaseDeg"))
				{
					if (result11ac->IQImbal_phaseDeg && result11ac->IQImbal_phaseDeg->length > index)
						return(result11ac->IQImbal_phaseDeg->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "rateInfo_bandwidthMhz"))
				{
					if (result11ac->rateInfo_bandwidthMhz && result11ac->rateInfo_bandwidthMhz->length > index)
						return(result11ac->rateInfo_bandwidthMhz->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "rateInfo_dataRateMbps"))
				{
					if (result11ac->rateInfo_dataRateMbps && result11ac->rateInfo_dataRateMbps->length > index)
						return(result11ac->rateInfo_dataRateMbps->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "rateInfo_spatialStreams"))
				{
					if (result11ac->rateInfo_spatialStreams && result11ac->rateInfo_spatialStreams->length > index)
						return(result11ac->rateInfo_spatialStreams->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "analyzedRange"))
				{
					if (result11ac->analyzedRange && result11ac->analyzedRange->length > index)
						return(result11ac->analyzedRange->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "rateInfo_spaceTimeStreams"))
				{
					if (result11ac->rateInfo_spaceTimeStreams && result11ac->rateInfo_spaceTimeStreams->length > index)
						return(result11ac->rateInfo_spaceTimeStreams->real[index]);
					else
						return NA_NUMBER;
				}
				else
				{
					return NA_NUMBER;
				}
			}

			else if (dynamic_cast<iqapiResultMimo *>(hndl->results))
			{
				double attn[4]={0.0};//, attnError[4]={0.0};
				resultMimo = dynamic_cast<iqapiResultMimo *>(hndl->results);

				if (!strcmp(measurement, "evmAvgAll"))
				{
					if (resultMimo->evmAvgAll && resultMimo->evmAvgAll->length > index)
						return(resultMimo->evmAvgAll->real[index]);
					else
						return NA_NUMBER;
				}
				else
					if (!strcmp(measurement, "packetDetection"))
					{
						return((double)resultMimo->packetDetection);
					}
					else if (!strcmp(measurement, "psduCRC"))
					{
						return((double)resultMimo->psduCRC);
					}
					else if (!strcmp(measurement, "acquisition"))
					{
						return((double)resultMimo->acquisition);
					}

					else if (!strcmp(measurement, "demodulation"))
					{
						return((double)resultMimo->demodulation);
					}

					else if (!strcmp(measurement, "evmAvgAll"))
					{
						if (resultMimo->evmAvgAll && resultMimo->evmAvgAll->length > index)
							return(resultMimo->evmAvgAll->real[index]);
						else
							return NA_NUMBER;
					}

					else if (!strcmp(measurement, "dcLeakageDbc"))
					{
						if (resultMimo->dcLeakageDbc && resultMimo->dcLeakageDbc->length > index)
							return(resultMimo->dcLeakageDbc->real[index]);
						else
							return NA_NUMBER;
					}
					else if (!strcmp(measurement, "powerPreambleDbm"))
					{
						if (resultMimo->powerPreambleDbm && resultMimo ->powerPreambleDbm->length > index)
						{
							return(resultMimo->powerPreambleDbm->real[index]);
						}
						else
							return NA_NUMBER;
					}

					else if (!strcmp(measurement, "rxRmsPowerDb"))
					{
						if (resultMimo->rxRmsPowerDb && resultMimo ->rxRmsPowerDb->length > index)
							if( analysisMimo->SequentialMimo.numSections>0 )
							{
								return(resultMimo->rxRmsPowerDb->real[index] +
										attn[index/analysisMimo->SequentialMimo.numSections] );
							}
							else
							{
								return(resultMimo->rxRmsPowerDb->real[index]);
							}
						else
							return NA_NUMBER;
					}
					else if (!strcmp(measurement, "isolationDb"))
					{
						if (resultMimo->isolationDb && resultMimo->isolationDb->length > index)
						{
							return(resultMimo->isolationDb->real[index]);
						}
						else
							return NA_NUMBER;
					}
					else if (!strcmp(measurement, "freqErrorHz"))
					{
						if (resultMimo->freqErrorHz && resultMimo->freqErrorHz->length > index)
							return(resultMimo->freqErrorHz->real[index]);
						else
							return NA_NUMBER;
					}

					else if (!strcmp(measurement, "symClockErrorPpm"))
					{
						if (resultMimo->symClockErrorPpm && resultMimo->symClockErrorPpm->length > index)
							return(resultMimo->symClockErrorPpm->real[index]);
						else
							return NA_NUMBER;
					}

					else if (!strcmp(measurement, "PhaseNoiseDeg_RmsAll"))
					{
						if (resultMimo->PhaseNoiseDeg_RmsAll && resultMimo->PhaseNoiseDeg_RmsAll->length > index)
							return(resultMimo->PhaseNoiseDeg_RmsAll->real[index]);
						else
							return NA_NUMBER;
					}

					else if (!strcmp(measurement, "IQImbal_amplDb"))
					{
						if (resultMimo->IQImbal_amplDb && resultMimo->IQImbal_amplDb->length > index)
							return(resultMimo->IQImbal_amplDb->real[index]);
						else
							return NA_NUMBER;
					}

					else if (!strcmp(measurement, "IQImbal_phaseDeg"))
					{
						if (resultMimo->IQImbal_phaseDeg && resultMimo->IQImbal_phaseDeg->length > index)
							return(resultMimo->IQImbal_phaseDeg->real[index]);
						else
							return NA_NUMBER;
					}

					else if (!strcmp(measurement, "rateInfo_bandwidthMhz"))
					{
						if (resultMimo->rateInfo_bandwidthMhz && resultMimo->rateInfo_bandwidthMhz->length > index)
							return(resultMimo->rateInfo_bandwidthMhz->real[index]);
						else
							return NA_NUMBER;
					}

					else if (!strcmp(measurement, "rateInfo_dataRateMbps"))
					{
						if (resultMimo->rateInfo_dataRateMbps && resultMimo->rateInfo_dataRateMbps->length > index)
							return(resultMimo->rateInfo_dataRateMbps->real[index]);
						else
							return NA_NUMBER;
					}

					else if (!strcmp(measurement, "rateInfo_spatialStreams"))
					{
						if (resultMimo->rateInfo_spatialStreams && resultMimo->rateInfo_spatialStreams->length > index)
							return(resultMimo->rateInfo_spatialStreams->real[index]);
						else
							return NA_NUMBER;
					}

					else if (!strcmp(measurement, "analyzedRange"))
					{
						if (resultMimo->analyzedRange && resultMimo->analyzedRange->length > index)
							return(resultMimo->analyzedRange->real[index]);
						else
							return NA_NUMBER;
					}
					else if (!strcmp(measurement, "htSig1_htLength"))
					{
						if (resultMimo->htSig1_htLength && resultMimo->htSig1_htLength->length > index)
							return(resultMimo->htSig1_htLength->real[index]);
						else
							return NA_NUMBER;
					}
					else if (!strcmp(measurement, "htSig1_mcsIndex"))
					{
						if (resultMimo->htSig1_mcsIndex && resultMimo->htSig1_mcsIndex->length > index)
							return(resultMimo->htSig1_mcsIndex->real[index]);
						else
							return NA_NUMBER;
					}
					else if (!strcmp(measurement, "htSig1_bandwidth"))
					{
						if (resultMimo->htSig1_bandwidth && resultMimo->htSig1_bandwidth->length > index)
							return(resultMimo->htSig1_bandwidth->real[index]);
						else
							return NA_NUMBER;
					}
					else if (!strcmp(measurement, "htSig2_advancedCoding"))
					{
						if (resultMimo->htSig2_advancedCoding && resultMimo->htSig2_advancedCoding->length > index)
							return(resultMimo->htSig2_advancedCoding->real[index]);
						else
							return NA_NUMBER;
					}
					else if (!strcmp(measurement, "rateInfo_spaceTimeStreams"))
					{
						if (resultMimo->rateInfo_spaceTimeStreams && resultMimo->rateInfo_spaceTimeStreams->length > index)
							return(resultMimo->rateInfo_spaceTimeStreams->real[index]);
						else
							return NA_NUMBER;
					}
					else
					{
						return NA_NUMBER;
					}
			}

			else if (dynamic_cast<iqapiResult11b *>(hndl->results))
			{
				result11b = dynamic_cast<iqapiResult11b *>(hndl->results);

				if (!strcmp(measurement, "lockedClock"))
				{
					return((double)result11b->lockedClock);
				}
				else if (!strcmp(measurement, "plcpCrcFail"))
				{
					return((double)result11b->plcpCrcFail);
				}
				else if (!strcmp(measurement, "psduCrcFail"))
				{
					return((double)result11b->psduCrcFail);
				}
				else if (!strcmp(measurement, "numPsduBytes"))
				{
					return((double)result11b->numPsduBytes);
				}
				else if (!strcmp(measurement, "longPreamble"))
				{
					return((double)result11b->longPreamble);
				}
				else if (!strcmp(measurement, "bitRateInMHz"))
				{
					return((double)result11b->bitRateInMHz);
				}
				else if (!strcmp(measurement, "evmPk"))
				{
					if (result11b->evmPk && result11b->evmPk->length > index)
						return(result11b->evmPk->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "evmAll"))
				{
					if (result11b->evmAll && result11b->evmAll->length > index)
						return(result11b->evmAll->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "evmInPreamble"))
				{
					if (result11b->evmInPreamble && result11b->evmInPreamble->length > index)
						return(result11b->evmInPreamble->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "evmInPsdu"))
				{
					if (result11b->evmInPsdu && result11b->evmInPsdu->length > index)
						return(result11b->evmInPsdu->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "powerPreambleDbm"))//LK
				{
					if (result11b->powerPreambleDbm && result11b->powerPreambleDbm->length > index)
						return(result11b->powerPreambleDbm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "loLeakageDb"))
				{
					if (result11b->loLeakageDb && result11b->loLeakageDb->length > index)
						return(result11b->loLeakageDb->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "freqErr"))
				{
					if (result11b->freqErr && result11b->freqErr->length > index)
						return(result11b->freqErr->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "clockErr"))
				{
					if (result11b->clockErr && result11b->clockErr->length > index)
						return(result11b->clockErr->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "ampErr"))
				{
					if (result11b->ampErr && result11b->ampErr->length > index)
						return(result11b->ampErr->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "ampErrDb"))
				{
					if (result11b->ampErrDb && result11b->ampErrDb->length > index)
						return(result11b->ampErrDb->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "phaseErr"))
				{
					if (result11b->phaseErr && result11b->phaseErr->length > index)
						return(result11b->phaseErr->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rmsPhaseNoise"))
				{
					if (result11b->rmsPhaseNoise && result11b->rmsPhaseNoise->length > index)
						return(result11b->rmsPhaseNoise->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rmsPowerNoGap"))
				{
					if (result11b->rmsPowerNoGap && result11b->rmsPowerNoGap->length > index)
						return(result11b->rmsPowerNoGap->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rmsPower"))
				{
					if (result11b->rmsPower && result11b->rmsPower->length > index)
						return(result11b->rmsPower->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "pkPower"))  // 11b
				{
					if (result11b->pkPower && result11b->pkPower->length > index)
						return(result11b->pkPower->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rmsMaxAvgPower"))
				{
					if (result11b->rmsMaxAvgPower && result11b->rmsMaxAvgPower->length > index)
						return(result11b->rmsMaxAvgPower->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "bitRate"))
				{
					if (result11b->bitRate && result11b->bitRate->length > index)
						return(result11b->bitRate->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "modType"))
				{
					if (result11b->modType && result11b->modType->length > index)
						return(result11b->modType->real[index]);
					else
						return NA_NUMBER;
				}

				//			else if (!strcmp(measurement, "maxFreqErr"))
				//			{
				//				if (result11b->maxFreqErr && result11b->maxFreqErr->length > index)
				//					return(result11b->maxFreqErr->real[index]);
				//				else
				//					return NA_NUMBER;
				//			}

				else if (!strcmp(measurement, "rmsMaxAvgPower"))
				{
					if (result11b->rmsMaxAvgPower && result11b->rmsMaxAvgPower->length > index)
						return(result11b->rmsMaxAvgPower->real[index]);
					else
						return NA_NUMBER;
				}
				else
				{
					return NA_NUMBER;
				}
			}

			else if (dynamic_cast<iqapiResultPower *>(hndl->results))
			{
				resultPower = dynamic_cast<iqapiResultPower *>(hndl->results);

				if (!strcmp(measurement, "valid"))
				{//temporary work around before IQapi team fix this.
					if (resultPower->P_av_each_burst_dBm && resultPower->P_av_each_burst_dBm->length > index)
						return 1;
					else
						return NA_NUMBER;
					//return((double)resultPower->valid);
				}

				else if (!strcmp(measurement, "P_av_each_burst"))
				{
					if (resultPower->P_av_each_burst && resultPower->P_av_each_burst->length > index)
						return(resultPower->P_av_each_burst->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "P_av_each_burst_dBm"))
				{
					if (resultPower->P_av_each_burst_dBm && resultPower->P_av_each_burst_dBm->length > index)
						return(resultPower->P_av_each_burst_dBm->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "P_pk_each_burst"))
				{
					if (resultPower->P_pk_each_burst && resultPower->P_pk_each_burst->length > index)
						return(resultPower->P_pk_each_burst->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "P_pk_each_burst_dBm"))
				{
					if (resultPower->P_pk_each_burst_dBm && resultPower->P_pk_each_burst_dBm->length > index)
						return(resultPower->P_pk_each_burst_dBm->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "P_av_all"))
				{
					if (resultPower->P_av_all && resultPower->P_av_all->length > index)
						return(resultPower->P_av_all->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "P_av_all_dBm"))
				{
					if (resultPower->P_av_all_dBm && resultPower->P_av_all_dBm->length > index)
						return(resultPower->P_av_all_dBm->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "P_peak_all_dBm"))
				{
					if (resultPower->P_peak_all_dBm && resultPower->P_peak_all_dBm->length > index)
						return(resultPower->P_peak_all_dBm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "P_peak_all"))
				{
					if (resultPower->P_peak_all && resultPower->P_peak_all->length > index)
						return(resultPower->P_peak_all->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "P_av_no_gap_all"))
				{
					if (resultPower->P_av_no_gap_all && resultPower->P_av_no_gap_all->length > index)
						return(resultPower->P_av_no_gap_all->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "P_av_no_gap_all_dBm"))
				{
					if (resultPower->P_av_no_gap_all_dBm && resultPower->P_av_no_gap_all_dBm->length > index)
						return(resultPower->P_av_no_gap_all_dBm->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "start_sec"))
				{
					if (resultPower->start_sec && resultPower->start_sec->length > index)
						return(resultPower->start_sec->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "stop_sec"))
				{
					if (resultPower->stop_sec && resultPower->stop_sec->length > index)
						return(resultPower->stop_sec->real[index]);
					else
						return NA_NUMBER;
				}
				else
				{
					return NA_NUMBER;
				}
			}

			else if (dynamic_cast<iqapiResultFFT *>(hndl->results))
			{
				resultFFT = dynamic_cast<iqapiResultFFT *>(hndl->results);

				if (!strcmp(measurement, "valid"))
				{
					return((double)resultFFT->valid);
				}

				else if (!strcmp(measurement, "length"))
				{
					if (resultFFT->length && resultFFT->length->length > index)
						return(resultFFT->length->real[index]);
					else
						return NA_NUMBER;
				}

				else
				{
					return NA_NUMBER;
				}
			}

			else if (dynamic_cast<iqapiResultCCDF *>(hndl->results))
			{
				resultCCDF = dynamic_cast<iqapiResultCCDF *>(hndl->results);

				if (!strcmp(measurement, "percent_pow"))
				{
					if (resultCCDF->percent_pow && resultCCDF->percent_pow->length > index)
						return(resultCCDF->percent_pow->real[index]);
					else
						return NA_NUMBER;
				}

				else
				{
					return NA_NUMBER;
				}
			}

			else if (dynamic_cast<iqapiResultSidelobe *>(hndl->results))
			{
				resultSidelobe = dynamic_cast<iqapiResultSidelobe *>(hndl->results);

				if (!strcmp(measurement, "res_bw_Hz"))
				{
					if (resultSidelobe->res_bw_Hz && resultSidelobe->res_bw_Hz->length > index)
						return(resultSidelobe->res_bw_Hz->real[index]);
					else
						return NA_NUMBER;
				} else if (!strcmp(measurement, "fft_bin_size_Hz"))
				{
					if (resultSidelobe->fft_bin_size_Hz && resultSidelobe->fft_bin_size_Hz->length > index)
						return(resultSidelobe->fft_bin_size_Hz->real[index]);
					else
						return NA_NUMBER;
				} else if (!strcmp(measurement, "peak_center"))
				{
					if (resultSidelobe->peak_center && resultSidelobe->peak_center->length > index)
						return(resultSidelobe->peak_center->real[index]);
					else
						return NA_NUMBER;
				} else if (!strcmp(measurement, "peak_1_left"))
				{
					if (resultSidelobe->peak_1_left && resultSidelobe->peak_1_left->length > index)
						return(resultSidelobe->peak_1_left->real[index]);
					else
						return NA_NUMBER;
				} else if (!strcmp(measurement, "peak_2_left"))
				{
					if (resultSidelobe->peak_2_left && resultSidelobe->peak_2_left->length > index)
						return(resultSidelobe->peak_2_left->real[index]);
					else
						return NA_NUMBER;
				} else if (!strcmp(measurement, "peak_1_right"))
				{
					if (resultSidelobe->peak_1_right && resultSidelobe->peak_1_right->length > index)
						return(resultSidelobe->peak_1_right->real[index]);
					else
						return NA_NUMBER;
				} else if (!strcmp(measurement, "peak_2_right"))
				{
					if (resultSidelobe->peak_2_right && resultSidelobe->peak_2_right->length > index)
						return(resultSidelobe->peak_2_right->real[index]);
					else
						return NA_NUMBER;
				} else if (!strcmp(measurement, "psd_dB"))
				{
					if (resultSidelobe->psd_dB && resultSidelobe->psd_dB->length > index)
						return(resultSidelobe->psd_dB->real[index]);
					else
						return NA_NUMBER;
				}
				else
				{
					return NA_NUMBER;
				}
			}
			else if (dynamic_cast<iqapiResultPowerRamp *>(hndl->results))
			{
				resultPowerRamp = dynamic_cast<iqapiResultPowerRamp *>(hndl->results);

				if (!strcmp(measurement, "on_time"))
				{
					if (resultPowerRamp->on_time && resultPowerRamp->on_time->length > index)
						return(resultPowerRamp->on_time->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "off_time"))
				{
					if (resultPowerRamp->off_time && resultPowerRamp->off_time->length > index)
						return(resultPowerRamp->off_time->real[index]);
					else
						return NA_NUMBER;
				}
				else
				{
					return NA_NUMBER;
				}
			}
			else if (dynamic_cast<iqapiResultBluetooth *>(hndl->results))
			{
				resultBluetooth = dynamic_cast<iqapiResultBluetooth *>(hndl->results);

				if (!strcmp(measurement, "dataRateDetect"))
				{
					return((double)resultBluetooth->dataRateDetect);
				}
				if (!strcmp(measurement, "valid"))
				{
					if (resultBluetooth->valid && resultBluetooth->valid->length > index)
						return(resultBluetooth->valid->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "bandwidth20dB"))
				{
					if (resultBluetooth->bandwidth20dB && resultBluetooth->bandwidth20dB->length > index)
						return(resultBluetooth->bandwidth20dB->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "P_av_each_burst"))
				{
					if (resultBluetooth->P_av_each_burst && resultBluetooth->P_av_each_burst->length > index)
						return(resultBluetooth->P_av_each_burst->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "P_pk_each_burst"))
				{
					if (resultBluetooth->P_pk_each_burst && resultBluetooth->P_pk_each_burst->length > index)
						return(resultBluetooth->P_pk_each_burst->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "freq_est"))
				{
					if (resultBluetooth->freq_est && resultBluetooth->freq_est->length > index)
						return(resultBluetooth->freq_est->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "freq_drift"))
				{
					if (resultBluetooth->freq_drift && resultBluetooth->freq_drift->length > index)
						return(resultBluetooth->freq_drift->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "deltaF1Average"))
				{
					if (resultBluetooth->deltaF1Average && resultBluetooth->deltaF1Average->length > index)
						return(resultBluetooth->deltaF1Average->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "deltaF2Max"))
				{
					if (resultBluetooth->deltaF2Max && resultBluetooth->deltaF2Max->length > index)
						return(resultBluetooth->deltaF2Max->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "deltaF2Average"))
				{
					if (resultBluetooth->deltaF2Average && resultBluetooth->deltaF2Average->length > index)
						return(resultBluetooth->deltaF2Average->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "deltaF2MaxAccess"))
				{
					if (resultBluetooth->deltaF2MaxAccess && resultBluetooth->deltaF2MaxAccess->length > index)
						return(resultBluetooth->deltaF2MaxAccess->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "deltaF2AvAccess"))
				{
					if (resultBluetooth->deltaF2AvAccess && resultBluetooth->deltaF2AvAccess->length > index)
						return(resultBluetooth->deltaF2AvAccess->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "EdrEVMAv"))
				{
					if (resultBluetooth->EdrEVMAv && resultBluetooth->EdrEVMAv->length > index)
						return(resultBluetooth->EdrEVMAv->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "EdrEVMpk"))
				{
					if (resultBluetooth->EdrEVMpk && resultBluetooth->EdrEVMpk->length > index)
						return(resultBluetooth->EdrEVMpk->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "EdrEVMvalid"))
				{
					if (resultBluetooth->EdrEVMvalid && resultBluetooth->EdrEVMvalid->length > index)
						return(resultBluetooth->EdrEVMvalid->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "EdrPowDiffdB"))
				{
					if (resultBluetooth->EdrPowDiffdB && resultBluetooth->EdrPowDiffdB->length > index)
						return(resultBluetooth->EdrPowDiffdB->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "freq_deviation"))
				{
					if (resultBluetooth->freq_deviation && resultBluetooth->freq_deviation->length > index)
						return(resultBluetooth->freq_deviation->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "freq_deviationpktopk"))
				{
					if (resultBluetooth->freq_deviationpktopk && resultBluetooth->freq_deviationpktopk->length > index)
						return(resultBluetooth->freq_deviationpktopk->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "freq_estHeader"))
				{
					if (resultBluetooth->freq_estHeader && resultBluetooth->freq_estHeader->length > index)
						return(resultBluetooth->freq_estHeader->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "EdrFreqExtremeEdronly"))
				{
					if (resultBluetooth->EdrFreqExtremeEdronly && resultBluetooth->EdrFreqExtremeEdronly->length > index)
						return(resultBluetooth->EdrFreqExtremeEdronly->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "EdrprobEVM99pass"))
				{
					if (resultBluetooth->EdrprobEVM99pass && resultBluetooth->EdrprobEVM99pass->length > index)
						return(resultBluetooth->EdrprobEVM99pass->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "EdrEVMvsTime"))
				{
					if (resultBluetooth->EdrEVMvsTime && resultBluetooth->EdrEVMvsTime->length > index)
						return(resultBluetooth->EdrEVMvsTime->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "validInput"))
				{
					if (resultBluetooth->validInput && resultBluetooth->validInput->length > index)
						return(resultBluetooth->validInput->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "maxfreqDriftRate"))
				{
					if (resultBluetooth->maxfreqDriftRate && resultBluetooth->maxfreqDriftRate->length > index)
						return(resultBluetooth->maxfreqDriftRate->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "EdrOmegaI"))
				{
					if (resultBluetooth->EdrOmegaI && resultBluetooth->EdrOmegaI->length > index)
						return(resultBluetooth->EdrOmegaI->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "EdrExtremeOmega0"))
				{
					if (resultBluetooth->EdrExtremeOmega0 && resultBluetooth->EdrExtremeOmega0->length > index)
						return(resultBluetooth->EdrExtremeOmega0->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "EdrExtremeOmegaI0"))
				{
					if (resultBluetooth->EdrExtremeOmegaI0 && resultBluetooth->EdrExtremeOmegaI0->length > index)
						return(resultBluetooth->EdrExtremeOmegaI0->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "payloadErrors"))
				{
					if (resultBluetooth->payloadErrors && resultBluetooth->payloadErrors->length > index)
						return(resultBluetooth->payloadErrors->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "maxPowerAcpDbm"))
				{
					if (resultBluetooth->maxPowerAcpDbm && resultBluetooth->maxPowerAcpDbm->length > index)
						return(resultBluetooth->maxPowerAcpDbm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "maxPowerEdrDbm"))
				{
					if (resultBluetooth->maxPowerEdrDbm && resultBluetooth->maxPowerEdrDbm->length > index)
						return(resultBluetooth->maxPowerEdrDbm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "meanNoGapPowerCenterDbm"))
				{
					if (resultBluetooth->meanNoGapPowerCenterDbm && resultBluetooth->meanNoGapPowerCenterDbm->length > index)
						return(resultBluetooth->meanNoGapPowerCenterDbm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "sequenceDefinition"))
				{
					if (resultBluetooth->sequenceDefinition && resultBluetooth->sequenceDefinition->length > index)
						return(resultBluetooth->sequenceDefinition->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "acpErrValid"))
				{
					return((int)resultBluetooth->acpErrValid);
				}
				//BT LE

				else if (!strcmp(measurement, "leFreqOffset"))
				{
					if (resultBluetooth->leFreqOffset && resultBluetooth->leFreqOffset->length > index)
						return(resultBluetooth->leFreqOffset->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leDeltaF1Avg"))
				{
					if (resultBluetooth->leDeltaF1Avg && resultBluetooth->leDeltaF1Avg->length > index)
						return(resultBluetooth->leDeltaF1Avg->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leDeltaF2Max"))
				{
					if (resultBluetooth->leDeltaF2Max && resultBluetooth->leDeltaF2Max->length > index)
					{
						double dF2temp = -NA_NUMBER;   // Delta_F2_Max is changed to report whole vector in the IQapi. Thus, it needs to sort out the min. Zhiyong 3/4/2010
						dF2temp = resultBluetooth->leDeltaF2Max->real[0];
						for(int dF2Ind=1;dF2Ind<resultBluetooth->leDeltaF2Max->length; dF2Ind++)
						{
							if (dF2temp > resultBluetooth->leDeltaF2Max->real[dF2Ind])
							{
								dF2temp=resultBluetooth->leDeltaF2Max->real[dF2Ind];
							}
						}
						return(dF2temp);
						//return(resultBluetooth->leDeltaF2Max->real[index]);
					}
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leDeltaF2Avg"))
				{
					if (resultBluetooth->leDeltaF2Avg && resultBluetooth->leDeltaF2Avg->length > index)
						return(resultBluetooth->leDeltaF2Avg->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leBelow185F2Max"))
				{
					if (resultBluetooth->leBelow185F2Max && resultBluetooth->leBelow185F2Max->length > index)
						return(resultBluetooth->leBelow185F2Max->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leFn"))
				{
					if (resultBluetooth->leFn && resultBluetooth->leFn->length > index)
						return(resultBluetooth->leFn->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leFnMax"))
				{
					if (resultBluetooth->leFnMax && resultBluetooth->leFnMax->length > index)
						return(resultBluetooth->leFnMax->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leDeltaF0FnMax"))
				{
					if (resultBluetooth->leDeltaF0FnMax && resultBluetooth->leDeltaF0FnMax->length > index)
						return(resultBluetooth->leDeltaF0FnMax->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leDeltaF1F0"))
				{
					if (resultBluetooth->leDeltaF1F0 && resultBluetooth->leDeltaF1F0->length > index)
						return(resultBluetooth->leDeltaF1F0->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leDeltaFnFn_5Max"))
				{
					if (resultBluetooth->leDeltaFnFn_5Max && resultBluetooth->leDeltaFnFn_5Max->length > index)
						return(resultBluetooth->leDeltaFnFn_5Max->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leFreqDevSyncAv"))
				{
					if (resultBluetooth->leFreqDevSyncAv && resultBluetooth->leFreqDevSyncAv->length > index)
						return(resultBluetooth->leFreqDevSyncAv->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "lePduLength"))
				{
					if (resultBluetooth->lePduLength && resultBluetooth->lePduLength->length > index)
						return(resultBluetooth->lePduLength->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leIsCrcOk"))
				{
					if (resultBluetooth->leIsCrcOk && resultBluetooth->leIsCrcOk->length > index)
						return(resultBluetooth->leIsCrcOk->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leValid"))
				{
					if (resultBluetooth->leValid && resultBluetooth->leValid->length > index)
						return(resultBluetooth->leValid->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leMaxPowerDbm"))
				{
					if (resultBluetooth->leMaxPowerDbm && resultBluetooth->leMaxPowerDbm->length > index)
						return(resultBluetooth->leMaxPowerDbm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "lePreambleSeq"))
				{
					if (resultBluetooth->lePreambleSeq && resultBluetooth->lePreambleSeq->length > index)
						return(resultBluetooth->lePreambleSeq->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leSyncWordSeq"))
				{
					if (resultBluetooth->leSyncWordSeq && resultBluetooth->leSyncWordSeq->length > index)
						return(resultBluetooth->leSyncWordSeq->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "lePduHeaderSeq"))
				{
					if (resultBluetooth->lePduHeaderSeq && resultBluetooth->lePduHeaderSeq->length > index)
						return(resultBluetooth->lePduHeaderSeq->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "lePduLengthSeq"))
				{
					if (resultBluetooth->lePduLengthSeq && resultBluetooth->lePduLengthSeq->length > index)
						return(resultBluetooth->lePduLengthSeq->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "lePayloadSeq"))
				{
					if (resultBluetooth->lePayloadSeq && resultBluetooth->lePayloadSeq->length > index)
						return(resultBluetooth->lePayloadSeq->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "leCrcSeq"))
				{
					if (resultBluetooth->leCrcSeq && resultBluetooth->leCrcSeq->length > index)
						return(resultBluetooth->leCrcSeq->real[index]);
					else
						return NA_NUMBER;
				}
				else
				{
					return NA_NUMBER;
				}
			}
			// Zigbee
			else if (dynamic_cast<iqapiResultZigbee *>(hndl->results))
			{
				resultZigbee = dynamic_cast<iqapiResultZigbee *>(hndl->results);
				if (!strcmp(measurement, "numSymbols"))
				{
					return((double)resultZigbee->numSymbols);
				}
				if (!strcmp(measurement, "bValid"))
				{
					return((double)resultZigbee->bValid);
				}
				else if (!strcmp(measurement, "rxPeakPower"))
				{
					if (resultZigbee->rxPeakPower && resultZigbee->rxPeakPower->length > index)
						return(resultZigbee->rxPeakPower->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rxRmsPowerAll"))
				{
					if (resultZigbee->rxRmsPowerAll && resultZigbee->rxRmsPowerAll->length > index)
						return(resultZigbee->rxRmsPowerAll->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rxRmsPowerNoGap"))
				{
					if (resultZigbee->rxRmsPowerNoGap && resultZigbee->rxRmsPowerNoGap->length > index)
						return(resultZigbee->rxRmsPowerNoGap->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rxPeakPowerDbm"))
				{
					if (resultZigbee->rxPeakPowerDbm && resultZigbee->rxPeakPowerDbm->length > index)
						return(resultZigbee->rxPeakPowerDbm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rxRmsPowerAllDbm"))
				{
					if (resultZigbee->rxRmsPowerAllDbm && resultZigbee->rxRmsPowerAllDbm->length > index)
						return(resultZigbee->rxRmsPowerAllDbm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rxRmsPowerNoGapDbm"))
				{
					if (resultZigbee->rxRmsPowerNoGapDbm && resultZigbee->rxRmsPowerNoGapDbm->length > index)
						return(resultZigbee->rxRmsPowerNoGapDbm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "sigType"))
				{
					if (resultZigbee->sigType && resultZigbee->sigType->length > index)
						return(resultZigbee->sigType->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "fsWaveHz"))
				{
					if (resultZigbee->fsWaveHz && resultZigbee->fsWaveHz->length > index)
						return(resultZigbee->fsWaveHz->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "overSampling"))
				{
					if (resultZigbee->overSampling && resultZigbee->overSampling->length > index)
						return(resultZigbee->overSampling->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "evmAll"))
				{
					if (resultZigbee->evmAll && resultZigbee->evmAll->length > index)
						return(resultZigbee->evmAll->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "evmAllOffset"))
				{
					if (resultZigbee->evmAllOffset && resultZigbee->evmAllOffset->length > index)
						return(resultZigbee->evmAllOffset->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "phaseNoiseDegRmsAll"))
				{
					if (resultZigbee->phaseNoiseDegRmsAll && resultZigbee->phaseNoiseDegRmsAll->length > index)
						return(resultZigbee->phaseNoiseDegRmsAll->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "freqOffsetFineHz"))
				{
					if (resultZigbee->freqOffsetFineHz && resultZigbee->freqOffsetFineHz->length > index)
						return(resultZigbee->freqOffsetFineHz->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "symClockErrorPpm"))
				{
					if (resultZigbee->symClockErrorPpm && resultZigbee->symClockErrorPpm->length > index)
						return(resultZigbee->symClockErrorPpm->real[index]);
					else
						return NA_NUMBER;
				}
#if defined(IQAPI_1_5_X)
				else if (!strcmp(measurement, "avgPsdu"))
				{
					if (resultZigbee->avgPsdu && resultZigbee->avgPsdu->length > index)
						return(resultZigbee->avgPsdu->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "avgShrPhr"))
				{
					if (resultZigbee->avgShrPhr && resultZigbee->avgShrPhr->length > index)
						return(resultZigbee->avgShrPhr->real[index]);
					else
						return NA_NUMBER;
				}
#else
				//RW 01/14/09: IQapi 1.6.2 changed "avgPsdu" to "evmPsdu", "avgShrPhr" to "evmShrPhr"
				else if (!strcmp(measurement, "evmPsdu"))
				{
					if (resultZigbee->evmPsdu && resultZigbee->evmPsdu->length > index)
						return(resultZigbee->evmPsdu->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "evmShrPhr"))
				{
					if (resultZigbee->evmShrPhr && resultZigbee->evmShrPhr->length > index)
						return(resultZigbee->evmShrPhr->real[index]);
					else
						return NA_NUMBER;
				}
#endif
			}
			else if (dynamic_cast<iqapiResultWave *>(hndl->results))
			{
				resultwave = dynamic_cast<iqapiResultWave *>(hndl->results);

				if (!strcmp(measurement, "dcDc"))
				{
					if (resultwave->dcDc && resultwave->dcDc->length > index)
						return(resultwave->dcDc->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "dcRms"))
				{
					if (resultwave->dcRms && resultwave->dcRms->length > index)
						return(resultwave->dcRms->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "dcMin"))
				{
					if (resultwave->dcMin && resultwave->dcMin->length > index)
						return(resultwave->dcMin->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "dcMax"))
				{
					if (resultwave->dcMax && resultwave->dcMax->length > index)
						return(resultwave->dcMax->real[index]);
					else
						return NA_NUMBER;
				}
				//			else if (!strcmp(measurement, "dcPkToPk"))
				//			{
				//				if (resultwave->dcPkToPk && resultwave->dcPkToPk->length > index)
				//					return(resultwave->dcPkToPk->real[index]);
				//				else
				//					return NA_NUMBER;
				//			}
				//			else if (!strcmp(measurement, "dcRmsI"))
				//			{
				//				if (resultwave->dcRmsI && resultwave->dcRmsI->length > index)
				//					return(resultwave->dcRmsI->real[index]);
				//				else
				//					return NA_NUMBER;
				//			}
				//			else if (!strcmp(measurement, "dcRmsQ"))
				//			{
				//				if (resultwave->dcRmsQ && resultwave->dcRmsQ->length > index)
				//					return(resultwave->dcRmsQ->real[index]);
				//				else
				//					return NA_NUMBER;
				//			}
				else if (!strcmp(measurement, "acDc"))
				{
					if (resultwave->acDc && resultwave->acDc->length > index)
						return(resultwave->acDc->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "acRms"))
				{
					if (resultwave->acRms && resultwave->acRms->length > index)
						return(resultwave->acRms->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "acMin"))
				{
					if (resultwave->acMin && resultwave->acMin->length > index)
						return(resultwave->acMin->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "acMax"))
				{
					if (resultwave->acMax && resultwave->acMax->length > index)
						return(resultwave->acMax->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "acPkToPk"))
				{
					if (resultwave->acPkToPk && resultwave->acPkToPk->length > index)
						return(resultwave->acPkToPk->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "acRmsI"))
				{
					if (resultwave->acRmsI && resultwave->acRmsI->length > index)
						return(resultwave->acRmsI->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "acRmsQ"))
				{
					if (resultwave->acRmsQ && resultwave->acRmsQ->length > index)
						return(resultwave->acRmsQ->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "rmsDb"))
				{
					if (resultwave->rmsDb && resultwave->rmsDb->length > index)
						return(resultwave->rmsDb->real[index]);
					else
						return NA_NUMBER;
				}
			}
			else if (dynamic_cast<iqapiResultCW *>(hndl->results))
			{
				resultCW = dynamic_cast<iqapiResultCW *>(hndl->results);
				if (!strcmp(measurement, "frequency"))
				{
					return resultCW->frequency;
				}
			}
			else
				return NA_NUMBER;
		}

	}
	return NA_NUMBER;
}

IQMEASURE_API double LP_GetScalarMeasurement(char *measurement, int index)
{
	::TIMER_StartTimer(timerIQmeasure, "LP_GetScalarMeasurement", &timeStart);

	double value = LP_GetScalarMeasurement_NoTimer(measurement, index);

	::TIMER_StopTimer(timerIQmeasure, "LP_GetScalarMeasurement", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_GetScalarMeasurement", timeDuration);

	return value;
}

// Keep this function with typo for backward compatibility
IQMEASURE_API int LP_GetVectorMeasurment(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	return LP_GetVectorMeasurement(measurement, bufferReal, bufferImag, bufferLength);
}

/*! @defgroup group_vector_measurement Available Measurement Names for LP_GetVectorMeasurement()
 *
 * Available measurement names vary for various analysis.  After an analysis has been performed successfully, by calling
 * one of the following functions:
 *   - LP_Analyze80211ag();
 *   - LP_AnalyzeMimo();
 *   - LP_Analyze80211b();
 *   - LP_Analyze80216d();
 *   - LP_Analyze80216e();
 *   - LP_AnalyzePower();
 *   - LP_AnalyzeFFT();
 *   - LP_AnalyzeCCDF();
 *   - LP_AnalyzeCW();
 *   - LP_AnalysisWave();
 *   - LP_AnalyzeSidelobe();
 *   - LP_AnalyzePowerRampOFDM();
 *   - LP_AnalyzePowerRamp80211b();
 *   - LP_AnalyzeBluetooth();
 *   - LP_AnalyzeZigbee();
 */
IQMEASURE_API int LP_GetVectorMeasurement_NoTimer(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	if (LibsInitialized)
	{
		if (!hndl->results || !bufferReal || !bufferLength)
		{
			return 0;
		}
		else
		{
			if (dynamic_cast<iqapiResultOFDM *>(hndl->results))
			{
				resultOfdm = dynamic_cast<iqapiResultOFDM *>(hndl->results);

				if (!strcmp(measurement, "hhEst"))
				{
					if (resultOfdm->hhEst && resultOfdm->hhEst->length)
					{
						if (bufferLength > resultOfdm->hhEst->length)
							bufferLength = resultOfdm->hhEst->length;

						memcpy(bufferReal, resultOfdm->hhEst->real, (sizeof(double)*bufferLength));

						if (resultOfdm->hhEst->imag && bufferImag)
							memcpy(bufferImag, resultOfdm->hhEst->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "spectralFlatness"))
				{
					if (resultOfdm->spectralFlatness && resultOfdm->spectralFlatness->length)
					{
						if (bufferLength > resultOfdm->spectralFlatness->length)
							bufferLength = resultOfdm->spectralFlatness->length;

						memcpy(bufferReal, resultOfdm->spectralFlatness->real, (sizeof(double)*bufferLength));

						if (resultOfdm->spectralFlatness->imag && bufferImag)
						memcpy(bufferImag, resultOfdm->spectralFlatness->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;

				}
				else if (!strcmp(measurement, "spectralFlatnessHighLimit"))
				{
					if (resultOfdm->spectralFlatnessHighLimit && resultOfdm->spectralFlatnessHighLimit->length)
					{
						if (bufferLength > resultOfdm->spectralFlatnessHighLimit->length)
							bufferLength = resultOfdm->spectralFlatnessHighLimit->length;

						memcpy(bufferReal, resultOfdm->spectralFlatnessHighLimit->real, (sizeof(double)*bufferLength));

						if (resultOfdm->spectralFlatnessHighLimit->imag && bufferImag)
						memcpy(bufferImag, resultOfdm->spectralFlatnessHighLimit->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;

				}
				else if (!strcmp(measurement, "spectralFlatnessLowLimit"))
				{
					if (resultOfdm->spectralFlatnessLowLimit && resultOfdm->spectralFlatnessLowLimit->length)
					{
						if (bufferLength > resultOfdm->spectralFlatnessLowLimit->length)
							bufferLength = resultOfdm->spectralFlatnessLowLimit->length;

						memcpy(bufferReal, resultOfdm->spectralFlatnessLowLimit->real, (sizeof(double)*bufferLength));

						if (resultOfdm->spectralFlatnessLowLimit->imag && bufferImag)
							memcpy(bufferImag, resultOfdm->spectralFlatnessLowLimit->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;

				}
				else if (!strcmp(measurement, "psdu"))
				{
					if (resultOfdm->psdu && resultOfdm->psdu->length)
					{
						if (bufferLength > resultOfdm->psdu->length)
							bufferLength = resultOfdm->psdu->length;

						memcpy(bufferReal, resultOfdm->psdu->real, (sizeof(double)*bufferLength));

						if (resultOfdm->psdu->imag && bufferImag)
							memcpy(bufferImag, resultOfdm->psdu->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "startPointers"))
				{
					if (resultOfdm->startPointers && resultOfdm->startPointers->length)
					{
						if (bufferLength > resultOfdm->startPointers->length)
							bufferLength = resultOfdm->startPointers->length;

						memcpy(bufferReal, resultOfdm->startPointers->real, (sizeof(double)*bufferLength));

						if (resultOfdm->startPointers->imag && bufferImag)
							memcpy(bufferImag, resultOfdm->startPointers->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "plcp"))
				{
					if (resultOfdm->plcp && resultOfdm->plcp->length)
					{
						if (bufferLength > resultOfdm->plcp->length)
							bufferLength = resultOfdm->plcp->length;

						memcpy(bufferReal, resultOfdm->plcp->real, (sizeof(double)*bufferLength));

						if (resultOfdm->plcp->imag && bufferImag)
							memcpy(bufferImag, resultOfdm->plcp->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
				}
			}

			if (dynamic_cast<iqapiResultWave *>(hndl->results))
			{
				resultwave = dynamic_cast<iqapiResultWave *>(hndl->results);

				if (!strcmp(measurement, "dcPkToPk"))
				{
					if (resultwave->dcPkToPk && resultwave->dcPkToPk->length)
					{
						if (bufferLength > resultwave->dcPkToPk->length)
							bufferLength = resultwave->dcPkToPk->length;

						memcpy(bufferReal, resultwave->dcPkToPk->real, (sizeof(double)*bufferLength));

						if (resultwave->dcPkToPk->imag && bufferImag)
							memcpy(bufferImag, resultwave->dcPkToPk->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
				}
				if (!strcmp(measurement, "dcRmsI"))
				{
					if (resultwave->dcRmsI && resultwave->dcRmsI->length)
					{
						if (bufferLength > resultwave->dcRmsI->length)
							bufferLength = resultwave->dcRmsI->length;

						memcpy(bufferReal, resultwave->dcRmsI->real, (sizeof(double)*bufferLength));

						if (resultwave->dcRmsI->imag && bufferImag)
							memcpy(bufferImag, resultwave->dcRmsI->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
				}
				if (!strcmp(measurement, "dcRmsQ"))
				{
					if (resultwave->dcRmsQ && resultwave->dcRmsQ->length)
					{
						if (bufferLength > resultwave->dcRmsQ->length)
							bufferLength = resultwave->dcRmsQ->length;

						memcpy(bufferReal, resultwave->dcRmsQ->real, (sizeof(double)*bufferLength));

						if (resultwave->dcRmsQ->imag && bufferImag)
							memcpy(bufferImag, resultwave->dcRmsQ->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
				}
			}
			if (dynamic_cast<iqapiResult11b *>(hndl->results))
			{
				result11b = dynamic_cast<iqapiResult11b *>(hndl->results);

				if (!strcmp(measurement, "evmInPlcp"))
				{
					if (result11b->evmInPlcp && result11b->evmInPlcp->length)
					{
						if (bufferLength > result11b->evmInPlcp->length)
							bufferLength = result11b->evmInPlcp->length;

						memcpy(bufferReal, result11b->evmInPlcp->real, (sizeof(double)*bufferLength));

						if (result11b->evmInPlcp->imag && bufferImag)
							memcpy(bufferImag, result11b->evmInPlcp->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "evmErr"))
				{
					if (result11b->evmErr && result11b->evmErr->length)
					{
						if (bufferLength > result11b->evmErr->length)
							bufferLength = result11b->evmErr->length;

						memcpy(bufferReal, result11b->evmErr->real, (sizeof(double)*bufferLength));

						if (result11b->evmErr->imag && bufferImag)
							memcpy(bufferImag, result11b->evmErr->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "loLeakageDb"))
				{
					if (result11b->loLeakageDb && result11b->loLeakageDb->length)
					{
						if (bufferLength > result11b->loLeakageDb->length)
							bufferLength = result11b->loLeakageDb->length;

						memcpy(bufferReal, result11b->loLeakageDb->real, (sizeof(double)*bufferLength));

						if (result11b->loLeakageDb->imag && bufferImag)
							memcpy(bufferImag, result11b->loLeakageDb->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
				}
			}
			else if (dynamic_cast<iqapiResultFFT *>(hndl->results))
			{
				resultFFT = dynamic_cast<iqapiResultFFT *>(hndl->results);

				if (!strcmp(measurement, "x"))
				{
					if (resultFFT->x && resultFFT->x->length)
					{
						if (bufferLength > resultFFT->x->length)
							bufferLength = resultFFT->x->length;

						memcpy(bufferReal, resultFFT->x->real, (sizeof(double)*bufferLength));

						if (resultFFT->x->imag && bufferImag)
							memcpy(bufferImag, resultFFT->x->imag, (sizeof(double)*bufferLength));

						return(bufferLength);

					}
					else
						return 0;
				}

				else if (!strcmp(measurement, "y"))
				{
					if (resultFFT->y && resultFFT->y->length)
					{
						if (bufferLength > resultFFT->y->length)
							bufferLength = resultFFT->y->length;

						memcpy(bufferReal, resultFFT->y->real, (sizeof(double)*bufferLength));

						if (resultFFT->y->imag && bufferImag)
							memcpy(bufferImag, resultFFT->y->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
			}
			else if (dynamic_cast<iqapiResult80216 *>(hndl->results))
			{
				result80216 = dynamic_cast<iqapiResult80216 *>(hndl->results);

				if (!strcmp(measurement, "channelEst"))
				{
					if (result80216->channelEst && result80216->channelEst->length)
					{
						if (bufferLength >result80216->channelEst->length)
							bufferLength = result80216->channelEst->length;

						memcpy(bufferReal, result80216->channelEst->real, (sizeof(double)*bufferLength));

						if (result80216->channelEst->imag && bufferImag)
							memcpy(bufferImag, result80216->channelEst->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				if (!strcmp(measurement, "analyzedRange"))
				{
					if (result80216->analyzedRange && result80216->analyzedRange->length)
					{
						if (bufferLength >result80216->analyzedRange->length)
							bufferLength = result80216->analyzedRange->length;

						memcpy(bufferReal, result80216->analyzedRange->real, (sizeof(double)*bufferLength));

						if (result80216->channelEst->imag && bufferImag)
							memcpy(bufferImag, result80216->analyzedRange->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				//				if (!strcmp(measurement, "numSymbols"))
				//				{
				//					if (result80216->numSymbols && result80216->numSymbols->length)
				//					{
				//						if (bufferLength >result80216->numSymbols->length)
				//							bufferLength = result80216->numSymbols->length;

				//						memcpy(bufferReal, result80216->numSymbols->real, (sizeof(double)*bufferLength));
				//
				//						if (result80216->numSymbols->imag && bufferImag)
				//							memcpy(bufferImag, result80216->numSymbols->imag, (sizeof(double)*bufferLength));

				//			    		return(bufferLength);
				//			 	}
				//					else
				//						return 0;
				//				}
				if (!strcmp(measurement, "rateId"))
				{
					if (result80216->rateId && result80216->rateId->length)
					{
						if (bufferLength >result80216->rateId->length)
							bufferLength = result80216->rateId->length;

						memcpy(bufferReal, result80216->rateId->real, (sizeof(double)*bufferLength));

						if (result80216->rateId->imag && bufferImag)
							memcpy(bufferImag, result80216->rateId->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				if (!strcmp(measurement, "demodSymbols"))
				{
					if (result80216->demodSymbols && result80216->demodSymbols->length)
					{
						if (bufferLength >result80216->demodSymbols->length)
							bufferLength = result80216->demodSymbols->length;

						memcpy(bufferReal, result80216->demodSymbols->real, (sizeof(double)*bufferLength));

						if (result80216->demodSymbols->imag && bufferImag)
							memcpy(bufferImag, result80216->demodSymbols->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else
				{
					return 0;
				}
			}
			//Z-99
			//else if (dynamic_cast<iqapiResult11ac *>(hndl->results))
			else if (dynamic_cast<iqapiResultWifiOfdm *>(hndl->results))
			{
				//Z-99
				//result11ac = dynamic_cast<iqapiResult11ac *>(hndl->results);
				result11ac = dynamic_cast<iqapiResultWifiOfdm *>(hndl->results);

				if (!strcmp(measurement, "channelEst"))
				{
					if (result11ac->channelEst && result11ac->channelEst->length)
					{
						if (bufferLength > result11ac->channelEst->length)
							bufferLength = result11ac->channelEst->length;

						memcpy(bufferReal, result11ac->channelEst->real, (sizeof(double)*bufferLength));

						if (result11ac->channelEst->imag && bufferImag)
							memcpy(bufferImag, result11ac->channelEst->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "spectralFlatness"))
				{
					if (result11ac->spectralFlatness && result11ac->spectralFlatness->length)
					{
						if (bufferLength > result11ac->spectralFlatness->length)
							bufferLength = result11ac->spectralFlatness->length;

						memcpy(bufferReal, result11ac->spectralFlatness->real, (sizeof(double)*bufferLength));

						if (result11ac->spectralFlatness->imag && bufferImag)
							memcpy(bufferImag, result11ac->spectralFlatness->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;

				}
				else if (!strcmp(measurement, "spectralFlatnessHighLimit"))
				{
					if (result11ac->spectralFlatnessHighLimit && result11ac->spectralFlatnessHighLimit->length)
					{
						if (bufferLength > result11ac->spectralFlatnessHighLimit->length)
							bufferLength = result11ac->spectralFlatnessHighLimit->length;

						memcpy(bufferReal, result11ac->spectralFlatnessHighLimit->real, (sizeof(double)*bufferLength));

						if (result11ac->spectralFlatnessHighLimit->imag && bufferImag)
							memcpy(bufferImag, result11ac->spectralFlatnessHighLimit->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;

				}
				else if (!strcmp(measurement, "spectralFlatnessLowLimit"))
				{
					if (result11ac->spectralFlatnessLowLimit && result11ac->spectralFlatnessLowLimit->length)
					{
						if (bufferLength > result11ac->spectralFlatnessLowLimit->length)
							bufferLength = result11ac->spectralFlatnessLowLimit->length;

						memcpy(bufferReal, result11ac->spectralFlatnessLowLimit->real, (sizeof(double)*bufferLength));

						if (result11ac->spectralFlatnessLowLimit->imag && bufferImag)
							memcpy(bufferImag, result11ac->spectralFlatnessLowLimit->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;

				}
				else if (!strcmp(measurement, "rxRmsPowerDb"))
				{
					if (result11ac->rxRmsPowerDb && result11ac->rxRmsPowerDb->length)
					{
						if (bufferLength > result11ac->rxRmsPowerDb->length)
							bufferLength = result11ac->rxRmsPowerDb->length;

						//double *tempBuffer = new double[bufferLength];
						//if( analysisMimo->SequentialMimo.numSections>0 )
						//{
						//	for(int i=0; i<result11ac->rxRmsPowerDb->length; i++)
						//	{
						//		tempBuffer[i] = result11ac->rxRmsPowerDb->real[i] + attn[i/analysisMimo->SequentialMimo.numSections];
						//	}
						//}
						//memcpy(bufferReal, tempBuffer, (sizeof(double)*bufferLength));
						//delete[] tempBuffer;

						memcpy(bufferReal, result11ac->rxRmsPowerDb->real, (sizeof(double)*bufferLength));

						if (result11ac->rxRmsPowerDb->imag && bufferImag)
							memcpy(bufferImag, result11ac->rxRmsPowerDb->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;

				}
				else if (!strcmp(measurement, "isolationDb"))
				{
					if(result11ac->isolationDb && result11ac->isolationDb->length)
					{
						if (bufferLength > result11ac->isolationDb->length)
							bufferLength = result11ac->isolationDb->length;

						memcpy(bufferReal, result11ac->isolationDb->real, (sizeof(double)*bufferLength));

						if (result11ac->isolationDb->imag && bufferImag)
							memcpy(bufferImag, result11ac->isolationDb->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "evmSymbols"))
				{
					if (result11ac->evmSymbols && result11ac->evmSymbols->length)
					{
						if (bufferLength > result11ac->evmSymbols->length)
							bufferLength = result11ac->evmSymbols->length;

						memcpy(bufferReal, result11ac->evmSymbols->real, (sizeof(double)*bufferLength));

						if (result11ac->evmSymbols->imag && bufferImag)
							memcpy(bufferImag, result11ac->evmSymbols->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "evmTones"))
				{
					if (result11ac->evmTones && result11ac->evmTones->length)
					{
						if (bufferLength > result11ac->evmTones->length)
							bufferLength = result11ac->evmTones->length;

						memcpy(bufferReal, result11ac->evmTones->real, (sizeof(double)*bufferLength));

						if (result11ac->evmTones->imag && bufferImag)
							memcpy(bufferImag, result11ac->evmTones->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "PhaseNoiseDeg_Symbols"))
				{
					if (result11ac->PhaseNoiseDeg_Symbols && result11ac->PhaseNoiseDeg_Symbols->length)
					{
						if (bufferLength > result11ac->PhaseNoiseDeg_Symbols->length)
							bufferLength = result11ac->PhaseNoiseDeg_Symbols->length;

						memcpy(bufferReal, result11ac->PhaseNoiseDeg_Symbols->real, (sizeof(double)*bufferLength));

						if (result11ac->PhaseNoiseDeg_Symbols->imag && bufferImag)
							memcpy(bufferImag, result11ac->PhaseNoiseDeg_Symbols->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "demodSymbols"))
				{
					if (result11ac->demodSymbols && result11ac->demodSymbols->length)
					{
						if (bufferLength > result11ac->demodSymbols->length)
							bufferLength = result11ac->demodSymbols->length;

						memcpy(bufferReal, result11ac->demodSymbols->real, (sizeof(double)*bufferLength));

						if (result11ac->demodSymbols->imag && bufferImag)
							memcpy(bufferImag, result11ac->demodSymbols->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "psduBits"))
				{
					if (result11ac->psduBits && result11ac->psduBits->length)
					{
						if (bufferLength > result11ac->psduBits->length)
							bufferLength = result11ac->psduBits->length;

						memcpy(bufferReal, result11ac->psduBits->real, (sizeof(double)*bufferLength));

						if (result11ac->psduBits->imag && bufferImag)
							memcpy(bufferImag, result11ac->psduBits->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
			else if (dynamic_cast<iqapiResultMimo *>(hndl->results))
			{
				resultMimo = dynamic_cast<iqapiResultMimo *>(hndl->results);

				if (!strcmp(measurement, "channelEst"))
				{
					if (resultMimo->channelEst && resultMimo->channelEst->length)
					{
						if (bufferLength > resultMimo->channelEst->length)
							bufferLength = resultMimo->channelEst->length;

						memcpy(bufferReal, resultMimo->channelEst->real, (sizeof(double)*bufferLength));

						if (resultMimo->channelEst->imag && bufferImag)
							memcpy(bufferImag, resultMimo->channelEst->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "spectralFlatness"))
				{
					if (resultMimo->spectralFlatness && resultMimo->spectralFlatness->length)
					{
						if (bufferLength > resultMimo->spectralFlatness->length)
							bufferLength = resultMimo->spectralFlatness->length;

						memcpy(bufferReal, resultMimo->spectralFlatness->real, (sizeof(double)*bufferLength));

						if (resultMimo->spectralFlatness->imag && bufferImag)
						memcpy(bufferImag, resultMimo->spectralFlatness->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;

				}
				else if (!strcmp(measurement, "spectralFlatnessHighLimit"))
				{
					if (resultMimo->spectralFlatnessHighLimit && resultMimo->spectralFlatnessHighLimit->length)
					{
						if (bufferLength > resultMimo->spectralFlatnessHighLimit->length)
							bufferLength = resultMimo->spectralFlatnessHighLimit->length;

						memcpy(bufferReal, resultMimo->spectralFlatnessHighLimit->real, (sizeof(double)*bufferLength));

						if (resultMimo->spectralFlatnessHighLimit->imag && bufferImag)
						memcpy(bufferImag, resultMimo->spectralFlatnessHighLimit->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;

				}
				else if (!strcmp(measurement, "spectralFlatnessLowLimit"))
				{
					if (resultMimo->spectralFlatnessLowLimit && resultMimo->spectralFlatnessLowLimit->length)
					{
						if (bufferLength > resultMimo->spectralFlatnessLowLimit->length)
							bufferLength = resultMimo->spectralFlatnessLowLimit->length;

						memcpy(bufferReal, resultMimo->spectralFlatnessLowLimit->real, (sizeof(double)*bufferLength));

						if (resultMimo->spectralFlatnessLowLimit->imag && bufferImag)
							memcpy(bufferImag, resultMimo->spectralFlatnessLowLimit->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;

				}
				else if (!strcmp(measurement, "rxRmsPowerDb"))
				{
					if (resultMimo->rxRmsPowerDb && resultMimo->rxRmsPowerDb->length)
					{
						if (bufferLength > resultMimo->rxRmsPowerDb->length)
							bufferLength = resultMimo->rxRmsPowerDb->length;

						//double *tempBuffer = new double[bufferLength];
						//if( analysisMimo->SequentialMimo.numSections>0 )
						//{
						//	for(int i=0; i<resultMimo->rxRmsPowerDb->length; i++)
						//	{
						//		tempBuffer[i] = resultMimo->rxRmsPowerDb->real[i] + attn[i/analysisMimo->SequentialMimo.numSections];
						//	}
						//}
						//memcpy(bufferReal, tempBuffer, (sizeof(double)*bufferLength));
						//delete[] tempBuffer;

						memcpy(bufferReal, resultMimo->rxRmsPowerDb->real, (sizeof(double)*bufferLength));

						if (resultMimo->rxRmsPowerDb->imag && bufferImag)
							memcpy(bufferImag, resultMimo->rxRmsPowerDb->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;

				}
				else if (!strcmp(measurement, "isolationDb"))
				{
					if (resultMimo->isolationDb && resultMimo->isolationDb->length)
					{
						if (bufferLength > resultMimo->isolationDb->length)
							bufferLength = resultMimo->isolationDb->length;

						memcpy(bufferReal, resultMimo->isolationDb->real, (sizeof(double)*bufferLength));

						if (resultMimo->isolationDb->imag && bufferImag)
							memcpy(bufferImag, resultMimo->isolationDb->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "evmSymbols"))
				{
					if (resultMimo->evmSymbols && resultMimo->evmSymbols->length)
					{
						if (bufferLength > resultMimo->evmSymbols->length)
							bufferLength = resultMimo->evmSymbols->length;

						memcpy(bufferReal, resultMimo->evmSymbols->real, (sizeof(double)*bufferLength));

						if (resultMimo->evmSymbols->imag && bufferImag)
							memcpy(bufferImag, resultMimo->evmSymbols->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "evmTones"))
				{
					if (resultMimo->evmTones && resultMimo->evmTones->length)
					{
						if (bufferLength > resultMimo->evmTones->length)
							bufferLength = resultMimo->evmTones->length;

						memcpy(bufferReal, resultMimo->evmTones->real, (sizeof(double)*bufferLength));

						if (resultMimo->evmTones->imag && bufferImag)
							memcpy(bufferImag, resultMimo->evmTones->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "PhaseNoiseDeg_Symbols"))
				{
					if (resultMimo->PhaseNoiseDeg_Symbols && resultMimo->PhaseNoiseDeg_Symbols->length)
					{
						if (bufferLength > resultMimo->PhaseNoiseDeg_Symbols->length)
							bufferLength = resultMimo->PhaseNoiseDeg_Symbols->length;

						memcpy(bufferReal, resultMimo->PhaseNoiseDeg_Symbols->real, (sizeof(double)*bufferLength));

						if (resultMimo->PhaseNoiseDeg_Symbols->imag && bufferImag)
							memcpy(bufferImag, resultMimo->PhaseNoiseDeg_Symbols->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "demodSymbols"))
				{
					if (resultMimo->demodSymbols && resultMimo->demodSymbols->length)
					{
						if (bufferLength > resultMimo->demodSymbols->length)
							bufferLength = resultMimo->demodSymbols->length;

						memcpy(bufferReal, resultMimo->demodSymbols->real, (sizeof(double)*bufferLength));

						if (resultMimo->demodSymbols->imag && bufferImag)
							memcpy(bufferImag, resultMimo->demodSymbols->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else
				{
					return 0;
				}
			}
			else if (dynamic_cast<iqapiResultZigbee *>(hndl->results))
			{
				resultZigbee = dynamic_cast<iqapiResultZigbee *>(hndl->results);

				if (!strcmp(measurement, "evmSymbols"))
				{
					if (resultZigbee->evmSymbols && resultZigbee->evmSymbols->length)
					{
						if (bufferLength > resultZigbee->evmSymbols->length)
							bufferLength = resultZigbee->evmSymbols->length;

						memcpy(bufferReal, resultZigbee->evmSymbols->real, (sizeof(double)*bufferLength));

						if (resultZigbee->evmSymbols->imag && bufferImag)
							memcpy(bufferImag, resultZigbee->evmSymbols->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "constData"))
				{
					if (resultZigbee->constData && resultZigbee->constData->length)
					{
						if (bufferLength > resultZigbee->constData->length)
							bufferLength = resultZigbee->constData->length;

						memcpy(bufferReal, resultZigbee->constData->real, (sizeof(double)*bufferLength));

						if (resultZigbee->constData->imag && bufferImag)
							memcpy(bufferImag, resultZigbee->constData->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "phaseNoiseDegError"))
				{
					if (resultZigbee->phaseNoiseDegError && resultZigbee->phaseNoiseDegError->length)
					{
						if (bufferLength > resultZigbee->phaseNoiseDegError->length)
							bufferLength = resultZigbee->phaseNoiseDegError->length;

						memcpy(bufferReal, resultZigbee->phaseNoiseDegError->real, (sizeof(double)*bufferLength));

						if (resultZigbee->phaseNoiseDegError->imag && bufferImag)
							memcpy(bufferImag, resultZigbee->phaseNoiseDegError->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "eyeDiagramData"))
				{
					if (resultZigbee->eyeDiagramData && resultZigbee->eyeDiagramData->length)
					{
						if (bufferLength > resultZigbee->eyeDiagramData->length)
							bufferLength = resultZigbee->eyeDiagramData->length;

						memcpy(bufferReal, resultZigbee->eyeDiagramData->real, (sizeof(double)*bufferLength));

						if (resultZigbee->eyeDiagramData->imag && bufferImag)
							memcpy(bufferImag, resultZigbee->eyeDiagramData->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "eyeDiagramTime"))
				{
					if (resultZigbee->eyeDiagramTime && resultZigbee->eyeDiagramTime->length)
					{
						if (bufferLength > resultZigbee->eyeDiagramTime->length)
							bufferLength = resultZigbee->eyeDiagramTime->length;

						memcpy(bufferReal, resultZigbee->eyeDiagramTime->real, (sizeof(double)*bufferLength));

						if (resultZigbee->eyeDiagramTime->imag && bufferImag)
							memcpy(bufferImag, resultZigbee->eyeDiagramTime->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "codeDomain"))
				{
					if (resultZigbee->codeDomain && resultZigbee->codeDomain->length)
					{
						if (bufferLength > resultZigbee->codeDomain->length)
							bufferLength = resultZigbee->codeDomain->length;

						memcpy(bufferReal, resultZigbee->codeDomain->real, (sizeof(double)*bufferLength));

						if (resultZigbee->codeDomain->imag && bufferImag)
							memcpy(bufferImag, resultZigbee->codeDomain->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else
				{
					return 0;
				}
			}




			else if (dynamic_cast<iqapiResultCCDF *>(hndl->results))
			{
				resultCCDF = dynamic_cast<iqapiResultCCDF *>(hndl->results);

				if (!strcmp(measurement, "prob"))
				{
					if (resultCCDF->prob && resultCCDF->prob->length)
					{
						if (bufferLength > resultCCDF->prob->length)
							bufferLength = resultCCDF->prob->length;

						memcpy(bufferReal, resultCCDF->prob->real, (sizeof(double)*bufferLength));

						if (resultCCDF->prob->imag && bufferImag)
							memcpy(bufferImag, resultCCDF->prob->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "power_rel_dB"))
				{
					if (resultCCDF->power_rel_dB && resultCCDF->power_rel_dB->length)
					{
						if (bufferLength > resultCCDF->power_rel_dB->length)
							bufferLength = resultCCDF->power_rel_dB->length;

						memcpy(bufferReal, resultCCDF->power_rel_dB->real, (sizeof(double)*bufferLength));

						if (resultCCDF->power_rel_dB->imag && bufferImag)
							memcpy(bufferImag, resultCCDF->power_rel_dB->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
			}
			else if (dynamic_cast<iqapiResultPowerRamp *>(hndl->results))
			{
				resultPowerRamp = dynamic_cast<iqapiResultPowerRamp *>(hndl->results);

				if (!strcmp(measurement, "on_mask_x"))
				{
					if (resultPowerRamp->on_mask_x && resultPowerRamp->on_mask_x->length)
					{
						if (bufferLength > resultPowerRamp->on_mask_x->length)
							bufferLength = resultPowerRamp->on_mask_x->length;

						memcpy(bufferReal, resultPowerRamp->on_mask_x->real, (sizeof(double)*bufferLength));

						if (resultPowerRamp->on_mask_x->imag && bufferImag)
							memcpy(bufferImag, resultPowerRamp->on_mask_x->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "off_mask_x"))
				{
					if (resultPowerRamp->off_mask_x && resultPowerRamp->off_mask_x->length)
					{
						if (bufferLength > resultPowerRamp->off_mask_x->length)
							bufferLength = resultPowerRamp->off_mask_x->length;

						memcpy(bufferReal, resultPowerRamp->off_mask_x->real, (sizeof(double)*bufferLength));

						if (resultPowerRamp->off_mask_x->imag && bufferImag)
							memcpy(bufferImag, resultPowerRamp->off_mask_x->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "on_mask_y"))
				{
					if (resultPowerRamp->on_mask_y && resultPowerRamp->on_mask_y->length)
					{
						if (bufferLength > resultPowerRamp->on_mask_y->length)
							bufferLength = resultPowerRamp->on_mask_y->length;

						memcpy(bufferReal, resultPowerRamp->on_mask_y->real, (sizeof(double)*bufferLength));

						if (resultPowerRamp->on_mask_y->imag && bufferImag)
							memcpy(bufferImag, resultPowerRamp->on_mask_y->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "off_mask_y"))
				{
					if (resultPowerRamp->off_mask_y && resultPowerRamp->off_mask_y->length)
					{
						if (bufferLength > resultPowerRamp->off_mask_y->length)
							bufferLength = resultPowerRamp->off_mask_y->length;

						memcpy(bufferReal, resultPowerRamp->off_mask_y->real, (sizeof(double)*bufferLength));

						if (resultPowerRamp->off_mask_y->imag && bufferImag)
							memcpy(bufferImag, resultPowerRamp->off_mask_y->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "on_power_inst"))
				{
					if (resultPowerRamp->on_power_inst && resultPowerRamp->on_power_inst->length)
					{
						if (bufferLength > resultPowerRamp->on_power_inst->length)
							bufferLength = resultPowerRamp->on_power_inst->length;

						memcpy(bufferReal, resultPowerRamp->on_power_inst->real, (sizeof(double)*bufferLength));

						if (resultPowerRamp->on_power_inst->imag && bufferImag)
							memcpy(bufferImag, resultPowerRamp->on_power_inst->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "off_power_inst"))
				{
					if (resultPowerRamp->off_power_inst && resultPowerRamp->off_power_inst->length)
					{
						if (bufferLength > resultPowerRamp->off_power_inst->length)
							bufferLength = resultPowerRamp->off_power_inst->length;

						memcpy(bufferReal, resultPowerRamp->off_power_inst->real, (sizeof(double)*bufferLength));

						if (resultPowerRamp->off_power_inst->imag && bufferImag)
							memcpy(bufferImag, resultPowerRamp->off_power_inst->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "on_power_peak"))
				{
					if (resultPowerRamp->on_power_peak && resultPowerRamp->on_power_peak->length)
					{
						if (bufferLength > resultPowerRamp->on_power_peak->length)
							bufferLength = resultPowerRamp->on_power_peak->length;

						memcpy(bufferReal, resultPowerRamp->on_power_peak->real, (sizeof(double)*bufferLength));

						if (resultPowerRamp->on_power_peak->imag && bufferImag)
							memcpy(bufferImag, resultPowerRamp->on_power_peak->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "off_power_peak"))
				{
					if (resultPowerRamp->off_power_peak && resultPowerRamp->off_power_peak->length)
					{
						if (bufferLength > resultPowerRamp->off_power_peak->length)
							bufferLength = resultPowerRamp->off_power_peak->length;

						memcpy(bufferReal, resultPowerRamp->off_power_peak->real, (sizeof(double)*bufferLength));

						if (resultPowerRamp->off_power_peak->imag && bufferImag)
							memcpy(bufferImag, resultPowerRamp->off_power_peak->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "on_time_vect"))
				{
					if (resultPowerRamp->on_time_vect && resultPowerRamp->on_time_vect->length)
					{
						if (bufferLength > resultPowerRamp->on_time_vect->length)
							bufferLength = resultPowerRamp->on_time_vect->length;

						memcpy(bufferReal, resultPowerRamp->on_time_vect->real, (sizeof(double)*bufferLength));

						if (resultPowerRamp->on_time_vect->imag && bufferImag)
							memcpy(bufferImag, resultPowerRamp->on_time_vect->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				} else if (!strcmp(measurement, "off_time_vect"))
				{
					if (resultPowerRamp->off_time_vect && resultPowerRamp->off_time_vect->length)
					{
						if (bufferLength > resultPowerRamp->off_time_vect->length)
							bufferLength = resultPowerRamp->off_time_vect->length;

						memcpy(bufferReal, resultPowerRamp->off_time_vect->real, (sizeof(double)*bufferLength));

						if (resultPowerRamp->off_time_vect->imag && bufferImag)
							memcpy(bufferImag, resultPowerRamp->off_time_vect->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else
				{
					return 0;
				}
			}
			else if (dynamic_cast<iqapiResultBluetooth *>(hndl->results))
			{
				resultBluetooth = dynamic_cast<iqapiResultBluetooth *>(hndl->results);

				if (!strcmp(measurement, "P_av_each_burst"))
				{
					if (resultBluetooth->P_av_each_burst && resultBluetooth->P_av_each_burst->length)
					{
						if (bufferLength > resultBluetooth->P_av_each_burst->length)
							bufferLength = resultBluetooth->P_av_each_burst->length;

						memcpy(bufferReal, resultBluetooth->P_av_each_burst->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->P_av_each_burst->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->P_av_each_burst->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "P_pk_each_burst"))
				{
					if (resultBluetooth->P_pk_each_burst && resultBluetooth->P_pk_each_burst->length)
					{
						if (bufferLength > resultBluetooth->P_pk_each_burst->length)
							bufferLength = resultBluetooth->P_pk_each_burst->length;

						memcpy(bufferReal, resultBluetooth->P_pk_each_burst->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->P_pk_each_burst->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->P_pk_each_burst->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "complete_burst"))
				{
					if (resultBluetooth->complete_burst && resultBluetooth->complete_burst->length)
					{
						if (bufferLength > resultBluetooth->complete_burst->length)
							bufferLength = resultBluetooth->complete_burst->length;

						memcpy(bufferReal, resultBluetooth->complete_burst->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->complete_burst->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->complete_burst->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "start_sec"))
				{
					if (resultBluetooth->start_sec && resultBluetooth->start_sec->length)
					{
						if (bufferLength > resultBluetooth->start_sec->length)
							bufferLength = resultBluetooth->start_sec->length;

						memcpy(bufferReal, resultBluetooth->start_sec->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->start_sec->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->start_sec->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "stop_sec"))
				{
					if (resultBluetooth->stop_sec && resultBluetooth->stop_sec->length)
					{
						if (bufferLength > resultBluetooth->stop_sec->length)
							bufferLength = resultBluetooth->stop_sec->length;

						memcpy(bufferReal, resultBluetooth->stop_sec->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->stop_sec->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->stop_sec->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "freq_est"))
				{
					if (resultBluetooth->freq_est && resultBluetooth->freq_est->length)
					{
						if (bufferLength > resultBluetooth->freq_est->length)
							bufferLength = resultBluetooth->freq_est->length;

						memcpy(bufferReal, resultBluetooth->freq_est->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->freq_est->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->freq_est->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "freqEstPacketPointer"))
				{
					if (resultBluetooth->freqEstPacketPointer && resultBluetooth->freqEstPacketPointer->length)
					{
						if (bufferLength > resultBluetooth->freqEstPacketPointer->length)
							bufferLength = resultBluetooth->freqEstPacketPointer->length;

						memcpy(bufferReal, resultBluetooth->freqEstPacketPointer->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->freqEstPacketPointer->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->freqEstPacketPointer->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "freq_drift"))
				{
					if (resultBluetooth->freq_drift && resultBluetooth->freq_drift->length)
					{
						if (bufferLength > resultBluetooth->freq_drift->length)
							bufferLength = resultBluetooth->freq_drift->length;

						memcpy(bufferReal, resultBluetooth->freq_drift->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->freq_drift->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->freq_drift->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "freqDriftPacketPointer"))
				{
					if (resultBluetooth->freqDriftPacketPointer && resultBluetooth->freqDriftPacketPointer->length)
					{
						if (bufferLength > resultBluetooth->freqDriftPacketPointer->length)
							bufferLength = resultBluetooth->freqDriftPacketPointer->length;

						memcpy(bufferReal, resultBluetooth->freqDriftPacketPointer->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->freqDriftPacketPointer->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->freqDriftPacketPointer->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "freqDeviationPointer"))
				{
					if (resultBluetooth->freqDeviationPointer && resultBluetooth->freqDeviationPointer->length)
					{
						if (bufferLength > resultBluetooth->freqDeviationPointer->length)
							bufferLength = resultBluetooth->freqDeviationPointer->length;

						memcpy(bufferReal, resultBluetooth->freqDeviationPointer->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->freqDeviationPointer->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->freqDeviationPointer->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "EdrFreqvsTime"))
				{
					if (resultBluetooth->EdrFreqvsTime && resultBluetooth->EdrFreqvsTime->length)
					{
						if (bufferLength > resultBluetooth->EdrFreqvsTime->length)
							bufferLength = resultBluetooth->EdrFreqvsTime->length;

						memcpy(bufferReal, resultBluetooth->EdrFreqvsTime->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->EdrFreqvsTime->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->EdrFreqvsTime->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "EdrExtremeOmegaI0"))
				{
					if (resultBluetooth->EdrExtremeOmegaI0 && resultBluetooth->EdrExtremeOmegaI0->length)
					{
						if (bufferLength > resultBluetooth->EdrExtremeOmegaI0->length)
							bufferLength = resultBluetooth->EdrExtremeOmegaI0->length;

						memcpy(bufferReal, resultBluetooth->EdrExtremeOmegaI0->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->EdrExtremeOmegaI0->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->EdrExtremeOmegaI0->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
			}
			else if (dynamic_cast<iqapiResultPower *>(hndl->results))
			{
				resultPower = dynamic_cast<iqapiResultPower *>(hndl->results);
				if (!strcmp(measurement, "start_sec"))
				{
					if (resultPower->start_sec && resultPower->start_sec->length)
					{
						if (bufferLength > resultPower->start_sec->length)
							bufferLength = resultPower->start_sec->length;

						memcpy(bufferReal, resultPower->start_sec->real, (sizeof(double)*bufferLength));

						if (resultPower->start_sec->imag && bufferImag)
							memcpy(bufferImag, resultPower->start_sec->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "stop_sec"))
				{
					if (resultPower->stop_sec && resultPower->stop_sec->length)
					{
						if (bufferLength > resultPower->stop_sec->length)
							bufferLength = resultPower->stop_sec->length;

						memcpy(bufferReal, resultPower->stop_sec->real, (sizeof(double)*bufferLength));

						if (resultPower->stop_sec->imag && bufferImag)
							memcpy(bufferImag, resultPower->stop_sec->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "P_av_each_burst_dBm"))
				{
					if (resultPower->P_av_each_burst_dBm && resultPower->P_av_each_burst_dBm->length)
					{
						if (bufferLength > resultPower->P_av_each_burst_dBm->length)
							bufferLength = resultPower->P_av_each_burst_dBm->length;

						memcpy(bufferReal, resultPower->P_av_each_burst_dBm->real, (sizeof(double)*bufferLength));

						if (resultPower->P_av_each_burst_dBm->imag && bufferImag)
							memcpy(bufferImag, resultPower->P_av_each_burst_dBm->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "P_pk_each_burst_dBm"))
				{
					if (resultPower->P_pk_each_burst_dBm && resultPower->P_pk_each_burst_dBm->length)
					{
						if (bufferLength > resultPower->P_pk_each_burst_dBm->length)
							bufferLength = resultPower->P_pk_each_burst_dBm->length;

						memcpy(bufferReal, resultPower->P_pk_each_burst_dBm->real, (sizeof(double)*bufferLength));

						if (resultPower->P_pk_each_burst_dBm->imag && bufferImag)
							memcpy(bufferImag, resultPower->P_av_each_burst_dBm->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "complete_burst"))
				{
					if (resultPower->complete_burst && resultPower->complete_burst->length)
					{
						if (bufferLength > resultPower->complete_burst->length)
							bufferLength = resultPower->complete_burst->length;

						memcpy(bufferReal, resultPower->complete_burst->real, (sizeof(double)*bufferLength));

						if (resultPower->complete_burst->imag && bufferImag)
							memcpy(bufferImag, resultPower->complete_burst->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
			}
#if !defined(IQAPI_1_5_X)
			else if (dynamic_cast<iqapiResultHT40 *>(hndl->results))
			{
				resultHT40 = dynamic_cast<iqapiResultHT40 *>(hndl->results);

				if (!strcmp(measurement, "x"))
				{
					if (resultHT40->ffts && resultHT40->len_of_ffts)
					{
						double *pReal = bufferReal;
						double *pImag = bufferImag;
						int    len = bufferLength;
						for(int i=0; i<resultHT40->len_of_ffts; i++)
						{
							if (len > resultHT40->ffts[i]->x->length)
							{
								//bufferLength = resultHT40->len_of_ffts * resultHT40->ffts[0]->length;

								memcpy(pReal, resultHT40->ffts[i]->x->real, (sizeof(double) * resultHT40->ffts[i]->x->length));
								pReal += resultHT40->ffts[i]->x->length;
								len -= resultHT40->ffts[i]->x->length;

								if (resultHT40->ffts[i]->x->imag && bufferImag)
								{
									memcpy(pImag, resultHT40->ffts[i]->x->imag, (sizeof(double) * resultHT40->ffts[i]->x->length));
									pImag += resultHT40->ffts[i]->x->length;
								}
							}
							else
							{
								// the buffer has been used up
								memcpy(pReal, resultHT40->ffts[i]->x->real, (sizeof(double) * len));
								if (resultHT40->ffts[i]->x->imag && bufferImag)
								{
									memcpy(pImag, resultHT40->ffts[i]->x->imag, (sizeof(double) * len));
								}
								break;
							}
						}

						//TODO: Fix this
						return(resultHT40->ffts[0]->x->length);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "y"))
				{
					if (resultHT40->ffts && resultHT40->len_of_ffts)
					{
						double *pReal = bufferReal;
						double *pImag = bufferImag;
						int    len = bufferLength;
						for(int i=0; i<resultHT40->len_of_ffts; i++)
						{
							if (len > resultHT40->ffts[i]->y->length)
							{
								//bufferLength = resultHT40->len_of_ffts * resultHT40->ffts[0]->length;

								memcpy(pReal, resultHT40->ffts[i]->y->real, (sizeof(double) * resultHT40->ffts[i]->y->length));
								pReal += resultHT40->ffts[i]->y->length;
								len -= resultHT40->ffts[i]->y->length;

								if (resultHT40->ffts[i]->y->imag && bufferImag)
								{
									memcpy(pImag, resultHT40->ffts[i]->y->imag, (sizeof(double) * resultHT40->ffts[i]->y->length));
									pImag += resultHT40->ffts[i]->y->length;
								}
							}
							else
							{
								// the buffer has been used up
								memcpy(pReal, resultHT40->ffts[i]->y->real, (sizeof(double) * len));
								if (resultHT40->ffts[i]->y->imag && bufferImag)
								{
									memcpy(pImag, resultHT40->ffts[i]->y->imag, (sizeof(double) * len));
								}
								break;
							}
						}

						return(resultHT40->ffts[0]->y->length);
					}
					else
						return 0;
				}


				//else if (!strcmp(measurement, "y"))
				//{
				//    if (resultHT40->y && resultHT40->y->length)
				//    {
				//        if (bufferLength > resultHT40->y->length)
				//            bufferLength = resultHT40->y->length;

				//        memcpy(bufferReal, resultHT40->y->real, (sizeof(double)*bufferLength));

				//        if (resultHT40->y->imag && bufferImag)
				//            memcpy(bufferImag, resultHT40->y->imag, (sizeof(double)*bufferLength));

				//        return(bufferLength);
				//    }
				//    else
				//        return 0;
				//}
			}
#endif


			else if (dynamic_cast<iqapiResultVHT80WideBand *>(hndl->results))
			{
				if (true == bIQxsFound || true == bIQxelFound)
				{
					resultVHT80Wideband = dynamic_cast<iqapiResultVHT80WideBand *>(hndl->results);

					if (!strcmp(measurement, "x"))
					{
						if (resultVHT80Wideband->ffts && resultVHT80Wideband->len_of_ffts)
						{
							double *pReal = bufferReal;
							double *pImag = bufferImag;
							int    len = bufferLength;
							for(int i=0; i<resultVHT80Wideband->len_of_ffts; i++)
							{
								if (len > resultVHT80Wideband->ffts[i]->x->length)
								{
									//bufferLength = resultVHT80Wideband->len_of_ffts * resultVHT80Wideband->ffts[0]->length;

									memcpy(pReal, resultVHT80Wideband->ffts[i]->x->real, (sizeof(double) * resultVHT80Wideband->ffts[i]->x->length));
									pReal += resultVHT80Wideband->ffts[i]->x->length;
									len -= resultVHT80Wideband->ffts[i]->x->length;

									if (resultVHT80Wideband->ffts[i]->x->imag && bufferImag)
									{
										memcpy(pImag, resultVHT80Wideband->ffts[i]->x->imag, (sizeof(double) * resultVHT80Wideband->ffts[i]->x->length));
										pImag += resultVHT80Wideband->ffts[i]->x->length;
									}
								}
								else
								{
									// the buffer has been used up
									memcpy(pReal, resultVHT80Wideband->ffts[i]->x->real, (sizeof(double) * len));
									if (resultVHT80Wideband->ffts[i]->x->imag && bufferImag)
									{
										memcpy(pImag, resultVHT80Wideband->ffts[i]->x->imag, (sizeof(double) * len));
									}
									break;
								}
							}

							//TODO: Fix this
							return(resultVHT80Wideband->ffts[0]->x->length);
						}
						else
							return 0;
					}
					else if (!strcmp(measurement, "y"))
					{
						if (resultVHT80Wideband->ffts && resultVHT80Wideband->len_of_ffts)
						{
							double *pReal = bufferReal;
							double *pImag = bufferImag;
							int    len = bufferLength;
							for(int i=0; i<resultVHT80Wideband->len_of_ffts; i++)
							{
								if (len > resultVHT80Wideband->ffts[i]->y->length)
								{
									//bufferLength = resultVHT80Wideband->len_of_ffts * resultVHT80Wideband->ffts[0]->length;

									memcpy(pReal, resultVHT80Wideband->ffts[i]->y->real, (sizeof(double) * resultVHT80Wideband->ffts[i]->y->length));
									pReal += resultVHT80Wideband->ffts[i]->y->length;
									len -= resultVHT80Wideband->ffts[i]->y->length;

									if (resultVHT80Wideband->ffts[i]->y->imag && bufferImag)
									{
										memcpy(pImag, resultVHT80Wideband->ffts[i]->y->imag, (sizeof(double) * resultVHT80Wideband->ffts[i]->y->length));
										pImag += resultVHT80Wideband->ffts[i]->y->length;
									}
								}
								else
								{
									// the buffer has been used up
									memcpy(pReal, resultVHT80Wideband->ffts[i]->y->real, (sizeof(double) * len));
									if (resultVHT80Wideband->ffts[i]->y->imag && bufferImag)
									{
										memcpy(pImag, resultVHT80Wideband->ffts[i]->y->imag, (sizeof(double) * len));
									}
									break;
								}
							}

							return(resultVHT80Wideband->ffts[0]->y->length);
						}
						else
							return 0;
					}
				}
				else
					return 0;
			}
			else
				return 0;
		}
	}

	return 0;
}

IQMEASURE_API int LP_GetVectorMeasurement(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	::TIMER_StartTimer(timerIQmeasure, "LP_GetVectorMeasurement", &timeStart);

	int ret = LP_GetVectorMeasurement_NoTimer(measurement, bufferReal, bufferImag, bufferLength);

	//// clear malloc and more, JK -1/15/12
	//// Workaround: Jarir, 1/13/12, add following to return spectrum
	//// between -40 to +40 MHz for regular HT20 to make compatible with IQ2010 //change from -33.0
	//// We should return all of it (-60 to +60MHz), but spike appears
	//// at +/- 50MHz with Daytona, which forced this workaround
	//static int numTruncSamp     = 0;                            // on each side
	//static int numSampleUsed    = 0;                            // return number of samples
	//double freqLeftCutoff       = -40.0e6;                      // truncation left-side cut-off frequency
	//if(true == bIQxsFound || true == bIQxelFound)        // 1. truncation only done for tester IQXS & DTNA so far
	//{
	//	if (g_lastPerformedAnalysisType == ANALYSIS_FFT)        // 2. truncation only for regular HT20 signal
	//	{
	//		if (!strcmp(measurement, "x"))                      // -----------------------
	//		{
	//			for (int k=0; k<ret; k++)
	//			{
	//				if (bufferReal[k] < freqLeftCutoff)
	//					numTruncSamp ++;
	//				else
	//					break;
	//			}

	//			numSampleUsed   = ret-2*numTruncSamp;
	//			ret  = numSampleUsed;                           // correct the return data length

	//			for(int j = 0; j< numSampleUsed; j++)
	//			{
	//				bufferReal[j]= bufferReal[j+numTruncSamp];
	//				bufferImag[j]= bufferImag[j+numTruncSamp];
	//			}
	//		}

	//		if (!strcmp(measurement, "y") && (numTruncSamp>0))  // ------------------
	//		{
	//			ret  = numSampleUsed;                           // correct the return data length
	//			for(int j = 0; j< numSampleUsed; j++)
	//			{
	//				bufferReal[j]= bufferReal[j+numTruncSamp];
	//				bufferImag[j]= bufferImag[j+numTruncSamp];
	//			}

	//			numTruncSamp    = 0;                            // clear
	//			numSampleUsed   = 0;
	//		}
	//	}
	//}


	::TIMER_StopTimer(timerIQmeasure, "LP_GetVectorMeasurement", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_GetVectorMeasurement", timeDuration);


	return ret;
}


void InstantiateAnalysisClasses()
{
	//::TIMER_StartTimer(timerIQmeasure, "LP_SetAnalysisMode", &timeStart);
	//if  ( iqapiSetAnalysisMode("UNLOCK_KEY_1147014E-95E0-440d-A752-5F7C08A2FEE9") != IQAPI_ERR_OK )
	//{
	//	throw hndl->lastErr;
	//}
	//::TIMER_StopTimer(timerIQmeasure, "LP_SetAnalysisMode", &timeDuration, &timeStop);
	//printf("\nTime to unlock analysis in InstantiateAnalysisClasses() is %0.3f ms\n", timeDuration);

	//temp,  Z-99
	//analysisWifiOfdm  = new iqapiAnalysis11ac();'
	analysisWifiOfdm  = new iqapiAnalysisWifiOfdm();
	analysisVHT80 = new iqapiAnalysisVHT80WideBand();
	analysis80216 = new iqapiAnalysis80216();
	analysisMimo = new iqapiAnalysisMimo();
	analysisOfdm = new iqapiAnalysisOFDM();
	analysis11b = new iqapiAnalysis11b();
	analysisPowerRampOfdm = new iqapiAnalysisPowerRamp(true);
	analysisPowerRamp11b = new iqapiAnalysisPowerRamp(false);
	analysisCW = new iqapiAnalysisCW();
	analysisSidelobe = new iqapiAnalysisSidelobe();
	analysisWave = new iqapiAnalysisWave();
	analysisCCDF = new iqapiAnalysisCCDF();
	analysisFFT = new iqapiAnalysisFFT();
	analysisPower = new iqapiAnalysisPower();
	analysisBluetooth = new iqapiAnalysisBluetooth();
	analysisZigbee = new iqapiAnalysisZigbee();
#if !defined(IQAPI_1_5_X)
	analysisHT40 = new iqapiAnalysisHT40();
#endif

	// FM
	analysisFmRf = new iqapiAnalysisFmRf();
	analysisFmAudioBase = new iqapiAnalysisFmAudioBase();
	analysisFmDemodSpectrum = new iqapiAnalysisFmDemodSpectrum();
	analysisFmMono = new iqapiAnalysisFmMono();
	analysisFmStereo = new iqapiAnalysisFmStereo();
	analysisFmAuto = new iqapiAnalysisFmAuto();
	analysisRds = new iqapiAnalysisRds();
	analysisRdsMono = new iqapiAnalysisRdsMono();
	analysisRdsStereo = new iqapiAnalysisRdsStereo();
	analysisRdsAuto = new iqapiAnalysisRdsAuto();
	analysisAudioStereo = new iqapiAnalysisAudioStereo();
	analysisAudioMono = new iqapiAnalysisAudioMono();

	//if  ( iqapiSetAnalysisMode("LOCK_KEY_431A4A5B-90A1-499c-B70F-11078C7C5954") != IQAPI_ERR_OK )
	//{
	//	throw hndl->lastErr;
	//}
	return;
}

//void  InitializeTesterSettings(void)
//{
//	if( NULL!=hndl )
//	{
//		//Choose to use IQV_VSA_TYPE_1 (peak power mode)
//		hndl->rx->powerMode = IQV_VSA_TYPE_1;
//		for (int i=0;i<nTesters;i++)
//		{
//			hndl->tx->vsg[i]->source = IQV_SOURCE_WAVE;  //this is needed to turn off the CW from VSG
//			//Setting VSG to IQV_PORT_OFF caused MW not to configure VSGs, although hndl shows everything is corrrect.
//            //TODO: the line below needs to be removed from the code
//            //hndl->tx->vsg[i]->port = IQV_PORT_OFF;
//
//			// Store settings for IQnxn
//			g_IQnxnSettings[i].vsaAmplDBm		= hndl->rx->vsa[i]->rfAmplDb;
//			g_IQnxnSettings[i].vsaPort			= hndl->rx->vsa[i]->port;
//			g_IQnxnSettings[i].vsgPort			= hndl->tx->vsg[i]->port;
//			g_IQnxnSettings[i].vsgPowerDBm		= hndl->tx->vsg[i]->rfGainDb;
//			g_IQnxnSettings[i].vsgRFEnabled		= hndl->tx->vsg[i]->enabled;
//		}
//		hndl->SetTxRx();
//	}
//	g_vsgMode = VSG_NO_MOD_FILE_LOADED;
//	return;
//}


void  InitializeTesterSettings(void)
{
	if( NULL!=hndl )
	{
		//Choose to use IQV_VSA_TYPE_1 (peak power mode)
		hndl->rx->powerMode = IQV_VSA_TYPE_1;
		for (int i=0;i<nTesters;i++)
		{
			hndl->tx->vsg[i]->source = IQV_SOURCE_WAVE;  //this is needed to turn off the CW from VSG
			//Setting VSG to IQV_PORT_OFF caused MW not to configure VSGs, although hndl shows everything is corrrect.
			//TODO: the line below needs to be removed from the code
			//hndl->tx->vsg[i]->port = IQV_PORT_OFF;

			// Store settings for IQnxn
			g_IQnxnSettings[i].vsaAmplDBm		= hndl->rx->vsa[i]->rfAmplDb;
			g_IQnxnSettings[i].vsaPort			= hndl->rx->vsa[i]->port;
			g_IQnxnSettings[i].vsgPort			= hndl->tx->vsg[i]->port;
			g_IQnxnSettings[i].vsgPowerDBm		= hndl->tx->vsg[i]->rfGainDb;
			g_IQnxnSettings[i].vsgRFEnabled		= hndl->tx->vsg[i]->enabled;
		}
		hndl->SetTxRx();
	}
	g_vsgMode = VSG_NO_MOD_FILE_LOADED;

	return;
}

IQMEASURE_API int LP_DualHead_ConOpen(int tokenID, char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_DualHead_ConOpen", &timeStart);

	if (LibsInitialized)
	{
		err = hndl->ConOpen(ipAddress1, ipAddress2, ipAddress3, ipAddress4);
		if(NULL!=ipAddress1 && NULL==ipAddress2 && NULL==ipAddress3 && NULL==ipAddress4)
		{
			nTesters = 1;
		}
		else if (NULL!=ipAddress1 && NULL!=ipAddress2 && NULL==ipAddress3 && NULL==ipAddress4)
		{
			nTesters = 2;
		}
		else if (NULL!=ipAddress1 && NULL!=ipAddress2 && NULL!=ipAddress3 && NULL==ipAddress4)
		{
			nTesters = 3;
		}
		else if (NULL!=ipAddress1 && NULL!=ipAddress2 && NULL!=ipAddress3 && NULL!=ipAddress4)
		{
			nTesters = 4;
		}
		else
		{
			err = -1;
		}

		if( ERR_OK==err )
		{
			if( !hndl->pDualHead->SetTokenID(IQV_TOKEN_NO_1, tokenID) )
			{
				// SetTokenID failed
				err = ERR_SET_TOKEN_FAILED;
			}
			else
			{
				err = ERR_OK;
				setDefaultCalled = false;
				InstantiateAnalysisClasses();
				//Choose to use IQV_VSA_TYPE_1 (peak power mode)

				// We cannot change anything of the tester before TokenRetrieveTimeout()
				//hndl->rx->powerMode = IQV_VSA_TYPE_1;
				//nTesters = hndl->nTesters;
				//err = hndl->SetTxRx();

				g_vsgMode = VSG_NO_MOD_FILE_LOADED;
			}
		}
		else
		{
			err = ERR_NO_CONNECTION;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_DualHead_ConOpen", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_DualHead_ConOpen", timeDuration);


	return err;
}


IQMEASURE_API int LP_DualHead_ObtainControl(unsigned int probeTimeMS, unsigned int timeOutMS)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_DualHead_ObtainControl", &timeStart);

	if (LibsInitialized)
	{
		if( hndl->ConValid() )
		{
			// Wait for the Token
			// TODO: add a total timeout
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[TokenRetrieveTimeout start]\n");
			if( hndl->pDualHead->TokenRetrieveTimeout(IQV_TOKEN_NO_1, probeTimeMS, timeOutMS) )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[TokenRetrieveTimeout done]\n");
				if( !setDefaultCalled )
				{
					hndl->SetDefault();
					setDefaultCalled = true;
					nTesters = hndl->nTesters;
					InstantiateAnalysisClasses();
					InitializeTesterSettings();
					if(NULL != hndl->hndlFm)
					{
						FmInitialized = true;
					}
				}
				else
				{
					//hndl->GetStatus();
					g_bDisableFreqFilter = true;
				}
			}
			else
			{
				// TokenRetrieveTimeout timed out
				//// If the total timeout is long enough to allow the other program to release, the timeout
				//// is caused by abnormal termination, so that TokenRelease() wasn't called on the other side.
				//printf("TokenReset...\n");
				//hndl->pDualHead->TokenReset();
				//printf("TokenReset done\n");
				err = ERR_TOKEN_RETRIEVAL_TIMEDOUT;
			}
		}
		else
		{
			err = ERR_NOT_INITIALIZED;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_DualHead_ObtainControl", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_DualHead_ObtainControl", timeDuration);


	return err;
}


IQMEASURE_API int LP_DualHead_ReleaseControl()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_DualHead_ReleaseControl", &timeStart);

	if (LibsInitialized)
	{
		if( hndl->pDualHead->TokenRelease(IQV_TOKEN_NO_1) )
		{
			// Token released
		}
		else
		{
			// Problem in releasing token
			// TODO: how to handle this?
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}


	::TIMER_StopTimer(timerIQmeasure, "LP_DualHead_ReleaseControl", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_DualHead_ReleaseControl", timeDuration);


	return err;
}


IQMEASURE_API int LP_DualHead_ConClose(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_DualHead_ConClose", &timeStart);

	if (LibsInitialized)
	{
		hndl->pDualHead->TokenRelease(IQV_TOKEN_NO_1);
		int iqapiError = hndl->ConClose();
		if (iqapiError)
		{
			err = ERR_GENERAL_ERR;
		}
		else
		{

		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}


	::TIMER_StopTimer(timerIQmeasure, "LP_DualHead_ConClose", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_DualHead_ConClose", timeDuration);


	return err;
}

IQMEASURE_API int LP_DualHead_GetTokenID(int *tokenID)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_DualHead_GetTokenID", &timeStart);

	if (LibsInitialized)
	{
		*tokenID = hndl->pDualHead->TokenID[IQV_TOKEN_NO_1];
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_DualHead_GetTokenID", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_DualHead_GetTokenID", timeDuration);


	return err;
}
IQMEASURE_API int LP_StartIQmeasureTimer()
{
	int err = ERR_OK;

	if(-1==timerIQmeasure)
	{
		TIMER_CreateTimer("IQMEASURE", &timerIQmeasure);
	}
	if(-1==loggerIQmeasure)
	{
		LOGGER_CreateLogger("IQMEASURE", &loggerIQmeasure);
	}

	return err;
}

IQMEASURE_API int LP_StopIQmeasureTimer()
{
	int err = ERR_OK;

	if(-1!=timerIQmeasure)
	{
		TIMER_ClearTimerHistory( timerIQmeasure );
	}

	return err;
}

IQMEASURE_API int LP_ReportTimerDurations()
{
	int err = ERR_OK;

	if(-1!=timerIQmeasure)
	{
		TIMER_ReportTimerDurations( timeStart, timeStop );
	}

	return err;
}

IQMEASURE_API int LP_SetLpcPath(char *litePointConnectionPath)
{


	int err = ERR_OK;

	/*
	   char *litePointConnectionPathReturn;

	   if (LibsInitialized)
	   {
	   if (bIQxelFound==true || bIQxsFound == true) // IQxel doesn't has this function anymore.
	   {
	   err = ERR_OK;
	   }else
	   {
	   hndl->SetLpcPath(litePointConnectionPath);

	//Check "Set path" is work or not
	litePointConnectionPathReturn = hndl->GetLpcPath();
	if(strcmp(litePointConnectionPath, litePointConnectionPathReturn)== 0)
	{
	err = ERR_OK;
	}
	else
	{
	err = ERR_SET_PATH_NOT_DONE;
	}
	}

	}
	else
	{
	err = ERR_NOT_INITIALIZED;
	}
	*/
	return err;
}

//IQMEASURE_API int LP_IQ2010EXT_NewMultiSegmentWaveform(const char *multiSegmentWaveformFileName, const char *multiSegmentMarkerFileName)
IQMEASURE_API int		LP_IQ2010EXT_NewMultiSegmentWaveform()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_NewMultiSegmentWaveform", &timeStart);

	if (LibsInitialized)
	{
		//		err = IQ2010EXT_NewMultiSegmentWaveform();	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_NewMultiSegmentWaveform", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_NewMultiSegmentWaveform", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_ClearAllWaveform(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_ClearAllWaveform", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		//		err = IQ2010EXT_ClearAllWaveform();	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_ClearAllWaveform", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_ClearAllWaveform", timeDuration);


	return err;
}

IQMEASURE_API int LP_IQ2010EXT_AddWaveform(const char *modFile, unsigned int *waveformIndex)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_AddWaveform", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_AddWaveform(modFile, waveformIndex);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_AddWaveform", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_AddWaveform", timeDuration);


	return err;
}

IQMEASURE_API int LP_IQ2010EXT_FinalizeMultiSegmentWaveform()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_FinalizeMultiSegmentWaveform", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_FinalizeMultiSegmentWaveform();	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_FinalizeMultiSegmentWaveform", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_FinalizeMultiSegmentWaveform", timeDuration);


	return err;
}

/*IQMEASURE_API int LP_IQ2010EXT_RxPer(int    waveformIndex,
  int    freqMHz,
  double powerLevelStart,
  double powerLevelStop,
  double step,
  int    packetCount,
  int    rfPort,
  double ackPowerRmsdBm,
  double ackTriggerLeveldBm
  )*/
IQMEASURE_API int		LP_IQ2010EXT_RxPer( int               waveformIndex,
		double            freqMHz,
		double            powerLevelStartdBm,
		double            powerLevelStopdBm,
		double            stepdB,
		int               packetCount,
		int				rfPort,
		double            ackPowerRmsdBm,
		double            ackTriggerLeveldBm)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_RxPer", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		// 		err = IQ2010EXT_RxPer( waveformIndex,
		// 							   freqMHz,
		// 							   powerLevelStart,
		// 							   powerLevelStop,
		// 							   step,
		// 							   packetCount,
		// 							   rfPort,
		// 							   ackPowerRmsdBm,
		// 							   ackTriggerLeveldBm
		// 							  );	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_RxPer", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_RxPer", timeDuration);


	return err;
}

IQMEASURE_API int LP_IQ2010EXT_GetRxPerResults_PB(double ackRespMax[],
		double ackRespMin[],
		double per[],
		int    arraySize
		)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_GetRxPerResults_PB", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		// 		err = IQ2010EXT_GetRxPerResults_PB( ackRespMax,
		//                                             ackRespMin,
		//                                             per,
		//                                             arraySize
		//                                            );	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_GetRxPerResults_PB", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_GetRxPerResults_PB", timeDuration);


	return err;
}

/*IQMEASURE_API int LP_IQ2010EXT_InitiateMultiCapture( double		dataRate,
  double		freqMHz,
  double		peakPowerLeveldBm,
  int	    captureLengthUs,
  int	    skipPktCnt,
  int	    captureCnt,
  int		rfPort,
  double		triggerLevelOffsetdB)*/
IQMEASURE_API int		LP_IQ2010EXT_InitiateMultiCapture( char				*dataRate,
		double              freqMHz,
		double	             rmsPowerLeveldBm,
		int	             captureLengthUs,
		int	             skipPktCnt,
		int	             captureCnt,
		int				 rfPort,
		double	             triggerLeveldBm)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_InitiateMultiCapture", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		//err = IQ2010EXT_InitiateMultiCapture( dataRate,
		//									  freqMHz,
		//									  peakPowerLeveldBm,
		//									  captureLengthUs,
		//									  skipPktCnt,
		//									  captureCnt,
		//									  rfPort,
		//									  triggerLevelOffsetdB);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_InitiateMultiCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_InitiateMultiCapture", timeDuration);

	return err;
}

//IQMEASURE_API int LP_IQ2010EXT_FinishMultiCapture(void)
//{
//	int err = ERR_OK;
//
//	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_FinishMultiCapture", &timeStart);
//
//	if (LibsInitialized && bIQ201xFound)
//	{
//		err = IQ2010EXT_FinishMultiCapture();	// TODO
//	}
//	else
//	{
//		err = ERR_NOT_INITIALIZED;
//	}
//
//	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_FinishMultiCapture", &timeDuration, &timeStop);
//	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_FinishMultiCapture", timeDuration);
//
//	return err;
//}
//
//IQMEASURE_API int LP_IQ2010EXT_Analyze80211ag( int ph_corr_mode,
//											   int ch_estimate,
//											   int sym_tim_corr,
//											   int freq_sync,
//											   int ampl_track)
//{
//	int err = ERR_OK;
//
//	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211ag", &timeStart);
//
//	if (LibsInitialized && bIQ201xFound)
//	{
//		err = IQ2010EXT_Analyze80211ag( (IQV_PH_CORR_ENUM)		ph_corr_mode,
//									    (IQV_CH_EST_ENUM)		ch_estimate,
//									    (IQV_SYM_TIM_ENUM)		sym_tim_corr,
//									    (IQV_FREQ_SYNC_ENUM)	freq_sync,
//									    (IQV_AMPL_TRACK_ENUM)	ampl_track);	// TODO
//	}
//	else
//	{
//		err = ERR_NOT_INITIALIZED;
//	}
//
//	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211ag", &timeDuration, &timeStop);
//	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_Analyze80211ag", timeDuration);
//
//	return err;
//}
//
//IQMEASURE_API int LP_IQ2010EXT_Analyze80211n( char *type,
//											  char *mode,
//											  int	enablePhaseCorr            ,
//											  int	enableSymTimingCorr        ,
//											  int	enableAmplitudeTracking    ,
//											  int	decodePSDU                 ,
//											  int	enableFullPacketChannelEst ,
//											  char *referenceFile			   ,
//											  int	packetFormat			   )
//{
//	int err = ERR_OK;
//
//	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211n", &timeStart);
//
//	if (LibsInitialized && bIQ201xFound)
//	{
//// 		err = IQ2010EXT_Analyze80211n( type,
//// 									   mode,
//// 									   enablePhaseCorr            ,
//// 									   enableSymTimingCorr        ,
//// 									   enableAmplitudeTracking    ,
//// 									   decodePSDU                 ,
//// 									   enableFullPacketChannelEst ,
//// 									   referenceFile			  ,
//// 									   packetFormat
//// 									   );	// TODO
//	}
//	else
//	{
//		err = ERR_NOT_INITIALIZED;
//	}
//
//	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211n", &timeDuration, &timeStop);
//	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_Analyze80211n", timeDuration);
//
//	return err;
//}
//
//IQMEASURE_API int LP_IQ2010EXT_Analyze80211b( int eq_taps           ,
//											  int DCremove11b_flag  ,
//											  int method_11b        )
//{
//	int err = ERR_OK;
//
//	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211b", &timeStart);
//
//	if (LibsInitialized && bIQ201xFound)
//	{
//		err = IQ2010EXT_Analyze80211b( (IQV_EQ_ENUM)		 eq_taps           ,
//									   (IQV_DC_REMOVAL_ENUM) DCremove11b_flag  ,
//									   (IQV_11B_METHOD_ENUM) method_11b
//									  );	// TODO
//	}
//	else
//	{
//		err = ERR_NOT_INITIALIZED;
//	}
//
//	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211b", &timeDuration, &timeStop);
//	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_Analyze80211b", timeDuration);
//
//	return err;
//}
//
//IQMEASURE_API int LP_IQ2010EXT_AnalyzePower(void)
//{
//	int err = ERR_OK;
//
//	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzePower", &timeStart);
//
//	if (LibsInitialized && bIQ201xFound)
//	{
//		err = IQ2010EXT_AnalyzePower();	// TODO
//	}
//	else
//	{
//		err = ERR_NOT_INITIALIZED;
//	}
//
//	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzePower", &timeDuration, &timeStop);
//	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_AnalyzePower", timeDuration);
//
//	return err;
//}
//
//IQMEASURE_API int LP_IQ2010EXT_GetDoubleMeasurements( char   *measurementName,
//													  double *average,
//													  double *minimum,
//													  double *maximum)
//{
//	int err = ERR_OK;
//
//	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_GetDoubleMeasurements", &timeStart);
//
//	if (LibsInitialized && bIQ201xFound)
//	{
//		err = IQ2010EXT_GetDoubleMeasurements( measurementName,
//											   average,
//											   minimum,
//											   maximum
//											  );	// TODO
//	}
//	else
//	{
//		err = ERR_NOT_INITIALIZED;
//	}
//
//	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_GetDoubleMeasurements", &timeDuration, &timeStop);
//	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_GetDoubleMeasurements", timeDuration);
//
//	return err;
//}
//
//IQMEASURE_API int LP_IQ2010EXT_GetStringMeasurement( char *measurementName,
//													 char *result,
//													 int   bufferSize,
//													 int   indexResult)
//{
//	int err = ERR_OK;
//
//	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_GetStringMeasurement", &timeStart);
//
//	if (LibsInitialized && bIQ201xFound)
//	{
//		err = IQ2010EXT_GetStringMeasurement( measurementName,
//											  result,
//											  bufferSize,
//											  indexResult
//											 );	// TODO
//	}
//	else
//	{
//		err = ERR_NOT_INITIALIZED;
//	}
//
//	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_GetStringMeasurement", &timeDuration, &timeStop);
//	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_GetStringMeasurement", timeDuration);
//
//	return err;
//}
//
//IQMEASURE_API int LP_IQ2010EXT_GetVectorMeasurement( char  *measurementName,
//													 double bufferReal[],
//													 double bufferImag[],
//													 int    bufferLength,
//													 int   *dataSize,
//													 int    indexResult)
//{
//	int err = ERR_OK;
//
//	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_GetVectorMeasurement", &timeStart);
//
//	if (LibsInitialized && bIQ201xFound)
//	{
//// 		err = IQ2010EXT_GetVectorMeasurement( measurementName,
//// 											  bufferReal,
//// 											  bufferImag,
//// 											  bufferLength,
//// 											  dataSize,
//// 											  indexResult
//// 											 );	// TODO
//	}
//	else
//	{
//		err = ERR_NOT_INITIALIZED;
//	}
//
//	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_GetVectorMeasurement", &timeDuration, &timeStop);
//	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_GetVectorMeasurement", timeDuration);
//
//	return err;
//}
//
//IQMEASURE_API int LP_IQ2010EXT_GetIntMeasurement( char *measurementName,
//												  int *result,
//												  int indexResult)
//{
//	int err = ERR_OK;
//
//	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_GetIntMeasurement", &timeStart);
//
//	if (LibsInitialized && bIQ201xFound)
//	{
//		err = IQ2010EXT_GetIntMeasurement( measurementName,
//										   result,
//										   indexResult
//										  );	// TODO
//	}
//	else
//	{
//		err = ERR_NOT_INITIALIZED;
//	}
//
//	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_GetIntMeasurement", &timeDuration, &timeStop);
//	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_GetIntMeasurement", timeDuration);
//
//	return err;
//}
//
//IQMEASURE_API int LP_IQ2010EXT_LoadMultiSegmentWaveFile(const char *signalFile, const char *markerFile)
//{
//	int err = ERR_OK;
//
//	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_LoadMultiSegmentWaveFile", &timeStart);
//
//	if (LibsInitialized && bIQ201xFound)
//	{
////		err = IQ2010EXT_LoadMultiSegmentWaveFile( signalFile,  markerFile );	// TODO
//	}
//	else
//	{
//		err = ERR_NOT_INITIALIZED;
//	}
//
//	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_LoadMultiSegmentWaveFile", &timeDuration, &timeStop);
//	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_LoadMultiSegmentWaveFile", timeDuration);
//
//	return err;
//}

//FM IQMeasure functions
#ifdef WIN32	//Not using for Mac at this time

IQMEASURE_API int LP_FM_SetVsg(unsigned int carrierFreqHz,
		double carrierPowerdBm,
		int modulationEnable,
		unsigned int totalFmDeviationHz,
		int stereoEnable,
		unsigned int pilotDeviationHz,
		int rdsEnable,
		unsigned int rdsDeviationHz,
		unsigned int preEmphasisUs,
		char* rdsTransmitString)
{
	int err = ERR_OK;
	int carrierOffsetHz = 0;
	int rdsPhaseShift = 0;
	IQV_FM_VSG_PREEMPHASIS_MODES preEmphasisModeSetting;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_SetVsg", &timeStart);

	if(FmInitialized)
	{
		try
		{
			err = hndl->hndlFm->Activate(IQV_FM_VSG_TECH_ACT);
			if(ERR_OK != err) { throw err;}

			hndl->hndlFm->SetParamFrequency(carrierFreqHz);
			hndl->hndlFm->SetParamPowerDbm(carrierPowerdBm);
			hndl->hndlFm->SetParamFrequencyOffset(carrierOffsetHz);

			if(PRE_EMPHASIS_75US == preEmphasisUs)
			{
				preEmphasisModeSetting = IQV_FM_VSG_PREEMPHASIS_75_MICRO_SEC;
			}
			else if(PRE_EMPHASIS_50US == preEmphasisUs)
			{
				preEmphasisModeSetting = IQV_FM_VSG_PREEMPHASIS_50_MICRO_SEC;
			}
			else if(PRE_EMPHASIS_25US == preEmphasisUs)
			{
				preEmphasisModeSetting = IQV_FM_VSG_PREEMPHASIS_25_MICRO_SEC;
			}
			else
			{
				preEmphasisModeSetting = IQV_FM_VSG_PREEMPHASIS_NO_PREEMPHASIS;
			}
			hndl->hndlFm->SetParamPreemphasisMode(preEmphasisModeSetting);
			hndl->hndlFm->SetParamFrequencyDeviation(totalFmDeviationHz); // this is the total deviation
			// totalFmDeviationHz = Audio deviation + pilot deviation ( if stereo enabled) + RDS deviation ( if RDS enabled)
			// so must be carefully specified

			/* For example:
			   Case 1:
			   MONO MODE, RDS DISABLED ( pilotDeviationHz, rdsDeviationHz values are ignored in this case)
			   totalFmDeviationHz = 75000 Hz.
			   then audio deviation = 75000 Hz.

			   Case 2:
			   STEREO MODE, RDS DISABLED ( pilotDeviationHz = 7500, rdsDeviationHz value is ignored)
			   totalFmDeviationHz = 75000 Hz.
			   Audio Deviation = totalFmDeviationHz - pilotDeviationHz( since stereo is enabled) = 67500 Hz.

			   Case 3:
			   STEREO MODE, RDS ENABLED ( pilotDeviationHz = 7500, rdsDeviationHz = 2000)
			   totalFmDeviationHz = 75000 Hz.
			   Audio Deviation = totalFmDeviationHz - pilotDeviationHz - rdsDeviationHz = 65500 Hz */

			if(IQV_FM_VSG_MODULATION_STATE_ON == modulationEnable)
			{
				hndl->hndlFm->SetParamModulationState(IQV_FM_VSG_MODULATION_STATE_ON);
			}
			else
			{
				hndl->hndlFm->SetParamModulationState(IQV_FM_VSG_MODULATION_STATE_OFF);
			}
			if(FM_STEREO == stereoEnable)
			{
				hndl->hndlFm->SetParamStereo(IQV_FM_VSG_STEREO_MODE_ON);
				pilotDeviationHz = pilotDeviationHz / 100;
				pilotDeviationHz = pilotDeviationHz * 100;
				hndl->hndlFm->SetParamPilotDeviation(pilotDeviationHz);

			}
			else
			{
				hndl->hndlFm->SetParamStereo(IQV_FM_VSG_STEREO_MODE_OFF);
				hndl->hndlFm->SetParamPilotDeviation(0);
			}
			if(IQV_FM_VSG_RDS_MODE_ON == rdsEnable)
			{
				hndl->hndlFm->SetParamRdsMode(IQV_FM_VSG_RDS_MODE_ON);
				hndl->hndlFm->SetParamRdsDeviation(rdsDeviationHz);
				if(NULL != rdsTransmitString)
				{
					hndl->hndlFm->LoadRdsData(rdsTransmitString);
				}
				else
				{
					rdsTransmitString = "LitepointFM";
					hndl->hndlFm->LoadRdsData(rdsTransmitString);
				}
				if(IQV_FM_VSG_RDS_PHASE_SHIFT_90 == rdsPhaseShift)
				{
					hndl->hndlFm->SetParamRdsPhaseShift(IQV_FM_VSG_RDS_PHASE_SHIFT_90);
				}
				else
				{
					hndl->hndlFm->SetParamRdsPhaseShift(IQV_FM_VSG_RDS_PHASE_SHIFT_NONE);
				}
			}
			else
			{
				hndl->hndlFm->SetParamRdsMode(IQV_FM_VSG_RDS_MODE_OFF);
				hndl->hndlFm->SetParamRdsDeviation(0);
			}
		}
		catch (int /*vsgSetupErr*/)
		{
			err = ERR_FM_SET_TX_FAILED;
		}
	}

	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_SetVsg", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_SetVsg", timeDuration);

	return err;
}


IQMEASURE_API int LP_FM_GetVsgSettings(unsigned int *carrierFreqHz,
		double *carrierPowerdBm,
		int *modulationEnabled,
		unsigned int *totalFmDeviationHz,
		int *stereoEnabled,
		unsigned int *pilotDeviationHz,
		int *rdsEnabled,
		unsigned int *rdsDeviationHz,
		unsigned int *preEmphasisMode)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_GetVsgSettings", &timeStart);

	if(FmInitialized)
	{
		try
		{


			err = hndl->hndlFm->GetTx();
			if(ERR_OK != err) throw err;

			hndl->hndlFm->GetParamFrequency(carrierFreqHz);
			hndl->hndlFm->GetParamPowerDbm(carrierPowerdBm);
			hndl->hndlFm->GetParamModulationState((IQV_FM_VSG_MODULATION_STATE *)modulationEnabled);
			hndl->hndlFm->GetParamFrequencyDeviation(totalFmDeviationHz);
			hndl->hndlFm->GetParamStereo((IQV_FM_VSG_STEREO_MODES *)stereoEnabled);
			hndl->hndlFm->GetParamPilotDeviation(pilotDeviationHz);
			hndl->hndlFm->GetParamRdsMode((IQV_FM_VSG_RDS_MODES *)rdsEnabled);
			hndl->hndlFm->GetParamRdsDeviation(rdsDeviationHz);
			hndl->hndlFm->GetParamPreemphasisMode((IQV_FM_VSG_PREEMPHASIS_MODES *)preEmphasisMode);

		}
		catch (int /*vsgGetSettingsErr*/)
		{
			err = ERR_FM_GET_RX_FAILED;
		}

	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_GetVsgSettings", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_GetVsgSettings", timeDuration);

	return err;
}


IQMEASURE_API int LP_FM_SetFrequency(unsigned int carrierFreqHz, double carrierPowerdBm = -(DBL_MAX))
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_SetFrequency", &timeStart);

	if(FmInitialized)
	{
		try
		{

			hndl->hndlFm->SetParamFrequency(carrierFreqHz);

			if(carrierPowerdBm != -(DBL_MAX))
				hndl->hndlFm->SetParamPowerDbm(carrierPowerdBm);

			err = hndl->hndlFm->SetTx();
			if(ERR_OK != err) throw err;

		}
		catch (int /*vsgSetupErr*/)
		{
			err = ERR_FM_SET_TX_FAILED;
		}
	}

	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_SetFrequency", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_SetFrequency", timeDuration);
	return err;
}


IQMEASURE_API int LP_FM_SetCarrierPower(double carrierPowerdBm)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_SetCarrierPower", &timeStart);

	if(FmInitialized)
	{
		try
		{

			hndl->hndlFm->SetParamPowerDbm(carrierPowerdBm);

			err = hndl->hndlFm->SetTx();
			if(ERR_OK != err) throw err;

		}
		catch (int /*vsgSetupErr*/)
		{
			err = ERR_FM_SET_TX_FAILED;
		}
	}

	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_SetCarrierPower", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_SetCarrierPower", timeDuration);
	return err;
}


IQMEASURE_API int LP_FM_SetAudioSingleTone(double audioToneFreqHz,
		int leftRightChannelSelect,
		double audioToneAmpPercent,
		int stereo)
{
	int err = ERR_OK;

	int TONE_INDEX_1 = 1;
	int TONE_INDEX_5 = 5;

	double audioToneAmpdB = (20*log10((audioToneAmpPercent/100)));

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_SetAudioSingleTone", &timeStart);
	hndl->hndlFm->RemoveParamAllAudioTone();

	if(FmInitialized)
	{
		try
		{
			if(FM_STEREO == stereo)
			{
				if(LEFT_ONLY == leftRightChannelSelect)
				{
					err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_1, audioToneFreqHz,
							(IQV_FM_VSG_AUDIO_CHANNELS)LEFT_ONLY, audioToneAmpdB);
					if(ERR_OK != err) throw err;
				}
				else if(RIGHT_ONLY == leftRightChannelSelect)
				{
					err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_5, audioToneFreqHz,
							(IQV_FM_VSG_AUDIO_CHANNELS)RIGHT_ONLY, audioToneAmpdB);
					if(ERR_OK != err) throw err;
				}
				else if(LEFT_RIGHT == leftRightChannelSelect)
				{
					err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_1, audioToneFreqHz,
							(IQV_FM_VSG_AUDIO_CHANNELS)LEFT_ONLY, audioToneAmpdB);
					if(ERR_OK != err) throw err;

					err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_5, audioToneFreqHz + 1000,
							(IQV_FM_VSG_AUDIO_CHANNELS)RIGHT_ONLY, audioToneAmpdB);
					if(ERR_OK != err) throw err;
				}
				else if(LEFT_EQUALS_RIGHT == leftRightChannelSelect)
				{
					err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_1, audioToneFreqHz,
							(IQV_FM_VSG_AUDIO_CHANNELS)LEFT_EQUALS_RIGHT, audioToneAmpdB);
					if(ERR_OK != err) throw err;
				}
				else if(LEFT_EQUALS_MINUS_RIGHT == leftRightChannelSelect)
				{
					err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_1, audioToneFreqHz,
							(IQV_FM_VSG_AUDIO_CHANNELS)LEFT_EQUALS_MINUS_RIGHT, audioToneAmpdB);
					if(ERR_OK != err) throw err;
				}
			}

			else if(FM_MONO == stereo)
			{
				err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_1, audioToneFreqHz,
						(IQV_FM_VSG_AUDIO_CHANNELS)LEFT_ONLY, audioToneAmpdB);
				if(ERR_OK != err) throw err;
			}

			/*err = hndl->hndlFm->SetTx();
			  if(ERR_OK != err) throw err;*/
		}

		catch (int /*singleToneAudioSetupErr*/)
		{
			err = ERR_FM_SET_AUDIO_TONE_FAILED;
		}
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_SetAudioSingleTone", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_SetAudioSingleTone", timeDuration);

	return err;

}

IQMEASURE_API int LP_FM_SetAudioToneArray(double* freqHz,
		int* channelSelect,
		double* amplitudePercent,
		int stereo,
		unsigned int toneCount)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_SetAudioToneArray", &timeStart);
	hndl->hndlFm->RemoveParamAllAudioTone();

	if(FmInitialized)
	{

		if(0 == toneCount)
		{
			err = ERR_FM_SET_AUDIO_TONE_FAILED;  // atleast 1 tone needs to be specified to be setup
		}
		else if( 12 < toneCount)
		{
			err = ERR_FM_SET_AUDIO_TONE_FAILED;  // cannot set more than 12 tones in VSG
		}
		else
		{
			try
			{
				for (unsigned int i = 0; i < toneCount; i++)
				{

					if(FM_STEREO == stereo)
					{
						if(0 == channelSelect[i] || 1 == channelSelect[i])
						{
							err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)i, freqHz[i],
									(IQV_FM_VSG_AUDIO_CHANNELS)channelSelect[i], (20*log10(amplitudePercent[i]/100)));
							if(ERR_OK != err) throw err;

						}
						else // channelSelect cannot be LEFT_RIGHT/LEFT_EQUALS_RIGHT/LEFT_EQUALS_MINUS_RIGHT for multitone audio setup. each tone's channel information must be set individually.
						{
							err = ERR_FM_SET_AUDIO_TONE_FAILED;
						}
					}
					else
					{
						err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)i, freqHz[i],
								(IQV_FM_VSG_AUDIO_CHANNELS)LEFT_ONLY, (20*log10(amplitudePercent[i]/100)));
						if(ERR_OK != err) throw err;

					}

				}
				err = hndl->hndlFm->SetTx();
				if(ERR_OK != err) throw err;
			}
			catch (int /*multiToneAudioSetupErr*/)
			{
				err = ERR_FM_GET_AUDIO_TONE_FAILED;
			}
		}
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_SetAudioToneArray", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_SetAudioToneArray", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_SetAudioToneArrayDeviation(double* freqHz,
		int* channelSelect,
		double* amplitudeDeviationHz,
		int stereo,
		unsigned int toneCount)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_SetAudioToneArrayDeviation", &timeStart);
	hndl->hndlFm->RemoveParamAllAudioTone();
	if(FmInitialized)
	{
		//	change audio setup to deviation mode
		hndl->hndlFm->SetParamAudioToneMode(IQV_FM_AUDIO_TONE_DEVIATION);

		if(0 == toneCount)
		{
			err = ERR_FM_SET_AUDIO_TONE_FAILED;  // atleast 1 tone needs to be specified to be setup
		}
		else if( 12 < toneCount)
		{
			err = ERR_FM_SET_AUDIO_TONE_FAILED;  // cannot set more than 12 tones in VSG
		}
		else
		{

			for (unsigned int i = 0; i < toneCount; i++)
			{
				try
				{


					if(FM_STEREO == stereo)
					{
						if(0 == channelSelect[i] || 1 == channelSelect[i] || 3 == channelSelect[i] || 4 == channelSelect[i] )
						{
							err = hndl->hndlFm->SetParamAudioToneWDeviation((IQV_FM_VSG_AUDIO_TONE_INDEX)i, freqHz[i],
									(IQV_FM_VSG_AUDIO_CHANNELS)channelSelect[i], amplitudeDeviationHz[i]);
							if(ERR_OK != err) throw err;

						}
						else // channelSelect cannot be LEFT_RIGHT for multitone audio setup in deviation mode
						{
							err = ERR_FM_SET_AUDIO_TONE_FAILED;
						}
					}
					else
					{
						err = hndl->hndlFm->SetParamAudioToneWDeviation((IQV_FM_VSG_AUDIO_TONE_INDEX)i, freqHz[i],
								(IQV_FM_VSG_AUDIO_CHANNELS)LEFT_ONLY, (amplitudeDeviationHz[i]));
						if(ERR_OK != err) throw err;

					}

					err = hndl->hndlFm->SetTx();
					if(ERR_OK != err) throw err;

				}
				catch (int /*multiToneAudioSetupErr*/)
				{
					err = ERR_FM_GET_AUDIO_TONE_FAILED;
				}
			}
		}
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_SetAudioToneArrayDeviation", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_SetAudioToneArrayDeviation", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_GetAudioToneArray(double *freqHz,
		int *channelSelect,
		double *amplitudePercent,
		unsigned int toneCount)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_GetAudioToneArray", &timeStart);

	if(FmInitialized)
	{
		try
		{

			err = hndl->hndlFm->GetTx();
			if(ERR_OK != err) throw err;

			double* audioFreqHz = new double;
			IQV_FM_VSG_AUDIO_CHANNELS* channel =  new IQV_FM_VSG_AUDIO_CHANNELS;
			double* audioAmplitudedB = new double;

			for (unsigned int toneIndex = 0; toneIndex < toneCount; toneIndex++)
			{

				err = hndl->hndlFm->GetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)toneIndex, audioFreqHz, channel, audioAmplitudedB);
				if(ERR_OK != err) throw err;

				freqHz[toneIndex] = *audioFreqHz;
				channelSelect[toneIndex] = *channel;
				amplitudePercent[toneIndex] = (pow(10,(*audioAmplitudedB/20))*100);


			}

			delete audioFreqHz;
			delete channel;
			delete audioAmplitudedB;
		}
		catch (int /*getAudioSetupErr*/)
		{
			err = ERR_FM_GET_AUDIO_TONE_FAILED;
		}
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_GetAudioToneArray", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_GetAudioToneArray", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_SetAudioToneModeAmplitude(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_SetAudioToneModeAmplitude", &timeStart);

	if(FmInitialized)
	{

		hndl->hndlFm->SetParamAudioToneMode(IQV_FM_AUDIO_TONE_AMPLITUDE);
		if(hndl->hndlFm->SetTx())
		{
			err = ERR_FM_SET_TX_FAILED;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_SetAudioToneModeAmplitude", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_SetAudioToneModeAmplitude", timeDuration);

	return err;
}


IQMEASURE_API int LP_FM_DeleteAudioTones()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_DeleteAudioTones", &timeStart);

	if(FmInitialized)
	{

		hndl->hndlFm->RemoveParamAllAudioTone();
		/*err = hndl->hndlFm->SetTx();
		  if(err)
		  {
		  err = ERR_FM_SET_TX_FAILED;
		  }*/
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_DeleteAudioTones", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_DeleteAudioTones", timeDuration);

	return err;
}


IQMEASURE_API int LP_FM_StartVsg()
{
	int err = ERR_OK;
	int vsgSettleTimems = 150;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_StartVsg", &timeStart);

	if(FmInitialized)
	{

		hndl->hndlFm->SetParamRfOutput(IQV_FM_VSG_RF_OUTPUT_ON);

		if(hndl->hndlFm->SetTx())
		{
			err = ERR_FM_SET_TX_FAILED;
		}
		Sleep(vsgSettleTimems);
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_StartVsg", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_StartVsg", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_StopVsg()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_StopVsg", &timeStart);

	if(FmInitialized)
	{
		try
		{


			hndl->hndlFm->SetParamRfOutput(IQV_FM_VSG_RF_OUTPUT_OFF);
			err = hndl->hndlFm->SetTx();
			if(ERR_OK != err) throw err;

			err = hndl->hndlFm->Deactivate(IQV_FM_VSG_TECH_ACT);
			if(ERR_OK != err) throw err;

		}
		catch(int /*stopVsgErr*/)
		{
			err = ERR_FM_SET_TX_FAILED;
		}
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_StopVsg", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_StopVsg", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_SetVsgDistortion(int amDistortionEnable,
		unsigned int amFrequencyHz,
		unsigned int amDepthPercent)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_SetVsgDistortion", &timeStart);

	if(FmInitialized)
	{

		if(IQV_FM_VSG_DISTORTION_AM == amDistortionEnable)
		{
			hndl->hndlFm->SetParamDistortion(IQV_FM_VSG_DISTORTION_AM, amFrequencyHz, amDepthPercent);
		}
		else
		{
			hndl->hndlFm->SetParamDistortion(IQV_FM_VSG_DISTORTION_OFF, 0, 0);
		}
		if(hndl->hndlFm->SetTx())
		{
			err = ERR_FM_SET_TX_FAILED;
		}
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_SetVsgDistortion", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_SetVsgDistortion", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_GetVsgDistortion(double *amDistortionEnableStatus,
		unsigned int *amFrequencyHz,
		unsigned int *amDepthPercent)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_GetVsgDistortion", &timeStart);

	if(FmInitialized)
	{

		if(hndl->hndlFm->GetTx())
		{
			err = ERR_FM_GET_RX_FAILED;
		}
		IQV_FM_VSG_DISTORTION_TYPES *amDistortionEnableStatus = new IQV_FM_VSG_DISTORTION_TYPES;

		hndl->hndlFm->GetParamDistortion(amDistortionEnableStatus, amFrequencyHz, amDepthPercent);
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_GetVsgDistortion", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_GetVsgDistortion", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_StartVsgInterference(double interfererRelativePowerdBm,
		int interfererCarrierOffsetHz,
		int interfererPeakFreqDeviationHz,
		int interfererModulationEnable,
		unsigned int interfererAudioFreqHz)
{

	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_StartVsgInterference", &timeStart);

	if(FmInitialized)
	{

		if (IQV_FM_VSG_MODULATION_FM == interfererModulationEnable)
		{
			hndl->hndlFm->SetParamInterferer(IQV_FM_VSG_INTERFERING_CARRIER_OFF,interfererCarrierOffsetHz,interfererRelativePowerdBm,
					interfererPeakFreqDeviationHz);
			if(hndl->hndlFm->SetTx())
			{
				err = ERR_FM_SET_TX_FAILED;
			}

			hndl->hndlFm->SetParamInterfererModulation(IQV_FM_VSG_MODULATION_FM, interfererAudioFreqHz);

			hndl->hndlFm->SetParamInterferer(IQV_FM_VSG_INTERFERING_CARRIER_ON,interfererCarrierOffsetHz,interfererRelativePowerdBm,
					interfererPeakFreqDeviationHz);

		}
		else
		{
			hndl->hndlFm->SetParamInterfererModulation(IQV_FM_VSG_MODULATION_FM, 0);
			hndl->hndlFm->SetParamInterferer(IQV_FM_VSG_INTERFERING_CARRIER_ON,interfererCarrierOffsetHz,interfererRelativePowerdBm,
					0);
		}
		if(hndl->hndlFm->SetTx())
		{
			err = ERR_FM_SET_TX_FAILED;
		}
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_StartVsgInterference", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_StartVsgInterference", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_GetVsgInterferenceSettings(double *interfererRelativePowerdBm,
		int *interfererCarrierOffsetHz,
		int *interfererPeakFreqDeviationHz,
		int *interfererModulationEnabled,
		unsigned int *interfererAudioFreqHz)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_GetVsgInterferenceSettings", &timeStart);

	if(FmInitialized)
	{

		if(hndl->hndlFm->GetTx())
		{
			err = ERR_FM_GET_RX_FAILED;
		}
		IQV_FM_VSG_INTERFERING_CARRIER_MODES *interfererMode = new IQV_FM_VSG_INTERFERING_CARRIER_MODES;

		hndl->hndlFm->GetParamInterferer(interfererMode, interfererCarrierOffsetHz, interfererRelativePowerdBm, interfererPeakFreqDeviationHz);
		hndl->hndlFm->GetParamInterfererModulation((IQV_FM_VSG_MODULATION_TYPES *)interfererModulationEnabled, interfererAudioFreqHz);
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_GetVsgInterferenceSettings", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_GetVsgInterferenceSettings", timeDuration);

	return err;
}


IQMEASURE_API int LP_FM_StopVsgInterference()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_StopVsgInterference", &timeStart);

	if(FmInitialized)
	{


		hndl->hndlFm->SetParamInterferer(IQV_FM_VSG_INTERFERING_CARRIER_OFF,0,0,0);

		if(hndl->hndlFm->SetTx())
		{
			err = ERR_FM_SET_TX_FAILED;
		}
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_StopVsgInterference", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_StopVsgInterference", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_SetVsa(unsigned int carrierFreqHz, double expectedPeakInputPowerdBm)
{

	int err = ERR_OK;

	double marginalInputPowerdBm = 1;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_SetVsa", &timeStart);

	if(FmInitialized)
	{
		int agcMode = OFF;
		int vsaSampleRate = SAMPLE_640KHz;
		double expectedPeakInputPowerdBmSetting;

		try
		{

			err = hndl->hndlFm->Activate(IQV_FM_VSA_TECH_ACT);
			if (ERR_OK != err) throw err;

			hndl->hndlFm->SetParamVsaAgcMode((IQV_FM_VSA_AGC_MODES)agcMode);
			hndl->hndlFm->SetParamVsaSampleRate((IQV_FM_VSA_SAMPLE_RATES)vsaSampleRate);
			hndl->hndlFm->SetParamVsaFrequency(carrierFreqHz);
			if(expectedPeakInputPowerdBm >= 10)
			{
				expectedPeakInputPowerdBmSetting = 10;
			}
			else if(expectedPeakInputPowerdBm <= -40)
			{
				expectedPeakInputPowerdBmSetting = -40;
			}
			else
			{
				expectedPeakInputPowerdBmSetting = (expectedPeakInputPowerdBm + marginalInputPowerdBm);
			}
			hndl->hndlFm->SetParamVsaPowerDbm(expectedPeakInputPowerdBmSetting);

			hndl->hndlFm->SetParamVsaRfInput(IQV_FM_VSA_RF_INPUT_ON);


			err = hndl->hndlFm->SetRx();
			if (ERR_OK != err) throw err;
		}
		catch (int /*vsaSetupErr*/)
		{
			err = ERR_FM_SET_RX_FAILED;
		}
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_SetVsa", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_SetVsa", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_GetVsaSettings(unsigned int *carrierFreqHz, double *expectedPeakInputPowerdBm)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_GetVsaSettings", &timeStart);

	if(FmInitialized)
	{


		if(hndl->hndlFm->GetRx())
		{
			err = ERR_FM_GET_RX_FAILED;
		}

		hndl->hndlFm->GetParamVsaFrequency(carrierFreqHz);
		hndl->hndlFm->GetParamVsaPowerDbm(expectedPeakInputPowerdBm);
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_GetVsaSettings", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_GetVsaSettings", timeDuration);

	return err;
}


IQMEASURE_API int LP_FM_VsaDataCapture(double captureTimemillisec)
{
	int err = ERR_OK;
	unsigned int captureTime = captureTimemillisec > 0 ? (int)captureTimemillisec : 0;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_VsaDataCapture", &timeStart);

	if(FmInitialized)
	{
		//Clear previous results stored
		g_FmRfScalarResults.clear();
		g_FmMonoScalarResults.clear();
		g_FmStereoScalarResults.clear();

		g_FmRfVectorResults.clear();
		g_FmMonoVectorResults.clear();
		g_FmStereoVectorResults.clear();


		if(hndl->hndlFm->Capture(captureTime))
		{
			err = ERR_FM_VSA_CAPTURE_FAILED;
		}
		// reset the audioCaptureOnly flag to indicate that this is a VSACapture
		g_audioCaptureOnly = false;
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_VsaDataCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_VsaDataCapture", timeDuration);

	return err;
}

// Analyze functions

IQMEASURE_API int LP_FM_Analyze_RF(int rfRBWHz, int rfOBWPercent, int rfPowerMeasBWHz, int windowType)
{
	int err = ERR_OK;

	//Clear previous results stored
	g_FmRfScalarResults.clear();

	g_FmRfVectorResults.clear();

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_Analyze_RF", &timeStart);

	if (FmInitialized)
	{

		if (!hndl->hndlFm->dataFm)
		{
			err = ERR_FM_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->hndlFm->dataFm->length)		//crezon revision
			{
				err = ERR_FM_NO_CAPTURE_DATA;
				return err;
			}
		}

		analysisFmRf->saResBw = rfRBWHz;
		analysisFmRf->rfPowerMeasurementBw = rfPowerMeasBWHz;
		analysisFmRf->percentObw = rfOBWPercent;
		analysisFmRf->windowType = (IQV_FM_WINDOW_TYPE)windowType;

		hndl->hndlFm->analysis = dynamic_cast<iqapiAnalysisFmRf *>(analysisFmRf);

		if (LP_Analyze_FM_Hndl())
		{
			err = ERR_FM_ANALYSIS_FAILED;
		}
		else
		{

		}
	}

	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_Analyze_RF", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_Analyze_RF", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_Analyze_Demod(int audioRBWHz)
{
	int err = ERR_OK;

	//Clear previous results stored
	g_FmRfScalarResults.clear();

	g_FmRfVectorResults.clear();

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_Analyze_Demod", &timeStart);

	if (FmInitialized)
	{
		if (!hndl->hndlFm->dataFm)
		{
			err = ERR_FM_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->hndlFm->dataFm->length)		//crezon revision
			{
				err = ERR_FM_NO_CAPTURE_DATA;
				return err;
			}
		}

		analysisFmDemodSpectrum->saResBw = audioRBWHz;

		hndl->hndlFm->analysis = dynamic_cast<iqapiAnalysisFmDemodSpectrum *>(analysisFmDemodSpectrum);

		if (LP_Analyze_FM_Hndl())
		{
			err = ERR_FM_ANALYSIS_FAILED;
		}
		else
		{

		}
	}
	else
		err = ERR_FM_NOT_INITIALIZED;

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_Analyze_Demod", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_Analyze_Demod", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_Analyze_Audio_Mono(int audioRBWHz,
		int audioFreqLowLimitHz,
		int audioFreqHiLimitHz,
		int filterCount,
		int filterType1,
		double filterParam1,
		int filterType2,
		double filterParam2,
		int filterType3,
		double filterParam3)

{
	int err = ERR_OK;

	//Clear previous results stored
	g_FmMonoScalarResults.clear();
	g_FmMonoVectorResults.clear();

	IQV_FM_FILTER_TYPE filter1 =(IQV_FM_FILTER_TYPE)filterType1;
	double filterPar1 = filterParam1;
	IQV_FM_FILTER_TYPE filter2 =(IQV_FM_FILTER_TYPE)filterType2;
	double filterPar2 = filterParam2;
	IQV_FM_FILTER_TYPE filter3 =(IQV_FM_FILTER_TYPE)filterType3;
	double filterPar3 = filterParam3;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_Analyze_Audio_Mono", &timeStart);

	if (FmInitialized)
	{
		if (!hndl->hndlFm->dataFm)
		{
			err = ERR_FM_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->hndlFm->dataFm->length)		//crezon revision
			{
				err = ERR_FM_NO_CAPTURE_DATA;
				return err;
			}
		}

		if(g_audioCaptureOnly)
		{
			analysisAudioMono->saNFilters = filterCount;
			analysisAudioMono->saResBw = audioRBWHz;
			analysisAudioMono->saFilterType1 = filter1;
			analysisAudioMono->saFilterParam1 = filterPar1;
			analysisAudioMono->saFilterType2 = filter2;
			analysisAudioMono->saFilterParam2 = filterPar2;
			analysisAudioMono->saFilterType3 = filter3;
			analysisAudioMono->saFilterParam3 = filterPar3;
			analysisAudioMono->audFreqL = audioFreqLowLimitHz;
			analysisAudioMono->audFreqH = audioFreqHiLimitHz;

			hndl->hndlFm->analysis = dynamic_cast<iqapiAnalysisAudioMono *>(analysisAudioMono);
		}
		else
		{
			analysisFmMono->saNFilters = filterCount;
			analysisFmMono->saResBw = audioRBWHz;
			analysisFmMono->saFilterType1 = filter1;
			analysisFmMono->saFilterParam1 = filterPar1;
			analysisFmMono->saFilterType2 = filter2;
			analysisFmMono->saFilterParam2 = filterPar2;
			analysisFmMono->saFilterType3 = filter3;
			analysisFmMono->saFilterParam3 = filterPar3;
			analysisFmMono->audFreqL = audioFreqLowLimitHz;
			analysisFmMono->audFreqH = audioFreqHiLimitHz;

			hndl->hndlFm->analysis = dynamic_cast<iqapiAnalysisFmMono *>(analysisFmMono);

		}

		if (LP_Analyze_FM_Hndl())
		{
			err = ERR_FM_ANALYSIS_FAILED;
		}
		else
		{

		}
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_Analyze_Audio_Mono", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_Analyze_Audio_Mono", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_Analyze_Audio_Stereo(int audioRBWHz,
		int audioFreqLowLimitHz,
		int audioFreqHiLimitHz,
		int filterCount,
		int filterType1,
		double filterParam1,
		int filterType2,
		double filterParam2,
		int filterType3,
		double filterParam3)

{
	int err = ERR_OK;

	//Clear previous results stored
	g_FmStereoScalarResults.clear();

	g_FmStereoVectorResults.clear();

	IQV_FM_FILTER_TYPE filter1 =(IQV_FM_FILTER_TYPE)filterType1;
	double filterPar1 = filterParam1;
	IQV_FM_FILTER_TYPE filter2 =(IQV_FM_FILTER_TYPE)filterType2;
	double filterPar2 = filterParam2;
	IQV_FM_FILTER_TYPE filter3 =(IQV_FM_FILTER_TYPE)filterType3;
	double filterPar3 = filterParam3;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_Analyze_Audio_Stereo", &timeStart);

	if (FmInitialized)
	{
		if (!hndl->hndlFm->dataFm)
		{
			err = ERR_FM_NO_CAPTURE_DATA;
			return err;
		}
		else
		{
			if (!hndl->hndlFm->dataFm->length)		//crezon revision
			{
				err = ERR_FM_NO_CAPTURE_DATA;
				return err;
			}
		}

		if(g_audioCaptureOnly)
		{
			analysisAudioStereo->saNFilters = filterCount;
			analysisAudioStereo->saResBw = audioRBWHz;
			analysisAudioStereo->saFilterType1 = filter1;
			analysisAudioStereo->saFilterParam1 = filterPar1;
			analysisAudioStereo->saFilterType2 = filter2;
			analysisAudioStereo->saFilterParam2 = filterPar2;
			analysisAudioStereo->saFilterType3 = filter3;
			analysisAudioStereo->saFilterParam3 = filterPar3;
			analysisAudioStereo->audFreqL = audioFreqLowLimitHz;
			analysisAudioStereo->audFreqH = audioFreqHiLimitHz;

			hndl->hndlFm->analysis = dynamic_cast<iqapiAnalysisAudioStereo *>(analysisAudioStereo);
		}
		else
		{
			//analysisFmStereo->saNFilters = filterCount;
			//analysisFmStereo->saResBw = audioRBWHz;
			//analysisFmStereo->saFilterType1 = filter1;
			//analysisFmStereo->saFilterParam1 = filterPar1;
			//analysisFmStereo->saFilterType2 = filter2;
			//analysisFmStereo->saFilterParam2 = filterPar2;
			//analysisFmStereo->saFilterType3 = filter3;
			//analysisFmStereo->saFilterParam3 = filterPar3;
			//analysisFmStereo->audFreqL = audioFreqLowLimitHz;
			//analysisFmStereo->audFreqH = audioFreqHiLimitHz;

			//hndl->hndlFm->analysis = dynamic_cast<iqapiAnalysisFmStereo *>(analysisFmStereo);

			analysisFmAuto->saNFilters = filterCount;
			analysisFmAuto->saResBw = audioRBWHz;
			analysisFmAuto->saFilterType1 = filter1;
			analysisFmAuto->saFilterParam1 = filterPar1;
			analysisFmAuto->saFilterType2 = filter2;
			analysisFmAuto->saFilterParam2 = filterPar2;
			analysisFmAuto->saFilterType3 = filter3;
			analysisFmAuto->saFilterParam3 = filterPar3;
			analysisFmAuto->audFreqL = audioFreqLowLimitHz;
			analysisFmAuto->audFreqH = audioFreqHiLimitHz;

			hndl->hndlFm->analysis = dynamic_cast<iqapiAnalysisFmAuto *>(analysisFmAuto);
		}

		if (LP_Analyze_FM_Hndl())
		{
			err = ERR_FM_ANALYSIS_FAILED;
		}
		else
		{

		}
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_Analyze_Audio_Stereo", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_Analyze_Audio_Stereo", timeDuration);

	return err;
}

IQMEASURE_API double LP_FM_GetScalarMeasurement_NoTimer(char *measurement, int index)
{
	if (FmInitialized)
	{
		if (!hndl->hndlFm->results)
		{
			return NA_NUMBER;
		}
		else
		{
			if(hndl->hndlFm->results)
			{
				if (dynamic_cast<iqapiResultFmRf *>(hndl->hndlFm->results))
				{
					resultFmRf = dynamic_cast<iqapiResultFmRf *>(hndl->hndlFm->results);

					if(g_FmRfScalarResults.find(measurement) == g_FmRfScalarResults.end())
					{
						g_FmRfScalarResults["totalPower"] = resultFmRf->totalPower;
						g_FmRfScalarResults["powerInBw"] = resultFmRf->powerInBw;
						g_FmRfScalarResults["obw"] = resultFmRf->obw;
						g_FmRfScalarResults["spectrumWaveLength"] = resultFmRf->spectrumWaveLength;
						g_FmRfScalarResults["RfSpectrumWaveSaLength"] = resultFmRf->RfSpectrumWaveSaLength;
						g_FmRfScalarResults["RfSpectrumWaveSaResBw"] = resultFmRf->RfSpectrumWaveSaResBw;
						g_FmRfScalarResults["RfSpectrumWaveSaNoiseBw"] = resultFmRf->RfSpectrumWaveSaNoiseBw;
						g_FmRfScalarResults["RfSpectrumWaveSaFreqStart"] = resultFmRf->RfSpectrumWaveSaFreqStart;
						g_FmRfScalarResults["RfSpectrumWaveSaFreqStop"] = resultFmRf->RfSpectrumWaveSaFreqStop;

					}
					else
					{
						// DO nothing
					}
					if(g_FmRfScalarResults.find(measurement) == g_FmRfScalarResults.end())
					{
						return NA_NUMBER;
					}
					else
					{
						return((double)g_FmRfScalarResults.find(measurement)->second);
					}
				}
				else if (dynamic_cast<iqapiResultFmDemodSpectrum *>(hndl->hndlFm->results))
				{
					resultFmDemodSpectrum = dynamic_cast<iqapiResultFmDemodSpectrum *>(hndl->hndlFm->results);

					if(g_FmRfScalarResults.find(measurement) == g_FmRfScalarResults.end())
					{

						g_FmRfScalarResults["demodPlusPeakDeviation"] = resultFmDemodSpectrum->demodPlusPeakDeviation;
						g_FmRfScalarResults["demodMinusPeakDeviation"] = resultFmDemodSpectrum->demodMinusPeakDeviation;
						g_FmRfScalarResults["demodAveragePeakDeviation"] = resultFmDemodSpectrum->demodAveragePeakDeviation;
						g_FmRfScalarResults["demodRmsPeakDeviation"] = resultFmDemodSpectrum->demodRmsPeakDeviation;
						g_FmRfScalarResults["demodCfo"] = resultFmDemodSpectrum->demodCfo;

						g_FmRfScalarResults["audioSpectrumWaveSaLength"] = resultFmDemodSpectrum->audioSpectrumWaveSaFreqStop;
						g_FmRfScalarResults["audioSpectrumWaveSaResBw"] = resultFmDemodSpectrum->audioSpectrumWaveSaFreqStop;
						g_FmRfScalarResults["audioSpectrumWaveSaNoiseBw"] = resultFmDemodSpectrum->audioSpectrumWaveSaFreqStop;
						g_FmRfScalarResults["audioSpectrumWaveSaFreqStart"] = resultFmDemodSpectrum->audioSpectrumWaveSaFreqStop;
						g_FmRfScalarResults["audioSpectrumWaveSaFreqStop"] = resultFmDemodSpectrum->audioSpectrumWaveSaFreqStop;

						g_FmRfScalarResults["audioSpectrumWaveLength"] = resultFmDemodSpectrum->audioSpectrumWaveLength;
						g_FmRfScalarResults["audioWaveLength"] = resultFmDemodSpectrum->audioWaveLength;

					}
					else
					{
						// DO nothing
					}
					if(g_FmRfScalarResults.find(measurement) == g_FmRfScalarResults.end())
					{
						return NA_NUMBER;
					}
					else
					{
						return((double)g_FmRfScalarResults.find(measurement)->second);
					}
				}
				else if (dynamic_cast<iqapiResultAudioStereo *>(hndl->hndlFm->results))
				{
					resultAudioStereo = dynamic_cast<iqapiResultAudioStereo *>(hndl->hndlFm->results);

					if(g_FmStereoScalarResults.find(measurement) == g_FmStereoScalarResults.end())
					{
						g_FmStereoScalarResults["audioSpectrumWaveSaLength"] = resultAudioStereo->audioSpectrumWaveSaLength;
						g_FmStereoScalarResults["audioSpectrumWaveSaResBw"] = resultAudioStereo->audioSpectrumWaveSaResBw;
						g_FmStereoScalarResults["audioSpectrumWaveSaNoiseBw"] = resultAudioStereo->audioSpectrumWaveSaNoiseBw;
						g_FmStereoScalarResults["audioSpectrumWaveSaFreqStart"] = resultAudioStereo->audioSpectrumWaveSaFreqStart;
						g_FmStereoScalarResults["audioSpectrumWaveSaFreqStop"] = resultAudioStereo->audioSpectrumWaveSaFreqStop;

						g_FmStereoScalarResults["leftAudioSpectrumWaveLength"] = resultAudioStereo->leftAudioSpectrumWaveLength;
						g_FmStereoScalarResults["leftAudioSpectrumWaveToneNSigFreq"] = resultAudioStereo->leftAudioSpectrumWaveToneNSigFreq;

						g_FmStereoScalarResults["leftAudioSpectrumWaveLength"] = resultAudioStereo->leftAudioSpectrumWaveLength;
						g_FmStereoScalarResults["leftAudioSpectrumWaveToneNSigFreq"] = resultAudioStereo->leftAudioSpectrumWaveToneNSigFreq;

						g_FmStereoScalarResults["rightAudioSpectrumWaveLength"] = resultAudioStereo->rightAudioSpectrumWaveLength;
						g_FmStereoScalarResults["rightAudioSpectrumWaveToneNSigFreq"] = resultAudioStereo->rightAudioSpectrumWaveToneNSigFreq;

						g_FmStereoScalarResults["leftAudioSpectrumWaveToneSigPower"] = resultAudioStereo->leftAudioSpectrumWaveToneSigPower[0];
						g_FmStereoScalarResults["rightAudioSpectrumWaveToneSigPower"] = resultAudioStereo->rightAudioSpectrumWaveToneSigPower[0];

						g_FmStereoScalarResults["leftAudioAnaSnr"] = resultAudioStereo->leftAudioAnaSnr;
						g_FmStereoScalarResults["rightAudioAnaSnr"] = resultAudioStereo->rightAudioAnaSnr;

						g_FmStereoScalarResults["leftAudioAnaSinad"] = resultAudioStereo->leftAudioAnaSinad;
						g_FmStereoScalarResults["rightAudioAnaSinad"] = resultAudioStereo->rightAudioAnaSinad;

						g_FmStereoScalarResults["leftAudioAnaThd"] = resultAudioStereo->leftAudioAnaThd;
						g_FmStereoScalarResults["rightAudioAnaThd"] = resultAudioStereo->rightAudioAnaThd;

						g_FmStereoScalarResults["leftAudioAnaThdPlusN"] = (20*log10(resultAudioStereo->leftAudioAnaThdPlusN));
						g_FmStereoScalarResults["rightAudioAnaThdPlusN"] = (20*log10(resultAudioStereo->rightAudioAnaThdPlusN));

						g_FmStereoScalarResults["leftAudioAnaTnhd"] = resultAudioStereo->leftAudioAnaTnhd;
						g_FmStereoScalarResults["rightAudioAnaTnhd"] = resultAudioStereo->rightAudioAnaTnhd;

						g_FmStereoScalarResults["crosstalkMaxPower"] = resultAudioStereo->crosstalkMaxPower;
						g_FmStereoScalarResults["crosstalkCrosstalk"] = resultAudioStereo->crosstalkCrosstalk[0];
						g_FmStereoScalarResults["crosstalkNSigFreq"] = resultAudioStereo->crosstalkNSigFreq;
					}
					else
					{

					}
					if(g_FmStereoScalarResults.find(measurement) == g_FmStereoScalarResults.end())
					{
						return NA_NUMBER;
					}
					else
					{
						return((double)g_FmStereoScalarResults.find(measurement)->second);
					}
				}
				else if (dynamic_cast<iqapiResultAudioMono *>(hndl->hndlFm->results))
				{

					resultAudioMono = dynamic_cast<iqapiResultAudioMono *>(hndl->hndlFm->results);
					if(g_FmMonoScalarResults.find(measurement) == g_FmMonoScalarResults.end())
					{

						g_FmMonoScalarResults["audioSpectrumWaveSaLength"] = resultAudioMono->audioSpectrumWaveSaLength;
						g_FmMonoScalarResults["audioSpectrumWaveSaResBw"] = resultAudioMono->audioSpectrumWaveSaResBw;
						g_FmMonoScalarResults["audioSpectrumWaveSaNoiseBw"] = resultAudioMono->audioSpectrumWaveSaNoiseBw;
						g_FmMonoScalarResults["audioSpectrumWaveSaFreqStart"] = resultAudioMono->audioSpectrumWaveSaFreqStart;
						g_FmMonoScalarResults["audioSpectrumWaveSaFreqStop"] = resultAudioMono->audioSpectrumWaveSaFreqStop;

						g_FmMonoScalarResults["audioAnaSnr"] = resultAudioMono->audioAnaSnr;
						g_FmMonoScalarResults["audioAnaSinad"] = resultAudioMono->audioAnaSinad;
						g_FmMonoScalarResults["audioAnaThd"] = resultAudioMono->audioAnaThd;
						g_FmMonoScalarResults["audioAnaThdPlusN"] = (20*log10(resultAudioMono->audioAnaThdPlusN));
						g_FmMonoScalarResults["audioAnaTnhd"] = resultAudioMono->audioAnaTnhd;

						g_FmMonoScalarResults["audioSpectrumWaveToneSigPower"] = resultAudioMono->audioSpectrumWaveToneSigPower[0];

						g_FmMonoScalarResults["audioSpectrumWaveLength"] = resultAudioMono->audioSpectrumWaveLength;
						g_FmMonoScalarResults["audioSpectrumWaveToneNSigFreq"] = resultAudioMono->audioSpectrumWaveToneNSigFreq;

					}
					else
					{

					}
					if(g_FmMonoScalarResults.find(measurement) == g_FmMonoScalarResults.end())
					{
						return NA_NUMBER;
					}
					else
					{
						return((double)g_FmMonoScalarResults.find(measurement)->second);
					}
				}
				else if (dynamic_cast<iqapiResultFmStereo *>(hndl->hndlFm->results))
				{
					resultFmStereo = dynamic_cast<iqapiResultFmStereo *>(hndl->hndlFm->results);
					if(g_FmStereoScalarResults.find(measurement) == g_FmStereoScalarResults.end())
					{
						g_FmStereoScalarResults["leftAudioSpectrumWaveToneSigPower"] = resultFmStereo->leftAudioSpectrumWaveToneSigPower[0];
						g_FmStereoScalarResults["rightAudioSpectrumWaveToneSigPower"] = resultFmStereo->rightAudioSpectrumWaveToneSigPower[0];

						g_FmStereoScalarResults["leftAudioAnaSnr"] = resultFmStereo->leftAudioAnaSnr;
						g_FmStereoScalarResults["rightAudioAnaSnr"] = resultFmStereo->rightAudioAnaSnr;
						g_FmStereoScalarResults["leftAudioAnaSinad"] = resultFmStereo->leftAudioAnaSinad;
						g_FmStereoScalarResults["rightAudioAnaSinad"] = resultFmStereo->rightAudioAnaSinad;

						g_FmStereoScalarResults["leftAudioAnaThd"] = resultFmStereo->leftAudioAnaThd;
						g_FmStereoScalarResults["rightAudioAnaThd"] = resultFmStereo->rightAudioAnaThd;

						g_FmStereoScalarResults["leftAudioAnaThdPlusN"] = (20*log10(resultFmStereo->leftAudioAnaThdPlusN));
						g_FmStereoScalarResults["rightAudioAnaThdPlusN"] = (20*log10(resultFmStereo->rightAudioAnaThdPlusN));

						g_FmStereoScalarResults["leftAudioAnaTnhd"] = resultFmStereo->leftAudioAnaTnhd;
						g_FmStereoScalarResults["rightAudioAnaTnhd"] = resultFmStereo->rightAudioAnaTnhd;

						g_FmStereoScalarResults["pilotFrequencyOffset"] = resultFmStereo->pilotFrequencyOffset;
						g_FmStereoScalarResults["crosstalkCrosstalk"] = resultFmStereo->crosstalkCrosstalk[0];

						g_FmStereoScalarResults["leftAudioSpectrumWaveLength"] = resultFmStereo->leftAudioSpectrumWaveLength;
						g_FmStereoScalarResults["leftAudioWaveLength"] = resultFmStereo->leftAudioWaveLength;
						g_FmStereoScalarResults["leftAudioSpectrumWaveToneNSigFreq"] = resultFmStereo->leftAudioSpectrumWaveToneNSigFreq;

						g_FmStereoScalarResults["rightAudioSpectrumWaveLength"] = resultFmStereo->rightAudioSpectrumWaveLength;
						g_FmStereoScalarResults["rightAudioWaveLength"] = resultFmStereo->rightAudioWaveLength;
						g_FmStereoScalarResults["rightAudioSpectrumWaveToneNSigFreq"] = resultFmStereo->rightAudioSpectrumWaveToneNSigFreq;

						g_FmStereoScalarResults["crosstalkNSigFreq"] = resultFmStereo->crosstalkNSigFreq;
						g_FmStereoScalarResults["crosstalkMaxPower"] = resultFmStereo->crosstalkMaxPower;
						g_FmStereoScalarResults["pilotIsStereo"] = resultFmStereo->pilotIsStereo;

						g_FmStereoScalarResults["audioDeviation"] = resultFmStereo->audioDeviation;
						g_FmStereoScalarResults["pilotDeviation"] = resultFmStereo->pilotDeviation;


					}
					else
					{

					}
					if(g_FmStereoScalarResults.find(measurement) == g_FmStereoScalarResults.end())
					{
						return NA_NUMBER;
					}
					else
					{
						return((double)g_FmStereoScalarResults.find(measurement)->second);
					}
				}

				else if (dynamic_cast<iqapiResultFmMono *>(hndl->hndlFm->results))
				{
					resultFmMono = dynamic_cast<iqapiResultFmMono *>(hndl->hndlFm->results);
					if(g_FmMonoScalarResults.find(measurement) == g_FmMonoScalarResults.end())
					{

						g_FmMonoScalarResults["AudioSpectrumWaveToneSigPower"] = resultFmMono->audioSpectrumWaveToneSigPower[0];

						g_FmMonoScalarResults["audioSpectrumWaveSaLength"] = resultFmMono->audioSpectrumWaveSaLength;
						g_FmMonoScalarResults["audioSpectrumWaveSaResBw"] = resultFmMono->audioSpectrumWaveSaResBw;
						g_FmMonoScalarResults["audioSpectrumWaveSaNoiseBw"] = resultFmMono->audioSpectrumWaveSaNoiseBw;
						g_FmMonoScalarResults["audioSpectrumWaveSaFreqStart"] = resultFmMono->audioSpectrumWaveSaFreqStart;
						g_FmMonoScalarResults["audioSpectrumWaveSaFreqStop"] = resultFmMono->audioSpectrumWaveSaFreqStop;

						g_FmMonoScalarResults["audioAnaSnr"] = resultFmMono->audioAnaSnr;
						g_FmMonoScalarResults["audioAnaSinad"] = resultFmMono->audioAnaSinad;
						g_FmMonoScalarResults["audioAnaThd"] = resultFmMono->audioAnaThd;
						g_FmMonoScalarResults["audioAnaThdPlusN"] = (20*log10(resultFmMono->audioAnaThdPlusN));
						g_FmMonoScalarResults["audioAnaTnhd"] = resultFmMono->audioAnaTnhd;

						g_FmMonoScalarResults["audioSpectrumWaveLength"] = resultFmMono->audioSpectrumWaveLength;
						g_FmMonoScalarResults["audioWaveLength"] = resultFmMono->audioWaveLength;
						g_FmMonoScalarResults["audioSpectrumWaveToneNSigFreq"] = resultFmMono->audioSpectrumWaveToneNSigFreq;

					}
					else
					{

					}
					if(g_FmMonoScalarResults.find(measurement) == g_FmMonoScalarResults.end())
					{
						return NA_NUMBER;
					}
					else
					{
						return((double)g_FmMonoScalarResults.find(measurement)->second);
					}

				}

			}
		}
	}
	else
	{
		return NA_NUMBER;
	}

	return NA_NUMBER;
}
IQMEASURE_API double LP_FM_GetScalarMeasurement(char *measurement, int index)
{
	::TIMER_StartTimer(timerIQmeasure, "LP_FM_GetScalarMeasurement", &timeStart);

	double value = LP_FM_GetScalarMeasurement_NoTimer(measurement, index);

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_GetScalarMeasurement", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_GetScalarMeasurement", timeDuration);

	return value;
}

IQMEASURE_API int LP_FM_GetVectorMeasurement_NoTimer(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{

	if (FmInitialized)
	{
		if (!hndl->hndlFm->results || !bufferReal || !bufferLength)
		{
			return 0;
		}
		if(hndl->hndlFm->results)
		{
			if (dynamic_cast<iqapiResultFmRf *>(hndl->hndlFm->results))
			{
				if(g_FmRfVectorResults.find(measurement) == g_FmRfVectorResults.end())
				{
					resultFmRf = dynamic_cast<iqapiResultFmRf *>(hndl->hndlFm->results);

					g_FmRfVectorResults["spectrumWaveX"].assign(resultFmRf->spectrumWaveX, (resultFmRf->spectrumWaveX + MAX_OUTPUT_WAVE_LENGTH));
					g_FmRfVectorResults["spectrumWaveY"].assign(resultFmRf->spectrumWaveY, (resultFmRf->spectrumWaveY + MAX_OUTPUT_WAVE_LENGTH));

				}
				else
				{
					// Do Nothing
				}
				if(g_FmRfVectorResults.find(measurement) == g_FmRfVectorResults.end())
				{
					return (int)NA_NUMBER;
				}
				else
				{
					for(int i = 0; i < bufferLength; i++)
					{
						bufferReal[i] = g_FmRfVectorResults[measurement].at(i);
					}
					return bufferLength;
				}
			}
			else if (dynamic_cast<iqapiResultAudioStereo *>(hndl->hndlFm->results))
			{
				if(g_FmStereoVectorResults.find(measurement) == g_FmStereoVectorResults.end())
				{
					resultAudioStereo = dynamic_cast<iqapiResultAudioStereo *>(hndl->hndlFm->results);

					g_FmStereoVectorResults["crosstalkChannelIndex"].assign(resultAudioStereo->crosstalkChannelIndex, (resultAudioStereo->crosstalkChannelIndex + 50));
					g_FmStereoVectorResults["crosstalkCrosstalk"].assign(resultAudioStereo->crosstalkCrosstalk, (resultAudioStereo->crosstalkCrosstalk + 50));
					g_FmStereoVectorResults["crosstalkSigFreq"].assign(resultAudioStereo->crosstalkSigFreq, (resultAudioStereo->crosstalkSigFreq + 50));

					g_FmStereoVectorResults["leftAudioSpectrumWaveToneSigFreq"].assign(resultAudioStereo->leftAudioSpectrumWaveToneSigFreq, (resultAudioStereo->leftAudioSpectrumWaveToneSigFreq + 50));
					g_FmStereoVectorResults["leftAudioSpectrumWaveToneSigPower"].assign(resultAudioStereo->leftAudioSpectrumWaveToneSigPower, (resultAudioStereo->leftAudioSpectrumWaveToneSigPower + 50));

					g_FmStereoVectorResults["leftAudioSpectrumWaveX"].assign(resultAudioStereo->leftAudioSpectrumWaveX, (resultAudioStereo->leftAudioSpectrumWaveX + MAX_OUTPUT_WAVE_LENGTH));
					g_FmStereoVectorResults["leftAudioSpectrumWaveY"].assign(resultAudioStereo->leftAudioSpectrumWaveY, (resultAudioStereo->leftAudioSpectrumWaveY + MAX_OUTPUT_WAVE_LENGTH));

					g_FmStereoVectorResults["rightAudioSpectrumWaveToneSigFreq"].assign(resultAudioStereo->rightAudioSpectrumWaveToneSigFreq, (resultAudioStereo->rightAudioSpectrumWaveToneSigFreq + 50));
					g_FmStereoVectorResults["rightAudioSpectrumWaveToneSigPower"].assign(resultAudioStereo->rightAudioSpectrumWaveToneSigPower, (resultAudioStereo->rightAudioSpectrumWaveToneSigPower + 50));

					g_FmStereoVectorResults["rightAudioSpectrumWaveX"].assign(resultAudioStereo->rightAudioSpectrumWaveX, (resultAudioStereo->rightAudioSpectrumWaveX + MAX_OUTPUT_WAVE_LENGTH));
					g_FmStereoVectorResults["rightAudioSpectrumWaveY"].assign(resultAudioStereo->rightAudioSpectrumWaveY, (resultAudioStereo->rightAudioSpectrumWaveY + MAX_OUTPUT_WAVE_LENGTH));

				}
				else
				{
					// Do Nothing
				}
				if(g_FmStereoVectorResults.find(measurement) == g_FmStereoVectorResults.end())
				{
					return (int)NA_NUMBER;
				}
				else
				{
					for(int i = 0; i < bufferLength; i++)
					{
						bufferReal[i] = g_FmStereoVectorResults[measurement].at(i);
					}
					return bufferLength;
				}
			}
			else if (dynamic_cast<iqapiResultAudioMono *>(hndl->hndlFm->results))
			{
				if(g_FmMonoVectorResults.find(measurement) == g_FmMonoVectorResults.end())
				{
					resultAudioMono = dynamic_cast<iqapiResultAudioMono *>(hndl->hndlFm->results);

					g_FmMonoVectorResults["audioSpectrumWaveToneSigFreq"].assign(resultAudioMono->audioSpectrumWaveToneSigFreq, (resultAudioMono->audioSpectrumWaveToneSigFreq + 50));
					g_FmMonoVectorResults["audioSpectrumWaveToneSigPower"].assign(resultAudioMono->audioSpectrumWaveToneSigPower, (resultAudioMono->audioSpectrumWaveToneSigPower + 50));

					g_FmMonoVectorResults["audioSpectrumWaveX"].assign(resultAudioMono->audioSpectrumWaveX, (resultAudioMono->audioSpectrumWaveX + MAX_OUTPUT_WAVE_LENGTH));
					g_FmMonoVectorResults["audioSpectrumWaveY"].assign(resultAudioMono->audioSpectrumWaveY, (resultAudioMono->audioSpectrumWaveY + MAX_OUTPUT_WAVE_LENGTH));
				}
				else
				{
					// Do Nothing
				}
				if(g_FmRfVectorResults.find(measurement) == g_FmRfVectorResults.end())
				{
					return (int)NA_NUMBER;
				}
				else
				{
					for(int i = 0; i < bufferLength; i++)
					{
						bufferReal[i] = g_FmRfVectorResults[measurement].at(i);
					}
					return bufferLength;
				}
			}
			else if (dynamic_cast<iqapiResultFmStereo *>(hndl->hndlFm->results))
			{
				if(g_FmStereoVectorResults.find(measurement) == g_FmStereoVectorResults.end())
				{
					resultFmStereo = dynamic_cast<iqapiResultFmStereo *>(hndl->hndlFm->results);

					g_FmStereoVectorResults["crosstalkChannelIndex"].assign(resultFmStereo->crosstalkChannelIndex, (resultFmStereo->crosstalkChannelIndex + 50));
					g_FmStereoVectorResults["crosstalkCrosstalk"].assign(resultFmStereo->crosstalkCrosstalk, (resultFmStereo->crosstalkCrosstalk + 50));
					g_FmStereoVectorResults["crosstalkSigFreq"].assign(resultFmStereo->crosstalkSigFreq, (resultFmStereo->crosstalkSigFreq + 50));

					g_FmStereoVectorResults["leftAudioSpectrumWaveToneSigFreq"].assign(resultFmStereo->leftAudioSpectrumWaveToneSigFreq, (resultFmStereo->leftAudioSpectrumWaveToneSigFreq + 50));
					g_FmStereoVectorResults["leftAudioSpectrumWaveToneSigPower"].assign(resultFmStereo->leftAudioSpectrumWaveToneSigPower, (resultFmStereo->leftAudioSpectrumWaveToneSigPower + 50));

					g_FmStereoVectorResults["leftAudioSpectrumWaveX"].assign(resultFmStereo->leftAudioSpectrumWaveX, (resultFmStereo->leftAudioSpectrumWaveX + MAX_OUTPUT_WAVE_LENGTH));
					g_FmStereoVectorResults["leftAudioSpectrumWaveY"].assign(resultFmStereo->leftAudioSpectrumWaveY, (resultFmStereo->leftAudioSpectrumWaveY + MAX_OUTPUT_WAVE_LENGTH));

					g_FmStereoVectorResults["leftAudioWaveX"].assign(resultFmStereo->leftAudioWaveX, (resultFmStereo->leftAudioWaveX + MAX_OUTPUT_WAVE_LENGTH));
					g_FmStereoVectorResults["leftAudioWaveY"].assign(resultFmStereo->leftAudioWaveY, (resultFmStereo->leftAudioWaveY + MAX_OUTPUT_WAVE_LENGTH));

					g_FmStereoVectorResults["rightAudioSpectrumWaveToneSigFreq"].assign(resultFmStereo->rightAudioSpectrumWaveToneSigFreq, (resultFmStereo->rightAudioSpectrumWaveToneSigFreq + 50));
					g_FmStereoVectorResults["rightAudioSpectrumWaveToneSigPower"].assign(resultFmStereo->rightAudioSpectrumWaveToneSigPower, (resultFmStereo->rightAudioSpectrumWaveToneSigPower + 50));

					g_FmStereoVectorResults["rightAudioSpectrumWaveToneSigPower"].assign(resultFmStereo->rightAudioSpectrumWaveX, (resultFmStereo->rightAudioSpectrumWaveX + MAX_OUTPUT_WAVE_LENGTH));
					g_FmStereoVectorResults["rightAudioSpectrumWaveToneSigPower"].assign(resultFmStereo->rightAudioSpectrumWaveY, (resultFmStereo->rightAudioSpectrumWaveY + MAX_OUTPUT_WAVE_LENGTH));

					g_FmStereoVectorResults["rightAudioWaveX"].assign(resultFmStereo->rightAudioWaveX, (resultFmStereo->rightAudioWaveX + MAX_OUTPUT_WAVE_LENGTH));
					g_FmStereoVectorResults["rightAudioWaveY"].assign(resultFmStereo->rightAudioWaveY, (resultFmStereo->rightAudioWaveY + MAX_OUTPUT_WAVE_LENGTH));

				}
				else
				{
					// Do Nothing
				}
				if(g_FmStereoVectorResults.find(measurement) == g_FmStereoVectorResults.end())
				{
					return (int)NA_NUMBER;
				}
				else
				{
					for(int i = 0; i < bufferLength; i++)
					{
						bufferReal[i] = g_FmStereoVectorResults[measurement].at(i);
					}
					return bufferLength;
				}
			}
			else if (dynamic_cast<iqapiResultFmMono *>(hndl->hndlFm->results))
			{
				if(g_FmMonoVectorResults.find(measurement) == g_FmMonoVectorResults.end())
				{
					resultFmMono = dynamic_cast<iqapiResultFmMono *>(hndl->hndlFm->results);

					g_FmMonoVectorResults["audioSpectrumWaveToneSigFreq"].assign(resultFmMono->audioSpectrumWaveToneSigFreq, (resultFmMono->audioSpectrumWaveToneSigFreq + 50));
					g_FmMonoVectorResults["audioSpectrumWaveToneSigPower"].assign(resultFmMono->audioSpectrumWaveToneSigPower, (resultFmMono->audioSpectrumWaveToneSigPower + 50));

					g_FmMonoVectorResults["audioSpectrumWaveX"].assign(resultFmMono->audioSpectrumWaveX, (resultFmMono->audioSpectrumWaveX + MAX_OUTPUT_WAVE_LENGTH));
					g_FmMonoVectorResults["audioSpectrumWaveY"].assign(resultFmMono->audioSpectrumWaveY, (resultFmMono->audioSpectrumWaveY + MAX_OUTPUT_WAVE_LENGTH));

					g_FmMonoVectorResults["audioWaveX"].assign(resultFmMono->audioWaveX, (resultFmMono->audioWaveX + MAX_OUTPUT_WAVE_LENGTH));
					g_FmMonoVectorResults["audioWaveY"].assign(resultFmMono->audioWaveY, (resultFmMono->audioWaveY + MAX_OUTPUT_WAVE_LENGTH));

				}
				else
				{
					// Do Nothing
				}
				if(g_FmRfVectorResults.find(measurement) == g_FmRfVectorResults.end())
				{
					return (int)NA_NUMBER;
				}
				else
				{
					for(int i = 0; i < bufferLength; i++)
					{
						bufferReal[i] = g_FmRfVectorResults[measurement].at(i);
					}
					return bufferLength;
				}
			}

		}
		else
		{
			return (int)NA_NUMBER;
		}
	}
	else
	{
		return (int)NA_NUMBER;
	}

	return (int)NA_NUMBER;
}

IQMEASURE_API int LP_FM_GetVectorMeasurement(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	::TIMER_StartTimer(timerIQmeasure, "LP_FM_GetVectorMeasurement", &timeStart);

	int ret = LP_FM_GetVectorMeasurement_NoTimer(measurement, bufferReal, bufferImag, bufferLength);

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_GetVectorMeasurement", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_GetVectorMeasurement", timeDuration);

	return ret;
}


IQMEASURE_API int LP_FM_AudioDataCapture(char* saveFileLocation,
		double captureDurationMilliSec,
		int samplingRate,
		int stereo)
{
	int err = ERR_OK;

	//Clear previous results stored
	g_FmRfScalarResults.clear();
	g_FmMonoScalarResults.clear();
	g_FmStereoScalarResults.clear();

	g_FmRfVectorResults.clear();
	g_FmMonoVectorResults.clear();
	g_FmStereoVectorResults.clear();

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_AudioDataCapture", &timeStart);

	int aimErr = ERR_OK;
	// Calling AIM Control function from AimControl.h
	aimErr = AIM_CaptureAndSaveAudio(samplingRate, captureDurationMilliSec, saveFileLocation, stereo);

	if(ERR_AIM_CAPTURE_AUDIO_FAILED == aimErr)
	{
		err = ERR_FM_CAPTURE_AUDIO_FAILED;
	}
	else if(ERR_AIM_SAVE_WAV_AUDIO_FAILED == aimErr)
	{
		err = ERR_FM_SAVE_WAV_AUDIO_FAILED;
	}
	::TIMER_StopTimer(timerIQmeasure, "LP_FM_AudioDataCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_AudioDataCapture", timeDuration);

	return 0;
}

IQMEASURE_API int LP_FM_LoadAudioCapture(char* fileName)
{
	//	const int   bufSize = 1024;
	//	int         numberOfChannel = 0;
	//	int         indexChannel = 0;
	//	char		temp[100]={};

	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_LoadAudioCapture", &timeStart);
	// need to add check for wav file validity - need to check for wave headers for valid format
	if(FmInitialized)
	{
		err = hndl->hndlFm->LoadSignalFileWav(fileName);
		// set g_audioCaptureOnly flag to indicate that this is an audio capture
		g_audioCaptureOnly = true;
	}
	else
	{
		err = ERR_FM_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_LoadAudioCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_LoadAudioCapture", timeDuration);

	return err;
}

// LP_FM_AudioStimulusGenerate function

IQMEASURE_API int LP_FM_AudioStimulusGenerateAndPlayMultiTone(double sampleRateHz,
		double deviationPercent,
		double peakVoltageLevelVolts,
		int toneCount,
		int stereoEnable,
		int* leftRightRelation,
		double* freqHz,
		double durationMilliSeconds,
		char* audioWaveFileOutput)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_AudioStimulusGenerateMultiTone", &timeStart);

	double peakVoltage = (deviationPercent * peakVoltageLevelVolts / 100);

	int aimErr = 0;

	// Calling AIM Control function from AimControl.h
	aimErr = AIM_GenerateAndPlayAudioMultiTone(sampleRateHz, peakVoltage, toneCount,
			stereoEnable, leftRightRelation, freqHz,
			durationMilliSeconds, audioWaveFileOutput);

	if(ERR_AIM_GENERATE_AUDIO_FAILED == aimErr)
	{
		err = ERR_FM_GENERATE_AUDIO_FAILED;
	}
	else if(ERR_AIM_SAVE_WAV_AUDIO_FAILED == aimErr)
	{
		err = ERR_FM_SAVE_WAV_AUDIO_FAILED;
	}
	else
	{
		err = ERR_FM_PLAY_AUDIO_FAILED;
	}
	::TIMER_StopTimer(timerIQmeasure, "LP_FM_AudioStimulusGenerateMultiTone", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_AudioStimulusGenerateMultiTone", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_AudioStimulusGenerateAndPlaySingleTone(double sampleRateHz,
		double deviationPercent,
		double peakVoltageLevelVolts,
		int stereoEnable,
		int leftRightRelation,
		double freqHz,
		double durationMilliSeconds,
		char* audioWaveFileOutput)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_AudioStimulusGenerateSingleTone", &timeStart);

	double peakVoltage = (deviationPercent * peakVoltageLevelVolts / 100);

	int aimErr = 0;

	// Calling AIM Control function from AimControl.h
	aimErr = AIM_GenerateAndPlayAudioSingleTone(sampleRateHz, peakVoltage,
			stereoEnable, leftRightRelation, freqHz,
			durationMilliSeconds, audioWaveFileOutput);

	if(ERR_AIM_GENERATE_AUDIO_FAILED == aimErr)
	{
		err = ERR_FM_GENERATE_AUDIO_FAILED;
	}
	else if(ERR_AIM_SAVE_WAV_AUDIO_FAILED == aimErr)
	{
		err = ERR_FM_SAVE_WAV_AUDIO_FAILED;
	}
	else if(ERR_AIM_PLAY_AUDIO_FAILED == aimErr)
	{
		err = ERR_FM_PLAY_AUDIO_FAILED;
	}
	else
	{
		err = aimErr;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_FM_AudioStimulusGenerateSingleTone", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_AudioStimulusGenerateSingleTone", timeDuration);

	return err;
}

IQMEASURE_API int LP_FM_StopAudioPlay()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_StopAudioPlay", &timeStart);

	int aimStopAudioErr = 0;

	// Calling AIM Control function from AimControl.h
	aimStopAudioErr = AIM_StopPlayAudio();

	if(ERR_AIM_STOP_AUDIO_PLAY_FAILED == aimStopAudioErr)
	{
		err = ERR_FM_STOP_AUDIO_PLAY_FAILED;
	}


	::TIMER_StopTimer(timerIQmeasure, "LP_FM_StopAudioPlay", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FM_StopAudioPlay", timeDuration);

	return err;
}

#endif //WIN32  //Not using FM for Mac at this time

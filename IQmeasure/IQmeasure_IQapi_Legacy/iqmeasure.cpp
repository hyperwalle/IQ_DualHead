// IQmeasure Library in IQlite

#include "stdafx.h"
//Move to stdafx.h
//#include "lp_stdio.h"
#include "iqapi.h"
#include "..\IQmeasure.h"
//#include "..\DebugAlloc.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"
#include "StringUtil.h"
#include "IQ2010Ext.h"
#ifdef WIN32
#include "AimControl.h"
#endif
//Move to stdafx.h
//#include "lp_string.h"
#include <float.h> // DBL_MAX on Mac
#include "..\..\Include\version.h"

using namespace UsedInMTK;

/*
#pragma comment(lib, "iqapi.lib")
#pragma comment(lib, "IQlite_Timer.lib")
#pragma comment(lib, "IQlite_Logger.lib")
#pragma comment(lib, "Util.lib")
#pragma comment(lib, "IQ2010Ext.lib")
#pragma comment(lib, "AimControl.lib")
*/

#ifndef WIN32
inline double abs(double x)
{ return (fabs(x)); }
#endif

double calc_energy (double data_i, double data_q)
{
	return (data_i*data_i + data_q*data_q);
}

void createSpectrumMask();

int PortMask[] = {BM_PORT1, BM_PORT2, BM_PORT3, BM_PORT4, PORT_DONE};
double *rawSpectrumMask_11b = NULL;
double *rawSpectrumMask_Zigbee  = NULL;
double *rawSpectrumMask_BT  = NULL;
double *rawSpectrumMask_BT_EDR  = NULL;

SpectrumMaskSpecification SpecMask;
iqapiAnalysisObw		*analysisObw = NULL;
iqapiResultObw			*resultObw = NULL;
int numSections;



iqapiHndl				*hndl = NULL;
int                     timerIQmeasure  = -1;
int						loggerIQmeasure = -1;
//double					timeStart, timeStop;
highrestimer::lp_time_t timeStart, timeStop; // -cfy@sunnyvale, 2012/3/13-
double                  timeDuration;
double					g_testerHwVersion = 0;			// -cfy@sunnyvale, 2012/3/13-
bool					g_bDisableFreqFilter = true;	// -cfy@sunnyvale, 2012/3/13-

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
iqapiAnalysisNfc		*analysisNfc = NULL;
#if !defined(IQAPI_1_5_X)
// iqapiAnalysisHT40 is supported in IQapi 1.6.x and beyond
iqapiAnalysisHT40		*analysisHT40 = NULL;
#endif
iqapiAnalysisHT40WideBandHiAccuracy   *analysisHT40WideBandHiAccuracy = NULL;
iqapiAnalysisHT20WideBandLoAccuracy   *analysisHT20WideBandLoAccuracy = NULL;
iqapiAnalysisHT20WideBandHiAccuracy   *analysisHT20WideBandHiAccuracy = NULL;

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
iqapiResultNfc			*resultNfc = NULL;

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
map<string, vector <double>> g_FmRfVectorResults;
map<string, vector <double>> g_FmMonoVectorResults;
map<string, vector <double>> g_FmStereoVectorResults;

#if !defined(IQAPI_1_5_X)
// iqapiResultHT40 is supported in IQapi 1.6.x and beyond
iqapiResultHT40		    *resultHT40 = NULL;
#endif
iqapiResultHT40WideBandHiAccuracy    *resultHT40WideBandHiAccuracy = NULL;
iqapiResultHT20WideBandLoAccuracy    *resultHT20WideBandLoAccuracy = NULL;
iqapiResultHT20WideBandHiAccuracy    *resultHT20WideBandHiAccuracy = NULL;

iqapiCapture			*capture = NULL;

bool					g_audioCaptureOnly = false;
bool					LibsInitialized = false;
bool                    setDefaultCalled = false;
int						nTesters = 0;
bool                    bIQ201xFound = false;

// FM Initialization
bool					FmInitialized = false;



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

//Thie variable will record capture type, capture type is one of enum IQV_CAPTURE_TYPE_ENUM
//The purpose is to identify which kind of analyze will be used. Defaule: IQV_NORMAL_CAPTURE_TYPE
IQV_CAPTURE_TYPE_ENUM  g_captureType = IQV_NORMAL_CAPTURE_TYPE;

// The structure below is used internally, and for IQnxn only
struct _tagIQnxnSettings
{
	double	vsaAmplDBm;
	double	vsgPowerDBm;
	enum	IQV_PORT_ENUM vsaPort;
	enum	IQV_PORT_ENUM vsgPort;
	int		vsgRFEnabled;
} g_IQnxnSettings[N_MAX_TESTERS];


#ifndef MAX_BUFFER_LEN
#define MAX_BUFFER_LEN 4096
#endif

#ifndef BT_SHIFT_HZ
#define BT_SHIFT_HZ    7e6
#endif

#define ROUND(X)	(int)(X > 0 ? X + 0.5 : X - 0.5) // Modified /* -cfy@sunnyvale, 2012/3/13- */

double g_amplitudeToleranceDb = 3;

using namespace std;
map <string, int>	 dataRateIQ2010Ext;

// This global variable is needed for selecting partial capture for analysis
// LP_SelectCaptureRangeForAnalysis() will set this variable
// LP_VsaDataCapture() will reset this variable to NULL
// Note: g_userData does not allocate memory at all.  It points to a portion of hndl->data
iqapiCapture *g_userData = NULL;

// Internal iqapi function for setting Number of threads to use in Matlab
extern "C" int iqapiSetCompThreadNumber(iqapiHndl *hndl, int compThreadNumber, int *numberCompThreadSetLast); // -cfy@sunnyvale, 2012/3/13-



// Internal functions
// Functions declared below are not supposed to go in IQmeasure.h
void  InstantiateAnalysisClasses();
int   LP_Analyze(void);
void  InitializeTesterSettings(void);

//! Convert data rate string to IQ2010Ext Index
/*!
 * \param[in] name The name of Data Rate, such as "DSSS-1", "CCK-11", "OFDM-54", "MCS7" etc.
 * \param[out] dataRateIndex The corresponding index for the specified data rate
 *
 * \return ERR_OK (0) If the specified data rate index has been found
 */
int LP_ConvertDataRateStringToIQ2010ExtIndex( char *name, int *dataRateIndex );
int LP_InitializeIQ2010ExtMap(void);
// End of Internal functions

//#ifdef WIN32 //[TBD] /* -cfy@sunnyvale, 2012/3/13- */
//BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
//{
//    switch (ul_reason_for_call)
//    {
//		case DLL_PROCESS_ATTACH:
//			LP_StartIQmeasureTimer();
//			LP_InitializeIQ2010ExtMap();
//			break;
//		case DLL_THREAD_ATTACH:
//			break;
//		case DLL_THREAD_DETACH:
//			break;
//		case DLL_PROCESS_DETACH:
//			//debug_ext::debug_mem_allocation::SendOutCurrentLog();
//			LP_FreeMemory();
//			break;
//    }
//    return TRUE;
//}
//#else
namespace {
	struct IQMeasureInit
	{
		IQMeasureInit()
		{
			LP_StartIQmeasureTimer();
			LP_InitializeIQ2010ExtMap();
		}
		~IQMeasureInit() { LP_FreeMemory(); }
	} measInit;
}
//#endif


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
		case ERR_INVALID_DATA_CAPTURE_RANGE:        return "Capture range selection is beyond valid range";

							    // Added some more error codes (in enum IQMEASURE_ERROR_CODES in IQMeasure.h to the function) - seshadri

		case ERR_INVALID_IP_ADDRESS:				return "Invalid IP Address";
		case ERR_TX_NOT_DONE:						return "TX Transmit is not complete yet";
		case ERR_INVALID_CAPTURE_INDEX:				return "The specified capture index is Invalid";
		case ERR_VSG_POWER_EXCEED_LIMIT:			return "Exceeded the VSG Power Limit";
		case ERR_DATARATE_DOES_NOT_EXIST:			return "Invalid Data Rate specified";
		case ERR_BUFFER_OVERFLOW:					return "Buffer Overflow";
		case ERR_SET_PATH_NOT_DONE:					return "Failed to set path of connectivity server";

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
		case ERR_FM_SAVE_WAV_AUDIO_FAILED:		return "Failed to Save Audio";
		case ERR_FM_PLAY_AUDIO_FAILED:			return "Failed to Play Audio using AIM Device";
		case ERR_FM_LOAD_AUDIO_FAILED:			return "Failed to Load Audio";
		case ERR_FM_STOP_AUDIO_PLAY_FAILED:	return "Failed to stop Audio Play";

		case ERR_MPTA_NOT_ENABLE:               return "MPTA enable failed";
		case ERR_MPTA_NOT_DISENABLE:            return "MPTA disable failed";
		case ERR_MPTA_SET_TXRX:                 return "MPTA set TxRx failed";
		case ERR_MPTA_CAPTURE_FAILED:           return "MPTA data capture failed";

		default:									return "Unknown error condition.";
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_GetErrorString", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_GetErrorString", timeDuration);
}

int LP_FreeMemory(void)
{
	int err = 0;

	::TIMER_StartTimer(timerIQmeasure, "LP_FreeMemory", &timeStart);

	if (hndl)	{ hndl->analysis = NULL; }

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

	if(analysisHT40WideBandHiAccuracy) {delete analysisHT40WideBandHiAccuracy; analysisHT40WideBandHiAccuracy = NULL;}
	if(analysisHT20WideBandLoAccuracy) {delete analysisHT20WideBandLoAccuracy; analysisHT20WideBandLoAccuracy = NULL;}
	if(analysisHT20WideBandHiAccuracy) {delete analysisHT20WideBandHiAccuracy; analysisHT20WideBandHiAccuracy = NULL;}

	if (analysisFmRf)                {delete analysisFmRf;                analysisFmRf                = NULL;}
	if (analysisFmAudioBase)         {delete analysisFmAudioBase;         analysisFmAudioBase         = NULL;}
	if (analysisFmDemodSpectrum)     {delete analysisFmDemodSpectrum;     analysisFmDemodSpectrum     = NULL;}
	if (analysisFmMono)              {delete analysisFmMono;              analysisFmMono              = NULL;}
	if (analysisFmStereo)            {delete analysisFmStereo;            analysisFmStereo            = NULL;}
	if (analysisFmAuto)              {delete analysisFmAuto;              analysisFmAuto              = NULL;}
	if (analysisRds)                 {delete analysisRds;                 analysisRds                 = NULL;}
	if (analysisRdsMono)             {delete analysisRdsMono;             analysisRdsMono             = NULL;}
	if (analysisRdsStereo)           {delete analysisRdsStereo;           analysisRdsStereo           = NULL;}
	if (analysisRdsAuto)             {delete analysisRdsAuto;             analysisRdsAuto             = NULL;}
	if (analysisAudioStereo)         {delete analysisAudioStereo;         analysisAudioStereo         = NULL;}
	if (analysisAudioMono)           {delete analysisAudioMono;           analysisAudioMono           = NULL;}

	if( rawSpectrumMask_11b != NULL ) {delete [] rawSpectrumMask_11b; rawSpectrumMask_11b = NULL;}
	if( rawSpectrumMask_Zigbee != NULL) {delete [] rawSpectrumMask_Zigbee; rawSpectrumMask_Zigbee = NULL;}
	if( rawSpectrumMask_BT != NULL) { delete [] rawSpectrumMask_BT;  rawSpectrumMask_BT = NULL;}
	if( rawSpectrumMask_BT_EDR != NULL ) { delete [] rawSpectrumMask_BT_EDR; rawSpectrumMask_BT_EDR = NULL; }

	//if (hndl) { delete hndl; hndl = NULL; } - this line was causing FM to crash in IQFact+. Commenting it for now.

	::TIMER_StopTimer(timerIQmeasure, "LP_FreeMemory", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_FreeMemory", timeDuration);


	return err;
}

// -cfy@sunnyvale, 2012/3/13-
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
/* <><~~ */

int LP_Analyze(void)
{
	int err = ERR_OK;

	// g_userData takes priority
	if( g_userData!=NULL )
	{
		err = hndl->Analyze( g_userData );
		if( IQAPI_ERR_OK != err )
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
		if( IQAPI_ERR_OK != err )
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

// FM Analyze hndl
int LP_Analyze_FM_Hndl(void)
{
	int err = ERR_OK;
	if( hndl->hndlFm->dataFm != NULL )
	{
		err = hndl->hndlFm->Analyze();
		if( IQAPI_ERR_OK != err)
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



IQMEASURE_API char *LP_GetIQapiHndlLastErrMsg()
{
	return hndl->lastErr;
}


IQMEASURE_API int LP_Init(int IQtype,int testControlMethod)
{
	int err = ERR_OK;

	// -cfy@sunnyvale, 2012/3/13-
	FmInitialized = false;
	LibsInitialized = false;
	/* <><~~ */
	::TIMER_StartTimer(timerIQmeasure, "LP_Init", &timeStart);

	// -cfy@sunnyvale, 2012/3/13-
	err = iqapiInit((bool)FALSE);
	/* <><~~ */
	if (hndl) { delete hndl; hndl = NULL; }
	hndl = new iqapiHndl();

	if(err == IQAPI_ERR_OK)
	{
		createSpectrumMask();
		LibsInitialized = true;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_Init", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_Init", timeDuration);

	return err;
}

IQMEASURE_API bool LP_GetVersion(char *buffer, int buf_size)
{
	// -cfy@sunnyvale, 2012/3/13-
	bool err = true;
	char ver[MAX_BUFFER_LEN],  specifiedVer[MAX_BUFFER_LEN], str[MAX_BUFFER_LEN] = "IQmeasure: ";
	string tmpStr1 = "", tmpStr2 = "";
	//char strDelimit[]   = "\r\t\n";
	//char *token = NULL, *nextToken = NULL;

	::TIMER_StartTimer(timerIQmeasure, "LP_GetVersion", &timeStart);

	// Append the IQmeasure version
	sprintf_s(str, MAX_BUFFER_LEN, "IQmeasure: %s %s\n", IQMEASURE_VERSION, SVN_REVISION); // Modified /* -cfy@sunnyvale, 2012/3/13- */

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
		sprintf_s(specifiedVer, "Tester%d FW:\t\t%s", testerNumber, ver);
		strcat_s(str, MAX_BUFFER_LEN, specifiedVer);
		strcpy_s(ver, MAX_BUFFER_LEN, "");

		hndl->GetFpgaVersion(&ver[0], MAX_BUFFER_LEN, IQV_FPGA_VSA, testerNumber);
		sprintf_s(specifiedVer, "\nTester%d VSAFPGA:\t%s", testerNumber, ver);
		strcat_s(str, MAX_BUFFER_LEN, specifiedVer);
		strcpy_s(ver, MAX_BUFFER_LEN, "");

		hndl->GetFpgaVersion(&ver[0], MAX_BUFFER_LEN, IQV_FPGA_VSG, testerNumber);
		sprintf_s(specifiedVer, "\nTester%d VSGFPGA:\t%s\n", testerNumber, ver);
		strcat_s(str, MAX_BUFFER_LEN, specifiedVer);
		strcpy_s(ver, MAX_BUFFER_LEN, "");
	}
	/* <><~~ */

	//For IQ201X Tester, bIQ201xFound = true , in order to set trigger type = 13.
	//IQ201X Serial Number: IQP-----
	tmpStr1 = str;
	if(string::npos != tmpStr1.find("IQP"))
	{
		bIQ201xFound = true;

		// If IQ2010 has been connected and IQ2010Ext has been initialized,
		// append IQ2010Ext version
		if (ERR_OK == IQ2010EXT_GetVersion(ver, MAX_BUFFER_LEN) )
		{
			strcat_s(str, MAX_BUFFER_LEN, ver);
		}
		else
		{
			// do nothing
		}
	}
	else
	{
		bIQ201xFound = false;
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
			// do nothing
		}

		hndl->analysis = NULL;
		if (hndl->ConValid())
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

IQMEASURE_API int LP_GetIQapiHndl(void **getHndl, bool *testerInitialized)
{
	*getHndl = (void *)hndl;
	*testerInitialized = LibsInitialized;
	return ERR_OK;
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
			err = hndl->ConInit(ipAddress);
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
			if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 0 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_InitTester", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable );
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently disabled!\n", "LP_InitTester", "fpgaLoadCalTable" );
				hndl->tx->vsg[0]->fpgaLoadCalTable = 1; 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_InitTester", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable );
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently enabled!\n", "LP_InitTester", "fpgaLoadCalTable" );
				hndl->SetTxRx();
				if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 2 )
				{
					::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_InitTester", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable );
					::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s!\n", "LP_InitTester", "Cal Table was successfully downloaded" );
				}
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
		//2012-02-07: when connecting to IQ2010, ipAddress has one of the two formats:
		//            1. 127.0.0.1 - connecting to the first one, or the only one IQ2010
		//                           using port 4000
		//            2. 127.0.0.1:<port_number>:<serial_number> - connecting to the IQ2010
		//                                       with specified serial_number and specified port_number
		string			serialNumber1 = "";
		string			ipAddressAndPort1 = "";
		vector<string>	splits1;

		string			serialNumber2 = "";
		string			ipAddressAndPort2 = "";
		vector<string>	splits2;

		splits1.clear();
		SplitString(ipAddress1, splits1, ":");

		splits2.clear();
		SplitString(ipAddress2, splits2, ":");



		if( splits1.size()==3 && splits2.size()==3)
		{
			// if both Serial# and Port# are specified in addition to the ip address
			serialNumber1 = Trim( splits1[2] );
			serialNumber2 = Trim( splits2[2] );

			ipAddressAndPort1 = Trim( splits1[0] ) + ":" + Trim( splits1[1] );
			ipAddressAndPort2 = Trim( splits2[0] ) + ":" + Trim( splits2[1] );

			err = hndl->ConInitBySerialNumber((char*)ipAddressAndPort1.c_str(), (char*)serialNumber1.c_str(), (char*)serialNumber2.c_str());
		}
		else if('I' == ipAddress1[0] && 'Q' == ipAddress1[1] &&  'P' == ipAddress1[2] &&
				'I' == ipAddress2[0] && 'Q' == ipAddress2[1] &&  'P' == ipAddress2[2])
		{
			// user has specified only Serial Number(works only for IQ2010) of the tester and it is found to be an IQ2010
			err = err = hndl->ConInitBySerialNumber("127.0.0.1", ipAddress1, ipAddress2);
		}
		else
		{
			err = hndl->ConInit(ipAddress1, ipAddress2 );
		}

		if ( 0 < err )
		{
			err = ERR_NO_CONNECTION;
		}
		else
		{
			nTesters = 2;
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
		//2012-02-07: when connecting to IQ2010, ipAddress has one of the two formats:
		//            1. 127.0.0.1 - connecting to the first one, or the only one IQ2010
		//                           using port 4000
		//            2. 127.0.0.1:<port_number>:<serial_number> - connecting to the IQ2010
		//                                       with specified serial_number and specified port_number
		string			serialNumber1 = "";
		string			ipAddressAndPort1 = "";
		vector<string>	splits1;

		string			serialNumber2 = "";
		string			ipAddressAndPort2 = "";
		vector<string>	splits2;

		string			serialNumber3 = "";
		string			ipAddressAndPort3 = "";
		vector<string>	splits3;

		splits1.clear();
		SplitString(ipAddress1, splits1, ":");

		splits2.clear();
		SplitString(ipAddress2, splits2, ":");

		splits3.clear();
		SplitString(ipAddress3, splits3, ":");

		if( splits1.size()==3 && splits2.size()==3 && splits3.size()==3)
		{
			// both Serial# and Port# are specified
			serialNumber1 = Trim( splits1[2] );
			serialNumber2 = Trim( splits2[2] );
			serialNumber3 = Trim( splits3[2] );

			ipAddressAndPort1 = Trim( splits1[0] ) + ":" + Trim( splits1[1] );
			ipAddressAndPort2 = Trim( splits2[0] ) + ":" + Trim( splits2[1] );
			ipAddressAndPort3 = Trim( splits3[0] ) + ":" + Trim( splits3[1] );

			err = hndl->ConInitBySerialNumber((char*)ipAddressAndPort1.c_str(), (char*)serialNumber1.c_str(), (char*)serialNumber2.c_str(), (char*)serialNumber3.c_str());
		}
		else if('I' == ipAddress1[0] && 'Q' == ipAddress1[1] &&  'P' == ipAddress1[2] &&
				'I' == ipAddress2[0] && 'Q' == ipAddress2[1] &&  'P' == ipAddress2[2] &&
				'I' == ipAddress3[0] && 'Q' == ipAddress3[1] &&  'P' == ipAddress3[2])
		{
			// user has specified only Serial Number(works only for IQ2010) of the tester and it is found to be an IQ2010
			err = err = hndl->ConInitBySerialNumber("127.0.0.1", ipAddress1, ipAddress2, ipAddress3);
		}
		else
		{
			err = hndl->ConInit(ipAddress1, ipAddress2, ipAddress3 );
		}

		if ( 0 < err )
		{
			err = ERR_NO_CONNECTION;
		}
		else
		{
			nTesters = 3;
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
		//2012-02-07: when connecting to IQ2010, ipAddress has one of the two formats:
		//            1. 127.0.0.1 - connecting to the first one, or the only one IQ2010
		//                           using port 4000
		//            2. 127.0.0.1:<port_number>:<serial_number> - connecting to the IQ2010
		//                                       with specified serial_number and specified port_number
		string			serialNumber1 = "";
		string			ipAddressAndPort1 = "";
		vector<string>	splits1;

		string			serialNumber2 = "";
		string			ipAddressAndPort2 = "";
		vector<string>	splits2;

		string			serialNumber3 = "";
		string			ipAddressAndPort3 = "";
		vector<string>	splits3;

		string			serialNumber4 = "";
		string			ipAddressAndPort4 = "";
		vector<string>	splits4;

		splits1.clear();
		SplitString(ipAddress1, splits1, ":");

		splits2.clear();
		SplitString(ipAddress2, splits2, ":");

		splits3.clear();
		SplitString(ipAddress3, splits3, ":");

		splits4.clear();
		SplitString(ipAddress4, splits4, ":");

		if( splits1.size()==3 && splits2.size()==3 && splits3.size()==3 && splits4.size()==3)
		{
			// both Serial# and Port# are specified
			serialNumber1 = Trim( splits1[2] );
			serialNumber2 = Trim( splits2[2] );
			serialNumber3 = Trim( splits3[2] );
			serialNumber4 = Trim( splits4[2] );

			ipAddressAndPort1 = Trim( splits1[0] ) + ":" + Trim( splits1[1] );
			ipAddressAndPort2 = Trim( splits2[0] ) + ":" + Trim( splits2[1] );
			ipAddressAndPort3 = Trim( splits3[0] ) + ":" + Trim( splits3[1] );
			ipAddressAndPort4 = Trim( splits4[0] ) + ":" + Trim( splits4[1] );

			err = hndl->ConInitBySerialNumber((char*)ipAddressAndPort1.c_str(), (char*)serialNumber1.c_str(), (char*)serialNumber2.c_str(), (char*)serialNumber3.c_str(), (char*)serialNumber4.c_str());
		}
		else if('I' == ipAddress1[0] && 'Q' == ipAddress1[1] &&  'P' == ipAddress1[2] &&
				'I' == ipAddress2[0] && 'Q' == ipAddress2[1] &&  'P' == ipAddress2[2] &&
				'I' == ipAddress3[0] && 'Q' == ipAddress3[1] &&  'P' == ipAddress3[2] &&
				'I' == ipAddress4[0] && 'Q' == ipAddress4[1] &&  'P' == ipAddress4[2])
		{
			// user has specified only Serial Number(works only for IQ2010) of the tester and it is found to be an IQ2010
			err = err = hndl->ConInitBySerialNumber("127.0.0.1", ipAddress1, ipAddress2, ipAddress3, ipAddress4);
		}
		else
		{
			err = hndl->ConInit(ipAddress1, ipAddress2, ipAddress3, ipAddress4 );
		}

		if ( 0 < err )
		{
			err = ERR_NO_CONNECTION;
		}
		else
		{
			nTesters = 4;
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

IQMEASURE_API int LP_Agc(double *rfAmplDb, bool allTesters) // -cfy@sunnyvale, 2012/3/13-
{
	int err = ERR_OK;
	//bool setupChanged = false;

	::TIMER_StartTimer(timerIQmeasure, "LP_Agc", &timeStart);

	if (LibsInitialized)
	{
		hndl->Agc(allTesters); // -cfy@sunnyvale, 2012/3/13-
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
		err = hndl->FrameTx(frameCnt);
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
		//InitializeTesterSettings();
		hndl->rx->powerMode = IQV_VSA_TYPE_1;	// Change VSA to peak power mode, fixed the ACP difference when RELOAD_MULTI_WAVEFORM_FILE is set
		//hndl->rx->powerMode = IQV_VSA_TYPE_1; // MASK correlation issue
		if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 0 )
		{
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetDefault", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable );
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently disabled!\n", "LP_SetDefault", "fpgaLoadCalTable" );
			hndl->tx->vsg[0]->fpgaLoadCalTable = 1;	
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetDefault", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable );
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently enabled!\n", "LP_SetDefault", "fpgaLoadCalTable" );
		}
		
		hndl->SetTxRx();
		if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 2 )
		{
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetDefault", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable );
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s!\n", "LP_SetDefault", "Cal Table was successfully downloaded" );
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SetDefault", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetDefault", timeDuration);

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
 *   - LP_AnalyzeNfc();
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
			//NFC
			else if (dynamic_cast<iqapiResultNfc *>(hndl->results))
			{
				resultNfc = dynamic_cast<iqapiResultNfc *>(hndl->results);
				if (!strcmp(measurement, "errTxt"))
				{
					if (resultNfc->errTxt)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->errTxt);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->errTxt, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "waveSaX_string"))
				{
					if ( resultNfc->rfAnalysis )
					{
						if (resultNfc->waveSaX_string)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->waveSaX_string);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->waveSaX_string, numberOfCharToCopy);
							return(numberOfCharToCopy);						}
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
				else if (!strcmp(measurement, "waveSaY_string"))
				{
					if ( resultNfc->rfAnalysis )
					{
						if (resultNfc->waveSaY_string)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->waveSaY_string);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->waveSaY_string, numberOfCharToCopy);
							return(numberOfCharToCopy);
						}
						else
						{
							return 0;
						}
					}
					else
					{
						return (int)NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveX_string"))
				{
					if ( resultNfc->rfAnalysis )
					{
						if (resultNfc->waveX_string)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->waveX_string);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->waveX_string, numberOfCharToCopy);
							return(numberOfCharToCopy);
						}
						else
						{
							return 0;
						}
					}
					else
					{
						return (int)NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveY_string"))
				{
					if ( resultNfc->rfAnalysis )
					{
						if (resultNfc->waveY_string)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->waveY_string);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->waveY_string, numberOfCharToCopy);
							return(numberOfCharToCopy);
						}
						else
						{
							return 0;
						}
					}
					else
					{

						return (int)NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaX_string"))
				{
					if (resultNfc->errTxt)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->errTxt);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->errTxt, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaY_string"))
				{
					if (resultNfc->capSigInfoSaY_string)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->capSigInfoSaY_string);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->capSigInfoSaY_string, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "capSigInfoWaveX_string"))
				{
					if (resultNfc->capSigInfoWaveX_string)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->capSigInfoWaveX_string);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->capSigInfoWaveX_string, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "capSigInfoWaveY_string"))
				{
					if (resultNfc->capSigInfoWaveY_string)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->capSigInfoWaveY_string);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->capSigInfoWaveY_string, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "waveInfoModType"))
				{
					if (resultNfc->waveInfoModType)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->waveInfoModType);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->waveInfoModType, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "waveInfoIsoType"))
				{
					if (resultNfc->waveInfoIsoType)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->waveInfoIsoType);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->waveInfoIsoType, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "frameInfoStartByte"))
				{
					if ( resultNfc->waveInfoBandwidthKHz > 106 )
					{
						if (resultNfc->frameInfoStartByte)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->frameInfoStartByte);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoStartByte, numberOfCharToCopy);
							return(numberOfCharToCopy);
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
				else if (!strcmp(measurement, "frameInfoPreamble"))
				{
					if ( resultNfc->waveInfoBandwidthKHz > 106 )
					{
						if (resultNfc->frameInfoPreamble)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->frameInfoPreamble);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoPreamble, numberOfCharToCopy);
							return(numberOfCharToCopy);
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
				else if (!strcmp(measurement, "frameInfoLen"))
				{
					if (resultNfc->frameInfoLen)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->frameInfoLen);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoLen, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "frameInfoPayload"))
				{
					if (resultNfc->frameInfoPayload)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->frameInfoPayload);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoPayload, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "frameInfoCrc"))
				{
					if (resultNfc->frameInfoCrc)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->frameInfoCrc);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoCrc, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "frameInfoSender"))
				{
					if (resultNfc->frameInfoSender)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->frameInfoSender);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoSender, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "frameInfoPayloadContentMsgType"))
				{
					if ( resultNfc->frameInfoDecodeTransportMsg )
					{
						if (resultNfc->frameInfoPayloadContentMsgType)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->frameInfoPayloadContentMsgType);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoPayloadContentMsgType, numberOfCharToCopy);
							return(numberOfCharToCopy);
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
				else if (!strcmp(measurement, "frameInfoPayloadContentNfcid"))
				{
					if ( resultNfc->frameInfoDecodeTransportMsg )
					{
						if (resultNfc->frameInfoPayloadContentNfcid)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->frameInfoPayloadContentNfcid);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoPayloadContentNfcid, numberOfCharToCopy);
							return(numberOfCharToCopy);
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
				else if (!strcmp(measurement, "frameInfoPayloadContentDid"))
				{
					if ( resultNfc->frameInfoDecodeTransportMsg )
					{
						if (resultNfc->frameInfoPayloadContentDid)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->frameInfoPayloadContentDid);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoPayloadContentDid, numberOfCharToCopy);
							return(numberOfCharToCopy);
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
				else if (!strcmp(measurement, "frameInfoPayloadContentGeneralBytesAscii"))
				{
					if ( resultNfc->frameInfoDecodeTransportMsg )
					{
						if (resultNfc->frameInfoPayloadContentGeneralBytesAscii)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->frameInfoPayloadContentGeneralBytesAscii);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoPayloadContentGeneralBytesAscii, numberOfCharToCopy);
							return(numberOfCharToCopy);
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
				else if (!strcmp(measurement, "frameInfoPayloadContentPfb"))
				{
					if ( resultNfc->frameInfoDecodeTransportMsg )
					{
						if (resultNfc->frameInfoPayloadContentPfb)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->frameInfoPayloadContentPfb);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoPayloadContentPfb, numberOfCharToCopy);
							return(numberOfCharToCopy);
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
				else if (!strcmp(measurement, "frameInfoPayloadContentPduType"))
				{
					if ( resultNfc->frameInfoDecodeTransportMsg )
					{
						if (resultNfc->frameInfoPayloadContentPduType)
						{
							int numberOfCharToCopy = 0;
							int tempLen = (int)strlen(resultNfc->frameInfoPayloadContentPduType);
							if ( bufferLength > tempLen )
								numberOfCharToCopy = tempLen;
							else
								numberOfCharToCopy = bufferLength-1;
							strncpy_s(bufferChar, bufferLength, resultNfc->frameInfoPayloadContentPduType, numberOfCharToCopy);
							return(numberOfCharToCopy);
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
				else if (!strcmp(measurement, "capSigInfoSaX_string"))
				{
					if (resultNfc->capSigInfoSaX_string)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->capSigInfoSaX_string);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->capSigInfoSaX_string, numberOfCharToCopy);
						return(numberOfCharToCopy);
					}
					else
					{
						return 0;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaY_string"))
				{
					if (resultNfc->capSigInfoSaY_string)
					{
						int numberOfCharToCopy = 0;
						int tempLen = (int)strlen(resultNfc->capSigInfoSaY_string);
						if ( bufferLength > tempLen )
							numberOfCharToCopy = tempLen;
						else
							numberOfCharToCopy = bufferLength-1;
						strncpy_s(bufferChar, bufferLength, resultNfc->capSigInfoSaY_string, numberOfCharToCopy);
						return(numberOfCharToCopy);
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
			else
			{
				return 0;
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
			if ( hndl->tx->vsg[i]->fpgaLoadCalTable == 0 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsgCw", "fpgaLoadCalTable", hndl->tx->vsg[i]->fpgaLoadCalTable);
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently disabled!\n", "LP_SetVsgCw", "fpgaLoadCalTable" );
				hndl->tx->vsg[i]->fpgaLoadCalTable = 1;	
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsgCw", "fpgaLoadCalTable", hndl->tx->vsg[i]->fpgaLoadCalTable);
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently enabled!\n", "LP_SetVsgCw", "fpgaLoadCalTable" );
			}
		}

		if (setupChanged)
		{
			if (hndl->SetTxRx())
				err = ERR_SET_TX_FAILED;
			if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 2 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsgCw", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable);
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s!\n", "LP_SetVsgCw", "Cal Table was successfully downloaded" );
			}
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
			if ( hndl->tx->vsg[i]->fpgaLoadCalTable == 0 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsgCw", "fpgaLoadCalTable", hndl->tx->vsg[i]->fpgaLoadCalTable); 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently disabled!\n", "LP_SetVsgCw", "fpgaLoadCalTable" );
				hndl->tx->vsg[i]->fpgaLoadCalTable = 1;	
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsgCw", "fpgaLoadCalTable", hndl->tx->vsg[i]->fpgaLoadCalTable);
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently enabled!\n", "LP_SetVsgCw", "fpgaLoadCalTable" );
			}
			
		}

		if ( offsetChanged )
		{
			if (hndl->SetTxRx())
				err = ERR_SET_TX_FAILED;
			if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 2 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsgCw", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable); 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s!\n", "LP_SetVsgCw", "Cal Table was successfully downloaded" );
			}
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
		// -cfy@sunnyvale, 2012/3/13-
		if ( (hndl->tx->rfFreqHz!=rfFreqHz)||(hndl->tx->freqShiftHz!=dFreqShiftHz) || g_bDisableFreqFilter == true)
		{
			hndl->tx->rfFreqHz = rfFreqHz;
			hndl->rx->rfFreqHz = rfFreqHz;

			hndl->tx->freqShiftHz = dFreqShiftHz;
			hndl->rx->freqShiftHz = dFreqShiftHz;
			setupChanged = true;
			g_bDisableFreqFilter = false;
			/* <><~~ */
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

		// -cfy@sunnyvale, 2012/3/13-
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
		/* <><~~ */

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
			if ( hndl->tx->vsg[i]->fpgaLoadCalTable == 0 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsg", "fpgaLoadCalTable", hndl->tx->vsg[i]->fpgaLoadCalTable); 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently disabled!\n", "LP_SetVsg", "fpgaLoadCalTable" );
				hndl->tx->vsg[i]->fpgaLoadCalTable = 1; 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsg", "fpgaLoadCalTable", hndl->tx->vsg[i]->fpgaLoadCalTable); 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently enabled!\n", "LP_SetVsg", "fpgaLoadCalTable" );
			}

		}
		if (setupChanged)
		{
			if (hndl->SetTxRx())
				err = ERR_SET_TX_FAILED;
			if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 2 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsg", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable); 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s!\n", "LP_SetVsg", "Cal Table was successfully downloaded" );
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

IQMEASURE_API int LP_SetVsg_Compensation(double		  dcErrI,
		double		  dcErrQ,
		double		  phaseErr,
		double		  gainErr,
		double		  delayErr)
{
	int err = ERR_OK;
	bool setupChanged = false;
	//	bool RFused = true;

	TIMER_StartTimer(timerIQmeasure, "LP_SetVsg_Compensation", &timeStart);

	if (LibsInitialized)
	{
		for (int i=0; i<nTesters; i++)
		{

			if (hndl->tx->vsg[i]->dcErrI != dcErrI)
			{
				hndl->tx->vsg[i]->dcErrI = dcErrI;
				setupChanged = true;
			}
			if (hndl->tx->vsg[i]->dcErrQ != dcErrQ)
			{
				hndl->tx->vsg[i]->dcErrQ = dcErrQ;
				setupChanged = true;
			}
			if (hndl->tx->vsg[i]->phaseErr != phaseErr)
			{
				hndl->tx->vsg[i]->phaseErr = phaseErr;
				setupChanged = true;
			}
			if (hndl->tx->vsg[i]->gainErr != gainErr)
			{
				hndl->tx->vsg[i]->gainErr = gainErr;
				setupChanged = true;
			}
			if (hndl->tx->vsg[i]->delayErr != delayErr)
			{
				hndl->tx->vsg[i]->delayErr = delayErr;
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

	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsg_Compensation", &timeDuration, &timeStop);

	return err;
}

IQMEASURE_API int LP_SetVsgTriggerType(int triggerType)
{
	int err = ERR_OK;
	bool setupChanged = false;
	bool RFused = true;
	int autoTriggerRearm = IQV_VSG_TRIG_REARM_AUTO;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsgTriggerType", &timeStart);

	if (LibsInitialized)
	{
		if(hndl->tx->triggerType != (IQV_VSG_TRIG)triggerType)
		{
			hndl->tx->triggerType = (IQV_VSG_TRIG)triggerType;
			// set trigger rearm so it is not necessary to rearm the VSG again and again.
			hndl->tx->triggerReArm = (IQV_VSG_TRIG_REARM)autoTriggerRearm;
			setupChanged = true;
		}
		if (setupChanged)
		{
			if (hndl->SetTxRx())
			{
				err = ERR_SET_TX_FAILED;
			}
			else
			{
				// Do Nothing
			}
		}
		else
		{
			// Do Nothing
		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	//After dealing with NxN function, restore g_nxnFunctionCalled to false
	g_nxnFunctionCalled = false;

	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsgTriggerType", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetVsgTriggerType", timeDuration);

	return err;
	// Modified /* -cfy@sunnyvale, 2012/3/13- */
}

IQMEASURE_API int LP_SetVsg_GapPower(double rfFreqHz, double rfPowerLeveldBm, int port, int gapPowerOff)
{
	int err = ERR_OK;
	bool setupChanged = false;
	bool RFused = true;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsg", &timeStart);

	if (LibsInitialized)
	{
		if ( (hndl->tx->rfFreqHz!=rfFreqHz)||(hndl->tx->freqShiftHz!=0)||g_bDisableFreqFilter == true ) // -cfy@sunnyvale, 2012/3/13-
		{
			hndl->tx->rfFreqHz = rfFreqHz;
			hndl->rx->rfFreqHz = rfFreqHz;

			hndl->tx->freqShiftHz = 0;
			hndl->rx->freqShiftHz = 0;
			setupChanged = true;
			g_bDisableFreqFilter = false; // -cfy@sunnyvale, 2012/3/13-
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

IQMEASURE_API int LP_SetVsgModulation(char *modFileName, int /*loadInternalWaveform*/)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsgModulation", &timeStart);

	//change file extension from .mod to .iqvsg
	char *pch = NULL;
	pch = strstr(modFileName,".iqvsg\0");
	if(pch!= NULL)
		strncpy (pch,".mod\0",5);

	if (LibsInitialized)
	{
		if (hndl->SetWave(modFileName))
		{
			err = ERR_SET_WAVE_FAILED;
		}
		else
		{
			// Mark as single MOD file mod
			g_vsgMode = ::VSG_SINGLE_MOD_FILE;
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
				NULL==analysisHT40 &&
				NULL==analysisHT40WideBandHiAccuracy &&
				NULL==analysisHT20WideBandLoAccuracy &&
				NULL==analysisHT20WideBandHiAccuracy)
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
				NULL==analysisZigbee &&
				NULL==analysisHT40WideBandHiAccuracy &&
				NULL==analysisHT20WideBandLoAccuracy &&
				NULL==analysisHT20WideBandHiAccuracy)
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
			if(IQAPI_ERR_OK != err)
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

		if( NULL!=loadIQDataToVsg )
		{
			err = hndl->SetWave( iqWave );
			if(IQAPI_ERR_OK != err)
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
			if (hndl->data->Save(sigFileName))
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

// -cfy@sunnyvale, 2012/3/13-
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
/* <><~~ */

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

		// g_userData is used for selecting a portion of capture for analysis
		// Any new capture will reset this variable to NULL
		if( NULL!=g_userData )
		{
			const int MAX_TESTER_NUM = 4;
			for(int j = 0 ; j < MAX_TESTER_NUM ; j++){
				g_userData->real[j] = NULL;
				g_userData->imag[j] = NULL;
			}
			delete g_userData;
			g_userData = NULL;
		}

		if (hndl->LoadSignalFile(sigFileName))
			err = ERR_LOAD_WAVE_FAILED;
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
				|| g_bDisableFreqFilter == true ) // -cfy@sunnyvale, 2012/3/13-
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
			g_bDisableFreqFilter = false; // -cfy@sunnyvale, 2012/3/13-
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

			// -cfy@sunnyvale, 2012/3/13-
			//if ( abs(hndl->rx->vsa[i]->rfAmplDb-rfAmplDb)>abs(g_amplitudeToleranceDb) )
			//if (hndl->rx->vsa[i]->rfAmplDb != rfAmplDb)
			bool rfAmplChanged = false;
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
			if (true == rfAmplChanged) // -cfy@sunnyvale, 2012/3/13-

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
			/* <><~~ */
			if ( hndl->tx->vsg[i]->fpgaLoadCalTable == 0 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsaBluetooth", "fpgaLoadCalTable", hndl->tx->vsg[i]->fpgaLoadCalTable); 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently disabled!\n", "LP_SetVsaBluetooth", "fpgaLoadCalTable" );
				hndl->tx->vsg[i]->fpgaLoadCalTable = 1;	
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsaBluetooth", "fpgaLoadCalTable", hndl->tx->vsg[i]->fpgaLoadCalTable); 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently enabled!\n", "LP_SetVsaBluetooth", "fpgaLoadCalTable" );
			}
		}

		if (setupChanged)
		{
			if (hndl->SetTxRx())
				err = ERR_SET_RX_FAILED;
			if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 2 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsaBluetooth", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable); 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s!\n", "LP_SetVsaBluetooth", "Cal Table was successfully downloaded" );
			}
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
	int  recordedTriggerType = 0; // -cfy@sunnyvale, 2012/3/13-

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsa", &timeStart);

	if (LibsInitialized)
	{

		// -cfy@sunnyvale, 2012/3/13-
		if(hndl->rx->powerMode == IQV_VSA_TYPE_0)  // always use IQV_VSA_TYPE_1 (Mask correlation issue BD-244)
			hndl->rx->powerMode = IQV_VSA_TYPE_1;
		/* <><~~ */

		if( g_previousOperationIsRX && nTesters>1 )
		{
			//TODO: this is to work around the problem on IQnxn that TX stops working after RX
			hndl->SetDefault();
			g_previousOperationIsRX = false;
		}

		// -cfy@sunnyvale, 2012/3/13-
		if ( (hndl->rx->rfFreqHz!=rfFreqHz)||(hndl->rx->freqShiftHz!=dFreqShiftHz)||(hndl->tx->rfFreqHz!=rfFreqHz) || g_bDisableFreqFilter == true)
		{
			hndl->rx->rfFreqHz = rfFreqHz;
			hndl->tx->rfFreqHz = rfFreqHz;
			hndl->rx->freqShiftHz = dFreqShiftHz;
			hndl->tx->freqShiftHz = dFreqShiftHz;
			setupChanged = true;
			g_bDisableFreqFilter = false;
		}
		/* <><~~ */

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

			// -cfy@sunnyvale, 2012/3/13-
			//			if ( abs(hndl->rx->vsa[i]->rfAmplDb-rfAmplDb)>abs(g_amplitudeToleranceDb) )
			bool rfAmplChanged = false;
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
				/* <><~~ */
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

			// -cfy@sunnyvale, 2012/3/13-
			//Workaround MPS mode takes 30 secs for the 1st LP_SetVsa
			recordedTriggerType = hndl->rx->triggerType;
			if (hndl->rx->triggerType == IQV_TRIG_TYPE_FREE_RUN && bIQ201xFound == true)
			{
				hndl->rx->triggerType = IQV_TRIG_TYPE_FOR_IQ2010_DIG;
				setupChanged = true;
			}
			/* <><~~ */
			if ( hndl->tx->vsg[i]->fpgaLoadCalTable == 0 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsa", "fpgaLoadCalTable", hndl->tx->vsg[i]->fpgaLoadCalTable); 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently disabled!\n", "LP_SetVsa", "fpgaLoadCalTable" );
				hndl->tx->vsg[i]->fpgaLoadCalTable = 1; 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsa", "fpgaLoadCalTable", hndl->tx->vsg[i]->fpgaLoadCalTable); 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently enabled!\n", "LP_SetVsa", "fpgaLoadCalTable" );
			}
		}

		if (setupChanged)
		{
			if (hndl->SetTxRx())
				err = ERR_SET_RX_FAILED;
			if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 2 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsa", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable); 
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s!\n", "LP_SetVsa", "Cal Table was successfully downloaded" );
			}

			// -cfy@sunnyvale, 2012/3/13-
			//Workaround MPS mode takes 30 secs for the 1st LP_SetVsa
			if (err == ERR_OK)
			{
				hndl->rx->triggerType = (IQV_TRIG_TYPE_ENUM)recordedTriggerType;
				if (hndl->SetTxRx())
					err = ERR_SET_RX_FAILED;
			}
			/* <><~~ */
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
		if ( hndl->rx->triggerTimeOut!=triggerTimeoutSec )
		{
			hndl->rx->triggerTimeOut = triggerTimeoutSec;
			if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 0 )
			{
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsaTriggerTimeout", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable);
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently disabled!\n", "LP_SetVsaTriggerTimeout", "fpgaLoadCalTable" );
				hndl->tx->vsg[0]->fpgaLoadCalTable = 1;		
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsaTriggerTimeout", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable);
				::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently enabled!\n", "LP_SetVsaTriggerTimeout", "fpgaLoadCalTable" );
			}
			if (hndl->SetTxRx())
			{
				err = ERR_SET_RX_FAILED;
				if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 2 )
				{
					::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "LP_SetVsaTriggerTimeout", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable);
					::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s!\n", "LP_SetVsaTriggerTimeout", "Cal Table was successfully downloaded" );
				}
			}
			else
			{
				err = ERR_OK;
			}
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

IQMEASURE_API int LP_VsaDataCapture(double samplingTimeSecs, int triggerType,
		double sampleFreqHz, int ht40Mode, IQMEASURE_CAPTURE_NONBLOCKING_STATES nonBlockingState )
{
	int err = ERR_OK;
	bool setupChanged = false;
	int  finalTriggerType = 0;

	::TIMER_StartTimer(timerIQmeasure, "LP_VsaDataCapture", &timeStart);

	if (triggerType == IQV_TRIG_TYPE_IF2_NO_CAL && bIQ201xFound == true)				// if it is a IQ201x, then use IQ201x trigger
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

		// g_userData is used for selecting a portion of capture for analysis
		// Any new capture will reset this variable to NULL
		if( NULL!=g_userData )
		{
			const int MAX_TESTER_NUM = 4;
			for(int j = 0 ; j < MAX_TESTER_NUM ; j++){
				g_userData->real[j] = NULL;
				g_userData->imag[j] = NULL;
			}
			delete g_userData;
			g_userData = NULL;
		}

		if (0!=err)
		{
			//do nothing
		}
		else
		{
			//SL, 2011-10-24
			//Avoid to call Capture(), the error code will be overwritten as 0 even if SetTxRx() is failed.

			// -cfy@sunnyvale, 2012/3/13-
			if(nonBlockingState==BLOCKING)
			{
				if( (int)IQV_HT_40_CAPTURE_TYPE <= ht40Mode &&
						(int)IQV_HT_40_WIDE_BAND_HI_ACCURACY_CAPTURE_TYPE >= ht40Mode)
				{
					err = hndl->Capture((IQV_CAPTURE_TYPE_ENUM)ht40Mode);
				}
				else
				{
					err = hndl->Capture();
				}
			}
			else if(nonBlockingState==ARM_TRIGGER)
			{
				err = hndl->ContCapture(IQV_DC_NON_BLOCKING);
			}
			else if(nonBlockingState==CHECK_DATA)
			{
				int timeout = 0;
				err = hndl->ContCapture(IQV_DC_WAIT_FOR_TRIGGER);

				while (!(hndl->data&&hndl->data->length[0]) && timeout<100)
				{
					Sleep(10);
					timeout++;
				}
			}
			else { } //do nothing
			/* <><~~ */
		}

		//SL, 2011-10-24
		//Cover by previous source code, to do more error code checking.
		/*//SL, 2011-10-7, for LO/High accuracy capture
		  if( (int)IQV_HT_40_CAPTURE_TYPE <= ht40Mode &&
		  (int)IQV_HT_40_WIDE_BAND_HI_ACCURACY_CAPTURE_TYPE >= ht40Mode)
		  {
		  err = hndl->Capture((IQV_CAPTURE_TYPE_ENUM)ht40Mode);
		  }
		  else
		  {
		  err = hndl->Capture();
		  }*/

		//SL, 2011-10-7, for LO/High accuracy capture
		//Cover by previous source code.
		//if( ON==ht40Mode )
		//{
		//	err = hndl->Capture( IQV_HT_40_CAPTURE_TYPE );
		//}
		//else
		//{
		//	err = hndl->Capture();
		//}

		//SL, 2011-10-07:
		//record capture type to decide which kind of analyze will be used.
		if(nonBlockingState!=ARM_TRIGGER) // -cfy@sunnyvale, 2012/3/13-
		{

			g_captureType = (IQV_CAPTURE_TYPE_ENUM)ht40Mode;

			if (0!=err)
			{
				err = ERR_CAPTURE_FAILED;
			}
			else	// Copy hndl->data
			{
				// do nothing
			}

			if( ON==ht40Mode )
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

IQMEASURE_API int LP_GetHndlDataPointers(double *real[],
		double *imag[],
		int *length,
		double *sampleFreqHz,
		int arraySize)
{
	int err = ERR_OK;

	if (LibsInitialized)
	{
		if( NULL!=hndl->data && NULL!=hndl->data->real[0] )
		{
			for(int i=0; i<min(N_MAX_TESTERS, arraySize); i++)
			{
				real[i]         = hndl->data->real[i];
				imag[i]         = hndl->data->imag[i];
				length[i]       = hndl->data->length[i];
				sampleFreqHz[i] = hndl->data->sampleFreqHz[i];
			}
		}
		else
		{
			err = ERR_NO_CAPTURE_DATA;
		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	return err;
}

IQMEASURE_API int LP_SaveUserDataToSigFile(char* sigFileName,
		double *real[],
		double *imag[],
		int *length,
		double *sampleFreqHz,
		int arraySize)
{
	int err = ERR_OK;

	if (LibsInitialized)
	{
		iqapiCapture *userData = new iqapiCapture();
		for(int i=0; i<min(N_MAX_TESTERS, arraySize); i++)
		{
			userData->real[i]             = real[i];
			userData->imag[i]             = imag[i];
			userData->length[i]           = length[i];
			userData->sampleFreqHz[i]     = sampleFreqHz[i];
		}
		if (userData->Save(sigFileName))
		{
			err = ERR_SAVE_WAVE_FAILED;
		}
		// Destructor of iqapiCapture will try to free the memory for real,image if they are not NULL
		// which may cause crash if the memory allocated belongs to another DLL
		// The solution is to set all pointers to NULL if they simply point to somewhere else
		for(int i=0; i<min(N_MAX_TESTERS, arraySize); i++)
		{
			userData->real[i]             = NULL;
			userData->imag[i]             = NULL;
		}
		delete userData;
		userData = NULL;
	}
	else
		err = ERR_NOT_INITIALIZED;

	return err;
}


IQMEASURE_API int LP_SelectCaptureRangeForAnalysis(double startPositionUs, double lengthUs,
		int packetsOffset, int packetsLength)
{
	// note: packetsOffset and packetsLength are ignored for IQ2010 (legacy)

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
			const int MAX_TESTER_NUM = 4;
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

// -cfy@sunnyvale, 2012/3/13-
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
/* <><~~ */

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

IQMEASURE_API int LP_Analyze80211ag(int ph_corr_mode, int ch_estimate, int sym_tim_corr, int freq_sync, int ampl_track, double prePowStartSec,
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

// LP_AnalyzeMimo kept for backward compatibility
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
		analysisMimo->type = type;
		analysisMimo->mode = mode;
		analysisMimo->enablePhaseCorr = enablePhaseCorr;
		analysisMimo->enableSymTimingCorr = enableSymTimingCorr;
		analysisMimo->enableAmplitudeTracking = enableAmplitudeTracking;
		analysisMimo->decodePSDU = decodePSDU;
		analysisMimo->enableFullPacketChannelEst = enableFullPacketChannelEst;
		analysisMimo->frequencyCorr = frequencyCorr;
		analysisMimo->referenceFile = referenceFile;
		analysisMimo->packetFormat = packetFormat;
		//if( 0==strcmp("sequential_mimo",mode) )
		//{
		//	analysisMimo->SequentialMimo.numSections = g_numSections;
		//	analysisMimo->SequentialMimo.sectionLenSec = g_sectionLenSec;
		//	analysisMimo->SequentialMimo.interSectionGapSec = 0;
		//}

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

IQMEASURE_API int LP_Analyze80211b(int eq_taps, int DCremove11b_flag, int method_11b,double prePowStartSec,
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

		// -cfy@sunnyvale, 2012/3/13-
		if (!analysis11b) {
			return ERR_ANALYSIS_FAILED;
		}
		/* <><~~ */

		analysis11b->eq_taps = (IQV_EQ_ENUM) eq_taps;
		analysis11b->DCremove11b_flag = (IQV_DC_REMOVAL_ENUM) DCremove11b_flag;
		analysis11b->method_11b = (IQV_11B_METHOD_ENUM) method_11b;


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
	IQV_WINDOW_TYPE_ENUM  l_windowType = IQV_WIN_BLACKMANHARRIS;


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

		//SL, 2011-10-07
		//Identify capture type to use suitable analysis.
		if(NULL != window_type)
		{
			if(!_stricmp(window_type,"blackmanharris"))
			{
				l_windowType = IQV_WIN_BLACKMANHARRIS;
			}
			else if(!_stricmp(window_type,"rectangular"))
			{
				l_windowType = IQV_WIN_RECT;
			}
			else if(!_stricmp(window_type,"hanning"))
			{
				l_windowType = IQV_WIN_HANNING;
			}
			else
			{
				l_windowType = IQV_WIN_BLACKMANHARRIS;
			}
		}

		switch(g_captureType)
		{
			case IQV_HT_20_WIDE_BAND_HI_ACCURACY_CAPTURE_TYPE:
				{
					analysisHT20WideBandHiAccuracy->windowType     = l_windowType;
					analysisHT20WideBandHiAccuracy->dbResolutionBW = res_bw;
					hndl->analysis = dynamic_cast<iqapiAnalysisHT20WideBandHiAccuracy *>(analysisHT20WideBandHiAccuracy);
				}
				break;
			case IQV_HT_20_WIDE_BAND_LO_ACCURACY_CAPTURE_TYPE:
				{
					analysisHT20WideBandLoAccuracy->windowType     = l_windowType;
					analysisHT20WideBandLoAccuracy->dbResolutionBW = res_bw;
					hndl->analysis = dynamic_cast<iqapiAnalysisHT20WideBandLoAccuracy *>(analysisHT20WideBandLoAccuracy);
				}
				break;
			case IQV_NORMAL_CAPTURE_TYPE:
			default:
				{
					//analysisFFT->F_sample = hndl->rx->sampleFreqHz;
					analysisFFT->NFFT = NFFT;
					analysisFFT->res_bw = res_bw;
					analysisFFT->window_type = window_type;
					hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisFFT);
				}
				break;
		}

		//SL, 2011-10-07, Move to switch(){}, case: IQV_NORMAL_CAPTURE_TYPE
		////analysisFFT->F_sample = hndl->rx->sampleFreqHz;
		//analysisFFT->NFFT = NFFT;
		//analysisFFT->res_bw = res_bw;
		//analysisFFT->window_type = window_type;
		//hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisFFT);

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
// Kept for backward compatibility
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

		//SL, 2011-10-07
		//Identify capture type to use suitable analysis.
		switch(g_captureType)
		{
			case IQV_HT_40_WIDE_BAND_HI_ACCURACY_CAPTURE_TYPE:
				hndl->analysis = dynamic_cast<iqapiAnalysisHT40WideBandHiAccuracy *>(analysisHT40WideBandHiAccuracy);
				break;
			case IQV_HT_40_WIDE_BAND_LO_ACCURACY_CAPTURE_TYPE:
			default:
				{
#if !defined(IQAPI_1_5_X)
					// iqapiResultHT40 is supported in IQapi 1.6.x and beyond
					hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisHT40);
#endif
				}
				break;
		}

		//SL, 2011-10-07, Move to switch(){}, case: IQV_HT_40_WIDE_BAND_LO_ACCURACY_CAPTURE_TYPE
		//#if !defined(IQAPI_1_5_X)
		//        // iqapiResultHT40 is supported in IQapi 1.6.x and beyond
		//        hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisHT40);
		//#endif
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
 *   - LP_AnalyzeNfc();
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
				else if (!strcmp(measurement, "rmsPhaseNoise"))
				{
					if (resultOfdm->rmsPhaseNoise && resultOfdm->rmsPhaseNoise->length > index)
						return(resultOfdm->rmsPhaseNoise->real[index]);
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
						return(resultOfdm->dcLeakageDbc->real[index]);
					else
						return NA_NUMBER;
				}

				else if (!strcmp(measurement, "SpectFlat"))
				{

					// calculate the power at each 64 point from  I & Q data
					int length, i;
					double data[64];
					double subCarrierPower[53], all_sub_power, avg_power;
					double x_axis[53];

					length =resultOfdm->hhEst->length;
					for (i = 0; i < length; i++) data[i] = calc_energy (resultOfdm->hhEst->real[i], resultOfdm->hhEst->imag[i]);

					// the first item of fft result array is the center freq, then right side band, following with left side band
					// element 0 is FFT output at DC
					// element 1 is FFT output for carrier +1
					// element 2 is FFT output for carrier +2
					// .
					// element 26 is FFT output for carrier 26
					// element 27 through 37 should be ignored
					// element 38 is FFT output for carrier -26
					// element 39 is FFT output for carrier -25
					// .
					// element 63 is FFT output for carrier -1

					// calculate the linear power of each subcarrier
					for (i = 1; i <= 26; i++) subCarrierPower[i + 26] = data[i];
					for (i = 38; i <= 63; i++) subCarrierPower[i - 38] = data[i];
					subCarrierPower[26] = (data[1] + data[63]) * 0.5;      //Interpolate

					//Calculate average energy (avg_power) from spectral lines ¡V16.. ¡V1 and +1.. +16
					all_sub_power = 0;
					for (i = 10; i <= 42; i++)
					{
						if (i != 26) all_sub_power += subCarrierPower[i];
					}
					all_sub_power = all_sub_power/32;       //32 = 16 + 16

					if(all_sub_power < 0.000001)
					{
						avg_power = -9.9e9;
					}
					else
					{
						avg_power = 10 * log10(all_sub_power);
					}

					//Shift level to reference to avg_power
					for (i = 0; i <= 52; i++)
					{
						subCarrierPower[i] = 10 * log10(subCarrierPower[i]) - avg_power;
						//sprintf(cTemp, "subCarrier: %d => %.3f\n", i - 26, subCarrierPower[i]);
						//sprintf(message, "%s%s", message, cTemp);

						x_axis[i] = i - 26;
					}

					//Spectral flatness test
					//----------------------------------------------------------------------

					int fail_point_Flatness = 0;

					for (i = 0; i <= 52; i++)
						if (subCarrierPower[i] > 2) fail_point_Flatness++;

					for (i = 0; i <= 9; i++)
						if (subCarrierPower[i] < -4) fail_point_Flatness++;

					for (i = 43; i <= 52; i++)
						if (subCarrierPower[i] < -4) fail_point_Flatness++;

					for (i = 10; i <= 42; i++)
						if ((subCarrierPower[i] < -2) && (i != 26)) fail_point_Flatness++;

					return ((double)fail_point_Flatness);
				}
				else
				{
					return NA_NUMBER;
				}

			}
			else if (dynamic_cast<iqapiResultHT40 *>(hndl->results))
			{
				resultHT40 = dynamic_cast<iqapiResultHT40 *>(hndl->results);

				if (!strcmp(measurement, "length"))
				{
					if (resultHT40->len_of_ffts)
						return((int)resultHT40->ffts[0]->x->length);
					else
						return NA_NUMBER;
				}

				else
				{
					return NA_NUMBER;
				}
			}
			else if (dynamic_cast<iqapiResultObw *>(hndl->results))
			{
				resultObw = dynamic_cast<iqapiResultObw *>(hndl->results);

				if (!strcmp(measurement, "upperFreq"))
				{
					if (resultObw->upperFreq && resultObw->upperFreq->length > index)
						return(resultObw->upperFreq->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "lowerFreq"))
				{
					if (resultObw->lowerFreq && resultObw->lowerFreq->length > index)
						return(resultObw->lowerFreq->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "actualPowerDbInObw"))
				{
					if (resultObw->actualPowerDbInObw && resultObw->actualPowerDbInObw->length > index)
						return(resultObw->actualPowerDbInObw->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "obwHz"))
				{
					if (resultObw->obwHz && resultObw->obwHz->length > index)
						return(resultObw->obwHz->real[index]);
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
				else if (!strcmp(measurement, "packetDetection"))
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

				//		else if (!strcmp(measurement, "loLeakageDb"))
				//		{
				//			if (result11b->loLeakageDb && result11b->loLeakageDb->length > index)
				//				return(result11b->loLeakageDb->real[index]);
				//			else
				//				return NA_NUMBER;
				//		}

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
				{
					return((double)resultPower->valid);
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
			else if (dynamic_cast<iqapiResultHT20WideBandLoAccuracy *>(hndl->results))
			{
				resultHT20WideBandLoAccuracy = dynamic_cast<iqapiResultHT20WideBandLoAccuracy *>(hndl->results);

				if (!strcmp(measurement, "length"))
				{
					if (resultHT20WideBandLoAccuracy->ffts && resultHT20WideBandLoAccuracy->len_of_ffts)
						return(resultHT20WideBandLoAccuracy->ffts[0]->x->length);
					else
						return NA_NUMBER;
				}
				else
				{
					return NA_NUMBER;
				}
			}
			else if (dynamic_cast<iqapiResultHT20WideBandHiAccuracy *>(hndl->results))
			{
				resultHT20WideBandHiAccuracy = dynamic_cast<iqapiResultHT20WideBandHiAccuracy *>(hndl->results);

				if (!strcmp(measurement, "length"))
				{
					if (resultHT20WideBandHiAccuracy->ffts && resultHT20WideBandHiAccuracy->len_of_ffts)
						return(resultHT20WideBandHiAccuracy->ffts[0]->x->length);
					else
						return NA_NUMBER;
				}
				else
				{
					return NA_NUMBER;
				}
			}
			else if (dynamic_cast<iqapiResultHT40WideBandHiAccuracy *>(hndl->results))
			{
				resultHT40WideBandHiAccuracy = dynamic_cast<iqapiResultHT40WideBandHiAccuracy *>(hndl->results);

				if (!strcmp(measurement, "length"))
				{
					if (resultHT40WideBandHiAccuracy->ffts && resultHT40WideBandHiAccuracy->len_of_ffts)
						return(resultHT40WideBandHiAccuracy->ffts[0]->x->length);
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
				else if (!strcmp(measurement, "P_av_no_gap_all_dBm"))
				{
					if (resultBluetooth->P_av_no_gap_all_dBm && resultBluetooth->P_av_no_gap_all_dBm->length > index)
						return(resultBluetooth->P_av_no_gap_all_dBm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "P_pk_each_burst_dBm"))
				{
					if (resultBluetooth->P_pk_each_burst_dBm && resultBluetooth->P_pk_each_burst_dBm->length > index)
						return(resultBluetooth->P_pk_each_burst_dBm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "P_av_each_burst_dBm"))
				{
					if (resultBluetooth->P_av_each_burst_dBm && resultBluetooth->P_av_each_burst_dBm->length > index)
						return(resultBluetooth->P_av_each_burst_dBm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "P_av_all_dBm"))
				{
					if (resultBluetooth->P_av_all_dBm && resultBluetooth->P_av_all_dBm->length > index)
						return(resultBluetooth->P_av_all_dBm->real[index]);
					else
						return NA_NUMBER;
				}
				else if (!strcmp(measurement, "P_peak_all_dBm"))
				{
					if (resultBluetooth->P_peak_all_dBm && resultBluetooth->P_peak_all_dBm->length > index)
						return(resultBluetooth->P_peak_all_dBm->real[index]);
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
				//else if (!strcmp(measurement, "leDeltaF2Max"))
				//{
				//    if (resultBluetooth->leDeltaF2Max && resultBluetooth->leDeltaF2Max->length > index)
				//        return(resultBluetooth->leDeltaF2Max->real[index]);
				//    else
				//        return NA_NUMBER;
				//}
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
			// NFC
			else if (dynamic_cast<iqapiResultNfc *>(hndl->results))
			{
				resultNfc = dynamic_cast<iqapiResultNfc *>(hndl->results);
				if (!strcmp(measurement, "valid"))
				{
					return((double)resultNfc->valid);
				}
				else if (!strcmp(measurement, "packetDetection"))
				{
					if ( resultNfc->valid == 1 )
					{
						return((int)resultNfc->packetDetection);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "acquisition"))
				{
					if ( resultNfc->valid == 1 )
					{
						return((int)resultNfc->acquisition);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "demodulation"))
				{
					if ( resultNfc->valid == 1 )
					{
						return((int)resultNfc->demodulation);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "completePacket"))
				{
					if ( resultNfc->valid == 1 )
					{
						return((int)resultNfc->completePacket);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "rfAnalysis"))
				{
					if ( resultNfc->valid == 1 )
					{
						return((int)resultNfc->rfAnalysis);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "start_sec"))
				{
					if ( resultNfc->start_sec->length > 0 )
					{
						return((double)resultNfc->start_sec->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "stop_sec"))
				{
					if ( resultNfc->stop_sec->length > 0 )
					{
						return((double)resultNfc->stop_sec->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "complete_burst[0]"))
				{
					if ( resultNfc->complete_burst->length > 0 )
					{
						return((double)resultNfc->complete_burst->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}

				else if (!strcmp(measurement, "complete_burst[1]"))
				{
					if ( resultNfc->complete_burst->length > 1 )
					{
						return((double)resultNfc->complete_burst->real[1]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveData"))
				{
					if ( resultNfc->waveData->length > 0 )
					{
						return((double)resultNfc->waveData->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveFs"))
				{
					if ( resultNfc->waveFs->length > 0 )
					{
						return((double)resultNfc->waveFs->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveLen"))
				{
					return((double)resultNfc->waveLen);
				}
				else if (!strcmp(measurement, "waveStartPt"))
				{
					return((double)resultNfc->waveStartPt);
				}
				else if (!strcmp(measurement, "waveSaX"))
				{
					if ( resultNfc->rfAnalysis )
					{
						if ( resultNfc->waveSaX->length > 0 )
						{
							return((double)resultNfc->waveSaX->real[0]);
						}
						else
						{
							return NA_NUMBER;
						}
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveSaY"))
				{
					if ( resultNfc->rfAnalysis )
					{
						if ( resultNfc->waveSaY->length > 0 )
						{
							return((double)resultNfc->waveSaY->real[0]);
						}
						else
						{
							return NA_NUMBER;
						}
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveSaLength"))
				{
					if ( resultNfc->rfAnalysis )
					{
						if ( resultNfc->waveSaLength->length > 0 )
						{
							return((double)resultNfc->waveSaLength->real[0]);
						}
						else
						{
							return NA_NUMBER;
						}
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveSaRes_bw"))
				{
					if ( resultNfc->rfAnalysis )
					{
						if ( resultNfc->waveSaRes_bw->length > 0 )
						{
							return((double)resultNfc->waveSaRes_bw->real[0]);
						}
						else
						{
							return NA_NUMBER;
						}
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveSaNoise_bw"))
				{
					if ( resultNfc->rfAnalysis )
					{
						if ( resultNfc->waveSaNoise_bw->length > 0 )
						{
							return((double)resultNfc->waveSaNoise_bw->real[0]);
						}
						else
						{
							return NA_NUMBER;
						}
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveX"))
				{
					if ( resultNfc->rfAnalysis )
					{
						if ( resultNfc->waveX->length > 0 )
						{
							return((double)resultNfc->waveX->real[0]);
						}
						else
						{
							return NA_NUMBER;
						}
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveY"))
				{
					if ( resultNfc->rfAnalysis )
					{
						if ( resultNfc->waveY->length > 0 )
						{
							return((double)resultNfc->waveY->real[0]);
						}
						else
						{
							return NA_NUMBER;
						}
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoCarrierFreq"))
				{
					if ( resultNfc->capSigInfoCarrierFreq->length > 0 )
					{
						return((double)resultNfc->capSigInfoCarrierFreq->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoBurstDurationSec"))
				{
					if ( resultNfc->capSigInfoBurstDurationSec->length > 0 )
					{
						return((double)resultNfc->capSigInfoBurstDurationSec->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaX"))
				{
					if ( resultNfc->capSigInfoSaX->length > 0 )
					{
						return((double)resultNfc->capSigInfoSaX->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaY"))
				{
					if ( resultNfc->capSigInfoSaY->length > 0 )
					{
						return((double)resultNfc->capSigInfoSaY->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaLength"))
				{
					if ( resultNfc->capSigInfoSaLength->length > 0 )
					{
						return((double)resultNfc->capSigInfoSaLength->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaRes_bw"))
				{
					if ( resultNfc->capSigInfoSaRes_bw->length > 0 )
					{
						return((double)resultNfc->capSigInfoSaRes_bw->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaNoise_bw"))
				{
					if ( resultNfc->capSigInfoSaNoise_bw->length > 0 )
					{
						return((double)resultNfc->capSigInfoSaNoise_bw->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoWaveX"))
				{
					if ( resultNfc->capSigInfoWaveX->length > 0 )
					{
						return((double)resultNfc->capSigInfoWaveX->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoWaveY"))
				{
					if ( resultNfc->capSigInfoWaveY->length > 0 )
					{
						return((double)resultNfc->capSigInfoWaveY->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveInfoModIndex"))
				{
					if ( resultNfc->waveInfoModIndex->length > 0 )
					{
						return((double)resultNfc->waveInfoModIndex->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveInfoModDepth"))
				{
					if ( resultNfc->waveInfoModDepth->length > 0 )
					{
						return((double)resultNfc->waveInfoModDepth->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveInfoCarrierAmplitudeDbV"))
				{
					if ( resultNfc->waveInfoCarrierAmplitudeDbV->length > 0 )
					{
						return((double)resultNfc->waveInfoCarrierAmplitudeDbV->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveInfoBandwidthKHz"))
				{
					return((int)resultNfc->waveInfoBandwidthKHz);
				}
				else if (!strcmp(measurement, "waveInfoNominalBitDuration"))
				{
					if ( resultNfc->waveInfoNominalBitDuration->length > 0 )
					{
						return((double)resultNfc->waveInfoNominalBitDuration->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveInfoNominalDataRate"))
				{
					if ( resultNfc->waveInfoNominalDataRate->length > 0 )
					{
						return((double)resultNfc->waveInfoNominalDataRate->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveInfoMeasuredDataRate"))
				{
					if ( resultNfc->waveInfoMeasuredDataRate->length > 0 )
					{
						return((double)resultNfc->waveInfoMeasuredDataRate->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveInfoMeasuredBitDuration"))
				{
					if ( resultNfc->waveInfoMeasuredBitDuration->length > 0 )
					{
						return((double)resultNfc->waveInfoMeasuredBitDuration->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveInfoOnWidthSec"))
				{
					if ( resultNfc->waveInfoOnWidthSec->length > 0 )
					{
						return((double)resultNfc->waveInfoOnWidthSec->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "waveInfoOffWidthSec"))
				{
					if ( resultNfc->waveInfoOffWidthSec->length > 0 )
					{
						return((double)resultNfc->waveInfoOffWidthSec->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "frameInfoAnalyzedFrameId"))
				{
					if ( resultNfc->frameInfoAnalyzedFrameId->length > 0 )
					{
						return((double)resultNfc->frameInfoAnalyzedFrameId->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "frameInfoCrcFail"))
				{
					return((bool)resultNfc->frameInfoCrcFail);
				}
				else if (!strcmp(measurement, "frameInfoParityCheckErrorByteInd"))
				{
					if ( resultNfc->frameInfoParityCheckErrorByteInd->length > 0 )
					{
						return((double)resultNfc->frameInfoParityCheckErrorByteInd->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "frameInfoSigType"))
				{
					if ( resultNfc->frameInfoSigType->length > 0 )
					{
						return((double)resultNfc->frameInfoSigType->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "frameInfoEvmDb"))
				{
					if ( resultNfc->frameInfoEvmDb->length > 0 )
					{
						return((double)resultNfc->frameInfoEvmDb->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "frameInfoSigConstellation"))
				{
					if ( resultNfc->frameInfoSigConstellation->length > 0 )
					{
						return((double)resultNfc->frameInfoSigConstellation->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "frameInfoPayloadContentCmdType"))
				{
					if ( resultNfc->frameInfoDecodeTransportMsg )
					{
						return((int)resultNfc->frameInfoPayloadContentCmdType);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecOnAmp"))
				{
					if ( resultNfc->pulseSpecOnAmp->length > 0 )
					{
						return((double)resultNfc->pulseSpecOnAmp->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecOffAmp"))
				{
					if ( resultNfc->pulseSpecOffAmp->length > 0 )
					{
						return((double)resultNfc->pulseSpecOffAmp->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecFallTime"))
				{
					if ( resultNfc->pulseSpecFallTime->length > 0 )
					{
						return((double)resultNfc->pulseSpecFallTime->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecRiseTime"))
				{
					if ( resultNfc->pulseSpecRiseTime->length > 0 )
					{
						return((double)resultNfc->pulseSpecRiseTime->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecOnOvershoot"))
				{
					if ( resultNfc->pulseSpecOnOvershoot->length > 0 )
					{
						return((double)resultNfc->pulseSpecOnOvershoot->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecOnUndershoot"))
				{
					if ( resultNfc->pulseSpecOnUndershoot->length > 0 )
					{
						return((double)resultNfc->pulseSpecOnUndershoot->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecOnRipple"))
				{
					if ( resultNfc->pulseSpecOnRipple->length > 0 )
					{
						return((double)resultNfc->pulseSpecOnRipple->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecOnSettlingTime"))
				{
					if ( resultNfc->pulseSpecOnSettlingTime->length > 0 )
					{
						return((double)resultNfc->pulseSpecOnSettlingTime->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecOffUndershoot"))
				{
					if ( resultNfc->pulseSpecOffUndershoot->length > 0 )
					{
						return((double)resultNfc->pulseSpecOffUndershoot->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecOffOvershoot"))
				{
					if ( resultNfc->pulseSpecOffOvershoot->length > 0 )
					{
						return((double)resultNfc->pulseSpecOffOvershoot->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecOffRipple"))
				{
					if ( resultNfc->pulseSpecOffRipple->length > 0 )
					{
						return((double)resultNfc->pulseSpecOffRipple->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecOffSettlingTime"))
				{
					if ( resultNfc->pulseSpecOffSettlingTime->length > 0 )
					{
						return((double)resultNfc->pulseSpecOffSettlingTime->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecT1"))
				{
					if ( resultNfc->pulseSpecT1->length > 0 )
					{
						return((double)resultNfc->pulseSpecT1->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecT2"))
				{
					if ( resultNfc->pulseSpecT2->length > 0 )
					{
						return((double)resultNfc->pulseSpecT2->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecT3"))
				{
					if ( resultNfc->pulseSpecT3->length > 0 )
					{
						return((double)resultNfc->pulseSpecT3->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "pulseSpecT4"))
				{
					if ( resultNfc->pulseSpecT4->length > 0 )
					{
						return((double)resultNfc->pulseSpecT4->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoCarrierFreq"))
				{
					if ( resultNfc->capSigInfoCarrierFreq->length > 0 )
					{
						return((double)resultNfc->capSigInfoCarrierFreq->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoBurstDurationSec"))
				{
					if ( resultNfc->capSigInfoBurstDurationSec->length > 0 )
					{
						return((double)resultNfc->capSigInfoBurstDurationSec->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaX"))
				{
					if ( resultNfc->capSigInfoSaX->length > 0 )
					{
						return((double)resultNfc->capSigInfoSaX->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaY"))
				{
					if ( resultNfc->capSigInfoSaY->length > 0 )
					{
						return((double)resultNfc->capSigInfoSaY->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaLength"))
				{
					if ( resultNfc->capSigInfoSaLength->length > 0 )
					{
						return((double)resultNfc->capSigInfoSaLength->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaRes_bw"))
				{
					if ( resultNfc->capSigInfoSaRes_bw->length > 0 )
					{
						return((double)resultNfc->capSigInfoSaRes_bw->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "capSigInfoSaNoise_bw"))
				{
					if ( resultNfc->capSigInfoSaNoise_bw->length > 0 )
					{
						return((double)resultNfc->capSigInfoSaNoise_bw->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "obw"))
				{
					if ( resultNfc->obw->length > 0 )
					{
						return((double)resultNfc->obw->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else if (!strcmp(measurement, "actualObw"))
				{
					if ( resultNfc->actualObw->length > 0 )
					{
						return((double)resultNfc->actualObw->real[0]);
					}
					else
					{
						return NA_NUMBER;
					}
				}
				else
				{
					return NA_NUMBER;
				}
			}
			else
			{
				return NA_NUMBER;
			}
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
				else if (!strcmp(measurement, "psdu"))
				{
					if (resultMimo->psduBits && resultMimo->psduBits->length)
					{
						if (bufferLength > resultMimo->psduBits->length)
							bufferLength = resultMimo->psduBits->length;

						memcpy(bufferReal, resultMimo->psduBits->real, (sizeof(double)*bufferLength));

						if (resultMimo->psduBits->imag && bufferImag)
							memcpy(bufferImag, resultMimo->psduBits->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
					{
						return 0;
					}
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

				else if (!strcmp(measurement, "maxPowerAcpDbm"))
				{
					if (resultBluetooth->maxPowerAcpDbm && resultBluetooth->maxPowerAcpDbm->length)
					{
						if (bufferLength > resultBluetooth->maxPowerAcpDbm->length)
							bufferLength = resultBluetooth->maxPowerAcpDbm->length;

						memcpy(bufferReal, resultBluetooth->maxPowerAcpDbm->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->maxPowerAcpDbm->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->maxPowerAcpDbm->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "maxPowerEdrDbm"))
				{
					if (resultBluetooth->maxPowerEdrDbm && resultBluetooth->maxPowerEdrDbm->length)
					{
						if (bufferLength > resultBluetooth->maxPowerEdrDbm->length)
							bufferLength = resultBluetooth->maxPowerEdrDbm->length;

						memcpy(bufferReal, resultBluetooth->maxPowerEdrDbm->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->maxPowerEdrDbm->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->maxPowerEdrDbm->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "meanNoGapPowerCenterDbm"))
				{
					if (resultBluetooth->meanNoGapPowerCenterDbm && resultBluetooth->meanNoGapPowerCenterDbm->length)
					{
						if (bufferLength > resultBluetooth->meanNoGapPowerCenterDbm->length)
							bufferLength = resultBluetooth->meanNoGapPowerCenterDbm->length;

						memcpy(bufferReal, resultBluetooth->meanNoGapPowerCenterDbm->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->meanNoGapPowerCenterDbm->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->meanNoGapPowerCenterDbm->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "sequenceDefinition"))
				{
					if (resultBluetooth->sequenceDefinition && resultBluetooth->sequenceDefinition->length)
					{
						if (bufferLength > resultBluetooth->sequenceDefinition->length)
							bufferLength = resultBluetooth->sequenceDefinition->length;

						memcpy(bufferReal, resultBluetooth->sequenceDefinition->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->sequenceDefinition->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->sequenceDefinition->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "leDeltaF2Max"))
				{
					if (resultBluetooth->leDeltaF2Max && resultBluetooth->leDeltaF2Max->length)
					{
						if (bufferLength > resultBluetooth->leDeltaF2Max->length)
							bufferLength = resultBluetooth->leDeltaF2Max->length;

						memcpy(bufferReal, resultBluetooth->leDeltaF2Max->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->leDeltaF2Max->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->leDeltaF2Max->imag, (sizeof(double)*bufferLength));

						return(bufferLength);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "leMaxPowerDbm"))
				{
					if (resultBluetooth->leMaxPowerDbm && resultBluetooth->leMaxPowerDbm->length)
					{
						if (bufferLength > resultBluetooth->leMaxPowerDbm->length)
							bufferLength = resultBluetooth->leMaxPowerDbm->length;

						memcpy(bufferReal, resultBluetooth->leMaxPowerDbm->real, (sizeof(double)*bufferLength));

						if (resultBluetooth->leMaxPowerDbm->imag && bufferImag)
							memcpy(bufferImag, resultBluetooth->leMaxPowerDbm->imag, (sizeof(double)*bufferLength));

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
			else if (dynamic_cast<iqapiResultHT40WideBandHiAccuracy *>(hndl->results))
			{
				resultHT40WideBandHiAccuracy = dynamic_cast<iqapiResultHT40WideBandHiAccuracy *>(hndl->results);

				if (!strcmp(measurement, "x"))
				{
					if (resultHT40WideBandHiAccuracy->ffts && resultHT40WideBandHiAccuracy->len_of_ffts)
					{
						double *pReal = bufferReal;
						double *pImag = bufferImag;
						int    len    = bufferLength;
						for(int i=0; i<resultHT40WideBandHiAccuracy->len_of_ffts; i++)
						{
							if (len > resultHT40WideBandHiAccuracy->ffts[i]->x->length)
							{
								//bufferLength = resultHT40->len_of_ffts * resultHT40->ffts[0]->length;

								memcpy(pReal, resultHT40WideBandHiAccuracy->ffts[i]->x->real, (sizeof(double) * resultHT40WideBandHiAccuracy->ffts[i]->x->length));
								pReal += resultHT40WideBandHiAccuracy->ffts[i]->x->length;
								len   -= resultHT40WideBandHiAccuracy->ffts[i]->x->length;

								if (resultHT40WideBandHiAccuracy->ffts[i]->x->imag && bufferImag)
								{
									memcpy(pImag, resultHT40WideBandHiAccuracy->ffts[i]->x->imag, (sizeof(double) * resultHT40WideBandHiAccuracy->ffts[i]->x->length));
									pImag += resultHT40WideBandHiAccuracy->ffts[i]->x->length;
								}
							}
							else
							{
								// the buffer has been used up
								memcpy(pReal, resultHT40WideBandHiAccuracy->ffts[i]->x->real, (sizeof(double) * len));
								if (resultHT40WideBandHiAccuracy->ffts[i]->x->imag && bufferImag)
								{
									memcpy(pImag, resultHT40WideBandHiAccuracy->ffts[i]->x->imag, (sizeof(double) * len));
								}
								break;
							}
						}

						//TODO: Fix this
						return(resultHT40WideBandHiAccuracy->ffts[0]->x->length);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "y"))
				{
					if (resultHT40WideBandHiAccuracy->ffts && resultHT40WideBandHiAccuracy->len_of_ffts)
					{
						double *pReal = bufferReal;
						double *pImag = bufferImag;
						int    len = bufferLength;
						for(int i=0; i<resultHT40WideBandHiAccuracy->len_of_ffts; i++)
						{
							if (len > resultHT40WideBandHiAccuracy->ffts[i]->y->length)
							{
								//bufferLength = resultHT40->len_of_ffts * resultHT40->ffts[0]->length;

								memcpy(pReal, resultHT40WideBandHiAccuracy->ffts[i]->y->real, (sizeof(double) * resultHT40WideBandHiAccuracy->ffts[i]->y->length));
								pReal += resultHT40WideBandHiAccuracy->ffts[i]->y->length;
								len   -= resultHT40WideBandHiAccuracy->ffts[i]->y->length;

								if (resultHT40WideBandHiAccuracy->ffts[i]->y->imag && bufferImag)
								{
									memcpy(pImag, resultHT40WideBandHiAccuracy->ffts[i]->y->imag, (sizeof(double) * resultHT40WideBandHiAccuracy->ffts[i]->y->length));
									pImag += resultHT40WideBandHiAccuracy->ffts[i]->y->length;
								}
							}
							else
							{
								// the buffer has been used up
								memcpy(pReal, resultHT40WideBandHiAccuracy->ffts[i]->y->real, (sizeof(double) * len));
								if (resultHT40WideBandHiAccuracy->ffts[i]->y->imag && bufferImag)
								{
									memcpy(pImag, resultHT40WideBandHiAccuracy->ffts[i]->y->imag, (sizeof(double) * len));
								}
								break;
							}
						}

						return(resultHT40WideBandHiAccuracy->ffts[0]->y->length);
					}
					else
						return 0;
				}
			}
			else if (dynamic_cast<iqapiResultHT20WideBandLoAccuracy *>(hndl->results))
			{
				resultHT20WideBandLoAccuracy = dynamic_cast<iqapiResultHT20WideBandLoAccuracy *>(hndl->results);

				if (!strcmp(measurement, "x"))
				{
					if (resultHT20WideBandLoAccuracy->ffts && resultHT20WideBandLoAccuracy->len_of_ffts)
					{
						double *pReal = bufferReal;
						double *pImag = bufferImag;
						int    len    = bufferLength;
						for(int i=0; i<resultHT20WideBandLoAccuracy->len_of_ffts; i++)
						{
							if (len > resultHT20WideBandLoAccuracy->ffts[i]->x->length)
							{
								//bufferLength = resultHT40->len_of_ffts * resultHT40->ffts[0]->length;

								memcpy(pReal, resultHT20WideBandLoAccuracy->ffts[i]->x->real, (sizeof(double) * resultHT20WideBandLoAccuracy->ffts[i]->x->length));
								pReal += resultHT20WideBandLoAccuracy->ffts[i]->x->length;
								len   -= resultHT20WideBandLoAccuracy->ffts[i]->x->length;

								if (resultHT20WideBandLoAccuracy->ffts[i]->x->imag && bufferImag)
								{
									memcpy(pImag, resultHT20WideBandLoAccuracy->ffts[i]->x->imag, (sizeof(double) * resultHT20WideBandLoAccuracy->ffts[i]->x->length));
									pImag += resultHT20WideBandLoAccuracy->ffts[i]->x->length;
								}
							}
							else
							{
								// the buffer has been used up
								memcpy(pReal, resultHT20WideBandLoAccuracy->ffts[i]->x->real, (sizeof(double) * len));
								if (resultHT20WideBandLoAccuracy->ffts[i]->x->imag && bufferImag)
								{
									memcpy(pImag, resultHT20WideBandLoAccuracy->ffts[i]->x->imag, (sizeof(double) * len));
								}
								break;
							}
						}

						//TODO: Fix this
						return(resultHT20WideBandLoAccuracy->ffts[0]->x->length);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "y"))
				{
					if (resultHT20WideBandLoAccuracy->ffts && resultHT20WideBandLoAccuracy->len_of_ffts)
					{
						double *pReal = bufferReal;
						double *pImag = bufferImag;
						int    len = bufferLength;
						for(int i=0; i<resultHT20WideBandLoAccuracy->len_of_ffts; i++)
						{
							if (len > resultHT20WideBandLoAccuracy->ffts[i]->y->length)
							{
								//bufferLength = resultHT40->len_of_ffts * resultHT40->ffts[0]->length;

								memcpy(pReal, resultHT20WideBandLoAccuracy->ffts[i]->y->real, (sizeof(double) * resultHT20WideBandLoAccuracy->ffts[i]->y->length));
								pReal += resultHT20WideBandLoAccuracy->ffts[i]->y->length;
								len   -= resultHT20WideBandLoAccuracy->ffts[i]->y->length;

								if (resultHT20WideBandLoAccuracy->ffts[i]->y->imag && bufferImag)
								{
									memcpy(pImag, resultHT20WideBandLoAccuracy->ffts[i]->y->imag, (sizeof(double) * resultHT20WideBandLoAccuracy->ffts[i]->y->length));
									pImag += resultHT20WideBandLoAccuracy->ffts[i]->y->length;
								}
							}
							else
							{
								// the buffer has been used up
								memcpy(pReal, resultHT20WideBandLoAccuracy->ffts[i]->y->real, (sizeof(double) * len));
								if (resultHT20WideBandLoAccuracy->ffts[i]->y->imag && bufferImag)
								{
									memcpy(pImag, resultHT20WideBandLoAccuracy->ffts[i]->y->imag, (sizeof(double) * len));
								}
								break;
							}
						}

						return(resultHT20WideBandLoAccuracy->ffts[0]->y->length);
					}
					else
						return 0;
				}
			}
			else if (dynamic_cast<iqapiResultHT20WideBandHiAccuracy *>(hndl->results))
			{
				resultHT20WideBandHiAccuracy = dynamic_cast<iqapiResultHT20WideBandHiAccuracy *>(hndl->results);

				if (!strcmp(measurement, "x"))
				{
					if (resultHT20WideBandHiAccuracy->ffts && resultHT20WideBandHiAccuracy->len_of_ffts)
					{
						double *pReal = bufferReal;
						double *pImag = bufferImag;
						int    len    = bufferLength;
						for(int i=0; i<resultHT20WideBandHiAccuracy->len_of_ffts; i++)
						{
							if (len > resultHT20WideBandHiAccuracy->ffts[i]->x->length)
							{
								//bufferLength = resultHT40->len_of_ffts * resultHT40->ffts[0]->length;

								memcpy(pReal, resultHT20WideBandHiAccuracy->ffts[i]->x->real, (sizeof(double) * resultHT20WideBandHiAccuracy->ffts[i]->x->length));
								pReal += resultHT20WideBandHiAccuracy->ffts[i]->x->length;
								len   -= resultHT20WideBandHiAccuracy->ffts[i]->x->length;

								if (resultHT20WideBandHiAccuracy->ffts[i]->x->imag && bufferImag)
								{
									memcpy(pImag, resultHT20WideBandHiAccuracy->ffts[i]->x->imag, (sizeof(double) * resultHT20WideBandHiAccuracy->ffts[i]->x->length));
									pImag += resultHT20WideBandHiAccuracy->ffts[i]->x->length;
								}
							}
							else
							{
								// the buffer has been used up
								memcpy(pReal, resultHT20WideBandHiAccuracy->ffts[i]->x->real, (sizeof(double) * len));
								if (resultHT20WideBandHiAccuracy->ffts[i]->x->imag && bufferImag)
								{
									memcpy(pImag, resultHT20WideBandHiAccuracy->ffts[i]->x->imag, (sizeof(double) * len));
								}
								break;
							}
						}

						//TODO: Fix this
						return(resultHT20WideBandHiAccuracy->ffts[0]->x->length);
					}
					else
						return 0;
				}
				else if (!strcmp(measurement, "y"))
				{
					if (resultHT20WideBandHiAccuracy->ffts && resultHT20WideBandHiAccuracy->len_of_ffts)
					{
						double *pReal = bufferReal;
						double *pImag = bufferImag;
						int    len = bufferLength;
						for(int i=0; i<resultHT20WideBandHiAccuracy->len_of_ffts; i++)
						{
							if (len > resultHT20WideBandHiAccuracy->ffts[i]->y->length)
							{
								//bufferLength = resultHT40->len_of_ffts * resultHT40->ffts[0]->length;

								memcpy(pReal, resultHT20WideBandHiAccuracy->ffts[i]->y->real, (sizeof(double) * resultHT20WideBandHiAccuracy->ffts[i]->y->length));
								pReal += resultHT20WideBandHiAccuracy->ffts[i]->y->length;
								len   -= resultHT20WideBandHiAccuracy->ffts[i]->y->length;

								if (resultHT20WideBandHiAccuracy->ffts[i]->y->imag && bufferImag)
								{
									memcpy(pImag, resultHT20WideBandHiAccuracy->ffts[i]->y->imag, (sizeof(double) * resultHT20WideBandHiAccuracy->ffts[i]->y->length));
									pImag += resultHT20WideBandHiAccuracy->ffts[i]->y->length;
								}
							}
							else
							{
								// the buffer has been used up
								memcpy(pReal, resultHT20WideBandHiAccuracy->ffts[i]->y->real, (sizeof(double) * len));
								if (resultHT20WideBandHiAccuracy->ffts[i]->y->imag && bufferImag)
								{
									memcpy(pImag, resultHT20WideBandHiAccuracy->ffts[i]->y->imag, (sizeof(double) * len));
								}
								break;
							}
						}

						return(resultHT20WideBandHiAccuracy->ffts[0]->y->length);
					}
					else
						return 0;
				}
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

	::TIMER_StopTimer(timerIQmeasure, "LP_GetVectorMeasurement", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_GetVectorMeasurement", timeDuration);


	return ret;
}


void InstantiateAnalysisClasses()
{
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
	analysisNfc = new iqapiAnalysisNfc();
#if !defined(IQAPI_1_5_X)
	analysisHT40 = new iqapiAnalysisHT40();
#endif
	analysisHT40WideBandHiAccuracy  = new iqapiAnalysisHT40WideBandHiAccuracy();
	analysisHT20WideBandLoAccuracy  = new iqapiAnalysisHT20WideBandLoAccuracy();
	analysisHT20WideBandHiAccuracy  = new iqapiAnalysisHT20WideBandHiAccuracy();

	// FM
	analysisFmRf                = new iqapiAnalysisFmRf();
	analysisFmAudioBase         = new iqapiAnalysisFmAudioBase();
	analysisFmDemodSpectrum     = new iqapiAnalysisFmDemodSpectrum();
	analysisFmMono              = new iqapiAnalysisFmMono();
	analysisFmStereo            = new iqapiAnalysisFmStereo();
	analysisFmAuto              = new iqapiAnalysisFmAuto();
	analysisRds                 = new iqapiAnalysisRds();
	analysisRdsMono             = new iqapiAnalysisRdsMono();
	analysisRdsStereo           = new iqapiAnalysisRdsStereo();
	analysisRdsAuto             = new iqapiAnalysisRdsAuto();
	analysisAudioStereo         = new iqapiAnalysisAudioStereo();
	analysisAudioMono           = new iqapiAnalysisAudioMono();

	return;
}

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
		if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 0 )
		{
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "InitializeTesterSettings", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable);
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently disabled!\n", "InitializeTesterSettings", "fpgaLoadCalTable" );
			hndl->tx->vsg[0]->fpgaLoadCalTable = 1;
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "InitializeTesterSettings", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable);
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is currently enabled!\n", "InitializeTesterSettings", "fpgaLoadCalTable" );
		}
		hndl->SetTxRx();
		if ( hndl->tx->vsg[0]->fpgaLoadCalTable == 2 )
		{
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s is %.2f!\n", "InitializeTesterSettings", "fpgaLoadCalTable", hndl->tx->vsg[0]->fpgaLoadCalTable);
			::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s] %s!\n", "InitializeTesterSettings", "Cal Table was successfully downloaded" );
		}
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
		//if(NULL!=ipAddress1 && NULL==ipAddress2 && NULL==ipAddress3 && NULL==ipAddress4)
		//{
		//    err = hndl->ConOpen(ipAddress1);
		//}
		//else if (NULL!=ipAddress1 && NULL!=ipAddress2 && NULL==ipAddress3 && NULL==ipAddress4)
		//{
		//    err = hndl->ConInit(ipAddress1, ipAddress2);
		//}
		//else if (NULL!=ipAddress1 && NULL!=ipAddress2 && NULL!=ipAddress3 && NULL==ipAddress4)
		//{
		//    err = hndl->ConInit(ipAddress1, ipAddress2, ipAddress3);
		//}
		//else if (NULL!=ipAddress1 && NULL!=ipAddress2 && NULL!=ipAddress3 && NULL!=ipAddress4)
		//{
		//    err = hndl->ConInit(ipAddress1, ipAddress2, ipAddress3, ipAddress4);
		//}
		//else
		//{
		//    err = -1;
		//}

		if( IQAPI_ERR_OK == err )
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
				// -cfy@sunnyvale, 2012/3/13-
				else
				{
					//workaround MPS load multiwave takes 15 secs to load after enable dual head and token in obtain control
					if (bIQ201xFound == true)
					{
						//hndl->SetDefault();
						//hndl->tx->vsg[0]->rfGainDb = -50;
						//hndl->rx->freqShiftHz = 0;
						//hndl->tx->triggerType = IQV_VSG_TRIG_FREE_RUN;
						//hndl->rx->triggerType = IQV_TRIG_TYPE_FOR_IQ2010;
						//hndl->SetTxRx();

					}
					g_bDisableFreqFilter = true;
				}
				/* <><~~ */
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
		// -cfy@sunnyvale, 2012/3/13-
		if (bIQ201xFound == true)
		{
			hndl->tx->vsg[0]->rfGainDb = -50;
			hndl->rx->freqShiftHz = 0;
			hndl->tx->triggerType = IQV_VSG_TRIG_FREE_RUN;
			hndl->rx->triggerType = IQV_TRIG_TYPE_FOR_IQ2010;
			hndl->SetTxRx();
		}
		/* <><~~ */

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


IQMEASURE_API int LP_IQ2010EXT_NewMultiSegmentWaveform(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_NewMultiSegmentWaveform", &timeStart);

	if (LibsInitialized)
	{
		err = IQ2010EXT_NewMultiSegmentWaveform();	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_NewMultiSegmentWaveform", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_NewMultiSegmentWaveform", timeDuration);


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

IQMEASURE_API int LP_IQ2010EXT_RxPer( int               waveformIndex,
		double            freqMHz,
		double            powerLevelStartdBm,
		double            powerLevelStopdBm,
		double            stepdB,
		int               packetCount,
		int				rfPort,
		double            ackPowerRmsdBm,
		double            ackTriggerLeveldBm
		)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_RxPer", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_RxPer( waveformIndex,
				freqMHz,
				powerLevelStartdBm,
				powerLevelStopdBm,
				stepdB,
				packetCount,
				(IQV_PORT_ENUM) rfPort,
				ackPowerRmsdBm,
				ackTriggerLeveldBm
				);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_RxPer", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_RxPer", timeDuration);


	return err;
}

IQMEASURE_API int LP_IQ2010EXT_NewMultiAnalysisCapture( char		  *dataRate,
		double         freqMHz,
		double	       rmsPowerLeveldBm,
		int	           skipPktCnt,
		int			   rfPort,
		double	       triggerLeveldBm)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_NewMultiAnalysisCapture", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		int dataRateIndex = 0;

		err = LP_ConvertDataRateStringToIQ2010ExtIndex( dataRate, &dataRateIndex );
		if (err==ERR_OK)
		{
			err = IQ2010EXT_NewMultiAnalysisCapture( (IQ2010EXT_DATA_RATE) dataRateIndex,
					freqMHz,
					rmsPowerLeveldBm,
					skipPktCnt,
					(IQV_PORT_ENUM) rfPort,
					triggerLeveldBm);	// TODO
		}
		else
		{
			err = ERR_DATARATE_DOES_NOT_EXIST;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_NewMultiAnalysisCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_NewMultiAnalysisCapture", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_AddMultiAnalysisCapture( int			analysisType,
		int	    	captureLengthUs,
		int	    	captureCnt)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_AddMultiAnalysisCapture", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_AddMultiAnalysisCapture( (IQ2010EXT_ANALYSIS_TYPE) analysisType,
				captureLengthUs,
				captureCnt);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_AddMultiAnalysisCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_AddMultiAnalysisCapture", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_InitiateMultiAnalysisCapture(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_InitiateMultiAnalysisCapture", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_InitiateMultiAnalysisCapture( );	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_InitiateMultiAnalysisCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_InitiateMultiAnalysisCapture", timeDuration);

	return err;
}


IQMEASURE_API int LP_IQ2010EXT_FinishMultiAnalysisCapture(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_FinishMultiAnalysisCapture", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_FinishMultiAnalysisCapture( );	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_FinishMultiAnalysisCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_FinishMultiAnalysisCapture", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_InitiateMultiCapture( char				*dataRate,
		double              freqMHz,
		double	             rmsPowerLeveldBm,
		int	             captureLengthUs,
		int	             skipPktCnt,
		int	             captureCnt,
		int                 rfPort,
		double	             triggerLeveldBm)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_InitiateMultiCapture", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		int dataRateIndex = 0;

		err = LP_ConvertDataRateStringToIQ2010ExtIndex( dataRate, &dataRateIndex );
		if (err==ERR_OK)
		{
			err = IQ2010EXT_InitiateMultiCapture( (IQ2010EXT_DATA_RATE) dataRateIndex,
					freqMHz,
					rmsPowerLeveldBm,
					captureLengthUs,
					skipPktCnt,
					captureCnt,
					(IQV_PORT_ENUM) rfPort,
					triggerLeveldBm);	// TODO
		}
		else
		{
			err = ERR_DATARATE_DOES_NOT_EXIST;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_InitiateMultiCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_InitiateMultiCapture", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_InitiateMultiCaptureHT40( char			   *dataRate,
		double             freqMHz,
		double	            rmsPowerLeveldBm,
		int	            captureLengthUs,
		int	            skipPktCnt,
		int	            captureCnt,
		int			    rfPort,
		double	            triggerLeveldBm)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_InitiateMultiCaptureHT40", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		int dataRateIndex = 0;

		err = LP_ConvertDataRateStringToIQ2010ExtIndex( dataRate, &dataRateIndex );
		if (err==ERR_OK)
		{
			err = IQ2010EXT_InitiateMultiCaptureHT40( (IQ2010EXT_DATA_RATE) dataRateIndex,
					freqMHz,
					rmsPowerLeveldBm,
					captureLengthUs,
					skipPktCnt,
					captureCnt,
					(IQV_PORT_ENUM) rfPort,
					triggerLeveldBm);	// TODO
		}
		else
		{
			err = ERR_DATARATE_DOES_NOT_EXIST;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_InitiateMultiCaptureHT40", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_InitiateMultiCaptureHT40", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_FinishMultiCapture(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_FinishMultiCapture", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_FinishMultiCapture();	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_FinishMultiCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_FinishMultiCapture", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_Analyze80211ag( IQV_PH_CORR_ENUM		ph_corr_mode,
		IQV_CH_EST_ENUM		ch_estimate,
		IQV_SYM_TIM_ENUM		sym_tim_corr,
		IQV_FREQ_SYNC_ENUM	freq_sync,
		IQV_AMPL_TRACK_ENUM	ampl_track)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211ag", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_Analyze80211ag(	ph_corr_mode,
				ch_estimate,
				sym_tim_corr,
				freq_sync,
				ampl_track);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211ag", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_Analyze80211ag", timeDuration);

	return err;
}


IQMEASURE_API int LP_IQ2010EXT_Analyze80211nSISO( int enablePhaseCorr,
		int enableSymTimingCorr,
		int enableAmplitudeTracking,
		int decodePSDU,
		int enableFullPacketChannelEst,
		int packetFormat)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211nSISO", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_Analyze80211nSISO( enablePhaseCorr,
				enableSymTimingCorr,
				enableAmplitudeTracking,
				decodePSDU,
				enableFullPacketChannelEst,
				packetFormat);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211nSISO", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_Analyze80211nSISO", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_Analyze80211nComposite(int enablePhaseCorr,
		int enableSymTimingCorr,
		int enableAmplitudeTracking,
		int enableFullPacketChannelEst,
		char *referenceFile,
		int packetFormat)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211nComposite", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_Analyze80211nComposite( enablePhaseCorr,
				enableSymTimingCorr,
				enableAmplitudeTracking,
				enableFullPacketChannelEst,
				referenceFile,
				packetFormat);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211nComposite", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_Analyze80211nComposite", timeDuration);

	return err;
}


IQMEASURE_API int LP_IQ2010EXT_Analyze80211b( IQV_EQ_ENUM			eq_taps,
		IQV_DC_REMOVAL_ENUM	DCremove11b_flag,
		IQV_11B_METHOD_ENUM	method_11b)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211b", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_Analyze80211b( eq_taps,
				DCremove11b_flag,
				method_11b
				);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_Analyze80211b", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_Analyze80211b", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_AnalyzePower( void )
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzePower", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_AnalyzePower();	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzePower", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_AnalyzePower", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_AnalyzeFFT( double  NFFT,
		double  res_bwHz,
		char   *window_type )
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzeFFT", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_AnalyzeFFT( NFFT, res_bwHz, window_type );	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzeFFT", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_AnalyzeFFT", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_AnalyzeMaskMeasurement(int maskType, double obwPercentage)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzeMaskMeasurement", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_AnalyzeMaskMeasurement( (WIFI_MASK_TYPE) maskType, obwPercentage);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzeMaskMeasurement", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_AnalyzeMaskMeasurement", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_AnalyzePowerRamp80211b(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzePowerRamp80211b", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_AnalyzePowerRamp80211b();	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzePowerRamp80211b", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_AnalyzePowerRamp80211b", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_AnalyzeBluetooth( double data_rate, char *analysis_type )
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzeBluetooth", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_AnalyzeBluetooth(data_rate, analysis_type);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzeBluetooth", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_AnalyzeBluetooth", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_AnalyzeSpectralMeasurement(int spectralType)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzeSpectralMeasurement", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_AnalyzeSpectralMeasurement( (IQ2010EXT_SPECTRAL_TYPE) spectralType );	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_AnalyzeSpectralMeasurement", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_AnalyzeSpectralMeasurement", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_GetNumberOfMeasurementElements(char *measurementName, int indexResult, int *numberOfMeasurementElement)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_GetNumberOfMeasurementElements", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_GetNumberOfMeasurementElements( measurementName, indexResult, numberOfMeasurementElement);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_GetNumberOfMeasurementElements", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_GetNumberOfMeasurementElements", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_GetNumberOfPerPowerLevels(char *measurementName, int *numberOfPerPowerLevel)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_GetNumberOfPerPowerLevels", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_GetNumberOfPerPowerLevels( measurementName, numberOfPerPowerLevel );	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_GetNumberOfPerPowerLevels", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_GetNumberOfPerPowerLevels", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_GetDoubleMeasurements( char *measurementName, double *average, double *minimum, double *maximum )
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_GetDoubleMeasurements", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_GetDoubleMeasurements( measurementName,
				average,
				minimum,
				maximum
				);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_GetDoubleMeasurements", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_GetDoubleMeasurements", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_GetStringMeasurement( char *measurementName, char *result, int  bufferSize, int indexResult )
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_GetStringMeasurement", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_GetStringMeasurement( measurementName,
				result,
				bufferSize,
				indexResult
				);	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_GetStringMeasurement", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_GetStringMeasurement", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_GetVectorMeasurement( char *measurementName, double values[], int bufferLength, int *dataSize, int indexResult )
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_GetVectorMeasurement", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_GetVectorMeasurement( measurementName, values, bufferLength, dataSize, indexResult );	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_GetVectorMeasurement", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_GetVectorMeasurement", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_GetIntMeasurement( char *measurementName, int *result, int indexResult )
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_GetIntMeasurement", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		err = IQ2010EXT_GetIntMeasurement( measurementName, result, indexResult );	// TODO
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_GetIntMeasurement", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_GetIntMeasurement", timeDuration);

	return err;
}

int LP_ConvertDataRateStringToIQ2010ExtIndex( char *name, int *dataRateIndex )
{
	int err = ERR_OK;

	map <string, int>::iterator dataRateIQ2010Ext_Iter;

	dataRateIQ2010Ext_Iter = dataRateIQ2010Ext.find(name);
	if( dataRateIQ2010Ext_Iter!=dataRateIQ2010Ext.end() )
	{
		*dataRateIndex = dataRateIQ2010Ext_Iter->second;
	}
	else
	{
		err = ERR_DATARATE_DOES_NOT_EXIST;
	}

	return err;
}

int LP_InitializeIQ2010ExtMap(void)
{
	int err = ERR_OK;

	// Initializes the WiFi data rate IQ2010Ext map
	dataRateIQ2010Ext.clear();
	dataRateIQ2010Ext.insert( pair<string, int>( "DSSS-1",   IQ2010EXT_DSSS_1 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "DSSS-2",   IQ2010EXT_DSSS_2 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "CCK-5_5",  IQ2010EXT_CCK_5_5 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "CCK-11",   IQ2010EXT_CCK_11 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-6",   IQ2010EXT_OFDM_6 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-9",   IQ2010EXT_OFDM_9 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-12",  IQ2010EXT_OFDM_12 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-18",  IQ2010EXT_OFDM_18 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-24",  IQ2010EXT_OFDM_24 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-36",  IQ2010EXT_OFDM_36 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-48",  IQ2010EXT_OFDM_48 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "OFDM-54",  IQ2010EXT_OFDM_54 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS0",	 IQ2010EXT_MCS0 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS1",	 IQ2010EXT_MCS1 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS2",	 IQ2010EXT_MCS2 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS3",	 IQ2010EXT_MCS3 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS4",	 IQ2010EXT_MCS4 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS5",	 IQ2010EXT_MCS5 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS6",	 IQ2010EXT_MCS6 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS7",	 IQ2010EXT_MCS7 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS8",	 IQ2010EXT_MCS8 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS9",	 IQ2010EXT_MCS9 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS10",	 IQ2010EXT_MCS10 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS11",	 IQ2010EXT_MCS11 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS12",	 IQ2010EXT_MCS12 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS13",	 IQ2010EXT_MCS13 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS14",	 IQ2010EXT_MCS14 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS15",	 IQ2010EXT_MCS15 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS16",	 IQ2010EXT_MCS16 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS17",	 IQ2010EXT_MCS17 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS18",	 IQ2010EXT_MCS18 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS19",	 IQ2010EXT_MCS19 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS20",	 IQ2010EXT_MCS20 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS21",	 IQ2010EXT_MCS21 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS22",	 IQ2010EXT_MCS22 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS23",	 IQ2010EXT_MCS23 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS24",	 IQ2010EXT_MCS24 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS25",	 IQ2010EXT_MCS25 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS26",	 IQ2010EXT_MCS26 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS27",	 IQ2010EXT_MCS27 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS28",	 IQ2010EXT_MCS28 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS29",	 IQ2010EXT_MCS29 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS30",	 IQ2010EXT_MCS30 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "MCS31",	 IQ2010EXT_MCS31 ));

	// Initializes the BT data rate IQ2010Ext map
	dataRateIQ2010Ext.insert( pair<string, int>( "1DH1",	 IQ2010EXT_BT_1DH1 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "1DH3",	 IQ2010EXT_BT_1DH3 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "1DH5",	 IQ2010EXT_BT_1DH5 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "2DH1",	 IQ2010EXT_BT_2DH1 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "2DH3",	 IQ2010EXT_BT_2DH3 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "2DH5",	 IQ2010EXT_BT_2DH5 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "3DH1",	 IQ2010EXT_BT_3DH1 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "3DH3",	 IQ2010EXT_BT_3DH3 ));
	dataRateIQ2010Ext.insert( pair<string, int>( "3DH5",	 IQ2010EXT_BT_3DH5 ));

	return err;
}

IQMEASURE_API int LP_SetLpcPath(char *litePointConnectionPath)
{
	int err = ERR_OK;
	char *litePointConnectionPathReturn;

	if (LibsInitialized)
	{
		if( NULL != litePointConnectionPath &&
				string(litePointConnectionPath).length() >0)
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
		else
		{
			//take default path and didn't do anything.
		}



	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	return err;
}

//FM IQMeasure functions
#ifdef WIN32	//Not using for Mac at this time // -cfy@sunnyvale, 2012/3/13-

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
			// Workaround for IQapi // Modified /* -cfy@sunnyvale, 2012/3/13- */
			// For double type we round to tenth
			carrierPowerdBm		= (double)ROUND(carrierPowerdBm * 10) / 10;
			// For int type we round to hundred //[TBD] Using round off. Should truncate? /* -cfy@sunnyvale, 2012/3/13- */
			totalFmDeviationHz	= ROUND((double)totalFmDeviationHz	/ 100) * 100;
			pilotDeviationHz	= ROUND((double)pilotDeviationHz	/ 100) * 100;
			rdsDeviationHz		= ROUND((double)rdsDeviationHz		/ 100) * 100;
			/* <><~~ */

			err = hndl->hndlFm->Activate(IQV_FM_VSG_TECH_ACT);
			if(IQAPI_ERR_OK != err) { throw err;}

			//[TBD] How to diagnose each SetParamXXX() error if not print error msg?
			//      May not good to call SetTx() for each SetParamXXX() /* -cfy@sunnyvale, 2012/3/13- */
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
				hndl->hndlFm->SetParamPilotDeviation(pilotDeviationHz);
			}
			else
			{
				hndl->hndlFm->SetParamStereo(IQV_FM_VSG_STEREO_MODE_OFF);
				//hndl->hndlFm->SetParamPilotDeviation(0); // Change back from 1000 to 0 coz issue fixed. // -cfy@sunnyvale, 2012/3/13-
			}
			if(IQV_FM_VSG_RDS_MODE_ON == rdsEnable)
			{
				hndl->hndlFm->SetParamRdsMode(IQV_FM_VSG_RDS_MODE_ON);
				hndl->hndlFm->SetParamRdsDeviation(rdsDeviationHz);
				if(NULL != rdsTransmitString)
				{
					err = hndl->hndlFm->LoadRdsData(rdsTransmitString);
				}
				else
				{
					rdsTransmitString = "LitepointFM";
					err = hndl->hndlFm->LoadRdsData(rdsTransmitString);
				}
				if(IQAPI_ERR_OK != err) throw err; // -cfy@sunnyvale, 2012/3/13-

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
				hndl->hndlFm->SetParamRdsDeviation(0); // Change back from 1000 to 0 coz issue fixed. // -cfy@sunnyvale, 2012/3/13-
			}

			//hndl->hndlFm->RemoveParamAllAudioTone(); //in case of mono need to make sure previous multi-tones are removed
			err = hndl->hndlFm->SetTx();  // added SetTx() in LP_FM_SetVsg to catch any errors while trying to setVsg
			if(IQAPI_ERR_OK != err) throw err; // -cfy@sunnyvale, 2012/3/13-

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
			if(IQAPI_ERR_OK != err) throw err;

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


// -cfy@sunnyvale, 2012/3/13-
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
			{
				//Round carrierPowerdBm to tenth.
				carrierPowerdBm = (double)ROUND(carrierPowerdBm * 10) / 10; // Modified /* -cfy@sunnyvale, 2012/3/13- */
				hndl->hndlFm->SetParamPowerDbm(carrierPowerdBm);
			}

			err = hndl->hndlFm->SetTx();
			if(IQAPI_ERR_OK != err) throw err;

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
			//Round carrierPowerdBm to tenth.
			carrierPowerdBm = (double)ROUND(carrierPowerdBm * 10) / 10; // Modified /* -cfy@sunnyvale, 2012/3/13- */
			hndl->hndlFm->SetParamPowerDbm(carrierPowerdBm);

			err = hndl->hndlFm->SetTx();
			if(IQAPI_ERR_OK != err) throw err;

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
/* <><~~ */


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
					if(IQAPI_ERR_OK != err) throw err;
				}
				else if(RIGHT_ONLY == leftRightChannelSelect)
				{
					err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_5, audioToneFreqHz,
							(IQV_FM_VSG_AUDIO_CHANNELS)RIGHT_ONLY, audioToneAmpdB);
					if(IQAPI_ERR_OK != err) throw err;
				}
				else if(LEFT_RIGHT == leftRightChannelSelect)
				{
					err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_1, audioToneFreqHz,
							(IQV_FM_VSG_AUDIO_CHANNELS)LEFT_ONLY, audioToneAmpdB);
					if(IQAPI_ERR_OK != err) throw err;

					err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_5, audioToneFreqHz,
							(IQV_FM_VSG_AUDIO_CHANNELS)RIGHT_ONLY, audioToneAmpdB);
					if(IQAPI_ERR_OK != err) throw err;
				}
				else if(LEFT_EQUALS_RIGHT == leftRightChannelSelect)
				{
					err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_1, audioToneFreqHz,
							(IQV_FM_VSG_AUDIO_CHANNELS)LEFT_EQUALS_RIGHT, audioToneAmpdB);
					if(IQAPI_ERR_OK != err) throw err;
				}
				else if(LEFT_EQUALS_MINUS_RIGHT == leftRightChannelSelect)
				{
					err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_1, audioToneFreqHz,
							(IQV_FM_VSG_AUDIO_CHANNELS)LEFT_EQUALS_MINUS_RIGHT, audioToneAmpdB);
					if(IQAPI_ERR_OK != err) throw err;
				}
			}

			else if(FM_MONO == stereo)
			{
				err = hndl->hndlFm->SetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)TONE_INDEX_1, audioToneFreqHz,
						(IQV_FM_VSG_AUDIO_CHANNELS)LEFT_ONLY, audioToneAmpdB);
				if(IQAPI_ERR_OK != err) throw err;
			}

			err = hndl->hndlFm->SetTx();
			if(IQAPI_ERR_OK != err) throw err;
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
							if(IQAPI_ERR_OK != err) throw err;

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
						if(IQAPI_ERR_OK != err) throw err;

					}

				}

				err = hndl->hndlFm->SetTx();
				if(IQAPI_ERR_OK != err) throw err;
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
							if(IQAPI_ERR_OK != err) throw err;

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
						if(IQAPI_ERR_OK != err) throw err;

					}

					err = hndl->hndlFm->SetTx();
					if(IQAPI_ERR_OK != err) throw err;

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
			if(IQAPI_ERR_OK != err) throw err;

			double* audioFreqHz = new double;
			IQV_FM_VSG_AUDIO_CHANNELS* channel =  new IQV_FM_VSG_AUDIO_CHANNELS;
			double* audioAmplitudedB = new double;

			for (unsigned int toneIndex = 0; toneIndex < toneCount; toneIndex++)
			{

				err = hndl->hndlFm->GetParamAudioTone((IQV_FM_VSG_AUDIO_TONE_INDEX)toneIndex, audioFreqHz, channel, audioAmplitudedB);
				if(IQAPI_ERR_OK != err) throw err;

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

// -cfy@sunnyvale, 2012/3/13-
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
/* <><~~ */


IQMEASURE_API int LP_FM_DeleteAudioTones()
{
	int err = ERR_OK;
	::TIMER_StartTimer(timerIQmeasure, "LP_FM_DeleteAudioTones", &timeStart);

	if(FmInitialized)
	{
		if(0 != hndl->hndlFm->GetNumberOfAudioTone())
		{
			hndl->hndlFm->RemoveParamAllAudioTone();
		}
		else
		{
			//No Tones setup so there is no need to delete the tones
		}
		//if(hndl->hndlFm->SetTx())
		//{
		//	err = ERR_FM_SET_TX_FAILED;
		//}
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
			if(IQAPI_ERR_OK != err) throw err;

			err = hndl->hndlFm->Deactivate(IQV_FM_VSG_TECH_ACT);
			if(IQAPI_ERR_OK != err) throw err;

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
		int interfererRelativePowerdBmInt = interfererRelativePowerdBm;
		double decimalforRoundup = (interfererRelativePowerdBm - interfererRelativePowerdBmInt);
		if(decimalforRoundup <0.5)
		{
			interfererRelativePowerdBm = floor(interfererRelativePowerdBm);
		}
		else
		{
			interfererRelativePowerdBm = ceil(interfererRelativePowerdBm);
		}

		if (IQV_FM_VSG_MODULATION_FM == interfererModulationEnable)
		{
			hndl->hndlFm->SetParamInterferer(IQV_FM_VSG_INTERFERING_CARRIER_OFF,interfererCarrierOffsetHz,interfererRelativePowerdBm,
					interfererPeakFreqDeviationHz);
			if(hndl->hndlFm->SetTx())
			{
				err = ERR_FM_SET_TX_FAILED;
			}

			hndl->hndlFm->SetParamInterfererModulation(IQV_FM_VSG_MODULATION_FM, interfererAudioFreqHz);
			if(hndl->hndlFm->SetTx())
			{
				err = ERR_FM_SET_TX_FAILED;
			}

			hndl->hndlFm->SetParamInterferer(IQV_FM_VSG_INTERFERING_CARRIER_ON,interfererCarrierOffsetHz,interfererRelativePowerdBm,
					interfererPeakFreqDeviationHz);
			if(hndl->hndlFm->SetTx())
			{
				err = ERR_FM_SET_TX_FAILED;
			}
		}
		else
		{
			hndl->hndlFm->SetParamInterfererModulation(IQV_FM_VSG_MODULATION_FM, 0);
			if(hndl->hndlFm->SetTx())
			{
				err = ERR_FM_SET_TX_FAILED;
			}
			hndl->hndlFm->SetParamInterferer(IQV_FM_VSG_INTERFERING_CARRIER_ON,interfererCarrierOffsetHz,interfererRelativePowerdBm,0);
			if(hndl->hndlFm->SetTx())
			{
				err = ERR_FM_SET_TX_FAILED;
			}
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

	double marginalInputPowerdBm = 0;//currently no margin is internally applied to VSA Gain settings. The user needs to set the value.

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_SetVsa", &timeStart);

	if(FmInitialized)
	{
		int agcMode = OFF;
		int vsaSampleRate = SAMPLE_640KHz;
		double expectedPeakInputPowerdBmSetting;

		try
		{

			err = hndl->hndlFm->Activate(IQV_FM_VSA_TECH_ACT);
			if (IQAPI_ERR_OK != err) throw err;

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
			if (IQAPI_ERR_OK != err) throw err;
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
				if(g_FmMonoVectorResults.find(measurement) == g_FmMonoVectorResults.end())
				{
					return (int)NA_NUMBER;
				}
				else
				{
					for(int i = 0; i < bufferLength; i++)
					{
						bufferReal[i] = g_FmMonoVectorResults[measurement].at(i);
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
				if(g_FmMonoVectorResults.find(measurement) == g_FmMonoVectorResults.end())
				{
					return (int)NA_NUMBER;
				}
				else
				{
					for(int i = 0; i < bufferLength; i++)
					{
						bufferReal[i] = g_FmMonoVectorResults[measurement].at(i);
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
	const int   bufSize = 1024;
	int         numberOfChannel = 0;
	int         indexChannel = 0;
	char		temp[100]={};

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
		err = ERR_FM_PLAY_AUDIO_FAILED; // -cfy@sunnyvale, 2012/3/13-
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




#pragma region GPS functions

#define     SET_DOPPLER_FREQUENCY         4000
#define     SET_GPS_POWER_DBM             -130
#define     SET_OPERATION_MODE1           IQV_OPERATION_MODE_MODULATION_OFF
// IQV_OPERATION_MODE_MODULATION
// IQV_OPERATION_MODE_MODULATION_NAV_DATA_OFF // no location info
#define     SET_GPS_RF_OUTPUT             IQV_RF_OUTPUT_ON
#define     SET_GPS_TRIGGER_STATE         IQV_TRIGGER_STATE_INT

char* functionName = {'\0'};

IQMEASURE_API int LP_GPS_SetActivate(void)
{
	int err  = ERR_OK;

	functionName = "LP_GPS_SetActivate";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	hndl->SetDefault();
	err = hndl->hndlGps->Activate();

	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_GPS_LoadScenarioFile( char* fileName, IQV_GPS_TRIGGER_STATE triggerState)
{
	int err  = ERR_OK;

	functionName = "LP_GPS_LoadScenarioFile";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	char   buffer[MAX_BUFFER_SIZE] = {'\0'};
	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		int numberOfChannel   = 0;
		int indexChannel      = 0;
		hndl->hndlGps->SetParamOperationMode(SET_OPERATION_MODE1);
		hndl->hndlGps->SetParamPowerDbm(SET_GPS_POWER_DBM, (IQV_GPS_CHANNEL_NUMBER) (IQV_GPS_CHANNEL_NUMBER_1));
		hndl->hndlGps->SetParamModulationState(IQV_MODULATION_STATE_ON, (IQV_GPS_CHANNEL_NUMBER) (IQV_GPS_CHANNEL_NUMBER_1) );
		hndl->hndlGps->SetParamSatelliteNumber(1, (IQV_GPS_CHANNEL_NUMBER) (IQV_GPS_CHANNEL_NUMBER_1));
		hndl->hndlGps->SetParamDopplerFrequency(SET_DOPPLER_FREQUENCY);
		hndl->hndlGps->SetParamRfOutput(SET_GPS_RF_OUTPUT);
		hndl->hndlGps->SetParamTriggerState(triggerState);
		hndl->hndlGps->SetTx();

		//----------------------------//
		//   Load ScenarioFile        //
		//----------------------------//
		if ( err <= IQAPI_ERR_OK )
		{
			err = hndl->hndlGps->LoadScenarioFile(fileName);
			if ( err > IQAPI_ERR_OK )
			{
				printf("Error: Load scenario file %s\nLast Error: %s\n", fileName, hndl->lastErr);
			}
			else
			{
				printf("OK: LoadScenarionFile => %s\n", fileName);
			}
		}
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}

	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_GPS_PlayScenarioFile( double powerDbm, double pathlossDb )
{
	int err  = ERR_OK;

	functionName = "LP_GPS_PlayScenarioFile";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	char   buffer[MAX_BUFFER_SIZE] = {'\0'};
	try
	{
		//----------------------------//
		// Specialized power setting  //
		//----------------------------//
		{
			hndl->hndlGps->GetTx();
			double testerPowerDbm = LP_GPS_CaculateTesterPower(powerDbm, pathlossDb);
			hndl->hndlGps->SetParamPowerDbm(testerPowerDbm, IQV_GPS_CHANNEL_NUMBER_1);
			hndl->hndlGps->SetParamPowerDbm(testerPowerDbm, IQV_GPS_CHANNEL_NUMBER_2);
			hndl->hndlGps->SetParamPowerDbm(testerPowerDbm, IQV_GPS_CHANNEL_NUMBER_3);
			hndl->hndlGps->SetParamPowerDbm(testerPowerDbm, IQV_GPS_CHANNEL_NUMBER_4);
			hndl->hndlGps->SetParamPowerDbm(testerPowerDbm, IQV_GPS_CHANNEL_NUMBER_5);
			hndl->hndlGps->SetParamPowerDbm(testerPowerDbm, IQV_GPS_CHANNEL_NUMBER_6);
			err = hndl->hndlGps->SetTx(false);
		}

		//----------------------------//
		//   Transmit ScenarioFile    //
		//----------------------------//
		if ( err <= IQAPI_ERR_OK )
		{
			err = hndl->hndlGps->PlayScenarioFile();
			if ( err > IQAPI_ERR_OK )
			{
				printf("Error: Playing scenario file. \nLast Error: %s\n", hndl->lastErr);
			}
			else
			{
				printf("OK: PlayScenarioFile\n\n");
			}
		}
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}

	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_GPS_ModulatedMode(int Nav_Mode, double pathlossDb, double powerA[6], int satelliteNumA[6], int dopplerFrequency, IQV_GPS_TRIGGER_STATE triggerState)
{
	int err  = ERR_OK;

	functionName = "LP_GPS_ModulatedMode";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	hndl->hndlGps->SetParamPowerDbm(SET_GPS_POWER_DBM, (IQV_GPS_CHANNEL_NUMBER) (IQV_GPS_CHANNEL_NUMBER_1));
	hndl->hndlGps->SetParamDopplerFrequency(dopplerFrequency);
	hndl->hndlGps->SetParamTriggerState(triggerState);
	hndl->hndlGps->SetParamModulationState(IQV_MODULATION_STATE_ON, (IQV_GPS_CHANNEL_NUMBER) (IQV_GPS_CHANNEL_NUMBER_1) );
	hndl->hndlGps->SetParamSatelliteNumber(1, (IQV_GPS_CHANNEL_NUMBER) (IQV_GPS_CHANNEL_NUMBER_1));
	hndl->hndlGps->SetParamRfOutput(SET_GPS_RF_OUTPUT);

	if (Nav_Mode == 0 ) // 0 means OFF
	{
		hndl->hndlGps->SetParamOperationMode(IQV_OPERATION_MODE_MODULATION_NAV_DATA_OFF);
	}
	else
	{
		hndl->hndlGps->SetParamOperationMode(IQV_OPERATION_MODE_MODULATION);
	}
	//err = ::LP_GPS_SetData("SetParamOperationMode", "IQV_OPERATION_MODE_MODULATION");

	for(int i = 0 ; i < 6 ; i++)
	{
		if ( (satelliteNumA[i] >= 1 && satelliteNumA[i] <= 37) )
		{
			double testerPowerDbm = LP_GPS_CaculateTesterPower(powerA[i], pathlossDb);

			hndl->hndlGps->SetParamPowerDbm(testerPowerDbm,				   (IQV_GPS_CHANNEL_NUMBER)i);
			hndl->hndlGps->SetParamSatelliteNumber(satelliteNumA[i],       (IQV_GPS_CHANNEL_NUMBER)i);
			hndl->hndlGps->SetParamModulationState(IQV_MODULATION_STATE_ON,(IQV_GPS_CHANNEL_NUMBER)i);
		}
		else
		{
			// satellite Number is not vaild
		}
	}

	err = hndl->hndlGps->SetTx(false);

	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

double LP_GPS_CaculateTesterPower(double powerDbm, double pathlossDb)
{
	double testerPowerDbm = 0.0;

	if ( (powerDbm <= -60 && powerDbm >= -145) )
	{
		testerPowerDbm = powerDbm + pathlossDb;
	}
	else
	{
		// power is out of range : set standard
		testerPowerDbm = -130 + pathlossDb;
	}
	return testerPowerDbm;
}

IQMEASURE_API int LP_GPS_ContinueWave(double powerDbm, double pathlossDb, IQV_GPS_TRIGGER_STATE triggerState, int freqOffset)
{
	int err  = ERR_OK;

	functionName = "LP_GPS_ContinueWave";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	hndl->hndlGps->SetParamOperationMode(IQV_OPERATION_MODE_CW);
	double testerPowerDbm = LP_GPS_CaculateTesterPower(powerDbm, pathlossDb);
	hndl->hndlGps->SetParamPowerDbm(testerPowerDbm, (IQV_GPS_CHANNEL_NUMBER)(IQV_GPS_CHANNEL_NUMBER_1));
	hndl->hndlGps->SetParamModulationState(IQV_MODULATION_STATE_ON, (IQV_GPS_CHANNEL_NUMBER) (IQV_GPS_CHANNEL_NUMBER_1) );
	hndl->hndlGps->SetParamSatelliteNumber(1, (IQV_GPS_CHANNEL_NUMBER) (IQV_GPS_CHANNEL_NUMBER_1));
	hndl->hndlGps->SetParamTriggerState(triggerState);
	freqOffset=freqOffset*1000;
	hndl->hndlGps->SetParamCwFrequencyOffset(freqOffset);
	hndl->hndlGps->SetParamRfOutput(IQV_RF_OUTPUT_ON);
	err = hndl->hndlGps->SetTx(false);

	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_GPS_SetDeactivate()
{
	int err  = ERR_OK;

	functionName = "LP_GPS_SetDeactivate";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	err = hndl->hndlGps->Deactivate();

	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_GPS_SetData(const char* inputName, const char* inputValue)
{
	int err  = ERR_OK;

	functionName = "LP_GPS_SetData";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);
	LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s], [%s] : [%s]\n", functionName, inputName, inputValue);

	if ( 0==strcmp(inputName, "SetParamOperationMode") )
	{
		/*
		   IQV_OPERATION_MODE_MODULATION_OFF
		   IQV_OPERATION_MODE_MODULATION
		   IQV_OPERATION_MODE_CW // no location info
		   IQV_OPERATION_MODE_MODULATION_NAV_DATA_OFF,	   //!< Indicates that CA Code modulation is turned on with navigation data off
		   */
		if ( 0==strcmp(inputValue, "IQV_OPERATION_MODE_MODULATION_OFF") ){
			hndl->hndlGps->SetParamOperationMode(IQV_OPERATION_MODE_MODULATION_OFF);
		}else if( 0==strcmp(inputValue, "IQV_OPERATION_MODE_MODULATION") ){
			hndl->hndlGps->SetParamOperationMode(IQV_OPERATION_MODE_MODULATION);
		}else if( 0==strcmp(inputValue, "IQV_OPERATION_MODE_CW") ){
			hndl->hndlGps->SetParamOperationMode(IQV_OPERATION_MODE_CW);
		}else if( 0==strcmp(inputValue, "IQV_OPERATION_MODE_MODULATION_NAV_DATA_OFF") ){
			hndl->hndlGps->SetParamOperationMode(IQV_OPERATION_MODE_MODULATION_NAV_DATA_OFF);
		}else{/* no valid parm */}
	}
	else if( 0==strcmp(inputName, "SetParamPowerDbm") )
	{
		/*
inputValue : tester Power : double
*/
		double testerPower = atof(inputValue);
		if ( testerPower <= -60 && testerPower >= -145 ){ /* check value between -145 and -60 dbm */
			hndl->hndlGps->SetParamPowerDbm(testerPower, IQV_GPS_CHANNEL_NUMBER_1);
		}else{/* no valid parm */}
	}
	else if( 0==strcmp(inputName, "SetParamModulationState") )
	{
		/*
		   IQV_MODULATION_STATE_OFF,	//!< Indicates that modulation state is off
		   IQV_MODULATION_STATE_ON	    //!< Indicates that modulation state is on
		   */
		if ( 0==strcmp(inputValue, "IQV_MODULATION_STATE_ON") ){
			hndl->hndlGps->SetParamModulationState(IQV_MODULATION_STATE_ON, IQV_GPS_CHANNEL_NUMBER_1);
		}else if( 0==strcmp(inputValue, "IQV_MODULATION_STATE_OFF") ){
			hndl->hndlGps->SetParamModulationState(IQV_MODULATION_STATE_OFF, IQV_GPS_CHANNEL_NUMBER_1 );
		}else{/* no valid parm */}
	}
	else if( 0==strcmp(inputName, "SetParamDopplerFrequency") )
	{
		/*
inputValue : doppler Frequency : int
*/
		int dopplerFrequency = atoi(inputValue);
		if ( true ){ /* check value */
			hndl->hndlGps->SetParamDopplerFrequency(dopplerFrequency);
		}else{/* no valid parm */}
	}
	else if( 0==strcmp(inputName, "SetParamRfOutput") )
	{
		/*
		   IQV_RF_OUTPUT_OFF,	//!< Indicates that RF output is off
		   IQV_RF_OUTPUT_ON	//!< Indicates that RF output is on
		   */
		if ( 0==strcmp(inputValue, "IQV_RF_OUTPUT_OFF") ){
			hndl->hndlGps->SetParamRfOutput(IQV_RF_OUTPUT_OFF);
		}else if( 0==strcmp(inputValue, "IQV_RF_OUTPUT_ON") ){
			hndl->hndlGps->SetParamRfOutput(IQV_RF_OUTPUT_ON);
		}else{/* no valid parm */}
	}
	else if( 0==strcmp(inputName, "SetParamTriggerState") )
	{
		/*
		   IQV_TRIGGER_STATE_INT,	//!< Indicates internal trigger state
		   IQV_TRIGGER_STATE_EXT	//!< Indicates external trigger state
		   */
		if ( 0==strcmp(inputValue, "IQV_TRIGGER_STATE_INT") ){
			hndl->hndlGps->SetParamTriggerState(IQV_TRIGGER_STATE_INT);
		}else if( 0==strcmp(inputValue, "IQV_TRIGGER_STATE_EXT") ){
			hndl->hndlGps->SetParamTriggerState(IQV_TRIGGER_STATE_EXT);
		}else{/* no valid parm */}
	}
	else if( 0==strcmp(inputName, "SetReplayScenarioMode") )
	{
		/*
		   IQV_GPS_REPLAY_OFF,
		   IQV_GPS_REPLAY_ON
		   */
		if ( 0==strcmp(inputValue, "IQV_GPS_REPLAY_OFF") ){
			hndl->hndlGps->SetReplayScenarioMode(IQV_GPS_REPLAY_OFF);
		}else if( 0==strcmp(inputValue, "IQV_GPS_REPLAY_ON") ){
			hndl->hndlGps->SetReplayScenarioMode(IQV_GPS_REPLAY_ON);
		}else{/* no valid parm */}
	}
	err = hndl->hndlGps->SetTx(false);

	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_GPS_GetChannelInfo( IQV_GPS_CHANNEL_NUMBER channelNumber, int *satelliteNumber, double *powerDbm, IQV_GPS_MODULATION_STATE *modulationState)
{
	int err  = ERR_OK;

	functionName = "LP_GPS_GetChannelInfo";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);


	err = hndl->hndlGps->GetTx();
	hndl->hndlGps->GetParamPowerDbm(powerDbm, channelNumber);
	hndl->hndlGps->GetParamModulationState(modulationState, channelNumber);
	hndl->hndlGps->GetParamSatelliteNumber(satelliteNumber, channelNumber);


	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_GPS_SetChannelInfo( IQV_GPS_CHANNEL_NUMBER channelNumber, int satelliteNumber, double powerDbm, IQV_GPS_MODULATION_STATE modulationState)
{
	int err  = ERR_OK;

	functionName = "LP_GPS_SetChannelInfo";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);


	hndl->hndlGps->SetParamPowerDbm(powerDbm, channelNumber);
	hndl->hndlGps->SetParamModulationState(modulationState, channelNumber);
	hndl->hndlGps->SetParamSatelliteNumber(satelliteNumber, channelNumber);
	err = hndl->hndlGps->SetTx(false);


	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

/*
   IQMEASURE_API int LP_GPS_GetData(char* inputName, char* &inputValue){
   functionName = "LP_GPS_GetData";
   ::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);
   int err  = IQAPI_ERR_OK;
////////--------
LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_ERROR, "[IQMEASURE],[%s]\n", functionName);
//--
hndl->hndlGps->GetTx();
char* value = "";
char* temp = "";
int value_i = 0;
hndl->hndlGps->GetHardwareVersion(value, 128);
inputValue = value;
//hndl->hndlGps->GetSoftwareVersion(value, 128);
//hndl->hndlGps->GetDriverVersion(value, 128);
//hndl->hndlGps->GetSerialNumber(value, 9);
//hndl->hndlGps->GetNumberOfChannel(&value_i);
//GPSData[ "GetNumberOfChannel" ] = value;
//hndl->hndlGps->GetParamRfOutput(&rfOutput);
//--
////////--------
::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);
return err;
}
*/

IQMEASURE_API int LP_PrintGlonassParameters(bool forceUpdate)
{
	int err = 0;

	functionName = "LP_PrintGlonassParameters";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	if (forceUpdate)
	{
		err = hndl->hndlGlonass->GetTx();
		if (err)
		{
			printf("Failed to retrieve parameters from a tester!\r\n");
			return err;
		}
	}

	int doppler;
	double powerLevel;
	IQV_GLONASS_OPERATION_MODE operationMode;
	IQV_GLONASS_RF_OUTPUT rfOutput;
	IQV_GLONASS_FREQUENCY_CHANNEL frequencyChannel = IQV_GLONASS_FREQUENCY_CHANNEL_1602000000Hz;

	hndl->hndlGlonass->GetParamDopplerFrequency(&doppler);
	hndl->hndlGlonass->GetParamOperationMode(&operationMode);
	hndl->hndlGlonass->GetParamPowerDbm(&powerLevel);
	hndl->hndlGlonass->GetParamRfOutput(&rfOutput);
	hndl->hndlGlonass->GetParamFrequencyChannel(&frequencyChannel);

	char operationModeStr[32];
	char rfOutputStr[32];
	char frequencyChannelStr[32];

	switch (operationMode)
	{
		case IQV_OPERATION_MODE_CW:
			strcpy_s(operationModeStr, "CW");
			break;
		case IQV_OPERATION_MODE_MODULATION:
			strcpy_s(operationModeStr, "Modulation");
			break;
		case IQV_OPERATION_MODE_MODULATION_NAV_DATA_OFF:
			strcpy_s(operationModeStr, "Modulation Nav Data Off");
			break;
		case IQV_OPERATION_MODE_MODULATION_OFF:
			strcpy_s(operationModeStr, "Modulation Off");
			break;
		default:
			strcpy_s(operationModeStr, "Unknown");
			break;
	}
	switch (frequencyChannel)
	{
		case IQV_GLONASS_FREQUENCY_CHANNEL_1598062500Hz :
			strcpy_s(frequencyChannelStr, "1598.062500 MHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1598625000Hz :
			strcpy_s(frequencyChannelStr, "1598.625000 MHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1599187500Hz :
			strcpy_s(frequencyChannelStr, "1599.187500 MHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1599750000Hz :
			strcpy_s(frequencyChannelStr, "1599.750000 MHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1600312500Hz :
			strcpy_s(frequencyChannelStr, "1600.312500 MHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1600875000Hz :
			strcpy_s(frequencyChannelStr, "1600.875000 MHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1601437500Hz :
			strcpy_s(frequencyChannelStr, "1601.437500 MHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1602000000Hz :
			strcpy_s(frequencyChannelStr, "1602.000000 KHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1602562500Hz :
			strcpy_s(frequencyChannelStr, "1602.562500 KHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1603125000Hz :
			strcpy_s(frequencyChannelStr, "1603.125000 KHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1603687500Hz :
			strcpy_s(frequencyChannelStr, "1603.687500 KHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1604250000Hz :
			strcpy_s(frequencyChannelStr, "1604.250000 KHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1604812500Hz :
			strcpy_s(frequencyChannelStr, "1604.812500 KHz");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1605375000Hz :
			strcpy_s(frequencyChannelStr, "1605.375000 KHz");
			break;
		default:
			strcpy_s(frequencyChannelStr, "Unknown");
			break;
	}

	strcpy_s(rfOutputStr, rfOutput == IQV_RF_OUTPUT_ON ? "ON" : "OFF");


	printf("\r\n");
	printf(" - Doppler Frequency   : %d\r\n"
			" - Operation Mode      : %s\r\n"
			" - RF Output           : %s\r\n"
			" - Power               : %f\r\n"
			" - Signal Frequency    : %s\r\n\r\n",
			doppler, operationModeStr, rfOutputStr, powerLevel, frequencyChannelStr);
	
	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_Glonass_SetOperationMode(IQV_GPS_OPERATION_MODE operationMode)
{
	int err = 0;

	functionName = "LP_Glonass_SetOperationMode";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	printf("[Frequency Channel] : ");

	switch (operationMode)
	{
		case IQV_OPERATION_MODE_CW:
			printf("CW ... ");
			break;
		case IQV_OPERATION_MODE_MODULATION:
			printf("Modulation ... ");
			break;
		case IQV_OPERATION_MODE_MODULATION_NAV_DATA_OFF:
			printf("Modulation Nav Data Off ... ");
			break;
		case IQV_OPERATION_MODE_MODULATION_OFF:
			printf("Modulation Off ... ");
			break;
		default:
			printf("Unknown\r\n");
			return 1;
	}

	hndl->hndlGlonass->SetParamOperationMode(operationMode);
	err = hndl->hndlGlonass->SetTx();

	if (err)
		printf("FAILED\r\n");
	else
		printf("OK\r\n");
	
	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_Glonass_SetPower(double powerdBm)
{
	printf("[Power Level] : %f ... ", powerdBm);

	functionName = "LP_Glonass_SetPower";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	int err = hndl->hndlGlonass->SetParamPowerDbm(powerdBm);
	if (err == IQAPI_ERR_OK)
		err = hndl->hndlGlonass->SetTx();

	if (err)
		printf("FAILED\r\n");
	else
		printf("OK\r\n");
	
	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_Glonass_SetFrequency(int frequencyChannel)
{
	printf("[Frequency Channel] : ");

	functionName = "LP_Glonass_SetFrequency";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	int err = 0;

	switch (frequencyChannel)
	{
		case IQV_GLONASS_FREQUENCY_CHANNEL_1598062500Hz :
			printf("1598.062500 MHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1598625000Hz :
			printf("1598.625000 MHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1599187500Hz :
			printf("1599.187500 MHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1599750000Hz :
			printf("1599.750000 MHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1600312500Hz :
			printf("1600.312500 MHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1600875000Hz :
			printf("1600.875000 MHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1601437500Hz :
			printf("1601.437500 MHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1602000000Hz :
			printf("1602.000000 KHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1602562500Hz :
			printf("1602.562500 KHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1603125000Hz :
			printf("1603.125000 KHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1603687500Hz :
			printf("1603.687500 KHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1604250000Hz :
			printf("1604.250000 KHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1604812500Hz :
			printf("1604.812500 KHz ... ");
			break;
		case IQV_GLONASS_FREQUENCY_CHANNEL_1605375000Hz :
			printf("1605.375000 KHz ... ");
			break;
		default:
			printf("Unknown");
			return 1;
	}

	err = hndl->hndlGlonass->SetParamFrequencyChannel( (IQV_GLONASS_FREQUENCY_CHANNEL) frequencyChannel);
	if (err == IQAPI_ERR_OK)
		err = hndl->hndlGlonass->SetTx();

	if (err)
		printf("FAILED\r\n");
	else
		printf("OK\r\n");
	
	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_Glonass_SetRfOutput(IQV_GPS_RF_OUTPUT rfOutput)
{
	printf("[RF Output] : %s", rfOutput == ::IQV_RF_OUTPUT_ON ? "ON ... " : "OFF ... ");

	functionName = "LP_Glonass_SetRfOutput";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	hndl->hndlGlonass->SetParamRfOutput(rfOutput);
	int err = hndl->hndlGlonass->SetTx();

	if (err)
		printf("FAILED\r\n");
	else
		printf("OK\r\n");
	
	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_Glonass_SetActivate(void)
{
	int err  = ERR_OK;

	functionName = "LP_Glonass_SetActivate";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	hndl->SetDefault();
	err = hndl->hndlGlonass->Activate();

	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

IQMEASURE_API int LP_Glonass_SetDeactivate()
{
	int err  = ERR_OK;

	functionName = "LP_Glonass_SetDeactivate";
	::TIMER_StartTimer(timerIQmeasure, functionName, &timeStart);

	err = hndl->hndlGlonass->Deactivate();

	::TIMER_StopTimer(timerIQmeasure, functionName, &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", functionName, timeDuration);

	return err;
}

#pragma endregion
#endif //WIN32  //Not using FM for Mac at this time
IQMEASURE_API int LP_GetCapture(int    dut,
		int    captureIndex,
		double *real[],
		double *imag[],
		int    length[])
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_GetCapture", &timeStart);

	if (LibsInitialized)
	{
		// g_userData takes priority
		if( g_userData!=NULL )
		{
			for(int i=0; i<N_MAX_TESTERS; i++)
			{
				real[i]   = g_userData->real[i];
				imag[i]   = g_userData->imag[i];
				length[i] = g_userData->length[i];
			}
		}
		else
		{
			for(int i=0; i<N_MAX_TESTERS; i++)
			{
				real[i]   = hndl->data->real[i];
				imag[i]   = hndl->data->imag[i];
				length[i] = hndl->data->length[i];
			}
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_GetCapture", &timeDuration, NULL);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_GetCapture", timeDuration);
	return err;
}

IQMEASURE_API int LP_EnableMultiThread(int enableMultiThread, int numberOfThreads)
{
	int err = ERR_OK;
	int lastSetThreadNumber;
	int forceSingleThread = 0;

	if (LibsInitialized)
	{
		if (hndl)
		{
			if(1 == enableMultiThread)
			{
				err = iqapiSetCompThreadNumber(hndl, numberOfThreads, &lastSetThreadNumber);
			}
			else
			{
				forceSingleThread = 1;
				err = iqapiSetCompThreadNumber(hndl, forceSingleThread, &lastSetThreadNumber);
			}
		}
		else
		{
			// no hndl means tester not initialized or hndl deleted
			err = ERR_SET_MULTI_THREAD_FAILED;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	return err;
}

IQMEASURE_API int LP_SetUserDataPtr(void *userData)
{
	int err = ERR_OK;
	g_userData  = (iqapiCapture *)userData;
	return err;
}

IQMEASURE_API int LP_SetAlcMode(IQAPI_ALC_MODES alcMode)
{
	hndl->tx->alcMode = (IQV_ALC_MODES) alcMode;
	return ERR_OK;
}

IQMEASURE_API int LP_GetAlcMode(IQAPI_ALC_MODES* alcMode)
{
	alcMode = (IQAPI_ALC_MODES*) &hndl->tx->alcMode;
	return ERR_OK;
}

IQMEASURE_API int LP_IQ2010EXT_Init(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_Init", &timeStart);

	if (LibsInitialized)
	{
		// Initialize IQ2010 Extension
		err = IQ2010EXT_Init(hndl);
		if(err!=ERR_OK)
		{
			bIQ201xFound = false;
		}
		else
		{
			bIQ201xFound = true;
		}
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_Init", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_Init", timeDuration);

	return err;
}

IQMEASURE_API int LP_IQ2010EXT_Terminate(void)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_IQ2010EXT_Terminate", &timeStart);

	if (LibsInitialized && bIQ201xFound)
	{
		// Exit IQ2010 Extension
		err = IQ2010EXT_Terminate();
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_IQ2010EXT_Terminate", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_IQ2010EXT_Terminate", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetDefaultNfc()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetDefaultNfc", &timeStart);

	if (LibsInitialized)
	{
		err = hndl->SetDefaultNfc();
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SetDefaultNfc", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetDefaultNfc", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetNfcTxAmplitude(IQV_USER_SIGNAL_AMPLITUDE_TYPE userSignalAmplitudeType, double userSignalAmplitude)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetNfcTxAmplitude", &timeStart);

	if (LibsInitialized)
	{
		hndl->txNfc->userSignalAmplitudeType = userSignalAmplitudeType;
		hndl->txNfc->userSignalAmplitude = userSignalAmplitude;
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SetNfcTxAmplitude", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetNfcTxAmplitude", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetTriggerMaskNfc()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_TriggerMaskNfc", &timeStart);

	if (LibsInitialized)
	{
		err = hndl->SetTriggerMaskNfc();
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_TriggerMaskNfc", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_TriggerMaskNfc", timeDuration);

	return err;

}
IQMEASURE_API int LP_InitiateNfc(char *fileName)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_InitiateNfc", &timeStart);

	if (LibsInitialized)
	{
		err = hndl->InitiateNfc(fileName);
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_InitiateNfc", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_InitiateNfc", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetRxNfc(double triggerTimeout, double sampleingTimeSecs)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetRxNfc", &timeStart);

	if (LibsInitialized)
	{
		hndl->rxNfc->triggerTimeOut = triggerTimeout;
		hndl->rxNfc->samplingTimeSecs = sampleingTimeSecs;
		err = hndl->SetRxNfc();
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SetRxNfc", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetRxNfc", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetTxNfc()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetTxNfc", &timeStart);

	if (LibsInitialized)
	{
		err = hndl->SetTxNfc();
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_SetTxNfc", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetTxNfc", timeDuration);

	return err;
}

IQMEASURE_API int LP_ContCapture(IQV_DC_CONT_ENUM dcContEnum)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_ContCapture", &timeStart);

	if (LibsInitialized)
	{
		err = hndl->ContCapture(dcContEnum);
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_ContCapture", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_ContCapture", timeDuration);

	return err;
}

IQMEASURE_API int LP_TerminateNfc()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_TerminateNfc", &timeStart);

	if (LibsInitialized)
	{
		err = hndl->TerminateNfc();
	}
	else
	{
		err = ERR_NOT_INITIALIZED;
	}

	::TIMER_StopTimer(timerIQmeasure, "LP_TerminateNfc", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_TerminateNfc", timeDuration);

	return err;
}

IQMEASURE_API int LP_AnalysisNfc()
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_AnalysisNfc", &timeStart);

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

		analysisNfc->analysisPhyDecodeTransportMsg = true;

		analysisNfc->analysisPhyDoRfAnalysis = true;
		analysisNfc->analysisPhyPlotRawSigPsd = true;

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisNfc);

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

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalysisNfc", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalysisNfc", timeDuration);

	return err;
}

IQMEASURE_API int LP_SetVsgModulation_SetPlayCondition(char *modFileName, bool autoPlay,int loadInternalWaveform)
{
	int err = ERR_OK;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsgModulation_SetPlayCondition", &timeStart);

	if (LibsInitialized)
	{
		if (hndl->SetWave(modFileName, autoPlay))
		{
			err = ERR_SET_WAVE_FAILED;
		}
		else
		{
			// Mark as single MOD file mod
			g_vsgMode = ::VSG_SINGLE_MOD_FILE;
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

IQMEASURE_API int LP_SetVsaBluetooth_BTShiftHz(double rfFreqHz, double rfAmplDb, int port,double extAttenDb, double triggerLevelDb, double triggerPreTime, double btShiftHz)
{
	int    err = ERR_OK;
	bool   setupChanged = false;
	bool   RFused = true;

	::TIMER_StartTimer(timerIQmeasure, "LP_SetVsaBluetooth_BTShiftHz", &timeStart);

	if (LibsInitialized)
	{
		if (hndl->rx->rfFreqHz != (rfFreqHz+btShiftHz) )
		{
			hndl->rx->rfFreqHz = rfFreqHz + btShiftHz;
			hndl->tx->rfFreqHz = rfFreqHz + btShiftHz;
			hndl->rx->freqShiftHz = btShiftHz;
			hndl->tx->freqShiftHz = -btShiftHz;

			//hndl->rx->rfFreqHz = rfFreqHz;
			//hndl->tx->rfFreqHz = rfFreqHz;
			//hndl->rx->freqShiftHz = 0;
			//hndl->tx->freqShiftHz = 0;

			setupChanged = true;
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

			// -cfy@sunnyvale, 2012/3/13-
			//if ( abs(hndl->rx->vsa[i]->rfAmplDb-rfAmplDb)>abs(g_amplitudeToleranceDb) )
			//if (hndl->rx->vsa[i]->rfAmplDb != rfAmplDb)
			bool rfAmplChanged = false;
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
			/* <><~~ */
		}

		if (setupChanged)
		{
			if (hndl->SetTxRx())
				err = ERR_SET_RX_FAILED;
		}
	}
	else
		err = ERR_NOT_INITIALIZED;

	::TIMER_StopTimer(timerIQmeasure, "LP_SetVsaBluetooth_BTShiftHz", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_SetVsaBluetooth_BTShiftHz", timeDuration);

	return err;
}


/* IQMEASURE_API int LP_TerminateAnalysisNfc()
   {
   delete analysisNfc;
   analysisNfc = NULL;

   }

   IQMEASURE_API int LP_ResultsAnalysisNfc()
   {
   PrintResult(hndl->results, NFC_e);
   }

   IQMEASURE_API int LP_ClearTriggerMaskNfc()
   {
   hndl->ClearTriggerMaskNfc();
   return err;
   }
   */

//Adding FM VSA Auto Range function
IQMEASURE_API int LP_FM_SetVsa_Agc_On(unsigned int carrierFreqHz, double expectedPeakInputPowerdBm)
{

	int err = ERR_OK;

	double marginalInputPowerdBm = 0;//currently no margin is internally applied to VSA Gain settings. The user needs to set the value.

	::TIMER_StartTimer(timerIQmeasure, "LP_FM_SetVsa", &timeStart);

	if(FmInitialized)
	{
		int agcMode = ON;
		int vsaSampleRate = SAMPLE_640KHz;
		//double expectedPeakInputPowerdBmSetting;

		try
		{

			err = hndl->hndlFm->Activate(IQV_FM_VSA_TECH_ACT);
			if (IQAPI_ERR_OK != err) throw err;

			//hndl->hndlFm->SetParamVsaPowerDbm(-40);
			//err = hndl->hndlFm->SetRx();
			//err = hndl->hndlFm->GetRx();
			double vsaPowerLeveldBm;
			hndl->hndlFm->GetParamVsaPowerDbm(&vsaPowerLeveldBm);

			hndl->hndlFm->SetParamVsaSampleRate((IQV_FM_VSA_SAMPLE_RATES)vsaSampleRate);
			hndl->hndlFm->SetParamVsaFrequency(carrierFreqHz);

			hndl->hndlFm->SetParamVsaAgcMode((IQV_FM_VSA_AGC_MODES)agcMode);
			err = hndl->hndlFm->SetRx();
			err = hndl->hndlFm->GetRx();
			IQV_FM_VSA_AGC_MODES agcModeStatus;
			hndl->hndlFm->GetParamVsaAgcMode(&agcModeStatus);

			hndl->hndlFm->SetParamVsaRfInput(IQV_FM_VSA_RF_INPUT_ON);
			err = hndl->hndlFm->SetRx();
			err = hndl->hndlFm->GetRx();

			Sleep(200);
			hndl->hndlFm->Capture(100);
			err = hndl->hndlFm->GetRx();
			hndl->hndlFm->GetParamVsaPowerDbm(&vsaPowerLeveldBm);


			hndl->hndlFm->SetParamVsaPowerDbm(vsaPowerLeveldBm);
			hndl->hndlFm->SetParamVsaPowerDbm(-15);
			//	hndl->hndlFm->SetParamVsaAgcMode((IQV_FM_VSA_AGC_MODES)OFF); // turning off AGC
			err = hndl->hndlFm->SetRx();
			//err = hndl->hndlFm->GetRx();
			//hndl->hndlFm->GetParamVsaPowerDbm(&vsaPowerLeveldBm);
			if (IQAPI_ERR_OK != err) throw err;
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



//IQMEASURE_API int LP_ResetToken()
//{
//	int err = ERR_OK;
//	if (LibsInitialized)
//	{
//		hndl->pDualHead->TokenReset();
//	}
//	else
//		err = ERR_NOT_INITIALIZED;
//	return err;
//}


void createSpectrumMask()
{
	int ind, ind1;

	//Get raw spectrum mask for OFDM and 11b
	//rawSpectrumMask_11b = (double *)malloc(FFT_Length * sizeof(double));
	//rawSpectrumMask_Zigbee = (double *)malloc(FFT_Length * sizeof(double));
	//rawSpectrumMask_BT = (double *)malloc(FFT_Length * sizeof(double));
	//rawSpectrumMask_BT_EDR = (double *)malloc(FFT_Length * sizeof(double));

	rawSpectrumMask_11b = new double [FFT_Length];
	rawSpectrumMask_Zigbee = new double [FFT_Length];
	rawSpectrumMask_BT = new double [FFT_Length];
	rawSpectrumMask_BT_EDR = new double [FFT_Length];

	//Get raw spectrum mask for 11b
	//---------------------------------------------------
	for (ind = 0; ind < FFT_Half_Length; ind++)
	{
		if (ind * FFT_Freq_Step <= 11.0e6)
			rawSpectrumMask_11b[ind + FFT_Half_Length] = 0;
		else if (ind * FFT_Freq_Step <= 22.0e6)
			rawSpectrumMask_11b[ind + FFT_Half_Length] = -30;
		else rawSpectrumMask_11b[ind + FFT_Half_Length] = -50.0;
	}

	//Get the other side
	ind1 = 0;
	for (ind = FFT_Length - 1; ind >= FFT_Half_Length; ind--)
	{
		rawSpectrumMask_11b[ind1] = rawSpectrumMask_11b[ind];
		ind1++;
	}

	//Get raw spectrum mask for Zigbee signal
	for (ind = 0; ind < FFT_Half_Length; ind++)
	{
		if (ind * FFT_Freq_Step <= 3.5e6)
			rawSpectrumMask_Zigbee[ind + FFT_Half_Length] = 0;
		else rawSpectrumMask_Zigbee[ind + FFT_Half_Length] = -20.0;
	}

	//Get the other side
	ind1 = 0;
	for (ind = FFT_Length - 1; ind >= FFT_Half_Length; ind--)
	{
		rawSpectrumMask_Zigbee[ind1] = rawSpectrumMask_Zigbee[ind];
		ind1++;
	}

	//Get raw spectrum mask for Bluetooth signal --> 1M
	for (ind = 0; ind < FFT_Half_Length; ind++)
	{
		//if (ind * FFT_Freq_Step <= 1e6)
		//        rawSpectrumMask_BT[ind + FFT_Half_Length] = 0;
		//else rawSpectrumMask_BT[ind + FFT_Half_Length] = -26.0;
		if (ind * FFT_Freq_Step <= 0.5e6)
			rawSpectrumMask_BT[ind + FFT_Half_Length] = 0;
		else if (ind * FFT_Freq_Step <= 2.5e6)
			rawSpectrumMask_BT[ind + FFT_Half_Length] = -20;
		else rawSpectrumMask_BT[ind + FFT_Half_Length] = -30.0;
	}

	//Get the other side
	ind1 = 0;
	for (ind = FFT_Length - 1; ind >= FFT_Half_Length; ind--)
	{
		rawSpectrumMask_BT[ind1] = rawSpectrumMask_BT[ind];
		ind1++;
	}

	//Get raw spectrum mask for Bluetooth signal --> EDR
	for (ind = 0; ind < FFT_Half_Length; ind++)
	{
		if (ind * FFT_Freq_Step <= 1e6)
			rawSpectrumMask_BT_EDR[ind + FFT_Half_Length] = 0;
		else if (ind * FFT_Freq_Step <= 2.5e6)
			rawSpectrumMask_BT_EDR[ind + FFT_Half_Length] = -26;
		else rawSpectrumMask_BT_EDR[ind + FFT_Half_Length] = -40.0;
	}

	//Get the other side
	ind1 = 0;
	for (ind = FFT_Length - 1; ind >= FFT_Half_Length; ind--)
	{
		rawSpectrumMask_BT_EDR[ind1] = rawSpectrumMask_BT_EDR[ind];
		ind1++;
	}

	//Index:0, OFDM 11g/a
	SpecMask.cutoff_mhz[0]     = 9.0;
	SpecMask.stopband_a_mhz[0] = 11.0;
	SpecMask.stopband_b_mhz[0] = 20.0;
	SpecMask.stopband_c_mhz[0] = 30.0;

	SpecMask.cutoff_dbr[0]     = 0;
	SpecMask.stopband_a_dbr[0] = -20.0;
	SpecMask.stopband_b_dbr[0] = -28.0;
	SpecMask.stopband_c_dbr[0] = -40.0;

	//Index:1, 11n MIMO HT20
	SpecMask.cutoff_mhz[1]     = 9.0;
	SpecMask.stopband_a_mhz[1] = 11.0;
	SpecMask.stopband_b_mhz[1] = 20.0;
	SpecMask.stopband_c_mhz[1] = 30.0;

	SpecMask.cutoff_dbr[1]     = 0;
	SpecMask.stopband_a_dbr[1] = -20.0;
	SpecMask.stopband_b_dbr[1] = -28.0;
	SpecMask.stopband_c_dbr[1] = -45.0;

	//Index:2, 11n MIMO HT40
	SpecMask.cutoff_mhz[2]     = 19.0;
	SpecMask.stopband_a_mhz[2] = 21.0;
	SpecMask.stopband_b_mhz[2] = 40.0;
	SpecMask.stopband_c_mhz[2] = 60.0;

	SpecMask.cutoff_dbr[2]     = 0;
	SpecMask.stopband_a_dbr[2] = -20.0;
	SpecMask.stopband_b_dbr[2] = -28.0;
	SpecMask.stopband_c_dbr[2] = -45.0;

	//Index:3, WiMAX 10M
	SpecMask.cutoff_mhz[3]     = 4.75;
	SpecMask.stopband_a_mhz[3] = 5.45;
	SpecMask.stopband_b_mhz[3] = 9.75;
	SpecMask.stopband_c_mhz[3] = 14.75;

	SpecMask.cutoff_dbr[3]     = 0;
	SpecMask.stopband_a_dbr[3] = -25.0;
	SpecMask.stopband_b_dbr[3] = -32.0;
	SpecMask.stopband_c_dbr[3] = -50.0;

	//Index:4, WiMAX 7M
	SpecMask.cutoff_mhz[4]     = 3.325;
	SpecMask.stopband_a_mhz[4] = 3.815;
	SpecMask.stopband_b_mhz[4] = 6.825;
	SpecMask.stopband_c_mhz[4] = 10.325;

	SpecMask.cutoff_dbr[4]     = 0;
	SpecMask.stopband_a_dbr[4] = -25.0;
	SpecMask.stopband_b_dbr[4] = -32.0;
	SpecMask.stopband_c_dbr[4] = -50.0;

	//Index:10, Customized WiMAX 5M
	SpecMask.cutoff_mhz[10]     = 2.5 + 0;
	SpecMask.stopband_a_mhz[10] = 2.5 + 1;
	SpecMask.stopband_b_mhz[10] = 2.5 + 5.5;
	SpecMask.stopband_c_mhz[10] = 2.5 + 15;

	SpecMask.cutoff_dbr[10]     = 0;
	SpecMask.stopband_a_dbr[10] = -13.0;
	SpecMask.stopband_b_dbr[10] = -26.0;
	SpecMask.stopband_c_dbr[10] = -38.0;

	//Index:11, Customized WiMAX 10M
	SpecMask.cutoff_mhz[11]     = 5 + 0;
	SpecMask.stopband_a_mhz[11] = 5 + 1;
	SpecMask.stopband_b_mhz[11] = 5 + 5.5;
	SpecMask.stopband_c_mhz[11] = 5 + 15;

	SpecMask.cutoff_dbr[11]     = 0;
	SpecMask.stopband_a_dbr[11] = -13.0;
	SpecMask.stopband_b_dbr[11] = -23.0;
	SpecMask.stopband_c_dbr[11] = -35.0;
}

IQMEASURE_API int LP_Analyze80211n_Seq(char *type,
		int enablePhaseCorr,
		int enableSymTimingCorr,
		int enableAmplitudeTracking,
		int decodePSDU,
		int enableFullPacketChannelEst,
		char *referenceFile,
		int numSections,
		double sectionLenSec)
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

		if( NULL==analysisMimo )
		{
			InstantiateAnalysisClasses();
		}

		analysisMimo->type = type;
		analysisMimo->mode = "sequential_mimo";
		analysisMimo->enablePhaseCorr = enablePhaseCorr;
		analysisMimo->enableSymTimingCorr = enableSymTimingCorr;
		analysisMimo->enableAmplitudeTracking = enableAmplitudeTracking;
		analysisMimo->decodePSDU = decodePSDU;
		analysisMimo->enableFullPacketChannelEst = enableFullPacketChannelEst;
		analysisMimo->referenceFile = referenceFile;

		// for "sequential_mimo" mode
		analysisMimo->SequentialMimo.numSections = numSections;
		analysisMimo->SequentialMimo.sectionLenSec = sectionLenSec;
		analysisMimo->SequentialMimo.interSectionGapSec = 0;

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
		err = ERR_NOT_INITIALIZED;

	return err;
}

double iq_interpolate (double y_max, double y_min, double x_max, double x_min, double x_val)
	//=============================================================================
	//	Description:
	//		A generic function for interpolating. Find y_val when x_val is present.
	//
	//                             (y_max - y_min)
	//    Returns y_val = y_min + (--------------- * (x_val - x_min))
	//                             (x_max - x_min)
	//=============================================================================
{
	return (y_min + ((y_max - y_min) / (x_max - x_min) * (x_val - x_min)));
}

bool calu_spectral_mask_spec(	int points, double max_power, double freqs_mhz[],
		int offset_num, double offsets_mhz[], double levels_dbr[],
		double spec_trace_dbr[] )
{
	// ****************************************************************************
	//	This method will build a spec array for every point, and return the array.
	// ****************************************************************************
	bool error=false;
	int i, j;
	double *spec_mhz, *spec_dbc;

	spec_mhz	= new double [ offset_num + 2 ];
	spec_dbc	= new double [ offset_num + 2 ];

	spec_mhz[0] = 0 ;
	spec_dbc[0] = levels_dbr[0] ;
	for ( i=1, j=0; i<=offset_num ; i++ ,j++ )
	{
		if ( offsets_mhz[j] == 0 )
			j-- ;
		spec_mhz[i] = offsets_mhz[j];
		spec_dbc[i] = levels_dbr [j];
	}
	spec_mhz[offset_num+1] = 99e3 ;
	spec_dbc[offset_num+1] = levels_dbr[j-1] ;

	int pt, rv, seg;
	double y_max=0, y_min=0, x_max=0, x_min=0, freq;

	for ( pt=0, rv=points-1 ; pt < (points/2) ; pt++, rv-- )
	{
		freq = freqs_mhz[rv];
		for ( seg=0 ; seg < (offset_num+1) ; seg++ )
		{
			if ( (freq >= spec_mhz[seg]) && (freq < spec_mhz[seg+1]) )
			{
				x_max = spec_mhz[seg+1];	// stop  freq
				x_min = spec_mhz[seg];		// start freq
				y_max = spec_dbc[seg+1];	// stop  dbr
				y_min = spec_dbc[seg];		// start dbr
			}
		}
		spec_trace_dbr[rv] = iq_interpolate ( y_max, y_min, x_max, x_min, freq ) + max_power ;
		spec_trace_dbr[pt] = spec_trace_dbr[rv];		// mirror image
	}

	free (spec_mhz);
	free (spec_dbc);

	return (error);
}


//Get spectral mask result
IQMEASURE_API int LP_GetResultSpectralMask(int bandwidth_index, double *ViolationPercent, int *length, double *freqs_MHz, double *trace_dBm, double *mask)
{
	double  ref_level;
	int n_violations = 0;
	int i;
	int fft_length;
	int offset_num = 4;
	double offsets_mhz[7], levels_dbr[7];

	*ViolationPercent = 100.0;

	if (bandwidth_index < 20)      //not 11b
	{
		offsets_mhz[0] = SpecMask.cutoff_mhz[bandwidth_index];
		levels_dbr[0]  = SpecMask.cutoff_dbr[bandwidth_index];

		offsets_mhz[1] = SpecMask.stopband_a_mhz[bandwidth_index];
		levels_dbr[1]  = SpecMask.stopband_a_dbr[bandwidth_index];
		offsets_mhz[2] = SpecMask.stopband_b_mhz[bandwidth_index];
		levels_dbr[2]  = SpecMask.stopband_b_dbr[bandwidth_index];
		offsets_mhz[3] = SpecMask.stopband_c_mhz[bandwidth_index];
		levels_dbr[3]  = SpecMask.stopband_c_dbr[bandwidth_index];
		/*
		   offsets_mhz[4] = SpecMask.stopband_d_mhz[bandwidth_index];
		   levels_dbr[4]  = SpecMask.stopband_d_dbr[bandwidth_index];
		   offsets_mhz[5] = SpecMask.stopband_e_mhz[bandwidth_index];
		   levels_dbr[5]  = SpecMask.stopband_e_dbr[bandwidth_index];
		   offsets_mhz[6] = SpecMask.stopband_f_mhz[bandwidth_index];
		   levels_dbr[6]  = SpecMask.stopband_f_dbr[bandwidth_index];
		   */
	}

	//FFT Analysis
	LP_AnalyzeFFT(0, 100e3, "hanning");

	fft_length = (int)LP_GetScalarMeasurement("length", 0);
	if (fft_length)
	{
		*length = fft_length;

		LP_GetVectorMeasurement("x", freqs_MHz, NULL, fft_length);
		LP_GetVectorMeasurement("y", trace_dBm, NULL, fft_length);

		for (i = 0; i < fft_length; i++)
		{
			freqs_MHz[i] = freqs_MHz[i] / 1e06;
		}

		// Search for the max. power level to use as ref_level.
		ref_level = trace_dBm[0];
		for (i = 1; i < fft_length; i++)
		{
			if (trace_dBm[i] > ref_level)
				ref_level = trace_dBm[i];
		}

		// Create the appropriate mask to be used for the test.
		if (bandwidth_index < 20)      //not 11b
			calu_spectral_mask_spec(fft_length, ref_level, freqs_MHz, offset_num, offsets_mhz, levels_dbr, mask);
		else if (bandwidth_index == SPECTRAL_MASK_11b)   //11b
		{
			for (i = 0; i < FFT_Length; i++)
				mask[i] = ref_level + rawSpectrumMask_11b[i];
		}
		else if (bandwidth_index == SPECTRAL_MASK_ZIGBEE)   //Zigbee
		{
			for (i = 0; i < FFT_Length; i++)
				mask[i] = ref_level + rawSpectrumMask_Zigbee[i];
		}
		else if (bandwidth_index == SPECTRAL_MASK_BT)   //bluetooth 1M
		{
			for (i = 0; i < FFT_Length; i++)
				mask[i] = ref_level + rawSpectrumMask_BT[i];
		}
		else if (bandwidth_index == SPECTRAL_MASK_BT_EDR)   //bluetooth EDR
		{
			for (i = 0; i < FFT_Length; i++)
				mask[i] = ref_level + rawSpectrumMask_BT_EDR[i];
		}

		// Compare the generated mask with the PSD data - sample by sample.
		// Count violations where the mask is below the PSD data value.
		for (i = 0; i < fft_length; i++)
		{
			if ( trace_dBm[i] > mask[i] )
				n_violations++;
		}

		*ViolationPercent = ((double)n_violations / (double)fft_length ) * 100;
	}

	return IQAPI_ERR_OK;
}

IQMEASURE_API int LP_GetResultSpectralMask_HT40(double *ViolationPercent, int *length, double *freqs_MHz, double *trace_dBm, double *mask)
{
	double  ref_level;
	int n_violations = 0;
	int i;
	int fft_length;	//=6144;
	int offset_num = 4;
	double offsets_mhz[7], levels_dbr[7];

	*ViolationPercent = 100.0;

	offsets_mhz[0] = SpecMask.cutoff_mhz[2];
	levels_dbr[0]  = SpecMask.cutoff_dbr[2];

	offsets_mhz[1] = SpecMask.stopband_a_mhz[2];
	levels_dbr[1]  = SpecMask.stopband_a_dbr[2];
	offsets_mhz[2] = SpecMask.stopband_b_mhz[2];
	levels_dbr[2]  = SpecMask.stopband_b_dbr[2];
	offsets_mhz[3] = SpecMask.stopband_c_mhz[2];
	levels_dbr[3]  = SpecMask.stopband_c_dbr[2];

	fft_length = (int)LP_GetScalarMeasurement("length", 0);
	if (fft_length)
	{
		*length = fft_length;

		if (*length % 2)
		{
			*length -= 1;
		}

		LP_GetVectorMeasurement("x", freqs_MHz, NULL, fft_length);
		LP_GetVectorMeasurement("y", trace_dBm, NULL, fft_length);

		for (i = 0; i < fft_length; i++)
		{
			freqs_MHz[i] = freqs_MHz[i] / 1e06;
		}

		// Search for the max. power level to use as ref_level.
		ref_level = trace_dBm[0];
		for (i = 1; i < fft_length; i++)
		{
			if (trace_dBm[i] > ref_level)
				ref_level = trace_dBm[i];
		}


		//calu_spectral_mask_spec(fft_length, ref_level, freqs_MHz, offset_num, offsets_mhz, levels_dbr, mask);
		calu_spectral_mask_spec(*length, ref_level, freqs_MHz, offset_num, offsets_mhz, levels_dbr, mask);


		// Compare the generated mask with the PSD data - sample by sample.
		// Count violations where the mask is below the PSD data value.
		for (i = 0; i < fft_length; i++)
		{
			if ( trace_dBm[i] > mask[i] )
				n_violations++;
		}

		*ViolationPercent = ((double)n_violations / (double)fft_length ) * 100;
	}
	else
		return IQAPI_GENERAL_ERROR;

	return IQAPI_ERR_OK;
}

IQMEASURE_API int LP_AnalyzeObw(double obwPercentage)	//0 <= obwPercentage <= 1
{
	int err = ERR_OK;


	::TIMER_StartTimer(timerIQmeasure, "LP_AnalyzeObw", &timeStart);

	if (LibsInitialized)
	{
		analysisObw->obwPercentage = obwPercentage;
		analysisObw->vsaNum = IQV_VSA_NUM_1;

		hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisObw);
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

	::TIMER_StopTimer(timerIQmeasure, "LP_AnalyzeObw", &timeDuration, &timeStop);
	::LOGGER_Write_Ext(LOG_IQMEASURE, loggerIQmeasure, LOGGER_INFORMATION, "[IQMEASURE],[%s],%.2f,ms\n", "LP_AnalyzeObw", timeDuration);


	return err;
}


// MPTA API
//-----------------------------------------------------
void setPathMPTA(TX_PATH_ENUM txPath)
{
	if (txPath == eTX_PATH_12)
	{
		numSections = 2;
		PortMask[0] = BM_PORT1;
		PortMask[1] = BM_PORT2;
		PortMask[2] = PORT_DONE;
	} else if (txPath == eTX_PATH_13) {
		numSections = 2;
		PortMask[0] = BM_PORT1;
		PortMask[1] = BM_PORT3;
		PortMask[2] = PORT_DONE;
	} else if (txPath == eTX_PATH_1) {
		numSections = 1;
		PortMask[0] = BM_PORT1;
		PortMask[1] = PORT_DONE;
	} else if (txPath == eTX_PATH_2) {
		numSections = 1;
		PortMask[0] = BM_PORT2;
		PortMask[1] = PORT_DONE;
	} else if (txPath == eTX_PATH_3) {
		numSections = 1;
		PortMask[0] = BM_PORT3;
		PortMask[1] = PORT_DONE;
	} else if (txPath == eTX_PATH_123) {
		numSections = 3;
		PortMask[0] = BM_PORT1;
		PortMask[1] = BM_PORT2;
		PortMask[2] = BM_PORT3;
		PortMask[3] = PORT_DONE;
	} else if (txPath == eTX_PATH_ALL) {
		numSections = 1;
		PortMask[0] = BM_PORT1;
		PortMask[1] = BM_PORT2;
		PortMask[2] = BM_PORT3;
		PortMask[3] = PORT_DONE;
	} else {
		numSections = 1;
		PortMask[0] = BM_PORT1;
		PortMask[1] = BM_PORT2;
		PortMask[2] = BM_PORT3;
		PortMask[3] = BM_PORT4;
		PortMask[4] = PORT_DONE;
	}
}

IQMEASURE_API bool LP_MptaAvailable()
{
	if (LibsInitialized)
	{
		if ( hndl->MptaAvailable())
			return true;
		else
			return false;
	}
	else
		return false;
}

IQMEASURE_API bool LP_MptaGetSerialNumber(char *serialNumber, int buf_size)
{
	char serialNumber1[100];
	bool bStatus;

	if (LibsInitialized)
	{
		bStatus = hndl->MptaGetSerialNumber(serialNumber1, 100);
		if (bStatus) strncpy_s(serialNumber,buf_size, serialNumber1, 99);
	}

	return bStatus;
}

IQMEASURE_API int LP_MptaEnable()
{
	int err = ERR_OK;
	if (LibsInitialized)
	{
		if ( hndl->MptaEnable())
			err = ERR_OK;
		else
			err = ERR_MPTA_NOT_ENABLE;
	}
	else
		err = ERR_NOT_INITIALIZED;
	return err;
}

IQMEASURE_API int LP_MptaDisable()
{
	int err = ERR_OK;
	if (LibsInitialized)
	{
		if ( hndl->MptaDisable())
			err = ERR_OK;
		else
			err = ERR_MPTA_NOT_DISENABLE;
	}
	else
		err = ERR_NOT_INITIALIZED;
	return err;
}

IQMEASURE_API int LP_MptaSetupCapture(TX_PATH_ENUM tx_path, double rfFreqHz, double peakSignaldB, double CaptureTimeUs)
{
	int err = ERR_OK;
	char cTemp[128] = {0};
	MPTA_TX_TEST tx_test;

	//VSG setting
	hndl->tx->rfFreqHz = rfFreqHz;
	hndl->rx->rfFreqHz = rfFreqHz;
	err = hndl->SetTxRx();
	if (err != IQAPI_ERR_OK)
	{
		err = ERR_MPTA_SET_TXRX;
		//printf(message, "SetTxRx error");
		return err;
	}

	//VSA setting
	setPathMPTA(tx_path);

	tx_test.fFreqHz         = rfFreqHz;	        // Channel frequency
	tx_test.fPeakSignaldB	= peakSignaldB;		// Peak signal level
	tx_test.nSkipCount      = 1;		        // skip 1 packet before capture
	tx_test.fCaptureTimeUs	= CaptureTimeUs;	// 200us
	//tx_test.nPortMask       = BM_PORT1+BM_PORT2;  // PORT 1 & PORT 2
	tx_test.nSequenceCount	= 1;		        // no. of capture sequence to store
	tx_test.pPortList       = PortMask;

	err = hndl->MptaSetupSwitchCapture(&tx_test);
	if (err)
		err = ERR_OK;
	else
		err = ERR_MPTA_CAPTURE_FAILED;

	return err;
}

// Carrier number
#define CARRIER_0    0
#define CARRIER_1    1
#define CARRIER_2    2
#define CARRIER_16   16
#define CARRIER_17   17
#define CARRIER_26   26
#define CARRIER_28   28
#define CARRIER_42   42
#define CARRIER_43   43
#define CARRIER_58   58


double CalculateIQtoP(double data_i, double data_q)
{
	return (data_i*data_i + data_q*data_q);
}

IQMEASURE_API int LP_LoResultSpectralFlatness11AG(int* carrierNo, double* margin, double* loLeakage, bool* flatnessPass, int* sub_carrier, double* data_per_carrier, double* average_center_power)
{
	int     result = ERR_OK;

	int     lengthIQ = 0;
	double  *dataQ, *dataI, *data;
	double  avg_center_power = 0, avg_power = 0;
	int     numberOfEst = 0;
	int     carrierFirst, carrierSecond, carrierThird, carrierFourth;
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	*flatnessPass = true;

	*margin    = NA_NUMBER;
	*loLeakage = NA_NUMBER;
	*carrierNo = (int)NA_NUMBER;

	numberOfEst = ::LP_GetVectorMeasurement("hhEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
	if ( numberOfEst!=64 )	// IQ data length, should be 64
	{
		return -1;
	}
	else
	{
		lengthIQ = numberOfEst;
	}

	dataI    = (double *) malloc (lengthIQ * sizeof (double));
	dataQ    = (double *) malloc (lengthIQ * sizeof (double));
	data     = (double *) malloc (lengthIQ * sizeof (double));

	if (dataQ && dataI && data)
	{
		// Get IQ data.
		for (int carrier=0;carrier<numberOfEst;carrier++)
		{
			dataI[carrier] = bufferReal[carrier];
			dataQ[carrier] = bufferImag[carrier];

			// 1. Convert I and Q to power: P = I*I + Q*Q (the formula is not 100% correct, but log will take out the ratio)
			// 2. Average P
			// 3. avg_power = 10 * log (avg_P)

			// Calculate the power at each point from I & Q data.
			data[carrier] = CalculateIQtoP( dataI[carrier], dataQ[carrier] );
		}

		// IEEE 802.11a/g Transmitter flatness
		// The average energy of the constellations in each of the spectral lines ¡V16.. ¡V1 and +1.. +16 will deviate no
		// more than ¡Ó 2 dB from their average energy. The average energy of the constellations in each of the spectral
		// lines ¡V26.. ¡V17 and +17.. +26 will deviate no more than +2/¡V4 dB from the average energy of spectral lines
		// ¡V16.. ¡V1 and +1.. +16. The data for this test shall be derived from the channel estimation step.

		carrierFirst  = CARRIER_1;
		carrierSecond = CARRIER_16;
		carrierThird  = CARRIER_17;
		carrierFourth = CARRIER_26;

		// Calculate the average energy for the spectral lines
		for (int i=carrierFirst;i<=carrierSecond;i++)
		{
			avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
		}
		// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
		for (int i=carrierThird;i<=carrierFourth;i++)
		{
			avg_power = avg_power + data[i] + data[lengthIQ-i];
		}
		avg_power = avg_power + avg_center_power;

		avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
		if (0!=avg_center_power)
		{
			avg_center_power = 10.0 * log10 (avg_center_power);
		}
		else
		{
			avg_center_power = 0;
		}
		avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
		if (0!=avg_power)
		{
			avg_power = 10.0 * log10 (avg_power);
		}
		else
		{
			avg_power = 0;
		}

		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			if (0!=data[carrier])
			{
				data[carrier] = 10.0 * log10 (data[carrier]);
			}
			else
			{
				data[carrier] = 0;
			}
		}

		// Check the flatness
		double deltaPower = 0;
		for (int i=carrierFirst;i<=carrierSecond;i++)
		{
			deltaPower = data[i]-avg_center_power;
			if ( deltaPower>=0 )
			{
				if ( (deltaPower>2) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					// Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if ((deltaPower-2)>(*margin))	// Save the margin
				{
					*margin = deltaPower-2;
					*carrierNo = i;
				}
				else
				{
					// do nothing
				}
			}
			else	// deltaPower<0
			{
				if ( (deltaPower<(-2)) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					    // Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if (((-2)-deltaPower)>(*margin))	// Save the margin
				{
					*margin = (-2)-deltaPower;
					*carrierNo = i;
				}
				else
				{
					// do nothing
				}
			}

			deltaPower = data[lengthIQ-i]-avg_center_power;
			if ( deltaPower>=0 )
			{
				if ( (deltaPower>2) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					// Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if ((deltaPower-2)>(*margin))	// Save the margin
				{
					*margin = deltaPower-2;
					*carrierNo = (-1)*i;
				}
				else
				{
					// do nothing
				}
			}
			else	// deltaPower<0
			{
				if ( (deltaPower<(-2)) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					    // Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if (((-2)-deltaPower)>(*margin))	// Save the margin
				{
					*margin = (-2)-deltaPower;
					*carrierNo = (-1)*i;
				}
				else
				{
					// do nothing
				}
			}

		}
		// Continue, Check the flatness. (side)
		for (int i=carrierThird;i<=carrierFourth;i++)
		{
			deltaPower = data[i]-avg_center_power;
			if ( deltaPower>=0 )
			{
				if ( (deltaPower>2) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					// Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if ((deltaPower-2)>(*margin))	// Save the margin
				{
					*margin = deltaPower-2;
					*carrierNo = i;
				}
				else
				{
					// do nothing
				}
			}
			else	// deltaPower<0
			{
				if ( (deltaPower<(-4)) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					    // Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if (((-4)-deltaPower)>(*margin))	// Save the margin
				{
					*margin = (-4)-deltaPower;
					*carrierNo = i;
				}
				else
				{
					// do nothing
				}
			}

			deltaPower = data[lengthIQ-i]-avg_center_power;
			if ( deltaPower>=0 )
			{
				if ( (deltaPower>2) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					// Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if ((deltaPower-2)>(*margin))	// Save the margin
				{
					*margin = deltaPower-2;
					*carrierNo = (-1)*i;
				}
				else
				{
					// do nothing
				}
			}
			else	// deltaPower<0
			{
				if ( (deltaPower<(-4)) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					    // Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if (((-4)-deltaPower)>(*margin))	// Save the margin
				{
					*margin = (-4)-deltaPower;
					*carrierNo = (-1)*i;
				}
				else
				{
					// do nothing
				}
			}

		}

		//if (flatnessPass==true)
		//{
		//	*margin    = NA_NUMBER;
		//	*carrierNo = 0;
		//}
		//else
		//{
		//	// do nothing
		//}

		// IEEE 802.11a/g - Transmitter center frequency leakage
		// Certain transmitter implementations may cause leakage of the center frequency component. Such leakage
		// (which manifests itself in a receiver as energy in the center frequency component) shall not exceed -15 dB
		// relative to overall transmitted power or, equivalently, +2 dB relative to the average energy of the rest of the
		// subcarriers. The data for this test shall be derived from the channel estimation phase.
		if (0!=carrierFourth)
		{
			*loLeakage = data[0] - avg_power - 10 * log10((double)(carrierFourth*2));	// overall transmitted power
		}
		else
		{
			*loLeakage = data[0] - avg_power;
		}

#if defined(_DEBUG)
		// print out the Mask into file for debug
		FILE *fp;
		fopen_s(&fp, "Log_Flatness_Result_11AG.csv", "w");
		if (fp)
		{
			fprintf(fp, "WiFi Flatness Carrier,Signal,Avg_Center_Power,Size: %d\n", lengthIQ);    // print to log file
			for (int carrier=0;carrier<lengthIQ;carrier++)
			{
				fprintf( fp, "%d,%8.2f,%8.2f\n", carrier, data[carrier], avg_center_power);    // print to log file
			}
			fclose(fp);
		}
		else
		{
			// fopen failed
		}
#endif

		//D.T.
		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			sub_carrier[carrier] = carrier;
			data_per_carrier[carrier] = data[carrier];
		}

		*average_center_power = avg_center_power;
	}
	else
	{
		// if (dataQ && dataI && data)
		result = -1;
	}

	if (dataQ)	   free (dataQ);
	if (dataI)	   free (dataI);
	if (data)	   free (data);

	return result;
}

IQMEASURE_API int LP_LoResultSpectralFlatness11N(int wifiMode, int* carrierNo, double* margin, double* loLeakage, bool* flatnessPass, int* sub_carrier, double* data_per_carrier, double* average_center_power, int streamIndex)  // streamIndex = 0, 1, 2, 3 (zero based)
{
	int     result = ERR_OK;

	int     lengthIQ;
	double  avg_center_power = 0, avg_power = 0;
	int     numberOfEst = 0, numberOfStream = 0, bandwidthMhz = 20;
	int     carrierFirst, carrierSecond, carrierThird, carrierFourth;
	double  *dataRaw, *data;
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];

	*flatnessPass = true;

	*margin    = NA_NUMBER;
	*loLeakage = NA_NUMBER;
	*carrierNo = (int)NA_NUMBER;

	numberOfEst = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
	if (numberOfEst <= 0)
	{
		return -1;
	}

	numberOfStream = (int)::LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);
	if (streamIndex >= numberOfStream)
	{
		return 0;	// special case, need return, but not error.
	}

	bandwidthMhz = (int)::LP_GetScalarMeasurement("rateInfo_bandwidthMhz", 0);
	if ( (wifiMode==WIFI_11N_HT20)&&(bandwidthMhz!=20) )
	{
		return -1;
	}
	else if ( (wifiMode==WIFI_11N_HT40)&&(bandwidthMhz!=40) )
	{
		return -1;
	}
	else
	{
		// IQ data length, should be 64 or 128
		if (bandwidthMhz==20)
		{
			lengthIQ = 64;
		}
		else
		{
			lengthIQ = 128;
		}
	}

	dataRaw  = (double *) malloc (numberOfEst * sizeof (double));
	data     = (double *) malloc (lengthIQ * sizeof (double));

	if (dataRaw && data)
	{
		// 1. Convert I and Q to power: P = I*I + Q*Q (the formula is not 100% correct, but log will take out the ratio)
		// 2. Average P
		// 3. avg_power = 10 * log (avg_P)

		// Calculate the power at each point from I & Q raw data.
		for (int carrier=0;carrier<numberOfEst;carrier++)
		{
			dataRaw[carrier] = CalculateIQtoP( bufferReal[carrier], bufferImag[carrier] );
		}

		// Initial the data[] array
		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			data[carrier] = 0;
		}

		// [Roman said] Since carrier 0 value are all the same cross all tester,
		// Thus, only need to retrieve once.
		data[0] = data[0] + dataRaw[0];

		// Get data by stream and each VSG.
		for (int carrier=1;carrier<lengthIQ;carrier++)
		{
			//for (int i=0;i<g_Tester_Number;i++)
			for (int i=0;i<1;i++)
			{
				int numberOfIndex = (lengthIQ*numberOfStream*i)+(numberOfStream*carrier)+streamIndex;
				if (numberOfIndex<numberOfEst)
				{
					data[carrier] = data[carrier] + dataRaw[numberOfIndex];
				}
				else
				{
					break;
				}
			}
		}

		// IEEE std 802.11n Transmitter flatness
		// HT20
		// In a 20 MHz channel and in corresponding 20 MHz transmission in a 40 MHz channel, the average energy
		// of the constellations in each of the spectral lines ¡V16 to ¡V1 and +1 to +16 shall deviate no more than ¡Ó 2 dB
		// from their average energy. The average energy of the constellations in each of the spectral lines ¡V28 to ¡V17
		// and +17 to +28 shall deviate no more than +2/¡V4 dB from the average energy of spectral lines ¡V16 to ¡V1 and +1 to +16.

		// HT40
		// In a 40 MHz transmission (excluding HT duplicate format and non-HT duplicate format) the average energy
		// of the constellations in each of the spectral lines ¡V42 to ¡V2 and +2 to +42 shall deviate no more than ¡Ó 2 dB
		// from their average energy. The average energy of the constellations in each of the spectral lines ¡V43 to ¡V58
		// and +43 to +58 shall deviate no more than +2/¡V4 dB from the average energy of spectral lines ¡V42 to ¡V2 and +2 to +42.

		// In HT duplicate format and non-HT duplicate format the average energy of the constellations in each of the
		// spectral lines -42 to -33, -31 to -6, +6 to +31, and +33 to +42 shall deviate no more than ¡Ó 2 dB from their
		// average energy. The average energy of the constellations in each of the spectral lines -43 to -58 and +43 to
		// +58 shall deviate no more than +2/-4 dB from the average energy of spectral lines -42 to -33, -31 to -6, +6
		// to +31, and +33 to +42.

		if (lengthIQ==64)	// HT20 mode
		{
			carrierFirst  = CARRIER_1;
			carrierSecond = CARRIER_16;
			carrierThird  = CARRIER_17;
			carrierFourth = CARRIER_28;
		}
		else				// HT40 mode, lengthIQ=128
		{
			carrierFirst  = CARRIER_2;
			carrierSecond = CARRIER_42;
			carrierThird  = CARRIER_43;
			carrierFourth = CARRIER_58;
		}

		// Calculate the average energy for the spectral lines
		for (int i=carrierFirst;i<=carrierSecond;i++)
		{
			avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
		}
		// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
		for (int i=carrierThird;i<=carrierFourth;i++)
		{
			avg_power = avg_power + data[i] + data[lengthIQ-i];
		}
		avg_power = avg_power + avg_center_power;

		avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
		if (0!=avg_center_power)
		{
			avg_center_power = 10.0 * log10 (avg_center_power);
		}
		else
		{
			avg_center_power= 0;
		}
		avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
		if (0!=avg_power)
		{
			avg_power = 10.0 * log10 (avg_power);
		}
		else
		{
			avg_power = 0;
		}

		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			if (0!=data[carrier])
			{
				data[carrier] = 10.0 * log10 (data[carrier]);
			}
			else
			{
				data[carrier] = 0;
			}
		}

		// Check the flatness
		double deltaPower = 0;
		for (int i=carrierFirst;i<=carrierSecond;i++)
		{
			deltaPower = data[i]-avg_center_power;
			if ( deltaPower>=0 )
			{
				if ( (deltaPower>2) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					// Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if ((deltaPower-2)>(*margin))	// Save the margin
				{
					*margin = deltaPower-2;
					*carrierNo = i;
				}
				else
				{
					// do nothing
				}
			}
			else	// deltaPower<0
			{
				if ( (deltaPower<(-2)) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					    // Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if (((-2)-deltaPower)>(*margin))	// Save the margin
				{
					*margin = (-2)-deltaPower;
					*carrierNo = i;
				}
				else
				{
					// do nothing
				}
			}

			deltaPower = data[lengthIQ-i]-avg_center_power;
			if ( deltaPower>=0 )
			{
				if ( (deltaPower>2) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					// Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if ((deltaPower-2)>(*margin))	// Save the margin
				{
					*margin = deltaPower-2;
					*carrierNo = (-1)*i;
				}
				else
				{
					// do nothing
				}
			}
			else	// deltaPower<0
			{
				if ( (deltaPower<(-2)) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					    // Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if (((-2)-deltaPower)>(*margin))	// Save the margin
				{
					*margin = (-2)-deltaPower;
					*carrierNo = (-1)*i;
				}
				else
				{
					// do nothing
				}
			}

		}
		// Continue, Check the flatness. (side)
		for (int i=carrierThird;i<=carrierFourth;i++)
		{
			deltaPower = data[i]-avg_center_power;
			if ( deltaPower>=0 )
			{
				if ( (deltaPower>2) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					// Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if ((deltaPower-2)>(*margin))	// Save the margin
				{
					*margin = deltaPower-2;
					*carrierNo = i;
				}
				else
				{
					// do nothing
				}
			}
			else	// deltaPower<0
			{
				if ( (deltaPower<(-4)) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					    // Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if (((-4)-deltaPower)>(*margin))	// Save the margin
				{
					*margin = (-4)-deltaPower;
					*carrierNo = i;
				}
				else
				{
					// do nothing
				}
			}

			deltaPower = data[lengthIQ-i]-avg_center_power;
			if ( deltaPower>=0 )
			{
				if ( (deltaPower>2) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					// Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if ((deltaPower-2)>(*margin))	// Save the margin
				{
					*margin = deltaPower-2;
					*carrierNo = (-1)*i;
				}
				else
				{
					// do nothing
				}
			}
			else	// deltaPower<0
			{
				if ( (deltaPower<(-4)) )	// Spectral Flatness Failed
				{
					flatnessPass = false;	// Faile
				}
				else					    // Spectral Flatness OK
				{
					// do nothing
				}
				// No matter Failed or OK, we need to save the margin
				if (((-4)-deltaPower)>(*margin))	// Save the margin
				{
					*margin = (-4)-deltaPower;
					*carrierNo = (-1)*i;
				}
				else
				{
					// do nothing
				}
			}

		}

		//if (flatnessPass==true)
		//{
		//	*margin    = NA_NUMBER;
		//	*carrierNo = 0;
		//}
		//else
		//{
		//	// do nothing
		//}

		// Get the center frequency leakage
		if (0!=carrierFourth)
		{
			*loLeakage = data[0] - avg_power - 10 * log10((double)(carrierFourth*2));
		}
		else
		{
			*loLeakage = data[0] - avg_power;
		}
		//*loLeakage = ::LP_GetScalarMeasurement("dcLeakageDbc", streamIndex);	// report from IQAPI

#if defined(_DEBUG)
		// print out the Mask into file for debug
		FILE *fp;
		char sigFileName[MAX_BUFFER_SIZE];
		sprintf_s(sigFileName, "Log_Flatness_Result_11N_Stream%02d.csv", streamIndex+1);
		fopen_s(&fp, sigFileName, "w");
		if (fp)
		{
			fprintf(fp, "WiFi Flatness Carrier,Signal,Avg_Center_Power,Size: %d\n", lengthIQ);    // print to log file
			for (int carrier=0;carrier<lengthIQ;carrier++)
			{
				fprintf( fp, "%d,%8.2f,%8.2f\n", carrier, data[carrier], avg_center_power);    // print to log file
			}
			fclose(fp);
		}
		else
		{
			// fopen failed
		}
#endif

		//D.T.
		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			sub_carrier[carrier] = carrier;
			data_per_carrier[carrier] = data[carrier];
		}

		*average_center_power = avg_center_power;
	}
	else
	{
		// if (dataQ && dataI && data && maxData && minData)
		result = -1;
	}

	if (dataRaw)   free (dataRaw);
	if (data)	   free (data);

	return result;
}

//-----------------------------------------------------------------------------------------------

// IQmeasureTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "iqapi.h"
#include "..\IQmeasure.h"
#include "..\DebugAlloc.h"
#include "math.h"
#include "time.h"
#include "vector"
#include "IQmeasureTest.h"




//#include <iomanip>
//Move to stdafx.h
//#include "lp_consoleMode.h"




#ifndef NA_NUMBER
#define NA_NUMBER -99999.99
#endif

char    errMsg[MAX_BUFFER_SIZE];
int		g_logger_id =-1;
int		Tag_runTime = 1;
char    g_csvFileName[128] = {'\0'};
BOOL    bExitFlag = FALSE;
char    g_IP_addr[128] = {'\0'};
int		ciTesterControlMode =0;
int		ciTesterType =0;
int     g_DH_TOKEN_ID = 0;
int	    g_DH_PORT=0;



typedef struct
{
	vector<string>		LP_Init;
	vector<string>		LP_Term;
	vector<string>		LP_GetIQapiHndl;
	vector<string>		LP_InitTester;
	vector<string>		LP_InitTester2;
	vector<string>		LP_InitTester3;
	vector<string>		LP_InitTester4;
	vector<string>		LP_ConClose;
	vector<string>		LP_ConOpen;
	vector<string>		LP_GetErrorString;
	vector<string>		LP_GetVersion;
	vector<string>		LP_GetIQapiHndlLastErrMsg;
	vector<string>		LP_DualHead_ConOpen;
	vector<string>		LP_DualHead_GetTokenID;
	vector<string>		LP_DualHead_ConClose;
	vector<string>		LP_DualHead_ObtainControl;
	vector<string>		LP_DualHead_ReleaseControl;
	vector<string>		LP_FM_SetFrequency;
	vector<string>		LP_FM_SetCarrierPower;
	vector<string>		LP_SetVsg_GapPower;
	vector<string>		LP_SetVsgCw;
	vector<string>		LP_SaveSignalArrays;
	vector<string>		LP_SaveIQDataToModulationFile;
	vector<string>		LP_SetVsg;
	vector<string>		LP_SetVsgModulation;
	vector<string>		LP_EnableVsgRF;
	vector<string>		LP_SetVsa;
	vector<string>		LP_VsaDataCapture;
	vector<string>		LP_SaveVsaSignalFile;
	vector<string>		LP_AnalyzeFFT;
	vector<string>		LP_AnalyzePower;
	vector<string>		LP_Analyze80211ac;
	vector<string>		LP_SetAnalysisParameterInteger;
	vector<string>		LP_SetAnalysisParameterIntegerArray;
	vector<string>		LP_CopyVsaCaptureData;
	vector<string>		LP_LoadVsaSignalFile;
	vector<string>		LP_SaveVsaGeneratorFile;
	vector<string>		LP_SetVsaBluetooth;
	vector<string>		LP_SetVsaTriggerTimeout;
	vector<string>		LP_SetVsaAmplitudeTolerance;
	vector<string>		LP_Agc;
	vector<string>		LP_SetFrameCnt;
	vector<string>		LP_TxDone;
	vector<string>		LP_EnableSpecifiedVsgRF;
	vector<string>		LP_EnableSpecifiedVsaRF;
	vector<string>		LP_GetVsaSettings;
	vector<string>		LP_GetSampleData;
	vector<string>		LP_SelectCaptureRangeForAnalysis;
	vector<string>		LP_Analyze80216d;
	vector<string>		LP_Analyze80216e;
	vector<string>		LP_Analyze80211p;
	vector<string>		LP_Analyze80211ag;
	vector<string>		LP_Analyze80211n;
	vector<string>		LP_AnalyzeMimo;
	vector<string>		LP_Analyze80211b;
	vector<string>		LP_AnalyzeCCDF;
	vector<string>		LP_AnalyzeCW;
	vector<string>		LP_AnalyzeCWFreq;
	vector<string>		LP_AnalysisWave;
	vector<string>		LP_AnalyzeSidelobe;
	vector<string>		LP_AnalyzePowerRampOFDM;
	vector<string>		LP_AnalyzePowerRamp80211b;
	vector<string>		LP_AnalyzeBluetooth;
	vector<string>		LP_AnalyzeZigbee;
	vector<string>		LP_AnalyzeHT40Mask;
	vector<string>		LP_GetScalarMeasurement;
	vector<string>		LP_GetVectorMeasurment;
	vector<string>		LP_GetStringMeasurment;
	vector<string>		LP_GetStringMeasurement;
	vector<string>		LP_PlotDataCapture;
	vector<string>		LP_Plot;
	vector<string>		LP_StartIQmeasureTimer;
	vector<string>		LP_StopIQmeasureTimer;
	vector<string>		LP_ReportTimerDurations;
	vector<string>		LP_SetTesterHwVersion;
	vector<string>		LP_GetTesterHwVersion;
	vector<string>		LP_SetLpcPath;
	vector<string>		LP_FM_SetVsg;
	vector<string>		LP_FM_GetVsgSettings;
	vector<string>		LP_FM_SetAudioSingleTone;
	vector<string>		LP_FM_SetAudioToneArray;
	vector<string>		LP_FM_DeleteAudioTones;
	vector<string>		LP_FM_StartVsg;
	vector<string>		LP_FM_StopVsg;
	vector<string>		LP_FM_SetVsgDistortion;
	vector<string>		LP_FM_GetVsgDistortion;
	vector<string>		LP_FM_StartVsgInterference;
	vector<string>		LP_FM_GetVsgInterferenceSettings;
	vector<string>		LP_FM_StopVsgInterference;
	vector<string>		LP_FM_SetVsa;
	vector<string>		LP_FM_GetVsaSettings;
	vector<string>		LP_FM_VsaDataCapture;
	vector<string>		LP_FM_Analyze_RF;
	vector<string>		LP_FM_Analyze_Demod;
	vector<string>		LP_FM_Analyze_Audio_Mono;
	vector<string>		LP_FM_Analyze_Audio_Stereo;
	vector<string>		LP_FM_GetScalarMeasurement;
	vector<string>		LP_FM_GetVectorMeasurement;
	vector<string>		LP_FM_AudioDataCapture;
	vector<string>		LP_FM_LoadAudioCapture;
	vector<string>		LP_FM_AudioStimulusGenerateAndPlayMultiTone;
	vector<string>		LP_FM_AudioStimulusGenerateAndPlaySingleTone;
	vector<string>		LP_FM_StopAudioPlay;
	vector<string>		LP_SaveVsaSignalFileText;
	vector<string>		LP_SaveTruncateCapture;
	vector<string>		LP_PwrCalFastGetElement;
	vector<string>		LP_FM_SetAudioToneModeAmplitude;
	vector<string>		LP_SetVsg_triggerType;
	vector<string>		LP_AnalyzeVHT80Mask;
	vector<string>		LP_FastCalGetPowerData;
	vector<string>		LP_PwrCalFreeElement;
	vector<string>		LP_FastCalMeasPower;
	vector<string>      SetTesterHwVersion;
	vector<string>		LP_GetVectorMeasurement;
	vector<string>		GetTesterHwVersion;
	vector<string>		LP_FM_SetAudioToneArrayDeviation;
	vector<string>		LP_GetScalarMeasurment;
	vector<string>		LP_AnalyzeZigbeeTerm;
	vector<string>		LP_PwrCalFastGetElemen;
	vector<string>		LP_FM_GetAudioToneArray;
	vector<string>		LP_others;
}strcutResult;

static void set_color(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);
}



unsigned int GetElapsedMSec(lp_time_t start, lp_time_t end)
{
	unsigned int time_ms = (unsigned int) (
			(end.time - start.time) * 1000
			+ (end.millitm - start.millitm)
			);
	return time_ms;
}

void CheckReturnCode( int returnCode, char *functionName)
{
	if( ERR_OK!=returnCode )
	{
		//TODO
		if( NULL!=functionName )
		{
			sprintf_s(errMsg, MAX_BUFFER_SIZE, "%s returned error: %d", functionName, returnCode);
		}
		else
		{
			sprintf_s(errMsg, MAX_BUFFER_SIZE, "Error code: %d", returnCode);
		}
		throw errMsg;
	}
	else
		printf("%s, return OK.\n", functionName);

	return;
}



double evmLinearAverage(double *resultArray, int averageTimes, double &averageResult, double &maxResult, double &minResult)
{
	int err=0;
	averageResult = 0.0;
	maxResult = minResult = resultArray[0];
	for (int i=0; i<averageTimes; i++)
	{
		if (resultArray[i]<minResult)
			minResult=resultArray[i];
		if (resultArray[i]>maxResult)
			maxResult=resultArray[i];
		averageResult+=resultArray[i];
	}
	averageResult=averageResult/averageTimes;

	return err;
}

void WiFi_Loopback()
{
	char   buffer[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester

	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(), "LP_Init()" );
		CheckReturnCode( LP_InitTester(IP_ADDR), "LP_InitTester()" );
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("%s\n", buffer);

		//-------------//
		//  Setup VSG  //
		//-------------//
		CheckReturnCode( LP_SetVsg(FREQ_HZ, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

		//--------------------//
		// Load waveform file //
		//--------------------//
		//CheckReturnCode( LP_SetVsgModulation("../mod/WiFi_OFDM-54.mod"), "LP_SetVsgModulation()" );
		CheckReturnCode( LP_SetVsgModulation("../../iqvsg/WiFi_OFDM-54.mod"), "LP_SetVsgModulation()" );

		//---------------//
		//  Turn on VSG  //
		//---------------//
		CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

		//------------------------------------------//
		//   Send packet, continuous transmission   //
		//------------------------------------------//
		CheckReturnCode( LP_SetFrameCnt(1), "LP_SetFrameCnt()" );
		int frameCnt = 0;	// number of frame to send, 0 means continuous transmission.
		CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );

		//----------------//
		//   Setup VSA    //
		//----------------//
		int peakToAverageRatio = 10;
		CheckReturnCode( LP_SetVsa(FREQ_HZ, (dutPowerLevel+peakToAverageRatio), PORT_LEFT), "LP_SetVsa()" );

		//----------------------//
		//  Perform VSA capture //
		//----------------------//
		CheckReturnCode( LP_VsaDataCapture(SAMPLING_TIME_SEC, VSA_TRIGGER_TYPE), "LP_VsaDataCapture()" );

		//---------------------------//
		//  Save sig file for debug  //
		//---------------------------//
		CheckReturnCode( LP_SaveVsaSignalFile("WiFi_Capture.sig"), "LP_SaveVsaSignalFile()" );

		//------------------------//
		//  Perform WiFi analysis //
		//------------------------//
		CheckReturnCode( LP_Analyze80211ag(), "LP_Analyze80211ag()" );

		//-------------------------//
		//  Retrieve Test Results  //
		//-------------------------//
		double result;
		result = LP_GetScalarMeasurement("psduCrcFail");
		result = LP_GetScalarMeasurement("plcpCrcPass");
		result = LP_GetScalarMeasurement("dataRate");
		printf("Datarate=%.1f Mbps\n", result);
		result = LP_GetScalarMeasurement("numSymbols");
		result = LP_GetScalarMeasurement("numPsduBytes");
		result = LP_GetScalarMeasurement("evmAll");
		printf("EVM_All=%.3f dB\n", result);
		result = LP_GetScalarMeasurement("evmData");
		result = LP_GetScalarMeasurement("evmPilot");
		result = LP_GetScalarMeasurement("codingRate");
		result = LP_GetScalarMeasurement("freqErr");
		result = LP_GetScalarMeasurement("clockErr");
		result = LP_GetScalarMeasurement("ampErr");
		result = LP_GetScalarMeasurement("ampErrDb");
		result = LP_GetScalarMeasurement("phaseErr");
		result = LP_GetScalarMeasurement("rmsPhaseNoise");
		result = LP_GetScalarMeasurement("rmsPowerNoGap");
		printf("Power_NoGap=%.3f dBm\n", result);
		result = LP_GetScalarMeasurement("rmsPower");
		result = LP_GetScalarMeasurement("pkPower");
		result = LP_GetScalarMeasurement("rmsMaxAvgPower");
		result = LP_GetScalarMeasurement("dcLeakageDbc");

		//----------------//
		//  Turn off VSG  //
		//----------------//
		CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );

		//----------------------------//
		//   Disconnect the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Term(), "LP_Term()" );
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}
}

void WiFi_11ag_Loopback()
{
	char   buffer[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester

	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(), "LP_Init()" );
		CheckReturnCode( LP_InitTester(IP_ADDR), "LP_InitTester()" );
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("%s\n", buffer);

		//-------------//
		//  Setup VSG  //
		//-------------//
		CheckReturnCode( LP_SetVsg(FREQ_HZ, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

		//--------------------//
		// Load waveform file //
		//--------------------//
		//CheckReturnCode( LP_SetVsgModulation("../mod/WiFi_OFDM-54.iqvsg"), "LP_SetVsgModulation()" );
		CheckReturnCode( LP_SetVsgModulation("../../iqvsg/WiFi_OFDM-54.iqvsg"), "LP_SetVsgModulation()" );

		//---------------//
		//  Turn on VSG  //
		//---------------//
		CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

		//------------------------------------------//
		//   Send packet, continuous transmission   //
		//------------------------------------------//
		int frameCnt = 0;	// number of frame to send, 0 means continuous transmission.
		CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );

		//----------------//
		//   Setup VSA    //
		//----------------//
		int peakToAverageRatio = 10;
		CheckReturnCode( LP_SetVsa(FREQ_HZ, (dutPowerLevel+peakToAverageRatio), PORT_LEFT), "LP_SetVsaBluetooth()" );

		//----------------------//
		//  Perform VSA capture //
		//----------------------//
		CheckReturnCode( LP_VsaDataCapture(SAMPLING_TIME_SEC, VSA_TRIGGER_TYPE), "LP_VsaDataCapture()" );

		//---------------------------//
		//  Save sig file for debug  //
		//---------------------------//
		CheckReturnCode( LP_SaveVsaSignalFile("WiFi_Capture.sig"), "LP_SaveVsaSignalFile()" );

		double result;
		CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );

		result = LP_GetScalarMeasurement("valid");
		printf("valid=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_each_burst");
		printf("P_av_each_burst=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_each_burst_dBm");
		printf("P_av_each_burst_dBm=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_pk_each_burst");
		printf("P_pk_each_burst=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
		printf("P_pk_each_burst_dBm=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_all");
		printf("P_av_all=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_peak_all");
		printf("P_peak_all=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_no_gap_all");
		printf("P_av_no_gap_all=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_all_dBm");
		printf("P_av_all_dBm=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_peak_all_dBm");
		printf("P_peak_all_dBm=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
		printf("P_av_no_gap_all_dBm=%.3f \n", result);

		double start_secRealArray [MAX_BUFFER_SIZE] = {0.0};
		double start_secImagArray [MAX_BUFFER_SIZE] = {0.0};
		double stop_secRealArray [MAX_BUFFER_SIZE] = {0.0};
		double stop_secImagArray [MAX_BUFFER_SIZE] = {0.0};
		double P_av_each_burst_dBmRealArray [MAX_BUFFER_SIZE] = {0.0};
		double P_av_each_burst_dBmImagArray [MAX_BUFFER_SIZE] = {0.0};
		double P_pk_each_burst_dBmRealArray [MAX_BUFFER_SIZE] = {0.0};
		double P_pk_each_burst_dBmImagArray [MAX_BUFFER_SIZE] = {0.0};
		double complete_burstRealArray [MAX_BUFFER_SIZE] = {0.0};
		double complete_burstImagArray [MAX_BUFFER_SIZE] = {0.0};

		int start_secArrayLength = LP_GetVectorMeasurement("start_sec", start_secRealArray, start_secImagArray, MAX_BUFFER_SIZE);
		int stop_secArrayLength = LP_GetVectorMeasurement("stop_sec", stop_secRealArray, stop_secImagArray, MAX_BUFFER_SIZE);
		int P_av_each_burst_dBmArrayLength = LP_GetVectorMeasurement("P_av_each_burst_dBm", P_av_each_burst_dBmRealArray, P_av_each_burst_dBmImagArray, MAX_BUFFER_SIZE);
		int P_pk_each_burst_dBmArrayLength = LP_GetVectorMeasurement("P_pk_each_burst_dBm", P_pk_each_burst_dBmRealArray, P_pk_each_burst_dBmImagArray, MAX_BUFFER_SIZE);
		int complete_burstArrayLength = LP_GetVectorMeasurement("complete_burst", complete_burstRealArray, complete_burstImagArray, MAX_BUFFER_SIZE);

		//------------------------//
		//  Perform WiFi analysis //
		//------------------------//
		CheckReturnCode( LP_Analyze80211ag(), "LP_Analyze80211ag()" );

		//-------------------------//
		//  Retrieve Test Results  //
		//-------------------------//
		result = LP_GetScalarMeasurement("psduCrcFail");
		printf("psduCrcFail=%.3f \n", result);

		result = LP_GetScalarMeasurement("plcpCrcPass");
		printf("plcpCrcPass=%.3f \n", result);

		result = LP_GetScalarMeasurement("dataRate");
		printf("Datarate=%.1f Mbps\n", result);

		result = LP_GetScalarMeasurement("numSymbols");
		printf("numSymbols=%.3f \n", result);

		result = LP_GetScalarMeasurement("numPsduBytes");
		printf("numPsduBytes=%.3f \n", result);

		result = LP_GetScalarMeasurement("evmAll");
		printf("EVM_All=%.3f dB\n", result);

		result = LP_GetScalarMeasurement("evmData");
		printf("evmData=%.3f \n", result);

		result = LP_GetScalarMeasurement("evmPilot");
		printf("evmPilot=%.3f \n", result);

		result = LP_GetScalarMeasurement("codingRate");
		printf("codingRate=%.3f \n", result);

		result = LP_GetScalarMeasurement("freqErr");
		printf("freqErr=%.3f \n", result);

		result = LP_GetScalarMeasurement("clockErr");
		printf("clockErr=%.3f \n", result);

		result = LP_GetScalarMeasurement("ampErr");
		printf("ampErr=%.3f \n", result);

		result = LP_GetScalarMeasurement("ampErrDb");
		printf("ampErrDb=%.3f \n", result);

		result = LP_GetScalarMeasurement("phaseErr");
		printf("phaseErr=%.3f \n", result);

		result = LP_GetScalarMeasurement("rmsPhaseNoise");
		printf("rmsPhaseNoise=%.3f \n", result);

		result = LP_GetScalarMeasurement("rmsPowerNoGap");
		printf("Power_NoGap=%.3f dBm\n", result);

		result = LP_GetScalarMeasurement("rmsPower");
		printf("rmsPower=%.3f \n", result);

		result = LP_GetScalarMeasurement("pkPower");
		printf("pkPower=%.3f \n", result);

		result = LP_GetScalarMeasurement("rmsMaxAvgPower");
		printf("rmsMaxAvgPower=%.3f \n", result);

		result = LP_GetScalarMeasurement("dcLeakageDbc");
		printf("dcLeakageDbc=%.3f \n", result);

		double hhEstRealArray [MAX_BUFFER_SIZE] = {0.0};
		double hhEstImagArray [MAX_BUFFER_SIZE] = {0.0};
		double psduRealArray [MAX_BUFFER_SIZE] = {0.0};
		double psduImagArray [MAX_BUFFER_SIZE] = {0.0};
		double startPointersRealArray [MAX_BUFFER_SIZE] = {0.0};
		double startPointersImagArray [MAX_BUFFER_SIZE] = {0.0};
		double plcpRealArray [MAX_BUFFER_SIZE] = {0.0};
		double plcpImagArray [MAX_BUFFER_SIZE] = {0.0};

		int hhEstArrayLength = LP_GetVectorMeasurement("hhEst", hhEstRealArray, hhEstImagArray, MAX_BUFFER_SIZE);
		int psduArrayLength = LP_GetVectorMeasurement("psdu", psduRealArray, psduImagArray, MAX_BUFFER_SIZE);
		int startPointersArrayLength = LP_GetVectorMeasurement("startPointers", startPointersRealArray, startPointersImagArray, MAX_BUFFER_SIZE);
		int plcpArrayLength = LP_GetVectorMeasurement("plcp", plcpRealArray, plcpImagArray, MAX_BUFFER_SIZE);

		//----------------//
		//  Turn off VSG  //
		//----------------//
		CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );

		//----------------------------//
		//   Disconnect the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Term(), "LP_Term()" );
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}
}

void WiFi_11b_Loopback()
{
	char   buffer[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester

	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(), "LP_Init()" );
		CheckReturnCode( LP_InitTester(IP_ADDR), "LP_InitTester()" );
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("%s\n", buffer);

		//-------------//
		//  Setup VSG  //
		//-------------//
		CheckReturnCode( LP_SetVsg(FREQ_HZ, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

		//--------------------//
		// Load waveform file //
		//--------------------//
		//CheckReturnCode( LP_SetVsgModulation("../mod/WiFi_CCK-5_5S.iqvsg"), "LP_SetVsgModulation()" );//TODO change to 11b modulation file
		CheckReturnCode( LP_SetVsgModulation("../../iqvsg/WiFi_CCK-5_5S.iqvsg"), "LP_SetVsgModulation()" );//TODO change to 11b modulation file

		//---------------//
		//  Turn on VSG  //
		//---------------//
		CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

		//------------------------------------------//
		//   Send packet, continuous transmission   //
		//------------------------------------------//
		int frameCnt = 0;	// number of frame to send, 0 means continuous transmission.
		CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );

		//----------------//
		//   Setup VSA    //
		//----------------//
		int peakToAverageRatio = 10;
		CheckReturnCode( LP_SetVsa(FREQ_HZ, (dutPowerLevel+peakToAverageRatio), PORT_LEFT), "LP_SetVsaBluetooth()" );

		//----------------------//
		//  Perform VSA capture //
		//----------------------//
		CheckReturnCode( LP_VsaDataCapture(SAMPLING_TIME_SEC, VSA_TRIGGER_TYPE), "LP_VsaDataCapture()" );

		//---------------------------//
		//  Save sig file for debug  //
		//---------------------------//
		CheckReturnCode( LP_SaveVsaSignalFile("WiFi_Capture.sig"), "LP_SaveVsaSignalFile()" );

		double result;
		CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );

		result = LP_GetScalarMeasurement("valid");
		printf("valid=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_each_burst");
		printf("P_av_each_burst=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_each_burst_dBm");
		printf("P_av_each_burst_dBm=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_pk_each_burst");
		printf("P_pk_each_burst=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
		printf("P_pk_each_burst_dBm=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_all");
		printf("P_av_all=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_peak_all");
		printf("P_peak_all=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_no_gap_all");
		printf("P_av_no_gap_all=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_all_dBm");
		printf("P_av_all_dBm=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_peak_all_dBm");
		printf("P_peak_all_dBm=%.3f \n", result);

		result = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
		printf("P_av_no_gap_all_dBm=%.3f \n", result);

		double start_secRealArray [MAX_BUFFER_SIZE] = {0.0};
		double start_secImagArray [MAX_BUFFER_SIZE] = {0.0};
		double stop_secRealArray [MAX_BUFFER_SIZE] = {0.0};
		double stop_secImagArray [MAX_BUFFER_SIZE] = {0.0};
		double P_av_each_burst_dBmRealArray [MAX_BUFFER_SIZE] = {0.0};
		double P_av_each_burst_dBmImagArray [MAX_BUFFER_SIZE] = {0.0};
		double P_pk_each_burst_dBmRealArray [MAX_BUFFER_SIZE] = {0.0};
		double P_pk_each_burst_dBmImagArray [MAX_BUFFER_SIZE] = {0.0};
		double complete_burstRealArray [MAX_BUFFER_SIZE] = {0.0};
		double complete_burstImagArray [MAX_BUFFER_SIZE] = {0.0};

		int start_secArrayLength = LP_GetVectorMeasurement("start_sec", start_secRealArray, start_secImagArray, MAX_BUFFER_SIZE);
		int stop_secArrayLength = LP_GetVectorMeasurement("stop_sec", stop_secRealArray, stop_secImagArray, MAX_BUFFER_SIZE);
		int P_av_each_burst_dBmArrayLength = LP_GetVectorMeasurement("P_av_each_burst_dBm", P_av_each_burst_dBmRealArray, P_av_each_burst_dBmImagArray, MAX_BUFFER_SIZE);
		int P_pk_each_burst_dBmArrayLength = LP_GetVectorMeasurement("P_pk_each_burst_dBm", P_pk_each_burst_dBmRealArray, P_pk_each_burst_dBmImagArray, MAX_BUFFER_SIZE);
		int complete_burstArrayLength = LP_GetVectorMeasurement("complete_burst", complete_burstRealArray, complete_burstImagArray, MAX_BUFFER_SIZE);

		//------------------------//
		//  Perform WiFi analysis //
		//------------------------//
		CheckReturnCode( LP_Analyze80211b(), "LP_Analyze80211b()" );

		//-------------------------//
		//  Retrieve Test Results  //
		//-------------------------//
		result = LP_GetScalarMeasurement("lockedClock");
		printf("lockedClock=%.3f \n", result);

		result = LP_GetScalarMeasurement("plcpCrcFail");
		printf("plcpCrcFail=%.3f \n", result);

		result = LP_GetScalarMeasurement("psduCrcFail");
		printf("psduCrcFail=%.3f \n", result);

		result = LP_GetScalarMeasurement("longPreamble");
		printf("longPreamble=%.3f \n", result);

		result = LP_GetScalarMeasurement("bitRateInMHz");
		printf("bitRateInMHz=%.3f \n", result);

		result = LP_GetScalarMeasurement("evmPk");
		printf("evmPk=%.3f \n", result);

		result = LP_GetScalarMeasurement("evmAll");
		printf("evmAll=%.3f \n", result);

		result = LP_GetScalarMeasurement("evmInPreamble");
		printf("evmInPreamble=%.3f \n", result);

		result = LP_GetScalarMeasurement("evmInPsdu");
		printf("evmInPsdu=%.3f \n", result);

		result = LP_GetScalarMeasurement("freqErr");
		printf("freqErr=%.3f \n", result);

		result = LP_GetScalarMeasurement("clockErr");
		printf("clockErr=%.3f \n", result);

		result = LP_GetScalarMeasurement("ampErr");
		printf("ampErr=%.3f \n", result);

		result = LP_GetScalarMeasurement("ampErrDb");
		printf("ampErrDb=%.3f \n", result);

		result = LP_GetScalarMeasurement("phaseErr");
		printf("phaseErr=%.3f \n", result);

		result = LP_GetScalarMeasurement("rmsPhaseNoise");
		printf("rmsPhaseNoise=%.3f \n", result);

		result = LP_GetScalarMeasurement("rmsPowerNoGap");
		printf("rmsPowerNoGap=%.3f \n", result);

		result = LP_GetScalarMeasurement("rmsPower");
		printf("rmsPower=%.3f \n", result);

		result = LP_GetScalarMeasurement("pkPower");
		printf("pkPower=%.3f \n", result);

		result = LP_GetScalarMeasurement("rmsMaxAvgPower");
		printf("rmsMaxAvgPower=%.3f \n", result);

		result = LP_GetScalarMeasurement("bitRate");
		printf("bitRate=%.3f \n", result);

		result = LP_GetScalarMeasurement("modType");
		printf("modType=%.3f \n", result);

		double evmInPlcpRealArray [MAX_BUFFER_SIZE] = {0.0};
		double evmInPlcpImagArray [MAX_BUFFER_SIZE] = {0.0};
		double evmErrRealArray [MAX_BUFFER_SIZE] = {0.0};
		double evmErrImagArray [MAX_BUFFER_SIZE] = {0.0};
		double loLeakageDbRealArray [MAX_BUFFER_SIZE] = {0.0};
		double loLeakageDbImagArray [MAX_BUFFER_SIZE] = {0.0};

		int evmInPlcpArrayLength = LP_GetVectorMeasurement("evmInPlcp", evmInPlcpRealArray, evmInPlcpImagArray, MAX_BUFFER_SIZE);
		int evmErrArrayLength = LP_GetVectorMeasurement("evmErr", evmErrRealArray, evmErrImagArray, MAX_BUFFER_SIZE);
		int loLeakageDbArrayLength = LP_GetVectorMeasurement("loLeakageDb", loLeakageDbRealArray, loLeakageDbImagArray, MAX_BUFFER_SIZE);

		//----------------//
		//  Turn off VSG  //
		//----------------//
		CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );

		//----------------------------//
		//   Disconnect the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Term(), "LP_Term()" );
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}
}

void Bluetooth_Loopback()
{
	char   buffer[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester

	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//

		//the second parameter (0: using IQApi, 1: using SCPI)
		//const int ciTesterControlMode = 1;
		CheckReturnCode( LP_Init(IQTYPE_XEL, ciTesterControlMode), "LP_Init()" );

		CheckReturnCode( LP_InitTester(IP_ADDR_IQXEL), "LP_InitTester()" );
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("%s\n", buffer);

		//-------------//
		//  Setup VSG  //
		//-------------//
		CheckReturnCode( LP_SetVsg(FREQ_HZ, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

		//--------------------//
		// Load waveform file //
		//--------------------//
		//Try again if jira that session errors occurrs when uploading or loading waveform files in BT is fixed.
		CheckReturnCode( LP_SetVsgModulation("../mod/1DH1_000088C0FFEE.iqvsg"), "LP_SetVsgModulation()" );
		//CheckReturnCode( LP_SetVsgModulation("../mod/11AC_MCS8_1strm_80MHz_2550bytes_pgap20us_LongGI_AMPDUoff_Fs160.iqvsg"), "LP_SetVsgModulation()" );
		//CheckReturnCode( LP_SetVsgModulation("BT/bt_1_dh5_prbs9_Fs80M.iqvsg"), "LP_SetVsgModulation()" );



		//---------------//
		//  Turn on VSG  //
		//---------------//
		CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

		//------------------------------------------//
		//   Send packet, continuous transmission   //
		//------------------------------------------//
		int frameCnt = 0;	// number of frame to send, 0 means continuous transmission.
		CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );

		//----------------//
		//   Setup VSA    //
		//----------------//
		int peakToAverageRatio = 3;
		CheckReturnCode( LP_SetVsaBluetooth(FREQ_HZ, (dutPowerLevel+peakToAverageRatio), PORT_LEFT), "LP_SetVsaBluetooth()" );

		//----------------------//
		//  Perform VSA capture //
		//----------------------//
		CheckReturnCode( LP_VsaDataCapture( SAMPLING_TIME_SEC, VSA_TRIGGER_TYPE), "LP_VsaDataCapture()" );

		//---------------------------//
		//  Save sig file for debug  //
		//---------------------------//
		//CheckReturnCode( LP_SaveVsaSignalFile("BT_Capture.sig"), "LP_SaveVsaSignalFile()" );
		CheckReturnCode( LP_SaveVsaSignalFile(".\\log\\BT_Capture"), "LP_SaveVsaSignalFile()" );

		//-----------------------------//
		//  Perform Bluetooth analysis //
		//-----------------------------//
		double datarate = 0;	// 0 (auto), or 1, 2, 3
		CheckReturnCode( LP_AnalyzeBluetooth(datarate), "LP_AnalyzeBluetooth()" );

		//-------------------------//
		//  Retrieve Test Results  //
		//-------------------------//
		double result;
		result = LP_GetScalarMeasurement("dataRateDetect",0);
		printf("Datarate=%.1f Mbps\n", result);
		result = LP_GetScalarMeasurement("valid",0);
		result = LP_GetScalarMeasurement("bandwidth20dB",0);
		result = LP_GetScalarMeasurement("deltaF1Average",0);
		result = LP_GetScalarMeasurement("deltaF2Max",0);
		result = LP_GetScalarMeasurement("deltaF2Average",0);
		result = LP_GetScalarMeasurement("deltaF2MaxAccess",0);
		result = LP_GetScalarMeasurement("deltaF2AvAccess",0);
		result = LP_GetScalarMeasurement("EdrEVMAv",0);
		printf("EDR_DEVM_Av=%.3f %c\n", result*100, '%');
		result = LP_GetScalarMeasurement("EdrEVMpk",0);
		printf("EDR_DEVM_Peak=%.3f %c\n", result*100, '%');
		result = LP_GetScalarMeasurement("EdrEVMvalid",0);
		result = LP_GetScalarMeasurement("EdrPowDiffdB",0);
		printf("EDR_Diff_Power=%.3f dB\n", result);
		result = LP_GetScalarMeasurement("freq_deviation",0);
		result = LP_GetScalarMeasurement("freq_deviationpktopk",0);
		result = LP_GetScalarMeasurement("freq_estHeader",0);
		result = LP_GetScalarMeasurement("EdrFreqExtremeEdronly",0);
		result = LP_GetScalarMeasurement("EdrprobEVM99pass",0);
		result = LP_GetScalarMeasurement("EdrEVMvsTime",0);
		result = LP_GetScalarMeasurement("validInput",0);
		result = LP_GetScalarMeasurement("maxfreqDriftRate",0);
		result = LP_GetScalarMeasurement("payloadErrors",0);

		//----------------//
		//  Turn off VSG  //
		//----------------//
		CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );

		//----------------------------//
		//   Disconnect the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Term(), "LP_Term()" );
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}
}

void Fft_Loopback()
{

	char   buffer[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester

	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(), "LP_Init()" );
		CheckReturnCode( LP_InitTester("192.168.109.180"), "LP_InitTester()" );
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("%s\n", buffer);

		//-------------//
		//  Setup VSG  //
		//-------------//
		double offsetFreqMhz = 10;
		CheckReturnCode( LP_SetVsgCw(FREQ_HZ, offsetFreqMhz, dutPowerLevel, PORT_RIGHT), "LP_SetVsgCw()");

		//---------------//
		//  Turn on VSG  //
		//---------------//
		CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

		//----------------//
		//   Setup VSA    //
		//----------------//
		int peakToAverageRatio = 10;
		CheckReturnCode( LP_SetVsa(FREQ_HZ, (dutPowerLevel+peakToAverageRatio), PORT_LEFT), "LP_SetVsa()" );

		//----------------------//
		//  Perform VSA capture //
		//----------------------//
		IQV_TRIG_TYPE_ENUM vsaTrigger = IQV_TRIG_TYPE_FREE_RUN;
		CheckReturnCode( LP_VsaDataCapture(SAMPLING_TIME_SEC, vsaTrigger), "LP_VsaDataCapture()" );

		//---------------------------//
		//  Save sig file for debug  //
		//---------------------------//
		CheckReturnCode( LP_SaveVsaSignalFile("SNR_Capture.sig"), "LP_SaveVsaSignalFile()" );

		CheckReturnCode( LP_AnalyzeFFT(), "LP_AnalyzeFFT()");

		double result;
		result = LP_GetScalarMeasurement("valid");
		printf("valid=%.3f \n", result);

		result = LP_GetScalarMeasurement("length");
		printf("length=%.3f \n", result);

		double xRealArray [MAX_BUFFER_SIZE] = {0.0};
		double xImagArray [MAX_BUFFER_SIZE] = {0.0};
		double yRealArray [MAX_BUFFER_SIZE] = {0.0};
		double yImagArray [MAX_BUFFER_SIZE] = {0.0};

		int xArrayLength = LP_GetVectorMeasurement("x", xRealArray, xImagArray, MAX_BUFFER_SIZE);
		int yArrayLength = LP_GetVectorMeasurement("y", yRealArray, yImagArray, MAX_BUFFER_SIZE);

		// if(xArrayLength == yArrayLength)
		// {
		// 	CheckReturnCode( LP_Plot(1, xRealArray, yRealArray, xArrayLength, ".-", "FFT", "Freq", "Power", 1), "LP_Plot()");
		// }
		// else
		// {
		// 	throw "Length of X and Y do not match!";
		// }

		//----------------//
		//  Turn off VSG  //
		//----------------//
		CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );

		//----------------------------//
		//   Disconnect the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Term(), "LP_Term()" );

	}
	catch(char *msg)
	{
		printf("Exception thrown: %s\n", msg);
	}
	catch(...)
	{
		printf("Exception thrown without msg!");
	}

}

//1. Perform LP_VsaDataCapture();
//2. Perform LP_AnalyzePower();
//3. for loop query the following info with LP_GetScalarMeasurement():
//- ¡§start_sec¡¨
//- ¡§stop_sec¡¨
//4.for each packetDetected, call  LP_SelectCaptureRangeForAnalysis(startUs, lengthUs) first.
//5. Perform analysis option that you needed:
//  LP_Analyze11b();
//  LP_Analyze11ag();
//  LP_Analyze11n();
//
//For NxN system, same with before, set vsaNum or analysis order before doing analysis.

void Multi_Packet_EVM_Analysis()
{

	char   buffer[MAX_BUFFER_SIZE] = {'\0'};
	double dutPowerLevel = -10.0;		// Estimated RMS power level at IQTester
	double cableLoss=1;
	string tmpStr1 = "", modFileName = ""; // saveCapturedName="";
	char saveCapturedName[MAX_BUFFER_SIZE];

	try
	{
		set_color(CM_GREEN);
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(), "LP_Init()" );
		CheckReturnCode( LP_InitTester2("192.168.109.180","192.168.109.181"), "LP_InitTester()" );
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("%s\n", buffer);


		CheckReturnCode( LP_SetVsg(FREQ_HZ, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

		//--------------------//
		// Load waveform file //
		//--------------------//
		tmpStr1="11ac_80MHz_MCS9_2s_4000_byte";

		//modFileName="../mod/";
		modFileName+=tmpStr1;
		modFileName+=".mod";

		printf("\n Loading mode file %s\n", modFileName.c_str());
		CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );

		int iRun = 0, numRun = 10;
		while(iRun<numRun)
		{
			iRun++;
			printf("_________________________________________________\n");
			set_color(CM_YELLOW);
			printf("Run %d\n", iRun);
			set_color(CM_GREEN);

			//----------------//
			//   Setup VSA    //
			//----------------//
			int peakToAverageRatio = 10;

			CheckReturnCode( LP_SetVsa(FREQ_HZ, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?

			//----------------------//
			//  Perform VSA capture //
			//----------------------//
			double captureLength = 1000e-6;
			double samplingRate =150e6;

			CheckReturnCode( LP_VsaDataCapture(captureLength, 13, samplingRate), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?
			//---------------------------//
			//  Save sig file for debug  //
			//---------------------------//
			//CheckReturnCode( LP_SaveVsaSignalFile("FastTrack_Capture_54M_legacy_1.sig"), "LP_SaveVsaSignalFile()" );
			sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.sig", "FastTrack_Capture", tmpStr1.c_str(),iRun);
			CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

			//-------------------------//
			//  Retrieve Test Results  //
			//-------------------------//

			CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );

			//------------------------------------------//
			//  Retrieve start and length of each burst //
			//------------------------------------------//
			double resultStartSec = 0.0;
			double resultStopSec = 0.0;
			int index = 0;

			const int maxLength = 1000;
			double resultStartSecArray [maxLength] = {0.0};
			double resultStopSecArray [maxLength] = {0.0};
			double resultLengthArray [maxLength] = {0.0};
			const double stopSecCaptureLengthRelativeRatio = 0.001;
			for( index = 0 ;  ; index ++ )
			{
				resultStartSec = LP_GetScalarMeasurement("start_sec", index);
				resultStopSec = LP_GetScalarMeasurement("stop_sec", index);

				if( NA_NUMBER == resultStartSec || NA_NUMBER == resultStopSec || fabs(resultStopSec - captureLength)/captureLength < stopSecCaptureLengthRelativeRatio ){//need to exclude stop_sec near capture length, it is not a full packet
					break;
				}else{
					resultStartSecArray[index] = resultStartSec;
					resultStopSecArray[index] = resultStopSec;
					resultLengthArray[index] = resultStopSec - resultStartSec;
					if(true == DEBUG_PRINT)
					{
						set_color(CM_MEGNETA);
						printf("\n resultStartSec is %f, resultStopSec is %f, resultLength is %f, index is %d", resultStartSec, resultStopSec, resultStopSec - resultStartSec, index);
					}
				}
			}

			set_color(CM_YELLOW);
			printf("\n There are %d packets in this capture.\n", index);\
				set_color(CM_GREEN);

			int arraySize = index;
			//----------------------------------//
			//  perform analysis for each burst //
			//----------------------------------//
			vector<int>		prefOrderSignal;
			int j = 0;
			const int MAX_TESTER_NUM = 4;
			const int numberOfTesters = 2;

			vector< vector<double> >    evmAvgAll(numberOfTesters, vector<double>(index));
			//Default order: prefOrderSignal={1,2,3,4}
			prefOrderSignal.clear();
			for(j=0;j<MAX_TESTER_NUM;j++)
			{
				prefOrderSignal.push_back(j+1);
			}

			double avgEvm, maxEvm, minEvm;
			int i = 0, k = 0;
			for( k = 0 ; k < numberOfTesters ; k++ )
			{
				printf("\n EVM at VSA: %d \n", k+1);
				for( i = 0 ; i < arraySize ; i++ )
				{
					if( 0.0 != resultLengthArray[i] )
					{
						//we multiply by 1000.0 * 1000.0 to change the unit from sec to usec
						LP_SelectCaptureRangeForAnalysis(resultStartSecArray[i] * 1000.0 * 1000.0, resultLengthArray[i] * 1000.0 * 1000.0);
						//CheckReturnCode( LP_Analyze80211ac(), "LP_Analyze80211ac()" );
						LP_Analyze80211ac();
						double evm = LP_GetScalarMeasurement("evmAvgAll", k);
						if(true == DEBUG_PRINT)
						{
							set_color(CM_MEGNETA);
							printf("No.%3d packet's evm is: %.2f \n", i+1, evm);
						}

						evmAvgAll[k][i]=evm;
					}
					else
					{
						//length is not a valid number
					}
				}
				if (!evmLinearAverage(&evmAvgAll[k][0], arraySize, (double&)avgEvm,(double&)maxEvm,(double&)minEvm ))
				{
					set_color(CM_GREEN);
					printf("Statics of EVM at VSA %d\n	Avg: %.2f dB	Max: %.2f dB	Min: %.2f dB\n\n", k+1, avgEvm, maxEvm, minEvm);
				}
			}

		}

		//----------------------------//
		//   Disconnect the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Term(), "LP_Term()" );
	}
	catch(char *msg)
	{
		set_color(CM_RED);
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		set_color(CM_RED);
		printf("ERROR!");
	}
}


void WiFi_11ac_Loopback()
{
	char   buffer[MAX_BUFFER_SIZE];
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	double dutPowerLevel = -10;		// Estimated RMS power level at IQTester
	double cableLoss = 1; //dB of path loss
	double maxCaptureTime=0.0, minCaptureTime=1e6, maxAnalysisTime=0.0, minAnalysisTime=1e6;


	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//

		int testerControl = 1;		// use IQmeasure_IQapi_SCPI
		//int testerControl = 1;		// use IQmeasure_SCPI

		CheckReturnCode( LP_Init(IQTYPE_XEL, testerControl), "LP_Init()" );
		CheckReturnCode( LP_InitTester(IP_ADDR_IQXEL), "LP_InitTester()" );
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("\n%s\n\n", buffer);

		char saveCapturedName[MAX_BUFFER_SIZE] = "WiFi_11ac.iqvsg";


		//-------------//
		//  Setup VSG  //
		//-------------//
		CheckReturnCode( LP_SetVsg(FREQ_HZ_5G, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

		//--------------------//
		// Load waveform file //
		//--------------------//

		CheckReturnCode( LP_SetVsgModulation("../../iqvsg/WiFi_11AC_VHT80_S1_MCS9.iqvsg"), "LP_SetVsgModulation()" );

		//---------------//
		//  Turn on VSG  //
		//---------------//
		CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

		//------------------------------------------//
		//   Send packet, continuous transmission   //
		//------------------------------------------//
		int frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
		CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );


		//----------------//
		//   Setup VSA    //
		//----------------//
		int peakToAverageRatio = 10;

		double samplingRate = 160e6;

		CheckReturnCode( LP_SetVsa(FREQ_HZ_5G, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT), "LP_SetVsa()" );


		//----------------------//
		//  Perform VSA capture //
		//----------------------//
		double captureTime =500e-6;
		CheckReturnCode( LP_VsaDataCapture(captureTime, 13,160e6), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?

		//---------------------------//
		//  Save sig file for debug  //
		//---------------------------//

		CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

		//-------------------------//
		//  Perform Power analysis //
		//-------------------------//

		//		CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );
		if( testerControl == 0 )//IQAPI
			CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );
		else if( testerControl == 1 )//SCPI
			CheckReturnCode( LP_AnalyzePower(3.2e-6, 15.0), "LP_AnalyzePower()" );

		//-------------------------//
		//  Retrieve Test Results  //
		//-------------------------//
		double result;

		result = LP_GetScalarMeasurement("valid"); //result not right yet.
		printf("valid: %.0f\n", result);

		result = LP_GetScalarMeasurement("P_av_each_burst_dBm");
		printf("P_av_each_burst_dBm: %.2f dBm\n", result);
		result = LP_GetScalarMeasurement("P_av_each_burst");
		printf("P_av_each_burst: %.6f mw\n", result);

		result = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
		printf("P_pk_each_burst_dBm: %.2f dBm\n", result);
		result = LP_GetScalarMeasurement("P_pk_each_burst");
		printf("P_pk_each_burst: %.6f mw\n", result);

		result = LP_GetScalarMeasurement("P_av_all_dBm");
		printf("P_av_all_dBm: %.2f dBm\n", result);
		result = LP_GetScalarMeasurement("P_av_all");
		printf("P_av_all: %.6f mw\n", result);

		result = LP_GetScalarMeasurement("P_peak_all_dBm");
		printf("P_peak_all_dBm: %.2f dBm\n", result);
		result = LP_GetScalarMeasurement("P_peak_all");
		printf("P_peak_all: %.6f mw\n", result);

		result = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
		printf("P_av_no_gap_all_dBm: %.2f dBm\n", result);
		result = LP_GetScalarMeasurement("P_av_no_gap_all");
		printf("P_av_no_gap_all: %.6f mw\n", result);

		result = LP_GetScalarMeasurement("start_sec");
		printf("start_sec: %.6f sec\n", result);
		result = LP_GetScalarMeasurement("stop_sec");
		printf("stop_sec: %.6f sec\n", result);


		//-------------------------//
		//  Perform 11ac analysis //
		//-------------------------//

		CheckReturnCode( LP_Analyze80211ac(), "LP_Analyze80211ac()" );

		//-------------------------//
		//  Retrieve Test Results  //
		//-------------------------//

		result = LP_GetScalarMeasurement("packetDetection");
		if (result == 0)
		{
			printf("\nValid Packet Header Detection: %s \n", "FALSE");
		}
		else
			printf("\nValid Packet Header Detection: %s \n", "TRUE");

		result = LP_GetScalarMeasurement("acquisition");
		if (result == 0)
		{
			printf("Valid HT Packet Detection: %s \n", "FALSE");
		}
		else
			printf("Valid HT Packet Detection: %s \n", "TRUE");


		result = LP_GetScalarMeasurement("psduCRC");
		if (result == 1)
		{
			printf("PSDU CRC: %s \n", "PASS");
		}
		else
			printf("PSDU CRC: %s \n", "FAIL");

		result = LP_GetScalarMeasurement("demodulation");
		if (result == 0)
		{
			printf("Streams Demodulated: %s \n\n", "FALSE");
		}
		else
			printf("Streams Demodulated: %s \n\n", "TRUE");


		result = LP_GetScalarMeasurement("freqErrorHz");
		printf("Frequency Error: %.2f Hz\n", result);

		result = LP_GetScalarMeasurement("symClockErrorPpm");
		printf("Symbol Clock Error: %.2f ppm\n", result);

		result = LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll");
		printf("RMS Phase Noise: %.2f deg\n", result);

		result = LP_GetScalarMeasurement("IQImbal_amplDb");
		printf("IQ Imbalance Amp: %.2f dB \n", result);

		result = LP_GetScalarMeasurement("IQImbal_phaseDeg");
		printf("IQ Imbalance phase: %0.2f deg \n", result);

		result = LP_GetScalarMeasurement("dcLeakageDbc");
		printf("LO leakage: %.2f dBc\n\n", result);

		double bandwidthMhz = LP_GetScalarMeasurement("rateInfo_bandwidthMhz");
		printf("Signal Bandwidth: %0.2f MHz\n", bandwidthMhz);

		result = LP_GetScalarMeasurement("rateInfo_spatialStreams");
		printf("Number of Spatial Streams: %1.0f \n", result);

		result = LP_GetScalarMeasurement("rateInfo_spaceTimeStreams");
		printf("Number of SpaceTime Streams: %1.0f \n", result);

		result = LP_GetScalarMeasurement("rateInfo_dataRateMbps");
		printf("Datarate: %0.2f Mbps\n", result);

		result = LP_GetScalarMeasurement("rxRmsPowerDb");
		printf("Power_NoGap: %.3f dBm\n", result);


		result = LP_GetScalarMeasurement("evmAvgAll");
		printf("EVM Avg All: %.3f dB\n\n", result);


		result = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
		if (result <= 0)
		{
			printf("Channel Estimation returns an error\n");
		}
		else
		{
			//cacluate the spectrum flatness.
			printf("Channel Estimation returns a vector with %3.0f elements\n", result);
			int numSubcarrier = 256; //number of Subcarrier for 80MHz
			double  *carrierPwr;
			int centerStart, centerEnd, sideStart, sideEnd;
			//iqPwr  = (double *) malloc (result * sizeof (double));
			carrierPwr     = (double *) malloc (numSubcarrier * sizeof (double));
			double avgCenterPwr=0.0, avgSidePwr=0.0, avgPwrSpectral=0.0;
			double maxCarrierPwr = -999.9, minCenterCarrierPwr = 99.9, minSideCarrierPwr = 99.9;

			if (bandwidthMhz == 20.0)
			{
				numSubcarrier = 64;
				centerStart   = CARRIER_1;
				centerEnd     = CARRIER_16;
				sideStart     = CARRIER_17;
				sideEnd       = CARRIER_28;
			}
			else
			{
				if (bandwidthMhz == 40.0)
				{
					numSubcarrier = 128;
					centerStart   = CARRIER_2;
					centerEnd     = CARRIER_42;
					sideStart     = CARRIER_43;
					sideEnd       = CARRIER_58;
				}
				else
				{
					numSubcarrier = 256;
					centerStart   = CARRIER_2;
					centerEnd     = CARRIER_84;
					sideStart     = CARRIER_85;
					sideEnd       = CARRIER_122;
				}
			}
			if (result != numSubcarrier)
			{
				printf("Channel estimation results does not match number of subcarriers for %2.0f MHz signal.\n", bandwidthMhz);
			}
			else
			{
				for (int carrier=0;carrier<result;carrier++) //only one stream here.
				{
					carrierPwr[carrier] = bufferReal[carrier]*bufferReal[carrier]+ bufferImag[carrier]*bufferImag[carrier];
				}
				// average power in the center
				for ( int i=centerStart;i<=centerEnd;i++)
				{
					avgCenterPwr = avgCenterPwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
				}
				// average power on side lobe.
				for (int i=sideStart;i<=sideEnd;i++)
				{
					avgSidePwr = avgSidePwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
				}

				avgPwrSpectral = avgCenterPwr+ avgSidePwr;

				avgCenterPwr = avgCenterPwr/((centerEnd-centerStart+1)*2);
				if (0!=avgCenterPwr)
				{
					avgCenterPwr = 10.0 * log10 (avgCenterPwr);
				}
				else
				{
					avgCenterPwr= 0;
				}
				avgPwrSpectral = avgPwrSpectral/((sideEnd-centerStart+1)*2);
				if (0!=avgPwrSpectral)
				{
					avgPwrSpectral = 10.0 * log10 (avgPwrSpectral);
				}
				else
				{
					avgPwrSpectral = 0;
				}

				for (int carrier=0;carrier<numSubcarrier;carrier++)
				{
					if (0!=carrierPwr[carrier])
					{
						carrierPwr[carrier] = 10.0 * log10 (carrierPwr[carrier]);
						if (carrierPwr[carrier]>maxCarrierPwr)
						{
							maxCarrierPwr = carrierPwr[carrier];
						}
						if ( (carrier>=centerStart && carrier <=centerEnd) || (carrier>=numSubcarrier-centerEnd && carrier <= numSubcarrier-centerStart) )
						{
							if(carrierPwr[carrier]< minCenterCarrierPwr)
							{
								minCenterCarrierPwr = carrierPwr[carrier];
							}
						}
						if ((carrier>=sideStart && carrier <=sideEnd) || (carrier>=numSubcarrier-sideEnd && carrier <=numSubcarrier-sideStart) )
						{
							if(carrierPwr[carrier]< minSideCarrierPwr)
							{
								minSideCarrierPwr = carrierPwr[carrier];
							}
						}
					}
					else
					{
						carrierPwr[carrier] = 0;
					}
				}
				printf("The maximum deviation is %0.2f dB\n", maxCarrierPwr-avgCenterPwr);
				printf("The minimum deviation are %0.2f dB in the center, %0.2f dB in the far side\n", minCenterCarrierPwr-avgCenterPwr, minSideCarrierPwr-avgCenterPwr);
				printf("The LO Leakage is %0.2f dBc \n", carrierPwr[0] - avgPwrSpectral - 10 * log10((double)((sideEnd-centerStart+1)*2)) );
			}

		}

		printf("11AC VHT Sig field Info:\n");
		result = LP_GetScalarMeasurement("VHTSigA1Bandwidth");
		printf("VHTSigA1Bandwidth=%1.0f. Means: %3.0f MHz\n", result, pow(2,result)*20);

		result = LP_GetScalarMeasurement("VHTSigA1Stbc");
		printf("VHTSigA1Stbc=%1.0f. Means: ", result);
		if(result == 0)
		{
			printf("STBC is not enabled for all streams.\n");
		}
		else
			printf("STBC is enabled for all streams.\n");

		result = LP_GetScalarMeasurement("VHTSigA2ShortGI");
		printf("VHTSigA2ShortGI=%1.0f\n", result);
		result = LP_GetScalarMeasurement("VHTSigA2AdvancedCoding");
		printf("VHTSigA2AdvancedCoding=%1.0f\n", result);
		result = LP_GetScalarMeasurement("VHTSigA2McsIndex");
		printf("VHTSigA2McsIndex=%1.0f\n", result);

		result = LP_GetScalarMeasurement("vhtSigBFieldCRC");
		printf("vhtSigBFieldCRC=%1.0f\n\n", result);

		//----------------//
		//  Turn off VSG  //
		//----------------//
		CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
		printf("\n");

	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
	ReadLogFiles;
}

void WiFi_11ac_MIMO_Loopback()
{
	//char   buffer[MAX_BUFFER_SIZE];
	double dutPowerLevel = -10;		// Estimated RMS power level at IQTester
	//double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	double cableLoss = 4; //dB of path loss


	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		//CheckReturnCode( LP_Init(), "LP_Init()" );
		//const int ciTesterControlMode = 1;
		CheckReturnCode( LP_Init(IQTYPE_XEL, ciTesterControlMode), "LP_Init()" );
		CheckReturnCode( LP_InitTester(IP_ADDR_IQXEL), "LP_InitTester()" );

		char * pcBuffer = NULL;
		pcBuffer = new char [MAX_BUFFER_SIZE];
		if (LP_GetVersion(pcBuffer, MAX_BUFFER_SIZE)==true)
			printf("\n%s\n\n", pcBuffer);
		SAFE_DELETE_ARRAY(pcBuffer);

		int dynamicRange = 3;
		int numRun=1;
		//int numRun = 30;

		//double maxPWRDynamicRange[MAX_BUFFER_SIZE], minPWRDynamicRange[MAX_BUFFER_SIZE], avgPWRDynamicRange[MAX_BUFFER_SIZE];
		//double maxEVMDynamicRange[MAX_BUFFER_SIZE], minEVMDynamicRange[MAX_BUFFER_SIZE], avgEVMDynamicRange[MAX_BUFFER_SIZE];

		// Use new and delete to avoid "stack overflow".
		double * pdMaxPWRDynamicRange = NULL; pdMaxPWRDynamicRange = new double [MAX_BUFFER_SIZE];
		double * pdMinPWRDynamicRange = NULL; pdMinPWRDynamicRange = new double [MAX_BUFFER_SIZE];
		double * pdAvgPWRDynamicRange = NULL; pdAvgPWRDynamicRange = new double [MAX_BUFFER_SIZE];
		double * pdMaxEVMDynamicRange = NULL; pdMaxEVMDynamicRange = new double [MAX_BUFFER_SIZE];
		double * pdMinEVMDynamicRange = NULL; pdMinEVMDynamicRange = new double [MAX_BUFFER_SIZE];
		double * pdAvgEVMDynamicRange = NULL; pdAvgEVMDynamicRange = new double [MAX_BUFFER_SIZE];


		string tmpStr1 = "", modFileName = "";
		char saveCapturedName[MAX_BUFFER_SIZE];

		//-------------//
		//  Setup VSG  //
		//-------------//

		CheckReturnCode( LP_SetVsg(FREQ_HZ, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

		//--------------------//
		// Load waveform file //
		//--------------------//
		//tmpStr1="WIFI_AC_BW80_SS1_MCS9_BCC_Fs160M";

		//modFileName="../mod/";
		modFileName="../mod/";//Luke

		tmpStr1 = "11AC_MCS8_1strm_80MHz_2550bytes_pgap20us_LongGI_AMPDUoff_Fs160";

		modFileName+=tmpStr1;
		modFileName+=".iqvsg";

		printf("\nLoading mod file %s\n",modFileName.c_str());
		CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );


		//double bufferRealX[MAX_BUFFER_SIZE], bufferImagX[MAX_BUFFER_SIZE];
		//double bufferRealY[MAX_BUFFER_SIZE], bufferImagY[MAX_BUFFER_SIZE];

		double * pdBufferRealX = NULL; pdBufferRealX = new double [MAX_BUFFER_SIZE];
		double * pdBufferRealY = NULL; pdBufferRealY = new double [MAX_BUFFER_SIZE];
		double * pdBufferImagX = NULL; pdBufferImagX = new double [MAX_BUFFER_SIZE];
		double * pdBufferImagY = NULL; pdBufferImagY = new double [MAX_BUFFER_SIZE];

		int peakToAverageRatio = 10;
		int frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
		int numAnalysisErr = 0, iRun=0;


		double freqHz = 5540e6;
		double samplingRate = 150e6;
		double maxPWR=-999.9, minPWR=999.9,avgPWR=0;
		double maxEVM=-999.9, minEVM=999.9,avgEVM=0;
		double captureTime = 200e-6;
		double result = 0.0;


		double * pdBufferReal = NULL;
		double * pdBufferImag = NULL;
		pdBufferReal = new double [MAX_BUFFER_SIZE];
		pdBufferImag = new double [MAX_BUFFER_SIZE];


		for (int iPwrErr=0-dynamicRange; iPwrErr<=dynamicRange+6; iPwrErr++ )
		{
			maxPWR=-999.9, minPWR=999.9,avgPWR=0;
			maxEVM=-999.9, minEVM=999.9,avgEVM=0;
			numAnalysisErr = 0, iRun=0;

			printf("\n VSA Amplitude Level is offset by %d dB\n",iPwrErr);
			while(iRun<numRun)
			{
				iRun++;
				printf("_________________________________________________\n");
				printf("Run %d\n", iRun);
				//---------------//
				//  Turn on VSG  //
				//---------------//
				//CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				//------------------------------------------//
				//   Send packet, continuous transmission   //
				//------------------------------------------//
				frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
				CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );


				//----------------//
				//   Setup VSA    //
				//----------------//
				peakToAverageRatio = 10;
				samplingRate = 150e6;

				if( iRun%3 == 0  )
					freqHz = 5540e6;
				else if( iRun%3 == 1 )
					freqHz = 5180e6;
				else if( iRun%3 == 2 )
					freqHz = 5320e6;

				CheckReturnCode( LP_SetVsa(freqHz, (dutPowerLevel-cableLoss+peakToAverageRatio+iPwrErr), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" );
				//----------------------//
				//  Perform VSA capture //
				//----------------------//
				captureTime = 200e-6;
				CheckReturnCode( LP_VsaDataCapture(captureTime, 13, samplingRate), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?

				//---------------------------//
				//  Save sig file for debug  //
				//---------------------------//

				//sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s_%s_offset_%ddB_%d.sig", "FastTrack_Capture", tmpStr1.c_str(),iPwrErr,iRun);
				sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s_offset_%ddB-%d.iqvsa", "FastTrack_Capture", tmpStr1.c_str(),iPwrErr,iRun);
				CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

				//------------------------//
				//  Perform 11ac analysis //
				//------------------------//
				printf("\nUsing 11AC analysis, iqapiAnalysis11ac\n");
				CheckReturnCode( LP_Analyze80211ac(), "LP_Analyze80211ac()" );

				//-------------------------//
				//  Retrieve Test Results  //
				//-------------------------//
				// 11ac result
				result = LP_GetScalarMeasurement("packetDetection");
				if (result == 0)
				{
					printf("\nValid Packet Header Detection: %s \n", "FALSE");
				}
				else
					printf("\nValid Packet Header Detection: %s \n", "TRUE");

				result = LP_GetScalarMeasurement("acquisition");
				if (result == 0)
				{
					printf("Valid HT Packet Detection: %s \n", "FALSE");
				}
				else
					printf("Valid HT Packet Detection: %s \n", "TRUE");


				result = LP_GetScalarMeasurement("psduCRC");
				if (result == 0)
				{
					printf("PSDU CRC: %s \n", "PASS");
				}
				else
				{
					printf("PSDU CRC: %s \n", "FAIL");
					printf("\n\n\nWanning: Analysis has error \n\n\n");
				}


				result = LP_GetScalarMeasurement("demodulation");
				if (result == 0)
				{
					printf("Streams Demodulated: %s \n\n", "FALSE");
				}
				else
					printf("Streams Demodulated: %s \n\n", "TRUE");

				//Not working for 1 stream signal.
				// result = LP_GetScalarMeasurement("isolationDb");
				// printf("isolationDb=%0.2f \n", result);

				result = LP_GetScalarMeasurement("freqErrorHz");
				printf("Frequency Error: %.2f Hz\n", result);

				result = LP_GetScalarMeasurement("symClockErrorPpm");
				printf("Symbol Clock Error: %.2f ppm\n", result);

				result = LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll");
				printf("RMS Phase Noise: %.2f deg\n", result);

				result = LP_GetScalarMeasurement("IQImbal_amplDb");
				printf("IQ Imbalance Amp: %.2f dB \n", result);

				result = LP_GetScalarMeasurement("IQImbal_phaseDeg");
				printf("IQ Imbalance phase: %0.2f deg \n", result);

				result = LP_GetScalarMeasurement("dcLeakageDbc");
				printf("LO leakage: %.2f dBc\n\n", result);

				double bandwidthMhz = LP_GetScalarMeasurement("rateInfo_bandwidthMhz");
				printf("Signal Bandwidth: %0.2f MHz\n", bandwidthMhz);

				double numStreams = LP_GetScalarMeasurement("rateInfo_spatialStreams");
				printf("Number of Spatial Streams: %1.0f \n", numStreams);
				numStreams = 1;


				result = LP_GetScalarMeasurement("rateInfo_spaceTimeStreams");
				printf("Number of SpaceTime Streams: %1.0f \n", result);

				result = LP_GetScalarMeasurement("rateInfo_dataRateMbps");
				printf("Datarate: %0.2f Mbps\n", result);
				if(result<540) //depends on current datarate of the mode files.
				{
					printf("\n\n\nWanning: Analysis has error \n\n\n");
					numAnalysisErr++;
				}

				result = LP_GetScalarMeasurement("rxRmsPowerDb");
				printf("Power_NoGap: %.3f dBm\n", result);
				if (result > maxPWR)
				{
					maxPWR=result;
				}
				if(result<minPWR)
				{
					minPWR=result;
				}
				avgPWR+=result;

				result = LP_GetScalarMeasurement("evmAvgAll");
				printf("EVM Avg All: %.3f dB\n\n", result);
				if (result > maxEVM)
				{
					maxEVM=result;
				}
				if(result<minEVM)
				{
					minEVM=result;
				}
				avgEVM+=result; //simple average in dB domain


				result = ::LP_GetVectorMeasurement("channelEst", pdBufferReal, pdBufferImag, MAX_BUFFER_SIZE);
				if (result <= 0)
				{
					printf("Channel Estimation returns an error\n");
				}
				else
				{
					//cacluate the spectrum flatness.
					printf("Channel Estimation returns a vector with %3.0f elements\n", result);
					int numSubcarrier = 256; //number of Subcarrier for 80MHz
					double  *carrierPwr;
					int centerStart, centerEnd, sideStart, sideEnd;
					//iqPwr  = (double *) malloc (result * sizeof (double));
					carrierPwr     = (double *) malloc (numSubcarrier * sizeof (double));
					double avgCenterPwr=0.0, avgSidePwr=0.0, avgPwrSpectral=0.0, allPwrSpectral=0.0;
					double maxCarrierPwr = -999.9, minCenterCarrierPwr = 99.9, minSideCarrierPwr = 99.9;

					if (bandwidthMhz == 20.0)
					{
						numSubcarrier = 64;
						centerStart   = CARRIER_1;
						centerEnd     = CARRIER_16;
						sideStart     = CARRIER_17;
						sideEnd       = CARRIER_28;
					}
					else
					{
						if (bandwidthMhz == 40.0)
						{
							numSubcarrier = 128;
							centerStart   = CARRIER_2;
							centerEnd     = CARRIER_42;
							sideStart     = CARRIER_43;
							sideEnd       = CARRIER_58;
						}
						else
						{
							numSubcarrier = 256;
							centerStart   = CARRIER_2;
							centerEnd     = CARRIER_84;
							sideStart     = CARRIER_85;
							sideEnd       = CARRIER_122;
						}
					}
					double numVSA = numStreams;
					if (result != numSubcarrier*numVSA*numStreams)
					{
						printf("Channel estimation results does not match number of subcarriers for %2.0f MHz signal.\n", bandwidthMhz);
					}
					else
					{
						int iCarrier=0;
						for (int iNumVsa = 0; iNumVsa<numVSA; iNumVsa++) //VSA based
						{
							allPwrSpectral=0.0;
							for (int carrier=0;carrier<numSubcarrier;carrier++)
							{
								carrierPwr[carrier]=0;
								////for (int iNumStream = 0; iNumStream<numStreams; iNumStream++)
								////{
								int iNumStream = iNumVsa; //only direct mapping result is meaningful
								iCarrier = (int)((numSubcarrier*numStreams*iNumVsa)+(numStreams*carrier)+iNumStream);
								carrierPwr[carrier] = carrierPwr[carrier]+pdBufferReal[iCarrier]*pdBufferReal[iCarrier]+ pdBufferImag[iCarrier]*pdBufferImag[iCarrier];
								////}
								//for (int iNumStream = 0; iNumStream<numStreams; iNumStream++)
								//{
								//	iCarrier = (int)((numSubcarrier*numStreams*iNumVsa)+(numStreams*carrier)+iNumStream);
								//	carrierPwr[carrier] = carrierPwr[carrier]+bufferReal[iCarrier]*bufferReal[iCarrier]+ bufferImag[iCarrier]*bufferImag[iCarrier];
								//}

							}

							// average power in the center
							for ( int i=centerStart;i<=centerEnd;i++)
							{
								avgCenterPwr = avgCenterPwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
							}
							// average power on side lobe.
							for (int i=sideStart;i<=sideEnd;i++)
							{
								avgSidePwr = avgSidePwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
							}

							avgPwrSpectral = avgCenterPwr+ avgSidePwr;

							avgCenterPwr = avgCenterPwr/((centerEnd-centerStart+1)*2);
							if (0!=avgCenterPwr)
							{
								avgCenterPwr = 10.0 * log10 (avgCenterPwr);
							}
							else
							{
								avgCenterPwr= 0;
							}
							allPwrSpectral=avgPwrSpectral;
							avgPwrSpectral = avgPwrSpectral/((sideEnd-centerStart+1)*2);
							if (0!=avgPwrSpectral)
							{
								avgPwrSpectral = 10.0 * log10 (avgPwrSpectral);
							}
							else
							{
								avgPwrSpectral = 0;
							}

							for (int carrier=0;carrier<numSubcarrier;carrier++)
							{
								if (0!=carrierPwr[carrier])
								{
									carrierPwr[carrier] = 10.0 * log10 (carrierPwr[carrier]);
									if (carrierPwr[carrier]>maxCarrierPwr)
									{
										maxCarrierPwr = carrierPwr[carrier];
									}
									if ( (carrier>=centerStart && carrier <=centerEnd) || (carrier>=numSubcarrier-centerEnd && carrier <= numSubcarrier-centerStart) )
									{
										if(carrierPwr[carrier]< minCenterCarrierPwr)
										{
											minCenterCarrierPwr = carrierPwr[carrier];
										}
									}
									if ((carrier>=sideStart && carrier <=sideEnd) || (carrier>=numSubcarrier-sideEnd && carrier <=numSubcarrier-sideStart) )
									{
										if(carrierPwr[carrier]< minSideCarrierPwr)
										{
											minSideCarrierPwr = carrierPwr[carrier];
										}
									}
								}
								else
								{
									carrierPwr[carrier] = 0;
								}
							}
							printf("The spectral flatness results of VSA %d are:\n", iNumVsa);
							printf("   The maximum deviation is %0.2f dB\n", maxCarrierPwr-avgCenterPwr);
							printf("   The minimum deviation are %0.2f dB in the center, %0.2f dB in the far side\n", minCenterCarrierPwr-avgCenterPwr, minSideCarrierPwr-avgCenterPwr);
							printf("   The LO Leakage is %0.2f dBc \n", carrierPwr[0] - avgPwrSpectral - 10 * log10((double)((sideEnd-centerStart+1)*2)) );
							allPwrSpectral = 10.0 * log10 (allPwrSpectral);
							printf("   The LO Leakage is %0.2f dBc \n", carrierPwr[0] - allPwrSpectral );
						}
					}

				}


				printf("11AC VHT Sig field Info:\n");
				result = LP_GetScalarMeasurement("VHTSigA1Bandwidth");
				printf("VHTSigA1Bandwidth=%1.0f. Means: %3.0f MHz\n", result, pow(2,result)*20);

				result = LP_GetScalarMeasurement("VHTSigA1Stbc");
				printf("VHTSigA1Stbc=%1.0f. Means: ", result);
				if(result == 0)
				{
					printf("STBC is not enabled for all streams.\n");
				}
				else
					printf("STBC is enabled for all streams.\n");


				result = LP_GetScalarMeasurement("VHTSigA2ShortGI");
				printf("VHTSigA2ShortGI=%1.0f\n", result);
				result = LP_GetScalarMeasurement("VHTSigA2AdvancedCoding");
				printf("VHTSigA2AdvancedCoding=%1.0f\n", result);
				result = LP_GetScalarMeasurement("VHTSigA2McsIndex");
				printf("VHTSigA2McsIndex=%1.0f\n", result);

				result = LP_GetScalarMeasurement("VHTSigBFieldCRC");
				printf("VHTSigBFieldCRC=%1.0f\n\n", result);


				//Perform Wideband Capture for VHT80
				captureTime = 2000e-6;
				int vht80MaskMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
				CheckReturnCode( LP_SetVsa(FREQ_HZ, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?
				CheckReturnCode( LP_VsaDataCapture(captureTime, 13, samplingRate, vht80MaskMode), "LP_VsaDataCapture()" );
				CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

				int    vsaAnalysisIndex = 0;

				while(vsaAnalysisIndex < 2)
				{
					//Step1: Set Analysis Parameter "vsaNum"
					//if(vht80MaskMode == 2)
					//{
					//	//AnalyzeVHT80Mask
					//	CheckReturnCode(LP_SetAnalysisParameterInteger("AnalyzeVHT80Mask", "vsaNum", vsaAnalysisIndex+1), "LP_SetAnalysisParameterInteger()" );
					//}
					//else
					//{
					//	//not implemented;
					//	CheckReturnCode(LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", vsaAnalysisIndex+1), "LP_SetAnalysisParameterInteger()" );
					//}

					//Step2: Perform Analysis
					if (vht80MaskMode == 2)
						// VHT80Mask mode
					{
						printf("\n Analyzing VHT80MHz Wideband Mask\n");
						CheckReturnCode( LP_AnalyzeVHT80Mask(), "LP_AnalyzeVHT80Mask()" );
					}
					else // HT20
					{
						printf("\n Analyzing normal Mask\n");
						CheckReturnCode( LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );
					}

					int bufferSizeX = ::LP_GetVectorMeasurement("x", pdBufferRealX, pdBufferImagX, MAX_BUFFER_SIZE);
					int bufferSizeY = ::LP_GetVectorMeasurement("y", pdBufferRealY, pdBufferImagY, MAX_BUFFER_SIZE);

					if(bufferSizeX == bufferSizeY)
					{
						CheckReturnCode( LP_Plot(1, pdBufferRealX, pdBufferRealY, bufferSizeX, ".-", "FFT", "Freq", "Power", 1), "LP_Plot()");
					}
					else
					{
						throw "Length of X and Y do not match!";
					}

					printf("\n VHT80MHz Wideband mask starts at %4.0f MHz results \n", pdBufferRealX[0]/(1e6) );
					vsaAnalysisIndex ++;
				}

				//----------------//
				//  Turn off VSG  //
				//----------------//
				//CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
				printf("\n");

			}
			printf("Statistics of %d run:\n",numRun);
			pdAvgPWRDynamicRange[iPwrErr+dynamicRange] = avgPWR/numRun;
			pdMaxPWRDynamicRange[iPwrErr+dynamicRange] = maxPWR;
			pdMinPWRDynamicRange[iPwrErr+dynamicRange] = minPWR;
			pdAvgEVMDynamicRange[iPwrErr+dynamicRange] = avgEVM/numRun;
			pdMaxEVMDynamicRange[iPwrErr+dynamicRange] = maxEVM;
			pdMinEVMDynamicRange[iPwrErr+dynamicRange] = minEVM;
			printf("Power_NoGap\n	Avg: %.2f dBm	Max: %.2f dBm	Min: %.2f dBm\n", pdAvgPWRDynamicRange[iPwrErr+dynamicRange], maxPWR, minPWR);
			printf("EVM\n	Avg: %.2f dB	Max: %.2f dB	Min: %.2f dB\n\n", pdAvgEVMDynamicRange[iPwrErr+dynamicRange],maxEVM,minEVM);

			if (numAnalysisErr>0)
			{
				printf("\n\n\nWanning: Analysis has been wrong for %d times out of %d run \n\n\n", numAnalysisErr, numRun);
			}
		}//end of for loops

		SAFE_DELETE_ARRAY(pdBufferReal);
		SAFE_DELETE_ARRAY(pdBufferImag);

		SAFE_DELETE_ARRAY(pdBufferRealX);
		SAFE_DELETE_ARRAY(pdBufferRealY);
		SAFE_DELETE_ARRAY(pdBufferImagX);
		SAFE_DELETE_ARRAY(pdBufferImagY);


		printf("Statistics of %d run:\n",numRun);
		printf("\n\nAmpLevelOffSet, Power_NoGap Average (dBm), Maximum(dBm), Minimum(dBm)\n");
		for (int iPwrErr=0-dynamicRange; iPwrErr<=dynamicRange+6; iPwrErr++ )
		{
			printf("%d, %.2f, %.2f, %.2f \n", iPwrErr, pdAvgPWRDynamicRange[iPwrErr+dynamicRange], pdMaxPWRDynamicRange[iPwrErr+dynamicRange], pdMinPWRDynamicRange[iPwrErr+dynamicRange]);
		}
		printf("\n\nAmpLevelOffSet, EVM Average (dB), Maximum(dB), Minimum(dB)\n");
		for (int iPwrErr=0-dynamicRange; iPwrErr<=dynamicRange+6; iPwrErr++ )
		{
			printf("%d, %.2f, %.2f, %.2f \n", iPwrErr, pdAvgEVMDynamicRange[iPwrErr+dynamicRange], pdMaxEVMDynamicRange[iPwrErr+dynamicRange], pdMinEVMDynamicRange[iPwrErr+dynamicRange]);
		}

		SAFE_DELETE_ARRAY(pdAvgPWRDynamicRange);
		SAFE_DELETE_ARRAY(pdMaxPWRDynamicRange);
		SAFE_DELETE_ARRAY(pdMinPWRDynamicRange);
		SAFE_DELETE_ARRAY(pdAvgEVMDynamicRange);
		SAFE_DELETE_ARRAY(pdMaxEVMDynamicRange);
		SAFE_DELETE_ARRAY(pdMinEVMDynamicRange);

	}

	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
}



void WiFi_11ac_Composite_Loopback()
{

	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester

	char tmpCompEvmStr[MAX_BUFFER_SIZE], tmpRefStr[MAX_BUFFER_SIZE];


	try
	{
		char   buffer[MAX_BUFFER_SIZE];
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(IQTYPE_XEL, ciTesterControlMode), "LP_Init()" );
		CheckReturnCode( LP_InitTester(IP_ADDR_IQXEL), "LP_InitTester()" );

		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("%s\n", buffer);

		//char tmpCompEvmStr[MAX_BUFFER_SIZE], tmpRefStr[MAX_BUFFER_SIZE];

		double resultComp = 0.0;
		//while(1)
		{

			for (int numCompStr=2;  numCompStr<4; numCompStr++)
			{

				//string tmpCompEvmStr1 = "";
				double resultComp;
				for (int iMcs = 0; iMcs<10; iMcs++)
					//for (int iMcs = 9 ; iMcs>=0 ; iMcs--)
				{
					if(numCompStr==3 && iMcs==6)
					{
						//not allow by standard
					}
					else
					{
						//sprintf_s(tmpCompEvmStr, MAX_BUFFER_SIZE, "../com_capture/%s%d%s%d.iqvsa", "WiFi_11AC_VHT80_S", numCompStr,"_MCS", iMcs);
						//sprintf_s(tmpCompEvmStr, MAX_BUFFER_SIZE, "user/%s%d%s%d.iqvsa", "WiFi_11AC_VHT80_S", numCompStr,"_MCS", iMcs);
						//sprintf_s(tmpCompEvmStr, MAX_BUFFER_SIZE, "../mod/%s%d%s%d.iqvsa", "WiFi_11AC_VHT80_S", numCompStr,"_MCS", iMcs);
						sprintf_s(tmpCompEvmStr, MAX_BUFFER_SIZE, "../../iqvsg/%s%d%s%d.iqvsa", "WiFi_11AC_VHT80_S", numCompStr,"_MCS", iMcs);//Luke
						printf("\nLoading composite capture file %s\n",tmpCompEvmStr);

						CheckReturnCode( LP_LoadVsaSignalFile(tmpCompEvmStr), "LP_LoadVsaSignalFile()" );

						//sprintf_s(tmpRefStr, MAX_BUFFER_SIZE, "../com_capture/%s%d%s%d.iqref", "WiFi_11AC_VHT80_S", numCompStr,"_MCS", iMcs);
						//sprintf_s(tmpRefStr, MAX_BUFFER_SIZE, "../mod/%s%d%s%d.iqref", "WiFi_11AC_VHT80_S", numCompStr,"_MCS", iMcs);
						sprintf_s(tmpCompEvmStr, MAX_BUFFER_SIZE, "../../iqvsg/%s%d%s%d.iqvsa", "WiFi_11AC_VHT80_S", numCompStr,"_MCS", iMcs);//Luke
						printf("Perform composite EVM analysis using ref file %s\n",tmpRefStr);
						//CheckReturnCode( LP_Analyze80211ac("composite", 1, 1, 0, 0, 1, 2,tmpRefStr,0), "LP_Analyze80211ac()" );
						CheckReturnCode( LP_Analyze80211ac("composite", 1, 1, 0, 0, 0,IQV_FREQUENCY_CORR_LTF,tmpRefStr,0), "LP_Analyze80211ac()" );
						//CheckReturnCode( LP_Analyze80211ac("composite", 0, 0, 0, 0, 0, 2,tmpRefStr,0), "LP_Analyze80211ac()" );


						//resultComp = LP_GetScalarMeasurement("packetDetection");
						//if (resultComp == 0)
						//{
						//	printf("\nValid Packet Header Detection: %s \n", "FALSE");
						//}
						//else
						//	printf("\nValid Packet Header Detection: %s \n", "TRUE");

						//resultComp = LP_GetScalarMeasurement("acquisition");
						//if (resultComp == 0)
						//{
						//	printf("Valid HT Packet Detection: %s \n", "FALSE");
						//}
						//else
						//	printf("Valid HT Packet Detection: %s \n", "TRUE");


						//resultComp = LP_GetScalarMeasurement("psduCRC");
						//if (resultComp == 1)
						//{
						//	printf("PSDU CRC: %s \n", "PASS");
						//}
						//else
						//	printf("PSDU CRC: %s \n", "FAIL");

						//resultComp = LP_GetScalarMeasurement("demodulation");
						//if (resultComp == 0)
						//{
						//	printf("Streams Demodulated: %s \n\n", "FALSE");
						//}
						//else
						//	printf("Streams Demodulated: %s \n\n", "TRUE");


						//resultComp = LP_GetScalarMeasurement("freqErrorHz");
						//printf("Frequency Error: %.2f Hz\n", resultComp);

						//resultComp = LP_GetScalarMeasurement("symClockErrorPpm");
						//printf("Symbol Clock Error: %.2f ppm\n", resultComp);

						//resultComp = LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll");
						//printf("RMS Phase Noise: %.2f deg\n", resultComp);

						//resultComp = LP_GetScalarMeasurement("IQImbal_amplDb");
						//printf("IQ Imbalance Amp: %.2f dB \n", resultComp);

						//resultComp = LP_GetScalarMeasurement("IQImbal_phaseDeg");
						//printf("IQ Imbalance phase: %0.2f deg \n", resultComp);

						//resultComp = LP_GetScalarMeasurement("dcLeakageDbc");
						//printf("LO leakage: %.2f dBc\n\n", resultComp);

						//double bandwidthMhz = LP_GetScalarMeasurement("rateInfo_bandwidthMhz");
						//printf("Signal Bandwidth: %0.2f MHz\n", bandwidthMhz);

						//resultComp = LP_GetScalarMeasurement("rateInfo_spatialStreams");
						//printf("Number of Spatial Streams: %1.0f \n", resultComp);

						//resultComp = LP_GetScalarMeasurement("rateInfo_spaceTimeStreams");
						//printf("Number of SpaceTime Streams: %1.0f \n", resultComp);
						//
						//resultComp = LP_GetScalarMeasurement("rateInfo_dataRateMbps");
						//printf("Datarate: %0.2f Mbps\n", resultComp);

						//resultComp = LP_GetScalarMeasurement("rxRmsPowerDb");
						//printf("Power_NoGap: %.3f dBm\n", resultComp);

						resultComp = LP_GetScalarMeasurement("evmAvgAll");
						printf("EVM Avg All: %.3f dB\n\n", resultComp);
					}
				}
			}


			printf("Press 'X' key to exit or any key to repeat the test..................\n");

			char ch = toupper(_getch());
			//if (ch=='X' || ch==ASCII_ESC)
			//	break;
		}
		//----------------------------//
		//   Disconnect the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Term(), "LP_Term()" );
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}

}

void Evm_Test()
{
	char   buffer[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester

	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(), "LP_Init()" );
		CheckReturnCode( LP_InitTester(IP_ADDR), "LP_InitTester()" );
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("%s\n", buffer);

		while(1)
		{
			CheckReturnCode( LP_LoadVsaSignalFile("../mod/test_MCS15.mod"), "LP_LoadVsaSignalFile()" );
			CheckReturnCode( LP_Analyze80211n("EWC", "nxn"), "LP_Analyze80211n()" );

			//-------------------------//
			//  Retrieve Test Results  //
			//-------------------------//
			double result;

			result = LP_GetScalarMeasurement("rateInfo_dataRateMbps", 0);
			printf("\nData Rate=%.3f Mbps\n", result);
			result = LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);
			printf("Spatial Streams=%d stream\n", (int)result);
			int spatialStreams = (int)result;


			int testerNumber = 3, validVsaNum = 2;

			for(int i=0;i<testerNumber;i++)  //not a good way to implement, what if the return value is changed, we should use spatial_stream.
			{
				result = LP_GetScalarMeasurement("evmAvgAll", i);
				printf("\nEVM_All_%d=%.3f dB\n", i, result);

				for(int j=0;j<spatialStreams;j++)
				{
					if(i != j)
					{
						result = 0.0 - ::LP_GetScalarMeasurement("isolationDb", (i*validVsaNum)+j);
						printf("IsolationDB_%d=%.3f DB\n", j, result);
					}
					else
					{
						continue;
					}
				}

				//Matrix: Nstream x NVsa, Stream in each VSA, order by prefOrderSignal
				for(int j=0;j<validVsaNum;j++)
				{
					result = ::LP_GetScalarMeasurement("rxRmsPowerDb", (i*validVsaNum)+j);
					printf("Stream_%d RMS Power=%.3f dBm\n", j, result);
				}

				//Matrix: Nstream x NVsa, VSA in each stream
				for(int j=0;j<validVsaNum;j++)
				{
					result = ::LP_GetScalarMeasurement("rxRmsPowerDb", i*validVsaNum+j);
					printf("VSA_%d RMS Power=%.3f dBm\n", j, result);
				}

				//STREAM_POWER_IN_VSA, streamInVsa
				for(int j=0;j<testerNumber ;j++)
				{
					result = ::LP_GetScalarMeasurement("rxRmsPowerDb", i*validVsaNum+j);
					printf("Stream_%d In VSA_%d Power=%.3f dBm\n", j, i, result);
				}
			}


			printf("Press 'X' key to exit or any key to repeat the test..................\n");

			char ch = toupper(_getch());
			if (ch=='X' || ch==ASCII_ESC)
				break;
		}
		//----------------------------//
		//   Disconnect the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Term(), "LP_Term()" );
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}
}

void CW_Test()
{
	char   buffer[MAX_BUFFER_SIZE];

	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(), "LP_Init()" );
		CheckReturnCode( LP_InitTester("192.168.109.172"), "LP_InitTester()" );
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("%s\n", buffer);

		double vsgPowerdBm = -10;
		double freqHz = 2442e6;
		double offsetFreqHz = 0;

		while(1)
		{
			CheckReturnCode( LP_SetVsgCw(freqHz, offsetFreqHz, vsgPowerdBm, PORT_RIGHT), "LP_SetVsg()" );

			//---------------//
			//  Turn on VSG  //
			//---------------//
			CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

			while(1)
			{
				printf("Press 'X' key to stop VSG..................\n");

				char ch = toupper(_getch());
				if (ch=='X' || ch==ASCII_ESC)
					break;
			}

			CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );

			//----------------------------//
			//   Disconnect the IQTester  //
			//----------------------------//
			CheckReturnCode( LP_Term(), "LP_Term()" );

			while(1)
			{
				printf("Press 'X' key to exit or any key to repeat the test..................\n");

				char ch = toupper(_getch());
				if (ch=='X' || ch==ASCII_ESC)
					break;
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
}

void	Glonass_Test()
{
	char   buffer[MAX_BUFFER_SIZE];

	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(0), "LP_Init()" );
		CheckReturnCode( LP_InitTester(IP_ADDR), "LP_InitTester()" );
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("%s\n", buffer);

		printf("\r\n\r\n"
				"=========================================\r\n"
				"=             TESTING GLONASS\r\n"
				"=========================================\r\n");


		printf("\r\n[Activating GPS] ... ");

		printf("OK\r\n");
		CheckReturnCode( LP_Glonass_SetActivate(), "LP_Glonass_SetActivate()" );

		CheckReturnCode( LP_PrintGlonassParameters(), "LP_PrintGlonassParameters()" );

		CheckReturnCode( LP_Glonass_SetRfOutput(IQV_RF_OUTPUT_ON), "LP_Glonass_SetRfOutput()" );
		CheckReturnCode( LP_Glonass_SetOperationMode(IQV_OPERATION_MODE_CW), "LP_Glonass_SetOperationMode()" );
		CheckReturnCode( LP_Glonass_SetPower(-80), "LP_Glonass_SetPower()" );

		// IQV_GLONASS_FREQUENCY_CHANNEL is -7
		CheckReturnCode( LP_Glonass_SetFrequency( -7 ), "LP_Glonass_SetFrequency()" );

		// IQV_GLONASS_FREQUENCY_CHANNEL_1602000000Hz is 0
		CheckReturnCode( LP_Glonass_SetFrequency( 0 ), "LP_Glonass_SetFrequency()" );

		CheckReturnCode( LP_Glonass_SetPower(-60), "LP_Glonass_SetPower()" );

		CheckReturnCode( LP_Glonass_SetRfOutput(IQV_RF_OUTPUT_OFF), "LP_Glonass_SetRfOutput()" );

		CheckReturnCode( LP_Glonass_SetRfOutput(IQV_RF_OUTPUT_ON), "LP_Glonass_SetRfOutput()" );

		CheckReturnCode( LP_Glonass_SetOperationMode(IQV_OPERATION_MODE_MODULATION_OFF), "LP_Glonass_SetOperationMode()" );

		CheckReturnCode( LP_Glonass_SetOperationMode(IQV_OPERATION_MODE_MODULATION_NAV_DATA_OFF), "LP_Glonass_SetOperationMode()" );

		CheckReturnCode( LP_Glonass_SetOperationMode(IQV_OPERATION_MODE_MODULATION), "LP_Glonass_SetOperationMode()" );

		CheckReturnCode( LP_Glonass_SetPower(-100), "LP_Glonass_SetPower()" );

		CheckReturnCode( LP_Glonass_SetPower(-130), "LP_Glonass_SetPower()" );

		CheckReturnCode( LP_PrintGlonassParameters(), "LP_PrintGlonassParameters()" );

		CheckReturnCode( LP_Glonass_SetDeactivate(), "LP_Glonass_SetDeactivate()" );
		//----------------------------//
		//   Disconnect the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Term(), "LP_Term()" );
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	while (FALSE == bExitFlag)
	{
		//tyu;07-04;timer logger


		//Get tester setup


		int len= GetPrivateProfileStringA("TESTER_SETUP","IP_ADDRESS",NULL,g_IP_addr,sizeof(g_IP_addr),".\\QA_Setup.ini");
		if ( len == 0)
		{
			//printf("[ERROR]  No \"Log_all.txt\"!");
			printf("[ERROR]  QA_setup information not right!");//Luke 2012-08-02
			printf("\nPress Any Key to exit...\n");
			_getch();
		}
		else
		{
			//do nothing
		}

		ciTesterType = GetPrivateProfileInt(_T("TESTER_SETUP"),_T("TESTER_TYPE"),0,_T(".\\QA_Setup.ini"));
		ciTesterControlMode = GetPrivateProfileInt(_T("TESTER_SETUP"),_T("TESTER_CONTROL_MODE"),0,_T(".\\QA_Setup.ini"));

		g_DH_TOKEN_ID= GetPrivateProfileInt(_T("TESTER_SETUP"),_T("DH_TOKEN_ID"),0,_T(".\\QA_Setup.ini"));
		g_DH_PORT= GetPrivateProfileInt(_T("TESTER_SETUP"),_T("DH_PORT"),0,_T(".\\QA_Setup.ini"));

		_mkdir( ".\\Log" );
		DeleteFile(_T(".\\Log\\Log_all.txt"));

		Sleep(1000);

		//Delete "Log_all.txt" if exsited

		if( -1 == g_logger_id )
		{
			::LOGGER_CreateLogger("IQmeasure_Test", &g_logger_id);  //log file name is "Log_all.txt" by default.
		}
		else
		{
		}

		::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"\n--------------------------Run %d-------------------\n\n",Tag_runTime);

		//end


		if( argc>1 )
		{
			if( 0==wcscmp(argv[1],_T("-wifi")) ){
				WiFi_Loopback();
			}else if( 0==wcscmp(argv[1],_T("-wifi-ag")) ){
				WiFi_11ag_Loopback();
			}else if( 0==wcscmp(argv[1],_T("-wifi-b")) ){
				WiFi_11b_Loopback();
			}else if( 0==wcscmp(argv[1],_T("-multi")) ){
				Multi_Packet_EVM_Analysis();
			}else if( 0==wcscmp(argv[1],_T("-bt")) ){
				Bluetooth_Loopback();
			}else if( 0==wcscmp(argv[1],_T("-fft")) ){
				Fft_Loopback();
			}else if( 0==wcscmp(argv[1],_T("-all")) ){
				printf("\n-------------------------\nStarting WiFi...\n\n");
				WiFi_Loopback();

				printf("\n-------------------------\nStarting Bluetooth...\n\n");
				Bluetooth_Loopback();

				printf("\n-------------------------\nStarting FFT...\n\n");
				Fft_Loopback();
			}else if( 0==wcscmp(argv[1],_T("-ac")) ){
				WiFi_11ac_Loopback();
			}else if( 0==wcscmp(argv[1],_T("-acmimo")) ){
				WiFi_11ac_MIMO_Loopback();
			}else if( 0==wcscmp(argv[1],_T("-evm")) ){
				Evm_Test();
			}else if( 0==wcscmp(argv[1],_T("-cw")) ){
				CW_Test();
			}
			else if( 0==wcscmp(argv[1],_T("-composite")) )
			{
				WiFi_11ac_Composite_Loopback();
			}
			//The following is for QA purpose
			else if( 0==wcscmp(argv[1],_T("-QA_11AC")))
			{
				QA_11AC();
			}
			else if( 0==wcscmp(argv[1],_T("-QA_11AC2")))
			{
				QA_11AC2();
			}
			else if( 0==wcscmp(argv[1],_T("-QA_11n")))
			{
				QA_11n();
			}
			else if( 0==wcscmp(argv[1],_T("-QA_11ag")))
			{
				QA_11ag();
			}
			else if( 0==wcscmp(argv[1],_T("-QA_11b")))
			{
				QA_11b();
			}
			else if( 0==wcscmp(argv[1],_T("-QA_CW")))
			{
				QA_CW();
			}
			else if( 0==wcscmp(argv[1],_T("-QA_FrameCount")) )
			{
				QA_FrameCount();
			}
			else if( 0==wcscmp(argv[1],_T("-QA_Misc")) )
			{
				QA_Misc();
			}
			else if( 0==wcscmp(argv[1],_T("-QA_Mask")) )
			{
				QA_Mask();
			}
			//tyu; 2012-07-11; QA BT
			else if( 0==wcscmp(argv[1],_T("-QA_BT")) )
			{
				QA_BT();
			}
			//tyu;2012-07-16; QA Dualhead
			else if ( 0==wcscmp(argv[1],_T("-QA_DH")) )
			{

				QA_Dualhead();

			}
			else if( 0 == wcscmp(argv[1], _T("-GPS")) ){


			}else if( 0 == wcscmp(argv[1], _T("-GLONASS")) ){
				Glonass_Test();
			}
			else
			{
				//do nothing
			}  //end
			//QA end

		}

		Tag_runTime +=1;
		//End

		set_color(CM_YELLOW);
		//	printf("\nPress Any Key to exit...\n");
		//	_getch();


		//tyu; 2012-7-6; Make it run continuously
		printf("\nPress any key to continue (x or Esc to exit)....\n");
		char ch;
		while (!_kbhit())
			Sleep(100);

		ch = toupper(_getch());
		//if (ch=='X' || ch==ASCII_ESC)
		//	bExitFlag = TRUE;
		if (ch=='X' || ch==ASCII_ESC){
			bExitFlag = TRUE;
			system("taskkill /f /im LitePointConnectivityServer.exe");
		}
	}

	return 0;
}


void ReadLogFiles (void)
{
	FILE *fp = NULL;
	char strLine[MAX_BUFFER_SIZE] = {'\0'};
	strcutResult sResult;

	//_mkdir(".\\Log");
	fopen_s(&fp,".\\Log\\Log_all.txt", "r");
	//fopen_s(&fp,".\\Log\\Log_all.txt", "w");//Luke 2012-08-02
	if (!fp)  //Log_all.txt is not exsited
	{
		printf("[ERROR]  No \"Log_all.txt\"!");
		printf("\nPress Any Key to exit...\n");
		_getch();

	}
	else   // Log_all.txt existed
	{
		//sResult.LP_Init.clear();
		while (fgets(strLine,MAX_BUFFER_SIZE,fp) != NULL)
		{

			if (strstr(strLine,"[IQMEASURE],"))  // Context we need
			{
				if( strstr(strLine, "[LP_Init]"))
				{
					sResult.LP_Init.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_InitTester]"))
				{
					sResult.LP_InitTester.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetVersion]"))
				{
					sResult.LP_GetVersion.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetVsg"))
				{
					sResult.LP_SetVsg.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetVsgModulation]"))
				{
					sResult.LP_SetVsgModulation.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_EnableVsgRF]"))
				{
					sResult.LP_EnableVsgRF.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetVsa]"))
				{
					sResult.LP_SetVsa.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_VsaDataCapture]"))
				{
					sResult.LP_VsaDataCapture.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SaveVsaSignalFile]"))
				{
					sResult.LP_SaveVsaSignalFile.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnayzeFFT]"))
				{
					sResult.LP_AnalyzeFFT.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzePower]"))
				{
					sResult.LP_AnalyzePower.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_Analyze80211ac]"))
				{
					sResult.LP_Analyze80211ac.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_Term]"))
				{
					sResult.LP_Term.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetIQapiHndl]"))
				{
					sResult.LP_GetIQapiHndl.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_InitTester2]"))
				{
					sResult.LP_InitTester2.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_InitTester3]"))
				{
					sResult.LP_InitTester3.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_InitTester4]"))
				{
					sResult.LP_InitTester4.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_ConClose]"))
				{
					sResult.LP_ConClose.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_ConOpen]"))
				{
					sResult.LP_ConOpen.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetErrorString]"))
				{
					sResult.LP_GetErrorString.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetIQapiHndlLastErrMsg]"))
				{
					sResult.LP_GetIQapiHndlLastErrMsg.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_DualHead_ConOpen]"))
				{
					sResult.LP_DualHead_ConOpen.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_DualHead_GetTokenID]"))
				{
					sResult.LP_DualHead_GetTokenID.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_DualHead_ConClose]"))
				{
					sResult.LP_DualHead_ConClose.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_DualHead_ObtainControl]"))
				{
					sResult.LP_DualHead_ObtainControl.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_DualHead_ReleaseControl]"))
				{
					sResult.LP_DualHead_ReleaseControl.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_SetFrequency]"))
				{
					sResult.LP_FM_SetFrequency.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_SetCarrierPower]"))
				{
					sResult.LP_FM_SetCarrierPower.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetVsg_GapPower]"))
				{
					sResult.LP_SetVsg_GapPower.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetVsgCw]"))
				{
					sResult.LP_SetVsgCw.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SaveSignalArrays]"))
				{
					sResult.LP_SaveSignalArrays.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SaveIQDataToModulationFile]"))
				{
					sResult.LP_SaveIQDataToModulationFile.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetAnalysisParameterInteger]"))
				{
					sResult.LP_SetAnalysisParameterInteger.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetAnalysisParameterIntegerArray]"))
				{
					sResult.LP_SetAnalysisParameterIntegerArray.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_CopyVsaCaptureData]"))
				{
					sResult.LP_CopyVsaCaptureData.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_LoadVsaSignalFile]"))
				{
					sResult.LP_LoadVsaSignalFile.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SaveVsaGeneratorFile]"))
				{
					sResult.LP_SaveVsaGeneratorFile.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetVsaBluetooth]"))
				{
					sResult.LP_SetVsaBluetooth.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetVsaTriggerTimeout]"))
				{
					sResult.LP_SetVsaTriggerTimeout.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetVsaAmplitudeTolerance]"))
				{
					sResult.LP_SetVsaAmplitudeTolerance.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_Agc]"))
				{
					sResult.LP_Agc.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetFrameCnt]"))
				{
					sResult.LP_SetFrameCnt.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_TxDone]"))
				{
					sResult.LP_TxDone.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_EnableSpecifiedVsgRF]"))
				{
					sResult.LP_EnableSpecifiedVsgRF.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_EnableSpecifiedVsaRF]"))
				{
					sResult.LP_EnableSpecifiedVsaRF.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetVsaSettings]"))
				{
					sResult.LP_GetVsaSettings.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetSampleData]"))
				{
					sResult.LP_GetSampleData.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SelectCaptureRangeForAnalysis]"))
				{
					sResult.LP_SelectCaptureRangeForAnalysis.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_Analyze80216d]"))
				{
					sResult.LP_Analyze80216d.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_Analyze80216e]"))
				{
					sResult.LP_Analyze80216e.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_Analyze80211p]"))
				{
					sResult.LP_Analyze80211p.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_Analyze80211ag]"))
				{
					sResult.LP_Analyze80211ag.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_Analyze80211n]"))
				{
					sResult.LP_Analyze80211n.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzeMimo]"))
				{
					sResult.LP_AnalyzeMimo.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_Analyze80211b]"))
				{
					sResult.LP_Analyze80211b.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzeCCDF]"))
				{
					sResult.LP_AnalyzeCCDF.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzeCW]"))
				{
					sResult.LP_AnalyzeCW.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzeCWFreq]"))
				{
					sResult.LP_AnalyzeCWFreq.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalysisWave]"))
				{
					sResult.LP_AnalysisWave.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzeSidelobe]"))
				{
					sResult.LP_AnalyzeSidelobe.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzePowerRampOFDM]"))
				{
					sResult.LP_AnalyzePowerRampOFDM.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzePowerRamp80211b]"))
				{
					sResult.LP_AnalyzePowerRamp80211b.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzeBluetooth]"))
				{
					sResult.LP_AnalyzeBluetooth.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzeZigbee]"))
				{
					sResult.LP_AnalyzeZigbeeTerm.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzeHT40Mask]"))
				{
					sResult.LP_AnalyzeHT40Mask.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetScalarMeasurment]"))
				{
					sResult.LP_GetScalarMeasurment.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetVectorMeasurment]"))
				{
					sResult.LP_GetVectorMeasurment.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetStringMeasurment]"))
				{
					sResult.LP_GetStringMeasurment.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetScalarMeasurement]"))
				{
					sResult.LP_GetScalarMeasurement.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetVectorMeasurement]"))
				{
					sResult.LP_GetVectorMeasurement.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_GetStringMeasurement]"))
				{
					sResult.LP_GetStringMeasurement.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_PlotDataCapture]"))
				{
					sResult.LP_PlotDataCapture.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_Plot]"))
				{
					sResult.LP_Plot.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_StartIQmeasureTimer]"))
				{
					sResult.LP_StartIQmeasureTimer.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_StopIQmeasureTimer]"))
				{
					sResult.LP_StopIQmeasureTimer.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_ReportTimerDurations]"))
				{
					sResult.LP_ReportTimerDurations.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[SetTesterHwVersion]"))
				{
					sResult.SetTesterHwVersion.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[GetTesterHwVersion]"))
				{
					sResult.GetTesterHwVersion.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetLpcPath]"))
				{
					sResult.LP_SetLpcPath.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_SetVsg]"))
				{
					sResult.LP_FM_SetVsg.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_GetVsgSettings]"))
				{
					sResult.LP_FM_GetVsgSettings.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_SetAudioSingleTone]"))
				{
					sResult.LP_FM_SetAudioSingleTone.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_SetAudioToneArray]"))
				{
					sResult.LP_FM_SetAudioToneArray.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_SetAudioToneArrayDeviation]"))
				{
					sResult.LP_FM_SetAudioToneArrayDeviation.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_GetAudioToneArray]"))
				{
					sResult.LP_FM_GetAudioToneArray.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_DeleteAudioTones]"))
				{
					sResult.LP_FM_DeleteAudioTones.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_StartVsg]"))
				{
					sResult.LP_FM_StartVsg.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_StopVsg]"))
				{
					sResult.LP_FM_StopVsg.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_SetVsgDistortion]"))
				{
					sResult.LP_FM_SetVsgDistortion.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_GetVsgDistortion]"))
				{
					sResult.LP_FM_GetVsgDistortion.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_StartVsgInterference]"))
				{
					sResult.LP_FM_StartVsgInterference.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_GetVsgInterferenceSettings]"))
				{
					sResult.LP_FM_GetVsgInterferenceSettings.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_StopVsgInterference]"))
				{
					sResult.LP_FM_StopVsgInterference.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_SetVsa]"))
				{
					sResult.LP_FM_SetVsa.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_GetVsaSettings]"))
				{
					sResult.LP_FM_GetVsaSettings.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_VsaDataCapture]"))
				{
					sResult.LP_FM_VsaDataCapture.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_Analyze_RF]"))
				{
					sResult.LP_FM_Analyze_RF.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_Analyze_Demod]"))
				{
					sResult.LP_FM_Analyze_Demod.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_Analyze_Audio_Mono]"))
				{
					sResult.LP_FM_Analyze_Audio_Mono.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_Analyze_Audio_Stereo]"))
				{
					sResult.LP_FM_Analyze_Audio_Stereo.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_GetScalarMeasurement]"))
				{
					sResult.LP_FM_GetScalarMeasurement.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_GetVectorMeasurement]"))
				{
					sResult.LP_FM_GetVectorMeasurement.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_AudioDataCapture]"))
				{
					sResult.LP_FM_AudioDataCapture.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_LoadAudioCapture]"))
				{
					sResult.LP_FM_LoadAudioCapture.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_AudioStimulusGenerateAndPlayMultiTone]"))
				{
					sResult.LP_FM_AudioStimulusGenerateAndPlayMultiTone.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_AudioStimulusGenerateAndPlaySingleTone]"))
				{
					sResult.LP_FM_AudioStimulusGenerateAndPlaySingleTone.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_StopAudioPlay]"))
				{
					sResult.LP_FM_StopAudioPlay.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SaveVsaSignalFileText]"))
				{
					sResult.LP_SaveVsaSignalFileText.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SaveTruncateCapture]"))
				{
					sResult.LP_SaveTruncateCapture.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_PwrCalFastGetElemen]"))
				{
					sResult.LP_PwrCalFastGetElemen.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FM_SetAudioToneModeAmplitude]"))
				{
					sResult.LP_FM_SetAudioToneModeAmplitude.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_SetVsg_triggerType]"))
				{
					sResult.LP_SetVsg_triggerType.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_AnalyzeVHT80Mask]"))
				{
					sResult.LP_AnalyzeVHT80Mask.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FastCalGetPowerData]"))
				{
					sResult.LP_FastCalGetPowerData.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_PwrCalFreeElement]"))
				{
					sResult.LP_PwrCalFreeElement.push_back(strLine);
					continue;
				}
				else if (strstr(strLine, "[LP_FastCalMeasPower]"))
				{
					sResult.LP_FastCalMeasPower.push_back(strLine);
					continue;
				}
				else
					sResult.LP_others.push_back(strLine);
			}
			else  //Context we don't need
			{
				continue;
			}
		}
	}

	fclose(fp);

	//Write to csv file
	fopen_s(&fp,g_csvFileName,"w+");
	if ( !fp)
	{
		printf("[ERROR]  Open csv file failed!");
		printf("\nPress Any Key to exit...\n");
		_getch();
	}


	for (unsigned int i=0; i<sResult.GetTesterHwVersion.size();i++)
		fprintf(fp,"%s",sResult.GetTesterHwVersion.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Agc.size();i++)
		fprintf(fp,"%s",sResult.LP_Agc.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalysisWave.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalysisWave.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Analyze80211ac.size();i++)
		fprintf(fp,"%s",sResult.LP_Analyze80211ac.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Analyze80211ag.size();i++)
		fprintf(fp,"%s",sResult.LP_Analyze80211ag.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Analyze80211b.size();i++)
		fprintf(fp,"%s",sResult.LP_Analyze80211b.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Analyze80211n.size();i++)
		fprintf(fp,"%s",sResult.LP_Analyze80211n.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Analyze80211p.size();i++)
		fprintf(fp,"%s",sResult.GetTesterHwVersion.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Analyze80211p.size();i++)
		fprintf(fp,"%s",sResult.LP_Analyze80216d.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Analyze80216d.size();i++)
		fprintf(fp,"%s",sResult.LP_Analyze80216e.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Analyze80216e.size();i++)
		fprintf(fp,"%s",sResult.GetTesterHwVersion.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzeBluetooth.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzeBluetooth.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzeCCDF.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzeCCDF.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzeCW.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzeCW.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzeCWFreq.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzeCWFreq.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzeFFT.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzeFFT.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzeHT40Mask.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzeHT40Mask.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzeMimo.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzeMimo.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzePower.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzePower.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzePowerRamp80211b.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzePowerRamp80211b.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzePowerRampOFDM.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzePowerRampOFDM.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzeSidelobe.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzeSidelobe.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzeVHT80Mask.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzeVHT80Mask.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzeZigbee.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzeZigbee.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_ConClose.size();i++)
		fprintf(fp,"%s",sResult.LP_ConClose.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_ConOpen.size();i++)
		fprintf(fp,"%s",sResult.LP_ConOpen.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_CopyVsaCaptureData.size();i++)
		fprintf(fp,"%s",sResult.LP_CopyVsaCaptureData.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_DualHead_ConClose.size();i++)
		fprintf(fp,"%s",sResult.LP_DualHead_ConClose.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_DualHead_ConOpen.size();i++)
		fprintf(fp,"%s",sResult.LP_DualHead_ConOpen.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_DualHead_GetTokenID.size();i++)
		fprintf(fp,"%s",sResult.LP_DualHead_GetTokenID.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_DualHead_ObtainControl.size();i++)
		fprintf(fp,"%s",sResult.LP_DualHead_ObtainControl.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_DualHead_ReleaseControl.size();i++)
		fprintf(fp,"%s",sResult.LP_DualHead_ReleaseControl.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_EnableSpecifiedVsaRF.size();i++)
		fprintf(fp,"%s",sResult.LP_EnableSpecifiedVsaRF.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_EnableSpecifiedVsgRF.size();i++)
		fprintf(fp,"%s",sResult.LP_EnableSpecifiedVsgRF.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_EnableVsgRF.size();i++)
		fprintf(fp,"%s",sResult.LP_EnableVsgRF.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FastCalGetPowerData.size();i++)
		fprintf(fp,"%s",sResult.LP_FastCalGetPowerData.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FastCalMeasPower.size();i++)
		fprintf(fp,"%s",sResult.LP_FastCalMeasPower.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_Analyze_Audio_Mono.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_Analyze_Audio_Mono.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_Analyze_Audio_Stereo.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_Analyze_Audio_Stereo.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_Analyze_Demod.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_Analyze_Demod.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_Analyze_RF.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_Analyze_RF.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_AudioDataCapture.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_AudioDataCapture.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_AudioStimulusGenerateAndPlayMultiTone.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_AudioStimulusGenerateAndPlayMultiTone.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_AudioStimulusGenerateAndPlaySingleTone.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_AudioStimulusGenerateAndPlaySingleTone.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_DeleteAudioTones.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_DeleteAudioTones.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_GetAudioToneArray.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_GetAudioToneArray.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_GetScalarMeasurement.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_GetScalarMeasurement.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_GetVectorMeasurement.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_GetVectorMeasurement.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_GetVsaSettings.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_GetVsaSettings.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_GetVsgDistortion.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_GetVsgDistortion.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_GetVsgInterferenceSettings.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_GetVsgInterferenceSettings.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_GetVsgSettings.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_GetVsgSettings.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_LoadAudioCapture.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_LoadAudioCapture.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_SetAudioSingleTone.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_SetAudioSingleTone.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_SetAudioToneArray.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_SetAudioToneArray.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_SetAudioToneArrayDeviation.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_SetAudioToneArrayDeviation.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_SetAudioToneArrayDeviation.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_SetAudioToneArrayDeviation.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_SetAudioToneModeAmplitude.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_SetAudioToneModeAmplitude.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_SetCarrierPower.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_SetCarrierPower.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_SetFrequency.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_SetFrequency.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_SetVsa.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_SetVsa.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_SetVsg.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_SetVsg.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_SetVsgDistortion.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_SetVsgDistortion.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_StartVsg.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_StartVsg.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_StartVsgInterference.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_StartVsgInterference.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_StopAudioPlay.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_StopAudioPlay.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_StopVsgInterference.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_StopVsgInterference.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_StopVsg.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_StopVsg.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_FM_VsaDataCapture.size();i++)
		fprintf(fp,"%s",sResult.LP_FM_VsaDataCapture.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetErrorString.size();i++)
		fprintf(fp,"%s",sResult.LP_GetErrorString.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetIQapiHndl.size();i++)
		fprintf(fp,"%s",sResult.LP_GetIQapiHndl.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetIQapiHndlLastErrMsg.size();i++)
		fprintf(fp,"%s",sResult.LP_GetIQapiHndlLastErrMsg.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetSampleData.size();i++)
		fprintf(fp,"%s",sResult.LP_GetSampleData.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetScalarMeasurement.size();i++)
		fprintf(fp,"%s",sResult.LP_GetScalarMeasurement.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetScalarMeasurment.size();i++)
		fprintf(fp,"%s",sResult.LP_GetScalarMeasurment.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetStringMeasurement.size();i++)
		fprintf(fp,"%s",sResult.LP_GetStringMeasurement.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetStringMeasurment.size();i++)
		fprintf(fp,"%s",sResult.LP_GetStringMeasurment.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetTesterHwVersion.size();i++)
		fprintf(fp,"%s",sResult.LP_GetTesterHwVersion.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetVectorMeasurement.size();i++)
		fprintf(fp,"%s",sResult.LP_GetVectorMeasurement.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetVectorMeasurment.size();i++)
		fprintf(fp,"%s",sResult.LP_GetVectorMeasurment.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetVersion.size();i++)
		fprintf(fp,"%s",sResult.LP_GetVersion.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_GetVsaSettings.size();i++)
		fprintf(fp,"%s",sResult.LP_GetVsaSettings.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Init.size();i++)
		fprintf(fp,"%s",sResult.LP_Init.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_InitTester.size();i++)
		fprintf(fp,"%s",sResult.LP_InitTester.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_InitTester2.size();i++)
		fprintf(fp,"%s",sResult.LP_InitTester2.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_InitTester3.size();i++)
		fprintf(fp,"%s",sResult.LP_InitTester3.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_InitTester4.size();i++)
		fprintf(fp,"%s",sResult.LP_InitTester4.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_LoadVsaSignalFile.size();i++)
		fprintf(fp,"%s",sResult.LP_LoadVsaSignalFile.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_AnalyzeZigbeeTerm.size();i++)
		fprintf(fp,"%s",sResult.LP_AnalyzeZigbeeTerm.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_others.size();i++)
		fprintf(fp,"%s",sResult.LP_others.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Plot.size();i++)
		fprintf(fp,"%s",sResult.LP_Plot.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_PlotDataCapture.size();i++)
		fprintf(fp,"%s",sResult.LP_PlotDataCapture.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_PwrCalFastGetElemen.size();i++)
		fprintf(fp,"%s",sResult.LP_PwrCalFastGetElemen.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_PwrCalFastGetElement.size();i++)
		fprintf(fp,"%s",sResult.LP_PwrCalFastGetElement.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_PwrCalFreeElement.size();i++)
		fprintf(fp,"%s",sResult.LP_PwrCalFreeElement.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_ReportTimerDurations.size();i++)
		fprintf(fp,"%s",sResult.LP_ReportTimerDurations.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SaveIQDataToModulationFile.size();i++)
		fprintf(fp,"%s",sResult.LP_SaveIQDataToModulationFile.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SaveSignalArrays.size();i++)
		fprintf(fp,"%s",sResult.LP_SaveSignalArrays.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SaveTruncateCapture.size();i++)
		fprintf(fp,"%s",sResult.LP_SaveTruncateCapture.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SaveVsaGeneratorFile.size();i++)
		fprintf(fp,"%s",sResult.LP_SaveVsaGeneratorFile.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SaveVsaSignalFile.size();i++)
		fprintf(fp,"%s",sResult.LP_SaveVsaSignalFile.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SaveVsaSignalFileText.size();i++)
		fprintf(fp,"%s",sResult.LP_SaveVsaSignalFileText.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SelectCaptureRangeForAnalysis.size();i++)
		fprintf(fp,"%s",sResult.LP_SelectCaptureRangeForAnalysis.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetAnalysisParameterInteger.size();i++)
		fprintf(fp,"%s",sResult.LP_SetAnalysisParameterInteger.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetAnalysisParameterIntegerArray.size();i++)
		fprintf(fp,"%s",sResult.LP_SetAnalysisParameterIntegerArray.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetFrameCnt.size();i++)
		fprintf(fp,"%s",sResult.LP_SetFrameCnt.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetLpcPath.size();i++)
		fprintf(fp,"%s",sResult.LP_SetLpcPath.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetTesterHwVersion.size();i++)
		fprintf(fp,"%s",sResult.LP_SetTesterHwVersion.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetVsa.size();i++)
		fprintf(fp,"%s",sResult.LP_SetVsa.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetVsaAmplitudeTolerance.size();i++)
		fprintf(fp,"%s",sResult.LP_SetVsaAmplitudeTolerance.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetVsaBluetooth.size();i++)
		fprintf(fp,"%s",sResult.LP_SetVsaBluetooth.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetVsaTriggerTimeout.size();i++)
		fprintf(fp,"%s",sResult.LP_SetVsaTriggerTimeout.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetVsg.size();i++)
		fprintf(fp,"%s",sResult.LP_SetVsg.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetVsg_GapPower.size();i++)
		fprintf(fp,"%s",sResult.LP_SetVsg_GapPower.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetVsg_triggerType.size();i++)
		fprintf(fp,"%s",sResult.LP_SetVsg_triggerType.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetVsgCw.size();i++)
		fprintf(fp,"%s",sResult.LP_SetVsgCw.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_SetVsgModulation.size();i++)
		fprintf(fp,"%s",sResult.LP_SetVsgModulation.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_StartIQmeasureTimer.size();i++)
		fprintf(fp,"%s",sResult.LP_StartIQmeasureTimer.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_StopIQmeasureTimer.size();i++)
		fprintf(fp,"%s",sResult.LP_StopIQmeasureTimer.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_Term.size();i++)
		fprintf(fp,"%s",sResult.LP_Term.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_TxDone.size();i++)
		fprintf(fp,"%s",sResult.LP_TxDone.at(i).c_str());
	for (unsigned int i=0; i<sResult.LP_VsaDataCapture.size();i++)
		fprintf(fp,"%s",sResult.LP_VsaDataCapture.at(i).c_str());
	for (unsigned int i=0; i<sResult.SetTesterHwVersion.size();i++)
		fprintf(fp,"%s",sResult.SetTesterHwVersion.at(i).c_str());

	fclose(fp);
}

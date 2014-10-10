//Wrapper


#include "stdafx.h"
#include "stdio.h"
#include "IQmeasure.h"
#include <stdio.h>
#include <string.h>
#include "Version.h"
#include "IQlite_Timer.h"
#include "IQlite_Logger.h"

#define MAX_BUFFER_SIZE 4096
#pragma comment(lib, "IQlite_Logger.lib")
#include <float.h> // DBL_MAX on Mac
#include <direct.h>
#include <tchar.h>
#include <atlbase.h>
#include <atlconv.h>

using namespace std;

//---------------------------------------------------------------
// Global variables
//---------------------------------------------------------------
static HMODULE DynamiclibraryHandle = NULL;
char dynamiclibraryName[512] = {};
int  g_IQtype= IQTYPE_2010;

char g_cTesterName[512] = {'\0'};

//---------------------------------------------------------------
// Macro definition
//---------------------------------------------------------------
#define LOAD_DLLPTR(a) \
	if (!(a##_Ptr = (a##Type) GetProcAddress((HMODULE)DynamiclibraryHandle, #a))) \
{\
	::LOGGER_Write_Ext(LOG_IQMEASURE, *LP_loggerIQmeasure_Ptr, LOGGER_WARNING, #a " is not found in the Dynamic library\n");\
	\
}\
else \
::LOGGER_Write_Ext(LOG_IQMEASURE, *LP_loggerIQmeasure_Ptr, LOGGER_INFORMATION, #a "() found in Dynamic library\n");\

//---------------------------------------------------------------
// Define Var prototype
//---------------------------------------------------------------
typedef int		*LP_loggerIQmeasureType;
typedef int		*LP_timerIQmeasureType;

//---------------------------------------------------------------
// Define function prototype
//---------------------------------------------------------------
typedef int		(*LP_InitType)(int testerControlMethod,int IQtype);
typedef int		(*LP_TermType)(void);
typedef int		(*LP_GetIQapiHndlType)(void **getHndl, bool *testerInitialized);
typedef bool	(*LP_GetVersionType)(char *buffer, int buf_size);
typedef int		(*LP_InitTesterType)(char *ipAddress);
typedef int		(*LP_InitTester2Type)(char *ipAddress1, char *ipAddress2);
typedef int		(*LP_InitTester3Type)(char *ipAddress1, char *ipAddress2, char *ipAddress3);
typedef int		(*LP_InitTester4Type)(char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4);
typedef int		(*LP_ConCloseType)(void);
typedef int		(*LP_ConOpenType)(void);
typedef char*	(*LP_GetErrorStringType)(int err);
typedef char*	(*LP_GetIQapiHndlLastErrMsgType)(void);

typedef int		(*LP_DualHead_ConOpenType)(int tokenID, char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4);
typedef int		(*LP_DualHead_GetTokenIDType)(int *tokenID);
typedef int		(*LP_DualHead_ConCloseType)();
typedef int		(*LP_DualHead_ObtainControlType)(unsigned int probeTimeMS, unsigned int timeOutMS);
typedef int		(*LP_DualHead_ReleaseControlType)();
typedef int		(*LP_SetAlcModeType)(IQAPI_ALC_MODES alcMode);
typedef int		(*LP_GetAlcModeType)(IQAPI_ALC_MODES* alcMode);
typedef int		(*LP_SetVsgType)(double rfFreqHz, double rfPowerLeveldBm, int port, bool setGapPowerOff, double dFreqShiftHz );
typedef int		(*LP_SetVsg_CompensationType)(double		  dcErrI,
		double		  dcErrQ,
		double		  phaseErr,
		double		  gainErr,
		double		  delayErr);
typedef int		(*LP_SetVsgTriggerTypeType)(int triggerType);
typedef int		(*LP_SetVsg_GapPowerType)(double rfFreqHz, double rfPowerLeveldBm, int port, int gapPowerOff);
typedef int		(*LP_SetVsgNxNType)(double rfFreqHz, double rfPowerLeveldBm[], int port[], double dFreqShiftHz);
typedef int		(*LP_SetVsgCwType)(double sineFreqHz, double offsetFrequencyMHz, double rfPowerLeveldBm, int port);
typedef int		(*LP_SaveSignalArraysType)(double *real[N_MAX_TESTERS],
		double *imag[N_MAX_TESTERS],
		int length[N_MAX_TESTERS],
		double sampleFreqHz[N_MAX_TESTERS],
		char *fileNameToSave);
typedef int		(*LP_SaveIQDataToModulationFileType)(double *real,
		double *imag,
		int length[N_MAX_TESTERS],
		char *modFileName,
		int  normalization,
		int loadIQDataToVsg);
typedef int		(*LP_SetVsgModulationType)(char *modFileName, int loadInternalWaveform);
typedef int		(*LP_SetAnalysisParameterIntegerType)(char *measurement, char *parameter, int value);
typedef int		(*LP_SetAnalysisParameterIntegerArrayType)(char *measurement, char *parameter, int *value, int valuesize);
typedef int		(*LP_CopyVsaCaptureDataType)(int fromVsaNum, int toVsaNum);
typedef int		(*LP_SaveVsaSignalFileType)(char *sigFileName);
typedef int		(*LP_LoadVsaSignalFileType)(char *sigFileName);
typedef int		(*LP_SaveVsaGeneratorFileType)(char *modFileName);
typedef int		(*LP_SetVsaBluetoothType)(double rfFreqHz, double rfAmplDb, int port, double triggerLevelDb, double triggerPreTime);
typedef int		(*LP_SetVsaType)(double rfFreqHz, double rfAmplDb, int port, double extAttenDb, double triggerLevelDb, double triggerPreTime, double dFreqShiftHz);
typedef int		(*LP_SetVsaTriggerTimeoutType)(double triggerTimeoutSec);
typedef int		(*LP_SetVsaNxNType)(double rfFreqHz, double rfAmplDb[], int port[], double extAttenDb, double triggerLevelDb, double triggerPreTime, double dFreqShiftHz);
typedef int		(*LP_SetVsaAmplitudeToleranceType)(double amplitudeToleranceDb);
typedef int		(*LP_AgcType)(double *rfAmplDb,bool allTesters);
typedef int		(*LP_SetFrameCntType)(int frameCnt);
typedef int		(*LP_TxDoneType)(void);
typedef int		(*LP_EnableVsgRFType)(int enabled);
typedef int		(*LP_EnableVsgRFNxNType)(int vsg1Enabled, int vsg2Enabled, int vsg3Enabled, int vsg4Enabled);
typedef int		(*LP_EnableSpecifiedVsgRFType)(int enabled, int vsgNumber);
typedef int		(*LP_EnableSpecifiedVsaRFType)(int enabled, int vsaNumber);
typedef int		(*LP_GetVsaSettingsType)(double *freqHz, double *ampl, IQAPI_PORT_ENUM *port, int *rfEnabled, double *triggerLevel);
typedef int		(*LP_VsaDataCaptureType)(double samplingTimeSecs, int triggerType, double sampleFreqHz, int ht40Mode,IQMEASURE_CAPTURE_NONBLOCKING_STATES nonBlockingState);
typedef int		(*LP_GetSampleDataType)(int vsaNum, double bufferReal[], double bufferImag[], int bufferLength);
typedef int		(*LP_GetHndlDataPointersType)(double *real[],double *imag[],int *length, double *sampleFreqHz, int arraySize);
typedef int		(*LP_SaveUserDataToSigFileType)(char* sigFileName,
		double *real[],
		double *imag[],
		int *length,
		double *sampleFreqHz,
		int arraySize);
typedef int		(*LP_SelectCaptureRangeForAnalysisType)(double startPositionUs, double lengthUs, int packetsOffset, int packetsLength);
typedef int		(*LP_Analyze80216dType)( double sigType	,
		double bandwidthHz	,
		double cyclicPrefix	,
		double rateId		,
		double numSymbols	,
		int ph_corr			,
		int ch_corr			,
		int freq_corr		,
		int timing_corr		,
		int ampl_track		,
		double decode
		);
typedef int		(*LP_Analyze80216eType)( double sigType	,
		double bandwidthHz	,
		double cyclicPrefix	,
		double rateId		,
		double numSymbols	,
		int ph_corr			,
		int ch_corr			,
		int freq_corr		,
		int timing_corr		,
		int ampl_track		,
		double decode	,
		char *map_conf_file
		);
typedef int		(*LP_Analyze80211pType)(int ph_corr_mode, int ch_estimate, int sym_tim_corr, int freq_sync, int ampl_track, int ofdm_mode);
typedef int		(*LP_Analyze80211agType)(int ph_corr_mode, int ch_estimate, int sym_tim_corr, int freq_sync, int ampl_track, double prePowStartSec,
		double prePowStopSec);
typedef int		(*LP_Analyze80211nType)(char *type,
		char *mode,
		int enablePhaseCorr ,
		int enableSymTimingCorr ,
		int enableAmplitudeTracking ,
		int decodePSDU ,
		int enableFullPacketChannelEst ,
		char *referenceFile,
		int packetFormat,
		int frequencyCorr,
		double prePowStartSec,
		double prePowStopSec);
typedef int		(*LP_AnalyzeMimoType)(char *type, char *mode, int enablePhaseCorr, int enableSymTimingCorr, int enableAmplitudeTracking, int decodePSDU, int enableFullPacketChannelEst, char *referenceFile);
typedef int		(*LP_Analyze80211bType)(int eq_taps,
		int DCremove11b_flag ,
		int method_11b,
		double prePowStartSec,
		double prePowStopSec);
typedef int		(*LP_AnalyzePowerType)(double T_interval, double max_pow_diff_dB);
typedef int		(*LP_AnalyzeFFTType)(double NFFT, double res_bw, char *window_type);
typedef int		(*LP_AnalyzeCCDFType)();
typedef int		(*LP_AnalyzeCWType)();
typedef int		(*LP_AnalyzeCWFreqType)();
typedef int		(*LP_AnalysisWaveType)();
typedef int		(*LP_AnalyzeSidelobeType)();
typedef int		(*LP_AnalyzePowerRampOFDMType)();
typedef int		(*LP_AnalyzePowerRamp80211bType)();
typedef int		(*LP_AnalyzeBluetoothType)(double data_rate, char *analysis_type );
typedef int		(*LP_AnalyzeZigbeeType)();
typedef int		(*LP_AnalyzeHT40MaskType)();
typedef double	(*LP_GetScalarMeasurmentType)(char *measurement, int index);
typedef int		(*LP_GetVectorMeasurmentType)(char *measurement, double bufferReal[], double bufferImag[], int bufferLength);
typedef int		(*LP_GetStringMeasurmentType)(char *measurement, char bufferChar[], int bufferLength);
typedef double	(*LP_GetScalarMeasurementType)(char *measurement, int index);
typedef int		(*LP_GetVectorMeasurementType)(char *measurement, double bufferReal[], double bufferImag[], int bufferLength);
typedef int		(*LP_GetStringMeasurementType)(char *measurement, char bufferChar[], int bufferLength);
typedef int		(*LP_PlotDataCaptureType)();
typedef int		(*LP_PlotType)(int figNum, double *x, double *y, int length, char *plotArgs, char *title, char *xtitle, char *ytitle, int keepPlot);
typedef int		(*LP_StartIQmeasureTimerType)();
typedef int		(*LP_StopIQmeasureTimerType)();
typedef int		(*LP_ReportTimerDurationsType)();

// Add LP_SetDefault(); Tracy Yu; 2012-03-31
typedef int     (*LP_SetDefaultType)(void);
typedef int     (*LP_FM_SetFrequencyType)(unsigned int carrierFreqHz, double carrierPowerdBm );
// Add LP_FM_SetCarrierPower(); Buddy; 2012-04-6
typedef int     (*LP_FM_SetCarrierPowerType)(double carrierPowerdBm );
typedef int     (*LP_ScpiCommandSetType)(char *scpiCommand);;
typedef int     (*LP_ScpiCommandQueryType)(char query[], unsigned int max_size, char response[], unsigned int *actual_size);

//For MTK META
typedef int     (*LP_GetResultSpectralMaskType)(int bandwidth_index, double *ViolationPercent, int *length, double *freqs_MHz, double *trace_dBm, double *mask);
typedef int     (*LP_GetResultSpectralMask_HT40Type)(double *ViolationPercent, int *length, double *freqs_MHz, double *trace_dBm, double *mask);
typedef int     (*LP_LoResultSpectralFlatness11AGType)(int* carrierNo, double* margin, double* loLeakage, bool* flatnessPass, int* sub_carrier, double* data_per_carrier, double* average_center_power);
typedef int     (*LP_LoResultSpectralFlatness11NType)(int wifiMode, int* carrierNo, double* margin, double* loLeakage, bool* flatnessPass, int* sub_carrier, double* data_per_carrier, double* average_center_power, int streamIndex);  // streamIndex = 0, 1, 2, 3 (zero based)

/*-------------------------*
 *	IQ2010 Ext Functions   *
 *--------------------------*/
typedef int		(*LP_IQ2010EXT_NewMultiAnalysisCaptureType)( char		   *dataRate,
		double          freqMHz,
		double	        rmsPowerLeveldBm,
		int	            skipPktCnt,
		int		        rfPort,
		double	        triggerLeveldBm);
typedef int		(*LP_IQ2010EXT_AddMultiAnalysisCaptureType)( int		    analysisType,
		int			captureLengthUs,
		int			captureCnt);
typedef int		(*LP_IQ2010EXT_InitiateMultiAnalysisCaptureType)(void);
typedef int		(*LP_IQ2010EXT_FinishMultiAnalysisCaptureType)(void);
typedef int		(*LP_IQ2010EXT_InitiateMultiCaptureType)( char				*dataRate,
		double              freqMHz,
		double	             rmsPowerLeveldBm,
		int	             captureLengthUs,
		int	             skipPktCnt,
		int	             captureCnt,
		int				 rfPort,
		double	             triggerLeveldBm);
typedef int		(*LP_IQ2010EXT_InitiateMultiCaptureHT40Type)( char			   *dataRate,
		double             freqMHz,
		double	            rmsPowerLeveldBm,
		int	            captureLengthUs,
		int	            skipPktCnt,
		int	            captureCnt,
		int                rfPort,
		double	            triggerLeveldBm);

typedef int		(*LP_IQ2010EXT_FinishMultiCaptureType)(void);
typedef int		(*LP_IQ2010EXT_Analyze80211agType)( IQV_PH_CORR_ENUM ph_corr_mode ,
		IQV_CH_EST_ENUM ch_estimate,
		IQV_SYM_TIM_ENUM sym_tim_corr,
		IQV_FREQ_SYNC_ENUM freq_sync,
		IQV_AMPL_TRACK_ENUM ampl_track);
typedef int		(*LP_IQ2010EXT_Analyze80211nSISOType)( int enablePhaseCorr ,
		int enableSymTimingCorr,
		int enableAmplitudeTracking ,
		int decodePSDU,
		int enableFullPacketChannelEst,
		int packetFormat);
typedef int		(*LP_IQ2010EXT_Analyze80211nCompositeType)(int enablePhaseCorr,
		int enableSymTimingCorr,
		int enableAmplitudeTracking,
		int enableFullPacketChannelEst,
		char *referenceFileL,
		int packetFormat );
typedef int		(*LP_IQ2010EXT_Analyze80211bType)( IQV_EQ_ENUM eq_taps,
		IQV_DC_REMOVAL_ENUM DCremove11b_flag ,
		IQV_11B_METHOD_ENUM method_11b);
typedef int		(*LP_IQ2010EXT_AnalyzePowerType)(void);
typedef int		(*LP_IQ2010EXT_AnalyzeFFTType)( double NFFT ,
		double res_bwHz,
		char *window_type);

typedef int		(*LP_IQ2010EXT_AnalyzeMaskMeasurementType)(int maskType, double obwPercentage);
typedef int		(*LP_IQ2010EXT_AnalyzePowerRamp80211bType)();
typedef int		(*LP_IQ2010EXT_AnalyzeBluetoothType)(double data_rate, char *analysis_type);
typedef int		(*LP_IQ2010EXT_AnalyzeSpectralMeasurementType)(int spectralType);
typedef int		(*LP_IQ2010EXT_RxPerType)( int               waveformIndex,
		double            freqMHz,
		double            powerLevelStartdBm,
		double            powerLevelStopdBm,
		double            stepdB,
		int               packetCount,
		int				rfPort,
		double            ackPowerRmsdBm,
		double            ackTriggerLeveldBm
		);
typedef int		(*LP_IQ2010EXT_NewMultiSegmentWaveformType)();
typedef int		(*LP_IQ2010EXT_AddWaveformType)(const char *modFile, unsigned int *waveformIndex );
typedef int		(*LP_IQ2010EXT_FinalizeMultiSegmentWaveformType)();
typedef int		(*LP_IQ2010EXT_GetNumberOfMeasurementElementsType)(char *measurementName, int indexResult, int *numberOfMeasurementElement);
typedef int		(*LP_IQ2010EXT_GetNumberOfPerPowerLevelsType)(char *measurementName, int *numberOfPerPowerLevel);
typedef int		(*LP_IQ2010EXT_GetDoubleMeasurementsType)(char *measurementName, double *average, double *minimum, double *maximum);
typedef int		(*LP_IQ2010EXT_GetStringMeasurementType)(char *measurementName, char *result, int  bufferSize, int indexResult);
typedef int		(*LP_IQ2010EXT_GetVectorMeasurementType)(char *measurementName, double values[], int bufferLength, int *dataSize, int indexResult);
typedef int		(*LP_IQ2010EXT_GetIntMeasurementType)(char *measurementName, int *result, int indexResult);
typedef int		(*LP_IQ2010EXT_InitType)(void);
typedef int		(*LP_IQ2010EXT_TerminateType)(void);

/*------------------*
 *	GPS Functions	*
 *-------------------*/

typedef int		(*LP_GPS_SetActivateType)();
typedef int		(*LP_GPS_LoadScenarioFileType)(char* fileName, IQV_GPS_TRIGGER_STATE triggerState);
typedef int		(*LP_GPS_PlayScenarioFileType)(double powerDbm, double pathlossDb);
typedef int		(*LP_GPS_ModulatedModeType)(int Nav_Mode, double pathlossDb, double powerA[6], int satelliteNumA[6], int dopplerFrequency, IQV_GPS_TRIGGER_STATE triggerState);
typedef int		(*LP_GPS_ContinueWaveType)(double powerDbm, double pathlossDb, IQV_GPS_TRIGGER_STATE triggerState, int freqOffset);
typedef int		(*LP_GPS_SetDeactivateType)();
typedef int		(*LP_GPS_GetChannelInfoType)(IQV_GPS_CHANNEL_NUMBER channelNumber,
		int *satelliteNumber,
		double *powerDbm,
		IQV_GPS_MODULATION_STATE *modulationState);
typedef int		(*LP_GPS_SetChannelInfoType)(IQV_GPS_CHANNEL_NUMBER channelNumber,
		int satelliteNumber,
		double powerDbm,
		IQV_GPS_MODULATION_STATE modulationState);
typedef int		(*LP_GPS_SetDataType)(const char* inputName, const char* inputValue);
typedef double		(*LP_GPS_CaculateTesterPowerType)(double powerDbm, double pathlossDb);

/*------------------------------*
 *	Glonass Functions	*
 *------------------------------*/
typedef int (*LP_PrintGlonassParametersType) (bool forceUpdate);
typedef int (*LP_Glonass_SetOperationModeType) (IQV_GPS_OPERATION_MODE operationMode);
typedef int (*LP_Glonass_SetPowerType) (double powerdBm);
typedef int (*LP_Glonass_SetFrequencyType) (int frequencyChannel);
typedef int (*LP_Glonass_SetRfOutputType) (IQV_GPS_RF_OUTPUT rfOutput);
typedef int (*LP_Glonass_SetActivateType)(void);
typedef int (*LP_Glonass_SetDeactivateType)();

/*----------------------------*
 *		FM Functions		  *
 *-----------------------------*/
typedef int		(*LP_SetLpcPathType)(char *litePointConnectionPath);
typedef int		(*LP_FM_SetVsgType)(unsigned int carrierFreqHz,
		double carrierPowerdBm,
		int modulationEnable,
		unsigned int totalFmDeviationHz,
		int stereoEnable,
		unsigned int pilotDeviationHz,
		int rdsEnable,
		unsigned int rdsDeviationHz,
		unsigned int preEmphasisUs,
		char* rdsTransmitString) ;
typedef int		(*LP_FM_GetVsgSettingsType)(unsigned int *carrierFreqHz,
		double *carrierPowerdBm,
		int *modulationEnabled,
		unsigned int *totalFmDeviationHz,
		int *stereoEnabled,
		unsigned int *pilotDeviationHz,
		int *rdsEnabled,
		unsigned int *rdsDeviationHz,
		unsigned int *preEmphasisMode
		);
typedef int		(*LP_FM_SetAudioSingleToneType)(double audioToneFreqHz,
		int leftRightChannelSelect,
		double audioToneAmpPercent,
		int stereo);
typedef int		(*LP_FM_SetAudioToneArrayType)(double* freqHz,
		int* channelSelect,
		double* amplitudePercent,
		int stereo,
		unsigned int toneCount);
typedef int		(*LP_FM_SetAudioToneArrayDeviationType)(double* freqHz,
		int* channelSelect,
		double* amplitudeDeviationHz,
		int stereo,
		unsigned int toneCount);
typedef int		(*LP_FM_GetAudioToneArrayType)(double *freqHz,
		int *channelSelect,
		double *amplitudePercent,
		unsigned int toneCount);
typedef int		(*LP_FM_DeleteAudioTonesType)();
typedef int		(*LP_FM_StartVsgType)();
typedef int		(*LP_FM_StopVsgType)();
typedef int		(*LP_FM_SetVsgDistortionType)(int amDistortionEnable,
		unsigned int amFrequencyHz,
		unsigned int amDepthPercent);
typedef int		(*LP_FM_GetVsgDistortionType)(double *amDistortionEnableStatus,
		unsigned int *amFrequencyHz,
		unsigned int *amDepthPercent);
typedef int		(*LP_FM_StartVsgInterferenceType)(double interfererRelativePowerdBm,
		int interfererCarrierOffsetHz,
		int interfererPeakFreqDeviationHz,
		int interfererModulationEnable,
		unsigned int interfererAudioFreqHz);
typedef int		(*LP_FM_GetVsgInterferenceSettingsType)(double *interfererRelativePowerdBm,
		int *interfererCarrierOffsetHz,
		int *interfererPeakFreqDeviationHz,
		int *interfererModulationEnabled,
		unsigned int *interfererAudioFreqHz);
typedef int		(*LP_FM_StopVsgInterferenceType)();
typedef int		(*LP_FM_SetVsaType)(unsigned int carrierFreqHz,
		double expectedPeakInputPowerdBm);
typedef int		(*LP_FM_GetVsaSettingsType)(unsigned int *carrierFreqHz,
		double *expectedPeakInputPowerdBm);
typedef int		(*LP_FM_VsaDataCaptureType)(double captureTimemillisec);
typedef int		(*LP_FM_Analyze_RFType)(int rfRBWHz,
		int rfOBWPercent,
		int rfPowerMeasBWHz,
		int windowType);
typedef int		(*LP_FM_Analyze_DemodType)(int audioRBWHz);
typedef int		(*LP_FM_Analyze_Audio_MonoType)(int audioRBWHz,
		int audioFreqLowLimitHz,
		int audioFreqHiLimitHz,
		int filterCount,
		int filterType1,
		double filterParam1,
		int filterType2,
		double filterParam2,
		int filterType3 ,
		double filterParam3 );
typedef int		(*LP_FM_Analyze_Audio_StereoType)(int audioRBWHz,
		int audioFreqLowLimitHz,
		int audioFreqHiLimitHz,
		int filterCount,
		int filterType1,
		double filterParam1,
		int filterType2,
		double filterParam2,
		int filterType3,
		double filterParam3);
typedef double		(*LP_FM_GetScalarMeasurementType)(char *measurement, int index);
typedef int		(*LP_FM_GetVectorMeasurementType)(char *measurement, double bufferReal[], double bufferImag[], int bufferLength);
typedef int		(*LP_FM_AudioDataCaptureType)(char* saveFileLocation,
		double captureDurationMilliSec,
		int samplingRate,
		int stereo);
typedef int		(*LP_FM_LoadAudioCaptureType)(char* fileName);
typedef int		(*LP_FM_AudioStimulusGenerateAndPlayMultiToneType)(double sampleRateHz,
		double deviationPercent,
		double peakVoltageLevelVolts,
		int toneCount,
		int stereoEnable,
		int* leftRightRelation,
		double* freqHz ,
		double durationMilliSeconds,
		char* audioWaveFileOutput);
typedef int		(*LP_FM_AudioStimulusGenerateAndPlaySingleToneType)(double sampleRateHz,
		double deviationPercent,
		double peakVoltageLevelVolts,
		int stereoEnable,
		int leftRightRelation,
		double freqHz ,
		double durationMilliSeconds,
		char* audioWaveFileOutput);
typedef int		(*LP_FM_StopAudioPlayType)();
typedef int		(*LP_GetCaptureType)(int    dut,
		int    captureIndex,
		double *real[],
		double *imag[],
		int    length[]);
typedef int		(*LP_EnableMultiThreadType)(int enableMultiThread,
		int numberOfThreads);
typedef int		(*LP_SetUserDataPtrType)(void *userData);
typedef int		(*LP_SetDefaultNfcType)();
typedef int		(*LP_SetNfcTxAmplitudeType)(IQV_USER_SIGNAL_AMPLITUDE_TYPE userSignalAmplitudeType, double userSignalAmplitude);
typedef int		(*LP_SetTriggerMaskNfcType)();
typedef int		(*LP_InitiateNfcType)(char *fileName);
typedef int		(*LP_SetRxNfcType)(double triggerTimeout, double sampleingTimeSecs);
typedef int		(*LP_SetTxNfcType)();
typedef int		(*LP_ContCaptureType)(IQV_DC_CONT_ENUM dcContEnum);
typedef int		(*LP_TerminateNfcType)();
typedef int		(*LP_AnalysisNfcType)();
typedef int		(*LP_SetVsgModulation_SetPlayConditionType)(char *modFileName, bool autoPlay, int loadInternalWaveform);
typedef int		(*LP_SetVsaBluetooth_BTShiftHzType)(double rfFreqHz, double rfAmplDb, int port, double extAttenDb, double triggerLevelDb, double triggerPreTime6, double btShiftHz);
typedef int		(*LP_FM_SetVsa_Agc_OnType)(unsigned int carrierFreqHz, double expectedPeakInputPowerdBm);

typedef double	(*LP_GetScalarMeasurement_NoTimerType)(char *measurement, int index);
typedef int		(*LP_GetVectorMeasurement_NoTimerType)(char *measurement, double bufferReal[], double bufferImag[], int bufferLength);
typedef int		(*LP_GetStringMeasurement_NoTimerType)(char *measurement, char bufferChar[], int bufferLength);

typedef int		(*LP_FM_SetAudioToneModeAmplitudeType)(void);  // IQlite Merge; Tracy Yu; 2012-04-06

//DTNA

typedef int		(*LP_SetVsg_triggerTypeType)(double rfFreqHz, double rfPowerLeveldBm, int port, int triggerType);
typedef int		(*LP_SaveVsaSignalFileTextType)(char *txtFileName);
typedef int		(*LP_SaveTruncateCaptureType)(char *sigFileName);
typedef int		(*LP_Analyze80211acType)(char *mode,
		int enablePhaseCorr,
		int enableSymTimingCorr   ,
		int enableAmplitudeTracking   ,
		int decodePSDU                ,
		int enableFullPacketChannelEst ,
		int frequencyCorr ,
		char *referenceFile,
		int packetFormat,
		int numberOfPacketToAnalysis,
		double prePowStartSec,
		double prePowStopSec);
typedef int		(*LP_AnalyzeVHT80MaskType)();
typedef int		(*LP_FastCalGetPowerDataType) (int bGetOrFree);
typedef int		(*LP_PwrCalFreeElementType)(void);
typedef int		(*LP_PwrCalFastGetElementType)(void);
typedef int		(*LP_FastCalMeasPowerType) (unsigned int start_time, unsigned int  stop_time, double *result);
typedef int		(*SetTesterHwVersionType)(double hwVersion);
typedef int		(*GetTesterHwVersionType)(double *hwVersion);

//---------------------------------------------------------------
// Define Var pointers
//---------------------------------------------------------------
LP_loggerIQmeasureType			LP_loggerIQmeasure_Ptr;
LP_timerIQmeasureType			LP_timerIQmeasure_Ptr;

//---------------------------------------------------------------
// Define Function pointers
//---------------------------------------------------------------
LP_InitType						LP_Init_Ptr;
LP_TermType						LP_Term_Ptr;
LP_GetIQapiHndlType				LP_GetIQapiHndl_Ptr;
LP_GetVersionType				LP_GetVersion_Ptr;
LP_InitTesterType				LP_InitTester_Ptr;
LP_InitTester2Type				LP_InitTester2_Ptr;
LP_InitTester3Type				LP_InitTester3_Ptr;
LP_InitTester4Type				LP_InitTester4_Ptr;
LP_ConCloseType					LP_ConClose_Ptr;
LP_ConOpenType					LP_ConOpen_Ptr;
LP_GetErrorStringType			LP_GetErrorString_Ptr;
LP_GetIQapiHndlLastErrMsgType	LP_GetIQapiHndlLastErrMsg_Ptr;
LP_DualHead_ConOpenType			LP_DualHead_ConOpen_Ptr;
LP_DualHead_GetTokenIDType		LP_DualHead_GetTokenID_Ptr;
LP_DualHead_ConCloseType		LP_DualHead_ConClose_Ptr;
LP_DualHead_ObtainControlType	LP_DualHead_ObtainControl_Ptr;
LP_DualHead_ReleaseControlType	LP_DualHead_ReleaseControl_Ptr;
LP_SetAlcModeType				LP_SetAlcMode_Ptr;
LP_GetAlcModeType				LP_GetAlcMode_Ptr;
LP_SetVsgType					LP_SetVsg_Ptr;
LP_SetVsg_CompensationType		LP_SetVsg_Compensation_Ptr;
LP_SetVsgTriggerTypeType		LP_SetVsgTriggerType_Ptr;
LP_SetVsg_GapPowerType			LP_SetVsg_GapPower_Ptr;
LP_SetVsgNxNType				LP_SetVsgNxN_Ptr;
LP_SetVsgCwType					LP_SetVsgCw_Ptr;
LP_SaveSignalArraysType			LP_SaveSignalArrays_Ptr;
LP_SaveIQDataToModulationFileType	LP_SaveIQDataToModulationFile_Ptr;
LP_SetVsgModulationType			LP_SetVsgModulation_Ptr;
LP_SetAnalysisParameterIntegerType	LP_SetAnalysisParameterInteger_Ptr;
LP_SetAnalysisParameterIntegerArrayType		LP_SetAnalysisParameterIntegerArray_Ptr;
LP_CopyVsaCaptureDataType		LP_CopyVsaCaptureData_Ptr;
LP_SaveVsaSignalFileType		LP_SaveVsaSignalFile_Ptr;
LP_LoadVsaSignalFileType		LP_LoadVsaSignalFile_Ptr;
LP_SaveVsaGeneratorFileType		LP_SaveVsaGeneratorFile_Ptr;
LP_SetVsaBluetoothType			LP_SetVsaBluetooth_Ptr;
LP_SetVsaType					LP_SetVsa_Ptr;
LP_SetVsaTriggerTimeoutType		LP_SetVsaTriggerTimeout_Ptr;
LP_SetVsaNxNType				LP_SetVsaNxN_Ptr;
LP_SetVsaAmplitudeToleranceType	LP_SetVsaAmplitudeTolerance_Ptr;
LP_AgcType						LP_Agc_Ptr;
LP_SetFrameCntType				LP_SetFrameCnt_Ptr;
LP_TxDoneType					LP_TxDone_Ptr;
LP_EnableVsgRFType				LP_EnableVsgRF_Ptr;
LP_EnableVsgRFNxNType			LP_EnableVsgRFNxN_Ptr;
LP_EnableSpecifiedVsgRFType		LP_EnableSpecifiedVsgRF_Ptr;
LP_EnableSpecifiedVsaRFType		LP_EnableSpecifiedVsaRF_Ptr;
LP_GetVsaSettingsType			LP_GetVsaSettings_Ptr;
LP_VsaDataCaptureType			LP_VsaDataCapture_Ptr;
LP_GetSampleDataType			LP_GetSampleData_Ptr;
LP_GetHndlDataPointersType		LP_GetHndlDataPointers_Ptr;
LP_SaveUserDataToSigFileType	LP_SaveUserDataToSigFile_Ptr;
LP_SelectCaptureRangeForAnalysisType	LP_SelectCaptureRangeForAnalysis_Ptr;
LP_Analyze80216dType			LP_Analyze80216d_Ptr;
LP_Analyze80216eType			LP_Analyze80216e_Ptr;
LP_Analyze80211pType			LP_Analyze80211p_Ptr;
LP_Analyze80211agType			LP_Analyze80211ag_Ptr;
LP_Analyze80211nType			LP_Analyze80211n_Ptr;
LP_AnalyzeMimoType				LP_AnalyzeMimo_Ptr;
LP_Analyze80211bType			LP_Analyze80211b_Ptr;
LP_AnalyzePowerType				LP_AnalyzePower_Ptr;
LP_AnalyzeFFTType				LP_AnalyzeFFT_Ptr;
LP_AnalyzeCCDFType				LP_AnalyzeCCDF_Ptr;
LP_AnalyzeCWType				LP_AnalyzeCW_Ptr;
LP_AnalyzeCWFreqType			LP_AnalyzeCWFreq_Ptr;
LP_AnalysisWaveType				LP_AnalysisWave_Ptr;
LP_AnalyzeSidelobeType			LP_AnalyzeSidelobe_Ptr;
LP_AnalyzePowerRampOFDMType		LP_AnalyzePowerRampOFDM_Ptr;
LP_AnalyzePowerRamp80211bType	LP_AnalyzePowerRamp80211b_Ptr;
LP_AnalyzeBluetoothType			LP_AnalyzeBluetooth_Ptr;
LP_AnalyzeZigbeeType			LP_AnalyzeZigbee_Ptr;
LP_AnalyzeHT40MaskType			LP_AnalyzeHT40Mask_Ptr;
LP_GetScalarMeasurmentType		LP_GetScalarMeasurment_Ptr;
LP_GetVectorMeasurmentType		LP_GetVectorMeasurment_Ptr;
LP_GetStringMeasurmentType		LP_GetStringMeasurment_Ptr;
LP_GetScalarMeasurementType		LP_GetScalarMeasurement_Ptr;
LP_GetVectorMeasurementType		LP_GetVectorMeasurement_Ptr;
LP_GetStringMeasurementType		LP_GetStringMeasurement_Ptr;
LP_PlotDataCaptureType			LP_PlotDataCapture_Ptr;
LP_PlotType						LP_Plot_Ptr;
LP_StartIQmeasureTimerType		LP_StartIQmeasureTimer_Ptr;
LP_StopIQmeasureTimerType		LP_StopIQmeasureTimer_Ptr;
LP_ReportTimerDurationsType		LP_ReportTimerDurations_Ptr;
LP_ScpiCommandSetType          LP_ScpiCommandSet_Ptr;
LP_ScpiCommandQueryType         LP_ScpiCommandQuery_Ptr;

LP_GetScalarMeasurement_NoTimerType	LP_GetScalarMeasurement_NoTimer_Ptr;
LP_GetVectorMeasurement_NoTimerType	LP_GetVectorMeasurement_NoTimer_Ptr;
LP_GetStringMeasurement_NoTimerType	LP_GetStringMeasurement_NoTimer_Ptr;

//Add LP_SetDefault(); Tracy Yu; 2012-03-31
LP_SetDefaultType				LP_SetDefault_Ptr;
LP_FM_SetAudioToneModeAmplitudeType	LP_FM_SetAudioToneModeAmplitude_Ptr;
LP_FM_SetFrequencyType			LP_FM_SetFrequency_Ptr;
LP_FM_SetCarrierPowerType		LP_FM_SetCarrierPower_Ptr;

//For MTK META
LP_GetResultSpectralMaskType          LP_GetResultSpectralMask_Ptr;
LP_GetResultSpectralMask_HT40Type     LP_GetResultSpectralMask_HT40_Ptr;
LP_LoResultSpectralFlatness11AGType   LP_LoResultSpectralFlatness11AG_Ptr;
LP_LoResultSpectralFlatness11NType    LP_LoResultSpectralFlatness11N_Ptr;


/*--------------------*
 *	IQ2010 Ext		  *
 *---------------------*/
LP_IQ2010EXT_NewMultiAnalysisCaptureType		LP_IQ2010EXT_NewMultiAnalysisCapture_Ptr;
LP_IQ2010EXT_AddMultiAnalysisCaptureType		LP_IQ2010EXT_AddMultiAnalysisCapture_Ptr;
LP_IQ2010EXT_InitiateMultiAnalysisCaptureType	LP_IQ2010EXT_InitiateMultiAnalysisCapture_Ptr;
LP_IQ2010EXT_FinishMultiAnalysisCaptureType		LP_IQ2010EXT_FinishMultiAnalysisCapture_Ptr;
LP_IQ2010EXT_InitiateMultiCaptureType			LP_IQ2010EXT_InitiateMultiCapture_Ptr;
LP_IQ2010EXT_InitiateMultiCaptureHT40Type		LP_IQ2010EXT_InitiateMultiCaptureHT40_Ptr;
LP_IQ2010EXT_FinishMultiCaptureType				LP_IQ2010EXT_FinishMultiCapture_Ptr;
LP_IQ2010EXT_Analyze80211agType					LP_IQ2010EXT_Analyze80211ag_Ptr;
LP_IQ2010EXT_Analyze80211nSISOType				LP_IQ2010EXT_Analyze80211nSISO_Ptr;
LP_IQ2010EXT_Analyze80211nCompositeType			LP_IQ2010EXT_Analyze80211nComposite_Ptr;
LP_IQ2010EXT_Analyze80211bType					LP_IQ2010EXT_Analyze80211b_Ptr;
LP_IQ2010EXT_AnalyzePowerType					LP_IQ2010EXT_AnalyzePower_Ptr;
LP_IQ2010EXT_AnalyzeFFTType						LP_IQ2010EXT_AnalyzeFFT_Ptr;
LP_IQ2010EXT_AnalyzeMaskMeasurementType			LP_IQ2010EXT_AnalyzeMaskMeasurement_Ptr;
LP_IQ2010EXT_AnalyzePowerRamp80211bType			LP_IQ2010EXT_AnalyzePowerRamp80211b_Ptr;
LP_IQ2010EXT_AnalyzeBluetoothType				LP_IQ2010EXT_AnalyzeBluetooth_Ptr;
LP_IQ2010EXT_AnalyzeSpectralMeasurementType		LP_IQ2010EXT_AnalyzeSpectralMeasurement_Ptr;
LP_IQ2010EXT_RxPerType							LP_IQ2010EXT_RxPer_Ptr;
LP_IQ2010EXT_NewMultiSegmentWaveformType		LP_IQ2010EXT_NewMultiSegmentWaveform_Ptr;
LP_IQ2010EXT_AddWaveformType					LP_IQ2010EXT_AddWaveform_Ptr;
LP_IQ2010EXT_FinalizeMultiSegmentWaveformType	LP_IQ2010EXT_FinalizeMultiSegmentWaveform_Ptr;
LP_IQ2010EXT_GetNumberOfMeasurementElementsType	LP_IQ2010EXT_GetNumberOfMeasurementElements_Ptr;
LP_IQ2010EXT_GetNumberOfPerPowerLevelsType		LP_IQ2010EXT_GetNumberOfPerPowerLevels_Ptr;
LP_IQ2010EXT_GetDoubleMeasurementsType			LP_IQ2010EXT_GetDoubleMeasurements_Ptr;
LP_IQ2010EXT_GetStringMeasurementType			LP_IQ2010EXT_GetStringMeasurement_Ptr;
LP_IQ2010EXT_GetVectorMeasurementType			LP_IQ2010EXT_GetVectorMeasurement_Ptr;
LP_IQ2010EXT_GetIntMeasurementType				LP_IQ2010EXT_GetIntMeasurement_Ptr;
LP_IQ2010EXT_InitType							LP_IQ2010EXT_Init_Ptr;
LP_IQ2010EXT_TerminateType						LP_IQ2010EXT_Terminate_Ptr;

/*------------*
 *	 GPS	  *
 *-------------*/
LP_GPS_SetActivateType			LP_GPS_SetActivate_Ptr;
LP_GPS_LoadScenarioFileType		LP_GPS_LoadScenarioFile_Ptr;
LP_GPS_PlayScenarioFileType		LP_GPS_PlayScenarioFile_Ptr;
LP_GPS_ModulatedModeType		LP_GPS_ModulatedMode_Ptr;
LP_GPS_ContinueWaveType			LP_GPS_ContinueWave_Ptr;
LP_GPS_SetDeactivateType		LP_GPS_SetDeactivate_Ptr;
LP_GPS_GetChannelInfoType		LP_GPS_GetChannelInfo_Ptr;
LP_GPS_SetChannelInfoType		LP_GPS_SetChannelInfo_Ptr;
LP_GPS_SetDataType				LP_GPS_SetData_Ptr;
LP_GPS_CaculateTesterPowerType	LP_GPS_CaculateTesterPower_Ptr;

/*----------------------*
 *	 Glonass	*
 *----------------------*/
LP_PrintGlonassParametersType	LP_PrintGlonassParameters_Ptr;
LP_Glonass_SetOperationModeType LP_Glonass_SetOperationMode_Ptr;
LP_Glonass_SetPowerType		LP_Glonass_SetPower_Ptr;
LP_Glonass_SetFrequencyType	LP_Glonass_SetFrequency_Ptr;
LP_Glonass_SetRfOutputType	LP_Glonass_SetRfOutput_Ptr;
LP_Glonass_SetActivateType	LP_Glonass_SetActivate_Ptr;
LP_Glonass_SetDeactivateType 	LP_Glonass_SetDeactivate_Ptr;

/*------*
 *  FM	*
 *-------*/
LP_SetLpcPathType				LP_SetLpcPath_Ptr;
LP_FM_SetVsgType				LP_FM_SetVsg_Ptr;
LP_FM_GetVsgSettingsType		LP_FM_GetVsgSettings_Ptr;
LP_FM_SetAudioSingleToneType	LP_FM_SetAudioSingleTone_Ptr;
LP_FM_SetAudioToneArrayType		LP_FM_SetAudioToneArray_Ptr;
LP_FM_SetAudioToneArrayDeviationType	LP_FM_SetAudioToneArrayDeviation_Ptr;
LP_FM_GetAudioToneArrayType		LP_FM_GetAudioToneArray_Ptr;
LP_FM_DeleteAudioTonesType		LP_FM_DeleteAudioTones_Ptr;
LP_FM_StartVsgType				LP_FM_StartVsg_Ptr;
LP_FM_StopVsgType				LP_FM_StopVsg_Ptr;
LP_FM_SetVsgDistortionType		LP_FM_SetVsgDistortion_Ptr;
LP_FM_GetVsgDistortionType		LP_FM_GetVsgDistortion_Ptr;
LP_FM_StartVsgInterferenceType	LP_FM_StartVsgInterference_Ptr;
LP_FM_GetVsgInterferenceSettingsType	LP_FM_GetVsgInterferenceSettings_Ptr;
LP_FM_StopVsgInterferenceType	LP_FM_StopVsgInterference_Ptr;
LP_FM_SetVsaType				LP_FM_SetVsa_Ptr;
LP_FM_GetVsaSettingsType		LP_FM_GetVsaSettings_Ptr;
LP_FM_VsaDataCaptureType		LP_FM_VsaDataCapture_Ptr;
LP_FM_Analyze_RFType			LP_FM_Analyze_RF_Ptr;
LP_FM_Analyze_DemodType			LP_FM_Analyze_Demod_Ptr;
LP_FM_Analyze_Audio_MonoType	LP_FM_Analyze_Audio_Mono_Ptr;
LP_FM_Analyze_Audio_StereoType	LP_FM_Analyze_Audio_Stereo_Ptr;
LP_FM_GetScalarMeasurementType	LP_FM_GetScalarMeasurement_Ptr;
LP_FM_GetVectorMeasurementType	LP_FM_GetVectorMeasurement_Ptr;
LP_FM_AudioDataCaptureType		LP_FM_AudioDataCapture_Ptr;
LP_FM_LoadAudioCaptureType		LP_FM_LoadAudioCapture_Ptr;
LP_FM_AudioStimulusGenerateAndPlayMultiToneType	LP_FM_AudioStimulusGenerateAndPlayMultiTone_Ptr;
LP_FM_AudioStimulusGenerateAndPlaySingleToneType	LP_FM_AudioStimulusGenerateAndPlaySingleTone_Ptr;
LP_FM_StopAudioPlayType			LP_FM_StopAudioPlay_Ptr;


LP_GetCaptureType				LP_GetCapture_Ptr;
LP_EnableMultiThreadType		LP_EnableMultiThread_Ptr;
LP_SetUserDataPtrType			LP_SetUserDataPtr_Ptr;
LP_SetDefaultNfcType			LP_SetDefaultNfc_Ptr;
LP_SetNfcTxAmplitudeType		LP_SetNfcTxAmplitude_Ptr;
LP_SetTriggerMaskNfcType		LP_SetTriggerMaskNfc_Ptr;
LP_InitiateNfcType				LP_InitiateNfc_Ptr;
LP_SetRxNfcType					LP_SetRxNfc_Ptr;
LP_SetTxNfcType					LP_SetTxNfc_Ptr;
LP_ContCaptureType				LP_ContCapture_Ptr;
LP_TerminateNfcType				LP_TerminateNfc_Ptr;
LP_AnalysisNfcType				LP_AnalysisNfc_Ptr;
LP_SetVsgModulation_SetPlayConditionType	LP_SetVsgModulation_SetPlayCondition_Ptr;
LP_SetVsaBluetooth_BTShiftHzType	LP_SetVsaBluetooth_BTShiftHz_Ptr;
LP_FM_SetVsa_Agc_OnType			LP_FM_SetVsa_Agc_On_Ptr;

//DTNA
LP_SetVsg_triggerTypeType		LP_SetVsg_triggerType_Ptr;
LP_SaveVsaSignalFileTextType	LP_SaveVsaSignalFileText_Ptr;
LP_SaveTruncateCaptureType		LP_SaveTruncateCapture_Ptr;
LP_Analyze80211acType			LP_Analyze80211ac_Ptr;
LP_AnalyzeVHT80MaskType			LP_AnalyzeVHT80Mask_Ptr;
LP_FastCalGetPowerDataType		LP_FastCalGetPowerData_Ptr;
LP_PwrCalFreeElementType		LP_PwrCalFreeElement_Ptr;
LP_PwrCalFastGetElementType		LP_PwrCalFastGetElement_Ptr;
LP_FastCalMeasPowerType			LP_FastCalMeasPower_Ptr;
SetTesterHwVersionType			SetTesterHwVersion_Ptr;
GetTesterHwVersionType			GetTesterHwVersion_Ptr;
//---------------------------------------------------------------
// Loading dynamic library function
//---------------------------------------------------------------

void LoadLibraryPopUpErrorMessageBox(LPTSTR lpszFunction)  //To pop up windows error message when used with COM objects!! , need to verify with MAC builds
{
	static bool DisplayMessage = true;

	if(DisplayMessage)
	{
		TCHAR szBuf[4096];
		TCHAR currDir[4096];
		//TCHAR dllDir[4096];
		//TCHAR sysDir[4096];
		//TCHAR winDir[4096];
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				dw,
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
				(LPTSTR) &lpMsgBuf,
				0, NULL );

		_wgetcwd(currDir, 4096);
		//GetDllDirectory(4096,dllDir);
		//GetSystemDirectory(sysDir, 4096);
		//GetWindowsDirectory(winDir, 4096);

		wsprintf(szBuf,
				L"\"LoadLibrary()\" failed with error %d:\n\t%s\nDLL Name:\n\t%s\nCurrrent Working Dir:\n\t%s\n", /*Dll Search Dir:\n\t%s\nSystem Dir:\n\t%s\nWindows Dir:\n\t%s\n", */
				dw, lpMsgBuf, lpszFunction, currDir/*, dllDir, sysDir, winDir*/);

		MessageBox(NULL, szBuf, L"Error", MB_OK);

		LocalFree(lpMsgBuf);
		DisplayMessage = false;
		//ExitProcess(dw);
	}
}


static int loadDynamicLibrary(void)
{
	static int alreadyLoaded = 0;
	//char currentPath[MAX_BUFFER_SIZE];

	if (alreadyLoaded)
		return 0;

	//    getcwd(currentPath, sizeof(currentPath));
	//    printf("Before working folder %s\n", currentPath);


	//	//------------------------------------------------------------------------
	//	// Loading  library
	//	//------------------------------------------------------------------------

	DynamiclibraryHandle = LoadLibrary(CA2W(dynamiclibraryName));
	if (!DynamiclibraryHandle)
	{
		LoadLibraryPopUpErrorMessageBox(CA2W(dynamiclibraryName));
		printf("\nError in loading %s", dynamiclibraryName);
		return -1;
	}
	else
	{
	}

	//Get loggerIQmeasure and timerIQmeasure from IQmeasure_xx.dll before using it in the IQmeasure.dll
	LP_loggerIQmeasure_Ptr = (LP_loggerIQmeasureType) GetProcAddress((HMODULE)DynamiclibraryHandle, "loggerIQmeasure");
	LP_timerIQmeasure_Ptr = (LP_timerIQmeasureType) GetProcAddress((HMODULE)DynamiclibraryHandle, "timerIQmeasure");

	::LOGGER_Write_Ext(LOG_IQMEASURE, *LP_loggerIQmeasure_Ptr, LOGGER_INFORMATION, "[IQMEASURE],Dynamic library \"%s\" loading OK!\n",dynamiclibraryName);


	//--------------------------------------------------------------
	// searching WiFi function pointer in dylib/DLL
	//--------------------------------------------------------------
	LOAD_DLLPTR(LP_GetScalarMeasurement_NoTimer);
	LOAD_DLLPTR(LP_GetStringMeasurement_NoTimer);
	LOAD_DLLPTR(LP_GetVectorMeasurement_NoTimer);
	LOAD_DLLPTR(LP_Init);
	LOAD_DLLPTR(LP_Term);
	LOAD_DLLPTR(LP_GetIQapiHndl);
	LOAD_DLLPTR(LP_GetVersion);
	LOAD_DLLPTR(LP_InitTester);
	LOAD_DLLPTR(LP_InitTester2);
	LOAD_DLLPTR(LP_InitTester3);
	LOAD_DLLPTR(LP_InitTester4);
	LOAD_DLLPTR(LP_ConClose);
	LOAD_DLLPTR(LP_ConOpen);
	LOAD_DLLPTR(LP_GetErrorString);
	LOAD_DLLPTR(LP_GetIQapiHndlLastErrMsg);
	LOAD_DLLPTR(LP_DualHead_ConOpen);
	LOAD_DLLPTR(LP_DualHead_GetTokenID);
	LOAD_DLLPTR(LP_DualHead_ConClose);
	LOAD_DLLPTR(LP_DualHead_ObtainControl);
	LOAD_DLLPTR(LP_DualHead_ReleaseControl);
	//LOAD_DLLPTR(LP_SetAlcMode);
	//LOAD_DLLPTR(LP_GetAlcMode);
	LOAD_DLLPTR(LP_SetVsg);
	LOAD_DLLPTR(LP_FM_SetFrequency);
	LOAD_DLLPTR(LP_FM_SetCarrierPower);


	LOAD_DLLPTR(LP_SetVsg_GapPower);
	LOAD_DLLPTR(LP_SetVsgNxN);
	LOAD_DLLPTR(LP_SetVsgCw);
	LOAD_DLLPTR(LP_SaveSignalArrays);
	LOAD_DLLPTR(LP_SaveIQDataToModulationFile);
	LOAD_DLLPTR(LP_SetVsgModulation);
	LOAD_DLLPTR(LP_SetVsgModulation_SetPlayCondition);
	LOAD_DLLPTR(LP_SetAnalysisParameterInteger);
	LOAD_DLLPTR(LP_SetAnalysisParameterIntegerArray);
	LOAD_DLLPTR(LP_CopyVsaCaptureData);
	LOAD_DLLPTR(LP_SaveVsaSignalFile);
	LOAD_DLLPTR(LP_LoadVsaSignalFile);
	LOAD_DLLPTR(LP_SaveVsaGeneratorFile);
	LOAD_DLLPTR(LP_SetVsaBluetooth);
	LOAD_DLLPTR(LP_SetVsa);
	LOAD_DLLPTR(LP_SetVsaTriggerTimeout);
	LOAD_DLLPTR(LP_SetVsaNxN);
	LOAD_DLLPTR(LP_SetVsaAmplitudeTolerance);
	LOAD_DLLPTR(LP_Agc);
	LOAD_DLLPTR(LP_SetFrameCnt);
	LOAD_DLLPTR(LP_TxDone);
	LOAD_DLLPTR(LP_EnableVsgRF);
	LOAD_DLLPTR(LP_EnableVsgRFNxN);
	LOAD_DLLPTR(LP_EnableSpecifiedVsgRF);
	LOAD_DLLPTR(LP_EnableSpecifiedVsaRF);
	LOAD_DLLPTR(LP_GetVsaSettings);
	LOAD_DLLPTR(LP_VsaDataCapture);
	LOAD_DLLPTR(LP_GetSampleData);
	LOAD_DLLPTR(LP_SelectCaptureRangeForAnalysis);
	LOAD_DLLPTR(LP_Analyze80216d);
	LOAD_DLLPTR(LP_Analyze80216e);
	LOAD_DLLPTR(LP_Analyze80211p);
	LOAD_DLLPTR(LP_Analyze80211ag);
	LOAD_DLLPTR(LP_Analyze80211n);
	LOAD_DLLPTR(LP_AnalyzeMimo);
	LOAD_DLLPTR(LP_Analyze80211b);
	LOAD_DLLPTR(LP_AnalyzePower);
	LOAD_DLLPTR(LP_AnalyzeFFT);
	LOAD_DLLPTR(LP_AnalyzeCCDF);
	LOAD_DLLPTR(LP_AnalyzeCW);
	LOAD_DLLPTR(LP_AnalyzeCWFreq);
	LOAD_DLLPTR(LP_AnalysisWave);
	LOAD_DLLPTR(LP_AnalyzeSidelobe);
	LOAD_DLLPTR(LP_AnalyzePowerRampOFDM);
	LOAD_DLLPTR(LP_AnalyzePowerRamp80211b);
	LOAD_DLLPTR(LP_AnalyzeBluetooth);
	LOAD_DLLPTR(LP_AnalyzeZigbee);
	LOAD_DLLPTR(LP_AnalyzeHT40Mask);
	LOAD_DLLPTR(LP_GetScalarMeasurment);
	LOAD_DLLPTR(LP_GetVectorMeasurment);
	LOAD_DLLPTR(LP_GetStringMeasurment);
	LOAD_DLLPTR(LP_GetScalarMeasurement);
	LOAD_DLLPTR(LP_GetVectorMeasurement);
	LOAD_DLLPTR(LP_GetStringMeasurement);
	LOAD_DLLPTR(LP_PlotDataCapture);
	LOAD_DLLPTR(LP_Plot);
	LOAD_DLLPTR(LP_StartIQmeasureTimer);
	LOAD_DLLPTR(LP_StopIQmeasureTimer);
	LOAD_DLLPTR(LP_ReportTimerDurations);
	LOAD_DLLPTR(SetTesterHwVersion);
	LOAD_DLLPTR(GetTesterHwVersion);
	LOAD_DLLPTR(LP_SetDefault);

	//For MTK META
	LOAD_DLLPTR(LP_GetResultSpectralMask);
	LOAD_DLLPTR(LP_GetResultSpectralMask_HT40);
	LOAD_DLLPTR(LP_LoResultSpectralFlatness11AG);
	LOAD_DLLPTR(LP_LoResultSpectralFlatness11N);

	/*------*
	 *  FM	*
	 *-------*/
	LOAD_DLLPTR(LP_SetLpcPath);
	LOAD_DLLPTR(LP_FM_SetVsg);
	LOAD_DLLPTR(LP_FM_GetVsgSettings);
	LOAD_DLLPTR(LP_FM_SetAudioSingleTone);
	LOAD_DLLPTR(LP_FM_SetAudioToneArray);
	LOAD_DLLPTR(LP_FM_SetAudioToneArrayDeviation);
	LOAD_DLLPTR(LP_FM_GetAudioToneArray);
	LOAD_DLLPTR(LP_FM_DeleteAudioTones);
	LOAD_DLLPTR(LP_FM_StartVsg);
	LOAD_DLLPTR(LP_FM_StopVsg);
	LOAD_DLLPTR(LP_FM_SetVsgDistortion);
	LOAD_DLLPTR(LP_FM_GetVsgDistortion);
	LOAD_DLLPTR(LP_FM_StartVsgInterference);
	LOAD_DLLPTR(LP_FM_GetVsgInterferenceSettings);
	LOAD_DLLPTR(LP_FM_StopVsgInterference);
	LOAD_DLLPTR(LP_FM_SetVsa);
	LOAD_DLLPTR(LP_FM_GetVsaSettings);
	LOAD_DLLPTR(LP_FM_VsaDataCapture);
	LOAD_DLLPTR(LP_FM_Analyze_RF);
	LOAD_DLLPTR(LP_FM_Analyze_Demod);
	LOAD_DLLPTR(LP_FM_Analyze_Audio_Mono);
	LOAD_DLLPTR(LP_FM_Analyze_Audio_Stereo);
	LOAD_DLLPTR(LP_FM_GetScalarMeasurement);
	LOAD_DLLPTR(LP_FM_GetVectorMeasurement);
	LOAD_DLLPTR(LP_FM_AudioDataCapture);
	LOAD_DLLPTR(LP_FM_LoadAudioCapture);
	LOAD_DLLPTR(LP_FM_AudioStimulusGenerateAndPlayMultiTone);
	LOAD_DLLPTR(LP_FM_AudioStimulusGenerateAndPlaySingleTone);
	LOAD_DLLPTR(LP_FM_StopAudioPlay);
	LOAD_DLLPTR(LP_SaveVsaSignalFileText);
	LOAD_DLLPTR(LP_SaveTruncateCapture);
	LOAD_DLLPTR(LP_PwrCalFastGetElement);
	LOAD_DLLPTR(LP_FM_SetAudioToneModeAmplitude);





	if( g_IQtype ==0)
	{
		LOAD_DLLPTR(LP_SetVsg_Compensation);
		LOAD_DLLPTR(LP_SetAlcMode);
		LOAD_DLLPTR(LP_GetAlcMode);
		LOAD_DLLPTR(LP_SetVsgTriggerType);
		LOAD_DLLPTR(LP_GetHndlDataPointers);
		LOAD_DLLPTR(LP_SaveUserDataToSigFile);
		LOAD_DLLPTR(LP_GetCapture);
		LOAD_DLLPTR(LP_EnableMultiThread);
		LOAD_DLLPTR(LP_SetUserDataPtr);
		LOAD_DLLPTR(LP_SetDefaultNfc);
		LOAD_DLLPTR(LP_SetNfcTxAmplitude);
		LOAD_DLLPTR(LP_SetTriggerMaskNfc);
		LOAD_DLLPTR(LP_InitiateNfc);
		LOAD_DLLPTR(LP_SetRxNfc);
		LOAD_DLLPTR(LP_SetTxNfc);
		LOAD_DLLPTR(LP_ContCapture);
		LOAD_DLLPTR(LP_TerminateNfc);
		LOAD_DLLPTR(LP_AnalysisNfc);
		LOAD_DLLPTR(LP_SetVsgModulation_SetPlayCondition);
		LOAD_DLLPTR(LP_SetVsaBluetooth_BTShiftHz);
		LOAD_DLLPTR(LP_FM_SetVsa_Agc_On);





		/*--------------------*
		 *	IQ2010 Ext		  *
		 *---------------------*/
		LOAD_DLLPTR(LP_IQ2010EXT_NewMultiAnalysisCapture);
		LOAD_DLLPTR(LP_IQ2010EXT_AddMultiAnalysisCapture);
		LOAD_DLLPTR(LP_IQ2010EXT_InitiateMultiAnalysisCapture);
		LOAD_DLLPTR(LP_IQ2010EXT_FinishMultiAnalysisCapture);
		LOAD_DLLPTR(LP_IQ2010EXT_InitiateMultiCapture);
		LOAD_DLLPTR(LP_IQ2010EXT_InitiateMultiCaptureHT40);
		LOAD_DLLPTR(LP_IQ2010EXT_FinishMultiCapture);
		LOAD_DLLPTR(LP_IQ2010EXT_Analyze80211ag);
		LOAD_DLLPTR(LP_IQ2010EXT_Analyze80211nSISO);
		LOAD_DLLPTR(LP_IQ2010EXT_Analyze80211nComposite);
		LOAD_DLLPTR(LP_IQ2010EXT_Analyze80211b);
		LOAD_DLLPTR(LP_IQ2010EXT_AnalyzePower);
		LOAD_DLLPTR(LP_IQ2010EXT_AnalyzeFFT);
		LOAD_DLLPTR(LP_IQ2010EXT_AnalyzeMaskMeasurement);
		LOAD_DLLPTR(LP_IQ2010EXT_AnalyzePowerRamp80211b);
		LOAD_DLLPTR(LP_IQ2010EXT_AnalyzeBluetooth);
		LOAD_DLLPTR(LP_IQ2010EXT_AnalyzeSpectralMeasurement);
		LOAD_DLLPTR(LP_IQ2010EXT_RxPer);
		LOAD_DLLPTR(LP_IQ2010EXT_NewMultiSegmentWaveform);
		LOAD_DLLPTR(LP_IQ2010EXT_AddWaveform);
		LOAD_DLLPTR(LP_IQ2010EXT_FinalizeMultiSegmentWaveform);
		LOAD_DLLPTR(LP_IQ2010EXT_GetNumberOfMeasurementElements);
		LOAD_DLLPTR(LP_IQ2010EXT_GetNumberOfPerPowerLevels);
		LOAD_DLLPTR(LP_IQ2010EXT_GetDoubleMeasurements);
		LOAD_DLLPTR(LP_IQ2010EXT_GetStringMeasurement);
		LOAD_DLLPTR(LP_IQ2010EXT_GetVectorMeasurement);
		LOAD_DLLPTR(LP_IQ2010EXT_GetIntMeasurement);
		LOAD_DLLPTR(LP_IQ2010EXT_Init);
		LOAD_DLLPTR(LP_IQ2010EXT_Terminate);


		/*------------*
		 *	 GPS	  *
		 *-------------*/
		LOAD_DLLPTR(LP_GPS_SetActivate);
		LOAD_DLLPTR(LP_GPS_LoadScenarioFile);
		LOAD_DLLPTR(LP_GPS_PlayScenarioFile);
		LOAD_DLLPTR(LP_GPS_ModulatedMode);
		LOAD_DLLPTR(LP_GPS_ContinueWave);
		LOAD_DLLPTR(LP_GPS_SetDeactivate);
		LOAD_DLLPTR(LP_GPS_GetChannelInfo);
		LOAD_DLLPTR(LP_GPS_SetChannelInfo);
		LOAD_DLLPTR(LP_GPS_SetData);
		//LOAD_DLLPTR(LP_GPS_CaculateTesterPower);

		/*----------------------*
		 *	Glonass		*
		 *----------------------*/

		LOAD_DLLPTR(LP_PrintGlonassParameters);
		LOAD_DLLPTR(LP_Glonass_SetOperationMode);
		LOAD_DLLPTR(LP_Glonass_SetPower);
		LOAD_DLLPTR(LP_Glonass_SetFrequency);
		LOAD_DLLPTR(LP_Glonass_SetRfOutput);
		LOAD_DLLPTR(LP_Glonass_SetActivate);
		LOAD_DLLPTR(LP_Glonass_SetDeactivate);
	}
	else  //DTNA
	{
		LOAD_DLLPTR(LP_SetVsg_triggerType);

		LOAD_DLLPTR(LP_Analyze80211ac);
		LOAD_DLLPTR(LP_AnalyzeVHT80Mask);
		LOAD_DLLPTR(LP_FastCalGetPowerData);
		LOAD_DLLPTR(LP_PwrCalFreeElement);
		LOAD_DLLPTR(LP_ScpiCommandSet);
		LOAD_DLLPTR(LP_ScpiCommandQuery);
		LOAD_DLLPTR(LP_FastCalMeasPower);

	}
	printf("----------------------------------------------------------------------------------\n");

	alreadyLoaded = 1;

	return 0;

	//FunctionNotFoundError:
	//	FreeLibrary(DynamiclibraryHandle);
	//	DynamiclibraryHandle = 0;
	//	exit(0);
	//	return 1;
}

// --------------------------------------------------------------------------------
// Implement the functions
// --------------------------------------------------------------------------------

IQMEASURE_API double LP_GetScalarMeasurement_NoTimer(char *measurement, int index)
{
	if (loadDynamicLibrary())
		return 1;
	//printf("--> LP_GetScalarMeasurement_NoTimer()\n");
	return (*LP_GetScalarMeasurement_NoTimer_Ptr)(measurement,index);
}

IQMEASURE_API int LP_GetVectorMeasurement_NoTimer(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GetVectorMeasurement_NoTimer()\n");
	return (*LP_GetVectorMeasurement_NoTimer_Ptr)(measurement,bufferReal,bufferImag,bufferLength);
}

IQMEASURE_API int LP_GetStringMeasurement_NoTimer(char *measurement, char bufferChar[], int bufferLength)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GetStringMeasurement_NoTimer()\n");
	return (*LP_GetStringMeasurement_NoTimer_Ptr)(measurement,bufferChar,bufferLength);
}

IQMEASURE_API int  LP_Init(int IQtype,int testerControlMethod)
{
	if ( IQtype == IQTYPE_2010)
	{
		strcpy_s(dynamiclibraryName,sizeof(dynamiclibraryName),IQLegacy_Dll);
		g_IQtype = 0;
	}
	else
	{
		if( testerControlMethod == 0 )
			strcpy_s(dynamiclibraryName, sizeof(dynamiclibraryName), "IQmeasure_IQapi_SCPI.dll");
		else if ( testerControlMethod == 1 )
			strcpy_s(dynamiclibraryName, sizeof(dynamiclibraryName), "IQmeasure_SCPI.dll");

		//strcpy_s(dynamiclibraryName,sizeof(dynamiclibraryName), IQXel_Dll);
		g_IQtype=1;
	}

	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_Init()\n");
	return (*LP_Init_Ptr)(IQtype,testerControlMethod);
}

IQMEASURE_API int	LP_Term(void)
{
	int returnVal = 0;
	if (loadDynamicLibrary())
	{
		FreeLibrary(DynamiclibraryHandle);
		DynamiclibraryHandle = NULL;
		return 1;
	}
	//  printf("--> LP_Term()\n");
	returnVal = (*LP_Term_Ptr)();
	FreeLibrary(DynamiclibraryHandle);
	DynamiclibraryHandle = NULL;

	return returnVal;
}

IQMEASURE_API int	LP_GetIQapiHndl(void **getHndl, bool *testerInitialized)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GetIQapiHndl()\n");
	return (*LP_GetIQapiHndl_Ptr)(getHndl,testerInitialized);
}


IQMEASURE_API  bool	LP_GetVersion(char *buffer, int buf_size)
{
	if (loadDynamicLibrary())
		return false;		//Since this is a bool value it should return 0 for false not 1 for true.
	//  printf("--> LP_GetVersion()\n");
	return (*LP_GetVersion_Ptr)(buffer,buf_size);
}

IQMEASURE_API int	LP_InitTester(char *ipAddress)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_InitTester()\n");
	return (*LP_InitTester_Ptr)(ipAddress);
}

IQMEASURE_API int	LP_InitTester2(char *ipAddress1, char *ipAddress2)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_InitTester2()\n");
	return (*LP_InitTester2_Ptr)(ipAddress1,ipAddress2);
}

IQMEASURE_API int	LP_InitTester3(char *ipAddress1, char *ipAddress2, char *ipAddress3)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_InitTester3()\n");
	return (*LP_InitTester3_Ptr)(ipAddress1,ipAddress2,ipAddress3);
}

IQMEASURE_API int	LP_InitTester4(char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_InitTester4()\n");
	return (*LP_InitTester4_Ptr)(ipAddress1,ipAddress2,ipAddress3,ipAddress4);
}

IQMEASURE_API int LP_ConClose(void)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_ConClose()\n");
	return (*LP_ConClose_Ptr)();
}

IQMEASURE_API int	LP_ConOpen(void)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_ConOpen()\n");
	return (*LP_ConOpen_Ptr)();
}

IQMEASURE_API int LP_SetDefault(void)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetDefault()\n");
	return (*LP_SetDefault_Ptr)();
}

IQMEASURE_API int LP_ScpiCommandSet(char *scpiCommand)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetDefault()\n");
	return (*LP_ScpiCommandSet_Ptr)(scpiCommand);
}

IQMEASURE_API int LP_ScpiCommandQuery(char query[], unsigned int max_size, char response[], unsigned int *actual_size)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetDefault()\n");
	return (*LP_ScpiCommandQuery_Ptr)(query, max_size, response, actual_size);
}
IQMEASURE_API int	LP_DualHead_ConOpen(int tokenID, char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_DualHead_ConOpen()\n");
	return (*LP_DualHead_ConOpen_Ptr)(tokenID,ipAddress1,ipAddress2,ipAddress3,ipAddress4);
}

IQMEASURE_API char*	LP_GetErrorString(int err)
{
	if (loadDynamicLibrary())
		return "";
	//   printf("--> LP_GetErrorString()\n");
	return (*LP_GetErrorString_Ptr)(err);
}

IQMEASURE_API char*	LP_GetIQapiHndlLastErrMsg()
{
	if (loadDynamicLibrary())
		return "";
	//   printf("--> LP_GetIQapiHndlLastErrMsg()\n");
	return (*LP_GetIQapiHndlLastErrMsg_Ptr)();
}

IQMEASURE_API int	LP_DualHead_GetTokenID(int *tokenID)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_DualHead_GetTokenID()\n");
	return (*LP_DualHead_GetTokenID_Ptr)(tokenID);
}

IQMEASURE_API int	LP_DualHead_ConClose()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_DualHead_ConClose()\n");
	return (*LP_DualHead_ConClose_Ptr)();
}
IQMEASURE_API int	LP_DualHead_ObtainControl(unsigned int probeTimeMS, unsigned int timeOutMS)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_DualHead_ObtainControl()\n");
	return (*LP_DualHead_ObtainControl_Ptr)(probeTimeMS,timeOutMS);
}

IQMEASURE_API int	LP_DualHead_ReleaseControl()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_DualHead_ReleaseControl()\n");
	return (*LP_DualHead_ReleaseControl_Ptr)();
}

IQMEASURE_API int	LP_SetAlcMode(IQAPI_ALC_MODES alcMode)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SetAlcMode()\n");
	return (*LP_SetAlcMode_Ptr)(alcMode);
}

IQMEASURE_API int	LP_GetAlcMode(IQAPI_ALC_MODES* alcMode)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_GetAlcMode()\n");
	return (*LP_GetAlcMode_Ptr)(alcMode);
}

IQMEASURE_API int	LP_SetVsg(double rfFreqHz, double rfPowerLeveldBm, int port, bool setGapPowerOff, double dFreqShiftHz)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetVsg()\n");
	return (*LP_SetVsg_Ptr)(rfFreqHz, rfPowerLeveldBm, port, setGapPowerOff, dFreqShiftHz);
}

IQMEASURE_API int	LP_SetVsg_Compensation(double		  dcErrI,
		double		  dcErrQ,
		double		  phaseErr,
		double		  gainErr,
		double		  delayErr)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SetVsg_Compensation()\n");
	return (*LP_SetVsg_Compensation_Ptr)(dcErrI,dcErrQ,phaseErr,gainErr,delayErr);
}

IQMEASURE_API int	LP_SetVsgTriggerType(int trigger)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SetVsgTriggerType()\n");
	return (*LP_SetVsgTriggerType_Ptr)(trigger);
}

IQMEASURE_API int	LP_SetVsg_GapPower(double rfFreqHz, double rfPowerLeveldBm, int port, int gapPowerOff)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SetVsg_GapPower()\n");
	return (*LP_SetVsg_GapPower_Ptr)(rfFreqHz,rfPowerLeveldBm,port,gapPowerOff);
}

IQMEASURE_API int	LP_SetVsgNxN(double rfFreqHz, double rfPowerLeveldBm[], int port[], double dFreqShiftHz)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetVsgNxN()\n");
	return (*LP_SetVsgNxN_Ptr)(rfFreqHz,rfPowerLeveldBm,port, dFreqShiftHz);
}

IQMEASURE_API int	LP_SetVsgCw(double sineFreqHz, double offsetFrequencyMHz, double rfPowerLeveldBm, int port)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetVsgCw()\n");
	return (*LP_SetVsgCw_Ptr)(sineFreqHz,offsetFrequencyMHz,rfPowerLeveldBm,port);
}

IQMEASURE_API int	LP_SaveSignalArrays(double *real[N_MAX_TESTERS],
		double *imag[N_MAX_TESTERS],
		int length[N_MAX_TESTERS],
		double sampleFreqHz[N_MAX_TESTERS],
		char *fileNameToSave)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SaveSignalArrays()\n");
	return (*LP_SaveSignalArrays_Ptr)(real,imag,length,sampleFreqHz,fileNameToSave);
}

IQMEASURE_API int	LP_SaveIQDataToModulationFile(double *real,
		double *imag,
		int length[N_MAX_TESTERS],
		char *modFileName,
		int  normalization,
		int loadIQDataToVsg)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SaveIQDataToModulationFile()\n");
	return (*LP_SaveIQDataToModulationFile_Ptr)(real,imag,length,modFileName,normalization,loadIQDataToVsg);
}

IQMEASURE_API int	LP_SetVsgModulation(char *modFileName, int loadInternalWaveform)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SetVsgModulation()\n");
	return (*LP_SetVsgModulation_Ptr)(modFileName, loadInternalWaveform);
}

IQMEASURE_API int	LP_SetAnalysisParameterInteger(char *measurement, char *parameter, int value)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetAnalysisParameterInteger()\n");
	return (*LP_SetAnalysisParameterInteger_Ptr)(measurement,parameter,value);
}

IQMEASURE_API int	LP_SetAnalysisParameterIntegerArray(char *measurement, char *parameter, int *value, int valuesize)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SetAnalysisParameterIntegerArray()\n");
	return (*LP_SetAnalysisParameterIntegerArray_Ptr)(measurement,parameter,value,valuesize);
}

IQMEASURE_API int	LP_CopyVsaCaptureData(int fromVsaNum, int toVsaNum)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_CopyVsaCaptureData()\n");
	return (*LP_CopyVsaCaptureData_Ptr)(fromVsaNum,toVsaNum);
}

IQMEASURE_API int	LP_SaveVsaSignalFile(char *sigFileName)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SaveVsaSignalFile()\n");
	return (*LP_SaveVsaSignalFile_Ptr)(sigFileName);
}

IQMEASURE_API int	LP_LoadVsaSignalFile(char *sigFileName)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_LoadVsaSignalFile()\n");
	return (*LP_LoadVsaSignalFile_Ptr)(sigFileName);
}

IQMEASURE_API int	LP_SaveVsaGeneratorFile(char *modFileName)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SaveVsaGeneratorFile()\n");
	return (*LP_SaveVsaGeneratorFile_Ptr)(modFileName);
}

IQMEASURE_API int	LP_SetVsaBluetooth(double rfFreqHz, double rfAmplDb, int port, double triggerLevelDb, double triggerPreTime)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetVsaBluetooth()\n");
	return (*LP_SetVsaBluetooth_Ptr)(rfFreqHz,rfAmplDb,port,triggerLevelDb,triggerPreTime);
}

IQMEASURE_API int	LP_SetVsa(double rfFreqHz, double rfAmplDb, int port, double extAttenDb, double triggerLevelDb, double triggerPreTime, double dFreqShiftHz)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetVsa()\n");
	return (*LP_SetVsa_Ptr)(rfFreqHz,rfAmplDb,port,extAttenDb,triggerLevelDb,triggerPreTime, dFreqShiftHz);
}

IQMEASURE_API int	LP_SetVsaTriggerTimeout(double triggerTimeoutSec)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetVsaTriggerTimeout()\n");
	return (*LP_SetVsaTriggerTimeout_Ptr)(triggerTimeoutSec);
}

IQMEASURE_API int	LP_SetVsaNxN(double rfFreqHz, double rfAmplDb[], int port[], double extAttenDb, double triggerLevelDb, double triggerPreTime, double dFreqShiftHz)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SetVsaNxN()\n");
	return (*LP_SetVsaNxN_Ptr)(rfFreqHz,rfAmplDb,port,extAttenDb,triggerLevelDb,triggerPreTime, dFreqShiftHz);
}

IQMEASURE_API int	LP_SetVsaAmplitudeTolerance(double amplitudeToleranceDb)
{
	if (loadDynamicLibrary())
		return 1;
	//    printf("--> LP_SetVsaAmplitudeTolerance()\n");
	return (*LP_SetVsaAmplitudeTolerance_Ptr)(amplitudeToleranceDb);
}

IQMEASURE_API int	LP_Agc(double *rfAmplDb,bool allTesters)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_Agc()\n");
	return (*LP_Agc_Ptr)(rfAmplDb,allTesters);
}

IQMEASURE_API int	LP_SetFrameCnt(int frameCnt)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_SetFrameCnt()\n");
	return (*LP_SetFrameCnt_Ptr)(frameCnt);
}

IQMEASURE_API int	LP_TxDone(void)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_TxDone()\n");
	return (*LP_TxDone_Ptr)();
}

IQMEASURE_API int	LP_EnableVsgRF(int enabled)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_EnableVsgRF()\n");
	return (*LP_EnableVsgRF_Ptr)(enabled);
}

IQMEASURE_API int	LP_EnableVsgRFNxN(int vsg1Enabled, int vsg2Enabled, int vsg3Enabled, int vsg4Enabled)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_EnableVsgRFNxN()\n");
	return (*LP_EnableVsgRFNxN_Ptr)(vsg1Enabled,vsg2Enabled,vsg3Enabled,vsg4Enabled);
}

IQMEASURE_API int	LP_EnableSpecifiedVsgRF(int enabled, int vsgNumber)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_EnableSpecifiedVsgRF()\n");
	return (*LP_EnableSpecifiedVsgRF_Ptr)(enabled,vsgNumber);
}

IQMEASURE_API int	LP_EnableSpecifiedVsaRF(int enabled, int vsaNumber)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_EnableSpecifiedVsaRF()\n");
	return (*LP_EnableSpecifiedVsaRF_Ptr)(enabled,vsaNumber);
}

IQMEASURE_API int	LP_GetVsaSettings(double *freqHz, double *ampl, IQAPI_PORT_ENUM *port, int *rfEnabled, double *triggerLevel)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_GetVsaSettings()\n");
	return (*LP_GetVsaSettings_Ptr)(freqHz,ampl,port,rfEnabled,triggerLevel);
}

IQMEASURE_API int	LP_VsaDataCapture(double samplingTimeSecs, int triggerType, double sampleFreqHz,
		int ht40Mode,IQMEASURE_CAPTURE_NONBLOCKING_STATES nonBlockingState)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_VsaDataCapture()\n");
	return (*LP_VsaDataCapture_Ptr)(samplingTimeSecs,triggerType,sampleFreqHz,ht40Mode,nonBlockingState);
}

IQMEASURE_API int	LP_GetSampleData(int vsaNum, double bufferReal[], double bufferImag[], int bufferLength)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_GetSampleData()\n");
	return (*LP_GetSampleData_Ptr)(vsaNum,bufferReal,bufferImag,bufferLength);
}

IQMEASURE_API int	LP_GetHndlDataPointers(double *real[],double *imag[],int *length, double *sampleFreqHz, int arraySize)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_GetHndlDataPointers()\n");
	return (*LP_GetHndlDataPointers_Ptr)(real,imag,length,sampleFreqHz,arraySize);
}

IQMEASURE_API int	LP_SaveUserDataToSigFile(char* sigFileName,
		double *real[],
		double *imag[],
		int *length,
		double *sampleFreqHz,
		int arraySize)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SaveUserDataToSigFile()\n");
	return (*LP_SaveUserDataToSigFile_Ptr)(sigFileName,real,imag,length,sampleFreqHz,arraySize);
}

IQMEASURE_API int	LP_SelectCaptureRangeForAnalysis(double startPositionUs, double lengthUs, int packetsOffset, int packetsLength)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SelectCaptureRangeForAnalysis()\n");
	return (*LP_SelectCaptureRangeForAnalysis_Ptr)(startPositionUs,lengthUs, packetsOffset, packetsLength);
}

IQMEASURE_API int	LP_Analyze80216d( double sigType	,
		double bandwidthHz	,
		double cyclicPrefix	,
		double rateId		,
		double numSymbols	,
		int ph_corr			,
		int ch_corr			,
		int freq_corr		,
		int timing_corr		,
		int ampl_track		,
		double decode
		)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_Analyze80216d()\n");
	return (*LP_Analyze80216d_Ptr)(sigType,bandwidthHz,cyclicPrefix,rateId,numSymbols,ph_corr,ch_corr,freq_corr,timing_corr,ampl_track,decode);
}

IQMEASURE_API int	LP_Analyze80216e( double sigType	,
		double bandwidthHz	,
		double cyclicPrefix	,
		double rateId		,
		double numSymbols	,
		int ph_corr			,
		int ch_corr			,
		int freq_corr		,
		int timing_corr		,
		int ampl_track		,
		double decode	,
		char *map_conf_file
		)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_Analyze80216e()\n");
	return (*LP_Analyze80216e_Ptr)(sigType,bandwidthHz,cyclicPrefix,rateId,numSymbols,ph_corr,ch_corr,freq_corr,timing_corr,ampl_track,decode,map_conf_file);
}

IQMEASURE_API int	LP_Analyze80211p(int ph_corr_mode, int ch_estimate, int sym_tim_corr, int freq_sync, int ampl_track, int ofdm_mode)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_Analyze80211p()\n");
	return (*LP_Analyze80211p_Ptr)(ph_corr_mode,ch_estimate,sym_tim_corr,freq_sync,ampl_track,ofdm_mode);
}

IQMEASURE_API int	LP_Analyze80211ag(int ph_corr_mode, int ch_estimate, int sym_tim_corr, int freq_sync, int ampl_track,double prePowStartSec,
		double prePowStopSec)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_Analyze80211ag()\n");
	return (*LP_Analyze80211ag_Ptr)(ph_corr_mode,ch_estimate,sym_tim_corr,freq_sync,ampl_track,prePowStartSec,prePowStopSec);
}

IQMEASURE_API int	LP_Analyze80211n(char *type,
		char *mode,
		int enablePhaseCorr ,
		int enableSymTimingCorr ,
		int enableAmplitudeTracking ,
		int decodePSDU ,
		int enableFullPacketChannelEst ,
		char *referenceFile,
		int packetFormat,
		int frequencyCorr,
		double prePowStartSec,
		double prePowStopSec)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_Analyze80211n()\n");
	return (*LP_Analyze80211n_Ptr)(type,mode,enablePhaseCorr,enableSymTimingCorr,
			enableAmplitudeTracking,decodePSDU,enableFullPacketChannelEst,referenceFile,packetFormat,frequencyCorr,prePowStartSec,prePowStopSec);
}

IQMEASURE_API int	LP_AnalyzeMimo(char *type, char *mode, int enablePhaseCorr, int enableSymTimingCorr, int enableAmplitudeTracking,
		int decodePSDU, int enableFullPacketChannelEst, char *referenceFile)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_AnalyzeMimo()\n");
	return (*LP_AnalyzeMimo_Ptr)(type,mode,enablePhaseCorr,enableSymTimingCorr,enableAmplitudeTracking,
			decodePSDU,enableFullPacketChannelEst,referenceFile);
}

IQMEASURE_API int	LP_Analyze80211b(int eq_taps,
		int DCremove11b_flag ,
		int method_11b,
		double prePowStartSec,
		double prePowStopSec)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_Analyze80211b()\n");
	return (*LP_Analyze80211b_Ptr)(eq_taps,DCremove11b_flag,method_11b,prePowStartSec,
			prePowStopSec);
}

IQMEASURE_API int	LP_AnalyzePower(double T_interval, double max_pow_diff_dB)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_AnalyzePower()\n");
	return (*LP_AnalyzePower_Ptr)(T_interval,max_pow_diff_dB);
}

IQMEASURE_API int	LP_AnalyzeFFT(double NFFT, double res_bw, char *window_type)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_AnalyzeFFT()\n");
	return (*LP_AnalyzeFFT_Ptr)(NFFT,res_bw,window_type);
}

IQMEASURE_API int	LP_AnalyzeCCDF()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_AnalyzeCCDF()\n");
	return (*LP_AnalyzeCCDF_Ptr)();
}

IQMEASURE_API int	LP_AnalyzeCW()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_AnalyzeCW()\n");
	return (*LP_AnalyzeCW_Ptr)();
}

IQMEASURE_API int	LP_AnalyzeCWFreq()
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_AnalyzeCWFreq()\n");
	return (*LP_AnalyzeCWFreq_Ptr)();
}

IQMEASURE_API int	LP_AnalysisWave()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_AnalysisWave()\n");
	return (*LP_AnalysisWave_Ptr)();
}

IQMEASURE_API int	LP_AnalyzeSidelobe()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_AnalyzeSidelobe()\n");
	return (*LP_AnalyzeSidelobe_Ptr)();
}

IQMEASURE_API int	LP_AnalyzePowerRampOFDM()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_AnalyzePowerRampOFDM()\n");
	return (*LP_AnalyzePowerRampOFDM_Ptr)();
}

IQMEASURE_API int	LP_AnalyzePowerRamp80211b()
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_AnalyzePowerRamp80211b()\n");
	return (*LP_AnalyzePowerRamp80211b_Ptr)();
}

IQMEASURE_API int	LP_AnalyzeBluetooth(double data_rate, char *analysis_type )
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_AnalyzeBluetooth()\n");
	return (*LP_AnalyzeBluetooth_Ptr)(data_rate,analysis_type);
}

IQMEASURE_API int	LP_AnalyzeZigbee()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_AnalyzeZigbee()\n");
	return (*LP_AnalyzeZigbee_Ptr)();
}

IQMEASURE_API int	LP_AnalyzeHT40Mask()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_AnalyzeHT40Mask()\n");
	return (*LP_AnalyzeHT40Mask_Ptr)();
}

IQMEASURE_API double	LP_GetScalarMeasurment(char *measurement, int index)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_GetScalarMeasurment()\n");
	return (*LP_GetScalarMeasurment_Ptr)(measurement,index);
}

IQMEASURE_API int	LP_GetVectorMeasurment(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_GetVectorMeasurment()\n");
	return (*LP_GetVectorMeasurment_Ptr)(measurement,bufferReal,bufferImag,bufferLength);
}

IQMEASURE_API int	LP_GetStringMeasurment(char *measurement, char bufferChar[], int bufferLength)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_GetStringMeasurment()\n");
	return (*LP_GetStringMeasurment_Ptr)(measurement,bufferChar,bufferLength);
}

IQMEASURE_API double	LP_GetScalarMeasurement(char *measurement, int index)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GetScalarMeasurement()\n");
	return (*LP_GetScalarMeasurement_Ptr)(measurement,index);
}

IQMEASURE_API int	LP_GetVectorMeasurement(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_GetVectorMeasurement()\n");
	return (*LP_GetVectorMeasurement_Ptr)(measurement,bufferReal,bufferImag,bufferLength);
}

IQMEASURE_API int	LP_GetStringMeasurement(char *measurement, char bufferChar[], int bufferLength)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_GetStringMeasurement()\n");
	return (*LP_GetStringMeasurement_Ptr)(measurement,bufferChar,bufferLength);
}

IQMEASURE_API int	LP_PlotDataCapture()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_PlotDataCapture()\n");
	return (*LP_PlotDataCapture_Ptr)();
}

IQMEASURE_API int	LP_Plot(int figNum, double *x, double *y, int length, char *plotArgs, char *title, char *xtitle, char *ytitle, int keepPlot)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_Plot()\n");
	return (*LP_Plot_Ptr)(figNum,x,y,length,plotArgs,title,xtitle,ytitle,keepPlot);
}

IQMEASURE_API int	LP_StartIQmeasureTimer()
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_StartIQmeasureTimer()\n");
	return (*LP_StartIQmeasureTimer_Ptr)();
}

IQMEASURE_API int	LP_StopIQmeasureTimer()
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_StopIQmeasureTimer()\n");
	return (*LP_StopIQmeasureTimer_Ptr)();
}

IQMEASURE_API int	LP_ReportTimerDurations()
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_ReportTimerDurations()\n");
	return (*LP_ReportTimerDurations_Ptr)();
}


/*-------------------------*
 *	IQ2010 Ext Functions   *
 *--------------------------*/

IQMEASURE_API int	LP_IQ2010EXT_NewMultiAnalysisCapture( char		   *dataRate,
		double          freqMHz,
		double	        rmsPowerLeveldBm,
		int	            skipPktCnt,
		int		        rfPort,
		double	        triggerLeveldBm)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_NewMultiAnalysisCapture()\n");
	return (*LP_IQ2010EXT_NewMultiAnalysisCapture_Ptr)(dataRate,freqMHz,rmsPowerLeveldBm,skipPktCnt,rfPort,triggerLeveldBm);
}

IQMEASURE_API int	LP_IQ2010EXT_AddMultiAnalysisCapture( int		    analysisType,
		int			captureLengthUs,
		int			captureCnt)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_AddMultiAnalysisCapture()\n");
	return (*LP_IQ2010EXT_AddMultiAnalysisCapture_Ptr)(analysisType,captureLengthUs,captureCnt);
}

IQMEASURE_API int	LP_IQ2010EXT_InitiateMultiAnalysisCapture(void)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_InitiateMultiAnalysisCapture()\n");
	return (*LP_IQ2010EXT_InitiateMultiAnalysisCapture_Ptr)();
}

IQMEASURE_API int	LP_IQ2010EXT_FinishMultiAnalysisCapture(void)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_IQ2010EXT_FinishMultiAnalysisCapture()\n");
	return (*LP_IQ2010EXT_FinishMultiAnalysisCapture_Ptr)();
}

IQMEASURE_API int	LP_IQ2010EXT_InitiateMultiCapture( char				*dataRate,
		double              freqMHz,
		double	             rmsPowerLeveldBm,
		int	             captureLengthUs,
		int	             skipPktCnt,
		int	             captureCnt,
		int				 rfPort,
		double	             triggerLeveldBm)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_InitiateMultiCapture()\n");
	return (*LP_IQ2010EXT_InitiateMultiCapture_Ptr)(dataRate,freqMHz,rmsPowerLeveldBm,captureLengthUs,skipPktCnt,captureCnt,rfPort,triggerLeveldBm);
}

IQMEASURE_API int	LP_IQ2010EXT_InitiateMultiCaptureHT40( char			   *dataRate,
		double             freqMHz,
		double	            rmsPowerLeveldBm,
		int	            captureLengthUs,
		int	            skipPktCnt,
		int	            captureCnt,
		int                rfPort,
		double	            triggerLeveldBm)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_IQ2010EXT_InitiateMultiCaptureHT40()\n");
	return (*LP_IQ2010EXT_InitiateMultiCaptureHT40_Ptr)(dataRate,freqMHz,rmsPowerLeveldBm,captureLengthUs,skipPktCnt,captureCnt,rfPort,triggerLeveldBm);
}


IQMEASURE_API int	LP_IQ2010EXT_FinishMultiCapture(void)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_IQ2010EXT_FinishMultiCapture()\n");
	return (*LP_IQ2010EXT_FinishMultiCapture_Ptr)();
}

IQMEASURE_API int	LP_IQ2010EXT_Analyze80211ag( IQV_PH_CORR_ENUM ph_corr_mode ,
		IQV_CH_EST_ENUM ch_estimate,
		IQV_SYM_TIM_ENUM sym_tim_corr,
		IQV_FREQ_SYNC_ENUM freq_sync,
		IQV_AMPL_TRACK_ENUM ampl_track)
{
	if (loadDynamicLibrary())
		return 1;
	//printf("--> LP_IQ2010EXT_Analyze80211ag()\n");
	return (*LP_IQ2010EXT_Analyze80211ag_Ptr)(ph_corr_mode,ch_estimate,sym_tim_corr,freq_sync,ampl_track);
}

IQMEASURE_API int	LP_IQ2010EXT_Analyze80211nSISO( int enablePhaseCorr ,
		int enableSymTimingCorr,
		int enableAmplitudeTracking ,
		int decodePSDU,
		int enableFullPacketChannelEst,
		int packetFormat)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_Analyze80211nSISO()\n");
	return (*LP_IQ2010EXT_Analyze80211nSISO_Ptr)(enablePhaseCorr,enableSymTimingCorr,enableAmplitudeTracking,decodePSDU,enableFullPacketChannelEst,packetFormat);
}

IQMEASURE_API int	LP_IQ2010EXT_Analyze80211nComposite(int enablePhaseCorr,
		int enableSymTimingCorr,
		int enableAmplitudeTracking,
		int enableFullPacketChannelEst,
		char *referenceFileL,
		int packetFormat )
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_Analyze80211nComposite()\n");
	return (*LP_IQ2010EXT_Analyze80211nComposite_Ptr)(enablePhaseCorr,enableSymTimingCorr,enableAmplitudeTracking,
			enableFullPacketChannelEst,referenceFileL,packetFormat);
}

IQMEASURE_API int	LP_IQ2010EXT_Analyze80211b( IQV_EQ_ENUM eq_taps,
		IQV_DC_REMOVAL_ENUM DCremove11b_flag ,
		IQV_11B_METHOD_ENUM method_11b)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_Analyze80211b()\n");
	return (*LP_IQ2010EXT_Analyze80211b_Ptr)(eq_taps,DCremove11b_flag,method_11b);
}

IQMEASURE_API int	LP_IQ2010EXT_AnalyzePower(void)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_AnalyzePower()\n");
	return (*LP_IQ2010EXT_AnalyzePower_Ptr)();
}

IQMEASURE_API int	LP_IQ2010EXT_AnalyzeFFT( double NFFT ,
		double res_bwHz,
		char *window_type)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_IQ2010EXT_AnalyzeFFT()\n");
	return (*LP_IQ2010EXT_AnalyzeFFT_Ptr)(NFFT,res_bwHz,window_type);
}


IQMEASURE_API int	LP_IQ2010EXT_AnalyzeMaskMeasurement(int maskType, double obwPercentage)
{
	if (loadDynamicLibrary())
		return 1;
	//printf("--> LP_IQ2010EXT_AnalyzeMaskMeasurement()\n");
	return (*LP_IQ2010EXT_AnalyzeMaskMeasurement_Ptr)(maskType,obwPercentage);
}

IQMEASURE_API int	LP_IQ2010EXT_AnalyzePowerRamp80211b()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_IQ2010EXT_AnalyzePowerRamp80211b()\n");
	return (*LP_IQ2010EXT_AnalyzePowerRamp80211b_Ptr)();
}

IQMEASURE_API int	LP_IQ2010EXT_AnalyzeBluetooth(double data_rate, char *analysis_type)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_AnalyzeBluetooth()\n");
	return (*LP_IQ2010EXT_AnalyzeBluetooth_Ptr)(data_rate,analysis_type);
}

IQMEASURE_API int	LP_IQ2010EXT_AnalyzeSpectralMeasurement(int spectralType)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_IQ2010EXT_AnalyzeSpectralMeasurement()\n");
	return (*LP_IQ2010EXT_AnalyzeSpectralMeasurement_Ptr)(spectralType);
}

IQMEASURE_API int	LP_IQ2010EXT_RxPer( int               waveformIndex,
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
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_RxPer()\n");
	return (*LP_IQ2010EXT_RxPer_Ptr)(waveformIndex,freqMHz,powerLevelStartdBm,powerLevelStopdBm,
			stepdB,packetCount,rfPort,ackPowerRmsdBm,ackTriggerLeveldBm);
}

IQMEASURE_API int	LP_IQ2010EXT_NewMultiSegmentWaveform()
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_NewMultiSegmentWaveform()\n");
	return (*LP_IQ2010EXT_NewMultiSegmentWaveform_Ptr)();
}

IQMEASURE_API int	LP_IQ2010EXT_AddWaveform(const char *modFile, unsigned int *waveformIndex )
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_AddWaveform()\n");
	return (*LP_IQ2010EXT_AddWaveform_Ptr)(modFile,waveformIndex);
}

IQMEASURE_API int	LP_IQ2010EXT_FinalizeMultiSegmentWaveform()
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_FinalizeMultiSegmentWaveform()\n");
	return (*LP_IQ2010EXT_FinalizeMultiSegmentWaveform_Ptr)();
}



IQMEASURE_API int	LP_IQ2010EXT_GetNumberOfMeasurementElements(char *measurementName, int indexResult, int *numberOfMeasurementElement)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_IQ2010EXT_GetNumberOfMeasurementElements()\n");
	return (*LP_IQ2010EXT_GetNumberOfMeasurementElements_Ptr)(measurementName,indexResult,numberOfMeasurementElement);
}

IQMEASURE_API int	LP_IQ2010EXT_GetNumberOfPerPowerLevels(char *measurementName, int *numberOfPerPowerLevel)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_IQ2010EXT_GetNumberOfPerPowerLevels()\n");
	return (*LP_IQ2010EXT_GetNumberOfPerPowerLevels_Ptr)(measurementName,numberOfPerPowerLevel);
}

IQMEASURE_API int	LP_IQ2010EXT_GetDoubleMeasurements(char *measurementName, double *average, double *minimum, double *maximum)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_IQ2010EXT_GetDoubleMeasurements()\n");
	return (*LP_IQ2010EXT_GetDoubleMeasurements_Ptr)(measurementName,average,minimum,maximum);
}

IQMEASURE_API int	LP_IQ2010EXT_GetStringMeasurement(char *measurementName, char *result, int  bufferSize, int indexResult)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_IQ2010EXT_GetStringMeasurement()\n");
	return (*LP_IQ2010EXT_GetStringMeasurement_Ptr)(measurementName,result,bufferSize,indexResult);
}

IQMEASURE_API int	LP_IQ2010EXT_GetVectorMeasurement(char *measurementName, double values[], int bufferLength, int *dataSize, int indexResult)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_IQ2010EXT_GetVectorMeasurement()\n");
	return (*LP_IQ2010EXT_GetVectorMeasurement_Ptr)(measurementName,values,bufferLength,dataSize,indexResult);
}

IQMEASURE_API int	LP_IQ2010EXT_GetIntMeasurement(char *measurementName, int *result, int indexResult)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_IQ2010EXT_GetIntMeasurement()\n");
	return (*LP_IQ2010EXT_GetIntMeasurement_Ptr)(measurementName,result,indexResult);
}

IQMEASURE_API int	LP_IQ2010EXT_Init(void)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_IQ2010EXT_Init()\n");
	return (*LP_IQ2010EXT_Init_Ptr)();
}

IQMEASURE_API int	LP_IQ2010EXT_Terminate(void)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_IQ2010EXT_Terminate()\n");
	return (*LP_IQ2010EXT_Terminate_Ptr)();
}

/*------------------*
 *	GPS Functions	*
 *-------------------*/

IQMEASURE_API int	LP_GPS_SetActivate()
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GPS_SetActivate()\n");
	return (*LP_GPS_SetActivate_Ptr)();
}

IQMEASURE_API int	LP_GPS_LoadScenarioFile(char* fileName, IQV_GPS_TRIGGER_STATE triggerState)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_GPS_LoadScenarioFile()\n");
	return (*LP_GPS_LoadScenarioFile_Ptr)(fileName,triggerState);
}

IQMEASURE_API int	LP_GPS_PlayScenarioFile(double powerDbm, double pathlossDb)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GPS_PlayScenarioFile()\n");
	return (*LP_GPS_PlayScenarioFile_Ptr)(powerDbm,pathlossDb);
}

IQMEASURE_API int	LP_GPS_ModulatedMode(int Nav_Mode, double pathlossDb, double powerA[6], int satelliteNumA[6], int dopplerFrequency, IQV_GPS_TRIGGER_STATE triggerState)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_GPS_ModulatedMode()\n");
	return (*LP_GPS_ModulatedMode_Ptr)(Nav_Mode,pathlossDb,powerA,satelliteNumA,dopplerFrequency,triggerState);
}

IQMEASURE_API int	LP_GPS_ContinueWave(double powerDbm, double pathlossDb, IQV_GPS_TRIGGER_STATE triggerState, int freqOffset)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GPS_ContinueWave()\n");
	return (*LP_GPS_ContinueWave_Ptr)(powerDbm,pathlossDb,triggerState,freqOffset);

}

IQMEASURE_API int	LP_GPS_SetDeactivate()
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GPS_SetDeactivate()\n");
	return (*LP_GPS_SetDeactivate_Ptr)();
}

IQMEASURE_API int	LP_GPS_GetChannelInfo(IQV_GPS_CHANNEL_NUMBER channelNumber,
		int *satelliteNumber,
		double *powerDbm,
		IQV_GPS_MODULATION_STATE *modulationState)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_GPS_GetChannelInfo()\n");
	return (*LP_GPS_GetChannelInfo_Ptr)(channelNumber,satelliteNumber,powerDbm,modulationState);
}

IQMEASURE_API int	LP_GPS_SetChannelInfo(IQV_GPS_CHANNEL_NUMBER channelNumber,
		int satelliteNumber,
		double powerDbm,
		IQV_GPS_MODULATION_STATE modulationState)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GPS_SetChannelInfo()\n");
	return (*LP_GPS_SetChannelInfo_Ptr)(channelNumber,satelliteNumber,powerDbm,modulationState);
}

IQMEASURE_API int	LP_GPS_SetData(const char* inputName, const char* inputValue)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GPS_SetData()\n");
	return (*LP_GPS_SetData_Ptr)(inputName,inputValue);
}

IQMEASURE_API double	LP_GPS_CaculateTesterPower(double powerDbm, double pathlossDb)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GPS_CaculateTesterPower()\n");
	return (*LP_GPS_CaculateTesterPower_Ptr)(powerDbm,pathlossDb);
}

/*------------------------------------------------*
 *		Glonass Functions		  *
 *------------------------------------------------*/

IQMEASURE_API int LP_PrintGlonassParameters(bool forceUpdate)
{
	if (loadDynamicLibrary())
		return 1;
	return (*LP_PrintGlonassParameters_Ptr)(forceUpdate);
}

IQMEASURE_API int LP_Glonass_SetOperationMode(IQV_GPS_OPERATION_MODE operationMode)
{
	if (loadDynamicLibrary())
		return 1;
	return (*LP_Glonass_SetOperationMode_Ptr)(operationMode);
}

IQMEASURE_API int LP_Glonass_SetPower(double powerdBm)
{
	if (loadDynamicLibrary())
		return 1;
	return (*LP_Glonass_SetPower_Ptr)(powerdBm);
}

IQMEASURE_API int LP_Glonass_SetFrequency(int frequencyChannel)
{
	if (loadDynamicLibrary())
		return 1;
	return (*LP_Glonass_SetFrequency_Ptr)(frequencyChannel);
}

IQMEASURE_API int LP_Glonass_SetRfOutput(IQV_GPS_RF_OUTPUT rfOutput)
{
	if (loadDynamicLibrary())
		return 1;
	return (*LP_Glonass_SetRfOutput_Ptr)(rfOutput);
}

IQMEASURE_API int LP_Glonass_SetActivate()
{
	if (loadDynamicLibrary())
		return 1;
	return (*LP_Glonass_SetActivate_Ptr)();
}

IQMEASURE_API int LP_Glonass_SetDeactivate()
{
	if (loadDynamicLibrary())
		return 1;
	return (*LP_Glonass_SetDeactivate_Ptr)();
}

/*----------------------------*
 *		FM Functions		  *
 *-----------------------------*/

IQMEASURE_API int	LP_SetLpcPath(char *litePointConnectionPath)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetLpcPath()\n");
	return (*LP_SetLpcPath_Ptr)(litePointConnectionPath);
}

IQMEASURE_API int	LP_FM_SetVsg(unsigned int carrierFreqHz,
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
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_SetVsg()\n");
	return (*LP_FM_SetVsg_Ptr)(carrierFreqHz,carrierPowerdBm,modulationEnable,totalFmDeviationHz,stereoEnable,
			pilotDeviationHz,rdsEnable,rdsDeviationHz,preEmphasisUs,rdsTransmitString);
}

IQMEASURE_API int	LP_FM_GetVsgSettings(unsigned int *carrierFreqHz,
		double *carrierPowerdBm,
		int *modulationEnabled,
		unsigned int *totalFmDeviationHz,
		int *stereoEnabled,
		unsigned int *pilotDeviationHz,
		int *rdsEnabled,
		unsigned int *rdsDeviationHz,
		unsigned int *preEmphasisMode
		)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_GetVsgSettings()\n");
	return (*LP_FM_GetVsgSettings_Ptr)(carrierFreqHz,carrierPowerdBm,modulationEnabled,
			totalFmDeviationHz,stereoEnabled,pilotDeviationHz,rdsEnabled,rdsDeviationHz,preEmphasisMode);
}

IQMEASURE_API int	LP_FM_SetAudioSingleTone(double audioToneFreqHz,
		int leftRightChannelSelect,
		double audioToneAmpPercent,
		int stereo)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_FM_SetAudioSingleTone()\n");
	return (*LP_FM_SetAudioSingleTone_Ptr)(audioToneFreqHz,leftRightChannelSelect,audioToneAmpPercent,stereo);
}

IQMEASURE_API int	LP_FM_SetAudioToneArray(double* freqHz,
		int* channelSelect,
		double* amplitudePercent,
		int stereo,
		unsigned int toneCount)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_FM_SetAudioToneArray()\n");
	return (*LP_FM_SetAudioToneArray_Ptr)(freqHz,channelSelect,amplitudePercent,stereo,toneCount);
}

IQMEASURE_API int	LP_FM_SetAudioToneArrayDeviation(double* freqHz,
		int* channelSelect,
		double* amplitudeDeviationHz,
		int stereo,
		unsigned int toneCount)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_SetAudioToneArrayDeviation()\n");
	return (*LP_FM_SetAudioToneArrayDeviation_Ptr)(freqHz,channelSelect,amplitudeDeviationHz,stereo,toneCount);
}

IQMEASURE_API int	LP_FM_GetAudioToneArray(double *freqHz,
		int *channelSelect,
		double *amplitudePercent,
		unsigned int toneCount)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_FM_GetAudioToneArray()\n");
	return (*LP_FM_GetAudioToneArray_Ptr)(freqHz,channelSelect,amplitudePercent,toneCount);
}

IQMEASURE_API int	LP_FM_DeleteAudioTones()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_DeleteAudioTones()\n");
	return (*LP_FM_DeleteAudioTones_Ptr)();
}

IQMEASURE_API int	LP_FM_StartVsg()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_StartVsg()\n");
	return (*LP_FM_StartVsg_Ptr)();
}

IQMEASURE_API int	LP_FM_StopVsg()
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_FM_StopVsg()\n");
	return (*LP_FM_StopVsg_Ptr)();
}

IQMEASURE_API int	LP_FM_SetVsgDistortion(int amDistortionEnable,
		unsigned int amFrequencyHz,
		unsigned int amDepthPercent)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_SetVsgDistortion()\n");
	return (*LP_FM_SetVsgDistortion_Ptr)(amDistortionEnable,amFrequencyHz,amDepthPercent);
}

IQMEASURE_API int	LP_FM_GetVsgDistortion(double *amDistortionEnableStatus,
		unsigned int *amFrequencyHz,
		unsigned int *amDepthPercent)
{
	if (loadDynamicLibrary())
		return 1;
	//    printf("--> LP_FM_GetVsgDistortion()\n");
	return (*LP_FM_GetVsgDistortion_Ptr)(amDistortionEnableStatus,amFrequencyHz,amDepthPercent);
}

IQMEASURE_API int	LP_FM_StartVsgInterference(double interfererRelativePowerdBm,
		int interfererCarrierOffsetHz,
		int interfererPeakFreqDeviationHz,
		int interfererModulationEnable,
		unsigned int interfererAudioFreqHz)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_FM_StartVsgInterference()\n");
	return (*LP_FM_StartVsgInterference_Ptr)(interfererRelativePowerdBm,interfererCarrierOffsetHz,
			interfererPeakFreqDeviationHz,interfererModulationEnable,interfererAudioFreqHz);
}

IQMEASURE_API int	LP_FM_GetVsgInterferenceSettings(double *interfererRelativePowerdBm,
		int *interfererCarrierOffsetHz,
		int *interfererPeakFreqDeviationHz,
		int *interfererModulationEnabled,
		unsigned int *interfererAudioFreqHz)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_FM_GetVsgInterferenceSettings()\n");
	return (*LP_FM_GetVsgInterferenceSettings)(interfererRelativePowerdBm,interfererCarrierOffsetHz,interfererPeakFreqDeviationHz,
			interfererModulationEnabled,interfererAudioFreqHz);
}

IQMEASURE_API int	LP_FM_StopVsgInterference()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_StopVsgInterference()\n");
	return (*LP_FM_StopVsgInterference_Ptr)();
}

IQMEASURE_API int	LP_FM_SetVsa(unsigned int carrierFreqHz,
		double expectedPeakInputPowerdBm)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_FM_SetVsa()\n");
	return (*LP_FM_SetVsa_Ptr)(carrierFreqHz,expectedPeakInputPowerdBm);
}

IQMEASURE_API int	LP_FM_GetVsaSettings(unsigned int *carrierFreqHz,
		double *expectedPeakInputPowerdBm)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_GetVsaSettings()\n");
	return (*LP_FM_GetVsaSettings_Ptr)(carrierFreqHz,expectedPeakInputPowerdBm);
}

IQMEASURE_API int	LP_FM_VsaDataCapture(double captureTimemillisec)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_FM_VsaDataCapture()\n");
	return (*LP_FM_VsaDataCapture_Ptr)(captureTimemillisec);
}

IQMEASURE_API int	LP_FM_Analyze_RF(int rfRBWHz,
		int rfOBWPercent,
		int rfPowerMeasBWHz,
		int windowType)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_FM_Analyze_RF()\n");
	return (*LP_FM_Analyze_RF_Ptr)(rfRBWHz,rfOBWPercent,rfPowerMeasBWHz,windowType);
}

IQMEASURE_API int	LP_FM_Analyze_Demod(int audioRBWHz)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_FM_Analyze_Demod()\n");
	return (*LP_FM_Analyze_Demod_Ptr)(audioRBWHz);
}

IQMEASURE_API int	LP_FM_Analyze_Audio_Mono(int audioRBWHz,
		int audioFreqLowLimitHz,
		int audioFreqHiLimitHz,
		int filterCount,
		int filterType1,
		double filterParam1,
		int filterType2,
		double filterParam2,
		int filterType3 ,
		double filterParam3 )
{
	if (loadDynamicLibrary())
		return 1;
	//    printf("--> LP_FM_Analyze_Audio_Mono()\n");
	return (*LP_FM_Analyze_Audio_Mono_Ptr)(audioRBWHz,audioFreqLowLimitHz,audioFreqHiLimitHz,filterCount,
			filterType1,filterParam1,filterType2,filterParam2,filterType3,filterParam3);
}

IQMEASURE_API int	LP_FM_Analyze_Audio_Stereo(int audioRBWHz,
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
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_FM_Analyze_Audio_Stereo()\n");
	return (*LP_FM_Analyze_Audio_Stereo_Ptr)(audioRBWHz,audioFreqLowLimitHz,audioFreqHiLimitHz,filterCount,
			filterType1,filterParam1,filterType2,filterParam2,filterType3,filterParam3);
}

IQMEASURE_API double	LP_FM_GetScalarMeasurement(char *measurement, int index)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_FM_GetScalarMeasurement()\n");
	return (*LP_FM_GetScalarMeasurement_Ptr)(measurement,index);
}

IQMEASURE_API int	LP_FM_GetVectorMeasurement(char *measurement, double bufferReal[], double bufferImag[], int bufferLength)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_GetVectorMeasurement()\n");
	return (*LP_FM_GetVectorMeasurement_Ptr)(measurement,bufferReal,bufferImag,bufferLength);
}

IQMEASURE_API int	LP_FM_AudioDataCapture(char* saveFileLocation,
		double captureDurationMilliSec,
		int samplingRate,
		int stereo)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_FM_AudioDataCapture()\n");
	return (*LP_FM_AudioDataCapture_Ptr)(saveFileLocation,captureDurationMilliSec,samplingRate,stereo);
}

IQMEASURE_API int	LP_FM_LoadAudioCapture(char* fileName)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_FM_LoadAudioCapture()\n");
	return (*LP_FM_LoadAudioCapture_Ptr)(fileName);
}

IQMEASURE_API int	LP_FM_AudioStimulusGenerateAndPlayMultiTone(double sampleRateHz,
		double deviationPercent,
		double peakVoltageLevelVolts,
		int toneCount,
		int stereoEnable,
		int* leftRightRelation,
		double* freqHz ,
		double durationMilliSeconds,
		char* audioWaveFileOutput)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_AudioStimulusGenerateAndPlayMultiTone()\n");
	return (*LP_FM_AudioStimulusGenerateAndPlayMultiTone_Ptr)(sampleRateHz,deviationPercent,peakVoltageLevelVolts,
			toneCount,stereoEnable,leftRightRelation,freqHz,durationMilliSeconds,audioWaveFileOutput);
}

IQMEASURE_API int	LP_FM_AudioStimulusGenerateAndPlaySingleTone(double sampleRateHz,
		double deviationPercent,
		double peakVoltageLevelVolts,
		int stereoEnable,
		int leftRightRelation,
		double freqHz ,
		double durationMilliSeconds,
		char* audioWaveFileOutput)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_AudioStimulusGenerateAndPlaySingleTone()\n");
	return (*LP_FM_AudioStimulusGenerateAndPlaySingleTone_Ptr)(sampleRateHz,deviationPercent,peakVoltageLevelVolts,
			stereoEnable,leftRightRelation,freqHz,durationMilliSeconds,audioWaveFileOutput);
}

IQMEASURE_API int	LP_FM_StopAudioPlay()
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_FM_StopAudioPlay()\n");
	return (*LP_FM_StopAudioPlay_Ptr)();
}

IQMEASURE_API int	LP_GetCapture(int    dut,
		int    captureIndex,
		double *real[],
		double *imag[],
		int    length[])
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_GetCapture()\n");
	return (*LP_GetCapture_Ptr)(dut,captureIndex,real,imag,length);
}

IQMEASURE_API int	LP_EnableMultiThread(int enableMultiThread,
		int numberOfThreads)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> ()\n");
	return (*LP_EnableMultiThread_Ptr)(enableMultiThread,numberOfThreads);
}

IQMEASURE_API int	LP_SetUserDataPtr(void *userData)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetUserDataPtr()\n");
	return (*LP_SetUserDataPtr_Ptr)(userData);
}

IQMEASURE_API int	LP_SetDefaultNfc()
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_SetDefaultNfc()\n");
	return (*LP_SetDefaultNfc_Ptr)();
}

IQMEASURE_API int	LP_SetNfcTxAmplitude(IQV_USER_SIGNAL_AMPLITUDE_TYPE userSignalAmplitudeType, double userSignalAmplitude)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_SetNfcTxAmplitude()\n");
	return (*LP_SetNfcTxAmplitude_Ptr)(userSignalAmplitudeType,userSignalAmplitude);
}

IQMEASURE_API int	LP_SetTriggerMaskNfc()
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_SetTriggerMaskNfc()\n");
	return (*LP_SetTriggerMaskNfc_Ptr)();
}

IQMEASURE_API int	LP_InitiateNfc(char *fileName)
{
	if (loadDynamicLibrary())
		return 1;
	// printf("--> LP_InitiateNfc()\n");
	return (*LP_InitiateNfc_Ptr)(fileName);
}

IQMEASURE_API int	LP_SetRxNfc(double triggerTimeout, double sampleingTimeSecs)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetRxNfc()\n");
	return (*LP_SetRxNfc_Ptr)(triggerTimeout,sampleingTimeSecs);
}

IQMEASURE_API int	LP_SetTxNfc()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetTxNfc()\n");
	return (*LP_SetTxNfc_Ptr)();
}

IQMEASURE_API int	LP_ContCapture(IQV_DC_CONT_ENUM dcContEnum)
{
	if (loadDynamicLibrary())
		return 1;
	//    printf("--> LP_ContCapture()\n");
	return (*LP_ContCapture_Ptr)(dcContEnum);
}

IQMEASURE_API int	LP_TerminateNfc()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_TerminateNfc()\n");
	return (*LP_TerminateNfc_Ptr)();
}

IQMEASURE_API int	LP_AnalysisNfc()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_AnalysisNfc()\n");
	return (*LP_AnalysisNfc_Ptr)();
}

IQMEASURE_API int	LP_SetVsgModulation_SetPlayCondition(char *modFileName, bool autoPlay, int loadInternalWaveform)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SetVsgModulation_SetPlayCondition()\n");
	return (*LP_SetVsgModulation_SetPlayCondition_Ptr)(modFileName,autoPlay, loadInternalWaveform);
}

IQMEASURE_API int	LP_SetVsaBluetooth_BTShiftHz(double rfFreqHz, double rfAmplDb, int port, double extAttenDb,
		double triggerLevelDb, double triggerPreTime6, double btShiftHz)
{
	if (loadDynamicLibrary())
		return 1;
	//    printf("--> LP_SetVsaBluetooth_BTShiftHz()\n");
	return (*LP_SetVsaBluetooth_BTShiftHz_Ptr)(rfFreqHz,rfAmplDb,port,extAttenDb,
			triggerLevelDb,triggerPreTime6,btShiftHz);
}

IQMEASURE_API int	LP_FM_SetVsa_Agc_On(unsigned int carrierFreqHz, double expectedPeakInputPowerdBm)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_SetVsa_Agc_On()\n");
	return (*LP_FM_SetVsa_Agc_On_Ptr)(carrierFreqHz,expectedPeakInputPowerdBm);
}

//DTNA
IQMEASURE_API int	LP_SetVsg_triggerType(double rfFreqHz, double rfPowerLeveldBm, int port, int triggerType)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SetVsg_triggerType()\n");
	return (*LP_SetVsg_triggerType_Ptr)(rfFreqHz,rfPowerLeveldBm,port,triggerType);
}

IQMEASURE_API int LP_SaveVsaSignalFileText(char *txtFileName)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_SaveVsaSignalFileText()\n");
	return (*LP_SaveVsaSignalFileText_Ptr)(txtFileName);
}

IQMEASURE_API int LP_SaveTruncateCapture(char *sigFileName)
{
	if (loadDynamicLibrary())
		return 1;
	//   printf("--> LP_SaveTruncateCapture()\n");
	return (*LP_SaveTruncateCapture_Ptr)(sigFileName);
}

IQMEASURE_API int	LP_Analyze80211ac(char *mode,
		int enablePhaseCorr,
		int enableSymTimingCorr   ,
		int enableAmplitudeTracking   ,
		int decodePSDU                ,
		int enableFullPacketChannelEst ,
		int frequencyCorr ,
		char *referenceFile,
		int packetFormat,
		int numberOfPacketToAnalysis,
		double prePowStartSec,
		double prePowStopSec)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_Analyze80211ac()\n");
	return (*LP_Analyze80211ac_Ptr)(mode,enablePhaseCorr,enableSymTimingCorr,enableAmplitudeTracking,
			decodePSDU,enableFullPacketChannelEst,frequencyCorr,referenceFile,packetFormat,numberOfPacketToAnalysis,prePowStartSec,prePowStopSec);
}

IQMEASURE_API int LP_AnalyzeVHT80Mask()
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_AnalyzeVHT80Mask()\n");
	return (*LP_AnalyzeVHT80Mask_Ptr)();
}

IQMEASURE_API int	SetTesterHwVersion(double hwVersion)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> SetTesterHwVersion()\n");
	return (*SetTesterHwVersion_Ptr)(hwVersion);
}

IQMEASURE_API int	GetTesterHwVersion(double *hwVersion)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> GetTesterHwVersion()\n");
	return (*GetTesterHwVersion_Ptr)(hwVersion);
}

// IQlite Merge; Tracy Yu; 2012-04-06
IQMEASURE_API int		LP_FM_SetAudioToneModeAmplitude(void)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_SetAudioToneModeAmplitude()\n");
	return (*LP_FM_SetAudioToneModeAmplitude_Ptr)();
}

// IQlite Merge; Tracy Yu; 2012-04-06
IQMEASURE_API int		LP_FM_SetFrequency(unsigned int carrierFreqHz, double carrierPowerdBm)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_SetFrequency()\n");
	return (*LP_FM_SetFrequency_Ptr)(carrierFreqHz,carrierPowerdBm);
}

// IQlite Merge; Buddy; 2012-04-06
IQMEASURE_API int		LP_FM_SetCarrierPower(double carrierPowerdBm)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FM_SetCarrierPower()\n");
	return (*LP_FM_SetCarrierPower_Ptr)(carrierPowerdBm);
}

// IQlite Merge; Buddy: add definition; 2012-04-06
IQMEASURE_API int		LP_FastCalMeasPower(unsigned int start_time, unsigned int  stop_time, double *result)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_FastCalMeasPower()\n");
	return (*LP_FastCalMeasPower_Ptr)(start_time, stop_time, result);
}

// IQlite Merge; Buddy: add definition; 2012-04-06
IQMEASURE_API int		LP_PwrCalFreeElement(void)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_PwrCalFreeElement()\n");
	return (*LP_PwrCalFreeElement_Ptr)();
}

// IQlite Merge; Buddy: add definition; 2012-04-06
IQMEASURE_API int		LP_PwrCalFastGetElement(void)
{
	if (loadDynamicLibrary())
		return 1;
	//  printf("--> LP_PwrCalFastGetElement()\n");
	return (*LP_PwrCalFastGetElement_Ptr)();
}


IQMEASURE_API int		LP_SetTesterName(char * pcName)
{
	strcpy_s(g_cTesterName, 512, pcName);
	return 0;
}

IQMEASURE_API char*     LP_GetTesterName()
{
	return g_cTesterName;
}



//For MTK META
IQMEASURE_API int	LP_GetResultSpectralMask(int bandwidth_index, double *ViolationPercent, int *length, double *freqs_MHz, double *trace_dBm, double *mask)
{
	if (loadDynamicLibrary())
		return 1;

	return (*LP_GetResultSpectralMask_Ptr)(bandwidth_index, ViolationPercent, length, freqs_MHz, trace_dBm, mask);
}

IQMEASURE_API int	LP_GetResultSpectralMask_HT40(double *ViolationPercent, int *length, double *freqs_MHz, double *trace_dBm, double *mask)
{
	if (loadDynamicLibrary())
		return 1;

	return (*LP_GetResultSpectralMask_HT40_Ptr)(ViolationPercent, length, freqs_MHz, trace_dBm, mask);
}

IQMEASURE_API int	LP_LoResultSpectralFlatness11AG(int* carrierNo, double* margin, double* loLeakage, bool* flatnessPass, int* sub_carrier, double* data_per_carrier, double* average_center_power)
{
	if (loadDynamicLibrary())
		return 1;

	return (*LP_LoResultSpectralFlatness11AG_Ptr)(carrierNo, margin, loLeakage, flatnessPass, sub_carrier, data_per_carrier, average_center_power);
}

IQMEASURE_API int	LP_LoResultSpectralFlatness11N(int wifiMode, int* carrierNo, double* margin, double* loLeakage, bool* flatnessPass, int* sub_carrier, double* data_per_carrier, double* average_center_power, int streamIndex)
{
	if (loadDynamicLibrary())
		return 1;

	return (*LP_LoResultSpectralFlatness11N_Ptr)(wifiMode, carrierNo, margin, loLeakage, flatnessPass, sub_carrier, data_per_carrier, average_center_power, streamIndex);
}

#pragma once
#include "IQmeasure_Common.h"

//#if IQMEASURE_SCPI_CONTROL_ENABLE
//#define INSTRUMENT_DRIVER_EXPLICIT_LINK

enum FileType
{
	IQVSG,
	IQVSA,
	IQREF
};


class CIQmeasure_Scpi: public CIQmeasure
{
	public:
		CIQmeasure_Scpi(void);	//!< Constructor
		~CIQmeasure_Scpi(void); //!< Destructor

		int     Term(void);
		int     InitTester(char *ipAddress);
		int     InitTester2(char *ipAddress1, char *ipAddress2);
		int     InitTester3(char *ipAddress1, char *ipAddress2, char *ipAddress3);
		int     InitTester4(char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4);
		int     DualHead_ConOpen(int tokenID, char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4);
		int     ConOpen(char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4);
		int     DualHead_GetTokenID(int *tokenID);
		int     DualHead_ConClose();
		int     DualHead_ObtainControl(unsigned int probeTimeMS, unsigned int timeOutMS);
		int     DualHead_ReleaseControl(void);
		int     SetVsg(double rfFreqHz, double rfPowerLeveldBm, int port, bool setGapPowerOff = true, double dFreqShiftHz = 0.0);
		int     SetVsg_triggerType(double rfFreqHz, double rfPowerLeveldBm, int port, int triggerType);
		int     SetVsg_GapPower(double rfFreqHz, double rfPowerLeveldBm, int port, int gapPowerOff);
		int     SetVsgNxN(double rfFreqHz, double rfPowerLeveldBm[], int port[], double dFreqShiftHz = 0.0);
		int     SetVsgCw(double rfFreqHz, double offsetFrequencyMHz, double rfPowerLeveldBm, int port);
		int     SetVsgModulation(char *modFileName, int loadInternalWaveform);
		int     LoadVsaSignalFile(char *sigFileName);
		int     SetVsaBluetooth(double rfFreqHz, double rfAmplDb, int port, double triggerLevelDb, double triggerPreTimeSecs);
		int     SetVsa(double rfFreqHz, double rfAmplDb, int port, double extAttenDb, double triggerLevelDb, double triggerPreTimeSecs, double dFreqShiftHz = 0.0);
		int     SetVsaTriggerTimeout(double triggerTimeoutSec);
		int     SetVsaNxN(double rfFreqHz, double rfAmplDb[], int port[], double extAttenDb, double triggerLevelDb, double triggerPreTimeSecs, double dFreqShiftHz = 0.0);
		int     Agc(double *rfAmplDb, bool allTesters);
		int     SetFrameCnt(int frameCnt);
		int     TxDone(void);
		int     EnableVsgRF(int enabled);
		int     EnableVsgRFNxN(int vsg1Enabled, int vsg2Enabled, int vsg3Enabled, int vsg4Enabled);
		int     EnableSpecifiedVsgRF(int enabled, int vsgNumber);
		int     EnableSpecifiedVsaRF(int enabled, int vsaNumber);
		int     GetVsaSettings(double *freqHz, double *ampl, IQAPI_PORT_ENUM *port, int *rfEnabled, double *triggerLevel);
		int     VsaDataCapture(double samplingTimeSecs, int triggerType, double sampleFreqHz, int htMode);
		int     SaveVsaSignalFile(char *sigFileName); // Added by Jarir for IQXS SCPI, 12/9/11

		int     Analyze80211ag(int ph_corr_mode, int ch_estimate, int sym_tim_corr, int freq_sync, int ampl_track);
		int     Analyze80211b(int eq_taps, int DCremove11b_flag, int method_11b);
		int     Analyze80211n(char *type, char *mode, int enablePhaseCorr, int enableSymTimingCorr, int enableAmplitudeTracking, int decodePSDU, int enableFullPacketChannelEst, char *referenceFile, int packetFormat, int frequencyCorr = 2);
		int     Analyze80211ac(char *mode, int enablePhaseCorr, int enableSymTimingCorr, int enableAmplitudeTracking, int decodePSDU, int enableFullPacketChannelEst, int frequencyCorr, char *referenceFile, int packetFormat);
		int     AnalyzeBluetooth(double data_rate, char *analysis_type );
		int     AnalyzePower(double T_interval=3.2e-6, double max_pow_diff_dB=15); // Added by Jarir for IQXS SCPI, 12/9/11
		int     AnalyzeCW(double intervalLength = 0.0, double measurementLength = 0.0); // Added by Jarir for IQXS SCPI, 12/9/11
		int     AnalyzeFFT(double NFFT=0, double res_bw=100e3, char *window_type=NULL);
		// Workaround, Jarir 1/13/12, add following AnalyzeHT20Mask() and AnalyzeHT40Mask() functions to differentiate mask
		// calculation between HT20 and HT40 signal, only a global is set and then
		// AnalyzeFFT() function is called.  HT20 spectrum needs truncation.
		int     AnalyzeHT20Mask(double NFFT=0, double res_bw=100e3, char *window_type=NULL);
		int     AnalyzeHT40Mask(double NFFT=0, double res_bw=100e3, char *window_type=NULL);
		//
		int     AnalyzeVHT80Mask();
		double  GetScalarMeasurement(char *measurement, int index);
		int     GetVectorMeasurement(char *measurement, double bufferReal[], double bufferImag[], int bufferLength = 0);
		int     GetStringMeasurement(char *measurement, char bufferChar[], int bufferLength); // Added by Jarir for IQXS SCPI BT, 12/9/11
		bool    GetVersion(char *buffer, int buf_size);

	private:
		double  GetScalarMeasurement_80211ac(char *measurement, int index);
		double  GetScalarMeasurement_80211ag(char *measurement, int index);
		double  GetScalarMeasurement_80211b(char *measurement, int index);
		double  GetScalarMeasurement_80211n(char *measurement, int index);
		double  GetScalarMeasurement_Bluetooth(char *measurement, int index); // Added by Jarir for IQXS SCPI for BT, 12/9/11
		double  GetScalarMeasurement_CW(char *measurement, int index); // Added by Jarir for IQXS SCPI for BT, 12/9/11
		int     GetVectorMeasurement_80211ac(char *measurement, double bufferReal[], double bufferImag[], int bufferLength);
		int     GetVectorMeasurement_80211ag(char *measurement, double bufferReal[], double bufferImag[], int bufferLength);
		int     GetVectorMeasurement_80211b(char *measurement, double bufferReal[], double bufferImag[], int bufferLength);
		int     GetVectorMeasurement_80211n(char *measurement, double bufferReal[], double bufferImag[], int bufferLength);
		int     GetVectorMeasurement_Bluetooth(char *measurement, double bufferReal[], double bufferImag[], int bufferLength);  // Added by Jarir for IQXS SCPI for BT, 12/9/11
		int     GetVectorMeasurement_FFT(char *measurement, double bufferReal[], double bufferImag[], int bufferLength);
		int     GetStringMeasurement_Bluetooth(char *measurement, char bufferChar[], int bufferLength);  // Added by Jarir for IQXS SCPI for BT, 12/9/11
		double  GetScalarMeasurement_Power(char *measurement, int index);  // Added by Jarir for IQXS SCPI for BT, 12/9/11



		//private:
		//
		//#ifdef INSTRUMENT_DRIVER_EXPLICIT_LINK
		//    int (*LP_DLL_Open_Session) (int *session, int comm, char* ipAddress, char* ipPort, int timeout_ms);
		//    int (*LP_DLL_Error_Check_Set) (int session, int state);
		//    int (*LP_DLL_SCPI_Set_Storage) (int state);
		//    int (*LP_DLL_Driver_Set_Storage) (int state);
		//    int (*LP_DLL_Rout_Port_Resource_Set) (int session, int portName, int portMode, unsigned int instance);
		//
		//    int (*LP_DLL_Sys_Reset) (int session);
		//    int (*LP_DLL_Sys_Wait) (int session);
		//    int (*LP_DLL_Sys_Data_Format_Set) (int session, int dataFormat);
		//    int (*LP_DLL_Sys_Capture_Segment_Data) (int session, unsigned int segment, unsigned int numOfSamples, float iqArray, unsigned int *actualSize);
		//
		//    int (*LP_DLL_VSA_Initiate) (int session, int instance);
		//    int (*LP_DLL_VSA_Sampling_Rate_Set) (int session, int instance, unsigned int samplingRate);
		//    int (*LP_DLL_VSA_Sampling_Rate_Get) (int session, int instance, unsigned int *samplingRate);
		//    int (*LP_DLL_VSA_Frequency_Set) (int session, int instance, double freqHz);
		//    int (*LP_DLL_VSA_Frequency_Get) (int session, int instance, double *freqHz);
		//    int (*LP_DLL_VSA_PowerLevel_Set) (int session, int instance, double vsaPwrDBm);
		//    int (*LP_DLL_VSA_PowerLevel_Get) (int session, int instance, double *vsaPwrDBm);
		//    int (*LP_DLL_VSA_Trigger_Source_Set) (int session, int instance, int triggerSource);
		//    int (*LP_DLL_VSA_Trigger_Source_Get) (int session, int instance, int *triggerSource);
		//    int (*LP_DLL_VSA_Trigger_Level_Set) (int session, int instance, double triggerLevelDB);
		//    int (*LP_DLL_VSA_Trigger_Level_Get) (int session, int instance, double *triggerLevelDB);
		//    int (*LP_DLL_VSA_Capture_Time_Set) (int session, int instance, double captureTimeSecs);
		//    int (*LP_DLL_VSA_Capture_Time_Get) (int session, int instance, double *captureTimeSecs);
		//    int (*LP_DLL_VSA_Pretrigger_Time_Set) (int session, int instance, double preTriggerTimeSecs);
		//    int (*LP_DLL_VSA_Pretrigger_Time_Get) (int session, int instance, double *preTriggerTimeSecs);
		//#endif

};

//#endif // #if IQMEASURE_SCPI_CONTROL_ENABLE

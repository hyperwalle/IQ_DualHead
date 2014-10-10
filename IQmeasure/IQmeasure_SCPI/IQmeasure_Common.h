#pragma once

#include "IQmeasure.h"

#ifndef BT_SHIFT_HZ
    #define BT_SHIFT_HZ    7e6
#endif

class CIQmeasure
{
public:	
    CIQmeasure(void);	//!< Constructor
    virtual ~CIQmeasure(void); //!< Destructor

    virtual int Term(void);
    virtual int InitTester(char *ipAddress);
    virtual int InitTester2(char *ipAddress1, char *ipAddress2);
    virtual int InitTester3(char *ipAddress1, char *ipAddress2, char *ipAddress3);
    virtual int InitTester4(char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4);
    virtual int DualHead_ConOpen(int tokenID, char *ipAddress1, char *ipAddress2=NULL, char *ipAddress3=NULL, char *ipAddress4=NULL);
    virtual int ConOpen(char *ipAddress1=NULL, char *ipAddress2=NULL, char *ipAddress3=NULL, char *ipAddress4=NULL);
    virtual int DualHead_GetTokenID(int *tokenID);
    virtual int DualHead_ConClose();
    virtual int DualHead_ObtainControl(unsigned int probeTimeMS=100, unsigned int timeOutMS=0);
    virtual int DualHead_ReleaseControl(void);
    virtual int SetVsg(double rfFreqHz, double rfPowerLeveldBm, int port, bool setGapPowerOff=true, double dFreqShiftHz=0.0);
    virtual int SetVsg_triggerType(double rfFreqHz, double rfPowerLeveldBm, int port, int triggerType);
    virtual int SetVsg_GapPower(double rfFreqHz, double rfPowerLeveldBm, int port, int gapPowerOff);
    virtual int SetVsgNxN(double rfFreqHz, double rfPowerLeveldBm[], int port[], double dFreqShiftHz=0.0);
    virtual int SetVsgCw(double rfFreqHz, double offsetFrequencyMHz, double rfPowerLeveldBm, int port);
    virtual int SetVsgModulation(char *modFileName, int loadInternalWaveform);
    virtual int LoadVsaSignalFile(char *sigFileName);
    virtual int SetVsaBluetooth(double rfFreqHz, double rfAmplDb, int port, double triggerLevelDb=-25, double triggerPreTime=10e-6);
    virtual int SetVsa(double rfFreqHz, double rfAmplDb, int port, double extAttenDb=0, double triggerLevelDb=-25, double triggerPreTime=10e-6, double dFreqShiftHz=0.0);
    virtual int SetVsaTriggerTimeout(double triggerTimeoutSec);
    virtual int SetVsaNxN(double rfFreqHz, double rfAmplDb[], int port[], double extAttenDb=0, double triggerLevelDb=-25, double triggerPreTime=10e-6, double dFreqShiftHz=0.0);
    virtual int Agc(double *rfAmplDb, bool allTesters);
    virtual int SetFrameCnt(int frameCnt);
    virtual int TxDone(void);
    virtual int EnableVsgRF(int enabled);
    virtual int EnableVsgRFNxN(int vsg1Enabled, int vsg2Enabled, int vsg3Enabled, int vsg4Enabled);
    virtual int EnableSpecifiedVsgRF(int enabled, int vsgNumber);
    virtual int EnableSpecifiedVsaRF(int enabled, int vsaNumber);
    virtual int GetVsaSettings(double *freqHz, double *ampl, int *port, int *rfEnabled, double *triggerLevel);
    virtual int VsaDataCapture(double samplingTimeSecs, int triggerType=6, double sampleFreqHz=80e6, int ht40Mode=OFF);
    virtual bool GetVersion(char *buffer, int buf_size);
};

static CIQmeasure *iqMeasure;

// Internal parameters and functions
extern int  nTesters;
extern bool g_bDisableFreqFilter;
extern bool FmInitialized;
extern bool	g_nxnFunctionCalled;
extern bool	g_previousOperationIsRX;
extern bool	g_nxnFunctionCalled;
extern double g_amplitudeToleranceDb;
extern int  g_lastPerformedAnalysisType;
extern int  g_vsgMode;
extern bool bIQ201xFound;

void  InstantiateAnalysisClasses();
int   LP_Analyze(void);
void  InitializeTesterSettings(void);
// End of Internal functions

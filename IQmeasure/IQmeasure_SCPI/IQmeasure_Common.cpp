#include "stdafx.h"
#include "IQmeasure_Common.h"

CIQmeasure::CIQmeasure(void)
{

}
CIQmeasure::~CIQmeasure(void)
{
	delete iqMeasure;
	iqMeasure = NULL;
}

int CIQmeasure::Term(void)
{
	return 0;
}

int CIQmeasure::InitTester(char *ipAddress)
{
	return 0;
}

int CIQmeasure::InitTester2(char *ipAddress1, char *ipAddress2)
{
	return 0;
}

int CIQmeasure::InitTester3(char *ipAddress1, char *ipAddress2, char *ipAddress3)
{
	return 0;
}

int CIQmeasure::InitTester4(char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4)
{
	return 0;
}

int CIQmeasure::DualHead_ConOpen(int tokenID, char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4)
{
	return 0;
}

int CIQmeasure::ConOpen(char *ipAddress1, char *ipAddress2, char *ipAddress3, char *ipAddress4)
{
	return 0;
}

int CIQmeasure::DualHead_GetTokenID(int *tokenID)
{
	return 0;
}

int CIQmeasure::DualHead_ConClose()
{
	return 0;
}

int CIQmeasure::DualHead_ObtainControl(unsigned int probeTimeMS, unsigned int timeOutMS)
{
	return 0;
}

int CIQmeasure::DualHead_ReleaseControl(void)
{
	return 0;
}

int CIQmeasure::SetVsg(double rfFreqHz, double rfPowerLeveldBm, int port, bool setGapPowerOff, double dFreqShiftHz)
{
	return 0;
}

int CIQmeasure::SetVsg_triggerType(double rfFreqHz, double rfPowerLeveldBm, int port, int triggerType)
{
	return 0;
}

int CIQmeasure::SetVsg_GapPower(double rfFreqHz, double rfPowerLeveldBm, int port, int gapPowerOff)
{
	return 0;
}

int CIQmeasure::SetVsgNxN(double rfFreqHz, double rfPowerLeveldBm[], int port[], double dFreqShiftHz)
{
	return 0;
}

int CIQmeasure::SetVsgCw(double sineFreqHz, double offsetFrequencyMHz, double rfPowerLeveldBm, int port)
{
	return 0;
}

int CIQmeasure::SetVsgModulation(char *modFileName, int loadInternalWaveform)
{
	return 0;
}

int CIQmeasure::LoadVsaSignalFile(char *sigFileName)
{
	return 0;
}

int CIQmeasure::SetVsaBluetooth(double rfFreqHz, double rfAmplDb, int port, double triggerLevelDb, double triggerPreTime)
{
	return 0;
}

int CIQmeasure::SetVsa(double rfFreqHz, double rfAmplDb, int port, double extAttenDb, double triggerLevelDb, double triggerPreTime, double dFreqShiftHz)
{
	return 0;
}

int CIQmeasure::SetVsaTriggerTimeout(double triggerTimeoutSec)
{
	return 0;
}

int CIQmeasure::SetVsaNxN(double rfFreqHz, double rfAmplDb[], int port[], double extAttenDb, double triggerLevelDb, double triggerPreTime, double dFreqShiftHz)
{
	return 0;
}

int CIQmeasure::Agc(double *rfAmplDb, bool allTesters)
{
	return 0;
}

int CIQmeasure::SetFrameCnt(int frameCnt)
{
	return 0;
}

int CIQmeasure::TxDone(void)
{
	return 0;
}

int CIQmeasure::EnableVsgRF(int enabled)
{
	return 0;
}

int CIQmeasure::EnableVsgRFNxN(int vsg1Enabled, int vsg2Enabled, int vsg3Enabled, int vsg4Enabled)
{
	return 0;
}

int CIQmeasure::EnableSpecifiedVsgRF(int enabled, int vsgNumber)
{
	return 0;
}

int CIQmeasure::EnableSpecifiedVsaRF(int enabled, int vsaNumber)
{
	return 0;
}

int CIQmeasure::GetVsaSettings(double *freqHz, double *ampl, int *port, int *rfEnabled, double *triggerLevel)
{
	return 0;
}

int CIQmeasure::VsaDataCapture(double samplingTimeSecs, int triggerType, double sampleFreqHz, int ht40Mode)
{
	return 0;
}

bool CIQmeasure::GetVersion(char *buffer, int buf_size)
{
	return true;
}

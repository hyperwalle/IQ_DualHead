/*! \This header file is only for FM Test internal used
 *
 * \section intro_sec Introduction
 *
 * Introduction goes here!
 *
 * \section revision_sec Revision History
 *
 * \subsection revision_0_0_2 0.0.2
 * Date: Mar 16, 2009
 * - Release with introduction of FlowComposer
 *
 * \subsection revision_0_0_1 0.0.1
 * Date: Feb 11, 2009
 * - First draft
 */

/*! \file FM_Test_Internal.h
 * \brief FM Test Functions (Internal used)
 */ 


#ifndef _FM_TEST_INTERNAL_H_
#define _FM_TEST_INTERNAL_H_

#include "IQlite_logger.h"

#ifndef MAX_POWER_LEVEL
	#define MAX_POWER_LEVEL   8
#endif

#ifndef MAX_BUFFER_SIZE
    #define MAX_BUFFER_SIZE 4096
#endif

#ifndef FM_ACP_SECTION
	#define FM_ACP_SECTION   11
#endif

#ifndef AIM_AUDIO_CAPTURE_WAVE_NAME
	#define AIM_AUDIO_CAPTURE_WAVE_LOCATION		"aim_audio_capture.wav"
#endif

enum {IQ_View, IQ_nxn};
enum {Linear=1, LOG_10=10, LOG_20=20, RMS_LOG_20=20};


typedef enum tagSettingValueType
{
    FM_SETTING_TYPE_INTEGER,                       /*!< */
    FM_SETTING_TYPE_DOUBLE,
    FM_SETTING_TYPE_STRING,
	FM_SETTING_TYPE_ARRAY_DOUBLE
} FM_SETTING_TYPE;

typedef struct tagFMTest_Setting
{
    FM_SETTING_TYPE type;
    void*             value;
    char*             unit;
    char*             helpText;
} FM_SETTING_STRUCT;


// The two structures below must be global because other FM functions have to access them.
typedef struct tagGlobalSettingParam
{
	// IQTester Parameters
	double VSG_MAX_POWER_FM;						/*!< Max output power of VSG for FM signal */
	double DUT_PEAK_VOLTAGE_LEVEL_VOLTS;			/*!<DUT Peak Input Voltage corresponding to Peak Deviation set. */
	int	   AUDIO_SAMPLING_RATE_HZ;						   /*!< Audio Sampling Rate in Hz */
	int	   AUDIO_RESOLUTION_BANDWIDTH_HZ;				   /*!< Audio Resolution Bandwidth in Hz */

	int	   TX_RF_RESOLUTION_BANDWIDTH_HZ;					   /*!< RF Resolution Bandwidth in Hz */
	int	   TX_OCCUPIED_BANDWIDTH_PERCENT;					   /*!< RF Occupied Bandwidth in Percent */
	int	   TX_RF_POWER_MEASUREMENT_BANDWIDTH_HZ;			   /*!< RF Power Measurement Bandwidth in Hz */

	int	   TX_RF_MEASUREMENTS_AVERAGE;
	int    TX_AUDIO_AVERAGE;							   /*!< Required minumum number of TX AUDIO measurements. */

	int    RX_AUDIO_AVERAGE;							   /*!< Required minumum number of RX AUDIO measurements. */

	int	   VSA_SAVE_CAPTURE_ALWAYS;
	int	   VSA_SAVE_CAPTURE_ON_FAILED;

	// DUT TX/RX settle time, default = 0 ms
	int	   DUT_TX_SETTLE_TIME_MS;					/*!< A delay time for DUT (TX) settle, Default = 0(ms). */
	int	   DUT_RX_SETTLE_TIME_MS;					/*!< A delay time for DUT (RX) settle, Default = 0(ms). */
	char I2S_TYPE[MAX_BUFFER_SIZE];				   /*Specifies if the I2S interface is MASTER/SLAVE - valid only for Digital Audio Interface*/
	char FM_AUDIO_INTERFACE[MAX_BUFFER_SIZE];	   /*Specifies the Audio Interface to the FM Audio ANALOG/DIGITAL*/

	// Retry Options
	int    retryTestItem;				/*!< Specify no. of retry for individual test item if failed */
	int	   retryTestItemDuringRetry;	/*!< Specify whether a DUT reset function will be called when a retry occurs */
	int	   retryErrorItems;				/*!< Specify whether a retry an item if an error is returned, this will not affect whether an item is retried for a limit failure */

} FM_GLOBAL_SETTING;

/*--------------------*
 * Internal Functions *
 *--------------------*/
void InitializeAllContainers(void);

void InitializeConnectTesterContainers(void);               // Used by FM_Connect_IQtester
void InitializeDisconnectTesterContainers(void);            // Used by FM_Disconnect_IQtester
void InitializeInsertDutContainers(void);                   // Used by FM_Insert_Dut
void InitializeInitializeDutContainers(void);               // Used by FM_Initialize_Dut
void InitializeRemoveDutContainers(void);                   // Used by FM_Remove_Dut

void InitializeTXVerify_RF_MEASUREMENTS_Containers(void);	// Needed by FM_TX_Verify_RF_MEASUREMENTS
void InitializeTXVerify_AUDIO_Containers(void);              // Needed by FM_TX_Verify_AUDIO

void InitializeRXVerify_RSSI_Containers(void);				 // Needed by FM_RX_Verify_RSSI
void InitializeRXVerify_AUDIO_Containers(void);              // Needed by FM_RX_Verify_AUDIO
void InitializeRXVerify_RDS_Containers(void);                // Needed by FM_RX_Verify_RDS


void InitializeGlobalSettingContainers(void);
void InitializeRunExternalProgramContainers(void);
void InitializeloadPathLossTableContainers(void);

int  ValidateIPAddress(char* ipAddress);
int  GetWaveformFileName(char* perfix, char* postfix, char* packetType, char* waveformFileName, int bufferSize);
double CheckSamplingTime(int FMMode, char *preamble11B, char *dataRate, char *packetFormat11N);
//void   ParseVersionString(const std::string &versionString, std::map<std::string, std::string> &versionMap);
void ParseSplitString(const std::string &splitteValue, const std::string spliteString, std::map<std::string, std::string> &spliteMap);

// Clear Memory
void CleanupAllFMContainers(void);

void ClearConnectLPReturn(void);
void ClearDisconnectLPReturn(void);
void ClearGlobalSettingReturn(void);
void ClearInitializeDutReturn(void);
void ClearInsertDutReturn(void);
void ClearLoadPathLossTableReturn(void);
void ClearRemoveDutReturn(void);
void ClearRunExternalProgramReturn(void);
void ClearRxVerify_AUDIO_Return(void);
void ClearRxVerify_RSSI_Return(void);
void ClearRxVerify_RDS_Return(void);
void ClearTxVerify_AUDIO_Return(void);
void ClearTxVerify_RF_MEASUREMENTS_Return(void);

#endif // end of #ifndef _FM_TEST_INTERNAL_H_


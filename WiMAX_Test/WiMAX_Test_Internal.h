/*! \This header file is only for WIMAX Test internal used
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

/*! \file WIMAX_Test_Internal.h
 * \brief WIMAX Test Functions (Internal used)
 */ 


#ifndef _WIMAX_TEST_INTERNAL_H_
#define _WIMAX_TEST_INTERNAL_H_


#define MAX_DATA_STREAM			4
#define MAX_TESTER_NUM			4
#define MAX_SEGMENT_NUM			6 /* #LPTW# cfy,-2010/06/07- */

#ifndef MAX_POWER_STEP
    #define MAX_POWER_STEP		100
#endif

#ifndef MIN_WIMAX_CW_POWER
    #define MIN_WIMAX_CW_POWER	-15
#endif

// Since we measure the CW power of CW tone frequency +- 2.5 MHz, thus make sure this value must set >= 3 MHz is better.
#ifndef WIMAX_FREQ_SHIFT_FOR_CW_MHZ
	#define WIMAX_FREQ_SHIFT_FOR_CW_MHZ		3
#endif

enum {IQ_MAX, IQ_nxn_Plus};
enum {Linear=1, LOG_10=10, LOG_20=20, RMS_LOG_20=20};

typedef enum tagSettingValueType
{
    WIMAX_SETTING_TYPE_INTEGER,                       /*!< */
    WIMAX_SETTING_TYPE_DOUBLE,
    WIMAX_SETTING_TYPE_STRING,
    WIMAX_SETTING_TYPE_ARRAY_DOUBLE
} WIMAX_SETTING_TYPE;

typedef enum tagStreamNumber
{
    WIMAX_ONE_STREAM,
    WIMAX_TWO_STREAM,
    WIMAX_THREE_STREAM,
    WIMAX_FOUR_STREAM
} WIMAX_STREAM_NUM;

typedef struct tagWIMAXTest_Setting
{
    WIMAX_SETTING_TYPE type;
    void*             value;
    char*             unit;
    char*             helpText;
} WIMAX_SETTING_STRUCT;


// The blow two structures need to be global, because other WIMAX functions have to access them
typedef struct tagGlobalSettingParam
{
    // Peak to Average Settings
    int    IQ_P_TO_A_WIMAX;									/*!< Peak to average for WIMAX. */
	int	   IQ_P_TO_A_CW;									/*!< Peak to average for CW signal. */

	// WiMAX DUT test mode
	char   WIMAX_TEST_MODE[MAX_BUFFER_SIZE];				/*!< Indicate the link or non-link test mode. Default=NON-LINK. */

    // PER Measurement Settings
    char   WAVEFORM_PATH[MAX_BUFFER_SIZE];					/*!< Waveform path where all modulation files are stored*/
    char   WAVEFORM_DESTINATION_MAC[MAX_BUFFER_SIZE];		/*!< Destination MAC address in all waveforms*/
	int    PER_VSG_TIMEOUT_SEC;                             /*!< Seconds for VSG timeout*/
    int    PER_WIMAX_PACKETS_NUM;							/*!< Packet number sent to DUT for PER testing*/
    char   WIMAX_WAVEFORM_NAME[MAX_BUFFER_SIZE];			/*!< Waveform name */

	// Let DUT keep transmit, default on.
	int	   DUT_KEEP_TRANSMIT;								/*!< A flag that to let Dut keep Tx until the configuration changed, 0: OFF, 1: ON, Default=ON */

	// DUT TX/RX settle time, default = 0 ms
	int	   DUT_TX_SETTLE_TIME_MS;							/*!< A delay time for DUT (TX) settle, Default = 0(ms). */
	int	   DUT_RX_SETTLE_TIME_MS;							/*!< A delay time for DUT (RX) settle, Default = 0(ms). */

    // IQTester Parameters
    int    VSA_TRIGGER_TYPE;								/*!< IQTester VSA signal trigger type setting. Default=IQV_TRIG_TYPE_IF2_NO_CAL */
	double VSA_AMPLITUDE_TOLERANCE_DB;						/*!< IQTester VSA amplitude setting tolerance. Default=+-3 dB */ 
    double VSA_TRIGGER_LEVEL_DB;							/*!< IQTester VSA signal trigger level setting. Default=-25 dB */
    double VSA_PRE_TRIGGER_TIME_US;							/*!< IQTester VSA signal pre-trigger time setting used for signal capture. */
	int    VSA_SAVE_CAPTURE_ON_FAILED;						/*!< A flag that to save "sig" file when capture failed, 0: OFF, 1: ON, Default=1 */
	int    VSA_SAVE_CAPTURE_ALWAYS;							/*!< A flag that to save "sig" file, always, 0: OFF, 1: ON, Default=OFF */
	int    VSA_PORT;										/*!< IQTester VSA port setting. Default=PORT_LEFT*/
    int    VSG_PORT;										/*!< IQTester VSG port setting. Default=PORT_LEFT*/
    double VSG_MAX_POWER_WIMAX;								/*!< Max output power of VSG for WIMAX signal */
	double VSG_POWER_FOR_SYNCHRONIZATION_DBM;				/*!< The output power of VSG for WiMAX synchronization. */

    double DUT_QUICK_POWER_CHECK_TOLERANCE_DB;				/*!< DUT quick power check amplitude setting tolerance. Default=+-5 */
	int    DUT_QUICK_POWER_CHECK_SETTLE_TIME_MS;			/*!< A delay time for DUT (TX) settle, if quick power check is out of range. Default = 1000(ms). */


    // [WIMAX] Measurement Parameters
	int	   ANALYSIS_WIMAX_PH_CORR_MODE;			//!< Specifies the phase tracking mode used for the analysis. Default: (2) IQV_PH_CORR_SYM_BY_SYM	
												/*!< Valid values are as follows:
												\n 1=Phase tracking off
												\n 2=Symbol by symbol phase tracking (fast){1, 2, 3}
												\n 3=10-symbol moving average (slow) (NOT supported currently) 
												*/
	int    ANALYSIS_WIMAX_CH_ESTIMATE;			//!< Specifies the channel estimation and correction mode. Default: (1) IQV_CH_EST_RAW	
												/*!< Valid values are as follows:
												\n 1=Channel estimate based on long preamble symbol
												\n 3=Channel estimate based on whole burst
												*/												
	int    ANALYSIS_WIMAX_FREQ_SYNC;			//!< Specifies the frequency offset correction mode. Default: (2) IQV_FREQ_SYNC_LONG_TRAIN	 
												/*!< Valid values are as follows:		
												\n 1=Does not perform frequency correction
												\n 2=performs frequency correction by using both coarse and fine frequency estimates
												\n 3=performs time-domain correction based on full packet measurement.
												*/	
	int    ANALYSIS_WIMAX_SYM_TIM_CORR;			//!< Specifies the timing offset correction mode. Default: (2) IQV_SYM_TIM_ON		 	 
												/*!< Valid values are as follows:
												\n 1=Does not perform timing correction
												\n 2=Correction for symbol clock offset
												*/		
	int    ANALYSIS_WIMAX_AMPL_TRACK;			//!< Indicates whether symbol to symbol amplitude tracking is enabled. Default: (1) IQV_AMPL_TRACK_OFF 	 
												/*!<Valid values are as follows:
												\n 1=Disabled
												\n 2=Enabled
												*/

	int	   ANALYSIS_FFT_RBW;				    /*!< This parameter is used to specify the resolution bandwidth in kHz. Default: 100 kHz. */
		
    // Power Measurement related parameters
    int    PM_IF_FREQ_SHIFT_MHZ;                    /*!< VSA center frequency shift in MHz in the case where DUT has a strong LO leakage */
    int    PM_AVERAGE;                              /*!< Number of packets for power averaging during power verification */
    int    PM_SAMPLE_INTERVAL_US;

    // Mask measurement related parameters
    int    MASK_FFT_AVERAGE;
    int    MASK_SAMPLE_INTERVAL_US;

	// Verify Spectrum related parameters
    int    SPECTRUM_SAMPLE_INTERVAL_US;

    // EVM measurement related parameters
    int    EVM_AVERAGE;
    int    EVM_SAMPLE_INTERVAL_US;						/*!< Capture time for measuring EVM. */
	//char   EVM_MAP_CONFIG_FILE_NAME[MAX_BUFFER_SIZE];	/*! For 802.16e analysis, a map configuration file (*.mcf) is required for non-auto-detect operation. */

} WIMAX_GLOBAL_SETTING;

typedef struct tagRecordParam
{
	// Mandatory Parameters
    int    FREQ_MHZ;                                /*! The center frequency (MHz). */  
	int    SIG_TYPE;								/*! Indicates the type of signal. */
	int    RATE_ID;									/*! Indicates signal modulation rate. Valid values for signal modulation rate are 0, 1, 2, 3, 4, 5, 6, 7 and  correspond to {AUTO_DETECT, BPSK 1/2, QPSK 1/2, QPSK 3/4, 16-QAM 1/2, 16-QAM 3/4, 64-QAM 2/3, 64-QAM 3/4}, respectively. */
	double BANDWIDTH_MHZ;							/*! Indicates signal bandwidth. Valid values for the signal bandwidth are 0(auto detect), 1.25, 1.5, 1.75, 2.5, 3, 3.5, 5, 5.5, 6, 7, 8.75, 10, 11, 12, 14, 15, 20.*/
    double CYCLIC_PREFIX;							/*! Indicates signal cyclic prefix ratio. Valid values for signal cyclic prefix ratio are as follows: 0(auto detect), 1/4, 1/8, 1/16, 1/32. */
	double NUM_SYMBOLS;								/*! Number of OFDM symbols in burst. */
    double POWER_DBM;								/*! The output power to verify EVM. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
    //double SAMPLING_TIME_US;                        /*! The sampling time to verify EVM. */
	char   MAP_CONFIG_FILE_NAME[MAX_BUFFER_SIZE];	/*! For 802.16e analysis, a map configuration file (*.mcf) is required for non-auto-detect operation. */

    // DUT Parameters
    int    ANT1;                                     /*!< DUT ANT1 on/off. */
    int    ANT2;                                     /*!< DUT ANT2 on/off. */
    int    ANT3;                                     /*!< DUT ANT3 on/off. */
    int    ANT4;                                     /*!< DUT ANT4 on/off. */

	// Rx only
	int    FRAME_COUNT;                             /*! Number of count to send data packet */

} WIMAX_RECORD_PARAM;

/*--------------------*
 * Internal Functions *
 *--------------------*/
void InitializeAllContainers(void);

void InitializeConnectTesterContainers(void);               // Needed by WIMAX_Connect_IQTester
void InitializeDisconnectTesterContainers(void);            // Needed by WIMAX_Disconnect_IQTester
void InitializeInsertDutContainers(void);                   // Needed by WIMAX_Insert_Dut
void InitializeInitializeDutContainers(void);               // Needed by WIMAX_Initialize_Dut
void InitializeRemoveDutContainers(void);                   // Needed by WIMAX_Remove_Dut
void InitializeResetDutContainers(void);                    // Needed by WIMAX_Reset_Dut
void InitializetxrxVerificationContainers(void);			// Needed by WIMAX_TX_RX_Verification
void InitializeTXVerifyEvmContainers(void);                 // Needed by WIMAX_TX_Verify_Evm
void InitializeTXVerifyMaskContainers(void);                // Needed by WIMAX_TX_Verify_Mask
void InitializeRXVerifyPerContainers(void);                 // Needed by WIMAX_RX_VERIFY_PER
void InitializeTXVerifyPowerContainers(void);				// Needed by WIMAX_TX_Verify_Power_Step
void InitializeTXRXCalContainers(void);                     // Needed by WIMAX_TXRX_Calibration
void InitializeGlobalSettingContainers(void);               // Needed by WIMAX_Global_Setting
void InitializeloadPathLossTableContainers(void);

void InitializeReadMacAddressContainers(void);
void InitializeWriteMacAddressContainers(void);

void InitializeReadEepromContainers(void);
void InitializeWriteEepromContainers(void);

// Clear Memory
void CleanupAllWiMaxContainers(void);

void ClearConnectLPReturn(void);
void ClearDisconnectLPReturn(void);
void ClearGlobalSettingReturn(void);
void ClearInitializeDutReturn(void);
void ClearInsertDutReturn(void);
void ClearLoadPathLossTableReturn(void);
void ClearReadEepromReturn(void);
void ClearReadMacAddressReturn(void);
void ClearRemoveDutReturn(void);
void ClearResetDutReturn(void);
void ClearRxVerifyPerReturn(void);
void ClearTxVerifyEvmReturn(void);
void ClearTxVerifyMaskReturn(void);
void ClearTxVerifyPowerReturn(void);
void ClearTxRxCalReturn(void);
void ClearTxRxVerificationReturn(void);
void ClearWriteEepromReturn(void);
void ClearWriteMacAddressReturn(void);


#endif // end of #ifndef _WIMAX_TEST_INTERNAL_H_


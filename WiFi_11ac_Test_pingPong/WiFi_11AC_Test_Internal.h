/*! \This header file is only for WiFi Test internal used
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

/*! \file WiFi_11AC_Test_Internal.h
 * \brief WiFi Test Functions (Internal used)
 */ 


#pragma warning(disable : 4996)			// turn off deprecated warning for remaining ones


#ifndef _WIFI_11AC_TEST_INTERNAL_H_
#define _WIFI_11AC_TEST_INTERNAL_H_


#define MAX_DATA_STREAM   4
#define MAX_TESTER_NUM    4
#define MAX_POSITIVE_SECTION	5
#define MAX_NEGATIVE_SECTION	5
#define TX_TARGET_POWER_FLAG	-99

#ifndef WIFI_FREQ_SHIFT_FOR_CW_MHZ
	#define WIFI_FREQ_SHIFT_FOR_CW_MHZ		3
#endif

#define TIME_11B_LONG_PREAMBLE			192e-6
#define TIME_11B_SHORT_PREAMBLE			96e-6
#define TIME_11G_PREAMBLE				16e-6
#define TIME_11N_GREEN_FIELD_HEADER		28e-6		// Green field header time, L-STF (8) + HT-LTF1 (8) + HT-SIG (8) + HT-LTF (4)		
#define TIME_11N_MIXED_MODE_HEADER		36e-6		// Mixed mode header time, L-STF (8) + L-LTF (8) + L-SIG (4) + HT-SIG (8) + HT-STF (4) + HT-LTF (4)

#define PACKET_FORMAT_VHT				"VHT"		// 11ac VHT mode keyword 
#define PACKET_FORMAT_HT_MF				"HT_MF"		// 11n HT-MF mode keyword, 
#define PACKET_FORMAT_HT_GF				"HT_GF"		// 11n HT-GF mode keyword, 
#define PACKET_FORMAT_NON_HT			"NON_HT"	// 11ag mode keyword, 

#define STANDARD_802_11_AC				"802.11ac"	// wireless standard 802.11ac keyword
#define STANDARD_802_11_N				"802.11n"	// wireless standard 802.11n keyword
#define STANDARD_802_11_AG				"802.11ag"	// wireless standard 802.11a or 802.11g keyword
#define STANDARD_802_11_B				"802.11b"	// wireless standard 802.11b keyword

enum {IQ_View, IQ_nxn};
enum {Linear=1, LOG_10=10, LOG_20=20, LOG_20_EVM, Linear_Sum = 0};
enum {Dut_Chain_Disabled, Dut_Chain_Enabled}; 

typedef std::vector<double> doubleV;

typedef enum tagSettingValueType
{
    WIFI_SETTING_TYPE_INTEGER,                       /*!< */
    WIFI_SETTING_TYPE_DOUBLE,
    WIFI_SETTING_TYPE_STRING,
    WIFI_SETTING_TYPE_ARRAY_DOUBLE
} WIFI_SETTING_TYPE;

typedef enum tagWifiMode
{
    WIFI_11B,            
    WIFI_11AG,
	WIFI_11N_HT20,
	WIFI_11N_HT40,
    WIFI_11N_MF_HT20,
	WIFI_11N_GF_HT20,
	WIFI_11N_MF_HT40,
    WIFI_11N_GF_HT40,
	WIFI_11AC_VHT20,
	WIFI_11AC_VHT40,
	WIFI_11AC_VHT80,
	WIFI_11AC_VHT160,
	WIFI_11AC_VHT80_80,
	WIFI_11AC_MF_HT20,
	WIFI_11AC_GF_HT20,
	WIFI_11AC_MF_HT40,
	WIFI_11AC_GF_HT40,
	WIFI_11AC_NON_HT
} WIFI_MODE;

typedef enum tagStreamNumber
{
    WIFI_ONE_STREAM,
    WIFI_TWO_STREAM,
    WIFI_THREE_STREAM,
    WIFI_FOUR_STREAM
} WIFI_STREAM_NUM;

typedef struct tagWiFiTest_Setting
{
    WIFI_SETTING_TYPE type;
    void*             value;
    char*             unit;
    char*             helpText;
} WIFI_SETTING_STRUCT;


// The blow two structures need to be global, because other WiFi functions have to access them
typedef struct tagGlobalSettingParam
{
    //Peak to Average Settings
    int    IQ_P_TO_A_11B_1M;                        /*!< Peak to average for 802.11b 1 Mbps. Default=2dB*/
    int    IQ_P_TO_A_11B_2M;                        /*!< Peak to average for 802.11b 2 Mbps. Default=2dB*/
    int    IQ_P_TO_A_11B_5_5M;                      /*!< Peak to average for 802.11b 5.5 Mbps. Default=2dB*/
    int    IQ_P_TO_A_11B_11M;                       /*!< Peak to average for 802.11b 11 Mbps. Default=2dB*/

    int    IQ_P_TO_A_11AG_6M;                       /*!< Peak to average for 802.11a/g 6 Mbps. Default=10dB*/
    int    IQ_P_TO_A_11AG_9M;                       /*!< Peak to average for 802.11a/g 9 Mbps. Default=10dB*/
    int    IQ_P_TO_A_11AG_12M;                      /*!< Peak to average for 802.11a/g 12 Mbps. Default=10dB*/
    int    IQ_P_TO_A_11AG_18M;                      /*!< Peak to average for 802.11a/g 18 Mbps. Default=10dB*/
    int    IQ_P_TO_A_11AG_24M;                      /*!< Peak to average for 802.11a/g 24 Mbps. Default=10dB*/
    int    IQ_P_TO_A_11AG_36M;                      /*!< Peak to average for 802.11a/g 36 Mbps. Default=10dB*/
    int    IQ_P_TO_A_11AG_48M;                      /*!< Peak to average for 802.11a/g 48 Mbps. Default=10dB*/
    int    IQ_P_TO_A_11AG_54M;                      /*!< Peak to average for 802.11a/g 54 Mbps. Default=10dB*/

    int    IQ_P_TO_A_11N_MCS0;                      /*!< Peak to average for 802.11n MCS0. Default=10dB*/
    int    IQ_P_TO_A_11N_MCS1;                      /*!< Peak to average for 802.11n MCS1. Default=10dB*/
    int    IQ_P_TO_A_11N_MCS2;                      /*!< Peak to average for 802.11n MCS2. Default=10dB*/
    int    IQ_P_TO_A_11N_MCS3;                      /*!< Peak to average for 802.11n MCS3. Default=10dB*/
    int    IQ_P_TO_A_11N_MCS4;                      /*!< Peak to average for 802.11n MCS4. Default=10dB*/
    int    IQ_P_TO_A_11N_MCS5;                      /*!< Peak to average for 802.11n MCS5. Default=10dB*/
    int    IQ_P_TO_A_11N_MCS6;                      /*!< Peak to average for 802.11n MCS6. Default=10dB*/
    int    IQ_P_TO_A_11N_MCS7;                      /*!< Peak to average for 802.11n MCS7. Default=10dB*/

    // PER Measurement Settings
    char   PER_WAVEFORM_PATH[MAX_BUFFER_SIZE];              /*!< Waveform path where all modulation files are stored*/
    char   PER_WAVEFORM_DESTINATION_MAC[MAX_BUFFER_SIZE];   /*!< Destination MAC address in all waveforms*/
	int    PER_VSG_TIMEOUT_SEC;                             /*!< Seconds for VSG timeout*/
	char   PER_WAVEFORM_PREFIX_11B[MAX_BUFFER_SIZE];		/*!< Prefix of 802.11b waveform name */
	char   PER_WAVEFORM_PREFIX_11AG[MAX_BUFFER_SIZE];		/*!< Prefix of 802.11ag waveform name */
	char   PER_WAVEFORM_PREFIX_11N[MAX_BUFFER_SIZE];		/*!< Prefix of 802.11n waveform name */
	char   PER_WAVEFORM_PREFIX_11AC[MAX_BUFFER_SIZE];		/*!< Prefix of 802.11ac waveform name */

	int	   PER_FRAME_COUNT_11B;								/*!< Number of frame count to verify 802.11b PER */
	int	   PER_FRAME_COUNT_11AG;							/*!< Number of frame count to verify 802.11ag PER */
	int	   PER_FRAME_COUNT_11N;								/*!< Number of frame count to verify 802.11n PER */
	int	   PER_FRAME_COUNT_11AC;							/*!< Number of frame count to verify 802.11ac PER */



	int	   DUT_KEEP_TRANSMIT;						/*!< A flag that to let Dut keep Tx until the configuration changed, 0: OFF, 1: ON, Default=ON */

	// DUT TX/RX settle time, default = 0 ms
	int	   DUT_TX_SETTLE_TIME_MS;					/*!< A delay time for DUT (TX) settle, Default = 0(ms). */
	int	   DUT_RX_SETTLE_TIME_MS;					/*!< A delay time for DUT (RX) settle, Default = 0(ms). */

    // IQTester Parameters
    int    VSA_TRIGGER_TYPE;                        /*!< IQTester VSA signal trigger type setting. Default=IQV_TRIG_TYPE_IF2_NO_CAL */
	double VSA_AMPLITUDE_TOLERANCE_DB;				/*!< IQTester VSA amplitude setting tolerance. Default=+-3 dB */ 
    double VSA_TRIGGER_LEVEL_DB;                    /*!< IQTester VSA signal trigger level setting. Default=-25 dB */
    double VSA_PRE_TRIGGER_TIME_US;                 /*!< IQTester VSA signal pre-trigger time setting used for signal capture. */
	int    VSA_SAVE_CAPTURE_ON_FAILED;				/*!< A flag that to save "sig" file when capture failed, 0: OFF, 1: ON, Default=1 */
	int    VSA_SAVE_CAPTURE_ALWAYS;				    /*!< A flag that to save "sig" file, always, 0: OFF, 1: ON, Default=OFF */
	int    VSA_PORT;                                /*!< IQTester VSA port setting. Default=PORT_LEFT*/
    int    VSG_PORT;                                /*!< IQTester VSG port setting. Default=PORT_LEFT*/
    double VSG_MAX_POWER_11B;						/*!< Max output power of VSG for 11B signal */
	double VSG_MAX_POWER_11G;						/*!< Max output power of VSG for 11G signal */
	double VSG_MAX_POWER_11N;						/*!< Max output power of VSG for 11N signal */
	double VSG_MAX_POWER_11AC;								/*!< Max output power of VSG for 11AC signal */
	
	//int    VSG_TRIGGER_TYPE;                        /*!< IQTester VSG signal trigger type setting. free-run most of the time*/

    // [802.11b] Parameters
    int    ANALYSIS_11B_EQ_TAPS;
    int    ANALYSIS_11B_DC_REMOVE_FLAG;
    int    ANALYSIS_11B_METHOD_11B;
	int    ANALYSIS_11B_FIXED_01_DATA_SEQUENCE;
		
    // [802.11a/g] Parameters
    int    ANALYSIS_11AG_PH_CORR_MODE;
    int    ANALYSIS_11AG_CH_ESTIMATE;
    int    ANALYSIS_11AG_SYM_TIM_CORR;
    int    ANALYSIS_11AG_FREQ_SYNC;
    int    ANALYSIS_11AG_AMPL_TRACK;
	int    ANALYSIS_OFDM_MODE;	

    // [MIMO] Parameters
    int    ANALYSIS_11N_PHASE_CORR;
    int    ANALYSIS_11N_SYM_TIMING_CORR;
    int    ANALYSIS_11N_AMPLITUDE_TRACKING;
    int    ANALYSIS_11N_DECODE_PSDU;
    int    ANALYSIS_11N_FULL_PACKET_CHANNEL_EST;
	int    ANALYSIS_11N_FREQUENCY_CORRELATION;

	 // [802.11 ac] Parameters
    int    ANALYSIS_11AC_PHASE_CORR;
    int    ANALYSIS_11AC_SYM_TIMING_CORR;
    int    ANALYSIS_11AC_AMPLITUDE_TRACKING;
    int    ANALYSIS_11AC_DECODE_PSDU;
    int    ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST;
	int    ANALYSIS_11AC_FREQUENCY_CORRELATION;

    // Power Measurement related parameters
    //RW-20090426: removed IQ_PM_METHOD
    //int    IQ_PM_METHOD;
    int    PM_IF_FREQ_SHIFT_MHZ;                    /*!< VSA center frequency shift in MHz in the case where DUT has a strong LO leakage */
    int    PM_AVERAGE;                              /*!< Number of packets for power averaging during power verification */
    int    PM_DSSS_SAMPLE_INTERVAL_US;
    int    PM_OFDM_SAMPLE_INTERVAL_US;
	int    PM_HALF_SAMPLE_INTERVAL_US;
	int    PM_QUAR_SAMPLE_INTERVAL_US;
    int    PM_LTS_START_OFFSET_US;				    // not available yet

    // Mask measurement related parameters
    int    MASK_FFT_AVERAGE;
    int    MASK_OFDM_SAMPLE_INTERVAL_US;
    int    MASK_DSSS_SAMPLE_INTERVAL_US;
	int    MASK_HALF_SAMPLE_INTERVAL_US;
	int    MASK_QUAR_SAMPLE_INTERVAL_US;
    int    MASK_INCL_PREAMBLE_DSSS;
    int    MASK_TIME_OFFSET_US;

	// Verify Spectrum related parameters
    int    SPECTRUM_OFDM_SAMPLE_INTERVAL_US;
    int    SPECTRUM_DSSS_SAMPLE_INTERVAL_US;
	int    SPECTRUM_HALF_SAMPLE_INTERVAL_US;
	int    SPECTRUM_QUAR_SAMPLE_INTERVAL_US;

    // EVM measurement related parameters
    int    EVM_AVERAGE;
    int    EVM_SYMBOL_NUM;
    int    EVM_PRE_TRIG_TIME_US;
	int    EVM_CAPTURE_TIME_11B_L_US;           /*!< Capture time for measuring 11b (long preamble) EVM.  =192+1000/11 */
	int    EVM_CAPTURE_TIME_11B_S_US;           /*!< Capture time for measuring 11b (short preamble) EVM. =96+1000/11 */
	int    EVM_CAPTURE_TIME_11AG_US;            /*!< Capture time for measuring 11ag EVM. =20+4*18 */
	int    EVM_CAPTURE_TIME_11N_MIXED_US;       /*!< Capture time for measuring 11n mixed format EVM. =20+12+4*4+4*18 */
	int    EVM_CAPTURE_TIME_11N_GREENFIELD_US;  /*!< Capture time for measuring 11n greenfield EVM. =24+4*4+4*18 */

	//EVM capture time for 802.11ac.  Values will be decided later.
	int    EVM_CAPTURE_TIME_11AC_VHT_US;
	int    EVM_CAPTURE_TIME_11AC_HT_MF_US;
	int    EVM_CAPTURE_TIME_11AC_HT_GF_US;
	int    EVM_CAPTURE_TIME_11AC_NON_HT_US;
	int    EVM_CAPTURE_TIME_11P_HALF_US;            /*!< Capture time for measuring 11p EVM. =40+8*18 */
	int    EVM_CAPTURE_TIME_11P_QUAR_US;            /*!< Capture time for measuring 11p EVM. =40+8*18 */
	
	//int    EVM_CAPTURE_PSDU_BYTE;				/*!< No. of PSDU byte to be captured */  //This is hard to use in IQwizard as it will assign same value for all data rate, Zhiyong May 17, 2011

	// Retry Options
	int    retryTestItem;				/*!< Specify no. of retry for individual test item if failed */
	int	   retryTestItemDuringRetry;	/*!< Specify whether a DUT reset function will be called when a retry occurs */
	int	   retryErrorItems;				/*!< Specify whether a retry an item if an error is returned, this will not affect whether an item is retried for a limit failure */

 

	// Indicate for 11AC, use cbw or bssBW to define the standard mask
	int		ANALYSIS_11AC_MASK_ACCORDING_CBW;										/*!< Indicate for 11AC, standard mask is according to CBW or bssBW*/ 

	int    RELATIVE_LIMIT;                             /*!< Use relative power limit*/
    int    RESET_OTA_ATTN_TABLE;

	//Read Limit files
	int    AUTO_READING_LIMIT;

} WIFI_GLOBAL_SETTING;

typedef struct tagRecordParam
{
    // Mandatory Parameters
	int    BSS_FREQ_MHZ_PRIMARY;                            /*!< For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment */
	int    BSS_FREQ_MHZ_SECONDARY;                          /*!< For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined */
	int    CH_FREQ_MHZ_PRIMARY_20MHz;                          /*!< The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ"   */
	int    CH_FREQ_MHZ;										/*!< The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used  */
	int    FREQ_MHZ_SECONDARY_80;                   /*! The center frequency (MHz) for secondary VHT80 in non-contiguous 80+80 mode */
	int    FREQ_MHZ_PRIMARY_20;                     /*! The center frequency (MHz) for primary 20 MHZ channel */
	int    NUM_STREAM_11AC;							/*! The number of streams 11AC (Only). */
    char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF channel bandwidth. */
	char   BSS_BANDWIDTH[MAX_BUFFER_SIZE];          /*! The RF bandwidth of basic service set (BSS) to verify EVM. */
	char   CH_BANDWIDTH[MAX_BUFFER_SIZE];                  /*!< The RF channel bandwidth to verify EVM. */
    char   DATA_RATE[MAX_BUFFER_SIZE];              /*! The data rate. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT[MAX_BUFFER_SIZE];          /*! The packet format of 11AC and 11N. */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];          /*! The Guard Interval for 11AC and 11N. */
    double POWER_DBM;                               /*! The amplitude level in dBm. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */

    // DUT Parameters
    int    ANT1;                                     /*!< DUT ANT1 on/off. */
    int    ANT2;                                     /*!< DUT ANT2 on/off. */
    int    ANT3;                                     /*!< DUT ANT3 on/off. */
    int    ANT4;                                     /*!< DUT ANT4 on/off. */
	//ANT5 to ANT8 in the future. 

	// Rx only
	int    FRAME_COUNT;                             /*! Number of count to send data packet */

} WIFI_RECORD_PARAM;


/*--------------------*
 * Internal Functions *
 *--------------------*/
int InitializeAllContainers(void);

int Initialize11ACConnectTesterContainers(void);               // Needed by WiFi_Connect_IQTester
int Initialize11ACDisconnectTesterContainers(void);            // Needed by WiFi_Disconnect_IQTester
int Initialize11ACInsertDutContainers(void);                   // Needed by WiFi_Insert_Dut
int Initialize11ACInitializeDutContainers(void);               // Needed by WiFi_Initialize_Dut
int Initialize11ACRemoveDutContainers(void);                   // Needed by WiFi_Remove_Dut
int Initialize11ACTXMultiVerificationContainers(void);
int Initialize11ACTXVerifyEvmContainers(void);                 // Needed by WiFi_TX_Verify_Evm
int Initialize11ACRXVerifyPerContainers(void);                 // Needed by WiFi_RX_Verify_Per
int Initialize11ACTXVerifyMaskContainers(void);                // Needed by WiFi_TX_Verify_Mask
int Initialize11ACRunDutCommandContainers(void);              // Needed by WiFi_Run_Dut_Command
int Initialize11ACTXVerifyPowerContainers(void);               // Needed by WiFi_TX_Verify_Power
int Initialize11ACGlobalSettingContainers(void);               // Needed by WiFi_Global_Setting
int Initialize11ACTXVerifyFlatnessContainers(void);            // Needed by WiFi_TX_Verify_Flatness
int Initialize11ACReadEepromContainers(void);                  // Needed by WiFi_Read_Eeprom
int Initialize11ACWriteEepromContainers(void);                 // Needed by WiFi_Write_Eeprom
int Initialize11ACWriteBBRegisterContainers(void);             // Needed by WiFi_Write_BB_Register.cpp
int Initialize11ACReadBBRegisterContainers(void);              // Needed by WiFi_Read_BB_Register.cpp
int Initialize11ACWriteRFRegisterContainers(void);             // Needed by WiFi_Write_RF_Register.cpp
int Initialize11ACReadRFRegisterContainers(void);              // Needed by WiFi_Read_RF_Register.cpp
int Initialize11ACWriteMacRegisterContainers(void);            // Needed by WiFi_Write_Mac_Register.cpp
int Initialize11ACReadMacRegisterContainers(void);             // Needed by WiFi_Read_Mac_Register.cpp
int Initialize11ACWriteSocRegisterContainers(void);            // Needed by WiFi_Write_Soc_Register.cpp
int Initialize11ACReadSocRegisterContainers(void);             // Needed by WiFi_Read_Soc_Register.cpp
int Initialize11ACRunExternalProgramContainers(void);          // Needed by WiFi_Run_External_Program
int Initialize11ACGetSerialNumberContainers(void);             // Needed by WiFi_Get_Serial_Number

int Initialize11ACTXCalContainers(void);                       // Needed by WiFi_TX_Calibration
int Initialize11ACWriteMacAddressContainers(void);             // Needed by WiFi_Write_Mac_Address
int Initialize11ACReadMacAddressContainers(void);              // Needed by WiFi_Read_Mac_Address

int Initialize11ACPowerModeDutContainers(void);				// Needed by WiFi Current test
int Initialize11ACloadPathLossTableContainers(void);
int Initialize11ACTemplateContainers(void);					// This is an example of Template_Function

extern int g_useLpDut;

#endif // end of #ifndef _WIFI_TEST_INTERNAL_H_


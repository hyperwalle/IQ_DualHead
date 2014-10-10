/*! \This header file is only for BT Test internal used
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

/*! \file BT_Test_Internal.h
 * \brief BT Test Functions (Internal used)
 */


#ifndef _BT_TEST_INTERNAL_H_
#define _BT_TEST_INTERNAL_H_

using namespace std;

#ifndef MAX_POWER_STEP
    #define MAX_POWER_STEP		20
#endif

#ifndef MAX_BUFFER_SIZE
    #define MAX_BUFFER_SIZE 4096
#endif

#ifndef BT_ACP_SECTION
	#define BT_ACP_SECTION   11
#endif

#ifndef BT_FREQ_SHIFT_FOR_CW_MHZ
	#define BT_FREQ_SHIFT_FOR_CW_MHZ		3
#endif

#define MPS_WAVEFILE_NAME "000000C0FFEE_MCS7_MCS4_MCS2_MCS0_54M_36M_24M_18M_12M_6M_11M_1M_1DH5_3DH5.mod"

enum {IQ_View, IQ_nxn};
enum {Linear=1, LOG_10=10, LOG_20=20, RMS_LOG_20=20};
enum TESTER_TYPE {TESTER_TYPE_IQVIEW=IQ_View, TESTER_TYPE_NXN=IQ_nxn, TESTER_TYPE_2010, TESTER_TYPE_A21};

typedef enum tagSettingValueType
{
    BT_SETTING_TYPE_INTEGER,                       /*!< */
    BT_SETTING_TYPE_DOUBLE,
    BT_SETTING_TYPE_STRING,
	BT_SETTING_TYPE_ARRAY_DOUBLE,
} BT_SETTING_TYPE;

typedef enum tagBTMode
{
    BT_DH1,
    BT_DH3,
    BT_DH5,
} BT_MODE;


typedef struct tagBTTest_Setting
{
    BT_SETTING_TYPE type;
    void*             value;
    char*             unit;
    char*             helpText;
} BT_SETTING_STRUCT;


// The two structures below must be global because other BT functions have to access them.
typedef struct tagGlobalSettingParam
{
    // Peak to Average Settings
    int    IQ_P_TO_A_BDR;									/*!< Peak to average for BDR. */
	int	   IQ_P_TO_A_EDR;									/*!< Peak to average for EDR. */
    int    IQ_P_TO_A_LE;									/*!< Peak to average for LE. */
	int	   IQ_P_TO_A_CW;									/*!< Peak to average for CW. */

    // BER Measurement Settings
    char   BER_WAVEFORM_PATH[MAX_BUFFER_SIZE];         /*!< Waveform path where all modulation files are stored*/
    char   BER_WAVEFORM_BD_ADDRESS[MAX_BUFFER_SIZE];   /*!< BD address in all waveforms*/
	int    BER_VSG_TIMEOUT_SEC;                        /*!< Seconds for VSG timeout*/

    // BDR
	int    TX_BDR_AVERAGE;							   /*!< Required minumum number of TX_BDR measurements. */
    int    BER_1DH1_PACKETS_LENGTH;                    /*!< Packet LENGTH of 1DH1 sent to DUT for BER testing */
    char   BER_1DH1_WAVEFORM_NAME[MAX_BUFFER_SIZE];    /*!< Waveform name for 1DH1 */
    int    BER_1DH3_PACKETS_LENGTH;                    /*!< Packet LENGTH of 1DH3 sent to DUT for BER testing */
    char   BER_1DH3_WAVEFORM_NAME[MAX_BUFFER_SIZE];    /*!< Waveform name for 1DH3 */
    int    BER_1DH5_PACKETS_LENGTH;                    /*!< Packet LENGTH of 1DH5 sent to DUT for BER testing */
    char   BER_1DH5_WAVEFORM_NAME[MAX_BUFFER_SIZE];    /*!< Waveform name for 1DH5 */

    // EDR
	int    TX_EDR_AVERAGE;							   /*!< Required minumum number of TX_EDR measurements. */
    int    BER_2DH1_PACKETS_LENGTH;                    /*!< Packet LENGTH of 2DH1 sent to DUT for BER testing */
    char   BER_2DH1_WAVEFORM_NAME[MAX_BUFFER_SIZE];    /*!< Waveform name for 2DH1 */
    int    BER_2DH3_PACKETS_LENGTH;                    /*!< Packet LENGTH of 2DH3 sent to DUT for BER testing */
    char   BER_2DH3_WAVEFORM_NAME[MAX_BUFFER_SIZE];    /*!< Waveform name for 2DH3 */
    int    BER_2DH5_PACKETS_LENGTH;                    /*!< Packet LENGTH of 2DH5 sent to DUT for BER testing */
    char   BER_2DH5_WAVEFORM_NAME[MAX_BUFFER_SIZE];    /*!< Waveform name for 2DH5 */

    int    BER_3DH1_PACKETS_LENGTH;                    /*!< Packet LENGTH of 3DH1 sent to DUT for BER testing */
    char   BER_3DH1_WAVEFORM_NAME[MAX_BUFFER_SIZE];    /*!< Waveform name for 3DH1 */
    int    BER_3DH3_PACKETS_LENGTH;                    /*!< Packet LENGTH of 3DH3 sent to DUT for BER testing */
    char   BER_3DH3_WAVEFORM_NAME[MAX_BUFFER_SIZE];    /*!< Waveform name for 3DH3 */
    int    BER_3DH5_PACKETS_LENGTH;                    /*!< Packet LENGTH of 3DH5 sent to DUT for BER testing */
    char   BER_3DH5_WAVEFORM_NAME[MAX_BUFFER_SIZE];    /*!< Waveform name for 3DH5 */

    // LE
    int    TX_LE_AVERAGE;							   /*!< Required minumum number of TX_LE measurements. */
    int    PER_1LE_PACKETS_LENGTH;                    /*!< Packet LENGTH of 1DH1 sent to DUT for BER testing */
    char   PER_1LE_WAVEFORM_NAME[MAX_BUFFER_SIZE];    /*!< Waveform name for 1DH1 */
    int    BT_1LE_PER_WAVEFORM_INTERVAL_US;            /*!< Waveform length for PER test , default = 625 us. */

	// Tx Power Control
	int    TX_POWER_CONTROL_AVERAGE;				   /*!< Required minumum number of TX_POWER_CONTROL measurements. */

    // IQTester Parameters
    int    VSA_TRIGGER_TYPE;                        /*!< IQtester VSA signal trigger type setting. Default=IQV_TRIG_TYPE_IF2_NO_CAL*/
	double VSA_AMPLITUDE_TOLERANCE_DB;				/*!< IQtester VSA amplitude setting tolerance. Default=+-3 dB */
	double VSA_TRIGGER_LEVEL_DB;                    /*!< IQtester VSA signal trigger level setting. Default=-25 dB*/
    double VSA_PRE_TRIGGER_TIME_US;                 /*!< IQtester VSA signal pre-trigger time setting used for signal capture. */
	int    VSA_SAVE_CAPTURE_ON_FAILED;				/*!< A flag to save "sig" file when capture fails; 0: OFF, 1: ON, Default=ON */
	int    VSA_SAVE_CAPTURE_ALWAYS;				    /*!< A flag to save "sig" file always; 0: OFF, 1: ON, Default=OFF */
	int    VSA_PORT;                                /*!< IQtester VSA port setting. Default=PORT_LEFT*/
    int    VSG_PORT;                                /*!< IQtester VSG port setting. Default=PORT_LEFT*/
    //int    VSG_TRIGGER_TYPE;                      /*!< IQtester VSG signal trigger type setting; free-run most of the time*/
    double VSG_MAX_POWER_BT;						/*!< Max output power of VSG for BT signal */

	int    BT_DH1_SAMPLE_INTERVAL_US;               /*!< Capture time for measuring DH1 packet, default = 625 us. */
	int    BT_DH3_SAMPLE_INTERVAL_US;               /*!< Capture time for measuring DH3 packet, default = 1875 us. */
	int    BT_DH5_SAMPLE_INTERVAL_US;               /*!< Capture time for measuring DH5 packet, default = 3125 us. */
    int    BT_1LE_SAMPLE_INTERVAL_US;               /*!< Capture time for measuring 1LE packet, default = 625 us. */


	// DUT TX/RX settle time, default = 0 ms
	int	   DUT_TX_SETTLE_TIME_MS;					/*!< A delay time for DUT (TX) settle, Default = 0(ms). */
	int	   DUT_RX_SETTLE_TIME_MS;					/*!< A delay time for DUT (RX) settle, Default = 0(ms). */
    int	   IQ2010_EXT_ENABLE;					    /*!< Auto-detect and enable the IQ2010_EXT mode. Default=ON*/
    TESTER_TYPE	IQ_TESTER_TYPE;				        /*!< Auto-detect and enable the IQ2010 or IQExpress. Default=2010*/
    bool   MULTI_WAVE_FILE_CHANGED;

	// Retry Options
	int    retryTestItem;				/*!< Specify no. of retry for individual test item if failed */
	int	   retryTestItemDuringRetry;	/*!< Specify whether a DUT reset function will be called when a retry occurs */
	int	   retryErrorItems;				/*!< Specify whether a retry an item if an error is returned, this will not affect whether an item is retried for a limit failure */

	//Read Limit files
	int    AUTO_READING_LIMIT;

    char   MP_WAVE_FILE_NAME[MAX_BUFFER_SIZE];
    vector <string> tab_MP_Wave_DataRate_segment;
} BT_GLOBAL_SETTING;

/*--------------------*
 * Internal Functions *
 *--------------------*/
int  InitializeAllContainers(void);

int InitializeConnectTesterContainers(void);               // Used by BT_Connect_IQtester
int InitializeDisconnectTesterContainers(void);            // Used by BT_Disconnect_IQtester
int InitializeInsertDutContainers(void);                   // Used by BT_Insert_Dut
int InitializeInitializeDutContainers(void);               // Used by BT_Initialize_Dut
int InitializeRemoveDutContainers(void);                   // Used by BT_Remove_Dut
int InitializeTXVerifyBDRContainers(void);                 // Used by BT_TX_Verify_BDR
int InitializeTXVerifyEDRContainers(void);					// Used by BT_TX_Verify_EDR
int InitializeRXVerifyBDRContainers(void);                 // Used by BT_RX_Verify_BDR
int InitializeRXVerifyEDRContainers(void);                 // Used by BT_RX_Verify_EDR

int InitializeTXPowerControlContainers(void);
int InitializeGlobalSettingContainers(void);
int InitializeWriteBDAddressContainers(void);              // Used by BT_WRITE_BD_ADDRESS
int InitializeReadBDAddressContainers(void);               // Used by BT_READ_BD_ADDRESS
int InitializeTXCalContainers(void);
int InitializeloadPathLossTableContainers(void);
int InitializeWriteDutInfoContainers(void);                //Used by BT_WRITE_DUT_INFO

int InitializeRunExternalProgramContainers(void);          // Needed by WiFi_Run_External_Program
int InitializePowerModeDutContainers(void);				// Needed by BT_Power_Mode_Dut
int InitializeResetDutContainers(void);

//BT LE
int InitializeTXVerifyLEContainers(void);					// Used by BT_TX_Verify_LE
int InitializeRXVerifyLEContainers(void);                  // Used by BT_RX_Verify_LE
int InitializebtleInitializeContainers(void);				// Needed by BT_LE_INITIALIZE
// Clear Memory
int CleanupAllBTContainers(void);

int ClearConnectLPReturn(void);
int ClearDisconnectLPReturn(void);
int ClearGlobalSettingReturn(void);
int ClearInitializeDutReturn(void);
int ClearInsertDutReturn(void);
int ClearLoadPathLossTableReturn(void);
int ClearCurrentTestReturn(void);
int ClearReadBDAddressReturn(void);
int ClearRemoveDutReturn(void);
void ClearRunExternalProgramReturn(void);
int ClearRxVerifyBdrReturn(void);
int ClearRxVerifyEdrReturn(void);
int ClearTxCalReturn(void);
int ClearTxPowerControlReturn(void);
int ClearTxVerifyBDRReturn(void);
int ClearTxVerifyEDRReturn(void);
int ClearWriteBDAddressReturn(void);
int ClearwriteDutInfoReturn(void);
//BT LE
int ClearRxVerifyLEReturn(void);
int ClearTxVerifyLEReturn(void);
int ClearbtleInitializeReturn(void);

#endif // end of #ifndef _BT_TEST_INTERNAL_H_


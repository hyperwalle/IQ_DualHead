/*! \This header file is only for GPS Test internal used
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

/*! \file GPS_Test_Internal.h
 * \brief GPS Test Functions (Internal used)
 */ 


#ifndef _GPS_TEST_INTERNAL_H_
#define _GPS_TEST_INTERNAL_H_


#ifndef MAX_POWER_STEP
    #define MAX_POWER_STEP		20
#endif

#ifndef MAX_BUFFER_SIZE
    #define MAX_BUFFER_SIZE 4096
#endif

#ifndef GPS_ACP_SECTION
	#define GPS_ACP_SECTION   11
#endif

#ifndef GPS_FREQ_SHIFT_FOR_CW_MHZ
	#define GPS_FREQ_SHIFT_FOR_CW_MHZ		3
#endif

enum {IQ_View, IQ_nxn};
enum {Linear=1, LOG_10=10, LOG_20=20, RMS_LOG_20=20};

typedef enum tagSettingValueType
{
    GPS_SETTING_TYPE_INTEGER,                       /*!< */
    GPS_SETTING_TYPE_DOUBLE,
    GPS_SETTING_TYPE_STRING,
	GPS_SETTING_TYPE_ARRAY_DOUBLE,
} GPS_SETTING_TYPE;

typedef enum tagGPSMode
{
    GPS_DH1,            
    GPS_DH3,
    GPS_DH5,
} GPS_MODE;

typedef struct tagGPSTest_Setting
{
    GPS_SETTING_TYPE type;
    void*             value;
    char*             unit;
    char*             helpText;
} GPS_SETTING_STRUCT;

// The two structures below must be global because other GPS functions have to access them.
typedef struct tagGlobalSettingParam
{
    double VSG_MAX_POWER_GPS;						/*!< Max output power of VSG for BT signal */
    int    VSA_TRIGGER_TYPE;                        /*!< IQtester VSA signal trigger type setting. Default=IQV_TRIG_TYPE_IF2_NO_CAL*/
	int	   DUT_RX_SETTLE_TIME_MS;					/*!< A delay time for DUT (RX) settle, Default = 0(ms). */
    int    DOPPLER_FREQUENCY;                       /*!< doppler frequency : for red-motivation fix, Default = 4000(hz). */
    int    TRIGGER_STATE;
    char   SCENARIO_FILE_PATH[MAX_BUFFER_SIZE];
	// [Jarir Fadlullah] Added for Brcm 2076 GPS DUT
	char   LOG_FILE_PATH[MAX_BUFFER_SIZE];
	char   CONFIG_TEMPLATE_XML[MAX_BUFFER_SIZE];
 char   GPS_HOST_SHELL_SCRIPT[MAX_BUFFER_SIZE];
 char   GPS_HOST_PATH[MAX_BUFFER_SIZE]; 
	int DELETE_TEMP_FILES;
	////////////////////////////////////////////////
} GPS_GLOBAL_SETTING;

/*--------------------*
 * Internal Functions *
 *--------------------*/
void InitializeAllGPSContainers(void);

void InitializeConnectTesterContainers(void);               // Used by GPS_Connect_IQtester
void InitializeDisconnectTesterContainers(void);            // Used by GPS_Disconnect_IQtester
void InitializeInsertDutContainers(void);                   // Used by GPS_Insert_Dut
void InitializeInitializeDutContainers(void);               // Used by GPS_Initialize_Dut
void InitializeRemoveDutContainers(void);                   // Used by GPS_Remove_Dut
void InitializeTXVerifyBDRContainers(void);                 // Used by GPS_TX_Verify_BDR
void InitializeTXVerifyEDRContainers(void);					// Used by GPS_TX_Verify_EDR
void InitializeRXVerifyBDRContainers(void);                 // Used by GPS_RX_Verify_BDR
void InitializeRXVerifyEDRContainers(void);                 // Used by GPS_RX_Verify_EDR
void InitializeTXPowerControlContainers(void);
void InitializeGlobalSettingContainers(void);
void InitializeWriteBDAddressContainers(void);              // Used by GPS_WRITE_BD_ADDRESS   
void InitializeReadBDAddressContainers(void);               // Used by GPS_READ_BD_ADDRESS   
void InitializeTXCalContainers(void);
void InitializeloadPathLossTableContainers(void);
void InitializeWriteDutInfoContainers(void);                //Used by GPS_WRITE_DUT_INFO

void InitializeRunExternalProgramContainers(void);          // Needed by WiFi_Run_External_Program
void InitializePowerModeDutContainers(void);				// Needed by GPS_Power_Mode_Dut

void InitializeGPSTransmitScenarioContainers(void);
void InitializeGPSModulatedModeContainers(void);
void InitializeGPSContinueWaveContainers(void);

// Clear Memory
void CleanupAllGPSContainers(void);

void ClearConnectLPReturn(void);
void ClearDisconnectLPReturn(void);
void ClearGlobalSettingReturn(void);
void ClearInitializeDutReturn(void);
void ClearInsertDutReturn(void);
void ClearLoadPathLossTableReturn(void);
void ClearCurrentTestReturn(void);
void ClearReadBDAddressReturn(void);
void ClearRemoveDutReturn(void);
void ClearRunExternalProgramReturn(void);
void ClearRxVerifyBdrReturn(void);
void ClearRxVerifyEdrReturn(void);
void ClearTxCalReturn(void);
void ClearTxPowerControlReturn(void);
void ClearTxVerifyBDRReturn(void);
void ClearTxVerifyEDRReturn(void);
void ClearWriteBDAddressReturn(void);
void ClearwriteDutInfoReturn(void);

void ClearTransmitScenarioReturn(void);
void ClearModulatedModeReturn(void);
void ClearContinueWaveReturn(void);

#endif // end of #ifndef _GPS_TEST_INTERNAL_H_


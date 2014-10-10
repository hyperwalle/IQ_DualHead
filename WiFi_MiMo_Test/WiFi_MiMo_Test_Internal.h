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

/*! \file WiFi_MIMO_Test_Internal.h
 * \brief WiFi MIMO Test Functions (Internal used)
 */ 

#pragma warning(disable : 4996)		// turn off deprecated warning for remaining ones


#ifndef _WIFI_MIMO_TEST_INTERNAL_H_
#define _WIFI_MIMO_TEST_INTERNAL_H_


#define MAX_DATA_STREAM   4
#define MAX_TESTER_NUM    4
#define MAX_CHAIN_NUM     4
#define MAX_POSITIVE_SECTION	5
#define MAX_NEGATIVE_SECTION	5
#define TX_TARGET_POWER_FLAG	-99      // IQlite merge; Tracy Yu ; 2012-03-31

#define TIME_11B_LONG_PREAMBLE			192e-6
#define TIME_11B_SHORT_PREAMBLE			96e-6
#define TIME_11G_PREAMBLE				16e-6
#define TIME_11N_GREEN_FIELD_HEADER		28e-6		// Green field header time, L-STF (8) + HT-LTF1 (8) + HT-SIG (8) + HT-LTF (4)		
#define TIME_11N_MIXED_MODE_HEADER		36e-6		// Mixed mode header time, L-STF (8) + L-LTF (8) + L-SIG (4) + HT-SIG (8) + HT-STF (4) + HT-LTF (4)


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
	WIFI_11P_DSRC10,
	WIFI_11P_QUAR5
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
	// IQlite merge; Tracy Yu ; 2012-03-31
	int    IQ_P_TO_A_11B_CH14;                      /*!< Peak to average for 802.11b at channel 14. Default=5dB*/

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

    // 11b
    int    PER_11B_1_PACKETS_NUM;                       /*!< Packet number of 1 Mbps sent to DUT for PER testing*/
    char   PER_11B_1_WAVEFORM_NAME[MAX_BUFFER_SIZE];    /*!< Waveform name for 1 Mbps (Long Preamble only)*/

    int    PER_11B_2_PACKETS_NUM;                       /*!< Packet number of 2 Mbps sent to DUT for PER testing*/
    char   PER_11B_2L_WAVEFORM_NAME[MAX_BUFFER_SIZE];   /*!< Waveform name for 2 Mbps (Long Preamble)*/
    char   PER_11B_2S_WAVEFORM_NAME[MAX_BUFFER_SIZE];   /*!< Waveform name for 2 Mbps (Short Preamble)*/

    int    PER_11B_5_5_PACKETS_NUM;                     /*!< Packet number of 5.5 Mbps sent to DUT for PER testing*/
    char   PER_11B_5_5L_WAVEFORM_NAME[MAX_BUFFER_SIZE]; /*!< Waveform name for 5.5 Mbps (Long Preamble)*/
    char   PER_11B_5_5S_WAVEFORM_NAME[MAX_BUFFER_SIZE]; /*!< Waveform name for 5.5 Mbps (Short Preamble)*/

    int    PER_11B_11_PACKETS_NUM;                      /*!< Packet number of 11 Mbps sent to DUT for PER testing*/
    char   PER_11B_11L_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for 11 Mbps (Long Preamble)*/
    char   PER_11B_11S_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for 11 Mbps (Short Preamble)*/

    // 11ag
    int    PER_11AG_6_PACKETS_NUM;                      /*!< Packet number of OFDM-6 sent to DUT for PER testing*/
    char   PER_11AG_6_WAVEFORM_NAME[MAX_BUFFER_SIZE];   /*!< Waveform name for OFDM-6*/

    int    PER_11AG_9_PACKETS_NUM;                      /*!< Packet number of OFDM-9 sent to DUT for PER testing*/
    char   PER_11AG_9_WAVEFORM_NAME[MAX_BUFFER_SIZE];   /*!< Waveform name for OFDM-9*/

    int    PER_11AG_12_PACKETS_NUM;                     /*!< Packet number of OFDM-12 sent to DUT for PER testing*/
    char   PER_11AG_12_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for OFDM-12*/

    int    PER_11AG_18_PACKETS_NUM;                     /*!< Packet number of OFDM-18 sent to DUT for PER testing*/
    char   PER_11AG_18_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for OFDM-18*/

    int    PER_11AG_24_PACKETS_NUM;                     /*!< Packet number of OFDM-24 sent to DUT for PER testing*/
    char   PER_11AG_24_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for OFDM-24*/

    int    PER_11AG_36_PACKETS_NUM;                     /*!< Packet number of OFDM-36 sent to DUT for PER testing*/
    char   PER_11AG_36_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for OFDM-36*/

    int    PER_11AG_48_PACKETS_NUM;                     /*!< Packet number of OFDM-48 sent to DUT for PER testing*/
    char   PER_11AG_48_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for OFDM-48*/

    int    PER_11AG_54_PACKETS_NUM;                     /*!< Packet number of OFDM-54 sent to DUT for PER testing*/
    char   PER_11AG_54_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for OFDM-54*/

	// 11p HALF
	int    PER_11P_HALF3_PACKETS_NUM;                      /*!< Packet number of HALF-3 sent to DUT for PER testing*/
	char   PER_11P_HALF3_WAVEFORM_NAME[MAX_BUFFER_SIZE];   /*!< Waveform name for HALF-3*/

	int    PER_11P_HALF4_5_PACKETS_NUM;                      /*!< Packet number of HALF-4.5 sent to DUT for PER testing*/
	char   PER_11P_HALF4_5_WAVEFORM_NAME[MAX_BUFFER_SIZE];   /*!< Waveform name for HALF-4.5*/

	int    PER_11P_HALF6_PACKETS_NUM;                     /*!< Packet number of HALF-6 sent to DUT for PER testing*/
	char   PER_11P_HALF6_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HALF-6*/

	int    PER_11P_HALF9_PACKETS_NUM;                     /*!< Packet number of HALF-9 sent to DUT for PER testing*/
	char   PER_11P_HALF9_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HALF-9*/

	int    PER_11P_HALF12_PACKETS_NUM;                     /*!< Packet number of HALF-12 sent to DUT for PER testing*/
	char   PER_11P_HALF12_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HALF-12*/

	int    PER_11P_HALF18_PACKETS_NUM;                     /*!< Packet number of HALF-18 sent to DUT for PER testing*/
	char   PER_11P_HALF18_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HALF-18*/

	int    PER_11P_HALF24_PACKETS_NUM;                     /*!< Packet number of HALF-24 sent to DUT for PER testing*/
	char   PER_11P_HALF24_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HALF-24*/

	int    PER_11P_HALF27_PACKETS_NUM;                     /*!< Packet number of HALF-27 sent to DUT for PER testing*/
	char   PER_11P_HALF27_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HALF-27*/
	// 11p QUAR
	int    PER_11P_QUAR1_5_PACKETS_NUM;                      /*!< Packet number of QUAR-1.5 sent to DUT for PER testing*/
	char   PER_11P_QUAR1_5_WAVEFORM_NAME[MAX_BUFFER_SIZE];   /*!< Waveform name for QUAR-1.5*/

	int    PER_11P_QUAR2_25_PACKETS_NUM;                      /*!< Packet number of QUAR-2.25 sent to DUT for PER testing*/
	char   PER_11P_QUAR2_25_WAVEFORM_NAME[MAX_BUFFER_SIZE];   /*!< Waveform name for QUAR-2.25*/

	int    PER_11P_QUAR3_PACKETS_NUM;                     /*!< Packet number of QUAR-3 sent to DUT for PER testing*/
	char   PER_11P_QUAR3_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for QUAR-3*/

	int    PER_11P_QUAR4_5_PACKETS_NUM;                     /*!< Packet number of QUAR-4.5 sent to DUT for PER testing*/
	char   PER_11P_QUAR4_5_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for QUAR-4.5*/

	int    PER_11P_QUAR6_PACKETS_NUM;                     /*!< Packet number of QUAR-6 sent to DUT for PER testing*/
	char   PER_11P_QUAR6_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for QUAR-6*/

	int    PER_11P_QUAR9_PACKETS_NUM;                     /*!< Packet number of QUAR-9 sent to DUT for PER testing*/
	char   PER_11P_QUAR9_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for QUAR-9*/

	int    PER_11P_QUAR12_PACKETS_NUM;                     /*!< Packet number of QUAR-12 sent to DUT for PER testing*/
	char   PER_11P_QUAR12_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for QUAR-12*/

	int    PER_11P_QUAR13_5_PACKETS_NUM;                     /*!< Packet number of QUAR-13.5 sent to DUT for PER testing*/
	char   PER_11P_QUAR13_5_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for QUAR-13.5*/

    // 11n HT20 mixed format
    int    PER_11N_HT20_MIXED_MCS0_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS0 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS0_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS0*/

    int    PER_11N_HT20_MIXED_MCS1_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS1 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS1_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS1*/

    int    PER_11N_HT20_MIXED_MCS2_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS2 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS2_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS2*/

    int    PER_11N_HT20_MIXED_MCS3_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS3 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS3_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS3*/

    int    PER_11N_HT20_MIXED_MCS4_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS4 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS4_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS4*/

    int    PER_11N_HT20_MIXED_MCS5_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS5 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS5_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS5*/

    int    PER_11N_HT20_MIXED_MCS6_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS6 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS6_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS6*/

    int    PER_11N_HT20_MIXED_MCS7_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS7 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS7_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS7*/

    int    PER_11N_HT20_MIXED_MCS8_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS8 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS8_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS8*/

    int    PER_11N_HT20_MIXED_MCS9_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS9 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS9_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS9*/

    int    PER_11N_HT20_MIXED_MCS10_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS10 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS10_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS10*/

    int    PER_11N_HT20_MIXED_MCS11_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS11 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS11_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS11*/

    int    PER_11N_HT20_MIXED_MCS12_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS12 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS12_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS12*/

    int    PER_11N_HT20_MIXED_MCS13_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS13 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS13_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS13*/

    int    PER_11N_HT20_MIXED_MCS14_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS14 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS14_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS14*/

    int    PER_11N_HT20_MIXED_MCS15_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS15 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS15_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS15*/
	
    int    PER_11N_HT20_MIXED_MCS16_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS16 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS16_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS16*/

	int    PER_11N_HT20_MIXED_MCS17_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS17 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS17_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS17*/

	int    PER_11N_HT20_MIXED_MCS18_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS18 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS18_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS18*/

	int    PER_11N_HT20_MIXED_MCS19_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS19 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS19_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS19*/

	int    PER_11N_HT20_MIXED_MCS20_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS20 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS20_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS20*/

	int    PER_11N_HT20_MIXED_MCS21_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS21 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS21_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS21*/

	int    PER_11N_HT20_MIXED_MCS22_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS22 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS22_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS22*/

	int    PER_11N_HT20_MIXED_MCS23_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS23 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS23_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS23*/

	int    PER_11N_HT20_MIXED_MCS24_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS24 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS24_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS24*/

	int    PER_11N_HT20_MIXED_MCS25_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS25 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS25_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS25*/

	int    PER_11N_HT20_MIXED_MCS26_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS26 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS26_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS26*/

	int    PER_11N_HT20_MIXED_MCS27_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS27 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS27_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS27*/

	int    PER_11N_HT20_MIXED_MCS28_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS28 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS28_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS28*/

	int    PER_11N_HT20_MIXED_MCS29_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS29 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS29_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS29*/

	int    PER_11N_HT20_MIXED_MCS30_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS30 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS30_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS30*/

	int    PER_11N_HT20_MIXED_MCS31_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format MCS31 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_MCS31_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format MCS31*/

	
	// 11n HT40 mixed format
    int    PER_11N_HT40_MIXED_MCS0_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS0 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS0_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS0*/

    int    PER_11N_HT40_MIXED_MCS1_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS1 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS1_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS1*/

    int    PER_11N_HT40_MIXED_MCS2_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS2 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS2_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS2*/

    int    PER_11N_HT40_MIXED_MCS3_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS3 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS3_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS3*/

    int    PER_11N_HT40_MIXED_MCS4_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS4 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS4_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS4*/

    int    PER_11N_HT40_MIXED_MCS5_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS5 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS5_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS5*/

    int    PER_11N_HT40_MIXED_MCS6_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS6 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS6_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS6*/

    int    PER_11N_HT40_MIXED_MCS7_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS7 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS7_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS7*/

    int    PER_11N_HT40_MIXED_MCS8_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS8 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS8_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS8*/

    int    PER_11N_HT40_MIXED_MCS9_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS9 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS9_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS9*/

    int    PER_11N_HT40_MIXED_MCS10_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS10 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS10_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS10*/

    int    PER_11N_HT40_MIXED_MCS11_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS11 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS11_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS11*/

    int    PER_11N_HT40_MIXED_MCS12_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS12 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS12_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS12*/

    int    PER_11N_HT40_MIXED_MCS13_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS13 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS13_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS13*/

    int    PER_11N_HT40_MIXED_MCS14_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS14 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS14_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS14*/

    int    PER_11N_HT40_MIXED_MCS15_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS15 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS15_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS15*/

    int    PER_11N_HT40_MIXED_MCS16_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS16 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS16_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS16*/

	int    PER_11N_HT40_MIXED_MCS17_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS17 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS17_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS18*/

	int    PER_11N_HT40_MIXED_MCS18_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS18 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS18_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS18*/

	int    PER_11N_HT40_MIXED_MCS19_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS19 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS19_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS19*/

	int    PER_11N_HT40_MIXED_MCS20_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS20 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS20_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS20*/

	int    PER_11N_HT40_MIXED_MCS21_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS21 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS21_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS21*/

	int    PER_11N_HT40_MIXED_MCS22_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS22 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS22_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS22*/

	int    PER_11N_HT40_MIXED_MCS23_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS23 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS23_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS23*/

	int    PER_11N_HT40_MIXED_MCS24_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS24 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS24_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS24*/

	int    PER_11N_HT40_MIXED_MCS25_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS25 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS25_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS25*/

	int    PER_11N_HT40_MIXED_MCS26_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS26 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS26_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS26*/

	int    PER_11N_HT40_MIXED_MCS27_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS27 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS27_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS27*/

	int    PER_11N_HT40_MIXED_MCS28_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS28 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS28_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS28*/

	int    PER_11N_HT40_MIXED_MCS29_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS29 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS29_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS29*/

	int    PER_11N_HT40_MIXED_MCS30_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS30 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS30_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS30*/

	int    PER_11N_HT40_MIXED_MCS31_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format MCS31 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_MCS31_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format MCS31*/


	// IQlite merge; Tracy Yu ; 2012-03-31
	 // 11n HT20 GREENFIELD format, long guard interval
    int    PER_11N_HT20_GREENFIELD_L_GI_MCS0_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS0 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS0*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS1_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS1 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS1*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS2_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS2 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS2*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS3_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS3 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS3*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS4_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS4 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS4*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS5_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS5 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS5*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS6_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS6 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS6*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS7_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS7 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS7*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS8_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS8 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS8*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS9_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS9 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS9*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS10_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS10 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS10*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS11_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS11 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS11*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS12_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS12 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS12*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS13_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS13 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS13*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS14_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS14 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS14*/

    int    PER_11N_HT20_GREENFIELD_L_GI_MCS15_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS15 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS15*/
	
    int    PER_11N_HT20_GREENFIELD_L_GI_MCS16_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS16 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS16*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS17_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS17 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS17*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS18_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS18 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS18*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS19_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS19 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS19*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS20_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS20 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS20*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS21_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS21 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS21*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS22_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS22 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS22*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS23_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS23 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS23*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS24_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS24 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS24*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS25_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS25 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS25*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS26_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS26 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS26*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS27_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS27 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS27*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS28_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS28 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS28*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS29_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS29 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS29*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS30_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS30 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS30*/

	int    PER_11N_HT20_GREENFIELD_L_GI_MCS31_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, long guard interval, MCS31 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, long guard interval, MCS31*/

	// IQlite merge; Tracy Yu ; 2012-03-31
	// 11n HT40 greenfield format, long guard interval
    int    PER_11N_HT40_GREENFIELD_L_GI_MCS0_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS0 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS0_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS0*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS1_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS1 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS1_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS1*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS2_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS2 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS2_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS2*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS3_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS3 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS3_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS3*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS4_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS4 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS4_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS4*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS5_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS5 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS5_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS5*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS6_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS6 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS6_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS6*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS7_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS7 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS7_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS7*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS8_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS8 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS8_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS8*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS9_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS9 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS9_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS9*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS10_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS10 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS10_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS10*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS11_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS11 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS11_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS11*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS12_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS12 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS12_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS12*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS13_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS13 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS13_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS13*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS14_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS14 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS14_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS14*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS15_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS15 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS15_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS15*/

    int    PER_11N_HT40_GREENFIELD_L_GI_MCS16_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS16 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS16_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS16*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS17_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS17 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS17_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS18*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS18_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS18 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS18_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS18*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS19_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS19 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS19_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS19*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS20_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS20 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS20_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS20*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS21_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS21 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS21_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS21*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS22_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS22 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS22_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS22*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS23_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS23 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS23_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS23*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS24_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS24 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS24_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS24*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS25_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS25 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS25_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS25*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS26_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS26 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS26_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS26*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS27_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS27 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS27_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS27*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS28_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS28 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS28_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS28*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS29_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS29 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS29_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS29*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS30_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS30 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS30_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS30*/

	int    PER_11N_HT40_GREENFIELD_L_GI_MCS31_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, long guard interval, MCS31 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_L_GI_MCS31_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, long guard interval, MCS31*/

	// IQlite merge; Tracy Yu ; 2012-03-31
	 // 11n HT20 mixed format, short guard interval
    int    PER_11N_HT20_MIXED_S_GI_MCS0_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS0 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS0_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS0*/

    int    PER_11N_HT20_MIXED_S_GI_MCS1_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS1 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS1_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS1*/

    int    PER_11N_HT20_MIXED_S_GI_MCS2_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS2 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS2_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS2*/

    int    PER_11N_HT20_MIXED_S_GI_MCS3_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS3 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS3_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS3*/

    int    PER_11N_HT20_MIXED_S_GI_MCS4_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS4 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS4_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS4*/

    int    PER_11N_HT20_MIXED_S_GI_MCS5_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS5 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS5_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS5*/

    int    PER_11N_HT20_MIXED_S_GI_MCS6_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS6 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS6_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS6*/

    int    PER_11N_HT20_MIXED_S_GI_MCS7_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS7 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS7_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS7*/

    int    PER_11N_HT20_MIXED_S_GI_MCS8_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS8 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS8_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS8*/

    int    PER_11N_HT20_MIXED_S_GI_MCS9_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS9 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS9_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS9*/

    int    PER_11N_HT20_MIXED_S_GI_MCS10_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS10 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS10_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS10*/

    int    PER_11N_HT20_MIXED_S_GI_MCS11_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS11 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS11_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS11*/

    int    PER_11N_HT20_MIXED_S_GI_MCS12_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS12 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS12_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS12*/

    int    PER_11N_HT20_MIXED_S_GI_MCS13_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS13 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS13_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS13*/

    int    PER_11N_HT20_MIXED_S_GI_MCS14_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS14 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS14_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS14*/

    int    PER_11N_HT20_MIXED_S_GI_MCS15_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS15 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS15_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS15*/
	
    int    PER_11N_HT20_MIXED_S_GI_MCS16_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS16 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS16_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS16*/

	int    PER_11N_HT20_MIXED_S_GI_MCS17_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS17 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS17_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS17*/

	int    PER_11N_HT20_MIXED_S_GI_MCS18_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS18 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS18_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS18*/

	int    PER_11N_HT20_MIXED_S_GI_MCS19_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS19 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS19_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS19*/

	int    PER_11N_HT20_MIXED_S_GI_MCS20_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS20 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS20_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS20*/

	int    PER_11N_HT20_MIXED_S_GI_MCS21_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS21 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS21_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS21*/

	int    PER_11N_HT20_MIXED_S_GI_MCS22_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS22 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS22_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS22*/

	int    PER_11N_HT20_MIXED_S_GI_MCS23_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS23 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS23_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS23*/

	int    PER_11N_HT20_MIXED_S_GI_MCS24_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS24 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS24_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS24*/

	int    PER_11N_HT20_MIXED_S_GI_MCS25_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS25 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS25_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS25*/

	int    PER_11N_HT20_MIXED_S_GI_MCS26_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS26 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS26_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS26*/

	int    PER_11N_HT20_MIXED_S_GI_MCS27_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS27 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS27_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS27*/

	int    PER_11N_HT20_MIXED_S_GI_MCS28_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS28 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS28_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS28*/

	int    PER_11N_HT20_MIXED_S_GI_MCS29_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS29 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS29_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS29*/

	int    PER_11N_HT20_MIXED_S_GI_MCS30_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS30 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS30_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS30*/

	int    PER_11N_HT20_MIXED_S_GI_MCS31_PACKETS_NUM;                     /*!< Packet number of HT20 mixed format, short guard interval, MCS31 sent to DUT for PER testing*/
    char   PER_11N_HT20_MIXED_S_GI_MCS31_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 mixed format, short guard interval, MCS31*/

	// IQlite merge; Tracy Yu ; 2012-03-31
	// 11n HT40 mixed format, short guard interval
    int    PER_11N_HT40_MIXED_S_GI_MCS0_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS0 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS0_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS0*/

    int    PER_11N_HT40_MIXED_S_GI_MCS1_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS1 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS1_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS1*/

    int    PER_11N_HT40_MIXED_S_GI_MCS2_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS2 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS2_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS2*/

    int    PER_11N_HT40_MIXED_S_GI_MCS3_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS3 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS3_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS3*/

    int    PER_11N_HT40_MIXED_S_GI_MCS4_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS4 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS4_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS4*/

    int    PER_11N_HT40_MIXED_S_GI_MCS5_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS5 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS5_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS5*/

    int    PER_11N_HT40_MIXED_S_GI_MCS6_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS6 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS6_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS6*/

    int    PER_11N_HT40_MIXED_S_GI_MCS7_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS7 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS7_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS7*/

    int    PER_11N_HT40_MIXED_S_GI_MCS8_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS8 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS8_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS8*/

    int    PER_11N_HT40_MIXED_S_GI_MCS9_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS9 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS9_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS9*/

    int    PER_11N_HT40_MIXED_S_GI_MCS10_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS10 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS10_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS10*/

    int    PER_11N_HT40_MIXED_S_GI_MCS11_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS11 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS11_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS11*/

    int    PER_11N_HT40_MIXED_S_GI_MCS12_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS12 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS12_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS12*/

    int    PER_11N_HT40_MIXED_S_GI_MCS13_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS13 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS13_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS13*/

    int    PER_11N_HT40_MIXED_S_GI_MCS14_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS14 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS14_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS14*/

    int    PER_11N_HT40_MIXED_S_GI_MCS15_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS15 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS15_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS15*/

    int    PER_11N_HT40_MIXED_S_GI_MCS16_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS16 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS16_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS16*/

	int    PER_11N_HT40_MIXED_S_GI_MCS17_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS17 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS17_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS18*/

	int    PER_11N_HT40_MIXED_S_GI_MCS18_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS18 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS18_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS18*/

	int    PER_11N_HT40_MIXED_S_GI_MCS19_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS19 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS19_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS19*/

	int    PER_11N_HT40_MIXED_S_GI_MCS20_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS20 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS20_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS20*/

	int    PER_11N_HT40_MIXED_S_GI_MCS21_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS21 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS21_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS21*/

	int    PER_11N_HT40_MIXED_S_GI_MCS22_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS22 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS22_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS22*/

	int    PER_11N_HT40_MIXED_S_GI_MCS23_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS23 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS23_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS23*/

	int    PER_11N_HT40_MIXED_S_GI_MCS24_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS24 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS24_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS24*/

	int    PER_11N_HT40_MIXED_S_GI_MCS25_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS25 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS25_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS25*/

	int    PER_11N_HT40_MIXED_S_GI_MCS26_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS26 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS26_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS26*/

	int    PER_11N_HT40_MIXED_S_GI_MCS27_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS27 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS27_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS27*/

	int    PER_11N_HT40_MIXED_S_GI_MCS28_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS28 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS28_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS28*/

	int    PER_11N_HT40_MIXED_S_GI_MCS29_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS29 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS29_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS29*/

	int    PER_11N_HT40_MIXED_S_GI_MCS30_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS30 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS30_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS30*/

	int    PER_11N_HT40_MIXED_S_GI_MCS31_PACKETS_NUM;                     /*!< Packet number of HT40 mixed format, short guard interval, MCS31 sent to DUT for PER testing*/
    char   PER_11N_HT40_MIXED_S_GI_MCS31_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 mixed format, short guard interval, MCS31*/

	// IQlite merge; Tracy Yu ; 2012-03-31
	 // 11n HT20 greenfield format, short guard interval
    int    PER_11N_HT20_GREENFIELD_S_GI_MCS0_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS0 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS0*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS1_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS1 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS1*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS2_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS2 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS2*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS3_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS3 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS3*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS4_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS4 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS4*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS5_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS5 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS5*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS6_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS6 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS6*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS7_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS7 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS7*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS8_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS8 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS8*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS9_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS9 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS9*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS10_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS10 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS10*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS11_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS11 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS11*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS12_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS12 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS12*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS13_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS13 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS13*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS14_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS14 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS14*/

    int    PER_11N_HT20_GREENFIELD_S_GI_MCS15_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS15 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS15*/
	
    int    PER_11N_HT20_GREENFIELD_S_GI_MCS16_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS16 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS16*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS17_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS17 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS17*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS18_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS18 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS18*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS19_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS19 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS19*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS20_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS20 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS20*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS21_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS21 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS21*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS22_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS22 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS22*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS23_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS23 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS23*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS24_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS24 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS24*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS25_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS25 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS25*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS26_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS26 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS26*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS27_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS27 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS27*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS28_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS28 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS28*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS29_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS29 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS29*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS30_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS30 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS30*/

	int    PER_11N_HT20_GREENFIELD_S_GI_MCS31_PACKETS_NUM;                     /*!< Packet number of HT20 greenfield format, short guard interval, MCS31 sent to DUT for PER testing*/
    char   PER_11N_HT20_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT20 greenfield format, short guard interval, MCS31*/

	// IQlite merge; Tracy Yu ; 2012-03-31
	// 11n HT40 greenfield format, short guard interval
    int    PER_11N_HT40_GREENFIELD_S_GI_MCS0_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS0 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS0_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS0*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS1_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS1 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS1_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS1*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS2_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS2 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS2_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS2*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS3_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS3 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS3_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS3*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS4_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS4 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS4_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS4*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS5_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS5 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS5_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS5*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS6_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS6 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS6_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS6*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS7_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS7 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS7_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS7*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS8_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS8 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS8_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS8*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS9_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS9 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS9_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS9*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS10_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS10 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS10_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS10*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS11_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS11 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS11_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS11*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS12_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS12 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS12_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS12*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS13_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS13 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS13_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS13*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS14_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS14 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS14_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS14*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS15_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS15 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS15_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS15*/

    int    PER_11N_HT40_GREENFIELD_S_GI_MCS16_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS16 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS16_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS16*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS17_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS17 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS17_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS18*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS18_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS18 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS18_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS18*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS19_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS19 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS19_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS19*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS20_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS20 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS20_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS20*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS21_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS21 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS21_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS21*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS22_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS22 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS22_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS22*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS23_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS23 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS23_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS23*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS24_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS24 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS24_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS24*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS25_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS25 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS25_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS25*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS26_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS26 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS26_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS26*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS27_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS27 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS27_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS27*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS28_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS28 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS28_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS28*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS29_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS29 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS29_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS29*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS30_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS30 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS30_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS30*/

	int    PER_11N_HT40_GREENFIELD_S_GI_MCS31_PACKETS_NUM;                     /*!< Packet number of HT40 greenfield format, short guard interval, MCS31 sent to DUT for PER testing*/
    char   PER_11N_HT40_GREENFIELD_S_GI_MCS31_WAVEFORM_NAME[MAX_BUFFER_SIZE];  /*!< Waveform name for HT40 greenfield format, short guard interval, MCS31*/

	// Let DUT keep transmit, default on.
	int	   DUT_KEEP_TRANSMIT;						/*!< A flag that to let Dut keep Tx until the configuration changed, 0: OFF, 1: ON, Default=ON */

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
    int    VSAs_PORT[MAX_TESTER_NUM];						/*!< IQTester VSAs port setting. Default=PORT_LEFT. */ 

	int    VSG_PORT;										/*!< IQTester VSG port setting. Default=PORT_LEFT*/
    int    VSGs_PORT[MAX_TESTER_NUM];						/*!< IQTester VSGs port setting. Default=PORT_LEFT. */ 
    double VSG_MAX_POWER_11B;								/*!< Max output power of VSG for 11B signal */
	double VSG_MAX_POWER_11G;								/*!< Max output power of VSG for 11G signal */
	double VSG_MAX_POWER_11N;								/*!< Max output power of VSG for 11N signal */
	
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

    // Power Measurement related parameters
    //RW-20090426: removed IQ_PM_METHOD
    //int    IQ_PM_METHOD;
    int    PM_IF_FREQ_SHIFT_MHZ;                    /*!< VSA center frequency shift in MHz in the case where DUT has a strong LO leakage */
    int    PM_AVERAGE;                              /*!< Number of packets for power averaging during power verification */
    int    PM_TRIGGER_DSSS_US;
    int    PM_TRIGGER_OFDM_US;
	int    PM_HALF_SAMPLE_INTERVAL_US;
	int    PM_QUAR_SAMPLE_INTERVAL_US;
    int    PM_LTS_START_OFFSET_US;				    // not available yet

    // Mask measurement related parameters
    int    MASK_FFT_AVERAGE;
    int    MASK_SMP_TM_OFDM_US;
    int    MASK_SMP_TM_DSSS_US;
	int    MASK_HALF_SAMPLE_INTERVAL_US;
	int    MASK_QUAR_SAMPLE_INTERVAL_US;
    int    MASK_INCL_PREAMBLE_DSSS;
    int    MASK_TIME_OFFSET_US;

	// Verify Spectrum related parameters
    int    SPECTRUM_SMP_TM_OFDM_US;
    int    SPECTRUM_SMP_TM_DSSS_US;
	int    SPECTRUM_HALF_SAMPLE_INTERVAL_US;
	int    SPECTRUM_QUAR_SAMPLE_INTERVAL_US;

    // EVM measurement related parameters
    int    EVM_AVERAGE;
    int    EVM_SYMBOL_NUM;
    int    EVM_PRE_TRIG_TIME_US;
    int    EVM_CAPTURE_TIME_11B_L_US;           /*!< Capture time for measuring 11b (long preamble) EVM.  =192+1000/11 + 3 */
    int    EVM_CAPTURE_TIME_11B_S_US;           /*!< Capture time for measuring 11b (short preamble) EVM. =96+1000/11 + 3 */
    int    EVM_CAPTURE_TIME_11AG_US;            /*!< Capture time for measuring 11ag EVM. =20+4*18+3 */
    int    EVM_CAPTURE_TIME_11N_MIXED_US;       /*!< Capture time for measuring 11n mixed format EVM. =20+12+4*4+4*18 + 3 */
    int    EVM_CAPTURE_TIME_11N_GREENFIELD_US;  /*!< Capture time for measuring 11n greenfield EVM. =24+4*4+4*18 + 3 */
	int    EVM_CAPTURE_TIME_11P_HALF_US;            /*!< Capture time for measuring 11p EVM. =40+8*18 */
	int    EVM_CAPTURE_TIME_11P_QUAR_US;            /*!< Capture time for measuring 11p EVM. =40+8*18 */
	//int    EVM_CAPTURE_PSDU_BYTE;				/*!< No. of PSDU byte to be captured */  //This is hard to use in IQwizard as it will assign same value for all data rate, Zhiyong May 17, 2011

	// Retry Options
	int    retryTestItem;				/*!< Specify no. of retry for individual test item if failed */
	int	   retryTestItemDuringRetry;	/*!< Specify whether a DUT reset function will be called when a retry occurs */
	int	   retryErrorItems;				/*!< Specify whether a retry an item if an error is returned, this will not affect whether an item is retried for a limit failure */

	//Read Limit files
	int    AUTO_READING_LIMIT;

    // 11n HT20 mixed format
	char   EVM_11N_HT20_MIXED_MCS0_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT20 mixed format MCS0*/
	char   EVM_11N_HT20_MIXED_MCS1_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT20 mixed format MCS1*/
	char   EVM_11N_HT20_MIXED_MCS2_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT20 mixed format MCS2*/
	char   EVM_11N_HT20_MIXED_MCS3_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT20 mixed format MCS3*/
	char   EVM_11N_HT20_MIXED_MCS4_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT20 mixed format MCS4*/
	char   EVM_11N_HT20_MIXED_MCS5_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT20 mixed format MCS5*/
	char   EVM_11N_HT20_MIXED_MCS6_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT20 mixed format MCS6*/
	char   EVM_11N_HT20_MIXED_MCS7_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT20 mixed format MCS7*/
    char   EVM_11N_HT20_MIXED_MCS8_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT20 mixed format MCS8*/
    char   EVM_11N_HT20_MIXED_MCS9_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT20 mixed format MCS9*/
    char   EVM_11N_HT20_MIXED_MCS10_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS10*/
    char   EVM_11N_HT20_MIXED_MCS11_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS11*/
    char   EVM_11N_HT20_MIXED_MCS12_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS12*/
    char   EVM_11N_HT20_MIXED_MCS13_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS13*/
    char   EVM_11N_HT20_MIXED_MCS14_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS14*/
    char   EVM_11N_HT20_MIXED_MCS15_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS15*/
	char   EVM_11N_HT20_MIXED_MCS16_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS16*/
	char   EVM_11N_HT20_MIXED_MCS17_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS17*/
	char   EVM_11N_HT20_MIXED_MCS18_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS18*/
	char   EVM_11N_HT20_MIXED_MCS19_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS19*/
	char   EVM_11N_HT20_MIXED_MCS20_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS20*/
	char   EVM_11N_HT20_MIXED_MCS21_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS21*/
	char   EVM_11N_HT20_MIXED_MCS22_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS22*/
	char   EVM_11N_HT20_MIXED_MCS23_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT20 mixed format MCS23*/

    // 11n HT40 mixed format
	char   EVM_11N_HT40_MIXED_MCS0_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT40 mixed format MCS0*/
	char   EVM_11N_HT40_MIXED_MCS1_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT40 mixed format MCS1*/
	char   EVM_11N_HT40_MIXED_MCS2_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT40 mixed format MCS2*/
	char   EVM_11N_HT40_MIXED_MCS3_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT40 mixed format MCS3*/
	char   EVM_11N_HT40_MIXED_MCS4_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT40 mixed format MCS4*/
	char   EVM_11N_HT40_MIXED_MCS5_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT40 mixed format MCS5*/
	char   EVM_11N_HT40_MIXED_MCS6_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT40 mixed format MCS6*/
	char   EVM_11N_HT40_MIXED_MCS7_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT40 mixed format MCS7*/
    char   EVM_11N_HT40_MIXED_MCS8_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT40 mixed format MCS8*/
    char   EVM_11N_HT40_MIXED_MCS9_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];   /*!< Reference file name for HT40 mixed format MCS9*/
    char   EVM_11N_HT40_MIXED_MCS10_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS10*/
    char   EVM_11N_HT40_MIXED_MCS11_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS11*/
    char   EVM_11N_HT40_MIXED_MCS12_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS12*/
    char   EVM_11N_HT40_MIXED_MCS13_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS13*/
    char   EVM_11N_HT40_MIXED_MCS14_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS14*/
    char   EVM_11N_HT40_MIXED_MCS15_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS15*/
	char   EVM_11N_HT40_MIXED_MCS16_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS16*/
	char   EVM_11N_HT40_MIXED_MCS17_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS17*/
	char   EVM_11N_HT40_MIXED_MCS18_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS18*/
	char   EVM_11N_HT40_MIXED_MCS19_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS19*/
	char   EVM_11N_HT40_MIXED_MCS20_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS20*/
	char   EVM_11N_HT40_MIXED_MCS21_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS21*/
	char   EVM_11N_HT40_MIXED_MCS22_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS22*/
	char   EVM_11N_HT40_MIXED_MCS23_REFERENCE_FILE_NAME[MAX_BUFFER_SIZE];  /*!< Reference file name for HT40 mixed format MCS23*/

    //RW-20090426: removed the below two parameters
    //int    IQ_PER_PACKET_NUM;
    //int    IQ_PER_RX_SENS_PACKET_NUM;

} WIFI_GLOBAL_SETTING;

typedef struct tagRecordParam
{
    // Mandatory Parameters
    int    FREQ_MHZ;                                /*! The center frequency (MHz). */  
    char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth. */
    char   DATA_RATE[MAX_BUFFER_SIZE];              /*! The data rate. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];      /*! The packet format of 11N(only). */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];     /*! The guard interval format of 11N(only). */// IQlite merge; Tracy Yu ; 2012-03-31
    double POWER_DBM;                               /*! The amplitude level in dBm. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */

    // DUT Parameters
    int    ANT1;                                     /*!< DUT ANT1 on/off. */
    int    ANT2;                                     /*!< DUT ANT2 on/off. */
    int    ANT3;                                     /*!< DUT ANT3 on/off. */
    int    ANT4;                                     /*!< DUT ANT4 on/off. */

	// Rx only
	int    FRAME_COUNT;                             /*! Number of count to send data packet */

} WIFI_RECORD_PARAM;

/*--------------------*
 * Internal Functions *
 *--------------------*/
int InitializeAllMIMOContainers(void);
void CleanupAllMIMOContainers(void);

int InitializeConnectTesterContainers(void);               // Needed by WiFi_Connect_IQTester
int InitializeDisconnectTesterContainers(void);            // Needed by WiFi_Disconnect_IQTester
int InitializeInsertDutContainers(void);                   // Needed by WiFi_Insert_Dut
int InitializeInitializeDutContainers(void);               // Needed by WiFi_Initialize_Dut
int InitializeRemoveDutContainers(void);                   // Needed by WiFi_Remove_Dut
int InitializeTXVerifyEvmContainers(void);                 // Needed by WiFi_TX_Verify_Evm
int InitializeRXVerifyPerContainers(void);                 // Needed by WiFi_RX_Verify_Per
int InitializeTXVerifyMaskContainers(void);                // Needed by WiFi_TX_Verify_Mask
void CleanupTXVerifyMaskContainers(void);					// Needed by WiFi_TX_Verify_Mask
// IQlite merge; Tracy Yu ; 2012-03-31
void CleanupTXConnectIQTester(void);
void CleanupTXDisconnectIQTester(void);                     // Needed by WiFi_Disconnect_IQTester
void CleanupGetSerialNumber(void);                          // Neede by WiFi_MiMo_Test_Internal
void CleanupGlobalSetting(void);                            // Needed by WiFi_Global_Setting
void CleanupInitializeDut(void);  
int InitializeRunDutCommandContainers(void);              // Needed by WiFi_Run_Dut_Command
void CleanupInsertDut(void);                                // Needed by WiFi_Insert_Dut
void CleanupLoadPathLossTable(void);                        // Needed by WiFi_Load_Path_Loss_Table
void CleanupPowerModeDut(void);                             // Needed by WiFi_Power_Mode_Dut
void CleanupReadBBRegister(void);                           // Needed by WiFi_Read_BB_Register
void CleanupReadEeprom(void);                               // Needed by WiFi_Read_Eeprom
void CleanupReadMacAddress(void);                           // Needed by WiFi_Read_Mac_Address
void CleanupReadMacRegister(void);							// Needed by WiFi_Read_Mac_Register
void CleanupReadRfRegister(void);							// Needed by WiFi_Read_RF_Register
void CleanupReadSocRegister(void);							// Needed by WiFi_Read_Soc_Register
void CleanupRemoveDut(void);								// Needed by WiFi_Remove_Dut
void CleanupRunExternalProgram(void);						// Needed by WiFi_Run_External_Program
void CleanupRXVerifyPer(void);								// Needed by WiFi_RX_Verify_Per
void CleanupTXCalibration(void);							// Needed by WiFi_TX_Calibration
void CleanupTXVerifyEvm(void);								// Needed by WiFi_TX_Verify_Evm
void CleanupTXVerifyMask(void);								// Needed by WiFi_TX_Verify_Mask
void CleanupTXVerifyPower(void);							// Needed by WiFi_TX_Verify_Power
void CleanupTXVerifySpectrum(void);							// Needed by WiFi_TX_Verify_Spectrum
void CleanupWriteBBRegister(void);							// Needed by WiFi_Write_BB_Register
void CleanupWriteEeprom(void);								// Needed by WiFi_Write_Eeprom
void CleanupWriteMacAddress(void);							// Needed by WiFi_Write_Mac_Address
void CleanupWriteMacRegister(void);							// Needed by WiFi_Write_Mac_Register
void CleanupWriteRFRegister(void);							// Needed by WiFi_Write_RF_Register
void CleanupWriteSocRegister(void);							// Needed by WiFi_Write_Soc_Register


//************************ HT40 HACK START ****************************
void EliminateSpike(double bufferRealY[], int spikeWidth, int spikeIndex); // HT40 HACK for WiFi_TX_Verify_Mask
//************************ HT40 HACK END ****************************
int InitializeTXVerifyPowerContainers(void);               // Needed by WiFi_TX_Verify_Power
int InitializeGlobalSettingContainers(void);               // Needed by WiFi_Global_Setting
int InitializeTXVerifyFlatnessContainers(void);            // Needed by WiFi_TX_Verify_Flatness
int InitializeReadEepromContainers(void);                  // Needed by WiFi_Read_Eeprom
int InitializeWriteEepromContainers(void);                 // Needed by WiFi_Write_Eeprom
int InitializeWriteBBRegisterContainers(void);             // Needed by WiFi_Write_BB_Register.cpp
int InitializeReadBBRegisterContainers(void);              // Needed by WiFi_Read_BB_Register.cpp
int InitializeWriteRFRegisterContainers(void);             // Needed by WiFi_Write_RF_Register.cpp
int InitializeReadRFRegisterContainers(void);              // Needed by WiFi_Read_RF_Register.cpp
int InitializeWriteMacRegisterContainers(void);            // Needed by WiFi_Write_Mac_Register.cpp
int InitializeReadMacRegisterContainers(void);             // Needed by WiFi_Read_Mac_Register.cpp
int InitializeWriteSocRegisterContainers(void);            // Needed by WiFi_Write_Soc_Register.cpp
int InitializeReadSocRegisterContainers(void);             // Needed by WiFi_Read_Soc_Register.cpp
int InitializeRunExternalProgramContainers(void);          // Needed by WiFi_Run_External_Program
int InitializeGetSerialNumberContainers(void);             // Needed by WiFi_Get_Serial_Number

int InitializeTXCalContainers(void);                       // Needed by WiFi_TX_Calibration
int InitializeWriteMacAddressContainers(void);             // Needed by WiFi_Write_Mac_Address
int InitializeReadMacAddressContainers(void);              // Needed by WiFi_Read_Mac_Address

int InitializePowerModeDutContainers(void);				// Needed by WiFi Current test
int InitializeloadPathLossTableContainers(void);
int InitializeInternalParameters(void);					
// Clear Memory
void ClearTxMaskReturn(void);  // IQlite merge; Tracy Yu ; 2012-03-31
int  ValidateIPAddress(char* ipAddress);
int  VerifyPowerMask (int txIndex, double *freqBuf, double *maskBuf, int maskBufCnt, int wifiMode);
int  VerifyOBW(double *maskBuf, int maskBufCnt, double obwPercentage, int &startIndex, int &stopIndex);
int  LoResultSpectralFlatness11AG(int* carrierNoUpA,
								  int* carrierNoUpB,
								  int* carrierNoLoA,
								  int* carrierNoLoB,
								  double* margin, 
								  int* leastMarginCarrier,
								  double* loLeakage, 
								  double* valueDbUpSectionA,
								  double* valueDbUpSectionB,
								  double* valueDbLoSectionA,
								  double* valueDbLoSectionB);

int  LoResultSpectralFlatness11N(int basedOnVsa,
								 int wifiMode,
								 int* carrierNoUpA,
								 int* carrierNoUpB,
								 int* carrierNoLoA,
								 int* carrierNoLoB,
								 double* failMargin,
								 int* leastMarginCarrier,
								 double* loLeakage,
								 double* valueDbUpSectionA,
								 double* valueDbUpSectionB,
								 double* valueDbLoSectionA,
								 double* valueDbLoSectionB,
								 int streamIndex);

double CalcVoltage (double data_i, double data_q);   // IQlite merge; Tracy Yu ; 2012-03-31
void ParseVersionString(const std::string &versionString, std::map<std::string, std::string> &versionMap);
int CheckConnectionVsaMasterAndAntennaPort( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap);
int CheckConnectionVsaAndAntennaPort( std::map<std::string, WIFI_SETTING_STRUCT>& inputMap, int *vsaMappingTx, int *portStatus);
double CalculateIQtoP(double data_i, double data_q);

#endif // end of #ifndef _WIFI_MIMO_TEST_INTERNAL_H_


#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include <math.h> // fmod on mac

using namespace std;

// Carrier number
#define CARRIER_0    0
#define CARRIER_1    1
#define CARRIER_2    2
#define CARRIER_6    6
#define CARRIER_16   16
#define CARRIER_17   17
#define CARRIER_26   26
#define CARRIER_28   28
#define CARRIER_31   31
#define CARRIER_32   32
#define CARRIER_33   33
#define CARRIER_42   42
#define CARRIER_43   43
#define CARRIER_58   58
#define CARRIER_70   70
#define CARRIER_84   84
#define CARRIER_85   85
#define CARRIER_95	 95
#define CARRIER_96	 96
#define CARRIER_97	 97
#define CARRIER_122  122


// These global variables are declared in WiFi_Test_Internal.cpp
extern TM_ID				g_WiFi_11ac_Test_ID;    
extern vDUT_ID				g_WiFi_11ac_Dut;
extern int					g_Tester_Type;
extern int					g_Tester_Number;
extern bool					g_vDutTxActived;
extern bool					g_dutConfigChanged;
extern WIFI_RECORD_PARAM	g_RecordedParam;
extern bool					 g_Tester_EnableVHT80;

// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_WiFi11ACGlobalSettingParam;
typedef struct tagDefaultMask
{ 
    double  freqOffset;
    double  deltaMaskPwr;
} DEFAULT_MASK_STRUCT;

static vector<DEFAULT_MASK_STRUCT> l_wifiMask_11b;
static vector<DEFAULT_MASK_STRUCT> l_wifiMask_11ag;
static vector<DEFAULT_MASK_STRUCT> l_wifiMask_11nHt20;
static vector<DEFAULT_MASK_STRUCT> l_wifiMask_11nHt40;
static vector<DEFAULT_MASK_STRUCT> l_wifiMask_11acBss20;
static vector<DEFAULT_MASK_STRUCT> l_wifiMask_11acBss40;
static vector<DEFAULT_MASK_STRUCT> l_wifiMask_11acBss80;
static vector<DEFAULT_MASK_STRUCT> l_wifiMask_11acBss160;
static vector<DEFAULT_MASK_STRUCT> l_wifiMask_11acBss80_80;

static void CheckFlatnessViolation_All(double deltaPower, int carrier, double lowLimit, double highLimit, double *sectionPower, int *sectionCarrier, double *failMargin, int *sectionCarrier2);
static int LoResultSpectralFlatness11AG_All( int* carrierNoUpA,
								  int* carrierNoUpB,
								  int* carrierNoLoA,
								  int* carrierNoLoB,
								  double* failMargin, 
								  int* leastMarginCarrier,
								  double* loLeakage, 
								  double* valueDbUpSectionA,
								  double* valueDbUpSectionB,
								  double* valueDbLoSectionA,
								  double* valueDbLoSectionB);
static int  LoResultSpectralFlatness11N_All(int basedOnVsa,
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
static int  LoResultSpectralFlatness11AC_All(int basedOnVsa,
								  int cbw,
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

static int VerifyPowerMask_All(double *freqBuf, double *maskBuf, int maskBufCnt, int wifiMode, int bssBW, int cbw);
static int VerifyOBW_All(double *maskBuf, int maskBufCnt, double obwPercentage, int &startIndex, int &stopIndex);



#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACtxMultiVerificationParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_11ACtxMultiVerificationReturnMap;

struct tagParam
{
    // Mandatory Parameters
	int    ENABLE_EVM;
	int    ENABLE_MASK;
	int    ENABLE_POWER;
	int    ENABLE_SPECTRUM;

    int    BSS_FREQ_MHZ_PRIMARY;                            /*!< For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment */
	int    BSS_FREQ_MHZ_SECONDARY;                          /*!< For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined */
	int    CH_FREQ_MHZ_PRIMARY_20MHz;                       /*!< The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ"   */
	int    CH_FREQ_MHZ;										/*!< The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used  */
	int    NUM_STREAM_11AC;							        /*!< The number of streams 11AC (Only). */
    char   BSS_BANDWIDTH[MAX_BUFFER_SIZE];                  /*!< The RF bandwidth of basic service set (BSS) to verify EVM. */
	char   CH_BANDWIDTH[MAX_BUFFER_SIZE];                   /*!< The RF channel bandwidth to verify EVM. */
    char   DATA_RATE[MAX_BUFFER_SIZE];              /*! The data rate to verify EVM. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT[MAX_BUFFER_SIZE];                  /*!< The packet format of 11AC and 11N. */
	char   GUARD_INTERVAL[MAX_BUFFER_SIZE];			/*! The guard interval format of 11N/AC(only). */
	char   STANDARD[MAX_BUFFER_SIZE];				/*!< The standard parameter used for signal analysis option or to discriminate the same data rates/package formats from different standards */
    double TX_POWER_DBM;                            /*! The output power to verify EVM. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	double OBW_PERCENTAGE;							/*! The percentage of OBW, default = 99% */
	
    double CAPTURE_LENGTH_OF_EVM_US;                /*! The capture length (time in us) to verify EVM. */
	double CAPTURE_LENGTH_OF_MASK_US;               /*! The capture length (time in us) to verify MASK. */
	double CAPTURE_LENGTH_OF_POWER_US;              /*! The capture length (time in us) to verify POWER. */
	double CAPTURE_LENGTH_OF_SPECTRUM_US;           /*! The capture length (time in us) to verify SPECTRUM. */

    double SAMPLING_TIME_US;                        /*! The sampling time to verify EVM. */
	double CORRECTION_FACTOR_11B;                   /*! The 802.11b(only) LO leakage correction factor. */

    // DUT Parameters
    int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on) */
    int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */
    int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */

	char   MASK_TEMPLATE[MAX_BUFFER_SIZE];           /*! Mask template */
} l_11ACtxMultiVerificationParam;

struct tagReturn
{	
	double TX_POWER_DBM;							   /*!< TX Power dBm Setting */

    // EVM Test Result 
    double EVM_AVG_DB;                  
    double EVM_AVG[MAX_DATA_STREAM];                   /*!< (Average) EVM test result on specific data stream. Format: EVM_AVG[SpecificStream] */
  
    // EVM_PK Test Result 
    double EVM_PK_DB;							   	   /*!< (11b only)EVM_PK over captured packets. */                  

    // POWER Test Result 
    double POWER_AVG_DBM;                  
    double POWER_AVG[MAX_DATA_STREAM];                 /*!< (Average) POWER test result on specific data stream. Format: POWER_AVG[SpecificStream] */
    double FREQ_ERROR_AVG;                             /*!< (Average) Frequency Error, unit is ppm */

	double AMP_ERR_DB;								   /*!< IQ Match Amplitude Error in dB. */
	double PHASE_ERR;								   /*!< IQ Match Phase Error. */
	double SYMBOL_CLK_ERR;							   /*!< Symbol clock error */
	double PHASE_NOISE_RMS_ALL;                        /*!< Frequency RMS Phase Noise. */ 

	double AMP_ERR_DB_STREAM[MAX_DATA_STREAM];		   /*!< IQ Match Amplitude Error in dB. per stream*/
	double PHASE_ERR_STREAM[MAX_DATA_STREAM];		   /*!< IQ Match Phase Error. per stream*/

	int	   SPATIAL_STREAM;	
	double DATA_RATE;
	double CABLE_LOSS_DB[MAX_DATA_STREAM];             /*! The path loss of test system. */

    // Mask Test Result 
    double MARGIN_DB_POSITIVE[MAX_POSITIVE_SECTION];		/*!< Margin to the mask, normally should be a negative number, if pass. */
	double MARGIN_DB_NEGATIVE[MAX_NEGATIVE_SECTION];		/*!< Margin to the mask, normally should be a negative number, if pass */
	double FREQ_AT_MARGIN_POSITIVE[MAX_POSITIVE_SECTION];	/*!< Point out the frequency offset at margin to the mask */
	double FREQ_AT_MARGIN_NEGATIVE[MAX_NEGATIVE_SECTION];	/*!< Point out the frequency offset at margin to the mask */
	double VIOLATION_PERCENT;								/*!< Percentage which fail the mask */
	vector <double> SPECTRUM_RAW_DATA_X;					/*!< spectrum X raw data  */
	vector <double> SPECTRUM_RAW_DATA_Y;					/*!< spectrum Y raw data  */

	// OBW Test Result
	double OBW_MHZ;

    // Flatness Test Result 
    double MARGIN_DB;              /*!< Margin to the flatness, normally should be a plus number */
    double VALUE_DB[MAX_DATA_STREAM];               /*!< Actually margin value to the average spectral center power. */
    double LO_LEAKAGE_DB;    
	int    LEAST_MARGIN_SUBCARRIER;    

	int    SUBCARRIER_LO_A;		/*!< Margin at which carrier in lower section A. */
	int    SUBCARRIER_LO_B;		/*!< Margin at which carrier in lower section B. */
	int    SUBCARRIER_UP_A;		/*!< Margin at which carrier in upper section A. */
	int    SUBCARRIER_UP_B;		/*!< Margin at which carrier in upper section B. */

	double VALUE_DB_LO_A;       /*!< Actually margin value to the average spectral center power in lower section A. */
	double VALUE_DB_LO_B;       /*!< Actually margin value to the average spectral center power in lower section B. */
	double VALUE_DB_UP_A;       /*!< Actually margin value to the average spectral center power in upper section A. */
	double VALUE_DB_UP_B;       /*!< Actually margin value to the average spectral center power in upper section B. */

	//channel list
	int    CH_FREQ_MHZ_PRIMARY_40MHz;       /*!< The center frequency (MHz) for primary 40 MHZ channel  */
	int    CH_FREQ_MHZ_PRIMARY_80MHz;       /*!< The center frequency (MHz) for primary 80 MHZ channel  */

    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_11ACtxMultiVerificationReturn;
#pragma endregion

#ifndef WIN32
int init11ACTXMultiVerificationContainer = Initialize11ACTXMultiVerificationContainers();
#endif

// These global variables/functions only for WiFi_TX_Multi_Verification.cpp
int Check11ACTxAllParameters( int *bssBW, \
							  int *cbw, \
							  int *bssPchannel, \
							  int *bssSchannel, \
							  int *wifiMode,  \
							  int *wifiStreamNum,  \
							  double *samplingTimeUs,  \
							  double *cableLossDb, \
							  double *peakToAvgRatio,  \
							  char* errorMsg );


//! WiFi TX Verify EVM
/*!
* Input Parameters
*
*  - Mandatory 
*      -# BSS_FREQ_MHZ_PRIMARY: For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment.
*	   -# BSS_FREQ_MHZ_SECONDARY;For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined.
*      -# DATA_RATE (string): The data rate to verify EVM
*      -# TX_POWER_DBM (double): The power (dBm) DUT is going to transmit at the antenna port
*	   -# CH_FREQ_MHZ_PRIMARY_20MHz: The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ".
*	   -# CH_FREQ_MHZ:The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used.
*	   -# BSS_BANDWIDTH[MAX_BUFFER_SIZE]: The RF bandwidth of basic service set (BSS) to verify EVM.
*	   -# CH_BANDWIDTH[MAX_BUFFER_SIZE]: The RF channel bandwidth to verify EVM.
*
* Return Values
*      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/

WIFI_11AC_TEST_API int WIFI_11AC_TX_Multi_Verification(void)
{
    int    err = ERR_OK;

    bool   analysisOK = false, captureOK  = false;     
    int    avgIteration = 0;
    int    bssPchannel = 0,bssSchannel = 0;
    int    dummyValue = 0;
    int    wifiMode = 0, wifiStreamNum = 0, VSAanylisisFormat = 0;
	int	   antSelection = 0;
	int    bssBW = 0, cbw = 0;
	int    VSAcaptureMode;  //0: 20MHz		1: 40MHz	2:80MHz		3: 160MHz		4: 80_80MHz
    
	//double samplingTimeUs = 0;
	double peakToAvgRatio = 0;
	double cableLossDb = 0;
	char   vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};


    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_11ACtxMultiVerificationReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_11ACtxMultiVerificationParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_11ACtxMultiVerificationReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	/*----------------------------------------------------------------------------------------
	 * Declare vectors for storing test results: vector< vector<double> >
	 * The outer vector is for TX streams, and the inner vector is for averaging of measurement
	 *-----------------------------------------------------------------------------------------*/
	vector< vector<double> >    evmAvgAll(MAX_DATA_STREAM, vector<double>(g_WiFi11ACGlobalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    rxRmsPowerDb(MAX_DATA_STREAM, vector<double>(g_WiFi11ACGlobalSettingParam.EVM_AVERAGE));
	vector< double >            evmPk(g_WiFi11ACGlobalSettingParam.EVM_AVERAGE);
	vector< double >            freqErrorHz(g_WiFi11ACGlobalSettingParam.EVM_AVERAGE);
	vector< double >            symbolClockErr(g_WiFi11ACGlobalSettingParam.EVM_AVERAGE);
	vector< vector<double> >    ampErrDb(MAX_DATA_STREAM, vector<double>(g_WiFi11ACGlobalSettingParam.EVM_AVERAGE));
	vector< vector<double> >    phaseErr(MAX_DATA_STREAM, vector<double>(g_WiFi11ACGlobalSettingParam.EVM_AVERAGE));
	vector< double >			phaseNoiseRms(g_WiFi11ACGlobalSettingParam.EVM_AVERAGE);

	try
	{
	   /*-----------------------------------------------------------*
		* Both g_WiFi_11ac_Test_ID and g_WiFi_11ac_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_WiFi_11ac_Test_ID<0 || g_WiFi_11ac_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] WiFi_Test_ID or WiFi_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_11ac_Test_ID, g_WiFi_11ac_Dut);
		}
		
		TM_ClearReturns(g_WiFi_11ac_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_11ACtxMultiVerificationParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Get input parameters return OK.\n");
		}

        // Error return of this function is irrelevant
        CheckDutTransmitStatus();

#pragma region Prepare input parameters

		err = Check11ACTxAllParameters( &bssBW, &cbw,&bssPchannel,&bssSchannel, &wifiMode, &wifiStreamNum, &l_11ACtxMultiVerificationParam.SAMPLING_TIME_US, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Prepare input parameters CheckTxEvmParameters() return OK.\n");
		}

#pragma endregion

		/*----------------------------*
		* Disable VSG output signal  *
		*----------------------------*/
		if (!g_useLpDut)
		{
			// make sure no signal is generated by the VSG
			err = ::LP_EnableVsgRF(0);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
			}
		}

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_11ac_Dut);

		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_FREQ_MHZ_PRIMARY",		l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_FREQ_MHZ_SECONDARY",		l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_SECONDARY);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ",				l_11ACtxMultiVerificationParam.CH_FREQ_MHZ);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_20MHz",	l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "NUM_STREAM_11AC",			l_11ACtxMultiVerificationParam.NUM_STREAM_11AC);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX1",						l_11ACtxMultiVerificationParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX2",						l_11ACtxMultiVerificationParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX3",						l_11ACtxMultiVerificationParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX4",						l_11ACtxMultiVerificationParam.TX4);
	
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_1",			l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[0]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_2",			l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[1]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_3",			l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[2]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_4",			l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[3]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "SAMPLING_TIME_US",			l_11ACtxMultiVerificationParam.SAMPLING_TIME_US);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "TX_POWER_DBM",				l_11ACtxMultiVerificationParam.TX_POWER_DBM);

		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "BSS_BANDWIDTH",			l_11ACtxMultiVerificationParam.BSS_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "CH_BANDWIDTH",			l_11ACtxMultiVerificationParam.CH_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "DATA_RATE",			l_11ACtxMultiVerificationParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "PACKET_FORMAT",		l_11ACtxMultiVerificationParam.PACKET_FORMAT);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "GUARD_INTERVAL",			l_11ACtxMultiVerificationParam.GUARD_INTERVAL);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "PREAMBLE",			l_11ACtxMultiVerificationParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "STANDARD",			l_11ACtxMultiVerificationParam.STANDARD);

		// the following parameters are not input parameters, why are they here?  6-20-2012 Jacky 
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_40MHz",	l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_80MHz",	l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
		if (( wifiMode== WIFI_11N_GF_HT40 ) || ( wifiMode== WIFI_11N_MF_HT40 )
			|| ( wifiMode== WIFI_11AC_GF_HT40 ) || ( wifiMode== WIFI_11AC_MF_HT40 )
			|| ( wifiMode== WIFI_11AC_VHT40 ) )
		{
			//vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ",			l_11ACtxMultiVerificationParam.CH_FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "PRIMARY_FREQ",		l_11ACtxMultiVerificationParam.CH_FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "SECONDARY_FREQ",		l_11ACtxMultiVerificationParam.CH_FREQ_MHZ+10);
		}
		else
		{
			//do nothing
		}		
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_BANDWIDTH",		bssBW);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_BANDWIDTH",		cbw);
	
		double chainGain = 0;
		int iChainGain = 0;
		chainGain = 10*log10((double)(l_11ACtxMultiVerificationParam.TX1 + l_11ACtxMultiVerificationParam.TX2 + l_11ACtxMultiVerificationParam.TX3 + l_11ACtxMultiVerificationParam.TX4));
		//Round off to thousandth
		iChainGain = (int)((chainGain * 1000) + .5);
		chainGain = (double)iChainGain / 1000;

		if ( (g_dutConfigChanged==true)||(g_vDutTxActived==false) )	
		{
			if ( g_vDutTxActived==true )
			{
			   /*-----------*
				*  Tx Stop  *
				*-----------*/
				err = ::vDUT_Run(g_WiFi_11ac_Dut, "TX_STOP");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_STOP) return OK.\n");
				}
			}
			else
			{
				// continue Dut configuration				
			}

			err = vDUT_Run(g_WiFi_11ac_Dut, "RF_SET_FREQ");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RF_SET_FREQ) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RF_SET_FREQ) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_11ac_Dut, "TX_SET_BW");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_SET_BW) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_SET_BW) return OK.\n");
			}
			err = vDUT_Run(g_WiFi_11ac_Dut, "TX_SET_DATA_RATE");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_SET_DATA_RATE) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_SET_DATA_RATE) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_11ac_Dut, "TX_SET_ANTENNA");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_SET_ANTENNA) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_SET_ANTENNA) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_11ac_Dut, "TX_PRE_TX");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_PRE_TX) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_PRE_TX) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_11ac_Dut, "TX_START");
			if ( ERR_OK!=err )
			{	
			   g_vDutTxActived = false;
			   // Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_START) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{
			   g_vDutTxActived = true;
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_START) return OK.\n");
			}

			// Delay for DUT settle
			if (0!=g_WiFi11ACGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
			{
				Sleep(g_WiFi11ACGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
			}
			else
			{
				// do nothing
			}
		}
		else
		{
			// g_dutConfigChanged==false, do nothing
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Since g_dutConfigChanged = false, skip Dut control section.\n");
		}
#pragma endregion

#pragma region Setup LP Tester and Capture
		/*--------------------*
		 * Setup IQTester VSA *
		 *--------------------*/
		err = ::LP_SetVsaAmplitudeTolerance(g_WiFi11ACGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);		
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to set VSA amplitude tolerance in dB.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_SetVsaAmplitudeTolerance(%.3f) return OK.\n", g_WiFi11ACGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		}

		err = ::LP_SetVsa(  l_11ACtxMultiVerificationParam.CH_FREQ_MHZ*1e6,
							l_11ACtxMultiVerificationParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGain,
							g_WiFi11ACGlobalSettingParam.VSA_PORT,
							0,
							g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_LEVEL_DB,
							g_WiFi11ACGlobalSettingParam.VSA_PRE_TRIGGER_TIME_US/1000000	
						  );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi_11AC] Fail to setup VSA, LP_SetVsa() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_SetVsa() return OK.\n");
		}
		
		/*------------------*
		 * Count Loop Times *
		 *------------------*/		 	
		int MASK_FFT_AVERAGE = 1; //TBD: Implement g_WiFi11ACGlobalSettingParam.MASK_FFT_AVERAGE = 3;
		int EVM_AVERAGE = g_WiFi11ACGlobalSettingParam.EVM_AVERAGE;

		/* Only use EVM test to get the TX power value. So ignore PM_AVERAGE		
		int loopTimes = 1;
		loopTimes = ( g_WiFi11ACGlobalSettingParam.PM_AVERAGE  > g_WiFi11ACGlobalSettingParam.EVM_AVERAGE) ? 
					((g_WiFi11ACGlobalSettingParam.PM_AVERAGE  > g_WiFi11ACGlobalSettingParam.MASK_FFT_AVERAGE) ?					
					  g_WiFi11ACGlobalSettingParam.PM_AVERAGE  : g_WiFi11ACGlobalSettingParam.MASK_FFT_AVERAGE) :
					((g_WiFi11ACGlobalSettingParam.EVM_AVERAGE > g_WiFi11ACGlobalSettingParam.MASK_FFT_AVERAGE) ? 
					  g_WiFi11ACGlobalSettingParam.EVM_AVERAGE : g_WiFi11ACGlobalSettingParam.MASK_FFT_AVERAGE); 
		*/
		
		/*------------------*
		 * Start While Loop *
		 *------------------*/
		avgIteration = 0;
		
		bool needInteration = l_11ACtxMultiVerificationParam.ENABLE_EVM || \
			  				  l_11ACtxMultiVerificationParam.ENABLE_MASK || \
							  l_11ACtxMultiVerificationParam.ENABLE_POWER || \
							  l_11ACtxMultiVerificationParam.ENABLE_SPECTRUM;

		while ( needInteration && (avgIteration < g_WiFi11ACGlobalSettingParam.EVM_AVERAGE || avgIteration < MASK_FFT_AVERAGE)) //TBD: Consider if the Average times <1
		{
			/*----------------------*
			*   VSA capture mode	*
			*   0: 20MHz			*
			*   0/1: 40MHz			*
			*	0: 80MHz			*
			*	3: 160MHz			*
			*	4: 80_80MHz			*
			*-----------------------*/
			if (l_11ACtxMultiVerificationParam.ENABLE_MASK && avgIteration < MASK_FFT_AVERAGE)
			{
				if ( g_Tester_EnableVHT80)   // 802.11ac ProtoType
				{
					if ( !g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW) // use BSS BW for spec mask
					{
						if ( bssBW == BW_80MHZ)					VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
						else if ( bssBW == BW_160MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //temporary 
						else if ( bssBW == BW_80_80MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 2; //temporary 
						else									VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;	
					}
					else		// use cbw for spec mask
					{
						if ( cbw == BW_80MHZ)					VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
						else if ( cbw == BW_160MHZ)				VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //temporary 
						else if ( cbw == BW_80_80MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 2; //temporary 
						else									VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;	
					}
				}
				else  // other testers
				{
					//if ( strstr (l_11ACtxMultiVerificationParam.PACKET_FORMAT,"11AC")) //not sure how other tester support 11ac
					  if ( strstr (l_11ACtxMultiVerificationParam.PACKET_FORMAT,PACKET_FORMAT_VHT))   // changed by bguo, 8/6/2012
					{
						if ( !g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW) // use BSS BW for spec mask
						{
							if ( bssBW == BW_80_80MHZ)				VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 2; //temporary 
							else if ( bssBW == BW_40MHZ)			VSAcaptureMode = IQV_HT_40_CAPTURE_TYPE;
							else if ( bssBW == BW_80MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE; // for  hndl->analysis = dynamic_cast<iqapiAnalysis *>(analysisVHT80);
							else if ( bssBW == BW_160MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //temporary 
							else									VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;			
						}
						else
						{
							if ( cbw == BW_80_80MHZ)				VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //temporary 
							else if ( cbw == BW_40MHZ)				VSAcaptureMode = IQV_HT_40_CAPTURE_TYPE;
							else if ( cbw == BW_80MHZ)				VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
							else if ( cbw == BW_160MHZ)				VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //temporary 
							else									VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;	
						}
					}
					else
					{
						if (cbw == BW_40MHZ)					VSAcaptureMode = IQV_HT_40_CAPTURE_TYPE;
						else									VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;
					}
				}
			}
			else // EVM & SPECTRUM
			{
				//if ( cbw == BW_80_80MHZ)			VSAcaptureMode = 4;
				//else if ( cbw == BW_40MHZ)			VSAcaptureMode = 1;
				//else if ( cbw == BW_80MHZ)			VSAcaptureMode = 0;
				//else if ( cbw == BW_160MHZ)			VSAcaptureMode = 3;
				//else								VSAcaptureMode = 0;	
				
				
				if ( cbw == BW_80_80MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 2; //not sure about this, need to double check
				else if ( cbw == BW_40MHZ)			VSAcaptureMode = IQV_HT_40_CAPTURE_TYPE;
				else if ( cbw == BW_80MHZ)			VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE; //we don't need wide band capture for EVM analysis.  //IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
				else if ( cbw == BW_160MHZ)			VSAcaptureMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE + 1; //not sure about this, need to double check. 
				else								VSAcaptureMode = IQV_NORMAL_CAPTURE_TYPE;	
			}

			if (avgIteration == 0) // Only need to set once
			{
				/*------------------------------------------*
				*	VSA analysis packet format				*					
				*	0: auto detect							*
				*	1: mixed format (11n)					*
				*	2: greenfield format (11n)				*
				*	3: legacy fomat ( 11a/g,11ac Non-HT)	*
				*	4: VHT format ( 11ac, VHT only)			*
				*-------------------------------------------*/

				if ( strstr ( l_11ACtxMultiVerificationParam.PACKET_FORMAT, PACKET_FORMAT_HT_MF))	// mixed format, 11n
				{
					VSAanylisisFormat = 1;
				}
				else if ( strstr ( l_11ACtxMultiVerificationParam.PACKET_FORMAT, PACKET_FORMAT_HT_GF))	// greenfield format, 11n
				{
					VSAanylisisFormat = 2;
				}
				else if ( strstr ( l_11ACtxMultiVerificationParam.PACKET_FORMAT, PACKET_FORMAT_VHT))	// 11ac
				{
					VSAanylisisFormat = 4;
				}
				else if ( strstr( l_11ACtxMultiVerificationParam.PACKET_FORMAT, PACKET_FORMAT_NON_HT))	// 11ag
				{
					VSAanylisisFormat = 3;
				}
				else
				{
					VSAanylisisFormat = 0;
				}
			}

			if (l_11ACtxMultiVerificationParam.ENABLE_MASK && avgIteration == 0) // Only need to set once for Mask
			{
				//-----------------------------------------------------
				// check mask template if specified
				//-----------------------------------------------------

				if (strlen(l_11ACtxMultiVerificationParam.MASK_TEMPLATE)>0)
				{
					char *ptr, buffer[MAX_BUFFER_SIZE];
					vector<DEFAULT_MASK_STRUCT> *maskTemplate;
					DEFAULT_MASK_STRUCT inputMask;
					int dataRate;

					TM_WiFiConvertDataRateNameToIndex(l_11ACtxMultiVerificationParam.DATA_RATE, &dataRate);      

					strcpy_s(buffer, l_11ACtxMultiVerificationParam.MASK_TEMPLATE);

					// check data rate to use the right mask template
					if (wifiMode == WIFI_11B)
					{
						maskTemplate = &l_wifiMask_11b;
					}
					else if (wifiMode == WIFI_11AG)
					{
						maskTemplate = &l_wifiMask_11ag;
					}
					else if (( wifiMode == WIFI_11N_MF_HT20 ) ||
						(wifiMode == WIFI_11N_GF_HT20))    //11n_HT20
					{
						maskTemplate = &l_wifiMask_11nHt20;
					}
					else if (( wifiMode == WIFI_11N_MF_HT40 ) ||
						(wifiMode == WIFI_11N_GF_HT40))          //11n_HT40
					{
						maskTemplate = &l_wifiMask_11nHt40;
					}
					else //11ac
					{
						if ( bssBW == BW_20MHZ)
						{
							maskTemplate = &l_wifiMask_11acBss20;
						}
						else if ( bssBW == BW_40MHZ)
						{
							maskTemplate = &l_wifiMask_11acBss40;
						}
						else if ( bssBW == BW_80MHZ)
						{
							maskTemplate = &l_wifiMask_11acBss80;
						}
						else if ( bssBW == BW_160MHZ)
						{
							maskTemplate = &l_wifiMask_11acBss160;
						}
						else 
						{
							maskTemplate = &l_wifiMask_11acBss80_80;
						}
					}

					// initialize mask template
					maskTemplate->clear();
					inputMask.deltaMaskPwr = 0;
					inputMask.freqOffset = 0;
					maskTemplate->push_back(inputMask);

					// add point to the template
					ptr = strtok(buffer, " \t");
					while(ptr)
					{
						inputMask.freqOffset = atof(ptr);
						ptr = strtok(NULL, " \t");
						if (ptr)
						{
							inputMask.deltaMaskPwr = atof(ptr);
							ptr = strtok(NULL, " \t");
							maskTemplate->push_back(inputMask);
						}
					}
				}
			}

			analysisOK = false;
			captureOK  = false;

		   /*----------------------------*
			* Perform Normal VSA capture *
			*----------------------------*/
			double sampleFreqHz = 160e6;

			/*------------------------------------------------------------*/
			/*For EVM Analysis, in HT20/HT40, using normal capture     ---*/
			/*------------------------------------------------------------*/
			//g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_TYPE
			err = ::LP_VsaDataCapture( l_11ACtxMultiVerificationParam.SAMPLING_TIME_US/1000000, g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_TYPE, sampleFreqHz, VSAcaptureMode );     
			if( ERR_OK!=err )	// capture is failed
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to capture signal at %d MHz. Retrying...\n", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ);

				double rxAmpl;
				LP_Agc(&rxAmpl, TRUE);	// do auto range on all testers

				err = ::LP_VsaDataCapture( l_11ACtxMultiVerificationParam.SAMPLING_TIME_US/1000000, g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_TYPE, sampleFreqHz, VSAcaptureMode );
				if( ERR_OK!=err )	// capture is failed
				{
				// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to capture signal at %d MHz.\n", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_VsaDataCapture() at %d MHz return OK.\n", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ);
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_VsaDataCapture() at %d MHz return OK.\n", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ);
			}
#pragma endregion
			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;
			if (1==g_WiFi11ACGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_All_SaveAlways", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ, l_11ACtxMultiVerificationParam.DATA_RATE, l_11ACtxMultiVerificationParam.CH_BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}
			/*------------------*
		 	 *  EVM Analysis    *
			 *------------------*/
			if ((l_11ACtxMultiVerificationParam.ENABLE_EVM && avgIteration < EVM_AVERAGE) ||
			    (l_11ACtxMultiVerificationParam.ENABLE_SPECTRUM && avgIteration < MASK_FFT_AVERAGE))
			{
				switch(wifiMode)
				{

#pragma region Analysis_802_11b
					case WIFI_11B:   
					{
						if (l_11ACtxMultiVerificationParam.ENABLE_SPECTRUM && avgIteration < MASK_FFT_AVERAGE)
						{
							// [Case 01]: 802.11b Analysis
							// TODO: The official way is to have a continuously transmitting DUT, no preamble, 2 Mbps, 01 data sequence. 
							if ( (0==strcmp(l_11ACtxMultiVerificationParam.DATA_RATE, "DSSS-2")) && (1==g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE) )
							{
								// Perform FFT analysis
								err = ::LP_AnalyzeFFT();   
								if ( ERR_OK!=err )
								{
									// Fail Analysis, thus save capture (Signal File) for debug
									sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ, l_11ACtxMultiVerificationParam.DATA_RATE, l_11ACtxMultiVerificationParam.CH_BANDWIDTH);
									WiFiSaveSigFile(sigFileNameBuffer);
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_AnalyzeFFT() return error.\n");
									throw logMessage;
								}
								else
								{
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_AnalyzeFFT() return OK.\n");
								}
							}
							break;
						}							
							
						// First, Check 11B correction factor  
						if (0==l_11ACtxMultiVerificationParam.CORRECTION_FACTOR_11B)
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Need correction factor (non-zero) for 802.11B LO leakage measurement.\n");
						}
						else
						{
						// Settings: No equalization, DC removal off, classic EVM method
							err = ::LP_Analyze80211b(   g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_EQ_TAPS,
														g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG,
														g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_METHOD_11B
														);
							if (ERR_OK!=err)
							{	// Fail Analysis, thus save capture (Signal File) for debug
								sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211b_Failed", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ, l_11ACtxMultiVerificationParam.DATA_RATE, l_11ACtxMultiVerificationParam.CH_BANDWIDTH);
								WiFiSaveSigFile(sigFileNameBuffer);
		
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_Analyze80211b() return error.\n");
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_Analyze80211b() return OK.\n");
							}
						}
					}

					break;
			
#pragma endregion
#pragma region Analysis_802_11ag

					case WIFI_11AG: 
					case WIFI_11AC_NON_HT:
					{
						// [Case 02]: 802.11a/g Analysis
						err = ::LP_Analyze80211ag(  g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
							g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
							g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
							g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
							g_WiFi11ACGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK
							);
						if ( ERR_OK!=err )
						{
							// Fail Analysis, thus save capture (Signal File) for debug
							sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211ag_Failed", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ, l_11ACtxMultiVerificationParam.DATA_RATE, l_11ACtxMultiVerificationParam.CH_BANDWIDTH);
							WiFiSaveSigFile(sigFileNameBuffer);
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_Analyze80211ag() return error.\n");					
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_Analyze80211ag() return OK.\n");
						}
					}

					break;

#pragma endregion
#pragma region Analysis_802_11n

					case WIFI_11N_GF_HT20:
					case WIFI_11N_GF_HT40:
					case WIFI_11N_MF_HT20:
					case WIFI_11N_MF_HT40: 
					case WIFI_11AC_MF_HT20:
					case WIFI_11AC_GF_HT20:
					case WIFI_11AC_MF_HT40:
					case WIFI_11AC_GF_HT40:
					{
						// [Case 03]: MIMO Analysis
						char referenceFileName[MAX_BUFFER_SIZE], analyzeMimoType[MAX_BUFFER_SIZE], analyzeMimoMode[MAX_BUFFER_SIZE];
	
						switch( g_Tester_Type )
						{
						case IQ_View:
							sprintf_s(analyzeMimoType, "11n");                        
							if ( wifiStreamNum==WIFI_ONE_STREAM )  // MCS0 ~ MCS7
							{        
								sprintf_s(analyzeMimoMode, "nxn");
								sprintf_s(referenceFileName, ""); 
							}
							else // MCS8 ~ MCS15 or more, need MIMO reference File *.ref
							{
								sprintf_s(analyzeMimoMode, "composite");
								err = GetDefaultWaveformFileName(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PATH, 
									"iqref", 
									wifiMode, 
									l_11ACtxMultiVerificationParam.NUM_STREAM_11AC,
									cbw, 
									l_11ACtxMultiVerificationParam.DATA_RATE, 
									l_11ACtxMultiVerificationParam.PREAMBLE, 
									l_11ACtxMultiVerificationParam.PACKET_FORMAT,
									l_11ACtxMultiVerificationParam.GUARD_INTERVAL, 
									referenceFileName, 
									MAX_BUFFER_SIZE);					
								if ( ERR_OK!=err )
								{
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to get reference file name.\n");
									throw logMessage;
								}
								else
								{
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] GetWaveformFileName(%s) return OK.\n", referenceFileName);
								}
							}
							break;
	
						case IQ_nxn:
							sprintf_s(analyzeMimoType, "EWC");
							sprintf_s(analyzeMimoMode, "nxn");
							sprintf_s(referenceFileName, "");
							break; 
	
						default:
							sprintf_s(analyzeMimoType, "EWC");
							sprintf_s(analyzeMimoMode, "nxn");
							sprintf_s(referenceFileName, "");
							break;
						}
	
						//Set Frequency Correction for 802.11n analysis
						err = ::LP_SetAnalysisParameterInteger("Analyze80211n", "frequencyCorr", g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION);
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_SetAnalysisParameterInteger() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_SetAnalysisParameterInteger() return OK.\n");
						}
	
						err = ::LP_Analyze80211n( analyzeMimoType,
							analyzeMimoMode,
							g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_PHASE_CORR,    
							g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR,    
							g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING,  
							g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_DECODE_PSDU, 
							g_WiFi11ACGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST,
							referenceFileName,
							VSAanylisisFormat);     
						if ( ERR_OK!=err )
						{
							// Fail Analysis, thus save capture (Signal File) for debug
							sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_AnalyzeMIMO_Failed", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ, l_11ACtxMultiVerificationParam.DATA_RATE, l_11ACtxMultiVerificationParam.CH_BANDWIDTH);
							WiFiSaveSigFile(sigFileNameBuffer);
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_Analyze80211n() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_Analyze80211n() return OK.\n");
						}
					}
	
					break;

#pragma endregion
#pragma region Analysis_802_11ac_mimo

					case WIFI_11AC_VHT20:
					case WIFI_11AC_VHT40:
					case WIFI_11AC_VHT80:
					case WIFI_11AC_VHT160:
					case WIFI_11AC_VHT80_80:           
					{
						// [Case 04]: 802.11AC Analysis
						char referenceFileName[MAX_BUFFER_SIZE], analyzeMimoMode[MAX_BUFFER_SIZE];

						switch( g_Tester_Type )
						{
						case IQ_View:                        
							if ( l_11ACtxMultiVerificationParam.NUM_STREAM_11AC==1 )  // MCS0 ~ MCS7
							{        
								sprintf_s(analyzeMimoMode, "nxn");
								sprintf_s(referenceFileName, ""); 
							}
							else // MCS8 ~ MCS15 or more, need MIMO reference File *.ref
							{
								sprintf_s(analyzeMimoMode, "composite");
								err = GetDefaultWaveformFileName(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PATH, 
									"iqref", 
									wifiMode, 
									l_11ACtxMultiVerificationParam.NUM_STREAM_11AC,
									cbw, 
									l_11ACtxMultiVerificationParam.DATA_RATE, 
									l_11ACtxMultiVerificationParam.PREAMBLE, 
									l_11ACtxMultiVerificationParam.PACKET_FORMAT,
									l_11ACtxMultiVerificationParam.GUARD_INTERVAL, 
									referenceFileName, 
									MAX_BUFFER_SIZE);							
								if ( ERR_OK!=err )
								{
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to get reference file name.\n");
									throw logMessage;
								}
								else
								{
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] GetWaveformFileName(%s) return OK.\n", referenceFileName);
								}
							}
							break;

						case IQ_nxn:
							sprintf_s(analyzeMimoMode, "nxn");
							sprintf_s(referenceFileName, "");
							break; 

						default:
							sprintf_s(analyzeMimoMode, "nxn");
							sprintf_s(referenceFileName, "");
							break;
						}

						//Set Frequency Correction for 802.11ac analysis
						err = ::LP_SetAnalysisParameterInteger("Analyze80211ac", "frequencyCorr", g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_FREQUENCY_CORRELATION);
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_SetAnalysisParameterInteger() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_SetAnalysisParameterInteger() return OK.\n");
						}

						err = ::LP_Analyze80211ac( analyzeMimoMode,
								g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_PHASE_CORR,    
								g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_SYM_TIMING_CORR,    
								g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_AMPLITUDE_TRACKING,  
								g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_DECODE_PSDU, 
								g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_FULL_PACKET_CHANNEL_EST,
								g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_FREQUENCY_CORRELATION,
								referenceFileName,
								VSAanylisisFormat);   

						//err = ::LP_Analyze80211ac();
						if ( ERR_OK!=err )
						{
							// Fail Analysis, thus save capture (Signal File) for debug
							sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_11AC_TX_Evm_AnalyzeMIMO_Failed", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ, l_11ACtxMultiVerificationParam.DATA_RATE, l_11ACtxMultiVerificationParam.CH_BANDWIDTH);
							WiFiSaveSigFile(sigFileNameBuffer);
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_Analyze80211ac() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_Analyze80211ac() return OK.\n");
						}

					}
					break;
#pragma endregion
					default:
						err = ERR_INVALID_ANALYSIS_TYPE;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Invalid WiFi mode used for Analysis!!\n");
						throw logMessage;
				}
			}

			/*-----------------------------*
			 *  Retrieve analysis Results  *
			 *-----------------------------*/
			if (l_11ACtxMultiVerificationParam.ENABLE_EVM && avgIteration < EVM_AVERAGE)
			{

				char sTestDetail[MAX_BUFFER_SIZE] = {'\0'};
				sprintf_s(sTestDetail, MAX_BUFFER_SIZE, "WiFi_TX_Multi_Verification_%d_%s", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ, l_11ACtxMultiVerificationParam.DATA_RATE);
	
				analysisOK = true;

#pragma region Retrieve_EVM_802_11abg_Results

				// Store the result, includes EVM, power etc.              
				if ( (wifiMode==WIFI_11B)||(wifiMode==WIFI_11AG) )        // 802.11a/b/g, but not n mode, only one stream
				{
					// Number of spatial streams
					l_11ACtxMultiVerificationReturn.SPATIAL_STREAM = 1;
	
					// EVM
					evmAvgAll[0][avgIteration] = ::LP_GetScalarMeasurement("evmAll",0); 
					if ( -99.00 >= evmAvgAll[0][avgIteration] )
					{
						analysisOK = false;
						evmAvgAll[0][avgIteration] = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(evmAll) return error.\n");
						throw logMessage;
					}
					
					// Peak EVM
					if (wifiMode==WIFI_11B)
					{
						evmPk[avgIteration] = ::LP_GetScalarMeasurement("evmPk", 0);
						if ( -99.00 >= evmPk[avgIteration] )
						{
							analysisOK = false;
							evmPk[avgIteration] = NA_NUMBER;
							WiFiSaveSigFile(sTestDetail);
							err = -1;
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(evmPk) return error.\n");
							throw logMessage;
						}
					}
	
					// Power 
					rxRmsPowerDb[0][avgIteration] = ::LP_GetScalarMeasurement("rmsPowerNoGap",0);
					if ( -99.00 >= rxRmsPowerDb[0][avgIteration] )
					{
						analysisOK = false;
						rxRmsPowerDb[0][avgIteration] = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(rmsPowerNoGap) return error.\n");
						throw logMessage;
					}
					else
					{
						// One stream data rate, should be only one antenna is ON	
						int antenaOrder = 0;
						err = CheckAntennaOrderByStream(l_11ACtxMultiVerificationReturn.SPATIAL_STREAM, l_11ACtxMultiVerificationParam.TX1, l_11ACtxMultiVerificationParam.TX2, l_11ACtxMultiVerificationParam.TX3, l_11ACtxMultiVerificationParam.TX4, &antenaOrder);
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] CheckAntennaOrderByStream() return error.\n");					
							throw logMessage;
						}
	
						rxRmsPowerDb[0][avgIteration] = rxRmsPowerDb[0][avgIteration] + l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[antenaOrder-1];
					}
	
					// Frequency Error
					freqErrorHz[avgIteration] = ::LP_GetScalarMeasurement("freqErr", 0); 
	
					// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
					ampErrDb[0][avgIteration] = ::LP_GetScalarMeasurement("ampErrDb", 0); 
	
					// IQ Match Phase Error.
					phaseErr[0][avgIteration] = ::LP_GetScalarMeasurement("phaseErr", 0);
	
					// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
					phaseNoiseRms[avgIteration] = ::LP_GetScalarMeasurement("rmsPhaseNoise",0);
	
					// Datarate 						
					if (wifiMode==WIFI_11B)
					{
						l_11ACtxMultiVerificationReturn.DATA_RATE = ::LP_GetScalarMeasurement("bitRateInMHz", 0);
					}
					else
					{                        
						l_11ACtxMultiVerificationReturn.DATA_RATE = ::LP_GetScalarMeasurement("dataRate", 0); 
					}
	
				}
#pragma endregion
#pragma region Retrieve_EVM_802_11nac_Results
				else /*if ( (wifiMode==WIFI_11N_GF_HT20)||(wifiMode==WIFI_11N_GF_HT40) ||
					(wifiMode==WIFI_11N_MF_HT20)||(wifiMode==WIFI_11N_MF_HT40) )       */
					// WIFI_11AC_VHT20 - WIFI_11AC_NON_HT
				{
					// Number of spatial streams
					l_11ACtxMultiVerificationReturn.SPATIAL_STREAM = (int) ::LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);
	
					if (g_Tester_Type==IQ_View)
					{
						for(int i=0;i<l_11ACtxMultiVerificationReturn.SPATIAL_STREAM;i++)
						{
	                        // Get EVM test result
	                        evmAvgAll[i][avgIteration] = ::LP_GetScalarMeasurement("evmAvgAll", 0); // only can get EVM_ALL, means index always = 0
							if ( -99.00 >= evmAvgAll[i][avgIteration] )
							{
								analysisOK = false;
								evmAvgAll[i][avgIteration] = NA_NUMBER;
								WiFiSaveSigFile(sTestDetail);
								err = -1;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(evmAvgAll) return error.\n");
								throw logMessage;
							}
	                        // Get Power test result
	                        rxRmsPowerDb[i][avgIteration] = ::LP_GetScalarMeasurement("rxRmsPowerDb", i);
							if ( -99.00 >= rxRmsPowerDb[i][avgIteration] )
							{
								analysisOK = false;
								rxRmsPowerDb[i][avgIteration] = NA_NUMBER;
								WiFiSaveSigFile(sTestDetail);
								err = -1;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(rxRmsPowerDb) return error.\n");
								throw logMessage;
							}
							else
							{
	                            //The current code will work correctly with 4x4 MIMO.  
	  							int antenaOrder = 0;
								err = CheckAntennaOrderByStream(i+1, l_11ACtxMultiVerificationParam.TX1, l_11ACtxMultiVerificationParam.TX2, l_11ACtxMultiVerificationParam.TX3, l_11ACtxMultiVerificationParam.TX4, &antenaOrder);
								if ( ERR_OK!=err )
								{
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] CheckAntennaOrderByStream() return error.\n");					
									throw logMessage;
								}
	
								rxRmsPowerDb[i][avgIteration] = rxRmsPowerDb[i][avgIteration] + l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[antenaOrder-1];
							}  
	
							// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
							ampErrDb[i][avgIteration] = ::LP_GetScalarMeasurement("IQImbal_amplDb", i); // Index always = 0
							phaseErr[i][avgIteration] = ::LP_GetScalarMeasurement("IQImbal_phaseDeg",i);
	                    }
					}
					else	// g_Tester_Type == IQnxn
					{
						for(int i=0;i<l_11ACtxMultiVerificationReturn.SPATIAL_STREAM;i++)
						{
							// EVM 
							evmAvgAll[i][avgIteration] = ::LP_GetScalarMeasurement("evmAvgAll",i); 
							if ( -99.00 >= evmAvgAll[i][avgIteration] )
							{
								analysisOK = false;
								evmAvgAll[i][avgIteration] = NA_NUMBER;
								WiFiSaveSigFile(sTestDetail);
								err = -1;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(evmAvgAll) return error.\n");
								throw logMessage;
							}
	
							// Power
							rxRmsPowerDb[i][avgIteration] = ::LP_GetScalarMeasurement("rxRmsPowerDb", i*(l_11ACtxMultiVerificationReturn.SPATIAL_STREAM+1));
							if ( -99.00 >= rxRmsPowerDb[i][avgIteration] )
							{
								analysisOK = false;
								rxRmsPowerDb[i][avgIteration] = NA_NUMBER;
								WiFiSaveSigFile(sTestDetail);
								err = -1;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(rxRmsPowerDb) return error.\n");
								throw logMessage;
							}
							else
							{
	                            //The current code will work correctly with 4x4 MIMO.  
	  							int antenaOrder = 0;
								err = CheckAntennaOrderByStream(i+1, l_11ACtxMultiVerificationParam.TX1, l_11ACtxMultiVerificationParam.TX2, l_11ACtxMultiVerificationParam.TX3, l_11ACtxMultiVerificationParam.TX4, &antenaOrder);
								if ( ERR_OK!=err )
								{
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] CheckAntennaOrderByStream() return error.\n");					
									throw logMessage;
								}
	
								rxRmsPowerDb[i][avgIteration] = rxRmsPowerDb[i][avgIteration] + l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[antenaOrder-1];
							}
	
							// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
							ampErrDb[i][avgIteration] = ::LP_GetScalarMeasurement("IQImbal_amplDb", i); // Index always = 0
							phaseErr[i][avgIteration] = ::LP_GetScalarMeasurement("IQImbal_phaseDeg",i);
						}
					}
					// Frequency Error
					freqErrorHz[avgIteration] = ::LP_GetScalarMeasurement("freqErrorHz",0);
	
					// Datarate
					l_11ACtxMultiVerificationReturn.DATA_RATE = ::LP_GetScalarMeasurement("rateInfo_dataRateMbps", 0); 
	
					// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
					phaseNoiseRms[avgIteration] = ::LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll", 0);
	
					// Symbol clock error
					symbolClockErr[avgIteration] = ::LP_GetScalarMeasurement("symClockErrorPpm", 0);
				}   
#pragma endregion

			}

			if (l_11ACtxMultiVerificationParam.ENABLE_SPECTRUM && avgIteration < MASK_FFT_AVERAGE) //TBD: Should handle the MASK_FFT_AVERAGE
			{
				/*---------------------*
				 *  SPECTRUM Results   *
				 *---------------------*/
				switch(wifiMode)
				{

#pragma region Retrieve_SPEC_802_11b_Results
				case WIFI_11B:                           
					{   // [Case 01]: 802.11b Analysis
						if ( (0==strcmp(l_11ACtxMultiVerificationParam.DATA_RATE, "DSSS-2")) && 
							 (1==g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_FIXED_01_DATA_SEQUENCE) )
						{
							// Store the result
							double bufferRealX[MAX_BUFFER_SIZE], bufferImagX[MAX_BUFFER_SIZE];
							double bufferRealY[MAX_BUFFER_SIZE], bufferImagY[MAX_BUFFER_SIZE];
							int    bufferSizeX = ::LP_GetVectorMeasurement("x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE);
							int    bufferSizeY = ::LP_GetVectorMeasurement("y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE);

							if ( (0>=bufferSizeX)||(0>=bufferSizeY) )
							{
								err = -1;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Can not retrieve FFT result, result length = 0.\n");
								throw logMessage;
							}
							else
							{                         					
								/*--------------------------*
								 *  Looking for peak power  *
								 *--------------------------*/
								double peakPower = NA_NUMBER;
								for ( int i=0;i<bufferSizeY;i++ )
								{
									if ( bufferRealY[i]>peakPower )  
									{   // store the current peak power and keep searching
										peakPower = bufferRealY[i];
									}
									else
									{
										// keep searching...
									}
								}

								if ( NA_NUMBER>=peakPower )
								{
									err = -1;
									peakPower = NA_NUMBER;
									LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Can not find out the peak power.\n");
									throw logMessage;
								}
								else
								{
									// Verify DC Leakage 
									int zeroIndex = (int)(bufferSizeX/2) - 3;					
									while(true)
									{						
										if (bufferRealX[zeroIndex]==0)
										{
											// this is the offset "ZERO", break the loop.
											break;
										}
										else
										{
											zeroIndex++;
										}

										if ( zeroIndex>((int)(bufferSizeX/2)+3) )
										{
											err = -1;
											LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to find out the offset zero of the FFT results.\n");
											throw logMessage;
										}
									}

									double deltaPower = pow(10,(bufferRealY[zeroIndex]/LOG_10)) - pow(10,(peakPower/LOG_10));
									if (0!=deltaPower)
									{
										deltaPower = fabs(deltaPower);	//Avoid negative value in log10()
										l_11ACtxMultiVerificationReturn.LO_LEAKAGE_DB =  10.0*(log10(deltaPower));					
									}
									else
									{
										err = -1;
										l_11ACtxMultiVerificationReturn.LO_LEAKAGE_DB = 0;
										LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to retrieve 11B loLeakageDb.\n");
										throw logMessage;
									}
								}
							}
						}
						else
						{
							// Store the result
							double bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
							int    bufferSize = ::LP_GetVectorMeasurement("loLeakageDb", bufferReal, bufferImag, MAX_BUFFER_SIZE);

							if (bufferSize>0)
							{
								l_11ACtxMultiVerificationReturn.LO_LEAKAGE_DB = bufferReal[0] + l_11ACtxMultiVerificationParam.CORRECTION_FACTOR_11B;
							}
							else
							{
								err = -1;
								l_11ACtxMultiVerificationReturn.LO_LEAKAGE_DB = 0;
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to retrieve 11B loLeakageDb.\n");
								throw logMessage;
							}
						}
					}
					break;
#pragma endregion
#pragma region Retrieve_SPEC_802_11ag_legacy_Results
				case WIFI_11AG: 
				case WIFI_11AC_NON_HT:
					{   // [Case 02]: 802.11a/g Analysis
						err = LoResultSpectralFlatness11AG_All( &l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_A, 
							&l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_B,
							&l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_A,
							&l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_B,
							&l_11ACtxMultiVerificationReturn.MARGIN_DB, 
							&l_11ACtxMultiVerificationReturn.LEAST_MARGIN_SUBCARRIER,
							&l_11ACtxMultiVerificationReturn.LO_LEAKAGE_DB,
							&l_11ACtxMultiVerificationReturn.VALUE_DB_UP_A,
							&l_11ACtxMultiVerificationReturn.VALUE_DB_UP_B,
							&l_11ACtxMultiVerificationReturn.VALUE_DB_LO_A,
							&l_11ACtxMultiVerificationReturn.VALUE_DB_LO_B
						);
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LoResultSpectralFlatness11AG_All() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LoResultSpectralFlatness11AG_All() return OK.\n");
						}
					}
					break;

#pragma endregion
#pragma region Retrieve_SPEC_802_11nac_HT20_40_Results
				case WIFI_11N_GF_HT20:
				case WIFI_11N_GF_HT40:
				case WIFI_11N_MF_HT20:
				case WIFI_11N_MF_HT40: 
				case WIFI_11AC_MF_HT20:
				case WIFI_11AC_GF_HT20:
				case WIFI_11AC_MF_HT40:
				case WIFI_11AC_GF_HT40:                      
					{   // [Case 03]: MIMO Analysis

							err = LoResultSpectralFlatness11N_All(  1, 
																wifiMode,
																&l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_A,
																&l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_B,
																&l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_A,
																&l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_B,
																&l_11ACtxMultiVerificationReturn.MARGIN_DB, 
																&l_11ACtxMultiVerificationReturn.LEAST_MARGIN_SUBCARRIER, 
																&l_11ACtxMultiVerificationReturn.LO_LEAKAGE_DB,
																&l_11ACtxMultiVerificationReturn.VALUE_DB_UP_A,
																&l_11ACtxMultiVerificationReturn.VALUE_DB_UP_B,
																&l_11ACtxMultiVerificationReturn.VALUE_DB_LO_A,
																&l_11ACtxMultiVerificationReturn.VALUE_DB_LO_B,
																0
															   );
							if ( ERR_OK!=err )
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LoResultSpectralFlatness11N_All() return error.\n");
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LoResultSpectralFlatness11N_All() return OK.\n");
							}
						//}
					}
					break;

		#pragma endregion
#pragma region Retrieve_SPEC_802_11ac_HT20_160_MIMO_Results

				case WIFI_11AC_VHT20:
				case WIFI_11AC_VHT40:
				case WIFI_11AC_VHT80:
				case WIFI_11AC_VHT160:
				case WIFI_11AC_VHT80_80:           
					{   // [Case 04]: 802.11AC Analysis
						
						err = LoResultSpectralFlatness11AC_All(  1, 
							cbw,
							&l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_A,
							&l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_B,
							&l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_A,
							&l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_B,
							&l_11ACtxMultiVerificationReturn.MARGIN_DB, 
							&l_11ACtxMultiVerificationReturn.LEAST_MARGIN_SUBCARRIER, 
							&l_11ACtxMultiVerificationReturn.LO_LEAKAGE_DB,
							&l_11ACtxMultiVerificationReturn.VALUE_DB_UP_A,
							&l_11ACtxMultiVerificationReturn.VALUE_DB_UP_B,
							&l_11ACtxMultiVerificationReturn.VALUE_DB_LO_A,
							&l_11ACtxMultiVerificationReturn.VALUE_DB_LO_B,
							0
							);
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac] LoResultSpectralFlatness11AC_All() return error.\n");
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac] LoResultSpectralFlatness11AC_All() return OK.\n");
						}

					}
					break;
#pragma endregion
				default:
					err = ERR_INVALID_ANALYSIS_TYPE;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Invalid WiFi mode used for Analysis!!\n");
					throw logMessage;
				}
			}

			/*------------------*
		 	 *  MASK Analysis   *
			 *------------------*/
			if (l_11ACtxMultiVerificationParam.ENABLE_MASK && avgIteration < MASK_FFT_AVERAGE)
			{
#pragma region Analysis_Mask
				//if (strstr(l_11ACtxMultiVerificationParam.PACKET_FORMAT,"11AC") ) // 11AC
				if (strstr(l_11ACtxMultiVerificationParam.PACKET_FORMAT,PACKET_FORMAT_VHT) )  //changed by bguo, 8/6/2012
				{
					if ( !g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW )  // use BSS BW for spec mask
					{

						if (( bssBW== BW_40MHZ) && ( ! g_Tester_EnableVHT80))
						{
							//AnalyzeHT40
							err = ::LP_SetAnalysisParameterInteger("AnalyzeHT40", "vsaNum", 1);
						}
						// VHT8160,VHT80_80 are not supported yet

						else if ( bssBW == BW_80MHZ)
						{
							//AnalyzeVHT80
							err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT80Mask", "vsaNum", 1);
						}
						else if (bssBW == BW_160MHZ)
						{
							//AnalyzeVHT160
							err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT160Mask", "vsaNum", 1);
						}
						else if ( bssBW == BW_80_80MHZ)
						{
							//AnalyzeVHT80_80
							err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT80_80Mask", "vsaNum", 1);
						}
						else   //bssBW = 20,40MHz
						{
							err = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", 1);

						}
					}
					else		// use CBW for spec mask
					{
						if (( cbw== BW_40MHZ) && ( ! g_Tester_EnableVHT80))
						{
							//AnalyzeHT40
							err = ::LP_SetAnalysisParameterInteger("AnalyzeHT40", "vsaNum", 1);
						}
						// VHT8160,VHT80_80 are not supported yet

						else if ( cbw == BW_80MHZ)
						{
							//AnalyzeVHT80
							err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT80Mask", "vsaNum", 1);
						}
						else if (cbw == BW_160MHZ)
						{
							//AnalyzeVHT160
							err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT160Mask", "vsaNum", 1);
						}
						else if ( cbw == BW_80_80MHZ)
						{
							//AnalyzeVHT80_80
							err = ::LP_SetAnalysisParameterInteger("AnalyzeVHT80_80Mask", "vsaNum", 1);
						}
						else   //bssBW = 20,40MHz
						{
							err = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", 1);

						}
					}
				}
				else   // 802.11 ab/b/g/n.  mask according to cbw
				{
					if ((cbw == BW_40MHZ)&& ( ! g_Tester_EnableVHT80))
					{
						//AnalyzeHT40
						err = ::LP_SetAnalysisParameterInteger("AnalyzeHT40", "vsaNum", 1);
					}
					else
					{
						err = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", 1);
					}
				}

				if ( ERR_OK!=err )
				{
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d", "WiFi_Tx_Mask_Set_Analysis_Param_Failed", l_11ACtxMultiVerificationParam.CH_FREQ_MHZ);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11ac] Set Analyze FFT parameter return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11ac] Set Analyze FFT parameter return OK.\n");
				}
				//Step2: Perform Analysis   --802.11 ac Prototype

				//if (strstr(l_11ACtxMultiVerificationParam.PACKET_FORMAT,"11AC")) // 11AC, mask according to BSS BW
				if (strstr(l_11ACtxMultiVerificationParam.PACKET_FORMAT,PACKET_FORMAT_VHT) )  //changed by bguo, 8/6/2012
				{
					if ( !g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW )  // use BSS BW for spec mask
					{

						if ( bssBW == BW_80MHZ)
						{
							//AnalyzeVHT80
							err = ::LP_AnalyzeVHT80Mask();
						}
						/*else if (bssBW == BW_160MHZ)
						{
						//AnalyzeVHT160
						err = ::LP_AnalyzeVHT160Mask();
						}
						else if ( bssBW== BW_80_80MHZ)
						{
						//AnalyzeVHT80
						err = ::LP_AnalyzeVHT80_80Mask();
						}*/

						else  // BW20, BW40
						{
							err = ::LP_AnalyzeFFT();	
						}
					}
					else   
					{
						if ( cbw == BW_80MHZ)
						{
							//AnalyzeVHT80
							err = ::LP_AnalyzeVHT80Mask();
						}
						/*else if (cbw == BW_160MHZ)
						{
						//AnalyzeVHT160
						err = ::LP_AnalyzeVHT160Mask();
						}
						else if ( cbw == BW_80_80MHZ)
						{
						//AnalyzeVHT80
						err = ::LP_AnalyzeVHT80_80Mask();
						}*/

						else  // BW20
						{
							err = ::LP_AnalyzeFFT();	
						}
					}
				}
				else		// 802.11 ab/b/g/n.  mask according to cbw
				{
					if ((cbw == BW_40MHZ) && ( ! g_Tester_EnableVHT80))
					{
						//AnalyzeHT40
						err = ::LP_AnalyzeHT40Mask();
					}
					else
					{
						err = ::LP_AnalyzeFFT();	
					}
				}

				////-------------------------------------------------------------------------
				//// if Tx port is disable and analysis fail, then just fill NA values
				////-------------------------------------------------------------------------
				//if (!txEnabled[vsaMappingTx[vsaAnalysisIndex]-1] && ERR_OK!=err)
				//{
				//	for (int x=0;x<MAX_POSITIVE_SECTION;x++)
				//	{
				//		l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[x]		= NA_DOUBLE;
				//		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[x]	= NA_DOUBLE;
				//		l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[x]		= NA_DOUBLE;
				//		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[x] = NA_DOUBLE;
				//	}
				//	err = ERR_OK;
				//}
				//else
				//{
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY, l_11ACtxMultiVerificationParam.DATA_RATE, l_11ACtxMultiVerificationParam.BSS_BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Analyze FFT return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Analyze FFT return OK.\n");
				}

#pragma endregion
			}

			/*-----------------------------*
			 *  Retrieve analysis Results  *
			 *-----------------------------*/

			if (l_11ACtxMultiVerificationParam.ENABLE_MASK && avgIteration < MASK_FFT_AVERAGE) //TBD: Should handle the MASK_FFT_AVERAGE
			{
#pragma region Retrieve MASK Results
				analysisOK = true;

				// Store the result

				double *bufferRealX = NULL, *bufferImagX = NULL, *bufferRealY=NULL, *bufferImagY=NULL; 
				bufferRealX = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));
				bufferImagX = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));
				bufferRealY = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));
				bufferImagY = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));

				int    bufferSizeX = ::LP_GetVectorMeasurement("x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE*8);
				int    bufferSizeY = ::LP_GetVectorMeasurement("y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE*8);


				if ( (0>=bufferSizeX)||(0>=bufferSizeY) )
				{
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Can not retrieve FFT result, result length = 0.\n");

					if (bufferRealX) free(bufferRealX);
					if (bufferImagX) free(bufferImagX);
					if (bufferRealY) free(bufferRealY);
					if (bufferImagY) free(bufferImagY);


					throw logMessage;
				}                   
				else
				{
					// clear raw spectrum data vector
					l_11ACtxMultiVerificationReturn.SPECTRUM_RAW_DATA_X.clear();
					l_11ACtxMultiVerificationReturn.SPECTRUM_RAW_DATA_Y.clear();

					//Return Mask Raw Data
					for(int i=0;i<bufferSizeX;i++)
					{
						l_11ACtxMultiVerificationReturn.SPECTRUM_RAW_DATA_X.push_back(bufferRealX[i]);  
					}
					for(int i=0;i<bufferSizeY;i++)
					{
						l_11ACtxMultiVerificationReturn.SPECTRUM_RAW_DATA_Y.push_back(bufferRealY[i]);  
					}

					// Verify MASK
					int pass = VerifyPowerMask_All( bufferRealX, bufferRealY, bufferSizeY, wifiMode, bssBW, cbw );

					// Verify OBW
					int startIndex = 0, stopIndex = 0;
					pass = VerifyOBW_All(bufferRealY, bufferSizeY, (l_11ACtxMultiVerificationParam.OBW_PERCENTAGE/100), startIndex, stopIndex);

					double OBW_FREQ_START_MHZ = bufferRealX[startIndex] / 1000000;
					double OBW_FREQ_STOP_MHZ  = bufferRealX[stopIndex]  / 1000000;
					l_11ACtxMultiVerificationReturn.OBW_MHZ  = OBW_FREQ_STOP_MHZ - OBW_FREQ_START_MHZ;

					if (bufferRealX) free(bufferRealX);
					if (bufferImagX) free(bufferImagX);
					if (bufferRealY) free(bufferRealY);
					if (bufferImagY) free(bufferImagY);
				}
#pragma endregion
			
			}

			avgIteration++;
		}   // End - avgIteration

		// This is a special case, only if "DUT_KEEP_TRANSMIT=0" then must do TX_STOP manually.
		if ( (g_WiFi11ACGlobalSettingParam.DUT_KEEP_TRANSMIT==0)&&(g_vDutTxActived==true) )
		{
		   /*-----------*
			*  Tx Stop  *
			*-----------*/
			err = ::vDUT_Run(g_WiFi_11ac_Dut, "TX_STOP");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_STOP) return OK.\n");
			}
		}

#pragma region Averaging and Saving Test Result
	   /*----------------------------------*
		* Averaging and Saving Test Result *
		*----------------------------------*/
		if ( (ERR_OK==err) && captureOK && analysisOK )
		{
			double dummyMax, dummyMin;
			//double dummyAvg[MAX_DATA_STREAM];

			for(int i=0;i<l_11ACtxMultiVerificationReturn.SPATIAL_STREAM;i++)
			{
				// Average EVM test result
				::AverageTestResult(&evmAvgAll[i][0], avgIteration, LOG_20, l_11ACtxMultiVerificationReturn.EVM_AVG[i], dummyMax, dummyMin);

				// Average Power test result
				::AverageTestResult(&rxRmsPowerDb[i][0], avgIteration, LOG_10, l_11ACtxMultiVerificationReturn.POWER_AVG[i], dummyMax, dummyMin);

				// Average Amp Error test result
				//::AverageTestResult(&ampErrDb[i][0], avgIteration, LOG_10, dummyAvg[i], dummyMax, dummyMin);
				::AverageTestResult(&ampErrDb[i][0], avgIteration, LOG_10, l_11ACtxMultiVerificationReturn.AMP_ERR_DB_STREAM[i], dummyMax, dummyMin);

				// Average Phase Error
				::AverageTestResult(&phaseErr[i][0], avgIteration, Linear, l_11ACtxMultiVerificationReturn.PHASE_ERR_STREAM[i], dummyMax, dummyMin);

			}
			::AverageTestResult(l_11ACtxMultiVerificationReturn.EVM_AVG, l_11ACtxMultiVerificationReturn.SPATIAL_STREAM, LOG_20, l_11ACtxMultiVerificationReturn.EVM_AVG_DB, dummyMax, dummyMin);
			::AverageTestResult(l_11ACtxMultiVerificationReturn.POWER_AVG, l_11ACtxMultiVerificationReturn.SPATIAL_STREAM, LOG_10, l_11ACtxMultiVerificationReturn.POWER_AVG_DBM, dummyMax, dummyMin);
			::AverageTestResult(l_11ACtxMultiVerificationReturn.AMP_ERR_DB_STREAM, l_11ACtxMultiVerificationReturn.SPATIAL_STREAM, LOG_10, l_11ACtxMultiVerificationReturn.AMP_ERR_DB, dummyMax, dummyMin);
			::AverageTestResult(l_11ACtxMultiVerificationReturn.PHASE_ERR_STREAM,  l_11ACtxMultiVerificationReturn.SPATIAL_STREAM, Linear, l_11ACtxMultiVerificationReturn.PHASE_ERR, dummyMax, dummyMin);
	
			if (wifiMode==WIFI_11B)
			{
				// (11b only) Average EVM_PK test result
				::AverageTestResult(&evmPk[0], avgIteration, LOG_20, dummyMax, l_11ACtxMultiVerificationReturn.EVM_PK_DB, dummyMin);
			}

			// Average RMS Phase Noise test result
			::AverageTestResult(&phaseNoiseRms[0], avgIteration, Linear, l_11ACtxMultiVerificationReturn.PHASE_NOISE_RMS_ALL, dummyMax, dummyMin);
			
			// Average Freq Error test result
			::AverageTestResult(&freqErrorHz[0], avgIteration, Linear, l_11ACtxMultiVerificationReturn.FREQ_ERROR_AVG, dummyMax, dummyMin);

			// Average Symbol clock Error test result
			::AverageTestResult(&symbolClockErr[0], avgIteration, Linear, l_11ACtxMultiVerificationReturn.SYMBOL_CLK_ERR, dummyMax, dummyMin);

			// Transfer the Freq Error result unit to ppm
			l_11ACtxMultiVerificationReturn.FREQ_ERROR_AVG = l_11ACtxMultiVerificationReturn.FREQ_ERROR_AVG/l_11ACtxMultiVerificationParam.CH_FREQ_MHZ;
			//l_11ACtxMultiVerificationReturn.FREQ_ERROR_MAX = l_11ACtxMultiVerificationReturn.FREQ_ERROR_MAX/l_11ACtxMultiVerificationParam.CH_FREQ_MHZ;
			//l_11ACtxMultiVerificationReturn.FREQ_ERROR_MIN = l_11ACtxMultiVerificationReturn.FREQ_ERROR_MIN/l_11ACtxMultiVerificationParam.CH_FREQ_MHZ;
		}
		else
		{
			// do nothing
		}
#pragma endregion 

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			sprintf_s(l_11ACtxMultiVerificationReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			l_11ACtxMultiVerificationReturn.TX_POWER_DBM = l_11ACtxMultiVerificationParam.TX_POWER_DBM;
			ReturnTestResults(l_11ACtxMultiVerificationReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_11ACtxMultiVerificationReturn.ERROR_MESSAGE, msg);

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_11ac_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
    }
    catch(...)
    {
		ReturnErrorMessage(l_11ACtxMultiVerificationReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
		err = -1;

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_11ac_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
    }

	// This is a special case, only when some error occur before the TX_STOP. 
	// This case will take care by the error handling, but must do TX_STOP manually.
	//if ( g_vDutTxActived )
	//{
	//	vDUT_Run(g_WiFi_11ac_Dut, "TX_STOP");
	//}
	//else
	//{
	//	// do nothing
	//}

	// Free memory
	evmAvgAll.clear();
	evmPk.clear();
	rxRmsPowerDb.clear();
	freqErrorHz.clear();

    return err;
}

int Initialize11ACTXMultiVerificationContainers(void)
{
    DEFAULT_MASK_STRUCT dummyStruct;

    /*----------------------*
     *  Construct the Mask  *
     *----------------------*/
    // Default Mask of 802.11b 
    dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11b.push_back(dummyStruct);
    dummyStruct.freqOffset   = 11;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11b.push_back(dummyStruct);
    dummyStruct.freqOffset   = 11;
    dummyStruct.deltaMaskPwr = -30;
    l_wifiMask_11b.push_back(dummyStruct);
    dummyStruct.freqOffset   = 22;
    dummyStruct.deltaMaskPwr = -30;
    l_wifiMask_11b.push_back(dummyStruct);
    dummyStruct.freqOffset   = 22;
    dummyStruct.deltaMaskPwr = -50;
    l_wifiMask_11b.push_back(dummyStruct);

    // Default Mask of 802.11ag 
    dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11ag.push_back(dummyStruct);
    dummyStruct.freqOffset   = 9;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11ag.push_back(dummyStruct);
    dummyStruct.freqOffset   = 11;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11ag.push_back(dummyStruct);
    dummyStruct.freqOffset   = 20;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11ag.push_back(dummyStruct);
    dummyStruct.freqOffset   = 30;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11ag.push_back(dummyStruct);

    // Default Mask of 802.11n HT20
    dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11nHt20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 9;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11nHt20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 11;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11nHt20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 20;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11nHt20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 30;
    dummyStruct.deltaMaskPwr = -45;
    l_wifiMask_11nHt20.push_back(dummyStruct);

    // Default Mask of 802.11n HT40
    dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11nHt40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 19;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11nHt40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 21;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11nHt40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 40;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11nHt40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 60;
    dummyStruct.deltaMaskPwr = -45;
    l_wifiMask_11nHt40.push_back(dummyStruct);

	//Default Mask of 802.11ac BSS20MHz
	dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss20.push_back(dummyStruct);
	dummyStruct.freqOffset   = 9;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 11;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11acBss20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 20;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11acBss20.push_back(dummyStruct);
    dummyStruct.freqOffset   = 30;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11acBss20.push_back(dummyStruct);

	//Default Mask of 802.11ac BSS40MHz
	dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss40.push_back(dummyStruct);
	dummyStruct.freqOffset   = 19;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 21;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11acBss40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 40;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11acBss40.push_back(dummyStruct);
    dummyStruct.freqOffset   = 60;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11acBss40.push_back(dummyStruct);

	//Default Mask of 802.11ac BSS80MHz
	dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss80.push_back(dummyStruct);
	dummyStruct.freqOffset   = 39;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 41;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11acBss80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 80;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11acBss80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 120;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11acBss80.push_back(dummyStruct);


	//Default Mask of 802.11ac BSS160MHz
	dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss160.push_back(dummyStruct);
	dummyStruct.freqOffset   = 79;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss160.push_back(dummyStruct);
    dummyStruct.freqOffset   = 81;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11acBss160.push_back(dummyStruct);
    dummyStruct.freqOffset   = 160;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11acBss160.push_back(dummyStruct);
    dummyStruct.freqOffset   = 240;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11acBss160.push_back(dummyStruct);

	//Default Mask of 802.11ac BSS80_80MHz. Not final, need to discuss about it later.
	dummyStruct.freqOffset   = 0;
    dummyStruct.deltaMaskPwr = -25;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
	dummyStruct.freqOffset   = 39;
    dummyStruct.deltaMaskPwr = 20;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 41;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
	dummyStruct.freqOffset   = 119;
    dummyStruct.deltaMaskPwr = 0;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 121;
    dummyStruct.deltaMaskPwr = -20;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
	dummyStruct.freqOffset   = 160;
    dummyStruct.deltaMaskPwr = -28;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);
    dummyStruct.freqOffset   = 200;
    dummyStruct.deltaMaskPwr = -40;
    l_wifiMask_11acBss80_80.push_back(dummyStruct);

    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_11ACtxMultiVerificationParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    l_11ACtxMultiVerificationParam.ENABLE_EVM = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.ENABLE_EVM))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationParam.ENABLE_EVM;
        setting.unit        = "";
        setting.helpText    = "Enable TX EVM test, default=1, means ON.";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ENABLE_EVM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxMultiVerificationParam.ENABLE_MASK = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.ENABLE_MASK))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationParam.ENABLE_MASK;
        setting.unit        = "";
        setting.helpText    = "Enable TX MASK test, default=1, means ON.";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ENABLE_MASK", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxMultiVerificationParam.ENABLE_POWER = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.ENABLE_POWER))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationParam.ENABLE_POWER;
        setting.unit        = "";
        setting.helpText    = "Enable TX POWER test, default=1, means ON.";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ENABLE_POWER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxMultiVerificationParam.ENABLE_SPECTRUM = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.ENABLE_SPECTRUM))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationParam.ENABLE_SPECTRUM;
        setting.unit        = "";
        setting.helpText    = "Enable TX SPECTRUM test, default=1, means ON.";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ENABLE_SPECTRUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH, MAX_BUFFER_SIZE, "BW-80");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_11ACtxMultiVerificationParam.BSS_BANDWIDTH;
		setting.unit        = "MHz";
		setting.helpText    = "BSS bandwidth\r\nValid options: BW-20, BW-40, BW-80, BW-80_80 or BW-160";
		l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_BANDWIDTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	strcpy_s(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, "0");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxMultiVerificationParam.CH_BANDWIDTH))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxMultiVerificationParam.CH_BANDWIDTH;
        setting.unit        = "MHz";
        setting.helpText    = "Channel bandwidth\r\nValid options:0, CBW-20, CBW-40, CBW-80, CBW-80_80 or CBW-160.\r\nFor 802.11ac, if it is zero,CH_BANDWIDTH equals as BSS_BANDWIDTH. For 802.11/a/b/g/n, it must always have value.";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_BANDWIDTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_11ACtxMultiVerificationParam.DATA_RATE, MAX_BUFFER_SIZE, "MCS0");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxMultiVerificationParam.DATA_RATE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxMultiVerificationParam.DATA_RATE;
        setting.unit        = "";
        setting.helpText    = "Data rate names:\r\nDSSS-1,DSSS-2,CCK-5_5,CCK-11\r\nOFDM-6,OFDM-9,OFDM-12,OFDM-18,OFDM-24,OFDM-36,OFDM-48,OFDM-54\r\nMCS0, MCS15, etc.";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_11ACtxMultiVerificationParam.PREAMBLE, MAX_BUFFER_SIZE, "SHORT");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxMultiVerificationParam.PREAMBLE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxMultiVerificationParam.PREAMBLE;
        setting.unit        = "";
        setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=SHORT.";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_11ACtxMultiVerificationParam.PACKET_FORMAT, MAX_BUFFER_SIZE, PACKET_FORMAT_VHT);
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxMultiVerificationParam.PACKET_FORMAT))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxMultiVerificationParam.PACKET_FORMAT;
        setting.unit        = "";
        setting.helpText    = "The packet format, VHT, HT_MF,HT_GF and NON_HT as defined in standard. Default=VHT.";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	// "STANDARD"
	strcpy_s(l_11ACtxMultiVerificationParam.STANDARD, MAX_BUFFER_SIZE, STANDARD_802_11_AC); 
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACtxMultiVerificationParam.STANDARD))    // Type_Checking
	{
		setting.value       = (void*)l_11ACtxMultiVerificationParam.STANDARD;
		setting.unit        = "";
		setting.helpText    = "Used for signal analysis option or to discriminating the same data rate or package format from different standards, taking value from 802.11ac, 802.11n, 802.11ag, 802.11b. Default = 802.11ac ";
		l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("STANDARD", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    // "GUARD_INTERVAL"
	strcpy_s(l_11ACtxMultiVerificationParam.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACtxMultiVerificationParam.GUARD_INTERVAL))    // Type_Checking
	{
		setting.value       = (void*)l_11ACtxMultiVerificationParam.GUARD_INTERVAL;
		setting.unit        = "";
		setting.helpText    = "Packet Guard Interval, Long or Short, default is Long";
		l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY = 5520;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment";
		l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_PRIMARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_SECONDARY = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_SECONDARY))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_SECONDARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined";
		l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_SECONDARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz;
		setting.unit        = "MHz";
		setting.helpText    = "The center frequency (MHz) for primary 20 MHZ channel, priority is lower than \"CH_FREQ_MHZ\".";
		l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_20MHz", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationParam.CH_FREQ_MHZ = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.CH_FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxMultiVerificationParam.CH_FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "It is the center frequency (MHz) for channel. If it is zero,\"CH_FREQ_MHZ_PRIMARY_20MHz\" will be used for 802.11ac. \r\nFor 802.11/a/b/g/n, it must alway have value.";
		l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_11ACtxMultiVerificationParam.NUM_STREAM_11AC = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.NUM_STREAM_11AC))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxMultiVerificationParam.NUM_STREAM_11AC;
		setting.unit        = "";
		setting.helpText    = "Number of spatial streams based on 11AC spec";
		l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("NUM_STREAM_11AC", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_11ACtxMultiVerificationParam.SAMPLING_TIME_US = 0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_11ACtxMultiVerificationParam.TX_POWER_DBM = 15.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationParam.TX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationParam.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxMultiVerificationParam.OBW_PERCENTAGE = 99.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationParam.OBW_PERCENTAGE))    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxMultiVerificationParam.OBW_PERCENTAGE;
        setting.unit        = "%";
        setting.helpText    = "The percentage of power for OBW measurement, default = 99%";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_PERCENTAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxMultiVerificationParam.CORRECTION_FACTOR_11B = 9.2;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationParam.CORRECTION_FACTOR_11B))    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxMultiVerificationParam.CORRECTION_FACTOR_11B;
        setting.unit  = "dB";
        setting.helpText  = "802.11b(only) LO leakage correction factor. Chipset specific.";
        l_11ACtxMultiVerificationParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("CORRECTION_FACTOR_11B", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxMultiVerificationParam.TX1 = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.TX1))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationParam.TX1;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxMultiVerificationParam.TX2 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.TX2))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationParam.TX2;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxMultiVerificationParam.TX3 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.TX3))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationParam.TX3;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxMultiVerificationParam.TX4 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxMultiVerificationParam.TX4))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationParam.TX4;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	strcpy_s(l_11ACtxMultiVerificationParam.MASK_TEMPLATE, MAX_BUFFER_SIZE, "");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxMultiVerificationParam.MASK_TEMPLATE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxMultiVerificationParam.MASK_TEMPLATE;
        setting.unit        = "";
        setting.helpText    = "Mask template, not use if blank";
        l_11ACtxMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("MASK_TEMPLATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
    /*----------------*
     * Return Values: *
     * ERROR_MESSAGE  *
     *----------------*/
    l_11ACtxMultiVerificationReturnMap.clear();

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz = 0;
    if (sizeof(int)==sizeof(l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz))    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 40 MHZ channel";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_40MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz = 0;
    if (sizeof(int)==sizeof(l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz))    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 80 MHZ channel";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_80MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	


    l_11ACtxMultiVerificationReturn.TX_POWER_DBM = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.TX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationReturn.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxMultiVerificationReturn.EVM_AVG_DB = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.EVM_AVG_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationReturn.EVM_AVG_DB;
        setting.unit        = "dB";
        setting.helpText    = "EVM average over captured packets and all data streams(MIMO)";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_AVG_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_11ACtxMultiVerificationReturn.EVM_AVG[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.EVM_AVG[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxMultiVerificationReturn.EVM_AVG[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "EVM_AVG_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Average EVM over captured packets on individual streams(MIMO)";
            l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }



    l_11ACtxMultiVerificationReturn.EVM_PK_DB = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.EVM_PK_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationReturn.EVM_PK_DB;
        setting.unit        = "dB";
        setting.helpText    = "(11b only)Peak EVM over captured packets.";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_PK_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxMultiVerificationReturn.AMP_ERR_DB = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.AMP_ERR_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationReturn.AMP_ERR_DB;
        setting.unit        = "dB";
        setting.helpText    = "IQ Match Amplitude Error in dB.";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("AMP_ERR_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for(int i=0;i<MAX_DATA_STREAM;i++)
	{
	    l_11ACtxMultiVerificationReturn.AMP_ERR_DB_STREAM[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.AMP_ERR_DB_STREAM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxMultiVerificationReturn.AMP_ERR_DB_STREAM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "AMP_ERR_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "IQ Match Amplitude Error in dB.(per stream)";
            l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
	}

	l_11ACtxMultiVerificationReturn.PHASE_ERR = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.PHASE_ERR))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationReturn.PHASE_ERR;
        setting.unit        = "Degree";
        setting.helpText    = "IQ Match Phase Error.";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PHASE_ERR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxMultiVerificationReturn.SYMBOL_CLK_ERR = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.SYMBOL_CLK_ERR))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxMultiVerificationReturn.SYMBOL_CLK_ERR;
		setting.unit        = "ppm";
		setting.helpText    = "Symbol Clock Error";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SYMBOL_CLK_ERR", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for(int i=0;i<MAX_DATA_STREAM;i++)
	{
	    l_11ACtxMultiVerificationReturn.PHASE_ERR_STREAM[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.PHASE_ERR_STREAM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxMultiVerificationReturn.PHASE_ERR_STREAM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "PHASE_ERR_%d", i+1);
			setting.unit        = "Degree";
			setting.helpText    = "IQ Match Phase Error.(per stream)";
            l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
	}

	//PHASE_NOISE_RMS_ALL
	l_11ACtxMultiVerificationReturn.PHASE_NOISE_RMS_ALL = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.PHASE_NOISE_RMS_ALL))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationReturn.PHASE_NOISE_RMS_ALL;
        setting.unit        = "Degree";
        setting.helpText    = "Frequency RMS Phase Noise.";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PHASE_NOISE_RMS_ALL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxMultiVerificationReturn.POWER_AVG_DBM = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.POWER_AVG_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationReturn.POWER_AVG_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Average Power over captured packets";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVG_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_11ACtxMultiVerificationReturn.POWER_AVG[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.POWER_AVG[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxMultiVerificationReturn.POWER_AVG[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_AVG_%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "Average Power over captured packets on individual streams";
            l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }


    l_11ACtxMultiVerificationReturn.FREQ_ERROR_AVG = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.FREQ_ERROR_AVG))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationReturn.FREQ_ERROR_AVG;
        setting.unit        = "ppm";
        setting.helpText    = "Average frequency error over captured packets";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_ERROR_AVG", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxMultiVerificationReturn.DATA_RATE = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.DATA_RATE))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationReturn.DATA_RATE;
        setting.unit        = "Mbps";
        setting.helpText    = "Data rate in Mbps, reported by IQAPI.";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxMultiVerificationReturn.SPATIAL_STREAM = (int)NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxMultiVerificationReturn.SPATIAL_STREAM))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxMultiVerificationReturn.SPATIAL_STREAM;
        setting.unit        = "";
        setting.helpText    = "Number of spatial stream, reported by IQAPI.";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SPATIAL_STREAM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxMultiVerificationReturn.LEAST_MARGIN_SUBCARRIER = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxMultiVerificationReturn.LEAST_MARGIN_SUBCARRIER))    // Type_Checking
	{
		setting.value = (void*)&l_11ACtxMultiVerificationReturn.LEAST_MARGIN_SUBCARRIER;
		setting.unit        = "";
		setting.helpText    = "Carrier no. with least margin";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("LEAST_MARGIN_SUBCARRIER", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_A = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_A))    // Type_Checking
	{
		setting.value = (void*)&l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_A;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of lower section A in each VSA.";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_LO_A", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_B = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_B))    // Type_Checking
	{
		setting.value = (void*)&l_11ACtxMultiVerificationReturn.SUBCARRIER_LO_B;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of lower section B in each VSA.";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_LO_B", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_A = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_A))    // Type_Checking
	{
		setting.value = (void*)&l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_A;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of upper section A in each VSA.";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_UP_A", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_B = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_B))    // Type_Checking
	{
		setting.value = (void*)&l_11ACtxMultiVerificationReturn.SUBCARRIER_UP_B;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of upper section B in each VSA.";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_UP_B", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationReturn.MARGIN_DB = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.MARGIN_DB))    // Type_Checking
	{
		setting.value = (void*)&l_11ACtxMultiVerificationReturn.MARGIN_DB;
		setting.unit        = "dB";
		setting.helpText    = "Margin to the average spectral center power in each VSA.";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("MARGIN_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationReturn.LO_LEAKAGE_DB = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.LO_LEAKAGE_DB))    // Type_Checking
	{
		setting.value = (void*)&l_11ACtxMultiVerificationReturn.LO_LEAKAGE_DB;
		setting.unit        = "dBc";
		setting.helpText    = "Lo leakage in dBc, normally should be a negative number in VSA, if pass.";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("LO_LEAKAGE_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationReturn.VALUE_DB_LO_A = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.VALUE_DB_LO_A))    // Type_Checking
	{
		setting.value = (void*)&l_11ACtxMultiVerificationReturn.VALUE_DB_LO_A;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of lower section A in each VSA.";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_LO_A", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationReturn.VALUE_DB_LO_B = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.VALUE_DB_LO_B))    // Type_Checking
	{
		setting.value = (void*)&l_11ACtxMultiVerificationReturn.VALUE_DB_LO_B;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of lower section B in each VSA.";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_LO_B", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationReturn.VALUE_DB_UP_A = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.VALUE_DB_UP_A))    // Type_Checking
	{
		setting.value = (void*)&l_11ACtxMultiVerificationReturn.VALUE_DB_UP_A;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of upper section A in each VSA.";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_UP_A", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxMultiVerificationReturn.VALUE_DB_UP_B = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.VALUE_DB_UP_B))    // Type_Checking
	{
		setting.value = (void*)&l_11ACtxMultiVerificationReturn.VALUE_DB_UP_B;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of upper section B in each VSA.";
		l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_UP_B", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    for (int i=0;i<MAX_POSITIVE_SECTION;i++)
    {
		l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "MARGIN_DB_UPPER_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
            l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_NEGATIVE_SECTION;i++)
    {
		l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "MARGIN_DB_LOWER_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
            l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_POSITIVE_SECTION;i++)
    {
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "FREQ_AT_MARGIN_UPPER_%d", i+1);
            setting.unit        = "MHz";
            setting.helpText    = "Point out the frequency offset at margin to the mask.";
            l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_NEGATIVE_SECTION;i++)
    {
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "FREQ_AT_MARGIN_LOWER_%d", i+1);
            setting.unit        = "MHz";
            setting.helpText    = "Point out the frequency offset at margin to the mask.";
            l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }
    
    l_11ACtxMultiVerificationReturn.VIOLATION_PERCENT = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.VIOLATION_PERCENT))    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxMultiVerificationReturn.VIOLATION_PERCENT;
        setting.unit        = "%";
        setting.helpText    = "Percentage which fail the mask";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VIOLATION_PERCENT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxMultiVerificationReturn.OBW_MHZ = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.OBW_MHZ))    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxMultiVerificationReturn.OBW_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "OBW for the specified power percentage";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_11ACtxMultiVerificationReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_11ACtxMultiVerificationReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_11ACtxMultiVerificationReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_11ACtxMultiVerificationReturn.SPECTRUM_RAW_DATA_X.clear();
    setting.type = WIFI_SETTING_TYPE_ARRAY_DOUBLE;
    if (0 == l_11ACtxMultiVerificationReturn.SPECTRUM_RAW_DATA_X.size())    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxMultiVerificationReturn.SPECTRUM_RAW_DATA_X;
        setting.unit        = "dBm/100kHz";
        setting.helpText    = "Spectrum X raw data.";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SPECTRUM_RAW_DATA_X", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxMultiVerificationReturn.SPECTRUM_RAW_DATA_Y.clear();
    setting.type = WIFI_SETTING_TYPE_ARRAY_DOUBLE;
    if (0 == l_11ACtxMultiVerificationReturn.SPECTRUM_RAW_DATA_Y.size())    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxMultiVerificationReturn.SPECTRUM_RAW_DATA_Y;
        setting.unit        = "dBm/100kHz";
        setting.helpText    = "Spectrum Y raw data.";
        l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SPECTRUM_RAW_DATA_Y", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxMultiVerificationReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxMultiVerificationReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxMultiVerificationReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_11ACtxMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

void CheckFlatnessViolation_All(double deltaPower, int carrier, double lowLimit, double highLimit, double *sectionPower, int *sectionCarrier, double *failMargin, int *sectionCarrier2)
{
	if ( deltaPower>=0 )
	{
		// No matter Failed or OK, we need to save the margin
		if ((deltaPower-highLimit)>(*failMargin))	// Save the margin
		{
			*failMargin = deltaPower-highLimit;
			*sectionPower = deltaPower;
			*sectionCarrier = carrier;
			*sectionCarrier2 = carrier;
		}
	}
	else
	{
		// No matter Failed or OK, we need to save the margin
		if ((lowLimit-deltaPower)>(*failMargin))	// Save the margin
		{
			*failMargin = lowLimit-deltaPower;
			*sectionPower = deltaPower;
			*sectionCarrier = carrier;
			*sectionCarrier2 = carrier;
		}
	}
}


int LoResultSpectralFlatness11AG_All( int* carrierNoUpA,
								  int* carrierNoUpB,
								  int* carrierNoLoA,
								  int* carrierNoLoB,
								  double* failMargin, 
								  int* leastMarginCarrier,
								  double* loLeakage, 
								  double* valueDbUpSectionA,
								  double* valueDbUpSectionB,
								  double* valueDbLoSectionA,
								  double* valueDbLoSectionB)
{
    int     result = ERR_OK;
   
    int     i, lengthIQ = 0;
    double  *dataQ, *dataI, *data;  
    double  avg_center_power = 0, avg_power = 0;
	int     numberOfEst = 0;
	int     carrierFirst, carrierSecond, carrierThird, carrierFourth;
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	bool    flatnessPass = true;


	/* for non-analysis cas, initialize margin and loLeakage to NaN*/
	*failMargin    = NA_NUMBER;
	*loLeakage = NA_NUMBER;
	
	*carrierNoUpA = NA_INTEGER;
	*carrierNoUpB = NA_INTEGER;
	*carrierNoLoA = NA_INTEGER;
	*carrierNoLoB = NA_INTEGER;

	/* for non-analysis cas, initialize valueDbUpSectionA/B and valudDbLoSectionA/B to NaN*/
	*valueDbUpSectionA = NA_NUMBER;
	*valueDbUpSectionB = NA_NUMBER;
	*valueDbLoSectionA = NA_NUMBER;
	*valueDbLoSectionB = NA_NUMBER;

	numberOfEst = ::LP_GetVectorMeasurement("hhEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
    if ( numberOfEst!=64 )	// IQ data length, should be 64
    {
        return -1;
    }
	else
	{
		lengthIQ = numberOfEst; 
	}
    
    dataI    = (double *) malloc (lengthIQ * sizeof (double));
	dataQ    = (double *) malloc (lengthIQ * sizeof (double));
    data     = (double *) malloc (lengthIQ * sizeof (double));

    if (dataQ && dataI && data)
    {
		// Get IQ data.			
		for (int carrier=0;carrier<numberOfEst;carrier++)
		{
			dataI[carrier] = bufferReal[carrier];
			dataQ[carrier] = bufferImag[carrier];

			// 1. Convert I and Q to power: P = I*I + Q*Q (the formula is not 100% correct, but log will take out the ratio)
			// 2. Average P 
			// 3. avg_power = 10 * log (avg_P)

			// Calculate the power at each point from I & Q data.
			data[carrier] = CalculateIQtoP( dataI[carrier], dataQ[carrier] );
		}

		// IEEE 802.11a/g Transmitter flatness
		// The average energy of the constellations in each of the spectral lines -16.. -1 and +1.. +16 will deviate no
		// more than +/- 2 dB from their average energy. The average energy of the constellations in each of the spectral
		// lines -26.. -17 and +17.. +26 will deviate no more than +2/-4 dB from the average energy of spectral lines
		// -16.. -1 and +1.. +16. The data for this test shall be derived from the channel estimation step.
		
		carrierFirst  = CARRIER_1;
		carrierSecond = CARRIER_16;
		carrierThird  = CARRIER_17;
		carrierFourth = CARRIER_26;
	
		// Calculate the "avg_center_power" from carriers of -16 to -1 and +1 to +16.
		// Calculate the "avg_power"        from carriers of -26 to -1 and +1 to +26.

        // Calculate the average energy for the spectral lines
		for (i=carrierFirst;i<=carrierSecond;i++)
		{
			avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
		}
		// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			avg_power = avg_power + data[i] + data[lengthIQ-i];
		}
		avg_power = avg_power + avg_center_power;

		avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
		if (0!=avg_center_power)
		{
			avg_center_power = 10.0 * log10 (avg_center_power);
		}
		else
		{
			avg_center_power = 0;
		}
		avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
		if (0!=avg_power)
		{
			avg_power = 10.0 * log10 (avg_power);
		}
		else
		{
			avg_power = 0;
		}

	    for (int carrier=0;carrier<lengthIQ;carrier++)
	    {
			if (0!=data[carrier])
			{
				data[carrier] = 10.0 * log10 (data[carrier]);
			}
			else
			{
				data[carrier] = 0;
			}
	    }

		double deltaPower = 0;
		double highLimit = 2, lowLimit = -2;
		double sectionMargin[4] = {-99, -99, -99, -99};
		int sectionCarrierNo[4];

		// Check the flatness for carrier -16 to -1 and +1 to +16.
		for (i=carrierFirst;i<=carrierSecond;i++)
		{
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
		}

		// Continue, Check the flatness -26 to -17 and +17 to +26.
		lowLimit = -4;
		double sectionBFailMargin = 0;
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
		}

		// get the highest fail margin
		*failMargin = -99;
		for (i=0 ; i<4 ; i++)
		{
			if (sectionMargin[i]>*failMargin)
			{
				*failMargin = sectionMargin[i];
				*leastMarginCarrier = sectionCarrierNo[i];
			}
		}

		// Get the center frequency leakage, data[0] is the center frequency power
		// minus "10 * log10((double)((carrierFourth-carrierFirst+1)*2))" for calculate dBc measurement
		*loLeakage = data[0] - avg_power - 10 * log10((double)((carrierFourth-carrierFirst+1)*2));

		#if defined(_DEBUG)		
			// print out the Mask into file for debug				
			FILE *fp;  
			fopen_s(&fp, "log/Log_Flatness_Result_11AG.csv", "w");    
			if (fp)
			{
				fprintf(fp, "WiFi Flatness Carrier,Signal,Avg_Center_Power,Size: %d\n", lengthIQ);    // print to log file 
				for (int carrier=0;carrier<lengthIQ;carrier++)
				{
					fprintf( fp, "%d,%8.2f,%8.2f\n", carrier, data[carrier], avg_center_power);    // print to log file 
				}
				fclose(fp);
			}
			else
			{
				// fopen failed
			}
		#endif
	}
	else
	{
		// if (dataQ && dataI && data)
		result = -1;
	}
    
    if (dataQ)	   free (dataQ);
    if (dataI)	   free (dataI);
    if (data)	   free (data);

    return result;
}

int  LoResultSpectralFlatness11N_All(int basedOnVsa,
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
								 int streamIndex)
// streamIndex = 0, 1, 2, 3 (zero based)
{
    int     result = ERR_OK;
   
    int     i, lengthIQ;     
    double  avg_center_power = 0, avg_power = 0;
	int     numberOfEst = 0, numberOfStream = 0, bandwidthMhz = 20;
	int     carrierFirst, carrierSecond, carrierThird, carrierFourth;
	double  *dataRaw, *data; 
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	bool    flatnessPass = true;
    int		vsaIndex = 0; 


	/* for non-analysis cas, initialize margin and loLeakage to NaN*/
	*failMargin = NA_NUMBER;
	*loLeakage = NA_NUMBER;

	*carrierNoUpA = NA_INTEGER;
	*carrierNoUpB = NA_INTEGER;
	*carrierNoLoA = NA_INTEGER;
	*carrierNoLoB = NA_INTEGER;

	/* for non-analysis cas, initialize valuedbUpSectionA/B to NaN*/
	*valueDbUpSectionA = NA_NUMBER;
	*valueDbUpSectionB = NA_NUMBER;
	*valueDbLoSectionA = NA_NUMBER;
	*valueDbLoSectionB = NA_NUMBER;

	vsaIndex = streamIndex;

    //NOTE: Nstreams x Ntones x NRx
	numberOfEst = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
    if (numberOfEst <= 0)
    {
        return -1;
    }

	numberOfStream = (int)::LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);

    if ((streamIndex >= numberOfStream) && !basedOnVsa)
    {
        return 0;	// special case, need return, but not error.
    }


	bandwidthMhz = (int)::LP_GetScalarMeasurement("rateInfo_bandwidthMhz", 0); 
	if ( (wifiMode==WIFI_11N_MF_HT20 || wifiMode==WIFI_11N_GF_HT20)&&(bandwidthMhz!=20) )
	{
		return -1;
	}
	else if ( (wifiMode==WIFI_11N_MF_HT40 || wifiMode==WIFI_11N_GF_HT40)&&(bandwidthMhz!=40) )
	{
		return -1;
	}
	else
	{
		// IQ data length, should be 64 or 128
		if (bandwidthMhz==20)
		{
			lengthIQ = 64;
		}
		else
		{
			lengthIQ = 128;
		}
	}
 
	dataRaw  = (double *) malloc (numberOfEst * sizeof (double));
    data     = (double *) malloc (lengthIQ * sizeof (double));

    if (dataRaw && data)
    {
		// 1. Convert I and Q to power: P = I*I + Q*Q (the formula is not 100% correct, but log will take out the ratio)
		// 2. Average P 
		// 3. avg_power = 10 * log (avg_P)

		// Calculate the power at each point from I & Q raw data.
		for (int carrier=0;carrier<numberOfEst;carrier++)
		{
			dataRaw[carrier] = CalculateIQtoP( bufferReal[carrier], bufferImag[carrier] );
		}

		// Initial the data[] array
		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			data[carrier] = 0;
		}	

        if(!basedOnVsa)
        {
		    // Get data by stream and each VSA.			
		    for (int carrier=0;carrier<lengthIQ;carrier++)
		    {
			    for (int i=0;i<g_Tester_Number;i++)
			    {
				    int numberOfIndex = (lengthIQ*numberOfStream*i)+(numberOfStream*carrier)+streamIndex;
				    if (numberOfIndex<numberOfEst)
				    {
					    data[carrier] = data[carrier] + dataRaw[numberOfIndex];
				    }
				    else
				    {
					    break;
				    }
			    }		
		    }
        }
        else // based on Vsa
        {
		    for (int carrier=0;carrier<lengthIQ;carrier++)
		    {
			    for (int i=0;i<numberOfStream;i++)
			    {
				    int numberOfIndex = (lengthIQ*numberOfStream*vsaIndex)+(numberOfStream*carrier)+i;
				    if (numberOfIndex<numberOfEst)
				    {
					    data[carrier] = data[carrier] + dataRaw[numberOfIndex];
				    }
				    else
				    {
					    break;
				    }
			    }		
		    }

        }




		// IEEE std 802.11n Transmitter flatness
		// HT20	
		// In a 20 MHz channel and in corresponding 20 MHz transmission in a 40 MHz channel, the average energy
		// of the constellations in each of the spectral lines ¡V16 to ¡V1 and +1 to +16 shall deviate no more than ¡Ó 2 dB
		// from their average energy. The average energy of the constellations in each of the spectral lines ¡V28 to ¡V17
		// and +17 to +28 shall deviate no more than +2/¡V4 dB from the average energy of spectral lines ¡V16 to ¡V1 and +1 to +16.

		// HT40
		// In a 40 MHz transmission (excluding HT duplicate format and non-HT duplicate format) the average energy
		// of the constellations in each of the spectral lines ¡V42 to ¡V2 and +2 to +42 shall deviate no more than ¡Ó 2 dB
		// from their average energy. The average energy of the constellations in each of the spectral lines ¡V43 to ¡V58
		// and +43 to +58 shall deviate no more than +2/¡V4 dB from the average energy of spectral lines ¡V42 to ¡V2 and +2 to +42.

		// In HT duplicate format and non-HT duplicate format the average energy of the constellations in each of the
		// spectral lines -42 to -33, -31 to -6, +6 to +31, and +33 to +42 shall deviate no more than ¡Ó 2 dB from their
		// average energy. The average energy of the constellations in each of the spectral lines -43 to -58 and +43 to
		// +58 shall deviate no more than +2/-4 dB from the average energy of spectral lines -42 to -33, -31 to -6, +6
		// to +31, and +33 to +42.

		if (lengthIQ==64)	// HT20 mode
		{
			carrierFirst  = CARRIER_1;
			carrierSecond = CARRIER_16;
			carrierThird  = CARRIER_17;
			carrierFourth = CARRIER_28;
		}
		else				// HT40 mode, lengthIQ=128
		{
			carrierFirst  = CARRIER_2;
			carrierSecond = CARRIER_42;
			carrierThird  = CARRIER_43;
			carrierFourth = CARRIER_58;
		}		


        /* Carrier number report order:
        * 20MHz: +1 ~ +32, -32 ~ -1
        * 40MHz: +1 ~ +64, -64 ~ -1
        */
        // Calculate the average energy for the spectral lines
		for ( i=carrierFirst;i<=carrierSecond;i++)
		{
			avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
		}
		// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			avg_power = avg_power + data[i] + data[lengthIQ-i];
		}
		avg_power = avg_power + avg_center_power;

		avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
		if (0!=avg_center_power)
		{
			avg_center_power = 10.0 * log10 (avg_center_power);
		}
		else
		{
			avg_center_power= 0;
		}
		avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
		if (0!=avg_power)
		{
			avg_power = 10.0 * log10 (avg_power);
		}
		else
		{
			avg_power = 0;
		}

		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			if (0!=data[carrier])
			{
				data[carrier] = 10.0 * log10 (data[carrier]);
			}
			else
			{
				data[carrier] = 0;
			}
		}

		// Check the flatness
		double deltaPower = 0;
		double highLimit = 2, lowLimit = -2;
		double sectionMargin[4] = {-99, -99, -99, -99};
		int sectionCarrierNo[4];

		for (i=carrierFirst;i<=carrierSecond;i++)
		{
			//UPPER SECTION A
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

			//LOWER SECTION A
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
		}

		// Continue, Check the flatness. (side)
		lowLimit = -4;
		for (i=carrierThird;i<=carrierFourth;i++)
		{
			//UPPER SECTION B
			deltaPower = data[i]-avg_center_power;
			CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);
			
			//LOWER SECTION B
			deltaPower = data[lengthIQ-i]-avg_center_power;
			CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
		}

		// get the highest fail margin
		*failMargin = -99;
		for (i=0 ; i<4 ; i++)
		{
			if (sectionMargin[i]>*failMargin)
			{
				*failMargin = sectionMargin[i];
				*leastMarginCarrier = sectionCarrierNo[i];
			}
		}

		// Get the center frequency leakage 
		*loLeakage = data[0] - avg_power - 10 * log10((double)((carrierFourth-carrierFirst+1)*2));

		#if defined(_DEBUG)
			// print out the Mask into file for debug				
			FILE *fp;  
			char sigFileName[MAX_BUFFER_SIZE];
			sprintf_s(sigFileName, "log/Log_Flatness_Result_11N_Stream%02d.csv", streamIndex+1);
			fopen_s(&fp, sigFileName, "w");      
			if (fp)
			{
				fprintf(fp, "WiFi Flatness Carrier,Signal,Avg_Center_Power,Size: %d\n", lengthIQ);    // print to log file 
				for (int carrier=0;carrier<lengthIQ;carrier++)
				{
					fprintf( fp, "%d,%8.2f,%8.2f\n", carrier, data[carrier], avg_center_power);    // print to log file 
				}
				fclose(fp);
			}
			else
			{
				// fopen failed
			}    
		#endif
	}
	else
	{
		// if (dataQ && dataI && data && maxData && minData)
		result = -1;
	}
    
    if (dataRaw)   free (dataRaw);
	if (data)	   free (data);

    return result;
}

int  LoResultSpectralFlatness11AC_All(int basedOnVsa,
								  int cbw,
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
								  int streamIndex)
								  // streamIndex = 0, 1, 2, 3 (zero based)
{
	int     result = ERR_OK;

	int     i, lengthIQ;     
	double  avg_center_power = 0, avg_power = 0;
	int     numberOfEst = 0, numberOfStream = 0, bandwidthMhz = 20;
	int     carrierFirst, carrierSecond, carrierThird, carrierFourth;

	double  *dataRaw, *data; 
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	bool    flatnessPass = true;
	int		vsaIndex = 0; 

	double sectionMargin[4] = {-99, -99, -99, -99};
	int sectionCarrierNo[4];

	// Added for 802.11ac
	int		carrierPilot;
	int		carrierFifth, carrierSixth, carrierSeventh,	carrierEigth, carrierNinth,carrierTenth;  
	int		carrierNumSection1, carrierNumSection2;
	//END



	/* for non-analysis cas, initialize margin and loLeakage to NaN*/
	*failMargin = NA_NUMBER;
	//	*loLeakage = NA_DOUBLE;

	*carrierNoUpA = NA_INTEGER;
	*carrierNoUpB = NA_INTEGER;
	*carrierNoLoA = NA_INTEGER;
	*carrierNoLoB = NA_INTEGER;

	/* for non-analysis cas, initialize valuedbUpSectionA/B to NaN*/
	*valueDbUpSectionA = NA_NUMBER;
	*valueDbUpSectionB = NA_NUMBER;
	*valueDbLoSectionA = NA_NUMBER;
	*valueDbLoSectionB = NA_NUMBER;

	vsaIndex = streamIndex;

	//NOTE: Nstreams x Ntones x NRx
	numberOfEst = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
	if (numberOfEst <= 0)
	{
		return -1;
	}

	numberOfStream = (int)::LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);

	if ((streamIndex >= numberOfStream) && !basedOnVsa)
	{
		return 0;	// special case, need return, but not error.
	}
	

	bandwidthMhz = (int)::LP_GetScalarMeasurement("rateInfo_bandwidthMhz", 0); 
	if ( ( cbw == BW_20MHZ) &&(bandwidthMhz!=20) )
	{
		return -1;
	}
	else if ( ( cbw == BW_40MHZ )&&(bandwidthMhz!=40) )
	{
		return -1;
	}
	else if ( ( cbw == BW_80MHZ )&&(bandwidthMhz!=80) )
	{
		return -1;
	}
	else if ( ( cbw == BW_160MHZ )&&(bandwidthMhz!=160) )
	{
		return -1;
	}
	else if (( cbw == BW_80_80MHZ ) &&(bandwidthMhz!=160) )     // BW_80_80MHZ not supported yet
	{
		return -1;
	}
	else     // bandwidthMHz is correct
	{
		if ( 0 != strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT,PACKET_FORMAT_NON_HT))    //802.11ac VHT, MF_HT, GF_HT
		{
			// IQ data length, should be 64 or 128
			if ( bandwidthMhz == 20)
			{
				lengthIQ = 64;
				carrierFirst  = CARRIER_1;
				carrierSecond = CARRIER_16;
				carrierThird  = CARRIER_17;
				carrierFourth = CARRIER_28;
			}
			else if (bandwidthMhz == 40)
			{
				lengthIQ = 128;
				carrierFirst  = CARRIER_2;
				carrierSecond = CARRIER_42;
				carrierThird  = CARRIER_43;
				carrierFourth = CARRIER_58;
			}
			else if (bandwidthMhz == 80)
			{
				lengthIQ = 256;
				carrierFirst  = CARRIER_2;
				carrierSecond = CARRIER_84;
				carrierThird  = CARRIER_85;
				carrierFourth = CARRIER_122;
			}
			else if (bandwidthMhz == 160)
			{
				// Not supported yet
				return 0;
			}
			else   // 80_80
			{
				// Not supported yet
				return 0;
			}
		}
		else   //802.11ac non-HT duplicate
		{
			if ( bandwidthMhz == 40)
			{
				lengthIQ = 128;
				carrierFirst  = CARRIER_6;
				carrierSecond = CARRIER_42;
				carrierPilot = CARRIER_32;
				carrierThird  = CARRIER_43;
				carrierFourth = CARRIER_58;
				//	carrierSecond = CARRIER_31;
				//	carrierThird  = CARRIER_33;
				//	carrierFourth = CARRIER_42;
				//	carrierFifth  = CARRIER_43;
				//	carrierSixth  = CARRIER_58;

			}
			else if (bandwidthMhz == 80)  // bandwidth = 80
			{
				lengthIQ = 256;
				carrierFirst	= CARRIER_6;
				carrierSecond	= CARRIER_31;
				carrierThird	= CARRIER_33;
				carrierFourth	= CARRIER_58;
				carrierFifth	= CARRIER_70;
				carrierSixth	= CARRIER_84;
				carrierNumSection1 = (carrierSecond - carrierFirst + 1) +
					(carrierFourth - carrierThird + 1) +
					(carrierSixth - carrierFifth + 1);

				carrierSeventh  = CARRIER_85;
				carrierEigth	= CARRIER_95;
				carrierNinth	= CARRIER_97;
				carrierTenth	= CARRIER_122;
				carrierNumSection2 = ( carrierEigth - carrierSeventh + 1) +
					( carrierTenth - carrierNinth +1);

			}
			else if ( bandwidthMhz == 160)
			{
				// Not supported yet
			}
			else		// bandwidth == 80_80
			{
				// Not supported yet
			}
		}
	}

	dataRaw  = (double *) malloc (numberOfEst * sizeof (double));
	data     = (double *) malloc (lengthIQ * sizeof (double));

	if (dataRaw && data)
	{
		// 1. Convert I and Q to power: P = I*I + Q*Q (the formula is not 100% correct, but log will take out the ratio)
		// 2. Average P 
		// 3. avg_power = 10 * log (avg_P)

		// Calculate the power at each point from I & Q raw data.
		for (int carrier=0;carrier<numberOfEst;carrier++)
		{
			dataRaw[carrier] = CalculateIQtoP( bufferReal[carrier], bufferImag[carrier] );
		}

		// Initial the data[] array
		for (int carrier=0;carrier<lengthIQ;carrier++)
		{
			data[carrier] = 0;
		}	

		if(!basedOnVsa)
		{
			// Get data by stream and each VSA.			
			for (int carrier=0;carrier<lengthIQ;carrier++)
			{
				for (int i=0;i<g_Tester_Number;i++)
				{
					int numberOfIndex = (lengthIQ*numberOfStream*i)+(numberOfStream*carrier)+streamIndex;
					if (numberOfIndex<numberOfEst)
					{
						data[carrier] = data[carrier] + dataRaw[numberOfIndex];
					}
					else
					{
						break;
					}
				}		
			}
		}
		else // based on Vsa
		{
			for (int carrier=0;carrier<lengthIQ;carrier++)
			{
				for (int i=0;i<numberOfStream;i++)
				{
					int numberOfIndex = (lengthIQ*numberOfStream*vsaIndex)+(numberOfStream*carrier)+i;
					if (numberOfIndex<numberOfEst)
					{
						data[carrier] = data[carrier] + dataRaw[numberOfIndex];
					}
					else
					{
						break;
					}
				}		
			}

		}


		if ( 0 != strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT,PACKET_FORMAT_NON_HT) && bandwidthMhz != 160)    //802.11ac VHT, MF_HT, GF_HT, BW = 20,40,80, 80_80
		{
			// Calculate the average energy for the spectral lines
			for ( i=carrierFirst;i<=carrierSecond;i++)
			{
				avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
			}
			// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
			for (i=carrierThird;i<=carrierFourth;i++)
			{
				avg_power = avg_power + data[i] + data[lengthIQ-i];
			}
			avg_power = avg_power + avg_center_power;

			avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
			if (0!=avg_center_power)
			{
				avg_center_power = 10.0 * log10 (avg_center_power);
			}
			else
			{
				avg_center_power= 0;
			}
			avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
			if (0!=avg_power)
			{
				avg_power = 10.0 * log10 (avg_power);
			}
			else
			{
				avg_power = 0;
			}

			for (int carrier=0;carrier<lengthIQ;carrier++)
			{
				if (0!=data[carrier])
				{
					data[carrier] = 10.0 * log10 (data[carrier]);
				}
				else
				{
					data[carrier] = 0;
				}
			}


			// Check the flatness
			double deltaPower = 0;
			double highLimit = 4, lowLimit = -4;
			//double sectionMargin[4] = {-99, -99, -99, -99};
			//int sectionCarrierNo[4];

			for (i=carrierFirst;i<=carrierSecond;i++)
			{
				//UPPER SECTION A
				deltaPower = data[i]-avg_center_power;
				CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

				//LOWER SECTION A
				deltaPower = data[lengthIQ-i]-avg_center_power;
				CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
			}

			// Continue, Check the flatness. (side)
			highLimit = 4, lowLimit = -6;
			for (i=carrierThird;i<=carrierFourth;i++)
			{
				//UPPER SECTION B
				deltaPower = data[i]-avg_center_power;
				CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);

				//LOWER SECTION B
				deltaPower = data[lengthIQ-i]-avg_center_power;
				CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
			}

		}
		else if ( 0 != strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT,PACKET_FORMAT_NON_HT) && bandwidthMhz == 160) 
		{
			// BW = 160MHz, not supported yet
		}
		else     //802.11ac non-HT
		{
			if ( bandwidthMhz == 40)
			{
				// Calculate the average energy for the spectral lines
				for ( i=carrierFirst;i<=carrierSecond;i++)
				{
					avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
				}
				// Carrier32 is pilot, not caculated in avg_center_power
				avg_center_power = avg_center_power -data[carrierPilot] - data[lengthIQ-carrierPilot];

				// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
				for (i=carrierThird;i<=carrierFourth;i++)
				{
					avg_power = avg_power + data[i] + data[lengthIQ-i];
				}
				avg_power = avg_power + avg_center_power;

				//avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
				// Carrier 32 is pilot, not caculated
				avg_center_power = avg_center_power/((carrierSecond-carrierFirst)*2);
				if (0!=avg_center_power)
				{
					avg_center_power = 10.0 * log10 (avg_center_power);
				}
				else
				{
					avg_center_power= 0;
				}
				avg_power = avg_power/((carrierFourth-carrierFirst+1)*2);
				if (0!=avg_power)
				{
					avg_power = 10.0 * log10 (avg_power);
				}
				else
				{
					avg_power = 0;
				}

				for (int carrier=0;carrier<lengthIQ;carrier++)
				{
					if (0!=data[carrier])
					{
						data[carrier] = 10.0 * log10 (data[carrier]);
					}
					else
					{
						data[carrier] = 0;
					}
				}

				// Check the flatness
				double deltaPower = 0;
				double highLimit = 4, lowLimit = -4;
				//double sectionMargin[4] = {-99, -99, -99, -99};
				//int sectionCarrierNo[4];

				for (i=carrierFirst;i<=carrierSecond;i++)
				{
					if ( i != carrierPilot)
					{
						//UPPER SECTION A
						deltaPower = data[i]-avg_center_power;
						CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

						//LOWER SECTION A
						deltaPower = data[lengthIQ-i]-avg_center_power;
						CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
					}
					else
					{
						// do nothing
					}
				}

				// Continue, Check the flatness. (side)
				highLimit = 4, lowLimit = -6;
				for (i=carrierThird;i<=carrierFourth;i++)
				{
					//UPPER SECTION B
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);

					//LOWER SECTION B
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
				}
			}
			else if ( bandwidthMhz == 80) 
			{
				// Calculate the average energy for the spectral lines
				for ( i=carrierFirst;i<=carrierSecond;i++)   // carrier 6 ~31
				{
					avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
				}
				for ( i=carrierThird;i<=carrierFourth;i++)   // carrier 33 ~58
				{
					avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
				}
				for ( i=carrierFifth;i<=carrierSixth;i++)   // carrier 70 ~84
				{
					avg_center_power = avg_center_power + data[i] + data[lengthIQ-i];
				}



				// Continue, calculating the (side) average energy. This one must add the average energy of avg_center_power.
				for (i=carrierSeventh;i<=carrierEigth;i++)
				{
					avg_power = avg_power + data[i] + data[lengthIQ-i];
				}
				for (i=carrierTenth;i<=carrierNinth;i++)
				{
					avg_power = avg_power + data[i] + data[lengthIQ-i];
				}

				avg_power = avg_power + avg_center_power;

				//avg_center_power = avg_center_power/((carrierSecond-carrierFirst+1)*2);
				// Carrier 32 is pilot, not caculated
				avg_center_power = avg_center_power/ ( carrierNumSection1 *2);
				if (0!=avg_center_power)
				{
					avg_center_power = 10.0 * log10 (avg_center_power);
				}
				else
				{
					avg_center_power= 0;
				}
				avg_power = avg_power/(carrierNumSection2 *2);
				if (0!=avg_power)
				{
					avg_power = 10.0 * log10 (avg_power);
				}
				else
				{
					avg_power = 0;
				}

				for (int carrier=0;carrier<lengthIQ;carrier++)
				{
					if (0!=data[carrier])
					{
						data[carrier] = 10.0 * log10 (data[carrier]);
					}
					else
					{
						data[carrier] = 0;
					}
				}

				// Check the flatness
				double deltaPower = 0;
				double highLimit = 4, lowLimit = -4;


				for (i=carrierFirst;i<=carrierSecond;i++)
				{
					//UPPER SECTION A
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

					//LOWER SECTION A
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
				}

				for (i=carrierThird;i<=carrierFourth;i++)
				{
					//UPPER SECTION A
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

					//LOWER SECTION A
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
				}

				for (i=carrierSixth;i<=carrierFifth;i++)
				{
					//UPPER SECTION A
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionA, carrierNoUpA, &sectionMargin[0], &sectionCarrierNo[0]);

					//LOWER SECTION A
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionA, carrierNoLoA, &sectionMargin[1], &sectionCarrierNo[1]);
				}

				// Continue, Check the flatness. (side)
				highLimit = 4, lowLimit = -6;
				for (i=carrierSeventh;i<=carrierEigth;i++)
				{
					//UPPER SECTION B
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);

					//LOWER SECTION B
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
				}

				for (i=carrierTenth;i<=carrierNinth;i++)
				{
					//UPPER SECTION B
					deltaPower = data[i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, i, lowLimit, highLimit, valueDbUpSectionB, carrierNoUpB, &sectionMargin[2], &sectionCarrierNo[2]);

					//LOWER SECTION B
					deltaPower = data[lengthIQ-i]-avg_center_power;
					CheckFlatnessViolation_All(deltaPower, -i, lowLimit, highLimit, valueDbLoSectionB, carrierNoLoB, &sectionMargin[3], &sectionCarrierNo[3]);
				}
			}
			else if ( bandwidthMhz == 160)
			{
				// not supported yet
			}
			else	// bandwidth = 80_80
			{
				// not supported yet
			}

		}


		// get the highest fail margin
		*failMargin = -99;
		for (i=0 ; i<4 ; i++)
		{
			if (sectionMargin[i]>*failMargin)
			{
				*failMargin = sectionMargin[i];
				*leastMarginCarrier = sectionCarrierNo[i];
			}
		}

		// Get the center frequency leakage 
		//	*loLeakage = data[0] - avg_power - 10 * log10((double)((carrierFourth-carrierFirst+1)*2));

		//for debug only
		if(0)
		{
			//#if defined(_DEBUG)
			// print out the Mask into file for debug				
			FILE *fp;  
			char sigFileName[MAX_BUFFER_SIZE];
			sprintf_s(sigFileName, "log/Log_Flatness_Result_11AC_Stream%02d.csv", streamIndex+1);
			fopen_s(&fp, sigFileName, "w");      
			if (fp)
			{
				fprintf(fp, "WiFi Flatness Carrier,Signal,Avg_Center_Power,Size: %d\n", lengthIQ);    // print to log file 
				for (int carrier=0;carrier<lengthIQ;carrier++)
				{
					fprintf( fp, "%d,%8.2f,%8.2f\n", carrier, data[carrier],
						data[carrier]-avg_center_power);    // print to log file 

				}
				fclose(fp);
			}
			else
			{
				// fopen failed
			}
		}
		//#endif
	}
	else
	{
		// if (dataQ && dataI && data && maxData && minData)
		result = -1;
	}

	if (dataRaw)   free (dataRaw);
	if (data)	   free (data);

	return result;
}



//=============================================================================
//	Description:
//		A generic function for interpolating. Find y_val when xVal is present.
//
//                             (yMax - yMin)
//    Returns y_val = yMin + (--------------- * (xVal - xMin))
//                             (xMax - xMin)
//=============================================================================
inline double IQInterpolate( double yMax, double yMin, double xMax, double xMin, double xVal ) 
{
	return (yMin + ((yMax - yMin) / (xMax - xMin) * (xVal - xMin)));
}

int VerifyPowerMask_All (double *freqBuf, double *maskBuf, int maskBufCnt, int wifiMode, int bssBW, int cbw)		
{
    int passTest = ERR_OK;

    //int    maskTopStart = 999999, maskTopStop = 0;
    double peakPower = NA_NUMBER;
	double *maskWiFi = new double[maskBufCnt];

    double firstRefFreq =  freqBuf[0]/1000000;				// to MHz
    double deltaFreq    = (freqBuf[1]-freqBuf[0])/1000000;	// to MHz
    

    vector<DEFAULT_MASK_STRUCT>::iterator maskBegin_Iter;
    vector<DEFAULT_MASK_STRUCT>::iterator maskEnd_Iter;
    vector<DEFAULT_MASK_STRUCT>::iterator dummy_Iter;

    /*--------------------------*
     *  Looking for peak power  *
     *--------------------------*/
    for (int i=0;i<maskBufCnt;i++)
    {
        if ( maskBuf[i]>peakPower )  
        {   // store the current peak power and keep searching
            peakPower = maskBuf[i];
        }
        else
        {
            // keep searching...
        }
    }

    /*-----------------------*
     *  Select default Mask  *
     *-----------------------*/
	if (wifiMode == WIFI_11B)    //11b
	{
		maskBegin_Iter = l_wifiMask_11b.begin();
        maskEnd_Iter = l_wifiMask_11b.end();
	}
	else if (wifiMode == WIFI_11AG)
	{
		maskBegin_Iter = l_wifiMask_11ag.begin();
        maskEnd_Iter = l_wifiMask_11ag.end();
	}
	else if ((wifiMode == WIFI_11N_MF_HT20) || (wifiMode == WIFI_11N_GF_HT20) )
	{
        maskBegin_Iter = l_wifiMask_11nHt20.begin();
        maskEnd_Iter = l_wifiMask_11nHt20.end();
	}
	else if ((wifiMode == WIFI_11N_MF_HT40) || (wifiMode == WIFI_11N_GF_HT40) )
	{
        maskBegin_Iter = l_wifiMask_11nHt40.begin();
        maskEnd_Iter = l_wifiMask_11nHt40.end();
	}
	else // 11ac
	{
		int tempBW = 0;

		if (  !g_WiFi11ACGlobalSettingParam.ANALYSIS_11AC_MASK_ACCORDING_CBW)
		{
			// Use bssBW to calculate standard mask
			tempBW = bssBW;
		}
		else
		{
			// Use CBW to calculate standard mask
			tempBW = cbw;
		}
		switch (tempBW)
		{
		case BW_20MHZ:
			maskBegin_Iter = l_wifiMask_11acBss20.begin();
            maskEnd_Iter = l_wifiMask_11acBss20.end();
			break;
		case BW_40MHZ:
			maskBegin_Iter = l_wifiMask_11acBss40.begin();
            maskEnd_Iter = l_wifiMask_11acBss40.end();
			break;
		case BW_80MHZ:
			maskBegin_Iter = l_wifiMask_11acBss80.begin();
            maskEnd_Iter = l_wifiMask_11acBss80.end();
			break;
		case BW_160MHZ:
			maskBegin_Iter = l_wifiMask_11acBss160.begin();
            maskEnd_Iter = l_wifiMask_11acBss160.end();
			break;
		case BW_80_80MHZ:
			maskBegin_Iter = l_wifiMask_11acBss80_80.begin();
            maskEnd_Iter = l_wifiMask_11acBss80_80.end();
			break;
		default:
			maskBegin_Iter = l_wifiMask_11acBss20.begin();
            maskEnd_Iter = l_wifiMask_11acBss20.end();
			break;
		}

	}

    /*-------------------------------*
     *  Construct the absolute Mask  *
     *-------------------------------*/
    int    sectionIndex = 0;
	double currentFreq  = 0, xVal = 0, yDeltaPwrdB = 0;

	// Initial the return results
	l_11ACtxMultiVerificationReturn.VIOLATION_PERCENT  = 0;
	for (int x=0;x<MAX_POSITIVE_SECTION;x++)
	{
		l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[x]		= NA_NUMBER;
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[x]	= NA_NUMBER;
		l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[x]		= NA_NUMBER;
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[x] = NA_NUMBER;
	}

    currentFreq = firstRefFreq;	 // point to first one

    for (int i=0;i<maskBufCnt;i++)
    {
        xVal = fabs(currentFreq);
        dummy_Iter = maskBegin_Iter;
		sectionIndex = 0;

        while ( dummy_Iter!=maskEnd_Iter ) // not end of mask vector
        {
            if ( xVal >= dummy_Iter->freqOffset )
            {
                dummy_Iter++;
				sectionIndex++;

                if ( dummy_Iter!=maskEnd_Iter ) // not end of mask vector
                {
                    if ( xVal < dummy_Iter->freqOffset )    // find out the range
                    {   
                        double xMax = dummy_Iter->freqOffset;
                        double yMax = dummy_Iter->deltaMaskPwr;                        
                        dummy_Iter--;
						sectionIndex--;
                        double xMin = dummy_Iter->freqOffset;
                        double yMin = dummy_Iter->deltaMaskPwr;                        
                        
                        yDeltaPwrdB = IQInterpolate( yMax, yMin, xMax, xMin, xVal );

                        //if (dummy_Iter->deltaMaskPwr==0) // This is the top of spectrum
                        //{
                        //    if (i<maskTopStart) maskTopStart = i;
                        //    if (i>maskTopStop)  maskTopStop  = i;
                        //}
                        //else
                        //{
                        //    // do nothing, not top
                        //}
                        break;
                    }
                    else
                    {
                        // not in this Mask range, continue and keep searching
                        continue;
                    }
                }
                else    // already end of vector, thus direct assign mask_Iter->deltaMaskPwr to yDeltaPwrdB
                {
                    dummy_Iter--;
					sectionIndex--;
                    yDeltaPwrdB = dummy_Iter->deltaMaskPwr;   
                    break;
                }
            }
            else
            {
                // no this case, because fabs(currentFreq) should >= 0
            }
        }         

        // Construct the absolute Mask and store into Array
        maskWiFi[i] = peakPower + yDeltaPwrdB;

		/*---------------------------------*
		 *  Report Margin by each section  *
		 *---------------------------------*/		
		if ( sectionIndex==0 )		// This is the "TOP" of spectral, just skip this section.
		{
			// do nothing, moving to next frequency step
		}
		else
		{
			double deltaPower = maskBuf[i] - maskWiFi[i];	// check Mask

			if ( i>(maskBufCnt/2) )		// Upper section
			{
				if ( deltaPower>l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[sectionIndex-1] )	// need to save this as margin
				{
					l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[sectionIndex-1]      = deltaPower;		// save margin		
//					l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[sectionIndex-1] = (freqBuf[i]/1e6) + l_11ACtxVerifyMaskParam.CH_FREQ_MHZ;	// save frequency
					l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[sectionIndex-1] = (freqBuf[i]/1e6);	// save frequency
				
				}
				else
				{
					// do nothing
				}			
			}
			else						// Lower section
			{
				if ( deltaPower>l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[sectionIndex-1] )	// need to save this as margin
				{
					l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[sectionIndex-1]      = deltaPower;		// save margin
//					l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[sectionIndex-1] = (freqBuf[i]/1e6) + l_11ACtxVerifyMaskParam.CH_FREQ_MHZ;	// save frequency
					l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[sectionIndex-1] = (freqBuf[i]/1e6);	// save frequency
				}
				else
				{
					// do nothing
				}	
			}

			if ( deltaPower>0 )  // Mask Failed
			{
				l_11ACtxMultiVerificationReturn.VIOLATION_PERCENT++;
			}
			else    // Mask Pass
			{
				// keep compare...
			}
		}

        // moving to next frequency step
        currentFreq = currentFreq + deltaFreq;
    }   // end - for (int i=0;i<maskBufCnt;i++)

	if ( wifiMode==WIFI_11B )	// 802.11b only have -2,-1,+1,+2 sections
	{
		l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[0]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[1]);		// save margin		
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[0] = l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[1];	// save frequency
		l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[0]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[1]);		// save margin
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[0] = l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[1];	// save frequency

		l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[1]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[3]);		// save margin		
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[1] = l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[3];	// save frequency
		l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[1]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[3]);		// save margin
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[1] = l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[3];	// save frequency

		l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[2]      = NA_NUMBER;				
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[2] = NA_NUMBER;
		l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[2]      = NA_NUMBER;
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[2] = NA_NUMBER;

		l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[3]      = NA_NUMBER;				
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[3] = NA_NUMBER;
		l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[3]      = NA_NUMBER;
		l_11ACtxMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[3] = NA_NUMBER;
	}
	else
	{
		// Turn Values to Positive
		l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[0]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[0]);     
		l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[0]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[0]);     
		
		l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[1]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[1]);     
		l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[1]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[1]);     
														  
		l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[2]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[2]);     
		l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[2]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[2]);     
														  
		l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[3]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_POSITIVE[3]);     
		l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[3]      = -1*(l_11ACtxMultiVerificationReturn.MARGIN_DB_NEGATIVE[3]);     
		
	}


    // Percentage, which fail the mask
	l_11ACtxMultiVerificationReturn.VIOLATION_PERCENT = (l_11ACtxMultiVerificationReturn.VIOLATION_PERCENT/maskBufCnt) * 100.0;

  //  #if defined(_DEBUG)
		//// print out the Mask into file for debug
		//FILE *fp;  
		//fopen_s(&fp, "Log_Mask_Result.csv", "w");    
		//if (fp)
		//{
		//	fprintf(fp, "Frequency,WiFi Mask,Signal,Size: %d\n", maskBufCnt);    // print to log file 
		//	for (int x=0;x<maskBufCnt;x++)
		//	{
		//		fprintf(fp, "%8.2f,%8.2f,%8.2f\n", freqBuf[x], maskWiFi[x], maskBuf[x]);    // print to log file 
		//	}
		//	fclose(fp);
		//}
  //  #endif

    // free memory
	delete [] maskWiFi;

    if (l_11ACtxMultiVerificationReturn.VIOLATION_PERCENT!=0)
    {
        passTest = -1;
    }
    else
    {
        passTest = ERR_OK;
    }

	return passTest;
}

int VerifyOBW_All(double *maskBuf, int maskBufCnt, double obwPercentage, int &startIndex, int &stopIndex)
{

	int passTest = ERR_OK;

	double  totalPower = 0;
	//double  obwPowerLimit = 0;
    double  obwPowerExceed = 0.0;
    double  sumPower = 0.0;


	// Step 1, calculate the total power
	for (int i=0;i<maskBufCnt;i++)
	{
		totalPower = totalPower + pow( 10,(maskBuf[i]/10) );
	}

	// Step 2, calculate the exceed part( >99%) by OBW percantage
    obwPowerExceed = totalPower * (1.0 - obwPercentage);
    obwPowerExceed /= 2.0;

    //Step 3, Set flag from left sideband and right sideband
    startIndex = 0;
    stopIndex  = maskBufCnt - 1;

    //Find the point in 99%, start from left sideband, increase frequency
    sumPower = pow( 10,(maskBuf[startIndex]/10) );
    while((sumPower <= obwPowerExceed) && (startIndex < (maskBufCnt/2)))
    {
        startIndex++;
        sumPower += pow( 10,(maskBuf[startIndex]/10) );
    }

    //Find the point in 99%, start from right sideband, decrease frequency
    sumPower = pow( 10,(maskBuf[stopIndex]/10) );
    while((sumPower <= obwPowerExceed) && (stopIndex > (maskBufCnt/2)))
    {
        stopIndex --;
        sumPower += pow( 10,(maskBuf[stopIndex]/10) );
    }

	return passTest;
}


//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
// 
//-------------------------------------------------------------------------------------
int Check11ACTxAllParameters( int *bssBW, int * cbw,int *bssPchannel,int *bssSchannel,int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// check common input parameters
		 err = CheckCommonParameters_WiFi_11ac(l_11ACtxMultiVerificationParamMap, 
			                                   l_11ACtxMultiVerificationReturnMap,
											   g_WiFi_11ac_Test_ID,
											   bssBW, 
											   cbw, 
											   bssPchannel, 
											   bssSchannel, 
											   wifiMode, 
											   wifiStreamNum, 
											   samplingTimeUs, 
											   cableLossDb);
		
		  if ( ERR_OK!=err )
		  {
			  LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Check Common Input Parameters failed.\n");
			  throw logMessage;
		  }
		  else
		  {
			  LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Check11ACCommonInputParameters() return OK.\n");
		  }
		  
		  
//		// Checking the Input Parameters
//
//		if ( 0!=strcmp(l_11ACtxMultiVerificationParam.PREAMBLE, "SHORT") && 0!=strcmp(l_11ACtxMultiVerificationParam.PREAMBLE, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_11ACtxMultiVerificationParam.PREAMBLE);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0!=strcmp(l_11ACtxMultiVerificationParam.GUARD_INTERVAL, "SHORT") && 0!=strcmp(l_11ACtxMultiVerificationParam.GUARD_INTERVAL, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown GUARD_INTERVAL, WiFi guard interval %s not supported.\n", l_11ACtxMultiVerificationParam.GUARD_INTERVAL);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if (( 0> l_11ACtxMultiVerificationParam.NUM_STREAM_11AC) || ( l_11ACtxMultiVerificationParam.NUM_STREAM_11AC >8))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] NUM_STREAM_11AC only can be 1~9. Please check input paramter \"NUM_STREAM_11AC\"!\n");
//			throw logMessage;
//		}
//		else
//		{
//			// do nothing
//		}
//		
//		/*------------------------------------------*
//		*   Check data rate	and packet format		*
//		*-------------------------------------------*/
//
//		if ( (0!=strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11AC_MF_HT")) && (0!=strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11AC_GF_HT")) &&
//			(0!=strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11N_MF_HT")) && (0!=strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11N_GF_HT")) &&
//			(0!=strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11AC_VHT")) &&( 0!=strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11AC_NON_HT"))
//			&&( 0!=strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "LEGACY_NON_HT")))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown PACKET_FORMAT, WiFi 11ac packet format %s not supported.\n", l_11ACtxMultiVerificationParam.PACKET_FORMAT);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0 == strcmp( l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11AC_NON_HT") &&
//			! strstr ( l_11ACtxMultiVerificationParam.DATA_RATE, "OFDM"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//				"[WiFi_11AC] 11AC_NON_HT, only OFDM-6,9,12,18,24,48,54 supported.\n");
//			throw logMessage;
//		}
//		else
//		{
//		}
//
//		err = TM_WiFiConvertDataRateNameToIndex(l_11ACtxMultiVerificationParam.DATA_RATE, &dummyInt);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_11ACtxMultiVerificationParam.DATA_RATE);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( 0 == strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11AC_VHT"))  // Data rate: MCS0 ~MCS9
//		{
//			if (( 14 <= dummyInt ) && (dummyInt <= 23))
//			{
//				// Data rate is right
//				//do nothing
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] PACKET_FORMAT and DATA_RATE don't match! The data rates of VHT_11AC must be MCS0 ~ MCS9, doesn't support %s!\n", &l_11ACtxMultiVerificationParam.DATA_RATE);
//				throw logMessage;
//
//			}
//
//			
//		}
//		else
//		{
//		}
//
//		if ( !strstr (l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11AC"))    // Legacy signal, CH_BANDWIDTH and CH_FREQ_MHZ must have values
//		{
//			if (( l_11ACtxMultiVerificationParam.CH_BANDWIDTH <= 0) || ( l_11ACtxMultiVerificationParam.CH_FREQ_MHZ <= 0))
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] For legacy mode,CH_BANDWIDTH and CH_FREQ_MHZ must have values! Please check input parameters! \n");
//				throw logMessage;
//
//			}
//			else
//			{
//				// do nothing
//			}
//		}
//		else
//		{
//			// do nothing
//		}
//
//#pragma region  Check bandwidth and channel list
//
//		/*----------------------*
//		*   Check bandwidth		*
//		*-----------------------*/
//		if ( 0!=strcmp(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH, "BW-20") && 0!=strcmp(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH, "BW-40") && 
//			 0!=strcmp(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH, "BW-80") && 0!=strcmp(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH, "BW-160") &&
//			 0!=strcmp(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH, "BW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACtxMultiVerificationParam.BSS_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH, "BW-20"))
//			{
//				*bssBW = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH, "BW-40"))
//			{
//				*bssBW = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH, "BW-80"))
//			{
//				*bssBW = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH, "BW-160"))
//			{
//				*bssBW = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxMultiVerificationParam.BSS_BANDWIDTH, "BW-80_80"))
//			{
//				*bssBW = BW_80_80MHZ;
//
//				if ( l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
//				{
//					err = -1;
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//						"[WiFi_11AC]  BSS_BANDWIDTH = BW_80_80MHZ,\"BSS_FREQ_MHZ_SECONDARY\" must have value! Please check input parameters!\n" );
//					throw logMessage;
//				}
//				else
//				{
//					// do nothing
//				}
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACtxMultiVerificationParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//
//		}
//
//		if ( 0 != strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "0") && 
//			0!=strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "CBW-20") && 0!=strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "CBW-40") && 
//			 0!=strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "CBW-80") && 0!=strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "CBW-160") &&
//			 0!=strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "CBW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown CBW BANDWIDTH, WiFi 11ac CBW bandwidth %s not supported.\n", l_11ACtxMultiVerificationParam.CH_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "0"))
//			{
//				sprintf_s(l_11ACtxMultiVerificationParam.CH_BANDWIDTH,MAX_BUFFER_SIZE,"C%s",l_11ACtxMultiVerificationParam.BSS_BANDWIDTH);
//				*cbw = *bssBW;
//			}
//			else if ( 0 == strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "CBW-20"))
//			{
//				*cbw = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "CBW-40"))
//			{
//				*cbw = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "CBW-80"))
//			{
//				*cbw = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "CBW-160"))
//			{
//				*cbw = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxMultiVerificationParam.CH_BANDWIDTH, "CBW-80_80"))
//			{
//				*cbw = BW_80_80MHZ;
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACtxMultiVerificationParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//		}
//	
//		if ( strstr ( l_11ACtxMultiVerificationParam.PREAMBLE,"11AC"))  //802.11ac, cbw can't be larger than bssBW
//		{
//			if (*bssBW != BW_80_80MHZ)
//			{
//				if (*cbw > *bssBW)
//				{
//					err = -1;
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Channel bandwidth can not be wider than BSS bandwidth.\n");
//					throw logMessage;
//				}
//				else 
//				{
//					// do nothing
//				}
//			}
//			else  //bssBW = BW_80_80MHZ
//			{
//				if ( *cbw == BW_160MHZ)
//				{
//					err = -1;
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] BBS bandwidth = 80+80MHz.Channel bandwidth can't be 160MHz.\n");
//					throw logMessage;
//				}
//				else
//				{
//					// do nothing
//				}
//			}
//		}
//		else   // only need cbw, no use of bssBW
//		{
//			// do nothing
//		}
//
//
//
//		//check cbw
//		if ( strstr( l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11N"))  //802.11n, BW= 20,40MHz
//		{
//			if ( *cbw != BW_20MHZ && *cbw != BW_40MHZ)
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//						"[WiFi_11AC]  11N bandwidth should be 20MHz or 40Mhz. Please check input parameters!\n" );
//				throw logMessage;
//			}
//			else
//			{
//				// do nothing
//			}
//		}
//		else if ( 0 == strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "LEGACY_NON_HT"))  // 11B and 11ag
//		{
//			if ( *cbw != BW_20MHZ)
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//						"[WiFi_11AC]  11B and 11AG bandwidth should be 20MHz. Please check input parameters!\n" );
//				throw logMessage;
//			}
//			else
//			{
//				// do nothing
//			}
//		}
//		else		// 802.11 ac
//		{
//			if ( 0 == strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11AC_VHT"))
//			{
//				//do nothing
//			}
//			else if ( 0 == strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11AC_MF_HT") ||
//				0 == strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11AC_GF_HT"))
//			{
//				if ( *cbw != BW_20MHZ && *cbw != BW_40MHZ)
//				{
//					err = -1;
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//							"[WiFi_11AC]  11AC_MF(GF)_HT bandwidth should be 20MHz or 40Mhz. Please check input parameters!\n" );
//					throw logMessage;
//				}
//				else
//				{
//					// do nothing
//				}
//			}
//			else // 11AC_NON_HT
//			{
//				if (*cbw == BW_20MHZ)
//				{
//					err = -1;
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//							"[WiFi_11AC]  11AC_NON_HT bandwidth can't be 20MHz. Please check input parameters!\n" );
//					throw logMessage;
//				}
//				else
//				{
//				}
//			}
//		}
//
//		/*--------------------------*
//		*   Check BSS frequency		*
//		*---------------------------*/
//		if (l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY <= 0)
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//					"[WiFi_11AC]  \"BSS_FREQ_MHZ_PRIMARY\" must have value! Please check input parameters!\n" );
//			throw logMessage;
//
//		}
//		else
//		{
//			// do nothing
//		}
//
//		err = TM_WiFiConvertFrequencyToChannel(l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY, bssPchannel);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( *bssBW == BW_80_80MHZ)  // Need BSS_FREQ_MHZ_SECONDARY
//		{
//			if ( l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//					"[WiFi_11AC]  BSS_BANDWIDTH = BW_80_80MHZ,\"BSS_FREQ_MHZ_SECONDARY\" must have value!\n" );
//				throw logMessage;
//			}
//			else
//			{
//					// do nothing
//			}
//
//			err = TM_WiFiConvertFrequencyToChannel(l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_SECONDARY, bssSchannel);      
//			if ( ERR_OK!=err )
//			{
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_SECONDARY);
//				throw logMessage;
//			}
//			else
//			{
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//			}
//
//		}
//		else   // Don't need BSS_FREQ_MHZ_SECONDARY
//		{
//			bssSchannel = 0;
//		}
//
//		
//		/*-------------------------------------------------------------------*
//		* Find channel list.											 	 *
//		* If CH_FREQ_MHz is not zero, we will directly use it to do caputre. *
//		* if not, we will "CH_FREQ_MHZ_PRIMARY_20" and "BSS_BANDWIDTH"		 *
//		* to calculate the freuqency to capture								 *
//		*--------------------------------------------------------------------*/
//
//		if ( strstr (l_11ACtxMultiVerificationParam.PACKET_FORMAT, "11AC"))
//		{
//
//			if ( 0 == l_11ACtxMultiVerificationParam.CH_FREQ_MHZ)  
//			{
//				//no CH_FREQ_MHZ, no CH_FREQ_MHZ_PRIMARY_20MHz
//				// Use default values
//				if ( 0 == l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz) 
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi_11AC] Don't have input parameters \"CH_FREQ_MHZ\" or \"CH_FREQ_MHZ_PRIMARY_20\", using BBS center freuqency as default!");
//					l_11ACtxMultiVerificationParam.CH_FREQ_MHZ = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY;
//
//					// all  use lower frequency for channel list
//					switch (*bssBW)
//					{
//					case BW_20MHZ:
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_40MHZ:
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 10;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_80MHZ:
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 20;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY;
//						break;
//					
//					case BW_160MHZ:			
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//				
//					case BW_80_80MHZ:				
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//
//					default:
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					}
//
//				}
//				else  //no CH_FREQ_MHZ, have CH_FREQ_MHZ_PRIMARY_20MHz. Use input CH_FREQ_MHZ_PRIMARY_20MHz to calculate
//				{
//					err = GetChannelList(*bssBW, l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY,l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//					if ( err != ERR_OK) // Wrong channel list
//					{
//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_11ACtxMultiVerificationParam.PACKET_FORMAT);
//						throw logMessage;
//					}
//					else  // Get channel list successfully
//					{
//					}
//				}
//
//				if (*cbw != BW_80_80MHZ)
//				{
//					if (*cbw == *bssBW)
//					{
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//					else if (*cbw == BW_20MHZ)
//					{
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ = l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz;
//					}
//					else if (*cbw == BW_40MHZ)
//					{
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ = l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
//					}
//					else if (*cbw == BW_80MHZ)
//					{
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ = l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
//					}
//					else
//					{
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//				}
//				else // cbw = BW_80_80MHZ, use BSS primary and secondary
//				{
//					l_11ACtxMultiVerificationParam.CH_FREQ_MHZ = NA_INTEGER;
//					// do nothing
//				}
//			}
//			else  // CH_FREQ_MHZ: non-zero 
//			{
//				//Check if input CH_FREQ_MHZ is correct
//				err = CheckChannelFreq(*bssBW, *cbw,
//					l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY,l_11ACtxMultiVerificationParam.CH_FREQ_MHZ);
//				
//				if (err != ERR_OK)
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input paramter \"CH_FREQ_MHZ\" wrong. Please check!\n");
//					throw logMessage;
//				}
//				else
//				{
//				}
//
//
//				// Check if input CH_FREQ_MHZ_PRIMARY_20MHz.
//				// if zero, calculate by default
//				// If non-zero and correct, use it directly
//				// If not-zero, but incorrect, throw error
//				if ( 0 != l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz)
//				{
//					err = CheckChPrimary20(*bssBW, *cbw,
//							l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY,l_11ACtxMultiVerificationParam.CH_FREQ_MHZ,l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz);
//					if ( err = ERR_OK)  //input CH_FREQ_MHZ_PRIMARY_20MHz is correct
//					{
//						err = GetChannelList(*bssBW, l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY,l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//							&l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//						if ( err != ERR_OK) // Wrong channel list
//						{
//							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Can't get channel list. Please check input paramters.\n", l_11ACtxMultiVerificationParam.PACKET_FORMAT);
//							throw logMessage;
//						}
//						else  // Get channel list successfully
//						{
//						}
//					}
//					else
//					{
//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Input paramter \"CH_FREQ_MHZ_PRIMARY_20MHz\" wrong. Please check!\n");
//						throw logMessage;
//					}
//				}
//				else //input CH_FREQ_MHZ_PRIMARY_20MHz is zero
//				{
//					//Get channel primary20
//					switch (*bssBW)
//					{
//					case BW_20MHZ:
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					case BW_40MHZ:  //cbw = BW_20.40MHz
//						if (*cbw == BW_20MHZ)
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 10;
//
//						}
//						else  //wrong cbw
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80MHZ:  //cbw = BW_20,40,80MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_11ACtxMultiVerificationParam.CH_FREQ_MHZ == l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_11ACtxMultiVerificationParam.CH_FREQ_MHZ == l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  //wrong cbw
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz =  NA_INTEGER;
//
//						}
//						break;
//					case BW_160MHZ:  //cbw = BW_20,40,80,160MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_11ACtxMultiVerificationParam.CH_FREQ_MHZ == l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 60)
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_11ACtxMultiVerificationParam.CH_FREQ_MHZ == l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_11ACtxMultiVerificationParam.CH_FREQ_MHZ == l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else if ( l_11ACtxMultiVerificationParam.CH_FREQ_MHZ == l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY + 60)
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY + 50;
//							}
//							else  //wrong CH_FREQ_MHZ
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							if (l_11ACtxMultiVerificationParam.CH_FREQ_MHZ == l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 40)
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_11ACtxMultiVerificationParam.CH_FREQ_MHZ == l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY + 40)
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_160MHZ)
//						{
//							if (l_11ACtxMultiVerificationParam.CH_FREQ_MHZ == l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY)
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else  //wring cbw
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80_80MHZ:
//						if ( *cbw == BW_20MHZ)
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if ( l_11ACtxMultiVerificationParam.CH_FREQ_MHZ == l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_11ACtxMultiVerificationParam.CH_FREQ_MHZ == l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else if ( *cbw == BW_80_80MHZ)
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  // wrong CH_FREQ_MHz
//						{
//							l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					default:
//						l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						break;
//					}
//				}
//
//				err = GetChannelList(*bssBW, l_11ACtxMultiVerificationParam.BSS_FREQ_MHZ_PRIMARY,l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//				if ( err != ERR_OK)  //Get channel list wrong
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_11ACtxMultiVerificationParam.PACKET_FORMAT);
//					throw logMessage;
//				}
//				else  //Get channel list successfully
//				{
//				}
//			}	
//
//			//Check channel list. If all are "NA_INTEGER", return error
//			if (( l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz == NA_INTEGER ) && 
//				(l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz == NA_INTEGER ) &&
//				(l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER))
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20/40/80MHz, please check input parameters.\n");
//				throw logMessage;
//			}
//			else
//			{
//				// do nothing
//			}
//		}
//		else   // non-ac, no use of CH_FREQ_MHZ_PRIMARY_20/40/80MHz
//		{
//			l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//			l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//			l_11ACtxMultiVerificationReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//		}
//#pragma endregion
//		
//		//VHT, cbw = BW_20MHZ, the stream number of MCS9 only can 3,6
//		if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )
//		{
//			if ( (l_11ACtxMultiVerificationParam.NUM_STREAM_11AC != 3) &&
//				 (l_11ACtxMultiVerificationParam.NUM_STREAM_11AC != 6) )
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] 802.11ac CBW = 20MHz, Data Rate = MCS9,NUM_STREAM_11AC only can be 3,6!\n");
//				throw logMessage;
//			}
//			else
//			{
//			}
//		}
//		else
//		{
//			// do nothing
//		}
//
//		// Convert parameter
//		err = WiFi_11ac_TestMode(l_11ACtxMultiVerificationParam.DATA_RATE, cbw, wifiMode, wifiStreamNum, l_11ACtxMultiVerificationParam.PACKET_FORMAT);
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Convert WiFi test mode function failed.\n");
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] WiFiTestMode() return OK.\n");
//		}
//		
//
//		// Check path loss (by ant and freq)
//		err = CheckPathLossTableExt(	g_WiFi_11ac_Test_ID,
//										l_11ACtxMultiVerificationParam.CH_FREQ_MHZ,
//								  l_11ACtxMultiVerificationParam.TX1,
//								  l_11ACtxMultiVerificationParam.TX2,
//								  l_11ACtxMultiVerificationParam.TX3,
//								  l_11ACtxMultiVerificationParam.TX4,
//										l_11ACtxMultiVerificationParam.CABLE_LOSS_DB,
//										l_11ACtxMultiVerificationReturn.CABLE_LOSS_DB,
//										cableLossDb,
//										TX_TABLE
//									);
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to get TX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] CheckPathLossTableExt(TX_TABLE) return OK.\n");
//		}
//
//		// Check sampling time 
//		if (0==l_11ACtxMultiVerificationParam.SAMPLING_TIME_US)
//		{
//			*samplingTimeUs = CheckSamplingTime(*wifiMode, l_11ACtxMultiVerificationParam.PREAMBLE, l_11ACtxMultiVerificationParam.DATA_RATE, l_11ACtxMultiVerificationParam.PACKET_FORMAT);
//		}
//		else	// SAMPLING_TIME_US != 0
//		{
//			*samplingTimeUs = l_11ACtxMultiVerificationParam.SAMPLING_TIME_US; //TBD - check if this works
//		}
		 
		 
		  // check unique input parameters used only for TX_Multi_Verification test ... 
		  // to do ....

		  /*-------------------------------------*
		  *  set PAPR values for vsa setting     *
		  *--------------------------------------*/
		  if ( *wifiMode==WIFI_11B )  
			  *peakToAvgRatio = g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_11M;    // CCK        
		  else                        
			  *peakToAvgRatio = g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_54M;   // OFDM 


		// Check Dut configuration changed or not
		if (  g_WiFi11ACGlobalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.
			  l_11ACtxMultiVerificationParam.CH_FREQ_MHZ != g_RecordedParam.CH_FREQ_MHZ ||
			  0!=strcmp(l_11ACtxMultiVerificationParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
			  0!=strcmp(l_11ACtxMultiVerificationParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
			  0!=strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, g_RecordedParam.PACKET_FORMAT) ||
			  0!=strcmp(l_11ACtxMultiVerificationParam.PACKET_FORMAT, g_RecordedParam.PACKET_FORMAT) ||
			  0!=strcmp(l_11ACtxMultiVerificationParam.GUARD_INTERVAL, g_RecordedParam.GUARD_INTERVAL_11N) ||
			  l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  l_11ACtxMultiVerificationParam.CH_BANDWIDTH!=g_RecordedParam.CH_BANDWIDTH ||
			  l_11ACtxMultiVerificationParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
			  l_11ACtxMultiVerificationParam.TX1!=g_RecordedParam.ANT1 ||
			  l_11ACtxMultiVerificationParam.TX2!=g_RecordedParam.ANT2 ||
			  l_11ACtxMultiVerificationParam.TX3!=g_RecordedParam.ANT3 ||
			  l_11ACtxMultiVerificationParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;			
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup
		g_RecordedParam.ANT1					= l_11ACtxMultiVerificationParam.TX1;
		g_RecordedParam.ANT2					= l_11ACtxMultiVerificationParam.TX2;
		g_RecordedParam.ANT3					= l_11ACtxMultiVerificationParam.TX3;
		g_RecordedParam.ANT4					= l_11ACtxMultiVerificationParam.TX4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_11ACtxMultiVerificationParam.CABLE_LOSS_DB[3];	
		g_RecordedParam.CH_FREQ_MHZ				= l_11ACtxMultiVerificationParam.CH_FREQ_MHZ;
	//	g_RecordedParam.CH_FREQ_MHZ_SECONDARY_80	= l_11ACtxMultiVerificationParam.CH_FREQ_MHZ_SECONDARY_80;
		g_RecordedParam.POWER_DBM				= l_11ACtxMultiVerificationParam.TX_POWER_DBM;	

		sprintf_s(g_RecordedParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, l_11ACtxMultiVerificationParam.CH_BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_11ACtxMultiVerificationParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_11ACtxMultiVerificationParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT, MAX_BUFFER_SIZE, l_11ACtxMultiVerificationParam.PACKET_FORMAT);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_11ACtxMultiVerificationParam.GUARD_INTERVAL);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11AC] CheckTxEvmParameters() Confirmed.\n");
	}
	catch(char *msg)
    {
        sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
    }
    catch(...)
    {
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11AC] Unknown Error!\n");
		err = -1;
    }

	
	return err;
}

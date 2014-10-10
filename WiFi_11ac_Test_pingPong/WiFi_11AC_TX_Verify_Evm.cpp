#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"
#include <math.h> // fmod on mac

using namespace std;

// These global variables are declared in WiFi_Test_Internal.cpp
extern TM_ID				g_WiFi_11ac_Test_ID;    
extern vDUT_ID				g_WiFi_11ac_Dut;
extern int					g_Tester_Type;
extern int					g_Tester_Number;
extern bool					g_vDutTxActived;
extern bool					g_dutConfigChanged;
extern WIFI_RECORD_PARAM	g_RecordedParam;

// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_WiFi11ACGlobalSettingParam;
extern int      g_Tester_DualHead;// add by daixin

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACtxVerifyEvmParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_11ACtxVerifyEvmReturnMap;

struct tagParam
{
    // Mandatory Parameters                       
    int		BSS_FREQ_MHZ_PRIMARY;					/*!< For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment */
	int		BSS_FREQ_MHZ_SECONDARY;					/*!< For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined */
	int		CH_FREQ_MHZ_PRIMARY_20MHz;				/*!< The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ"   */
	int		CH_FREQ_MHZ;							/*!< The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used  */
	int		NUM_STREAM_11AC;						/*!< The number of streams 11AC (Only). */
    char	BSS_BANDWIDTH[MAX_BUFFER_SIZE];			/*!< The RF bandwidth of basic service set (BSS) to verify EVM. */
	char	CH_BANDWIDTH[MAX_BUFFER_SIZE];			/*!< The RF channel bandwidth to verify EVM. */
    char	DATA_RATE[MAX_BUFFER_SIZE];				/*!< The data rate to verify EVM. */
	char	PREAMBLE[MAX_BUFFER_SIZE];				/*!< The preamble type of 11B(only). */
	char	PACKET_FORMAT[MAX_BUFFER_SIZE];			/*!< The packet format of 11AC and 11N. */
	char   GUARD_INTERVAL[MAX_BUFFER_SIZE];			/*! The guard interval format of 11N/AC(only). */
	char	STANDARD[MAX_BUFFER_SIZE];				/*!< The standard parameter used for signal analysis option or to discriminate the same data rates/package formats from different standards */
    double	TX_POWER_DBM;							/*!< The output power to verify EVM. */
    double	CABLE_LOSS_DB[MAX_DATA_STREAM];			/*!< The path loss of test system. */
    double	SAMPLING_TIME_US;                       /*!< The sampling time to verify EVM. */
    // DUT Parameters
    int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on) */
    int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */
    int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */
} l_11ACtxVerifyEvmParam;

struct tagReturn
{	
	double TX_POWER_DBM;							/*!< TX Power dBm Setting */
													
   // EVM Test Result                       		
    double EVM_AVG_DB;                      		/*!<  Overall EVM test result, averaging over different stream */
    double EVM_AVG[MAX_DATA_STREAM];        		/*!< (Average) EVM test result on specific data stream. Format: EVM_AVG[SpecificStream] */
    double EVM_MAX[MAX_DATA_STREAM];        		/*!< (Maximum) EVM test result on specific data stream. Format: EVM_MAX[SpecificStream] */
    double EVM_MIN[MAX_DATA_STREAM];        		/*!< (Minimum) EVM test result on specific data stream. Format: EVM_MIN[SpecificStream] */
													
	// EVM_PK Test Result                   		
    double EVM_PK_DB;								/*!< (11b only)EVM_PK over captured packets. */                  
													
	// POWER Test Result                    		
    double POWER_AVG_DBM;                   		
    double POWER_AVG[MAX_DATA_STREAM];      		/*!< (Average) POWER test result on specific data stream. Format: POWER_AVG[SpecificStream] */
    double POWER_MAX[MAX_DATA_STREAM];      		/*!< (Maximum) POWER test result on specific data stream. Format: POWER_MAX[SpecificStream] */
    double POWER_MIN[MAX_DATA_STREAM];      		/*!< (Minimum) POWER test result on specific data stream. Format: POWER_MIN[SpecificStream] */
	// Frequency error                      		
    double FREQ_ERROR_AVG;                  		/*!< (Average) Frequency Error, unit is ppm */
    double FREQ_ERROR_MAX;                  		/*!< (Maximum) Frequency Error, unit is ppm */
    double FREQ_ERROR_MIN;                  		/*!< (Minimum) Frequency Error, unit is ppm */
													
	double AMP_ERR_DB;								/*!< IQ Match Amplitude Error in dB. */
	double PHASE_ERR;								/*!< IQ Match Phase Error. */
	double SYMBOL_CLK_ERR;							/*!< Symbol clock error */
	double PHASE_NOISE_RMS_ALL;                        /*!< Frequency RMS Phase Noise. */ 

	double AMP_ERR_DB_STREAM[MAX_DATA_STREAM];	   	/*!< IQ Match Amplitude Error in dB. per stream*/
	double PHASE_ERR_STREAM[MAX_DATA_STREAM];	   	/*!< IQ Match Phase Error. per stream*/
                                                
	int	   SPATIAL_STREAM;	                    
	double DATA_RATE;                           
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
                                                
	//channel list                              
	int    CH_FREQ_MHZ_PRIMARY_40MHz;       		/*!< The center frequency (MHz) for primary 40 MHZ channel  */
	int    CH_FREQ_MHZ_PRIMARY_80MHz;       		/*!< The center frequency (MHz) for primary 80 MHZ channel  */

    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_11ACtxVerifyEvmReturn;
#pragma endregion

#ifndef WIN32
int init11ACTxVerifyEvmContainer = Initialize11ACTXVerifyEvmContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Evm.cpp
int Check11ACTxEvmParameters( int *bssBW, 
						  int *cbw,
						  int *bssPchannel, 
						  int *bssSchannel,
						  int *wifiMode, 
						  int *wifiStreamNum, 
						  double *samplingTimeUs, 
						  double *cableLossDb,
						  double *peakToAvgRatio, 
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

WIFI_11AC_TEST_API int WIFI_11AC_TX_Verify_Evm(void)
{
    int		err = ERR_OK;
    int		avgIteration		= 0;
    int		bssPchannel			= 0;
	int		bssSchannel			= 0;	
    int		dummyValue			= 0;
    int		wifiMode			= 0;
	int		wifiStreamNum		= 0;
	int		VSAanylisisFormat	= 0;
	int		antSelection		= 0;
	int		bssBW				= 0;		//
	int		cbw					= 0;		//
	int		VSAcaptureMode		= 0;		// 0: 20MHz		1: 40MHz	2:80MHz		3: 160MHz		4: 80_80MHz
	
	bool	analysisOK			= false;	//
	bool	captureOK			= false;	//
	
	double	samplingTimeUs		= 0;		//
	double	peakToAvgRatio		= 0;		//
	double	cableLossDb			= 0;		//
	
	char	vErrorMsg[MAX_BUFFER_SIZE]			= {'\0'};
	char	logMessage[MAX_BUFFER_SIZE]			= {'\0'};
	char	sigFileNameBuffer[MAX_BUFFER_SIZE]	= {'\0'};


    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_11ACtxVerifyEvmReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_11ACtxVerifyEvmParamMap);
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
        RespondToQueryReturn(l_11ACtxVerifyEvmReturnMap);
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
		err = GetInputParameters(l_11ACtxVerifyEvmParamMap);
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

		err = Check11ACTxEvmParameters( &bssBW, &cbw,&bssPchannel,&bssSchannel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
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
#if 1 //ddxx
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
#endif
#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frequency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_11ac_Dut);

		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_FREQ_MHZ_PRIMARY",		l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_FREQ_MHZ_SECONDARY",		l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ",				l_11ACtxVerifyEvmParam.CH_FREQ_MHZ);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_20MHz",	l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "NUM_STREAM_11AC",			l_11ACtxVerifyEvmParam.NUM_STREAM_11AC);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX1",						l_11ACtxVerifyEvmParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX2",						l_11ACtxVerifyEvmParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX3",						l_11ACtxVerifyEvmParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX4",						l_11ACtxVerifyEvmParam.TX4);
	
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_1",			l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[0]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_2",			l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[1]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_3",			l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[2]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "CABLE_LOSS_DB_4",			l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[3]);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "SAMPLING_TIME_US",			l_11ACtxVerifyEvmParam.SAMPLING_TIME_US);	//samplingTimeUs
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "TX_POWER_DBM",				l_11ACtxVerifyEvmParam.TX_POWER_DBM);

		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "BSS_BANDWIDTH",				l_11ACtxVerifyEvmParam.BSS_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "CH_BANDWIDTH",				l_11ACtxVerifyEvmParam.CH_BANDWIDTH);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "DATA_RATE",					l_11ACtxVerifyEvmParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "PACKET_FORMAT",				l_11ACtxVerifyEvmParam.PACKET_FORMAT);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "GUARD_INTERVAL",			l_11ACtxVerifyEvmParam.GUARD_INTERVAL);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "PREAMBLE",					l_11ACtxVerifyEvmParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "STANDARD",					l_11ACtxVerifyEvmParam.STANDARD);



		// the following parameters are not input parameters, why are they here?  6-20-2012 Jacky 
		//vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_40MHz",	l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz);
		//vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_80MHz",	l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz);

		if (( wifiMode== WIFI_11N_GF_HT40 ) || ( wifiMode== WIFI_11N_MF_HT40 )
			|| ( wifiMode== WIFI_11AC_GF_HT40 ) || ( wifiMode== WIFI_11AC_MF_HT40 )
			|| ( wifiMode== WIFI_11AC_VHT40 ) )
		{
			//vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ",			l_11ACtxVerifyEvmParam.CH_FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "PRIMARY_FREQ",			l_11ACtxVerifyEvmParam.CH_FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "SECONDARY_FREQ",			l_11ACtxVerifyEvmParam.CH_FREQ_MHZ+10);
		}
		else
		{
			//do nothing
		}		
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_BANDWIDTH",		bssBW);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_BANDWIDTH",		cbw);
		

        

		/*----------------------*
		*   VSA capture mode	*
		*   0: 20MHz			*
		*   0/1: 40MHz			*
		*	0: 80MHz			*
		*	?: 160MHz 			*
		*	?: 80_80MHz			*
		*-----------------------*/
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

		/*------------------------------------------*
		*	VSA analysis packet format				*					
		*	0: auto detect							*
		*	1: mixed format (11n)					*
		*	2: greenfield format (11n)				*
		*	3: legacy fomat ( 11a/g,11ac Non-HT)	*
		*	4: VHT format ( 11ac, VHT only)			*
		*-------------------------------------------*/

		if ( strstr ( l_11ACtxVerifyEvmParam.PACKET_FORMAT, PACKET_FORMAT_HT_MF))	// mixed format, 11n
		{
			VSAanylisisFormat = 1;
		}
		else if ( strstr ( l_11ACtxVerifyEvmParam.PACKET_FORMAT, PACKET_FORMAT_HT_GF))	 // greenfield format, 11n
		{
			VSAanylisisFormat = 2;
		}
		else if ( strstr ( l_11ACtxVerifyEvmParam.PACKET_FORMAT, PACKET_FORMAT_VHT))	// 11ac
		{
			VSAanylisisFormat = 4;
		}
		else if ( strstr( l_11ACtxVerifyEvmParam.PACKET_FORMAT, PACKET_FORMAT_NON_HT))	// 11ag
		{
			VSAanylisisFormat = 3;
		}
		else
		{
			VSAanylisisFormat = 0;
		}
		VSAanylisisFormat = 0; //debug for Ralink RT85592
		double chainGain = 0;
		int iChainGain = 0;
		chainGain = 10*log10((double)(l_11ACtxVerifyEvmParam.TX1 + l_11ACtxVerifyEvmParam.TX2 + l_11ACtxVerifyEvmParam.TX3 + l_11ACtxVerifyEvmParam.TX4));
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

		err = ::LP_SetVsa(  l_11ACtxVerifyEvmParam.CH_FREQ_MHZ*1e6,
							l_11ACtxVerifyEvmParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGain,
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
		 * Start While Loop *
		 *------------------*/
		avgIteration = 0;
		while ( avgIteration<g_WiFi11ACGlobalSettingParam.EVM_AVERAGE )
		{
			analysisOK = false;
			captureOK  = false;

		   /*----------------------------*
			* Perform Normal VSA capture *
			*----------------------------*/

			/*------------------------------------------------------------*/
			/*For EVM Analysis, in HT20/HT40, using normal capture     ---*/
			/*------------------------------------------------------------*/


			//g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_TYPE
			err = ::LP_VsaDataCapture( samplingTimeUs/1000000, g_WiFi11ACGlobalSettingParam.VSA_TRIGGER_TYPE, 160e6, VSAcaptureMode );     
			if ( ERR_OK!=err )
			{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to capture signal.\n");
					throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_VsaDataCapture() return OK.\n");
			}


			/*--------------*
			 *  Capture OK  *
			 *--------------*/
			captureOK = true;
			if (1==g_WiFi11ACGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
			{
				// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_SaveAlways", l_11ACtxVerifyEvmParam.CH_FREQ_MHZ, l_11ACtxVerifyEvmParam.DATA_RATE, l_11ACtxVerifyEvmParam.CH_BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
			}
			else
			{
				// do nothing
			}



			/*----------------*
			 *  EVM Analysis  *
			 *----------------*/
			switch(wifiMode)
			{

#pragma region Analysis_802_11b

			case WIFI_11B:   
				{
					// [Case 01]: 802.11b Analysis
					err = ::LP_Analyze80211b(   g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_EQ_TAPS,
						g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG,
						g_WiFi11ACGlobalSettingParam.ANALYSIS_11B_METHOD_11B
						);
					if (ERR_OK!=err)
					{	// Fail Analysis, thus save capture (Signal File) for debug
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211b_Failed", l_11ACtxVerifyEvmParam.CH_FREQ_MHZ, l_11ACtxVerifyEvmParam.DATA_RATE, l_11ACtxVerifyEvmParam.CH_BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);

						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_Analyze80211b() return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_Analyze80211b() return OK.\n");
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
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_Analyze80211ag_Failed", l_11ACtxVerifyEvmParam.CH_FREQ_MHZ, l_11ACtxVerifyEvmParam.DATA_RATE, l_11ACtxVerifyEvmParam.CH_BANDWIDTH);
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
								l_11ACtxVerifyEvmParam.NUM_STREAM_11AC,
								cbw, 
								l_11ACtxVerifyEvmParam.DATA_RATE, 
								l_11ACtxVerifyEvmParam.PREAMBLE, 
								l_11ACtxVerifyEvmParam.PACKET_FORMAT, 
								l_11ACtxVerifyEvmParam.GUARD_INTERVAL, 
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
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Evm_AnalyzeMIMO_Failed", l_11ACtxVerifyEvmParam.CH_FREQ_MHZ, l_11ACtxVerifyEvmParam.DATA_RATE, l_11ACtxVerifyEvmParam.CH_BANDWIDTH);
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
#pragma region Analysis_802_11ac_mimo And Retrieve Result

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
						if ( l_11ACtxVerifyEvmParam.NUM_STREAM_11AC==1 )  // MCS0 ~ MCS7
						{        
							sprintf_s(analyzeMimoMode, "nxn");
							sprintf_s(referenceFileName, ""); 
						}
						else // MCS8 ~ MCS15 or more, need MIMO reference File *.iqref
						{
							sprintf_s(analyzeMimoMode, "composite");
							err = GetDefaultWaveformFileName(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PATH, 
								"iqref", 
								wifiMode, 
								l_11ACtxVerifyEvmParam.NUM_STREAM_11AC,
								cbw, 
								l_11ACtxVerifyEvmParam.DATA_RATE, 
								l_11ACtxVerifyEvmParam.PREAMBLE, 
								l_11ACtxVerifyEvmParam.PACKET_FORMAT, 
								l_11ACtxVerifyEvmParam.GUARD_INTERVAL, 
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
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_11AC_TX_Evm_AnalyzeMIMO_Failed", l_11ACtxVerifyEvmParam.CH_FREQ_MHZ, l_11ACtxVerifyEvmParam.DATA_RATE, l_11ACtxVerifyEvmParam.CH_BANDWIDTH);
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

			default:
				err = ERR_INVALID_ANALYSIS_TYPE;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Invalid WiFi mode used for Analysis!!\n");
				throw logMessage;
				
			}

#pragma endregion
#pragma region Retrieve analysis Results
			/*-----------------------------*
			 *  Retrieve analysis Results  *
			 *-----------------------------*/
			char sTestDetail[MAX_BUFFER_SIZE] = {'\0'};
			sprintf_s(sTestDetail, MAX_BUFFER_SIZE, "WiFi_TX_Verify_Evm_%d_%s", l_11ACtxVerifyEvmParam.CH_FREQ_MHZ, l_11ACtxVerifyEvmParam.DATA_RATE);

			avgIteration++;
			analysisOK = true;

			// Store the result, includes EVM, power etc.              
			if ( (wifiMode==WIFI_11B)||(wifiMode==WIFI_11AG) )        // 802.11a/b/g, but not n mode, only one stream
			{
				// Number of spatial streams
				l_11ACtxVerifyEvmReturn.SPATIAL_STREAM = 1;

				// EVM
				evmAvgAll[0][avgIteration-1] = ::LP_GetScalarMeasurement("evmAll",0); 
				if ( -99.00 >= evmAvgAll[0][avgIteration-1] )
				{
					analysisOK = false;
					evmAvgAll[0][avgIteration-1] = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(evmAll) return error.\n");
					throw logMessage;
				}
				
				// Peak EVM
				if (wifiMode==WIFI_11B)
				{
					evmPk[avgIteration-1] = ::LP_GetScalarMeasurement("evmPk", 0);
					if ( -99.00 >= evmPk[avgIteration-1] )
					{
						analysisOK = false;
						evmPk[avgIteration-1] = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(evmPk) return error.\n");
						throw logMessage;
					}
				}

				// Power 
				rxRmsPowerDb[0][avgIteration-1] = ::LP_GetScalarMeasurement("rmsPowerNoGap",0);
				if ( -99.00 >= rxRmsPowerDb[0][avgIteration-1] )
				{
					analysisOK = false;
					rxRmsPowerDb[0][avgIteration-1] = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(rmsPowerNoGap) return error.\n");
					throw logMessage;
				}
				else
				{
					// One stream data rate, should be only one antenna is ON	
					int antenaOrder = 0;
					err = CheckAntennaOrderByStream(l_11ACtxVerifyEvmReturn.SPATIAL_STREAM, l_11ACtxVerifyEvmParam.TX1, l_11ACtxVerifyEvmParam.TX2, l_11ACtxVerifyEvmParam.TX3, l_11ACtxVerifyEvmParam.TX4, &antenaOrder);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] CheckAntennaOrderByStream() return error.\n");					
						throw logMessage;
					}

					rxRmsPowerDb[0][avgIteration-1] = rxRmsPowerDb[0][avgIteration-1] + l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
				}

				// Frequency Error
				freqErrorHz[avgIteration-1] = ::LP_GetScalarMeasurement("freqErr", 0); 

				// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
				ampErrDb[0][avgIteration-1] = ::LP_GetScalarMeasurement("ampErrDb", 0); 

				// IQ Match Phase Error.
				phaseErr[0][avgIteration-1] = ::LP_GetScalarMeasurement("phaseErr", 0);

				// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
				phaseNoiseRms[avgIteration-1] = ::LP_GetScalarMeasurement("rmsPhaseNoise",0);

				// Datarate 						
				if (wifiMode==WIFI_11B)
				{
					l_11ACtxVerifyEvmReturn.DATA_RATE = ::LP_GetScalarMeasurement("bitRateInMHz", 0);
				}
				else
				{                        
					l_11ACtxVerifyEvmReturn.DATA_RATE = ::LP_GetScalarMeasurement("dataRate", 0); 
				}

			}
			else /*if ( (wifiMode==WIFI_11N_GF_HT20)||(wifiMode==WIFI_11N_GF_HT40) ||
				(wifiMode==WIFI_11N_MF_HT20)||(wifiMode==WIFI_11N_MF_HT40) )       */               
			{
				// Number of spatial streams
				l_11ACtxVerifyEvmReturn.SPATIAL_STREAM = (int) ::LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);

				if (g_Tester_Type==IQ_View)
				{
					for(int i=0;i<l_11ACtxVerifyEvmReturn.SPATIAL_STREAM;i++)
					{
                        // Get EVM test result
                        evmAvgAll[i][avgIteration-1] = ::LP_GetScalarMeasurement("evmAvgAll", 0); // only can get EVM_ALL, means index always = 0
						if ( -99.00 >= evmAvgAll[i][avgIteration-1] )
						{
							analysisOK = false;
							evmAvgAll[i][avgIteration-1] = NA_NUMBER;
							WiFiSaveSigFile(sTestDetail);
							err = -1;
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(evmAvgAll) return error.\n");
							throw logMessage;
						}
                        // Get Power test result
                        rxRmsPowerDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("rxRmsPowerDb", i);
						if ( -99.00 >= rxRmsPowerDb[i][avgIteration-1] )
						{
							analysisOK = false;
							rxRmsPowerDb[i][avgIteration-1] = NA_NUMBER;
							WiFiSaveSigFile(sTestDetail);
							err = -1;
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(rxRmsPowerDb) return error.\n");
							throw logMessage;
						}
						else
						{
                            //The current code will work correctly with 4x4 MIMO.  
  							int antenaOrder = 0;
							err = CheckAntennaOrderByStream(i+1, l_11ACtxVerifyEvmParam.TX1, l_11ACtxVerifyEvmParam.TX2, l_11ACtxVerifyEvmParam.TX3, l_11ACtxVerifyEvmParam.TX4, &antenaOrder);
							if ( ERR_OK!=err )
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] CheckAntennaOrderByStream() return error.\n");					
								throw logMessage;
							}

							rxRmsPowerDb[i][avgIteration-1] = rxRmsPowerDb[i][avgIteration-1] + l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
						}  

						// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
						ampErrDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_amplDb", i); // Index always = 0
						phaseErr[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_phaseDeg",i);
                    }
				}
				else	// g_Tester_Type == IQnxn
				{
					for(int i=0;i<l_11ACtxVerifyEvmReturn.SPATIAL_STREAM;i++)
					{
						// EVM 
						evmAvgAll[i][avgIteration-1] = ::LP_GetScalarMeasurement("evmAvgAll",i); 
						if ( -99.00 >= evmAvgAll[i][avgIteration-1] )
						{
							analysisOK = false;
							evmAvgAll[i][avgIteration-1] = NA_NUMBER;
							WiFiSaveSigFile(sTestDetail);
							err = -1;
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(evmAvgAll) return error.\n");
							throw logMessage;
						}

						// Power
						rxRmsPowerDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("rxRmsPowerDb", i*(l_11ACtxVerifyEvmReturn.SPATIAL_STREAM+1));
						if ( -99.00 >= rxRmsPowerDb[i][avgIteration-1] )
						{
							analysisOK = false;
							rxRmsPowerDb[i][avgIteration-1] = NA_NUMBER;
							WiFiSaveSigFile(sTestDetail);
							err = -1;
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement(rxRmsPowerDb) return error.\n");
							throw logMessage;
						}
						else
						{
                            //The current code will work correctly with 4x4 MIMO.  
  							int antenaOrder = 0;
							err = CheckAntennaOrderByStream(i+1, l_11ACtxVerifyEvmParam.TX1, l_11ACtxVerifyEvmParam.TX2, l_11ACtxVerifyEvmParam.TX3, l_11ACtxVerifyEvmParam.TX4, &antenaOrder);
							if ( ERR_OK!=err )
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] CheckAntennaOrderByStream() return error.\n");					
								throw logMessage;
							}

							rxRmsPowerDb[i][avgIteration-1] = rxRmsPowerDb[i][avgIteration-1] + l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[antenaOrder-1];
						}

						// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
						ampErrDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_amplDb", i); // Index always = 0
						phaseErr[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_phaseDeg",i);
					}
				}
				// Frequency Error
				freqErrorHz[avgIteration-1] = ::LP_GetScalarMeasurement("freqErrorHz",0);

				// Datarate
				l_11ACtxVerifyEvmReturn.DATA_RATE = ::LP_GetScalarMeasurement("rateInfo_dataRateMbps", 0); 

				// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
				phaseNoiseRms[avgIteration-1] = ::LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll", 0);

				// Symbol clock error
				symbolClockErr[avgIteration-1] = ::LP_GetScalarMeasurement("symClockErrorPpm", 0);
			}                
	//		else
	//		{
	//			/*--------------------------------------------*/
	//			/* Retrieve analysis Results for 11ac          */
	//		    /*--------------------------------------------*/
	//		
	//		
	//			
	//			// Number of spatial streams
	//			l_11ACtxVerifyEvmReturn.SPATIAL_STREAM = (int) ::LP_GetScalarMeasurement("rateInfo_spatialStreams", 0);
	//			if (l_11ACtxVerifyEvmReturn.SPATIAL_STREAM == (int)NA_NUMBER)
	//			{
	//				err = -1;
	//				// Fail Analysis, thus save capture (Signal File) for debug
	//				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_11AC_TX_Evm_Invalid_Analysis_Results", l_11ACtxVerifyEvmParam.CH_FREQ_MHZ, l_11ACtxVerifyEvmParam.DATA_RATE, l_11ACtxVerifyEvmParam.CH_BANDWIDTH);
	//				WiFiSaveSigFile(sigFileNameBuffer);
	//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_GetScalarMeasurement() return error.\n");
	//				throw logMessage;
	//			}

	//			

	// //           no need to run IQ_view for MIMO
	//			if (g_Tester_Type==IQ_View)
	//			{
	//				for(int i=0;i<l_11ACtxVerifyEvmReturn.SPATIAL_STREAM;i++)
	//				{
	//					// Get EVM test result
	//					evmAvgAll[i][avgIteration-1] = ::LP_GetScalarMeasurement("evmAvgAll", 0); // only can get EVM_ALL, means index always = 0

	//					if ( -99.00 >= evmAvgAll[i][avgIteration-1] )
	//					{
	//						analysisOK = false;
	//						evmAvgAll[i][avgIteration-1] = NA_NUMBER;
	//						WiFiSaveSigFile(sTestDetail);
	//						err = -1;
	//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(evmAvgAll) chain %i return error.\n", i);
	//						throw logMessage;
	//					}

	//					// Get Power test result
	//					double measuredPower = ::LP_GetScalarMeasurement("rxRmsPowerDb", i);
	//					if (measuredPower!=NA_NUMBER)
	//						measuredPower += l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[i]

	//					rxRmsPowerDb[i][avgIteration-1] = measuredPower;

	//					printf("Power %6.2f stream %u iteration %u\n", measuredPower, i, avgIteration);

	//					if ( -99.00 >= rxRmsPowerDb[i][avgIteration-1] )
	//					{
	//						analysisOK = false;
	//						rxRmsPowerDb[i][avgIteration-1] = NA_NUMBER;
	//						WiFiSaveSigFile(sTestDetail);
	//						err = -1;
	//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_GetScalarMeasurement(rxRmsPowerDb) chain %i return error.\n", i);
	//						throw logMessage;
	//					}
	//					else
	//					{
	//						//The current code will work correctly with 2x2 MIMO.  
	//						//For more than 2x2 MIMO, need to implement more complicated algorithm

	//						l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[i]

	//						if (l_11ACtxVerifyEvmReturn.SPATIAL_STREAM == 1)
	//						{
	//							// One stream data rate, should be only one antenna is ON	
	//							if      (l_11ACtxVerifyEvmParam.TX1==1)	antSelection = 0;
	//							else if (l_11ACtxVerifyEvmParam.TX2==1)	antSelection = 1;
	//							else if (l_11ACtxVerifyEvmParam.TX3==1)	antSelection = 2;
	//							else if (l_11ACtxVerifyEvmParam.TX4==1)	antSelection = 3;
	//							else    							antSelection = 0;

	//							rxRmsPowerDb[i][avgIteration-1] = rxRmsPowerDb[i][avgIteration-1] + l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[antSelection];
	//						}
	//						else if (1 == l_11ACtxVerifyEvmParam.TX1 && 1 == l_11ACtxVerifyEvmParam.TX2)
	//						{
	//							rxRmsPowerDb[i][avgIteration-1] = rxRmsPowerDb[i][avgIteration-1] + l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[i];
	//						}
	//						else
	//						{
	//							rxRmsPowerDb[i][avgIteration-1] = rxRmsPowerDb[i][avgIteration-1] + cableLossDb;
	//						}
	//					}  

	//					// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
	//					ampErrDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_amplDb", i); // Index always = 0
	//					phaseErr[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_phaseDeg", i);

	//				}
	//			}
	//			else	// g_Tester_Type == IQnxn
	//			{
	////					for(int i=0;i<l_11ACtxVerifyEvmReturn.SPATIAL_STREAM;i++)  
	//				for(int i=0;i<g_Tester_Number;i++)  //not a good way to implement, what if the return value is changed, we should use spatial_stream. 
	//				{
	//					double evm;

	//					evm = ::LP_GetScalarMeasurement("evmAvgAll",i); 
	//					if (evm!=INVALID_EVM) 
	//					{
	//						// EVM 
	//						evmAvgAll[i][avgIteration-1] = evm;

	//						
	//						


	//						// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
	//						ampErrDb[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_amplDb", i);
	//						phaseErr[i][avgIteration-1] = ::LP_GetScalarMeasurement("IQImbal_phaseDeg", i);

	//						// Frequency Error
	//						freqErrorHz[avgIteration-1] = ::LP_GetScalarMeasurement("freqErrorHz",0);

	//						// Datarate
	//						l_11ACtxVerifyEvmReturn.DATA_RATE = ::LP_GetScalarMeasurement("rateInfo_dataRateMbps", 0); 

	//						// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
	//						phaseNoiseRms[avgIteration-1] = ::LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll", 0);

	//						symbolClockErr[avgIteration-1] = ::LP_GetScalarMeasurement("symClockErrorPpm", 0);
	//					}
	//				}
	//			}
	//		}
#pragma endregion
		}   // End - avgIteration

	 //  /*-----------*
		//*  Tx Stop  *
		//*-----------*/
		//err = ::vDUT_Run(g_WiFi_11ac_Dut, "TX_STOP");		
		//if ( ERR_OK!=err )
		//{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
		//	err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
		//	if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
		//	{
		//		err = -1;	// set err to -1, means "Error".
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
		//		throw logMessage;
		//	}
		//	else	// Just return normal error message in this case
		//	{
		//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_STOP) return error.\n");
		//		throw logMessage;
		//	}
		//}
		//else
		//{
		//	g_vDutTxActived = false;
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_STOP) return OK.\n");
		//}
		/*if(g_Tester_DualHead)
		{
			err = LP_DualHead_ReleaseControl();
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_DualHead_ReleaseControl() failed.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_DualHead_ReleaseControl() successful.\n");
			}
		}*/
#pragma region Averaging and Saving Test Result
	   /*----------------------------------*
		* Averaging and Saving Test Result *
		*----------------------------------*/
		if ( (ERR_OK==err) && captureOK && analysisOK )
		{
			double dummyMax, dummyMin;
			//double dummyAvg[MAX_DATA_STREAM];

			for(int i=0;i<l_11ACtxVerifyEvmReturn.SPATIAL_STREAM;i++)
			{
				// Average EVM test result
				::AverageTestResult(&evmAvgAll[i][0], avgIteration, LOG_20, l_11ACtxVerifyEvmReturn.EVM_AVG[i], l_11ACtxVerifyEvmReturn.EVM_MAX[i], l_11ACtxVerifyEvmReturn.EVM_MIN[i]);

				// Average Power test result
				::AverageTestResult(&rxRmsPowerDb[i][0], avgIteration, LOG_10, l_11ACtxVerifyEvmReturn.POWER_AVG[i], l_11ACtxVerifyEvmReturn.POWER_MAX[i], l_11ACtxVerifyEvmReturn.POWER_MIN[i]);

				// Average Amp Error test result
				//::AverageTestResult(&ampErrDb[i][0], avgIteration, LOG_10, dummyAvg[i], dummyMax, dummyMin);
				::AverageTestResult(&ampErrDb[i][0], avgIteration, LOG_10, l_11ACtxVerifyEvmReturn.AMP_ERR_DB_STREAM[i], dummyMax, dummyMin);

				// Average Phase Error
				::AverageTestResult(&phaseErr[i][0], avgIteration, Linear, l_11ACtxVerifyEvmReturn.PHASE_ERR_STREAM[i], dummyMax, dummyMin);

			}
			::AverageTestResult(l_11ACtxVerifyEvmReturn.EVM_AVG, l_11ACtxVerifyEvmReturn.SPATIAL_STREAM, LOG_20, l_11ACtxVerifyEvmReturn.EVM_AVG_DB, dummyMax, dummyMin);
			::AverageTestResult(l_11ACtxVerifyEvmReturn.POWER_AVG, l_11ACtxVerifyEvmReturn.SPATIAL_STREAM, LOG_10, l_11ACtxVerifyEvmReturn.POWER_AVG_DBM, dummyMax, dummyMin);
			::AverageTestResult(l_11ACtxVerifyEvmReturn.AMP_ERR_DB_STREAM, l_11ACtxVerifyEvmReturn.SPATIAL_STREAM, LOG_10, l_11ACtxVerifyEvmReturn.AMP_ERR_DB, dummyMax, dummyMin);
			::AverageTestResult(l_11ACtxVerifyEvmReturn.PHASE_ERR_STREAM,  l_11ACtxVerifyEvmReturn.SPATIAL_STREAM, Linear, l_11ACtxVerifyEvmReturn.PHASE_ERR, dummyMax, dummyMin);
	
			if (wifiMode==WIFI_11B)
			{
				// (11b only) Average EVM_PK test result
				::AverageTestResult(&evmPk[0], avgIteration, LOG_20, dummyMax, l_11ACtxVerifyEvmReturn.EVM_PK_DB, dummyMin);
			}

			// Average RMS Phase Noise test result
			::AverageTestResult(&phaseNoiseRms[0], avgIteration, Linear, l_11ACtxVerifyEvmReturn.PHASE_NOISE_RMS_ALL, dummyMax, dummyMin);
			
			// Average Freq Error test result
			::AverageTestResult(&freqErrorHz[0], avgIteration, Linear, l_11ACtxVerifyEvmReturn.FREQ_ERROR_AVG, l_11ACtxVerifyEvmReturn.FREQ_ERROR_MAX, l_11ACtxVerifyEvmReturn.FREQ_ERROR_MIN);

			// Average Symbol clock Error test result
			::AverageTestResult(&symbolClockErr[0], avgIteration, Linear, l_11ACtxVerifyEvmReturn.SYMBOL_CLK_ERR, dummyMax, dummyMin);

			// Transfer the Freq Error result unit to ppm
			l_11ACtxVerifyEvmReturn.FREQ_ERROR_AVG = l_11ACtxVerifyEvmReturn.FREQ_ERROR_AVG/l_11ACtxVerifyEvmParam.CH_FREQ_MHZ;
			l_11ACtxVerifyEvmReturn.FREQ_ERROR_MAX = l_11ACtxVerifyEvmReturn.FREQ_ERROR_MAX/l_11ACtxVerifyEvmParam.CH_FREQ_MHZ;
			l_11ACtxVerifyEvmReturn.FREQ_ERROR_MIN = l_11ACtxVerifyEvmReturn.FREQ_ERROR_MIN/l_11ACtxVerifyEvmParam.CH_FREQ_MHZ;
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
			sprintf_s(l_11ACtxVerifyEvmReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			l_11ACtxVerifyEvmReturn.TX_POWER_DBM = l_11ACtxVerifyEvmParam.TX_POWER_DBM;
			ReturnTestResults(l_11ACtxVerifyEvmReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_11ACtxVerifyEvmReturn.ERROR_MESSAGE, msg);

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
		ReturnErrorMessage(l_11ACtxVerifyEvmReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
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

int Initialize11ACTXVerifyEvmContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_11ACtxVerifyEvmParamMap.clear();

    WIFI_SETTING_STRUCT setting;

	strcpy_s(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH, MAX_BUFFER_SIZE, "BW-80");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_11ACtxVerifyEvmParam.BSS_BANDWIDTH;
		setting.unit        = "MHz";
		setting.helpText    = "BSS bandwidth\r\nValid options: BW-20, BW-40, BW-80, BW-80_80 or BW-160";
		l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_BANDWIDTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	strcpy_s(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, "0");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxVerifyEvmParam.CH_BANDWIDTH))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxVerifyEvmParam.CH_BANDWIDTH;
        setting.unit        = "MHz";
        setting.helpText    = "Channel bandwidth\r\nValid options:0, CBW-20, CBW-40, CBW-80, CBW-80_80 or CBW-160.\r\nFor 802.11ac, if it is zero,CH_BANDWIDTH equals as BSS_BANDWIDTH. For 802.11/a/b/g/n, it must always have value.";
        l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_BANDWIDTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_11ACtxVerifyEvmParam.DATA_RATE, MAX_BUFFER_SIZE, "MCS0");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxVerifyEvmParam.DATA_RATE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxVerifyEvmParam.DATA_RATE;
        setting.unit        = "";
        setting.helpText    = "Data rate names:\r\nDSSS-1,DSSS-2,CCK-5_5,CCK-11\r\nOFDM-6,OFDM-9,OFDM-12,OFDM-18,OFDM-24,OFDM-36,OFDM-48,OFDM-54\r\nMCS0, MCS15, etc.";
        l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_11ACtxVerifyEvmParam.PREAMBLE, MAX_BUFFER_SIZE, "SHORT");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxVerifyEvmParam.PREAMBLE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxVerifyEvmParam.PREAMBLE;
        setting.unit        = "";
        setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=SHORT.";
        l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	// "PACKET_FORMAT"
    strcpy_s(l_11ACtxVerifyEvmParam.PACKET_FORMAT, MAX_BUFFER_SIZE, PACKET_FORMAT_VHT);
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxVerifyEvmParam.PACKET_FORMAT))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxVerifyEvmParam.PACKET_FORMAT;
        setting.unit        = "";
        setting.helpText    = "The packet format, VHT, HT_MF,HT_GF and NON_HT as defined in standard. Default=VHT.";
        l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
	
	// "STANDARD"
	strcpy_s(l_11ACtxVerifyEvmParam.STANDARD, MAX_BUFFER_SIZE, STANDARD_802_11_AC); 
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACtxVerifyEvmParam.STANDARD))    // Type_Checking
	{
		setting.value       = (void*)l_11ACtxVerifyEvmParam.STANDARD;
		setting.unit        = "";
		setting.helpText    = "Used for signal analysis option or to discriminating the same data rate or package format from different standards, taking value from 802.11ac, 802.11n, 802.11ag, 802.11b. Default = 802.11ac ";
		l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("STANDARD", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	strcpy_s(l_11ACtxVerifyEvmParam.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACtxVerifyEvmParam.GUARD_INTERVAL))    // Type_Checking
	{
		setting.value       = (void*)l_11ACtxVerifyEvmParam.GUARD_INTERVAL;
		setting.unit        = "";
		setting.helpText    = "Packet Guard Interval, Long or Short, default is Long";
		l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY = 5520;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment";
		l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_PRIMARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 80+80MHz bandwidth, it provides the center frequency of the secondary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined";
		l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_SECONDARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz;
		setting.unit        = "MHz";
		setting.helpText    = "The center frequency (MHz) for primary 20 MHZ channel, priority is lower than \"CH_FREQ_MHZ\".";
		l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_20MHz", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACtxVerifyEvmParam.CH_FREQ_MHZ = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxVerifyEvmParam.CH_FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxVerifyEvmParam.CH_FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "It is the center frequency (MHz) for channel. If it is zero,\"CH_FREQ_MHZ_PRIMARY_20MHz\" will be used for 802.11ac. \r\nFor 802.11/a/b/g/n, it must alway have value.";
		l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	l_11ACtxVerifyEvmParam.NUM_STREAM_11AC = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACtxVerifyEvmParam.NUM_STREAM_11AC))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxVerifyEvmParam.NUM_STREAM_11AC;
		setting.unit        = "";
		setting.helpText    = "Number of spatial streams based on 11AC spec";
		l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("NUM_STREAM_11AC", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_11ACtxVerifyEvmParam.SAMPLING_TIME_US = 0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmParam.SAMPLING_TIME_US))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmParam.SAMPLING_TIME_US;
        setting.unit        = "us";
        setting.helpText    = "Capture time in micro-seconds";
        l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("SAMPLING_TIME_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_11ACtxVerifyEvmParam.TX_POWER_DBM = 15.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmParam.TX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmParam.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port";
        l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxVerifyEvmParam.TX1 = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxVerifyEvmParam.TX1))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmParam.TX1;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxVerifyEvmParam.TX2 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxVerifyEvmParam.TX2))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmParam.TX2;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxVerifyEvmParam.TX3 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxVerifyEvmParam.TX3))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmParam.TX3;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxVerifyEvmParam.TX4 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxVerifyEvmParam.TX4))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmParam.TX4;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_11ACtxVerifyEvmParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
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
    l_11ACtxVerifyEvmReturnMap.clear();

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = 0;
    if (sizeof(int)==sizeof(l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz))    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 40 MHZ channel";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_40MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	setting.type = WIFI_SETTING_TYPE_INTEGER;
	l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = 0;
    if (sizeof(int)==sizeof(l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz))    // Type_Checking
    {
        setting.value = (void*)&l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
        setting.unit  = "MHz";
        setting.helpText = "The center frequency (MHz) for PRIMARY 80 MHZ channel";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string, WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_80MHz", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	


    l_11ACtxVerifyEvmReturn.TX_POWER_DBM = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.TX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxVerifyEvmReturn.EVM_AVG_DB = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.EVM_AVG_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.EVM_AVG_DB;
        setting.unit        = "dB";
        setting.helpText    = "EVM average over captured packets and all data streams(MIMO)";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_AVG_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_11ACtxVerifyEvmReturn.EVM_AVG[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.EVM_AVG[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxVerifyEvmReturn.EVM_AVG[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "EVM_AVG_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Average EVM over captured packets on individual streams(MIMO)";
            l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_11ACtxVerifyEvmReturn.EVM_MAX[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.EVM_MAX[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxVerifyEvmReturn.EVM_MAX[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "EVM_MAX_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Maximum EVM over captured packets on individual streams(MIMO)";
            l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_11ACtxVerifyEvmReturn.EVM_MIN[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.EVM_MIN[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxVerifyEvmReturn.EVM_MIN[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "EVM_MIN_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Minimum EVM over captured packets on individual streams(MIMO)";
            l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    l_11ACtxVerifyEvmReturn.EVM_PK_DB = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.EVM_PK_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.EVM_PK_DB;
        setting.unit        = "dB";
        setting.helpText    = "(11b only)Peak EVM over captured packets.";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_PK_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxVerifyEvmReturn.AMP_ERR_DB = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.AMP_ERR_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.AMP_ERR_DB;
        setting.unit        = "dB";
        setting.helpText    = "IQ Match Amplitude Error in dB.";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("AMP_ERR_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for(int i=0;i<MAX_DATA_STREAM;i++)
	{
	    l_11ACtxVerifyEvmReturn.AMP_ERR_DB_STREAM[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.AMP_ERR_DB_STREAM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxVerifyEvmReturn.AMP_ERR_DB_STREAM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "AMP_ERR_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "IQ Match Amplitude Error in dB.(per stream)";
            l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
	}

	l_11ACtxVerifyEvmReturn.PHASE_ERR = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.PHASE_ERR))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.PHASE_ERR;
        setting.unit        = "Degree";
        setting.helpText    = "IQ Match Phase Error.";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PHASE_ERR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxVerifyEvmReturn.SYMBOL_CLK_ERR = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.SYMBOL_CLK_ERR))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACtxVerifyEvmReturn.SYMBOL_CLK_ERR;
		setting.unit        = "ppm";
		setting.helpText    = "Symbol Clock Error";
		l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SYMBOL_CLK_ERR", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for(int i=0;i<MAX_DATA_STREAM;i++)
	{
	    l_11ACtxVerifyEvmReturn.PHASE_ERR_STREAM[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.PHASE_ERR_STREAM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxVerifyEvmReturn.PHASE_ERR_STREAM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "PHASE_ERR_%d", i+1);
			setting.unit        = "Degree";
			setting.helpText    = "IQ Match Phase Error.(per stream)";
            l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
	}

	//PHASE_NOISE_RMS_ALL
	l_11ACtxVerifyEvmReturn.PHASE_NOISE_RMS_ALL = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.PHASE_NOISE_RMS_ALL))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.PHASE_NOISE_RMS_ALL;
        setting.unit        = "Degree";
        setting.helpText    = "Frequency RMS Phase Noise.";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PHASE_NOISE_RMS_ALL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxVerifyEvmReturn.POWER_AVG_DBM = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.POWER_AVG_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.POWER_AVG_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Average Power over captured packets";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVG_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_11ACtxVerifyEvmReturn.POWER_AVG[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.POWER_AVG[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxVerifyEvmReturn.POWER_AVG[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_AVG_%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "Average Power over captured packets on individual streams";
            l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_11ACtxVerifyEvmReturn.POWER_MAX[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.POWER_MAX[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxVerifyEvmReturn.POWER_MAX[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_MAX_%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "Maximum Power over captured packets on individual streams";
            l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_11ACtxVerifyEvmReturn.POWER_MIN[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.POWER_MIN[i]))    // Type_Checking
        {
            setting.value = (void*)&l_11ACtxVerifyEvmReturn.POWER_MIN[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_MIN_%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "Minimum Power over captured packets on individual streams";
            l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    l_11ACtxVerifyEvmReturn.FREQ_ERROR_AVG = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.FREQ_ERROR_AVG))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.FREQ_ERROR_AVG;
        setting.unit        = "ppm";
        setting.helpText    = "Average frequency error over captured packets";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_ERROR_AVG", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxVerifyEvmReturn.FREQ_ERROR_MAX = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.FREQ_ERROR_MAX))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.FREQ_ERROR_MAX;
        setting.unit        = "ppm";
        setting.helpText    = "Maximum frequency error over captured packets";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_ERROR_MAX", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_11ACtxVerifyEvmReturn.FREQ_ERROR_MIN = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.FREQ_ERROR_MIN))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.FREQ_ERROR_MIN;
        setting.unit        = "ppm";
        setting.helpText    = "Minimum frequency error over captured packets";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_ERROR_MIN", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxVerifyEvmReturn.DATA_RATE = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.DATA_RATE))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.DATA_RATE;
        setting.unit        = "Mbps";
        setting.helpText    = "Data rate in Mbps, reported by IQAPI.";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_11ACtxVerifyEvmReturn.SPATIAL_STREAM = (int)NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_11ACtxVerifyEvmReturn.SPATIAL_STREAM))    // Type_Checking
    {
        setting.value       = (void*)&l_11ACtxVerifyEvmReturn.SPATIAL_STREAM;
        setting.unit        = "";
        setting.helpText    = "Number of spatial stream, reported by IQAPI.";
        l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SPATIAL_STREAM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_11ACtxVerifyEvmReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_11ACtxVerifyEvmReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_11ACtxVerifyEvmReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_11ACtxVerifyEvmReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACtxVerifyEvmReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_11ACtxVerifyEvmReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_11ACtxVerifyEvmReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}


double CheckSamplingTime(int wifiMode, char *preamble11B, char *dataRate, char *packetFormat)
{
    double samplingTimeUs = 300;

	if ( wifiMode==WIFI_11B )
	{
		if (0==strcmp(preamble11B, "LONG"))
		{
			samplingTimeUs = g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11B_L_US;
		}
		else	// 802.11b preamble = SHORT
		{
			if (0==strcmp(dataRate, "DSSS-1"))	//  for 1 Mbps (Long Preamble only)
			{
				samplingTimeUs = g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11B_L_US;
			}
			else	// Using default sampling time (short preamble)
			{
				samplingTimeUs = g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11B_S_US;			
			}
		}
	}
	else if ( wifiMode==WIFI_11AG )
	{
		samplingTimeUs = g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AG_US;
	}
	else			// 802.11ac
	{
		if ( 0==strcmp( packetFormat, PACKET_FORMAT_VHT))
		{
			samplingTimeUs = g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11AC_VHT_US;  // 802.11ac
		}
		else if ( 0==strcmp( packetFormat, PACKET_FORMAT_HT_MF))
		{
			samplingTimeUs = g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11N_MIXED_US;  //802.11n MF
		}
		else if ( 0==strcmp( packetFormat, PACKET_FORMAT_HT_GF))
		{
			samplingTimeUs = g_WiFi11ACGlobalSettingParam.EVM_CAPTURE_TIME_11N_GREENFIELD_US;   //802.11n GF
		}

	}

	return samplingTimeUs;
}


//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
// 
//-------------------------------------------------------------------------------------
int Check11ACTxEvmParameters( int *bssBW, int * cbw,int *bssPchannel,int *bssSchannel,int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// check common input parameters
          err = CheckCommonParameters_WiFi_11ac(l_11ACtxVerifyEvmParamMap, 
			                                    l_11ACtxVerifyEvmReturnMap, 
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
		  
		  
		  
//		if ( 0!=strcmp(l_11ACtxVerifyEvmParam.PREAMBLE, "SHORT") && 0!=strcmp(l_11ACtxVerifyEvmParam.PREAMBLE, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_11ACtxVerifyEvmParam.PREAMBLE);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0!=strcmp(l_11ACtxVerifyEvmParam.GUARD_INTERVAL, "SHORT") && 0!=strcmp(l_11ACtxVerifyEvmParam.GUARD_INTERVAL, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown GUARD_INTERVAL, WiFi guard interval %s not supported.\n", l_11ACtxVerifyEvmParam.GUARD_INTERVAL);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if (( 0> l_11ACtxVerifyEvmParam.NUM_STREAM_11AC) || ( l_11ACtxVerifyEvmParam.NUM_STREAM_11AC >8))
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
//		if ( (0!=strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11AC_MF_HT")) && (0!=strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11AC_GF_HT")) &&
//			(0!=strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11N_MF_HT")) && (0!=strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11N_GF_HT")) &&
//			(0!=strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11AC_VHT")) &&( 0!=strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11AC_NON_HT"))
//			&&( 0!=strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "LEGACY_NON_HT")))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown PACKET_FORMAT, WiFi 11ac packet format %s not supported.\n", l_11ACtxVerifyEvmParam.PACKET_FORMAT);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0 == strcmp( l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11AC_NON_HT") &&
//			! strstr ( l_11ACtxVerifyEvmParam.DATA_RATE, "OFDM"))
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
//		err = TM_WiFiConvertDataRateNameToIndex(l_11ACtxVerifyEvmParam.DATA_RATE, &dummyInt);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_11ACtxVerifyEvmParam.DATA_RATE);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( 0 == strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11AC_VHT"))  // Data rate: MCS0 ~MCS9
//		{
//			if (( 14 <= dummyInt ) && (dummyInt <= 23))
//			{
//				// Data rate is right
//				//do nothing
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] PACKET_FORMAT and DATA_RATE don't match! The data rates of VHT_11AC must be MCS0 ~ MCS9, doesn't support %s!\n", &l_11ACtxVerifyEvmParam.DATA_RATE);
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
//		if ( !strstr (l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11AC"))    // Legacy signal, CH_BANDWIDTH and CH_FREQ_MHZ must have values
//		{
//			if (( l_11ACtxVerifyEvmParam.CH_BANDWIDTH <= 0) || ( l_11ACtxVerifyEvmParam.CH_FREQ_MHZ <= 0))
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
//		if ( 0!=strcmp(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH, "BW-20") && 0!=strcmp(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH, "BW-40") && 
//			 0!=strcmp(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH, "BW-80") && 0!=strcmp(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH, "BW-160") &&
//			 0!=strcmp(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH, "BW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACtxVerifyEvmParam.BSS_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH, "BW-20"))
//			{
//				*bssBW = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH, "BW-40"))
//			{
//				*bssBW = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH, "BW-80"))
//			{
//				*bssBW = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH, "BW-160"))
//			{
//				*bssBW = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxVerifyEvmParam.BSS_BANDWIDTH, "BW-80_80"))
//			{
//				*bssBW = BW_80_80MHZ;
//
//				if ( l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
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
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACtxVerifyEvmParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//
//		}
//
//		if ( 0 != strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "0") && 
//			0!=strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "CBW-20") && 0!=strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "CBW-40") && 
//			 0!=strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "CBW-80") && 0!=strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "CBW-160") &&
//			 0!=strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "CBW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown CBW BANDWIDTH, WiFi 11ac CBW bandwidth %s not supported.\n", l_11ACtxVerifyEvmParam.CH_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "0"))
//			{
//				sprintf_s(l_11ACtxVerifyEvmParam.CH_BANDWIDTH,MAX_BUFFER_SIZE,"C%s",l_11ACtxVerifyEvmParam.BSS_BANDWIDTH);
//				*cbw = *bssBW;
//			}
//			else if ( 0 == strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "CBW-20"))
//			{
//				*cbw = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "CBW-40"))
//			{
//				*cbw = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "CBW-80"))
//			{
//				*cbw = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "CBW-160"))
//			{
//				*cbw = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, "CBW-80_80"))
//			{
//				*cbw = BW_80_80MHZ;
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACtxVerifyEvmParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//		}
//	
//		if ( strstr ( l_11ACtxVerifyEvmParam.PREAMBLE,"11AC"))  //802.11ac, cbw can't be larger than bssBW
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
//		if ( strstr( l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11N"))  //802.11n, BW= 20,40MHz
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
//		else if ( 0 == strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "LEGACY_NON_HT"))  // 11B and 11ag
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
//			if ( 0 == strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11AC_VHT"))
//			{
//				//do nothing
//			}
//			else if ( 0 == strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11AC_MF_HT") ||
//				0 == strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11AC_GF_HT"))
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
//		if (l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY <= 0)
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
//		err = TM_WiFiConvertFrequencyToChannel(l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY, bssPchannel);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( *bssBW == BW_80_80MHZ)  // Need BSS_FREQ_MHZ_SECONDARY
//		{
//			if ( l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
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
//			err = TM_WiFiConvertFrequencyToChannel(l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY, bssSchannel);      
//			if ( ERR_OK!=err )
//			{
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_SECONDARY);
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
//		if ( strstr (l_11ACtxVerifyEvmParam.PACKET_FORMAT, "11AC"))
//		{
//
//			if ( 0 == l_11ACtxVerifyEvmParam.CH_FREQ_MHZ)  
//			{
//				//no CH_FREQ_MHZ, no CH_FREQ_MHZ_PRIMARY_20MHz
//				// Use default values
//				if ( 0 == l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz) 
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi_11AC] Don't have input parameters \"CH_FREQ_MHZ\" or \"CH_FREQ_MHZ_PRIMARY_20\", using BBS center freuqency as default!");
//					l_11ACtxVerifyEvmParam.CH_FREQ_MHZ = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//
//					// all  use lower frequency for channel list
//					switch (*bssBW)
//					{
//					case BW_20MHZ:
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_40MHZ:
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 10;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_80MHZ:
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 20;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//						break;
//					
//					case BW_160MHZ:			
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//				
//					case BW_80_80MHZ:				
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//
//					default:
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					}
//
//				}
//				else  //no CH_FREQ_MHZ, have CH_FREQ_MHZ_PRIMARY_20MHz. Use input CH_FREQ_MHZ_PRIMARY_20MHz to calculate
//				{
//					err = GetChannelList(*bssBW, l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY,l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//					if ( err != ERR_OK) // Wrong channel list
//					{
//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_11ACtxVerifyEvmParam.PACKET_FORMAT);
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
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//					else if (*cbw == BW_20MHZ)
//					{
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ = l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz;
//					}
//					else if (*cbw == BW_40MHZ)
//					{
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ = l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
//					}
//					else if (*cbw == BW_80MHZ)
//					{
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ = l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
//					}
//					else
//					{
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//				}
//				else // cbw = BW_80_80MHZ, use BSS primary and secondary
//				{
//					l_11ACtxVerifyEvmParam.CH_FREQ_MHZ = NA_INTEGER;
//					// do nothing
//				}
//			}
//			else  // CH_FREQ_MHZ: non-zero 
//			{
//				//Check if input CH_FREQ_MHZ is correct
//				err = CheckChannelFreq(*bssBW, *cbw,
//					l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY,l_11ACtxVerifyEvmParam.CH_FREQ_MHZ);
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
//				if ( 0 != l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz)
//				{
//					err = CheckChPrimary20(*bssBW, *cbw,
//							l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY,l_11ACtxVerifyEvmParam.CH_FREQ_MHZ,l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz);
//					if ( err == ERR_OK)  //input CH_FREQ_MHZ_PRIMARY_20MHz is correct
//					{
//						err = GetChannelList(*bssBW, l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY,l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//							&l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//						if ( err != ERR_OK) // Wrong channel list
//						{
//							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Can't get channel list. Please check input paramters.\n", l_11ACtxVerifyEvmParam.PACKET_FORMAT);
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
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					case BW_40MHZ:  //cbw = BW_20.40MHz
//						if (*cbw == BW_20MHZ)
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 10;
//
//						}
//						else  //wrong cbw
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80MHZ:  //cbw = BW_20,40,80MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_11ACtxVerifyEvmParam.CH_FREQ_MHZ == l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_11ACtxVerifyEvmParam.CH_FREQ_MHZ == l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  //wrong cbw
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz =  NA_INTEGER;
//
//						}
//						break;
//					case BW_160MHZ:  //cbw = BW_20,40,80,160MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_11ACtxVerifyEvmParam.CH_FREQ_MHZ == l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 60)
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_11ACtxVerifyEvmParam.CH_FREQ_MHZ == l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_11ACtxVerifyEvmParam.CH_FREQ_MHZ == l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else if ( l_11ACtxVerifyEvmParam.CH_FREQ_MHZ == l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 60)
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 50;
//							}
//							else  //wrong CH_FREQ_MHZ
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							if (l_11ACtxVerifyEvmParam.CH_FREQ_MHZ == l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 40)
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_11ACtxVerifyEvmParam.CH_FREQ_MHZ == l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 40)
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_160MHZ)
//						{
//							if (l_11ACtxVerifyEvmParam.CH_FREQ_MHZ == l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY)
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else  //wring cbw
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80_80MHZ:
//						if ( *cbw == BW_20MHZ)
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if ( l_11ACtxVerifyEvmParam.CH_FREQ_MHZ == l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_11ACtxVerifyEvmParam.CH_FREQ_MHZ == l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else if ( *cbw == BW_80_80MHZ)
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  // wrong CH_FREQ_MHz
//						{
//							l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					default:
//						l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						break;
//					}
//				}
//
//				err = GetChannelList(*bssBW, l_11ACtxVerifyEvmParam.BSS_FREQ_MHZ_PRIMARY,l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//				if ( err != ERR_OK)  //Get channel list wrong
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_11ACtxVerifyEvmParam.PACKET_FORMAT);
//					throw logMessage;
//				}
//				else  //Get channel list successfully
//				{
//				}
//			}	
//
//			//Check channel list. If all are "NA_INTEGER", return error
//			if (( l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz == NA_INTEGER ) && 
//				(l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz == NA_INTEGER ) &&
//				(l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER))
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
//			l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//			l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//			l_11ACtxVerifyEvmReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//		}
//#pragma endregion
//		
//		//VHT, cbw = BW_20MHZ, the stream number of MCS9 only can 3,6
//		if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )
//		{
//			if ( (l_11ACtxVerifyEvmParam.NUM_STREAM_11AC != 3) &&
//				 (l_11ACtxVerifyEvmParam.NUM_STREAM_11AC != 6) )
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
//		err = WiFi_11ac_TestMode(l_11ACtxVerifyEvmParam.DATA_RATE, cbw, wifiMode, wifiStreamNum, l_11ACtxVerifyEvmParam.PACKET_FORMAT);
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
//										l_11ACtxVerifyEvmParam.CH_FREQ_MHZ,
//								  l_11ACtxVerifyEvmParam.TX1,
//								  l_11ACtxVerifyEvmParam.TX2,
//								  l_11ACtxVerifyEvmParam.TX3,
//								  l_11ACtxVerifyEvmParam.TX4,
//										l_11ACtxVerifyEvmParam.CABLE_LOSS_DB,
//										l_11ACtxVerifyEvmReturn.CABLE_LOSS_DB,
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
//		if (0==l_11ACtxVerifyEvmParam.SAMPLING_TIME_US)
//		{
//			*samplingTimeUs = CheckSamplingTime(*wifiMode, l_11ACtxVerifyEvmParam.PREAMBLE, l_11ACtxVerifyEvmParam.DATA_RATE, l_11ACtxVerifyEvmParam.PACKET_FORMAT);
//		}
//		else	// SAMPLING_TIME_US != 0
//		{
//			*samplingTimeUs = l_11ACtxVerifyEvmParam.SAMPLING_TIME_US;
//		}
		

		  //check unique input parameters used only for TX_EVM test ... 
		  /*-------------------------------------*
		  *  set PAPR values for vsa setting     *
		  *--------------------------------------*/
		  if ( *wifiMode==WIFI_11B )  
			  *peakToAvgRatio = g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_11M;    // CCK        
		  else                        
			  *peakToAvgRatio = g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_54M;   // OFDM 

		// Check Dut configuration changed or not
		if (  g_WiFi11ACGlobalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.
			  l_11ACtxVerifyEvmParam.CH_FREQ_MHZ != g_RecordedParam.CH_FREQ_MHZ ||
			  0!=strcmp(l_11ACtxVerifyEvmParam.CH_BANDWIDTH, g_RecordedParam.CH_BANDWIDTH) ||
			  0!=strcmp(l_11ACtxVerifyEvmParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
			  0!=strcmp(l_11ACtxVerifyEvmParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
			  0!=strcmp(l_11ACtxVerifyEvmParam.PACKET_FORMAT, g_RecordedParam.PACKET_FORMAT) ||
			  0!=strcmp(l_11ACtxVerifyEvmParam.GUARD_INTERVAL, g_RecordedParam.GUARD_INTERVAL_11N) ||
			  l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  l_11ACtxVerifyEvmParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
			  l_11ACtxVerifyEvmParam.TX1!=g_RecordedParam.ANT1 ||
			  l_11ACtxVerifyEvmParam.TX2!=g_RecordedParam.ANT2 ||
			  l_11ACtxVerifyEvmParam.TX3!=g_RecordedParam.ANT3 ||
			  l_11ACtxVerifyEvmParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;			
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup
		g_RecordedParam.ANT1					= l_11ACtxVerifyEvmParam.TX1;
		g_RecordedParam.ANT2					= l_11ACtxVerifyEvmParam.TX2;
		g_RecordedParam.ANT3					= l_11ACtxVerifyEvmParam.TX3;
		g_RecordedParam.ANT4					= l_11ACtxVerifyEvmParam.TX4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_11ACtxVerifyEvmParam.CABLE_LOSS_DB[3];	
		g_RecordedParam.CH_FREQ_MHZ				= l_11ACtxVerifyEvmParam.CH_FREQ_MHZ;
	//	g_RecordedParam.CH_FREQ_MHZ_SECONDARY_80	= l_11ACtxVerifyEvmParam.CH_FREQ_MHZ_SECONDARY_80;
		g_RecordedParam.POWER_DBM				= l_11ACtxVerifyEvmParam.TX_POWER_DBM;	

		sprintf_s(g_RecordedParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, l_11ACtxVerifyEvmParam.CH_BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_11ACtxVerifyEvmParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_11ACtxVerifyEvmParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT, MAX_BUFFER_SIZE, l_11ACtxVerifyEvmParam.PACKET_FORMAT);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_11ACtxVerifyEvmParam.GUARD_INTERVAL);


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

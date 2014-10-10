#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;


// These global variables are declared in WiFi_Test_Internal.cpp
extern TM_ID				g_WiFi_Test_ID;    
extern vDUT_ID				g_WiFi_Dut;
extern int					g_Tester_Type;
extern int					g_Tester_Number;
extern bool					g_vDutTxActived;
extern bool					g_dutConfigChanged;
extern double				g_last_TxPower_dBm_Record;
extern WIFI_RECORD_PARAM	g_RecordedParam;

// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_WiFiGlobalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_txMultiVerificationParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_txMultiVerificationReturnMap;

struct tagtxMultiVerificationParam /* #LPTW# cfy,-2010/05/16- */
{
    // Mandatory Parameters
	int    ENABLE_EVM;
	int    ENABLE_MASK;
	int    ENABLE_POWER;
	int    ENABLE_SPECTRUM;

    int    FREQ_MHZ;                                /*! The center frequency (MHz). */  
    char   BANDWIDTH[MAX_BUFFER_SIZE];              /*! The RF bandwidth to verify EVM. */
    char   DATA_RATE[MAX_BUFFER_SIZE];              /*! The data rate to verify EVM. */
	char   PREAMBLE[MAX_BUFFER_SIZE];               /*! The preamble type of 11B(only). */
	char   PACKET_FORMAT_11N[MAX_BUFFER_SIZE];      /*! The packet format of 11N(only). */
	char   GUARD_INTERVAL_11N[MAX_BUFFER_SIZE];     /*! The guard interval format of 11N(only). */

	char   TX_POWER_CLASS[MAX_BUFFER_SIZE];			/*! The power class of 11p(only).*/	
	char   MASK_TEMPLATE[MAX_BUFFER_SIZE];

    double TX_POWER_DBM;                            /*! The output power to verify EVM. */
    double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	double OBW_PERCENTAGE;							/*! The percentage of OBW, default = 99% */
	double CORRECTION_FACTOR_11B;                   /*! The 802.11b(only) LO leakage correction factor. */

    double CAPTURE_LENGTH_OF_EVM_US;                /*! The capture length (time in us) to verify EVM. */
	double CAPTURE_LENGTH_OF_MASK_US;               /*! The capture length (time in us) to verify MASK. */
	double CAPTURE_LENGTH_OF_POWER_US;              /*! The capture length (time in us) to verify POWER. */
	double CAPTURE_LENGTH_OF_SPECTRUM_US;           /*! The capture length (time in us) to verify SPECTRUM. */

    double SAMPLING_TIME_US;                        /*! The sampling time to verify Power. */ // TBD, not yet implemented

    // DUT Parameters
    int    TX1;                                     /*!< DUT TX1 on/off. Default=1(on) */
    int    TX2;                                     /*!< DUT TX2 on/off. Default=0(off) */
    int    TX3;                                     /*!< DUT TX3 on/off. Default=0(off) */
    int    TX4;                                     /*!< DUT TX4 on/off. Default=0(off) */

	int    ARRAY_HANDLING_METHOD;                   /*! The flag to handle array result. Default: 0; 0: Do nothing, 1: Display result on screen, 2: Display result and log to file */
} l_txMultiVerificationParam;

struct tagtxMultiVerificationReturn
{
	double TX_POWER_DBM;                            /*!< TX power dBm setting */
  
    // EVM Test Result 
    double EVM_AVG_DB;                  
    double EVM_AVG[MAX_DATA_STREAM];						/*!< (Average) EVM test result on specific data stream. Format: EVM_AVG[SpecificStream] */
  
    // EVM_PK Test Result 
    double EVM_PK_DB;							   			/*!< (11b only)EVM_PK over captured packets. */                  

    // POWER Test Result 
    double POWER_AVG_DBM;                  
    double POWER_AVG[MAX_DATA_STREAM];						/*!< (Average) POWER test result on specific data stream. Format: POWER_AVG[SpecificStream] */

    double FREQ_ERROR_AVG;									/*!< (Average) Frequency Error, unit is ppm */

	double AMP_ERR_DB;										/*!< IQ Match Amplitude Error in dB. */
	double PHASE_ERR;										/*!< IQ Match Phase Error. */
	double PHASE_NOISE_RMS_ALL;								/*!< Frequency RMS Phase Noise. */ 
	double SYMBOL_CLK_ERR;									/*!< Symbol clock error*/

	double AMP_ERR_DB_STREAM[MAX_DATA_STREAM];				/*!< IQ Match Amplitude Error in dB. per stream*/
	double PHASE_ERR_STREAM[MAX_DATA_STREAM];				/*!< IQ Match Phase Error. per stream*/

	int	   SPATIAL_STREAM;	
	double DATA_RATE;
    // Mask Test Result 
    double MARGIN_DB_POSITIVE[MAX_POSITIVE_SECTION];		/*!< Margin to the mask, normally should be a negative number, if pass. */
	double MARGIN_DB_NEGATIVE[MAX_NEGATIVE_SECTION];		/*!< Margin to the mask, normally should be a negative number, if pass */
	double FREQ_AT_MARGIN_POSITIVE[MAX_POSITIVE_SECTION];	/*!< Point out the frequency offset at margin to the mask */
	double FREQ_AT_MARGIN_NEGATIVE[MAX_NEGATIVE_SECTION];	/*!< Point out the frequency offset at margin to the mask */
	double VIOLATION_PERCENT;								/*!< Percentage which fail the mask */

	vector <double> *SPECTRUM_RAW_DATA_X_VSA[MAX_TESTER_NUM];	/*!< spectrum X raw data in each VSA.  */
	vector <double> *SPECTRUM_RAW_DATA_Y_VSA[MAX_TESTER_NUM];	/*!< spectrum Y raw data in each VSA.  */

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

	double CABLE_LOSS_DB[MAX_DATA_STREAM];					/*! The path loss of test system. */

    char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_txMultiVerificationReturn;
#pragma endregion


void ClearTxMultiVerificationReturn(void)
{
	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		if( NULL!=l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i] )
		{
			delete l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i];
			l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i] = NULL;
		}

		if( NULL!=l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i] )
		{
			delete l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i];
			l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i] = NULL;
		}
	}

	l_txMultiVerificationParamMap.clear();
	l_txMultiVerificationReturnMap.clear();
}

#ifndef WIN32
int initTXMultiVerificationContainers = InitializeTXMultiVerificationContainers(void); // Modified /* -cfy@sunnyvale, 2012/3/13- */
#endif

// These global variables/functions only for WiFi_TX_Verify_Evm.cpp
int CheckTxVerificationParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *cableLossDb, double *peakToAvgRatio, char* errorMsg );
int WiFi_TX_Multi_Verification_2010ExtTest(void);


//! WiFi TX Multi Verification
/*!
* Input Parameters
*
*  - Mandatory 
*      -# FREQ_MHZ (double): The center frequency (MHz)
*      -# DATA_RATE (string): The data rate to verify EVM
*      -# TX_POWER_DBM (double): The power (dBm) DUT is going to transmit at the antenna port
*
* Return Values
*      -# A string that contains all DUT info, such as FW version, driver version, chip revision, etc.
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/


WIFI_TEST_API int WiFi_TX_Multi_Verification(void)
{
    int    err = ERR_OK; 
    int    dummyValue = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	
    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_txMultiVerificationReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_txMultiVerificationParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_txMultiVerificationReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
	   /*-----------------------------------------------------------*
		* Both g_WiFi_Test_ID and g_WiFi_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		if( g_WiFi_Test_ID<0 || g_WiFi_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID or WiFi_Dut not valid. WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFi_Test_ID = %d and WiFi_Dut = %d.\n", g_WiFi_Test_ID, g_WiFi_Dut);
		}
		
		TM_ClearReturns(g_WiFi_Test_ID);

		/*----------------------*
		 * Get input parameters *
		 *----------------------*/
		err = GetInputParameters(l_txMultiVerificationParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get input parameters return OK.\n");
		}


		/*------------------------------------------------------*
		* check 2010Ext test, run 2010 Ext test and return,		*
		* needed to split WIFI_TX_Multi_Verifcation into two to	* 
		* avoid a stack check issue.							*
		* Eric 6/24/2012!!										*
		*------------------------------------------------------*/
		if ( g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE )
		{
			return WiFi_TX_Multi_Verification_2010ExtTest();
		}


		/*------------------------------------------------------*
		* Or else start Multi Verification Test					*
		*------------------------------------------------------*/
		int    avgIteration = 0;
		int    channel = 0, HT40ModeOn = 0, packetFormat = 0;
		int    wifiMode = 0, wifiStreamNum = 0;
		double samplingTimeUs = 0, peakToAvgRatio = 0, cableLossDb = 0;
		double chainGainDb = 0.0;
		char   vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
		char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};
		bool   analysisOK = false, captureOK = false, enableMultiCapture = false;   

        // Error return of this function is irrelevant
        CheckDutTransmitStatus();

#pragma region Prepare input parameters

		err = CheckTxVerificationParameters( &channel, &wifiMode, &wifiStreamNum, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxVerificationParameters() return OK.\n");
		}

		// Modified /* -cfy@sunnyvale, 2012/3/13- */
		// Considering stream number while calculating chain gain for composite measurement so as to set the proper Rx amplitude in tester
		int antennaNum = l_txMultiVerificationParam.TX1 + l_txMultiVerificationParam.TX2 + l_txMultiVerificationParam.TX3 + l_txMultiVerificationParam.TX4;
		if(antennaNum > 0)
		{
			chainGainDb = 10.0 * log10((double)antennaNum / wifiStreamNum);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Set Chain Gain of %.2f dB.\n", chainGainDb);
		}			
		else
		{
			chainGainDb = 0.0;
		}
		/* <><~~ */
		
        if (0==strcmp(l_txMultiVerificationParam.PACKET_FORMAT_11N, "MIXED"))
		{
            packetFormat = 1;
		}
        else if (0 == strcmp(l_txMultiVerificationParam.PACKET_FORMAT_11N, "GREENFIELD"))
		{
            packetFormat = 2; 
		}
        else
		{
            packetFormat = 0;
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
			}
		}
		/*--------------------*
		 * Setup IQTester VSA *
		 *--------------------*/
		err = ::LP_SetVsaAmplitudeTolerance(g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);		
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to set VSA amplitude tolerance in dB.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsaAmplitudeTolerance(%.3f) return OK.\n", g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		}

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

		if( wifiMode==WIFI_11N_HT40 )
		{
			HT40ModeOn = 1;   // 1: HT40 mode;
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txMultiVerificationParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txMultiVerificationParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txMultiVerificationParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode 
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txMultiVerificationParam.FREQ_MHZ);
		}		  

		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txMultiVerificationParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txMultiVerificationParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_txMultiVerificationParam.GUARD_INTERVAL_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txMultiVerificationParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txMultiVerificationParam.BANDWIDTH);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txMultiVerificationParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txMultiVerificationParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txMultiVerificationParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txMultiVerificationParam.TX4);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs); // -cfy@sunnyvale, 2012/3/13-
		// -cfy@sunnyvale, 2012/3/13-
		// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
		if ( (l_txMultiVerificationParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
		{ // Indicate that the Tx target power will be determined by channel and data rate, rather than the value input by users in IQwizard 
			if ( g_dutConfigChanged == true )
			{
				/*-----------*
				*  Tx Stop  *
				*-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
				}

				/*----------------------*
				*  Get Tx Target Power  *
				*-----------------------*/
				double tx_target_power = 0;
				err = ::vDUT_Run(g_WiFi_Dut, "GET_TX_TARGET_POWER");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				   {
					   err = -1;	// set err to -1, means "Error".
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					   throw logMessage;
				   }
				   else	// Just return normal error message in this case
				   {
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(GET_TX_TARGET_POWER) return error.\n");
					   throw logMessage;
				   }
				}
				else
				{  
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(GET_TX_TARGET_POWER) return OK.\n");
				}
				vDUT_GetDoubleParameter (g_WiFi_Dut, "TX_TARGET_POWER_DBM", &tx_target_power);
				l_txMultiVerificationParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txMultiVerificationParam.TX_POWER_DBM;
			}
			else
			{
				l_txMultiVerificationParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		/* <><~~ */

		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txMultiVerificationParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", 	l_txMultiVerificationParam.TX_POWER_DBM); // -cfy@sunnyvale, 2012/3/13-

		if ( (g_dutConfigChanged==true)||(g_vDutTxActived==false) )	
		{
			if ( g_vDutTxActived==true )
			{
			   /*-----------*
				*  Tx Stop  *
				*-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
				}
			}
			else
			{
				// continue Dut configuration				
			}

			err = vDUT_Run(g_WiFi_Dut, "RF_SET_FREQ");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RF_SET_FREQ) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RF_SET_FREQ) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_SET_BW");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_BW) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_BW) return OK.\n");
			}
			err = vDUT_Run(g_WiFi_Dut, "TX_SET_DATA_RATE");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_DATA_RATE) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_DATA_RATE) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_SET_ANTENNA");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_ANTENNA) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_ANTENNA) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_PRE_TX");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_PRE_TX) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_PRE_TX) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_START");
			if ( ERR_OK!=err )
			{	
			   g_vDutTxActived = false;
			   // Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_START) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{
			   g_vDutTxActived = true;
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_START) return OK.\n");
			}

			// Delay for DUT settle
			if (0!=g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
			{
				Sleep(g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
			}
			else
			{
				// do nothing
			}
		}
		else
		{
			// g_dutConfigChanged==false, do nothing
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Since g_dutConfigChanged = false, skip Dut control section.\n");
		}
#pragma endregion

	   
		analysisOK = false;
		captureOK  = false;
		
		// Insert Keyword into WIFI TX Container
	    ::TM_ClearParameters(g_WiFi_Test_ID);
	    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "FREQ_MHZ",				l_txMultiVerificationParam.FREQ_MHZ);
	    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX1",						l_txMultiVerificationParam.TX1);
	    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX2",						l_txMultiVerificationParam.TX2);
	    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX3",						l_txMultiVerificationParam.TX3);
	    ::TM_AddIntegerParameter(g_WiFi_Test_ID, "TX4",						l_txMultiVerificationParam.TX4);
		::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_1",			l_txMultiVerificationParam.CABLE_LOSS_DB[0]);
		::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_2",			l_txMultiVerificationParam.CABLE_LOSS_DB[1]);
		::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_3",			l_txMultiVerificationParam.CABLE_LOSS_DB[2]);
		::TM_AddDoubleParameter (g_WiFi_Test_ID, "CABLE_LOSS_DB_4",			l_txMultiVerificationParam.CABLE_LOSS_DB[3]);
	    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "SAMPLING_TIME_US",		l_txMultiVerificationParam.CAPTURE_LENGTH_OF_EVM_US);
	    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "TX_POWER_DBM",			l_txMultiVerificationParam.TX_POWER_DBM);
	    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "OBW_PERCENTAGE",			l_txMultiVerificationParam.OBW_PERCENTAGE);	
	    ::TM_AddDoubleParameter (g_WiFi_Test_ID, "CORRECTION_FACTOR_11B",	l_txMultiVerificationParam.CORRECTION_FACTOR_11B);
	    ::TM_AddStringParameter (g_WiFi_Test_ID, "BANDWIDTH",				l_txMultiVerificationParam.BANDWIDTH);
	    ::TM_AddStringParameter (g_WiFi_Test_ID, "DATA_RATE",				l_txMultiVerificationParam.DATA_RATE);
	    ::TM_AddStringParameter (g_WiFi_Test_ID, "PREAMBLE",				l_txMultiVerificationParam.PREAMBLE);	
	    ::TM_AddStringParameter (g_WiFi_Test_ID, "PACKET_FORMAT_11N",		l_txMultiVerificationParam.PACKET_FORMAT_11N);
		::TM_AddStringParameter (g_WiFi_Test_ID, "GUARD_INTERVAL_11N",		l_txMultiVerificationParam.GUARD_INTERVAL_11N);

		::TM_AddIntegerParameter(g_WiFi_Test_ID, "ARRAY_HANDLING_METHOD",	l_txMultiVerificationParam.ARRAY_HANDLING_METHOD);
		::TM_AddStringParameter (g_WiFi_Test_ID, "TX_POWER_CLASS",			l_txMultiVerificationParam.TX_POWER_CLASS);
	    ::TM_AddStringParameter (g_WiFi_Test_ID, "MASK_TEMPLATE",			l_txMultiVerificationParam.MASK_TEMPLATE);

		//-----------//
		//    EVM    //
		//-----------//
		if ( l_txMultiVerificationParam.ENABLE_EVM ) // 1:ENABLE
		{
	    	err = WiFi_TX_Verify_Evm();
		    if( ERR_OK!=err )
			{
				captureOK = analysisOK = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function WiFi_TX_Verify_Evm() Failed.\n");
				throw logMessage;
			}
			else
			{	
				captureOK = analysisOK = true;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function WiFi_TX_Verify_Evm() OK.\n");
			}		
						
			// Get EVM/EVM_PK Test Results
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "EVM_AVG_DB", 			&l_txMultiVerificationReturn.EVM_AVG_DB);
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "EVM_PK_DB", 			&l_txMultiVerificationReturn.EVM_PK_DB);
		    for (int i=0;i<MAX_DATA_STREAM;i++)
		    {			
	            char tempStr[MAX_BUFFER_SIZE];
	            sprintf_s(tempStr, "EVM_AVG_%d", i+1);			    			
				::TM_GetDoubleReturn(g_WiFi_Test_ID, tempStr, 			&l_txMultiVerificationReturn.EVM_AVG[i]);
			}
			
			// Get POWER Test Results
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "POWER_AVG_DBM", 		&l_txMultiVerificationReturn.POWER_AVG_DBM);
		    for (int i=0;i<MAX_DATA_STREAM;i++)
		    {			
	            char tempStr[MAX_BUFFER_SIZE];
	            sprintf_s(tempStr, "POWER_AVG_%d", i+1);			    			
				::TM_GetDoubleReturn(g_WiFi_Test_ID, tempStr, 			&l_txMultiVerificationReturn.POWER_AVG[i]);
			}

			::TM_GetDoubleReturn(g_WiFi_Test_ID, "FREQ_ERROR_AVG", 		&l_txMultiVerificationReturn.FREQ_ERROR_AVG);
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "AMP_ERR_DB", 			&l_txMultiVerificationReturn.AMP_ERR_DB);
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "PHASE_ERR", 			&l_txMultiVerificationReturn.PHASE_ERR);
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "PHASE_NOISE_RMS_ALL", &l_txMultiVerificationReturn.PHASE_NOISE_RMS_ALL);
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "SYMBOL_CLK_ERR",		&l_txMultiVerificationReturn.SYMBOL_CLK_ERR);
			::TM_GetIntegerReturn(g_WiFi_Test_ID, "SPATIAL_STREAM", 		&l_txMultiVerificationReturn.SPATIAL_STREAM);	
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "DATA_RATE", 			&l_txMultiVerificationReturn.DATA_RATE);
			
		    for (int i=0;i<MAX_DATA_STREAM;i++)
		    {			
	            char tempStr[MAX_BUFFER_SIZE];
	            sprintf_s(tempStr, "AMP_ERR_DB_%d", i+1);	    			
				::TM_GetDoubleReturn(g_WiFi_Test_ID, tempStr, 			&l_txMultiVerificationReturn.AMP_ERR_DB_STREAM[i]);
			}
		    for (int i=0;i<MAX_DATA_STREAM;i++)
		    {			
	            char tempStr[MAX_BUFFER_SIZE];
	            sprintf_s(tempStr, "PHASE_ERR_%d", i+1);			    			
				::TM_GetDoubleReturn(g_WiFi_Test_ID, tempStr, 			&l_txMultiVerificationReturn.PHASE_ERR_STREAM[i]);
			}
		}

		//------------//
		//    MASK    //
		//------------//
		if (l_txMultiVerificationParam.ENABLE_MASK) // 1:ENABLE
		{
			err = WiFi_TX_Verify_Mask();
	    	if( ERR_OK!=err )
			{
				captureOK = analysisOK = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function WiFi_TX_Verify_Mask() Failed.\n");
				throw logMessage;
			}
			else
			{	
				captureOK = analysisOK = true;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function WiFi_TX_Verify_Mask() OK.\n");
			}
			
			// Get Mask/OBW Test Results
			//::TM_GetDoubleReturn(g_WiFi_Test_ID, "POWER_AVERAGE_DBM", 	l_txMultiVerificationReturn.POWER_AVG_DBM); //POWER Test will overwrite it
			
		    for (int i=0;i<MAX_POSITIVE_SECTION;i++)
		    {
	            char tempStr[MAX_BUFFER_SIZE];
	            sprintf_s(tempStr, "MASK_MARGIN_DB_UPPER_%d", i+1);
				::TM_GetDoubleReturn(g_WiFi_Test_ID, tempStr, 	&l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[i]);
			}	
		    for (int i=0;i<MAX_NEGATIVE_SECTION;i++)
		    {
	            char tempStr[MAX_BUFFER_SIZE];
	            sprintf_s(tempStr, "MASK_MARGIN_DB_LOWER_%d", i+1);
				::TM_GetDoubleReturn(g_WiFi_Test_ID, tempStr, 	&l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[i]);
			}		
		    for (int i=0;i<MAX_POSITIVE_SECTION;i++)
		    {
	            char tempStr[MAX_BUFFER_SIZE];
	            sprintf_s(tempStr, "FREQ_AT_MARGIN_UPPER_%d", i+1);
				::TM_GetDoubleReturn(g_WiFi_Test_ID, tempStr, 	&l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[i]);
			}
		    for (int i=0;i<MAX_NEGATIVE_SECTION;i++)
		    {
	            char tempStr[MAX_BUFFER_SIZE];
	            sprintf_s(tempStr, "FREQ_AT_MARGIN_LOWER_%d", i+1);
				::TM_GetDoubleReturn(g_WiFi_Test_ID, tempStr, 	&l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[i]);
			}
				
			for(int i=0;i<MAX_TESTER_NUM;i++)
			{
				char tempStr[MAX_BUFFER_SIZE];
				sprintf_s(tempStr, MAX_BUFFER_SIZE, "RAW_DATA_FREQ_VSA%d", i+1);

				int arraySize = 0;
				double *arrayDouble = NULL;
				
				::TM_GetArrayDoubleReturnSize(g_WiFi_Test_ID, tempStr, &arraySize);

				arrayDouble = new double[arraySize];
				
				if (!::TM_GetArrayDoubleReturn(g_WiFi_Test_ID, tempStr, arrayDouble, arraySize)) // 0: No Error
				{
					l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i]->clear();
					
					for (int j = 0; j < arraySize ; j++)
					{
						l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i]->push_back(arrayDouble[j]);
					}
				}
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Successfully obtained SPECTRUM_RAW_DATA_X_VSA (Size of %d) from TM.\n", l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i]->size());
			}
			

			for(int i=0;i<MAX_TESTER_NUM;i++)
			{
				char tempStr[MAX_BUFFER_SIZE];
				int arraySize = 0;
				double *arrayDouble = NULL;
				
				sprintf_s(tempStr, MAX_BUFFER_SIZE, "RAW_DATA_POWER_VSA%d", i+1);

				::TM_GetArrayDoubleReturnSize(g_WiFi_Test_ID, tempStr, &arraySize);

				arrayDouble = new double[arraySize];
				
				if (!::TM_GetArrayDoubleReturn(g_WiFi_Test_ID, tempStr, arrayDouble, arraySize)) // 0: No Error
				{
					l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->clear();
					
					for (int j = 0; j < arraySize ; j++)
					{
						l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->push_back(arrayDouble[j]);
					}
				}
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Successfully obtained SPECTRUM_RAW_DATA_Y_VSA (Size of %d) from TM.\n", l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->size());
			}
			
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "VIOLATION_PERCENT", 	&l_txMultiVerificationReturn.VIOLATION_PERCENT);
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "OBW_MHZ", 			&l_txMultiVerificationReturn.OBW_MHZ);

		}
		
		//-------------//
		//    POWER    //
		//-------------//
		if ( l_txMultiVerificationParam.ENABLE_POWER||l_txMultiVerificationParam.ENABLE_MASK ) // 1:ENABLE
		{	    	
			err = WiFi_TX_Verify_Power();
	    	if( ERR_OK!=err )
			{
				captureOK = analysisOK = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function WiFi_TX_Verify_Power() Failed.\n");
				throw logMessage;
			}
			else
			{	
				captureOK = analysisOK = true;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function WiFi_TX_Verify_Power() OK.\n");
			}
			
			// Get POWER Test Results
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "POWER_AVERAGE_DBM", 	&l_txMultiVerificationReturn.POWER_AVG_DBM); // Overwrite EVM test result

		}

		//----------------//
		//    SPECTRUM    //
		//----------------//
		if ( l_txMultiVerificationParam.ENABLE_SPECTRUM==1 )
		{	    					
			err = WiFi_TX_Verify_Spectrum();
	    	if( ERR_OK!=err )
			{
				captureOK = analysisOK = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Function WiFi_TX_Verify_Spectrum() Failed.\n");
				throw logMessage;
			}
			else
			{	
				captureOK = analysisOK = true;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Function WiFi_TX_Verify_Spectrum() OK.\n");
			}

			// -cfy@sunnyvale, 2012/3/13-			
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "TX_POWER_DBM", 	&l_txMultiVerificationReturn.TX_POWER_DBM);

			::TM_GetIntegerReturn(g_WiFi_Test_ID, "LEAST_MARGIN_SUBCARRIER", 	&l_txMultiVerificationReturn.LEAST_MARGIN_SUBCARRIER);
			::TM_GetIntegerReturn(g_WiFi_Test_ID, "SUBCARRIER_LO_A", 	&l_txMultiVerificationReturn.SUBCARRIER_LO_A);
			::TM_GetIntegerReturn(g_WiFi_Test_ID, "SUBCARRIER_LO_B", 	&l_txMultiVerificationReturn.SUBCARRIER_LO_B);
			::TM_GetIntegerReturn(g_WiFi_Test_ID, "SUBCARRIER_UP_A", 	&l_txMultiVerificationReturn.SUBCARRIER_UP_A);
			::TM_GetIntegerReturn(g_WiFi_Test_ID, "SUBCARRIER_UP_B", 	&l_txMultiVerificationReturn.SUBCARRIER_UP_B);

			::TM_GetDoubleReturn(g_WiFi_Test_ID, "MARGIN_DB", 			&l_txMultiVerificationReturn.MARGIN_DB);
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "LO_LEAKAGE_DB", 		&l_txMultiVerificationReturn.LO_LEAKAGE_DB);
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "VALUE_DB_LO_A", 		&l_txMultiVerificationReturn.VALUE_DB_LO_A);
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "VALUE_DB_LO_B",		&l_txMultiVerificationReturn.VALUE_DB_LO_B);
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "VALUE_DB_UP_A", 		&l_txMultiVerificationReturn.VALUE_DB_UP_A);
			::TM_GetDoubleReturn(g_WiFi_Test_ID, "VALUE_DB_UP_B", 		&l_txMultiVerificationReturn.VALUE_DB_UP_B);

		/* <><~~ */
		}

		// This is a special case, only if "DUT_KEEP_TRANSMIT=0" then must do TX_STOP manually.
		if ( (g_WiFiGlobalSettingParam.DUT_KEEP_TRANSMIT==0)&&(g_vDutTxActived==true) )
		{
		   /*-----------*
			*  Tx Stop  *
			*-----------*/
			err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/		 
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			sprintf_s(l_txMultiVerificationReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txMultiVerificationReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_txMultiVerificationReturn.ERROR_MESSAGE, msg);

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
    }
    catch(...)
    {
		ReturnErrorMessage(l_txMultiVerificationReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
    }

	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i]->clear();
		l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->clear();
	}


    return err;
}


int WiFi_TX_Multi_Verification_2010ExtTest(void)
{

	int    err = ERR_OK; 
	int    dummyValue = 0;
	int    avgIteration = 0;
	int    channel = 0, HT40ModeOn = 0, packetFormat = 0;
	int    wifiMode = 0, wifiStreamNum = 0;
	int    numberOfResultSet = 0;
	int    dataSize = 0;
	double samplingTimeUs = 0, peakToAvgRatio = 0, cableLossDb = 0;
	double chainGainDb = 0.0;
	double dutTxPower = NA_NUMBER;
	double bufferRealX[MAX_BUFFER_SIZE];
	double bufferRealY[MAX_BUFFER_SIZE];
	double dummyAvg = 0, dummyMax = 0, dummyMin = 0;
	double bufferReal [MAX_BUFFER_SIZE] = {0};
	double bufferImage[MAX_BUFFER_SIZE] = {0};
	char   vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};
	bool   analysisOK = false, captureOK = false, enableMultiCapture = false;   

	try
	{
		// Error return of this function is irrelevant
		CheckDutTransmitStatus();

#pragma region Prepare input parameters

		err = CheckTxVerificationParameters( &channel, &wifiMode, &wifiStreamNum, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Prepare input parameters CheckTxVerificationParameters() return OK.\n");
		}

		// Modified /* -cfy@sunnyvale, 2012/3/13- */
		// Considering stream number while calculating chain gain for composite measurement so as to set the proper Rx amplitude in tester
		int antennaNum = l_txMultiVerificationParam.TX1 + l_txMultiVerificationParam.TX2 + l_txMultiVerificationParam.TX3 + l_txMultiVerificationParam.TX4;
		if(antennaNum > 0)
		{
			chainGainDb = 10.0 * log10((double)antennaNum / wifiStreamNum);
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Set Chain Gain of %.2f dB.\n", chainGainDb);
		}			
		else
		{
			chainGainDb = 0.0;
		}
		/* <><~~ */
		
		if (0==strcmp(l_txMultiVerificationParam.PACKET_FORMAT_11N, "MIXED"))
		{
			packetFormat = 1;
		}
		else if (0 == strcmp(l_txMultiVerificationParam.PACKET_FORMAT_11N, "GREENFIELD"))
		{
			packetFormat = 2; 
		}
		else
		{
			packetFormat = 0;
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
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
			}
		}
		/*--------------------*
		 * Setup IQTester VSA *
		 *--------------------*/
		err = ::LP_SetVsaAmplitudeTolerance(g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);		
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to set VSA amplitude tolerance in dB.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsaAmplitudeTolerance(%.3f) return OK.\n", g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB);
		}

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_Dut);

		if( wifiMode==WIFI_11N_HT40 )
		{
			HT40ModeOn = 1;   // 1: HT40 mode;
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",       l_txMultiVerificationParam.FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "PRIMARY_FREQ",   l_txMultiVerificationParam.FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_Dut, "SECONDARY_FREQ", l_txMultiVerificationParam.FREQ_MHZ+10);
		}
		else
		{
			HT40ModeOn = 0;   // 0: Normal 20MHz mode 
			vDUT_AddIntegerParameter(g_WiFi_Dut, "FREQ_MHZ",      l_txMultiVerificationParam.FREQ_MHZ);
		}		  

		vDUT_AddStringParameter (g_WiFi_Dut, "PREAMBLE",		  l_txMultiVerificationParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_Dut, "PACKET_FORMAT_11N", l_txMultiVerificationParam.PACKET_FORMAT_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "GUARD_INTERVAL_11N", l_txMultiVerificationParam.GUARD_INTERVAL_11N);
		vDUT_AddStringParameter (g_WiFi_Dut, "DATA_RATE",		  l_txMultiVerificationParam.DATA_RATE);
		vDUT_AddStringParameter (g_WiFi_Dut, "BANDWIDTH",		  l_txMultiVerificationParam.BANDWIDTH);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "CHANNEL_BW",		  HT40ModeOn);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX1",				  l_txMultiVerificationParam.TX1);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX2",				  l_txMultiVerificationParam.TX2);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX3",				  l_txMultiVerificationParam.TX3);
		vDUT_AddIntegerParameter(g_WiFi_Dut, "TX4",				  l_txMultiVerificationParam.TX4);
		vDUT_AddDoubleParameter (g_WiFi_Dut, "SAMPLING_TIME_US",  samplingTimeUs); // -cfy@sunnyvale, 2012/3/13-
		// -cfy@sunnyvale, 2012/3/13-
		// Added GET_TX_TARGET_POWER feature for AR6004. When TX_POWER_DBM is set to -99, default target power will be applied.
		if ( (l_txMultiVerificationParam.TX_POWER_DBM == TX_TARGET_POWER_FLAG) )
		{ // Indicate that the Tx target power will be determined by channel and data rate, rather than the value input by users in IQwizard 
			if ( g_dutConfigChanged == true )
			{
				/*-----------*
				*  Tx Stop  *
				*-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
				}

				/*----------------------*
				*  Get Tx Target Power  *
				*-----------------------*/
				double tx_target_power = 0;
				err = ::vDUT_Run(g_WiFi_Dut, "GET_TX_TARGET_POWER");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				   {
					   err = -1;	// set err to -1, means "Error".
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					   throw logMessage;
				   }
				   else	// Just return normal error message in this case
				   {
					   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(GET_TX_TARGET_POWER) return error.\n");
					   throw logMessage;
				   }
				}
				else
				{  
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(GET_TX_TARGET_POWER) return OK.\n");
				}
				vDUT_GetDoubleParameter (g_WiFi_Dut, "TX_TARGET_POWER_DBM", &tx_target_power);
				l_txMultiVerificationParam.TX_POWER_DBM = tx_target_power;
				g_last_TxPower_dBm_Record = l_txMultiVerificationParam.TX_POWER_DBM;
			}
			else
			{
				l_txMultiVerificationParam.TX_POWER_DBM = g_last_TxPower_dBm_Record;
			}
		}
		else
		{
			// do nothing
		}
		/* <><~~ */

		vDUT_AddDoubleParameter (g_WiFi_Dut, "TX_POWER_DBM",	  l_txMultiVerificationParam.TX_POWER_DBM);
		TM_SetDoubleParameter(g_WiFi_Test_ID, "TX_POWER_DBM", 	l_txMultiVerificationParam.TX_POWER_DBM); // -cfy@sunnyvale, 2012/3/13-

		if ( (g_dutConfigChanged==true)||(g_vDutTxActived==false) )	
		{
			if ( g_vDutTxActived==true )
			{
			   /*-----------*
				*  Tx Stop  *
				*-----------*/
				err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");		
				if ( ERR_OK!=err )
				{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
					err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
					if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
					{
						err = -1;	// set err to -1, means "Error".
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
						throw logMessage;
					}
					else	// Just return normal error message in this case
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
						throw logMessage;
					}
				}
				else
				{
					g_vDutTxActived = false;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
				}
			}
			else
			{
				// continue Dut configuration				
			}

			err = vDUT_Run(g_WiFi_Dut, "RF_SET_FREQ");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(RF_SET_FREQ) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(RF_SET_FREQ) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_SET_BW");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_BW) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_BW) return OK.\n");
			}
			err = vDUT_Run(g_WiFi_Dut, "TX_SET_DATA_RATE");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_DATA_RATE) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_DATA_RATE) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_SET_ANTENNA");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_SET_ANTENNA) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_SET_ANTENNA) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_PRE_TX");
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_PRE_TX) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{  
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_PRE_TX) return OK.\n");
			}
			
			if ( (g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE)&&(enableMultiCapture==false) )
			{
			   /*--------------------------------------*
				* Set Trigger Timeout for DUT TX_START *
				*--------------------------------------*/
				err = ::LP_SetVsaTriggerTimeout(g_WiFiGlobalSettingParam.VSA_TRIGGER_TIMEOUT_SEC);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,  "[WiFi] Fail to set VSA trigger timeout, LP_SetVsaTriggerTimeout(%d sec) return error.\n", g_WiFiGlobalSettingParam.VSA_TRIGGER_TIMEOUT_SEC);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetVsaTriggerTimeout(%d sec) return OK.\n", g_WiFiGlobalSettingParam.VSA_TRIGGER_TIMEOUT_SEC);
				}

			   /*---------------------------*
				* New IQ2010Ext VSA capture *
				*---------------------------*/
				err = ::LP_IQ2010EXT_NewMultiAnalysisCapture(   l_txMultiVerificationParam.DATA_RATE,
																l_txMultiVerificationParam.FREQ_MHZ,
																l_txMultiVerificationParam.TX_POWER_DBM-cableLossDb+chainGainDb,
																g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
																g_WiFiGlobalSettingParam.VSA_PORT,
																l_txMultiVerificationParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
															  );
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new MultiAnalysisCapture at %d MHz.\n", l_txMultiVerificationParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_NewMultiAnalysisCapture() at %d MHz return OK.\n", l_txMultiVerificationParam.FREQ_MHZ);
				}	

			   /*----------------------------*
				* Add Multi-Analysis Capture *
				*----------------------------*/
				// IQ2010EXT_ANALYSIS_TYPE:	0=EVM, 1=MASK_20M, 2=MASK_40M, 3=POWER, 4=POWER_RAMP, 5=SPECTRUM.
				int analysisType = -1;

				//-----------//
				//    EVM    //
				//-----------//
				if ( l_txMultiVerificationParam.ENABLE_EVM==1 )
				{
					analysisType = 0;	// EVM

					double timeUs = CheckSamplingTime(wifiMode, l_txMultiVerificationParam.PREAMBLE, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.PACKET_FORMAT_11N);
					// Check sampling time 
					if (0==l_txMultiVerificationParam.CAPTURE_LENGTH_OF_EVM_US)
					{
						samplingTimeUs = timeUs;
					}
					else	// SAMPLING_TIME_US != 0
					{
						if ( l_txMultiVerificationParam.CAPTURE_LENGTH_OF_EVM_US>=timeUs )
						{
							samplingTimeUs = l_txMultiVerificationParam.CAPTURE_LENGTH_OF_EVM_US;
						}
						else
						{
							err = -1;
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] The CAPTURE_LENGTH_OF_EVM_US is too short, IQlite recommend %.1f us", timeUs );
							throw logMessage;
						}
					}

					err = ::LP_IQ2010EXT_AddMultiAnalysisCapture( analysisType, (int)samplingTimeUs, g_WiFiGlobalSettingParam.EVM_AVERAGE );
					if( ERR_OK!=err )	// capture is failed
					{
						// Fail Capture
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to add Multi-Analysis Capture, analysisType=%d.\n", analysisType );
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AddMultiAnalysisCapture(analysisType=%d, %d us) return OK.\n", analysisType, (int)samplingTimeUs );
					}	
				}

				//------------//
				//    MASK    //
				//------------//
				if ( l_txMultiVerificationParam.ENABLE_MASK==1 )
				{					
					if ( HT40ModeOn==1 )	// HT40 mode on
					{
						analysisType = 2;	// MASK HT40
					}
					else
					{
						analysisType = 1;	// MASK HT20
					}

					// Check Capture Time 
					if (0==l_txMultiVerificationParam.CAPTURE_LENGTH_OF_MASK_US)
					{
						if ( wifiMode==WIFI_11B )
						{
							samplingTimeUs = g_WiFiGlobalSettingParam.MASK_DSSS_SAMPLE_INTERVAL_US;
						}
						else if ( wifiMode==WIFI_11AG )
						{
							samplingTimeUs = g_WiFiGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US;
						}
						else	// 802.11n
						{		
							// TODO
							samplingTimeUs = g_WiFiGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US;
						}
					}
					else	// SAMPLING_TIME_US != 0
					{
						samplingTimeUs = l_txMultiVerificationParam.CAPTURE_LENGTH_OF_MASK_US;
					}

					err = ::LP_IQ2010EXT_AddMultiAnalysisCapture( analysisType, (int)samplingTimeUs, 1 );
					if( ERR_OK!=err )	// capture is failed
					{
						// Fail Capture
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to add Multi-Analysis Capture, analysisType=%d.\n", analysisType );
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AddMultiAnalysisCapture(analysisType=%d, %d us) return OK.\n", analysisType, (int)samplingTimeUs );
					}	
				}

				//-------------//
				//    POWER    //
				//-------------//
				if ( (l_txMultiVerificationParam.ENABLE_POWER==1)||(l_txMultiVerificationParam.ENABLE_MASK==1) )
				{
					analysisType = 3;	// POWER

					// Check sampling time 
					if (0==l_txMultiVerificationParam.CAPTURE_LENGTH_OF_POWER_US)
					{
						if ( wifiMode==WIFI_11B )
						{
							samplingTimeUs = g_WiFiGlobalSettingParam.PM_DSSS_SAMPLE_INTERVAL_US;
						}
						else if ( wifiMode==WIFI_11AG )
						{
							samplingTimeUs = g_WiFiGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US;
						}
						else	// 802.11n
						{		
							// TODO
							samplingTimeUs = g_WiFiGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US;
						}
					}
					else	// SAMPLING_TIME_US != 0
					{
						samplingTimeUs = l_txMultiVerificationParam.CAPTURE_LENGTH_OF_POWER_US;
					}

					err = ::LP_IQ2010EXT_AddMultiAnalysisCapture( analysisType, (int)samplingTimeUs, g_WiFiGlobalSettingParam.PM_AVERAGE );
					if( ERR_OK!=err )	// capture is failed
					{
						// Fail Capture
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to add Multi-Analysis Capture, analysisType=%d.\n", analysisType );
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AddMultiAnalysisCapture(analysisType=%d, %d us) return OK.\n", analysisType, (int)samplingTimeUs );
					}	
				}

				//----------------//
				//    SPECTRUM    //
				//----------------//
				if ( l_txMultiVerificationParam.ENABLE_SPECTRUM==1 )
				{
					BOOL bRunMultiAnalysisCapture = true;
					if( wifiMode == WIFI_11B )
					{
						if( l_txMultiVerificationParam.ENABLE_EVM == 0 )
						{
							analysisType = 0; //EVM: to obtain loLeakageDb
						}
						else
						{
							bRunMultiAnalysisCapture = false; //If ENABLE_EVM is 1, we have already do the multi analysis capture. So, skip it.
						}
					}
					else
					{
						analysisType = 6;	// SPECTRUM
					}

					// Check sampling time 
					if (0==l_txMultiVerificationParam.CAPTURE_LENGTH_OF_SPECTRUM_US)
					{
						if ( wifiMode==WIFI_11B )
						{
							samplingTimeUs = g_WiFiGlobalSettingParam.SPECTRUM_DSSS_SAMPLE_INTERVAL_US;
						}
						else if ( wifiMode==WIFI_11AG )
						{
							samplingTimeUs = g_WiFiGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US;
						}
						else	// 802.11n
						{		
							// TODO
							samplingTimeUs = g_WiFiGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US;
						}
					}
					else	// SAMPLING_TIME_US != 0
					{
						samplingTimeUs = l_txMultiVerificationParam.CAPTURE_LENGTH_OF_SPECTRUM_US;
					}
					
					if( bRunMultiAnalysisCapture )
					{
						err = ::LP_IQ2010EXT_AddMultiAnalysisCapture( analysisType, (int)samplingTimeUs, 1 );
						if( ERR_OK!=err )	// capture is failed
						{
							// Fail Capture
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to add Multi-Analysis Capture, analysisType=%d.\n", analysisType );
							throw logMessage;
						}
						else
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AddMultiAnalysisCapture(analysisType=%d, %d us) return OK.\n", analysisType, (int)samplingTimeUs );
						}
					}
					else{}//do nothing
				}

			   /*-------------------------------*
				* Perform IQ2010Ext VSA capture *
				*-------------------------------*/
				err = ::LP_IQ2010EXT_InitiateMultiAnalysisCapture();
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to initial MultiAnalysisCapture at %d MHz.\n", l_txMultiVerificationParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiAnalysisCapture() at %d MHz return OK.\n", l_txMultiVerificationParam.FREQ_MHZ);
				}
				
				enableMultiCapture = true;
			}

			err = vDUT_Run(g_WiFi_Dut, "TX_START");
			if ( ERR_OK!=err )
			{	
			   g_vDutTxActived = false;
			   // Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
			   err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
			   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			   {
				   err = -1;	// set err to -1, means "Error".
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
				   throw logMessage;
			   }
			   else	// Just return normal error message in this case
			   {
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_START) return error.\n");
				   throw logMessage;
			   }
			}
			else
			{
			   g_vDutTxActived = true;
			   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_START) return OK.\n");
			}

			// Delay for DUT settle
			if (0!=g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS)
			{
				Sleep(g_WiFiGlobalSettingParam.DUT_TX_SETTLE_TIME_MS);
			}
			else
			{
				// do nothing
			}
		}
		else
		{
			// g_dutConfigChanged==false, do nothing
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Since g_dutConfigChanged = false, skip Dut control section.\n");
		}
#pragma endregion

	   /*---------------------------*
		* Start IQ2010_Ext Function *
		*---------------------------*/	

		analysisOK = false;
		captureOK  = false;

#pragma region Auto_Gain_Control
		/*------------------------------
			Search VSA optimal amplitude
			Perform Power Analysis
		--------------------------------*/
		if(g_WiFiGlobalSettingParam.AUTO_GAIN_CONTROL_ENABLE)
		{

			if(enableMultiCapture)
			{
				err = ::LP_IQ2010EXT_FinishMultiAnalysisCapture();
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_FinishMultiAnalysisCapture() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_FinishMultiAnalysisCapture() return OK.\n");
				}	
			}
			else
			{
				//do nothing
			}
			// Check sampling time 
			if (0==l_txMultiVerificationParam.CAPTURE_LENGTH_OF_POWER_US)
			{
				if ( wifiMode==WIFI_11B )
				{
					samplingTimeUs = g_WiFiGlobalSettingParam.PM_DSSS_SAMPLE_INTERVAL_US;
				}
				else if ( wifiMode==WIFI_11AG )
				{
					samplingTimeUs = g_WiFiGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US;
				}
				else	// 802.11n
				{		
					// TODO
					samplingTimeUs = g_WiFiGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US;
				}
			}
			else	// SAMPLING_TIME_US != 0
			{
				samplingTimeUs = l_txMultiVerificationParam.CAPTURE_LENGTH_OF_POWER_US;
			}

			double powerMeasured = l_txMultiVerificationParam.TX_POWER_DBM - cableLossDb + chainGainDb;
			double dummyDouble   = NA_NUMBER;
			int    loopIndex     = 0;

			do
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Doing auto gain coontrol(Loop: %d).\n", loopIndex+1);

				//Skip 0 count and capture 1 packet
				err = ::LP_IQ2010EXT_InitiateMultiCapture(  l_txMultiVerificationParam.DATA_RATE,
															l_txMultiVerificationParam.FREQ_MHZ,
															powerMeasured,
															(int)samplingTimeUs,  
															0,
															1,
															g_WiFiGlobalSettingParam.VSA_PORT,
													        powerMeasured+peakToAvgRatio+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
												          );
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new initiate MultiCapture at %d MHz.\n", l_txMultiVerificationParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiCapture() at %d MHz return OK.\n", l_txMultiVerificationParam.FREQ_MHZ);
				}	

				err = ::LP_IQ2010EXT_FinishMultiCapture();
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_FinishMultiCapture() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_FinishMultiCapture() return OK.\n");
				}	
				//IQ2010Ext Power Analysis
				err = ::LP_IQ2010EXT_AnalyzePower();
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Multi_Verification_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_AnalyzePower(%d) return error.\n", l_txMultiVerificationParam.FREQ_MHZ);
					throw logMessage;
				}
				else
				{
					//do nothing
				}

				// P_av_no_gap_all_dBm  
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "P_av_no_gap_all_dBm", &powerMeasured,&dummyDouble, &dummyDouble);
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Multi_Verification_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(P_av_no_gap_all_dBm) return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(P_av_no_gap_all_dBm: %2.1f) return OK.\n", (powerMeasured + cableLossDb));
				}

				
				if( (powerMeasured > (l_txMultiVerificationParam.TX_POWER_DBM + g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB - cableLossDb)) ||
					(powerMeasured < (l_txMultiVerificationParam.TX_POWER_DBM - g_WiFiGlobalSettingParam.VSA_AMPLITUDE_TOLERANCE_DB - cableLossDb))
			      )
			     {
					 //do nothing, continue to next loop

				 }
				else
				{
					//VSA ampltude is optimal, break the loop						
					break;
				}

				loopIndex ++;

			}while(loopIndex < AUTO_GAIN_CONTROL_LOOP);

			
			//After get optimal vsa amplitude, initial capture and finish capture again
           
			if(powerMeasured > NA_NUMBER)
			{
				dutTxPower = powerMeasured + cableLossDb - chainGainDb;					
			}
			else
			{
				dutTxPower = l_txMultiVerificationParam.TX_POWER_DBM;
			}
			
		    enableMultiCapture = false;
		
		}
		else
		{
			//Did not perform auto gain control
		}
#pragma endregion

		if ( enableMultiCapture==false )
		{
		   /*---------------------------*
			* New IQ2010Ext VSA capture *
			*---------------------------*/
			if(g_WiFiGlobalSettingParam.AUTO_GAIN_CONTROL_ENABLE)
			{
				err	= ::LP_IQ2010EXT_NewMultiAnalysisCapture(   l_txMultiVerificationParam.DATA_RATE,
																l_txMultiVerificationParam.FREQ_MHZ,
																dutTxPower-cableLossDb+chainGainDb,
																g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
																g_WiFiGlobalSettingParam.VSA_PORT,
																dutTxPower-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
															  );

			}
			else
			{
				err = ::LP_IQ2010EXT_NewMultiAnalysisCapture(   l_txMultiVerificationParam.DATA_RATE,
																l_txMultiVerificationParam.FREQ_MHZ,
																l_txMultiVerificationParam.TX_POWER_DBM-cableLossDb+chainGainDb,
																g_WiFiGlobalSettingParam.VSA_SKIP_PACKET_COUNT,
																g_WiFiGlobalSettingParam.VSA_PORT,
																l_txMultiVerificationParam.TX_POWER_DBM-cableLossDb+peakToAvgRatio+chainGainDb+g_WiFiGlobalSettingParam.VSA_TRIGGER_LEVEL_DB
															  );
			}
			if( ERR_OK!=err )	// capture is failed
			{
				// Fail Capture
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to new MultiAnalysisCapture at %d MHz.\n", l_txMultiVerificationParam.FREQ_MHZ);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_NewMultiAnalysisCapture() at %d MHz return OK.\n", l_txMultiVerificationParam.FREQ_MHZ);
			}	

		   /*----------------------------*
			* Add Multi-Analysis Capture *
			*----------------------------*/
			// IQ2010EXT_ANALYSIS_TYPE:	0=EVM, 1=MASK_20M, 2=MASK_40M, 3=POWER, 4=POWER_RAMP, 5=SPECTRUM.
			int analysisType = -1;

			//-----------//
			//    EVM    //
			//-----------//
			if ( l_txMultiVerificationParam.ENABLE_EVM==1 )
			{
				analysisType = 0;	// EVM

				double timeUs = CheckSamplingTime(wifiMode, l_txMultiVerificationParam.PREAMBLE, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.PACKET_FORMAT_11N);
				// Check sampling time 
				if (0==l_txMultiVerificationParam.CAPTURE_LENGTH_OF_EVM_US)
				{
					samplingTimeUs = timeUs;
				}
				else	// SAMPLING_TIME_US != 0
				{
					if ( l_txMultiVerificationParam.CAPTURE_LENGTH_OF_EVM_US>=timeUs )
					{
						samplingTimeUs = l_txMultiVerificationParam.CAPTURE_LENGTH_OF_EVM_US;
					}
					else
					{
						err = -1;
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] The CAPTURE_LENGTH_OF_EVM_US is too short, IQlite recommend %.1f us", timeUs );
						throw logMessage;
					}
				}

				err = ::LP_IQ2010EXT_AddMultiAnalysisCapture( analysisType, (int)samplingTimeUs, g_WiFiGlobalSettingParam.EVM_AVERAGE );
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to add Multi-Analysis Capture, analysisType=%d.\n", analysisType );
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AddMultiAnalysisCapture(analysisType=%d, %d us) return OK.\n", analysisType, (int)samplingTimeUs );
				}	
			}

			//------------//
			//    MASK    //
			//------------//
			if ( l_txMultiVerificationParam.ENABLE_MASK==1 )
			{					
				if ( HT40ModeOn==1 )	// HT40 mode on
				{
					analysisType = 2;	// MASK HT40
				}
				else
				{
					analysisType = 1;	// MASK HT20
				}

				// Check Capture Time 
				if (0==l_txMultiVerificationParam.CAPTURE_LENGTH_OF_MASK_US)
				{
					if ( wifiMode==WIFI_11B )
					{
						samplingTimeUs = g_WiFiGlobalSettingParam.MASK_DSSS_SAMPLE_INTERVAL_US;
					}
					else if ( wifiMode==WIFI_11AG )
					{
						samplingTimeUs = g_WiFiGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US;
					}
					else	// 802.11n
					{		
						// TODO
						samplingTimeUs = g_WiFiGlobalSettingParam.MASK_OFDM_SAMPLE_INTERVAL_US;
					}
				}
				else	// SAMPLING_TIME_US != 0
				{
					samplingTimeUs = l_txMultiVerificationParam.CAPTURE_LENGTH_OF_MASK_US;
				}

				err = ::LP_IQ2010EXT_AddMultiAnalysisCapture( analysisType, (int)samplingTimeUs, 1 );
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to add Multi-Analysis Capture, analysisType=%d.\n", analysisType );
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AddMultiAnalysisCapture(analysisType=%d, %d us) return OK.\n", analysisType, (int)samplingTimeUs );
				}	
			}

			//-------------//
			//    POWER    //
			//-------------//
			if ( (l_txMultiVerificationParam.ENABLE_POWER==1)||(l_txMultiVerificationParam.ENABLE_MASK==1) )
			{
				analysisType = 3;	// POWER

				// Check sampling time 
				if (0==l_txMultiVerificationParam.CAPTURE_LENGTH_OF_POWER_US)
				{
					if ( wifiMode==WIFI_11B )
					{
						samplingTimeUs = g_WiFiGlobalSettingParam.PM_DSSS_SAMPLE_INTERVAL_US;
					}
					else if ( wifiMode==WIFI_11AG )
					{
						samplingTimeUs = g_WiFiGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US;
					}
					else	// 802.11n
					{		
						// TODO
						samplingTimeUs = g_WiFiGlobalSettingParam.PM_OFDM_SAMPLE_INTERVAL_US;
					}
				}
				else	// SAMPLING_TIME_US != 0
				{
					samplingTimeUs = l_txMultiVerificationParam.CAPTURE_LENGTH_OF_POWER_US;
				}

				err = ::LP_IQ2010EXT_AddMultiAnalysisCapture( analysisType, (int)samplingTimeUs, g_WiFiGlobalSettingParam.PM_AVERAGE );
				if( ERR_OK!=err )	// capture is failed
				{
					// Fail Capture
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to add Multi-Analysis Capture, analysisType=%d.\n", analysisType );
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AddMultiAnalysisCapture(analysisType=%d, %d us) return OK.\n", analysisType, (int)samplingTimeUs );
				}	
			}

			//----------------//
			//    SPECTRUM    //
			//----------------//
			if ( l_txMultiVerificationParam.ENABLE_SPECTRUM==1 )
			{
				BOOL bRunMultiAnalysisCapture = true;
				if( wifiMode == WIFI_11B )
				{
					if( l_txMultiVerificationParam.ENABLE_EVM == 0 )
					{
						analysisType = 0; //EVM: to obtain loLeakageDb
					}
					else
					{
						bRunMultiAnalysisCapture = false; //If ENABLE_EVM is 1, we have already do the multi analysis capture. So, skip it.
					}
				}
				else
				{
					analysisType = 6;	// SPECTRUM
				}

				// Check sampling time 
				if (0==l_txMultiVerificationParam.CAPTURE_LENGTH_OF_SPECTRUM_US)
				{
					if ( wifiMode==WIFI_11B )
					{
						samplingTimeUs = g_WiFiGlobalSettingParam.SPECTRUM_DSSS_SAMPLE_INTERVAL_US;
					}
					else if ( wifiMode==WIFI_11AG )
					{
						samplingTimeUs = g_WiFiGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US;
					}
					else	// 802.11n
					{		
						// TODO
						samplingTimeUs = g_WiFiGlobalSettingParam.SPECTRUM_OFDM_SAMPLE_INTERVAL_US;
					}
				}
				else	// SAMPLING_TIME_US != 0
				{
					samplingTimeUs = l_txMultiVerificationParam.CAPTURE_LENGTH_OF_SPECTRUM_US;
				}
				if( bRunMultiAnalysisCapture )
				{
					err = ::LP_IQ2010EXT_AddMultiAnalysisCapture( analysisType, (int)samplingTimeUs, 1 );					
					if( ERR_OK!=err )	// capture is failed
					{
						// Fail Capture
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to add Multi-Analysis Capture, analysisType=%d.\n", analysisType );
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AddMultiAnalysisCapture(analysisType=%d, %d us) return OK.\n", analysisType, (int)samplingTimeUs );
					}	
				}
				else{}//do nothing
			}

		   /*-------------------------------*
			* Perform IQ2010Ext VSA capture *
			*-------------------------------*/
			err = ::LP_IQ2010EXT_InitiateMultiAnalysisCapture();
			if( ERR_OK!=err )	// capture is failed
			{
				// Fail Capture
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] IQ2010EXT Fail to initial MultiAnalysisCapture at %d MHz.\n", l_txMultiVerificationParam.FREQ_MHZ);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_InitiateMultiAnalysisCapture() at %d MHz return OK.\n", l_txMultiVerificationParam.FREQ_MHZ);
			}
			
			enableMultiCapture = true;
		}

	   /*------------------------------*
		* Finish IQ2010Ext VSA capture *
		*------------------------------*/
		err = ::LP_IQ2010EXT_FinishMultiAnalysisCapture();
		if( ERR_OK!=err )	// capture is failed
		{
			// Fail Capture
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_FinishMultiAnalysisCapture() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_FinishMultiAnalysisCapture() return OK.\n");
		}	

		/*--------------*
		 *  Capture OK  *
		 *--------------*/
		captureOK = true;
		if (1==g_WiFiGlobalSettingParam.VSA_SAVE_CAPTURE_ALWAYS)
		{
			// TODO: must give a warning that VSA_SAVE_CAPTURE_ALWAYS is ON
			sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Multi_Verification_SaveAlways", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
			WiFiSaveSigFile(sigFileNameBuffer);
		}
		else
		{
			// do nothing
		}


		/*--------------------------*
		 *  IQ2010Ext EVM Analysis  *
		 *--------------------------*/
		if ( l_txMultiVerificationParam.ENABLE_EVM==1 )
		{
#pragma region Analysis_802_11b
			if ( wifiMode==WIFI_11B )                            
			{   // [Case 01]: 802.11b Analysis
				err = ::LP_IQ2010EXT_Analyze80211b( (IQV_EQ_ENUM)		  g_WiFiGlobalSettingParam.ANALYSIS_11B_EQ_TAPS,
													(IQV_DC_REMOVAL_ENUM) g_WiFiGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG,
													(IQV_11B_METHOD_ENUM) g_WiFiGlobalSettingParam.ANALYSIS_11B_METHOD_11B
												  );
				if (ERR_OK!=err)
				{	// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Multi_Verification_Analyze80211b_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);

					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_Analyze80211b() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_Analyze80211b() return OK.\n");
				}				
			}
#pragma endregion
#pragma region Analysis_802_11ag
			else if ( wifiMode==WIFI_11AG )  
			{   // [Case 02]: 802.11a/g Analysis
				err = ::LP_IQ2010EXT_Analyze80211ag(  (IQV_PH_CORR_ENUM)	g_WiFiGlobalSettingParam.ANALYSIS_11AG_PH_CORR_MODE,
													  (IQV_CH_EST_ENUM)		g_WiFiGlobalSettingParam.ANALYSIS_11AG_CH_ESTIMATE,
													  (IQV_SYM_TIM_ENUM)	g_WiFiGlobalSettingParam.ANALYSIS_11AG_SYM_TIM_CORR,
													  (IQV_FREQ_SYNC_ENUM)	g_WiFiGlobalSettingParam.ANALYSIS_11AG_FREQ_SYNC,
													  (IQV_AMPL_TRACK_ENUM) g_WiFiGlobalSettingParam.ANALYSIS_11AG_AMPL_TRACK
													);
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Multi_Verification_Analyze80211ag_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_Analyze80211ag() return error.\n");					
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_Analyze80211ag() return OK.\n");
				}
			}
#pragma endregion
#pragma region Analysis_802_11n
			else if ( (wifiMode==WIFI_11N_HT20)||(wifiMode==WIFI_11N_HT40) )                         
			{   // [Case 03]: MIMO Analysis
				char referenceFileName[MAX_BUFFER_SIZE], analyzeMimoType[MAX_BUFFER_SIZE], analyzeMimoMode[MAX_BUFFER_SIZE];

				//Set Frequency Correction for 802.11n analysis
				err = ::LP_SetAnalysisParameterInteger("Analyze80211n", "frequencyCorr", g_WiFiGlobalSettingParam.ANALYSIS_11N_FREQUENCY_CORRELATION);
				if ( ERR_OK!=err )
				{
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_SetAnalysisParameterInteger() return error.\n");
				   throw logMessage;
				}
				else
				{
				   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_SetAnalysisParameterInteger() return OK.\n");
				}

				switch( g_Tester_Type )
				{
					case IQ_View:
						sprintf_s(analyzeMimoType, "EWC");                        
						if ( wifiStreamNum==WIFI_ONE_STREAM )  // MCS0 ~ MCS7
						{        
							sprintf_s(analyzeMimoMode, "nxn");
							sprintf_s(referenceFileName, ""); 

							err = ::LP_IQ2010EXT_Analyze80211nSISO( g_WiFiGlobalSettingParam.ANALYSIS_11N_PHASE_CORR, 
																	g_WiFiGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR, 
																	g_WiFiGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING, 
																	g_WiFiGlobalSettingParam.ANALYSIS_11N_DECODE_PSDU,
																	g_WiFiGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST,
																	packetFormat); 
							if ( ERR_OK!=err )
							{
								// Fail Analysis, thus save capture (Signal File) for debug
								sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Multi_Verification_AnalyzeMIMO_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
								WiFiSaveSigFile(sigFileNameBuffer);
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_Analyze80211nSISO() return error.\n");
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_Analyze80211nSISO() return OK.\n");
							}
						}
						else // MCS8 ~ MCS15 or more, need MIMO reference File *.ref
						{
							sprintf_s(analyzeMimoMode, "composite");
							err = GetWaveformFileName("EVM", 
													  "REFERENCE_FILE_NAME", 
													  wifiMode, 
													  l_txMultiVerificationParam.BANDWIDTH, 
													  l_txMultiVerificationParam.DATA_RATE, 
													  l_txMultiVerificationParam.PREAMBLE, 
													  l_txMultiVerificationParam.PACKET_FORMAT_11N, 
													  l_txMultiVerificationParam.GUARD_INTERVAL_11N,
													  referenceFileName, 
													  MAX_BUFFER_SIZE);							
							if ( ERR_OK!=err )
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get reference file name.\n");
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] GetWaveformFileName(REFERENCE_FILE_NAME) return OK.\n");
							}

							err = ::LP_IQ2010EXT_Analyze80211nComposite( g_WiFiGlobalSettingParam.ANALYSIS_11N_PHASE_CORR, 
																		 g_WiFiGlobalSettingParam.ANALYSIS_11N_SYM_TIMING_CORR, 
																		 g_WiFiGlobalSettingParam.ANALYSIS_11N_AMPLITUDE_TRACKING, 
																		 g_WiFiGlobalSettingParam.ANALYSIS_11N_FULL_PACKET_CHANNEL_EST, 
																		 referenceFileName,
																		 packetFormat);
							if ( ERR_OK!=err )
							{
								// Fail Analysis, thus save capture (Signal File) for debug
								sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Multi_Verification_AnalyzeMIMO_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
								WiFiSaveSigFile(sigFileNameBuffer);
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_Analyze80211nComposite() return error.\n");
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_Analyze80211nComposite() return OK.\n");
							}
						}
						break;

					case IQ_nxn:
						sprintf_s(analyzeMimoType, "EWC");
						sprintf_s(analyzeMimoMode, "nxn");
						sprintf_s(referenceFileName, "");
						
						// TODO: IQ2010Ext not supported

						break; 

					default:
						sprintf_s(analyzeMimoType, "EWC");
						sprintf_s(analyzeMimoMode, "nxn");
						sprintf_s(referenceFileName, "");

						// TODO: IQ2010Ext not supported

						break;
				}
			}
#pragma endregion

#pragma region Retrieve analysis Results
			/*---------------------------------------*
			 *  Retrieve IQ2010EXT analysis Results  *
			 *---------------------------------------*/
			char sTestDetail[MAX_BUFFER_SIZE] = {'\0'};
			sprintf_s(sTestDetail, MAX_BUFFER_SIZE, "WiFi_TX_Multi_Verification_%d_%s", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE);
			analysisOK = true;

			// Store the result, includes EVM, power etc.              
			if ( (wifiMode==WIFI_11B)||(wifiMode==WIFI_11AG) )        // 802.11a/b/g, but not n mode, only one stream
			{
				// Number of spatial streams
				l_txMultiVerificationReturn.SPATIAL_STREAM = 1;

				// EVM
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "evmAll", &l_txMultiVerificationReturn.EVM_AVG[0], &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.EVM_AVG_DB = l_txMultiVerificationReturn.EVM_AVG[0] = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(evmAll) return error.\n");
					throw logMessage;
				}
				else
				{
					l_txMultiVerificationReturn.EVM_AVG_DB = l_txMultiVerificationReturn.EVM_AVG[0];
				}
				
				// Peak EVM
				if (wifiMode==WIFI_11B)
				{
					err = ::LP_IQ2010EXT_GetDoubleMeasurements( "evmPk", &dummyAvg, &dummyMin, &l_txMultiVerificationReturn.EVM_PK_DB );
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						l_txMultiVerificationReturn.EVM_PK_DB = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(evmPk) return error.\n");
						throw logMessage;
					}
				}

				// Power 
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "rmsPowerNoGap", &l_txMultiVerificationReturn.POWER_AVG[0], &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.POWER_AVG_DBM = l_txMultiVerificationReturn.POWER_AVG[0] = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(rmsPowerNoGap) return error.\n");
					throw logMessage;
				}
				else
				{
					// One stream data rate, should be only one antenna is ON	
					int antenaOrder = 0;
					err = CheckAntennaOrderByStream(l_txMultiVerificationReturn.SPATIAL_STREAM, l_txMultiVerificationParam.TX1, l_txMultiVerificationParam.TX2, l_txMultiVerificationParam.TX3, l_txMultiVerificationParam.TX4, &antenaOrder);
					if ( ERR_OK!=err )
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");					
						throw logMessage;
					}

					l_txMultiVerificationReturn.POWER_AVG[0]	 = l_txMultiVerificationReturn.POWER_AVG[0] + l_txMultiVerificationParam.CABLE_LOSS_DB[antenaOrder-1];
					l_txMultiVerificationReturn.POWER_AVG_DBM    = l_txMultiVerificationReturn.POWER_AVG[0];
				}

				// Frequency Error
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "freqErr", &l_txMultiVerificationReturn.FREQ_ERROR_AVG, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.FREQ_ERROR_AVG = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(freqErr) return error.\n");
					throw logMessage;
				}
				else
				{
					// Transfer the Freq Error result unit to ppm
					l_txMultiVerificationReturn.FREQ_ERROR_AVG = l_txMultiVerificationReturn.FREQ_ERROR_AVG/l_txMultiVerificationParam.FREQ_MHZ;
				}

				// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "ampErrDb", &l_txMultiVerificationReturn.AMP_ERR_DB, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.AMP_ERR_DB = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(ampErrDb) return error.\n");
					throw logMessage;
				}

				// IQ Match Phase Error.
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "phaseErr", &l_txMultiVerificationReturn.PHASE_ERR, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.PHASE_ERR = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(phaseErr) return error.\n");
					throw logMessage;
				}

				// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "rmsPhaseNoise", &l_txMultiVerificationReturn.PHASE_NOISE_RMS_ALL, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.PHASE_NOISE_RMS_ALL = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(rmsPhaseNoise) return error.\n");
					throw logMessage;
				}

				// Symbol Clock Error for 11a/b/g
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "clockErr", &l_txMultiVerificationReturn.SYMBOL_CLK_ERR, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.SYMBOL_CLK_ERR = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(clockErr) return error.\n");
					throw logMessage;
				}

				// Datarate 						
				if (wifiMode==WIFI_11B)
				{
					double dummyAvg = 0, dummyMax = 0, dummyMin = 0;
					//int dummyInt = (int)NA_NUMBER;
					//err = ::LP_IQ2010EXT_GetIntMeasurement( "bitRateInMHz", &dummyInt, 0 );
					err = ::LP_IQ2010EXT_GetDoubleMeasurements( "bitRateInMHz", &dummyAvg, &dummyMin, &dummyMax );
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						l_txMultiVerificationReturn.DATA_RATE = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetIntMeasurement(bitRateInMHz) return error.\n");
						throw logMessage;
					}
					else
					{
						l_txMultiVerificationReturn.DATA_RATE = dummyAvg;
					}
				}
				else
				{ 
					int dummyInt = (int)NA_NUMBER;
					err = ::LP_IQ2010EXT_GetIntMeasurement( "dataRate", &dummyInt, 0 );
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						l_txMultiVerificationReturn.DATA_RATE = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetIntMeasurement(dataRate) return error.\n");
						throw logMessage;
					}
					else
					{
						l_txMultiVerificationReturn.DATA_RATE = dummyInt;
					}
				}

			}
			else    // 802.11n, includes more than one stream
			{
				// TODO:  Now the IQ2010Ext only support to MCS 0 ~ 7, so index should be always = 0.

				// Number of spatial streams
				double	dummyDoule = NA_NUMBER;
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "rateInfo_spatialStreams", &dummyDoule, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.SPATIAL_STREAM = (int)NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(rateInfo_spatialStreams) return error.\n");
					throw logMessage;
				}
				else
				{
					l_txMultiVerificationReturn.SPATIAL_STREAM = (int)dummyDoule;
				}


				for(int i=0;i<l_txMultiVerificationReturn.SPATIAL_STREAM;i++)
				{
					// EVM  ( only can get EVM_ALL, means index always = 0 )
					err = ::LP_IQ2010EXT_GetDoubleMeasurements( "evmAvgAll", &l_txMultiVerificationReturn.EVM_AVG[i], &dummyMin, &dummyMax );
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						l_txMultiVerificationReturn.EVM_AVG_DB = l_txMultiVerificationReturn.EVM_AVG[i] = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(evmAvgAll) return error.\n");
						throw logMessage;
					}
					else
					{
						l_txMultiVerificationReturn.EVM_AVG_DB = l_txMultiVerificationReturn.EVM_AVG[i];
					}

					// Power 
					err = ::LP_IQ2010EXT_GetDoubleMeasurements( "rxRmsPowerDb", &l_txMultiVerificationReturn.POWER_AVG[i], &dummyMin, &dummyMax );
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						l_txMultiVerificationReturn.POWER_AVG_DBM = l_txMultiVerificationReturn.POWER_AVG[i] = NA_NUMBER;
						WiFiSaveSigFile(sTestDetail);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(rxRmsPowerDb) return error.\n");
						throw logMessage;
					}
					else
					{	
						int antenaOrder = 0;
						err = CheckAntennaOrderByStream(i+1, l_txMultiVerificationParam.TX1, l_txMultiVerificationParam.TX2, l_txMultiVerificationParam.TX3, l_txMultiVerificationParam.TX4, &antenaOrder);
						if ( ERR_OK!=err )
						{
							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");					
							throw logMessage;
						}

						l_txMultiVerificationReturn.POWER_AVG[i]	 = l_txMultiVerificationReturn.POWER_AVG[i] + l_txMultiVerificationParam.CABLE_LOSS_DB[antenaOrder-1];
						l_txMultiVerificationReturn.POWER_AVG_DBM    = l_txMultiVerificationReturn.POWER_AVG[i];
					}

				}

				// IQ Match Amplitude Error  (IQ gain imbalance in dB, per stream)
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "IQImbal_amplDb", &l_txMultiVerificationReturn.AMP_ERR_DB, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.AMP_ERR_DB = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(IQImbal_amplDb) return error.\n");
					throw logMessage;
				}

				// IQ Match Phase Error.
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "IQImbal_phaseDeg", &l_txMultiVerificationReturn.PHASE_ERR, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.PHASE_ERR = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(IQImbal_phaseDeg) return error.\n");
					throw logMessage;
				}

				// Frequency Error
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "freqErrorHz", &l_txMultiVerificationReturn.FREQ_ERROR_AVG, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.FREQ_ERROR_AVG = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(freqErrorHz) return error.\n");
					throw logMessage;
				}
				else
				{
					// Transfer the Freq Error result unit to ppm
					l_txMultiVerificationReturn.FREQ_ERROR_AVG = l_txMultiVerificationReturn.FREQ_ERROR_AVG/l_txMultiVerificationParam.FREQ_MHZ;
				}

				// RMS Phase Noise, mappin the value "RMS Phase Noise" in IQsignal
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "PhaseNoiseDeg_RmsAll", &l_txMultiVerificationReturn.PHASE_NOISE_RMS_ALL, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.PHASE_NOISE_RMS_ALL = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(PhaseNoiseDeg_RmsAll) return error.\n");
					throw logMessage;
				}

				// Datarate 			
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "rateInfo_dataRateMbps", &l_txMultiVerificationReturn.DATA_RATE, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.DATA_RATE = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(rateInfo_dataRateMbps) return error.\n");
					throw logMessage;
				}

				// Symbol Clock Error for 11n
				err = ::LP_IQ2010EXT_GetDoubleMeasurements( "symClockErrorPpm", &l_txMultiVerificationReturn.SYMBOL_CLK_ERR, &dummyMin, &dummyMax );
				if ( ERR_OK!=err )
				{
					analysisOK = false;
					l_txMultiVerificationReturn.SYMBOL_CLK_ERR = NA_NUMBER;
					WiFiSaveSigFile(sTestDetail);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(symClockErrorPpm) return error.\n");
					throw logMessage;
				}
		
			}         
		}
#pragma endregion

		/*---------------------------*
		 *  IQ2010Ext MASK Analysis  *
		 *---------------------------*/
		if ( l_txMultiVerificationParam.ENABLE_MASK==1 )
		{
#pragma region Analysis
			// maskType can be (0):WIFI_11B, (1):WIFI_11AG, (2):WIFI_11N_HT20 or (3):WIFI_11N_HT40
			err = ::LP_IQ2010EXT_AnalyzeMaskMeasurement( wifiMode, l_txMultiVerificationParam.OBW_PERCENTAGE/100 );	
			if ( ERR_OK!=err )
			{
				// Fail Analysis, thus save capture (Signal File) for debug
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_AnalyzeMaskMeasurement(%d) return error.\n", wifiMode);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AnalyzeMaskMeasurement(%d) return OK.\n", wifiMode);
			}
#pragma endregion

#pragma region Retrieve analysis Results
			/*-----------------------------------*
			 *  Retrieve IQ2010EXT Mask Results  *
			 *-----------------------------------*/
			analysisOK = true;

			

			err = ::LP_IQ2010EXT_GetNumberOfMeasurementElements( "x", 0, &numberOfResultSet );
			if ( numberOfResultSet<=0 )
			{
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Can not retrieve FFT result, result length = 0.\n");
				throw logMessage;
			}                   
			else
			{
				::LP_IQ2010EXT_GetVectorMeasurement( "x", bufferRealX, MAX_BUFFER_SIZE, &dataSize, 0 );
				::LP_IQ2010EXT_GetVectorMeasurement( "y", bufferRealY, MAX_BUFFER_SIZE, &dataSize, 0 );

				l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[0]->clear();
				l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[0]->clear();

				//Return Mask Raw Data
				for(int i=0;i<numberOfResultSet;i++)
				{
					l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[0]->push_back(bufferRealX[i]);
				}
				for(int i=0;i<numberOfResultSet;i++)
				{
					l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[0]->push_back(bufferRealY[i]);  
				}
			}

			err = ::LP_IQ2010EXT_GetDoubleMeasurements( "VIOLATION_PERCENT_VSA1", &l_txMultiVerificationReturn.VIOLATION_PERCENT, &dummyMin, &dummyMax );
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txMultiVerificationReturn.VIOLATION_PERCENT = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(VIOLATION_PERCENT_VSA1) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(VIOLATION_PERCENT_VSA1) return OK.\n");
			}
			
			err = ::LP_IQ2010EXT_GetDoubleMeasurements( "OBW_MHZ_VSA1", &l_txMultiVerificationReturn.OBW_MHZ, &dummyMin, &dummyMax );
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txMultiVerificationReturn.OBW_MHZ = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(OBW_MHZ_VSA1) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(OBW_MHZ_VSA1) return OK.\n");
			}

			err  = ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_LO_A_VSA1", &l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[0], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_LO_B_VSA1", &l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[1], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_UP_A_VSA1", &l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[0], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_UP_B_VSA1", &l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[1], &dummyMin, &dummyMax );
			if ( wifiMode!=WIFI_11B )	
			{
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_LO_C_VSA1", &l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[2], &dummyMin, &dummyMax );			
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_LO_D_VSA1", &l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[3], &dummyMin, &dummyMax );
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_UP_C_VSA1", &l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[2], &dummyMin, &dummyMax );			
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "FREQ_MHZ_UP_D_VSA1", &l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[3], &dummyMin, &dummyMax );
			}
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[0] = l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[1] = l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[2] = l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[3] = NA_NUMBER;
				l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[0] = l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[1] = l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[2] = l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[3] = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(FREQ_MHZ_VSA) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(FREQ_MHZ_VSA) return OK.\n");
			}

			// Transfer the absolute frequency results to relative frequency offset
			//for (int i=0;i<MAX_POSITIVE_SECTION;i++)
			//{
			//	if (l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[i]!=NA_NUMBER)		
			//	{
			//		l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[i] = l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[i]-l_txMultiVerificationParam.FREQ_MHZ;
			//	}
			//	if (l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[i]!=NA_NUMBER)		
			//	{
			//		l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[i] = l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[i]-l_txMultiVerificationParam.FREQ_MHZ;
			//	}
			//}

			err  = ::LP_IQ2010EXT_GetDoubleMeasurements( "MARGIN_DB_LO_A_VSA1", &l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[0], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MARGIN_DB_LO_B_VSA1", &l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[1], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MARGIN_DB_UP_A_VSA1", &l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[0], &dummyMin, &dummyMax );
			err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MARGIN_DB_UP_B_VSA1", &l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[1], &dummyMin, &dummyMax );
			if ( wifiMode!=WIFI_11B )	
			{
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MARGIN_DB_LO_C_VSA1", &l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[2], &dummyMin, &dummyMax );
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MARGIN_DB_LO_D_VSA1", &l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[3], &dummyMin, &dummyMax );
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MARGIN_DB_UP_C_VSA1", &l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[2], &dummyMin, &dummyMax );
				err += ::LP_IQ2010EXT_GetDoubleMeasurements( "MARGIN_DB_UP_D_VSA1", &l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[3], &dummyMin, &dummyMax );
			}
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[0] = l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[1] = l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[2] = l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[3] = NA_NUMBER;
				l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[0] = l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[1] = l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[2] = l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[3] = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Mask_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(MARGIN_DB_VSA) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(MARGIN_DB_VSA) return OK.\n");
			}
#pragma endregion
		}

		/*----------------------------*
		 *  IQ2010Ext Power Analysis  *
		 *----------------------------*/
		if ( (l_txMultiVerificationParam.ENABLE_POWER==1)||(l_txMultiVerificationParam.ENABLE_MASK==1) )
		{
			err = ::LP_IQ2010EXT_AnalyzePower();
			if ( ERR_OK!=err )
			{
				// Fail Analysis, thus save capture (Signal File) for debug
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_AnalyzePower(%d) return error.\n", l_txMultiVerificationParam.FREQ_MHZ);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AnalyzePower() return OK.\n");
			}

#pragma region Retrieve analysis Results
			/*---------------------------------------*
			 *  Retrieve IQ2010EXT analysis Results  *
			 *---------------------------------------*/			 
			analysisOK = true;

			// P_av_no_gap_all_dBm 
			err = ::LP_IQ2010EXT_GetDoubleMeasurements( "P_av_no_gap_all_dBm", &l_txMultiVerificationReturn.POWER_AVG_DBM, &dummyMin, &dummyMax );
			if ( ERR_OK!=err )
			{
				analysisOK = false;
				l_txMultiVerificationReturn.POWER_AVG_DBM = NA_NUMBER;
				sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Power_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
				WiFiSaveSigFile(sigFileNameBuffer);
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(P_av_no_gap_all_dBm) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(P_av_no_gap_all_dBm) return OK.\n");
				// Since the limitation, we assume that all path loss value are very close.	
				int antenaOrder = 0;
				err = CheckAntennaOrderByStream(1, l_txMultiVerificationParam.TX1, l_txMultiVerificationParam.TX2, l_txMultiVerificationParam.TX3, l_txMultiVerificationParam.TX4, &antenaOrder);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] CheckAntennaOrderByStream() return error.\n");					
					throw logMessage;
				}

				l_txMultiVerificationReturn.POWER_AVG_DBM = l_txMultiVerificationReturn.POWER_AVG_DBM + l_txMultiVerificationParam.CABLE_LOSS_DB[antenaOrder-1];
			}
#pragma endregion
		}


		/*-------------------------------*
		 *  IQ2010Ext Spectrum Analysis  *
		 *-------------------------------*/
		if ( l_txMultiVerificationParam.ENABLE_SPECTRUM==1 )
		{
			//For 80211b
			if( wifiMode == WIFI_11B )
			{
				//Run ::LP_IQ2010EXT_Analyze80211b to get loLeakageDb
				if( l_txMultiVerificationParam.ENABLE_EVM == 0 )
				{	
					err = ::LP_IQ2010EXT_Analyze80211b( (IQV_EQ_ENUM)		  g_WiFiGlobalSettingParam.ANALYSIS_11B_EQ_TAPS,
															   (IQV_DC_REMOVAL_ENUM) g_WiFiGlobalSettingParam.ANALYSIS_11B_DC_REMOVE_FLAG,
															   (IQV_11B_METHOD_ENUM) g_WiFiGlobalSettingParam.ANALYSIS_11B_METHOD_11B
															  );
					if (ERR_OK!=err)
					{	// Fail Analysis, thus save capture (Signal File) for debug
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Multi_Verification_Analyze80211b_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);

						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_Analyze80211b() return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_Analyze80211b() return OK.\n");
					}		
				}
				else
				{
					//do nothing
				}

			}
			else
			{
#pragma region Analysis
				// spectralType can be (0):WIFI_11AG, (1):WIFI_11N_HT20, (2):WIFI_11N_HT40 or (3):WIFI_11N
				err = ::LP_IQ2010EXT_AnalyzeSpectralMeasurement( wifiMode-1 );	
				if ( ERR_OK!=err )
				{
					// Fail Analysis, thus save capture (Signal File) for debug
					sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
					WiFiSaveSigFile(sigFileNameBuffer);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_AnalyzeSpectralMeasurement(%d) return error.\n", wifiMode-1);
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AnalyzeSpectralMeasurement(%d) return OK.\n", wifiMode-1);
				}
	#pragma endregion

	#pragma region Retrieve analysis Results
				/*---------------------------------------*
				 *  Retrieve IQ2010EXT Spectrum Results  *
				 *---------------------------------------*/
				analysisOK = true;
				//For 11b, get "loLeakageDb".
				if( wifiMode == WIFI_11B )
				{
					double dloLeakage = NA_NUMBER;
					err = ::LP_IQ2010EXT_GetDoubleMeasurements( "loLeakageDb", &dloLeakage, &dummyMin, &dummyMax );
					if ( ERR_OK != err )
					{
						analysisOK = false;
						l_txMultiVerificationReturn.LO_LEAKAGE_DB = NA_NUMBER;
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(loLeakage) return error.\n");
						throw logMessage;
					}
					else
						l_txMultiVerificationReturn.LO_LEAKAGE_DB = dloLeakage;

				}
				else
				{//For other types, get "LO_LEAKAGE_DBR_VSA1".
					err = ::LP_IQ2010EXT_GetDoubleMeasurements( "LO_LEAKAGE_DBR_VSA1", &l_txMultiVerificationReturn.LO_LEAKAGE_DB, &dummyMin, &dummyMax );

					if ( ERR_OK!=err )
					{
						analysisOK = false;
						l_txMultiVerificationReturn.LO_LEAKAGE_DB = NA_NUMBER;
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(LO_LEAKAGE_DBR_VSA1) return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(LO_LEAKAGE_DBR_VSA1) return OK.\n");
					}

					err  = ::LP_IQ2010EXT_GetIntMeasurement( "SUBCARRIER_LO_A_VSA1", &l_txMultiVerificationReturn.SUBCARRIER_LO_A, 0 ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
					err += ::LP_IQ2010EXT_GetIntMeasurement( "SUBCARRIER_LO_B_VSA1", &l_txMultiVerificationReturn.SUBCARRIER_LO_B, 0 ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
					err += ::LP_IQ2010EXT_GetIntMeasurement( "SUBCARRIER_UP_A_VSA1", &l_txMultiVerificationReturn.SUBCARRIER_UP_A, 0 ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
					err += ::LP_IQ2010EXT_GetIntMeasurement( "SUBCARRIER_UP_B_VSA1", &l_txMultiVerificationReturn.SUBCARRIER_UP_B, 0 ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetIntMeasurement(SUBCARRIER_VSA1) return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetIntMeasurement(SUBCARRIER_VSA1) return OK.\n");
					}

					err  = ::LP_IQ2010EXT_GetDoubleMeasurements( "VALUE_DB_LO_A_VSA1", &l_txMultiVerificationReturn.VALUE_DB_LO_A, &dummyMin, &dummyMax ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
					err += ::LP_IQ2010EXT_GetDoubleMeasurements( "VALUE_DB_LO_B_VSA1", &l_txMultiVerificationReturn.VALUE_DB_LO_B, &dummyMin, &dummyMax ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
					err += ::LP_IQ2010EXT_GetDoubleMeasurements( "VALUE_DB_UP_A_VSA1", &l_txMultiVerificationReturn.VALUE_DB_UP_A, &dummyMin, &dummyMax ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
					err += ::LP_IQ2010EXT_GetDoubleMeasurements( "VALUE_DB_UP_B_VSA1", &l_txMultiVerificationReturn.VALUE_DB_UP_B, &dummyMin, &dummyMax ); // Modified /* -cfy@sunnyvale, 2012/3/13- */
					if ( ERR_OK!=err )
					{
						analysisOK = false;
						sprintf_s(sigFileNameBuffer, MAX_BUFFER_SIZE, "%s_%d_%s_%s", "WiFi_TX_Spectrum_Analysis_Failed", l_txMultiVerificationParam.FREQ_MHZ, l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH);
						WiFiSaveSigFile(sigFileNameBuffer);
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "LP_IQ2010EXT_GetDoubleMeasurements(VALUE_DB_VSA1) return error.\n");
						throw logMessage;
					}
					else
					{
						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_GetDoubleMeasurements(VALUE_DB_VSA1) return OK.\n");
					}

				}
	#pragma endregion
			}
		}



		// This is a special case, only if "DUT_KEEP_TRANSMIT=0" then must do TX_STOP manually.
		if ( (g_WiFiGlobalSettingParam.DUT_KEEP_TRANSMIT==0)&&(g_vDutTxActived==true) )
		{
		   /*-----------*
			*  Tx Stop  *
			*-----------*/
			err = ::vDUT_Run(g_WiFi_Dut, "TX_STOP");		
			if ( ERR_OK!=err )
			{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
				err = ::vDUT_GetStringReturn(g_WiFi_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
				if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
				{
					err = -1;	// set err to -1, means "Error".
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
					throw logMessage;
				}
				else	// Just return normal error message in this case
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] vDUT_Run(TX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/		 
		if ( ERR_OK==err && captureOK && analysisOK )
		{
			sprintf_s(l_txMultiVerificationReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_txMultiVerificationReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_txMultiVerificationReturn.ERROR_MESSAGE, msg);

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
	}
	catch(...)
	{
		ReturnErrorMessage(l_txMultiVerificationReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
		err = -1;

		if ( g_vDutTxActived )
		{
			int err = ERR_OK;
			err = vDUT_Run(g_WiFi_Dut, "TX_STOP");
			if( err == ERR_OK )
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] vDUT_Run(TX_STOP) return OK.\n");
			}
		}
	}

	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i]->clear();
		l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->clear();
	}


    return err;
}


int InitializeTXMultiVerificationContainers(void)
{
    /*------------------*
     * Input Parameters  *
     *------------------*/
    l_txMultiVerificationParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    strcpy_s(l_txMultiVerificationParam.BANDWIDTH, MAX_BUFFER_SIZE, "HT20");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txMultiVerificationParam.BANDWIDTH))    // Type_Checking
    {
        setting.value       = (void*)l_txMultiVerificationParam.BANDWIDTH;
        setting.unit        = "";
        setting.helpText    = "Channel bandwidth\r\nValid options: HT20 or HT40";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BANDWIDTH", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txMultiVerificationParam.DATA_RATE, MAX_BUFFER_SIZE, "OFDM-54");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txMultiVerificationParam.DATA_RATE))    // Type_Checking
    {
        setting.value       = (void*)l_txMultiVerificationParam.DATA_RATE;
        setting.unit        = "";
        setting.helpText    = "Data rate names:\r\nDSSS-1,DSSS-2,CCK-5_5,CCK-11\r\nOFDM-6,OFDM-9,OFDM-12,OFDM-18,OFDM-24,OFDM-36,OFDM-48,OFDM-54\r\nMCS0, MCS15, etc.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txMultiVerificationParam.PREAMBLE, MAX_BUFFER_SIZE, "LONG");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txMultiVerificationParam.PREAMBLE))    // Type_Checking
    {
        setting.value       = (void*)l_txMultiVerificationParam.PREAMBLE;
        setting.unit        = "";
        setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=LONG.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_txMultiVerificationParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, "MIXED");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txMultiVerificationParam.PACKET_FORMAT_11N))    // Type_Checking
    {
        setting.value       = (void*)l_txMultiVerificationParam.PACKET_FORMAT_11N;
        setting.unit        = "";
        setting.helpText    = "The packet format of 11N(only), can be MIXED or GREENFIELD, Default=MIXED.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT_11N", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txMultiVerificationParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, "LONG");
    setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txMultiVerificationParam.GUARD_INTERVAL_11N))    // Type_Checking
    {
		setting.value       = (void*)l_txMultiVerificationParam.GUARD_INTERVAL_11N;
        setting.unit        = "";
        setting.helpText    = "The guard interval format of 11N(only), can be LONG or SHORT, Default=LONG.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL_11N", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	strcpy_s(l_txMultiVerificationParam.TX_POWER_CLASS, MAX_BUFFER_SIZE, "A");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_txMultiVerificationParam.TX_POWER_CLASS))    // Type_Checking
	{
		setting.value       = (void*)l_txMultiVerificationParam.TX_POWER_CLASS;
		setting.unit        = "";
		setting.helpText    = "The power class of 11p(only), can be A, B, C or D, Default=A.";
		l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_CLASS", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_txMultiVerificationParam.ENABLE_EVM = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txMultiVerificationParam.ENABLE_EVM))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.ENABLE_EVM;
        setting.unit        = "";
        setting.helpText    = "Enable TX EVM test, default=1, means ON.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ENABLE_EVM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationParam.ENABLE_MASK = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txMultiVerificationParam.ENABLE_MASK))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.ENABLE_MASK;
        setting.unit        = "";
        setting.helpText    = "Enable TX MASK test, default=1, means ON.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ENABLE_MASK", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationParam.ENABLE_POWER = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txMultiVerificationParam.ENABLE_POWER))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.ENABLE_POWER;
        setting.unit        = "";
        setting.helpText    = "Enable TX POWER test, default=1, means ON.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ENABLE_POWER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationParam.ENABLE_SPECTRUM = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txMultiVerificationParam.ENABLE_SPECTRUM))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.ENABLE_SPECTRUM;
        setting.unit        = "";
        setting.helpText    = "Enable TX SPECTRUM test, default=1, means ON.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ENABLE_SPECTRUM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txMultiVerificationParam.FREQ_MHZ = 2412;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txMultiVerificationParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationParam.OBW_PERCENTAGE= 99.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationParam.OBW_PERCENTAGE))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.OBW_PERCENTAGE;
        setting.unit        = "%";
        setting.helpText    = "The percentage of power for OBW measurement, default = 99%";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_PERCENTAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txMultiVerificationParam.CAPTURE_LENGTH_OF_EVM_US = 0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationParam.CAPTURE_LENGTH_OF_EVM_US))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.CAPTURE_LENGTH_OF_EVM_US;
        setting.unit        = "us";
        setting.helpText    = "The capture length (time in us) to verify EVM.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAPTURE_LENGTH_OF_EVM_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationParam.CAPTURE_LENGTH_OF_MASK_US = 0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationParam.CAPTURE_LENGTH_OF_MASK_US))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.CAPTURE_LENGTH_OF_MASK_US;
        setting.unit        = "us";
        setting.helpText    = "The capture length (time in us) to verify MASK.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAPTURE_LENGTH_OF_MASK_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationParam.CAPTURE_LENGTH_OF_POWER_US = 0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationParam.CAPTURE_LENGTH_OF_POWER_US))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.CAPTURE_LENGTH_OF_POWER_US;
        setting.unit        = "us";
        setting.helpText    = "The capture length (time in us) to verify POWER.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAPTURE_LENGTH_OF_POWER_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationParam.CAPTURE_LENGTH_OF_SPECTRUM_US = 0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationParam.CAPTURE_LENGTH_OF_SPECTRUM_US))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.CAPTURE_LENGTH_OF_SPECTRUM_US;
        setting.unit        = "us";
        setting.helpText    = "The capture length (time in us) to verify SPECTRUM.";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CAPTURE_LENGTH_OF_SPECTRUM_US", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txMultiVerificationParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txMultiVerificationParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_txMultiVerificationParam.CABLE_LOSS_DB[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txMultiVerificationParam.TX_POWER_DBM = 15.0;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationParam.TX_POWER_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txMultiVerificationParam.TX1 = 1;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txMultiVerificationParam.TX1))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.TX1;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX1", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txMultiVerificationParam.TX2 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txMultiVerificationParam.TX2))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.TX2;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX2", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txMultiVerificationParam.TX3 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txMultiVerificationParam.TX3))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.TX3;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX3", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txMultiVerificationParam.TX4 = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txMultiVerificationParam.TX4))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationParam.TX4;
        setting.unit        = "";
        setting.helpText    = "DUT TX path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX4", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationParam.ARRAY_HANDLING_METHOD = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txMultiVerificationParam.ARRAY_HANDLING_METHOD))    // Type_Checking
    {
        setting.value = (void*)&l_txMultiVerificationParam.ARRAY_HANDLING_METHOD;
        setting.unit        = "";
        setting.helpText    = "Use the flag to handle array result. Default: 0; 0: Do nothing, 1: Display result on screen, 2: Display result and log to file";
        l_txMultiVerificationParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ARRAY_HANDLING_METHOD", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }
    
    l_txMultiVerificationParam.CORRECTION_FACTOR_11B = 9.2;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationParam.CORRECTION_FACTOR_11B))    // Type_Checking
    {
        setting.value = (void*)&l_txMultiVerificationParam.CORRECTION_FACTOR_11B;
        setting.unit  = "dB";
        setting.helpText  = "802.11b(only) LO leakage correction factor. Chipset specific.";
        l_txMultiVerificationParamMap.insert( pair<string, WIFI_SETTING_STRUCT>("CORRECTION_FACTOR_11B", setting) );
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
    l_txMultiVerificationReturnMap.clear();

    l_txMultiVerificationReturn.EVM_AVG_DB = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationReturn.EVM_AVG_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationReturn.EVM_AVG_DB;
        setting.unit        = "dB";
        setting.helpText    = "EVM average over captured packets and all data streams(MIMO)";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_AVG_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txMultiVerificationReturn.EVM_AVG[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txMultiVerificationReturn.EVM_AVG[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txMultiVerificationReturn.EVM_AVG[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "EVM_AVG_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Average EVM over captured packets on individual streams(MIMO)";
            l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    l_txMultiVerificationReturn.EVM_PK_DB = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationReturn.EVM_PK_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationReturn.EVM_PK_DB;
        setting.unit        = "dB";
        setting.helpText    = "(11b only)Peak EVM over captured packets.";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("EVM_PK_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationReturn.AMP_ERR_DB = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationReturn.AMP_ERR_DB))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationReturn.AMP_ERR_DB;
        setting.unit        = "dB";
        setting.helpText    = "IQ Match Amplitude Error in dB.";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("AMP_ERR_DB", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for(int i=0;i<MAX_DATA_STREAM;i++)
	{
	    l_txMultiVerificationReturn.AMP_ERR_DB_STREAM[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txMultiVerificationReturn.AMP_ERR_DB_STREAM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txMultiVerificationReturn.AMP_ERR_DB_STREAM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "AMP_ERR_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "IQ Match Amplitude Error in dB.(per stream)";
            l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
	}

	l_txMultiVerificationReturn.PHASE_ERR = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationReturn.PHASE_ERR))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationReturn.PHASE_ERR;
        setting.unit        = "Degree";
        setting.helpText    = "IQ Match Phase Error.";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PHASE_ERR", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationReturn.SYMBOL_CLK_ERR = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txMultiVerificationReturn.SYMBOL_CLK_ERR))    // Type_Checking
	{
		setting.value       = (void*)&l_txMultiVerificationReturn.SYMBOL_CLK_ERR;
		setting.unit        = "ppm";
		setting.helpText    = "Symbol Clock Error";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SYMBOL_CLK_ERR", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	for(int i=0;i<MAX_DATA_STREAM;i++)
	{
	    l_txMultiVerificationReturn.PHASE_ERR_STREAM[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txMultiVerificationReturn.PHASE_ERR_STREAM[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txMultiVerificationReturn.PHASE_ERR_STREAM[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "PHASE_ERR_%d", i+1);
			setting.unit        = "Degree";
			setting.helpText    = "IQ Match Phase Error.(per stream)";
            l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
	}

	//PHASE_NOISE_RMS_ALL
	l_txMultiVerificationReturn.PHASE_NOISE_RMS_ALL = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationReturn.PHASE_NOISE_RMS_ALL))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationReturn.PHASE_NOISE_RMS_ALL;
        setting.unit        = "Degree";
        setting.helpText    = "Frequency RMS Phase Noise.";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("PHASE_NOISE_RMS_ALL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_txMultiVerificationReturn.POWER_AVG_DBM = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationReturn.POWER_AVG_DBM))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationReturn.POWER_AVG_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Average Power over captured packets";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_AVG_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    for (int i=0;i<MAX_DATA_STREAM;i++)
    {
        l_txMultiVerificationReturn.POWER_AVG[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txMultiVerificationReturn.POWER_AVG[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txMultiVerificationReturn.POWER_AVG[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "POWER_AVG_%d", i+1);
            setting.unit        = "dBm";
            setting.helpText    = "Average Power over captured packets on individual streams";
            l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    l_txMultiVerificationReturn.FREQ_ERROR_AVG = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationReturn.FREQ_ERROR_AVG))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationReturn.FREQ_ERROR_AVG;
        setting.unit        = "ppm";
        setting.helpText    = "Average frequency error over captured packets";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("FREQ_ERROR_AVG", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationReturn.DATA_RATE = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationReturn.DATA_RATE))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationReturn.DATA_RATE;
        setting.unit        = "Mbps";
        setting.helpText    = "Data rate in Mbps, reported by IQAPI.";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationReturn.SPATIAL_STREAM = (int)NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_txMultiVerificationReturn.SPATIAL_STREAM))    // Type_Checking
    {
        setting.value       = (void*)&l_txMultiVerificationReturn.SPATIAL_STREAM;
        setting.unit        = "";
        setting.helpText    = "Number of spatial stream, reported by IQAPI.";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SPATIAL_STREAM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_txMultiVerificationReturn.CABLE_LOSS_DB[i] = NA_NUMBER;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_txMultiVerificationReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_txMultiVerificationReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    for (int i=0;i<MAX_POSITIVE_SECTION;i++)
    {
		l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txMultiVerificationReturn.MARGIN_DB_POSITIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "MASK_MARGIN_DB_UPPER_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
            l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_NEGATIVE_SECTION;i++)
    {
		l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txMultiVerificationReturn.MARGIN_DB_NEGATIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "MASK_MARGIN_DB_LOWER_%d", i+1);
            setting.unit        = "dB";
            setting.helpText    = "Margin to the mask, normally should be a negative number, if pass.";
            l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_POSITIVE_SECTION;i++)
    {
		l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txMultiVerificationReturn.FREQ_AT_MARGIN_POSITIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "FREQ_AT_MARGIN_UPPER_%d", i+1);
            setting.unit        = "MHz";
            setting.helpText    = "Point out the frequency offset at margin to the mask.";
            l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }

    for (int i=0;i<MAX_NEGATIVE_SECTION;i++)
    {
		l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[i] = NA_NUMBER;
        setting.type = WIFI_SETTING_TYPE_DOUBLE;
        if (sizeof(double)==sizeof(l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[i]))    // Type_Checking
        {
            setting.value = (void*)&l_txMultiVerificationReturn.FREQ_AT_MARGIN_NEGATIVE[i];
            char tempStr[MAX_BUFFER_SIZE];
            sprintf_s(tempStr, "FREQ_AT_MARGIN_LOWER_%d", i+1);
            setting.unit        = "MHz";
            setting.helpText    = "Point out the frequency offset at margin to the mask.";
            l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
        }
        else    
        {
            printf("Parameter Type Error!\n");
            exit(1);
        }
    }
    
    l_txMultiVerificationReturn.VIOLATION_PERCENT = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationReturn.VIOLATION_PERCENT))    // Type_Checking
    {
        setting.value = (void*)&l_txMultiVerificationReturn.VIOLATION_PERCENT;
        setting.unit        = "%";
        setting.helpText    = "Percentage which fail the mask";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VIOLATION_PERCENT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationReturn.OBW_MHZ = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationReturn.OBW_MHZ))    // Type_Checking
    {
        setting.value = (void*)&l_txMultiVerificationReturn.OBW_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "OBW for the specified power percentage";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("OBW_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }


	// -cfy@sunnyvale, 2012/3/13-			
	l_txMultiVerificationReturn.TX_POWER_DBM = NA_NUMBER;
    setting.type = WIFI_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_txMultiVerificationReturn.TX_POWER_DBM))   // Type_Checking
    {
        setting.value = (void*)&l_txMultiVerificationReturn.TX_POWER_DBM;
        setting.unit        = "dBm";
        setting.helpText    = "Expected power level at DUT antenna port";
        l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("TX_POWER_DBM", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_txMultiVerificationReturn.LEAST_MARGIN_SUBCARRIER = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txMultiVerificationReturn.LEAST_MARGIN_SUBCARRIER))    // Type_Checking
	{
		setting.value = (void*)&l_txMultiVerificationReturn.LEAST_MARGIN_SUBCARRIER;
		setting.unit        = "";
		setting.helpText    = "Carrier no. with least margin";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("LEAST_MARGIN_SUBCARRIER", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txMultiVerificationReturn.SUBCARRIER_LO_A = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txMultiVerificationReturn.SUBCARRIER_LO_A))    // Type_Checking
	{
		setting.value = (void*)&l_txMultiVerificationReturn.SUBCARRIER_LO_A;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of lower section A in each VSA.";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_LO_A", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txMultiVerificationReturn.SUBCARRIER_LO_B = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txMultiVerificationReturn.SUBCARRIER_LO_B))    // Type_Checking
	{
		setting.value = (void*)&l_txMultiVerificationReturn.SUBCARRIER_LO_B;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of lower section B in each VSA.";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_LO_B", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txMultiVerificationReturn.SUBCARRIER_UP_A = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txMultiVerificationReturn.SUBCARRIER_UP_A))    // Type_Checking
	{
		setting.value = (void*)&l_txMultiVerificationReturn.SUBCARRIER_UP_A;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of upper section A in each VSA.";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_UP_A", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txMultiVerificationReturn.SUBCARRIER_UP_B = NA_INTEGER;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_txMultiVerificationReturn.SUBCARRIER_UP_B))    // Type_Checking
	{
		setting.value = (void*)&l_txMultiVerificationReturn.SUBCARRIER_UP_B;
		setting.unit        = "";
		setting.helpText    = "Margin at which carrier of upper section B in each VSA.";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("SUBCARRIER_UP_B", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txMultiVerificationReturn.MARGIN_DB = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txMultiVerificationReturn.MARGIN_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txMultiVerificationReturn.MARGIN_DB;
		setting.unit        = "dB";
		setting.helpText    = "Margin to the average spectral center power in each VSA.";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("MARGIN_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
    
	l_txMultiVerificationReturn.LO_LEAKAGE_DB = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txMultiVerificationReturn.LO_LEAKAGE_DB))    // Type_Checking
	{
		setting.value = (void*)&l_txMultiVerificationReturn.LO_LEAKAGE_DB;
		setting.unit        = "dBc";
		setting.helpText    = "Lo leakage in dBc, normally should be a negative number in VSA, if pass.";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("LO_LEAKAGE_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txMultiVerificationReturn.VALUE_DB_LO_A = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txMultiVerificationReturn.VALUE_DB_LO_A))    // Type_Checking
	{
		setting.value = (void*)&l_txMultiVerificationReturn.VALUE_DB_LO_A;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of lower section A in each VSA.";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_LO_A", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txMultiVerificationReturn.VALUE_DB_LO_B = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txMultiVerificationReturn.VALUE_DB_LO_B))    // Type_Checking
	{
		setting.value = (void*)&l_txMultiVerificationReturn.VALUE_DB_LO_B;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of lower section B in each VSA.";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_LO_B", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txMultiVerificationReturn.VALUE_DB_UP_A = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txMultiVerificationReturn.VALUE_DB_UP_A))    // Type_Checking
	{
		setting.value = (void*)&l_txMultiVerificationReturn.VALUE_DB_UP_A;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of upper section A in each VSA.";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_UP_A", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_txMultiVerificationReturn.VALUE_DB_UP_B = NA_NUMBER;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_txMultiVerificationReturn.VALUE_DB_UP_B))    // Type_Checking
	{
		setting.value = (void*)&l_txMultiVerificationReturn.VALUE_DB_UP_B;
		setting.unit        = "dB";
		setting.helpText    = "Actually margin value to the average spectral center power of upper section B in each VSA.";
		l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("VALUE_DB_UP_B", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	/* <><~~ */
	

	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i] = new vector<double>();
		l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i]->clear();
		setting.type = WIFI_SETTING_TYPE_ARRAY_DOUBLE;
		if (0 == l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i]->size())    // Type_Checking
		{
			setting.value = (void*)l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_X_VSA[i];
			setting.unit        = "Hz";
			setting.helpText    = "Spectrum X raw data.";
			char tmpStr[MAX_BUFFER_SIZE];
			sprintf_s(tmpStr, MAX_BUFFER_SIZE, "RAW_DATA_FREQ_VSA%d", i+1);
			l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tmpStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	for(int i=0;i<MAX_TESTER_NUM;i++)
	{
		l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i] = new vector<double>();
		l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->clear();
		setting.type = WIFI_SETTING_TYPE_ARRAY_DOUBLE;
		if (0 == l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i]->size())    // Type_Checking
		{
			setting.value = (void*)l_txMultiVerificationReturn.SPECTRUM_RAW_DATA_Y_VSA[i];
			setting.unit        = "dBm/100kHz";
			setting.helpText    = "Spectrum Y raw data.";
			char tmpStr[MAX_BUFFER_SIZE];
			sprintf_s(tmpStr, MAX_BUFFER_SIZE, "RAW_DATA_POWER_VSA%d", i+1);
		    l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tmpStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

    l_txMultiVerificationReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_txMultiVerificationReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_txMultiVerificationReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
       l_txMultiVerificationReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

//-------------------------------------------------------------------------------------
// This is a function for checking the input parameters before the test.
// 
//-------------------------------------------------------------------------------------
int CheckTxVerificationParameters( int *channel, int *wifiMode, int *wifiStreamNum, double *cableLossDb, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		// Checking the Input Parameters
		err = TM_WiFiConvertFrequencyToChannel(l_txMultiVerificationParam.FREQ_MHZ, channel);      
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown FREQ_MHZ, convert WiFi frequncy %d to channel failed.\n", l_txMultiVerificationParam.FREQ_MHZ);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		err = TM_WiFiConvertDataRateNameToIndex(l_txMultiVerificationParam.DATA_RATE, &dummyInt);      
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_txMultiVerificationParam.DATA_RATE);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] TM_WiFiConvertFrequencyToChannel() return OK.\n");
		}

		if ( 0!=strcmp(l_txMultiVerificationParam.BANDWIDTH, "HT20") && 0!=strcmp(l_txMultiVerificationParam.BANDWIDTH, "HT40") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown BANDWIDTH, WiFi bandwidth %s not supported.\n", l_txMultiVerificationParam.BANDWIDTH);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txMultiVerificationParam.PREAMBLE, "SHORT") && 0!=strcmp(l_txMultiVerificationParam.PREAMBLE, "LONG") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_txMultiVerificationParam.PREAMBLE);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txMultiVerificationParam.PACKET_FORMAT_11N, "MIXED") && 0!=strcmp(l_txMultiVerificationParam.PACKET_FORMAT_11N, "GREENFIELD") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown PACKET_FORMAT_11N, WiFi 11n packet format %s not supported.\n", l_txMultiVerificationParam.PACKET_FORMAT_11N);
			throw logMessage;
		}
		if ( 0!=strcmp(l_txMultiVerificationParam.GUARD_INTERVAL_11N, "LONG") && 0!=strcmp(l_txMultiVerificationParam.GUARD_INTERVAL_11N, "SHORT") )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Unknown GUARD_INTERVAL_11N, WiFi 11n guard interval format %s not supported.\n", l_txMultiVerificationParam.GUARD_INTERVAL_11N);
			throw logMessage;
		}

		// Convert parameter
		err = WiFiTestMode(l_txMultiVerificationParam.DATA_RATE, l_txMultiVerificationParam.BANDWIDTH, wifiMode, wifiStreamNum);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Convert WiFi test mode function failed.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFiTestMode() return OK.\n");
		}

		// Check path loss (by ant and freq)
		err = CheckPathLossTableExt( g_WiFi_Test_ID,
								  l_txMultiVerificationParam.FREQ_MHZ,
								  l_txMultiVerificationParam.TX1,
								  l_txMultiVerificationParam.TX2,
								  l_txMultiVerificationParam.TX3,
								  l_txMultiVerificationParam.TX4,
								  l_txMultiVerificationParam.CABLE_LOSS_DB,
								  l_txMultiVerificationReturn.CABLE_LOSS_DB,
								  cableLossDb,
								  TX_TABLE
								);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to get TX_CABLE_LOSS_DB of Path_%d from path loss table.\n", err);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] CheckPathLossTableExt(TX_TABLE) return OK.\n");
		}

		if ( *wifiMode==WIFI_11B )  
		{
			if ( *channel==14 )
			{
				*peakToAvgRatio = g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_CH14;    // CCK (Channel 14)   
			}
			else
			{
				*peakToAvgRatio = g_WiFiGlobalSettingParam.IQ_P_TO_A_11B_11M;    // CCK 
			}
		}
		else
		{
			*peakToAvgRatio = g_WiFiGlobalSettingParam.IQ_P_TO_A_11AG_54M;   // OFDM 
		}

		// Check Dut configuration changed or not
		if (  g_WiFiGlobalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.
			  0!=strcmp(l_txMultiVerificationParam.BANDWIDTH, g_RecordedParam.BANDWIDTH) ||
			  0!=strcmp(l_txMultiVerificationParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
			  0!=strcmp(l_txMultiVerificationParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
			  0!=strcmp(l_txMultiVerificationParam.PACKET_FORMAT_11N, g_RecordedParam.PACKET_FORMAT_11N) ||
			  0!=strcmp(l_txMultiVerificationParam.GUARD_INTERVAL_11N, g_RecordedParam.GUARD_INTERVAL_11N) ||
			  l_txMultiVerificationParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			  l_txMultiVerificationParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			  l_txMultiVerificationParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			  l_txMultiVerificationParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			  l_txMultiVerificationParam.FREQ_MHZ!=g_RecordedParam.FREQ_MHZ ||
			  l_txMultiVerificationParam.TX_POWER_DBM!=g_RecordedParam.POWER_DBM ||
			  l_txMultiVerificationParam.TX1!=g_RecordedParam.ANT1 ||
			  l_txMultiVerificationParam.TX2!=g_RecordedParam.ANT2 ||
			  l_txMultiVerificationParam.TX3!=g_RecordedParam.ANT3 ||
			  l_txMultiVerificationParam.TX4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;			
		}
		else
		{
			g_dutConfigChanged = false;
		}

		// Save the current setup
		g_RecordedParam.ANT1					= l_txMultiVerificationParam.TX1;
		g_RecordedParam.ANT2					= l_txMultiVerificationParam.TX2;
		g_RecordedParam.ANT3					= l_txMultiVerificationParam.TX3;
		g_RecordedParam.ANT4					= l_txMultiVerificationParam.TX4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_txMultiVerificationParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_txMultiVerificationParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_txMultiVerificationParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_txMultiVerificationParam.CABLE_LOSS_DB[3];	
		g_RecordedParam.FREQ_MHZ				= l_txMultiVerificationParam.FREQ_MHZ;	
		g_RecordedParam.POWER_DBM				= l_txMultiVerificationParam.TX_POWER_DBM;	

		sprintf_s(g_RecordedParam.BANDWIDTH, MAX_BUFFER_SIZE, l_txMultiVerificationParam.BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_txMultiVerificationParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_txMultiVerificationParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT_11N, MAX_BUFFER_SIZE, l_txMultiVerificationParam.PACKET_FORMAT_11N);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_txMultiVerificationParam.GUARD_INTERVAL_11N);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] CheckTxVerificationParameters() Confirmed.\n");
	}
	catch(char *msg)
    {
        sprintf_s(errorMsg, MAX_BUFFER_SIZE, msg);
    }
    catch(...)
    {
		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi] Unknown Error!\n");
		err = -1;
    }

	
	return err;
}

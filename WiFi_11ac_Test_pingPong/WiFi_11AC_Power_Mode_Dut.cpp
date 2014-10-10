#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_11AC_Test.h"
#include "WiFi_11AC_Test_Internal.h"
#include "IQmeasure.h"
#include "vDUT.h"

using namespace std;

// These global variables are declared in WiFi_Test_Internal.cpp
extern TM_ID                 g_WiFi_11ac_Test_ID;    
extern vDUT_ID               g_WiFi_11ac_Dut;
extern bool					 g_vDutTxActived;
extern bool					 g_dutConfigChanged;
extern WIFI_RECORD_PARAM     g_RecordedParam;

// This global variable is declared in WiFi_Global_Setting.cpp
extern WIFI_GLOBAL_SETTING g_WiFi11ACGlobalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_11ACPowerModeDutParamMap;

// Return Value Container
map<string, WIFI_SETTING_STRUCT> l_11ACPowerModeDutReturnMap;


struct tagParam
{
	// Mandatory Parameters
	
	char DUT_POWER_MODE[MAX_BUFFER_SIZE];              /*! DUT power mode.                   */
	char DUT_OPERATION[MAX_BUFFER_SIZE];               /*! DUT operation, start or stop      */
	//for general parameters
    int    BSS_FREQ_MHZ_PRIMARY;                            /*!< For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment */
	int    BSS_FREQ_MHZ_SECONDARY;                          /*!< For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined */
	int    CH_FREQ_MHZ_PRIMARY_20MHz;                       /*!< The center frequency (MHz) for primary 20 MHZ channel, priority is lower than "CH_FREQ_MHZ"   */
	int    CH_FREQ_MHZ;										/*!< The center frequency (MHz) for channel, when it is zero,"CH_FREQ_MHZ_PRIMARY_20" will be used  */
	int    NUM_STREAM_11AC;							        /*!< The number of streams 11AC (Only). */
    char   BSS_BANDWIDTH[MAX_BUFFER_SIZE];                  /*!< The RF bandwidth of basic service set (BSS) to verify EVM. */
	char   CH_BANDWIDTH[MAX_BUFFER_SIZE];                   /*!< The RF channel bandwidth to verify EVM. */
	char   DATA_RATE[MAX_BUFFER_SIZE];				   /*! The data rate to verify POWER.    */
	double POWER_DBM;                                  /*! The output power .                */
	int    CONTINOUS_TX_MODE;			               /*! Whether to send out continuous signal. */

	//for TX current test
	
	char   PREAMBLE[MAX_BUFFER_SIZE];                  /*! The preamble type of 11B(only).   */
	char   PACKET_FORMAT[MAX_BUFFER_SIZE];                  /*!< The packet format of 11AC and 11N. */
	char   GUARD_INTERVAL[MAX_BUFFER_SIZE];				/*! The guard interval format of 11N/AC(only). */
	char   STANDARD[MAX_BUFFER_SIZE];				        /*!< The standard parameter used for signal analysis option or to discriminate the same data rates/package formats from different standards */
	double CABLE_LOSS_DB[MAX_DATA_STREAM];             /*! The path loss of test system.     */
	
	//double T_INTERVAL;                               /*! This field is used to specify the interval that is used to determine if power is present or not. */
	//double MAX_POW_DIFF_DB;                          /*! This field is used to specify the maximum power difference between packets that are expected to be detected. */

	// DUT Parameters
	int    ATTEN1;                                     /*!< DUT attenuator1 on/off. Default=1(on)  */
	int    ATTEN2;                                     /*!< DUT attenuator2 on/off. Default=0(off) */
	int    ATTEN3;                                     /*!< DUT attenuator3 on/off. Default=0(off) */
	int    ATTEN4;                                     /*!< DUT attenuator4 on/off. Default=0(off) */

	// waveform paramters
	char   WAVEFORM_NAME[MAX_BUFFER_SIZE]; 						    /*! File name of waveform to verify PER */
	
} l_11ACPowerModeDutParam;

struct tagReturn
{   
	double CABLE_LOSS_DB[MAX_DATA_STREAM];          /*! The path loss of test system. */
	//channel list
	int    CH_FREQ_MHZ_PRIMARY_40MHz;       /*!< The center frequency (MHz) for primary 40 MHZ channel  */
	int    CH_FREQ_MHZ_PRIMARY_80MHz;       /*!< The center frequency (MHz) for primary 80 MHZ channel  */

	char   ERROR_MESSAGE[MAX_BUFFER_SIZE];
	char   WAVEFORM_NAME[MAX_BUFFER_SIZE]; 						    /*! File name of waveform to verify PER */
} l_11ACPowerModeDutReturn;
#pragma endregion

#ifndef WIN32
int init11ACPowerModeDutContainers = Initialize11ACPowerModeDutContainers();
#endif

// These global variables/functions only for WiFi_TX_Verify_Power.cpp
int Check11ACPowerModeParameters(int *bssBW, 
						  int *cbw,
						  int *bssPchannel, 
						  int *bssSchannel,
						  int *wifiMode, 
						  int *wifiStreamNum, 
						  double *samplingTimeUs, 
						  double *cableLossDb,
						  double *peakToAvgRatio, 
						  char* errorMsg );


//! WiFi TX Verify POWER
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
*      -# A string for error message
*
* \return 0 No error occurred
* \return -1 Error(s) occurred.  Please see the returned error message for details
*/

enum E_CURR_TEST_MODE{E_CURR_TEST_TX, E_CURR_TEST_RX,E_CURR_TEST_SLEEP,E_CURR_TEST_IDLE,E_CURR_TEST_ERROR};
enum E_CURR_TEST_OPERATION{E_OPERATION_START, E_OPERATION_STOP,E_OPERATION_ERROR};

WIFI_11AC_TEST_API int WIFI_11AC_Power_Mode_Dut(void)
{
	int    err = ERR_OK;

//	bool   analysisOK = false, captureOK  = false;     
//	int    avgIteration = 0;
    int    bssPchannel = 0,bssSchannel = 0;
    int    dummyValue = 0;
    int    wifiMode = 0, wifiStreamNum = 0, VSAanylisisFormat = 0;
//	double samplingTimeUs = 0;
	int    bssBW = 0, cbw = 0;
	int    VSAcaptureMode;  //0: 20MHz		1: 40MHz	2:80MHz		3: 160MHz		4: 80_80MHz
    double MaxVSGPowerlimit = 0;
	double samplingTimeUs = 0, peakToAvgRatio = 0, cableLossDb = 0;
	char   vErrorMsg[MAX_BUFFER_SIZE]  = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};
//	char   sigFileNameBuffer[MAX_BUFFER_SIZE] = {'\0'};

    int  current_test_mode = E_CURR_TEST_ERROR;
	int  current_operation_mode = E_OPERATION_ERROR;
	/*---------------------------------------*
	* Clear Return Parameters and Container *
	*---------------------------------------*/
	ClearReturnParameters(l_11ACPowerModeDutReturnMap);

	/*------------------------*
	* Respond to QUERY_INPUT *
	*------------------------*/
	err = TM_GetIntegerParameter(g_WiFi_11ac_Test_ID, "QUERY_INPUT", &dummyValue);
	if( ERR_OK==err )
	{
		RespondToQueryInput(l_11ACPowerModeDutParamMap);
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
		RespondToQueryReturn(l_11ACPowerModeDutReturnMap);
		return err;
	}
	else
	{
		// do nothing
	}
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
		err = GetInputParameters(l_11ACPowerModeDutParamMap);
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

		err = Check11ACPowerModeParameters( &bssBW, &cbw,&bssPchannel,&bssSchannel, &wifiMode, &wifiStreamNum, &samplingTimeUs, &cableLossDb, &peakToAvgRatio, vErrorMsg );
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Prepare input parameters CheckTxPowerParameters() return OK.\n");
		}
#pragma endregion

#pragma region Configure DUT to transmit
		/*---------------------------*
		 * Configure DUT to transmit *
		 *---------------------------*/
		// Set DUT RF frquency, tx power, antenna, data rate
		// And clear vDut parameters at beginning.
		vDUT_ClearParameters(g_WiFi_11ac_Dut);

		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_FREQ_MHZ_PRIMARY",		l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_FREQ_MHZ_SECONDARY",		l_11ACPowerModeDutParam.BSS_FREQ_MHZ_SECONDARY);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ",					l_11ACPowerModeDutParam.CH_FREQ_MHZ);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_20MHz",	l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_40MHz",	l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ_PRIMARY_80MHz",	l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
	
		if ((wifiMode== WIFI_11N_GF_HT40) || ( wifiMode== WIFI_11N_MF_HT40 ))
		{
			//vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_FREQ_MHZ",			l_11ACPowerModeDutParam.CH_FREQ_MHZ);
			vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "PRIMARY_FREQ",		l_11ACPowerModeDutParam.CH_FREQ_MHZ-10);
			vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "SECONDARY_FREQ",		l_11ACPowerModeDutParam.CH_FREQ_MHZ+10);
		}
		else
		{
			//do nothing
		}		  

		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "BSS_BANDWIDTH",		bssBW);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "CH_BANDWIDTH",		cbw);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "NUM_STREAM_11AC",		l_11ACPowerModeDutParam.NUM_STREAM_11AC);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "PREAMBLE",			l_11ACPowerModeDutParam.PREAMBLE);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "PACKET_FORMAT",		l_11ACPowerModeDutParam.PACKET_FORMAT);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "GUARD_INTERVAL",			l_11ACPowerModeDutParam.GUARD_INTERVAL);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "STANDARD",		l_11ACPowerModeDutParam.STANDARD);
		vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "TX_POWER_DBM",		l_11ACPowerModeDutParam.POWER_DBM);
		vDUT_AddStringParameter (g_WiFi_11ac_Dut, "DATA_RATE",			l_11ACPowerModeDutParam.DATA_RATE);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX1",					l_11ACPowerModeDutParam.ATTEN1);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX2",					l_11ACPowerModeDutParam.ATTEN2);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX3",					l_11ACPowerModeDutParam.ATTEN3);
		vDUT_AddIntegerParameter(g_WiFi_11ac_Dut, "TX4",					l_11ACPowerModeDutParam.ATTEN4);
        vDUT_AddDoubleParameter (g_WiFi_11ac_Dut, "SAMPLING_TIME_US",	samplingTimeUs);

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

		if ( strstr ( l_11ACPowerModeDutParam.PACKET_FORMAT, PACKET_FORMAT_HT_MF))	// mixed format, 11n
		{
			VSAanylisisFormat = 1;
		}
		else if ( strstr ( l_11ACPowerModeDutParam.PACKET_FORMAT, PACKET_FORMAT_HT_GF))	// greenfield format, 11n
		{
			VSAanylisisFormat = 2;
		}
		else if ( strstr ( l_11ACPowerModeDutParam.PACKET_FORMAT, PACKET_FORMAT_VHT))	// 11ac
		{
			VSAanylisisFormat = 4;
		}
		else if ( strstr( l_11ACPowerModeDutParam.PACKET_FORMAT, PACKET_FORMAT_NON_HT))	// 11ag
		{
			VSAanylisisFormat = 3;
		}
		else
		{
			VSAanylisisFormat = 0;
		}

		double chainGain = 0;
		int iChainGain = 0;
		chainGain = 10*log10((double)(l_11ACPowerModeDutParam.ATTEN1 + l_11ACPowerModeDutParam.ATTEN2 + l_11ACPowerModeDutParam.ATTEN3 + l_11ACPowerModeDutParam.ATTEN4));
		//Round off to thousandth
		iChainGain = (int)((chainGain * 1000) + .5);
		chainGain = (double)iChainGain / 1000;

	   if((current_test_mode == E_CURR_TEST_TX)&&(current_operation_mode == E_OPERATION_START))
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
		  
		   //////////////////////////////////////////////////////////////////////////
		    // This function call will return a vDUT_ERR: vDUT_ERR_NO_DUT_REGISTER_FUNCTION_FOUND
		   //  When it is called on a Dut Control that has not registered a TX_SET_CONTINUOUS function call.
		   //////////////////////////////////////////////////////////////////////////
		   //err = vDUT_Run(g_WiFi_11ac_Dut, "TX_SET_CONTINUOUS");
		   //if ( ERR_OK!=err )
		   //{	// Check vDut return "ERROR_MESSAGE" or not, if "Yes", must handle it.
		   //	   err = ::vDUT_GetStringReturn(g_WiFi_11ac_Dut, "ERROR_MESSAGE", vErrorMsg, MAX_BUFFER_SIZE);
		   //	   if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
		   //	   {
		   //		   err = -1;	// set err to -1, means "Error".
		   //		   LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vErrorMsg);
		   //		   throw logMessage;
		   //	   }
			  // else	// Just return normal error message in this case
			  // {
				 //  LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(TX_SET_CONTINUOUS) return error.\n");
				 //  throw logMessage;
			  // }
		   //}
		   //else
		   //{  
			  // LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(TX_SET_CONTINUOUS) return OK.\n");
		   //}
		   //////////////////////////////////////////////////////////////////////////

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
	   }	
#pragma endregion

#pragma region STOP DUT TX CURRENT test
	   else if((current_test_mode == E_CURR_TEST_TX) &&(current_operation_mode == E_OPERATION_STOP))
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
		

#pragma endregion
			
#pragma region Start RX CURRENT test

	   else if((current_test_mode == E_CURR_TEST_RX)
		   &&(current_operation_mode == E_OPERATION_START))
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

			err = vDUT_Run(g_WiFi_11ac_Dut, "RX_SET_ANTENNA");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_SET_ANTENNA) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_SET_ANTENNA) return OK.\n");
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

			err = vDUT_Run(g_WiFi_11ac_Dut, "RX_SET_BW");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_SET_BW) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_SET_BW) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_11ac_Dut, "RX_SET_DATA_RATE");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_SET_DATA_RATE) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_SET_DATA_RATE) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_11ac_Dut, "RX_SET_FILTER");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_SET_FILTER) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_SET_FILTER) return OK.\n");
			}
#pragma region Setup LP Tester
			/*----------------------------*
			* Disable VSG output signal  *
			*----------------------------*/
			// make sure no signal coming out VSG
			err = ::LP_EnableVsgRF(0);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to turn off VSG.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
			}

			/*---------------------*
			* Load waveform Files *
			*---------------------*/
			char   modFile[MAX_BUFFER_SIZE];

			if ( strlen(l_11ACPowerModeDutParam.WAVEFORM_NAME))	// Use the waveform customer defines
			{
				sprintf_s(modFile, MAX_BUFFER_SIZE, "%s/%s",g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PATH, l_11ACPowerModeDutParam.WAVEFORM_NAME);

				// Wave file checking
				FILE *waveFile;
				fopen_s(&waveFile, modFile, "r");
				if (!waveFile)  // No such file exist
				{
					err = -1;
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, 
						"[WiFi_11AC] Wrong waveform file name: %s. Please check input paramter!\n",l_11ACPowerModeDutParam.WAVEFORM_NAME);
					throw logMessage;
					
				}
				else
				{
					strcpy_s(l_11ACPowerModeDutReturn.WAVEFORM_NAME,MAX_BUFFER_SIZE, modFile);
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Customer defines waveform file name: %s!\n",
						l_11ACPowerModeDutReturn.WAVEFORM_NAME,MAX_BUFFER_SIZE);

					fclose(waveFile);
					waveFile = NULL;
				}

			}
			else		// Use default waveform name
			{
				err = GetDefaultWaveformFileName(g_WiFi11ACGlobalSettingParam.PER_WAVEFORM_PATH,
										"iqvsg", //only iqvsg file support in daytona. might need more robust way if we allow IQ2010 etc to support 11ac signal
										wifiMode,
										l_11ACPowerModeDutParam.NUM_STREAM_11AC,
										cbw,
										l_11ACPowerModeDutParam.DATA_RATE,
										l_11ACPowerModeDutParam.PREAMBLE,
										l_11ACPowerModeDutParam.PACKET_FORMAT,
										l_11ACPowerModeDutParam.GUARD_INTERVAL,
										modFile, 
										MAX_BUFFER_SIZE);
				strcpy_s(l_11ACPowerModeDutReturn.WAVEFORM_NAME,MAX_BUFFER_SIZE, modFile);
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
						"[WiFi_11AC] Fail to get waveform file name: %s, GetDefaultWaveformFileName() return error.\n",l_11ACPowerModeDutReturn.WAVEFORM_NAME);

					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION,
						"[WiFi_11AC] GetDefaultWaveformFileName() return OK. Waveform file Name: %s\n",l_11ACPowerModeDutReturn.WAVEFORM_NAME);

				}
			}
			

			// Load the whole MOD file for continuous transmit
			err = ::LP_SetVsgModulation( modFile );
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] LP_SetVsgModulation( %s ) return error.\n", modFile);
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_SetVsgModulation( %s ) return OK.\n", modFile);
			}

		/*-------------------*
		* Setup IQTester VSG *
		*--------------------*/
		// Since the limitation, we assume that all path loss value are very close.

		// check VSG output power limit first
		if( wifiMode==WIFI_11B )
		{
			MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11B;
		}
		else if( wifiMode==WIFI_11AG )
		{
			MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11G; 
		}
		else if ( strstr (l_11ACPowerModeDutParam.PACKET_FORMAT, PACKET_FORMAT_VHT)) 
		{
			MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11AC; 
		}
		else	// 802.11N
		{
			MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11N; 
		}

		//else if ( strstr (l_11ACPowerModeDutParam.PACKET_FORMAT, "11N"))
		//{
		//	MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11N; 
		//}
		//else	// 802.11ac
		//{
		//	MaxVSGPowerlimit = g_WiFi11ACGlobalSettingParam.VSG_MAX_POWER_11AC; 
		//}

		double VSG_POWER = l_11ACPowerModeDutParam.POWER_DBM+cableLossDb;
		if ( VSG_POWER>MaxVSGPowerlimit )
		{
			//ERR_VSG_POWER_EXCEED_LIMIT
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, MaxVSGPowerlimit);
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Set VSG port by g_WiFi11ACGlobalSettingParam.VSG_PORT=[%d].\n", g_WiFi11ACGlobalSettingParam.VSG_PORT);
		err = ::LP_SetVsg(l_11ACPowerModeDutParam.CH_FREQ_MHZ*1e6, VSG_POWER, g_WiFi11ACGlobalSettingParam.VSG_PORT);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to setup VSG, LP_SetVsg() return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] LP_SetVsg() return OK.\n");
		}

			/*---------------*
			*  Turn on VSG  *
			*---------------*/
			err = ::LP_EnableVsgRF(1);
			if ( ERR_OK!=err )
			{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Fail to turn on VSG, LP_EnableVsgRF(1) return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Turn on LP_EnableVsgRF(1) return OK.\n");
			}

			/*-----------------------------*
			* Send one packet for warm up *
			*-----------------------------*/           
			err = ::LP_SetFrameCnt(1);
			if ( ERR_OK!=err )
			{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] VSG send out packet LP_SetFrameCnt(1) failed.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] VSG send out one packet for warm up LP_SetFrameCnt(1) return OK.\n");
			}
			// After warm up, no signal coming out from VSG
#pragma endregion

#pragma region Start RX receiving
			/*---------------------*
			*  Do RX pre_RX frst  *
			*---------------------*/
			err = vDUT_Run(g_WiFi_11ac_Dut, "RX_PRE_RX");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_PRE_RX) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_PRE_RX) return OK.\n");
			}

			/*-------------------------*
			* clear RX packet counter *
			*-------------------------*/
			err = vDUT_Run(g_WiFi_11ac_Dut, "RX_CLEAR_STATS");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_CLEAR_STATS) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_CLEAR_STATS) return OK.\n");
			}

			/*----------*
			* RX Start *
			*----------*/		
			err = vDUT_Run(g_WiFi_11ac_Dut, "RX_START");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_START) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = true;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_START) return OK.\n");
			}
			
			err = ::LP_SetFrameCnt(0);  // continious transmit
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Set VSG continious transmit LP_SetFrameCnt(0) failed.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Set VSG continious transmit LP_SetFrameCnt(0) return OK.\n");
			}
		}
#pragma endregion
	

#pragma region Stop RX CURRENT test
		/*-----------*
		*  Rx Stop  *
		*-----------*/
		else if((current_test_mode == E_CURR_TEST_RX)&&(current_operation_mode == E_OPERATION_STOP))
		{
			err = ::LP_SetFrameCnt(1);		//stop VSG
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Set VSG send out one packet LP_SetFrameCnt(1) failed.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] Set VSG send out one packet LP_SetFrameCnt(1) return OK.\n");
			}

			err = vDUT_Run(g_WiFi_11ac_Dut, "RX_STOP");		
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
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] vDUT_Run(RX_STOP) return error.\n");
					throw logMessage;
				}
			}
			else
			{
				g_vDutTxActived = false;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] vDUT_Run(RX_STOP) return OK.\n");
			}
		}
#pragma endregion			

#pragma region START SLEEP CURRENT test
			//John: 8688 don't support SLEEP mode ( at least 8686 dont support, maybe supported by 8688?
			//anyway, just return an error code for now
		 else if(current_test_mode == E_CURR_TEST_SLEEP)			
		 {
			 err = -1;
			 LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] SLEEP MODE not implemented for this chip.\n");
			 throw logMessage;
		 }
#pragma endregion	

#pragma region START IDLE CURRENT test
		 else if(current_test_mode == E_CURR_TEST_IDLE)
		 {
			 //for IDLE test, do nothing

		 }
#pragma endregion	
	   /*-----------------------*
		*  Return Test Results  *
		*-----------------------*/
		if ( ERR_OK==err )
		{
			sprintf_s(l_11ACPowerModeDutReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults( l_11ACPowerModeDutReturnMap);
		}
		else
		{
			// do nothing
		}

	}
	catch(char *msg)
	{
		ReturnErrorMessage(l_11ACPowerModeDutReturn.ERROR_MESSAGE, msg);
	}
	catch(...)
	{
		ReturnErrorMessage(l_11ACPowerModeDutReturn.ERROR_MESSAGE, "[WiFi_11AC] Unknown Error!\n");
	}

	
	return err;
}

int Initialize11ACPowerModeDutContainers(void)
{
	/*------------------*
	* Input Parameters  *
	*------------------*/
	l_11ACPowerModeDutParamMap.clear();

	WIFI_SETTING_STRUCT setting;

	strcpy_s(l_11ACPowerModeDutParam.DUT_POWER_MODE, MAX_BUFFER_SIZE, "TX");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACPowerModeDutParam.DUT_POWER_MODE))    // Type_Checking
	{
		setting.value       = (void*)l_11ACPowerModeDutParam.DUT_POWER_MODE;
		setting.unit        = "";
		setting.helpText    = "DUT POWER MODE: TX , RX, SLEEP or IDLE";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DUT_POWER_MODE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_11ACPowerModeDutParam.DUT_OPERATION, MAX_BUFFER_SIZE, "START");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACPowerModeDutParam.DUT_OPERATION))    // Type_Checking
	{
		setting.value       = (void*)l_11ACPowerModeDutParam.DUT_OPERATION;
		setting.unit        = "";
		setting.helpText    = "DUT OPERATION: START or STOP";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DUT_OPERATION", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACPowerModeDutParam.WAVEFORM_NAME[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACPowerModeDutParam.WAVEFORM_NAME))    // Type_Checking
	{
		setting.value       = (void*)l_11ACPowerModeDutParam.WAVEFORM_NAME;
		setting.unit        = "";
		setting.helpText    = "File name of waveform to verify PER. if no value, use default waveform name defined in the program.";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("WAVEFORM_NAME", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	strcpy_s(l_11ACPowerModeDutParam.BSS_BANDWIDTH, MAX_BUFFER_SIZE, "BW-80");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACPowerModeDutParam.BSS_BANDWIDTH))    // Type_Checking
	{
		setting.value       = (void*)l_11ACPowerModeDutParam.BSS_BANDWIDTH;
		setting.unit        = "MHz";
		setting.helpText    = "BSS bandwidth\r\nValid options: BW-20, BW-40, BW-80, BW-80_80 or BW-160";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_BANDWIDTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	strcpy_s(l_11ACPowerModeDutParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, "0");
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_11ACPowerModeDutParam.CH_BANDWIDTH))    // Type_Checking
    {
        setting.value       = (void*)l_11ACPowerModeDutParam.CH_BANDWIDTH;
        setting.unit        = "MHz";
       setting.helpText    = "Channel bandwidth\r\nValid options:0, CBW-20, CBW-40, CBW-80, CBW-80_80 or CBW-160.\r\nFor 802.11ac, if it is zero,CH_BANDWIDTH equals as BSS_BANDWIDTH. For 802.11/a/b/g/n, it must always have value.";
        l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_BANDWIDTH", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_11ACPowerModeDutParam.DATA_RATE, MAX_BUFFER_SIZE, "MCS0");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACPowerModeDutParam.DATA_RATE))    // Type_Checking
	{
		setting.value       = (void*)l_11ACPowerModeDutParam.DATA_RATE;
		setting.unit        = "";
		setting.helpText    = "Data rate names:\r\nDSSS-1,DSSS-2,CCK-5_5,CCK-11\r\nOFDM-6,OFDM-9,OFDM-12,OFDM-18,OFDM-24,OFDM-36,OFDM-48,OFDM-54\r\nMCS0, MCS15, etc.";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("DATA_RATE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_11ACPowerModeDutParam.PREAMBLE, MAX_BUFFER_SIZE, "SHORT");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACPowerModeDutParam.PREAMBLE))    // Type_Checking
	{
		setting.value       = (void*)l_11ACPowerModeDutParam.PREAMBLE;
		setting.unit        = "";
		setting.helpText    = "The preamble type of 11B(only), can be SHORT or LONG, Default=SHORT.";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PREAMBLE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	strcpy_s(l_11ACPowerModeDutParam.PACKET_FORMAT, MAX_BUFFER_SIZE, PACKET_FORMAT_VHT);
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACPowerModeDutParam.PACKET_FORMAT))    // Type_Checking
	{
		setting.value       = (void*)l_11ACPowerModeDutParam.PACKET_FORMAT;
		setting.unit        = "";
        setting.helpText    = "The packet format, VHT, HT_MF,HT_GF and NON_HT as defined in standard. Default=VHT.";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("PACKET_FORMAT", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	// "STANDARD"
	strcpy_s(l_11ACPowerModeDutParam.STANDARD, MAX_BUFFER_SIZE, STANDARD_802_11_AC); 
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACPowerModeDutParam.STANDARD))    // Type_Checking
	{
		setting.value       = (void*)l_11ACPowerModeDutParam.STANDARD;
		setting.unit        = "";
		setting.helpText    = "Used for signal analysis option or to discriminating the same data rate or package format from different standards, taking value from 802.11ac, 802.11n, 802.11ag, 802.11b. Default = 802.11ac ";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("STANDARD", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	
	strcpy_s(l_11ACPowerModeDutParam.GUARD_INTERVAL, MAX_BUFFER_SIZE, "LONG");
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACPowerModeDutParam.GUARD_INTERVAL))    // Type_Checking
	{
		setting.value       = (void*)l_11ACPowerModeDutParam.GUARD_INTERVAL;
		setting.unit        = "";
		setting.helpText    = "Packet Guard Interval, Long or Short, default is Long";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("GUARD_INTERVAL", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY = 5520;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 20,40,80 or 160MHz bandwidth, it provides the BSS center frequency. For 80+80MHz bandwidth, it proivdes the center freuqency of the primary segment";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_PRIMARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACPowerModeDutParam.BSS_FREQ_MHZ_SECONDARY = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACPowerModeDutParam.BSS_FREQ_MHZ_SECONDARY))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACPowerModeDutParam.BSS_FREQ_MHZ_SECONDARY;
		setting.unit        = "MHz";
		setting.helpText    = "For 80+80MHz bandwidth, it proivdes the center freuqency of the sencodary segment. For 20,40,80 or 160MHz bandwidth, it is not undefined";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("BSS_FREQ_MHZ_SECONDARY", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz;
		setting.unit        = "MHz";
		setting.helpText    = "The center frequency (MHz) for primary 20 MHZ channel, priority is lower than \"CH_FREQ_MHZ\".";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ_PRIMARY_20MHz", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACPowerModeDutParam.CH_FREQ_MHZ = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACPowerModeDutParam.CH_FREQ_MHZ))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACPowerModeDutParam.CH_FREQ_MHZ;
		setting.unit        = "MHz";
		setting.helpText    = "It is the center frequency (MHz) for channel. If it is zero,\"CH_FREQ_MHZ_PRIMARY_20MHz\" will be used for 802.11ac. \r\nFor 802.11/a/b/g/n, it must alway have value.";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CH_FREQ_MHZ", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}
	

	l_11ACPowerModeDutParam.NUM_STREAM_11AC = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACPowerModeDutParam.NUM_STREAM_11AC))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACPowerModeDutParam.NUM_STREAM_11AC;
		setting.unit        = "";
		setting.helpText    = "Number of spatial streams based on 11AC spec";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("NUM_STREAM_11AC", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACPowerModeDutParam.POWER_DBM = 15.0;
	setting.type = WIFI_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_11ACPowerModeDutParam.POWER_DBM))    // Type_Checking
	{
		setting.value = (void*)&l_11ACPowerModeDutParam.POWER_DBM;
		setting.unit        = "dBm";
		setting.helpText    = "Expected power level at DUT antenna port";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("POWER_DBM", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}


	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_11ACPowerModeDutParam.CABLE_LOSS_DB[i] = 0.0;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_11ACPowerModeDutParam.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value       = (void*)&l_11ACPowerModeDutParam.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_11ACPowerModeDutParam.CONTINOUS_TX_MODE = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACPowerModeDutParam.CONTINOUS_TX_MODE))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACPowerModeDutParam.CONTINOUS_TX_MODE;
		setting.unit        = "";
		setting.helpText    = "Select to transmit continuous signal or not(0: with gap, 1: without gap).";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("CONTINOUS_TX_MODE", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACPowerModeDutParam.ATTEN1 = 1;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACPowerModeDutParam.ATTEN1))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACPowerModeDutParam.ATTEN1;
		setting.unit        = "";
		setting.helpText    = "DUT attenuator path 1 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ATTEN1", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACPowerModeDutParam.ATTEN2 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACPowerModeDutParam.ATTEN2))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACPowerModeDutParam.ATTEN2;
		setting.unit        = "";
		setting.helpText    = "DUT attenuator path 2 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ATTEN2", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACPowerModeDutParam.ATTEN3 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACPowerModeDutParam.ATTEN3))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACPowerModeDutParam.ATTEN3;
		setting.unit        = "";
		setting.helpText    = "DUT attenuator path 3 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ATTEN3", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

	l_11ACPowerModeDutParam.ATTEN4 = 0;
	setting.type = WIFI_SETTING_TYPE_INTEGER;
	if (sizeof(int)==sizeof(l_11ACPowerModeDutParam.ATTEN4))    // Type_Checking
	{
		setting.value       = (void*)&l_11ACPowerModeDutParam.ATTEN4;
		setting.unit        = "";
		setting.helpText    = "DUT attenuator path 4 ON/OFF\r\nValid options are 1(ON) and 0(OFF)";
		l_11ACPowerModeDutParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("ATTEN4", setting) );
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
	l_11ACPowerModeDutReturnMap.clear();

	   

	for (int i=0;i<MAX_DATA_STREAM;i++)
	{
		l_11ACPowerModeDutReturn.CABLE_LOSS_DB[i] = NA_DOUBLE;
		setting.type = WIFI_SETTING_TYPE_DOUBLE;
		if (sizeof(double)==sizeof(l_11ACPowerModeDutReturn.CABLE_LOSS_DB[i]))    // Type_Checking
		{
			setting.value = (void*)&l_11ACPowerModeDutReturn.CABLE_LOSS_DB[i];
			char tempStr[MAX_BUFFER_SIZE];
			sprintf_s(tempStr, "CABLE_LOSS_DB_%d", i+1);
			setting.unit        = "dB";
			setting.helpText    = "Cable loss from the DUT antenna port to tester";
			l_11ACPowerModeDutReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>(tempStr, setting) );
		}
		else    
		{
			printf("Parameter Type Error!\n");
			exit(1);
		}
	}

	l_11ACPowerModeDutReturn.ERROR_MESSAGE[0] = '\0';
	setting.type = WIFI_SETTING_TYPE_STRING;
	if (MAX_BUFFER_SIZE==sizeof(l_11ACPowerModeDutReturn.ERROR_MESSAGE))    // Type_Checking
	{
		setting.value       = (void*)l_11ACPowerModeDutReturn.ERROR_MESSAGE;
		setting.unit        = "";
		setting.helpText    = "Error message occurred";
		l_11ACPowerModeDutReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
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
int Check11ACPowerModeParameters( int *bssBW, int * cbw,int *bssPchannel,int *bssSchannel,int *wifiMode, int *wifiStreamNum, double *samplingTimeUs, double *cableLossDb, double *peakToAvgRatio, char* errorMsg )
{
	int    err = ERR_OK;
	int    dummyInt = 0;
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
         // check common input parameters
		err = CheckCommonParameters_WiFi_11ac(l_11ACPowerModeDutParamMap, 
			                                  l_11ACPowerModeDutParamMap, 
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
//		if ( 0!=strcmp(l_11ACPowerModeDutParam.PREAMBLE, "SHORT") && 0!=strcmp(l_11ACPowerModeDutParam.PREAMBLE, "LONG") )
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown PREAMBLE, WiFi preamble %s not supported.\n", l_11ACPowerModeDutParam.PREAMBLE);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0!=strcmp(l_11ACPowerModeDutParam.GUARD_INTERVAL, "SHORT") && 0!=strcmp(l_11ACPowerModeDutParam.GUARD_INTERVAL, "LONG") )
//		{
//		err = -1;
//		LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown GUARD_INTERVAL, WiFi guard interval %s not supported.\n", l_11ACPowerModeDutParam.GUARD_INTERVAL);
//		throw logMessage;
//		}
//		else
//		{
//		//do nothing
//		}
//
//		if (( 0> l_11ACPowerModeDutParam.NUM_STREAM_11AC) || ( l_11ACPowerModeDutParam.NUM_STREAM_11AC >8))
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
//		if ( (0!=strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "11AC_MF_HT")) && (0!=strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "11AC_GF_HT")) &&
//			(0!=strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "11N_MF_HT")) && (0!=strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "11N_GF_HT")) &&
//			(0!=strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "11AC_VHT")) &&( 0!=strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "11AC_NON_HT"))
//			&&( 0!=strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "LEGACY_NON_HT")))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown PACKET_FORMAT, WiFi 11ac packet format %s not supported.\n", l_11ACPowerModeDutParam.PACKET_FORMAT);
//			throw logMessage;
//		}
//		else
//		{
//			//do nothing
//		}
//
//		if ( 0 == strcmp( l_11ACPowerModeDutParam.PACKET_FORMAT, "11AC_NON_HT") &&
//			! strstr ( l_11ACPowerModeDutParam.DATA_RATE, "OFDM"))
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
//		err = TM_WiFiConvertDataRateNameToIndex(l_11ACPowerModeDutParam.DATA_RATE, &dummyInt);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown DATA_RATE, convert WiFi datarate %s to index failed.\n", l_11ACPowerModeDutParam.DATA_RATE);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( 0 == strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "11AC_VHT"))  // Data rate: MCS0 ~MCS9
//		{
//			if (( 14 <= dummyInt ) && (dummyInt <= 23))
//			{
//				// Data rate is right
//				//do nothing
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] PACKET_FORMAT and DATA_RATE don't match! The data rates of VHT_11AC must be MCS0 ~ MCS9, doesn't support %s!\n", &l_11ACPowerModeDutParam.DATA_RATE);
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
//		if ( !strstr (l_11ACPowerModeDutParam.PACKET_FORMAT, "11AC"))    // Legacy signal, CH_BANDWIDTH and CH_FREQ_MHZ must have values
//		{
//			if (( l_11ACPowerModeDutParam.CH_BANDWIDTH <= 0) || ( l_11ACPowerModeDutParam.CH_FREQ_MHZ <= 0))
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
//		if ( 0!=strcmp(l_11ACPowerModeDutParam.BSS_BANDWIDTH, "BW-20") && 0!=strcmp(l_11ACPowerModeDutParam.BSS_BANDWIDTH, "BW-40") && 
//			0!=strcmp(l_11ACPowerModeDutParam.BSS_BANDWIDTH, "BW-80") && 0!=strcmp(l_11ACPowerModeDutParam.BSS_BANDWIDTH, "BW-160") &&
//			0!=strcmp(l_11ACPowerModeDutParam.BSS_BANDWIDTH, "BW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACPowerModeDutParam.BSS_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_11ACPowerModeDutParam.BSS_BANDWIDTH, "BW-20"))
//			{
//				*bssBW = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACPowerModeDutParam.BSS_BANDWIDTH, "BW-40"))
//			{
//				*bssBW = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACPowerModeDutParam.BSS_BANDWIDTH, "BW-80"))
//			{
//				*bssBW = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACPowerModeDutParam.BSS_BANDWIDTH, "BW-160"))
//			{
//				*bssBW = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACPowerModeDutParam.BSS_BANDWIDTH, "BW-80_80"))
//			{
//				*bssBW = BW_80_80MHZ;
//
//				if ( l_11ACPowerModeDutParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
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
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACPowerModeDutParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//
//		}
//
//		if ( 0 != strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "0") && 
//			0!=strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "CBW-20") && 0!=strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "CBW-40") && 
//			0!=strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "CBW-80") && 0!=strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "CBW-160") &&
//			0!=strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "CBW-80_80"))
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown CBW BANDWIDTH, WiFi 11ac CBW bandwidth %s not supported.\n", l_11ACPowerModeDutParam.CH_BANDWIDTH);
//			throw logMessage;
//		}
//		else
//		{
//			if ( 0 == strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "0"))
//			{
//				sprintf_s(l_11ACPowerModeDutParam.CH_BANDWIDTH,MAX_BUFFER_SIZE,"C%s",l_11ACPowerModeDutParam.BSS_BANDWIDTH);
//				*cbw = *bssBW;
//			}
//			else if ( 0 == strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "CBW-20"))
//			{
//				*cbw = BW_20MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "CBW-40"))
//			{
//				*cbw = BW_40MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "CBW-80"))
//			{
//				*cbw = BW_80MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "CBW-160"))
//			{
//				*cbw = BW_160MHZ;
//			}
//			else if ( 0 == strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, "CBW-80_80"))
//			{
//				*cbw = BW_80_80MHZ;
//			}
//			else
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown BSS BANDWIDTH, WiFi 11ac BSS bandwidth %s not supported.\n", l_11ACPowerModeDutParam.BSS_BANDWIDTH);
//				throw logMessage;
//			}
//		}
//
//		if ( strstr ( l_11ACPowerModeDutParam.PREAMBLE,"11AC"))  //802.11ac, cbw can't be larger than bssBW
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
//		if ( strstr( l_11ACPowerModeDutParam.PACKET_FORMAT, "11N"))  //802.11n, BW= 20,40MHz
//		{
//			if ( *cbw != BW_20MHZ && *cbw != BW_40MHZ)
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//					"[WiFi_11AC]  11N bandwidth should be 20MHz or 40Mhz. Please check input parameters!\n" );
//				throw logMessage;
//			}
//			else
//			{
//				// do nothing
//			}
//		}
//		else if ( 0 == strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "LEGACY_NON_HT"))  // 11B and 11ag
//		{
//			if ( *cbw != BW_20MHZ)
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//					"[WiFi_11AC]  11B and 11AG bandwidth should be 20MHz. Please check input parameters!\n" );
//				throw logMessage;
//			}
//			else
//			{
//				// do nothing
//			}
//		}
//		else		// 802.11 ac
//		{
//			if ( 0 == strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "11AC_VHT"))
//			{
//				//do nothing
//			}
//			else if ( 0 == strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "11AC_MF_HT") ||
//				0 == strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, "11AC_GF_HT"))
//			{
//				if ( *cbw != BW_20MHZ && *cbw != BW_40MHZ)
//				{
//					err = -1;
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//						"[WiFi_11AC]  11AC_MF(GF)_HT bandwidth should be 20MHz or 40Mhz. Please check input parameters!\n" );
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
//						"[WiFi_11AC]  11AC_NON_HT bandwidth can't be 20MHz. Please check input parameters!\n" );
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
//		if (l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY <= 0)
//		{
//			err = -1;
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//				"[WiFi_11AC]  \"BSS_FREQ_MHZ_PRIMARY\" must have value! Please check input parameters!\n" );
//			throw logMessage;
//
//		}
//		else
//		{
//			// do nothing
//		}
//
//		err = TM_WiFiConvertFrequencyToChannel(l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY, bssPchannel);      
//		if ( ERR_OK!=err )
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY);
//			throw logMessage;
//		}
//		else
//		{
//			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi_11AC] TM_WiFiConvertFrequencyToChannel() return OK.\n");
//		}
//
//		if ( *bssBW == BW_80_80MHZ)  // Need BSS_FREQ_MHZ_SECONDARY
//		{
//			if ( l_11ACPowerModeDutParam.BSS_FREQ_MHZ_SECONDARY <= 0 )
//			{
//				err = -1;
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR,
//					"[WiFi_11AC]  BSS_BANDWIDTH = BW_80_80MHZ,\"BSS_FREQ_MHZ_SECONDARY\" must have value!\n" );
//				throw logMessage;
//			}
//			else
//			{
//				// do nothing
//			}
//
//			err = TM_WiFiConvertFrequencyToChannel(l_11ACPowerModeDutParam.BSS_FREQ_MHZ_SECONDARY, bssSchannel);      
//			if ( ERR_OK!=err )
//			{
//				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Unknown FREQ_MHZ, convert WiFi frequency %d to channel failed.\n", l_11ACPowerModeDutParam.BSS_FREQ_MHZ_SECONDARY);
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
//		if ( strstr (l_11ACPowerModeDutParam.PACKET_FORMAT, "11AC"))
//		{
//
//			if ( 0 == l_11ACPowerModeDutParam.CH_FREQ_MHZ)  
//			{
//				//no CH_FREQ_MHZ, no CH_FREQ_MHZ_PRIMARY_20MHz
//				// Use default values
//				if ( 0 == l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz) 
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_WARNING, "[WiFi_11AC] Don't have input parameters \"CH_FREQ_MHZ\" or \"CH_FREQ_MHZ_PRIMARY_20\", using BBS center freuqency as default!");
//					l_11ACPowerModeDutParam.CH_FREQ_MHZ = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY;
//
//					// all  use lower frequency for channel list
//					switch (*bssBW)
//					{
//					case BW_20MHZ:
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_40MHZ:
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 10;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//
//					case BW_80MHZ:
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 20;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY;
//						break;
//
//					case BW_160MHZ:			
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//
//					case BW_80_80MHZ:				
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 70;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 60;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 40;
//						break;
//
//					default:
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					}
//
//				}
//				else  //no CH_FREQ_MHZ, have CH_FREQ_MHZ_PRIMARY_20MHz. Use input CH_FREQ_MHZ_PRIMARY_20MHz to calculate
//				{
//					err = GetChannelList(*bssBW, l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY,l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//						&l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//					if ( err != ERR_OK) // Wrong channel list
//					{
//						LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_11ACPowerModeDutParam.PACKET_FORMAT);
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
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//					else if (*cbw == BW_20MHZ)
//					{
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ = l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz;
//					}
//					else if (*cbw == BW_40MHZ)
//					{
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ = l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz;
//					}
//					else if (*cbw == BW_80MHZ)
//					{
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ = l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz;
//					}
//					else
//					{
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY;
//					}
//				}
//				else // cbw = BW_80_80MHZ, use BSS primary and secondary
//				{
//					l_11ACPowerModeDutParam.CH_FREQ_MHZ = NA_INTEGER;
//					// do nothing
//				}
//			}
//			else  // CH_FREQ_MHZ: non-zero 
//			{
//				//Check if input CH_FREQ_MHZ is correct
//				err = CheckChannelFreq(*bssBW, *cbw,
//					l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY,l_11ACPowerModeDutParam.CH_FREQ_MHZ);
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
//				if ( 0 != l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz)
//				{
//					err = CheckChPrimary20(*bssBW, *cbw,
//						l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY,l_11ACPowerModeDutParam.CH_FREQ_MHZ,l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz);
//					if ( err == ERR_OK)  //input CH_FREQ_MHZ_PRIMARY_20MHz is correct
//					{
//						err = GetChannelList(*bssBW, l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY,l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//							&l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//						if ( err != ERR_OK) // Wrong channel list
//						{
//							LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Can't get channel list. Please check input paramters.\n", l_11ACPowerModeDutParam.PACKET_FORMAT);
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
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//						l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//						break;
//					case BW_40MHZ:  //cbw = BW_20.40MHz
//						if (*cbw == BW_20MHZ)
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 10;
//
//						}
//						else  //wrong cbw
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80MHZ:  //cbw = BW_20,40,80MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_11ACPowerModeDutParam.CH_FREQ_MHZ == l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_11ACPowerModeDutParam.CH_FREQ_MHZ == l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  //wrong cbw
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz =  NA_INTEGER;
//
//						}
//						break;
//					case BW_160MHZ:  //cbw = BW_20,40,80,160MHz
//						if ( *cbw == BW_20MHZ)
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if (l_11ACPowerModeDutParam.CH_FREQ_MHZ == l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 60)
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_11ACPowerModeDutParam.CH_FREQ_MHZ == l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_11ACPowerModeDutParam.CH_FREQ_MHZ == l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else if ( l_11ACPowerModeDutParam.CH_FREQ_MHZ == l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY + 60)
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY + 50;
//							}
//							else  //wrong CH_FREQ_MHZ
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							if (l_11ACPowerModeDutParam.CH_FREQ_MHZ == l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 40)
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else if (l_11ACPowerModeDutParam.CH_FREQ_MHZ == l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY + 40)
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_160MHZ)
//						{
//							if (l_11ACPowerModeDutParam.CH_FREQ_MHZ == l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY)
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 70;
//							}
//							else // wrong CH_FREQ_MHZ
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else  //wring cbw
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					case BW_80_80MHZ:
//						if ( *cbw == BW_20MHZ)
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.CH_FREQ_MHZ;
//						}
//						else if ( *cbw == BW_40MHZ)
//						{
//							if ( l_11ACPowerModeDutParam.CH_FREQ_MHZ == l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 20)
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 30;
//							}
//							else if (l_11ACPowerModeDutParam.CH_FREQ_MHZ == l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY + 20)
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY + 10;
//							}
//							else  // wrong CH_FREQ_MHZ
//							{
//								l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//							}
//						}
//						else if ( *cbw == BW_80MHZ)
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else if ( *cbw == BW_80_80MHZ)
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY - 30;
//						}
//						else  // wrong CH_FREQ_MHz
//						{
//							l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						}
//						break;
//					default:
//						l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//						break;
//					}
//				}
//
//				err = GetChannelList(*bssBW, l_11ACPowerModeDutParam.BSS_FREQ_MHZ_PRIMARY,l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz,
//					&l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz,&l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz);
//				if ( err != ERR_OK)  //Get channel list wrong
//				{
//					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi_11AC] Wrong CH_FREQ_MHZ_PRIMARY_20MHz value. Please check input paramters.\n", l_11ACPowerModeDutParam.PACKET_FORMAT);
//					throw logMessage;
//				}
//				else  //Get channel list successfully
//				{
//				}
//			}	
//
//			//Check channel list. If all are "NA_INTEGER", return error
//			if (( l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz == NA_INTEGER ) && 
//				(l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz == NA_INTEGER ) &&
//				(l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER))
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
//			l_11ACPowerModeDutParam.CH_FREQ_MHZ_PRIMARY_20MHz = NA_INTEGER;
//			l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_40MHz = NA_INTEGER;
//			l_11ACPowerModeDutReturn.CH_FREQ_MHZ_PRIMARY_80MHz = NA_INTEGER;
//		}
//#pragma endregion
//
//		//VHT, cbw = BW_20MHZ, the stream number of MCS9 only can 3,6
//		if ( ( *cbw == BW_20MHZ) && ( dummyInt == 23) )
//		{
//			if ( (l_11ACPowerModeDutParam.NUM_STREAM_11AC != 3) &&
//				(l_11ACPowerModeDutParam.NUM_STREAM_11AC != 6) )
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
//		err = WiFi_11ac_TestMode(l_11ACPowerModeDutParam.DATA_RATE, cbw, wifiMode, wifiStreamNum, l_11ACPowerModeDutParam.PACKET_FORMAT);
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
//			l_11ACPowerModeDutParam.CH_FREQ_MHZ,
//			l_11ACPowerModeDutParam.ATTEN1,
//			l_11ACPowerModeDutParam.ATTEN2,
//			l_11ACPowerModeDutParam.ATTEN3,
//			l_11ACPowerModeDutParam.ATTEN4,
//			l_11ACPowerModeDutParam.CABLE_LOSS_DB,
//			l_11ACPowerModeDutReturn.CABLE_LOSS_DB,
//			cableLossDb,
//			TX_TABLE
//			);
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
//		//// Check sampling time 
//		//if (0==l_11ACPowerModeDutParam.SAMPLING_TIME_US)
//		//{
//			*samplingTimeUs = CheckSamplingTime(*wifiMode, l_11ACPowerModeDutParam.PREAMBLE, l_11ACPowerModeDutParam.DATA_RATE, l_11ACPowerModeDutParam.PACKET_FORMAT);
//		//}
//		//else	// SAMPLING_TIME_US != 0
//		//{
//		//	*samplingTimeUs = l_11ACPowerModeDutParam.SAMPLING_TIME_US;
//		//}

		  /*-------------------------------------*
		  *  set PAPR values for vsa setting     *
		  *--------------------------------------*/
		  if ( *wifiMode==WIFI_11B )  
			  *peakToAvgRatio = g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11B_11M;    // CCK        
		  else                        
			  *peakToAvgRatio = g_WiFi11ACGlobalSettingParam.IQ_P_TO_A_11AG_54M;   // OFDM 


		// Check Dut configuration changed or not
		if (  g_WiFi11ACGlobalSettingParam.DUT_KEEP_TRANSMIT==0	||		// means need to configure DUT everytime, thus set g_dutConfigChanged = true , always.
			l_11ACPowerModeDutParam.CH_FREQ_MHZ != g_RecordedParam.CH_FREQ_MHZ ||
			0!=strcmp(l_11ACPowerModeDutParam.CH_BANDWIDTH, g_RecordedParam.CH_BANDWIDTH) ||
			0!=strcmp(l_11ACPowerModeDutParam.DATA_RATE, g_RecordedParam.DATA_RATE) ||
			0!=strcmp(l_11ACPowerModeDutParam.PREAMBLE,  g_RecordedParam.PREAMBLE) ||
			0!=strcmp(l_11ACPowerModeDutParam.PACKET_FORMAT, g_RecordedParam.PACKET_FORMAT) ||
			0!=strcmp(l_11ACPowerModeDutParam.GUARD_INTERVAL, g_RecordedParam.GUARD_INTERVAL_11N) ||
			l_11ACPowerModeDutParam.CABLE_LOSS_DB[0]!=g_RecordedParam.CABLE_LOSS_DB[0] ||
			l_11ACPowerModeDutParam.CABLE_LOSS_DB[1]!=g_RecordedParam.CABLE_LOSS_DB[1] ||
			l_11ACPowerModeDutParam.CABLE_LOSS_DB[2]!=g_RecordedParam.CABLE_LOSS_DB[2] ||
			l_11ACPowerModeDutParam.CABLE_LOSS_DB[3]!=g_RecordedParam.CABLE_LOSS_DB[3] ||
			l_11ACPowerModeDutParam.POWER_DBM!=g_RecordedParam.POWER_DBM ||
			l_11ACPowerModeDutParam.ATTEN1!=g_RecordedParam.ANT1 ||
			l_11ACPowerModeDutParam.ATTEN2!=g_RecordedParam.ANT2 ||
			l_11ACPowerModeDutParam.ATTEN3!=g_RecordedParam.ANT3 ||
			l_11ACPowerModeDutParam.ATTEN4!=g_RecordedParam.ANT4)
		{
			g_dutConfigChanged = true;			
		}
		else
		{
			g_dutConfigChanged = false;			
		}

		// Save the current setup
		g_RecordedParam.ANT1					= l_11ACPowerModeDutParam.ATTEN1;
		g_RecordedParam.ANT2					= l_11ACPowerModeDutParam.ATTEN2;
		g_RecordedParam.ANT3					= l_11ACPowerModeDutParam.ATTEN3;
		g_RecordedParam.ANT4					= l_11ACPowerModeDutParam.ATTEN4;		
		g_RecordedParam.CABLE_LOSS_DB[0]		= l_11ACPowerModeDutParam.CABLE_LOSS_DB[0];
		g_RecordedParam.CABLE_LOSS_DB[1]		= l_11ACPowerModeDutParam.CABLE_LOSS_DB[1];
		g_RecordedParam.CABLE_LOSS_DB[2]		= l_11ACPowerModeDutParam.CABLE_LOSS_DB[2];
		g_RecordedParam.CABLE_LOSS_DB[3]		= l_11ACPowerModeDutParam.CABLE_LOSS_DB[3];	
		g_RecordedParam.CH_FREQ_MHZ				= l_11ACPowerModeDutParam.CH_FREQ_MHZ;
		g_RecordedParam.POWER_DBM				= l_11ACPowerModeDutParam.POWER_DBM;	

		sprintf_s(g_RecordedParam.CH_BANDWIDTH, MAX_BUFFER_SIZE, l_11ACPowerModeDutParam.CH_BANDWIDTH);
		sprintf_s(g_RecordedParam.DATA_RATE, MAX_BUFFER_SIZE, l_11ACPowerModeDutParam.DATA_RATE);
		sprintf_s(g_RecordedParam.PREAMBLE,  MAX_BUFFER_SIZE, l_11ACPowerModeDutParam.PREAMBLE);
		sprintf_s(g_RecordedParam.PACKET_FORMAT, MAX_BUFFER_SIZE, l_11ACPowerModeDutParam.PACKET_FORMAT);
		sprintf_s(g_RecordedParam.GUARD_INTERVAL_11N, MAX_BUFFER_SIZE, l_11ACPowerModeDutParam.GUARD_INTERVAL);


		sprintf_s(errorMsg, MAX_BUFFER_SIZE, "[WiFi_11AC] CheckTxPowerParameters() Confirmed.\n");
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

#include "stdafx.h"
#include "TestManager.h"
#include "BT_Test.h"
#include "BT_Test_Internal.h"
#include "IQmeasure.h"
#include "math.h"
#include "vDUT.h"

using namespace std;

// This global variable is declared in BT_Test_Internal.cpp
extern TM_ID      g_BT_Test_ID;
extern vDUT_ID    g_BT_Dut;
extern int		  g_Tester_Type;

// This global variable is declared in BT_Global_Setting.cpp
extern BT_GLOBAL_SETTING g_BTGlobalSettingParam;

#pragma region Define Input and Return structures (two containers and two structs)
// Input Parameter Container
map<string, BT_SETTING_STRUCT> l_rxVerifyLeParamMap;

// Return Value Container
map<string, BT_SETTING_STRUCT> l_rxVerifyLeReturnMap;

struct tagParam
{
    // Mandatory Parameters
	int       FREQ_MHZ;								/*! The center frequency (MHz). */				
	char      PACKET_TYPE[MAX_BUFFER_SIZE];			/*! The pack type to verify LE */
	char      WAVEFORM_NAME[MAX_BUFFER_SIZE];		/*! The waveform name to verify LE, LE_Multi (multi packets waveform) for DVT or LE (single packet waveform). use global setting now */
    int       NUM_PACKET_IN_WAVEFORM;			    /*! The number of packets in one waveform. */ 
    int		  PACKET_LENGTH;						/*! The number of payload length to verify LE. */
	double    RX_POWER_LEVEL;						/*! The output power to verify LE. No power control in LE, but chipset (BRCM) may want to add it in the future*/  
	double    RX_PACKET_NUMBER;						/*! The number of packet for PER test. */ 
	double    CABLE_LOSS_DB;						/*! The path loss of test system. */

} l_rxVerifyLeParam;

struct tagReturn
{
    // RX Power Level
    double     RX_POWER_LEVEL;     /*!< POWER_LEVEL dBm for the PER test. Format: POWER LEVEL, No power control in LE, but chipset (BRCM) may want to add it in the future */

    // PER Test Result 
    double	   GOOD_PACKETS;		   /*!< GOOD_PACKETS is the number of packets that reported from Dut */
	double     TOTAL_PACKETS;		   /*!< TOTAL_PACKETS is the total number of bits that transmitted by the testers. will be >= RX_PACKET_NUMBER  */
	double	   PER;                /*!< PER test result on specific data stream. */
	double     CABLE_LOSS_DB;						/*! The path loss of test system. */
    char       ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_rxVerifyLeReturn;
#pragma endregion

#ifndef WIN32
int initRXVerifyLEContainers = InitializeRXVerifyLEContainers();
#endif

int  ClearRxVerifyLEReturn(void)
{
	l_rxVerifyLeParamMap.clear();
	l_rxVerifyLeReturnMap.clear();
	return 0;
}

BT_TEST_API int BT_RX_Verify_LE(void)
{
    int    err = ERR_OK;

//    bool   analysisOK = false, captureOK  = false;
	bool   vDutActived = false;     
    int    dummyValue = 0;
	int	   packetLength= 0;
    int    waveformNumber = 0;  //number of waveform to be transmitted by the VSG
	double cableLossDb = 0;
	char   vDutErrorMsg[MAX_BUFFER_SIZE] = {'\0'};
	char   logMessage[MAX_BUFFER_SIZE] = {'\0'};

    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_rxVerifyLeReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_rxVerifyLeParamMap);
        return err;
    }
	else
	{
		// do nothing
	}

    /*-------------------------*
     * Respond to QUERY_RETURN *
     *-------------------------*/
    err = TM_GetIntegerParameter(g_BT_Test_ID, "QUERY_RETURN", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryReturn(l_rxVerifyLeReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}

	try
	{
		//IQapi can handle this now. 
		///*-----------------------------------------------------------*
  //      * Only IQ201X support BT LE, Zhiyong Huang on Jan 26, 2011 *
  //      *-----------------------------------------------------------*/

  //      if (g_BTGlobalSettingParam.IQ_TESTER_TYPE != TESTER_TYPE_2010) //Only IQ201X can support dirty packet in MPS mode
  //      {
  //          err = -1;
  //          LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT_LE] Tester is not IQ201X, it cannot support BT LE test.\n");
  //          throw logMessage;
  //      }
  //      else
  //      {
  //          //do nothing           
  //      }

	   /*-----------------------------------------------------------*
		* Both g_BT_Test_ID and g_BT_Dut need to be valid (>=0) *
		*-----------------------------------------------------------*/
		TM_ClearReturns(g_BT_Test_ID);
		if( g_BT_Test_ID<0 || g_BT_Dut<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] BT_Test_ID or BT_Dut not valid.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}

		/*--------------------------*
		 * Get mandatory parameters *
		 *--------------------------*/
		err = GetInputParameters(l_rxVerifyLeParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Input parameters are not completed.\n");
			throw logMessage;
		}

		// Check path loss (by ant and freq)
		if ( 0==l_rxVerifyLeParam.CABLE_LOSS_DB )
		{
			err = TM_GetPathLossAtFrequency(g_BT_Test_ID, l_rxVerifyLeParam.FREQ_MHZ, &l_rxVerifyLeParam.CABLE_LOSS_DB, 0, RX_TABLE);
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Failed to get CABLE_LOSS_DB from path loss table.\n", err);
				throw logMessage;
			}
		}
		else
		{
			// do nothing
		}

#pragma region Setup LP Tester
	   /*----------------------------*
		* Disable VSG output signal  *
		*----------------------------*/
		// make sure no signal is generated by the VSG
		err = ::LP_EnableVsgRF(0);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to turn off VSG, LP_EnableVsgRF(0) return error.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Turn off VSG LP_EnableVsgRF(0) return OK.\n");
		}

	   /*---------------------*
		* Load waveform Files *
		*---------------------*/
		char   modFile[MAX_BUFFER_SIZE] = {'\0'};
		//err = GetWaveformFileName("PER",		
		//						  "WAVEFORM_NAME", 
		//						  l_rxVerifyLeParam.PACKET_TYPE,
		//						  modFile, 
		//						  MAX_BUFFER_SIZE);
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to get waveform file name.\n");
		//	throw logMessage;
		//}

        //each test item specify its own mod file.
        err = AddWaveformPath( l_rxVerifyLeParam.WAVEFORM_NAME, modFile, MAX_BUFFER_SIZE);
        if ( ERR_OK!=err )
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to find the waveform in mod folder.\n");
            throw logMessage;
        }
	    
        // Load the whole MOD file for continuous transmit
        err = LP_SetVsgModulation( modFile);
        if ( ERR_OK!=err )
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] LP_SetVsgModulation( %s ) return error.\n", modFile);
            throw logMessage;
        }

        //1LE_10a and 1LE_10b is for internal used only. 1LE_20 is for Fuji to evaluate whether it match DP PER test. 

        if (strstr(l_rxVerifyLeParam.WAVEFORM_NAME , "1LE_20."))
        {
            l_rxVerifyLeParam.NUM_PACKET_IN_WAVEFORM = 20;					
        }
        else if(strstr(l_rxVerifyLeParam.WAVEFORM_NAME , "1LE_10a."))
        {
            l_rxVerifyLeParam.NUM_PACKET_IN_WAVEFORM = 10;
        }
        else if(strstr(l_rxVerifyLeParam.WAVEFORM_NAME , "1LE_10b."))
        {
            l_rxVerifyLeParam.NUM_PACKET_IN_WAVEFORM = 10;
        }
        else if(strstr(l_rxVerifyLeParam.WAVEFORM_NAME , "1LE_CRC_ERROR."))
        {
            l_rxVerifyLeParam.NUM_PACKET_IN_WAVEFORM = 2;
        }
        else
        {
            l_rxVerifyLeParam.NUM_PACKET_IN_WAVEFORM = 1;
        }

        g_BTGlobalSettingParam.MULTI_WAVE_FILE_CHANGED = true;
	   /*--------------------*
		* Setup IQTester VSG *
		*--------------------*/
		cableLossDb = l_rxVerifyLeParam.CABLE_LOSS_DB;

		double VSG_POWER = l_rxVerifyLeParam.RX_POWER_LEVEL + cableLossDb;
		if ( VSG_POWER>g_BTGlobalSettingParam.VSG_MAX_POWER_BT )
		{
			//ERR_VSG_POWER_EXCEED_LIMIT
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Required VSG power %.2f dBm exceed MAX power limit %.2f dBm!\n", VSG_POWER, g_BTGlobalSettingParam.VSG_MAX_POWER_BT);
			throw logMessage;
		}
		else
		{
			// do nothing
		}		

        //check whether it is a supported frequency/channel. 
        if (l_rxVerifyLeParam.FREQ_MHZ>= 2402 && l_rxVerifyLeParam.FREQ_MHZ <= 2480)
        {
            if( 0 == l_rxVerifyLeParam.FREQ_MHZ%2)
            {
                //right channel, do nothing
            }
            else
            {
                LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] %d MHz is not supported in BT LE, channel step is 2MHz\n", l_rxVerifyLeParam.FREQ_MHZ);
                throw logMessage; //do nothing
            }                
        }
        else
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] %d MHz is not supported in BT LE, out of range! \n", l_rxVerifyLeParam.FREQ_MHZ);
            throw logMessage;          
        }		
        
        LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Set VSG port by g_BTGlobalSettingParam.VSG_PORT=[%d].\n", g_BTGlobalSettingParam.VSG_PORT);
        err = LP_SetVsg(l_rxVerifyLeParam.FREQ_MHZ*1e6, VSG_POWER, g_BTGlobalSettingParam.VSG_PORT);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to setup VSG.\n");
			throw logMessage;
		}		


        /*--------------------------------------------------*
        * Calculate and set number of waveform for PER Test *
        *---------------------------------------------------*/ 
        waveformNumber = (int)ceil(l_rxVerifyLeParam.RX_PACKET_NUMBER/l_rxVerifyLeParam.NUM_PACKET_IN_WAVEFORM);
        if (!waveformNumber)
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to get the number of packet for PER test.\n");
            throw logMessage;

        }
        else
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] Get the number of packet GetPacketNumber() return OK.\n");
        }

    
       /*---------------*
        *  Turn on VSG  *
        *---------------*/
        err = LP_EnableVsgRF(1);
        if ( ERR_OK!=err )
        {
        	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to turn on VSG.\n");
        	throw logMessage;
        }	

        /*----------------------------------------------------------------*
        * Send one packet for warm up and make sure that no TX in the VSG *
        *-----------------------------------------------------------------*/           
        err = ::LP_SetFrameCnt(1);
        if ( ERR_OK!=err )
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] VSG send out packet LP_SetFrameCnt(1) failed.\n");
            throw logMessage;
        }
        else
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] LP_SetFrameCnt(1) return OK.\n");
        }
        // After warm up, no signal coming out from VSG

        //Zhiyong Debug Here to check whether delay help.
        //Sleep(5000);

#pragma endregion

#pragma region Configure DUT to Receive
		/*------------------------------------------*
		 * Configure DUT to Receive - PRBS9 Pattern *
		 *------------------------------------------*/
		// Set DUT RF frequency...
		// And clear vDUT parameters at the beginning.
		vDUT_ClearParameters(g_BT_Dut);

        vDUT_AddIntegerParameter(g_BT_Dut, "FREQ_MHZ",			l_rxVerifyLeParam.FREQ_MHZ);	

        //needed for DUT control to decide whether it is BER or PER test. 
        vDUT_AddStringParameter (g_BT_Dut, "PACKET_TYPE",	    l_rxVerifyLeParam.PACKET_TYPE); 
        
        // Check packet length, may not need at all. For future development/support purpose only. 		
        if (0==l_rxVerifyLeParam.PACKET_LENGTH)
        {
            GetPacketLength("PER", "PACKETS_LENGTH", l_rxVerifyLeParam.PACKET_TYPE, &packetLength);
        }
        else	
        {
            packetLength= l_rxVerifyLeParam.PACKET_LENGTH;
        }
        
        //Not needed for current BT LE
        vDUT_AddIntegerParameter(g_BT_Dut, "PACKET_LENGTH",     packetLength);


		err = vDUT_Run(g_BT_Dut, "RX_CLEAR_STATS");		
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.
			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Just return normal error message in this case
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(RX_CLEAR_STATS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			// do nothing
		}

		err = vDUT_Run(g_BT_Dut, "RX_START");		
		if ( ERR_OK!=err )
		{	
			vDutActived = false;
			//Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDUT
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Return error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(RX_START) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			vDutActived = true;
		}

        /*---------------------------------------------*
        * Send specific number of packets for PER test *
        *----------------------------------------------*/  
        err = ::LP_SetFrameCnt(waveformNumber);
        if ( ERR_OK!=err )
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] VSG send out packet LP_SetFrameCnt(%d) return error.\n", waveformNumber);
            throw logMessage;
        }
        else
        {
            LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[BT] VSG send out packet LP_SetFrameCnt(%d) return OK.\n", waveformNumber);
        }

		// Delay for DUT settle, depends on the number packets are sent.
		if (0!=g_BTGlobalSettingParam.BT_1LE_PER_WAVEFORM_INTERVAL_US)
		{
			Sleep((DWORD)((g_BTGlobalSettingParam.BT_1LE_PER_WAVEFORM_INTERVAL_US*l_rxVerifyLeParam.NUM_PACKET_IN_WAVEFORM*waveformNumber+1000)/1e3)); //Has to recalculate again rather than using l_rxVerifyLeParam.RX_PACKET_NUMBER, adding 1000 us for the DUT to settle
		}
		else
		{
			// do nothing
		}
#pragma endregion

#pragma region Retrieve analysis Results
	   /*--------------------*
		* Get Rx PER Result  *
		*--------------------*/
		double totalPackets = 0;
		double goodPackets  = 0;
		double badPackets   = 0;

		err = vDUT_Run(g_BT_Dut, "RX_GET_STATS");		
		if ( ERR_OK!=err )
		{	// Check if vDUT returns "ERROR_MESSAGE" or not; if "Yes", then the error message must be handled and returned to the upper layer.			err = vDUT_GetStringReturn(g_BT_Dut, "ERROR_MESSAGE", vDutErrorMsg, MAX_BUFFER_SIZE);
			if ( ERR_OK==err )	// Get "ERROR_MESSAGE" from vDut
			{
				err = -1;	// set err to -1, means "Error".
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, vDutErrorMsg);
				throw logMessage;
			}
			else	// Return error message
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_Run(RX_GET_STATS) return error.\n");
				throw logMessage;
			}
		}
		else
		{
			vDutActived = false; //the DUT already stops RX test based on Bluetooth LE spec. 
		}

		l_rxVerifyLeReturn.RX_POWER_LEVEL = l_rxVerifyLeParam.RX_POWER_LEVEL;

		err = vDUT_GetDoubleReturn(g_BT_Dut, "GOOD_PACKETS", &goodPackets);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] vDUT_GetDoubleReturn(GOOD_PACKETS) return error.\n");
			throw logMessage;
		}		
		if ( goodPackets<0 )
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] DUT report GOOD_PACKETS less than 0.\n");
			throw logMessage;
		}
		else
		{
			l_rxVerifyLeReturn.GOOD_PACKETS = goodPackets;
		}

        totalPackets = l_rxVerifyLeParam.NUM_PACKET_IN_WAVEFORM*waveformNumber;
        l_rxVerifyLeReturn.TOTAL_PACKETS = totalPackets;

		// Sometimes the DUT can get packets through other means, such as from the air
		// This is a workaround for some DUTs that return more ack than the number of packets sent
		if ( goodPackets > totalPackets )	
		{
			goodPackets = totalPackets;
//			l_rxVerifyLeReturn.GOOD_PACKETS  = goodPackets;
	//		l_rxVerifyLeReturn.TOTAL_PACKETS = totalPackets;
		}
		else
		{
			// do nothing	
		}
		
		badPackets = totalPackets - goodPackets;

		if( totalPackets!=0 )
		{
			l_rxVerifyLeReturn.PER = (badPackets/totalPackets) * 100.0;
		}
		else	// In this case, totalPackets = 0
		{
			l_rxVerifyLeReturn.PER = 100.0;
		}

#pragma endregion

	   /*-----------*
		*  Rx Stop  *
		*-----------*/
        //In LE, the DUT stops RX test to report test results, thus no need to stop RX again. 

		
	   /*----------------------------*
		* Disable VSG output signal  *
		*----------------------------*/
		err = LP_EnableVsgRF(0);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[BT] Fail to turn off VSG.\n");
			throw logMessage;
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if ( ERR_OK==err )
		{
			// Return Path Loss (dB)
			l_rxVerifyLeReturn.CABLE_LOSS_DB = l_rxVerifyLeParam.CABLE_LOSS_DB;

			sprintf_s(l_rxVerifyLeReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_rxVerifyLeReturnMap);
		}
		else
		{
			// do nothing
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_rxVerifyLeReturn.ERROR_MESSAGE, msg);
		if (err==ERR_OK)
			err = -1;
    }
    catch(...)
    {
		ReturnErrorMessage(l_rxVerifyLeReturn.ERROR_MESSAGE, "[BT] Unknown Error!\n");
		err = -1;
    }

	// This is a special case and happens only when certain errors occur before the RX_STOP. 
	// This is handled by error handling, but RX_STOP must be handled manually.
	
	if ( vDutActived )
	{
		vDUT_Run(g_BT_Dut, "RX_GET_STATS");
	}
	else
	{
		// do nothing
	}

    return err;
}

int InitializeRXVerifyLEContainers(void)
{
    /*-----------------*
     * Input Parameters *
     *-----------------*/
    l_rxVerifyLeParamMap.clear();

    BT_SETTING_STRUCT setting;

	l_rxVerifyLeParam.RX_POWER_LEVEL = -70.0;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyLeParam.RX_POWER_LEVEL))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyLeParam.RX_POWER_LEVEL;
        setting.unit        = "dBm";
        setting.helpText    = "The output power to verify PER of LE.";
        l_rxVerifyLeParamMap.insert( pair<string,BT_SETTING_STRUCT>("RX_POWER_LEVEL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerifyLeParam.FREQ_MHZ = 2402;
    setting.type = BT_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_rxVerifyLeParam.FREQ_MHZ))    // Type_Checking
    {
        setting.value       = (void*)&l_rxVerifyLeParam.FREQ_MHZ;
        setting.unit        = "MHz";
        setting.helpText    = "Channel center frequency in MHz, 2 MHz per step";
        l_rxVerifyLeParamMap.insert( pair<string,BT_SETTING_STRUCT>("FREQ_MHZ", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    strcpy_s(l_rxVerifyLeParam.PACKET_TYPE, MAX_BUFFER_SIZE, "1LE");
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyLeParam.PACKET_TYPE))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerifyLeParam.PACKET_TYPE;
        setting.unit        = "";
        setting.helpText    = "Sets the packet type, only allowed value is 1LE. Default = 1LE";
        l_rxVerifyLeParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_TYPE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    //l_rxVerifyLeParam.PACKET_LENGTH = 0;
    //setting.type = BT_SETTING_TYPE_INTEGER;
    //if (sizeof(int)==sizeof(l_rxVerifyLeParam.PACKET_LENGTH))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_rxVerifyLeParam.PACKET_LENGTH;
    //    setting.unit        = "octets";
    //    setting.helpText    = "Not Used in current LE Test! Has to be 37 octets. \nReserved for future use. The number of octets of payload data to verify LE RX. Maximum value is 37. Default = 0, means using default global setting value.";
    //    l_rxVerifyLeParamMap.insert( pair<string,BT_SETTING_STRUCT>("PACKET_LENGTH", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

    strcpy_s(l_rxVerifyLeParam.WAVEFORM_NAME, MAX_BUFFER_SIZE, "1LE.mod");
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyLeParam.WAVEFORM_NAME))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerifyLeParam.WAVEFORM_NAME;
        setting.unit        = "";
        setting.helpText    = "Sets the waveform name for 1LE PER test, Default = 1LE.mod, ideal PRBS9 LE waveform.";
        l_rxVerifyLeParamMap.insert( pair<string,BT_SETTING_STRUCT>("WAVEFORM_NAME", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    //l_rxVerifyLeParam.NUM_PACKET_IN_WAVEFORM = 0; //might needed for future use
    //setting.type = BT_SETTING_TYPE_INTEGER;
    //if (sizeof(int)==sizeof(l_rxVerifyLeParam.NUM_PACKET_IN_WAVEFORM))    // Type_Checking
    //{
    //    setting.value       = (void*)&l_rxVerifyLeParam.NUM_PACKET_IN_WAVEFORM;
    //    setting.unit        = "";
    //    setting.helpText    = "The number of packets in one waveform to verify LE. Default = 0 means customer only use LP provided waveform and the program will automatically detect number of packets";
    //    l_rxVerifyLeParamMap.insert( pair<string,BT_SETTING_STRUCT>("NUM_PACKET_IN_WAVEFORM", setting) );
    //}
    //else    
    //{
    //    printf("Parameter Type Error!\n");
    //    exit(1);
    //}

	l_rxVerifyLeParam.RX_PACKET_NUMBER = 1500;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyLeParam.RX_PACKET_NUMBER))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyLeParam.RX_PACKET_NUMBER;
        setting.unit        = "";
        setting.helpText    = "The minimum number of packets for PER test. Default value is 1,500.";
        l_rxVerifyLeParamMap.insert( pair<string,BT_SETTING_STRUCT>("RX_PACKET_NUMBER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyLeParam.CABLE_LOSS_DB = 0.0;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyLeParam.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyLeParam.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester. Default = 0,  means using default global setting value.";
		l_rxVerifyLeParamMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
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
    l_rxVerifyLeReturnMap.clear();

	l_rxVerifyLeReturn.TOTAL_PACKETS = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyLeReturn.TOTAL_PACKETS))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyLeReturn.TOTAL_PACKETS;
        setting.unit        = "packets";
        setting.helpText    = "TOTAL_PACKETS is the total number of packets that transmitted by the tester.";
        l_rxVerifyLeReturnMap.insert( pair<string,BT_SETTING_STRUCT>("TOTAL_PACKETS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyLeReturn.GOOD_PACKETS = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyLeReturn.GOOD_PACKETS))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyLeReturn.GOOD_PACKETS;
        setting.unit        = "packets";
        setting.helpText    = "GOOD_PACKETS is the number of received packets that reported by the DUT.";
        l_rxVerifyLeReturnMap.insert( pair<string,BT_SETTING_STRUCT>("GOOD_PACKETS", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_rxVerifyLeReturn.PER = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyLeReturn.PER))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyLeReturn.PER;
        setting.unit        = "%";
        setting.helpText    = "PER in percentage.";
        l_rxVerifyLeReturnMap.insert( pair<string,BT_SETTING_STRUCT>("PER", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyLeReturn.RX_POWER_LEVEL = NA_NUMBER;
    setting.type = BT_SETTING_TYPE_DOUBLE;
    if (sizeof(double)==sizeof(l_rxVerifyLeReturn.RX_POWER_LEVEL))    // Type_Checking
    {
        setting.value = (void*)&l_rxVerifyLeReturn.RX_POWER_LEVEL;
        setting.unit        = "dBm";
        setting.helpText    = "RX power level for PER test";
        l_rxVerifyLeReturnMap.insert( pair<string,BT_SETTING_STRUCT>("RX_POWER_LEVEL", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

	l_rxVerifyLeReturn.CABLE_LOSS_DB = NA_NUMBER;
	setting.type = BT_SETTING_TYPE_DOUBLE;
	if (sizeof(double)==sizeof(l_rxVerifyLeReturn.CABLE_LOSS_DB))    // Type_Checking
	{
		setting.value       = (void*)&l_rxVerifyLeReturn.CABLE_LOSS_DB;
		setting.unit        = "dB";
		setting.helpText    = "Cable loss from the DUT antenna port to tester";
		l_rxVerifyLeReturnMap.insert( pair<string,BT_SETTING_STRUCT>("CABLE_LOSS_DB", setting) );
	}
	else    
	{
		printf("Parameter Type Error!\n");
		exit(1);
	}

    l_rxVerifyLeReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = BT_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_rxVerifyLeReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_rxVerifyLeReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_rxVerifyLeReturnMap.insert( pair<string,BT_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}


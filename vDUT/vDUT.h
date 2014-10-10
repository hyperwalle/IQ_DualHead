/*! \mainpage Virtual DUT Interface
 *
 * \section intro_sec Introduction
 *
 * Virtual DUT provides a unified interface and facility for the following:
 * - Isolate the actual DUT implementation from top level applications
 * - Allow multiple DUTs (one DUT per technology) to exist in the same application
 *
 * Virtual DUT does not have any actual implementation of a DUT.  Instead, Virtual DUT manages a list of 
 * pre-defined, standardized DUT function names, such as "INSERT_DUT", "START_TX", etc., each of which 
 * will have a callback function to be "installed" by the actual DUT implementation.
 *
 * Once the callback function of a pre-defined DUT function name, for example, "INSERT_DUT" is "installed" 
 * by a DUT implementation, Virtual DUT knows that this function is supported by the DUT, and will execute 
 * the callback function when needed.  For those function names whose callback functions are left empty (NULL pointer),
 * Virtual DUT knows that this DUT does not support those functions.
 *
 * Virtual DUT allows multiple DUTs to exist in the same solution, with the limitation of one DUT per technology.
 * Technologies include the following:
 *    - WIFI
 *    - BT
 *    - WIMAX
 *    - GPS
 *    - FM
 *
 * The prototype of callback function is defined as below:
 * \code void (*pointerToFunction)(void) \endcode
 *
 * It's clearly seen that by the prototype above callback functions do not take parameters and nor do they return 
 * values.  But, in general, functions do need to take inputs and generate outputs.  So, how will Virtual DUT 
 * deal with inputs and outputs?
 *
 * \subsection inputs_outputs Inputs and Outputs of Callback Functions
 *
 * Virtual DUT uses containers to manage input parameters and return values.
 *
 * The following funtions are provided to callers for passing parameters to callees:
 *      - vDUT_ClearParameters()
 *      - vDUT_AddIntegerParameter()
 *      - vDUT_AddDoubleParameter()
 *      - vDUT_AddStringParameter()
 *
 * And the following functions are provided to callees for retrieving input parameters:
 *      - vDUT_GetIntegerParameter()
 *      - vDUT_GetDoubleParameter()
 *      - vDUT_GetStringParameter()
 *
 * The following functions are provided to callees for returning values:
 *      - vDUT_ClearReturns()
 *      - vDUT_AddIntegerReturn()
 *      - vDUT_AddDoubleReturn()
 *      - vDUT_AddStringReturn()
 *
 * And the following functions are provided to callers for retrieving returned values:
 *      - vDUT_GetIntegerReturn()
 *      - vDUT_GetDoubleReturn()
 *      - vDUT_GetStringReturn()
 *
 * \section revision_sec Revision History
 *
 * \subsection revision_0_0_2 0.0.2
 * Date: Mar 16, 2009
 *
 * \subsection revision_0_0_1 0.0.1
 * Date: Aug 13, 2008
 * - First draft
 */

/*! \file vDUT.h
 * \brief vDUT Interface functions 
 */ 
#include <vector>


#ifndef _VDUT_H_
#define _VDUT_H_

#if !defined (__cplusplus)
    #define vDUT_API extern
#elif !defined (WIN32)
    #define vDUT_API extern "C"
#elif defined(VDUT_EXPORTS)
    #define vDUT_API extern "C" __declspec(dllexport)
#else
    #define vDUT_API extern "C" __declspec(dllimport) 
#endif

#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif
#ifndef BOOL
    typedef int BOOL;
#endif

typedef char * vDUT_STR;      /*!< An ANSI C string typedef for convenience in vDUT */
typedef signed int vDUT_ID;   /*!< An unsigned int typedef for convenience in vDUT */

#define MAX_TECHNOLOGIES_COUNT  8

#define TX_TARGET_POWER_FLAG	-99

#ifndef MAX_BUFFER_SIZE
    #define MAX_BUFFER_SIZE 4096
#endif

#ifndef MAX_POWER_STEP
    #define MAX_POWER_STEP 50
#endif

/*! Error Code Definition
 */
typedef enum vDUT_EC
{
	vDUT_ERR_DUT_NOT_CONNECTED = -1,		 /*!< DUT is not connected*/
    vDUT_ERR_OK,                             /*!< Function completes with no errors*/
    vDUT_ERR_TECHNOLOGY_DOES_NOT_EXIST,      /*!< Specified technology is not one of the pre-defined technologies*/
    vDUT_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED, /*!< Another DUT has registered the specified technology*/
	vDUT_ERR_FUNCTION_NOT_SUPPORTED,         /*!< DUT does not support the specified function*/
	vDUT_ERR_FUNCTION_NOT_DEFINED,           /*!< The specified function has not been defined yet*/
	vDUT_ERR_PARAM_DOES_NOT_EXIST,           /*!< The specified parameter does not exist*/
    vDUT_ERR_INVALID_DUT_ID,                 /*!< Invalid DUT ID*/
    vDUT_ERR_DUT_FUNCTION_ERROR,             /*!< DUT function had errors*/
	vDUT_ERR_WIFI_FREQ_DOES_NOT_EXIST,       /*!< WiFi frequency does not exist*/
    vDUT_ERR_FAILED_TO_LOAD_DUT_DLL,         /*!< Failed to load the specified DUT control DLL*/
    vDUT_ERR_NO_DUT_REGISTER_FUNCTION_FOUND, /*!< No register function found in the DUT control DLL*/
    vDUT_ERR_DUT_REGISTER_FUNCTION_FAILED,   /*!< Register function in the DUT control DLL failed*/
    vDUT_ERR_MAXIMUM_NUM              
} vDUT_RETURN;

typedef enum tagDutType
{
    DUT_TYPE_LEGACY,
    DUT_TYPE_MPS
}vDUT_DUT_TYPE;

typedef enum tagPreambleType
{
    PREAMBLE_TYPE_SHORT,
    PREAMBLE_TYPE_LONG
}vDUT_PREAMBLE_TYPE;

typedef enum tagBandWidthMode
{
    BANDWIDTH_MODE_HT20,
    BANDWIDTH_MODE_HT40,
	BANDWIDTH_MODE_VHT20,
	BANDWIDTH_MODE_VHT40,
	BANDWIDTH_MODE_VHT80,
	BANDWIDTH_MODE_NOT_SUPPORT 
}vDUT_BANDWIDTH_MODE;

typedef enum tagBandWidth
{
    BW_20MHZ,
	BW_40MHZ,
	BW_80MHZ,
	BW_160MHZ,
	BW_80_80MHZ,
	BW_NOT_SUPPORT
}vDUT_BANDWIDTH;


typedef enum tag11nPacketFormatType
{
    PACKET_FORMAT_11N_MIXED,
    PACKET_FORMAT_11N_GREENFIELD
}vDUT_PACKET_FORMAT_11N;
typedef enum tag11nGuardIntervalFormatType
{
	GUARD_INTERVAL_FORMAT_11N_LONG,
	GUARD_INTERVAL_FORMAT_11N_SHORT
}vDUT_GUARD_INTERVAL_11N;

typedef enum tagvDutDataRateType
{
    DSSS1,  
    DSSS2,  
    CCK5_5, 
    CCK11,  
    PBCC22, 
    OFDM6,  
    OFDM9,  
    OFDM12, 
    OFDM18, 
    OFDM24, 
    OFDM36, 
    OFDM48, 
    OFDM54, 
    MCS0 = 14,    
    MCS1,    
    MCS2,    
    MCS3,    
    MCS4,    
    MCS5,    
    MCS6,    
    MCS7,    
    MCS8,    
    MCS9,    
    MCS10,   
    MCS11,   
    MCS12,   
    MCS13,   
    MCS14,   
    MCS15,   
    MCS16,   
    MCS17,   
    MCS18,   
    MCS19,   
    MCS20,   
    MCS21,   
    MCS22,   
    MCS23,   
    MCS24,   
    MCS25,   
    MCS26,   
    MCS27,   
    MCS28,   
    MCS29,   
    MCS30,   
    MCS31,
    BT_1DH1 = 100,
    BT_1DH3,
    BT_1DH5,
    BT_2DH1,
    BT_2DH3,
    BT_2DH5,
    BT_3DH1,
    BT_3DH3,
    BT_3DH5,
    BT_1LE,
  	HALF3=200,
    HALF4_5,
    HALF6,
    HALF9,
  	HALF12,
  	HALF18,
  	HALF24,
  	HALF27,
  	QUAR1_5,
  	QUAR2_25,
  	QUAR3,
  	QUAR4_5,
  	QUAR6,
  	QUAR9,
  	QUAR12,
  	QUAR13_5,
	DATA_RATE_NOT_SUPPORT
}vDUT_DATA_RATE;


typedef enum tagCalBandType 
{
    CAL_B_BAND, 
    CAL_A_BAND_LOW, 
    CAL_A_BAND_MID, 
    CAL_A_BAND_HIGH,
    CAL_INVALID_BAND
}vDUT_CAL_BAND_TYPE;



typedef enum tagSeqMeasType
{
    SEQ_MEAS_EVM,
    SEQ_MEAS_MASK,
    SEQ_MEAS_POWER,
    SEQ_MEAS_SPECTRUM,
    SEQ_MEAS_RAMP,
    SEQ_MEAS_RX_PER_SWEEP,
    SEQ_MEAS_RX_PER_TRGT_SENS, //for low power sens test
    SEQ_MEAS_RX_PER_MAX_SENS,  //for max power sens test
    MAX_SEQ_MEAS_NUM
}SEQ_MEAS_TYPE;

// Bluetooth modulation pattern
enum 
{
	BT_PATTERN_00=1,		// 0x00 8-bit Pattern	   ZERO : all zero
	BT_PATTERN_FF,			// 0xFF 8-bit Pattern      ONE1 : all ones
	BT_PATTERN_AA,			// 0xAA 8-bit Pattern      HEX_A : 1010
	BT_PATTERN_PRBS,		// PRBS9 Pattern           RANDOM ;  PRBS9
	BT_PATTERN_F0=9			// 0xF0 8-bit Pattern	   HEX_F0 : 11110000
};							

enum 
{ 
	BT_LE_PATTERN_PRBS9  = 0x00, // "Pseudo-Random bit sequence 9"
	BT_LE_PATTERN_F0     = 0x01, // "Pattern of alternating bits '11110000'"
	BT_LE_PATTERN_AA     = 0x02, // "Pattern of alternating bits '10101010'
	BT_LE_PATTERN_PRBS15 = 0x03, // "Pseudo-Random bit sequence 15 - Optional"
	BT_LE_PATTERN_FF     = 0x04, // "Pattern of All '1' bits - Optional"
	BT_LE_PATTERN_00     = 0x05, // "Pattern of All '0' bits - Optional"
	BT_LE_PATTERN_0F     = 0x06, // "Pattern of alternating bits '00001111' - Optional"
	BT_LE_PATTERN_55     = 0x07 // "Pattern of alternating bits '01010101' - Optional"
};


typedef struct vDUT_SeqTxMeasConfig
{
    SEQ_MEAS_TYPE           measType;
    int                     freqMhz;
    int                     packetSkipCount;
    int                     captureCount;
    int                     captureLengthUs; 
    int                     preTriggerTimeUs;
    int                     packetNumOfBytes;
    int                     startTimeUs;
    int                     captureEndUs;
    vDUT_PREAMBLE_TYPE      preamble;
    vDUT_DATA_RATE          dataRate;
    vDUT_BANDWIDTH_MODE     bandWidth;
    vDUT_PACKET_FORMAT_11N  packetFormat11n;
	vDUT_GUARD_INTERVAL_11N guardIntervalFormat11N;
    double                  powerLevelDBm;   
    double                  obwPercentage;
    double                  correctionFactor11B;
    double                  txChainGainDb;
}SEQ_TX_CONFIG;

typedef struct vDUT_SeqRxMeasConfig
{
    SEQ_MEAS_TYPE           measType;
    int                     freqMhz;
    int                     packetNumber;
    int                     segmentNum;
    vDUT_PREAMBLE_TYPE      preamble;
    vDUT_DATA_RATE          dataRate;
    vDUT_PACKET_FORMAT_11N  packetFormat11n;
	vDUT_GUARD_INTERVAL_11N guardIntervalFormat11N;
    double                  sweepStepDb;
    double                  startPowerDBm;
    double                  stopPowerDBm; 
    double                  perSensPercentage;
    double                  rxAckPowerDBm;
}SEQ_RX_CONFIG;

typedef struct vDUT_SeqTxOneDataRateConfig
{
    vDUT_DATA_RATE      dataRate;
    double              p_to_a;
	std::vector <SEQ_TX_CONFIG> seqTxConfig;
}SEQ_TX_ONE_DATARATE_CONFIG;

//! Register DUT with a specific technology, such as WIFI, BT, WIMAX, etc.
/*!
 *
 * \param[in] technology Name of the technology to register 
 * \param[out] *dutID Return the ID of the DUT that has been registered
 *
 * \return vDUT_ERR_OK if the specified Technology Name is registered with the DUT
 * \return vDUT_ERR_TECHNOLOGY_DOES_NOT_EXIST if the specified technology is not one of the pre-defined technologies
 * \return vDUT_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED if the specified technology has been registered by another DUT
 *
 * \remark Virtual DUT supports the concept of multiple technologies, or Multicom, to allow multiple DUTs
 * to exist in the same solution.  Each technology will have zero, or one DUT to be registered.  One DUT 
 * can only register to one technology.
 */
vDUT_API vDUT_RETURN vDUT_RegisterTechnology(const vDUT_STR technology, vDUT_ID *dutID);

//! Unregister a DUT
/*!
 * \param[out] technology Technology name with a DUT registered.  
 *
 * \return vDUT_ERR_OK if the DUT has been unregistered
 * \remark After vDUT_UnregisterTechnology(), the same technology can be registered with different DUT
 */
vDUT_API vDUT_RETURN vDUT_UnregisterTechnology(const vDUT_STR technology);



//! Register a DUT DLL for a specific technology, such as WIFI, BT, WIMAX, etc.
/*!
 *
 * \param[in] technology Name of the technology to register for
 * \param[in] dutDllFileName Specifies the DUT control DLL file name
 * \param[out] *dutID Returns the ID of the DUT that has been registered
 *
 * \return vDUT_ERR_OK if the specified DUT control DLL has been registered for the technology
 * \return vDUT_ERR_TECHNOLOGY_DOES_NOT_EXIST if the specified technology is not one of the pre-defined technologies
 * \return vDUT_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED if the specified technology has been registered by another DUT
 * \return vDUT_ERR_FAILED_TO_LOAD_DUT_DLL
 * \return vDUT_ERR_NO_DUT_REGISTER_FUNCTION_FOUND
 * \return vDUT_ERR_DUT_REGISTER_FUNCTION_FAILED
 *
 * \remark Virtual DUT supports the concept of multiple technologies, or Multicom, to allow multiple DUTs
 * to exist in the same solution.  Each technology will have zero, or one DUT to be registered for, and one DUT 
 * can only register for one technology.
 *
 * vDUT_RegisterDutDll() differs from vDUT_RegisterTechnology() as follows:
 *      -# vDUT_RegisterTechnology() was made to be called in a particular DUT control.  In this way,
 *         the DUT control has to expose one function to be called explicitly in test modules
 *      -# vDUT_RegisterDutDll() is meant to be called directly from test modules.  By doing so, users
 *         can easily select any DUT control by specifying a DLL name.  More importantly, this is happening
 *         during run-time stage.
 *         However, each DUT control DLL has to export one function, following the below name convention:
 *              - int <TECHNOLOGY>_DutRegister(void)
 *          where <TECHNOLOGY> is one of the pre-defined technologies in vDUT, listed below:
 *              - WIFI
 *              - BT
 *              - WIMAX
 *              - FM
 *              - GPS
 */
vDUT_API vDUT_RETURN vDUT_RegisterDutDll(const vDUT_STR technology, vDUT_STR dutDllFileName, vDUT_ID *dutID);


//! Unregister a DUT DLL
/*!
 * \param[out] technology Technology name with a DUT registered.  
 * \param[in,out] *dutID Points to the DUT ID that has been registered previously.  It'll be set to -1.
 *
 * \return vDUT_ERR_OK if the DUT has been unregistered
 * \remark After vDUT_UnregisterDutDll(), the same technology can be registered with different DUT
 */
vDUT_API vDUT_RETURN vDUT_UnregisterDutDll(const vDUT_STR technology, vDUT_ID *dutID);


//! Install a DUT callback function for a specified function name
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] functionName The function name (one of the pre-defined function names)
 * \param[in] pointerToFunction The callback function (pointer to function)
 *
 * \return vDUT_ERR_OK if the callback function has been "installed" to the specified function name
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 * \return vDUT_ERR_FUNCTION_NOT_DEFINED The specified function name is not defined
 *
 * \remark The C++ complier will guarauntee that the callback function has been implemented.
 */
vDUT_API vDUT_RETURN vDUT_InstallCallbackFunction(vDUT_ID dutID, const vDUT_STR functionName, int (*pointerToFunction)(void) );

//! Run a DUT function
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] functionName The function name (one of the pre-defined function names)
 *
 * \return vDUT_ERR_OK if the DUT function executed successfully
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 * \return vDUT_ERR_FUNCTION_NOT_DEFINED The specified function name is not defined
 * \return vDUT_ERR_FUNCTION_NOT_SUPPORTED The specified function name does not have a callback function installed
 * \return vDUT_ERR_DUT_FUNCTION_ERROR The DUT function generated error(s)
 *
 */
vDUT_API vDUT_RETURN vDUT_Run(vDUT_ID dutID, const vDUT_STR functionName);

//! Clear all parameter containers
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 *
 * \return vDUT_ERR_OK
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 *
 */
vDUT_API vDUT_RETURN vDUT_ClearParameters(vDUT_ID dutID);

//! Add an Integer parameter
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Parameter name
 * \param[in] paramValue Parameter value
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 */
vDUT_API vDUT_RETURN vDUT_AddIntegerParameter(vDUT_ID dutID, const vDUT_STR paramName, const int paramValue);

//! Add a Double parameter
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Parameter name
 * \param[in] paramValue Parameter value
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 */
vDUT_API vDUT_RETURN vDUT_AddDoubleParameter(vDUT_ID dutID, const vDUT_STR paramName, const double paramValue);

//! Add a String parameter
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Parameter name
 * \param[in] paramValue Parameter value
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 */
vDUT_API vDUT_RETURN vDUT_AddStringParameter(vDUT_ID dutID, const vDUT_STR paramName, const vDUT_STR paramValue);


//! Remove an Integer parameter
/*!
* \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
* \param[in] paramName Parameter name
*
* \return vDUT_ERR_OK No errors
* \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
*/
vDUT_API vDUT_RETURN vDUT_RemoveIntegerParameter(vDUT_ID dutID, const vDUT_STR paramName);

//! Remove a Double parameter
/*!
* \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
* \param[in] paramName Parameter name
*
* \return vDUT_ERR_OK No errors
* \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
*/
vDUT_API vDUT_RETURN vDUT_RemoveDoubleParameter(vDUT_ID dutID, const vDUT_STR paramName);

//! Remove a String parameter
/*!
* \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
* \param[in] paramName Parameter name
*
* \return vDUT_ERR_OK No errors
* \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
*/
vDUT_API vDUT_RETURN vDUT_RemoveStringParameter(vDUT_ID dutID, const vDUT_STR paramName);


//! Get an Integer parameter
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Parameter name
 * \param[out] paramValue Parameter value
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 * \return vDUT_ERR_PARAM_DOES_NOT_EXIST The parameter name does not exist in the container
 */
vDUT_API vDUT_RETURN vDUT_GetIntegerParameter(vDUT_ID dutID, const vDUT_STR paramName, int* paramValue);

//! Get a Double parameter
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Parameter name
 * \param[out] paramValue Parameter value
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 * \return vDUT_ERR_PARAM_DOES_NOT_EXIST The parameter name does not exist in the container
 */
vDUT_API vDUT_RETURN vDUT_GetDoubleParameter(vDUT_ID dutID, const vDUT_STR paramName, double* paramValue);

//! Get a String parameter
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Parameter name
 * \param[out] paramValue Parameter value
 * \param[in] bufferSize Indicates the size of paramValue
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 * \return vDUT_ERR_PARAM_DOES_NOT_EXIST The parameter name does not exist in the container
 */
vDUT_API vDUT_RETURN vDUT_GetStringParameter(vDUT_ID dutID, const vDUT_STR paramName, vDUT_STR paramValue, int bufferSize);

//! Clear all return value containers
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 *
 * \return vDUT_ERR_OK
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 *
 */
vDUT_API vDUT_RETURN vDUT_ClearReturns(vDUT_ID dutID);

//! Add an Integer return value
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Return parameter name
 * \param[in] paramValue Return parameter value
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 */
vDUT_API vDUT_RETURN vDUT_AddIntegerReturn(vDUT_ID dutID, const vDUT_STR paramName, const int paramValue);

//! Add a Double return value
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Return parameter name
 * \param[in] paramValue Return parameter value
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 */
vDUT_API vDUT_RETURN vDUT_AddDoubleReturn(vDUT_ID dutID, const vDUT_STR paramName, const double paramValue);

//! Add a String return value
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Return parameter name
 * \param[in] paramValue Return parameter value
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 */
vDUT_API vDUT_RETURN vDUT_AddStringReturn(vDUT_ID dutID, const vDUT_STR paramName, const vDUT_STR paramValue);

//! Get an Integer return value
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Return parameter name
 * \param[out] paramValue Return parameter value
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 * \return vDUT_ERR_PARAM_DOES_NOT_EXIST The parameter name does not exist in the container
 */
vDUT_API vDUT_RETURN vDUT_GetIntegerReturn(vDUT_ID dutID, const vDUT_STR paramName, int* paramValue);

//! Get a Double return value
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Return parameter name
 * \param[out] paramValue Return parameter value
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 * \return vDUT_ERR_PARAM_DOES_NOT_EXIST The parameter name does not exist in the container
 */
vDUT_API vDUT_RETURN vDUT_GetDoubleReturn(vDUT_ID dutID, const vDUT_STR paramName, double* paramValue);

//! Get a String return value
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] paramName Return parameter name
 * \param[out] paramValue Return parameter value
 * \param[in] bufferSize Indicates the size of paramValue
 *
 * \return vDUT_ERR_OK No errors
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 * \return vDUT_ERR_PARAM_DOES_NOT_EXIST The parameter name does not exist in the container
 */
vDUT_API vDUT_RETURN vDUT_GetStringReturn(vDUT_ID dutID, const vDUT_STR paramName, vDUT_STR paramValue, int bufferSize);

//! Return the description for the specified error code
/*!
 *
 * \param[in] errorNum Error number returned by any vDUT functions 
 *
 * \return A pointer to the error description (ANSI C String)
 *
 */
vDUT_API char* vDUT_GetErrorString(vDUT_RETURN errorNum);

//! Convert WiFi frequency (MHz) to channel number
/*!
 *
 * \param[in] freq Frequency in MHz 
 * \param[out] channel The channel number corresponding to the specified frequency 
 *
 * \return vDUT_ERR_OK If the specified frequency has been converted to channel number
 *
 */
vDUT_API vDUT_RETURN vDUT_WiFiConvertFrequencyToChannel(int freq, int* channel);

//! Convert WiFi channel to frequency (MHz) number
/*!
 *
 * \param[in]  channel, The channel number 
 * \param[out] freq, Frequency in MHz corresponding to the specified channel 
 *
 * \return vDUT_ERR_OK If the channel number has been converted to frequency
 *
 */
vDUT_API vDUT_RETURN vDUT_WiFiConvertChannelToFrequency(int channel, int *freq);
//! Convert WiFi data rate to index
/*!
 *
 * \param[in] name The name of Data Rate, such as "DSSS-1", "CCK-11", "OFDM-54", "MCS7", "MCS15", etc. 
 * \param[out] index The corresponding index for the specified data rate 
 *
 * \return vDUT_ERR_OK If the specified data rate has been found
 *
 */
vDUT_API vDUT_RETURN vDUT_WiFiConvertDataRateNameToIndex(vDUT_STR name, vDUT_DATA_RATE* index);


//! Get vDut version information
/*!
 *
 * \param[out] paramValue Parameter value 
 * \param[in] bufferSize Indicates the size of paramValue
 *
 * \return vDUT_ERR_OK If the vDut version information has been found
 *
 */
vDUT_API vDUT_RETURN vDUT_GetVersion(vDUT_STR paramValue, int bufferSize);

//! Bypass the DUT control
/*!
 * \param[in] vDutControlFlag  A flag to bypass the Dut control, 0 means bypass, 1 means need Dut control, default is 1.
 *
 * \return vDUT_ERR_OK 
 */
vDUT_API vDUT_RETURN vDUT_ByPassDutControl(int vDutControlFlag);

//Test Functions
vDUT_API vDUT_RETURN vDUT_GetDoubleReturnPairs(vDUT_ID dutID,const int maxPairs, int *numDoublePairs, char doublePairName[][64], double doublePairValue[]);
vDUT_API vDUT_RETURN vDUT_GetIntegerReturnPairs(vDUT_ID dutID,const int maxPairs, int *numIntegerPairs, char integerPairName[][64], int integerPairValue[]);
vDUT_API vDUT_RETURN vDUT_AddSequenceParameters(vDUT_ID dutID, SEQ_TX_CONFIG *sequenceParameters, int seqStepCounter);
vDUT_API vDUT_RETURN vDUT_AddTxOneDataRateSequenceParameters(vDUT_ID dutID, std::vector<SEQ_TX_ONE_DATARATE_CONFIG>::iterator &sequenceIter, int seqStepCounter);
vDUT_API vDUT_RETURN vDUT_AddRxSequenceParameters(vDUT_ID dutID, SEQ_RX_CONFIG *sequenceParameters, int seqStepCounter);
vDUT_API vDUT_RETURN vDUT_GetSequenceParameters(vDUT_ID dutID, SEQ_TX_CONFIG *sequenceParameters);
vDUT_API vDUT_RETURN vDUT_GetTxOneDataRateSequenceParameters(vDUT_ID dutID, std::vector<SEQ_TX_ONE_DATARATE_CONFIG> &sequenceTxOneDataRate);
vDUT_API vDUT_RETURN vDUT_GetRxSequenceParameters(vDUT_ID dutID, SEQ_RX_CONFIG *sequenceParameters);
vDUT_API vDUT_RETURN vDUT_WiFiConvertDataRateNameToMbps(vDUT_STR name, double* dataRateMbps);
vDUT_API vDUT_RETURN vDUT_WiFiConvertPacketLengthUsToNumberOfBytes(int packetLengthUs, char *dataRate, vDUT_BANDWIDTH_MODE bandwidth, vDUT_PREAMBLE_TYPE preamble, vDUT_PACKET_FORMAT_11N packetType11n, int* numberOfBytes);
vDUT_API vDUT_RETURN vDUT_GetArrayDoubleReturn(vDUT_ID dutID, const vDUT_STR paramName, double *paramValue, const int arraySize);
vDUT_API vDUT_RETURN vDUT_AddArrayDoubleReturn(vDUT_ID dutID, const vDUT_STR paramName, const double *paramValue, const int arraySize);
vDUT_API vDUT_RETURN vDUT_GetDutObjectPointer(void **DutObj, int *DutConnected);
vDUT_API vDUT_RETURN vDUT_SetDutObjectPointer(void *DutObj, int DutConnected);
vDUT_API vDUT_RETURN vDUT_SetMiscIntVal(const vDUT_STR paramName, const int mIntVal);
vDUT_API vDUT_RETURN vDUT_GetMiscIntVal(const vDUT_STR paramName, int *pIntVal);
vDUT_API vDUT_RETURN vDUT_SetMiscStrVal(const vDUT_STR paramName, const vDUT_STR paramValue);
vDUT_API vDUT_RETURN vDUT_GetMiscStrVal(const vDUT_STR paramName, vDUT_STR paramValue, int bufferSize);
vDUT_API vDUT_RETURN vDUT_ClearMiscValues(void);

#endif

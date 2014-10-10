/*! \mainpage Test Manager Interface
 *
 * \section intro_sec Introduction
 *
 *
 * Test Manager accommodates executing test functions of multiple technologies, which include the following:
 *    - WIFI
 *    - BT
 *    - WIMAX
 *    - GPS
 *    - FM
 *
 * Test Manager defines a set of  "Function Keywords", such as TX_VERIFY_EVM, RX_VERIFY_PER 
 * for each technology.  Click the links below to see all function keywords defined for each technology:
 *    - \ref wifi_test_function_keywords  "WiFi Test Function Keywords"
 *    - \ref bt_test_function_keywords    "Bluetooth Test Function Keywords"
 *    - \ref wimax_test_function_keywords "WiMAX Test Function Keywords"
 *    - \ref gps_test_function_keywords   "GPS Test Function Keywords"
 *    - \ref fm_test_function_keywords    "FM Test Function Keywords"
 *
 * Users will use those "Function Keywords" to create a "Test Flow" without knowing the specific C function names.
 * Test Manager is responsible for executing the actual C functions based on the specified "Function Keywords"
 *
 * \subsection test_manager_execute_c_functions How does Test Manager execute the actual C functions?
 *
 * Test Manager maintains a table, each row of which contains a "Function Keyword" and a pointer to function, with 
 * the following prototype:
 *
 * \code 
    void (*pointerToFunction)(void) 
 * \endcode
 *
 * When Test Manager starts, it sets the pointer to function to NULL for all "Function Keywords", so no test functions
 * can be executed.
 *
 * The test implementation of each technology needs to provide a special function to its users.  This special function
 * will set the pointer to function in the table maintained in Test Manager to the actual C function for each 
 * "Function Keyword".  The actual C functions are called "Callback" functions.
 *
 * Let's take WiFi Test as the example.  If the WiFi Test is implemented in project WiFi_Test, and this project includes
 * a special function called WiFi_Test_Register(), as shown below:
 *
 * \code
    WIFI_TEST_API int WiFi_Test_Register()
    {
        TM_ID technologyID=-1;
        TM_RegisterTechnology("WIFI", &technologyID);
        if( technologyID>=0 )
        {  
            TM_InstallCallbackFunction(technologyID, "GLOBAL_SETTINGS",      WiFi_Global_Settings);
            TM_InstallCallbackFunction(technologyID, "CONNECT_IQ_TESTER",    WiFi_Connect_IQTester);
            TM_InstallCallbackFunction(technologyID, "DISCONNECT_IQ_TESTER", WiFi_Disconnect_IQTester);
            TM_InstallCallbackFunction(technologyID, "INSERT_DUT",           WiFi_Insert_Dut);
            TM_InstallCallbackFunction(technologyID, "REMOVE_DUT",           WiFi_Remove_Dut);
            TM_InstallCallbackFunction(technologyID, "TX_CALIBRATION",       WiFi_TX_Calibration);
			TM_InstallCallbackFunction(technologyID, "RX_CALIBRATION",       WiFi_RX_Calibration);
            TM_InstallCallbackFunction(technologyID, "TX_VERIFY_EVM",        WiFi_TX_Verify_Evm);
            TM_InstallCallbackFunction(technologyID, "TX_VERIFY_POWER",      WiFi_TX_Verify_Power);
            TM_InstallCallbackFunction(technologyID, "TX_VERIFY_MASK",       WiFi_TX_Verify_Mask);
            TM_InstallCallbackFunction(technologyID, "RX_RSSI_CAL",          WiFi_Rssi_Cal);
            TM_InstallCallbackFunction(technologyID, "RX_VERIFY_PER",        WiFi_RX_Verify_Per);
            TM_InstallCallbackFunction(technologyID, "RX_VERIFY_PER_MAX",    WiFi_RX_Verify_Per_Max);
            TM_InstallCallbackFunction(technologyID, "RX_SWEEP_PER",         WiFi_Sweep_Per);
            TM_InstallCallbackFunction(technologyID, "READ_EEPROM",          WiFi_Read_Eeprom);
            TM_InstallCallbackFunction(technologyID, "WRITE_EEPROM",         WiFi_Write_Eeprom);
            TM_InstallCallbackFunction(technologyID, "RUN_EXTERNAL_PROGRAM", WiFi_Run_External_Program);
            g_WiFi_Test_ID = technologyID;
        }

        return technologyID;
    }
 * \endcode
 *
 * Once users call WiFi_Test_Register(), for those "Function Keywords" that have callback functions installed,
 * Test Manager knows those "Function Keywords" are executable. And for those "Function Keywords" that does not
 * have callback functions installed, Test Manager knows they are not executable.
 *
 * As described earlier, users only know the pre-defined "Function Keywords" for each technology, and Test Manager 
 * is responsible for executing them.  All "Function Keywords" are executed by calling TM_Run(), as shown below:
 *
 * \code TM_Run(WiFi_Test, "TX_VERIFY_EVM"); \endcode
 *
 * \remark The C/C++ compiler ensures that all callback functions have been declared and implemented properly;
 * otherwise, compiling errors, or linking errors will occur.
 *
 * \subsection inputs_outputs How do Callback Functions handle Inputs and Outputs?
 *
 * The prototype of pointerToFunction does not take any input parameters, but actual C functions must take one or
 * more input parameters, with various types, such as integer, double, or string.
 *
 * Test Manager maintains a number of containers for input parameters.  Users add input parameters as a pair of 
 * (keyword, value) into containers before calling the "Function Keyword" as shown below:
 *
 * \code
        TM_ClearParameters();
        TM_AddIntegerParameter(WiFi_Test, "FREQ_MHZ",      2437);
        TM_AddStringParameter (WiFi_Test, "BANDWIDTH",     "HT20");
        TM_AddStringParameter (WiFi_Test, "DATA_RATE_STR", "OFDM-54");
        TM_AddDoubleParameter (WiFi_Test, "TX_POWER_DBM",  13);
        TM_AddDoubleParameter (WiFi_Test, "CABLE_LOSS",    7);
        TM_AddDoubleParameter (WiFi_Test, "SAMPLING_TIME", 500e-6);
 
        err = TM_Run(WiFi_Test, "TX_VERIFY_EVM");
 * \endcode
 * 
 * The actual C function for "TX_VERIFY_EVM" will retrieve those input parameters.
 *
 * Test Manager also maintains a number of containers for return values, which are working in the opposite way to input
 * parameter containers.  In the actual C functions, return values are added into containers, and users will retrive those
 * return values upon running those functions successfully.
 *
 * \subsection query_input_and_return How do users know how many input parameters and return values?
 *
 * Each one of the actual C functions for "Function Keywords" shall support two special input parameter names:
 *      -# QUERY_INPUT: For users to query all required input parameters
 *      -# QUERY_RETURN: For users to query all return values
 * These two special input parameter names take priority over other parameter names.  Both are defined of Integer
 * type, although the data type does not matter.  If the Integer container contains either QUERY_INPUT, or QUERY_RETURN,
 * but not both, the C functions will put all required input parameters, or return values, into the return containers 
 * (Integer, Double, and String).  Users have to QUERY_INPUT and QUERY_RETURN individually.
 *
 * \section revision_sec Revision History
 *
 * \subsection revision_0_0_2 0.0.2
 * Date: Mar 16, 2009
 * - Release with introduction of FlowComposer
 *
 * \subsection revision_0_0_1 0.0.1
 * Date: Aug 13, 2008
 * - First draft
 */

/*! \file TestManager.h
 * \brief Test Manager Interface functions 
 */ 
#ifndef _TM_H_
#define _TM_H_

#include <float.h> // DBL_MAX on Mac

#if !defined (__cplusplus)
    #define TM_API extern
#elif !defined (WIN32)
    #define TM_API extern "C"
#elif defined(TM_EXPORTS)
    #define TM_API extern "C" __declspec(dllexport)
#else
    #define TM_API extern "C" __declspec(dllimport) 
#endif

#if !defined (WIN32)
    #define __stdcall
#endif

//#ifdef TM_EXPORTS
//
//    #define TM_API 
//
//#else
//
//    #define TM_API 
//
//#endif

#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif
#ifndef BOOL
    typedef int BOOL;
#endif
#ifndef MAX_BUFFER_SIZE
    #define MAX_BUFFER_SIZE 4096
#endif

#ifndef NA_NUMBER
    #define NA_NUMBER -99999.99
#endif

#ifndef NA_DOUBLE
	#define NA_DOUBLE -(DBL_MAX)
#endif

#ifndef NA_INTEGER
	#define NA_INTEGER -(INT_MAX)
#endif

#ifndef MAX_RESULT_NAME
    #define MAX_RESULT_NAME 35 /* 20110812 change 30 to 35 for MPS return parameter length increasement*/
#endif                         /*  from OFDM-54.PER_SWEEP.NUMBER_OF_POWER_LEVELS To OFDM-54.PER_SWEEP.NUMBER_OF_POWER_LEVELS.@15.00 */ 

#ifndef MAX_PARAM_RETURN
    #define MAX_PARAM_RETURN 50
#endif

#ifndef MAX_SEQ_STEPS
    #define MAX_SEQ_STEPS  256
#endif

#ifndef MAX_DATA_RATE_NUM
//    #define MAX_DATA_RATE_NUM 21
	#define MAX_DATA_RATE_NUM 45		// support up to MCS31
#endif

enum tagPathLossTable
{
    TX_TABLE,
    RX_TABLE,
    MAX_PATHLOSS_TABLE_NUM
};

typedef char * TM_STR;      /*!< An ANSI C string typedef for convenience in TM */
typedef signed int TM_ID;   /*!< An unsigned int typedef for convenience in TM */

//! Define the supported technologies
/*!
 * The following 7 technologies exist:
 *    - WIFI
 *    - BT
 *    - WIMAX
 *    - GPS
 *    - FM
 */
enum tagTechnology
{
    WIFI,
	WIFI_MIMO,
	WIFI_11AC,
	WIFI_11AC_MIMO,
	WIFI_MPS,
	BT,
    WIMAX,
    GPS,
    FM,
	IQREPORT,
    MAX_TECHNOLOGY_NUM
};

enum tagContainerRetrievalOrder
{
    FIRST,
    NEXT
};

/*! Error Code Definition
 */
typedef enum TM_EC
{
    TM_ERR_OK,                             /*!< Function completes with no errors*/
    TM_ERR_TECHNOLOGY_DOES_NOT_EXIST,      /*!< Specified technology is not one of the pre-defined technologies*/
    TM_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED, /*!< Another DUT has registered the specified technology*/
	TM_ERR_FUNCTION_NOT_SUPPORTED,         /*!< DUT does not support the specified function*/
	TM_ERR_FUNCTION_NOT_DEFINED,           /*!< The specified function has not been defined yet*/
	TM_ERR_PARAM_DOES_NOT_EXIST,           /*!< The specified parameter does not exist*/
    TM_ERR_INVALID_TECHNOLOGY_ID,          /*!< Invalid DUT ID*/
    TM_ERR_FUNCTION_ERROR,				   /*!< DUT function had errors*/
	TM_ERR_WIFI_FREQ_DOES_NOT_EXIST,       /*!< WiFi frequency does not exist*/
    TM_ERR_FAILED_TO_LOAD_TECHNOLOGY_DLL,         /*!< Failed to load the specified Test module DLL*/
    TM_ERR_NO_TECHNOLOGY_REGISTER_FUNCTION_FOUND, /*!< No register function found in the Test module DLL*/
    TM_ERR_TECHNOLOGY_REGISTER_FUNCTION_FAILED,   /*!< Register function in the Test module DLL failed*/
    TM_ERR_NO_VALUE_DEFINED,                      /*!< No values defined*/
    TM_ERR_FAILED_TO_OPEN_FILE,                   /*!< Failed to open file*/
    TM_ERR_DATARATE_DOES_NOT_EXIST,           /*!< The specified parameter does not exist*/
    TM_ERR_MEAS_TYPE_DOES_NOT_EXIST,            /*!< The specified measurement type does not exist*/

    TM_ERR_MAXIMUM_NUM              
} TM_RETURN;

typedef enum tagTmSeqMeasType
{
    TM_SEQ_MEAS_EVM,
    TM_SEQ_MEAS_MASK,
    TM_SEQ_MEAS_POWER,
    TM_SEQ_MEAS_SPECTRUM,
    TM_SEQ_MEAS_RAMP,
    TM_SEQ_MEAS_RX_PER_SWEEP,
    TM_SEQ_MEAS_RX_PER_TRGT_SENS, //for low power sens test
    TM_SEQ_MEAS_RX_PER_MAX_SENS,  //for max power sens test
    TM_MAX_SEQ_MEAS_NUM
}TM_SEQ_MEAS_TYPE;

typedef struct tagTmSeqResultValue
{
    int     numOfValues;
    double  values[MAX_SEQ_STEPS];
    char    unit[MAX_RESULT_NAME];
    char    paramName[MAX_RESULT_NAME];          
} TM_SEQ_PARAM_RETURN;

typedef struct tagTmSeqMeasResult
{
    int                 numOfResults;
    TM_SEQ_PARAM_RETURN results[MAX_PARAM_RETURN];            
} TM_SEQ_ONE_MEAS_TYPE_RESULTS;
 
// //Sequence return structure
// typedef struct tagTmSeqMeasTypeResult
// {
//     int                 numOfMeasResult;
//     TM_SEQ_MEAS_TYPE    measType;
//     TM_SEQ_ONE_MEAS_TYPE_RESULTS measResults[MAX_SWEEP_STEPS];                   
// }TM_SEQ_MEAS_TYPE_RESULTS;
// 
// //Sequence return structure
// typedef struct tagTmSeqOneDataRateResult
// {
//     int                      numOfMeasType;
//     char                     dataRate[MAX_RESULT_NAME];
//     TM_SEQ_MEAS_TYPE_RESULTS measTypeResults[MAX_SWEEP_STEPS];                   
// }TM_SEQ_ONE_DATA_RATE_RESULTS;

//! Register a specific wireless test, such as WIFI, BT, WIMAX, etc.
/*!
 *
 * \param[in] technology Name of the technology to register 
 * \param[out] *technologyID A pointer to TM_ID that will return the ID of the test that has been registered
 *
 * \return TM_ERR_OK if the specified Technology Name is registered successfully
 * \return TM_ERR_TECHNOLOGY_DOES_NOT_EXIST if the specified technology is not one of the pre-defined technologies
 * \return TM_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED if the specified technology had been registered by others
 *
 */
TM_API TM_RETURN __stdcall TM_RegisterTechnology(const TM_STR technology, TM_ID *technologyID);

//! Unregister a wireless test
/*!
 * \param[out] technology Technology NAME that had been registered  
 *
 * \return TM_ERR_OK if the wireless test has been unregistered
 */
TM_API TM_RETURN __stdcall TM_UnregisterTechnology(const TM_STR technology);

//! Register a Technology DLL for a specific technology, such as WIFI, BT, WIMAX, etc.
/*!
 *
 * \param[in] technology Name to register for
 * \param[in] technologyDllFileName Specifies the Test module DLL file name
 * \param[out] *technologyID Returns the ID of the technology that has been registered
 *
 * \return TM_ERR_OK if the specified Test module DLL has been registered for the technology
 * \return TM_ERR_TECHNOLOGY_DOES_NOT_EXIST if the specified technology is not one of the pre-defined technologies
 * \return TM_ERR_TECHNOLOGY_HAS_BEEN_REGISTERED if the specified technology has been registered by another Test module
 * \return TM_ERR_FAILED_TO_LOAD_TECHNOLOGY_DLL
 * \return TM_ERR_NO_TECHNOLOGY_REGISTER_FUNCTION_FOUND
 * \return TM_ERR_TECHNOLOGY_REGISTER_FUNCTION_FAILED
 *
 * \remark TestManager supports the concept of multiple technologies, or Multicom, to allow multiple Test modules
 * to exist in the same solution.  Each technology will have zero, or one Test module to be registered for, and one 
 * Test module can only register for one technology.
 *
 * TM_RegisterTechnologyDll() differs from TM_RegisterTechnology() as follows:
 *      -# TM_RegisterTechnology() was made to be called in a particular Test module.  In this way,
 *         the Test module has to expose one function to be called explicitly in user application.
 *      -# TM_RegisterTechnologyDll() is meant to be called directly from user application.  By doing so, users
 *         can easily select any Test module by specifying a DLL name.  More importantly, this is happening
 *         during run-time stage.
 *         However, each Test module DLL has to export one function, following the below name convention:
 *              - int <TECHNOLOGY>_TechnologyRegister(void)
 *          where <TECHNOLOGY> is one of the pre-defined technologies in TestManager, listed below:
 *              - WIFI
 *              - BT
 *              - WIMAX
 *              - FM
 *              - GPS
 */
TM_API TM_RETURN __stdcall TM_RegisterTechnologyDll(const TM_STR technology, TM_STR technologyDllFileName, TM_ID *technologyID);

//! Unregister a Technology DLL
/*!
 * \param[out] technology name with a Test module registered.  
 * \param[in,out] *technologyID Points to the Test module ID that has been registered previously.  It'll be set to -1.
 *
 * \return TM_ERR_OK if the technology has been unregistered
 * \remark After TM_UnregisterTechnologyDll(), the same technology can be registered with different Test module
 */
TM_API TM_RETURN __stdcall TM_UnregisterTechnologyDll(const TM_STR technology, TM_ID *technologyID);

//! Get Technology Information managed by Test Manager
/*!
 * \param[out] technologyName Returns the technology name  
 * \param[in] bufferSize Indicates the buffer size of technologuName  
 * \param[out] technologyIndex Returns the technology index.  Valid values are enums defined in enum tagTechnology  
 * \param[out] testFunctionCount Returns the number of pre-defined Function Keywords
 * \param[in] order Indicates the order of calling this function.  The very first call uses FIRST, and following calls use NEXT  
 *
 * \return TM_ERR_OK if no errors
 *
 * \remark Users need to call this function in a while loop until it returns a value other than TM_ERR_OK.  The very first
 * call specifies order as FIRST, and the following calls specify NEXT until the function returns non TM_ERR_OK value.
 */
TM_API TM_RETURN __stdcall TM_GetTechnologyInfo(char *technologyName, 
                                                int bufferSize, 
                                                int *technologyIndex, 
                                                int *testFunctionCount, 
                                                int order);

//! Get the index for specified Technology name
/*!
 * \param[in] technologyName Specifies Technology name  
 * \param[out] technologyIndex Returns the technology index.  Valid values are enums defined in enum tagTechnology  
 * \param[out] testFunctionCount Returns the number of pre-defined Function Keywords
 *
 * \return TM_ERR_OK if no errors
 */
TM_API TM_RETURN __stdcall TM_GetTechnologyIndex(const char *technologyName, 
                                                 int *technologyIndex, 
                                                 int *testFunctionCount);

TM_API TM_RETURN __stdcall TM_GetFunctionKeyword(const char* technologyName, 
                                                 char *functionKeyword, 
                                                 int bufferSize, 
                                                 BOOL *callbackInstalled, 
                                                 int order);

//! Install a callback function for a specified function name
/*!
 * \param[in] dutID The registered DUT ID by vDUT_RegisterTechnology()
 * \param[in] functionName The function name (one of the pre-defined function names)
 * \param[in] pointerToFunction The callback function (pointer to function)
 *
 * \return vDUT_ERR_OK if the callback function has been "installed" to the specified function name
 * \return vDUT_ERR_INVALID_DUT_ID The specified DUT ID is invalid
 * \return vDUT_ERR_FUNCTION_NOT_DEFINED The specified function name is not defined
 *
 * \remark The C/C++ complier will guarauntee that the callback function has been implemented.
 */
TM_API TM_RETURN __stdcall TM_InstallCallbackFunction(TM_ID technologyID, const TM_STR functionKeyword, int (*pointerToFunction)(void) );

//! Run a Test function
/*!
 * \param[in] technologyID The registered technology ID
 * \param[in] functionKeyword The function name (one of the pre-defined function names)
 *
 * \return TM_ERR_OK if no errors
 * \return TM_ERR_INVALID_TECHNOLOGY_ID The specified Technology ID is invalid
 * \return TM_ERR_FUNCTION_NOT_DEFINED The specified function name is not defined
 * \return TM_ERR_FUNCTION_NOT_SUPPORTED The specified function name does not have a callback function installed
 * \return TM_ERR_FUNCTION_ERROR The function generated error(s)
 *
 */
TM_API TM_RETURN __stdcall TM_Run(TM_ID technologyID, const TM_STR functionKeyword);

//! Clear all input parameter containers for the specified technology
/*!
 * \param[in] technologyID The registered technology ID
 *
 * \return TM_ERR_OK if no errors
 * \return TM_ERR_INVALID_TECHNOLOGY_ID The specified Technology ID is invalid
 */
TM_API TM_RETURN __stdcall TM_ClearParameters(TM_ID technologyID);

TM_API TM_RETURN __stdcall TM_AddIntegerParameter(TM_ID technologyID, const TM_STR paramName, const int paramValue);
TM_API TM_RETURN __stdcall TM_AddDoubleParameter(TM_ID technologyID, const TM_STR paramName, const double paramValue);
TM_API TM_RETURN __stdcall TM_AddStringParameter(TM_ID technologyID, const TM_STR paramName, const TM_STR paramValue);
TM_API TM_RETURN __stdcall TM_GetIntegerParameter(TM_ID technologyID, const TM_STR paramName, int* paramValue);
TM_API TM_RETURN __stdcall TM_GetDoubleParameter(TM_ID technologyID, const TM_STR paramName, double* paramValue);
TM_API TM_RETURN __stdcall TM_GetStringParameter(TM_ID technologyID, const TM_STR paramName, TM_STR paramValue, int bufferSize);
TM_API TM_RETURN __stdcall TM_SetDoubleParameter(TM_ID technologyID, const TM_STR paramName, double paramValue);

//! Clear all return value containers for the specified technology
/*!
 * \param[in] technologyID The registered technology ID
 *
 * \return TM_ERR_OK if no errors
 * \return TM_ERR_INVALID_TECHNOLOGY_ID The specified Technology ID is invalid
 */
TM_API TM_RETURN __stdcall TM_ClearReturns(TM_ID technologyID);
TM_API TM_RETURN __stdcall TM_AddIntegerReturn(TM_ID technologyID, const TM_STR paramName, const int paramValue);
TM_API TM_RETURN __stdcall TM_AddDoubleReturn(TM_ID technologyID, const TM_STR paramName, const double paramValue);
TM_API TM_RETURN __stdcall TM_AddStringReturn(TM_ID technologyID, const TM_STR paramName, const TM_STR paramValue);
TM_API TM_RETURN __stdcall TM_AddArrayDoubleReturn(TM_ID technologyID, const TM_STR paramName, const double *paramValue, const int arraySize);

TM_API TM_RETURN __stdcall TM_AddUnitReturn(TM_ID technologyID, const TM_STR paramName, const TM_STR unit);
TM_API TM_RETURN __stdcall TM_GetUnitReturn(TM_ID technologyID, const TM_STR paramName, TM_STR unitValue, int bufferSize);
TM_API TM_RETURN __stdcall TM_AddHelpTextReturn(TM_ID technologyID, const TM_STR paramName, const TM_STR helpText);

TM_API TM_RETURN __stdcall TM_GetIntegerReturn(TM_ID technologyID, const TM_STR paramName, int* paramValue);
TM_API TM_RETURN __stdcall TM_GetDoubleReturn(TM_ID technologyID, const TM_STR paramName, double* paramValue);
TM_API TM_RETURN __stdcall TM_GetStringReturn(TM_ID technologyID, const TM_STR paramName, TM_STR paramValue, int bufferSize);
TM_API TM_RETURN __stdcall TM_GetArrayDoubleReturnSize(TM_ID technologyID, const TM_STR paramName,int* arraySize);
TM_API TM_RETURN __stdcall TM_GetArrayDoubleReturn(TM_ID technologyID, const TM_STR paramName, double *paramValue, const int arraySize);

TM_API TM_RETURN __stdcall TM_GetSeqDataRateReturn(TM_ID technologyID, int *numOfDataRate, TM_STR *dataRateList);
TM_API TM_RETURN __stdcall TM_GetSeqOneDataRateMeasTypeReturn(TM_ID technologyID, const TM_STR dataRate, int *numOfMeasType, TM_SEQ_MEAS_TYPE *measTypeList);
TM_API TM_RETURN __stdcall TM_GetSeqOneMeasTypeReturn(TM_ID technologyID, const TM_STR dataRate, TM_SEQ_MEAS_TYPE measType, TM_SEQ_ONE_MEAS_TYPE_RESULTS *seqOneMeasTypeResults);
TM_API TM_RETURN __stdcall TM_ApplyDataRateMeasTypeReturn(TM_ID technologyID, const TM_STR dataRate, TM_SEQ_MEAS_TYPE measType, const TM_STR paramName, const double *paramValue, const int arraySize, const TM_STR unit);
 


//! Get details of one item in the Integer Param Container
/*!
 * \param[in] technologyID The registered technology ID
 * \param[out] paramName Returns the item name
 * \param[in] bufferSize Specifies the buffer size of paramName
 * \param[out] unit Returns the unit.  If no need to return unit, specify NULL, and unitSize will be ignored.
 * \param[in] unitSize Specifies the buffer size of unit
 * \param[out] helpText Returns the help text.  If no need to return help, specify NULL, and helpTextSize will be ignored.
 * \param[in] helpTextSize Specifies the buffer size of helpText
 * \param[out] paramValue Returns the value..  If no need to return the value, specify NULL.
 *
 * \return TM_ERR_OK if no errors
 * \return TM_ERR_INVALID_TECHNOLOGY_ID The specified Technology ID is invalid
 */
TM_API TM_RETURN __stdcall TM_GetIntegerParamPair(TM_ID technologyID, 
                                                  TM_STR paramName, 
                                                  int bufferSize, 
                                                  TM_STR unit, 
                                                  int unitSize, 
                                                  TM_STR helpText, 
                                                  int helpTextSize, 
                                                  int* paramValue, 
                                                  int order);

//! Get details of one item in the Integer Return Container
/*!
 * \param[in] technologyID The registered technology ID
 * \param[out] paramName Returns the item name
 * \param[in] bufferSize Specifies the buffer size of paramName
 * \param[out] unit Returns the unit.  If no need to return unit, specify NULL, and unitSize will be ignored.
 * \param[in] unitSize Specifies the buffer size of unit
 * \param[out] helpText Returns the help text.  If no need to return help, specify NULL, and helpTextSize will be ignored.
 * \param[in] helpTextSize Specifies the buffer size of helpText
 * \param[out] paramValue Returns the value..  If no need to return the value, specify NULL.
 *
 * \return TM_ERR_OK if no errors
 * \return TM_ERR_INVALID_TECHNOLOGY_ID The specified Technology ID is invalid
 */
TM_API TM_RETURN __stdcall TM_GetIntegerReturnPair(TM_ID technologyID, 
                                                   TM_STR paramName, 
                                                   int bufferSize, 
                                                   TM_STR unit, 
                                                   int unitSize, 
                                                   TM_STR helpText, 
                                                   int helpTextSize, 
                                                   int* paramValue, 
                                                   int order);

//! Get details of one item in the Double Param Container
/*!
 * \param[in] technologyID The registered technology ID
 * \param[out] paramName Returns the item name
 * \param[in] bufferSize Specifies the buffer size of paramName
 * \param[out] unit Returns the unit.  If no need to return unit, specify NULL, and unitSize will be ignored.
 * \param[in] unitSize Specifies the buffer size of unit
 * \param[out] helpText Returns the help text.  If no need to return help, specify NULL, and helpTextSize will be ignored.
 * \param[in] helpTextSize Specifies the buffer size of helpText
 * \param[out] paramValue Returns the value..  If no need to return the value, specify NULL.
 *
 * \return TM_ERR_OK if no errors
 * \return TM_ERR_INVALID_TECHNOLOGY_ID The specified Technology ID is invalid
 */
TM_API TM_RETURN __stdcall TM_GetDoubleParamPair(TM_ID technologyID, 
                                                 TM_STR paramName, 
                                                 int bufferSize, 
                                                 TM_STR unit, 
                                                 int unitSize, 
                                                 TM_STR helpText, 
                                                 int helpTextSize, 
                                                 double* paramValue, 
                                                 int order);

//! Get details of one item in the Double Return Container
/*!
 * \param[in] technologyID The registered technology ID
 * \param[out] paramName Returns the item name
 * \param[in] bufferSize Specifies the buffer size of paramName
 * \param[out] unit Returns the unit.  If no need to return unit, specify NULL, and unitSize will be ignored.
 * \param[in] unitSize Specifies the buffer size of unit
 * \param[out] helpText Returns the help text.  If no need to return help, specify NULL, and helpTextSize will be ignored.
 * \param[in] helpTextSize Specifies the buffer size of helpText
 * \param[out] paramValue Returns the value..  If no need to return the value, specify NULL.
 *
 * \return TM_ERR_OK if no errors
 * \return TM_ERR_INVALID_TECHNOLOGY_ID The specified Technology ID is invalid
 */
TM_API TM_RETURN __stdcall TM_GetDoubleReturnPair(  TM_ID technologyID, 
													TM_STR paramName, 
													int bufferSize, 
													TM_STR unit, 
													int unitSize, 
													TM_STR helpText, 
													int helpTextSize, 
													double* paramValue, 
													int order);

//! Get details of one item in the String Param Container
/*!
 * \param[in] technologyID The registered technology ID
 * \param[out] paramName Returns the item name
 * \param[in] bufferSize Specifies the buffer size of paramName
 * \param[out] unit Returns the unit.  If no need to return unit, specify NULL, and unitSize will be ignored.
 * \param[in] unitSize Specifies the buffer size of unit
 * \param[out] helpText Returns the help text.  If no need to return help, specify NULL, and helpTextSize will be ignored.
 * \param[in] helpTextSize Specifies the buffer size of helpText
 * \param[out] paramValue Returns the value..  If no need to return the value, specify NULL.
 * \param[in] paramValueBufferSize Specifies the buffer size of paramValue
 *
 * \return TM_ERR_OK if no errors
 * \return TM_ERR_INVALID_TECHNOLOGY_ID The specified Technology ID is invalid
 */
TM_API TM_RETURN __stdcall TM_GetStringParamPair(TM_ID technologyID, 
                                                 TM_STR paramName, 
                                                 int bufferSize, 
                                                 TM_STR unit, 
                                                 int unitSize, 
                                                 TM_STR helpText, 
                                                 int helpTextSize, 
                                                 TM_STR paramValue,
                                                 int paramValueBufferSize,
                                                 int order);

//! Get details of one item in the String Return Container
/*!
 * \param[in] technologyID The registered technology ID
 * \param[out] paramName Returns the item name
 * \param[in] bufferSize Specifies the buffer size of paramName
 * \param[out] unit Returns the unit.  If no need to return unit, specify NULL, and unitSize will be ignored.
 * \param[in] unitSize Specifies the buffer size of unit
 * \param[out] helpText Returns the help text.  If no need to return help, specify NULL, and helpTextSize will be ignored.
 * \param[in] helpTextSize Specifies the buffer size of helpText
 * \param[out] paramValue Returns the value..  If no need to return the value, specify NULL.
 * \param[in] paramValueBufferSize Specifies the buffer size of paramValue
 *
 * \return TM_ERR_OK if no errors
 * \return TM_ERR_INVALID_TECHNOLOGY_ID The specified Technology ID is invalid
 */
TM_API TM_RETURN __stdcall TM_GetStringReturnPair(	TM_ID technologyID, 
													TM_STR paramName, 
													int bufferSize, 
													TM_STR unit, 
													int unitSize, 
													TM_STR helpText, 
													int helpTextSize, 
													TM_STR paramValue,
													int paramValueBufferSize,
													int order);

//! Get details of one item in the Array Return Container
/*!
 * \param[in] technologyID The registered technology ID
 * \param[out] paramName Returns the item name
 * \param[in] bufferSize Specifies the buffer size of paramName
 * \param[out] unit Returns the unit.  If no need to return unit, specify NULL, and unitSize will be ignored.
 * \param[in] unitSize Specifies the buffer size of unit
 * \param[out] helpText Returns the help text.  If no need to return help, specify NULL, and helpTextSize will be ignored.
 * \param[in] helpTextSize Specifies the buffer size of helpText
 * \param[out] paramValue Returns the value..  If no need to return the value, specify NULL.
 * \param[in] paramValueBufferSize Specifies the buffer size of paramValue
 *
 * \return TM_ERR_OK if no errors
 * \return TM_ERR_INVALID_TECHNOLOGY_ID The specified Technology ID is invalid
 */
TM_API TM_RETURN __stdcall TM_GetArrayDoubleReturnPair(TM_ID technologyID, 
                                                       TM_STR paramName, 
                                                       int bufferSize, 
                                                       TM_STR unit, 
                                                       int unitSize, 
                                                       TM_STR helpText, 
                                                       int helpTextSize, 
                                                       double* paramValue,
                                                       int order);

//! Return the description for the specified error code
/*!
 *
 * \param[in] errorNum Error number returned by any TM functions 
 *
 * \return A pointer to the error description (ANSI C String)
 *
 */
TM_API char* __stdcall TM_GetErrorString(TM_RETURN errorNum);

//! Convert WiFi frequency (MHz) to channel number
/*!
 *
 * \param[in] freq, Frequency in MHz 
 * \param[out] channel, The channel number corresponding to the specified frequency 
 *
 * \return TM_ERR_OK If the specified frequency has been converted to channel number
 *
 */
TM_API TM_RETURN __stdcall TM_WiFiConvertFrequencyToChannel(int freq, int* channel);

//! Convert WiFi channel to frequency (MHz) number
/*!
 *
 * \param[in]  channel, The channel number 
 * \param[out] freq, Frequency in MHz corresponding to the specified channel 
 *
 * \return TM_ERR_OK If the channel number has been converted to frequency
 *
 */
TM_API TM_RETURN __stdcall TM_WiFiConvertChannelToFrequency(int channel, int *freq);

//! Convert WiFi data rate to index
/*!
 *
 * \param[in] name The name of Data Rate, such as "DSSS-1", "CCK-11", "OFDM-54", "MCS7", "MCS15", etc. 
 * \param[out] index The corresponding index for the specified data rate 
 *
 * \return TM_ERR_OK If the specified data rate has been found
 *
 */
TM_API TM_RETURN __stdcall TM_WiFiConvertDataRateNameToIndex(TM_STR name, int* index);

//! Convert WiFi index to data rate
/*!
 *
 * \param[in] index The corresponding index for the specified data rate
 * \param[in] helpTextSize Specifies the buffer size of helpText
 * \param[out] name The name of Data Rate, such as "DSSS-1", "CCK-11", "OFDM-54", "MCS7", "MCS15", etc.  
 *
 * \return TM_ERR_OK If the specified data rate has been found
 *
 */
TM_API TM_RETURN __stdcall TM_WiFiConvertIndexToDataRateName(int index, TM_STR name, int nameTextSize);

//! Convert WiFi data rate to Mbps
/*!
 *
 * \param[in] name The name of Data Rate, such as "DSSS-1", "CCK-11", "OFDM-54", "MCS7", "MCS15", etc. 
 * \param[out] dataRateMbps The corresponding Mbps for the specified data rate 
 *
 * \return TM_ERR_OK If the specified data rate has been found
 *
 */
TM_API TM_RETURN __stdcall TM_WiFiConvertDataRateNameToMbps(TM_STR name, double* dataRateMbps);

//! Convert WiFi data rate to IQ2010Ext Index
/*!
 *
 * \param[in] name The name of Data Rate, such as "DSSS-1", "CCK-11", "OFDM-54", "MCS7" etc. 
 * \param[out] dataRateIndex The corresponding index for the specified data rate 
 *
 * \return TM_ERR_OK If the specified data rate index has been found
 *
 */
TM_API TM_RETURN __stdcall TM_WiFiConvertDataRateNameToIQ2010ExtIndex(TM_STR name, int* dataRateIndex);

//! Get Test Manager version information
/*!
 *
 * \param[out] paramValue Parameter value 
 * \param[in] bufferSize Indicates the size of paramValue
 *
 * \return TM_ERR_OK If the Test Manager version information has been found
 *
 */
TM_API TM_RETURN __stdcall TM_GetVersion(TM_STR paramValue, int bufferSize);


//! Update path loss at specified frequency
/*!
 *
 * \param[in] technologyID The registered technology ID 
 * \param[in] freqMHz The specified frequency (MHz) 
 * \param[in] pathLoss An array of path loss
 * \param[in] pathLossCount Indicates the count of path loss array
 * \param[in] indicatorTxRx An indicator to indicate the Tx or Rx table, Tx is 0 and Rx is 1.
 *
 * \remark A number of path loss values can be specified for any given frequency.
 *         The order of those values corresponds to DUT's RF TX/RX paths.
 *         If DUT has only one RF path, just one path loss value is needed.
 *         For a 2x2 MIMO DUT, two values are needed, while 4 values will be needed
 *         for a 4x4 MIMO DUT.
 *
 * \return TM_ERR_OK if no errors
 *
 */
TM_API TM_RETURN __stdcall TM_UpdatePathLossAtFrequency(TM_ID	technologyID,
                                                        int		freqMHz,
                                                        double *pathLoss,
                                                        int		pathLossCount,
														int		indicatorTxRx);


//! Update path loss as specified in a text file
/*!
 *
 * \param[in] technologyID The registered technology ID 
 * \param[in] pathLossFileName A text file that contains path loss information
 * \param[in] indicatorTxRx An indicator to indicate the Tx or Rx table, Tx is 0 and Rx is 1.
 *
 * \remark For easy editing, the comma-delimited .csv format is chosen for storing
 *         the path loss table, which may look like:
 * \code

 //Path Loss Table,,,,
2412,2,2,2,2
5180,3,3,3,3
5825,4,4,4,4

 * \endcode
 * The first number is frequency (MHz), followed by a number of path loss values, which correspond to 
 * DUT's RF TX/RX paths.
 *
 * This function will clear all path loss information before loading the specified file.
 *
 * \return TM_ERR_OK if no errors
 *
 */
TM_API TM_RETURN __stdcall TM_UpdatePathLossByFile(TM_ID technologyID, TM_STR pathLossFileName, int indicatorTxRx);


//! Update path loss at specified frequency
/*!
 *
 * \param[in] technologyID The registered technology ID 
 * \param[in] freqMHz The specified frequency (MHz) 
 * \param[in] pathLoss Path loss to be returned
 * \param[in] pathLossIndex Specifies the index of path for which the path loss will be returned
 * \param[in] indicatorTxRx An indicator to indicate the Tx or Rx table, Tx is 0 and Rx is 1.
 *
 * \remark For DUT that has only one TX/RX path, pathLossIndex would be always 0;
 *         For a 2x2 MIMO DUT, pathLossIndex could be 0 or 1;
 *         Similarly, for a 4x4 DUT, pathLossIndex could be 0, 1, 2, or 3.
 *
 * \return TM_ERR_OK if no  errors
 *
 */
TM_API TM_RETURN __stdcall TM_GetPathLossAtFrequency(TM_ID  technologyID,
                                                     int	freqMHz,
                                                     double *pathLoss,
                                                     int	pathLossIndex,
													 int	indicatorTxRx);

//! Get Device Under Test Information
/*!
 *
 * \param[out] dutSerialNumber device serial number
 * \param[in] dutSerialNumberBufferSize Indicates the size of dutSerialNumber
 * \param[out] dutManufacturer device manufacturer
 * \param[in] dutManufacturerBufferSize Indicates the size of dutManufacturer
 * \param[out] dutModel device model
 * \param[in] dutModelBufferSize Indicates the size of dutModel
 * \param[out] dutRevision device revision
 * \param[in] dutRevisionBufferSize Indicates the size of dutRevision
 * \param[out] dutDescription device description
 * \param[in] dutDescriptionBufferSize Indicates the size of dutDescription
 *
 * \return TM_ERR_OK If the DUT Serial Number information has been found
 *
 */
TM_API TM_RETURN __stdcall TM_GetDutInfo(TM_STR dutSerialNumber, int dutSerialNumberBufferSize,
                                         TM_STR dutManufacturer, int dutManufacturerBufferSize,
                                         TM_STR dutModel, int dutModelBufferSize,
                                         TM_STR dutRevision, int dutRevisionBufferSize,
                                         TM_STR dutDescription, int dutDescriptionBufferSize);

//! Set Device Under Test Information
/*!
 *
 * \param[in] dutSerialNumber device serial number
 * \param[in] dutManufacturer device manufacturer
 * \param[in] dutModel device model
 * \param[in] dutRevision device revision
 * \param[in] dutDescription device description
 *
 * \return TM_ERR_OK If the DUT Serial Number information has been set
 *
 */
TM_API TM_RETURN __stdcall TM_SetDutInfo(TM_STR deviceSerialNumber,
                                         TM_STR deviceManufacturer = "",
                                         TM_STR deviceModel = "",
                                         TM_STR deviceRevision = "",
                                         TM_STR deviceDescription = "");



//! Check the Multi-Segment Waveform Index Map
/*!
 *
 * \param[out] mapIsEmpty an indicator of map is empty or not
 *
 * \return TM_ERR_OK if no errors
 *
 */
TM_API TM_RETURN __stdcall TM_CheckMultiWaveformIndexMap(bool *mapIsEmpty);


//! Clear the Multi-Segment Waveform Index Map
/*!
 * \return TM_ERR_OK if no errors
 */
TM_API TM_RETURN __stdcall TM_ClearMultiWaveformIndexMap(void);


//! Add Multi-Segment Waveform name with index into map
/*!
 *
 * \param[in] waveformName A keyword string of waveform name or data rate name 
 * \param[in] index	The waveform index used by IQ2010 extension function identifying its data rate
 *
 * \return TM_ERR_OK if no errors
 *
 */
TM_API TM_RETURN __stdcall TM_AddMultiWaveformWithIndex(TM_STR waveformName, int index);

//! Get Multi-Segment Waveform index from map
/*!
 *
 * \param[in] waveformName A keyword string of waveform name or data rate name 
 * \param[out] index	The waveform index used by IQ2010 extension function identifying its data rate
 *
 * \return TM_ERR_OK if no errors
 *
 */
TM_API TM_RETURN __stdcall TM_GetMultiWaveformIndex(TM_STR waveformName, int *index);

//! Apply dataRate measure results to the result map
/*!
 *
 * \param[in] dataRate A keyword string of data rate name 
 * \param[out] enum measure type (evm, mask ...)
 *
 * \return TM_ERR_OK if no errors
 *
 */
TM_API TM_RETURN __stdcall TM_GetSeqMeasureResults(const TM_STR dataRate, int mpsMeasureType);

#endif

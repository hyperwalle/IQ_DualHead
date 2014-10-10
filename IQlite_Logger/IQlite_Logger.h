/*! \mainpage Logger Utility DLL for IQlite
 *
 * \section intro_sec Introduction
 *
 * \section revision_sec Revision History
 *
 * \subsection revision_0_0_2 0.0.2
 * Date: Mar 16, 2009
 * - First release
 *
 * \subsection revision_0_0_1 0.0.1
 * Date: TBD
 * - First draft
 */

/*! \file IQlite_Logger.h
 * \brief Logger Utility DLL for IQlite
 */ 


#ifndef _IQTLITE_LOGGER_H_
#define _IQTLITE_LOGGER_H_

#if !defined (__cplusplus)
    #define IQLITE_LOGGER_API extern
#elif !defined (WIN32)
    #define IQLITE_LOGGER_API extern "C"
#elif defined(IQLITE_LOGGER_EXPORTS)
    #define IQLITE_LOGGER_API extern "C" __declspec(dllexport)
#else
    #define IQLITE_LOGGER_API extern "C" __declspec(dllimport) 
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
#ifndef MAX_BUFFER_SIZE
    #define MAX_BUFFER_SIZE 4096
#endif
#ifndef MAX_LOGGER_SIZE
    #define MAX_LOGGER_SIZE 200
#endif
#ifndef NA_NUMBER
    #define NA_NUMBER -99999.99
#endif

typedef enum tagLOGGER_LEVEL
{
    LOGGER_NONE,
    LOGGER_ERROR,
    LOGGER_WARNING,
    LOGGER_INFORMATION               
} LOGGER_LEVEL;

typedef enum tagLOGGER_SOURCE
{
    LOG_IQLITE_CORE,
	LOG_IQLITE_TM,
	LOG_IQLITE_VDUT,
	LOG_IQMEASURE,
    LOG_DUT_DLL,
    LOG_WIFIMPS_RESULT_FORMAT,
    LOG_SHOW_POWER_LEVEL,
	LOG_DUT1,
	LOG_DUT2,
	LOG_DUT3,
	LOG_DUT4,
	LOG_SCPI_SEPARATED,
    MAX_LOGGER_SOURCE              
} LOGGER_SOURCE;

/*! Error Code Definition
 */
typedef enum LOGGER_EC
{
    LOGGER_ERR_OK,                             /*!< Function completeds with no errors*/
	LOGGER_ERR_FUNCTION_NOT_SUPPORTED,         /*!< Timer does not support the specified function*/
	LOGGER_ERR_FUNCTION_NOT_DEFINED,           /*!< The specified function has not been defined yet*/
	LOGGER_ERR_PARAM_DOES_NOT_EXIST,           /*!< The specified parameter does not exist*/
    LOGGER_ERR_INVALID_LOGGER_ID,              /*!< Invalid Logger ID*/
    LOGGER_ERR_LOGGER_ID_OVER_RANGE,           /*!< Over Max_Logger ID range*/
    LOGGER_ERR_INVALID_SUPERVISOR_ID,          /*!< Invalid Supervisor ID*/    
	LOGGER_ERR_FAILED_TO_CREATE_DIRECTORY,	   /*!< Failed to create the logger directory*/
    LOGGER_ERR_LOGGER_FUNCTION_ERROR,          /*!< Logger function had errors*/                  
} LOGGER_RETURN;


//! IQlite_Logger Create Logger function
/*!
 *
 * \param[in] char* loggerName, a name for create a logger  
 * \param[out] int *loggerID, function return a logger ID
 * \param[in] char* specificFileName=NULL, if user don't want to use specific log file name, can ignore this parameter.  
 *
 * \return LOGGER_ERR_OK If initial timer successful
 *
 */
IQLITE_LOGGER_API LOGGER_RETURN LOGGER_CreateLogger(char* loggerName, int *loggerID, char* specificFileName=0);

//! IQlite_Logger write Log function
/*!
 *
 * \param[in] int loggerID, a ID to identify specific logger
 * \param[in] LOGGER_LEVEL level, four levels as a filter to log the message, includes LOGGER_NONE, LOGGER_ERROR, LOGGER_WARNING, LOGGER_INFORMATION   
 * \param[in] const char *format, the log message
 *
 * \return LOGGER_ERR_OK If initial timer successful
 *
 */
IQLITE_LOGGER_API LOGGER_RETURN LOGGER_Write(int loggerID, LOGGER_LEVEL level, const char *format, ...);
IQLITE_LOGGER_API LOGGER_RETURN LOGGER_Write2(int loggerID, LOGGER_LEVEL level, int color, const char *format, ...);

//! IQlite_Logger write Log function (Ext)
/*!
 *
 * \param[in] int loggerSource, a flag to indicate that logger source
 * \param[in] int loggerID, a ID to identify specific logger
 * \param[in] LOGGER_LEVEL level, four levels as a filter to log the message, includes LOGGER_NONE, LOGGER_ERROR, LOGGER_WARNING, LOGGER_INFORMATION   
 * \param[in] const char *format, the log message
 *
 * \return LOGGER_ERR_OK If initial timer successful
 *
 */
IQLITE_LOGGER_API LOGGER_RETURN LOGGER_Write_Ext(LOGGER_SOURCE loggerSource, int loggerID, LOGGER_LEVEL level, const char *format, ...);

//! IQlite_Logger set Log Level function
/*!
 *
 * \param[in] LOGGER_LEVEL level, four levels as a filter to log the message, includes LOGGER_NONE, LOGGER_ERROR, LOGGER_WARNING, LOGGER_INFORMATION   
 *
 *
 * \return LOGGER_ERR_OK If initial timer successful
 *
 */
IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetLogLevel(LOGGER_LEVEL level);

//! IQlite_Logger set Log Input Info Level function
/*!
*
* \param[in] LOGGER_LEVEL level, includes 0, OFF, 1, ON determins the level to set, either LOGGER_NONE, LOGGER_ERROR
*
*
* \return LOGGER_ERR_OK If initial timer successful
*
*/IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetInputInfoLevel(LOGGER_LEVEL level);

//! IQlite_Logger initialize IQlite logger and reading the IQlite_logger.ini setting
/*!
*
* \param[in] None
*
*
* \return LOGGER_ERR_OK If initial timer successful
*
*/
IQLITE_LOGGER_API LOGGER_RETURN LOGGER_Initialization(void);

//! IQlite_Logger get Log Level function
/*!
*
* \param[in] none
*
*
* \return LOGGER_LEVEL level, four levels as a filter to log the message, includes LOGGER_NONE, LOGGER_ERROR, LOGGER_WARNING, LOGGER_INFORMATION   
*
*/IQLITE_LOGGER_API LOGGER_LEVEL LOGGER_GetLogLevel(void);

//! IQlite_Logger get Log Input Info Level function
/*!
*
* \param[in] none
*
*
* \return LOGGER_LEVEL level, LOGGER_NONE, LOGGER_ERROR to determine whether or not to print input info information to the log 
*
*/IQLITE_LOGGER_API LOGGER_LEVEL LOGGER_GetLogInputInfoLevel(void);

//! IQlite_Logger get Logger Source Level
/*!
*
* \param[in] LOGGER_SOURCE loggerSoure takes value from LOG_IQLITE_CORE,LOG_IQLITE_TM, LOG_IQLITE_VDUT, LOG_IQMEASURE, LOG_DUT_DLL, MAX_LOGGER_SOURCE;
* \param[in] int* pIntVal, return the int value (*intVal) of the indicated LOGGER_SOURCE
*
* \return LOGGER_ERR_OK 
*
*/IQLITE_LOGGER_API LOGGER_RETURN LOGGER_GetLoggerSourceLevel(LOGGER_SOURCE loggerSoure, int* pIntVal);



//! IQlite_Logger set callback function for Dut1
/*!
 *
 * \param[in] The callback function will be call when outputing log
 *
 *
 * \return LOGGER_ERR_OK
 *
 */
IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetLogCallBackDut1(void (__stdcall * pFunc) (const char*));

//! IQlite_Logger set callback function for Dut2
/*!
 *
 * \param[in] The callback function will be call when outputing log
 *
 *
 * \return LOGGER_ERR_OK
 *
 */
IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetLogCallBackDut2(void (__stdcall * pFunc) (const char*));

//! IQlite_Logger set callback function for Dut3
/*!
 *
 * \param[in] The callback function will be call when outputing log
 *
 *
 * \return LOGGER_ERR_OK
 *
 */
IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetLogCallBackDut3(void (__stdcall * pFunc) (const char*));

//! IQlite_Logger set callback function for Dut4
/*!
 *
 * \param[in] The callback function will be call when outputing log
 *
 *
 * \return LOGGER_ERR_OK
 *
 */
IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetLogCallBackDut4(void (__stdcall * pFunc) (const char*));

#endif //end of _IQTLITE_LOGGER_H_
/*! \mainpage Timer Utility DLL for IQlite
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

/*! \file IQlite_Timer.h
 * \brief Timer Utility DLL for IQlite
 */ 


#ifndef _IQTLITE_TIMER_H_
#define _IQTLITE_TIMER_H_


#ifdef WIN32
#include <sys/timeb.h>
#else
#include <stdint.h>
#endif


#if !defined(__cplusplus)
    #define IQLITE_TIMER_API extern
    #error "just checking"
#elif !defined(WIN32)
    #define IQLITE_TIMER_API extern "C" 
#elif defined IQLITE_TIMER_EXPORTS
    #define IQLITE_TIMER_API extern "C" __declspec(dllexport)
#else
    #define IQLITE_TIMER_API extern "C" __declspec(dllimport) 
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
#ifndef MAX_TIMER_SIZE
    #define MAX_TIMER_SIZE 200
#endif
#ifndef MAX_TIMER_VECTOR_SIZE
    #define MAX_TIMER_VECTOR_SIZE 4096
#endif	
#ifndef NA_NUMBER
    #define NA_NUMBER -99999.99
#endif


#ifdef WIN32
	typedef struct __timeb64 lp_time_t;
#else
	typedef uint64_t lp_time_t;
#endif


/*! Error Code Definition
 */
typedef enum TIMER_EC
{
    TIMER_ERR_OK,                             /*!< Function completeds with no errors*/
	TIMER_ERR_FUNCTION_NOT_SUPPORTED,         /*!< Timer does not support the specified function*/
	TIMER_ERR_FUNCTION_NOT_DEFINED,           /*!< The specified function has not been defined yet*/
	TIMER_ERR_PARAM_DOES_NOT_EXIST,           /*!< The specified parameter does not exist*/
    TIMER_ERR_INVALID_TIMER_ID,               /*!< Invalid Timer ID*/
    TIMER_ERR_TIMER_ID_OVER_RANGE,            /*!< Over Max_Timer ID range*/
    TIMER_ERR_INVALID_SUPERVISOR_ID,          /*!< Invalid Supervisor ID*/
    TIMER_ERR_FAILED_TO_CREATE_SUPERVISOR_ID, /*!< Failed to Create Supervisor ID*/
	TIMER_ERR_FAILED_TO_CREATE_DIRECTORY,	  /*!< Failed to create the timer directory*/
    TIMER_ERR_TIMER_FUNCTION_ERROR,           /*!< Timer function had errors*/                  
} TIMER_RETURN;


//! IQlite_Timer Create Timer function
/*!
 *
 * \param[in]  char* timerName, a name for create a timer
 * \param[out] int *timerID, function return a timer ID
 * \param[in]  BOOL createSupervisorID, default = FALSE, to create a supervisor ID that has right to call TIMER_ClearTimerHistory() function  
 *
 * \return TIMER_ERR_OK If initial timer successful
 *
 */
IQLITE_TIMER_API TIMER_RETURN TIMER_CreateTimer(char* timerName, int *timerID, BOOL createSupervisorID=FALSE);

//! IQlite_Timer Clear Timer History function
/*!
 *
 * \param[in]  int supervisorID, a supervisor ID that has right to call TIMER_ClearTimerHistory() function
 * \param[out] None 
 *
 * \return TIMER_ERR_OK If initial timer successful
 *
 */
IQLITE_TIMER_API TIMER_RETURN TIMER_ClearTimerHistory(int supervisorID);

//! IQlite_Timer Start Timer function
/*!
 *
 * \param[in]  int timerID, an ID for timer identity 
 * \param[in]  char* tag, a name of time tag  
 * \param[out] double *timeStamp, default = NULL, return a start time stamp 
 *
 * \return TIMER_ERR_OK If initial timer successful
 *
 */
IQLITE_TIMER_API TIMER_RETURN TIMER_StartTimer(int timerID, char* tag, lp_time_t *timeStamp=NULL);

//! IQlite_Timer Stop Timer function
/*!
 *
 * \param[in]  int timerID, an ID for timer identity 
 * \param[in]  char* tag, a name of time tag  
 * \param[out] double *durationInMiniSec, return the duration of this time tag in mini second  
 * \param[out] double *timeStamp, default = NULL, return a stop time stamp 
 *
 * \return TIMER_ERR_OK If initial timer successful
 *
 */
IQLITE_TIMER_API TIMER_RETURN TIMER_StopTimer(int timerID, char* tag, double *durationInMiliSec, lp_time_t *timeStamp=NULL);

//! IQlite_Timer Report Timer Durations function
/*!
 *
 * \param[in]  double timeStampStart, a start time stamp
 * \param[in]  double timeStampStop, a stop time stamp
 *
 * \return TIMER_ERR_OK If initial timer successful
 *
 */
IQLITE_TIMER_API TIMER_RETURN TIMER_ReportTimerDurations(lp_time_t timeStampStart, lp_time_t timeStampStop);

#endif //end of _IQTLITE_TIMER_H_
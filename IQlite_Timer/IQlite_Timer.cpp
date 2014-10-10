// IQlite_Timer.cpp : Defines the entry point for the DLL application.
//
#pragma warning(disable : 4996)		// turn off deprecated warning for remaining ones

#include "lp_stdlib.h"
#include <string>
#include <map>
#include <errno.h>  // errno

#include "IQlite_Timer.h"
#include "lp_highrestimer.h"
#include "lp_direct.h" // _mkdir
#include "lp_time.h" // time
#include "lp_stdio.h" // sprintf_s
#include "StringUtil.h"


using namespace std;

// Global variables
const char  *g_IQlite_Timer_Version = "3.0.0.2 (2012-07-26)\n";

// High Performance Counter
int       g_sequenceOfID = -1;
int       g_supervisorID = -1;
int       g_turnTimerOn  = 0;
highrestimer::lp_time_t  g_highPerformanceFreq;


typedef struct tagIQlite_Timer
{
    BOOL           timerStarted;
    char           description[MAX_BUFFER_SIZE];
    highrestimer::lp_time_t  counterStart;
    highrestimer::lp_time_t  counterStop;
    double         durationInMiniSec;
} TIMER_STRUCT;

vector <TIMER_STRUCT> IQliteTimeStamp_Vector[MAX_TIMER_SIZE];
vector <TIMER_STRUCT>::size_type vectorSize;
double                  IQliteTimeStamp_Once[MAX_TIMER_SIZE]= {0};

map <string, int> IQliteTimerID_Map;
map <string, int>::iterator IQliteTimerID_Iter;
typedef pair <string, int> IQliteTimerID_Pair;

typedef struct tagIQlite_TimerSummary
{
    char           description[MAX_BUFFER_SIZE];
    int            iteration;
    double         totalTimeUsed;
} TIMER_SUMMARY_STRUCT;

vector <TIMER_SUMMARY_STRUCT> IQliteTimerSummary_Vector[MAX_TIMER_SIZE];
vector <TIMER_SUMMARY_STRUCT>::size_type vectorSizeOfSummary;

// STARTUP
// DLL startup and initialization code.





static TIMER_RETURN TIMER_IQliteTimerInitiation(void);
static TIMER_RETURN tret = TIMER_IQliteTimerInitiation();

static TIMER_RETURN TIMER_IQliteTimerInitiation(void)
{
    TIMER_RETURN ret = TIMER_ERR_OK;
	
	ret = (TIMER_RETURN)_mkdir( "Timer" );	// create the log file directory
	if( ret==TIMER_ERR_OK )	
	{
		// Directory was successfully created
	}
	else
	{
		if (errno==EEXIST)		// EEXIST: File exists, code = 17. 
		{
			ret = TIMER_ERR_OK;
		}
		else
		{
			// Failed to create the directory
			return TIMER_ERR_FAILED_TO_CREATE_DIRECTORY;
		}
	}

    highrestimer::HighResTimerInit();
	highrestimer::GetTime(g_highPerformanceFreq);	//initalize g_highPerformanceFreq timer

    /* 
    This is not needed, they are empty when they are constructed.
    // Vector and Map Initiation
    for (int i=0;i<MAX_TIMER_SIZE;i++)
    {
        IQliteTimeStamp_Vector[i].clear();
        IQliteTimerSummary_Vector[i].clear();
    }
    IQliteTimerID_Map.clear();
    */
    FILE *timerSetupFile = NULL;
    fopen_s( &timerSetupFile, "IQlite_Timer.ini", "rt" );

    if( NULL!=timerSetupFile )
    {
        char buffer[MAX_BUFFER_SIZE] = {'\0'};
        string line;
        vector<string> splits;
        while( fgets(buffer, MAX_BUFFER_SIZE, timerSetupFile) != NULL )
        {
		    int x=0;
		    while (buffer[x])
			{			
			    buffer[x] = toupper(buffer[x]);
			    x++;
		    }

            line = buffer;
            Trim( line );
            if( (0==line.size())||( (string::npos==line.find("="))&&((string::npos!=line.find("#"))||(string::npos!=line.find("//"))) ) )
            {
                // skip blank and comment lines
                continue;
            } 
            else
            {
				splits.clear();
                SplitString(line, splits, "=");
                if ( splits.size()==2 )
                {
					if ( string::npos!=splits[0].find("DEBUG_TIMER") )
					{
						g_turnTimerOn = atoi(splits[1].c_str());
					}
					else
					{
						// do nothing
					}	
                }
				else
				{
					// do nothing
				}
				continue;
            }
        }
        fclose(timerSetupFile);
    }
	else
	{
		// file does not exist, then set Timer to OFF
		g_turnTimerOn = 0;
	}

    return ret;
}

IQLITE_TIMER_API TIMER_RETURN TIMER_ClearTimerHistory(int supervisorID)
{
    TIMER_RETURN ret = TIMER_ERR_OK;

    if ( supervisorID==g_supervisorID )
    {
        for (int i=0;i<MAX_TIMER_SIZE;i++)
        {
            IQliteTimeStamp_Vector[i].clear();
        }
    }
    else
    {
        ret = TIMER_ERR_INVALID_SUPERVISOR_ID;
    }

    return ret;
}

IQLITE_TIMER_API TIMER_RETURN TIMER_CreateTimer(char* timerName, int *timerID, BOOL createSupervisorID)
{
    TIMER_RETURN ret = TIMER_ERR_OK;

    if ( g_sequenceOfID < MAX_TIMER_SIZE )  // TimerID must less than MAX_TIMER_SIZE
    {
        // We need to check if another user has registered the same TimerID or not
        IQliteTimerID_Iter = IQliteTimerID_Map.find( timerName );
        if( IQliteTimerID_Iter==IQliteTimerID_Map.end() ) // Can't find this Timer Name in Map, then insert ID directly
        {        
            g_sequenceOfID ++;

            *timerID = g_sequenceOfID;
            IQliteTimerID_Map.insert(IQliteTimerID_Pair(timerName, g_sequenceOfID));

            // Create Supervisor ID
            if ( createSupervisorID==TRUE )
            {       
                if ( -1==g_supervisorID )   // no g_supervisorID, allow to create ID.
                {
                    g_supervisorID = g_sequenceOfID;            
                }
                else    // already had g_supervisorID, return error
                {
                    ret = TIMER_ERR_FAILED_TO_CREATE_SUPERVISOR_ID;
                }
            }            
        }
        else    // another user has registered the same TimerID, return error
        {
            ret = TIMER_ERR_INVALID_TIMER_ID;
        }
    }
    else
    {
        ret = TIMER_ERR_TIMER_ID_OVER_RANGE;
    }

    return ret;
}

IQLITE_TIMER_API TIMER_RETURN TIMER_StartTimer(int timerID, char* tag, lp_time_t *timeStamp)
{
    TIMER_RETURN ret = TIMER_ERR_OK;

    TIMER_STRUCT   dummyStruct;
    highrestimer::lp_time_t  dummyCounter; 

    vector <TIMER_STRUCT>::size_type vectorSize;

    if( timerID>-1 && timerID<=g_sequenceOfID )    // Check ID not over range
    {
		//if(0==g_turnTimerOn)
  //      {
  //          highrestimer::GetTime(dummyCounter);
  //          IQliteTimeStamp_Once[timerID] = dummyCounter;

  //          if(NULL!=timeStamp)
  //          {
  //              *timeStamp = dummyCounter;// /(double)g_highPerformanceFreq.QuadPart) * 1000;  // Return this time stamp
  //          }
  //          else
  //          {
  //              // no action for NULL pointer
  //          }
  //         
  //      }
  //      else
		{
			vectorSize = IQliteTimeStamp_Vector[timerID].size( );
			vectorSize--;

			// Check Timer vector size first, in case the user never call the function "TIMER_ReportTimerDurations()" to clear Timer.
			if ( (int)vectorSize > MAX_TIMER_VECTOR_SIZE )
			{
				IQliteTimeStamp_Vector[timerID].clear();
				vectorSize = IQliteTimeStamp_Vector[timerID].size( );
				vectorSize--;
			}
			else
			{
				// do nothing
			}

			for (int i=(int)vectorSize;i>=0;i--)
			{
				if ( 0==strcmp(IQliteTimeStamp_Vector[timerID][i].description, tag) )     // find out the correct index of timer vector
				{
					if ( IQliteTimeStamp_Vector[timerID][i].timerStarted==TRUE )    // Check Timer status == TRUE, (start)
					{
						// Timer started already, stop Timer automatically and return error
						double dummyduration;

						TIMER_StopTimer(timerID, tag, &dummyduration, 0);  // Call stop Timer function automatically
						return TIMER_ERR_TIMER_FUNCTION_ERROR;
					}
					else    // This timer stop already, thus user can use the same tag name to start again.
					{
						break;
					}
				}
				else
				{
					// time tag not match, keep searching... (for loop)
				}
			}  

			highrestimer::GetTime(dummyCounter);
			dummyStruct.timerStarted = TRUE;    // Mark this timer started
			sprintf_s(dummyStruct.description, MAX_BUFFER_SIZE, "%s", tag);
			dummyStruct.counterStart = dummyCounter;
			dummyStruct.durationInMiniSec = -1;
			dummyStruct.counterStop = dummyCounter; // Just give counterStop a dummy value

			if(NULL!=timeStamp)
			{
				*timeStamp = dummyCounter;  // Return this time stamp
			}
			else
			{
				// no action for NULL pointer
			}

			IQliteTimeStamp_Vector[timerID].push_back(dummyStruct);
		}
    }
    else
    {
		*timeStamp = g_highPerformanceFreq;  // Return this time stamp, leave this here for timer alignment!! 
        ret = TIMER_ERR_INVALID_TIMER_ID;;
    }

    return ret;
}

IQLITE_TIMER_API TIMER_RETURN TIMER_StopTimer(int timerID, char* tag, double *durationInMiniSec, lp_time_t *timeStamp)
{
    TIMER_RETURN ret = TIMER_ERR_OK;

    highrestimer::lp_time_t  dummyCounter;  
    vector <TIMER_STRUCT>::size_type vectorSize;

  
    if( timerID>-1 && timerID<=g_sequenceOfID )    // Check ID not over range
    {
		//if(0 == g_turnTimerOn) //timer flag is off
  //      {
  //          highrestimer::GetTime(dummyCounter);
  //          if(NULL!=timeStamp)
  //          {
  //              *timeStamp = dummyCounter;  // Return this time stamp
  //          }
  //          else
  //          {
  //              // no action for NULL pointer
  //          }

  //          if(NULL!=durationInMiniSec)
  //          {
		//		//IQliteTimeStamp_Vector[timerID][i].durationInMiniSec = highrestimer::GetElapsedMSec(IQliteTimeStamp_Vector[timerID][i].counterStart, IQliteTimeStamp_Vector[timerID][i].counterStop);
  //              *durationInMiniSec = ((double)(dummyCounter. -IQliteTimeStamp_Once[timerID])/dummyCounter) * 1000;

		//		if(NULL!=durationInMiniSec)
		//		{
		//			*durationInMiniSec = IQliteTimeStamp_Vector[timerID][i].durationInMiniSec;
		//		}
  //          }
  //          else
  //          {
  //              // no action for NULL pointer
  //          }
  //      }
  //      else //time flag is on
        {

			vectorSize = IQliteTimeStamp_Vector[timerID].size( );
			vectorSize--;

			for (int i=(int)vectorSize;i>=0;i--)
			{
				if ( 0==strcmp(IQliteTimeStamp_Vector[timerID][i].description, tag) )     // find out the correct index of timer vector
				{
					if ( IQliteTimeStamp_Vector[timerID][i].timerStarted==TRUE )    // Check Timer status == TRUE, (start)
					{
						highrestimer::GetTime(dummyCounter);
						IQliteTimeStamp_Vector[timerID][i].counterStop = dummyCounter;
						IQliteTimeStamp_Vector[timerID][i].timerStarted = FALSE;    // Mark this timer stoped

						if(NULL!=timeStamp)
						{
							*timeStamp = dummyCounter;  // Return this time stamp
						}
						else
						{
							// no action for NULL pointer
						}
	                    
						//IQliteTimeStamp_Vector[timerID][i].durationInMiniSec = ((double)(IQliteTimeStamp_Vector[timerID][i].counterStop.QuadPart-IQliteTimeStamp_Vector[timerID][i].counterStart.QuadPart) / (double)g_highPerformanceFreq.QuadPart) * 1000;
						IQliteTimeStamp_Vector[timerID][i].durationInMiniSec = highrestimer::GetElapsedMSec(IQliteTimeStamp_Vector[timerID][i].counterStart, IQliteTimeStamp_Vector[timerID][i].counterStop);
						if(NULL!=durationInMiniSec)
						{
							*durationInMiniSec = IQliteTimeStamp_Vector[timerID][i].durationInMiniSec;
						}
						else
						{
							// no action for NULL pointer
						}

						//if (g_turnTimerOn==0)	// means turn OFF timer
						//{
						//	//IQliteTimeStamp_Vector[timerID].erase(vectorIterator);
						//	return ret;
						//}
						//else
						//{
						//	// do nothing
						//}
					}
					else    // This timer not start, return error
					{
						*durationInMiniSec = -1;
						ret = TIMER_ERR_TIMER_FUNCTION_ERROR;
					}

					break;
				}
				else
				{
					// time tag not match, keep searching... (for loop)
				}

				
	    			
			}       

        }       
    }
    else
    {
        ret = TIMER_ERR_INVALID_TIMER_ID;;
    }

    return ret;
}

IQLITE_TIMER_API TIMER_RETURN TIMER_ReportTimerDurations(lp_time_t timeStampStart, lp_time_t timeStampStop)
{
    TIMER_RETURN ret = TIMER_ERR_OK;

    FILE  *logByDistribution, *logByTagSummary, *logByID;
	char   buffer[MAX_BUFFER_SIZE] = {'\0'};
    double totalDuration, timeUsed[MAX_TIMER_SIZE] = {0};   

    TIMER_SUMMARY_STRUCT dummyStruct;

    // Timer vector initiation
    for (int i=0;i<MAX_TIMER_SIZE;i++)
    {        
        IQliteTimerSummary_Vector[i].clear();
    }
	if (g_turnTimerOn == 0)
	{
		//for (int i=0;i<MAX_TIMER_SIZE;i++)
		//{            
		//	IQliteTimeStamp_Vector[i].clear();
		//	IQliteTimerSummary_Vector[i].clear();
		//}
		//return ret;
	}
	else
	{
		// The total timer duration that defined by user in ms. <timeStampStart, timeStampStop>
		//totalDuration = ((double)(timeStampStop-timeStampStart) / (double)g_highPerformanceFreq.QuadPart) * 1000;
		totalDuration = highrestimer::GetElapsedMSec(timeStampStart, timeStampStop);

		// Get system time
		//CTime loggerTime = CTime::GetCurrentTime();   
		//CString cStr = loggerTime.Format(_T("%Y.%B.%d-%H.%M.%S"));

		//char c_time[MAX_BUFFER_SIZE], c_path[MAX_BUFFER_SIZE];
		//size_t i;
		//int sizeOfString = cStr.GetLength() + 1; 
		//LPTSTR pStrTmp = new TCHAR[sizeOfString];
		//_tcscpy_s( pStrTmp, sizeOfString, cStr );
		//wcstombs_s( &i, c_time, sizeOfString , pStrTmp, sizeOfString );
		//delete pStrTmp;

		//char c_time[MAX_BUFFER_SIZE] = {'\0'};
		char c_path[MAX_BUFFER_SIZE] = {'\0'};

		struct tm *current = NULL;
		time_t bintime = NULL;
		time(&bintime);
		current = localtime(&bintime);	
		//strftime(c_time, MAX_BUFFER_SIZE, "%y.%b.%d-%h.%m.%s", current);	
		//sprintf_s(c_path, MAX_BUFFER_SIZE, ".\\Timer\\%s", c_time);

		//localtime_s(current, &bintime);
		sprintf_s(c_path, MAX_BUFFER_SIZE, ".\\Timer\\%d.%d.%d-%d.%d.%d", (current->tm_year+1900), (current->tm_mon+1), current->tm_mday, current->tm_hour, current->tm_min, current->tm_sec);

		// Log by Distribution - open file <1>
		sprintf_s(buffer, MAX_BUFFER_SIZE, "%s_TimerDistribution.txt", c_path);
		fopen_s( &logByDistribution, buffer, "a+" ); 
		if (NULL==logByDistribution)
		{
			ret = TIMER_ERR_TIMER_FUNCTION_ERROR;
			return ret;
		}
		fprintf(logByDistribution, "===================================================================\n");
		fprintf(logByDistribution, "[Timer ID]   [Timer Name]           [Time Used]    [Percentage]\n");
		fprintf(logByDistribution, "===================================================================\n");

		// Log by TagSummary - open file <2>
		sprintf_s(buffer, MAX_BUFFER_SIZE, "%s_TimeTagSummary.txt", c_path);
		fopen_s( &logByTagSummary, buffer, "a+" ); 
		if (NULL==logByTagSummary)
		{
			ret = TIMER_ERR_TIMER_FUNCTION_ERROR;
			return ret;
		}
		fprintf(logByTagSummary, "==========================================================================================\n");
		fprintf(logByTagSummary, "[ID][Time Tag]             [Iteration]   [Total Time Used]   [Time/Per]   [Percentage]\n");
		fprintf(logByTagSummary, "==========================================================================================\n");

		for(int idIndex=0;idIndex<MAX_TIMER_SIZE;idIndex++)
		{
			vectorSize = IQliteTimeStamp_Vector[idIndex].size( );
			vectorSize--;

			if ( (int)vectorSize>=0 )  // Timer Vector not empty
			{
				// Log by Timer ID - open file <3>
				sprintf_s(buffer, MAX_BUFFER_SIZE, "%s_ID%d_TimerDuration.txt", c_path, idIndex);
				fopen_s( &logByID, buffer, "a" );
				if (NULL==logByID)
				{
					ret = TIMER_ERR_TIMER_FUNCTION_ERROR;
					return ret;
				}
				fprintf(logByID, "===========================================\n");
				fprintf(logByID, "[Timer Tag]                  [Time Used]\n");
				fprintf(logByID, "===========================================\n");

				for (int index=0;index<=(int)vectorSize;index++)    // print out the Timer description and its duration(sec)
				{
					// check the time tag within the range that defined by user. <timeStampStart, timeStampStop>
					if ( ( (IQliteTimeStamp_Vector[idIndex][index].counterStart) >= timeStampStart)
						 &&(IQliteTimeStamp_Vector[idIndex][index].counterStop <= timeStampStop) )
					{
						if ( -1!=IQliteTimeStamp_Vector[idIndex][index].durationInMiniSec )  // Check duration has value or not
						{
							// This is a special case, if the timer ID contain more than one time tag and its time stamp "start" and "stop" = time window, then must skip this one.
							if (  (IQliteTimeStamp_Vector[idIndex][index].counterStart == timeStampStart)
								&&(IQliteTimeStamp_Vector[idIndex][index].counterStop == timeStampStop)&&(0!=(int)vectorSize) )
							{
								continue;
							}

							////////////////////////////////
							//  Ready for data collection //
							////////////////////////////////
							// for file 1
							timeUsed[idIndex] = timeUsed[idIndex] + IQliteTimeStamp_Vector[idIndex][index].durationInMiniSec;   

							// for file 2
							BOOL foundTimeTag = FALSE;
							vectorSizeOfSummary = IQliteTimerSummary_Vector[idIndex].size( );
							vectorSizeOfSummary--;
							for (int j=0;j<=(int)vectorSizeOfSummary;j++)
							{
								// looking the same time tag name (description) inside the summary vector
								if ( 0==strcmp(IQliteTimerSummary_Vector[idIndex][j].description, IQliteTimeStamp_Vector[idIndex][index].description) )
								{
									foundTimeTag = TRUE;
									IQliteTimerSummary_Vector[idIndex][j].iteration++;
									IQliteTimerSummary_Vector[idIndex][j].totalTimeUsed = IQliteTimerSummary_Vector[idIndex][j].totalTimeUsed + IQliteTimeStamp_Vector[idIndex][index].durationInMiniSec;
									break;
								}
								else
								{
									foundTimeTag = FALSE;
									// keep searching...
								}
							}
							if (foundTimeTag==FALSE)    
							{   // Add a new time tag information into summary vector. (By idIndex)
								sprintf_s(dummyStruct.description, MAX_BUFFER_SIZE, "%s", IQliteTimeStamp_Vector[idIndex][index].description);
								dummyStruct.iteration = 1;
								dummyStruct.totalTimeUsed = IQliteTimeStamp_Vector[idIndex][index].durationInMiniSec;
								IQliteTimerSummary_Vector[idIndex].push_back(dummyStruct);
							}

							 // Log by ID, for file 3
							fprintf(logByID, "%-25s %10.3f ms\n", IQliteTimeStamp_Vector[idIndex][index].description, IQliteTimeStamp_Vector[idIndex][index].durationInMiniSec);
						}
						else
						{
							ret = TIMER_ERR_PARAM_DOES_NOT_EXIST;
						}
					}
					else
					{
						// If not within the range of timeStampStart ~ timeStampStop, skip and do nothing 
					}
				}
	            
				fclose(logByID);
			}
			else
			{
				//ret = TIMER_ERR_TIMER_FUNCTION_ERROR;
			}     
	        
			// Log by Distribution, for file 1
			IQliteTimerID_Iter = IQliteTimerID_Map.begin();
			while (IQliteTimerID_Iter!=IQliteTimerID_Map.end())
			{
				if (IQliteTimerID_Iter->second==idIndex)
				{
					fprintf(logByDistribution, "[ID-%03d]        %-20s %9.3f ms %13.1f\n", idIndex, IQliteTimerID_Iter->first.c_str(), timeUsed[idIndex], (timeUsed[idIndex]/totalDuration)*100 );    // for file 1
				}
				else
				{
					// keep searching timer name
				}

				IQliteTimerID_Iter++;
			}
	      
		}  // end loop - for(int idIndex=0;idIndex<=MAX_TIMER_SIZE;idIndex++)


		// Log by TagSummary, for file 2
	    for(int i=0;i<MAX_TIMER_SIZE;i++)
		{
			vectorSizeOfSummary = IQliteTimerSummary_Vector[i].size( );
			vectorSizeOfSummary--;
			for (int j=0;j<=(int)vectorSizeOfSummary;j++)
			{
				if (j==0)   fprintf(logByTagSummary, "[ID-%d]\n", i);
				fprintf(logByTagSummary, "    %-24s %6d %15.3f ms %12.3f ms %12.1f\n", IQliteTimerSummary_Vector[i][j].description, IQliteTimerSummary_Vector[i][j].iteration, IQliteTimerSummary_Vector[i][j].totalTimeUsed, (IQliteTimerSummary_Vector[i][j].totalTimeUsed/IQliteTimerSummary_Vector[i][j].iteration), (IQliteTimerSummary_Vector[i][j].totalTimeUsed/totalDuration)*100 );
			}
		}

		fclose(logByDistribution);
		fclose(logByTagSummary);
	}
	
	// TODO: Check the size of time stamp, if it is too big, then must clear some of them.
	// Vector Initiation
	for (int i=0;i<MAX_TIMER_SIZE;i++)
	{
		IQliteTimeStamp_Vector[i].clear();
		IQliteTimerSummary_Vector[i].clear();
	}

	return ret;

}
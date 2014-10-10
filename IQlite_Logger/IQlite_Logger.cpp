// IQlite_Logger.cpp : Defines the entry point for the DLL application.
//

// turn off deprecated warning for remaining ones
#pragma warning(disable : 4996)		// turn off deprecated warning for remaining ones


#include "lp_windows.h"
#include <errno.h>
#include "lp_stdlib.h"
#include <string>
#include <map>
#include <vector>

#include "lp_stdio.h" // sprintf_s
#include "lp_time.h"
#include "lp_direct.h"
#include "lp_string.h"
#include "lp_consoleMode.h"
#include "StringUtil.h"
#include "IQlite_Logger.h"



using namespace std;

// Global variables
const char   *g_IQlite_Logger_Version = "3.0.0.1 (2012-07-20)\n";
char          g_defaultFileName[MAX_BUFFER_SIZE] = {""};
int           g_sequenceOfID      = -1;
bool		  g_readLogIniFile    = false;
LOGGER_LEVEL  g_loggerLevel       = LOGGER_ERROR;
LOGGER_LEVEL  g_loggerInputInfoLevel = LOGGER_ERROR;
int			  g_IQliteLoggerSource[MAX_LOGGER_SOURCE];
int			  g_logAllForDebug	  = 0;

typedef struct tagIQlite_LoggerINFORMATION
{
    int      loggerID;
    char     loggerFilePath[MAX_BUFFER_SIZE];
} LOGGER_INFO_STRUCT;

static map <string, LOGGER_INFO_STRUCT> IQliteLoggerID_Map;
static map <string, LOGGER_INFO_STRUCT>::iterator IQliteLoggerID_Iter;
typedef pair <string, LOGGER_INFO_STRUCT> IQliteLoggerID_Pair;



void (__stdcall * mainLogger) (const char*);
void (__stdcall * dut1Logger) (const char*);
void (__stdcall * dut2Logger) (const char*);
void (__stdcall * dut3Logger) (const char*);
void (__stdcall * dut4Logger) (const char*);


static LOGGER_RETURN LOGGER_Write_CallBack(int loggerID, LOGGER_LEVEL level, void (__stdcall * pFunc) (const char*), const char *format, ...);

//IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetLogCallBackMain(void (__stdcall * pFunc) (const char*))
//{
//    LOGGER_RETURN ret = LOGGER_ERR_OK;
//	if(pFunc)
//	{
//		mainLogger = pFunc;
//	}
//	return ret;
//}

IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetLogCallBackDut1(void (__stdcall * pFunc) (const char*))
{
    LOGGER_RETURN ret = LOGGER_ERR_OK;
	if(pFunc)
	{
		dut1Logger = pFunc;
	}
	return ret;
}

IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetLogCallBackDut2(void (__stdcall * pFunc) (const char*))
{
    LOGGER_RETURN ret = LOGGER_ERR_OK;
	if(pFunc)
	{
		dut2Logger = pFunc;
	}
	return ret;
}

IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetLogCallBackDut3(void (__stdcall * pFunc) (const char*))
{
    LOGGER_RETURN ret = LOGGER_ERR_OK;
	if(pFunc)
	{
		dut3Logger = pFunc;
	}
	return ret;
}

IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetLogCallBackDut4(void (__stdcall * pFunc) (const char*))
{
    LOGGER_RETURN ret = LOGGER_ERR_OK;
	if(pFunc)
	{
		dut4Logger = pFunc;
	}
	return ret;
}


// STARTUP
// DLL startup and initialization code.
// This code uses the above global variables, and it must be executed AFTER they are constructed.
static LOGGER_RETURN logger_startup = LOGGER_Initialization();

IQLITE_LOGGER_API LOGGER_RETURN LOGGER_Initialization(void)
{
    LOGGER_RETURN ret = LOGGER_ERR_OK;
	bool	setLevelOk = false;

	for (int i=0;i<MAX_LOGGER_SOURCE;i++)
	{
		g_IQliteLoggerSource[i] = 1;	// default set all source to ON
	}

	ret = (LOGGER_RETURN)_mkdir( "Log" );	// create the log file directory
	if( ret==LOGGER_ERR_OK )	
	{
		// Directory was successfully created
	}
	else
	{
		if (errno==EEXIST)		// EEXIST: File exists, code = 17. 
		{
			ret = LOGGER_ERR_OK;
		}
		else
		{
			// Failed to create the directory
			return LOGGER_ERR_FAILED_TO_CREATE_DIRECTORY;
		}
	}

    // not needed: IQliteLoggerID_Map.clear();

    FILE *loggerSetupFile = NULL;
    fopen_s( &loggerSetupFile, "IQlite_Logger.ini", "rt" );

    if( NULL!=loggerSetupFile )
    {
        char buffer[MAX_BUFFER_SIZE] = {'\0'};
        string line;
        vector<string> splits;
        while( !feof( loggerSetupFile ) )
        {
            fgets(buffer, MAX_BUFFER_SIZE, loggerSetupFile);

		    int x=0;
		    while (buffer[x])
			{			
			    buffer[x] = toupper(buffer[x]);
			    x++;
		    }

            line = buffer;
            Trim( line );
			// skip comment lines
			if( string::npos!=line.find("#") )
			{
				line.erase(line.find("#"));
			}
			if( string::npos!=line.find("//") )
			{
				line.erase(line.find("//"));
			}
            if( (0==line.size())||(string::npos==line.find("=")) )
            {
                // skip blank lines, and the lines without an "=".
                continue;
            } 
            else
            {
				splits.clear();
                SplitString(line, splits, "=");
                if ( splits.size()==2 )
                {
                    // a valid Mask_Type must be < reference(ref)/absolute(abs) >
					if ( string::npos!=splits[0].find("LOG_IQLITE_CORE") )
					{
						g_IQliteLoggerSource[LOG_IQLITE_CORE] = atoi(splits[1].c_str());
					}
					else if ( string::npos!=splits[0].find("LOG_IQLITE_TM") )
					{
						g_IQliteLoggerSource[LOG_IQLITE_TM] = atoi(splits[1].c_str());
					}
					else if ( string::npos!=splits[0].find("LOG_IQLITE_VDUT") )
					{
						g_IQliteLoggerSource[LOG_IQLITE_VDUT] = atoi(splits[1].c_str());
					}
					else if ( string::npos!=splits[0].find("LOG_IQMEASURE") )
					{
						g_IQliteLoggerSource[LOG_IQMEASURE] = atoi(splits[1].c_str());
					}
					else if ( string::npos!=splits[0].find("LOG_DUT_DLL") )
					{
						g_IQliteLoggerSource[LOG_DUT_DLL] = atoi(splits[1].c_str());
					}
					else if ( string::npos!=splits[0].find("LOG_DUT1") )
					{
						g_IQliteLoggerSource[LOG_DUT1] = atoi(splits[1].c_str());
					}
					else if ( string::npos!=splits[0].find("LOG_DUT2") )
					{
						g_IQliteLoggerSource[LOG_DUT2] = atoi(splits[1].c_str());
					}
					else if ( string::npos!=splits[0].find("LOG_DUT3") )
					{
						g_IQliteLoggerSource[LOG_DUT3] = atoi(splits[1].c_str());
					}
					else if ( string::npos!=splits[0].find("LOG_DUT4") )
					{
						g_IQliteLoggerSource[LOG_DUT4] = atoi(splits[1].c_str());
					}
					else if ( string::npos!=splits[0].find("LOGGER_LEVEL") )
					{
						if ( string::npos!=splits[1].find("NONE") )
						{
							LOGGER_SetLogLevel(LOGGER_NONE);
						}
						else if ( string::npos!=splits[1].find("ERROR") )
						{
							LOGGER_SetLogLevel(LOGGER_ERROR);
						}
						else if ( string::npos!=splits[1].find("WARNING") )
						{
							LOGGER_SetLogLevel(LOGGER_WARNING);
						}
						else if ( string::npos!=splits[1].find("INFORMATION") )
						{
							LOGGER_SetLogLevel(LOGGER_INFORMATION);
						}
						else
						{
							LOGGER_SetLogLevel(LOGGER_ERROR);	// set Log_Level to default
						}
						setLevelOk = true;
//						g_readLogIniFile = true;
					}
					else if ( string::npos!=splits[0].find("PRINT_INPUT_INFO") )
					{
						if ( string::npos!=splits[1].find("1") )
						{
							LOGGER_SetInputInfoLevel(LOGGER_ERROR);
						}
						else if ( string::npos!=splits[1].find("ON") )
						{
							LOGGER_SetInputInfoLevel(LOGGER_ERROR);
						}
						else if ( string::npos!=splits[1].find("0") )
						{
							LOGGER_SetInputInfoLevel(LOGGER_NONE);
						}
						else if ( string::npos!=splits[1].find("OFF") )
						{
							LOGGER_SetInputInfoLevel(LOGGER_NONE);
						}
						else
						{
							LOGGER_SetLogLevel(LOGGER_ERROR);	// set Log_Level to default
						}
					}
					else if ( string::npos!=splits[0].find("LOG_ALL_FOR_DEBUG") )
					{
						g_logAllForDebug = atoi(splits[1].c_str());
					}
					else if ( string::npos!=splits[0].find("LOG_WIFIMPS_RESULT_FORMAT") )
					{
                        g_IQliteLoggerSource[LOG_WIFIMPS_RESULT_FORMAT] = atoi(splits[1].c_str());
					}
                    else if ( string::npos!=splits[0].find("LOG_SHOW_POWER_LEVEL") )
					{
                        g_IQliteLoggerSource[LOG_SHOW_POWER_LEVEL] = atoi(splits[1].c_str());
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
        fclose(loggerSetupFile);
    }
	else
	{
		// file does not exist, then set Log_Level to default
		LOGGER_SetLogLevel(LOGGER_ERROR);
		g_readLogIniFile = false;
	}

    return ret;
}

IQLITE_LOGGER_API LOGGER_RETURN LOGGER_CreateLogger(char* loggerName, int *loggerID, char* specificFileName)
{
    LOGGER_RETURN ret = LOGGER_ERR_OK;

    LOGGER_INFO_STRUCT dummyStruct;

	char bufferStr[MAX_BUFFER_SIZE] = {'\0'};

    if ( g_sequenceOfID < MAX_LOGGER_SIZE )  // ID must less than MAX_LOGGER_SIZE
    {
        // Get system time
        //CTime loggerTime = CTime::GetCurrentTime();   
        //CString cStr = loggerTime.Format(_T("%Y.%B.%d-%H.%M.%S"));

        //size_t i;
        //int sizeOfString = cStr.GetLength() + 1; 
        //LPTSTR pStrTmp = new TCHAR[sizeOfString];
        //_tcscpy_s( pStrTmp, sizeOfString, cStr );
        //wcstombs_s( &i, bufferStr, sizeOfString , pStrTmp, sizeOfString );
        //delete pStrTmp;

		struct tm *current = NULL;
		time_t bintime;
		time(&bintime);
		current = localtime(&bintime);
		strftime(bufferStr, MAX_BUFFER_SIZE, "%Y.%B.%d-%H.%M.%S", current);

        // We need to check if another user has registered the same ID or not
        IQliteLoggerID_Iter = IQliteLoggerID_Map.find( loggerName );
        if( IQliteLoggerID_Iter==IQliteLoggerID_Map.end() ) // If no one using this Logger Name, then we can create and insert ID directly
        {        
            g_sequenceOfID ++;
            *loggerID = g_sequenceOfID;

            dummyStruct.loggerID = g_sequenceOfID;   
            if ( specificFileName!=NULL && strlen(specificFileName)>0 ) // using user defined file name
            {
                sprintf_s(dummyStruct.loggerFilePath, MAX_BUFFER_SIZE, "%s-%s", bufferStr, specificFileName);                
            }
            else 
            {
                if (0==g_sequenceOfID)  // This is the first time to create ID and default file name
                {
					//sprintf_s(dummyStruct.loggerFilePath, MAX_BUFFER_SIZE, "%s-%s", bufferStr, "Debug_Log");
					//sprintf_s(g_defaultFileName, MAX_BUFFER_SIZE, "%s-%s", bufferStr, "Debug_Log");                    
                    sprintf_s(dummyStruct.loggerFilePath, MAX_BUFFER_SIZE, "%s", "Log_all");
                    sprintf_s(g_defaultFileName, MAX_BUFFER_SIZE, "%s", "Log_all");                    
                }
                else    // using default file name
                {
                    sprintf_s(dummyStruct.loggerFilePath, MAX_BUFFER_SIZE, "%s", g_defaultFileName);
                }
            }            
            IQliteLoggerID_Map.insert(IQliteLoggerID_Pair(loggerName, dummyStruct));  
        }
        else    // another user had registered the same LoggerID already, then return error
        {
            //ret = LOGGER_ERR_INVALID_LOGGER_ID;

			// To fix [IQlite-112] "IQlite_logger setting only applies to the first run", so do not returen error in this case  
			*loggerID = IQliteLoggerID_Iter->second.loggerID;
        }
    }
    else
    {
        ret = LOGGER_ERR_LOGGER_ID_OVER_RANGE;
    }
    
    return ret;
}



IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetInputInfoLevel(LOGGER_LEVEL level)
{
	LOGGER_RETURN ret = LOGGER_ERR_OK;

	if ( !g_readLogIniFile )
	{
		g_loggerInputInfoLevel = level;
	}
	else
	{
		// Already read and set logger level from ini file, thus do nothing
	}

	//printf("\n### Set Logger_Level to %d ###\n", level);

	return ret;

}

IQLITE_LOGGER_API LOGGER_RETURN LOGGER_SetLogLevel(LOGGER_LEVEL level)
{
    LOGGER_RETURN ret = LOGGER_ERR_OK;

	if ( !g_readLogIniFile )
	{
		g_loggerLevel = level;
	}
	else
	{
		// Already read and set logger level from ini file, thus do nothing
	}

	//printf("\n### Set Logger_Level to %d ###\n", level);
       
    return ret;
}


IQLITE_LOGGER_API LOGGER_LEVEL LOGGER_GetLogLevel(void)
{
	return g_loggerLevel;
}

IQLITE_LOGGER_API LOGGER_LEVEL LOGGER_GetLogInputInfoLevel(void)
{
	return g_loggerInputInfoLevel;
}

IQLITE_LOGGER_API LOGGER_RETURN LOGGER_GetLoggerSourceLevel(LOGGER_SOURCE loggerSoure, int* pIntVal)
{
   *pIntVal = g_IQliteLoggerSource[loggerSoure]; //return an int value 
   return LOGGER_ERR_OK;
}


IQLITE_LOGGER_API LOGGER_RETURN LOGGER_Write_Ext(LOGGER_SOURCE loggerSource, int loggerID, LOGGER_LEVEL level, const char *format, ...)
{
    LOGGER_RETURN ret = LOGGER_ERR_OK;

	char buffer[MAX_BUFFER_SIZE] = {'\0'};

	if ( g_IQliteLoggerSource[loggerSource]==1 )	// 1 means ON
	{
        // Log message format ... variable argument list
        va_list ap;
        va_start(ap, format);
		vsprintf_s(buffer, MAX_BUFFER_SIZE, format, ap);
		va_end(ap);

		ret = LOGGER_Write( loggerID, level, buffer );    

		switch(loggerSource)
		{
		case LOG_DUT1:
			LOGGER_Write_CallBack(loggerID, level, dut1Logger, buffer);
		break;

		case LOG_DUT2:
			LOGGER_Write_CallBack(loggerID, level, dut2Logger, buffer);
		break;

		case LOG_DUT3:
			LOGGER_Write_CallBack(loggerID, level, dut3Logger, buffer);
		break;

		case LOG_DUT4:
			LOGGER_Write_CallBack(loggerID, level, dut4Logger, buffer);
		break;

		default:
			LOGGER_Write_CallBack(loggerID, level, mainLogger, buffer);
		break;
		}
		     
	}
	else
	{
		// Since OFF, skip and do nothing...
	}

    return ret;
}


IQLITE_LOGGER_API LOGGER_RETURN LOGGER_Write(int loggerID, LOGGER_LEVEL level, const char *format, ...)
{
    LOGGER_RETURN ret = LOGGER_ERR_OK;

    
    // Check log level, if level more than g_loggerLevel, then ignore this log message
    if ( (g_logAllForDebug==0)&&((level==LOGGER_NONE)||(level>g_loggerLevel)) )    
    {
        return ret;  // Ignore this logMessage, don't need to write it into file
    }
    
    if( loggerID>-1 && loggerID<=g_sequenceOfID )    // Check ID not over range
    {
        IQliteLoggerID_Iter = IQliteLoggerID_Map.begin();
        while( IQliteLoggerID_Iter!=IQliteLoggerID_Map.end() ) 
        {
            if ( IQliteLoggerID_Iter->second.loggerID==loggerID )  // Check Logger ID and try to find out the path name
            {       
                // Ready for log
				//-----------------//
                //  Write to file  //   
				//-----------------//
				char path[MAX_BUFFER_SIZE] = {'\0'};

                // For the file size controled. If size more than MAX_FILE_SIZE, we need to increase the file index.
                sprintf_s(path, MAX_BUFFER_SIZE, "./log/%s.txt", IQliteLoggerID_Iter->second.loggerFilePath );
                
                // Log message format ... variable argument list
                va_list ap;
                va_start(ap, format);

                FILE *fp;  
                int length;
                fopen_s(&fp, path, "a+");    
 		        if (fp)
		        {
			        length = vfprintf(fp, format, ap);    // print to log file 
			        fclose(fp);
		        }

				//-----------------//
                //  Show on screen //   
				//-----------------//
				// Check log level, if level more than g_loggerLevel, then ignore this log message
				if ( (level==LOGGER_NONE)||(level>g_loggerLevel) )    
				{
					// Ignore this logMessage, don't need to show on screen
				}
				else
				{
					// Set Log color (on screen)
					if (level==LOGGER_ERROR)
					{
						//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY | FOREGROUND_RED);  
                        SetConsoleMode(CM_RED);
					}
					else if (level==LOGGER_WARNING)
					{
						//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY | FOREGROUND_BLUE);  
                        SetConsoleMode(CM_BLUE);
					}
					else if (level==LOGGER_INFORMATION)
					{
						//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY | FOREGROUND_GREEN);  
                        SetConsoleMode(CM_GREEN);
					}
					else
					{
						//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);  
                        SetConsoleMode(CM_WHITE);
					}
					// Print out the message on screen
					vprintf(format, ap);
					// Set color back to white
					//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);  
                    SetConsoleMode(CM_WHITE);

				}

                va_end(ap);              
                         
                break;  // break while loop 
            }
            else
            {
                IQliteLoggerID_Iter++;
            }
        }       
        
    }
    else
    {
        ret = LOGGER_ERR_INVALID_LOGGER_ID;;
    }

    return ret;
}


// Internal Function.
LOGGER_RETURN LOGGER_Write_CallBack(int loggerID, LOGGER_LEVEL level, void (__stdcall * pFunc) (const char*), const char *format, ...)
{
    LOGGER_RETURN ret = LOGGER_ERR_OK;

	/*------------------------------
	For Multi-Thread consideration
	-------------------------------*/
	map <string, LOGGER_INFO_STRUCT>::iterator l_IQliteLoggerID_Iter;
    
    // Check log level, if level more than g_loggerLevel, then ignore this log message
    if ( (g_logAllForDebug==0)&&((level==LOGGER_NONE)||(level>g_loggerLevel)) )    
    {
        return ret;  // Ignore this logMessage, don't need to write it into file
    }
    
    if(pFunc && (loggerID>-1 && loggerID<=g_sequenceOfID))    // Check ID not over range
    {
        /*IQliteLoggerID_Iter = IQliteLoggerID_Map.begin();*/
		l_IQliteLoggerID_Iter = IQliteLoggerID_Map.begin();
        while( /*IQliteLoggerID_Iter!=IQliteLoggerID_Map.end()*/
			   l_IQliteLoggerID_Iter!=IQliteLoggerID_Map.end()) 
        {
            if ( /*IQliteLoggerID_Iter->second.loggerID==loggerID*/
				 l_IQliteLoggerID_Iter->second.loggerID==loggerID)  // Check Logger ID and try to find out the path name
            {   
                // Log message format ... variable argument list
                va_list ap;
                va_start(ap, format);

				//-----------------//
                //  Show on screen //   
				//-----------------//
				// Check log level, if level more than g_loggerLevel, then ignore this log message
				if ( (level==LOGGER_NONE)||(level>g_loggerLevel) )    
				{
					// Ignore this logMessage, don't need to show on screen
				}
				else
				{
					// Set Log color (on screen)
					if (level==LOGGER_ERROR)
					{
					}
					else if (level==LOGGER_WARNING)
					{
					}
					else if (level==LOGGER_INFORMATION)
					{
					}
					else
					{
					}

					pFunc(format);
				}

                va_end(ap);              
                         
                break;  // break while loop 
            }
            else
            {
                //IQliteLoggerID_Iter++;
				l_IQliteLoggerID_Iter++;
            }
        }       
        
    }
    else
    {
        ret = LOGGER_ERR_INVALID_LOGGER_ID;;
    }

    return ret;
}


IQLITE_LOGGER_API LOGGER_RETURN LOGGER_Write2(int loggerID, LOGGER_LEVEL level, int color, const char *format, ...)
{
	LOGGER_RETURN ret = LOGGER_ERR_OK;


	// Check log level, if level more than g_loggerLevel, then ignore this log message
	if ( (g_logAllForDebug==0)&&((level==LOGGER_NONE)||(level>g_loggerLevel)) )    
	{
		return ret;  // Ignore this logMessage, don't need to write it into file
	}

	if( loggerID>-1 && loggerID<=g_sequenceOfID )    // Check ID not over range
	{
		IQliteLoggerID_Iter = IQliteLoggerID_Map.begin();
		while( IQliteLoggerID_Iter!=IQliteLoggerID_Map.end() ) 
		{
			if ( IQliteLoggerID_Iter->second.loggerID==loggerID )  // Check Logger ID and try to find out the path name
			{       
				// Ready for log
				//-----------------//
				//  Write to file  //   
				//-----------------//
				char path[MAX_BUFFER_SIZE] = {'\0'};

				// For the file size controled. If size more than MAX_FILE_SIZE, we need to increase the file index.
				sprintf_s(path, MAX_BUFFER_SIZE, "./log/%s.txt", IQliteLoggerID_Iter->second.loggerFilePath );

				// Log message format ... variable argument list
				va_list ap;
				va_start(ap, format);

				FILE *fp;  
				int length;
				fopen_s(&fp, path, "a+");    
				if (fp)
				{
					length = vfprintf(fp, format, ap);    // print to log file 
					fclose(fp);
				}

				//-----------------//
				//  Show on screen //   
				//-----------------//
				// Check log level, if level more than g_loggerLevel, then ignore this log message
				if ( (level==LOGGER_NONE)||(level>g_loggerLevel) )    
				{
					// Ignore this logMessage, don't need to show on screen
				}
				else
				{
					SetConsoleMode((CONSOLE_COLOR)color);

					// Print out the message on screen
					vprintf(format, ap);
					// Set color back to white
					//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);  
					SetConsoleMode(CM_WHITE);

				}

				va_end(ap);              

				break;  // break while loop 
			}
			else
			{
				IQliteLoggerID_Iter++;
			}
		}       

	}
	else
	{
		ret = LOGGER_ERR_INVALID_LOGGER_ID;;
	}

	return ret;
}



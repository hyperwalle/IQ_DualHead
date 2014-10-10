#include "stdafx.h"
#include "TestManager.h"
#include "WiFi_Test.h"
#include "WiFi_Test_Internal.h"
#include "IQmeasure.h"
#include "StringUtil.h"

using namespace std;


// Input Parameter Container
map<string, WIFI_SETTING_STRUCT> l_loadMultiSegmentWaveformParamMap;

// Return Value Container 
map<string, WIFI_SETTING_STRUCT> l_loadMultiSegmentWaveformReturnMap;


struct tagParam
{
    int     RELOAD_MULTI_WAVEFORM_FILE;								/*!< A flag to control reload multi-segment waveform file. Default = 0 (OFF) */
    char    MULTI_WAVEFORM_DEFINITION_FILE[MAX_BUFFER_SIZE];

} l_loadMultiSegmentWaveformParam;

struct tagReturn
{
    char    ERROR_MESSAGE[MAX_BUFFER_SIZE];
} l_loadMultiSegmentWaveformReturn;


void ClearLoadMultiSegmentWaveformReturn(void)
{
	l_loadMultiSegmentWaveformParamMap.clear();
	l_loadMultiSegmentWaveformReturnMap.clear();
}


WIFI_TEST_API int WiFi_Load_Multi_Segment_Waveform(void)
{
    int  err = ERR_OK;
	int  dummyValue = 0;
	char logMessage[MAX_BUFFER_SIZE] = {'\0'};


    /*---------------------------------------*
     * Clear Return Parameters and Container *
     *---------------------------------------*/
	ClearReturnParameters(l_loadMultiSegmentWaveformReturnMap);

    /*------------------------*
     * Respond to QUERY_INPUT *
     *------------------------*/
    err = TM_GetIntegerParameter(g_WiFi_Test_ID, "QUERY_INPUT", &dummyValue);
    if( ERR_OK==err )
    {
        RespondToQueryInput(l_loadMultiSegmentWaveformParamMap);
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
        RespondToQueryReturn(l_loadMultiSegmentWaveformReturnMap);
        return err;
    }
	else
	{
		// do nothing
	}


	try
	{
	   /*---------------------------------------*
		* g_WiFi_Test_ID need to be valid (>=0) *
		*---------------------------------------*/
		if( g_WiFi_Test_ID<0 )  
		{
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] WiFi_Test_ID not valid.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] WiFi_Test_ID = %d.\n", g_WiFi_Test_ID);
		}

		TM_ClearReturns(g_WiFi_Test_ID);

       /*----------------------*
        * Get input parameters *
        *----------------------*/
        err = GetInputParameters(l_loadMultiSegmentWaveformParamMap);
		if ( ERR_OK!=err )
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Input parameters are not complete.\n");
			throw logMessage;
		}
		else
		{
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Get input parameters return OK.\n");
		}

	   /*---------------*
		* Enable VSG RF *
		*---------------*/
		// Make sure VSG enabled
		//err = ::LP_EnableVsgRF(1);
		//if ( ERR_OK!=err )
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to turn on VSG, LP_EnableVsgRF(1) return error.\n");
		//	throw logMessage;
		//}
		//else
		//{
		//	LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] Turn on VSG LP_EnableVsgRF(1) return OK.\n");
		//}

		if(!g_WiFiGlobalSettingParam.IQ2010_EXT_ENABLE)
		{
			// The "LOAD_MULTI_SEGMENT_WAVEFORM" only supported when IQ2010Ext enabled.
			err = -1;
			LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] The LOAD_MULTI_SEGMENT_WAVEFORM support IQ2010Ext mode only.\n");
			throw logMessage;
		}
		else
		{
			// do nothing
		}

       /*----------------------------*
        * Check Map is empty or not  *
        *----------------------------*/
		bool mapIsEmpty = false;
		TM_CheckMultiWaveformIndexMap( &mapIsEmpty );

		if ( mapIsEmpty||l_loadMultiSegmentWaveformParam.RELOAD_MULTI_WAVEFORM_FILE )
		{
		   /*------------------------------------*
			* Clear the Multi-Waveform Index Map *
			*------------------------------------*/
			TM_ClearMultiWaveformIndexMap();

			/*---------------------------------------------*
			* Start a new Multi-segment Waveform Creation *
			*---------------------------------------------*/
			err = LP_IQ2010EXT_NewMultiSegmentWaveform();	// TODO
			if ( ERR_OK!=err )
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to start a new Multi-segment Waveform Creation, LP_IQ2010EXT_NewMultiSegmentWaveform() return error.\n");
				throw logMessage;
			}
			else
			{
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_NewMultiSegmentWaveform() return OK.\n");
			}

		   /*-------------------------------*
			* Load Multi-Waveform from file *
			*-------------------------------*/
			FILE *multiWaveformFile = NULL;
			fopen_s( &multiWaveformFile, l_loadMultiSegmentWaveformParam.MULTI_WAVEFORM_DEFINITION_FILE, "rt" );

			if( NULL!=multiWaveformFile )
			{					
				char buffer[MAX_BUFFER_SIZE] = {'\0'};
				string line;
				vector<string> splits;
				while( !feof( multiWaveformFile ) )
				{
					fgets(buffer, MAX_BUFFER_SIZE, multiWaveformFile);

					int x=0;
					while (buffer[x])
					{			
						buffer[x] = toupper(buffer[x]);
						x++;
					}

					line = buffer;
					Trim( line );

					if( (0==line.size())||(string::npos!=line.find("#"))||(string::npos!=line.find("//")) )
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
							unsigned int  index = 9999;
							char fileBuffer[MAX_BUFFER_SIZE] = {'\0'};
						   /*----------------------------------------------*
							* Add a Waveform to the Multi-segment Waveform *
							*----------------------------------------------*/
							// A valid format must be "[Waveform Keyword From Global Settings]= [User Defined Mod File Name]".	
							Trim( splits[0] );
							Trim( splits[1] );
							sprintf_s(fileBuffer, MAX_BUFFER_SIZE, "%s/%s", g_WiFiGlobalSettingParam.PER_WAVEFORM_PATH, splits[1].c_str());
							err = LP_IQ2010EXT_AddWaveform( fileBuffer, &index );
							if ( ERR_OK!=err )
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to add a Waveform to the Multi-segment Waveform, LP_IQ2010EXT_AddWaveform(%s) return error.\n", fileBuffer);
								throw logMessage;
							}
							else
							{
								LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_AddWaveform(%s) return OK.\n", fileBuffer);
							}

							sprintf_s(fileBuffer, MAX_BUFFER_SIZE, "%s", splits[0].c_str());
							TM_AddMultiWaveformWithIndex( fileBuffer, index );
						}
						else
						{
							// do nothing
						}
						continue;
					}
				}
				fclose(multiWaveformFile);

			   /*----------------------------------------------*
				* Finalize the Multi-segment Waveform Creation *
				*----------------------------------------------*/
				err = LP_IQ2010EXT_FinalizeMultiSegmentWaveform();
				if ( ERR_OK!=err )
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Fail to finalize the Multi-segment Waveform Creation, LP_IQ2010EXT_FinalizeMultiSegmentWaveform() return error.\n");
					throw logMessage;
				}
				else
				{
					LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_INFORMATION, "[WiFi] LP_IQ2010EXT_FinalizeMultiSegmentWaveform() return OK.\n");
				}	

				splits.clear();
			}
			else
			{
				// Failed to open the multi-waveform definition file
				err = -1;
				LogReturnMessage(logMessage, MAX_BUFFER_SIZE, LOGGER_ERROR, "[WiFi] Failed to open the multi-waveform definition file.\n");
				throw logMessage;
			}

		}
		else
		{
			// do nothing
		}

		/*-----------------------*
		 *  Return Test Results  *
		 *-----------------------*/
		if (ERR_OK==err)
		{
			sprintf_s(l_loadMultiSegmentWaveformReturn.ERROR_MESSAGE, MAX_BUFFER_SIZE, "[Info] Function completed.\n");
			ReturnTestResults(l_loadMultiSegmentWaveformReturnMap);
		}
	}
	catch(char *msg)
    {
        ReturnErrorMessage(l_loadMultiSegmentWaveformReturn.ERROR_MESSAGE, msg);
    }
    catch(...)
    {
		ReturnErrorMessage(l_loadMultiSegmentWaveformReturn.ERROR_MESSAGE, "[WiFi] Unknown Error!\n");
    }

    return err;
}

int InitializeloadMultiSegmentWaveformContainers(void)
{
    /*------------------*
     * Input Parameters: *
     * IQTESTER_IP01    *
     *------------------*/
    l_loadMultiSegmentWaveformParamMap.clear();

    WIFI_SETTING_STRUCT setting;

    setting.unit        = "";
    setting.helpText    = "An user defined multi-Segment waveform definition file for IQtester VSG.";
    setting.type = WIFI_SETTING_TYPE_STRING;
    strcpy_s(l_loadMultiSegmentWaveformParam.MULTI_WAVEFORM_DEFINITION_FILE, MAX_BUFFER_SIZE, "Multi_Segment_Waveform.ini");
    if (MAX_BUFFER_SIZE==sizeof(l_loadMultiSegmentWaveformParam.MULTI_WAVEFORM_DEFINITION_FILE))    // Type_Checking
    {
        setting.value       = (void*)l_loadMultiSegmentWaveformParam.MULTI_WAVEFORM_DEFINITION_FILE;
        l_loadMultiSegmentWaveformParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("MULTI_WAVEFORM_DEFINITION_FILE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    l_loadMultiSegmentWaveformParam.RELOAD_MULTI_WAVEFORM_FILE = 0;
    setting.type = WIFI_SETTING_TYPE_INTEGER;
    if (sizeof(int)==sizeof(l_loadMultiSegmentWaveformParam.RELOAD_MULTI_WAVEFORM_FILE))    // Type_Checking
    {
        setting.value       = (void*)&l_loadMultiSegmentWaveformParam.RELOAD_MULTI_WAVEFORM_FILE;
        setting.unit        = "";
        setting.helpText    = "A flag to control reload multi-segment waveform file. Default = 0 (OFF)";
        l_loadMultiSegmentWaveformParamMap.insert( pair<string,WIFI_SETTING_STRUCT>("RELOAD_MULTI_WAVEFORM_FILE", setting) );
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
    l_loadMultiSegmentWaveformReturnMap.clear();

    l_loadMultiSegmentWaveformReturn.ERROR_MESSAGE[0] = '\0';
    setting.type = WIFI_SETTING_TYPE_STRING;
    if (MAX_BUFFER_SIZE==sizeof(l_loadMultiSegmentWaveformReturn.ERROR_MESSAGE))    // Type_Checking
    {
        setting.value       = (void*)l_loadMultiSegmentWaveformReturn.ERROR_MESSAGE;
        setting.unit        = "";
        setting.helpText    = "Error message occurred";
        l_loadMultiSegmentWaveformReturnMap.insert( pair<string,WIFI_SETTING_STRUCT>("ERROR_MESSAGE", setting) );
    }
    else    
    {
        printf("Parameter Type Error!\n");
        exit(1);
    }

    return 0;
}

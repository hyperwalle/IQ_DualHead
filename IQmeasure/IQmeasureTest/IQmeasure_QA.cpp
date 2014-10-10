#include "stdafx.h"
#include "iqapi.h"
#include "IQmeasure.h"
#include "DebugAlloc.h"
#include "math.h"
#include "time.h"
#include "vector"

#include "lp_consoleMode.h"


typedef struct __timeb64 lp_time_t;

#define DEBUG_PRINT true

#define FREQ_HZ			  2442e6 //5.53e9 //5550e6
#define SAMPLING_TIME_SEC 500e-6	 
#define VSA_TRIGGER_TYPE  IQV_TRIG_TYPE_IF2_NO_CAL
#define MAX_BUFFER_SIZE   8192
#define ASCII_ESC 27
// Carrier number
#define CARRIER_0    0
#define CARRIER_1    1
#define CARRIER_2    2
#define CARRIER_16   16
#define CARRIER_17   17
#define CARRIER_26   26
#define CARRIER_28   28
#define CARRIER_42   42
#define CARRIER_43   43
#define CARRIER_58   58
#define CARRIER_84   84
#define CARRIER_85   85
#define CARRIER_122  122

int _tmain(int argc, _TCHAR* argv[])
{
    if( argc>1 )
    {
    
        if( 0==wcscmp(argv[1],_T("-AC_QA")) )
        {
            AC_QA();
        }
	


	}

	set_color(CM_YELLOW);
	printf("\nPress Any Key to exit...\n");
    _getch();
			
	return 0;
}

void CheckReturnCode( int returnCode, char *functionName='\0' )
{
    if( ERR_OK!=returnCode )
    {
        //TODO
        if( NULL!=functionName )
        {
            sprintf_s(errMsg, MAX_BUFFER_SIZE, "%s returned error: %d", functionName, returnCode);
        }
        else
        {
            sprintf_s(errMsg, MAX_BUFFER_SIZE, "Error code: %d", returnCode);
        }
        throw errMsg;
    }
    else
        printf("%s, return OK.\n", functionName);

    return;
}

void AC_QA()

{
	char   buffer[MAX_BUFFER_SIZE];
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester
	double cableLoss = 1; //dB of path loss
	double maxCaptureTime=0.0, minCaptureTime=1e6, maxAnalysisTime=0.0, minAnalysisTime=1e6;
	string tmpStr1 = "", modFileName = ""; // saveCapturedName="";


			char saveCapturedName[MAX_BUFFER_SIZE];
		CheckReturnCode( LP_Init(), "LP_Init()" );	
		CheckReturnCode( LP_InitTester(IP_ADDR), "LP_InitTester()" );

        if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	
            printf("\n%s\n\n", buffer);
		int iRun=0, numRun=100;

}

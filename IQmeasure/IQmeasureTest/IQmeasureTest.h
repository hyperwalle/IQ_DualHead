typedef struct __timeb64 lp_time_t;

#define DEBUG_PRINT true
//#define IP_ADDR			  "192.168.109.177"
//#define IP_ADDR			  "192.168.109.180"
//#define IP_ADDR			  "192.168.109.181" //"IQXS00120"
//#define IP_ADDR			  "192.168.109.182"
//#define IP_ADDR			  "192.168.109.183"
//#define IP_ADDR			  "127.0.0.1"
//#define IP_ADDR			      "192.168.2.206"  //DTNA-00009
//#define IP_ADDR			  "192.168.1.153"  //DTNA-00018
//#define IP_ADDR			  "192.168.0.241"  //DTNA-00024
//#define IP_ADDR			  "192.168.1.126"  //DTNA-00024
//#define IP_ADDR			  "192.168.0.8"  //Daytona-B10
//#define IP_ADDR			  "192.168.1.249"  //DTNA-00031
//#define IP_ADDR			  "192.168.2.140"  //DTNA-00032
//#define IP_ADDR			  "192.168.1.143"  //DTNA-00048
//#define IP_ADDR			  "192.168.0.57"  //DTNA-00125
//#define IP_ADDR			  "192.168.2.245"  //DTNA-00114
//#define IP_ADDR			  "192.168.2.37"  //DTNA-00031
//#define IP_ADDR			  "192.168.1.3"  //DTNA-00031
//#define IP_ADDR			  "192.168.2.196"  //DTNA-00023
//#define IP_ADDR			  "192.168.3.44"  //DTNA-00040
//#define IP_ADDR			  "192.168.2.198"  //DTNA-00054
#define IP_ADDR				  "127.0.0.1"
#define IP_ADDR_IQXEL		  "192.168.100.253"  //DTNA-00033
#define IP_ADDR_IQXEL1		  "192.168.2.74"  //DTNA-00033
#define IP_ADDR_IQXEL2		  "192.168.2.74"  //DTNA-00033
#define IP_ADDR_IQXEL3		  "192.168.2.74"  //DTNA-00033
#define IP_ADDR_IQXEL4		  "192.168.0.219"  

#define FREQ_HZ			  2442e6 //5.53e9 //5550e6
#define FREQ_HZ_5G			  5775e6
#define SAMPLING_TIME_SEC 500e-6
#define VSA_TRIGGER_TYPE  IQV_TRIG_TYPE_IF2_NO_CAL
//#define MAX_BUFFER_SIZE   8192
#define MAX_FILENAME_SIZE  128   //tyu;2012-07-11; QA BT
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


#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

void QA_CW(void);
void QA_11AC(void);
void QA_11AC2(void);
void QA_11ag(void);
void QA_11b(void);
void QA_11n(void);
void QA_FrameCount(void);
void QA_Mask(void);
void QA_Misc(void);
void QA_BT(void);   //tyu;2012-07-11; QA BT
void QA_Dualhead(void);  //tyu;2012-07-16; QA Dualhead


void CheckReturnCode( int returnCode, char *functionName='\0' );
unsigned int GetElapsedMSec(lp_time_t start, lp_time_t end);
void ReadLogFiles (void);





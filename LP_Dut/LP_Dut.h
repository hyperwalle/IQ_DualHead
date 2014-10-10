#ifndef _LP_DUT_H_
#define _LP_DUT_H_


#if !defined (__cplusplus)
	#define LP_DUT_API extern
#elif !defined (WIN32)
	#define LP_DUT_API extern "C"
#elif defined(LP_DUT_EXPORTS)
	#define LP_DUT_API extern "C" __declspec(dllexport)
#else
	#define LP_DUT_API extern "C" __declspec(dllimport) 
#endif

#ifndef MAX_BUFFER_SIZE
	#define MAX_BUFFER_SIZE 4096
#endif
#ifndef DELAY_FOR_POWER_SETTLE_MS
	#define DELAY_FOR_POWER_SETTLE_MS	500
#endif


// WiFi DUT DLL needs to export only one function: WIFI_DutRegister()
LP_DUT_API int WIFI_DutRegister(void);


int LP_dut_info(void);
int LP_insert_dut(void);
int LP_remove_dut(void);
int LP_initialize_dut(void);
int LP_prepare_tx(void);
int LP_tx_set_bw(void);
int LP_tx_set_data_rate(void);
int LP_set_freq(void);
int LP_tx_set_antenna(void);
int LP_tx_start(void);
int LP_tx_stop(void);
int LP_prepare_rx(void);
int LP_rx_set_bw(void);
int LP_rx_set_data_rate(void);
int LP_set_bssid_filter(void);
int LP_set_rx_antenna(void);
int LP_clear_rx_pckts(void);
int LP_get_rx_stats(void);
int LP_rx_start(void);
int LP_rx_stop(void);
int LP_get_serial_number(void);
int LP_write_mac_address(void);
int GetWaveformFileName(char* filePath, char* fileType, int HT40ModeOn, char* datarate, char* preamble, char* packetFormat11N, char* waveformFileName, int bufferSize);
int LP_query_power_dbm(void);
int LP_bt_tx_start(void);
int LP_btle_tx_start(void);
int LP_bt_get_rx_stats(void);
int LP_bt_clear_rx_pckts(void);

#endif 
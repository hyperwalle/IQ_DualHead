/* Version Histroy */
/* 
version: 1.2
date: 2011-04-29
1. Update function IP_Exits() to fulfil ARP -d with windows API

version: 1.1
date: 2011-04-13
1. Add function IP_KillProcName
*/
#ifdef _daixin_support
#define WINAPIN_DLL extern "C" __declspec(dllexport)
#else
#define WINAPIN_DLL extern "C" __declspec(dllimport)
#endif

#ifndef _dx_get_adapter_info_
#define _dx_get_adapter_info_
/* 判斷PC上是否有IP設定在 pIpAddr 網段的網卡存在, 有則返回true, 反之返回false */
/* 需傳入完整的目標IP地址, 函數會刪除ARP緩存并解析IP的前綴,以確認是否有網卡與產品在同一網段 */
/* 此函數亦可用來判斷指定IP網段的網卡是否有網線插入并link */
WINAPIN_DLL bool IP_Exits(char *pIpAddr/*in*/);
/* 確認 IP 地址 pIpAddr 是否可以 ping 通, uiPingTimes 為次數設定,uiPacketSize */
/* 為封包大小，只有有一次 ping OK, 即返回 true, 反之返回 false */
WINAPIN_DLL bool IP_Ping(char *pIpAddr/*in*/,unsigned int uiPingTimes/*in*/, unsigned int uiPacketSize/*in, max=65535*/);
/* 確認 IP 地址 pIpAddr 是否可以 ping 通, uiPingTimes 為次數設定,uiPacketSize */
/* 為封包大小，只有有一次 ping fail, 即返回 false, 反之返回 true */
WINAPIN_DLL bool IP_PingContinous(char *pIpAddr/*in*/,unsigned int uiPingTimes/*in*/, unsigned int uiPacketSize/*in, max=65535*/);
/* 執行 telnetenable, 若執行成功則返回 true, 反之返回 false */
WINAPIN_DLL bool IP_TelnetEnable(char *pIpAddr/*in*/,char* pMac/*in*/);
/* 獲取遠端 IP 設備所屬的 MAC 地址, pMac 容量不可少于13位, 若找到該IP并獲取到 MAC, 返回 true ,反之返回 false */
WINAPIN_DLL bool IP_ArpMac(char *pIpAddr/*in*/,char* pMac/*out*/);
/* 終止一個指定的進程 */
WINAPIN_DLL bool IP_KillProcName(char *pProcName/*in*/);
#endif
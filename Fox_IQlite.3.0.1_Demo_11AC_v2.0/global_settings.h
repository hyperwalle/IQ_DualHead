#include "stdafx.h"
#include <iomanip>
using namespace std;
#pragma once
#define Char2WChar(lpMultiByteStr,cbMultiByte,lpWideCharStr,cbWideChar)	MultiByteToWideChar(CP_ACP,0,lpMultiByteStr,cbMultiByte,lpWideCharStr,cbWideChar) 
#define WChar2Char(lpWideCharStr,cbWideChar,lpMultiByteStr,cbMultiByte) WideCharToMultiByte(CP_ACP,0,lpWideCharStr,cbWideChar,lpMultiByteStr,cbMultiByte,NULL,NULL)

int read_Global_Settings(char* pszGlobalSettingFileName);
int run_Global_Settings_AddParam(TM_ID TM_Test);
int ParseTestItem(char *pszScriptFile);
bool ShowTestResultEVM(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn, int iTestCount);
bool ShowTestResultMask(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn, int iTestCount);
bool ShowTestResultPower(TX_PARAM_IN *txParam, TX_PARAM_RETURN *txReturn, int iTestCount);
bool ShowTestResultPER(RX_PARAM_IN *rxParam, RX_PARAM_RETURN *rxReturn, int iTestCount);
int myprintf(const char* pstr);

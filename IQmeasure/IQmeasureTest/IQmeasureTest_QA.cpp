// IQmeasureTest_QA.cpp : Defines the entry point for the console application.
// Only for QA purpose

#include "stdafx.h"
#include "iqapi.h"
#include "..\IQmeasure.h"
#include "..\DebugAlloc.h"
#include "math.h"
#include "time.h"
#include "vector"
//Move to stdafx.h
//#include "lp_consoleMode.h"
#include "IQmeasureTest.h"


extern void CheckReturnCode( int returnCode, char *functionName );
extern unsigned int GetElapsedMSec(lp_time_t start, lp_time_t end);

extern int		g_logger_id;
extern char    g_csvFileName[128];
extern  int Tag_runTime;
extern  int		ciTesterControlMode;
extern  int		ciTesterType;
extern  char    g_IP_addr[128];
extern int     g_DH_TOKEN_ID;
extern int	    g_DH_PORT;


void GetTime(lp_time_t& time)
{
	_ftime64_s(&time);
}

void QA_CW(void)
{
	char   buffer[MAX_BUFFER_SIZE];
	char   saveCapturedName[MAX_BUFFER_SIZE];

	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(ciTesterType, ciTesterControlMode), "LP_Init()" );

		//tyu;2012-7-6; Timer log
		if (ciTesterType == IQTYPE_XEL)
		{
			if (ciTesterControlMode == 0)  // IQapi_scpi
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_QA_CW_Run_%d.csv",Tag_runTime);
			}
			else   //SCPI
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_QA_CW_Run_%d.csv",Tag_runTime);
			}
		}
		else
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQ2010_QA_CW_Run_%d.csv",Tag_runTime);
		}
		//End

		//tyu; 2012-7-6; IP adddress is set in the setup file
		CheckReturnCode( LP_InitTester(g_IP_addr), "LP_InitTester()" );


		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)	printf("%s\n", buffer);

		double vsgPowerdBm = -10;
		double freqHz = 5180e6;
		double offsetFreqHz = 0;
		double result;



		CheckReturnCode( LP_SetVsgCw(freqHz, offsetFreqHz, vsgPowerdBm, PORT_RIGHT), "LP_SetVsg()" );

		//---------------//
		//  Turn on VSG  //
		//---------------//
		CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

		CheckReturnCode( LP_SetVsa(freqHz, vsgPowerdBm, PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?

		CheckReturnCode( LP_VsaDataCapture(250e-6, 0, 160e6), "LP_VsaDataCapture()" ); //250us capture, free-run; 160Mhz sample rate

		sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s.iqvsa", "CW");


		CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

		CheckReturnCode( LP_AnalyzeCW(), "LP_AnalyzeCW()" );

		result = LP_GetScalarMeasurement("frequency");
		printf("frequency: %.2f Hz\n", result);

		CheckReturnCode( LP_EnableVsgRF(0), "LP_DisableVsgRF()" );


		//----------------------------//
		//   Disconnect the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Term(), "LP_Term()" );
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}
}

void QA_11AC(void)
{
	char   buffer[MAX_BUFFER_SIZE];
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester
	double cableLoss = 1;           //dB of path loss
	double maxCaptureTime=0.0, minCaptureTime=1e6, maxAnalysisTime=0.0, minAnalysisTime=1e6;
	double freqHz;

	//----------------------------//
	//   Output File Operation    //
	//----------------------------//
	FILE *fp_power, *fp_analysis1, *fp_analysis2, *fp_analysis3;
	char *outFileName  = ".\\log\\11AC_QA_Power.csv";
	char *outFileName1 = ".\\log\\11AC_QA_Analysis1.csv";
	char *outFileName2 = ".\\log\\11AC_QA_Analysis2.csv";
	char *outFileName3 = ".\\log\\11AC_QA_Analysis3.csv";



	try
	{
		//CheckReturnCode( LP_Init(ciTesterType, ciTesterControlMode), "LP_Init()" );
		//	CheckReturnCode( LP_InitTester(g_IP_addr), "LP_InitTester()" );
		//CheckReturnCode( LP_Term(), "LP_Term()" );

		fopen_s(&fp_power,outFileName,"w+");
		fopen_s(&fp_analysis1,outFileName1,"w+");
		fopen_s(&fp_analysis2,outFileName2,"w+");
		fopen_s(&fp_analysis3,outFileName3,"w+");

		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		//the second parameter (0: using IQApi, 1: using SCPI)
		//const int ciTesterControlMode = 1;
		CheckReturnCode( LP_Init(ciTesterType, ciTesterControlMode), "LP_Init()" );

		//tyu;2012-7-6; Timer log
		if (ciTesterControlMode == 0)  // IQapi_scpi
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_QA_11AC_Run_%d.csv",Tag_runTime);
		}
		else   //SCPI
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_QA_11AC_Run_%d.csv",Tag_runTime);
		}
		//End


		//tyu; 2012-7-6; IP adddress is set in the setup file
		CheckReturnCode( LP_InitTester(g_IP_addr), "LP_InitTester()" );


		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)
			printf("\n%s\n\n", buffer);
		int iRun=0, numRun=5;

		double maxPWR=-999.9, minPWR=999.9,avgPwrSpectral=0;
		double maxEVM=-999.9, minEVM=999.9,avgEVM=0;
		string modFileName = "";
		string tempString = "";
		char saveCapturedName[MAX_BUFFER_SIZE];

		for( int freq_id = 1; freq_id <= 2; freq_id++)
			for (int signal_id = 1; signal_id <= 3; signal_id++)

			{

				if (1==freq_id)
					freqHz = 2422e6;
				else if (2==freq_id)
					freqHz = 5180e6;

				if (1==signal_id)
					tempString = "WiFi_11AC_VHT80_S1_MCS9";
				//else if (2==signal_id)
				//	tempString = "11AC_MCS9_1strm_80MHz_2850bytes_pgap20us_LongGI_AMPDUoff_Fs160";
				else if (2==signal_id)
					tempString = "WiFi_11AC_VHT40_S1_MCS9";
				else if (3==signal_id)
					tempString = "WiFi_11AC_VHT20_S1_MCS8";
				else
				{
				}

				//-------------//
				//  Setup VSG  //
				//-------------//

				CheckReturnCode( LP_SetVsg(freqHz, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

				//--------------------//
				// Load waveform file //
				//--------------------//
				//modFileName = "../mod/"; //Upload the *.iqvsg to user folder first
				modFileName = "../../iqvsg/";//Luke
				modFileName+=tempString;
				modFileName+=".iqvsg";
				printf("\n Loading mode file %s\n", modFileName.c_str());
				CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );

				//---------------//
				//  Turn on VSG  //
				//---------------//
				CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				//------------------------------------------//
				//   Send packet, continuous transmission   //
				//tyu; 2012-07-07;
				//------------------------------------------//
				int frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
				int err =0;
				CheckReturnCode( LP_SetFrameCnt(1000), "LP_SetFrameCnt(1000)" );
				while (true)
				{
					err = LP_TxDone();
					if ( err != 0)
						continue;
					else
						break;
				}
				CheckReturnCode( LP_SetFrameCnt(0), "LP_SetFrameCnt()" );

				//End

				//----------------//
				//   Setup VSA    //
				//----------------//
				int peakToAverageRatio = 10;
				double samplingRate = 160e6;
				double captureTime;
				CheckReturnCode( LP_SetVsa(freqHz, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?

				//----------------------//
				//  Perform VSA capture //
				//----------------------//
				//Cannot use loopback test in VHT80.
				for (int trigger_type = 1; trigger_type <=13; trigger_type+=12 )
					//for (int capture_type = 0; capture_type <6; capture_type+=6 )
					for (int capture_type = 0; capture_type <=6; capture_type+=6 )

					{

						lp_time_t starttime, stoptime;
						if (13 == trigger_type)
							captureTime = 206e-6;
						else
							captureTime = 2000e-6;

						fprintf(fp_power,"\n=====%.2f====%s ====",freqHz,tempString.c_str());
						fprintf(fp_power,"=======Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_power,"=====Valid, P_av_each_burst_dBm, P_av_each_burst, P_pk_each_burst_dBm, P_pk_each_burst, P_av_all_dBm, P_av_all, P_peak_all_dBm, P_peak_all, P_av_no_gap_all_dBm, P_av_no_gap_all, start_sec, stop_sec=======================\n");

						fprintf(fp_analysis1,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis1,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis1,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						fprintf(fp_analysis2,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis2,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis2,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						fprintf(fp_analysis3,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis3,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis3,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						int iRun=0, numRun=5;
						while(iRun<numRun)
						{
							iRun++;
							printf("_________________________________________________\n");
							printf("Run %d\n", iRun);

							GetTime(starttime);

							CheckReturnCode( LP_VsaDataCapture(captureTime, trigger_type, samplingRate, capture_type), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?

							GetTime(stoptime);

							int elapsed_ms = GetElapsedMSec(starttime, stoptime);

							if (13 == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Signal trigger& Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Signal trigger& VHT 80 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}


							if (1  == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Free Run& Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Free Run& VHT 80 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}

							if (elapsed_ms<minCaptureTime)
								minCaptureTime=elapsed_ms;
							if(elapsed_ms>maxCaptureTime)
								maxCaptureTime=elapsed_ms;
							//---------------------------//
							//  Save sig file for debug  //
							//---------------------------//

							sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.iqvsa", "FastTrack_Capture", tempString.c_str(),iRun);
							//Need the Log folder to exist first.
							CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

							double result,result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13,result14,result15,result16,result17,result18;

							//----------------//
							//  Mask Analysis //
							//----------------//
							if ( 0 == capture_type)
							{
								CheckReturnCode(LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );
								result1 = LP_GetScalarMeasurement("valid"); //result not right yet.
								printf("FFT analyze valid: %.0f\n", result1);
								result1 = LP_GetScalarMeasurement("length"); //result not right yet.
								printf("Number of point on spectrum: %.0f\n", result1);

							}
							else
								if ( 6 == capture_type)
								{
									CheckReturnCode(LP_AnalyzeVHT80Mask(), "LP_AnalyzeVHT80Mask()" );
								}
							//-------------------------//
							//  Retrieve Test Results  //
							//-------------------------//

							if( ciTesterControlMode == 0 )//IQAPI
								CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );
							else if( ciTesterControlMode == 1 )//SCPI
								CheckReturnCode( LP_AnalyzePower(3.2e-6, 15.0), "LP_AnalyzePower()" );

							//CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );
							result1 = LP_GetScalarMeasurement("valid"); //result not right yet.
							printf("valid: %.0f\n", result1);

							result2 = LP_GetScalarMeasurement("P_av_each_burst_dBm");
							printf("P_av_each_burst_dBm: %.2f dBm\n", result2);

							result3 = LP_GetScalarMeasurement("P_av_each_burst");
							printf("P_av_each_burst: %.6f mw\n", result3);

							result4 = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
							printf("P_pk_each_burst_dBm: %.2f dBm\n", result4);

							result5 = LP_GetScalarMeasurement("P_pk_each_burst");
							printf("P_pk_each_burst: %.6f mw\n", result5);

							result6 = LP_GetScalarMeasurement("P_av_all_dBm");
							printf("P_av_all_dBm: %.2f dBm\n", result6);

							result7 = LP_GetScalarMeasurement("P_av_all");
							printf("P_av_all: %.6f mw\n", result7);

							result8 = LP_GetScalarMeasurement("P_peak_all_dBm");
							printf("P_peak_all_dBm: %.2f dBm\n", result8);

							result9= LP_GetScalarMeasurement("P_peak_all");
							printf("P_peak_all: %.6f mw\n", result9);

							result10 = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
							printf("P_av_no_gap_all_dBm: %.2f dBm\n", result10);

							result11 = LP_GetScalarMeasurement("P_av_no_gap_all");
							printf("P_av_no_gap_all: %.6f mw\n", result11);

							result12 = LP_GetScalarMeasurement("start_sec");
							printf("start_sec: %.6f sec\n", result12);

							result13 = LP_GetScalarMeasurement("stop_sec");
							printf("stop_sec: %.6f sec\n", result13);

							fprintf(fp_power,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.8f,%.8f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13);
							//11AC analysis

							printf("\nUsing 11AC analysis, iqapiAnalysis11ac\n");
							lp_time_t starttime4, stoptime4;

							for (int analysis_option_combination =1; analysis_option_combination<=3;analysis_option_combination++)
							{
								if (1==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac(), "LP_Analyze80211ac()" );
								}
								if (2==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac("nxn",0,0,1,1,1,0), "LP_Analyze80211ac()" );
								}
								if (3==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac("nxn",0,0,1,1,1,1), "LP_Analyze80211ac()" );
								}

								GetTime(stoptime4);

								int elapsed_ms4 = GetElapsedMSec(starttime4, stoptime4);
								printf("************ Elapsed time for LP_Analyze80211ac() option %d: %d ms ************\n", analysis_option_combination, elapsed_ms4);

								if (elapsed_ms4<minAnalysisTime)
									minAnalysisTime=elapsed_ms4;

								if(elapsed_ms4>maxAnalysisTime)
									maxAnalysisTime=elapsed_ms4;

								//11n and 11ac result

								result1 = LP_GetScalarMeasurement("psduCRC");
								if (result1 == 1)
								{
									printf("PSDU CRC: %s \n", "PASS");
								}
								else
									printf("PSDU CRC: %s \n", "FAIL");

								//Not working for 1 stream signal.
								// result = LP_GetScalarMeasurement("isolationDb");
								// printf("isolationDb=%0.2f \n", result);

								result2 = LP_GetScalarMeasurement("freqErrorHz");
								printf("Frequency Error: %.2f Hz\n", result2);

								result3 = LP_GetScalarMeasurement("symClockErrorPpm");
								printf("Symbol Clock Error: %.2f ppm\n", result3);

								result4 = LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll");
								printf("RMS Phase Noise: %.2f deg\n", result4);

								result5 = LP_GetScalarMeasurement("IQImbal_amplDb");
								printf("IQ Imbalance Amp: %.2f dB \n", result5);

								result6 = LP_GetScalarMeasurement("IQImbal_phaseDeg");
								printf("IQ Imbalance phase: %0.2f deg \n", result6);

								result7 = LP_GetScalarMeasurement("dcLeakageDbc");
								printf("LO leakage: %.2f dBc\n\n", result7);

								double bandwidthMhz = LP_GetScalarMeasurement("rateInfo_bandwidthMhz");
								printf("Signal Bandwidth: %0.2f MHz\n", bandwidthMhz);

								result8 = LP_GetScalarMeasurement("rateInfo_spatialStreams");
								printf("Number of Spatial Streams: %1.0f \n", result8);

								result9 = LP_GetScalarMeasurement("rateInfo_spaceTimeStreams");
								printf("Number of SpaceTime Streams: %1.0f \n", result9);

								char   result_char1[255];
								char   result_char2[255];

								LP_GetStringMeasurement("rateInfo_codingRate",result_char1,255);
								printf("codingRate: %s \n", result_char1);

								LP_GetStringMeasurement("rateInfo_modulation",result_char2,255);
								printf("modulation: %s \n", result_char2);

								result10 = LP_GetScalarMeasurement("rateInfo_dataRateMbps");
								printf("Datarate: %0.2f Mbps\n", result10);

								result11 = LP_GetScalarMeasurement("rxRmsPowerDb");
								printf("Power_NoGap: %.3f dBm\n", result11);

								if (result11 > maxPWR)
								{
									maxPWR=result11;
								}
								if(result11<minPWR)
								{
									minPWR=result11;
								}
								avgPwrSpectral+=result11;

								result12 = LP_GetScalarMeasurement("evmAvgAll");
								printf("EVM Avg All: %.3f dB\n\n", result12);

								if (result12 > maxEVM)
								{
									maxEVM=result12;
								}
								if(result12<minPWR)
								{
									minEVM=result12;
								}
								avgEVM+=result12;

								//			result = LP_GetScalarMeasurement("htSigFieldCRC");
								//			printf("htSigFieldCRC=%0.2f \n", result);

								result = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
								if (result <= 0)
								{
									printf("Channel Estimation returns an error\n");
								}
								else
								{
									//cacluate the spectrum flatness.
									printf("Channel Estimation returns a vector with %3.0f elements\n", result);
									int numSubcarrier = 256; //number of Subcarrier for 80MHz
									double  *carrierPwr;
									int centerStart, centerEnd, sideStart, sideEnd;
									//iqPwr  = (double *) malloc (result * sizeof (double));
									carrierPwr     = (double *) malloc (numSubcarrier * sizeof (double));
									double avgCenterPwr=0.0, avgSidePwr=0.0, avgPwrSpectral=0.0;
									double maxCarrierPwr = -999.9, minCenterCarrierPwr = 99.9, minSideCarrierPwr = 99.9;

									if (bandwidthMhz == 20.0)
									{
										numSubcarrier = 64;
										centerStart   = CARRIER_1;
										centerEnd     = CARRIER_16;
										sideStart     = CARRIER_17;
										sideEnd       = CARRIER_28;
									}
									else
									{
										if (bandwidthMhz == 40.0)
										{
											numSubcarrier = 128;
											centerStart   = CARRIER_2;
											centerEnd     = CARRIER_42;
											sideStart     = CARRIER_43;
											sideEnd       = CARRIER_58;
										}
										else
										{
											numSubcarrier = 256;
											centerStart   = CARRIER_2;
											centerEnd     = CARRIER_84;
											sideStart     = CARRIER_85;
											sideEnd       = CARRIER_122;
										}
									}
									if (result != numSubcarrier)
									{
										printf("Channel estimation results does not match number of subcarriers for %2.0f MHz signal.\n", bandwidthMhz);
									}
									else
									{
										for (int carrier=0;carrier<result;carrier++) //only one stream here.
										{
											carrierPwr[carrier] = bufferReal[carrier]*bufferReal[carrier]+ bufferImag[carrier]*bufferImag[carrier];
										}
										// average power in the center
										for ( int i=centerStart;i<=centerEnd;i++)
										{
											avgCenterPwr = avgCenterPwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
										}
										// average power on side lobe.
										for (int i=sideStart;i<=sideEnd;i++)
										{
											avgSidePwr = avgSidePwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
										}

										avgPwrSpectral = avgCenterPwr+ avgSidePwr;

										avgCenterPwr = avgCenterPwr/((centerEnd-centerStart+1)*2);
										if (0!=avgCenterPwr)
										{
											avgCenterPwr = 10.0 * log10 (avgCenterPwr);
										}
										else
										{
											avgCenterPwr= 0;
										}
										avgPwrSpectral = avgPwrSpectral/((sideEnd-centerStart+1)*2);
										if (0!=avgPwrSpectral)
										{
											avgPwrSpectral = 10.0 * log10 (avgPwrSpectral);
										}
										else
										{
											avgPwrSpectral = 0;
										}

										for (int carrier=0;carrier<numSubcarrier;carrier++)
										{
											if (0!=carrierPwr[carrier])
											{
												carrierPwr[carrier] = 10.0 * log10 (carrierPwr[carrier]);
												if (carrierPwr[carrier]>maxCarrierPwr)
												{
													maxCarrierPwr = carrierPwr[carrier];
												}
												if ( (carrier>=centerStart && carrier <=centerEnd) || (carrier>=numSubcarrier-centerEnd && carrier <= numSubcarrier-centerStart) )
												{
													if(carrierPwr[carrier]< minCenterCarrierPwr)
													{
														minCenterCarrierPwr = carrierPwr[carrier];
													}
												}
												if ((carrier>=sideStart && carrier <=sideEnd) || (carrier>=numSubcarrier-sideEnd && carrier <=numSubcarrier-sideStart) )
												{
													if(carrierPwr[carrier]< minSideCarrierPwr)
													{
														minSideCarrierPwr = carrierPwr[carrier];
													}
												}
											}
											else
											{
												carrierPwr[carrier] = 0;
											}
										}
										printf("The maximum deviation is %0.2f dB\n", maxCarrierPwr-avgCenterPwr);
										printf("The minimum deviation are %0.2f dB in the center, %0.2f dB in the far side\n", minCenterCarrierPwr-avgCenterPwr, minSideCarrierPwr-avgCenterPwr);
										printf("The LO Leakage is %0.2f dBc \n", carrierPwr[0] - avgPwrSpectral - 10 * log10((double)((sideEnd-centerStart+1)*2)) );
									}

								}

								printf("11AC VHT Sig field Info:\n");
								result13 = LP_GetScalarMeasurement("vhtSigA1Bandwidth");
								printf("VHTSigA1Bandwidth=%1.0f. Means: %3.0f MHz\n", result13, pow(2,result13)*20);

								result14 = LP_GetScalarMeasurement("vhtSigA1Stbc");
								printf("VHTSigA1Stbc=%1.0f. Means: ", result14);
								if(result14 == 0)
								{
									printf("STBC is not enabled for all streams.\n");
								}
								else
									printf("STBC is enabled for all streams.\n");
								//printf("VHTSigA1Stbc=%1.0f\n", result);

								result15 = LP_GetScalarMeasurement("vhtSigA2ShortGI");
								printf("VHTSigA2ShortGI=%1.0f\n", result);
								result16 = LP_GetScalarMeasurement("vhtSigA2AdvancedCoding");
								printf("VHTSigA2AdvancedCoding=%1.0f\n", result);
								result17 = LP_GetScalarMeasurement("vhtSigA2McsIndex");
								printf("VHTSigA2McsIndex=%1.0f\n", result);

								result18 = LP_GetScalarMeasurement("VHTSigBFieldCRC");
								printf("vhtSigBFieldCRC=%1.0f\n\n", result);

								if(1== analysis_option_combination)
									fprintf(fp_analysis1,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);

								if(2== analysis_option_combination)
									fprintf(fp_analysis2,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);

								if(3== analysis_option_combination)
									fprintf(fp_analysis3,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);


							}
							//----------------//
							//  Turn off VSG  //
							//----------------//

							printf("\n");

							//printf("Press 'X' key to exit or any key to repeat the test..................\n");

							//char ch = toupper(_getch());
							//if (ch=='X' || ch==ASCII_ESC)
							//    break;

						}  //analysis combination for-loop
					}  //VSA capture for-loop&trigger for-loop
				CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
			} //freq and vsg file for-loop
		printf("Statistics of %d run:\n",numRun);
		printf("Power_NoGap\n	Avg: %.2f dBm	Max: %.2f dBm	Min: %.2f dBm\n", avgPwrSpectral/numRun,maxPWR,minPWR);
		printf("EVM\n	Avg: %.2f dB	Max: %.2f dB	Min: %.2f dB\n\n", avgEVM/numRun,maxEVM,minEVM);
		printf("Capture time:  Max: %.0f ms	Min: %.0f ms\n", maxCaptureTime,minCaptureTime);
		printf("Analysis time:  Max: %.0f ms	Min: %.0f ms\n", maxAnalysisTime,minAnalysisTime);
	}


	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
	fclose(fp_power);
	fclose( fp_analysis1 );
	fclose(fp_analysis2);
	fclose(fp_analysis3);
	ReadLogFiles();
}




void QA_11AC2(void)
{
	char   buffer[MAX_BUFFER_SIZE];
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester
	double cableLoss = 1;           //dB of path loss
	double maxCaptureTime=0.0, minCaptureTime=1e6, maxAnalysisTime=0.0, minAnalysisTime=1e6;
	double freqHz;

	//----------------------------//
	//   Output File Operation    //
	//----------------------------//
	FILE *fp_power, *fp_analysis1, *fp_analysis2, *fp_analysis3;
	char *outFileName  = "11AC_QA_Power.csv";
	char *outFileName1 = "11AC_QA_Analysis1.csv";
	char *outFileName2 = "11AC_QA_Analysis2.csv";
	char *outFileName3 = "11AC_QA_Analysis3.csv";



	try
	{
		fopen_s(&fp_power,outFileName,"w+");
		fopen_s(&fp_analysis1,outFileName1,"w+");
		fopen_s(&fp_analysis2,outFileName2,"w+");
		fopen_s(&fp_analysis3,outFileName3,"w+");


		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(ciTesterType), "LP_Init()" );

		//tyu;2012-7-6; Timer log
		if (ciTesterType == IQTYPE_XEL)
		{
			if (ciTesterControlMode == 0)  // IQapi_scpi
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_QA_11AC2_Run_%d.csv",Tag_runTime);
			}
			else   //SCPI
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_QA_11AC2_Run_%d.csv",Tag_runTime);
			}
		}
		else
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQ2010_QA_11AC2_Run_%d.csv",Tag_runTime);
		}
		//End

		CheckReturnCode( LP_InitTester(IP_ADDR_IQXEL), "LP_InitTester()" );
		//CheckReturnCode( LP_InitTester("IQP08376"), "LP_InitTester()" );
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)
			printf("\n%s\n\n", buffer);
		int iRun=0, numRun=5;

		double maxPWR=-999.9, minPWR=999.9,avgPwrSpectral=0;
		double maxEVM=-999.9, minEVM=999.9,avgEVM=0;
		string modFileName = ""; // saveCapturedName="";
		string tempString = "";
		char saveCapturedName[MAX_BUFFER_SIZE];

		for( int freq_id = 1; freq_id <= 2; freq_id++)
			for (int signal_id = 1; signal_id <= 4; signal_id++)

			{

				if (1==freq_id)
					freqHz = 2422e6;
				else if (2==freq_id)
					freqHz = 5180e6;

				if (1==signal_id)
					tempString = "11AC_MCS8_1strm_80MHz_2550bytes_pgap20us_LongGI_AMPDUoff_Fs160";
				else if (2==signal_id)
					tempString = "11AC_MCS9_1strm_80MHz_2850bytes_pgap20us_LongGI_AMPDUoff_Fs160";
				else if (3==signal_id)
					tempString = "11AC_MCS8_1strm_20MHz_1500bytes_pgap20us_LongGI_AMPDUoff_Fs160";
				else if (4==signal_id)
					tempString = "11AC_MCS9_1strm_40MHz_3000bytes_pgap20us_LongGI_AMPDUoff_Fs160";

				//-------------//
				//  Setup VSG  //
				//-------------//

				CheckReturnCode( LP_SetVsg(FREQ_HZ, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

				//--------------------//
				// Load waveform file //
				//--------------------//
				//modFileName = "../mod/";
				modFileName = "../../iqvsg/";//Luke
				modFileName+=tempString;
				modFileName+=".iqvsg";
				printf("\n Loading mode file %s\n", modFileName.c_str());
				CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );

				//---------------//
				//  Turn on VSG  //
				//---------------//
				CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				//----------------//
				//   Setup VSA    //
				//----------------//
				int peakToAverageRatio = 10;
				double samplingRate = 160e6;
				double captureTime;

				CheckReturnCode( LP_SetVsa(FREQ_HZ, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?

				//CheckReturnCode( LP_SetVsa(FREQ_HZ, -15, PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" );

				//----------------------//
				//  Perform VSA capture //
				//----------------------//
				for (int trigger_type = 1; trigger_type <=13; trigger_type+=12 )
					for (int capture_type = 0; capture_type <=6; capture_type+=6 )

					{

						lp_time_t starttime, stoptime;
						if (13 == trigger_type)
							captureTime = 206e-6;
						else
							captureTime = 500e-6;

						fprintf(fp_power,"\n=====%.2f====%s ====",freqHz,tempString.c_str());
						fprintf(fp_power,"=======Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_power,"=====Valid, P_av_each_burst_dBm, P_av_each_burst, P_pk_each_burst_dBm, P_pk_each_burst, P_av_all_dBm, P_av_all, P_peak_all_dBm, P_peak_all, P_av_no_gap_all_dBm, P_av_no_gap_all, start_sec, stop_sec=======================\n");

						fprintf(fp_analysis1,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis1,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis1,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						fprintf(fp_analysis2,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis2,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis2,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						fprintf(fp_analysis3,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis3,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis3,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						int iRun=0, numRun=5;
						while(iRun<numRun)
						{
							iRun++;
							printf("_________________________________________________\n");
							printf("Run %d\n", iRun);

							GetTime(starttime);

							CheckReturnCode( LP_VsaDataCapture(captureTime, trigger_type, samplingRate, capture_type), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?

							GetTime(stoptime);

							int elapsed_ms = GetElapsedMSec(starttime, stoptime);

							if (13 == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Signal trigger& Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Signal trigger& VHT 80 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}


							if (1  == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Free Run& Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Free Run& VHT 80 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}

							if (elapsed_ms<minCaptureTime)
								minCaptureTime=elapsed_ms;
							if(elapsed_ms>maxCaptureTime)
								maxCaptureTime=elapsed_ms;
							//---------------------------//
							//  Save sig file for debug  //
							//---------------------------//

							sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.iqvsa", "FastTrack_Capture", tempString.c_str(),iRun);
							CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveTruncateCapture()" );

							//----------------//
							//  Mask Analysis //
							//----------------//
							if ( 0 == capture_type)
								CheckReturnCode(LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );
							else
								if ( 6 == capture_type)
									CheckReturnCode(LP_AnalyzeVHT80Mask(), "LP_AnalyzeVHT80Mask()" );



							//-------------------------//
							//  Retrieve Test Results  //
							//-------------------------//
							double result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13,result14,result15,result16,result17,result18;

							CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );
							result1 = LP_GetScalarMeasurement("valid"); //result not right yet.
							printf("valid: %.0f\n", result1);

							result2 = LP_GetScalarMeasurement("P_av_each_burst_dBm");
							printf("P_av_each_burst_dBm: %.2f dBm\n", result2);

							result3 = LP_GetScalarMeasurement("P_av_each_burst");
							printf("P_av_each_burst: %.6f mw\n", result3);

							result4 = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
							printf("P_pk_each_burst_dBm: %.2f dBm\n", result4);

							result5 = LP_GetScalarMeasurement("P_pk_each_burst");
							printf("P_pk_each_burst: %.6f mw\n", result5);

							result6 = LP_GetScalarMeasurement("P_av_all_dBm");
							printf("P_av_all_dBm: %.2f dBm\n", result6);

							result7 = LP_GetScalarMeasurement("P_av_all");
							printf("P_av_all: %.6f mw\n", result7);

							result8 = LP_GetScalarMeasurement("P_peak_all_dBm");
							printf("P_peak_all_dBm: %.2f dBm\n", result8);

							result9= LP_GetScalarMeasurement("P_peak_all");
							printf("P_peak_all: %.6f mw\n", result9);

							result10 = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
							printf("P_av_no_gap_all_dBm: %.2f dBm\n", result10);

							result11 = LP_GetScalarMeasurement("P_av_no_gap_all");
							printf("P_av_no_gap_all: %.6f mw\n", result11);

							result12 = LP_GetScalarMeasurement("start_sec");
							printf("start_sec: %.6f sec\n", result12);

							result13 = LP_GetScalarMeasurement("stop_sec");
							printf("stop_sec: %.6f sec\n", result13);

							fprintf(fp_power,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13);

							CheckReturnCode( LP_AnalyzePowerRampOFDM(), "LP_AnalyzePowerRampOFDM()" );

							result1 = LP_GetScalarMeasurement("on_time");
							printf("on_time: %.2f\n", result1);
							result2 = LP_GetScalarMeasurement("off_time");
							printf("off_time: %.2f\n", result2);

							double result;
							result = ::LP_GetVectorMeasurement("on_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);



							CheckReturnCode( LP_AnalyzePowerRamp80211b(), "LP_AnalyzePowerRamp80211b()" );

							result1 = LP_GetScalarMeasurement("on_time");
							printf("on_time: %.2f\n", result1);
							result2 = LP_GetScalarMeasurement("off_time");
							printf("off_time: %.2f\n", result2);

							result = ::LP_GetVectorMeasurement("on_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);

							//	 CheckReturnCode( LP_AnalyzeSidelobe(), "LP_AnalyzeSidelobe()" ); // not available in IQapi 2.0.1.28

							//	 CheckReturnCode( LP_AnalyzeCCDF(), "LP_AnalyzeCCDF()" ); // not available in IQapi 2.0.1.28



							//11AC analysis

							printf("\nUsing 11AC analysis, iqapiAnalysis11ac\n");
							lp_time_t starttime4, stoptime4;

							for (int analysis_option_combination =1; analysis_option_combination<=3;analysis_option_combination++)
							{
								if (1==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac(), "LP_Analyze80211ac()" );
								}
								if (2==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac("nxn",0,0,1,1,1,0), "LP_Analyze80211ac()" );
								}
								if (3==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac("nxn",0,0,1,1,1,1), "LP_Analyze80211ac()" );
								}

								GetTime(stoptime4);

								int elapsed_ms4 = GetElapsedMSec(starttime4, stoptime4);
								printf("************ Elapsed time for LP_Analyze80211ac() option %d: %d ms ************\n", analysis_option_combination, elapsed_ms4);

								if (elapsed_ms4<minAnalysisTime)
									minAnalysisTime=elapsed_ms4;

								if(elapsed_ms4>maxAnalysisTime)
									maxAnalysisTime=elapsed_ms4;

								//11n and 11ac result

								result1 = LP_GetScalarMeasurement("psduCRC");
								if (result1 == 1)
								{
									printf("PSDU CRC: %s \n", "PASS");
								}
								else
									printf("PSDU CRC: %s \n", "FAIL");

								//Not working for 1 stream signal.
								// result = LP_GetScalarMeasurement("isolationDb");
								// printf("isolationDb=%0.2f \n", result);

								result2 = LP_GetScalarMeasurement("freqErrorHz");
								printf("Frequency Error: %.2f Hz\n", result2);

								result3 = LP_GetScalarMeasurement("symClockErrorPpm");
								printf("Symbol Clock Error: %.2f ppm\n", result3);

								result4 = LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll");
								printf("RMS Phase Noise: %.2f deg\n", result4);

								result5 = LP_GetScalarMeasurement("IQImbal_amplDb");
								printf("IQ Imbalance Amp: %.2f dB \n", result5);

								result6 = LP_GetScalarMeasurement("IQImbal_phaseDeg");
								printf("IQ Imbalance phase: %0.2f deg \n", result6);

								result7 = LP_GetScalarMeasurement("dcLeakageDbc");
								printf("LO leakage: %.2f dBc\n\n", result7);

								double bandwidthMhz = LP_GetScalarMeasurement("rateInfo_bandwidthMhz");
								printf("Signal Bandwidth: %0.2f MHz\n", bandwidthMhz);

								result8 = LP_GetScalarMeasurement("rateInfo_spatialStreams");
								printf("Number of Spatial Streams: %1.0f \n", result8);

								result9 = LP_GetScalarMeasurement("rateInfo_spaceTimeStreams");
								printf("Number of SpaceTime Streams: %1.0f \n", result9);

								char   result_char1[255];
								char   result_char2[255];

								LP_GetStringMeasurement("rateInfo_codingRate",result_char1,255);
								printf("codingRate: %s \n", result_char1);

								LP_GetStringMeasurement("rateInfo_modulation",result_char2,255);
								printf("modulation: %s \n", result_char2);

								result10 = LP_GetScalarMeasurement("rateInfo_dataRateMbps");
								printf("Datarate: %0.2f Mbps\n", result10);

								result11 = LP_GetScalarMeasurement("rxRmsPowerDb");
								printf("Power_NoGap: %.3f dBm\n", result11);

								if (result11 > maxPWR)
								{
									maxPWR=result11;
								}
								if(result11<minPWR)
								{
									minPWR=result11;
								}
								avgPwrSpectral+=result11;

								result12 = LP_GetScalarMeasurement("evmAvgAll");
								printf("EVM Avg All: %.3f dB\n\n", result12);

								if (result12 > maxEVM)
								{
									maxEVM=result12;
								}
								if(result12<minPWR)
								{
									minEVM=result12;
								}
								avgEVM+=result12;

								//			result = LP_GetScalarMeasurement("htSigFieldCRC");
								//			printf("htSigFieldCRC=%0.2f \n", result);

								result = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
								if (result <= 0)
								{
									printf("Channel Estimation returns an error\n");
								}
								else
								{
									//cacluate the spectrum flatness.
									printf("Channel Estimation returns a vector with %3.0f elements\n", result);
									int numSubcarrier = 256; //number of Subcarrier for 80MHz
									double  *carrierPwr;
									int centerStart, centerEnd, sideStart, sideEnd;
									//iqPwr  = (double *) malloc (result * sizeof (double));
									carrierPwr     = (double *) malloc (numSubcarrier * sizeof (double));
									double avgCenterPwr=0.0, avgSidePwr=0.0, avgPwrSpectral=0.0;
									double maxCarrierPwr = -999.9, minCenterCarrierPwr = 99.9, minSideCarrierPwr = 99.9;

									if (bandwidthMhz == 20.0)
									{
										numSubcarrier = 64;
										centerStart   = CARRIER_1;
										centerEnd     = CARRIER_16;
										sideStart     = CARRIER_17;
										sideEnd       = CARRIER_28;
									}
									else
									{
										if (bandwidthMhz == 40.0)
										{
											numSubcarrier = 128;
											centerStart   = CARRIER_2;
											centerEnd     = CARRIER_42;
											sideStart     = CARRIER_43;
											sideEnd       = CARRIER_58;
										}
										else
										{
											numSubcarrier = 256;
											centerStart   = CARRIER_2;
											centerEnd     = CARRIER_84;
											sideStart     = CARRIER_85;
											sideEnd       = CARRIER_122;
										}
									}
									if (result != numSubcarrier)
									{
										printf("Channel estimation results does not match number of subcarriers for %2.0f MHz signal.\n", bandwidthMhz);
									}
									else
									{
										for (int carrier=0;carrier<result;carrier++) //only one stream here.
										{
											carrierPwr[carrier] = bufferReal[carrier]*bufferReal[carrier]+ bufferImag[carrier]*bufferImag[carrier];
										}
										// average power in the center
										for ( int i=centerStart;i<=centerEnd;i++)
										{
											avgCenterPwr = avgCenterPwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
										}
										// average power on side lobe.
										for (int i=sideStart;i<=sideEnd;i++)
										{
											avgSidePwr = avgSidePwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
										}

										avgPwrSpectral = avgCenterPwr+ avgSidePwr;

										avgCenterPwr = avgCenterPwr/((centerEnd-centerStart+1)*2);
										if (0!=avgCenterPwr)
										{
											avgCenterPwr = 10.0 * log10 (avgCenterPwr);
										}
										else
										{
											avgCenterPwr= 0;
										}
										avgPwrSpectral = avgPwrSpectral/((sideEnd-centerStart+1)*2);
										if (0!=avgPwrSpectral)
										{
											avgPwrSpectral = 10.0 * log10 (avgPwrSpectral);
										}
										else
										{
											avgPwrSpectral = 0;
										}

										for (int carrier=0;carrier<numSubcarrier;carrier++)
										{
											if (0!=carrierPwr[carrier])
											{
												carrierPwr[carrier] = 10.0 * log10 (carrierPwr[carrier]);
												if (carrierPwr[carrier]>maxCarrierPwr)
												{
													maxCarrierPwr = carrierPwr[carrier];
												}
												if ( (carrier>=centerStart && carrier <=centerEnd) || (carrier>=numSubcarrier-centerEnd && carrier <= numSubcarrier-centerStart) )
												{
													if(carrierPwr[carrier]< minCenterCarrierPwr)
													{
														minCenterCarrierPwr = carrierPwr[carrier];
													}
												}
												if ((carrier>=sideStart && carrier <=sideEnd) || (carrier>=numSubcarrier-sideEnd && carrier <=numSubcarrier-sideStart) )
												{
													if(carrierPwr[carrier]< minSideCarrierPwr)
													{
														minSideCarrierPwr = carrierPwr[carrier];
													}
												}
											}
											else
											{
												carrierPwr[carrier] = 0;
											}
										}
										printf("The maximum deviation is %0.2f dB\n", maxCarrierPwr-avgCenterPwr);
										printf("The minimum deviation are %0.2f dB in the center, %0.2f dB in the far side\n", minCenterCarrierPwr-avgCenterPwr, minSideCarrierPwr-avgCenterPwr);
										printf("The LO Leakage is %0.2f dBc \n", carrierPwr[0] - avgPwrSpectral - 10 * log10((double)((sideEnd-centerStart+1)*2)) );
									}

								}

								printf("11AC VHT Sig field Info:\n");
								result13 = LP_GetScalarMeasurement("vhtSigA1Bandwidth");
								printf("VHTSigA1Bandwidth=%1.0f. Means: %3.0f MHz\n", result13, pow(2,result13)*20);

								result14 = LP_GetScalarMeasurement("vhtSigA1Stbc");
								printf("VHTSigA1Stbc=%1.0f. Means: ", result14);
								if(result14 == 0)
								{
									printf("STBC is not enabled for all streams.\n");
								}
								else
									printf("STBC is enabled for all streams.\n");
								//printf("VHTSigA1Stbc=%1.0f\n", result);

								result15 = LP_GetScalarMeasurement("vhtSigA2ShortGI");
								printf("VHTSigA2ShortGI=%1.0f\n", result);
								result16 = LP_GetScalarMeasurement("vhtSigA2AdvancedCoding");
								printf("VHTSigA2AdvancedCoding=%1.0f\n", result);
								result17 = LP_GetScalarMeasurement("vhtSigA2McsIndex");
								printf("VHTSigA2McsIndex=%1.0f\n", result);

								result18 = LP_GetScalarMeasurement("vhtSigBFieldCRC");
								printf("vhtSigBFieldCRC=%1.0f\n\n", result);

								if(1== analysis_option_combination)
									fprintf(fp_analysis1,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);

								if(2== analysis_option_combination)
									fprintf(fp_analysis2,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);

								if(3== analysis_option_combination)
									fprintf(fp_analysis3,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);


							}
							//----------------//
							//  Turn off VSG  //
							//----------------//

							printf("\n");

							//printf("Press 'X' key to exit or any key to repeat the test..................\n");

							//char ch = toupper(_getch());
							//if (ch=='X' || ch==ASCII_ESC)
							//    break;

						}  //analysis combination for-loop
					}  //VSA capture for-loop&trigger for-loop
				CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
			} //freq and vsg file for-loop
		printf("Statistics of %d run:\n",numRun);
		printf("Power_NoGap\n	Avg: %.2f dBm	Max: %.2f dBm	Min: %.2f dBm\n", avgPwrSpectral/numRun,maxPWR,minPWR);
		printf("EVM\n	Avg: %.2f dB	Max: %.2f dB	Min: %.2f dB\n\n", avgEVM/numRun,maxEVM,minEVM);
		printf("Capture time:  Max: %.0f ms	Min: %.0f ms\n", maxCaptureTime,minCaptureTime);
		printf("Analysis time:  Max: %.0f ms	Min: %.0f ms\n", maxAnalysisTime,minAnalysisTime);
	}


	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
	fclose(fp_power);
	fclose( fp_analysis1 );
	fclose(fp_analysis2);
	fclose(fp_analysis3);
	ReadLogFiles();
}



void QA_11n(void)
{
	char   buffer[MAX_BUFFER_SIZE];
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester
	double cableLoss = 1;           //dB of path loss
	double maxCaptureTime=0.0, minCaptureTime=1e6, maxAnalysisTime=0.0, minAnalysisTime=1e6;
	double freqHz;

	//----------------------------//
	//   Output File Operation    //
	//----------------------------//
	FILE *fp_power, *fp_analysis1, *fp_analysis2, *fp_analysis3, *fp_analysis4, *fp_analysis5;
	char *outFileName  = ".\\Log\\11n_QA_Power.csv";
	char *outFileName1 = ".\\Log\\11n_QA_Analysis1.csv";
	char *outFileName2 = ".\\Log\\11n_QA_Analysis2.csv";
	char *outFileName3 = ".\\Log\\11n_QA_Analysis3.csv";
	char *outFileName4 = ".\\Log\\HT20Mask.csv";
	char *outFileName5 = ".\\Log\\HT40Mask.csv";


	try
	{
		fopen_s(&fp_power,outFileName,"w+");
		fopen_s(&fp_analysis1,outFileName1,"w+");
		fopen_s(&fp_analysis2,outFileName2,"w+");
		fopen_s(&fp_analysis3,outFileName3,"w+");
		fopen_s(&fp_analysis4,outFileName4,"w+");
		fopen_s(&fp_analysis5,outFileName5,"w+");


		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		//CheckReturnCode( LP_Init(), "LP_Init()" );
		//const int ciTesterControlMode = 1;
		CheckReturnCode( LP_Init(ciTesterType, ciTesterControlMode), "LP_Init()" );

		//tyu;2012-7-6; Timer log
		if (ciTesterType == IQTYPE_XEL)
		{
			if (ciTesterControlMode == 0)  // IQapi_scpi
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_QA_11n_Run_%d.csv",Tag_runTime);
			}
			else   //SCPI
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_QA_11n_Run_%d.csv",Tag_runTime);
			}
		}
		else
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQ2010_QA_11n_Run_%d.csv",Tag_runTime);
		}
		//End




		//tyu; 2012-7-6; IP adddress is set in the setup file
		CheckReturnCode( LP_InitTester(g_IP_addr), "LP_InitTester()" );


		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)
			printf("\n%s\n\n", buffer);


		double maxPWR=-999.9, minPWR=999.9,avgPwrSpectral=0;
		double maxEVM=-999.9, minEVM=999.9,avgEVM=0;
		string modFileName = ""; // saveCapturedName="";
		string tempString = "";
		char saveCapturedName[MAX_BUFFER_SIZE];

		for( int freq_id = 1; freq_id <= 2; freq_id++)
			for (int signal_id = 1; signal_id <= 4; signal_id++)

			{

				if (1==freq_id)
					freqHz = 2422e6;
				else if (2==freq_id)
					freqHz = 5180e6;

				if (1==signal_id)
					tempString = "WiFi_HT20_MCS0";
				else if (2==signal_id)
					tempString = "WiFi_HT20_MCS7";
				else if (3==signal_id)
					tempString = "WiFi_HT40_MCS0";
				else if (4==signal_id)
					tempString = "WiFi_HT40_MCS7";

				//-------------//
				//  Setup VSG  //
				//-------------//

				CheckReturnCode( LP_SetVsg(freqHz, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

				//--------------------//
				// Load waveform file //
				//--------------------//
				//modFileName="../mod/";

				//modFileName+=tempString;

				//tyu; 2012-7-6; Seclet different format for IQ2010 and IQxel
				//Luke 2012-8-02: seperate mod and iqvsg folder
				//modFileName+=".iqvsg";
				if (ciTesterType == IQTYPE_XEL)
				{
					modFileName="../../iqvsg/";
					modFileName+=tempString;
					modFileName+=".iqvsg";
				}
				else		//IQ2010/view/flex
				{
					modFileName="../../mod/";
					modFileName+=tempString;
					modFileName+=".mod";
				}

				//end


				printf("\n Loading mode file %s\n", modFileName.c_str());
				CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );

				//---------------//
				//  Turn on VSG  //
				//---------------//
				CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				//------------------------------------------//
				//   Send packet, continuous transmission   //
				//tyu; 2012-07-07;
				//------------------------------------------//
				int frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
				int err =0;
				CheckReturnCode( LP_SetFrameCnt(1000), "LP_SetFrameCnt(1000000)" );
				while (true)
				{
					err = LP_TxDone();
					if ( err != 0)
						continue;
					else break;
				}
				CheckReturnCode( LP_SetFrameCnt(0), "LP_SetFrameCnt()" );

				//----------------//
				//   Setup VSA    //
				//----------------//
				int peakToAverageRatio = 10;
				//  double samplingRate = 160e6;      //tyu;2012-7-6; Timer log
				double samplingRate;       //tyu;2012-7-6; Timer log
				double captureTime;

				//tyu;2012-7-6; Timer log
				if (ciTesterType == IQTYPE_XEL)
				{
					samplingRate = 160e6;

				}
				else
				{
					samplingRate = 80e6;
				}
				//End

				CheckReturnCode( LP_SetVsa(freqHz, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?

				//----------------------//
				//  Perform VSA capture //
				//----------------------//
				for (int trigger_type = 1; trigger_type <=13; trigger_type+=12 )
					for (int capture_type = 0; capture_type <=1; capture_type+=1 )

					{

						lp_time_t starttime, stoptime;
						if (13 == trigger_type)
							captureTime = 206e-6;
						else
							captureTime = 2400e-6;

						fprintf(fp_power,"\n=====%.2f====%s ====",freqHz,tempString.c_str());
						fprintf(fp_power,"=======Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_power,"=====Valid, P_av_each_burst_dBm, P_av_each_burst, P_pk_each_burst_dBm, P_pk_each_burst, P_av_all_dBm, P_av_all, P_peak_all_dBm, P_peak_all, P_av_no_gap_all_dBm, P_av_no_gap_all, start_sec, stop_sec=======================\n");

						fprintf(fp_analysis1,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis1,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis1,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_codingRate, rateInfo_dataRateMbps, htSigBFieldCRC, channelEst, rxRmsPowerDb, evmAvgAll =======================\n");

						fprintf(fp_analysis2,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis2,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis2,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_codingRate, rateInfo_dataRateMbps, htSigBFieldCRC, channelEst, rxRmsPowerDb, evmAvgAll =======================\n");

						fprintf(fp_analysis3,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis3,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis3,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_codingRate, rateInfo_dataRateMbps, htSigBFieldCRC, channelEst, rxRmsPowerDb, evmAvgAll =======================\n");

						int iRun=0, numRun=5;
						while(iRun<numRun)
						{
							iRun++;
							printf("_________________________________________________\n");
							printf("Run %d\n", iRun);

							GetTime(starttime);

							CheckReturnCode( LP_VsaDataCapture(captureTime, trigger_type, samplingRate, capture_type), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?

							GetTime(stoptime);

							int elapsed_ms = GetElapsedMSec(starttime, stoptime);

							if (13 == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Signal trigger & Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Signal trigger & HT40 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}


							if (1  == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Free Run & Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Free Run & HT 40 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}

							if (elapsed_ms<minCaptureTime)
								minCaptureTime=elapsed_ms;
							if(elapsed_ms>maxCaptureTime)
								maxCaptureTime=elapsed_ms;
							//---------------------------//
							//  Save sig file for debug  //
							//---------------------------//

							//tyu; 2012-7-6; Seclet different format for IQ2010 and IQxel
							//sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.iqvsa", "FastTrack_Capture", tempString.c_str(),iRun);
							if (ciTesterType == IQTYPE_XEL)
							{
								sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.iqvsa", "FastTrack_Capture", tempString.c_str(),iRun);
							}
							else		//IQ2010/view/flex
							{
								sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.mod", "FastTrack_Capture", tempString.c_str(),iRun);
							}

							//end

							CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

							//----------------//
							//  Mask Analysis //
							//----------------//
							if ( 0 == capture_type)
							{

							}
							else if ( 1 == capture_type)

							{
								CheckReturnCode(LP_AnalyzeHT40Mask(), "LP_AnalyzeHT40Mask()" );

							}
							else
							{
							}

							//-------------------------//
							//  Retrieve Test Results  //
							//-------------------------//
							double result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13;

							if( ciTesterControlMode == 0 )//IQAPI
								CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );
							else if( ciTesterControlMode == 1 )//SCPI
								CheckReturnCode( LP_AnalyzePower(3.2e-6, 15.0), "LP_AnalyzePower()" );


							result1 = LP_GetScalarMeasurement("valid"); //result not right yet.
							printf("valid: %.0f\n", result1);

							result2 = LP_GetScalarMeasurement("P_av_each_burst_dBm");
							printf("P_av_each_burst_dBm: %.2f dBm\n", result2);

							result3 = LP_GetScalarMeasurement("P_av_each_burst");
							printf("P_av_each_burst: %.6f mw\n", result3);

							result4 = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
							printf("P_pk_each_burst_dBm: %.2f dBm\n", result4);

							result5 = LP_GetScalarMeasurement("P_pk_each_burst");
							printf("P_pk_each_burst: %.6f mw\n", result5);

							result6 = LP_GetScalarMeasurement("P_av_all_dBm");
							printf("P_av_all_dBm: %.2f dBm\n", result6);

							result7 = LP_GetScalarMeasurement("P_av_all");
							printf("P_av_all: %.6f mw\n", result7);

							result8 = LP_GetScalarMeasurement("P_peak_all_dBm");
							printf("P_peak_all_dBm: %.2f dBm\n", result8);

							result9= LP_GetScalarMeasurement("P_peak_all");
							printf("P_peak_all: %.6f mw\n", result9);

							result10 = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
							printf("P_av_no_gap_all_dBm: %.2f dBm\n", result10);

							result11 = LP_GetScalarMeasurement("P_av_no_gap_all");
							printf("P_av_no_gap_all: %.6f mw\n", result11);

							result12 = LP_GetScalarMeasurement("start_sec");
							printf("start_sec: %.6f sec\n", result12);

							result13 = LP_GetScalarMeasurement("stop_sec");
							printf("stop_sec: %.6f sec\n", result13);

							fprintf(fp_power,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.8f,%.8f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13);
							//11n analysis

							printf("\nUsing 11n analysis, iqapiAnalysis11n\n");
							lp_time_t starttime4, stoptime4;

							for (int analysis_option_combination =1; analysis_option_combination<=3;analysis_option_combination++)
							{
								if (1==analysis_option_combination)
								{
									GetTime(starttime4);
									//tyu; 2012-7-6; Seclet different format for IQ2010 and IQxel
									if (ciTesterType == IQTYPE_XEL)
									{
										CheckReturnCode( LP_Analyze80211n("ewc","nxn",1,1,0,0,0), "LP_Analyze80211n()" );
									}
									else
									{
										CheckReturnCode( LP_Analyze80211n("EWC","nxn",1,1,0,0,0), "LP_Analyze80211n()" );
									}
									//End
								}
								if (2==analysis_option_combination)
								{
									GetTime(starttime4);
									//tyu; 2012-7-6; Seclet different format for IQ2010 and IQxel
									if (ciTesterType == IQTYPE_XEL)
									{
										CheckReturnCode( LP_Analyze80211n("ewc","nxn",0,0,1,1,1), "LP_Analyze80211n()" );
									}
									else
									{
										CheckReturnCode( LP_Analyze80211n("EWC","nxn",0,0,1,1,1), "LP_Analyze80211n()" );
									}
									//End
								}
								if (3==analysis_option_combination)
								{
									GetTime(starttime4);
									//tyu; 2012-7-6; Seclet different format for IQ2010 and IQxel
									if (ciTesterType == IQTYPE_XEL)
									{
										CheckReturnCode( LP_Analyze80211n("ewc","nxn",0,0,1,0,1), "LP_Analyze80211n()" );
									}
									else
									{
										CheckReturnCode( LP_Analyze80211n("EWC","nxn",0,0,1,0,1), "LP_Analyze80211n()" );
									}
									//End
									//CheckReturnCode( LP_Analyze80211n("ewc","nxn",0,0,1,0,1), "LP_Analyze80211n()" );

								}

								GetTime(stoptime4);

								int elapsed_ms4 = GetElapsedMSec(starttime4, stoptime4);
								printf("************ Elapsed time for LP_Analyze80211n() option %d: %d ms ************\n", analysis_option_combination, elapsed_ms4);

								if (elapsed_ms4<minAnalysisTime)
									minAnalysisTime=elapsed_ms4;

								if(elapsed_ms4>maxAnalysisTime)
									maxAnalysisTime=elapsed_ms4;

								//11n and 11ac result

								result1 = LP_GetScalarMeasurement("psduCRC");
								if (result1 == 1)
								{
									printf("PSDU CRC: %s \n", "PASS");
								}
								else
									printf("PSDU CRC: %s \n", "FAIL");

								//Not working for 1 stream signal.
								// result = LP_GetScalarMeasurement("isolationDb");
								// printf("isolationDb=%0.2f \n", result);

								result2 = LP_GetScalarMeasurement("freqErrorHz");
								printf("Frequency Error: %.2f Hz\n", result2);

								result3 = LP_GetScalarMeasurement("symClockErrorPpm");
								printf("Symbol Clock Error: %.2f ppm\n", result3);

								result4 = LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll");
								printf("RMS Phase Noise: %.2f deg\n", result4);

								result5 = LP_GetScalarMeasurement("IQImbal_amplDb");
								printf("IQ Imbalance Amp: %.2f dB \n", result5);

								result6 = LP_GetScalarMeasurement("IQImbal_phaseDeg");
								printf("IQ Imbalance phase: %0.2f deg \n", result6);

								result7 = LP_GetScalarMeasurement("dcLeakageDbc");
								printf("LO leakage: %.2f dBc\n\n", result7);

								result8 = LP_GetScalarMeasurement("rateInfo_bandwidthMhz");
								printf("Signal Bandwidth: %0.2f MHz\n", result8);

								double bandwidthMhz;
								bandwidthMhz = result8;

								char   result_char1[255];

								LP_GetStringMeasurement("rateInfo_codingRate",result_char1,255);
								printf("codingRate: %s \n", result_char1);

								result9 = LP_GetScalarMeasurement("rateInfo_dataRateMbps");
								printf("Datarate: %0.2f Mbps\n", result9);


								result10 = LP_GetScalarMeasurement("htSigBFieldCRC");
								printf("vhtSigBFieldCRC=%1.0f\n", result10);

								/**************************************************************
								  added extral result
								 ***************************************************************/

								/*result11 = LP_GetScalarMeasurement("packetType");
								  printf("packetType: %.3f dBm\n", result11);

								  result11 = LP_GetScalarMeasurement("spectralFlatness");
								  printf("spectralFlatness: %.3f dBm\n", result11);

								  result11 = LP_GetScalarMeasurement("spectralFlatnessHighLimit");
								  printf("spectralFlatnessHighLimit: %.3f dBm\n", result11);

								  result11 = LP_GetScalarMeasurement("spectralFlatnessLowLimit");
								  printf("spectralFlatnessLowLimit: %.3f dBm\n", result11);

								  result11 = LP_GetScalarMeasurement("channelBandwidth");
								  printf("channelBandwidth: %.3f dBm\n", result11);*/

								/**************************************************************
								  added extral result
								 ***************************************************************/



								result11 = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
								if (result11 <= 0)
								{
									printf("Channel Estimation returns an error\n");
								}
								else
								{
									//cacluate the spectrum flatness.
									printf("Channel Estimation returns a vector with %3.0f elements\n", result11);
									int numSubcarrier = 256; //number of Subcarrier for 80MHz
									double  *carrierPwr;
									int centerStart, centerEnd, sideStart, sideEnd;
									//iqPwr  = (double *) malloc (result * sizeof (double));
									carrierPwr     = (double *) malloc (numSubcarrier * sizeof (double));
									double avgCenterPwr=0.0, avgSidePwr=0.0, avgPwrSpectral=0.0;
									double maxCarrierPwr = -999.9, minCenterCarrierPwr = 99.9, minSideCarrierPwr = 99.9;

									if (bandwidthMhz == 20.0)
									{
										numSubcarrier = 64;
										centerStart   = CARRIER_1;
										centerEnd     = CARRIER_16;
										sideStart     = CARRIER_17;
										sideEnd       = CARRIER_28;
									}
									else
									{
										if (bandwidthMhz == 40.0)
										{
											numSubcarrier = 128;
											centerStart   = CARRIER_2;
											centerEnd     = CARRIER_42;
											sideStart     = CARRIER_43;
											sideEnd       = CARRIER_58;
										}
										else
										{
											numSubcarrier = 256;
											centerStart   = CARRIER_2;
											centerEnd     = CARRIER_84;
											sideStart     = CARRIER_85;
											sideEnd       = CARRIER_122;
										}
									}
									if (result11 != numSubcarrier)
									{
										printf("Channel estimation results does not match number of subcarriers for %2.0f MHz signal.\n", bandwidthMhz);
									}
									else
									{
										for (int carrier=0;carrier<result11;carrier++) //only one stream here.
										{
											carrierPwr[carrier] = bufferReal[carrier]*bufferReal[carrier]+ bufferImag[carrier]*bufferImag[carrier];
										}
										// average power in the center
										for ( int i=centerStart;i<=centerEnd;i++)
										{
											avgCenterPwr = avgCenterPwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
										}
										// average power on side lobe.
										for (int i=sideStart;i<=sideEnd;i++)
										{
											avgSidePwr = avgSidePwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
										}

										avgPwrSpectral = avgCenterPwr+ avgSidePwr;

										avgCenterPwr = avgCenterPwr/((centerEnd-centerStart+1)*2);
										if (0!=avgCenterPwr)
										{
											avgCenterPwr = 10.0 * log10 (avgCenterPwr);
										}
										else
										{
											avgCenterPwr= 0;
										}
										avgPwrSpectral = avgPwrSpectral/((sideEnd-centerStart+1)*2);
										if (0!=avgPwrSpectral)
										{
											avgPwrSpectral = 10.0 * log10 (avgPwrSpectral);
										}
										else
										{
											avgPwrSpectral = 0;
										}

										for (int carrier=0;carrier<numSubcarrier;carrier++)
										{
											if (0!=carrierPwr[carrier])
											{
												carrierPwr[carrier] = 10.0 * log10 (carrierPwr[carrier]);
												if (carrierPwr[carrier]>maxCarrierPwr)
												{
													maxCarrierPwr = carrierPwr[carrier];
												}
												if ( (carrier>=centerStart && carrier <=centerEnd) || (carrier>=numSubcarrier-centerEnd && carrier <= numSubcarrier-centerStart) )
												{
													if(carrierPwr[carrier]< minCenterCarrierPwr)
													{
														minCenterCarrierPwr = carrierPwr[carrier];
													}
												}
												if ((carrier>=sideStart && carrier <=sideEnd) || (carrier>=numSubcarrier-sideEnd && carrier <=numSubcarrier-sideStart) )
												{
													if(carrierPwr[carrier]< minSideCarrierPwr)
													{
														minSideCarrierPwr = carrierPwr[carrier];
													}
												}
											}
											else
											{
												carrierPwr[carrier] = 0;
											}


										}

										printf("The maximum deviation is %0.2f dB\n", maxCarrierPwr-avgCenterPwr);
										printf("The minimum deviation are %0.2f dB in the center, %0.2f dB in the far side\n", minCenterCarrierPwr-avgCenterPwr, minSideCarrierPwr-avgCenterPwr);
										printf("The LO Leakage is %0.2f dBc \n", carrierPwr[0] - avgPwrSpectral - 10 * log10((double)((sideEnd-centerStart+1)*2)) );
									}

								}

								printf("11AC VHT Sig field Info:\n");

								result12 = LP_GetScalarMeasurement("rxRmsPowerDb");
								printf("Power_NoGap: %.3f dBm\n", result12);

								if (result12 > maxPWR)
								{
									maxPWR=result12;
								}
								if(result12<minPWR)
								{
									minPWR=result12;
								}
								avgPwrSpectral+=result12;

								result13 = LP_GetScalarMeasurement("evmAvgAll");
								printf("EVM Avg All: %.3f dB\n\n", result13);

								if (result13 > maxEVM)
								{
									maxEVM=result11;
								}
								if(result13<minEVM)
								{
									minEVM=result13;
								}
								avgEVM+=result13;


								if(1== analysis_option_combination)
									fprintf(fp_analysis1,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %s, %.2f, %.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result_char1,result9,result10,result11,result12,result13);

								if(2== analysis_option_combination)
									fprintf(fp_analysis2,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %s, %.2f, %.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result_char1,result9,result10,result11,result12,result13);

								if(3== analysis_option_combination)
									fprintf(fp_analysis3,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %s, %.2f, %.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result_char1,result9,result10,result11,result12,result13);


							}
							//----------------//
							//  Turn off VSG  //
							//----------------//

							printf("\n");

							//printf("Press 'X' key to exit or any key to repeat the test..................\n");

							//char ch = toupper(_getch());
							//if (ch=='X' || ch==ASCII_ESC)
							//    break;

						}  //analysis combination for-loop //end of while
					}  //VSA capture for-loop&trigger for-loop
				CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
			} //freq and vsg file for-loop
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");

	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
	fclose(fp_power);
	fclose( fp_analysis1 );
	fclose(fp_analysis2);
	fclose(fp_analysis3);
	fclose(fp_analysis4);
	fclose(fp_analysis5);
	ReadLogFiles();
}


void QA_11ag(void)
{
	char   buffer[MAX_BUFFER_SIZE];
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester
	double cableLoss = 1;           //dB of path loss
	double maxCaptureTime=0.0, minCaptureTime=1e6, maxAnalysisTime=0.0, minAnalysisTime=1e6;
	double freqHz;

	//----------------------------//
	//   Output File Operation    //
	//----------------------------//
	FILE *fp_power, *fp_analysis1, *fp_analysis2, *fp_analysis3;
	char *outFileName  = ".\\Log\\11ag_QA_Power.csv";
	char *outFileName1 = ".\\Log\\11ag_QA_Analysis1.csv";
	char *outFileName2 = ".\\Log\\11ag_QA_Analysis2.csv";
	char *outFileName3 = ".\\Log\\11ag_QA_Analysis3.csv";

	fopen_s(&fp_power,outFileName,"w+");
	fopen_s(&fp_analysis1,outFileName1,"w+");
	fopen_s(&fp_analysis2,outFileName2,"w+");
	fopen_s(&fp_analysis3,outFileName3,"w+");

	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		//c
		//const int ciTesterControlMode = 1;
		CheckReturnCode( LP_Init(ciTesterType, ciTesterControlMode), "LP_Init()" );


		//tyu;2012-7-6; Timer log
		if (ciTesterType == IQTYPE_XEL)
		{
			if (ciTesterControlMode == 0)  // IQapi_scpi
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_QA_11ag_Run_%d.csv",Tag_runTime);
			}
			else   //SCPI
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_QA_11ag_Run_%d.csv",Tag_runTime);
			}
		}
		else
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQ2010_QA_11ag_Run_%d.csv",Tag_runTime);
		}
		//End

		//tyu; 2012-7-6; IP adddress is set in the setup file
		CheckReturnCode( LP_InitTester(g_IP_addr), "LP_InitTester()" );

		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)
			printf("\n%s\n\n", buffer);


		double maxPWR=-999.9, minPWR=999.9,avgPwrSpectral=0;
		double maxEVM=-999.9, minEVM=999.9,avgEVM=0;
		string modFileName = ""; // saveCapturedName="";
		string tempString = "";
		char saveCapturedName[MAX_BUFFER_SIZE];

		for( int freq_id = 1; freq_id <= 2; freq_id++)
			for (int signal_id = 1; signal_id <= 2; signal_id++)

			{

				if (1==freq_id)
					freqHz = 2422e6;
				else if (2==freq_id)
					freqHz = 5180e6;

				if (1==signal_id)
					tempString = "WiFi_OFDM-6";
				else if (2==signal_id)
					tempString = "WiFi_OFDM-54";
				else
				{
				}


				//-------------//
				//  Setup VSG  //
				//-------------//

				CheckReturnCode( LP_SetVsg(freqHz, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

				//--------------------//
				// Load waveform file //
				//--------------------//
				//modFileName="../mod/";


				//modFileName+=tempString;
				//modFileName += ".mod";
				//	modFileName += ".iqvsg";

				//tyu; 2012-7-6; Seclet different format for IQ2010 and IQxel
				//Luke 2012-8-02: seperate mod and iqvsg folder
				//modFileName+=".iqvsg";
				if (ciTesterType == IQTYPE_XEL)
				{
					modFileName="../../iqvsg/";
					modFileName+=tempString;
					modFileName+=".iqvsg";
				}
				else		//IQ2010/view/flex
				{
					modFileName="../../mod/";
					modFileName+=tempString;
					modFileName+=".mod";
				}

				//end

				printf("\n Loading mode file %s\n", modFileName.c_str());
				CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );

				//---------------//
				//  Turn on VSG  //
				//---------------//
				CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				//------------------------------------------//
				//   Send packet, continuous transmission   //
				//tyu; 2012-07-07;
				//------------------------------------------//
				int frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
				int err =0;
				CheckReturnCode( LP_SetFrameCnt(1000), "LP_SetFrameCnt()" );
				while (true)
				{
					err = LP_TxDone();
					if ( err != 0)
						continue;
					else break;
				}
				CheckReturnCode( LP_SetFrameCnt(0), "LP_SetFrameCnt()" );

				//----------------//
				//   Setup VSA    //
				//----------------//
				int peakToAverageRatio = 10;
				double samplingRate = 160e6;
				double captureTime;

				//tyu;2012-7-6; Timer log
				if (ciTesterType == IQTYPE_XEL)
				{
					samplingRate = 160e6;

				}
				else
				{
					samplingRate = 80e6;
				}
				//End

				CheckReturnCode( LP_SetVsa(freqHz, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?

				//CheckReturnCode( LP_SetVsa(FREQ_HZ, -15, PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" );

				//----------------------//
				//  Perform VSA capture //
				//----------------------//
				for (int trigger_type = 1; trigger_type <=13; trigger_type+=12 )
					for (int capture_type = 0; capture_type <=1; capture_type+=1 )

					{

						lp_time_t starttime, stoptime;
						if (13 == trigger_type)
							captureTime = 206e-6;
						else
							captureTime = 2000e-6;

						fprintf(fp_power,"\n=====%.2f====%s ====",freqHz,tempString.c_str());
						fprintf(fp_power,"=======Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_power,"=====Valid, P_av_each_burst_dBm, P_av_each_burst, P_pk_each_burst_dBm, P_pk_each_burst, P_av_all_dBm, P_av_all, P_peak_all_dBm, P_peak_all, P_av_no_gap_all_dBm, P_av_no_gap_all, start_sec, stop_sec=======================\n");

						fprintf(fp_analysis1,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis1,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis1,"=====psduCrcFail, dataRate, numSymbols, numPsduBytes, codingRate, freqErr, clockErr, ampErr, ampErrDb, phaseErr, rmsPhaseNoise, pkPower, dcLeakageDbc, evmSymbols,hhEst, rmsPowerNoGap, evmAll =======================\n");


						fprintf(fp_analysis2,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis2,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis2,"=====psduCrcFail, dataRate, numSymbols, numPsduBytes, codingRate, freqErr, clockErr, ampErr, ampErrDb, phaseErr, rmsPhaseNoise, pkPower, dcLeakageDbc, evmSymbols,hhEst, rmsPowerNoGap, evmAll =======================\n");


						fprintf(fp_analysis3,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis3,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis3,"=====psduCrcFail, dataRate, numSymbols, numPsduBytes, codingRate, freqErr, clockErr, ampErr, ampErrDb, phaseErr, rmsPhaseNoise, pkPower, dcLeakageDbc, evmSymbols,hhEst, rmsPowerNoGap, evmAll =======================\n");


						int iRun=0, numRun=5;
						while(iRun<numRun)
						{
							iRun++;
							printf("_________________________________________________\n");
							printf("Run %d\n", iRun);

							GetTime(starttime);

							CheckReturnCode( LP_VsaDataCapture(captureTime, trigger_type, samplingRate, capture_type), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?

							GetTime(stoptime);

							int elapsed_ms = GetElapsedMSec(starttime, stoptime);

							if (13 == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Signal trigger & Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Signal trigger & HT40 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}


							if (1  == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Free Run & Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Free Run & HT 40 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}

							if (elapsed_ms<minCaptureTime)
								minCaptureTime=elapsed_ms;
							if(elapsed_ms>maxCaptureTime)
								maxCaptureTime=elapsed_ms;
							//---------------------------//
							//  Save sig file for debug  //
							//---------------------------//

							//tyu;2012-07-17;  support both IQ2010 and IQXel
							if (ciTesterType == IQTYPE_XEL)
							{
								sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-Trig%d-Capture%d-%d.iqvsa", "FastTrack_Capture", tempString.c_str(),trigger_type, capture_type,iRun);
							}
							else		//IQ2010/view/flex
							{
								sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-Trig%d-Capture%d-%d.sig", "FastTrack_Capture", tempString.c_str(),trigger_type, capture_type,iRun);
							}
							//end

							CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

							//----------------//
							//  Mask Analysis //
							//----------------//
							if ( 0 == capture_type)
								CheckReturnCode(LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );
							else
								if ( 6 == capture_type)
									CheckReturnCode(LP_AnalyzeHT40Mask(), "LP_AnalyzeHT40Mask()" );

							//-------------------------//
							//  Retrieve Test Results  //
							//-------------------------//
							double result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13;

							if( ciTesterControlMode == 0 )//IQAPI
								CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );
							else if( ciTesterControlMode == 1 )//SCPI
								CheckReturnCode( LP_AnalyzePower(3.2e-6, 15), "LP_AnalyzePower()" );


							result1 = LP_GetScalarMeasurement("valid"); //result not right yet.
							printf("valid: %.0f\n", result1);

							result2 = LP_GetScalarMeasurement("P_av_each_burst_dBm");
							printf("P_av_each_burst_dBm: %.2f dBm\n", result2);

							result3 = LP_GetScalarMeasurement("P_av_each_burst");
							printf("P_av_each_burst: %.6f mw\n", result3);

							result4 = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
							printf("P_pk_each_burst_dBm: %.2f dBm\n", result4);

							result5 = LP_GetScalarMeasurement("P_pk_each_burst");
							printf("P_pk_each_burst: %.6f mw\n", result5);

							result6 = LP_GetScalarMeasurement("P_av_all_dBm");
							printf("P_av_all_dBm: %.2f dBm\n", result6);

							result7 = LP_GetScalarMeasurement("P_av_all");
							printf("P_av_all: %.6f mw\n", result7);

							result8 = LP_GetScalarMeasurement("P_peak_all_dBm");
							printf("P_peak_all_dBm: %.2f dBm\n", result8);

							result9= LP_GetScalarMeasurement("P_peak_all");
							printf("P_peak_all: %.6f mw\n", result9);

							result10 = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
							printf("P_av_no_gap_all_dBm: %.2f dBm\n", result10);

							result11 = LP_GetScalarMeasurement("P_av_no_gap_all");
							printf("P_av_no_gap_all: %.6f mw\n", result11);

							result12 = LP_GetScalarMeasurement("start_sec");
							printf("start_sec: %.6f sec\n", result12);

							result13 = LP_GetScalarMeasurement("stop_sec");
							printf("stop_sec: %.6f sec\n", result13);

							fprintf(fp_power,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13);
							//11AC analysis

							printf("\nUsing 11ag analysis, iqapiAnalysis11n\n");
							lp_time_t starttime4, stoptime4;

							//	IQMEASURE_API int LP_Analyze80211ag(int ph_corr_mode=2, int ch_estimate=1, int sym_tim_corr=2, int freq_sync=2, int ampl_track=1);
							//											1,2,3			1,2(not supported),3			1,2	 1(not supported),2,3          1,2
							for (int analysis_option_combination =1; analysis_option_combination<=3;analysis_option_combination++)
							{
								if (1==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ag(), "LP_Analyze80211ag()" );
								}
								if (2==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ag(1,1,1,3,2), "LP_Analyze80211ag2()" );
								}
								if (3==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ag(1,3,1,3,2), "LP_Analyze80211ag3()" );
								}

								GetTime(stoptime4);

								int elapsed_ms4 = GetElapsedMSec(starttime4, stoptime4);
								printf("************ Elapsed time for LP_Analyze80211ag() option %d: %d ms ************\n", analysis_option_combination, elapsed_ms4);

								if (elapsed_ms4<minAnalysisTime)
									minAnalysisTime=elapsed_ms4;

								if(elapsed_ms4>maxAnalysisTime)
									maxAnalysisTime=elapsed_ms4;

								//11n and 11ac result

								result1 = LP_GetScalarMeasurement("psduCrcFail");
								if (result1 == 1)
								{
									printf("PSDU CRC: %s \n", "Fail");
								}
								else
									printf("PSDU CRC: %s \n", "Pass");

								//Not working for 1 stream signal.
								// result = LP_GetScalarMeasurement("isolationDb");
								// printf("isolationDb=%0.2f \n", result);

								result2 = LP_GetScalarMeasurement("dataRate");
								printf("Datarate=%.2f Mbps\n", result2);
								result3 = LP_GetScalarMeasurement("numSymbols");
								printf("numSymbols=%.2f \n", result3);
								result4 = LP_GetScalarMeasurement("numPsduBytes");
								printf("numPsduBytes=%.2f \n", result4);


								result5 = LP_GetScalarMeasurement("codingRate");
								printf("codingRate=%.2f \n", result5);
								result6 = LP_GetScalarMeasurement("freqErr");
								printf("freqErr=%.2f \n", result6);
								result7 = LP_GetScalarMeasurement("clockErr");
								printf("clockErr=%.2f \n", result7);
								result8 = LP_GetScalarMeasurement("ampErr");
								printf("ampErr=%.2f \n", result8);
								result9 = LP_GetScalarMeasurement("ampErrDb");
								printf("ampErrDb=%.2f \n", result9);
								result10 = LP_GetScalarMeasurement("phaseErr");
								printf("phaseErr=%.2f \n", result10);
								result11 = LP_GetScalarMeasurement("rmsPhaseNoise");
								printf("rmsPhaseNoise=%.2f \n", result11);

								result12 = LP_GetScalarMeasurement("pkPower");
								printf("pkPower=%.2f \n", result12);
								result13 = LP_GetScalarMeasurement("dcLeakageDbc");
								printf("dcLeakageDbc=%.2f \n", result13);


								// iqapiMeasurement	*psdu;		    //!< The PSDU data. 0/1 values. Includes the MAC Header and the FCS, if present.
								//iqapiMeasurement	*freq_vector;	//!< Preamble frequency error
								//iqapiMeasurement	*freq_vector_time; //!< Preamble frequency error versus time
								//iqapiMeasurement	*startPointers;	//!< Provides the approximate starting locations of each packet detected in the input data. If no packet detected, this is an empty vector.


								double result14,result15,result16,result17;
								result14 = LP_GetScalarMeasurement("evmSymbols");
								printf("evmSymbols=%.2f \n", result14);
								result16 = LP_GetScalarMeasurement("rmsPowerNoGap");
								result17 = LP_GetScalarMeasurement("evmAll");
								printf("Power_All=%.3f dB\n", result16);
								printf("EVM_All=%.3f dB\n", result17);

								if (result16 > maxPWR)
								{
									maxPWR=result16;
								}
								if(result16<minPWR)
								{
									minPWR=result16;
								}
								avgPwrSpectral+=result16;

								if (result17 > maxEVM)
								{
									maxEVM=result17;
								}
								if(result17<minEVM)
								{
									minEVM=result17;
								}
								avgEVM+=result17;

								result15 = ::LP_GetVectorMeasurement("hhEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
								if (result15 <= 0)
								{
									printf("Channel Estimation returns an error\n");
								}
								else
								{
									//cacluate the spectrum flatness.
									printf("Channel Estimation returns a vector with %3.0f elements\n", result15);

									double  *carrierPwr;
									int centerStart, centerEnd, sideStart, sideEnd;
									double numSubcarrier = 64;
									//iqPwr  = (double *) malloc (result * sizeof (double));
									carrierPwr     = (double *) malloc (numSubcarrier * sizeof (double));
									double avgCenterPwr=0.0, avgSidePwr=0.0, avgPwrSpectral=0.0;
									double maxCarrierPwr = -999.9, minCenterCarrierPwr = 99.9, minSideCarrierPwr = 99.9;

									centerStart   = CARRIER_1;
									centerEnd     = CARRIER_16;
									sideStart     = CARRIER_17;
									sideEnd       = CARRIER_28;

									if (result15 != numSubcarrier)
									{
										printf("Channel estimation results does not match number of subcarriers.\n");
									}
									else
									{
										for (int carrier=0;carrier<result15;carrier++) //only one stream here.
										{
											carrierPwr[carrier] = bufferReal[carrier]*bufferReal[carrier]+ bufferImag[carrier]*bufferImag[carrier];
										}
										// average power in the center
										for ( int i=centerStart;i<=centerEnd;i++)
										{
											avgCenterPwr = avgCenterPwr + carrierPwr[i] + carrierPwr[(int)(numSubcarrier-i)];
										}
										// average power on side lobe.
										for (int i=sideStart;i<=sideEnd;i++)
										{
											avgSidePwr = avgSidePwr + carrierPwr[i] + carrierPwr[(int)(numSubcarrier-i)];
										}

										avgPwrSpectral = avgCenterPwr+ avgSidePwr;

										avgCenterPwr = avgCenterPwr/((centerEnd-centerStart+1)*2);
										if (0!=avgCenterPwr)
										{
											avgCenterPwr = 10.0 * log10 (avgCenterPwr);
										}
										else
										{
											avgCenterPwr= 0;
										}
										avgPwrSpectral = avgPwrSpectral/((sideEnd-centerStart+1)*2);
										if (0!=avgPwrSpectral)
										{
											avgPwrSpectral = 10.0 * log10 (avgPwrSpectral);
										}
										else
										{
											avgPwrSpectral = 0;
										}

										for (int carrier=0;carrier<numSubcarrier;carrier++)
										{
											if (0!=carrierPwr[carrier])
											{
												carrierPwr[carrier] = 10.0 * log10 (carrierPwr[carrier]);
												if (carrierPwr[carrier]>maxCarrierPwr)
												{
													maxCarrierPwr = carrierPwr[carrier];
												}
												if ( (carrier>=centerStart && carrier <=centerEnd) || (carrier>=numSubcarrier-centerEnd && carrier <= numSubcarrier-centerStart) )
												{
													if(carrierPwr[carrier]< minCenterCarrierPwr)
													{
														minCenterCarrierPwr = carrierPwr[carrier];
													}
												}
												if ((carrier>=sideStart && carrier <=sideEnd) || (carrier>=numSubcarrier-sideEnd && carrier <=numSubcarrier-sideStart) )
												{
													if(carrierPwr[carrier]< minSideCarrierPwr)
													{
														minSideCarrierPwr = carrierPwr[carrier];
													}
												}
											}
											else
											{
												carrierPwr[carrier] = 0;
											}


										}

										printf("The maximum deviation is %0.2f dB\n", maxCarrierPwr-avgCenterPwr);
										printf("The minimum deviation are %0.2f dB in the center, %0.2f dB in the far side\n", minCenterCarrierPwr-avgCenterPwr, minSideCarrierPwr-avgCenterPwr);
										printf("The LO Leakage is %0.2f dBc \n", carrierPwr[0] - avgPwrSpectral - 10 * log10((double)((sideEnd-centerStart+1)*2)) );
									}

								}


								if(1== analysis_option_combination)
									fprintf(fp_analysis1,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13,result14,result15,result16,result17);
								//	fclose( fp_analysis1 );
								if(2== analysis_option_combination)
									fprintf(fp_analysis2,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %.2f, %.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13,result14,result15,result16,result17);

								if(3== analysis_option_combination)
									fprintf(fp_analysis3,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %.2f, %.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13,result14,result15,result16,result17);


							}
							//----------------//
							//  Turn off VSG  //
							//----------------//

							printf("\n");

							//printf("Press 'X' key to exit or any key to repeat the test..................\n");

							//char ch = toupper(_getch());
							//if (ch=='X' || ch==ASCII_ESC)
							//    break;

						}  //analysis combination for-loop
					}  //VSA capture for-loop&trigger for-loop
				CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
			} //freq and vsg file for-loop

	}


	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);

	}
	catch(...)
	{
		printf("ERROR!");
	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
	fclose(fp_power);
	fclose( fp_analysis1 );
	fclose(fp_analysis2);
	fclose(fp_analysis3);
	ReadLogFiles();
}




void QA_11b(void)
{
	char   buffer[MAX_BUFFER_SIZE];
	//	double	bufferImag[MAX_BUFFER_SIZE];
	//	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester
	double cableLoss = 1;           //dB of path loss
	double maxCaptureTime=0.0, minCaptureTime=1e6, maxAnalysisTime=0.0, minAnalysisTime=1e6;
	double freqHz;

	//----------------------------//
	//   Output File Operation    //
	//----------------------------//
	FILE *fp_power, *fp_analysis1, *fp_analysis2, *fp_analysis3;
	char *outFileName  = ".\\Log\\11b_QA_Power.csv";
	char *outFileName1 = ".\\Log\\11b_QA_Analysis1.csv";
	char *outFileName2 = ".\\Log\\11b_QA_Analysis2.csv";
	char *outFileName3 = ".\\Log\\11b_QA_Analysis3.csv";


	try
	{
		fopen_s(&fp_power,outFileName,"w+");
		fopen_s(&fp_analysis1,outFileName1,"w+");
		fopen_s(&fp_analysis2,outFileName2,"w+");
		fopen_s(&fp_analysis3,outFileName3,"w+");


		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		//CheckReturnCode( LP_Init(), "LP_Init()" );
		//const int ciTesterControlMode = 1;
		CheckReturnCode( LP_Init(ciTesterType, ciTesterControlMode), "LP_Init()" );

		//tyu;2012-7-6; Timer log
		if (ciTesterType == IQTYPE_XEL)
		{
			if (ciTesterControlMode == 0)  // IQapi_scpi
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_QA_11b_Run_%d.csv",Tag_runTime);
			}
			else   //SCPI
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_QA_11b_Run_%d.csv",Tag_runTime);
			}
		}
		else
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQ2010_QA_11b_Run_%d.csv",Tag_runTime);
		}
		//End
		//tyu; 2012-7-6; IP adddress is set in the setup file
		CheckReturnCode( LP_InitTester(g_IP_addr), "LP_InitTester()" );


		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)
			printf("\n%s\n\n", buffer);


		double maxPWR=-999.9, minPWR=999.9,avgPwrSpectral=0;
		double maxEVM=-999.9, minEVM=999.9,avgEVM=0;
		string modFileName = ""; // saveCapturedName="";
		string tempString = "";
		char saveCapturedName[MAX_BUFFER_SIZE];

		for( int freq_id = 1; freq_id <= 2; freq_id++)
			for (int signal_id = 1; signal_id <= 4; signal_id++)

			{

				if (1==freq_id)
					freqHz = 2412e6;
				else if (2==freq_id)
					freqHz = 2472e6;

				if (1==signal_id)
					tempString = "WiFi_DSSS-1";
				else if (2==signal_id)
					tempString = "WiFi_DSSS-2S";
				else if (3==signal_id)
					tempString = "WiFi_CCK-5_5L";
				else if (4==signal_id)
					tempString = "WiFi_CCK-11L";

				//-------------//
				//  Setup VSG  //
				//-------------//

				CheckReturnCode( LP_SetVsg(freqHz, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

				//--------------------//
				// Load waveform file //
				//--------------------//
				//modFileName="../mod/";
				//modFileName="../../iqvsg/";
				//modFileName+=tempString;
				//modFileName+=".iqvsg";
				//Luke 2012-8-02: seperate mod and iqvsg folder
				if (ciTesterType == IQTYPE_XEL)
				{
					modFileName="../../iqvsg/";
					modFileName+=tempString;
					modFileName+=".iqvsg";
				}
				else		//IQ2010/view/flex
				{
					modFileName="../../mod/";
					modFileName+=tempString;
					modFileName+=".mod";
				}

				printf("\n Loading mode file %s\n", modFileName.c_str());
				CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );

				//---------------//
				//  Turn on VSG  //
				//---------------//
				CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				//------------------------------------------//
				//   Send packet, continuous transmission   //
				//tyu; 2012-07-07;
				//------------------------------------------//
				int frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
				CheckReturnCode( LP_SetFrameCnt(1000), "LP_SetFrameCnt()" );
				while (true)
				{
					int err = LP_TxDone();
					if ( err != 0)
						continue;
					else break;
				}
				CheckReturnCode( LP_SetFrameCnt(0), "LP_SetFrameCnt()" );

				//----------------//
				//   Setup VSA    //
				//----------------//
				int peakToAverageRatio = 10;
				double samplingRate = 160e6;
				double captureTime;

				CheckReturnCode( LP_SetVsa(freqHz, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?

				//----------------------//
				//  Perform VSA capture //
				//----------------------//
				for (int trigger_type = 1; trigger_type <=13; trigger_type+=12 )
					for (int capture_type = 0; capture_type <=1; capture_type+=1 )

					{

						lp_time_t starttime, stoptime;
						if (13 == trigger_type)
							captureTime = 206e-6;
						else
							captureTime = 10000e-6;

						fprintf(fp_power,"\n=====%.2f====%s ====",freqHz,tempString.c_str());
						fprintf(fp_power,"=======Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_power,"=====Valid, P_av_each_burst_dBm, P_av_each_burst, P_pk_each_burst_dBm, P_pk_each_burst, P_av_all_dBm, P_av_all, P_peak_all_dBm, P_peak_all, P_av_no_gap_all_dBm, P_av_no_gap_all, start_sec, stop_sec=======================\n");

						fprintf(fp_analysis1,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis1,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis1,"=====lockedClock, plcpCrcFail, psduCrcFail, longPreamble, bitRateInMHz, evmPk, freqErr, clockErr, ampErr, ampErrDb, phaseErr, rmsPhaseNoise, rmsPowerNoGap, rmsPower, pkPower, rmsMaxAvgPower, modType, rmsPowerNoGap, evmAll=======================\n");


						fprintf(fp_analysis2,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis2,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis2,"=====lockedClock, plcpCrcFail, psduCrcFail, longPreamble, bitRateInMHz, evmPk, freqErr, clockErr, ampErr, ampErrDb, phaseErr, rmsPhaseNoise, rmsPowerNoGap, rmsPower, pkPower, rmsMaxAvgPower, modType, rmsPowerNoGap, evmAll=======================\n");


						fprintf(fp_analysis3,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis3,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis3,"=====lockedClock, plcpCrcFail, psduCrcFail, longPreamble, bitRateInMHz, evmPk, freqErr, clockErr, ampErr, ampErrDb, phaseErr, rmsPhaseNoise, rmsPowerNoGap, rmsPower, pkPower, rmsMaxAvgPower, modType, rmsPowerNoGap, evmAll=======================\n");


						int iRun=0, numRun=5;
						while(iRun<numRun)
						{
							iRun++;
							printf("_________________________________________________\n");
							printf("Run %d\n", iRun);

							GetTime(starttime);

							CheckReturnCode( LP_VsaDataCapture(captureTime, trigger_type, samplingRate, capture_type), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?

							GetTime(stoptime);

							int elapsed_ms = GetElapsedMSec(starttime, stoptime);

							if (13 == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Signal trigger & Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Signal trigger & HT40 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}


							if (1  == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Free Run & Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Free Run & HT 40 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}

							if (elapsed_ms<minCaptureTime)
								minCaptureTime=elapsed_ms;
							if(elapsed_ms>maxCaptureTime)
								maxCaptureTime=elapsed_ms;
							//---------------------------//
							//  Save sig file for debug  //
							//---------------------------//
							if (ciTesterType == IQTYPE_XEL)
								sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.iqvsa", "FastTrack_Capture", tempString.c_str(),iRun);
							else
								sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.sig", "FastTrack_Capture", tempString.c_str(),iRun);
							CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

							//----------------//
							//  Mask Analysis //
							//----------------//
							if ( 0 == capture_type)
								CheckReturnCode(LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );
							else
								if ( 6 == capture_type)
									CheckReturnCode(LP_AnalyzeHT40Mask(), "LP_AnalyzeHT40Mask()" );

							//-------------------------//
							//  Retrieve Test Results  //
							//-------------------------//
							double result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13,result14,result15,result16,result17,result18;

							if( ciTesterControlMode == 0 )//IQAPI
								CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );
							else if( ciTesterControlMode == 1 )//SCPI
								CheckReturnCode( LP_AnalyzePower(3.2e-6, 15.0), "LP_AnalyzePower()" );

							result1 = LP_GetScalarMeasurement("valid"); //result not right yet.
							printf("valid: %.0f\n", result1);

							result2 = LP_GetScalarMeasurement("P_av_each_burst_dBm");
							printf("P_av_each_burst_dBm: %.2f dBm\n", result2);

							result3 = LP_GetScalarMeasurement("P_av_each_burst");
							printf("P_av_each_burst: %.6f mw\n", result3);

							result4 = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
							printf("P_pk_each_burst_dBm: %.2f dBm\n", result4);

							result5 = LP_GetScalarMeasurement("P_pk_each_burst");
							printf("P_pk_each_burst: %.6f mw\n", result5);

							result6 = LP_GetScalarMeasurement("P_av_all_dBm");
							printf("P_av_all_dBm: %.2f dBm\n", result6);

							result7 = LP_GetScalarMeasurement("P_av_all");
							printf("P_av_all: %.6f mw\n", result7);

							result8 = LP_GetScalarMeasurement("P_peak_all_dBm");
							printf("P_peak_all_dBm: %.2f dBm\n", result8);

							result9= LP_GetScalarMeasurement("P_peak_all");
							printf("P_peak_all: %.6f mw\n", result9);

							result10 = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
							printf("P_av_no_gap_all_dBm: %.2f dBm\n", result10);

							result11 = LP_GetScalarMeasurement("P_av_no_gap_all");
							printf("P_av_no_gap_all: %.6f mw\n", result11);

							result12 = LP_GetScalarMeasurement("start_sec");
							printf("start_sec: %.6f sec\n", result12);

							result13 = LP_GetScalarMeasurement("stop_sec");
							printf("stop_sec: %.6f sec\n", result13);

							fprintf(fp_power,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.8f,%.8f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13);
							//11AC analysis

							printf("\nUsing 11n analysis, iqapiAnalysis11n\n");
							lp_time_t starttime4, stoptime4;

							//IQMEASURE_API int LP_Analyze80211b(int eq_taps      = 5,     1,5,7,9
							//                               int DCremove11b_flag = 0,	   0,1
							//                               int method_11b       = 1);    1,2
							for (int analysis_option_combination =1; analysis_option_combination<=3;analysis_option_combination++)
							{
								if (1==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211b(1,0,1), "LP_Analyze80211b()" );
								}
								if (2==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211b(5,1,1), "LP_Analyze80211b2()" );
								}
								if (3==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211b(9,1,2), "LP_Analyze80211b3()" );
								}

								GetTime(stoptime4);

								int elapsed_ms4 = GetElapsedMSec(starttime4, stoptime4);
								printf("************ Elapsed time for LP_Analyze80211b() option %d: %d ms ************\n", analysis_option_combination, elapsed_ms4);

								if (elapsed_ms4<minAnalysisTime)
									minAnalysisTime=elapsed_ms4;

								if(elapsed_ms4>maxAnalysisTime)
									maxAnalysisTime=elapsed_ms4;

								//11b result

								result1 = LP_GetScalarMeasurement("lockedClock");
								printf("lockedClock=%.3f \n", result1);

								result2 = LP_GetScalarMeasurement("plcpCrcFail");
								printf("plcpCrcFail=%.3f \n", result2);

								result3 = LP_GetScalarMeasurement("psduCrcFail");
								printf("psduCrcFail=%.3f \n", result3);

								result4 = LP_GetScalarMeasurement("longPreamble");
								printf("longPreamble=%.3f \n", result4);

								result5 = LP_GetScalarMeasurement("bitRateInMHz");
								printf("bitRateInMHz=%.3f \n", result5);

								result6 = LP_GetScalarMeasurement("evmPk");
								printf("evmPk=%.3f \n", result6);


								result7 = LP_GetScalarMeasurement("freqErr");
								printf("freqErr=%.3f \n", result7);

								result8 = LP_GetScalarMeasurement("clockErr");
								printf("clockErr=%.3f \n", result8);

								result9 = LP_GetScalarMeasurement("ampErr");
								printf("ampErr=%.3f \n", result9);

								result10 = LP_GetScalarMeasurement("ampErrDb");
								printf("ampErrDb=%.3f \n", result10);

								result11 = LP_GetScalarMeasurement("phaseErr");
								printf("phaseErr=%.3f \n", result11);

								result12 = LP_GetScalarMeasurement("rmsPhaseNoise");
								printf("rmsPhaseNoise=%.3f \n", result12);

								result13 = LP_GetScalarMeasurement("rmsPowerNoGap");
								printf("rmsPowerNoGap=%.3f \n", result13);

								result14 = LP_GetScalarMeasurement("rmsPower");
								printf("rmsPower=%.3f \n", result14);

								result15 = LP_GetScalarMeasurement("pkPower");
								printf("pkPower=%.3f \n", result15);

								result16 = LP_GetScalarMeasurement("rmsMaxAvgPower");
								printf("rmsMaxAvgPower=%.3f \n", result16);

								result17 = LP_GetScalarMeasurement("modType");
								printf("modType=%.3f \n", result17);


								result18 = LP_GetScalarMeasurement("rmsPowerNoGap");
								double result19;
								result19 = LP_GetScalarMeasurement("evmAll");
								printf("Power_All=%.3f dB\n", result18);
								printf("EVM_All=%.3f dB\n", result19);

								if (result18 > maxPWR)
								{
									maxPWR=result18;
								}
								if(result16<minPWR)
								{
									minPWR=result18;
								}
								avgPwrSpectral+=result18;

								if (result19 > maxEVM)
								{
									maxEVM=result19;
								}
								if(result19<minEVM)
								{
									minEVM=result19;
								}
								avgEVM+=result19;


								if(1== analysis_option_combination)
									fprintf(fp_analysis1,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13,result14,result15,result16,result17,result18,result19);
								//	fclose( fp_analysis1 );
								if(2== analysis_option_combination)
									fprintf(fp_analysis2,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %.2f, %.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result4,result9,result10,result11,result12,result13,result14,result15,result16,result17,result18,result19);

								if(3== analysis_option_combination)
									fprintf(fp_analysis3,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %.2f, %.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result4,result9,result10,result11,result12,result13,result14,result15,result16,result17,result18,result19);


							}
							//----------------//
							//  Turn off VSG  //
							//----------------//

							printf("\n");

							//printf("Press 'X' key to exit or any key to repeat the test..................\n");

							//char ch = toupper(_getch());
							//if (ch=='X' || ch==ASCII_ESC)
							//    break;

						}  //analysis combination for-loop
					}  //VSA capture for-loop&trigger for-loop
				CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
			} //freq and vsg file for-loop
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
	fclose(fp_power);
	fclose( fp_analysis1 );
	fclose(fp_analysis2);
	fclose(fp_analysis3);

}

void QA_FrameCount(void)
{
	char   buffer[MAX_BUFFER_SIZE];
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester
	double cableLoss = 1;           //dB of path loss
	double maxCaptureTime=0.0, minCaptureTime=1e6, maxAnalysisTime=0.0, minAnalysisTime=1e6;
	double freqHz;

	//----------------------------//
	//   Output File Operation    //
	//----------------------------//
	FILE *fp_power, *fp_analysis1, *fp_analysis2, *fp_analysis3;
	char *outFileName  = ".\\Log\\11AC_QA_Power.csv";
	char *outFileName1 = ".\\Log\\11AC_QA_Analysis1.csv";
	char *outFileName2 = ".\\Log\\11AC_QA_Analysis2.csv";
	char *outFileName3 = ".\\Log\\11AC_QA_Analysis3.csv";

	try
	{


		fopen_s(&fp_power,outFileName,"w+");
		fopen_s(&fp_analysis1,outFileName1,"w+");
		fopen_s(&fp_analysis2,outFileName2,"w+");
		fopen_s(&fp_analysis3,outFileName3,"w+");

		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(ciTesterType,ciTesterControlMode), "LP_Init()" );

		//tyu;2012-7-6; Timer log
		if (ciTesterType == IQTYPE_XEL)
		{
			if (ciTesterControlMode == 0)  // IQapi_scpi
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_QA_FrameCount_Run_%d.csv",Tag_runTime);
			}
			else   //SCPI
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_QA_FrameCount_Run_%d.csv",Tag_runTime);
			}
		}
		else
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQ2010_Run_%d.csv",Tag_runTime);
		}
		//End

		//tyu; 2012-7-6; IP adddress is set in the setup file
		CheckReturnCode( LP_InitTester(g_IP_addr), "LP_InitTester()" );


		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)
			printf("\n%s\n\n", buffer);
		int iRun=0, numRun=5;

		double maxPWR=-999.9, minPWR=999.9,avgPwrSpectral=0;
		double maxEVM=-999.9, minEVM=999.9,avgEVM=0;
		string modFileName = ""; // saveCapturedName="";
		string tempString = "";
		char saveCapturedName[MAX_BUFFER_SIZE];

		for( int freq_id = 2; freq_id <= 2; freq_id++)
			for (int signal_id = 1; signal_id <= 4; signal_id++)

			{

				if (1==freq_id)
					freqHz = 2422e6;
				else if (2==freq_id)
					freqHz = 5180e6;

				if (1==signal_id)
					tempString = "11AC_MCS8_1strm_80MHz_2550bytes_pgap20us_LongGI_AMPDUoff_Fs160";
				else if (2==signal_id)
					tempString = "11AC_MCS9_1strm_80MHz_2850bytes_pgap20us_LongGI_AMPDUoff_Fs160";
				else if (3==signal_id)
					tempString = "11AC_MCS8_1strm_20MHz_1500bytes_pgap20us_LongGI_AMPDUoff_Fs160";
				else if (4==signal_id)
					tempString = "11AC_MCS9_1strm_40MHz_3000bytes_pgap20us_LongGI_AMPDUoff_Fs160";

				//-------------//
				//  Setup VSG  //
				//-------------//

				CheckReturnCode( LP_SetVsg(freqHz, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

				//--------------------//
				// Load waveform file //
				//--------------------//
				//modFileName="../mod/";
				modFileName="../../iqvsg/";//Luke
				modFileName+=tempString;
				modFileName+=".iqvsg";
				printf("\n Loading mode file %s\n", modFileName.c_str());
				CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );
				CheckReturnCode( LP_SetFrameCnt(1), "LP_SetFrameCnt()" );

				//---------------//
				//  Turn on VSG  //
				//---------------//
				while(1)
				{
					CheckReturnCode( LP_SetFrameCnt(7), "LP_SetFrameCnt()" );
				}
				CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
				//----------------//
				//   Setup VSA    //
				//----------------//
				int peakToAverageRatio = 10;
				double samplingRate = 160e6;
				double captureTime;

				CheckReturnCode( LP_SetVsa(freqHz, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?

				//CheckReturnCode( LP_SetVsa(FREQ_HZ, -15, PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" );

				//----------------------//
				//  Perform VSA capture //
				//----------------------//
				for (int trigger_type = 13; trigger_type <=13; trigger_type+=12 )
					for (int capture_type = 0; capture_type <=6; capture_type+=6 )

					{

						lp_time_t starttime, stoptime;
						if (13 == trigger_type)
							captureTime = 206e-6;
						else
							captureTime = 500e-6;

						fprintf(fp_power,"\n=====%.2f====%s ====",freqHz,tempString.c_str());
						fprintf(fp_power,"=======Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_power,"=====Valid, P_av_each_burst_dBm, P_av_each_burst, P_pk_each_burst_dBm, P_pk_each_burst, P_av_all_dBm, P_av_all, P_peak_all_dBm, P_peak_all, P_av_no_gap_all_dBm, P_av_no_gap_all, start_sec, stop_sec=======================\n");

						fprintf(fp_analysis1,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis1,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis1,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						fprintf(fp_analysis2,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis2,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis2,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						fprintf(fp_analysis3,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis3,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis3,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						int iRun=0, numRun=5;
						while(iRun<numRun)
						{
							iRun++;
							printf("_________________________________________________\n");
							printf("Run %d\n", iRun);

							GetTime(starttime);

							CheckReturnCode( LP_VsaDataCapture(captureTime, trigger_type, samplingRate, capture_type), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?

							GetTime(stoptime);

							int elapsed_ms = GetElapsedMSec(starttime, stoptime);

							if (13 == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Signal trigger& Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Signal trigger& VHT 80 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}


							if (1  == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Free Run& Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Free Run& VHT 80 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}

							if (elapsed_ms<minCaptureTime)
								minCaptureTime=elapsed_ms;
							if(elapsed_ms>maxCaptureTime)
								maxCaptureTime=elapsed_ms;
							//---------------------------//
							//  Save sig file for debug  //
							//---------------------------//

							//sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.iqvsa", "FastTrack_Capture", tempString.c_str(),iRun);
							//tyu;2012-07-17;  support both IQ2010 and IQXel
							if (ciTesterType == IQTYPE_XEL)
							{
								sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-Trig%d-Capture%d-%d.iqvsa", "FastTrack_Capture", tempString.c_str(),trigger_type, capture_type,iRun);
							}
							else		//IQ2010/view/flex
							{
								sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-Trig%d-Capture%d-%d.sig", "FastTrack_Capture", tempString.c_str(),trigger_type, capture_type,iRun);
							}
							//end

							CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

							//----------------//
							//  Mask Analysis //
							//----------------//
							if ( 0 == capture_type)
								CheckReturnCode(LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );
							else
								if ( 6 == capture_type)
									CheckReturnCode(LP_AnalyzeVHT80Mask(), "LP_AnalyzeVHT80Mask()" );



							//-------------------------//
							//  Retrieve Test Results  //
							//-------------------------//
							double result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13;//,result14,result15,result16,result17,result18,result19;

							CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );
							result1 = LP_GetScalarMeasurement("valid"); //result not right yet.
							printf("valid: %.0f\n", result1);

							result2 = LP_GetScalarMeasurement("P_av_each_burst_dBm");
							printf("P_av_each_burst_dBm: %.2f dBm\n", result2);

							result3 = LP_GetScalarMeasurement("P_av_each_burst");
							printf("P_av_each_burst: %.6f mw\n", result3);

							result4 = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
							printf("P_pk_each_burst_dBm: %.2f dBm\n", result4);

							result5 = LP_GetScalarMeasurement("P_pk_each_burst");
							printf("P_pk_each_burst: %.6f mw\n", result5);

							result6 = LP_GetScalarMeasurement("P_av_all_dBm");
							printf("P_av_all_dBm: %.2f dBm\n", result6);

							result7 = LP_GetScalarMeasurement("P_av_all");
							printf("P_av_all: %.6f mw\n", result7);

							result8 = LP_GetScalarMeasurement("P_peak_all_dBm");
							printf("P_peak_all_dBm: %.2f dBm\n", result8);

							result9= LP_GetScalarMeasurement("P_peak_all");
							printf("P_peak_all: %.6f mw\n", result9);

							result10 = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
							printf("P_av_no_gap_all_dBm: %.2f dBm\n", result10);

							result11 = LP_GetScalarMeasurement("P_av_no_gap_all");
							printf("P_av_no_gap_all: %.6f mw\n", result11);

							result12 = LP_GetScalarMeasurement("start_sec");
							printf("start_sec: %.6f sec\n", result12);

							result13 = LP_GetScalarMeasurement("stop_sec");
							printf("stop_sec: %.6f sec\n", result13);

							fprintf(fp_power,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.8f,%.8f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13);

							CheckReturnCode( LP_AnalyzePowerRampOFDM(), "LP_AnalyzePowerRampOFDM()" );

							result1 = LP_GetScalarMeasurement("on_time");
							printf("on_time: %.2f\n", result1);
							result2 = LP_GetScalarMeasurement("off_time");
							printf("off_time: %.2f\n", result2);


							double result,result14,result15,result16,result17,result18;
							result = ::LP_GetVectorMeasurement("on_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);



							CheckReturnCode( LP_AnalyzePowerRamp80211b(), "LP_AnalyzePowerRamp80211b()" );

							result1 = LP_GetScalarMeasurement("on_time");
							printf("on_time: %.2f\n", result1);
							result2 = LP_GetScalarMeasurement("off_time");
							printf("off_time: %.2f\n", result2);

							result = ::LP_GetVectorMeasurement("on_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);

							//	 CheckReturnCode( LP_AnalyzeSidelobe(), "LP_AnalyzeSidelobe()" ); // not available in IQapi 2.0.1.28

							//	 CheckReturnCode( LP_AnalyzeCCDF(), "LP_AnalyzeCCDF()" ); // not available in IQapi 2.0.1.28



							//11AC analysis

							printf("\nUsing 11AC analysis, iqapiAnalysis11ac\n");
							lp_time_t starttime4, stoptime4;

							for (int analysis_option_combination =1; analysis_option_combination<=3;analysis_option_combination++)
							{
								if (1==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac(), "LP_Analyze80211ac()" );
								}
								if (2==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac("nxn",0,0,1,1,1,0), "LP_Analyze80211ac()" );
								}
								if (3==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac("nxn",0,0,1,1,1,1), "LP_Analyze80211ac()" );
								}

								GetTime(stoptime4);

								int elapsed_ms4 = GetElapsedMSec(starttime4, stoptime4);
								printf("************ Elapsed time for LP_Analyze80211ac() option %d: %d ms ************\n", analysis_option_combination, elapsed_ms4);

								if (elapsed_ms4<minAnalysisTime)
									minAnalysisTime=elapsed_ms4;

								if(elapsed_ms4>maxAnalysisTime)
									maxAnalysisTime=elapsed_ms4;

								//11n and 11ac result

								result1 = LP_GetScalarMeasurement("psduCRC");
								if (result1 == 1)
								{
									printf("PSDU CRC: %s \n", "PASS");
								}
								else
									printf("PSDU CRC: %s \n", "FAIL");

								//Not working for 1 stream signal.
								// result = LP_GetScalarMeasurement("isolationDb");
								// printf("isolationDb=%0.2f \n", result);

								result2 = LP_GetScalarMeasurement("freqErrorHz");
								printf("Frequency Error: %.2f Hz\n", result2);

								result3 = LP_GetScalarMeasurement("symClockErrorPpm");
								printf("Symbol Clock Error: %.2f ppm\n", result3);

								result4 = LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll");
								printf("RMS Phase Noise: %.2f deg\n", result4);

								result5 = LP_GetScalarMeasurement("IQImbal_amplDb");
								printf("IQ Imbalance Amp: %.2f dB \n", result5);

								result6 = LP_GetScalarMeasurement("IQImbal_phaseDeg");
								printf("IQ Imbalance phase: %0.2f deg \n", result6);

								result7 = LP_GetScalarMeasurement("dcLeakageDbc");
								printf("LO leakage: %.2f dBc\n\n", result7);

								double bandwidthMhz = LP_GetScalarMeasurement("rateInfo_bandwidthMhz");
								printf("Signal Bandwidth: %0.2f MHz\n", bandwidthMhz);

								result8 = LP_GetScalarMeasurement("rateInfo_spatialStreams");
								printf("Number of Spatial Streams: %1.0f \n", result8);

								result9 = LP_GetScalarMeasurement("rateInfo_spaceTimeStreams");
								printf("Number of SpaceTime Streams: %1.0f \n", result9);

								char   result_char1[255];
								char   result_char2[255];

								LP_GetStringMeasurement("rateInfo_codingRate",result_char1,255);
								printf("codingRate: %s \n", result_char1);

								LP_GetStringMeasurement("rateInfo_modulation",result_char2,255);
								printf("modulation: %s \n", result_char2);

								result10 = LP_GetScalarMeasurement("rateInfo_dataRateMbps");
								printf("Datarate: %0.2f Mbps\n", result10);

								result11 = LP_GetScalarMeasurement("rxRmsPowerDb");
								printf("Power_NoGap: %.3f dBm\n", result11);

								if (result11 > maxPWR)
								{
									maxPWR=result11;
								}
								if(result11<minPWR)
								{
									minPWR=result11;
								}
								avgPwrSpectral+=result11;

								result12 = LP_GetScalarMeasurement("evmAvgAll");
								printf("EVM Avg All: %.3f dB\n\n", result12);

								if (result12 > maxEVM)
								{
									maxEVM=result12;
								}
								if(result12<minPWR)
								{
									minEVM=result12;
								}
								avgEVM+=result12;

								//			result = LP_GetScalarMeasurement("htSigFieldCRC");
								//			printf("htSigFieldCRC=%0.2f \n", result);

								result = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
								if (result <= 0)
								{
									printf("Channel Estimation returns an error\n");
								}
								else
								{
									//cacluate the spectrum flatness.
									printf("Channel Estimation returns a vector with %3.0f elements\n", result);
									int numSubcarrier = 256; //number of Subcarrier for 80MHz
									double  *carrierPwr;
									int centerStart, centerEnd, sideStart, sideEnd;
									//iqPwr  = (double *) malloc (result * sizeof (double));
									carrierPwr     = (double *) malloc (numSubcarrier * sizeof (double));
									double avgCenterPwr=0.0, avgSidePwr=0.0, avgPwrSpectral=0.0;
									double maxCarrierPwr = -999.9, minCenterCarrierPwr = 99.9, minSideCarrierPwr = 99.9;

									if (bandwidthMhz == 20.0)
									{
										numSubcarrier = 64;
										centerStart   = CARRIER_1;
										centerEnd     = CARRIER_16;
										sideStart     = CARRIER_17;
										sideEnd       = CARRIER_28;
									}
									else
									{
										if (bandwidthMhz == 40.0)
										{
											numSubcarrier = 128;
											centerStart   = CARRIER_2;
											centerEnd     = CARRIER_42;
											sideStart     = CARRIER_43;
											sideEnd       = CARRIER_58;
										}
										else
										{
											numSubcarrier = 256;
											centerStart   = CARRIER_2;
											centerEnd     = CARRIER_84;
											sideStart     = CARRIER_85;
											sideEnd       = CARRIER_122;
										}
									}
									if (result != numSubcarrier)
									{
										printf("Channel estimation results does not match number of subcarriers for %2.0f MHz signal.\n", bandwidthMhz);
									}
									else
									{
										for (int carrier=0;carrier<result;carrier++) //only one stream here.
										{
											carrierPwr[carrier] = bufferReal[carrier]*bufferReal[carrier]+ bufferImag[carrier]*bufferImag[carrier];
										}
										// average power in the center
										for ( int i=centerStart;i<=centerEnd;i++)
										{
											avgCenterPwr = avgCenterPwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
										}
										// average power on side lobe.
										for (int i=sideStart;i<=sideEnd;i++)
										{
											avgSidePwr = avgSidePwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
										}

										avgPwrSpectral = avgCenterPwr+ avgSidePwr;

										avgCenterPwr = avgCenterPwr/((centerEnd-centerStart+1)*2);
										if (0!=avgCenterPwr)
										{
											avgCenterPwr = 10.0 * log10 (avgCenterPwr);
										}
										else
										{
											avgCenterPwr= 0;
										}
										avgPwrSpectral = avgPwrSpectral/((sideEnd-centerStart+1)*2);
										if (0!=avgPwrSpectral)
										{
											avgPwrSpectral = 10.0 * log10 (avgPwrSpectral);
										}
										else
										{
											avgPwrSpectral = 0;
										}

										for (int carrier=0;carrier<numSubcarrier;carrier++)
										{
											if (0!=carrierPwr[carrier])
											{
												carrierPwr[carrier] = 10.0 * log10 (carrierPwr[carrier]);
												if (carrierPwr[carrier]>maxCarrierPwr)
												{
													maxCarrierPwr = carrierPwr[carrier];
												}
												if ( (carrier>=centerStart && carrier <=centerEnd) || (carrier>=numSubcarrier-centerEnd && carrier <= numSubcarrier-centerStart) )
												{
													if(carrierPwr[carrier]< minCenterCarrierPwr)
													{
														minCenterCarrierPwr = carrierPwr[carrier];
													}
												}
												if ((carrier>=sideStart && carrier <=sideEnd) || (carrier>=numSubcarrier-sideEnd && carrier <=numSubcarrier-sideStart) )
												{
													if(carrierPwr[carrier]< minSideCarrierPwr)
													{
														minSideCarrierPwr = carrierPwr[carrier];
													}
												}
											}
											else
											{
												carrierPwr[carrier] = 0;
											}
										}
										printf("The maximum deviation is %0.2f dB\n", maxCarrierPwr-avgCenterPwr);
										printf("The minimum deviation are %0.2f dB in the center, %0.2f dB in the far side\n", minCenterCarrierPwr-avgCenterPwr, minSideCarrierPwr-avgCenterPwr);
										printf("The LO Leakage is %0.2f dBc \n", carrierPwr[0] - avgPwrSpectral - 10 * log10((double)((sideEnd-centerStart+1)*2)) );
									}

								}

								printf("11AC VHT Sig field Info:\n");
								result13 = LP_GetScalarMeasurement("vhtSigA1Bandwidth");
								printf("VHTSigA1Bandwidth=%1.0f. Means: %3.0f MHz\n", result13, pow(2,result13)*20);

								result14 = LP_GetScalarMeasurement("vhtSigA1Stbc");
								printf("VHTSigA1Stbc=%1.0f. Means: ", result14);
								if(result14 == 0)
								{
									printf("STBC is not enabled for all streams.\n");
								}
								else
									printf("STBC is enabled for all streams.\n");
								//printf("VHTSigA1Stbc=%1.0f\n", result);

								result15 = LP_GetScalarMeasurement("vhtSigA2ShortGI");
								printf("VHTSigA2ShortGI=%1.0f\n", result);
								result16 = LP_GetScalarMeasurement("vhtSigA2AdvancedCoding");
								printf("VHTSigA2AdvancedCoding=%1.0f\n", result);
								result17 = LP_GetScalarMeasurement("vhtSigA2McsIndex");
								printf("VHTSigA2McsIndex=%1.0f\n", result);

								result18 = LP_GetScalarMeasurement("vhtSigBFieldCRC");
								printf("vhtSigBFieldCRC=%1.0f\n\n", result);

								if(1== analysis_option_combination)
									fprintf(fp_analysis1,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);

								if(2== analysis_option_combination)
									fprintf(fp_analysis2,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);

								if(3== analysis_option_combination)
									fprintf(fp_analysis3,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);


							}
							//----------------//
							//  Turn off VSG  //
							//----------------//

							printf("\n");

						}  //analysis combination for-loop
					}  //VSA capture for-loop&trigger for-loop
				CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
			} //freq and vsg file for-loop
		printf("Statistics of %d run:\n",numRun);
		printf("Power_NoGap\n	Avg: %.2f dBm	Max: %.2f dBm	Min: %.2f dBm\n", avgPwrSpectral/numRun,maxPWR,minPWR);
		printf("EVM\n	Avg: %.2f dB	Max: %.2f dB	Min: %.2f dB\n\n", avgEVM/numRun,maxEVM,minEVM);
		printf("Capture time:  Max: %.0f ms	Min: %.0f ms\n", maxCaptureTime,minCaptureTime);
		printf("Analysis time:  Max: %.0f ms	Min: %.0f ms\n", maxAnalysisTime,minAnalysisTime);
	}


	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");
	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
	fclose(fp_power);
	fclose( fp_analysis1 );
	fclose(fp_analysis2);
	fclose(fp_analysis3);
}


void QA_Misc(void)
{
	char   buffer[MAX_BUFFER_SIZE];
	double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester
	double cableLoss = 1;           //dB of path loss
	double maxCaptureTime=0.0, minCaptureTime=1e6, maxAnalysisTime=0.0, minAnalysisTime=1e6;
	double freqHz;

	//----------------------------//
	//   Output File Operation    //
	//----------------------------//
	FILE *fp_power, *fp_analysis1, *fp_analysis2, *fp_analysis3;
	char *outFileName  = "11AC_QA_Power.csv";
	char *outFileName1 = "11AC_QA_Analysis1.csv";
	char *outFileName2 = "11AC_QA_Analysis2.csv";
	char *outFileName3 = "11AC_QA_Analysis3.csv";



	try
	{
		fopen_s(&fp_power,outFileName,"w+");
		fopen_s(&fp_analysis1,outFileName1,"w+");
		fopen_s(&fp_analysis2,outFileName2,"w+");
		fopen_s(&fp_analysis3,outFileName3,"w+");

		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(ciTesterType), "LP_Init()" );

		//tyu;2012-7-6; Timer log
		if (ciTesterType == IQTYPE_XEL)
		{
			if (ciTesterControlMode == 0)  // IQapi_scpi
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_QA_Misc_Run_%d.csv",Tag_runTime);
			}
			else   //SCPI
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_QA_Misc_Run_%d.csv",Tag_runTime);
			}
		}
		else
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQ2010_QA_Misc_Run_%d.csv",Tag_runTime);
		}
		//End


		//tyu; 2012-7-6; IP adddress is set in the setup file
		CheckReturnCode( LP_InitTester(g_IP_addr), "LP_InitTester()" );


		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)
			printf("\n%s\n\n", buffer);
		int iRun=0, numRun=5;

		double maxPWR=-999.9, minPWR=999.9,avgPwrSpectral=0;
		double maxEVM=-999.9, minEVM=999.9,avgEVM=0;
		string modFileName = ""; // saveCapturedName="";
		string tempString = "";
		char saveCapturedName[MAX_BUFFER_SIZE];

		sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/FastTrack_Capture-11AC_MCS9_1strm_80MHz_2850bytes_pgap20us_LongGI_AMPDUoff_Fs160-2.iqvsa");
		CheckReturnCode( LP_LoadVsaSignalFile(saveCapturedName), "LP_LoadVsaSignalFile()" );
		CheckReturnCode(LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );

		sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/FastTrack_Capture-11AC_MCS9_1strm_80MHz_2850bytes_pgap20us_LongGI_AMPDUoff_Fs160-3.iqvsa");
		CheckReturnCode(LP_LoadVsaSignalFile(saveCapturedName), "LP_LoadVsaSignalFile()" );
		CheckReturnCode(LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );


		for( int freq_id = 2; freq_id <= 2; freq_id++)
			for (int signal_id = 1; signal_id <= 4; signal_id++)

			{

				if (1==freq_id)
					freqHz = 2422e6;
				else if (2==freq_id)
					freqHz = 5180e6;

				if (1==signal_id)
					tempString = "11AC_MCS8_1strm_80MHz_2550bytes_pgap20us_LongGI_AMPDUoff_Fs160";
				else if (2==signal_id)
					tempString = "11AC_MCS9_1strm_80MHz_2850bytes_pgap20us_LongGI_AMPDUoff_Fs160";
				else if (3==signal_id)
					tempString = "11AC_MCS8_1strm_20MHz_1500bytes_pgap20us_LongGI_AMPDUoff_Fs160";
				else if (4==signal_id)
					tempString = "11AC_MCS9_1strm_40MHz_3000bytes_pgap20us_LongGI_AMPDUoff_Fs160";

				//-------------//
				//  Setup VSG  //
				//-------------//

				CheckReturnCode( LP_SetVsg(freqHz, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

				//--------------------//
				// Load waveform file //
				//--------------------//
				//modFileName="../mod/";
				modFileName="../../iqvsg/";//Luke
				modFileName+=tempString;
				modFileName+=".iqvsg";
				printf("\n Loading mode file %s\n", modFileName.c_str());
				CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );

				//---------------//
				//  Turn on VSG  //
				//---------------//

				CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				//------------------------------------------//
				//   Send packet, continuous transmission   //
				//tyu; 2012-07-07;
				//------------------------------------------//
				int frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
				CheckReturnCode( LP_SetFrameCnt(1), "LP_SetFrameCnt()" );
				while (true)
				{
					int err = LP_TxDone();
					if ( err != 0)
						continue;
					else break;
				}
				CheckReturnCode( LP_SetFrameCnt(0), "LP_SetFrameCnt()" );

				//----------------//
				//   Setup VSA    //
				//----------------//
				int peakToAverageRatio = 10;
				double samplingRate = 160e6;
				double captureTime;

				CheckReturnCode( LP_SetVsa(freqHz, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?

				//----------------------//
				//  Perform VSA capture //
				//----------------------//
				for (int trigger_type = 1; trigger_type <=13; trigger_type+=12 )
					for (int capture_type = 0; capture_type <=6; capture_type+=6 )

					{

						lp_time_t starttime, stoptime;
						if (13 == trigger_type)
							captureTime = 206e-6;
						else
							captureTime = 500e-6;

						fprintf(fp_power,"\n=====%.2f====%s ====",freqHz,tempString.c_str());
						fprintf(fp_power,"=======Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_power,"=====Valid, P_av_each_burst_dBm, P_av_each_burst, P_pk_each_burst_dBm, P_pk_each_burst, P_av_all_dBm, P_av_all, P_peak_all_dBm, P_peak_all, P_av_no_gap_all_dBm, P_av_no_gap_all, start_sec, stop_sec=======================\n");

						fprintf(fp_analysis1,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis1,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis1,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						fprintf(fp_analysis2,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis2,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis2,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						fprintf(fp_analysis3,"\n==== %.2f==== %s =========",freqHz,tempString.c_str());
						fprintf(fp_analysis3,"===========Trigger type %d; Capture Type %d=================\n",trigger_type,capture_type);
						fprintf(fp_analysis3,"=====psduCRC, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_codingRate, rateInfo_modulation, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll, channelEst, vhtSigA1Bandwidth, vhtSigA1Stbc, vhtSigA2ShortGI, vhtSigA2AdvancedCoding, vhtSigA2McsIndex, vhtSigBFieldCRC=======================\n");

						int iRun=0, numRun=5;
						while(iRun<numRun)
						{
							iRun++;
							printf("_________________________________________________\n");
							printf("Run %d\n", iRun);

							GetTime(starttime);

							int intarray[3] = {8192};


							GetTime(stoptime);

							int elapsed_ms = GetElapsedMSec(starttime, stoptime);

							if (13 == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Signal trigger& Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Signal trigger& VHT 80 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}


							if (1  == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Free Run& Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else
									printf("************ Free Run& VHT 80 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}

							if (elapsed_ms<minCaptureTime)
								minCaptureTime=elapsed_ms;
							if(elapsed_ms>maxCaptureTime)
								maxCaptureTime=elapsed_ms;
							//---------------------------//
							//  Save sig file for debug  //
							//---------------------------//
							sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/FastTrack_Capture-11AC_MCS9_1strm_80MHz_2850bytes_pgap20us_LongGI_AMPDUoff_Fs160-2.iqvsa");
							CheckReturnCode( LP_LoadVsaSignalFile(saveCapturedName), "LP_LoadVsaSignalFile()" );
							CheckReturnCode(LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );

							sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/FastTrack_Capture-11AC_MCS9_1strm_80MHz_2850bytes_pgap20us_LongGI_AMPDUoff_Fs160-3.iqvsa");
							CheckReturnCode(LP_LoadVsaSignalFile(saveCapturedName), "LP_LoadVsaSignalFile()" );
							CheckReturnCode(LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );

							//----------------//
							//  Mask Analysis //
							//----------------//
							/* if ( 0 == capture_type)
							   CheckReturnCode(LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );
							   else*/
							if ( 6 == capture_type)
								CheckReturnCode(LP_AnalyzeVHT80Mask(), "LP_AnalyzeVHT80Mask()" );



							//-------------------------//
							//  Retrieve Test Results  //
							//-------------------------//
							double result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13;

							CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );
							result1 = LP_GetScalarMeasurement("valid"); //result not right yet.
							printf("valid: %.0f\n", result1);

							result2 = LP_GetScalarMeasurement("P_av_each_burst_dBm");
							printf("P_av_each_burst_dBm: %.2f dBm\n", result2);

							result3 = LP_GetScalarMeasurement("P_av_each_burst");
							printf("P_av_each_burst: %.6f mw\n", result3);

							result4 = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
							printf("P_pk_each_burst_dBm: %.2f dBm\n", result4);

							result5 = LP_GetScalarMeasurement("P_pk_each_burst");
							printf("P_pk_each_burst: %.6f mw\n", result5);

							result6 = LP_GetScalarMeasurement("P_av_all_dBm");
							printf("P_av_all_dBm: %.2f dBm\n", result6);

							result7 = LP_GetScalarMeasurement("P_av_all");
							printf("P_av_all: %.6f mw\n", result7);

							result8 = LP_GetScalarMeasurement("P_peak_all_dBm");
							printf("P_peak_all_dBm: %.2f dBm\n", result8);

							result9= LP_GetScalarMeasurement("P_peak_all");
							printf("P_peak_all: %.6f mw\n", result9);

							result10 = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
							printf("P_av_no_gap_all_dBm: %.2f dBm\n", result10);

							result11 = LP_GetScalarMeasurement("P_av_no_gap_all");
							printf("P_av_no_gap_all: %.6f mw\n", result11);

							result12 = LP_GetScalarMeasurement("start_sec");
							printf("start_sec: %.6f sec\n", result12);

							result13 = LP_GetScalarMeasurement("stop_sec");
							printf("stop_sec: %.6f sec\n", result13);

							fprintf(fp_power,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.8f,%.8f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13);

							CheckReturnCode( LP_AnalyzePowerRampOFDM(), "LP_AnalyzePowerRampOFDM()" );

							result1 = LP_GetScalarMeasurement("on_time");
							printf("on_time: %.2f\n", result1);
							result2 = LP_GetScalarMeasurement("off_time");
							printf("off_time: %.2f\n", result2);


							double result,result14,result15,result16,result17,result18;
							result = ::LP_GetVectorMeasurement("on_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);



							CheckReturnCode( LP_AnalyzePowerRamp80211b(), "LP_AnalyzePowerRamp80211b()" );

							result1 = LP_GetScalarMeasurement("on_time");
							printf("on_time: %.2f\n", result1);
							result2 = LP_GetScalarMeasurement("off_time");
							printf("off_time: %.2f\n", result2);

							result = ::LP_GetVectorMeasurement("on_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_x", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_mask_y", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_inst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_power_peak", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("on_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);
							result = ::LP_GetVectorMeasurement("off_time_vect", bufferReal, bufferImag, MAX_BUFFER_SIZE);
							printf("size of vector %.2f\n", result);

							//11AC analysis

							printf("\nUsing 11AC analysis, iqapiAnalysis11ac\n");
							lp_time_t starttime4, stoptime4;

							for (int analysis_option_combination =1; analysis_option_combination<=3;analysis_option_combination++)
							{
								if (1==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac(), "LP_Analyze80211ac()" );
								}
								if (2==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac("nxn",0,0,1,1,1,0), "LP_Analyze80211ac()" );
								}
								if (3==analysis_option_combination)
								{
									GetTime(starttime4);
									CheckReturnCode( LP_Analyze80211ac("nxn",0,0,1,1,1,1), "LP_Analyze80211ac()" );
								}

								GetTime(stoptime4);

								int elapsed_ms4 = GetElapsedMSec(starttime4, stoptime4);
								printf("************ Elapsed time for LP_Analyze80211ac() option %d: %d ms ************\n", analysis_option_combination, elapsed_ms4);

								if (elapsed_ms4<minAnalysisTime)
									minAnalysisTime=elapsed_ms4;

								if(elapsed_ms4>maxAnalysisTime)
									maxAnalysisTime=elapsed_ms4;

								//11n and 11ac result

								result1 = LP_GetScalarMeasurement("psduCRC");
								if (result1 == 1)
								{
									printf("PSDU CRC: %s \n", "PASS");
								}
								else
									printf("PSDU CRC: %s \n", "FAIL");

								//Not working for 1 stream signal.
								// result = LP_GetScalarMeasurement("isolationDb");
								// printf("isolationDb=%0.2f \n", result);

								result2 = LP_GetScalarMeasurement("freqErrorHz");
								printf("Frequency Error: %.2f Hz\n", result2);

								result3 = LP_GetScalarMeasurement("symClockErrorPpm");
								printf("Symbol Clock Error: %.2f ppm\n", result3);

								result4 = LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll");
								printf("RMS Phase Noise: %.2f deg\n", result4);

								result5 = LP_GetScalarMeasurement("IQImbal_amplDb");
								printf("IQ Imbalance Amp: %.2f dB \n", result5);

								result6 = LP_GetScalarMeasurement("IQImbal_phaseDeg");
								printf("IQ Imbalance phase: %0.2f deg \n", result6);

								result7 = LP_GetScalarMeasurement("dcLeakageDbc");
								printf("LO leakage: %.2f dBc\n\n", result7);

								double bandwidthMhz = LP_GetScalarMeasurement("rateInfo_bandwidthMhz");
								printf("Signal Bandwidth: %0.2f MHz\n", bandwidthMhz);

								result8 = LP_GetScalarMeasurement("rateInfo_spatialStreams");
								printf("Number of Spatial Streams: %1.0f \n", result8);

								result9 = LP_GetScalarMeasurement("rateInfo_spaceTimeStreams");
								printf("Number of SpaceTime Streams: %1.0f \n", result9);

								char   result_char1[255];
								char   result_char2[255];

								LP_GetStringMeasurement("rateInfo_codingRate",result_char1,255);
								printf("codingRate: %s \n", result_char1);

								LP_GetStringMeasurement("rateInfo_modulation",result_char2,255);
								printf("modulation: %s \n", result_char2);

								result10 = LP_GetScalarMeasurement("rateInfo_dataRateMbps");
								printf("Datarate: %0.2f Mbps\n", result10);

								result11 = LP_GetScalarMeasurement("rxRmsPowerDb");
								printf("Power_NoGap: %.3f dBm\n", result11);

								if (result11 > maxPWR)
								{
									maxPWR=result11;
								}
								if(result11<minPWR)
								{
									minPWR=result11;
								}
								avgPwrSpectral+=result11;

								result12 = LP_GetScalarMeasurement("evmAvgAll");
								printf("EVM Avg All: %.3f dB\n\n", result12);

								if (result12 > maxEVM)
								{
									maxEVM=result12;
								}
								if(result12<minPWR)
								{
									minEVM=result12;
								}
								avgEVM+=result12;

								//			result = LP_GetScalarMeasurement("htSigFieldCRC");
								//			printf("htSigFieldCRC=%0.2f \n", result);

								result = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
								if (result <= 0)
								{
									printf("Channel Estimation returns an error\n");
								}
								else
								{
									//cacluate the spectrum flatness.
									printf("Channel Estimation returns a vector with %3.0f elements\n", result);
									int numSubcarrier = 256; //number of Subcarrier for 80MHz
									double  *carrierPwr;
									int centerStart, centerEnd, sideStart, sideEnd;
									//iqPwr  = (double *) malloc (result * sizeof (double));
									carrierPwr     = (double *) malloc (numSubcarrier * sizeof (double));
									double avgCenterPwr=0.0, avgSidePwr=0.0, avgPwrSpectral=0.0;
									double maxCarrierPwr = -999.9, minCenterCarrierPwr = 99.9, minSideCarrierPwr = 99.9;

									if (bandwidthMhz == 20.0)
									{
										numSubcarrier = 64;
										centerStart   = CARRIER_1;
										centerEnd     = CARRIER_16;
										sideStart     = CARRIER_17;
										sideEnd       = CARRIER_28;
									}
									else
									{
										if (bandwidthMhz == 40.0)
										{
											numSubcarrier = 128;
											centerStart   = CARRIER_2;
											centerEnd     = CARRIER_42;
											sideStart     = CARRIER_43;
											sideEnd       = CARRIER_58;
										}
										else
										{
											numSubcarrier = 256;
											centerStart   = CARRIER_2;
											centerEnd     = CARRIER_84;
											sideStart     = CARRIER_85;
											sideEnd       = CARRIER_122;
										}
									}
									if (result != numSubcarrier)
									{
										printf("Channel estimation results does not match number of subcarriers for %2.0f MHz signal.\n", bandwidthMhz);
									}
									else
									{
										for (int carrier=0;carrier<result;carrier++) //only one stream here.
										{
											carrierPwr[carrier] = bufferReal[carrier]*bufferReal[carrier]+ bufferImag[carrier]*bufferImag[carrier];
										}
										// average power in the center
										for ( int i=centerStart;i<=centerEnd;i++)
										{
											avgCenterPwr = avgCenterPwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
										}
										// average power on side lobe.
										for (int i=sideStart;i<=sideEnd;i++)
										{
											avgSidePwr = avgSidePwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
										}

										avgPwrSpectral = avgCenterPwr+ avgSidePwr;

										avgCenterPwr = avgCenterPwr/((centerEnd-centerStart+1)*2);
										if (0!=avgCenterPwr)
										{
											avgCenterPwr = 10.0 * log10 (avgCenterPwr);
										}
										else
										{
											avgCenterPwr= 0;
										}
										avgPwrSpectral = avgPwrSpectral/((sideEnd-centerStart+1)*2);
										if (0!=avgPwrSpectral)
										{
											avgPwrSpectral = 10.0 * log10 (avgPwrSpectral);
										}
										else
										{
											avgPwrSpectral = 0;
										}

										for (int carrier=0;carrier<numSubcarrier;carrier++)
										{
											if (0!=carrierPwr[carrier])
											{
												carrierPwr[carrier] = 10.0 * log10 (carrierPwr[carrier]);
												if (carrierPwr[carrier]>maxCarrierPwr)
												{
													maxCarrierPwr = carrierPwr[carrier];
												}
												if ( (carrier>=centerStart && carrier <=centerEnd) || (carrier>=numSubcarrier-centerEnd && carrier <= numSubcarrier-centerStart) )
												{
													if(carrierPwr[carrier]< minCenterCarrierPwr)
													{
														minCenterCarrierPwr = carrierPwr[carrier];
													}
												}
												if ((carrier>=sideStart && carrier <=sideEnd) || (carrier>=numSubcarrier-sideEnd && carrier <=numSubcarrier-sideStart) )
												{
													if(carrierPwr[carrier]< minSideCarrierPwr)
													{
														minSideCarrierPwr = carrierPwr[carrier];
													}
												}
											}
											else
											{
												carrierPwr[carrier] = 0;
											}
										}
										printf("The maximum deviation is %0.2f dB\n", maxCarrierPwr-avgCenterPwr);
										printf("The minimum deviation are %0.2f dB in the center, %0.2f dB in the far side\n", minCenterCarrierPwr-avgCenterPwr, minSideCarrierPwr-avgCenterPwr);
										printf("The LO Leakage is %0.2f dBc \n", carrierPwr[0] - avgPwrSpectral - 10 * log10((double)((sideEnd-centerStart+1)*2)) );
									}

								}

								printf("11AC VHT Sig field Info:\n");
								result13 = LP_GetScalarMeasurement("vhtSigA1Bandwidth");
								printf("VHTSigA1Bandwidth=%1.0f. Means: %3.0f MHz\n", result13, pow(2,result13)*20);

								result14 = LP_GetScalarMeasurement("vhtSigA1Stbc");
								printf("VHTSigA1Stbc=%1.0f. Means: ", result14);
								if(result14 == 0)
								{
									printf("STBC is not enabled for all streams.\n");
								}
								else
									printf("STBC is enabled for all streams.\n");
								//printf("VHTSigA1Stbc=%1.0f\n", result);

								result15 = LP_GetScalarMeasurement("vhtSigA2ShortGI");
								printf("VHTSigA2ShortGI=%1.0f\n", result);
								result16 = LP_GetScalarMeasurement("vhtSigA2AdvancedCoding");
								printf("VHTSigA2AdvancedCoding=%1.0f\n", result);
								result17 = LP_GetScalarMeasurement("vhtSigA2McsIndex");
								printf("VHTSigA2McsIndex=%1.0f\n", result);

								result18 = LP_GetScalarMeasurement("vhtSigBFieldCRC");
								printf("vhtSigBFieldCRC=%1.0f\n\n", result);

								if(1== analysis_option_combination)
									fprintf(fp_analysis1,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);

								if(2== analysis_option_combination)
									fprintf(fp_analysis2,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);

								if(3== analysis_option_combination)
									fprintf(fp_analysis3,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %s, %s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,bandwidthMhz,result8,result9,result_char1,result_char2,result10,result11,result12,result,result13,result14,result15,result16,result17,result18);


							}
							//----------------//
							//  Turn off VSG  //
							//----------------//

							printf("\n");

							//printf("Press 'X' key to exit or any key to repeat the test..................\n");

							//char ch = toupper(_getch());
							//if (ch=='X' || ch==ASCII_ESC)
							//    break;

						}  //analysis combination for-loop
					}  //VSA capture for-loop&trigger for-loop
				CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
			} //freq and vsg file for-loop
		printf("Statistics of %d run:\n",numRun);
		printf("Power_NoGap\n	Avg: %.2f dBm	Max: %.2f dBm	Min: %.2f dBm\n", avgPwrSpectral/numRun,maxPWR,minPWR);
		printf("EVM\n	Avg: %.2f dB	Max: %.2f dB	Min: %.2f dB\n\n", avgEVM/numRun,maxEVM,minEVM);
		printf("Capture time:  Max: %.0f ms	Min: %.0f ms\n", maxCaptureTime,minCaptureTime);
		printf("Analysis time:  Max: %.0f ms	Min: %.0f ms\n", maxAnalysisTime,minAnalysisTime);
	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);

	}
	catch(...)
	{
		printf("ERROR!");

	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
	fclose(fp_power);
	fclose( fp_analysis1 );
	fclose(fp_analysis2);
	fclose(fp_analysis3);
	ReadLogFiles();
}

/*******************************

  QA_Mask() tested LP_AnalyzeFFT LP_AnalyzeHT40FFT LP_AnalyzeVHT80Mask()
  Generated FFT_Mask.csv(120M hz BW, 3073 points) HT40_FFT_Mask.csv(120M hz BW, 3073 points) and VHT80_Mask.csv (240M hz BW, 6145 points)

 ************************************/
void QA_Mask(void)
{
	char   buffer[MAX_BUFFER_SIZE];

	//tyu; 2012-07-19; Fix bug
	//double  bufferReal[MAX_BUFFER_SIZE], bufferImag[MAX_BUFFER_SIZE];
	double  bufferReal[MAX_BUFFER_SIZE*2], bufferImag[MAX_BUFFER_SIZE*2];

	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester
	double cableLoss = 1;           //dB of path loss
	double maxCaptureTime=0.0, minCaptureTime=1e6, maxAnalysisTime=0.0, minAnalysisTime=1e6;
	double freqHz;

	//----------------------------//
	//   Output File Operation    //
	//----------------------------//
	FILE *FFT, *HT40FFT, *VHT80FFT;
	char *outFileName  = ".\\Log\\FFT_Mask.csv";
	char *outFileName1 = ".\\Log\\HT40_FFT_Mask.csv";
	char *outFileName2 = ".\\Log\\VHT80_Mask.csv";


	fopen_s(&FFT,outFileName,"w+");
	fopen_s(&HT40FFT,outFileName1,"w+");
	fopen_s(&VHT80FFT,outFileName2,"w+");


	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(ciTesterType, ciTesterControlMode), "LP_Init()" );

		//tyu;2012-7-6; Timer log
		if (ciTesterType == IQTYPE_XEL)
		{
			if (ciTesterControlMode == 0)  // IQapi_scpi
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_Run_%d.csv",Tag_runTime);
			}
			else   //SCPI
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_Run_%d.csv",Tag_runTime);
			}
		}
		else
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQ2010_Run_%d.csv",Tag_runTime);
		}
		//End

		//tyu; 2012-7-6; IP adddress is set in the setup file
		CheckReturnCode( LP_InitTester(g_IP_addr), "LP_InitTester()" );


		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)
			printf("\n%s\n\n", buffer);
		int iRun=0, numRun=5;

		double maxPWR=-999.9, minPWR=999.9,avgPwrSpectral=0;
		double maxEVM=-999.9, minEVM=999.9,avgEVM=0;
		string modFileName = ""; // saveCapturedName="";
		string tempString = "";
		char saveCapturedName[MAX_BUFFER_SIZE];


		for( int freq_id = 1; freq_id <= 2; freq_id++)
			for (int signal_id = 1; signal_id <= 4; signal_id++)

			{

				if (1==freq_id)
					freqHz = 2422e6;
				else if (2==freq_id)
					freqHz = 5180e6;

				if (ciTesterType == IQTYPE_XEL)
				{
					if (1==signal_id)
						tempString = "WiFi_11AC_VHT80_S1_MCS9";
					else if (2==signal_id)
						tempString = "WiFi_11AC_VHT80_S1_MCS8";
					else if (3==signal_id)
						tempString = "WiFi_11AC_VHT40_S1_MCS9";
					else if (4==signal_id)
						tempString = "WiFi_11AC_VHT20_S1_MCS8";
					modFileName="../../iqvsg/";
					modFileName+=tempString;
					modFileName+=".iqvsg";
				}
				else
				{
					if (1==signal_id)
						tempString = "WiFi_HT40_MCS7";
					else if (2==signal_id)
						tempString = "WiFi_HT40_MCS14";
					else if (3==signal_id)
						tempString = "WiFi_HT20_MCS7";
					else if (4==signal_id)
						tempString = "WiFi_HT20_MCS14";
					modFileName="../../mod/";
					modFileName+=tempString;
					modFileName+=".mod";
				}

				//-------------//
				//  Setup VSG  //
				//-------------//

				CheckReturnCode( LP_SetVsg(freqHz, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

				//--------------------//
				// Load waveform file //
				//--------------------//
				//modFileName="../mod/";
				//modFileName="../../iqvsg/";//Luke: move up
				//modFileName+=tempString;
				//modFileName+=".iqvsg";
				printf("\n Loading mode file %s\n", modFileName.c_str());
				CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );

				//---------------//
				//  Turn on VSG  //
				//---------------//
				CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				//------------------------------------------//
				//   Send packet, continuous transmission   //
				//tyu; 2012-07-07;
				//------------------------------------------//
				int frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
				CheckReturnCode( LP_SetFrameCnt(1), "LP_SetFrameCnt()" );
				while (true)
				{
					int err = LP_TxDone();
					if ( err != 0)
						continue;
					else break;
				}
				CheckReturnCode( LP_SetFrameCnt(0), "LP_SetFrameCnt()" );

				//----------------//
				//   Setup VSA    //
				//----------------//
				int peakToAverageRatio = 10;
				double samplingRate = 0;
				double captureTime;
				int capture_ref =0;

				CheckReturnCode( LP_SetVsa(freqHz, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?

				//CheckReturnCode( LP_SetVsa(FREQ_HZ, -15, PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" );

				//----------------------//
				//  Perform VSA capture //
				//----------------------//
				//Added by Luke****************
				if (ciTesterType == IQTYPE_XEL)
				{
					samplingRate = 160e6;
					capture_ref = 6;
				}
				else
				{
					samplingRate = 80e6;
					capture_ref = 5;
				}
				//**********************end
				for (int trigger_type = 13; trigger_type <=13; trigger_type+=12 )
					for (int capture_type = 0; capture_type <=capture_ref; capture_type+=capture_ref )

					{

						lp_time_t starttime, stoptime;
						if (13 == trigger_type)
							captureTime = 206e-6;
						else
							captureTime = 500e-6;

						int iRun=0, numRun=1;
						while(iRun<numRun)
						{
							iRun++;
							printf("_________________________________________________\n");
							printf("Run %d\n", iRun);

							GetTime(starttime);

							CheckReturnCode( LP_VsaDataCapture(captureTime, trigger_type, samplingRate, capture_type), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?

							GetTime(stoptime);

							int elapsed_ms = GetElapsedMSec(starttime, stoptime);

							if (13 == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Signal trigger& Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else if (5 ==capture_type)
									printf("************ IQ2010 Signal trigger& VHT 40 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else if (6 ==capture_type)
									printf("************ IQXEL Signal trigger& VHT 80 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}


							if (1  == trigger_type)
							{
								if (0 == capture_type)
									printf("************ Free run& Normal Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else if (5 ==capture_type)
									printf("************ IQ2010 Free run& VHT 40 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
								else if (6 ==capture_type)
									printf("************ IQXEL Free run& VHT 80 Capture Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);

							}

							if (elapsed_ms<minCaptureTime)
								minCaptureTime=elapsed_ms;
							if(elapsed_ms>maxCaptureTime)
								maxCaptureTime=elapsed_ms;
							//---------------------------//
							//  Save sig file for debug  //
							//---------------------------//

							//	sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.iqvsa", "FastTrack_Capture", tempString.c_str(),iRun);
							//tyu;2012-07-17;  support both IQ2010 and IQXel
							if (ciTesterType == IQTYPE_XEL)
							{
								sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-Trig%d-Capture%d-%d.iqvsa", "FastTrack_Capture", tempString.c_str(),trigger_type, capture_type,iRun);
							}
							else		//IQ2010/view/flex
							{
								sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-Trig%d-Capture%d-%d.sig", "FastTrack_Capture", tempString.c_str(),trigger_type, capture_type,iRun);
							}
							//end
							CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );



							//----------------//
							//  Mask Analysis //
							//----------------//
							if ( 0 == capture_type)
							{
								CheckReturnCode(LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );

								int length_20x = LP_GetVectorMeasurement("x", bufferReal, bufferImag, 6075);
								//printf ("frequency: length_20x = %d ***************", length_20x);
								for (int i=0;i<length_20x;i++)
									fprintf(FFT,"%.2f,",bufferReal[i]);
								fprintf(FFT,"\n");

								int length_20y = LP_GetVectorMeasurement("y", bufferReal, bufferImag, 6075);
								for (int i=0;i<length_20y;i++)
									fprintf(FFT,"%.2f,",bufferReal[i]);
								fprintf(FFT,"\n\n");
								if (ciTesterType == IQTYPE_XEL)
								{
									CheckReturnCode(LP_AnalyzeHT40Mask(), "LP_AnalyzeHT40FFT()" );
									int length_40x = LP_GetVectorMeasurement("x", bufferReal, bufferImag, 3075);
									for (int i=0;i<length_40x;i++)
										fprintf(HT40FFT,"%.2f,",bufferReal[i]);
									fprintf(HT40FFT,"\n");
									int length_40y = LP_GetVectorMeasurement("y", bufferReal, bufferImag, 3075);
									for (int i=0;i<length_40y;i++)
										fprintf(HT40FFT,"%.2f,",bufferReal[i]);
									fprintf(HT40FFT,"\n\n");

								}
							}
							else if (5 == capture_type)//Luke: add for 2010 40MHZ capture
							{
								CheckReturnCode(LP_AnalyzeHT40Mask(), "LP_AnalyzeHT40FFT()" );
								int length_40x_ = LP_GetVectorMeasurement("x", bufferReal, bufferImag, 6150);
								for (int i=0;i<length_40x_;i++)
									fprintf(HT40FFT,"%.2f,",bufferReal[i]);
								fprintf(HT40FFT,"\n");

								int length_40y_ = LP_GetVectorMeasurement("y", bufferReal, bufferImag, 6150);
								for (int i=0;i<length_40y_ ;i++)
									fprintf(HT40FFT,"%.2f,",bufferReal[i]);
								fprintf(HT40FFT,"\n\n");
							}


							//fprintf(fp_power,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.8f,%.8f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13);

							else if ( 6 == capture_type)
							{
								CheckReturnCode(LP_AnalyzeVHT80Mask(), "LP_AnalyzeVHT80Mask()" );

								//tyu; 2012-07-19; Fix bug
								int length_80x=LP_GetVectorMeasurement("x", bufferReal, bufferImag, 6150);
								for (int i=0;i<length_80x;i++)
									fprintf(VHT80FFT,"%.2f,",bufferReal[i]);
								fprintf(VHT80FFT,"\n");
								int length_80y = LP_GetVectorMeasurement("y", bufferReal, bufferImag, 6150);
								for (int i=0;i<length_80y;i++)
									fprintf(VHT80FFT,"%.2f,",bufferReal[i]);
								fprintf(VHT80FFT,"\n\n");

							}
							//-------------------------//
							//  Retrieve Test Results  //
							//-------------------------//


							//----------------//
							//  Turn off VSG  //
							//----------------//

							printf("\n");

							//printf("Press 'X' key to exit or any key to repeat the test..................\n");


						} //analysis combination for-loop
					}  //VSA capture for-loop&trigger for-loop
				CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
			} //freq and vsg file for-loop

	}


	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);

	}
	catch(...)
	{
		printf("ERROR!");

	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
	fclose(FFT);
	fclose(HT40FFT);
	fclose(VHT80FFT);
	ReadLogFiles();

}



void AC_MIMO_Loopback(void)
{
	char   	buffer[400];
	double 	dutPowerLevel = -10;		// Estimated RMS power level at IQTester
	double  bufferReal[400];
	double	bufferImag[400];
	//double	bufferImag[MAX_BUFFER_SIZE];
	double 	cableLoss = 4; //dB of path loss
	int 	numMimo	= 3;





	try
	{
		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(ciTesterType, ciTesterControlMode), "LP_Init()" );

		//tyu;2012-7-6; Timer log
		if (ciTesterType == IQTYPE_XEL)
		{
			if (ciTesterControlMode == 0)  // IQapi_scpi
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_AC_MIMO_Loopback_Run_%d.csv",Tag_runTime);
			}
			else   //SCPI
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_AC_MIMO_Loopback_Run_%d.csv",Tag_runTime);
			}
		}
		else
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQ2010_AC_MIMO_Loopback_Run_%d.csv",Tag_runTime);
		}
		//End
		//CheckReturnCode( LP_InitTester(IP_ADDR), "LP_InitTester()" );
		//CheckReturnCode( LP_InitTester( "192.168.0.213:IQP07973"), "LP_InitTester()" );
		if (numMimo == 3){
			CheckReturnCode( LP_InitTester3( IP_ADDR_IQXEL1, IP_ADDR_IQXEL2, IP_ADDR_IQXEL3), "LP_InitTester3()" );
			// DTNA00057=192.168.1.2; DTNA00026=192.168.1.66; DTNA00021=192.168.4.177
		}else if (numMimo == 2){
			CheckReturnCode( LP_InitTester2( "192.168.4.177", "192.168.0.220"), "LP_InitTester2()" );
		}else{ //by default only 1 tester.
			CheckReturnCode( LP_InitTester( "192.168.4.177"), "LP_InitTester1()" );
			//			CheckReturnCode( LP_InitTester( "192.168.2.74"), "LP_InitTester1()" );
		}
		//CheckReturnCode( LP_InitTester2( "192.168.109.180", "192.168.109.181"), "LP_InitTester2()" );
		//CheckReturnCode( LP_InitTester2( "192.168.3.2", "192.168.1.66"), "LP_InitTester2()" );
		//CheckReturnCode( LP_InitTester3( "192.168.3.2", "192.168.1.66","192.168.1.2"), "LP_InitTester3()" );
		//CheckReturnCode( LP_InitTester2( "192.168.3.2", "192.168.1.66"), "LP_InitTester2()" );
		//CheckReturnCode( LP_InitTester3( "192.168.109.180", "192.168.109.181", "192.168.109.182"), "LP_InitTester3()" );
		//CheckReturnCode( LP_InitTester( "192.168.0.213:IQP07973"), "LP_InitTester()" );
		//CheckReturnCode( LP_InitTester2( "192.168.0.213:4000:IQP07973", "192.168.0.213:4000:IQP08364"), "LP_InitTester2()" );
		//CheckReturnCode( LP_InitTester3( "192.168.3.36:4000:IQP07973", "192.168.3.36:4000:IQP08364", "192.168.3.36:4000:IQP07796"), "LP_InitTester3()" );
		//CheckReturnCode( LP_InitTester( "127.0.0.1:4000:IQP07973"), "LP_InitTester()" );
		//CheckReturnCode( LP_InitTester2( "192.168.0.213:4000:IQP07973", "192.168.0.213:4000:IQP08364"), "LP_InitTester2()" );
		//CheckReturnCode( LP_InitTester3( "192.168.3.36:4000:IQP07973", "192.168.3.36:4000:IQP08364", "192.168.3.36:4000:IQP07796"), "LP_InitTester3()" );
		//CheckReturnCode( LP_InitTester3( "192.168.3.36:IQP07973", "192.168.3.36:IQP08364", "192.168.3.36:IQP07796"), "LP_InitTester3()" );


		//CheckReturnCode( LP_InitTester( "192.168.0.213:4000:IQP07973"),"LP_InitTester()");
		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)
			printf("\n%s\n\n", buffer);
		if (0)
		{
			CheckReturnCode( LP_LoadVsaSignalFile("../log/WiFi_TX_Mask_SaveAlways_5220_OFDM-9_BW-20-2012.4.20-13.9.58.625.iqvsa"), "LP_LoadVsaSignalFile()" );
			printf("\n Analyzing normal Mask\n");
			CheckReturnCode( LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );
			double bufferRealX[MAX_BUFFER_SIZE], bufferImagX[MAX_BUFFER_SIZE];
			double bufferRealY[MAX_BUFFER_SIZE], bufferImagY[MAX_BUFFER_SIZE];
			int bufferSizeX = ::LP_GetVectorMeasurement("x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE);
			int bufferSizeY = ::LP_GetVectorMeasurement("y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE);
			printf("\n VHT80MHz Wideband mask starts at %4.0f MHz results \n", bufferRealX[0]/(1e6) );
		}



		////check composite EVM
		//for (int numCompStr=2;  numCompStr<4; numCompStr++)
		//{
		//	char tmpCompEvmStr[MAX_BUFFER_SIZE], tmpRefStr[MAX_BUFFER_SIZE];
		//	//string tmpCompEvmStr1 = "";
		//	double resultComp;
		//	for (int iMcs = 0; iMcs<10; iMcs++)
		//	{
		//		//numCompStr =2;
		//		//iMcs =9;
		//		if(numCompStr==3 && iMcs==6)
		//		{
		//			//not allow by standard
		//		}
		//		else
		//		{
		//			sprintf_s(tmpCompEvmStr, MAX_BUFFER_SIZE, "../com_capture/%s%d%s%d.iqvsa", "WiFi_11AC_VHT80_S", numCompStr,"_MCS", iMcs);
		//			printf("\nLoading composite capture file %s\n",tmpCompEvmStr);
		//			CheckReturnCode( LP_LoadVsaSignalFile(tmpCompEvmStr), "LP_LoadVsaSignalFile()" );

		//			sprintf_s(tmpRefStr, MAX_BUFFER_SIZE, "../com_capture/%s%d%s%d.iqref", "WiFi_11AC_VHT80_S", numCompStr,"_MCS", iMcs);
		//			printf("Perform composite EVM analysis using ref file %s\n",tmpRefStr);
		//			CheckReturnCode( LP_Analyze80211ac("composite", 1, 1, 0, 0, 0, IQV_FREQUENCY_CORR_LTF+1,tmpRefStr,0), "LP_Analyze80211ac()" );

		//			resultComp = LP_GetScalarMeasurement("evmAvgAll");
		//			printf("EVM Avg All: %.3f dB\n\n", resultComp);
		//		}
		//	}

		//}
		int 	dynamicRange = 3;
		int 	numRun = 1;
		double 	maxPWRDynamicRange[MAX_BUFFER_SIZE] = {0.0};
		double	minPWRDynamicRange[MAX_BUFFER_SIZE] = {0.0};
		double	avgPWRDynamicRange[MAX_BUFFER_SIZE] = {0.0};
		double 	maxEVMDynamicRange[MAX_BUFFER_SIZE] = {0.0};
		double	minEVMDynamicRange[MAX_BUFFER_SIZE] = {0.0};
		double	avgEVMDynamicRange[MAX_BUFFER_SIZE] = {0.0};
		string 	tmpStr1 = "";
		string	modFileName = ""; //saveCapturedName="";
		char 	saveCapturedName[MAX_BUFFER_SIZE] = {'\0'};

		//-------------//
		//  Setup VSG  //
		//-------------//

		CheckReturnCode( LP_SetVsg(FREQ_HZ, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

		//--------------------//
		// Load waveform file //
		//--------------------//
		//tmpStr1="WiFi_CCK-11S";
		//tmpStr1="WiFi_OFDM-54";
		//tmpStr1="WiFi_HT20_MCS7";
		//tmpStr1="WiFi_HT40_MCS21";
		//tmpStr1="11ac_80MHz_mcs8_3000bytes";
		//tmpStr1="11ac_80MHz_mcs9";
		//tmpStr1="11ac_80MHz_mcs8_new";
		//tmpStr1="11ac_80MHz_MCS9_2s_4000_byte";
		//tmpStr1="WIFI_AC_BW80_SS1_MCS9_BCC_Fs160M";
		//tmpStr1="WiFi_11AC_VHT20_S2_MCS8";
		tmpStr1="WiFi_11AC_VHT80_S3_MCS9";
		//tmpStr1="WiFi_11AC_VHT20_S3_MCS8";

		//tmpStr1="11ac_VHT40_MCS9_3s_2000_bytes";
		//tmpStr1="11ac_80MHz_MCS9_3s_4000_byte";
		//tmpStr1="11ac_VHT40_MCS9_3s_2000_bytes";
		//tmpStr1="VHT-160MHz_MCS9_3s_4000_byte_63ms";
		//        tmpStr1="WiFi_11AC_VHT80_S2_MCS7";

		if (numMimo == 3)
			tmpStr1="WiFi_11AC_VHT80_S3_MCS9";
		else if (numMimo == 2)
			tmpStr1="WiFi_11AC_VHT80_S2_MCS9";
		else //by default only 1 tester.
			tmpStr1="WiFi_11AC_VHT80_S1_MCS9";
		//tmpStr1="WIFI_AC_BW80_SS2_MCS9_BCC_Fs160M";
		//modFileName = "../mod/";
		modFileName = "../../iqvsg/";
		modFileName += tmpStr1;
		//modFileName+=".mod";
		modFileName += ".iqvsg";

		printf("\nLoading mod file %s\n",modFileName.c_str());
		CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );
		//CheckReturnCode( LP_SetVsgModulation("../mod/WiFi_OFDM-54.mod"), "LP_SetVsgModulation()" );
		//CheckReturnCode( LP_SetVsgModulation("../mod/WiFi_HT20_MCS7.mod"), "LP_SetVsgModulation()" );
		//CheckReturnCode( LP_SetVsgModulation("../mod/WiFi_HT40_MCS7.mod"), "LP_SetVsgModulation()" );
		//CheckReturnCode( LP_SetVsgModulation("11ac_80MHz_mcs8_3000bytes.mod"), "LP_SetVsgModulation()" );

		FILE 	*fp_11AC_MIMO = NULL;
		char 	*outFileName = "11AC_QA_MIMO.csv";
		fopen_s(&fp_11AC_MIMO,outFileName,"w+");

		for ( int iPwrErr = 0-dynamicRange; iPwrErr <= dynamicRange+6; iPwrErr++ )
		{
			double 	maxPWR=-999.9, minPWR=999.9, avgPWR=0;
			double 	maxEVM=-999.9, minEVM=999.9, avgEVM=0;
			int 	numAnalysisErr = 0, iRun = 0;

			printf("\n VSA Amplitude Level is offset by %d dB\n",iPwrErr);
			while(iRun<numRun)
			{
				iRun++;
				printf("_________________________________________________\n");
				printf("Run %d\n", iRun);
				//---------------//
				//  Turn on VSG  //
				//---------------//
				CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				//CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );

				//CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				//------------------------------------------//
				//   Send packet, continuous transmission   //
				//------------------------------------------//
				for (int frameCnt = 200; frameCnt<2000;frameCnt+=100) //100;	// number of frame to send, 0 means continuous transmission.
				{
					CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );

					// Test TX_DONE
					int err;
					while ( 1 )
					{
						err = ::LP_TxDone();
						if (ERR_OK!=err)	// Tx not finish yet...
						{
							printf("TX is not done\n");
							Sleep(50);

						}
						else
						{
							printf("TX is done\n");
							// Tx Done, then break the while loop.
							break;
						}
					}
				}
				//frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
				//CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );

				int frameCnt = 10; //100;	// number of frame to send, 0 means continuous transmission.
				CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );

				frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
				CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );

				frameCnt = 10; //100;	// number of frame to send, 0 means continuous transmission.
				CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );

				//CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );

				frameCnt = 50; //100;	// number of frame to send, 0 means continuous transmission.
				CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );

				//CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				frameCnt = 0; //100;	// number of frame to send, 0 means continuous transmission.
				CheckReturnCode( LP_SetFrameCnt(frameCnt), "LP_SetFrameCnt()" );


				//CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );

				//----------------//
				//   Setup VSA    //
				//----------------//
				int 	peakToAverageRatio = 10;
				double 	freqHz = 5540e6;
				double 	samplingRate = 160e6;

				CheckReturnCode( LP_SetVsa(FREQ_HZ, (dutPowerLevel-cableLoss+peakToAverageRatio+iPwrErr), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?

				//CheckReturnCode( LP_SetVsa(FREQ_HZ, -15, PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" );

				//----------------------//
				//  Perform VSA capture //
				//----------------------//
				double captureTime = 1000e-6;
				CheckReturnCode( LP_VsaDataCapture(captureTime, 13, samplingRate), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?
				//CheckReturnCode( LP_VsaDataCapture(captureTime, 1, samplingRate), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?

				//---------------------------//
				//  Save sig file for debug  //
				//---------------------------//
				//saveCapturedName ="./log/DTNA_Capture";
				//saveCapturedName +=tmpStr1;
				//saveCapturedName +=i;
				//saveCapturedName +=".sig";
				sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s_%s_offset_%ddB_%d.iqvsa", "DTNA_Capture", tmpStr1.c_str(),iPwrErr,iRun);
				//sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.iqvsa", "DTNA_Capture", tmpStr1.c_str(),iRun);
				//CheckReturnCode( LP_SaveVsaSignalFile((char *)saveCapturedName.c_str()), "LP_SaveVsaSignalFile()" );
				CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

				//------------------------//
				//  Perform WiFi analysis //
				//------------------------//

				////11b analysis, for test purpose although it is not supported in 11ac
				//printf("\nUsing 11b analysis, iqapiAnalysis11b\n");
				//CheckReturnCode( LP_Analyze80211b(), "LP_Analyze80211b()" );

				////11ag analysis
				//         printf("\nUsing OFDM analysis, iqapiAnalysisOFDM\n");
				//CheckReturnCode( LP_Analyze80211ag(), "LP_Analyze80211ag()" );


				//         //-------------------------//
				//         //  Retrieve Test Results  //
				//         //-------------------------//
				double 	result;


				////11b results
				//result = LP_GetScalarMeasurement("evmPk");
				//printf("evmPk: %0.2f \n", result);

				//			 //11ag results
				//
				//	         result = LP_GetScalarMeasurement("psduCrcFail");
				//			 printf("psduCrcFail: %0.2f \n", result);
				//	         result = LP_GetScalarMeasurement("plcpCrcPass");
				//			 printf("plcpCrcPass: %0.2f \n", result);
				//	         result = LP_GetScalarMeasurement("dataRate");
				//	         printf("Datarate=%0.2f Mbps\n", result);
				//	         result = LP_GetScalarMeasurement("numSymbols");
				//	         result = LP_GetScalarMeasurement("numPsduBytes");
				//	         result = LP_GetScalarMeasurement("evmAll");
				//	         printf("EVM_All=%.3f dB\n", result);
				//	         result = LP_GetScalarMeasurement("evmData");
				//	         result = LP_GetScalarMeasurement("evmPilot");
				//	         result = LP_GetScalarMeasurement("codingRate");
				//	         result = LP_GetScalarMeasurement("freqErr");
				//	         result = LP_GetScalarMeasurement("clockErr");
				//	         result = LP_GetScalarMeasurement("ampErr");
				//	         result = LP_GetScalarMeasurement("ampErrDb");
				//	         result = LP_GetScalarMeasurement("phaseErr");
				//	         result = LP_GetScalarMeasurement("rmsPhaseNoise");
				//	         result = LP_GetScalarMeasurement("rmsPowerNoGap");
				//	         printf("Power_NoGap=%.3f dBm\n", result);
				//	         result = LP_GetScalarMeasurement("rmsPower");
				//	         result = LP_GetScalarMeasurement("pkPower");
				//	         result = LP_GetScalarMeasurement("rmsMaxAvgPower");
				//			 result = LP_GetScalarMeasurement("dcLeakageDbc");
				//			 printf("LO leakage is: %.2f dBc\n", result);
				//
				//
				//			 //11n analysis
				//printf("\nUsing 11n analysis, iqapiAnalysis11ac\n");
				//CheckReturnCode( LP_Analyze80211n("EWC", "nxn"), "LP_Analyze80211n()" );

				//11AC analysis
				printf("\nUsing 11AC analysis, iqapiAnalysis11ac\n");
				CheckReturnCode( LP_Analyze80211ac(), "LP_Analyze80211ac()" );

				fprintf(fp_11AC_MIMO, " iPwrErr is %d ; iRun is %d\n", iPwrErr, iRun);
				fprintf(fp_11AC_MIMO,"%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s\n", "packetDetection", "acquisition", "psduCRC", "demodulation", "freqErrorHz", "symClockErrorPpm", "PhaseNoiseDeg_RmsAll", "IQImbal_amplDb", "IQImbal_phaseDeg", "dcLeakageDbc", "rateInfo_bandwidthMhz", "rateInfo_spatialStreams", "rateInfo_spaceTimeStreams", "rateInfo_dataRateMbps", "rxRmsPowerDb", "evmAvgAll");

				//11n and 11ac result
				double packetDetection = LP_GetScalarMeasurement("packetDetection");
				if (0 == packetDetection){
					printf("\nValid Packet Header Detection: %s \n", "FALSE");
				}else{
					printf("\nValid Packet Header Detection: %s \n", "TRUE");
				}

				double acquisition = LP_GetScalarMeasurement("acquisition");
				if (0 == acquisition){
					printf("Valid HT Packet Detection: %s \n", "FALSE");
				}else{
					printf("Valid HT Packet Detection: %s \n", "TRUE");
				}

				double psduCRC = LP_GetScalarMeasurement("psduCRC");
				if (1 == psduCRC){
					printf("PSDU CRC: %s \n", "PASS");
				}else{
					printf("PSDU CRC: %s \n", "FAIL");
					printf("\n\n\nWanning: Analysis has error \n\n\n");
				}

				double demodulation = LP_GetScalarMeasurement("demodulation");
				if (0 == demodulation){
					printf("Streams Demodulated: %s \n\n", "FALSE");
				}else{
					printf("Streams Demodulated: %s \n\n", "TRUE");
				}

				//Not working for 1 stream signal.
				// result = LP_GetScalarMeasurement("isolationDb");
				// printf("isolationDb=%0.2f \n", result);

				double freqErrorHz = LP_GetScalarMeasurement("freqErrorHz");
				printf("Frequency Error: %.2f Hz\n", freqErrorHz);

				double symClockErrorPpm = LP_GetScalarMeasurement("symClockErrorPpm");
				printf("Symbol Clock Error: %.2f ppm\n", symClockErrorPpm);

				double PhaseNoiseDeg_RmsAll = LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll");
				printf("RMS Phase Noise: %.2f deg\n", PhaseNoiseDeg_RmsAll);

				double IQImbal_amplDb = LP_GetScalarMeasurement("IQImbal_amplDb");
				printf("IQ Imbalance Amp: %.2f dB \n", IQImbal_amplDb);

				double IQImbal_phaseDeg = LP_GetScalarMeasurement("IQImbal_phaseDeg");
				printf("IQ Imbalance phase: %0.2f deg \n", IQImbal_phaseDeg);

				double dcLeakageDbc = LP_GetScalarMeasurement("dcLeakageDbc");
				printf("LO leakage: %.2f dBc\n\n", dcLeakageDbc);

				double rateInfo_bandwidthMhz = LP_GetScalarMeasurement("rateInfo_bandwidthMhz");
				printf("Signal Bandwidth: %0.2f MHz\n", rateInfo_bandwidthMhz);

				double rateInfo_spatialStreams = LP_GetScalarMeasurement("rateInfo_spatialStreams");
				printf("Number of Spatial Streams: %1.0f \n", rateInfo_spatialStreams);

				double rateInfo_spaceTimeStreams = LP_GetScalarMeasurement("rateInfo_spaceTimeStreams");
				printf("Number of SpaceTime Streams: %1.0f \n", rateInfo_spaceTimeStreams);

				double numStreams = 1;

				//results seems to be not meaningful
				//result = LP_GetScalarMeasurement("analyzedRange");
				//printf("analyzedRange=%0.2f \n", result);

				/*result = LP_GetScalarMeasurement("htSig1_htLength");
				  printf("htSig1_htLength=%0.2f \n", result);

				  result = LP_GetScalarMeasurement("htSig1_mcsIndex");
				  printf("htSig1_mcsIndex=%0.2f \n", result);

				  result = LP_GetScalarMeasurement("htSig1_bandwidth");
				  printf("htSig1_bandwidth=%0.2f \n", result);

				  result = LP_GetScalarMeasurement("htSig2_advancedCoding");
				  printf("htSig2_advancedCoding=%0.2f \n", result);*/

				double rateInfo_dataRateMbps = LP_GetScalarMeasurement("rateInfo_dataRateMbps");
				printf("Datarate: %0.2f Mbps\n", rateInfo_dataRateMbps);
				if(rateInfo_dataRateMbps<540) //depends on current datarate of the mode files.
				{
					printf("\n\n\nWanning: Analysis has error \n\n\n");
					numAnalysisErr++;
				}

				double powerPreambleDbm = LP_GetScalarMeasurement("powerPreambleDbm", 0);
				printf("powerPreambleDbm at chain 1: %.3f dBm\n", powerPreambleDbm);
				powerPreambleDbm = LP_GetScalarMeasurement("powerPreambleDbm",1);
				printf("powerPreambleDbm at chain 2: %.3f dBm\n", powerPreambleDbm);
				powerPreambleDbm = LP_GetScalarMeasurement("powerPreambleDbm",2);
				printf("powerPreambleDbm at chain 3: %.3f dBm\n", powerPreambleDbm);

				double rxRmsPowerDb = LP_GetScalarMeasurement("rxRmsPowerDb");
				printf("Power_NoGap: %.3f dBm\n", rxRmsPowerDb);
				if (rxRmsPowerDb > maxPWR)
				{
					maxPWR = rxRmsPowerDb;
				}
				if(rxRmsPowerDb < minPWR)
				{
					minPWR = rxRmsPowerDb;
				}
				avgPWR += rxRmsPowerDb;

				double evmAvgAll = LP_GetScalarMeasurement("evmAvgAll");
				printf("EVM Avg All: %.3f dB\n\n", evmAvgAll);
				if (evmAvgAll > maxEVM)
				{
					maxEVM = evmAvgAll;
				}
				if(evmAvgAll < minEVM)
				{
					minEVM = evmAvgAll;
				}
				avgEVM += evmAvgAll; //simple average in dB domain

				fprintf(fp_11AC_MIMO,"%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n", packetDetection, acquisition, psduCRC, demodulation, freqErrorHz, symClockErrorPpm, PhaseNoiseDeg_RmsAll, IQImbal_amplDb, IQImbal_phaseDeg, dcLeakageDbc, rateInfo_bandwidthMhz, rateInfo_spatialStreams, rateInfo_spaceTimeStreams, rateInfo_dataRateMbps, rxRmsPowerDb, evmAvgAll);
				printf("11AC VHT Sig field Info:\n");
				double VHTSigA1Bandwidth = LP_GetScalarMeasurement("VHTSigA1Bandwidth");
				printf("VHTSigA1Bandwidth=%1.0f. \n", VHTSigA1Bandwidth);

				double vhtSigA1Bandwidth = LP_GetScalarMeasurement("vhtSigA1Bandwidth");
				printf("vhtSigA1Bandwidth=%1.0f. \n", vhtSigA1Bandwidth);

				double VHTSigA1Stbc = LP_GetScalarMeasurement("VHTSigA1Stbc");
				printf("VHTSigA1Stbc=%1.0f. Means: ", VHTSigA1Stbc);
				if(0 == VHTSigA1Stbc){
					printf("STBC is not enabled for all streams.\n");
				}else{
					printf("STBC is enabled for all streams.\n");
				}

				double vhtSigA1Stbc = LP_GetScalarMeasurement("vhtSigA1Stbc");
				printf("vhtSigA1Stbc=%1.0f. Means: ", vhtSigA1Stbc);
				if(0 == vhtSigA1Stbc){
					printf("STBC is not enabled for all streams.\n");
				}else{
					printf("STBC is enabled for all streams.\n");
				}

				double VHTSigA2ShortGI = LP_GetScalarMeasurement("VHTSigA2ShortGI");
				printf("VHTSigA2ShortGI=%1.0f\n", VHTSigA2ShortGI);

				double vhtSigA2ShortGI = LP_GetScalarMeasurement("vhtSigA2ShortGI");
				printf("vhtSigA2ShortGI=%1.0f\n", vhtSigA2ShortGI);

				double VHTSigA2AdvancedCoding = LP_GetScalarMeasurement("VHTSigA2AdvancedCoding");
				printf("VHTSigA2AdvancedCoding=%1.0f\n", VHTSigA2AdvancedCoding);

				double vhtSigA2AdvancedCoding = LP_GetScalarMeasurement("vhtSigA2AdvancedCoding");
				printf("vhtSigA2AdvancedCoding=%1.0f\n", vhtSigA2AdvancedCoding);

				double VHTSigA2McsIndex = LP_GetScalarMeasurement("VHTSigA2McsIndex");
				printf("VHTSigA2McsIndex=%1.0f\n", VHTSigA2McsIndex);

				double vhtSigA2McsIndex = LP_GetScalarMeasurement("vhtSigA2McsIndex");
				printf("vhtSigA2McsIndex=%1.0f\n", vhtSigA2McsIndex);

				double VHTSigBFieldCRC = LP_GetScalarMeasurement("VHTSigBFieldCRC");
				printf("VHTSigBFieldCRC=%1.0f\n\n", VHTSigBFieldCRC);

				double vhtSigBFieldCRC = LP_GetScalarMeasurement("vhtSigBFieldCRC");
				printf("vhtSigBFieldCRC=%1.0f\n\n", vhtSigBFieldCRC);

				//			result = LP_GetScalarMeasurement("htSigFieldCRC");
				//			printf("htSigFieldCRC=%0.2f \n", result);

				//int lengthIQ =64;
				//int numberOfStream =3;
				//int numberOfEst=lengthIQ*numberOfStream*numberOfStream;
				//for (int vsaIndex =0;  vsaIndex<3; vsaIndex++)
				//{
				//	printf("\n VSA index %d",vsaIndex);
				//	for (int carrier=0;carrier<lengthIQ;carrier++)
				//	{
				//		printf("\n   carrier index %d \n     stream index: ",carrier);
				//		for (int i=0;i<numberOfStream;i++)
				//		{
				//			int numberOfIndex = (lengthIQ*numberOfStream*vsaIndex)+(numberOfStream*carrier)+i;
				//			printf("%d,",numberOfIndex);
				//		}
				//	}
				//}
				result = ::LP_GetVectorMeasurement("channelEst", bufferReal, bufferImag, MAX_BUFFER_SIZE);
				if (result <= 0){
					printf("Channel Estimation returns an error\n");
				}else{
					//cacluate the spectrum flatness.
					printf("Channel Estimation returns a vector with %3.0f elements\n", result);
					int 	numSubcarrier = 256; //number of Subcarrier for 80MHz
					double  *carrierPwr = NULL;
					int 	centerStart, centerEnd, sideStart, sideEnd;
					//iqPwr  = (double *) malloc (result * sizeof (double));
					carrierPwr = (double *) malloc (numSubcarrier * sizeof (double));
					double 	avgCenterPwr=0.0, avgSidePwr=0.0, avgPwrSpectral=0.0, allPwrSpectral=0.0;
					double 	maxCarrierPwr = -999.9, minCenterCarrierPwr = 99.9, minSideCarrierPwr = 99.9;

					if (20.0 == rateInfo_bandwidthMhz){
						numSubcarrier = 64;
						centerStart   = CARRIER_1;
						centerEnd     = CARRIER_16;
						sideStart     = CARRIER_17;
						sideEnd       = CARRIER_28;
					}else{
						if (40.0 == rateInfo_bandwidthMhz){
							numSubcarrier = 128;
							centerStart   = CARRIER_2;
							centerEnd     = CARRIER_42;
							sideStart     = CARRIER_43;
							sideEnd       = CARRIER_58;
						}else{
							numSubcarrier = 256;
							centerStart   = CARRIER_2;
							centerEnd     = CARRIER_84;
							sideStart     = CARRIER_85;
							sideEnd       = CARRIER_122;
						}
					}
					int numVSA = numMimo;
					if (result != numSubcarrier*numVSA*numStreams){
						printf("Channel estimation results does not match number of subcarriers for %2.0f MHz signal.\n", rateInfo_bandwidthMhz);
					}else{
						int iCarrier=0;
						for (int iNumVsa = 0; iNumVsa<numVSA; iNumVsa++) //VSA based
						{
							allPwrSpectral=0.0;
							//printf("\n Carrier index, real, imagine\n");
							for (int carrier=0;carrier<numSubcarrier;carrier++)
							{
								carrierPwr[carrier]=0;
								for (int iNumStream = 0; iNumStream<numStreams; iNumStream++)
								{
									//int iNumStream = iNumVsa; //only direct mapping result is meaningful
									iCarrier = (int)((numSubcarrier*numStreams*iNumVsa)+(numStreams*carrier)+iNumStream);
									//printf("%d, %0.6f, %0.6f\n",iCarrier,bufferReal[iCarrier],bufferImag[iCarrier]);
									carrierPwr[carrier] = carrierPwr[carrier]+bufferReal[iCarrier]*bufferReal[iCarrier]+ bufferImag[iCarrier]*bufferImag[iCarrier];
								}
								//for (int iNumStream = 0; iNumStream<numStreams; iNumStream++)
								//{
								//	iCarrier = (int)((numSubcarrier*numStreams*iNumVsa)+(numStreams*carrier)+iNumStream);
								//	carrierPwr[carrier] = carrierPwr[carrier]+bufferReal[iCarrier]*bufferReal[iCarrier]+ bufferImag[iCarrier]*bufferImag[iCarrier];
								//}

							}

							// average power in the center
							for ( int i=centerStart;i<=centerEnd;i++)
							{
								avgCenterPwr = avgCenterPwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
							}
							// average power on side lobe.
							for (int i=sideStart;i<=sideEnd;i++)
							{
								avgSidePwr = avgSidePwr + carrierPwr[i] + carrierPwr[numSubcarrier-i];
							}

							avgPwrSpectral = avgCenterPwr + avgSidePwr;

							avgCenterPwr = avgCenterPwr/((centerEnd-centerStart+1)*2);
							if (0!=avgCenterPwr){
								avgCenterPwr = 10.0 * log10 (avgCenterPwr);
							}else{
								avgCenterPwr= 0;
							}
							allPwrSpectral=avgPwrSpectral;
							avgPwrSpectral = avgPwrSpectral/((sideEnd-centerStart+1)*2);
							if (0!=avgPwrSpectral){
								avgPwrSpectral = 10.0 * log10 (avgPwrSpectral);
							}else{
								avgPwrSpectral = 0;
							}

							for (int carrier=0;carrier<numSubcarrier;carrier++)
							{
								if (0!=carrierPwr[carrier]){
									carrierPwr[carrier] = 10.0 * log10 (carrierPwr[carrier]);
									if ( (carrier>=centerStart && carrier <=centerEnd) || (carrier>=numSubcarrier-centerEnd && carrier <= numSubcarrier-centerStart) ){
										if(carrierPwr[carrier]< minCenterCarrierPwr){
											minCenterCarrierPwr = carrierPwr[carrier];
										}
										if (carrierPwr[carrier]>maxCarrierPwr)
										{
											maxCarrierPwr = carrierPwr[carrier];
										}
									}
									if ((carrier>=sideStart && carrier <=sideEnd) || (carrier>=numSubcarrier-sideEnd && carrier <=numSubcarrier-sideStart) ){
										if(carrierPwr[carrier]< minSideCarrierPwr){
											minSideCarrierPwr = carrierPwr[carrier];
										}
										if (carrierPwr[carrier]>maxCarrierPwr)
										{
											maxCarrierPwr = carrierPwr[carrier];
										}
									}
								}else{
									carrierPwr[carrier] = 0;
								}
							}
							printf("The spectral flatness results of VSA %d are:\n", iNumVsa);
							printf("   The maximum deviation is %0.2f dB\n", maxCarrierPwr-avgCenterPwr);
							printf("   The minimum deviation are %0.2f dB in the center, %0.2f dB in the far side\n", minCenterCarrierPwr-avgCenterPwr, minSideCarrierPwr-avgCenterPwr);
							printf("   The LO Leakage is %0.2f dBc \n", carrierPwr[0] - avgPwrSpectral - 10 * log10((double)((sideEnd-centerStart+1)*2)) );
							/*allPwrSpectral = 10.0 * log10 (allPwrSpectral);
							  printf("   The LO Leakage is %0.2f dBc \n", carrierPwr[0] - allPwrSpectral );*/
						}
						result = LP_GetScalarMeasurement("dcLeakageDbc");
						printf("dcLeakageDbc: %0.2f dBc\n", result);
					}
					free( carrierPwr );
				}

				//Perform Wideband Capture for VHT80
				captureTime = 1000e-6;

				int vht80MaskMode = IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
				CheckReturnCode( LP_SetVsa(FREQ_HZ, (dutPowerLevel+peakToAverageRatio-cableLoss), PORT_LEFT, 0, -25, 10e-6), "LP_SetVsa()" ); //port was 1, for instrument driver only?
				CheckReturnCode( LP_VsaDataCapture(captureTime, 13, samplingRate, vht80MaskMode), "LP_VsaDataCapture()" );
				CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

				double bufferRealX[MAX_BUFFER_SIZE], bufferImagX[MAX_BUFFER_SIZE];
				double bufferRealY[MAX_BUFFER_SIZE], bufferImagY[MAX_BUFFER_SIZE];
				int    vsaAnalysisIndex = 0;

				while(vsaAnalysisIndex < 2)
				{
					//Step1: Set Analysis Parameter "vsaNum"
					//if(vht80MaskMode == 2)
					//{
					//	//AnalyzeVHT80Mask
					//	CheckReturnCode(LP_SetAnalysisParameterInteger("AnalyzeVHT80Mask", "vsaNum", vsaAnalysisIndex+1), "LP_SetAnalysisParameterInteger()" );
					//}
					//else
					//{
					//	//not implemented;
					//	CheckReturnCode(LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", vsaAnalysisIndex+1), "LP_SetAnalysisParameterInteger()" );
					//}

					//Step2: Perform Analysis
					if (vht80MaskMode == IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE)
						// VHT80Mask mode
					{
						printf("\n Analyzing VHT80MHz Wideband Mask\n");
						CheckReturnCode( LP_AnalyzeVHT80Mask(), "LP_AnalyzeVHT80Mask()" );
					}
					else // HT20
					{
						printf("\n Analyzing normal Mask\n");
						CheckReturnCode( LP_AnalyzeFFT(), "LP_AnalyzeFFT()" );
					}

					int bufferSizeX = ::LP_GetVectorMeasurement("x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE);
					int bufferSizeY = ::LP_GetVectorMeasurement("y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE);



					/*	int length = 10000;
						double *y = new double[length];
						double *x = new double[length];
						int length1 = LP_GetVectorMeasurement("y", y, y, length);
						int length2 = LP_GetVectorMeasurement("x", x, x, length);*/

					/// Test plot
					//int       length = 360, length1 = 360;
					//double *x = (double *) malloc((sizeof(double)*length));
					//double *y = (double *) malloc((sizeof(double)*length));

					//double *x1 = (double *) malloc((sizeof(double)*length1));
					//double *y1 = (double *) malloc((sizeof(double)*length1));

					//for ( int iCount=0; iCount < length; iCount++ )
					//{
					//	x[iCount] = iCount;
					//	y[iCount] = sin(iCount * 3.14 / 180);
					//}

					//for ( int iCount=0; iCount < length1; iCount++ )
					//{
					//	x1[iCount] = iCount;
					//	y1[iCount] = cos(iCount * 3.14 / 180);

					//}

					//for ( int iCount=0; iCount < 1; iCount++ )
					//{
					//	LP_Plot(1, x, y, length, "r*:", "Sine", "X", "Y", 0);

					//	LP_Plot(1, x1, y1, length1, "b*:", "Cosine", "X", "Y", 1);
					//}

					//free (x);
					//free (y);
					//free (x1);
					//free (y1);



					if(bufferSizeX == bufferSizeY)
					{
						CheckReturnCode( LP_Plot(1, bufferRealX, bufferRealY, bufferSizeX, ".-", "FFT", "Freq", "Power", 1), "LP_Plot()");
						FILE 	*fp_mask = NULL;
						char 	*outFileName = "11AC_QA_Mask.csv";
						char 	buffer [MAX_BUFFER_SIZE] = {'\0'};
						string	outputString;
						fopen_s(&fp_mask,outFileName,"w+");
						fprintf(fp_mask,"%s, %s\n", "bufferRealX", "bufferRealY");
						for(int i = 0 ; i < MAX_BUFFER_SIZE ; i++){
							//fprintf(fp_mask,"%f, %f\n", bufferRealX[i], bufferRealY[i]);
							sprintf_s(buffer,sizeof(buffer),"%f, %f\n", bufferRealX[i], bufferRealY[i]);
							outputString += buffer;
						}
						fprintf(fp_mask, "%s", outputString.c_str());
						fclose(fp_mask);
					}
					else
					{
						throw "Length of X and Y do not match!";
					}

					//            if(bufferRealX[0] < -129e6 )
					//{
					//	printf("\n VHT80MHz Wideband Mask have 260MHz results \n");
					//}
					//else
					//{
					//	if (bufferRealX[0] < -109e6)
					//	{
					//		printf("\n VHT80MHz Wideband Mask have 109MHz results \n");
					//	}
					//	else
					//		printf("\n VHT80MHz Wideband Mask fails\n");
					//}
					printf("\n VHT80MHz Wideband mask starts at %4.0f MHz results \n", bufferRealX[0]/(1e6) );
					vsaAnalysisIndex ++;
				}

				//----------------//
				//  Turn off VSG  //
				//----------------//
				//CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );
				printf("\n");

				//printf("Press 'X' key to exit or any key to repeat the test..................\n");

				//char ch = toupper(_getch());
				//if (ch=='X' || ch==ASCII_ESC)
				//    break;
			}
			printf("Statistics of %d run:\n",numRun);
			avgPWRDynamicRange[iPwrErr+dynamicRange]=avgPWR/numRun;
			maxPWRDynamicRange[iPwrErr+dynamicRange]=maxPWR;
			minPWRDynamicRange[iPwrErr+dynamicRange]=minPWR;
			avgEVMDynamicRange[iPwrErr+dynamicRange]=avgEVM/numRun;
			maxEVMDynamicRange[iPwrErr+dynamicRange]=maxEVM;
			minEVMDynamicRange[iPwrErr+dynamicRange]=minEVM;
			printf("Power_NoGap\n	Avg: %.2f dBm	Max: %.2f dBm	Min: %.2f dBm\n", avgPWRDynamicRange[iPwrErr+dynamicRange],maxPWR,minPWR);
			printf("EVM\n	Avg: %.2f dB	Max: %.2f dB	Min: %.2f dB\n\n", avgEVMDynamicRange[iPwrErr+dynamicRange],maxEVM,minEVM);

			//printf("Power_NoGap\n	Avg: %.2f dBm	Max: %.2f dBm	Min: %.2f dBm\n", avgPWR/numRun,maxPWR,minPWR);
			//printf("EVM\n	Avg: %.2f dB	Max: %.2f dB	Min: %.2f dB\n\n", avgEVM/numRun,maxEVM,minEVM);
			if (numAnalysisErr>0)
			{
				printf("\n\n\nWanning: Analysis has been wrong for %d times out of %d run \n\n\n", numAnalysisErr, numRun);
			}
		}

		fclose(fp_11AC_MIMO);

		printf("Statistics of %d run:\n",numRun);
		printf("\n\nAmpLevelOffSet, Power_NoGap Average (dBm), Maximum(dBm), Minimum(dBm)\n");
		for (int iPwrErr=0-dynamicRange; iPwrErr<=dynamicRange+6; iPwrErr++ )
		{
			printf("%d, %.2f, %.2f, %.2f \n", iPwrErr, avgPWRDynamicRange[iPwrErr+dynamicRange],maxPWRDynamicRange[iPwrErr+dynamicRange],minPWRDynamicRange[iPwrErr+dynamicRange]);
		}
		printf("\n\nAmpLevelOffSet, EVM Average (dB), Maximum(dB), Minimum(dB)\n");
		for (int iPwrErr=0-dynamicRange; iPwrErr<=dynamicRange+6; iPwrErr++ )
		{
			printf("%d, %.2f, %.2f, %.2f \n", iPwrErr, avgEVMDynamicRange[iPwrErr+dynamicRange],maxEVMDynamicRange[iPwrErr+dynamicRange],minEVMDynamicRange[iPwrErr+dynamicRange]);
		}

	}


	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
		//        LpGetSysError(errMsg);
		//printf("ERROR: %s\n", errMsg);
	}
	catch(...)
	{
		printf("ERROR!");
		//       LpGetSysError(errMsg);
		//printf("ERROR: %s\n", errMsg);
	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
	ReadLogFiles();
}


//tyu;2012-07-11; QA BT
void QA_BT(void)
{
	char   buffer[MAX_BUFFER_SIZE];
	double dutPowerLevel = -20;		// Estimated RMS power level at IQTester
	double cableLoss = 1;           //dB of path loss
	double maxCaptureTime=0.0, minCaptureTime=1e6, maxAnalysisTime=0.0, minAnalysisTime=1e6;
	double freqHz;

	//----------------------------//
	//   Output File Operation    //
	//----------------------------//
	FILE *fp_power, *fp_analysis1;
	char outFileName[MAX_FILENAME_SIZE];
	char outFileName1[MAX_FILENAME_SIZE];



	sprintf_s(outFileName1,MAX_FILENAME_SIZE,".\\Log\\QA_BT_Analysis1_Run_%d.csv", Tag_runTime);
	sprintf_s(outFileName,MAX_FILENAME_SIZE,".\\Log\\QA_BT_Power_Run_%d.csv", Tag_runTime);



	try
	{
		fopen_s(&fp_power,outFileName,"w+");
		fopen_s(&fp_analysis1,outFileName1,"w+");


		//----------------------------//
		//   Initialize the IQTester  //
		//----------------------------//
		CheckReturnCode( LP_Init(ciTesterType, ciTesterControlMode), "LP_Init()" );

		if (ciTesterType == IQTYPE_XEL)
		{
			if (ciTesterControlMode == 0)  // IQapi_scpi
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_QA_BT_Run_%d.csv",Tag_runTime);
			}
			else   //SCPI
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_QA_BT_Run_%d.csv",Tag_runTime);
			}
		}
		else
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQ2010_QA_BT_Run_%d.csv",Tag_runTime);
		}


		CheckReturnCode( LP_InitTester(g_IP_addr), "LP_InitTester()" );


		if (LP_GetVersion(buffer, MAX_BUFFER_SIZE)==true)
			printf("\n%s\n\n", buffer);
		::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"\n%s\n\n",buffer);


		double maxPWR=-999.9, minPWR=999.9,avgPwrSpectral=0;
		double maxEVM=-999.9, minEVM=999.9,avgEVM=0;
		string modFileName = ""; // saveCapturedName="";
		string tempString = "";
		char saveCapturedName[MAX_BUFFER_SIZE];

		for( int freq_id = 1; freq_id <= 3; freq_id++)    //3 channel: LO,MI,HI
			for (int signal_id = 1; signal_id <= 10; signal_id++)   //10 data rate

			{

				if (1==freq_id)
					freqHz = 2402e6;  //BT channel 1
				else if (2==freq_id)
					freqHz = 2441e6;      //channel 39
				else  freqHz = 2480e6;		//channel 78

				if (1==signal_id)  //1DH1
					tempString = "1DH1_000088C0FFEE";
				else if (2==signal_id)  //1DH3
					tempString = "1DH3_000088C0FFEE";
				else if (3==signal_id)  //1DH5
					tempString = "1DH5_000088C0FFEE";
				else if (4==signal_id)  //2DH1
					tempString = "2DH1_000088C0FFEE";
				else if (5==signal_id)  //2DH3
					tempString = "2DH3_000088C0FFEE";
				else if (6==signal_id)  //2DH5
					tempString = "2DH5_000088C0FFEE";
				else if (7==signal_id)  //3DH1
					tempString = "3DH1_000088C0FFEE";
				else if (8==signal_id)  //3DH3
					tempString = "3DH3_000088C0FFEE";
				else if (9==signal_id)//3DH5
					tempString = "3DH5_000088C0FFEE";
				else // add LE
					tempString = "BT_1LE_prbs9_Fs80M";
				//-------------//
				//  Setup VSG  //
				//-------------//

				CheckReturnCode( LP_SetVsg(freqHz, dutPowerLevel, PORT_RIGHT), "LP_SetVsg()" );

				//--------------------//
				// Load waveform file //
				//--------------------//
				//modFileName="../mod/";

				//modFileName+=tempString;

				//Luke 2012-8-02: seperate mod and iqvsg folder
				if (ciTesterType == IQTYPE_XEL)
				{
					modFileName="../../iqvsg/";
					modFileName+=tempString;
					modFileName+=".iqvsg";
				}
				else		//IQ2010/view/flex
				{
					modFileName="../../mod/";
					modFileName+=tempString;
					modFileName+=".mod";
				}



				printf("\n Loading mode file %s\n", modFileName.c_str());
				::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"\n Loading mode file %s\n", modFileName.c_str());
				CheckReturnCode( LP_SetVsgModulation((char*)modFileName.c_str()), "LP_SetVsgModulation()" );

				//---------------//
				//  Turn on VSG  //
				//---------------//
				CheckReturnCode( LP_EnableVsgRF(1), "LP_EnableVsgRF()" );

				//------------------------------------------//
				//   Send packet, continuous transmission   //
				//tyu; 2012-07-07;
				//------------------------------------------//
				int frameCnt = 0;	// number of frame to send, 0 means continuous transmission.
				int err =0;
				CheckReturnCode( LP_SetFrameCnt(1000), "LP_SetFrameCnt()" );
				while (true)
				{
					err = LP_TxDone();
					if ( err != 0)
						continue;
					else break;
				}
				CheckReturnCode( LP_SetFrameCnt(0), "LP_SetFrameCnt()" );

				//----------------//
				//   Setup VSA    //
				//----------------//
				int peakToAverageRatio = 10;
				//  double samplingRate = 160e6;      //tyu;2012-7-6; Timer log
				double samplingRate;       //tyu;2012-7-6; Timer log
				double captureTime;

				//tyu;2012-7-6; Timer log
				if (ciTesterType == IQTYPE_XEL)
				{
					samplingRate = 160e6;

				}
				else
				{
					samplingRate = 80e6;
				}
				//End

				CheckReturnCode(LP_SetVsaBluetooth(  freqHz,
							dutPowerLevel+peakToAverageRatio-cableLoss,
							PORT_LEFT,
							-25,
							10e-6), "LP_SetVsaBluetooth()");

				//----------------------//
				//  Perform VSA capture //
				//----------------------//
				for (int trigger_type = 13; trigger_type <=13; trigger_type+=12 )
					//		for (int capture_type = 0; capture_type <=1; capture_type+=1 )

				{

					lp_time_t starttime, stoptime;
					if (13 == trigger_type)
					{
						if (1==signal_id || 4==signal_id ||7==signal_id)    //DH1
							captureTime = 650e-6;
						else if (2==signal_id || 5==signal_id ||8==signal_id)    //DH3
							captureTime = 2000e-6;
						else //DH5
							captureTime = 3250e-6;

					}
					else
						captureTime = 5e-3;;

					fprintf(fp_power,"\n=====%.2f====%s ====",freqHz,tempString.c_str());
					fprintf(fp_power,"=======Trigger type %d=================\n",trigger_type);
					fprintf(fp_power,"=====Valid, P_av_each_burst_dBm, P_av_each_burst, P_pk_each_burst_dBm, P_pk_each_burst, P_av_all_dBm,\
							P_av_all, P_peak_all_dBm, P_peak_all, P_av_no_gap_all_dBm, P_av_no_gap_all, start_sec, stop_sec=======================\n");

					fprintf(fp_analysis1,"\n=====%.2f====%s ====",freqHz,tempString.c_str());
					fprintf(fp_analysis1,"=======Trigger type %d=================\n",trigger_type);
					fprintf(fp_analysis1,"=====Data Rate(Mbps),Bandwidth_20dB, P_pk_each_burst_dBm, P_av_each_burst_dBm, P_av_no_gap_all_dBm, deltaF2AvAccess(kHz), deltaF2MaxAccess(kHz), \
							deltaF2Average(kHz), deltaF2Max(kHz), deltaF1Average(kHz), EdrOmegaI(kHz), EdrExtremeOmegaI0(kHz),\
							EdrExtremeOmega0(kHz),EdrEVMpk(%),EdrEVMAv(%),EdrPowDiffdB,EdrFreqExtremeEdronly(kHz),freq_deviation(kHz),freq_deviationpktopk(kHz),\
							freq_est(kHz),freq_estHeader(kHz),payloadErrors,EdrPowDiffdB,EdrprobEVM99pass,\
							start_sec, stop_sec=======================\n");

					int iRun=0, numRun=5;
					while(iRun<numRun)
					{
						iRun++;
						printf("_________________________________________________\n");
						printf("Test %d\n", iRun);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"_________________________________________________\n");
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Test %d\n", iRun);

						GetTime(starttime);

						//CheckReturnCode( LP_VsaDataCapture(captureTime, trigger_type, samplingRate, capture_type), "LP_VsaDataCapture()" ); //trigger type was 2, only for instrument driver? in IQapi, it is for external trigger?
						CheckReturnCode( LP_VsaDataCapture(captureTime, trigger_type, samplingRate), "LP_VsaDataCapture()" );
						GetTime(stoptime);

						int elapsed_ms = GetElapsedMSec(starttime, stoptime);

						if (13 == trigger_type)
						{
							printf("************ Signal trigger Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
							::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"************ Signal trigger Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
						}
						else
						{
							printf("************ Free Run Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
							::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"************ Free Run Elapsed time for LP_VsaDataCapture(): %d ms ************\n", elapsed_ms);
						}


						if (elapsed_ms<minCaptureTime)
							minCaptureTime=elapsed_ms;
						if(elapsed_ms>maxCaptureTime)
							maxCaptureTime=elapsed_ms;
						//---------------------------//
						//  Save sig file for debug  //
						//---------------------------//

						//tyu; 2012-7-6; Seclet different format for IQ2010 and IQxel
						//sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%s-%s-%d.iqvsa", "FastTrack_Capture", tempString.c_str(),iRun);
						if (ciTesterType == IQTYPE_XEL)
						{
							sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%f-Trig%d-%s-%s-%d.iqvsa" ,freqHz/1e6,trigger_type,"FastTrack_Capture", tempString.c_str(),iRun);
						}
						else		//IQ2010/view/flex
						{
							sprintf_s(saveCapturedName, MAX_BUFFER_SIZE, "./log/%f-Trig%d-%s-%s-%d.sig",freqHz/1e6,trigger_type, "FastTrack_Capture",  tempString.c_str(),iRun);
						}

						//end

						CheckReturnCode( LP_SaveVsaSignalFile(saveCapturedName), "LP_SaveVsaSignalFile()" );

						//	LP_LoadVsaSignalFile("D:\\Log_brcm4334\\BT_TX_EDR_PRBS9_SaveAlways_2402_2DH1.sig");

						//-------------------------//
						//  Retrieve Test Results  //
						//-------------------------//
						double result,result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13,
						       result14,result15,result16,result17,result18,result19,result20,result21,result22,result23,result24,result25 ;
						double resultAcp[11];

						if( ciTesterControlMode == 0 )//IQAPI
							CheckReturnCode( LP_AnalyzePower(), "LP_AnalyzePower()" );
						else if( ciTesterControlMode == 1 )//SCPI
							CheckReturnCode( LP_AnalyzePower(3.2e-6, 15.0), "LP_AnalyzePower()" );


						result1 = LP_GetScalarMeasurement("valid"); //result not right yet.
						printf("valid: %.0f\n", result1);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"valid: %.0f\n", result1);

						result2 = LP_GetScalarMeasurement("P_av_each_burst_dBm");
						printf("P_av_each_burst_dBm: %.2f dBm\n", result2);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"P_av_each_burst_dBm: %.2f dBm\n", result2);

						result3 = LP_GetScalarMeasurement("P_av_each_burst");
						printf("P_av_each_burst: %.6f mw\n", result3);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"P_av_each_burst: %.6f mw\n", result3);

						result4 = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
						printf("P_pk_each_burst_dBm: %.2f dBm\n", result4);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"P_pk_each_burst_dBm: %.2f dBm\n", result4);

						result5 = LP_GetScalarMeasurement("P_pk_each_burst");
						printf("P_pk_each_burst: %.6f mw\n", result5);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"P_pk_each_burst: %.6f mw\n", result5);

						result6 = LP_GetScalarMeasurement("P_av_all_dBm");
						printf("P_av_all_dBm: %.2f dBm\n", result6);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"P_av_all_dBm: %.2f dBm\n", result6);

						result7 = LP_GetScalarMeasurement("P_av_all");
						printf("P_av_all: %.6f mw\n", result7);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"P_av_all: %.6f mw\n", result7);

						result8 = LP_GetScalarMeasurement("P_peak_all_dBm");
						printf("P_peak_all_dBm: %.2f dBm\n", result8);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"P_peak_all_dBm: %.2f dBm\n", result8);

						result9= LP_GetScalarMeasurement("P_peak_all");
						printf("P_peak_all: %.6f mw\n", result9);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"P_peak_all: %.6f mw\n", result9);

						result10 = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
						printf("P_av_no_gap_all_dBm: %.2f dBm\n", result10);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"P_av_no_gap_all_dBm: %.2f dBm\n", result10);

						result11 = LP_GetScalarMeasurement("P_av_no_gap_all");
						printf("P_av_no_gap_all: %.6f mw\n", result11);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"P_av_no_gap_all: %.6f mw\n", result11);

						result12 = LP_GetScalarMeasurement("start_sec");
						printf("start_sec: %.6f sec\n", result12);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"start_sec: %.6f sec\n", result12);

						result13 = LP_GetScalarMeasurement("stop_sec");
						printf("stop_sec: %.6f sec\n", result13);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"stop_sec: %.6f sec\n", result13);

						fprintf(fp_power,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.8f,%.8f\n",result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,result13);
						//BT analysis

						printf("\nUsing BT analysis, analysisBluetooth\n");
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"\nUsing BT analysis, analysisBluetooth\n");

						lp_time_t starttime4, stoptime4;

						/*   for (int analysis_option_combination =1; analysis_option_combination<=6;analysis_option_combination++)
						     {
						     if (1==analysis_option_combination)
						     {
						     GetTime(starttime4);
						     CheckReturnCode(LP_AnalyzeBluetooth(0,"PowerOnly"));
						     }
						     else if (2==analysis_option_combination)
						     {
						     GetTime(starttime4);
						     CheckReturnCode(LP_AnalyzeBluetooth(0,"20dbBandwidthOnly"));

						     }
						     else if (3==analysis_option_combination)
						     {
						     GetTime(starttime4);
						     CheckReturnCode(LP_AnalyzeBluetooth(0,"PowerandFreq"));

						     }
						     else if (4==analysis_option_combination)
						     {
						     GetTime(starttime4);
						     CheckReturnCode(LP_AnalyzeBluetooth(0,"All"));

						     }
						     else if (5==analysis_option_combination)
						     {
						     GetTime(starttime4);
						     CheckReturnCode(LP_AnalyzeBluetooth(0,"ACP"));

						     }
						     else
						     {
						     GetTime(starttime4);
						     CheckReturnCode(LP_AnalyzeBluetooth(0,"AllPlus"));

						     }
						     }*/

						GetTime(starttime4);
						CheckReturnCode(LP_AnalyzeBluetooth(0,"AllPlus"),"LP_AnalyzeBluetooth()");

						GetTime(stoptime4);

						int elapsed_ms4 = GetElapsedMSec(starttime4, stoptime4);
						printf("************ Elapsed time for LP_AnalyzeBluetooth(0,\"AllPlus\")ms ************\n");
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"************ Elapsed time for LP_AnalyzeBluetooth(0,\"AllPlus\")ms ************\n");

						if (elapsed_ms4<minAnalysisTime)
							minAnalysisTime=elapsed_ms4;

						if(elapsed_ms4>maxAnalysisTime)
							maxAnalysisTime=elapsed_ms4;

						result = LP_GetScalarMeasurement("dataRateDetect");
						printf("Data Rate: %.2f Mbps\n", result);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Data Rate: %.2f Mbps\n", result);

						result23= LP_GetScalarMeasurement("bandwidth20dB",0)/1e6;
						printf("Bandwidth 20dB: %.2f MHz%\n", result23);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Bandwidth 20dB: %.2f MHz%\n", result23);

						if (ciTesterType == IQTYPE_2010)
						{
							result1 = LP_GetScalarMeasurement("P_pk_each_burst_dBm");
							printf("Peak Power: %.2f dBm\n", result1);
							::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Peak Power: %.2f dBm\n", result1);

							result2 = LP_GetScalarMeasurement("P_av_each_burst_dBm");
							printf("Avg. Power(all): %.2f dBm\n", result2);
							::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Avg. Power(all): %.2f dBm\n", result2);

							result3 = LP_GetScalarMeasurement("P_av_no_gap_all_dBm");
							printf("Avg. Power(no gap): %.2f dBm\n", result3);
							::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Avg. Power(no gap): %.2f dBm\n", result3);
						}
						else   // IQXEL
						{
							result1 = 10*log10(LP_GetScalarMeasurement("P_pk_each_burst"));
							printf("Peak Power: %.2f dBm\n", result1);
							::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Peak Power: %.2f dBm\n", result1);

							result3 =  10*log10(LP_GetScalarMeasurement("P_av_each_burst"));
							printf("Avg. Power(burst): %.2f dBm\n", result3);
							::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Avg. Power(burst): %.2f dBm\n", result3);
						}


						if (result3 > maxPWR)
						{
							maxPWR=result3;
						}
						if(result3<minPWR)
						{
							minPWR=result3;
						}
						avgPwrSpectral+=result3;

						result4 = LP_GetScalarMeasurement("deltaF2AvAccess")/1000;
						printf("Delta F2 Avg Access: %.2f kHz\n", result4);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Delta F2 Avg Access: %.2f kHz\n", result4);

						result5 = LP_GetScalarMeasurement("deltaF2MaxAccess")/1000;
						printf("Delta F2 Max Access: %.2f kHz\n", result5);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Delta F2 Max Access: %.2f kHz\n", result5);

						result6 = LP_GetScalarMeasurement("deltaF2Average")/1000;
						printf("Delta F2 Avg: %.2f kHz\n", result6);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Delta F2 Avg: %.2f kHz\n", result6);

						result7 = LP_GetScalarMeasurement("deltaF2Max")/1000;
						printf("Delta F2 Max: %.2f kHz\n", result7);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Delta F2 Max: %.2f kHz\n", result7);

						result8 = LP_GetScalarMeasurement("deltaF1Average")/1000;
						printf("Delta F1 Avg: %.2f kHz\n", result8);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Delta F1 Avg: %.2f kHz\n", result8);

						result9 = LP_GetScalarMeasurement("EdrOmegaI",0) / 1000;
						printf("Omega_i: %.2f kHz\n", result9);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Omega_i: %.2f kHz\n", result9);

						result10 = LP_GetScalarMeasurement("EdrExtremeOmegaI0",0) / 1000;
						printf("Max(Omega_io): %.2f kHz\n", result10);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Max(Omega_io): %.2f kHz\n", result10);

						result11= LP_GetScalarMeasurement("EdrExtremeOmega0",0) / 1000;
						printf("Max(Omega_o): %.2f kHz\n", result11);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Max(Omega_o): %.2f kHz\n", result11);

						result12= LP_GetScalarMeasurement("EdrEVMpk",0)*100;
						printf("EDR DEVM Peak.: %.2f %\n", result12);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"EDR DEVM Peak.: %.2f %\n", result12);

						result13= LP_GetScalarMeasurement("EdrEVMAv",0)*100;
						printf("EDR DEVM Avg.: %.2f %\n", result13);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"EDR DEVM Avg.: %.2f %\n", result13);

						/*	   if (result13 > maxEVM)
							   {
							   maxEVM=result11;
							   }

							   if(result13<minEVM)
							   {
							   minEVM=result13;
							   }
							   avgEVM+=result13;*/

						result14= LP_GetScalarMeasurement("EdrPowDiffdB",0);
						printf("EDR Diff. Power: %.2f dB\n", result14);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"EDR Diff. Power: %.2f dB\n", result14);

						result15= LP_GetScalarMeasurement("EdrFreqExtremeEdronly",0)/1000;
						printf("EDR Max.Freq.Var: %.2f kHz\n", result15);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"EDR Max.Freq.Var: %.2f kHz\n", result15);

						result16= LP_GetScalarMeasurement("freq_deviation",0)/1000;
						printf("Freq Deviation: %.2f kHz\n", result16);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Freq Deviation: %.2f kHz\n", result16);

						result17= LP_GetScalarMeasurement("freq_deviationpktopk",0)/1000;
						printf("Freq Deviation Peak to Peak: %.2f kHz\n", result17);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Freq Deviation Peak to Peak: %.2f kHz\n", result17);

						result18= LP_GetScalarMeasurement("freq_est",0)/1000;
						printf("Freq Est: %.2f kHz\n", result18);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Freq Est: %.2f kHz\n", result18);

						result19= LP_GetScalarMeasurement("freq_estHeader",0)/1000;
						printf("Freq Est Header: %.2f kHz\n", result19);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Freq Est Header: %.2f kHz\n", result19);

						result20= LP_GetScalarMeasurement("payloadErrors",0);
						printf("Payload errors: %.2f\n", result20);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Payload errors: %.2f\n", result20);

						result21= LP_GetScalarMeasurement("EdrPowDiffdB",0);
						printf("EDR Diff, Power: %.2f dB \n", result21);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"EDR Diff, Power: %.2f dB \n", result21);

						result22= LP_GetScalarMeasurement("EdrprobEVM99pass",0)*100;
						printf("EDR Prob EVM 99 Pass: %.2f %\n", result22);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"EDR Prob EVM 99 Pass: %.2f %\n", result22);

						if ( 1==LP_GetScalarMeasurement("acpErrValid",0) )
						{
							for (int i=0;i<=10;i++)
							{
								if (signal_id <4)
								{
									// maxPowerAcpDbm
									resultAcp[i] = LP_GetScalarMeasurement("maxPowerAcpDbm", i);
								}
								else
									resultAcp[i] = LP_GetScalarMeasurement("maxPowerEdrDbm", i);

							}
						}
						printf("maxPowerAcpDbm: %.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n",
								resultAcp[0],resultAcp[1],resultAcp[2],resultAcp[3],resultAcp[4],resultAcp[5],resultAcp[6],
								resultAcp[7],resultAcp[8],resultAcp[9],resultAcp[10]);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"maxPowerAcpDbm: %.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n",
								resultAcp[0],resultAcp[1],resultAcp[2],resultAcp[3],resultAcp[4],resultAcp[5],resultAcp[6],
								resultAcp[7],resultAcp[8],resultAcp[9],resultAcp[10]);

						result24 = LP_GetScalarMeasurement("start_sec");
						printf("start_sec: %.6f sec\n", result24);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"start_sec: %.6f sec\n", result24);

						result25 = LP_GetScalarMeasurement("stop_sec");
						printf("stop_sec: %.6f sec\n", result25);
						::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"stop_sec: %.6f sec\n", result25);


						//Save to log file
						fprintf(fp_analysis1,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f,%.2f,%.2f,%.2f,\
								%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n",
								result, result23,result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11,result12,
								result13,result14,result15,result16,result17,result18,result19,result20,result21,result22,result24,result25);
						fprintf(fp_analysis1,"ACP,%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n",
								resultAcp[0],resultAcp[1],resultAcp[2],resultAcp[3],resultAcp[4],resultAcp[5],resultAcp[6],
								resultAcp[7],resultAcp[8],resultAcp[9],resultAcp[10]);

						//	if(1== analysis_option_combination)
						//	fprintf(fp_analysis1,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %s, %.2f, %.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result_char1,result9,result10,result11,result12,result13);

						//	if(2== analysis_option_combination)
						//	fprintf(fp_analysis2,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %s, %.2f, %.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result_char1,result9,result10,result11,result12,result13);

						//	if(3== analysis_option_combination)
						//	fprintf(fp_analysis3,"%.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f, %.2f, %s, %.2f, %.2f,%.2f,%.2f,%.2f\n",result1,result2,result3,result4,result5,result6,result7,result8,result_char1,result9,result10,result11,result12,result13);


					}

					printf("Statistics of %d run:\n",numRun);
					::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Statistics of %d run:\n",numRun);
					printf("Power_NoGap\nAvg: %.2f dBm\tMax: %.2f dBm\tMin: %.2f dBm\n", avgPwrSpectral/numRun,maxPWR,minPWR);
					::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Power_NoGap\nAvg: %.2f dBm\tMax: %.2f dBm\tMin: %.2f dBm\n", avgPwrSpectral/numRun,maxPWR,minPWR);
					printf("EVM\nAvg: %.2f dB\tMax: %.2f dB\tMin: %.2f dB\n", avgEVM/numRun,maxEVM,minEVM);
					printf("Capture time: \tMax: %.0f ms\tMin: %.0f ms\n", maxCaptureTime,minCaptureTime);
					::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Capture time: \tMax: %.0f ms\tMin: %.0f ms\n", maxCaptureTime,minCaptureTime);
					printf("Analysis time: \tMax: %.0f ms\tMin: %.0f ms\n", maxAnalysisTime,minAnalysisTime);
					::LOGGER_Write(g_logger_id,LOGGER_INFORMATION,"Analysis time: \tMax: %.0f ms\tMin: %.0f ms\n", maxAnalysisTime,minAnalysisTime);

					//----------------//
					//  Turn off VSG  //
					//----------------//

					printf("\n");

				} //analysis combination for-loop //end of while
			}  //VSA capture for-loop&trigger for-loop
		CheckReturnCode( LP_EnableVsgRF(0), "LP_EnableVsgRF()" );


	} //freq and vsg file for-loop
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");

	}

	CheckReturnCode( LP_Term(), "LP_Term()" );
	fclose(fp_power);
	fclose( fp_analysis1 );
	ReadLogFiles();
}



//tyu;2012-07-16; QA Dualhead
void QA_Dualhead()
{
	int err = 0;
	char buffer[MAX_BUFFER_SIZE] = {'\0'};

	try
	{
		CheckReturnCode( LP_Init(ciTesterType, ciTesterControlMode), "LP_Init()" );

		if (ciTesterType == IQTYPE_XEL)
		{
			if (ciTesterControlMode == 0)  // IQapi_scpi
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_IQapi_SCPI_QA_DH_Run_%d.csv",Tag_runTime);
			}
			else   //SCPI
			{
				sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQxel_SCPI_QA_DH_Run_%d.csv",Tag_runTime);
			}
		}
		else
		{
			sprintf_s(g_csvFileName,sizeof(g_csvFileName),".\\Log\\IQ2010_QA_DH_Run_%d.csv",Tag_runTime);
		}

		//Init tester
		CheckReturnCode(LP_DualHead_ConOpen(g_DH_TOKEN_ID,g_IP_addr),"LP_DualHead_ConOpen()");;

		int tokenID=0;
		CheckReturnCode(LP_DualHead_GetTokenID( &tokenID ),"LP_DualHead_GetTokenID()");
		::LOGGER_Write(g_logger_id,LOGGER_INFORMATION, "LP_DualHead_GetTokenID= %u.\n",tokenID);

		CheckReturnCode(LP_GetVersion(buffer, MAX_BUFFER_SIZE),"LP_GetVersion()");
		printf("\n%s\n\n", buffer);
		::LOGGER_Write(g_logger_id,LOGGER_INFORMATION, "LP_GetVersion:%s.\n",buffer);


		// Connection to tester(s) has been established.  Wait for token...
		err = LP_DualHead_ObtainControl(2000,0);
		if ( ERR_OK!=err )
		{
			::LOGGER_Write(g_logger_id,LOGGER_ERROR, "Failed to obtain control.\n");
			throw "Failed to obtain control.\n";
		}
		else
		{
			//do nothing
		}

		for(int i=20;i>0;i--)
		{

			double amplValue=0;
			CheckReturnCode( LP_SetVsa(5520*1e6, 0, g_DH_PORT, 0, -25, 10e-6), "LP_SetVsa()" );
			LP_Agc(&amplValue);
			CheckReturnCode( LP_SetVsa(5520*1e6, amplValue, g_DH_PORT, 0, -25, 10e-6), "LP_SetVsa()" );
			CheckReturnCode(LP_EnableVsgRF(0),"LP_EnableVsgRF()");

			double samplingRate =0;
			if (ciTesterType == IQTYPE_XEL)
			{
				samplingRate = 160e6;

			}
			else
			{
				samplingRate = 80e6;
			}



			CheckReturnCode( LP_VsaDataCapture(200e-6, 13,samplingRate , 0), "LP_VsaDataCapture()" );
			CheckReturnCode( LP_SaveVsaSignalFile(".\\Log\\WIFI_AC_BW20_SS1_MCS2_BCC_Fs80M.iqvsg"), "LP_SaveVsaSignalFile()" );

			double result,result1,result2,result3,result4,result5,result6,result7,result8,result9,result10,result11;

			CheckReturnCode( LP_Analyze80211ac(), "LP_Analyze80211ac()" );
			result1 = LP_GetScalarMeasurement("psduCRC");
			if (result1 == 1)
			{
				printf("PSDU CRC: %s \n", "PASS");
			}
			else
				printf("PSDU CRC: %s \n", "FAIL");

			//Not working for 1 stream signal.
			// result = LP_GetScalarMeasurement("isolationDb");
			// printf("isolationDb=%0.2f \n", result);

			result2 = LP_GetScalarMeasurement("freqErrorHz");
			printf("Frequency Error: %.2f Hz\n", result2);

			result3 = LP_GetScalarMeasurement("symClockErrorPpm");
			printf("Symbol Clock Error: %.2f ppm\n", result3);

			result4 = LP_GetScalarMeasurement("PhaseNoiseDeg_RmsAll");
			printf("RMS Phase Noise: %.2f deg\n", result4);

			result5 = LP_GetScalarMeasurement("IQImbal_amplDb");
			printf("IQ Imbalance Amp: %.2f dB \n", result5);

			result6 = LP_GetScalarMeasurement("IQImbal_phaseDeg");
			printf("IQ Imbalance phase: %0.2f deg \n", result6);

			result7 = LP_GetScalarMeasurement("dcLeakageDbc");
			printf("LO leakage: %.2f dBc\n\n", result7);

			double bandwidthMhz = LP_GetScalarMeasurement("rateInfo_bandwidthMhz");
			printf("Signal Bandwidth: %0.2f MHz\n", bandwidthMhz);

			result8 = LP_GetScalarMeasurement("rateInfo_spatialStreams");
			printf("Number of Spatial Streams: %1.0f \n", result8);

			result9 = LP_GetScalarMeasurement("rateInfo_spaceTimeStreams");
			printf("Number of SpaceTime Streams: %1.0f \n", result9);

			char   result_char1[255];
			char   result_char2[255];

			LP_GetStringMeasurement("rateInfo_codingRate",result_char1,255);
			printf("codingRate: %s \n", result_char1);

			LP_GetStringMeasurement("rateInfo_modulation",result_char2,255);
			printf("modulation: %s \n", result_char2);

			result10 = LP_GetScalarMeasurement("rateInfo_dataRateMbps");
			printf("Datarate: %0.2f Mbps\n", result10);

			result11 = LP_GetScalarMeasurement("rxRmsPowerDb");
			printf("Power_NoGap: %.3f dBm\n", result11);
		}

		CheckReturnCode(LP_DualHead_ReleaseControl(),"LP_DualHead_ReleaseControl()");
		CheckReturnCode(LP_Term(),"LP_Term()");


	}
	catch(char *msg)
	{
		printf("ERROR: %s\n", msg);
	}
	catch(...)
	{
		printf("ERROR!");

	}
}
//end

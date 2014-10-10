#include "stdafx.h"
#include "iqapi.h"
#include "IQmeasure.h"

extern iqapiCapture *g_userData;
extern iqapiHndl				*hndl;

static double *g_fastcal_pwr_i = NULL;
static double *g_fastcal_pwr_q = NULL;
static unsigned int g_fastcal_length_i;
static unsigned int g_fastcal_length_q;



IQMEASURE_API int LP_PwrCalFastGetElement(void)
{
	int err = ERR_OK;

	//    g_fast_buffSize = g_userData->length[0];

	if(g_userData->length[0] == 0)
		return 0;

	//     g_fast_Real_Ptr = g_userData->real[0];
	//     g_fast_Imag_Ptr = g_userData->imag[0];

	if(LP_FastCalGetPowerData(TRUE))
	{
		// Data capture is invalid.
		LP_PwrCalFreeElement();
		err = ERR_INVALID_DATA_CAPTURE_RANGE;
		LP_FastCalGetPowerData(FALSE);
		return -1;
	}

	return err;
}

IQMEASURE_API int LP_PwrCalFreeElement(void)
{
	int err = ERR_OK;

	if(g_userData != NULL)
	{
		//         g_userData->real[0] = g_fast_Real_Ptr;
		//         g_userData->imag[0] = g_fast_Imag_Ptr;
		//         g_userData->length[0] = g_fast_buffSize;

		LP_FastCalGetPowerData(FALSE);
	}
	return err;
}

IQMEASURE_API int LP_FastCalGetPowerData (int bGetOrFree)                //indicate get or free data
{
	int max_samples;   // Max. number of samples in captured data.
	int err = ERR_OK;

	// Check for valid data.
	if(bGetOrFree)
	{
		if(g_userData != NULL && g_userData->length)
		{
			// We have valid data.
			//             char sBuf[MAX_BUFFER_SIZE] = {0};
			//             sprintf_s(sBuf, "log/Verify_captured_%d.sig", (int)(hndl->rx->rfFreqHz/1e6));
			//             hndl->data->Save(sBuf);
			// Proprietary way of calculating the power level - remove gap if exists
			// This remove gap algorithm use average power as reference.

			// ---------------------------------------------------------------------------
			// NOTE: It is assumed that the DC contents created by IQview has been removed
			//       by performing AGC calibration
			// ---------------------------------------------------------------------------

			// Max. number of samples in captured data.
			max_samples = (int)((hndl->rx->samplingTimeSecs +
						hndl->rx->triggerPreTime) *hndl->rx->sampleFreqHz);

			if(max_samples > g_userData->length[0])
				max_samples = g_userData->length[0];

			g_fastcal_pwr_i       = (double *) malloc (max_samples * sizeof (double));
			g_fastcal_pwr_q       = (double *) malloc (max_samples * sizeof (double));
			memset(g_fastcal_pwr_i,0,max_samples*sizeof(double));
			memset(g_fastcal_pwr_q,0,max_samples*sizeof(double));

			if(g_fastcal_pwr_i && g_fastcal_pwr_q )
			{
				g_fastcal_length_i = max_samples;
				g_fastcal_length_q = max_samples;

				memcpy(g_fastcal_pwr_i, g_userData->real[0],max_samples*sizeof(double));
				memcpy(g_fastcal_pwr_q, g_userData->imag[0],max_samples*sizeof(double));
			}
			else
			{
				if(g_fastcal_pwr_i)
				{
					free (g_fastcal_pwr_i);
					g_fastcal_pwr_i = NULL;
				}
				if(g_fastcal_pwr_q)
				{
					free (g_fastcal_pwr_q);
					g_fastcal_pwr_q = NULL;
				}
				err = 0;
			}
		}
		else
		{
			err = ERR_NO_VALID_ANALYSIS;
		}
	}
	else
	{
		if(g_fastcal_pwr_i)
		{
			free (g_fastcal_pwr_i);
			g_fastcal_pwr_i = NULL;
		}
		if(g_fastcal_pwr_q)
		{
			free (g_fastcal_pwr_q);
			g_fastcal_pwr_q = NULL;
		}
	}

	return err;
}

IQMEASURE_API int LP_FastCalMeasPower (unsigned int start_time, unsigned int  stop_time, double *result)
{
	double avg_pwr;
	int pwr_len = 0;
	unsigned int i;
	int err = ERR_OK;

	start_time = (unsigned int)(start_time * hndl->rx->sampleFreqHz / 1e6);
	stop_time = (unsigned int)(stop_time * hndl->rx->sampleFreqHz / 1e6);

	if(g_fastcal_pwr_i && g_fastcal_pwr_q && (g_fastcal_length_i == g_fastcal_length_q))
	{
		// Find the average power of the total power.
		avg_pwr = 0;
		for (i = start_time; i < stop_time; i++)
		{
			avg_pwr  += g_fastcal_pwr_i[i] * g_fastcal_pwr_i[i] + g_fastcal_pwr_q[i] * g_fastcal_pwr_q[i];

			pwr_len++;
		}
		avg_pwr = avg_pwr / pwr_len;


		*result = avg_pwr;
		*result = 10 * log10(*result);
	}
	else
	{
		err = ERR_INVALID_DATA_CAPTURE_RANGE;
	}

	return err;
}
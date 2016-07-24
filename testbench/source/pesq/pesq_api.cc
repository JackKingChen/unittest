/*******************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2014-03-29
*
*
*******************************************************************/

#include "pesq_api.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
static const double pesq_k_align_filter_dB [26] [2] = 
{
    {0.,   -500},
    {50.,  -500},
    {100., -500},
    {125., -500},
    {160., -500},
    {200., -500},
    {250., -500},
    {300., -500},
    {350.,  0},
    {400.,  0},
    {500.,  0},
    {600.,  0},
    {630.,  0},
    {800.,  0},
    {1000., 0},
    {1250., 0},
    {1600., 0},
    {2000., 0},
    {2500., 0},
    {3000., 0},
    {3250., 0},
    {3500., -500},
    {4000., -500},
    {5000., -500},
    {6300., -500},
    {8000., -500}
}; 

static const double pesq_k_standard_IRS_filter_dB [26] [2] = 
{
    {  0., -200},
    { 50., -40}, 
    {100., -20},
    {125., -12},
    {160.,  -6},
    {200.,   0},
    {250.,   4},
    {300.,   6},
    {350.,   8},
    {400.,  10},
    {500.,  11},
    {600.,  12},
    {700.,  12},
    {800.,  12},
    {1000., 12},
    {1300., 12},
    {1600., 12},
    {2000., 12},
    {2500., 12},
    {3000., 12},
    {3250., 12},
    {3500., 4},
    {4000., -200},
    {5000., -200},
    {6300., -200},
    {8000., -200}
}; 

static const float pesq_k_WB_InIIR_Hsos_8k[PESQ_LINIIR] = 
{
    2.6657628f,  -5.3315255f,  2.6657628f,  -1.8890331f,  0.89487434f 
};

static const float pesq_k_WB_InIIR_Hsos_16k[PESQ_LINIIR] =
{
    2.740826f,  -5.4816519f,  2.740826f,  -1.9444777f,  0.94597794f 
};


static const float pesq_k_InIIR_Hsos_8k[PESQ_LINIIR] =
{
    0.885535424f,        -0.885535424f,  0.000000000f,   -0.771070709f,  0.000000000f,
    0.895092588f,       1.292907193f,   0.449260174f,   1.268869037f,   0.442025372f,
    4.049527940f,       -7.865190042f,  3.815662102f,   -1.746859852f,  0.786305963f,
    0.500002353f,       -0.500002353f,  0.000000000f,   0.000000000f,   0.000000000f,
    0.565002834f,       -0.241585934f,  -0.306009671f,  0.259688659f,   0.249979657f,
    2.115237288f,       0.919935084f,   1.141240051f,   -1.587313419f,  0.665935315f,
    0.912224584f,       -0.224397719f,  -0.641121413f,  -0.246029464f,  -0.556720590f,
    0.444617727f,       -0.307589321f,  0.141638062f,   -0.996391149f,  0.502251622f 
};

static const float pesq_k_InIIR_Hsos_16k[PESQ_LINIIR] =
{ 
    0.325631521f,        -0.086782860f,  -0.238848661f,  -1.079416490f,  0.434583902f,
    0.403961804f,        -0.556985881f,  0.153024077f,   -0.415115835f,  0.696590244f,
    4.736162769f,        3.287251046f,   1.753289019f,   -1.859599046f,  0.876284034f,
    0.365373469f,        0.000000000f,   0.000000000f,   -0.634626531f,  0.000000000f,
    0.884811506f,        0.000000000f,   0.000000000f,   -0.256725271f,  0.141536777f,
    0.723593055f,        -1.447186099f,  0.723593044f,   -1.129587469f,  0.657232737f,
    1.644910855f,        -1.817280902f,  1.249658063f,   -1.778403899f,  0.801724355f,
    0.633692689f,        -0.284644314f,  -0.319789663f,  0.000000000f,   0.000000000f,
    1.032763031f,        0.268428979f,   0.602913323f,   0.000000000f,   0.000000000f,
    1.001616361f,        -0.823749013f,  0.439731942f,   -0.885778255f,  0.000000000f,
    0.752472096f,        -0.375388990f,  0.188977609f,   -0.077258216f,  0.247230734f,
    1.023700575f,        0.001661628f,   0.521284240f,   -0.183867259f,  0.354324187f 
};


/************************************************************************/
/*                                                                      */
/************************************************************************/

static void pesq_FixPowerLevel(PESQ_t *pesq,PESQ_Wave_t *info,long maxNsamples) 
{
    float *align_filtered;
    float  global_scale;
    float  power_above_300Hz;
    long   n;
    long   i;

    n = info-> Nsamples;
    align_filtered = (float *)malloc ((n + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000)) * sizeof (float));

    for (i = 0; i < n + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); i++) 
    {
        align_filtered [i] = info-> data [i];
    }

    pesq_Filter_FFT(pesq,align_filtered, info-> Nsamples, 26, pesq_k_align_filter_dB);

    power_above_300Hz = (float) pesq_dsp_powof (align_filtered, 
        PESQ_SEARCHBUFFER * pesq->downsample, 
        n - PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000),
        maxNsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000));

    global_scale = (float) sqrt (PESQ_TARGET_AVG_POWER / power_above_300Hz); 

    for (i = 0; i < n; i++)
    {
        info-> data [i] *= global_scale;
    }

    free(align_filtered);
}

static void pesq_InputFilter(PESQ_t *pesq,PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info, float * ftmp)
{
    pesq_Filter_DC(ref_info->data, ref_info->Nsamples ,pesq->downsample);
    pesq_Filter_DC(deg_info->data, deg_info->Nsamples ,pesq->downsample);

    pesq_Filter_IIR(pesq, ref_info->data, ref_info->Nsamples );
    pesq_Filter_IIR(pesq, deg_info->data, deg_info->Nsamples );
}

static void pesq_CalcVAD(PESQ_t *pesq, PESQ_Wave_t * sinfo )
{
    pesq_Filter_VAD(pesq, sinfo, sinfo-> data, sinfo-> VAD, sinfo-> logVAD);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
int pesq_init(PESQ_t *pesq,int sample_rate)
{
    memset(pesq,0,sizeof(PESQ_t));

    if( 16000 == sample_rate )
    {
        pesq->downsample = PESQ_DOWNSAMPLE_16K_BAND;
        pesq->alignNfft  = PESQ_ALIGN_NFFT_16K;

        pesq->InIIR_Hsos = pesq_k_InIIR_Hsos_16k;
        pesq->InIIR_Nsos = PESQ_IN_IIR_NSOS_16K;
    }
    else if( 8000 == sample_rate )
    {
        pesq->downsample = PESQ_DOWNSAMPLE_08K_BAND;
        pesq->alignNfft  = PESQ_ALIGN_NFFT_08K;

        pesq->InIIR_Hsos = pesq_k_InIIR_Hsos_8k;
        pesq->InIIR_Nsos = PESQ_IN_IIR_NSOS_08K;
    }
    else
    {
        return -1;
    }

    pesq->samplerate = sample_rate;
    return 0;
}

int pesq_free(PESQ_t *pesq)
{
    pesq_free_data(pesq);

    /*reset*/
    memset(pesq,0,sizeof(PESQ_t));

    return 0;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
int pesq_measure(PESQ_t *pesq,PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info,PESQ_Result_t * err_info)
{
    float        * ftmp        = pesq->run_temp;
    int            maxNsamples = std::max (ref_info-> Nsamples, deg_info-> Nsamples);
    float        * model_ref;
    float        * model_deg;
    const float  * WB_InIIR_Hsos;
    long           WB_InIIR_Nsos;
    long           i;

    /*
    * fix power
    */
    pesq_FixPowerLevel(pesq,ref_info, maxNsamples);
    pesq_FixPowerLevel(pesq,deg_info, maxNsamples);

    /*
    * IRS filtering...
    */
    if( pesq->samplerate == 16000 )
    {
        WB_InIIR_Nsos = 1;
        WB_InIIR_Hsos = pesq_k_WB_InIIR_Hsos_16k;
    } 
    else
    {
        WB_InIIR_Nsos = 1;
        WB_InIIR_Hsos = pesq_k_WB_InIIR_Hsos_8k;
    }

    /*
    * use filter
    */
    if( ref_info->input_filter == 1 )
    {
        pesq_Filter_FFT(pesq,ref_info-> data, ref_info-> Nsamples, 26, pesq_k_standard_IRS_filter_dB);
    }
    else
    {
        for( i = 0; i < 16; i++ ) 
        {
            ref_info->data[PESQ_SEARCHBUFFER * pesq->downsample + i - 1]
            *= (float)i / 16.0f;
            ref_info->data[ref_info->Nsamples - PESQ_SEARCHBUFFER * pesq->downsample - i]
            *= (float)i / 16.0f;
        }

        pesq_IIR_Filt(WB_InIIR_Hsos, WB_InIIR_Nsos, NULL,
            ref_info->data + PESQ_SEARCHBUFFER * pesq->downsample,
            ref_info->Nsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample, NULL );
    }
    if( deg_info->input_filter == 1 ) 
    {
        pesq_Filter_FFT(pesq,deg_info-> data, deg_info-> Nsamples, 26, pesq_k_standard_IRS_filter_dB);
    } 
    else 
    {
        for( i = 0; i < 16; i++ ) 
        {
            deg_info->data[PESQ_SEARCHBUFFER * pesq->downsample + i - 1]
            *= (float)i / 16.0f;
            deg_info->data[deg_info->Nsamples - PESQ_SEARCHBUFFER * pesq->downsample - i]
            *= (float)i / 16.0f;
        }
        pesq_IIR_Filt(WB_InIIR_Hsos, WB_InIIR_Nsos, NULL,
            deg_info->data + PESQ_SEARCHBUFFER * pesq->downsample,
            deg_info->Nsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample, NULL );
    }

    /*
    * 
    */
    model_ref = (float *) malloc ((ref_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000)) * sizeof (float));
    model_deg = (float *) malloc ((deg_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000)) * sizeof (float));

    for (i = 0; i < ref_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); i++) 
    {
        model_ref [i] = ref_info-> data [i];
    }

    for (i = 0; i < deg_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); i++) 
    {
        model_deg [i] = deg_info-> data [i];
    }

    pesq_InputFilter(pesq, ref_info, deg_info, ftmp );

    pesq_CalcVAD(pesq,ref_info);
    pesq_CalcVAD(pesq,deg_info);

    pesq_Filter_CrudeAlign(pesq,ref_info, deg_info, err_info, PESQ_WHOLE_SIGNAL, ftmp);

    pesq_utterance_locate(pesq,ref_info, deg_info, err_info, ftmp);

    for (i = 0; i < ref_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); i++)
    {
        ref_info-> data [i] = model_ref [i];
    }

    for (i = 0; i < deg_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); i++) 
    {
        deg_info-> data [i] = model_deg [i];
    }

    free (model_ref);
    free (model_deg);

    if (ref_info-> Nsamples < deg_info-> Nsamples) 
    {
        float *new_ref = (float *) malloc((deg_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000)) * sizeof(float));
        long  i;

        for (i = 0; i < ref_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); i++)
        {
            new_ref [i] = ref_info-> data [i];
        }
        for (i = ref_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); 
            i < deg_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); i++) 
        {
            new_ref [i] = 0.0f;
        }
        free (ref_info-> data);
        ref_info-> data = new_ref;
        new_ref = NULL;
    } 
    else 
    {
        if (ref_info-> Nsamples > deg_info-> Nsamples) 
        {
            float *new_deg = (float *) malloc((ref_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000)) * sizeof(float));
            long  i;

            for (i = 0; i < deg_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); i++) 
            {
                new_deg [i] = deg_info-> data [i];
            }
            for (i = deg_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); 
                i < ref_info-> Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); i++) 
            {
                new_deg [i] = 0.0f;
            }
            free (deg_info-> data);
            deg_info-> data = new_deg;
            new_deg = NULL;
        }
    }

    /*
    * Acoustic model processing...
    */
    pesq_psychoacoustic_model(pesq,ref_info, deg_info, err_info, ftmp);

    if ( err_info->mode == PESQ_NB_MODE )
    {
        err_info->mapped_mos = 0.999f+4.0f/(1.0f+(float)exp((-1.4945f*err_info->pesq_mos+4.6607f)));
    }
    else
    {
        err_info->mapped_mos = 0.999f+4.0f/(1.0f+(float)exp((-1.3669f*err_info->pesq_mos+3.8224f)));
    }

    return 0;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
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
#define DEBUG_FR            0
#define ZWICKER_POWER       0.23 
#define CRITERIUM_FOR_SILENCE_OF_5_SAMPLES 500.

/************************************************************************/
/*                                                                      */
/************************************************************************/
static const int pesq_k_nr_of_hz_bands_per_bark_band_8k [42] = 
{
    1,    1,    1,    1,    1,    
    1,    1,    1,    2,    1,    
    1,    1,    1,    1,    2,    
    1,    1,    2,    2,    2,    
    2,    2,    2,    2,    2,    
    3,    3,    3,    3,    4,    
    3,    4,    5,    4,    5,    
    6,    6,    7,    8,    9,    
    9,    11,    
};

static const double pesq_k_centre_of_band_bark_8k [42] = 
{
    0.078672,     0.316341,     0.636559,     0.961246,     1.290450,     
    1.624217,     1.962597,     2.305636,     2.653383,     3.005889,     
    3.363201,     3.725371,     4.092449,     4.464486,     4.841533,     
    5.223642,     5.610866,     6.003256,     6.400869,     6.803755,     
    7.211971,     7.625571,     8.044611,     8.469146,     8.899232,     
    9.334927,     9.776288,     10.223374,     10.676242,     11.134952,     
    11.599563,     12.070135,     12.546731,     13.029408,     13.518232,     
    14.013264,     14.514566,     15.022202,     15.536238,     16.056736,     
    16.583761,     17.117382
};

static const double pesq_k_centre_of_band_hz_8k [42] = 
{
    7.867213,     31.634144,     63.655895,     96.124611,     129.044968,     
    162.421738,     196.259659,     230.563568,     265.338348,     300.588867,     
    336.320129,     372.537140,     409.244934,     446.448578,     484.568604,     
    526.600586,     570.303833,     619.423340,     672.121643,     728.525696,     
    785.675964,     846.835693,     909.691650,     977.063293,     1049.861694,     
    1129.635986,     1217.257568,     1312.109497,     1412.501465,     1517.999390,     
    1628.894165,     1746.194336,     1871.568848,     2008.776123,     2158.979248,     
    2326.743164,     2513.787109,     2722.488770,     2952.586670,     3205.835449,     
    3492.679932,     3820.219238
};

static const double pesq_k_width_of_band_bark_8k [42] = 
{
    0.157344,     0.317994,     0.322441,     0.326934,     0.331474,     
    0.336061,     0.340697,     0.345381,     0.350114,     0.354897,     
    0.359729,     0.364611,     0.369544,     0.374529,     0.379565,     
    0.384653,     0.389794,     0.394989,     0.400236,     0.405538,     
    0.410894,     0.416306,     0.421773,     0.427297,     0.432877,     
    0.438514,     0.444209,     0.449962,     0.455774,     0.461645,     
    0.467577,     0.473569,     0.479621,     0.485736,     0.491912,     
    0.498151,     0.504454,     0.510819,     0.517250,     0.523745,     
    0.530308,     0.536934
};

static const double pesq_k_width_of_band_hz_8k [42] = 
{
    15.734426,     31.799433,     32.244064,     32.693359,     33.147385,     
    33.606140,     34.069702,     34.538116,     35.011429,     35.489655,     
    35.972870,     36.461121,     36.954407,     37.452911,     40.269653,     
    42.311859,     45.992554,     51.348511,     55.040527,     56.775208,     
    58.699402,     62.445862,     64.820923,     69.195374,     76.745667,     
    84.016235,     90.825684,     97.931152,     103.348877,     107.801880,     
    113.552246,     121.490601,     130.420410,     143.431763,     158.486816,     
    176.872803,     198.314697,     219.549561,     240.600098,     268.702393,     
    306.060059,     349.937012
};

static const double pesq_k_pow_dens_correction_factor_8k [42] = 
{ 
    100.000000,     99.999992,     100.000000,     100.000008,     100.000008, 
    100.000015,     99.999992,     99.999969,     50.000027,     100.000000,     
    99.999969,     100.000015,     99.999947,     100.000061,     53.047077,     
    110.000046,     117.991989,     65.000000,     68.760147,     69.999931,     
    71.428818,     75.000038,     76.843384,     80.968781,     88.646126,     
    63.864388,     68.155350,     72.547775,     75.584831,     58.379192,     
    80.950836,     64.135651,     54.384785,     73.821884,     64.437073,     
    59.176456,     65.521278,     61.399822,     58.144047,     57.004543,     
    64.126297,     59.248363
};

static const double pesq_k_abs_thresh_power_8k [42] = 
{
    51286152.000000,     2454709.500000,     70794.593750,     4897.788574,     1174.897705,     
    389.045166,     104.712860,     45.708820,     17.782795,     9.772372,     
    4.897789,     3.090296,     1.905461,     1.258925,     0.977237,     
    0.724436,     0.562341,     0.457088,     0.389045,     0.331131,     
    0.295121,     0.269153,     0.257040,     0.251189,     0.251189,     
    0.251189,     0.251189,     0.263027,     0.288403,     0.309030,     
    0.338844,     0.371535,     0.398107,     0.436516,     0.467735,     
    0.489779,     0.501187,     0.501187,     0.512861,     0.524807,     
    0.524807,     0.524807
};

static const int pesq_k_nr_of_hz_bands_per_bark_band_16k [49] = 
{
    1,    1,    1,    1,    1,    
    1,    1,    1,    2,    1,    
    1,    1,    1,    1,    2,    
    1,    1,    2,    2,    2,    
    2,    2,    2,    2,    2,    
    3,    3,    3,    3,    4,    
    3,    4,    5,    4,    5,    
    6,    6,    7,    8,    9,    
    9,    12,    12,    15,    16,    
    18,    21,    25,    20
};

static const double pesq_k_centre_of_band_bark_16k [49] = 
{
    0.078672,     0.316341,     0.636559,     0.961246,     1.290450,     
    1.624217,     1.962597,     2.305636,     2.653383,     3.005889,     
    3.363201,     3.725371,     4.092449,     4.464486,     4.841533,     
    5.223642,     5.610866,     6.003256,     6.400869,     6.803755,     
    7.211971,     7.625571,     8.044611,     8.469146,     8.899232,     
    9.334927,     9.776288,     10.223374,     10.676242,     11.134952,     
    11.599563,     12.070135,     12.546731,     13.029408,     13.518232,     
    14.013264,     14.514566,     15.022202,     15.536238,     16.056736,     
    16.583761,     17.117382,     17.657663,     18.204674,     18.758478,     
    19.319147,     19.886751,     20.461355,     21.043034
};

static const double pesq_k_centre_of_band_hz_16k [49] = 
{
    7.867213,     31.634144,     63.655895,     96.124611,     129.044968,     
    162.421738,     196.259659,     230.563568,     265.338348,     300.588867,     
    336.320129,     372.537140,     409.244934,     446.448578,     484.568604,     
    526.600586,     570.303833,     619.423340,     672.121643,     728.525696,     
    785.675964,     846.835693,     909.691650,     977.063293,     1049.861694,     
    1129.635986,     1217.257568,     1312.109497,     1412.501465,     1517.999390,     
    1628.894165,     1746.194336,     1871.568848,     2008.776123,     2158.979248,     
    2326.743164,     2513.787109,     2722.488770,     2952.586670,     3205.835449,     
    3492.679932,     3820.219238,     4193.938477,     4619.846191,     5100.437012,     
    5636.199219,     6234.313477,     6946.734863,     7796.473633
};

static const double pesq_k_width_of_band_bark_16k [49] = 
{
    0.157344,     0.317994,     0.322441,     0.326934,     0.331474,     
    0.336061,     0.340697,     0.345381,     0.350114,     0.354897,     
    0.359729,     0.364611,     0.369544,     0.374529,     0.379565,     
    0.384653,     0.389794,     0.394989,     0.400236,     0.405538,     
    0.410894,     0.416306,     0.421773,     0.427297,     0.432877,     
    0.438514,     0.444209,     0.449962,     0.455774,     0.461645,     
    0.467577,     0.473569,     0.479621,     0.485736,     0.491912,     
    0.498151,     0.504454,     0.510819,     0.517250,     0.523745,     
    0.530308,     0.536934,     0.543629,     0.550390,     0.557220,     
    0.564119,     0.571085,     0.578125,     0.585232
};

static const double pesq_k_width_of_band_hz_16k [49] = 
{ 
    15.734426,     31.799433,     32.244064,     32.693359,     33.147385,     
    33.606140,     34.069702,     34.538116,     35.011429,     35.489655,     
    35.972870,     36.461121,     36.954407,     37.452911,     40.269653,     
    42.311859,     45.992554,     51.348511,     55.040527,     56.775208,     
    58.699402,     62.445862,     64.820923,     69.195374,     76.745667,     
    84.016235,     90.825684,     97.931152,     103.348877,     107.801880,     
    113.552246,     121.490601,     130.420410,     143.431763,     158.486816,     
    176.872803,     198.314697,     219.549561,     240.600098,     268.702393,     
    306.060059,     349.937012,     398.686279,     454.713867,     506.841797,     
    564.863770,     637.261230,     794.717285,     931.068359
};

static const double pesq_k_pow_dens_correction_factor_16k [49] = 
{ 
    100.000000,     99.999992,     100.000000,     100.000008,     100.000008,     
    100.000015,     99.999992,     99.999969,     50.000027,     100.000000,     
    99.999969,     100.000015,     99.999947,     100.000061,     53.047077,     
    110.000046,     117.991989,     65.000000,     68.760147,     69.999931,     
    71.428818,     75.000038,     76.843384,     80.968781,     88.646126,     
    63.864388,     68.155350,     72.547775,     75.584831,     58.379192,     
    80.950836,     64.135651,     54.384785,     73.821884,     64.437073,     
    59.176456,     65.521278,     61.399822,     58.144047,     57.004543,     
    64.126297,     54.311001,     61.114979,     55.077751,     56.849335,     
    55.628868,     53.137054,     54.985844,     79.546974
};

static const double pesq_k_abs_thresh_power_16k [49] = 
{
    51286152.000000,     2454709.500000,     70794.593750,     4897.788574,     1174.897705,     
    389.045166,     104.712860,     45.708820,     17.782795,     9.772372,     
    4.897789,     3.090296,     1.905461,     1.258925,     0.977237,     
    0.724436,     0.562341,     0.457088,     0.389045,     0.331131,     
    0.295121,     0.269153,     0.257040,     0.251189,     0.251189,     
    0.251189,     0.251189,     0.263027,     0.288403,     0.309030,     
    0.338844,     0.371535,     0.398107,     0.436516,     0.467735,     
    0.489779,     0.501187,     0.501187,     0.512861,     0.524807,     
    0.524807,     0.524807,     0.512861,     0.478630,     0.426580,     
    0.371535,     0.363078,     0.416869,     0.537032
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
static void short_term_fft (int Nf, PESQ_Wave_t *info,
                            float *window, long start_sample, float *hz_spectrum, float *fft_tmp)
{
    int n, k;        

    for (n = 0; n < Nf; n++ )
    {
        fft_tmp [n] = info-> data [start_sample + n] * window [n];
    }
    pesq_RealFFT_Forword(fft_tmp, Nf);

    for (k = 0; k < Nf / 2; k++ ) 
    {
        hz_spectrum [k] = fft_tmp [k << 1] * fft_tmp [k << 1] + fft_tmp [1 + (k << 1)] * fft_tmp [1 + (k << 1)];
    }    

    hz_spectrum [0] = 0;
}

static void freq_warping (const int bark_band_table [],const double *correction_table,
                          int number_of_hz_bands, float *hz_spectrum,
                          int bands, float *pitch_pow_dens, long frame,float Sp)
{

    int        hz_band = 0;
    int        bark_band;
    double    sum;

    for (bark_band = 0; bark_band < bands; bark_band++)
    {
        int n = bark_band_table [bark_band];
        int i;

        sum = 0;
        for (i = 0; i < n; i++) {
            sum += hz_spectrum [hz_band++];
        }

        sum *= correction_table [bark_band];
        sum *= Sp;
        pitch_pow_dens [frame * bands + bark_band] = (float) sum;
    }
}

static float total_audible (const double*abs_table,int frame, float *pitch_pow_dens, float factor,int bands) 
{
    int        band;
    float     h, threshold;
    double  result;

    result = 0.;
    for (band= 1; band< bands; band++) {
        h = pitch_pow_dens [frame * bands + band];
        threshold = (float) (factor * abs_table [band]);
        if (h > threshold) {
            result += h;
        }
    }
    return (float) result;
}

static void time_avg_audible_of (const double*abs_table,int number_of_frames, int *silent,
                                 float *pitch_pow_dens, float *avg_pitch_pow_dens, 
                                 int total_number_of_frames,int bands)
{
    int    frame;
    int    band;

    for (band = 0; band < bands; band++)
    {
        double result = 0;
        for (frame = 0; frame < number_of_frames; frame++) {
            if (!silent [frame]) {
                float h = pitch_pow_dens [frame * bands + band];
                if (h > 100 * abs_table [band]) {
                    result += h;
                }
            }
        }

        avg_pitch_pow_dens [band] = (float) (result / total_number_of_frames);
    }
}            

static void freq_resp_compensation (int number_of_frames, float *pitch_pow_dens_ref,
                                    float *avg_pitch_pow_dens_ref, float *avg_pitch_pow_dens_deg, 
                                    float constant,int bands)
{
    int band;

    for (band = 0; band < bands; band++)
    {
        float    x = (avg_pitch_pow_dens_deg [band] + constant) / (avg_pitch_pow_dens_ref [band] + constant);
        int        frame;

        if (x > (float) 100.0) {x = (float) 100.0;} 
        if (x < (float) 0.01) {x = (float) 0.01;}   

        for (frame = 0; frame < number_of_frames; frame++) 
        {
            pitch_pow_dens_ref [frame * bands + band] *= x;
        }        
    }
}


static void intensity_warping_of (const double*abs_table,const double *centre_table,
                                  float *loudness_dens, int frame, float *pitch_pow_dens,int bands,float Sl)
{
    int        band;
    float    h;
    double    modified_zwicker_power;

    for (band = 0; band < bands; band++) {
        float threshold = (float) abs_table [band];
        float input = pitch_pow_dens [frame * bands + band];

        if (centre_table [band] < (float) 4) {
            h =  (float) 6 / ((float) centre_table [band] + (float) 2);
        } else {
            h = (float) 1;
        }
        if (h > (float) 2) {h = (float) 2;}
        h = (float) pow (h, (float) 0.15); 
        modified_zwicker_power = ZWICKER_POWER * h;

        if (input > threshold) {
            loudness_dens [band] = (float) (pow (threshold / 0.5, modified_zwicker_power)
                * (pow (0.5 + 0.5 * input / threshold, modified_zwicker_power) - 1));
        } else {
            loudness_dens [band] = 0;
        }

        loudness_dens [band] *= (float) Sl;
    }    
}

static float pseudo_Lp (const double *width_table,int bands, float *x, float p) 
{   
    double totalWeight = 0;
    double result = 0;
    int    band;

    for (band = 1; band < bands; band++) 
    {
        float h = (float) fabs (x [band]);        
        float w = (float) width_table [band];
        float prod = h * w;

        result += pow (prod, p);
        totalWeight += w;
    }

    result /= totalWeight;
    result = pow (result, (double)(1.0f/p));
    result *= totalWeight;

    return (float) result;
}  

static void multiply_with_asymmetry_factor (float      *disturbance_dens, 
                                            int         frame, 
                                            int         bands,
                                            const float   * const pitch_pow_dens_ref, 
                                            const float   * const pitch_pow_dens_deg) 
{
    int   i;
    float ratio, h;

    for (i = 0; i < bands; i++) 
    {
        ratio = (pitch_pow_dens_deg [frame * bands + i] + (float) 50)
            / (pitch_pow_dens_ref [frame * bands + i] + (float) 50);

        h = (float) pow (ratio, (float) 1.2);    
        if (h > (float) 12) {h = (float) 12;}
        if (h < (float) 3) {h = (float) 0.0;}

        disturbance_dens [i] *= h;
    }
}

static int compute_delay (long start_sample, 
                          long stop_sample, 
                          long search_range, 
                          float *time_series1, 
                          float *time_series2,
                          float *max_correlation)
{

    double            power1, power2, normalization;
    long            i;
    float           *x1, *x2, *y;
    double            h;
    long            n = stop_sample - start_sample;   
    long            power_of_2 = pesq_dsp_nextpow2 (2 * n);
    long            best_delay;

    power1 = pesq_dsp_powof (time_series1, start_sample, stop_sample, stop_sample - start_sample) * (double) n/(double) power_of_2;
    power2 = pesq_dsp_powof (time_series2, start_sample, stop_sample, stop_sample - start_sample) * (double) n/(double) power_of_2;
    normalization = sqrt (power1 * power2);

    if ((power1 <= 1E-6) || (power2 <= 1E-6)) {
        *max_correlation = 0;
        return 0;
    }

    x1 = (float *) malloc ((power_of_2 + 2) * sizeof (float));;
    x2 = (float *) malloc ((power_of_2 + 2) * sizeof (float));;
    y = (float *) malloc ((power_of_2 + 2) * sizeof (float));;

    for (i = 0; i < power_of_2 + 2; i++) {
        x1 [i] = 0.;
        x2 [i] = 0.;
        y [i] = 0.;
    }

    for (i = 0; i < n; i++) {
        x1 [i] = (float) fabs (time_series1 [i + start_sample]);
        x2 [i] = (float) fabs (time_series2 [i + start_sample]);
    }

    pesq_RealFFT_Forword (x1, power_of_2);
    pesq_RealFFT_Forword (x2, power_of_2);

    for (i = 0; i <= power_of_2 / 2; i++) { 
        x1 [2 * i] /= power_of_2;
        x1 [2 * i + 1] /= power_of_2;                
    }

    for (i = 0; i <= power_of_2 / 2; i++) { 
        y [2*i] = x1 [2*i] * x2 [2*i] + x1 [2*i + 1] * x2 [2*i + 1];
        y [2*i + 1] = -x1 [2*i + 1] * x2 [2*i] + x1 [2*i] * x2 [2*i + 1];
    }    

    pesq_RealFFT_Inverse (y, power_of_2);

    best_delay = 0;
    *max_correlation = 0;

    for (i = -search_range; i <= -1; i++) {
        h = (float) fabs (y [(i + power_of_2)]) / normalization;
        if (fabs (h) > (double) *max_correlation) {
            *max_correlation = (float) fabs (h);
            best_delay= i;
        }
    }

    for (i = 0; i < search_range; i++) {
        h = (float) fabs (y [i]) / normalization;
        if (fabs (h) > (double) *max_correlation) {
            *max_correlation = (float) fabs (h);
            best_delay= i;
        }
    }

    free (x1);
    free (x2);
    free (y);

    return best_delay;
}


#define NUMBER_OF_PSQM_FRAMES_PER_SYLLABE       20


float Lpq_weight (int         start_frame,
                  int         stop_frame,
                  float         power_syllable,
                  float      power_time,
                  float        *frame_disturbance,
                  float        *time_weight) {

                      double    result_time= 0;
                      double  total_time_weight_time = 0;
                      int        start_frame_of_syllable;

                      for (start_frame_of_syllable = start_frame; 
                          start_frame_of_syllable <= stop_frame; 
                          start_frame_of_syllable += NUMBER_OF_PSQM_FRAMES_PER_SYLLABE/2) {

                              double  result_syllable = 0;
                              int     count_syllable = 0;
                              int     frame;

                              for (frame = start_frame_of_syllable;
                                  frame < start_frame_of_syllable + NUMBER_OF_PSQM_FRAMES_PER_SYLLABE;
                                  frame++) {
                                      if (frame <= stop_frame) {
                                          float h = frame_disturbance [frame];
                                          result_syllable +=  pow (h, power_syllable); 
                                      }
                                      count_syllable++;                
                              }

                              result_syllable /= count_syllable;
                              result_syllable = pow (result_syllable, (double) 1/power_syllable);        

                              result_time+=  pow (time_weight [start_frame_of_syllable - start_frame] * result_syllable, (double)power_time); 
                              total_time_weight_time += pow (time_weight [start_frame_of_syllable - start_frame], power_time);
                      }

                      result_time /= total_time_weight_time;
                      result_time= pow (result_time, (double) 1.0 / power_time);

                      return (float) result_time;
}

void set_to_sine (PESQ_Wave_t *info, float amplitude, float omega) {
    long i;

    for (i = 0; i < info-> Nsamples; i++) {
        info-> data [i] = amplitude * (float) sin (omega * i);
    }
}

float maximum_of (float *x, long start, long stop) {
    long i;
    float result = -1E20f;

    for (i = start; i < stop; i++) {
        if (result < x [i]) {
            result = x [i];
        }
    }

    return result;
}

float integral_of (const double *width_table,float *x, long frames_after_start,int bands) 
{
    double result = 0;
    int    band;

    for (band = 1; band < bands; band++)
    {
        result += x [frames_after_start * bands + band] * width_table [band];        
    }
    return (float) result;    


    return (float) result;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
int pesq_id_searchwindows(PESQ_t *pesq, PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info,PESQ_Result_t * err_info)
{
    long  Utt_num = 0;
    long  count, VAD_length;
    long  this_start;
    int   speech_flag = 0;
    float VAD_value;
    long  del_deg_start;
    long  del_deg_end;

    VAD_length = ref_info-> Nsamples / pesq->downsample;

    del_deg_start = PESQ_MINUTTLENGTH - err_info-> Crude_DelayEst / pesq->downsample;
    del_deg_end =
        ((*deg_info).Nsamples - err_info-> Crude_DelayEst) / pesq->downsample -
        PESQ_MINUTTLENGTH;

    for (count = 0; count < VAD_length; count++)
    {
        VAD_value = ref_info-> VAD [count];

        if( (VAD_value > 0.0f) && (speech_flag == 0) ) 
        {
            speech_flag = 1;
            this_start = count;
            err_info-> UttSearch_Start [Utt_num] = count - PESQ_SEARCHBUFFER;
            if( err_info-> UttSearch_Start [Utt_num] < 0 )
                err_info-> UttSearch_Start [Utt_num] = 0;
        }

        if( ((VAD_value == 0.0f) || (count == (VAD_length-1))) &&
            (speech_flag == 1) ) 
        {
            speech_flag = 0;
            err_info-> UttSearch_End [Utt_num] = count + PESQ_SEARCHBUFFER;
            if( err_info-> UttSearch_End [Utt_num] > VAD_length - 1 )
                err_info-> UttSearch_End [Utt_num] = VAD_length -1;

            if( ((count - this_start) >= PESQ_MINUTTLENGTH) &&
                (this_start < del_deg_end) &&
                (count > del_deg_start) )
                Utt_num++;            
        }
    }

    err_info-> Nutterances = Utt_num;
    return Utt_num;
} 





/************************************************************************/
/*                                                                      */
/************************************************************************/
void pesq_id_utterances(PESQ_t *pesq, PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info,PESQ_Result_t * err_info )
{
    long  Utt_num = 0;
    long  Largest_uttsize = 0;
    long  count, VAD_length;
    int   speech_flag = 0;
    float VAD_value;
    long  this_start;
    long  last_end;
    long  del_deg_start;
    long  del_deg_end;

    VAD_length = ref_info-> Nsamples / pesq->downsample;

    del_deg_start = PESQ_MINUTTLENGTH - err_info-> Crude_DelayEst / pesq->downsample;
    del_deg_end =
        ((*deg_info).Nsamples - err_info-> Crude_DelayEst) / pesq->downsample -
        PESQ_MINUTTLENGTH;

    for (count = 0; count < VAD_length ; count++)
    {
        VAD_value = ref_info-> VAD [count];
        if( (VAD_value > 0.0f) && (speech_flag == 0) ) 
        {
            speech_flag = 1;
            this_start = count;
            err_info-> Utt_Start [Utt_num] = count;
        }

        if( ((VAD_value == 0.0f) || (count == (VAD_length-1))) &&
            (speech_flag == 1) ) 
        {
            speech_flag = 0;
            err_info-> Utt_End [Utt_num] = count;

            if( ((count - this_start) >= PESQ_MINUTTLENGTH) &&
                (this_start < del_deg_end) &&
                (count > del_deg_start) )
                Utt_num++;            
        }
    }

    err_info-> Utt_Start [0] = PESQ_SEARCHBUFFER;
    err_info-> Utt_End [err_info-> Nutterances-1] = (VAD_length - PESQ_SEARCHBUFFER);

    for (Utt_num = 1; Utt_num < err_info-> Nutterances; Utt_num++ )
    {
        this_start = err_info-> Utt_Start [Utt_num];
        last_end = err_info-> Utt_End [Utt_num - 1];
        count = (this_start + last_end) / 2;
        err_info-> Utt_Start [Utt_num] = count;
        err_info-> Utt_End [Utt_num - 1] = count;
    }

    this_start = (err_info-> Utt_Start [0] * pesq->downsample) + err_info-> Utt_Delay [0];
    if( this_start < (PESQ_SEARCHBUFFER * pesq->downsample) )
    {
        count = PESQ_SEARCHBUFFER +
            (pesq->downsample - 1 - err_info-> Utt_Delay [0]) / pesq->downsample;
        err_info-> Utt_Start [0] = count;
    }
    last_end = (err_info-> Utt_End [err_info-> Nutterances-1] * pesq->downsample) +
        err_info-> Utt_Delay [err_info-> Nutterances-1];
    if( last_end > ((*deg_info).Nsamples - PESQ_SEARCHBUFFER * pesq->downsample) )
    {
        count = ( (*deg_info).Nsamples -
            err_info-> Utt_Delay [err_info-> Nutterances-1] ) / pesq->downsample -
            PESQ_SEARCHBUFFER;
        err_info-> Utt_End [err_info-> Nutterances-1] = count;
    }

    for (Utt_num = 1; Utt_num < err_info-> Nutterances; Utt_num++ )
    {
        this_start =
            (err_info-> Utt_Start [Utt_num] * pesq->downsample) +
            err_info-> Utt_Delay [Utt_num];
        last_end =
            (err_info-> Utt_End [Utt_num - 1] * pesq->downsample) +
            err_info-> Utt_Delay [Utt_num - 1];
        if( this_start < last_end )
        {
            count = (this_start + last_end) / 2;
            this_start =
                (pesq->downsample - 1 + count - err_info-> Utt_Delay [Utt_num]) / pesq->downsample;
            last_end =
                (count - err_info-> Utt_Delay [Utt_num - 1]) / pesq->downsample;
            err_info-> Utt_Start [Utt_num] = this_start;
            err_info-> Utt_End [Utt_num - 1] = last_end;
        }
    }

    for (Utt_num = 0; Utt_num < err_info-> Nutterances; Utt_num++ )
        if( (err_info-> Utt_End [Utt_num] - err_info-> Utt_Start [Utt_num])
    > Largest_uttsize )
    Largest_uttsize = 
    err_info-> Utt_End [Utt_num] - err_info-> Utt_Start [Utt_num];

    err_info-> Largest_uttsize = Largest_uttsize;
}

void pesq_utterance_split(PESQ_t *pesq, PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info,PESQ_Result_t * err_info, float * ftmp )
{
    long Utt_id;
    long Utt_DelayEst;
    long Utt_Delay;
    float Utt_DelayConf;
    long Utt_Start;
    long Utt_End;
    long Utt_SpeechStart;
    long Utt_SpeechEnd;
    long Utt_Len;
    long step;
    long Best_ED1, Best_ED2;
    long Best_D1, Best_D2;
    float Best_DC1, Best_DC2;
    long Best_BP;
    long Largest_uttsize = 0;

    Utt_id = 0;
    while( (Utt_id < err_info-> Nutterances) &&
        (err_info-> Nutterances < PESQ_MAXNUTTERANCES) )
    {
        Utt_DelayEst = err_info-> Utt_DelayEst [Utt_id];
        Utt_Delay = err_info-> Utt_Delay [Utt_id];
        Utt_DelayConf = err_info-> Utt_DelayConf [Utt_id];
        Utt_Start = err_info-> Utt_Start [Utt_id];
        Utt_End = err_info-> Utt_End [Utt_id];

        Utt_SpeechStart = Utt_Start;
        while( (Utt_SpeechStart < Utt_End) && (ref_info-> VAD [Utt_SpeechStart] <= 0.0f) )
            Utt_SpeechStart++;
        Utt_SpeechEnd = Utt_End;
        while( (Utt_SpeechEnd > Utt_Start) && (ref_info-> VAD [Utt_SpeechEnd] <= 0.0f) )
            Utt_SpeechEnd--;
        Utt_SpeechEnd++;
        Utt_Len = Utt_SpeechEnd - Utt_SpeechStart;

        if( Utt_Len >= 200 )
        {
            pesq_Filter_SplitAlign(pesq, ref_info, deg_info, err_info, ftmp,
                Utt_Start, Utt_SpeechStart, Utt_SpeechEnd, Utt_End,
                Utt_DelayEst, Utt_DelayConf,
                &Best_ED1, &Best_D1, &Best_DC1,
                &Best_ED2, &Best_D2, &Best_DC2,
                &Best_BP);

            if( (Best_DC1 > Utt_DelayConf) && (Best_DC2 > Utt_DelayConf) )
            {
                for (step = err_info-> Nutterances-1; step > Utt_id; step-- )
                {
                    err_info-> Utt_DelayEst [step +1] = err_info-> Utt_DelayEst [step];
                    err_info-> Utt_Delay [step +1] = err_info-> Utt_Delay [step];
                    err_info-> Utt_DelayConf [step +1] = err_info-> Utt_DelayConf [step];
                    err_info-> Utt_Start [step +1] = err_info-> Utt_Start [step];
                    err_info-> Utt_End [step +1] = err_info-> Utt_End [step];
                    err_info-> UttSearch_Start [step +1] = err_info-> Utt_Start [step];
                    err_info-> UttSearch_End [step +1] = err_info-> Utt_End [step];
                }
                err_info-> Nutterances++;

                err_info-> Utt_DelayEst [Utt_id] = Best_ED1;
                err_info-> Utt_Delay [Utt_id] = Best_D1;
                err_info-> Utt_DelayConf [Utt_id] = Best_DC1;

                err_info-> Utt_DelayEst [Utt_id +1] = Best_ED2;
                err_info-> Utt_Delay [Utt_id +1] = Best_D2;
                err_info-> Utt_DelayConf [Utt_id +1] = Best_DC2;

                err_info-> UttSearch_Start [Utt_id +1] = err_info-> UttSearch_Start [Utt_id];
                err_info-> UttSearch_End [Utt_id +1] = err_info-> UttSearch_End [Utt_id];

                if( Best_D2 < Best_D1 )
                {
                    err_info-> Utt_Start [Utt_id] = Utt_Start;
                    err_info-> Utt_End [Utt_id] = Best_BP;
                    err_info-> Utt_Start [Utt_id +1] = Best_BP;
                    err_info-> Utt_End [Utt_id +1] = Utt_End;
                }
                else
                {
                    err_info-> Utt_Start [Utt_id] = Utt_Start;
                    err_info-> Utt_End [Utt_id] = Best_BP + (Best_D2 - Best_D1) / (2 * pesq->downsample);
                    err_info-> Utt_Start [Utt_id +1] = Best_BP - (Best_D2 - Best_D1) / (2 * pesq->downsample);
                    err_info-> Utt_End [Utt_id +1] = Utt_End;
                }

                if( (err_info-> Utt_Start [Utt_id] - PESQ_SEARCHBUFFER) * pesq->downsample + Best_D1 < 0 )
                    err_info-> Utt_Start [Utt_id] =
                    PESQ_SEARCHBUFFER + (pesq->downsample - 1 - Best_D1) / pesq->downsample;

                if( (err_info-> Utt_End [Utt_id +1] * pesq->downsample + Best_D2) >
                    ((*deg_info).Nsamples - PESQ_SEARCHBUFFER * pesq->downsample) )
                    err_info-> Utt_End [Utt_id +1] =
                    ((*deg_info).Nsamples - Best_D2) / pesq->downsample - PESQ_SEARCHBUFFER;

            }
            else Utt_id++;
        }
        else Utt_id++;
    }

    for (Utt_id = 0; Utt_id < err_info-> Nutterances; Utt_id++ )
        if( (err_info-> Utt_End [Utt_id] - err_info-> Utt_Start [Utt_id])
    > Largest_uttsize )
    Largest_uttsize = 
    err_info-> Utt_End [Utt_id] - err_info-> Utt_Start [Utt_id];

    err_info-> Largest_uttsize = Largest_uttsize;
}

void pesq_utterance_locate(PESQ_t *pesq, PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info,
                           PESQ_Result_t * err_info, float * ftmp )
{    
    long Utt_id;

    pesq_id_searchwindows(pesq, ref_info, deg_info, err_info );

    for (Utt_id = 0; Utt_id < err_info-> Nutterances; Utt_id++)
    {
        pesq_Filter_CrudeAlign(pesq,ref_info, deg_info, err_info, Utt_id, ftmp);
        pesq_Filter_TimeAlign(pesq,ref_info, deg_info, err_info, Utt_id, ftmp);
    }

    pesq_id_utterances(pesq, ref_info, deg_info, err_info );

    pesq_utterance_split(pesq, ref_info, deg_info, err_info, ftmp );   
}


void pesq_psychoacoustic_model(PESQ_t *pesq,
                               PESQ_Wave_t    * ref_info, 
                               PESQ_Wave_t    * deg_info,
                               PESQ_Result_t  * err_info, 
                               float          * ftmp)
{
    const int *nr_of_hz_bands_per_bark_band;
    const double *centre_of_band_bark;
    const double *centre_of_band_hz;
    const double *width_of_band_bark;
    const double *width_of_band_hz;
    const double *pow_dens_correction_factor;
    const double *abs_thresh_power;

    long    maxNsamples = std::max (ref_info-> Nsamples, deg_info-> Nsamples);
    long    Nf = pesq->downsample * 8L;
    long    start_frame, stop_frame;
    long    samples_to_skip_at_start, samples_to_skip_at_end;
    float   sum_of_5_samples;
    long    n, i;
    float   power_ref, power_deg;
    long    frame;
    float   *fft_tmp;
    float   *hz_spectrum_ref, *hz_spectrum_deg;
    float   *pitch_pow_dens_ref, *pitch_pow_dens_deg;
    float   *loudness_dens_ref, *loudness_dens_deg;
    float   *avg_pitch_pow_dens_ref, *avg_pitch_pow_dens_deg;
    float   *deadzone;
    float   *disturbance_dens, *disturbance_dens_asym_add;
    float    total_audible_pow_ref, total_audible_pow_deg;
    int     *silent;
    float    oldScale, scale;
    int     *frame_was_skipped;
    float   *frame_disturbance;
    float   *frame_disturbance_asym_add;
    float   *total_power_ref;
    int         utt;

#ifdef CALIBRATE
    int     periodInSamples;
    int     numberOfPeriodsPerFrame;
    float   omega; 
#endif

    float   peak;

#define    MAX_NUMBER_OF_BAD_INTERVALS        1000

    int        *frame_is_bad; 
    int        *smeared_frame_is_bad; 
    int         start_frame_of_bad_interval [MAX_NUMBER_OF_BAD_INTERVALS];    
    int         stop_frame_of_bad_interval [MAX_NUMBER_OF_BAD_INTERVALS];    
    int         start_sample_of_bad_interval [MAX_NUMBER_OF_BAD_INTERVALS];    
    int         stop_sample_of_bad_interval [MAX_NUMBER_OF_BAD_INTERVALS];   
    int         number_of_samples_in_bad_interval [MAX_NUMBER_OF_BAD_INTERVALS];    
    int         delay_in_samples_in_bad_interval  [MAX_NUMBER_OF_BAD_INTERVALS];    
    int         number_of_bad_intervals= 0;
    int         search_range_in_samples;
    int         bad_interval;
    float      *untweaked_deg = NULL;
    float      *tweaked_deg = NULL;
    float      *doubly_tweaked_deg = NULL;
    int         there_is_a_bad_frame = 0;
    float      *time_weight;
    float       d_indicator, a_indicator;
    int   nn;
    float Sl;
    float Sp;
    float Whanning [PESQ_Nfmax];

    for (n = 0L; n < Nf; n++ ) {
        Whanning [n] = (float)(0.5 * (1.0 - cos((PESQ_TWOPI * n) / Nf)));
    }

    switch (pesq->samplerate)
    {
    case 8000:
        pesq->bands = 42;
        Sl = (float) PESQ_Sl_8k;
        Sp = (float) PESQ_Sp_8k;
        nr_of_hz_bands_per_bark_band = pesq_k_nr_of_hz_bands_per_bark_band_8k;
        centre_of_band_bark = pesq_k_centre_of_band_bark_8k;
        centre_of_band_hz = pesq_k_centre_of_band_hz_8k;
        width_of_band_bark = pesq_k_width_of_band_bark_8k;
        width_of_band_hz = pesq_k_width_of_band_hz_8k;
        pow_dens_correction_factor = pesq_k_pow_dens_correction_factor_8k;
        abs_thresh_power = pesq_k_abs_thresh_power_8k;
        break;
    case 16000:
        pesq->bands = 49;
        Sl = (float) PESQ_Sl_16k;
        Sp = (float) PESQ_Sp_16k;
        nr_of_hz_bands_per_bark_band = pesq_k_nr_of_hz_bands_per_bark_band_16k;
        centre_of_band_bark = pesq_k_centre_of_band_bark_16k;
        centre_of_band_hz = pesq_k_centre_of_band_hz_16k;
        width_of_band_bark = pesq_k_width_of_band_bark_16k;
        width_of_band_hz = pesq_k_width_of_band_hz_16k;
        pow_dens_correction_factor = pesq_k_pow_dens_correction_factor_16k;
        abs_thresh_power = pesq_k_abs_thresh_power_16k;
        break;
    default:
        return ;
    }

    samples_to_skip_at_start = 0;
    do 
    {
        sum_of_5_samples= (float) 0;
        for (i = 0; i < 5; i++) 
        {
            sum_of_5_samples += (float) fabs (ref_info-> data [PESQ_SEARCHBUFFER * pesq->downsample + samples_to_skip_at_start + i]);
        }
        if (sum_of_5_samples< CRITERIUM_FOR_SILENCE_OF_5_SAMPLES) {
            samples_to_skip_at_start++;         
        }        
    } while ((sum_of_5_samples< CRITERIUM_FOR_SILENCE_OF_5_SAMPLES) 
        && (samples_to_skip_at_start < maxNsamples / 2));

    samples_to_skip_at_end = 0;
    do 
    {
        sum_of_5_samples= (float) 0;
        for (i = 0; i < 5; i++) 
        {
            sum_of_5_samples += 
                (float) fabs (ref_info-> data [maxNsamples - 
                PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000) - 1 - samples_to_skip_at_end - i]);
        }
        if (sum_of_5_samples< CRITERIUM_FOR_SILENCE_OF_5_SAMPLES) 
        {
            samples_to_skip_at_end++;         
        }

    } while ((sum_of_5_samples< CRITERIUM_FOR_SILENCE_OF_5_SAMPLES) 
        && (samples_to_skip_at_end < maxNsamples / 2));

    start_frame = samples_to_skip_at_start / (Nf /2);
    stop_frame  =
        (maxNsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  *
        (pesq->samplerate / 1000) - samples_to_skip_at_end) / (Nf /2) - 1; 

    power_ref = (float) pesq_dsp_powof (ref_info-> data, 
        PESQ_SEARCHBUFFER * pesq->downsample, 
        maxNsamples - PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000),
        maxNsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000)); 
    power_deg = (float) pesq_dsp_powof (deg_info-> data, 
        PESQ_SEARCHBUFFER * pesq->downsample, 
        maxNsamples - PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000),
        maxNsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000));

    fft_tmp             = (float *) malloc ((Nf + 2) * sizeof (float));
    hz_spectrum_ref     = (float *) malloc ((Nf / 2) * sizeof (float));
    hz_spectrum_deg     = (float *) malloc ((Nf / 2) * sizeof (float));

    frame_is_bad        = (int *) malloc ((stop_frame + 1) * sizeof (int)); 
    smeared_frame_is_bad=(int *) malloc ((stop_frame + 1) * sizeof (int)); 

    silent              = (int *) malloc ((stop_frame + 1) * sizeof (int));

    pitch_pow_dens_ref  = (float *) malloc ((stop_frame + 1) * pesq->bands * sizeof (float));
    pitch_pow_dens_deg  = (float *) malloc ((stop_frame + 1) * pesq->bands * sizeof (float));

    frame_was_skipped  = (int *) malloc ((stop_frame + 1) * sizeof (int));

    frame_disturbance  = (float *) malloc ((stop_frame + 1) * sizeof (float));
    frame_disturbance_asym_add    = (float *) malloc ((stop_frame + 1) * sizeof (float));

    avg_pitch_pow_dens_ref = (float *) malloc (pesq->bands * sizeof (float));
    avg_pitch_pow_dens_deg = (float *) malloc (pesq->bands * sizeof (float));
    loudness_dens_ref      = (float *) malloc (pesq->bands * sizeof (float));
    loudness_dens_deg      = (float *) malloc (pesq->bands * sizeof (float));;
    deadzone               = (float *) malloc (pesq->bands * sizeof (float));;
    disturbance_dens       = (float *) malloc (pesq->bands * sizeof (float));
    disturbance_dens_asym_add = (float *) malloc (pesq->bands * sizeof (float));    

    time_weight         = (float *) malloc ((stop_frame + 1) * sizeof (float));
    total_power_ref     = (float *) malloc ((stop_frame + 1) * sizeof (float));

#ifdef CALIBRATE
    periodInSamples = pesq->samplerate / 1000;
    numberOfPeriodsPerFrame = Nf / periodInSamples;
    omega = (float) (PESQ_TWOPI / periodInSamples);    
    peak;

    set_to_sine (ref_info, (float) 29.54, (float) omega);    
#endif

    for (frame = 0; frame <= stop_frame; frame++) 
    {
        int start_sample_ref = PESQ_SEARCHBUFFER * pesq->downsample + frame * Nf / 2;
        int start_sample_deg;
        int delay;    

        short_term_fft (Nf, ref_info, Whanning, start_sample_ref, hz_spectrum_ref, fft_tmp);

        assert (err_info-> Nutterances >= 1);

        utt = err_info-> Nutterances - 1;
        while ((utt >= 0) && (err_info-> Utt_Start [utt] * pesq->downsample > start_sample_ref)) 
        {
            utt--;
        }
        if (utt >= 0) 
        {
            delay = err_info-> Utt_Delay [utt];
        }
        else 
        {
            delay = err_info-> Utt_Delay [0];        
        }
        start_sample_deg = start_sample_ref + delay;         

        if ((start_sample_deg > 0) && (start_sample_deg + Nf < maxNsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000)))
        {
            short_term_fft (Nf, deg_info, Whanning, start_sample_deg, hz_spectrum_deg, fft_tmp);            
        } 
        else 
        {
            for (i = 0; i < Nf / 2; i++) 
            {
                hz_spectrum_deg [i] = 0;
            }
        }

        freq_warping (nr_of_hz_bands_per_bark_band,
            pow_dens_correction_factor,
            Nf / 2, hz_spectrum_ref, pesq->bands, pitch_pow_dens_ref, frame,Sp);

        peak = maximum_of (pitch_pow_dens_ref, 0, pesq->bands);    

        freq_warping (nr_of_hz_bands_per_bark_band,
            pow_dens_correction_factor,
            Nf / 2, hz_spectrum_deg, pesq->bands, pitch_pow_dens_deg, frame,Sp);

        total_audible_pow_ref = total_audible (abs_thresh_power,frame, pitch_pow_dens_ref, 1E2,pesq->bands);
        total_audible_pow_deg = total_audible (abs_thresh_power,frame, pitch_pow_dens_deg, 1E2,pesq->bands);        

        silent [frame] = (total_audible_pow_ref < 1E7);     
    }    

    time_avg_audible_of (abs_thresh_power,
        stop_frame + 1, silent, pitch_pow_dens_ref,
        avg_pitch_pow_dens_ref,
        (maxNsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000)) / (Nf / 2) - 1,
        pesq->bands);

    time_avg_audible_of (abs_thresh_power,
        stop_frame + 1, silent, pitch_pow_dens_deg,
        avg_pitch_pow_dens_deg, 
        (maxNsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000)) / (Nf / 2) - 1,
        pesq->bands);

#ifndef CALIBRATE
    freq_resp_compensation (stop_frame + 1, pitch_pow_dens_ref, 
        avg_pitch_pow_dens_ref, avg_pitch_pow_dens_deg, 1000,pesq->bands);
#endif

    oldScale = 1;
    for (frame = 0; frame <= stop_frame; frame++)
    {
        int band;

        total_audible_pow_ref = total_audible (abs_thresh_power,frame, pitch_pow_dens_ref, 1,pesq->bands);
        total_audible_pow_deg = total_audible (abs_thresh_power,frame, pitch_pow_dens_deg, 1,pesq->bands);        
        total_power_ref [frame] = total_audible_pow_ref;

        scale = (total_audible_pow_ref + (float) 5E3) / (total_audible_pow_deg + (float) 5E3);

        if (frame > 0) 
        {
            scale = (float) 0.2 * oldScale + (float) 0.8*scale;
        }
        oldScale = scale;

#define MAX_SCALE   5.0

        if (scale > (float) MAX_SCALE) scale = (float) MAX_SCALE;

#define MIN_SCALE   3E-4

        if (scale < (float) MIN_SCALE) 
        {
            scale = (float) MIN_SCALE;
        }

        for (band = 0; band < pesq->bands; band++) {
            pitch_pow_dens_deg [frame * pesq->bands + band] *= scale;
        }

        intensity_warping_of (abs_thresh_power,
            centre_of_band_bark,loudness_dens_ref, frame, pitch_pow_dens_ref,pesq->bands,Sl); 
        intensity_warping_of (abs_thresh_power,
            centre_of_band_bark,loudness_dens_deg, frame, pitch_pow_dens_deg,pesq->bands,Sl); 

        for (band = 0; band < pesq->bands; band++) 
        {
            disturbance_dens [band] = loudness_dens_deg [band] - loudness_dens_ref [band];
        }

        for (band = 0; band < pesq->bands; band++) 
        {
            deadzone [band] = std::min(loudness_dens_deg [band], loudness_dens_ref [band]);    
            deadzone [band] *= 0.25;
        }

        for (band = 0; band < pesq->bands; band++) 
        {
            float d = disturbance_dens [band];
            float m = deadzone [band];

            if (d > m) 
            {
                disturbance_dens [band] -= m;
            } 
            else
            {
                if (d < -m) 
                {
                    disturbance_dens [band] += m;
                } 
                else 
                {
                    disturbance_dens [band] = 0;
                }
            }
        }    

        frame_disturbance [frame] = pseudo_Lp (width_of_band_bark,pesq->bands, disturbance_dens, PESQ_D_POW_F);    

#define THRESHOLD_BAD_FRAMES   30

        if (frame_disturbance [frame] > THRESHOLD_BAD_FRAMES) 
        {
            there_is_a_bad_frame = 1;
        }

        multiply_with_asymmetry_factor (disturbance_dens, frame,pesq->bands, pitch_pow_dens_ref, pitch_pow_dens_deg);

        frame_disturbance_asym_add [frame] = pseudo_Lp (width_of_band_bark,pesq->bands, disturbance_dens, PESQ_A_POW_F);
    }

    for (frame = 0; frame <= stop_frame; frame++) 
    {
        frame_was_skipped [frame] = 0;
    }

    for (utt = 1; utt < err_info-> Nutterances; utt++) 
    {
        int frame1 = (int) floor ((float)((err_info-> Utt_Start [utt] - PESQ_SEARCHBUFFER ) * pesq->downsample + err_info-> Utt_Delay [utt]) / (Nf / 2));
        int j = (int) floor ((float)(err_info-> Utt_End [utt-1] - PESQ_SEARCHBUFFER) * pesq->downsample + err_info-> Utt_Delay [utt-1]) / (Nf / 2);
        int delay_jump = err_info-> Utt_Delay [utt] - err_info-> Utt_Delay [utt-1];

        if (frame1 > j)
        {
            frame1 = j;
        }

        if (frame1 < 0) 
        {
            frame1 = 0;
        }

        if (delay_jump < -(int) (Nf / 2)) 
        {
            int frame2 = (int) ((err_info-> Utt_Start [utt] - PESQ_SEARCHBUFFER) 
                * pesq->downsample + std::max(0.0f, (float)fabs((float)delay_jump))) / (Nf / 2) + 1; 

            for (frame = frame1; frame <= frame2; frame++)  
            {
                if (frame < stop_frame) 
                {
                    frame_was_skipped [frame] = 1;

                    frame_disturbance [frame] = 0;
                    frame_disturbance_asym_add [frame] = 0;
                }
            } 
        }    
    }

    nn = PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000) + maxNsamples;

    tweaked_deg = (float *) malloc (nn * sizeof (float));

    for (i = 0; i < nn; i++) 
    {
        tweaked_deg [i] = 0;
    } 

    for (i = PESQ_SEARCHBUFFER * pesq->downsample; i < nn - PESQ_SEARCHBUFFER * pesq->downsample; i++) 
    {
        int  utt = err_info-> Nutterances - 1;
        long delay, j;

        while ((utt >= 0) && (err_info-> Utt_Start [utt] * pesq->downsample > i))
        {
            utt--;
        }
        if (utt >= 0) 
        {
            delay = err_info-> Utt_Delay [utt];
        }
        else 
        {
            delay = err_info-> Utt_Delay [0];
        }

        j = i + delay;
        if (j < PESQ_SEARCHBUFFER * pesq->downsample) 
        {
            j = PESQ_SEARCHBUFFER * pesq->downsample;
        }
        if (j >= nn - PESQ_SEARCHBUFFER * pesq->downsample)
        {
            j = nn - PESQ_SEARCHBUFFER * pesq->downsample - 1;
        }
        tweaked_deg [i] = deg_info-> data [j];
    }

    if (there_is_a_bad_frame) 
    {
        for (frame = 0; frame <= stop_frame; frame++) 
        {  
            frame_is_bad [frame] = (frame_disturbance [frame] > THRESHOLD_BAD_FRAMES);

            smeared_frame_is_bad [frame] = 0;
        }
        frame_is_bad [0] = 0;

#define SMEAR_RANGE 2

        for (frame = SMEAR_RANGE; frame < stop_frame - SMEAR_RANGE; frame++) 
        {
            long max_itself_and_left = frame_is_bad [frame];
            long max_itself_and_right = frame_is_bad [frame];
            long mini, i;

            for (i = -SMEAR_RANGE; i <= 0; i++) {
                if (max_itself_and_left < frame_is_bad [frame  + i]) {
                    max_itself_and_left = frame_is_bad [frame  + i];
                }
            }

            for (i = 0; i <= SMEAR_RANGE; i++) {
                if (max_itself_and_right < frame_is_bad [frame + i]) {
                    max_itself_and_right = frame_is_bad [frame + i];
                }
            }

            mini = max_itself_and_left;
            if (mini > max_itself_and_right) 
            {
                mini = max_itself_and_right;
            }

            smeared_frame_is_bad [frame] = mini;
        }

#define MINIMUM_NUMBER_OF_BAD_FRAMES_IN_BAD_INTERVAL    5

        number_of_bad_intervals = 0;    
        frame = 0; 
        while (frame <= stop_frame) 
        {
            while ((frame <= stop_frame) && (!smeared_frame_is_bad [frame])) 
            {
                frame++; 
            }

            if (frame <= stop_frame) { 
                start_frame_of_bad_interval [number_of_bad_intervals] = frame;

                while ((frame <= stop_frame) && (smeared_frame_is_bad [frame])) 
                {
                    frame++; 
                }

                if (frame <= stop_frame) 
                {
                    stop_frame_of_bad_interval [number_of_bad_intervals] = frame; 

                    if (stop_frame_of_bad_interval [number_of_bad_intervals] - start_frame_of_bad_interval [number_of_bad_intervals] 
                    >= MINIMUM_NUMBER_OF_BAD_FRAMES_IN_BAD_INTERVAL) 
                    {
                        number_of_bad_intervals++; 
                    }
                }
            }
        }

        for (bad_interval = 0; bad_interval < number_of_bad_intervals; bad_interval++) 
        {
            start_sample_of_bad_interval [bad_interval] = 
                start_frame_of_bad_interval [bad_interval] * (Nf / 2) + PESQ_SEARCHBUFFER * pesq->downsample;
            stop_sample_of_bad_interval [bad_interval]  =  
                stop_frame_of_bad_interval [bad_interval] * (Nf / 2) + Nf + PESQ_SEARCHBUFFER* pesq->downsample;

            if (stop_frame_of_bad_interval [bad_interval] > stop_frame) 
            {
                stop_frame_of_bad_interval [bad_interval] = stop_frame; 
            }

            number_of_samples_in_bad_interval [bad_interval] = 
                stop_sample_of_bad_interval [bad_interval] - start_sample_of_bad_interval [bad_interval];
        }


#define SEARCH_RANGE_IN_TRANSFORM_LENGTH    4

        search_range_in_samples= SEARCH_RANGE_IN_TRANSFORM_LENGTH * Nf;

        for (bad_interval= 0; bad_interval< number_of_bad_intervals; bad_interval++) 
        {
            float  *ref = (float *) malloc ( (2 * search_range_in_samples + number_of_samples_in_bad_interval [bad_interval]) * sizeof (float));
            float  *deg = (float *) malloc ( (2 * search_range_in_samples + number_of_samples_in_bad_interval [bad_interval]) * sizeof (float));
            int     i;
            float   best_correlation;
            int     delay_in_samples;

            for (i = 0; i < search_range_in_samples; i++) 
            {
                ref[i] = 0.0f;
            }
            for (i = 0; i < number_of_samples_in_bad_interval [bad_interval]; i++) 
            {
                ref [search_range_in_samples + i] = ref_info-> data [start_sample_of_bad_interval [bad_interval] + i];
            }
            for (i = 0; i < search_range_in_samples; i++)
            {
                ref [search_range_in_samples + number_of_samples_in_bad_interval [bad_interval] + i] = 0.0f;
            }

            for (i = 0; 
                i < 2 * search_range_in_samples + number_of_samples_in_bad_interval [bad_interval];
                i++) 
            {

                int j = start_sample_of_bad_interval [bad_interval] - search_range_in_samples + i;
                int nn = maxNsamples - PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000);

                if (j < PESQ_SEARCHBUFFER * pesq->downsample) 
                {
                    j = PESQ_SEARCHBUFFER * pesq->downsample;
                }
                if (j >= nn) 
                {
                    j = nn - 1;
                }
                deg [i] = tweaked_deg [j]; 
            }

            delay_in_samples= compute_delay (0, 
                2 * search_range_in_samples + number_of_samples_in_bad_interval [bad_interval], 
                search_range_in_samples,
                ref, 
                deg,
                &best_correlation);

            delay_in_samples_in_bad_interval [bad_interval] =  delay_in_samples;

            if (best_correlation < 0.5) 
            {
                delay_in_samples_in_bad_interval  [bad_interval] = 0;
            } 

            free (ref);
            free (deg);
        }

        if (number_of_bad_intervals > 0) 
        {
            doubly_tweaked_deg = (float *) malloc ((maxNsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000)) * sizeof (float));

            for (i = 0; i < maxNsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000); i++)
            {
                doubly_tweaked_deg [i] = tweaked_deg [i];
            }

            for (bad_interval= 0; bad_interval< number_of_bad_intervals; bad_interval++) 
            {
                int delay = delay_in_samples_in_bad_interval  [bad_interval];
                int i;

                for (i = start_sample_of_bad_interval [bad_interval]; i < stop_sample_of_bad_interval [bad_interval]; i++)
                {
                    float h;
                    int j = i + delay;
                    if (j < 0) 
                    {
                        j = 0;
                    }
                    if (j >= maxNsamples) 
                    {
                        j = maxNsamples - 1;

                    }
                    doubly_tweaked_deg [i] = h = tweaked_deg [j];        
                }
            }

            untweaked_deg = deg_info-> data;
            deg_info-> data = doubly_tweaked_deg;

            for (bad_interval= 0; bad_interval < number_of_bad_intervals; bad_interval++)
            {
                for (frame = start_frame_of_bad_interval [bad_interval]; 
                    frame < stop_frame_of_bad_interval [bad_interval]; 
                    frame++) 
                {

                    int start_sample_ref = PESQ_SEARCHBUFFER * pesq->downsample + frame * Nf / 2;
                    int start_sample_deg = start_sample_ref;

                    short_term_fft (Nf, deg_info, Whanning, start_sample_deg, hz_spectrum_deg, fft_tmp);            

                    freq_warping (nr_of_hz_bands_per_bark_band,
                        pow_dens_correction_factor,
                        Nf / 2, hz_spectrum_deg, pesq->bands, pitch_pow_dens_deg, frame,Sp);
                }    

                oldScale = 1;
                for (frame = start_frame_of_bad_interval [bad_interval]; 
                    frame < stop_frame_of_bad_interval [bad_interval]; 
                    frame++) 
                {
                    int band;

                    total_audible_pow_ref = total_audible (abs_thresh_power,frame, pitch_pow_dens_ref, 1,pesq->bands);
                    total_audible_pow_deg = total_audible (abs_thresh_power,frame, pitch_pow_dens_deg, 1,pesq->bands);        

                    scale = (total_audible_pow_ref + (float) 5E3) / (total_audible_pow_deg + (float) 5E3);

                    if (frame > 0)
                    {
                        scale = (float) 0.2 * oldScale + (float) 0.8*scale;
                    }
                    oldScale = scale;

                    if (scale > (float) MAX_SCALE) scale = (float) MAX_SCALE;

                    if (scale < (float) MIN_SCALE) 
                    {
                        scale = (float) MIN_SCALE;
                    }

                    for (band = 0; band < pesq->bands; band++) 
                    {
                        pitch_pow_dens_deg [frame * pesq->bands + band] *= scale;
                    }

                    intensity_warping_of (abs_thresh_power,centre_of_band_bark,
                        loudness_dens_ref, frame, pitch_pow_dens_ref,pesq->bands,Sl); 
                    intensity_warping_of (abs_thresh_power,centre_of_band_bark,
                        loudness_dens_deg, frame, pitch_pow_dens_deg,pesq->bands,Sl); 

                    for (band = 0; band < pesq->bands; band++) 
                    {
                        disturbance_dens [band] = loudness_dens_deg [band] - loudness_dens_ref [band];
                    }

                    for (band = 0; band < pesq->bands; band++) 
                    {
                        deadzone [band] = std::min(loudness_dens_deg [band], loudness_dens_ref [band]);    
                        deadzone [band] *= 0.25;
                    }

                    for (band = 0; band < pesq->bands; band++)
                    {
                        float d = disturbance_dens [band];
                        float m = deadzone [band];

                        if (d > m) 
                        {
                            disturbance_dens [band] -= m;
                        } 
                        else 
                        {
                            if (d < -m) 
                            {
                                disturbance_dens [band] += m;
                            }
                            else
                            {
                                disturbance_dens [band] = 0;
                            }
                        }
                    }    

                    frame_disturbance [frame] = std::min(frame_disturbance [frame] ,
                        pseudo_Lp (width_of_band_bark,pesq->bands, disturbance_dens, PESQ_D_POW_F));    

                    multiply_with_asymmetry_factor (disturbance_dens, frame,pesq->bands, pitch_pow_dens_ref, pitch_pow_dens_deg);

                    frame_disturbance_asym_add [frame] = std::min(frame_disturbance_asym_add [frame],
                        pseudo_Lp (width_of_band_bark,pesq->bands, disturbance_dens, PESQ_A_POW_F));    
                }
            }    
            free (doubly_tweaked_deg);
            deg_info->data = untweaked_deg;
        }
    }


    for (frame = 0; frame <= stop_frame; frame++) 
    {
        float h = 1;

        if (stop_frame + 1 > 1000) 
        {
            long n = (maxNsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample) / (Nf / 2) - 1;
            double timeWeightFactor = (n - (float) 1000) / (float) 5500;
            if (timeWeightFactor > (float) 0.5) timeWeightFactor = (float) 0.5;
            h = (float) (((float) 1.0 - timeWeightFactor) + timeWeightFactor * (float) frame / (float) n);
        }

        time_weight [frame] = h;
    }

    for (frame = 0; frame <= stop_frame; frame++) 
    {

        float h = (float) pow ((total_power_ref [frame] + 1E5) / 1E7, 0.04); 

        frame_disturbance [frame] /= h;
        frame_disturbance_asym_add [frame] /= h;

        if (frame_disturbance [frame] > 45)
        {
            frame_disturbance [frame] = 45;
        }
        if (frame_disturbance_asym_add [frame] > 45)
        {
            frame_disturbance_asym_add [frame] = 45;
        }            
    }

    d_indicator = Lpq_weight (start_frame, stop_frame, PESQ_D_POW_S, PESQ_D_POW_T, frame_disturbance, time_weight);    
    a_indicator = Lpq_weight (start_frame, stop_frame, PESQ_A_POW_S, PESQ_A_POW_T, frame_disturbance_asym_add, time_weight);       

    err_info-> pesq_mos = (float) (4.5 - PESQ_D_WEIGHT * d_indicator - PESQ_A_WEIGHT * a_indicator); 

    free (fft_tmp);
    free (hz_spectrum_ref);
    free (hz_spectrum_deg);
    free (silent);
    free (pitch_pow_dens_ref);
    free (pitch_pow_dens_deg);
    free (frame_was_skipped);
    free (avg_pitch_pow_dens_ref);
    free (avg_pitch_pow_dens_deg);
    free (loudness_dens_ref);
    free (loudness_dens_deg);
    free (deadzone);
    free (disturbance_dens);
    free (disturbance_dens_asym_add);
    free (total_power_ref);

    free (frame_is_bad);
    free (smeared_frame_is_bad);

    free (time_weight);
    free (frame_disturbance);
    free (frame_disturbance_asym_add);
    free (tweaked_deg);

    return;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

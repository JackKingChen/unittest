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

static float filter_interpolate(float freq, const double filter_curve_db [][2],int number_of_points) 
{
    double  result;
    int     i;
    double  freqLow, freqHigh;
    double  curveLow, curveHigh;

    if (freq <= filter_curve_db [0][0]) {
        freqLow = filter_curve_db [0][0];
        curveLow = filter_curve_db [0][1];
        freqHigh = filter_curve_db [1][0];
        curveHigh = filter_curve_db [1][1];

        result = ((freq - freqLow) * curveHigh + (freqHigh - freq) * curveLow)/ (freqHigh - freqLow);

        return (float) result;
    }

    if (freq >= filter_curve_db [number_of_points-1][0]) {
        freqLow = filter_curve_db [number_of_points-2][0];
        curveLow = filter_curve_db [number_of_points-2][1];
        freqHigh = filter_curve_db [number_of_points-1][0];
        curveHigh = filter_curve_db [number_of_points-1][1];

        result = ((freq - freqLow) * curveHigh + (freqHigh - freq) * curveLow)/ (freqHigh - freqLow);

        return (float) result;
    }

    i = 1;
    freqHigh = filter_curve_db [i][0];
    while (freqHigh < freq) {
        i++;
        freqHigh = filter_curve_db [i][0];    
    }
    curveHigh = filter_curve_db [i][1];

    freqLow = filter_curve_db [i-1][0];
    curveLow = filter_curve_db [i-1][1];

    result = ((freq - freqLow) * curveHigh + (freqHigh - freq) * curveLow)/ (freqHigh - freqLow);

    return (float) result;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void pesq_Filter_DC( float * data, long Nsamples,long dn)
{
    float *p;
    long count;
    float facc = 0.0f;

    long ofs = PESQ_SEARCHBUFFER * dn;

    p = data + ofs;
    for( count = (Nsamples - 2 * ofs); count > 0L; count-- )
        facc += *(p++);
    facc /= Nsamples;

    p = data + ofs;
    for( count = (Nsamples - 2 * ofs); count > 0L; count-- )
        *(p++) -= facc;

    p = data + ofs;
    for( count = 0L; count < dn; count++ )
        *(p++) *= (0.5f + count) / dn;

    p = data + Nsamples - ofs - 1L;
    for( count = 0L; count < dn; count++ )
        *(p--) *= (0.5f + count) / dn;
}


void pesq_Filter_IIR(PESQ_t *pesq, float * data, long Nsamples )
{
    pesq_IIR_Filt(pesq->InIIR_Hsos, pesq->InIIR_Nsos, NULL,
        data, Nsamples + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000), NULL );
}

void pesq_Filter_FFT (PESQ_t *pesq, float * data, long maxNsamples, int number_of_points, const double filter_curve_db [][2] )
{ 
    long    n           = maxNsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample + PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000);
    long    pow_of_2    = pesq_dsp_nextpow2 (n);
    float    *x         = (float *) malloc ((pow_of_2 + 2) * sizeof (float));

    float    factorDb, factor;

    float   overallGainFilter = filter_interpolate ((float) 1000, filter_curve_db, number_of_points); 
    float   freq_resolution;
    int        i;

    for (i = 0; i < pow_of_2 + 2; i++) {
        x [i] = 0;
    }

    for (i = 0; i < n; i++) {
        x [i] = data [i + PESQ_SEARCHBUFFER * pesq->downsample];    
    }

    pesq_RealFFT_Forword (x, pow_of_2);

    freq_resolution = (float) pesq->samplerate / (float) pow_of_2;


    for (i = 0; i <= pow_of_2/2; i++) { 
        factorDb = filter_interpolate (i * freq_resolution, filter_curve_db, number_of_points) - overallGainFilter;
        factor = (float) pow ((float) 10, factorDb / (float) 20); 

        x [2 * i] *= factor;       
        x [2 * i + 1] *= factor;   
    }

    pesq_RealFFT_Inverse (x, pow_of_2);

    for (i = 0; i < n; i++) {
        data [i + PESQ_SEARCHBUFFER * pesq->downsample] = x[i];    
    }

    free (x);
}

void pesq_Filter_VAD(PESQ_t *pesq, PESQ_Wave_t * pinfo, float * data, float * VAD, float * logVAD)
{
    float g;
    float LevelThresh;
    float LevelNoise;
    float StDNoise;
    float LevelSig;
    float LevelMin;
    long  count;
    long  iteration;
    long  length;
    long  start;
    long  finish;
    long  Nwindows = (*pinfo).Nsamples / pesq->downsample;

    for( count = 0L; count < Nwindows; count++ )
    {
        VAD[count] = 0.0f;
        for( iteration = 0L; iteration < pesq->downsample; iteration++ )
        {
            g = data[count * pesq->downsample + iteration];
            VAD[count] += (g * g);
        }
        VAD[count] /= pesq->downsample;
    }

    LevelThresh = 0.0f;
    for( count = 0L; count < Nwindows; count++ )
        LevelThresh += VAD[count];
    LevelThresh /= Nwindows;

    LevelMin = 0.0f;
    for( count = 0L; count < Nwindows; count++ )
        if( VAD[count] > LevelMin )
            LevelMin = VAD[count];
    if( LevelMin > 0.0f )
        LevelMin *= 1.0e-4f;
    else
        LevelMin = 1.0f;

    for( count = 0L; count < Nwindows; count++ )
        if( VAD[count] < LevelMin )
            VAD[count] = LevelMin;

    for( iteration = 0L; iteration < 12L; iteration++ )
    {
        LevelNoise = 0.0f;
        StDNoise = 0.0f;
        length = 0L;
        for( count = 0L; count < Nwindows; count++ )
            if( VAD[count] <= LevelThresh )
            {
                LevelNoise += VAD[count];
                length++;
            }
            if( length > 0L )
            {
                LevelNoise /= length;
                for( count = 0L; count < Nwindows; count++ )
                    if( VAD[count] <= LevelThresh )
                    {
                        g = VAD[count] - LevelNoise;
                        StDNoise += g * g;
                    }
                    StDNoise = (float)sqrt(StDNoise / length);
            }

            LevelThresh = 1.001f * (LevelNoise + 2.0f * StDNoise);
    }

    LevelNoise = 0.0f;
    LevelSig = 0.0f;
    length = 0L;
    for( count = 0L; count < Nwindows; count++ )
    {
        if( VAD[count] > LevelThresh )
        {
            LevelSig += VAD[count];
            length++;
        }
        else
            LevelNoise += VAD[count];
    }
    if( length > 0L )
        LevelSig /= length;
    else
        LevelThresh = -1.0f;
    if( length < Nwindows )
        LevelNoise /= (Nwindows - length);
    else
        LevelNoise = 1.0f;

    for( count = 0L; count < Nwindows; count++ )
        if( VAD[count] <= LevelThresh )
            VAD[count] = -VAD[count];

    VAD[0] = -LevelMin;
    VAD[Nwindows-1] = -LevelMin;

    start = 0L;
    finish = 0L;
    for( count = 1; count < Nwindows; count++ )
    {
        if( (VAD[count] > 0.0f) && (VAD[count-1] <= 0.0f) )
            start = count;
        if( (VAD[count] <= 0.0f) && (VAD[count-1] > 0.0f) )
        {
            finish = count;
            if( (finish - start) <= PESQ_MINSPEECHLGTH )
                for( iteration = start; iteration < finish; iteration++ )
                    VAD[iteration] = -VAD[iteration];
        }
    }

    if( LevelSig >= (LevelNoise * 1000.0f) )
    {
        for( count = 1; count < Nwindows; count++ )
        {
            if( (VAD[count] > 0.0f) && (VAD[count-1] <= 0.0f) )
                start = count;
            if( (VAD[count] <= 0.0f) && (VAD[count-1] > 0.0f) )
            {
                finish = count;
                g = 0.0f;
                for( iteration = start; iteration < finish; iteration++ )
                    g += VAD[iteration];
                if( g < 3.0f * LevelThresh * (finish - start) )
                    for( iteration = start; iteration < finish; iteration++ )
                        VAD[iteration] = -VAD[iteration];
            }
        }
    }

    start = 0L;
    finish = 0L;
    for( count = 1; count < Nwindows; count++ )
    {
        if( (VAD[count] > 0.0f) && (VAD[count-1] <= 0.0f) )
        {
            start = count;
            if( (finish > 0L) && ((start - finish) <= PESQ_JOINSPEECHLGTH) )
                for( iteration = finish; iteration < start; iteration++ )
                    VAD[iteration] = LevelMin;
        }
        if( (VAD[count] <= 0.0f) && (VAD[count-1] > 0.0f) )
            finish = count;
    }

    start = 0L;
    for( count = 1; count < Nwindows; count++ )
    {
        if( (VAD[count] > 0.0f) && (VAD[count-1] <= 0.0f) )
            start = count;
    }
    if( start == 0L )
    {
        for( count = 0L; count < Nwindows; count++ )
            VAD[count] = (float)fabs(VAD[count]);
        VAD[0] = -LevelMin;
        VAD[Nwindows-1] = -LevelMin;
    }

    count = 3;
    while( count < (Nwindows-2) )
    {
        if( (VAD[count] > 0.0f) && (VAD[count-2] <= 0.0f) )
        {
            VAD[count-2] = VAD[count] * 0.1f;
            VAD[count-1] = VAD[count] * 0.3f;
            count++;
        }
        if( (VAD[count] <= 0.0f) && (VAD[count-1] > 0.0f) )
        {
            VAD[count] = VAD[count-1] * 0.3f;
            VAD[count+1] = VAD[count-1] * 0.1f;
            count += 3;
        }
        count++;
    }

    for( count = 0L; count < Nwindows; count++ )
        if( VAD[count] < 0.0f ) VAD[count] = 0.0f;

    if( LevelThresh <= 0.0f )
        LevelThresh = LevelMin;
    for( count = 0L; count < Nwindows; count++ )
    {
        if( VAD[count] <= LevelThresh )
            logVAD[count] = 0.0f;
        else
            logVAD[count] = (float)log( VAD[count]/LevelThresh );
    }
}

void pesq_Filter_CrudeAlign(PESQ_t *pesq,PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info, PESQ_Result_t * err_info,
                 long Utt_id, float * ftmp)
{
    long  nr;
    long  nd;
    long  startr;
    long  startd;
    long  count;
    long  I_max;
    float max;
    float * ref_VAD = (*ref_info).logVAD;
    float * deg_VAD = (*deg_info).logVAD;
    float * Y;

    if( Utt_id == PESQ_WHOLE_SIGNAL )
    {
        nr = (*ref_info).Nsamples / pesq->downsample;
        nd = (*deg_info).Nsamples / pesq->downsample;
        startr = 0L;
        startd = 0L;
    }
    else if( Utt_id == PESQ_MAXNUTTERANCES )
    {
        startr = (*err_info).UttSearch_Start[PESQ_MAXNUTTERANCES-1];
        startd = startr + (*err_info).Utt_DelayEst[PESQ_MAXNUTTERANCES-1] / pesq->downsample;

        if ( startd < 0L )
        {
            startr = -(*err_info).Utt_DelayEst[PESQ_MAXNUTTERANCES-1] / pesq->downsample;
            startd = 0L;
        }

        nr = (*err_info).UttSearch_End[PESQ_MAXNUTTERANCES-1] - startr;
        nd = nr;

        if( startd + nd > (*deg_info).Nsamples / pesq->downsample )
            nd = (*deg_info).Nsamples / pesq->downsample - startd;
    }
    else
    {
        startr = (*err_info).UttSearch_Start[Utt_id];
        startd = startr + (*err_info).Crude_DelayEst / pesq->downsample;

        if ( startd < 0L )
        {
            startr = -(*err_info).Crude_DelayEst / pesq->downsample;
            startd = 0L;
        }

        nr = (*err_info).UttSearch_End[Utt_id] - startr;
        nd = nr;

        if( startd + nd > (*deg_info).Nsamples / pesq->downsample )
            nd = (*deg_info).Nsamples / pesq->downsample - startd;
    }

    Y  = ftmp;

    if( (nr > 1L) && (nd > 1L) )
        pesq_RealFFT_Correct( ref_VAD + startr, nr, deg_VAD + startd, nd, Y );

    max = 0.0f;
    I_max = nr - 1;
    if( (nr > 1L) && (nd > 1L) )
        for( count = 0L; count < (nr+nd-1); count++ )
            if( Y[count] > max )
            {
                max = Y[count];
                I_max = count;
            }

            if( Utt_id == PESQ_WHOLE_SIGNAL )
            {
                (*err_info).Crude_DelayEst = (I_max - nr + 1) * pesq->downsample;
                (*err_info).Crude_DelayConf = 0.0f;
            }
            else if( Utt_id == PESQ_MAXNUTTERANCES )
            {
                (*err_info).Utt_Delay[PESQ_MAXNUTTERANCES-1] =
                    (I_max - nr + 1) * pesq->downsample + (*err_info).Utt_DelayEst[PESQ_MAXNUTTERANCES-1];
            }
            else
            {
                (*err_info).Utt_DelayEst[Utt_id] =
                    (I_max - nr + 1) * pesq->downsample + (*err_info).Crude_DelayEst;
            }
}

void pesq_Filter_TimeAlign(PESQ_t *pesq,PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info, PESQ_Result_t * err_info,
                long Utt_id, float * ftmp)
{
    long  count;
    long  I_max;
    float v_max;
    long  estdelay;
    long  startr;
    long  startd;
    float * X1;
    float * X2;
    float * H;
    float * Window;
    float r1, i1;
    long  kernel;
    float Hsum;

    estdelay = (*err_info).Utt_DelayEst[Utt_id];

    X1 = ftmp;
    X2 = ftmp + pesq->alignNfft + 2;
    H  = (ftmp + 4 + 2 * pesq->alignNfft);
    for( count = 0L; count < pesq->alignNfft; count++ )
        H[count] = 0.0f;
    Window = ftmp + 5 * pesq->alignNfft;

    for( count = 0L; count < pesq->alignNfft; count++ )
        Window[count] = (float)(0.5 * (1.0 - cos((PESQ_TWOPI * count) / pesq->alignNfft)));

    startr = (*err_info).UttSearch_Start[Utt_id] * pesq->downsample;
    startd = startr + estdelay;

    if ( startd < 0L )
    {
        startr = -estdelay;
        startd = 0L;
    }

    while( ((startd + pesq->alignNfft) <= (*deg_info).Nsamples) &&
        ((startr + pesq->alignNfft) <= ((*err_info).UttSearch_End[Utt_id] * pesq->downsample)) )
    {
        for( count = 0L; count < pesq->alignNfft; count++ )
        {
            X1[count] = (*ref_info).data[count + startr] * Window[count];
            X2[count] = (*deg_info).data[count + startd] * Window[count];

        }
        pesq_RealFFT_Forword( X1, pesq->alignNfft );
        pesq_RealFFT_Forword( X2, pesq->alignNfft );

        for( count = 0L; count <= pesq->alignNfft / 2; count++ )
        {
            r1 = X1[count * 2]; i1 = -X1[1 + (count * 2)];
            X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
            X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
        }

        pesq_RealFFT_Inverse( X1, pesq->alignNfft );

        v_max = 0.0f;
        for( count = 0L; count < pesq->alignNfft; count++ )
        {
            r1 = (float) fabs(X1[count]);
            X1[count] = r1;
            if( r1 > v_max ) v_max = r1;
        }
        v_max *= 0.99f;
        for( count = 0L; count < pesq->alignNfft; count++ )
            if( X1[count] > v_max )
                H[count] += pow( v_max, 0.125f );

        startr += (pesq->alignNfft / 4);
        startd += (pesq->alignNfft / 4);
    }

    Hsum = 0.0f;
    for( count = 0L; count < pesq->alignNfft; count++ )
    {
        Hsum += H[count];
        X1[count] = H[count];
        X2[count] = 0.0f;        
    }

    X2[0] = 1.0f;
    kernel = pesq->alignNfft / 64;
    for( count = 1; count < kernel; count++ )
    {
        X2[count] = 1.0f - ((float)count) / ((float)kernel);
        X2[(pesq->alignNfft - count)] = 1.0f - ((float)count) / ((float)kernel);
    }
    pesq_RealFFT_Forword( X1, pesq->alignNfft );
    pesq_RealFFT_Forword( X2, pesq->alignNfft );

    for( count = 0L; count <= pesq->alignNfft / 2; count++ )
    {
        r1 = X1[count * 2]; i1 = X1[1 + (count * 2)];
        X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
        X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
    }
    pesq_RealFFT_Inverse( X1, pesq->alignNfft );

    for( count = 0L; count < pesq->alignNfft; count++ )
    {
        if( Hsum > 0.0 )
            H[count] = (float) fabs(X1[count]) / Hsum;
        else
            H[count] = 0.0f;
    }

    v_max = 0.0f;
    I_max = 0L;
    for( count = 0L; count < pesq->alignNfft; count++ )
        if( H[count] > v_max )
        {
            v_max = H[count];
            I_max = count;
        }
        if( I_max >= (pesq->alignNfft/2) )
            I_max -= pesq->alignNfft;

        (*err_info).Utt_Delay[Utt_id] = estdelay + I_max;
        (*err_info).Utt_DelayConf[Utt_id] = v_max;
}

void pesq_Filter_SplitAlign(PESQ_t *pesq, PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info,
                 PESQ_Result_t * err_info, float * ftmp,
                 long Utt_Start, long Utt_SpeechStart, long Utt_SpeechEnd, long Utt_End,
                 long Utt_DelayEst, float Utt_DelayConf,
                 long * Best_ED1, long * Best_D1, float * Best_DC1,
                 long * Best_ED2, long * Best_D2, float * Best_DC2,
                 long * Best_BP)
{
    long count, bp, k;
    long Utt_Len = Utt_SpeechEnd - Utt_SpeechStart;
    long Utt_Test = PESQ_MAXNUTTERANCES - 1;

    long N_BPs;
    long Utt_BPs[41];
    long Utt_ED1[41], Utt_ED2[41];
    long Utt_D1[41], Utt_D2[41];
    float Utt_DC1[41], Utt_DC2[41];

    long Delta, Step, Pad;

    long  estdelay;
    long  I_max;
    float v_max, n_max;
    long  startr;
    long  startd;
    float * X1;
    float * X2;
    float * H;
    float * Window;
    float r1, i1;
    long  kernel;
    float Hsum;

    *Best_DC1 = 0.0f;
    *Best_DC2 = 0.0f;

    X1 = ftmp;
    X2 = ftmp + 2 + pesq->alignNfft;
    H  = (ftmp + 4 + 2 * pesq->alignNfft);
    Window = ftmp + 6 + 3 * pesq->alignNfft;
    for( count = 0L; count < pesq->alignNfft; count++ )
        Window[count] = (float)(0.5 * (1.0 - cos((PESQ_TWOPI * count) / pesq->alignNfft)));
    kernel = pesq->alignNfft / 64;

    Delta = pesq->alignNfft / (4 * pesq->downsample);

    Step = (long) ((0.801 * Utt_Len + 40 * Delta - 1)/(40 * Delta));
    Step *= Delta;

    Pad = Utt_Len / 10;
    if( Pad < 75 ) Pad = 75;
    Utt_BPs[0] = Utt_SpeechStart + Pad;
    N_BPs = 0;
    do {
        N_BPs++;
        Utt_BPs[N_BPs] = Utt_BPs[N_BPs-1] + Step;
    } while( (Utt_BPs[N_BPs] <= (Utt_SpeechEnd - Pad)) && (N_BPs < 40) );

    if( N_BPs <= 0 ) return;  

    for( bp = 0; bp < N_BPs; bp++ )
    {
        (*err_info).Utt_DelayEst[Utt_Test] = Utt_DelayEst;
        (*err_info).UttSearch_Start[Utt_Test] = Utt_Start;
        (*err_info).UttSearch_End[Utt_Test] = Utt_BPs[bp];

        pesq_Filter_CrudeAlign(pesq, ref_info, deg_info, err_info, PESQ_MAXNUTTERANCES, ftmp);
        Utt_ED1[bp] = (*err_info).Utt_Delay[Utt_Test];

        (*err_info).Utt_DelayEst[Utt_Test] = Utt_DelayEst;
        (*err_info).UttSearch_Start[Utt_Test] = Utt_BPs[bp];
        (*err_info).UttSearch_End[Utt_Test] = Utt_End;

        pesq_Filter_CrudeAlign(pesq, ref_info, deg_info, err_info, PESQ_MAXNUTTERANCES, ftmp);
        Utt_ED2[bp] = (*err_info).Utt_Delay[Utt_Test];
    }

    for( bp = 0; bp < N_BPs; bp++ )
        Utt_DC1[bp] = -2.0f;
    while( 1 )
    {
        bp = 0;
        while( (bp < N_BPs) && (Utt_DC1[bp] > -2.0) )
            bp++;
        if( bp >= N_BPs )
            break;

        estdelay = Utt_ED1[bp];

        for( count = 0L; count < pesq->alignNfft; count++ )
            H[count] = 0.0f;
        Hsum = 0.0f;

        startr = Utt_Start * pesq->downsample;
        startd = startr + estdelay;

        if ( startd < 0L )
        {
            startr = -estdelay;
            startd = 0L;
        }

        while( ((startd + pesq->alignNfft) <= (*deg_info).Nsamples) &&
            ((startr + pesq->alignNfft) <= (Utt_BPs[bp] * pesq->downsample)) )
        {
            for( count = 0L; count < pesq->alignNfft; count++ )
            {
                X1[count] = (*ref_info).data[count + startr] * Window[count];
                X2[count] = (*deg_info).data[count + startd] * Window[count];                
            }
            pesq_RealFFT_Forword( X1, pesq->alignNfft );
            pesq_RealFFT_Forword( X2, pesq->alignNfft );

            for( count = 0L; count <= pesq->alignNfft / 2; count++ )
            {
                r1 = X1[count * 2]; i1 = -X1[1 + (count * 2)];
                X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
                X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
            }

            pesq_RealFFT_Inverse( X1, pesq->alignNfft );

            v_max = 0.0f;
            for( count = 0L; count < pesq->alignNfft; count++ )
            {
                r1 = (float) fabs(X1[count]);
                X1[count] = r1;
                if( r1 > v_max ) v_max = r1;
            }
            v_max *= 0.99f;
            n_max = pow( v_max, 0.125f ) / kernel;

            for( count = 0L; count < pesq->alignNfft; count++ )
                if( X1[count] > v_max )
                {
                    Hsum += n_max * kernel;
                    for( k = 1-kernel; k < kernel; k++ )
                        H[(count + k + pesq->alignNfft) % pesq->alignNfft] +=
                        n_max * (kernel - fabs((float)k));
                }

                startr += (pesq->alignNfft / 4);
                startd += (pesq->alignNfft / 4);
        }

        v_max = 0.0f;
        I_max = 0L;
        for( count = 0L; count < pesq->alignNfft; count++ )
            if( H[count] > v_max )
            {
                v_max = H[count];
                I_max = count;
            }
            if( I_max >= (pesq->alignNfft/2) )
                I_max -= pesq->alignNfft;

            Utt_D1[bp] = estdelay + I_max;
            if( Hsum > 0.0 )
                Utt_DC1[bp] = v_max / Hsum;
            else
                Utt_DC1[bp] = 0.0f;

            while( bp < (N_BPs - 1) )
            {
                bp++;
                if( (Utt_ED1[bp] == estdelay) && (Utt_DC1[bp] <= -2.0) )
                {
                    while( ((startd + pesq->alignNfft) <= (*deg_info).Nsamples) &&
                        ((startr + pesq->alignNfft) <= (Utt_BPs[bp] * pesq->downsample)) )
                    {
                        for( count = 0L; count < pesq->alignNfft; count++ )
                        {
                            X1[count] = (*ref_info).data[count + startr] * Window[count];
                            X2[count] = (*deg_info).data[count + startd] * Window[count];
                        }
                        pesq_RealFFT_Forword( X1, pesq->alignNfft );
                        pesq_RealFFT_Forword( X2, pesq->alignNfft );

                        for( count = 0L; count <= pesq->alignNfft/2; count++ )
                        {
                            r1 = X1[count * 2]; i1 = -X1[1 + (count * 2)];
                            X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
                            X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
                        }

                        pesq_RealFFT_Inverse( X1, pesq->alignNfft );

                        v_max = 0.0f;
                        for( count = 0L; count < pesq->alignNfft; count++ )
                        {
                            r1 = (float) fabs(X1[count]);
                            X1[count] = r1;
                            if( r1 > v_max ) v_max = r1;
                        }
                        v_max *= 0.99f;
                        n_max = pow( v_max, 0.125f ) / kernel;

                        for( count = 0L; count < pesq->alignNfft; count++ )
                            if( X1[count] > v_max )
                            {
                                Hsum += n_max * kernel;
                                for( k = 1-kernel; k < kernel; k++ )
                                    H[(count + k + pesq->alignNfft) % pesq->alignNfft] +=
                                    n_max * (kernel - fabs((float)k));
                            }

                            startr += (pesq->alignNfft / 4);
                            startd += (pesq->alignNfft / 4);
                    }

                    v_max = 0.0f;
                    I_max = 0L;
                    for( count = 0L; count < pesq->alignNfft; count++ )
                        if( H[count] > v_max )
                        {
                            v_max = H[count];
                            I_max = count;
                        }
                        if( I_max >= (pesq->alignNfft/2) )
                            I_max -= pesq->alignNfft;

                        Utt_D1[bp] = estdelay + I_max;
                        if( Hsum > 0.0 )
                            Utt_DC1[bp] = v_max / Hsum;
                        else
                            Utt_DC1[bp] = 0.0f;
                }
            }
    }

    for( bp = 0; bp < N_BPs; bp++ )
    {
        if( Utt_DC1[bp] > Utt_DelayConf )
            Utt_DC2[bp] = -2.0f;
        else
            Utt_DC2[bp] = 0.0f;
    }
    while( 1 )
    {
        bp = N_BPs - 1;
        while( (bp >= 0) && (Utt_DC2[bp] > -2.0) )
            bp--;
        if( bp < 0 )
            break;

        estdelay = Utt_ED2[bp];

        for( count = 0L; count < pesq->alignNfft; count++ )
            H[count] = 0.0f;
        Hsum = 0.0f;

        startr = Utt_End * pesq->downsample - pesq->alignNfft;
        startd = startr + estdelay;

        if ( (startd + pesq->alignNfft) > (*deg_info).Nsamples )
        {
            startd = (*deg_info).Nsamples - pesq->alignNfft;
            startr = startd - estdelay;
        }

        while( (startd >= 0L) &&
            (startr >= (Utt_BPs[bp] * pesq->downsample)) )
        {
            for( count = 0L; count < pesq->alignNfft; count++ )
            {
                X1[count] = (*ref_info).data[count + startr] * Window[count];
                X2[count] = (*deg_info).data[count + startd] * Window[count];
            }
            pesq_RealFFT_Forword( X1, pesq->alignNfft );
            pesq_RealFFT_Forword( X2, pesq->alignNfft );

            for( count = 0L; count <= pesq->alignNfft/2; count++ )
            {
                r1 = X1[count * 2]; i1 = -X1[1 + (count * 2)];
                X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
                X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
            }

            pesq_RealFFT_Inverse( X1, pesq->alignNfft );

            v_max = 0.0f;
            for( count = 0L; count < pesq->alignNfft; count++ )
            {
                r1 = fabs(X1[count]);
                X1[count] = r1;
                if( r1 > v_max ) v_max = r1;
            }
            v_max *= 0.99f;
            n_max = pow( v_max, 0.125f ) / kernel;

            for( count = 0L; count < pesq->alignNfft; count++ )
                if( X1[count] > v_max )
                {
                    Hsum += n_max * kernel;
                    for( k = 1-kernel; k < kernel; k++ )
                        H[(count + k + pesq->alignNfft) % pesq->alignNfft] +=
                        n_max * (kernel - fabs((float)k));
                }

                startr -= (pesq->alignNfft / 4);
                startd -= (pesq->alignNfft / 4);
        }

        v_max = 0.0f;
        I_max = 0L;
        for( count = 0L; count < pesq->alignNfft; count++ )
            if( H[count] > v_max )
            {
                v_max = H[count];
                I_max = count;
            }
            if( I_max >= (pesq->alignNfft/2) )
                I_max -= pesq->alignNfft;

            Utt_D2[bp] = estdelay + I_max;
            if( Hsum > 0.0 )
                Utt_DC2[bp] = v_max / Hsum;
            else
                Utt_DC2[bp] = 0.0f;

            while( bp > 0 )
            {
                bp--;
                if( (Utt_ED2[bp] == estdelay) && (Utt_DC2[bp] <= -2.0) )
                {
                    while( (startd >= 0L) &&
                        (startr >= (Utt_BPs[bp] * pesq->downsample)) )
                    {
                        for( count = 0L; count < pesq->alignNfft; count++ )
                        {
                            X1[count] = (*ref_info).data[count + startr] * Window[count];
                            X2[count] = (*deg_info).data[count + startd] * Window[count];
                        }
                        pesq_RealFFT_Forword( X1, pesq->alignNfft );
                        pesq_RealFFT_Forword( X2, pesq->alignNfft );

                        for( count = 0L; count <= pesq->alignNfft / 2; count++ )
                        {
                            r1 = X1[count * 2]; i1 = -X1[1 + (count * 2)];
                            X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
                            X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
                        }

                        pesq_RealFFT_Inverse( X1, pesq->alignNfft );

                        v_max = 0.0f;
                        for( count = 0L; count < pesq->alignNfft; count++ )
                        {
                            r1 = (float) fabs(X1[count]);
                            X1[count] = r1;
                            if( r1 > v_max ) v_max = r1;
                        }
                        v_max *= 0.99f;
                        n_max = (float) pow( v_max, 0.125f ) / kernel;

                        for( count = 0L; count < pesq->alignNfft; count++ )
                            if( X1[count] > v_max )
                            {
                                Hsum += n_max * kernel;
                                for( k = 1-kernel; k < kernel; k++ )
                                    H[(count + k + pesq->alignNfft) % pesq->alignNfft] +=
                                    n_max * (kernel - (float) fabs((float)k));
                            }

                            startr -= (pesq->alignNfft / 4);
                            startd -= (pesq->alignNfft / 4);
                    }

                    v_max = 0.0f;
                    I_max = 0L;
                    for( count = 0L; count < pesq->alignNfft; count++ )
                        if( H[count] > v_max )
                        {
                            v_max = H[count];
                            I_max = count;
                        }
                        if( I_max >= (pesq->alignNfft/2) )
                            I_max -= pesq->alignNfft;

                        Utt_D2[bp] = estdelay + I_max;
                        if( Hsum > 0.0 )
                            Utt_DC2[bp] = v_max / Hsum;
                        else
                            Utt_DC2[bp] = 0.0f;
                }
            }
    }

    for( bp = 0; bp < N_BPs; bp++ )
    {
        if( (abs(Utt_D2[bp] - Utt_D1[bp]) >= pesq->downsample) &&
            ((Utt_DC1[bp] + Utt_DC2[bp]) > ((*Best_DC1) + (*Best_DC2))) &&
            (Utt_DC1[bp] > Utt_DelayConf) && (Utt_DC2[bp] > Utt_DelayConf) )
        {
            *Best_ED1 = Utt_ED1[bp]; *Best_D1 = Utt_D1[bp]; *Best_DC1 = Utt_DC1[bp];
            *Best_ED2 = Utt_ED2[bp]; *Best_D2 = Utt_D2[bp]; *Best_DC2 = Utt_DC2[bp];
            *Best_BP = Utt_BPs[bp];
        }
    }
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
